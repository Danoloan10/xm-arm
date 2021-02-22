/*
 * $FILE: gic.h
 *
 * board definitions
 *
 * $VERSION$ - 13/08/2014
 *
 * Author: Manuel Muñoz <mmunoz@ai2.upv.es>
 * Author: Javier O. Coronel <jcoronel@fentiss.com>
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE file for the license terms.
 */

#ifndef _CORE_INCLUDE_ARM_GIC_H__
#define _CORE_INCLUDE_ARM_GIC_H__

#ifdef CONFIG_ARM

#ifdef _XM_KERNEL_
#include <arch/io.h>
#include <arch/processor.h>
#ifdef CONFIG_ZYNQ
#	include <drivers/zynqboard/zynqboard.h>
#endif /*CONFIG_ZYNQ*/
#else
#include <xm_inc/arch/io.h>
#include <xm_inc/arch/processor.h>
#ifdef CONFIG_ZYNQ
#	include <xm_inc/drivers/zynqboard/zynqboard.h>
#endif /*CONFIG_ZYNQ*/
#endif

/*************************
 * Distributor registers *
 *************************/

#define	ICD_OFFSET						(0x00001000)
#define ICD_BASE_ADDRESS				((PERIOHBASE) + (ICD_OFFSET))

//#define GET_ADDRES(BASE, N, X)			((BASE)+((xm_u32_t)(((N*X)/32))))
//#define GET_BITMASK(N,X,VAL)		(VAL<<(X%N))

//#define WRITE_IN_GAP(BASE, OFFSET, N, X, VAL)	SetReg(BASE, GET_ADDRES(OFFSET, N, X), GETBITMASK(N,X,VAL))
//#define READ_FROM_GAP(BASE, OFFSET, N, X, VAL)	SetReg(BASE, GET_ADDRES(OFFSET, N, X), GETBITMASK(N,X,VAL))

#define ICDDCR					(0x00000000)	/*Distributor Control Register*/
#define ICDDCR_ENA_MASK				(BIT(0))
#define ICDDCR_DIS_MASK				(0)
#define ICDDCR_ENABLE()				WriteReg(ICD_BASE_ADDRESS, ICDDCR, ICDDCR_ENA_MASK) /*The GIC monitors the peripheral interrupt signals, and forwards pending interrupts to the CPU interfaces.*/
#define ICDDCR_DISABLE()			WriteReg(ICD_BASE_ADDRESS, ICDDCR, ICDDCR_DIS_MASK) /*The GIC ignores all peripheral interrupt signals, and does not forward pending interrupts to the CPU interfaces.*/

#define ICDICTR					(0x00000004)	/*Interrupt Controller Type Register*/
#define ICDICTR_IT_LINES_NUMBER_MASK		(0x0000001F)
#define ICD_GET_INT_LINES_NUMBER()	(ReadReg(  ICD_BASE_ADDRESS, ICDICTR) & (ICDICTR_IT_LINES_NUMBER_MASK))

#define ICDISRn					(0x00000080)	/*Interrupt Security*/
#define ICDISR_ADDRESS(X)			((ICDISRn)+(4*((xm_u32_t)((X/32)))))
#define ICDISR_BITMASK(Y)			(BIT(Y%32))
#define ICDISR_SECURE(X)			ClearReg(ICD_BASE_ADDRESS, ICDISR_ADDRESS(X), ICDISR_BITMASK(X))
#define ICDISR_NONSECURE(X)			SetReg(  ICD_BASE_ADDRESS, ICDISR_ADDRESS(X), ICDISR_BITMASK(X))
#define ICDISD_SEC					(10)
#define ICDISD_NONSEC				(11)

#define ICDISERn				(0x00000100)	/*Interrupt Set-Enable Register*/
#define ICDISER_ABS_ADDRESS     (ICD_BASE_ADDRESS+ICDISERn)    /*Interrupt Set-Enable Register*/
#define ICDISER_ADDRESS(X)			((ICDISERn)+(4*((xm_u32_t)((X/32)))))
#define ICDISER_BITMASK(Y)			(BIT(Y%32))

