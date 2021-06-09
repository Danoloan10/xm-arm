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

void *malloc(size_t size)
{
	return NULL;
}

void free(void *ptr)
{
}
