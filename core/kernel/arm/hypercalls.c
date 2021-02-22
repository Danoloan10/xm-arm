/*
 * $FILE: hypercalls.c
 *
 * XM's hypercalls
 *
 * $VERSION$
 *
 * Author: Miguel Masmano 		<mmasmano@fentiss.com>
 * Author: Manuel Muñoz 		<mmunoz@ai2.upv.es>
 * Author: Javier O. Coronel 	<jcoronel@fentiss.com>
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license terms.
 */
/*
 * Changelog:
 * - [25/11/2015:SPR-151125-01:#96] Fix the minor issue. Comment / delete the kprintf.
 * - [12/01/16:XM-ARM-2:SPR-150615-01:#59]: Remove dead code.
 */
#ifdef CONFIG_ARM

#include <assert.h>
#include <gaccess.h>
#include <hypercalls.h>
#include <sched.h>
#include <local.h>
#include <arch/io.h>
#include <drivers/zynqboard/zynq_tzpc.h>

static inline xm_u32_t IoPortLogSearch(xm_u32_t port)
{
	localSched_t *sched=GET_LOCAL_SCHED();
	struct xmcPartition *cfg=GetPartitionCfg(sched->cKThread);
	struct xmcIoPort *ioPort;
	xm_u32_t a0, a1;
	xm_s32_t i;

	for (i=0; i<cfg->noIoPorts;i++){
		ioPort=&xmcIoPortTab[i+cfg->ioPortsOffset];
		if (ioPort->type==XM_RESTRICTED_IOPORT) {
			a0=ioPort->restricted.address;
			a1=a0+sizeof(xm_u32_t);
			if (port>=a0&&port<a1)
				return ioPort->restricted.mask;
		} else {
			a0=ioPort->range.base;
			a1=a0+sizeof(xm_u32_t)*ioPort->range.noPorts;
			if (port>=a0&&port<a1)
				return ~0UL;
		}
	}

	return 0;
}



__hypercall xm_s32_t ArmIoOutportSys(xm_u32_t port, xm_u32_t value)
{
	xm_u32_t mask=0, oldValue;

	ASSERT(!HwIsSti());
	if (port&0x3) return XM_INVALID_PARAM;
	if (!(mask=IoPortLogSearch(port)))
		return XM_PERM_ERROR;

	if (mask==~0UL)
	{  /* If no mask then write directly into the port */
		if((port>=CA9_SLCR_BASE_ADDRESS)&&(port<=(CA9_SLCR_BASE_ADDRESS+0x00000B74)))
		{	CA9_TZ_UNLOCK_SLCR(); }
			StoreIoReg(port, value);
		if((port>=CA9_SLCR_BASE_ADDRESS)&&(port<=(CA9_SLCR_BASE_ADDRESS+0x00000B74)))
		{	CA9_TZ_LOCK_SLCR();  }
		
	} else {           /* Otherwise read first the value of the port */
		/* <track id="restricted-port-mask"> */
		oldValue=LoadIoReg(port);
		if((port>=CA9_SLCR_BASE_ADDRESS)&&(port<=(CA9_SLCR_BASE_ADDRESS+0x00000B74)))
		{	CA9_TZ_UNLOCK_SLCR(); }
		StoreIoReg(port, ((oldValue&~(mask))|(value&mask)));
		if((port>=CA9_SLCR_BASE_ADDRESS)&&(port<=(CA9_SLCR_BASE_ADDRESS+0x00000B74)))
		{	CA9_TZ_LOCK_SLCR();  }
		/* </track id="restricted-port-mask"> */
	}
	return XM_OK;
}

__hypercall xm_u32_t ArmIoInportSys(xm_u32_t port, xm_u32_t *__gParam value)
{
	xm_u32_t mask=0;

	ASSERT(!HwIsSti());

	if (CheckGParam(value, 4, sizeof(xm_u32_t), 1)<0)
		return XM_INVALID_PARAM;

	if (!value) return XM_INVALID_PARAM;
	if (port&0x3) return XM_INVALID_PARAM;
	if (!(mask=IoPortLogSearch(port)))
		return XM_PERM_ERROR;

	*value=(LoadIoReg(port)&mask);

	return XM_OK;
}

