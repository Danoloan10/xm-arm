/*
 * $FILE: vmmap.c
 *
 * Virtual memory map management
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
 * - [23/11/2015:SPR-151123-01:#94] Map XM in a 16MB area.
 * - [25/11/2015:SPR-151125-01:#96] Devices area starts in address 0x40000000.
 * - [09/02/16:XM-ARM-2:SPR-151125-01:#97] Map TZ registers that allow PL non-secure access during XN setup.
 * - [10/12/2015:XM-ARM-2:SPR-160121-02:#104] Add a pragma to avoid an unexpected overflow warning.
 * - [15/02/16:XM-ARM-2:SPR-160215-01:#109] XM nonSecure areas are deprecated.
 * - [15/02/16:XM-ARM-2:SPR-160215-02:#110] Map XM code area in 16MB area.
 * - [15/02/16:XM-ARM-2:SPR-160215-02:#110] Map XM data area in an area multiple of 16MB depending on xmc size.
 * - [24/08/16:XM-ARM-2:SPR-160819-02:#118] Map OCM in HIGH addresses with proper attributes.
 * - [08/09/16:XM-ARM-2:SPR-160908-01:#123] Allow access to entire Memory Map. Clean Code.
 * - [13/09/16:XM-ARM-2:SPR-160913-01:#125] Avoid PTL2 Remapping when PTL1 is present.
 * - [21/10/16:XM-ARM-2:SPR-161013-01:#126] Reserve a LargePage (1Mb)  for PCT, at the end of XM code map.
 * - [21/11/16:XM-ARM-2:SPR-161121-01:#130] XM NonSecure sections mapped in a PAGE (4kB).
 * - [31/01/17:XM-ARM-2:CP-161019-01:#143] Provide space for the default memory Map PTDL1.
 */

#include <assert.h>
#include <boot.h>
#include <rsvmem.h>
#include <kthread.h>
#include <stdc.h>
#include <vmmap.h>
#include <virtmm.h>
#include <physmm.h>
#include <xmconf.h>
#include <arch/xm_def.h>
#include <arch/processor.h>
#include <arch/paging.h>
#include <arch/physmm.h>
#include <arch/gic.h>
#include <drivers/zynqboard/zynq_mmu.h>
#include <drivers/zynqboard/zynqboard.h>
#ifdef CONFIG_ENABLE_CACHE
#include <drivers/zynqboard/zynq_cache.h>
#endif

#define DEVICE_BASE_ADDRESS XM_VMMAP_PL_START
#define DEVICE_TOP_ADDRESS  XM_VMMAP_OCM_HIGH_START-1

xmAddress_t VmMapFindPTL2FromForAddress(xmAddress_t ptdL1, xmAddress_t vAddr);
xmWord_t **ptdL1S_list;

static inline xmWord_t MapLpt1Entry(xmAddress_t pAddr, xmWord_t attr)
{
	xmWord_t entry = _PG_ARCH_PTE_PRESENT;
	entry |= (pAddr & LPAGE_MASK);
	entry |= _PG_ARCH_ACCESSPERMISSION_2_LPT1((attr & _PG_ARCH_ACCESSPERMISSION_MASK));
	entry |= (((attr&_PG_ARCH_SECURE_MASK)==_PG_ARCH_NONSECURE)?MMU_L1PT_TLB_SECTION_NONSECURE:MMU_L1PT_TLB_SECTION_SECURE);
	entry |= (((attr&_PG_ARCH_SHAREABLE_MASK)==_PG_ARCH_SHAREABLE)?MMU_L1PT_TLB_SECTION_SHAREABLE:MMU_L1PT_TLB_SECTION_NONSHAREABLE);
	entry |= (((attr&_PG_ARCH_EXECUTIONNEVER_MASK)==_PG_ARCH_EXECUTIONNEVER_MASK)?MMU_L1PT_TLB_SECTION_EXECUTIONNEVER:MMU_L1PT_TLB_SECTION_EXECUTIONALLOWED);
	entry |= _PG_ARCH_MEMORYTYPE_2_LPT1((attr & _PG_ARCH_MEMORYTYPE_MASK));
#if 0
	eprintf("\t\t\t\t\tLP1 ENTRY: 0x%x 0x%x\n", entry&LPAGE_MASK, entry&(~LPAGE_MASK));
#endif
	return entry;
}

