/*
 * 
 * - printf() based on sprintf() from gcctest9.c Volker Oth
 * - Changes made by Holger Klabunde
 * - Changes made by Martin Thomas for the efsl debug output
 * - Changes made by speiro for lpc2000 devices for PaRTiKle
 * 
 */

/*
 * Changelog: 
 *   - [02/08/2016] jcoronel  XM-ARM-2:SPR-160802-01: Remove functions related to "printf". 
                              This functionality is reemplimented in the file stdio.c
 */

#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <xm.h>

#include <stdio.h>

xm_s32_t atoi(const char* s)
{
    long int v=0;
    int sign=1;
    unsigned base;

    while ( *s == ' '  ||  (unsigned int)(*s - 9) < 5u){
        s++;
    }

    switch (*s) {
        case '-':
            sign=-1;
        case '+':
            ++s;
    }
	
    base=10;
	if (*s == '0') {
        if (*(s+1) == 'x') { /* WARNING: assumes ascii. */
            s++;
            base = 16;
        }
        else {
            base = 8;
        }
        s++;
	}

    while ((unsigned int) (*s - '0') < base){
        v = v * base + *s - '0'; 
        ++s;
    }

    if (sign==-1){
        return -v;
    }
    return v;
}

/* strtoul: xm */

/* strtol: xm */

/* strtoull: xm */

/* strtoll: xm */

/* basename: xm */

/* exit: ??? */


struct __buddy_entry {
	bool free;           // u8 - 1B
	xm_u8_t order;       // u8 - 1B
};

// N = M / ( 2^k + T )
static inline size_t __buddy_list_size(const struct xmPhysicalMemMap *xmPM)
{
	return (size_t)
		(xmPM->size / (BLOCK_BYTE_SIZE + sizeof(struct __buddy_entry)));
}

// H = M - N * T
static inline size_t __heap_size(const struct xmPhysicalMemMap *xmPM)
{
	return (size_t) xmPM->size
		- (__buddy_list_size(xmPM) * sizeof(struct __buddy_entry));
}

static inline struct __buddy_entry *__buddy_list_start
	(
		const struct xmPhysicalMemMap *xmPM
	)
{
	return (struct __buddy_entry *) ADDR2PTR(
			xmPM->startAddr + __heap_size(xmPM)
		);
}

static inline xmAddress_t __heap_start(const struct xmPhysicalMemMap *xmPM)
{
	return xmPM->startAddr;
}

static inline xm_u8_t __min_greater_order(size_t size)
{
	xm_u8_t order = ((size & (size - 1)) == 0) ? 0 : 1;
	while (size >>= 1) order++;

	if (order > BLOCK_BYTE_ORDER) {
		return order - BLOCK_BYTE_ORDER;
	} else {
		return 0;
	}
}

#define IS_READ_ONLY(xmPM) (xmPM->flags & (XM_MEM_AREA_READONLY | XM_MEM_AREA_ROM))

struct __buddy_entry *buddy_list = NULL;
size_t buddy_list_size = 0;

struct xmPhysicalMemMap *xmPM = NULL;

void *malloc(size_t size)
{
	// zedboard: 512MB

	xm_u8_t order, target_order;
	xm_u32_t buddy, block;

	void *mem = NULL;

	if (xmPM == NULL) {
		struct xmPhysicalMemMap *iter = XM_get_partition_mmap();

		int i;
		for(i = 0; (i < XM_params_get_PCT()->noPhysicalMemAreas) && (xmPM == NULL); i++)
		{
			if (iter->flags & XM_MEM_AREA_FLAG0) {
				xmPM = iter;
			} else iter++;
		}

		if (xmPM == NULL)
			return NULL;
	}

	if (buddy_list == NULL) {
		buddy_list      = __buddy_list_start(xmPM);
		buddy_list_size = __buddy_list_size (xmPM);
		buddy_list[0].free  = true;
		buddy_list[0].order =  __min_greater_order(__heap_size(xmPM));
	}

	target_order = __min_greater_order(size);

	for (block = 0; (mem == NULL) && (block < buddy_list_size); block += (1 << target_order)) {
		if (buddy_list[block].free) {
			order = buddy_list[block].order;

			while (order > target_order) {
				order--;

				buddy = block ^ order;

				buddy_list[block].order = order;

				buddy_list[buddy].free  = true;
				buddy_list[buddy].order = order;
			}

			if (order == target_order) {
				buddy_list[block].free  = false;
				mem = ADDR2PTR(__heap_start(xmPM) + (1 << BLOCK_BYTE_ORDER) * block);
			}
		}
	}

	return mem;
}

void free(void *ptr)
{
	xm_u32_t block, buddy;
	xm_u8_t order;
	xmAddress_t address;

	if ((ptr == NULL) || (buddy_list == NULL))
		return;

	address = PTR2ADDR(ptr);

	block = (address - __heap_start(xmPM)) >> BLOCK_BYTE_ORDER;
	buddy_list[block].free = true;

	order = buddy_list[block].order;
	buddy = block ^ (1 << order);
	
	while (buddy < buddy_list_size
			&&  buddy_list[buddy].free
			&& (buddy_list[buddy].order >= order))
	{
		order++;

		block >>= order;
		block <<= order;

		buddy_list[block].order = order;

		buddy = block ^ (1 << order);
	}
}
