/*
 * $FILE: spinlock.h
 *
 * Spin locks related stuffs
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
#ifndef _XM_ARM_SPINLOCK_H_
#define _XM_ARM_SPINLOCK_H_

#ifndef _XM_KERNEL_
#error Kernel file, do not include.
#endif

#include <arm/processor.h>

typedef struct {
    volatile xm_u8_t lock;
} archSpinLock_t;

#define __ARCH_SPINLOCK_UNLOCKED {0}

static inline void __ArchSpinLock(archSpinLock_t *lock)
{
/*	prefetchw(&lock->slock);*/
#if 0
#define TICKET_SHIFT	0
	xm_u32_t tmp, newval;
	xm_s32_t lockval;

	__asm__ __volatile__(									\
			"1:						\n\t"					\
			"ldrex	%0, [%3]		\n\t"					\
			"add	%1, %0, %4		\n\t"					\
			"strex	%2, %1, [%3]	\n\t"					\
			"arm	%2, #0			\n\t"					\
			"bne	1b				\n\t"					\
			"dmb					\n\t"					\
			:"=&r" (lockval), "=&r" (newval), "=&r" (tmp)	\
			:"r" (&lock->lock), "I" (1 << TICKET_SHIFT)	\
			:"cc");
#else
#define TICKET_SHIFT	0
	xm_s32_t lockval;
	__asm__ __volatile__(																\
            "1: 						\n\t"											\
            "ldrex 		%0, [%1]		\n\t"		/*load the lock value*/				\
            "cmp 		%0, #0			\n\t"		/*is the lock free?	*/				\
            "strexeq 	%0, %2, [%1]	\n\t"		/*try and claim the lock*/			\
            "cmpeq		%0, #0			\n\t"		/*did this succeed?*/				\
            "bne 		1b				\n\t"		/*no – try again*/					\
			"dmb						\n\t"											\
			:"=&r" (lockval)															\
			:"r" (&lock->lock), "r" (1 << TICKET_SHIFT)									\
			:"cc");
#endif
}

static inline void __ArchSpinUnlock(archSpinLock_t *lock) {
	__asm__ __volatile__(																\
            "dmb 						\n\t"											\
            "str 		%1, [%0]		\n\t"		/*clear the lock*/					\
            "dsb 						\n\t"											\
			:																			\
			:"r" (&lock->lock), "r" (0)													\
			:);
	return;
/*TODO ARMPorting: Not implemented yet*/
//Todo #ifdef CONFIG_MULTICORE_SUPPORT
//    __asm__ __volatile__("stb %%g0, [%0]" : : "r" (&lock->lock) : "memory");
//#endif
//static inline void arch_spin_unlock(arch_spinlock_t *lock)
//{
//	smp_mb();
//	lock->tickets.owner++;
//	dsb_sev();
//}
}


static inline xm_s32_t __ArchSpinTryLock(archSpinLock_t *lock) {
/*TODO ARMPorting: Not implemented yet*/
#ifdef NOT_IMPLEMENTED_YET
    xm_u32_t result=0;
#ifdef CONFIG_MULTICORE_SUPPORT
    __asm__ __volatile__("ldstuba [%1] 1, %0" /* ASI_LEON23_DCACHE_MISS */ \
                         : "=r" (result) \
                         : "r" (lock) \
                         : "memory");
#endif
    return (result==0);
#endif /*NOT_IMPLEMENTED_YET*/
#define TICKET_SHIFT	0
    xm_s32_t lockval;
	__asm__ __volatile__(																\
            "1: 						\n\t"											\
            "ldrex 		%0, [%1]		\n\t"		/*load the lock value*/				\
            "cmp 		%0, #0			\n\t"		/*is the lock free?	*/				\
            "strexeq 	%0, %2, [%1]	\n\t"		/*try and claim the lock*/			\
			"dmb						\n\t"											\
			:"=&r" (lockval)															\
			:"r" (&lock->lock), "r" (1 << TICKET_SHIFT)									\
			:"cc");
	return lockval;
/*static inline int arch_spin_trylock(archSpinLock_t *lock)
{
	unsigned long contended, res;
	xm_u32_t slock;

	prefetchw(&lock->lock);
	do {
		__asm__ __volatile__(
		"	ldrex	%0, [%3]\n"
		"	mov	%2, #0\n"
		"	subs	%1, %0, %0, ror #16\n"
		"	addeq	%0, %0, %4\n"
		"	strexeq	%2, %0, [%3]"
		: "=&r" (slock), "=&r" (contended), "=&r" (res)
		: "r" (&lock->lock), "I" (1 << TICKET_SHIFT)
		: "cc");
	} while (res);

	if (!contended) {
		smp_mb();
		return 1;
	} else {
		return 0;
	}
}*/
}


#define __ArchSpinIsLocked(x) (*(volatile xm_s8_t *)(&(x)->lock)>0)

/*TODO ARMPorting: Not implemented yet*/
//static inline xm_u32_t GetIpl(void) {
//    xm_u32_t retVal;
//    __asm__ __volatile__("rd %%psr, %0" : "=r" (retVal));
//    return retVal;
//}

//#ifndef PSR_PIL_MASK
//#define PSR_PIL_MASK 0x00000f00 // Proc interrupt level
//#endif

static inline void HwCli(void) {
    __asm__ __volatile__ ("cpsid if\n\t");
}

static inline void HwSti(void) {
    __asm__ __volatile__ ("cpsie if\n\t");
}

static inline xm_u32_t __SaveFlagsCli(void) {
    xm_u32_t retval=0;
    __asm__ __volatile__ ("mrs %0, cpsr\n\t" : "=r" (retval):);
    HwCli();
    return retval;
}

#define HwSaveFlagsCli(flags) ((flags)=__SaveFlagsCli())

static inline void HwRestoreFlags(xm_u32_t flags) {
    __asm__ __volatile__ ("msr cpsr_c, %0\n\t" : : "r" (flags):);
}

#define HwSaveFlags(flags) ((flags)=0)

/*TODO ARMPorting: To be tested*/
//ToDo XM Hay que ver que pasa con las FIQ. S_IRQ ==> S_FIQ
static inline xm_s32_t HwIsSti(void) {
	xm_u32_t retval=0;
    __asm__ __volatile__ ("mrs %0, cpsr\n\t" : "=r" (retval):);
//    return !(retval & PSR_I);
    return ((!(retval & PSR_I))||(!(retval & PSR_F)));
}

#endif /* _XM_ARM_SPINLOCK_H_ */