static inline xmWord_t MapLpt1Lpt2Descriptor(xmAddress_t pAddr, xmWord_t attr)
{
	xmWord_t entry = _PG_ARCH_PTD_PRESENT;
	entry |= (pAddr & PTDL1L2_MASK);
	entry |= (((attr & _PG_ARCH_SECURE_MASK) == _PG_ARCH_NONSECURE) ? MMU_L2PT_TLB_PTENTRY_NONSECURE : MMU_L2PT_TLB_PTENTRY_SECURE);
#if 0
	eprintf("\t\t\t\t\tLP1 2 LP2 Desc: 0x%x 0x%x\n", entry&PTDL1L2_MASK, entry&(~PTDL1L2_MASK));
#endif
	return entry;
}

static inline xmWord_t MapLpt2Entry(xmAddress_t pAddr, xmWord_t attr)
{
	xmWord_t entry = _PG_ARCH_PTE_PRESENT;
	entry |= (pAddr & PAGE_MASK);
	entry |= _PG_ARCH_ACCESSPERMISSION_2_LPT2(attr & _PG_ARCH_ACCESSPERMISSION_MASK);
	entry |= (((attr&_PG_ARCH_SHAREABLE_MASK)==_PG_ARCH_SHAREABLE)?MMU_L2PT_TLB_SECTION_SHAREABLE:MMU_L2PT_TLB_SECTION_NONSHAREABLE);
	entry |= (((attr&_PG_ARCH_EXECUTIONNEVER_MASK)==_PG_ARCH_EXECUTIONNEVER_MASK)?MMU_L2PT_TLB_SECTION_EXECUTIONNEVER:MMU_L2PT_TLB_SECTION_EXECUTIONALLOWED);
	entry |= _PG_ARCH_MEMORYTYPE_2_LPT2((attr & _PG_ARCH_MEMORYTYPE_MASK));
#if 0
	eprintf("\t\t\t\t\tLP2 ENTRY: 0x%x 0x%x\n", entry&PAGE_MASK, entry&(~PAGE_MASK));
#endif
	return entry;
}

