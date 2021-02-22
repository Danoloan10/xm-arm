/*
 * $FILE: partition.c
 *
 * Fent Innovative Software Solutions
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE file for the license terms.
 */
#include <string.h>
#include <stdio.h>
#include <xm.h>
#include <irqs.h>

#define PRINT(...) do { \
        printf("[vc%d:P%d] ", XM_get_vcpuid(), XM_PARTITION_SELF); \
        printf(__VA_ARGS__); \
} while (0)


void PartitionMain(void)
{
    PRINT("Hello World!\n"); 
    XM_idle_self();
    PRINT("Hello World!\n"); 
    XM_idle_self();
    PRINT("Hello World!\n"); 
    XM_idle_self();

    XM_halt_partition(XM_PARTITION_SELF);
}
