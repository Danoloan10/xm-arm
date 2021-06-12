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



#define BLOCK_BYTE_ORDER 6 // 64B, p = 3,125%
#define BLOCK_BYTE_SIZE  ( 1 << BLOCK_BYTE_ORDER )
#define STACK_SIZE_TOTAL (STACK_SIZE - STACK_SIZE_EXCPT)

struct __buddy_entry {
	bool free;           // u8 - 1B
	xm_u8_t order;       // u8 - 1B
};

static inline size_t __mem_size(const struct xmPhysicalMemMap *xmPM)
{
	return (size_t) xmPM->size;
}

static inline size_t __n_buddy_blocks(const struct xmPhysicalMemMap *xmPM)
{
	return (size_t) ((__mem_size(xmPM) - STACK_SIZE_TOTAL) / (BLOCK_BYTE_SIZE + sizeof(struct __buddy_entry)));
}

static inline size_t __buddy_list_size(const struct xmPhysicalMemMap *xmPM)
{
	return (size_t) __n_buddy_blocks(xmPM) * sizeof(struct __buddy_entry);
}

static inline size_t __heap_size(const struct xmPhysicalMemMap *xmPM)
{
	return (size_t) __mem_size(xmPM) - __buddy_list_size(xmPM);
}

// TODO revisar: quizás no empieze siempre en mappedAt
static inline xmAddress_t __memory_start(const struct xmPhysicalMemMap *xmPM)
{
	return xmPM->mappedAt;
}

static inline struct __buddy_entry *__buddy_list_start
	(
		const struct xmPhysicalMemMap *xmPM
	)
{
	return (struct __buddy_entry *) __memory_start(xmPM);
}

static inline xmAddress_t __heap_start(const struct xmPhysicalMemMap *xmPM)
{
	return __memory_start(xmPM) + __buddy_list_size(xmPM);
}

static inline xm_u8_t __min_greater_order(size_t size)
{
	xm_u8_t order = 0;
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

void *malloc(size_t size)
{
	// zedboard: 512MB
	
	// static const xm_s32_t nareas = -1;

	struct xmPhysicalMemMap *xmPM;
	//xm_u32_t partno = 0;
	xm_u8_t order;
	int i, j;

	void *mem = NULL;

	//TODO añadir más comprobaciones

//	if (nareas <= 0) 
//		nparts = XM_params_get_PTC()->noPhysicalMemAreas;
//	if (nareas <= 0)
//		return NULL;

	xmPM = XM_get_partition_mmap();

//	for(partno = 0; partno < nareas || mem != NULL; partno++) {
//		// TODO terminar
//		xmPM++;
//	}

	if (buddy_list == NULL) {
		buddy_list      = __buddy_list_start(xmPM);
		buddy_list_size = __buddy_list_size (xmPM);
	}

	order = __min_greater_order(size);

	for (i = 0; (mem == NULL) && (i < buddy_list_size); i += (1 << order)) {
		bool free = true;

		for (j = 0; free && j < (i + (1 << order)); j++) {
			free &= buddy_list[i+j].free;
		}

		if (free) {
			for (j = 0; j < (i + (1 << order)); j++) {
				buddy_list[i+j].free  = false;
				buddy_list[i+j].order = order;
			}
			mem = ADDR2PTR(__heap_start(xmPM) + (1 << (order + BLOCK_BYTE_ORDER)));
		}
	}

	return mem;
}

void free(void *ptr)
{
	xm_u8_t block_start, order;
	xmAddress_t address;
	int i;

	if ((ptr == NULL) || (buddy_list == NULL))
		return;

	address = PTR2ADDR(ptr);
	block_start = address >> BLOCK_BYTE_ORDER;

	order = buddy_list[block_start].order;

	for (i = block_start; i < block_start + order; i++) {
		buddy_list[i].free  = true;
		buddy_list[i].order = 0;
	}
}
