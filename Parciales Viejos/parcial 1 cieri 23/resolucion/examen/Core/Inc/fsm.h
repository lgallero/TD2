#ifndef FSM_H
#define FSM_H

/*==================[inclusions]=============================================*/

/*==================[cplusplus]==============================================*/

#ifdef __cplusplus
extern "C" {
#endif

/*==================[macros]=================================================*/

/*==================[typedef]================================================*/

/** @brief Enumeration of all the states */
typedef enum {
	INICIAR_CONV,
	LEER_EOC,
	LEER_SENAL_1,
	LEER_SENAL_2,
	LEER_SENAL_3,
	ESPERAR
} FSM_STATES_T;

/*==================[external data declaration]==============================*/

/*==================[external functions declaration]=========================*/

void fsm_init(void);

void fsm_runCycle(void);

void fsm_tick(void);

void fsm_ev_transicion_EOC(void);

void fsm_set_convst_1(void);
void fsm_set_convst_0(void);
void fsm_set_CS1_1(void);
void fsm_set_CS1_0(void);
void fsm_set_CS2_1(void);
void fsm_set_CS2_0(void);
void fsm_set_CS3_1(void);
void fsm_set_CS3_0(void);
void fsm_set_RD_1(void);
void fsm_set_RD_0(void);
void fsm_guardar_S1(void);
void fsm_guardar_S2(void);
void fsm_guardar_S3(void);

/*==================[cplusplus]==============================================*/

#ifdef __cplusplus
}
#endif

/*==================[end of file]============================================*/
#endif /* #ifndef FSM_H */
