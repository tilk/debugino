#include "uart_helper.h"
#include "stm32f1xx_ll_usart.h"

void UARTHelper_Init(UARTHelper_HandleTypeDef *huarth, UART_HandleTypeDef *huart, osMessageQueueId_t queueTX, osMessageQueueId_t queueRX, osSemaphoreId_t semTX)
{
  huarth->huart = huart;
  huarth->queueTX = queueTX;
  huarth->queueRX = queueRX;
  huarth->semTX = semTX;

  osSemaphoreAcquire(huarth->semTX, osWaitForever);
/*
  LL_USART_EnableIT_IDLE(huarth->huart->Instance);
  __HAL_DMA_ENABLE_IT(huarth->huart->hdmarx, DMA_IT_TC);
  HAL_UART_Receive_DMA(huarth->huart, huarth->DMA_RX_Buffer, DMA_RX_BUFFER_SIZE);
  huarth->huart->RxState = HAL_UART_STATE_READY;
  */
}

void UARTHelper_IRQHandler(UARTHelper_HandleTypeDef *huarth)
{
  if (LL_USART_IsActiveFlag_IDLE(huarth->huart->Instance)) {
    LL_USART_ClearFlag_IDLE(huarth->huart->Instance);
  }
}

void UARTHelper_RXDMAIRQHandler(UARTHelper_HandleTypeDef *huarth)
{
  if (__HAL_DMA_GET_IT_SOURCE(huarth->huart->hdmarx, DMA_IT_HT) != RESET) {
    __HAL_DMA_CLEAR_FLAG(huarth->huart->hdmarx, __HAL_DMA_GET_HT_FLAG_INDEX(huarth->huart->hdmarx));
  }
  if (__HAL_DMA_GET_IT_SOURCE(huarth->huart->hdmarx, DMA_IT_TC) != RESET) {
    __HAL_DMA_CLEAR_FLAG(huarth->huart->hdmarx, __HAL_DMA_GET_TC_FLAG_INDEX(huarth->huart->hdmarx));
  }
}

void UARTHelper_TXCpltHandler(UARTHelper_HandleTypeDef *huarth)
{
  osSemaphoreRelease(huarth->semTX);
}

void UARTHelper_TX(UARTHelper_HandleTypeDef *huarth)
{
  int n = 0; 
  osStatus_t status = osMessageQueueGet(huarth->queueTX, &huarth->DMA_TX_Buffer[n], NULL, osWaitForever);
  while (status == osOK) {
    n++;
    if (n >= DMA_TX_BUFFER_SIZE) break;
    status = osMessageQueueGet(huarth->queueTX, &huarth->DMA_TX_Buffer[n], NULL, 0);
  }
  if (n > 0) {
    HAL_UART_Transmit_DMA(huarth->huart, huarth->DMA_TX_Buffer, n);
    osSemaphoreAcquire(huarth->semTX, osWaitForever);
  }
}

