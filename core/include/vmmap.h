/*
 * $FILE: vmmap.h
 *
 * Virtual memory map manager
 *
 * Version: XtratuM-2.1.0
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license terms.
 */

#ifndef _XM_VMMAP_H_
#define _XM_VMMAP_H_

#ifndef _XM_KERNEL_
#error Kernel file, do not include.
#endif

/*TODO ARMPorting: Find a proper place*/
#ifdef CONFIG_MMU
#define MMU_L12PT_TLB_DESCRIPTOR_TYPE_MASK (0x00000003)
#endif

#define _PG_ATTR_PRESENT (1<<16)
#define _PG_ATTR_USER (1<<17)
#define _PG_ATTR_RW (1<<18)
#define _PG_ATTR_CACHED (1<<19)
//#define _PG_ATTR_PARTITION (1<<20)

#ifdef CONFIG_MMU
extern void SetupDefaultVmMap(void);
extern void SetupVmMap(xmAddress_t *stFrameArea, xm_s32_t *noFrames);
#elif CONFIG_MPU
extern void SetupVmMap(void);
#endif

extern xm_u32_t SetupPageTable(struct xmcPartition *cfg, xmAddress_t pct, xmWord_t ** ptdL1S_p, xmWord_t ** ptdL1NS_p) __WARN_UNUSED_RESULT;
extern void SetupPtdL1(xmWord_t *ptdL1NS, struct xmcPartition *cfg);
inline xmAddress_t MapAddr(xmWord_t *ptdL1, xmAddress_t vAddr, xmAddress_t pAddr, xmSize_t size, xmWord_t attr);
inline xmAddress_t VmMapUser(xmWord_t *ptdL1, xmAddress_t pAddr, xmAddress_t vAddr, xmSize_t size, xm_u32_t flags);

extern xm_u32_t VmArchAttr2Attr(xm_u32_t entry);
extern xm_u32_t VmAttr2ArchAttr(xm_u32_t entry);

#ifdef CONFIG_ARM
extern xmWord_t MapPartitionControlTable(xmAddress_t * _partitionThread);
//extern xm_s32_t VmMapCheckAddressAccess(xmAddress_t Addr, xmWord_t R0_W1, xmWord_t securityLevel);
extern xm_s32_t VmMapCheckAddressReadAccess(xmAddress_t Addr, xmWord_t securityLevel);
extern xm_s32_t VmMapCheckAddressWriteAccess(xmAddress_t Addr, xmWord_t securityLevel);
extern xm_s32_t VmMapCheckMemAreaAccess(xmAddress_t memAreaAddress, xmSize_t size, xmWord_t R0_W1, xmWord_t securityLevel);

extern void InitMMU(void);
/*extern xmWord_t UnMapPartitionControlTable(xmAddress_t pct);*/
#endif

#define ROUNDUP(addr, _pS) ((((~(addr))+1)&((_pS)-1))+(addr))
#define ROUNDDOWN(addr, _pS) ((addr)&~((_pS)-1))
#define SIZE2PAGES(size) \
    (((((~(size))+1)&(PAGE_SIZE-1))+(size))>>PAGE_SHIFT)

#endif
