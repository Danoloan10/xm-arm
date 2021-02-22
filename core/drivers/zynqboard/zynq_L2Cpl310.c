/*
 * $FILE: zynq_L2Cpl310.c
 *
 * L2 Cache definitions
 *
 * $VERSION$ - 21/02/2017
 *
 * Author: Yolanda Valiente <yvaliente@fentiss.com>
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE file for the license terms.
 */

#ifdef CONFIG_ENABLE_CACHE_L2

#include <drivers/zynqboard/zynq_L2Cpl310.h>
#include "processor.h"

void EnableL2Cache(void)
{
	xm_u32_t reg;

	/* Write 0x020202 to the register at 0xF8000A1C. This is a mandatory step. */
	WriteReg(CA9_SLCR_BASE_ADDRESS, CA9_SLCR_RESERVED, 0x020202);

    /* Write to the auxiliary, tag RAM latency, data RAM latency, pre-fetch,
		and Power Control registers using a read-modify-write to set up global
		configurations:
		  ° Associativity and way size
		  ° Latencies for RAM accesses
		  ° Allocation policy
		  ° Pre-fetch and power capabilities
		  */
	reg = ReadL2CAuxiliaryControlReg();
	L2C_CONFIGURE_ACR_ASSOCIATIVITY_8WAY(reg);
	L2C_CONFIGURE_ACR_WAY_SIZE(reg, L2C_ACR_WAY_SIZE_64kb);
	L2C_CONFIGURE_ACR_CACHE_REPLACE_POLICY_ROUND_ROBIN(reg);
	L2C_CONFIGURE_ACR_PARITY_ENABLE(reg);
	L2C_CONFIGURE_ACR_SHARED_ENABLE(reg);
	WriteL2CAuxiliaryControlReg(reg);

	reg = ReadL2CTagRAMControlReg();
	L2C_CONFIGURE_RAM_SETUP_LATENCY(reg, L2C_RAMCR_LATENCY_2CYCLES);
	L2C_CONFIGURE_RAM_READ_LATENCY(reg, L2C_RAMCR_LATENCY_2CYCLES);
	L2C_CONFIGURE_RAM_WRITE_LATENCY(reg, L2C_RAMCR_LATENCY_2CYCLES);
	WriteL2CTagRAMControlReg(reg);

	reg = ReadL2CDataRAMControlReg();
	L2C_CONFIGURE_RAM_SETUP_LATENCY(reg, L2C_RAMCR_LATENCY_2CYCLES);
	L2C_CONFIGURE_RAM_READ_LATENCY(reg, L2C_RAMCR_LATENCY_3CYCLES);
	L2C_CONFIGURE_RAM_WRITE_LATENCY(reg, L2C_RAMCR_LATENCY_2CYCLES);
	WriteL2CDataRAMControlReg(reg);

	reg = ReadL2CPrefetchControlReg();
	L2C_PFCR_DATA_PREF_EN_ENABLE(reg);
	L2C_PFCR_DATA_INST_EN_ENABLE(reg);
	WriteL2CPrefetchControlReg(reg);

	/* Write to the interrupt clear register to clear any residual raw
		  interrupts set.  */
	WriteL2CInterruptClearReg(ReadL2CInterruptRawStatusReg());

	/* Write to the interrupt mask register if it is desired to enable
		  interrupts. */
	WriteL2CInterruptMaskReg(L2C_IMR_EN_INT_MASK);

	InvalidateL2Cache();

	/* Write to control register 1 with the LSB set to 1 to enable the
		  cache. */
	L2C_L2_ENABLE();

    L2CacheSync();

	DSB_INST();
	ISB_INST();
}

void DisableL2Cache(void)
{
	FlushL2Cache();

	L2C_L2_DISABLE();

	DSB_INST();
	ISB_INST();
}

void InvalidateL2Cache(void)
{
	/* Secure write to invalidate by way, offset 0x77C, to invalidate all entries
		in cache:
		  ° Write 0xFFFF to 0x77C
		  ° Poll the cache maintenance register until invalidate operation is
		    complete.
		    */
	WriteL2CInvWayReg(L2C_IWR_INV_ALL_ENTRIES_MASK);
	while ((ReadL2CInvWayReg() & L2C_IWR_INV_ALL_ENTRIES_MASK) != 0) {
		continue;
	}

	L2CacheSync();

	DSB_INST();
	ISB_INST();
}

void InvalidateL2CacheRange(xmAddress_t addr, xm_u32_t size)
{
	xmAddress_t localAddr = addr;
	xm_u32_t end;

	if (size == 0)
		return;

	end = localAddr + size;
	localAddr = localAddr & ~(L2C_CACHELINE - 1);

	/* Disable Write-back and line fills */
	WriteL2CDebugControlReg(L2C_DCR_WB_LF_DISABLE_MASK);

	while (localAddr < end) {
		WriteL2CInvPaReg(localAddr);
		L2CacheSync();
		localAddr += L2C_CACHELINE;
	}

	/* Enable Write-back and line fills */
	WriteL2CDebugControlReg(L2C_DCR_WB_LF_ENABLE_MASK);

	DSB_INST();
	ISB_INST();
}

void InvalidateL2CacheLine(xmAddress_t addr)
{
	WriteL2CInvPaReg(addr);

	DSB_INST();
	ISB_INST();
}

void FlushL2Cache(void)
{
	/* Disable Write-back and line fills */
	WriteL2CDebugControlReg(L2C_DCR_WB_LF_DISABLE_MASK);

	/* Clean and invalidate by way:
		  ° Write 0xFFFF to 0x7FC
		  ° Poll the cache maintenance register until invalidate operation is
		    complete.
		    */
	WriteL2CCleanInvWayReg(L2C_CIWR_INV_ALL_ENTRIES_MASK);
	while ((ReadL2CCleanInvWayReg() & L2C_CIWR_INV_ALL_ENTRIES_MASK) != 0) {
		continue;
	}

	L2CacheSync();

	/* Enable Write-back and line fills */
	WriteL2CDebugControlReg(L2C_DCR_WB_LF_ENABLE_MASK);

	DSB_INST();
	ISB_INST();
}

void FlushL2CacheRange(xmAddress_t addr, xm_u32_t size)
{
	xmAddress_t localAddr = addr;
	xm_u32_t end;

	if (size == 0)
		return;

	end = localAddr + size;
	localAddr = localAddr & ~(L2C_CACHELINE - 1);

	/* Disable Write-back and line fills */
	WriteL2CDebugControlReg(L2C_DCR_WB_LF_DISABLE_MASK);

	while (localAddr < end) {
		WriteL2CCleanInvPaReg(localAddr);
		L2CacheSync();
		localAddr += L2C_CACHELINE;
	}

	/* Enable Write-back and line fills */
	WriteL2CDebugControlReg(L2C_DCR_WB_LF_ENABLE_MASK);

	DSB_INST();
	ISB_INST();
}

#endif /*CONFIG_ENABLE_CACHE_L2*/
