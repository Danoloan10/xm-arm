/*
 * $FILE: processor.h
 *
 * Processor
 *
 * $VERSION$
 *
 * Author: Miguel Masmano <mmasmano@fentiss.com>
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE file for the license terms.
 */
/*
 * Changelog:
 * - [10/12/15:XM-ARM-2:SPR-150715-03:#73] Update cache management.
 * - [15/01/16:XM-ARM-2:SPR-160108-01:#101] Update cache management function definitions.
 */

#ifndef _XM_ARCH_PROCESSOR_H_
#define _XM_ARCH_PROCESSOR_H_

#ifdef _XM_KERNEL_
#include <arch/io.h>
#include <arch/asm.h>
#ifdef CONFIG_ZYNQ
#	include <drivers/zynqboard/zynqboard.h>
#endif /*CONFIG_ZYNQ*/
#else
#include <xm_inc/arch/io.h>
#include <xm_inc/arch/asm.h>
#ifdef CONFIG_ZYNQ
#	include <xm_inc/drivers/zynqboard/zynqboard.h>
#endif /*CONFIG_ZYNQ*/
#endif

#ifdef _XM_KERNEL_

#define PSR_MODE			(0x1f)
#	define PSR_MODE_USR		(0x10)
#	define PSR_MODE_FIQ		(0x11)
#	define PSR_MODE_IRQ		(0x12)
#	define PSR_MODE_SVC		(0x13)
#	define PSR_MODE_MON		(0x16)
#	define PSR_MODE_ABORT	(0x17)
#	define PSR_MODE_UNDF	(0x1b)
#	define PSR_MODE_SYS		(0x1f)


//#define PSR_T (1<<5)
#define PSR_F (1<<6)
#define PSR_I BIT(7)
#define PSR_A (1<<8) // imprecise data aborts
#define PSR_E (1<<9) // Endianess
//#define PSR_IT (0x3f<<10)
//#define PSR_GE (0xf<<16)
//#define PSR_J (1<<24)
//#define PSR_CC (0x1f<<27)


#ifdef CONFIG_VFP_EXTENSION
#define CP_FP_MASK		0x00F00000
#define CP_FP_EN_PRIV	0x00500000   /*Enable FP for privileged access only*/
#define CP_FP_EN_FULL	0x00F00000   /*Enable FP for full access*/
#endif /*CONFIG_VFP_EXTENSION*/


//#ifndef __ASSEMBLY__
//#include <arch/asm.h>

//#define DCACHE_FROZEN  0x0
//#define ICACHE_FROZEN  0x0
//#define CACHE_MASK 0x3

/*
 * System-Level Control registers
 */
#define CA9_SLCR_BASE_ADDRESS		0xF8000000

/*
 * ACCES SCR Register
 */
#define WR_SCR(scr)			WRITE_CP15(0,scr, c1, c1, 0)
#define RD_SCR(scr)			READ_CP15(0,scr, c1, c1, 0)
#define ST_SCR(mask)		SET_CP15(0,mask, c1, c1, 0)
#define CR_SCR(mask)		CLEAR_CP15(0,mask, c1, c1, 0)

/*Determines External Abort behavior for Secure and Non-secure worlds:
0 = Branch to abort mode on an External Abort exception, reset value
1 = Branch to Secure Monitor mode on an External Abort exception.*/
#define CA9_SCR_EA_MASK		(BIT(3))
#define CA9_SCR_EA_AS_EXCEPTION()	CR_SCR(CA9_SCR_EA_MASK)
#define CA9_SCR_EA_AS_MONITOR()		ST_SCR(CA9_SCR_EA_MASK)
/*Determines FIQ behavior for Secure and Non-secure worlds:
0 = Branch to FIQ mode on an FIQ exception, reset value
1 = Branch to Secure Monitor mode on an FIQ exception.*/
#define CA9_SCR_FIQ_MASK	(BIT(2))
#define CA9_SCR_FIQ_AS_EXCEPTION()	CR_SCR(CA9_SCR_FIQ_MASK)
#define CA9_SCR_FIQ_AS_MONITOR()	ST_SCR(CA9_SCR_FIQ_MASK)
/*Determines IRQ behavior for Secure and Non-secure worlds:
0 = Branch to IRQ mode on an IRQ exception, reset value
1 = Branch to Secure Monitor mode on an IRQ exception.*/
#define CA9_SCR_IRQ_MASK	(BIT(1))
#define CA9_SCR_IRQ_AS_EXCEPTION()	CR_SCR(CA9_SCR_IRQ_MASK)
#define CA9_SCR_IRQ_AS_MONITOR()	ST_SCR(CA9_SCR_IRQ_MASK)
/*Defines the world for the processor:
0 = Secure, reset value
1 = Non-secure.*/
#define CA9_SCR_NS_MASK		(BIT(0))
#define CA9_SCR_SECURE()			CR_SCR(CA9_SCR_NS_MASK)
#define CA9_SCR_NONSECURE()			ST_SCR(CA9_SCR_NS_MASK)


