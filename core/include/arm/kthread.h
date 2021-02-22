/*
 * $FILE: kthread.h
 *
 * Kernel, Guest or Layer0 thread
 *
 * Author: Miguel Masmano <mmasmano@fentiss.com>
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license terms.
 */

#ifndef _XM_ARCH_KTHREAD_H_
#define _XM_ARCH_KTHREAD_H_

//#include <irqs.h>
//#include <arch/processor.h>
#include <arch/xm_def.h>
//#include <arch/atomic.h>

#ifndef _XM_KERNEL_
#error Kernel file, do not include.
#endif

struct kThreadArch {
#if defined(CONFIG_MPU)
    xm_u32_t wpReg;
#else
    xmAddress_t ptdL1S;
    xmAddress_t ptdL1NS;
#endif
    xm_u32_t tbr;
#ifdef CONFIG_VFP_EXTENSION
/* Floating point registers */
//  xm_u32_t fpuRegs[XM_FPU_REG_NR] ALIGNED_C;
//    xm_u32_t fpuStatus;
    fpuCtxt_t fpuRegs;
#endif /*CONFIG_VFP_EXTENSION*/
    xmAddress_t entry;
    xm_u32_t nonsecure_vbar;
};

#endif /* _XM_ARCH_KTHREAD_H_ */
