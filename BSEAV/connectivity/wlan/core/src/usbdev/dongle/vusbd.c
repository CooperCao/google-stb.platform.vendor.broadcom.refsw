/*
 * Virtual USB device
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

#if defined(BCMUSBDEV_COMPOSITE) && defined(BCM_VUSBD)

#include <stddef.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmendian.h>
#include <bcmutils.h>
#include <bcmdevs.h>
#include <osl.h>
#include <siutils.h>
#include <wlioctl.h>
#include <usb.h>
#include <sbusbd.h>
#include <dngl_bus.h>
#include <dngl_api.h>
#include <usbdev.h>
#include <usbdev_dbg.h>
#include <usbrdl.h>
#include <sbusbd.h>
#include <vusbd_pub.h>
#include <vusbd.h>
#include <hnddma.h>
#include <rte_isr.h>

#if BCMCHIPID == BCM43242_CHIP_ID
#include "vusbd_43242_bt_patch.h"	/* compile in BT patch C array */
#elif  BCMCHIPID == BCM4350_CHIP_ID
#ifdef VUSB_4350c0
#include "vusbd_4350c0_bt_patch.h"	/* compile in BT patch C array */
#endif
#ifdef VUSB_4350b1
#include "vusbd_4350b1_bt_patch.h"	/* compile in BT patch C array */
#endif
#elif BCMCHIPID == BCM43569_CHIP_ID
#include "vusbd_4350c0_bt_patch.h"	/* compile in BT patch C array */
#else
#include "vusbd_4350c0_bt_patch.h"	/* compile in BT patch C array - Default */
#endif /* BCMCHIPID */

static struct vusbd *g_vusbd = NULL;
int vusbd_msglevel = VUSBD_ERROR;


/* If this macro was disabled, stack size should be increased to 65K */
#if BCMCHIPID == BCM43242_CHIP_ID
#define STATIC_VUSBD_MEMORY
#endif

#ifdef STATIC_VUSBD_MEMORY
struct vusbd vusbd_mem;
uint8  pdesc_mem[VUSBD_MAX_EPS_3][256];
uint8  ci_pdesc_mem[64];
uint8  co_pdesc_mem[112];
uint8  bo_data_ptr[VUSBD_RING_SIZE_DTF_16][1024];
uint8  co_data_ptr[VUSBD_CTRL_RING_SIZE_DTF][64];
#endif /* STATIC_VUSBD_MEMORY */
#ifdef VUSBD_EXPT
#define BUSYWAIT_DUR_OC		75
static int busywait_dur_oc = BUSYWAIT_DUR_OC;
#endif	/* VUSBD_EXPT */


#ifdef VUSBD_EXPT
/* busywait for BT open connection timeout */
static void busywait_oc(void)
{

	OSL_DELAY(busywait_dur_oc);
}
#else
#define busywait_oc()
#endif	/* VUSBD_EXPT */


#ifdef VUSBD_DBG
#define TSTR_MAX	600
char tstr[TSTR_MAX] = ""; int tstr_idx = 0;	/* trace string */


static uint32 host_reftime_ms = 0;
static uint32 dongle_time_ref = 0;

void
vusbd_hndrte_set_reftime(uint32 reftime_ms)
{
	host_reftime_ms = reftime_ms;
	dongle_time_ref = OSL_SYSUPTIME();
}

uint32
vusbd_hndrte_reftime_ms(void)
{
	uint32 dongle_time_ms;

	dongle_time_ms = OSL_SYSUPTIME() - dongle_time_ref;
	dongle_time_ms += host_reftime_ms;

	return dongle_time_ms;
}


/* trace print.  No wrap. */
static void tpr(const char *fmt, ...)
{
	va_list ap;
	int count;
	/* dongle ref time */
	uint32 dongle_time_ms;

	/* We know the 1st part consists of 7 characters; 1 is for \0
	 * Simplify by assuming the 2nd part is 8 characters or less.
	 */
	if ((tstr_idx + 7 + 1 + 8) >= TSTR_MAX) {
		return;
	}
	dongle_time_ms = vusbd_hndrte_reftime_ms();
	count = snprintf(tstr + tstr_idx, sizeof(tstr) - tstr_idx, "%02u.%03u ",
		dongle_time_ms / 1000, dongle_time_ms % 1000);
	va_start(ap, fmt);
	count += vsnprintf(tstr + tstr_idx + count, sizeof(tstr) - tstr_idx - count, fmt, ap);
	tstr_idx += count;
	va_end(ap);
}
#else
#define tpr(fmt, args...)
#endif	/* VUSBD_DBG */

static void vusbd_post_vusbd(void)
{
	vusbd_trace("G_BT_RDY is 0x%08x", VUSBD_READ_MEM(G_BT_RDY));

	if (VUSBD_READ_MEM(G_BT_RDY) == 0)
		return;

	/* pass vusbd address to BT */
	VUSBD_WRITE_MEM(VUSBD_BT_PORT_ADDR, g_vusbd);

	vusbd_set_WLAN_2_BT_intr(g_vusbd, VUSBD_INT_WLAN2BT_EVENT);

	vusbd_trace("pass to BT fw vusbd struct %x at %x", (uint32)g_vusbd, VUSBD_BT_PORT_ADDR);
}

#define BT_DL_STATE_IDLE 0
#define BT_DL_STATE_ADDR 1
#define BT_DL_STATE_EXPECT_DATA 2

uint16 bt_dl_state = BT_DL_STATE_IDLE;
uint32 bt_dl_address_base = 0x19000000;
uint32 bt_dl_address = 0;


int vusbd_bt_patch_dl(struct vusbd *vusbd, usb_device_request_t *dr, void *p);
int vusbd_dispatch_bt_dl(struct vusbd *vusbd, void *p);

