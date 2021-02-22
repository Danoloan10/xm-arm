/*
 * $FILE: kthread.c
 *
 * Kernel, Guest or L0 context (ARCH dependent part)
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
 * - [10/12/15:XM-ARM-2:SPR-150601-01:#53] Assign the user irqs depending on the current partition.
 * - [10/12/15:XM-ARM-2:SPR-150715-03:#73] Update cache management on Context Switch.
 */

#include <assert.h>
#include <gaccess.h>
#include <kthread.h>
#include <processor.h>
#include <sched.h>
#include <vmmap.h>
#include <stdc.h>
#include <arch/xm_def.h>

#include <vmmap.h>
#include <drivers/zynqboard/zynq_tzpc.h>
#include "drivers/zynqboard/zynq_mmu.h"
#ifdef CONFIG_ENABLE_CACHE
#include <drivers/zynqboard/zynq_cache.h>
#endif
#include "arm/gic.h"

#ifdef CONFIG_MPU
#define WPR1_REG 0x0
#define WPR2_REG 0x4
#define WPR_EN 0x80000000
#define WPR_BP 0x40000000
#define WPR_TAG_OFFS 15
#define WPR_MASK_OFFS 0
#define WPR_TAG_MASK 0x3FFF8000
#define WPR_MASK_MASK 0x7FFF
#define REG_SIZE (32*1024)
#define REG_OFF 15
#define REG_MASK 0x3FFF8000
#endif

#if 0
xm_s32_t irq2Vector[CONFIG_NO_HWIRQS]=
{
    0, // RESERVED
    INTERRUPT_LEVEL_1,
    INTERRUPT_LEVEL_2,
    INTERRUPT_LEVEL_3,
    INTERRUPT_LEVEL_4,
    INTERRUPT_LEVEL_5,
    INTERRUPT_LEVEL_6,
    INTERRUPT_LEVEL_7,
    INTERRUPT_LEVEL_8,
    INTERRUPT_LEVEL_9,
    INTERRUPT_LEVEL_10,
    INTERRUPT_LEVEL_11,
    INTERRUPT_LEVEL_12,
    INTERRUPT_LEVEL_13,
    INTERRUPT_LEVEL_14,
    INTERRUPT_LEVEL_15,
};
#endif

