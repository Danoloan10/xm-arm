/*
 * $FILE: cortexa9_globalTimer.c
 *
 * board definitions
 *
 * $VERSION$ - 22/08/2014
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
 * - [16/06/16:CP-160616-01:#113] Configurate the clock prescaler depending on xcf cpu clock value.
 */

#ifdef CONFIG_ZYNQ

#include "drivers/zynqboard/cortexa9_globalTimer.h"
#include <processor.h>
#include <ktimer.h>
#include <arch/xm_def.h>

#include <spinlock.h>

static hwClock_t globalClock;
static spinLock_t spinlock=SPINLOCK_INIT;

static xm_s32_t Init_CA9GT_Clock(void)
{
    xm_u32_t cpu_3x2x_clockKhz = GetCpuKhz()/2;
    xm_u32_t globalClockPrescaler = ((((xm_u32_t)(((cpu_3x2x_clockKhz)/(1000))-1))<0xff)?(((xm_u32_t)(((cpu_3x2x_clockKhz)/(1000))-1))):(0xff));

	CA9_GT_CONTROL_SET_PSK(globalClockPrescaler);
	CA9_GT_COMPARATOR_DISABLED();
	CA9_GT_DISABLE_IRQ();
	WriteGlobalTimerReg(CA9_GT_COUNTER0,0);
	WriteGlobalTimerReg(CA9_GT_COUNTER1,0);
	CA9_GT_ENABLE_TIMER();

	globalClock.freqKhz = (cpu_3x2x_clockKhz/((globalClockPrescaler+1)));
	globalClock.flags |= HWCLOCK_ENABLED;

	return 1;
}

static hwTime_t Read_CA9GT_Clock(void)
{
	hwTime_t cTime;
	xm_u32_t low32, up32, last_up32;

	if (!(globalClock.flags&HWCLOCK_ENABLED))
	{
		//eprintf("ARMPorting:\tTimer disabled\n");
		return 0;
	}

	SpinLock(&spinlock);

	up32 = ReadGlobalTimerReg(CA9_GT_COUNTER1);
	do {
		last_up32 = up32;
		low32 = ReadGlobalTimerReg(CA9_GT_COUNTER0);
		up32 = ReadGlobalTimerReg(CA9_GT_COUNTER1);
	} while (up32 != last_up32);

	cTime = up32;
	cTime <<= 32;
	cTime |= low32;
	SpinUnlock(&spinlock);
	return cTime;
}

static void Shutdown_CA9GT_Clock(void)
{
	CA9_GT_DISABLE_TIMER();
}

static hwClock_t globalClock={
		.name="CortexA9 Global Clock",
		.flags=0,
		.freqKhz=(CPU_3x2x_CLOCK/(1000*(CA9_GT_PRESCALER_VALUE_2_1US+1))),
		.InitClock=Init_CA9GT_Clock,
		.GetTimeUsec=Read_CA9GT_Clock,
		.ShutdownClock=Shutdown_CA9GT_Clock,
};

hwClock_t *sysHwClock=&globalClock;
#endif /*CONFIG_ZYNQ*/