int vusbd_dispatch_bt_dl(struct vusbd *vusbd, void *p)
{
	uint8 *data, *q;
	uint32 j = 0;

	printf("\n vusbd_dispatch_bt_dl len 0x%x", PKTLEN(vusbd->osh, p));
	data = (uint8 *)PKTDATA(vusbd->osh, p);
	q = (uint8 *)bt_dl_address;
	for (j = 0; j < PKTLEN(vusbd->osh, p); j++) {
		*q++ = *(data+j);
	}
	printf("\n addr 0x%x data 0x%x %x %x %x %x", (uint32)q, *data, *(data+1),
		*(data+2), *(data+3), *(data+4));
	bt_dl_state = BT_DL_STATE_IDLE;
	return 1;
}


int vusbd_bt_patch_dl(struct vusbd *vusbd, usb_device_request_t *dr, void *p)
{
	switch (dr->wValue) {
		case 0xF00D:
			printf("\n ADDR wIndex : 0x%x wLen : 0x%x", dr->wIndex, dr->wLength);
			bt_dl_address_base = 0x19000000 | ((dr->wIndex & 0xFF)<< 16);
			printf("\n Addr base 0x%x", bt_dl_address_base);
			bt_dl_state = BT_DL_STATE_ADDR;
			break;
		case 0xF01D:
			printf("\n DATA wIndex : 0x%x wLen : 0x%x", dr->wIndex, dr->wLength);
			bt_dl_address = bt_dl_address_base | (dr->wIndex & 0xFFFF);
			printf("\bt_dl_address 0x%x LEN 0x%x", bt_dl_address, dr->wLength);
			bt_dl_state = BT_DL_STATE_EXPECT_DATA;
			break;
		case 0xF02D:
			printf("\ndownload starts");
			vusbd->post_vusbd = vusbd_post_vusbd;
			VUSBD_WRITE_MEM(G_BT_RDY, 0);
	/* 		vusbd->bt_dl_inprogress = TRUE; */
			break;
		case 0xF03D:
			printf("\n download ends");
			VUSBD_WRITE_MEM(VUSBD_BT_GO_POLL, VUSBD_BT_GO_POLL_VAL);
			vusbd->bt_dl_inprogress = FALSE;
			break;
		default:
			return -1;
	}
	return 0;
}


static void BCMPREATTACHFN(vusbd_bt_patch_write)(struct vusbd *vusbd)
{
	uint32 addr;
	uint8 nbytes;
	uint8 *p, *q;
	int i, j;


	if ((VUSBD_READ_MEM(G_BT_RDY) & 0xFF) == 0x1) {
		vusbd_trace("G_BT_RDY addr 0x%08x content 0x%08x",
			G_BT_RDY, VUSBD_READ_MEM(G_BT_RDY));
		vusbd_trace("Bt patch already running");
		return;
	}

	vusbd_trace("G_BT_RDY addr 0x%08x content 0x%08x", G_BT_RDY, VUSBD_READ_MEM(G_BT_RDY));

	for (i = 0; i < sizeof(bt_patch_arr); ) {
		p = (uint8 *)bt_patch_arr+i;
		addr  = *p << 24; p++;
		addr |= *p << 16; p++;
		addr |= *p <<  8; p++;
		addr |= *p;       p++;
		nbytes = *p++;
		/* Write one byte at a time */
		q = (uint8 *)addr;
		for (j = 0; j < nbytes; j++) {
			*q++ = *p++;
		}
		i += 4 + 1 + nbytes;
	}
	vusbd_trace("Wrote bt patch");

	VUSBD_WRITE_MEM(VUSBD_BT_GO_POLL, VUSBD_BT_GO_POLL_VAL);

}

struct vusbd *BCMPREATTACHFN(vusbd_attach)(osl_t *osh, struct dngl_bus *bus, bool usb30d)
{
	struct vusbd *vusbd = NULL;
	int i;
	void *pdesc;
	struct vusbd_pub *pub;

	ASSERT(osh);
	ASSERT(bus);

#ifdef STATIC_VUSBD_MEMORY
	vusbd = &vusbd_mem;
#else
	vusbd = (struct vusbd *)vusbd_malloc(osh, sizeof(struct vusbd));
	if (vusbd == NULL) {
		vusbd_err("fail to alloc memory\n");
		return NULL;
	}
#endif
	ASSERT((uint32)vusbd < VUSBD_WL_MEM_MAP_LIMIT);
	bzero(vusbd, sizeof(struct vusbd));

