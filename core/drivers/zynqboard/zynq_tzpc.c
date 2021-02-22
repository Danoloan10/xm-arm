/*_
 * $FILE: zynq_tzpc.c
 *
 * board definitions
 *
 * $VERSION$ - 14/10/2014
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
 * - [09/02/16:XM-ARM-2:SPR-151125-01:#97] Create a function to enable PL non-secure access.
 * - [08/09/16:XM-ARM-2:SPR-160908-01:#123] Allow access to entire Memory Map.
 * - [21/03/17:XM-ARM-2:SPR-161209-02:#135] PL access rights to M_AXI_GP1.
 */

#ifdef CONFIG_ZYNQ
#include "arch/io.h"
#include "drivers/zynqboard/zynq_tzpc.h"
void zynq_fpga_NSAccess(void)
{
	WriteReg(CA9_SLCR_BASE_ADDRESS, XILINX_TZ_LVL_SHFTR_EN, 0x0000000F);//LVL_SHFTR_EN
	WriteReg(CA9_SLCR_BASE_ADDRESS, XILINX_TZ_SDIO, 0x3);   //TZ_SDIO
	WriteReg(CA9_SLCR_BASE_ADDRESS, XILINX_security_fssw_s0, 0x1); 	//security_fssw_s0
	WriteReg(CA9_SLCR_BASE_ADDRESS, XILINX_security_fssw_s1, 0x1);	//security_fssw_s1
}
#endif /*CONFIG_ZYNQ*/
