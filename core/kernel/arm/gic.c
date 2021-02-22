/*
 * $FILE: gic.c
 *
 * board definitions
 *
 * $VERSION$ - 13/08/2014
 *
 * Author: Manuel Muñoz <mmunoz@ai2.upv.es>
 * Author: Javier O. Coronel <jcoronel@fentiss.com>
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE file for the license terms.
 */

#include <assert.h>
//#include <kdevice.h>
#include <irqs.h>
//#include <processor.h>
//#include <arch/processor.h>
#include <arch/xm_def.h>
#include <arm/gic.h>
#include <local.h>

#ifdef CONFIG_ARM
//#define BANKED_REG_SUPPORT
#ifdef BANKED_REG_SUPPORT
#define BANKEDREG_APPLY(X)	\
    CA9_SCR_SECURE();		\
    X;						\
    CA9_SCR_NONSECURE();	\
    X;						\
    CA9_SCR_SECURE()
#endif

void HwIrqGetMask(xm_u32_t *mask)
{
    xm_s32_t e;
    for (e=0; e<HWIRQS_VECTOR_SIZE; e++)
        mask[e]=(~(ReadReg(ICD_BASE_ADDRESS, ICDISERn+(e*4))));
}
void HwIrqSetMask(xm_u32_t *mask)
{
    xm_s32_t e;
    for (e=0; e<HWIRQS_VECTOR_SIZE; e++)
        WriteReg(ICD_BASE_ADDRESS, ICDISERn+(e*4),(~(mask[e])));
}
void HwIrqGetPendMask(xm_u32_t *mask)
{
    xm_s32_t e;
    for (e=0; e<HWIRQS_VECTOR_SIZE; e++)
    {
        mask[e]=(~(ReadReg(ICD_BASE_ADDRESS, ICDISPRn+(e*4))));
//        kprintf("Pending %x\n", mask[e]);
    }
}
void HwIrqSetPendMask(xm_u32_t *mask)
{
    xm_s32_t e;
    for (e=0; e<HWIRQS_VECTOR_SIZE; e++)
    {
        WriteReg(ICD_BASE_ADDRESS, ICDICPRn+(e*4),(0xFFFFFFFF));
        WriteReg(ICD_BASE_ADDRESS, ICDISPRn+(e*4),(~(mask[e])));
    }
}
void HwIrqClearPend(void)
{
    xm_s32_t e;
    for (e=0; e<HWIRQS_VECTOR_SIZE; e++)
    {
        WriteReg(ICD_BASE_ADDRESS, ICDICPRn+(e*4),(0xFFFFFFFF));
    }
}
void HwAckIrq(xm_u32_t irq)
{
	HwEndIrq(irq);
	//return ReadReg(ICC_BASE_ADDRESS, ICCIAR);
}
void HwEndIrq(xm_u32_t irq)
{
	WriteReg(ICC_BASE_ADDRESS, ICCEOIR, irq);
}
void HwForceIrq(xm_u32_t irq)
{
	ICDISPR_SETPENDING(irq);
}
void HwClearIrq(xm_u32_t irq)
{
	ICDICPR_CLEARPENDING(irq);
}
#define SGI_NONSEC_PR	0x0
#define SGI_SEC_PR		0x0
#define SPI_NONSEC_PR	0x0
#define SPI_SEC_PR		0x0
#define PPI_NONSEC_PR	0x0
#define PPI_SEC_PR		0x0

static void setIrqSecState(unsigned int irqID, unsigned int irqSecurityLevel)
{
	/*2.a*//*Interrupt Security Registers*/
	if (irqSecurityLevel == ICDISD_NONSEC)	//Secure=0 Non-Secure=1
	{	ICDISR_NONSECURE(irqID);	}
	else	//Secure=0 Non-Secure=1
	{	ICDISR_SECURE(irqID);		}
}

