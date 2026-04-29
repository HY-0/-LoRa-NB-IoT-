#include "nbiot_usart.h"

static TIM_TypeDef *g_tim_handle = NBIOT_TIM_INTERFACE;     /* ATK-MW1278D Timer */
static USART_TypeDef *g_uart_handle = NBIOT_UART_INTERFACE; /* ATK-MW1278D UART */

static struct
{
    uint8_t buf[NBIOT_UART_RX_BUF_SIZE]; /* 帧接收缓冲 */
    struct
    {
        uint16_t len : 15;  /* 帧接收长度，sta[14:0] */
        uint16_t finsh : 1; /* 帧接收完成标志，sta[15] */
    } sta;                  /* 帧状态信息 */
} g_uart_rx_frame = {0};    /* ATK-MW1278D UART接收帧缓冲信息结构体 */
static uint8_t g_uart_tx_buf[NBIOT_UART_TX_BUF_SIZE];

/**
 * @brief       ATK-MW1278D UART printf
 * @param       fmt: 待打印的数据
 * @retval      无
 */
void nbiot_uart_printf(char *fmt, ...)
{
    va_list ap;
    uint16_t len;

    va_start(ap, fmt);
    vsprintf((char *)g_uart_tx_buf, fmt, ap);
    va_end(ap);

    len = strlen((const char *)g_uart_tx_buf);
    usart_send_bytes(g_uart_handle, g_uart_tx_buf, len); // 使用你的发送函数
}

void UART2_SendString(char *s)
{
    nbiot_uart_printf("%s", s);
}

void UART2_Send_Command(char *s)
{
    nbiot_uart_printf("%s\r\n", s);
}

/**
 * @brief       ATK-MW1278D UART重新开始接收数据
 * @param       无
 * @retval      无
 */
void nbiot_uart_rx_restart(void)
{
    g_uart_rx_frame.sta.len = 0;
    g_uart_rx_frame.sta.finsh = 0;
}

/**
 * @brief       获取NB-IoT UART接收到的一帧数据
 * @param       无
 * @retval      NULL: 未接收到一帧数据
 *              其他: 接收到的一帧数据
 */
uint8_t *nbiot_uart_rx_get_frame(void)
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
 * @brief       获取ATK-MW1278D UART接收到的一帧数据的长度
 * @param       无
 * @retval      0   : 未接收到一帧数据
 *              其他: 接收到的一帧数据的长度
 */
uint16_t nbiot_uart_rx_get_frame_len(void)
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
 * @brief       ATK-MW1278D Timer MSP初始化
 * @param       无
 * @retval      无
 */
static void nbiot_timer_msp_init(void)
{
    // 使能定时器时钟
    NBIOT_TIM_CLK_ENABLE();

    // 配置定时器中断
    NVIC_InitTypeDef nvic;
    nvic.NVIC_IRQChannel = NBIOT_TIM_IRQn;
    nvic.NVIC_IRQChannelPreemptionPriority = 1; // 低于UART抢占优先级（UART为0）
    nvic.NVIC_IRQChannelSubPriority = 0;
    nvic.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic);
}

/**
 * @brief       ATK-MW1278D Timer初始化MSP回调函数
 * @param       htim: Timer句柄指针
 * @retval      无
 */
void nbiot_timer_init(void)
{
    // 底层 MSP（时钟、中断）
    nbiot_timer_msp_init();

    // 定时器时基配置
    TIM_TimeBaseInitTypeDef tim;
    TIM_TimeBaseStructInit(&tim);
    tim.TIM_Prescaler = NBIOT_TIM_PRESCALER - 1; // 如 7200-1
    tim.TIM_CounterMode = TIM_CounterMode_Up;
    tim.TIM_Period = 100 - 1; // 自动重载值，产生 10ms 中断（10kHz * 100）
    tim.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInit(g_tim_handle, &tim);

    // 使能更新中断（但定时器默认未启动，需在收到第一个字节时启动）
    TIM_ITConfig(g_tim_handle, TIM_IT_Update, ENABLE);
}

/**
 * @brief       ATK-MW1278D UART初始化
 * @param       baudrate: UART通讯波特率
 * @retval      无
 */
