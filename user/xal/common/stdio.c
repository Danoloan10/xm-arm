/*
 * $FILE: stdio.c
 *
 * Standard buffered input/output
 *
 * $VERSION$
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license.terms.
 */
/*
 * - Sept 2012 speiro: Minimal printf() initial implementation
 * - Dec 2012  speiro: simplify vsprintf() with fmtflags
 * - Dec 2012  speiro: added vsnprintf() with strings size checks
 * - Nov 2014  jacopa: Include itoa64 and convert function itoa to 32bits. Modify vsnprintf: take into account offsets due to 64 bits variables.
 * - Ago 2016  jacopa/mmasmano: add %lld support for 64bits variables.
 * - [29/03/2017] yvaliente  XM-ARM-2:CP-170329-01: Add support for print float %[.d]f.
 * - [20/06/2017] yvaliente  XM-ARM-2:CP-170329-01: Reset the accuracy between each tag.
 */

#include <stdarg.h>
#include <stddef.h>
#include <string.h>
#include <xm.h>

#define ABS(v) ((v < 0) ? -v: v)

int putchar(int c) {
#if 0
    char d=c;
    XM_write_console(&d, 1);
#else
    static char buff[512];
    static int i=0;
    if (i > (sizeof(buff)-2)) {
        XM_write_console(&buff[0], i);
        i=0;
    }
    buff[i++]=c;
  
    if (c == '\n' || c == '\r') {
        if (i)
            XM_write_console(&buff[0], i);
        i=0;
    }
  
#endif
    return c;
}

static void uartputs(char *s, int n)
{
    int i;

    for(i=0; i<n; i++)
        putchar(s[i]);
}

static xm_u64_t DivMod64(xm_u64_t numer, xm_u64_t denom, xm_u64_t *remain)
{
    xm_u64_t quotient = 0, quotbit = 1;

    if (denom == 0) {
        return 0;
    }

    while (((xm_s64_t) denom >= 0) && (denom < numer)) {
        denom <<= 1;
        quotbit <<= 1;
    }

    while (quotbit && (numer != 0)) {
        if (denom <= numer) {
            numer -= denom;
            quotient += quotbit;
        }
        denom >>= 1;
        quotbit >>= 1;
    }
    
    if (remain)
        *remain = numer;
    
    return quotient;
}

#define SCRATCH 32
static __attribute__ ((noinline)) xm_s32_t itoa(xm_u32_t uval, char a[SCRATCH], xm_u64_t base)
{
	xm_u32_t n=SCRATCH-1;
        xm_u64_t i = 0;
	char hex[16] = "0123456789abcdef";

	if(base < 0 || base > sizeof(hex))
		return n;
	a[n] = '\0';
	do { 
            uval = DivMod64(uval, base, &i);
            //i = uval % base;
            //uval /= base;
            a[--n] = hex[i];
	} while(uval);
	return n;
}

static __attribute__ ((noinline)) xm_s32_t itoa64(xm_u64_t uval, char a[SCRATCH], xm_u64_t base)
{
	xm_u32_t n=SCRATCH-1;
        xm_u64_t i = 0;
	char hex[16] = "0123456789abcdef";

	if(base < 0 || base > sizeof(hex))
		return n;
	a[n] = '\0';
	do {
            //i = uval % base;
            //uval /= base;
            uval = DivMod64(uval, base, &i);
            a[--n] = hex[i];
	} while(uval);
	return n;
}

static const double round_nums[8] =
{ 0.5, 0.05, 0.005, 0.0005, 0.00005, 0.000005, 0.0000005, 0.00000005 };

