/*
 * $FILE: zynq_L2Cpl310.h
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
/*
 * Changelog:
 */

#ifndef _CORE_INCLUDE_DRIVERS_ZYNQ_ZYNQ_CACHE_H__
#define _CORE_INCLUDE_DRIVERS_ZYNQ_ZYNQ_CACHE_H__

#ifdef CONFIG_ENABLE_CACHE_L2
#include "drivers/zynqboard/zynq_L2Cpl310.h"
#endif /* CONFIG_ENABLE_CACHE_L2 */

#ifdef CONFIG_ENABLE_CACHE

/* L1 cache for XM Data section */
#ifdef CONFIG_DCACHE_L1_POLICY_WB_WA
#define DCACHE_L1_POLICY				(_PG_ARCH_CACHEPOLICY_WB_WA)
#else
#define DCACHE_L1_POLICY				(_PG_ARCH_CACHEPOLICY_NONCACHEABLE)
#endif
/* L1 cache for XM Code section */
#ifdef CONFIG_ICACHE_L1_POLICY_WB_WA
#define ICACHE_L1_POLICY				(_PG_ARCH_CACHEPOLICY_WB_WA)
#elif defined(CONFIG_ICACHE_L1_POLICY_WT_NWA)
#define ICACHE_L1_POLICY				(_PG_ARCH_CACHEPOLICY_WT_NWA)
#elif defined(CONFIG_ICACHE_L1_POLICY_WB_NWA)
#define ICACHE_L1_POLICY				(_PG_ARCH_CACHEPOLICY_WB_NWA)
#else
#define ICACHE_L1_POLICY				(_PG_ARCH_CACHEPOLICY_NONCACHEABLE)
#endif
/* L1 cache for partition data and code sections */
#ifdef CONFIG_CACHE_L1_PARTITION_POLICY_WB_WA
#define CACHE_L1_PARTITION_POLICY		(_PG_ARCH_CACHEPOLICY_WB_WA)
#elif defined(CONFIG_CACHE_L1_PARTITION_POLICY_WT_NWA)
#define CACHE_L1_PARTITION_POLICY		(_PG_ARCH_CACHEPOLICY_WT_NWA)
#elif defined(CONFIG_CACHE_L1_PARTITION_POLICY_WB_NWA)
#define CACHE_L1_PARTITION_POLICY		(_PG_ARCH_CACHEPOLICY_WB_NWA)
#else
#define CACHE_L1_PARTITION_POLICY		(_PG_ARCH_CACHEPOLICY_NONCACHEABLE)
#endif

#if !defined(CONFIG_ENABLE_CACHE_L2)||!(CONFIG_ENABLE_CACHE_L2)
#define CACHE_L2_POLICY					(_PG_ARCH_CACHEPOLICY_NONCACHEABLE)
#endif


#define _PG_ARCH_NONCACHEABLE			(_PG_ARCH_MEMORYTYPE_CACHED(_PG_ARCH_CACHEPOLICY_NONCACHEABLE,_PG_ARCH_CACHEPOLICY_NONCACHEABLE))
#if (DCACHE_L1_POLICY != _PG_ARCH_CACHEPOLICY_WB_WA)
#define _PG_ARCH_DCACHEABLE				(_PG_ARCH_NONCACHEABLE)		/* requirement for L1/DDR exclusive access */
#else
#define _PG_ARCH_DCACHEABLE				(_PG_ARCH_MEMORYTYPE_CACHED(CACHE_L2_POLICY,DCACHE_L1_POLICY))
#endif
#define _PG_ARCH_ICACHEABLE				(_PG_ARCH_MEMORYTYPE_CACHED(CACHE_L2_POLICY,ICACHE_L1_POLICY))
#define _PG_ARCH_USER_CACHEABLE			(_PG_ARCH_MEMORYTYPE_CACHED(CACHE_L2_POLICY,CACHE_L1_PARTITION_POLICY))


/*Coprocessor Cache Size Identification Register*/
#define GET_CCSIDR(Rd) READ_CP15(1,Rd,c0,c0,0)
#define SET_CCSIDR(Rd) WRITE_CP15(1,Rd,c0,c0,0)

/**********
 * Caches *
 **********/

#define L1CACHELINE	32
#define L1DNUMWAYS 	4

#define DCACHE SCTLR_C_BIT
#define ICACHE SCTLR_I_BIT

/*
 * Cache Maintenance
 */
