/*
 * xDC controller driver
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: xdc_core.c 412056 2013-07-11 18:10:57Z $
 */

#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmdevs.h>
#include <bcmendian.h>
#include <bcmutils.h>
#include <bcmstdlib.h>
#include <sbhnddma.h>
#include <hnddma.h>
#include <hndpmu.h>
#include <osl.h>
#include <hndsoc.h>
#include <sbusbd.h>
#include <proto/ethernet.h>
#include <dngl_bus.h>
#include <usb.h>
#include <usbdev.h>
#include <usbdev_dbg.h>
#include <usbstd.h>
#include <dngl_api.h>
#include <dngl_protocol.h>
#include <sbchipc.h>
#include "xdc.h"
#include "xdc_rte.h"

#ifdef USB_XDCI

static int xdc_phy_pll30_ctrl(struct usbdev_chip *uxdc);
static int xdc_init_phy(struct usbdev_chip *uxdc, bool usb30_ss);
static void xdc_disable_scrambler(struct xdcd *xdc);
static void xdc_set_ssc_wlanfriendly(struct usbdev_chip *uxdc);

int xdc_core_isr(void *ch)
{
	struct usbdev_chip *uxdc;
	struct xdcd *xdc;
	uint32 status;

	uxdc = (struct usbdev_chip *)ch;
	xdc = uxdc->xdc;

	status = R_REG(xdc->osh, &xdc->opregs->status);

	xdc_trace((" xdc_core_intr irq=%x\n", status));


	if (!(status & XDC_USBSTS_EINT)) {
		return 0;
	}

	/* write 1 to clear Event Interrupt */
	status |= XDC_USBSTS_EINT;
	W_REG(xdc->osh, &xdc->opregs->status, status);

	status = R_REG(xdc->osh, &xdc->irset->irq_pend);
	status |= ER_INTR_EN|ER_IRQ_PENDING;
	W_REG(xdc->osh, &xdc->irset->irq_pend, status);

	/* WAR: U2 is still alive when U2E=0 with U2T!=0, monitor it, and clear it */
	/* HW4350-173 for 4350B0/B1 */
	if (xdc->u1u2_enable == 0) {
		status = R_REG(xdc->osh, &xdc->opregs->portpw);
		if (status&XDC_PORTPM_U2T) {
			W_REG(xdc->osh, &xdc->opregs->portpw, XDC_PORTPM_W2S);
		}
	}

	xdc_trace(("return IRQ_HANDLED\n"));
	return 1;
} /* xdc_core_isr */

/**
 * Called after the USB30 core signalled an interrupt to indicate that it posted a new event on the
 * 'event ring' in dongle memory. Event TRBs are used to report events associated with Command Ring
 * (CR), Transfer Rings and a variety of other controller related events (Port Status Change, etc.).
 */
int xdc_event(struct xdcd *xdc, int loop)
{
	struct xdc_trb *event;
	struct xdc_trb *deq, *lst_trb;
	struct xdc_ring *ring;
	uint32 type, update = 0;
	int resch = 0;

	xdc_trace((" xdc_event \n"));

	ring = xdc->event_ring;
	lst_trb = &ring->trbs[xdc->trbs_per_evtring - 1];
	deq = ring->dequeue;

	do {
		event = deq;

		if ((ltoh32(event->flags)&TRB_CYCLE) != ring->cycle) {
			xdc_dbg(("Mismatch in cycle state, no undecode event.\n"));
			break;
		}

		if (update >= xdc->resch_event) {
			resch = 1;
			break;
		}

		type = (event->flags & TRB_TYPE_BITMASK) >> 10;
		xdc_dbg(("TRB TYPE ID = %d\n", type));

		if (type >= XFER_EVT_TRB && type <= PORT_EVT_TRB) {
			/* calls back xdc_xfer_event, xdc_cmd_event, xdc_port_event */
			xdc->event_fn[type - XFER_EVT_TRB](xdc, event);
		} else {
			if (type == XDC_EVT_TRB) {
				xdc_err(("HC EVT = %d\n", GET_CC_CODE(ltoh32(event->status))));
				ASSERT(0);
				while (1);
			} else {
				xdc_err(("unknown evt = %d \n", type));
			}
		}

		xdc_dbg(("Update Evnet Ring"));

		update++;

		if (deq == lst_trb) {
			ring->cycle ^= 0x01;
			deq = ring->trbs;
		} else {
			deq++;
		}
	} while (loop);

	ring->dequeue = deq;

	if (update) {
		update = (uint32)&ring->dequeue->buf_addr;
		/* update &= (~ ERST_PTR_ALIGN); no need to check? */
		update |= ERST_EHB_BITS;

		xdc_W_REG64(xdc, update, 0, &xdc->irset->erst_deq64[0]);
	}

	return resch;
} /* xdc_event */

