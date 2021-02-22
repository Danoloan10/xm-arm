/*
 * $FILE: setup.c
 *
 * Setting up and starting up the kernel (arch dependent part)
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
 * - [09/02/16:XM-ARM-2:SPR-151125-01:#97] Enable PL non-secure access when feature is present in "xmcTab.hpv".
 * - [15/02/16:XM-ARM-2:SPR-160215-01:#109] Delete not required function CopyNSAccesibleData.
 */


#include <assert.h>
#include <boot.h>
#include <stdc.h>
#include <physmm.h>

void __VBOOT CopyDataSection(void) {
    extern xm_u8_t _spdata[], _sdata[], _edata[];
    memcpy(_sdata, _spdata, _edata-_sdata);
}

void __VBOOT SetupArchLocal(xm_s32_t cpuid) {
}

#include <arch/io.h>

void __VBOOT EarlySetupArchCommon(void) {
    SetReg(0xF8000000,0x00007000,BIT(3)|BIT(4)|BIT(5)|BIT(6)); /* DBGEN NIDEN SPIDEN SPNIDEN */
    extern void EarlySetupCpu(void);
    /* There is at least one processor in the system */
    SET_NRCPUS(1);
    EarlySetupCpu();
}

void __VBOOT SetupArchCommon(void) {
#ifdef CONFIG_VFP_EXTENSION
    // Disabling FPU
	HwDisableFpu();
#endif /*CONFIG_VFP_EXTENSION*/
#ifdef CONFIG_ZYNQ
#include <xmconf.h>
	if((xmcTab.hpv.hypFeatures&XM_HYP_FEAT_FPGA_PART_ACCESS)==XM_HYP_FEAT_FPGA_PART_ACCESS)
	{
		extern void zynq_fpga_NSAccess(void);
		zynq_fpga_NSAccess();
	}
#endif
}
