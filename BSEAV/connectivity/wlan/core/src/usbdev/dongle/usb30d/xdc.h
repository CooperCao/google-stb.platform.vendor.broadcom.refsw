/*
* xDC device controller driver
* Broadcom Proprietary and Confidential. Copyright (C) 2017,
* All Rights Reserved.
* 
* This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
* the contents of this file may not be disclosed to third parties, copied
* or duplicated in any form, in whole or in part, without the prior
* written permission of Broadcom.
*
* $Id: xdc.h 406060 2013-06-05 23:13:52Z $
*/

/**
 * Firmware operates the xDCI controller through:
 * - Data structures established in system memory:
 *     - Contexts (DVX (device context) with SLX (slot context) and EPXs (end point context))
 *         - DVX is an array of 32 entries. Software may alter the contents of DVX indirectly
 *           through commands.
 *         - SLX contains information relating to control, state, address and power management.
 *         - EPX defines the configuration of an EP such as EP Number, EP Type, and bandwidth
 *           related information (Maximum Packet Size, Maximum Burst Size, etc).
 *     - Commands in CR (Command Ring), issued by firmware to the controller. Read-only by
 *       controller.
 *     - TDs in TR (Transfer Ring). A Work Item on a Transfer Ring is called a Transfer Descriptor
 *       (TD) and is comprised of one or more Transfer TRB data structures. Firmware is the producer
 *       of all Transfer TRBs and the controller is the consumer.
 *     - Events in ER (Event Ring), emitted by the controller to firmware. Command completion,
 *       transfer completion or controller level events.
 *     - Device Context Base Address Array (DCBAA), an array of 2 entries. DCBAA[0] points to the
 *       Scratchpad Buffer (SPB) of the controller. DCBAA[1] contains the address of the Device
 *       Context (DVX).
 * - Registers
 *     - Constants and parameters in CP (Capability register set)
 *     - Control and status in OR (Operational register set). Used during configuration and
 *       operation.
 *     - Events and interrupts in RT (Runtime register set). Microframe index reading, ER access
 *       and interrupt manipulation.
 *     - Doorbells in DB (Doorbell array). Two registers are defined in this group: doorbell to
 *       address the CR (DBLCMD) and that for calling EPs of the device.
 *     - Extended capabilities in EC
 *
 * Ring entries (commands, TDs, events) are TRBs (Transfer Request Block). A TRN is in fact a list
 * of Buffer Descriptors (BDs) for typical scatter/gather DMA.
 *
 * There are only two entries in the Doorbell Register Array.  One entry (register DBLCMD) is
 * designated to the Command Ring (CR); whereas the other (register DBLDVX) is the doorbell for
 * various endpoints of the device.
 */


#ifndef _xdc_h_
#define	_xdc_h_

#include <typedefs.h>
#include <siutils.h>

#define XDC_ERROR_VAL	0x0001
#define XDC_TRACE_VAL	0x0002
#define XDC_DBG_VAL	0x0004
#define XDC_DBG_LTSSM   0x0008

#define DL_GO		2

extern uint32 xdc_msg_level;	/**< Print messages even for non-debug drivers */

#ifdef xBCMDBG
#define xdc_err(args)	do {if (xdc_msg_level & XDC_ERROR_VAL) printf args;} while (0)
#define XDC_DEBUG
#define xdc_trace(args)	do {if (xdc_msg_level & XDC_TRACE_VAL) printf args;} while (0)
#define xdc_dbg(args)   do {if (xdc_msg_level & XDC_DBG_VAL) printf args;} while (0)
#else
#undef XDC_DEBUG
#define xdc_err(args)	do {if (xdc_msg_level & XDC_ERROR_VAL) printf args;} while (0)
#define xdc_trace(args)
#define xdc_dbg(args)
#endif /* BCM_DBG */

/** LTSSM = Link Training and Status State Machine */
#define xdc_dbg_ltssm(args) do {if (xdc_msg_level & XDC_DBG_LTSSM) printf args;} while (0)

#ifdef BCMUSBDEV_COMPOSITE
#define MAX_EPNUM	8
#else
#define MAX_EPNUM	4
#endif /* BCMUSBDEV_COMPOSITE */

#define EP_MAX_IDX	(MAX_EPNUM*2+1)

#define XDC_WAR_REG	0x180071E4
#define XDC_WAR_REG0	0x18006FD0
#define XDC_WAR_REG1	0x18006FD4
#define XDC_WAR_REG2	0x18006FD8
#define XDC_WAR_REG3	0x18006FDC
#define XDC_WAR_REG4	0x18006FE0
#define XDC_WAR_REG5	0x18006FE4
#define XDC_WAR_REG6	0x18006FE8
#define XDC_WAR_REG7	0x18006FEC
#define XDC_WAR_REG8	0x18006FF0
#define XDC_WAR_REG9	0x18006FF4
#define XDC_WAR_REG10	0x18006FF8
#define XDC_WAR_REG11	0x18006FFC


/* This is should be added to usbstd.hi */
enum usb_device_speed {
	USB_SPEED_UNKNOWN = 0,	/* enumerating */
	USB_SPEED_LOW,		/**< usb 1.1 */
	USB_SPEED_FULL,		/**< usb 1.1 */
	USB_SPEED_HIGH,		/**< usb 2.0 */
	USB_SPEED_SUPER		/**< usb 3.0 */
};

/** States of a software based state machine: control pipe */
enum xdc_ctrl_state {
	CTRL_PIPE_IDLE = 0,
	WAIT_STATUS_START,
	WAIT_DATA_START,
	WAIT_DATA_XMIT,
	WAIT_STATUS_XMIT,
	CTRL_PIPE_PENDING
};

/** States of a software based state machine: device state */
enum xdc_device_state {
	XDC_STATE_CONNETED = 0,
	XDC_STATE_DEFAULT,
	XDC_STATE_ADDRESS,
	XDC_STATE_CONFIGURED,
	XDC_STATE_DYING,
	XDC_STATE_HALTED
};

typedef volatile struct {
		/* Device Control */
		uint32 devcontrol;   /* DevControl 0x000 */
		uint32 devstatus;	  /* DevStatus 0x004 */
		uint32 PAD0[16];
		uint32 intrstatus;   /* InterruptStatus 0x048 */
		uint32 intrmaskwlan; /* InterruptMaskWLAN 0x04c */
		uint32 intrmaskbt;   /* InterruptMaskBT    0x050 */
		uint32 mficount;	  /* MfiCount	   0x054 */

		uint32 PAD1[98];
		uint32 clkctlstatus; /* ClkCtrlStatus	 0x01E0 */
		uint32 workaround;
		uint32 PAD2[62];
		uint32 hsicphyctrl1;  /* HSICPhyCtrl1	0x2e0 */
		uint32 hsicphyctrl2;  /* HSICPhyCtrl1	0x2e4 */
		uint32 hsicphystat1;  /* HSICPhyState	0x2e8 */
		uint32 PAD3[5];
		uint32 phybertctrl1; /* phyBertCtrl1 0x300 */
		uint32 phybertctrl2; /* phyBertCtrl2 0x304 */
		uint32 phybertstat1; /* phyBertState1 0x308 */
		uint32 phybertstat2; /* phyBertState2 0x30C */
		uint32 phyutmictl1; /* phyUtmiCtl1 0x310 */
		uint32 phytpctl1;   /* phyTpCtl1	  0x314 */
		uint32 usb30dgpiosel;	  /* Usb30dGPIOSel	 0x318 */
		uint32 usb30dgpiooe;	  /* Usb30dGPIOoe	 0x31C */
		uint32 usb30dmdioctl;	  /* Usb30dMdioCtl	  0x320 */
		uint32 usb30dmdiorddata; /* Usb30MdioRdData 0x324 */
		uint32 phymiscctrl; /* phyMiscCtrl  0x328 */
		uint32 PAD4[1];
		uint32 xdcdebugsel1;	  /* xdcDebugSel1	  0x330 */
		uint32 usb30dgpioout;	  /* Usb30dGPIOout	 0x334 */
} usb_xdc_shim_regs_t;

