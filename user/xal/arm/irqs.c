/*
 * $FILE: irqs.c
 *
 * board definitions
 *
 * $VERSION$ - 19/09/2014
 *
 * Author: Manuel Muñoz <mmunoz@ai2.upv.es>
 * Author: Javier O. Coronel <jcoronel@fentiss.com>
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE file for the license terms.
 */

#include <string.h>
#include <stdio.h>
#include <xm.h>
#include <irqs.h>
#include <arm/gic.h>

#define PRINT(...) do { \
 printf("\t\t\t\t[P%d] ", XM_PARTITION_SELF); \
 printf(__VA_ARGS__); \
} while (0)

void ArchUnexpectedIrq(irqCtxt_t *ctxt)
{}
void ArchUnexpectedSwi(xm_s32_t swiNr, irqCtxt_t *ctxt)
{}
void ArchUnexpectedDataAbort(trapCtxt_t *ctxt)
{
    xm_u32_t DFAR, DFSR, ADFSR;
    __asm__ __volatile__ (                              \
            "mrc p15, 0, %0, c6, c0, 0\n\t" /*DFAR*/    \
            "mrc p15, 0, %1, c5, c0, 0\n\t" /*DFSR*/    \
            "mrc p15, 0, %2, c5, c1, 0\n\t" /*ADFSR*/   \
        :"=r"(DFAR),"=r"(DFSR),"=r"(ADFSR)          \
        :                                           \
        :);
    PRINT("\tDFAR:%x\n", DFAR);
    PRINT("\tDFSR:%x\n", DFSR);
    PRINT("\tADFSR:%x\n", ADFSR);

}
void ArchUnexpectedPrefetchAbort(trapCtxt_t *ctxt)
{
    xm_u32_t IFAR, IFSR, AIFSR;
    __asm__ __volatile__ (                              \
            "mrc p15, 0, %0, c6, c0, 2\n\t" /*IFAR*/    \
            "mrc p15, 0, %1, c5, c0, 1\n\t" /*IFSR*/    \
            "mrc p15, 0, %2, c5, c1, 1\n\t" /*AIFSR*/   \
        :"=r"(IFAR),"=r"(IFSR),"=r"(AIFSR)          \
        :                                           \
        :);
    PRINT("\tIFAR:%x\n", IFAR);
    PRINT("\tIFSR:%x\n", IFSR);
    PRINT("\tAIFSR:%x\n", AIFSR);
}
void ArchUnexpectedUndefInstr(trapCtxt_t *ctxt)
{}

#ifdef CONFIG_VFP_EXTENSION
#define ENCOD_FP_EXC1 0x03E00000
#define ENCOD_FP_INST 0x0c000a00
#define ENCOD_FP_EXC2 0x03000000
//#define FPU_VERBOSE
xm_u32_t checkFPU(xm_u32_t instruction)
{
    if(!(XM_params_get_PCT()->flags&PARTITION_FP_F))
    {
#ifdef FPU_VERBOSE
        PRINT("NO FP PARTITION\n");
#endif
        return 0;
    }

    if((instruction&ENCOD_FP_INST)!=ENCOD_FP_INST)
    {
#ifdef FPU_VERBOSE
        PRINT("NO FP INSTRUCTION\n");
#endif
        return 0;
    }

    if((instruction&ENCOD_FP_EXC1)==0)
    {
#ifdef FPU_VERBOSE
        PRINT("FP EXC1\n");
#endif
        return 0;
    }

    if((instruction&ENCOD_FP_EXC2)==ENCOD_FP_EXC2)
    {
#ifdef FPU_VERBOSE
        PRINT("FP EXC2\n");
#endif
        return 0;
    }

//           ldr r2, [r9]
//           movw r1, #:lower16:ENCOD_FP_INST
//           movt r1, #:upper16:ENCOD_FP_INST
//           and r3, r2, r1
//           cmp r3, r1
//           movne r0, #0
//           bne 1f
//
//           ands r3, r2, #ENCOD_FP_EXC1
//           moveq r0, #0
//           beq 1f
//
//           and r3, r2, #ENCOD_FP_EXC2
//           cmp r3, #ENCOD_FP_EXC2
//           moveq r0, #0
//           beq 1f
//
//    /*       fmrx r0, FPEXC*/
//    /*       and  r0, r0, #0x40000000*/
//    /*       cmp  r0, #0*/
//    /*       moveq r0, #0*/
//    /*       beq 1f */

    return 1;
}
#endif /*CONFIG_VFP_EXTENSION*/

void XalDoIrq(trapCtxt_t *ctxt)
{
	XalEndOfIRQ(ctxt->irqNr);

	xm_u32_t cpu;
	xm_u32_t irq;

	XalGetIrqNCpu(ctxt->irqNr, &irq, &cpu);

//	PRINT("------------XalDoIrq %d CPU %d-----------------\n", irq, cpu);

	switch(irq)
	{
    case XM_EXTIRQ_SGI_NUM:
        ctxt->irqNr = irq = XM_params_get_PCT()->irqIndex;
        if(irq >= (CONFIG_NO_HWIRQS+XM_VT_EXT_MAX))
        {
            return; // Spurious IRQ;
        }
        XM_clear_irqpend(0, 1<<(irq-CONFIG_NO_HWIRQS));
        break;
    default:
        break;
    }

    if(irqHandlersTab[irq])
    {
        (*(irqHandlersTab[irq]))(ctxt);
    }
}