static xmAddress_t __MapAddr(xmWord_t *ptdL1, xmAddress_t vAddr, xmAddress_t pAddr_, xmSize_t size, xmWord_t attr, int mapOwnPtdL2)
{
	xmAddress_t addr;
	xmWord_t *ptdL2;
	xm_s32_t l1e, l2e;
/*#define COUNTLOG*/
#ifdef COUNTLOG
	xm_s32_t lp_c = 0, ptd_c = 0, p_c = 0;
#endif
	xmAddress_t pAddr = pAddr_;

	for (addr = vAddr; (addr >= vAddr) && (addr < (vAddr + size -1));)
	{
		l1e = VA2PtdL1(addr);
		l2e = VA2PtdL2(addr);
		/*L1*/
		switch ((ptdL1[l1e] & (_PG_ARCH_PTE_PRESENT | _PG_ARCH_PTD_PRESENT)))
		{
		case 0:
			if ((!l2e) && (((addr + (LPAGE_SIZE)) <= (vAddr + size)))
					&& (!(pAddr & ((LPAGE_SIZE) - 1))))
			{
				ptdL1[l1e] = MapLpt1Entry(pAddr, attr);
#ifdef COUNTLOG
				lp_c++;
#endif
				pAddr += LPAGE_SIZE;
				addr += LPAGE_SIZE;
				continue;
			}
			GET_MEMAZ(ptdL2, PTDL2SIZE, PTDL2ALIGN);
#ifdef COUNTLOG
			ptd_c++;
#endif
			ptdL1[l1e] = MapLpt1Lpt2Descriptor(_VIRT2PHYS(ptdL2), attr);
			if (mapOwnPtdL2) {
#if 0
	eprintf("\t\t\t\t\tMap Own LP2: 0x%x\n", _VIRT2PHYS(ptdL2));
#endif
				__MapAddr(ptdL1, _VIRT2PHYS(ptdL2),_VIRT2PHYS(ptdL2), PTDL2SIZE, attr, PTDL2_NONMAP);
			}

			break;
        case _PG_ARCH_PTE_PRESENT:
            pAddr = (pAddr&LPAGE_MASK) + LPAGE_SIZE;
            addr += (addr&LPAGE_MASK) + LPAGE_SIZE;
            continue;
            break;
		case _PG_ARCH_PTD_PRESENT:
			ptdL2 = (xmWord_t *) _PHYS2VIRT(ptdL1[l1e] & PTDL1L2_MASK);
			break;
		default:
			eprintf("\t\t[MMAP]: ERROR L1PTE in use 0x%x 0x%x when mapping 0x%x\n\t\t[MMAP]ASSERT\n", &(ptdL1[l1e]), ptdL1[l1e], pAddr);
			ptdL1[l1e] = 0;
			ASSERT(0);
			return -1;
			break;
		}
		/*L2*/
		switch ((ptdL2[l2e] & (_PG_ARCH_PTE_PRESENT)))
		{
		case 0:
		default:
			ptdL2[l2e] = MapLpt2Entry(pAddr, attr);

#ifdef COUNTLOG
			p_c++;
#endif
			pAddr += PAGE_SIZE;
			addr += PAGE_SIZE;
			break;
		}
	}
#ifdef COUNTLOG
	eprintf("\t\t[MMAP]: [0x%08x] L1E %d | ptd %d | L2E %d\n", ptdL1, lp_c, ptd_c, p_c);
#endif
#if 0
	eprintf("\t\t[MMAP]: 0x%08x->[0x%08x{0x%08x}-0x%08x] [0x%08x]\n", ptdL1, pAddr_, vAddr, ((xm_u32_t) (pAddr)) - 1, attr);
#endif
	return addr;
}
#if 0 /*Unused function*/
static inline xmAddress_t __UnMapAddr(xmWord_t *ptdL1, xmAddress_t vAddr, xmAddress_t pAddr, xmSize_t size, xmWord_t attr)
{
	xmAddress_t addr;
	xmWord_t *ptdL2=0;
	xm_s32_t l1e, l2e;

	for (addr = vAddr; (addr >= vAddr) && addr < (vAddr + size);)
	{
		l1e = VA2PtdL1(addr);
		l2e = VA2PtdL2(addr);
		switch ((ptdL1[l1e] & (_PG_ARCH_PTE_PRESENT | _PG_ARCH_PTD_PRESENT)))
		{
		case 0:
			pAddr += LPAGE_SIZE;
			addr += LPAGE_SIZE;
			continue;
			break;
		case _PG_ARCH_PTD_PRESENT:
			break;
		case _PG_ARCH_PTE_PRESENT:
			ptdL1[l1e] = 0;
			pAddr += LPAGE_SIZE;
			addr += LPAGE_SIZE;
			break;
		default:
			ASSERT(0);
			return -1;
			break;
		}
		ptdL2[l2e] = 0;
		pAddr += PAGE_SIZE;
		addr += PAGE_SIZE;
	}
	return addr;
}
#endif
xmAddress_t MapAddr(xmWord_t *ptdL1, xmAddress_t vAddr, xmAddress_t pAddr, xmSize_t size, xmWord_t attr)
{
    return __MapAddr(ptdL1, vAddr, pAddr, size, (attr & (~_PG_ARCH_SECURE_MASK)), PTDL2_NONMAP);
}
xmAddress_t VmMapUser(xmWord_t *ptdL1, xmAddress_t pAddr, xmAddress_t vAddr, xmSize_t size, xm_u32_t flags)
{
    return __MapAddr(ptdL1, vAddr, pAddr, size, (VmAttr2ArchAttr(flags) | _PG_ARCH_NONSECURE), PTDL2_MAP);
}
xmAddress_t VmMapUserArchAttr(xmWord_t *ptdL1, xmAddress_t pAddr, xmAddress_t vAddr, xmSize_t size, xm_u32_t flags, int mapOwnPtdL2)
{
    return __MapAddr(ptdL1, vAddr, pAddr, size, (flags | _PG_ARCH_NONSECURE), mapOwnPtdL2);
}

