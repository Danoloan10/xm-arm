/*
 * $FILE: kthread.c
 *
 * Kernel and Guest context
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
 * - [10/12/15:XM-ARM-2:SPR-150715-03:#73] Add cache configuration before jump to partition.
 * - [21/10/16:XM-ARM-2:SPR-161013-02:#128] Add parameter XM_PCTRLTAB_VADDR in Jump to partition function.
 * - [23/03/17: XM-ARM-2:SPR-170322-01:#147] Modify the ResetPartition function to save the state of the partition.
 */

#include <assert.h>
#include <rsvmem.h>
#include <ktimer.h>
#include <kthread.h>
#include <physmm.h>
#include <sched.h>
#include <stdc.h>
#include <virtmm.h>
#include <vmmap.h>
#include <xmef.h>

#include <local.h>

#include <objects/status.h>
#include <objects/trace.h>
#include <arch/asm.h>
#include <arch/xm_def.h>

#ifdef CONFIG_ENABLE_CACHE
#include <drivers/zynqboard/zynq_cache.h>
#endif

static kThread_t *kThreadPool;
static struct guest *guestPool;

xmId_t PartitionId2PId(xmId_t partitionId) {
    xm_s32_t n;
    if (partitionId==XM_HYPERVISOR_ID)
        return XM_HYPERVISOR_ID;
    for (n=0; n<xmcTab.noPartitions; n++)
        if (partitionTab[n].cfg->rId==partitionId)
            return n;
    return xmcTab.noPartitions;
}

static void KThrTimerHndl(kTimer_t *kTimer, void *args) {
    kThread_t *k=(kThread_t *)args;
    CHECK_KTHR_SANITY(k);
    SetPartitionExtIrqPending(GetPartition(k), XM_VT_EXT_HW_TIMER);
}

void SetupKThreads(void) { 
    GET_MEMZ(kThreadPool, sizeof(kThread_t)*xmcTab.noPartitions*CONFIG_NO_VCPUS);
    GET_MEMZ(guestPool, sizeof(struct guest)*xmcTab.noPartitions*CONFIG_NO_VCPUS);
}

void InitIdle(kThread_t *idle, xm_s32_t cpu) {
    //localCpu_t *lCpu=GET_LOCAL_CPU();
    localProcessor_t *info=GET_LOCAL_PROCESSOR();
    idle->ctrl.magic1=idle->ctrl.magic2=KTHREAD_MAGIC;
    DynListInit(&idle->ctrl.localActiveKTimers);
    idle->ctrl.irqCpuCtxt=0;
    idle->ctrl.kStack=0;
    idle->ctrl.g=0;
    xm_u32_t e;
    HwIrqGetMask(idle->ctrl.irqMask);
    for (e=0; e<HWIRQS_VECTOR_SIZE; e++)
    {
        idle->ctrl.irqMask[e]&=info->cpu.globalIrqMask[e];
        idle->ctrl.irqPendMask[e]=0xFFFFFFFF;
    }
    SetKThreadFlags(idle, KTHREAD_DCACHE_ENABLED_F|KTHREAD_ICACHE_ENABLED_F|KTHREAD_READY_F);
    ClearKThreadFlags(idle, KTHREAD_HALTED_F);
}

static void __attribute__ ((section (".nsecArea.text"),noinline)) JMP_NONSECURE_PARTITION(xmAddress_t entry, kThread_t * k, xmAddress_t pctAddr)
{
	(void)k;
	xm_u32_t cpsr=0;
	__asm__ __volatile__("mov  r0, %[PCT]       \n\t"
			             "mrs  %[CPSR], cpsr    \n\t"
						 "bic  %[CPSR], %[MASK] \n\t"
            			 "orr  %[CPSR], %[MODE] \n\t"
			             "msr  cpsr_c, %[CPSR]  \n\t"
                         "bx   %[ENTRY]         \n\t"
			             :[CPSR]"+r"(cpsr)
			             :[PCT]"r"(pctAddr),[ENTRY]"r"(entry),[MODE]"I"(PSR_MODE_SVC),[MASK]"I"(PSR_MODE | PSR_F | PSR_I):"r0");
//	ASSERT(0/* This point shall be never reached. */);
}

