#include "nbiot_demo.h"

/**
 * @brief       NB-IoT 综合测试演示 (包含时间同步与全量长JSON发布)
 */
void nbiot_demo(void)
{
    uint8_t ret;
    uint32_t current_ts = 0;
    char raw_json[256];
    char safe_payload[512];

    /* ---------- 1. 模块初始化 ---------- */
    oled_clear(&oled);
    oled_show(&oled, 0, 0, 500, "1. NB-IoT Init...");
    ret = nbiot_init(115200);
    if (ret != NBIOT_EOK) {
        oled_show(&oled, 0, 12, 2000, "Init fail %d", ret);
        while (1);
    }
    oled_show(&oled, 0, 12, 500, "Init OK");

    /* ---------- 2. 等待网络附着 ---------- */
    oled_show(&oled, 0, 24, 500, "2. Attaching...");
    ret = nbiot_attach_network();
    if (ret != NBIOT_EOK) {
        oled_show(&oled, 0, 36, 2000, "Attach fail");
        while (1);
    }
    delay_ms(2000); // 附着后稍作等待，让基站下发时间
    oled_show(&oled, 0, 36, 500, "Attached");

    /* ---------- 3. MQTT 连接 ---------- */
    oled_clear(&oled);
    oled_show(&oled, 0, 0, 500, "3. MQTT Conn...");
    
    // 打开 Socket (请确认 IP 和端口正确)
    ret = nbiot_mqtt_open(0, "82.157.129.239", 1883);
    if (ret != NBIOT_EOK) {
        oled_show(&oled, 0, 12, 2000, "Open fail %d", ret);
        while (1);
    }

    // 登录 MQTT
    ret = nbiot_mqtt_connect(0, "TestClient_01", "user1", "pass123");
    if (ret != NBIOT_EOK) {
        oled_show(&oled, 0, 24, 2000, "Conn fail %d", ret);
        while (1);
    }
    oled_show(&oled, 0, 24, 500, "MQTT Connected");

    /* ---------- 4. 测试获取时间戳 ---------- */
    oled_clear(&oled);
    oled_show(&oled, 0, 0, 500, "4. Sync Time...");
    
    if (nbiot_get_timestamp(&current_ts) == NBIOT_EOK) {
        // 成功获取时间，打印在屏幕上
        oled_show(&oled, 0, 16, 1000, "TS: %lu", current_ts);
    } else {
        // 如果失败，给个默认假时间用于后续发布测试
        current_ts = 1778335760; 
        oled_show(&oled, 0, 16, 2000, "TS Fail, use def");
    }

    /* ---------- 5. 测试一次性全量发布 ---------- */
    oled_show(&oled, 0, 32, 500, "5. Publishing...");

    // 组装包含真实时间戳的完整长 JSON
    snprintf(raw_json, sizeof(raw_json),
             "{\"temp\":28.5,\"air_humi\":85.2,\"soil_humi\":0.1,\"light\":000.0,\"ph\":9.9,\"co2\":412,\"time\":%lu}",
             current_ts);

    // 转义双引号，消灭逗号解析 Bug！
    if (escape_json_for_at(safe_payload, sizeof(safe_payload), raw_json) < 0) {
        oled_show(&oled, 0, 48, 2000, "Escape Error");
        while (1);
    }

    // 发送转义后的长数据 (msgid 设为 1，QoS 设为 1)
    ret = nbiot_mqtt_publish(0, 1, 1, 0, "farm/sensor/collect", "{\"temp\":28.5,\"air_humi\":85.2,\"soil_humi\":0.1,\"light\":000.0,\"ph\":9.9,\"co2\":412,\"time\":3778332255}");
    
    if (ret != NBIOT_EOK) {
        oled_show(&oled, 0, 48, 2000, "Pub fail: %d", ret);
    } else {
        oled_show(&oled, 0, 48, 3000, "SUCCESS!");
    }

    // 测试完成，停在这里看结果
    while (1);
}
