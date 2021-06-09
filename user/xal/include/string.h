/*
 * $FILE: string.h
 *
 * string functions
 *
 * $VERSION$
 *
 * Author: Miguel Masmano <mmasmano@fentiss.com>
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license terms.
 */

#ifndef _XAL_STRING_H_
#define _XAL_STRING_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <xm.h>
#include <stddef.h>

extern void *memset(void *dst, xm_s32_t s, size_t count);
extern void *memcpy(void *dst, const void* src, size_t count);
extern ssize_t memcmp(const void *dst, const void *src, size_t count);
extern char *strcpy(char *dst, const char *src);
extern char *strncpy(char *dest, const char *src, xmSize_t n);
extern char *strcat(char *s, const char* t);
extern char *strncat(char *s, const char *t, xmSize_t n);
extern ssize_t strcmp(const char *s, const char *t);
extern ssize_t strncmp(const char *s1, const char *s2, xmSize_t n);
extern size_t strlen(const char *s);
extern char *strrchr(const char *t, xm_s32_t c);
extern char *strchr(const char *t, xm_s32_t c);
extern char *strstr(const char *haystack, const char *needle);
extern void *memmove(void *dst, const void *src, xmSize_t count);

extern size_t strnlen(const char *s, size_t len);

#ifdef __cplusplus
}
#endif

#endif
