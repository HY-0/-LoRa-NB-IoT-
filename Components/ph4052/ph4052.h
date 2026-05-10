#ifndef __PH4052_H
#define __PH4052_H

#include "stm32f10x.h"
#include "ph4052_adc.h"

/* ────────── 错误码 ────────── */
#define PH4052_EOK                0
#define PH4052_ERROR              1

/* ────────── 校准系数（25°C 基准） ────────── */
#define PH_SLOPE      (-5.7541f)
#define PH_INTERCEPT   16.654f

uint8_t ph4052_init(void);
uint8_t ph4052_measure(float *ph_out);


#endif
