#ifndef _USART_H_
#define _USART_H_

#include "stm32f10x.h"
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include "delay.h"

#define LINE_SEPERATOR_CR   0x00 // 回车 \r
#define LINE_SEPERATOR_LF   0x01 // 换行 \n
#define LINE_SEPERATOR_CRLF 0x02 // 回车+换行 \r\n

void usart_send_byte(USART_TypeDef *USARTx, const uint8_t Data);
void usart_send_bytes(USART_TypeDef *USARTx, const uint8_t *pData, uint16_t Size);
void usart_send_char(USART_TypeDef *USARTx, const char C);
void usart_send_string(USART_TypeDef *USARTx, const char *Str);
void usart_printf(USART_TypeDef *USARTx, const char *Format, ...);

 uint8_t usart_receive_byte(USART_TypeDef *USARTx);
uint16_t usart_receive_bytes(USART_TypeDef *USARTx, uint8_t *pDataOut, uint16_t Size, int Timeout);
     int usart_receive_line(USART_TypeDef *USARTx, char *pStrOut, uint16_t MaxLength, uint16_t LineSeperator, int Timeout);

#endif

