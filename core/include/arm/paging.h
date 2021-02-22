/*
 * $FILE: paging.h
 *
 * ARM version
 *
 * $VERSION$
 *
 * Author: Miguel Masmano <mmasmano@fentiss.com>
 * Author: Manuel Muñoz   <mmunoz@ai2.upv.es>
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license terms.
 */
/*
 * Changelog:
 * - [08/09/16:XM-ARM-2:SPR-160908-01:#123] Add documentation track. Clean Code.
 * - [21/10/16:XM-ARM-2:SPR-161013-01:#126] Reserve a LargePage (1Mb) for PCT.
 * - [21/10/16:XM-ARM-2:SPR-161013-02:#128] Move XM_PCTRLTAB_VADDR definition to virtmm.h.
 */

#ifndef __XM_ARM_PAGING_H__
#define __XM_ARM_PAGING_H__

//#define PTD_LEVELS 2

//@ \void{<track id="paging-page-size">}
#define PAGE_SIZE  (4*1024)      /*4kB page*/
#define LPAGE_SIZE (1*1024*1024) /*1MB section*/
//@ \void{</track id="paging-page-size">}



#define PAGE_MASK	(~(PAGE_SIZE-1))
#define LPAGE_MASK (~(LPAGE_SIZE-1))

#define PAGE_SHIFT	12
#define LPAGE_SHIFT 20

#define PTDL1_SHIFT 20
#define PTDL2_SHIFT 12

#define PTDL1L2_MASK	(~(0x400-1))
#define PTDL1L2_SHIFT	10


#define PTDL1SIZE ((4*1024)*sizeof(xmWord_t))
#define PTDL2SIZE (((LPAGE_SIZE)/(PAGE_SIZE))*sizeof(xmWord_t))

#define PTDL1ALIGN	PTDL1SIZE
#define PTDL2ALIGN	PTDL2SIZE

#define PTDL2_MAP 		1
#define PTDL2_NONMAP	0

#ifdef _XM_KERNEL_

#ifndef __ASSEMBLY__
/*TODO ARMPorting: Not implemented yet*/
//#define _VIRT2PHYS(x) ((xm_u32_t)(x))
//#define _PHYS2VIRT(x) ((xm_u32_t)(x))
#define _VIRT2PHYS(x) ((xm_u32_t)(x)-CONFIG_XM_OFFSET+CONFIG_XM_LOAD_ADDR)
#define _PHYS2VIRT(x) ((xm_u32_t)(x)+CONFIG_XM_OFFSET-CONFIG_XM_LOAD_ADDR)


#define VA2PtdL1(x) (((x)&0xfff00000)>>PTDL1_SHIFT)
#define VA2PtdL2(x) (((x)&0x000ff000)>>PTDL2_SHIFT)

#else /*__ASSEMBLY__*/
/*TODO ARMPorting: Not implemented yet*/
//#define _VIRT2PHYS(x) ((x)-CONFIG_XM_OFFSET+CONFIG_XM_LOAD_ADDR)
//#define _PHYS2VIRT(x) ((x)+CONFIG_XM_OFFSET-CONFIG_XM_LOAD_ADDR)
#endif /*__ASSEMBLY__*/

#define _PG_ARCH_PTE_PRESENT 0x00000002
#define _PG_ARCH_PTD_PRESENT 0x00000001

#define _PG_ARCH_ACCESSPERMISSION_MASK			(0x7)
#define _PG_ARCH_ACCESSPERMISSION_NA_NA			(0x0)
#define _PG_ARCH_ACCESSPERMISSION_RW_NA			(0x1)
#define _PG_ARCH_ACCESSPERMISSION_RW_RO			(0x2)
#define _PG_ARCH_ACCESSPERMISSION_RW_RW			(0x3)
#define _PG_ARCH_ACCESSPERMISSION_RO_NA			(0x5)
#define _PG_ARCH_ACCESSPERMISSION_RO_RO			(0x6)

#define _PG_ARCH_ACCESSPERMISSION_2_LPT1(AP)	((((AP)&(0x4))<<(15-2))|(((AP)&(0x3))<<(10)))
#define _PG_ARCH_ACCESSPERMISSION_2_LPT2(AP)	((((AP)&(0x4))<<(9-2))|(((AP)&(0x3))<<(4)))

#define _PG_ARCH_SECURE_MASK					(BIT(3))
#define _PG_ARCH_SECURE							(0)
#define _PG_ARCH_NONSECURE						(_PG_ARCH_SECURE_MASK)

#define _PG_ARCH_MEMORYTYPE_MASK					(0x1F0)
#define _PG_ARCH_MEMORYTYPE_STRONGLY_ORDERED		(0x000)
#define _PG_ARCH_MEMORYTYPE_SHAREABLE_DEVICE		(0x010)
#define _PG_ARCH_MEMORYTYPE_OUTERINNER_WT_NWA		(0x020)
#define _PG_ARCH_MEMORYTYPE_OUTERINNER_WB_NWA		(0x030)
#define _PG_ARCH_MEMORYTYPE_OUTERINNER_NONCACHEABLE	(0x040)
#define _PG_ARCH_MEMORYTYPE_OUTERINNER_WB_WA		(0x070)
#define _PG_ARCH_MEMORYTYPE_NONSHAREABLE_DEVICE		(0x0A0)
#define _PG_ARCH_MEMORYTYPE_CACHED(outerPolicy,innerPolicy)	((((0x1)<<(4))|(((outerPolicy)&(0x3))<<(2))|((innerPolicy)&(0x3)))<<4)	/* outer(L2) - inner(L1) */

#define _PG_ARCH_CACHEPOLICY_NONCACHEABLE		(0x0)
#define _PG_ARCH_CACHEPOLICY_WB_WA				(0x1)
#define _PG_ARCH_CACHEPOLICY_WT_NWA				(0x2)
#define _PG_ARCH_CACHEPOLICY_WB_NWA				(0x3)

#define _PG_ARCH_MEMORYTYPE_2_LPT1(MT)	((((MT>>6)&(0x7))<<(12))|(((MT>>4)&(0x3))<<(2)))
#define _PG_ARCH_MEMORYTYPE_2_LPT2(MT)	((((MT>>6)&(0x7))<<(6)) |(((MT>>4)&(0x3))<<(2)))

#define _PG_ARCH_SHAREABLE_MASK			(BIT(9))
#define _PG_ARCH_SHAREABLE				(_PG_ARCH_SHAREABLE_MASK)
#define _PG_ARCH_NONSHAREABLE			(0)

#define _PG_ARCH_EXECUTIONNEVER_MASK	(BIT(10))
#define _PG_ARCH_EXECUTIONNEVER			(_PG_ARCH_XN_MASK)
#define _PG_ARCH_EXECUTIONALLOWED		(0)


#ifdef CONFIG_MMU
#define XM_VMAPSTART CONFIG_XM_OFFSET
#define XM_VMAPEND 0xffffffff
#define XM_VMAPSIZE ((XM_VMAPEND-XM_VMAPSTART)+1)

#endif /*CONFIG_MMU*/

#endif /*_XM_KERNEL_*/

#endif /*__XM_ARM_PAGING_H__*/
