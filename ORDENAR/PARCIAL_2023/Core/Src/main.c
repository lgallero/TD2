/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

typedef struct{
	uint8_t id;
	uint32_t Y;
	GPIO_TypeDef *S1_port;
	GPIO_TypeDef *S2_port;
	GPIO_TypeDef *O1_port;
	GPIO_TypeDef *O2_port;
	GPIO_TypeDef *Pul_port;
	uint16_t S1_pin;
	uint16_t S2_pin;
	uint16_t O1_pin;
	uint16_t O2_pin;
	uint16_t Pul_pin;
	xSemaphoreHandle semaf1;
	xSemaphoreHandle semaf2;
	xSemaphoreHandle semaf_pul;
} TASK_DATA_T;



/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define QUEUE_LEN (5)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart2;

/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* USER CODE BEGIN PV */
static uint8_t dato_cola;
static QueueHandle_t queue_uart;
static TASK_DATA_T tanque1, tanque2;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
void StartDefaultTask(void *argument);

/* USER CODE BEGIN PFP */

static void initTaskData (void)
{
		tanque1.id = 0;
		tanque1.Y = 10000;
		tanque1.S1_port= S1_1_GPIO_Port;
		tanque1.S2_port= S2_1_GPIO_Port;
		tanque1.O1_port= O1_1_GPIO_Port;
		tanque1.O2_port= O2_1_GPIO_Port;
		tanque1.Pul_port= I1_1_GPIO_Port;
		tanque1.S1_pin= S1_1_Pin;
		tanque1.S2_pin= S2_1_Pin;
		tanque1.O1_pin= O1_1_Pin;
		tanque1.O2_pin= O2_1_Pin;
		tanque1.Pul_pin= I1_1_Pin;
		tanque1.semaf1= xSemaphoreCreateBinary();
		tanque1.semaf2= xSemaphoreCreateBinary();
		tanque1.semaf_pul= xSemaphoreCreateBinary();

		tanque2.id = 1;
		tanque2.Y = 10000;
		tanque2.S1_port= S1_2_GPIO_Port;
		tanque2.S2_port= S2_2_GPIO_Port;
		tanque2.O1_port= O1_2_GPIO_Port;
		tanque2.O2_port= O2_2_GPIO_Port;
		tanque2.Pul_port= I1_2_GPIO_Port;
		tanque2.S1_pin= S1_2_Pin;
		tanque2.S2_pin= S2_2_Pin;
		tanque2.O1_pin= O1_2_Pin;
		tanque2.O2_pin= O2_2_Pin;
		tanque2.Pul_pin= I1_2_Pin;
		tanque2.semaf1= xSemaphoreCreateBinary();
		tanque2.semaf2= xSemaphoreCreateBinary();
		tanque2.semaf_pul= xSemaphoreCreateBinary();


		HAL_GPIO_WritePin(O1_1_GPIO_Port, O1_1_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(O2_1_GPIO_Port, O2_1_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(O1_2_GPIO_Port, O1_2_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(O2_2_GPIO_Port, O2_2_Pin, GPIO_PIN_RESET);
}

void task_tanque (void *a)
{
	TASK_DATA_T *parametros;
	parametros = (TASK_DATA_T*) a;

	uint8_t item;

	xSemaphoreTake(parametros->semaf2,portMAX_DELAY);

		HAL_GPIO_WritePin(parametros->O2_port, parametros->O2_pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(parametros->O1_port, parametros->O1_pin, GPIO_PIN_SET);

		if(xSemaphoreTake(parametros->semaf1,parametros->Y) == pdTRUE)
		{
			HAL_GPIO_WritePin(parametros->O1_port, parametros->O1_pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(parametros->O2_port, parametros->O2_pin, GPIO_PIN_SET);
		}
		else {

			item = parametros->id;

			xQueueSend(queue_uart, &item, 0);
			HAL_GPIO_WritePin(parametros->O2_port, parametros->O2_pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(parametros->O1_port, parametros->O1_pin, GPIO_PIN_RESET);

			xSemaphoreTake(parametros->semaf_pul,portMAX_DELAY);
		}
}



void Task_uart (void *a)
{
		uint8_t item;
		uint8_t buff[20];

		while(1){
			xQueueReceive(queue_uart, &item, portMAX_DELAY);
			snprintf((char*)buff,20,"ALARMA: %d\r\n",(int)item);
			HAL_UART_Transmit(&huart2, buff, (uint16_t)strlen((char *)buff), 100);
		}
}

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  	xTaskCreate(task_tanque, (const char *)"taskTanque1", configMINIMAL_STACK_SIZE, &tanque1, osPriorityNormal, 0);
  	xTaskCreate(task_tanque, (const char *)"taskTanque2", configMINIMAL_STACK_SIZE, &tanque2, osPriorityNormal, 0);
  	xTaskCreate(Task_uart, (const char *)"task_uart", configMINIMAL_STACK_SIZE, NULL, osPriorityNormal, 0);

  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED_BP_GPIO_Port, LED_BP_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, O1_1_Pin|O2_1_Pin|O1_2_Pin|O2_2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : LED_BP_Pin */
  GPIO_InitStruct.Pin = LED_BP_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED_BP_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : I1_2_Pin I1_1_Pin */
  GPIO_InitStruct.Pin = I1_2_Pin|I1_1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : O1_1_Pin O2_1_Pin O1_2_Pin O2_2_Pin */
  GPIO_InitStruct.Pin = O1_1_Pin|O2_1_Pin|O1_2_Pin|O2_2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : S1_1_Pin S2_1_Pin S1_2_Pin S2_2_Pin */
  GPIO_InitStruct.Pin = S1_1_Pin|S2_1_Pin|S1_2_Pin|S2_2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN 5 */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END 5 */
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM1 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM1)
  {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
