/*
 * $FILE: ocmpartition.c
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

#define FLAG2STR(flags, name) (((flags&name)==name)?#name:"")
void PartitionMain(void)
{
    xm_u32_t i;
    PRINT("Example.010\n");
    PRINT("I'am a XM partition running on OCM memory\n");
    XM_idle_self();
    PRINT("My memory map is:\n");

    struct xmPhysicalMemMap * mem = XM_get_partition_mmap();

    for(i=0;i<XM_params_get_PCT()->noPhysicalMemAreas;i++)
    {
        PRINT("Mem Area %s : [0x%x:0x%x] mapped at 0x%x [%s%s%s%s]\n",
            mem[i].name,
            mem[i].startAddr,
            mem[i].startAddr+mem[i].size-1,
            mem[i].mappedAt,
            FLAG2STR(mem[i].flags,XM_MEM_AREA_UNMAPPED),
            FLAG2STR(mem[i].flags,XM_MEM_AREA_READONLY),
            FLAG2STR(mem[i].flags,XM_MEM_AREA_UNCACHEABLE),
            FLAG2STR(mem[i].flags,XM_MEM_AREA_ROM),
            FLAG2STR(mem[i].flags,XM_MEM_AREA_TAGGED),
            FLAG2STR(mem[i].flags,XM_MEM_AREA_FLAG0),
            FLAG2STR(mem[i].flags,XM_MEM_AREA_FLAG1),
            FLAG2STR(mem[i].flags,XM_MEM_AREA_FLAG2),
            FLAG2STR(mem[i].flags,XM_MEM_AREA_FLAG3));
    }
    PRINT("Example Ended!\n");
    PRINT("Going Halt\n");
    XM_halt_partition(XM_PARTITION_SELF);
}
