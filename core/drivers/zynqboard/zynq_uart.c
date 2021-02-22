/*
 * $FILE: zynq_uart.c
 *
 * $VERSION$
 *
 * Author: Manuel Muñoz <mmunoz@ai2.upv.es>
 * Author: Javier O. Coronel <jcoronel@fentiss.com>
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license terms.
 */
/*
 * Changelog:
 * - [18/11/2015:SPR-151118-01:#93] Remove unused CPUFrequency parameter from Uart initialization.
 * - [18/11/2015:SPR-151118-01:#93] Configure Early Uart depending on Kconfig selection.
 * - [16/06/16:CP-160616-01:#113] Add LookUp Table to obtain freq depending on uart clock and baudrate.
 */

#ifdef CONFIG_ZYNQ

#include "drivers/zynqboard/zynq_uart.h"
#include "drivers/zynqboard/zynq_tzpc.h"
#include "arm/io.h"

#include <spinlock.h>
#include <kdevice.h>
#include <processor.h>
//#include <boot.h>
//#include <arch/leon.h>
//#include <objdir.h>
//#include <processor.h>
//#include <queue.h>
//#include <rsvmem.h>

#define DoNop() __asm__ __volatile__ ("nop\n\t" ::)

static kDevice_t uartTab[CONFIG_DEV_NO_UARTS];

static const kDevice_t *GetUart(xm_u32_t subId) {
    if (subId>=CONFIG_DEV_NO_UARTS)
        return 0;
    //if ((subId>=CONFIG_DEV_NO_UARTS)||!xmcTab.deviceTab.uart[subId].baudRate)
    //return 0;
    return &uartTab[subId];
}

#define UART0_IRQ 59
#define UART1_IRQ 82

static struct uartInfo {
    xm_u32_t base;
    xm_s32_t irq;
} uartInfo[]={
    [0]={
    		.base=ZYNQ_UART0_BASE_ADDRESS,
    		.irq=UART0_IRQ,
    },
    [1]={
    		.base=ZYNQ_UART1_BASE_ADDRESS,
    		.irq=UART1_IRQ,
    },
};

static spinLock_t uartSLock=SPINLOCK_INIT;

//#define UART_WAIT_LOOP
#ifdef UART_WAIT_LOOP
static xm_u32_t uartCharCnt=0;
void uart_loop(void)
{
	uartCharCnt++;
	if (uartCharCnt > 80)
	{
		uartCharCnt = 0;
		volatile unsigned int aux = 0x0000FFFF;
		do {
			DoNop();
			aux--;
			DoNop();
			DoNop();
		}while(aux!=0);
	}
}
#endif

static inline void _WriteUart(xm_u32_t base, xm_s32_t a)
{
	/*Make sure that the uart is ready for new char's before continuing*/
	while ((( ReadReg(base, ZYNQ_UART_channel_sts) ) & UART_STS_TXEMPTY) == 0)
//	while ((( ReadReg(base, ZYNQ_UART_channel_sts)) & UART_STS_TXFULL) > 0)
	{	DoNop();	}

	WriteReg(base, ZYNQ_UART_tx_rx_fifo, a ); /* Transmit char */
#ifdef UART_WAIT_LOOP
	uart_loop();
#endif
}

static xm_s32_t WriteUart(const kDevice_t *kDev, xm_u8_t *buffer, xm_s32_t len)
{
	xm_s32_t e;
	SpinLock(&uartSLock);
	for (e = 0; e < len; e++)
	{
		_WriteUart(uartInfo[kDev->subId].base, buffer[e]);
#ifdef CONFIG_UART_ADD_CARRIAGE_RETURN
		if (buffer[e]=='\n')
		{	_WriteUart(uartInfo[kDev->subId].base, '\r');}
#endif
	}
	SpinUnlock(&uartSLock);
	return len;
}