static void setIrqSetPriority(unsigned int irqID, unsigned int priority, unsigned int irqSecurityLevel, unsigned int currentSecurityState)
{
	ICDIPR_SETPRIO(irqID, 0);
	return;
	/*2.3*//*Interrupt Priority Registers 0=Highest 0xFF= Lowest*/
	if (currentSecurityState)
	{ kprintf("Non-Secure %d", irqID);
//		if(securityState == ICDISD_NONSEC)		//Secure=0 Non-Secure=1
//		 {	ICDIPR_SETPRIO(id, priority);	}
//		else	//Secure=0 Non-Secure=1
//		 {	ICDIPR_SETPRIO(id, priority);	}
	}
	else
	{ kprintf("Secure %d", irqID);
		if (irqSecurityLevel == ICDISD_NONSEC)	//Secure=0 Non-Secure=1
		{	ICDIPR_SETPRIO(irqID, priority);	}
		else	//Secure=0 Non-Secure=1
		{	ICDIPR_SETPRIO(irqID, priority);		}
	}
	kprintf(" x%x\n", (ReadReg(ICD_BASE_ADDRESS, ICDIPR_ADDRESS(irqID))	& (ICDIPR_BITMASK(irqID, 0xFF))));
}

static void addSGIFromID(unsigned int irqID, unsigned int irqSecurityLevel, unsigned int currentSecurityState)
{
	if(!((irqID>=GIC_SGI_FIRST_ID)&&(irqID<=GIC_SGI_LAST_ID)))
		return;

	/**** DISTRIBUtOR CONFIGURATION ****/
	/**** 0-. DISABLE GIC *****/
	ICDDCR_DISABLE();
	/**** 1-. SPIs ****/
	/**** 2-. PPIs ****/
	/*2.a*//*Interrupt Security Registers*/
	setIrqSecState(irqID, irqSecurityLevel);

	/*2.b*//*Set Interrupt configuration register select level-sensitive or edge-triggered*/

	/*2.3*//*Interrupt Priority Registers 0=Highest 0xFF= Lowest*/
	setIrqSetPriority(irqID, SGI_SEC_PR, irqSecurityLevel, currentSecurityState);
	/*2.d*//*Specify the target processor list for each interrupt*/
//	ICDIPTR_SET_TARGETCPU(id,0x3);

	/*2.4*//*Enable the interrupt set enable register*/
	ICDISER_ENABLE(irqID);

	/**** 3-. CPU INTERFACE REGISTERS *****/
	/*3.a*//*Configure in CPU interface register map*/
	/*3.b*/
	/*3.c*//*Enable the Processor Interface - Enable the GIC - from asm_config*/
	/**** 4-. ENABLE GIC *****/
	ICDDCR_ENABLE();
}
static void addSPIFromID(unsigned int irqID, unsigned int irqSecurityLevel, unsigned int currentSecurityState)
{
	if(!((irqID>=GIC_SPI_FIRST_ID)&&(irqID<=GIC_SPI_LAST_ID)))
		return;

	/**** DISTRIBUtOR CONFIGURATION ****/
	/**** 0-. DISABLE GIC *****/
	ICDDCR_DISABLE();
	/**** 1-. SPIs ****/
	/*1.a*//*Interrupt Security Registers*/
	setIrqSecState(irqID, irqSecurityLevel);

	/*2.b*//*Set Interrupt configuration register select level-sensitive or edge-triggered*//*TODO Improvement: Add parameter for this*/
//	ICDICFR_EDGE(id);	//WriteReg(ICD_BASE_ADDRESS, ICDICFR_ADDRESS(27), ICDICFR_BITMASK(27));

	/*2.c*//*Interrupt Priority Registers 0=Highest 0xFF= Lowest*/
	setIrqSetPriority(irqID, SGI_SEC_PR, irqSecurityLevel, currentSecurityState);

	/*2.d*//*Specify the target processor list for each interrupt*/
//	ICDIPTR_SET_TARGETCPU(irqID,0x1);
//	WriteReg(ICD_BASE_ADDRESS, ICDIPTR_ADDRESS(id), ICDIPTR_BITMASK(id, 0));

	/*2.e*//*Enable the interrupt set enable register*/
	ICDISER_ENABLE(irqID);	//ICD->REG_ICDISERn[0] = BIT(27);

	/**** 2-. PPIs ****/

	/**** 3-. CPU INTERFACE REGISTERS *****/
	/*3.a*//*Configure in CPU interface register map*/
	/*3.b*/
	/*3.c*//*Enable the Processor Interface - Enable the GIC - from asm_config*/
	/**** 4-. ENABLE GIC *****/
	ICDDCR_ENABLE();
}
static void addPPIFromID(unsigned int irqID, unsigned int irqSecurityLevel, unsigned int currentSecurityState )
{
	if(!((irqID>=GIC_PPI_FIRST_ID)&&(irqID<=GIC_PPI_LAST_ID)))
		return;
	/**** DISTRIBUtOR CONFIGURATION ****/
	/**** 0-. DISABLE GIC *****/
	ICDDCR_DISABLE();
	/**** 1-. SPIs ****/
	/**** 2-. PPIs ****/
	/*2.a*//*Interrupt Security Registers*/
	setIrqSecState(irqID, irqSecurityLevel);

	/*2.b*//*Set Interrupt configuration register select level-sensitive or edge-triggered*/
//	ICDICFR_EDGE(id);	//WriteReg(ICD_BASE_ADDRESS, ICDICFR_ADDRESS(27), ICDICFR_BITMASK(27));

	/*2.3*//*Interrupt Priority Registers 0=Highest 0xFF= Lowest*/
	setIrqSetPriority(irqID, SGI_SEC_PR, irqSecurityLevel, currentSecurityState);

	/*2.4*//*Enable the interrupt set enable register*/
	ICDISER_ENABLE(irqID);	//ICD->REG_ICDISERn[0] = BIT(27);

	/**** 3-. CPU INTERFACE REGISTERS *****/
	/*3.a*//*Configure in CPU interface register map*/
	/*3.b*/
	/*3.c*//*Enable the Processor Interface - Enable the GIC - from asm_config*/
	/***** 4-. ENABLE GIC *****/
	ICDDCR_ENABLE();
}

