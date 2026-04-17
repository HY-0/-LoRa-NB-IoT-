#include "lora.h"

/**
 * @brief       ATK-MW1278D模块硬件初始化 (激活MD0，AUX的功能)
 * @param       无
 * @retval      无
 */
static void lora_hw_init(void)
{
    GPIO_InitTypeDef gpio_init_struct;
    
    /* 使能 MD0, AUX 时钟 */
    LORA_AUX_GPIO_CLK_ENABLE();
    LORA_MD0_GPIO_CLK_ENABLE();
    
	/* AUX 引脚：上拉输入 */
	gpio_init_struct.GPIO_Pin   = LORA_AUX_GPIO_PIN;    /* 应定义为 GPIO_Pin_11 */
    gpio_init_struct.GPIO_Mode  = GPIO_Mode_IPU;        /* 上拉输入 */
	gpio_init_struct.GPIO_Speed = GPIO_Speed_50MHz;     /* 速度可设为 50MHz */
	GPIO_Init(LORA_AUX_GPIO_PORT, &gpio_init_struct);

	/* MD0 引脚：推挽输出，初始低电平 */
	gpio_init_struct.GPIO_Pin   = LORA_MD0_GPIO_PIN;    /* 应定义为 GPIO_Pin_12 */
    gpio_init_struct.GPIO_Mode  = GPIO_Mode_Out_PP;     /* 推挽输出 */
	gpio_init_struct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(LORA_MD0_GPIO_PORT, &gpio_init_struct);
    
    LORA_MD0(0);     /* 默认通信模式 */
}

/**
 * @brief       LoRa模块初始化
 * @param       baudrate: ATK-MW1278D模块UART通讯波特率
 * @retval      LORA_EOK  : ATK-MW1278D模块初始化成功，函数执行成功
 *              LORA_ERROR: ATK-MW1278D模块初始化失败，函数执行失败
 */
uint8_t lora_init(uint32_t baudrate)
{
    uint8_t ret;                 /* 错误类型标记 */
    
    lora_hw_init();              /* 硬件初始化 */
    lora_uart_init(baudrate);    /* UART初始化 */
	lora_enter_config();         /* 进入配置模式 */
    ret = lora_at_test();        /* AT指令测试 */
	lora_exit_config();          /* 退出配置模式 */
    
    if (ret != LORA_EOK)         /* Lora初始化失败 */
    {
        return LORA_ERROR;       /* 错误类型 */
    }

    return LORA_EOK;             /* Lora初始化成功 */
}

/**
 * @brief       ATK-MW1278D模块进入配置模式
 * @param       无
 * @retval      无
 */
void lora_enter_config(void)
{
    LORA_MD0(1);
}

/**
 * @brief       ATK-MW1278D模块退出配置模式
 * @param       无
 * @retval      无
 */
void lora_exit_config(void)
{
    LORA_MD0(0);
}

/**
 * @brief       判断ATK-MW1278D模块是否空闲
 * @note        仅当ATK-MW1278D模块空闲的时候，才能发送数据
 * @param       无
 * @retval      LORA_EOK  : ATK-MW1278D模块空闲
 *              LORA_EBUSY: ATK-MW1278D模块忙
 */
uint8_t lora_free(void)
{
    if (LORA_AUX() != 0)
    {
        return LORA_EBUSY;
    }
    
    return LORA_EOK;
}

/**
 * @brief       向ATK-MW1278D模块发送AT指令
 * @param       cmd    : 待发送的AT指令
 *              ack    : 等待的响应
 *              timeout: 等待超时时间
 * @retval      LORA_EOK     : 函数执行成功
 *              LORA_ETIMEOUT: 等待期望应答超时，函数执行失败
 */
uint8_t lora_send_at_cmd(char *cmd, char *ack, uint32_t timeout)
{
    uint8_t *ret = NULL;
    
    lora_uart_rx_restart();
    lora_uart_printf("%s\r\n", cmd);
    
    if ((ack == NULL) || (timeout == 0))
    {
        return LORA_EOK;
    }
    else
    {
        while (timeout > 0)
        {
            ret = lora_uart_rx_get_frame();
            if (ret != NULL)
            {
                if (strstr((const char *)ret, ack) != NULL)
                {
                    return LORA_EOK;
                }
                else
                {
                    lora_uart_rx_restart();
                }
            }
            timeout--;
            delay_ms(1);
        }
        
        return LORA_ETIMEOUT;
    }
}

