/**
 * @file    bh1750.h
 * @brief   BH1750光照传感器驱动头文件
 * @author  Lora项目组
 * @date    2024-01-15
 * @version 1.0.0
 */

#ifndef __BH1750_H
#define __BH1750_H

#include "bh1750_i2c.h"
#include "stm32f10x.h"
#include "delay.h"
#include "i2c.h"

/* 引脚定义 */
#define BH1750_ADDR_GPIO_PORT              GPIOB
#define BH1750_ADDR_GPIO_PIN               GPIO_Pin_5               
#define BH1750_ADDR_GPIO_CLK_ENABLE()      do{ RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE); }while(0)

/* IO 操作 */
#define BH1750_ADDR(x)                     do{ (x) ?                                                                        \
                                                        GPIO_WriteBit(BH1750_ADDR_GPIO_PORT, BH1750_ADDR_GPIO_PIN, Bit_SET) :     \
                                                        GPIO_WriteBit(BH1750_ADDR_GPIO_PORT, BH1750_ADDR_GPIO_PIN, Bit_RESET);    \
                                                    }while(0)

/* 命令定义 */
#define BH1750_CMD_POWER_DOWN       0x00  /* 断电 */
#define BH1750_CMD_POWER_ON         0x01  /* 通电 */
#define BH1750_CMD_RESET            0x07  /* 复位数据寄存器 */
#define BH1750_CMD_CONT_H_RES       0x10  /* 连续高分辨率模式 (1 lx) */
#define BH1750_CMD_CONT_H_RES2      0x11  /* 连续高分辨率模式2 (0.5 lx) */
#define BH1750_CMD_CONT_L_RES       0x13  /* 连续低分辨率模式 (4 lx) */
#define BH1750_CMD_ONE_H_RES        0x20  /* 一次高分辨率模式 */
#define BH1750_CMD_ONE_H_RES2       0x21  /* 一次高分辨率模式2 */
#define BH1750_CMD_ONE_L_RES        0x23  /* 一次低分辨率模式 */

/* 测量时间（毫秒） */
#define BH1750_MEAS_TIME_H_RES      180   /* 高分辨率模式最大测量时间 */
#define BH1750_MEAS_TIME_H_RES2     180   /* 高分辨率模式2最大测量时间 */
#define BH1750_MEAS_TIME_L_RES       24   /* 低分辨率模式最大测量时间 */

/* 测量模式枚举（简化上层调用） */
typedef enum {
    BH1750_MODE_CONT_H_RES = 0,   /* 连续高分辨率 */
    BH1750_MODE_CONT_H_RES2,      /* 连续高分辨率2 */
    BH1750_MODE_CONT_L_RES,       /* 连续低分辨率 */
    BH1750_MODE_ONE_H_RES,        /* 单次高分辨率 */
    BH1750_MODE_ONE_H_RES2,       /* 单次高分辨率2 */
    BH1750_MODE_ONE_L_RES         /* 单次低分辨率 */
} BH1750_Mode_t;

/* 设备结构体（便于管理多个传感器） */
typedef struct {
    I2C_TypeDef *I2Cx;      /* 使用的 I2C 外设 */
    uint8_t addr;           /* 从机地址（左对齐） */
    BH1750_Mode_t mode;     /* 当前测量模式 */
    float lux;              /* 最新光照值 (lx) */
} BH1750_Dev_t;

/* 函数声明 */
uint8_t bh1750_init(void);
uint8_t bh1750_power_on(void);
uint8_t bh1750_power_down(void);
uint8_t bh1750_reset(void);
uint8_t bh1750_set_mode(BH1750_Mode_t mode);
uint8_t bh1750_read_lux(float *lux);
uint8_t bh1750_measure(float *lux);

#endif
