/*
 * $FILE: processor.c
 *
 * Processor
 *
 * $VERSION$
 *
 * Author: Miguel Masmano <mmasmano@fentiss.com>
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license terms.
 */
/*
 * Changelog:
 * - [10/12/15:XM-ARM-2:SPR-150715-03:#73] Add function to manage the cache state.
 * - [15/01/16:XM-ARM-2:SPR-160108-01:#101] Update cache management function.
 */

#include <assert.h>
#include <boot.h>
#include <linkage.h>
#include <kdevice.h>
#include <processor.h>
#include <physmm.h>
#include <sched.h>
#include <stdc.h>
#include <xmconf.h>

#include <arch/xm_def.h>

xm_u32_t GetCpuKhz(void)
{
	if (xmcTab.hpv.cpuTab[GET_CPU_ID()].freq == XM_CPUFREQ_AUTO)
	{
		PWARN("Clock freq must not be set as auto in ARM\n");
		HaltSystem();
	}
	return xmcTab.hpv.cpuTab[GET_CPU_ID()].freq;
}

/*#define PCR_CPUID_MASK 0xF0000000
#define PCR_CPUID_SHIFT 28*/

xm_u32_t __GetCpuHwId(void)
{
#ifdef CONFIG_SMP_SUPPORT
	xm_u32_t aux;
	CA9_GET_MPIDR(aux);
    return (CA9_GET_PROCESSOR_ID_MASK & aux);
#else
    return 0;
#endif
}

xm_u32_t __GetCpuId(void)
{
	return  __GetCpuHwId();
}

void __SetCpuId(xm_u32_t id) {
}

void __SetCpuHwId(xm_u32_t hwId) {
}

void __VBOOT SetupCpu(void) {
}

#ifdef CONFIG_VFP_EXTENSION
#define EN_VFP_BIT (1<<30) /*Enable VFP extensions on FPEXC*/

void HwEnableFpu(void)
{
//  unsigned int tmp;
    CA9_NSACR_CP10CP11_NS();
    CLEARNSET_CPACR(CP_FP_EN_FULL, CP_FP_MASK);
//  GET_CPACR(tmp);
//  tmp&=~CP_FP_MASK;
//  tmp|=CP_FP_EN_FULL;
//  SET_CPACR(tmp);

    FPEXC_EN_SET();
    /*  __asm__ __volatile__("fmrx %0, FPEXC\n\t":"=r"(tmp)::"memory");
     tmp|=EN_VFP_BIT;
     __asm__ __volatile__("fmxr FPEXC,%0\n\t"::"r"(tmp):"memory");*/
}

void HwDisableFpu(void)
{
    CA9_NSACR_CP10CP11_SONLY();
    CLEARNSET_CPACR(CP_FP_EN_PRIV, CP_FP_MASK);
//  unsigned int tmp;
//  GET_CPACR(tmp);
//  tmp&=~CP_FP_MASK;
//  tmp|=CP_FP_EN_PRIV;
//  SET_CPACR(tmp);
    FPEXC_EN_CLR();
    /*  __asm__ __volatile__("fmrx %0, FPEXC\n\t":"=r"(tmp)::"memory");
     tmp&=~EN_VFP_BIT;
     __asm__ __volatile__("fmxr FPEXC,%0\n\t"::"r"(tmp):"memory");*/
}

void HwSetFpuUserReady(void)
{
//  unsigned int tmp;
    CA9_NSACR_CP10CP11_NS();
    CLEARNSET_CPACR(CP_FP_EN_FULL, CP_FP_MASK);
//  GET_CPACR(tmp);
//  tmp&=~CP_FP_MASK;
//  tmp|=CP_FP_EN_FULL;
//  SET_CPACR(tmp);

//    FPEXC_EN_SET();
    /*  __asm__ __volatile__("fmrx %0, FPEXC\n\t":"=r"(tmp)::"memory");
     tmp|=EN_VFP_BIT;
     __asm__ __volatile__("fmxr FPEXC,%0\n\t"::"r"(tmp):"memory");*/
}

