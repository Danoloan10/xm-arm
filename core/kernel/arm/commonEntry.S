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

ASM_ALIGN

ENTRY(_CheckPendingIrq)
        push {lr}
        bl RaisePendIrqs
        cmp r0, #-1

        movne r1,r0
        addne r0, sp, #4 
        blne ArchEmulPendIrqs

        pop {pc}

ENTRY(_RaiseSwException)
        push {lr}
        mov r1,r0
        ldr r0, [r10, #_CTRL_OFFSET+_IRQCPUCTXT_OFFSET] 
        /*TODO ARMPorting: Not implemented yet*/
        /*todo        bl ArchEmulPendSwException*/
        pop {pc}

ENTRY(_CheckPendingException)
        push {lr}
        /*TODO ARMPorting: Not implemented yet*/
        /*todo        bl RaisePendExceptions*/
        cmp r0, #-1

        beq 1f

        cmp r0, #NO_TRAPS

        blcs 2f
        
        movcc r1,r0
        addcc r0, sp, #4 
        blcc ArchEmulPendException
        b 1f

2:
        movcs r1,r0
        addcs r0, sp, #4 
        blcs ArchEmulPendIrqs

1:
        pop {pc}

ENTRY(_nonSecIrq)
		sub		sp, sp, #0x8
		str		r0, [sp]
        getLocalProcessorInfo
        ldr r0, [r0, #24]
        ldr r0, [r0, #24]
        ldr r0, [r0, #280]
        add	r0, r0, #0x18
		str		r0, [sp,#0x4]
		pop 	{r0,pc}

ENTRY(_nonSecFiq)
		sub		sp, sp, #0x8
		str		r0, [sp]
        getLocalProcessorInfo
        ldr r0, [r0, #24]
        ldr r0, [r0, #24]
        ldr r0, [r0, #280]
        add	r0, r0, #0x1c
		str		r0, [sp,#0x4]
		pop 	{r0,pc}
