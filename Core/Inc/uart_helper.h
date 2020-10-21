#ifndef __UART_HELPER_H
#define __UART_HELPER_H

#include "stm32f1xx_hal.h"
#include "cmsis_os2.h"

#define DMA_RX_BUFFER_SIZE 64
#define DMA_TX_BUFFER_SIZE 64

typedef struct {
  UART_HandleTypeDef *huart;
  uint8_t DMA_RX_Buffer[DMA_RX_BUFFER_SIZE];
  uint8_t DMA_TX_Buffer[DMA_TX_BUFFER_SIZE];
  size_t position_RX;
  osMessageQueueId_t queueTX, queueRX;
  osSemaphoreId_t semTX;
} UARTHelper_HandleTypeDef;

void UARTHelper_Init(UARTHelper_HandleTypeDef *huarth, UART_HandleTypeDef *huart, osMessageQueueId_t queueTX, osMessageQueueId_t queueRX, osSemaphoreId_t semTX);

void UARTHelper_IRQHandler(UARTHelper_HandleTypeDef *huarth);
void UARTHelper_RXDMAIRQHandler(UARTHelper_HandleTypeDef *huarth);
void UARTHelper_TXCpltHandler(UARTHelper_HandleTypeDef *huarth);

void UARTHelper_TX(UARTHelper_HandleTypeDef *huarth);

void UARTHelper_SendBreak(UARTHelper_HandleTypeDef *huarth);
size_t UARTHelper_Send(UARTHelper_HandleTypeDef *huarth, uint8_t *buf, size_t len);
size_t UARTHelper_TrySend(UARTHelper_HandleTypeDef *huarth, uint8_t *buf, size_t len);
size_t UARTHelper_TrySend1(UARTHelper_HandleTypeDef *huarth, uint8_t *buf, size_t len);
size_t UARTHelper_Recv(UARTHelper_HandleTypeDef *huarth, uint8_t *buf, size_t len);
size_t UARTHelper_TryRecv(UARTHelper_HandleTypeDef *huarth, uint8_t *buf, size_t len);
size_t UARTHelper_TryRecv1(UARTHelper_HandleTypeDef *huarth, uint8_t *buf, size_t len);

#endif
