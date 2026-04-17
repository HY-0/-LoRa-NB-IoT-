#include "nbiot.h"



static void nbiot_hw_init(void)
{
    
}

int nbiot_init(uint32_t baudrate)
{
    int ret;

    nbiot_hw_init();
    nbiot_usart_init(115200);
    ret = network_init();

    if (ret != 1) 
    {
        printf("Network init failed, cannot connect MQTT.\n");
        return ret;  // 直接返回 network_init 的错误码
    }
}

int network_init(void)
{
    int ret;

    // 步骤1：检查信号强度
    ret = UART2_Send_AT_Command("AT+CSQ", "+CSQ:", 1, 500);
    if (ret == 0) 
    {
        printf("AT+CSQ 无响应\n");
        return AT_SIGNAL_WEAK;
    }

    // 步骤2：查看网络注册状态（修正）
    ret = UART2_Send_AT_Command("AT+CREG?", "+CREG:", 1, 500);
    if (ret == 0) 
    {
        printf("AT+CREG 无响应\n");
        return AT_REGISTER_FAIL;
    }
    // 检查是否已注册到网络（状态 1,5,6 均为已注册，可进行数据业务）
    if (!find_str_from_uart2_buf("+CREG: 0,1", NULL) &&
        !find_str_from_uart2_buf("+CREG: 0,5", NULL) &&
        !find_str_from_uart2_buf("+CREG: 0,6", NULL))
    {
        printf("未注册到网络，无法进行数据业务\n");
        return AT_REGISTER_FAIL;
    }

    // 步骤3：检查 PS 域附着
    ret = UART2_Send_AT_Command("AT+CGATT?", "+CGATT: 1", 1, 500);
    if (ret == 0) 
    {
        // 未附着，尝试手动附着
        ret = UART2_Send_AT_Command("AT+CGATT=1", "OK", 1, 3000); // 延长等待
        if (ret == 0) return AT_NETWORK_NOT_ATTACHED;
        // 再次检查
        ret = UART2_Send_AT_Command("AT+CGATT?", "+CGATT: 1", 1, 500);
        if (ret == 0) return AT_NETWORK_NOT_ATTACHED;
    }

    // 步骤4：检查 IP 地址
    ret = UART2_Send_AT_Command("AT+CGPADDR=0", "+CGPADDR: 0,", 1, 500);
    if (ret == 1) 
    {
        // 已有 IP，直接跳转到测试连通性
        printf("已有 IP 地址，跳过 APN 配置和 PDP 激活。\n");
        goto test_ping;
    }

    // 没有 IP，尝试激活 PDP
    printf("无 IP，尝试激活 PDP...\n");
    ret = UART2_Send_AT_Command("AT+CGACT=1,0", "OK", 1, 5000); // 延长等待
    if (ret == 0) 
    {
        // 激活失败，尝试重新配置 APN（先直接设置）
        printf("激活 PDP 失败，尝试重新配置 APN...\n");
        ret = UART2_Send_AT_Command("AT+CGDCONT=0,\"IPV4V6\",\"cmnbiot\"", "OK", 1, 500);
        if (ret == 0) {
            // 直接设置失败，尝试进入飞行模式后再设置
            printf("直接设置 APN 失败，尝试进入飞行模式...\n");
            UART2_Send_AT_Command("AT+CFUN=0", "OK", 1, 500);
            ret = UART2_Send_AT_Command("AT+CGDCONT=0,\"IPV4V6\",\"cmnbiot\"", "OK", 1, 500);
            UART2_Send_AT_Command("AT+CFUN=1", "OK", 1, 5000);
            if (ret == 0) return AT_APN_CONFIG_ERROR;
            delay_ms(3000);
        }
        // 重新尝试激活 PDP
        ret = UART2_Send_AT_Command("AT+CGACT=1,0", "OK", 1, 5000);
        if (ret == 0) return AT_NO_IP_ADDRESS;
        // 再次检查 IP
        ret = UART2_Send_AT_Command("AT+CGPADDR=0", "+CGPADDR: 0,", 1, 500);
        if (ret == 0) return AT_NO_IP_ADDRESS;
    }

test_ping:
    // 步骤5：测试数据连通性（可选，若模块不支持可注释）
    ret = UART2_Send_AT_Command("AT+CGATT?", "+CGATT: 1", 1, 500);
    if (ret == 0) {
        printf("Ping 前 PS 附着丢失\n");
        return AT_NETWORK_NOT_ATTACHED;
    }
    ret = UART2_Send_AT_Command("AT+CGPADDR=0", "+CGPADDR: 0,", 1, 500);
    if (ret == 0) 
    {
        printf("Ping 前 IP 地址丢失\n");
        return AT_NO_IP_ADDRESS;
    }

    // 执行 Ping，匹配成功标志（+ECPING: SUCC）
    ret = UART2_Send_AT_Command("AT+ECPING=\"82.157.129.239\"", "+ECPING: SUCC", 1, 8000);
    if (ret == 0) 
    {
        printf("Ping 失败，数据链路可能受限\n");
        return AT_DATA_LINK_ERROR;
    }

    printf("网络初始化及数据链路测试通过\n");
    return 1;
}

