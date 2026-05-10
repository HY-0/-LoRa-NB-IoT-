#include "sh393.h"

/**
 * @brief       SH393 土壤湿度传感器初始化
 * @param       无
 * @retval      SH393_EOK   : 初始化成功
 *              SH393_ERROR : 初始化失败
 */
uint8_t sh393_init(void)
{
    if (sh393_adc_init() != SH393_ADC_EOK)
        return SH393_ERROR;
    return SH393_EOK;
}

/**
 * @brief       获取土壤湿度百分比（基于线性标定，带超时保护）
 * @param       percent: 存放湿度百分比（0.0 ~ 100.0）
 * @retval      SH393_EOK   : 成功
 *              SH393_ERROR : 参数错误或ADC读取失败
 *
 * @note        转换公式：
 *               假设传感器在干燥空气中输出约 V_MIN (V)，完全浸水输出 V_MAX (V)，
 *               ADC 参考电压 3.3V，12bit 精度。
 *               V_MIN 和 V_MAX 应根据实际传感器在干燥/湿润条件下的实测电压调整。
 */
uint8_t sh393_measure(float *percent)
{
    uint16_t adc_val;
    
    if (percent == NULL)
        return SH393_ERROR;
    
   
    if (sh393_adc_read(&adc_val) != SH393_ADC_EOK)
            return SH393_ERROR;
    
    /* 核心公式：湿度(%) = 100 - (ADC平均值 / 40.96)
       推导：ADC=4095 → 0%，ADC=0 → 100%
       注意：40.96 = 4095 / 100，但直接用 40.96 更准确 */
    *percent = 100.0f - (adc_val / 40.96f);
    
    /* 边界裁剪 */
    if (*percent < 0.0f) *percent = 0.0f;
    if (*percent > 100.0f) *percent = 100.0f;
    
    return SH393_EOK;
}
