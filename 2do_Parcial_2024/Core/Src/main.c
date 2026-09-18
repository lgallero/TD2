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
	uint16_t SI_PIN;
	uint16_t SF_PIN;
	uint16_t BARRERA_PIN;
	uint16_t TIPO;
	xSemaphoreHandle sem_SI;
	xSemaphoreHandle sem_SF;
}TASK_DATA_T;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define S1_PIN (0) /* Pin sensor entrada barrera entrada */
#define S2_PIN (1) /* Pin sensor salida barrera entrada */
#define S3_PIN (2) /* Pin sensor entrada barrera salida */
#define S4_PIN (3) /* Pin sensor salida barrera salida */
#define O1_PIN (4) /* Pin para control barrera entrada */
#define O2_PIN (5) /* Pin para control barrera salida */
#define O3_PIN (6) /* Pin semáforo verde */
#define O4_PIN (7) /* Pin semáforo rojo */

#define QUEUE_LEN (5)

#define ENTRADA 0
#define SALIDA 1
#define O5_MINUTOS 2
#define TIEMPO 60000
#define ESPERA 300000
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

uint32_t leer_nro_autos(void){return 0;}; /* Thread-safe: no */
uint32_t max_autos(void){return 0;}; /* Thread-safe: si */
void decrementar_autos(void){}; /* Thread-safe: no */
void incrementar_autos(void){}; /* Thread-safe: no */
uint32_t ingreso_permitido(void){return 0;}; /* Thread-safe: si */

/* Escribe 0 o 1 en el pin */
void gpio_write(uint16_t pin, uint32_t valor){};
/* Devuelve 0 o 1 en función del estado del pin */
uint32_t gpio_read(uint16_t pin){return 0;};
/* Envía el mensaje str de len bytes utilizando interrupciones de UART */
uint32_t uart_send(uint8_t *str, uint16_t len);
/* Prototipo del callback del handler de interrupciones de GPIO */
void HAL_GPIO_EXTI_Callback(uint16_t pin);

static TASK_DATA_T entrada, salida;

static xSemaphoreHandle mutex_colector, sem_cont;

// Para la Queue
static QueueHandle_t queue_uart;
static uint8_t dato_cola;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
void StartDefaultTask(void *argument);

/* USER CODE BEGIN PFP */

static void initTasksData(void)
{
	entrada.SI_PIN = S1_PIN;
	entrada.SF_PIN = S2_PIN;
	entrada.BARRERA_PIN = O1_PIN;
	entrada.sem_SI = xSemaphoreCreateBinary();
	entrada.sem_SF = xSemaphoreCreateBinary();
	entrada.TIPO = ENTRADA;


	salida.SI_PIN = S3_PIN;
	salida.SF_PIN = S4_PIN;
	salida.BARRERA_PIN = O2_PIN;
	salida.sem_SI = xSemaphoreCreateBinary();
	salida.sem_SF = xSemaphoreCreateBinary();
	salida.TIPO = SALIDA;

	gpio_write(O1_PIN,0);
	gpio_write(O2_PIN,0);
	gpio_write(O3_PIN,0);
	gpio_write(O4_PIN,0);

	mutex_colector = xSemaphoreCreateMutex();
	sem_cont = xSemaphoreCreateBinary();
}

