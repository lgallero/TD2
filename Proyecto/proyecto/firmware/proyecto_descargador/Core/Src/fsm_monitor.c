/*
 * fsm_monitor.c
 *
 * Versión: FINAL GOLD (Checksum Activo + Debug RAW + Aviso Espera)
 * - Valida Checksum en :R50 (Descarta ruido).
 * - Imprime trama cruda [RX RAW] en consola.
 * - Envia aviso JSON cuando entra a espera.
 * - Mantiene logica de Interrupcion y JSON previo.
 */

#include "fsm_monitor.h"
#include "main.h"
#include "user_gpio.h"
#include "esp8266.h"
#include "rtc_driver.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ==============================================================================
 * DEFINICIONES
 * ============================================================================== */

#define RX_RS485_BUFFER_LEN 256U
#define ESP_RX_QUEUE_LENGTH 8
#define UMBRAL_TEMP_ON      40.0f
#define UMBRAL_TEMP_OFF     35.0f
#define TIMEOUT_REASK_MS    10000
#define DEBOUNCE_TIME_MS    250

typedef enum {
    BAT_PLOMO = 0,
    BAT_LI_ION = 1,
    BAT_LIFEPO4 = 2,
    BAT_TEST = 3
} tipo_bateria_t;

/* ==============================================================================
 * VARIABLES GLOBALES
 * ============================================================================== */

FSM_MONITOR_STATES_T state;
FSM_PREGUNTAS_STATES_T pregunta;

/* Variables Configuración */
uint8_t TENSION_NOMINAL = 0;
uint8_t CAPACIDAD = 0;
uint8_t TIPO_BATERIA = 0;

/* Cálculos */
float tension_corte_calculada = 0.0f;
float tension_alta_calculada = 0.0f;
float corriente_limite_calculada = 0.0f;
float potencia_maxima_calculada = 0.0f;
uint8_t nivel_carga_calculado = 0;

/* Control */
uint32_t medicion_cnt = 0;
static uint8_t codigo_fin_descarga = 0;

/* Flags Interrupción */
volatile bool ev_esp_ON = false;
volatile bool ev_pulsador = false;

/* Hardware Handles */
extern UART_HandleTypeDef huart3;
static uint8_t rs485_rx_buffer[RX_RS485_BUFFER_LEN];
static volatile size_t rs485_rx_index = 0U;
static volatile bool rs485_line_received = false;

extern QueueHandle_t cola_rx;
static StaticQueue_t cola_rx_struct;
static uint8_t cola_rx_storage[ESP_RX_QUEUE_LENGTH * sizeof(esp_rx_msg_t)];

/* Estructuras de Datos */
typedef struct {
    float tension;
    float corriente;
    float temperatura;
    float capacidad_restante;
    uint8_t estado_salida;
    bool   salida_activa;
    char hora_str[16];
} monitor_medicion_t;

typedef struct {
    uint32_t ovp;
    uint32_t lvp;
    uint32_t ocp;
    uint32_t opp;
    uint32_t otp;
    uint32_t capacity;
} monitor_config_read_t;

static monitor_medicion_t ultima_medicion = {0};

/* ==============================================================================
 * PROTOTIPOS LOCALES
 * ============================================================================== */
void calcular_parametros_descarga(void);
void enviar_parametros_a_monitor(void);
void enviar_hora_rtc_a_monitor(void);
void RS485_Send(const char *cmd);
bool monitor_parse_r50(const char *frame, monitor_medicion_t *out);
bool monitor_parse_r51(const char *frame, monitor_config_read_t *out);
void enviar_trama_estado_esp(const monitor_medicion_t *m, bool fin, const char *motivo);
void gestion_temperatura(float temp_actual);
void chequear_cambio_tension_manual(bool forzar);
void enviar_comando_W(uint8_t funcion, uint32_t valor_final);
const char* obtener_descripcion_error_kh(uint8_t codigo);

