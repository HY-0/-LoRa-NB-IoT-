#include "transmission.h"

/* ---------- 内部辅助函数 ---------- */
uint8_t lora_startup(uint32_t baudrate)
{
    uint8_t ret;

    /* 显示标题 */
    oled_clear(&oled);
    oled_set_pen(&oled, PEN_COLOR_WHITE, 1);         /* 设置白画笔 */
    oled_set_brush(&oled, PEN_COLOR_TRANSPARENT);    /* 透明画刷 */
    oled_set_cursor(&oled, 0, 25);
    oled_draw_string(&oled, "*******************");
    oled_set_cursor(&oled, 0, 40);
    oled_draw_string(&oled, "<<<<   LoRa   >>>>");
    oled_set_cursor(&oled, 0, 55);
    oled_draw_string(&oled, "*******************");
    oled_send_buffer(&oled);   // 立即显示
    delay_ms(500);            // 保留必要延时

    oled_clear(&oled);
	/* 1. 硬件初始化 */
    ret = lora_init(baudrate);
    if (ret != LORA_EOK)
    {
        oled_clear(&oled);
        oled_set_cursor(&oled, 30, 25);
        oled_draw_string(&oled, "LoRa err!");
        oled_send_buffer(&oled);
        led0_toggle();            // 错误指示

        return ret;
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
        oled_set_cursor(&oled, 30, 25);
        oled_draw_string(&oled, "LoRa err!");
        oled_send_buffer(&oled);
        return ret;
    }
    else
    {
        oled_clear(&oled);
        oled_set_cursor(&oled, 30, 40);
        oled_draw_string(&oled, "LoRa ok!");
        oled_send_buffer(&oled);
        delay_ms(500);           // 让用户看到成功信息
    }
    
    return LORA_EOK;
}

uint8_t nbiot_startup(uint32_t baudrate)
{
    uint8_t ret;
    nbiot_csq_t csq;

    /* 显示标题 */
    oled_clear(&oled);
    oled_set_pen(&oled, PEN_COLOR_WHITE, 1);         /* 设置白画笔 */
    oled_set_brush(&oled, PEN_COLOR_TRANSPARENT);    /* 透明画刷 */
    oled_set_cursor(&oled, 0, 25);
    oled_draw_string(&oled, "*******************");
    oled_set_cursor(&oled, 0, 40);
    oled_draw_string(&oled, "<<<   NB-IoT   >>>");
    oled_set_cursor(&oled, 0, 55);
    oled_draw_string(&oled, "*******************");
    oled_send_buffer(&oled);   // 立即显示
    delay_ms(500);            // 保留必要延时

    // 1. 模块硬件初始化 + AT 测试 + 关闭回显
    ret = nbiot_init(baudrate);
    if (ret != NBIOT_EOK) {
        oled_clear(&oled);
        oled_set_cursor(&oled, 30, 40);
        oled_draw_string(&oled, "NB err!");
        oled_send_buffer(&oled);
        return ret;
    }
    oled_clear(&oled);                     // 清屏，准备显示进度
    oled_set_cursor(&oled, 30, 40);
    oled_draw_string(&oled, "NB OK!");
    oled_send_buffer(&oled);               // 立即刷新

    // 2. 附着网络 (等待网络注册)
    ret = nbiot_attach_network();
    if (ret != NBIOT_EOK) {
        oled_clear(&oled);
        oled_set_cursor(&oled, 30, 40);
        oled_draw_string(&oled, "NB err!");
        oled_send_buffer(&oled);
        return ret;
    }
    delay_ms(3000);                        // 等待网络附着完成（必须）
    oled_clear(&oled);
    oled_set_cursor(&oled, 0, 12);
    oled_draw_string(&oled, "Attached");
    oled_send_buffer(&oled);

    // 3. 查询信号质量
    nbiot_get_csq(&csq);
    oled_set_cursor(&oled, 0, 24);
    oled_printf(&oled, "CSQ:%d,%d", csq.rssi, csq.ber);
    oled_send_buffer(&oled);

    // 4. 打开 MQTT 连接（socket）
    ret = nbiot_mqtt_open(0, "82.157.129.239", 1883);
    if (ret != NBIOT_EOK) {
        oled_clear(&oled);
        oled_set_cursor(&oled, 10, 20);
        oled_draw_string(&oled, "MQTT open fail");
        oled_send_buffer(&oled);
        return ret;
    }
    oled_set_cursor(&oled, 0, 36);
    oled_draw_string(&oled, "MQTT open OK");
    oled_send_buffer(&oled);

    // 5. MQTT 连接
    ret = nbiot_mqtt_connect(0, "myClient123", "user1", "pass123");
    if (ret != NBIOT_EOK) {
        oled_clear(&oled);
        oled_set_cursor(&oled, 10, 20);
        oled_draw_string(&oled, "MQTT conn fail");
        oled_send_buffer(&oled);
        return ret;
    }
    oled_set_cursor(&oled, 0, 48);
    oled_draw_string(&oled, "MQTT conn OK");
    oled_send_buffer(&oled);

    // 6. 订阅主题
    nbiot_mqtt_subscribe(0, 1, "farm/sensor/collect", 1);
    oled_set_cursor(&oled, 0, 60);
    oled_draw_string(&oled, "Subscribed");
    oled_send_buffer(&oled);

    // 可选：等待 2 秒后清屏，准备显示传感器数据
    delay_ms(2000);
    oled_clear(&oled);
    oled_send_buffer(&oled);

    return NBIOT_EOK;
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
        "\"light\":%.1f,\"ph\":%.1f,\"co2\":%hu,\"time\":%lu}",
        data->air_temp,
        data->air_humi,
        data->soil_humi,
        data->light,
        data->ph,
        data->co2,
        data->timestamp);
}
// NB-IoT GA7模块不支持16进制格式
// /* 将字符串转换为大写十六进制字符串，dst 必须足够大（2*len+1） */
// static void str_to_hex(const char *src, char *dst)
// {
//     while (*src) {
//         sprintf(dst, "%02X", (unsigned char)*src);
//         dst += 2;
//         src++;
//     }
//     *dst = '\0';
// }

