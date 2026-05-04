#ifndef __ACQUISITION_H
#define __ACQUISITION_H

#include "buzzer.h"
#include "bh1750.h"
#include "sht30.h"
#include "sh393.h"
#include "delay.h"
#include "oled.h"
#include "led.h"


/* 错误码定义 */
#define ACQ_OK       0                  /* 数据采集成功 */
#define ACQ_ERROR    1                  /* 数据采集失败 */
#define ACQ_NO_DATA  2                  /* 没有读取到数据 */

/* 传感器数据结构体 */

typedef struct {
    float temperature;          /* 温度 (°C) */
    float air_humidity;         /* 空气湿度 (%RH) */
    float soil_humidity;        /* 土壤湿度 (0.0 ~ 100.0%) */
    float light_intensity;      /* 光照强度 (lux) */
    float ph;                   /* 土壤/水体 pH 值 (0.00 ~ 14.00) */
    float co2;                  /* 二氧化碳浓度 (ppm) */
    
    uint32_t timestamp;    /* 时间戳 */
    uint8_t data_valid;    /* 数据有效性标志 */
} sensor_data_t;

/* 函数声明 */
void acquisition_init(void);
uint8_t acquisition_poll(void);
uint8_t acquisition_read(sensor_data_t *data);

#endif //  __ACQUISITION_H