#define SIZE_MULTIPLEOF(S,M)	((S&(~(M-1)))+((S&(M-1))?(M):(0)))

void __VBOOT SetupDefaultVmMap()
{
	xmWord_t attr, dev_attr;
	extern xmWord_t _defaultPtdL1S, _defaultPtdL1NS;
	xmWord_t * dPtdL1S  = ((xmWord_t *)(&(_defaultPtdL1S)));
	xmWord_t * dPtdL1NS = ((xmWord_t *)(&(_defaultPtdL1NS)));

	memset(dPtdL1S, 0, PTDL1SIZE);
	memset(dPtdL1NS, 0, PTDL1SIZE);

	attr = _PG_ARCH_DCACHEABLE | _PG_ARCH_ACCESSPERMISSION_RW_RW;

	/* 0x00000000 - 0x000fffff (cacheable)*/
    MapAddr(dPtdL1S, ((xmAddress_t) (XM_VMMAP_OCM_LOW_START)),
    		((xmAddress_t) (XM_VMMAP_OCM_LOW_START)), XM_VMMAP_DDR_START, attr);
    VmMapUserArchAttr(dPtdL1NS, ((xmAddress_t) (XM_VMMAP_OCM_LOW_START)),
    		((xmAddress_t) (XM_VMMAP_OCM_LOW_START)), XM_VMMAP_DDR_START, attr, PTDL2_NONMAP);

    /* 0x00100000 - 0x3fffffff (DDR Cacheable) */
    MapAddr(dPtdL1S, ((xmAddress_t) (XM_VMMAP_DDR_START)),
    		((xmAddress_t) (XM_VMMAP_DDR_START)), XM_VMMAP_DDR_SIZE, (attr | _PG_ARCH_SHAREABLE));
    VmMapUserArchAttr(dPtdL1NS, ((xmAddress_t) (XM_VMMAP_DDR_START)),
    		((xmAddress_t) (XM_VMMAP_DDR_START)), XM_VMMAP_DDR_SIZE, attr, PTDL2_NONMAP);


	dev_attr = _PG_ARCH_MEMORYTYPE_SHAREABLE_DEVICE | _PG_ARCH_ACCESSPERMISSION_RW_RW;

    /* 0x40000000 - 0x7fffffff (FPGA slave0) */
    /* 0x80000000 - 0xbfffffff (FPGA slave1) */
    MapAddr(dPtdL1S, ((xmAddress_t) (XM_VMMAP_PL_START)),
        ((xmAddress_t) (XM_VMMAP_PL_START)), XM_VMMAP_PL_SIZE, dev_attr);
    VmMapUserArchAttr(dPtdL1NS, ((xmAddress_t) (XM_VMMAP_PL_START)),
        ((xmAddress_t) (XM_VMMAP_PL_START)), XM_VMMAP_PL_SIZE, dev_attr, PTDL2_NONMAP);

    /* 0xe0000000 - 0xe02fffff (Memory mapped devices)
    * UART/USB/IIC/SPI/CAN/GEM/GPIO/QSPI/SD/NAND */
    MapAddr(dPtdL1S, ((xmAddress_t) (XM_VMMAP_IOP_START)),
        ((xmAddress_t) (XM_VMMAP_IOP_START)), XM_VMMAP_IOP_SIZE, dev_attr);
    VmMapUserArchAttr(dPtdL1NS, ((xmAddress_t) (XM_VMMAP_IOP_START)),
        ((xmAddress_t) (XM_VMMAP_IOP_START)), XM_VMMAP_IOP_SIZE, dev_attr, PTDL2_NONMAP);

    /* 0xf8000000 - 0xf8ffffff (AMBA APB Peripherals) */
    MapAddr(dPtdL1S, ((xmAddress_t) (XM_VMMAP_SLCR_START)),
    		((xmAddress_t) (XM_VMMAP_SLCR_START)), (0x1000000), dev_attr);
    VmMapUserArchAttr(dPtdL1NS, ((xmAddress_t) (XM_VMMAP_SLCR_START)),
    		((xmAddress_t) (XM_VMMAP_SLCR_START)), (0x1000000), dev_attr, PTDL2_NONMAP);

#if 0
	eprintf("XM SEC PAGE TABLE\n");
	printTLB(dPtdL1S);
#endif
}

