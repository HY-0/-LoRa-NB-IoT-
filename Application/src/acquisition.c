#include "acquisition.h"
#include "display.h"

/* 静态全局变量：缓存最新传感器数据 */
static sensor_data_t g_sensor_data = {0};

/* ─── 内部辅助：各传感器读取封装 ─── */
static uint8_t read_air_humiture(float *temp, float *humi)
{
    return sht30_measure(temp, humi);
}

static uint8_t read_soil_humidity(float *percent)
{
    return sh393_measure(percent);
}

static uint8_t read_light(float *lux)
{
    return bh1750_measure(lux);
}

static uint8_t read_ph(float *ph)
{
    return ph4052_measure(ph);
}

static uint8_t read_co2(uint16_t *ppm)
{
    return jw01_measure(ppm);
}


/* ─── 启动时带显示的初始化流程 ─── */
static void sensor_startup_common(const char *name, uint8_t (*init_func)(void))
{
    oled_clear(&oled);
    oled_set_cursor(&oled, 0, 25);
    oled_printf(&oled, "*******************");
    oled_set_cursor(&oled, 0, 40);
    oled_printf(&oled, "<<<<  %s  >>>>", name);
    oled_set_cursor(&oled, 0, 55);
    oled_printf(&oled, "*******************");
    oled_send_buffer(&oled);           // 立即刷新
    delay_ms(500);                     // 延时仍然保留，为了让用户看到启动信息

    oled_clear(&oled);

    uint8_t ret = init_func();
    if (ret == 0)
    {
        oled_set_cursor(&oled, 30, 40);
        oled_printf(&oled, "%s OK!", name);
    }
    else
    {
        oled_set_cursor(&oled, 30, 40);
        oled_printf(&oled, "%s Err!", name);
        while(1);  /* 初始化失败则死循环，方便调试 */
    }
    oled_send_buffer(&oled);
    delay_ms(1000);
}

/* ─── 对外接口：初始化所有传感器 ─── */
void acquisition_init(void)
{
    sensor_startup_common("BH1750", bh1750_init);
    sensor_startup_common("SHT30",  sht30_init);
    sensor_startup_common("SH393",  sh393_init);
    sensor_startup_common("JW01", jw01_init);
    sensor_startup_common("PH4052", ph4052_init);
    oled_clear(&oled);
}

/* ─── 对外接口：采集一次数据（更新缓存）─── */
uint8_t acquisition_poll(void)
{
    uint8_t bh1750_status = read_light(&g_sensor_data.light);
    uint8_t sht30_status  = read_air_humiture(&g_sensor_data.air_temp,
                                              &g_sensor_data.air_humi);
    uint8_t sh393_status  = read_soil_humidity(&g_sensor_data.soil_humi);
    uint8_t jw01_status   = read_co2(&g_sensor_data.co2);
    uint8_t ph4052_status = read_ph(&g_sensor_data.ph);

    // 这里暂时改为了逻辑或，方便调试
    /* 五个传感器均成功才认为数据有效 */
    if (bh1750_status == BH1750_EOK ||
        sht30_status  == SHT30_EOK  ||
        sh393_status  == SH393_EOK  ||
        jw01_status   == JW01_EOK   ||
        ph4052_status == PH4052_EOK)
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
