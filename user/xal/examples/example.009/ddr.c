/*
 * $FILE: partition.c
 *
 * Fent Innovative Software Solutions
 *
 * $LICENSE:
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <string.h>
#include <stdio.h>
#include <xm.h>
#include <irqs.h>

#define PRINT(...) do { \
        printf("[P%d] ", XM_PARTITION_SELF); \
        printf(__VA_ARGS__); \
} while (0)



xm_u32_t * p;

/*SELECT THE TEST: 1-2-3-4 */
#define TEST (1)

#if ((TEST<1) || (TEST>4))
#   error "Test number is not selected."
#endif

void PartitionMain(void)
{
    xm_u32_t val;
    PRINT("------------------------\n");
    PRINT("---DDR Example Start!---\n");

#if TEST==1    /* Accessing own memory in the partition memory space. */
    xm_u32_t   v = 1111*(XM_PARTITION_SELF+1);
    p = (&v);
    PRINT("\n");
    PRINT("  * Accessing own memory(0x%x)\n", p);
    val = *p;
    PRINT("  Read Value (%d)\n", val);

#elif TEST==2   /* Accessing own memory but not mapped.*/
    p = ((xm_u32_t *)(0x18000000));
    *p = 1111*(XM_PARTITION_SELF+1);
    xmWord_t * pageTable = XM_params_get_userPTL1();
    unmapAddress(pageTable, 0x18000000);
    __asm__ __volatile__(\
            "mcr  p15, 0, r0, c8, c5, 0\n\t"      /*flush tlb*/                       \
            "isb\n\t"                                                                 \
            "dsb\n\t":::);
    PRINT("\n");
    PRINT("  * Accessing memory NS but not mapped (0x%x)\n", p);
    val = *p;
    PRINT("  Read Value (%d)\n", val);

#elif TEST==3   /* Accessing mapped memory outside NS space*/
    p = ((xm_u32_t*)( ((!XM_PARTITION_SELF)?(0x14000000):(0x10000000)) ));
    xmWord_t * pageTable = XM_params_get_userPTL1();
    mapAddress(pageTable, ((xmAddress_t)(p)));
    __asm__ __volatile__(\
            "mcr  p15, 0, r0, c8, c5, 0\n\t"      /*flush tlb*/                       \
            "isb\n\t"                                                                 \
            "dsb\n\t":::);
    PRINT("\n");
    PRINT("  * Accessing memory map but Secure (0x%x)\n", p);
    val = *p;
    PRINT("  Read Value (%d)\n", val);

#elif TEST==4   /* Accessing outside NS space and not mapped*/
    p = ((xm_u32_t *)(0x1C000000));
    PRINT("\n");
    PRINT("  * Accessing memory Secure and not mapped (0x%x)\n", p);
    val = *p;
    PRINT("  Read Value (%d)\n", val);
#endif
    PRINT("  * End Example DDR. GOING HALT\n");

        XM_halt_partition(XM_PARTITION_SELF);
}
