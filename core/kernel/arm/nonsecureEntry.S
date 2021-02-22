/*
 * $FILE: entry.S
 *
 * $VERSION$
 *
 * Author: Javier Coronel <jcoronel@fentiss.com>
 *         Miguel Masmano <mmasmano@fentiss.com>
 *
 * $LICENSE:
 * (c) Fent Innovative Software Solutions. All rights reserved.
 *     Read LICENSE file for the license terms.
 */
/*
 * Changelog:
 * - [22/08/16:XM-ARM-2:SPR-160819-01:#116] Use getLocalProcessorInfo to access struct localProcessorInfo.
 * - [15/02/16:XM-ARM-2:SPR-160215-01:#109] _flush_and_iret could be call directly.
 * - [15/02/16:XM-ARM-2:SPR-160215-01:#109] _clean_gic_pending could be call directly.
 */

#include <comp.h>
#include <linkage.h>
#include <hypercalls.h>
        
#include <arm/asm_offsets.h>

#include <arm/gic.h>

#include <arm/asm.h>
#include <arm/irqs.h>
#include <arm/processor.h>

.section .nsecArea.Handlers, "ax"
ASM_ALIGN

ENTRY(_NSUndefInstr)
        sub lr, lr, #4
        push {r0-r12, lr}
        mov r4, sp
        mrs r3, cpsr
        mrs r2, spsr
        mov r1, sp
        mov r0, #0
        push {r0-r4, lr}
		getLocalProcessorInfo
        ldr r0, [r0, #_CKTHREAD_OFFSET]
        str sp, [r0, #_CTRL_OFFSET+_IRQCPUCTXT_OFFSET]
        mov r0, sp
        bl DoTrap
        mov r0, sp
        bl _CheckPendingException
        pop {r0-r4, lr}
        msr cpsr_cxsf, r3
        msr spsr_cxsf, r2
        mov sp, r4
        ldmia sp!, {r0-r12, pc}^

ENTRY(_NSSvc)
        push {r0-r12, lr}
        mov r4, sp
        mrs r3, cpsr
        mrs r2, spsr
        mov r1, sp
        mov r0, #0x08
        push {r0-r4, lr}
        mov r0, sp
        bl DoUnexpectedTrap
        pop {r0-r4, lr}
        msr cpsr_cxsf, r3
        msr spsr_cxsf, r2
        mov sp, r4
        ldmia sp!, {r0-r12, pc}^

ENTRY(_NSPrefetchAbort)
        sub lr, lr, #4
        push {r0-r12, lr}
        mov r4, sp
        mrs r3, cpsr
        mrs r2, spsr
        mov r1, sp
        mov r0, #1
        push {r0-r4, lr}
		getLocalProcessorInfo
        ldr r0, [r0, #_CKTHREAD_OFFSET]
        str sp, [r0, #_CTRL_OFFSET+_IRQCPUCTXT_OFFSET]
        mov r0, sp
        bl DoTrap
        mov r0, sp
        bl _CheckPendingException
        pop {r0-r4, lr}
        msr cpsr_cxsf, r3
        msr spsr_cxsf, r2
        mov sp, r4
        ldmia sp!, {r0-r12, pc}^

ENTRY(_NSDataAbort)
        sub lr, lr, #8
        push {r0-r12, lr}
        mov r4, sp
        mrs r3, cpsr
        mrs r2, spsr
        mov r1, sp
        mov r0, #2
        push {r0-r4, lr}
		getLocalProcessorInfo
        ldr r0, [r0, #_CKTHREAD_OFFSET]
        str sp, [r0, #_CTRL_OFFSET+_IRQCPUCTXT_OFFSET]
        mov r0, sp
        bl DoTrap
        mov r0, sp
        bl _CheckPendingException
        pop {r0-r4, lr}
        msr cpsr_cxsf, r3
        msr spsr_cxsf, r2
        mov sp, r4
        ldmia sp!, {r0-r12, pc}^

ENTRY(_NSIrq)
        sub lr, lr, #4
        push {r0-r12, lr}

        mov r8, sp
        mrs r6, spsr
        mrs r7, cpsr

        mov r5, sp
        mov sp, r8
        push {r0, r5-r8, lr}

        movw r1, #:lower16:ICCIAR_ADDRESS /*IRQINDEX*/
        movt r1, #:upper16:ICCIAR_ADDRESS /*IRQINDEX*/
        ldr r0, [r1]

        str r0, [sp, #_CTXTIRQNR_OFFSET]

        movw r1, #:lower16:ICCIAR_IRQMASK
        movt r1, #:upper16:ICCIAR_IRQMASK
        mvn  r1, r1
        bic r1, r0, r1
        cmp r1, #0
        bne 1f

        movw    r1, #:lower16:ICCEOIR_ADDRESS /*IRQINDEX*/
        movt    r1, #:upper16:ICCEOIR_ADDRESS /*IRQINDEX*/
        streq   r0, [r1]

1:
        pop {r0, r5-r8, lr}
        mov sp, r5
        msr cpsr_cxsf, r7
        msr spsr_cxsf, r6
        mov sp, r8

        ldmia   sp!, {r0-r12, pc}^ /*iret*/

ENTRY(_NSFiq)
        push {r0-r12, lr}
        mov r4, sp
        mrs r3, cpsr
        mrs r2, spsr
        mov r1, sp
        mov r0, #0x1c
        push {r0-r4, lr}
        mov r0, sp
        bl DoUnexpectedTrap
        pop {r0-r4, lr}
        msr cpsr_cxsf, r3
        msr spsr_cxsf, r2
        mov sp, r4
        ldmia sp!, {r0-r12, pc}^

.section .nsecArea.text, "ax"

ENTRY(_clean_gic_pending)
        push    {r0-r12, lr}    /*to MON STACK*/
        mrs     r1, spsr
        mrs     r0, cpsr
        cps     #18             /*Set IRQ Mode*/
        push    {r0-r1}
        movw r3, #:lower16:ICDISER_ABS_ADDRESS /*ICDISER*/
        movt r3, #:upper16:ICDISER_ABS_ADDRESS /*ICDISER*/
        mov r4, r3
        ldr r0, [r4,#4]!
        ldr r1, [r4,#4]!
        ldr r2, [r4]
        push    {r0-r3}
        movw r4, #:lower16:ICDICER_ABS_ADDRESS /*ICDISER*/
        movt r4, #:upper16:ICDICER_ABS_ADDRESS /*ICDISER*/
        mvn  r0, #0
        str r0, [r3,#4]!
        str r0, [r3,#4]!
        str r0, [r3]

        cpsie   i               /*Enable interrupts*/
        nop
        cpsid   i           /*Disable interrupts*/

        pop     {r0-r3}
        str r0, [r3,#4]!
        str r1, [r3,#4]!
        str r2, [r3]

        pop     {r0-r1}

        smc     0
        nop
        msr     cpsr_cxsf, r0
        msr     spsr_cxsf, r1

        mrc p15,0,r0,c1,c1,0
        orr r0, r0, #1
        mcr p15,0,r0,c1,c1,0

        pop {r0-r12, pc}    /*from MON stak*/

		nop
.section .boot.text, "ax"

