/*
 * $FILE: zynq_uart.h
 *
 * board definitions
 *
 * $VERSION$
 *
 * Author: Manuel Muñoz <mmunoz@ai2.upv.es>
 * Author: Javier O. Coronel <jcoronel@fentiss.com>
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE file for the license terms.
 */
/* Changelog:
 * - [16/06/16:CP-160616-01:#113] Uart Update LookUpTable Values.
 */ 
#ifndef _ZYNQ7000_UART_H_
#define _ZYNQ7000_UART_H_

#ifdef CONFIG_ZYNQ

/************************** Constant Definitions *****************************/
#define ZYNQ_UART0_BASE_ADDRESS 					0xe0000000
#define ZYNQ_UART1_BASE_ADDRESS 					0xe0001000

typedef enum /*XPAR_PS7_UART_X_UART_CLK_FREQ_HZ supported values*/
{
    UARTCLK100000000=0U,
    UARTCLK50000000=1U,
    UARTCLK_NUMBER=2U
}uartClockFreq_t;

#define UART_DEF_FREQ (UARTCLK50000000)

typedef enum /*baudRates_e*/
{
    BR9600=0U,
    BR115200=1U,
    BR230400=2U,
    BR_NUMBER=3U
}baudRate_t;

#define UART_DEF_BR     (BR115200)

typedef struct /*uartLookUpTableBaudRate_s*/
{
    xm_u32_t   baudRate;
    xm_u32_t   BRGRCD;
    xm_u32_t   BDIVCD;
}uartLookUpTableBaudRate_t;

typedef struct /*uartLookUpTableFreq_s*/
{
    xm_u32_t   uartClockFreq;
    uartLookUpTableBaudRate_t baudRates[BR_NUMBER];
}uartLookUpTableFreq_t;

uartLookUpTableFreq_t uartLookUpTable[UARTCLK_NUMBER] =
{
        [UARTCLK50000000]  = {
                .uartClockFreq = 50000000U,
                .baudRates = {
                    [BR9600]    = {.baudRate = 9600U,   .BRGRCD = 651U, .BDIVCD = 7U},
                    [BR115200]  = {.baudRate = 115200U, .BRGRCD = 62U,  .BDIVCD = 6U},
                    [BR230400]  = {.baudRate = 230400U, .BRGRCD = 31U,  .BDIVCD = 6U},
                    }
                },
        [UARTCLK100000000] = {
                .uartClockFreq = 100000000U,
                .baudRates = {
                    [BR9600]    = {.baudRate = 9600U,   .BRGRCD = 1736, .BDIVCD = 5U},
                    [BR115200]  = {.baudRate = 115200U, .BRGRCD = 124U, .BDIVCD = 6U},
                    [BR230400]  = {.baudRate = 230400U, .BRGRCD = 62U,  .BDIVCD = 6U},
                    }
                },
};

/* Bits defined in the Register Channel_sts_reg0 */
#define UART_STS_TXFULL 	BIT(4) /* Transmitter FIFO Full continuous status:
                               0: Tx FIFO is not full
                               1: Tx FIFO is full*/
/* Bits defined in the Register Channel_sts_reg0 */
#define UART_STS_TNFULL 	BIT(14) /* Transmitter FIFO Full continuous status:
                               0: Tx FIFO is not near to full
                               1: Tx FIFO is near to full*/
#define UART_STS_TXEMPTY 	BIT(3) /* Transmitter FIFO Full continuous status:
                               0: Tx FIFO is not empty
                               1: Tx FIFO is empty*/

/*Register Control_reg0 BitMask */
#define ZYNQ_UART_CR_STOPBRK BIT(8)  /* Stop transmitter break */
#define ZYNQ_UART_CR_STTBRK  BIT(7)  /* Start transmitter break */
#define ZYNQ_UART_CR_RSTTO   BIT(6)  /* Restart receiver timeout counter */
#define ZYNQ_UART_CR_TXDIS   BIT(5)  /* Transmit disable */
#define ZYNQ_UART_CR_TXEN    BIT(4)  /* Transmit enable */
#define ZYNQ_UART_CR_RXDIS   BIT(3)  /* Receive disable */
#define ZYNQ_UART_CR_RXEN    BIT(2)  /* Receive enable */
#define ZYNQ_UART_CR_TXRES   BIT(1)  /* Software reset for Tx data path */
#define ZYNQ_UART_CR_RXRES   BIT(0)  /* Software reset for Rx data path */

/*Register Control mode_reg0 BitMask*/
#define ZYNQ_UART_MR_PAR                  /*Parity type select [5:3]*/
#define ZYNQ_UART_MR_PAR_EVEN     0       /* 000: even parity
                                           001: odd parity
                                           010: forced to 0 parity (space)
                                           011: forced to 1 parity (mark)*/
#define ZYNQ_UART_MR_PAR_NONE BIT(5)      /* 1xx: no parity*/
#define ZYNQ_UART_MR_CHRL                 /* Character length select:
                                             11: 6 bits
                                             10: 7 bits*/
#define ZYNQ_UART_MR_CHRL_8               /*   0x: 8 bits*/

#define ZYNQ_UART_MR_CLKS                 /* Clock source select:*/
#define ZYNQ_UART_MR_CLKS_REF_CLK 0       /*  0: clock source is uart_ref_clk*/

#define ZYNQ_UART_control					(0x00) /* UART Control Register def=0x128 */
#define ZYNQ_UART_mode						(0x04)/* UART Mode Register def=0 */
#define ZYNQ_UART_intrpt_en					(0x08)/* Interrupt Enable Register def=0 */
#define ZYNQ_UART_intrpt_dis				(0x0c)/* Interrupt Disable Register def=0 */
#define ZYNQ_UART_intrpt_mask				(0x10)/* Interrupt Mask Register def=0 */
#define ZYNQ_UART_chnl_int_sts				(0x14)/* Channel Interrupt Status Register def=x200 */
#define ZYNQ_UART_baud_rate_gen				(0x18)/* Baud Rate Generator Register def=0x28B */
#define ZYNQ_UART_Rcvr_timeout				(0x1c)/* Receiver Timeout Register def=0 */
#define ZYNQ_UART_Rcvr_FIFO_trigger_level0	(0x20)/* Receiver FIFO Trigger Level Register */
#define ZYNQ_UART_Modem_ctrl				(0x24)/* Modem Control Register def=0 */
#define ZYNQ_UART_Modem_sts					(0x28)/* Modem Status Register */
#define ZYNQ_UART_channel_sts				(0x2c)/* Channel Status Register def=0 */
#define ZYNQ_UART_tx_rx_fifo				(0x30)/* Transmit and Receive FIFO def=0 */
#define ZYNQ_UART_baud_rate_divider			(0x34)/* Baud Rate Divider def=0xf */
#define ZYNQ_UART_Flow_delay				(0x38)/* Flow Control Delay Register  def=0*/
#define ZYNQ_UART_Tx_FIFO_trigger_level		(0x3c)/* Transmitter FIFO Trigger Level Register */

extern xm_s32_t InitUart(void);
#ifdef CONFIG_EARLY_OUTPUT
extern void SetupEarlyOutput(void);
#endif

#endif /*CONFIG_ZYNQ*/

#endif /* _ZYNQ7000_UART_H_ */
