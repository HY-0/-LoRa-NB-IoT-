#include "jw01_usart.h"

static TIM_TypeDef *g_tim_handle = JW01_TIM_INTERFACE;
static USART_TypeDef *g_uart_handle = JW01_UART_INTERFACE;

static struct {
    uint8_t buf[JW01_UART_RX_BUF_SIZE];
    struct {
        uint16_t len   : 15;
        uint16_t finsh : 1;
    } sta;
} g_uart_rx_frame = {0};

void jw01_uart_rx_restart(void)
{
    g_uart_rx_frame.sta.len   = 0;
    g_uart_rx_frame.sta.finsh = 0;
}

uint8_t *jw01_uart_rx_get_frame(void)
{
    if (g_uart_rx_frame.sta.finsh == 1) {
        g_uart_rx_frame.buf[g_uart_rx_frame.sta.len] = '\0';
        return g_uart_rx_frame.buf;
    }
    return NULL;
}

uint16_t jw01_uart_rx_get_frame_len(void)
{
    if (g_uart_rx_frame.sta.finsh == 1)
        return g_uart_rx_frame.sta.len;
    return 0;
}

/* 定时器 MSP 初始化 */
static void jw01_timer_msp_init(void)
{
    JW01_TIM_CLK_ENABLE();

    NVIC_InitTypeDef nvic;
    nvic.NVIC_IRQChannel                   = JW01_TIM_IRQn;
    nvic.NVIC_IRQChannelPreemptionPriority  = 1;
    nvic.NVIC_IRQChannelSubPriority        = 0;
    nvic.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&nvic);
}

void jw01_timer_init(void)
{
    jw01_timer_msp_init();

    TIM_TimeBaseInitTypeDef tim;
    TIM_TimeBaseStructInit(&tim);
    tim.TIM_Prescaler     = JW01_TIM_PRESCALER - 1;
    tim.TIM_CounterMode   = TIM_CounterMode_Up;
    tim.TIM_Period        = 100 - 1;          /* 10ms  */
    tim.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInit(g_tim_handle, &tim);

    TIM_ITConfig(g_tim_handle, TIM_IT_Update, ENABLE);
}

void jw01_uart_init(uint32_t baudrate)
{
    GPIO_InitTypeDef  gpio;
    USART_InitTypeDef usart;
    NVIC_InitTypeDef  nvic;

    /* 使能时钟 */
    JW01_UART_CLK_ENABLE();
    JW01_UART_TX_GPIO_CLK_ENABLE();
    JW01_UART_RX_GPIO_CLK_ENABLE();

    /* TX 引脚：复用推挽输出 */
    gpio.GPIO_Pin   = JW01_UART_TX_GPIO_PIN;
    gpio.GPIO_Mode  = GPIO_Mode_AF_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(JW01_UART_TX_GPIO_PORT, &gpio);

    /* RX 引脚：浮空输入 */
    gpio.GPIO_Pin   = JW01_UART_RX_GPIO_PIN;
    gpio.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
    GPIO_Init(JW01_UART_RX_GPIO_PORT, &gpio);

    /* USART 配置 */
    USART_StructInit(&usart);
    usart.USART_BaudRate            = baudrate;
    usart.USART_WordLength          = USART_WordLength_8b;
    usart.USART_StopBits            = USART_StopBits_1;
    usart.USART_Parity              = USART_Parity_No;
    usart.USART_Mode                = USART_Mode_Rx | USART_Mode_Tx;
    usart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_Init(g_uart_handle, &usart);

    /* 使能 RXNE 中断 */
    USART_ITConfig(g_uart_handle, USART_IT_RXNE, ENABLE);

    /* 中断优先级 */
    nvic.NVIC_IRQChannel                   = JW01_UART_IRQn;
    nvic.NVIC_IRQChannelPreemptionPriority = 1;   /* 可根据系统调整 */
    nvic.NVIC_IRQChannelSubPriority        = 0;
    nvic.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&nvic);

    USART_Cmd(g_uart_handle, ENABLE);

    jw01_timer_init();
}

/* UART 中断处理 */
void JW01_UART_IRQHandler(void)
{
    uint8_t tmp;

    if (USART_GetFlagStatus(g_uart_handle, USART_FLAG_ORE) != RESET) {
        USART_ClearFlag(g_uart_handle, USART_FLAG_ORE);
        (void)USART_ReceiveData(g_uart_handle);
    }

    if (USART_GetITStatus(g_uart_handle, USART_IT_RXNE) != RESET) {
        tmp = USART_ReceiveData(g_uart_handle);

        if (g_uart_rx_frame.sta.len < JW01_UART_RX_BUF_SIZE - 1) {
            TIM_SetCounter(g_tim_handle, 0);
            if (g_uart_rx_frame.sta.len == 0)
                TIM_Cmd(g_tim_handle, ENABLE);
            g_uart_rx_frame.buf[g_uart_rx_frame.sta.len] = tmp;
            g_uart_rx_frame.sta.len++;
        } else {
            /* 溢出，重置 */
            g_uart_rx_frame.sta.len = 0;
            g_uart_rx_frame.buf[g_uart_rx_frame.sta.len] = tmp;
            g_uart_rx_frame.sta.len++;
        }
        USART_ClearITPendingBit(g_uart_handle, USART_IT_RXNE);
    }
}

/* 定时器中断处理（帧完成标志） */
void JW01_TIM_IRQHandler(void)
{
    if (TIM_GetITStatus(g_tim_handle, TIM_IT_Update) != RESET) {
        TIM_ClearITPendingBit(g_tim_handle, TIM_IT_Update);
        g_uart_rx_frame.sta.finsh = 1;
        TIM_Cmd(g_tim_handle, DISABLE);
    }
}
