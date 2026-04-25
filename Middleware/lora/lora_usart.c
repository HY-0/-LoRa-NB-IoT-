#include "lora_usart.h"


static TIM_TypeDef *g_tim_handle = LORA_TIM_INTERFACE;                      /* LoRa Timer */
static USART_TypeDef *g_uart_handle = LORA_UART_INTERFACE;                  /* LoRa UART */

// 驱动层自定义类型尽量私有化，若应用层用到可以在应用层头文件typedef
static struct
{
    uint8_t buf[LORA_UART_RX_BUF_SIZE];                     /* 帧接收缓冲 */
    struct
    {
        uint16_t len    : 15;                               /* 帧接收长度，sta[14:0] */
        uint16_t finsh  : 1;                                /* 帧接收完成标志，sta[15] */
    } sta;                                                  /* 帧状态信息 */
} g_uart_rx_frame = {0};                                    /* LoRa UART接收帧缓冲信息结构体 */
static uint8_t g_uart_tx_buf[LORA_UART_TX_BUF_SIZE];        /* LoRa UART发送缓冲 */

// /**
//  * @brief       LoRa UART printf
//  * @param       fmt: 待打印的数据
//  * @retval      无
//  */
// void lora_uart_printf(char *fmt, ...)
// {
//     va_list ap;
//     uint16_t len;
    
//     va_start(ap, fmt);
//     vsprintf((char *)g_uart_tx_buf, fmt, ap);
//     va_end(ap);
    
//     len = strlen((const char *)g_uart_tx_buf);
//     usart_send_bytes(g_uart_handle, g_uart_tx_buf, len);
// }

uint8_t lora_uart_printf(char *fmt, ...)
{
    va_list ap;
    int16_t len;
    
    va_start(ap, fmt);
    len = vsnprintf((char *)g_uart_tx_buf, LORA_UART_TX_BUF_SIZE, fmt, ap);
    va_end(ap);
    
    if(len < 0)
    {
        return LORA_PRINTF_ERR_FORMAT;
    }

    if(len >= LORA_UART_TX_BUF_SIZE)
    {
        len = LORA_UART_TX_BUF_SIZE - 1;  // 发送截断内容

        usart_send_bytes(g_uart_handle, g_uart_tx_buf, len);  /* 使用你的发送函数 */

        return LORA_PRINTF_ERR_TRUNCATED;
    }

    usart_send_bytes(g_uart_handle, g_uart_tx_buf, len);  /* 使用你的发送函数 */

    return LORA_PRINTF_OK;
}

/**
 * @brief       LoRa UART重新开始接收数据
 * @param       无
 * @retval      无
 */
void lora_uart_rx_restart(void)
{
    g_uart_rx_frame.sta.len     = 0;
    g_uart_rx_frame.sta.finsh   = 0;
}

/**
 * @brief       获取LoRa UART接收到的一帧数据
 * @param       无
 * @retval      NULL: 未接收到一帧数据
 *              其他: 接收到的一帧数据
 */
uint8_t *lora_uart_rx_get_frame(void)
{
    if (g_uart_rx_frame.sta.finsh == 1)
    {
        g_uart_rx_frame.buf[g_uart_rx_frame.sta.len] = '\0';
        return g_uart_rx_frame.buf;
    }
    else
    {
        return NULL;
    }
}

/**
 * @brief       获取LoRa UART接收到的一帧数据的长度
 * @param       无
 * @retval      NULL   : 未接收到一帧数据
 *              其他: 接收到的一帧数据的长度
 */
uint16_t lora_uart_rx_get_frame_len(void)
{
    if (g_uart_rx_frame.sta.finsh == 1)
    {
        return g_uart_rx_frame.sta.len;
    }
    else
    {
        return 0;
    }
}

/**
 * @brief       LoRa Timer MSP初始化
 * @param       无
 * @retval      无
 */
static void lora_timer_msp_init(void)
{
    /* 使能定时器时钟 */
    LORA_TIM_CLK_ENABLE();

    /* 配置定时器中断 */
    NVIC_InitTypeDef nvic;
    nvic.NVIC_IRQChannel                    = LORA_TIM_IRQn;
    nvic.NVIC_IRQChannelPreemptionPriority  = 1;   /* 低于UART抢占优先级（UART为0） */
    nvic.NVIC_IRQChannelSubPriority         = 0;
    nvic.NVIC_IRQChannelCmd                 = ENABLE;
    NVIC_Init(&nvic);
}

/**
 * @brief       LoRa Timer初始化MSP回调函数
 * @param       htim: Timer句柄指针
 * @retval      无
 */
