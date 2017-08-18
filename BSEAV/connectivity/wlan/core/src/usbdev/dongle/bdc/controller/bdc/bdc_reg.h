/*
 *  Defines BDC registers
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 *
 * <<Broadcom-WL-IPTag/Proprietary:>>
 *
 * $Id$
 */

#ifndef _BDCREG_H_
#define _BDCREG_H_

#define B_0	0x01
#define B_1	0x02
#define B_2	0x04
#define B_3	0x08
#define B_4	0x10
#define B_5	0x20
#define B_6	0x40
#define B_7	0x80
#define B_8	0x100
#define B_9	0x200
#define B_10	0x400
#define B_11	0x800
#define B_12	0x1000
#define B_13	0x2000
#define B_14	0x4000
#define B_15	0x8000
#define B_16	0x10000
#define B_17	0x20000
#define B_18	0x40000
#define B_19	0x80000
#define B_20	0x100000
#define B_21	0x200000
#define B_22	0x400000
#define B_23	0x800000
#define B_24	0x1000000
#define B_25	0x2000000
#define B_26	0x4000000
#define B_27	0x8000000
#define B_28	0x10000000
#define B_29	0x20000000
#define B_30	0x40000000
#define B_31	0x80000000

/* Register definition for USB_CONFIG_CONTROL_REG(cr_pad_config_adr11) */
#define USB_PDN_DUSB_DP		0x00000008		/* Pull-down on downstream D+ line. */
#define USB_PDN_DUSB_DN		0x00000040		/* Pull-down on downstream D- line */
#define USB_SUSPEND_DUSB_MUX	0x00000080		/* Mux select for downstream transceiver
							 * suspend
							 */
#define USB_SUSPEND_DUSB_VAL	0x00000100		/* FW value for downstream transceiver
							 * suspend
							 */

#define USB_PUP_IDLE_HUSB_DP	0x00010000	/* Pull-up on upstream D+ line. */
#define USB_PUP_ACT_HUSB_DP_MUX 0x00020000	/* Mux select for active pull-up on upstream D+ */
#define USB_PUP_ACT_HUSB_DP_VAL 0x00040000	/* FW value for active pull-up on upstream D+ */
#define USB_PDN_HUSB_DP		0x00080000	/* Pull-down on upstream D+ line */
#define USB_PUP_IDLE_HUSB_DN	0x00100000	/* Pull-up on upstream D- line */
#define USB_PUP_ACT_HUSB_DN	0x00200000	/* Pull-down on upstream D- line */
/* Pull-down on upstream D- line. In the USB IP doc, this is called 'pdn_pup_act_HUSB_DN' */
#define USB_PDN_HUSB_DN		0x00400000
#define USB_SUSPEND_HUSB_MUX	0x00800000	/* Mux select for upstream transceiver suspend */
#define USB_SUSPEND_HUSB_VAL	0x01000000	/* FW value for upstream transceiver suspend */

