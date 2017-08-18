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

#ifndef	_vusb_pub_h_
#define	_vusb_pub_h_

#define VUSBD_DEVICE_STATE_DISABLED		0x0
#define VUSBD_DEVICE_STATE_ACTIVE		0x1
#define VUSBD_DEVICE_STATE_SUSPEND_PENDGIN	0x2
#define VUSBD_DEVICE_STATE_SUSPENDED		0x3
#define VUSBD_DEVICE_STATE_MASK			0x00ff

#define VUSBD_DEVICE_ADDR_MASK			0xff00
#define VUSBD_DEVICE_ADDR_SHIFT			8

/* max. number of endpoints in VUSBD, not including control ep */
#define VUSBD_MAX_EPS_3		3
#define VUSBD_MAX_EPS		5

#define VUSBD_EP_STATE_DISABLED		0x0
#define VUSBD_EP_STATE_ACTIVE		0x1
#define VUSBD_EP_STATE_STALL		0x2

#define VUSBD_BT2WLAN_EVENT_RESERVED	0x0
#define VUSBD_BT2WLAN_EVENT_ENABLE	0x1
#define VUSBD_BT2WLAN_EVENT_DISABLE	0x2
#define VUSBD_BT2WLAN_EVENT_RESET	0x3
#define VUSBD_BT2WLAN_EVENT_BUSRESET	0x4
#define VUSBD_BT2WLAN_EVENT_SUSPEND	0x5

#define VUSBD_WLAN2BT_EVENT_RESERVED 0x0
#define VUSBD_WLAN2BT_EVENT_ENABLE	 0x1
#define VUSBD_WLAN2BT_EVENT_DISABLE  0x2
#define VUSBD_WLAN2BT_EVENT_RESET	 0x3
#define VUSBD_WLAN2BT_EVENT_BUSRESET 0x4
#define VUSBD_WLAN2BT_EVENT_SUSPEND	 0x5
#define VUSBD_WLAN2BT_EVENT_RESUME	 0x6
#define VUSBD_WLAN2BT_EVENT_ALTSET	 0x7


#define VUSBD_RINGSIZE_DFT		8

/*
 * Endpoint index usage
 * 0: Interrupt
 * 1: Bulk-in
 * 2: Bulk-out
 * 3: ISO-in
 * 4: ISO-out
*/

#define VUSBD_EP_INDEX_INTR		0
#define VUSBD_EP_INDEX_BULK_IN		1
#define VUSBD_EP_INDEX_BULK_OUT		2
#define VUSBD_EP_INDEX_ISO_IN		3
#define VUSBD_EP_INDEX_ISO_OUT		4

/*
 * interrupt bits assignment
 * bit 0 - 4 for ep 0 - 4
 * bit 5-12	reservered
 * bit 13 	BT->WLAN evebt
 * bit 14	WLAN->BT event
 * bit 15 	control ep
 */

#define VUSBD_INT_BT2WLAN_EVENT	13
#define VUSBD_INT_WLAN2BT_EVENT	14
#define VUSBD_INT_CONTROL	15

#define VUSBD_EVENT_BT_RESERVED 0
#define VUSBD_EVENT_BT_ENABLE 	1
#define VUSBD_EVENT_BT_DISABLE 	2
#define VUSBD_EVENT_BT_RESET 	3

#define VUSBD_RING_SIZE_DTF	32
#define VUSBD_RING_SIZE_DTF_8	8
#define VUSBD_RING_SIZE_DTF_16	16
#define VUSBD_CTRL_RING_SIZE_DTF 4

#define VUSBD_DIR_WLAN_2_BT	0
#define VUSBD_DIR_BT_2_WLAN	1
#define VUSBD_BT_BULK_SIZE	64

#define VUSBD_MAGIC	0x55aa00ff

struct vusbd_ep_desc_ring {
	/* which endpoint this ring is associated with */
	uint8	ep_index;
	/* ring size, need be power of 2, 256 max */
	uint8	ring_size;
	/* direction 0: WLAN->BT 1:BT->WLAN */
	uint8	direction;
	/*
	 * Producer index. For WLAN->BT ring. WLAN is the owner, read only to BT
	 * for BT->WLAN ring, BT is the owner, read only to WLAN
	 */
	uint8 prod; /* prod index */
	/*
	 * Consumer index. For WLAN->BT ring. BT is the owner, read only to WLAN
	 * for BT->WLAN ring, WLAN is the owner, read only to BT
	 */
	uint8 cons; /* cons index */
	uint8 completed;
	/* Descriptor ring address, may need remapping before use */
	uint32 desc; /* event ring don't use this field */
};

