#ifndef __SH393_ADC_H
#define __SH393_ADC_H

#include <stddef.h>
#include "stm32f10x.h"
#include "delay.h"
#include "adc.h"

/* ────────── 硬件引脚与 ADC 定义 ────────── */
#define SH393_ADC_GPIO_PORT           GPIOA
#define SH393_ADC_GPIO_PIN            GPIO_Pin_1
#define SH393_ADC_GPIO_CLK_ENABLE()   do{ RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE); }while(0)

#define SH393_ADC_INTERFACE           ADC1
#define SH393_ADC_CHANNEL             ADC_Channel_1
#define SH393_ADC_SAMPLE_TIME         ADC_SampleTime_239Cycles5
#define SH393_ADC_CLK_ENABLE()        do{ RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE); }while(0)

/* ADC 硬件校准超时，约 100ms（仅用于上电初始化） */
#define  SH393_ADC_TIMEOUT  10000

/* ────────── 错误码 ────────── */
#define SH393_ADC_EOK                0
#define SH393_ADC_ERROR              1

/* ────────── 函数声明 ────────── */
uint8_t sh393_adc_init(void);                    /* 初始化 ADC 通道，成功返回 SH393_ADC_EOK */
uint8_t sh393_adc_read(uint16_t *value);        /* 读取一次 ADC 原始值，成功返回 SH393_ADC_EOK */

#endif /* __SH393_ADC_H */