static void _HwEnableIrq(xm_u32_t irqID, xm_u32_t irqSecurityLevel, xm_u32_t currentSecurityState)
{
	if(irqID<=GIC_SGI_LAST_ID)
	{	addSGIFromID(irqID, irqSecurityLevel, currentSecurityState);}
	else if(irqID<=GIC_PPI_LAST_ID)
	{	addPPIFromID(irqID, irqSecurityLevel, currentSecurityState);}
	else if(irqID<=GIC_SPI_LAST_ID)
	{	addSPIFromID(irqID, irqSecurityLevel, currentSecurityState);}
	else
	{
	    cpuCtxt_t _ctxt;
        GetCpuCtxt(&_ctxt);
        SystemPanic(&_ctxt, "%s:%u: failed assertion \n", __FILE__, __LINE__);
	}
}
void HwEnableUserIrq(xm_u32_t irq)
{
	_HwEnableIrq(irq, ICDISD_NONSEC, 0);
}
void HwEnableUserIrqFromUsr(xm_u32_t irq)
{
	_HwEnableIrq(irq, ICDISD_NONSEC, 1);
}
void HwEnableIrq(xm_u32_t irq)
{
	_HwEnableIrq(irq, ICDISD_SEC, 0);
}
void HwEnableIrqFromUsr(xm_u32_t irq)
{
	_HwEnableIrq(irq, ICDISD_SEC, 1);
}
void HwDisableIrq(xm_u32_t irq)
{
	/**** DISTRIBUtOR CONFIGURATION ****/
	/**** 0-. DISABLE GIC *****/
	ICDDCR_DISABLE();

	/*2.e*//*Enable the interrupt set enable register*/
	ICDISER_DISABLE(irq);	//ICD->REG_ICDISERn[0] = BIT(27);

	/**** 4-. ENABLE GIC *****/
	ICDDCR_ENABLE();
}

