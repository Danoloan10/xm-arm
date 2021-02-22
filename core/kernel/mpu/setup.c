/*
 * $FILE: setup.c
 *
 * Setting up and starting up the kernel
 *
 * $VERSION$
 *
 * Author: Miguel Masmano <mmasmano@fentiss.com>
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license terms.
 */

#include <assert.h>
#include <sched.h>
#include <kthread.h>
#include <spinlock.h>

__NOINLINE void FreeBootMem(void) {
    extern void IdleTask(void);  
    ASSERT(!HwIsSti());
    GET_LOCAL_SCHED()->flags|=LOCAL_SCHED_ENABLED;
    HwSti();
    Schedule();
    IdleTask();
}
