#include "bh1750_i2c.h"

static I2C_TypeDef *g_i2c_handle = BH1750_I2C_INTERFACE;    /* BH1750 I2C */

/**
 * @brief       初始化 BH1750 使用的 I2C 硬件（GPIO + I2C 外设）
 * @param       clock_speed: I2C 时钟速率（如 100000）
 * @retval      无
 */
void bh1750_i2c_init(uint32_t clock_speed)
{
    GPIO_InitTypeDef gpio;
    I2C_InitTypeDef i2c;

    /* 1. 使能 GPIO 和 I2C 时钟 */
    BH1750_I2C_SCL_GPIO_CLK_ENABLE();
    BH1750_I2C_SDA_GPIO_CLK_ENABLE();
    BH1750_I2C_CLK_ENABLE();

    /* 2. 配置 SCL、SDA 为复用开漏输出 */
    gpio.GPIO_Pin   = BH1750_I2C_SCL_GPIO_PIN;
    gpio.GPIO_Mode  = GPIO_Mode_AF_OD;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(BH1750_I2C_SCL_GPIO_PORT, &gpio);

    gpio.GPIO_Pin   = BH1750_I2C_SDA_GPIO_PIN;
    GPIO_Init(BH1750_I2C_SDA_GPIO_PORT, &gpio);

    /* 3. 配置 I2C 外设 */
    I2C_StructInit(&i2c);
    i2c.I2C_Mode                = I2C_Mode_I2C;
    i2c.I2C_DutyCycle           = I2C_DutyCycle_2;
    i2c.I2C_OwnAddress1         = 0x00;
    i2c.I2C_Ack                 = I2C_Ack_Enable;
    i2c.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    i2c.I2C_ClockSpeed          = clock_speed;
    I2C_Init(g_i2c_handle, &i2c);

    /* 4. 使能 I2C */
    I2C_Cmd(g_i2c_handle, ENABLE);
}

/**
 * @brief       通过 I2C 发送单字节命令
 * @param       cmd: 命令字节
 * @retval      BH1750_EOK: 成功，BH1750_ERROR: 失败
 */
int bh1750_i2c_write_cmd(uint8_t cmd)
{
    if (i2c_send_bytes(g_i2c_handle, BH1750_SLAVE_ADDR, &cmd, 1) != 0) 
    {
        return BH1750_ERROR;
    }
    return BH1750_EOK;
}

/**
 * @brief       通过 I2C 读取 2 字节原始光照数据
 * @param       raw: 输出 16 位原始值
 * @retval      BH1750_EOK: 成功，BH1750_ERROR: 失败
 */
int bh1750_i2c_read_raw(uint16_t *raw)
{
    uint8_t buf[2];
    if (i2c_receive_bytes(g_i2c_handle, BH1750_SLAVE_ADDR, buf, 2) != 0) 
    {
        return BH1750_ERROR;
    }
    *raw = ((uint16_t)buf[0] << 8) | buf[1];
    return BH1750_EOK;
}
