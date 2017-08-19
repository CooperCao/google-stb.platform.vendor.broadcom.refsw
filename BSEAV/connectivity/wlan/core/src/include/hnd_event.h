/*
 * Event support for hnd.
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id:$
 */
#ifndef	_HNDRTE_EVENT_H
#define	_HNDRTE_EVENT_H

#include <typedefs.h>
#include <rte_dev.h>
#include <proto/dnglevent.h>

#define BCM_DNGL_LEN \
	(sizeof(bcm_dngl_event_t) - sizeof(bcmeth_hdr_t) - sizeof(struct ether_header))

/* Common hnd event library routines: init library and create a event */
extern void hnd_event_init(osl_t *osh, hnd_dev_t *busdev);
extern int hnd_send_dngl_event(uint32 dngl_event, uchar *data, uint32 datalen);

/* Helper routine to create a SOCRAM ASSERT INDICATE EVENT */
#define SOCRAM_ASSRT_INDICATE() \
	hnd_event_socramind_assrt(__FUNCTION__, (uint32)__builtin_return_address(0))
extern void hnd_event_socramind_assrt(const char *fn, uint32 caller);

#endif /* _HNDRTE_EVENT_H */