/**
 * Dongle -> host direction
 *     @param[in] ep_index endpoint index to use
 *     @param[in] p        packet to send to host
 * Returns TRUE on success.
 */
int xdc_tx(void *ch, int ep_index, void *p)
{
	struct usbdev_chip *uxdc;
	struct xdc_ep *ep;
	int ret = 0;

	uxdc = (struct usbdev_chip *)ch;

	if (p == NULL) {
		if (ep_index == 0) {
			xdc_send_NAK(uxdc->xdc);
		}
		return 0;
	}

	xdc_trace(("xdc_tx\n"));
	ep = &uxdc->xdc->ep[ep_index];

#ifndef XDC_INT_XFER
	if (ep->type == UE_INTERRUPT) {
		PKTFREE(uxdc->osh, p, TRUE);
		return TRUE;
	}
#endif

	if (uxdc->suspended) {
		xdc_err(("ep%d: device suspended", ep_index));
		PKTFREE(uxdc->osh, p, TRUE);
		return FALSE;
	}

	if (ep->lb_tail) {
		PKTSETNEXT(uxdc->osh, ep->lb_tail, p);
		xdc_trace(("ep%d: lb is not empty", ep_index));

	} else {
		ep->lb_head = p;
	}

	ep->lb_tail = pktlast(uxdc->osh, p);

	p = ep->lb_head;

	ret = xdc_send_lbuf(ep, p);

	xdc_trace(("tx done\n"));

	return ret;
} /* xdc_event */

/**
 * Related to multiple TRBs (Transfer Request Blocks) in a single TD (Transfer Descriptor) for tx.
 * There is a limit to the amount of TRBs that can be part of a TD. This function enforces that
 * limit on caller supplied chain of TRBs.
 *
 * @param ep    [in/out] dongle->host endpoint.
 * @param p     [in]     linked list of TRBs destined for the host
 * @param trbs  [in]     number of packets/TRBs on the linked list to iterate
 *
 * At exit, the linked list 'p' has been 'terminated' after 'trbs' items, and ep->lb_head points at
 * the 'next TRB' (p+trbs) of the chain.
 */
static void xdc_trim_lbuf(struct xdc_ep *ep, void *p, uint32 trbs)
{
	struct xdcd *xdc;
	void *p1;

	xdc = ep->xdc;

	p1 = p;
	do {
		ep->lb_head = p1;
		p1 = PKTNEXT(xdc->osh, p1);
		trbs --;
	} while (trbs);

	PKTSETNEXT(xdc->osh, ep->lb_head, NULL);
	ep->lb_head = p1;
}

/**
 * Transmits one or more TRBs to the host over the caller supplied end point.
 * @param ep    [in/out] dongle->host endpoint.
 * @param p     [in/out] linked list of TRBs to transmit. Caller looses ownership.
 */
int xdc_send_lbuf(struct xdc_ep *ep, void *p)
{
	struct xdcd *xdc;
	uint32 lbuf_cnt, trbs, tx_threshld;
	int ret = 0;

	 /* Don't do flow control if an invalid pointer is passed */
	if (p == NULL)
		return FALSE;

	xdc = ep->xdc;

	tx_threshld = ep->ring_sz>>2;
	trbs = xdc_chk_ring_space(xdc, ep->xf_ring, EP_RUNNING, 1, ep->ring_sz);
	if (trbs < tx_threshld) {
		usbdev_txstop(xdc->uxdc->bus, ep->index);
		ep->flow_ctrl = TRUE;
	}

	if (trbs < 1) {
		return FALSE;
	}

	lbuf_cnt = pktsegcnt(xdc->osh, p);

	if (lbuf_cnt > MAX_TD_TRBS) {
		lbuf_cnt = MAX_TD_TRBS;
		if (lbuf_cnt > trbs) {
			lbuf_cnt = trbs;
		}
		trbs = lbuf_cnt;
		xdc_trim_lbuf(ep, p, trbs);
	} else {
		/* lbuf_cnt < MAX_TX_TRBS */
		if (lbuf_cnt > trbs) {
			/* available trbs space is smaller than transfer pkts */
			lbuf_cnt = trbs;
			xdc_trim_lbuf(ep, p, trbs);
		} else {
			/* OK, we have enough space to send whole list */
			ep->lb_head = NULL;
			ep->lb_tail = NULL;
		}
	}

	ret = xdc_tx_urb(xdc, ep, p);
	if (ret < 0) {
		if (ep->lb_head == NULL) {
			ep->lb_head = p;
			ep->lb_tail = pktlast(xdc->osh, p);
		} else {
			PKTSETNEXT(xdc->osh, pktlast(xdc->osh, p), ep->lb_head);
			ep->lb_head = p;
		}
		return FALSE;
	}

	return TRUE;
} /* xdc_send_lbuf */

