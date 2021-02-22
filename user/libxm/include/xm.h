/*
 * $FILE: xm.h
 *
 * Guest header file
 *
 * $VERSION$
 *
 * Author: Miguel Masmano <mmasmano@fentiss.com>
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license terms.
 */

#ifndef _LIB_XM_H_
#define _LIB_XM_H_

#ifdef _XM_KERNEL_
#error Guest file, do not include.
#endif

#include <xm_inc/config.h>
#include <xm_inc/arch/arch_types.h>
#include <xm_inc/config.h>
#include <xm_inc/xmef.h>
#include <xmhypercalls.h>

#ifndef __ASSEMBLY__

#include <xm_inc/hypercalls.h>
#include <xm_inc/guest.h>

extern struct libXmParams {
    partitionControlTable_t *partCtrlTab;
    struct xmPhysicalMemMap *partMemMap;
    xmWord_t *commPortBitmap;
} libXmParams;

extern __stdcall void init_libxm(partitionControlTable_t *partCtrlTab);

static inline struct xmPhysicalMemMap *XM_get_partition_mmap(void) {    
    return libXmParams.partMemMap;
}

static inline partitionControlTable_t *XM_params_get_PCT(void) {
    return libXmParams.partCtrlTab;
}

static inline xmWord_t *XM_get_commport_bitmap(void) {    
    return libXmParams.commPortBitmap;
}

static inline xmId_t XM_get_number_vcpus(void){
    return (XM_params_get_PCT())->noVCpus;
}

#include <comm.h>
#include <hm.h>
#include <hypervisor.h>
#include <trace.h>
#include <status.h>

#endif /* __ASSEMBLY__ */

#endif /* _LIB_XM_H_ */
