/**
 * @file    sht30_demo.c
 * @brief   SHT30温湿度传感器演示程序
 * @author  Lora项目组
 * @date    2024-01-15
 * @version 1.0.0
 */

#include "sht30_demo.h"

/**
 * @brief   SHT30演示入口函数
 * @param   无
 * @retval  无
 */
void sht30_demo_run(void)
{
    float temp, humi;
    uint8_t ret;
    /* char disp_buf[32]; */

    /* 初始化 OLED 画笔 */
    oled_set_pen(&oled, PEN_COLOR_WHITE, 1);
    oled_set_brush(&oled, PEN_COLOR_TRANSPARENT);

    /* 显示标题（仿 lora_demo 风格） */
    oled_clear(&oled);
    oled_show(&oled, 0, 25,   0, "*******************");
    oled_show(&oled, 0, 40,   0, "<<  SHT30 Demo  >>");
    oled_show(&oled, 0, 55, 500, "*******************");

    /* 初始化 SHT30 传感器 */
    ret = sht30_init();
    if (ret != SHT30_EOK)
    {
        oled_clear(&oled);
        oled_show(&oled, 30, 25, 500, "SHT30 Init Err!");
        while (1);
    }

    oled_clear(&oled);
    oled_show(&oled, 15, 40, 500, "SHT30 Init OK");
    delay_ms(1000);

    /* 主循环：周期性读取并显示温湿度 */
    while (1)
    {
        ret = sht30_read_humiture(&temp, &humi);
        if (ret == SHT30_EOK)
        {
            oled_clear(&oled);
            oled_show(&oled, 0, 20, 0, "T: %.2f C", temp);
            oled_show(&oled, 0, 35, 0, "H: %.2f %%", humi);
        }
        else if (ret == SHT30_ECRC)
        {
            oled_clear(&oled);
            oled_show(&oled, 0, 25, 0, "CRC Error!      ");
        }
        else
        {
            oled_clear(&oled);
            oled_show(&oled, 0, 25, 0, "Read Error!     ");
        }

        delay_ms(500);  /* 每 500ms 刷新一次 */
    }
}
