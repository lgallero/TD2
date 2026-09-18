#include "main.h"
#include "fsm.h"
#include "user_gpio.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

/*==================[macros and definitions]=================================*/

#define TAM 128

/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/
uint16_t read_d0_d11(void){
	return 1;
}
/*==================[internal data definition]===============================*/

static FSM_STATES_T state;

static bool ev_transicion_EOC;
static bool ev_Tick1ms;

static uint32_t count_ms_1;
static uint32_t count_ms_2;
static uint32_t count;

static char valor1[TAM];
static char valor2[TAM];
static char valor3[TAM];

static uint16_t valor_s1;
static uint16_t valor_s2;
static uint16_t valor_s3;



/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

static void clearEvents(void) {
	ev_transicion_EOC=0;
	ev_Tick1ms = 0;
}

/*==================[external functions definition]==========================*/

void fsm_init(void) {
	state = INICIAR_CONV;
	clearEvents();
	count_ms_1 = 0;
	count_ms_2 = 0;
	count=0;
	fsm_set_convst_1();
}

void fsm_runCycle(void) {
	switch (state) {
	case(INICIAR_CONV):
						if(ev_Tick1ms  && count>=1){
							count=0;
							fsm_set_convst_0();
							state=LEER_EOC;
						}
						else if(ev_Tick1ms && count <1){
							count++;
						}
	break;
	case(LEER_EOC):
						if(ev_transicion_EOC){
							fsm_set_CS1_0();
							fsm_set_RD_0();
							state=LEER_SENAL_1;
						}
	break;
	case(LEER_SENAL_1):
						if(ev_Tick1ms  && count>=1){
							count=0;
							fsm_set_CS1_1();
							fsm_set_CS2_0();
							fsm_set_RD_1();
							fsm_guardar_S1();
							HAL_UART_Transmit_IT(&huart1, (uint8_t*)&valor_s1,TAM);
							state=LEER_SENAL_2;
						}
						else if(ev_Tick1ms && count <1){
							count++;
						}
	break;
	case(LEER_SENAL_2):
						if(ev_Tick1ms  && count>=1){
							count=0;
							fsm_set_CS2_1();
							fsm_set_CS3_0();
							fsm_guardar_S2();
							HAL_UART_Transmit_IT(&huart1, (uint8_t*)&valor_s2, TAM);
							state=LEER_SENAL_3;
						}
						else if(ev_Tick1ms && count <1){
							count++;
						}
	break;
	case(LEER_SENAL_3):
						if(ev_Tick1ms  && count>=1){
							count=0;
							fsm_set_CS3_1();
							fsm_guardar_S3();
							HAL_UART_Transmit_IT(&huart1, (uint8_t*)&valor_s3, TAM);
							state=ESPERAR;
						}
						else if(ev_Tick1ms && count <1){
							count++;
						}
	break;
	case(ESPERAR):
						if(ev_Tick1ms  && count>=100){
							count=0;
							fsm_set_convst_1();
							state=INICIAR_CONV;
						}
						else if(ev_Tick1ms && count <100){
							count++;

						}

	break;


	}

	clearEvents();
}

void fsm_tick(void) {
	ev_Tick1ms = 1;
}

void fsm_ev_1(void) {
	ev_transicion_EOC = 1;
}

void fsm_guardar_S1(void){
	valor_s1 = (uint16_t) read_d0_d11();
	snprintf((char*)valor1, TAM,"D%d: %d", 1,valor_s1);
}
void fsm_guardar_S2(void){
	valor_s2 = (uint16_t) read_d0_d11();
	snprintf((char*)valor2, TAM,"D%d: %d", 2,valor_s1);
}
void fsm_guardar_S3(void){
	valor_s3 = (uint16_t) read_d0_d11();
	snprintf((char*)valor3, TAM,"D%d: %d", 3,valor_s1);
}



/*==================[end of file]============================================*/