#define ICDICERn				(0x00000180)	/*Interrupt Clear-Enable Register*/
#define ICDICER_ABS_ADDRESS     (ICD_BASE_ADDRESS+ICDICERn)    /*Interrupt Clear-Enable Register*/
#define ICDICER_ADDRESS(X)			((ICDICERn)+(4*((xm_u32_t)((X/32)))))
#define ICDISER_ENABLE(X)			WriteReg(ICD_BASE_ADDRESS, ICDISER_ADDRESS(X), ICDISER_BITMASK(X))
#define ICDISER_DISABLE(X)			WriteReg(ICD_BASE_ADDRESS, ICDICER_ADDRESS(X), ICDISER_BITMASK(X))
#define ICDISER_ISENABLED(X)		((ReadReg(ICD_BASE_ADDRESS, ICDISER_ADDRESS(X)) & ICDISER_BITMASK(X)) && 1)
#define ICDISER_WHOISENABLED(X)		(ReadReg(ICD_BASE_ADDRESS, ICDISER_ADDRESS(X)))


#define ICDIPRn					(0x00000400)	/*Interrupt Priority Registers*/
#define ICDIPR_ADDRESS(X)			((ICDIPRn)+(4*((xm_u32_t)((X/4)))))
#define ICDIPR_MASK(Y)				(0xff<<(Y%4))
#define ICDIPR_BITMASK(Y,PRIO)		(PRIO<<(Y%4))
#define ICDIPR_SETPRIO(Z,PR)		ClearNSetReg(ICD_BASE_ADDRESS, ICDIPR_ADDRESS(Z), ICDIPR_MASK(Z), ICDIPR_BITMASK(Z,PR))

#define ICDIPTRn				(0x00000800)	/*Interrupt CPU Target Registers*/
#define ICDIPTR_ADDRESS(X)			((ICDIPTRn)+(4*((xm_u32_t)((X/4)))))
#define ICDIPTR_MASK(Y)				(0xff<<(Y%4))
#define ICDIPTR_BITMASK(Y,CPU)		((BIT(CPU))<<(Y%4))
#define ICDIPTR_SET_TARGETCPU(IRQ,CPU)		ClearNSetReg(ICD_BASE_ADDRESS, ICDIPTR_ADDRESS(IRQ), ICDIPTR_MASK(IRQ), ICDIPTR_BITMASK(IRQ,CPU))
#define ICDIPTR_ADD_TARGETCPU(IRQ,CPU)		SetReg(ICD_BASE_ADDRESS, ICDIPTR_ADDRESS(IRQ), ICDIPTR_BITMASK(IRQ,CPU))
#define ICDIPTR_CLEAR_TARGETCPU(IRQ,CPU)	ClearReg(ICD_BASE_ADDRESS, ICDIPTR_ADDRESS(IRQ), ICDIPTR_BITMASK(IRQ,CPU))

#define ICDICFRn				(0x00000c00)	/*Interrupt Configuration Register*/
#define ICDICFR_ADDRESS(X)			((ICDICFRn)+(4*((xm_u32_t)((X/16)))))
#define ICDICFR_BITMASK(Y)			(BIT((Y%16)+1))
#define ICDICFR_EDGE(X)				SetReg(  ICD_BASE_ADDRESS, ICDICFR_ADDRESS(X), ICDICFR_BITMASK(X))
#define ICDICFR_LEVEL(X)			ClearReg(ICD_BASE_ADDRESS, ICDICFR_ADDRESS(X), ICDICFR_BITMASK(X))

#define ICDISPRn				(0x00000200)	/*Interrupt Set-Pending Register*/
#define ICDISPR_ADDRESS(X)			((ICDISPRn)+(4*((xm_u32_t)((X/32)))))
#define ICDISPR_BITMASK(Y)			(BIT(Y%32))
#define ICDISPR_SETPENDING(X)			WriteReg(		ICD_BASE_ADDRESS, ICDISPR_ADDRESS(X), ICDISPR_BITMASK(X))
#define ICDISPR_ISPENDING(X)			((ReadReg(	ICD_BASE_ADDRESS, ICDISPR_ADDRESS(X)) & ICDISPR_BITMASK(X))&&1)
#define ICDISPR_WHOISPENDING(X)			(ReadReg(	ICD_BASE_ADDRESS, ICDISPR_ADDRESS(X)))

#define ICDICPRn                (0x00000280)    /*Interrupt Clear-Pending Register*/
#define ICDICPR_ABS_ADDRESS         (ICD_BASE_ADDRESS+ICDICPRn)
#define ICDICPR_ADDRESS(X)			((ICDICPRn)+(4*((xm_u32_t)((X/32)))))
#define ICDICPR_BITMASK(Y)			(BIT(Y%32))
#define ICDICPR_CLEARPENDING(X)			WriteReg(		ICD_BASE_ADDRESS, ICDICPR_ADDRESS(X), ICDICPR_BITMASK(X))
//#define ICDICPR_ISPENDING(X)			(~(ReadReg(	ICD_BASE_ADDRESS, ICDICPR_ADDRESS(X), ICDICPR_BITMASK(X)) && ICDICPR_BITMASK(X)))