#ifndef LOOP_BACK
void xdc_rx(struct usbdev_chip *uxdc, int ep_index, void *p)
{
	if (p == NULL)
		return;

	usbdev_rx(uxdc->bus, ep_index, p);

	xdc_trace(("xdc_rx done\n"));
}
#endif

/**
 * Transmits one or more URBs to the host over the caller supplied end point. One URB contains one
 * TRB.
 * @param xdc   [in/out]
 * @param ep    [in/out] dongle->host endpoint.
 * @param p     [in/out] linked list of TRBs to transmit. Caller looses ownership.
 */
int xdc_tx_urb(struct xdcd *xdc, struct xdc_ep *ep, void *p)
{
	struct xdc_urb *urb;
	uint32 wr, urb_type, dir;
	int ret = 0;

	wr = ep->urb_wr;

	/* here alloc xdc_urb buffer from the ahead of p->data */
	if (ep->num == 0) {
		urb = &xdc->ep0_urb[wr];

		wr = (wr + 1)%EP0_MAX_URB;
		ep->urb_wr = wr;

		dir = (ep->xdc->ep0_in) ? TRB_DIR_IN : 0;
		urb_type = DATA_TRB;
	} else {
		urb = (struct xdc_urb *)ep->urb + wr;
		if (urb->buf) {
			xdc_err(("urb ovwr wr = %d\n", wr));
			return -1;
		}

		wr = (wr + 1)%ep->max_urb;

		ep->urb_wr = wr;
		dir = 0;
		urb_type = NORMAL_TRB;
#ifdef BCMUSBDEV_COMPOSITE
		if (ep->type == UE_ISOCHRONOUS) {
			urb_type = ISOC_TRB;
		}
#endif /* BCMUSBDEV_COMPOSITE */
	}

	urb->length = PKTLEN(xdc->osh, p);
	urb->actual = 0;
	urb->trb_num = 0;
	urb->lbuf = 1;
	urb->buf = p;
	urb->trb = NULL;

	ret = xdc_submit_urb(ep, urb, urb_type, dir);

	if (ret < 0) {
		urb->buf = NULL;
		urb->trb = NULL;
		if (ep->num) {
			if (ep->urb_wr) {
				ep->urb_wr--;
			} else {
				ep->urb_wr = ep->max_urb - 1;
			}
		}
	}

	return ret;
} /* xdc_tx_urb */

/** starts XDC core operation, called only once during firmware initialization */
int xdc_start(struct xdcd *xdc)
{
	uint32 cmd;
	int ret = 0;
	uint32 timeout = 160000;

	OR_REG(xdc->osh, &xdc->opregs->command, XDC_USBCMD_RUN);

	while (timeout) {
		OSL_DELAY(1);
		cmd = R_REG(xdc->osh, &xdc->opregs->status);
		if ((cmd&XDC_USBSTS_CH) == 0) {
			xdc->dev_state &= ~XDC_STATE_HALTED;
			ret = 0;
			break;
		}
		timeout--;
	}

	if (timeout == 0) {
		ret = -1;
	}

	if (xdc->speed == USB_SPEED_SUPER) {
		/* Need to reduce impact of USB SS interference on WLAN and vice versa */
		xdc_err(("USB3 SS detected, disabling scrambler and enabling WLAN friendly SSC\n"));

		xdc_disable_scrambler(xdc);
		xdc_set_ssc_wlanfriendly(xdc->uxdc);
	}

	return ret;
} /* xdc_start */

