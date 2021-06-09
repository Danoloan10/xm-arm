/*
 * $FILE: stdio.h
 *
 * stdio file
 *
 * $VERSION$
 *
 * Author: Miguel Masmano <mmasmano@fentiss.com>
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license terms.
 */

#ifndef _XAL_STDIO_H_
#define _XAL_STDIO_H_

#include <stdarg.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

extern ssize_t printf(const char *format,...);
extern ssize_t vprintf(const char *fmt, va_list args);
extern ssize_t sprintf(char *s, char const *fmt, ...);
extern ssize_t snprintf(char *s, size_t n, const char *fmt, ...);
extern ssize_t vsprintf(char *str, const char *format, va_list ap);
extern ssize_t putchar(xm_s32_t c);

#define fprintf(file, ...) printf(__VA_ARGS__) 

extern ssize_t vsnprintf(char *str, size_t n, const char *fmt, va_list ap);

#ifdef __cplusplus
}
#endif

#endif