/* 0x1800_3000 - 0x1800_3fff	USB30d controller (b_dC) register region */
#define UBDC_BDCCFG0_ADR	0x00000000
#define UBDC_BDCCFG0		(*(volatile unsigned int *)UBDC_BDCCFG0_ADR)
#define UBDC_BDCCFG1_ADR	0x00000004
#define UBDC_BDCCFG1		(*(volatile unsigned int *)UBDC_BDCCFG1_ADR)
#define UBDC_BDCCAP0_ADR	0x00000008
#define UBDC_BDCCAP0		(*(volatile unsigned int *)UBDC_BDCCAP0_ADR)
#define UBDC_BDCCAP1_ADR	0x0000000c
#define UBDC_BDCCAP1		(*(volatile unsigned int *)UBDC_BDCCAP1_ADR)
#define UBDC_CMDPAR0_ADR	0x00000010
#define UBDC_CMDPAR0		(*(volatile unsigned int *)UBDC_CMDPAR0_ADR)
#define UBDC_CMDPAR1_ADR	0x00000014
#define UBDC_CMDPAR1		(*(volatile unsigned int *)UBDC_CMDPAR1_ADR)
#define UBDC_CMDPAR2_ADR	0x00000018
#define UBDC_CMDPAR2		(*(volatile unsigned int *)UBDC_CMDPAR2_ADR)
#define UBDC_CMDSC_ADR		0x0000001c
#define UBDC_CMDSC		(*(volatile unsigned int *)UBDC_CMDSC_ADR)
#define UBDC_USPSC_ADR		0x00000020
#define UBDC_USPSC		(*(volatile unsigned int *)UBDC_USPSC_ADR)
#define UBDC_USPTDS_ADR		0x00000024
#define UBDC_USPTDS		(*(volatile unsigned int *)UBDC_USPTDS_ADR)
#define UBDC_USPPMS_ADR		0x00000028
#define UBDC_USPPMS		(*(volatile unsigned int *)UBDC_USPPMS_ADR)
#define UBDC_USPPM2_ADR		0x0000002c
#define UBDC_USPPM2		(*(volatile unsigned int *)UBDC_USPPM2_ADR)
#define UBDC_EXCPBA_ADR		0x00000030
#define UBDC_EXCPBA		(*(volatile unsigned int *)UBDC_EXCPBA_ADR)
#define UBDC_SPBBAL_ADR		0x00000038
#define UBDC_SPBBAL		(*(volatile unsigned int *)UBDC_SPBBAL_ADR)
#define UBDC_SPBBAH_ADR		0x0000003c
#define UBDC_SPBBAH		(*(volatile unsigned int *)UBDC_SPBBAH_ADR)
#define UBDC_BDCSC_ADR		0x00000040
#define UBDC_BDCSC		(*(volatile unsigned int *)UBDC_BDCSC_ADR)
#define UBDC_MFNUM_ADR		0x00000048
#define UBDC_MFNUM		(*(volatile unsigned int *)UBDC_MFNUM_ADR)
#define UBDC_XSFNTF_ADR		0x0000004c
#define UBDC_XSFNTF		(*(volatile unsigned int *)UBDC_XSFNTF_ADR)
#define UBDC_DEVICESTSA_ADR	0x00000058
#define UBDC_DEVICESTSA		(*(volatile unsigned int *)UBDC_DEVICESTSA_ADR)
#define UBDC_DEVICESTSB_ADR	0x0000005c
#define UBDC_DEVICESTSB		(*(volatile unsigned int *)UBDC_DEVICESTSB_ADR)
#define UBDC_EPS0_ADR		0x00000060
#define UBDC_EPS0		(*(volatile unsigned int *)UBDC_EPS0_ADR)
#define UBDC_EPS1_ADR		0x00000064
#define UBDC_EPS1		(*(volatile unsigned int *)UBDC_EPS1_ADR)
#define UBDC_EPS2_ADR		0x00000068
#define UBDC_EPS2		(*(volatile unsigned int *)UBDC_EPS2_ADR)
#define UBDC_EPS3_ADR		0x0000006c
#define UBDC_EPS3		(*(volatile unsigned int *)UBDC_EPS3_ADR)
#define UBDC_EPS4_ADR		0x00000070
#define UBDC_EPS4		(*(volatile unsigned int *)UBDC_EPS4_ADR)
#define UBDC_EPS5_ADR		0x00000074
#define UBDC_EPS5		(*(volatile unsigned int *)UBDC_EPS5_ADR)
#define UBDC_EPS6_ADR		0x00000078
#define UBDC_EPS6		(*(volatile unsigned int *)UBDC_EPS6_ADR)
#define UBDC_EPS7_ADR		0x0000007c
#define UBDC_EPS7		(*(volatile unsigned int *)UBDC_EPS7_ADR)
#define UBDC_SRRBAL0_ADR	0x00000200
#define UBDC_SRRBAL0		(*(volatile unsigned int *)UBDC_SRRBAL0_ADR)
#define UBDC_SRRBAH0_ADR	0x00000204
#define UBDC_SRRBAH0		(*(volatile unsigned int *)UBDC_SRRBAH0_ADR)
#define UBDC_SRRINT0_ADR	0x00000208
#define UBDC_SRRINT0		(*(volatile unsigned int *)UBDC_SRRINT0_ADR)
#define UBDC_INTCLS0_ADR	0x0000020c
#define UBDC_INTCLS0		(*(volatile unsigned int *)UBDC_INTCLS0_ADR)
#define UBDC_SRRBAL1_ADR	0x00000210
#define UBDC_SRRBAL1		(*(volatile unsigned int *)UBDC_SRRBAL1_ADR)
#define UBDC_SRRBAH1_ADR	0x00000214
#define UBDC_SRRBAH1		(*(volatile unsigned int *)UBDC_SRRBAH1_ADR)
#define UBDC_SRRINT1_ADR	0x00000218
#define UBDC_SRRINT1		(*(volatile unsigned int *)UBDC_SRRINT1_ADR)
#define UBDC_INTCLS1_ADR	0x0000021c
#define UBDC_INTCLS1		(*(volatile unsigned int *)UBDC_INTCLS1_ADR)
#define UBDC_ECHBIU_ADR		0x00000c00
#define UBDC_ECHBIU		(*(volatile unsigned int *)UBDC_ECHBIU_ADR)
#define UBDC_BIUSPC_ADR		0x00000c04
#define UBDC_BIUSPC		(*(volatile unsigned int *)UBDC_BIUSPC_ADR)
#define UBDC_BIUSPC1_ADR	0x00000c08
#define UBDC_BIUSPC1		(*(volatile unsigned int *)UBDC_BIUSPC1_ADR)
#define UBDC_BIUSPC2_ADR	0x00000c0c
#define UBDC_BIUSPC2		(*(volatile unsigned int *)UBDC_BIUSPC2_ADR)
#define UBDC_BIULPM_ADR		0x00000c10
#define UBDC_BIULPM		(*(volatile unsigned int *)UBDC_BIULPM_ADR)
#define UBDC_BIUQOS_ADR		0x00000c14
#define UBDC_BIUQOS		(*(volatile unsigned int *)UBDC_BIUQOS_ADR)
#define UBDC_ECHCSR_ADR		0x00000c20
#define UBDC_ECHCSR		(*(volatile unsigned int *)UBDC_ECHCSR_ADR)
#define UBDC_CSRSPC_ADR		0x00000c24
#define UBDC_CSRSPC		(*(volatile unsigned int *)UBDC_CSRSPC_ADR)
#define UBDC_ECHAIU_ADR		0x00000c30
#define UBDC_ECHAIU		(*(volatile unsigned int *)UBDC_ECHAIU_ADR)
#define UBDC_AIUDMA_ADR		0x00000c34
#define UBDC_AIUDMA		(*(volatile unsigned int *)UBDC_AIUDMA_ADR)
#define UBDC_AIUSTM_ADR		0x00000c38
#define UBDC_AIUSTM		(*(volatile unsigned int *)UBDC_AIUSTM_ADR)
#define UBDC_AIUOEPS0_ADR	0x00000c40
#define UBDC_AIUOEPS0		(*(volatile unsigned int *)UBDC_AIUOEPS0_ADR)
#define UBDC_AIUOEPS1_ADR	0x00000c44
#define UBDC_AIUOEPS1		(*(volatile unsigned int *)UBDC_AIUOEPS1_ADR)
#define UBDC_AIUOEPS2_ADR	0x00000c48
#define UBDC_AIUOEPS2		(*(volatile unsigned int *)UBDC_AIUOEPS2_ADR)
#define UBDC_AIUOEPS3_ADR	0x00000c4c
#define UBDC_AIUOEPS3		(*(volatile unsigned int *)UBDC_AIUOEPS3_ADR)
#define UBDC_AIUOEPS4_ADR	0x00000c50
#define UBDC_AIUOEPS4		(*(volatile unsigned int *)UBDC_AIUOEPS4_ADR)
#define UBDC_AIUIEPS0_ADR	0x00000c80
#define UBDC_AIUIEPS0		(*(volatile unsigned int *)UBDC_AIUIEPS0_ADR)
#define UBDC_AIUIEPS1_ADR	0x00000c84
#define UBDC_AIUIEPS1		(*(volatile unsigned int *)UBDC_AIUIEPS1_ADR)
#define UBDC_AIUIEPS2_ADR	0x00000c88
#define UBDC_AIUIEPS2		(*(volatile unsigned int *)UBDC_AIUIEPS2_ADR)
#define UBDC_AIUIEPS3_ADR	0x00000c8c
#define UBDC_AIUIEPS3		(*(volatile unsigned int *)UBDC_AIUIEPS3_ADR)
#define UBDC_AIUIEPS4_ADR	0x00000c90
#define UBDC_AIUIEPS4		(*(volatile unsigned int *)UBDC_AIUIEPS4_ADR)
#define UBDC_AIUIEPS5_ADR	0x00000c94
#define UBDC_AIUIEPS5		(*(volatile unsigned int *)UBDC_AIUIEPS5_ADR)
#define UBDC_AIUIEPS6_ADR	0x00000c98
#define UBDC_AIUIEPS6		(*(volatile unsigned int *)UBDC_AIUIEPS6_ADR)

