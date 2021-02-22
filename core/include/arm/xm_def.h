/*
 * $FILE: xm_def.h
 *
 * XM's ARMv7 CortexA9 hardware configuration
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
 * - [12/01/16:XM-ARM-2:SPR-150615-01:#59]: Remove dead code.
 */

#ifndef _XM_ARCH_XMDEF_H_
#define _XM_ARCH_XMDEF_H_

#ifdef CONFIG_VFP_EXTENSION

#   ifdef CONFIG_ZYNQ
#define HX_REG_16B_NR      0
#define SX_REG_32B_NR      32
#define DX_REG_64B_NR      32
#define QX_REG_128B_NR     16
#define FP_REGS_NR         3   /*FPSCR,FPSID,FPEXC*/
#   endif /* CONFIG_ZYNQ */

#	ifndef CONFIG_ZYNQ
#define HX_REG_16B_NR      0
#define SX_REG_32B_NR      32
#define DX_REG_64B_NR      16
#define QX_REG_128B_NR     0
#define FP_REGS_NR         3
#   endif /* CONFIG_ZYNQ */

#define XM_FPU_REGS_SIZE   (  (HX_REG_16B_NR)/2  +\
                              (SX_REG_32B_NR)    +\
                              (DX_REG_64B_NR)*2  +\
                              (QX_REG_128B_NR)*4 +\
                              (FP_REGS_NR) )

#   ifndef __ASSEMBLY__
#   	if 0 /*fpuCtxt_t*/
typedef struct _fpuCtxt
{
	xm_u32_t FPEXC;
	xm_u32_t FOSCR;
	xm_u32_t FPSID; /*FPSCR,FPSID,FPEXC*/
    xm_u16_t s[HX_REG_16B_NR];
    xm_u32_t s[SX_REG_32B_NR];
	xm_u64_t d[DX_REG_64B_NR];
	xm_u128_t q[QX_REG_128B_NR];
}fpuCtxt_t;
//#define XM_FPU_REG_NR    (sizeof(fpuCtxt_t))
#       else /*fpuCtxt_t*/
typedef xm_u32_t fpuCtxt_t[XM_FPU_REGS_SIZE+10];
#       endif /*fpuCtxt_t*/
#   endif /* __ASSEMBLY__ */

#endif /* CONFIG_VFP_EXTENSION */

#define __ARCH_MEMCPY

#endif /* _XM_ARCH_XMDEF_H_ */
