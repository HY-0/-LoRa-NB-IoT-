#include "stm32f10x.h"

int i2c_send_bytes(I2C_TypeDef *I2Cx, uint8_t Addr, const uint8_t *pData, uint16_t Size);
int i2c_receive_bytes(I2C_TypeDef *I2Cx, uint8_t Addr, uint8_t *pBuffer, uint16_t Size);