void taskBarrera (void *a){
	TASK_DATA_T *p;
	p = (TASK_DATA_T*)a;

	uint32_t cont = 0; // Para contar el tiempo de barrera
	uint8_t item;


	while(1)
	{
		if(leer_nro_autos() < max_autos()) // Hay lugares disponibles, prendo las luces
		{
			gpio_write(O3_PIN,1);
			gpio_write(O4_PIN,0);
		}
		else if(leer_nro_autos() == max_autos() ||ingreso_permitido() == 0 )
		{
			gpio_write(O3_PIN,0);
			gpio_write(O4_PIN,1);
		}
		xSemaphoreTake(p->sem_SI,portMAX_DELAY); // Tomo semafoto del sensor de ENTRADA
		if (leer_nro_autos() == max_autos()  || ingreso_permitido() == 0) // Si esta lleno o ingreso permitido es 0 no permito la entrada de vehiculos
		{
			if(p->TIPO == SALIDA) // Es del tipo salida
			{
				gpio_write(p->BARRERA_PIN, 1); // abro la barrera de SALIDA
				while (xSemaphoreTake(p->sem_SF,0) == pdFALSE && cont < TIEMPO) // Mientras no se active S2 y el contador sea menor a 1 min
				{
					cont++; // Contador suma
					vTaskDelay(1); // Espera 1ms
				}
				// Salio del While por dos razones: por TIEMPO o se activo S2

				if (cont < TIEMPO ) // Paso el auto porque se activo el semaforo
				{
					xSemaphoreGive(sem_cont); //Reseteo contador de 5 min
					// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
					xSemaphoreTake(mutex_colector,portMAX_DELAY);
					decrementar_autos();
					xSemaphoreGive(mutex_colector);
					// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

					item = p->TIPO;
					xQueueSend(queue_uart, &item, 0); // Mando el mensaje a la UART
				}
			}

		}
		else // Condiciones normales
		{
			gpio_write(p->BARRERA_PIN, 1);
			while (xSemaphoreTake(p->sem_SF,0) == pdFALSE && cont < TIEMPO) // Mientras no se active S2 y el contador sea menor a 1 min
			{
				cont++; // Contador suma
				vTaskDelay(1); //Espera 1ms
			}
			// Salio del While por dos razones: por TIEMPO o se activo S2

			if (cont < TIEMPO ) // Paso el auto porque se activo el semaforo
			{

				if(p->TIPO == ENTRADA){

					xSemaphoreGive(sem_cont); //Reseteo contador de 5 min
					// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
					xSemaphoreTake(mutex_colector,portMAX_DELAY);
					incrementar_autos();
					xSemaphoreGive(mutex_colector);
					// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

				}else if ( p->TIPO == SALIDA)
				{
					xSemaphoreGive(sem_cont); //Reseteo contador de 5 min
					// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
					xSemaphoreTake(mutex_colector,portMAX_DELAY);
					decrementar_autos();
					xSemaphoreGive(mutex_colector);
					// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
				}
				item = p->TIPO;
				xQueueSend(queue_uart, &item, 0); // Mando el mensaje a la UART
			}


		}

		gpio_write(p->BARRERA_PIN, 0); // Cierro la barrera
		cont = 0; // Reseteo el contador
	}
}


void Task_uart(void *p){

	uint8_t item;
	uint8_t buff[128];

	while(1){
		xQueueReceive(queue_uart, &item, portMAX_DELAY);

		if(item == ENTRADA)
		{
			snprintf((char*)buff,128," “Barrera de ENTRADA: %d-%d\r\n”",(int)leer_nro_autos(),(int)max_autos());
		}
		if(item == SALIDA)
		{
			snprintf((char*)buff,128," “Barrera de SALIDA: %d-%d\r\n”",(int)leer_nro_autos(),(int)max_autos());
		}
		if(item == O5_MINUTOS)
		{
			snprintf((char*)buff,128," “ESTADO: %d-%d\r\n”",(int)leer_nro_autos(),(int)max_autos());
		}

		HAL_UART_Transmit(&huart2, buff, (uint16_t)strlen((char *)buff), 100);
	}

}

void Task_cont (void *a) // Esta tarea va a contar el tiempo desde que no se abrio las barreras
{
	uint8_t item;
	uint32_t cont = 0;

	while(1)
	{
		while(xSemaphoreTake(sem_cont,0) == pdFALSE && cont < ESPERA) // Espero que se termine el tiempo o Pase un auto por las BARRERAS
		{
			cont++;
			vTaskDelay(1);
		}

		if ( cont  > ESPERA ) // 5 minutos desde que no hubo autos en las barreras
		{
			item = O5_MINUTOS;
			xQueueSend(queue_uart, &item, 0);
		}

		cont = 0;
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
	queue_uart = xQueueCreate(QUEUE_LEN, sizeof(dato_cola));
	/* USER CODE END RTOS_QUEUES */

	/* Create the thread(s) */
	/* creation of defaultTask */
	defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

	/* USER CODE BEGIN RTOS_THREADS */
	/* add threads, ... */
	xTaskCreate(taskBarrera, (const char *)"TaskBarrera_Entrada", configMINIMAL_STACK_SIZE, &entrada, osPriorityNormal, NULL);
	xTaskCreate(taskBarrera, (const char *)"TaskBarrera_Salida", configMINIMAL_STACK_SIZE, &salida, osPriorityNormal, NULL);
	xTaskCreate(Task_uart, (const char *)"task_uart", configMINIMAL_STACK_SIZE, NULL, osPriorityNormal, NULL);
	xTaskCreate(Task_cont, (const char *)"task_cont", configMINIMAL_STACK_SIZE, NULL, osPriorityNormal, NULL);

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

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(LED_BP_GPIO_Port, LED_BP_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin : LED_BP_Pin */
	GPIO_InitStruct.Pin = LED_BP_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(LED_BP_GPIO_Port, &GPIO_InitStruct);

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