/* ==============================================================================
 * INTERRUPCIONES
 * ============================================================================== */

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART3)
    {
        uint8_t c = rs485_rx_buffer[rs485_rx_index];
        if (c == '\n' || rs485_rx_index >= RX_RS485_BUFFER_LEN - 1)
        {
            rs485_rx_buffer[rs485_rx_index] = '\0';
            rs485_line_received = true;
            rs485_rx_index = 0;
        }
        else
        {
            rs485_rx_index++;
        }
        HAL_UART_Receive_IT(&huart3, &rs485_rx_buffer[rs485_rx_index], 1);
    }
}

void ev_esp_ON_raise(void) { ev_esp_ON = true; }

void ev_pulsador_raise(void) {
    static uint32_t last_press_tick = 0;
    uint32_t current_tick = HAL_GetTick();
    if ((current_tick - last_press_tick) > DEBOUNCE_TIME_MS) {
        ev_pulsador = true;
        last_press_tick = current_tick;
    }
}

/* ==============================================================================
 * UTILIDADES HARDWARE (RS485)
 * ============================================================================== */

void RS485_Send(const char *cmd) {
    HAL_GPIO_WritePin(RS485_DE_GPIO_Port, RS485_DE_Pin, GPIO_PIN_SET);
    vTaskDelay(pdMS_TO_TICKS(2));
    HAL_UART_Transmit(&huart3, (uint8_t*)cmd, strlen(cmd), HAL_MAX_DELAY);
    while (__HAL_UART_GET_FLAG(&huart3, UART_FLAG_TC) == RESET);
    HAL_GPIO_WritePin(RS485_DE_GPIO_Port, RS485_DE_Pin, GPIO_PIN_RESET);
}

static uint8_t calcular_checksum_tx(uint32_t valor) {
    return (uint8_t)((valor % 255) + 1);
}

void enviar_comando_W(uint8_t funcion, uint32_t valor_final) {
    char cmd[64];
    uint8_t checksum = calcular_checksum_tx(valor_final);
    snprintf(cmd, sizeof(cmd), ":W%02u=1,%u,%lu,\r\n", funcion, checksum, (unsigned long)valor_final);
    memset(rs485_rx_buffer, 0, RX_RS485_BUFFER_LEN);
    rs485_rx_index = 0;
    rs485_line_received = false;
    RS485_Send(cmd);
    vTaskDelay(pdMS_TO_TICKS(150));
}

void enviar_parametros_a_monitor(void) {
    printf("[CONF] Cargando parametros al monitor...\r\n");
    enviar_comando_W(20, (uint32_t)(tension_alta_calculada * 100.0f));
    enviar_comando_W(21, (uint32_t)(tension_corte_calculada * 100.0f));
    enviar_comando_W(22, (uint32_t)(corriente_limite_calculada * 100.0f));
    enviar_comando_W(24, (uint32_t)(potencia_maxima_calculada * 100.0f));
    enviar_comando_W(25, 200); // OTP
    uint32_t capacidad_envio = (uint32_t)CAPACIDAD * 10;
    printf("[CONF] Configurando Capacidad: %d Ah\r\n", CAPACIDAD);
    enviar_comando_W(28, capacidad_envio);
    enviar_comando_W(60, 100);
}

void enviar_hora_rtc_a_monitor(void) {
    RTC_DateTime_t t = RTC_Driver_GetDateTime();
    uint32_t fecha_fmt = (t.Year * 10000) + (t.Month * 100) + t.Day;
    enviar_comando_W(11, fecha_fmt);
    vTaskDelay(pdMS_TO_TICKS(150));
    uint32_t tiempo_fmt = (t.Hours * 10000) + (t.Minutes * 100) + t.Seconds;
    enviar_comando_W(12, tiempo_fmt);
}

/* ==============================================================================
 * PARSERS
 * ============================================================================== */

