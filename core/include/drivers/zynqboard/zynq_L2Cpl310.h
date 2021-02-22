/*
 * $FILE: zynq_L2Cpl310.h
 *
 * L2 Cache definitions
 *
 * $VERSION$ - 21/02/2017
 *
 * Author: Yolanda Valiente <yvaliente@fentiss.com>
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE file for the license terms.
 */
/*
 * Changelog:
 */

#ifndef _CORE_INCLUDE_DRIVERS_ZYNQ_ZYNQ_L2CPL310_H__
#define _CORE_INCLUDE_DRIVERS_ZYNQ_ZYNQ_L2CPL310_H__

#ifdef CONFIG_ENABLE_CACHE_L2

#ifdef CONFIG_CACHE_L2_POLICY_WB_WA
#define CACHE_L2_POLICY					(_PG_ARCH_CACHEPOLICY_WB_WA)
#elif defined(CONFIG_CACHE_L2_POLICY_WT_NWA)
#define CACHE_L2_POLICY					(_PG_ARCH_CACHEPOLICY_WT_NWA)
#elif defined(CONFIG_CACHE_L2_POLICY_WB_NWA)
#define CACHE_L2_POLICY					(_PG_ARCH_CACHEPOLICY_WB_NWA)
#else
#define CACHE_L2_POLICY					(_PG_ARCH_CACHEPOLICY_NONCACHEABLE)
#endif

#define CA9_SLCR_RESERVED				(0x0A1C)		/* SLCR Reserved register */
#define L2C_BASE_ADDRESS				0xF8F02000

#define L2C_CACHELINE					32

#define ReadL2CReg(RegOffset)			ReadReg(L2C_BASE_ADDRESS, RegOffset)
#define WriteL2CReg(RegOffset, value)	WriteReg(L2C_BASE_ADDRESS, RegOffset, value)
#define SetL2CReg(RegOffset, value)		SetReg(L2C_BASE_ADDRESS, RegOffset, value)
#define ClearL2CReg(RegOffset, value)	ClearReg(L2C_BASE_ADDRESS, RegOffset, value)

#define SetL2CRegVar(regvar, value)	reg = ((regvar & (~((volatile xm_u32_t)(value)))) | ((volatile xm_u32_t)(value)))
#define ClearL2CRegVar(regvar, value)	reg = (regvar & (~((volatile xm_u32_t)(value))))
#define ConfigureL2CRegVar(regvar, field, mask, value)	regvar = ((regvar & (~((volatile xm_u32_t)(mask)))) | ((volatile xm_u32_t)(value<<field)))

/****************************************************/
/* 	Control Register								*/
/*		name:reg1_control							*/
/****************************************************/
#define L2C_CR						(0x0100)
#define L2C_CR_L2_ENABLE_MASK		(BIT(0))  			/* L2 cache enable.*/

#define L2C_L2_ENABLE()				SetL2CReg(L2C_CR, L2C_CR_L2_ENABLE_MASK)
#define L2C_L2_DISABLE()			ClearL2CReg(L2C_CR, L2C_CR_L2_ENABLE_MASK)

/****************************************************/
/* 	Auxiliary Control Register						*/
/*		name:reg1_aux_control						*/
/****************************************************/
#define L2C_ACR								(0x0104)
#define L2C_ACR_EXCL_MASK					(BIT(12))	/* Exclusive cache configuration*/
#define L2C_ACR_ASSOCIATIVITY_MASK 			(BIT(16))	/* Associativity. 0: 8-way.  1: 16-way.*/
#define L2C_ACR_WAY_SIZE_BIT 				(17)
#define L2C_ACR_WAY_SIZE_MASK 				(7<<L2C_ACR_WAY_SIZE_BIT)	/* bits 17:19 */
#define L2C_ACR_PARITY_MASK					(BIT(21))
#define L2C_ACR_SHARED_MASK					(BIT(22))
#define L2C_ACR_CACHE_REPLACE_POLICY_MASK	(BIT(25))	/* Cache replacement policy. 0: pseudo-random.  1: round-robin.*/

#define L2C_ACR_WAY_SIZE_16kb		(0x1)
#define L2C_ACR_WAY_SIZE_32kb		(0x2)
#define L2C_ACR_WAY_SIZE_64kb		(0x3)
#define L2C_ACR_WAY_SIZE_128kb		(0x4)
#define L2C_ACR_WAY_SIZE_256kb		(0x5)
#define L2C_ACR_WAY_SIZE_512kb		(0x6)

