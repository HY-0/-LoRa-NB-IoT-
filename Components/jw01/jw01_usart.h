#ifndef __JW01_USART_H
#define __JW01_USART_H

#include "stm32f10x.h"
#include "usart.h"
#include "delay.h"

/* 引脚定义 */
#define JW01_UART_TX_GPIO_PORT           GPIOA
#define JW01_UART_TX_GPIO_PIN            GPIO_Pin_2
#define JW01_UART_TX_GPIO_CLK_ENABLE()   do{ RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE); }while(0)

#define JW01_UART_RX_GPIO_PORT           GPIOA
#define JW01_UART_RX_GPIO_PIN            GPIO_Pin_3
#define JW01_UART_RX_GPIO_CLK_ENABLE()   do{ RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE); }while(0)

#define JW01_UART_INTERFACE              USART2         /* 通常 JW01 接 USART3，视硬件图 */
#define JW01_UART_IRQn                   USART2_IRQn
#define JW01_UART_IRQHandler             USART2_IRQHandler
#define JW01_UART_CLK_ENABLE()           do{ RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE); }while(0)

/* 定时器用于帧超时 */
#define JW01_TIM_INTERFACE               TIM2
#define JW01_TIM_IRQn                    TIM2_IRQn
#define JW01_TIM_IRQHandler              TIM2_IRQHandler
#define JW01_TIM_CLK_ENABLE()            do{ RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE); }while(0)
#define JW01_TIM_PRESCALER               7200   /* 按72MHz主频，分频7200得10kHz，计数值100得10ms */

#define JW01_UART_RX_BUF_SIZE            128

/* 错误码，保留与上层一致 */
#define JW01_EOK                         0
#define JW01_ERROR                       1
#define JW01_ETIMEOUT                    2
#define JW01_EINVAL                      3

void jw01_uart_init(uint32_t baudrate);
void jw01_uart_rx_restart(void);
uint8_t *jw01_uart_rx_get_frame(void);
uint16_t jw01_uart_rx_get_frame_len(void);

#endif
