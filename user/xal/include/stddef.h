/*
 * $FILE: stddef.h
 *
 * stddef file
 *
 * $VERSION$
 *
 * Author: Miguel Masmano <mmasmano@fentiss.com>
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license terms.
 */

#ifndef _XAL_STDDEF_H_
#define _XAL_STDDEF_H_

#ifdef __cplusplus
extern "C" {
#endif

#undef offsetof
#ifdef __compiler_offsetof
#define offsetof(_type, _member) __compiler_offsetof(_type,_member)
#else
#define offsetof(_type, _member) ((xmSize_t) &((_type *)0)->_member)
#endif

typedef xm_u32_t size_t;
typedef xm_s32_t ssize_t;

#ifdef __cplusplus
}
#endif

#endif
