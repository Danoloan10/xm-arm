/*
 * $FILE: hypercalls.c
 *
 * XM's hypercalls
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
 * - [10/12/15:XM-ARM-2:SPR-150715-03:#73] Update cache management HCALL.
 * - [15/01/16:XM-ARM-2:SPR-160108-01:#101] Update icache dcache management.
 * - [15/01/16:XM-ARM-2:SPR-160121-01:#103] XM_set_timer(XM_EXEC_CLOCK,...) uses XM_HW_CLOCK as reference for absTime comparation.
 * - [09/09/16:XM-ARM-2:SPR-160906-01:#124] XM_reset_partition: Entry Point argument Check.
 * - [23/03/17: XM-ARM-2:SPR-170322-01:#147] XM_RESET_COLD is used as initial reset status when reset system is called.
 */

#include <assert.h>
#include <kthread.h>
#include <gaccess.h>
#include <physmm.h>
#include <processor.h>
#include <sched.h>
#include <stdc.h>
#include <hypercalls.h>
#include <virtmm.h>
#include <vmmap.h>

#include <objects/status.h>
#include <objects/trace.h>
#ifdef CONFIG_ENABLE_CACHE
#include <drivers/zynqboard/zynq_cache.h>
#endif

extern xm_u32_t resetStatusInit[];

__hypercall xm_s32_t HaltPartitionSys(xmId_t partitionId) {
    localProcessor_t *info=GET_LOCAL_PROCESSOR();
    xmId_t pId=PartitionId2PId(partitionId);
    
    ASSERT(!HwIsSti());
    if (partitionId!=GetPartitionCfg(info->sched.cKThread)->rId) {
        if (!(GetPartitionCfg(info->sched.cKThread)->flags&XM_PART_SYSTEM))
            return XM_PERM_ERROR;
        if (pId>=xmcTab.noPartitions)
	    return XM_INVALID_PARAM;
        HALT_PARTITION(pId);
#ifdef CONFIG_DEBUG_HYPERCALL
        kprintf("[HYPERCALL] (P%d) Halted\n", pId);
#endif
#ifdef CONFIG_SMP_SUPPORT
        xm_s32_t e;
        for (e=0; e<CONFIG_NO_VCPUS; e++) {
            xm_u8_t cpu=xmcVCpuTab[KID2PARTID(pId)*CONFIG_NO_VCPUS+e].cpu;
            if (cpu!=GET_CPU_ID()&&(cpu!=255)) {
                kThread_t *k=partitionTab[KID2PARTID(pId)].kThread[e];
                if (localProcessorInfo[cpu].sched.cKThread==k)
                    SendIpi(cpu, RESCHED_SYSTEM_IPI_MSG);
            }
        }
#endif
        return XM_OK;
    }
#ifdef CONFIG_DEBUG_HYPERCALL
    kprintf("[HYPERCALL] (P%d) Halted\n", pId);
#endif
    HALT_PARTITION(pId);
    Schedule();
    return XM_OK;
}

__hypercall xm_s32_t HaltSystemSys(void) {
    localProcessor_t *info=GET_LOCAL_PROCESSOR();
    extern void HaltSystem(void);
    
    ASSERT(!HwIsSti());
    if (!(GetPartitionCfg(info->sched.cKThread)->flags&XM_PART_SYSTEM))
	return XM_PERM_ERROR;
#ifdef CONFIG_SMP_SUPPORT
    SendAllIpi(HALT_SYSTEM_IPI_MSG);
#endif
    HwCli();
#ifdef CONFIG_DEBUG_HYPERCALL
    kprintf("[HYPERCALL] HaltSystem\n");
#endif
    HaltSystem();
    return XM_OK;
}

// XXX: the ".data" section is restored during the initialisation
__hypercall xm_s32_t ResetSystemSys(xm_u32_t resetMode) {
    localProcessor_t *info=GET_LOCAL_PROCESSOR();
    
    ASSERT(!HwIsSti());
    if (!(GetPartitionCfg(info->sched.cKThread)->flags&XM_PART_SYSTEM))
    	return XM_PERM_ERROR;
	if ((resetMode != XM_WARM_RESET) && (resetMode != XM_COLD_RESET))
		return XM_INVALID_PARAM;
    /* partitions are cold reset when reset system is produced */
    resetStatusInit[0] = XM_COLD_RESET;
#ifdef CONFIG_DEBUG_HYPERCALL
    kprintf("[HYPERCALL] ResetSystem\n");
#endif
    ResetSystem(resetMode);
    return XM_OK;
}

