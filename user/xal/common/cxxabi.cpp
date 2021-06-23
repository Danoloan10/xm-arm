/**
 * ABI support functions for C++
 * from: https://wiki.osdev.org/C++
 * */

#include <xm.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ATEXIT_MAX_FUNCS 128

typedef unsigned uarch_t;

typedef struct
{
	/*
	* Each member is at least 4 bytes large. Such that each entry is 12bytes.
	* 128 * 12 = 1.5KB exact.
	**/
	void (*destructor_func)(void *);
	void *obj_ptr;
	void *dso_handle;
} atexit_func_entry_t;

void __cxa_pure_virtual()
{
	XM_halt_partition(XM_PARTITION_SELF);
}

atexit_func_entry_t __atexit_funcs[ATEXIT_MAX_FUNCS];
uarch_t __atexit_first_free = 0;
 
void *__dso_handle = 0;

void __cxa_init_atexit()
{
	for(int i = 0; i < ATEXIT_MAX_FUNCS; i++)
		__atexit_funcs[i].destructor_func = 0;
}
 
int __cxa_atexit(void (*f)(void *), void *objptr, void *dso)
{
	printf("__cxa_atexit\n");
	if (__atexit_first_free >= ATEXIT_MAX_FUNCS) {return -1;};
	__atexit_funcs[__atexit_first_free].destructor_func = f;
	__atexit_funcs[__atexit_first_free].obj_ptr = objptr;
	__atexit_funcs[__atexit_first_free].dso_handle = dso;
	while(!__atexit_funcs[__atexit_first_free].destructor_func)
		__atexit_first_free++;
	return 0;
}
 
void __cxa_finalize(void *f)
{
	printf("__cxa_finalize\n");
	uarch_t i = __atexit_first_free;
	if (!f)
	{
		while (i--)
		{
			if (__atexit_funcs[i].destructor_func)
			{
				(*__atexit_funcs[i].destructor_func)(__atexit_funcs[i].obj_ptr);
			}
		}
		return;
	}
 
	while (i--)
	{
		if (__atexit_funcs[i].destructor_func == f)
		{
			(*__atexit_funcs[i].destructor_func)(__atexit_funcs[i].obj_ptr);
			__atexit_funcs[i].destructor_func = 0;

		}
		if (__atexit_funcs[i].destructor_func == 0)
		{
			__atexit_first_free = i;
		}
	}
}


#ifdef __cplusplus
}
#endif
