/*
 * $FILE: hypercalls.c
 *
 * XM system calls definitions
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
 * - [12/01/16:XM-ARM-2:SPR-150615-01:#59]: Remove dead code.
 */

#include <xmhypercalls.h>
#include <xm_inc/hypercalls.h>
#include <hypervisor.h>

void XM_disable_irqs(void) {
    __asm__ __volatile__ ("cpsid if\n\t");
}

void XM_enable_irqs(void) {
    __asm__ __volatile__ ("cpsie if\n\t");
}