void StartUpGuest(xmAddress_t entry)
{
	localSched_t * sched =  GET_LOCAL_SCHED();
    kThread_t *k=sched->cKThread;
    cpuCtxt_t ctxt;
    KThreadArchInit(k);
    ResumeVClock(&k->ctrl.g->vClock, &k->ctrl.g->vTimer);
    
#ifdef CONFIG_ENABLE_CACHE
    xm_u32_t cache=0;
    if (AreKThreadFlagsSet(k, KTHREAD_DCACHE_ENABLED_F))
        cache|=DCACHE;
    if (AreKThreadFlagsSet(k, KTHREAD_ICACHE_ENABLED_F))
        cache|=ICACHE;
    SetCacheState(cache);
#endif
#ifdef CONFIG_ARM
    /*TODO ARMPorting: Is here the proper place??*/
    CA9_SCR_NONSECURE();
    CA9_SET_VBAR(entry);
#ifdef CONFIG_ENABLE_CACHE
    NSSetCacheState(cache);
#endif

    // JMP_PARTITION must enable interrupts
    JMP_NONSECURE_PARTITION(entry, k, XM_PCTRLTAB_VADDR);
#else
    // JMP_PARTITION must enable interrupts
    JMP_PARTITION(entry, k);
#endif
    
    GetCpuCtxt(&ctxt);
    PartitionPanic(&ctxt, __XM_FILE__":%u:0x%x: executing unreachable code!", __LINE__, k);
}

static inline void FillPartCtrlTabCold(partitionControlTable_t *partCtrlTab, kThread_t *k);

static inline void GetLocalIrqMask(xm_u32_t mask[], xmId_t id, xm_u32_t cpuId) {
    localProcessor_t *info=&localProcessorInfo[cpuId];
    xm_s32_t e;
	for (e=0; e<HWIRQS_VECTOR_SIZE; e++)
	{
	    mask[e] = info->cpu.globalIrqMask[e];

	}
    for (e=0; e<CONFIG_NO_HWIRQS; e++)
		if (xmcTab.hpv.hwIrqTab[e].owner==id)
			mask[e/32] =~ (1<<(e%32));
}

kThread_t *CreateKThread(xmId_t pId, xmId_t id, struct xmcPartition *cfg) {
    xm_u32_t cpuId;
    kThread_t *k;
    k=&kThreadPool[pId*CONFIG_NO_VCPUS+id];
    k->ctrl.magic1=k->ctrl.magic2=KTHREAD_MAGIC;
    k->ctrl.g=&guestPool[pId*CONFIG_NO_VCPUS+id];
    k->ctrl.g->id=PART_VCPU_ID2KID(pId, id);
    DynListInit(&k->ctrl.localActiveKTimers);
#ifdef CONFIG_SMP_SUPPORT
    k->ctrl.lock=SPINLOCK_INIT;
#endif
    if (cfg->flags&XM_PART_SYSTEM) {
        ClearKThreadFlags(k, KTHREAD_NO_PARTITIONS_FIELD);
        SetKThreadFlags(k, (xmcTab.noPartitions<<16)&KTHREAD_NO_PARTITIONS_FIELD);
    }
    if (!(cfg->flags&XM_PART_ICACHE_DISABLED)) 
        SetKThreadFlags(k, KTHREAD_ICACHE_ENABLED_F);
    if (!(cfg->flags&XM_PART_DCACHE_DISABLED)) 
        SetKThreadFlags(k, KTHREAD_DCACHE_ENABLED_F);
    cpuId=xmcVCpuTab[pId*CONFIG_NO_VCPUS+id].cpu;
    InitKTimer(cpuId, &k->ctrl.g->kTimer, KThrTimerHndl, k, k);
    InitVTimer(cpuId, &k->ctrl.g->vTimer, k);
    SetKThreadFlags(k, KTHREAD_HALTED_F);
    GetLocalIrqMask(k->ctrl.irqMask, pId, cpuId);
    k->ctrl.irqCpuCtxt=0;
    //k->ctrl.irqMask=HwIrqGetMask()|localIrqMask;
    ArchCreateKThread(k);
    return k;
}

