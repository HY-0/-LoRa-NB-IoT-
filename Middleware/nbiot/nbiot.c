#include "nbiot.h"

/* NB-IoT MQTT 连接状态标志 (1=已连接, 0=未连接) */
volatile uint8_t g_nb_mqtt_connected = 0;

/**
 * @brief       NB-IoT模块硬件初始化 (激活PWR, RESET功能)
 * @param       无
 * @retval      无
 */
static void nbiot_hw_init(void)
{
    GPIO_InitTypeDef gpio_init_struct;
    
    /* 使能 PWR, RESET 时钟 */
    NBIOT_PWR_GPIO_CLK_ENABLE();
    NBIOT_RESET_GPIO_CLK_ENABLE();
    
    /* PWR 引脚：推挽输出，初始高电平 */
    gpio_init_struct.GPIO_Pin   = NBIOT_PWR_GPIO_PIN;
    gpio_init_struct.GPIO_Mode  = GPIO_Mode_Out_PP;
    gpio_init_struct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(NBIOT_PWR_GPIO_PORT, &gpio_init_struct);
    
    /* RESET 引脚：推挽输出，初始高电平（低电平复位） */
    gpio_init_struct.GPIO_Pin   = NBIOT_RESET_GPIO_PIN;
    gpio_init_struct.GPIO_Mode  = GPIO_Mode_Out_PP;
    gpio_init_struct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(NBIOT_RESET_GPIO_PORT, &gpio_init_struct);
    
    NBIOT_PWR(1);           /* 模块供电正常 */
    NBIOT_RESET(1);         /* 复位释放 */
}

/**
 * @brief 转义 JSON 字符串中的双引号，用于 AT 命令的 payload 参数
 * @param src  原始 JSON 字符串（例如 {"temp":22.5}）
 * @param dst  输出缓冲区，必须足够大（最长约为 2*len(src)+1）
 */
// 去掉 static，让外部文件可以调用
int escape_json_for_at(char *dst, size_t dst_len, const char *src) {
    int n = 0;
    while (*src) {
        if (n + 2 >= dst_len) return -1;  // 留至少2字节给下一个字符和'\0'
        if (*src == '\\') {
            dst[n++] = '\\';
            dst[n++] = '\\';
        } else if (*src == '"') {
            dst[n++] = '\\';
            dst[n++] = '"';
        } else {
            dst[n++] = *src;
        }
        src++;
    }
    dst[n] = '\0';
    return n;
}

/**
 * @brief       NB-IoT模块初始化
 * @param       baudrate: NB-IoT模块UART通讯波特率
 * @retval      NBIOT_EOK  : NB-IoT模块初始化成功，函数执行成功
 *              NBIOT_ERROR: NB-IoT模块初始化失败，函数执行失败
 */
uint8_t nbiot_init(uint32_t baudrate)
{
    uint8_t ret;
    
    nbiot_hw_init();                        /* 硬件初始化 */
    
    /* 硬件复位模块 */
    NBIOT_RESET(0);
    delay_ms(300);
    NBIOT_RESET(1);
    delay_ms(2000);                         /* 等待模块启动 */                    
    
    nbiot_uart_init(baudrate);              /* UART初始化 */
    ret = nbiot_at_test();                  /* AT指令测试 */
    
    if (ret != NBIOT_EOK)
    {
        return NBIOT_ERROR;
    }
    
    /* 关闭回显，方便后续解析 */
    nbiot_echo_config(NBIOT_DISABLE);
    
    return NBIOT_EOK;
}

/**
 * @brief       向NB-IoT模块发送AT指令
 * @param       cmd    : 待发送的AT指令
 *              ack    : 等待的响应（通常为"OK"）
 *              timeout: 等待超时时间 (ms)
 * @retval      NBIOT_EOK    : 函数执行成功
 *              NBIOT_ERROR  : 收到ERROR应答
 *              NBIOT_TIMEOUT: 等待期望应答超时，函数执行失败
 */