#define CA9_SET_VBAR(X)		WRITE_CP15(0,X,c12,c0,0)
#define CA9_GET_VBAR(X)		READ_CP15(0,X,c12,c0,0)
#define CA9_SET_MVBAR(X)	WRITE_CP15(0,X,c12,c0,1)
#define CA9_GET_MVBAR(X)	READ_CP15(0,X,c12,c0,1)

/*Multiprocesor Affinity Register*/
#define CA9_GET_PROCESSOR_ID_MASK		(0x00000003)
#define CA9_GET_CLUSTER_ID_MASK			(0x00000f00)
#define CA9_GET_MPIDR(mpidr)				READ_CP15(0,mpidr,c0,c0,5)

/*SNOOP Control Unit*/
#define CA9_SCU_CONFIG_REGISTER		(0x00000004)
#define CA9_SCU_CONTROL_REGISTER	(0x00000000)
#define CA9_SCU_INVALIDATE_REGISTER	(0x0000000C)
#define CA9_SCU_SNSAC_REGISTER		(0x00000054)
#define CA9_SCU_SPECLINEFILLS_MASK	(BIT(3))
#define CA9_SCU_PARITY_MASK			(BIT(2))
#define CA9_SCU_ENABLE_MASK			(BIT(0))
#define CA9_SCU_CPUS_NUMBER_MASK	(0x00000003)
#define CA9_SCU_INVALIDATE_MASK		(0xFFFF)
#define CA9_READ_SCU_CONFIG()		ReadReg(PERIOHBASE, CA9_SCU_CONFIG_REGISTER)
#define CA9_SCU_INVALIDATE()		WriteReg(PERIOHBASE, CA9_SCU_INVALIDATE_REGISTER, CA9_SCU_INVALIDATE_MASK)
#define CA9_SCU_ENABLE()			SetReg(PERIOHBASE, CA9_SCU_CONTROL_REGISTER, CA9_SCU_ENABLE_MASK)
#define CA9_SCU_DISABLE()			ClearReg(PERIOHBASE, CA9_SCU_CONTROL_REGISTER, CA9_SCU_ENABLE_MASK)
#define CA9_SCU_PARITY_ENABLE()		SetReg(PERIOHBASE, CA9_SCU_CONTROL_REGISTER, CA9_SCU_PARITY_MASK)
#define CA9_SCU_PARITY_DISABLE()	ClearReg(PERIOHBASE, CA9_SCU_CONTROL_REGISTER, CA9_SCU_PARITY_MASK)
#define CA9_SCU_SPECLINEFILLS_ENABLE()	SetReg(PERIOHBASE, CA9_SCU_CONTROL_REGISTER, CA9_SCU_SPECLINEFILLS_MASK)
#define CA9_SCU_SPECLINEFILLS_DISABLE()	ClearReg(PERIOHBASE, CA9_SCU_CONTROL_REGISTER, CA9_SCU_SPECLINEFILLS_MASK)
#define CA9_WRITE_SCU_SNSAC(value)	WriteReg(PERIOHBASE, CA9_SCU_SNSAC_REGISTER, value)

#ifndef __ASSEMBLY__
extern void EnableScu(void);
#endif /*__ASSEMBLY__*/


