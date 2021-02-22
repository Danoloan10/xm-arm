/*
 * $FILE: mem.c
 *
 * System physical memory
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
 * - [21/01/16:XM-ARM-2:SPR-160121-03:#105]: Remove ambiguous tab UNCHECKED_ID
 * - [21/01/16:XM-ARM-2:SPR-160121-04:#106]: Memory access checks as Secure when XM_HYPERVISOR_ID is set.
 */

#include <assert.h>
#include <boot.h>
#include <hypercalls.h>
#include <objdir.h>
#include <physmm.h>
#include <vmmap.h>
#include <sched.h>
#include <stdc.h>

#include <objects/mem.h>

static inline xm_s32_t CopyArea(xmAddress_t dstAddr, xmId_t dstId, xmAddress_t srcAddr, xmId_t srcId, xmSSize_t size) {
    localProcessor_t *info=GET_LOCAL_PROCESSOR();
    xm_u32_t flags;

	/* SPR-150415-07 */
    xmId_t dstPId = PartitionId2PId(dstId);
    xmId_t srcPId = PartitionId2PId(srcId);
    xmId_t currId = PartitionId2PId(GetPartitionCfg(info->sched.cKThread)->rId);

    if (size<=0) return 0;

    if (dstId != currId)
    {   if (!(GetPartitionCfg(info->sched.cKThread)->flags&XM_PART_SYSTEM))
        {   return XM_PERM_ERROR;   }
    }
    if (srcId != currId)
    {   if (!(GetPartitionCfg(info->sched.cKThread)->flags&XM_PART_SYSTEM))
        {   return XM_PERM_ERROR;   }
    }
    if (dstPId!=XM_HYPERVISOR_ID)
    {
        if ((dstPId<0)||(dstPId>=xmcTab.noPartitions))
        {   return XM_INVALID_PARAM;   }

        if (!PmmFindArea(dstAddr, size, (dstPId!=XM_HYPERVISOR_ID)?&partitionTab[dstPId]:0, &flags))
        {   return XM_INVALID_PARAM;   }

        if (flags&XM_MEM_AREA_READONLY)
        {   return XM_INVALID_PARAM;   }
    }

    if( VmMapCheckAddressReadAccess(srcAddr, ((srcPId!=XM_HYPERVISOR_ID)?1:0)/*(srcPId==currId)*/ ) )
    {	return XM_INVALID_PARAM; }
    if( VmMapCheckAddressWriteAccess(dstAddr, ((srcPId!=XM_HYPERVISOR_ID)?1:0)/*(dstPId==currId)*/ ) )
    {	return XM_INVALID_PARAM; }

    if (srcPId!=XM_HYPERVISOR_ID)
    {
        if ((srcPId<0)||(srcPId>=xmcTab.noPartitions))
        {   return XM_INVALID_PARAM;   }
        
        if (!PmmFindArea(srcAddr, size, (srcPId!=XM_HYPERVISOR_ID)?&partitionTab[srcPId]:0, &flags))
        {   return XM_INVALID_PARAM;   }

/*      if (flags&XM_MEM_AREA_READONLY)
            return XM_INVALID_PARAM;*/
    }

#if defined(CONFIG_MPU)
    memcpy((void *)dstAddr, (void*)srcAddr, size);
#else
    /*TODO ARMPorting: Not implemented yet*/
    //Todo MEMREAD from leon.h
#	ifndef CONFIG_ARM
	UnalignMemCpy((void *)dstAddr, (void*)srcAddr, size, (RdMem_t)ReadByPassMmuWord, (RdMem_t)ReadByPassMmuWord, (WrMem_t)WriteByPassMmuWord);
#	else
#if 0
	xm_u32_t mode;
	READ_CPSR(mode);
	ASSERT((mode & PSR_MODE)==PSR_MODE_MON);
	xm_u32_t sec;
	RD_SCR(sec);
	ASSERT((sec & CA9_SCR_NS_MASK)==0);
#endif
	memcpy(((void *)(dstAddr)), ((void*)(srcAddr)), size);
#	endif
#endif
    
    return size;
}

/*#if 0
static inline xm_s32_t CopyMem(void *dst, void *src, kThread_t *dstThread, kThread_t *srcThread, xm_s32_t size) {
    struct physPage *pageSrc, *pageDst;
    xm_u8_t *vAddrSrc, *vAddrDst;
    xmAddress_t addrSrc, addrDst;
    xm_u32_t flags;
    xm_s32_t s;

    if (size<=0) return 0;
    for (addrDst=(xmAddress_t)dst, addrSrc=(xmAddress_t)src, s=size; addrSrc<((xmAddress_t)src+size); addrSrc+=PAGE_SIZE, addrDst+=PAGE_SIZE, s-=PAGE_SIZE) {
	if (srcThread)
	    pageSrc=PmmFindPage(addrSrc&PAGE_MASK, srcThread, 0);
	else
	    pageSrc=PmmFindAnonymousPage(addrSrc&PAGE_MASK);
	if (!pageSrc)
	    return XM_INVALID_PARAM;

	if (dstThread) {
	    pageDst=PmmFindPage(addrDst&PAGE_MASK, dstThread, &flags);	
	    if ((pageDst)&&(flags&XM_MEM_AREA_READONLY))
		return XM_PERM_ERROR;
		
	} else
	    pageDst=PmmFindAnonymousPage(addrDst&PAGE_MASK);
	if (!pageDst)
	    return XM_INVALID_PARAM;

	vAddrSrc=VCacheMapPage(addrSrc, pageSrc);
	vAddrDst=VCacheMapPage(addrDst, pageDst);
	memcpy(vAddrDst, vAddrSrc, (s<PAGE_SIZE)?s:PAGE_SIZE);
	VCacheUnlockPage(pageSrc);
	VCacheUnlockPage(pageDst);
    }

    return size;
}
#endif*/

static xm_s32_t CtrlMem(xmObjDesc_t desc, xm_u32_t cmd, union memCmd *__gParam args) {
    if (!args)
	return XM_INVALID_PARAM;

    if (CheckGParam(args, sizeof(union memCmd), 4, PFLAG_NOT_NULL)<0) 
        return XM_INVALID_PARAM;

    switch(cmd) {
    case XM_OBJ_MEM_CPY_AREA:
        return CopyArea(args->cpyArea.dstAddr, args->cpyArea.dstId, args->cpyArea.srcAddr, args->cpyArea.srcId, args->cpyArea.size);
        break;
    }

    return XM_INVALID_PARAM;
}

static const struct object memObj={
    .Ctrl=(ctrlObjOp_t)CtrlMem,
};

xm_s32_t __VBOOT SetupMem(void) {    
    objectTab[OBJ_CLASS_MEM]=&memObj;
    return 0;
}

REGISTER_OBJ(SetupMem);

