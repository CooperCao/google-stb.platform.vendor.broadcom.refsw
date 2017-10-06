/*
 * Dongle BUS interface
 * SDIO NDIS Implementation
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
#include <osl.h>
#include <bcmdefs.h>
#include <bcmdevs.h>
#include <bcmutils.h>
#include <bcmendian.h>
#include <ndis.h>
#if (0>= 0x0600)
#include <wdf.h>
#include <WdfMiniport.h>
#endif
#include "dbus.h"

typedef struct {
	dbus_pub_t *pub; /* Must be first */

	void *cbarg;
	dbus_intf_callbacks_t *cbs;

#if (0>= 0x0600)
	WDFDEVICE WdfDevice;
#endif
	NDIS_HANDLE AdapterHandle;

#ifdef BCMDONGLEHOST
	NDIS_SPIN_LOCK lock;
	NDIS_SPIN_LOCK rxlock;
	NDIS_SPIN_LOCK txlock;
#else
	shared_sem_t lock;
	shared_sem_t rxlock;
	shared_sem_t txlock;
	shared_sem_t wilock;
#endif /* BCMDONGLEHOST */

	bool lock_init;

#ifdef BCMDONGLEHOST
	/* Thread based operation */
	uint tickcnt;
	NDIS_MINIPORT_TIMER wd_timer;
	bool wd_timer_valid;

	NDIS_TIMER  dpc_timer;
	NDIS_MINIPORT_TIMER txq_timer;
#else
#if (0>= 0x0600)
	NDIS_HANDLE txq_work_item;
#else
	NDIS_WORK_ITEM txq_work_item;
#endif 
	uint workitem_callbacks;
	bool work_item_init;
	bool work_item_sched;
#endif /* BCMDONGLEHOST */
} sdos_info_t;

/* Global function prototypes */
extern int bcmsdh_probe(shared_info_t *sh, void* bar, void** regsva,
                        uint irq, void ** probe_arg, void * wl);
extern void bcmsdh_remove(osl_t *osh, void *instance, shared_info_t *sh);
#ifdef BCMDONGLEHOST
extern void sdstd_status_intr_enable(void *sdh, bool enable);
#endif /* BCMDONGLEHOST */

/* Local function prototypes */
static void dbus_sdos_send_complete(void *arg);
static void dbus_sdos_recv_complete(void *arg, dbus_irb_rx_t *rxirb);
static void dbus_sdos_ctl_complete(sdos_info_t *sdos_info, int type);
static int dbus_sdos_errhandler(void *bus, int err);
static int dbus_sdos_state_change(void *bus, int state);
static void dbus_sdos_disconnect_cb(void);
static void dbus_sdos_probe_dpc(ulong data);
static int dhd_probe_thread(void *data);
static void dbus_sdos_dpc(void *arg);
static void dbus_sdos_isr(bool *InterruptRecognized, bool *QueueMiniportHandleInterrupt, void *arg);

/* Functions shared between dbus_sdio.c/dbus_sdio_os.c */
extern int dbus_sdio_txq_process(void *cbarg);

/* This stores SDIO info during NDIS probe callback
 * since attach() is not called yet at this point
 */
typedef struct {
	void *sdos_info;

#if (0< 0x0600)
	NDIS_MINIPORT_INTERRUPT intr;   /* interrupt object */
	uint intrvector;     /* interrupt vector */
	uint intrlevel;      /* interrupt level */
#else
	NDIS_HANDLE     intr;           /* interrupt object */
#endif 
	NDIS_PHYSICAL_ADDRESS bar0;
	uint32 bar0_size;

#ifdef BCMDONGLEHOST
	NDIS_SPIN_LOCK mhalt_lock; /* Blocks mhalt() while dpc() is active() */
#else
	shared_sem_t mhalt_lock;
#endif /* BCMDONGLEHOST */

	bool mhalted;
	bool in_dpc;
} probe_info_t;

typedef struct {
	void *context;
} work_tcb_t;

static work_tcb_t probe_work;
static probe_info_t g_probe_info;
#if (0>= 0x0600)
static WDFDEVICE	g_WdfDevice = NULL;
#endif
static NDIS_HANDLE	g_AdapterHandle = NULL;
static shared_info_t *g_sh = NULL;
extern bcmsdh_driver_t sdh_driver;

#define MOD_PARAM_PATHLEN       2048
extern char firmware_path[MOD_PARAM_PATHLEN];
extern char nvram_path[MOD_PARAM_PATHLEN];
/* load firmware and/or nvram values from the filesystem */

/* Watchdog frequency */
uint dhd_watchdog_ms = 10;

/* Watchdog thread priority, -1 to use kernel timer */
int dhd_watchdog_prio = 97;

/* DPC thread priority, -1 to use tasklet */
int dhd_dpc_prio = 98;

/* DPC thread priority, -1 to use tasklet */
extern int dhd_dongle_memsize;

#define DHD_IDLETIME_TICKS 1

/* Idle timeout for backplane clock */
int dhd_idletime = DHD_IDLETIME_TICKS;

/* Use polling */
uint dhd_poll = FALSE;

/* Use interrupts */
uint dhd_intr = TRUE;

