/*
 * $FILE: xmconf.h
 *
 * Config parameters for both, XM and partitions
 *
 * $VERSION$
 *
 * Authors: Miguel Masmano <mmasmano@fentiss.com>
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license terms.
 */
/*
 * Changelog:
 * - [09/02/16:XM-ARM-2:SPR-151125-01:#97] Define Hypervisor Features tags XM_HYP_FEAT... .
 */

#ifndef _XM_ARCH_XMCONF_H_
#define _XM_ARCH_XMCONF_H_
#ifndef __ASSEMBLY__
enum zynq_dev_e{
    ZYNQ_DEV_CAN0=0,
    ZYNQ_DEV_CAN1=1,
    ZYNQ_DEV_QSPI=2,
    ZYNQ_DEV_GPIO=3,
    ZYNQ_DEV_I2C0=4,
    ZYNQ_DEV_I2C1=5,
#ifndef CONFIG_DEV_UART_0
    ZYNQ_DEV_UA0=6,
#endif /* CONFIG_DEV_UART_0 */
#ifndef CONFIG_DEV_UART_1
    ZYNQ_DEV_UA1=7,
#endif /* CONFIG_DEV_UART_1 */
    ZYNQ_DEV_SPI0=8,
    ZYNQ_DEV_SPI1=9,
    ZYNQ_DEV_SMC=10,
    ZYNQ_DEV_GEM0=11,
    ZYNQ_DEV_GEM1=12,
    ZYNQ_DEV_USB0=13,
    ZYNQ_DEV_USB1=14,
    ZYNQ_DEV_TTC1=15,
    ZYNQ_N_DEV=16
};
#endif /*__ASSEMBLY__*/

/*Not change the order: hm-event*/
/* <track id="test-hw-events-arm"> */
/* </track id="test-hw-events-arm"> */
//@% <track id="hm-ev-xm-triggered-arch">
#define XM_HM_EV_ARM_UNDEF_INSTR (XM_HM_MAX_GENERIC_EVENTS+0)
#define XM_HM_EV_ARM_PREFETCH_ABORT (XM_HM_MAX_GENERIC_EVENTS+1)
#define XM_HM_EV_ARM_DATA_ABORT (XM_HM_MAX_GENERIC_EVENTS+2)
#define XM_HM_EV_ARM_DATA_ALIGNMENT_FAULT (XM_HM_MAX_GENERIC_EVENTS+3)
#define XM_HM_EV_ARM_DATA_BACKGROUND_FAULT (XM_HM_MAX_GENERIC_EVENTS+4)
#define XM_HM_EV_ARM_DATA_PERMISSION_FAULT (XM_HM_MAX_GENERIC_EVENTS+5)
#define XM_HM_EV_ARM_INSTR_ALIGNMENT_FAULT (XM_HM_MAX_GENERIC_EVENTS+6)
#define XM_HM_EV_ARM_INSTR_BACKGROUND_FAULT (XM_HM_MAX_GENERIC_EVENTS+7)
#define XM_HM_EV_ARM_INSTR_PERMISSION_FAULT (XM_HM_MAX_GENERIC_EVENTS+8)
//@ </track id="hm-ev-xm-triggered-arch">

#define XM_HM_MAX_ARCH_EVENTS	9

#define XM_HM_MAX_EVENTS (XM_HM_MAX_GENERIC_EVENTS+XM_HM_MAX_ARCH_EVENTS)

#ifndef __ASSEMBLY__
//
//#ifdef CONFIG_DEV_FPGA_ITARFREE
// #ifndef _XM_KERNEL_
// #include <xm_inc/drivers/fpga_xmconf.h>
// #else
// #include <drivers/fpga_xmconf.h>
// #endif
//#else
// #define XM_HM_MAX_EXTERNAL_EVENTS   0
//#endif
//
//#define XM_HM_MAX_ARCH_EVENTS 19
//#define XM_HM_MAX_EVENTS (XM_HM_MAX_GENERIC_EVENTS + XM_HM_MAX_ARCH_EVENTS + XM_HM_MAX_APP_EVENTS)
//
//
//#define XM_MEM_REG_STRAM 0
//#define XM_MEM_REG_SDRAM 1
//
//#define XM_TRAP_IRQ_15 15 // UNUSED // 15
//#define XM_TRAP_PCI_IRQ 14 // PCI // 14
//#define XM_TRAP_IRQ_13 13 // UNUSED // 13
//#define XM_TRAP_IRQ_12 12 // UNUSED // 12
//#define XM_TRAP_DSU_IRQ 11 // DSU // 11
//#define XM_TRAP_IRQ_10 10 // UNUSED // 10
//#define XM_TRAP_TIMER2_IRQ 9 // TIMER2 // 9
//#define XM_TRAP_TIMER1_IRQ 8 // TIMER1 // 8
//#define XM_TRAP_IO3_IRQ 7 // IO3 // 7
//#define XM_TRAP_IO2_IRQ 6 // IO2 // 6
//#define XM_TRAP_IO1_IRQ 5 // IO1 // 5
//#define XM_TRAP_IO0_IRQ 4 // IO0 // 4
//#define XM_TRAP_UART1_IRQ 3 // UART1 // 3
//#define XM_TRAP_UART2_IRQ 2 // UART2 // 2
//#define XM_TRAP_AMBA_IRQ 1 // AMBA // 1
//#define XM_TRAP_IRQ_0 0
//
//#define XM_CPU_FEATURE_WA1 0x1
#ifndef __ASSEMBLY__
enum hypFeatures_e{
	XM_HYP_FEAT_NONE=0,
#ifdef CONFIG_ZYNQ
	XM_HYP_FEAT_FPGA_PART_ACCESS=(1<<0),
#endif
};
#endif /*__ASSEMBLY__*/
//
//// [base, end[
struct xmcIoPort {
    xm_u32_t type;
#define XM_IOPORT_RANGE 0
#define XM_RESTRICTED_IOPORT 1
    union xmcIoPortUnion {
	struct xmcIoPortRange {
	    xmIoAddress_t base;
	    xm_s32_t noPorts;
	} range;
	struct xmcRestrictdIoPort {
	    xmIoAddress_t address;
	    xm_u32_t mask;
#define XM_DEFAULT_RESTRICTED_IOPORT_MASK (~0)
	} restricted;
    } portUnion;
};
#endif /*__ASSEMBLY__*/
#endif /*_XM_ARCH_XMCONF_H_*/
