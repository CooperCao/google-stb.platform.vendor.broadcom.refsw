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
 * $Id: xdc_rte.h 406060 2013-06-05 23:13:52Z $
 */

#ifndef _xdc_rte_h_
#define _xdc_rte_h_

#define XDC_ALIGN_MASK   	(16 - 1)
#define XDC_ALIGN_BITS		4
#define UXDC_ALIGN_BITS		2
#define XDC_REQALIGN_BITS	10 /* 1024 */
#define XDC_DCBAA_ALIGN_BITS	6 /* 64 */

#define XDCI_OTP_VAR_U1U2_MASK               0xf   /**< Bits 0-3 for U1U2 */
#define XDCI_OTP_VAR_COMMA_RELOCK_MASK       0x10  /**< Bit to enable Comma relock */
#define XDCI_OTP_VAR_CDR_CLAMP_MASK          0x20  /**< Bit to enable CDR clamp */
#define XDCI_OTP_VAR_ENABLE_PIPE_RESET_MASK  0x40  /**< Bit to enable pipe reset WAR */
#define XDCI_OTP_VAR_EXTEND_LFPS_MASK        0x80  /**< Bit to enable longer LFPS */
/** Enable monitor LTSSM/reset of CDR Phy on connect */
#define XDCI_OTP_VAR_ENABLE_LTSSM_CONNECT   0x100
/** Enable monitor LTSSM/reset of CDR Phy on U3 exit */
#define XDCI_OTP_VAR_ENABLE_LTSSM_U3                0x200
/** Enable monitor LTSSM/reset of CDR Phy during bus reset */
#define XDCI_OTP_VAR_ENABLE_LTSSM_SR                0x400
/** Enable WAR for U3 remote wake up */
#define XDCI_OTP_VAR_ENABLE_LTSSM_U3_REMOTE         0x800
/** Enable U1 Entry */
#define XDCI_OTP_VAR_ENABLE_U1_ENTRY_MASK           0x1000


#define XDCI_PORT_U1TO(p)    ((p) & 0xff)
#define XDCI_PORT_W1S   (1 << 17)
#define XDCI_PORT_U1E   (1 << 30)

int xdc_ctrl_event(struct xdcd *xdc, struct xdc_urb *,
                           struct xdc_trb *tx_trb,
                           struct xdc_trb *event,
                           uint32 ep_index);
int xdc_bulk_event(struct xdcd *xdc, struct xdc_urb *,
                           struct xdc_trb *tx_trb,
                           struct xdc_trb *event,
                           uint32 ep_index);

void xdc_urb_isocin_done(struct xdcd *xdc, uint32 ep_index, struct xdc_urb *urb);
void xdc_urb_isocout_done(struct xdcd *xdc, uint32 ep_index, struct xdc_urb *urb);
void xdc_urb_bulkin_done(struct xdcd *xdc, uint32 ep_index, struct xdc_urb *urb);
void xdc_urb_bulkout_done(struct xdcd *xdc, uint32 ep_index, struct xdc_urb *urb);
void xdc_urb_ctrl_done(struct xdcd *xdc, uint32 ep_index, struct xdc_urb *urb);
int xdc_urb_dequeue(struct xdc_ep *_ep, struct xdc_urb *_urb);
void xdc_get_setup_pkt(struct xdcd *xdc, struct xdc_trb *event);
void xdc_setup_data(struct xdcd *xdc, struct xdc_trb *event);
void xdc_setup_status(struct xdcd *xdc, struct xdc_trb *event);
int xdc_xfer_event(struct xdcd *xdc, struct xdc_trb *event);
int xdc_cmd_event(struct xdcd *, struct xdc_trb *);
int xdc_port_event(struct xdcd *xdc, struct xdc_trb *event);
int xdc_ep_init(struct xdcd *xdc);
int xdc_queue_rxbuf(struct xdc_ep *ep, struct xdcd *xdc, void *p);
int xdc_halt_ep(struct xdc_ep *, int);
int xdc_send_NAK(struct xdcd *xdc);
int xdc_disable_ep(struct xdc_ep *ep);
int xdc_enable_ep(struct xdcd *xdc,
	const usb_endpoint_descriptor_t *desc,
	const usb_endpoint_companion_descriptor_t *sscmp);
int xdc_tx(void *ch, int ep_index, void *);
#ifndef LOOP_BACK
void xdc_rx(struct usbdev_chip *uxdc, int ep_index, void *p);
#endif
int xdci_force_speed(struct usbdev_chip *uxdc);
void xdci_set_otp_var_u1u2(struct usbdev_chip *uxdc);
uint32 xdci_get_otp_var_u1u2(struct xdcd *xdc);
uint32 xdci_indirect_read(struct xdcd *xdci, int offset);
void xdci_indirect_write(struct xdcd *xdci, int offset, uint32 val);
uint32 xdci_get_link_state(struct xdcd *xdci);
#ifdef XDCI_MONITOR_LTSSM_WAR
void xdci_scale_timer(struct xdcd *xdci, uint32 timer_val);
void xdci_set_cdr_reset_eco(struct xdcd *xdci, int enable);
void xdci_u3exit_remote1(struct xdcd *xdci);
void xdci_u3exit_remote2(struct xdcd *xdci);
void xdci_u3exit_remote3(struct xdcd *xdci);
void xdci_monitor_ltssm(struct xdcd *xdci);
void xdci_set_lfps_duration(struct xdcd *xdci, int duration);
void xdci_monitor_ltss_on_u3exit(struct xdcd *xdci);
void xdci_ltssm_reset_histogram(struct xdcd *xdci);
void xdci_ltssm_dump_histogram(struct xdcd *xdci);
void xdci_poll_ltssm(void);
#endif
int xdc_function_wake(struct xdcd *xdc);
#endif /* xdc_rte.h */