#define B_0				0x01
#define B_1				0x02
#define B_2				0x04
#define B_3				0x08
#define B_4				0x10
#define B_5				0x20
#define B_6				0x40
#define B_7				0x80
#define B_8				0x100
#define B_9				0x200
#define B_10				0x400
#define B_11				0x800
#define B_12				0x1000
#define B_13				0x2000
#define B_14				0x4000
#define B_15				0x8000
#define B_16				0x10000
#define B_17				0x20000
#define B_18				0x40000
#define B_19				0x80000
#define B_20				0x100000
#define B_21				0x200000
#define B_22				0x400000
#define B_23				0x800000
#define B_24				0x1000000
#define B_25				0x2000000
#define B_26				0x4000000
#define B_27				0x8000000
#define B_28				0x10000000
#define B_29				0x20000000
#define B_30				0x40000000
#define B_31				0x80000000

#define XDC_EXT2REG_BASE		0x2800
#define EP_EXT2REG_OFFSET		0x00
#define XDC_EXTREG_BASE			0xF800
#define EP_EXTREG_OFFSET		0x400
#define EP_SEQN_MASK			0x7C0
#define EP_SEQN_BITS			6
#define EP_SEQN_LOAD			B_5
#define EP_STATUS_ACTIVE		B_0
#define EP2_SEQN_BITS			3

/**
 * InterruptStatus(offset 0x048) and InterruptMaskWLAN fields.
 * Bits 0 - 15: interrupts from xDC Device Core; corresponding to the event rings per interface.
 * Fore core rev 0, 4 interfaces were defined, therfore bits (15:4) are not used.
 */
#define XDC_HSE_INTR			B_16 /**< Host system error */
#define XDC_MFI_UPDATE			B_17 /**< micro frame index */
#define XDC_VBUS_LOST			B_18 /**< Voltage on Bus. To detect if host disconnected. */
#define XDC_VBUS_BACK			B_19
#define XDC_SSRSTINTR			B_20 /**< host issued superspeed (usb30) reset */
#define XDC_HSRSTINTR			B_21 /**< host issued high speed (usb20) reset */
#define XDC_ALHSTRESUMEINTR		B_22 /**< asserted by auxlite when U3.exit LFPS detected */

/* CoreControl register in DMP  0x18106408 */
#define CCTRL_MAINCLK_EN		B_0 /**< Enables all the main functional clocks e.g. phy */
#define CCTRL_FORCE_GTCLK_ON		B_1 /**< all the clock gates will be enabled */
#define CCTRL_AUXCLK_EN			B_2 /**< enables the aux clock to the xdc */
#define CCTRL_NDR_EN			B_3 /**< 1 = PHY will not drive bus upon reset */
#define CCTRL_SPEED			(B_5|B_6|B_7) /**< indicates FS/HSIC/USB2/USB3 */
#define CCTRL_SPEED_SHIFT		5

/** UTMI is the interface to the USB 2.0 (LS/FS) PHY. Related to register 'phyutmictl1'. */
#define UTMI_PHYPT_DISABLE		B_15
#define UTMI_RXSYNC_DETLEN_MASK	(B_14|B_13|B_12)
#define UTMI_PHY_IDDQ			B_3
#define UTMI_PHY_ISO			B_2
#define UTMI_PLL_RESETB			B_0

#define USB3_PIPE_RESETB		B_11

#define DEV_CTRL_DEVRESET		B_0
#define DEV_CTRL_PHYRESET		B_12

/** related to chipcommon core 'pllcontrol_data' register */
#define PLL_SEQ_START			B_28
#define PLL_REFCLK40_MODE		B_25
#define PLL_CTRL_UPDATE			B_10

#define XDC_MDIO_SLAVE_ADDR		0x0A

/** MDIO addresses. MDIO registers control USB PHY. */
#define XDCI_MDIO_BASE_REG		0x1F
#define XDCI_MDIO_PLL30_BLOCK		0x8000
#define XDCI_MDIO_RXPMD_BLOCK		0x8020
#define XDCI_MDIO_TXPMD_BLOCK		0x8040
#define XDCI_MDIO_PIPE_BLOCK		0x8060
#define XDCI_MDIO_AFE30_BLOCK		0x8080
#define XDCI_MDIO_UTMI_BLOCK		0x80A0
#define XDCI_MDIO_AFE20_BLOCK		0x80C0
#define XDCI_MDIO_AEQ_BLOCK		0x80E0
/* XDCI_MDIO_TXPMD_BLOCK */
#define XDCI_MDIO_TXPMD_REV_ID_REG		0x00
#define XDCI_MDIO_TXPMD_GEN_CTRL_REG		0x01
#define XDCI_MDIO_TXPMD_FREQ_CTRL1_REG		0x02
#define XDCI_MDIO_TXPMD_FREQ_CTRL2_REG		0x03
#define XDCI_MDIO_TXPMD_FREQ_CTRL3_REG		0x04
#define XDCI_MDIO_TXPMD_FREQ_CTRL4_REG		0x05
#define XDCI_MDIO_TXPMD_FREQ_CTRL5_REG		0x06
#define XDCI_MDIO_TXPMD_FREQ_CTRL_STAT1_REG	0x07
#define XDCI_MDIO_TXPMD_FREQ_CTRL_STAT2_REG	0x08
#define XDCI_MDIO_TXPMD_FREQ_CTRL_STAT3_REG	0x09
#define XDCI_MDIO_TXPMD_FREQ_CTRL_STAT4_REG	0x0A
#define XDCI_MDIO_TXPMD_HSPMD_STAT1_REG		0x0B
/* XDCI_MDIO_TXPMD_GEN_CTRL_REG fields */
#define XDCI_MDIO_TXPMD_GEN_CTRL_tx_ssc_en_frc		B_0
#define XDCI_MDIO_TXPMD_GEN_CTRL_tx_ssc_en_frc_val	B_1
#define XDCI_MDIO_TXPMD_GEN_CTRL_tx_pmd_en_frc		B_2
#define XDCI_MDIO_TXPMD_GEN_CTRL_tx_pmd_en_frc_val	B_3
#define XDCI_MDIO_TXPMD_GEN_CTRL_tx_phase_rotate_step	B_4
#define XDCI_MDIO_TXPMD_GEN_CTRL_tx_phase_rotate_dir	B_5
#define XDCI_MDIO_TXPMD_GEN_CTRL_tx_phase_rotate_trig	B_6
#define XDCI_MDIO_TXPMD_GEN_CTRL_tx_phase_rotate_tmenab	B_7
#define XDCI_MDIO_TXPMD_GEN_CTRL_tx_phs_stp_inv		B_8
#define XDCI_MDIO_TXPMD_GEN_CTRL_tx_host_track_en	B_9
#define XDCI_MDIO_TXPMD_GEN_CTRL_hstr_filt_bypass	B_10
#define XDCI_MDIO_TXPMD_GEN_CTRL_ana_farend_lpbk	B_11
#define XDCI_MDIO_TXPMD_GEN_CTRL_tx_phact_8b4bn		B_12

