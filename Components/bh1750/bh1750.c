#include "bh1750.h"

static BH1750_Mode_t g_bh1750_mode = BH1750_MODE_CONT_H_RES;

/* ==================== 硬件初始化（ADDR 引脚） ==================== */
static void bh1750_hw_init(void)
{
    GPIO_InitTypeDef gpio;

    BH1750_ADDR_GPIO_CLK_ENABLE();

    gpio.GPIO_Pin   = BH1750_ADDR_GPIO_PIN;
    gpio.GPIO_Mode  = GPIO_Mode_Out_PP;
    gpio.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(BH1750_ADDR_GPIO_PORT, &gpio);

    /* 根据 BH1750_ADDR_STATE 设置 ADDR 引脚电平（来自 bh1750_i2c.h） */
    BH1750_ADDR(BH1750_ADDR_STATE);
}

/* ==================== 内部辅助函数 ==================== */
static uint8_t bh1750_send_cmd(uint8_t cmd)
{
    return bh1750_i2c_write_cmd(cmd);
}

static uint8_t bh1750_read_raw(uint16_t *raw)
{
    return bh1750_i2c_read_raw(raw);
}

static float bh1750_raw_to_lux(uint16_t raw, BH1750_Mode_t mode)
{
    float lux = raw / 1.2f;
    if (mode == BH1750_MODE_CONT_H_RES2 || mode == BH1750_MODE_ONE_H_RES2) 
    {
        lux /= 2.0f;
    }
    return lux;
}

static uint16_t bh1750_get_meas_time(BH1750_Mode_t mode)
{
    switch (mode) 
    {
        case BH1750_MODE_CONT_H_RES:
        case BH1750_MODE_CONT_H_RES2:
        case BH1750_MODE_ONE_H_RES:
        case BH1750_MODE_ONE_H_RES2:
            return BH1750_MEAS_TIME_H_RES;
        case BH1750_MODE_CONT_L_RES:
        case BH1750_MODE_ONE_L_RES:
            return BH1750_MEAS_TIME_L_RES;
        default:
            return 0;
    }
}

static uint8_t bh1750_mode_to_cmd(BH1750_Mode_t mode)
{
    switch (mode) 
    {
        case BH1750_MODE_CONT_H_RES:   return BH1750_CMD_CONT_H_RES;
        case BH1750_MODE_CONT_H_RES2:  return BH1750_CMD_CONT_H_RES2;
        case BH1750_MODE_CONT_L_RES:   return BH1750_CMD_CONT_L_RES;
        case BH1750_MODE_ONE_H_RES:    return BH1750_CMD_ONE_H_RES;
        case BH1750_MODE_ONE_H_RES2:   return BH1750_CMD_ONE_H_RES2;
        case BH1750_MODE_ONE_L_RES:    return BH1750_CMD_ONE_L_RES;
        default: return 0;
    }
}

/* ==================== 用户 API ==================== */

/**
 * @brief       初始化 BH1750 光照传感器
 * @param       无
 * @retval      BH1750_EOK: 成功，其他: 失败
 */
uint8_t bh1750_init(void)
{
    uint8_t ret;

    /* 1. 初始化 ADDR 引脚（先拉好电平） */
    bh1750_hw_init();

    /* 2. 初始化 I2C 硬件 */
    bh1750_i2c_init(100000);

    /* 3. 等待芯片上电稳定 */
    delay_ms(10);

    /* 4. 发送上电指令 */
    ret = bh1750_send_cmd(BH1750_CMD_POWER_ON);
    if (ret != BH1750_EOK) 
    {
        return ret;
    }

    /* 5. 设置为默认连续高分辨率模式 */
    g_bh1750_mode = BH1750_MODE_CONT_H_RES;
    return bh1750_send_cmd(BH1750_CMD_CONT_H_RES);
}

/**
 * @brief       打开 BH1750 电源
 * @param       无
 * @retval      BH1750_EOK: 成功，其他: 失败
 */
uint8_t bh1750_power_on(void)
{
    return bh1750_send_cmd(BH1750_CMD_POWER_ON);
}

/**
 * @brief       关闭 BH1750 电源（进入低功耗模式）
 * @param       无
 * @retval      BH1750_EOK: 成功，其他: 失败
 */
uint8_t bh1750_power_down(void)
{
    return bh1750_send_cmd(BH1750_CMD_POWER_DOWN);
}

/**
 * @brief       复位 BH1750 传感器
 * @param       无
 * @retval      BH1750_EOK: 成功，其他: 失败
 */
uint8_t bh1750_reset(void)
{
    return bh1750_send_cmd(BH1750_CMD_RESET);
}

/**
 * @brief       设置 BH1750 测量模式
 * @param       mode: 测量模式
 * @retval      BH1750_EOK: 成功，BH1750_EINVAL: 无效模式，BH1750_ERROR: 通信失败
 */
uint8_t bh1750_set_mode(BH1750_Mode_t mode)
{
    uint8_t cmd = bh1750_mode_to_cmd(mode);
    if (cmd == 0) 
    {
        return BH1750_EINVAL;
    }

    if (bh1750_send_cmd(cmd) != BH1750_EOK) 
    {
        return BH1750_ERROR;
    }

    g_bh1750_mode = mode;
    return BH1750_EOK;
}

/**
 * @brief       读取光照强度（lux）
 * @param       lux: 输出光照强度值（lux）
 * @retval      BH1750_EOK: 成功，其他: 失败
 */
uint8_t bh1750_read_lux(float *lux)
{
    uint16_t raw;
    uint16_t wait_ms;

    if (g_bh1750_mode == BH1750_MODE_ONE_H_RES  ||
        g_bh1750_mode == BH1750_MODE_ONE_H_RES2 ||
        g_bh1750_mode == BH1750_MODE_ONE_L_RES)
    {
        wait_ms = bh1750_get_meas_time(g_bh1750_mode);
        if (wait_ms > 0) 
        {
            delay_ms(wait_ms);
        }
    }

    if (bh1750_read_raw(&raw) != BH1750_EOK) 
    {
        return BH1750_ERROR;
    }

    *lux = bh1750_raw_to_lux(raw, g_bh1750_mode);
    return BH1750_EOK;
}

/**
 * @brief       单次测量光照强度（自动设置为单次高分辨率模式）
 * @param       lux: 输出光照强度值（lux）
 * @retval      BH1750_EOK: 成功，其他: 失败
 */
uint8_t bh1750_measure(float *lux)
{
    uint8_t ret = bh1750_set_mode(BH1750_MODE_ONE_H_RES);
    if (ret != BH1750_EOK) 
    {
        return ret;
    }
    return bh1750_read_lux(lux);
}
