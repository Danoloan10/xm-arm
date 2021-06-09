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

extern ssize_t memcmp(const void *dst, const void *src, size_t count);
extern void *  memcpy(void *dst, const void* src, size_t count);
extern void *  memmove(void *dst, const void *src, size_t count);
extern void *  memset(void *dst, xm_s32_t s, size_t count);

extern char *  strcat(char *s, const char* t);
extern ssize_t strcmp(const char *s, const char *t);
extern char *  strcpy(char *dst, const char *src);
extern size_t  strlen(const char *s);

extern char *  strncat(char *s, const char *t, size_t n);
extern ssize_t strncmp(const char *s1, const char *s2, size_t n);
extern char *  strncpy(char *dest, const char *src, size_t n);
extern size_t  strnlen(const char *s, size_t len);

extern char *  strchr(const char *t, xm_s32_t c);
extern char *  strrchr(const char *t, xm_s32_t c);
extern char *  strstr(const char *haystack, const char *needle);

#ifdef __cplusplus
}
#endif

#endif