#ifndef __ASSEMBLY__
#define L1_DCACHE_ENABLE() \
	__asm__ __volatile__(	"mrc     p15, 0, r0, c1, c0, 0\n\t"		/*Read System Control Register configuration data*/ \
							"orr     r0, r0, #(1 << 2)\n\t"			/*Set C bit*/ \
							"mcr     p15, 0, r0, c1, c0, 0\n\t"		/*Write System Control Register configuration data*/ \
						:::"r0")
#endif /*__ASSEMBLY__*/

#ifndef __ASSEMBLY__
#define L1_ICACHE_ENABLE() \
	__asm__ __volatile__(	"mrc     p15, 0, r0, c1, c0, 0\n\t"		/*Read System Control Register configuration data*/ \
							"orr     r0, r0, #(1 << 12)\n\t"		/*Set I bit*/ \
							"mcr     p15, 0, r0, c1, c0, 0\n\t"		/*Write System Control Register configuration data*/ \
						:::"r0")
#endif /*__ASSEMBLY__*/

#ifndef __ASSEMBLY__
#define L1_DCACHE_DISABLE() \
	__asm__ __volatile__(	"mrc     p15, 0, r0, c1, c0, 0\n\t"		/*Read System Control Register configuration data*/ \
							"bic     r0, r0, #(1 << 2)\n\t"			/*Clear C bit*/ \
							"mcr     p15, 0, r0, c1, c0, 0\n\t"		/*Write System Control Register configuration data*/ \
						:::"r0")
#endif /*__ASSEMBLY__*/

#ifndef __ASSEMBLY__
#define L1_ICACHE_DISABLE() \
	__asm__ __volatile__(	"mrc     p15, 0, r0, c1, c0, 0\n\t"		/*Read System Control Register configuration data*/ \
							"bic     r0, r0, #(1 << 12)\n\t"		/*Clear I bit*/ \
							"mcr     p15, 0, r0, c1, c0, 0\n\t"		/*Write System Control Register configuration data*/ \
						:::"r0")
#endif /*__ASSEMBLY__*/

#ifndef __ASSEMBLY__
#define L1_DCACHE_INVALIDATE() \
        __asm__ __volatile__(                                                                                   \
        "push	{r4-r12}\n\t"																					\
        "mov     r0, #0\n\t"																					\
        "mcr     p15, 0, r0, c7, c5, 0\n\t"     /*ICIALLU - Invalidate entire I Cache, and flushes branch target cache*/\
        "mrc     p15, 1, r0, c0, c0, 1\n\t"     /*Read CLIDR*/													\
        "ands    r3, r0, #0x7000000\n\t"																		\
        "mov     r3, r3, LSR #23\n\t"           /*Cache level value (naturally aligned)*/						\
        "beq     5f\n\t"				                           												\
        "mov     r10, #0\n\t"																					\
        "1:\n\t"								/*invalidate_caches_loop1*/										\
        "add     r2, r10, r10, LSR #1\n\t"     /*Work out 3xcachelevel*/										\
        "mov     r1, r0, LSR r2\n\t"           /*bottom 3 bits are the Cache type for this level*/				\
        "and     r1, r1, #7\n\t"               /*get those 3 bits alone*/										\
        "cmp     r1, #2\n\t"																					\
        "blt     4f\n\t"                       /*no cache or only instruction cache at this level*/				\
        "mcr     p15, 2, r10, c0, c0, 0\n\t"   /*write the Cache Size selection register*/						\
        "isb\n\t"                              /*ISB to sync the change to the CacheSizeID reg*/				\
        "mrc     p15, 1, r1, c0, c0, 0\n\t"    /*reads current Cache Size ID register*/							\
        "and     r2, r1, #7\n\t"              /*extract the line length field*/									\
        "add     r2, r2, #4\n\t"               /*add 4 for the line length offset (log2 16 bytes)*/				\
        "ldr     r4, =0x000003FF\n\t"																			\
        "ands    r4, r4, r1, LSR #3\n\t"       /*R4 is the max number on the way size (right aligned)*/			\
        "clz     r5, r4\n\t"                   /*R5 is the bit position of the way size increment*/				\
        "ldr     r7, =0x00007FFF\n\t"																			\
        "ands    r7, r7, r1, LSR #13\n\t"      /*R7 is the max number of the index size (right aligned)*/		\
        "2:\n\t"							   /*invalidate_caches_loop2*/										\
        "mov     r9, R4\n\t"                   /*R9 working copy of the max way size (right aligned)*/			\
        "3:\n\t"				               /*invalidate_caches_loop3*/     									\
        "orr     r11, r10, r9, LSL r5\n\t"     /*factor in the way number and cache number into R11*/			\
        "orr     r11, r11, r7, LSL r2\n\t"     /*factor in the index number*/									\
        "mcr     p15, 0, r11, c7, c6, 2\n\t"   /*DCISW - invalidate by set/way*/								\
        "subs    r9, r9, #1\n\t"               /*decrement the way number*/										\
        "bge     3b\n\t"					                    												\
        "subs    r7, r7, #1\n\t"               /*decrement the index*/											\
        "bge     2b\n\t"						                    											\
        "4:\n\t"							   /*invalidate_caches_skip*/										\
        "add     r10, r10, #2\n\t"             /*increment the cache number*/									\
        "cmp     r3, r10\n\t"																					\
        "bgt     1b\n\t"                    																	\
        "5:\n\t"							   /*invalidate_caches_finished*/									\
        "pop	{r4-r12}\n\t"																					\
        :::)