static uint8_t nbiot_send_at_cmd(char *cmd, char *ack, uint32_t timeout)
{
    uint8_t *ret = NULL;
    
    nbiot_uart_rx_restart();
    nbiot_uart_printf("%s\r\n", cmd);
    
    if ((ack == NULL) || (timeout == 0))
    {
        return NBIOT_EOK;
    }
    else
    {
        while (timeout > 0)
        {
            ret = nbiot_uart_rx_get_frame();
            if (ret != NULL)
            {
                /* 优先检查错误应答 */
                if (strstr((const char *)ret, "ERROR") != NULL)
                {
                    return NBIOT_ERROR;
                }
                
                if (strstr((const char *)ret, ack) != NULL)
                {
                    return NBIOT_EOK;
                }
                nbiot_uart_rx_restart();
            }
            timeout--;
            delay_ms(1);
        }
        return NBIOT_TIMEOUT;
    }
}

/**
 * @brief       NB-IoT模块AT指令测试
 * @param       无
 * @retval      NBIOT_EOK  : AT指令测试成功
 *              NBIOT_ERROR: AT指令测试失败
 */
uint8_t nbiot_at_test(void)
{
    uint8_t ret;
    uint8_t i;
    
    for (i = 0; i < 10; i++)
    {
        ret = nbiot_send_at_cmd("AT", "OK", NBIOT_AT_TIMEOUT);
        if (ret == NBIOT_EOK)
        {
            return NBIOT_EOK;
        }
    }
    return NBIOT_ERROR;
}

/**
 * @brief       NB-IoT模块指令回显配置
 * @param       enable: NBIOT_DISABLE: 关闭指令回显
 *                      NBIOT_ENABLE : 开启指令回显
 * @retval      NBIOT_EOK   : 指令回显配置成功
 *              NBIOT_ERROR : 指令回显配置失败
 *              NBIOT_EINVAL: 输入参数有误
 */
uint8_t nbiot_echo_config(nbiot_enable_t enable)
{
    uint8_t ret;
    char cmd[5] = {0};
    
    switch (enable)
    {
        case NBIOT_ENABLE:
            sprintf(cmd, "ATE1");
            break;
        case NBIOT_DISABLE:
            sprintf(cmd, "ATE0");
            break;
        default:
            return NBIOT_EINVAL;
    }
    
    ret = nbiot_send_at_cmd(cmd, "OK", NBIOT_AT_TIMEOUT);
    if (ret != NBIOT_EOK)
    {
        return NBIOT_ERROR;
    }
    return NBIOT_EOK;
}

/**
 * @brief       NB-IoT模块软件复位
 * @param       无
 * @retval      NBIOT_EOK  : 软件复位成功
 *              NBIOT_ERROR: 软件复位失败
 */
uint8_t nbiot_sw_reset(void)
{
    uint8_t ret;
    
    ret = nbiot_send_at_cmd("AT+NRB", "OK", NBIOT_LONG_TIMEOUT);
    if (ret != NBIOT_EOK)
    {
        return NBIOT_ERROR;
    }
    return NBIOT_EOK;
}



/**
 * @brief       查询NB-IoT信号质量
 * @param       csq: 指向信号质量结构体的指针
 * @retval      NBIOT_EOK   : 查询成功
 *              NBIOT_ERROR : 查询失败
 *              NBIOT_TIMEOUT: 超时
 */
uint8_t nbiot_get_csq(nbiot_csq_t *csq)
{
    uint8_t *frame;
    int rssi, ber;
    
    if (csq == NULL) return NBIOT_EINVAL;
    
    nbiot_uart_rx_restart();
    nbiot_uart_printf("AT+CSQ\r\n");
    
    uint32_t timeout = NBIOT_AT_TIMEOUT;
    int got_csq = 0;
    
    while (timeout--)
    {
        frame = nbiot_uart_rx_get_frame();
        if (frame != NULL)
        {
            // 查找 "+CSQ:" 子串并解析数值
            char *p = strstr((const char *)frame, "+CSQ:");
            if (p != NULL && sscanf(p, "+CSQ: %d,%d", &rssi, &ber) == 2)
            {
                csq->rssi = rssi;
                csq->ber  = ber;
                got_csq = 1;
            }
            
            // 检查是否有 "OK"
            if (strstr((const char *)frame, "OK") != NULL)
            {
                return got_csq ? NBIOT_EOK : NBIOT_ERROR;
            }
            
            // 当前帧中没有 OK，清空缓冲并继续等待后续帧
            nbiot_uart_rx_restart();
        }
        delay_ms(1);
    }
    return NBIOT_TIMEOUT;
}

#include "usart.h"

/**
 * @brief       查询NB-IoT网络注册状态
 * @param       stat: 指向注册状态变量的指针
 * @retval      NBIOT_EOK   : 查询成功
 *              NBIOT_ERROR : 查询失败
 *              NBIOT_TIMEOUT: 超时
 */