/* SDIO Drive Strength (in milliamps) */
uint dhd_sdiod_drive_strength = 6;

/* Tx/Rx bounds */
extern uint dhd_txbound;
extern uint dhd_rxbound;

/* Deferred transmits */
extern uint dhd_deferred_tx;




/*
 * SDIO Linux dbus_intf_t
 */
static void * dbus_sdos_attach(dbus_pub_t *pub, void *cbarg, dbus_intf_callbacks_t *cbs);
static void dbus_sdos_detach(dbus_pub_t *pub, void *info);
static int dbus_sdos_send_irb(void *bus, dbus_irb_tx_t *txirb);
static int dbus_sdos_recv_irb(void *bus, dbus_irb_rx_t *rxirb);
static int dbus_sdos_cancel_irb(void *bus, dbus_irb_tx_t *txirb);
static int dbus_sdos_send_ctl(void *bus, uint8 *buf, int len);
static int dbus_sdos_recv_ctl(void *bus, uint8 *buf, int len);
static int dbus_sdos_get_attrib(void *bus, dbus_attrib_t *attrib);
static int dbus_sdos_pnp(void *bus, int event);
static int dbus_sdos_remove(void *bus);
static int dbus_sdos_up(void *bus);
static int dbus_sdos_down(void *bus);
static int dbus_sdos_stop(void *bus);
static bool dbus_sdos_device_exists(void *bus);
static bool dbus_sdos_dlneeded(void *bus);
static int dbus_sdos_dlstart(void *bus, uint8 *fw, int len);
static int dbus_sdos_dlrun(void *bus);
static bool dbus_sdos_recv_needed(void *bus);
static void *dbus_sdos_exec_rxlock(void *bus, exec_cb_t cb, struct exec_parms *args);
static void *dbus_sdos_exec_txlock(void *bus, exec_cb_t cb, struct exec_parms *args);
static int dbus_sdos_sched_dpc(void *bus);
static int dbus_sdos_lock(void *bus);
static int dbus_sdos_unlock(void *bus);
static int dbus_sdos_sched_probe_cb(void *bus);

static dbus_intf_t dbus_sdos_intf = {
	dbus_sdos_attach,
	dbus_sdos_detach,
	dbus_sdos_up,
	dbus_sdos_down,
	dbus_sdos_send_irb,
	dbus_sdos_recv_irb,
	dbus_sdos_cancel_irb,
	dbus_sdos_send_ctl,
	dbus_sdos_recv_ctl,
	NULL, /* get_stats */
	dbus_sdos_get_attrib,
	dbus_sdos_pnp,
	dbus_sdos_remove,
	NULL, /* resume */
	NULL, /* suspend */
	dbus_sdos_stop,
	NULL, /* reset */
	NULL, /* pktget */
	NULL, /* pktfree */
	NULL, /* iovar_op */
	NULL, /* dump */
	NULL, /* set_config */
	NULL, /* get_config */
	dbus_sdos_device_exists,
	dbus_sdos_dlneeded,
	dbus_sdos_dlstart,
	dbus_sdos_dlrun,
	dbus_sdos_recv_needed,
	dbus_sdos_exec_rxlock,
	dbus_sdos_exec_txlock,
	NULL, /* tx_timer_init */
	NULL, /* tx_timer_start */
	NULL, /* tx_timer_stop */
	dbus_sdos_sched_dpc,
	dbus_sdos_lock,
	dbus_sdos_unlock,
	dbus_sdos_sched_probe_cb

	/* shutdown */

	/* recv_stop */
	/* recv_resume */
};

static probe_cb_t probe_cb = NULL;
static disconnect_cb_t disconnect_cb = NULL;
static void *probe_arg = NULL;
static void *disc_arg = NULL;

#ifdef BCMDONGLEHOST
#define LOCK_INIT(lock) NdisAllocateSpinLock(lock)
#define LOCK(lock) NdisAcquireSpinLock(lock)
#define UNLOCK(lock) NdisReleaseSpinLock(lock)
#define LOCK_FREE(lock) NdisFreeSpinLock(lock)
#else
#define LOCK_INIT(lock) shared_sem_init(lock, 1)
#define LOCK(lock) shared_sem_acquire(lock)
#define UNLOCK(lock) shared_sem_release(lock)
#define LOCK_FREE(lock)

typedef void (*workitem_cb_t)(void *arg1, void *arg2);
static int dbus_sdos_init_workitem(sdos_info_t *sdos_info, void *work_item);
static void dbus_sdos_queue_workitem(sdos_info_t *sdos_info, void *work_item, workitem_cb_t fn);
static void dbus_sdos_free_work_item(sdos_info_t *sdos_info, void *work_item);
static void dbus_txq_workitem(void *arg1, void *arg2);
#endif /* BCMDONGLEHOST */

#define MAX_WORKITEM_DLY 20000	/* in us */
#define MAX_DPC_DLY 20000	/* in us */

static void
dbus_sdos_disconnect_cb()
{
	if (disconnect_cb)
		disconnect_cb(disc_arg);
}

