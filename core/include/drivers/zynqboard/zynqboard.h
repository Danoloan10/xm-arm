/*
 * $FILE: zynqboard.h
 *
 * board definitions
 *
 * $VERSION$ - 29/09/2014
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
 * - [08/09/16:XM-ARM-2:SPR-160908-01:#123] Define Zynq Memory Map areas.
 * - [11/12/15:XM-ARM-2:SPR-151211-01:#100] Add the calculated CPU_3x2x_CLOCK clock instead PERIPHCLK that is a define.
 * - [16/06/16:CP-160616-01:#113] Configurate the clock prescaled depending on xcf cpu clock value.
 */

#ifndef __DRIVERS_ZYNQBOARD_ZYNQBOARD_H__
#define __DRIVERS_ZYNQBOARD_ZYNQBOARD_H__

#ifdef	CONFIG_ZYNQ

#define PERIOHBASE		0xf8f00000

#define PS_CLK						(100000000.0/3.0)

#define PLL_ARM_FEEDBACK_DIV		(30)

#define PLL_ARM_DIV0				(2)

#define ARM_PLL_CLK					((PS_CLK*PLL_ARM_FEEDBACK_DIV)/PLL_ARM_DIV0)

#define XPAR_CPU_CORTEXA9_CORE_CLOCK_FREQ_HZ ARM_PLL_CLK

#define CPU_1x_CLOCK                ((XPAR_CPU_CORTEXA9_CORE_CLOCK_FREQ_HZ)/6.0)
#define CPU_2x_CLOCK                ((XPAR_CPU_CORTEXA9_CORE_CLOCK_FREQ_HZ)/3.0)
#define CPU_3x2x_CLOCK              ((XPAR_CPU_CORTEXA9_CORE_CLOCK_FREQ_HZ)/2.0)
#define CPU_6x4x_CLOCK              ((XPAR_CPU_CORTEXA9_CORE_CLOCK_FREQ_HZ)/1.0)


#define XM_VMMAP_OCM_LOW_START  (0x00000000u)
#define XM_VMMAP_OCM_LOW_SIZE   (256u*0x400u)
#define XM_VMMAP_DDR_START      (0x00100000)
#define XM_VMMAP_DDR_SIZE       (1023u*0x400u*0x400u)
#define XM_VMMAP_PL_START       (0x40000000)
#define XM_VMMAP_PL_SIZE        (2u*0x400u*0x400u*0x400u)
#define XM_VMMAP_IOP_START      (0xE0000000)
#define XM_VMMAP_IOP_SIZE       (3u*0x400u*0x400u)
#define XM_VMMAP_SMC_START      (0xE1000000)
#define XM_VMMAP_SMC_SIZE       (80u*0x400u*0x400u)
#define XM_VMMAP_SLCR_START     (0xF8000000)
#define XM_VMMAP_SLCR_SIZE      (3u*0x400u)
#define XM_VMMAP_PS_START       (0xF8001000)
#define XM_VMMAP_PS_SIZE        (8u*0x400u*0x400u)+(60u*0x400u)
#define XM_VMMAP_CPU_START      (0xF8900000)
#define XM_VMMAP_CPU_SIZE       (6u*0x400u*0x400u)+(12u*0x400u)
#define XM_VMMAP_QSPI_START     (0xFC000000)
#define XM_VMMAP_QSPI_SIZE      (32u*0x400u*0x400u)
#define XM_VMMAP_OCM_HIGH_START (0xFFFC0000)
#define XM_VMMAP_OCM_HIGH_SIZE  (256u*0x400u)

#endif /*CONFIG_ZYNQ*/

#endif /*__DRIVERS_ZYNQBOARD_ZYNQBOARD_H__*/
