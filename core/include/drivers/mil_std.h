/*
 * $FILE: mil_std.h
 *
 *
 * $VERSION$
 *
 * Author: Javier O. Coronel <jcoronel@fentiss.com>
 *
 * $LICENSE:
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef _XM_DEV_MIL_STD_H_
#define _XM_DEV_MIL_STD_H_

/*Base address MIL-STD-1553BRM to Core1553BRM*/
#define MIL_STD_BASE	0xFFF00000

/*Register on Core1553BRM*/
#define CORE1553BRM_CTRL        0x00
#define CORE1553BRM_OP_STATUS   0x04
#define CORE1553BRM_CMD         0x08
#define CORE1553BRM_IT_MASK     0x0C
#define CORE1553BRM_IT_PEND     0x10
#define CORE1553BRM_IT_PTR      0x14
#define CORE1553BRM_ENH_FEAT    0x80


/*Interrupts on Core1553BRM*/
#define CORE1553BRM_IRQ_DMAF    1<<15  /*HW IRQ*/
#define CORE1553BRM_IRQ_WRAPF   1<<14  /*HW IRQ*/
#define CORE1553BRM_IRQ_TAPF    1<<13  /*HW IRQ*/
#define CORE1553BRM_IRQ_BITF    1<<12  /*HW IRQ*/
#define CORE1553BRM_IRQ_MERR    1<<11  /*MSG IRQ*/
#define CORE1553BRM_IRQ_SUBAD   1<<10  /*MSG IRQ*/
#define CORE1553BRM_IRQ_BDRCV   1<<9   /*MSG IRQ*/
#define CORE1553BRM_IRQ_IXEQ0   1<<8   /*MSG IRQ*/
#define CORE1553BRM_IRQ_ILLCMD  1<<7   /*MSG IRQ*/
#define CORE1553BRM_IRQ_N_A     1<<6
#define CORE1553BRM_IRQ_EOL     1<<5   /*MSG IRQ*/
#define CORE1553BRM_IRQ_ILLOCMD 1<<4   /*MSG IRQ*/
#define CORE1553BRM_IRQ_ILLOP   1<<3   /*MSG IRQ*/
#define CORE1553BRM_IRQ_RTF     1<<2   /*MSG IRQ*/
#define CORE1553BRM_IRQ_CBA     1<<1   /*MSG IRQ*/
#define CORE1553BRM_IRQ_MBC     1<<0   /*MSG IRQ*/


/*Mode Core1554BRM*/
#define CORE1553BRM_MODE_BC        0x00
#define CORE1553BRM_MODE_RT        0x01
#define CORE1553BRM_MODE_BM        0x10
#define CORE1553BRM_MODE_BM_RT     0x11

#define MIL_STD_STATUS_CTRL  0xFFF00100
#define MIL_STD_IT_SETTING   0xFFF00104
#define MIL_STD_AHB_PAGE_ADR 0xFFF00108

#define MIL_STD_IT_SETTING_INTLEVEL  1<<0
#define MIL_STD_IT_SETTING_INTACKH   1<<1
#define MIL_STD_IT_SETTING_INTACKM   1<<2

/*MIL-STD-1553B CLOCK in GRGPREG*/

#define MIL_STD_1553_CLK_SELECT_BIT       1<<5
  #define CLK_SELECT_GEN        0
  #define CLK_SELECT_CLK_INPUT  1
#define MIL_STD_1553_CLK_DIVH_BIT         1<<14
#define MIL_STD_1553_CLK_DIVL_BIT         1<<16
#define MIL_STD_1553_CLK_DIV_OUT_BIT      1<<18
  #define CLK_DIV_OUT_SYS_CLK   1
  #define CLK_DIV_OUT_DIV_CLK   0




#endif /* _XM_DEV_MIL_STD_H_ */


