/*
 * Virtual USB device header
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 * $Id$
 */

#ifndef	_vusb_h_
#define	_vusb_h_

#define VUSBD_PKTQ_LEN 				50
#define VUSBD_BULK_OUT_NUM_FRAG		2	/* number of fragments */
#define VUSBD_A2DP_BULK_OUT_LEN		681

#if BCMCHIPID == BCM43242_CHIP_ID

#define VUSBD_BT_REGION_ADDR 		0x19000000
#define VUSBD_BT_GO_POLL_VAL		0x000b0201
#define VUSBD_WL_MEM_MAP_LIMIT		0x80000 /* address limit that can be mapped to BT */
#define VUSBD_BT_2_WL_INT_BIT		0x0 /* write 1 to interrupt WLAN, write 0 to clear it  */
#define VUSBD_WL_2_BT_INT_BIT		0x1 /* write 1 to interrupt BTFM, write 0 to clear it  */
#define VUSBD_BT_2_WL_INT_NUM		5
#define VUSBD_BT_PORT_ADDR 			(BT_ADDR_TO_WLAN(0x00202868))
#define VUSBD_WLAN_BT_INTR_PORT		(BT_ADDR_TO_WLAN(0x00640098))
#define VUSBD_BT_WLAN_INTR_PORT		(BT_ADDR_TO_WLAN(0x00640098))
#define VUSBD_BT_GO_POLL			(BT_ADDR_TO_WLAN(0x002009e0))

#elif  BCM4350_CHIP(BCMCHIPID)

#ifdef VUSB_4350b1
#define VUSBD_BT_REGION_ADDR 		0x19000000
#define VUSBD_BT_GO_POLL_VAL		0x000d0201
#define VUSBD_WL_MEM_MAP_LIMIT		0x240000 /* address limit that can be mapped to BT */
#define VUSBD_BT_2_WL_INT_BIT		0x0 /* write 1 to interrupt WLAN, write 0 to clear it  */
#define VUSBD_WL_2_BT_INT_BIT		0x1 /* write 1 to interrupt BTFM, write 0 to clear it  */
#define VUSBD_BT_2_WL_INT_NUM		7
#define VUSBD_BT_PORT_ADDR 			(BT_ADDR_TO_WLAN(0x00202c5c))
#define VUSBD_WLAN_BT_INTR_PORT		(BT_ADDR_TO_WLAN(0x006400a0))
#define VUSBD_BT_WLAN_INTR_PORT		(BT_ADDR_TO_WLAN(0x006400a0))
#define VUSBD_BT_GO_POLL			(BT_ADDR_TO_WLAN(0x00200b20))
#else
#define VUSBD_BT_REGION_ADDR 		0x19000000
#define VUSBD_BT_GO_POLL_VAL		0x000d0201
#define VUSBD_WL_MEM_MAP_LIMIT		0x240000 /* address limit that can be mapped to BT */
#define VUSBD_BT_2_WL_INT_BIT		0x2 /* write 1 to interrupt WLAN, write 0 to clear it  */
#define VUSBD_WL_2_BT_INT_BIT		0x1 /* write 1 to interrupt BTFM, write 0 to clear it  */
#define VUSBD_BT_2_WL_INT_NUM		11
#define VUSBD_BT_PORT_ADDR 			(BT_ADDR_TO_WLAN(0x00200c74))
#define VUSBD_WLAN_BT_INTR_PORT		(BT_ADDR_TO_WLAN(0x006400a0))
#define VUSBD_BT_WLAN_INTR_PORT		(BT_ADDR_TO_WLAN(0x006400a4))
#define VUSBD_BT_GO_POLL			(BT_ADDR_TO_WLAN(0x00200858))
#endif /* VUSB_4350b1 */

#endif /* BCMCHIPID */

#define G_BT_RDY					(VUSBD_BT_PORT_ADDR)
#define BT_ADDR_TO_WLAN(x)			(VUSBD_BT_REGION_ADDR + (x))

#define VUSBD_READ_MEM(addr) 		(*((volatile uint32 *)(addr)))
#define VUSBD_WRITE_MEM(addr, value)(*((volatile uint32 *)(addr)) = ((uint32)(value)))


#define NEXT_INDEX(index, ring_size) (((index) + 1) & (ring_size - 1))
#define AVAILABLE_SPACE(ring) ((ring)->ring_size - \
	(((ring)->prod - (ring)->cons) & (ring->ring_size - 1)))

/* Vusbd debug macros */

