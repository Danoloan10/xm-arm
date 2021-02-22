/*
 * $FILE: serial.c
 *
 * Generic code to access the serial
 *
 * $VERSION$
 *
 * Author: Miguel Masmano <mmasmano@fentiss.com>
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license terms.
 */
/*
 * Changelog:
 * - [18/11/2015:SPR-151118-01:#93] Configure Uart depending on Kconfig selection
# - [16/06/2016:CP-160616-01:#113] Add Uart clock freq configuration.
 */

#ifdef CONFIG_OUTPUT_ENABLED

#define DoNop() __asm__ __volatile__ ("nop\n\t" ::)

#define BIT(X)		(1<<X)

/************************** Constant Definitions *****************************/
#define ZYNQ_UART0_BASE_ADDRESS 		((unsigned int*)(0xe0000000))
#define ZYNQ_UART1_BASE_ADDRESS 		((unsigned int*)(0xe0001000))

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
#define UART_STS_TXEMPTY 	BIT(3) /* Transmitter FIFO Full continuous status:
                               0: Tx FIFO is not empty
                               1: Tx FIFO is empty*/
/* Bits defined in the Register Channel_sts_reg0 */
#define UART_STS_TNFULL 	BIT(14) /* Transmitter FIFO Full continuous status:
                               0: Tx FIFO is not near to full
                               1: Tx FIFO is near to full*/
#define UART_STS_TEMPTY		(BIT(3))

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

/*Registers structure*/
typedef struct zynq_uart_regs
{
	volatile unsigned int control;					/* UART Control Register def=0x128 */
	volatile unsigned int mode;						/* UART Mode Register def=0 */
	volatile unsigned int intrpt_en;					/* Interrupt Enable Register def=0 */
	volatile unsigned int intrpt_dis;				/* Interrupt Disable Register def=0 */
	volatile unsigned int intrpt_mask;				/* Interrupt Mask Register def=0 */
	volatile unsigned int chnl_int_sts;				/* Channel Interrupt Status Register def=x200 */
	volatile unsigned int baud_rate_gen;				/* Baud Rate Generator Register def=0x28B */
	volatile unsigned int Rcvr_timeout;				/* Receiver Timeout Register def=0 */
	volatile unsigned int Rcvr_FIFO_trigger_level0;	/* Receiver FIFO Trigger Level Register */
	volatile unsigned int Modem_ctrl;				/* Modem Control Register def=0 */
	volatile unsigned int Modem_sts;					/* Modem Status Register */
	volatile unsigned int channel_sts;				/* Channel Status Register def=0 */
	volatile unsigned int tx_rx_fifo;				/* Transmit and Receive FIFO def=0 */
	volatile unsigned int baud_rate_divider;			/* Baud Rate Divider def=0xf */
	volatile unsigned int Flow_delay;				/* Flow Control Delay Register  def=0*/
	volatile unsigned int Tx_FIFO_trigger_level;		/* Transmitter FIFO Trigger Level Register */

}uart_t;


#ifdef CONFIG_UART0
uart_t * uart = ((uart_t*)(ZYNQ_UART0_BASE_ADDRESS));
#define CONFIG_XPAR_PS7_UART_CLK_FREQ_HZ CONFIG_XPAR_PS7_UART_0_UART_CLK_FREQ_HZ
#else
#	ifdef CONFIG_UART1
uart_t * uart = ((uart_t*)(ZYNQ_UART1_BASE_ADDRESS));
#define CONFIG_XPAR_PS7_UART_CLK_FREQ_HZ CONFIG_XPAR_PS7_UART_1_UART_CLK_FREQ_HZ
#	endif
#endif

void InitOutput(void)
{
	/* Disable the transmitter and receiver before writing to the Baud Rate Generator */
	uart->control = 0;

	/* Set Baudrate to 115,200 Baud */
    uartClockFreq_t clockFreq=-1;
    baudRate_t baudRate=-1;

    xm_u32_t confFound = 0U;

    uart->baud_rate_divider = uartLookUpTable[UART_DEF_FREQ].baudRates[UART_DEF_BR].BDIVCD;
    uart->baud_rate_gen = uartLookUpTable[UART_DEF_FREQ].baudRates[UART_DEF_BR].BRGRCD;

    while((confFound==0)&&(++clockFreq<UARTCLK_NUMBER))
    {
        if(uartLookUpTable[clockFreq].uartClockFreq != CONFIG_XPAR_PS7_UART_CLK_FREQ_HZ)
        {   continue;    }

        while((confFound==0)&&(++baudRate<BR_NUMBER))
        {
            if(uartLookUpTable[clockFreq].baudRates[baudRate].baudRate != CONFIG_UART_BAUDRATE)
            {   continue;   }
            uart->baud_rate_divider = uartLookUpTable[clockFreq].baudRates[baudRate].BDIVCD;
            uart->baud_rate_gen = uartLookUpTable[clockFreq].baudRates[baudRate].BRGRCD;
            confFound = 1;
        }
    }


	/*Set 8-bit NoParity 1-StopBit*/
	uart->mode = ZYNQ_UART_MR_PAR_NONE;

	/*Enable Rx & Tx*/
	uart->control = (ZYNQ_UART_CR_TXEN | ZYNQ_UART_CR_RXDIS | ZYNQ_UART_CR_TXRES);
}
void xputchar(int c)
{
		/*Make sure that the uart is ready for new char's before continuing*/
		/*while ( ((uart1->channel_sts) & UART_STS_TXFULL) > 0)*/
		while ( ((uart->channel_sts) & UART_STS_TXEMPTY) == 0)
		{	DoNop(); }

		uart->tx_rx_fifo = ((unsigned char)(c)); /* Transmit char */
#ifdef CONFIG_UART_ADD_CARRIAGE_RETURN
		if (((unsigned char)(c))=='\n')
			xputchar('\r');
#endif
}
#else

void InitOutput(void){};
void xputchar(int c){};

#endif