static xm_s32_t __InitUart(xm_u32_t base, xm_u32_t baudrate, xm_u32_t uartClockFreq)
{
	/* Disable the transmitter and receiver before writing to the Baud Rate Generator */
	WriteReg(base, ZYNQ_UART_control, 0);

	uartClockFreq_t clockFreq=-1;
	baudRate_t baudRate=-1;

	xm_u32_t confFound = 0U;

    WriteReg(base, ZYNQ_UART_baud_rate_divider, uartLookUpTable[UART_DEF_FREQ].baudRates[UART_DEF_BR].BDIVCD);
    WriteReg(base, ZYNQ_UART_baud_rate_gen, uartLookUpTable[UART_DEF_FREQ].baudRates[UART_DEF_BR].BRGRCD);

	while((confFound==0)&&(++clockFreq<UARTCLK_NUMBER))
	{
	    if(uartLookUpTable[clockFreq].uartClockFreq != uartClockFreq)
	    {   continue;    }

	    while((confFound==0)&&(++baudRate<BR_NUMBER))
        {
            if(uartLookUpTable[clockFreq].baudRates[baudRate].baudRate != baudrate)
            {   continue;   }
            WriteReg(base, ZYNQ_UART_baud_rate_divider, uartLookUpTable[clockFreq].baudRates[baudRate].BDIVCD);
            WriteReg(base, ZYNQ_UART_baud_rate_gen, uartLookUpTable[clockFreq].baudRates[baudRate].BRGRCD);
            confFound = 1;
        }
	}

	/*Set 8-bit NoParity 1-StopBit*/
	WriteReg(base, ZYNQ_UART_mode, ZYNQ_UART_MR_PAR_NONE);

	/*Enable Rx & Tx*/
	WriteReg(base, ZYNQ_UART_control, ZYNQ_UART_CR_TXEN | ZYNQ_UART_CR_RXEN | ZYNQ_UART_CR_TXRES | ZYNQ_UART_CR_RXRES );

#ifdef CONFIG_FAST_OUTPUT
	CA9_TZ_NONSECURE_UART1();
#endif

	return 0;
}

xm_s32_t InitUart(void)
{
	memset(uartTab, 0, sizeof(kDevice_t) * CONFIG_DEV_NO_UARTS);

#ifdef CONFIG_DEV_UART_0
	uartTab[0].subId = 0;
    uartTab[0].Write = WriteUart;
#if 0
    uartTab[0].Read=ReadUart;
#endif
    if (xmcTab.deviceTab.uart[0].baudRate)
        __InitUart(uartInfo[0].base, xmcTab.deviceTab.uart[0].baudRate, CONFIG_XPAR_PS7_UART_0_UART_CLK_FREQ_HZ);
#endif

#ifdef CONFIG_DEV_UART_1
	uartTab[1].subId = 1;
	uartTab[1].Write = WriteUart;
#if 0
    uartTab[1].Read=ReadUart;
#endif
    if (xmcTab.deviceTab.uart[1].baudRate)
        __InitUart(uartInfo[1].base, xmcTab.deviceTab.uart[1].baudRate, CONFIG_XPAR_PS7_UART_1_UART_CLK_FREQ_HZ);
#endif

    GetKDevTab[XM_DEV_UART_ID]=GetUart;

	return 0;
}

REGISTER_KDEV_SETUP(InitUart);

#ifdef CONFIG_EARLY_UART0
static xm_s32_t dflEarlyUart=0;
#define CONFIG_XPAR_PS7_UART_CLK_FREQ_HZ CONFIG_XPAR_PS7_UART_0_UART_CLK_FREQ_HZ
#elif CONFIG_EARLY_UART1
static xm_s32_t dflEarlyUart=1;
#define CONFIG_XPAR_PS7_UART_CLK_FREQ_HZ CONFIG_XPAR_PS7_UART_1_UART_CLK_FREQ_HZ
#endif

#ifdef CONFIG_EARLY_OUTPUT
static xm_u8_t * early_uart_init_text = "EARLY OUTPUT INITIALIZED!\n\0";
void SetupEarlyOutput(void)
{
	__InitUart(uartInfo[dflEarlyUart].base, CONFIG_EARLY_UART_BAUDRATE, CONFIG_XPAR_PS7_UART_CLK_FREQ_HZ);
	while(*(early_uart_init_text))
	  _WriteUart(uartInfo[dflEarlyUart].base, *(early_uart_init_text++));
}
#endif /*CONFIG_EARLY_OUTPUT*/

#ifdef CONFIG_EARLY_OUTPUT
void EarlyPutChar(xm_u8_t c)
{
	_WriteUart(uartInfo[dflEarlyUart].base, c);
#ifdef CONFIG_UART_ADD_CARRIAGE_RETURN
    if (c=='\n')
        _WriteUart(uartInfo[dflEarlyUart].base, '\r');
#endif
}

#endif /*CONFIG_EARLY_OUTPUT*/

#endif /*CONFIG_ZYNQ*/
