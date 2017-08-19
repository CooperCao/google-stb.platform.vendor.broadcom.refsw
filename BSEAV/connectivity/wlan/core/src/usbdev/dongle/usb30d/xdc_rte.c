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
 * $Id: xdc_rte.c 412849 2013-07-16 21:42:57Z $
*/

#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmdevs.h>
#include <bcmendian.h>
#include <bcmutils.h>
#include <bcmstdlib.h>
#include <hndpmu.h>
#include <osl.h>
#include <osl_ext.h>
#include <usb.h>
#include <usbstd.h>
#include <usbdev.h>
#include <usbdev_dbg.h>
#include <hndsoc.h>
#include <aidmp.h>
#include "xdc.h"
#include "xdc_rte.h"

#ifdef USB_XDCI
void xdci2wlan_irq_on(struct usbdev_chip *ch);
void xdci2wlan_irq_off(struct usbdev_chip *ch);
int xdci2wlan_isr(struct usbdev_chip *ch);
void xdci_ep_detach(struct usbdev_chip *ch, int ep_index);
bool xdci_match(uint vendor, uint device);
int xdci_usb30(struct usbdev_chip *ch);
int xdci_resume(struct usbdev_chip *ch);

struct usbdev_chip *
BCMATTACHFN(xdci_attach)(void *drv, uint vendor, uint device, osl_t *osh,
	volatile void *regs, uint bus);
uint32
BCMATTACHFN(xdci_init)(struct usbdev_chip *ch, bool disconnect);
void
BCMATTACHFN(xdci_detach)(struct usbdev_chip *ch, bool disable);

#ifdef XDC_DEBUG
uint32 xdc_msg_level = XDC_ERROR_VAL | XDC_DBG_VAL | XDC_TRACE_VAL;
#else
uint32 xdc_msg_level = XDC_ERROR_VAL;
#endif

#define EP0_MAX_PKT_SIZE 512
#ifndef USBCTLBUFSZ
#define USBCTLBUFSZ 128
#endif /* USBCTLBUFSZ */


usb_endpoint_descriptor_t xdc_ep0_desc = {
	bLength: USB_ENDPOINT_DESCRIPTOR_SIZE,
	bDescriptorType: UDESC_ENDPOINT,
	bEndpointAddress: 0,
	bmAttributes: UE_CONTROL,
	wMaxPacketSize: 0,
	bInterval: 0
};

void xdci_set_otp_var_u1u2(struct usbdev_chip *uxdc)
{
	struct xdcd *xdc = uxdc->xdc;
	char *otp_str;

	otp_str = getvar(NULL, "usb30u1u2");
	/* otp_str = "0x1ff1"; */
	if ((uxdc->id == 0x83d) && (uxdc->rev == 0x2)) {
		otp_str = "0x1ff1";
	} else if ((uxdc->id == 0x83d) && (uxdc->rev == 0x3)) {
		/* otp_str = "0x1ff1"; */
	}

	printf("USB CORE ID:0x%x USB CORE REV:0x%x otp_str:%s\n",
		uxdc->id, uxdc->rev, otp_str);

	if (otp_str != NULL) {
		xdc->uxdc->otp_u1u2 = bcm_strtoul(otp_str, NULL, 0);
		xdc_dbg_ltssm(("OTP usb30u1u2=%x\n", xdc->uxdc->otp_u1u2));
	}
	xdc_dbg_ltssm(("u1u2=%x\n", xdc->uxdc->otp_u1u2));
}

uint32 xdci_get_otp_var_u1u2(struct xdcd *xdc)
{
	return xdc->uxdc->otp_u1u2;
}