void nbiot_uart_init(uint32_t baudrate)
{
    GPIO_InitTypeDef gpio;
    USART_InitTypeDef usart;
    NVIC_InitTypeDef nvic;

    /*使能 nbiot*/
    NBIOT_UART_CLK_ENABLE();
    NBIOT_UART_TX_GPIO_CLK_ENABLE();
    NBIOT_UART_RX_GPIO_CLK_ENABLE();

    // TX 引脚：复用推挽输出
    gpio.GPIO_Pin = NBIOT_UART_TX_GPIO_PIN;    // USART2_TXD(PA.2)
    gpio.GPIO_Mode = GPIO_Mode_AF_PP;          // 复用推挽输出
    gpio.GPIO_Speed = GPIO_Speed_50MHz;        // 设置引脚输出最大速率为50MHz
    GPIO_Init(NBIOT_UART_TX_GPIO_PORT, &gpio); // 调用库函数中的GPIO初始化函数，初始化USART1_TXD(PA.9)

    // RX 引脚：浮空输入
    gpio.GPIO_Pin = NBIOT_UART_RX_GPIO_PIN;    // USART2_RXD(PA.3)
    gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;    // 浮空输入
    GPIO_Init(NBIOT_UART_RX_GPIO_PORT, &gpio); // 调用库函数中的GPIO初始化函数，初始化USART1_RXD(PA.10)

    // USART 配置
    USART_StructInit(&usart);
    usart.USART_BaudRate = baudrate;
    usart.USART_WordLength = USART_WordLength_8b;
    usart.USART_StopBits = USART_StopBits_1;
    usart.USART_Parity = USART_Parity_No;
    usart.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    usart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_Init(g_uart_handle, &usart); // 初始化串口2

    USART_ITConfig(g_uart_handle, USART_IT_RXNE, ENABLE); // 使能串口2接收中断

    // 配置中断优先级
    nvic.NVIC_IRQChannel = NBIOT_TIM_IRQn;
    nvic.NVIC_IRQChannelPreemptionPriority = 1; // 抢占优先级1
    nvic.NVIC_IRQChannelSubPriority = 0;        // 从优先级0
    nvic.NVIC_IRQChannelCmd = ENABLE;           // IRQ通道使能
    NVIC_Init(&nvic);                           // 根据指定的参数初始化VIC寄存器

    USART_Cmd(g_uart_handle, ENABLE);

    nbiot_timer_init();
}

/**
 * @brief       ATK-MW1278D UART中断回调函数
 * @param       无
 * @retval      无
 */
void NBIOT_UART_IRQHandler(void)
{
    uint8_t tmp;

    // 处理溢出错误
    if (USART_GetFlagStatus(g_uart_handle, USART_FLAG_ORE) != RESET)
    {
        USART_ClearFlag(g_uart_handle, USART_FLAG_ORE);
        (void)USART_ReceiveData(g_uart_handle); // 读 DR 清除 ORE
    }

    if (USART_GetITStatus(g_uart_handle, USART_IT_RXNE) != RESET)
    {
        tmp = USART_ReceiveData(g_uart_handle);

        if (g_uart_rx_frame.sta.len < NBIOT_UART_RX_BUF_SIZE - 1)
        {
            TIM_SetCounter(g_tim_handle, 0); // 重置定时器
            if (g_uart_rx_frame.sta.len == 0)
            {
                TIM_Cmd(g_tim_handle, ENABLE); // 开启定时器
            }
            g_uart_rx_frame.buf[g_uart_rx_frame.sta.len] = tmp;
            g_uart_rx_frame.sta.len++;
        }
        else
        {
            // 缓冲溢出，重新开始
            g_uart_rx_frame.sta.len = 0;
            g_uart_rx_frame.buf[g_uart_rx_frame.sta.len] = tmp;
            g_uart_rx_frame.sta.len++;
        }
        USART_ClearITPendingBit(g_uart_handle, USART_IT_RXNE);
    }
}

/**
 * @brief       ATK-MW1278D Timer中断回调函数
 * @param       无
 * @retval      无
 */
void NBIOT_TIM_IRQHandler(void)
{
    if (TIM_GetITStatus(g_tim_handle, TIM_IT_Update) != RESET)
    {
        TIM_ClearITPendingBit(g_tim_handle, TIM_IT_Update);
        g_uart_rx_frame.sta.finsh = 1;  // 标记帧接收完成
        TIM_Cmd(g_tim_handle, DISABLE); // 停止定时器
    }
}

/*******************************************************************************
 * 函数名 : Second_AT_Command
 * 描述   : 发送AT指令函数
 * 输入   : 发送数据的指针、希望收到的应答、发送等待时间(单位：20ms)
 * 输出   :
 * 返回   :
 * 注意   :
 *******************************************************************************/
extern char cmd_mode;

u8 UART2_Send_AT_Command(char *b, char *a, u8 wait_time, u32 interval_time)
{
    u8 i, ret = 0;

    i = 0;
    while (i < wait_time) // 如果没有找到 就继续再发一次指令 再进行查找目标字符串
    {
        UART2_Send_Command(b);                // 串口2发送 b 字符串 他会自动发送\r\n  相当于发送了一个指令
        delay_ms(interval_time);              // 等待一定时间 传50的话就是 50*20ms = 1秒
        if (find_str_from_uart2_buf(a, NULL)) // 查找需要应答的字符串 a
        {
            printf("*************\r\n");
            ret = 1;
            break;
        }
        printf("#################\r\n");
        i++;
    }
    return ret;
}