bool monitor_parse_r50(const char *frame, monitor_medicion_t *out) {
    if (!frame || !out) return false;
    if (strstr(frame, "50=") == NULL) return false;

    // Copiamos la parte de datos despues del '='
    char buf[256];
    const char *p = strchr(frame, '='); if(!p) return false; p++;
    strncpy(buf, p, sizeof(buf)-1); buf[sizeof(buf)-1]='\0';

    char *token, *saveptr;
    int idx = 0;
    long val;

    // Variables para validación Checksum
    long recv_c = 0; // Checksum recibido (Indice 1)
    long sum_c = 0;  // Suma acumulada (Indices 2+)

    token = strtok_r(buf, ",", &saveptr);
    while (token) {
        val = strtol(token, NULL, 10);

        // Logica Checksum
        if (idx == 1) recv_c = val;
        else if (idx >= 2) sum_c += val;

        switch(idx) {
            case 2: out->tension = (float)val / 100.0f; break;
            case 3: out->corriente = (float)val / 100.0f; break;
            case 4: out->capacidad_restante = (float)val / 1000.0f; break;
            case 8: out->temperatura = (float)val - 100.0f; break;
            case 10: out->estado_salida = (uint8_t)val; break;
            case 15: strncpy(out->hora_str, token, 15); break;
        }
        token = strtok_r(NULL, ",", &saveptr); idx++; if(idx>15) break;
    }

    // --- 1. VALIDACION CHECKSUM (Matemática) ---
    long calc_c = (sum_c % 255) + 1;
    if (calc_c != recv_c) {
        printf("[R50] Checksum Error: Recv=%ld Calc=%ld. Descartado.\r\n", recv_c, calc_c);
        return false;
    }

    // --- 2. VALIDACION DE ESTADO (Lógica / Sanity Check) ---
    // Valores validos segun manual: 0, 1, 2, 3, 4, 5, 6, 99.
    // Si llega cualquier otro numero, es ruido que pasó el checksum.
    bool estado_valido = (out->estado_salida <= 6) || (out->estado_salida == 99);

    if (!estado_valido) {
        printf("[R50] Estado Invalido detectado (%u). Trama corrupta/ruido. Descartado.\r\n", out->estado_salida);
        return false; // <--- AQUÍ DESCARTAMOS LA MEDICIÓN
    }

    // Si pasó ambos filtros, es una trama válida
    out->salida_activa = (out->estado_salida == 0);
    return true;
}

bool monitor_parse_r51(const char *frame, monitor_config_read_t *out) {
    if (!frame || !out) return false;
    if (strstr(frame, "51=") == NULL) return false;
    char buf[256];
    const char *p = strchr(frame, '='); if(!p) return false; p++;
    strncpy(buf, p, sizeof(buf)-1); buf[sizeof(buf)-1]='\0';
    char *token, *saveptr; int idx = 0; long val;
    token = strtok_r(buf, ",", &saveptr);
    while (token) {
        val = strtol(token, NULL, 10);
        switch(idx) {
            case 2: out->ovp = (uint32_t)val; break;
            case 3: out->lvp = (uint32_t)val; break;
            case 5: out->ocp = (uint32_t)val; break;
            case 6: out->opp = (uint32_t)val; break;
            case 7: out->otp = (uint32_t)val; break;
            case 10: out->capacity = (uint32_t)val; break;
        }
        token = strtok_r(NULL, ",", &saveptr); idx++; if(idx > 12) break;
    }
    return true;
}

const char* obtener_descripcion_error_kh(uint8_t codigo) {
    switch(codigo) {
        case 0: return "ON";
        case 1: return "OVP";
        case 2: return "OCP";
        case 3: return "LVP";
        case 4: return "NCP";
        case 5: return "OPP";
        case 6: return "OTP";
        case 99: return "OFF_Manual";
        default: return "UNK";
    }
}

/* ==============================================================================
 * REPORTE WIFI Y CONTROL (JSON)
 * ============================================================================== */

void formatear_hora_robusto(const char *raw, char *dest) {
    char temp[7] = "000000";
    int len = strlen(raw);
    if (len > 0 && len <= 6) {
        memcpy(temp + (6 - len), raw, len);
    }
    sprintf(dest, "%c%c:%c%c:%c%c", temp[0], temp[1], temp[2], temp[3], temp[4], temp[5]);
}