#define ICDSGIR					(0x00000f00)	/*Software Generated Interrupt Register*/
//#define ICDSGIR_SEND_ITSELF_MASK	(BIT(25))
//#define ICDSGIR_SEND_OTHERS_MASK	(BIT(24))
//#define ICDSGIR_SEND_LIST_MASK	(BIT(24))
//#define ICDSGIR_SEND_TO_NONSEC_MASK	(BIT(15))
//#define ICDSGIR_SEND_TO_SEC_MASK	(0)
//#define ICDSGIR_SEND_ITSELF(sgi,sec)	WriteReg(ICD_BASE_ADDRESS, ICDSGIR, ((sgi)|(sec)|ICDSGIR_SEND_ITSELF_MASK) )

//#define ICDSGIR_SEND_TOOTHERS(sgi,sec)	WriteReg(ICD_BASE_ADDRESS, ICDSGIR, ((sgi)|(sec)|ICDSGIR_SEND_OTHERS_MASK) )

//#define ICDSGIR_SEND_ITSELF(sgi,sec)	WriteReg( ICD_BASE_ADDRESS, ICDSGIR, ((sgi)|(sec)|ICDSGIR_SEND_ITSELF_MASK/*|(1<<16)*/) )
//#define ICDSGIR_SEND_ITSELF_NONSEC(sgi)	ICDSGIR_SEND_ITSELF(sgi, ICDSGIR_SEND_TO_NONSEC_MASK)
//#define ICDSGIR_SEND_ITSELF_SEC(sgi)	ICDSGIR_SEND_ITSELF(sgi, ICDSGIR_SEND_TO_SEC_MASK)
//#define SEND_SGI(sgi)					ICDSGIR_SEND_ITSELF_SEC(sgi)

#define ICDSGIR_ID_MASK					(0x0000000F)
#define ICDSGIR_SATT_MASK				(0x00008000)
#define 	ICDSGIR_SATT_SECURE				(0x00000000)
#define 	ICDSGIR_SATT_NONSECURE			(ICDSGIR_SATT_MASK)
#define ICDSGIR_TARGET_LIST_MASK		(0x00FF0000)
#define ICDSGIR_TARGET_FILTER_MASK	 	(0x03000000)
#define 	ICDSGIR_TARGET_FILTER_LIST		(0x00000000)
#define 	ICDSGIR_TARGET_FILTER_OTHER		(0x01000000)
#define 	ICDSGIR_TARGET_FILTER_ITSELF	(0x02000000)
#define 	ICDSGIR_TARGET LIST(list)		((list<<16)|ICDSGIR_TARGET_FILTER_LIST)

#define ICDSGIR_SEND_ITSELF(sgi,sec)	WriteReg(ICD_BASE_ADDRESS, ICDSGIR, ((sgi)|(sec)|ICDSGIR_TARGET_FILTER_ITSELF) )
#define ICDSGIR_SEND_TOOTHERS(sgi,sec)	WriteReg(ICD_BASE_ADDRESS, ICDSGIR, ((sgi)|(sec)|ICDSGIR_TARGET_FILTER_OTHER) )

#define ICDABRn					(0x00000300)	/*Active bit Register*/

/*************************************
 * Interrupt Controler CPU Interface *
 *************************************/

#define	ICC_OFFSET						(0x00000100)
#define ICC_BASE_ADDRESS				((PERIOHBASE) + (ICC_OFFSET))

#define ICCICR					(0x00000000)		/*CPU interface control*/
#define ICCICR_SECURE_MASK				(BIT(0))
#define ICCICR_NONSECURE_MASK			(BIT(1))
#define ICCICR_ACK_CTL_MASK				(BIT(2))
#define ICCICR_FIQ_EN_MASK				(BIT(3))
#define ICCICR_SBPR_MASK				(BIT(4))
#define ICCICR_ENABLE_SECURE()				SetReg(ICC_BASE_ADDRESS, ICCICR, ICCICR_SECURE_MASK)
#define ICCICR_DISABLE_SECURE()				ClearReg(ICC_BASE_ADDRESS, ICCICR, ICCICR_SECURE_MASK)
#define ICCICR_ENABLE_NONSECURE()			SetReg(ICC_BASE_ADDRESS, ICCICR, ICCICR_NONSECURE_MASK)
#define ICCICR_DISABLE_NONSECURE()			ClearReg(ICC_BASE_ADDRESS, ICCICR, ICCICR_NONSECURE_MASK)
#define ICCICR_ENABLE_SECURE_AS_FIQ()		SetReg(ICC_BASE_ADDRESS, ICCICR, ICCICR_FIQ_EN_MASK)
#define ICCICR_DISABLE_SECURE_AS_FIQ()		ClearReg(ICC_BASE_ADDRESS, ICCICR, ICCICR_FIQ_EN_MASK)
#define ICCICR_ENABLE_SECURE_ACK()			SetReg(ICC_BASE_ADDRESS, ICCICR, ICCICR_ACK_CTL_MASK)
#define ICCICR_DISABLE_SECURE_ACK()			ClearReg(ICC_BASE_ADDRESS, ICCICR, ICCICR_ACK_CTL_MASK)

