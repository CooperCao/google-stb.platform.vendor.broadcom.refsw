/*
 *  bdcep is the end point for BDC
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
#include <bdcep.h>

#ifdef USB_BYPASS_REENUM
/*
 * ===============================================================================================
 * ! ALL usb globals are placed in a separate region.  If no re-enumeration is required on
 * reboot,
 * ! all USB variables will retain their value.	This header must be applied to all USB files
 * ===============================================================================================
 */
#pragma arm section rwdata = "usb_area_rw" zidata = "usb_area_zi"
#endif

bdc_status_code bdcep_intr_transfer(bdc_ep *ep, bdcd *bdc, urb_t *urb);
bdc_status_code bdcep_transfer(urb_t *urb);
void bdcep_set_maximum_packet_size(bdc_ep *ep, bdcd *bdc);
void bdcep_clear_or_set_stall(usb_endpoint *ep, int is_set);

extern uint8 bdc_bd_ep[];
extern bdc_command_trb bdc_command_trbs[];

/*******************************************************************************
* Local Declarations
******************************************************************************
*/

/*******************************************************************************
* Function Definitions
******************************************************************************
*/
bdc_status_code
bdcep_bulk_transfer(bdc_ep *ep, bdcd *bdc, urb_t *urb, uint32 max_transfer_size)
{
	usb_device_t *device;
	bdc_bd_list *bd_list;
	bdc_bd *bd;
	bdc_bd *first_bd;
	uint8 *buf;
	uint32 field;
	uint32 length;
	uint32 packet_count;
	int bd_count;

	ep = (bdc_ep *)urb->ep;
	device = ((usb_endpoint *)ep)->device;
	bdc_dbg(("bdcep_bulk_transfer length=%x\n", urb->expected_length));

	if (max_transfer_size == 0) {
		ASSERT(TRUE);
		return BDC_STATUS_FAILED;
	}

	if (urb->expected_length) {
		bd_count = (urb->expected_length - 1) / max_transfer_size + 1;
	} else {
		bd_count = 1;
	}

	bd_list = &ep->transfer_bd_list;

	bd = bdcbdlist_allocate_multiple_bd(bd_list, bd_count);

	ASSERT(bd);
	if (bd == NULL) {
		bdc_dbg(("bdcep_bulk_transfer No TRB bd_count=%x\n", bd_count));
		return BDC_STATUS_PENDING;
	}

	((usb_endpoint *)ep)->pending_size = urb->expected_length;
	ep->bd_count = (uint8)bd_count;
	ASSERT(bd_count < 255);

	buf = urb->data_buffer;
	length = 0;
	first_bd = bd;
	field = 0;
	bdc_dbg(("	  bd_count=%x\n", bd_count));
	bdc_dbg(("	  bd index=%x\n", bd - ep->transfer_bd_list.first_bd));
	while (bd_count) {
		if (device->controller->attribute & USBCONTROLLER_ATTRIBUTE_FORCE_WORD_ALIGN) {
			/* For OUT data, schedule multiple bd to write into aligned addresses */
			if ((((usb_endpoint *)ep)->address & USB_DIR_DEVICE_TO_HOST) == 0) {
				while ((uint32)buf % 4 != 0) {
					buf++;
				}
			}
		}
		bd->generic.parameter_low = (uint32)buf;
		bd->generic.parameter_high = 0;
		bd->generic.status = BDCBD_XSF_INTR_TARGET(0);

		if (ENDPOINT_TYPE_ISOC == ((usb_endpoint *)ep)->type) {
			field = BDCBD_XSF_EOT | BDCBD_XSF_SOT | BDCBD_XSF_TO_TFS(1)
				| BDCBD_XSF_IOC | BDCBD_XSF_TYPE_TRANSFER;

			if ((((usb_endpoint *)ep)->address & USB_DIR_DEVICE_TO_HOST) == 0) {
				field |= BDCBD_XSF_ISP;
			}

			if (length == 0) {
				field |= BDCBD_XSF_SBF;
			}

			/* First BD */
			bd->generic.status |= BDCBD_XSF_TYPE_LTF;
		} else {
			field = BDCBD_XSF_ISP | BDCBD_XSF_TYPE_TRANSFER;
			if (length == 0) {
				/* First BD */
				bd->generic.status |= BDCBD_XSF_TYPE_LTF;
				field |= BDCBD_XSF_SOT | BDCBD_XSF_SBF;

				if (urb->expected_length) {
					packet_count =
						(urb->expected_length - 1) /
						ep->max_packet_size + 1;
				} else {
					packet_count = 1;
				}
				field |= BDCBD_XSF_TO_TFS(packet_count);
			}
		}

		bdc_dbg(("	  bd=%p\n", bd));
		buf += max_transfer_size;
		length += max_transfer_size;

		if (length > urb->expected_length) {
			length -= max_transfer_size;
			bd->generic.status |= urb->expected_length - length;
		} else {
			bd->generic.status |= max_transfer_size;
		}

		bd_count--;
		if (bd_count == 0) {
			break;
		}

		bdc_dbg(("bdcep_bulk_transfer bd=%p\n", bd));
		if (ENDPOINT_TYPE_ISOC != ((usb_endpoint *)ep)->type) {
			field |= BDCBD_XSF_TYPE_CHAIN;
		}

		bd->generic.control = field;
		/* osapi_LOCK_CONTEXT */
		bd = bdcbdlist_get_next(bd_list, bd);
		/* osapi_UNLOCK_CONTEXT */
	}

	field |= BDCBD_XSF_IOC | BDCBD_XSF_EOT;
	bdc_dbg(("bdcep_bulk_transfer control=%x\n", field));
	bdc_dbg((" status=%x\n", bd->generic.status));
	bd->generic.control = field;
	first_bd->generic.control &= ~BDCBD_XSF_SBF;

	W_REG(bdc->osh, (uint32 *)(bdc->bdc_base_reg + UBDC_XSFNTF_ADR), ep->index);
	bdc_dbg((" index=%x\n", ep->index));

	return BDC_STATUS_SUCCESS;
}

