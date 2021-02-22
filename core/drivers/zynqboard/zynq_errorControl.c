/*
 * $FILE: zynq_errorControl.c
 *
 * $VERSION$
 *
 * Author: Yolanda Valiente <yvaliente@fentiss.com>
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license terms.
 */
/*
 * Changelog:
 */

#ifdef CONFIG_ZYNQ

#include <irqs.h>
#include <arch/smp.h>
#include <objects/status.h>
#include <drivers/zynqboard/zynq_errorControl.h>
#ifdef CONFIG_ENABLE_CACHE
#include <drivers/zynqboard/zynq_cache.h>
#endif


static void ParityL1CacheTLBHandler(cpuCtxt_t *ctxt, void *data) {
	if (L1CACHE_TLB_CPU0_ERROR())
		kprintf("Parity error detected in L1 Cache/TLB/GHB/BTAC from CPU 0\n");
	if (L1CACHE_TLB_CPU1_ERROR())
		kprintf("Parity error detected in L1 Cache/TLB/GHB/BTAC from CPU 1\n");

	xm_u32_t ifsr = GetMmuIFaultStatusReg();
	xm_u32_t dfsr = GetMmuFaultStatusReg();

	/* Instruction TLB error */
	if ((ifsr==FSR_TLB_SYNC_EXTERNAL_ABORT_1LEVEL) || (ifsr==FSR_TLB_SYNC_EXTERNAL_ABORT_2LEVEL) ||
			(ifsr==FSR_TLB_SYNC_PARITY_ERROR_1LEVEL) || (ifsr==FSR_TLB_SYNC_PARITY_ERROR_2LEVEL)) {
		InvalidateEntireInstructionTLB();
		systemStatus.noTLBParityErrors++;
	}
	/* Data TLB error */
	if ((dfsr==FSR_TLB_SYNC_EXTERNAL_ABORT_1LEVEL) || (dfsr==FSR_TLB_SYNC_EXTERNAL_ABORT_2LEVEL) ||
			(dfsr==FSR_TLB_SYNC_PARITY_ERROR_1LEVEL) || (dfsr==FSR_TLB_SYNC_PARITY_ERROR_2LEVEL)) {
		InvalidateEntireDataTLB();
		systemStatus.noTLBParityErrors++;
	}
	/* L1 Instruction cache error */
	if ((ifsr==FSR_SYNC_EXTERNAL_ABORT) || (ifsr==FSR_MEMORY_ACCESS_SYNC_PARITY_ERROR)) {
		xmAddress_t ifar = (xmAddress_t)GetMmuIFaultAddressReg();
		//kprintf("IFSR:%x - IFAR:0x%x\n", ifsr, ifar);
		InvalidateL1ICacheLine(ifar);
		systemStatus.noL1ICacheParityErrors++;
	}
	/* L1 Data cache error */
	if ((dfsr==FSR_SYNC_EXTERNAL_ABORT) || (dfsr==FSR_MEMORY_ACCESS_SYNC_PARITY_ERROR) ||
			(dfsr==DFSR_ASYNC_EXTERNAL_ABORT) || (dfsr==DFSR_MEMORY_ACCESS_ASYNC_PARITY_ERROR)) {
		/* Only WB_WA cache policy is supported. No action to avoid the loss of data */
		//xmAddress_t dfar = (xmAddress_t)GetMmuFaultAddressReg();
		//kprintf("DFSR:%x - DFAR:0x%x\n", dfsr, dfar);
		//InvalidateL1DCacheLine(dfar);
		systemStatus.noL1DCacheParityErrors++;
	}

	if (L1CACHE_TLB_ERROR())
		kprintf("Unrecoverable parity error\n");
}

#ifdef CONFIG_ENABLE_CACHE_L2
static void ParityL2CacheHandler(cpuCtxt_t *ctxt, void *data) {
	kprintf("Parity error detected in L2 Cache\n");

	/* Only invalidate when L2 cache policy is WT_NWA to avoid the loss of data */
	if (CACHE_L2_POLICY == _PG_ARCH_CACHEPOLICY_WT_NWA) {
		InvalidateL2Cache();

		/* Clear the pending interrupts */
		WriteL2CInterruptClearReg(ReadL2CInterruptRawStatusReg());
	}

	systemStatus.noL2CacheParityErrors++;

	if (L2CACHE_ERROR())
		kprintf("Unrecoverable parity error\n");
}
#endif

static void ParitySCUHandler(cpuCtxt_t *ctxt, void *data) {
	kprintf("Parity error detected in SCU\n");

	/* Invalidate scu */
	CA9_SCU_INVALIDATE();

	systemStatus.noSCUParityErrors++;

	if (SCU_ERROR())
		kprintf("Unrecoverable parity error\n");
}

void SetupErrorControl(void) {
#ifdef CONFIG_ENABLE_CACHE
    HwEnableIrq(ZYNQ_L1CACHE_TLB_CPU0_IRQ);
    SetIrqHandler(ZYNQ_L1CACHE_TLB_CPU0_IRQ, ParityL1CacheTLBHandler, 0);

    HwEnableIrq(ZYNQ_L1CACHE_TLB_CPU1_IRQ);
    SetIrqHandler(ZYNQ_L1CACHE_TLB_CPU1_IRQ, ParityL1CacheTLBHandler, 0);
#endif

#ifdef CONFIG_ENABLE_CACHE_L2
    if(GET_CPU_ID()==0) {
		HwEnableIrq(ZYNQ_L2CACHE_IRQ);
		SetIrqHandler(ZYNQ_L2CACHE_IRQ, ParityL2CacheHandler, 0);
    }
#endif

    HwEnableIrq(ZYNQ_SCU_IRQ);
    SetIrqHandler(ZYNQ_SCU_IRQ, ParitySCUHandler, 0);
}

int ErrorControlCheckParity(cpuCtxt_t *ctxt) {

	int irq = XM_HM_MAX_GENERIC_EVENTS+ctxt->irqNr;

	if ((irq != XM_HM_EV_ARM_PREFETCH_ABORT) && (irq != XM_HM_EV_ARM_DATA_ABORT))
		return 0;

	if (!(PARITY_ERROR()))
		return 0;

	if (L1CACHE_TLB_ERROR())
		ParityL1CacheTLBHandler(ctxt, 0);
#ifdef CONFIG_ENABLE_CACHE_L2
	if (L2CACHE_ERROR())
		ParityL2CacheHandler(ctxt, 0);
#endif
	if (SCU_ERROR())
		ParitySCUHandler(ctxt, 0);

	/* Returns if there is a parity error and has been fixed */
	return !(PARITY_ERROR());
}

#endif /*CONFIG_ZYNQ*/
