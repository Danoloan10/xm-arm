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
/*
 * Changelog:
 * - [21/01/16:XM-ARM-2:SPR-160121-05:#107] Update XAL API HCalls.
 */

#ifndef _XAL_IRQS_H_
#define _XAL_IRQS_H_

#include <arch/irqs.h>
#include <xm.h>

#ifdef __cplusplus
extern "C" {
#endif

//#define XAL_XMEXT_TRAP(_xmtrap)  ((( _xmtrap ) - XM_VT_EXT_FIRST)+224)

typedef void (*exceptionHandler_t)(trapCtxt_t *);

extern exceptionHandler_t trapHandlersTab[];
extern xm_s32_t InstallIrqHandler(xm_s32_t trapNr, exceptionHandler_t handler);
extern void SetupIrqs(void);

#ifdef CONFIG_ARM
typedef void (*irqHandler_t)(irqCtxt_t *);
typedef void (*swiHandler_t)(int ,irqCtxt_t *);

extern irqHandler_t irqHandlersTab[];
extern swiHandler_t swiHandlersTab[];
extern exceptionHandler_t exceptionHandlersTab[];


extern xm_s32_t InstallDataAbortHandler(exceptionHandler_t handler);
extern xm_s32_t InstallPrefetchAbortHandler(exceptionHandler_t handler);
extern xm_s32_t InstallUndefInstrHandler(exceptionHandler_t handler);
#ifdef CONFIG_VFP_EXTENSION
extern xm_s32_t InstallFPFault(exceptionHandler_t handler);
#endif /*CONFIG_VFP_EXTENSION*/

extern xm_s32_t InstallIrqHandler(xm_s32_t irqNr, exceptionHandler_t handler);
extern xm_s32_t InstallSwiHandler(xm_s32_t swiNr, swiHandler_t handler);
extern void SetupSwi(void);

#endif /*CONFIG_ARM*/

#ifdef __cplusplus
}
#endif

#endif /*_XAL_IRQS_H_*/
