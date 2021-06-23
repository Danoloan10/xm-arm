/*
 * $FILE: assert.h
 *
 * Assert definition
 *
 * $VERSION$
 * 
 * Author: Miguel Masmano <mmasmano@fentiss.com>
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license terms.
 */

#ifndef _XAL_ASSERT_H_
#define _XAL_ASSERT_H_

#ifdef _DEBUG_

#include <xm.h>
#include <stdio.h>

#define assert(exp) do { \
    if(!(exp)) { \
		printf(__FILE__":%u: failed assertion `"#exp"'\n", __LINE__); \
		XM_halt_partition(XM_PARTITION_SELF); \
	} \
} while (0);

#define ASSERT(exp) assert(exp)

#else

#define assert(exp) ((void)0)

#endif

#endif