/* ========== 数据发送 ========== */
uint8_t transmission_lora_send(const sensor_data_t *data)
{   
    char json_buf[256];
    uint16_t len;
    uint8_t ret_lora;

    if (!data->data_valid) return TRANS_NO_DATA;

    len = pack_json(data, json_buf, sizeof(json_buf));
    if (len == 0) return TRANS_ERROR;

    /* ----- LoRa 发送 ----- */
    if (lora_free() == LORA_EBUSY) {
        ret_lora = TRANS_BUSY;
    } else {
        uint8_t lora_ret = lora_uart_printf("%s\r\n", json_buf);
        switch (lora_ret) {
            case LORA_PRINTF_OK:          ret_lora = TRANS_OK;      break;
            case LORA_PRINTF_ERR_TRUNCATED: ret_lora = TRANS_TRUNCATED; break;
            default:                      ret_lora = TRANS_ERROR;   break;
        }
    }

    /* 根据 LoRa 发送结果返回 */
    switch (ret_lora) {
        case TRANS_OK:       return TRANS_OK;
        case TRANS_BUSY:     return TRANS_BUSY;
        default:             return TRANS_ERROR;
    }
}

uint8_t transmission_nbiot_send(const sensor_data_t *data)
{   
    char json_buf[256];
    char safe_payload[512]; // 转义后的缓冲区，因为增加了反斜杠，需要设大一些
    uint16_t len;
    uint8_t ret_nbiot;
    
    // 增加静态变量，每次发送累加，避免 QoS=1 队列塞满导致的超时
    static uint16_t s_mqtt_msgid = 1; 

    if (!data->data_valid) return TRANS_NO_DATA;

    // 1. 生成原始 JSON 字符串
    len = pack_json(data, json_buf, sizeof(json_buf));
    if (len == 0) return TRANS_ERROR;

    // 2. 转义双引号 (将 " 变成 \")
    if (escape_json_for_at(safe_payload, sizeof(safe_payload), json_buf) < 0) {
        return TRANS_ERROR; // 缓冲区不足转义失败
    }

    // 3. 使用转义后的字符串发送 (不使用 hex 函数)
    uint8_t nb_ret = nbiot_mqtt_publish(0, s_mqtt_msgid, 1, 0, "farm/sensor/collect", safe_payload);

    // 4. 发送完毕后更新 msgid，限定在 1 ~ 65535 范围内
    s_mqtt_msgid++;
    if (s_mqtt_msgid == 0) {
        s_mqtt_msgid = 1;
    }

    /* 根据 NB-IoT 发送结果映射错误码 */
    switch (nb_ret) {
        case NBIOT_EOK:      ret_nbiot = TRANS_OK;    break;
        case NBIOT_TIMEOUT:  ret_nbiot = TRANS_BUSY;  break;
        default:             ret_nbiot = TRANS_ERROR; break;
    }

    return ret_nbiot;
}

