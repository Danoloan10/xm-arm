/*
 * $FILE: start.S
 *
 * $VERSION$
 *
 * Author: Manuel Muñoz <mmunoz@ai2.upv.es>
 * Author: Javier O. Coronel <jcoronel@fentiss.com>
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE file for the license terms.
 */
/*
 * Changelog:
 * - [10/12/15:XM-ARM-2:SPR-150527-01:#51] _Reset must jump to the address in r0 instead _XM_Reset.
 * - [21/01/16:XM-ARM-2:SPR-150608-03:#57] Fix SystemReset on SMP.
 * - [15/02/16:XM-ARM-2:SPR-160215-01:#109] Delete deprecated function call. "CopyNSAccesibleData".
 */

#include <comp.h>
#include <linkage.h>
        
.section .irqVectors, "ax"
ASM_ALIGN
// Secure Exceptions
ENTRY(start)
ENTRY(_start)
ENTRY(exceptions)
    b    _XM_Reset                 /*0x00*/
    b    _UndefInstr               /*0x04*/
    b    _Svc                      /*0x08*/
    b    _PrefetchAbort            /*0x0c*/
    b    _DataAbort                /*0x10*/
    nop                            /* Placeholder for address exception vector 0x14*/
    b    _Irq                      /*0x18*/
    b    _Fiq                      /*0x1c*/

ENTRY(monitorExceptions)
    b    .                         /* Placeholder for address exception vector 0x20*/
    b    .                         /* Placeholder for address exception vector 0x24*/
    b    _Smc                      /*0x28*/
    b    _MonPrefetchAbort         /*0x2c*/
    b    _MonDataAbort             /*0x30*/
    nop                            /* Placeholder for address exception vector 0x34*/
    b    _MonIrq                   /*0x38*/
    b    _MonFiq                   /*0x3c*/
.section .nsecArea.ExcVector, "ax"
ENTRY(nonSecExceptions)
    b    .                         /*0x40*/
    b    _NSUndefInstr             /*0x44*/
    b    _NSSvc                    /*0x48*/
    b    _NSPrefetchAbort          /*0x4c*/
    b    _NSDataAbort              /*0x50*/
    nop                            /* Placeholder for address exception vector 0x14*/
    b    _NSIrq                    /*0x58*/
    b    _NSFiq                    /*0x5c*/
        
.section .boot.text, "ax"
ENTRY(_XM_Reset)

/* ONLY ALLOW CPU0 THROUGH */
/* Read MPIDR, multiprocessor Affinity Register*/
	mrc	p15,0,r5,c0,c0,5
	and	r5, r5, #0x0000000f		/*TODO ARMPorting: Not implemented yet*//*Todo Define a Mask Tag for this */
	cmp	r5, #0

#ifndef CONFIG_SMP_SUPPORT
/*	Only continue booting if ID==0 */
	bne		WaitForEventLoop
#else
	blne	WaitForEvent
    bne     ModesAndStack
#endif

InitBSS:
    movw r1, #:lower16:_sbss
    movt r1, #:upper16:_sbss
    movw r2, #:lower16:_ebss
    movt r2, #:upper16:_ebss

    sub r2, r2, r1
    mov r3, #0
bssloop:
        subs r2, r2, #1
        strb r3, [r1], #1
	    bne bssloop

ModesAndStack:
	/*IRQ and FIQ disable*/
	cpsid 	if
	/*Imprecise aborts enable*/
	cpsie 	a

	ldr     r0, =monitorExceptions         	/* Get address of Monitor's vector table */
	mcr     p15, 0, r0, c12, c0, 1       	/* Write Monitor Vector Base Address Register */
	ldr     r0, =exceptions        		    /* Get address of Secure's vector table */
	mcr     p15, 0, r0, c12, c0, 0       	/* Write Vector Base Address Register */

    movw r4, #:lower16:__idle_stack_top
    movt r4, #:upper16:__idle_stack_top
#ifdef CONFIG_SMP_SUPPORT
    cmp r5, #1
    subge   r4, r4, #CONFIG_KSTACK_SIZE
    cmp r5, #2
    subge   r4, r4, #CONFIG_KSTACK_SIZE
    cmp r5, #3
    subge   r4, r4, #CONFIG_KSTACK_SIZE