/** OTP/SPROM/nvram variables can be used to initialize USB30 core registers */
static void xdci_regs_update(struct xdcd *xdc)
{
	char *otp_str;
	uint32 i, regAddr, val;
	uint32 usb_reg0[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	uint32 usb_reg1[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	uint32 otp_u1u2;

	if ((otp_str = getvar(NULL, "usb30u1u2")) != NULL) {
		otp_u1u2 = bcm_strtoul(otp_str, NULL, 0);
		xdc->u1u2_enable = (uint8) (otp_u1u2 & XDCI_OTP_VAR_U1U2_MASK);
		xdc_dbg(("usb30_u1u2 = %d \n", xdc->u1u2_enable));
	}

	if ((otp_str = getvar(NULL, "usb30regs0")) != NULL) {
		for (i = 0; i < 10; i++) {
			usb_reg0[i] = getintvararray(NULL, "usb30regs0", i);
			xdc_dbg(("reg[%d]=%x ", i, usb_reg0[i]));
		}
		xdc_dbg(("\n"));

		for (i = 0; i < 10; i += 2) {
			if (usb_reg0[i] == 0)
				break;
			regAddr = usb_reg0[i];
			val = usb_reg0[i+1];
			W_REG(xdc->osh, (uint32 *)XDC_INDIRECT_ADDR, regAddr);
			OSL_DELAY(2);
			W_REG(xdc->osh, (uint32 *)XDC_INDIRECT_DATA, val);
			OSL_DELAY(2);
		}
	}

	if ((otp_str = getvar(NULL, "usb30regs1")) != NULL) {
		for (i = 0; i < 4; i++) {
			usb_reg1[i] = getintvararray(NULL, "usb30regs1", i);
			xdc_dbg(("reg[%d]=%x ", i, usb_reg1[i]));
		}
		xdc_dbg(("\n"));
		for (i = 0; i < 4; i += 2) {
			if (usb_reg1[i] == 0)
				break;
			regAddr = usb_reg1[i];
			val = usb_reg1[i+1];
			W_REG(xdc->osh, (uint32 *)regAddr, val);
			OSL_DELAY(2);
		}
	}
} /* xdci_regs_update */

/** host -> dongle flow control */
void xdci_rxflowcontrol(struct usbdev_chip *ch, int ep_index, bool state)
{
	struct usbdev_chip *uxdc;
	struct xdc_ep *ep;

	uxdc = ch;

	xdc_dbg(("rxflowcontrl..ep = %d \n", ep_index));
	ep = &uxdc->xdc->ep[ep_index];

	ep->flow_ctrl = state;

	if (state == OFF) {
		xdc_rx_malloc_chk(uxdc->xdc, ep);
	}
}

/** bootloader specific function */
void xdci_indicate_start(void *ch, uint32 start, uint32 no_dis)
{
}

/** enables or disables u1/u2 power state on request of the host */
void xdci_enable_u1u2(void *ch, bool enable_u1u2)
{
	struct usbdev_chip *uxdc;
	struct xdcd *xdc;
	uxdc = (struct usbdev_chip *)ch;

	xdc = uxdc->xdc;

	if ((uxdc->id == 0x83d) && (uxdc->rev == 0x3))
	{
		if (enable_u1u2)
			xdc->u1u2_enable = 1;
		else
			xdc->u1u2_enable = 0;

		printf("1xdci_enable_u1u2 xdc->u1u2_enable:%d\n", (uint8)xdc->u1u2_enable);
	}

	printf("xdci_enable_u1u2 xdc->u1u2_enable:%d\n", (uint8)xdc->u1u2_enable);
}

/** bootloader specific: host requested different USB speed */
int xdci_indicate_speed(void *ch, uint32 speed)
{
	struct usbdev_chip *uxdc;
	struct xdcd *xdc;
	uxdc = (struct usbdev_chip *)ch;

	xdc = uxdc->xdc;
	printf("speed0 = %d \n", speed);

	if (speed == 0) {
			if (xdc->speed == XDC_PORT_SS)
				speed = XDC_PORT_HS;
			else
				speed = XDC_PORT_SS;
	} else {
		if (speed == xdc->speed)
			return 0;
	}

	uxdc->reconnect = 1;
	uxdc->speed = speed;
	printf("speed = %d \n", speed);

	return 1;
}

/**
 * Switch between HS (USB 2.1) and SS (USB 3.0) speeds. Used by bootloader. Called on host request.
 */
int xdci_force_speed(struct usbdev_chip *uxdc)
{
	struct xdcd *xdc;
	uint32 rdval, orig_coreid;
	uint32 cs;
	uint32 usb_hs = 0;

	xdc = uxdc->xdc;

	if (uxdc->speed == XDC_PORT_HS) {
		/* switch speed from SS to HS */
		printf("switch to HS\n");
		usb_hs = B_6; /* set ioControl.SPEED = b010 HS */
	} else {
		usb_hs = 0; /* set ioControl.SPEED = b000 SS */
		printf("switch to SS\n");
	}


	xdci_soft_disconnect(xdc);

	cs = (CCTRL_SPEED);

	orig_coreid = si_coreid(uxdc->sih);
	si_setcore(uxdc->sih, USB30D_CORE_ID, 0);
	/* w 0x18106408.CoreCntro.SPEED[7,5] */
	rdval = si_wrapperreg(uxdc->sih, AI_IOCTRL, cs, usb_hs);
	printf("18106408 = %x \n", rdval);
	OSL_DELAY(1);

	si_setcoreidx(uxdc->sih, orig_coreid);

	return 1;
} /* xdci_force_speed */


si_t *
xdci_get_sih(struct usbdev_chip *xdc)
{
	struct usbdev_chip *uxdc;

	uxdc = (struct usbdev_chip *)xdc;
	return (uxdc->sih);
}

void xdci2wlan_irq_on(struct usbdev_chip *ch)
{
	uint32 temp;
	struct usbdev_chip *uxdc = (struct usbdev_chip *)ch;

	temp = R_REG(uxdc->osh, &uxdc->regs->intrmaskwlan);
	/* b[0, 3] is core interrupt for rings */
	temp |= 0x0F;

	W_REG(uxdc->osh, &uxdc->regs->intrmaskwlan, temp);
}

void xdci2wlan_irq_off(struct usbdev_chip *ch)
{
	uint32 temp;
	struct usbdev_chip *uxdc = (struct usbdev_chip *)ch;

	temp = R_REG(uxdc->osh, &uxdc->regs->intrmaskwlan);
	temp &= ~0x0F; /* b[0, 3] is core interrupt for rings */

	W_REG(uxdc->osh, &uxdc->regs->intrmaskwlan, temp);
}

/** called once, during firmware initialization */
uint32 xdc_enable_irq(struct xdcd *xdc)
{
	uint32 temp;

	/* OP->command[3] + IR->irq_pend(IMAN)[1] */
	xdc_trace(("xdc_enable_irq \n"));
	temp = R_REG(xdc->osh, &xdc->irset->irq_ctrl);
	temp &= ~ER_IMODI_MASK;
	/* Immediate Interrupts */
	W_REG(xdc->osh, &xdc->irset->irq_ctrl, temp);

	temp = R_REG(xdc->osh, &xdc->opregs->command);
	temp |= (XDC_USBCMD_INTE | XDC_USBCMD_HSEE);
	xdc_dbg(("command register = 0x%x.", temp));

	W_REG(xdc->osh, &xdc->opregs->command, temp);
	temp = R_REG(xdc->osh, &xdc->irset->irq_pend);

	/* OP->status[3]  -> IR->irq_pend[0] */
	xdc_dbg(("irset = %p irset->irq_pend = 0x%x \n",
		xdc->irset, (unsigned int) ER_INT_ENABLE(temp)));
	W_REG(xdc->osh,  &xdc->irset->irq_pend, ER_INT_ENABLE(temp));


	return 1;
} /* xdc_enable_irq */

uint32 xdc_disable_irq(struct xdcd *xdc)
{
	uint32 temp;

	xdc_trace(("\nxdc_disable_irq()\n"));

	temp = R_REG(xdc->osh, &xdc->opregs->command);
	temp &= ~(XDC_USBCMD_INTE);
	xdc_dbg(("\nDisable interrupts, cmd = 0x%x.", temp));

	W_REG(xdc->osh, &xdc->opregs->command, temp);
	temp = R_REG(xdc->osh, &xdc->irset->irq_pend);

	W_REG(xdc->osh,  &xdc->irset->irq_pend, ER_INT_DISABLE(temp));

	/* OP->status[3]  -> IR->irq_pend[o] */

	return 1;
}

/** used to read 'internal' registers */
uint32 xdci_indirect_read(struct xdcd *xdci, int offset)
{
	uint32 *dbgw = (uint32 *) ((char*)xdci->cpregs + XDCI_IRA_WA_REG);
	uint32 *dbgr = (uint32 *) ((char*)xdci->cpregs + XDCI_IRA_RD_REG);
	uint32 retval;

	W_REG(xdci->osh, dbgw, offset);
	retval = R_REG(xdci->osh, dbgr);
	return retval;
}

/** used to write 'internal' registers */
void xdci_indirect_write(struct xdcd *xdci, int offset, uint32 val)
{
	uint32 *dbgw = (uint32 *) ((char*)xdci->cpregs + XDCI_IRA_WA_REG);
	uint32 *dbgr = (uint32 *) ((char*)xdci->cpregs + XDCI_IRA_RD_REG);

	W_REG(xdci->osh, dbgw, offset);
	W_REG(xdci->osh, dbgr, val);
}

/** returns e.g. XDC_LINK_STATE_RESUME */
uint32 xdci_get_link_state(struct xdcd *xdci)
{
	uint32 portsc;
	uint32 link_state;

	portsc = R_REG(xdci->osh, &xdci->opregs->portsc);
	link_state =  (portsc  & XDC_PORTSC_PLS) >> XDC_PORTSC_PLS_SHIFT;

	return link_state;
}

#ifdef XDCI_MONITOR_LTSSM_WAR

#define XDCI_LTSSM_STATE_POLLING_IDLE	0x70
#define XDCI_LTSSM_STATE_POLLING_ACT	0x75
#define XDCI_LTSSM_STATE_POLLING_LFPS	0x71
#define XDCI_LTSSM_STATE_POLLING_RXEQ	0x73
#define XDCI_LTSSM_STATE_POLLING_CFG	0x76
#define XDCI_LTSSM_STATE_U0		0x00
#define XDCI_LTSSM_STATE_SS_DIS_DEF	0x41
#define XDCI_LTSSM_STATE_SS_INACT_IDLE	0x60
#define XDCI_LTSSM_STATE_RECOVERY_ACT	0x81
#define XDCI_LTSSM_STATE_RECOVERY_CFG	0x82
#define XDCI_LTSSM_STATE_RXDET_ACT	0x52
#define XDCI_CPREG_HIST_BASE     0x1820

/* ILL - Internal Link Layer */
#define XDCI_ILL_STATUS_AND_CONTROL 0x1800
#define XDCI_PLS_MASK        (0xf << 5)
#define XDCI_PLS_SHIFT       5
#define XDCI_LINK_STATE_SS_DIS 4

int xdci_debug_monitor_ltssm = 1;
void xdci_phy_cdr_reset(struct xdcd *xdci);
void xdci_monitor_ltssm(struct xdcd *xdci);
void xdci_ltssm_reset_histogram(struct xdcd *xdci);
static uint32 xdci_read_ltssm_histogram_entry(struct xdcd *xdci, int offset);
static void xdci_mdelay(uint32 msecs);
static int xdci_is_ltssm_histogram_state(struct xdcd *xdci, uint8 *state,
	int state_cnt, int time_ms, uint8 *mstate);
static uint32 xdci_read_internal_reg(struct xdcd *xdci, int offset);

/** XDCI_MONITOR_LTSSM_WAR specific */
static uint32 xdci_read_internal_reg(struct xdcd *xdci, int offset)
{
	uint32 *ira_w = (uint32 *) ((char*)xdci->cpregs + XDCI_IRA_WA_REG);
	uint32 *ira_r = (uint32 *) ((char*)xdci->cpregs + XDCI_IRA_RD_REG);
	uint32 rval;

	W_REG(xdci->osh, ira_w, offset);
	rval = R_REG(xdci->osh, ira_r);
	return rval;
}

/** XDCI_MONITOR_LTSSM_WAR specific */
static uint32 xdci_read_ltssm_histogram_entry(struct xdcd *xdci, int offset)
{
	uint32 *dbgw = (uint32 *) ((char*)xdci->cpregs + XDCI_IRA_WA_REG);
	uint32 *dbgr = (uint32 *) ((char*)xdci->cpregs + XDCI_IRA_RD_REG);
	uint32 ltssm;

	W_REG(xdci->osh, dbgw, XDCI_CPREG_HIST_BASE + offset * 4);
	ltssm = R_REG(xdci->osh, dbgr);
	return ltssm;
}

/** XDCI_MONITOR_LTSSM_WAR specific */
static void xdci_mdelay(uint32 msecs)
{
	OSL_DELAY(msecs * 1000);
}

/** XDCI_MONITOR_LTSSM_WAR specific */
void xdci_ltssm_reset_histogram(struct xdcd *xdci)
{
	uint32 i = 0;
	uint32 hstgrm_base = XDCI_CPREG_HIST_BASE;
	uint32 *dbgw = (uint32 *) ((char*)xdci->cpregs + XDCI_IRA_WA_REG);
	uint32 *dbgr = (uint32 *) ((char*)xdci->cpregs + XDCI_IRA_RD_REG);

	xdc_dbg_ltssm(("reset_hist\n"));
	for (i = 0; i < 4; i++)
	{
		W_REG(xdci->osh, dbgw, hstgrm_base);
		W_REG(xdci->osh, dbgr, 0xcccccccc);
		hstgrm_base += 4;
	}
}

/** XDCI_MONITOR_LTSSM_WAR specific */
void xdci_ltssm_dump_histogram(struct xdcd *xdci)
{
	int i;

	xdc_dbg_ltssm(("Histogram = "));
	for (i = 0; i < 4; i++) {
		xdc_dbg_ltssm(("%08x ", xdci_read_ltssm_histogram_entry(xdci, i)));
	}
	xdc_dbg_ltssm(("\n"));
}


/**
 * XDCI_MONITOR_LTSSM_WAR specific.
 * Returns 1 if any one of 16 entries in the LTSSM histogram is any of the entries in the state
 * array.
 *     @param state_cnt length of the state array
 *     @param *mstate the state that matched a histogram entry
 */
static int xdci_is_ltssm_histogram_state(struct xdcd *xdci, uint8 *state,
	int state_cnt, int time_ms, uint8 *mstate) {

	uint32 ltssm_histogram[4];
	uint8 *ltssm_histogram_u8 = (uint8*) &ltssm_histogram;
	int i, j, time;

	for (time = 0; time < time_ms; time++) {
		for (i = 0; i < 4; i++)
			ltssm_histogram[i]  =
				xdci_read_ltssm_histogram_entry(xdci, i);
		for (i = 0; i < 16; i++) {
			for (j = 0; j < state_cnt; j++) {
				if (ltssm_histogram_u8[i] == state[j]) {
					if (mstate)
						*mstate = state[j];
					return 1;
				}
			}
		}
		xdci_mdelay(1);
	}
	return 0;
}

/** XDCI_MONITOR_LTSSM_WAR specific */
void xdci_phy_cdr_reset(struct xdcd *xdci)
{
	uint16 rdval;
	uint32 val1, val2, val3;

	xdc_mdio_wreg(xdci->uxdc, XDCI_MDIO_BASE_REG, XDCI_MDIO_RXPMD_BLOCK);
	val1 = xdc_mdio_rreg(xdci->uxdc, 0xa);
	val2 = xdc_mdio_rreg(xdci->uxdc, 0x8);
	xdc_mdio_wreg(xdci->uxdc, XDCI_MDIO_BASE_REG, XDCI_MDIO_PIPE_BLOCK);
	val3 = xdc_mdio_rreg(xdci->uxdc, 0x17);

	/* select PIPE block */
	xdc_mdio_wreg(xdci->uxdc, XDCI_MDIO_BASE_REG, XDCI_MDIO_PIPE_BLOCK);
	rdval = xdc_mdio_rreg(xdci->uxdc, 0xd);
	/* Set and clear the reset bit15 in rigiste 0xd */
	xdc_mdio_wreg(xdci->uxdc, 0xd, rdval | 0x8000);
	xdci_mdelay(1);
	xdc_mdio_wreg(xdci->uxdc, 0xd, rdval & ~0x8000);
	xdc_dbg_ltssm(("CDR reset done : 0x8020:0xa = %08x, 0x8020:0x8 = %08x,"
		" 0x8060:0x17 = %08x\n", val1, val2, val3));

}


/**
 * XDCI_MONITOR_LTSSM_WAR specific.
 * This monitors the LTSSM for a duration of initial_timeout (a local variable) and checks if it is
 * stuck in active polling state. If it is stuck there then it issues a PHY CDR reset to hopefully
 * get it out of active polling.
 */
void xdci_monitor_ltssm(struct xdcd *xdci)
{
	int initial_timeout = 360; /* in msec */
	uint8 link_state = 0;
	int reset_cnt = 0;
	uint32 ill_sc;

	xdc_dbg_ltssm(("In xdci_monitor_ltssm \n"));

	xdci_mdelay(5);

	/** Check for initial_timeout milli seconds of softconnect, if link
	    goes to ss.disabled or polling.act
	*/
	do {
		uint8 desired_states1[3] = {
			XDCI_LTSSM_STATE_SS_DIS_DEF,
			XDCI_LTSSM_STATE_POLLING_ACT, XDCI_LTSSM_STATE_U0 };

		ill_sc = xdci_read_internal_reg(xdci, XDCI_ILL_STATUS_AND_CONTROL);
		if (((ill_sc & XDCI_PLS_MASK)  >> XDCI_PLS_SHIFT) == XDCI_LINK_STATE_SS_DIS) {
			link_state = XDCI_LINK_STATE_SS_DIS;
			xdc_dbg_ltssm(("SS.Disabled from internal reg SUSPSC=%08x\n",
			link_state));
			goto out;
		}
		if (xdci_is_ltssm_histogram_state(xdci, desired_states1, 3, initial_timeout,
		&link_state)) {
			xdc_dbg_ltssm(("Link state = %x \n", link_state));
			if (link_state == XDCI_LTSSM_STATE_U0 ||
			link_state == XDCI_LTSSM_STATE_SS_DIS_DEF)
				goto out;
			else
				break;
		}
		xdci_mdelay(1);
		initial_timeout--;
	} while (initial_timeout);

	xdc_dbg_ltssm(("xdci_monitor_ltssm: timeout=%d\n", initial_timeout));

	if (!initial_timeout) {
		uint32 ill_sc = xdci_read_internal_reg(xdci, XDCI_ILL_STATUS_AND_CONTROL);
		xdc_dbg_ltssm(("xdci_monitor_ltssm: No CDR reset required ill_sc=%x.\n", ill_sc));
		xdci_ltssm_dump_histogram(xdci);
		return;
	}

	xdc_dbg_ltssm(("Check  if polling.\n"));

	xdci_mdelay(4);
	xdci_ltssm_dump_histogram(xdci);
	while (1) {
		uint8 desired_states2[3] = {
			XDCI_LTSSM_STATE_POLLING_CFG,
			XDCI_LTSSM_STATE_SS_INACT_IDLE,
			XDCI_LTSSM_STATE_U0 };
		/* Check if we went ahead of polling.active to either
		   polling.config or ss.disabled.
		*/
		if (xdci_is_ltssm_histogram_state(xdci, desired_states2, 3, 1, NULL))  {
			/* we are not stuck, nothing to do, return */
			xdc_dbg_ltssm(("reset_cnt=%d : Link out of Polling.ACT \n", reset_cnt));
			goto out;
		} else {
			if (reset_cnt >= 2) {
				xdc_dbg_ltssm(("ERROR: Link still stuck in Polling.ACT\n"));
				goto out;
			}
		}
		/* We are stuck in polling ACT,Give a CDR reset to PHY */
		xdci_phy_cdr_reset(xdci);
		reset_cnt++;
		xdci_mdelay(2);
		xdci_ltssm_dump_histogram(xdci);
	}
out:
	xdci_ltssm_dump_histogram(xdci);
	xdci_ltssm_reset_histogram(xdci);
} /* xdci_monitor_ltssm */

static uint32 xdci_get_susptd(struct xdcd *xdci);

/** XDCI_MONITOR_LTSSM_WAR specific */
static uint32 xdci_get_susptd(struct xdcd *xdci)
{
	uint32 *susptd_reg = (uint32 *) ((char*)xdci->cpregs + 0x42c);
	return R_REG(xdci->uxdc->osh, susptd_reg);
}

/** XDCI_MONITOR_LTSSM_WAR specific */
void xdci_monitor_ltss_on_u3exit(struct xdcd *xdci)
{
	int timeout = 20; /* in msec */
	int reset_cnt = 0;

	xdc_dbg_ltssm(("xdci_monitor_ltssm_on_u3exit susptd=%08x \n", xdci_get_susptd(xdci)));

	/* See if we entered Recovery.Active */
	do {
		uint8 state = XDCI_LTSSM_STATE_RECOVERY_ACT;
		if (xdci_is_ltssm_histogram_state(xdci, &state, 1, 1, NULL)) {
			xdci_ltssm_dump_histogram(xdci);
			break;
		}
		xdci_mdelay(1);
		timeout--;
	} while (timeout);

	xdc_dbg_ltssm(("U3 Exit: timeout=%d \n", timeout));
	if (!timeout) {
	        xdc_dbg_ltssm(("Link didn't enter Recovery.ACT\n"));
		goto out;
	}

	xdci_mdelay(1);
	while (1) {
		uint8 recovery_done_states[5] = {
			XDCI_LTSSM_STATE_RECOVERY_CFG,
			XDCI_LTSSM_STATE_RXDET_ACT,
			XDCI_LTSSM_STATE_U0,
			XDCI_LTSSM_STATE_SS_INACT_IDLE,
			XDCI_LTSSM_STATE_SS_DIS_DEF };

		/* Check if we exit out of Recovery.Active */
		if (xdci_is_ltssm_histogram_state(xdci, recovery_done_states, 5, 1, NULL))  {
			xdc_dbg_ltssm(("U3 Exit : reset_cnt=%d : Link out of Recovery_ACT. \n",
			reset_cnt));
			goto out;
		}  else {
			if (reset_cnt >= 2) {
				xdc_dbg_ltssm(("ERROR: Link is stuck in Recovery.Act.\n"));
				goto out;
			}
		}
		xdci_phy_cdr_reset(xdci);
		reset_cnt++;
		xdci_mdelay(2);
		xdci_ltssm_dump_histogram(xdci);
	}
out:
	xdci_ltssm_dump_histogram(xdci);
	xdci_ltssm_reset_histogram(xdci);
	return;
} /* xdci_monitor_ltss_on_u3exit */

/** XDCI_MONITOR_LTSSM_WAR specific */
void xdci_scale_timer(struct xdcd *xdci, uint32 timer_val)
{
	uint32 stc1;

	/* Scale the timer back to 1250 */
	stc1 = xdci_indirect_read(xdci, XDCI_SSLL_STC1);

	xdc_dbg_ltssm(("timer_val=%x STC1=%08x ", timer_val, stc1));
	stc1 &= ~(0xffff);
	stc1 |= timer_val;

	xdci_indirect_write(xdci, XDCI_SSLL_STC1, stc1);
	stc1 = xdci_indirect_read(xdci, XDCI_SSLL_STC1);
	xdc_dbg_ltssm(("STC1=%08x", stc1));
}

/** XDCI_MONITOR_LTSSM_WAR specific */
void xdci_set_cdr_reset_eco(struct xdcd *xdci, int enable)
{
	uint32 hw_eco_reg;

	/* Re-enable the HW ECO */
	hw_eco_reg = xdci_indirect_read(xdci, XDCI_SSLL_DEBUG_CTRL0);
	xdc_dbg_ltssm(("hw_eco_reg=%08x ", hw_eco_reg));

	if (enable == 1) {
		/* Enable CDR_RST ECO by clearing bit 25:21 */
		hw_eco_reg &= ~(0x1f << 21);
	} else {
		hw_eco_reg |= (0x1f << 21);
	}
	xdc_dbg_ltssm(("%08x ", hw_eco_reg));
	xdci_indirect_write(xdci, XDCI_SSLL_DEBUG_CTRL0, hw_eco_reg);
	hw_eco_reg = xdci_indirect_read(xdci, XDCI_SSLL_DEBUG_CTRL0);
	xdc_dbg_ltssm(("%08x \n", hw_eco_reg));

}

/** XDCI_MONITOR_LTSSM_WAR specific. Duration is 1 for 10ms  LFPS and 0 for 1 ms LFPS */
void xdci_set_lfps_duration(struct xdcd *xdci, int duration)
{
	uint32 *susptd_reg = (uint32 *) ((char*)xdci->cpregs + 0x42c);
	uint32 susptd_val;

	xdc_dbg_ltssm(("Increase LFPS duration\n"));
	susptd_val = R_REG(xdci->uxdc->osh, susptd_reg);
	xdc_dbg_ltssm(("susptd_read=%08x ", susptd_val));
	if (duration == 1) {
		/* Clear bit 23 for longer host initiated u3 exit */
		susptd_val &= 0xff7fffff;
	} else {
		susptd_val |= 1 << 23;
	}
	xdc_dbg_ltssm(("susptd_write=%08x ", susptd_val));
	W_REG(xdci->uxdc->osh, susptd_reg, susptd_val);
	susptd_val = R_REG(xdci->uxdc->osh, susptd_reg);
	xdc_dbg_ltssm(("susptd_read=%08x \n", susptd_val));
}

static void xdci_set_cdr_reset(struct xdcd *xdci);
static void xdci_clear_cdr_reset(struct xdcd *xdci);

/** XDCI_MONITOR_LTSSM_WAR specific */
static void xdci_set_cdr_reset(struct xdcd *xdci)
{
	uint16 rdval;

	/* select PIPE block */
	xdc_mdio_wreg(xdci->uxdc, XDCI_MDIO_BASE_REG, XDCI_MDIO_PIPE_BLOCK);
	rdval = xdc_mdio_rreg(xdci->uxdc, 0xd);
	xdc_mdio_wreg(xdci->uxdc, 0xd, rdval | 0x8000);
}

/** XDCI_MONITOR_LTSSM_WAR specific */
static void xdci_clear_cdr_reset(struct xdcd *xdci)
{
	uint16 rdval;

	/* select PIPE block */
	xdc_mdio_wreg(xdci->uxdc, XDCI_MDIO_BASE_REG, XDCI_MDIO_PIPE_BLOCK);
	rdval = xdc_mdio_rreg(xdci->uxdc, 0xd);
	xdc_mdio_wreg(xdci->uxdc, 0xd, rdval & ~0x8000);
}

/** XDCI_MONITOR_LTSSM_WAR specific. Called before software writes PLS = 0 */
void xdci_u3exit_remote1(struct xdcd *xdci)
{
	xdc_dbg_ltssm(("In xdci_u3exit_remote1\n"));
	/* SUSPTD[26]=0 should be cleared during initialization */
	xdci_set_lfps_duration(xdci, 0);
	/* Clear the HW ECO */
	xdci_set_cdr_reset_eco(xdci, 0);
	/* Scale the timer to issue longer LFPS */
	xdci_scale_timer(xdci, 1250);
}

/** XDCI_MONITOR_LTSSM_WAR specific. Called from the U0 event if it is due to remote wakeup */
void xdci_u3exit_remote2(struct xdcd *xdci)
{
	xdc_dbg_ltssm(("xdci_u3exit_remote2 \n"));
	xdci_scale_timer(xdci, 125);
	xdci_set_cdr_reset_eco(xdci, 1);
	xdci_set_lfps_duration(xdci, 1);
}

/** XDCI_MONITOR_LTSSM_WAR specific */
void xdci_u3exit_remote3(struct xdcd *xdci)
{
#define U3_ACT     0x31
#define U3_ACT_NEW 0x34
#define U3_EXIT	   0x32
	uint32 timeout = 0;
	uint32 pll_timeout = 0;
	uint32 link_state;
	uint32 histogram3;
	uint32 remain_timeout = 0;


	pll_timeout = 3 * 1000; /* 3 ms */
	xdc_dbg_ltssm(("In u3exit_remote3\n"));
	xdci_set_cdr_reset(xdci);

	/* PLL lock: Check the kink histogram if the last byte of DWORD3 is not U3.
	   Act or U3.Act_new then exit
	*/
	do {
		uint8 state = 0;
		histogram3 = xdci_read_ltssm_histogram_entry(xdci, 3);
		state = histogram3 & 0xff;
		xdc_dbg_ltssm(("Histogram3=%08x state=%x\n", histogram3, state));
		if (state == U3_EXIT)
			break;
		OSL_DELAY(10);
		pll_timeout -= 10;
	} while (pll_timeout);


	xdci_ltssm_dump_histogram(xdci);
	xdc_dbg_ltssm(("pll_timeout=%d\n", pll_timeout));

	if (!pll_timeout)
		xdc_dbg_ltssm(("Link was stuck in U3 Act for 3 ms\n"));

	timeout = 1000 * 10 + 100 ; /* 10000 us = 10ms + 100usec */

	/* Do a tight polling for a total time of 10ms on PLS to see if we entered
	   start of resume, poll every 10usec
	*/
	do {
		link_state = xdci_get_link_state(xdci);
		if (link_state == XDC_LINK_STATE_RESUME)
			break;
		OSL_DELAY(10);
		timeout -= 10;
	} while (timeout);


	if (!timeout) {
		xdc_dbg_ltssm(("The Link didn't enter start of resume within 10ms\n"));
		goto out;
	}
	xdc_dbg_ltssm(("Remote wakeup remaining timeout in us %d\n", timeout));

	/* Since the Host started LFPS we will never send LFPS for more than 10ms
	   wait for remaining timeout.
	   If the remaining timeout is more than 5ms then wait for 5ms then clear cdr reset,
	   if its less then wait for remaining time
	   and scale back the timer
	*/
	if (timeout > 5000) {
		OSL_DELAY(5000);
		xdci_clear_cdr_reset(xdci);
		remain_timeout = timeout - 5000;
		OSL_DELAY(remain_timeout);
	} else {
		OSL_DELAY(timeout);
		xdci_scale_timer(xdci, 125);
		remain_timeout = 5000 - timeout;
		OSL_DELAY(remain_timeout);
		xdci_clear_cdr_reset(xdci);
	}

	link_state = xdci_get_link_state(xdci);
out:
	xdci_clear_cdr_reset(xdci);
	if (link_state == XDC_LINK_STATE_U3) {
		xdc_dbg_ltssm(("We are still in U3\n"));
		/* Scale the timer back to 1 */
		/* This is to make sure we dont issue 10*10=100ms of LFPS  due to scaling
		   as xDC thinks host didn't respond with LFPS handshake
		*/
		xdci_scale_timer(xdci, 1);
		/* Wait for 100us */
		OSL_DELAY(100);
		/* Scale the timer back to 1250 */
		xdci_scale_timer(xdci, 1250);
		/* Wait for U0 event and then do rest of the stuff */
	} else {
		xdc_dbg_ltssm(("We are out of U3\n"));
		/* Change the timer back to 125 */
		xdci_scale_timer(xdci, 125);
		/* Reenable the HW ECO */
		xdci_set_cdr_reset_eco(xdci, 1);
		xdci_set_lfps_duration(xdci, 1);
	}
} /* xdci_u3exit_remote3 */

#endif /* XDCI_MONITOR_LTSSM_WAR */

/** soft connect the xDC to its DSP */
uint32 xdci_soft_connect(struct xdcd *xdc)
{
	uint32 val;

	/* waiting for vbus power is presented */
	while (1)
	{
		val = R_REG(xdc->osh, &xdc->opregs->portsc);
		if (val&XDC_PORTSC_PP)
			break;
	};
	/* write PORTSC.PE to enable PORT */
	xdc_dbg(("connect portsc = %x \n", val));
#ifdef XDCI_MONITOR_LTSSM_WAR
	xdci_ltssm_reset_histogram(xdc);
#endif
	val &= ~XDC_PORTSC_PLS;
	xdc_dbg(("enable portsc = %x \n", val));
	W_REG(xdc->osh, &xdc->opregs->portsc, val|XDC_PORTSC_PE);
	OSL_DELAY(200);

#ifdef XDCI_MONITOR_LTSSM_WAR
	if (xdci_get_otp_var_u1u2(xdc) & XDCI_OTP_VAR_ENABLE_LTSSM_CONNECT) {
		xdci_monitor_ltssm(xdc);
	}
#endif

	return 1;
}

/** soft disconnect the xDC from its DSP */
uint32 xdci_soft_disconnect(struct xdcd *xdc)
{
	uint32 val;

	val = R_REG(xdc->osh, &xdc->opregs->portsc);
	xdc_dbg(("disconnect portsc = %x \n", val));

	/* write PORTSC.PE to enable PORT */
	val = XDC_PORTSC_LWS |(XDC_LINK_STATE_SS_DIS<<5);
	W_REG(xdc->osh, &xdc->opregs->portsc, val);
	OSL_DELAY(10000);

	return 1;
}

/** also called during 'restore' */
int xdci_connect(struct xdcd *xdc)
{
	uint32 val;

	val = R_REG(xdc->osh, &xdc->opregs->portsc);

	if (((val&XDC_PORTSC_PLS)>>5) <= XDC_LINK_STATE_U2) {
		xdci_soft_disconnect(xdc);
	}

	xdci_soft_connect(xdc);

	return 0;
}

/**
 * In case of -nodis- firmware: restores the connection with the host. Only called once during
 * firmware lifetime.
 */
int xdci_restore(struct usbdev_chip *uxdc)
{
	struct xdc_ep *ep;
	uint32 addr;
	uint32 seqN;
	uint32 val;

	val = R_REG(uxdc->osh, &uxdc->xdc->opregs->portsc);
	val = (val&XDC_PORTSC_PLS)>>5;

	addr = *((uint32 *)(XDC_WAR_REG));

	seqN = (addr>>8)&0x1F;
	addr = addr&SLX_ADDR_MASK;

	if (addr && (val <= XDC_LINK_STATE_U2)) {
		xdc_dbg(("xdc restore addr = %d, seqN = %d\n", addr, seqN));
		xdc_connect_event(uxdc->xdc, 0);
		xdc_set_address(uxdc->xdc, addr);
		usbdev_setcfg(uxdc->bus, 1);
		if (seqN) {
			ep = &uxdc->xdc->ep[usbdev_getwlbulkout(uxdc->bus)];
			if (ep->num) {
				xdc_set_SeqN(uxdc->xdc, ep->num, ep->dir, seqN);
			}
		}
		uxdc->pwmode = (uchar)R_REG(uxdc->osh, (uint32*)XDC_WAR_REG1);
		uxdc->xdc->dev_state = XDC_STATE_CONFIGURED;
	} else {
		xdci_connect(uxdc->xdc);
	}
	return 0;
}

/**
 * For test purposes, the user can force the device to wake up using the serial console. This
 * function is called when the RTOS (RTE) calls the 'resume' handler of the attached device.
 */
int xdci_resume(struct usbdev_chip *ch)
{
	struct usbdev_chip *uxdc = ch;
	struct xdcd *xdc = uxdc->xdc;
	uint32 portsc, temp;

	xdc_trace(("xdc_resume()\n"));

	if (uxdc->remote_wakeup == 0) {
		xdc_dbg_ltssm(("Return from xdci_resume\n"));
		return BCME_ERROR;
	}

#ifdef XDCI_MONITOR_LTSSM_WAR
	if (xdc->speed == USB_SPEED_SUPER) {
		if (xdci_get_otp_var_u1u2(xdc) & XDCI_OTP_VAR_ENABLE_LTSSM_U3_REMOTE)
			xdci_u3exit_remote1(xdc);
	}
#endif
	portsc = R_REG(xdc->osh, &xdc->opregs->portsc);
	xdc_dbg(("portsc = 0x%08x\n", portsc));
	temp = portsc & (~XDC_PORTSC_PLS);
	temp |= XDC_PORTSC_LWS;
	W_REG(xdc->osh, &xdc->opregs->portsc, temp);
	printf("xdc_resume portsc=%08x %08x", portsc, temp);
	if (xdc->speed == USB_SPEED_SUPER) {
#ifdef XDCI_MONITOR_LTSSM_WAR
		if (xdci_get_otp_var_u1u2(xdc) & XDCI_OTP_VAR_ENABLE_LTSSM_U3_REMOTE)
			xdci_u3exit_remote3(xdc);
#endif
		if (uxdc->suspended) {
			uxdc->remote_wakeup_issued = 1;
		}
	}
	return BCME_OK;
} /* xdci_resume */

/** Returns supported USB revision (2.0, 2.1, 3) */
int xdci_usb30(struct usbdev_chip *ch)
{
	struct usbdev_chip *uxdc;
	uxdc = ch;

	if (uxdc->id == USB30D_CORE_ID)
	{
		if (uxdc->xdc->speed <= USB_SPEED_HIGH)
			return UD_USB_2_1_DEV;
		else
			return UD_USB_3_DEV;
	} else {
		return UD_USB_2_0_DEV;
	}
}


bool
xdci_match(uint vendor, uint device)
{
	xdc_trace(("xdci_match %x", device));
	if (vendor != VENDOR_BROADCOM)
		return FALSE;

	switch (device) {
	case BCM47XX_USBD_ID:
	case BCM47XX_USB20D_ID:
	case BCM47XX_USB30D_ID:
		return TRUE;
	}

	return FALSE;
}

/** Used by composite USB (VUSB) firmware. */
void xdci_ep_stall(struct usbdev_chip *ch, int ep_index)
{
	struct usbdev_chip *uxdc;
	struct xdcd *xdc;
	struct xdc_ep *ep;
	uint32 ep_num;

	uxdc = ch;
	xdc = uxdc->xdc;
	ep = &xdc->ep[ep_index];

	ep_num = ep->num;

	xdc_trace(("ep_stall\n"));
	if (ep_num == 0) {
		xdc_send_NAK(xdc);
		return;
	}

	xdc_halt_ep(ep, 1);
}

/** Called by e.g. bootloader on hard disconnect */
void xdci_ep_detach(struct usbdev_chip *ch, int ep_index)
{
	struct usbdev_chip *uxdc;
	struct xdc_ep *ep;

	uxdc = ch;
	ep = &uxdc->xdc->ep[ep_index];

	xdc_trace(("ep_detach\n"));
	/* Disable endpoint */
	xdc_disable_ep(ep);

	dbg("done");

	return;
}

/** ep_attach() and called by usbdev_setcfg */
uint32 xdci_ep_attach(struct usbdev_chip *ch, const usb_endpoint_descriptor_t *endpoint,
	const usb_endpoint_companion_descriptor_t *sscmp,
	int config, int interface, int alternate)
{
	struct usbdev_chip *uxdc;
	struct xdcd *xdc;
	uint32 ep_index;
	int ret = 0;

	xdc_trace(("xdc_ep_attach ch = %p\n", ch));

	xdc_dbg(("des = %p, config = %d, interface = %d, alternate = %d \n",
		endpoint, config, interface, alternate));

	uxdc = ch;
	xdc = uxdc->xdc;


	ret = xdc_enable_ep(xdc, endpoint, sscmp);

	if (ret < 0)
		xdc_err(("ep attach failed ret = %d \n", ret));

	ep_index = (uint32)ret;

	xdc_trace(("xdc_ep_attach done\n"));
	return ep_index;
}

/** Called when the USB30d core signals an interrupt */
int xdci_dpc(struct usbdev_chip *ch)
{
	struct usbdev_chip *uxdc;
	struct xdcd *xdc;
	int resched;

	resched = 0;

	uxdc = ((struct usbdev_chip *)ch);
	xdc = uxdc->xdc;

	resched = xdc_event(xdc, 1);

	return resched;
}

/** This route is ISR for USB interrupt to WLAN ARM */
int xdci2wlan_isr(struct usbdev_chip *ch)
{
	uint32 usbintstatus;
	uint32 usbintwlanmask = 0x0F; /* b[0, 3] is core interrpt for rings */
	struct usbdev_chip *uxdc = (struct usbdev_chip *)ch;

	usbintstatus = R_REG(uxdc->osh, &uxdc->regs->intrstatus);

	if (!(usbintstatus&usbintwlanmask))
		return FALSE;

	W_REG(uxdc->osh, &uxdc->regs->intrstatus, usbintstatus);

	xdc_trace(("xdci2wlan_isr = %x\n", usbintstatus));

	if (!uxdc->up) {
		return FALSE;
	}


#ifndef XDC_EVT_POLLING
	return xdc_core_isr(ch);
#else
	xdc_core_isr(ch);
	return TRUE;
#endif /* XDC_EVT_POLLING */
} /* xdci2wlan_isr */

/** direction: dongle -> host */
int xdci_tx(struct usbdev_chip *ch, int ep_index, void *p)
{
	return (xdc_tx(ch, ep_index, p));
}

/** Called once during firmware lifetime, e.g. when the host driver unloads */
void
BCMATTACHFN(xdci_detach)(struct usbdev_chip *ch, bool disable)
{
	uint32 devcontrl;
	uint32 usb_nodis = 0;

	struct usbdev_chip *uxdc = (struct usbdev_chip *)ch;

	xdc_dbg(("xdci_detach\n"));



	/* Reset endpoints */
	xdc_stop_eps(uxdc->xdc);
	usbdev_reset(uxdc->bus);
	xdc_disable_irq(uxdc->xdc);
	xdci2wlan_irq_off(uxdc);

	/* Free device state */
	usbdev_detach(uxdc->bus);

	xdc_free_mem(uxdc->xdc);

	if (usb_nodis == 0) {
		/* Put the core back into reset */
		if (disable)
			si_core_disable(uxdc->sih, 0);


		/* Detach from SB bus */
		si_detach(uxdc->sih);

		/* Free chip state */
		xdci_soft_disconnect(uxdc->xdc);

		/* Assert Dev Reset */
		devcontrl = R_REG(uxdc->osh, &uxdc->regs->devcontrol);
		devcontrl |= B_0;
		W_REG(uxdc->osh, &uxdc->regs->devcontrol, devcontrl);
		OSL_DELAY(400);
	} else {
		xdc_stop(uxdc->xdc);
	}

	MFREE(uxdc->osh, uxdc->xdc->mem, (sizeof(struct xdcd)));
	MFREE(uxdc->osh, uxdc->mem, sizeof(struct usbdev_chip));

	xdc_trace(("xdci_detach done"));
} /* xdci_detach */

/** Called on firmware initialization, once during firmware lifetime */
struct usbdev_chip *
BCMATTACHFN(xdci_attach)(void *drv, uint vendor, uint device, osl_t *osh,
	volatile void *regs, uint bus)
{
	struct usbdev_chip *uxdc;
	struct xdcd *xdc;
	void *mem;
	char *vars;
	uint vars_len;
	int i;

	xdc_dbg(("xdci_attach regs %p\n", regs));

	/* check if usbd30 core exist */
	if (!xdci_match(vendor, device))
		return NULL;

	/* allocate usbdev_chip structure */
	if (!(mem = MALLOC_ALIGN(osh, sizeof(struct usbdev_chip), UXDC_ALIGN_BITS))) {
		xdc_err(("%s: out of memory, malloced %d bytes", __FUNCTION__, MALLOCED(osh)));
		ASSERT(0);
		return NULL;
	}

	uxdc = (struct usbdev_chip *)mem;

	xdc_dbg(("uxdc = %p\n", mem));

	bzero(uxdc, sizeof(struct usbdev_chip));
	uxdc->drv = drv;
	uxdc->osh = osh;
	uxdc->regs = (usb_xdc_shim_regs_t *) (regs + 0x1000);

	uxdc->mem = mem;

	/* allocate xdc structure */
	if (!(mem = MALLOC_ALIGN(osh, sizeof(struct xdcd), XDC_ALIGN_BITS))) {
		xdc_err(("%s: out of memory, malloced %d bytes", __FUNCTION__, MALLOCED(osh)));
		MFREE(osh, uxdc, sizeof(struct usbdev_chip));
		ASSERT(0);
		return NULL;
	}


	xdc = (struct xdcd *)mem;
	bzero(xdc, sizeof(struct xdcd));
	uxdc->xdc = xdc;
	uxdc->xdc->mem = mem;
	xdc->uxdc = uxdc;

	xdc->osh = osh;

	uxdc->device = device;
	xdc->event_fn[0] = xdc_xfer_event;
	xdc->event_fn[1] = xdc_cmd_event;
	xdc->event_fn[2] = xdc_port_event;

	/* Attach dci to bus */
	if (!(uxdc->sih = si_attach(device, osh, regs, bus, NULL, &vars, &vars_len)))
		goto err;

	uxdc->id = si_coreid(uxdc->sih);
	uxdc->rev = si_corerev(uxdc->sih);

	xdc_dbg(("core rev = %x, id = %x \n", uxdc->rev, uxdc->id));

	uxdc->non_disconnect = 0;

	/* Enable u1u2 by default, only for usbd30 rev 0x3 */
	if ((uxdc->id == 0x83d) && (uxdc->rev == 0x3)) {
		xdc_err(("xdci_attach xdc->u1u2_enable\n"));
		xdc->u1u2_enable = 1;
	} else {
		xdc->u1u2_enable = 0;
	}

#ifdef BCMUSB_NODISCONNECT
	uxdc->non_disconnect = 1;
	i = *((uint32 *)(XDC_WAR_REG));
	if (i == 0) {
		/* Bootloader indicate non_disconnect is not ready */
		uxdc->non_disconnect = 0;
	}
#endif

	if (uxdc->non_disconnect == 0) {
		xdc_init_hw(uxdc);
		OSL_DELAY(1000);
		xdci_regs_update(xdc);
	}


	xdc->cpregs = (struct xdc_cpregs *) regs;
	xdc_dbg(("xdc->cpregs: %p  ", xdc->cpregs));

	xdc->opregs = (struct xdc_opregs *) (regs +
		XDC_CAP_LEN(R_REG(xdc->osh, &xdc->cpregs->cpliver)));

	xdc_dbg(("xdc->opregs: %p  ", xdc->opregs));

	xdc->rtregs = (struct xdc_rtregs *) (regs +
		(R_REG(xdc->osh, &xdc->cpregs->rtreg_off) &
		RT_REGOFF_MASK));

	xdc->trbs_per_evtring = MAX_TRBS_EVTRING;
	xdc->trbs_per_cmdring = MAX_TRBS_CMDRING;
	xdc->trbs_per_txring = MAX_TRBS_TXRING;
	xdc->trbs_per_rxring = MAX_TRBS_RXRING;
	xdc->trbs_per_ctlring = MAX_TRBS_CTLRING;
	xdc->trbs_iso_txring = MAX_ISOC_TRBS_TXRING;
	xdc->trbs_iso_rxring = MAX_ISOC_TRBS_RXRING;
	xdc->trbs_intr_txring = MAX_INTR_TRBS_TXRING;
	xdc->trbs_intr_rxring = MAX_INTR_TRBS_RXRING;
	xdc->max_rsv_trbs = MAX_RXRSV_TRB;
	xdc->max_rxbuf_sz = MAX_RXBLK_SIZE;
	xdc->isoc_max_rsv_trbs = ISOC_MAX_RXRSV_TRB;
	xdc->isoc_max_rxbuf_sz = ISOC_MAX_RXBLK_SIZE;
	xdc->intr_max_rsv_trbs = INTR_MAX_RXRSV_TRB;
	xdc->intr_max_rxbuf_sz = INTR_MAX_RXBLK_SIZE;
	xdc->resch_event = MAX_RESCH_EVENT;

	xdc_err(("XDC : TX=%d RX=%d, RESCH=%d RSV=%d\n",
		MAX_TRBS_TXRING, MAX_TRBS_RXRING, MAX_RESCH_EVENT, MAX_RXRSV_TRB));

	if (uxdc->non_disconnect) {
		i = R_REG(xdc->osh, &xdc->opregs->portsc);
		xdc->speed = (i&PORT_SPEED_MASK)>>10;
		if (xdc->speed == XDC_PORT_SS) {
			uxdc->disconnect = 0;
		} else {
			xdc->speed = XDC_PORT_HS;
		}
		xdc_err(("connected speed = %d \n", xdc->speed));
	} else {
		xdc->speed = XDC_PORT_HS;
	}

	if (!(uxdc->bus = usbdev_attach(uxdc->osh, uxdc, uxdc->sih))) {
		/* Detach from SB bus */
		si_detach(uxdc->sih);
		goto err;
	}

	i = xdc_init_mem(xdc);
	if (i == 0) {
		xdc_trace(("xdci_attach done\n"));
		return uxdc;
	}

err:
	MFREE(osh, uxdc, sizeof(struct usbdev_chip));
	MFREE(osh, xdc->mem, (sizeof(struct xdcd)));
	uxdc->up = FALSE;

	return NULL;
} /* xdci_attach */

/** Called on firmware initialization, once during firmware lifetime */
uint32
BCMATTACHFN(xdci_init)(struct usbdev_chip *ch, bool disconnect)
{
	int ret;
	struct xdcd *xdc;
	struct xdc_ep *ep;
	struct usbdev_chip *uxdc = (struct usbdev_chip *)ch;

	xdc_trace(("xdci_init\n"));


	xdc = uxdc->xdc;

	ret = xdc_ep_init(xdc);

	xdc_ep0_desc.wMaxPacketSize = htol16(EP0_MAX_PKT_SIZE);
	ep = &xdc->ep[0];
	xdc->setup_fn[0] = xdc_get_setup_pkt; /* called on setup packet received */
	xdc->setup_fn[1] = xdc_setup_data;    /* called on data stage for a control transfer */
	xdc->setup_fn[2] = xdc_setup_status;  /* called on status stage for a control transfer */

	/* enable ep0 */
	ret = xdc_enable_ep(xdc, &xdc_ep0_desc, NULL);
	OSL_DELAY(1);

	if (ret) {
		xdc_err(("failed to enable EP0 %d\n", ep->index));
		ASSERT(0);
	}

	xdci2wlan_irq_on(uxdc);
	xdc_enable_irq(xdc);

	ret = xdc_start(xdc);
	if (ret) {
		xdc_err(("failed to start XDC \n"));
		ASSERT(0);
		return ret;

	}

	if (uxdc->non_disconnect == 1) {
		xdci_restore(uxdc);
	} else
	{
		W_REG(xdc->osh, (uint32 *)XDC_WAR_REG0, 0);
		xdci_connect(xdc);
	}

#if defined(BCMPKTPOOL_ENABLED)
	W_REG(xdc->osh, (uint32 *)XDC_WAR_REG11, (uint32)pktpool_shared);
#endif

	uxdc->up = TRUE;

	return ret;
} /* xdci_init */

bool
xdci_hsic(struct usbdev_chip *chip)
{
	return FALSE;
}

#endif /* USB_XDCI */
