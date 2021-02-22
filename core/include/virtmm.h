/*
 * $FILE: virtmm.h
 *
 * Virtual memory manager
 *
 * $VERSION$
 *
 * Author: Miguel Masmano <mmasmano@fentiss.com>
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license terms.
 *
 */
/*
 * Changelog:
 * - [21/10/16:XM-ARM-2:SPR-161013-02:#128] Define XM_PCTRLTAB_VADDR in a different page for each CPU.
 */

#ifndef _XM_VIRTMM_H_
#define _XM_VIRTMM_H_

#ifndef _XM_KERNEL_
#error Kernel file, do not include.
#endif

#include <arch/arch_types.h>
#ifdef CONFIG_SMP_SUPPORT
#include <arch/smp.h>
#endif
#include <arch/paging.h>

extern xmWord_t XM_PCTRLTAB_VADDR_BASE;
#ifndef CONFIG_SMP_SUPPORT
#define XM_PCTRLTAB_VADDR       ((xmAddress_t)(&XM_PCTRLTAB_VADDR_BASE))
#else
#define XM_PCTRLTAB_VADDR       ((xmAddress_t)(((xmAddress_t)(&XM_PCTRLTAB_VADDR_BASE)) + (GET_CPU_ID()*PAGE_SIZE)))
#endif
#define XM_PCTRLTAB_RSV_SIZE    ((xmSize_t)(LPAGE_SIZE))

extern void SetupVirtMM(void);
extern xmAddress_t VmmAlloc(xm_s32_t npag);
extern void VmmPrintMap(void);
extern xm_s32_t VmmIsFree(xmAddress_t vAddr);
extern xm_s32_t VmmGetNoFreeFrames(void);

#endif
