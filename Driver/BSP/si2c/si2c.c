#include "si2c.h"

void delay(uint32_t us) { for(uint32_t i = 0; i<8*us; i++); }

/**
 * @brief       对软件I2C硬件进行初始化
 */
static void si2c_hw_init(void)
{
	GPIO_InitTypeDef gpio;

	// #1. 使能SCL引脚的时钟
	SI2C_SCL_GPIO_CLK_ENABLE();
	
	// #2. 对SCL和SDA写1
	SI2C_SCL_WRITE(1);
	SI2C_SDA_WRITE(1);
	
	// #2. 使能SDA引脚的时钟
	SI2C_SDA_GPIO_CLK_ENABLE();
	
	// #3. 初始化SCL引脚为输出开漏
	gpio.GPIO_Pin 	= SI2C_SCL_GPIO_PIN;
	gpio.GPIO_Mode 	= GPIO_Mode_Out_OD;
	gpio.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(SI2C_SCL_GPIO_PORT, &gpio);
	
	// #4. 初始化SDA引脚为输出开漏
	gpio.GPIO_Pin 	= SI2C_SDA_GPIO_PIN;
	gpio.GPIO_Mode 	= GPIO_Mode_Out_OD;
	gpio.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(SI2C_SDA_GPIO_PORT, &gpio);
}

/**
 * @brief       对软件I2C进行初始化
 */
void si2c_init(void)
{
	si2c_hw_init();
}

/**
 * @brief       发送一个字节
 * @param       Byte: 要发送的字节
 * @retval      0: ACK，其它: NAK
 */
static uint8_t si2c_send_byte(uint8_t Byte)
{
	for(int8_t i=7; i>=0; i--)
	{
		SI2C_SCL_WRITE(0); // 将SCL拉低
		SI2C_SDA_WRITE((Byte & (0x01<<i)) ? 1 : 0); // 变SDA的电压
		delay(2); // 延迟1/2周期
		
		SI2C_SCL_WRITE(1); // 将SCL拉高
		delay(2); // 延迟1/2周期
	}
	
	// 读取ACK
	SI2C_SCL_WRITE(0); // 将SCL拉低
	SI2C_SDA_WRITE(1); // 将SDA释放
	delay(2); // 延迟1/4周期
	
	SI2C_SCL_WRITE(1); // 将SCL拉高
	delay(2); // 延迟1/4周期
	
	// 返回ACK状态（0表示ACK，1表示NAK）
	return SI2C_SDA_READ();
}

/**
 * @brief       通过软件I2C向从机写入多个字节
 * @param       Addr: 填写从机的地址，左对齐 - A6 A5 A4 A3 A2 A1 A0 0
 * @param       pData: 要发送的数据（数组）
 * @param       Size: 要发送的数据的数量，以字节为单位
 * @retval      0: 发送成功
 * @retval      -1: 寻址失败
 * @retval      -2: 数据被拒收
 */
__weak int si2c_send_bytes(uint8_t Addr, const uint8_t *pData, uint16_t Size)
{
	SI2C_SDA_WRITE(1);
	SI2C_SCL_WRITE(1);
	
	// #1. 发送起始位
	SI2C_SDA_WRITE(0);
	delay(1);
	
	// #2. 发送从机地址+RW
	if(si2c_send_byte(Addr & 0xfe) != 0)
	{
		si2c_send_stop();
		return -1; // 寻址失败
	}
	
	// #3. 发送数据
	for(uint16_t i=0; i<Size; i++)
	{
		if(si2c_send_byte(pData[i]) != 0)
		{
			si2c_send_stop();
			return -2; // 数据被拒收
		}
	}
	
	// #4. 发送停止位
	si2c_send_stop();
	
	return 0;
}

/**
 * @brief       从从机读取一个字节的数据
 * @param       Ack: 0 - 回NAK，1 - 回ACK
 * @retval      读取到的数据
 */
static uint8_t si2c_receive_byte(uint8_t Ack)
{
	uint8_t ret = 0;
	
	for(int8_t i=7; i>=0; i--)
	{
		SI2C_SCL_WRITE(0); // scl拉低
		SI2C_SDA_WRITE(1); // 释放SDA
		delay(2); // 延迟1/2周期
		SI2C_SCL_WRITE(1); // scl拉高
		delay(2); // 延迟1/2周期
		
		if(SI2C_SDA_READ() == Bit_SET) // 如果读到的比特位为1
		{
			ret |= 0x01 << i; // 写入比特位
		}
		else // 如果读到的比特位为0
		{
			// 什么也不干
		}
	}
	
	// 回复ACK或NAK
	
	SI2C_SCL_WRITE(0); // scl拉低
	
	if(Ack)
	{
		SI2C_SDA_WRITE(0); // sda拉低
	}
	else
	{
		SI2C_SDA_WRITE(1); // sda拉高
	}
	
	delay(2); // 延迟1/2周期
	
	return ret;
}

/**
 * @brief       通过软件I2C从从机读多个字节
 * @param       Addr: 填写从机的地址，左对齐 - A6 A5 A4 A3 A2 A1 A0 0
 * @param       pBuffer: 接收缓冲区（数组）
 * @param       Size: 要读取的数据的数量，以字节为单位
 * @retval      0: 发送成功
 * @retval      -1: 寻址失败
 */
__weak int si2c_receive_bytes(uint8_t Addr, uint8_t *pBuffer, uint16_t Size)
{
	SI2C_SDA_WRITE(1);
	SI2C_SCL_WRITE(1);
	
	// #1. 发送起始位
	SI2C_SDA_WRITE(0);
	delay(1);
	
	// #2. 发送从机地址+RW
	if(si2c_send_byte(Addr | 0x01) != 0)
	{
		si2c_send_stop();
		return -1; // 寻址失败
	}
	
	// #3. 接收
	for(uint16_t i=0; i<Size; i++)
	{
		// 最后字节回NAK(0)，其他回ACK(1)
		pBuffer[i] = si2c_receive_byte((i == Size-1) ? 0 : 1);
	}
	
	// #4. 发送停止位
	si2c_send_stop();
	
	return 0;
}

/**
 * @brief 回调函数
 * 
 * @param addr 
 * @param pdata 
 * @param size 
 * @return int 
 */
int si2c_write_bytes(uint8_t addr, const uint8_t *pdata, uint16_t size)
{
	return si2c_send_bytes(addr, pdata, size);
}

/**
 * @brief       发送停止位
 */
static void si2c_send_stop(void)
{
	SI2C_SCL_WRITE(0); 	// scl拉低
	delay(1); 			// 延迟1/4周期
	SI2C_SDA_WRITE(0); 	// sda拉低
	delay(1); 			// 延迟1/4周期
	SI2C_SCL_WRITE(1); 	// scl拉高
	delay(1); 			// 延迟1/4周期
	SI2C_SDA_WRITE(1); 	// sda拉高
	delay(1); 			// 延迟1/4周期
}
