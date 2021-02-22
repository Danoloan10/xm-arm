/*
 * $FILE: irqs.c
 *
 * Independent part of interrupt handling
 *
 * $VERSION$
 *
 * Author: Miguel Masmano <mmasmano@fentiss.com>
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license terms.
 */

#include <bitwise.h>
#include <boot.h>
#include <irqs.h>
#include <kthread.h>
#include <sched.h>
#include <stdc.h>
#include <processor.h>
#include <objects/hm.h>
#ifdef CONFIG_OBJ_STATUS_ACC
#include <objects/status.h>
#endif
#include <local.h>
#ifdef CONFIG_ENABLE_CACHE
#include <drivers/zynqboard/zynq_cache.h>
#endif


// Definitions
struct irqTabEntry irqHandlerTab[CONFIG_NO_HWIRQS];
exceptionHandler_t trapHandlerTab[NO_TRAPS];
hwIrqCtrl_t hwIrqCtrl[CONFIG_NO_HWIRQS];

void DefaultIrqHandler(cpuCtxt_t *ctxt, void *data) {
	localSched_t *sched=GET_LOCAL_SCHED();
	struct xmcPartition *cfg=GetPartitionCfg(sched->cKThread);
	xmHmLog_t hmLog;

    memset(&hmLog, 0, sizeof(xmHmLog_t));
    hmLog.opCode|=XM_HM_EV_UNEXPECTED_TRAP<<HMLOG_OPCODE_EVENT_BIT;

    hmLog.opCode|=(ctxt->irqNr&HMLOG_OPCODE_MODID_MASK)<<HMLOG_OPCODE_MODID_BIT;

    if (sched->cKThread->ctrl.g)
        hmLog.opCode|=cfg->rId<<HMLOG_OPCODE_PARTID_BIT;
    hmLog.opCode|=HMLOG_OPCODE_SYS_MASK;

    CpuCtxt2HmCpuCtxt(ctxt, &hmLog.cpuCtxt);
    hmLog.opCode|=HMLOG_OPCODE_VALID_CPUCTXT_MASK;
    kprintf("Unexpected irq %d\n", ctxt->irqNr);
    HmRaiseEvent(&hmLog);
}

static void TriggerIrqHandler(cpuCtxt_t *ctxt, void *data) {
    xmId_t partId;
    partId=xmcTab.hpv.hwIrqTab[ctxt->irqNr].owner;

    SetPartitionHwIrqPending(&partitionTab[partId], ctxt->irqNr);
}

void SetTrapPending(cpuCtxt_t *ctxt) {
    localProcessor_t *info=GET_LOCAL_PROCESSOR();
    ASSERT(!AreKThreadFlagsSet(info->sched.cKThread, KTHREAD_TRAP_PENDING_F));
    SetKThreadFlags(info->sched.cKThread, KTHREAD_TRAP_PENDING_F);
}

/*static inline xmAddress_t IsInPartExTable(xmAddress_t addr) {
    extern struct exPTable {
        xmAddress_t a;
        xmAddress_t b;
    } exPTable[]; 
    struct exPTable *exPTablePtr;
    xm_s32_t e;
    
    for (exPTablePtr=exPTable; exPTablePtr; exPTablePtr=(struct exPTable *)exPTablePtr[e].b)
    {
        for (e=0; exPTablePtr[e].a; e++)
        {   if (addr==exPTablePtr[e].a)
            {    return exPTablePtr[e].b;}
        }
    }
    return 0;
}*/

