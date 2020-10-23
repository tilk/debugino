#include "uart_helper.h"
#include "stm32f1xx_ll_usart.h"
#include "queue_io.h"

void UARTHelper_Init(UARTHelper_HandleTypeDef *huarth, UART_HandleTypeDef *huart, osMessageQueueId_t queueTX, osMessageQueueId_t queueRX, osSemaphoreId_t semTX, osSemaphoreId_t semBrkTX, osSemaphoreId_t semBrkRX)
{
  huarth->huart = huart;
  huarth->queueTX = queueTX;
  huarth->queueRX = queueRX;
  huarth->semTX = semTX;
  huarth->semBrkTX = semBrkTX;
  huarth->semBrkRX = semBrkRX;
  huarth->position_RX = 0;

  osSemaphoreAcquire(huarth->semTX, osWaitForever);
  if (huarth->semBrkRX) osSemaphoreAcquire(huarth->semBrkRX, osWaitForever);
  if (huarth->semBrkTX) osSemaphoreAcquire(huarth->semBrkTX, osWaitForever);

  LL_USART_EnableIT_IDLE(huarth->huart->Instance);
  if (huarth->semBrkRX) LL_USART_EnableIT_LBD(huarth->huart->Instance);
  HAL_DMA_Start_IT(huarth->huart->hdmarx, (uint32_t)&huarth->huart->Instance->DR, (uint32_t) huarth->DMA_RX_Buffer, DMA_RX_BUFFER_SIZE);
  __HAL_DMA_ENABLE_IT(huarth->huart->hdmarx, DMA_IT_TC | DMA_IT_HT);
  LL_USART_EnableDMAReq_RX(huarth->huart->Instance);
}

void UARTHelper_HandleRX(UARTHelper_HandleTypeDef *huarth)
{
  size_t pos = DMA_RX_BUFFER_SIZE - __HAL_DMA_GET_COUNTER(huarth->huart->hdmarx);
  if (pos == DMA_RX_BUFFER_SIZE) pos = 0;
  while (huarth->position_RX != pos) {
    osStatus_t status = osMessageQueuePut(huarth->queueRX, &huarth->DMA_RX_Buffer[huarth->position_RX], 0, 0);
    if (status != osOK) break;
    huarth->position_RX++;
    if (huarth->position_RX == DMA_RX_BUFFER_SIZE)
        huarth->position_RX = 0;
  }
}

void UARTHelper_IRQHandler(UARTHelper_HandleTypeDef *huarth)
{
  if (LL_USART_IsActiveFlag_IDLE(huarth->huart->Instance)) {
    LL_USART_ClearFlag_IDLE(huarth->huart->Instance);
    UARTHelper_HandleRX(huarth);
  }
  if (LL_USART_IsActiveFlag_LBD(huarth->huart->Instance)) {
    LL_USART_ClearFlag_LBD(huarth->huart->Instance);
    if (huarth->semBrkRX) osSemaphoreRelease(huarth->semBrkRX);
  }
}

void UARTHelper_RXDMAIRQHandler(UARTHelper_HandleTypeDef *huarth)
{
  if (__HAL_DMA_GET_IT_SOURCE(huarth->huart->hdmarx, DMA_IT_HT) != RESET) {
    __HAL_DMA_CLEAR_FLAG(huarth->huart->hdmarx, __HAL_DMA_GET_HT_FLAG_INDEX(huarth->huart->hdmarx));
    UARTHelper_HandleRX(huarth);
  }
  if (__HAL_DMA_GET_IT_SOURCE(huarth->huart->hdmarx, DMA_IT_TC) != RESET) {
    __HAL_DMA_CLEAR_FLAG(huarth->huart->hdmarx, __HAL_DMA_GET_TC_FLAG_INDEX(huarth->huart->hdmarx));
    UARTHelper_HandleRX(huarth);
  }
}

void UARTHelper_TXCpltHandler(UARTHelper_HandleTypeDef *huarth)
{
  osSemaphoreRelease(huarth->semTX);
}

void UARTHelper_TX(UARTHelper_HandleTypeDef *huarth)
{
  int n = 0; 
  osStatus_t status = osMessageQueueGet(huarth->queueTX, &huarth->DMA_TX_Buffer[n], 0, 0);
  if (status != osOK) {
    if (huarth->semBrkTX) osSemaphoreRelease(huarth->semBrkTX);
    status = osMessageQueueGet(huarth->queueTX, &huarth->DMA_TX_Buffer[n], 0, osWaitForever);
    if (huarth->semBrkTX) osSemaphoreAcquire(huarth->semBrkTX, osWaitForever);
  }
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

void UARTHelper_SendBreak(UARTHelper_HandleTypeDef *huarth)
{
  osSemaphoreAcquire(huarth->semBrkTX, osWaitForever);
  LL_USART_RequestBreakSending(huarth->huart->Instance);
  osSemaphoreRelease(huarth->semBrkTX);
}

bool UARTHelper_PollBreak(UARTHelper_HandleTypeDef *huarth)
{
  return osSemaphoreAcquire(huarth->semBrkRX, 0) == osOK;
}

bool UARTHelper_PollRecv(UARTHelper_HandleTypeDef *huarth)
{
  return Queue_PollRecv(huarth->queueRX);
}

size_t UARTHelper_Send(UARTHelper_HandleTypeDef *huarth, uint8_t *buf, size_t len)
{
  return Queue_Send(huarth->queueTX, buf, len);
}

size_t UARTHelper_TrySend(UARTHelper_HandleTypeDef *huarth, uint8_t *buf, size_t len)
{
  return Queue_TrySend(huarth->queueTX, buf, len);
}

size_t UARTHelper_TrySend1(UARTHelper_HandleTypeDef *huarth, uint8_t *buf, size_t len)
{
  return Queue_TrySend1(huarth->queueTX, buf, len);
}

size_t UARTHelper_Recv(UARTHelper_HandleTypeDef *huarth, uint8_t *buf, size_t len)
{
  return Queue_Recv(huarth->queueRX, buf, len);
}

size_t UARTHelper_TryRecv(UARTHelper_HandleTypeDef *huarth, uint8_t *buf, size_t len)
{
  return Queue_TryRecv(huarth->queueRX, buf, len);
}

size_t UARTHelper_TryRecv1(UARTHelper_HandleTypeDef *huarth, uint8_t *buf, size_t len)
{
  return Queue_TryRecv1(huarth->queueRX, buf, len);
}