int dbl2stri(char *s, double dbl, unsigned dec_digits, int always_decimal)
{
	char *output = s;
	int size = 0;

 	//*******************************************
 	//  extract negative info
 	//*******************************************
 	if (dbl < 0.0)
 	{
 		*output++ = '-';
 	    dbl *= -1.0;
 		size++;
 	}

 	//  handling rounding by adding .5LSB to the floating-point data
 	if (dec_digits < 8)
 	{
 		dbl += round_nums[dec_digits];
 	}

 	//**************************************************************************
 	//  construct fractional multiplier for specified number of digits.
 	//**************************************************************************
 	unsigned int mult = 1;
 	unsigned int idx;
 	for (idx = 0; idx < dec_digits; idx++)
 		mult *= 10;

 	unsigned int wholeNum = (unsigned int) dbl;
 	unsigned int decimalNum = (unsigned int) ((dbl - wholeNum) * mult);

 	/*******************************************
 	 *  convert integer portion
 	 *******************************************/
 	char tbfr[40];
 	idx = 0;
 	while (wholeNum != 0)
 	{
 		tbfr[idx++] = '0' + (wholeNum % 10);
 		wholeNum /= 10;
 	}
 	// printf("%.3f: whole=%s, dec=%d\n", dbl, tbfr, decimalNum) ;
 	if (idx == 0)
 	{
 		*output++ = '0';
 		size++;
 	}
 	else
 	{
 		while (idx > 0)
 		{
 			*output++ = tbfr[idx - 1];  //lint !e771
 	 		size++;
 			idx--;
 		}
 	}
 	if (dec_digits > 0 || (always_decimal == 1))
 	{
 		*output++ = '.';
 		size++;

 		//*******************************************
 	    //  convert fractional portion
 	    //*******************************************
 		idx = 0;
 		while (decimalNum != 0)
 		{
 			tbfr[idx++] = '0' + (decimalNum % 10);
 			decimalNum /= 10;
 		}
 		/*  pad the decimal portion with 0s as necessary; */
 		/*  We wouldn't want to report 3.093 as 3.93, would we?? */
 		while (idx < dec_digits)
 		{
 			tbfr[idx++] = '0';
 		}
 		// printf("decimal=%s\n", tbfr) ;
 	    if (idx == 0)
 	    {
 	    	*output++ = '0';
 	 		size++;
 	    }
 	    else
 	    {
 	    	while (idx > 0)
 	        {
 	    		*output++ = tbfr[idx - 1];
 	    		size++;
 	    		idx--;
 	        }
 	    }
 	}
 	return size;
}

#define FLAG_DOFMT 		(1<<0)
#define FLAG_ISLONG		(1<<1)
#define FLAG_ISVLONG	(1<<2)
#define FLAG_PRECISION	(1<<3)

#define DEFAULT_PRECISION 6			/* Precision is 6 by default */

#define CHECKSTR(strStart, strPtr, strMaxLen) ((strPtr - strStart) >= strMaxLen)