//#ifdef CONFIG_FLUSH_CACHE_AFTER_CS
//static kThread_t *cSPrevPart=0;
//#endif
extern xmAddress_t _nonSecExceptions;
void _clean_gic_pending(void);
extern xmWord_t * ptdL1S;
void SwitchKThreadArchPre(kThread_t *new, kThread_t *current)
{
    /**********************************
     * Interrupts and Mode Assertions *
     **********************************/
    ASSERT(!HwIsSti());
    xm_u32_t mode;
    READ_CPSR(mode);
    ASSERT((mode & PSR_MODE) == PSR_MODE_MON);

    /********
     * IRQs *
     ********/
    //Save VBAR and set XMTempVBAR.
    CA9_SCR_NONSECURE();
    if ((current) && (current->ctrl.g))
    {  CA9_GET_VBAR(current->ctrl.g->kArch.nonsecure_vbar);  }

    xm_u32_t m[HWIRQS_VECTOR_SIZE];
    HwIrqGetPendMask(m);
    if((~(m[0]))&(BIT(XM_EXTIRQ_SGI_NUM)))
    {
//        kprintf("CLEAN SGI\n");
        CA9_SET_VBAR(&_nonSecExceptions);
        _clean_gic_pending();
        HwIrqGetPendMask(m);
        ASSERT(!((~(m[0]))&(BIT(XM_EXTIRQ_SGI_NUM))));
    }

    if ((new) && (new->ctrl.g))
    {  CA9_SET_VBAR(new->ctrl.g->kArch.nonsecure_vbar);  }
    else
    {CA9_SET_VBAR(&_nonSecExceptions);}
    CA9_SCR_SECURE();

    /****************
     * Memory Acces *
     ****************/
#ifdef CONFIG_MMU
    if ((new) && (new->ctrl.g))
    {
        CA9_SCR_SECURE();
        if (new->ctrl.g->kArch.ptdL1S)
        {
#if 0
            eprintf("\t\t[PRE]ptdL1S 0x%x ptdL1NS 0x%x\n", new->ctrl.g->kArch.ptdL1S, new->ctrl.g->kArch.ptdL1NS);
#endif
            /*Secure World*/
            WR_TTBR0(new->ctrl.g->kArch.ptdL1S);
//            WR_TTBR1(_ptdL1S);
//            WR_TTBCR(0x0);
    		FlushCache();
            FlushTlbGlobal();
        }
        if (new->ctrl.g->kArch.ptdL1NS)
        {
            /*Non-Secure World*/
            CA9_SCR_NONSECURE();
//        	DisableMMU();
            WR_TTBR0(new->ctrl.g->kArch.ptdL1NS);
//            WR_TTBR1(_ptdL1NS);
//            WR_TTBCR(0x3);
            NSFlushCache();
            /*Back to Secure World*/
            NSFlushTlbGlobal();
            CA9_SCR_SECURE();
        }
/*        CA9_SCR_SECURE();
        EnableMMU();
        CA9_SCR_NONSECURE();
        EnableMMU();
        CA9_SCR_SECURE();*/
    }
    if ((new) && (new->ctrl.g))
    {
        if (new->ctrl.g->kArch.ptdL1S && new->ctrl.g->kArch.ptdL1NS)
        {
            /*Map Partition Control Table*/
            /*MapPartitionControlTable(((xmAddress_t) (new->ctrl.g->partCtrlTab)));*/
            MapPartitionControlTable( ((xmAddress_t *)(new)) );
        }
    }
#else /* CONFIG_MMU */
//    if (new->ctrl.g) {
//        StoreIoReg(LEON_MEMORYWPR_BASE+WPR1_REG, new->ctrl.g->kArch.wpReg);
//
//#if defined(CONFIG_MPU) && defined(CONFIG_AT697F)
//        //In case there is no shared memory area configured in xm_cf.xml file, WPR2 is programmed with the same value as WPR1
//        if (xmcTab.hpv.noSharedMemoryAreas == 0) {
//            StoreIoReg(LEON_MEMORY_WPR_BASE+WPR2_REG, new->ctrl.g->kArch.wpReg);
//        }
//#endif
//    }

#endif /* CONFIG_MMU */
    /*
     * APB Devices
     */
    CA9_SCR_SECURE();
    CA9_TZ_CLEAN_APB();

    if ((new) && (new->ctrl.g) && (new->ctrl.g->partCtrlTab))
    {
        xm_u32_t dev = new->ctrl.g->partCtrlTab->arch.devices;
        CA9_TZ_WRITE_APB(dev);
    }

    /*******
     * FPU *
     *******/
#ifdef CONFIG_VFP_EXTENSION
    if (current)
    {
        struct xmcPartition * partCfg = GetPartitionCfg(current);
        if (((partCfg) && (partCfg->flags & XM_PART_FP)))
        {
            if (current->ctrl.g)
            {
                fpuCtxt_t * ptr = (&(current->ctrl.g->kArch.fpuRegs));
                HwSaveFpu(ptr);
            }
        }
        HwDisableFpu();
    }
    if (new)
    {
        struct xmcPartition * partCfg = GetPartitionCfg(new);
        if (((partCfg) && (partCfg->flags & XM_PART_FP)))
        {
            HwSetFpuUserReady();
            if (new->ctrl.g)
            {
                fpuCtxt_t * ptr = (&(new->ctrl.g->kArch.fpuRegs));
                HwRestoreFpu(ptr);
            }
            else
            {
                HwDisableFpu();
            }
        }
        else
        {
            HwDisableFpu();
        }
    }
#endif /*CONFIG_VFP_EXTENSION*/
#if 0
    xm_s32_t irqNr;
    for (irqNr = 0; irqNr < CONFIG_NO_HWIRQS; irqNr++)
    {
        if ((new) && (new->ctrl.g) && (xmcTab.hpv.hwIrqTab[irqNr].owner == new->ctrl.g->id))
        {

            CA9_SCR_SECURE();
                ICDDCR_DISABLE();
                ICDISR_NONSECURE(irqNr);
                ICDIPR_SETPRIO(irqNr,0);
                ICDISER_ENABLE(irqNr);
                ICDDCR_ENABLE();
            CA9_SCR_NONSECURE();
                ICDDCR_DISABLE();
                ICDISR_NONSECURE(irqNr);
                ICDIPR_SETPRIO(irqNr,0);
                ICDISER_ENABLE(irqNr);
                ICDDCR_ENABLE();
            CA9_SCR_SECURE();
        }
        else
        {
            CA9_SCR_SECURE();
                ICDDCR_DISABLE();
                ICDISR_SECURE(irqNr);
                ICDDCR_ENABLE();
            CA9_SCR_NONSECURE();
                ICDDCR_DISABLE();
                ICDISR_SECURE(irqNr);
                ICDDCR_ENABLE();
            CA9_SCR_SECURE();
        }
    }
#endif /**/
//#ifdef CONFIG_FLUSH_CACHE_AFTER_CS
//    if (current->ctrl.g)
//	cSPrevPart=current;
//#endif
    FlushCache();
    FlushTlbGlobal();
}

