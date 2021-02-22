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
#include <boot.h>

void __VBOOT SetupVirtMM(xmAddress_t sAddr, xmAddress_t eAddr) {
}

xm_s32_t SetupPageTable(kThread_t *k) {
/*    struct xmcPartition *cfg=k->ctrl.g->cfg;
    xm_s32_t e;
    for (e=0; e<cfg->noPhysicalMemoryAreas; e++) {
        if (xmcPhysMemAreaTab[e+cfg->physicalMemoryAreasOffset].flags&XM_MEM_AREA_UNCACHEABLE)
	   CLEAR_KTHREAD_FLAGS(k, KTHREAD_DCACHE_ENABLED_F|KTHREAD_ICACHE_ENABLED_F);
        else
        {
           SET_KTHREAD_FLAGS(k, KTHREAD_DCACHE_ENABLED_F|KTHREAD_ICACHE_ENABLED_F);
    //       SetCacheState(DCACHE|ICACHE);
        }
    }*/
    return 0;
}
