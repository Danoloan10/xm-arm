/*
 * $FILE: io.h
 *
 * IO devices access
 *
 * $VERSION$
 *
 * Author: Miguel Masmano <mmasmano@fentiss.com>
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE file for the license terms.
 */

#ifndef _XM_ARCH_IO_H_
#define _XM_ARCH_IO_H_
//
#ifdef CONFIG_ARM
#ifdef _XM_KERNEL_

#include <arch/arch_types.h>

#ifndef __ASSEMBLY__

/*inline void StoreIoReg(xmAddress_t pAddr, xm_u32_t value) {
    *((volatile xm_u32_t *)pAddr)=value;
}*/

#define StoreIoReg(pAddr, value)	\
		(*((volatile xm_u32_t *)pAddr)=value)

/*inline xm_u32_t LoadIoReg(xmAddress_t pAddr) {
    return *((volatile xm_u32_t *)pAddr);
}*/

#define LoadIoReg(pAddr)	\
		(*((volatile xm_u32_t *)pAddr))

#endif


/*32BITS*/
#define ReadReg32(Base, RegOffset)			((xm_u32_t)(*((volatile xm_u32_t *)(((Base)+(RegOffset))))))
#define WriteReg32(Base, RegOffset, value)	(*((volatile xm_u32_t *)(((Base)+(RegOffset)))) = ((volatile xm_u32_t)(value)))
#define SetReg32(Base, RegOffset, value)	(*((volatile xm_u32_t *)(((Base)+(RegOffset)))) |= ((volatile xm_u32_t)(value)))
#define ClearReg32(Base, RegOffset, value)	(*((volatile xm_u32_t *)(((Base)+(RegOffset)))) &= (~((volatile xm_u32_t)(value))))
#define ClearNSetReg32(Base, RegOffset, mask, value)	(*((volatile xm_u32_t *)(((Base)+(RegOffset)))) = (((*((volatile xm_u32_t *)(((Base)+(RegOffset))))) & (~((volatile xm_u32_t)(mask)))) | ((volatile xm_u32_t)(value))))
/*64BITS*/
#define ReadReg64(Base, RegOffset)			((xm_u64_t)(*((volatile xm_u64_t*)(((Base)+(RegOffset))))))
#define WriteReg64(Base, RegOffset, value)	(*((volatile xm_u64_t *)(((Base)+(RegOffset)))) = ((volatile xm_u64_t)(value)))
#define SetReg64(Base, RegOffset, value)	(*((volatile xm_u64_t *)(((Base)+(RegOffset)))) |= ((volatile xm_u64_t)(value)))
#define ClearReg64(Base, RegOffset, value)	(*((volatile xm_u64_t *)(((Base)+(RegOffset)))) &= (~((volatile xm_u64_t)(value))))
#define ClearNSetReg64(Base, RegOffset, mask, value)	(*((volatile xm_u64_t *)(((Base)+(RegOffset)))) = (((*((volatile xm_u64_t *)(((Base)+(RegOffset))))) & (~((volatile xm_u64_t)(mask)))) | ((volatile xm_u64_t)(value))))

/*GENERICS*/
#define ReadReg					ReadReg32
#define WriteReg				WriteReg32
#define SetReg					SetReg32
#define ClearReg				ClearReg32
#define ClearNSetReg			ClearNSetReg32
//#define BIT(X)					((1)<<(X))

#endif /* _XM_KERNEL_ */
#endif /* CONFIG_ARM */
#endif /*_XM_ARCH_IO_H_*/
