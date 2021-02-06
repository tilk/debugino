/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "cmsis_os2.h"
#include "uart_helper.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
void FlashLED(uint16_t led);

extern osMessageQueueId_t queueUARTtoUSBHandle;
extern osMessageQueueId_t queueUSBtoUARTHandle;
extern osMessageQueueId_t queueDWIREtoDEBUGHandle;
extern osMessageQueueId_t queueDEBUGtoDWIREHandle;
extern osMessageQueueId_t queueDEBUGtoUSBHandle;
extern osMessageQueueId_t queueUSBtoDEBUGHandle;
extern osSemaphoreId_t txSemaphore1Handle;
extern osSemaphoreId_t txSemaphore2Handle;
extern UARTHelper_HandleTypeDef huarth1;
extern UARTHelper_HandleTypeDef huarth3;
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LED_DEBUG_Pin GPIO_PIN_13
#define LED_DEBUG_GPIO_Port GPIOC
#define LED_RX_Pin GPIO_PIN_14
#define LED_RX_GPIO_Port GPIOC
#define LED_TX_Pin GPIO_PIN_15
#define LED_TX_GPIO_Port GPIOC
#define LED_POWER_Pin GPIO_PIN_2
#define LED_POWER_GPIO_Port GPIOA
#define CURRENT_Pin GPIO_PIN_3
#define CURRENT_GPIO_Port GPIOA
#define DWIRE_Pin GPIO_PIN_10
#define DWIRE_GPIO_Port GPIOB
#define CLKOUT_Pin GPIO_PIN_8
#define CLKOUT_GPIO_Port GPIOA
#define UART_TX_Pin GPIO_PIN_6
#define UART_TX_GPIO_Port GPIOB
#define UART_RX_Pin GPIO_PIN_7
#define UART_RX_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
