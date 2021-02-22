/*
 * $FILE: zynq_mmu.c
 *
 * board definitions
 *
 * $VERSION$ - 15/07/2014
 *
 * Author: Manuel Muñoz <mmunoz@ai2.upv.es>
 * Author: Javier O. Coronel <jcoronel@fentiss.com>
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE file for the license terms.
 */

#include "drivers/zynqboard/zynq_mmu.h"
#include "processor.h"
#include "arch/arch_types.h"
#include "arch/asm.h"

#ifdef CONFIG_ZYNQ

// Enable the TLB
void EnableMMU(void)
{
    WR_DACR(1);

    // Set SCTLR.M (bit 0) to enable TLB
    SET_SCTLR(0x1);

    DSB_INST(); /* dsb	allow the MMU to start up */
	ISB_INST(); /* isb	flush prefetch buffer */
}

void DisableMMU(void)
{
    // Set SCTLR.M (bit 0) to enable TLB
    CLEAR_SCTLR(0x1);

    DSB_INST(); /* dsb	allow the MMU to start up */
	ISB_INST(); /* isb	flush prefetch buffer */

}

void __attribute__ ((section (".nsecArea.text"))) NSFlushTlbGlobal(void)
{
	xm_u32_t cpsr, spsr;
	xmAddress_t auxsp;

	NSOP_PRE(cpsr, spsr, auxsp);

	FlushTlbGlobal();

	NSOP_POST(cpsr, spsr, auxsp);
}

#endif /*CONFIG_ZYNQ*/
