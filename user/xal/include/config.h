/*
 * $FILE: config.h
 *
 * configuration of XAL
 *
 * $VERSION$
 *
 * Author: Miguel Masmano <mmasmano@fentiss.com>
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license terms.
 */

#ifndef _XAL_CONFIG_H_
#define _XAL_CONFIG_H_

#include <autoconf.h>

#define STACK_SIZE (CONFIG_STACK_KB*1024)
#define STACK_SIZE_EXCPT (CONFIG_STACK_EXCPT_KB*1024)

#define BLOCK_BYTE_ORDER CONFIG_BLOCK_BYTE_ORDER
#define BLOCK_BYTE_SIZE  ( 1 << BLOCK_BYTE_ORDER )

#endif
