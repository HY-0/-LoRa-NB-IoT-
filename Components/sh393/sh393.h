#ifndef __SH393_H
#define __SH393_H

#include "stm32f10x.h"
#include "sh393_adc.h"    /* 底层 ADC 驱动 */

/* ────────── 传感器专属错误码 ────────── */
#define SH393_EOK                0
#define SH393_ERROR              1

/* ────────── 函数声明 ────────── */
uint8_t sh393_init(void);                        /* 初始化传感器（调用 ADC 初始化） */
uint8_t sh393_measure(float *percent);           /* 获取土壤湿度百分比（0.0~100.0） */

#endif /* __SH393_H */
