/*
 * $FILE: linkage.h
 *
 * Definition of some macros to ease the interoperatibility between
 * assembly and C
 *
 * $VERSION$
 *
 * Author: Miguel Masmano <mmasmano@fentiss.com>
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license terms.
 */

#ifndef _XM_ARCH_LINKAGE_H_
#define _XM_ARCH_LINKAGE_H_

#ifndef _ASSEMBLY_
#define ALIGNMENT 4
#define ASM_ALIGN .balign ALIGNMENT
#define __stdcall
#define __hypercall __attribute__((noinline, section(".text")))
#define ALIGNED_C __attribute__((aligned(2)))
#endif

#endif
