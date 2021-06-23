// Just call the standard __cxx functions

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif


extern int __cxa_atexit(void (*f)(void *), void *objptr, void *dso);
extern void __cxa_finalize(void *f);
extern void __cxa_init_atexit();

extern void (**__init_array_start)();
extern void (**__init_array_end)();

//extern void (**__fini_array_start)();
//extern void (**__fini_array_end)();


void _init()
{
	__cxa_init_atexit();

	printf("hola %d\n", __init_array_end - __init_array_start);
 	for (void (**p)() = __init_array_start; p < __init_array_end; ++p)
		(*p)();
}

//void _fini()
//{
//	printf("adios %d\n", __fini_array_end - __fini_array_start);
// 	for (void (**p)() = __fini_array_start; p < __fini_array_end; ++p)
//		(*p)();
//}
//

int __aeabi_atexit(void (*f)(void *), void *objptr, void *dso)
{
	return __cxa_atexit(f, objptr, dso);
}
 
void __aeabi_finalize(void *f)
{
	__cxa_finalize(f);
}

#ifdef __cplusplus
}
#endif
