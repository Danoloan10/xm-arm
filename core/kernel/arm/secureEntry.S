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
 */

#include <comp.h>
#include <linkage.h>
#include <hypercalls.h>
        
#include <arm/asm_offsets.h>

#include <arm/gic.h>

#include <arm/asm.h>
#include <arm/irqs.h>
#include <arm/processor.h>

#ifdef CONFIG_VFP_EXTENSION
#define ENCOD_FP_EXC1 0x3E00000
#define ENCOD_FP_INST 0xc000a00
#define ENCOD_FP_EXC2 0x3000000
#define KTHREAD_FP_F  (1<<1) /*Defined in kthread.h*/
#define EN_VFP_BIT (1<<30) /*Enable VFP extensions on FPEXC*/
#endif /*CONFIG_VFP_EXTENSION*/
ASM_ALIGN

ENTRY(_UndefInstr)
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

ENTRY(_Svc)
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

ENTRY(_PrefetchAbort)
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

ENTRY(_DataAbort)
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

ENTRY(_Irq)
        push {r0-r12, lr}
        mov r4, sp
        mrs r3, cpsr
        mrs r2, spsr
        mov r1, sp
        mov r0, #0x18
        push {r0-r4, lr}
        mov r0, sp
        bl DoUnexpectedTrap
        pop {r0-r4, lr}
        msr cpsr_cxsf, r3
        msr spsr_cxsf, r2
        mov sp, r4
        ldmia sp!, {r0-r12, pc}^

ENTRY(_Fiq)
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
