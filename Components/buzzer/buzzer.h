#ifndef __BUZZER_H
#define __BUZZER_H

#include "stm32f10x.h"
#include "delay.h"

/* 引脚定义 */
#define BUZZER_GPIO_PORT           GPIOB
#define BUZZER_GPIO_PIN            GPIO_Pin_1
#define BUZZER_GPIO_CLK_ENABLE()   do{ RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE); }while(0)

/* IO 操作 */
#define BUZZER_IO_WRITE(x)         do{ (x) ?                                                                \
                                            GPIO_WriteBit(BUZZER_GPIO_PORT, BUZZER_GPIO_PIN, Bit_SET) :     \
                                            GPIO_WriteBit(BUZZER_GPIO_PORT, BUZZER_GPIO_PIN, Bit_RESET);    \
                                            }while(0)

/* 常用频率预设（Hz） */
#define BUZZER_FREQ_LOW      500   /* 低沉提示音 */
#define BUZZER_FREQ_MID      250   /* 标准提示音 */
#define BUZZER_FREQ_HIGH     166   /* 尖锐告警音 */
#define BUZZER_FREQ_ALARM    200   /* 紧急报警音 */

/* 鸣叫场景 */
typedef enum {
    BUZZER_SCENE_CLICK,      /* 按键音 */
    BUZZER_SCENE_OK,         /* 成功提示 */
    BUZZER_SCENE_WARN,       /* 一般警告 */
    BUZZER_SCENE_ALARM       /* 紧急报警 */
} BuzzerScene_t;

/* 鸣叫模式 */
typedef enum {
    BUZZER_MODE_CONTINUOUS,   // 持续鸣叫
    BUZZER_MODE_INTERMITTENT  // 间歇鸣叫
} BuzzerMode_t;

/* 间歇鸣叫节奏模式枚举 */
typedef enum {
    BUZZER_RHYTHM_NONE,
    BUZZER_RHYTHM_FAST,      /* 急促：响80ms，停50ms */
    BUZZER_RHYTHM_NORMAL,    /* 标准：响150ms，停100ms */
    BUZZER_RHYTHM_SLOW,      /* 缓慢：响300ms，停200ms */
    BUZZER_RHYTHM_ALARM      /* 报警：响200ms，停100ms */
} BuzzerRhythm_t;

/* 错误码 */
#define BUZZER_EOK             0
#define BUZZER_ERROR           1

/* 操作函数 */
uint8_t buzzer_init(void);
uint8_t buzzer_power_on(void);
uint8_t buzzer_power_off(void);
   void buzzer_beep_continuous(uint16_t half_us, uint16_t duration_ms);
   void buzzer_beep_intermittent(uint16_t half_us, uint32_t duration_ms, BuzzerRhythm_t rhythm);
   void buzzer_beep_scene(BuzzerScene_t scene, BuzzerMode_t mode, BuzzerRhythm_t rhythm, uint16_t duration_ms);



#endif
