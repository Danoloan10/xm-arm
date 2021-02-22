/*
 * $FILE: asm.h
 *
 * Processor
 *
 * $VERSION$
 *
 * Author: Miguel Masmano <mmasmano@fentiss.com>
 *         Javier Coronel <jcoronel@fentiss.com>
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE file for the license terms.
 */
/*
 * Changelog:
 * - [10/12/15:XM-ARM-2:SPR-150715-03:#73] Update SCTLR macros to i&d cache management.
 * - [15/01/16:XM-ARM-2:SPR-160108-01:#101] Add DMB Instruction.
 * - [16/02/16:XM-ARM-2:SPR-160215-01:#109] Remove dead code.
 * - [22/08/16:XM-ARM-2:SPR-160819-01:#116] Add macro to access localProcessorInfo struct in SMP.
 * - [24/08/16:XM-ARM-2:SPR-160819-02:#118] Avoid use OCM memory for SMP sev when FSBL Bootloader.
 * - [21/10/16:XM-ARM-2:SPR-161013-02:#128] Add parameter XM_PCTRLTAB_VADDR in Jump to partition function.
 * - [20/02/17:XM-ARM-2:CP-161214-01:#139] Enable BPU on ZYNQ.
 */

#ifndef _XM_ARCH_ASM_H_
#define _XM_ARCH_ASM_H_

#ifdef CONFIG_ARM

#ifndef __ASSEMBLY__
#	ifdef _XM_KERNEL_
#		include <linkage.h>
#	else /*_XM_KERNEL_*/
#		include <xm_inc/linkage.h>
#	endif /*_XM_KERNEL_*/
#	ifndef _GENERATE_OFFSETS_
#		ifdef _XM_KERNEL_
#			include <arch/asm_offsets.h>
#		else /*_XM_KERNEL_*/
#			include <xm_inc/arch/asm_offsets.h>
#		endif /*_XM_KERNEL_*/
#	endif /* _XM_KERNEL_ */
#endif /*__ASSEMBLY__*/


#define BIT(X)					((1)<<(X))

/*Processor Suspension*/
#ifndef __ASSEMBLY__
#define 	WAIT_FOR_INTERUPT()			\
			__asm__ __volatile__("wfi \n\t":::)
#define 	WAIT_FOR_EVENT()			\
			__asm__ __volatile__("wfe \n\t":::)
#define 	SEND_PROCESSOR_EVENT()		\
            __asm__ __volatile__("sev \n\t":::)
#endif /* __ASSEMBLY__ */

/*
 * Coprocessors register access
 */
