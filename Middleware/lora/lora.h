#ifndef __LORA_H
#define __LORA_H

#include <string.h>
#include <stdio.h>
#include "lora_usart.h"
#include "stm32f10x.h"
#include "delay.h"
#include "sys.h"

/* AT响应等待超时时间（毫秒） */
#define LORA_AT_TIMEOUT  500

/* 引脚定义 */
#define LORA_AUX_GPIO_PORT           GPIOA
#define LORA_AUX_GPIO_PIN            GPIO_Pin_8               
#define LORA_AUX_GPIO_CLK_ENABLE()   do{ RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE); }while(0)

#define LORA_MD0_GPIO_PORT           GPIOB
#define LORA_MD0_GPIO_PIN            GPIO_Pin_15
#define LORA_MD0_GPIO_CLK_ENABLE()   do{ RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE); }while(0)

/* IO操作 */
#define LORA_AUX()                   GPIO_ReadInputDataBit(LORA_AUX_GPIO_PORT, LORA_AUX_GPIO_PIN)
#define LORA_MD0(x)                  do{ (x) ?                                                                      \
                                                GPIO_WriteBit(LORA_MD0_GPIO_PORT, LORA_MD0_GPIO_PIN, Bit_SET) :     \
                                                GPIO_WriteBit(LORA_MD0_GPIO_PORT, LORA_MD0_GPIO_PIN, Bit_RESET);    \
                                            }while(0)

/* 使能枚举 */
typedef enum
{
    LORA_DISABLE             = 0x00,
    LORA_ENABLE,
} lora_enable_t;

/* 发射功率枚举 */
typedef enum
{
    LORA_TPOWER_11DBM        = 0,   /* 11dBm */
    LORA_TPOWER_14DBM        = 1,   /* 14dBm */
    LORA_TPOWER_17DBM        = 2,   /* 17dBm */
    LORA_TPOWER_20DBM        = 3,   /* 20dBm（默认） */
} lora_tpower_t;

/* 工作模式枚举 */
typedef enum
{
    LORA_WORKMODE_NORMAL     = 0,    /* 一般模式（默认） */
    LORA_WORKMODE_WAKEUP     = 1,    /* 唤醒模式 */
    LORA_WORKMODE_LOWPOWER   = 2,    /* 省电模式 */
    LORA_WORKMODE_SIGNAL     = 3,    /* 信号强度模式 */
} lora_workmode_t;

/* 发射模式枚举 */
typedef enum
{
    LORA_TMODE_TT            = 0,    /* 透明传输（默认） */
    LORA_TMODE_DT            = 1,    /* 定向传输 */
} lora_tmode_t;

/* 空中速率枚举 */
typedef enum
{
    LORA_WLRATE_0K3          = 0,    /* 0.3Kbps */
    LORA_WLRATE_1K2          = 1,    /* 1.2Kbps */
    LORA_WLRATE_2K4          = 2,    /* 2.4Kbps */
    LORA_WLRATE_4K8          = 3,    /* 4.8Kbps */
    LORA_WLRATE_9K6          = 4,    /* 9.6Kbps */
    LORA_WLRATE_19K2         = 5,    /* 19.2Kbps（默认） */
} lora_wlrate_t;

/* 休眠时间枚举 */
typedef enum
{
    LORA_WLTIME_1S           = 0,    /* 1秒（默认） */
    LORA_WLTIME_2S           = 1,    /* 2秒 */
} lora_wltime_t;

/* 串口通信波特率枚举 */
typedef enum
{
    LORA_UARTRATE_1200BPS    = 0,    /* 1200bps */
    LORA_UARTRATE_2400BPS    = 1,    /* 2400bps */
    LORA_UARTRATE_4800BPS    = 2,    /* 4800bps */
    LORA_UARTRATE_9600BPS    = 3,    /* 9600bps */
    LORA_UARTRATE_19200BPS   = 4,    /* 19200bps */
    LORA_UARTRATE_38400BPS   = 5,    /* 38400bps */
    LORA_UARTRATE_57600BPS   = 6,    /* 57600bps */
    LORA_UARTRATE_115200BPS  = 7,    /* 115200bps（默认） */
} lora_uartrate_t;

/* 串口通讯校验位枚举 */
typedef enum
{
    LORA_UARTPARI_NONE       = 0,    /* 无校验（默认） */
    LORA_UARTPARI_EVEN       = 1,    /* 偶校验 */
    LORA_UARTPARI_ODD        = 2,    /* 奇校验 */
} lora_uartpari_t;

// /* 操作函数 */
uint8_t lora_init(uint32_t baudrate);                                                 /* ATK-MW1278D初始化 */
void lora_enter_config(void);                                                         /* ATK-MW1278D模块进入配置模式 */
void lora_exit_config(void);                                                          /* ATK-MW1278D模块进退出置模式 */
uint8_t lora_free(void);                                                              /* 判断ATK-MW1278D模块是否空闲 */
uint8_t lora_send_at_cmd(char *cmd, char *ack, uint32_t timeout);                     /* 向ATK-MW1278D模块发送AT指令 */
uint8_t lora_at_test(void);                                                           /* ATK-MW1278D模块AT指令测试 */
uint8_t lora_echo_config(lora_enable_t enable);                                       /* ATK-MW1278D模块指令回显配置 */
uint8_t lora_sw_reset(void);                                                          /* ATK-MW1278D模块软件复位 */
uint8_t lora_flash_config(lora_enable_t enable);                                      /* ATK-MW1278D模块参数保存配置 */
uint8_t lora_default(void);                                                           /* ATK-MW1278D模块恢复出厂配置 */
uint8_t lora_addr_config(uint16_t addr);                                              /* ATK-MW1278D模块设备地址配置 */
uint8_t lora_tpower_config(lora_tpower_t tpower);                                     /* ATK-MW1278D模块发射功率配置 */
uint8_t lora_workmode_config(lora_workmode_t workmode);                               /* ATK-MW1278D模块工作模式配置 */
uint8_t lora_tmode_config(lora_tmode_t tmode);                                        /* ATK-MW1278D模块发送模式配置 */
uint8_t lora_wlrate_channel_config(lora_wlrate_t wlrate, uint8_t channel);            /* ATK-MW1278D模块空中速率和信道配置 */
uint8_t lora_wltime_config(lora_wltime_t wltime);                                     /* ATK-MW1278D模块休眠时间配置 */
uint8_t lora_uart_config(lora_uartrate_t baudrate, lora_uartpari_t parity);           /* ATK-MW1278D模块串口配置 */

#endif // __LORA_H