void InitMMU()
{
	extern xmWord_t _defaultPtdL1S, _defaultPtdL1NS;
	xmWord_t * dPtdL1S  = ((xmWord_t *)(&(_defaultPtdL1S)));
	xmWord_t * dPtdL1NS = ((xmWord_t *)(&(_defaultPtdL1NS)));

    CA9_SCR_SECURE();
    WR_TTBR0(dPtdL1S);
    WR_TTBCR(0);
    EnableMMU();
    CA9_SCR_NONSECURE();
    WR_TTBR0(dPtdL1NS);
    WR_TTBCR(0);
    EnableMMU();
    CA9_SCR_SECURE();
}

void __VBOOT SetupVmMap(xmAddress_t *stFrameArea, xm_s32_t *noFrames)
{
	xmAddress_t end, vAddr, pAddr;
	xm_u32_t size;

	xmWord_t attr, dev_attr;
	xm_s32_t flags, cnt;
	xm_s32_t rtrn __attribute__((unused));
	xmWord_t *ptdL1S;

	GET_MEMAZ(ptdL1S_list, xmcTab.noPartitions*sizeof(xmWord_t*), ALIGNMENT);

	for(cnt=0;cnt<xmcTab.noPartitions;cnt++)
	{
		GET_MEMAZ(ptdL1S_list[cnt], PTDL1SIZE, PTDL1ALIGN);
	}
	ptdL1S = ptdL1S_list[0];

	pAddr = xmcPhysMemAreaTab[xmcTab.hpv.physicalMemoryAreasOffset].startAddr;
	end = pAddr + xmcPhysMemAreaTab[xmcTab.hpv.physicalMemoryAreasOffset].size - 1;
	flags = xmcPhysMemAreaTab[xmcTab.hpv.physicalMemoryAreasOffset].flags;

	*stFrameArea = ROUNDUP(_PHYS2VIRT(end+1), LPAGE_SIZE);
	*noFrames = ((XM_VMAPEND - *stFrameArea) + 1) / PAGE_SIZE;

	vAddr = xmcPhysMemAreaTab[xmcTab.hpv.physicalMemoryAreasOffset].mappedAt;

	/* XM Code */
	ptdL1S[VA2PtdL1(vAddr)] = 0;

	for(cnt=0; cnt<xmcTab.hpv.noPhysicalMemoryAreas; cnt++)
	{
		unsigned int memAreaIndx = xmcTab.hpv.physicalMemoryAreasOffset + cnt;
		pAddr = xmcPhysMemAreaTab[memAreaIndx].startAddr;
		vAddr = xmcPhysMemAreaTab[memAreaIndx].mappedAt;
		flags = xmcPhysMemAreaTab[memAreaIndx].flags;

		if (!(flags & XM_MEM_AREA_UNCACHEABLE))
		{
			if (pAddr==CONFIG_XM_LOAD_ADDR)
				attr = _PG_ARCH_ICACHEABLE;
			else
				attr = _PG_ARCH_DCACHEABLE;
		}
		else
        { attr = _PG_ARCH_NONCACHEABLE; }

		switch(pAddr)
		{
		case CONFIG_XM_LOAD_ADDR :
			size = 16*1024*1024-XM_PCTRLTAB_RSV_SIZE;
            attr |= _PG_ARCH_ACCESSPERMISSION_RO_RO;
			break;
		case CONFIG_XM_LOAD_DATA_ADDR :
			size = SIZE_MULTIPLEOF(xmcPhysMemAreaTab[memAreaIndx].size, 16*1024*1024);
			attr |= _PG_ARCH_SHAREABLE;
    		attr |= _PG_ARCH_EXECUTIONNEVER_MASK;
    		attr |= _PG_ARCH_ACCESSPERMISSION_RW_RW;
			break;
		default :
			size = xmcPhysMemAreaTab[memAreaIndx].size;
    		attr |= _PG_ARCH_ACCESSPERMISSION_RW_RW;
			break;
		}
		MapAddr(ptdL1S, vAddr, pAddr, size, attr);
	}

	dev_attr = _PG_ARCH_MEMORYTYPE_SHAREABLE_DEVICE | _PG_ARCH_ACCESSPERMISSION_RW_RW;

    MapAddr(ptdL1S, ((xmAddress_t) (XM_VMMAP_PL_START)),
        ((xmAddress_t) (XM_VMMAP_PL_START)), XM_VMMAP_PL_SIZE, dev_attr);
    MapAddr(ptdL1S, ((xmAddress_t) (XM_VMMAP_IOP_START)),
        ((xmAddress_t) (XM_VMMAP_IOP_START)), (XM_VMMAP_QSPI_START+XM_VMMAP_QSPI_SIZE)-XM_VMMAP_IOP_START, dev_attr);

#if 0
	eprintf("XM SEC PAGE TABLE\n");
	printTLB(ptdL1S);
#endif
}