#endif /*__ASSEMBLY__*/

/*
 * Branch Predictor
 */
#define BPI_ENABLE() SET_SCTLR(SCTLR_Z_BIT)
#define BPI_DISABLE() CLEAR_SCTLR(SCTLR_Z_BIT)

#define DCacheLineSel() /* Cache size selection register. Select cache L0 D-cache in CSSR.*/\
					WRITE_CP15(2,0,c0,c0,0)

#define ICacheLineSel() /* Cache size selection register. Select cache L0 I-cache in CSSR.*/\
					WRITE_CP15(2,1,c0,c0,0)

#define CacheICIALLU() /*ICIALLU Invalidate all inst caches to PoU. Also flushes branch target cache.*/\
					WRITE_CP15(0,1,c7,c5,0)

#define CacheICIMVAU(Rd) /*ICIMVAU Invalidate instruction cache line by address to PoU.*/\
					WRITE_CP15(0,Rd,c7,c5,1)

#define CacheBPIALL() /*BPIALL Invalidate entire branch predictor array.*/\
					WRITE_CP15(0,1,c7,c5,6)

#define CacheBPIMVA() /*BPIMVA Invalidate address from branch predictor array.*/\
					WRITE_CP15(0,1,c7,c5,7)

#define CacheDCIMVAC(Rd) /*DCIMVAC Invalidate data or unified cache line by address to PoU.*/\
					WRITE_CP15(0,Rd,c7,c6,1)

#define CacheDCISW(Rd) /*DCISW Invalidate data or unified cache line by set/way.*/\
					WRITE_CP15(0,Rd,c7,c6,2)

#define CacheDCCMVAC() /*DCCMVAC Clean data or unified cache line by address to PoC.*/\
					WRITE_CP15(0,1,c7,c10,1)

#define CacheDCCSW() /*DCCSW Clean data or unified cache line by set/way.*/\
					WRITE_CP15(0,1,c7,c10,2)

#define CacheDCCMVAU() /*DCCMVAU Clean data or unified cache line by address to PoU.*/\
					WRITE_CP15(0,1,c7,c11,1)

#define CacheDCCIMVAC(Rd) /*DCCIMVAC Clean and invalidate data or unified cache line by address to PoC.*/\
					WRITE_CP15(0,Rd,c7,c14,1)

#define CacheDCCISW(Rd) /*DCCISW Clean and invalidate data or unified cache line by set/way.*/\
					WRITE_CP15(0,Rd,c7,c14,2)


#ifndef __ASSEMBLY__
extern void EnableCache(void);
extern void DisableCache(void);
extern void InvalidateCache(void);
extern void InvalidateL1DCache(void);
extern void InvalidateL1ICache(void);
extern void InvalidateL1DCacheRange(xmAddress_t addr, xm_u32_t size);
extern void InvalidateL1ICacheRange(xmAddress_t addr, xm_u32_t size);
extern void InvalidateL1DCacheLine(xmAddress_t addr);
extern void InvalidateL1ICacheLine(xmAddress_t addr);
extern void FlushCache(void);
extern void FlushDCache(void);
extern void FlushICache(void);
extern void FlushDCacheRange(xmAddress_t addr, xm_u32_t size);
extern void NSFlushCache(void);
#endif /*__ASSEMBLY__*/

#endif /* CONFIG_ENABLE_CACHE */

#endif /*_CORE_INCLUDE_DRIVERS_ZYNQ_ZYNQ_CACHE_H__*/
