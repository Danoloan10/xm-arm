/*
 * $FILE: setup.c
 *
 * Setting up and starting up the kernel
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
 * - [21/01/16:XM-ARM-2:SPR-160121-06:#108] Some partitions are not loaded in SMP.
 * - [21/01/16:XM-ARM-2:SPR-150608-03:#57] Fix SystemReset on SMP.
 */

#include <assert.h>
#include <boot.h>
#include <comp.h>
#include <rsvmem.h>
#include <kdevice.h>
#include <ktimer.h>
#include <stdc.h>
#include <irqs.h>
#include <objdir.h>
#include <physmm.h>
#include <processor.h>
#include <sched.h>
//#include <log.h>
#include <kthread.h>
#include <vmmap.h>
#include <virtmm.h>
#include <xmconf.h>

#include <local.h>

#include <objects/status.h>
#include <objects/console.h>
#include <arch/paging.h>
#include <arch/xm_def.h>


// CPU's frequency
xm_u32_t cpuKhz;
xm_u16_t __nrCpus=0;

struct xmcPartition *xmcPartitionTab;
struct xmcMemoryRegion *xmcMemRegTab;
struct xmcMemoryArea *xmcPhysMemAreaTab;
#if defined(CONFIG_MPU) && defined(CONFIG_AT697F)
struct xmcSharedMemoryArea *xmcSharedMemAreaTab;
#endif
struct xmcCommChannel *xmcCommChannelTab;
struct xmcCommPort *xmcCommPorts;
struct xmcIoPort *xmcIoPortTab;
struct xmcSchedCyclicSlot *xmcSchedCyclicSlotTab;
struct xmcSchedCyclicPlan *xmcSchedCyclicPlanTab;
struct xmcRsvMem *xmcRsvMemTab;
struct xmcBootPart *xmcBootPartTab;
struct xmcVCpu *xmcVCpuTab;
xm_u8_t *xmcDstIpvi;
xm_s8_t *xmcStringTab;

#if defined(CONFIG_DEV_MEMBLOCK)||defined(CONFIG_DEV_MEMBLOCK_MODULE)
struct xmcMemBlock *xmcMemBlockTab;
#endif

// Local info
localProcessor_t localProcessorInfo[CONFIG_NO_CPUS];

extern xm_u32_t resetStatusInit[];

static volatile xm_s32_t procWaiting __VBOOTDATA=0;

extern xm_u8_t _sxm[], _exm[], physXmcTab[];
extern void start(void);

struct xmHdr xmHdr __XMHDR={
    .sSignature=XMEF_XM_MAGIC,
    .compilationXmAbiVersion=XM_SET_VERSION(XM_ABI_VERSION, XM_ABI_SUBVERSION, XM_ABI_REVISION),
    .compilationXmApiVersion=XM_SET_VERSION(XM_API_VERSION, XM_API_SUBVERSION, XM_API_REVISION),
    .noCustomFiles=1,
    .customFileTab={
	[0]={.sAddr=(xmAddress_t)physXmcTab, .size=0,},
    },
    .eSignature=XMEF_XM_MAGIC,
};

extern __NOINLINE void FreeBootMem(void);

void IdleTask(void) {
	while(1) {
		DoPreemption();
	}
}

void HaltSystem(void) {
	extern void __HaltSystem(void);
	/*    kprintf("System halted.\n"); */

    __HaltSystem();
}

void ResetSystem(xm_u32_t resetMode) {
    extern xm_u32_t sysResetCounter[];
    extern xm_u32_t sysResetStatus[];
    extern void start(void);
    extern void _Reset(xmAddress_t);
#ifdef CONFIG_SMP_SUPPORT
    extern volatile xm_u32_t resetMask[];
#endif
    ASSERT(!HwIsSti());
    sysResetCounter[0]++;
    sysResetStatus[0]=resetMode;

#ifdef CONFIG_SMP_SUPPORT
    if ((resetMode&XM_RESET_MODE)==XM_WARM_RESET) {
        SendIpi(0, WARM_RESET_SYSTEM_IPI_MSG);
    } else {
            SendIpi(0, COLD_RESET_SYSTEM_IPI_MSG);
    }
	if (GET_CPU_ID()==0) {
		while (resetMask[0]);
    }
#endif

    extern void SystemReset(void);
    SystemReset();

    if ((resetMode&XM_RESET_MODE)==XM_WARM_RESET) {
        _Reset((xmAddress_t)start);
    } else { // Cold Reset
        sysResetCounter[0]=0;
        resetStatusInit[0]=0;
//        kprintf("RSW 0x%x\n", xmcTab.rsw.entryPoint);
        _Reset(xmcTab.rsw.entryPoint);
    }
    while(1);
}