static void
dbus_sdos_send_complete(void *arg)
{
	sdos_info_t *sdos_info = arg;
	dbus_irb_tx_t *txirb = NULL;
	int status = DBUS_OK;

	if (sdos_info->cbarg && sdos_info->cbs) {
		if (sdos_info->cbs->send_irb_complete)
			sdos_info->cbs->send_irb_complete(sdos_info->cbarg, txirb, status);
	}
}

static void
dbus_sdos_recv_complete(void *arg, dbus_irb_rx_t *rxirb)
{
	int status = DBUS_OK;
	sdos_info_t *sdos_info = arg;

	if (sdos_info->cbarg && sdos_info->cbs) {
		if (sdos_info->cbs->recv_irb_complete)
			sdos_info->cbs->recv_irb_complete(sdos_info->cbarg, rxirb, status);
	}
}

static void
dbus_sdos_ctl_complete(sdos_info_t *sdos_info, int type)
{
	int status = DBUS_ERR;

	if (sdos_info == NULL)
		return;

	if (sdos_info->cbarg && sdos_info->cbs) {
		if (sdos_info->cbs->ctl_complete)
			sdos_info->cbs->ctl_complete(sdos_info->cbarg, type, status);
	}
}

static void
dbusos_stop(sdos_info_t *sdos_info)
{
#ifdef BCMDONGLEHOST
	bool cancel;

	/* Clear the watchdog timer */
	cancel = FALSE;
	NdisMCancelTimer(&sdos_info->wd_timer, &cancel);
	sdos_info->wd_timer_valid = FALSE;

	cancel = FALSE;
	NdisCancelTimer(&sdos_info->dpc_timer, &cancel);
#endif /* BCMDONGLEHOST */
}

static bool
dbus_sdos_device_exists(void *bus)
{
	return TRUE;
}

static bool
dbus_sdos_dlneeded(void *bus)
{
	return FALSE;
}

static int
dbus_sdos_dlstart(void *bus, uint8 *fw, int len)
{
	return DBUS_ERR;
}

static int
dbus_sdos_dlrun(void *bus)
{
	return DBUS_ERR;
}

static bool
dbus_sdos_recv_needed(void *bus)
{
	return FALSE;
}

static void*
dbus_sdos_exec_rxlock(void *bus, exec_cb_t cb, struct exec_parms *args)
{
	sdos_info_t *sdos_info = (sdos_info_t *) bus;
	void *ret;

	if (sdos_info == NULL)
		return NULL;

	LOCK(&sdos_info->rxlock);
	ret = cb(args);
	UNLOCK(&sdos_info->rxlock);

	return ret;
}

static void*
dbus_sdos_exec_txlock(void *bus, exec_cb_t cb, struct exec_parms *args)
{
	sdos_info_t *sdos_info = (sdos_info_t *) bus;
	void *ret;

	if (sdos_info == NULL)
		return NULL;

	LOCK(&sdos_info->txlock);
	ret = cb(args);
	UNLOCK(&sdos_info->txlock);

	return ret;
}

static int
dbus_sdos_sched_dpc(void *bus)
{
	sdos_info_t *sdos_info = (sdos_info_t *) bus;

	if (sdos_info == NULL)
		return DBUS_ERR;

	return DBUS_OK;
}

static int
dbus_sdos_lock(void *bus)
{
	sdos_info_t *sdos_info = (sdos_info_t *) bus;

	if (sdos_info == NULL)
		return DBUS_ERR;

	LOCK(&sdos_info->lock);
	return DBUS_OK;
}

static int
dbus_sdos_unlock(void *bus)
{
	sdos_info_t *sdos_info = (sdos_info_t *) bus;

	if (sdos_info == NULL)
		return DBUS_ERR;

	UNLOCK(&sdos_info->lock);
	return DBUS_OK;
}

static int
dbus_sdos_sched_probe_cb(void *bus)
{
	return DBUS_OK;
}

static int
dbus_sdos_probe()
{
	int ep;
	int ret = DBUS_OK;
	ULONG nameLength;
	NTSTATUS ntStatus;
	void *usbos_info;

	DBUSTRACE(("%s:\n", __FUNCTION__));

	if (probe_cb) {
		disc_arg = probe_cb(probe_arg, "", SDIO_BUS, 0);
		if (disc_arg == NULL) {
			DBUSERR(("g_probe_cb return NULL\n"));
			return DBUS_ERR;
		}
	}

	return ret;
}

static void
dbus_sdos_disconnect()
{
	DBUSTRACE(("%s:\n", __FUNCTION__));

	if (disconnect_cb) {
		disconnect_cb(disc_arg);
	}
}

#ifdef BCMDONGLEHOST
static void
dbus_wd_timer_init(sdos_info_t *sdos_info, uint wdtick)
{
	bool cancel = FALSE;

	/* Stop timer and restart at new value */
	if (sdos_info->wd_timer_valid == TRUE) {
		NdisMCancelTimer(&sdos_info->wd_timer, &cancel);
		sdos_info->wd_timer_valid = FALSE;
	}

	dhd_watchdog_ms = (uint)wdtick;
	sdos_info->wd_timer_valid = FALSE;
}

