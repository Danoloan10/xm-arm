/*
 * $FILE: irqs.h
 *
 * IRQS
 *
 * $VERSION$
 *
 * Author: Manuel Muñoz <mmunoz@ai2.upv.es>
 * Author: Javier O. Coronel <jcoronel@fentiss.com>
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license terms.
 */

#ifndef _XM_ARCH_IRQS_H_
#define _XM_ARCH_IRQS_H_

#ifdef _XM_KERNEL_
#include <arch/xm_def.h>
//#include <arch/gic.h>
#else
#include <xm_inc/arch/xm_def.h>
//#include <xm_inc/arch/gic.h>
#endif

#define XM_EXTIRQ_SGI_NUM	(0)

#define XM_SEND_EXTIRQ() 	ICDSGIR_SEND_ITSELF(XM_EXTIRQ_SGI_NUM, ICDSGIR_SATT_NONSECURE)

#ifdef CONFIG_SMP_SUPPORT
#	define XM_IPI_SGI_NUM		(8)
//#	define XM_SEND_IPI() 		ICDSGIR_SEND_TOOTHERS(XM_IPI_SGI_NUM, 0)
#endif


#define HWIRQS_VECTOR_SIZE		\
                (   (CONFIG_NO_HWIRQS%32)   ? \
                    (CONFIG_NO_HWIRQS/32)+1 : \
                    (CONFIG_NO_HWIRQS/32)   )

#define NO_TRAPS 3

#ifndef __ASSEMBLY__
struct trapHandler {
//    xmAddress_t pc;
};
#endif /* __ASSEMBLY__ */

#ifdef _XM_KERNEL_

#ifndef __ASSEMBLY__
typedef struct _cpuCtxt {
    xm_u32_t irqNr;
    xm_u32_t sp;
    xm_u32_t spsr;
    xm_u32_t cpsr;
    xm_u32_t trapSp;
    xm_u32_t lr;
    xm_u32_t r[13];
    xm_u32_t pc;
    //struct _cpuCtxt *prev;
} cpuCtxt_t;

#define CpuCtxt2HmCpuCtxt(cpuCtxt, hmCpuCtxt) do { \
    (hmCpuCtxt)->pc=(cpuCtxt)->pc; \
    (hmCpuCtxt)->cpsr=(cpuCtxt)->cpsr; \
    (hmCpuCtxt)->spsr=(cpuCtxt)->spsr; \
    GET_DFAR((hmCpuCtxt)->far); \
} while(0)


#define PrintHmCpuCtxt(ctxt) \
     kprintf("[HM] PC:0x%lx CPSR:0x%lx SPSR:0x%lx\n", (ctxt)->pc, (ctxt)->cpsr, (ctxt)->spsr)

/*#define PrintHmExtraCtxt(eventId, ctxt) \
     (eventId>XM_HM_EV_ARM_DATA_ABORT)? kprintf("     FAR:0x%lx\n", (ctxt)->far): 0*/

#define GET_ECODE(ctxt) 0
#define GET_CTXT_PC(ctxt) (ctxt)->pc
#define SET_CTXT_PC(ctxt, _pc) do { \
    (ctxt)->pc=_pc; \
} while(0)

#include <guest.h>
#include <arch/processor.h>

#ifndef CONFIG_ARM /*iFlag is not mantained in ARM*/
/*TODO ARMPorting: to be tested */
static inline void InitPCtrlTabIrqs(xm_u32_t *iFlags)
{
    (*iFlags)&=~PSR_I;
}

/*TODO ARMPorting: to be tested */
static inline xm_s32_t ArePCtrlTabIrqsSet(xm_u32_t iFlags)
{
    return ((iFlags&PSR_I)!=PSR_I)?1:0;
}

/*TODO ARMPorting: to be tested */
static inline void DisablePCtrlTabIrqs(xm_u32_t *iFlags) {
    (*iFlags)|=PSR_I;
}

/*TODO ARMPorting: to be tested */
static inline void EnablePCtrlTabIrqs(xm_u32_t *iFlags) {
    (*iFlags)&=~PSR_I;
}
#endif
/*TODO ARMPorting: to be tested */
static inline void MaskPCtrlTabIrq(xm_u32_t *mask, xm_u32_t bitmap) {
    // doing nothing
}

static inline xm_s32_t IsSvIrqCtxt(cpuCtxt_t *ctxt)
{
    return ((ctxt->spsr&PSR_MODE)==PSR_MODE_MON)?1:0;
}

/*TODO ARMPorting: Not implemented*/
static inline xm_s32_t ArchEmulTrapIrq(cpuCtxt_t *ctxt, partitionControlTable_t *partCtrlTab, xm_s32_t irqNr)
{
    return partCtrlTab->trap2Vector[irqNr];
}

/*TODO ARMPorting: Not implemented*/
static inline xm_s32_t ArchEmulHwIrq(cpuCtxt_t *ctxt, partitionControlTable_t *partCtrlTab, xm_s32_t irqNr)
{
    return partCtrlTab->hwIrq2Vector[irqNr];
}

/*TODO ARMPorting: to be tested */
static inline xm_s32_t ArchEmulExtIrq(cpuCtxt_t *ctxt, partitionControlTable_t *partCtrlTab, xm_s32_t irqNr)
{
	partCtrlTab->irqIndex = (irqNr + CONFIG_NO_HWIRQS);
    return partCtrlTab->extIrq2Vector[irqNr];
}
#endif //__ASSEMBLY__

/* <track id="hardware-interrupt-list"> */
//#define UNDEF_INSTR 0x0
//#define PREFETCH_ABORT 0x1
//#define DATA_ABORT 0x2
//
//#ifdef CONFIG_TMS570LS31x
//#define RTI0_IRQ_CHANNEL_NR  2
//#define RTI1_IRQ_CHANNEL_NR  3
//#define RTI2_IRQ_CHANNEL_NR  4
//#define RTI3_IRQ_CHANNEL_NR  5
//#endif
//
//#ifdef CONFIG_MDHBX
//#define TIMER0_IRQ_CHANNEL_NR  0
//#define TIMERN_FIQ_CHANNEL_NR  0
//#endif
///* </track id="hardware-interrupt-list"> */
//
#endif //_XM_KERNEL_

#endif //_XM_ARCH_IRQS_H_
