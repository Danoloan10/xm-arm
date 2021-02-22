/*
 * $FILE: irqs.c
 *
 * IRQS' code
 *
 * $VERSION$
 *
 * Author: Miguel Masmano <mmasmano@fentiss.com>
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license terms.
 */

#ifdef CONFIG_ARM

#include <assert.h>
#include <bitwise.h>
#include <irqs.h>
#include <kthread.h>
#include <physmm.h>
#include <processor.h>
#include <sched.h>
#include <stdc.h>
#include <arch/gic.h>
#include <local.h>

xm_s32_t ArchTrapIsSysCtxt(cpuCtxt_t *ctxt) {
    return 0;
}

#ifdef NOT_IMPLEMENTED_YET
#if defined(CONFIG_MMU)
static xm_s32_t ArmTrapPageFault(cpuCtxt_t *ctxt)
{
	localSched_t *sched=GET_LOCAL_SCHED();
	xm_u32_t faultAddress=GetMmuFaultAddressReg();

    sched->cKThread->ctrl.g->partCtrlTab->arch.faultStatusReg=GetMmuFaultStatusReg();
    sched->cKThread->ctrl.g->partCtrlTab->arch.faultAddressReg=faultAddress;
    return 0;
}
#endif
#endif

void ArchEmulPendIrqs(cpuCtxt_t *ctxt, xm_u32_t irqNr)
{
	XM_SEND_EXTIRQ();
}

void ArchEmulPendException(cpuCtxt_t *ctxt, xm_u32_t irqNr) {}

xmAddress_t IrqVector2Address(xm_s32_t vector)
{
    localSched_t *sched=GET_LOCAL_SCHED();
    xmAddress_t *tbr=(xmAddress_t *)sched->cKThread->ctrl.g->partCtrlTab->arch.tbr;
    cpuCtxt_t *ctxt=sched->cKThread->ctrl.irqCpuCtxt;
    if (CheckGParam(tbr, 256*16, 4, 0)<0)
        PartitionPanic(ctxt,"Partition unrecoverable error: incorrect TBR address (0x%x)\n", tbr);
    return (xmAddress_t)&(tbr[vector*4]);
}

void ArchSetupIrqs(void)
{
    InitGIC(GET_CPU_ID());

#if defined(CONFIG_MMU)
    /*TODO ARMPorting: Comment for build, not implemented yet */
#ifdef NOT_IMPLEMENTED_YET
    SetTrapHandler(4, ArmTrapPageFault);
    SetTrapHandler(1, ArmTrapPageFault);
    SetTrapHandler(16, ArmTrapPageFault);
    SetTrapHandler(15, ArmTrapPageFault);
#endif
#endif
}

//#ifdef CONFIG_EARLY_PRINT
//#	ifndef AUXPRINTF
//#		define AUXPRINTF eprintf
//#	endif
//#else
//#	ifndef AUXPRINTF
//#		define AUXPRINTF kprintf
//#	endif
//#endif
//
//void PFetchAbort_tempHandler(xm_u32_t IFAR, xm_u32_t IFSR, xm_u32_t AIFSR, xmAddress_t faultInst, xm_u32_t mon)
//{
////	AUXPRINTF("------------PFetchAbort_tempHandler %s -----------------\n",mon?"MON":"SEC");
////	AUXPRINTF("FInstAddr:%x IDFAR:%x IFSR:%x AIFSR:%x\n", faultInst, IFAR, IFSR, AIFSR);
//	AUXPRINTF("\n[%d]",GET_LOCAL_SCHED()->cKThread->ctrl.g->id);
//	AUXPRINTF("%s", mon?"MON":"SEC");
//	AUXPRINTF(" IA:%x", faultInst);
//	AUXPRINTF(" IFAR:%x", IFAR);
//	AUXPRINTF(" IFSR:%x", IFSR);
//	AUXPRINTF(" AIFSR:%x\n", AIFSR);
//
//	while(1);
//}
//
//void DataAbort_tempHandler(xm_u32_t DFAR, xm_u32_t DFSR, xm_u32_t ADFSR, xmAddress_t faultInst, xm_u32_t mon)
//{
////	AUXPRINTF("------------DataAbort_tempHandler %s -----------------\n",mon?"MON":"SEC");
////	AUXPRINTF("FInstAddr:%x DFAR:%x DFSR:%x ADFSR:%x\n", faultInst, DFAR, DFSR, ADFSR);
//	AUXPRINTF("\n[%d]",GET_LOCAL_SCHED()->cKThread->ctrl.g->id);
//	AUXPRINTF("%s", mon?"MON":"SEC");
//	AUXPRINTF(" IA:%x", faultInst);
//	AUXPRINTF(" DFAR:%x", DFAR);
//	AUXPRINTF(" DFSR:%x", DFSR);
//	AUXPRINTF(" ADFSR:%x\n", ADFSR);
//	while(1);
///*	switch (GET_SRC_FAULT(DFSR))
//	{
//	case ALIGNMENT_FAULT:
//		AUXPRINTF("ALIGNMENT_FAULT Addr:0x%x\n",DFAR);
//		break;
//	case SECTION_TRANSLATION_FAULT:
//		AUXPRINTF("SECTION_TRANSLATION_FAULT Addr:0x%x\n",DFAR);
//		break;
//	case PAGE_TRANSLATION_FAULT:
//		AUXPRINTF("PAGE_TRANSLATION_FAULT Addr:0x%x\n",DFAR);
//		break;
//	case SECTION_ACCES_FLAG_FAULT:
//		AUXPRINTF("SECTION_ACCES_FLAG_FAULT Addr:0x%x\n",DFAR);
//		break;
//	case PAGE_ACCES_FLAG_FAULT:
//		AUXPRINTF("PAGE_ACCES_FLAG_FAULT Addr:0x%x\n",DFAR);
//		break;
//	case SECTION_DOMAIN_FAULT:
//		AUXPRINTF("SECTION_DOMAIN_FAULT Addr:0x%x\n",DFAR);
//		break;
//	case PAGE_DOMAIN_FAULT:
//		AUXPRINTF("PAGE_DOMAIN_FAULT Addr:0x%x\n",DFAR);
//		break;
//	case SECTION_PERMISION_FAULT:
//		AUXPRINTF("SECTION_PERMISION_FAULT Addr:0x%x\n",DFAR);
//		break;
//	case PAGE_PERMISION_FAULT:
//		AUXPRINTF("PAGE_PERMISION_FAULT Addr:0x%x\n",DFAR);
//		break;
//	case SYNC_EXTERNAL_ABORT:
//		AUXPRINTF("SYNC_EXTERNAL_ABORT Addr:0x%x\n",DFAR);
//		break;
//	case ASYNC_EXTERNAL_ABORT:
//		AUXPRINTF("ASYNC_EXTERNAL_ABORT Addr:0x%x\n",DFAR);
//		break;
//	case MEM_ACCESS_SYNC_PARITY_FAULT:
//		AUXPRINTF("MEM_ACCESS_SYNC_PARITY_FAULT Addr:0x%x\n",DFAR);
//		break;
//	case MEM_ACCESS_ASYNC_PARITY_FAULT:
//		AUXPRINTF("MEM_ACCESS_ASYNC_PARITY_FAULT Addr:0x%x\n",DFAR);
//		break;
//	default:
//		AUXPRINTF("Unexpected Fault %0x%x\n",GET_SRC_FAULT(DFSR));
//	}
//	AUXPRINTF("After ABORT--HALT\n");
//	XM_halt_partition(XM_PARTITION_SELF);*/
//}

#endif /*CONFIG_ARM*/

