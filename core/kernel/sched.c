/*
 * $FILE: sched.c
 *
 * Scheduling related stuffs
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
#include <audit.h>
#include <boot.h>
#include <rsvmem.h>
#include <irqs.h>
#include <sched.h>
#include <smp.h>
#include <spinlock.h>
#include <stdc.h>
#include <arch/asm.h>

#if defined(CONFIG_DEV_LICE_INTERFACE) || defined(CONFIG_DEV_LICE_SCHEDULING)
#include <drivers/lice_interface.h>
#endif

partition_t *partitionTab;
static struct schedData *schedDataTab;

static const struct xmcSchedCyclicPlan idleCyclicPlanTab= {
    .nameOffset = 0,
    .id = 0,
    .majorFrame = 0,
    .noSlots = 0,
    .slotsOffset = 0,
};

void __VBOOT InitSched(void) {
    extern void SetupKThreads(void);

    SetupKThreads();
    GET_MEMZ(partitionTab, sizeof(partition_t)*xmcTab.noPartitions);
    GET_MEMZ(schedDataTab, sizeof(struct schedData)*xmcTab.hpv.noCpus);
}

#ifdef CONFIG_PLAN_EXTSYNC
static volatile xm_s32_t extSync=0, actExtSync=0;

static void SchedSyncHandler(cpuCtxt_t *irqCtxt, void *extra) {
    extSync=1;
    Schedule();
}
#endif

void __VBOOT InitSchedLocal(kThread_t *idle) {
	/*TOBEAPPROVED ARMPorting: Get directly Sched instead info*/
	localSched_t * sched =  GET_LOCAL_SCHED();

#ifdef CONFIG_PLAN_EXTSYNC
    xm_u32_t irqNr = xmcSchedCyclicPlanTab[xmcTab.hpv.cpuTab[GET_CPU_ID()].schedCyclicPlansOffset].extSync;
    if (irqNr != -1) {
        localCpu_t *lCpu=GET_LOCAL_CPU();
        (*lCpu->globalIrqMask) &=~(1<<irqNr); //Keep in mind the ext sync irq in the globalIrqMask
        actExtSync=1;
        SetIrqHandler(irqNr, SchedSyncHandler, 0);
        HwEnableIrq(irqNr);
    }
#endif

	InitIdle(idle, GET_CPU_ID());
	sched->cKThread=sched->idleKThread=idle;
	if( (GET_CPU_ID()+1) <= xmcTab.hpv.noCpus )
	{
        sched->data=&schedDataTab[GET_CPU_ID()];
        memset(sched->data, 0, sizeof(struct schedData));
        InitKTimer(GET_CPU_ID(), &sched->data->kTimer, (void (*)(struct kTimer *, void *))SetSchedPending, NULL, NULL);
        sched->data->slot=-1;
        sched->data->plan.new=&xmcSchedCyclicPlanTab[xmcTab.hpv.cpuTab[GET_CPU_ID()].schedCyclicPlansOffset];
        sched->data->plan.current=0;
        sched->data->plan.prev=0;
	}
}

////////////////// CYCLIC SCHEDULER
xm_s32_t SwitchSchedPlan(xm_s32_t newPlanId, xm_s32_t *oldPlanId) {
    localProcessor_t *info=GET_LOCAL_PROCESSOR();
    xm_s32_t e;
    *oldPlanId=-1;
    if (info->sched.data->plan.current)
        *oldPlanId=info->sched.data->plan.current->id;

    for (e=0; e<xmcTab.hpv.noCpus; e++) {
        if (newPlanId<xmcTab.hpv.cpuTab[e].noSchedCyclicPlans)
            localProcessorInfo[e].sched.data->plan.new=&xmcSchedCyclicPlanTab[xmcTab.hpv.cpuTab[e].schedCyclicPlansOffset+newPlanId];
        else
            localProcessorInfo[e].sched.data->plan.new=&idleCyclicPlanTab;
    }
    return 0;
}