#endif
	mov 	sp, r4
	mrs 	r1, cpsr

	/*FIQ*/
    cps 	#0x11 /* FIQ MODE */
#	ldr		r13,=FIQ_stack			/* FIQ stack pointer */
	mov 	sp, r4
    msr 	spsr_cxsf, r1

	/*IRQ*/
    cps 	#0x12 /* IRQ MODE */
#	ldr		r13,=IRQ_stack			/* IRQ stack pointer */
	mov 	sp, r4
    msr 	spsr_cxsf, r1

	/*SVC*/
    cps 	#0x13 /* SUPERVISOR MODE */
#	ldr		r13,=SVC_stack			/* SVC stack pointer */
	mov 	sp, r4
    msr 	spsr_cxsf, r1

	/*ABORT*/
    cps 	#0x17 /* ABORT MODE */
#	ldr		r13,=ABT_stack		/* ABORT stack pointer */
	mov 	sp, r4
    msr 	spsr_cxsf, r1

	/*UNDEF*/
    cps 	#0x1B /* UNDEF MODE */
#	ldr		r13,=UNF_stack		/* UNDEF stack pointer */
	mov 	sp, r4
    msr 	spsr_cxsf, r1

	/*SYS*/
    cps 	#0x1F /* SYS MODE */
#	ldr		r13,=SYS_stack			/* SYS stack pointer */
	mov 	sp, r4
    msr 	spsr_cxsf, r1

	/*MONITOR*/
    cps 	#0x16 /* MONITOR MODE */
#	ldr		r13,=MON_stack		/* MONITOR stack pointer */
	mov 	sp, r4
    msr 	spsr_cxsf, r1

#ifdef CONFIG_ZYNQ
	bl		ZynqInit
#endif

#ifdef CONFIG_SMP_SUPPORT
    cmp r5, #0

    mov r0, r5 /* processor cpu 0 */
    movw r1, #:lower16:__idle_kthread
    movt r1, #:upper16:__idle_kthread

    cmp r5, #1
    addge   r1, r1, #CONFIG_KSTACK_SIZE
    cmp r5, #2
    addge   r1, r1, #CONFIG_KSTACK_SIZE
    cmp r5, #3
    addge   r1, r1, #CONFIG_KSTACK_SIZE

    cmp r5, #0

    blne    InitSecondaryCpu
#endif

    bl CopyDataSection

	mov r0, #0 /* processor cpu 0 */
	movw r1, #:lower16:__idle_kthread
	movt r1, #:upper16:__idle_kthread

	bl Setup

	b _XM_Reset

WaitForEventLoop:
	wfe
	b	WaitForEventLoop

WaitForEvent:
	wfe
	bx	lr

.text
_HaltSystemEndlessLoop:
ENTRY(__HaltSystem)
        b _HaltSystemEndlessLoop

ENTRY(_Reset)
#ifdef CONFIG_SMP_SUPPORT
        push {r0-r12, lr}
		bl	ClearResetMask
		pop  {r0-r12, lr}
#endif
        blx      r0
        b       .

ENTRY(__AsmStartUp)
        pop {r0, r1}
        blx r1

ENTRY(_NonSecUndefinedTrap)
        push {r0-r12, lr}
        mov r4, sp
        mrs r3, cpsr
        mrs r2, spsr
        mov r1, sp
        mov r0, #0x4f
        push {r0-r4, lr}
        mov r0, sp
        bl DoUnexpectedTrap
        pop {r0-r4, lr}
        msr cpsr_cxsf, r3
        msr spsr_cxsf, r2
        mov sp, r4
        ldmia sp!, {r0-r12, pc}^


.section ".bss"
ASM_ALIGN
__idle_stack:
        .zero CONFIG_KSTACK_SIZE*CONFIG_NO_CPUS
ASM_ALIGN
__idle_stack_top:
__idle_kthread:
/*        .fill ((CONFIG_KSTACK_SIZE*CONFIG_NO_CPUS)/4),4,0*/
        .zero CONFIG_KSTACK_SIZE*CONFIG_NO_CPUS
ASM_ALIGN
__idle_kthread_top:
        
        
.section .kbuild_info, "a"
.ascii "BUILD_TIME="TO_STR(BUILD_TIME)"\n"
.ascii "BUILD_IDR="TO_STR(BUILD_IDR)"\n"
.previous
