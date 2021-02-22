/*
 * $FILE: partition.c
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

#include <string.h>
#include <stdio.h>
#include <xm.h>
#include <irqs.h>
#include "i2c.h"

#define PRINT(...) do { \
        printf("[P%d] ", XM_PARTITION_SELF); \
        printf(__VA_ARGS__); \
} while (0)

void printI2C(i2cDev_t * _i2c)
{
    PRINT("i2c 0x%x\n", _i2c);
    PRINT("    I2C_CR        = 0x%x\n",  _i2c->I2C_CR);
    PRINT("    I2C_SR        = 0x%x\n",  _i2c->I2C_SR);
    PRINT("    I2C_ADDR      = 0x%x\n",  _i2c->I2C_ADDR);
    PRINT("    I2C_DATA      = 0x%x\n",  _i2c->I2C_DATA);
    PRINT("    I2C_ISR       = 0x%x\n",  _i2c->I2C_ISR);
    PRINT("    I2C_XFER_SIZE = 0x%x\n",  _i2c->I2C_XFER_SIZE);
    PRINT("    I2C_SLV_PAUSE = 0x%x\n",  _i2c->I2C_SLV_PAUSE);
    PRINT("    I2C_TIME_OUT  = 0x%x\n",  _i2c->I2C_TIME_OUT);
    PRINT("    I2C_IMR       = 0x%x\n",  _i2c->I2C_IMR);
    PRINT("    I2C_IER       = 0x%x\n",  _i2c->I2C_IER);
    PRINT("    I2C_IDR       = 0x%x\n",  _i2c->I2C_IDR);
}

void PartitionMain(void)
{
    PRINT("------------------------\n");
    PRINT("---I2C Example 008 Start!---\n");

    PRINT("\n");
    PRINT("  * Accessing I2C0(0x%x) at 0x%x\n", i2c0_phy, i2c0_phy);
    printI2C(i2c0_phy);

    PRINT("\n");
    PRINT("  * Accessing I2C1(0x%x) at 0x%x\n", i2c1_phy, i2c1_phy);
    printI2C(i2c1_phy);

    PRINT("  * End Example I2C. GOING HALT\n");

        XM_halt_partition(XM_PARTITION_SELF);
}