struct vusbd_ep_transfer_desc {
	/* length of data */
	uint16 len;
	/* length transfer completed */
	uint16 complete_len;
	/*
	 * completion status TX descriptors (IN endpoints). Filled by WLAN after
	 * transaction is done
	 * 0: transfter completed succssfully
	 * 1: transfter failed
	*/
	uint16 status;
	/*
	 *if len is 0, then this descriptor contains a command and has no buffers
	 * command 0: send zero length packet
	 * command 1: STALL the endpoint
	 */
	uint16 command;
	uint32 buf; /* Address of the buffer, may need remapped before use */
	void * pkt; /* used by WLAN only, used to point to the original pkt */
};

/*
* OUT descriptor for control transfer: WLAN->BT
* contains setup packet
* contains data packet for setup transfer with OUT data phase
*/
struct vusbd_control_ep_out_desc {
	/* 1: setup with IN data phase, 0: setup with OUT data phase */
	uint16 type; /*  bmRequestType */
	/* length of setup packet */
	uint16 setup_len;
	/* length of the data packet in data phase */
	uint16 data_len;
	/* used to sync up with the vusb_control_ep_in_desc for response phase */
	uint16 seq;
	 /* address of the buffer holding the setup packet, maybe need remap before use */
	uint32 setup;
	/* address  of the buffer holding the data packet. Maybe need before use */
	uint32 data;
	/*  used by WLAN only, used to save setup data */
	uint8 setup_pkt[8];
	/*  used by WLAN only, used to point to the original data pkt */
	void *data_pkt;
};

/*
* IN descriptor for control transfer: BT -> WLAN
* contains setup packet
* contains data packet for setup transfer with OUT data phase
*/
struct vusbd_control_ep_in_desc {
	/* 0: setup with IN data phase, 1: setup with OUT data phase */
	uint16 type;
	/*
	 * response for the status phase for setup with OUT data phase
	 * 0: send zero length packet
	 * 1: STALL
	 * 2: NAK
	*/
	uint16 response;
	/* used to sync up with the vusb_control_ep_in_desc for response phase */
	uint16 seq;
	/* length of the data packet in data phase */
	uint16 data_len;
	/* address  of the buffer holding the data packet. Maybe need before use */
	uint32 data;
};

struct vusbd_BT_stats {
	uint16	int_set_cnt;
	uint16	int_clear_cnt;
	uint16	data_ep_trans_enq_cnt[VUSBD_MAX_EPS];
	uint16	data_ep_trans_deq_cnt[VUSBD_MAX_EPS];
	uint16	ctrl_ep_trans_enq_cnt;
	uint16	ctrl_ep_trans_deq_cnt;
	uint16	event_enq_cnt;
	uint16	event_deq_cnt;
	uint32	data_ep_byte_send[VUSBD_MAX_EPS];
	uint32	data_ep_byte_recv[VUSBD_MAX_EPS];
};

struct vusbd_WLAN_stats {
	uint16	int_set_cnt;
	uint16	int_clear_cnt;
	uint16	data_ep_trans_enq_cnt[VUSBD_MAX_EPS];
	uint16	data_ep_trans_deq_cnt[VUSBD_MAX_EPS];
	uint16	ctrl_ep_trans_enq_cnt;
	uint16	ctrl_ep_trans_deq_cnt;
	uint16	event_enq_cnt;
	uint16	event_deq_cnt;
	uint32	data_ep_byte_send[VUSBD_MAX_EPS];
	uint32	data_ep_byte_recv[VUSBD_MAX_EPS];
};

struct vusbd_pub {
	uint32	magic1;
	/* Registers */
	/* bit 0-7 state; bit 8-15: device address */
	volatile uint16 device_status;
	/* bit 0-3: Number of IN endpoints, bit 4-7: number of OUT ep */
	volatile uint16 configure;
	/* WLAN->BT interrupt: one bit per endpint index */
	volatile uint16 WLAN_intstatus;
	/* BT -> WLAN  interrupt: one bit per endpint index */
	volatile uint16 BT_intstatus;
	/* ep state */
	volatile uint8 endpoint_status[VUSBD_MAX_EPS];
	volatile uint8 endpoint_addr[VUSBD_MAX_EPS];
	/* data endpoints  */
	struct vusbd_ep_desc_ring ep_ring[VUSBD_MAX_EPS];
	/* control endpoints  */
	/* WLAN->BT ring for control transfer */
	struct vusbd_ep_desc_ring control_ep_out;
	/* BT->WLAN ring for control transfer */
	struct vusbd_ep_desc_ring control_ep_in;
	/* event send from wlan to BT */
	struct vusbd_ep_desc_ring event_q_to_bt;
	/* event send from BT to WLAN */
	struct vusbd_ep_desc_ring event_q_to_wlan;
	uint32 bt_2_wlan_events[VUSBD_RING_SIZE_DTF];
	uint32 wlan_2_bt_events[VUSBD_RING_SIZE_DTF];
	struct vusbd_BT_stats bt_stats;
	struct vusbd_WLAN_stats wlan_stats;
	uint32	magic2;
};
#endif /* _vusb_pub_h_ */
