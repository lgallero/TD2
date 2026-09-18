/* Copyright 2021, TD2-FRH
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef FSM_GARAJE_H
#define FSM_GARAJE_H

/** @addtogroup fsm_garaje FSM Garaje
  * @{
  */

/*==================[inclusions]=============================================*/

/*==================[cplusplus]==============================================*/

#ifdef __cplusplus
extern "C" {
#endif

/*==================[macros]=================================================*/

/*==================[typedef]================================================*/

/** @brief Enumeration of all the states */
typedef enum {
	REPOSO,
	INGRESANDO,
	ESPERANDO_EGRESO,
	ALARMA
} FSM_GARAJE_STATES_T;

/*==================[external data declaration]==============================*/

/*==================[external functions declaration]=========================*/

/** @brief State machine initialization */
void fsm_garaje_init(void);

/** @brief This function executes an RTC step */
void fsm_garaje_runCycle(void);

/** @brief Tick function that generates the time events */
void fsm_garaje_tick(void);

/** @addtogroup fsm_garaje_events FSM Garaje events
 * @{ */
void fsm_garaje_raise_evSensor1_On(void);
void fsm_garaje_raise_evSensor2_On(void);
void fsm_garaje_raise_evSensor2_Off(void);
/** @} */

/** @addtogroup fsm_garaje_actions FSM Garaje actions
 * @{ */
void fsm_garaje_abrirBarrera(void);
void fsm_garaje_cerrarBarrera(void);
void fsm_garaje_activarAlarma(void);
void fsm_garaje_apagarAlarma(void);
/** @} */

/*==================[cplusplus]==============================================*/

#ifdef __cplusplus
}
#endif

/** @} doxygen end group definition */
/*==================[end of file]============================================*/
#endif /* #ifndef FSM_GARAJE_H */
