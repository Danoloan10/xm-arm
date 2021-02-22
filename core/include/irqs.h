/*
 * $FILE: irqs.h
 *
 * IRQS
 *
 * $VERSION$
 *
 * Author: Miguel Masmano <mmasmano@fentiss.com>
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license terms.
 */

#ifndef _XM_IRQS_H_
#define _XM_IRQS_H_

#ifndef _XM_KERNEL_
#error Kernel file, do not include.
#endif

#ifndef __ASSEMBLY__

#include <linkage.h>
#ifdef CONFIG_ARM       /*FIXME: check why the symbolic link is not available*/
#include <arm/irqs.h>
#else
#include <arch/irqs.h>
#endif

#define SCHED_PENDING 0x80000000
#define IRQ_IN_PROGRESS (~SCHED_PENDING)

typedef void (*irqHandler_t)(cpuCtxt_t *, void *);
typedef void (*swiHandler_t)(int ,cpuCtxt_t *);
typedef xm_s32_t (*exceptionHandler_t)(cpuCtxt_t *);

struct irqTabEntry {
    irqHandler_t handler;
    void *data;
};

extern irqHandler_t irqHandlersTab[];
extern swiHandler_t swiHandlersTab[];
extern exceptionHandler_t exceptionHandlersTab[];

extern xm_s32_t InstallDataAbortHandler(exceptionHandler_t handler);
extern xm_s32_t InstallPrefetchAbortHandler(exceptionHandler_t handler);
extern xm_s32_t InstallUndefInstrHandler(exceptionHandler_t handler);
extern xm_s32_t InstallIrqHandler(xm_s32_t irqNr, exceptionHandler_t handler);
extern xm_s32_t InstallSwiHandler(xm_s32_t swiNr, swiHandler_t handler);
#ifdef CONFIG_VFP_EXTENSION
extern xm_s32_t InstallFPFault(exceptionHandler_t handler);
#endif

extern void SetupIrqs(void);
extern void ArchSetupIrqs(void);
extern irqHandler_t SetIrqHandler(xm_s32_t, irqHandler_t, void *);

// Control over each interrupt
typedef struct {
    void (*Enable)(xm_u32_t irq);
    void (*Disable)(xm_u32_t irq);
    void (*Ack)(xm_u32_t irq);
    void (*End)(xm_u32_t irq);
    void (*Force)(xm_u32_t irq);
    void (*Clear)(xm_u32_t irq);
} hwIrqCtrl_t;

extern hwIrqCtrl_t hwIrqCtrl[CONFIG_NO_HWIRQS];

#ifdef CONFIG_ARM
extern void HwIrqGetMask(xm_u32_t *mask);
extern void HwIrqSetMask(xm_u32_t *mask);

extern void HwIrqGetPendMask(xm_u32_t *mask);
extern void HwIrqSetPendMask(xm_u32_t *mask);
extern void HwIrqClearPend(void);

extern void HwDisableIrq(xm_u32_t irq);
extern void HwEnableIrq(xm_u32_t irq);
extern void HwEnableIrqFromUsr(xm_u32_t irq);
extern void HwEnableUserIrq(xm_u32_t irq);
extern void HwEnableUserIrqFromUsr(xm_u32_t irq);
extern void HwAckIrq(xm_u32_t irq);
extern void HwEndIrq(xm_u32_t irq);
extern void HwForceIrq(xm_u32_t irq);
extern void HwClearIrq(xm_u32_t irq);

extern void SetupSGIs(void);
#endif /*CONFIG_ARM*/

extern xm_s32_t MaskHwIrq(xm_s32_t irq);
extern xm_s32_t UnmaskHwIrq(xm_s32_t irq);
extern void SetTrapPending(cpuCtxt_t *ctxt);
extern xm_s32_t ArchTrapIsSysCtxt(cpuCtxt_t *ctxt);
extern xmAddress_t IrqVector2Address(xm_s32_t vector);

#endif /*__ASSEMBLY__*/

#endif /*_XM_IRQS_H_*/

