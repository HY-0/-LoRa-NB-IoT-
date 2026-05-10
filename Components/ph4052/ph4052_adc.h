#ifndef __PH4052_ADC_H
#define __PH4052_ADC_H

#include <stddef.h>
#include "stm32f10x.h"
#include "delay.h"
#include "adc.h"

/* ────────── 硬件引脚与 ADC 定义 ────────── */
#define PH4052_ADC_GPIO_PORT           GPIOA
#define PH4052_ADC_GPIO_PIN            GPIO_Pin_0
#define PH4052_ADC_GPIO_CLK_ENABLE()   do{ RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE); }while(0)

#define PH4052_ADC_INTERFACE           ADC1
#define PH4052_ADC_CHANNEL             ADC_Channel_0
#define PH4052_ADC_SAMPLE_TIME         ADC_SampleTime_239Cycles5
#define PH4052_ADC_CLK_ENABLE()        do{ RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE); }while(0)

/* 超时保护（ADC 校准或转换最大等待次数，单次循环约 10us） */
#define PH4052_ADC_TIMEOUT             10000

/* ────────── 错误码 ────────── */
#define PH4052_ADC_EOK                0
#define PH4052_ADC_ERROR              1

/* ────────── 函数声明 ────────── */
uint8_t ph4052_adc_init(void);                    /* 初始化 ADC 通道，成功返回 PH4052_ADC_EOK */
uint8_t ph4052_adc_read(uint16_t *value);        /* 读取一次 ADC 原始值，成功返回 PH4052_ADC_EOK */

#endif /* __PH4052_ADC_H */
