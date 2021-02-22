/*
 * $FILE: cortexa9_globalTimer.h
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
 */

#ifndef _CORTEXA9_GLOBALTIMER_H__
#define _CORTEXA9_GLOBALTIMER_H__

#ifdef CONFIG_ZYNQ

#include <arch/io.h>
#include <drivers/zynqboard/zynqboard.h>

#define CA9_GT_BASE_ADDRESS			((PERIOHBASE)+(0x0200))

#define SetGlobalTimerReg(Reg, Val)		SetReg(CA9_GT_BASE_ADDRESS, Reg, Val)
#define ClearGlobalTimerReg(Reg, Val)	ClearReg(CA9_GT_BASE_ADDRESS, Reg, Val)
#define ReadGlobalTimerReg(Reg)			ReadReg(CA9_GT_BASE_ADDRESS, Reg)
#define WriteGlobalTimerReg(Reg, Val)	WriteReg(CA9_GT_BASE_ADDRESS, Reg, Val)

#define CA9_GT_COUNTER0					(0x00)
#define CA9_GT_COUNTER1					(0x04)

#define CA9_GT_CONTROL					(0x08)
#define CA9_GT_CONTROL_TIMER_ENABLE			(BIT(0))  /* this bit is NOT banked */
#define CA9_GT_ENABLE_TIMER()					SetGlobalTimerReg(CA9_GT_CONTROL, CA9_GT_CONTROL_TIMER_ENABLE)
#define CA9_GT_DISABLE_TIMER()					ClearGlobalTimerReg(CA9_GT_CONTROL, CA9_GT_CONTROL_TIMER_ENABLE)
#define CA9_GT_CONTROL_COMP_ENABLE			(BIT(1))	/* banked per processor */
#define CA9_GT_COMPARATOR_ENABLED()				SetGlobalTimerReg(CA9_GT_CONTROL, CA9_GT_CONTROL_COMP_ENABLE)
#define CA9_GT_COMPARATOR_DISABLED()			ClearGlobalTimerReg(CA9_GT_CONTROL, CA9_GT_CONTROL_COMP_ENABLE)
#define CA9_GT_CONTROL_IRQ_ENABLE			(BIT(2))	/* banked per processor*/
#define CA9_GT_ENABLE_IRQ()						SetGlobalTimerReg(CA9_GT_CONTROL, CA9_GT_CONTROL_IRQ_ENABLE)
#define CA9_GT_DISABLE_IRQ()					ClearGlobalTimerReg(CA9_GT_CONTROL, CA9_GT_CONTROL_IRQ_ENABLE)
#define CA9_GT_CONTROL_AUTO_INC				(BIT(3))	/* banked per processor*/
#define CA9_GT_SINGLE_SHOT_MODE()				ClearGlobalTimerReg(CA9_GT_CONTROL, CA9_GT_CONTROL_AUTO_INC)
#define CA9_GT_AUTO_RELOAD_MODE()				SetGlobalTimerReg(CA9_GT_CONTROL, CA9_GT_CONTROL_AUTO_INC)
#define CA9_GT_CONTROL_VAL_TO_PSK(X)		((X<<8)&0x0000ff00)
#define CA9_GT_CONTROL_VAL_FROM_PSK(X)		(((X)&0x0000ff00)>>8)
#define CA9_GT_CONTROL_SET_PSK(X)				WriteGlobalTimerReg(CA9_GT_CONTROL, CA9_GT_CONTROL_VAL_TO_PSK(X))
#define CA9_GT_CONTROL_GET_PSK()				CA9_GT_CONTROL_VAL_FROM_PSK(ReadGlobalTimerReg(CA9_GT_CONTROL))

#define CA9_GT_INT_STATUS				(0x0c)
#define CA9_GT_INT_STATUS_EVENT_FLAG		BIT(0)
#define CA9_GT_INT_GET_FLAG()					ReadGlobalTimerReg(CA9_GT_INT_STATUS)
#define CA9_GT_INT_CLEAR_FLAG()					WriteGlobalTimerReg(CA9_GT_INT_STATUS, CA9_GT_INT_STATUS_EVENT_FLAG)

#define CA9_GT_COMP0					(0x10)
#define CA9_GT_COMP1					(0x14)
#define CA9_GT_AUTO_INC					(0x18)

#define CA9_GT_PRESCALER_VALUE_2_1US		((((xm_u32_t)(((CPU_3x2x_CLOCK)/(1000000.0))-1))<0xff)?(((xm_u32_t)(((CPU_3x2x_CLOCK)/(1000000.0))-1))):(0xff))

#define CA9_GT_PPI_ID_GLOBAL_TIMER		(27)

#endif /*CONFIG_ZYNQ*/

#endif /*__CORTEXA9_GLOBALTIMER_H__*/
