/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
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

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LED_BP_Pin GPIO_PIN_13
#define LED_BP_GPIO_Port GPIOC
#define FSM_STATE_Pin GPIO_PIN_0
#define FSM_STATE_GPIO_Port GPIOA
#define WIFI_STATE_Pin GPIO_PIN_1
#define WIFI_STATE_GPIO_Port GPIOA
#define OUT_24_48V_Pin GPIO_PIN_4
#define OUT_24_48V_GPIO_Port GPIOA
#define FAN_OUT_Pin GPIO_PIN_5
#define FAN_OUT_GPIO_Port GPIOA
#define LOAD2_Pin GPIO_PIN_6
#define LOAD2_GPIO_Port GPIOA
#define LOAD1_Pin GPIO_PIN_7
#define LOAD1_GPIO_Port GPIOA
#define RS485_DE_Pin GPIO_PIN_1
#define RS485_DE_GPIO_Port GPIOB
#define TENSION_24_48_Pin GPIO_PIN_3
#define TENSION_24_48_GPIO_Port GPIOB
#define AUTO_MANUAL_Pin GPIO_PIN_4
#define AUTO_MANUAL_GPIO_Port GPIOB
#define PULSADOR_Pin GPIO_PIN_5
#define PULSADOR_GPIO_Port GPIOB
#define PULSADOR_EXTI_IRQn EXTI9_5_IRQn

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