////////////////// CYCLIC SCHEDULER
xm_s32_t SwitchSchedPlanImm(xm_s32_t newPlanId, xm_s32_t *oldPlanId) {
    localProcessor_t *info=GET_LOCAL_PROCESSOR();
    xm_s32_t e;
    xmTime_t cTime;
    *oldPlanId=-1;
    if (info->sched.data->plan.current)
        *oldPlanId=info->sched.data->plan.current->id;

    cTime=GetSysClockUsec();
    for (e=0; e<xmcTab.hpv.noCpus; e++) {
        struct schedData *cyclic=localProcessorInfo[e].sched.data;
        if (newPlanId<xmcTab.hpv.cpuTab[e].noSchedCyclicPlans)
        {
            localProcessorInfo[e].sched.data->plan.new=&xmcSchedCyclicPlanTab[xmcTab.hpv.cpuTab[e].schedCyclicPlansOffset+newPlanId];
        }
        else
        {
            localProcessorInfo[e].sched.data->plan.new=&idleCyclicPlanTab;
        }
        cyclic->nextAct=cTime;
        cyclic->mjf=cTime;
    }
    return 0;
}

inline void MakePlanSwitch(xmTime_t cTime, struct schedData *data) {
    if (data->plan.current!=data->plan.new) {
        data->plan.prev=data->plan.current;
        data->plan.current=data->plan.new;
        data->planSwitchTime=cTime;
        data->slot=-1;
        data->mjf=0;
//#ifdef CONFIG_SYSTEM_INTEGRITY
        data->slotExec=-1;
//#endif
    }
}