#define BDCREG_SRR_RWS	(1 << 4)
#define BDCREG_SRR_RST	(1 << 3)
#define	BDCREG_SRR_ISR	(1 << 2)
#define BDCREG_SRR_IE	(1 << 1)
#define BDCREG_SRR_IP	(1 << 0)
#define BDCREG_SRR_EPI(p)	(((p) & (0xFFU << 24)) >> 24)
#define BDCREG_SRR_DPI(p)	(((p) & (0xFFU << 16)) >> 16)
#define BDCREG_SRR_DPI_MASK (0xFF << 16)
#define BDCREG_SRR_TO_DPI(p) ((p) << 16)

#define	BDCREG_SC_HLE	(1 << 16)
#define BDCREG_SC_RWE	(1 << 3)

/* BDC Status and Control - SEC 5.5, BDCSC */
#define	BDCREG_SC_COP_RESET	(0x01U << 29)
#define BDCREG_SC_COP_RUN	(0x02U << 29)
#define	BDCREG_SC_COP_STOP	(0x04U << 29)
#define BDCREG_SC_COP_SAVE	(0x05U << 29)
#define BDCREG_SC_COP_RESTORE	(0x06U << 29)
#define BDCREG_SC_COP_MASK	(BDCREG_SC_COP_RESET | BDCREG_SC_COP_RUN | BDCREG_SC_COP_STOP)
#define BDCREG_SC_COS		(0x01U << 28)

