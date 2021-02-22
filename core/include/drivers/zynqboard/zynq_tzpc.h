/*
 * $FILE: zynq_tzpc.h
 *
 * board definitions
 *
 * $VERSION$ - 14/10/2014
 *
 * Author: Manuel Muñoz <mmunoz@ai2.upv.es>
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE file for the license terms.
 */
/*
 * Changelog:
 * - [08/09/16:XM-ARM-2:SPR-160908-01:#123] Add tag to avoid magic numbers.
 */

#ifndef _CORE_INCLUDE_DRIVERS_ZYNQ_ZYNQ_TZPC_H__
#define _CORE_INCLUDE_DRIVERS_ZYNQ_ZYNQ_TZPC_H__

#ifdef CONFIG_ARM

#include <arch/io.h>
#include <arch/processor.h>

#define CA9_TZPC_TZPCR0SIZE			(0x000)  /*R/W	Secure RAM Region Size Register.		Reset:0x00000200*/
#define CA9_TZPC_TZPCR0SIZE_MASK		(0x000003FF)  /*0 <= 1 = 4kB  <= 0x200*/
#define CA9_TZPC_TZPCDECPROT0Stat	(0x800)  /*RO	Decode Protection 0-2 Status Registers.	Reset:0x00000000*/
#define CA9_TZPC_TZPCDECPROT0Set	(0x804)  /*WO	Decode Protection 0-2 Set Registers*/
#define CA9_TZPC_TZPCDECPROT0Clr	(0x808)  /*WO	Decode Protection 0-2 Clear Registers*/
#define CA9_TZPC_TZPCDECPROT1Stat	(0x80C)  /*RO	Decode Protection 0-2 Status Registers.	Reset:0x00000000*/
#define CA9_TZPC_TZPCDECPROT1Set	(0x810)  /*WO	Decode Protection 0-2 Set Registers*/
#define CA9_TZPC_TZPCDECPROT1Clr	(0x814)  /*WO	Decode Protection 0-2 Clear Registers*/
#define CA9_TZPC_TZPCDECPROT2Stat	(0x818)  /*RO	Decode Protection 0-2 Status Registers.	Reset:0x00000000*/
#define CA9_TZPC_TZPCDECPROT2Set	(0x81C)  /*WO	Decode Protection 0-2 Set Registers*/
#define CA9_TZPC_TZPCDECPROT2Clr	(0x820)  /*WO	Decode Protection 0-2 Clear Registers*/
#define CA9_TZPC_TZPCPERIPHID0		(0xFE0)  /*RO	Peripheral Identification Register 0.	Reset:0x00000070*/
#define CA9_TZPC_TZPCPERIPHID1		(0xFE4)  /*RO	Peripheral Identification Register 1.	Reset:0x00000018*/
#define CA9_TZPC_TZPCPERIPHID2		(0xFE8)  /*RO	Peripheral Identification Register 2.	Reset:0x00000004*/
#define CA9_TZPC_TZPCPERIPHID3		(0xFEC)  /*RO	Peripheral Identification Register 3.	Reset:0x00000000*/
#define CA9_TZPC_TZPCPCELLID0		(0xFF0)  /*RO	TZPC Identification Register 0.			Reset:0x0000000D*/
#define CA9_TZPC_TZPCPCELLID1		(0xFF4)  /*RO	TZPC Identification Register 1.			Reset:0x000000F0*/
#define CA9_TZPC_TZPCPCELLID2		(0xFF8)  /*RO	TZPC Identification Register 2.			Reset:0x00000005*/
#define CA9_TZPC_TZPCPCELLID3		(0xFFC)  /*RO	TZPC Identification Register 3.			Reset:0x000000B1*/


#define CA9_TZ_BASE_ADDRESS1		(0xE0200000)
#define CA9_TZ_BASE_ADDRESS2		(CA9_SLCR_BASE_ADDRESS)

/**/
#define CA9_TZ_SECURITY2_SDIO0		(0x08)
#define CA9_TZ_SECURITY3_SDIO1		(0x0C)
#define CA9_TZ_SECURITY4_QSPI		(0x10)
#define CA9_TZ_SECURITY6_APB_SLAVES	(0x18)
#define CA9_TZ_APB_UART0_MASK			BIT(6)
#define CA9_TZ_APB_UART1_MASK			BIT(7)
#define CA9_TZ_WRITE_APB(apb)				WriteReg(CA9_TZ_BASE_ADDRESS1, CA9_TZ_SECURITY6_APB_SLAVES, apb)
#define CA9_TZ_CLEAN_APB()					WriteReg(CA9_TZ_BASE_ADDRESS1, CA9_TZ_SECURITY6_APB_SLAVES, 0)
#define CA9_TZ_SECURE_UART0()				ClearReg(CA9_TZ_BASE_ADDRESS1, CA9_TZ_SECURITY6_APB_SLAVES, CA9_TZ_APB_UART0_MASK)
#define CA9_TZ_NONSECURE_UART0()			SetReg(CA9_TZ_BASE_ADDRESS1, CA9_TZ_SECURITY6_APB_SLAVES, CA9_TZ_APB_UART0_MASK)
#define CA9_TZ_SECURE_UART1()				ClearReg(CA9_TZ_BASE_ADDRESS1, CA9_TZ_SECURITY6_APB_SLAVES, CA9_TZ_APB_UART1_MASK)
#define CA9_TZ_NONSECURE_UART1()			SetReg(CA9_TZ_BASE_ADDRESS1, CA9_TZ_SECURITY6_APB_SLAVES, CA9_TZ_APB_UART1_MASK)
#define CA9_TZ_SECURITY7_SMC		(0x1C)

