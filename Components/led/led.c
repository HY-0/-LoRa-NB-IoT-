#include "led.h"

/**
 * @brief       LED初始化
 * @param       无
 * @retval      无
 */
void led_init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

    GPIO_InitTypeDef gpio;

    gpio.GPIO_Pin = GPIO_Pin_13;
    gpio.GPIO_Speed = GPIO_Speed_2MHz;
    gpio.GPIO_Mode = GPIO_Mode_Out_OD;
    GPIO_Init(GPIOC, &gpio);
    GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_SET);
}

/**
 * @brief       LED0闪烁（无限循环）
 * @param       无
 * @retval      无
 */
void led0_toggle(void)
{
    while(1)
    {
        GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_RESET);
        delay_ms(200);
        GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_SET);
        delay_ms(200);
    }
}
