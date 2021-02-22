/*
 * $FILE: sched.h
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

#ifndef _XM_SCHED_H_
#define _XM_SCHED_H_

#ifndef _XM_KERNEL_
#error Kernel file, do not include.
#endif

#include <processor.h>
#include <kthread.h>
#include <objects/status.h>
#include <local.h>
#ifdef CONFIG_ZYNQ
#include <drivers/zynqboard/zynq_errorControl.h>
#endif /*CONFIG_ZYNQ*/

extern xm_s32_t nrPartitions;
#ifdef CONFIG_PLAN_EXTSYNC
extern void SchedSyncHandler(cpuCtxt_t *irqCtxt, void *extra);
#endif

extern void InitSched(void);
extern void InitSchedLocal(kThread_t *idle);
extern void Schedule(void);
extern void SetSchedPending(void);
extern xm_s32_t SwitchSchedPlan(xm_s32_t newPlanId, xm_s32_t *oldPlanId);
extern xm_s32_t SwitchSchedPlanImm(xm_s32_t newPlanId, xm_s32_t *oldPlanId);

static inline void SchedYield(localProcessor_t *info, kThread_t *k) {
    info->sched.data->kThread=k;
    Schedule();
}

static inline void DoPreemption(void) {
    localProcessor_t *info=GET_LOCAL_PROCESSOR();
    HwIrqSetMask(info->cpu.globalIrqMask);
    HwSti();
    CpuPowerDown();
    HwCli();
    HwIrqSetMask(info->sched.cKThread->ctrl.irqMask);
}

static inline void PreemptionOn(void) {
#ifdef CONFIG_VOLUNTARY_PREEMPTION
	/*TOBEAPPROVED ARMPorting: Get directly Sched instead info*/
    localSched_t * sched =  GET_LOCAL_SCHED();
    HwIrqGetMask(sched->cKThread->ctrl.irqMask);
    HwIrqSetMask(globalIrqMask);
    HwSti();
#endif
}

static inline void PreemptionOff(void) {
#ifdef CONFIG_VOLUNTARY_PREEMPTION
	/*TOBEAPPROVED ARMPorting: Get directly Sched instead info*/
    localSched_t * sched =  GET_LOCAL_SCHED();
    HwCli();
    HwIrqSetMask(sched->cKThread->ctrl.irqMask);
#endif
}

static inline void HALT_VCPU(xmId_t partId, xmId_t vCpuId) {
    SetKThreadFlags(partitionTab[partId].kThread[vCpuId], KTHREAD_HALTED_F);    
}

static inline void SUSPEND_PARTITION(xmId_t id) {
    xm_s32_t e;
    for (e=0; e<CONFIG_NO_VCPUS; e++)
        SetKThreadFlags(partitionTab[id].kThread[e], KTHREAD_SUSPENDED_F);
}

static inline void RESUME_PARTITION(xmId_t id) {
    xm_s32_t e;
    for (e=0; e<CONFIG_NO_VCPUS; e++)
        ClearKThreadFlags(partitionTab[id].kThread[e], KTHREAD_SUSPENDED_F);
}

static inline void SHUTDOWN_PARTITION(xmId_t id) {
    SetPartitionExtIrqPending(&partitionTab[id], XM_VT_EXT_SHUTDOWN);
}

/*
static inline void IDLE_PARTITION(xmId_t id) {
    xm_s32_t e;
    for (e=0; e<partitionTab[id].cfg->noVCpus; e++)
        ClearKThreadFlags(partitionTab[id].kThread[e], KTHREAD_READY_F);
}
*/
static inline void HALT_PARTITION(xmId_t id) { 
    xm_s32_t e;
    for (e=0; e<CONFIG_NO_VCPUS; e++)
        SetKThreadFlags(partitionTab[id].kThread[e], KTHREAD_HALTED_F);
}

extern inline void MakePlanSwitch(xmTime_t cTime, struct schedData *data);

#endif
