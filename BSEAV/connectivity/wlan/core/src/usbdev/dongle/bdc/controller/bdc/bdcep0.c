/*
 *  Defines end point 0 for BDC
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

#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmdevs.h>
#include <bcmendian.h>
#include <bcmutils.h>
#include <bcmstdlib.h>
#include <usbdevice.h>
#include <bdc_usb.h>
#include <usbcontroller.h>
#include <bdc.h>
#include <bdcep0.h>

bdc_status_code bdcep0_transfer_start(urb_t *urb);
void bdcep0_clear_or_set_stall(usb_endpoint *ep0, int is_set);
void bdcep0_transfer_event(bdc_ep0 *ep0, bdcd *bdc, bdc_bd *evt);

extern uint8 bdc_bd_ep0[BDC_BD_PER_EP_MAX * sizeof(bdc_bd)];

bdc_status_code
bdcep0_transfer_start(urb_t *urb)
{
	bdc_ep0 *ep0;
	bdc_bd_list *bd_list;
	bdc_bd *bd;
	uint32 field;
	uint32 packet_count;
	bdcd *bdc;

	bdc_fn(("%s\n", __FUNCTION__));

	ep0 = (bdc_ep0 *)urb->ep;

	bdc = (bdcd *)urb->ep->device->controller;

	if ((ep0->status & BDCEP0_STATUS_DATA_STAGE) == 0) {
		bdc_dbg(("bdcep0_transfer_start BDCEP0_DATA_STAGE pending\n"));
		return BDC_STATUS_PENDING;
	}

	if ((urb->attribute & URB_ATTRIBUTE_NOT_LAST) == 0) {
		ep0->status &= ~BDCEP0_STATUS_DATA_STAGE;
	}

	bd_list = &ep0->transfer_bd_list;
	bdc_dbg(("bdcep0_transfer_start enqueue=%x\n", bd_list->enqueue));
	bd = bdcbdlist_allocate_multiple_bd(bd_list, 1);
	ASSERT(bd);
	if (bd == NULL) {
		bdc_dbg(("bdcep0_transfer_start no bd, dequeue=%x\n", bd_list->dequeue));
		return BDC_STATUS_PENDING;
	}

	((usb_endpoint *)ep0)->pending_size = urb->expected_length;

	packet_count = 1;
	if (urb->expected_length) {
		packet_count = (urb->expected_length - 1) /
			(1 << ((usb_endpoint *)ep0)->max_packet_size);
		packet_count++;
	}

	field = BDCBD_XSF_TO_TFS(packet_count);
	field |= BDCBD_XSF_TYPE_DATA_STAGE;
	if (((usb_endpoint0 *)ep0)->setup[0] & 0x80) {
		field |= BDCBD_XSF_DIR_IN;
	}

	field |= BDCBD_XSF_IOC | BDCBD_XSF_ISP;
	if (ep0->status & BDCEP0_STATUS_EXPECTING_DATA)	{
		field |= BDCBD_XSF_SOT;
		ep0->status &= ~BDCEP0_STATUS_EXPECTING_DATA;
	}

	bd->generic.parameter_low = (uint32)urb->data_buffer;
	bd->generic.parameter_high = 0;
	bd->generic.status = urb->expected_length | BDCBD_XSF_INTR_TARGET(0) | BDCBD_XSF_TYPE_LTF;
	if ((urb->attribute & URB_ATTRIBUTE_NOT_LAST) == 0) {
		field |= BDCBD_XSF_EOT;
		bd->generic.status |= BDCBD_XSF_TYPE_LTF;
	}

	bd->generic.control = field;

	bdc_dbg(("bdcep0_transfer_start status=%x\n", bd->generic.status));
	bdc_dbg((" control=%x\n", bd->generic.control));

	W_REG(bdc->osh, (uint32 *)(bdc->bdc_base_reg + UBDC_XSFNTF_ADR), 1);

	return BDC_STATUS_SUCCESS;
}

void
bdcep0_status_start(bdc_ep0 *ep0, bdcd *bdc)
{
	bdc_bd_list *bd_list;
	bdc_bd *bd;
	uint32 field;

	bdc_dbg(("bdcep0_status_start: status=%x\n", ep0->status));
	if ((ep0->status & BDCEP0_STATUS_STATUS_STAGE) == 0 ||
		(ep0->status & BDCEP0_STATUS_COMPLETED) == 0) {
		return;
	}

	field = BDCBD_XSF_TYPE_STATUS_STAGE | BDCBD_XSF_TO_TFS(1);
	if ((((usb_endpoint0 *)ep0)->setup[0] & 0x80) == 0) {
		field |= BDCBD_XSF_DIR_IN;
	}

	field |= BDCBD_XSF_IOC | BDCBD_XSF_ISP | BDCBD_XSF_EOT | BDCBD_XSF_SOT;

	ep0->status &= ~(BDCEP0_STATUS_STATUS_STAGE | BDCEP0_STATUS_COMPLETED);

	bd_list = &ep0->transfer_bd_list;
	bd = bdcbdlist_allocate_multiple_bd(bd_list, 1);
	bdc_dbg(("bdcep0_status_start bd=%p\n", bd));
	ASSERT(bd);

	bd->generic.parameter_low = 0;
	bd->generic.parameter_high = 0;
	bd->generic.status = BDCBD_XSF_INTR_TARGET(0) | BDCBD_XSF_TYPE_LTF;

	bd->generic.control = field;

	W_REG(bdc->osh, (uint32 *)(bdc->bdc_base_reg + UBDC_XSFNTF_ADR), 1);

	bdc_dbg(("bdcep0_status_start status=%x\n", bd->generic.status));
	bdc_dbg((" control=%x\n", bd->generic.control));
	bdc_dbg((" SRRBAL=%x\n",
		R_REG(bdc->osh, (uint32 *)(bdc->bdc_base_reg + UBDC_SRRBAL0_ADR))));
}

void
bdcep0_setup_complete(bdc_ep0 *ep0, bdcd *bdc, bdc_status_code status)
{
	bdc_fn(("%s\n", __FUNCTION__));

	if (BDC_STATUS_SUCCESS == status) {
		ep0->status |= BDCEP0_STATUS_COMPLETED;
	} else {
		ep0->status |= BDCEP0_STATUS_STALL;
	}

	bdcep0_status_start(ep0, bdc);
}

void
bdcep0_clear_or_set_stall(usb_endpoint *ep0, int is_set)
{
	bdc_fn(("%s\n", __FUNCTION__));

	if (is_set) {
		((bdc_ep0 *)ep0)->status |= BDCEP0_STATUS_STALL;
		bdcep0_status_start((bdc_ep0 *)ep0, (bdcd *)ep0->device->controller);
	}
}

void
bdcep0_set_speed(bdc_ep0 *ep0, uint32 speed)
{
	bdc_fn(("%s\n", __FUNCTION__));

	switch (speed) {
	case USBDEVICE_SPEED_FULL:
		((usb_endpoint *)ep0)->max_packet_size = 6;
		break;
	case USBDEVICE_SPEED_LOW:
		((usb_endpoint *)ep0)->max_packet_size = 3;
		break;
	case USBDEVICE_SPEED_HIGH:
		((usb_endpoint *)ep0)->max_packet_size = 6;
		break;
	case USBDEVICE_SPEED_SUPER:
		((usb_endpoint *)ep0)->max_packet_size = 9;
		break;
	default:
		break;
	}
}

void
bdcep0_transfer_event(bdc_ep0 *ep0, bdcd *bdc, bdc_bd *evt)
{
	bdc_bd_list *bd_list;
	bdc_bd *bd;
	uint32 complete_code;
	urb_t *urb;
	uint32 length;
	uint32 command;
	bdc_command cmd = {
		0, 0, 0, 0
	};
	bdc_status_code status;
	usb_controller *controller = (usb_controller *)bdc;

	bd_list = &ep0->transfer_bd_list;

	complete_code = evt->transfer_event.flags & BDCBD_XSF_COMP_CODE;

	bdc_dbg(("bdcep0_transfer_event:cc=%x\n", complete_code));
	if (BDCBD_XSF_COMP_SETUP_RECEIVED == complete_code) {
		bdc_setup_received(bdc, evt);
		return;
	} else if (BDCBD_XSF_COMP_DATA_START == complete_code)	{
		if (bdc->ep0.status & BDCEP0_STATUS_PENDING_RESET) {
			bdc->ep0.status &= ~BDCEP0_STATUS_PENDING_RESET;

			cmd.cmd = BDCREG_CMD_EPO | BDCREG_SUB_CMD_EP_RESET |
				BDCREG_CMD_CSA_PRESERVE_TOGGLE;
			bdc_issue_command(bdc, &cmd);
		}

		ep0->status |= BDCEP0_STATUS_DATA_STAGE | BDCEP0_STATUS_EXPECTING_DATA;

		if (ep0->status & BDCEP0_STATUS_STALL) {
			cmd.cmd = BDCREG_CMD_EPO | BDCREG_SUB_CMD_EP_STALL | BDCREG_CMD_EPN(1);
			bdc_issue_command(bdc, &cmd);
			bdc_dbg(("bdcep0_transfer_event:BDCEP0_STATUS_STALL\n"));
		} else {
			if (((usb_endpoint *)ep0)->urb_list) {
				((usb_endpoint *)ep0)->transfer_start(
						((usb_endpoint *)ep0)->urb_list);
			}
		}

		return;
	} else if (BDCBD_XSF_COMP_STATUS_START == complete_code) {
		if (bdc->ep0.status & BDCEP0_STATUS_PENDING_RESET) {
			bdc->ep0.status &= ~BDCEP0_STATUS_PENDING_RESET;

			cmd.cmd = BDCREG_CMD_EPO | BDCREG_SUB_CMD_EP_RESET |
				BDCREG_CMD_CSA_PRESERVE_TOGGLE;
			bdc_issue_command(bdc, &cmd);
		}

		ep0->status |= BDCEP0_STATUS_STATUS_STAGE;

		if (ep0->status & BDCEP0_STATUS_SET_ADDRESS) {
			bdc_set_address(bdc, bdc->address);
		} else {
			if (ep0->status & BDCEP0_STATUS_STALL) {
				cmd.cmd = BDCREG_CMD_EPO | BDCREG_SUB_CMD_EP_STALL | BDCREG_CMD_EPN(
						1);
				bdc_issue_command(bdc, &cmd);
			} else {
				bdcep0_status_start(ep0, bdc);
			}
		}

		return;
	}

	status = BDC_STATUS_SUCCESS;
	if (BDCBD_XSF_COMP_BABBLE == complete_code) {
		/* ASSERT(0, evt->transfer_event.flags); */
		bdc_err(("BDCBD_XSF_COMP_BABBLE\n"));
		return;
	}

	if (BDCBD_XSF_COMP_SUCCESS != complete_code && BDCBD_XSF_COMP_SHORT != complete_code) {
		return;
	}

	bdc_dbg(("bdcep0_transfer_event: evt.flags=%x\n", evt->transfer_event.flags));
	bdc_dbg(("bdcep0_transfer_event: evt.buf=%x\n", evt->transfer_event.buffer_low));
	bd = (bdc_bd *)evt->transfer_event.buffer_low;
	ASSERT(bd);

	command = bd->generic.control & BDCBD_EVENT_TYPE;
	if (BDCBD_XSF_TYPE_DATA_STAGE == command) {
		length = evt->transfer_event.length & BDCBD_EVENT_BD_LENGTH;
		ASSERT(((usb_endpoint *)ep0)->pending_size >= length);
		((usb_endpoint *)ep0)->pending_size -= length;

		urb = ((usb_endpoint *)ep0)->urb_list;
		if (urb) {
			urb->finished_length = ((usb_endpoint *)ep0)->pending_size;
		}
	}

	bdcbdlist_move_dequeue(bd_list);

	if (BDCBD_XSF_TYPE_DATA_STAGE == command) {
		bdc_dbg(("bdcep0_transfer_event length=%x\n", urb->finished_length));
		controller->ft->transfer_complete(controller, (usb_endpoint *)ep0, status);
	}
}

