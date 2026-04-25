#ifndef __TRANSMISSION_H
#define __TRANSMISSION_H

#include "display.h"
#include "buzzer.h"
#include "delay.h"
#include "nbiot.h"
#include "lora.h"
#include "oled.h"
#include "key.h"
#include "led.h"

/* LoRa模块配置参数定义 */
#define DEMO_ADDR       2                        /* 设备地址 */
#define DEMO_WLRATE     LORA_WLRATE_19K2         /* 空中速率 */
#define DEMO_CHANNEL    20                       /* 信道 */
#define DEMO_TPOWER     LORA_TPOWER_20DBM        /* 发射功率 */
#define DEMO_WORKMODE   LORA_WORKMODE_NORMAL     /* 工作模式 */
#define DEMO_TMODE      LORA_TMODE_TT            /* 发射模式 */
#define DEMO_WLTIME     LORA_WLTIME_1S           /* 休眠时间 */
#define DEMO_UARTRATE   LORA_UARTRATE_115200BPS  /* UART通讯波特率 */
#define DEMO_UARTPARI   LORA_UARTPARI_NONE

/* 错误码 */
#define TRANS_OK          0
#define TRANS_ERROR       1
#define TRANS_NO_DATA     2
#define TRANS_BUSY        3
#define TRANS_TRUNCATED   4

/* 传输模式定义 */
typedef enum {
    TRANSMISSION_MODE_LORA = 0,
    TRANSMISSION_MODE_NBIOT
} transmission_mode_t;


void transmission_init(void);                           /* 初始化配置 */
uint8_t transmission_send(const sensor_data_t *data);   /* 数据发送 */
void transmission_receive(void);                        /* 接收处理（应在主循环中周期性调用） */


#endif // !__TRANSMISSION_H