__hypercall xm_s32_t SetCacheStateSys(xm_u32_t cache, xm_u32_t action) {
#ifdef CONFIG_ENABLE_CACHE
    localProcessor_t *info=GET_LOCAL_PROCESSOR();
#endif
    ASSERT(!HwIsSti());
#ifdef CONFIG_ENABLE_CACHE
    if ((action<XM_ACTIVATE_CACHE)||(action>XM_FLUSH_CACHE))
	return XM_INVALID_PARAM;

    if ((cache!=XM_DCACHE)&&(cache!=XM_ICACHE))
        return XM_INVALID_PARAM;

    if (((GetPartition(info->sched.cKThread)->cfg->flags&XM_PART_ICACHE_DISABLED)) && (cache==XM_ICACHE))
        return XM_INVALID_CONFIG;
    if (((GetPartition(info->sched.cKThread)->cfg->flags&XM_PART_DCACHE_DISABLED)) && (cache==XM_DCACHE))
        return XM_INVALID_CONFIG;

    switch(action) {
    case XM_FLUSH_CACHE:
        if (cache&XM_DCACHE)
        {
        	SetKThreadFlags(info->sched.cKThread, KTHREAD_FLUSH_DCACHE_F);
        	FlushDCache();
        }
        if (cache&XM_ICACHE)
        {
        	SetKThreadFlags(info->sched.cKThread, KTHREAD_FLUSH_ICACHE_F);
        	FlushICache();
        }
        return XM_OK;
        break;
    case XM_ACTIVATE_CACHE:
        if (cache&XM_DCACHE)
            SetKThreadFlags(info->sched.cKThread, KTHREAD_DCACHE_ENABLED_F);
        if (cache&XM_ICACHE)
            SetKThreadFlags(info->sched.cKThread, KTHREAD_ICACHE_ENABLED_F);
        SetKThreadFlags(info->sched.cKThread, KTHREAD_FLUSH_DCACHE_F|KTHREAD_FLUSH_ICACHE_F);
        break;
    case XM_DEACTIVATE_CACHE:
        if (cache&XM_DCACHE)
            ClearKThreadFlags(info->sched.cKThread, KTHREAD_DCACHE_ENABLED_F);
        if (cache&XM_ICACHE)
            ClearKThreadFlags(info->sched.cKThread, KTHREAD_ICACHE_ENABLED_F);
        SetKThreadFlags(info->sched.cKThread, KTHREAD_FLUSH_DCACHE_F|KTHREAD_FLUSH_ICACHE_F);
        break;
    }
    info->sched.cKThread->ctrl.g->partCtrlTab->flags&=~(PARTITION_DCACHE_ENABLED_F|PARTITION_ICACHE_ENABLED_F);
    info->sched.cKThread->ctrl.g->partCtrlTab->flags|=(PARTITION_DCACHE_ENABLED_F|PARTITION_ICACHE_ENABLED_F)&info->sched.cKThread->ctrl.flags;
    cache = 0;
    if(AreKThreadFlagsSet(info->sched.cKThread, KTHREAD_FLUSH_DCACHE_F))
    	cache |= DCACHE;
    if(AreKThreadFlagsSet(info->sched.cKThread, KTHREAD_FLUSH_ICACHE_F))
    	cache |= ICACHE;
	CA9_SCR_SECURE();
    SetCacheState(cache);
	CA9_SCR_NONSECURE();
    NSSetCacheState(cache);
	CA9_SCR_SECURE();
    return XM_OK;
#else
    return XM_INVALID_CONFIG;
#endif
}

__hypercall xm_s32_t SwitchSchedPlanSys(xm_u32_t newPlanId, xm_u32_t *__gParam currentPlanId) {
    localProcessor_t *info=GET_LOCAL_PROCESSOR();

    ASSERT(!HwIsSti());
    if (!(GetPartitionCfg(info->sched.cKThread)->flags&XM_PART_SYSTEM))
        return XM_PERM_ERROR;

    if (CheckGParam(currentPlanId, sizeof(xm_u32_t), 4, PFLAG_RW|PFLAG_NOT_NULL)<0)
        return XM_INVALID_PARAM;

    if (info->sched.data->plan.current->id==newPlanId)
        return XM_NO_ACTION;

    if ((newPlanId<1)||(newPlanId>=xmcTab.hpv.cpuTab[GET_CPU_ID()].noSchedCyclicPlans)) return XM_INVALID_PARAM;
    
    if (SwitchSchedPlan(newPlanId, currentPlanId))
        return XM_INVALID_CONFIG;

#ifdef CONFIG_SMP_SUPPORT
    SendAllIpi(RESCHED_SYSTEM_IPI_MSG);
#endif

    return XM_OK;
}