void DoTrap(cpuCtxt_t *ctxt) {
	localSched_t *sched=GET_LOCAL_SCHED();
	struct xmcPartition *cfg=GetPartitionCfg(sched->cKThread);
	xmHmLog_t hmLog;
#ifndef CONFIG_CORTEX_A9
	xm_s32_t action;
#endif
    ASSERT(ctxt->irqNr<NO_TRAPS);
/*	xmWord_t pc;

	kprintf("PC=0x%x ctxtpc=0x%x\n", IsInPartExTable(GET_CTXT_PC(ctxt)), GET_CTXT_PC(ctxt));
    if ((pc=IsInPartExTable(GET_CTXT_PC(ctxt)))) {
        SET_CTXT_PC(ctxt, pc);
        return;
    }
*/

#ifdef CONFIG_ZYNQ
    if (ErrorControlCheckParity(ctxt))
    	return;
#endif /*CONFIG_ZYNQ*/

    if (trapHandlerTab[ctxt->irqNr])
        if (trapHandlerTab[ctxt->irqNr](ctxt))
            return;

	memset(&hmLog, 0, sizeof(xmHmLog_t));
	hmLog.opCode|=
			((ctxt->irqNr+XM_HM_MAX_GENERIC_EVENTS)<<HMLOG_OPCODE_EVENT_BIT);
	if (IsSvIrqCtxt(ctxt)) {
		if (sched->cKThread->ctrl.g)
		{	hmLog.opCode|=(cfg->rId<<HMLOG_OPCODE_PARTID_BIT);}
		if (ArchTrapIsSysCtxt(ctxt))
		{	hmLog.opCode|=HMLOG_OPCODE_SYS_MASK;}
	} else {
		hmLog.opCode|=(cfg->rId<<HMLOG_OPCODE_PARTID_BIT);
	}

    CpuCtxt2HmCpuCtxt(ctxt, &hmLog.cpuCtxt);
    hmLog.opCode|=HMLOG_OPCODE_VALID_CPUCTXT_MASK;
#ifdef CONFIG_VERBOSE_TRAP
    DumpState(ctxt);
#endif
#ifndef CONFIG_CORTEX_A9
    action =
#endif
             HmRaiseEvent(&hmLog);
    if (IsSvIrqCtxt(ctxt)&&((hmLog.opCode&HMLOG_OPCODE_SYS_MASK)!=HMLOG_OPCODE_SYS_MASK))
        PartitionPanic(ctxt,"Partition in unrecoverable state\n");
    if (!IsSvIrqCtxt(ctxt)) {
#ifndef CONFIG_CORTEX_A9
        if (action)
	{	SetTrapPending(ctxt); }
#endif
} else
	{SystemPanic(ctxt,"Unexpected/unhandled trap - TRAP: 0x%x ERROR CODE: 0x%x\n", sched->cKThread->ctrl.g->partCtrlTab->trap2Vector[ctxt->irqNr], GET_ECODE(ctxt));}
}

void DoUnrecovExcp(cpuCtxt_t *ctxt) {
	localSched_t *sched=GET_LOCAL_SCHED();
	struct xmcPartition *cfg=GetPartitionCfg(sched->cKThread);
	xmHmLog_t hmLog;

	memset(&hmLog, 0, sizeof(xmHmLog_t));
	hmLog.opCode|=(XM_HM_EV_PARTITION_UNRECOVERABLE<<HMLOG_OPCODE_EVENT_BIT);
	if (IsSvIrqCtxt(ctxt)) {
		if (sched->cKThread->ctrl.g)
			{hmLog.opCode|=(cfg->rId<<HMLOG_OPCODE_PARTID_BIT);}
		hmLog.opCode|=HMLOG_OPCODE_SYS_MASK;
	} else {
		hmLog.opCode|=(cfg->rId<<HMLOG_OPCODE_PARTID_BIT);
	}

    CpuCtxt2HmCpuCtxt(ctxt, &hmLog.cpuCtxt);
    hmLog.opCode|=HMLOG_OPCODE_VALID_CPUCTXT_MASK;
    DumpState(ctxt);
    HmRaiseEvent(&hmLog);

    PartitionPanic(ctxt,"Partition unrecoverable error : 0x%x\n", ctxt->irqNr);
}

void DoUnexpectedTrap(cpuCtxt_t * ctxt)
{
    kprintf("UnexpectedTrap %d\n", ctxt->irqNr);
    xmHmLog_t hmLog;

    memset(&hmLog, 0, sizeof(xmHmLog_t));
    hmLog.opCode|=XM_HM_EV_UNEXPECTED_TRAP<<HMLOG_OPCODE_EVENT_BIT;
    hmLog.opCode|=(ctxt->irqNr&HMLOG_OPCODE_MODID_MASK)<<HMLOG_OPCODE_MODID_BIT;
    hmLog.opCode|=HMLOG_OPCODE_PARTID_MASK;
    hmLog.opCode|=HMLOG_OPCODE_SYS_MASK;

    CpuCtxt2HmCpuCtxt(ctxt, &hmLog.cpuCtxt);
    hmLog.opCode|=HMLOG_OPCODE_VALID_CPUCTXT_MASK;
    HmRaiseEvent(&hmLog);
}

