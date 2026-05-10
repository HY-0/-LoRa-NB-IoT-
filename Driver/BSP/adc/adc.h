#ifndef _ADCX_H_
#define _ADCX_H_

#include <stddef.h>
#include "stm32f10x.h"                  // Device header
#include "delay.h"

/* ────────── ADC 超时宏（单位：循环次数，每循环 delay_us(10)）────────── */
#define ADC_CALIB_TIMEOUT     10000   // 校准超时（约 100ms）
#define ADC_SAMPLE_TIMEOUT    1000    // 单次采样超时（约 10ms）

/* ────────── 错误码 ────────── */
#define ADC_EOK                0
#define ADC_ERROR              1

uint8_t adcx_init(ADC_TypeDef* ADCx);
uint8_t adcx_get_value(ADC_TypeDef* ADCx, uint8_t ADC_Channel, uint8_t ADC_SampleTime, uint16_t *value);

#endif
