/*
 * $FILE: boot.S
 *
 * Generic boot code, sets up a valid stack and jumps to the C code
 *
 * $VERSION$
 *
 * Author: Javier O. Coronel <jcoronel@fentiss.com>
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license terms.
 */
/*
 * Changelog:
 * - [21/10/16:XM-ARM-2:SPR-161013-02:#128] Use function startVCPUs for vcpus different than 0.
 * - [21/03/17:XM-ARM-2:CP-161209-04:#137] Automatically enable FPU in XAL if fp flag in funtion ZynqInit.
 */
/* <track id="PARTITION_BOOT_CODE"> */
/*#define STACK_SIZE 8192*/
#include <xm.h>
#include <xm_inc/arch/asm_offsets.h>
#include <arch/irqs.h>
#include <arm/gic.h>
#include <xm_inc/xmconf.h>

#define ENCOD_FP_EXC1 0x3E00000
#define ENCOD_FP_INST 0xc000a00
#define ENCOD_FP_EXC2 0x3000000
#define KTHREAD_FP_F  (1<<1) /*Defined in kthread.h*/
#define PSR_I (1<<7)

.section .text.init, "ax"
.balign 8

.global start, _start

_start: 
start:
		b	_Reset					/*0x00*/
		b	_UndefInstr				/*0x04*/
		b	_Svc					/*0x08*/
		b	_PrefetchAbort			/*0x0c*/
		b	_DataAbort				/*0x10*/
		nop	/* Placeholder for address exception vector 0x14*/
		b	_Irq					/*0x18*/
		b	_Fiq					/*0x1c*/

_UndefInstr:
        sub lr, lr, #4
        push {r0-r12, lr}
        mrs  r6, spsr
        mrs  r7, cpsr

#ifdef CONFIG_VFP_EXTENSION
        ldr r0, [lr]
        push {lr}
        bl  checkFPU
        pop {lr}
        cmp r0, #0
        beq 10f

        movwne r9, #:lower16:fpFaultHandler
        movtne r9, #:upper16:fpFaultHandler
        ldr r9, [r9]
        ldr r9, [r9]
        cmp r9, #0
        bne 11f
#endif
10:
#        mrc p15, 0, r0, c6, c0, 2   /*IFAR*/
#        mrc p15, 0, r1, c5, c0, 1   /*IFSR*/
#        mrc p15, 0, r2, c5, c1, 1   /*AIFSR*/

        mov r5, lr //pc
//        mrs  r6, spsr /*Is done before the comparations*/
//        mrs  r7, cpsr /*Is done before the comparations*/
        mov r8, #0 //far
        push {r5-r8}

        movw r0, #:lower16:XM_HM_EV_ARM_UNDEF_INSTR
        movt r0, #:upper16:XM_HM_EV_ARM_UNDEF_INSTR
        mov  r1, sp
        smc  #arm_hm_raise_event_nr
        .long 0x24786879
        nop
        add sp, sp, #16 /*pop {r5-r8}*//*recover the registers value is not needed*/

        movw r9, #:lower16:undefInstrHandler
        movt r9, #:upper16:undefInstrHandler

        ldr r9, [r9]
        ldr r9, [r9]

        cmp r9, #0
        beq 12f

11:
//        mrs  r6, spsr /*Is done before the comparations*/
//        mrs  r7, cpsr /*Is done before the comparations*/
        mov r5, sp
        mov r8, sp
        mvn r0,#0
        push {r0, r5-r8, lr}
        mov r0,sp
        blx r9
        add sp, sp, #24 /*pop {r0, r5-r8, lr}*//*recover the registers value is not needed*/

12:
        ldmia sp!, {r0-r12, pc}^ // iret