/* ========== 接收处理（主循环中调用） ========== */
uint8_t transmission_receive(sensor_data_t *out_data)        
{
    uint8_t *frame;
    sensor_data_t rx_data;
    int fields;

    /* ----- LoRa 接收（原有代码） ----- */
    frame = lora_uart_rx_get_frame();
    if (frame != NULL) {
        usart_printf(USART2, "lora rx: %s", frame);
        memset(&rx_data, 0, sizeof(rx_data));
        fields = sscanf((char*)frame,
            "{\"temp\":%f,\"air_humi\":%f,\"soil_humi\":%f,"
            "\"light\":%f,\"ph\":%f,\"co2\":%hu,\"time\":%lu}",
            &rx_data.air_temp, &rx_data.air_humi, &rx_data.soil_humi,
            &rx_data.light, &rx_data.ph, &rx_data.co2, &rx_data.timestamp);
        if (fields == 7) {
            rx_data.data_valid = 1;
            *out_data = rx_data;
            lora_uart_rx_restart();
            return TRANS_RECV_OK;
        } 
        else 
        {
            return TRANS_RECV_PARSE_ERROR;
        }
    }

    /* ----- NB‑IoT MQTT 接收 ----- */
    nbiot_mqtt_recv_t mqtt_msg;
    if (nbiot_mqtt_recv(&mqtt_msg) == NBIOT_EOK) {
        // 解析 MQTT payload 中的 JSON（前提是 payload 是字符串格式）
        memset(&rx_data, 0, sizeof(rx_data));
        fields = sscanf(mqtt_msg.payload,
            "{\"temp\":%f,\"air_humi\":%f,\"soil_humi\":%f,"
            "\"light\":%f,\"ph\":%f,\"co2\":%hu,\"time\":%lu}",
            &rx_data.air_temp, &rx_data.air_humi, &rx_data.soil_humi,
            &rx_data.light, &rx_data.ph, &rx_data.co2, &rx_data.timestamp);
        if (fields == 7) {
            rx_data.data_valid = 1;
            *out_data = rx_data;
            return TRANS_RECV_OK;
        } 
        else 
        {
            return TRANS_RECV_PARSE_ERROR;
        }
    }

    return TRANS_RECV_NO_DATA;
}

/* ========== 模块内部状态 ========== */
comm_status_t get_comm_status(void)
{
    comm_status_t status;
    static int8_t cached_rssi = -1;
    static uint32_t last_csq_time = 0;
    static uint8_t first_call = 1;      // 新增：首次运行标志
    uint32_t now = get_ms();

    status.lora_ready = (lora_free() == LORA_EOK) ? 1 : 0;
    status.nbiot_connected = g_nb_mqtt_connected;
    status.nbiot_rssi = cached_rssi;

    // 逻辑：如果是首次运行，或者时间差 >= 10000ms，就触发
    if (first_call || (now - last_csq_time >= 10000)) {
        first_call = 0;                 // 触发后立刻清除标志
        
        nbiot_csq_t csq;
        if (nbiot_get_csq(&csq) == NBIOT_EOK) {
            cached_rssi = csq.rssi;
        } else {
            cached_rssi = -1;
        }
        last_csq_time = now;
        status.nbiot_rssi = cached_rssi;
    }
    return status;
}
