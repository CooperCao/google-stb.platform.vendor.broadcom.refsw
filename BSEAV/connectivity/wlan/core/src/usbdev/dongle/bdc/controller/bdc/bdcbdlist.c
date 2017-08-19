/*
 *  BDC Transfer Request Block.
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
#include <bcmdevs.h>
#include <bcmendian.h>
#include <bcmutils.h>
#include <bcmstdlib.h>
#include <usbdevice.h>
#include <bdc_usb.h>
#include <usbcontroller.h>
#include <bdc.h>
#include <bdcbdlist.h>

#ifdef USB_BYPASS_REENUM
/*
 * ===============================================================================================
 * ! ALL usb globals are placed in a separate region.  If no re-enumeration is required on
 * reboot,
 * ! all USB variables will retain their value.  This header must be applied to all USB files
 * ===============================================================================================
 */
#pragma arm section rwdata = "usb_area_rw" zidata = "usb_area_zi"
#endif

/*******************************************************************************
* Local Declarations
******************************************************************************
*/

/*******************************************************************************
* Function Definitions
******************************************************************************
*/

bdc_bd *
bdcbdlist_get_next(bdc_bd_list *bd_list, bdc_bd *bd)
{
	bdc_bd *next_bd;
	uint16 index;

	if ((bd_list->flags & BDCBDLIST_FLAG_LINKED) == 0) {
		index = bd - bd_list->first_bd;
		if (index == (bd_list->max_count - 1)) {
			return bd_list->first_bd;
		} else {
			return bd + 1;
		}
	}

	next_bd = bd + 1;
	while (TRUE) {
		if (BDCBD_XSF_TYPE_CHAIN == (next_bd->link.control & BDCBD_XSF_TYPE)) {
			next_bd = (bdc_bd *)next_bd->link.segment_low;
			continue;
		} else {
			break;
		}
	}

	return next_bd;
}

bdc_bd *
bdcbdlist_allocate_multiple_bd(bdc_bd_list *bd_list, int bd_count)
{
	bdc_bd *first_bd;
	bdc_bd *next_bd;
	int count;
	uint16 index;

	/* First make sure we have enough free TRB */
	first_bd = &bd_list->first_bd[bd_list->enqueue];
	count = bd_count;
	next_bd = first_bd;
	while (count) {
		next_bd = bdcbdlist_get_next(bd_list, next_bd);
		bdc_dbg(("bdcbdlist_allocate_multiple_bd:next_bd=%p\n", next_bd));
		index = next_bd - bd_list->first_bd;
		if (index == bd_list->dequeue) {
			ASSERT(0);
			return NULL;
		}

		count--;
	}

	bd_list->enqueue = next_bd - bd_list->first_bd;
	next_bd->generic.control = BDCBD_XSF_SBF;

	return first_bd;
}

void
bdcbdlist_move_dequeue(bdc_bd_list *bd_list)
{
	bdc_bd *bd;

	bd_list->dequeue++;
	if ((bd_list->flags & BDCBDLIST_FLAG_LINKED) == 0) {
		if (bd_list->dequeue == bd_list->max_count) {
			bd_list->dequeue = 0;
		}

		return;
	}

	bd = &bd_list->first_bd[bd_list->dequeue];
	while (TRUE) {
		if (BDCBD_XSF_TYPE_CHAIN == (bd->link.control & BDCBD_XSF_TYPE)) {
			bd = (bdc_bd *)bd->link.segment_low;
			continue;
		} else {
			break;
		}
	}

	bd_list->dequeue = bd - bd_list->first_bd;
}

void
bdcbdlist_init(bdc_bd_list *bd_list, bdc_bd *first_bd, uint16 max_count,
	uint8 flags, uint32 list_type)
{
	bdc_bd *bd;

	bd_list->first_bd = first_bd;
	bd_list->enqueue = 0;
	bd_list->dequeue = 0;
	bd_list->max_count = max_count;

	if (list_type == BDCBDLIST_TYPE_BD) {
		first_bd->generic.control = BDCBD_XSF_SBF;
	}

	bd_list->flags = 0;
	if (flags & BDCBDLIST_FLAG_LINKED) {
		bd_list->flags = BDCBDLIST_FLAG_LINKED;

		bd = first_bd + max_count - 1;
		bd->link.control = BDCBD_XSF_TYPE_CHAIN;
		bd->link.segment_low = (uint32)first_bd;
		bd->link.segment_high = 0;
	}
}