inline void _CleanGIC(void)
{
	unsigned int r;
	for(r=0;r<HWIRQS_VECTOR_SIZE;r++)
	{	WriteReg(ICD_BASE_ADDRESS, ICDICER_ADDRESS((r*32)),0xffffffff); }
}
void SetupSGIs(void)
{
//Extended IRQs
    HwEnableUserIrq(XM_EXTIRQ_SGI_NUM);
    ICDISER_DISABLE(XM_EXTIRQ_SGI_NUM); //Disable SGI for idle

//IPIs /*Commented while is enabled in smp.c*/
//    HwEnableIrq(XM_IPI_SGI_NUM);
//    ICDISER_DISABLE(XM_IPI_SGI_NUM); //Disable SGI for idle

//    CA9_SCR_NONSECURE();
//    HwEnableUserIrqFromUsr(XM_EXTIRQ_SGI_NUM);
//    CA9_SCR_SECURE();

//    cpu->globalIrqMask[0]&=(~0x1);
}

static inline void _InitGIC(xm_u32_t ccpu_)
{
	ASSERT(CONFIG_NO_HWIRQS==(32*(ICD_GET_INT_LINES_NUMBER()+1)));

	if(!ccpu_){_CleanGIC();}
	CA9_SCR_FIQ_AS_MONITOR();
	CA9_SCR_EA_AS_MONITOR();

	/**** 0-. DISABLE GIC *****/
	if(!ccpu_)
	{
#	ifdef BANKED_REG_SUPPORT
		BANKEDREG_APPLY(ICDDCR_DISABLE());
#	else
		ICDDCR_DISABLE();
#	endif
	}
	/**** 3-. CPU INTERFACE REGISTERS *****/
	/*3.a*//*Configure in CPU interface register map*/
	WriteReg(ICC_BASE_ADDRESS, ICCPMR, 0xff);

	/*3.b*//*Configure bit point. [0x0 to 0x3]*/
#	ifdef BANKED_REG_SUPPORT
	WriteReg(ICC_BASE_ADDRESS, ICCBPR,  0x2);
	WriteReg(ICC_BASE_ADDRESS, ICCABPR, 0x3);
#	else
	WriteReg(ICC_BASE_ADDRESS, ICCBPR,  0x0);
#	endif

	/*3.c*//*Enable the Processor Interface - Enable the GIC - from asm_config*/
#	ifdef BANKED_REG_SUPPORT
	BANKEDREG_APPLY(ICCICR_ENABLE_SECURE_AS_FIQ());
#	else
	ICCICR_ENABLE_SECURE_AS_FIQ();
#	endif
	ICCICR_ENABLE_NONSECURE();
#	ifdef BANKED_REG_SUPPORT
	BANKEDREG_APPLY(ICCICR_ENABLE_SECURE());
#	else
	ICCICR_ENABLE_SECURE();
#	endif

	/**** 4-. ENABLE GIC *****/
	if(!ccpu_)
	{
#	ifdef BANKED_REG_SUPPORT
		BANKEDREG_APPLY(ICDDCR_ENABLE());
#	else
		ICDDCR_ENABLE();
#	endif
	}
}

void InitGIC(xm_u32_t ccpu)
{
	//eprintf("ARMPorting:\tInitGIC\n");
	xm_s32_t irq;

// Masking all HW IRQs, all irqs have the same priority level (0)
	_InitGIC(ccpu);

	//    StoreIoReg(GetGICCPUBase()+ITMP_REG, 0);

    for (irq=0; irq<CONFIG_NO_HWIRQS; irq++)
    {
    	hwIrqCtrl[irq].Enable	=	HwEnableIrq;
        hwIrqCtrl[irq].Disable	=	HwDisableIrq;
        hwIrqCtrl[irq].Ack	=	HwAckIrq;
        hwIrqCtrl[irq].End	=	HwEndIrq;
        hwIrqCtrl[irq].Force	=	HwForceIrq;
        hwIrqCtrl[irq].Clear	=	HwClearIrq;
    }
}

#else
#error "CONFIG_ARM not defined or arch file build improperly"
#endif