void DoIrq(cpuCtxt_t *ctxt)
{
    localProcessor_t *info = GET_LOCAL_PROCESSOR();
#ifdef CONFIG_ARM
    xm_u32_t receivedIrq = (ctxt->irqNr);
    xm_u32_t currIrq = (receivedIrq & 0x000003ff);
#else
    xm_u32_t currIrq = (ctxt->irqNr);
#endif /*CONFIG_ARM*/
    //localCpu_t *cpu=GET_LOCAL_CPU();
    ASSERT(!HwIsSti());
#ifdef CONFIG_OBJ_STATUS_ACC
    systemStatus.noIrqs++;
#endif /*CONFIG_OBJ_STATUS_ACC*/
    info->cpu.irqNestingCounter++;
#ifndef CONFIG_ARM
    HwAckIrq(currIrq);
#endif /*CONFIG_ARM*/

    if (irqHandlerTab[currIrq].handler)
    {
        (*(irqHandlerTab[currIrq].handler))(ctxt, irqHandlerTab[currIrq].data);
    }
    else
    {
        DefaultIrqHandler(ctxt, 0);
    }

#ifdef CONFIG_ARM
    HwEndIrq(receivedIrq);
#endif /*CONFIG_ARM*/
#ifdef CONFIG_OBJ_STATUS_ACC
    systemStatus.irqLastOccurence[currIrq]=GetSysClockUsec();
#endif /*CONFIG_OBJ_STATUS_ACC*/
    info->cpu.irqNestingCounter--;
    do
    {
        Schedule();
    } while (info->cpu.irqNestingCounter == SCHED_PENDING);
    ASSERT(!HwIsSti());
    ASSERT(!(info->cpu.irqNestingCounter & SCHED_PENDING));
}

void __VBOOT SetupIrqs(void) {
    xm_s32_t irqNr;

    for (irqNr=0; irqNr<CONFIG_NO_HWIRQS; irqNr++) {
	if (xmcTab.hpv.hwIrqTab[irqNr].owner!=XM_IRQ_NO_OWNER) {
	    irqHandlerTab[irqNr]=(struct irqTabEntry){
		.handler=TriggerIrqHandler, 
		.data=0,
	    };
	} else  {
	    irqHandlerTab[irqNr]=(struct irqTabEntry){
		.handler=DefaultIrqHandler, 
		.data=0,
	    };
	}
    }

    for (irqNr=0; irqNr<NO_TRAPS; irqNr++)
	{trapHandlerTab[irqNr]=0;}

    ArchSetupIrqs();
}

irqHandler_t SetIrqHandler(xm_s32_t irq, irqHandler_t irqHandler, void *data) {
    irqHandler_t oldHandler=irqHandlerTab[irq].handler;
    if (irqHandler) {
	irqHandlerTab[irq]=(struct irqTabEntry){
	    .handler=irqHandler,
	    .data=data,
	};
    } else
	irqHandlerTab[irq]=(struct irqTabEntry){
	    .handler=DefaultIrqHandler,
	    .data=0,
	};
    return oldHandler;
}

exceptionHandler_t SetTrapHandler(xm_s32_t trap, exceptionHandler_t trapHandler) {
    exceptionHandler_t oldHandler=trapHandlerTab[trap];

    trapHandlerTab[trap]=trapHandler;
    return oldHandler;
}

static inline xm_s32_t AreHwIrqsPending(partitionControlTable_t *partCtrlTab) {
	xm_s32_t eIrq=0, e;

    for (e=0; e<HWIRQS_VECTOR_SIZE; e++)
    {
    	if ((eIrq=((partCtrlTab->hwIrqsPend[e])&(~partCtrlTab->hwIrqsMask[e]))))
    	{   break; }
    }
    if (eIrq)
    {
#ifdef CONFIG_HWIRQ_PRIO_FBS
        eIrq=_Ffs(eIrq);
#else
        eIrq=_Fls(eIrq);
#endif
		ASSERT((eIrq>=0)&&(eIrq<CONFIG_NO_HWIRQS));
		return eIrq;
	}
	return -1;
}

static inline xm_s32_t AreExtIrqsPending(partitionControlTable_t *partCtrlTab) {
    xm_s32_t eIrq;
    eIrq=partCtrlTab->extIrqsPend&~partCtrlTab->extIrqsMask;
    if (eIrq) {
#ifdef CONFIG_HWIRQ_PRIO_FBS
        eIrq=_Ffs(eIrq);
#else
        eIrq=_Fls(eIrq);
#endif
	return eIrq;
    }       
  
    return -1;
}

