#ifndef __BH1750_I2C_H
#define __BH1750_I2C_H

#include "stm32f10x.h"
#include "i2c.h"

/* 引脚定义 */
#define BH1750_I2C_SCL_GPIO_PORT           GPIOB
#define BH1750_I2C_SCL_GPIO_PIN            GPIO_Pin_6
#define BH1750_I2C_SCL_GPIO_CLK_ENABLE()   do{ RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE); }while(0)

#define BH1750_I2C_SDA_GPIO_PORT           GPIOB
#define BH1750_I2C_SDA_GPIO_PIN            GPIO_Pin_7               
#define BH1750_I2C_SDA_GPIO_CLK_ENABLE()   do{ RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE); }while(0)

#define BH1750_I2C_INTERFACE               I2C1
#define BH1750_I2C_CLK_ENABLE()            do{ RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE); }while(0)

/* 从机地址配置 */
#define BH1750_ADDR_STATE               0   /* 0: ADDR 接低电平 → 地址 0x46 */

#if (BH1750_ADDR_STATE == 0)
    #define BH1750_SLAVE_ADDR           0x46    /* 7位地址 0x23 << 1 */
#else
    #define BH1750_SLAVE_ADDR           0xB8    /* 7位地址 0x5C << 1 */
#endif

/* 错误码 */
#define BH1750_EOK                  0     /* 成功 */
#define BH1750_ERROR                1     /* 通用错误 */
#define BH1750_ETIMEOUT             2     /* 超时错误 */
#define BH1750_EINVAL               3     /* 参数无效 */

void bh1750_i2c_init(uint32_t clock_speed);
 int bh1750_i2c_write_cmd(uint8_t cmd);
 int bh1750_i2c_read_raw(uint16_t *raw);

#endif 
