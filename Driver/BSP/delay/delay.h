#ifndef __DELAY_H
#define __DELAY_H

#include "stm32f10x.h"

    void systick_init(void);    // 延迟函数初始化
    void delay_ms(uint32_t ms); // 延迟
uint32_t get_ms(void);          // 获取系统的当前时间
uint64_t get_us(void);          // 获取当前的微秒级时间
    void delay_us(uint32_t us); // 微秒级延迟

#endif
