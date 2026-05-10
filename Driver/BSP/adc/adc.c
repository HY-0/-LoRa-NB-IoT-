#include "adc.h"


static uint8_t first_init = 1;

uint8_t adcx_init(ADC_TypeDef* ADCx)
{
    ADC_InitTypeDef adc;
    uint32_t timeout;

    if(!first_init) 
    {
        first_init = 0;
        return ADC_EOK;
    }

    /* 1. 参数检查 */
    if (ADCx == NULL) return ADC_ERROR;
    if (ADCx != ADC1 && ADCx != ADC2 && ADCx != ADC3) return ADC_ERROR;  // 可选

    /* 2. 根据 ADCx 使能对应时钟 */
    if (ADCx == ADC1) RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
    else if (ADCx == ADC2) RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC2, ENABLE);
    else if (ADCx == ADC3) RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC3, ENABLE);
    else return ADC_ERROR;

    /* 3. 配置 ADC 时钟分频（全局配置一次即可，多次调用无害） */
    RCC_ADCCLKConfig(RCC_PCLK2_Div6);

    /* 4. ADC 基础配置 */
    adc.ADC_Mode               = ADC_Mode_Independent;
    adc.ADC_DataAlign          = ADC_DataAlign_Right;
    adc.ADC_ExternalTrigConv   = ADC_ExternalTrigConv_None;
    adc.ADC_ContinuousConvMode = DISABLE;
    adc.ADC_ScanConvMode       = DISABLE;
    adc.ADC_NbrOfChannel       = 1;
    ADC_Init(ADCx, &adc);

    /* 5. 使能 ADC 并校准 */
    ADC_Cmd(ADCx, ENABLE);
    delay_us(20);   // 等待稳定

    ADC_ResetCalibration(ADCx);
    timeout = ADC_CALIB_TIMEOUT;
    while (ADC_GetResetCalibrationStatus(ADCx) == SET) {
        if (--timeout == 0) return ADC_ERROR;
        delay_us(10);
    }

    ADC_StartCalibration(ADCx);
    timeout = ADC_CALIB_TIMEOUT;
    while (ADC_GetCalibrationStatus(ADCx) == SET) {
        if (--timeout == 0) return ADC_ERROR;
        delay_us(10);
    }

    return ADC_EOK;
}

/**
 * @brief  ADC单次采集（带超时保护）
 * @param  ADCx           ADC外设，如ADC1
 * @param  ADC_Channel    通道号
 * @param  ADC_SampleTime 采样周期，pH推荐239.5周期
 * @param  value          出参，存放12位转换结果
 * @retval ADC_EOK        成功
 * @retval ADC_ERROR      超时或空指针
 */
uint8_t adcx_get_value(ADC_TypeDef* ADCx, uint8_t ADC_Channel, uint8_t ADC_SampleTime, uint16_t *value)
{
    uint32_t timeout;

    if (value == NULL) return ADC_ERROR;

    ADC_RegularChannelConfig(ADCx, ADC_Channel, 1, ADC_SampleTime);
    ADC_SoftwareStartConvCmd(ADCx, ENABLE);

    timeout = ADC_SAMPLE_TIMEOUT;
    while (ADC_GetFlagStatus(ADCx, ADC_FLAG_EOC) == RESET)
    {
        if (--timeout == 0)     // 超时退出，防止死锁
        {
            *value = 0;
            return ADC_ERROR;
        }
        delay_us(10);
    }

    *value = ADC_GetConversionValue(ADCx);
    return ADC_EOK;
}
