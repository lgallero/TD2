.syntax     unified
.thumb

.section    .text
.align      2

.global zero
.type zero, %function
zero:
    // R0: Puntero a memoria a borrar
    // R1: Cantidad de elementos
    PUSH    {R2, R3}
    EOR     R2, R2, R2      // R2 = R2^R2 = 0
    LSL     R1, R1, #2      // R1 = R1 * 4
    ADD     R3, R0, R1      // R3 = R0 + R1
loop:
    STR     R2, [R3, #-4]!  // *(--R3) = R2
    CMP     R3, R0          // Comparo R3 con R0
    BNE     loop            // Si son diferentes salto a loop
    POP     {R2, R3}        // Recupero los registros
    BX      LR              // Vuelvo de la función

.global zero_cbz
.type zero_cbz, %function
zero_cbz:
    // R0: Puntero a la memoria a borrar
    // R1: Cantidad de elementos a borrar
    PUSH    {R2}
    MOV     R2, #0
loop_cbz:
    CBZ     R1, fin_zero_cbz
    STR     R2, [R0], #4
    SUB     R1, #1
    B       loop_cbz
fin_zero_cbz:
    POP     {R2}
    BX      LR

.end
