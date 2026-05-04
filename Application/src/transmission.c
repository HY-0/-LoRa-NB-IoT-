#include "transmission.h"

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
    
    return LORA_EOK;
}

uint8_t nbiot_startup(uint32_t baudrate)
{
    return 0;
}

/* ---------- 对外接口实现 ---------- */
void transmission_lora_init(void)
{
    lora_startup(115200);
    oled_clear(&oled);   // 如果不需要在初始化时清屏，可删除
}

void transmission_nbiot_init(void)
{
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
        "{\"temp\":%.1f,\"air_humi\":%.1f,\"soil_humi\":%.1f,"
        "\"light\":%.1f,\"ph\":%.2f,\"co2\":%.0f,\"time\":%lu}",
        data->temperature,
        data->air_humidity,
        data->soil_humidity,
        data->light_intensity,
        data->ph,
        data->co2,
        data->timestamp);
}

/* ========== 数据发送 ========== */
uint8_t transmission_send(const sensor_data_t *data)
{   
    char tx_buf[256];
    uint16_t len;
    uint8_t ret_lora, ret_nbiot;

    if (!data->data_valid) return TRANS_NO_DATA;

    len = pack_json(data, tx_buf, sizeof(tx_buf));
    
    if (len == 0) return TRANS_ERROR;

    /* LoRa 发送 */
    if (lora_free() == LORA_EBUSY) 
    {
        ret_lora = TRANS_BUSY;
    } 
    else 
    {
        uint8_t lora_ret = lora_uart_printf("%s\r\n", tx_buf);
        switch (lora_ret) 
        {
            case LORA_PRINTF_OK:          ret_lora = TRANS_OK;          break;
            case LORA_PRINTF_ERR_TRUNCATED: ret_lora = TRANS_TRUNCATED; break;
            default:                      ret_lora = TRANS_ERROR;       break;
        }
    }
    /* NB‑IoT 发送（预留） */
    ret_nbiot = TRANS_ERROR;

    if (ret_lora == TRANS_OK || ret_nbiot == TRANS_OK) return TRANS_OK;
    else if (ret_lora == TRANS_BUSY && ret_nbiot == TRANS_BUSY) return TRANS_BUSY;
    else return TRANS_ERROR;
}

/* ========== 接收处理（主循环中调用） ========== */
uint8_t transmission_receive(sensor_data_t *out_data)        
{
    uint8_t *frame;
    sensor_data_t rx_data;
    int fields;

    /* LoRa 接收 */
    frame = lora_uart_rx_get_frame();
    if (frame != NULL) 
    {
        memset(&rx_data, 0, sizeof(rx_data));
        fields = sscanf((char*)frame,
            "{\"temp\":%f,\"air_humi\":%f,\"soil_humi\":%f,"
            "\"light\":%f,\"ph\":%f,\"co2\":%f,\"time\":%lu}",
            &rx_data.temperature,
            &rx_data.air_humidity,
            &rx_data.soil_humidity,
            &rx_data.light_intensity,
            &rx_data.ph,
            &rx_data.co2,
            &rx_data.timestamp);
        
        if (fields == 5) 
        {   
            rx_data.data_valid = 1;
            *out_data = rx_data;
            return TRANS_RECV_OK;
        } 
        else 
        {
            return TRANS_RECV_PARSE_ERROR;
        }
    }

    /* NB‑IoT 接收处理（预留） */
    // 可在此处获取 MQTT 消息，但注意应返回统一 sensor_data_t
    // 若需要单独的 NB‑IoT 消息处理，可另写函数

    return TRANS_RECV_NO_DATA;
}

/* ========== 模块内部状态 ========== */
comm_status_t get_comm_status(void)
{
    comm_status_t status;
    status.lora_ready      = (lora_free() == LORA_EOK) ? 1 : 0;
    status.nbiot_connected = 0;   /* 待实现：读取 NB-IoT 连接状态标志 */
    return status;
}