/***************************
 * VIRTUAL MEM TRANSLATION *
 ***************************/
#define CA9_VMT_WR_ATS12NSOPR(Addr)		WRITE_CP15(0, Addr, c7, c8, 4) /* 32-bit WO Stages 1 and 2 Non-secure only PL1 read */
#define CA9_VMT_WR_ATS12NSOPW(Addr)		WRITE_CP15(0, Addr, c7, c8, 5) /* 32-bit WO Stages 1 and 2 Non-secure only PL1 write */
#define CA9_VMT_WR_ATS12NSOUR(Addr)		WRITE_CP15(0, Addr, c7, c8, 6) /* 32-bit WO Stages 1 and 2 Non-secure only unprivileged read */
#define CA9_VMT_WR_ATS12NSOUW(Addr)		WRITE_CP15(0, Addr, c7, c8, 7) /* 32-bit WO Stages 1 and 2 Non-secure only unprivileged write */
#define CA9_VMT_WR_ATS1CPR(Addr)		WRITE_CP15(0, Addr, c7, c8, 0) /* 32-bit WO Stage 1 Current state PL1 read */
#define CA9_VMT_WR_ATS1CPW(Addr)		WRITE_CP15(0, Addr, c7, c8, 1) /* 32-bit WO Stage 1 Current state PL1 write */
#define CA9_VMT_WR_ATS1CUR(Addr)        WRITE_CP15(0, Addr, c7, c8, 2) /* 32-bit WO Stage 1 Current state unprivileged read */
#define CA9_VMT_WR_ATS1CUW(Addr)        WRITE_CP15(0, Addr, c7, c8, 3) /* 32-bit WO Stage 1 Current state unprivileged write */
#define CA9_VMT_WR_ATS1HR(Addr)         WRITE_CP15(4, Addr, c7, c8, 0) /* 32-bit WO Stage 1 Hyp mode read */
#define CA9_VMT_WR_ATS1HW(Addr)         WRITE_CP15(4, Addr, c7, c8, 1) /* 32-bit WO Stage 1 Hyp mode write */
#define CA9_VMT_RD_PAR(Res)             READ_CP15(0, Res, c7, c4, 0)   /* 32-bit RW Physical Address Register */

/*Non-Secure Access control Register*/
#define CA9_RD_NSACR(Rd)                     READ_CP15(0,Rd,c1,c1,2)
#define CA9_WR_NSACR(Wr)                     WRITE_CP15(0,Wr,c1,c1,2)
#define CA9_SET_NSACR(Msk)                   SET_CP15(0,Msk,c1,c1,2)
#define CA9_CLR_NSACR(Msk)                   CLEAR_CP15(0,Msk,c1,c1,2)
#define CA9_NSACR_CP10_MASK                     (BIT(10))
#define CA9_NSACR_CP11_MASK                     (BIT(11))
#define CA9_NSACR_CP10CP11_NS()                 CA9_SET_NSACR(CA9_NSACR_CP10_MASK|CA9_NSACR_CP11_MASK)
#define CA9_NSACR_CP10CP11_SONLY()              CA9_CLR_NSACR(CA9_NSACR_CP10_MASK|CA9_NSACR_CP11_MASK)

#define DoNop() __asm__ __volatile__ ("nop\n\t" ::)

#define CpuPowerDown() __asm__ __volatile__ ("wfe\n\t" ::)

//#if defined(CONFIG_PLAN_EXTSYNC)&&defined(CONFIG_MDHBX)
//#define MANUAL_DBG_EXT_SYNCH
//#endif
//
//#ifdef CONFIG_ENABLE_CACHE
//

#ifndef __ASSEMBLY__
extern void SetCacheState(xm_u32_t cache);
extern void NSSetCacheState(xm_u32_t cache);
#endif /*__ASSEMBLY__*/

#ifndef __ASSEMBLY__
extern void FlushICache(void);
extern void FlushDCache(void);
extern void FlushCache(void);
#endif /*__ASSEMBLY__*/

