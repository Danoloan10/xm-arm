/*
 * $FILE: uart.c
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
#include "uart.h"

#define PRINT(...) do { \
        printf("[P%d] ", XM_PARTITION_SELF); \
        printf(__VA_ARGS__); \
} while (0)


void InitUart(uart_t * _uart)
{
    /* Disable the transmitter and receiver before writing to the Baud Rate Generator */
    _uart->control = 0;

    /* Set Baudrate to 115,200 Baud */
    _uart->baud_rate_divider =  ZYNQ_UART_BDIV_CD_115200;
    _uart->baud_rate_gen = ZYNQ_UART_BRGR_CD_115200;

    /*Set 8-bit NoParity 1-StopBit*/
    _uart->mode = ZYNQ_UART_MODE_PAR_NONE|ZYNQ_UART_MODE_CHMODE_LOCAL_LOOPBACK;

    _uart->intrpt_en = 0xfff;

    /*Enable Rx & Tx*/
    _uart->control = (ZYNQ_UART_CR_TXEN | ZYNQ_UART_CR_RXEN | ZYNQ_UART_CR_TXRES | ZYNQ_UART_CR_RXRES);
}
void printUART(uart_t * _uart)
{
    PRINT("UART 0x%x\n", _uart);
    PRINT("    control    0x%x\n",  _uart->control);                  /* UART Control Register def=0x128 */
    PRINT("    mode       0x%x\n",  _uart->mode);                     /* UART Mode Register def=0 */
    PRINT("    intE       0x%x\n",  _uart->intrpt_en);                /* Interrupt Enable Register def=0 */
    PRINT("    intDis     0x%x\n",  _uart->intrpt_dis);               /* Interrupt Disable Register def=0 */
    PRINT("    intMask    0x%x\n",  _uart->intrpt_mask);              /* Interrupt Mask Register def=0 */
    PRINT("    ChInt      0x%x\n",  _uart->chnl_int_sts);             /* Channel Interrupt Status Register def=x200 */
    PRINT("    BaudTG     0x%x\n",  _uart->baud_rate_gen);            /* Baud Rate Generator Register def=0x28B */
    PRINT("    RecivT     0x%x\n",  _uart->Rcvr_timeout);             /* Receiver Timeout Register def=0 */
    PRINT("    RxFIFO     0x%x\n",  _uart->Rcvr_FIFO_trigger_level0); /* Receiver FIFO Trigger Level Register */
    PRINT("    ModemCt    0x%x\n",  _uart->Modem_ctrl);               /* Modem Control Register def=0 */
    PRINT("    ModemSt    0x%x\n",  _uart->Modem_sts);                /* Modem Status Register */
    PRINT("    Channel    0x%x\n",  _uart->channel_sts);              /* Channel Status Register def=0 */
    PRINT("    TxFIFO     0x%x\n",  _uart->tx_rx_fifo);               /* Transmit and Receive FIFO def=0 */
    PRINT("    BRDv       0x%x\n",  _uart->baud_rate_divider);        /* Baud Rate Divider def=0xf */
    PRINT("    FlowCtr    0x%x\n",  _uart->Flow_delay);               /* Flow Control Delay Register  def=0*/
    PRINT("    TxFIFOT    0x%x\n",  _uart->Tx_FIFO_trigger_level);    /* Transmitter FIFO Trigger Level Register */
}

void UAputChar(uart_t * _uart, xm_u8_t c)
{
    while(((_uart->channel_sts) & UART_STS_TXEMPTY) == 0)
        __asm__ __volatile__("nop\n\r":::);

    _uart->tx_rx_fifo = c;
}


void PartitionMain(void)
{
    PRINT("--------------------------------------\n");
    PRINT("--example.007 - UART Example Start!---\n");

    uart_t * uart0 = ((uart_t*)(ZYNQ_UART0_BASE_ADDRESS));

    PRINT("\n");
    PRINT("  * Initialize UART0\n");
    InitUart(uart0);

    PRINT("\n");
    PRINT("  * Print UART0 registers\n");
    printUART(uart0);

    xm_u8_t c = '?';
    PRINT("\n");
    PRINT("  * Send the character \'%c\' through UART0\n", c);
    UAputChar(uart0, c);

    PRINT("\n");
    PRINT("  * End Example UART0. GOING HALT\n");

    XM_halt_partition(XM_PARTITION_SELF);
}