#define VUSBD_ERROR		0x00000100
#define VUSBD_TRACE		0x00000200
#define VUSBD_DEBUG		0x00000400
#define VUSBD_PRHEX		0x00000800
#define VUSBD_INFORM	0x00001000

#define vusbd_print(bit, fmt, args...) do { \
	extern int vusbd_msglevel; \
	if (vusbd_msglevel & (bit)) \
		printf("%s: " fmt "\n", __FUNCTION__ , ## args); \
} while (0)

/* Debug functions */
#if defined(VUSBD_DBG)
#define vusbd_err(fmt, args...) 	vusbd_print(VUSBD_ERROR, fmt , ## args)
#define vusbd_trace(fmt, args...) 	vusbd_print(VUSBD_TRACE, fmt , ## args)
#define vusbd_dbg(fmt, args...) 	vusbd_print(VUSBD_DEBUG, fmt , ## args)
#define vusbd_hex(msg, buf, len) 	do { \
								extern int vusbd_msglevel; \
								if (vusbd_msglevel & (VUSBD_PRHEX))\
									prhex(msg, buf, len); \
								} while (0)
#else
#define vusbd_err(fmt, args...)
#define vusbd_trace(fmt, args...)
#define vusbd_dbg(fmt, args...)
#define vusbd_hex(msg, buf, len)
#endif


struct vusbdev_endpoint {
	const	usb_endpoint_descriptor_t *descriptor;
	uint16	mps;
	uint8	address;
	uint8	attributes;
	uint8	hw_ep;	/* the hardware endpoint this ep is mapped to */
	struct pktq pktq;
};

struct vusbd {
	struct vusbd_pub vusbd_pub;
	/* struct used only by wlan */
	osl_t *osh;
	struct dngl_bus *bus;
	struct vusbdev_endpoint data_ep[VUSBD_MAX_EPS];
	struct vusbdev_endpoint control_ep;
	uint8  ep_map[EP_MAX];	/* hardware ep to virtual ep mapping */
	uint8  ctrl_out_pending;
	uint32 control_transfer_seq;
	void   (*post_vusbd)(void);
	bool   usb30d;
	bool   bt_dl_inprogress;
};

extern struct vusbd *vusbd_attach(osl_t *osh, struct dngl_bus *bus, bool usb30d);
extern void vusbd_detach(struct vusbd *vusbd);
extern int vusbd_enable(struct vusbd *vusbd, uint8 device_addr);
extern int vusbd_disable(struct vusbd *vusbd, uint8 device_addr);
extern void vusbd_ep_attach(struct vusbd *vusbd, const usb_endpoint_descriptor_t *ep_desc,
	uint8 ep);
extern int vusbd_ep_disable(struct vusbd *vusbd, uint8 ep);
extern int vusbd_send(struct vusbd *vusbd, int ep_index, void *p);
extern int vusbd_recv(struct vusbd *vusbd, uint8 hw_ep, void *p);
extern int vusbd_process_data_transfer(struct vusbd *vusbd, struct vusbd_ep_desc_ring *ep_ring);
extern int vusbd_handle_event(struct vusbd *vusbd, uint32 event);
extern int vusbd_process_bt2wlan_event(struct vusbd *vusbd);
extern void vusbd_intr(void *cbdata);
extern void vusbd_set_bt_interrupt(struct vusbd * vusbd);
extern void vusbd_process_control_transfer(struct vusbd *vusbd);
extern int vusbd_rx_complete(struct vusbd *vusbd, struct vusbd_ep_desc_ring *ep_ring);
extern int vusbd_setup(struct vusbd *vusbd, usb_device_request_t *dr, void *p);
extern int vusbd_ctrl_dispatch(struct vusbd *vusbd, void *p);
extern void *vusbd_malloc(osl_t *osh, int size);
extern void vusb_mem_release(struct vusbd * vusbd);
extern void vusbd_set_WLAN_2_BT_intr(struct vusbd *vusbd, uint8 int_bit);
extern void vusbd_clear_BT_2_WLAN_intr(struct vusbd *vusbd);
extern uint32 vusbd_hndrte_reftime_ms(void);
extern void vusbd_hndrte_set_reftime(uint32 reftime_ms);
extern int vusbd_get_hw_ep(struct vusbd * vusbd, int ep_index);
extern int usbdev_rx_dispatch(struct dngl_bus *bus, int ep, void *p);
extern void vusbd_send_setalt_event(struct vusbd *vusbd);
extern void vusbd_process_setalt(struct vusbd *vusbd, uint32 intf, uint32 altset);

#endif /* _vusbdev_h_ */
