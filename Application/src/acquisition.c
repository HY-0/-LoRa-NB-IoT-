#include "acquisition.h"
#include "display.h"

/* 静态全局变量：缓存最新传感器数据 */
static sensor_data_t g_sensor_data = {0};

/* ─── 内部辅助：各传感器读取封装 ─── */
static uint8_t read_light(float *lux)
{
    return bh1750_measure(lux);
}

static uint8_t read_humiture(float *temp, float *humi)
{
    return sht30_read_humiture(temp, humi);
}

static uint8_t read_soil_humidity(float *percent)
{
    return sh393_get_humidity(percent);
}

/* ─── 启动时带显示的初始化流程 ─── */
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
        while(1);  /* 初始化失败则死循环，方便调试 */
    }
    delay_ms(1000);
}

/* ─── 对外接口：初始化所有传感器 ─── */
void acquisition_init(void)
{
    sensor_startup_common("BH1750", bh1750_init);
    sensor_startup_common("SHT30",  sht30_init);
    sensor_startup_common("SH393",  sh393_init);
    oled_clear(&oled);
}

/* ─── 对外接口：采集一次数据（更新缓存）─── */
uint8_t acquisition_poll(void)
{
    uint8_t bh1750_status = read_light(&g_sensor_data.light_intensity);
    uint8_t sht30_status  = read_humiture(&g_sensor_data.temperature,
                                      &g_sensor_data.air_humidity);
    uint8_t sh393_status  = read_soil_humidity(&g_sensor_data.soil_humidity);

    /* 三个传感器均成功才认为数据有效 */
    if (bh1750_status == BH1750_EOK &&
        sht30_status  == SHT30_EOK  &&
        sh393_status  == SH393_EOK)
    {
        g_sensor_data.timestamp  = get_ms();   /* 记录时间戳 */
        g_sensor_data.data_valid = 1;
    }
    else
    {
        g_sensor_data.data_valid = 0;
        return ACQ_ERROR;
    }

    return ACQ_OK;
}

/* ─── 对外接口：读取最新有效数据 ─── */
uint8_t acquisition_read(sensor_data_t *data)
{
    if (data == NULL)
        return ACQ_ERROR;

    if (!g_sensor_data.data_valid)
        return ACQ_NO_DATA;

    *data = g_sensor_data;
    return ACQ_OK;
}
