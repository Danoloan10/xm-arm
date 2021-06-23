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
#include <stdlib.h>
#include <xm.h>
#include <irqs.h>
#include <assert.h>

#define PRINT(...) do { \
        printf("[P%d] ", XM_PARTITION_SELF); \
        printf(__VA_ARGS__); \
} while (0)

#define TIME(__expr) do { \
	xmTime_t t1, t0; \
	assert(XM_get_time(XM_HW_CLOCK, &t0) == XM_OK); \
	__expr; \
	assert(XM_get_time(XM_HW_CLOCK, &t1) == XM_OK); \
	PRINT("time: %d\n", (int)(t1-t0)); \
} while (0)

// Heap size 128MB
// For the heap to be this size, the memory region must be at least
//     128MB + 2^(27-k)*2
// where k is the BLOCK_BYTE_ORDER parameter configured in XtratuM
// The default value for k is 6 -> 132MB of mem are required
#define HEAP_SIZE (1 << 27) 

void PartitionMain(void)
{
	int i;
	const char *hola = "hola", *adios = "adios";
	char *str = NULL, *other = NULL;

	// basic usage
	TIME(
		str = (char *) malloc(5 * sizeof(char));
	);
	assert(str != NULL);
	PRINT("hola?\n");
	strcpy(str, hola);
	PRINT("%s\n", str);
	TIME(
		free(str);
	);

	// check that free memory is used again
	other = (char *) malloc(1450 * sizeof(char));
	assert(str == other);

	// more than one object at a time
	str = (char *) malloc(6 * sizeof(char));
	assert(str != other);
	strcpy(str, adios);

	PRINT("hola?\n");
	PRINT("%s\n", other);
	PRINT("%s\n", str);

	free(str);
	free(other);

	// malloc more memory than available
	TIME(
		str = (char *) malloc(HEAP_SIZE + 1);
	);
	assert(str == NULL);

	// try to malloc more memory than available in two times
	other = (char *) malloc(1);
	assert(other != NULL);
	TIME(
		str = (char *) malloc(HEAP_SIZE);
	);
	assert(str == NULL);
	free(other);

	// now there is enough memory
	TIME(
		str = (char *) malloc(HEAP_SIZE);
	);
	assert(str != NULL);

	// check that memory can be used
	memset(str, 'o', HEAP_SIZE);
	for(i = 0; i < HEAP_SIZE; i++) {
		assert(str[i] == 'o');
	}

	PRINT("SUCCESS!\n");

    XM_halt_partition(XM_PARTITION_SELF);
}