uint8_t nbiot_get_creg(nbiot_creg_stat_t *stat)
{
    uint8_t *frame;
    int n, reg;
    if (stat == NULL) return NBIOT_EINVAL;

    nbiot_uart_rx_restart();
    nbiot_uart_printf("AT+CREG?\r\n");

    uint32_t timeout = NBIOT_AT_TIMEOUT;
    int got_creg = 0;
    while (timeout--) {
        frame = nbiot_uart_rx_get_frame();
        if (frame != NULL) {
            char *p = strstr((const char *)frame, "+CREG:");
            if (p != NULL && sscanf(p, "+CREG: %d,%d", &n, &reg) == 2) {
                *stat = (nbiot_creg_stat_t)reg;
                got_creg = 1;
            }
            if (strstr((const char *)frame, "OK") != NULL) {
                return got_creg ? NBIOT_EOK : NBIOT_ERROR;
            }
            nbiot_uart_rx_restart();
        }
        delay_ms(1);
    }
    return NBIOT_TIMEOUT;
}

/**
 * @brief       附着网络
 * @param       无
 * @retval      NBIOT_EOK   : 成功
 *              NBIOT_ERROR : 失败
 */
uint8_t nbiot_attach_network(void)
{
    return nbiot_send_at_cmd("AT+CGATT=1", "OK", NBIOT_LONG_TIMEOUT);
}

/**
 * @brief       打开MQTT连接
 * @param       socket_id: socket标识
 *              host     : 服务器地址（IP或域名）
 *              port     : 端口号
 * @retval      NBIOT_EOK   : 成功
 *              NBIOT_ERROR : 失败
 *              NBIOT_TIMEOUT: 超时
 */
uint8_t nbiot_mqtt_open(uint8_t socket_id, const char *host, uint16_t port)
{
    uint8_t *frame;
    char cmd[80] = {0};
    sprintf(cmd, "AT+ECMTOPEN=%d,\"%s\",%d", socket_id, host, port);

    nbiot_uart_rx_restart();
    nbiot_uart_printf("%s\r\n", cmd);

    uint32_t timeout = NBIOT_LONG_TIMEOUT;
    int got_status = 0;
    int result_val = -1;

    while (timeout--) {
        frame = nbiot_uart_rx_get_frame();
        if (frame) {
            if (strstr((const char *)frame, "ERROR"))
                return NBIOT_ERROR;

            char *p = strstr((const char *)frame, "+ECMTOPEN:");
            if (p) {
                int id, res;
                if (sscanf(p, "+ECMTOPEN: %d,%d", &id, &res) >= 2) {
                    got_status = 1;
                    result_val = res;
                }
            }

            if (strstr((const char *)frame, "OK")) {
                if (got_status)
                    return (result_val == 0) ? NBIOT_EOK : NBIOT_ERROR;
                else
                    return NBIOT_EOK;   // 只收到 OK，认为成功
            }
            nbiot_uart_rx_restart();
        }
        delay_ms(1);
    }
    return NBIOT_TIMEOUT;
}

/**
 * @brief       MQTT连接
 * @param       socket_id: socket标识
 *              clientid : 客户端ID
 *              username : 用户名
 *              password : 密码
 * @retval      NBIOT_EOK   : 成功
 *              NBIOT_ERROR : 失败
 *              NBIOT_TIMEOUT: 超时
 */
/**
 * @brief       MQTT连接
 * @param       socket_id: socket标识
 *              clientid : 客户端ID
 *              username : 用户名
 *              password : 密码
 * @retval      NBIOT_EOK   : 成功
 *              NBIOT_ERROR : 失败
 *              NBIOT_TIMEOUT: 超时
 */
