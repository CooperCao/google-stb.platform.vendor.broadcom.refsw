/*
 * RPC OSL NDIS port
 * Broadcom 802.11abg Networking Device Driver
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
#ifdef BCMDRIVER

#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmendian.h>
#include <osl.h>
#include <bcmutils.h>

#include <rpc_osl.h>

/* WdfSpinLockAcquire/WdfSpinLockRelease */
#define NDIS_RPC_LOCK(context)		NdisAcquireSpinLock(&(context)->lock)
#define NDIS_RPC_UNLOCK(context)	NdisReleaseSpinLock(&(context)->lock)


struct rpc_osl {
	osl_t		*osh;
	bool		wakeup;

	NDIS_SPIN_LOCK	lock;
	bool		gotspinlocks;

	NDIS_EVENT	event;
};

rpc_osl_t *
rpc_osl_attach(osl_t *osh)
{
	rpc_osl_t *rpc_osh;

	if ((rpc_osh = (rpc_osl_t *)MALLOC(osh, sizeof(rpc_osl_t))) == NULL)
		return NULL;

	NdisAllocateSpinLock(&rpc_osh->lock);
	rpc_osh->gotspinlocks = TRUE;

	NdisInitializeEvent(&rpc_osh->event);

	rpc_osh->wakeup = FALSE;
	rpc_osh->osh = osh;

	return rpc_osh;
}

void
rpc_osl_detach(rpc_osl_t *rpc_osh)
{
	if (!rpc_osh)
		return;

	if (rpc_osh->gotspinlocks) {
		NdisFreeSpinLock(&rpc_osh->lock);
		rpc_osh->gotspinlocks = FALSE;
	}

	NdisResetEvent(&rpc_osh->event);


	MFREE(rpc_osh->osh, rpc_osh, sizeof(rpc_osl_t));
}

int
rpc_osl_wait(rpc_osl_t *rpc_osh, uint ms, bool *ptimedout)
{
	bool ret;

	/* No reentrancy */
	ASSERT(rpc_osh->wakeup == FALSE);

	/* Wake me up */
	rpc_osh->wakeup = TRUE;

	/* yield the control back to OS, wait for wake_up() call up to timeout ms */
	ret = NdisWaitEvent(&rpc_osh->event, ms);

	NdisResetEvent(&rpc_osh->event);

	if (!ret) {
		printf("!!!rpc_osl_wait %d ms failed!!!\n", ms);
		if (ptimedout)
			*ptimedout = TRUE;
	}

	/* Don't wake me up any more */
	rpc_osh->wakeup = FALSE;

	return 0;
}

void
rpc_osl_wake(rpc_osl_t *rpc_osh)
{
	/* Wake up the waiting thread */
	NdisSetEvent(&rpc_osh->event);
}

void
rpc_osl_lock(rpc_osl_t *rpc_osh)
{
	NDIS_RPC_LOCK(rpc_osh);
}

void
rpc_osl_unlock(rpc_osl_t *rpc_osh)
{
	NDIS_RPC_UNLOCK(rpc_osh);
}


#endif /* BCMDRIVER */
