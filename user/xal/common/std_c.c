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

#include <stdarg.h>
#include <string.h>
#include <xm.h>

xm_s32_t atoi(const char* s) {
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

char *strchr(const char *t, int c) {
    register char ch;

    ch = c;
    for (;;) {
        if (*t == ch) break; if (!*t) return 0; ++t;
    }
    return (char*)t;
}


char *strcat(char *s, const char* t) {
    char *dest=s;
    s+=strlen(s);
    for (;;) {
        if (!(*s=*t)) break; ++s; ++t;
    }
    return dest;
}

char *strcpy(char *dest, const char* src) {
    xm_s8_t *ret = dest;
    while ((*dest++ = *src++))
        ;
    return ret;
}

char *strncat(char *s, const char *t, xmSize_t n) {
    char *dest=s;
    register char *max;
    s+=strlen(s);
    if ((max=s+n)==s) goto fini;
    for (;;) {
        if (!(*s = *t)) break; if (++s==max) break; ++t;
    }
    *s=0;
fini:
    return dest;
}

void *memset(void *dst, int s, unsigned int count) {
    register char * a = dst;
    count++;
    while (--count)
        *a++ = s;
    return dst;
}

void *memcpy(void* dst, const void* src, unsigned int count) {
    register char *d=dst;
    register const char *s=src;
    ++count;
    while (--count) {
        *d = *s;
        ++d; ++s;
    }
    return dst;
}

char *strncpy(char *dest, const char *src, xmSize_t n) {
    xm_s32_t j;

    memset(dest,0,n);

    for (j=0; j<n && src[j]; j++)
	dest[j]=src[j];

    if (j>=n)
	dest[n-1]=0;

    return dest;
}

int strncmp(const char *s1, const char *s2, xmSize_t n) {
  register const unsigned char *a=(const unsigned char*)s1;
  register const unsigned char *b=(const unsigned char*)s2;
  register const unsigned char *fini=a+n;

  while (a < fini) {
    register int res = *a-*b;
    if (res) return res;
    if (!*a) return 0;
    ++a; ++b;
  }
  return 0;
}
int strcmp(const char *s, const char *t) {
    char x;

    for (;;) {
	x = *s; if (x != *t) break; if (!x) break; ++s; ++t;
    }
    return ((int)x)-((int)*t);
}

int memcmp(const void *dst, const void *src, unsigned int count) {
    int r;
    const char *d=dst;
    const char *s=src;
    ++count;
    while (--count) {
        if ((r=(*d - *s)))
            return r;
        ++d;
        ++s;
    }
    return 0;
}

unsigned int strlen(const char *s) {
    unsigned int i;
    if (!s) return 0;
    for (i = 0; *s; ++s) ++i;
    return i;
}
