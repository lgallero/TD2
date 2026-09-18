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
#define I1_2_Pin GPIO_PIN_1
#define I1_2_GPIO_Port GPIOB
#define I1_1_Pin GPIO_PIN_2
#define I1_1_GPIO_Port GPIOB
#define O1_1_Pin GPIO_PIN_12
#define O1_1_GPIO_Port GPIOB
#define O2_1_Pin GPIO_PIN_13
#define O2_1_GPIO_Port GPIOB
#define O1_2_Pin GPIO_PIN_14
#define O1_2_GPIO_Port GPIOB
#define O2_2_Pin GPIO_PIN_15
#define O2_2_GPIO_Port GPIOB
#define S1_1_Pin GPIO_PIN_8
#define S1_1_GPIO_Port GPIOA
#define S2_1_Pin GPIO_PIN_9
#define S2_1_GPIO_Port GPIOA
#define S1_2_Pin GPIO_PIN_10
#define S1_2_GPIO_Port GPIOA
#define S2_2_Pin GPIO_PIN_11
#define S2_2_GPIO_Port GPIOA

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
