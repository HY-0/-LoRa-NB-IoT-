#include "sh393_adc.h"

static ADC_TypeDef *g_adc_handle = SH393_ADC_INTERFACE;

/**
 * @brief       SH393 ADC 初始化（GPIO、时钟、校准，带超时保护）
 * @param       无
 * @retval      SH393_ADC_EOK   : 初始化成功
 *              SH393_ADC_ERROR : 初始化失败
 */
uint8_t sh393_adc_init(void)
{
    GPIO_InitTypeDef gpio;
    ADC_InitTypeDef adc;
    uint32_t timeout;

    /* ─── 1. GPIO 初始化（模拟输入）─── */
    SH393_ADC_GPIO_CLK_ENABLE();
    gpio.GPIO_Pin   = SH393_ADC_GPIO_PIN;
    gpio.GPIO_Mode  = GPIO_Mode_AIN;
    GPIO_Init(SH393_ADC_GPIO_PORT, &gpio);

    /* ─── 2. 使能 ADC 时钟 ─── */
    SH393_ADC_CLK_ENABLE();

    /* ─── 3. ADC 时钟分频 ─── */
    RCC_ADCCLKConfig(RCC_PCLK2_Div6);

    /* ─── 4. ADC 基础配置 ─── */
    adc.ADC_Mode               = ADC_Mode_Independent;
    adc.ADC_DataAlign          = ADC_DataAlign_Right;
    adc.ADC_ExternalTrigConv   = ADC_ExternalTrigConv_None;
    adc.ADC_ContinuousConvMode = DISABLE;
    adc.ADC_ScanConvMode       = DISABLE;
    adc.ADC_NbrOfChannel       = 1;
    ADC_Init(g_adc_handle, &adc);

    /* ─── 5. 使能 ADC 并校准（带超时）─── */
    ADC_Cmd(g_adc_handle, ENABLE);

    ADC_ResetCalibration(g_adc_handle);
    timeout = SH393_ADC_TIMEOUT;
    while (ADC_GetResetCalibrationStatus(g_adc_handle) == SET)
    {
        if (--timeout == 0) return SH393_ADC_ERROR;
        delay_us(10);
    }

    ADC_StartCalibration(g_adc_handle);
    timeout = SH393_ADC_TIMEOUT;
    while (ADC_GetCalibrationStatus(g_adc_handle) == SET)
    {
        if (--timeout == 0) return SH393_ADC_ERROR;
        delay_us(10);
    }

    return SH393_ADC_EOK;
}

/**
 * @brief       读取 SH393 ADC 原始值（单次转换，带超时保护）
 * @param       value: 存放 ADC 转换结果的指针
 * @retval      SH393_ADC_EOK   : 读取成功
 *              SH393_ADC_ERROR : 读取失败
 */
uint8_t sh393_adc_read(uint16_t *value)
{
    uint32_t timeout;

    if (value == NULL) return SH393_ADC_ERROR;

    ADC_RegularChannelConfig(g_adc_handle, SH393_ADC_CHANNEL, 1, SH393_ADC_SAMPLE_TIME);
    ADC_SoftwareStartConvCmd(g_adc_handle, ENABLE);

    timeout = SH393_ADC_TIMEOUT;
    while (ADC_GetFlagStatus(g_adc_handle, ADC_FLAG_EOC) == RESET)
    {
        if (--timeout == 0)
        {
            *value = 0;
            return SH393_ADC_ERROR;
        }
        delay_us(10);
    }

    *value = ADC_GetConversionValue(g_adc_handle);
    return SH393_ADC_EOK;
}
