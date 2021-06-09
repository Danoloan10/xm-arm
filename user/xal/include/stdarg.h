/*
 * $FILE: stdarg.h
 *
 * std arguments
 *
 * $VERSION$
 *
 * Author: Miguel Masmano <mmasmano@fentiss.com>
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license terms.
 */

#ifndef _XAL_STDARG_H_
#define _XAL_STDARG_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef __builtin_va_list va_list;

#define va_start(v, l) __builtin_va_start(v,l)
#define va_end(v) __builtin_va_end(v)
#define va_arg(v, l) __builtin_va_arg(v,l)

#ifdef __cplusplus
}
#endif

#endif
