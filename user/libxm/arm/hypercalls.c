/*
 * $FILE: hypercalls.c
 *
 * XM system calls definitions
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
 * - [10/12/15:XM-ARM-2:SPR-150715-03:#73] Enable HCALL XM_set_cache_state.
 * - [21/01/16:XM-ARM-2:SPR-160121-05:#107] HCall API is not up to date: XM_raise_ipvi is commented.
 * - [03/03/16:XM-ARM-2:SPR-150716-03:#80] Add comments to clarify function XM_clear_irqmask functionality
 */

#include <xm.h>

__stdcall xmId_t XM_get_vcpuid(void) {
    xm_s32_t _r;
    _XM_HCALL0(get_vcpuid_nr, _r);
    return _r;
}
//xm_hcall0r(get_vcpuid);
xm_hcall1r(halt_vcpu, xm_u32_t, vcpuId);
xm_hcall2r(reset_vcpu, xm_u32_t, vcpuId, xmAddress_t, entry);

xm_hcall0r(halt_system);
xm_hcall1r(reset_system, xm_u32_t, resetMode);
xm_hcall1r(halt_partition, xm_u32_t, partitionId);
xm_hcall1r(suspend_partition, xm_u32_t, partitionId);
xm_hcall1r(resume_partition, xm_u32_t, partitionId);
xm_hcall1r(resume_imm_partition, xm_u32_t, partitionId);
xm_hcall4r(reset_partition, xm_u32_t, partitionId, xmAddress_t, ePoint, xm_u32_t, resetMode, xm_u32_t, status);
xm_hcall1r(shutdown_partition, xm_u32_t, partitionId);
xm_hcall0r(idle_self);
xm_hcall4r(read_object, xmObjDesc_t, objDesc, void *, buffer, xm_u32_t, size, xm_u32_t *, flags);
xm_hcall4r(write_object, xmObjDesc_t, objDesc, void *, buffer, xm_u32_t, size, xm_u32_t *, flags);
xm_hcall3r(ctrl_object, xmObjDesc_t, objDesc, xm_u32_t, cmd, void *, arg);
//xm_hcall3r(seek_object, xmObjDesc_t, traceStream, xm_u32_t, offset, xm_u32_t, whence);
xm_hcall2r(get_time, xm_u32_t, clock_id, xmTime_t *, time);

xm_hcall2r(arm_hm_raise_event, xm_u32_t, hmEvent, void *, dataEvent)


//xm_hcall2r(clear_irqmask, xm_u32_t *, hwIrqsMask, xm_u32_t, extIrqsMask);
ASMLINK xm_s32_t XM_clear_irqmask(xm_u32_t * hwIrqsMask, xm_u32_t extIrqsMask)
{
    xm_s32_t _r;
	if (extIrqsMask)
	{
		xm_u32_t * reg;
		reg = ((xm_u32_t *)(0xf8f01000)); *reg =  ((xm_u32_t)(0)); //ICDDCR  Disable Distributor
		reg = ((xm_u32_t *)(0xf8f01080)); *reg |= ((xm_u32_t)(1)); //ICDISR0 Enable Non-Secure Access
		reg = ((xm_u32_t *)(0xf8f01400)); *reg =  ((xm_u32_t)((((*reg)&(~0xff))|(0x0)))); //ICDIPR0 Set Priority
//		reg = ((xm_u32_t *)(0xf8f01400)); *reg |= ((xm_u32_t)((0x0)));
		reg = ((xm_u32_t *)(0xf8f01100)); *reg |= ((xm_u32_t)(1)); //ICDISER0 Enable IRQ
		reg = ((xm_u32_t *)(0xf8f01000)); *reg =  ((xm_u32_t)(1)); //ICDDCR Enable Distributor

/*		HwEnableUserIrqFromUsr(0);*//*XM_EXTIRQ_SGI_NUM*/
	}
	_XM_HCALL2(hwIrqsMask, extIrqsMask, clear_irqmask_nr, _r);
	return _r;
}
xm_hcall2r(set_irqmask, xm_u32_t *, hwIrqsMask, xm_u32_t, extIrqsPend);
xm_hcall2r(set_irqpend, xm_u32_t *, hwIrqMask, xm_u32_t, extIrqMask);
xm_hcall2r(clear_irqpend, xm_u32_t *, hwIrqMask, xm_u32_t, extIrqMask);
//xm_hcall3r(route_irq, xm_u32_t, type, xm_u32_t, irq, xm_u16_t, vector);
xm_hcall1r(raise_ipvi, xm_u8_t, no_ipvi);
//
xm_hcall2r(set_cache_state, xm_u32_t, cache, xm_u32_t, action);
xm_hcall2r(switch_sched_plan, xm_u32_t, newPlanId, xm_u32_t *, currentPlanId);
xm_hcall2r(switch_imm_sched_plan, xm_u32_t, newPlanId, xm_u32_t *, currentPlanId);
//
xm_hcall2r(get_gid_by_name, xm_u8_t *, name, xm_u32_t, entity);
//
////ARM ARCH
//xm_hcall1r(arm_set_spsr, xm_u32_t, mode);
//xm_hcall1r(arm_get_spsr, xm_u32_t *, mode);
//xm_hcall1r(arm_set_cpsr, xm_u32_t, mode);
//xm_hcall1r(arm_get_cpsr, xm_u32_t *, mode);
//xm_hcall0r(arm_iret);
//

static xmTime_t aux_abstime=0;
static xmTime_t aux_interval=0;
ASMLINK xm_s32_t XM_set_timer(xm_u32_t clock_id, xmTime_t abstime, xmTime_t interval) {
    xm_s32_t _r;
    aux_abstime = abstime;
    aux_interval = interval;
    _XM_HCALL3(clock_id, &aux_interval, &aux_abstime, set_timer_nr, _r);
    return _r;
}

xm_hcall2r(arm_inport, xm_u32_t, port, xm_u32_t *, value);
xm_hcall2r(arm_outport, xm_u32_t, port, xm_u32_t, value);
