/*
 * This is common definitions for CM3 and CR4 tests,
 * runnung on 43602 chip.
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id$
*/
#ifndef _ARMS_43602_H
#define _ARMS_43602_H

#include "pcie_core.h"
#include "bcmdefs.h"

#define dmb		asm("DMB")

#define ACCESS(addr) *(uint32 volatile *) ((addr))


/* Address Map */
#define REGBASE_CHIPC		0x18000000
#define REGBASE_DOT11 		0x18001000
#define REGBASE_ARMCR4		0x18002000
#define REGBASE_PCIEGEN2	0x18003000
#define REGBASE_SOCSRAM		0x18004000
#define REGBASE_ARMCM3		0x18005000
#define REGBASE_CR4DBG		0x18006000


#define MEMBASE_ATCM_ROM	0x00000000
#define MEMBASE_ATCM_RAM	0x00180000
#define MEMBASE_BTCM_RAM	0x00200000
#define MEMBASE_SOCSRAM		0x00300000

#define PCIE_ADDR_MATCH1	0x08000000
#define NIC301_GPV_ADDR		0x18200000
#define PCIE_ADDR_MATCH2	0x80000000

/* PCIEGEN2 */
#define PCIEGEN2_CAPABILITIES	0x08
#define PCIEGEN2_INTSTATUS		0x20
#define PCIEGEN2_INTMASK		0x24
#define PCIEGEN2_INTMASK2		0x5c
#define PCIEGEN2_DEV2HOST_DB0	0x148
#define PCIEGEN2_DEV2HOST_DB1	0x14c

#define PCIEGEN2_DMA_MASK		0x00000080
#define PCIEGEN2_DB0_MASK		0x00010000
#define PCIEGEN2_DB1_MASK		0x00020000

#define PCIEGEN2_DE_MASK		0x00000400
#define PCIEGEN2_DA_MASK		0x00000800
#define PCIEGEN2_DP_MASK		0x00001000
#define PCIEGEN2_RU_MASK		0x00002000
#define PCIEGEN2_RI_MASK		0x00010000
#define PCIEGEN2_XI_MASK		0x01000000

/* CM3 interrupts */
#define ARMCM3_NMIISR_STATUS	0x10
#define ARMCM3_NMI_MASK			0x14
#define ARMCM3_ISR_MASK			0x18
#define ARMCM3_SWINT_REG		0x1c

#define ARMCM3_TIMER_INTR_MASK	0x00000020
#define ARMCM3_PCIE2_INTR_MASK	0x00000008
#define ARMCM3_SW0_INTR_MASK	0x00010000
#define ARMCM3_SW1_INTR_MASK	0x00020000

#define ARMCM3_INT_TYPE_NMI		2
#define ARMCM3_INT_TYPE_EXT		16

#define ARMCM3_TIMER_INT		6

/* CR4 interrupts */
#define ARMCR4_SWINT_REG		0x1c

#define ARMCR4_CC_INTR_MASK				0x00000001
#define ARMCR4_TIMER_INTR_MASK			0x00000004
#define ARMCR4_PCIE2_INTR_MASK			0x00000008
#define ARMCR4_SW0_INTR_MASK			0x00010000
#define ARMCR4_SW1_INTR_MASK			0x00020000
#define ARMCR4_CLOCK_STABLE_INTR_MASK	0x20000000

#define ARMCR4_INT_MASK_TIMER			0x00000001

#define ARMCR4_CLOCK_CONTROL_REG	0x1e0


/* Tests command/status interface */
#define HOST_CM3_CMD_OFF		(MEMBASE_BTCM_RAM + 0x0000)
#define HOST_CR4_CMD_OFF		(MEMBASE_BTCM_RAM + 0x0400)
#define CM3_HOST_STAT_OFF		(MEMBASE_BTCM_RAM + 0x0800)
#define CR4_HOST_STAT_OFF		(MEMBASE_BTCM_RAM + 0x0c00)
#define CR4_CM3_CMD_OFF			(MEMBASE_BTCM_RAM + 0x1000)
#define CM3_CR4_CMD_OFF			(MEMBASE_BTCM_RAM + 0x1400)
#define CR4_CM3_STAT_OFF		(MEMBASE_BTCM_RAM + 0x1800)
#define CM3_CR4_STAT_OFF		(MEMBASE_BTCM_RAM + 0x1c00)

/* TCM Stress test memory allocations */
#define DDRINGTX_ADDR	(MEMBASE_SOCSRAM + 0x4000)
#define DDRINGRX_ADDR	(MEMBASE_SOCSRAM + 0x4100)
#define BME0_SRAM_ADDR	(MEMBASE_SOCSRAM + 0x6000)
#define BME1_SRAM_ADDR	(MEMBASE_SOCSRAM + 0x7000)

#define TCM_DMA_ADDR	(MEMBASE_BTCM_RAM + 0x2000)
#define TCM_CM3_ADDR	(MEMBASE_BTCM_RAM + 0x3000)
#define TCM_CR4_ADDR	(MEMBASE_BTCM_RAM + 0x4000)
#define TCM_BME0_ADDR	(MEMBASE_BTCM_RAM + 0x6000)
#define TCM_BME1_ADDR	(MEMBASE_BTCM_RAM + 0x7000)