__hypercall xm_s32_t PartitionDoTrap(xm_u32_t hmEvent, struct hmCpuCtxt * dataEvent)
{
    localSched_t *sched=GET_LOCAL_SCHED();
    struct xmcPartition *cfg=GetPartitionCfg(sched->cKThread);
    xmHmLog_t hmLog;

    memset(&hmLog, 0, sizeof(xmHmLog_t));
    hmLog.opCode|= ((hmEvent<<HMLOG_OPCODE_EVENT_BIT)&HMLOG_OPCODE_EVENT_MASK);

    if (sched->cKThread->ctrl.g)
        hmLog.opCode |= ((cfg->rId<<HMLOG_OPCODE_PARTID_BIT)&HMLOG_OPCODE_PARTID_MASK);

    memcpy(&(hmLog.cpuCtxt), dataEvent, sizeof(struct hmCpuCtxt));
    hmLog.opCode |= HMLOG_OPCODE_VALID_CPUCTXT_MASK;

#ifdef CONFIG_VERBOSE_TRAP
//    DumpState(ctxt);
#endif


    xm_s32_t action=HmRaiseEvent(&hmLog);
/*    if (IsSvIrqCtxt(ctxt)&&((hmLog.opCode&HMLOG_OPCODE_SYS_MASK)!=HMLOG_OPCODE_SYS_MASK))
        PartitionPanic(ctxt,"Partition in unrecoverable state\n");
    if (!IsSvIrqCtxt(ctxt)) {
    if (action)
    {   SetTrapPending(ctxt); }
} else
    {SystemPanic(ctxt,"Unexpected/unhandled trap - TRAP: 0x%x ERROR CODE: 0x%x\n", sched->cKThread->ctrl.g->partCtrlTab->trap2Vector[ctxt->irqNr], GET_ECODE(ctxt));}
*/
    return action;
}

// Hypercall table
HYPERCALL_TAB(HaltPartitionSys); // 0
HYPERCALL_TAB(SuspendPartitionSys); // 1
HYPERCALL_TAB(ResumePartitionSys); // 2
HYPERCALL_TAB(ResetPartitionSys); // 3
HYPERCALL_TAB(ShutdownPartitionSys); // 4
HYPERCALL_TAB(HaltSystemSys); // 5
HYPERCALL_TAB(ResetSystemSys); // 6
HYPERCALL_TAB(IdleSelfSys); // 7

HYPERCALL_TAB(GetTimeSys); // 8
HYPERCALL_TAB(SetTimerSys); // 9
HYPERCALL_TAB(ReadObjectSys); // 10
HYPERCALL_TAB(WriteObjectSys); // 11
HYPERCALL_TAB(SeekObjectSys); // 12
HYPERCALL_TAB(CtrlObjectSys); // 13

HYPERCALL_TAB(ClearIrqMaskSys); // 14
HYPERCALL_TAB(SetIrqMaskSys); // 15
HYPERCALL_TAB(ForceIrqsSys); // 16
HYPERCALL_TAB(ClearIrqsSys); // 17
HYPERCALL_TAB(RouteIrqSys); // 18

HYPERCALLR_TAB(SetCacheStateSys); // 19

HYPERCALLR_TAB(SwitchSchedPlanSys); // 20
HYPERCALLR_TAB(GetGidByName); // 21

HYPERCALL_TAB(0); //22

HYPERCALLR_TAB(ArmIoInportSys); // 23
HYPERCALLR_TAB(ArmIoOutportSys); // 24
HYPERCALL_TAB(0); // 25

HYPERCALL_TAB(0); // 26
HYPERCALLR_TAB(0); // 27
HYPERCALLR_TAB(0); // 28

HYPERCALLR_TAB(0); // 29
HYPERCALLR_TAB(0); // 30

HYPERCALL_TAB(ResetVCpuSys); // 31
HYPERCALL_TAB(HaltVCpuSys); // 32
HYPERCALL_TAB(GetVCpuIdSys); // 33
HYPERCALL_TAB(RaiseIpviSys); // 34

HYPERCALL_TAB(PartitionDoTrap); // 35
HYPERCALL_TAB(ResumePartitionImmSys); // 36
HYPERCALL_TAB(SwitchSchedPlanImmSys); // 37

#ifdef CONFIG_AUDIT_EVENTS
xm_u8_t auditAsmHCall[NR_ASM_HYPERCALLS];

void AuditAsmHCall(void) {
#if 0
	xmWord_t payload[2];
	xm_s32_t e;
	for (e=0; e<NR_ASM_HYPERCALLS; e++)
		if (auditAsmHCall[e]) {
			payload[0]=e;
			payload[1]=auditAsmHCall[e];
			RaiseAuditEvent(TRACE_HCALLS_MODULE, AUDIT_ASMHCALL, 2, payload);
			auditAsmHCall[e]=0;
		}
#endif
}

#endif

#endif /*CONFIG_ARM*/
