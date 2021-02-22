/*
 * $FILE: processor.h
 *
 * Processor functions
 *
 * $VERSION$
 *
 * Author: Miguel Masmano <mmasmano@fentiss.com>
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license terms.
 */

#ifndef _XM_PROCESSOR_H_
#define _XM_PROCESSOR_H_

#ifndef _XM_KERNEL_
#error Kernel file, do not include.
#endif

#include <irqs.h>
#include <smp.h>
#include <arch/processor.h>

extern void HaltSystem(void);
extern void ResetSystem(xm_u32_t resetMode);


extern void DumpState(cpuCtxt_t *regs);
extern void PartitionPanic(cpuCtxt_t *ctxt, xm_s8_t *fmt, ...);
extern void SystemPanic(cpuCtxt_t *ctxt, xm_s8_t *fmt, ...);
//extern void StackBackTrace(xm_u32_t);

extern xm_u32_t cpuKhz;
extern xm_u32_t GetCpuKhz(void);
extern void SetupCpu(void);
extern void SetupArchLocal(xm_s32_t cpuid);
extern void EarlySetupArchCommon(void);
extern void SetupArchCommon(void);
#if defined(CONFIG_MPU) && defined(CONFIG_AT697F)
extern void SetupSharedMemoryArea(void);
#endif

extern void HwEnableFpu(void);
extern void HwDisableFpu(void);
extern void HwSetFpuUserReady(void);
extern void HwSetFpuUserNotAllow(void);
extern xm_s32_t HwIsEnableFpu(void);
extern void HwSaveFpu(fpuCtxt_t * _fpuCtxt);
extern void HwRestoreFpu(fpuCtxt_t * _fpuCtxt);

#endif