_Svc:
        push {r0-r12, lr}
        ldr r10,[lr,#-4]
        bic r10,r10,#0xff000000
        movw r1, #:lower16:swiHandlersTab
        movt r1, #:upper16:swiHandlersTab

        lsl r10, r10, #2
        ldr r1, [r1, r10]

        cmp r1, #0
        beq 11f

        blx r1
11:
        ldmia sp!, {r0-r12, pc}^ // iret

_PrefetchAbort:
        sub lr, lr, #4
        push {r0-r12, lr}

        mov  r5, lr
        mrs  r6, spsr
        mrs  r7, cpsr
        mrc p15, 0, r8, c6, c0, 2   /*IFAR*/
#        mrc p15, 0, r1, c5, c0, 1   /*IFSR*/
#        mrc p15, 0, r2, c5, c1, 1   /*AIFSR*/
        push {r5-r8}

        movw r0, #:lower16:XM_HM_EV_ARM_PREFETCH_ABORT
        movt r0, #:upper16:XM_HM_EV_ARM_PREFETCH_ABORT
        mov  r1, sp
        smc  #arm_hm_raise_event_nr
        .long 0x24786879
        add sp, sp, #16 /*pop {r5-r8}*//*recover the registers value is not needed*/

        movw r9, #:lower16:prefetchAbortHandler
        movt r9, #:upper16:prefetchAbortHandler

        ldr r9, [r9]
        ldr r9, [r9]

        cmp r9, #0
        beq 11f

        mov r5, sp
        mov r8, sp
        mvn r0,#0
        push {r0, r5-r8, lr}
        mov r0,sp
        blx r9
        add sp, sp, #24 /*pop {r0, r5-r8, lr}*//*recover the registers value is not needed*/

11:
        ldmia sp!, {r0-r12, pc}^ // iret
_DataAbort:
        sub lr, lr, #8
        push {r0-r12, lr}

        mov  r5, lr
        mrs  r6, spsr
        mrs  r7, cpsr
        mrc p15, 0, r8, c6, c0, 0   /*DFAR*/
#        mrc p15, 0, r1, c5, c0, 0   /*DFSR*/
#        mrc p15, 0, r2, c5, c1, 0   /*ADFSR*/
        push {r5-r8}

        movw r0, #:lower16:XM_HM_EV_ARM_DATA_ABORT
        movt r0, #:upper16:XM_HM_EV_ARM_DATA_ABORT
        mov  r1, sp
        smc  #arm_hm_raise_event_nr
        .long 0x24786879
        add sp, sp, #16 /*pop {r5-r8}*//*recover the registers value is not needed*/

        movw r9, #:lower16:dataAbortHandler
        movt r9, #:upper16:dataAbortHandler

        ldr r9, [r9]
        ldr r9, [r9]

        cmp r9, #0
        beq 11f

        mrs  r6, spsr
        mrs  r7, cpsr
        mov r5, sp
        mov r8, sp
        movw r0, #:lower16:XM_HM_EV_ARM_DATA_ABORT
        movt r0, #:upper16:XM_HM_EV_ARM_DATA_ABORT
        push {r0, r5-r8, lr}
        mov r0,sp
        blx r9
        add sp, sp, #24 /*pop {r0, r5-r8, lr}*//*recover the registers value is not needed*/

11:
        ldmia sp!, {r0-r12, pc}^ // iret

_Irq:
		sub lr, lr, #4
        push {r0-r12, lr}

        sub r9, lr, #4
        mov r8, sp
        mrs r6, spsr
        mrs r7, cpsr

        mov r5, sp
        mov sp, r8
        push {r0, r5-r8, lr}

        movw r0, #:lower16:ICCIAR_ADDRESS/*IRQINDEX*/
        movt r0, #:upper16:ICCIAR_ADDRESS/*IRQINDEX*/
        ldr r0, [r0]                /*Get here the index to the highest priority IRQ interrupt*/
        movw r1, #:lower16:ICCIAR_IRQMASK
        movt r1, #:upper16:ICCIAR_IRQMASK
        mvn  r1, r1
        bic r1, r0, r1
        cmp r1, #CONFIG_NO_HWIRQS+1
        bcs 11f

        str r0, [sp, #_CTXTIRQNR_OFFSET]
        push 	{r0}
        mov r0, sp
        bl XalDoIrq
		pop 	{r0}
11:
		pop 	{r0, r5-r8, lr}
        mov 	sp, r5
        msr 	cpsr_cxsf, r7
        msr 	spsr_cxsf, r6
        mov 	sp, r8

        ldmia sp!, {r0-r12, pc}^ // iret

_Fiq:        
1:      b 1b
        
_Reset:
		/*InitBSS*/
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
        /*SVC*/
    	cpsid 	if

        movw    r4, #:lower16:__stack_top_exceptions
        movt    r4, #:upper16:__stack_top_exceptions
        mov     sp, r4
        mrs 	r1, cpsr

    	/*FIQ*/
        cps 	#0x11 /* FIQ MODE */
    	mov 	sp, r4
        msr 	spsr_cxsf, r1
        
    	/*IRQ*/
        cps 	#0x12 /* IRQ MODE */
    	mov 	sp, r4
        msr 	spsr_cxsf, r1

    	/*SVC*/
        cps 	#0x13 /* SUPERVISOR MODE */
    	mov 	sp, r4
        msr 	spsr_cxsf, r1

    	/*ABORT*/
        cps 	#0x17 /* ABORT MODE */
    	mov 	sp, r4
        msr 	spsr_cxsf, r1

    	/*UNDEF*/
        cps 	#0x1B /* UNDEF MODE */
    	mov 	sp, r4
        msr 	spsr_cxsf, r1

    	/*USR*/ /*SYS*/
        movw    r4, #:lower16:__stack_top
        movt    r4, #:upper16:__stack_top

        cps 	#0x1F /* SYS MODE */
    	mov 	sp, r4
        msr 	spsr_cxsf, r1
    	cpsid 	if

		push    {r0}
        bl CopyDataSection
        pop		{r0}

        bl init_libxm

        bl SetupIrqs       //Install irq handlers by default

        bl SetupSwi        //Install swi handlers by default

        bl SetupExceptions //Install execptions handlers by default

        bl SetupVCpus

        bl ZynqInit

        bl __cxx_ctor

        bl PartitionMain
        
        bl __cxx_dtor

        b EndlessLoop

        movw r0, #:lower16:libXmParams
        movt r0, #:upper16:libXmParams
        ldr r0, [r0]
        ldr r0, [r0, #_IDPART_OFFSET]
        __XM_HC(halt_partition_nr)


.balign 8
.global startVCPUs
startVCPUs:
#ifndef CONFIG_SMP_SUPPORT
		b	.
#else
		nop
        __XM_HC(get_vcpuid_nr)
        mov r5, r0

        movw r4, #:lower16:__stack_top_exceptions
        movt r4, #:upper16:__stack_top_exceptions
        cmp r5, #1
        subge   r4, r4, #STACK_SIZE_EXCPT
        cmp r5, #2
        subge   r4, r4, #STACK_SIZE_EXCPT
        cmp r5, #3
        subge   r4, r4, #STACK_SIZE_EXCPT
        mov sp, r4
        mrs 	r1, cpsr

    	/*FIQ*/
        cps 	#0x11 /* FIQ MODE */
    	mov 	sp, r4
        msr 	spsr_cxsf, r1

    	/*IRQ*/
        cps 	#0x12 /* IRQ MODE */
    	mov 	sp, r4
        msr 	spsr_cxsf, r1

    	/*SVC*/
        cps 	#0x13 /* SUPERVISOR MODE */
    	mov 	sp, r4
        msr 	spsr_cxsf, r1

    	/*ABORT*/
        cps 	#0x17 /* ABORT MODE */
    	mov 	sp, r4
        msr 	spsr_cxsf, r1

    	/*UNDEF*/
        cps 	#0x1B /* UNDEF MODE */
    	mov 	sp, r4
        msr 	spsr_cxsf, r1

    	/*USR*/ /*SYS*/
        movw r4, #:lower16:__stack_top
        movt r4, #:upper16:__stack_top
        cmp r5, #1
        subge   r4, r4, #STACK_SIZE
        cmp r5, #2
        subge   r4, r4, #STACK_SIZE
        cmp r5, #3
        subge   r4, r4, #STACK_SIZE

        cps 	#0x1F /* SYS MODE */
    	mov 	sp, r4
        msr 	spsr_cxsf, r1
    	cpsid 	if

        bl PartitionMain

        b EndlessLoop

        movw r0, #:lower16:libXmParams
        movt r0, #:upper16:libXmParams
        ldr r0, [r0]
        ldr r0, [r0, #_IDPART_OFFSET]
        __XM_HC(halt_partition_nr)
#endif

EndlessLoop:
        	wfe
        	b	EndlessLoop


.section ".bss"
.align 8
__stack_exceptions:
        .fill ((STACK_SIZE_EXCPT*CONFIG_NO_CPUS)/4),4,0
/*        .zero STACK_SIZE_EXCPT*/
__stack_top_exceptions:

.align 8
__stack:
        .fill ((STACK_SIZE*CONFIG_NO_CPUS)/4),4,0
/*        .zero STACK_SIZE*CONFIG_NO_CPUS*/
__stack_top:
