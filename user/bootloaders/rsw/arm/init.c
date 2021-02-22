/*
 * $FILE: init.c
 *
 * TI TMS570 board initialisation
 *
 * $VERSION$
 *
 * Author: Manuel Muñoz <mmunoz@ai2.upv.es>
 * Author: Javier O. Coronel <jcoronel@fentiss.com>
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE file for the license terms.
 */

//#include <arm/io.h>
//#include <arm/asm.h>
//#include <arm/processor.h>

#ifdef CONFIG_ZYNQ

/*
 * Cache Maintenance
 */
#ifndef __ASSEMBLY__
#define CACHES_ENABLE() \
	__asm__ __volatile__(	"mrc     p15, 0, r0, c1, c0, 0\n\t"		/*Read System Control Register configuration data*/ \
							"orr     r0, r0, #(1 << 2)\n\t"			/*Set C bit*/ \
							"orr     r0, r0, #(1 << 12)\n\t"		/*Set I bit*/ \
							"mcr     p15, 0, r0, c1, c0, 0\n\t"		/*Write System Control Register configuration data*/ \
						:::"r0")
#endif /*__ASSEMBLY__*/

#ifndef __ASSEMBLY__
#define CACHES_DISABLE() \
	__asm__ __volatile__(	"mrc     p15, 0, r0, c1, c0, 0\n\t"		/*Read System Control Register configuration data*/ \
							"bic     r0, r0, #(1 << 2)\n\t"			/*Clear C bit*/ \
							"bic     r0, r0, #(1 << 12)\n\t"		/*Clear I bit*/ \
							"mcr     p15, 0, r0, c1, c0, 0\n\t"		/*Write System Control Register configuration data*/ \
						:::"r0")
#endif /*__ASSEMBLY__*/

#ifndef __ASSEMBLY__
#define CACHES_INVALIDATE() \
		__asm__ __volatile__(	"push	{r4-r12}\n\t"																					\
								"mov     r0, #0\n\t"																					\
								"mcr     p15, 0, r0, c7, c5, 0\n\t"     /*ICIALLU - Invalidate entire I Cache, and flushes branch target cache*/\
								"mrc     p15, 1, r0, c0, c0, 1\n\t"     /*Read CLIDR*/													\
								"ands    r3, r0, #0x7000000\n\t"																		\
								"mov     r3, r3, LSR #23\n\t"           /*Cache level value (naturally aligned)*/						\
								"beq     invalidate_caches_finished\n\t"																\
								"mov     r10, #0\n\t"																					\
								"invalidate_caches_loop1:\n\t"																			\
								"add     r2, r10, r10, LSR #1\n\t"     /*Work out 3xcachelevel*/										\
								"mov     r1, r0, LSR r2\n\t"           /*bottom 3 bits are the Cache type for this level*/				\
								"and     r1, r1, #7\n\t"               /*get those 3 bits alone*/										\
								"cmp     r1, #2\n\t"																					\
								"blt     invalidate_caches_skip\n\t"  /*no cache or only instruction cache at this level*/				\
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
								"invalidate_caches_loop2:\n\t"																			\
								"mov     r9, R4\n\t"                   /*R9 working copy of the max way size (right aligned)*/			\
								"invalidate_caches_loop3:\n\t"																			\
								"orr     r11, r10, r9, LSL r5\n\t"     /*factor in the way number and cache number into R11*/			\
								"orr     r11, r11, r7, LSL r2\n\t"     /*factor in the index number*/									\
								"mcr     p15, 0, r11, c7, c6, 2\n\t"   /*DCISW - invalidate by set/way*/								\
								"subs    r9, r9, #1\n\t"               /*decrement the way number*/										\
								"bge     invalidate_caches_loop3\n\t"																	\
								"subs    r7, r7, #1\n\t"               /*decrement the index*/											\
								"bge     invalidate_caches_loop2\n\t"																	\
								"invalidate_caches_skip:\n\t"																			\
								"add     r10, r10, #2\n\t"             /*increment the cache number*/									\
								"cmp     r3, r10\n\t"																					\
								"bgt     invalidate_caches_loop1\n\t"																	\
								"invalidate_caches_finished:\n\t"																		\
								"pop	{r4-r12}\n\t"																					\
								:::)
#endif /*__ASSEMBLY__*/

/*
 * TLB
 */
#ifndef __ASSEMBLY__
#define TLB_INVALIDATE_UNIFIELD() \
	__asm__ __volatile__(	"mov	r0, #1\n\t"										\
							"mcr     p15, 0, r0, c8, c7, 0\n\t"		/*TLBIALL - Invalidate entire unified TLB*/ \
						:::"r0")
#endif /*__ASSEMBLY__*/

void ZynqInit(void)
{
	CACHES_DISABLE();

	CACHES_INVALIDATE();

	TLB_INVALIDATE_UNIFIELD();

	CACHES_ENABLE();
}

#endif /*CONFIG_ZYNQ*/