bdc_status_code
bdcep_intr_transfer(bdc_ep *ep, bdcd *bdc, urb_t *urb)
{
	/* Currently interrupt transfer should be the same as bulk transfer */
	bdc_dbg(("bdcep_intr_transfer length=%x\n", urb->expected_length));
	bdc_dbg(("buffer=%p\n", urb->data_buffer));
	return bdcep_bulk_transfer(ep, bdc, urb, BDCBD_MAX_BD_TRANSFER_SIZE);
}

/* In the dongle->host path, urb has data ,
 * In host->dongle path, urb is just memory to receive data later on
 */
bdc_status_code
bdcep_transfer(urb_t *urb)
{
	bdcd *bdc;
	bdc_ep *ep;

	bdc = (bdcd *)urb->ep->device->controller;
	ep = (bdc_ep *)urb->ep;

	bdc_dbg(("bdcep_transfer: ep=%x\n", ((usb_endpoint *)ep)->address));
	bdc_dbg(("bdcep_transfer: length=%x\n", urb->expected_length));

	if (((usb_endpoint *)ep)->address & USB_DIR_DEVICE_TO_HOST) {
		if ((((usb_endpoint *)ep)->address & 0xF) > bdc->max_in_ep) {
			ASSERT(0);
			return BDC_STATUS_FAILED;
		}
	} else {
		if (((usb_endpoint *)ep)->address > bdc->max_out_ep) {
			ASSERT(0);
			return BDC_STATUS_FAILED;
		}
	}

	if (bdc->condition & BDC_CONDITION_U2) {
		bdc->condition &= ~BDC_CONDITION_U2;

		((usb_controller *)bdc)->ft->remote_wake((usb_controller *)bdc);
	}

	if (ENDPOINT_TYPE_BULK == ((usb_endpoint *)ep)->type) {
		return bdcep_bulk_transfer(ep, bdc, urb, BDCBD_MAX_BD_TRANSFER_SIZE);
	} else if (ENDPOINT_TYPE_INTR == ((usb_endpoint *)ep)->type)    {
		return bdcep_intr_transfer(ep, bdc, urb);
	}

	ASSERT(0);
	return BDC_STATUS_FAILED;
}

