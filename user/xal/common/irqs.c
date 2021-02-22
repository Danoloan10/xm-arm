/*
 * $FILE: irqs.c
 *
 * Generic traps' handler
 *
 * $VERSION$
 *
 * Author: Miguel Masmano <mmasmano@fentiss.com>
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license terms.
 */

#include <xm.h>
#include <stdio.h>
#include <irqs.h>

#ifdef CONFIG_ARM

irqHandler_t irqHandlersTab[IRQTAB_LENGTH];
swiHandler_t swiHandlersTab[SWITAB_LENGTH];
typedef enum {  XAL_EXCEPTION_UNDEF_INSTRUCTION=0,
                XAL_EXCEPTION_PREFETCH_ABORT=1,
                XAL_EXCEPTION_DATA_ABORT=2,
#ifdef CONFIG_VFP_EXTENSION
                XAL_EXCEPTION_FP_FAULT=3,
#endif /*CONFIG_VFP_EXTENSION*/
                XAL_EXCEPTION_NUMBER}exceptionHandlerType_t;
exceptionHandler_t exceptionHandlersTab[XAL_EXCEPTION_NUMBER];
const exceptionHandler_t ** dataAbortHandler     =
        ((const exceptionHandler_t **)(&(exceptionHandlersTab[XAL_EXCEPTION_DATA_ABORT]) ));
const exceptionHandler_t ** prefetchAbortHandler =
        ((const exceptionHandler_t **)(&(exceptionHandlersTab[XAL_EXCEPTION_PREFETCH_ABORT]) ));
const exceptionHandler_t ** undefInstrHandler    =
        ((const exceptionHandler_t **)(&(exceptionHandlersTab[XAL_EXCEPTION_UNDEF_INSTRUCTION]) ));
#ifdef CONFIG_VFP_EXTENSION
const exceptionHandler_t ** fpFaultHandler       =
        ((const exceptionHandler_t **)(&(exceptionHandlersTab[XAL_EXCEPTION_FP_FAULT]) ));
#endif /*CONFIG_VFP_EXTENSION*/


static void UnexpectedIrq(irqCtxt_t *ctxt)
{
    printf("[P%d] Unexpected irqNr %d (pc: 0x%x)\n", XM_PARTITION_SELF, ctxt->irqNr, ctxt->lr);
    ArchUnexpectedIrq(ctxt);

    printf("[P%d] Going HALT\n", XM_PARTITION_SELF);
    XM_halt_partition(XM_PARTITION_SELF);
}

static void UnexpectedSwi(xm_s32_t swiNr, irqCtxt_t *ctxt)
{
    printf("[P%d] Unexpected swi 0x%x (pc: 0x%x)\n", XM_PARTITION_SELF, swiNr, ctxt->lr);
    ArchUnexpectedSwi(swiNr, ctxt);

//    printf("[P%d] Going HALT\n", XM_PARTITION_SELF);
//    XM_halt_partition(XM_PARTITION_SELF);
}

static void UnexpectedDataAbort(trapCtxt_t *ctxt)
{
    printf("[P%d] Unexpected DataAbort (pc: 0x%x)\n", XM_PARTITION_SELF, ctxt->lr);
    ArchUnexpectedDataAbort(ctxt);
    ctxt->pc += 0x4;

//    printf("[P%d] Going HALT\n", XM_PARTITION_SELF);
//    XM_halt_partition(XM_PARTITION_SELF);
}

static void UnexpectedPrefetchAbort(trapCtxt_t *ctxt)
{
    printf("[P%d] Unexpected PrefetchAbort (pc: 0x%x)\n", XM_PARTITION_SELF, ctxt->lr);
    ArchUnexpectedPrefetchAbort(ctxt);

    printf("[P%d] Going HALT\n", XM_PARTITION_SELF);
    XM_halt_partition(XM_PARTITION_SELF);
}