static uint32 xdc_mdio_delay(struct usbdev_chip *uxdc, int delay)
{
	uint32 val = 0;

	while (delay--) {
		val = R_REG(uxdc->osh, (uint32 *)(&uxdc->regs->usb30dmdioctl));
	}
	return val;
}

/** write a register of the USB PHY (which supports both USB2 and USB3) */
void xdc_mdio_wreg(struct usbdev_chip *uxdc, uint16 addr, uint16 data)
{
	uint32 val, sel;
	osl_t *osh;
	uint32 delay = 2;

	osh = uxdc->osh;

	sel = (XDC_MDIO_SLAVE_ADDR << USB_MDIOCTL_ID_SHIFT) |
		(USB_MDIOCTL_SMSEL_CLKEN << USB_MDIOCTL_SMSEL_SHIFT);
	W_REG(osh, (uint32 *)(&uxdc->regs->usb30dmdioctl), sel);

	val = (addr << USB_MDIOCTL_REGADDR_SHIFT) | (data << USB_MDIOCTL_WRDATA_SHIFT) | sel;
	val |= USB_MDIOCTL_WR_EN;
	OSL_DELAY(delay);

	W_REG(osh, (uint32 *)(&uxdc->regs->usb30dmdioctl), val);
	/* Wait ~1.1us: 64 clks @ 60Mhz */
	OSL_DELAY(delay);

	xdc_mdio_delay(uxdc, 8);
}

/** read a register of the USB PHY (which supports both USB2 and USB3) */
uint16 xdc_mdio_rreg(struct usbdev_chip *uxdc, uint16 addr)
{
	uint32 val, sel;
	osl_t *osh;
	uint32 delay = 2;

	osh = uxdc->osh;

	sel = (XDC_MDIO_SLAVE_ADDR << USB_MDIOCTL_ID_SHIFT) |
		(USB_MDIOCTL_SMSEL_CLKEN << USB_MDIOCTL_SMSEL_SHIFT);
	W_REG(osh, (uint32 *)(&uxdc->regs->usb30dmdioctl), sel);
	OSL_DELAY(delay);

	xdc_mdio_delay(uxdc, 8);

	val = (uint32)(addr << USB_MDIOCTL_REGADDR_SHIFT) | sel;
	val |= USB_MDIOCTL_RD_EN;
	W_REG(osh, (uint32 *)(&uxdc->regs->usb30dmdioctl), val);
	/* Wait ~1.1us: 64 clks @ 60Mhz */
	OSL_DELAY(delay);
	xdc_mdio_delay(uxdc, 8);

	val = R_REG(osh, (uint32 *)(&uxdc->regs->usb30dmdiorddata));
	OSL_DELAY(delay);

	return val;
} /* xdc_mdio_rreg */

/** disable USB SS scrambler, scrambling affects USB<->WLAN interference */
void xdc_disable_scrambler(struct xdcd *xdc)
{
	/* Request host to disable scrambler, will take effect after link retraining */
	uint32 temp = xdci_indirect_read(xdc, XDCI_SSLL_SUSPTD);
	temp |= (XDCI_SSLL_SUSPTD_DISABLE_SCRAMBLING |
		XDCI_SSLL_SUSPTD_DISABLE_SCRAMBLING_STROBE);
	xdci_indirect_write(xdc, XDCI_SSLL_SUSPTD, temp);

	/* force link retraining */
	temp = xdci_indirect_read(xdc, XDCI_SSLL_SUSPSC);
	temp &= ~XDC_PORTSC_PLS;
	temp |= XDC_PORTSC_LWS | (XDC_LINK_STATE_RECOV << XDC_PORTSC_PLS_SHIFT);
	xdci_indirect_write(xdc, XDCI_SSLL_SUSPSC, temp);
}

