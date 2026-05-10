#include "nbiot_demo.h"
/**
 * @brief       NB-IoT 演示入口
 */
void nbiot_demo(void)
{
    uint8_t ret;
    // nbiot_mqtt_recv_t msg;


    /* ---------- 1. 模块初始化 ---------- */
    oled_clear(&oled);
    oled_show(&oled, 0, 10, 500, "NB-IoT Init...");
    ret = nbiot_init(115200);                     /* 根据模块波特率调整 */
    if (ret != NBIOT_EOK) {
        oled_clear(&oled);
        oled_show(&oled, 0, 20, 2000, "Init fail %d", ret);
        while (1);
    }
    oled_show(&oled, 0, 20, 500, "Init OK");

    delay_ms(1000);                             /* 等待模块稳定 */

    /* ---------- 2. 查询信号质量 ---------- */
    nbiot_csq_t csq;
    if (nbiot_get_csq(&csq) == NBIOT_EOK) {
        oled_show(&oled, 0, 32, 500, "CSQ:%d,%d", csq.rssi, csq.ber);
    } else {
        oled_show(&oled, 0, 32, 500, "CSQ:fail");
    }

    /* ---------- 3. 查询网络注册状态 ---------- */
    nbiot_creg_stat_t stat;
    if (nbiot_get_creg(&stat) == NBIOT_EOK) {
        switch (stat) {
            case NBIOT_CREG_REGISTERED_HOME:
                oled_show(&oled, 0, 48, 500, "Reg:Home"); break;
            case NBIOT_CREG_REGISTERED_ROAMING:
                oled_show(&oled, 0, 48, 500, "Reg:Roam"); break;
            case NBIOT_CREG_SEARCHING:
                oled_show(&oled, 0, 48, 500, "Search"); break;
            case NBIOT_CREG_DENIED:
                oled_show(&oled, 0, 48, 500, "Denied"); break;
            case NBIOT_CREG_REGISTERED_HOME_NBIOT:   // 新增
                oled_show(&oled, 0, 48, 500, "Reg:NBIoT"); break;
            default:
                oled_show(&oled, 0, 48, 500, "Unknow"); break;
        }
    } else {
        oled_show(&oled, 0, 48, 500, "CREG:fail");
    }

    /* ---------- 4. 打开 MQTT 连接 ---------- */
    oled_clear(&oled);
    oled_show(&oled, 0, 10, 500, "Open MQTT...");
    ret = nbiot_mqtt_open(0, "82.157.129.239", 1883);
    if (ret != NBIOT_EOK) {
        oled_show(&oled, 0, 20, 2000, "Open fail %d", ret);
        while (1);
    }
    oled_show(&oled, 0, 20, 500, "Open OK");

    /* ---------- 5. MQTT 连接 ---------- */
    oled_show(&oled, 0, 32, 500, "Connect...");
    ret = nbiot_mqtt_connect(0, "myClient123", "user1", "pass123");
    if (ret != NBIOT_EOK) {
        oled_show(&oled, 0, 48, 2000, "Conn fail %d", ret);
        while (1);
    }
    oled_show(&oled, 0, 48, 500, "Connect OK");

    /* ---------- 6. 订阅主题 ---------- */
    oled_clear(&oled);
    oled_show(&oled, 0, 10, 500, "Subscribe...");
    ret = nbiot_mqtt_subscribe(0, 1, "farm/sensor/collect", 1);
    if (ret != NBIOT_EOK) {
        oled_show(&oled, 0, 20, 2000, "Sub fail %d", ret);
        while (1);
    }
    oled_show(&oled, 0, 20, 500, "Sub OK");

    /* ---------- 7. 发布测试消息 ---------- */
    oled_show(&oled, 0, 32, 500, "Publish...");

    ret = nbiot_mqtt_publish(0, 2, 1, 0, "farm/sensor/collect", 
            "{\"temp\":28.5,\"air_humi\":65.2,\"soil_humi\":40.1,\"light\":1000.0,\"time\":3978484480}");
    ret = nbiot_mqtt_publish(0, 2, 1, 0, "farm/sensor/collect", 
            "{\"soil_humi\":66.1,\"light\":6000.0,\"ph\":6.9,\"co2\":412,\"time\":3978484480}");
    if (ret != NBIOT_EOK) {
        oled_show(&oled, 0, 48, 2000, "Pub fail %d", ret);
        while (1);
    }
    oled_show(&oled, 0, 48, 500, "Pub OK");

    // /* ---------- 8. 循环接收消息 ---------- */
    // oled_clear(&oled);
    // oled_show(&oled, 0, 10, 500, "Receiving...");
    // while (1) {
    //     ret = nbiot_mqtt_recv(&msg);
    //     if (ret == NBIOT_EOK) {
    //         oled_show(&oled, 0, 20, 200, "RX:%s", msg.payload);
    //         if (strstr(msg.payload, "quit") != NULL) {
    //             oled_show(&oled, 0, 32, 500, "Quit cmd");
    //             nbiot_mqtt_close(0);
    //             break;
    //         }
    //     } else if (ret == NBIOT_EBUSY) {
    //         oled_show(&oled, 0, 48, 0, "..wait..");
    //     } else {
    //         oled_show(&oled, 0, 48, 500, "RX err %d", ret);
    //     }
    //     delay_ms(200);
    // }

    // oled_show(&oled, 0, 0, 2000, "Demo finished");
    while (1);
}
