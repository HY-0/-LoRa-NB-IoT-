#ifndef  __S2IC_H 
#define  __S2IC_H

#include "stm32f10x.h"
#include "oled.h"
#include "si2c.h"

/* 引脚定义 */
#define SI2C_SCL_GPIO_PORT           GPIOB
#define SI2C_SCL_GPIO_PIN            GPIO_Pin_8               
#define SI2C_SCL_GPIO_CLK_ENABLE()   do{ RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE); }while(0)

#define SI2C_SDA_GPIO_PORT           GPIOB
#define SI2C_SDA_GPIO_PIN            GPIO_Pin_9
#define SI2C_SDA_GPIO_CLK_ENABLE()   do{ RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE); }while(0)

/* IO操作 */
// #define SI2C_SCL_READ ((GPIO_ReadInputDataBit(SI2C->SCL_GPIOx, SI2C->SCL_GPIO_Pin) == Bit_SET) ? 1 : 0)
// #define SI2C_SDA_READ ((GPIO_ReadInputDataBit(SI2C->SDA_GPIOx, SI2C->SDA_GPIO_Pin) == Bit_SET) ? 1 : 0)

#define SI2C_SCL_READ() 	     GPIO_ReadInputDataBit(SI2C_SCL_GPIO_PORT, SI2C_SCL_GPIO_PIN)
#define SI2C_SDA_READ() 	     GPIO_ReadInputDataBit(SI2C_SDA_GPIO_PORT, SI2C_SDA_GPIO_PIN)

// 给宏参数加上括号是个好习惯
// #define scl_w(v) GPIO_WriteBit(SI2C_SCL_GPIO_PORT, SI2C_SCL_GPIO_PIN, ((v)?Bit_SET:Bit_RESET))
// #define sda_w(v) GPIO_WriteBit(SI2C_SDA_GPIO_PORT, SI2C_SDA_GPIO_PIN, ((v)?Bit_SET:Bit_RESET))
// #define scl_r ((GPIO_ReadInputDataBit(SI2C_SCL_GPIO_PORT, SI2C_SCL_GPIO_PIN) == Bit_SET) ? 1 : 0)
// #define sda_r ((GPIO_ReadInputDataBit(SI2C_SDA_GPIO_PORT, SI2C_SDA_GPIO_PIN) == Bit_SET) ? 1 : 0)


#define SI2C_SCL_WRITE(x)             do{ (x) ?                                                               \
                                             GPIO_WriteBit(SI2C_SCL_GPIO_PORT, SI2C_SCL_GPIO_PIN, Bit_SET) :  \
                                             GPIO_WriteBit(SI2C_SCL_GPIO_PORT, SI2C_SCL_GPIO_PIN, Bit_RESET); \
                                            }while(0)
#define SI2C_SDA_WRITE(x)             do{ (x) ?                                                                \
                                             GPIO_WriteBit(SI2C_SDA_GPIO_PORT, SI2C_SDA_GPIO_PIN, Bit_SET) :   \
                                             GPIO_WriteBit(SI2C_SDA_GPIO_PORT, SI2C_SDA_GPIO_PIN, Bit_RESET);  \
                                            }while(0)
          void si2c_init(void);
static uint8_t si2c_send_byte(uint8_t Byte);
           int si2c_send_bytes(uint8_t Addr, const uint8_t *pData, uint16_t Size);
static uint8_t si2c_receive_byte(uint8_t Ack);
           int si2c_receive_bytes(uint8_t Addr, uint8_t *pBuffer, uint16_t Size);
           int si2c_write_bytes(uint8_t addr, const uint8_t *pdata, uint16_t size);
   static void si2c_send_stop(void);

#endif 
