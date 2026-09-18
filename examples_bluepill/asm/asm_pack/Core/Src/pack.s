.syntax     unified
.thumb

.section    .text
.align      2

/*
void pack(int16_t *datos, int len, uint32_t *salida);
R0 <- datos
R1 <- len
R2 <- salida
*/
.global pack
.type pack, %function
pack:
    push    {r3-r5, lr}

    mov     r3, 0
    mov     r4, 0

emp_loop:
    ubfx    r4, r0, 1, 2
    ldrsh   r3, [r0], 2
    ssat    r3, 8, r3

    ldr     r5, [r2]
    tbb     [pc, r4]
emp_tabla:
    .byte (emp0-emp_tabla)/2
    .byte (emp1-emp_tabla)/2
    .byte (emp2-emp_tabla)/2
    .byte (emp3-emp_tabla)/2
emp0:
    bfi     r5, r3, 0, 8
    str     r5, [r2]
    b       emp_loop_cont
emp1:
    bfi     r5, r3, 8, 8
    str     r5, [r2]
    b       emp_loop_cont
emp2:
    bfi     r5, r3, 16, 8
    str     r5, [r2]
    b       emp_loop_cont
emp3:
    bfi     r5, r3, 24, 8
    str     r5, [r2]

    add     r2, 4
emp_loop_cont:
    subs    r1, 1
    bne     emp_loop

    pop     {r3-r5, pc}

.end