void SwitchKThreadArchPost(kThread_t *current)
{
//	//eprintf("ARMPorting:\t====> SwitchKThreadArchPost 0x%x\n", current);
    ASSERT(!HwIsSti());
    /*	if (current->ctrl.g)
     eprintf("<-----PST %x-------\n", current->ctrl.g->id);
     else
     eprintf("<-----PST I-------\n");*/

//#ifdef CONFIG_MMU
//    if (current->ctrl.g)
//    {
//        if (current->ctrl.g->kArch.ptdL1)
//        {
//        	/*UnMap Partition Control Table*/
//        	UnMapPartitionControlTable(current->ctrl.g->partCtrlTab);
//        }
//    }
//#endif /* CONFIG_MMU */
//    // Flushing the cache
//    FlushTlbGlobal();
//    NONSEC_FLUSH();
//#ifdef CONFIG_FLUSH_CACHE_AFTER_CS
//    if (current->ctrl.g)
//        SetKThreadFlags(current, KTHREAD_FLUSH_DCACHE_F|KTHREAD_FLUSH_ICACHE_F);
//#endif
//	//eprintf("ARMPorting:\t<==== SwitchKThreadArchPost\n");
#if 0
	CA9_SCR_SECURE();
	ICDDCR_DISABLE(); DSB_INST(); ISB_INST();
	CA9_SCR_NONSECURE();
	ICDDCR_DISABLE(); DSB_INST(); ISB_INST();
	CA9_SCR_SECURE();
#endif
    xm_s32_t irqNr;
	CA9_SCR_SECURE();
	for (irqNr = 0; irqNr < CONFIG_NO_HWIRQS; irqNr++)
    {
		if(irqNr == XM_EXTIRQ_SGI_NUM)
        {
			ICDISR_NONSECURE(irqNr);
			ICDIPR_SETPRIO(irqNr,0);
			ICDISER_ENABLE(irqNr);
			continue;
        }
    	if ((current) && (current->ctrl.g) && (xmcTab.hpv.hwIrqTab[irqNr].owner == current->ctrl.g->id))
        {
                ICDISR_NONSECURE(irqNr);
                ICDIPR_SETPRIO(irqNr,0);
                ICDISER_ENABLE(irqNr);
        }
        else
        {
                ICDISR_SECURE(irqNr);
        }
    }
    CA9_SCR_NONSECURE();
	for (irqNr = 0; irqNr < CONFIG_NO_HWIRQS; irqNr++)
    {
		if(irqNr == XM_EXTIRQ_SGI_NUM)
        {
			ICDISR_NONSECURE(irqNr);
			ICDIPR_SETPRIO(irqNr,0);
			ICDISER_ENABLE(irqNr);
			continue;
        }
    	if ((current) && (current->ctrl.g) && (xmcTab.hpv.hwIrqTab[irqNr].owner == current->ctrl.g->id))
        {
                ICDISR_NONSECURE(irqNr);
                ICDIPR_SETPRIO(irqNr,0);
                ICDISER_ENABLE(irqNr);
        }
        else
        {
                ICDISR_SECURE(irqNr);
        }
    }

#if 0
	CA9_SCR_SECURE();
	ICDDCR_ENABLE(); DSB_INST(); ISB_INST();
	CA9_SCR_NONSECURE();
	ICDDCR_ENABLE(); DSB_INST(); ISB_INST();
	CA9_SCR_SECURE();
#endif

	if (current)
	{
#ifdef CONFIG_ENABLE_CACHE
		xm_u32_t cache=0;
		if (AreKThreadFlagsSet(current, KTHREAD_DCACHE_ENABLED_F))
		    cache|=DCACHE;
		if (AreKThreadFlagsSet(current, KTHREAD_ICACHE_ENABLED_F))
		    cache|=ICACHE;
		CA9_SCR_SECURE();
		SetCacheState(cache);
		CA9_SCR_NONSECURE();
		NSSetCacheState(cache);
		CA9_SCR_SECURE();
#endif
	}
}

