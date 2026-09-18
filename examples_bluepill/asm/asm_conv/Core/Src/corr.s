.syntax     unified
.thumb

.section    .text
.align      2

#define x  r0
#define nx r1
#define h  r2
#define nh r3
#define y  r4
#define i  r5
#define j  r6

.global conv_asm
.type conv_asm, %function
conv_asm:
    push {r1-r11, lr}

    ldm r0, {r0-r4}

    /*
    for (i = 0; i < (nx+nh); i++)
        y[i] = 0;
    */
    add r5, nx, nh
    eor r6, r6
    eor r7, r7
loop_zero:
    str r7, [y, r6, lsl 2]
    add r6, 1
    cmp r6, r5
    bne loop_zero

/* lazo principal
    for (i = 0; i < nx; i++) {
        for (j = 0; j < nh; j++) {
            y[i+j] += h[j] * x[i];
        }
    }
*/
    eor i, i
loop_i:
    /* r7 = x[i] */
    ldr r7, [x, i, lsl 2]

    eor j, j
loop_j:
    /* r8 = h[j] */
    ldr r8, [h, j, lsl 2]

    /* r9 = i+j */
    add r9, i, j

    /* r10 = y[i+j] */
    ldr r10, [y, r9, lsl 2]

    /* r11 = 0 */
    eor r11, r11

    /* r10:r11 += r8 * r7 */
    /* y[i+j] += h[j] * x[i]; */
    smlal r11, r10, r8, r7

    str r10, [y, r9, lsl 2]

    add j, 1
    cmp j, nh
    bne loop_j

    add i, 1
    cmp i, nx
    bne loop_i

    pop {r1-r11, pc}

.end
