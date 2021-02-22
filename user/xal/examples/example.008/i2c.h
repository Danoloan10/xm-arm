/*
 * $FILE: i2c.h
 *
 * Fent Innovative Software Solutions
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
#ifndef _I2C_H_
#define _I2C_H_
/*Registers structure*/
typedef struct zynq_i2c_regs
{
    volatile unsigned int I2C_CR;        /* Control Register, RW */
    volatile unsigned int I2C_SR;        /* Status Register, RO */
    volatile unsigned int I2C_ADDR;      /* I2C Address Register, RW */
    volatile unsigned int I2C_DATA;      /* I2C Data Register, RW */
    volatile unsigned int I2C_ISR;       /* Interrupt Status Register, RW */
    volatile unsigned int I2C_XFER_SIZE; /* Transfer Size Register, RW */
    volatile unsigned int I2C_SLV_PAUSE; /* Slave monitor pause Register, RW */
    volatile unsigned int I2C_TIME_OUT;  /* Time Out Register, RW */
    volatile unsigned int I2C_IMR;       /* Interrupt Mask Register, RO */
    volatile unsigned int I2C_IER;       /* Interrupt Enable Register, WO */
    volatile unsigned int I2C_IDR;       /* Interrupt Disable Register, WO */
}i2cDev_t;

i2cDev_t * i2c0_phy  = ((i2cDev_t *)(0xE0004000));
i2cDev_t * i2c1_phy  = ((i2cDev_t *)(0xE0005000));

#endif