	vusbd->bus = bus;
	vusbd->osh = osh;
	vusbd->usb30d = usb30d;
	vusbd->bt_dl_inprogress = FALSE;
	pub = &vusbd->vusbd_pub;
	pub->magic1 = VUSBD_MAGIC;

#ifdef STATIC_VUSBD_MEMORY
	for (i = 0; i < VUSBD_MAX_EPS_3; i++) {
		pub->ep_ring[i].ring_size = VUSBD_RING_SIZE_DTF_16;
#else
	for (i = 0; i < VUSBD_MAX_EPS; i++) {
		pub->ep_ring[i].ring_size = VUSBD_RING_SIZE_DTF;
#endif
		pub->ep_ring[i].ep_index = i;

		switch (i) {
		case VUSBD_EP_INDEX_INTR:
			pub->ep_ring[i].direction = VUSBD_DIR_BT_2_WLAN;
			break;

		case VUSBD_EP_INDEX_BULK_IN:
			pub->ep_ring[i].direction = VUSBD_DIR_BT_2_WLAN;
			break;

		case VUSBD_EP_INDEX_BULK_OUT:
			pub->ep_ring[i].direction = VUSBD_DIR_WLAN_2_BT;
			break;

		case VUSBD_EP_INDEX_ISO_IN:
			pub->ep_ring[i].direction = VUSBD_DIR_BT_2_WLAN;
			break;

		case VUSBD_EP_INDEX_ISO_OUT:
			pub->ep_ring[i].direction = VUSBD_DIR_WLAN_2_BT;
			break;

		default:
			break;
		}

		/* pre allocate all descriptors */
#ifdef STATIC_VUSBD_MEMORY
		pdesc = &(pdesc_mem[i]);
#else
		pdesc = (struct vusbd_ep_transfer_desc *)vusbd_malloc(osh,
			sizeof(struct vusbd_ep_transfer_desc) * pub->ep_ring[i].ring_size);
		if (pdesc == NULL) {
			vusbd_err("fail to alloc memory\n");
			goto fail;
		}
#endif
		ASSERT((uint32)pdesc < VUSBD_WL_MEM_MAP_LIMIT);
		pub->ep_ring[i].desc = (uint32)pdesc;
	}

	/* ring for control endpoint WLAN->BT */
	pub->control_ep_out.direction = VUSBD_DIR_WLAN_2_BT;
#ifdef STATIC_VUSBD_MEMORY
	pdesc = &(co_pdesc_mem);
	pub->control_ep_out.ring_size = VUSBD_CTRL_RING_SIZE_DTF;
#else
	pub->control_ep_out.ring_size = VUSBD_RING_SIZE_DTF;
	pdesc = vusbd_malloc(osh,
		sizeof(struct vusbd_control_ep_out_desc) * pub->control_ep_out.ring_size);
	if (pdesc == NULL) {
		vusbd_err("fail to alloc memory\n");
		goto fail;
	}
#endif
	ASSERT((uint32)pdesc < VUSBD_WL_MEM_MAP_LIMIT);
	pub->control_ep_out.desc = (uint32)pdesc;

	/* ring for control endpoint BT -> WLAN */
	pub->control_ep_in.direction = VUSBD_DIR_BT_2_WLAN;
#ifdef STATIC_VUSBD_MEMORY
	pdesc = &(ci_pdesc_mem);
	pub->control_ep_in.ring_size = VUSBD_CTRL_RING_SIZE_DTF;
#else
	pub->control_ep_in.ring_size = VUSBD_RING_SIZE_DTF;
	pdesc = vusbd_malloc(osh,
		sizeof(struct vusbd_control_ep_in_desc) * pub->control_ep_in.ring_size);
	if (pdesc == NULL) {
		vusbd_err("fail to alloc memory\n");
		goto fail;
	}
#endif
	ASSERT((uint32)pdesc < VUSBD_WL_MEM_MAP_LIMIT);
	pub->control_ep_in.desc = (uint32)pdesc;

	/* initialize event queue */
	pub->event_q_to_bt.direction = VUSBD_DIR_WLAN_2_BT;
	pub->event_q_to_bt.ring_size = VUSBD_RING_SIZE_DTF;

	pub->event_q_to_wlan.direction = VUSBD_DIR_BT_2_WLAN;
	pub->event_q_to_wlan.ring_size = VUSBD_RING_SIZE_DTF;

	pub->event_q_to_bt.ep_index = 0xFF;
	g_vusbd = vusbd;
	hnd_isr_register_n(0, VUSBD_BT_2_WL_INT_NUM, vusbd_intr, vusbd);

	pub->magic2 = VUSBD_MAGIC;

	vusbd->post_vusbd = vusbd_post_vusbd;

	vusbd_bt_patch_write(vusbd);

	return vusbd;

#ifndef STATIC_VUSBD_MEMORY
fail:
#endif
	vusbd_detach(vusbd);
	return NULL;
}

void vusbd_detach(struct vusbd *vusbd)
{
	struct vusbd_pub *pub;

	ASSERT(vusbd);
	pub = &vusbd->vusbd_pub;

#ifndef STATIC_VUSBD_MEMORY
	int i;
	for (i = 0; i < VUSBD_MAX_EPS; i++) {
		if (pub->ep_ring[i].desc) {
			MFREE(vusbd->osh, (void *)pub->ep_ring[i].desc,
				sizeof(struct vusbd_ep_transfer_desc)
					* pub->ep_ring[i].ring_size);
		}
	}
	if (pub->control_ep_in.desc) {
		MFREE(vusbd->osh, (void *)pub->control_ep_in.desc,
			sizeof(struct vusbd_control_ep_in_desc)
				* pub->control_ep_in.ring_size);
	}
	if (pub->control_ep_out.desc) {
		MFREE(vusbd->osh, (void *)pub->control_ep_out.desc,
			sizeof(struct vusbd_control_ep_out_desc) *
			pub->control_ep_out.ring_size);
	}
	MFREE(vusbd->osh, vusbd, sizeof(struct vusbd));
#endif /* STATIC_VUSBD_MEMORY */
	g_vusbd = NULL;
}

int vusbd_enable(struct vusbd *vusbd, uint8 device_addr)
{
	struct vusbd_pub *pub;

	ASSERT(vusbd->bus);
	pub = &vusbd->vusbd_pub;

	pub->device_status = VUSBD_DEVICE_STATE_ACTIVE;
	pub->device_status |= (device_addr <<  VUSBD_DEVICE_ADDR_SHIFT);
	vusbd_trace("vusbd device enabled\n");
	return 0;
}


int vusbd_disable(struct vusbd *vusbd, uint8 device_addr)
{

	struct vusbd_pub *pub;

	ASSERT(vusbd->bus);
	pub = &vusbd->vusbd_pub;

	pub->device_status &= (~VUSBD_DEVICE_STATE_MASK);
	pub->device_status |= VUSBD_DEVICE_STATE_DISABLED;
	vusbd_trace("vusbd device disabled\n");
	return 0;
}

void vusbd_ep_attach(struct vusbd *vusbd, const usb_endpoint_descriptor_t *ep_desc, uint8 ep)
{
	uint8 ep_index = 0;
	struct vusbd_pub *pub;

	ASSERT(vusbd->bus);
	pub = &vusbd->vusbd_pub;

	switch (UE_GET_XFERTYPE(ep_desc->bmAttributes)) {
	case UE_INTERRUPT:
		ASSERT(UE_GET_DIR(ep_desc->bEndpointAddress) == UE_DIR_IN);
		ep_index = VUSBD_EP_INDEX_INTR;
		vusbd->data_ep[ep_index].descriptor = ep_desc;
		vusbd->data_ep[ep_index].hw_ep = ep;
		vusbd->data_ep[ep_index].mps = 16;
		break;

	case UE_BULK:
		if (UE_GET_DIR(ep_desc->bEndpointAddress) == UE_DIR_OUT) {
			ep_index = VUSBD_EP_INDEX_BULK_OUT;
			pktqinit(&vusbd->data_ep[ep_index].pktq, VUSBD_PKTQ_LEN);
		} else {
			ep_index = VUSBD_EP_INDEX_BULK_IN;
		}
		vusbd->data_ep[ep_index].mps = VUSBD_BT_BULK_SIZE;
		vusbd->data_ep[ep_index].descriptor = ep_desc;
		vusbd->data_ep[ep_index].hw_ep = ep;
		break;

#ifdef STATIC_VUSBD_MEMORY
	default:
		vusbd_err("unsupport endpoint! %x, ep %d \n",
			ep_desc->bmAttributes, ep);
#else
	case UE_ISOCHRONOUS:
		if (UE_GET_DIR(ep_desc->bEndpointAddress) == UE_DIR_OUT) {
			ep_index = VUSBD_EP_INDEX_ISO_OUT;
		} else {
			ep_index = VUSBD_EP_INDEX_ISO_IN;
		}
		vusbd->data_ep[ep_index].descriptor = ep_desc;
		vusbd->data_ep[ep_index].hw_ep = ep;
		break;
	default:
		vusbd_err("unsupport endpoint! %x, ep %d \n",
			ep_desc->bmAttributes, ep);
		ASSERT(0);
#endif /* STATIC_VUSBD_MEMORY */
	}

	vusbd->data_ep[ep_index].attributes = ep_desc->bmAttributes;
	vusbd->data_ep[ep_index].address = ep_desc->bEndpointAddress;

	pub->endpoint_status[ep_index] = VUSBD_EP_STATE_ACTIVE;
	vusbd->ep_map[ep] = ep_index;
	vusbd_trace("vusbd ep_index %d attached to hw_ep %d\n", ep_index, ep);
}

int vusbd_ep_disable(struct vusbd *vusbd, uint8 ep)
{
	struct vusbd_pub *pub;

	uint8 ep_index = vusbd->ep_map[ep];
	pub = &vusbd->vusbd_pub;

	pub->endpoint_status[ep_index] = VUSBD_EP_STATE_DISABLED;
	return 0;
}

/* start BT->WLAN transfer */
int vusbd_send(struct vusbd *vusbd, int ep_index, void *p)
{
	ASSERT(vusbd->bus);
	ASSERT(p);
	ASSERT(vusbd->data_ep[ep_index].hw_ep);

	vusbd_trace("vusbd_send ep_index 0x%x ep 0x%x\n", ep_index, vusbd->data_ep[ep_index].hw_ep);

	ep_index += 32;
	return usbdev_bus_tx(vusbd->bus, p, ep_index);
}

/* start WLAN->BT transfer */
int vusbd_recv(struct vusbd *vusbd, uint8 hw_ep, void *p)
{
	uint8 ep_index;
	uint32 total_len = 0, transfer_size;
	struct vusbd_ep_desc_ring *ring;
	struct vusbd_ep_transfer_desc *desc, *desc_table;
	struct vusbd_pub *pub;
	int cnt = 0;
	int i;
	bool interrupt = FALSE;
	bool is_a2dp_bo = FALSE;

	ASSERT(vusbd);
	ASSERT(vusbd->bus);
	pub = &vusbd->vusbd_pub;

	if (vusbd->bt_dl_inprogress) {
		vusbd_dispatch_bt_dl(vusbd, p);
	}

	ASSERT(hw_ep < EP_MAX);

	ep_index = vusbd->ep_map[hw_ep];

#ifdef STATIC_VUSBD_MEMORY
	ASSERT(ep_index < VUSBD_MAX_EPS_3);
#else
	ASSERT(ep_index < VUSBD_MAX_EPS);
#endif

	ring = &pub->ep_ring[ep_index];

	ASSERT(ring->direction == VUSBD_DIR_WLAN_2_BT);
	desc_table = (struct vusbd_ep_transfer_desc *)ring->desc;
	ASSERT(desc_table);
	if (p) {
		pktenq(&vusbd->data_ep[ep_index].pktq, p);
		vusbd_trace("vusbd_recv ep %d, q len %d\n", ep_index,
			pktq_n_pkts_tot(&vusbd->data_ep[ep_index].pktq));
	}

	while ((p = pktqprec_peek(&vusbd->data_ep[ep_index].pktq, 0)) != NULL) {
		total_len  = PKTLEN(vusbd->osh, p);
		if (ep_index != VUSBD_EP_INDEX_BULK_OUT) {
			if (NEXT_INDEX(ring->prod, ring->ring_size) == ring->cons) {
				vusbd_err("ep ring %d full\n", ep_index);
				return 0;
			}
			p = pktdeq(&vusbd->data_ep[ep_index].pktq);
			desc = desc_table + ring->prod;
			ASSERT(desc);
			bzero(desc, sizeof(struct vusbd_ep_transfer_desc));
			desc->buf = (uint32)PKTDATA(vusbd->osh, p);
			desc->len = total_len;
			ring->prod = NEXT_INDEX(ring->prod, ring->ring_size);
			vusbd_trace("ep %d, send %d bytes to BT\n", ep_index, total_len);
			vusbd->vusbd_pub.wlan_stats.data_ep_trans_enq_cnt[ep_index]++;
			vusbd->vusbd_pub.wlan_stats.data_ep_byte_send[ep_index] += total_len;
			interrupt = TRUE;
		} else {
			if (AVAILABLE_SPACE(ring) <
				 ((total_len + VUSBD_BT_BULK_SIZE - 1) / VUSBD_BT_BULK_SIZE + 1)) {
				vusbd_err("_recv %d, no space %d %d %d\n",
					ep_index, ring->prod, ring->cons, AVAILABLE_SPACE(ring));
				return 0;
			}
			p = pktdeq(&vusbd->data_ep[ep_index].pktq);
			ASSERT(p);
			if (p) {
				total_len  = PKTLEN(vusbd->osh, p);
			} else {
				return 0;
			}
			if (total_len == VUSBD_A2DP_BULK_OUT_LEN) {
				is_a2dp_bo = TRUE;
			}

			for (i = 0; i < VUSBD_BULK_OUT_NUM_FRAG && total_len; i++) {
				if (i < (VUSBD_BULK_OUT_NUM_FRAG - 1))
					transfer_size = (total_len > VUSBD_BT_BULK_SIZE) ?
					VUSBD_BT_BULK_SIZE: total_len;
				else
					transfer_size = total_len;
				desc = desc_table + ring->prod;
				ASSERT(desc);
				bzero(desc, sizeof(struct vusbd_ep_transfer_desc));
#ifdef STATIC_VUSBD_MEMORY
				memcpy(&(bo_data_ptr[ring->prod]), PKTDATA(vusbd->osh, p),
					transfer_size);
				desc->buf = (uint32)(&(bo_data_ptr[ring->prod]));
#else
				desc->buf = (uint32)PKTDATA(vusbd->osh, p);
#endif
				desc->len = transfer_size;
				PKTPULL(vusbd->osh, p, transfer_size);
				total_len -= transfer_size;
				if (total_len == 0)
					desc->pkt = p;
				vusbd->vusbd_pub.wlan_stats.data_ep_trans_enq_cnt[ep_index]++;
				vusbd->vusbd_pub.wlan_stats.data_ep_byte_send[ep_index]
					+= transfer_size;
				cnt++;
				vusbd_trace("ep %d, send %d bytes to BT %d\n",
					ep_index, transfer_size, cnt);
				ring->prod = NEXT_INDEX(ring->prod, ring->ring_size);
				ASSERT(ring->prod != ring->cons);
				interrupt = TRUE;
			}

			if (total_len) {
				vusbd_err("ep %d %d overflow! %d bytes left",
					ep_index, hw_ep, PKTLEN(vusbd->osh, p));
			}
			ASSERT(total_len == 0);
		}
	}
	if (interrupt) {
		vusbd_set_WLAN_2_BT_intr(vusbd, ep_index);
	}
	busywait_oc();
	return 0;
}
int vusbd_rx_complete(struct vusbd *vusbd, struct vusbd_ep_desc_ring *ep_ring)
{
	struct vusbd_ep_transfer_desc  *desc_table, *desc;
	struct vusbd_pub *pub;
	bool interrupt = FALSE;

	vusbd_trace("vusbd_rx_complete\n");

	ASSERT(vusbd);
	pub = &vusbd->vusbd_pub;

	desc_table = (struct vusbd_ep_transfer_desc *)ep_ring->desc;
	ASSERT(desc_table);
	while (ep_ring->completed != ep_ring->cons) {
		desc = desc_table + ep_ring->completed;
		ASSERT(desc);
		if (desc->pkt) {
			PKTFREE(vusbd->osh, desc->pkt, 0);
			desc->pkt = NULL;
			desc->buf = 0;
		}
		vusbd_trace("ep %d, desc index %d, transfer %d bytes completed\n",
			ep_ring->ep_index, ep_ring->completed, desc->len);
		ep_ring->completed = NEXT_INDEX(ep_ring->completed, ep_ring->ring_size);
		interrupt = TRUE;
	}
	if (interrupt)
		vusbd_set_WLAN_2_BT_intr(vusbd, ep_ring->ep_index);

	if (pktq_n_pkts_tot(&vusbd->data_ep[ep_ring->ep_index].pktq))
		vusbd_recv(vusbd, vusbd->data_ep[ep_ring->ep_index].hw_ep, NULL);
	return 0;
}

int vusbd_process_data_transfer(struct vusbd *vusbd, struct vusbd_ep_desc_ring *ep_ring)
{
	uint8 index = ep_ring->ep_index;
	uint16 mps;
	struct vusbd_ep_transfer_desc  *desc_table, *desc;
	uint32 total_len = 0;
	void  *pkt;
	bool zlp_avoidance;
	struct vusbd_pub *pub;
	bool interrupt = FALSE;

	ASSERT(vusbd);
	pub = &vusbd->vusbd_pub;

	desc_table = (struct vusbd_ep_transfer_desc *)ep_ring->desc;
	ASSERT(desc_table);

	mps = vusbd->data_ep[index].mps;
	ASSERT(mps);

	vusbd_trace("vusbd_process_data_transfer cons: 0x%x prod: 0x%x\n",
		ep_ring->cons, ep_ring->prod);

	while (ep_ring->cons != ep_ring->prod) {
		desc = desc_table + ep_ring->cons;
		ASSERT(desc);
		ASSERT(desc->buf);

		zlp_avoidance = 0;
		total_len = desc->len;

		if (total_len % mps == 0) {
			total_len += 1;
			zlp_avoidance = 1;
			vusbd_trace("zlp_avoidance enable!\n");
		}

		pkt = PKTGET(vusbd->osh, total_len, TRUE);
		if (!pkt) {
			vusbd_err("fail to alloc packet %d bytes\n", total_len);
			/* discard pkt */
			ep_ring->cons = NEXT_INDEX(ep_ring->cons, ep_ring->ring_size);
			vusbd_set_WLAN_2_BT_intr(vusbd, index);
			return 0;
		}

		/* make sure the packet buffer  is not chained */
		ASSERT(PKTNEXT(vusbd->osh, pkt) == NULL);
		if (zlp_avoidance) {
			bcopy((void *)BT_ADDR_TO_WLAN(desc->buf),
				PKTDATA(vusbd->osh, pkt), total_len-1);
		}
		else {
			bcopy((void *)BT_ADDR_TO_WLAN(desc->buf),
				PKTDATA(vusbd->osh, pkt), total_len);
		}
		PKTSETLEN(vusbd->osh, pkt, total_len);

		ep_ring->cons = NEXT_INDEX(ep_ring->cons, ep_ring->ring_size);
		interrupt = TRUE;

		vusbd->vusbd_pub.wlan_stats.data_ep_trans_deq_cnt[index]++;
		vusbd->vusbd_pub.wlan_stats.data_ep_byte_recv[index] += desc->len;
		vusbd_hex("IN:", (uint8 *)PKTDATA(vusbd->osh, pkt), desc->len);
		vusbd_trace("send to host Len 0x%x zlp_av: 0x%x\n", total_len, zlp_avoidance);

		vusbd_send(vusbd, index, pkt);
	}
	if (interrupt)
		vusbd_set_WLAN_2_BT_intr(vusbd, index);
	return 0;
}

int vusbd_handle_event(struct vusbd *vusbd, uint32 event)
{
	ASSERT(vusbd);
	switch (event) {
	case	VUSBD_BT2WLAN_EVENT_ENABLE:
		vusbd->post_vusbd = NULL;
		break;

	case	VUSBD_BT2WLAN_EVENT_DISABLE:
		break;

	case	VUSBD_BT2WLAN_EVENT_RESET:
		break;

	case	VUSBD_BT2WLAN_EVENT_BUSRESET:
		break;

	case	VUSBD_BT2WLAN_EVENT_SUSPEND:
		break;
	default:
		vusbd_err("unexpected event %x\n", event);
	}
	vusbd_trace("event %x\n", event);
	return 0;
}

int vusbd_process_bt2wlan_event(struct vusbd *vusbd)
{
	struct vusbd_ep_desc_ring *ring;
	uint32	event;
	struct vusbd_pub *pub;

	vusbd_trace("vusbd_process_bt2wlan_event\n");

	ASSERT(vusbd);
	pub = &vusbd->vusbd_pub;
	ring = &pub->event_q_to_wlan;

	while (ring->cons != ring->prod) {
		event = pub->bt_2_wlan_events[ring->cons];
		vusbd_trace("recv event %x\n", event);
		vusbd_handle_event(vusbd, event);
		ring->cons = NEXT_INDEX(ring->cons, ring->ring_size);
	}
	return 0;
}


static void vusbd_process_bt(struct vusbd *vusbd)
{
	int i;
	struct vusbd_ep_desc_ring *ep_ring;

	struct vusbd_pub *pub;

	ASSERT(vusbd);
	pub = &vusbd->vusbd_pub;

	if (vusbd == NULL) {
		vusbd_err("invalid vusbd instance!\n");
		return;
	}

	vusbd_trace("vusbd_process_bt\n");

	/* Post vusbd struct to BT fw */
	if (vusbd->post_vusbd)
		vusbd->post_vusbd();

	/* check device status */
	if (pub->device_status == VUSBD_DEVICE_STATE_DISABLED) {
		/*
		 * Device is disabled, there shouldn't be any event from BT
		 * set interrutp to BT to let it check device_status and shut down
		 */
		 /* Error Handling neeed */

	}

	/* check event from BT */
	vusbd_process_bt2wlan_event(vusbd);

	/* process all endpoint */
	/* control ep first */
	vusbd_process_control_transfer(vusbd);

	/* data eps */
#ifdef STATIC_VUSBD_MEMORY
	for (i = 0; i < VUSBD_MAX_EPS_3; i++) {
#else
	for (i = 0; i < VUSBD_MAX_EPS; i++) {
#endif
		ep_ring = &pub->ep_ring[i];
		if (pub->endpoint_status[i] != VUSBD_EP_STATE_ACTIVE)
			continue;
		if (ep_ring->direction == VUSBD_DIR_WLAN_2_BT) {
			vusbd_rx_complete(vusbd, ep_ring);
			if (ep_ring->cons == ep_ring->prod) {
				if (vusbd->vusbd_pub.WLAN_intstatus &
					(1 << pub->ep_ring[i].ep_index)) {
					vusbd->vusbd_pub.WLAN_intstatus &=
						(~(1 << pub->ep_ring[i].ep_index));
				}
			}
			continue;
		}
		/* BT->WLAN data endpoint */
		if (ep_ring->cons == ep_ring->prod)
			continue;
		vusbd_process_data_transfer(vusbd, ep_ring);
	}

	ep_ring = &pub->event_q_to_bt;
	if (ep_ring->cons == ep_ring->prod) {
		if (vusbd->vusbd_pub.WLAN_intstatus & (1 << VUSBD_INT_WLAN2BT_EVENT)) {
			vusbd->vusbd_pub.WLAN_intstatus &= (~(1 << VUSBD_INT_WLAN2BT_EVENT));
		}
	}
}

void vusbd_intr(void *cbdata)
{
	struct vusbd *vusbd = (struct vusbd *)cbdata;
	tpr("i");
	vusbd_clear_BT_2_WLAN_intr(vusbd);
	vusbd_process_bt(vusbd);
}


/*
 * return  -1 to stall the endpoint
 */
int vusbd_setup(struct vusbd *vusbd, usb_device_request_t *dr, void *p)
{
	struct vusbd_ep_desc_ring *ring;
	struct vusbd_control_ep_out_desc *desc;
	uint8 next_index;
	struct vusbd_pub *pub;

	ASSERT(vusbd);
	ASSERT(p);

	if (dr->bRequest == 0x1F) {
		return vusbd_bt_patch_dl(vusbd, dr, p);
	}

	pub = &vusbd->vusbd_pub;
	ring = &pub->control_ep_out;

	ASSERT(ring->direction == VUSBD_DIR_WLAN_2_BT);

	if (vusbd->ctrl_out_pending) {
		vusbd_err("get a new setup packet while the previous one is waiting for OUT \n");
	}
	vusbd->ctrl_out_pending = 0;

	vusbd_trace("get setup \n");
	/* free complete transfer first */
	while (ring->completed != ring->cons) {
		desc = (struct vusbd_control_ep_out_desc *)ring->desc;
		desc = desc + ring->completed;
		ASSERT(desc);
		if (desc->data_pkt) {
			PKTFREE(vusbd->osh, desc->data_pkt, 0);
			desc->data_pkt = NULL;
		}
		ring->completed = NEXT_INDEX(ring->completed, ring->ring_size);
	}
	vusbd_trace("ctl ep, move completed to %d, cons %d prod %d\n",
		ring->completed, ring->cons, ring->prod);
	if (NEXT_INDEX(ring->prod, ring->ring_size) == ring->cons) {
		vusbd_err("control out  ring full prod %d, cons %d, ring_size %d\n",
		ring->prod, ring->cons, ring->ring_size);
		/* This will STALL it */
		return -1;
	}

	desc = (struct vusbd_control_ep_out_desc *)ring->desc;
	desc += ring->prod;
	ASSERT(desc);

	vusbd->control_transfer_seq++;
	desc->type = dr->bmRequestType & UT_READ;
	desc->setup_len = 8;
	desc->data_len = dr->wLength;
	desc->seq = vusbd->control_transfer_seq;
	ASSERT((uint32)PKTDATA(vusbd->osh, p) < VUSBD_WL_MEM_MAP_LIMIT);
	desc->data = 0;
	desc->data_pkt = NULL;
	bcopy(PKTDATA(vusbd->osh, p), (void *)desc->setup_pkt, 8);
	desc->setup = (uint32)desc->setup_pkt;
	next_index = NEXT_INDEX(ring->prod, ring->ring_size);

	vusbd->ctrl_out_pending = !(dr->bmRequestType & UT_READ);
	if (vusbd->ctrl_out_pending) {
		/* don't post the descriptor until we get OUT */
		vusbd_trace("SETUP type %x len %d waiting for OUT\n",
			dr->bmRequestType, PKTLEN(vusbd->osh, p));
		return 0;
	}
	ring->prod = next_index;
	vusbd->vusbd_pub.wlan_stats.ctrl_ep_trans_enq_cnt++;
	vusbd_trace("send ctrl out to BT request type %x len %d\n",
		dr->bmRequestType, desc->data_len);
	vusbd_hex("setup:", (uint8 *)desc->setup, desc->setup_len);
	vusbd_hex("data:", (uint8 *)desc->data, desc->data_len);
	vusbd_set_WLAN_2_BT_intr(vusbd, VUSBD_INT_CONTROL);
	return 0;
}

void vusbd_process_control_transfer(struct vusbd *vusbd)
{
	struct vusbd_ep_desc_ring *ring;
	struct vusbd_control_ep_in_desc *desc;
	void  *pkt;
	struct vusbd_pub *pub;

	ASSERT(vusbd);
	pub = &vusbd->vusbd_pub;

	ring = &pub->control_ep_in;
	ASSERT(ring->direction == VUSBD_DIR_BT_2_WLAN);

	desc = (struct vusbd_control_ep_in_desc *)ring->desc;
	vusbd_trace("vusbd_process_control_transfer cons: 0x%x prod 0x%x\n",
		ring->cons, ring->prod);
	while (ring->cons != ring->prod) {
		desc += ring->cons;

		ASSERT(desc);
		if (desc->seq != vusbd->control_transfer_seq) {
			ring->cons = NEXT_INDEX(ring->cons, ring->ring_size);
			vusbd_err("control seq. out of sync %d %d\n",
				desc->seq, vusbd->control_transfer_seq);
			continue;
		}
		if (!(desc->type & UT_READ)) {
			ring->cons = NEXT_INDEX(ring->cons, ring->ring_size);
			/* for OUT  data phase */
			switch (desc->response) {
			case 0:
				/* send zero length packet */
				/* this is done alreay */
				break;
			case 1:
				/* STALL */
				vusbd_err("ctrl: send STALL\n");
				usbd_stall_ep(vusbd->bus, 0);
				break;
			case 2:
				/* NAK */
				vusbd_err("ctrl: send NAK\n");
				/* TODO */
				break;
			}
			vusbd->vusbd_pub.wlan_stats.ctrl_ep_trans_deq_cnt++;
			vusbd_trace("Contrl OUT: BT repsonse %d\n", desc->response);
		} else {
			/* DATA for IN phase */
			pkt = PKTGET(vusbd->osh, desc->data_len, TRUE);
			if (!pkt) {
				vusbd_err("fail to alloc packet %d bytes\n", desc->data_len);
				return;
			}
			/* make sure the packet buffer is not chained */
			ASSERT(PKTNEXT(vusbd->osh, pkt) == NULL);
			ASSERT(desc->data);
			bcopy((void *)BT_ADDR_TO_WLAN(desc->data), PKTDATA(vusbd->osh, pkt),
				desc->data_len);
			PKTSETLEN(vusbd->osh, pkt, desc->data_len);
			ring->cons = NEXT_INDEX(ring->cons, ring->ring_size);
			vusbd->vusbd_pub.wlan_stats.ctrl_ep_trans_deq_cnt++;
			vusbd_trace("ctrl IN get %d bytes from BT \n", desc->data_len);

			/* send out IN data */
			usbdev_bus_sendctl(vusbd->bus, pkt);

		}
	}
	if (ring->cons == ring->prod) {
		vusbd->vusbd_pub.WLAN_intstatus &=  (~(1 << VUSBD_INT_WLAN2BT_EVENT));
	}
	return;
}

void vusbd_set_WLAN_2_BT_intr(struct vusbd *vusbd, uint8 int_bit)
{
	volatile uint32 intr;
	volatile uint32 *preg = (void *)(VUSBD_WLAN_BT_INTR_PORT);

	ASSERT(int_bit <= VUSBD_INT_CONTROL);
	vusbd->vusbd_pub.WLAN_intstatus |=  (1 << int_bit);

	*preg |= (1 << VUSBD_WL_2_BT_INT_BIT);
	if ((*preg & (1 << VUSBD_WL_2_BT_INT_BIT)) == 0) {
		/*
		 * Due to a race we have to set our interrupt bit too. That
		 * causes a whole bunch of spurious interrupts but
		 * it is better than the alternative which is missed
		 * interrupts
		 */
		*preg |= (1 << VUSBD_WL_2_BT_INT_BIT) |
		        (1 << VUSBD_BT_2_WL_INT_BIT);

		/*
		 * Now check for the other type of race where the BT
		 * side wins  In that case we have to re-set the bit.
		 * Once should do it because of the amount of
		 * processing in between and the relative speeds of
		 * the processors.
		 */
		intr = *preg;
		if ((intr & (1 << VUSBD_WL_2_BT_INT_BIT)) == 0) {
			*preg = intr | (1 << VUSBD_WL_2_BT_INT_BIT) |
			        (1 << VUSBD_BT_2_WL_INT_BIT);
		}

	}

	vusbd->vusbd_pub.wlan_stats.int_set_cnt++;
}

void vusbd_clear_BT_2_WLAN_intr(struct vusbd *vusbd)
{
	volatile uint32 *preg = (void *)(VUSBD_BT_WLAN_INTR_PORT);

	*preg &= (~(1 << VUSBD_BT_2_WL_INT_BIT));
	vusbd->vusbd_pub.wlan_stats.int_clear_cnt++;
}

/*
 * return 1 if this pkt belongs to BT
 * return 0 if this pkt belongs to WLAN
 */
int vusbd_ctrl_dispatch(struct vusbd *vusbd, void *p)
{
	struct vusbd_ep_desc_ring *ring;
	struct vusbd_control_ep_out_desc *desc;
	uint8 next_index;
	struct vusbd_pub *pub;

	ASSERT(vusbd);
	ASSERT(p);
	pub = &vusbd->vusbd_pub;
	/* For bulk dl try */
	if (bt_dl_state == BT_DL_STATE_EXPECT_DATA) {
		return vusbd_dispatch_bt_dl(vusbd, p);
	}


	if (!vusbd->ctrl_out_pending) {
		vusbd_trace("ctrl out, get PKT for WLAN\n");
		return 0;
	}
	vusbd_trace("ctrl out, get PKT for BT\n");

	ring = &pub->control_ep_out;
	ASSERT(ring->direction == VUSBD_DIR_WLAN_2_BT);
	if (NEXT_INDEX(ring->prod, ring->ring_size) == ring->cons) {
		vusbd_err("ctrl out ring full\n");
		return 0;
	}

	desc = (struct vusbd_control_ep_out_desc*)ring->desc;
	desc += ring->prod;

	desc->data_len = PKTLEN(vusbd->osh, p);

	ASSERT(desc->seq == vusbd->control_transfer_seq);
	ASSERT((uint32)PKTDATA(vusbd->osh, p) < VUSBD_WL_MEM_MAP_LIMIT);
#ifdef STATIC_VUSBD_MEMORY
	memcpy(&(co_data_ptr[ring->prod]), (PKTDATA(vusbd->osh, p)), PKTLEN(vusbd->osh, p));
	desc->data = (uint32)(&(co_data_ptr[ring->prod]));
#else
	desc->data = (uint32)PKTDATA(vusbd->osh, p);
#endif
	desc->data_pkt = p;

	next_index = NEXT_INDEX(ring->prod, ring->ring_size);
	ring->prod = next_index;
	vusbd->ctrl_out_pending = 0;
	vusbd_trace("send ctrl out to BT request type %x len %d\n", desc->type, desc->data_len);
	vusbd_hex("setup:", (uint8 *)desc->setup, desc->setup_len);
	vusbd_hex("data:", (uint8 *)desc->data, desc->data_len);
	vusbd_set_WLAN_2_BT_intr(vusbd, VUSBD_INT_CONTROL);
	return 1;
}

void *vusbd_malloc(osl_t *osh, int size)
{
	void *p;
	p = MALLOC(osh, size);
	if (!p) {
		vusbd_err("fail to alloc %d\n", size);
		return p;
	}

	ASSERT((uint32)p < VUSBD_WL_MEM_MAP_LIMIT);
	if ((uint32)p < VUSBD_WL_MEM_MAP_LIMIT) {
		return p;
	}
	else {
		return NULL;
	}

}


int vusbd_get_hw_ep(struct vusbd *vusbd, int ep_index)
{
	ASSERT(ep_index >= 32);

	ep_index -= 32;
	return (vusbd->data_ep[ep_index].hw_ep);
}


void vusbd_send_setalt_event(struct vusbd *vusbd)
{

	struct vusbd_ep_desc_ring *ring;
	struct vusbd_pub *pub;
	uint8 next_index;

	ASSERT(vusbd);
	pub = &vusbd->vusbd_pub;
	ring = &pub->event_q_to_bt;

	pub->wlan_2_bt_events[ring->prod] = VUSBD_WLAN2BT_EVENT_ALTSET;
	next_index = NEXT_INDEX(ring->prod, ring->ring_size);
	ring->prod = next_index;

	vusbd_set_WLAN_2_BT_intr(vusbd, VUSBD_INT_WLAN2BT_EVENT);
	return;
}

void vusbd_process_setalt(struct vusbd *vusbd, uint32 intf, uint32 altset)
{
	vusbd_trace("vusbd_process_setal Intf %d AltSet %d", intf, altset);

	if ((intf == 1) && (altset == 2)) {
		vusbd_send_setalt_event(vusbd);
	}

	return;
}

#endif /* BCMUSBDEV_COMPOSITE && BCM_VUSBD */