void enviar_trama_estado_esp(const monitor_medicion_t *m, bool fin, const char *motivo) {
    if (!m) return;
    char msg[192];
    char hora_fmt[10];

    formatear_hora_robusto(m->hora_str, hora_fmt);
    medicion_cnt++;

    if (!fin) {
        snprintf(msg, sizeof(msg),
                 "{\"t\":\"DAT\",\"n\":%lu,\"v\":%.2f,\"i\":%.2f,\"cr\":%.3f,\"h\":\"%s\"}\r\n",
                 medicion_cnt,
                 m->tension,
                 m->corriente,
                 m->capacidad_restante,
                 hora_fmt);
    } else {
        const char *txt_motivo = motivo ? motivo : "UNK";
        float cap_medida = (float)CAPACIDAD - m->capacidad_restante;
        if (cap_medida < 0) cap_medida = 0;

        printf("[FIN] Motivo:%s (Codigo:%u) | Medido:%.3f Ah\r\n",
               txt_motivo, codigo_fin_descarga, cap_medida);

        if (strcmp(txt_motivo, "UNK") == 0) {
             char motivo_debug[16];
             snprintf(motivo_debug, sizeof(motivo_debug), "UNK(%u)", codigo_fin_descarga);
             snprintf(msg, sizeof(msg),
                 "{\"t\":\"FIN\",\"motivo\":\"%s\",\"Capacidad restante\":%.3f,\"Capacidad medida\":%.3f,\"h\":\"%s\"}\r\n",
                 motivo_debug,
                 m->capacidad_restante,
                 cap_medida,
                 hora_fmt);
        } else {
             snprintf(msg, sizeof(msg),
                 "{\"t\":\"FIN\",\"motivo\":\"%s\",\"Capacidad restante\":%.3f,\"Capacidad medida\":%.3f,\"h\":\"%s\"}\r\n",
                 txt_motivo,
                 m->capacidad_restante,
                 cap_medida,
                 hora_fmt);
        }
    }
    esp8266_send_data(msg, strlen(msg));
}

void gestion_temperatura(float temp_actual) {
    static bool fan_on = false;
    if (temp_actual > UMBRAL_TEMP_ON && !fan_on) {
        printf("[FAN] ON (%.1f C)\r\n", temp_actual);
        activarVentilacion(); fan_on = true;
    }
    else if (temp_actual < UMBRAL_TEMP_OFF && fan_on) {
        printf("[FAN] OFF (%.1f C)\r\n", temp_actual);
        desactivarVentilacion(); fan_on = false;
    }
}

void chequear_cambio_tension_manual(bool forzar) {
    static GPIO_PinState ultimo_estado = GPIO_PIN_SET;
    if (forzar) ultimo_estado = !HAL_GPIO_ReadPin(TENSION_24_48_GPIO_Port, TENSION_24_48_Pin);
    GPIO_PinState estado_actual = HAL_GPIO_ReadPin(TENSION_24_48_GPIO_Port, TENSION_24_48_Pin);
    if (estado_actual != ultimo_estado) {
        uint32_t ovp_val;
        if (estado_actual == GPIO_PIN_RESET) {
            configurarTensionDescarga(0); ovp_val = 3000;
            printf("[MANUAL] Llave 24V (OVP 30V)\r\n");
        } else {
            configurarTensionDescarga(1); ovp_val = 5500;
            printf("[MANUAL] Llave 48V (OVP 55V)\r\n");
        }
        enviar_comando_W(20, ovp_val);
        ultimo_estado = estado_actual;
    }
}

void calcular_parametros_descarga(void) {
    if (TIPO_BATERIA == BAT_LI_ION || TIPO_BATERIA == BAT_LIFEPO4 || TIPO_BATERIA == BAT_TEST) {
        nivel_carga_calculado = 2; corriente_limite_calculada = 10.0f;
    } else {
        if (CAPACIDAD > 25) { nivel_carga_calculado = 2; corriente_limite_calculada = 10.0f; }
        else { nivel_carga_calculado = 1; corriente_limite_calculada = 5.0f; }
    }
    if (TENSION_NOMINAL == 0) {
        tension_alta_calculada = 30.0f; configurarTensionDescarga(0);
        potencia_maxima_calculada = (corriente_limite_calculada > 9.0f) ? 300.0f : 150.0f;
    } else {
        tension_alta_calculada = 55.0f; configurarTensionDescarga(1);
        potencia_maxima_calculada = (corriente_limite_calculada > 9.0f) ? 600.0f : 300.0f;
    }
    switch (TIPO_BATERIA) {
        case BAT_PLOMO: tension_corte_calculada = (TENSION_NOMINAL == 0) ? ((CAPACIDAD < 25) ? 21.0f : 22.2f) : ((CAPACIDAD < 25) ? 42.0f : 44.4f); break;
        case BAT_LI_ION: tension_corte_calculada = (TENSION_NOMINAL == 0) ? 24.5f : 49.0f; break;
        case BAT_LIFEPO4: tension_corte_calculada = (TENSION_NOMINAL == 0) ? 24.0f : 48.0f; break;
        case BAT_TEST: tension_corte_calculada = 10.0f; break;
    }
}

