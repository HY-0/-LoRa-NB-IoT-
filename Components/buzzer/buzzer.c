#include "buzzer.h"

/* 对于 buzzer 这种极简外设，软件层出问题的可能性远大于硬件层 */
/* 只需软件层错误码即可，没必要硬件检测 */
static void buzzer_hw_init(void)
{
    GPIO_InitTypeDef gpio;

    BUZZER_GPIO_CLK_ENABLE();

    gpio.GPIO_Pin   = BUZZER_GPIO_PIN;
    gpio.GPIO_Mode  = GPIO_Mode_Out_PP;
    gpio.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(BUZZER_GPIO_PORT, &gpio);
    buzzer_power_off();     /* 防止出现非预期短期促鸣声 */
}

/**
 * @brief       初始化蜂鸣器
 * @param       无
 * @retval      BUZZER_EOK: 成功
 */
uint8_t buzzer_init(void)
{
    buzzer_hw_init();
    
    /* 自检（可选，可考虑取消以减少鸣响） */
    buzzer_power_on();
    delay_ms(50);
    buzzer_power_off();
    
    return BUZZER_EOK;
}

/**
 * @brief       打开蜂鸣器电源
 * @param       无
 * @retval      BUZZER_EOK: 成功
 */
uint8_t buzzer_power_on(void)
{
    /* 1. 写入低电平 */
    BUZZER_IO_WRITE(0);
    delay_us(10);

    return BUZZER_EOK;
}

/**
 * @brief       关闭蜂鸣器电源
 * @param       无
 * @retval      BUZZER_EOK: 成功
 */
uint8_t buzzer_power_off(void)
{
    BUZZER_IO_WRITE(1);
    delay_us(10);
    
    return BUZZER_EOK;
}

/**
 * @brief       持续鸣叫指定时长（阻塞式）
 * @param       half_us     : 半周期微秒数（决定音调）
 * @param       duration_ms : 鸣叫总时长（毫秒）
 * @retval      无
 */
void buzzer_beep_continuous(uint16_t half_us, uint16_t duration_ms)
{
    uint32_t start = get_ms();
    while ((get_ms() - start) < duration_ms) 
    {
        BUZZER_IO_WRITE(0);
        delay_us(half_us);
        BUZZER_IO_WRITE(1);
        delay_us(half_us);
    }
    buzzer_power_off();       /* 确保关闭 */
}

/* 方案： */
/* 给间歇性蜂鸣间歇固定 */
/* 给scene新增模式选择宏定义 */

/**
 * @brief       间歇鸣叫（在规定总时长内，以“响 on_ms、停 off_ms”循环）
 * @param       half_us     : 半周期微秒数（决定音调）
 * @param       total_ms    : 总持续时间（毫秒）
 * @param       on_ms       : 每次鸣叫时长（毫秒）
 * @param       off_ms      : 两次鸣叫间隔时长（毫秒）
 * @retval      无
 * @note        阻塞式，适用于报警场景
 */
void buzzer_beep_intermittent(uint16_t half_us, uint32_t duration_ms, BuzzerRhythm_t rhythm)
{   
    static uint16_t on_ms;
    static uint16_t off_ms;

    switch(rhythm)
    {
        case BUZZER_RHYTHM_FAST: on_ms = 80, off_ms = 50;     break;
        case BUZZER_RHYTHM_NORMAL: on_ms = 150, off_ms = 100; break;
        case BUZZER_RHYTHM_SLOW: on_ms = 300, off_ms = 200;   break;
        case BUZZER_RHYTHM_ALARM: on_ms = 200, off_ms = 100;  break;
        default: return;
    }

    uint32_t start = get_ms();
    while ((get_ms() - start) < duration_ms) 
    {
        buzzer_beep_continuous(half_us, on_ms);          /* 鸣叫 on_ms */
        if ((get_ms() - start) >= duration_ms) 
        {
            break;                                /* 总时间到，立即退出 */
        }
        delay_ms(off_ms);                         /* 停止间隔 */
    }
    buzzer_power_off();
}


/**
 * @brief       场景化发声（自动匹配频率）
 * @param       scene       : 场景类型
 * @param       duration_ms : 持续时间（毫秒）
 */
void buzzer_beep_scene(BuzzerScene_t scene, BuzzerMode_t mode, BuzzerRhythm_t rhythm, uint16_t duration_ms)
{
    uint16_t half_us;

    switch (scene) 
    {
        case BUZZER_SCENE_CLICK: half_us = BUZZER_FREQ_MID;   break;
        case BUZZER_SCENE_OK:    half_us = BUZZER_FREQ_LOW;   break;
        case BUZZER_SCENE_WARN:  half_us = BUZZER_FREQ_ALARM; break;
        case BUZZER_SCENE_ALARM: half_us = BUZZER_FREQ_HIGH;  break;
        default: return;
    }

    if(mode == BUZZER_MODE_CONTINUOUS) /* 间歇鸣叫模式还没解决 */
    {
        buzzer_beep_continuous(half_us, duration_ms);
    }
    else if(mode == BUZZER_MODE_INTERMITTENT)
    {
        /* 加了 rhythm 后耦合太大了，API 整体架构分层也不明晰 */
        buzzer_beep_intermittent(half_us, duration_ms, rhythm);
    }
}

/* static 前缀：static 修饰过的函数不能在头文件声明 */
/* 在开发期末可以给一些辅助函数加上，提高安全性，如果还在开发中就不建议加，不然不好调试 */

