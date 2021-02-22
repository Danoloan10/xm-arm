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
/*
 * Changelog:
 * - [11/12/15:XM-ARM-2:SPR-151211-01:#100] Configure the PLL_ARM to obtain the expected CPU_3x2x_CLOCK.
 * - [09/02/16:XM-ARM-2:SPR-151125-01:#97] Review PLL configuration for IO.
 * - [25/02/16:XM-ARM-2:SPR-160225-01:#111] Hardware not configured when OVPSIM/IMPERAS simulator is used.
 * - [16/06/16:CP-160616-01:#113] Clocks are configured by an external access so PLL configuration is not needed.
 * - [20/02/17:XM-ARM-2:CP-161214-01:#139] Enable BPU on ZYNQ.
 * - [21/02/17:XM-ARM-2:SPR-170221-01:#146] Initialize BPU on system reset.
 */

#include <arm/io.h>
#include <arm/asm.h>
#include <processor.h>
#include <vmmap.h>

#ifdef CONFIG_ZYNQ
#include <drivers/zynqboard/zynq_tzpc.h>
#include <drivers/zynqboard/zynq_mmu.h>
#ifdef CONFIG_ENABLE_CACHE
#include <drivers/zynqboard/zynq_cache.h>
#endif

static inline void mask_write(unsigned int addr, unsigned int mask, unsigned int val)
{
	__asm__ __volatile__("ldr  r1, [%0]\r\n"				\
						 "bic  r1, r1, %1\r\n"				\
						 "orr  r1, r1, %2\r\n"				\
						 "str  r1, [%0]\r\n"				\
						 :									\
						 :"r"(addr),"r"(mask),"r"(val)		\
						 :"r1");
}

static inline void mask_poll (unsigned int addr , unsigned int mask)
{
	__asm__ __volatile__("1:\n\t"							\
						 "ldr  r1, [%0]\n\t"				\
						 "and  r1, r1, %1\n\t"				\
						 "cmp  r1, #0\n\t"					\
						 "beq  1b\n\t"						\
						 :									\
						 :"r"(addr),"r"(mask)				\
						 :"r1");
}

static inline void SetupPLL(void)
{
    //PLL Configuration is assumed provided by an external agent.
}

static inline void MapClocks(void) {

}

static inline void InitACTLR(void) {

#ifdef CONFIG_SMP_SUPPORT
    SET_ACTLR(ACTLR_SMP_BIT);		/* Set SMP bit*/
#endif
#ifdef CONFIG_ENABLE_CACHE
    SET_ACTLR(ACTLR_FW_BIT);		/* Cache/TLB maintenance broadcast */
    SET_ACTLR(ACTLR_PARITY_BIT);	/* Enable parity*/
#endif

}

static inline void SystemInit(void) {
    SetupPLL();
    MapClocks();

    CA9_SCR_SECURE();
    DisableCache();
    CA9_SCR_NONSECURE();
    DisableCache();
    CA9_SCR_SECURE();

    InvalidateCache();				/* Invalidate cache */

    CA9_SCR_SECURE();
    DisableMMU();
    CA9_SCR_NONSECURE();
    DisableMMU();
    CA9_SCR_SECURE();

	InvalidateEntireUnifiedTLB();	/* Invalidate TLBs */

    CLIENT_DOMAIN_ACCES_CONTROL();

    CA9_TZ_CLEAN_DDR_ACCESSMAP();

    CA9_TZ_CLEAN_APB();

    HwDisableFpu();

    if (GET_CPU_ID()==0) {
        EnableScu();
		SetupDefaultVmMap();
    }
	InitMMU();
#ifdef CONFIG_ENABLE_CACHE
    EnableCache();
#endif

    InitACTLR();
}

void SystemReset(void) {
	CA9_SCR_SECURE();
	DisableCache();
	CA9_SCR_NONSECURE();
	DisableCache();
	CA9_SCR_SECURE();

	InvalidateCache();				/* Invalidate cache */

	CA9_SCR_SECURE();
	DisableMMU();
	CA9_SCR_NONSECURE();
	DisableMMU();
	CA9_SCR_SECURE();

	InvalidateEntireUnifiedTLB();	/* Invalidate TLBs */

    CLIENT_DOMAIN_ACCES_CONTROL();

    CA9_TZ_CLEAN_DDR_ACCESSMAP();

    CA9_TZ_CLEAN_APB();

    HwDisableFpu();

    if (GET_CPU_ID()==0) {
        EnableScu();
		SetupDefaultVmMap();
    }
	InitMMU();
#ifdef CONFIG_ENABLE_CACHE
	EnableCache();
#endif

    InitACTLR();
}

void InitCP15() {
}

void ZynqInit(void) {

	SystemInit();
    InitCP15();
}

#endif /*CONFIG_ZYNQ*/
