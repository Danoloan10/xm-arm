/*
 * $FILE: cortexa9_privateTimer.h
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
 */

#ifndef _CORTEXA9_PRIVATETIMER_H__
#define _CORTEXA9_PRIVATETIMER_H__

#ifdef CONFIG_ZYNQ

#include <arch/io.h>
#include <drivers/zynqboard/zynqboard.h>

/****** TIMER1 *******/
#define CA9_PT_BASE_ADDRESS				((PERIOHBASE)+(0x0600))

#define SetPrivateTimerReg(Reg, Val)	SetReg(CA9_PT_BASE_ADDRESS, Reg, Val)
#define ClearPrivateTimerReg(Reg, Val)	ClearReg(CA9_PT_BASE_ADDRESS, Reg, Val)
#define ReadPrivateTimerReg(Reg)		ReadReg(CA9_PT_BASE_ADDRESS, Reg)
#define WritePrivateTimerReg(Reg, Val)	WriteReg(CA9_PT_BASE_ADDRESS, Reg, Val)

#define CA9_PT1_LOAD					(0x00)	/*Private Timer1 Load Register*/

#define CA9_PT1_COUNTER					(0x04)	/*Private Timer1 Counter Register*/

#define CA9_PT1_CONTROL					(0x08)	/*Private Timer1 Control Register*/
#define CA9_PT1_CONTROL_TIMER_ENABLE		BIT(0)
#define CA9_PT1_ENABLE_TIMER()					SetPrivateTimerReg(CA9_PT1_CONTROL, CA9_PT1_CONTROL_TIMER_ENABLE)
#define CA9_PT1_DISABLE_TIMER()					ClearPrivateTimerReg(CA9_PT1_CONTROL, CA9_PT1_CONTROL_TIMER_ENABLE)
#define CA9_PT1_CONTROL_AUTO_LOAD			BIT(1)
#define CA9_PT1_SINGLE_SHOT_MODE()				ClearPrivateTimerReg(CA9_PT1_CONTROL, CA9_PT1_CONTROL_AUTO_LOAD)
#define CA9_PT1_AUTO_RELOAD_MODE()				SetPrivateTimerReg(CA9_PT1_CONTROL, CA9_PT1_CONTROL_AUTO_LOAD)
#define CA9_PT1_CONTROL_IRQ_ENABLE			BIT(2)
#define CA9_PT1_ENABLE_IRQ()					SetPrivateTimerReg(CA9_PT1_CONTROL, CA9_PT1_CONTROL_IRQ_ENABLE)
#define CA9_PT1_DISABLE_IRQ()					ClearPrivateTimerReg(CA9_PT1_CONTROL, CA9_PT1_CONTROL_IRQ_ENABLE)
#define CA9_PT1_CONTROL_VAL_TO_PSK(X)		((X<<8)&0x0000ff00)
#define CA9_PT1_CONTROL_VAL_FROM_PSK(X)		(((X)&0x0000ff00)>>8)
#define CA9_PT1_CONTROL_SET_PSK(X)				WritePrivateTimerReg(CA9_PT1_CONTROL, CA9_PT1_CONTROL_VAL_TO_PSK(X))
#define CA9_PT1_CONTROL_GET_PSK()				CA9_PT1_CONTROL_VAL_FROM_PSK(ReadPrivateTimerReg(CA9_PT1_CONTROL))


#define CA9_PT1_INT_STATUS				(0x0c)	/*Private Timer1 Interrupt Status Register*/
#define CA9_PT1_INT_STATUS_EVENT_FLAG		BIT(0)
#define CA9_PT1_INT_GET_FLAG()					ReadPrivateTimerReg(CA9_PT1_INT_STATUS)
#define CA9_PT1_INT_CLEAR_FLAG()					WritePrivateTimerReg(CA9_PT1_INT_STATUS, CA9_PT1_INT_STATUS_EVENT_FLAG)

#define CA9_PT1_PRESCALER_VALUE_2_US		((((xm_u32_t)(((CPU_3x2x_CLOCK)/(1000000.0))-1))<0xff)?(((xm_u32_t)(((CPU_3x2x_CLOCK)/(1000000.0))-1))):(0xff))

/****** WATCHDOG_TIMER2 *******/
#define CA9_WDPT2_LOAD						(0x20)	/*Private WatchDog/Timer2 Load Register*/

#define CA9_WDPT2_COUNTER					(0x24)	/*Private WatchDog/Timer2 Counter Register*/

