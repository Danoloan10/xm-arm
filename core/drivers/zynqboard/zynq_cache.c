/*
 * $FILE: zynq_cache.c
 *
 * Cache definitions
 *
 * $VERSION$ - 21/02/2017
 *
 * Author: Yolanda Valiente <yvaliente@fentiss.com>
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE file for the license terms.
 */

#ifdef CONFIG_ENABLE_CACHE

#include <drivers/zynqboard/zynq_cache.h>
#include "processor.h"

/*************
 * L1 Caches *
 *************/

inline void FlushL1DCache(void)
{
	xm_u32_t ccsidr = 0;
	xm_u32_t cacheSize, numWays, lineSize, numSet;
	xm_u32_t way, wayIndex, set, setIndex;

	DCacheLineSel();

	GET_CCSIDR(ccsidr);

	/* Calculate cache size */
	cacheSize = (ccsidr >> 13) & 0x1FF;
	cacheSize +=1;
	cacheSize *=128;

	/* Get the number of ways */
	/*
	 * numWays = (ccsidr & 0x3ffU) >> 3U;
	 * numWays += 1U; */
	numWays = L1DNUMWAYS;

	/* Calculate the cacheline size and set size */
	lineSize = (ccsidr & 0x07) + 4;

	numSet = cacheSize/numWays;
	numSet /= (0x00000001 << lineSize);

	way = 0;
	set = 0;
	for (wayIndex=0; wayIndex < numWays; wayIndex++) {
		for (setIndex=0; setIndex < numSet; setIndex++) {
			CacheDCCISW(way | set);
			set += (0x00000001 << lineSize);
		}
		set = 0;
		way += 0x40000000;
	}

	DSB_INST();
	ISB_INST();
}

inline void FlushL1DCacheRange(xmAddress_t addr, xm_u32_t size)
{
	xmAddress_t localAddr = addr;
	xm_u32_t end;

	if (size == 0)
		return;

	end = localAddr + size;
	localAddr = localAddr & ~(L1CACHELINE - 1);

	DCacheLineSel();

	while (localAddr < end) {
		CacheDCCIMVAC(localAddr);
		localAddr += L1CACHELINE;
	}

	DSB_INST();
	ISB_INST();
}

void InvalidateL1DCache(void)
{
	L1_DCACHE_INVALIDATE();

	DSB_INST();
	ISB_INST();
}

inline void InvalidateL1DCacheRange(xmAddress_t addr, xm_u32_t size)
{
	xmAddress_t localAddr = addr;
	xm_u32_t end;

	if (size == 0)
		return;

	end = localAddr + size;
	localAddr = localAddr & ~(L1CACHELINE - 1);

	DCacheLineSel();

	while (localAddr < end) {
		CacheDCIMVAC(localAddr);
		localAddr += L1CACHELINE;
	}

	DSB_INST();
	ISB_INST();
}

inline void InvalidateL1DCacheLine(xmAddress_t addr)
{
	DCacheLineSel();

	CacheDCIMVAC((addr & (~0x1F)));

	DSB_INST();
	ISB_INST();
}

void InvalidateL1ICache(void)
{
	ICacheLineSel();

	/* Invalidate the instruction cache */
	CacheICIALLU();

	DSB_INST();
	ISB_INST();
}

inline void InvalidateL1ICacheRange(xmAddress_t addr, xm_u32_t size)
{
	xmAddress_t localAddr = addr;
	xm_u32_t end;

	if (size == 0)
		return;

	end = localAddr + size;
	localAddr = localAddr & ~(L1CACHELINE - 1);

	ICacheLineSel();

	while (localAddr < end) {
		CacheICIMVAU(localAddr);
		localAddr += L1CACHELINE;
	}

	DSB_INST();
	ISB_INST();
}

inline void InvalidateL1ICacheLine(xmAddress_t addr)
{
	ICacheLineSel();

	CacheICIMVAU((addr & (~0x1F)));

	DSB_INST();
	ISB_INST();
}

void EnableL1DCache(void)
{
	InvalidateL1DCache();

	L1_DCACHE_ENABLE();
}

void EnableL1ICache(void)
{
	InvalidateL1ICache();

	L1_ICACHE_ENABLE();
}

void DisableL1DCache(void)
{
	FlushL1DCache();

    L1_DCACHE_DISABLE();
}

void DisableL1ICache(void)
{
	InvalidateL1ICache();

    L1_ICACHE_DISABLE();
}


/*************
 *  Caches   *
 *************/

inline void FlushICache(void)
{
	DMB_INST();
	CacheICIALLU();
	CacheBPIALL();
	DSB_INST();
	ISB_INST();
}

inline void FlushDCache(void)
{
	FlushL1DCache();
}

inline void FlushDCacheRange(xmAddress_t addr, xm_u32_t size)
{
	FlushL1DCacheRange(addr, size);
#ifdef CONFIG_ENABLE_CACHE_L2
	if (CACHE_L2_POLICY != _PG_ARCH_CACHEPOLICY_WT_NWA)
		FlushL2CacheRange(addr, size);
#endif
}

void FlushCache(void)
{
	FlushDCache();
	FlushICache();
#ifdef CONFIG_ENABLE_CACHE_L2
	if (CACHE_L2_POLICY != _PG_ARCH_CACHEPOLICY_WT_NWA)
		FlushL2Cache();
#endif
}


void __attribute__ ((section (".nsecArea.text"))) NSFlushCache(void)
{
	xm_u32_t cpsr, spsr;
	xmAddress_t auxsp;
	NSOP_PRE(cpsr, spsr, auxsp);

	FlushDCache();
	FlushICache();

	NSOP_POST(cpsr, spsr, auxsp);
}

void InvalidateCache(void)
{
#ifdef CONFIG_ENABLE_CACHE_L2
    InvalidateL2Cache();		/* Invalidate L2 cache */
#endif
	InvalidateL1ICache();		/* Invalidate icache */
	CacheBPIALL();				/* Invalidate branch predictor array */
	InvalidateL1DCache();		/* Invalidate dcache */
}

void EnableCache(void)
{
	CA9_SCR_SECURE();
	EnableL1ICache();
	EnableL1DCache();
#ifdef CONFIG_ENABLE_CACHE_L2
	if (GET_CPU_ID()==0) {
		EnableL2Cache();
	}
#endif
	BPI_ENABLE();
	CA9_SCR_NONSECURE();
	EnableL1ICache();
	EnableL1DCache();
    BPI_ENABLE();
    CA9_SCR_SECURE();
}

void DisableCache(void)
{
#ifdef CONFIG_ENABLE_CACHE_L2
    if (GET_CPU_ID()==0) {
    	DisableL2Cache();
    }
#endif
    DisableL1DCache();
    DisableL1ICache();
}

inline int SetL1CacheState(xm_u32_t cache)
{
	xm_u32_t tmp = 0;
	cache &= SCTLR_C_BIT | SCTLR_I_BIT;
	GET_SCTLR(tmp);
	if ((tmp & (SCTLR_C_BIT | SCTLR_I_BIT)) == cache)
		return 0;
	tmp &= ~(SCTLR_C_BIT | SCTLR_I_BIT);
	tmp |= cache;
	WR_SCTLR(tmp);
	return 1;
}

inline void SetCacheState(xm_u32_t cache)
{
	if (SetL1CacheState(cache))
		FlushCache();
}

inline void NSSetCacheState(xm_u32_t cache)
{
	if (SetL1CacheState(cache))
		NSFlushCache();
}

#endif /*CONFIG_ENABLE_CACHE*/
