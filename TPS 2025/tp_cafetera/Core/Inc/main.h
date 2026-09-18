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
extern UART_HandleTypeDef huart1;
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
#define ESP_ENABLE_Pin GPIO_PIN_4
#define ESP_ENABLE_GPIO_Port GPIOA
#define LED_TESTIGO_Pin GPIO_PIN_12
#define LED_TESTIGO_GPIO_Port GPIOB
#define BUZZER_Pin GPIO_PIN_13
#define BUZZER_GPIO_Port GPIOB
#define EROGACION_CAFE_Pin GPIO_PIN_14
#define EROGACION_CAFE_GPIO_Port GPIOB
#define EROGACION_LECHE_Pin GPIO_PIN_15
#define EROGACION_LECHE_GPIO_Port GPIOB
#define CAFE_NEGRO_Pin GPIO_PIN_10
#define CAFE_NEGRO_GPIO_Port GPIOA
#define CAFE_NEGRO_EXTI_IRQn EXTI15_10_IRQn
#define INGRESO_FICHA_Pin GPIO_PIN_11
#define INGRESO_FICHA_GPIO_Port GPIOA
#define INGRESO_FICHA_EXTI_IRQn EXTI15_10_IRQn
#define CAFE_LECHE_Pin GPIO_PIN_15
#define CAFE_LECHE_GPIO_Port GPIOA
#define CAFE_LECHE_EXTI_IRQn EXTI15_10_IRQn

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