#define LAST_WORD(addr, size)	ACCESS((addr) + (size) - 4)

/* Host commands */
#define HOST_CMD_START_WFI			1
#define HOST_CMD_SWINT_ENABLE		2
#define HOST_CMD_SET_SWINT			3
#define HOST_CMD_DOORBELL_ENABLE	4
#define HOST_CMD_DOORBELL_SET		5
#define HOST_CMD_DMA_TRANSFER		6
#define HOST_CMD_BME_TRANSFER		7
#define HOST_CMD_TCM_STRESS			8
#define HOST_CMD_CLOCK_SWITCH		9
#define HOST_CMD_CLOCK_STRESS		10
#define HOST_CMD_DMA_ALIGNMENT		11
#define HOST_CMD_PMU_TIMER			12

typedef struct {
	uint32 cmd;
	uint32 params[1];
} host_cmd_t;

/* SW INT params */
typedef struct {
	uint32 swint;	/* 0 or 1 */
	uint32 is_nmi;	/* nmi(1) or isr(0) */
} swint_params_t;

/* Doorbell params */
typedef struct {
	uint32 db;		/* 0 or 1 */
	uint32 val;		/* value */
} doorbell_params_t;

/* DMA Transfer */
typedef struct {
	dma64dd_t		*ddringtx;	/* 00 - tx descr table addr */
	dma64dd_t		*ddringrx;	/* 04 - rx descr table addr */
	uint32			ntxd;		/* 08 - tx desr table entries num */
	uint32			nrxd;		/* 0c - rx desr table entries num */
	uint32			tx_control; /* 10 - dma tx control reg val */
	uint32			rx_control;	/* 14 - dma tx control reg val */
	uint32			src_len;	/* 18 - transfer src len */
	uint32			dst_len;	/* 1c - transfer dst len */
	dma64addr_t 	src; 		/* 20 - transfer src buffer */
	dma64addr_t 	dst;		/* 28 - transfer dst buffer */
	uint32			src_nd;		/* 30 - transfer num of src descr entries */
	uint32			dst_nd;		/* 34 - transfer num of dst descr entries */
	uint32			h2d;		/* 38 - d2h(0) or h2d(1) */
	uint32			intmask;	/* 3c - enable dma interrupts mask */
} dma_params_t;

/* BME transfer */
typedef struct {
	uint32	src0;	/* 00 - src address bme0 (0 - for bzero operation) */
	uint32	dst0;	/* 04 - dst address bme0 */
	uint32	len0;	/* 08 - length bme0 */
	uint32	src1;	/* 0c - src address bme1 (0 - for bzero operation) */
	uint32	dst1;	/* 10 - dst address bme1 */
	uint32	len1;	/* 14 - length bme1 */
} bme_params_t;

/* TCM Stress */
typedef struct {
	dma64addr_t		haddr; 	/* 00 - dma ddr host addr */
} stress_params_t;

/* CM3 status */
#define CM3_MAIN_ENTERED		0x01010101
#define CM3_WFI_LOOP_STARTED	0x10101010


typedef struct {
	uint32 count;				/* 00 - cm3 alive */
	uint32 status;				/* 04 - latest test status */
	uint32 int_count[32];		/* 08 - (+ 4*int_type) */
	uint32 swint_count[2];		/* 88 - swint count */
	uint32 doorbell[2];			/* 90 - dorbell value */
	uint32 dma_src[2];			/* 98 - dma low[0] hi[1] addr */
	uint32 dma_dst[2];			/* a0 - dma low[0] hi[1] addr */
	uint32 h2d_de_count;		/* a8 - DescrErr int count  */
	uint32 h2d_da_count;		/* ac - DataErr int count */
	uint32 h2d_dp_count;		/* b0 - DescrProtErr int count */
	uint32 h2d_ru_count;		/* b4 - RecvDescrUf int count */
	uint32 h2d_ri_count;		/* b8 - RcvInt int count */
	uint32 h2d_xi_count;		/* bc - XmtInt int count */
	uint32 d2h_de_count;		/* c0 - DescrErr int count  */
	uint32 d2h_da_count;		/* c4 - DataErr int count */
	uint32 d2h_dp_count;		/* c8 - DescrProtErr int count */
	uint32 d2h_ru_count;		/* cc - RecvDescrUf int count */
	uint32 d2h_ri_count;		/* d0 - RcvInt int count */
	uint32 d2h_xi_count;		/* d4 - XmtInt int count */
	uint32 core_int_count[32];	/* d8 - (+ 4*core) */
	uint32 bme_cmplt_cnt[2];	/* 158 - bme 0/1 complete wait loop count */
	uint32 pmu_int_count;		/* 160 - pmu interrupt count */
	uint32 clk_sw1_count;		/* 164 - clock switch count */
	uint32 clk_sw2_count;		/* 168 - clock switch count */

} arm_stat_t;

/* CR4 status */
#define CR4_MAIN_ENTERED		0x01010101
#define CR4_WFI_LOOP_STARTED	0x10101010


#endif /* _ARMS_43602_H */