static int
dhd_watchdog_thread(void *data)
{
	sdos_info_t *sdos_info = (sdos_info_t *)data;

	/* Run until signal received */
	while (1) {
		if (sdos_info->pub->busstate != DBUS_STATE_DOWN) {
			if (sdos_info->cbarg && sdos_info->cbs) {
				if (sdos_info->cbs->watchdog)
					sdos_info->cbs->watchdog(sdos_info->cbarg);
			}
		}

		/* Count the tick for reference */
		sdos_info->tickcnt++;

		/* Reschedule the watchdog */
		if (sdos_info->wd_timer_valid) {
			NdisMSetTimer(&sdos_info->wd_timer, dhd_watchdog_ms);
		}
	}
}

static void
dbus_wd_timer(PVOID sysarg1, PVOID context, PVOID sysarg2, PVOID sysarg3)
{
	sdos_info_t *sdos_info = (sdos_info_t *)context;

	if (sdos_info == NULL)
		return;
}

static void
dbus_dpc_timer(PVOID sysarg1, PVOID context, PVOID sysarg2, PVOID sysarg3)
{
	sdos_info_t *sdos_info = (sdos_info_t *)context;

	if (sdos_info == NULL)
		return;

	if (sdos_info->pub->busstate != DBUS_STATE_DOWN) {
		if (sdos_info->cbarg && sdos_info->cbs) {
			if (sdos_info->cbs->dpc)
				if (sdos_info->cbs->dpc(sdos_info->cbarg, FALSE)) {
					printf("Reschedule DPC\n");
					dbus_sdos_sched_dpc(sdos_info);
				}
		}
	}
}

static void
dbus_txq_timer(PVOID systemspecific1, NDIS_HANDLE context,
	PVOID systemspecific2, PVOID systemspecific3)
{
	sdos_info_t *sdos_info = (sdos_info_t *)context;

	if (sdos_info == NULL)
		return;

	dbus_sdio_txq_process(sdos_info->cbarg);
}
#endif /* BCMDONGLEHOST */

static void
dbus_sdos_isr(bool *InterruptRecognized, bool *QueueMiniportHandleInterrupt, void *arg)
{
	probe_info_t *pinfo = arg;
	sdos_info_t *sdos_info = pinfo->sdos_info;
	bool wantdpc = FALSE;

	if (sdos_info == NULL)
		return;

	*InterruptRecognized = FALSE; /* Not our interrupt */
	*QueueMiniportHandleInterrupt = FALSE; /* Do not schedule dpc */

	if (sdos_info->cbarg && sdos_info->cbs) {
		if (sdos_info->cbs->isr)
			if (sdos_info->cbs->isr(sdos_info->cbarg, &wantdpc) == TRUE) {
				/* Our interrupt */
				*InterruptRecognized = TRUE;

				if (wantdpc == TRUE)
					/* Schedule DPC */
					*QueueMiniportHandleInterrupt = TRUE;
			}
	}
}

static void
dbus_sdos_dpc(void *arg)
{
	probe_info_t *pinfo = arg;
	sdos_info_t *sdos_info = pinfo->sdos_info;

	if (sdos_info == NULL)
		return;

	pinfo->in_dpc = TRUE;

	LOCK(&pinfo->mhalt_lock);
	if (pinfo->mhalted == TRUE) {
		DBUSERR(("Exit dpc...driver is halted\n"));
		goto dpc_exit;
	}

	if (sdos_info->cbarg && sdos_info->cbs) {
		if (sdos_info->cbs->dpc) {
			if (sdos_info->cbs->dpc(sdos_info->cbarg, FALSE)) {
				/* FIX: Reschedule here for NDIS?? */
			}

#ifdef BCMDONGLEHOST
			/*
			* FIX: Must do this for interrupt mode
			* sdstd_isr()-->isr_cb() turns this off; we need to turn it back on
			*/
			if (g_sh != NULL)
				sdstd_status_intr_enable(g_sh->sdh, TRUE);
			else
				DBUSERR(("g_sh is NULL\n"));
#endif /* BCMDONGLEHOST */
		}

	}

dpc_exit:
	UNLOCK(&pinfo->mhalt_lock);
	pinfo->in_dpc = FALSE;
}

static int
dhd_probe_thread(void *data)
{
	probe_info_t *pinfo = (probe_info_t *) data;

	if (probe_cb) {
		disc_arg = probe_cb(probe_arg, "", 0, 0);
	}
}

/* FIX: Need to tight this into dbus_sdio.c */
static void *
dbus_sdos_open_image(char * filename)
{
	 return NULL;
}

/* FIX: Need to tight this into dbus_sdio.c */
static int
dbus_sdos_get_image_block(char * buf, int len, void * image)
{
	return 0;
}

/* FIX: Need to tight this into dbus_sdio.c */
static void
dbus_sdos_close_image(void * image)
{
}

static void
dbus_sdos_probe_dpc(ulong data)
{
	probe_info_t *pinfo;

	pinfo = (probe_info_t *) data;
	if (probe_cb) {
		disc_arg = probe_cb(probe_arg, "", 0, 0);
	}
}

static int
dbus_sdos_send_ctl(void *bus, uint8 *buf, int len)
{
	sdos_info_t *sdos_info = (sdos_info_t *) bus;

	if ((sdos_info == NULL) || (buf == NULL) || (len == 0))
		return DBUS_ERR;

	return DBUS_OK;
}

