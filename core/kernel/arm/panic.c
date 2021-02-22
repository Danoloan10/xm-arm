/*
 * $FILE: panic.c
 *
 * Code executed in a panic situation
 *
 * $VERSION$
 *
 * Author: Miguel Masmano <mmasmano@fentiss.com>
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license terms.
 */

#include <kthread.h>
#include <processor.h>
#include <sched.h>
#include <stdc.h>
#include <objects/hm.h>

void DumpState(cpuCtxt_t *regs) {
    kprintf("**************************************************\n");
    kprintf("CPU state:\n");
    xm_s32_t i;
    for(i=0;i<13;i++)
    	kprintf("Reg r%d:0x%x\n",i,regs->r[i]);
    kprintf("PC:0x%x SP:0x%x TrapSP:0x%x LR:0x%x\n", regs->pc, regs->sp, regs->trapSp, regs->lr);
    kprintf("CPSR:0x%x SPSR:0x%x IRQNR:0x%x\n", regs->cpsr, regs->spsr, regs->irqNr);
#if defined(CONFIG_MMU)||defined(CONFIG_MPU)
    kprintf("FSR:0x%x FAR:0x%x\n", GetMmuFaultStatusReg(), GetMmuFaultAddressReg());
#endif
    kprintf("**************************************************\n");
}