static inline xm_s32_t AreExtTrapsPending(partitionControlTable_t *partCtrlTab) {
    xm_s32_t eIrq;

    eIrq=partCtrlTab->extIrqsPend&XM_EXT_TRAPS;
    if (eIrq) {
#ifdef CONFIG_HWIRQ_PRIO_FBS
        eIrq=_Ffs(eIrq);
#else
        eIrq=_Fls(eIrq);
#endif
	return eIrq;
    }       
  
    return -1;
}

/*TODO ARMPorting: to be tested*/
xm_s32_t RaisePendIrqs(cpuCtxt_t *ctxt) {
	/*TOBEAPPROVED ARMPorting: Get directly Sched instead info*/
	localSched_t *sched=GET_LOCAL_SCHED();
	partitionControlTable_t *partCtrlTab;
	xm_s32_t eIrq, emul;

	if (!sched->cKThread->ctrl.g||IsSvIrqCtxt(ctxt))
		return ~0;

    // SwTrap
#ifndef CONFIG_ARM
    if (sched->cKThread->ctrl.g->swTrap&0x1) {
        emul=sched->cKThread->ctrl.g->swTrap>>1;
        sched->cKThread->ctrl.g->swTrap=0;
        return IrqVector2Address(emul);
    }
#endif
    
    partCtrlTab=sched->cKThread->ctrl.g->partCtrlTab;
    // 1) Check pending traps
    if (AreKThreadFlagsSet(sched->cKThread, KTHREAD_TRAP_PENDING_F)) {
        ClearKThreadFlags(sched->cKThread, KTHREAD_TRAP_PENDING_F);
//        DisablePCtrlTabIrqs(&partCtrlTab->iFlags); /*iFlag is not mantained in ARM*/
        emul=ArchEmulTrapIrq(ctxt, partCtrlTab, ctxt->irqNr);
    	FlushDCacheRange(((xmAddress_t)partCtrlTab), sizeof(partitionControlTable_t));
        return IrqVector2Address(emul);
    }

    // 2) Check pending extended trap
    if ((eIrq=AreExtTrapsPending(partCtrlTab))>-1) {
        partCtrlTab->extIrqsPend&=~(1<<eIrq);
//        DisablePCtrlTabIrqs(&partCtrlTab->iFlags);/*iFlag is not mantained in ARM*/
        emul=ArchEmulExtIrq(ctxt, partCtrlTab, eIrq);
    	FlushDCacheRange(((xmAddress_t)partCtrlTab), sizeof(partitionControlTable_t));
        return IrqVector2Address(emul);
    }

#ifndef CONFIG_ARM
    // At this moment irq flags must be set /*iFlag is not mantained in ARM*/
    if (!ArePCtrlTabIrqsSet(partCtrlTab->iFlags))
        return ~0;
#endif

    // 3) Check pending hwirqs
    if ((eIrq=AreHwIrqsPending(partCtrlTab))>-1) {
        xmClearBit(partCtrlTab->hwIrqsPend, eIrq, CONFIG_NO_HWIRQS);
//        partCtrlTab->hwIrqsPend&=~(1<<eIrq);
        MaskPCtrlTabIrq(((xm_u32_t*)(&(partCtrlTab->hwIrqsMask))), (1<<eIrq));
//        DisablePCtrlTabIrqs(&partCtrlTab->iFlags); /*iFlag is not mantained in ARM*/
        emul=ArchEmulHwIrq(ctxt, partCtrlTab, eIrq);
    	FlushDCacheRange(((xmAddress_t)partCtrlTab), sizeof(partitionControlTable_t));
        return IrqVector2Address(emul);
    }

     // 4) Check pending extirqs
    if ((eIrq=AreExtIrqsPending(partCtrlTab))>-1) {
//        partCtrlTab->extIrqsPend&=~(1<<eIrq);
        MaskPCtrlTabIrq( ((xm_u32_t*)(&(partCtrlTab->extIrqsMask))), (1<<eIrq));
//        DisablePCtrlTabIrqs(&partCtrlTab->iFlags); /*iFlag is not mantained in ARM*/
        emul=ArchEmulExtIrq(ctxt, partCtrlTab, eIrq);
    	FlushDCacheRange(((xmAddress_t)partCtrlTab), sizeof(partitionControlTable_t));
        return IrqVector2Address(emul);
    }

    // No emulation required
	FlushDCacheRange(((xmAddress_t)partCtrlTab), sizeof(partitionControlTable_t));
    return ~0;
}
