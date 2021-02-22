/*
 * $FILE: kthread.c
 *
 * Kernel and Guest context
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
#include <boot.h>
#include <kthread.h>
#include <sched.h>
#include <stdc.h>
#include <vmmap.h>
#include <xmef.h>

void FillMmPartCtrlTabWarm(partitionControlTable_t *partCtrlTab, kThread_t *k) {
}

void FillMmPartCtrlTabCold(partitionControlTable_t *partCtrlTab, kThread_t *k) {
}
