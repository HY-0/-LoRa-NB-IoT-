#include "sht30.h"

/* 当前使用的测量命令（默认高重复性） */
static uint16_t g_sht30_meas_cmd = SHT30_CMD_MEAS_HIGH_REP;
static uint16_t g_sht30_meas_time = SHT30_MEAS_TIME_HIGH;

/* ==================== 内部辅助函数 ==================== */

/**
 * @brief       SHT30 CRC-8 校验
 * @param       data: 数据指针
 * @param       len:  数据长度
 * @retval      计算出的 CRC 值
 */
static uint8_t sht30_crc8(const uint8_t *data, uint8_t len)
{
    uint8_t crc = 0xFF;
    uint8_t byteCtr, bit;

    for (byteCtr = 0; byteCtr < len; byteCtr++) {
        crc ^= data[byteCtr];
        for (bit = 8; bit > 0; --bit) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ SHT30_CRC_POLYNOMIAL;
            } else {
                crc = (crc << 1);
            }
        }
    }
    return crc;
}

/* ==================== 用户 API ==================== */

/**
 * @brief       初始化 SHT30 传感器
 * @param       无
 * @retval      SHT30_EOK: 成功，SHT30_ERROR: 失败
 */
uint8_t sht30_init(void)
{
    /* 初始化 I2C 硬件 */
    sht30_i2c_init(SHT30_I2C_SPEED);

    /* 等待传感器上电稳定 */
    delay_ms(10);

    /* 可选：发送软复位 */
    // sht30_soft_reset();

    return SHT30_EOK;
}

/**
 * @brief       软件复位 SHT30
 * @param       无
 * @retval      SHT30_EOK: 成功，SHT30_ERROR: 失败
 */
uint8_t sht30_soft_reset(void)
{
    if (sht30_i2c_write_cmd(SHT30_CMD_SOFT_RESET) != SHT30_EOK) {
        return SHT30_ERROR;
    }
    delay_ms(1);  /* 复位后等待至少 1ms */
    return SHT30_EOK;
}

/**
 * @brief       读取原始温湿度数据（未经 CRC 校验）
 * @param       temp_raw: 输出温度原始值（16 位有符号）
 * @param       humi_raw: 输出湿度原始值（16 位无符号）
 * @retval      SHT30_EOK: 成功，SHT30_ERROR: I2C 通信失败，SHT30_ECRC: CRC 校验失败
 */
uint8_t sht30_read_humiture_raw(int16_t *temp_raw, uint16_t *humi_raw)
{
    uint8_t buf[6];
    uint8_t crc;

    /* 1. 发送测量命令 */
    if (sht30_i2c_write_cmd(g_sht30_meas_cmd) != SHT30_EOK) {
        return SHT30_ERROR;
    }

    /* 2. 等待测量完成 */
    delay_ms(g_sht30_meas_time);

    /* 3. 读取 6 字节数据 */
    if (sht30_i2c_read_raw(buf, 6) != SHT30_EOK) {
        return SHT30_ERROR;
    }

    /* 4. CRC 校验 */
    crc = sht30_crc8(&buf[0], 2);
    if (crc != buf[2]) {
        return SHT30_ECRC;
    }
    crc = sht30_crc8(&buf[3], 2);
    if (crc != buf[5]) {
        return SHT30_ECRC;
    }

    /* 5. 提取原始值 */
    *temp_raw = (int16_t)((buf[0] << 8) | buf[1]);
    *humi_raw = (uint16_t)((buf[3] << 8) | buf[4]);

    return SHT30_EOK;
}

/**
 * @brief       读取温湿度（转换为实际物理值）
 * @param       temp_celsius: 输出温度（摄氏度）
 * @param       humi_percent: 输出相对湿度（%RH）
 * @retval      SHT30_EOK: 成功，其他: 失败
 */
uint8_t sht30_measure(float *temp_celsius, float *humi_percent)
{
    int16_t temp_raw;
    uint16_t humi_raw;
    uint8_t ret;

    ret = sht30_read_humiture_raw(&temp_raw, &humi_raw);
    if (ret != SHT30_EOK) {
        return ret;
    }

    /* 温度转换公式：T[°C] = -45 + 175 * (ST / (2^16 - 1)) */
    *temp_celsius = -45.0f + 175.0f * ((float)temp_raw / 65535.0f);

    /* 湿度转换公式：RH[%] = 100 * (SRH / (2^16 - 1)) */
    *humi_percent = 100.0f * ((float)humi_raw / 65535.0f);

    return SHT30_EOK;
}
