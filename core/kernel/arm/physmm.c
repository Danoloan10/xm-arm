/*
 * $FILE: physmm.c
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
 * - [24/08/16:XM-ARM-2:SPR-160819-02:#118] Add Partitions Access to OCM.
 */

#include <arm/physmm.h>
#include <kthread.h>

#include <drivers/zynqboard/zynq_tzpc.h>

static void ByPassTZDDRProtection(void)
{
    CA9_TZ_UNLOCK_SLCR();
    CA9_TZ_CLEAN_DDR_ACCESSMAP();
    CA9_TZ_WRITE_DDR_ACCESSMAP(0xFFFFFFFF);
    CA9_TZ_LOCK_SLCR();
}

static void ByPassTZOCMProtection(void)
{
    CA9_TZ_UNLOCK_SLCR();
    WriteReg(CA9_TZ_BASE_ADDRESS2, XILINX_TZ_OCM_RAM0 , 0xFFFFFFFF);
    WriteReg(CA9_TZ_BASE_ADDRESS2, XILINX_TZ_OCM_RAM1 , 0xFFFFFFFF);
    WriteReg(CA9_TZ_BASE_ADDRESS2, XILINX_TZ_OCM , 0xFFFFFFFF);
    CA9_TZ_LOCK_SLCR();
}

void ArchSetupPhysMM(void) {
	ByPassTZDDRProtection();
	ByPassTZOCMProtection();
}
