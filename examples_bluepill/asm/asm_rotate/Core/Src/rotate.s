.syntax     unified
.thumb

.section    .text
.align      2

.global Sumar
.type Sumar, %function
Sumar:
    // Recibo por R0 y R1 dos valores
    // Los sumo y devuelvo por R0
    ADD     R0, R1, R0
    BX      LR

.global RotarDerecha
.type RotarDerecha, %function
RotarDerecha:
    // Recibe dos parámetros: en R0 el valor y en R1 cuánto rota
    // Cuando se pasan parámetros se reciben desde R0 a R3
    // donde el primer parámetro es R0, R1 es el segundo y así
    // El retorno de la función es por R0
    ROR     R0, R0, R1
    BX      LR

.global RotarIzquierda
.type RotarIzquierda, %function
RotarIzquierda:
    PUSH    {R2, LR}        // Manda R2 y LR a la pila
    RSB     R2, R1, #32     // Calculo R2 = 32 - R1
    ROR     R0, R0, R2      // Roto a la izquierda
    POP     {R2, PC}        // Restaura R2 y vuelve de la función

.end
