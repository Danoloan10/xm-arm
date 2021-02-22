/*
 * $FILE: gaccess.h
 *
 * Access rules from XM to partition address space
 *
 * $VERSION$
 *
 * Author: Miguel Masmano <mmasmano@fentiss.com>
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license terms.
 */

#ifndef _XM_GACCESS_H_
#define _XM_GACCESS_H_

#ifndef _XM_KERNEL_
#error Kernel file, do not include.
#endif

#include <arch/gaccess.h>
#include <vmmap.h>

#define __gParam __archGParam

#define PFLAG_RW 0x1
#define PFLAG_NOT_NULL 0x2

#ifdef CONFIG_MMU
static inline xm_s32_t CheckGParam(void *param, xmSize_t size, xm_u32_t aligment, xm_s32_t flags) {

    if (!(flags&PFLAG_NOT_NULL)&&!param)
        return 0;

    if ((((xmAddress_t)param>=CONFIG_XM_OFFSET)&&((xmAddress_t)param<(CONFIG_XM_OFFSET+1*16*1024*1024)))||
        (((xmAddress_t)param+size>=CONFIG_XM_OFFSET) && (((xmAddress_t)param+size)<(CONFIG_XM_OFFSET+1*16*1024*1024))))
        return -1;
    if (((xm_u32_t)param&(aligment-1)))
        return -1;
    if(VmMapCheckMemAreaAccess(((xmAddress_t)(param)), size, 0, 1))
        return -1;
    if (flags&PFLAG_RW)
    {
        if(VmMapCheckMemAreaAccess(((xmAddress_t)(param)), size, 1, 1))
            return -1;
    }
    return 0;
}

#else // CONFIG_MPU
extern xm_s32_t CheckGParam(void *param, xmSize_t size, xm_u32_t aligment, xm_s32_t flags);

#endif

#endif