void
bdcep_transfer_event(bdc_ep *ep, bdcd *bdc, bdc_bd *evt)
{
	bdc_bd_list *bd_list;
	urb_t *urb;
	uint32 length;
	uint32 complete_code;
	bdc_status_code status;
	usb_controller *controller = (usb_controller *)bdc;

	bd_list = &ep->transfer_bd_list;

	complete_code = evt->transfer_event.flags & BDCBD_XSF_COMP_CODE;

	bdc_dbg(("bdcep_transfer_event:cc=%x\n", complete_code));

	status = BDC_STATUS_SUCCESS;

	switch (complete_code) {
	case BDCBD_XSF_COMP_SUCCESS:
	case BDCBD_XSF_COMP_SHORT:
		break;
	case BDCBD_XSF_COMP_STOPPED:
		return;
	default:
		status = BDC_STATUS_FAILED;
	}

	ASSERT(evt->transfer_event.buffer_low);

	/* bdc_dbg(("	control=%x\n", bd->generic.control)); */
	bdc_dbg(("	ep=%p\n", ep));
	/* bdc_dbg(("	bd index=%x\n", bd - ep->transfer_bd_list.first_bd)); */

	length = evt->transfer_event.length & BDCBD_EVENT_BD_LENGTH;
	ASSERT(((usb_endpoint *)ep)->pending_size >= length);
	((usb_endpoint *)ep)->pending_size -= length;
	bdc_dbg(("bdcep_transfer_event length=%x\n", ((usb_endpoint *)ep)->pending_size));

	/* osapi_LOCK_CONTEXT */
	urb = ((usb_endpoint *)ep)->urb_list;
	if (urb) {
		urb->finished_length = ((usb_endpoint *)ep)->pending_size;
	}

	bdcbdlist_move_dequeue(bd_list);
	/* osapi_UNLOCK_CONTEXT */

	if (urb && urb->transfer_callback) {
		/* Looks like this code is only for TX data, dongle->host */
		ASSERT(ep->bd_count);
		ep->bd_count--;
		bdc_dbg(("bdcep_transfer_event bd_count=%x\n", ep->bd_count));
		bdc_dbg(("finished_length=%x\n", urb->finished_length));
		if (ep->bd_count == 0) {
			((usb_endpoint *)ep)->transfer_start(urb);
		}
	} else {
		/* host->dongle */
		bdc_dbg(("bdcep_transfer_event length=%x\n", urb->finished_length));
		controller->ft->transfer_complete(controller, (usb_endpoint *)ep, status);
	}
}

void
bdcep_set_maximum_packet_size(bdc_ep *ep, bdcd *bdc)
{
	switch (bdc->speed) {
	case USBDEVICE_SPEED_FULL:
		if (ENDPOINT_TYPE_INTR == ((usb_endpoint *)ep)->type) {
			if (BDCEP_INTERRUPT_IN == ((usb_endpoint *)ep)->address) {
				((usb_endpoint *)ep)->max_packet_size = 4;	/* 16 bytes */
				ep->max_packet_size = 16;
			} else {
				((usb_endpoint *)ep)->max_packet_size = 3;	/* 8 bytes */
				ep->max_packet_size = 8;
			}
		} else {
			if (BDCEP_DEBUG_BULK_IN == ((usb_endpoint *)ep)->address ||
			    BDCEP_DEBUG_BULK_OUT == ((usb_endpoint *)ep)->address) {
				((usb_endpoint *)ep)->max_packet_size = 5;	/* 64 bytes */
				ep->max_packet_size = 32;
			} else {
				((usb_endpoint *)ep)->max_packet_size = 6;	/* 64 bytes */
				ep->max_packet_size = 64;
			}
		}
		break;
	case USBDEVICE_SPEED_LOW:
		((usb_endpoint *)ep)->max_packet_size = 3;		/* 8 bytes */
		ep->max_packet_size = 8;
		break;
	case USBDEVICE_SPEED_HIGH:
		((usb_endpoint *)ep)->max_packet_size = 9;		/* 512 bytes */
		ep->max_packet_size = 512;
		break;
	case USBDEVICE_SPEED_SUPER:
		((usb_endpoint *)ep)->max_packet_size = 10;	/* 1024 bytes */
		ep->max_packet_size = 1024;
		break;
	default:
		break;
	}
}

void
bdcep_config(bdc_ep *ep, bdcd *bdc)
{
	bdc_bd *bd;
	bdc_command cmd;
	int index;

	ep->index = ((((usb_endpoint *)ep)->address & 0x0F) << 1);
	if (((usb_endpoint *)ep)->address & USB_DIR_DEVICE_TO_HOST) {
		ep->index++;
	}

	bdc_dbg(("bdcep_init: type=%x add= %x, index= %x\n", ((usb_endpoint *)ep)->type,
		((usb_endpoint *)ep)->address, ep->index));

	ep->status |= BDCEP_STATUS_ENABLED;

	index = (ep->index >> 1) + (ep->index & 0x01) * BDC_EP_OUT_MAX - 1;
	bd = (bdc_bd *)&bdc_bd_ep[index * BDC_BD_PER_EP_MAX * sizeof(bdc_bd)];
	memset(bd, 0, BDC_BD_PER_EP_MAX * sizeof(bdc_bd));
	bdcbdlist_init(&ep->transfer_bd_list, bd, BDC_BD_PER_EP_MAX, BDCBDLIST_FLAG_LINKED,
			BDCBDLIST_TYPE_BD);

	if (((usb_endpoint *)ep)->address & USB_DIR_DEVICE_TO_HOST) {
		if ((((usb_endpoint *)ep)->address & 0xF) > bdc->max_in_ep) {
			ASSERT(0);
			return;
		}
	} else {
		if (((usb_endpoint *)ep)->address > bdc->max_out_ep) {
			ASSERT(0);
			return;
		}
	}

	cmd.param0 = (uint32)ep->transfer_bd_list.first_bd;
	cmd.param1 = 0;

	if (ENDPOINT_TYPE_ISOC == ((usb_endpoint *)ep)->type) {
		cmd.param2 = BDCREG_CMD_EP_TYPE_ISOC | BDCREG_CMD_EP_INTERVAL(0);
	} else if (ENDPOINT_TYPE_BULK == ((usb_endpoint *)ep)->type)    {
		cmd.param2 = BDCREG_CMD_EP_TYPE_BULK;
	} else if (ENDPOINT_TYPE_INTR == ((usb_endpoint *)ep)->type)    {
		cmd.param2 = BDCREG_CMD_EP_TYPE_INTR | BDCREG_CMD_EP_INTERVAL(0);
	} else {
		ASSERT(0);
		return;
	}

	cmd.param2 |= BDCREG_CMD_MAX_PACKET(ep->max_packet_size);
	cmd.param2 |= BDCREG_CMD_MAX_BURST(0) | BDCREG_CMD_EP_MULT(0);

	cmd.cmd = BDCREG_CMD_EPC | BDCREG_SUB_CMD_EP_ADD | BDCREG_CMD_EPN(ep->index);

	bdc_dbg(("bdcep_init: cmd=%x\n", cmd.cmd));
	bdc_dbg(("bdcep_init: param0=%x\n", cmd.param0));
	bdc_dbg(("bdcep_init: param1=%x\n", cmd.param1));
	bdc_dbg(("bdcep_init: param2=%x\n", cmd.param2));
	bdc_issue_command(bdc, &cmd);
}

