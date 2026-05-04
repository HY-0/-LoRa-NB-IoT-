#ifndef __TRANSMISSION_H
#define __TRANSMISSION_H

#include "acquisition.h"
#include "buzzer.h"
#include "delay.h"
#include "nbiot.h"
#include "lora.h"
#include "oled.h"
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

/* ─── 发送错误码 ─── */
#define TRANS_OK         0
#define TRANS_ERROR      1
#define TRANS_BUSY       2
#define TRANS_NO_DATA    3
#define TRANS_TRUNCATED  4

/* ─── 接收状态码 ─── */
#define TRANS_RECV_OK           0
#define TRANS_RECV_NO_DATA      1
#define TRANS_RECV_PARSE_ERROR  2

/* ── 通信状态结构体 ── */
typedef struct {
    uint8_t lora_ready;       /* LoRa 模块空闲可发送 (1: 就绪, 0: 忙) */
    uint8_t nbiot_connected;  /* NB-IoT 是否已连接服务器 (1: 已连接, 0: 未连接) */
} comm_status_t;

/* 初始化配置 */
void transmission_lora_init(void);
void transmission_nbiot_init(void);     
                     
uint8_t transmission_send(const sensor_data_t *data);   /* 数据发送 */
uint8_t transmission_receive(sensor_data_t *out_data);                        /* 接收处理（应在主循环中周期性调用） */
comm_status_t get_comm_status(void);

#endif // !__TRANSMISSION_H
