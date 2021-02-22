/*
 * $FILE: string.c
 *
 * String related functions
 *
 * $VERSION$
 *
 * Author: Javier Coronel <jcoronel@fentiss.com>
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE file for the license terms.
 */

#include <stdc.h>
#include <arch/xm_def.h>

#ifdef __ARCH_MEMCPY

extern void _memcpy(void *dst, void *src, xm_u32_t count);

void *memcpy(void *dst, const void *src, xm_u32_t count) {
    _memcpy((void *)dst,(void *)src,count);
    return dst;
}

#endif