#define ICCPMR					(0x00000004)		/*Interrupt priority mask*/
#define ICCPMR_MASK						(0x000000ff)
#define ICC_SETPRIORITYMASK(Pr)			WriteReg(ICC_BASE_ADDRESS, ICCPMR, Pr)

#define ICCBPR					(0x00000008)		/*Binary point*/
#define ICCABPR					(0x0000001C)		/*Aliased non-secure binary point*/

#define ICCIAR					(0x0000000C)		/*Interrupt acknowledge*/
#define ICCIAR_ADDRESS					((ICC_BASE_ADDRESS)+(ICCIAR))
#define ICCIAR_IRQMASK					(0x000003ff)
#define ICCIAR_CPUMASK					(0x00001c00)

#define ICCEOIR					(0x00000010)		/*End of interrupt*/
#define ICCEOIR_ADDRESS                  ((ICC_BASE_ADDRESS)+(ICCEOIR))


#define ICCRPR					(0x00000014)		/*Running Priority*/
#define ICC_GETRUNNINGPRIORITY()			ReadReg(ICC_BASE_ADDRESS, ICCRPR)

#define ICCHPIR					(0x00000018)		/*Highest Pending Interrupt*/
#define ICC_GETHIESTPENDINGIRQ()					ReadReg(ICC_BASE_ADDRESS, ICCHPIR)

#define IRQINDEX				(ICC_BASE_ADDRESS + ICCIAR)

#define GIC_NS_SGI_FIRST_ID		(0)
#define GIC_NS_SGI_LAST_ID		(7)
#define GIC_S_SGI_FIRST_ID		(8)
#define GIC_S_SGI_LAST_ID		(15)
#define GIC_SGI_FIRST_ID		(GIC_NS_SGI_FIRST_ID)
#define GIC_SGI_LAST_ID			(GIC_S_SGI_LAST_ID)
#define GIC_PPI_FIRST_ID		(16)
#define GIC_PPI_LAST_ID			(31)
#define GIC_SPI_FIRST_ID		(32)
#define GIC_SPI_LAST_ID			(1023)

#define GIC_ID_GLOBAL_TIMER		(27)
#define GIC_ID_LEGACY_nFIQ		(28)
#define GIC_ID_PRIVATE_TIMER	(29)
#define GIC_ID_WATCHDOG_TIMER	(30)
#define GIC_ID_LEGACY_NIRQ		(31)
#define GIC_ID_SHARED_PERIPH	(32)

#define GIC_SPI_STAT_0			(0x00001D04)	/*Returns the status of the IRQ ID32 to ID63.*/
#define GIC_SPI_STAT_1			(0x00001D08)	/*Returns the status of the IRQ ID64 to ID95.*/
#define GIC_SPI_STAT_L1CACHE_TLB_CPU0_MASK	(BIT(0))
#define GIC_SPI_STAT_L1CACHE_TLB_CPU1_MASK	(BIT(1))
#define GIC_SPI_STAT_L2CACHE_MASK			(BIT(2))
#define GIC_SPI_STAT_SCU_MASK				(BIT(28))
#define GET_GIC_SPI_STAT_0()				ReadReg(PERIOHBASE, GIC_SPI_STAT_0)
#define GET_GIC_SPI_STAT_1()				ReadReg(PERIOHBASE, GIC_SPI_STAT_1)

#ifndef __ASSEMBLY__
    extern void InitGIC(xm_u32_t);
#endif /* __ASSEMBLY__*/

#else
#error "CONFIG_ARM not defined or arch file included improperly"
#endif /*__CONFIG_ARM__*/

#endif /*_CORE_INCLUDE_ARM_GIC_H__*/
