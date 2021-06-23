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
#include <xal.h>

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
#define HEAP_SIZE (1 << 27) 

class A {
	private:
		int a;
	public:
		int geta() {
			return a;
		}
		int geta(int &v) {
			v = a;
		}
		void sum(int b) {
			a += b;
		}
		A(int _a):a(_a) {
			printf("hola: %d\n", a);
		}
};

A global(10);

void PartitionMain(void)
{
	A a(3), *b;

	// local constructors work
	printf("%d\n", a.geta());
	a.sum(4 % 3);
	printf("%d\n", a.geta());

	// global constructors do not work
	printf("%d\n", global.geta());
	global.sum(4);
	printf("%d\n", global.geta());

	// test new and delete operators
	int bint;
	b = new A(5);
	b->geta(bint);
	printf("%d\n", bint);
	b->sum(4);
	printf("%d\n", b->geta());
	delete(b);

	XM_halt_partition(XM_PARTITION_SELF);
}
