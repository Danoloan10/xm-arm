/*
 * $FILE: devid.h
 *
 * Devices Ids
 *
 * $VERSION$
 *
 * Author: Miguel Masmano <mmasmano@fentiss.com>
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license terms.
 */

#ifndef _XM_DEVID_H_
#define _XM_DEVID_H_

#define XM_DEV_INVALID_ID 0xFFFF

/*TOBEAPPROVED ARMPorting: It could be changed to an enum*/
#define XM_DEV_LOGSTORAGE_ID 0
#define XM_DEV_UART_ID 1
#define XM_DEV_VGA_ID 2
#define XM_DEV_FPGA_ITARFREE_ID 3
#define XM_DEV_SPW_ID 4
#define XM_DEV_MIL_STD_ID 5

#define NO_KDEV 7

//enum xm_dev_enum
//{
//	XM_DEV_LOGSTORAGE_ID/*=0*/,
//	XM_DEV_UART_ID/*=1*/,
//	XM_DEV_VGA_ID/*=2*/,
//	XM_DEV_FPGA_ITARFREE_ID/*=3*/,
//	XM_DEV_SPW_ID/*=4*/,
//	XM_DEV_MIL_STD_ID/*=5*/,
//	NO_KDEV,
//};

#endif
