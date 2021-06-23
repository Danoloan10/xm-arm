/*
 * $FILE: xmhypercalls.h
 *
 * Arch hypercalls definition
 *
 * $VERSION$
 *
 * Author: Miguel Masmano <mmasmano@fentiss.com>
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license terms.
 */
/*
 * Changelog:
 * - [21/01/16:XM-ARM-2:SPR-160121-05:#107] HCall API is not up to date: XM_arm_hm_raise_event lack.
 */

#ifndef _ARCH_LIB_XM_HYPERCALLS_H_
#define _ARCH_LIB_XM_HYPERCALLS_H_

#ifdef _XM_KERNEL_
#error Guest file, do not include.
#endif /* _XM_KERNEL_ */

//#define __STR(x) #x
//#define TO_STR(x) __STR(x)
#define ASMLINK

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __ASSEMBLY__

#include <xm_inc/linkage.h>

#include <xm_inc/hypercalls.h>

/* <track id="DOC_HYPERCALL_MECHANISM">   */
/*ARM*/

#define __DO_XMHC(hc) "smc "TO_STR(hc)"\n\t" \
                      ".long 0x24786879\n\t"

/*"swi #"TO_STR(hc)"\n\t" \*/
/*                      ".long 0x24786879\n\t"*/

#define _XM_HCALL0(_hc_nr, _r) \
    __asm__ __volatile__ (__DO_XMHC(_hc_nr) \
                          "mov %0, r0\n\t" : "=r" (_r) : : \
                          "r0")

#define _XM_HCALL1(a0, _hc_nr, _r) \
    __asm__ __volatile__ ("mov r0, %1\n\t" \
                          __DO_XMHC(_hc_nr) \
                          "mov %0, r0\n\t" : "=r" (_r) : "0" (a0):  \
                          "r0")

#define _XM_HCALL2(a0, a1, _hc_nr, _r) \
    __asm__ __volatile__ ("mov r0, %1\n\t" \
                          "mov r1, %2\n\t"  \
                          __DO_XMHC(_hc_nr) \
                          "mov %0, r0\n\t" : "=r" (_r) : "0" (a0), "r" (a1): \
                          "r0", "r1")


#define _XM_HCALL3(a0, a1, a2, _hc_nr, _r) \
    __asm__ __volatile__ ("mov r0, %1\n\t" \
                          "mov r1, %2\n\t"  \
                          "mov r2, %3\n\t"  \
                          __DO_XMHC(_hc_nr) \
                          "mov %0, r0\n\t" : "=r" (_r) : "0" (a0), "r" (a1), "r" (a2): \
                          "r0", "r1", "r2")

#define _XM_HCALL4(a0, a1, a2, a3, _hc_nr, _r)  \
    __asm__ __volatile__ ("mov r0, %1\n\t" \
                          "mov r1, %2\n\t"  \
                          "mov r2, %3\n\t"  \
                          "mov r3, %4\n\t"  \
                          __DO_XMHC(_hc_nr) \
                          "mov %0, r0\n\t" : "=r" (_r) : "0" (a0), "r" (a1), "r" (a2), "r" (a3): \
                          "r0", "r1", "r2", "r3")

/*No available in ARM*/
/*
#define _XM_HCALL5(a0, a1, a2, a3, a4, _hc_nr, _r) \
    __asm__ __volatile__ ("mov r0, %1\n\t" \
                          "mov r1, %2\n\t"  \
                          "mov r2, %3\n\t"  \
                          "mov r3, %4\n\t"  \
                          "mov r4, %5\n\t"  \
                          __DO_XMHC(_hc_nr) \
                          "mov %0, r0\n\t" : "=r" (_r) : "0" (a0), "r" (a1), "r" (a2), "r" (a3), "r" (a4): \
                          "r0", "r1", "r2", "r3")
*/

#define xm_hcall0(_hc) \
ASMLINK void XM_##_hc(void) { \
    xm_s32_t _r ; \
    _XM_HCALL0(_hc##_nr, _r); \
}

#define xm_hcall0r(_hc) \
ASMLINK xm_s32_t XM_##_hc(void) { \
    xm_s32_t _r ; \
    _XM_HCALL0(_hc##_nr, _r); \
    return _r; \
}

