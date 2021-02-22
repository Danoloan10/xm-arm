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

#ifndef _XM_LINKAGE_H_
#define _XM_LINKAGE_H_

#ifdef _XM_KERNEL_
#include <arch/linkage.h>
#include <arch/paging.h>
#else
#include <xm_inc/arch/linkage.h>
#include <xm_inc/arch/paging.h>
#endif

#ifndef CONFIG_ARM
	#define PAGE_ALIGN .align PAGE_SIZE
#else
	#define PAGE_ALIGN .balign PAGE_SIZE
#endif

#ifndef _ASSEMBLY_
#define __NOINLINE __attribute__((noinline))
#define __PACKED __attribute__((__packed__))
#define __WARN_UNUSED_RESULT __attribute__ ((warn_unused_result))
#endif

#define SYMBOL_NAME(X) X
#define SYMBOL_NAME_LABEL(X) X##:

#define ENTRY(name) \
    .globl SYMBOL_NAME(name); \
    ASM_ALIGN; \
    SYMBOL_NAME_LABEL(name)

#define __STR(x) #x
#define TO_STR(x) __STR(x)

#endif
