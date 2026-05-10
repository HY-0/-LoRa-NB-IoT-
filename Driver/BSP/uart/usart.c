#include "usart.h"


/**
 * @brief       使用串口发送一个字节的数据
 * @param       USARTx: 串口名称，如USART1, USART2, USART3 ...
 * @param       Data: 要发送的数据
 */
void usart_send_byte(USART_TypeDef *USARTx, const uint8_t Data)
{
	usart_send_bytes(USARTx, &Data, 1);
}

/**
 * @brief       使用串口发送多个字节的数据
 * @param       USARTx: 串口名称，如USART1, USART2, USART3 ...
 * @param       pData: 要发送的数据（数组）
 * @param       Size: 要发送数据的数量，单位是字节
 */
__weak void usart_send_bytes(USART_TypeDef *USARTx, const uint8_t *pData, uint16_t Size)
{
	if(Size == 0) return;
	
	for(uint16_t i=0; i < Size; i++)
	{
		while(USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET);
		
		USART_SendData(USARTx, pData[i]);
	}
	
	while(USART_GetFlagStatus(USARTx, USART_FLAG_TC) == RESET);
}

/**
 * @brief       通过串口发送一个字符
 * @param       USARTx: 串口名称，如USART1, USART2, USART3 ...
 * @param       C: 要发送的字符
 */
void usart_send_char(USART_TypeDef *USARTx, const char C)
{
	usart_send_bytes(USARTx, (const uint8_t *)&C, 1);
}

/**
 * @brief       通过串口发送字符串
 * @param       USARTx: 串口名称，如USART1, USART2, USART3 ...
 * @param       Str: 要发送的字符串
 */
void usart_send_string(USART_TypeDef *USARTx, const char *Str)
{
	usart_send_bytes(USARTx, (const uint8_t *)Str, strlen(Str));
}

/**
 * @brief       通过串口格式化打印字符串
 * @param       USARTx: 串口名称，如USART1, USART2, USART3 ...
 * @param       Format: 字符串的格式
 * @param       ...: 可变参数
 */
void usart_printf(USART_TypeDef *USARTx, const char *Format, ...)
{
	char format_buffer[512];
	va_list argptr;
	
	__va_start(argptr, Format);
	
	vsnprintf(format_buffer, sizeof(format_buffer), Format, argptr);
	
	__va_end(argptr);
	
	usart_send_string(USARTx, format_buffer);
}


/**
 * @brief       通过串口读取一字节的数据
 * @param       USARTx: 串口名称，如USART1, USART2, USART3 ...
 * @retval      读取到的字节
 */
uint8_t usart_receive_byte(USART_TypeDef *USARTx)
{
	while(USART_GetFlagStatus(USARTx, USART_FLAG_RXNE) == RESET);
	
	return USART_ReceiveData(USARTx);
}

/**
 * @brief       通过串口读取多个字节的数据
 * @param       USARTx: 串口名称，如USART1, USART2, USART3 ...
 * @param       pDataOut: 输出参数，读取到的数据将输出到此数组当中
 * @param       Size: 需要读取的字节数量
 * @param       Timeout: 超时时间，单位是毫秒，负数表示无限长。如果超时时间内没有读取完成则返回。
 * @retval      实际读取到的数据数量
 */
__weak uint16_t usart_receive_bytes(USART_TypeDef *USARTx, uint8_t *pDataOut, uint16_t Size, int Timeout)
{
	uint32_t expireTime;
	
	systick_init();
	
	if(Timeout >= 0)
	{
		expireTime = get_ms() + Timeout; // 计算过期时间，过期时间 = 当前时间+Timeout
	}
	
	uint16_t i = 0;
	
	do
	{
		if(USART_GetFlagStatus(USARTx, USART_FLAG_RXNE) == SET)
		{
			pDataOut[i++] = USART_ReceiveData(USARTx);
			
			if(i==Size) break;
		}
	}
	while(Timeout < 0 || get_us() < expireTime); // 判断是否超时
	
	return i;
}


/**
 * @brief       通过串口读取一行字符串
 * @param       USARTx: 串口名称，如USART1, USART2, USART3 ...
 * @param       pStrOut: 输出参数，读取到的数据将输出到此数组当中
 * @param       MaxLength: 字符串的最大长度
 * @param       LineSeperator: 行分隔符 LINE_SEPERATOR_CR - 回车 \r
 *                                         LINE_SEPERATOR_LF - 换行 \n
 *                                         LINE_SEPERATOR_CRLF - 回车+换行 \r\n
 * @param       Timeout: 超时时间，单位是毫秒，负数表示无限长。如果超时时间内没有读取完成则返回
 * @retval      0: 成功读到一行字符串
 * @retval      -1: 超时（Timeout内未读到一行完整的字符串）
 * @retval      -2: 超过字符串的最大长度（字符串的最大长度用MaxLength参数设置）
 */
int usart_receive_line(USART_TypeDef *USARTx, char *pStrOut, uint16_t MaxLength, uint16_t LineSeperator, int Timeout)
{
	// 如果最大长度都不足以装下行分隔符
	// 就直接返回失败
	if(MaxLength < 2 || ((LineSeperator == LINE_SEPERATOR_CRLF) && (MaxLength < 1)))
	{
		return -2;
	}
	
	int ret = -1;
	uint32_t expireTime;
	
	systick_init(); // 要用到单片机当前时间，所以初始化延迟函数
	
	if(Timeout >= 0)
	{
		expireTime = get_ms() + Timeout; // 计算过期时间，过期时间 = 当前时间+Timeout
	}
	
	uint16_t i = 0;
	
	do
	{
		if(USART_GetFlagStatus(USARTx, USART_FLAG_RXNE) == SET)
		{
			char c = (char)USART_ReceiveData(USARTx);
			pStrOut[i++] = c;
			
			if(LineSeperator == LINE_SEPERATOR_CR && c == '\r') // \r
			{
				ret = 0;
				break;
			}
			else if(LineSeperator == LINE_SEPERATOR_LF && c == '\n') // \n
			{
				ret = 0;
				break;
			}
			else if(i >= 2 && pStrOut[i-2] == '\r' && c == '\n') // \r\n
			{
				ret = 0;
				break;
			}
			
			if(i == MaxLength) // 超过最大长度
			{
				ret = -2;
				break;
			}
		}
	}
	while(Timeout < 0 || get_ms() < expireTime); // 判断是否超时
	
	// 在字符串末尾增加'\0'
	if(i == MaxLength)
	{
		pStrOut[i-1] = '\0';
	}
	else
	{
		pStrOut[i] = '\0';
	}
	
	return ret;
}