__hypercall xm_s32_t SwitchSchedPlanImmSys(xm_u32_t newPlanId, xm_u32_t *__gParam currentPlanId) {
    localProcessor_t *info=GET_LOCAL_PROCESSOR();

    ASSERT(!HwIsSti());
    if (!(GetPartitionCfg(info->sched.cKThread)->flags&XM_PART_SYSTEM))
        return XM_PERM_ERROR;

    if (CheckGParam(currentPlanId, sizeof(xm_u32_t), 4, PFLAG_RW|PFLAG_NOT_NULL)<0)
        return XM_INVALID_PARAM;

    if (info->sched.data->plan.current->id==newPlanId)
        return XM_NO_ACTION;

    if ((newPlanId<1)||(newPlanId>=xmcTab.hpv.cpuTab[GET_CPU_ID()].noSchedCyclicPlans)) return XM_INVALID_PARAM;

    if (SwitchSchedPlanImm(newPlanId, currentPlanId))
        return XM_INVALID_CONFIG;

#ifdef CONFIG_SMP_SUPPORT
    SendAllIpi(RESCHED_SYSTEM_IPI_MSG);
#endif
    Schedule();
    return XM_OK;
}

__hypercall xm_s32_t SuspendPartitionSys(xmId_t partitionId) {
    localProcessor_t *info=GET_LOCAL_PROCESSOR();
    xmId_t pId=PartitionId2PId(partitionId);
    ASSERT(!HwIsSti());
    if (partitionId!=GetPartitionCfg(info->sched.cKThread)->rId) {
        if (!(GetPartitionCfg(info->sched.cKThread)->flags&XM_PART_SYSTEM))
            return XM_PERM_ERROR;
        if (pId>=xmcTab.noPartitions)
	    return XM_INVALID_PARAM;
        
#ifdef CONFIG_DEBUG_HYPERCALL
        kprintf("[HYPERCALL] (P%d) Suspended\n", pId);
#endif
        SUSPEND_PARTITION(pId); 
#ifdef CONFIG_SMP_SUPPORT
        xm_s32_t e;
        for (e=0; e<CONFIG_NO_VCPUS; e++) {
            xm_u8_t cpu=xmcVCpuTab[KID2PARTID(pId)*CONFIG_NO_VCPUS+e].cpu;
            if (cpu!=GET_CPU_ID()&&(cpu!=255)) {
                kThread_t *k=partitionTab[KID2PARTID(pId)].kThread[e];
                if (localProcessorInfo[cpu].sched.cKThread==k)
                    SendIpi(cpu, RESCHED_SYSTEM_IPI_MSG);
            }
        }
#endif

        return XM_OK;
    }
#ifdef CONFIG_DEBUG_HYPERCALL
    kprintf("[HYPERCALL] (P%d) Suspended\n", pId);
#endif
    SUSPEND_PARTITION(pId);
    Schedule();
    return XM_OK;
}

__hypercall xm_s32_t ResumePartitionSys(xmId_t partitionId) {
    localProcessor_t *info=GET_LOCAL_PROCESSOR();
    xmId_t pId=PartitionId2PId(partitionId);
    ASSERT(!HwIsSti());
    if (partitionId!=GetPartitionCfg(info->sched.cKThread)->rId) {
        if (!(GetPartitionCfg(info->sched.cKThread)->flags&XM_PART_SYSTEM))
            return XM_PERM_ERROR;
        if (pId>=xmcTab.noPartitions)
	    return XM_INVALID_PARAM;
        
#ifdef CONFIG_DEBUG_HYPERCALL
        kprintf("[HYPERCALL] (P%d) Resumed\n", pId);
#endif
        RESUME_PARTITION(pId);
#ifdef CONFIG_SMP_SUPPORT
        xm_s32_t e;
        for (e=0; e<CONFIG_NO_VCPUS; e++) {
            xm_u8_t cpu=xmcVCpuTab[KID2PARTID(pId)*CONFIG_NO_VCPUS+e].cpu;
            if (cpu!=GET_CPU_ID()&&(cpu!=255)) {
                kThread_t *k=partitionTab[KID2PARTID(pId)].kThread[e];
                if (localProcessorInfo[cpu].sched.cKThread==k)
                    SendIpi(cpu, RESCHED_SYSTEM_IPI_MSG);
            }
        }
#endif
        return XM_OK;
    }
#ifdef CONFIG_DEBUG_HYPERCALL
    kprintf("[HYPERCALL] (P%d) Resumed\n", pId);
#endif
    RESUME_PARTITION(pId);
    Schedule();
    return XM_OK;
}