static int
dbus_sdos_recv_ctl(void *bus, uint8 *buf, int len)
{
	sdos_info_t *sdos_info = (sdos_info_t *) bus;

	if ((sdos_info == NULL) || (buf == NULL) || (len == 0))
		return DBUS_ERR;

	return DBUS_OK;
}

static int
dbus_sdos_get_attrib(void *bus, dbus_attrib_t *attrib)
{
	sdos_info_t *sdos_info = (sdos_info_t *) bus;

	if ((sdos_info == NULL) || (attrib == NULL))
		return DBUS_ERR;

	attrib->bustype = DBUS_SDIO;
	attrib->vid = 0;
	attrib->pid = 0;
	/* FIX: 4319 support? */
	attrib->devid = 0x4322;

	/* FIX: Need nchan for both TX and RX?;
	 * BDC uses one RX pipe and one TX pipe
	 * RPC may use two RX pipes and one TX pipe?
	 */
	attrib->nchan = 1;
	attrib->mtu = 0;

	return DBUS_OK;
}

static int
dbus_sdos_pnp(void *bus, int event)
{
	sdos_info_t *sdos_info = (sdos_info_t *) bus;

	if (sdos_info == NULL)
		return DBUS_ERR;

	if (event == DBUS_PNP_DISCONNECT) {
		DBUSTRACE(("Got surprise removal event\n"));
	}

	return DBUS_OK;
}

static int
dbus_sdos_remove(void *bus)
{
	sdos_info_t *sdos_info = (sdos_info_t *) bus;

	if (sdos_info == NULL)
		return DBUS_ERR;

	if (g_sh) {
#if (0< 0x0600)
		/* Force NDIS to issue NdisDevicePnPEventSurpriseRemoved */
		NdisMRemoveMiniport(g_sh->adapterhandle);
#endif
	}

	return DBUS_OK;
}

static int
dbus_sdos_up(void *bus)
{
	sdos_info_t *sdos_info = (sdos_info_t *) bus;

	if (sdos_info == NULL)
		return DBUS_ERR;

#ifdef BCMDONGLEHOST
	dbus_wd_timer_init(sdos_info, dhd_watchdog_ms);
#endif /* BCMDONGLEHOST */

	return DBUS_OK;
}

static int
dbus_sdos_down(void *bus)
{
	sdos_info_t *sdos_info = (sdos_info_t *) bus;

	return DBUS_OK;
}

static int
dbus_sdos_send_irb(void *bus, dbus_irb_tx_t *txirb)
{
	sdos_info_t *sdos_info = (sdos_info_t *) bus;
	int ret = DBUS_ERR;

	if (sdos_info == NULL)
		return DBUS_ERR;

	return ret;
}

static int
dbus_sdos_recv_irb(void *bus, dbus_irb_rx_t *rxirb)
{
	sdos_info_t *sdos_info = (sdos_info_t *) bus;
	int ret = DBUS_ERR;

	if (sdos_info == NULL)
		return DBUS_ERR;

	return ret;
}

static int
dbus_sdos_cancel_irb(void *bus, dbus_irb_tx_t *txirb)
{
	sdos_info_t *sdos_info = (sdos_info_t *) bus;

	if (sdos_info == NULL)
		return DBUS_ERR;

	/* FIX: Need to implement */
	return DBUS_ERR;
}

static int
dbus_sdos_stop(void *bus)
{
	sdos_info_t *sdos_info = (sdos_info_t *) bus;

	if (sdos_info == NULL)
		return DBUS_ERR;

	dbusos_stop(sdos_info);
	return DBUS_OK;
}

int
dbus_sdos_errhandler(void *bus, int err)
{
	sdos_info_t *sdos_info = (sdos_info_t *) bus;

	if (sdos_info == NULL)
		return DBUS_ERR;

	if (sdos_info->cbarg && sdos_info->cbs) {
		if (sdos_info->cbs->errhandler)
			sdos_info->cbs->errhandler(sdos_info->cbarg, err);
	}

	return DBUS_OK;
}

int
dbus_sdos_state_change(void *bus, int state)
{
	sdos_info_t *sdos_info = (sdos_info_t *) bus;

	if (sdos_info == NULL)
		return DBUS_ERR;

	if (sdos_info->cbarg && sdos_info->cbs) {
		if (sdos_info->cbs->state_change)
			sdos_info->cbs->state_change(sdos_info->cbarg, state);
	}

	return DBUS_OK;
}

