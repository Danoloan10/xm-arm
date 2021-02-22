/*
 * $FILE: physmm.c
 *
 * Physical memory manager
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
#include <list.h>
#include <physmm.h>
#include <processor.h>
#include <sched.h>
#include <smp.h>
#include <stdc.h>
#include <arch/xm_def.h>

struct physPage *PmmFindAnonymousPage(xmAddress_t pAddr) {
    xm_s32_t e;

    pAddr&=PAGE_MASK;
    for (e=0; e<xmcTab.noRegions; e++) {
	if ((pAddr>=xmcMemRegTab[e].startAddr)&&((pAddr+PAGE_SIZE-1)<=(xmcMemRegTab[e].startAddr+xmcMemRegTab[e].size-1))) {
	    if (!(xmcMemRegTab[e].flags&XMC_REG_FLAG_PGTAB)) return 0;

	    return (struct physPage *)1;
	}
    }
    PWARN("Page 0x%x does not exist\n", pAddr);

    return 0;
}

struct physPage *PmmFindPage(xmAddress_t pAddr, kThread_t *k, xm_u32_t *flags) {
    struct xmcMemoryArea *memArea;    
    struct xmcPartition *cfg;
    xm_s32_t e;
    pAddr&=PAGE_MASK;
    cfg=k->ctrl.g->cfg;
    for (e=0; e<cfg->noPhysicalMemoryAreas; e++) {
	memArea=&xmcPhysMemAreaTab[e+cfg->physicalMemoryAreasOffset];
	if ((pAddr>=memArea->startAddr)&&((pAddr+PAGE_SIZE-1)<=(memArea->startAddr+memArea->size-1))) {
	    if (!(xmcMemRegTab[memArea->memoryRegionOffset].flags&XMC_REG_FLAG_PGTAB)) return 0;
	    if (flags)
		*flags=memArea->flags;
	    return (struct physPage *)1;
	}
    }
    PWARN("Page 0x%x does not belong to %d\n", pAddr, cfg->rId);

    return 0;
}

xm_s32_t PmmFindArea(xmAddress_t pAddr, xmSSize_t size, kThread_t *k, xm_u32_t *flags) {
    struct xmcMemoryArea *memArea;
    struct xmcPartition *cfg;
    xm_s32_t l, r, c;
    xmAddress_t a, b;

    if (k) {
        cfg=k->ctrl.g->cfg;
        for (l=0, r=cfg->noPhysicalMemoryAreas-1; l<=r; ) {
            c=(l+r)>>1;
            memArea=&xmcPhysMemAreaTab[c+cfg->physicalMemoryAreasOffset];
            a=memArea->startAddr;
            b=a+memArea->size-1;
            if (pAddr<a) {
                r=c-1;
            } else {
                if ((pAddr+size-1)>b) {
                l=c+1;
                } else {
                    ASSERT((pAddr>=a)&&((pAddr+size-1)<=b));
                    if (flags)
                        *flags=memArea->flags;
                    return 1;
                }
            }
        }
    } else {
        for (l=0, r=xmcTab.noRegions-1; l<=r; ) {
            c=(l+r)>>1;
            a=xmcMemRegTab[c].startAddr;
            b=a+xmcMemRegTab[c].size-1;
            if (pAddr<a) {
                r=c-1;
            } else {
                if ((pAddr+size-1)>b) {
                l=c+1;
                } else {
                    ASSERT((pAddr>=a)&&((pAddr+size-1)<=b));
                    if (flags)
                        *flags=xmcMemRegTab[c].flags;
                    return 1;
                }
            }
        }
    }
    return 0;
}

xm_s32_t CheckGParam(void *param, xmSize_t size, xm_u32_t aligment, xm_s32_t flags) {
    localSched_t *sched=GET_LOCAL_SCHED();
    struct xmcMemoryArea *memArea;
    struct xmcPartition *cfg;
    xm_s32_t l, r, c;
    xmAddress_t a, b, pAddr;

    if (!(flags&PFLAG_NOT_NULL)&&!param)
        return 0;
    
    if (!(flags&PFLAG_RW))
        return 0;

    cfg=sched->cKThread->ctrl.g->cfg;
    pAddr=(xmAddress_t)param;

    for (l=0, r=cfg->noPhysicalMemoryAreas-1; l<=r; ) {
        c=(l+r)>>1;
        memArea=&xmcPhysMemAreaTab[c+cfg->physicalMemoryAreasOffset];
        a=memArea->startAddr;
        b=a+memArea->size-1;
        if (pAddr<a) {
            r=c-1;
        } else {
            if (pAddr>b) {
                l=c+1;
            } else {
                ASSERT((pAddr>=a)&&((pAddr+(size-1))<=b));
                return 0;
            }
        }
    }

    return -1;

}

void PmmCheckPhysMemAreaTab(kThread_t *k) {
}

void PmmResetPartition(kThread_t *k) {
}

void *VCacheMapPage(xm_u32_t pAddr, struct physPage *page) {
    return (void*)pAddr;
}

void VCacheUnlockPage(struct physPage *page) {
}

void SetupPhysMM(void) {
}
