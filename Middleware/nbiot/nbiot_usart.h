#ifndef __NBIOT_USART_H
#define __NBIOT_USART_H

#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include "stm32f10x.h"
#include "usart.h"          /* 标准库串口驱动（包含中断处理） */
#include "delay.h"          /* 用于超时计数 */

/* 引脚定义 */
#define NBIOT_UART_TX_GPIO_PORT           GPIOA
#define NBIOT_UART_TX_GPIO_PIN            GPIO_Pin_2
#define NBIOT_UART_TX_GPIO_CLK_ENABLE()   do{ RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE); }while(0)

#define NBIOT_UART_RX_GPIO_PORT           GPIOA
#define NBIOT_UART_RX_GPIO_PIN            GPIO_Pin_3
#define NBIOT_UART_RX_GPIO_CLK_ENABLE()   do{ RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE); }while(0)

/* 串口外设及中断 */
#define NBIOT_UART_INTERFACE              USART2
#define NBIOT_UART_IRQn                   USART2_IRQn
#define NBIOT_UART_IRQHandler             USART2_IRQHandler
#define NBIOT_UART_CLK_ENABLE()           do{ RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE); }while(0)

/* 可选：硬件定时器（用于串口空闲超时） */
#define NBIOT_TIM_INTERFACE               TIM3
#define NBIOT_TIM_IRQn                    TIM3_IRQn
#define NBIOT_TIM_IRQHandler              TIM3_IRQHandler
#define NBIOT_TIM_CLK_ENABLE()            do{ RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); }while(0)
#define NBIOT_TIM_PRESCALER               7200   /* 根据系统时钟调整，用于产生1ms基准 */

/* ===================== 串口缓冲宏定义 ===================== */
#define NBIOT_UART_RX_BUF_SIZE            512    /* 串口接收缓冲大小（原Buf2_Max） */
#define NBIOT_UART_TX_BUF_SIZE            128
#define NBIOT_LINE_BUF_LEN                200    /* 行缓冲长度（原BUF_LEN） */
#define NBIOT_DEBUG_BUF_LEN               128    /* 调试缓冲长度（原LING_BUF_LEN） */

// /* 兼容原有命名（可选，若需保持旧代码） */
// #define Buf2_Max      NBIOT_UART_RX_BUF_SIZE
// #define BUF_LEN       NBIOT_LINE_BUF_LEN
// #define LING_BUF_LEN  NBIOT_DEBUG_BUF_LEN

// /* ===================== 数据结构 ===================== */
// /* 行解析结构 */
// typedef struct parse_buffer {
//     char buf[NBIOT_LINE_BUF_LEN];   /* 存放一行数据 */
//     int pos;                        /* 当前填充位置 */
// } paser_buf_t;

// /* ===================== 函数声明 ===================== */
// /* 串口初始化 */
    void nbiot_uart_init(uint32_t baudrate);          /* 初始化UART（配置GPIO、时钟、中断） */
    void nbiot_uart_printf(char *fmt, ...);           /* 格式化发送数据（类似printf） */

// /* 接收缓冲管理 */
    void nbiot_uart_rx_restart(void);                 /* 重新开始接收数据（清空缓冲） */
uint8_t *nbiot_uart_rx_get_frame(void);           /* 获取当前接收的一帧数据 */
uint16_t nbiot_uart_rx_get_frame_len(void);       /* 获取帧长度 */
// void nbiot_uart_clear_rx_buf(void);               /* 清除接收缓冲（原clear_uart2_buf） */
// void nbiot_uart_clear_rx_buf_accord_pos(void);    /* 根据pos清除缓冲（原clear_uart2_buf_accord_pos） */

// /* 行数据解析接口 */
// int nbiot_uart_try_read_line(char *buf, int len); /* 尝试读取一行（原try_read_line） */

// /* 调试缓冲（可选，用于日志输出） */
// void nbiot_debug_clear(void);                     /* 清除调试缓冲（原clear_debug_buf） */
// void nbiot_debug_add_char(unsigned char c);       /* 添加字符到调试缓冲（原add_char_to_debug_buf） */
// char *nbiot_debug_get_buf(void);                  /* 获取调试缓冲内容（原get_debug_buf） */

// /* 原始缓冲操作（如需保留兼容性） */
// void nbiot_uart_add_char_to_rx_buf(unsigned char c);  /* 原add_char_to_uart2_buf */
// int nbiot_uart_get_rx_buf_pos(void);                  /* 原get_uart2_buf_pos */
// void nbiot_uart_show_rx_buf_pos(void);                /* 原show_read_buf_pos */
// int nbiot_uart_find_str_from_rx_buf(char *str1, char *str2);  /* 原find_str_from_uart2_buf */
// int nbiot_uart_rx_buf_end_with_crcn(void);            /* 原uart2_buf_find_crcn */

// /* 其它辅助函数 */
// int nbiot_find_str(char *a);                          /* 原find_str（可能在调试缓冲中查找） */

#endif /* __NBIOT_USART_H */