#ifdef BCMDONGLEHOST
/* Ported from wl_ndis.c */
#define  RESSZ            (4 * 1024)
static int
dbus_sdos_query_resrc(shared_info_t *sh, probe_info_t *pinfo)
{
	PNDIS_RESOURCE_LIST reslist;
	PCM_PARTIAL_RESOURCE_DESCRIPTOR presdes;
	NDIS_STATUS status;
	int err = DBUS_OK;
	uint i;

	i = RESSZ;
	reslist = MALLOC(sh->osh, i);
	if (!reslist) {
		printf("wl%d: MALLOC error\n", sh->unit);
		err = DBUS_ERR;
		goto err;
	}

	bzero(reslist, RESSZ);
#if (0< 0x0600)
	NdisMQueryAdapterResources(&status, sh->confighandle, reslist, &i);
	if (NDIS_ERROR(status)) {
		err = DBUS_ERR;
		goto err;
	}
#endif

	/* find resources */
	for (i = 0; i < reslist->Count; i++) {
		presdes = &reslist->PartialDescriptors[i];

		switch (presdes->Type) {
		case CmResourceTypeMemory:
			/* map in BAR0 registers (ignore BAR1 for now) */
			if (pinfo->bar0_size != 0)
				continue;

			/* The standard SDIO code maps the bar */
			pinfo->bar0 = presdes->u.Memory.Start;
			pinfo->bar0_size = presdes->u.Memory.Length;
			break;
		case CmResourceTypeInterrupt:	/* save vector and level */
#if (0< 0x0600)
			pinfo->intrvector = presdes->u.Interrupt.Vector;
			pinfo->intrlevel = presdes->u.Interrupt.Level;
#endif
			break;
		default:
			break;
		}
	}

	if (pinfo->bar0_size == 0)
		err = DBUS_ERR;

#if (0< 0x0600)
	if ((pinfo->intrvector == 0) || (pinfo->intrlevel == 0))
		err = DBUS_ERR;
#endif

err:
	if (reslist)
		MFREE(sh->osh, reslist, RESSZ);
	return err;
}

/* Ported from wl_ndis.c */
static int
dbus_sdos_probe_dev(shared_info_t *sh, probe_info_t *pinfo)
{
	NDIS_STATUS status;
	int i;
	uint16 ids[2];
	int slot = 0;
	int err = DBUS_ERR;

	/* Check for Arasan Standard SDIO controller */
	/* read pci vendor and device id */
#if (0< 0x0600)
	i = NdisReadPciSlotInformation(sh->adapterhandle, slot, 0, ids, sizeof(ids));
#else
	i = NdisMGetBusData(sh->adapterhandle, PCI_WHICHSPACE_CONFIG, 0, ids, sizeof(ids));
#endif
	if (i != sizeof(ids)) {
		DBUSERR(("%s: Can't read PCI slot info, slot %d, i = %d\n",
		         __FUNCTION__, slot, i));
		err = DBUS_ERR;
		goto err;
	}

	/* Looking for Arasan STD HC or TI or Ricon STD HC */
	if (((ids[0] == VENDOR_SI_IMAGE) && (ids[1] == 0x0670)) ||
	    ((ids[0] == VENDOR_RICOH) &&
	     (ids[1] == R5C822_SDIOH_ID)) ||
	    ((ids[0] == VENDOR_TI) &&
	     (ids[1] == PCIXX21_FLASHMEDIA_ID ||
	      ids[1] == PCIXX21_FLASHMEDIA0_ID ||
	      ids[1] == PCIXX21_SDIOH0_ID ||
	      ids[1] == PCIXX21_SDIOH_ID)) ||
	    ((ids[0] == VENDOR_BROADCOM) &&
	     (ids[1] == SPIH_FPGA_ID || ids[1] == SDIOH_FPGA_ID))) {
		err = dbus_sdos_query_resrc(sh, pinfo);
	}
err:
	return err;
}
#endif /* BCMDONGLEHOST */

