#include "key.h"

/**
 * @brief       按键初始化函数
 * @param       无
 * @retval      无
 */
static void key_hw_init(void)
{
    GPIO_InitTypeDef gpio_init_struct;
    KEY0_GPIO_CLK_ENABLE();                                 /* KEY0时钟使能 */
    KEY1_GPIO_CLK_ENABLE();                                 /* KEY1时钟使能 */
    WKUP_GPIO_CLK_ENABLE();                                 /* WKUP时钟使能 */

    gpio_init_struct.GPIO_Pin = KEY0_GPIO_PIN;              /* KEY0引脚 */
    gpio_init_struct.GPIO_Mode = GPIO_Mode_IPU;             /* 输入上拉 */
    gpio_init_struct.GPIO_Speed = GPIO_Speed_50MHz;         /* 可调高速 */
    GPIO_Init(KEY0_GPIO_PORT, &gpio_init_struct);           /* KEY0引脚模式设置,上拉输入 */

    gpio_init_struct.GPIO_Pin = KEY1_GPIO_PIN;              /* KEY1引脚 */
    gpio_init_struct.GPIO_Mode = GPIO_Mode_IPU;             /* 输入上拉 */
    gpio_init_struct.GPIO_Speed = GPIO_Speed_50MHz;         /* 可调高速 */
    GPIO_Init(KEY1_GPIO_PORT, &gpio_init_struct);           /* KEY1引脚模式设置,上拉输入 */

    gpio_init_struct.GPIO_Pin = WKUP_GPIO_PIN;              /* WKUP引脚 */
    gpio_init_struct.GPIO_Mode = GPIO_Mode_IPD;             /* 输入下拉 */                
    GPIO_Init(WKUP_GPIO_PORT, &gpio_init_struct);           /* WKUP引脚模式设置,下拉输入 */
}

void key_init(void)
{
    key_hw_init();
}

/**
 * @brief       按键扫描函数
 * @note        该函数有响应优先级(同时按下多个按键): WK_UP > KEY1 > KEY0!!
 * @param       mode:0 / 1, 具体含义如下:
 *   @arg       0,  不支持连续按(当按键按下不放时, 只有第一次调用会返回键值,
 *                  必须松开以后, 再次按下才会返回其他键值)
 *   @arg       1,  支持连续按(当按键按下不放时, 每次调用该函数都会返回键值)
 * @retval      键值, 定义如下:
 *              KEY0_PRES, 1, KEY0按下
 *              KEY1_PRES, 2, KEY1按下
 *              WKUP_PRES, 3, WKUP按下
 */
uint8_t key_scan(uint8_t mode)
{
    static uint8_t key_up = 1;  /* 按键按松开标志 */
    uint8_t keyval = 0;

    if(mode) key_up = 1;       /* 支持连按 */

    if(key_up && (KEY0 == Bit_RESET || KEY1 == Bit_RESET || WK_UP == Bit_SET))  /* 按键松开标志为1, 且有任意一个按键按下了 */
    {
        delay_ms(10);           /* 去抖动 */
        key_up = 0;

        if(KEY0 == Bit_RESET)  keyval = KEY0_PRES;

        if(KEY1 == Bit_RESET)  keyval = KEY1_PRES;

        if(WK_UP == Bit_SET) keyval = WKUP_PRES;
    }
    else if(KEY0 == Bit_SET && KEY1 == Bit_SET && WK_UP == Bit_RESET) /* 没有任何按键按下, 标记按键松开 */
    {
        key_up = 1;
    }

    return keyval;              /* 返回键值 */
}


