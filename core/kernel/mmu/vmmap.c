/*
 * $FILE: vmmap.c
 *
 * Virtual memory map
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
 * - [07/09/16:XM-ARM-2:SPR-160907-01:#122] Assign Partition page tables attributes properly.
 */

#include <assert.h>
#include <kthread.h>
#include <physmm.h>
#include <rsvmem.h>
#include <sched.h>
#include <vmmap.h>
#include <virtmm.h>
#include <arch/paging.h>
#include <arch/physmm.h>

static inline xmAddress_t VAddr2PAddr(struct xmcMemoryArea *mAreas, xm_s32_t noAreas, xmAddress_t vAddr) {
    xm_s32_t e;
    for (e=0; e<noAreas; e++)
        if ((mAreas[e].mappedAt<=vAddr)&&(((mAreas[e].mappedAt+mAreas[e].size)-1)>=vAddr))
            return vAddr-mAreas[e].mappedAt+mAreas[e].startAddr;
    return -1;
}


#define GET_PTDL1_ARCH(ptd, Size, Align) GET_MEMAZ(ptd, Size, Align)

#ifdef CONFIG_ARM
xmWord_t MapPartitionControlTable(xmAddress_t * _partitionThread)
{
	kThread_t * partitionThread = ((kThread_t *)(_partitionThread));
//    if (VmMapUser(_ptdL1, _VIRT2PHYS(pct), CONFIG_XM_OFFSET+16*1024*1024, PAGE_SIZE, attr)<0)
	xmWord_t attr=_PG_ATTR_PRESENT|_PG_ATTR_USER;
//	xmAddress_t vAddr = _VIRT2PHYS(pct);
	xmAddress_t vAddr = XM_PCTRLTAB_VADDR;
	xm_s32_t aux;
/*	aux = VmMapUser(((xmWord_t*)(partitionThread->ctrl.g->kArch.ptdL1S_p)),
					_VIRT2PHYS(partitionThread->ctrl.g->partCtrlTab),
					vAddr,
					PAGE_SIZE,
					((xm_u32_t)(attr)) );
    if (aux<0)
        return 0;*/
    aux = VmMapUser(((xmWord_t*)(partitionThread->ctrl.g->kArch.ptdL1NS)),
    				_VIRT2PHYS(partitionThread->ctrl.g->partCtrlTab),
					vAddr,
					PAGE_SIZE,
					((xm_u32_t)(attr)));
    if (aux<0)
        return 0;
    return _VIRT2PHYS(partitionThread->ctrl.g->kArch.ptdL1NS);
}
/*xmWord_t UnMapPartitionControlTable(xmAddress_t pct)
{
//    if (VmMapUser(_ptdL1, _VIRT2PHYS(pct), CONFIG_XM_OFFSET+16*1024*1024, PAGE_SIZE, attr)<0)
	xmWord_t attr=_PG_ATTR_PRESENT|_PG_ATTR_USER;
    if (__UnMapAddr(_ptdL1, _VIRT2PHYS(pct), pct, PAGE_SIZE, attr)<0)
    	return 0;
    return _VIRT2PHYS(_ptdL1);
}*/
#endif /* CONGIG_ARM */


xm_u32_t SetupPageTable(struct xmcPartition *cfg, xmAddress_t pct, xmWord_t ** ptdL1S_p, xmWord_t ** ptdL1NS_p) {
//	eprintf("SetupPageTable\n");
//#define	USER_MAPP_IN_FIXED_SIZE
#ifndef USER_MAPP_IN_FIXED_SIZE
    xmAddress_t size;
#endif /*USER_MAPP_IN_FIXED_SIZE*/
    xmAddress_t vAddr, addr;
    xmWord_t attr;
    xm_s32_t e;

	extern xmWord_t **ptdL1S_list;
	*ptdL1S_p = ptdL1S_list[cfg->rId];
    GET_MEMAZ(*ptdL1NS_p, PTDL1SIZE, PTDL1ALIGN);

#if 0
    kprintf("User/Partition ptdL1S_p 0x%x ptdL1NS 0x%x\n", *ptdL1S_p, *ptdL1NS_p);
#endif

	SetupPtdL1(*ptdL1NS_p, cfg);

	/*	unsigned int * aux;
	for(aux=ptdL1;aux<ptdL1+(0x100);aux++)
		if((*aux & (_PG_ARCH_PTE_PRESENT | _PG_ARCH_PTD_PRESENT)))
			eprintf("Usr mapped: 0x%x=0x%x\n",aux,*aux);*/

    for (e=0; e<cfg->noPhysicalMemoryAreas; e++) {
        if (xmcPhysMemAreaTab[e+cfg->physicalMemoryAreasOffset].flags&XM_MEM_AREA_UNMAPPED)
            continue;
        addr=xmcPhysMemAreaTab[e+cfg->physicalMemoryAreasOffset].startAddr;
#ifndef USER_MAPP_IN_FIXED_SIZE
        size=xmcPhysMemAreaTab[e+cfg->physicalMemoryAreasOffset].size;
#endif /*USER_MAPP_IN_FIXED_SIZE*/
        vAddr=xmcPhysMemAreaTab[e+cfg->physicalMemoryAreasOffset].mappedAt;
        attr=_PG_ATTR_PRESENT|_PG_ATTR_USER;
        
        if (!(xmcPhysMemAreaTab[e+cfg->physicalMemoryAreasOffset].flags&XM_MEM_AREA_UNCACHEABLE))
            attr|=_PG_ATTR_CACHED;
        
        if (!(xmcPhysMemAreaTab[e+cfg->physicalMemoryAreasOffset].flags&XM_MEM_AREA_READONLY))
            attr|=_PG_ATTR_RW;
#ifdef USER_MAPP_IN_FIXED_SIZE
        if (VmMapUser(*ptdL1NS_p, addr, vAddr, LPAGE_SIZE, attr)<0)
            return XM_INVALID_CONFIG;
        xm_u32_t cnt;
    	for(cnt=0;cnt<xmcTab.noPartitions;cnt++)
    	{
    		if (VmMapUser(ptdL1S_list[cnt], addr, vAddr, LPAGE_SIZE, attr)<0)
    		//if (MapAddr(ptdL1S_list[cnt], addr, vAddr, LPAGE_SIZE, (VmAttr2ArchAttr(attr)))<0)
                return XM_INVALID_CONFIG;
    	}
#else /*USER_MAPP_IN_FIXED_SIZE*/
        if (VmMapUser(*ptdL1NS_p, addr, vAddr, size, attr)<0)
            return XM_INVALID_CONFIG;
        xm_u32_t cnt;
    	for(cnt=0;cnt<xmcTab.noPartitions;cnt++)
    	{
            if (VmMapUser(ptdL1S_list[cnt], addr, vAddr, size, attr|_PG_ATTR_RW)<0)
    		//if (MapAddr(ptdL1S_list[cnt], addr, vAddr, size, VmAttr2ArchAttr(attr|_PG_ATTR_RW))<0)
                return XM_INVALID_CONFIG;
    	}
#endif /*USER_MAPP_IN_FIXED_SIZE*/
        attr=_PG_ATTR_PRESENT|_PG_ATTR_USER;

#ifndef CONFIG_ARM
        if (VmMapUser(ptdL1, _VIRT2PHYS(pct), CONFIG_XM_OFFSET+16*1024*1024, PAGE_SIZE, attr)<0)
        	return XM_INVALID_CONFIG;
#else
        /*Is mapped in the pre_ctxt_sw*/
#endif
    }
    return XM_OK;
}
