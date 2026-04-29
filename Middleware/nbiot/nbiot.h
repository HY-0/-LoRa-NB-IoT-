#ifndef __NBIOT_H
#define __NBIOT_H

#include <string.h>
#include <stdio.h>
#include "nbiot_usart.h"
#include "stm32f10x.h"
#include "delay.h"          /* 提供毫秒级延时 */



/* ===================== 硬件引脚定义（示例） ===================== */
/* 可根据实际硬件修改，以下为参考 */
/* DRX、DTX */
#define NBIOT_PWR_GPIO_PORT           GPIOB
#define NBIOT_PWR_GPIO_PIN            GPIO_Pin_12
#define NBIOT_PWR_GPIO_CLK_ENABLE()   do{ RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE); }while(0)

#define NBIOT_RST_GPIO_PORT           GPIOB
#define NBIOT_RST_GPIO_PIN            GPIO_Pin_13
#define NBIOT_RST_GPIO_CLK_ENABLE()   do{ RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE); }while(0)

/* IO操作宏（根据模块实际控制引脚） */
#define NBIOT_PWR_ENABLE()            GPIO_WriteBit(NBIOT_PWR_GPIO_PORT, NBIOT_PWR_GPIO_PIN, Bit_SET)
#define NBIOT_PWR_DISABLE()           GPIO_WriteBit(NBIOT_PWR_GPIO_PORT, NBIOT_PWR_GPIO_PIN, Bit_RESET)
#define NBIOT_RST_ASSERT()            GPIO_WriteBit(NBIOT_RST_GPIO_PORT, NBIOT_RST_GPIO_PIN, Bit_RESET)
#define NBIOT_RST_DEASSERT()          GPIO_WriteBit(NBIOT_RST_GPIO_PORT, NBIOT_RST_GPIO_PIN, Bit_SET)

/* ===================== 错误代码 ===================== */
#define NBIOT_EOK           0       /* 成功 */
#define NBIOT_ERROR         1       /* 通用错误 */
#define NBIOT_ETIMEOUT      2       /* 超时错误 */
#define NBIOT_EINVAL        3       /* 参数错误 */
#define NBIOT_EBUSY         4       /* 忙错误 */

#define AT_SIGNAL_WEAK            -1
#define AT_REGISTER_FAIL          -2
#define AT_NETWORK_NOT_ATTACHED   -3
#define AT_APN_CONFIG_ERROR       -4
#define AT_NO_IP_ADDRESS          -5
#define AT_DATA_LINK_ERROR        -6
#define AT_MQTT_CONFIG_ERROR      -7
#define AT_CREATE_INSTANCE_ERROR  -8
#define AT_REGISTER_SERVER_ERROR  -9
#define AT_MQTT_SUB_ERROR         -10
#define AT_MQTT_PUB_ERROR         -11

#define AT_CMD_MAX_LEN            64
/* ===================== 网络注册状态（可选） ===================== */
typedef enum {
    NBIOT_REG_NOT_REGISTERED = 0,
    NBIOT_REG_REGISTERED     = 1,
    NBIOT_REG_SEARCHING      = 2,
    NBIOT_REG_DENIED         = 3,
    NBIOT_REG_UNKNOWN        = 4,
    NBIOT_REG_ROAMING        = 5,
} nbiot_reg_state_t;

/* ===================== 函数声明 ===================== */
/* 模块初始化 */
uint8_t nbiot_init(uint32_t baudrate);          /* 初始化NB-IoT模块（含串口、引脚等） */

/* AT指令基础接口 */
uint8_t nbiot_send_at_cmd(char *cmd, char *ack, uint32_t timeout);   /* 发送AT指令并等待应答 */

/* 模块基础操作 */
uint8_t nbiot_test_communicate_pass(void);      /* 测试通讯串口是否通（AT指令测试） */
uint8_t nbiot_close_ate(void);                  /* 关闭命令回显 */
uint8_t nbiot_wait_creg(uint32_t timeout);      /* 等待模块注册成功 */
uint8_t nbiot_active_pdp(void);                 /* 激活PDP上下文 */

/* MQTT业务接口 */
int connet_mqtt_server(void);                   /* 连接MQTT服务器 */
int send_mqtt_message(char *msg);               /* 发送MQTT消息 */
int disconnect_mqtt_server(void);               /* 断开MQTT服务器 */
int deal_mqtt_message(char *buf);               /* 处理接收到的MQTT消息 */

#endif
