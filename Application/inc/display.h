#ifndef __DISPLAY_H
#define __DISPLAY_H

#include "transmission.h"
#include "acquisition.h"
#include "oled.h"
#include "key.h"

#define TEMP_THRESHOLD      0.5f   /* 温度变化超过0.5°C才刷新 */
#define AIR_HUMI_THRESHOLD  2.0f   /* 空气湿度变化超过2%RH才刷新 */
#define SOIL_HUMI_THRESHOLD 2.0f   /* 土壤湿度变化超过2%才刷新 */
#define LIGHT_THRESHOLD     50.0f  /* 光照变化超过50 lux才刷新 */
#define PH_THRESHOLD        0.1f   /* pH变化超过0.1才刷新 */
#define CO2_THRESHOLD       100.0f /* CO2变化超过100 ppm才刷新 */

/* 浮点绝对值宏 */
#define ABS_DIFF(a, b) ((a) > (b) ? ((a) - (b)) : ((b) - (a)))

/* ── 显示页面枚举 ── */
typedef enum {
    PAGE_ENV  = 0,   /* 环境数据 */
    PAGE_SOIL = 1,   /* 土壤与水质 */
    PAGE_COMM = 2    /* 通信状态 */
} display_page_t;

/* ── 函数声明 ── */
void display_init(void);
void display_sensor_data(const sensor_data_t *data, const comm_status_t *comm, uint8_t key_value);
void display_error(const char *msg);
void display_clear(void);

#endif
