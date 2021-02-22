/*
 * $FILE: uart.h
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
#ifndef _UART_H_
#define _UART_H_

#define BIT(X)      (1<<X)

/************************** Constant Definitions *****************************/
#define ZYNQ_UART0_BASE_ADDRESS         0xe0000000

/*Registers structure*/
typedef struct zynq_uart_regs
{
    volatile unsigned int control;                  /* UART Control Register def=0x128 */
        /* Register Control_reg0 BitMask */
        #define ZYNQ_UART_CR_STOPBRK                BIT(8)  /* Stop transmitter break */
        #define ZYNQ_UART_CR_STTBRK                 BIT(7)  /* Start transmitter break */
        #define ZYNQ_UART_CR_RSTTO                  BIT(6)  /* Restart receiver timeout counter */
        #define ZYNQ_UART_CR_TXDIS                  BIT(5)  /* Transmit disable */
        #define ZYNQ_UART_CR_TXEN                   BIT(4)  /* Transmit enable */
        #define ZYNQ_UART_CR_RXDIS                  BIT(3)  /* Receive disable */
        #define ZYNQ_UART_CR_RXEN                   BIT(2)  /* Receive enable */
        #define ZYNQ_UART_CR_TXRES                  BIT(1)  /* Software reset for Tx data path */
        #define ZYNQ_UART_CR_RXRES                  BIT(0)  /* Software reset for Rx data path */

    volatile unsigned int mode;                     /* UART Mode Register def=0 */
    /*Register Control mode_reg0 BitMask*/
        #define ZYNQ_UART_MODE_CHMODE_NORMAL            (0x0)
        #define ZYNQ_UART_MODE_CHMODE_AUTO_ECHO         (BIT(8))
        #define ZYNQ_UART_MODE_CHMODE_LOCAL_LOOPBACK    (BIT(9))
        #define ZYNQ_UART_MODE_CHMODE_REMOTE_LOOPBACK   (BIT(8)|BIT(9))
        #define ZYNQ_UART_MODE_PAR                  /*Parity type select [5:3]*/
        #define ZYNQ_UART_MODE_PAR_EVEN     0       /* 000: even parity
                                                   001: odd parity
                                                   010: forced to 0 parity (space)
                                                   011: forced to 1 parity (mark)*/
        #define ZYNQ_UART_MODE_PAR_NONE     BIT(5)  /* 1xx: no parity*/
        #define ZYNQ_UART_MODE_CHRL                 /* Character length select:
                                                     11: 6 bits
                                                     10: 7 bits*/
        #define ZYNQ_UART_MODE_CHRL_8               /*   0x: 8 bits*/

        #define ZYNQ_UART_MODE_CLKS                 /* Clock source select:*/
        #define ZYNQ_UART_MODE_CLKS_REF_CLK 0       /*  0: clock source is uart_ref_clk*/
    volatile unsigned int intrpt_en;                    /* Interrupt Enable Register def=0 */
    volatile unsigned int intrpt_dis;               /* Interrupt Disable Register def=0 */
    volatile unsigned int intrpt_mask;              /* Interrupt Mask Register def=0 */
    volatile unsigned int chnl_int_sts;             /* Channel Interrupt Status Register def=x200 */
    volatile unsigned int baud_rate_gen;                /* Baud Rate Generator Register def=0x28B */
        #define ZYNQ_UART_BRGR_CD_115200       62 /*Baud Rate Clock Divisor*/
        #define ZYNQ_UART_BRGR_CD_460800       9 /*Baud Rate Clock Divisor*/
    volatile unsigned int Rcvr_timeout;             /* Receiver Timeout Register def=0 */
    volatile unsigned int Rcvr_FIFO_trigger_level0; /* Receiver FIFO Trigger Level Register */
    volatile unsigned int Modem_ctrl;               /* Modem Control Register def=0 */
    volatile unsigned int Modem_sts;                    /* Modem Status Register */
    volatile unsigned int channel_sts;              /* Channel Status Register def=0 */
        #define UART_STS_TXFULL                 BIT(4) /* Transmitter FIFO Full continuous status: 0=not-full:1=full*/
        #define UART_STS_TXEMPTY                BIT(3) /* Transmitter FIFO Full continuous status: 0=not-empty:1=empty*/
    volatile unsigned int tx_rx_fifo;               /* Transmit and Receive FIFO def=0 */
    volatile unsigned int baud_rate_divider;            /* Baud Rate Divider def=0xf */
        #define ZYNQ_UART_BDIV_CD_115200       6  /*Baud Rate Clock Divisor*/
        #define ZYNQ_UART_BDIV_CD_460800       11  /*Baud Rate Clock Divisor*/
    volatile unsigned int Flow_delay;               /* Flow Control Delay Register  def=0*/
    volatile unsigned int Tx_FIFO_trigger_level;        /* Transmitter FIFO Trigger Level Register */

}uart_t;
#endif

