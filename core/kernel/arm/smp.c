/*
 * $FILE: smp.c
 *
 * Symmetric multiprocessor support
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
 * - [21/01/16:XM-ARM-2:SPR-150608-03:#57] Fix SystemReset on SMP.
 */

#include <assert.h>
#include <boot.h>
#include <local.h>
#include <sched.h>
#include <smp.h>
#include <spinlock.h>
#include <arm/asm.h>
#include <arm/gic.h>

static struct ipiMsg {
    spinLock_t lock;
    xm_u32_t msg;
    xm_u32_t bitmap;
} ipiMsg[CONFIG_NO_CPUS];

static void FillIpiMsg(xm_u32_t msg)
{
    xm_u32_t cpuId=GET_CPU_ID();
    SpinLock(&ipiMsg[cpuId].lock);
    ipiMsg[cpuId].msg=msg;
    ipiMsg[cpuId].bitmap=0;
    SpinUnlock(&ipiMsg[cpuId].lock);
}

void SendAllIpi(xm_u32_t msg) {
    FillIpiMsg(msg);
//    XM_SEND_IPI();
    ICDSGIR_SEND_TOOTHERS(XM_IPI_SGI_NUM, 0);
}

void SendIpi(xm_u32_t cpuId, xm_u32_t msg) {
    FillIpiMsg(msg);
//    XM_SEND_IPI();
    ICDSGIR_SEND_TOOTHERS(XM_IPI_SGI_NUM, 0);
}

#ifdef CONFIG_SMP_SUPPORT
extern volatile xm_u32_t resetMask[];
static spinLock_t resetMaskLock = SPINLOCK_INIT;
void ClearResetMask(void) {
	SpinLock(&resetMaskLock);
    resetMask[0] &= ~(1<<GET_CPU_ID());
    SpinUnlock(&resetMaskLock);
}

void SetResetMask(xm_u32_t cpuId) {
    resetMask[0] |= (1<<cpuId);
}
#endif

static void IpiHandler(cpuCtxt_t *ctxt, void *data) {
    extern void start(void);
    extern void _Reset(xmAddress_t);
    extern void SystemReset(void);
    xm_u32_t cpuId=GET_CPU_ID(), e, msg;
    for (e=0; e<CONFIG_NO_CPUS; e++) {
        if (e==cpuId)
            continue;
        if (ipiMsg[e].bitmap&(1<<cpuId))
            continue;
        SpinLock(&ipiMsg[e].lock);
        ipiMsg[e].bitmap|=(1<<cpuId);
        msg=ipiMsg[e].msg;
        SpinUnlock(&ipiMsg[e].lock);
        switch(msg) {
        case HALT_SYSTEM_IPI_MSG:
            HwEndIrq(XM_IPI_SGI_NUM);
            HaltSystem();
            break;
        case WARM_RESET_SYSTEM_IPI_MSG:
            HwEndIrq(XM_IPI_SGI_NUM);
            SystemReset();
            _Reset((xmAddress_t)start);
            break;
        case COLD_RESET_SYSTEM_IPI_MSG:
            HwEndIrq(XM_IPI_SGI_NUM);
            SystemReset();
            _Reset(xmcTab.rsw.entryPoint);
            break;
        case RESCHED_SYSTEM_IPI_MSG:
            HwEndIrq(XM_IPI_SGI_NUM);
            Schedule();
            break;
        }
    }
}

void __VBOOT SetupLocalSmp(void) {
    localProcessor_t *info=GET_LOCAL_PROCESSOR();
    HwEnableIrq(XM_IPI_SGI_NUM);
    (*info->cpu.globalIrqMask)&=~(1<<XM_IPI_SGI_NUM);
    SetIrqHandler(XM_IPI_SGI_NUM, IpiHandler, 0);
}

void __VBOOT SetupSmp(void) {
//	extern xm_u32_t sysResetCounter[];
    localProcessor_t *info=GET_LOCAL_PROCESSOR();
    xm_s32_t cpu;
    memset(ipiMsg, 0, sizeof(struct ipiMsg)*CONFIG_NO_CPUS);
    for (cpu=0; cpu<CONFIG_NO_CPUS; cpu++)
        ipiMsg[cpu].lock=SPINLOCK_INIT;
    (*info->cpu.globalIrqMask)&=~(1<<XM_IPI_SGI_NUM);
    SET_NRCPUS((GET_CPU_NUMBER()<xmcTab.hpv.noCpus)?GET_CPU_NUMBER():xmcTab.hpv.noCpus);

    for (cpu=GET_CPU_ID()+1; cpu<GET_NRCPUS(); cpu++)
    {
//	    if (sysResetCounter[0]==0)
//	    {
	    	ASSERT(GET_CPU_NUMBER()<=2);
	    	SEND_PROCESSOR_EVENT();
//		}
        SetResetMask(cpu);
	}
}
