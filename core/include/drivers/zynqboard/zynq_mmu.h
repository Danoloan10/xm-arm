/*
 * $FILE: zynq_mmu.h
 *
 * board definitions
 *
 * $VERSION$ - 15/07/2014
 *
 * Author: Manuel Muñoz     <mmunoz@ai2.upv.es>
 * Author: Javier O.Coronel <jcoronel@fentiss.com>
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE file for the license terms.
 */

#ifndef _ZYNQ_MMU_H__
#define _ZYNQ_MMU_H__

#ifdef CONFIG_ZYNQ

#include "arch/asm.h"

/* MMU Common Macros and Definitions */
//L1PT Descriptors Types
#define MMU_L1PT_TLB_DESCRIPTOR_TYPE_MASK (0x00000003)
#define MMU_L1PT_TLB_DESCRIPTOR_INVALID_ENTRY			(0x0)			/* The associated MVA is unmapped, and attempting to access it generates a Translation fault */
#define MMU_L1PT_TLB_DESCRIPTOR_PAGETABLE_ENTRY			(0x1)			/* The descriptor gives the physical address of a second-level translation table. */
#define MMU_L1PT_TLB_DESCRIPTOR_SECTION_ENTRY			(0x2)			/* Section descriptor for the associated MVA. */
#define MMU_L1PT_TLB_DESCRIPTOR_SUPERSECTION_ENTRY		((0x2)|BIT(18))	/* Supersection descriptor for the associated MVA. */

//Memory Access Permissions
#define MMU_L1PT_TLB_ACCESSPERMISSION_NA_NA			(0x0)
#define MMU_L1PT_TLB_ACCESSPERMISSION_RW_NA			(0x1)
#define MMU_L1PT_TLB_ACCESSPERMISSION_RW_RO			(0x2)
#define MMU_L1PT_TLB_ACCESSPERMISSION_RW_RW			(0x3)
#define MMU_L1PT_TLB_ACCESSPERMISSION_RO_NA			(0x5)
#define MMU_L1PT_TLB_ACCESSPERMISSION_RO_RO			(0x6)

//Memory Attributes Encoding
#define MMU_L1PT_TLB_MEMORYATTR_NONCACHEABLE		(0x0)
#define MMU_L1PT_TLB_MEMORYATTR_CACHEABLE			(BIT(3))
#define MMU_L1PT_TLB_MEMORYATTR_NONBUFFERABLE		(0x0)
#define MMU_L1PT_TLB_MEMORYATTR_BUFFERABLE			(BIT(2))

/* Section & SuperSections Macros and Definitions */
#define MMU_L1PT_TLB_SECTION_AP_2AP(X)		((((X)&(0x4))<<(15-2))|(((X)&(0x3))<<(10)))

#define MMU_L1PT_TLB_SECTION_SECURE			(0x0)
#define MMU_L1PT_TLB_SECTION_NONSECURE			(BIT(19))

#define MMU_L1PT_TLB_SECTION_SUPERSECTION		(BIT(18))
#define MMU_L1PT_TLB_SECTION_SECTION			(0x0)

#define MMU_L1PT_TLB_SECTION_GLOBAL			(0x0)
#define MMU_L1PT_TLB_SECTION_PROCESSSPECIFIC	(BIT(17))

#define MMU_L1PT_TLB_SECTION_SHAREABLE			(BIT(16))
#define MMU_L1PT_TLB_SECTION_NONSHAREABLE		(0x0)

#define MMU_L1PT_TLB_SECTION_AP_2TEX(X)		((X<<12)&0x00007000)

#define MMU_L1PT_TLB_SECTION_AP_2DOMAIN(X)		((X<<5)&0x000001e0)

#define MMU_L1PT_TLB_SECTION_EXECUTIONNEVER		(BIT(4))
#define MMU_L1PT_TLB_SECTION_EXECUTIONALLOWED	(0x0)

#define MMU_L1PT_TLB_SECTION_BASEADDRESS_MASK	(0xfff00000)
#define MMU_L1PT_TLB_SECTION_BASE_ADDRESS(X)	(X & MMU_L1PT_TLB_SECTION_BASEADDRESS_MASK)

#define MMU_L1PT_TLB_SUPERSECTION_BASEADDRESS_MASK	(0xff000000)
#define MMU_L1PT_TLB_SUPERSECTION_BASE_ADDRESS(X)	(X & MMU_L1PT_TLB_SUPERSECTION_BASEADDRESS_MASK)



/*void MMU_L1PT_add_SectionSuperSection(
		xm_u32_t physicalBaseAddress,
		xm_u32_t virtualAddress,
		xm_u32_t secure,
		xm_u32_t nonGlobal,
		xm_u32_t shareable,
		xm_u32_t accessPermissions,
		xm_u32_t cacheable,
		xm_u32_t bufferable,
		xm_u32_t typeExtension,
		xm_u32_t domain,
		xm_u32_t executionNever,
		xm_u32_t section,
		xm_u32_t * _table);*/

#define MMU_L1PT_add16MB_SuperSection(physicalBaseAddress, virtualAddress, secure, nonGlobal, shareable, accessPermissions, cacheable, bufferable, typeExtension, domain, executionNever, _table) \
		MMU_L1PT_add_SectionSuperSection(physicalBaseAddress, virtualAddress, secure, nonGlobal, shareable, accessPermissions, cacheable, bufferable, typeExtension, domain, executionNever, MMU_L1PT_TLB_DESCRIPTOR_SUPERSECTION_ENTRY, _table)

#define MMU_L1PT_add1MB_Section(physicalBaseAddress, virtualAddress, secure, nonGlobal, shareable, accessPermissions, cacheable, bufferable, typeExtension, domain, executionNever, _table) \
		MMU_L1PT_add_SectionSuperSection(physicalBaseAddress, virtualAddress, secure, nonGlobal, shareable, accessPermissions, cacheable, bufferable, typeExtension, domain, executionNever, MMU_L1PT_TLB_DESCRIPTOR_SECTION_ENTRY, _table)

/* L2PT Entries */
#define MMU_L2PT_TLB_PTENTRY_SECURE				(0x0)
#define MMU_L2PT_TLB_PTENTRY_NONSECURE			(BIT(3))

#define MMU_L2PT_TLB_SECTION_SHAREABLE			(BIT(10))
#define MMU_L2PT_TLB_SECTION_NONSHAREABLE		(0x0)

#define MMU_L2PT_TLB_SECTION_EXECUTIONNEVER		(BIT(0))
#define MMU_L2PT_TLB_SECTION_EXECUTIONALLOWED	(0x0)

#define MMU_L2PT_TLB_PTENTRY_AP_2DOMAIN(X)		((X<<5)&0x000001e0)

#define MMU_L2PT_TLB_PTENTRY_BASEADDRESS_MASK	(0xfffffc00)
#define MMU_L2PT_TLB_PTENTRY_BASE_ADDRESS(X)	(X & MMU_L2PT_TLB_PTENTRY_BASEADDRESS_MASK)



//#define MMU_TTBCR_TTBDivision	(3)
//#define MMU_TTBCR_TTBDivisionAddres ((MMU_TTBCR_TTBDivision!=0)?BIT(32-MMU_TTBCR_TTBDivision):0xFFFFFFFF)

extern void EnableMMU(void);
extern void DisableMMU(void);
extern void NSFlushTlbGlobal(void);

#endif /*CONFIG_ZYNQ*/

#endif /*_ZYNQ_MMU_H__*/