partition_t *CreatePartition(struct xmcPartition *cfg) {
    partition_t *p;
    xm_u8_t *pct;
    xmWord_t *pgTabS, *pgTabNS;
    xm_s32_t i;

    ASSERT((cfg->id>=0)&&(cfg->id<xmcTab.noPartitions));
    p=&partitionTab[cfg->id];
    p->cfg=cfg;
    GET_MEMAZ(pct, PCT_SIZE*CONFIG_NO_VCPUS, PTDL1ALIGN);
    p->pctArray=(xmAddress_t)pct;

    ASSERT(SetupPageTable(cfg, (xmAddress_t)pct, &pgTabS, &pgTabNS) == XM_OK);

    for (i=0; i<CONFIG_NO_VCPUS; i++)
    {
        p->kThread[i]=CreateKThread(cfg->id, i, cfg);
        p->kThread[i]->ctrl.g->partCtrlTab=(partitionControlTable_t *)(pct+PCT_SIZE*i);
        FillPartCtrlTabCold(p->kThread[i]->ctrl.g->partCtrlTab, p->kThread[i]);
        p->kThread[i]->ctrl.g->kArch.ptdL1S = ((xmAddress_t)(pgTabS));
        p->kThread[i]->ctrl.g->kArch.ptdL1NS = ((xmAddress_t)(pgTabNS));
    }
    return p;
}

static inline void FillPartCtrlTabCold(partitionControlTable_t *partCtrlTab, kThread_t *k) {
    struct xmPhysicalMemMap *memMap=(struct xmPhysicalMemMap *)((xmAddress_t)partCtrlTab+sizeof(partitionControlTable_t));
    struct xmcPartition *cfg=GetPartitionCfg(k);
    xm_s32_t e;
    xmWord_t *commPortBitmap;
    commPortBitmap=(xmWord_t *)((xmAddress_t)memMap+sizeof(struct xmPhysicalMemMap)*cfg->noPhysicalMemoryAreas);
    memset(partCtrlTab, 0, sizeof(partitionControlTable_t));
    partCtrlTab->magic=KTHREAD_MAGIC;
    partCtrlTab->xmVersion=XM_VERSION;
    partCtrlTab->xmAbiVersion=XM_SET_VERSION(XM_ABI_VERSION, XM_ABI_SUBVERSION, XM_ABI_REVISION);
    partCtrlTab->xmApiVersion=XM_SET_VERSION(XM_API_VERSION, XM_API_SUBVERSION, XM_API_REVISION);
    partCtrlTab->cpuKhz=cpuKhz;
    for (e=0; e<HWIRQS_VECTOR_SIZE; e++)
        partCtrlTab->hwIrqs[e]=cfg->hwIrqs[e];
    partCtrlTab->flags=k->ctrl.flags;
    if(cfg->flags&XM_PART_SYSTEM)
    {
        partCtrlTab->flags |= PARTITION_SYSTEM_F;
    }
    if(cfg->flags&XM_PART_FP)
    {
        partCtrlTab->flags |= PARTITION_FP_F;
    }
    partCtrlTab->id=cfg->rId;
    partCtrlTab->noVCpus = CONFIG_NO_VCPUS;
    strncpy(partCtrlTab->name, &xmcStringTab[cfg->nameOffset], CONFIG_ID_STRING_LENGTH);
    for (e=0; e<HWIRQS_VECTOR_SIZE; e++)
    {partCtrlTab->hwIrqsMask[e]=~0;}
    partCtrlTab->extIrqsMask|=~0;

    partCtrlTab->noPhysicalMemAreas=cfg->noPhysicalMemoryAreas;
    for (e=0; e<cfg->noPhysicalMemoryAreas; e++) {
        if (xmcPhysMemAreaTab[e+cfg->physicalMemoryAreasOffset].flags&XM_MEM_AREA_TAGGED)
            strncpy(memMap[e].name, &xmcStringTab[xmcPhysMemAreaTab[e+cfg->physicalMemoryAreasOffset].nameOffset], CONFIG_ID_STRING_LENGTH);
        memMap[e].startAddr=xmcPhysMemAreaTab[e+cfg->physicalMemoryAreasOffset].startAddr;
        memMap[e].mappedAt=xmcPhysMemAreaTab[e+cfg->physicalMemoryAreasOffset].mappedAt;
        memMap[e].size=xmcPhysMemAreaTab[e+cfg->physicalMemoryAreasOffset].size;
        memMap[e].flags=xmcPhysMemAreaTab[e+cfg->physicalMemoryAreasOffset].flags;
    }
    xmClearBitmap(commPortBitmap, cfg->noPorts);
    partCtrlTab->noCommPorts=cfg->noPorts;

    FillMmPartCtrlTabCold(partCtrlTab, k);
    FillArchPartCtrlTabCold(partCtrlTab, k);
}

