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

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
typedef StaticQueue_t osStaticMessageQDef_t;
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 128 * 4
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
/* Definitions for queueDWIREtoUSB */
osMessageQueueId_t queueDWIREtoUSBHandle;
uint8_t queueDWIREtoUSBBuffer[ 128 * sizeof( uint8_t ) ];
osStaticMessageQDef_t queueDWIREtoUSBControlBlock;
const osMessageQueueAttr_t queueDWIREtoUSB_attributes = {
  .name = "queueDWIREtoUSB",
  .cb_mem = &queueDWIREtoUSBControlBlock,
  .cb_size = sizeof(queueDWIREtoUSBControlBlock),
  .mq_mem = &queueDWIREtoUSBBuffer,
  .mq_size = sizeof(queueDWIREtoUSBBuffer)
};
/* Definitions for queueUSBtoDWIRE */
osMessageQueueId_t queueUSBtoDWIREHandle;
uint8_t queueUSBtoDWIREBuffer[ 128 * sizeof( uint8_t ) ];
osStaticMessageQDef_t queueUSBtoDWIREControlBlock;
const osMessageQueueAttr_t queueUSBtoDWIRE_attributes = {
  .name = "queueUSBtoDWIRE",
  .cb_mem = &queueUSBtoDWIREControlBlock,
  .cb_size = sizeof(queueUSBtoDWIREControlBlock),
  .mq_mem = &queueUSBtoDWIREBuffer,
  .mq_size = sizeof(queueUSBtoDWIREBuffer)
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);

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

  /* creation of queueDWIREtoUSB */
  queueDWIREtoUSBHandle = osMessageQueueNew (128, sizeof(uint8_t), &queueDWIREtoUSB_attributes);

  /* creation of queueUSBtoDWIRE */
  queueUSBtoDWIREHandle = osMessageQueueNew (128, sizeof(uint8_t), &queueUSBtoDWIRE_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* init code for USB_DEVICE */
  MX_USB_DEVICE_Init();
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  for(;;)
  {
//    osDelay(1);
    uint8_t data;
    osStatus_t status;
    status = osMessageQueueGet(queueUSBtoUARTHandle, &data, NULL, 0);
    if (status == osOK)
      osMessageQueuePut(queueUARTtoUSBHandle, &data, 0, 0);
    status = osMessageQueueGet(queueUSBtoDWIREHandle, &data, NULL, 0);
    if (status == osOK)
      osMessageQueuePut(queueDWIREtoUSBHandle, &data, 0, 0);
    
  }
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
