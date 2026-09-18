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
#include "string.h"
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef struct TASK_DATA {
	uint16_t M_1_PIN;
	GPIO_TypeDef *M_1_PORT;
	uint16_t M_2_PIN;
	GPIO_TypeDef *M_2_PORT;

	uint8_t id;

	xSemaphoreHandle sem_BTN;
}TASK_DATA_T;

typedef struct {
	uint8_t id;
	uint32_t ampere;
	uint32_t volt;
	uint8_t condicion;
	uint8_t carga;
} queue_item_t;


/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define QUEUE_LEN (5)

#define VACIO 0
#define BAJA 1
#define MEDIA 2
#define ALTA 3

#define ALARMA 1
#define OK 0

#define V_MIN 5
#define V_MAX 150
#define I_MAX 5

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

UART_HandleTypeDef huart1;

/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* USER CODE BEGIN PV */

void wifi_send(char *str){}; // envío de mensajes a través del módulo WiFi

uint32_t medicion_corriente (uint8_t id) {return 0;}; // funcion para obterner el valor de Corriente segun medidor
uint32_t medicion_tension (uint8_t id) {return 0;};// funcion para obterner el valor de Tension segun medidor

uint8_t comprobar_medicion(uint8_t a ,TASK_DATA_T *p); // Devuelve 0 si las mediciones estan bien o 1 si hay error. Tambien carga en Queue los valores de Tension y Corriente

void conectarCarga(uint8_t CARGA, TASK_DATA_T *p);
void desconectarCarga(TASK_DATA_T *p);

static uint8_t dato_cola;
static QueueHandle_t queue_uart;

static TASK_DATA_T medidor1, medidor2;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_ADC1_Init(void);
static void MX_USART1_UART_Init(void);
void StartDefaultTask(void *argument);

/* USER CODE BEGIN PFP */

