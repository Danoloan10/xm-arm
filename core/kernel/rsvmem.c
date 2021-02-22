/*
 * $FILE: rsvmem.c
 *
 * Memory for structures
 *
 * $VERSION$
 *
 * Author: Miguel Masmano <mmasmano@fentiss.com>
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license terms.
 */

#include <xmconf.h>
#include <assert.h>
#include <processor.h>

void InitRsvMem(void) {
    xm_s32_t e;
    for (e=0; xmcRsvMemTab[e].obj; e++)
    {
    	xmcRsvMemTab[e].usedAlign&=~RSV_MEM_USED;
    }
}

//#define PRINTTHIS
#ifdef PRINTTHIS
#ifdef CONFIG_EARLY_OUTPUT
#   define printhis eprintf
#else
#   define printhis kprintf
#endif /*CONFIG_EARLY_OUTPUT*/

void printRsvMem(xm_u32_t E)
{
    xm_s32_t e,a=0;
    xm_u8_t tag=' ';
    for (e=0; xmcRsvMemTab[e].obj; e++)
    {
        if((xmcRsvMemTab[e].usedAlign&RSV_MEM_USED))
            tag = '-';
        else if((!a)&&(e == E))
        {   tag = '*';a = 1;  }
        else
            tag = ' ';

        printhis("%c%d 0x%08x+%08x %d %u%c\n", tag, e, (xmAddress_t)&xmcTab, xmcRsvMemTab[e].obj, xmcRsvMemTab[e].size, xmcRsvMemTab[e].usedAlign&~RSV_MEM_USED, tag);
    }
}
#endif /*PRINTTHIS*/

void *AllocRsvMem(xm_u32_t size, xm_u32_t align) {
	xm_s32_t e;

#ifdef PRINTTHIS
	printhis("------AllocRsvMem %d %d ", size, align);
#endif /*PRINTTHIS*/
	for (e=0; xmcRsvMemTab[e].obj; e++)
	{
		if (!(xmcRsvMemTab[e].usedAlign&RSV_MEM_USED)&&((xmcRsvMemTab[e].usedAlign&~RSV_MEM_USED)==align)&&(xmcRsvMemTab[e].size==size))
		{
#ifdef PRINTTHIS
		    printhis("------ %d OK\n", e);
			printRsvMem(e);
#endif /*PRINTTHIS*/
			xmcRsvMemTab[e].usedAlign|=RSV_MEM_USED;
			return (void *)((xmAddress_t)xmcRsvMemTab[e].obj+(xmAddress_t)&xmcTab);
		}
	}
#ifdef PRINTTHIS
	printhis("------ ERR\n");
#undef printhis
#undef PRINTTHIS
#endif /*PRINTTHIS*/
	return 0;
}

#ifdef CONFIG_DEBUG
void RsvMemDebug(void) {
    xm_s32_t e;
    for (e=0; xmcRsvMemTab[e].obj; e++)
	if (!(xmcRsvMemTab[e].usedAlign&RSV_MEM_USED))
            PWARN("RsvMem not used %d:%d:0x%x\n", xmcRsvMemTab[e].usedAlign&~RSV_MEM_USED, xmcRsvMemTab[e].size, xmcRsvMemTab[e].obj);
    //SystemPanic("RsvMem not used %d:%d:%x\n", xmcRsvMemTab[e].align,xmcRsvMemTab[e].size, xmcRsvMemTab[e].obj);
}
#endif