int
dbus_bus_osl_register(int vid, int pid, probe_cb_t prcb,
	disconnect_cb_t discb, void *prarg, dbus_intf_t **intf, void *param1, void *param2)
{
	int ret = 0;
	int err = DBUS_OK;
	void *regsva;
	NDIS_STATUS status;
	int irq = 0;
	probe_info_t *pinfo = &g_probe_info;

	probe_cb = prcb;
	disconnect_cb = discb;
	probe_arg = prarg;

	*intf = &dbus_sdos_intf;
	bzero(pinfo, sizeof(probe_info_t));

	g_AdapterHandle = param1;
	if (g_AdapterHandle == NULL) {
		DBUSERR(("Expects param1 to be MiniportHandle\n"));
		return DBUS_ERR_REG_PARAM;
	}

	/* shared_info_t is used by:
	 *   wl_ndis.c, ndshared.c, bcmsdh_ndis.c, bcmsdstd_ndis.c, and dbus_sdio_ndis.c
	 * To register for interrupt: sh->intr are sh->irq_level but are also
	 * initialized in wl_ndis.c.  Now, DBUS will initialize these explicitly
	 * but leave the other initializations as is because SDIO NIC uses them.
	 */
	g_sh = param2;
	if (g_sh == NULL) {
		DBUSERR(("Expects param2 to be shared_info_t\n"));
		return DBUS_ERR_REG_PARAM;
	}

#if (0>= 0x0600)
	g_WdfDevice = g_sh->wdfdevice;
	if (g_WdfDevice == NULL)
		DBUSERR(("WdfDevice is NULL!\n"));
#endif

#ifdef BCMDONGLEHOST	/* for certain type of PCI/SDIO adapters */
	err = dbus_sdos_probe_dev(g_sh, pinfo);
	if (err != DBUS_OK) {
		DBUSERR(("dbus_sdos_probe_dev FAILED\n"));
		goto err;
	}
#endif /* BCMDONGLEHOST */

	/* Initialize relevant shared_info_t fields */
#if (0< 0x0600)
	g_sh->intr = &pinfo->intr;
	g_sh->irq_level = pinfo->intrlevel;
#endif
	/* Turn on polling */
	g_sh->poll_mode = FALSE;

	/* FIX: dbus_sdio.c uses interrupt mode which needs isr()/dpc()
	 * registered before bcmsdh_probe() but shared_interrupt_register()
	 * needs sdh handle which is called in bcmsdh_probe();
	 * So need to preload these handlers first; Need to fix.
	 */
	g_sh->isr_cb = dbus_sdos_isr;
	g_sh->dpc_cb = dbus_sdos_dpc;
	g_sh->isr_arg = g_sh->dpc_arg = pinfo;

#if (0< 0x0600)
	irq = pinfo->intrvector;
#endif

	LOCK_INIT(&pinfo->mhalt_lock);

	if (bcmsdh_register(&sdh_driver) < 0) {
		err = DBUS_ERR;
		goto err;
	}

	ret = bcmsdh_probe(g_sh, &pinfo->bar0, &regsva, irq, &disc_arg, 0);
	if (ret < 0) {
		err = DBUS_ERR;
		goto err;
	}

#ifdef BCMDONGLEHOST /* for big Windows interrupt registered to sdio bus driver */
	/* FIX: dbus_sdio.c uses interrupt mode which needs isr()/dpc()
	 * register before bcmsdh_probe() but shared_interrupt_register()
	 * needs sdh handle;
	 *
	 * Also, this need bcmsdh_attach() to happen before this call
	 */
#if (0< 0x0600)
	status = shared_interrupt_register(0, 0, 0, 0, 0, 0, 0, g_sh,
		dbus_sdos_isr, pinfo, dbus_sdos_dpc, pinfo);
#else
	status = shared_interrupt_register(g_sh->adapterhandle,
		g_sh->adaptercontext, NULL, NULL,
		NULL, NULL, NULL);

	/* FIX: Need to fix and test for NDIS60 */
	status = NDIS_STATUS_FAILURE;
#endif

	if (NDIS_ERROR(status)) {
		DBUSERR(("shared_interrupt_register failed = 0x%x\n", status));
		err = DBUS_ERR;
		goto err;
	}
#endif /* BCMDONGLEHOST */

err:
	return err;
}

int
dbus_bus_osl_deregister()
{
	probe_info_t *pinfo;
	pinfo = &g_probe_info;

	if (g_sh == NULL) {
		DBUSERR(("ERROR: g_sh is NULL\n"));
		return DBUS_ERR;
	}

	LOCK(&pinfo->mhalt_lock);
	pinfo->mhalted = TRUE;
	UNLOCK(&pinfo->mhalt_lock);

#ifdef BCMDONGLEHOST /* for big Windows interrupt registered to sdio bus driver */
#if (0< 0x0600)
	/* Do this asap because we don't want isr to fire */
	shared_interrupt_deregister(g_sh->intr, g_sh);
#endif
#endif /* BCMDONGLEHOST */

	bcmsdh_remove(g_sh->osh, disc_arg, g_sh);
	g_sh = NULL;
	disc_arg = NULL;

	if (pinfo->in_dpc)
		SPINWAIT(pinfo->in_dpc, MAX_DPC_DLY);
	ASSERT(pinfo->in_dpc == 0);

	LOCK_FREE(&pinfo->mhalt_lock);

	bcmsdh_unregister();
	return DBUS_OK;
}

static void *
dbus_sdos_attach(dbus_pub_t *pub, void *cbarg, dbus_intf_callbacks_t *cbs)
{
	sdos_info_t *sdos_info;

	sdos_info = MALLOC(pub->osh, sizeof(sdos_info_t));
	if (sdos_info == NULL)
		return NULL;

	ASSERT(OFFSETOF(sdos_info_t, pub) == 0);

	bzero(sdos_info, sizeof(sdos_info_t));

	sdos_info->pub = pub;
	sdos_info->cbarg = cbarg;
	sdos_info->cbs = cbs;

	sdos_info->AdapterHandle = g_AdapterHandle;
#if (0>= 0x0600)
	sdos_info->WdfDevice = g_WdfDevice;
#endif

	/* initialize locks */
	LOCK_INIT(&sdos_info->rxlock);
	LOCK_INIT(&sdos_info->txlock);
	LOCK_INIT(&sdos_info->lock);
#ifndef BCMDONGLEHOST
	LOCK_INIT(&sdos_info->wilock);
#endif /* !BCMDONGLEHOST */

	sdos_info->lock_init = TRUE;

#ifdef BCMDONGLEHOST
	/* Set up the watchdog timer */
	NdisMInitializeTimer(&sdos_info->wd_timer, sdos_info->AdapterHandle,
		dbus_wd_timer, sdos_info);
	NdisInitializeTimer(&sdos_info->dpc_timer, dbus_dpc_timer, (PVOID)sdos_info);
	NdisMInitializeTimer(&sdos_info->txq_timer, sdos_info->AdapterHandle,
		dbus_txq_timer, (PVOID)sdos_info);
#else
	/* init workitem */
	if (dbus_sdos_init_workitem(sdos_info, &sdos_info->txq_work_item) != DBUS_OK) {
		dbus_sdos_detach(pub, sdos_info);
		return NULL;
	}
	sdos_info->work_item_init = TRUE;
#endif /* BCMDONGLEHOST */

	/* Needed for disconnect() */
	g_probe_info.sdos_info = sdos_info;

	return (void *) sdos_info;
}

