#ifndef _ADCX_H_
#define _ADCX_H_

#include "stm32f10x.h"                  // Device header


uint16_t adc_get_value(ADC_TypeDef* ADCx, uint8_t ADC_Channel,uint8_t ADC_SampleTime);

#endif
