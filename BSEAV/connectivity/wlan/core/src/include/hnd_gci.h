/*
 * HND GCI control software interface
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: $
 *
 * <<Broadcom-WL-IPTag/Proprietary:>>
 */

#ifndef _hnd_gci_h_
#define _hnd_gci_h_

typedef struct gci_mb_handle gci_mb_handle_t;

typedef void (*gci_mb_handler_t)(uint32 event, uint32 input, void *arg);

extern int hnd_gci_init(si_t *sih);

#ifdef WLGCIMBHLR
extern gci_mb_handle_t * hnd_gci_mb_handler_register(uint regidx, uint32 mask,
	gci_mb_handler_t cb, void *arg);
extern void hnd_gci_mb_handler_unregister(gci_mb_handle_t *gcimb);
extern void hnd_gci_mb_handler_process(uint32 stat, si_t *sih);
#else
#define hnd_gci_mb_handler_register(regidx, mask, cb, arg) \
	((gci_mb_handle_t *)(((int)cb ^ (int)cb)))
#define hnd_gci_mb_handler_unregister(gch) do {} while (0)
#define hnd_gci_mb_handler_process(stat, sih) do {} while (0)
#endif /* WLGCIMBHLR */
#endif /* _hnd_gci_h_ */