//READ
#ifndef __ASSEMBLY__
#define READ_CPX(cpN,op_1,dest, cRn,cRm,op_2) \
    __asm__ __volatile__ ("mrc "#cpN",#"#op_1", %0,"#cRn","#cRm",#"#op_2"\n\t" :"=r"(dest)::)
#else
#	define READ_CPX(cpN,op_1,Rd, cRn,cRm,op_2) \
        mrc cpN,##op_1,Rd,cRn,cRm,##op_2
#endif
//WRITE
#ifndef __ASSEMBLY__
#define WRITE_CPX(cpN,op_1,dest, cRn,cRm,op_2) \
    __asm__ __volatile__ ("mcr "#cpN",#"#op_1", %0,"#cRn","#cRm",#"#op_2"\n\t" ::"r"(dest):)
#else
#	define WRITE_CPX(cpN,op_1,Rd, cRn,cRm,op_2) \
        mrc cpN,##op_1,Rd,cRn,cRm,##op_2
#endif
//SET
#ifndef __ASSEMBLY__
#define SET_CPX(cpN,op_1,dest, cRn,cRm,op_2) \
	__asm__ __volatile__ (	"mrc "#cpN",#"#op_1", r0,"#cRn","#cRm",#"#op_2"\n\t"	\
							"orr	r0,	r0,	%0\n\t"									\
							"mcr "#cpN",#"#op_1", r0,"#cRn","#cRm",#"#op_2"\n\t"	\
							::"r"(dest):"r0")
#else
#	define SET_CPX(cpN,op_1,Rd, cRn,cRm,op_2) \
        mrc cpN,##op_1,Rd,cRn,cRm,##op_2
#endif
//CLEAR
#ifndef __ASSEMBLY__
#define CLEAR_CPX(cpN,op_1,dest, cRn,cRm,op_2) \
	__asm__ __volatile__ (	"mrc "#cpN",#"#op_1", r0,"#cRn","#cRm",#"#op_2"\n\t"	\
							"bic	r0,	r0,	%0\n\t"									\
							"mcr "#cpN",#"#op_1", r0,"#cRn","#cRm",#"#op_2"\n\t"	\
							::"r"(dest):"r0")
#else
#	define CLEAR_CPX(cpN,op_1,Rd, cRn,cRm,op_2) \
        mrc cpN,##op_1,Rd,cRn,cRm,##op_2
#endif
//CLEARNSET
#ifndef __ASSEMBLY__
#define CLEARNSET_CPX(cpN,op_1,dest, cRn,cRm,op_2,mask) \
	__asm__ __volatile__ (	"mrc "#cpN",#"#op_1", r0,"#cRn","#cRm",#"#op_2"\n\t"	\
							"bic	r0,	r0,	%1\n\t"									\
							"orr	r0,	r0,	%0\n\t"									\
							"mcr "#cpN",#"#op_1", r0,"#cRn","#cRm",#"#op_2"\n\t"	\
							::"r"(dest),"r"(mask):"r0")
#else
#	define CLEARNSET_CPX(cpN,op_1,Rd, cRn,cRm,op_2) \
        mrc cpN,##op_1,Rd,cRn,cRm,##op_2
#endif


#ifndef __ASSEMBLY__
#define READ_CPSR(Rd) \
	__asm__ __volatile__ ("mrs	%0, cpsr\n\t" :"=r"(Rd)::)
#define WRITE_CPSR(Wr) \
	__asm__ __volatile__ ("msr	cpsr, %0\n\t" ::"r"(Wr):)
#define SET_CPSR(Wr) \
	__asm__ __volatile__ (	"msr	cpsr, r0\n\t"		\
							"orr	r0,	r0,	%0\n\t"		\
							"msr	cpsr, r0\n\t"		\
							::"r"(Wr):"r0")
#endif

/*
 * CP15 register access
 */
#define READ_CP15(op_1,dest, cRn,cRm,op_2)	READ_CPX( p15,op_1,dest, cRn,cRm,op_2)
#define WRITE_CP15(op_1,dest, cRn,cRm,op_2)	WRITE_CPX( p15,op_1,dest, cRn,cRm,op_2)
#define SET_CP15(op_1,dest, cRn,cRm,op_2)	SET_CPX( p15,op_1,dest, cRn,cRm,op_2)
#define CLEAR_CP15(op_1,dest, cRn,cRm,op_2)	CLEAR_CPX( p15,op_1,dest, cRn,cRm,op_2)
#define CLEARNSET_CP15(op_1,dest, cRn,cRm,op_2,mask)	CLEARNSET_CPX( p15,op_1,dest, cRn,cRm,op_2,mask)


#ifndef __ASSEMBLY__
#define ISB_INST() __asm__ __volatile__("isb\n\t"::)
#define DSB_INST() __asm__ __volatile__("dsb\n\t"::)
#define DMB_INST() __asm__ __volatile__("dmb\n\t"::)
#else
#define ISB_INST() isb
#define DSB_INST() dsb
#define DMB_INST() dmb
#endif

/*Control of the MMU tlb pointers*/
#define WR_TTBR0(_VAL)		WRITE_CP15(0,_VAL, c2, c0, 0)
#define WR_TTBR1(_VAL)		WRITE_CP15(0,_VAL, c2, c0, 1)
#define RD_TTBR0(_VAL)		READ_CP15(0,_VAL, c2, c0, 0)
#define RD_TTBR1(_VAL)		READ_CP15(0,_VAL, c2, c0, 1)
#define WR_TTBCR(_VAL)		WRITE_CP15(0,_VAL, c2, c0, 2)
#define TTBCR_128b				(0x7)
#define TTBCR_254b				(0x6)
#define TTBCR_512b				(0x5)
#define TTBCR_1kb				(0x4)
#define TTBCR_2kb				(0x3)
#define TTBCR_4kb				(0x2)
#define TTBCR_8kb				(0x1)
#define TTBCR_16kb				(0x0)
#define TTBCR_PD0				(BIT(4))
#define TTBCR_PD1				(BIT(5))

#define WR_DACR(_VAL)  WRITE_CP15(0,_VAL, c3, c0, 0)

/*Coprocessor Access Control Register*/
#define GET_CPACR(Rd) READ_CP15(0,Rd,c1,c0,2)
#define SET_CPACR(Rd) WRITE_CP15(0,Rd,c1,c0,2)
//#define _CPACR(Rd) WRITE_CP15(0,Rd,c1,c0,2)
#define CLEARNSET_CPACR(Wr,mask) CLEARNSET_CP15(0,Wr,c1,c0,2,mask)

//
///*MPU Type Register*/
//#define GET_MPUIR(Rd) READ_CP15(0,Rd,c0,c0,4)

/*System Control register - SCTLR*/
#define SCTLR_M_BIT      (1<<0)    /*MMU enable bit.*/
#define SCTLR_A_BIT      (1<<1)    /*Alignment bit.*/
#define SCTLR_C_BIT      (1<<2)    /*Enable L1 data cache*/
#define SCTLR_Z_BIT      (1<<11)   /*Enable branch predictor*/
#define SCTLR_I_BIT      (1<<12)   /*Enable L1 instruction cache*/
#define SCTLR_V_BIT      (1<<13)   /*Exception Vectors bit. 0: base address 0x00. 1:0xffff0000*/
#define SCTLR_RR_BIT     (1<<14)   /*Round-robin bit, controls replacement strategy for inst and data caches 0: Random 1: Round-robin*/
#define SCTLR_BR_BIT     (1<<17)   /*Background region bit*/
#define SCTLR_DZ_BIT     (1<<19)   /*Divide by Zero fault enable bit*/
#define SCTLR_AFE_BIT    (1<<29)   /*Access flag enable.*/
#define SCTLR_IE_BIT     (1<<31)   /*Instruction Endiannes. 0: Little-endian. 1: Big-endian*/
#ifdef CONFIG_ZYNQ
#define SCTLR_EE_BIT     (1<<25)   /*Exception Endianness*/
#define SCTLR_NMFI_BIT   (1<<27)   /*Non-maskable FIQ (NMFI) support*/
#define SCTLR_TE_BIT     (1<<30)   /*Thumb Exception enable*/
#endif

#define GET_SCTLR(Rd)		READ_CP15(0,Rd,c1,c0,0)
#define WR_SCTLR(Rd)		WRITE_CP15(0,Rd,c1,c0,0)
#define SET_SCTLR(Rd)		SET_CP15(0,Rd,c1,c0,0);ISB_INST()
#define CLEAR_SCTLR(Rd)		CLEAR_CP15(0,Rd,c1,c0,0);ISB_INST()

/*Auxiliary Control register - ACTLR*/
#define ACTLR_FW_BIT      		(1<<0)	/*Cache/TLB maintenance broadcast.*/
#define ACTLR_L2_PREFETCH_BIT   (1<<1)	/*L2 Prefetch hint.*/
#define ACTLR_L1_PREFETCH_BIT   (1<<2)	/*Dside prefetch.*/
#define ACTLR_SMP_BIT      		(1<<6)  /*SMP bit.*/
#define ACTLR_EXCL_BIT     		(1<<7)  /*Exclusive cache bit.*/
#define ACTLR_PARITY_BIT      	(1<<9)  /*Parity bit.*/

#define GET_ACTLR(Rd)		READ_CP15(0,Rd,c1,c0,1)
#define WR_ACTLR(Rd)		WRITE_CP15(0,Rd,c1,c0,1)
#define SET_ACTLR(Rd)		SET_CP15(0,Rd,c1,c0,1);ISB_INST()
#define CLEAR_ACTLR(Rd)		CLEAR_CP15(0,Rd,c1,c0,1);ISB_INST()


/*
 * TLB
 */
#ifndef __ASSEMBLY__
#define TLB_INVALIDATE_UNIFIELD()                             \
do                                                            \
{                                                             \
    xm_u32_t aux;                                             \
    /*TLBIALL - Invalidate entire unified TLB*/               \
    __asm__ __volatile__( "mov	%0, #1\n\t"                   \
                          "mcr     p15, 0, %0, c8, c7, 0\n\t" \
						 :"+r"(aux)::)                        \
while(0)
#endif /*__ASSEMBLY__*/

#ifndef __ASSEMBLY__
#define IS_TLB_INVALIDATE_UNIFIELD()                          \
do                                                            \
{                                                             \
    xm_u32_t aux;                                             \
    /*TLBIALLIS - Invalidate entire unified TLB Inner Sh*/    \
    __asm__ __volatile__( "mov	%0, #1\n\t"                   \
                          "mcr     p15, 0, %0, c8, c3, 0\n\t" \
						 :"+r"(aux)::)                        \
while(0)
#endif /*__ASSEMBLY__*/


#ifndef __ASSEMBLY__
/* Set up Domain Access Control Reg
 * ----------------------------------
 * b00 - No Access (abort)
 * b01 - Client (respect table entry)
 * b10 - RESERVED
 * b11 - Manager (ignore access permissions)
 * Setting D0 to client, all others to No Access
 */
#define CLIENT_DOMAIN_ACCES_CONTROL() \
		__asm__ __volatile__(	"mov	r0, #0x01\n\t"				\
								"mcr	p15, 0, r0, c3, c0, 0"		\
								:::"r0")
#endif /*__ASSEMBLY__*/

//#define INVALIDATE_INST_CACHE()  WRITE_CP15(0,0,c7,c5,0)
//#define INVALIDATE_DATA_CACHE()  WRITE_CP15(0,0,c15,c5,0)
//
//#define INVALIDATE_CACHE()  INVALIDATE_INST_CACHE();INVALIDATE_DATA_CACHE()
//
///****Data abort exception****/
//
///*Data Fault Status Register*/
//#define GET_DFSR(Rd) READ_CP15(0,Rd,c5,c0,0)
//
///*Data Fault Address Register*/
//#define GET_DFAR(Rd) READ_CP15(0,Rd,c6,c0,0)
//
//
///****Prefetch Abort exceptions****/
//
///*Instruction Fault Status Register*/
//#define GET_IFSR(Rd) READ_CP15(0,Rd,c5,c0,1)
//
///*Instruction Fault Address Register*/
//#define GET_IFAR(Rd) READ_CP15(0,Rd,c6,c0,2)
//
///*Fault status bits*/
//#define FS_MASK 0x40F /*[10,3:0]*/
//#define FS_AF   0x1   /*Alignment fault - MPU fault*/
//#define FS_BF   0x0   /*Background fault - MPU fault*/
//#define FS_PF   0xd   /*Permission fault - MPU fault*/
//#define FS_DE   0x2   /*Debug event*/
//#define FS_SEA  0x8   /*Synchronous external abort*/
//#define FS_ID1  0x404 /*Implementation defined - Lockdown*/
//#define FS_ID2  0x40A /*Implementation defined - Coprocessor abort*/
//#define FS_SPE  0x40A /*Memory access synchronous parity error*/
//
//#define FS_ASEA 0x406 /*Asynchronous external abort - only in DFSR*/
//#define FS_ASPE 0x408 /*Memory access asynchronous parity error - only in DFSR*/
//
//
//#ifndef __ASSEMBLY__
/* #ifdef CONFIG_MMU */

#ifndef __ASSEMBLY__

#define	NSOP_POST(cpsr, spsr, auxsp)                      \
	__asm__ __volatile__ ( "pop {r0-r12,lr} \n\t"         \
			               "mov sp, %[AUXSP] \n\t"        \
                           "smc 0 \n\t"                   \
                           "nop \n\t"                     \
                           "msr cpsr_cxsf, %[CPSR] \n\t"  \
                           "msr spsr_cxsf, %[SPSR] \n\t"  \
                           :                              \
			               :[CPSR]"r"(cpsr),              \
                            [SPSR]"r"(spsr),              \
                            [AUXSP]"r"(auxsp)             \
                           :"lr", "sp", "cc")

#define	NSOP_PRE(cpsr, spsr, auxsp)                             \
	extern xm_u32_t _xm_nonsec_stack_top[];                     \
	xm_u32_t stack_top = ((xm_u32_t)(_xm_nonsec_stack_top)) - (GET_CPU_ID() * 2 * 1024); \
	__asm__ __volatile__ ( "mrs  %[SPSR], spsr \n\t"            \
                           "mrs  %[CPSR], cpsr \n\t"            \
                           "msr  spsr_cxsf, %[CPSR] \n\t"       \
                           "orr r4, %[CPSR],%[NSMODE] \n\t"     \
                           "msr cpsr_c, r4 \n\t"                \
                           "mov %[AUXSP], sp \n\t"              \
                           "mov sp, %[NONSECSP] \n\t"           \
                           "push {r0-r12,lr} \n\t"              \
                           :[CPSR]"=&r"(cpsr),                  \
                            [SPSR]"=&r"(spsr),                  \
                            [AUXSP]"=&r"(auxsp)                 \
                           :[NSMODE]"I"(0x000000DF),            \
                            [NONSECSP]"r"(stack_top)			\
                           :"r4", "r5", "sp", "cc")
#endif /*__ASSEMBLY__*/

#ifndef __ASSEMBLY__
/* #define ASM_EXPTABLE(_a, _b) 		\
     ".section .exptable, \"a\"\n\t"	\
     ".align 4\n\t"						\
     ".long "#_a"\n\t"					\
     ".long "#_b"\n\t"					\
     ".previous\n\t" 					*/
#endif /*__ASSEMBLY__*/

#ifndef __ASSEMBLY__
/* #define EXPTABLE(_a) 				\
     __asm__ (ASM_EXPTABLE(_a)) */
#endif /*__ASSEMBLY__*/

#ifndef __ASSEMBLY__
/* #define ASM_RW(_s, _tmp) 										\
    __asm__ __volatile__ ("orn %0, %%g0, %0\n\t" : "=r" (ret)); 	\
    __asm__ __volatile__ ("1:ld"_s" [%2], %1\n\t"					\
                          "2:st"_s" %1, [%2]\n\t"					\
                          "mov %%g0, %0\n\t"						\
                          "3:\n\t"									\
                          ASM_EXPTABLE(1b, 3b)						\
                          ASM_EXPTABLE(2b, 3b)						\
                          : "=r" (ret), "=r" (_tmp) : "r" (addr)); */
#endif /*__ASSEMBLY__*/




#ifndef __ASSEMBLY__

#define GetCpuCtxt(ctxt)																					\
			do {			 																				\
			__asm__ __volatile__ (	"str r0,  [%0,#"TO_STR(_CTXTR0_OFFSET)"]\n\t":  :"r" (ctxt)	:"r0");		\
			__asm__ __volatile__ (	"str r1,  [%0,#"TO_STR(_CTXTR1_OFFSET)"]\n\t":  :"r" (ctxt)	:"r1");		\
			__asm__ __volatile__ (	"str r2,  [%0,#"TO_STR(_CTXTR2_OFFSET)"]\n\t":  :"r" (ctxt)	:"r2");		\
			__asm__ __volatile__ (	"str r3,  [%0,#"TO_STR(_CTXTR3_OFFSET)"]\n\t":  :"r" (ctxt)	:"r3");		\
			__asm__ __volatile__ (	"str r4,  [%0,#"TO_STR(_CTXTR4_OFFSET)"]\n\t":  :"r" (ctxt)	:"r4");		\
			__asm__ __volatile__ (	"str r5,  [%0,#"TO_STR(_CTXTR5_OFFSET)"]\n\t":  :"r" (ctxt)	:"r5");		\
			__asm__ __volatile__ (	"str r6,  [%0,#"TO_STR(_CTXTR6_OFFSET)"]\n\t":  :"r" (ctxt)	:"r6");		\
			__asm__ __volatile__ (	"str r7,  [%0,#"TO_STR(_CTXTR7_OFFSET)"]\n\t":  :"r" (ctxt)	:"r7");		\
			__asm__ __volatile__ (	"str r8,  [%0,#"TO_STR(_CTXTR8_OFFSET)"]\n\t":  :"r" (ctxt)	:"r8");		\
			__asm__ __volatile__ (	"str r9,  [%0,#"TO_STR(_CTXTR9_OFFSET)"]\n\t":  :"r" (ctxt)	:"r9");		\
			__asm__ __volatile__ (	"str r10, [%0,#"TO_STR(_CTXTR10_OFFSET)"]\n\t": :"r" (ctxt)	:"r10");	\
			__asm__ __volatile__ (	"str r11, [%0,#"TO_STR(_CTXTR11_OFFSET)"]\n\t": :"r" (ctxt)	:"r11");	\
			__asm__ __volatile__ (	"str r12, [%0,#"TO_STR(_CTXTR12_OFFSET)"]\n\t": :"r" (ctxt)	:"r12");	\
			__asm__ __volatile__ (	"str pc,  [%0,#"TO_STR(_CTXTPC_OFFSET)"]\n\t":  :"r" (ctxt) :	  );	\
			__asm__ __volatile__ (	"str lr,  [%0,#"TO_STR(_CTXTLR_OFFSET)"]\n\t":  :"r" (ctxt) :	  );	\
			__asm__ __volatile__ (	"str sp,  [%0,#"TO_STR(_CTXTSP_OFFSET)"]\n\t":  :"r" (ctxt) :	  );	\
			__asm__ __volatile__ (	"mrs r0, cpsr\n\t"														\
                               	    "str r0,  [%0,#"TO_STR(_CTXTCPSR_OFFSET)"]\n\t"::"r" (ctxt) :"r0" );	\
			__asm__ __volatile__ (	"mrs r0, spsr\n\t"														\
                               	    "str r0,  [%0,#"TO_STR(_CTXTSPSR_OFFSET)"]\n\t"::"r" (ctxt) :"r0" );	\
			} while(0)
#endif /*__ASSEMBLY__*/

/* Both DFSR and IFSR possible values */
#define FSR_TLB_SYNC_EXTERNAL_ABORT_1LEVEL		(0xC)
#define FSR_TLB_SYNC_EXTERNAL_ABORT_2LEVEL		(0xE)
#define FSR_TLB_SYNC_PARITY_ERROR_1LEVEL		(0x40C)
#define FSR_TLB_SYNC_PARITY_ERROR_2LEVEL		(0x40E)
#define FSR_SYNC_EXTERNAL_ABORT					(0x8)
#define FSR_MEMORY_ACCESS_SYNC_PARITY_ERROR		(0x409)
/* DFSR possible values */
#define DFSR_ASYNC_EXTERNAL_ABORT				(0x406)
#define DFSR_MEMORY_ACCESS_ASYNC_PARITY_ERROR	(0x408)

#ifndef __ASSEMBLY__
#define GET_DFAR(DFAR)   \
            READ_CP15(0,DFAR,c6,c0,0)
            /* __asm__ __volatile__("mrc p15, 0, %0, c6, c0, 0\n\t":"=r"(DFAR)::)   DFAR */
#define GET_DFSR(DFSR)   \
            READ_CP15(0,DFSR,c5,c0,0)
            /*__asm__ __volatile__("mrc p15, 0, %0, c5, c0, 0\n\t":"=r"(DFSR)::)   DFSR*/
#define GET_IFAR(IFAR)   \
            READ_CP15(0,IFAR,c6,c0,2)
            /* __asm__ __volatile__("mrc p15, 0, %0, c6, c0, 2\n\t":"=r"(IFAR)::)   IFAR */
#define GET_IFSR(IFSR)   \
            READ_CP15(0,IFSR,c5,c0,1)
            /*__asm__ __volatile__("mrc p15, 0, %0, c5, c0, 1\n\t":"=r"(IFSR)::)   IFSR*/
#define GET_ADFSR(ADFSR)   \
        READ_CP15(0,ADFSR,c5,c1,0)
            /*__asm__ __volatile__("mrc p15, 0, %0, c5, c1, 0\n\t":"=r"(ADFSR)::)   ADFSR*/
#endif /*__ASSEMBLY__*/

#ifndef __ASSEMBLY__
/*TODO ARMPorting: Comment for build, not implemented yet */
//#define AsmRWCheck(X,Y,Z)	(X==Y)
#ifdef NOT_IMPLEMENTED_YET
static inline xm_s32_t AsmRWCheck(xmAddress_t param, xmSize_t size, xm_u32_t align) {
     xmAddress_t addr;
     xm_s32_t ret;
     xm_u8_t tmp1;
     xm_u16_t tmp2;
     xm_u32_t tmp4;
     xm_u64_t tmp8;
     for (addr=param; addr<param+size; addr=(addr&PAGE_MASK)+PAGE_SIZE) {
         switch (align) {
         case 1:
             ASM_RW("ub", tmp1);
             break;
         case 2:
             ASM_RW("uh", tmp2);
             break;
         case 4:
             ASM_RW("", tmp4);
             break;
         case 8:
             ASM_RW("d", tmp8);
             break;
         }
         if (ret)
             return -1;
     }
     return 0;
 }
#endif
#endif /*__ASSEMBLY__*/

#ifndef __ASSEMBLY__
/* #define ASM_RD(_s, _tmp) \
     __asm__ __volatile__ ("orn %0, %%g0, %0\n\t" : "=r" (ret)); \
     __asm__ __volatile__ ("1:ld"_s" [%2], %1\n\t" \
                           "mov %%g0, %0\n\t" \
                           "2:\n\t" \
                           ASM_EXPTABLE(1b, 2b) \
                           : "=r"(ret), "=r" (_tmp) : "r" (addr)) */
#endif /*__ASSEMBLY__*/

#ifndef __ASSEMBLY__
/*TODO ARMPorting: Comment for build, not implemented yet */
#define AsmROnlyCheck(X,Y,Z) (X==Y)
/* static inline xm_s32_t AsmROnlyCheck(xmAddress_t param, xmSize_t size, xm_u32_t align) {
     xmAddress_t addr;
     xm_s32_t ret;
     xm_u8_t tmp1;
     xm_u16_t tmp2;
     xm_u32_t tmp4;
     xm_u64_t tmp8;
     for (addr=param; addr<param+size; addr=(addr&PAGE_MASK)+PAGE_SIZE) {
         switch (align) {
         case 1:
             ASM_RD("ub", tmp1);
             break;
         case 2:
             ASM_RD("uh", tmp2);
             break;
         case 4:
             ASM_RD("", tmp4);
             break;
         case 8:
             ASM_RD("d", tmp8);
             break;
         }

         if (ret)
             return -1;
     }

     return 0;
 }*/
#endif /*__ASSEMBLY__*/

#ifndef __ASSEMBLY__
/* #define LoadPtdL1(_ptdL1S, _id) do { \
     contextTab[(_id)+1]=((_ptdL1S)>>4)|_PG_ARCH_PTD_PRESENT; \
     SetMmuCtxt((_id)+1); \
 } while(0)
 */
#endif /*__ASSEMBLY__*/

#ifndef __ASSEMBLY__
#define SET_MODE_STACK(newMode, oldMode, stack) \
    __asm__ __volatile__ ("cps #"#newMode"\n\t" \
                          "mov sp, %0\n\t" \
                          "cps #"#oldMode"\n\t" \
                          : : "r" (stack))
#endif /*__ASSEMBLY__*/


#ifndef __ASSEMBLY__
#define CONTEXT_SWITCH(newThread, currentThread) 				\
    __asm__ __volatile__ (".global .Tbegin_cs, .Tend_cs\n\t"	\
                          ".Tbegin_cs:"							\
                          "push {r0-r12, lr}\n\t"				\
                          "mrs r1, cpsr\n\t"					\
                          "cps #0x16\n\t"	/*MON_MODE*/		\
                          "push {r1}\n\t"						\
                          "cps #0x11\n\t"	/*FIQ_MODE*/		\
                          "mrs r1, spsr\n\t"					\
                          "mov r2, sp\n\t"						\
                          "mov r3, lr\n\t"						\
                          "cps #0x16\n\t"	/*MON_MODE*/		\
                          "push {r1-r3,r8-r12}\n\t"				\
                          "cps #0x12\n\t"	/*IRQ_MODE*/		\
                          "mrs r1, spsr\n\t"					\
                          "mov r2, sp\n\t"						\
                          "mov r3, lr\n\t"						\
                          "cps #0x16\n\t"	/*MON_MODE*/		\
                          "push {r1-r3}\n\t"					\
                          "cps #0x13\n\t"	/*SVC_MODE*/		\
                          "mrs r1, spsr\n\t"					\
                          "mov r2, sp\n\t"						\
                          "mov r3, lr\n\t"						\
                          "cps #0x16\n\t"	/*MON_MODE*/		\
                          "push {r1-r3}\n\t"					\
                          "cps #0x17\n\t"	/*ABT_MODE*/		\
                          "mrs r1, spsr\n\t"					\
                          "mov r2, sp\n\t"						\
                          "mov r3, lr\n\t"						\
                          "cps #0x16\n\t"	/*MON_MODE*/		\
                          "push {r1-r3}\n\t"					\
                          "cps #0x1b\n\t"	/*UND_MODE*/		\
                          "mrs r1, spsr\n\t"					\
                          "mov r2, sp\n\t"						\
                          "mov r3, lr\n\t"						\
                          "cps #0x16\n\t"	/*MON_MODE*/		\
                          "push {r1-r3}\n\t"					\
                          "cps #0x1f\n\t"	/*SYS_MODE*/		\
                          "mrs r1, spsr\n\t"					\
                          "mov r2, sp\n\t"						\
                          "mov r3, lr\n\t"						\
                          "cps #0x16\n\t"	/*MON_MODE*/		\
                          "push {r1-r3}\n\t"					\
                          "movw r1, #:lower16:1f\n\t"			\
                          "movt r1, #:upper16:1f\n\t"			\
                          "push {r1}\n\t"						\
                          "ldr r1, [%1]\n\t"					\
                          "str sp, [r1, # "TO_STR(_KSTACK_OFFSET)"]\n\t" \
                          "ldr sp, [%0, # "TO_STR(_KSTACK_OFFSET)"]\n\t" \
                          "str %0, [%1]\n\t"					\
                          "pop {pc}\n\t"						\
                          "1:\n\t"								\
                          "pop {r1-r3}\n\t"						\
                          "cps #0x1f\n\t"	/*SYS_MODE*/		\
                          "msr spsr_cxsf, r1\n\t"					\
                          "mov sp, r2\n\t"						\
                          "mov lr, r3\n\t"						\
                          "cps #0x16\n\t"	/*MON_MODE*/		\
                          "pop {r1-r3}\n\t"						\
                          "cps #0x1b\n\t"	/*UND_MODE*/		\
                          "msr spsr_cxsf, r1\n\t"					\
                          "mov sp, r2\n\t"						\
                          "mov lr, r3\n\t"						\
                          "cps #0x16\n\t"	/*MON_MODE*/		\
                          "pop {r1-r3}\n\t"						\
                          "cps #0x17\n\t"	/*ABT_MODE*/		\
                          "msr spsr_cxsf, r1\n\t"					\
                          "mov sp, r2\n\t"						\
                          "mov lr, r3\n\t"						\
                          "cps #0x16\n\t"	/*MON_MODE*/		\
                          "pop {r1-r3}\n\t"						\
                          "cps #0x13\n\t"	/*SVC_MODE*/		\
                          "msr spsr_cxsf, r1\n\t"					\
                          "mov sp, r2\n\t"						\
                          "mov lr, r3\n\t"						\
                          "cps #0x16\n\t"	/*MON_MODE*/		\
                          "pop {r1-r3}\n\t"						\
                          "cps #0x12\n\t"	/*IRQ_MODE*/		\
                          "msr spsr_cxsf, r1\n\t"					\
                          "mov sp, r2\n\t"						\
                          "mov lr, r3\n\t"						\
                          "cps #0x16\n\t"	/*MON_MODE*/		\
                          "pop {r1-r3,r8-r12}\n\t"				\
                          "cps #0x11\n\t"	/*FIQ_MODE*/		\
                          "msr spsr_cxsf, r1\n\t"					\
                          "mov sp, r2\n\t"						\
                          "mov lr, r3\n\t"						\
                          "cps #0x16\n\t"	/*MON_MODE*/		\
                          "pop {r1}\n\t"						\
                          "msr cpsr_cxsf, r1\n\t"					\
                          "pop {r0-r12, lr}\n\t"				\
                          ".Tend_cs:"							\
							::"r" (newThread), "r" (currentThread):"r1","r2","r3")
#endif /*__ASSEMBLY__*/

#ifndef __ASSEMBLY__
#	define DoNop() __asm__ __volatile__ ("nop\n\t" ::)
#endif /*__ASSEMBLY__*/

#ifndef __ASSEMBLY__
/*#define GetCpuCtxt(ctxt) do { \
} while(0)*/
#endif /*__ASSEMBLY__*/


#ifdef CONFIG_VFP_EXTENSION
#   ifndef __ASSEMBLY__
#define FPEXC_EN_SET()													\
__asm__ __volatile__(	"fmrx r0, FPEXC\n\t"							\
						"orr  r0, r0, #"TO_STR(EN_VFP_BIT)"\n\t"		\
						"fmxr FPEXC,r0\n\t"								\
						:::"r0")
#define FPEXC_EN_CLR()													\
__asm__ __volatile__(	"fmrx r0, FPEXC\n\t"							\
						"bic  r0, r0, #"TO_STR(EN_VFP_BIT)"\n\t"		\
						"fmxr FPEXC,r0\n\t"								\
						:::"r0")
#   endif /* __ASSEMBLY__ */
#endif /*CONFIG_VFP_EXTENSION*/

#ifdef __ASSEMBLY__
.macro  getLocalProcessorInfo
        movw r0, #:lower16:localProcessorInfo   /*GET_LOCAL_SCHED()*/
        movt r0, #:upper16:localProcessorInfo
#ifdef CONFIG_SMP_SUPPORT
        push {r1}
        mrc     p15,#0,r1,c0,c0,#5
        ands    r1, r1, #0x0000000f
        addne   r0, #_LOCALPROCESSOR_T_SIZEOF
        pop {r1}
#endif
.endm
#endif /*__ASSEMBLY__*/

#ifdef __ASSEMBLY__
#define STANDART_DOTRAP_EXCEPTION(EXCP_N)               \
        sub lr, lr, #4                                  \
        push {r0-r12, lr}                               \
        mov r4, sp                                      \
        mrs r3, cpsr                                    \
        mrs r2, spsr                                    \
        mov r1, sp                                      \
        mov r0, #EXCP_N                                 \
        push {r0-r4, lr}                                \
        getLocalProcessorInfo                           \
        ldr r0, [r0, #_CKTHREAD_OFFSET]                 \
        str sp, [r0, #_CTRL_OFFSET+_IRQCPUCTXT_OFFSET]  \
        mov r0, sp                                      \
        bl DoTrap                                       \
        mov r0, sp                                      \
        bl _CheckPendingException                       \
        pop {r0-r4, lr}                                 \
        msr cpsr_cxsf, r3                               \
        msr spsr_cxsf, r2                               \
        mov sp, r4                                      \
        ldmia sp!, {r0-r12, pc}^
#endif /*__ASSEMBLY__*/

#endif	/*CONFIG_ARM*/

#endif /*_XM_ARCH_ASM_H_*/