/* ==============================================================================
 * TAREA PRINCIPAL (FSM)
 * ============================================================================== */

static void tarea_control(void *a)
{
    uint8_t pulsador_state = 0;
    char cmd_wifi[128];
    TickType_t last_measure_tick = 0;
    const TickType_t MEASURE_PERIOD = pdMS_TO_TICKS(2000);
    static TickType_t rtc_request_tick = 0;
    static TickType_t last_ask_tick = 0;

    vTaskDelay(pdMS_TO_TICKS(1000));
    printf("\r\n=== SISTEMA INICIADO ===\r\n");
    desactivarResistencias();
    enviar_comando_W(10, 0);

    for (;;)
    {
        if (state == DESCARGANDO_MANUAL || state == DESCARGANDO_AUTOMATICO) {
            HAL_GPIO_WritePin(FSM_STATE_GPIO_Port, FSM_STATE_Pin, GPIO_PIN_SET);
        } else {
            HAL_GPIO_WritePin(FSM_STATE_GPIO_Port, FSM_STATE_Pin, GPIO_PIN_RESET);
        }

        uint32_t isrflags = READ_REG(huart3.Instance->SR);
        if ((isrflags & USART_SR_ORE) || (isrflags & USART_SR_NE) || (isrflags & USART_SR_FE)) {
            __HAL_UART_CLEAR_OREFLAG(&huart3); __HAL_UART_CLEAR_NEFLAG(&huart3); __HAL_UART_CLEAR_FEFLAG(&huart3);
            HAL_UART_Receive_IT(&huart3, &rs485_rx_buffer[rs485_rx_index], 1);
        }

        switch (state)
        {
        case INIT:
            if (ev_esp_ON) {
                printf("[INIT] WiFi Conectado. Verificando RTC...\r\n");
                if (RTC_Driver_CheckStatus() == 0) {
                    state = PEDIR_RTC;
                } else {
                    enviar_hora_rtc_a_monitor();
                    enviar_comando_W(25, 200);
                    chequear_cambio_tension_manual(true);
                    state = IDLE;
                }
            }
            break;

        case PEDIR_RTC:
            esp8266_flush_rx();
            snprintf(cmd_wifi, sizeof(cmd_wifi), "RTC ERROR. INGRESE: AAMMDDHHMMSS\r\n");
            printf("[RTC] Pidiendo: %s", cmd_wifi);
            esp8266_send_data(cmd_wifi, strlen(cmd_wifi));
            rtc_request_tick = xTaskGetTickCount();
            last_ask_tick = xTaskGetTickCount();
            state = WAIT_RTC;
            break;

        case WAIT_RTC:
            {
                esp_rx_msg_t rtc_msg;
                if (xQueueReceive(cola_rx, &rtc_msg, pdMS_TO_TICKS(100)) == pdPASS) {
                    char *ptr = (char*)rtc_msg.data;
                    if (strlen(ptr) >= 12) {
                        RTC_DateTime_t newDt = {0};
                        char temp[3] = {0};
                        strncpy(temp, ptr, 2);      newDt.Year = atoi(temp); ptr += 2;
                        strncpy(temp, ptr, 2);      newDt.Month = atoi(temp); ptr += 2;
                        strncpy(temp, ptr, 2);      newDt.Day = atoi(temp); ptr += 2;
                        strncpy(temp, ptr, 2);      newDt.Hours = atoi(temp); ptr += 2;
                        strncpy(temp, ptr, 2);      newDt.Minutes = atoi(temp); ptr += 2;
                        strncpy(temp, ptr, 2);      newDt.Seconds = atoi(temp);

                        if (newDt.Year >= 20 && newDt.Month > 0 && newDt.Day > 0) {
                            RTC_Driver_SetDateTime(newDt);
                            enviar_hora_rtc_a_monitor();
                            enviar_comando_W(25, 200);
                            chequear_cambio_tension_manual(true);
                            state = IDLE;
                        } else { state = PEDIR_RTC; }
                    }
                }
                if ((xTaskGetTickCount() - last_ask_tick) > pdMS_TO_TICKS(TIMEOUT_REASK_MS)) {
                    snprintf(cmd_wifi, sizeof(cmd_wifi), "RTC ERROR. INGRESE: AAMMDDHHMMSS\r\n");
                    esp8266_send_data(cmd_wifi, strlen(cmd_wifi));
                    last_ask_tick = xTaskGetTickCount();
                }
                if ((xTaskGetTickCount() - rtc_request_tick) > pdMS_TO_TICKS(60000)) {
                    RTC_DateTime_t def = {0,0,0,1,1,25};
                    RTC_Driver_SetDateTime(def);
                    enviar_hora_rtc_a_monitor();
                    enviar_comando_W(25, 200);
                    chequear_cambio_tension_manual(true);
                    state = IDLE;
                }
            }
            break;

        case IDLE:
            if (HAL_GPIO_ReadPin(AUTO_MANUAL_GPIO_Port, AUTO_MANUAL_Pin) == GPIO_PIN_SET) {
                pregunta = PREGUNTA_TENSION; state = CONFIGURANDO; medicion_cnt = 0;
            } else { state = MANUAL; }
            break;

        case CONFIGURANDO:
            if (HAL_GPIO_ReadPin(AUTO_MANUAL_GPIO_Port, AUTO_MANUAL_Pin) == GPIO_PIN_RESET) { state = IDLE; break; }
            esp_rx_msg_t rx_msg; uint8_t dato_rx;

            switch(pregunta) {
                case PREGUNTA_TENSION:
                    esp8266_flush_rx();
                    snprintf(cmd_wifi, sizeof(cmd_wifi), "INGRESE TENSION: 24V [0] o 48V [1]\r\n");
                    printf("[CONF] P: %s", cmd_wifi); esp8266_send_data(cmd_wifi, strlen(cmd_wifi));
                    last_ask_tick = xTaskGetTickCount(); pregunta = WAIT_1; break;

                case WAIT_1:
                    if (xQueueReceive(cola_rx, &rx_msg, pdMS_TO_TICKS(100)) == pdPASS) {
                        dato_rx = (uint8_t)atoi((char *)rx_msg.data);
                        if(dato_rx <= 1) { TENSION_NOMINAL = dato_rx; pregunta = PREGUNTA_CAPACIDAD; } else pregunta = PREGUNTA_TENSION;
                    } else if ((xTaskGetTickCount() - last_ask_tick) > pdMS_TO_TICKS(TIMEOUT_REASK_MS)) {
                        pregunta = PREGUNTA_TENSION;
                    }
                    break;

                case PREGUNTA_CAPACIDAD:
                    snprintf(cmd_wifi, sizeof(cmd_wifi), "INGRESE CAPACIDAD: 10 a 100\r\n");
                    printf("[CONF] P: %s", cmd_wifi); esp8266_send_data(cmd_wifi, strlen(cmd_wifi));
                    last_ask_tick = xTaskGetTickCount(); pregunta = WAIT_2; break;

                case WAIT_2:
                    if (xQueueReceive(cola_rx, &rx_msg, pdMS_TO_TICKS(100)) == pdPASS) {
                        dato_rx = (uint8_t)atoi((char *)rx_msg.data);
                        if(dato_rx >= 10 && dato_rx <= 100) { CAPACIDAD = dato_rx; pregunta = PREGUNTA_TIPO; } else pregunta = PREGUNTA_CAPACIDAD;
                    } else if ((xTaskGetTickCount() - last_ask_tick) > pdMS_TO_TICKS(TIMEOUT_REASK_MS)) {
                        pregunta = PREGUNTA_CAPACIDAD;
                    }
                    break;

                case PREGUNTA_TIPO:
                    snprintf(cmd_wifi, sizeof(cmd_wifi), "TIPO: PLOMO[0] LI-ION[1] LIFEPO4[2]\r\n");
                    printf("[CONF] P: %s", cmd_wifi); esp8266_send_data(cmd_wifi, strlen(cmd_wifi));
                    last_ask_tick = xTaskGetTickCount(); pregunta = WAIT_3; break;

                case WAIT_3:
                    if (xQueueReceive(cola_rx, &rx_msg, pdMS_TO_TICKS(100)) == pdPASS) {
                        dato_rx = (uint8_t)atoi((char *)rx_msg.data);
                        if(dato_rx <= 2) { TIPO_BATERIA = dato_rx; pregunta = ENVIAR_PARAMETROS; } else pregunta = PREGUNTA_TIPO;
                    } else if ((xTaskGetTickCount() - last_ask_tick) > pdMS_TO_TICKS(TIMEOUT_REASK_MS)) {
                        pregunta = PREGUNTA_TIPO;
                    }
                    break;

                case ENVIAR_PARAMETROS:
                    calcular_parametros_descarga(); enviar_parametros_a_monitor();
                    pregunta = CHECK_PARAMETROS; break;

                case CHECK_PARAMETROS:
                    memset(rs485_rx_buffer, 0, RX_RS485_BUFFER_LEN); rs485_rx_index=0; rs485_line_received=false;
                    RS485_Send(":R51=1,2,1\r\n");
                    TickType_t start = xTaskGetTickCount();
                    while(!rs485_line_received && (xTaskGetTickCount()-start < pdMS_TO_TICKS(1000))) vTaskDelay(1);

                    if (rs485_line_received) {
                        monitor_config_read_t cfg;
                        if (monitor_parse_r51((char*)rs485_rx_buffer, &cfg)) {
                            bool match_ok = true;
                            if (match_ok) {
                                printf("[CONF] VERIFICACION EXITOSA. Iniciando...\r\n");

                                // --- AVISO NUEVO: ESPERANDO DESCARGA ---
                                char msg_wait[] = "{\"t\":\"INFO\",\"msg\":\"Config OK. Esperando inicio...\"}\r\n";
                                esp8266_send_data(msg_wait, strlen(msg_wait));

                                state = ESPERANDO_DESCARGA;
                            } else {
                                printf("[CONF] ERROR PARAMETROS. Reintentando...\r\n");
                                pregunta = ENVIAR_PARAMETROS;
                            }
                        } else pregunta = ENVIAR_PARAMETROS;
                    } else pregunta = ENVIAR_PARAMETROS;
                    vTaskDelay(pdMS_TO_TICKS(500));
                    break;
            }
            break;

        case ESPERANDO_DESCARGA:
            enviar_comando_W(10, 0); vTaskDelay(5000);
            bool safe = false; uint8_t tries = 0;
            while (tries < 3 && !safe) {
                memset(rs485_rx_buffer, 0, RX_RS485_BUFFER_LEN); rs485_rx_index=0; rs485_line_received=false;
                RS485_Send(":R50=1,2,1\r\n");
                TickType_t s = xTaskGetTickCount();
                while(!rs485_line_received && (xTaskGetTickCount()-s < 1000)) vTaskDelay(1);

                if (rs485_line_received) {
                    // --- DEBUG RAW PRE-CHECK ---
                    printf("[RX RAW] %s\r\n", rs485_rx_buffer);

                    monitor_medicion_t m;
                    if (monitor_parse_r50((char*)rs485_rx_buffer, &m)) {
                        if (m.estado_salida != 0 && m.estado_salida != 99) {
                            codigo_fin_descarga = m.estado_salida; state = FIN_DESCARGA; tries=99; break;
                        } else safe = true;
                    }
                }
                if (!safe) { vTaskDelay(500); tries++; }
            }
            if (tries==99) break;
            if (safe) { enviar_comando_W(10, 1); state = DESCARGANDO_AUTOMATICO; }
            else { codigo_fin_descarga = 255; state = FIN_DESCARGA; }
            break;

        case FIN_DESCARGA:
            vTaskDelay(1000);
            enviar_trama_estado_esp(&ultima_medicion, true, (codigo_fin_descarga==255)?"COM_ERROR":obtener_descripcion_error_kh(codigo_fin_descarga));
            vTaskDelay(3000); enviar_comando_W(10, 0); state = IDLE;
            break;

        case DESCARGANDO_AUTOMATICO:
        case MANUAL:
        case DESCARGANDO_MANUAL: {
            bool en_manual = (HAL_GPIO_ReadPin(AUTO_MANUAL_GPIO_Port, AUTO_MANUAL_Pin) == GPIO_PIN_RESET);
            if (state == DESCARGANDO_AUTOMATICO && en_manual) {
                enviar_trama_estado_esp(&ultima_medicion, true, "CAMBIO_A_MANUAL");
                enviar_comando_W(10, 0); desactivarResistencias(); chequear_cambio_tension_manual(true); state = IDLE; break;
            }
            if ((state == MANUAL || state == DESCARGANDO_MANUAL) && !en_manual) {
                if(state==DESCARGANDO_MANUAL) enviar_comando_W(10, 0);
                desactivarResistencias();
                state=IDLE;
                break;
            }

            if (state == MANUAL || state == DESCARGANDO_MANUAL) {
                chequear_cambio_tension_manual(false);
                if (ev_pulsador) {
                    if (state == DESCARGANDO_MANUAL) {
                        pulsador_state++; if (pulsador_state > 2) pulsador_state = 0;
                        printf("[MANUAL] Cargas: Nivel %d\r\n", pulsador_state);
                        activarResistencias(pulsador_state);
                    } else {
                        printf("[MANUAL] Boton ignorado (Salida OFF - Inicie Descarga)\r\n");
                    }
                    ev_pulsador = false;
                }
            } else { activarResistencias(nivel_carga_calculado); }

            if ((xTaskGetTickCount() - last_measure_tick) >= MEASURE_PERIOD) {
                last_measure_tick = xTaskGetTickCount();
                memset(rs485_rx_buffer, 0, RX_RS485_BUFFER_LEN); rs485_rx_index=0; rs485_line_received=false;
                RS485_Send(":R50=1,2,1\r\n");
                TickType_t s = xTaskGetTickCount();
                while(!rs485_line_received && (xTaskGetTickCount()-s < 2000)) vTaskDelay(1);

                if (rs485_line_received) {
                    // --- DEBUG RAW EN DESCARGA ---
                    printf("[RX RAW] %s\r\n", rs485_rx_buffer);

                    monitor_medicion_t m;
                    if (monitor_parse_r50((char*)rs485_rx_buffer, &m)) {
                        gestion_temperatura(m.temperatura); ultima_medicion = m;
                        printf("[DATA] V:%.2f I:%.2f T:%.1f OUT:%s\r\n",
                               m.tension, m.corriente, m.temperatura, m.salida_activa?"ON":"OFF");

                        if (state == MANUAL && m.salida_activa) { desactivarResistencias(); pulsador_state=0; state=DESCARGANDO_MANUAL; }
                        else if (state == DESCARGANDO_MANUAL && !m.salida_activa) { desactivarResistencias(); state=MANUAL; }
                        else if (state == DESCARGANDO_AUTOMATICO) {
                            enviar_trama_estado_esp(&m, false, NULL);
                            if (!m.salida_activa) { codigo_fin_descarga=m.estado_salida; desactivarResistencias(); state=FIN_DESCARGA; }
                        }
                    }
                } else { printf("[TIMEOUT] Sin respuesta monitor\r\n"); }
            }
            break;
        }
        default: state = INIT; break;
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void FSM_MONITOR_INIT(void) {
    state = INIT; pregunta = PREGUNTA_TENSION;
    if (!cola_rx) cola_rx = xQueueCreateStatic(ESP_RX_QUEUE_LENGTH, sizeof(esp_rx_msg_t), cola_rx_storage, &cola_rx_struct);
    HAL_UART_Receive_IT(&huart3, &rs485_rx_buffer[0], 1);
    xTaskCreate(tarea_control, "control", 512, NULL, tskIDLE_PRIORITY + 2, NULL);
}