__hypercall xm_s32_t ResumePartitionImmSys(xmId_t partitionId) {
    localProcessor_t *info=GET_LOCAL_PROCESSOR();
    xmId_t pId=PartitionId2PId(partitionId);
    ASSERT(!HwIsSti());
    if (partitionId!=GetPartitionCfg(info->sched.cKThread)->rId) {
        if (!(GetPartitionCfg(info->sched.cKThread)->flags&XM_PART_SYSTEM))
            return XM_PERM_ERROR;
        if (pId>=xmcTab.noPartitions)
	    return XM_INVALID_PARAM;
#ifdef CONFIG_DEBUG_HYPERCALL
        kprintf("[HYPERCALL] (P%d) Immediately Resumed\n", pId);
#endif
        RESUME_PARTITION(pId);
#ifdef CONFIG_SMP_SUPPORT
        xm_s32_t e;
        for (e=0; e<CONFIG_NO_VCPUS; e++) {
            xm_u8_t cpu=xmcVCpuTab[KID2PARTID(pId)*CONFIG_NO_VCPUS+e].cpu;
            if (cpu!=GET_CPU_ID()&&(cpu!=255)) {
            	xm_s32_t slotTabEntry;
            	struct schedData *cyclic=localProcessorInfo[cpu].sched.data;
                slotTabEntry=cyclic->plan.current->slotsOffset+cyclic->slot;
                if (xmcSchedCyclicSlotTab[slotTabEntry].partitionId == KID2PARTID(pId)){
                	xm_u32_t t;
                	xmTime_t cTime=GetSysClockUsec();
                	t=cTime-cyclic->sExec;
                    if ((t>=xmcSchedCyclicSlotTab[slotTabEntry].sExec)&&(t<=xmcSchedCyclicSlotTab[slotTabEntry].eExec)){
                      cyclic->nextAct=cTime;
                      SendIpi(cpu, RESCHED_SYSTEM_IPI_MSG);
                    }
                }
            }
        }
#endif
        return XM_OK;
    }
#ifdef CONFIG_DEBUG_HYPERCALL
    kprintf("[HYPERCALL] (P%d) Immediately Resumed\n", pId);
#endif
    RESUME_PARTITION(pId);
    Schedule();
    return XM_OK;
}

__hypercall xm_s32_t ResetPartitionSys(xmId_t partitionId, xmAddress_t entryPoint, xm_u32_t resetMode, xm_u32_t status) {
    localProcessor_t *info=GET_LOCAL_PROCESSOR();
    xmId_t pId=PartitionId2PId(partitionId);
    xm_u32_t i;
    ASSERT(!HwIsSti());

    if ((resetMode!=XM_WARM_RESET)&&(resetMode!=XM_COLD_RESET))
        return XM_INVALID_PARAM;
    if (partitionId!=GetPartitionCfg(info->sched.cKThread)->rId) {
        if (!(GetPartitionCfg(info->sched.cKThread)->flags&XM_PART_SYSTEM))
            return XM_PERM_ERROR;
        if (pId>=xmcTab.noPartitions)
	    return XM_INVALID_PARAM;
    }

    for (i=0; i<xmcPartitionTab[pId].noPhysicalMemoryAreas; i++) {
        if ((entryPoint >= xmcPhysMemAreaTab[i+xmcPartitionTab[pId].physicalMemoryAreasOffset].mappedAt) &&
            (entryPoint < (xmcPhysMemAreaTab[i+xmcPartitionTab[pId].physicalMemoryAreasOffset].mappedAt
            + xmcPhysMemAreaTab[i+xmcPartitionTab[pId].physicalMemoryAreasOffset].size)))
            break;
    }

    if ( (i >= xmcPartitionTab[pId].noPhysicalMemoryAreas) ||
        (entryPoint&0x7) )
    {
        entryPoint=xmcBootPartTab[pId].entryPoint;
    }
    else
    {
        xmcBootPartTab[pId].entryPoint=entryPoint;
    }

    HALT_PARTITION(pId);
    ResetPartition(&partitionTab[pId], entryPoint, (resetMode==XM_COLD_RESET)?1:0, status);
#ifdef CONFIG_DEBUG_HYPERCALL
        kprintf("[HYPERCALL] (P%d) Reset\n", pId);
#endif
#ifdef CONFIG_SMP_SUPPORT
    xm_s32_t e;
    for (e=0; e<CONFIG_NO_VCPUS; e++) {
        xm_u8_t cpu=xmcVCpuTab[KID2PARTID(pId)*CONFIG_NO_VCPUS+e].cpu;
        if (cpu!=GET_CPU_ID()&&(cpu!=255)) {
            kThread_t *k=partitionTab[KID2PARTID(pId)].kThread[e];
            if (localProcessorInfo[cpu].sched.cKThread==k)
                SendIpi(cpu, RESCHED_SYSTEM_IPI_MSG);
        }
    }
#endif
    return XM_OK;
}