#define CA9_WDPT2_CONTROL					(0x28)	/*Private WatchDog/Timer2 Control Register*/
#define CA9_WDPT2_CONTROL_WDTMR_ENABLE			BIT(0)
#define CA9_WDPT2_ENABLE_WDTMR()					SetPrivateTimerReg(CA9_WDPT2_CONTROL, CA9_WDPT2_CONTROL_WDTMR_ENABLE)
#define CA9_WDPT2_DISABLE_WDTMR()					ClearPrivateTimerReg(CA9_WDPT2_CONTROL, CA9_WDPT2_CONTROL_WDTMR_ENABLE)
#define CA9_WDPT2_CONTROL_AUTO_LOAD				BIT(1)
#define CA9_WDPT2_SINGLE_SHOT_MODE()				ClearPrivateTimerReg(CA9_WDPT2_CONTROL, CA9_WDPT2_CONTROL_AUTO_LOAD)
#define CA9_WDPT2_AUTO_RELOAD_MODE()				SetPrivateTimerReg(CA9_WDPT2_CONTROL, CA9_WDPT2_CONTROL_AUTO_LOAD)
#define CA9_WDPT2_CONTROL_IRQ_ENABLE			BIT(2)
#define CA9_WDPT2_ENABLE_IRQ()						SetPrivateTimerReg(CA9_WDPT2_CONTROL, CA9_WDPT2_CONTROL_IRQ_ENABLE)
#define CA9_WDPT2_DISABLE_IRQ()						ClearPrivateTimerReg(CA9_WDPT2_CONTROL, CA9_WDPT2_CONTROL_IRQ_ENABLE)
#define CA9_WDPT2_CONTROL_WATCHDOG_TIMER_MODE	BIT(3)
#define CA9_WDPT2_TIMER_MODE()					ClearPrivateTimerReg(CA9_WDPT2_CONTROL, CA9_WDPT2_CONTROL_WATCHDOG_TIMER_MODE)
#define CA9_WDPT2_WATCHDOG_MODE()				SetPrivateTimerReg(CA9_WDPT2_CONTROL, CA9_WDPT2_CONTROL_WATCHDOG_TIMER_MODE)
#define CA9_WDPT2_CONTROL_VAL_TO_PSK(X)		((X<<8)&0x0000ff00)
#define CA9_WDPT2_CONTROL_VAL_FROM_PSK(X)		(((X)&0x0000ff00)>>8)
#define CA9_WDPT2_CONTROL_SET_PSK(X)				WritePrivateTimerReg(CA9_WDPT2_CONTROL, CA9_WDPT2_CONTROL_VAL_TO_PSK(X))
#define CA9_WDPT2_CONTROL_GET_PSK()				CA9_WDPT2_CONTROL_VAL_FROM_PSK(ReadPrivateTimerReg(CA9_WDPT2_CONTROL))

#define CA9_WDPT2_INT_STATUS				(0x2c)	/*Private WatchDog/Timer2 Interrupt Status Register*/
#define CA9_WDPT2_INT_STATUS_EVENT_FLAG		BIT(0)
#define CA9_WDPT2_INT_GET_FLAG()					ReadPrivateTimerReg(CA9_WDPT2_INT_STATUS)
#define CA9_WDPT2_INT_CLEAR_FLAG()					WritePrivateTimerReg(CA9_WDPT2_INT_STATUS, 0x1)

#define CA9_WDPT2_RESET_STATUS				(0x30)	/*Private WatchDog/Timer2 Reset Status Register*/
#define CA9_WDPT2_RESET_STATUS_EVENT_FLAG		BIT(0)
#define CA9_WDPT2_RESET_GET_FLAG()					ReadPrivateTimerReg(CA9_WDPT2_RESET_STATUS)
#define CA9_WDPT2_RESET_CLEAR_FLAG()				WritePrivateTimerReg(CA9_WDPT2_RESET_STATUS, 0x1)

#define CA9_WDPT2_WATCHDOG_DISABLE			(0x34)	/*Private WatchDog/Timer2 Watchdog Disable Register*/
#define CA9_WDPT2_WATCHDOG_DISABLE_W1			(0x12345678)	/*Private WatchDog/Timer2 Watchdog Disable Value1*/
#define CA9_WDPT2_WATCHDOG_DISABLE_W2			(0x87654321)	/*Private WatchDog/Timer2 Watchdog Disable Value2*/
#define CA9_WDPT2_DISABLE_WATCHDOG_MODE()			WritePrivateTimerReg(CA9_WDPT2_WATCHDOG_DISABLE, CA9_WDPT2_WATCHDOG_DISABLE_W1); WritePrivateTimerReg(CA9_WDPT2_WATCHDOG_DISABLE, CA9_WDPT2_WATCHDOG_DISABLE_W2)

#define CA9_WDPT2_PRESCALER_VALUE_2_1US		((((xm_u32_t)(((CPU_3x2x_CLOCK)/(1000000.0))-1))<0xff)?(((xm_u32_t)(((CPU_3x2x_CLOCK)/(1000000.0))-1))):(0xff))

#define CA9_PT1_PPI_ID_PRIVATE_TIMER	(29)

#endif /*CONFIG_ZYNQ*/

#endif /*__CORTEXA9_PRIVATETIMER_H__*/
