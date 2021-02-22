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

#define OCMADDRESS  ((xm_u32_t *)(0xFFFF0000))

void PartitionMain(void)
{
    volatile xm_u32_t * a = OCMADDRESS;
    xm_u32_t i, read, writed;

    PRINT("Example.010\n");
    PRINT("Accessing OCM on address 0x%x\n", OCMADDRESS);
    XM_idle_self();
    writed = *a;
    for(i=0;i<10;i++)
    {
        read = *a;
        PRINT("In address 0x%x, %d has been read %s expected\n", a, read, ((read == writed)?"as is":"but not"));
        writed = read + 5;
        *a = writed;
        PRINT("In address 0x%x, %d has been writed\n", a, writed);
        XM_idle_self();
    }
    PRINT("Example Ended!\n");
    PRINT("Going Halt\n");
    XM_halt_partition(XM_PARTITION_SELF);
}