/**
 * @brief       ATK-MW1278D模块AT指令测试
 * @param       无
 * @retval      LORA_EOK  : AT指令测试成功
 *              LORA_ERROR: AT指令测试失败
 */
uint8_t lora_at_test(void)
{
    uint8_t ret;
    uint8_t i;
    
    for (i=0; i<10; i++)
    {
        ret = lora_send_at_cmd("AT", "OK", LORA_AT_TIMEOUT);
        if (ret == LORA_EOK)
        {
            return LORA_EOK;
        }
    }
    
    return LORA_ERROR;
}

// /**
//  * @brief       ATK-MW1278D模块指令回显配置
//  * @param       enable: LORA_DISABLE: 关闭指令回显
//  *                      LORA_ENABLE : 开启指令回显
//  * @retval      LORA_EOK   : 指令回显配置成功
//  *              LORA_ERROR : 指令回显配置失败
//  *              LORA_EINVAL: 输入参数有误
//  */
// uint8_t lora_echo_config(LORA_enable_t enable)
// {
//     uint8_t ret;
//     char cmd[5] = {0};
    
//     switch (enable)
//     {
//         case LORA_ENABLE:
//         {
//             sprintf(cmd, "ATE1");
//             break;
//         }
//         case LORA_DISABLE:
//         {
//             sprintf(cmd, "ATE0");
//             break;
//         }
//         default:
//         {
//             return LORA_EINVAL;
//         }
//     }
    
//     ret = lora_send_at_cmd(cmd, "OK", LORA_AT_TIMEOUT);
//     if (ret != LORA_EOK)
//     {
//         return LORA_ERROR;
//     }
    
//     return LORA_EOK;
// }

// /**
//  * @brief       ATK-MW1278D模块软件复位
//  * @param       无
//  * @retval      LORA_EOK  : 软件复位成功
//  *              LORA_ERROR: 软件复位失败
//  */
// uint8_t lora_sw_reset(void)
// {
//     uint8_t ret;
    
//     ret = lora_send_at_cmd("AT+RESET", "OK", LORA_AT_TIMEOUT);
//     if (ret != LORA_EOK)
//     {
//         return LORA_ERROR;
//     }
    
//     return LORA_EOK;
// }

// /**
//  * @brief       ATK-MW1278D模块参数保存配置
//  * @param       enable: LORA_DISABLE: 不保存参数
//  *                      LORA_ENABLE : 保存参数
//  * @retval      LORA_EOK   : 参数保存配置成功
//  *              LORA_ERROR : 参数保存配置失败
//  *              LORA_EINVAL: 输入参数有误
//  */
// uint8_t LORA_flash_config(LORA_enable_t enable)
// {
//     uint8_t ret;
//     char cmd[11] = {0};
    
//     switch (enable)
//     {
//         case LORA_DISABLE:
//         {
//             sprintf(cmd, "AT+FLASH=0");
//             break;
//         }
//         case LORA_ENABLE:
//         {
//             sprintf(cmd, "AT+FLASH=1");
//             break;
//         }
//         default:
//         {
//             return LORA_EINVAL;
//         }
//     }
    
//     ret = LORA_send_at_cmd(cmd, "OK", LORA_AT_TIMEOUT);
//     if (ret != LORA_EOK)
//     {
//         return LORA_ERROR;
//     }
    
//     return LORA_EOK;
// }

// /**
//  * @brief       ATK-MW1278D模块恢复出厂配置
//  * @param       无
//  * @retval      LORA_EOK   : 恢复出厂配置成功
//  *              LORA_ERROR : 恢复出厂配置失败
//  */
// uint8_t LORA_default(void)
// {
//     uint8_t ret;
    
//     ret = LORA_send_at_cmd("AT+DEFAULT", "OK", LORA_AT_TIMEOUT);
//     if (ret != LORA_EOK)
//     {
//         return LORA_ERROR;
//     }
    
//     return LORA_EOK;
// }

/**
 * @brief       ATK-MW1278D模块设备地址配置
 * @param       addr: 设备地址
 * @retval      LORA_EOK   : 设备地址配置成功
 *              LORA_ERROR : 设备地址配置失败
 */