void
bdcep0_config(bdc_ep0 *ep0, bdcd *bdc)
{
	bdc_command cmd;

	bdc_fn(("%s\n", __FUNCTION__));

	ep0->status = 0;

	memset(bdc_bd_ep0, 0, sizeof(bdc_bd_ep0));
	bdcbdlist_init(&ep0->transfer_bd_list, (bdc_bd *)bdc_bd_ep0, BDC_BD_PER_EP_MAX,
			BDCBDLIST_FLAG_LINKED,
			BDCBDLIST_TYPE_BD);

	cmd.param0 = (uint32)ep0->transfer_bd_list.first_bd;
	cmd.param1 = 0;
	cmd.param2 = BDCREG_CMD_EP_TYPE_CTRL |
			BDCREG_CMD_MAX_PACKET(1 << ((usb_endpoint *)ep0)->max_packet_size) |
			BDCREG_CMD_MAX_BURST(0) | BDCREG_CMD_EP_MULT(0);

	cmd.cmd = BDCREG_CMD_EPC | BDCREG_SUB_CMD_EP_ADD | BDCREG_CMD_EPN(1);

	bdc_issue_command(bdc, &cmd);
}

void
bdcep0_un_config(bdc_ep0 *ep, bdcd *bdc)
{
	bdc_command cmd = {
		0, 0, 0, 0
	};

	bdc_fn(("%s\n", __FUNCTION__));

	cmd.cmd = BDCREG_CMD_EPO | BDCREG_SUB_CMD_EP_STOP | BDCREG_CMD_EPN(1);
	bdc_issue_command(bdc, &cmd);

	memset(bdc_bd_ep0, 0, sizeof(bdc_bd_ep0));
	bdcbdlist_init(&ep->transfer_bd_list, (bdc_bd *)bdc_bd_ep0, BDC_BD_PER_EP_MAX,
			BDCBDLIST_FLAG_LINKED,
			BDCBDLIST_TYPE_BD);
	bdc_dbg(("bdcep0_un_config dequeue=%x\n", ep->transfer_bd_list.dequeue));

	ep->status = 0;
}

void
bdcep0_init(bdc_ep0 *ep0, usb_controller *controller)
{
	bdc_fn(("%s\n", __FUNCTION__));

	usbendpoint0_init((usb_endpoint0 *)ep0, controller);

	((usb_endpoint *)ep0)->transfer_start = bdcep0_transfer_start;
	((usb_endpoint *)ep0)->clear_or_set_stall = bdcep0_clear_or_set_stall;
	ep0->transfer_event = bdcep0_transfer_event;

	memset(bdc_bd_ep0, 0, sizeof(bdc_bd_ep0));
	bdcbdlist_init(&ep0->transfer_bd_list, (bdc_bd *)bdc_bd_ep0, BDC_BD_PER_EP_MAX,
			BDCBDLIST_FLAG_LINKED,
			BDCBDLIST_TYPE_BD);
}
