#include "display.h"

void display_init(void)
{
    oled_clear(&oled);
    oled_set_pen(&oled, PEN_COLOR_WHITE, 1);
    oled_set_brush(&oled, PEN_COLOR_TRANSPARENT); 
}

void display_sensor_data(const sensor_data_t *data, display_mode_t mode)
{
    display_init();

    /* 标题 */
    oled_show(&oled, 0, 10, 0, "   Agri Monitor");
    oled_show(&oled, 0, 20, 0, "==================");
    
    /* 温度 */
    oled_show(&oled, 0, 30, 0, "Temp: %.1f C", data->temperature);
    
    /* 湿度 */
    oled_show(&oled, 0, 45, 0, "Humi: %.1f %%RH", data->humidity);
    
    /* 光照 */
    oled_show(&oled, 0, 60, 0, "Light: %.1f lux", data->light_intensity);
    
    /* 传输模式 */
    if (mode == DISPLAY_MODE_LORA) 
    {
        oled_show(&oled, 0, 85, 0, "Mode: LoRa");
    } 
    else 
    {
        oled_show(&oled, 0, 85, 0, "Mode: NB-IoT");
    }
}

void display_error(const char *msg)
{
    display_clear();
    oled_show(&oled, 0, 40, 0, msg);
}

void display_clear(void)
{
    oled_clear(&oled);
}