static kThread_t *GetReadyKThread(struct schedData *cyclic) {
    const struct xmcSchedCyclicPlan *plan;
    xmTime_t cTime=GetSysClockUsec();
    kThread_t *newK=0;
    xm_u32_t t, nextTime;
    xm_s32_t slotTabEntry;
#ifdef CONFIG_WATCHDOG
    struct hwTimerData *hwWatchdogData=0;
    hwTimer_t *watchdog=AllocWatchdog(hwWatchdogData);
#endif
    if (cyclic->nextAct>cTime)
        return (cyclic->kThread&&!AreKThreadFlagsSet(cyclic->kThread, KTHREAD_HALTED_F|KTHREAD_SUSPENDED_F)&&AreKThreadFlagsSet(cyclic->kThread, KTHREAD_READY_F))?cyclic->kThread:0;
    plan=cyclic->plan.current;
    if (cyclic->mjf<=cTime) {
#ifdef CONFIG_WATCHDOG
        if(numMafs==CONFIG_WATCHDOG_NMAF)
            numMafs=0;
	if(!numMafs){
#ifdef CONFIG_AUDIT_EVENTS
            RaiseAuditEvent(XM_HYPERVISOR_ID, XM_AUDIT_START_WATCHDOG);
#endif
#ifdef CONFIG_DEV_LICE_INTERFACE
            LiceCmdStartWatchdog();
#endif
            watchdog->SetHwTimer(cTime+CONFIG_WATCHDOG_TIMEOUT,hwWatchdogData);
#ifdef CONFIG_DEV_LICE_INTERFACE
            LiceCmdEndWatchdog();
#endif
#ifdef CONFIG_AUDIT_EVENTS
            RaiseAuditEvent(XM_HYPERVISOR_ID, XM_AUDIT_END_WATCHDOG);
#endif

	}
        numMafs++;
#endif
#ifdef CONFIG_PLAN_EXTSYNC
        if (actExtSync&&!extSync) {
            cyclic->slot=-1;            
            return 0; // Idle
        }
        //extSync=0;
#endif
        MakePlanSwitch(cTime, cyclic);
        plan=cyclic->plan.current;
	if (cyclic->slot>=0) {
            while(cyclic->mjf<=cTime) {
                cyclic->sExec=cyclic->mjf;
                cyclic->mjf+=plan->majorFrame;
            }
#ifdef CONFIG_OBJ_STATUS_ACC
	    systemStatus.currentMaf++;
#endif
        } else {
	    cyclic->sExec=cTime;
	    cyclic->mjf=plan->majorFrame+cyclic->sExec;
	}
//#ifdef CONFIG_SYSTEM_INTEGRITY
        if ((plan->noSlots!=cyclic->slotExec+1)&&(cyclic->slot>=0)){
            /*xmHmLog_t hmLog;
            xm_s32_t slotTab=cyclic->plan->slotsOffset+cyclic->slotExec;
            hmLog.partitionId=xmcSchedCyclicSlotTab[slotTab].partitionId;
            hmLog.system=0;
            hmLog.eventId=XM_HM_EV_OVERRUN;
            HmRaiseEvent(&hmLog);*/
	}
        cyclic->slotExec=-1;
//#endif
	cyclic->slot=0;
#ifdef CONFIG_WATCHDOG
	if(numMafs==CONFIG_WATCHDOG_NMAF)
            numMafs=0;
	if(numMafs==0)
	{
#ifdef CONFIG_DEV_LICE_INTERFACE
            LiceCmdStartWatchdog();
#endif
#ifdef CONFIG_AUDIT_EVENTS
            RaiseAuditEvent(XM_HYPERVISOR_ID, XM_AUDIT_START_WATCHDOG);
#endif
            watchdog->SetHwTimer(cTime+CONFIG_WATCHDOG_TIMEOUT,hwWatchdogData);
#ifdef CONFIG_DEV_LICE_INTERFACE
            LiceCmdEndWatchdog();
#endif
#ifdef CONFIG_AUDIT_EVENTS
            RaiseAuditEvent(XM_HYPERVISOR_ID, XM_AUDIT_END_WATCHDOG);
#endif
	}
	numMafs++;
#endif
    }
#ifdef CONFIG_PLAN_EXTSYNC
    extSync=0;
#endif
    t=cTime-cyclic->sExec;
    nextTime=plan->majorFrame;
    // Calculate our next slot
    if (cyclic->slot>=plan->noSlots)
	goto out; // getting idle
    while (t>=xmcSchedCyclicSlotTab[plan->slotsOffset+cyclic->slot].eExec) {
	cyclic->slot++;
	if (cyclic->slot>=plan->noSlots)
	    goto out; // getting idle
    }
    slotTabEntry=plan->slotsOffset+cyclic->slot;
    if (t>=xmcSchedCyclicSlotTab[slotTabEntry].sExec) {
        ASSERT((xmcSchedCyclicSlotTab[slotTabEntry].partitionId>=0)&&(xmcSchedCyclicSlotTab[slotTabEntry].partitionId<xmcTab.noPartitions));
        ASSERT(partitionTab[xmcSchedCyclicSlotTab[slotTabEntry].partitionId].kThread[xmcSchedCyclicSlotTab[slotTabEntry].vCpuId]);
        newK=partitionTab[xmcSchedCyclicSlotTab[slotTabEntry].partitionId].kThread[xmcSchedCyclicSlotTab[slotTabEntry].vCpuId];
        if (!AreKThreadFlagsSet(newK, KTHREAD_HALTED_F|KTHREAD_SUSPENDED_F)&&
            AreKThreadFlagsSet(newK, KTHREAD_READY_F)) {
            nextTime=xmcSchedCyclicSlotTab[slotTabEntry].eExec;
        } else {
//#ifdef CONFIG_SYSTEM_INTEGRITY
	    cyclic->slotExec=cyclic->slot;
//#endif
            newK=0;
            if ((cyclic->slot+1)<plan->noSlots)
                nextTime=xmcSchedCyclicSlotTab[slotTabEntry+1].sExec;
        }        
    } else {
	nextTime=xmcSchedCyclicSlotTab[slotTabEntry].sExec;
    }

out:
    ASSERT(cyclic->nextAct<(nextTime+cyclic->sExec));
    cyclic->nextAct=nextTime+cyclic->sExec;
    ArmKTimer(&cyclic->kTimer, cyclic->nextAct, 0);
    slotTabEntry=plan->slotsOffset+cyclic->slot;
#if 0
    if (newK) {
	kprintf("[%d][%d:%d] cTime %ld -> sExec %ld eExec %ld\n",
                GET_CPU_ID(),
		cyclic->slot,
		xmcSchedCyclicSlotTab[slotTabEntry].partitionId, 
		cTime, xmcSchedCyclicSlotTab[slotTabEntry].sExec+cyclic->sExec, 
		xmcSchedCyclicSlotTab[slotTabEntry].eExec+cyclic->sExec);
    } else {
	kprintf("[CPU%d] IDLE: %ld\n", GET_CPU_ID(), cTime);
    }
#endif
  
    if (newK&&newK->ctrl.g) {
        newK->ctrl.g->partCtrlTab->schedInfo.releasePoint=(xmcSchedCyclicSlotTab[slotTabEntry].flags&XM_PERIOD_START_SLOT)?1:0;
        newK->ctrl.g->partCtrlTab->schedInfo.noSlot=cyclic->slot;
        newK->ctrl.g->partCtrlTab->schedInfo.id=xmcSchedCyclicSlotTab[slotTabEntry].id;
        newK->ctrl.g->partCtrlTab->schedInfo.slotDuration=xmcSchedCyclicSlotTab[slotTabEntry].eExec-xmcSchedCyclicSlotTab[slotTabEntry].sExec;
        SetPartitionExtIrqPending(GetPartition(newK), XM_VT_EXT_CYCLIC_SLOT_START);
#ifdef CONFIG_SYSTEM_INTEGRITY
#define CONFIG_CS_THRESHOLD 200
        if (((cyclic->slotExec+1)!=cyclic->slot)||(t>xmcSchedCyclicSlotTab[cyclic->plan->slotsOffset+cyclic->slot].sExec+CONFIG_CS_THRESHOLD)){
/*		xmHmLog_t hmLog;
		if (cyclic->slotExec < 0)
			hmLog.partitionId=cyclic->plan->noSlots-1;
		else
			hmLog.partitionId=cyclic->slotExec;
		xm_s32_t slotTab=cyclic->plan->slotsOffset+hmLog.partitionId;
		hmLog.partitionId=xmcSchedCyclicSlotTab[slotTab].partitionId;
        	hmLog.system=0;
		hmLog.eventId=XM_HM_EV_OVERRUN;
        	HmRaiseEvent(&hmLog);
*/
	    }
            cyclic->slotExec=cyclic->slot;
#endif
    }

    cyclic->kThread=newK;

    return newK;
} 

