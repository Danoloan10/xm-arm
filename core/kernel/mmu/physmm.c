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
#include <rsvmem.h>
#include <physmm.h>
#include <processor.h>
#include <sched.h>
#include <stdc.h>
#include <virtmm.h>
#include <vmmap.h>
#include <arch/paging.h>
#include <arch/physmm.h>
#include <arch/xm_def.h>

struct physPage *PmmFindAnonymousPage(xmAddress_t pAddr) {
    xm_s32_t l, r, c;
    xmAddress_t a, b;

    pAddr&=PAGE_MASK;
    for (l=0, r=xmcTab.noRegions-1; l<=r; ) {
        c=(l+r)>>1;
        a=xmcMemRegTab[c].startAddr;
        b=a+xmcMemRegTab[c].size-1;
        if (pAddr<a) {
            r=c-1;
        } else {
            if (pAddr>b) {
                l=c+1;
            } else {
                ASSERT((pAddr>=a)&&((pAddr+PAGE_SIZE-1)<=b));
                if (!(xmcMemRegTab[c].flags&XMC_REG_FLAG_PGTAB)) return 0;

                return 0;//&physPageTab[c][(pAddr-a)>>PAGE_SHIFT];
            }
        }
    }

    PWARN("Page 0x%x does not exist\n", pAddr);

    return 0;
}

struct physPage *PmmFindPage(xmAddress_t pAddr, partition_t *p, xm_u32_t *flags) {
    struct xmcMemoryArea *memArea;    
    xm_s32_t l, r, c;
    xmAddress_t a, b;
    ASSERT(p);
    pAddr&=PAGE_MASK;
    for (l=0, r=p->cfg->noPhysicalMemoryAreas-1; l<=r; ) {
        c=(l+r)>>1;
        memArea=&xmcPhysMemAreaTab[c+p->cfg->physicalMemoryAreasOffset];
        a=memArea->startAddr;
        b=a+memArea->size-1;
        if (pAddr<a) {
            r=c-1;
        } else {
            if (pAddr>b) {
                l=c+1;
            } else {
                struct xmcMemoryRegion *memRegion=
                    &xmcMemRegTab[memArea->memoryRegionOffset];
                ASSERT((pAddr>=a)&&((pAddr+PAGE_SIZE-1)<=b));
                if (!(memRegion->flags&XMC_REG_FLAG_PGTAB))
                    return 0;
                if (flags)
                    *flags=memArea->flags;
                
                return 0;//&physPageTab[memArea->memoryRegionOffset][(pAddr-memRegion->startAddr)>>PAGE_SHIFT];
            }                
        }
    }

    PWARN("Page 0x%x does not belong to %d\n", pAddr, p->cfg->rId);

    return 0;
}

xm_s32_t PmmFindAddr(xmAddress_t pAddr, partition_t *p, xm_u32_t *flags) {
    struct xmcMemoryArea *memArea;
    xm_s32_t l, r, c;
    xmAddress_t a, b;

    pAddr&=PAGE_MASK;

    for (l=0, r=p->cfg->noPhysicalMemoryAreas-1; l<=r; ) {
        c=(l+r)>>1;
        memArea=&xmcPhysMemAreaTab[c+p->cfg->physicalMemoryAreasOffset];
        a=memArea->startAddr;
        b=a+memArea->size-1;
        if (pAddr<a) {
            r=c-1;
        } else {
            if (pAddr>b) {
                l=c+1;
            } else {
                ASSERT((pAddr>=a)&&((pAddr+PAGE_SIZE-1)<=b));
                if (flags)
                    *flags=memArea->flags;
                return 1;
            }
        }
    }

    PWARN("Page 0x%x does not belong to %d\n", pAddr, p->cfg->rId);

    return 0;
}

xm_s32_t PmmFindArea(xmAddress_t pAddr, xmSSize_t size, partition_t *p, xm_u32_t *flags) {
    struct xmcMemoryArea *memArea;
    xm_s32_t l, r, c;
    xmAddress_t a, b;

    if (p) {
        for (l=0, r=p->cfg->noPhysicalMemoryAreas-1; l<=r; ) {
            c=(l+r)>>1;
            memArea=&xmcPhysMemAreaTab[c+p->cfg->physicalMemoryAreasOffset];
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

static inline void PmmCheckMemRegTab(void) {
    xmAddress_t a, b;
    xm_s32_t e;

    a=xmcMemRegTab[0].startAddr;
    for (e=1; e<xmcTab.noRegions; e++) {
        b=xmcMemRegTab[e].startAddr;
        if (a>=b) 
            PartitionPanic(0, "[PmmCheckMemRegTab] unsorted memory regions table");
        a=b;
    }
}

void PmmCheckPhysMemAreaTab(partition_t *p) {
    xmAddress_t a, b;
    xm_s32_t e;

    a=xmcPhysMemAreaTab[p->cfg->physicalMemoryAreasOffset].startAddr;
    for (e=1; e<p->cfg->noPhysicalMemoryAreas; e++) {
        b=xmcPhysMemAreaTab[e+p->cfg->physicalMemoryAreasOffset].startAddr;
        if (a>=b) 
            PartitionPanic(0, "[PmmCheckPhysMemAreaTab] Partition %d: unsorted physical memory areas", p->cfg->rId);
        a=b;
    }
}

void PmmResetPartition(partition_t *p) {
/*
    struct xmcMemoryRegion *memRegion;
    struct xmcMemoryArea *memArea;
    struct xmcPartition *cfg;
    struct physPage *page;
    xmAddress_t addr;
    xm_s32_t e;
    cfg=k->ctrl.g->cfg;
    for (e=0; e<cfg->noPhysicalMemoryAreas; e++) {
        memArea=&xmcPhysMemAreaTab[e+cfg->physicalMemoryAreasOffset];
        memRegion=&xmcMemRegTab[memArea->memoryRegionOffset];
        if (memRegion->flags&XMC_REG_FLAG_PGTAB)
            for (addr=memArea->startAddr; addr<memArea->startAddr+memArea->size; addr+=PAGE_SIZE) {
                page=&physPageTab[memArea->memoryRegionOffset][(addr-memRegion->startAddr)>>PAGE_SHIFT];
                page->type=PPAG_STD;
                page->counter=0;
                if (page->mapped)
                    VCacheUnlockPage(page);
            }
    }
*/
}

void *VCacheMapPage(xmAddress_t pAddr, struct physPage *page) {
    return (void *)pAddr;
}

void VCacheUnlockPage(struct physPage *page) {
}

void SetupPhysMM(void) {
	ArchSetupPhysMM();
    PmmCheckMemRegTab();
}