uint8_t lora_addr_config(uint16_t addr)
{
    uint8_t ret;
    char cmd[14] = {0};
    
    sprintf(cmd, "AT+ADDR=%02X,%02X", (uint8_t)(addr >> 8) & 0xFF, (uint8_t)addr & 0xFF);
    
    ret = lora_send_at_cmd(cmd, "OK", LORA_AT_TIMEOUT);
    if (ret != LORA_EOK)
    {
        return LORA_ERROR;
    }
    
    return LORA_EOK;
}

/**
 * @brief       ATK-MW1278D模块发射功率配置
 * @param       tpower: LORA_TPOWER_11DBM: 11dBm
 *                      LORA_TPOWER_14DBM: 14dBm
 *                      LORA_TPOWER_17DBM: 17dBm
 *                      LORA_TPOWER_20DBM: 20dBm（默认）
 * @retval      LORA_EOK   : 发射功率配置成功
 *              LORA_ERROR : 发射功率配置失败
 *              LORA_EINVAL: 输入参数有误
 */
uint8_t lora_tpower_config(LORA_tpower_t tpower)
{
    uint8_t ret;
    char cmd[12] = {0};
    
    switch (tpower)
    {
        case LORA_TPOWER_11DBM:
        case LORA_TPOWER_14DBM:
        case LORA_TPOWER_17DBM:
        case LORA_TPOWER_20DBM:
        {
            break;
        }
        default:
        {
            return LORA_EINVAL;
        }
    }
    
    sprintf(cmd, "AT+TPOWER=%d", tpower);
    
    ret = lora_send_at_cmd(cmd, "OK", LORA_AT_TIMEOUT);
    if (ret != LORA_EOK)
    {
        return LORA_ERROR;
    }
    
    return LORA_EOK;
}

/**
 * @brief       ATK-MW1278D模块工作模式配置
 * @param       workmode: LORA_WORKMODE_NORMAL  : 一般模式（默认）
 *                        LORA_WORKMODE_WAKEUP  : 唤醒模式
 *                        LORA_WORKMODE_LOWPOWER: 省电模式
 *                        LORA_WORKMODE_SIGNAL  : 信号强度模式
 * @retval      LORA_EOK   : 工作模式配置成功
 *              LORA_ERROR : 工作模式配置失败
 *              LORA_EINVAL: 输入参数有误
 */
uint8_t lora_workmode_config(LORA_workmode_t workmode)
{
    uint8_t ret;
    char cmd[12] = {0};
    
    switch (workmode)
    {
        case LORA_WORKMODE_NORMAL:
        case LORA_WORKMODE_WAKEUP:
        case LORA_WORKMODE_LOWPOWER:
        case LORA_WORKMODE_SIGNAL:
        {
            break;
        }
        default:
        {
            return LORA_EINVAL;
        }
    }
    
    sprintf(cmd, "AT+CWMODE=%d", workmode);
    
    ret = lora_send_at_cmd(cmd, "OK", LORA_AT_TIMEOUT);
    if (ret != LORA_EOK)
    {
        return LORA_ERROR;
    }
    
    return LORA_EOK;
}

/**
 * @brief       ATK-MW1278D模块发送模式配置
 * @param       tmode: LORA_TMODE_TT: 透明传输（默认）
 *                     LORA_TMODE_DT: 定向传输
 * @retval      LORA_EOK   : 发送模式配置成功
 *              LORA_ERROR : 发送模式配置失败
 *              LORA_EINVAL: 输入参数有误
 */
uint8_t lora_tmode_config(LORA_tmode_t tmode)
{
    uint8_t ret;
    char cmd[11] = {0};
    
    switch (tmode)
    {
        case LORA_TMODE_TT:
        case LORA_TMODE_DT:
        {
            break;
        }
        default:
        {
            return LORA_EINVAL;
        }
    }
    
    sprintf(cmd, "AT+TMODE=%d", tmode);
    
    ret = lora_send_at_cmd(cmd, "OK", LORA_AT_TIMEOUT);
    if (ret != LORA_EOK)
    {
        return LORA_ERROR;
    }
    
    return LORA_EOK;
}

/**
 * @brief       ATK-MW1278D模块空中速率和信道配置
 * @param       wlrate : LORA_WLRATE_0K3 : 0.3Kbps
 *                       LORA_WLRATE_1K2 : 1.2Kbps
 *                       LORA_WLRATE_2K4 : 2.4Kbps
 *                       LORA_WLRATE_4K8 : 4.8Kbps
 *                       LORA_WLRATE_9K6 : 9.6Kbps
 *                       LORA_WLRATE_19K2: 19.2Kbps（默认）
 *              channel: 信道，范围0~83
 * @retval      LORA_EOK   : 空中速率和信道配置成功
 *              LORA_ERROR : 空中速率和信道配置失败
 *              LORA_EINVAL: 输入参数有误
 */