void lora_timer_init(void)
{
    /* 底层 MSP（时钟、中断） */
    lora_timer_msp_init();

    /* 定时器时基配置 */
    TIM_TimeBaseInitTypeDef tim;
    TIM_TimeBaseStructInit(&tim);
    tim.TIM_Prescaler     = LORA_TIM_PRESCALER - 1;  /* 如 7200-1 */
    tim.TIM_CounterMode   = TIM_CounterMode_Up;
    tim.TIM_Period        = 100 - 1;                        /* 自动重载值，产生 10ms 中断（10kHz * 100） */
    tim.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInit(g_tim_handle, &tim);

    /* 使能更新中断（但定时器默认未启动，需在收到第一个字节时启动） */
    TIM_ITConfig(g_tim_handle, TIM_IT_Update, ENABLE);
}

/**
 * @brief       LoRa UART初始化
 * @param       baudrate: UART通讯波特率
 * @retval      无
 */
void lora_uart_init(uint32_t baudrate)
{
    GPIO_InitTypeDef gpio;
    USART_InitTypeDef usart;
    NVIC_InitTypeDef nvic;

    /* 使能时钟 lora */
    LORA_UART_CLK_ENABLE();
    LORA_UART_TX_GPIO_CLK_ENABLE();
    LORA_UART_RX_GPIO_CLK_ENABLE();

    /* TX 引脚：复用推挽输出 */
    gpio.GPIO_Pin   = LORA_UART_TX_GPIO_PIN;
    gpio.GPIO_Mode  = GPIO_Mode_AF_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(LORA_UART_TX_GPIO_PORT, &gpio);

    /* RX 引脚：浮空输入 */
    gpio.GPIO_Pin   = LORA_UART_RX_GPIO_PIN;
    gpio.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
    GPIO_Init(LORA_UART_RX_GPIO_PORT, &gpio);

    /* USART 配置 */
    USART_StructInit(&usart);
    usart.USART_BaudRate   = baudrate;
    usart.USART_WordLength = USART_WordLength_8b;
    usart.USART_StopBits   = USART_StopBits_1;
    usart.USART_Parity     = USART_Parity_No;
    usart.USART_Mode       = USART_Mode_Tx | USART_Mode_Rx;
    usart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_Init(g_uart_handle, &usart);

    /* 使能 RXNE 中断 */
    USART_ITConfig(g_uart_handle, USART_IT_RXNE, ENABLE);

    /* 配置中断优先级 */
    nvic.NVIC_IRQChannel                   = LORA_UART_IRQn;
    nvic.NVIC_IRQChannelPreemptionPriority = 0;
    nvic.NVIC_IRQChannelSubPriority        = 0;
    nvic.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&nvic);

    /* 最后使能 USART */
    USART_Cmd(g_uart_handle, ENABLE);

    /* 定时器初始化 */
    lora_timer_init();
}

/**
 * @brief       LoRa UART中断回调函数
 * @param       无
 * @retval      无
 */
void LORA_UART_IRQHandler(void)
{
    uint8_t tmp;

   /* 处理溢出错误 */
   if (USART_GetFlagStatus(g_uart_handle, USART_FLAG_ORE) != RESET) 
    {
        USART_ClearFlag(g_uart_handle, USART_FLAG_ORE);
        (void)USART_ReceiveData(g_uart_handle);  /* 读 DR 清除 ORE */
    }

    if (USART_GetITStatus(g_uart_handle, USART_IT_RXNE) != RESET) 
    {
        tmp = USART_ReceiveData(g_uart_handle);

        if (g_uart_rx_frame.sta.len < LORA_UART_RX_BUF_SIZE - 1) 
        {
            TIM_SetCounter(g_tim_handle, 0);                 /* 重置定时器 */
            if (g_uart_rx_frame.sta.len == 0) 
            {
                TIM_Cmd(g_tim_handle, ENABLE);                /* 开启定时器 */
            }
            g_uart_rx_frame.buf[g_uart_rx_frame.sta.len] = tmp;
            g_uart_rx_frame.sta.len++;
        } 
        else 
        {
            /* 缓冲溢出，重新开始 */
            g_uart_rx_frame.sta.len = 0;
            g_uart_rx_frame.buf[g_uart_rx_frame.sta.len] = tmp;
            g_uart_rx_frame.sta.len++;
        }
        USART_ClearITPendingBit(g_uart_handle, USART_IT_RXNE);
    }
}

/**
 * @brief       LoRa Timer中断回调函数
 * @param       无
 * @retval      无
 */
void LORA_TIM_IRQHandler(void)
{
    if (TIM_GetITStatus(g_tim_handle, TIM_IT_Update) != RESET) 
    {
        TIM_ClearITPendingBit(g_tim_handle, TIM_IT_Update);
        g_uart_rx_frame.sta.finsh = 1;              /* 标记帧接收完成 */
        TIM_Cmd(g_tim_handle, DISABLE);             /* 停止定时器 */
    }
}
