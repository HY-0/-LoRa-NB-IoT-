#ifndef __JW01_H
#define __JW01_H

#include "jw01_usart.h"
#include <string.h>
#include <stdio.h>

/* JW01 传感器数据包长度 */
#define JW01_PACKET_LEN     6

/* 传感器状态枚举 */
typedef enum {
    JW01_OK = 0,
    JW01_ERR_CHECKSUM,
    JW01_ERR_TIMEOUT,
    JW01_ERR_INVALID
} jw01_status_t;

/* 初始化传感器（硬件引脚、串口） */
uint8_t jw01_init(void);

/* 读取 CO2 浓度值（阻塞等待最新数据） */
uint8_t jw01_measure(uint16_t *ppm);

/* 底层帧接收标志（是否需要暴露可根据需求） */
// uint8_t jw01_get_rx_flag(void);

#endif
