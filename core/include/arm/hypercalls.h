/*
 * $FILE: hypercalls.h
 *
 * Processor-related hypercalls definition
 *
 * Author: Miguel Masmano <mmasmano@fentiss.com>
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license terms.
 */

#ifndef _XM_ARCH_HYPERCALLS_H_
#define _XM_ARCH_HYPERCALLS_H_

#ifndef __ASSEMBLY__
unsigned int get_monitor_call_number(unsigned int op);
#endif /*__ASSEMBLY__*/

//@ \void{<track id="hypercall-numbers">}
#define __HALT_PARTITION_NR 0
#define __SUSPEND_PARTITION_NR 1
#define __RESUME_PARTITION_NR 2
#define __RESET_PARTITION_NR 3
#define __SHUTDOWN_PARTITION_NR 4
#define __HALT_SYSTEM_NR 5
#define __RESET_SYSTEM_NR 6
#define __IDLE_SELF_NR 7

#define __GET_TIME_NR 8
#define __SET_TIMER_NR 9
#define __READ_OBJECT_NR 10
#define __WRITE_OBJECT_NR 11
#define __SEEK_OBJECT_NR 12
#define __CTRL_OBJECT_NR 13

#define __CLEAR_IRQ_MASK_NR 14
#define __SET_IRQ_MASK_NR 15
#define __FORCE_IRQS_NR 16
#define __CLEAR_IRQS_NR 17
#define __ROUTE_IRQ_NR 18

#define __SET_CACHE_STATE_NR 19

#define __SWITCH_SCHED_PLAN_NR 20
#define __GET_GID_BY_NAME_NR 21

#define arm_inport_nr 23
#define arm_outport_nr 24
//#define reserved_0 29
//#define reserved_1 30

#define __RESET_VCPU_NR 31
#define __HALT_VCPU_NR 32
#define __GET_VCPUID_NR 33
#define __RAISE_IPVI_NR 34

#define arm_hm_raise_event_nr 35
#define __RESUME_IMM_PARTITION_NR 36
#define __SWITCH_IMM_SCHED_PLAN_NR 37
//@ \void{</track id="hypercall-numbers">}

//#define arm_set_spsr_nr 29
//#define arm_get_spsr_nr 30
//#define arm_set_cpsr_nr 31
//#define arm_get_cpsr_nr 32
//#define arm_iret_nr		35
//#define arm_inport_nr	35
//#define arm_outport_nr	36


#define NR_HYPERCALLS 38

//@ \void{<track id="asm-hypercall-numbers">}
//@ \void{</track id="asm-hypercall-numbers">}
//#define NR_ASM_HYPERCALLS 7

#ifndef __ASSEMBLY__

/*#define ASM_HYPERCALL_TAB(_ahc) \
    __asm__ (".section .ahypercallstab, \"a\"\n\t" \
	     ".align 4\n\t" \
	     ".long "#_ahc"\n\t" \
	     ".previous\n\t")*/

#ifdef CONFIG_SPW_RTC
struct ioOutPortMsg {
   xm_u32_t * msg;
   xm_s32_t size;
   xmAddress_t checkingAddr;
   xm_u32_t validationValue;
   xm_u32_t validationMask;
};
#endif

#endif

#endif
