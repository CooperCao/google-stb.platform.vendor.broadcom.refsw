/*
 * SDIO access interface for generic drivers that do not use an operating
 * system specific driver model.
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


/* ---- Include Files ---------------------------------------------------- */

#include "typedefs.h"
#include "osl.h"
#include "bcmsdh.h"


/* ---- Public Variables ------------------------------------------------- */
/* ---- Private Constants and Types -------------------------------------- */
/* ---- Private Variables ------------------------------------------------ */

/* driver info, initialized when bcmsdh_register is called */
static bcmsdh_driver_t g_drvinfo;

static bcmsdh_info_t *g_sdh;


/* ---- Private Function Prototypes -------------------------------------- */
/* ---- Functions -------------------------------------------------------- */


/* ----------------------------------------------------------------------- */
int
bcmsdh_register(bcmsdh_driver_t *driver)
{
	g_drvinfo = *driver;
	return 0;
}


/* ----------------------------------------------------------------------- */
void
bcmsdh_unregister(void)
{
	memset(&g_drvinfo, 0, sizeof(g_drvinfo));
}


/* ----------------------------------------------------------------------- */
void
bcmsdh_remove(osl_t *osh, void *instance)
{
	if (instance)
		g_drvinfo.detach(instance);

	if (g_sdh)
		bcmsdh_detach(osh, g_sdh);
}


/* ----------------------------------------------------------------------- */
void*
bcmsdh_probe(osl_t *osh)
{
	void	*bus;
	uint32	vendevid;
	void	*regsva;

	bus = NULL;
	g_sdh = NULL;

	g_sdh = bcmsdh_attach(osh, NULL, &regsva, 0);
	if (g_sdh == NULL) {
		BCMSDH_ERROR(("%s: bcmsdh_attach failed\n", __FUNCTION__));
		goto err;
	}

	/* Read the real manufacturing and device ID from the CIS */
	vendevid = bcmsdh_query_device(g_sdh);

	/* try to attach to the target device */
	bus = g_drvinfo.attach((vendevid >> 16), (vendevid & 0xFFFF),
	                               0, 0, 0, 0, regsva, osh, g_sdh);
	if (bus == NULL) {
		BCMSDH_ERROR(("%s: device attach failed\n", __FUNCTION__));
		goto err;
	}

	return (bus);

	/* error handling */
err:
	bcmsdh_remove(osh, bus);

	return (NULL);
}