uint8_t nbiot_mqtt_connect(uint8_t socket_id, const char *clientid,
                           const char *username, const char *password)
{
    uint8_t *frame;
    char cmd[150] = {0};
    int id, res;     // 必须定义，否则报错

    sprintf(cmd, "AT+ECMTCONN=%d,\"%s\",\"%s\",\"%s\"",
            socket_id, clientid, username, password);

    nbiot_uart_rx_restart();
    nbiot_uart_printf("%s\r\n", cmd);

    uint32_t timeout = NBIOT_LONG_TIMEOUT;
    while (timeout--) {
        frame = nbiot_uart_rx_get_frame();
        if (frame != NULL) {
            if (strstr((const char *)frame, "ERROR") != NULL) {
                g_nb_mqtt_connected = 0;
                return NBIOT_ERROR;
            }

            char *p = strstr((const char *)frame, "+ECMTCONN:");
            if (p != NULL) {
                if (sscanf(p, "+ECMTCONN: %d,%d", &id, &res) >= 2) {
                    if (res == 0) {
                        g_nb_mqtt_connected = 1;
                        return NBIOT_EOK;
                    } else {
                        g_nb_mqtt_connected = 0;
                        return NBIOT_ERROR;
                    }
                }
            }

            if (strstr((const char *)frame, "OK") != NULL) {
                // 有些模块只返回OK，也认为连接成功
                g_nb_mqtt_connected = 1;
                return NBIOT_EOK;
            }

            nbiot_uart_rx_restart();
        }
        delay_ms(1);
    }
    g_nb_mqtt_connected = 0;
    return NBIOT_TIMEOUT;
}

/**
 * @brief       MQTT订阅主题
 * @param       socket_id: socket标识
 *              msgid    : 报文ID
 *              topic    : 主题字符串
 *              qos      : 服务质量等级 (0,1,2)
 * @retval      NBIOT_EOK   : 成功
 *              NBIOT_ERROR : 失败
 *              NBIOT_TIMEOUT: 超时
 */
uint8_t nbiot_mqtt_subscribe(uint8_t socket_id, uint16_t msgid,
                             const char *topic, uint8_t qos)
{
    uint8_t *frame;
    char cmd[128] = {0};
    sprintf(cmd, "AT+ECMTSUB=%d,%d,\"%s\",%d", socket_id, msgid, topic, qos);

    nbiot_uart_rx_restart();
    nbiot_uart_printf("%s\r\n", cmd);

    uint32_t timeout = NBIOT_AT_TIMEOUT;
    int got_status = 0;
    int result_val = -1;

    while (timeout--) {
        frame = nbiot_uart_rx_get_frame();
        if (frame) {
            if (strstr((const char *)frame, "ERROR"))
                return NBIOT_ERROR;

            char *p = strstr((const char *)frame, "+ECMTSUB:");
            if (p) {
                int id, mid, res;
                if (sscanf(p, "+ECMTSUB: %d,%d,%d", &id, &mid, &res) >= 3) {
                    got_status = 1;
                    result_val = res;
                }
            }

            if (strstr((const char *)frame, "OK")) {
                if (got_status)
                    return (result_val == 0) ? NBIOT_EOK : NBIOT_ERROR;
                else
                    return NBIOT_EOK;
            }
            nbiot_uart_rx_restart();
        }
        delay_ms(1);
    }
    return NBIOT_TIMEOUT;
}

/**
 * @brief       MQTT发布消息
 * @param       socket_id: socket标识
 *              msgid    : 报文ID
 *              qos      : 服务质量等级
 *              retain   : 保留标志 (0或1)
 *              topic    : 主题
 *              payload  : 消息内容
 * @retval      NBIOT_EOK   : 成功
 *              NBIOT_ERROR : 失败
 *              NBIOT_TIMEOUT: 超时
 */
uint8_t nbiot_mqtt_publish(uint8_t socket_id, uint16_t msgid,
                           uint8_t qos, uint8_t retain,
                           const char *topic, const char *payload)
{
    uint8_t *frame;
    char cmd[512] = {0};
    sprintf(cmd, "AT+ECMTPUB=%d,%d,%d,%d,\"%s\",\"%s\"",
            socket_id, msgid, qos, retain, topic, payload);

    int len = strlen(cmd);
    nbiot_uart_rx_restart();
    nbiot_uart_printf("%s\r\n", cmd);

    uint32_t timeout = NBIOT_AT_TIMEOUT;
    int got_status = 0;
    int result_val = -1;

    while (timeout--) {
        frame = nbiot_uart_rx_get_frame();
        if (frame) {

            if (strstr((const char *)frame, "ERROR"))
                return NBIOT_ERROR;

            char *p = strstr((const char *)frame, "+ECMTPUB:");
            if (p) {
                int id, mid, res;
                if (sscanf(p, "+ECMTPUB: %d,%d,%d", &id, &mid, &res) >= 3) {
                    got_status = 1;
                    result_val = res;
                }
            }

            if (strstr((const char *)frame, "OK")) {
                if (got_status)
                    return (result_val == 0) ? NBIOT_EOK : NBIOT_ERROR;
                else
                    return NBIOT_EOK;
            }
            nbiot_uart_rx_restart();
        }
        delay_ms(1);
    }
    return NBIOT_TIMEOUT;
}