struct usbdev_chip {
		void *drv;			/**< Driver handle */
		uint id;			/**< USB device core ID */
		uint rev;			/**< USB device core revision */
		osl_t *osh;			/**< os (Driver PCI) handle */
		usb_xdc_shim_regs_t *regs; 	/**< USB registers */
		si_t *sih;			/**< SiliconBackplane handle */
		struct dngl_bus *bus;		/**< high level USB bus logic handle */

		struct xdcd *xdc;

		unsigned char up;
		unsigned char suspended; 	/**< Device is USB suspended */
		unsigned char remote_wakeup;
		unsigned char self_power;

		unsigned char pwmode;
		unsigned char status_delayed;
		unsigned char disconnect;	/**< 1=device re-enumerates at init time */
		unsigned char non_disconnect;	/**< not re-enumerate after FW download */
		volatile uint32 dl_go;
		uint16 addr;
		uint8 reconnect;
		uint8 speed;

		uint32 device;
		void *mem;

		uint8	remote_wakeup_issued;	/**< for U3 remote wakeup WAR */
		uint32 otp_u1u2;
};

/**
 * Layout of XDC Capability registers. This set of registers is read-only (RO), specifying
 * implementation related parameters and features.
 */
struct xdc_cpregs {
	uint32 cpliver;
	uint32 sparams1;
	uint32 sparams2;
	uint32 sparams3;
	uint32 cparams;
	uint32 db_off;
	uint32 rtreg_off;
	uint32 rsv;
};

/** Capability register */
#define XDC_CAP_LEN(p)		(((p)>> 0) & 0x00ff)
#define XDC_CAP_ITC(p)		(((p)>> 8) & 0xf)	/**< Interface spec: xHCI, xDCI, etc */
#define XDC_CAP_VER(p)		(((p)>> 16) & 0xffff)
#define XDC_CAP_MSLS(p)		(((p)>> 0) & 0xff) /** #entries in Device Ctx and Doorbell Arrays */
#define XDC_CAP_MITS(p)		(((p)>> 8) & 0x7ff) /**< max number of interrupts */
#define XDC_CAP_NPTS(p)		(((p)>> 24) & 0xff) /**< number of ports (should be 1) */

#define XDC_CAP_IST(p)		(((p)>> 0) & 0xf)
#define XDC_CAP_ERST_MAX(p)	(((p)>> 4) & 0xf)
#define XDC_CAP_MSPBH(p)	(((p)>> 21) & 1f)
#define XDC_CAP_MSPBL(p)	(((p)>> 27) & 1f)
#define XDC_CAP_SPR(p)		(((p)>> 26) & 0x1) /**< Scratchpad Restore */
#define XDC_SCPAD_NUM(p)	(((p) >> 27) & 0x1f)
#define XDC_CAP_U1L(p)		(((p)>> 0) & 0xff)
#define XDC_CAP_U2L(p)		(((p)>> 16) & 0xff)

#define XDC_CAP_64BIT_ADDR(p)  ((p) & (1 << 0))
#define XDC_CAP_64B(p)  ((p) & (1 << 2))
#define XDC_CAP_LIGHT_RST(p)    ((p) & (1 << 5))
#define XDC_CAP_LTC(p)     ((p) & (1 << 6))
#define XDC_CAP_NSS(p)     ((p) & (1 << 7))
#define XDC_CAP_PAE(p)     ((p) & (1 << 8))
#define XDC_CAP_MPSA(p)    (1 << ((((p) >> 12) & 0xf) + 1)) /**< Max size of primary stream array */
#define XDC_CAP_XECP(p)    (((p)>>16)&0xffff) /**< Extended capability pointer in DW */


#define CMD_RING_ALIGN_BITS      (0x3f)

#define DBOFF_MASK      (~0x3)
#define RT_REGOFF_MASK     (~0x1f)

/**< endpoint state */
#define XDC_EP_ENABLED		(1 << 0)
#define XDC_EP_STALL		(1 << 1)
#define XDC_EP_BUSY		(1 << 4)

/**
 * XDC operational register set. These registers are for USB device controller configuration and
 * operation. Written by software at run time, they specify the rings (TR, CR and ER). The Upstream
 * Facing Port (USP) is also operated through registers of this group.
 */
struct xdc_opregs {
	uint32 command;
	uint32 status;
	uint32 page_size;
	uint32 rsv1;
	uint32 rsv2;
	uint32 rsv3;
	uint32 cmd_ring[2];
	uint32 rsv4[4];
	uint32 dcbaa[2]; /**< Device context base address array */
	uint32 rsv5[242];
	uint32 portsc;   /**< Port Status and Controller Register */
	uint32 portpw;
	uint32 portlk;
	uint32 portdbg;
	uint32 rsv6;
	uint32 portpw20;
	uint32 rsv7[1020];
};

/* Internal / indirect registers */
#define XDC_INDIRECT_ADDR		0x18006FA8
#define XDC_INDIRECT_DATA		0x18006FAC
#define XDCI_IRA_WA_REG			0xFA8
#define XDCI_IRA_RD_REG			0xFAC
#define XDCI_SSLL_BASE_ADDR		0x1800
#define XDCI_SSLL_SUSPSC		0x1800 /**< Status & Control, maps to opregs->portsc */
#define XDCI_SSLL_SUSPLI		0x1808 /**< Link Error Count */
#define XDCI_SSLL_SUSPPM		0x180C /**< Power Management */
#define XDCI_SSLL_SUSPTD		0x1810 /**< Test & Debug, maps to opregs->portdbg */
#define XDCI_SSLL_STC0			0x1814 /**< Timer Config 0 */
#define XDCI_SSLL_STC1			0x1818 /**< Timer Config 1 */
#define XDCI_SSLL_PORTSC		0x181C /**< Port status */
#define XDCI_SSLL_DEBUG_CTRL0		0x1850 /**< Debug Control 0 */

