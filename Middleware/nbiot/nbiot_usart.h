#ifndef __NBIOT_USART_H
#define __NBIOT_USART_H

#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include "stm32f10x.h"
#include "usart.h"          /* 标准库串口驱动（包含中断处理） */
#include "delay.h"          /* 用于超时计数 */

/* 引脚定义 */
#define NBIOT_UART_TX_GPIO_PORT           GPIOB 
#define NBIOT_UART_TX_GPIO_PIN            GPIO_Pin_10
#define NBIOT_UART_TX_GPIO_CLK_ENABLE()   do{ RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE); }while(0)

#define NBIOT_UART_RX_GPIO_PORT           GPIOB
#define NBIOT_UART_RX_GPIO_PIN            GPIO_Pin_11
#define NBIOT_UART_RX_GPIO_CLK_ENABLE()   do{ RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE); }while(0)

/* 串口外设及中断 */
#define NBIOT_UART_INTERFACE              USART3
#define NBIOT_UART_IRQn                   USART3_IRQn
#define NBIOT_UART_IRQHandler             USART3_IRQHandler
#define NBIOT_UART_CLK_ENABLE()           do{ RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE); }while(0)
#define NBIOT_UART_DEINIT()               do{ USART_DeInit(NBIOT_UART_INTERFACE); }while(0)   /* 复位 USART3 至默认状态 */
/* 硬件定时器 */
#define NBIOT_TIM_INTERFACE               TIM3
#define NBIOT_TIM_IRQn                    TIM3_IRQn
#define NBIOT_TIM_IRQHandler              TIM3_IRQHandler
#define NBIOT_TIM_CLK_ENABLE()            do{ RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); }while(0)
#define NBIOT_TIM_PRESCALER               7200   /* 根据系统时钟调整，用于产生1ms基准 */

/* ===================== 串口缓冲宏定义 ===================== */
#define NBIOT_UART_RX_BUF_SIZE            512    /* 串口接收缓冲大小 */
#define NBIOT_UART_TX_BUF_SIZE            512

/* ===================== 函数声明 ===================== */
/* 串口初始化 */
void nbiot_uart_init(uint32_t baudrate);          /* 初始化UART（配置GPIO、时钟、中断） */
void nbiot_uart_printf(char *fmt, ...);           /* 格式化发送数据（类似printf） */

/* 接收缓冲管理 */
    void nbiot_uart_rx_restart(void);             /* 重新开始接收数据（清空缓冲） */
uint8_t *nbiot_uart_rx_get_frame(void);           /* 获取当前接收的一帧数据 */
uint16_t nbiot_uart_rx_get_frame_len(void);       /* 获取帧长度 */

#endif /* __NBIOT_USART_H */
