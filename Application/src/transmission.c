#include "transmission.h"

/* ========== 模块内部状态 ========== */
static transmission_mode_t g_current_mode = TRANSMISSION_MODE_LORA;

/* ---------- 内部辅助函数 ---------- */
uint8_t lora_startup(uint32_t baudrate)
{
    uint8_t ret;

    /* 显示标题 */
    oled_clear(&oled);
    oled_set_pen(&oled, PEN_COLOR_WHITE, 1);         /* 设置白画笔 */
    oled_set_brush(&oled, PEN_COLOR_TRANSPARENT);    /* 透明画刷 */
    oled_show(&oled, 0, 25,   0, "*******************");
    oled_show(&oled, 0, 40,   0, "<<<<STM32-LoRa>>>>");
    oled_show(&oled, 0, 55, 500, "*******************");

	/* 1. 硬件初始化 */
    ret = lora_init(baudrate);
    if (ret != LORA_EOK)
    {
        oled_clear(&oled);
        oled_show(&oled, 30, 25, 500, "LoRa err!");

        led0_toggle();
    }

    /* 2. 进入配置模式 */
    lora_enter_config();
    
    /* 3. 逐一配置参数，累加错误码 */
    ret  = lora_addr_config(DEMO_ADDR);
    ret += lora_wlrate_channel_config(DEMO_WLRATE, DEMO_CHANNEL);
    ret += lora_tpower_config(DEMO_TPOWER);
    ret += lora_workmode_config(DEMO_WORKMODE);
    ret += lora_tmode_config(DEMO_TMODE);
    ret += lora_wltime_config(DEMO_WLTIME);
    ret += lora_uart_config(DEMO_UARTRATE, DEMO_UARTPARI);
    
    /* 4. 退出配置模式 */
    lora_exit_config();

    /* 5. 检查配置是否全部成功 */
    if (ret != LORA_EOK)
    {
        oled_clear(&oled);
        oled_show(&oled, 30, 25, 500, "LoRa err!");

        return ret;
    }
    else
    {
        oled_clear(&oled);
        oled_show(&oled, 30, 40, 500, "LoRa ok!");
    }
    
    return ret;
}

uint8_t nbiot_startup(uint32_t baudrate)
{
    return 0;
}

/* ---------- 对外接口实现 ---------- */
void transmission_init(void)
{
    lora_startup(115200);
    nbiot_startup(115200);
    oled_clear(&oled);
}

/* ========== JSON 打包 ========== */
static uint16_t pack_json(const sensor_data_t *data, char *buf, uint16_t size)
{
    if (!data->data_valid) 
    {
        return 0;
    }

    return snprintf(buf, size,
        "{\"temp\":%.1f,\"humi\":%.1f,\"light\":%.1f,\"time\":%lu}",
        data->temperature,
        data->humidity,
        data->light_intensity,
        data->timestamp);
}

/* ========== 数据发送 ========== */
uint8_t transmission_send(const sensor_data_t *data)
{   
    uint8_t ret;
    char tx_buf[128];
    uint16_t len;

    if (!data->data_valid) 
    {
        return TRANS_NO_DATA;
    }

    len = pack_json(data, tx_buf, sizeof(tx_buf));
    if (len == 0) 
    {
        return TRANS_ERROR;
    }

    /* 根据当前模式发送 */
    if (g_current_mode == TRANSMISSION_MODE_LORA) 
    {
        /* 检查 LoRa 模块是否空闲 */
        if (lora_free() == LORA_EBUSY) 
        {
            return TRANS_BUSY;
        }
        
        /* 发送 JSON 数据 */
        ret = lora_uart_printf("%s\r\n", tx_buf);   // 添加换行便于接收端解析

        /* 根据发送结果进行处理 */
        switch (ret) 
        {
            case LORA_PRINTF_OK:
                buzzer_beep_scene(BUZZER_SCENE_OK, BUZZER_MODE_INTERMITTENT, BUZZER_RHYTHM_FAST, 200);
                return TRANS_OK;

            case LORA_PRINTF_ERR_FORMAT:
                /* 格式化错误（理论上不应发生，但可记录日志） */
                return TRANS_ERROR;

            case LORA_PRINTF_ERR_TRUNCATED:
                /* 数据被截断但仍发送，可根据业务需求决定是否视为成功 */
                buzzer_beep_scene(BUZZER_SCENE_OK, BUZZER_MODE_INTERMITTENT, BUZZER_RHYTHM_ALARM, 200);
                return TRANS_TRUNCATED;

            default:
                return TRANS_ERROR;
        }
    } 
    else 
    {
        // NB-IoT 发送（待实现）
        return TRANS_ERROR;
    }
}

/* ========== 接收处理（主循环中调用） ========== */
void transmission_receive(void)
{
    uint8_t *rx_frame;
    if (g_current_mode != TRANSMISSION_MODE_LORA) return;

    rx_frame = lora_uart_rx_get_frame();
    if (rx_frame != NULL)
    {
        sensor_data_t rx_data = {0};
        /* 解析 JSON */
        if (sscanf((char*)rx_frame, "{\"temp\":%f,\"humi\":%f,\"light\":%f,\"time\":%lu}",
                   &rx_data.temperature,
                   &rx_data.humidity,
                   &rx_data.light_intensity,
                   &rx_data.timestamp) == 4)
        {
            rx_data.data_valid = 1;
            display_sensor_data(&rx_data, DISPLAY_MODE_LORA);
            buzzer_beep_scene(BUZZER_SCENE_OK, BUZZER_MODE_INTERMITTENT, BUZZER_RHYTHM_FAST, 200);
        }
        else
        {
            display_error("Parse Error");
        }

        lora_uart_rx_restart();
        delay_ms(4000);
    }
}

