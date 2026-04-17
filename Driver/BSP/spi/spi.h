#include "stm32f10x.h"	

/**
 * @brief       使用SPI总线收发数据
 * @param       SPIx: 所用的SPI接口的名称，可以填SPI1或SPI2
 * @param       pDataTx: 要发送的数据（数组）
 * @param       pDataRx: 接收数据缓冲区（数组），从从机接收到的数据被保存在这个参数当中
 * @param       Size: 要收发数据的数量，以字节为单位
 */
void My_SPI_MasterTransmitReceive(SPI_TypeDef *SPIx, const uint8_t *pDataTx, uint8_t *pDataRx, uint16_t Size);