/* XDCI_SSLL_SUSPTD fields, also used for opregs->portdbg */
#define XDCI_SSLL_SUSPTD_BERT_CMD_MASK			0xF
#define XDCI_SSLL_SUSPTD_BERT_CMD_SHIFT			0
#define XDCI_SSLL_SUSPTD_BERT_CMD_STROBE		B_4
#define XDCI_SSLL_SUSPTD_ENTER_LOOPBACK			B_5
#define XDCI_SSLL_SUSPTD_LOOPBACK_MODE_MASK		(B_6|B_7)
#define XDCI_SSLL_SUSPTD_LOOPBACK_MODE_SHIFT		B_6
#define XDCI_SSLL_SUSPTD_DISABLE_SCRAMBLING		B_8
#define XDCI_SSLL_SUSPTD_DISABLE_SCRAMBLING_STROBE	B_9
#define XDCI_SSLL_SUSPTD_FAST_SIM_MODE			B_10
#define XDCI_SSLL_SUSPTD_NO_SCRAMBLE_CHANGE		B_11
#define XDCI_SSLL_SUSPTD_IGNORE_DPP_DURING_RETRY	B_12
#define XDCI_SSLL_SUSPTD_LMP_TIMER_IN_U0		B_13
#define XDCI_SSLL_SUSPTD_PLL_OFF_DURING_U2		B_14
#define XDCI_SSLL_SUSPTD_RXHPB_DPH_FIX			B_15
#define XDCI_SSLL_SUSPTD_RXHPB_OPT			B_16
#define XDCI_SSLL_SUSPTD_RXHUB_RESET			B_17
/* reserved						B_18 */
#define XDCI_SSLL_SUSPTD_PIPE_RESET			B_19
#define XDCI_SSLL_SUSPTD_FORCE_PWRDN_P2			B_20
#define XDCI_SSLL_SUSPTD_CLR_PHYSTS_TIMER		B_21
#define XDCI_SSLL_SUSPTD_NO_PIPERST_RXDETECT		B_22
#define XDCI_SSLL_SUSPTD_FAST_LP2_EXIT			B_23
#define XDCI_SSLL_SUSPTD_PIPERST_POLLING		B_24
#define XDCI_SSLL_SUSPTD_RXD_REQ_LEVEL			B_25
#define XDCI_SSLL_SUSPTD_FAST_LP1_EXIT			B_26
#define XDCI_SSLL_SUSPTD_DPH_RLS_DLY			B_27
#define XDCI_SSLL_SUSPTD_LFPS_GLITCH_TOLERANCE		B_28
#define XDCI_SSLL_SUSPTD_PIPERST_RECOVERY		B_29
/* reserved						B_30 */
#define XDCI_SSLL_SUSPTD_LFPSPAR_ON_DURING_PLLOFF	B_31

#define XDCI_20MAC_BASE_ADDR		0x2800

/* DEBUGCTL0 and DEVADDR register */

#define XDC_DEVADDR_OFFSET      (0x80)
#define XDC_DEBUGCTL0_OFFSET    (0xDC)
#define XDC_DEVADDR             (XDC_EXT2REG_BASE+XDC_DEVADDR_OFFSET)
#define XDC_DEBUGCTL0           (XDC_EXT2REG_BASE+XDC_DEBUGCTL0_OFFSET)

#define XDC_DEVVALUE            (1)

#define FORCE_UTMIST            (2)
#define UTMIFSMST               (3)
#define UTMIFSMST_MASK          (~(0x3F<<UTMIFSMST))
#define UTMIFSMST_15            (15)

/** xdc->opregs->command register */
#define XDC_USBCMD_RUN    (1 << 0)  /**< Controller Run/Stop */
#define XDC_USBCMD_RST    (1 << 1)  /**< Controller reset - equivalent to POR or hardware reset */
#define XDC_USBCMD_INTE   (1 << 2)  /**< Global interrupt enable */
#define XDC_USBCMD_HSEE   (1 << 3)  /**< Host System Error Enable */
#define XDC_USBCMD_LRST   (1 << 7)  /**< Light controller reset */
#define XDC_USBCMD_CSS    (1 << 8)  /**< Controller Save State */
#define XDC_USBCMD_CRS    (1 << 9)  /**< Controller Restore State */
#define XDC_USBCMD_EWE    (1 << 10) /**< Enable microframe count wrap event */

#define XDC_MAX_PORTS(p)	(((p) >> 24) & 0x7fU)

#define XDC_PORT_SS		(0x4)
#define XDC_PORT_HS		(0x3)
#define XDC_PORT_FS		(0x1)
#define	XDC_PORT_LS		(0x2)
#define XDC_PORT_UNKN		(0x0)

#define	PORT_SPEED_MASK		(0xf << 10)

/** xdc->opregs->status register */
#define XDC_USBSTS_CH		(1 << 0)  /**< Controller Halted */
#define XDC_USBSTS_HSE		(1 << 2)  /**< Host System Error */
#define XDC_USBSTS_EINT		(1 << 3)  /**< Event Interrupt */
#define XDC_USBSTS_PCD		(1 << 4)  /**< Port Change Detected */
#define XDC_USBSTS_SSS		(1 << 8)  /**< Save State Status */
#define XDC_USBSTS_RSS		(1 << 9)  /**< Restore State Status */
#define XDC_USBSTS_SRE		(1 << 10) /**< Save or Restore Error */
#define XDC_USBSTS_CNR		(1 << 11) /**< Controller Not Ready */
#define XDC_USBSTS_CE		(1 << 12) /**< Controller Error */

/** Command Ring Control Lo Registers  */
#define XDC_CRCRL_RCS		(1 << 0) /**< Ring Cycle State */
#define XDC_CRCRL_CS		(1 << 1) /**< Command Stop */
#define XDC_CRCRL_CA		(1 << 2) /**< Command Abort */
#define XDC_CRCRL_CRR		(1 << 3) /**< Command Ring Running */
#define XDC_CRCRL_CRPL		(0xffffffc << 6)


/* PORTSC, also used for XDCI_SSLL_SUSPSC */
#define XDC_PORTSC_CCS		(1 << 0)    /**< Current Connect Status */
#define XDC_PORTSC_PE		(1 << 1)    /**< Port Enable */
#define XDC_PORTSC_PD		(1 << 2)
#define XDC_PORTSC_SR		(1 << 4)    /**< Start of reset */
#define XDC_PORTSC_PLS		(0xf << 5)  /**< Port Link State */
#define XDC_PORTSC_PP		(1 << 9)    /**< Port Power */
#define XDC_PORTSC_PS		(0xf << 10) /**< Port Connection Speed */
#define XDC_PORTSC_LWS		(1 << 16)   /**< Port Link State Write Strobe */
#define XDC_PORTSC_CSC		(1 << 17)   /**< Connect Status Change */
#define XDC_PORTSC_PCE		(1 << 18)   /**< Port Connect Error */
#define XDC_PORTSC_PRC		(1 << 21)   /**< Port Reset Change */
#define XDC_PORTSC_PLC		(1 << 22)   /**< Port Link State Change */
#define XDC_PORTSC_CEC		(1 << 23)   /**< Port Configure Error Change */
#define XDC_PORTSC_PPC		(1 << 24)   /**< Port Power Change */

#define XDC_PORTSC_PLS_SHIFT	(5)

/** Port Power Management Register (PORTPM) */
#define XDC_PORTPM_U1T	(0xff)
#define XDC_PORTPM_U2T	(0xff << 8)
#define XDC_PORTPM_W1S	(1 << 17)
#define XDC_PORTPM_W2S	(1 << 18)
#define XDC_PORTPM_U2A	(1 << 29)
#define XDC_PORTPM_U1E	(1 << 30)
#define XDC_PORTPM_U2E	(1 << 31)