void SetupPtdL1(xmWord_t *ptdL1NS, struct xmcPartition *cfg)
{
	xmAddress_t /*end, */vAddr, pAddr;
	xm_u32_t size;
	xmWord_t attr, dev_attr;
	xm_s32_t flags;
	xm_s32_t rtrn __attribute__((unused));

    unsigned int cnt;
	for(cnt=0; cnt<xmcTab.hpv.noPhysicalMemoryAreas; cnt++)
	{
		unsigned int memAreaIndx = xmcTab.hpv.physicalMemoryAreasOffset + cnt;
		pAddr = xmcPhysMemAreaTab[memAreaIndx].startAddr;
		vAddr = xmcPhysMemAreaTab[memAreaIndx].mappedAt;
		flags = xmcPhysMemAreaTab[memAreaIndx].flags;

		if (!(flags & XM_MEM_AREA_UNCACHEABLE))
		{ attr = _PG_ARCH_USER_CACHEABLE; }
		else { attr = _PG_ARCH_NONCACHEABLE; }

		extern xmAddress_t _nonSecExceptions;
		switch(pAddr)
		{
		case CONFIG_XM_LOAD_ADDR :
            /*Only the NonSecure memory area shall be included in partition MMap.*/
            pAddr = ((xmAddress_t)(&_nonSecExceptions));
            vAddr = ((xmAddress_t)(&_nonSecExceptions));
            size = PAGE_SIZE;
            attr |= _PG_ARCH_ACCESSPERMISSION_RO_RO;
            break;
		case CONFIG_XM_LOAD_DATA_ADDR :
		    /*XM Data section shall not be included in partition MMap.*/
		    continue;
			break;
		default :
			size = xmcPhysMemAreaTab[memAreaIndx].size;
			attr |= _PG_ARCH_ACCESSPERMISSION_RW_RW;
			break;
		}

		rtrn = VmMapUserArchAttr(ptdL1NS, pAddr, vAddr, size, attr, PTDL2_MAP);
	}


	attr = _PG_ARCH_USER_CACHEABLE | _PG_ARCH_ACCESSPERMISSION_RW_RW;
	dev_attr = _PG_ARCH_MEMORYTYPE_SHAREABLE_DEVICE | _PG_ARCH_ACCESSPERMISSION_RW_RW;

    /* 2 CONTINUOUS PAGEs GIC */
    rtrn = VmMapUserArchAttr(ptdL1NS, ((xmAddress_t) (ICC_BASE_ADDRESS)), ((xmAddress_t) (ICC_BASE_ADDRESS)), PAGE_SIZE, dev_attr, PTDL2_MAP);
    rtrn = VmMapUserArchAttr(ptdL1NS, ((xmAddress_t) (ICD_BASE_ADDRESS)), ((xmAddress_t) (ICD_BASE_ADDRESS)), PAGE_SIZE, dev_attr, PTDL2_MAP);

    /* Non-secure stack */
	extern xm_u32_t _xm_nonsec_stack;
	xm_u32_t nsecStack = ((xm_u32_t)(&_xm_nonsec_stack));
    rtrn = VmMapUserArchAttr(ptdL1NS, ((xmAddress_t) (nsecStack))&(PAGE_MASK),((xmAddress_t) (nsecStack))&(PAGE_MASK), PAGE_SIZE, attr, PTDL2_MAP);

    /* Own NS PtL1 */
    rtrn = VmMapUserArchAttr(ptdL1NS, ((xmAddress_t) (ptdL1NS)),((xmAddress_t) (ptdL1NS)), PTDL1SIZE, attr, PTDL2_MAP);
}

