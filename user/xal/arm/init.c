/*
 * $FILE: init.c
 *
 * Copy data section to SRAM
 *
 * $VERSION$
 *
 * Author: Fent Innovative Software Solutions
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE file for the license terms.
 */
/*
 * Changelog:
 * - [21/10/16:XM-ARM-2:SPR-161013-02:#128] Start vCpus diferent than 0 using funtion startVCPUs.
 * - [21/03/17:XM-ARM-2:CP-161209-04:#137] Automatically enable FPU in XAL if fp flag is set using funtion EnableFPU.
 */
#include <xm.h>
#include <string.h>

void CopyDataSection(void) {
    extern xm_u8_t _spdata[], _sdata[], _edata[];
    memcpy(_sdata, _spdata, _edata-_sdata);
}

//#include <xm.h>
//#include <config.h>

//extern struct xmImageHdr xmImageHdr;
extern xmAddress_t startVCPUs[];

void SetupVCpus(void)
{
	int i;
	if (XM_get_vcpuid() == 0)
	{
		for (i = 1; i < XM_get_number_vcpus(); i++)
		{
			XM_reset_vcpu(i, (xmAddress_t) startVCPUs);
		}
	}
}

void EnableFPU(void)
{
    if(XM_params_get_PCT()->flags&PARTITION_FP_F)
    {
    	__asm__ __volatile__(   "fmrx r0, FPEXC\n\t"                	\
                        "orr  r0, r0, #0x40000000\n\t"                  \
                        "fmxr FPEXC,r0\n\t"                             \
                        :::"r0");
    }
}

/* Initialize Zynq board main characteristics */
void ZynqInit(void)
{
#ifdef CONFIG_VFP_EXTENSION
	EnableFPU();
#endif /*CONFIG_VFP_EXTENSION*/

}

