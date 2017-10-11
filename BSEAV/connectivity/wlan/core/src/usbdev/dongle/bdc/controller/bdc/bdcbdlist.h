/*
 *  Defines Transfer Request Block.
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

#ifndef _BDCBDLIST_H_
#define _BDCBDLIST_H_

#include <typedefs.h>
#include <bdcbd.h>

#define BDCBDLIST_FLAG_LINKED 0x00000001

#define BDCBDLIST_TYPE_BD	1
#define BDCBDLIST_TYPE_SR	2

typedef struct _bdc_bd_list {
	bdc_bd *first_bd;
	uint16 enqueue;
	uint16 dequeue;
	uint16 max_count;
	uint16 flags;
} bdc_bd_list;

void bdcbdlist_move_dequeue(bdc_bd_list *bd_list);
void bdcbdlist_init(bdc_bd_list *bd_list, bdc_bd *first_bd, uint16 max_count, uint8 flags,
		uint32 list_type);
bdc_bd * bdcbdlist_allocate_multiple_bd(bdc_bd_list *bd_list, int bd_count);
bdc_bd * bdcbdlist_get_next(bdc_bd_list *bd_list, bdc_bd *bd);

#endif /* _BDCBDLIST_H_ */
