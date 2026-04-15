#ifndef __LORA_UART_H
#define __LORA_UART_H

#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include "usart.h"          // 你的标准库串口驱动
#include "delay.h"          // 你的延时函数（用于获取 tick）
#include "sys.h"

/* 引脚定义 */
#define LORA_UART_TX_GPIO_PORT           GPIOA
#define LORA_UART_TX_GPIO_PIN            GPIO_Pin_9
#define LORA_UART_TX_GPIO_CLK_ENABLE()   do{ RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE); }while(0)

#define LORA_UART_RX_GPIO_PORT           GPIOA
#define LORA_UART_RX_GPIO_PIN            GPIO_Pin_10
#define LORA_UART_RX_GPIO_CLK_ENABLE()   do{ RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE); }while(0)

#define LORA_UART_INTERFACE              USART1
#define LORA_UART_IRQn                   USART1_IRQn
#define LORA_UART_IRQHandler             USART1_IRQHandler
#define LORA_UART_CLK_ENABLE()           do{ RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE); }while(0)


#define LORA_TIM_INTERFACE               TIM2
#define LORA_TIM_IRQn                    TIM2_IRQn
#define LORA_TIM_IRQHandler              TIM2_IRQHandler
#define LORA_TIM_CLK_ENABLE()            do{ RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE); }while(0)
#define LORA_TIM_PRESCALER               7200   // 可根据时钟频率重新计算

/* UART收发缓冲大小 */
#define LORA_UART_RX_BUF_SIZE            128
#define LORA_UART_TX_BUF_SIZE            128

/* 错误代码 */
#define LORA_EOK             0       /* 没有错误 */
#define LORA_ERROR           1       /* 通用错误 */
#define LORA_ETIMEOUT        2       /* 超时错误 */
#define LORA_EINVAL          3       /* 参数错误 */
#define LORA_EBUSY           4       /* 忙错误 */

/* 操作函数 */
void lora_uart_printf(char *fmt, ...);       /* ATK-MW1278D UART printf */
void lora_uart_rx_restart(void);             /* ATK-MW1278D UART重新开始接收数据 */
uint8_t *lora_uart_rx_get_frame(void);       /* 获取lora UART接收到的一帧数据 */
uint16_t lora_uart_rx_get_frame_len(void);   /* 获取lora UART接收到的一帧数据的长度 */
void lora_uart_init(uint32_t baudrate);      /* ATK-MW1278D UART初始化 */

#endif