static inline void FillPartCtrlTabWarm(partitionControlTable_t *partCtrlTab, kThread_t *k) {
    xm_s32_t e;
    for (e=0; e<HWIRQS_VECTOR_SIZE; e++){
        partCtrlTab->hwIrqsMask[e]=~0;	/*TODO ARMPorting: It could be an TAG.*/
        partCtrlTab->hwIrqsPend[e]=0;	/*TODO ARMPorting: It could be an TAG.*/
    }
    partCtrlTab->extIrqsMask|=~0;		/*TODO ARMPorting: It could be an TAG.*/
    partCtrlTab->extIrqsPend=0;			/*TODO ARMPorting: It could be an TAG.*/

//    InitPCtrlTabIrqs(&partCtrlTab->iFlags); /*iFlag is not mantained in ARM*/
    FillMmPartCtrlTabWarm(partCtrlTab, k);
    FillArchPartCtrlTabWarm(partCtrlTab, k);
}

void ResetKThread(kThread_t *k, xmAddress_t entryPoint) {
    localProcessor_t *info=GET_LOCAL_PROCESSOR();
    SetKThreadFlags(k, KTHREAD_READY_F);
    ClearKThreadFlags(k, KTHREAD_HALTED_F);
    ClearKThreadFlags(k, KTHREAD_SUSPENDED_F);

    ArchResetKThread(k,entryPoint);

    if (k!=info->sched.cKThread) 
    {    SetupKStack(k, StartUpGuest, entryPoint); }
    else 
    {    StartUpGuest(entryPoint);}
}

void ResetPartition(partition_t *p, xmAddress_t entryPoint, xm_u32_t cold, xm_u32_t status) {
    extern void ResetPartitionPorts(struct xmcPartition *cfg);
    kThread_t *k=p->kThread[0];
    HALT_PARTITION(p->cfg->id);
    switch (cold) {
    case 0:
        k->ctrl.g->resetCounter++;
        k->ctrl.g->partCtrlTab->resetCounter=k->ctrl.g->resetCounter;
        k->ctrl.g->resetStatus = status;
        FillPartCtrlTabWarm(k->ctrl.g->partCtrlTab, k);
        break;
    case 1:
        k->ctrl.g->resetCounter = 0;
        k->ctrl.g->resetStatus = status;
        FillPartCtrlTabCold(k->ctrl.g->partCtrlTab, k);     
        break;
    }
    ResetPartitionPorts(p->cfg);
    ResetKThread(k, entryPoint);
}