xm_u32_t VmArchAttr2Attr(xm_u32_t entry)
{
	xm_u32_t flags = entry & (PAGE_SIZE - 1), attr = 0;

	if (flags & _PG_ARCH_PTE_PRESENT)
		attr |= _PG_ATTR_PRESENT;
	if ((flags & _PG_ARCH_ACCESSPERMISSION_RW_RW) | (flags & _PG_ARCH_ACCESSPERMISSION_RO_RO))
		attr |= _PG_ATTR_USER;
	if ((flags & _PG_ARCH_ACCESSPERMISSION_RW_RW))
		attr |= _PG_ATTR_RW;
	if (flags & _PG_ARCH_USER_CACHEABLE)
		attr |= _PG_ATTR_CACHED;

	return attr | (flags & ~0x9f);
}

xm_u32_t VmAttr2ArchAttr(xm_u32_t flags)
{
	xm_u32_t attr = 0;

	if (flags & _PG_ATTR_PRESENT)
		attr |= _PG_ARCH_PTE_PRESENT;
	if (flags & _PG_ATTR_USER)
	{
		if (flags & _PG_ATTR_RW)
			attr |= _PG_ARCH_ACCESSPERMISSION_RW_RW;
		else
			attr |= _PG_ARCH_ACCESSPERMISSION_RO_RO;
	}
	else
	{
		if (flags & _PG_ATTR_RW)
			attr |= _PG_ARCH_ACCESSPERMISSION_RW_RW;
		else
			attr |= _PG_ARCH_ACCESSPERMISSION_RO_RO;
	}

	if (flags & _PG_ATTR_CACHED)
		attr |= _PG_ARCH_USER_CACHEABLE;
	else
		attr |= _PG_ARCH_NONCACHEABLE;

	return attr | (flags & 0xffff);
}

static inline xm_s32_t VmMapCheckAddressAccess(xmAddress_t Addr, xmWord_t R0_W1, xmWord_t securityLevel)
{
	xm_u32_t res;
	if(!R0_W1)
		CA9_VMT_WR_ATS1CPR(Addr);		/* 32-bit WO Stage 1 Current state PL1 read */
	else
		CA9_VMT_WR_ATS1CPW(Addr);		/* 32-bit WO Stage 1 Current state PL1 write */
	CA9_VMT_RD_PAR(res);			/* 32-bit RW Physical Address Register */

#if 0 /* 1=PRINT CHECK RESULTS */
	char aux[20];
	if(((res&0xfffff000)==(Addr&0xfffff000)))
		sprintf(aux,"%s","EQUAL");
	else
		sprintf(aux,"0x%x",(res&0xfffff000));
	kprintf("Check %s result 0x%x %s 0x%x %s%c %s\n",
			(R0_W1?"wr":"rd"),
			Addr,
			aux,
			(res&0x00000fff),
			((res&BIT(9))?"NS":"S"),
			((((res&BIT(9))&&1)!=securityLevel)?'!':' '),
			((res&BIT(0))?"FAIL":"OK"));
#endif
    if(((res&0xfffff000) < XM_VMMAP_OCM_HIGH_START))
	{
        if(((res&BIT(9))&&1)!=securityLevel)
        {   return -1;   }
        if((res & BIT(0)))
        {   return -2;   }
    }
	return 0;
}
xm_s32_t VmMapCheckAddressReadAccess(xmAddress_t Addr, xmWord_t securityLevel)
{	return VmMapCheckAddressAccess(Addr,0,securityLevel); }
xm_s32_t VmMapCheckAddressWriteAccess(xmAddress_t Addr, xmWord_t securityLevel)
{	return VmMapCheckAddressAccess(Addr,1,securityLevel); }