void KThreadArchInit(kThread_t *k)
{
}

void SetupKStack(kThread_t *k, void *StartUp, xmAddress_t entry)
{
    extern void __AsmStartUp(void);

    k->ctrl.kStack = (xm_u32_t *) &k->kStack[CONFIG_KSTACK_SIZE];
    *--(k->ctrl.kStack) = (xm_u32_t) StartUp;
    *--(k->ctrl.kStack) = (xm_u32_t) entry;
    *--(k->ctrl.kStack) = (xm_u32_t) __AsmStartUp;
}

#define IS_POW_TWO(x) (x&&!(x&(x-1)))
void ArchCreateKThread(kThread_t *k)
{
    k->ctrl.irqMask[0] &= (~0x1);

    xm_u32_t e;
    for (e=0; e<HWIRQS_VECTOR_SIZE; e++)
    {   k->ctrl.irqPendMask[e] = ((xm_u32_t)(~0));  }

//#ifdef CONFIG_MPU
//    xm_u32_t tag, mask;
//    struct xmPhysicalMemMap memMap;
//    struct xmcPartition *conf=k->ctrl.g->cfg;
//    // Checking that the memory area of the guest
//
//    // 1) For this configuration (CONFIG_MPU), only one area is allowed
//    if (conf->noPhysicalMemoryAreas!=1) PartitionPanic(0, "[Guest] Only one memory area expected\n");
//
//    memMap.startAddr=xmcPhysMemAreaTab[k->ctrl.g->cfg->physicalMemoryAreasOffset].startAddr;
//    memMap.size=xmcPhysMemAreaTab[k->ctrl.g->cfg->physicalMemoryAreasOffset].size;
//
//    // 2) The area size must be at _least_ 32KB and power of two
//    if ((memMap.size&(REG_SIZE-1))&&!IS_POW_TWO(memMap.size))
//        goto bad_area;
//
//    // 3) The start address must be greater than or equal to the size or must be power of two
//    if ((memMap.startAddr<memMap.size)&&!IS_POW_TWO(memMap.startAddr))
//        goto bad_area;
//
//    // 4) Size must be multiple of 32 KB (REG_SIZE)
//    //and start address must be multiple of 32 KB (REG_SIZE)
//    //and size must be less or equal to start address value
//    if ((memMap.size & (REG_SIZE-1)) ||(memMap.startAddr & (REG_SIZE-1)) ||(memMap.size > memMap.startAddr))
//        goto bad_area;
//
//    // 5) Start address must be a multiple of size
//    if (memMap.startAddr % memMap.size)
//        goto bad_area;
//
//    //Configuration of WPR1 with BP = 0
//    //Make writable region from (memMap.startAddr) to (memMap.startAddr + memMap.size)
//    mask = ((~(memMap.size-1)) & REG_MASK);
//    tag = (memMap.startAddr & REG_MASK);
//    k->ctrl.g->kArch.wpReg = (mask>>REG_OFF)|tag;
//
//    //WPR2 configuration will depend on the board target. In case it is used, it will be configured in the XtratuM setup.
//    return;
//
//bad_area:
//    PartitionPanic(0, "[Guest] Bad memory area [0x%x - 0x%x]\n", memMap.startAddr, memMap.startAddr+memMap.size);
//#endif
}

