/*
 * $FILE: user/xal/arm/gic.c
 *
 * $VERSION$
 *
 * Author: Manuel Muñoz <mmunoz@ai2.upv.es>
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license terms.
 */

#include <arm/gic.h>

void XalEndOfIRQ(xm_u32_t irqNr)
{
	WriteReg(ICC_BASE_ADDRESS, ICCEOIR, irqNr);
}

void XalGetIrqNCpu(xm_u32_t irqNr, xm_u32_t *irq, xm_u32_t *cpu)
{
	*cpu = (((irqNr)&(ICCIAR_CPUMASK))>>10);
	*irq = ((irqNr)&(ICCIAR_IRQMASK));
}
