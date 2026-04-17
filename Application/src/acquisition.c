#include "acquisition.h"
#include "display.h"

/* 静态全局变量：缓存最新传感器数据 */
static sensor_data_t g_sensor_data = {0};

/* ---------- 内部辅助函数 ---------- */
static uint8_t read_light(float *lux)
{
    return bh1750_measure(lux); 
}

static uint8_t read_humiture(float *temp, float *humi)
{
    return sht30_read_humiture(temp, humi);
}

/* ---------- 启动时带显示的初始化 ---------- */
static void sensor_startup_common(const char *name, uint8_t (*init_func)(void))
{
    oled_clear(&oled);
    oled_show(&oled, 0, 25, 0,   "*******************");
    oled_show(&oled, 0, 40, 0,   "<<<<  %s  >>>>", name);
    oled_show(&oled, 0, 55, 500, "*******************");
    oled_clear(&oled);

    uint8_t ret = init_func();
    
    if (ret == 0) 
    {
        oled_clear(&oled);
        oled_show(&oled, 30, 40, 0, "%s OK!", name);
    } 
    else 
    {
        oled_clear(&oled);
        oled_show(&oled, 30, 40, 0, "%s Err!", name);
        while(1);
    }
    delay_ms(1000);
}

/* ---------- 对外接口实现 ---------- */
void acquisition_init(void)
{
    sensor_startup_common("BH1750", bh1750_init);
    sensor_startup_common("SHT30", sht30_init);
    oled_clear(&oled);
}

uint8_t acquisition_poll(void)
{
    uint8_t light_ret = read_light(&g_sensor_data.light_intensity);
    uint8_t humi_ret  = read_humiture(&g_sensor_data.temperature, &g_sensor_data.humidity);

    if (light_ret == BH1750_EOK && humi_ret == SHT30_EOK) 
    {
        g_sensor_data.timestamp = get_ms();
        g_sensor_data.data_valid = 1;
    } 
    else 
    {
        g_sensor_data.data_valid = 0;
        return ACQ_ERROR;
    }

    return ACQ_OK;
}

uint8_t acquisition_read(sensor_data_t *data)
{
    if (!g_sensor_data.data_valid) 
    {
        return ACQ_NO_DATA;
    }

    *data = g_sensor_data;

    return ACQ_OK;
}

