/*
 * $FILE: virtmm.c
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
 */

#include <assert.h>
#include <rsvmem.h>
#include <boot.h>
#include <processor.h>
#include <virtmm.h>
#include <vmmap.h>
#include <stdc.h>
#include <arch/paging.h>
#include <arch/xm_def.h>

static xmAddress_t vmmStartAddr; 
static xm_s32_t noFrames;

xm_s32_t VmmGetNoFreeFrames(void) {
    ASSERT(noFrames>=0);
    return noFrames;
}

void __VBOOT SetupVirtMM(void) {
    xmAddress_t st, end;
    xm_u32_t flags;

    st=xmcPhysMemAreaTab[xmcTab.hpv.physicalMemoryAreasOffset].startAddr;
    end=st+xmcPhysMemAreaTab[xmcTab.hpv.physicalMemoryAreasOffset].size-1;
    flags=xmcPhysMemAreaTab[xmcTab.hpv.physicalMemoryAreasOffset].flags;
    eprintf("XM map: [0x%"PRNT_ADDR_FMT"x - 0x%"PRNT_ADDR_FMT"x] flags: 0x%x\n", st, end, flags);
    ASSERT(st==CONFIG_XM_LOAD_ADDR);    
    SetupVmMap(&vmmStartAddr, &noFrames);
    eprintf("[VMM] Free [0x%"PRNT_ADDR_FMT"x-0x%"PRNT_ADDR_FMT"x] %d frames\n", vmmStartAddr, XM_VMAPEND, noFrames);
}
