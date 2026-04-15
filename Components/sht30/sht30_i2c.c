#include "sht30_i2c.h"
#include "i2c.h"        /* 您的 I2C 底层收发函数 */
#include "delay.h"

static I2C_TypeDef *g_i2c_handle = SHT30_I2C_INTERFACE;

/**
 * @brief       初始化 SHT30 使用的 I2C 硬件（GPIO + I2C 外设）
 * @param       clock_speed: I2C 时钟速率（如 100000）
 * @retval      无
 */
void sht30_i2c_init(uint32_t clock_speed)
{
    GPIO_InitTypeDef gpio;
    I2C_InitTypeDef i2c;

    /* 使能 GPIO 和 I2C 时钟 */
    SHT30_I2C_SCL_GPIO_CLK_ENABLE();
    SHT30_I2C_SDA_GPIO_CLK_ENABLE();
    SHT30_I2C_CLK_ENABLE();

    /* 配置 SCL、SDA 为复用开漏输出 */
    gpio.GPIO_Pin   = SHT30_I2C_SCL_GPIO_PIN;
    gpio.GPIO_Mode  = GPIO_Mode_AF_OD;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(SHT30_I2C_SCL_GPIO_PORT, &gpio);

    gpio.GPIO_Pin   = SHT30_I2C_SDA_GPIO_PIN;
    GPIO_Init(SHT30_I2C_SDA_GPIO_PORT, &gpio);

    /* 配置 I2C 外设 */
    I2C_StructInit(&i2c);
    i2c.I2C_Mode                = I2C_Mode_I2C;
    i2c.I2C_DutyCycle           = I2C_DutyCycle_2;
    i2c.I2C_OwnAddress1         = 0x00;
    i2c.I2C_Ack                 = I2C_Ack_Enable;
    i2c.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    i2c.I2C_ClockSpeed          = clock_speed;
    I2C_Init(g_i2c_handle, &i2c);

    /* 使能 I2C */
    I2C_Cmd(g_i2c_handle, ENABLE);
}

/**
 * @brief       向 SHT30 发送 16 位命令（先发高字节，再发低字节）
 * @param       cmd: 16 位命令码（如 0x2C06）
 * @retval      SHT30_EOK: 成功，SHT30_ERROR: 失败
 */
int sht30_i2c_write_cmd(uint16_t cmd)
{
    uint8_t buf[2];
    buf[0] = (cmd >> 8) & 0xFF;   /* MSB */
    buf[1] = cmd & 0xFF;          /* LSB */

    if (i2c_send_bytes(g_i2c_handle, SHT30_SLAVE_ADDR, buf, 2) != 0) {
        return SHT30_ERROR;
    }
    return SHT30_EOK;
}

/**
 * @brief       从 SHT30 读取指定长度的数据
 * @param       buf: 接收缓冲区
 * @param       len: 要读取的字节数（通常为 6）
 * @retval      SHT30_EOK: 成功，SHT30_ERROR: 失败
 */
int sht30_i2c_read_raw(uint8_t *buf, uint8_t len)
{
    if (i2c_receive_bytes(g_i2c_handle, SHT30_SLAVE_ADDR, buf, len) != 0) {
        return SHT30_ERROR;
    }
    return SHT30_EOK;
}