/** configure WLAN friendly SSC */
void xdc_set_ssc_wlanfriendly(struct usbdev_chip *uxdc)
{
	uint16 val1;

	/* Select TXPMD */
	xdc_mdio_wreg(uxdc, XDCI_MDIO_BASE_REG, XDCI_MDIO_TXPMD_BLOCK);
	/* Read general control Register */
	val1 = xdc_mdio_rreg(uxdc, XDCI_MDIO_TXPMD_GEN_CTRL_REG);
	/* enable scc */
	val1 |= (XDCI_MDIO_TXPMD_GEN_CTRL_tx_ssc_en_frc |
		XDCI_MDIO_TXPMD_GEN_CTRL_tx_ssc_en_frc_val);
	/* configure ssc */
	/* Frequency Control Register 3, b[0:9]: fixed_fmax */
	/* fmax = 4000 -> round ($fmax / 30.5) = 131 */
	xdc_mdio_wreg(uxdc, XDCI_MDIO_TXPMD_FREQ_CTRL3_REG, 131);
	/* Frequency Control Register 2, b[0:9]: fixed_fmin */
	/* fmin = 2000 -> round ($fmin / 30.5) = 66 */
	xdc_mdio_wreg(uxdc, XDCI_MDIO_TXPMD_FREQ_CTRL2_REG, 66);
	/* Frequency Control Register 1, b[0:15]: Fixed_fstep */
	/* round (52.8 * ($val4 - $val3) = 3432 */
	xdc_mdio_wreg(uxdc, XDCI_MDIO_TXPMD_FREQ_CTRL1_REG, 3432);
	xdc_mdio_wreg(uxdc, XDCI_MDIO_TXPMD_GEN_CTRL_REG, val1);
}

/**
 * PLL30 ONLY BEGIN
 * Initialize PHY PLL30 controls
 * usb30d_init_phy_pll30 for SS, based on 40 MHz reference clock
 *     we will store usbssmdio0=0x1f8000,0x0af401,0x0b08009,0x020303
 *     usbssmdio1=0x040007, 0x070000,0x0c0003,0x0ebe77,
 *     usbssmdio2=0x1f8060,0x040002,0x1f8040,0x011003
 *     into OTP
 */
int xdc_phy_pll30_ctrl(struct usbdev_chip *uxdc)
{
	uint32 len, i, val, j, array_len;
	uint16 addr, data;
	char mdiodesc[24];
	const char *otp_str;
#ifdef BCM4350_SIMULATION
	uint mdio[32] = {0x1f8000, 0x0a6401, 0x0b800c, 0x0c0007, 0x020303, 0x040007,
		0x070000, 0x0c0003, 0x0eb277, 0x1f8060, 0x040002, 0xAADDDD
	};
#else
	uint mdio[32] = {0x1f8000, 0x0a6401, 0x0b800c, 0x1f8060,
		0x037102, 0x05ff08, 0xAADDDD};
#endif /* BCM4350_SIMULATION */
	len = 0;

	for (i = 0; i < USB_MDIO_ADDR_MAX; i++) {
		(void)snprintf(mdiodesc, sizeof(mdiodesc), "usbssmdio%d", i);
		if ((otp_str = getvar(NULL, mdiodesc)) != NULL) {
			array_len = getintvararraysize(NULL, mdiodesc);
			for (j = 0; j < array_len; j++) {
				mdio[len] = getintvararray(NULL, mdiodesc, j);
				if (mdio[len] == 0xAADDDD) {
					break;
				} else {
					len++;
				}
			}
		} else {
			break;
		}
	}

	if (len == 0) {
#ifdef BCM4350_SIMULATION
		len = 11;
#else
		len = 6;
		if (uxdc->rev >= 2) {
			/* A0 B0 B1 = 0, C0 rev = 2 */
			mdio[3] = 0x0c0007;
			len = 4;
		}
#endif /* BCM4350_SIMULATION */
	}

	for (i = 0; i < len; i++) {
		val = mdio[i];
		addr = val>>16;
		data = val&0xffff;

		xdc_dbg(("write mdio addr = %x data = %x \n", addr, data));
		if (addr > 0x1f) {
			break;
		}
		xdc_mdio_wreg(uxdc, addr, data);
		xdc_dbg(("read mdio addr = %x data = %x \n",
			addr, (uint32)xdc_mdio_rreg(uxdc, addr)));
	}

	return 0;
} /* xdc_phy_pll30_ctrl */