u8 UART2_Send_AT_Command_Ext(char *b, char *a, char *aa, u8 wait_time, u32 interval_time)
{
    u8 i, ret = 0;

    i = 0;
    while (i < wait_time) // 如果没有找到 就继续再发一次指令 再进行查找目标字符串
    {
        UART2_Send_Command(b);              // 串口2发送 b 字符串 他会自动发送\r\n  相当于发送了一个指令
        delay_ms(interval_time);            // 等待一定时间 传50的话就是 50*20ms = 1秒
        if (find_str_from_uart2_buf(a, aa)) // 查找需要应答的字符串 a
        {
            printf("*************\r\n");
            ret = 1;
            break;
        }
        i++;
    }
    return ret;
}

void UART2_Send_Command_END(char *s)
{
    clear_uart2_buf();   // 清空接收数据的buffer
    UART2_SendString(s); // 发出字符串
}

u8 UART2_Send_AT_Command_End(char *b, char *a, u8 wait_time, u32 interval_time)
{
    u8 i, ret = 0;
    i = 0;
    while (i < wait_time) // 如果没有找到 就继续再发一次指令 再进行查找目标字符串
    {
        UART2_Send_Command_END(b); // 串口2发送 b 字符串 这里不发送\r\n
        delay_ms(interval_time);   // 等待一定时间 传50的话就是 50*20ms = 1秒
        if (find_str(a))           // 查找需要应答的字符串 a
        {
            ret = 1;
        }
        i++;
    }
    return ret;
}

/*
**把接收改为满循环缓冲区，buf_pos指向写位置，read_pos指向读位置，读完的数据同时被设置成0
**read_pos == buf_pos：缓冲区为空
**read_pos总是指向需要读的位置
**buf_pos总是指向下一个要写入的位置，如果下一个要写的位置等于read_pos那么就认为是满，此时会浪费一个字节
*/
static char uart2_buf[Buf2_Max] = {'\0'}; // 串口2接收缓存
static int buf_pos = 0;
static int read_pos = 0;
static paser_buf_t line_buf;

void clear_debug_buf(void)
{
    memset((char *)&line_buf, 0, sizeof(line_buf));
}

void add_char_to_debug_buf(unsigned char c)
{
    line_buf.buf[line_buf.pos] = c;
    line_buf.pos++;
    if (line_buf.pos >= (BUF_LEN - 1))
    {
        line_buf.pos = 0;
    }
}

char *get_debug_buf(void)
{
    return line_buf.buf;
}

void add_char_to_uart2_buf(unsigned char c)
{
    int tmp;
    tmp = (buf_pos + 1) % Buf2_Max;
    /* 已满，无法再放入 */
    if (tmp == read_pos)
    {
        return;
    }
    uart2_buf[buf_pos] = c;
    buf_pos = tmp;
}
int get_uart2_buf_pos(void)
{
    return buf_pos;
}

int uart2_buf_find_crcn(void)
{
    if (buf_pos < 2)
    {
        return 0;
    }
    if (uart2_buf[buf_pos - 2] == '\r' && uart2_buf[buf_pos - 1] == '\n')
    {
        return 1;
    }
    return 0;
}
int find_str_from_uart2_buf(char *str, char *str2)
{
    char read_buf[32] = {'\0'};

    if (str == NULL)
    {
        return 0;
    }
    while (1)
    {
        if (try_read_line(read_buf, 32) == 0)
        {
            printf("read_buf:%s\r\n", read_buf);
            if (strstr(read_buf, str) || (str2 != NULL && strstr(read_buf, str2)))
            {
                printf("find str:%s\r\n", str);
                return 1;
            }
        }
        else
        {
            printf("not find str:%s\r\n", str);
            return 0;
        }
    }
}
void clear_uart2_buf(void)
{
    memset((char *)uart2_buf, 0, Buf2_Max);
    buf_pos = 0; // 接收字符串的起始存储位置
    read_pos = 0;
}

