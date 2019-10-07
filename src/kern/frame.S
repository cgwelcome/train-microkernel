#include <arm.h>
#include <kernel.h>

.global swi_handler_init
.type swi_handler_init, %function
swi_handler_init:
    mov r0, #SWI_HANDLER_ADDR
    ldr r1, =swi_handler
    str r1, [r0]
    bx lr

.global hwi_handler_init
.type hwi_handler_init, %function
hwi_handler_init:
    mov r0, #HWI_HANDLER_ADDR
    ldr r1, =hwi_handler
    str r1, [r0]
    bx lr

.global switch_frame
.type switch_frame, %function
switch_frame:
    push {r4-r12, lr}
    push {r0-r3}
    ldr lr, [r0]    @ Load Task PC
    ldr r4, [r2]    @ Load Task SPSR
    msr SPSR_fc, r4
    msr CPSR_fc, DEFAULT_SYSMODE
    ldr sp, [r1] @ Load Task Trapframe
    pop {r0-r12, lr}
    msr CPSR_fc, DEFAULT_SVCMODE
    movs pc, lr

swi_handler:
    msr CPSR, DEFAULT_SYSMODE
    push {r0-r12, lr}
    mov r5, sp
    msr CPSR, DEFAULT_SVCMODE
    pop {r0-r3}
    str lr, [r0]        @ Store PC
    str r5, [r1]        @ Store Trapframe
    mrs r6, SPSR
    str r6, [r2]        @ Store SPSR
    ldr r0, [lr, #-4]   @ Load return value
    pop {r4-r12, pc}

hwi_handler:
    msr CPSR, DEFAULT_SYSMODE
    push {r0-r12, lr}
    mov r5, sp
    msr CPSR, DEFAULT_IRQMODE
    sub r6, lr, #4
    mrs r7, SPSR
    msr CPSR, DEFAULT_SVCMODE
    pop {r0-r3}
    str r6, [r0]                @ Store PC
    str r5, [r1]                @ Store Trapframe
    str r7, [r2]                @ Store SPSR
    mov r0, #HW_INTERRUPT       @ Load return value
    pop {r4-r12, pc}
