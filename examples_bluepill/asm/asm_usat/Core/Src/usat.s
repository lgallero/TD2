.syntax     unified
.thumb

.section    .text
.align      2

/*
void usat10(int32_t *p, uint32_t len);
R0 <- p
R1 <- len (cantidad de words)
*/
.global usat10
.type usat10, %function
usat10:
    CBZ     R1, end_loop
loop:
    LDR     R2, [R0]
    USAT    R2, 10, R2
    STR     R2, [R0], 4
    SUBS    R1, 1
    BNE     loop
end_loop:
    BX      LR

.end