__hypercall xm_s32_t ShutdownPartitionSys(xmId_t partitionId) {
    localProcessor_t *info=GET_LOCAL_PROCESSOR();
    xmId_t pId=PartitionId2PId(partitionId);
    ASSERT(!HwIsSti());
    if (partitionId!=GetPartitionCfg(info->sched.cKThread)->rId) {
        if (!(GetPartitionCfg(info->sched.cKThread)->flags&XM_PART_SYSTEM))
            return XM_PERM_ERROR;
	if (pId>=xmcTab.noPartitions)
	    return XM_INVALID_PARAM;
    }

#ifdef CONFIG_DEBUG_HYPERCALL
    kprintf("[HYPERCALL] (P%d) Shutdown\n", pId);
#endif
    SHUTDOWN_PARTITION(pId);
#ifdef CONFIG_SMP_SUPPORT
    xm_s32_t e;
    for (e=0; e<CONFIG_NO_VCPUS; e++) {
        xm_u8_t cpu=xmcVCpuTab[KID2PARTID(pId)*CONFIG_NO_VCPUS+e].cpu;
        if (cpu!=GET_CPU_ID()&&(cpu!=255)) {
            kThread_t *k=partitionTab[KID2PARTID(pId)].kThread[e];
            if (localProcessorInfo[cpu].sched.cKThread==k)
                SendIpi(cpu, RESCHED_SYSTEM_IPI_MSG);
        }
    }
#endif
    return XM_OK;
}

__hypercall xm_s32_t IdleSelfSys(void) {
    localProcessor_t *info=GET_LOCAL_PROCESSOR();
    ASSERT(!HwIsSti());
//    ClearKThreadFlags(info->sched.cKThread, KTHREAD_READY_F);
    SchedYield(info, info->sched.idleKThread);
    return XM_OK;
}

