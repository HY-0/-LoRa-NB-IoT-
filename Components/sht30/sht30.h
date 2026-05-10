/**
 * @file    sht30.h
 * @brief   SHT30温湿度传感器驱动头文件
 * @author  Lora项目组
 * @date    2024-01-15
 * @version 1.0.0
 */

#ifndef __SHT30_H
#define __SHT30_H

#include "sht30_i2c.h"
#include "delay.h"

/* ==================== SHT30 命令定义 ==================== */

#define SHT30_CMD_MEAS_HIGH_REP       0x2C06  /* 高重复性测量（推荐） */
#define SHT30_CMD_MEAS_MED_REP        0x2C0D  /* 中重复性测量 */
#define SHT30_CMD_MEAS_LOW_REP        0x2C10  /* 低重复性测量 */
#define SHT30_CMD_SOFT_RESET          0x30A2  /* 软复位 */
#define SHT30_CMD_HEATER_ENABLE       0x306D  /* 使能加热器 */
#define SHT30_CMD_HEATER_DISABLE      0x3066  /* 关闭加热器 */
#define SHT30_CMD_READ_STATUS         0xF32D  /* 读状态寄存器 */

/* 测量等待时间（毫秒） */
#define SHT30_MEAS_TIME_HIGH          50      /* 高重复性典型时间 */
#define SHT30_MEAS_TIME_MED           20      /* 中重复性 */
#define SHT30_MEAS_TIME_LOW           10      /* 低重复性 */

/* CRC 多项式 */
#define SHT30_CRC_POLYNOMIAL          0x31    /* X^8 + X^5 + X^4 + 1 */

/* 错误码扩展 */
#define SHT30_ETIMEOUT                2       /* 超时错误 */
#define SHT30_ECRC                    3       /* CRC校验错误 */

/* ==================== 高层 API ==================== */

/**
 * @brief   初始化SHT30传感器
 * @param   无
 * @retval  SHT30_EOK   - 初始化成功
 *          SHT30_ERROR - 初始化失败
 */
uint8_t sht30_init(void);

/**
 * @brief   软件复位SHT30传感器
 * @param   无
 * @retval  SHT30_EOK   - 复位成功
 *          SHT30_ERROR - 复位失败
 */
uint8_t sht30_soft_reset(void);

/**
 * @brief   读取温湿度原始数据（16位格式）
 * @param   temp_raw - 温度原始值指针
 * @param   humi_raw - 湿度原始值指针
 * @retval  SHT30_EOK      - 读取成功
 *          SHT30_ECRC     - CRC校验失败
 *          SHT30_ETIMEOUT - 读取超时
 */
uint8_t sht30_read_humiture_raw(int16_t *temp_raw, uint16_t *humi_raw);


/**
 * @brief   读取温湿度数据（浮点格式）
 * @param   temp_celsius   - 温度值指针（单位：摄氏度）
 * @param   humi_percent   - 湿度值指针（单位：百分比）
 * @retval  SHT30_EOK      - 读取成功
 *          SHT30_ECRC     - CRC校验失败
 *          SHT30_ETIMEOUT - 读取超时
 */
uint8_t sht30_measure(float *temp_celsius, float *humi_percent);

#endif /* __SHT30_H */