/**
 * @brief MQTT发布消息（十六进制负载模式，用于含特殊字符的数据，如JSON）
 * @param socket_id socket标识
 * @param msgid 报文ID
 * @param qos 服务质量
 * @param retain 保留标志
 * @param topic 主题
 * @param payload_hex 十六进制负载字符串（不加双引号）
 * @return NBIOT_EOK/NBIOT_ERROR/NBIOT_TIMEOUT
 */
uint8_t nbiot_mqtt_publish_hex(uint8_t socket_id, uint16_t msgid,
                               uint8_t qos, uint8_t retain,
                               const char *topic, const char *payload_hex)
{
    uint8_t *frame;
    char cmd[600] = {0};
    // 注意：payload_hex 不加双引号
    sprintf(cmd, "AT+ECMTPUB=%d,%d,%d,%d,\"%s\",%s",
            socket_id, msgid, qos, retain, topic, payload_hex);
        usart_printf(USART2, "cmd:%s\r\n", cmd);
    nbiot_uart_rx_restart();
    nbiot_uart_printf("%s\r\n", cmd);

    uint32_t timeout = NBIOT_AT_TIMEOUT;
    int got_status = 0;
    int result_val = -1;

    while (timeout--) {
        frame = nbiot_uart_rx_get_frame();
        if (frame != NULL) {
            usart_printf(USART2, "frame:%s\r\n", frame);
            if (strstr((const char *)frame, "ERROR"))
                return NBIOT_ERROR;

            char *p = strstr((const char *)frame, "+ECMTPUB:");
            if (p) {
                int id, mid, res;
                if (sscanf(p, "+ECMTPUB: %d,%d,%d", &id, &mid, &res) >= 3) {
                    got_status = 1;
                    result_val = res;
                }
            }

            if (strstr((const char *)frame, "OK")) {
                if (got_status)
                    return (result_val == 0) ? NBIOT_EOK : NBIOT_ERROR;
                else
                    return NBIOT_EOK;
            }
            nbiot_uart_rx_restart();
        }
        delay_ms(1);
    }
    return NBIOT_TIMEOUT;
}

/**
 * @brief       关闭MQTT连接
 * @param       socket_id: socket标识
 * @retval      NBIOT_EOK   : 成功
 *              NBIOT_ERROR : 失败
 */
uint8_t nbiot_mqtt_close(uint8_t socket_id)
{
    char cmd[20] = {0};          // 必须定义
    sprintf(cmd, "AT+ECMTCLOSE=%d", socket_id);
    uint8_t ret = nbiot_send_at_cmd(cmd, "OK", NBIOT_AT_TIMEOUT);
    if (ret == NBIOT_EOK) {
        g_nb_mqtt_connected = 0;
    }
    return ret;
}

/**
 * @brief       接收MQTT消息（从串口缓存中提取一条）
 * @param       recv: 指向接收消息结构体的指针
 * @retval      NBIOT_EOK  : 成功提取一条消息
 *              NBIOT_EBUSY: 暂无消息
 *              NBIOT_ERROR: 格式解析失败
 */
uint8_t nbiot_mqtt_recv(nbiot_mqtt_recv_t *recv)
{
    uint8_t *ret;
    int id, msgid;
    char topic[64];
    char payload[NBIOT_RECV_MSG_MAX_LEN];
    
    if (recv == NULL) return NBIOT_EINVAL;
    
    ret = nbiot_uart_rx_get_frame();
    if (ret == NULL)
    {
        return NBIOT_EBUSY;          /* 没有完整帧 */
    }
    
    /* 尝试解析 +ECMTRECV: <id>,<msgid>,"<topic>","<payload>" */
    if (sscanf((const char *)ret, "+ECMTRECV: %d,%d,\"%[^\"]\",\"%[^\"]\"",
               &id, &msgid, topic, payload) == 4)
    {
        recv->socket_id = id;
        recv->msgid = msgid;
        strncpy(recv->topic, topic, sizeof(recv->topic) - 1);
        recv->topic[sizeof(recv->topic) - 1] = '\0';
        strncpy(recv->payload, payload, sizeof(recv->payload) - 1);
        recv->payload[sizeof(recv->payload) - 1] = '\0';
        
        nbiot_uart_rx_restart();     /* 消费该帧 */
        return NBIOT_EOK;
    }
    
    /* 如果不是 MQTT 消息帧，清空继续 */
    nbiot_uart_rx_restart();
    return NBIOT_ERROR;
}

