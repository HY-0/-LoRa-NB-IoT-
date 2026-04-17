#include "bh1750_demo.h"

/**
 * @brief       BH1750 演示入口函数
 * @param       无
 * @retval      无
 */
void bh1750_demo_run(void)
{
    float lux;
    uint8_t ret;

    /* 初始化 OLED 画笔 */
    oled_set_pen(&oled, PEN_COLOR_WHITE, 1);
    oled_set_brush(&oled, PEN_COLOR_TRANSPARENT);

    /* 显示标题（仿 lora_demo 风格） */
    oled_clear(&oled);
    oled_show(&oled, 0, 25,   0, "*******************");
    oled_show(&oled, 0, 40,   0, "<< BH1750 Demo >>");
    oled_show(&oled, 0, 55, 500, "*******************");

    /* 初始化 BH1750 传感器 */
    ret = bh1750_init();
    if (ret != BH1750_EOK)
    {
        oled_clear(&oled);
        oled_show(&oled, 30, 25, 500, "BH1750 Err!");

        /* 可以加入 LED 闪烁提示 */
        led0_toggle();
        while (1);
    }

    oled_clear(&oled);
    oled_show(&oled, 30, 50, 500, "BH1750 OK!");
    delay_ms(1000);

    /* 主循环：周期性读取并显示光照值 */
    while (1)
    {
        ret = bh1750_measure(&lux);     /* 单次测量（自动等待完成） */
        if (ret == BH1750_EOK)
        {
            oled_clear(&oled);
            oled_show(&oled, 0, 25, 0, "Light: %.2f lux   ", lux);
        }
        else
        {
            oled_clear(&oled);
            oled_show(&oled, 0, 25, 0, "Read Error!    ");
        }

        delay_ms(500);   /* 每 500ms 刷新一次 */
    }
}