#define USB_2_PORTPM_TESTMODE_SHIFT     28
#define USB_2_PORTPM_TESTMODE_MASK      0xf0000000;

#define XDC_U1_ENABLED			2	/**< transition into U1 state */
#define XDC_U2_ENABLED			3	/**< transition into U2 state */
#define XDC_LTM_ENABLED			4	/**< Latency tolerance messages */
#define XDC_SELF_POWERED		0	/**< (read only) */
#define XDC_REMOTE_WAKEUP		1	/**< dev may initiate wakeup */
#define XDC_TEST_MODE			2	/**< (wired high speed only) */
#define XDC_BATTERY			2	/**< (wireless) */
#define XDC_B_HNP_ENABLE		3	/**< (otg) dev may initiate HNP */
#define XDC_WUSB_DEVICE			3	/**< (wireless) */
#define XDC_A_HNP_SUPPORT		4	/**< (otg) RH port supports HNP */
#define XDC_A_ALT_HNP_SUPPORT		5	/**< (otg) other RH port does */
#define XDC_DEBUG_MODE			6	/**< (special devices only) */

/* PORTPM-SS */
#define XDC_PORTPM_L1S	(0x7 << 0)
#define XDC_PORTPM_RWE	(1  << 3)
#define XDC_PORTPM_HIRD	(0xf << 4)
#define XDC_PORTPM_HLE	(1 << 16)
#define XDC_PORTPM_PTC	(0xf << 28)

#define XDC_PORTSC_RWC	(XDC_PORTSC_CSC | XDC_PORTSC_PCE |\
	XDC_PORTSC_PRC | XDC_PORTSC_PLC | XDC_PORTSC_CEC| XDC_PORTSC_PPC)

/** Possible values of portsc register bitfield */
enum xdc_link_state {
	/* In SuperSpeed */
	XDC_LINK_STATE_U0              = 0x00, /**< HS: enabled */
	XDC_LINK_STATE_U1              = 0x01,
	XDC_LINK_STATE_U2              = 0x02, /**< HS: L1 */
	XDC_LINK_STATE_U3              = 0x03, /**< HS: L2 */
	XDC_LINK_STATE_SS_DIS          = 0x04,
	XDC_LINK_STATE_RX_DET          = 0x05, /**< HS: Disabled */
	XDC_LINK_STATE_SS_INACT        = 0x06,
	XDC_LINK_STATE_POLL            = 0x07,
	XDC_LINK_STATE_RECOV           = 0x08,
	XDC_LINK_STATE_HRESET          = 0x09, /**< HS:USB2 reset */
	XDC_LINK_STATE_CMPLY           = 0x0a,
	XDC_LINK_STATE_LPBK            = 0x0b,
	XDC_LINK_STATE_RESUME          = 0x0f,
	XDC_LINK_STATE_MASK            = 0x0f
};

/** Register subset per ER (registers under xdc->irset) */
struct xdc_irreg {
	uint32 irq_pend; 
	uint32 irq_ctrl;
	uint32 erst_sz;
	uint32 rsv1;
	uint32 erst_base64[2];
	uint32 erst_deq64[2];
};

/* ERST IMAN register bits */
#define ER_INTR_EN		0x02
#define ER_IRQ_PENDING		0x01
#define ER_INT_PENDING(p)	((p) & 0x1)
#define ER_INT_RESET(p)		((p) & 0xfffffffe)
#define ER_INT_ENABLE(p)	((ER_INT_RESET(p)) | 0x2)
#define ER_INT_DISABLE(p)	((ER_INT_RESET(p)) & ~(0x2))

#define ERST_SZ_MASK		(0xffffU << 16)
#define ERST_SEG_NUM		1

#define ERST_DESI_BITS		(0x7)
#define ERST_EHB_BITS		(1 << 3)
#define ERST_PTR_ALIGN          (0xf)
#define DB_TGT(ep, stream)	((((ep) + 1) & 0xff) | ((stream) << 16))


#define ER_IMODI_MASK		(0xffff)
#define ER_IMODC_MASK			(0xffff << 16)

/**
 * Registers in this Run Time register group cover microframe index reading, ER (Event Ring) access
 * and interrupt manipulation.
 */
struct xdc_rtregs {
	uint32 mframe_index;
	uint32 rsv[7];
	struct xdc_irreg irset[128];
};

/**
 * Doorbell register set (currently 2). Doorbell to address the CR (DBLCMD) and that for calling
 * EPs of the device. See registers under &ep->xdc->dba->...
 */
struct xdc_db {
	uint32 db_cmd; /**< related to command ring (CR) */
	uint32 db_dvx; /**< device context */
	uint32 rsv[254];
};

#define XDC_DEVICE_CTX  0x1
#define XDC_INPUT_CTX   0x2

/**
 * Slot context (SLX) contains information relating to control, state, address and power management.
 * It does not refer to a 'time slot', but to a 'USB slot' instead.
 */
struct xdc_slx {
	uint32 info;
	uint32 rsv0;
	uint32 itgt;
	uint32 dev_state;
	uint32 rsv[4];
};

/* SLX */
#define SLX_RST
#define SLX_SPEED		(0xf << 20)
#define SLX_LCE_MASK		(0x1fU << 27U)
#define SLX_LCE(p)		((p) << 27U)   /**< Last Context Entry */
#define SLX_LCE_TO_EP(p)	(((p) >> 27) - 1)
#define SLOT_FLAG		(1 << 0)
#define EP0_FLAG		(1 << 1)

/**
 * An Endpoint Context (EPX) contains configuration and operating conditions about an EP:
 * - Endpoint Type (TYP)
 * - Endpoint State (ES)
 * - Maximum Packet Size (MPS)
 * - Maximum Burst Size (MBS)
 * - Maximum ISO Bursts (MULT)
 * - Service Interval (ITV)
 */
struct xdc_epctx {
	uint32 info1;
	uint32 info2;
	uint32 deq;
	uint32 deqh;
	uint32 rsv[4];
};

#define EPX_EP_TYPE(p)		((p) << 3)
#define ISO_OUT_EP			1
#define BULK_OUT_EP			2
#define INT_OUT_EP			3
#define CTRL_EP				4
#define ISOC_IN_EP			5
#define BULK_IN_EP			6
#define INT_IN_EP			7

/* deq bitmasks */
#define EPX_CYCLE_MASK          (1 << 0)

#define EP_DISABLED       0
#define EP_RUNNING        1
#define EP_HALTED         2
#define EP_STOPPED        3
#define EP_ERROR          4

#define EPX_MULT(p)		(((p) & 0x3) << 8)
#define EPX_ITV(p)		(((p) & 0xff) << 16)		/**< service interval */
#define EPX_ITV2UFRAMES(p)	(1 << (((p) >> 16) & 0xff)

#define EPX_MAX_BURST(p)	(((p)&0xff) << 8)
#define EPX_MPS(p)		(((p)&0xffff) << 16)
#define EPX_MPS_MASK		(0xffff << 16)
#define MPS_FROM_EPX(p)		(((p) >> 16) & 0xffff
#define EPX_SLOT_ID		(0x01 << 24)
#define SLX_ADDR_MASK		(0x7F)