inline xm_s32_t VmMapCheckMemAreaAccess(xmAddress_t memAreaAddress, xmSize_t size, xmWord_t R0_W1, xmWord_t securityLevel)
{
     xmAddress_t addr;
     for (addr=memAreaAddress; addr<memAreaAddress+size; addr=(addr&PAGE_MASK)+PAGE_SIZE)
     {
         if(VmMapCheckAddressAccess(addr, R0_W1, securityLevel))
         {   return -1; }
     }
     return 0;
}


xmAddress_t VmMapFindPTL2FromForAddress(xmAddress_t ptdL1, xmAddress_t vAddr)
{
	xmAddress_t rtrn = ((xmAddress_t)(-1));
	xm_u32_t l1e = VA2PtdL1(vAddr);
	xm_u32_t cL1Entry = ((xm_u32_t *) (ptdL1))[l1e];

	switch ((cL1Entry & (_PG_ARCH_PTE_PRESENT | _PG_ARCH_PTD_PRESENT)))
	{
	case _PG_ARCH_PTD_PRESENT:
		rtrn = ((xmAddress_t)(cL1Entry&PTDL1L2_MASK));
		break;
	case _PG_ARCH_PTE_PRESENT:
		rtrn = ((xmAddress_t)(-2));
		break;
	}
	return rtrn;
}


#if 0

void printPTD2(xmAddress_t tlb, xmAddress_t base)
{
	xm_u32_t * _tlb = ((xm_u32_t *) (tlb));
	xm_u32_t * _end = ((xm_u32_t *) (tlb+1024));
	xm_u32_t a ;

	for (; _tlb < _end; _tlb++)
	{
		a = ((xm_s32_t)(((xm_u32_t)(_tlb))-((xm_u32_t)(tlb))))<<10;
		a += base;
		switch (((*_tlb) & (_PG_ARCH_PTE_PRESENT)))
		{
		case _PG_ARCH_PTE_PRESENT:

			if((a>=0x10000000)&&(a<0x40000000))
			eprintf("\tPTE[0x%x] V=0x%x -> PH=0x%x Attr=0x%x\n", _tlb, a, (*_tlb)&PAGE_MASK, (*_tlb)&(~PAGE_MASK));
			break;
		case 0:
			break;
		default:
			eprintf("\tNOT SPECTED ENTRY 0x%x\n",*_tlb);
			break;
		}
	}
}

void printTLB(xmAddress_t tlb)
{
	xm_u32_t * _tlb = ((xm_u32_t *) (tlb));
	xm_u32_t * _end = _tlb + (0x400*4);
	xm_u32_t a ;
	eprintf("TLB: 0x%x:0x%x\n", _tlb, _end);

	for (; _tlb <= _end; _tlb++)
	{
		a = ((xm_s32_t)(((xm_u32_t)(_tlb))-((xm_u32_t)(tlb))))<<18;
		switch (((*_tlb) & (_PG_ARCH_PTE_PRESENT | _PG_ARCH_PTD_PRESENT)))
		{
		case _PG_ARCH_PTD_PRESENT:
				eprintf("PTD[0x%x] V=0x%x -> PtdL2[0x%x] Attr=0x%x\n", _tlb, a, (*_tlb)&PTDL1L2_MASK, (*_tlb)&(~PTDL1L2_MASK));
				printPTD2(((*_tlb)&PTDL1L2_MASK), a);
			break;
		case _PG_ARCH_PTE_PRESENT:
			eprintf("PTE[0x%x] V=0x%x -> PH=0x%x Attr=0x%x\n", _tlb, a, (*_tlb)&LPAGE_MASK, (*_tlb)&(~LPAGE_MASK));
			break;
		case 0:
			break;
		default:
			eprintf("NOT SPECTED ENTRY 0x%x\n",*_tlb);
			break;
		}
	}
}
#endif
