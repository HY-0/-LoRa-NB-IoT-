#ifndef __NBIOT_H
#define __NBIOT_H

#include <string.h>
#include <stdio.h>
#include "nbiot_usart.h"     /* 使用自己的 UART 接口 */
#include "stm32f10x.h"
#include "delay.h"
#include "sys.h"

/* AT响应等待超时时间（毫秒） */
#define NBIOT_AT_TIMEOUT        5000     /* 普通命令超时 */
#define NBIOT_LONG_TIMEOUT      50000    /* 长操作超时（网络注册、连接等） */

/* ---------- 硬件引脚定义（请根据实际接线修改）---------- */
#define NBIOT_PWR_GPIO_PORT         GPIOC
#define NBIOT_PWR_GPIO_PIN          GPIO_Pin_13
#define NBIOT_PWR_GPIO_CLK_ENABLE() do{ RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE); }while(0)

#define NBIOT_RESET_GPIO_PORT       GPIOC
#define NBIOT_RESET_GPIO_PIN        GPIO_Pin_14
#define NBIOT_RESET_GPIO_CLK_ENABLE() do{ RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE); }while(0)

/* IO操作 */
#define NBIOT_PWR(x)     do{ (x) ?                                                                  \
                               GPIO_WriteBit(NBIOT_PWR_GPIO_PORT, NBIOT_PWR_GPIO_PIN, Bit_SET) :    \
                               GPIO_WriteBit(NBIOT_PWR_GPIO_PORT, NBIOT_PWR_GPIO_PIN, Bit_RESET);   \
                           }while(0)

#define NBIOT_RESET(x)   do{ (x) ?                                                                      \
                               GPIO_WriteBit(NBIOT_RESET_GPIO_PORT, NBIOT_RESET_GPIO_PIN, Bit_SET) :    \
                               GPIO_WriteBit(NBIOT_RESET_GPIO_PORT, NBIOT_RESET_GPIO_PIN, Bit_RESET);   \
                           }while(0)


/* 通用错误码 */
#define NBIOT_EOK                0       /* 没有错误 */
#define NBIOT_ERROR              1       /* 通用错误 */
#define NBIOT_TIMEOUT            2       /* 超时错误 */
#define NBIOT_EINVAL             3       /* 参数错误 */
#define NBIOT_EBUSY              4       /* 忙错误 */

/* 打印错误码 */
#define NBIOT_PRINTF_OK          0       /* 成功 */
#define NBIOT_PRINTF_ERR_FORMAT  1       /* 格式化错误 */
#define NBIOT_PRINTF_ERR_TRUNCATED 2     /* 输出被截断 */

/* ---------- 全局状态变量 ---------- */
extern volatile uint8_t g_nb_mqtt_connected;   // NB-IoT MQTT 连接状态 (1:已连接, 0:未连接)

/* ---------- 使能/禁用枚举 ---------- */
typedef enum {
    NBIOT_DISABLE = 0x00,
    NBIOT_ENABLE,
} nbiot_enable_t;

/* ---------- 信号质量结构体 ---------- */
typedef struct {
    int rssi;       /* 接收信号强度，如 5 */
    int ber;        /* 误码率，如 0 */
} nbiot_csq_t;

/* ---------- 网络注册状态枚举 ---------- */
typedef enum {
    NBIOT_CREG_NOT_REGISTERED       = 0,    /* 未注册 */
    NBIOT_CREG_REGISTERED_HOME      = 1,    /* 已注册，归属网络 */
    NBIOT_CREG_SEARCHING            = 2,    /* 搜索中 */
    NBIOT_CREG_DENIED               = 3,    /* 注册被拒绝 */
    NBIOT_CREG_UNKNOWN              = 4,    /* 未知 */
    NBIOT_CREG_REGISTERED_ROAMING   = 5,    /* 已注册，漫游 */
    NBIOT_CREG_REGISTERED_HOME_NBIOT= 6     /* 已注册，归属网络（NB-IoT模式） */
} nbiot_creg_stat_t;

/* ---------- MQTT 接收消息定义 ---------- */
#define NBIOT_RECV_MSG_MAX_LEN      256

typedef struct {
    int socket_id;
    int msgid;
    char topic[64];
    char payload[NBIOT_RECV_MSG_MAX_LEN];
} nbiot_mqtt_recv_t;

/* ---------- 函数声明 ---------- */
/* 初始化与基础操作 */
uint8_t nbiot_init(uint32_t baudrate);
uint8_t nbiot_at_test(void);
uint8_t nbiot_echo_config(nbiot_enable_t enable);
uint8_t nbiot_sw_reset(void);

/* 网络操作 */
uint8_t nbiot_get_csq(nbiot_csq_t *csq);
uint8_t nbiot_get_creg(nbiot_creg_stat_t *stat);
uint8_t nbiot_attach_network(void);

/* MQTT 操作 */
uint8_t nbiot_mqtt_open(uint8_t socket_id, const char *host, uint16_t port);
uint8_t nbiot_mqtt_connect(uint8_t socket_id, const char *clientid,
                           const char *username, const char *password);
uint8_t nbiot_mqtt_subscribe(uint8_t socket_id, uint16_t msgid,
                             const char *topic, uint8_t qos);
uint8_t nbiot_mqtt_publish(uint8_t socket_id, uint16_t msgid,
                           uint8_t qos, uint8_t retain,
                           const char *topic, const char *payload);

uint8_t nbiot_mqtt_publish_hex(uint8_t socket_id, uint16_t msgid,
                               uint8_t qos, uint8_t retain,
                               const char *topic, const char *payload_hex);

uint8_t nbiot_mqtt_close(uint8_t socket_id);

/* 消息接收（轮询） */
uint8_t nbiot_mqtt_recv(nbiot_mqtt_recv_t *recv);

#endif /* __NBIOT_H */
