/*
 * $FILE: hypercalls.h
 *
 * Hypercalls definition
 *
 * Author: Miguel Masmano <mmasmano@fentiss.com>
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license terms.
 */
/*
 * Changelog:
 * - [10/12/15:XM-ARM-2:SPR-150715-01:#71] Fix the misspelled label XM_DEACTIVATE_CACHE.
 */

#ifndef _XM_HYPERCALLS_H_
#define _XM_HYPERCALLS_H_

#ifdef _XM_KERNEL_
#	include <arch/hypercalls.h>
#else /*_XM_KERNEL_*/
#	include <xm_inc/arch/hypercalls.h>
#endif /*_XM_KERNEL_*/

/* <track id="abi-api-versions"> */
#define XM_ABI_VERSION 3
#define XM_ABI_SUBVERSION 1
#define XM_ABI_REVISION 0

#define XM_API_VERSION 3
#define XM_API_SUBVERSION 1
#define XM_API_REVISION 2
/* </track id="abi-api-versions"> */
// Generic hypercalls 

#define HYPERCALL_NOT_IMPLEMENTED (~0)
#define halt_partition_nr __HALT_PARTITION_NR
#define suspend_partition_nr __SUSPEND_PARTITION_NR
#define resume_partition_nr __RESUME_PARTITION_NR
#define reset_partition_nr __RESET_PARTITION_NR
    #define XM_RESET_MODE 0x1
    #define XM_COLD_RESET 0x0
    #define XM_WARM_RESET 0x1
    #define XM_RESET_INVALID_HDR -1
#define shutdown_partition_nr __SHUTDOWN_PARTITION_NR
#define halt_system_nr __HALT_SYSTEM_NR
#define reset_system_nr __RESET_SYSTEM_NR
#define idle_self_nr __IDLE_SELF_NR
#define get_time_nr __GET_TIME_NR
//@ \void{<track id="DOC-CLOCKS-AVAILABLE">}
    #define XM_HW_CLOCK (0x0)
    #define XM_EXEC_CLOCK (0x1)
//@ \void{</track id="DOC-CLOCKS-AVAILABLE">}
//    #define XM_WATCHDOG_TIMER (0x2)

#define set_timer_nr __SET_TIMER_NR
#define read_object_nr __READ_OBJECT_NR
#define write_object_nr __WRITE_OBJECT_NR
#define seek_object_nr __SEEK_OBJECT_NR
    #define XM_OBJ_SEEK_CUR 0x0
    #define XM_OBJ_SEEK_SET 0x1
    #define XM_OBJ_SEEK_END 0x2
#define ctrl_object_nr __CTRL_OBJECT_NR

#define clear_irqmask_nr __CLEAR_IRQ_MASK_NR
#define set_irqmask_nr __SET_IRQ_MASK_NR
#define set_irqpend_nr __FORCE_IRQS_NR
#define clear_irqpend_nr __CLEAR_IRQS_NR
#define route_irq_nr __ROUTE_IRQ_NR
//IRQS
//@% <track id="xm-interrupt-types">
    #define XM_TRAP_TYPE 0x0
    #define XM_HWIRQ_TYPE 0x1
    #define XM_EXTIRQ_TYPE 0x2
//@ </track id="xm-interrupt-types">

#define set_cache_state_nr __SET_CACHE_STATE_NR
/* <track id="cache-types-codes-list"> */
  #define XM_DCACHE 0x1
  #define XM_ICACHE 0x2
/* </track id="cache-types-codes-list"> */

  #define XM_FREEZE_DCACHE 0x4
  #define XM_FREEZE_ICACHE 0x8

/* <track id="cache-actions-codes-list"> */
  #define XM_ACTIVATE_CACHE 0x1
  #define XM_DEACTIVATE_CACHE 0x2
  #define XM_FLUSH_CACHE 0x3
/* </track id="cache-actions-codes-list"> */


#define switch_sched_plan_nr __SWITCH_SCHED_PLAN_NR
#define switch_imm_sched_plan_nr __SWITCH_IMM_SCHED_PLAN_NR
#define get_gid_by_name_nr __GET_GID_BY_NAME_NR
  #define XM_PARTITION_NAME 0x0
  #define XM_PLAN_NAME 0x1

#define reset_vcpu_nr __RESET_VCPU_NR
#define halt_vcpu_nr __HALT_VCPU_NR
#define get_vcpuid_nr __GET_VCPUID_NR
#define raise_ipvi_nr __RAISE_IPVI_NR

#define resume_imm_partition_nr __RESUME_IMM_PARTITION_NR


// Returning values

/* <track id="error-codes-list"> */
#define XM_OK                 (0)
#define XM_NO_ACTION         (-1)
#define XM_UNKNOWN_HYPERCALL (-2)
#define XM_INVALID_PARAM     (-3)
#define XM_PERM_ERROR        (-4)
#define XM_INVALID_CONFIG    (-5)
#define XM_INVALID_MODE      (-6)
#define XM_NOT_AVAILABLE     (-7)
#define XM_OP_NOT_ALLOWED    (-8)
/* </track id="error-codes-list"> */

#ifndef __ASSEMBLY__

#ifdef CONFIG_ARM
#define HYPERCALLR_TAB(_hc) \
    __asm__ (".section .hypercallstab, \"a\"\n\t" \
	     ".balign 4\n\t" \
	     ".long "#_hc"\n\t" \
	     ".previous\n\t")
#endif /*CONFIG_ARM*/

#ifdef CONFIG_ARM
#define HYPERCALL_TAB(_hc) \
    __asm__ (".section .hypercallstab, \"a\"\n\t" \
	     ".balign 4\n\t" \
	     ".long "#_hc"\n\t" \
	     ".previous\n\t")
#endif /*CONFIG_ARM*/


#endif /*__ASSEMBLY__*/

#endif /*_XM_HYPERCALLS_H_*/