void HwSetFpuUserNotAllow(void)
{
    CA9_NSACR_CP10CP11_SONLY();
    CLEARNSET_CPACR(CP_FP_EN_PRIV, CP_FP_MASK);
//  unsigned int tmp;
//  GET_CPACR(tmp);
//  tmp&=~CP_FP_MASK;
//  tmp|=CP_FP_EN_PRIV;
//  SET_CPACR(tmp);
//    FPEXC_EN_CLR();
    /*  __asm__ __volatile__("fmrx %0, FPEXC\n\t":"=r"(tmp)::"memory");
     tmp&=~EN_VFP_BIT;
     __asm__ __volatile__("fmxr FPEXC,%0\n\t"::"r"(tmp):"memory");*/
}

xm_s32_t HwIsEnableFpu(void)
{
    unsigned int tmp;
    GET_CPACR(tmp);
    if ((tmp & CP_FP_MASK) == CP_FP_EN_FULL)
        return 0;
    return -1;
}

void HwSaveFpu(fpuCtxt_t * _fpuCtxt)
{
    __asm__ __volatile__ (                 \
          /*"ldr r1, [%0]           \n\t"*/\
            "mov r1, %0             \n\t"  \
            "fmrx r4, FPEXC         \n\t"  \
            "mov r5, r4             \n\t"  \
            "orr r5, r5, #0x40000000\n\t"  \
            "fmxr FPEXC, r5         \n\t"  \
            "fmrx r5, FPSCR         \n\t"  \
            "stmia r1!,{r4,r5}      \n\t"  \
            "vstmia.32 r1!,{s0-s15} \n\t"  \
            "vstmia.32 r1!,{s16-s31}\n\t"  \
            "vstmia.64 r1!,{d0-d15} \n\t"  \
            "vstmia.64 r1!,{d16-d31}\n\t"  \
            "vstmia    r1!,{q0-q7}  \n\t"  \
            "vstmia    r1!,{q8-q15} \n\t"  \
            :                              \
            :"r"(_fpuCtxt)                 \
            :"r1","r4","r5");

//ldr r1, [r3, #_CTRL_OFFSET+_G_OFFSET]
//add r1, r1, #_FPUREGS_OFFSET+_KARCH_OFFSET
//fmrx r4, FPEXC
//fmrx r5, FPSCR
//stmia r1!,{r4,r5}
//vstmia.32 r1!,{s0-s15}
//vstmia.32 r1!,{s16-s31}
//vstmia.64 r1!,{d0-d15}
}

void HwRestoreFpu(fpuCtxt_t * _fpuCtxt)
{
    __asm__ __volatile__ (                 \
          /*"ldr r1, [%0]           \n\t"*/\
            "mov r1, %0             \n\t"  \
            "fmrx r4, FPEXC         \n\t"  \
            "orr r4, r4, #0x40000000\n\t"  \
            "fmxr FPEXC, r4         \n\t"  \
            "ldmia r1!,{r4,r5}      \n\t"  \
            "vldmia.32 r1!,{s0-s15} \n\t"  \
            "vldmia.32 r1!,{s16-s31}\n\t"  \
            "vldmia.64 r1!,{d0-d15} \n\t"  \
            "vldmia.64 r1!,{d16-d31}\n\t"  \
            "vldmia    r1!,{q0-q7}  \n\t"  \
            "vldmia    r1!,{q8-q15} \n\t"  \
            "fmxr FPSCR, r5         \n\t"  \
            "fmxr FPEXC, r4         \n\t"  \
            :                              \
            :"r"(_fpuCtxt)                 \
            :"r1","r4","r5");

//str r2, [r0, #_FPUOWNER_OFFSET]
//ldr r1, [r2, #_CTRL_OFFSET+_G_OFFSET]
//add r1, r1, #_FPUREGS_OFFSET+_KARCH_OFFSET
//ldmia r1!,{r4,r5}
//vldmia.32 r1!,{s0-s15}
//vldmia.32 r1!,{s16-s31}
//vldmia.64 r1!,{d0-d15}
//fmxr FPSCR, r5
}
#endif /*CONFIG_VFP_EXTENSION*/


void __VBOOT EarlySetupCpu(void) {
	cpuKhz=GetCpuKhz();
}

xm_u32_t __GetNoCpus(void)
{
	return ( ((CA9_READ_SCU_CONFIG() & CA9_SCU_CPUS_NUMBER_MASK)) + 1 );
}

void EnableScu(void)
{
	CA9_SCU_PARITY_ENABLE();
	CA9_SCU_SPECLINEFILLS_ENABLE();

    CA9_SCU_INVALIDATE();		/* Invalidate scu */
	CA9_SCU_ENABLE();
}
