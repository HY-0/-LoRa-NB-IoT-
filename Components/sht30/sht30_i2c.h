#ifndef __SHT30_I2C_H
#define __SHT30_I2C_H

#include "stm32f10x.h"
#include "i2c.h"

/* ==================== I2C 硬件引脚定义（仿 LORA_UART 风格） ==================== */
#define SHT30_I2C_SCL_GPIO_PORT           GPIOB
#define SHT30_I2C_SCL_GPIO_PIN            GPIO_Pin_10
#define SHT30_I2C_SCL_GPIO_CLK_ENABLE()   do{ RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE); }while(0)

#define SHT30_I2C_SDA_GPIO_PORT           GPIOB
#define SHT30_I2C_SDA_GPIO_PIN            GPIO_Pin_11
#define SHT30_I2C_SDA_GPIO_CLK_ENABLE()   do{ RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE); }while(0)

#define SHT30_I2C_INTERFACE               I2C2
#define SHT30_I2C_CLK_ENABLE()            do{ RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, ENABLE); }while(0)

/* I2C 通信速率（标准模式 100kHz） */
#define SHT30_I2C_SPEED                   100000

/* ==================== 从机地址配置 ==================== */
#define SHT30_ADDR_LEVEL                  0   /* 0：ADDR 接低电平（地址 0x44），1：ADDR 接高电平（地址 0x45） */

#if (SHT30_ADDR_LEVEL == 0)
    #define SHT30_SLAVE_ADDR              0x88    /* 7位地址 0x44 << 1 */
#else
    #define SHT30_SLAVE_ADDR              0x8A    /* 7位地址 0x45 << 1 */
#endif

/* 错误码 */
#define SHT30_EOK                         0
#define SHT30_ERROR                       1

/* ==================== 底层 API ==================== */
void sht30_i2c_init(uint32_t clock_speed);
int  sht30_i2c_write_cmd(uint16_t cmd);
int  sht30_i2c_read_raw(uint8_t *buf, uint8_t len);

#endif /* __SHT30_I2C_H */