void
bdcep_un_config(bdc_ep *ep, bdcd *bdc)
{
	bdc_command cmd = {
		0, 0, 0, 0
	};

	bdc_dbg(("bdcep_un_config: ep=%x\n", ep->base.address));
	ep->status = 0;

	cmd.cmd = BDCREG_CMD_EPO |
		BDCREG_SUB_CMD_EP_STOP |
		BDCREG_CMD_EPN(ep->index) |
		BDCREG_CMD_CSA_XSD;

	bdc_issue_command(bdc, &cmd);
	/* bdc_dbg(("bdcep_un_config: bd=%x\n", bd)); */

	cmd.cmd = BDCREG_CMD_EPC | BDCREG_SUB_CMD_EP_DROP | BDCREG_CMD_EPN(ep->index);
	bdc_issue_command(bdc, &cmd);
	/* bdc_dbg(("bdcep_un_config: bd=%x\n", bd)); */
}

void
bdcep_clear_or_set_stall(usb_endpoint *ep, int is_set)
{
	bdcd *bdc;
	bdc_command cmd = {
		0, 0, 0, 0
	};

	bdc = (bdcd *)ep->device->controller;
	cmd.cmd = BDCREG_CMD_EPN(((bdc_ep *)ep)->index);
	if (is_set) {
		cmd.cmd |= BDCREG_CMD_EPO | BDCREG_SUB_CMD_EP_STALL;
	} else {
		cmd.cmd |= BDCREG_CMD_EPO | BDCREG_SUB_CMD_EP_RESET | BDCREG_CMD_CSA_RESET_TOGGLE;
	}

	bdc_issue_command(bdc, &cmd);
}

void
bdcep_init(bdc_ep *ep, usb_controller *controller)
{
	bdcd *bdc;

	usbendpoint_init((usb_endpoint *)ep, controller);

	((usb_endpoint *)ep)->transfer_start = bdcep_transfer;
	((usb_endpoint *)ep)->clear_or_set_stall = bdcep_clear_or_set_stall;
	ep->transfer_event = bdcep_transfer_event;

	bdc = (bdcd *)controller;
	switch (((usb_endpoint *)ep)->address) {
	case BDCEP_INTERRUPT_IN:
		((usb_endpoint *)ep)->type = ENDPOINT_TYPE_INTR;
		break;

	case BDCEP_BULK_IN:
	case BDCEP_BULK_OUT:
		((usb_endpoint *)ep)->type = ENDPOINT_TYPE_BULK;
		break;

	case BDCEP_DEBUG_BULK_IN:
	case BDCEP_DEBUG_BULK_OUT:
		((usb_endpoint *)ep)->type = ENDPOINT_TYPE_BULK;
		break;

	case BDCEP_KEYBOARD_IN:
		((usb_endpoint *)ep)->type = ENDPOINT_TYPE_INTR;
		break;

	case BDCEP_MOUSE_IN:
		((usb_endpoint *)ep)->type = ENDPOINT_TYPE_INTR;
		break;

	default:
		ASSERT(0);
	}

	bdcep_set_maximum_packet_size(ep, bdc);
	bdcep_config(ep, bdc);
}
