    @ This file is jni/multiple.s
    .text
    .align  2
    .global armFunction
    .type   armFunction, %function
armFunction:
    @ Multiply by 10. Input value and return value in r0
    stmfd   sp!, {fp,ip,lr}
    mov r3, r0, asl #3  @ r3=r0<<3=r0*8
    add r0, r3, r0, asl #1  @ r0=r3+r0<<1=r0*8+r0*2=r0*10
    ldmfd   sp!, {fp,ip,lr}
    bx  lr
    .size   armFunction, .-armFunction