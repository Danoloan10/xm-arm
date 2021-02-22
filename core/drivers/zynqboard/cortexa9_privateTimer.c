/*
 * $FILE: cortexa9_privateTimer.c
 *
 * board definitions
 *
 * $VERSION$ - 26/08/2014
 *
 * Author: Manuel Muñoz <mmunoz@ai2.upv.es>
 * Author: Javier O. Coronel <jcoronel@fentiss.com>
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE file for the license terms.
 */
/*
 * Changelog:
 * - [11/12/15:XM-ARM-2:SPR-151211-01:#100] Use the clock from CPU_3x2x_CLOCK that is calculated instead PERIPHCLK that is a define.
 * - [16/06/16:CP-160616-01:#113] Configurate the clock prescaled depending on xcf cpu clock value.
 */


#ifdef CONFIG_ZYNQ

#include "drivers/zynqboard/cortexa9_privateTimer.h"

//#include <assert.h>
//#include <kdevice.h>
#include <ktimer.h>
//#include <stdc.h>
#include <processor.h>
#include <arch/irqs.h>
#include <arch/xm_def.h>
#include <arch/gic.h>
#include <local.h>
//#include <spinlock.h>

static hwTimer_t privateTimer;
static timerHandler_t privateHandler;

static void CA9PT_IrqHandler(cpuCtxt_t *ctxt, void *irqData)
{
	WritePrivateTimerReg(CA9_PT1_INT_STATUS, CA9_PT1_INT_STATUS_EVENT_FLAG);
//	HwEnableIrq(CA9_PT1_PPI_ID_PRIVATE_TIMER);

    if (privateHandler)
    	(*privateHandler)();
}

/*static inline xm_s32_t GetTimerIrq(xm_s32_t timer)
{

//#ifdef CONFIG_LEON2
//    return TIMER1_TRAP_NR+(timer-1);
//#else
//    return TIMER1_TRAP_NR+(timer-1)+__GetCpuHwId()*2;
//#endif
}*/

/*static void Get_CA9PT_TimerIrqHandler(cpuCtxt_t *ctxt, void *irqData)
{
    if (privateHandler)
	(*privateHandler)();
    HwEnableIrq(GetTimerIrq(2));
}*/

static xm_s32_t Init_CA9PT_Timer(void)
{
	struct localCpu *cpu=GET_LOCAL_CPU();

    xm_u32_t cpu_3x2x_clockKhz = GetCpuKhz()/2;
    xm_u32_t privateTimerPrescaler = ((((xm_u32_t)(((cpu_3x2x_clockKhz)/(1000))-1))<0xff)?(((xm_u32_t)(((cpu_3x2x_clockKhz)/(1000))-1))):(0xff));

    CA9_PT1_DISABLE_TIMER();
    CA9_PT1_SINGLE_SHOT_MODE();

    CA9_PT1_CONTROL_SET_PSK(privateTimerPrescaler);

    WritePrivateTimerReg(CA9_PT1_COUNTER, 0);

    CA9_PT1_ENABLE_IRQ();
    CA9_PT1_INT_CLEAR_FLAG();

    cpu->globalIrqMask[(CA9_PT1_PPI_ID_PRIVATE_TIMER>>5)]&=~(1<<(CA9_PT1_PPI_ID_PRIVATE_TIMER-(32*(CA9_PT1_PPI_ID_PRIVATE_TIMER>>5))));

    HwEnableIrq(CA9_PT1_PPI_ID_PRIVATE_TIMER);

    SetIrqHandler(CA9_PT1_PPI_ID_PRIVATE_TIMER, CA9PT_IrqHandler, 0);

    privateTimer.freqKhz=(cpu_3x2x_clockKhz/((privateTimerPrescaler+1)));
    /*flags*/
    privateTimer.flags|=HWTIMER_ENABLED;

	return 0;
}

static void Set_CA9PT_Timer(xmTime_t interval)
{
    xm_u32_t l=(xm_u32_t)interval;
    CA9_PT1_DISABLE_TIMER();
//    CA9_PT1_SINGLE_SHOT_MODE();
//    CA9_PT1_ENABLE_IRQ();
    WritePrivateTimerReg(CA9_PT1_COUNTER, l);
//    WriteReg(CA9_PT_BASE_ADDRESS, CA9_PT1_LOAD, 0);
    CA9_PT1_ENABLE_TIMER();
}

static xmTime_t Get_CA9PT_MaxInterval(void)
{
    return 10000000LL; // 10s
//    return 1000000LL; // 1s
}

static xmTime_t Get_CA9PT_MinInterval(void)
{
    return 50LL; // 50usec
}

static timerHandler_t Set_CA9PT_TimerHandler(timerHandler_t TimerHandler)
{
    timerHandler_t OldPrivateUserHandler = privateHandler;

    privateHandler = TimerHandler;
    return OldPrivateUserHandler;
}

static void Shutdown_CA9PT_Timer(void)
{
	CA9_PT1_DISABLE_TIMER();
}

static hwTimer_t privateTimer={
    .name="CortexA9 Private Timer1",
    .flags=HWCLOCK_PER_CPU,
	.freqKhz=(CPU_3x2x_CLOCK/(1000*(CA9_PT1_PRESCALER_VALUE_2_US+1))),
    .InitHwTimer=Init_CA9PT_Timer,
    .SetHwTimer=Set_CA9PT_Timer,
    .GetMaxInterval=Get_CA9PT_MaxInterval,
    .GetMinInterval=Get_CA9PT_MinInterval,
    .SetTimerHandler=Set_CA9PT_TimerHandler,
    .ShutdownHwTimer=Shutdown_CA9PT_Timer,
};

hwTimer_t * sysHwTimer=&privateTimer;
hwTimer_t *GetSysHwTimer(void)
{
    return &privateTimer;
}


#endif /*CONFIG_ZYNQ*/
