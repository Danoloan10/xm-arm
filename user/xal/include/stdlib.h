/*
 * $FILE: stdlib.h
 *
 * standard library functions
 *
 * $VERSION$
 *
 * Author: Miguel Masmano <mmasmano@fentiss.com>
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license terms.
 */

#ifndef _XAL_STDLIB_H_
#define _XAL_STDLIB_H_

#include <stddef.h>

#ifndef NULL
#define NULL 0
#endif

#ifdef __cplusplus
extern "C" {
#endif

extern xm_s32_t atoi(const char* s);
extern xm_u32_t strtoul(const char *ptr, char **endptr, xm_s32_t base);
extern xm_s32_t strtol(const char *nptr, char **endptr, xm_s32_t base);
extern xm_u64_t strtoull(const char *ptr, char **endptr, xm_s32_t base);
extern xm_s64_t strtoll(const char *nptr, char **endptr, xm_s32_t base);
extern char *basename(char *path);
extern void exit(xm_s32_t status);

extern void *malloc(size_t size);
extern void free(void *ptr);

#ifdef __cplusplus
}
#endif

#endif