/**
 * The Input Context data structure specifies the endpoints and the operations to be performed on
 * those endpoints by either Configure Endpoint command or Address Device Command.
 */
struct xdc_ctx_icc {
	uint32 drop_ctx_flag;
	uint32 add_ctx_flag;
	uint32 rsv1[6];
};


struct xdc_dca {
	uint32 devctx_ptr0[2];
	uint32 devctx_ptr1[2];
	void *mem;
};

/* XFER Event bit fields */
#define EP_IDX_OF_TRB(p) (((p) >> 16) & 0x1f)

/* Completion Code */
#define	CC_CODE_MASK		(0xff << 24)
#define GET_CC_CODE(p)	(((p) & CC_CODE_MASK) >> 24)
#define CC_SUCCESS		1
#define CC_DB_ERR		2 /**< Data Buffer Error */
#define CC_BABBLE		3 /**< Babble Error */
#define CC_TX_ERR		4 /**< Transaction Error */
#define CC_TRB_ERR		5
#define CC_STALL		6
#define CC_ENOMEM		7 /**< No memory */
#define CC_BW_ERR		8 /**< No Bandwidth */
#define CC_ENO_SLOTS	9
#define CC_STREAM_ERR	10
#define CC_EBAD_SLT		11
#define CC_EBAD_EP		12
#define CC_SHORT_TX		13
#define CC_UNDER_RUN	14
#define CC_OVER_RUN		15
#define CC_VF_FULL		16
#define CC_CTX_INVL		17
#define CC_BW_OVER		18
#define CC_CTX_STATE	19
#define CC_PING_ERR		20
#define CC_ER_FULL		21
#define CC_DEV_ERR		22
#define CC_MISSED_INT	23
#define CC_CMD_STOP		24
#define CC_CMD_ABORT	25
#define CC_STOP			26
#define CC_STOP_INVAL	27
#define CC_DBG_ABORT	28
#define CC_MEL_ERR		29
#define CC_BUFF_OVER	31
#define CC_ENT_LST		32
#define CC_UN_DEF		33
#define CC_STRID_ERR	34
#define CC_2ND_BW_ERR	35
#define	CC_SPLIT_ERR	36

#define CC_SETUP_RECV 	224
#define CC_DATA_START 	225
#define CC_STATUS_START 226

#define LINK_CYCLE	(0x1<<1)

#define EP_INDEX_OF_TRB(p)		((((p) & (0x1f << 16)) >> 16) - 1)
#define	EP_ID_TO_TRB(p)			((((p) + 1) & 0x1f) << 16)

#define SUSPEND_PORT_TO_TRB(p)		(((p) & 1) << 23)

#define MAX_TD_TRBS	32 /**< max number of Transfer Descriptors in one TRB */

/* TRB, a structure in device memory */
#define TRB_LEN(p)	((p) & 0x1ffff)
#define TRB_TDS(p)	((p)<<17)		/**< TD size: number of (chained) TRBs in TD */
#define TRB_ITGT(p)	(((p) & 0x3ff) << 22) 	/**< interrupt target */
#define GET_ITGT(p)	(((p) >> 22) & 0x3ff)
#define TRB_TBC(p)	(((p) & 0x3) << 7)
#define TRB_TLBPC(p)	(((p) & 0xf) << 16)


#define TRB_CYCLE	(1<<0)  /**< This bit marks the EQP of the TR */
#define TRB_ENT		(1<<1)  /**< Evaluate NexT TRB flag */
#define TRB_ISP		(1<<2)  /**< Interrupt on short packet */
#define TRB_NO_SNOOP	(1<<3)
#define TRB_CHAIN	(1<<4)  /**< To form multi-TRB Transfer Descriptors for scatter/gather */
#define TRB_IOC		(1<<5)  /**< Generate interrupt on TRB completion */
#define TRB_IDT		(1<<6)  /**< Immediate Data. 1=DPL/DPH fields contain data, not a pointer */
#define TRB_BEI		(1<<9)  /**< Block Event Interrupt. 1=disable interrupt */
#define TRB_TSP		(B_9)	/**< Transfer State Preserve */
#define TRB_BSR		(B_9)

/* Ctrl xfer TRB special fields */
#define TRB_DIR_IN	(1<<16)
#define	TRB_TX_TYPE(p)	((p) << 16)
#define	TRB_DATA_OUT	2		/**< host->device */
#define	TRB_DATA_IN	3		/**< device->host */

/**
 * A Transfer Request Block is a structure in dongle memory that is read only by the xDC controller.
 * There are command, transfer (xfer), event, 'normal' and 'vendor defined' (VD) TRBs. All TRBs have
 * subtypes.
 */
struct xdc_trb {
	uint32 buf_addr;
	uint32 buf_addrh;
	uint32 status;
	uint32 flags;      /**< contains 'evaluate next', subtype, 'cycle' bits */
};


/* TRB bit mask */
#define	TRB_TYPE_BITMASK	(0xfc00)
#define TRB_TYPE(p)		((p) << 10)

/* TRB type */
#define NORMAL_TRB		1 /**< used for bulk, isochronous and interrupt transfers */
#define SETUP_TRB		2 /**< setup stage for ctrl transfers */
#define DATA_TRB		3 /**< data stage for ctrl transfers */
#define STATUS_TRB		4 /**< status stage for control transfers */
#define ISOC_TRB		5
#define LINK_TRB		6 /**< for sizing and non-contiguous TR and CMD Rings */
#define EVENT_DATA_TRB		7
#define TR_NOOP_TRB		8
#define ENABLE_SLOT_TRB		9
#define DISABLE_SLOT_TRB	10
#define ADDR_DEV_TRB		11
#define CONFIG_EP_TRB		12
#define EVAL_CXT_TRB		13
#define RESET_EP_TRB		14
#define STOP_RING_TRB		15 /**< stop endpoint temporarily so firmware may operate on ring */
#define SET_DEQ_TRB		16 /**< modify DQP field of an endpoint or stream context */
#define RESET_DEV_TRB		17
#define FORCE_EVT_TRB		18 /**< allows a VMM manager to emulate a USB endpoint */
#define NEG_BW_TRB			19
#define SET_LT_TRB			20
#define GET_BW_TRB			21
#define FORCE_HEADER_TRB	22 /**< send a Link Mgt (LMP) or Transaction Pkt (TP) to the host */
#define CMD_NOOP_TRB		23
#define XFER_EVT_TRB		32
#define CMD_COMP_TRB		33
#define PORT_EVT_TRB		34
#define BW_EVT_TRB			35
#define DoorB_EVT_TRB		36
#define XDC_EVT_TRB			37
#define MFIDX_WRAP_TRB		39 /**< occurs roughly once every 2 seconds */
#define STALL_EP_TRB		49
#define SET_OPCMD_TRB		50

#define SEGMENT_SHIFT		10
#define TRB_MAX_BUFF_SIZE	(1 << 16)