ssize_t vsnprintf(char *s, xm_s32_t nc, const char * __restrict fmt, va_list ap)
{
	const char *p;
	char *bs, *sval;
	char fval[128];
	char buf[SCRATCH];
	xm_u32_t n, fmtflag, base;
    xm_s32_t nParam=0;
    int flen;

    unsigned int precision = DEFAULT_PRECISION;
    double dbl = 0;

	bs = s;
	for (p = fmt; *p; p++) {
		if (CHECKSTR(bs, s, nc))
			break;

		fmtflag = 0;
		if (*p == '%'){
			fmtflag |= FLAG_DOFMT;
            nParam++;

			/* Obtain precision, or padding for integer specifiers */
    		if (*++p == '.'){
    			precision = 0;
				p++;			 /* Get char after precision char */
    			while (*p >= '0' && *p <= '9')
    			{
    			    precision = 10 * precision + (*p - '0');
    				p++;
    			}
    			/* The precision has been limited to 9 digits */
    			if (precision > 9)
    			{
    				precision = 9;
    			}
    		} else {
    			precision = DEFAULT_PRECISION;
    		}
			p--;
		} else
			*s++ = *p;

		while(fmtflag & FLAG_DOFMT)
		switch (*++p) {
		case 'l': {
			fmtflag |= (fmtflag & FLAG_ISLONG)? FLAG_ISVLONG: FLAG_ISLONG;
			break;
		}
		case 'u':
		case 'd':
		case 'x': {
			base=10;
			if (*p == 'x')
				base=16;
			if (*p == 'd'){
                                if (fmtflag & FLAG_ISVLONG){
                                   xm_s64_t llval;
                                   xm_u32_t ivalh,ivall;//,neg=0;
                                   if (nParam==1)
                                      ivalh = va_arg(ap, xm_u32_t); /*Remove the first parameter dirty*/
#ifdef CONFIG_TARGET_BIG_ENDIAN
                                   ivalh = va_arg(ap, xm_u32_t);
                                   ivall = va_arg(ap, xm_u32_t);
#else
                                   ivall = va_arg(ap, xm_u32_t);
                                   ivalh = va_arg(ap, xm_u32_t);
#endif
                                   llval=(xm_s64_t)((((xm_u64_t)ivalh)<<32)|(xm_u64_t)ivall);
/*				   if (llval < 0){
                                      neg=1;
                                      llval*=-1;
                                   }*/
				   n = itoa64(ABS(llval), buf, base);
				   if (llval < 0)
                                      buf[--n] = '-';
                                }
                                else{
                                   xm_s32_t ival;
				   ival = va_arg(ap, xm_s32_t);
				   n = itoa(ABS(ival), buf, base);
				   if (ival < 0) buf[--n] = '-';
                                }
			} else {
                                if (fmtflag & FLAG_ISVLONG){
	                           xm_u64_t ullval;
                                   xm_u32_t uvalh,uvall;
                                   if (nParam==1)
                                      uvalh = va_arg(ap, xm_u32_t); /*Remove the first parameter dirty*/
#ifdef CONFIG_TARGET_BIG_ENDIAN
                                   uvalh = va_arg(ap, xm_u32_t);
                                   uvall = va_arg(ap, xm_u32_t);
#else
                                   uvall = va_arg(ap, xm_u32_t);
                                   uvalh = va_arg(ap, xm_u32_t);
#endif
                                   ullval=(((xm_u64_t)uvalh)<<32)|(xm_u64_t)uvall;
				   n = itoa64(ullval, buf, base);
                                }
                                else{
	                           xm_u32_t uval;
				   uval = va_arg(ap, xm_u32_t);
				   n = itoa(uval, buf,base);
                                }
			}
			if (CHECKSTR(bs, s+strlen(&buf[n]), nc))
				break;
			s = strcpy(s, &buf[n]) + (sizeof(buf)-1 - n);
			fmtflag = 0;
			break;
				  }
		case 's': {
			sval = va_arg(ap, char *);
			if (CHECKSTR(bs, s+strlen(sval), nc))
				break;
			s = strcpy(s, sval) + strlen(sval);
			fmtflag = 0;
			break;
		}
		case 'f':
			dbl = (double) va_arg(ap, double);
			flen = dbl2stri(fval, dbl, precision, 0);
			if (CHECKSTR(bs, s+flen, nc))
				break;
			s = strcpy(s, fval) + flen;
			fmtflag = 0;
		 	break;
		default:
			*s++ = *p;
			break;
		}
	}

	return (s - bs);
}

ssize_t vsprintf(char *s, const char *fmt, va_list ap)
{
	ssize_t n = 0;

	n = vsnprintf(s, 1024, fmt, ap);

	return n;
}

ssize_t vprintf(const char * __restrict fmt, va_list ap)
{
	ssize_t n = 0;
	static char str[1024];

	n = vsnprintf(str, sizeof(str), fmt, ap);
	uartputs(str, n);

	return n;
}

ssize_t snprintf(char *s, xm_s32_t nc, const char *fmt, ...)
{
	ssize_t n = 0;
	va_list ap;

	memset(s, 0, nc);
	va_start(ap, fmt);
	n = vsnprintf(s, nc, fmt, ap);
	va_end(ap);

	return n;
}

ssize_t sprintf(char *str, const char *fmt, ...)
{
	ssize_t n = 0;
	va_list ap;

	va_start(ap, fmt);
	n = vsprintf(str, fmt, ap);
	va_end(ap);

	return n;
}

extern ssize_t printf(const char * __restrict fmt, ...) __attribute__ ((format (gnu_printf, 1, 2)));
ssize_t printf(const char * __restrict fmt, ...)
{
	va_list ap;
	ssize_t n = 0;

	va_start(ap, fmt);
	n = vprintf(fmt, ap);
	va_end(ap);

	return n;
}