static void __VBOOT CreateLocalInfo(void) {
    xm_s32_t e,e1;
    if (!GET_NRCPUS()) {
        cpuCtxt_t ctxt;
        GetCpuCtxt(&ctxt);
        SystemPanic(&ctxt, "No cpu found in the system\n");    
    }
    memset(localProcessorInfo, 0, sizeof(localProcessor_t)*CONFIG_NO_CPUS);
    for (e=0; e<CONFIG_NO_CPUS; e++)
		for (e1=0; e1<HWIRQS_VECTOR_SIZE; e1++)
        localProcessorInfo[e].cpu.globalIrqMask[e1]=~0;

}

static void __VBOOT LocalSetup(xm_s32_t cpuId, kThread_t *idle) {
    ASSERT(!HwIsSti());
    ASSERT(xmcTab.hpv.noCpus>cpuId);
    SetupCpu();
    SetupArchLocal(cpuId);
    SetupHwTimer();
    SetupKTimers();
    SetupErrorControl();
#ifdef CONFIG_SMP_SUPPORT
    SetupLocalSmp();
#endif
#ifdef CONFIG_ARM
    SetupSGIs();
#endif
    InitSchedLocal(idle);
#if defined(CONFIG_MPU) && defined(CONFIG_AT697F)
    SetupSharedMemoryArea();
#endif
}

static void __VBOOT SetupPartitions(void) {
    xmAddress_t st, end, vSt, vEnd;
    partition_t *k;
    xm_s32_t e, a;

    kprintf ("%d Partition(s) created\n", xmcTab.noPartitions);
    // Creating the partitions
    extern xmWord_t** ptdL1S_list;
    for(e=1;e<xmcTab.noPartitions;e++)
	{
		memcpy(ptdL1S_list[e], ptdL1S_list[0], PTDL1SIZE);
	}
    for (e=0; e<xmcTab.noPartitions; e++) {
        if ((k=CreatePartition(&xmcPartitionTab[e]))) {
	    kprintf("P%d (\"%s\":%d) flags: [", e, &xmcStringTab[xmcPartitionTab[e].nameOffset], xmcPartitionTab[e].rId);
	    if (xmcPartitionTab[e].flags&XM_PART_SYSTEM)
                kprintf(" SYSTEM");
	    if (xmcPartitionTab[e].flags&XM_PART_FP)
               kprintf(" FP");
	    kprintf(" ]:\n");
	    for (a=0; a<xmcPartitionTab[e].noPhysicalMemoryAreas; a++) {
		st=xmcPhysMemAreaTab[a+xmcPartitionTab[e].physicalMemoryAreasOffset].startAddr;
		end=st+xmcPhysMemAreaTab[a+xmcPartitionTab[e].physicalMemoryAreasOffset].size-1;
                vSt=xmcPhysMemAreaTab[a+xmcPartitionTab[e].physicalMemoryAreasOffset].mappedAt;
		vEnd=vSt+xmcPhysMemAreaTab[a+xmcPartitionTab[e].physicalMemoryAreasOffset].size-1;
                
		kprintf("    [0x%lx:0x%lx - 0x%lx:0x%lx]", st, vSt, end, vEnd);
		kprintf(" flags: 0x%x", xmcPhysMemAreaTab[a+xmcPartitionTab[e].physicalMemoryAreasOffset].flags);
		kprintf("\n");
	    }

            if ((xmcBootPartTab[e].flags&XM_PART_BOOT)&&(xmcPartitionTab[e].flags&XM_PART_BOOT)) 
                ResetPartition(k, xmcBootPartTab[k->cfg->id].entryPoint, 1, resetStatusInit[0]);  
	} else {
            cpuCtxt_t ctxt;
            GetCpuCtxt(&ctxt);
	    SystemPanic(&ctxt, "[LoadGuests] Error creating partition");
        }
    }
}