#define L2C_CONFIGURE_ACR_EXCL_DISABLE(regvar)			ClearL2CRegVar(regvar, L2C_ACR_EXCL_MASK)
#define L2C_CONFIGURE_ACR_EXCL_ENABLE(regvar)			SetL2CRegVar(regvar, L2C_ACR_EXCL_MASK)
#define L2C_CONFIGURE_ACR_WAY_SIZE(regvar, value)		ConfigureL2CRegVar(regvar, L2C_ACR_WAY_SIZE_BIT, L2C_ACR_WAY_SIZE_MASK, value)
#define L2C_CONFIGURE_ACR_ASSOCIATIVITY_8WAY(regvar)	ClearL2CRegVar(regvar, L2C_ACR_ASSOCIATIVITY_MASK)
#define L2C_CONFIGURE_ACR_ASSOCIATIVITY_16WAY(regvar)	SetL2CRegVar(regvar, L2C_ACR_ASSOCIATIVITY_MASK)
#define L2C_CONFIGURE_ACR_PARITY_DISABLE(regvar)		ClearL2CRegVar(regvar, L2C_ACR_PARITY_MASK)
#define L2C_CONFIGURE_ACR_PARITY_ENABLE(regvar)			SetL2CRegVar(regvar, L2C_ACR_PARITY_MASK)
#define L2C_CONFIGURE_ACR_SHARED_DISABLE(regvar)		ClearL2CRegVar(regvar, L2C_ACR_SHARED_MASK)
#define L2C_CONFIGURE_ACR_SHARED_ENABLE(regvar)			SetL2CRegVar(regvar, L2C_ACR_SHARED_MASK)
#define L2C_CONFIGURE_ACR_CACHE_REPLACE_POLICY_PSEUDO_RANDOM(regvar)	ClearL2CRegVar(regvar, L2C_ACR_CACHE_REPLACE_POLICY_MASK)
#define L2C_CONFIGURE_ACR_CACHE_REPLACE_POLICY_ROUND_ROBIN(regvar)		SetL2CRegVar(regvar, L2C_ACR_CACHE_REPLACE_POLICY_MASK)

#define ReadL2CAuxiliaryControlReg()				ReadL2CReg(L2C_ACR)
#define WriteL2CAuxiliaryControlReg(value)			WriteL2CReg(L2C_ACR, value)

/****************************************************/
/* 	Tag and Data RAM Latency Control Registers		*/
/*		name:reg1_tag_ram_control and
 * 					reg1_data_ram_control			*/
/****************************************************/
#define L2C_TRAMCR						(0x0108)		/* Configures Tag RAM latencies */
#define L2C_DRAMCR						(0x010C)		/* Configures data RAM latencies */
#define L2C_RAMCR_SETUP_LATENCY_BIT		(0)
#define L2C_RAMCR_SETUP_LATENCY_MASK	(7)									/* bits 0:2 */
#define L2C_RAMCR_READ_LATENCY_BIT		(4)
#define L2C_RAMCR_READ_LATENCY_MASK		(7<<L2C_RAMCR_READ_LATENCY_BIT)		/* bits 4:6 */
#define L2C_RAMCR_WRITE_LATENCY_BIT		(8)
#define L2C_RAMCR_WRITE_LATENCY_MASK	(7<<L2C_RAMCR_WRITE_LATENCY_BIT)	/* bits 8:10 */

#define L2C_RAMCR_LATENCY_1CYCLE	(0x0)
#define L2C_RAMCR_LATENCY_2CYCLES	(0x1)
#define L2C_RAMCR_LATENCY_3CYCLES	(0x2)
#define L2C_RAMCR_LATENCY_4CYCLES	(0x3)
#define L2C_RAMCR_LATENCY_5CYCLES	(0x4)
#define L2C_RAMCR_LATENCY_6CYCLES	(0x5)
#define L2C_RAMCR_LATENCY_7CYCLES	(0x6)
#define L2C_RAMCR_LATENCY_8CYCLES	(0x7)

