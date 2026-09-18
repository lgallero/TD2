.syntax     unified
.thumb

.section    .text
.align      2

.global sumar
.type sumar, %function
sumar:
    ADD     R0, R1
    BX      LR

.global sumar_varios
.type sumar_varios, %function
sumar_varios:
    PUSH    {R2, R3, R4}
    MOVS    R2, #0      // R2 como acumulador
    MOVS    R3, #0      // R3 como indice
    CBZ     R1, salir
seguir_sumando:
    LDR     R4, [R0, R3, LSL 2]     // R4 = *(R0 + R3<<2)
    ADD     R2, R4
    ADD     R3, #1
    CMP     R3, R1
    BNE     seguir_sumando
salir:
    MOVS    R0, R2
    POP     {R2, R3, R4}
    BX      LR

.end