static void UnexpectedUndefInstr(trapCtxt_t *ctxt)
{
    printf("[P%d] Unexpected UndefInstr (pc: 0x%x)\n", XM_PARTITION_SELF, ctxt->lr);
    ArchUnexpectedUndefInstr(ctxt);

    printf("[P%d] Going HALT\n", XM_PARTITION_SELF);
    XM_halt_partition(XM_PARTITION_SELF);
}
#ifdef CONFIG_VFP_EXTENSION
static void UnexpectedFPFault(trapCtxt_t *ctxt)
{
    printf("[P%d] Unexpected FP Fault (pc: 0x%x)\n", XM_PARTITION_SELF, ctxt->lr);
__asm__ __volatile__(   "fmrx r0, FPEXC\n\t"                            \
                        "orr  r0, r0, #0x40000000\n\t"                  \
                        "fmxr FPEXC,r0\n\t"                             \
                        :::"r0");
}
#endif /*CONFIG_VFP_EXTENSION*/

xm_s32_t InstallDataAbortHandler(exceptionHandler_t handler)
{
    if (handler)
        exceptionHandlersTab[XAL_EXCEPTION_DATA_ABORT] = handler;
    else
        exceptionHandlersTab[XAL_EXCEPTION_DATA_ABORT] = UnexpectedDataAbort;
    return 0;
}

xm_s32_t InstallPrefetchAbortHandler(exceptionHandler_t handler)
{
    if (handler)
        exceptionHandlersTab[XAL_EXCEPTION_PREFETCH_ABORT] = handler;
    else
        exceptionHandlersTab[XAL_EXCEPTION_PREFETCH_ABORT] = UnexpectedPrefetchAbort;
    return 0;
}

xm_s32_t InstallUndefInstrHandler(exceptionHandler_t handler)
{
    if (handler)
        exceptionHandlersTab[XAL_EXCEPTION_UNDEF_INSTRUCTION] = handler;
    else
        exceptionHandlersTab[XAL_EXCEPTION_UNDEF_INSTRUCTION] = UnexpectedUndefInstr;
    return 0;
}

#ifdef CONFIG_VFP_EXTENSION
xm_s32_t InstallFPFault(exceptionHandler_t handler)
{
    if (handler)
        exceptionHandlersTab[XAL_EXCEPTION_FP_FAULT] = handler;
    else
        exceptionHandlersTab[XAL_EXCEPTION_FP_FAULT] = UnexpectedFPFault;
    return 0;
}
#endif /*CONFIG_VFP_EXTENSION*/

xm_s32_t InstallIrqHandler(xm_s32_t irqNr, exceptionHandler_t handler)
{
    if (irqNr < 0 || irqNr > IRQTAB_LENGTH)
        return -1;

    if (handler)
        irqHandlersTab[irqNr] = handler;
    else
        irqHandlersTab[irqNr] = UnexpectedIrq;
    return 0;
}

xm_s32_t InstallSwiHandler(xm_s32_t swiNr, swiHandler_t handler)
{
    if (swiNr < 0 || swiNr > SWITAB_LENGTH)
        return -1;

    if (handler)
        swiHandlersTab[swiNr] = handler;
    else
        swiHandlersTab[swiNr] = UnexpectedSwi;
    return 0;
}

void SetupIrqs(void)
{
    xm_s32_t e;

    for (e = 0; e < IRQTAB_LENGTH; e++)
    {
        irqHandlersTab[e] = UnexpectedIrq;
    }

    /*    for (e=0; e<XM_VT_EXT_MAX; e++)
     XM_route_irq(XM_EXTIRQ_TYPE, e, 224+e);*/
}

void SetupSwi(void)
{
    xm_s32_t e;

    for (e = 0; e < SWITAB_LENGTH; e++)
        swiHandlersTab[e] = UnexpectedSwi;

}

void SetupExceptions(void)
{
    exceptionHandlersTab[XAL_EXCEPTION_UNDEF_INSTRUCTION] = UnexpectedUndefInstr;
    exceptionHandlersTab[XAL_EXCEPTION_PREFETCH_ABORT] = UnexpectedPrefetchAbort;
    exceptionHandlersTab[XAL_EXCEPTION_DATA_ABORT] = UnexpectedDataAbort;
#ifdef CONFIG_VFP_EXTENSION
    exceptionHandlersTab[XAL_EXCEPTION_FP_FAULT] = UnexpectedFPFault;
#endif /*CONFIG_VFP_EXTENSION*/
}
#endif /*CONFIG_ARM*/