void SetSchedPending(void) {
    localProcessor_t *info=GET_LOCAL_PROCESSOR();
    //localCpu_t *cpu=GET_LOCAL_CPU();
    info->cpu.irqNestingCounter|=SCHED_PENDING;
}

void Schedule(void) {
    localProcessor_t *info=GET_LOCAL_PROCESSOR();
    xmWord_t hwFlags;
    kThread_t *newK;

    CHECK_KTHR_SANITY(info->sched.cKThread);
  
    if (!(info->sched.flags&LOCAL_SCHED_ENABLED)) {
	info->cpu.irqNestingCounter&=~(SCHED_PENDING);
	return;
    }

    HwSaveFlagsCli(hwFlags);
    // When an interrupt is in-progress, the scheduler shouldn't be invoked
    if (info->cpu.irqNestingCounter&IRQ_IN_PROGRESS) {
	info->cpu.irqNestingCounter|=SCHED_PENDING;
	HwRestoreFlags(hwFlags);
	return;
    }
 
    info->cpu.irqNestingCounter&=(~SCHED_PENDING);
    if (!(newK=GetReadyKThread(info->sched.data)))
	{	newK=info->sched.idleKThread;	}
    
    CHECK_KTHR_SANITY(newK);

#if 0
    CA9_SCR_SECURE();
    FlushCache();
    CA9_SCR_NONSECURE();
    NSFlushCache();
    CA9_SCR_SECURE();
#endif

    if (newK!=info->sched.cKThread) {
#if defined(CONFIG_DEV_LICE_INTERFACE) || defined(CONFIG_DEV_LICE_SCHEDULING)
	xm_u32_t mem_cfg1;
	mem_cfg1=set_iop_bit();
#endif
#if 0
	if (newK->ctrl.g)
	    kprintf("newK: %d 0x%x ", GetPartitionCfg(newK)->id, newK);
	else
	    kprintf("newK: idle ");

	if (info->sched.cKThread->ctrl.g)
	    kprintf("curK: %d 0x%x\n", GetPartitionCfg(info->sched.cKThread)->id, info->sched.cKThread);
	else
	    kprintf("curK: idle\n");
#endif

	SwitchKThreadArchPre(newK, info->sched.cKThread);
	if (info->sched.cKThread->ctrl.g) // not idle kthread
	    StopVClock(&info->sched.cKThread->ctrl.g->vClock,  &info->sched.cKThread->ctrl.g->vTimer);

#ifdef CONFIG_DEV_LICE_SCHEDULING
	if (info->sched.cKThread->ctrl.g)
            LiceCmdEndPartition(GetPartitionCfg(info->sched.cKThread)->id, info->sched.cKThread->ctrl.g->partCtrlTab->schedInfo.id);
        else
            LiceCmdEndPartition(0xffff, 0xffff);
#endif
	if (newK->ctrl.g)
	    SetHwTimer(TraverseKTimerQueue(&newK->ctrl.localActiveKTimers, GetSysClockUsec()));        
	HwIrqGetMask(info->sched.cKThread->ctrl.irqMask);
	HwIrqSetMask(newK->ctrl.irqMask);
#ifdef CONFIG_DEV_LICE_SCHEDULING
	if (newK->ctrl.g)
            LiceCmdStartPartition(GetPartitionCfg(newK)->id, newK->ctrl.g->partCtrlTab->schedInfo.id);
	else
	  LiceCmdStartPartition(0xffff, 0xffff);
#endif
#ifdef CONFIG_AUDIT_EVENTS
        if (info->sched.cKThread->ctrl.g)
            RaiseAuditEvent(info->sched.cKThread->ctrl.g->cfg->id, XM_AUDIT_END_PARTITION);           
        else
            RaiseAuditEvent(XM_HYPERVISOR_ID, XM_AUDIT_END_PARTITION);          

	if (newK->ctrl.g)
            RaiseAuditEvent(newK->ctrl.g->cfg->id, XM_AUDIT_END_PARTITION);
        else
            RaiseAuditEvent(XM_HYPERVISOR_ID, XM_AUDIT_END_PARTITION); 
#endif
#ifdef CONFIG_DEV_LICE_INTERFACE
        LiceCmdBeginCS();
#endif
#ifdef CONFIG_AUDIT_EVENTS
        RaiseAuditEvent(XM_HYPERVISOR_ID, XM_AUDIT_BEGIN_CS); 
#endif
	CONTEXT_SWITCH(newK, &info->sched.cKThread);
#ifdef CONFIG_DEV_LICE_INTERFACE
        LiceCmdEndCS();
#endif
#ifdef CONFIG_AUDIT_EVENTS
        RaiseAuditEvent(XM_HYPERVISOR_ID, XM_AUDIT_END_CS); 
#endif
#if defined(CONFIG_DEV_LICE_INTERFACE) || defined(CONFIG_DEV_LICE_SCHEDULING)
	restore_iop_bit(mem_cfg1);
#endif
	if (info->sched.cKThread->ctrl.g)
	    ResumeVClock(&info->sched.cKThread->ctrl.g->vClock, &info->sched.cKThread->ctrl.g->vTimer);
	SwitchKThreadArchPost(info->sched.cKThread);
    }
    HwRestoreFlags(hwFlags);
}