#define BDCREG_SC_STATUS_MASK	(0x07 << 20)
#define BDCREG_SC_STATUS_HALT	(0x01 << 20)
#define BDCREG_SC_STATUS_NORMAL (0x02 << 20)
#define BDCREG_SC_STATUS_BUSY	(0x07 << 20)

#define	BDCI_CSTS(p) (((p) & (0x7 << 22)) >> 22)
#define	BDCREG_SC_GIE	(0x01 << 1)
#define BDCREG_SC_GIP	(0x01 << 0)

#define BDCREG_CMD_EP_TYPE_CTRL (0x00 << 22)
#define BDCREG_CMD_EP_TYPE_ISOC (0x01 << 22)
#define BDCREG_CMD_EP_TYPE_BULK (0x02 << 22)
#define BDCREG_CMD_EP_TYPE_INTR (0x03 << 22)

#define BDCREG_CMD_MAX_BURST(p)		((p) << 6)
#define BDCREG_CMD_MAX_PACKET(p)	((p) << 10)
#define BDCREG_CMD_EP_INTERVAL(p)	(p)
#define BDCREG_CMD_EP_MULT(p)		((p) << 4)

/* 5.4 Upstream Port Register Set */
#define BDCREG_USPSC_VBC	(1 << 31)
#define	BDCREG_USPSC_PRC	(1 << 30)
#define BDCREG_USPSC_PCE	(1 << 29)
#define BDCREG_USPSC_CFC	(1 << 28)
#define	BDCREG_USPSC_PCC	(1 << 27)
#define	BDCREG_USPSC_PSC	(1 << 26)
#define BDCREG_USPSC_VBUS	(1 << 25)
#define BDCREG_USPSC_PRS	(1 << 24)
#define BDCREG_USPSC_PCS	(1 << 23)
#define BDCREG_USPSC_PSP(p) (((p) & (0x7 << 20)) >> 20)
#define	BDCREG_USPSC_SCN	(1 << 8)
#define	BDCREG_USPSC_SDC	(1 << 7)
#define	BDCREG_USPSC_SWS	(1 << 4)
#define BDCREG_USPSC_PST(p) (p & 0xF)

#define BDCREG_USPSC_USPSC_RW (BDCREG_USPSC_SCN | BDCREG_USPSC_SDC | BDCREG_USPSC_SWS | 0xf)

/* 5.3 BDC Command register */
#define BDCI_CMD_ABT			0xff
#define BDCI_CMD_DNC			0x6
#define	BDCI_CMD_DCO			0x5
#define	BDCREG_CMD_EPO			0x4
#define	BDCI_CMD_BLA			0x3
#define	BDCREG_CMD_EPC			0x2
#define BDCREG_CMD_DEVICE_CONFIGURATION 0x1
#define BDCI_CMD_NOP			0x0

#define BDCREG_CMD_CSA_PRESERVE_TOGGLE	(1 << 15)
#define BDCREG_CMD_CSA_RESET_TOGGLE	(2 << 15)
#define BDCREG_CMD_CSA_XSD		(3 << 15)