int connet_mqtt_server(void)
{
    int ret;

    // ==================== 步骤1：配置 MQTT 保活时间 ====================
    ret = UART2_Send_AT_Command("AT+ECMTCFG=\"keepalive\",0,120", "OK", 1, 200);
    if (ret == 0) return AT_MQTT_CONFIG_ERROR;

    // ==================== 步骤2：打开 TCP 连接（匹配 +ECMTOPEN: 0,0） ====================
    // 注意：模块会先返回 OK，然后异步上报 +ECMTOPEN: 0,0
    // 因此 UART2_Send_AT_Command 必须等待足够长时间，并在整个缓冲区中查找目标字符串
    ret = UART2_Send_AT_Command("AT+ECMTOPEN=0,\"82.157.129.239\",1883", "OK", 1, 400);
    if (ret == 0) return AT_CREATE_INSTANCE_ERROR;

    // ==================== 步骤3：发送 MQTT CONNECT（匹配 +ECMTCONN: 0,0） ====================
    // 根据实际测试，服务器可能需要用户名密码，这里使用 user1/pass123
    ret = UART2_Send_AT_Command("AT+ECMTCONN=0,\"myClient123\",\"user1\",\"pass123\"", "+ECMTCONN: 0,0,0", 1, 250);
    if (ret == 0) return AT_REGISTER_SERVER_ERROR;

    // ==================== 步骤4：订阅主题（匹配 +ECMTSUB: 0,1,0,1） ====================
    // 注意：消息 ID 必须与发送的 ID（这里为 1）一致
    ret = UART2_Send_AT_Command("AT+ECMTSUB=0,1,\"/test/topic\",1", "+ECMTSUB: 0,1,0,1", 1, 300);
    if (ret == 0) return AT_MQTT_SUB_ERROR;

    return 1;
}

int str2hex(char *msg, char *hex, int len)
{
    int i;
    
    if (strlen(msg) > len / 2)
    {
        return -1;
    }
    for (i = 0; i < strlen(msg); i++)
    {
        hex[2*i] = msg[i] / 16;
        if (hex[2*i] >= 10)
        {
            hex[2*i] = hex[2*i] - 10 + 'A';
        } else
        {
            hex[2*i] = hex[2*i] + '0';
        }
        hex[2*i + 1] = msg[i] % 16;
        if (hex[2*i + 1] >= 10)
        {
            hex[2*i + 1] = hex[2*i + 1] - 10 + 'A';
        } else
        {
            hex[2*i + 1] = hex[2*i + 1] + '0';
        }
    }
    return 1;
}

int send_mqtt_message(char *msg)
{
    u8 ret;
    char at_ecmtpub[AT_CMD_MAX_LEN] = {'\0'};
    
    if (msg == NULL) {
        return -1;
    }
    
    // 使用与订阅一致的话题（例如 /test/topic）
    snprintf(at_ecmtpub, AT_CMD_MAX_LEN, 
             "AT+ECMTPUB=0,2,1,0,\"/test/topic\",\"%s\"", msg);
    printf("at_sktsend:%s\r\n", at_ecmtpub);
    
    // 等待异步上报 +ECMTPUB: 0,3333,0，超时 2000ms
    ret = UART2_Send_AT_Command(at_ecmtpub, "+ECMTPUB: 0,2,0", 1, 100);
    if (ret == 0) {
        return AT_MQTT_PUB_ERROR;
    }
    
    return 1;
}

int disconnect_mqtt_server(void)
{
    int ret;
    
    ret = UART2_Send_AT_Command_Ext("AT+ECMTDISC=0","OK", "ERROR:",1,150);
    ret += UART2_Send_AT_Command_Ext("AT+ECMTCLOSE=0","OK", "ERROR:",1,150);
	return ret;
}

int deal_mqtt_message(char *buf)
{
    char *ptr;
    int count = 0;
    
    /* +ECMTRECV: 0,0,"/pc/msg",i'm pc */
    if (buf == NULL || strlen(buf) == 0 || strlen(buf) >= LING_BUF_LEN)
    {
        return -1;
    }
    ptr = buf;
    while (*ptr != '\0')
    {
        if (*ptr == ',')
        {
            count++;
        }
        if (count >=2)
        {
            break;
        }
        ptr++;
    }
    if (*ptr != ',')
    {
        return -2;
    }
    ptr++;
    printf("recv topic and msg is :%s\r\n", ptr);
    return 1;
}