/**
 * A TRB Ring is a circular queue of TRBs.
 * - Transfer Rings (TRs) provides data transport to or from the host. There is one to one mapping
 *   between TRs and unidirectional EPs. EP0 is considered bi-directional and associated with a
 *   single TR.
 * - Event Rings (ERs) provided a way for xDC to report its interval events to software, including
 *   Transfer and command completion status, Port status changes, Controller related events and
 *   exceptions.
 * - The Command Ring (CR) allows software to issue commands to xDC for configuration of device and
 *   endpoints.
 * A ring is empty if enqueue pointer == dequeue pointer.
 * TR and CR ring items are produced by firmware and consumed by the xDC controller.
 * ER ring items are produced by the xDC controller and consumed by firmware.
 */
struct xdc_ring {
	struct xdc_trb *trbs;
	/* Enqueue Pointer (EQP), owned by the producer, refers to the logical beginning of valid
	 * entries of a TRB ring. It is the address of the next TRB in the ring available for the
	 * producer to write things into. The producer advances this pointer once this TRB (work
	 * item) is constructed.
	 */
	struct xdc_trb *enqueue;
	/* Dequeue Pointer (DQP), owned by the consumer, refers to the logical end of valid entries
	 * of a TRB ring.  It is the address of the next TRB to be serviced by the consumer.
	 */
	struct xdc_trb *dequeue;
	uint32 cycle;			/**< one bit that toggles between 1 and 0 */
};

/** Records event ring information */
struct xdc_erst {
	uint32 seg_addr64[2]; /**< point to event ring address */
	uint32 seg_sz;
	uint32 rsv;
};

#define	ERST_SIZE	64
#define	ERST_TBLS	1

#define	CMD_TIMEOUT	60000

struct lpm_save {
	uchar	u1sel;
	uchar	u1pel;
	ushort	u2sel;
	ushort	u2pel;
	ushort	rsv;
};

#ifndef USB_EVT_SCHE
#define USB_EVT_SCHE		32
#endif

#define MAX_BULKIN_NUM		4
#define MAX_TXBLK_SIZE 		(1024*4)
#define MAX_RXBLK_SIZE 		(MAXPKTDATABUFSZ)
#define MAX_TRBS_RING		16 /* < 256 */
#define MAX_ISOC_TRBS_TXRING	16
#define MAX_ISOC_TRBS_RXRING	16
#define MAX_INTR_TRBS_TXRING	16
#define MAX_INTR_TRBS_RXRING	16
#define MAX_TRBS_TXRING		USB_NTXD
#define MAX_TRBS_RXRING		USB_NRXD
#define MAX_TRBS_EVTRING	(MAX_TRBS_TXRING + MAX_TRBS_RXRING)
#define MAX_TRBS_CTLRING	16
#define MAX_TRBS_CMDRING	16
#define MAX_RXRSV_TRB		USBBULK_RXBUFS /* < 16 */
#define MAX_RESCH_EVENT		USB_EVT_SCHE /* 8 16 32 <32 */
#define TX_MAX_URB		MAX_TRBS_TXRING
#define RX_MAX_URB		(MAX_RXRSV_TRB + 4)

#define ISOC_MAX_RXRSV_TRB	4 /* < 16 */
#define ISOC_TX_MAX_URB		MAX_ISOC_TRBS_TXRING
#define ISOC_RX_MAX_URB		(ISOC_MAX_RXRSV_TRB + 4)
#define ISOC_MAX_RXBLK_SIZE  	(1024)

#define INTR_MAX_RXRSV_TRB	4 /* < 16 */
#define INTR_TX_MAX_URB		MAX_INTR_TRBS_TXRING
#define INTR_RX_MAX_URB		(INTR_MAX_RXRSV_TRB + 4)
#define INTR_MAX_RXBLK_SIZE  	(1024)


#define EP0_MAX_URB		USBCTL_RXBUFS /* >= 8 */

#define XDC_SUSPD_PIPE_RESET_BIT 29
#define XDC_SUSPD_LFPS_BIT 23

/** USB Request Block */
struct xdc_urb {
	void		*buf;

	uint16		length;
	uint8		trb_num;
	uint8		lbuf;

	uint16		actual;
	int16		trbcc;

	struct xdc_trb *trb;
	struct xdc_urb *next;
};

typedef int (*ep_evt_fn)(struct xdcd *, struct xdc_urb *,
                                struct xdc_trb *,
                                struct xdc_trb *,
                                uint32 ep_index);

typedef void (*ep_done_fn)(struct xdcd *xdc, uint32 ep_index, struct xdc_urb *);

/** USB endpoint */
struct xdc_ep {
	struct xdc_ring *xf_ring;
	struct xdc_urb *deq_urb;
	struct xdc_trb *dequeue;
	void *urb;

	uint8 state;
	uint8 ring_reactive;
	uint8 cycle_bit;
	uint8 num;

	uint8 type;
	uint8 dir;
	uint8 index;
	uint8 intv;

	uint16	maxpacket;
	uint8	maxburst;
	uint8	seqn;

	uint8 ring_sz;
	uint8 flow_ctrl;
	uint16 blk_sz;

	uint8 urb_wr;
	uint8 malloc_fail;
	uint8 max_urb;
	uint8 max_rxrsv_trb;

	ep_evt_fn evt_fn;
	ep_done_fn done_fn;

	struct xdc_urb *urb_head;
	struct xdc_urb *urb_tail;
	void *lb_head;
	void *lb_tail;

	struct xdcd *xdc;
};

typedef	void (*xdc_stp_fn)(struct xdcd *, struct xdc_trb *);
typedef int (*xdc_evt_fn)(struct xdcd *, struct xdc_trb *);

struct xdcd {
	struct xdc_cpregs *cpregs; /**< capability registers */
	struct xdc_opregs *opregs; /**< operation registers */
	struct xdc_rtregs *rtregs; /**< runtime registers */
	struct xdc_irreg *irset;   /**< irq registers */
	struct xdc_db *dba;        /**< doorbell array (size: 2 elements) */
	struct xdc_dca *dcbaa_ptr; /**< Device Context Base Address Array (size: 2 elements) */

	uint32 *sp_tbl;		/**< scratchpad table in host memory */
	uint32 sp_addr;		/**< scratchpad host memory address */
	uint32 sp_num;		/**< number of 32 bits words in scratchpad table */

	uint32 page_size;
	uint32 cparams;		/**< Capability register params */

	struct xdc_erst *er;	/**< Event ring in host memory */
	unsigned int num_er;	/**< #entries in event ring */
	uint32 erst_addr;	/**< Event ring address in host memory */
	unsigned int erst_sz;	/**< Event ring size in host memory */

	struct xdc_ring *cmd_ring;
	struct xdc_ring *event_ring;

	uint32 inctx_size;	/**< Input context size in [bytes] */
	uint8 *inctx;		/**< Input context */
	uint32 inctx_adr;	/**< Input context */

	uint32 outctx_size;	/**< Output context size (also called DVX) in [bytes] */
	uint8 *outctx;		/**< Output context (also called Device Context) */
	uint32 outctx_adr;	/**< Output context */

	struct xdc_ep ep[EP_MAX_IDX]; /**< endpoints */

	uint32 cmd_status;
	uint32 cmd_completion_flag;
	uint32 togo_cmd;
	void (*cmd_callback)(struct xdc_ep *ep);

	xdc_evt_fn event_fn[3];