/* Protection area [0xF800_0000 to 0xF800_0B74] */
#define SLCR_LOCK					(0x0004)	/* SLCR Write Protection Lock - Write the lock key, 0x767B */
#define SLCR_UNLOCK					(0x0008)	/* SLCR Write Protection Lock - Write the lock key, 0xDF0D */
#define CA9_TZ_LOCK_SLCR()				WriteReg(CA9_SLCR_BASE_ADDRESS, SLCR_LOCK, 0x767B)
#define CA9_TZ_UNLOCK_SLCR()			WriteReg(CA9_SLCR_BASE_ADDRESS, SLCR_UNLOCK, 0xDF0D)


#define SLCR_APU_CTRL				(0x0300)
#define CA9_CP15SDISABLED()			WriteReg(CA9_TZ_BASE_ADDRESS2, SLCR_APU_CTRL, BIT(0))

#define XILINX_TZ_DMAC_RST_CTRL 	(0x020C)	/*32 RW 0x00000000 DMA Controller SW Reset Control*/
#define XILINX_TZ_OCM_RAM0  		(0x0400)	/*32 RW 0x00000000 OCM RAM TrustZone Config 0*/
#define XILINX_TZ_OCM_RAM1  		(0x0404)	/*32 RW 0x00000000 OCM RAM TrustZone Config 1*/
#define XILINX_TZ_OCM  				(0x0408)	/*32 RW 0x00000000 OCM ROM TrustZone Config*/
#define XILINX_TZ_DDR_RAM  			(0x0430)	/*32 RW 0x00000000 DDR RAM TrustZone Config*/
#define	CA9_TZ_ADD_DDR_64MB_BLOCK(X)	SetReg(CA9_TZ_BASE_ADDRESS2, XILINX_TZ_DDR_RAM , (BIT(((X)>>26))))
#define	CA9_TZ_DEL_DDR_64MB_BLOCK(X)	ClearReg(CA9_TZ_BASE_ADDRESS2, XILINX_TZ_DDR_RAM , (BIT(((X)>>26))))
#define CA9_TZ_WRITE_DDR_ACCESSMAP(X)	WriteReg(CA9_TZ_BASE_ADDRESS2, XILINX_TZ_DDR_RAM , X)
#define CA9_TZ_CLEAN_DDR_ACCESSMAP()	WriteReg(CA9_TZ_BASE_ADDRESS2, XILINX_TZ_DDR_RAM , 0)
#define XILINX_TZ_DMA_NS  			(0x0440)	/*32 RW 0x00000000 DMAC TrustZone Config*/
#define XILINX_TZ_DMA_IRQ_NS  		(0x0444)	/*32 RW 0x00000000 DMAC TrustZone Config for Interrupts*/
#define XILINX_TZ_DMA_PERIPH_NS  	(0x0448)	/*32 RW 0x00000000 DMAC TrustZone Config for Peripherals*/
#define XILINX_TZ_GEM  				(0x0450)	/*32 RW 0x00000000 Ethernet TrustZone Config*/
#define XILINX_TZ_SDIO  			(0x0454)	/*32 RW 0x00000000 SDIO TrustZone Config*/
#define XILINX_TZ_USB  				(0x0458)	/*32 RW 0x00000000 USB TrustZone Config*/
#define XILINX_TZ_FPGA_M  			(0x0484)	/*32 RW 0x00000000 FPGA master ports TrustZone Disable*/
#define XILINX_TZ_FPGA_AFI  		(0x0488)	/*32 RW 0x00000000 FPGA AFI AXI ports TrustZone Disable*/
#define XILINX_TZ_LVL_SHFTR_EN      (0x0900)
#define XILINX_security_fssw_s0 	(0x90001C)	/*1 WO 0x00000000 M_AXI_GP0 security setting*/
#define XILINX_security_fssw_s1 	(0x900020)	/*1 WO 0x00000000 M_AXI_GP1 security setting*/
#define XILINX_security_apb			(0x900028)	/*6 WO 0x00000000 APB boot secure ports setting*/


#endif /* CONFIG_ARM */

#endif /*_CORE_INCLUDE_DRIVERS_ZYNQ_ZYNQ_TZPC_H__*/
