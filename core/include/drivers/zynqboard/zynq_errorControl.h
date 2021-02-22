/*
 * $FILE: zynq_errorControl.h
 *
 * board definitions
 *
 * $VERSION$
 *
 * Author: Yolanda Valiente <yvaliente@fentiss.com>
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE file for the license terms.
 */
/* Changelog:
 */ 
#ifdef CONFIG_ZYNQ

#include <arch/gic.h>
#include <arch/irqs.h>

/*APU Interrupts*/
#define ZYNQ_L1CACHE_TLB_CPU0_IRQ	(32)  /* Any of the L1 instruction cache, L1 data cache, TLB, GHB, and BTAC parity errors from CPU 0 */
#define ZYNQ_L1CACHE_TLB_CPU1_IRQ	(33)  /* Any of the L1 instruction cache, L1 data cache, TLB, GHB, and BTAC parity errors from CPU 1 */
#define ZYNQ_L2CACHE_IRQ			(34)  /* Any errors, including parity errors, from the L2 controller */
#define ZYNQ_PMU_CPU0_IRQ			(37)  /* Performance monitor unit (PMU) of CPU0 */
#define ZYNQ_PMU_CPU1_IRQ			(38)  /* Performance monitor unit (PMU) of CPU1 */
#define ZYNQ_SCU_IRQ				(92)  /* Any of the parity errors from SCU generate a third interrupt */

#define PARITY_ERROR()				((L1CACHE_TLB_ERROR()) || (L2CACHE_ERROR()) || (SCU_ERROR()))
#define L1CACHE_TLB_ERROR()			((L1CACHE_TLB_CPU0_ERROR()) || (L1CACHE_TLB_CPU1_ERROR()))
#define L1CACHE_TLB_CPU0_ERROR()	((GET_GIC_SPI_STAT_0() & GIC_SPI_STAT_L1CACHE_TLB_CPU0_MASK) == GIC_SPI_STAT_L1CACHE_TLB_CPU0_MASK)
#define L1CACHE_TLB_CPU1_ERROR()	((GET_GIC_SPI_STAT_0() & GIC_SPI_STAT_L1CACHE_TLB_CPU1_MASK) == GIC_SPI_STAT_L1CACHE_TLB_CPU1_MASK)
#define L2CACHE_ERROR()				((GET_GIC_SPI_STAT_0() & GIC_SPI_STAT_L2CACHE_MASK) == GIC_SPI_STAT_L2CACHE_MASK)
#define SCU_ERROR()					((GET_GIC_SPI_STAT_1() & GIC_SPI_STAT_SCU_MASK) == GIC_SPI_STAT_SCU_MASK)

#ifndef __ASSEMBLY__
extern void SetupErrorControl(void);
extern int ErrorControlCheckParity(cpuCtxt_t *ctxt);
#endif /*__ASSEMBLY__*/

#endif /*CONFIG_ZYNQ*/