//#endif
//
//#define LoadPartitionPageTable(k)
//#define LoadXmPageTable()
//
/*#define HwEnableFpuHpv() do { \
    xm_u32_t __tmp1; \
    GET_CPACR(__tmp1); \
    __tmp1&=~CP_FP_MASK; \
    __tmp1|=CP_FP_EN_PRIV; \
    SET_CPACR(__tmp1); \
} while(0)*/

//#endif

#endif /*_XM_KERNEL_*/
#ifndef __ASSEMBLY__
/*#define HwSaveSp(sp) \
    __asm__ __volatile__ ("mov %%sp, %0\n\t" : "=r" (sp))
*/
/*#define HwSaveFp(fp) \
    __asm__ __volatile__ ("mov %%fp, %0\n\t" : "=r" (fp))
*/
/*#define HwDisableFpu() do { \
    xm_u32_t __tmp1, __tmp2; \
    __asm__ __volatile__ ("rd %%psr, %0\n\t" \
	   		  "set "TO_STR(PSR_EF_BIT)", %1\n\t" \
			  "andn %0, %1, %0\n\t"	\
			  "wr %0, %%psr\n\t" : "=r" (__tmp1), "=r" (__tmp2):); \
} while(0)
*/
//#define DoNop() __asm__ __volatile__ ("nop\n\t" ::)

//#define CpuPowerDown() DoNop()

//static inline void FlushDCache(void) {
//    xm_u32_t cCR;
//#ifdef CONFIG_LEON2
//    cCR=LoadIoReg(LEON_CCR_BASE);
//#else
//    __asm__ __volatile__("lda [%%g0] %1, %0\n\t":"=r"(cCR): "i"(LEON_CCR_BASE):"memory");
//#endif
//    cCR|=1<<22;
//#ifdef CONFIG_LEON2
//    StoreIoReg(LEON_CCR_BASE, cCR);
//#else
//    __asm__ __volatile__("sta %0, [%%g0] %1\n\t"::"r"(cCR), "i"(LEON_CCR_BASE):"memory");
//#endif
//}
//
//static inline void FlushICache(void) {
//    xm_u32_t cCR;
//#ifdef CONFIG_LEON2
//    cCR=LoadIoReg(LEON_CCR_BASE);
//#else
//     __asm__ __volatile__("lda [%%g0] %1, %0\n\t":"=r"(cCR): "i"(LEON_CCR_BASE):"memory");
//#endif
//     cCR|=1<<21;
//#ifdef CONFIG_LEON2
//    StoreIoReg(LEON_CCR_BASE, cCR);
//#else
//    __asm__ __volatile__("sta %0, [%%g0] %1\n\t"::"r"(cCR), "i"(LEON_CCR_BASE):"memory");
//#endif
//}
//
//static inline void FlushCache(void) {
//    xm_u32_t cCR;
//#ifdef CONFIG_LEON2
//    cCR=LoadIoReg(LEON_CCR_BASE);
//#else
//    __asm__ __volatile__("lda [%%g0] %1, %0\n\t":"=r"(cCR): "i"(LEON_CCR_BASE):"memory");
//#endif
//    cCR|=0x3<<21;
//#ifdef CONFIG_LEON2
//    StoreIoReg(LEON_CCR_BASE, cCR);
//#else
//    __asm__ __volatile__("sta %0, [%%g0] %1\n\t"::"r"(cCR), "i"(LEON_CCR_BASE):"memory");
//#endif
//}
//
//
//
//static inline void DisableCache(xm_u32_t cache) {
//    xm_u32_t cCR;
//#ifdef CONFIG_LEON2
//    cCR=LoadIoReg(LEON_CCR_BASE);
//#else
//    __asm__ __volatile__("lda [%%g0] %1, %0\n\t":"=r"(cCR): "i"(LEON_CCR_BASE):"memory");
//#endif
//    cCR&=~(cache);
//#ifdef CONFIG_LEON2
//    StoreIoReg(LEON_CCR_BASE, cCR);
//#else
//    __asm__ __volatile__("sta %0, [%%g0] %1\n\t"::"r"(cCR), "i"(LEON_CCR_BASE):"memory");
//#endif
//}
//
//static inline void EnableCache(xm_u32_t cache) {
//    xm_u32_t cCR;
//#ifdef CONFIG_LEON2
//    cCR=LoadIoReg(LEON_CCR_BASE);
//#else
//    __asm__ __volatile__("lda [%%g0] %1, %0\n\t":"=r"(cCR): "i"(LEON_CCR_BASE):"memory");
//#endif
//    cCR|=cache;
//#ifdef CONFIG_LEON2
//    StoreIoReg(LEON_CCR_BASE, cCR);
//#else
//    __asm__ __volatile__("sta %0, [%%g0] %1\n\t"::"r"(cCR), "i"(LEON_CCR_BASE):"memory");
//#endif
//}
//
//