#define L2C_CONFIGURE_RAM_SETUP_LATENCY(regvar, value)		ConfigureL2CRegVar(regvar, L2C_RAMCR_SETUP_LATENCY_BIT, L2C_RAMCR_SETUP_LATENCY_MASK, value)
#define L2C_CONFIGURE_RAM_READ_LATENCY(regvar, value)		ConfigureL2CRegVar(regvar, L2C_RAMCR_READ_LATENCY_BIT, L2C_RAMCR_READ_LATENCY_MASK, value)
#define L2C_CONFIGURE_RAM_WRITE_LATENCY(regvar, value)		ConfigureL2CRegVar(regvar, L2C_RAMCR_WRITE_LATENCY_BIT, L2C_RAMCR_WRITE_LATENCY_MASK, value)

#define ReadL2CTagRAMControlReg()			ReadL2CReg(L2C_TRAMCR)
#define WriteL2CTagRAMControlReg(value)		WriteL2CReg(L2C_TRAMCR, value)
#define ReadL2CDataRAMControlReg()			ReadL2CReg(L2C_DRAMCR)
#define WriteL2CDataRAMControlReg(value)	WriteL2CReg(L2C_DRAMCR, value)


/****************************************************/
/* 	Interrupt Mask Register							*/
/*		name:reg2_int_mask							*/
/****************************************************/
#define L2C_IMR					(0x0214)
#define L2C_IMR_ERRRD_MASK		(0x00000040)		/* Error on L2 data RAM (Read) */
#define L2C_IMR_ERRRT_MASK		(0x00000020)		/* Error on L2 tag RAM (Read) */
#define L2C_IMR_ERRWD_MASK		(0x00000010)		/* Error on L2 data RAM (Write) */
#define L2C_IMR_ERRW_MASK		(0x00000008)		/* Error on L2 tag RAM (Write) */
#define L2C_IMR_PARRD_MASK		(0x00000004)		/* Parity Error on L2 data RAM (Read) */
#define L2C_IMR_PARRT_MASK		(0x00000002)		/* Parity Error on L2 tag RAM (Read) */
#define L2C_IMR_ALL_MASK		(0x1FF)
#define L2C_IMR_EN_INT_MASK		(L2C_IMR_PARRD_MASK | L2C_IMR_PARRT_MASK)	/* Bits 8:0. 0: Masked 1: Enabled */

#define ReadL2CInterruptMaskReg()			ReadL2CReg(L2C_IMR)
#define WriteL2CInterruptMaskReg(value)		WriteL2CReg(L2C_IMR, value)


/****************************************************/
/* 	Raw Interrupt Status Register					*/
/*		name:reg2_int_raw_status					*/
/****************************************************/
#define L2C_IRSR							(0x021C)

#define ReadL2CInterruptRawStatusReg()		ReadL2CReg(L2C_IRSR)


/****************************************************/
/* 	Interrupt CLEAR Register						*/
/*		name:reg2_int_clear							*/
/****************************************************/
#define L2C_ICR								(0x0220)
#define L2C_ICR_CLEAR_MASK					(0xFF)		/* Write 1 in 8:0 to clear the corresponding bits */

#define ReadL2CInterruptClearReg()			ReadL2CReg(L2C_ICR)
#define WriteL2CInterruptClearReg(value)	WriteL2CReg(L2C_ICR, value)


/****************************************************/
/* 	Register to drain the STB						*/
/*		name: reg7_cache_sync						*/
/****************************************************/
#define L2C_CACHE_SYNC						(0x0730)

#define ReadL2CCacheSyncReg()				ReadL2CReg(L2C_CACHE_SYNC)
#define WriteL2CCacheSyncReg(value)			WriteL2CReg(L2C_CACHE_SYNC, value)
#define L2CacheSync()						WriteL2CCacheSyncReg(0x0)


/****************************************************/
/* 	Register to Invalidate Line by PA: Specific
 *  L2 cache line is marked as not valid			*/
/*		name:reg7_inv_pa							*/
/****************************************************/
#define L2C_IPR							(0x0770)

#define ReadL2CInvPaReg()				ReadL2CReg(L2C_IPR)
#define WriteL2CInvPaReg(value)			WriteL2CReg(L2C_IPR, value)


/****************************************************/
/* 	Register to invalidate all data in specified
 * 	ways, including dirty data						*/
/*		name:reg7_inv_way							*/
/****************************************************/
#define L2C_IWR							(0x077C)
#define L2C_IWR_INV_ALL_ENTRIES_MASK	(0xFFFF)

#define ReadL2CInvWayReg()				ReadL2CReg(L2C_IWR)
#define WriteL2CInvWayReg(value)		WriteL2CReg(L2C_IWR, value)


