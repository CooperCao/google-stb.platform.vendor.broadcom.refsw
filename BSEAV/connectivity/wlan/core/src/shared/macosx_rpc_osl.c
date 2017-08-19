/*
 * RPC OSL Mac OS port
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id$
 */

#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmendian.h>
#include <osl.h>
#include <bcmutils.h>
#include <rpc_osl.h>

struct rpc_osl {
	osl_t *osh;
};

rpc_osl_t *
rpc_osl_attach(osl_t *osh)
{
	rpc_osl_t *rpc_osh;

	rpc_osh = (rpc_osl_t *)MALLOC(osh, sizeof(rpc_osl_t));

	if (rpc_osh == NULL)
		return NULL;

	memset(rpc_osh, 0, sizeof(rpc_osl_t));

	rpc_osh->osh = osh;

	return rpc_osh;
}

void
rpc_osl_detach(rpc_osl_t *rpc_osh)
{
	if (!rpc_osh)
		return;
}

int
rpc_osl_wait(rpc_osl_t *rpc_osh, uint ms, bool *ptimedout)
{
	return 0;
}

void
rpc_osl_wake(rpc_osl_t *rpc_osh)
{
}

void
rpc_osl_lock(rpc_osl_t *rpc_osh)
{
}

void
rpc_osl_unlock(rpc_osl_t *rpc_osh)
{
}