#define BDCREG_SUB_CMD_EP_ADD		(0x1 << 17)
#define BDCREG_SUB_CMD_EP_DROP		(0x2 << 17)
#define BDCREG_SUB_CMD_EP_STOP		(0x2 << 17)
#define BDCREG_SUB_CMD_EP_STALL		(0x4 << 17)
#define BDCREG_SUB_CMD_EP_STATUS	(0x5 << 17)
#define BDCREG_SUB_CMD_EP_RESTORE	(0x6 << 17)
#define BDCREG_SUB_CMD_EP_RESET		(0x1 << 17)

#define BDCREG_SUB_CMD_DEVICE_ADDRESS	(0x1 << 17)
#define BDCREG_SUB_CMD_DEVICE_PARAMETER (0x2 << 17)

#define	BDCREG_CMD_CWS (1 << 5)
#define	BDCREG_CMD_CST(p)	((p) & (0xf << 6))
#define BDCREG_CMD_EPN(p)	((p) << 10)
#define	BDCI_CMD_EP_SNR_RST	(0x2 << 15)
#define	BDCI_CMD_EP_SNR		(0x1 << 15)
#define	BDCI_CMD_EP_STP		(0x1 << 15)
#define	BDCI_CMD_EP_STP_SS	(0x2 << 15)
#define BDCI_SUB_CMD_ADD	(0x1 << 17)
#define BDCI_SUB_CMD_PAR	(0x2 << 17)

#define BDCREG_CMD_CST_IDLE	(0x0 << 6)
#define BDCI_CMDS_SUCC		0x1
#define BDCI_CMDS_UNKN		0x2
#define BDCI_CMDS_PARA		0x3
#define BDCI_CMDS_STAT		0x4
#define BDCI_CMDS_FAIL		0x5
#define BDCI_CMDS_INTL		0x6
#define BDCI_CMD_ABTD		0x7
#define BDCREG_CMD_CST_BUSY	(0xF << 6)
#define BDCREG_CMD_CID(p) (((uint32)(p)) << 28)
#define BDCREG_CMD_MAX_MASK (0xF)

#define BDCREG_PORT_STATE_WRITE_STROBE (1 << 4)

#define BDCREG_PORT_STATE 0xF
#define BDCREG_get_link_state(value)	((value) & BDCREG_PORT_STATE)
#define BDCREG_to_port_link_state(value)	(value)

typedef enum _bdc_link_state {
	/* In Super_speed */
	BDC_LINK_STATE_U0 = 0x00,
	BDC_LINK_STATE_U1 = 0x01,
	BDC_LINK_STATE_U2 = 0x02,
	BDC_LINK_STATE_U3 = 0x03,
	BDC_LINK_STATE_SS_DIS = 0x04,
	BDC_LINK_STATE_RESUME = 0x0F
} bdc_link_state;

/* Fields from usb30d_shim_intr_status, ena, mode */
#define USB30D_SHIM_INTR_BDC		(0x1)
#define USB30D_SHIM_INTR_BDC_INT_UNUSED (0x2)
#define USB30D_SHIM_INTR_BUS_RESET	(0x4)
#define USB30D_SHIM_INTR_SYSTEM_ERR	(0x8)
#define USB30D_SHIM_INTR_MFI_UPDATE	(0x10)
#define USB30D_SHIM_INTR_AUX_LITE	(0x20)

/* Fields from usb30d_shim_dev_control */
#define USB30D_SHIM_DEV_CTRL_BDC_STRAP		(0x1)
#define USB30D_SHIM_DEV_CTRL_RMT_WAKEUP		(0x2)
#define USB30D_SHIM_DEV_CTRL_BUSPWR		(0x4)
#define USB30D_SHIM_DEV_CTRL_VBUS_PRESENT	(0x8)
#define USB30D_SHIM_DEV_CTRL_VDD_RETENTION	(0x10)


/* Section for indirect access to internal registers */
#define BDC_IRAADR_OFFSET	(0xF98)
#define BDC_IRADAT_OFFSET	(0xF9C)
#define BDC_IRA_BASE_OFFSET	(0x2800)

#define BDC_IRA_STATUS		(0x98)
#define BDC_IRA_STATUS_LPM_RWE	(1 << 16)

#define BDC_IRA_MACTMR0			(0x9c)
#define BDC_IRA_MACTMR0_TDRSMUP_MASK	(0xf)

#endif /* _BDCREG_H_ */