void conectarCarga(uint8_t CARGA, TASK_DATA_T *a){

	TASK_DATA_T *p;
	p = (TASK_DATA_T*)a;

	switch (CARGA) {

	case VACIO:
		HAL_GPIO_WritePin(p->M_1_PORT, p->M_1_PORT, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(p->M_2_PORT, p->M_2_PORT, GPIO_PIN_RESET);
		break;
	case BAJA:
		HAL_GPIO_WritePin(p->M_1_PORT, p->M_1_PORT, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(p->M_2_PORT, p->M_2_PORT, GPIO_PIN_SET);
		break;
	case MEDIA:
		HAL_GPIO_WritePin(p->M_1_PORT, p->M_1_PORT, GPIO_PIN_SET);
		HAL_GPIO_WritePin(p->M_2_PORT, p->M_2_PORT, GPIO_PIN_RESET);
		break;
	case ALTA:
		HAL_GPIO_WritePin(p->M_1_PORT, p->M_1_PORT, GPIO_PIN_SET);
		HAL_GPIO_WritePin(p->M_2_PORT, p->M_2_PORT, GPIO_PIN_SET);
		break;

	}

}

void desconectarCarga(TASK_DATA_T *p)
{
	HAL_GPIO_WritePin(p->M_1_PORT, p->M_1_PORT, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(p->M_2_PORT, p->M_2_PORT, GPIO_PIN_RESET);
}

uint8_t comprobar_medicion(uint8_t load ,TASK_DATA_T *p)
{
	uint32_t Ampere;
	uint32_t Volt;
	queue_item_t item;

	//obtengo los valores del ADC
	Ampere = medicion_corriente (p->id);
	Volt = medicion_tension(p->id);
	item.ampere = Ampere;
	item.volt = Volt;
	item.id = p->id;
	item.carga = load;

	if(Volt < V_MIN || Volt > V_MAX || Ampere > I_MAX){ // Condicion de Alarma
		item.condicion =  ALARMA;
	}
	else // Condiciones normales
	{
		item.condicion =  OK;
	}
	xQueueSend(queue_uart, &item, 0);
}

static void initTasksData(void)
{
	medidor1.M_1_PORT = M1_1_GPIO_Port;
	medidor1.M_1_PIN = M1_1_Pin;
	medidor1.M_2_PORT = M1_2_GPIO_Port;
	medidor1.M_2_PIN = M1_2_Pin;
	medidor1.sem_BTN = xSemaphoreCreateBinary();
	medidor1.id = 1;


	medidor2.M_1_PORT = M2_1_GPIO_Port;
	medidor2.M_1_PIN = M2_1_Pin;
	medidor1.M_2_PORT = M2_2_GPIO_Port;
	medidor1.M_2_PIN = M2_2_Pin;
	medidor2.sem_BTN = xSemaphoreCreateBinary();
	medidor2.id = 2;


	HAL_GPIO_WritePin(M1_1_GPIO_Port, M1_1_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(M1_2_GPIO_Port, M1_2_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(M2_1_GPIO_Port, M2_1_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(M2_2_GPIO_Port, M2_2_Pin, GPIO_PIN_RESET);

}

void task_medidor (void *a)
{
	TASK_DATA_T *p;
	p = (TASK_DATA_T*)a;
	uint8_t alarma = 0;
	uint8_t condiciones_normales = 0;
	uint8_t item;
	while(1){

		xSemaphoreTake(p->sem_BTN,portMAX_DELAY); // Espero que conecten el medidor y toquen el boton

		while(alarma == 0 || condiciones_normales == 0)
		{
			conectarCarga(VACIO, p); //Vacio
			vTaskDelay(2000); // 2 seg
			alarma = comprobar_medicion(VACIO,p);

			if(!alarma){
				vTaskDelay(3000);
				conectarCarga(BAJA, p);
				vTaskDelay(2000);
				alarma = comprobar_medicion(BAJA,p);
			}

			if(!alarma){
				vTaskDelay(3000);
				conectarCarga(MEDIA, p);
				vTaskDelay(2000);
				alarma = comprobar_medicion(MEDIA,p);
			}

			if(!alarma){
				vTaskDelay(3000);
				conectarCarga(ALTA, p);
				vTaskDelay(2000);
				alarma = comprobar_medicion(ALTA,p);
			}

			if(alarma == 0) // Todo bien
			{
				condiciones_normales = 1; // salgo del while
			}
		}

		if(alarma == 1) // Error
		{
			desconectarCarga(p);
			vTaskDelay(10000); // Espero 10 segundos de que ocurrio el error
		}

		// Reseteo banderas
		condiciones_normales = 0;
		alarma = 0;

	}

}

void Task_uart (void *a)
{

	queue_item_t item;
	uint8_t buff[20];

	while(1){
		xQueueReceive(queue_uart, &item, portMAX_DELAY);

		if(item.condicion == OK)
		{
			snprintf((char*)buff,20,"Transformador %d - Carga %d - %d V - %d A\r\n", item.id , item.carga , item.volt, item.ampere);
		}
		else if(item.condicion == ALARMA)
		{
			snprintf((char*)buff,20,"Transformador %d - ERROR - %d V - %d A\r\n",item.id , item.volt, item.ampere);
		}

		wifi_send(buff);
	}
}



void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	BaseType_t switch_required = pdFALSE;

	switch (GPIO_Pin) {

	case BTN_1_Pin:
		xSemaphoreGiveFromISR(medidor1.sem_BTN,&switch_required);
		break;

	case BTN_2_Pin:
		xSemaphoreGiveFromISR(medidor2.sem_BTN,&switch_required);
		break;
	default:
		break;
	}
	portEND_SWITCHING_ISR(switch_required);
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
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  initTasksData();
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
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */

  xTaskCreate(task_medidor, (const char *)"taskMedidor1", configMINIMAL_STACK_SIZE, &medidor1, osPriorityNormal, 0);
  xTaskCreate(task_medidor, (const char *)"taskMedidor2", configMINIMAL_STACK_SIZE, &medidor2, osPriorityNormal, 0);
  xTaskCreate(Task_uart, (const char *)"task_uart", configMINIMAL_STACK_SIZE, NULL, osPriorityNormal, 0);
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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

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
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV2;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);

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
  HAL_GPIO_WritePin(GPIOB, M1_1_Pin|M1_2_Pin|M2_1_Pin|M2_2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : BTN_2_Pin BTN_1_Pin */
  GPIO_InitStruct.Pin = BTN_2_Pin|BTN_1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : M1_1_Pin M1_2_Pin M2_1_Pin M2_2_Pin */
  GPIO_InitStruct.Pin = M1_1_Pin|M1_2_Pin|M2_1_Pin|M2_2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

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