void clear_uart2_buf_accord_pos(void)
{
    if (buf_pos > 0 && buf_pos < Buf2_Max)
    {
        memset((char *)uart2_buf, 0, buf_pos);
        buf_pos = 0;
        return;
    }
    if (buf_pos > 0)
    {
        clear_uart2_buf();
    }
}
/*
**尝试从buf中读取一行，读取成功返回0，读取失败返回负值。
**读取完一行之后，会把buf最后一个字节设置成\0，方便字符串查找
*/
void show_read_buf_pos(void)
{
    printf("read_pos:%d, buf_pos:%d\r\n", read_pos, buf_pos);
}
int try_read_line(char *buf, int len)
{
    int i, end_pos, found = 0;

    /* 缓冲区为空 */
    if (read_pos == buf_pos || (read_pos + 1) == buf_pos)
    {
        return -1;
    }
    /* read_pos在buf_pos后面end_pos是缓冲区结束的位置 */
    if (read_pos < buf_pos)
    {
        end_pos = buf_pos;
    }
    else /* read_pos在buf_pos前面 说明发生了循环 */
    {
        end_pos = Buf2_Max + buf_pos;
    }
    /* i < end_pos是因为要用到i+1这个位置 */
    for (i = read_pos; i < end_pos; i++)
    {
        /* 说明找到了一行 */
        if (uart2_buf[i % Buf2_Max] == '\r' && uart2_buf[(i + 1) % Buf2_Max] == '\n')
        {
            found = 1;
        }
        if ((i - read_pos) >= len)
        {
            printf("one line len %d bytes out of %d bytes, please modify it.\r\n", i - read_pos, len);
            clear_uart2_buf();
            return -2;
        }
        if (read_pos == (i % Buf2_Max))
        {
            found = 0; // 说明要读出位置和和找到的位置相同，也就是read_pos直接指向了\r\n，并没有其他字符串
            continue;
        }
        if (found)
        {
            /* 1.先拷贝出一行数据 */
            /* 说明\n的位置在read_pos前面，可以直接拷贝 */
            if (read_pos < (i % Buf2_Max))
            {
                memcpy(buf, &uart2_buf[read_pos], i - read_pos + 1); // 把\r字符也拷贝进来了
                buf[i - read_pos] = '\0';                            /* 把\r修改成'\0' */
                /* 2.清除缓冲区数据 */
                memset((char *)&uart2_buf[read_pos], 0, i - read_pos + 2); // 把\n字符也置零
            }
            else /* 肯定不会出现 read_pos == (i % Buf2_Max) 的情况*/
            {
                memcpy(buf, &uart2_buf[read_pos], Buf2_Max - read_pos);
                memset(&uart2_buf[read_pos], 0, Buf2_Max - read_pos);
                memcpy(&buf[Buf2_Max - read_pos], uart2_buf, (i % Buf2_Max) + 1);
                memset((char *)uart2_buf, 0, (i % Buf2_Max) + 1 + 1); //(i % Buf2_Max) + 1是\r的位置
                buf[Buf2_Max - read_pos + (i % Buf2_Max)] = '\0';     /* 把\r修改成'\0' */
            }
            /* 3.修改读指针的位置 */
            read_pos = (i + 1 + 1) % Buf2_Max; // i+1的位置是\n， i+1+1是下一个要读的位置
            show_read_buf_pos();
            return 0;
        }
    }

    return -3;
}

/*******************************************************************************
 * 函数名 : find_str
 * 描述   : 判断缓存中是否含有指定的字符串
 * 输入   :
 * 输出   :
 * 返回   : 1 找到指定字符，0 未找到指定字符
 * 注意   :
 *******************************************************************************/

int find_str(char *a)
{
    if (strstr(uart2_buf, a) != NULL)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
/*******************************************************************************
 * 函数名 : test_communicate_pass
 * 描述   : 判断串口是否能够通讯
 * 输入   :
 * 输出   :
 * 返回   :
 * 注意   :
 *******************************************************************************/
int test_communicate_pass(void)
{
    int ret;

    ret = UART2_Send_AT_Command("AT", "OK", 3, 50); // 测试通信是否成功
    return ret;
}

/*******************************************************************************
 * 函数名 : close_ate
 * 描述   : 关闭命令回显
 * 输入   :
 * 输出   :
 * 返回   :
 * 注意   :
 *******************************************************************************/
int close_ate(void)
{
    int ret;

    ret = UART2_Send_AT_Command("ATE0", "OK", 3, 50);
    return ret;
}

/*******************************************************************************
 * 函数名 : wait_creg
 * 描述   : 等待模块注册成功
 * 输入   :
 * 输出   :
 * 返回   :
 * 注意   :
 *******************************************************************************/
int wait_creg(void)
{
    int ret;

    ret = UART2_Send_AT_Command_Ext("AT+CREG?", "+CREG: 0,6", "+CREG: 0,7", 1, 100); // 测试通信是否成功
    return ret;
}
/*******************************************************************************
 * 函数名 : active_pdp
 * 描述   : 激活Pdp
 * 输入   :
 * 输出   :
 * 返回   :
 * 注意   : 如果pdp已经激活，那么就不用激活了
 *******************************************************************************/
int active_pdp(void)
{
    int ret;

    ret = UART2_Send_AT_Command("AT+QIACT?", "+QIACT:1,1,1", 1, 100);
    if (ret == 1)
    {
        return 1;
    }
    ret = UART2_Send_AT_Command("AT+QIACT=1", "OK", 1, 100);
    return ret;
}
