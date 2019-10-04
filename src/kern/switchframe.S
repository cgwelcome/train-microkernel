#include <arm.h>

.global switchframe
.type switchframe, %function
switchframe:
    push {r4-r12, lr}
    mov r4, #SWI_HANDLER_ADDR
    ldr r5, =swi_handler
    str r5, [r4]
    push {r0-r3}
    ldr lr, [r0]    @ Load Task PC
    ldr r4, [r2]    @ Load Task SPSR
    nop
    msr SPSR_fc, r4
    msr CPSR_fc, DEFAULT_SYSMODE
    ldr sp, [r1] @ Load Task Trapframe
    pop {r0-r12, lr}
    msr CPSR_fc, DEFAULT_SVCMODE
    movs pc, lr

swi_handler:
    msr CPSR_fc, DEFAULT_SYSMODE
    push {r0-r12, lr}
    mov r5, sp
    msr CPSR_fc, DEFAULT_SVCMODE
    pop {r0-r3}
    str lr, [r0]    @ Store PC
    str r5, [r1]    @ Store Trapframe
    mrs r6, SPSR_fc
    str r6, [r3]    @ Store SPSR
    ldr r0, [lr, #-4]   @ Load switchframe
    pop {r4-r12, lr}
    bx lr