#define xm_hcall1(_hc, _t0, _a0) \
ASMLINK void XM_##_hc(_t0 _a0) { \
    xm_s32_t _r ; \
    _XM_HCALL1(_a0, _hc##_nr, _r); \
}

#define xm_hcall1r(_hc, _t0, _a0) \
ASMLINK xm_s32_t XM_##_hc(_t0 _a0) { \
    xm_s32_t _r ; \
    _XM_HCALL1(_a0, _hc##_nr, _r); \
    return _r; \
}

#define xm_hcall2(_hc, _t0, _a0, _t1, _a1) \
ASMLINK void XM_##_hc(_t0 _a0, _t1 _a1) { \
    xm_s32_t _r ; \
    _XM_HCALL2(_a0, _a1, _hc##_nr, _r); \
}

#define xm_hcall2r(_hc, _t0, _a0, _t1, _a1) \
ASMLINK xm_s32_t XM_##_hc(_t0 _a0, _t1 _a1)  { \
    xm_s32_t _r ; \
    _XM_HCALL2(_a0, _a1, _hc##_nr, _r); \
    return _r; \
}

#define xm_hcall3(_hc, _t0, _a0, _t1, _a1, _t2, _a2) \
ASMLINK void XM_##_hc(_t0 _a0, _t1 _a1, _t2 _a2) { \
    xm_s32_t _r ; \
    _XM_HCALL3(_a0, _a1, _a2, _hc##_nr, _r); \
}

#define xm_hcall3r(_hc, _t0, _a0, _t1, _a1, _t2, _a2) \
ASMLINK xm_s32_t XM_##_hc(_t0 _a0, _t1 _a1, _t2 _a2) { \
    xm_s32_t _r ; \
    _XM_HCALL3(_a0, _a1, _a2, _hc##_nr, _r); \
    return _r; \
}

#define xm_hcall4(_hc, _t0, _a0, _t1, _a1, _t2, _a2, _t3, _a3) \
ASMLINK void XM_##_hc(_t0 _a0, _t1 _a1, _t2 _a2, _t3 _a3) { \
    xm_s32_t _r ; \
    _XM_HCALL4(_a0, _a1, _a2, _a3, _hc##_nr, _r); \
}

#define xm_hcall4r(_hc, _t0, _a0, _t1, _a1, _t2, _a2, _t3, _a3) \
ASMLINK xm_s32_t XM_##_hc(_t0 _a0, _t1 _a1, _t2 _a2, _t3 _a3) { \
    xm_s32_t _r ; \
    _XM_HCALL4(_a0, _a1, _a2, _a3, _hc##_nr, _r); \
    return _r; \
}

#define xm_hcall5(_hc, _t0, _a0, _t1, _a1, _t2, _a2, _t3, _a3, _t4, _a4) \
ASMLINK void XM_##_hc(_t0 _a0, _t1 _a1, _t2 _a2, _t3 _a3, _t4 _a4) { \
    xm_s32_t _r ; \
    _XM_HCALL5(_a0, _a1, _a2, _a3, _a4, _hc##_nr, _r);  \
}

#define xm_hcall5r(_hc, _t0, _a0, _t1, _a1, _t2, _a2, _t3, _a3, _t4, _a4) \
ASMLINK xm_s32_t XM_##_hc(_t0 _a0, _t1 _a1, _t2 _a2, _t3 _a3, _t4 _a4) { \
    xm_s32_t _r ; \
    _XM_HCALL5(_a0, _a1, _a2, _a3, _a4, _hc##_nr, _r);  \
    return _r; \
}

extern xm_s32_t XM_arm_inport(xm_u32_t portAddr, xm_u32_t *ptrValue);
extern xm_s32_t XM_arm_outport(xm_u32_t portAddr, xm_u32_t value);
extern xm_s32_t XM_arm_hm_raise_event(xm_u32_t hmEvent, void * dataEvent);

#else

#define __XM_HC(hc) smc hc ; \
        .long 0x24786879

#endif /* __ASSEMBLY__ */

#ifdef __cplusplus
} // extern C
#endif

#endif /* _ARCH_LIB_XM_HYPERCALLS_H_ */