/** Only called once during firmware lifetime */
int xdc_init_phy(struct usbdev_chip *uxdc, bool usb30_ss)
{
	int ret = 0;
	chipcregs_t *cc;
	uint32 origidx, intr_val;
	uint32 rdval, to = 0x3000;
	uint32 otp_u1u2;

	/* Remember original core before switch to chipc */
	cc = (chipcregs_t *)si_switch_core(uxdc->sih, CC_CORE_ID, &origidx, &intr_val);

	W_REG(uxdc->osh, &cc->pllcontrol_addr, 0x06); /* write 0x18000660 0x06 */

	rdval = R_REG(uxdc->osh, &cc->pllcontrol_data); /* read back 0x18000664 */
	xdc_dbg(("0x18000664 pllcontrl_data = %x\n", rdval));

	if (usb30_ss) {
		/* [26:25] refclk_mode set to be 0x01 to be 40Mhz */
		if ((rdval&0x06000000) != PLL_REFCLK40_MODE) {
			rdval = (rdval&(~0x06000000))|PLL_REFCLK40_MODE;
			/* write 0x18000664 0x02xxxxxx */
			W_REG(uxdc->osh, &cc->pllcontrol_data, rdval);
		}
		rdval = R_REG(uxdc->osh, &cc->pmucontrol); /* read back 0x18000600 */
		rdval |= PLL_CTRL_UPDATE; /* [0x18000600].bit10 = 1 */
		W_REG(uxdc->osh, &cc->pmucontrol, rdval);
		rdval = R_REG(uxdc->osh, &cc->pmucontrol); /* read back 0x18000600 */
	}

	/* 0x18007310.b[15] port_disable = 1,  .b[14:12] rx_sync_det_len = 5 = 0xD000 */
	rdval = UTMI_PHYPT_DISABLE|(5<<12);
	W_REG(uxdc->osh, &uxdc->regs->phyutmictl1, rdval);
	rdval = R_REG(uxdc->osh, &uxdc->regs->phyutmictl1);
	xdc_dbg(("0x18007310 phyutmictl1 = %x\n", rdval));

	/* 0x18007310.b[15] port_disable = 0, .b[14:12] rx_sync_det_len = 5 */
	/* b[3] disable_phy_iddq = 1, b[2] disable_phy_iso = 1 0x500C */
	rdval = (rdval&(~UTMI_PHYPT_DISABLE))|(UTMI_PHY_IDDQ|UTMI_PHY_ISO);
	xdc_dbg(("0x18007310 phyutmictl1 = %x\n", rdval));
	W_REG(uxdc->osh, &uxdc->regs->phyutmictl1, rdval);

	/* 0x18007328.b[11] usb3_pipe_resetb = 0 to start phy reset */
	rdval = R_REG(uxdc->osh, &uxdc->regs->phymiscctrl);
	xdc_dbg(("0x18007328 phymisctrl = %x\n", rdval));

	W_REG(uxdc->osh, &uxdc->regs->usb30dmdioctl, 0x15);

	/* 0x18007000.b[12] = 1, assert PHY/mdio reset */
	rdval = R_REG(uxdc->osh, &uxdc->regs->devcontrol);
	rdval |= DEV_CTRL_PHYRESET;
	W_REG(uxdc->osh, &uxdc->regs->devcontrol, rdval);

	/* 0x18007000.b[12] = 0, Deassert PHY/mdio reset */
	rdval = R_REG(uxdc->osh, &uxdc->regs->devcontrol);
	xdc_dbg(("devcontrol = %x \n", rdval));
	rdval &= (~DEV_CTRL_PHYRESET);
	W_REG(uxdc->osh, &uxdc->regs->devcontrol, rdval);
	rdval = R_REG(uxdc->osh, &uxdc->regs->devcontrol);
	xdc_dbg(("devcontrol = %x \n", rdval));

#ifndef BCM4350_FPGA
	if (usb30_ss)
		ret = xdc_phy_pll30_ctrl(uxdc);
#endif

	/* Deassert pll_resetb to start up PHY PLL20 */
	/* 0x18007310.b[0] pll_resetb = 1 */
	rdval = R_REG(uxdc->osh, &uxdc->regs->phyutmictl1);
	rdval |= UTMI_PLL_RESETB;
	W_REG(uxdc->osh, &uxdc->regs->phyutmictl1, rdval);
	rdval = R_REG(uxdc->osh, &uxdc->regs->phyutmictl1);

	if (1 || usb30_ss) {
		/* Assert 0x18000664.b[28] pll_seq_start = 1 to start up PHY PLL 30 */
		rdval = R_REG(uxdc->osh, &cc->pllcontrol_data);
		rdval |= PLL_SEQ_START;
		W_REG(uxdc->osh, &cc->pllcontrol_data, rdval);

		/* Assert 0x18000600.b[10] pll_ctrl_update = 1 to start up PHY PLL 30 */
		rdval = R_REG(uxdc->osh, &cc->pmucontrol);
		rdval |= PLL_CTRL_UPDATE;
		W_REG(uxdc->osh, &cc->pmucontrol, rdval);
#ifndef BCM4350_FPGA
		/* polling for PLL LOCK */
		xdc_mdio_wreg(uxdc, XDCI_MDIO_BASE_REG, XDCI_MDIO_PLL30_BLOCK);
		do {
			rdval = (uint32)xdc_mdio_rreg(uxdc, 0x00);
			if (rdval&0x400)
				break;
			to--;
		} while (to != 0);
#endif /* BCM4350_FPGA */
	}

	/* Read and buffer the OTP variable, so that we don't need to
	   read it repeatedly in LTSSM routines.
	*/
	xdci_set_otp_var_u1u2(uxdc);
	otp_u1u2 = xdci_get_otp_var_u1u2(uxdc->xdc);
	xdc_dbg_ltssm(("Disable deglitch bypass.\n"));
	xdc_mdio_wreg(uxdc, XDCI_MDIO_BASE_REG, XDCI_MDIO_PIPE_BLOCK);
	xdc_mdio_wreg(uxdc, 0x3,  0x4302);

	if (otp_u1u2 & XDCI_OTP_VAR_COMMA_RELOCK_MASK) {
		xdc_dbg_ltssm(("Turning on comma relock \n"));
		xdc_mdio_wreg(uxdc, 0x14, 0xE8);
	}
	if (otp_u1u2 & XDCI_OTP_VAR_CDR_CLAMP_MASK) {
		xdc_dbg_ltssm(("Enabling cdr clamp\n"));
		xdc_mdio_wreg(uxdc, XDCI_MDIO_BASE_REG, XDCI_MDIO_RXPMD_BLOCK);
		xdc_mdio_wreg(uxdc, 0x7,  0x145);
	}

	/* Return to original core */
	si_restore_core(uxdc->sih, origidx, intr_val);

	if (to == 0) {
		ret = -1;
		xdc_err(("PHY PLL lock timeout \n"));
	}

	return ret;
} /* xdc_init_phy */