void FillArchPartCtrlTabWarm(partitionControlTable_t *partCtrlTab, kThread_t *k)
{
    struct xmcPartition *cfg = GetPartitionCfg(k);
    partCtrlTab->irqIndex = 0x0; /*TODO ARMPorting: It could be an TAG. Maybe undefined.*/
    partCtrlTab->arch.devices = cfg->apbDevices;
}

void FillArchPartCtrlTabCold(partitionControlTable_t *partCtrlTab, kThread_t *k)
{
    struct xmcPartition *cfg = GetPartitionCfg(k);
    partCtrlTab->irqIndex = 0x0; /*TODO ARMPorting: It could be an TAG. Maybe undefined.*/
    partCtrlTab->arch.devices = cfg->apbDevices;

//    xm_s32_t e;
//    partCtrlTab->trap2Vector[0]=DATA_STORE_ERROR;
//    partCtrlTab->trap2Vector[1]=INSTRUCTION_ACCESS_MMU_MISS;
//    partCtrlTab->trap2Vector[2]=INSTRUCTION_ACCESS_ERROR;
//    partCtrlTab->trap2Vector[3]=R_REGISTER_ACCESS_ERROR;
//    partCtrlTab->trap2Vector[4]=INSTRUCTION_ACCESS_EXCEPTION;
//    partCtrlTab->trap2Vector[5]=PRIVILEGED_INSTRUCTION;
//    partCtrlTab->trap2Vector[6]=ILLEGAL_INSTRUCTION;
//    partCtrlTab->trap2Vector[7]=FP_DISABLED;
//    partCtrlTab->trap2Vector[8]=CP_DISABLED;
//    partCtrlTab->trap2Vector[9]=UNIMPLEMENTED_FLUSH;
//    partCtrlTab->trap2Vector[10]=WATCHPOINT_DETECTED;
//    partCtrlTab->trap2Vector[11]=MEM_ADDRESS_NOT_ALIGNED;
//    partCtrlTab->trap2Vector[12]=FP_EXCEPTION;
//    partCtrlTab->trap2Vector[13]=CP_EXCEPTION;
//    partCtrlTab->trap2Vector[14]=DATA_ACCESS_ERROR;
//    partCtrlTab->trap2Vector[15]=DATA_ACCESS_MMU_MISS;
//    partCtrlTab->trap2Vector[16]=DATA_ACCESS_EXCEPTION;
//    partCtrlTab->trap2Vector[17]=TAG_OVERFLOW;
//    partCtrlTab->trap2Vector[18]=DIVISION_BY_ZERO;
//
//    for (e=0; e<CONFIG_NO_HWIRQS; e++)
//        partCtrlTab->hwIrq2Vector[e]=irq2Vector[e];
//
//    for (e=0; e<XM_VT_EXT_MAX; e++)
//	partCtrlTab->extIrq2Vector[e]=224+e;
}

void ArchResetKThread(kThread_t *k, xmAddress_t entryPoint)
{
    k->ctrl.g->kArch.entry = entryPoint;
    k->ctrl.g->kArch.nonsecure_vbar = entryPoint;
#   ifdef    CONFIG_VFP_EXTENSION
//    memset(k->ctrl.g->kArch.fpuRegs,0,(XM_FPU_REGS_SIZE/4));
    if(k == GetPartition(k)->kThread[0])
    {
        HwDisableFpu();
        fpuCtxt_t * ptr = (&(k->ctrl.g->kArch.fpuRegs));
        HwSaveFpu(ptr);
    }
#   endif /*CONFIG_VFP_EXTENSION*/
}
