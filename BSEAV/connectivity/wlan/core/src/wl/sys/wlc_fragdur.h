/**
 * @file
 * @brief
 * Maximum TX duration enforcement via fragmentation.
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

#ifndef _wlc_fragdur_h_
#define _wlc_fragdur_h_

#include <wlc_types.h>

extern wlc_fragdur_info_t *wlc_fragdur_attach(wlc_info_t *wlc);
extern void wlc_fragdur_detach(wlc_fragdur_info_t *fragdur);
extern uint wlc_fragdur_threshold(wlc_fragdur_info_t *fragdur, struct scb *scb, uint8 ac);
extern int wlc_fragdur_length(wlc_fragdur_info_t *fragdur);

#endif /* _wlc_fragdur_h_ */