/**
 * Start of hardware initialization. Initializes USB PHY. Resets core but does not start it. Only
 * called once during firmware lifetime.
 */
int xdc_init_hw(struct usbdev_chip *uxdc)
{
	int ret = 0;
	uint32 rdval, orig_coreid;
	uint32 cs;
	char *otp_str;
	uint32 usb_hs = 0;

	cs = uxdc->sih->chipst;
	xdc_dbg(("cc status %x\n", cs));

	/* IFC is 110 , we will overwrite CoreCntro.SPEED[7,5] to 000 */
	if (CST4350_CHIPMODE_USB30D_WL(cs)) {
		cs = ~0;
	} else {
		/* IFC is 101 , we will leave CoreCntro.SPEED[7,5] to AS-IS */
		cs = ~(CCTRL_SPEED);
	}

	if ((otp_str = getvar(NULL, "usbnoss")) != NULL) {

		usb_hs = (uint32)bcm_strtoul(otp_str, NULL, 0);
		if (usb_hs) {
			cs = ~0;
			usb_hs = B_6; /* set ioControl.SPEED = b010 HS */
		}
	}

	orig_coreid = si_coreid(uxdc->sih);
	si_setcore(uxdc->sih, USB30D_CORE_ID, 0);
	/* w 0x18106800.b[0] = 1 to reset */
	si_wrapperreg(uxdc->sih, AI_RESETCTRL, ~0, AIRC_RESET);
	OSL_DELAY(1);
	/* w 0x18106408.b[0,2] = 0x07 to enable clk, rd back value */
	rdval = si_wrapperreg(uxdc->sih, AI_IOCTRL, cs,
		(usb_hs|CCTRL_NDR_EN|CCTRL_MAINCLK_EN|CCTRL_FORCE_GTCLK_ON|CCTRL_AUXCLK_EN));
	xdc_dbg(("18106408 = %x \n", rdval));
	OSL_DELAY(1);
	/* w 0x18106800.b[0] = 0 to clear reset */
	si_wrapperreg(uxdc->sih, AI_RESETCTRL, ~0, 0x00);
	OSL_DELAY(1);
	/* w 0x18106408.b[0,2] = 0x05 to clear ForceGateClkOn, rd back value */
	rdval = si_wrapperreg(uxdc->sih, AI_IOCTRL, cs,
		(usb_hs|CCTRL_NDR_EN|CCTRL_MAINCLK_EN|CCTRL_AUXCLK_EN));
	xdc_dbg(("18106408 = %x \n", rdval));

	si_setcoreidx(uxdc->sih, orig_coreid);

	xdc_dbg(("CoreControl = %x \n", rdval));
	/* Check CoreContrl.SPEED field to detect GPIO strap option */
	rdval = (rdval&CCTRL_SPEED)>>CCTRL_SPEED_SHIFT;

	ret = xdc_init_phy(uxdc, ((rdval == 0)?TRUE:FALSE));

	/* Deassert XDC reset bit 0x18007000.b[0] DevReset = 0 */
	rdval = R_REG(uxdc->osh, &uxdc->regs->devcontrol);

	xdc_dbg(("DevReset = %x", rdval));
	if (rdval&DEV_CTRL_DEVRESET) {
		rdval &= ~DEV_CTRL_DEVRESET;
		W_REG(uxdc->osh, &uxdc->regs->devcontrol, rdval);
	}

	rdval = R_REG(uxdc->osh, &uxdc->regs->devcontrol);

	return ret;
} /* xdc_init_phy */