uint8_t lora_wlrate_channel_config(LORA_wlrate_t wlrate, uint8_t channel)
{
    uint8_t ret;
    char cmd[15] = {0};
    
    switch (wlrate)
    {
        case LORA_WLRATE_1K2:
        case LORA_WLRATE_2K4:
        case LORA_WLRATE_4K8:
        case LORA_WLRATE_9K6:
        case LORA_WLRATE_19K2:
        {
            break;
        }
        default:
        {
            return LORA_EINVAL;
        }
    }
    
    if (channel > 83)
    {
        return LORA_EINVAL;
    }
    
    sprintf(cmd, "AT+WLRATE=%d,%d", channel, wlrate);
    
    ret = lora_send_at_cmd(cmd, "OK", LORA_AT_TIMEOUT);
    if (ret != LORA_EOK)
    {
        return LORA_ERROR;
    }
    
    return LORA_EOK;
}

/**
 * @brief       ATK-MW1278D模块休眠时间配置
 * @param       wltime: LORA_WLTIME_1S: 1秒（默认）
 *                      LORA_WLTIME_2S: 2秒
 * @retval      LORA_EOK   : 休眠时间配置成功
 *              LORA_ERROR : 休眠时间配置失败
 *              LORA_EINVAL: 输入参数有误
 */
uint8_t lora_wltime_config(LORA_wltime_t wltime)
{
    uint8_t ret;
    char cmd[12] = {0};
    
    switch (wltime)
    {
        case LORA_WLTIME_1S:
        case LORA_WLTIME_2S:
        {
            break;
        }
        default:
        {
            return LORA_EINVAL;
        }
    }
    
    sprintf(cmd, "AT+WLTIME=%d", wltime);
    
    ret = lora_send_at_cmd(cmd, "OK", LORA_AT_TIMEOUT);
    if (ret != LORA_EOK)
    {
        return LORA_ERROR;
    }
    
    return LORA_EOK;
}

/**
 * @brief       ATK-MW1278D模块串口配置
 * @param       baudrate: LORA_UARTRATE_1200BPS  : 1200bps
 *                        LORA_UARTRATE_2400BPS  : 2400bps
 *                        LORA_UARTRATE_4800BPS  : 4800bps
 *                        LORA_UARTRATE_9600BPS  : 9600bps
 *                        LORA_UARTRATE_19200BPS : 19200bps
 *                        LORA_UARTRATE_38400BPS : 38400bps
 *                        LORA_UARTRATE_57600BPS : 57600bps
 *                        LORA_UARTRATE_115200BPS: 115200bps（默认）
 *              parity  : LORA_UARTPARI_NONE: 无校验（默认）
 *                        LORA_UARTPARI_EVEN: 偶校验
 *                        LORA_UARTPARI_ODD : 奇校验
 * @retval      LORA_EOK   : 串口配置成功
 *              LORA_ERROR : 串口配置失败
 *              LORA_EINVAL: 输入参数有误
 */
uint8_t lora_uart_config(LORA_uartrate_t baudrate, LORA_uartpari_t parity)
{
    uint8_t ret;
    char cmd[12] = {0};
    
    switch (baudrate)
    {
        case LORA_UARTRATE_1200BPS:
        case LORA_UARTRATE_2400BPS:
        case LORA_UARTRATE_4800BPS:
        case LORA_UARTRATE_9600BPS:
        case LORA_UARTRATE_19200BPS:
        case LORA_UARTRATE_38400BPS:
        case LORA_UARTRATE_57600BPS:
        case LORA_UARTRATE_115200BPS:
        {
            break;
        }
        default:
        {
            return LORA_EINVAL;
        }
    }
    
    switch (parity)
    {
        case LORA_UARTPARI_NONE:
        case LORA_UARTPARI_EVEN:
        case LORA_UARTPARI_ODD:
        {
            break;
        }
        default:
        {
            return LORA_EINVAL;
        }
    }
    
    sprintf(cmd, "AT+UART=%d,%d", baudrate, parity);
    
    ret = lora_send_at_cmd(cmd, "OK", LORA_AT_TIMEOUT);
    if (ret != LORA_EOK)
    {
        return LORA_ERROR;
    }
    
    return LORA_EOK;
}
