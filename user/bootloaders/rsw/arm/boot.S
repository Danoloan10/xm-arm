/*
 * $FILE: boot.S
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
 * - [24/08/16:XM-ARM-2:SPR-160819-02:#118] Avoid use OCM memory for SMP sev when FSBL Bootloader.
 */

//#include <linkage.h>

#define MIN_STACK_FRAME 0x60

#define ASM_ALIGN .balign 8

#define SYMBOL_NAME(X) X
#define SYMBOL_NAME_LABEL(X) X##:

#define ENTRY(name) \
    .globl SYMBOL_NAME(name); \
    ASM_ALIGN; \
    SYMBOL_NAME_LABEL(name)

#define __STR(x) #x
#define TO_STR(x) __STR(x)

.global start, _start, _halt_system

ASM_ALIGN

.section .text.traptab, "ax"
// Secure Exceptions
ENTRY(start)
ENTRY(_start)
ENTRY(exceptions)
	b	_Reset					/*0x00 Reset*/
	b	__HaltSystem			/*0x04 Undefined Instruction*/
	b	__HaltSystem			/*0x08 SVC SMC*/
	b	__HaltSystem			/*0x0c Prefetch Abort*/
	b	__HaltSystem			/*0x10 Data Abort*/
	nop	/* Placeholder for address exception vector 0x14*/
	b	__HaltSystem			/*0x18 IRQ*/
	b	__HaltSystem			/*0x1c FIQ*/

ENTRY(_Reset)

/* ONLY ALLOW CPU0 THROUGH */
/* Read MPIDR, multiprocessor Affinity Register*/
	mrc	p15,0,r1,c0,c0,5
	and	r1, r1, #0x0000000f		/*TODO ARMPorting: Not implemented yet*//*Todo Define a Mask Tag for this */
	cmp	r1, #0

/*	Only continue booting if ID==0 */
	bne		__WaitForSignal

/*	Get up CPus ID!=0 */
	movw    r1, #:lower16:_Reset
    movt    r1, #:upper16:_Reset
  	mvn     r2, #15
  	str     r1, [r2]
  	sev

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

	ldr     r0, =exceptions        		    /* Get address of Secure's vector table */
	mcr     p15, 0, r0, c12, c0, 0       	/* Write Vector Base Address Register */

    movw r4, #:lower16:__rsw_stack_top
    movt r4, #:upper16:__rsw_stack_top
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

	/*SVC*/
    cps 	#0x13 /* SUPERVISOR MODE */
#	ldr		r13,=SVC_stack			/* SVC stack pointer */
	mov 	sp, r4
    msr 	spsr_cxsf, r1

#	bl CopyDataSection
	movw r0, #:lower16:_sdata
	movt r0, #:upper16:_sdata
	movw r1, #:lower16:_sdata_lma
	movt r1, #:upper16:_sdata_lma
	movw r2, #:lower16:_edata
	movt r2, #:upper16:_edata
	sub  r2, r2, r0
	bl 	memcpy



	mov r0, #0 /* processor cpu 0 */
	movw r1, #:lower16:__rsw_stack
	movt r1, #:upper16:__rsw_stack

	bl	ZynqInit

	bl RSwMain

.text
ENTRY(_halt_system)
ENTRY(__HaltSystem)
	wfe
    b 	__HaltSystem

ENTRY(__WaitForSignal)
    wfe
    b Jump2EntryPoint
    b __WaitForSignal

.section ".bss"

ASM_ALIGN
__rsw_stack:
        .zero CONFIG_STACK_SIZE*1024
ASM_ALIGN
__rsw_stack_top:


.section .kbuild_info, "a"
.ascii "BUILD_TIME="TO_STR(BUILD_TIME)"\n"
.ascii "BUILD_IDR="TO_STR(BUILD_IDR)"\n"

.previous
