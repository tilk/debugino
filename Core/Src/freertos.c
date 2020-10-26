/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "usart.h"
#include "uart_helper.h"
#include "debugino.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
typedef StaticTask_t osStaticThreadDef_t;
typedef StaticQueue_t osStaticMessageQDef_t;
typedef StaticSemaphore_t osStaticSemaphoreDef_t;
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define UART_TX_BUFFER_SIZE 16
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for debugger */
osThreadId_t debuggerHandle;
const osThreadAttr_t debugger_attributes = {
  .name = "debugger",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 256 * 4
};
/* Definitions for uartSender1 */
osThreadId_t uartSender1Handle;
uint32_t uartSender1Buffer[ 128 ];
osStaticThreadDef_t uartSender1ControlBlock;
const osThreadAttr_t uartSender1_attributes = {
  .name = "uartSender1",
  .stack_mem = &uartSender1Buffer[0],
  .stack_size = sizeof(uartSender1Buffer),
  .cb_mem = &uartSender1ControlBlock,
  .cb_size = sizeof(uartSender1ControlBlock),
  .priority = (osPriority_t) osPriorityHigh,
};
/* Definitions for uartSender2 */
osThreadId_t uartSender2Handle;
uint32_t uartSender2Buffer[ 128 ];
osStaticThreadDef_t uartSender2ControlBlock;
const osThreadAttr_t uartSender2_attributes = {
  .name = "uartSender2",
  .stack_mem = &uartSender2Buffer[0],
  .stack_size = sizeof(uartSender2Buffer),
  .cb_mem = &uartSender2ControlBlock,
  .cb_size = sizeof(uartSender2ControlBlock),
  .priority = (osPriority_t) osPriorityHigh,
};
/* Definitions for queueUARTtoUSB */
osMessageQueueId_t queueUARTtoUSBHandle;
uint8_t queueUARTtoUSBBuffer[ 128 * sizeof( uint8_t ) ];
osStaticMessageQDef_t queueUARTtoUSBControlBlock;
const osMessageQueueAttr_t queueUARTtoUSB_attributes = {
  .name = "queueUARTtoUSB",
  .cb_mem = &queueUARTtoUSBControlBlock,
  .cb_size = sizeof(queueUARTtoUSBControlBlock),
  .mq_mem = &queueUARTtoUSBBuffer,
  .mq_size = sizeof(queueUARTtoUSBBuffer)
};
/* Definitions for queueUSBtoUART */
osMessageQueueId_t queueUSBtoUARTHandle;
uint8_t queueUSBtoUARTBuffer[ 128 * sizeof( uint8_t ) ];
osStaticMessageQDef_t queueUSBtoUARTControlBlock;
const osMessageQueueAttr_t queueUSBtoUART_attributes = {
  .name = "queueUSBtoUART",
  .cb_mem = &queueUSBtoUARTControlBlock,
  .cb_size = sizeof(queueUSBtoUARTControlBlock),
  .mq_mem = &queueUSBtoUARTBuffer,
  .mq_size = sizeof(queueUSBtoUARTBuffer)
};
/* Definitions for queueDWIREtoDEBUG */
osMessageQueueId_t queueDWIREtoDEBUGHandle;
uint8_t queueDWIREtoDEBUGBuffer[ 128 * sizeof( uint8_t ) ];
osStaticMessageQDef_t queueDWIREtoDEBUGControlBlock;
const osMessageQueueAttr_t queueDWIREtoDEBUG_attributes = {
  .name = "queueDWIREtoDEBUG",
  .cb_mem = &queueDWIREtoDEBUGControlBlock,
  .cb_size = sizeof(queueDWIREtoDEBUGControlBlock),
  .mq_mem = &queueDWIREtoDEBUGBuffer,
  .mq_size = sizeof(queueDWIREtoDEBUGBuffer)
};
/* Definitions for queueDEBUGtoDWIRE */
osMessageQueueId_t queueDEBUGtoDWIREHandle;
uint8_t queueDEBUGtoDWIREBuffer[ 128 * sizeof( uint8_t ) ];
osStaticMessageQDef_t queueDEBUGtoDWIREControlBlock;
const osMessageQueueAttr_t queueDEBUGtoDWIRE_attributes = {
  .name = "queueDEBUGtoDWIRE",
  .cb_mem = &queueDEBUGtoDWIREControlBlock,
  .cb_size = sizeof(queueDEBUGtoDWIREControlBlock),
  .mq_mem = &queueDEBUGtoDWIREBuffer,
  .mq_size = sizeof(queueDEBUGtoDWIREBuffer)
};
/* Definitions for queueDEBUGtoUSB */
osMessageQueueId_t queueDEBUGtoUSBHandle;
uint8_t queueDEBUGtoUSBBuffer[ 128 * sizeof( uint8_t ) ];
osStaticMessageQDef_t queueDEBUGtoUSBControlBlock;
const osMessageQueueAttr_t queueDEBUGtoUSB_attributes = {
  .name = "queueDEBUGtoUSB",
  .cb_mem = &queueDEBUGtoUSBControlBlock,
  .cb_size = sizeof(queueDEBUGtoUSBControlBlock),
  .mq_mem = &queueDEBUGtoUSBBuffer,
  .mq_size = sizeof(queueDEBUGtoUSBBuffer)
};
/* Definitions for queueUSBtoDEBUG */
osMessageQueueId_t queueUSBtoDEBUGHandle;
uint8_t queueUSBtoDEBUGBuffer[ 128 * sizeof( uint8_t ) ];
osStaticMessageQDef_t queueUSBtoDEBUGControlBlock;
const osMessageQueueAttr_t queueUSBtoDEBUG_attributes = {
  .name = "queueUSBtoDEBUG",
  .cb_mem = &queueUSBtoDEBUGControlBlock,
  .cb_size = sizeof(queueUSBtoDEBUGControlBlock),
  .mq_mem = &queueUSBtoDEBUGBuffer,
  .mq_size = sizeof(queueUSBtoDEBUGBuffer)
};
/* Definitions for txSemaphore1 */
osSemaphoreId_t txSemaphore1Handle;
osStaticSemaphoreDef_t txSemaphore1ControlBlock;
const osSemaphoreAttr_t txSemaphore1_attributes = {
  .name = "txSemaphore1",
  .cb_mem = &txSemaphore1ControlBlock,
  .cb_size = sizeof(txSemaphore1ControlBlock),
};
/* Definitions for txSemaphore2 */
osSemaphoreId_t txSemaphore2Handle;
osStaticSemaphoreDef_t txSemaphore2ControlBlock;
const osSemaphoreAttr_t txSemaphore2_attributes = {
  .name = "txSemaphore2",
  .cb_mem = &txSemaphore2ControlBlock,
  .cb_size = sizeof(txSemaphore2ControlBlock),
};
/* Definitions for breakRxSemaphore */
osSemaphoreId_t breakRxSemaphoreHandle;
osStaticSemaphoreDef_t breakRxSemaphoreControlBlock;
const osSemaphoreAttr_t breakRxSemaphore_attributes = {
  .name = "breakRxSemaphore",
  .cb_mem = &breakRxSemaphoreControlBlock,
  .cb_size = sizeof(breakRxSemaphoreControlBlock),
};
/* Definitions for breakTxSemaphore */
osSemaphoreId_t breakTxSemaphoreHandle;
osStaticSemaphoreDef_t breakTxSemaphoreControlBlock;
const osSemaphoreAttr_t breakTxSemaphore_attributes = {
  .name = "breakTxSemaphore",
  .cb_mem = &breakTxSemaphoreControlBlock,
  .cb_size = sizeof(breakTxSemaphoreControlBlock),
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDebugger(void *argument);
void StartUartSender1(void *argument);
void StartUartSender2(void *argument);

extern void MX_USB_DEVICE_Init(void);
void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* Create the semaphores(s) */
  /* creation of txSemaphore1 */
  txSemaphore1Handle = osSemaphoreNew(1, 1, &txSemaphore1_attributes);

  /* creation of txSemaphore2 */
  txSemaphore2Handle = osSemaphoreNew(1, 1, &txSemaphore2_attributes);

  /* creation of breakRxSemaphore */
  breakRxSemaphoreHandle = osSemaphoreNew(1, 1, &breakRxSemaphore_attributes);

  /* creation of breakTxSemaphore */
  breakTxSemaphoreHandle = osSemaphoreNew(1, 1, &breakTxSemaphore_attributes);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of queueUARTtoUSB */
  queueUARTtoUSBHandle = osMessageQueueNew (128, sizeof(uint8_t), &queueUARTtoUSB_attributes);

  /* creation of queueUSBtoUART */
  queueUSBtoUARTHandle = osMessageQueueNew (128, sizeof(uint8_t), &queueUSBtoUART_attributes);

  /* creation of queueDWIREtoDEBUG */
  queueDWIREtoDEBUGHandle = osMessageQueueNew (128, sizeof(uint8_t), &queueDWIREtoDEBUG_attributes);

  /* creation of queueDEBUGtoDWIRE */
  queueDEBUGtoDWIREHandle = osMessageQueueNew (128, sizeof(uint8_t), &queueDEBUGtoDWIRE_attributes);

  /* creation of queueDEBUGtoUSB */
  queueDEBUGtoUSBHandle = osMessageQueueNew (128, sizeof(uint8_t), &queueDEBUGtoUSB_attributes);

  /* creation of queueUSBtoDEBUG */
  queueUSBtoDEBUGHandle = osMessageQueueNew (128, sizeof(uint8_t), &queueUSBtoDEBUG_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  UARTHelper_Init(&huarth1, &huart1, queueUSBtoUARTHandle, queueUARTtoUSBHandle, txSemaphore1Handle, NULL, NULL);
  UARTHelper_Init(&huarth3, &huart3, queueDEBUGtoDWIREHandle, queueDWIREtoDEBUGHandle, txSemaphore2Handle, breakTxSemaphoreHandle, breakRxSemaphoreHandle);
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of debugger */
  debuggerHandle = osThreadNew(StartDebugger, NULL, &debugger_attributes);

  /* creation of uartSender1 */
  uartSender1Handle = osThreadNew(StartUartSender1, NULL, &uartSender1_attributes);

  /* creation of uartSender2 */
  uartSender2Handle = osThreadNew(StartUartSender2, NULL, &uartSender2_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_StartDebugger */
/**
  * @brief  Function implementing the debugger thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDebugger */
void StartDebugger(void *argument)
{
  /* init code for USB_DEVICE */
  MX_USB_DEVICE_Init();
  /* USER CODE BEGIN StartDebugger */
  /* Infinite loop */
  DebugInit(&huarth3);
  for(;;)
  {
    DebugLoop();
  }
  /* USER CODE END StartDebugger */
}

/* USER CODE BEGIN Header_StartUartSender1 */
/**
* @brief Function implementing the uartSender1 thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartUartSender1 */
void StartUartSender1(void *argument)
{
  /* USER CODE BEGIN StartUartSender1 */
  /* Infinite loop */
  for(;;)
  {
    UARTHelper_TX(&huarth1);
  }
  /* USER CODE END StartUartSender1 */
}

/* USER CODE BEGIN Header_StartUartSender2 */
/**
* @brief Function implementing the uartSender2 thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartUartSender2 */
void StartUartSender2(void *argument)
{
  /* USER CODE BEGIN StartUartSender2 */
  /* Infinite loop */
  for(;;)
  {
    UARTHelper_TX(&huarth3);
  }
  /* USER CODE END StartUartSender2 */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
