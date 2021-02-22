/*
 * $FILE: gen_offsets.h
 *
 * ASM offsets, this file only can be included from asm-offset.c
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
 * - [22/08/16:XM-ARM-2:SPR-160819-01:#116] Add localProcessorInfo struct size to access the array in SMP.
 */

#ifndef __GEN_OFFSETS_H_
#define _GEN_OFFSETS_H_
#ifndef _GENERATE_OFFSETS_
#error Do not include this file
#endif

#include <arch/irqs.h>
#include <objects/commports.h>
#include <objects/console.h>
#include <objects/status.h>
#include <objects/trace.h>
#include <drivers/memblock.h>
#include <arch/atomic.h>
#include <kthread.h>
#include <queue.h>
#include <sched.h>
#include <kdevice.h>
#include <physmm.h>
#include <local.h>

static inline void GenerateOffsets(void) {
    DEFINE(localSched, offsetof(localProcessor_t, sched), );

	DEFINE(cKThread, offsetof(localProcessor_t, sched)+offsetof(localSched_t, cKThread), );
#ifdef CONFIG_VFP_EXTENSION
    DEFINE(fpuOwner, (offsetof(localProcessor_t, sched)+offsetof(localSched_t, fpuOwner)), );
#endif /*CONFIG_VFP_EXTENSION*/
    // kthread_t
    DEFINE(ctrl, offsetof(kThread_t, ctrl),);

    // struct __kthread
    DEFINE(flags, offsetof(struct __kThread, flags),);
    DEFINE(g, offsetof(struct __kThread, g),);
    DEFINE(kStack, offsetof(struct __kThread, kStack),);
    DEFINE(irqCpuCtxt, offsetof(struct __kThread, irqCpuCtxt),);
//
//    // struct guest
//    DEFINE(partCtrlTab, offsetof(struct guest, partCtrlTab),);
    DEFINE(kArch, offsetof(struct guest, kArch),);
//    DEFINE(swTrap, offsetof(struct guest, swTrap),);
//
//    // struct xmcPartition
//    DEFINE(hwIrqs, offsetof(struct xmcPartition,hwIrqs),);
//    DEFINE(cfg_flags, offsetof(struct xmcPartition, flags),);
    DEFINE(irqIndex, offsetof(partitionControlTable_t, irqIndex),);

//    // gctrl_t
//    DEFINE(iFlags, offsetof(partitionControlTable_t, iFlags),);
//    DEFINE(hwIrqsPend, offsetof(partitionControlTable_t, hwIrqsPend),);
//    DEFINE(hwIrqsMask, offsetof(partitionControlTable_t, hwIrqsMask),);
//    DEFINE(extIrqsPend, offsetof(partitionControlTable_t, extIrqsPend),);
//    DEFINE(extIrqsMask, offsetof(partitionControlTable_t, extIrqsMask),);
    DEFINE(idPart, offsetof(partitionControlTable_t, id),);
//
//    // xm_atomic_t
//    DEFINE(val, offsetof(xmAtomic_t, val),);
//
    // struct kthread_arch
    DEFINE(tbr, offsetof(struct kThreadArch, tbr),);

#ifdef CONFIG_VFP_EXTENSION
    DEFINE(fpuRegs, offsetof(struct kThreadArch, fpuRegs),);
#endif /*CONFIG_VFP_EXTENSION*/
    DEFINE(centry, offsetof(struct guest, kArch)+offsetof(struct kThreadArch, entry),);
//
//    // struct  irqTabEntry
//    DEFINE(handler, offsetof(struct irqTabEntry, handler), );
//    DEFINE(data, offsetof(struct irqTabEntry, data), );
//
//    // struct xmc hpv
//    DEFINE(hpv, offsetof(struct xmc, hpv), );
//
//    // struct xmcHpv cpuTab
//    DEFINE(cpuTab, offsetof(struct xmcHpv, cpuTab), );
//
//    //  struct cpu features
//    DEFINE(features, offsetof(struct cpu, features), );
//
    // struct partition
//    DEFINE(partition_cfg, offsetof(partition_t, cfg), );
//

// cpuCtxt_t_t fields
    DEFINE(ctxtIrqNr, offsetof( cpuCtxt_t, irqNr),);
//    DEFINE(prev, offsetof(cpuCtxt_t, prev), );
    DEFINE(ctxtsp, offsetof(cpuCtxt_t, sp), );
    DEFINE(ctxtspsr, offsetof(cpuCtxt_t, spsr), );
    DEFINE(ctxtcpsr, offsetof(cpuCtxt_t, cpsr), );
    DEFINE(ctxttrapsp, offsetof(cpuCtxt_t, trapSp), );
    DEFINE(ctxtlr, offsetof(cpuCtxt_t, lr), );
    DEFINE(ctxtpc, offsetof(cpuCtxt_t, pc), );
    DEFINE(ctxtr0, offsetof(cpuCtxt_t, r), );
    DEFINE(ctxtr1, offsetof(cpuCtxt_t, r), );
    DEFINE(ctxtr2, offsetof(cpuCtxt_t, r), );
    DEFINE(ctxtr3, offsetof(cpuCtxt_t, r), );
    DEFINE(ctxtr4, offsetof(cpuCtxt_t, r), );
    DEFINE(ctxtr5, offsetof(cpuCtxt_t, r), );
    DEFINE(ctxtr6, offsetof(cpuCtxt_t, r), );
    DEFINE(ctxtr7, offsetof(cpuCtxt_t, r), );
    DEFINE(ctxtr8, offsetof(cpuCtxt_t, r), );
    DEFINE(ctxtr9, offsetof(cpuCtxt_t, r), );
    DEFINE(ctxtr10, offsetof(cpuCtxt_t, r), );
    DEFINE(ctxtr11, offsetof(cpuCtxt_t, r), );
    DEFINE(ctxtr12, offsetof(cpuCtxt_t, r), );

    // sizeof
    DEFINE2(kthread_t,  sizeof(kThread_t), );
    DEFINE2(kthreadptr_t,  sizeof(kThread_t *), );
    DEFINE2(partition_t, sizeof(partition_t), );
    DEFINE2(struct_guest,  sizeof(struct guest), );
    DEFINE2(kdevice_t, sizeof(kDevice_t), );
    DEFINE2(struct_memblockdata, sizeof(struct memBlockData), );
    DEFINE2(struct_console, sizeof(struct console), );
    DEFINE2(xmPartitionStatus_t, sizeof(xmPartitionStatus_t), );
    DEFINE2(union_channel, sizeof(union channel), );
    DEFINE2(struct_port, sizeof(struct port), );
    DEFINE2(struct_msg, sizeof(struct msg), );
//    DEFINE2(struct_physpageptr, sizeof(struct physPage *), );
//    DEFINE2(struct_physpage, sizeof(struct physPage), );
    DEFINE2(struct_scheddata, sizeof(struct schedData), );
    DEFINE2(struct_queue, sizeof(struct queue), );
    DEFINE2(struct_xmtraceevent, sizeof(struct xmTraceEvent), );
    DEFINE2(struct_xmhmlog, sizeof(struct xmHmLog), );
//    DEFINE2(cpuctxt, sizeof(cpuCtxt_t), );
//#ifdef CONFIG_AMP_SUPPORT
//    DEFINE2(struct_channelctrlsampling, sizeof(struct channelCtrlSampling), );
//    DEFINE2(struct_channelctrlqueuing, sizeof(struct channelCtrlQueuing), );
//#endif
#ifdef CONFIG_SMP_SUPPORT
    DEFINE2(localProcessor_t, sizeof(localProcessor_t), );
#endif
}

#endif
