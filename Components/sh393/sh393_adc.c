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

    /* ─── 1. GPIO 初始化（模拟输入）─── */
    SH393_ADC_GPIO_CLK_ENABLE();
    gpio.GPIO_Pin   = SH393_ADC_GPIO_PIN;
    gpio.GPIO_Mode  = GPIO_Mode_AIN;
    GPIO_Init(SH393_ADC_GPIO_PORT, &gpio);

        /* 1. 初始化 ADC 外设（如果尚未初始化） */
    if (adcx_init(g_adc_handle) != ADC_EOK)   // SH393_ADC_INTERFACE 定义为 ADC1
        return SH393_ADC_ERROR;

    return SH393_ADC_EOK;
}

/**
 * @brief       读取 SH393 ADC 原始值（中值滤波）
 * @param       value: 存放 ADC 转换结果的指针
 * @retval      SH393_ADC_EOK   : 读取成功
 *              SH393_ADC_ERROR : 读取失败
 */
uint8_t sh393_adc_read(uint16_t *value)
{
    uint16_t buf[8];
    uint16_t tmp;
    uint8_t  round, cmp;

    if (value == NULL) return SH393_ADC_ERROR;

    /* 连续采集8次 */
    for (round = 0; round < 8; round++)
    {
        uint8_t ret = adcx_get_value(g_adc_handle, SH393_ADC_CHANNEL,
                                     ADC_SampleTime_239Cycles5, &buf[round]);
        if (ret != ADC_EOK) return SH393_ADC_ERROR;
    }

    /* 排序 */
    for (round = 0; round < 7; round++)
    {
        for (cmp = round + 1; cmp < 8; cmp++)
        {
            if (buf[round] > buf[cmp])
            {
                tmp        = buf[round];
                buf[round] = buf[cmp];
                buf[cmp]  = tmp;
            }
        }
    }

    /* 去掉头尾两个极值，中间四个取平均 */
    *value = (buf[2] + buf[3] + buf[4] + buf[5]) / 4;

    return SH393_ADC_EOK;
}
