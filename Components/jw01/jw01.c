#include "jw01.h"

/**
 * @brief  JW01 传感器初始化
 * @param  baudrate: 串口波特率 (通常 9600)
 * @retval JW01_EOK 成功
 */
uint8_t jw01_init(void)
{
    jw01_uart_init(9600);
    return JW01_EOK;   /* 简单起见，无需发送 AT 测试 */
}

/**
 * @brief  读取二氧化碳浓度
 * @param  ppm: 输出浓度值 (ppm)
 * @retval JW01_EOK        成功
 *         JW01_ERR_CHECKSUM 校验错误
 *         JW01_ERR_TIMEOUT  超时未收到数据
 */
uint8_t jw01_measure(uint16_t *ppm)
{
    uint8_t *frame = NULL;
    uint32_t timeout = 1000; // 等待 1 秒

    /* 等待一个新帧 */
    while (timeout > 0) {
        frame = jw01_uart_rx_get_frame();
        if (frame != NULL) {
            break;
        }
        timeout--;
        delay_ms(1);
    }

    if (frame == NULL) {
        return JW01_ERR_TIMEOUT;
    }

    /* 校验数据包格式：长度6，帧头0x2C */
    uint16_t len = jw01_uart_rx_get_frame_len();
    if (len < JW01_PACKET_LEN) {
        return JW01_ERR_INVALID;
    }

    /* 校验和：前5字节相加的低字节等于第6字节 */
    uint8_t sum = frame[0] + frame[1] + frame[2] + frame[3] + frame[4];
    if (sum != frame[5]) {
        return JW01_ERR_CHECKSUM;
    }

    /* 提取浓度值（高字节在前） */
    *ppm = (uint16_t)(frame[1] << 8) | frame[2];
    return JW01_EOK;
}
