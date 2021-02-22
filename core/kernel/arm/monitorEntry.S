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
 * - [15/02/16:XM-ARM-2:SPR-160215-01:#109] Define global symbols.
 */

#include <comp.h>
#include <linkage.h>
#include <hypercalls.h>
        
#include <arm/asm_offsets.h>

#include <arm/gic.h>

#include <arm/asm.h>
#include <arm/irqs.h>
#include <arm/processor.h>

.section .secHandlers, "ax"
.global _NSecArea
.global _EndNSecArea
ASM_ALIGN

.macro GET_MONITOR_CALL_NUMBER
        push    {r3}
        and     r3, r0, #3840       /*0x00000f00*/
        and     r0, r0, #15         /*0x0000000f*/
        orr     r0, r0, r3, lsr #4  /*0x00000004*/
        pop     {r3}
.endm

ENTRY(_MonReset)
        push {r0-r12, lr}
        mov r4, sp
        mrs r3, cpsr
        mrs r2, spsr
        mov r1, sp
        mov r0, #0x20
        push {r0-r4, lr}
        mov r0, sp
        bl DoUnexpectedTrap
        pop {r0-r4, lr}
        msr cpsr_cxsf, r3
        msr spsr_cxsf, r2
        mov sp, r4
        ldmia sp!, {r0-r12, pc}^

ENTRY(_MonUndefInstr)
        push {r0-r12, lr}
        mov r4, sp
        mrs r3, cpsr
        mrs r2, spsr
        mov r1, sp
        mov r0, #0x24
        push {r0-r4, lr}
        mov r0, sp
        bl DoUnexpectedTrap
        pop {r0-r4, lr}
        msr cpsr_cxsf, r3
        msr spsr_cxsf, r2
        mov sp, r4
        ldmia sp!, {r0-r12, pc}^

ENTRY(_Smc)
        push {r0-r12, lr}

        movw    r5, #:lower16:_NSecArea
        movt    r5, #:upper16:_NSecArea
        movw    r6, #:lower16:_EndNSecArea
        movt    r6, #:upper16:_EndNSecArea

        cmp 	lr,r5
        blt 	5f
        cmp     lr,r6
        bls     4f

5:      mrc 	p15,0,r5,c1,c1,0	/*Could be avoid if is always in monitor=secure*/
        bic 	r5, r5, #1
        mcr 	p15,0,r5,c1,c1,0

        mov 	r8, sp
        mrs 	r6, spsr
        mrs 	r7, cpsr

        mov 	r5, sp
        mov 	sp, r8
        push 	{r0, r5-r8, lr}

		push    {r0}
		getLocalProcessorInfo
		mov     r10, r0
		pop		{r0}
        ldr 	r10, [r10, #_CKTHREAD_OFFSET]
        str 	sp, [r10, #_CTRL_OFFSET+_IRQCPUCTXT_OFFSET]

        sub 	r9, lr, #4
        ldr 	r5, [r9]
		ldr 	r6, [r9, #4]

        movw 	r10, #:lower16:0x24786879
        movt 	r10, #:upper16:0x24786879

        cmp 	r6, r10

        ldr 	r6, [sp, #0x4c]
        add 	r6, r6, #4
        str 	r6, [sp, #0x4c]

		push 	{r0}
		mov 	r0,r5
		GET_MONITOR_CALL_NUMBER
		mov 	r5, r0
		pop 	{r0}

        cmp 	r5, #NR_HYPERCALLS

        movwge 	r0, #:lower16:XM_UNKNOWN_HYPERCALL
        movtge 	r0, #:upper16:XM_UNKNOWN_HYPERCALL
        bge 	1f

        movw 	r6, #:lower16:hypercallsTab
        movt 	r6, #:upper16:hypercallsTab
        ldr 	r6, [r6, r5, lsl #2]

        cmp 	r6, #0
        movweq 	r0, #:lower16:XM_UNKNOWN_HYPERCALL
        movteq 	r0, #:upper16:XM_UNKNOWN_HYPERCALL
        blxne 	r6

1:
        str 	r0, [sp, #0x18]
2:
        mov 	r0, sp

        bl 		_CheckPendingIrq

3:
        pop 	{r0, r5-r8, lr}
        mov 	sp, r5
        msr 	cpsr_cxsf, r7
        msr 	spsr_cxsf, r6
        mov 	sp, r8

        mrc 	p15,0,r5,c1,c1,0 /*Could be avoid if is always in monitor=secure*/
        orr 	r5,r5,#1
        mcr 	p15,0,r5,c1,c1,0

/*        b _flush_and_iret*/
        ldmia 	sp!, {r0-r12, pc}^ /*iret*/
4:
		ldmia 	sp!, {r0-r12, pc}

ENTRY(_MonPrefetchAbort)
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

ENTRY(_MonDataAbort)
        sub lr, lr, #4
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

ENTRY(_MonIrq)
        push {r0-r12, lr}
        mov r4, sp
        mrs r3, cpsr
        mrs r2, spsr
        mov r1, sp
        mov r0, #0x38
        push {r0-r4, lr}
        mov r0, sp
        bl DoUnexpectedTrap
        pop {r0-r4, lr}
        msr cpsr_cxsf, r3
        msr spsr_cxsf, r2
        mov sp, r4
        ldmia sp!, {r0-r12, pc}^

ENTRY(_MonFiq)
        sub lr, lr, #4
        push {r0-r12, lr}	/*On MON stack*/

        mrc p15,0,r5,c1,c1,0
        bic r5, r5, #1
        mcr p15,0,r5,c1,c1,0

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
        bic r0, r0, r1
        cmp r0, #CONFIG_NO_HWIRQS+1
        bcs 1f

		getLocalProcessorInfo
        ldr r0, [r0, #_CKTHREAD_OFFSET]
        str sp, [r0, #_CTRL_OFFSET+_IRQCPUCTXT_OFFSET]

        mov r0, sp
        bl DoIrq

        mov r0, sp
        bl _CheckPendingIrq

1:
        pop {r0, r5-r8, lr}
        mov sp, r5
        msr cpsr_cxsf, r7
        msr spsr_cxsf, r6
        mov sp, r8

        mrc p15,0,r5,c1,c1,0
        orr r5,r5,#1
        mcr p15,0,r5,c1,c1,0

/*        b _flush_and_iret*/
        ldmia   sp!, {r0-r12, pc}^ /*iret*/

.section .boot.text, "ax"