static void __VBOOT LoadCfgTab(void) {
    // Check configuration file
    if (xmcTab.signature!=XMC_SIGNATURE)
    	HaltSystem();
#define CALC_ABS_ADDR_XMC(_offset) (void *)(xmcTab._offset+(xmAddress_t)&xmcTab)
    
    xmcPartitionTab=CALC_ABS_ADDR_XMC(partitionTabOffset);
    xmcBootPartTab=CALC_ABS_ADDR_XMC(bootPartitionTabOffset);
    xmcMemRegTab=CALC_ABS_ADDR_XMC(memoryRegionsOffset);
    xmcPhysMemAreaTab=CALC_ABS_ADDR_XMC(physicalMemoryAreasOffset);
#if defined(CONFIG_MPU) && defined(CONFIG_AT697F)
    xmcSharedMemAreaTab=CALC_ABS_ADDR_XMC(sharedMemoryAreasOffset);
#endif
    xmcCommChannelTab=CALC_ABS_ADDR_XMC(commChannelTabOffset);
    xmcCommPorts=CALC_ABS_ADDR_XMC(commPortsOffset);
    xmcIoPortTab=CALC_ABS_ADDR_XMC(ioPortsOffset);
    xmcSchedCyclicSlotTab=CALC_ABS_ADDR_XMC(schedCyclicSlotsOffset);
    xmcSchedCyclicPlanTab=CALC_ABS_ADDR_XMC(schedCyclicPlansOffset);
    xmcStringTab=CALC_ABS_ADDR_XMC(stringsOffset);   
    xmcRsvMemTab=CALC_ABS_ADDR_XMC(rsvMemTabOffset);
    xmcDstIpvi=CALC_ABS_ADDR_XMC(ipviDstOffset);     
#if defined(CONFIG_DEV_MEMBLOCK)||defined(CONFIG_DEV_MEMBLOCK_MODULE)
    xmcMemBlockTab=CALC_ABS_ADDR_XMC(deviceTab.memBlocksOffset);
#endif
    xmcVCpuTab=CALC_ABS_ADDR_XMC(vCpuTabOffset);
}

#ifdef CONFIG_SMP_SUPPORT
static spinLock_t sLBarrier=SPINLOCK_INIT;
static volatile xm_u32_t syncProc0=0, syncProc=0;

static void SyncSmp0(void) {
    SpinLock(&sLBarrier);
    syncProc0++;
    SpinUnlock(&sLBarrier);
    while(syncProc0!=GET_NRCPUS());
}

static void SyncSmp(void) {
    SpinLock(&sLBarrier);
    syncProc++;
    SpinUnlock(&sLBarrier);
//    if (GET_CPU_ID()==1)
    //      while(1) kprintf("%ld\n", GetSysClockUsec());
    while(syncProc!=GET_NRCPUS());
}

#endif

void __VBOOT Setup(xm_s32_t cpuId, kThread_t *idle){
	ASSERT(!HwIsSti());
    ASSERT(GET_CPU_ID()==0);
#ifdef CONFIG_EARLY_OUTPUT
	extern void SetupEarlyOutput(void);
	SetupEarlyOutput();
#endif

    LoadCfgTab();
    InitRsvMem();
    EarlySetupArchCommon();
    SetupVirtMM();
    SetupPhysMM();
    SetupArchCommon();
    SetupIrqs();
    SetupKDev();
    SetupObjDir();
    //SetupLog();
    kprintf("\nXM Hypervisor (%x.%x r%x) Built %s %s\n", ((XM_VERSION>>16)&0xFF), ((XM_VERSION>>8)&0xFF), (XM_VERSION&0xFF), __DATE__, __TIME__);
    kprintf("Detected %lu.%luMHz processor.\n", (xm_u32_t)(cpuKhz/1000), (xm_u32_t)(cpuKhz%1000));
    InitSched();
    CreateLocalInfo();
    SetupSysClock();
    LocalSetup(cpuId, idle);
    SetupPartitions();
#ifdef CONFIG_SMP_SUPPORT    
    SetupSmp();
    SyncSmp0();
#endif
#if 0
    RsvMemDebug();
#endif
#ifdef CONFIG_SMP_SUPPORT
    SyncSmp();
#endif
    FreeBootMem();
}

#ifdef CONFIG_SMP_SUPPORT
void __VBOOT InitSecondaryCpu(xm_s32_t cpuId, kThread_t *idle) {
	kprintf("InitSecondaryCpu %d 0x%x\n", cpuId, idle);
    ASSERT(GET_CPU_ID()!=0);
#ifdef CONFIG_ARM
    ArchSetupIrqs();
#endif
    LocalSetup(cpuId, idle);
	/*TOBEAPPROVED ARMPorting: Get directly Sched instead info*/
    GET_LOCAL_SCHED()->flags|=LOCAL_SCHED_ENABLED;
    SyncSmp0();
    SyncSmp();
    Schedule();
    IdleTask();
}
#endif