	xdc_stp_fn setup_fn[3];
	usb_device_request_t setup_pkt;
	void *stppkt_lbuf;	/**< setup packet buffer */
	uint8 *setup_buf;	/**< setup packet buffer */
	uint8 ctrl_xfer;	/**< USB control transfer */
	uint8 ctrl_nak;		/**< avoids download failure with power cycle for some hosts */
	uint8 ep0_in;		/**< Control endpoint device->host */
	uint8 ctrl_pipe_state;

	struct xdc_urb ep0_urb[EP0_MAX_URB];
	struct xdc_urb status_urb;

	uint32 speed;
	struct lpm_save lpm;	/** Link power management */

	uint32 dev;
	uint32 dev_state;

	void *mem;  /**< point to no align mem address */
	osl_t *osh; /**< os (Driver PCI) handle */

	/* define ctx erst size 64 address(2048 + 64)/32 address( 1024 + 32) */
	uint8 trbs_per_evtring;
	uint8 trbs_per_cmdring;
	uint8 trbs_intr_txring;
	uint8 trbs_intr_rxring;

	uint16 intr_max_rxbuf_sz;
	uint8 intr_max_rsv_trbs;
	uint8 trbs_per_ctlring;

	uint8 trbs_per_txring;
	uint8 trbs_per_rxring;
	uint8 trbs_iso_txring;
	uint8 trbs_iso_rxring;

	uint16 max_rsv_trbs;
	uint8 u1u2_enable;
	uint8 rsv;

	uint16 isoc_max_rxbuf_sz;
	uint16 isoc_max_rsv_trbs;

	uint32 max_rxbuf_sz;
	uint32 resch_event;

	struct usbdev_chip *uxdc;

	uint8 test_mode_nr;
	uint8 test_mode;
};

void xdc_set_SeqN(struct xdcd *xdc, int ep_num, int dir, int seqn);

int xdc_init_mem(struct xdcd *xdc);
void xdc_free_mem(struct xdcd *xdc);
int xdc_core_isr(void *ch);
int xdc_start(struct xdcd *xdc);
int xdc_stop(struct xdcd *xdc);
int xdc_init_hw(struct usbdev_chip *uxdc);
int xdc_config_ep(struct xdcd *xdc, struct xdc_ep *ep);

struct xdc_ring *xdc_alloc_ring(struct xdcd *xdc, bool link_trbs, int trbs_num);
void xdc_free_ring(struct xdcd *xdc, struct xdc_ring *ring, int trbs_num);
int xdc_submit_urb(struct xdc_ep *ep, struct xdc_urb *urb, uint32 trb_type, uint32 dir);
void xdc_rx_malloc_chk(struct xdcd *xdc, struct xdc_ep *ep);
int xdc_send_lbuf(struct xdc_ep *ep, void *p);

int xdc_set_address(struct xdcd *, uint32);

uint32 xdc_enable_irq(struct xdcd *xdc);
uint32 xdc_disable_irq(struct xdcd *xdc);

int xdc_event(struct xdcd *xdc, int loop);
uint32 xdc_suspend(struct xdcd *xdcd);
int xdci_resume(struct usbdev_chip *ch);

uint32 xdci_soft_connect(struct xdcd *xdc);
uint32 xdci_soft_disconnect(struct xdcd *xdc);
uint32 xdc_disconnect_event(struct xdcd *xdc);
int xdc_connect_event(struct xdcd *xdc, int reset);

int xdci_connect(struct xdcd *xdc);
int xdci_restore(struct usbdev_chip *uxdc);

int xdc_add_ep_ctx(struct xdcd *xdc, struct xdc_ep *ep);
int xdc_remove_ep_ctx(struct xdcd *xdc, struct xdc_ep *ep);

int xdc_cmd(struct xdcd *xdc, struct xdc_ep *ep, uint32 cmd);

#ifdef XDC_DEBUG
void xdc_dump_irset(struct xdcd *xdc, int set_num);
void xdc_dump_registers(struct xdcd *xdc);
void xdc_dump_run_regs(struct xdcd *xdc);
void xdc_dump_trb_offsets(struct xdcd *xdc, struct xdc_trb *trb);
void xdc_dump_trb(struct xdcd *xdc, struct xdc_trb *trb);
void xdc_dump_segment(struct xdcd *xdc, struct xdc_trb *trbs, uint32);
void xdc_dump_ring(struct xdcd *xdc, struct xdc_ring *ring, uint32);
void xdc_dump_erst(struct xdcd *xdc);
void xdc_dump_cmd_ptrs(struct xdcd *xdc);
void xdc_dump_ring_ptrs(struct xdcd *xdc, struct xdc_ring *ring);
void xdc_dump_ctx(struct xdcd *xdc, uint32 type,
	uint32 start_ep, uint32 last_ep);
void xdc_dump_ctxs(uint32 arg, uint argc, char *argv[]);
void xdc_dump_port(struct xdcd *xdc);
void xdc_dump_ep_rings(struct xdcd *xdc, unsigned int ep_index);
void xdc_dump_pointer(uint32 arg, uint argc, char *argv[]);
void xdc_dump_rings(uint32 arg, uint argc, char *argv[]);
#endif /* XDC_DEBUG */

void xdc_stop_eps(struct xdcd *xdc);
void xdc_incr_deqptr(struct xdcd *, struct xdc_ring *, uint32 ring_sz);
void xdc_enq_is_lktrb(struct xdcd *xdc, struct xdc_ring *ring, uint32 ring_sz);
int xdc_tx_urb(struct xdcd *xdc, struct xdc_ep *ep, void *p);

void xdc_get_new_deq(struct xdc_ep *ep, struct xdc_urb *);

void xdc_noop_trb(struct xdcd *, struct xdc_ring *, struct xdc_urb *);

void xdc_set_dequeue(struct xdc_ep *ep);

uint32 xdc_chk_ring_space(struct xdcd *, struct xdc_ring *, uint32, uint32, uint32);

void xdc_active_rings(struct xdc_ep *ep);

struct xdc_urb *xdc_alloc_rx_urb(struct xdcd *xdc, struct xdc_ep *ep, void *p);
int xdc_set_test_mode(struct xdcd *xdc, int mode);

static inline void xdc_R_REG64(uint32 *ret, const struct xdcd *xdc,
                                uint32 * regs)
{
	uint32 *ptr = regs;
	uint32 val_lo = R_REG(xdc->osh, ptr);
	uint32 val_hi = R_REG(xdc->osh, ptr + 1);

	ret[0] = val_lo;
	ret[1] = val_hi;

	return;
}

static inline void xdc_W_REG64(struct xdcd *xdc,
	const uint32 val_l, const uint32 val_h, uint32 *regs)
{
	uint32 *ptr = (uint32 *) regs;
	uint32 val_lo = val_l;
#ifdef XDC_64BIT
	uint32 val_hi = val_h;
#endif

	W_REG(xdc->osh, ptr, val_lo);
#ifdef XDC_64BIT
	W_REG(xdc->osh, ptr + 1, val_hi);
#endif
}

#ifdef USB_XDCI
extern void xdc_mdio_wreg(struct usbdev_chip *uxdc, uint16 addr, uint16 data);
extern uint16 xdc_mdio_rreg(struct usbdev_chip *uxdc, uint16 addr);
#endif

#endif /* xdc.h */