/**
 * Stops hardware. Only called once during firmware lifetime, typically when host unloads driver.
 */
int xdc_stop(struct xdcd *xdc)
{
	uint32 status;
	uint32 timeout = 160000;

	xdc_trace(("stop the xDC\n"));

	status = R_REG(xdc->osh, &xdc->opregs->status) & XDC_USBSTS_CH;

	if (status == 0) {
		AND_REG(xdc->osh, &xdc->opregs->command,
			~(XDC_USBCMD_INTE|XDC_USBCMD_HSEE|XDC_USBCMD_RUN));
		while (timeout) {
			status = R_REG(xdc->osh, &xdc->opregs->status);
			if ((status&XDC_USBSTS_CH) == 1) {
				xdc->dev_state |= XDC_STATE_HALTED;
				break;
			}
			timeout--;
		}
	}

	if (timeout == 0) {
		xdc_err(("xdc is unable to stop.\n"));
		return -1;
	}

	return 0;
} /* xdc_stop */

/** adds a single empty receive buffer to endpoint for host->dongle communication */
int xdc_queue_rxbuf(struct xdc_ep *ep, struct xdcd *xdc, void *p)
{
	struct xdc_urb *urb;
	uint32 max_urb_wr, trb_type;
	int ret;

	max_urb_wr = ep->max_urb;

	trb_type = NORMAL_TRB;

#ifdef BCMUSBDEV_COMPOSITE
	if (ep->type == UE_ISOCHRONOUS) {
		trb_type = ISOC_TRB;
	}
#endif /* BCMUSBDEV_COMPOSITE */

	urb = xdc_alloc_rx_urb(xdc, ep, p);

	if (urb) {
		ret = xdc_submit_urb(ep, urb, trb_type, 0);

		if (ret >= 0)
			return 0;

		xdc_err(("urb fill error =  %d\n", ret));
		if (ep->urb_wr)
			ep->urb_wr--;
		else
			ep->urb_wr = max_urb_wr - 1;

		p = (void *)urb->buf;
		PKTFREE(xdc->osh, p, FALSE);
		urb->buf = NULL;
		urb->trb = NULL;
	} else {
		ret = -1;
	}

	return ret;
} /* xdc_queue_rxbuf */

/** Unused function */
int xdc_urb_dequeue(struct xdc_ep *ep, struct xdc_urb *urb)
{
	struct xdcd *xdc;

	uint32 ret;

	xdc_trace(("xdc_urb_dequeue \n"));
	if (!ep || !urb)
		return -1;

	xdc = ep->xdc;
	ep->deq_urb = urb;

	ret = xdc_cmd(xdc, ep, STOP_RING_TRB);

	xdc_trace(("xdc_urb_dequeue done\n"));
	return ret;
}

#endif /* USB_XDCI */