//static inline void SetMmuCtxt(xm_u8_t ctxt)
//{
//	xm_u8_t a = ctxt;
//#ifdef CONFIG_MMU
//ToDo
//    __asm__ __volatile__ ("sta %0, [%1] %2\n\t" :: "r"(ctxt), "r"(LEON_MMU_CTXT_REG), "i"(LEON_MMU_ASI): "memory");
//#endif
//
//}

//#define LoadXmPageTable() SetMmuCtxt(0)
/*#define LoadPartitionPageTable(k) do { \
  SetMmuCtxt(GetPartitionCfg(k)->id+1); \
} while(0)
*/
//static inline xm_u8_t GetMmuCtxt(void) {
//    xm_u8_t ctxt;
//    __asm__ __volatile__ ("lda [%1] %2, %0\n\t" :"=r" (ctxt): "r"(LEON_MMU_CTXT_REG), "i"(LEON_MMU_ASI): "memory");
//    return ctxt;
//}
//
//static inline void SetMmuCtxtPtr(xm_u32_t *ctxtPtr) {
//    xmAddress_t cPtr=(xmAddress_t)ctxtPtr>>4;
//    __asm__ __volatile__ ("sta %0, [%1] %2\n\t" :: "r"(cPtr), "r"(LEON_MMU_CPR_REG), "i"(LEON_MMU_ASI): "memory");
//}
//


static inline xm_u32_t GetMmuFaultStatusReg(void) {
#ifdef CONFIG_MMU
    xm_u32_t reg;
    GET_DFSR(reg);
    return reg;
#else
    return 0;
#endif
}
//
static inline xm_u32_t GetMmuFaultAddressReg(void) {
#ifdef CONFIG_MMU
    xm_u32_t reg;
    GET_DFAR(reg);
    return reg;
#else
    return 0;
#endif
}
static inline xm_u32_t GetMmuIFaultStatusReg(void) {
#ifdef CONFIG_MMU
    xm_u32_t reg;
    GET_IFSR(reg);
    return reg;
#else
    return 0;
#endif
}
static inline xm_u32_t GetMmuIFaultAddressReg(void) {
#ifdef CONFIG_MMU
    xm_u32_t reg;
    GET_IFAR(reg);
    return reg;
#else
    return 0;
#endif
}
static inline xm_u32_t GetMmuAuxiliaryFaultAddressReg(void) {
#ifdef CONFIG_MMU
    xm_u32_t reg;
    GET_ADFSR(reg);
    return reg;
#else
    return 0;
#endif
}
//

/*TODO ARMPorting: To be tested*/

#define InvalidateEntireInstructionTLB() \
					WRITE_CP15(0,1,c8,c5,0)

#define InvalidateEntireDataTLB() \
					WRITE_CP15(0,1,c8,c6,0)

#define InvalidateEntireUnifiedTLB() \
					WRITE_CP15(0,1,c8,c7,0)

#define FlushTlb()  \
			InvalidateEntireUnifiedTLB();		\
			InvalidateEntireDataTLB();			\
			InvalidateEntireInstructionTLB();	\
			DSB_INST();							\
			ISB_INST()

#define FlushTlbEntry(X) InvalidateEntireUnifiedTLB()

#define FlushTlbGlobal() \
				FlushTlb()

#endif /*__ASSEMBLY__*/

#endif /*_XM_ARCH_PROCESSOR_H_*/