/****************************************************/
/* 	Register to clean and Invalidate Line by PA		*/
/*		name:reg7_clean_inv_pa						*/
/****************************************************/
#define L2C_CIPR						(0x07F0)

#define ReadL2CCleanInvPaReg()			ReadL2CReg(L2C_CIPR)
#define WriteL2CCleanInvPaReg(value)	WriteL2CReg(L2C_CIPR, value)


/****************************************************/
/* 	Register to clean and invalidate by way			*/
/*		name:reg7_clean_inv_way						*/
/****************************************************/
#define L2C_CIWR						(0x07FC)
#define L2C_CIWR_INV_ALL_ENTRIES_MASK	(0xFFFF)

#define ReadL2CCleanInvWayReg()			ReadL2CReg(L2C_CIWR)
#define WriteL2CCleanInvWayReg(value)	WriteL2CReg(L2C_CIWR, value)


/****************************************************/
/* 	Prefetch Control Register						*/
/*		name:reg15_prefetch_ctrl					*/
/****************************************************/
#define L2C_PFCR					(0x0F60)		/* Enables prefetch-related features */
#define L2C_PFCR_DATA_PREF_EN_MASK	(BIT(28))		/* Enable data prefetching.*/
#define L2C_PFCR_INST_PREF_EN_MASK	(BIT(29))  		/* Enable instruction prefetching.*/

#define L2C_PFCR_DATA_PREF_EN_ENABLE(regvar)	SetL2CRegVar(regvar, L2C_PFCR_DATA_PREF_EN_MASK)
#define L2C_PFCR_DATA_PREF_EN_DISABLE(regvar)	CleartL2CRegVar(regvar, L2C_PFCR_DATA_PREF_EN_MASK)
#define L2C_PFCR_DATA_INST_EN_ENABLE(regvar)	SetL2CRegVar(regvar, L2C_PFCR_INST_PREF_EN_MASK)
#define L2C_PFCR_DATA_INST_EN_DISABLE(regvar)	ClearL2CRegVar(regvar, L2C_PFCR_INST_PREF_EN_MASK)

#define ReadL2CPrefetchControlReg()			ReadL2CReg(L2C_PFCR)
#define WriteL2CPrefetchControlReg(value)	WriteL2CReg(L2C_PFCR, value)


/****************************************************/
/* 	Debug Control Register: Controls the operating
 *  mode clock and power modes						*/
/*		name:reg15_debug_ctrl						*/
/****************************************************/
#define L2C_DCR								(0x0F40)
#define L2C_DCR_WB_LF_DISABLE_MASK			(0x3)			/* Disable Write-back and line fills. */
#define L2C_DCR_WB_LF_ENABLE_MASK			(0x0)			/* Enable Write-back and line fills. */

#define ReadL2CDebugControlReg()			ReadL2CReg(L2C_DCR)
#define WriteL2CDebugControlReg(value)		WriteL2CReg(L2C_DCR, value)


/****************************************************/
/* 	Power Control Register: Forces specific cache
 *  behavior required for debug.					*/
/*		name:reg15_power_ctrl						*/
/****************************************************/
#define L2C_PWCR							(0x0F80)
#define L2C_PWCR_STANDBY_MODE_EN_MASK		(BIT(0))			/* Standby mode enable. */
#define L2C_PWCR_DYNAMIC_CLK_GATING_EN_MASK	(BIT(1))			/* Dynamic clock gating enable. */

#define ReadL2CPowerControlReg()			ReadL2CReg(L2C_PWCR)
#define WriteL2CPowerControlReg(value)		WriteL2CReg(L2C_PWCR, value)



#ifndef __ASSEMBLY__
extern void EnableL2Cache(void);
extern void DisableL2Cache(void);
extern void InvalidateL2Cache(void);
extern void InvalidateL2CacheRange(xmAddress_t addr, xm_u32_t size);
extern void InvalidateL2CacheLine(xmAddress_t addr);
extern void FlushL2Cache(void);
extern void FlushL2CacheRange(xmAddress_t addr, xm_u32_t size);
#endif /*__ASSEMBLY__*/

#endif /* CONFIG_ENABLE_CACHE_L2 */

#endif /*_CORE_INCLUDE_DRIVERS_ZYNQ_ZYNQ_L2CPL310_H__*/