/**
 * @brief       通过 AT+CCLK? 获取网络时间，并计算为 Unix 时间戳
 * @param       timestamp: 输出参数，返回计算得到的 Unix 时间戳
 * @retval      NBIOT_EOK   : 成功
 * NBIOT_ERROR : 失败/未同步
 * NBIOT_TIMEOUT: 超时
 */
uint8_t nbiot_get_timestamp(uint32_t *timestamp)
{
    uint8_t *frame;
    int year, month, day, hour, minute, second, tz;
    char sign;

    nbiot_uart_rx_restart();
    nbiot_uart_printf("AT+CCLK?\r\n");

    uint32_t timeout = NBIOT_AT_TIMEOUT;
    int got_time = 0;

    while (timeout--) {
        frame = nbiot_uart_rx_get_frame();
        if (frame != NULL) {
            // 匹配 +CCLK: "26/05/10,17:45:08+32" (注意：有的模块没有引号，有的带引号)
            char *p = strstr((const char *)frame, "+CCLK:");
            if (p != NULL) {
                // 尝试解析带引号的格式
                int parsed = sscanf(p, "+CCLK: \"%d/%d/%d,%d:%d:%d%c%d\"",
                                    &year, &month, &day, &hour, &minute, &second, &sign, &tz);
                if (parsed < 6) {
                    // 如果解析失败，尝试解析不带引号的格式 (兼容性处理)
                    parsed = sscanf(p, "+CCLK: %d/%d/%d,%d:%d:%d%c%d",
                                    &year, &month, &day, &hour, &minute, &second, &sign, &tz);
                }

                if (parsed >= 6) {
                    got_time = 1;
                    if (parsed < 8) { tz = 0; sign = '+'; } // 如果没有时区，默认时区偏移为0
                }
            }

            if (strstr((const char *)frame, "ERROR") != NULL) {
                return NBIOT_ERROR;
            }

            if (strstr((const char *)frame, "OK") != NULL) {
                if (got_time) {
                    // 校验时间合法性，防止数组越界或溢出
                    if (month < 1 || month > 12 || day < 1 || day > 31 ||
                        hour > 23 || minute > 59 || second > 59) {
                        return NBIOT_ERROR;
                    }

                    // --- 将年月日时分秒转换为 Unix 时间戳 (UTC) ---
                    uint32_t days = 0;
                    int y = (year < 100) ? (2000 + year) : year; // 兼容两位数或四位数年份
                    
                    // 1. 计算自1970年以来的整年天数
                    for (int i = 1970; i < y; i++) {
                        if ((i % 4 == 0 && i % 100 != 0) || (i % 400 == 0)) days += 366; // 闰年
                        else days += 365;
                    }
                    
                    // 2. 计算当年的整月天数
                    int m_days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
                    if ((y % 4 == 0 && y % 100 != 0) || (y % 400 == 0)) m_days[1] = 29;
                    for (int i = 0; i < month - 1; i++) days += m_days[i];
                    
                    // 3. 加上当月走过的天数
                    days += (day - 1);

                    // 4. 计算当前总秒数
                    uint32_t epoch = days * 86400 + hour * 3600 + minute * 60 + second;

                    // 5. 扣除时区偏差（NB-IoT时区单位通常是 15分钟(900秒) 的倍数）
                    // 例如 +32 代表 UTC+8 (32 * 15分钟 = 480分钟 = 8小时)
                    // int tz_offset_sec = tz * 15 * 60;
                    // if (sign == '+') epoch -= tz_offset_sec; // 减去东区偏移回到UTC
                    // else if (sign == '-') epoch += tz_offset_sec; // 加上西区偏移回到UTC

                    *timestamp = epoch;
                    return NBIOT_EOK;
                }
                return NBIOT_ERROR;
            }
            nbiot_uart_rx_restart(); // 如果不是OK，继续等待下一帧
        }
        delay_ms(1);
    }
    return NBIOT_TIMEOUT;
}
