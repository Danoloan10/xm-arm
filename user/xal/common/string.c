/*
 * 
 * - prssize_tf() based on sprssize_tf() from gcctest9.c Volker Oth
 * - Changes made by Holger Klabunde
 * - Changes made by Martin Thomas for the efsl debug output
 * - Changes made by speiro for lpc2000 devices for PaRTiKle
 * 
 */

/*
 * - [09/06/2021] danielalcaiden: added stddef types
 */


#include <stdarg.h>
#include <stddef.h>
#include <string.h>
#include <xm.h>


ssize_t memcmp(const void *dst, const void *src, size_t count)
{
	ssize_t r;
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

void *memcpy(void* dst, const void* src, size_t count)
{
	register char *d=dst;
	register const char *s=src;
	++count;
	while (--count) {
		*d = *s;
		++d; ++s;
	}
	return dst;
}

/* memmove: xm */

void *memset(void *dst, xm_s32_t s, size_t count)
{
	register char * a = dst;
	count++;
	while (--count)
		*a++ = s;
	return dst;
}

char *strcat(char *s, const char* t)
{
	char *dest=s;
	s+=strlen(s);
	for (;;) {
		if (!(*s=*t)) break; ++s; ++t;
	}
	return dest;
}

ssize_t strcmp(const char *s, const char *t)
{
	char x;
	for (;;) {
		x = *s; if (x != *t) break; if (!x) break; ++s; ++t;
	}
	return ((ssize_t)x)-((ssize_t)*t);
}

char *strcpy(char *dest, const char* src)
{
	xm_s8_t *ret = dest;
	while ((*dest++ = *src++))
		;
	return ret;
}

size_t strlen(const char *s)
{
	size_t i;
	if (!s) return 0;
	for (i = 0; *s; ++s) ++i;
	return i;
}

char *strncat(char *s, const char *t, size_t n)
{
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

ssize_t strncmp(const char *s1, const char *s2, size_t n)
{
	register const unsigned char *a=(const unsigned char*)s1;
	register const unsigned char *b=(const unsigned char*)s2;
	register const unsigned char *fini=a+n;
	while (a < fini) {
		register ssize_t res = *a-*b;
		if (res) return res;
		if (!*a) return 0;
		++a; ++b;
	}
	return 0;
}

size_t strnlen(const char *s, size_t len)
{
	size_t i;
	if (!s) return 0;
	for (i = 0; (i < len); ++s){
		if (*s) ++i;
		else len = i;
	}
	return i;
}

char *strncpy(char *dest, const char *src, size_t n)
{
	xm_s32_t j;

	memset(dest,0,n);

	for (j=0; j<n && src[j]; j++)
		dest[j]=src[j];

	if (j>=n)
		dest[n-1]=0;

	return dest;
}



char *strchr(const char *t, xm_s32_t c)
{
	register char ch;

	ch = c;
	for (;;) {
		if (*t == ch) break; if (!*t) return 0; ++t;
	}
	return (char*)t;
}

/* strrchr: xm */

/* strstr: xm */
