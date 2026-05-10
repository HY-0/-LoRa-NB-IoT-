#include "display.h"

/* 缓存上一次显示的内容，用于判断是否需要刷新 */
static sensor_data_t last_data = {0};
static comm_status_t last_comm = {0};

static display_page_t last_page = PAGE_ENV;
static display_page_t current_page = PAGE_COMM;

static uint8_t first_call = 1;

/**
 * @brief   显示初始化
 */
void display_init(void)
{
    oled_clear(&oled);
    oled_set_pen(&oled, PEN_COLOR_WHITE, 1);
    oled_set_brush(&oled, PEN_COLOR_TRANSPARENT);
}

/**
 * @brief   多页面显示（传感器数据 + 通信状态），按键切换
 * @param   data       : 传感器数据指针（可为 NULL）
 * @param   comm       : 通信状态指针（可为 NULL）
 * @param   key_value  : 按键值
 */
void display_sensor_data(const sensor_data_t *data, const comm_status_t *comm,
                         uint8_t key_value)
{
    /* ---- 按键切换页面（无论是否刷新都会执行） ---- */
    if (key_value == KEY1_PRES) 
    {
        current_page = (display_page_t)((current_page + 1) % 3);
    }

    /* ---- 检查是否需要真正刷新屏幕 ---- */
    uint8_t need_refresh = 0;

    if (first_call) 
    {
        need_refresh = 1;
        first_call = 0;
    } 
    else 
    {
        if (current_page != last_page) 
        {
            need_refresh = 1;
        }
        else if (data != NULL) 
        {
            switch (current_page) 
            {
                case PAGE_ENV:
                    if (ABS_DIFF(data->air_temp,    last_data.air_temp)     >= TEMP_THRESHOLD     ||
                        ABS_DIFF(data->air_humi,    last_data.air_humi)     >= AIR_HUMI_THRESHOLD ||
                        ABS_DIFF(data->light, last_data.light)  >= LIGHT_THRESHOLD)
                        need_refresh = 1;
                    break;
                case PAGE_SOIL:
                    if (ABS_DIFF(data->ph,              last_data.ph)               >= PH_THRESHOLD  ||
                        ABS_DIFF(data->co2,             last_data.co2)              >= CO2_THRESHOLD ||
                        ABS_DIFF(data->soil_humi,   last_data.soil_humi)     >= SOIL_HUMI_THRESHOLD)
                        need_refresh = 1;
                    break;
                case PAGE_COMM:
                    // 通信状态变化在下面统一处理
                    break;
            }
        }
        /* 通信状态变化立即刷新（布尔量，无阈值） */
        if (!need_refresh && current_page == PAGE_COMM && comm != NULL) 
        {
            if (comm->lora_ready != last_comm.lora_ready ||
                comm->nbiot_connected != last_comm.nbiot_connected)
                need_refresh = 1;
        }
    }

    /* 如果不需要刷新，直接退出，不操作屏幕 */
    if (!need_refresh) 
    {
        return;
    }

    /* 保存当前显示内容 */
    if (data) last_data = *data;
    if (comm) last_comm = *comm;
    last_page = current_page;

    /* 快速刷新屏幕 */
    display_init();

    switch (current_page) 
    {
        case PAGE_ENV:
            oled_set_cursor(&oled, 25, 10);
            oled_draw_string(&oled, " Ambient   [1]");
            oled_set_cursor(&oled, 0, 20);
            oled_draw_string(&oled, "==================");
            if (data != NULL) 
            {
                oled_set_cursor(&oled, 0, 32);
                oled_printf(&oled, "Temp: %.1f C", data->air_temp);
                oled_set_cursor(&oled, 0, 44);
                oled_printf(&oled, "Light: %.1f lux", data->light);
                oled_set_cursor(&oled, 0, 58);
                oled_printf(&oled, "AirHumi: %.1f %%RH", data->air_humi);
            } 
            else 
            {
                oled_set_cursor(&oled, 0, 32);
                oled_draw_string(&oled, "Temp: N/A");
                oled_set_cursor(&oled, 0, 44);
                oled_draw_string(&oled, "Light: N/A");
                oled_set_cursor(&oled, 0, 58);
                oled_draw_string(&oled, "Air Huim: N/A");
            }
            break;

        case PAGE_SOIL:
            oled_set_cursor(&oled, 15, 10);
            oled_draw_string(&oled, " Subsurface  [2]");
            oled_set_cursor(&oled, 0, 20);
            oled_draw_string(&oled, "==================");
            if (data != NULL) 
            {
                oled_set_cursor(&oled, 0, 32);
                oled_printf(&oled, "PH: %.1f", data->ph);
                oled_set_cursor(&oled, 0, 44);
                oled_printf(&oled, "CO2: %hu ppm", data->co2);
                oled_set_cursor(&oled, 0, 58);
                oled_printf(&oled, "SoilHumi: %.1f %%", data->soil_humi);
            } 
            else 
            {
                oled_set_cursor(&oled, 0, 32);
                oled_draw_string(&oled, "PH  : N/A");
                oled_set_cursor(&oled, 0, 44);
                oled_draw_string(&oled, "CO2 : N/A");
                oled_set_cursor(&oled, 0, 58);
                oled_draw_string(&oled, "Soil: N/A");
            }
            break;

        case PAGE_COMM:
            oled_set_cursor(&oled, 10, 10);
            oled_draw_string(&oled, " Connectivity [3]");
            oled_set_cursor(&oled, 0, 20);
            oled_draw_string(&oled, "==================");
            if (comm != NULL) 
            {
                oled_set_cursor(&oled, 0, 30);
                oled_printf(&oled, "LoRa: %s", comm->lora_ready ? "Ready" : "Busy");
                oled_set_cursor(&oled, 0, 46);
                oled_printf(&oled, "NB-IoT: %s", comm->nbiot_connected ? "Connected" : "Disconn");
                oled_set_cursor(&oled, 0, 62);
                oled_printf(&oled, "NB CSQ: %d", comm->nbiot_rssi);
            } 
            else 
            {
                oled_set_cursor(&oled, 0, 36);
                oled_draw_string(&oled, "LoRa: ?");
                oled_set_cursor(&oled, 0, 58);
                oled_draw_string(&oled, "NB-IoT: ?");
            }
            break;

        default:
            break;
    }

    /* 清空缓存 */
    oled_send_buffer(&oled);
}

void display_error(const char *msg)
{
    oled_clear(&oled);
    oled_set_cursor(&oled, 0, 40);
    oled_draw_string(&oled, msg);
    oled_send_buffer(&oled);
}

void display_clear(void)
{
    oled_clear(&oled);
}