#ifdef CONFIG_ARM
__hypercall xm_s32_t SetTimerSys(xm_u32_t clockId, xmTime_t * _interval, xmTime_t * _abstime) {
    xmTime_t abstime, interval;
    abstime = *(_abstime);
    interval = *(_interval);
#else
__hypercall xm_s32_t SetTimerSys(xm_u32_t clockId, xmTime_t abstime, xmTime_t interval) {
#endif
   localProcessor_t *info=GET_LOCAL_PROCESSOR();
    xm_s32_t ret=XM_OK;
    xmTime_t cTime;
    xmTime_t minInterval = info->time.sysHwTimer->GetMinInterval();
    ASSERT(!HwIsSti());
    if ((abstime<0LL)||(interval<0LL)||((interval>0LL)&&(interval<minInterval)))
	return XM_INVALID_PARAM;

    // Disarming a timer
    if (!abstime) {
	switch(clockId) {
	case XM_HW_CLOCK:
	    DisarmKTimer(&info->sched.cKThread->ctrl.g->kTimer);
	    return XM_OK;
	case XM_EXEC_CLOCK:
	    DisarmVTimer(&info->sched.cKThread->ctrl.g->vTimer, &info->sched.cKThread->ctrl.g->vClock);
	    return XM_OK;
	default:
	    return XM_INVALID_PARAM;
	}
    }
    // Arming a timer
    if (interval!=0){
		switch(clockId) {
		case XM_HW_CLOCK:
			cTime=GetSysClockUsec();
		break;
		case XM_EXEC_CLOCK:
			cTime=GetTimeUsecVClock(&info->sched.cKThread->ctrl.g->vClock);
		break;
		default:
		return XM_INVALID_PARAM;
		}
        if ((abstime+interval)<cTime)
            abstime+=interval*((cTime-abstime)/interval);
    }
    switch(clockId) {
    case XM_HW_CLOCK:
	ret=ArmKTimer(&info->sched.cKThread->ctrl.g->kTimer, abstime, interval);
	break;
    case XM_EXEC_CLOCK:
	ret=ArmVTimer(&info->sched.cKThread->ctrl.g->vTimer, &info->sched.cKThread->ctrl.g->vClock, abstime, interval);
	break;
    default:
	return XM_INVALID_PARAM;
    }

    return ret;
}

__hypercall xm_s32_t GetTimeSys(xm_u32_t clockId, xmTime_t *__gParam time) {
    localProcessor_t *info=GET_LOCAL_PROCESSOR();
    	/*SPR-150601-02*/
    if (CheckGParam(time, sizeof(xm_s64_t), 4, PFLAG_RW|PFLAG_NOT_NULL)<0)
        return XM_INVALID_PARAM;

    switch(clockId) {
    case XM_HW_CLOCK:
	*time=GetSysClockUsec();
	break;       
    case XM_EXEC_CLOCK:
	*time=GetTimeUsecVClock(&info->sched.cKThread->ctrl.g->vClock);
	break;
    default:
	return XM_INVALID_PARAM;
    }
    return XM_OK;
}

/*
  __hypercall xm_s32_t SetIrqLevelSys(xm_u32_t level) {
  localSched_t *sched=GET_LOCAL_SCHED();
  ASSERT(!HwIsSti());
    
    
  //info->sched.cKThread->ctrl.g->partCtrlTab->iFlags&=~IFLAGS_IRQ_MASK;
  //info->sched.cKThread->ctrl.g->partCtrlTab->iFlags|=level&IFLAGS_IRQ_MASK;

  return XM_OK;
  }
*/

__hypercall xm_s32_t ClearIrqMaskSys(xm_u32_t * hwIrqsMask, xm_u32_t extIrqsMask) {
    localSched_t * sched =  GET_LOCAL_SCHED();

/*    struct xmcPartition *cfg=GetPartitionCfg(sched->cKThread);

    xm_u32_t unmasked;
    xm_s32_t e, i;
*/
    ASSERT(!HwIsSti());
/*    if(hwIrqsMask)
    {
        for (e=0; e<HWIRQS_VECTOR_SIZE; e++)
            sched->cKThread->ctrl.g->partCtrlTab->hwIrqsMask[e]&=~hwIrqsMask[e];
        for (i=0; i<HWIRQS_VECTOR_SIZE; i++) {
        	unmasked=((cfg->hwIrqs[i])&(hwIrqsMask[i]));
            for (e=0; unmasked; e++)
               if (unmasked&(1<<e)) {
                  HwEnableIrq(e);
                  unmasked&=~(1<<e);
               }
        }
    }
*/
    sched->cKThread->ctrl.g->partCtrlTab->extIrqsMask&=(~extIrqsMask);

    return XM_OK;
}

__hypercall xm_s32_t SetIrqMaskSys(xm_u32_t * hwIrqsMask, xm_u32_t extIrqsMask) {
    localSched_t * sched =  GET_LOCAL_SCHED();
/*    struct xmcPartition *cfg=GetPartitionCfg(sched->cKThread);
    xm_u32_t masked;
    xm_s32_t e, i;
*/
    ASSERT(!HwIsSti());
/*    if (hwIrqsMask){
       for (e=0; e<HWIRQS_VECTOR_SIZE; e++)
          sched->cKThread->ctrl.g->partCtrlTab->hwIrqsMask[e]|=hwIrqsMask[e];
       for (i=0; i<HWIRQS_VECTOR_SIZE; i++) {
          masked=((hwIrqsMask[i])&(cfg->hwIrqs[i]));
          for (e=0; masked; e++)
              if (masked&(1<<e)) {
                 HwDisableIrq(e);
                 masked&=~(1<<e);
              }
       }
    }
*/
    sched->cKThread->ctrl.g->partCtrlTab->extIrqsMask|=extIrqsMask;

    return XM_OK;
}

__hypercall xm_s32_t ForceIrqsSys(xm_u32_t * hwIrqMask, xm_u32_t extIrqMask) {
    localSched_t * sched =  GET_LOCAL_SCHED();
/*    xm_u32_t forced;
    xm_s32_t e, i;
    struct xmcPartition *cfg=GetPartitionCfg(sched->cKThread);
*/
    ASSERT(!HwIsSti());

/*    if (hwIrqMask){
       for (i=0; i<HWIRQS_VECTOR_SIZE; i++) {
           forced=((hwIrqMask[i])&(cfg->hwIrqs[i]));
           hwIrqMask[i]&=~forced;
           for (e=0; forced; e++)
               if (forced&(1<<e)) {
                  HwForceIrq(e);
                  forced&=~(1<<e);
               }
           for (e=0; hwIrqMask[i]; e++)
               if (hwIrqMask[i]&(1<<e)) {
                  sched->cKThread->ctrl.g->partCtrlTab->hwIrqsPend[i]|=(1<<e);
                  hwIrqMask[i]&=~(1<<e);
               }
        }
    }*/
    sched->cKThread->ctrl.g->partCtrlTab->extIrqsPend|=extIrqMask;
    return XM_OK;
}

__hypercall xm_s32_t ClearIrqsSys(xm_u32_t * hwIrqMask, xm_u32_t extIrqMask) {
	/*TOBEAPPROVED ARMPorting: Get directly Sched instead info*/
    localSched_t * sched =  GET_LOCAL_SCHED();
/*    struct xmcPartition *cfg=GetPartitionCfg(sched->cKThread);
    xm_u32_t pending;
    xm_s32_t e,i;
*/
    ASSERT(!HwIsSti());
/*    if (hwIrqMask)
    {
       for (e=0; e<HWIRQS_VECTOR_SIZE; e++)
           sched->cKThread->ctrl.g->partCtrlTab->hwIrqsPend[e]&=~hwIrqMask[e];
       for (i=0; i<HWIRQS_VECTOR_SIZE; i++)
       {
           pending=((hwIrqMask[i])&(cfg->hwIrqs[i]));
           for (e=0; pending; e++)
               if (pending&(1<<e))
               {
                   HwClearIrq(e);
                   pending&=~(1<<e);
               }
       }
    }
*/
    sched->cKThread->ctrl.g->partCtrlTab->extIrqsPend&=(~extIrqMask);
    sched->cKThread->ctrl.g->partCtrlTab->irqIndex = 1023;  // Clear extended irqs
#ifdef CONFIG_ARM
    if(extIrqMask)
    	HwClearIrq(0); //This is not working in Zynq
#endif
    return XM_OK;
}

__hypercall xm_s32_t RouteIrqSys(xm_u32_t type, xm_u32_t irq, xm_u16_t vector) {
	/*TOBEAPPROVED ARMPorting: Get directly Sched instead info*/
    localSched_t * sched =  GET_LOCAL_SCHED();
    ASSERT(!HwIsSti());
    
    if (irq>=32)
        return XM_INVALID_PARAM;
    switch(type) {
    case XM_TRAP_TYPE:
        sched->cKThread->ctrl.g->partCtrlTab->trap2Vector[irq]=vector;
        break;
    case XM_HWIRQ_TYPE:
        sched->cKThread->ctrl.g->partCtrlTab->hwIrq2Vector[irq]=vector;
        break;
    case XM_EXTIRQ_TYPE:
        sched->cKThread->ctrl.g->partCtrlTab->extIrq2Vector[irq]=vector;
        break;
    default:
        return XM_INVALID_PARAM;
    }

    return XM_OK;
}

__hypercall xm_s32_t ReadObjectSys(xmObjDesc_t objDesc, void *__gParam buffer, xmSize_t size, xm_u32_t *__gParam flags) {
    xm_s32_t class;

    ASSERT(!HwIsSti());
    
    class=OBJDESC_GET_CLASS(objDesc);
    if (objectTab[class]&&objectTab[class]->Read)
	return objectTab[class]->Read(objDesc, buffer, size, flags);
    
    return XM_INVALID_PARAM;
}

__hypercall xm_s32_t WriteObjectSys(xmObjDesc_t objDesc, void *__gParam buffer, xmSize_t size, xm_u32_t *__gParam flags) {
    xm_s32_t class;
    ASSERT(!HwIsSti());
    class=OBJDESC_GET_CLASS(objDesc);
    if (objectTab[class]&&objectTab[class]->Write)
	return objectTab[class]->Write(objDesc, buffer, size, flags);
    
    return XM_INVALID_PARAM;
}

__hypercall xm_s32_t SeekObjectSys(xmObjDesc_t objDesc, xmAddress_t offset, xm_u32_t whence) {
    xm_s32_t class;

    ASSERT(!HwIsSti());
    
    class=OBJDESC_GET_CLASS(objDesc);
    if (objectTab[class]&&objectTab[class]->Seek)
	return objectTab[class]->Seek(objDesc, offset, whence);

    return XM_INVALID_PARAM;
}

__hypercall xm_s32_t CtrlObjectSys(xmObjDesc_t objDesc, xm_u32_t cmd, void *__gParam arg) {
    xm_s32_t class;

    ASSERT(!HwIsSti());
    class=OBJDESC_GET_CLASS(objDesc);
    if (objectTab[class]&&objectTab[class]->Ctrl)
	return objectTab[class]->Ctrl(objDesc, cmd, arg);
    return XM_INVALID_PARAM;
}

__hypercall xm_s32_t RaiseIpviSys(xm_u8_t noIpvi) {
    localProcessor_t *info=GET_LOCAL_PROCESSOR();
    struct xmcPartIpvi *ipvi;
    xm_s32_t e, vcpu;
    kThread_t *k;

    ASSERT(!HwIsSti());
    if ((noIpvi<XM_VT_EXT_IPVI0)||(noIpvi>=XM_VT_EXT_IPVI0+CONFIG_MAX_NO_IPVI)) 
	return XM_INVALID_PARAM;
    ipvi=&GetPartitionCfg(info->sched.cKThread)->ipviTab[noIpvi-XM_VT_EXT_IPVI0];
    if (ipvi->noDsts<=0)
        return XM_NO_ACTION;

    for (e=0; e<ipvi->noDsts; e++){
        partition_t *p=&partitionTab[xmcDstIpvi[ipvi->dstOffset+e]];
        SetPartitionExtIrqPending(p, noIpvi);
        for (vcpu=0; vcpu<CONFIG_NO_VCPUS; vcpu++) {
            k=p->kThread[vcpu];
            if (AreKThreadFlagsSet(k, KTHREAD_HALTED_F))
                continue;
#ifdef CONFIG_SMP_SUPPORT
            xm_u8_t cpu=xmcVCpuTab[(KID2PARTID(k->ctrl.g->id)*CONFIG_NO_VCPUS)+KID2VCPUID(k->ctrl.g->id)].cpu;
            if (cpu!=GET_CPU_ID()&&(cpu!=255)) {
                if (localProcessorInfo[cpu].sched.cKThread==k) {
                    SendIpi(cpu, RESCHED_SYSTEM_IPI_MSG);
                }
            }
#endif
            
        }
    }
    return XM_OK;
}

__hypercall xm_s32_t GetGidByName(xm_u8_t *__gParam name, xm_u32_t entity) {
    localProcessor_t *info=GET_LOCAL_PROCESSOR();
    struct xmcPartition *cfg=GetPartitionCfg(info->sched.cKThread);
    xm_s32_t e, id=XM_INVALID_CONFIG;

    if (CheckGParam(name, CONFIG_ID_STRING_LENGTH, 1, PFLAG_NOT_NULL)<0) 
	return XM_INVALID_PARAM;

    switch(entity) {
    case XM_PARTITION_NAME:
        for (e=0; e<xmcTab.noPartitions; e++)
            if (!strncmp(&xmcStringTab[xmcPartitionTab[e].nameOffset], name, CONFIG_ID_STRING_LENGTH))  {
                id=xmcPartitionTab[e].id;
                if (id!=cfg->id)
                    if (!(GetPartitionCfg(info->sched.cKThread)->flags&XM_PART_SYSTEM))
                        return XM_PERM_ERROR;
                break;
            }        
        break;
    case XM_PLAN_NAME:
        for (e=0; e<xmcTab.noSchedCyclicPlans; e++)
            if (!strncmp(&xmcStringTab[xmcSchedCyclicPlanTab[e].nameOffset], name, CONFIG_ID_STRING_LENGTH))  {
                id=xmcSchedCyclicPlanTab[e].id;
                break;
            }
        break;
    default:
        return XM_INVALID_PARAM;
    }
    
    return id;
}

__hypercall xm_s32_t GetVCpuIdSys(void) {
    localProcessor_t *info=GET_LOCAL_PROCESSOR();
    return KID2VCPUID(info->sched.cKThread->ctrl.g->id);
}

__hypercall xm_s32_t HaltVCpuSys(xmId_t vCpuId) {
    localProcessor_t *info=GET_LOCAL_PROCESSOR();
    kThread_t *k;
    
    ASSERT(!HwIsSti());
    if (vCpuId>=CONFIG_NO_VCPUS)
        return XM_INVALID_PARAM;
    k=GetPartition(info->sched.cKThread)->kThread[vCpuId];
    
    HALT_VCPU(KID2PARTID(k->ctrl.g->id), KID2VCPUID(k->ctrl.g->id));
    if (k==info->sched.cKThread)
        Schedule();
#ifdef CONFIG_SMP_SUPPORT
    else {
        xm_u8_t cpu=xmcVCpuTab[CONFIG_NO_VCPUS*KID2PARTID(k->ctrl.g->id)+vCpuId].cpu;
        if (cpu!=GET_CPU_ID()&&(cpu!=255))
            if (localProcessorInfo[cpu].sched.cKThread==k)
                SendIpi(cpu, RESCHED_SYSTEM_IPI_MSG);
    }
#endif
    return XM_OK;
}

__hypercall xm_s32_t ResetVCpuSys(xmId_t vCpuId, xmAddress_t entryPoint) {
    localProcessor_t *info=GET_LOCAL_PROCESSOR();
    partition_t *partition=GetPartition(info->sched.cKThread);

    if (vCpuId>=CONFIG_NO_VCPUS)
        return XM_INVALID_PARAM;

    if (CheckGParam((void *)entryPoint, CONFIG_ID_STRING_LENGTH, 8, PFLAG_NOT_NULL)<0) 
	return XM_INVALID_PARAM;

    HALT_VCPU(partition->cfg->id, vCpuId);
    ResetKThread(partition->kThread[vCpuId], entryPoint);

#ifdef CONFIG_SMP_SUPPORT
    xm_u8_t cpu=xmcVCpuTab[CONFIG_NO_VCPUS*KID2PARTID(partition->kThread[vCpuId]->ctrl.g->id)+vCpuId].cpu;
    if ((cpu!=GET_CPU_ID())&&(cpu!=255))
        if (localProcessorInfo[cpu].sched.cKThread==partition->kThread[vCpuId])
            SendIpi(cpu, RESCHED_SYSTEM_IPI_MSG);
#endif
    return XM_OK;
}