static void
dbus_sdos_detach(dbus_pub_t *pub, void *info)
{
	sdos_info_t *sdos_info = (sdos_info_t *) info;
	osl_t *osh = pub->osh;

	if (sdos_info == NULL)
		return;

	dbusos_stop(sdos_info);

	if (sdos_info->lock_init) {
		LOCK_FREE(&sdos_info->lock);
		LOCK_FREE(&sdos_info->rxlock);
		LOCK_FREE(&sdos_info->txlock);
#ifndef BCMDONGLEHOST
		LOCK_FREE(&sdos_info->wilock);
#endif /* !BCMDONGLEHOST */
	}

#ifndef BCMDONGLEHOST
	if (sdos_info->work_item_init)
		dbus_sdos_free_work_item(sdos_info, &sdos_info->txq_work_item);
#endif /* !BCMDONGLEHOST */

	g_probe_info.sdos_info = NULL;
	MFREE(osh, sdos_info, sizeof(sdos_info_t));
}

int
dbus_sdio_txq_sched(void *bus)
{
	sdos_info_t *sdos_info = (sdos_info_t *) bus;

	if (sdos_info == NULL)
		return DBUS_ERR;

#ifdef BCMDONGLEHOST
	NdisMSetTimer(&sdos_info->txq_timer, 0);
#else
	dbus_sdos_queue_workitem(sdos_info, &sdos_info->txq_work_item, dbus_txq_workitem);
#endif /* BCMDONGLEHOST */

	return DBUS_OK;
}

int
dbus_sdio_txq_stop(void *bus)
{
	sdos_info_t *sdos_info = (sdos_info_t *) bus;

	if (sdos_info == NULL)
		return DBUS_ERR;

#ifdef BCMDONGLEHOST
	{
		bool cancel = FALSE;
		NdisMCancelTimer(&sdos_info->txq_timer, &cancel);
	}
#else
	/* waiting for pending workitem finished */
	if (sdos_info->workitem_callbacks)
		SPINWAIT(sdos_info->workitem_callbacks, MAX_WORKITEM_DLY);
	ASSERT(sdos_info->workitem_callbacks == 0);
#endif /* BCMDONGLEHOST */

	return DBUS_OK;
}

int
dbus_sdio_dpc_stop(void *bus)
{
	sdos_info_t *sdos_info = (sdos_info_t *) bus;

	if (sdos_info == NULL)
		return DBUS_ERR;

	return DBUS_OK;
}

#ifndef BCMDONGLEHOST
static void
dbus_txq_workitem(void *arg1, void *arg2)
{
#if (0>= 0x0600)
	sdos_info_t *sdos_info = (sdos_info_t*)arg1;
#else
	sdos_info_t *sdos_info = (sdos_info_t*)arg2;
#endif 

	LOCK(&sdos_info->wilock);
	sdos_info->work_item_sched = FALSE;
	UNLOCK(&sdos_info->wilock);

	dbus_sdio_txq_process(sdos_info->cbarg);

	LOCK(&sdos_info->wilock);
	ASSERT(sdos_info->workitem_callbacks);
	sdos_info->workitem_callbacks--;
	UNLOCK(&sdos_info->wilock);
}

static void
dbus_sdos_queue_workitem(sdos_info_t *sdos_info, void *work_item, workitem_cb_t fn)
{
	LOCK(&sdos_info->wilock);
	if (sdos_info->work_item_sched) {
		UNLOCK(&sdos_info->wilock);
		return;
	}

	sdos_info->work_item_sched = TRUE;
	sdos_info->workitem_callbacks++;

	UNLOCK(&sdos_info->wilock);

#if (0>= 0x0600)
	NdisQueueIoWorkItem(*((PNDIS_HANDLE)work_item), fn, sdos_info);
#else
	NdisInitializeWorkItem((PNDIS_WORK_ITEM)work_item, fn, sdos_info);
	NdisScheduleWorkItem((PNDIS_WORK_ITEM)work_item);
#endif 
}

static int
dbus_sdos_init_workitem(sdos_info_t *sdos_info, void *work_item)
{
	int status = DBUS_OK;

#if (0>= 0x0600)
	NDIS_HANDLE wi = NdisAllocateIoWorkItem(sdos_info->AdapterHandle);
	if (wi != NULL)
		*((PNDIS_HANDLE)work_item) = wi;
	else
		status = DBUS_ERR;
#else
	UNREFERENCED_PARAMETER(sdos_info);
	UNREFERENCED_PARAMETER(work_item);
#endif 

	return status;
}

static void
dbus_sdos_free_work_item(sdos_info_t *sdos_info, void *work_item)
{
	ASSERT(sdos_info->workitem_callbacks == 0);
#if (0>= 0x0600)
	NdisFreeIoWorkItem(*((PNDIS_HANDLE)work_item));
#endif 
}
#endif /* !BCMDONGLEHOST */
