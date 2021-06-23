/*
 * $FILE: irqs.h
 *
 * IRQS
 *
 * $VERSION$
 *
 * Author: Javier Coronel <jcoronel@fentiss.com>
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license terms.
 */
/*
 * Changelog:
 * - [21/01/16:XM-ARM-2:SPR-160121-05:#107] Add FPU management HCalls.
 */
#ifndef _XAL_ARCH_IRQS_H_
#define _XAL_ARCH_IRQS_H_

#include <xm_inc/guest.h>

#define TRAPTAB_LENGTH 256

#define GET_SRC_FAULT(fsr) (fsr&0xf)|((fsr&(1<<10))>>6)

#define ALIGNMENT_FAULT					(0x00000001)

#define SECTION_TRANSLATION_FAULT		(0x00000005)
#define PAGE_TRANSLATION_FAULT			(0x00000007)

#define SECTION_ACCES_FLAG_FAULT		(0x00000003)
#define PAGE_ACCES_FLAG_FAULT			(0x00000006)

#define SECTION_DOMAIN_FAULT			(0x00000009)
#define PAGE_DOMAIN_FAULT				(0x0000000e)

#define SECTION_PERMISION_FAULT			(0x0000000d)
#define PAGE_PERMISION_FAULT			(0x0000000f)

#define SYNC_EXTERNAL_ABORT          	(0x00000008)
#define ASYNC_EXTERNAL_ABORT          	(0x00000016)

#define MEM_ACCESS_SYNC_PARITY_FAULT 	(0x00000019)
#define MEM_ACCESS_ASYNC_PARITY_FAULT 	(0x00000018)


//#define ALIGNMENT_FAULT              0x01
//#define BACKGROUND_FAULT             0x00
//#define PERMISSION_FAULT             0x0d
//#define SYNC_EXTERNAL_ABORT          0x08
//#define MEM_ACCESS_SYNC_PARITY_ERROR 0x19

#define IRQTAB_LENGTH CONFIG_NO_HWIRQS+XM_VT_EXT_MAX  /*reserved for IRQS*/
#define SWITAB_LENGTH 256  /*reserved for SWI*/

#define XAL_XMEXT_IRQ(_xmextirq) ((( _xmextirq ) - XM_VT_EXT_FIRST)+XM_VT_HW_MAX)  
#define XAL_XMEXT_TRAP(_xmextirq) XAL_XMEXT_IRQ(_xmextirq)  /*by compatibility with sparc version*/ 

#define HwCli() __asm__ __volatile__ ("cpsid if\n\t")

#define HwSti() __asm__ __volatile__ ("cpsie if\n\t")

#define FPU_ENABLE()										\
	__asm__ __volatile__ (	"fmrx r0, FPEXC\n\t"			\
							"orr  r0, r0, #0x40000000\n\t"	\
							"fmxr FPEXC, r0\n\t"			\
							:::);
#define FPU_DISABLE()										\
	__asm__ __volatile__ (	"fmrx r0, FPEXC\n\t"			\
							"bic  r0, r0, #0x40000000\n\t"	\
							"fmxr FPEXC, r0\n\t"			\
							:::);

#define XM_EXTIRQ_SGI_NUM	(0)

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __ASSEMBLY__

typedef struct _irqCtxt {
    xm_u32_t irqNr;
    xm_u32_t sp;
    xm_u32_t spsr;
    xm_u32_t cpsr;
    xm_u32_t trapSp;
    xm_u32_t lr;
    xm_u32_t r[13];
    xm_u32_t pc;
} irqCtxt_t,trapCtxt_t;

extern void XalAckIrq(xm_u32_t irqNr);
extern void ArchUnexpectedIrq(irqCtxt_t *ctxt);
extern void ArchUnexpectedSwi(xm_s32_t swiNr, irqCtxt_t *ctxt);
extern void ArchUnexpectedDataAbort(trapCtxt_t *ctxt);
extern void ArchUnexpectedPrefetchAbort(trapCtxt_t *ctxt);
extern void ArchUnexpectedUndefInstr(trapCtxt_t *ctxt);
#   ifdef CONFIG_VFP_EXTENSION
extern xm_u32_t checkFPU(xm_u32_t instruction);
#   endif /*CONFIG_VFP_EXTENSION*/

#endif /*__ASSEMBLY__*/

#ifdef __cplusplus
}
#endif

#endif /*_XAL_ARCH_IRQS_H_*/
