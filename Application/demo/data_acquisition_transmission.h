/**
 * @file    data_acquisition_transmission.h
 * @brief   数据采集与传输集成模块头文件 - LoRa/NB-IoT农业监测系统
 * @author  Lora项目组
 * @date    2024-01-15
 * @version 1.0.0
 */

#ifndef __DATA_ACQUISITION_TRANSMISSION_H
#define __DATA_ACQUISITION_TRANSMISSION_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* 错误码定义 */
#define DATA_ACQ_OK         0x00    /* 数据采集成功 */
#define DATA_ACQ_ERROR      0x01    /* 数据采集失败 */

#define DATA_TX_OK          0x00    /* 数据传输成功 */
#define DATA_TX_ERROR       0x01    /* 数据传输失败 */
#define DATA_TX_NO_DATA     0x02    /* 无有效数据可传输 */

#define DATA_MODE_OK        0x00    /* 模式设置成功 */
#define DATA_MODE_ERROR     0x01    /* 模式设置失败 */

#define DATA_GET_OK         0x00    /* 数据获取成功 */
#define DATA_GET_ERROR      0x01    /* 数据获取失败 */

/* 传输模式定义 */
#define TRANSMISSION_MODE_LORA      0x00    /* LoRa传输模式 */
#define TRANSMISSION_MODE_NBIOT     0x01    /* NB-IoT传输模式 */

/* 传感器数据结构体 */
typedef struct {
    float temperature;     /* 温度 (°C) */
    float humidity;        /* 湿度 (%RH) */
    float light_intensity; /* 光照强度 (lux) */
    uint32_t timestamp;    /* 时间戳 */
    uint8_t data_valid;    /* 数据有效性标志 */
} sensor_data_t;

/* 函数声明 */

/**
 * @brief       传感器数据采集
 * @param       无
 * @retval      DATA_ACQ_OK    : 数据采集成功
 *              DATA_ACQ_ERROR : 数据采集失败
 */
uint8_t data_acquisition(void);

/**
 * @brief       数据显示到OLED
 * @param       无
 * @retval      无
 */
void data_display(void);

/**
 * @brief       数据打包为JSON格式
 * @param       buf: 输出缓冲区
 * @param       size: 缓冲区大小
 * @retval      打包后的数据长度
 */
uint16_t data_pack_json(char *buf, uint16_t size);

/**
 * @brief       数据传输
 * @param       无
 * @retval      DATA_TX_OK     : 数据传输成功
 *              DATA_TX_ERROR  : 数据传输失败
 *              DATA_TX_NO_DATA: 无有效数据可传输
 */
uint8_t data_transmission(void);

/**
 * @brief       设置传输模式
 * @param       mode: 传输模式
 *                 TRANSMISSION_MODE_LORA  - LoRa传输
 *                 TRANSMISSION_MODE_NBIOT - NB-IoT传输
 * @retval      DATA_MODE_OK   : 模式设置成功
 *              DATA_MODE_ERROR: 模式设置失败
 */
uint8_t set_transmission_mode(uint8_t mode);

/**
 * @brief       获取当前传输模式
 * @param       无
 * @retval      当前传输模式
 */
uint8_t get_transmission_mode(void);

/**
 * @brief       获取传感器数据
 * @param       data: 传感器数据结构体指针
 * @retval      DATA_GET_OK    : 获取成功
 *              DATA_GET_ERROR : 无有效数据
 */
uint8_t get_sensor_data(sensor_data_t *data);

/**
 * @brief       数据采集与传输主任务
 * @param       无
 * @retval      无
 */
void data_acquisition_transmission_task(void);

#ifdef __cplusplus
}
#endif

#endif /* __DATA_ACQUISITION_TRANSMISSION_H */
