/*
 * Header file for save-restore memmory functionality in driver.
 *
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
 * $Id: hndsrmem.h 611946 2016-01-12 14:39:59Z $
 */

#ifndef	_hndsrmem_h_
#define	_hndsrmem_h_

#if !defined(BCMDONGLEHOST) && defined(SRMEM)

#include <rte.h>
#include <hnd_pkt.h>
#include <hnd_lbuf.h>
#include <siutils.h>
#include <bcmutils.h>

/* WL_ENAB_RUNTIME_CHECK may be set based upon the #define below (for ROM builds). It may also
 * be defined via makefiles (e.g. ROM auto abandon unoptimized compiles).
 */

typedef struct srmem_info srmem_t;

extern srmem_t *srmem;

	extern bool _srmem;
	#define SRMEM_ENAB()   (_srmem)

void *srmem_alloc(srmem_t *srmem, uint size);
void srmem_inused(srmem_t *srmem, struct lbuf *p, bool inused);
void srmem_enable(srmem_t *srmem, bool enable);
void srmem_init(si_t *sih);
void srmem_deinit(srmem_t *srmem);

#define PKTSRGET(len)		(void *)srmem_alloc(srmem, (len))
#define PKTSRMEM_DEC_INUSE(p) srmem_inused(srmem, (struct lbuf *)(p), FALSE)
#define PKTSRMEM_INC_INUSE(p) srmem_inused(srmem, (struct lbuf *)(p), TRUE)

#define SRMEM_ENABLE(enable)	srmem_enable(srmem, enable)

#else /* SRMEM */

#define SRMEM_ENAB() (0)

#define srmem_alloc(srmem, size)	(NULL)
#define srmem_inused(srmem, p, inused)
#define srmem_init(sih)
#define srmem_deinit(srmem)

#define PKTSRGET(len)		(void *)(NULL)
#define PKTSRMEM_INC_INUSE(p)
#define PKTSRMEM_DEC_INUSE(p)

#define SRMEM_ENABLE(enable)

#endif /* SRMEM */

#endif	/* _hndsrmem_ */
