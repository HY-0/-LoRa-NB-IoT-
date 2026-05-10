#include "ph4052.h"

/**
 * @brief       PH4502 模块初始化（含 ADC + DS18B20）
 * @retval      PH4502_EOK   : 成功
 *              PH4502_ERROR : 失败
 */
uint8_t ph4052_init(void)
{
    if (ph4052_adc_init() != PH4052_ADC_EOK)     return PH4052_ERROR;

    return PH4052_EOK;
}

/**
 * @brief       单次 pH 测量（25°C 校准，不含温度补偿）
 * @param       ph_out : 出参，存放 pH 值（范围 0.00 ~ 14.00）
 * @retval      PH4052_EOK   : 测量成功
 *              PH4052_ERROR : 参数为空或 ADC 读取失败
 * 
 * @note        pH = PH_SLOPE * voltage + PH_INTERCEPT
 *              voltage = adc_val / 4095.0 * 3.3 (实际 3.3V 参考)
 *              暂时未引入温度补偿，适用于室温环境
 */
uint8_t ph4052_measure(float *ph_out)
{
    uint16_t adc_val;
    float    voltage;

    if (ph_out == NULL) return PH4052_ERROR;

    if (ph4052_adc_read(&adc_val) != PH4052_ADC_EOK)
        return PH4052_ERROR;

    voltage = (adc_val / 4096.0f) * 3.3f;

    /* 仅用 25°C 校准系数换算 */
    *ph_out = PH_SLOPE * voltage + PH_INTERCEPT;

    if (*ph_out > 14.0f) *ph_out = 14.0f;
    if (*ph_out <  0.0f) *ph_out =  0.0f;

    return PH4052_EOK;
}
