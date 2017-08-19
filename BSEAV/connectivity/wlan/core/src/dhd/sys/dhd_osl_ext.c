/*
 * Broadcom Dongle Host Driver (DHD), network interface. Port to the
 * OSL extension API.
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
#include "osl_ext.h"
#include "dhd_dbg.h"
#include "bcmsdh_generic.h"
#include "bcmutils.h"
#include "dngl_stats.h"
#include "dhd.h"
#include "dhd_bus.h"
#include "dhd_proto.h"
#include "wl_drv.h"
#include "bcmendian.h"
#include "epivers.h"
#include <proto/bcmevent.h>

#ifdef PROP_TXSTATUS
#include <wlfc_proto.h>
#include <dhd_wlfc.h>
#endif

/* ---- Public Variables ------------------------------------------------- */

/* Watchdog frequency */
uint dhd_watchdog_ms;

#ifdef DHD_DEBUG
/* Console poll interval */
uint dhd_console_ms;
#endif /* DHD_DEBUG */

uint dhd_slpauto = TRUE;

/* Idle timeout for backplane clock */
int dhd_idletime;

/* Use polling */
uint dhd_poll;

/* Use interrupts */
uint dhd_intr;

/* Assumed xtal frequency */
uint dhd_xtal_khz;

/* ARP offload agent mode */
uint dhd_arp_mode = 0;

/* ARP offload enable */
uint dhd_arp_enable = FALSE;

/* Pkt filte enable control */
uint dhd_pkt_filter_enable = FALSE;

/*  Pkt filter init setup */
uint dhd_pkt_filter_init = 1;

/* Pkt filter mode control */
uint dhd_master_mode = TRUE;

/* Contorl fw roaming */
uint dhd_roam_disable = 1;

/* Control radio state */
uint dhd_radio_up = 0;

/* SDIO Drive Strength (in milliamps) */
uint dhd_sdiod_drive_strength;



/* ---- Private Constants and Types -------------------------------------- */

#define ETHER_MTU 1518

#define DHD_TASK_STACK_SIZE_BYTES	(32*1024)
#define DHD_TASK_PRIORITY 		OSL_EXT_TASK_HIGH_NORMAL_PRIORITY

#define WDOG_TASK_STACK_SIZE_BYTES	(32*1024)
#define WDOG_TASK_PRIORITY 		OSL_EXT_TASK_NORMAL_PRIORITY


/* Version string to report */
#ifdef BCMDBG
#define DHD_COMPILED "\nCompiled in " SRCBASE
#else
#define DHD_COMPILED
#endif

#ifndef	SRCBASE
#define	SRCBASE "."
#endif /* SRCBASE */


/* Map between public WL driver handle and internal DHD info struct. */
#define WL_DRV_HDL_TO_DHD_INFO(hdl) ((struct dhd_info*) (hdl))
#define DHD_INFO_TO_WL_DRV_HDL(dhd) ((wl_drv_hdl) (dhd))

/* Maximum register netif callbacks */
#define MAX_NETIF_CALLBACK	3

/* Maximum register event callbacks */
#define MAX_EVENT_CALLBACK	3

/* Maximum register event packet callbacks */
#define MAX_EVENT_PACKET_CALLBACK	1

#define MAX_INTERFACES			3
#define MAX_IFNAME_SIZE			8

/* Interface control information */
typedef struct dhd_if {

	/* Back pointer to dhd_info. */
	struct dhd_info			*info;

	/* Inteface index assigned by dongle */
	int 				idx;

	/* Interface name. */
	char				name[MAX_IFNAME_SIZE];

	/* User registered callback for interfacing to network stack (e.g. TCP/IP stack). */
	wl_drv_netif_callbacks_t	netif_callbacks[MAX_NETIF_CALLBACK];

	/* User registered callback for posting events. */
	wl_drv_event_callback		event_callback[MAX_EVENT_CALLBACK];

	/* User registered callback for event packets. */
	wl_drv_event_packet_callback	event_packet_callback[MAX_EVENT_PACKET_CALLBACK];

	bool 				in_use;

} dhd_if_t;

/* Local private structure (extension of pub) */
typedef struct dhd_info {
	dhd_pub_t			pub;

	/* Watchdog timer */
	osl_ext_timer_t			wd_timer;

	/* Semaphore used to signal watchdog task to run. */
	osl_ext_sem_t			wd_task_signal;

	/* Watchdog task. */
	osl_ext_task_t			wd_task;

	bool				wd_timer_valid;

	/* IOCTL response wait semaphore */
	osl_ext_sem_t			ioctl_resp_wait;

#ifdef PROP_TXSTATUS
	/* Mutex for wl flow control */
	osl_ext_mutex_t			wlfc_mutex;
#endif

	/* Mutex for protocol critical sections. */
	osl_ext_mutex_t			proto_mutex;

	/* Semaphore used to signal DHD task to run. */
	osl_ext_sem_t			dhd_task_signal;

	/* DHD task used to send and receive packets. */
	osl_ext_task_t			dhd_task;

	/* Mutex for bus critical sections. */
	osl_ext_mutex_t			sd_mutex;

	/* Mutex for TX queue critical sections. */
	osl_ext_mutex_t			txq_mutex;

	/* Semaphore/flag used for ctrl event waits. */
	osl_ext_sem_t			ctrl_wait;
	bool				ctrl_waiting;
	bool				ctrl_notified;

	/* Flag used to signal DHD task to exit for driver de-initialization. */
	bool 				exit_dhd_task;

	/* Flag used to signal that DHD task has exited. */
	bool 				dhd_task_done;

	/* Flag used to signal watchdog task to exit for driver de-initialization. */
	bool 				exit_watchdog_task;

	/* Flag used to signal that watchdog task has exited. */
	bool 				watchdog_task_done;

	/* IOCTL response timeout */
	unsigned int			ioctl_timeout_msec;

	/* User registered callback for interfacing to file-system. */
	wl_drv_file_callbacks_t		file_callbacks;

	/* Pending 802.1X packets in TX queue. */
	volatile unsigned int 		pend_8021x_cnt;

	/* Interface control information */
	dhd_if_t 			iflist[MAX_INTERFACES];

} dhd_info_t;


/* ---- Private Variables ------------------------------------------------ */

/* Version string to report */
static const char dhd_version[] = "Dongle Host Driver, version " EPI_VERSION_STR
#ifdef BCMDBG
"\nCompiled in " SRCBASE " on " __DATE__ " at " __TIME__
#endif
;

static dhd_info_t *g_dhd_info;


/* ---- Private Function Prototypes -------------------------------------- */

static int dhd_open(dhd_info_t *dhd);
static int dhd_stop(dhd_info_t *dhd);
static void dhd_task(osl_ext_task_arg_t arg);
static void dhd_watchdog_task(osl_ext_task_arg_t arg);
static void dhd_watchdog_timer_callback(osl_ext_timer_arg_t arg);
static int dhd_wait_pend8021x(dhd_info_t *dhd);


/* ---- Functions -------------------------------------------------------- */

/* ----------------------------------------------------------------------- */
wl_drv_hdl
wl_drv_init(const char *fw_pathname, const char *nv_pathname,
            const char *nvram_params, wl_drv_file_callbacks_t *file_callbacks)
{
	int		ret = -1;
	struct dhd_bus	*bus;
	osl_t 		*osh;
	dhd_info_t 	*dhd;

	/* Init globals. */
	g_dhd_info = NULL;
	dhd_watchdog_ms = 50;
#ifdef DHD_DEBUG
	dhd_console_ms = 250;
#endif /* DHD_DEBUG */
	dhd_idletime = DHD_IDLETIME_TICKS;
	dhd_poll = FALSE;
	dhd_intr = TRUE;
	dhd_xtal_khz = 0;
	dhd_sdiod_drive_strength = 6;


	/* Display version information. */
	printf("%s\n", dhd_version);

	/* Ensure that one and only one of dhd_poll or dhd_intr is TRUE.
	* We must set SDIO for either polling mode or interrupt mode.
	* Not both, not none.
	*/
	ASSERT(dhd_poll != dhd_intr);


	/* Init operating system abstraction layer. */
	osh = osl_attach();
	if (osh == NULL)
		goto err;


	/* Init the bus driver sub-layer. */
	ret = dhd_bus_register();
	if (ret == -1)
		goto err;


	bus = bcmsdh_probe(osh);
	if (bus == NULL)
		goto err;

	/* 'nvram_params' and 'nv_path' are mutually exclusive. */
	if ((nvram_params != NULL) && (nv_pathname != NULL))
		goto err;


	/* Set nvram and firmware paths. */
	if (fw_pathname != NULL)
		strncpy(fw_path, fw_pathname, sizeof(fw_path));

	if (nv_pathname != NULL)
		strncpy(nv_path, nv_pathname, sizeof(nv_path));

	dhd_bus_set_nvram_params(bus, nvram_params);


	dhd = *(dhd_info_t **)bus;

	/* Save file-system access callback functions. */
	if (file_callbacks != NULL)
		dhd->file_callbacks = *file_callbacks;

	/* Start-up the driver... */
	if (dhd_open(dhd)) {
		DHD_ERROR(("%s: dhd_open failed\n", __FUNCTION__));
		goto err;
	}

	return (DHD_INFO_TO_WL_DRV_HDL(dhd));

err:
	return (NULL);
}


/* ----------------------------------------------------------------------- */
void
wl_drv_deinit(wl_drv_hdl hdl)
{
	osl_t *osh;
	struct dhd_info *dhd = WL_DRV_HDL_TO_DHD_INFO(hdl);

	if (dhd == NULL)
		dhd = g_dhd_info;

	dhd_stop(dhd);

	/* save osh as dhd will be free'ed */
	osh = dhd->pub.osh;

	bcmsdh_remove(osh, dhd->pub.bus);

	dhd_bus_unregister();

	osl_detach(osh);
}


/* ----------------------------------------------------------------------- */
void
wl_drv_register_event_callback_if(wl_drv_hdl hdl, unsigned int ifidx,
                                  wl_drv_event_callback cb)
{
	struct dhd_info *dhd = WL_DRV_HDL_TO_DHD_INFO(hdl);
	int i;

	if (ifidx >= MAX_INTERFACES) {
		return;
	}

	if (dhd == NULL)
		dhd = g_dhd_info;

	for (i = 0; i < MAX_EVENT_CALLBACK; i++) {
		if (dhd->iflist[ifidx].event_callback[i] == NULL) {
			dhd->iflist[ifidx].event_callback[i] = cb;
			break;
		}
	}
}


/* ----------------------------------------------------------------------- */
void
wl_drv_deregister_event_callback_if(wl_drv_hdl hdl, unsigned int ifidx,
                                    wl_drv_event_callback cb)
{
	struct dhd_info *dhd = WL_DRV_HDL_TO_DHD_INFO(hdl);
	int i;

	if (ifidx >= MAX_INTERFACES) {
		return;
	}

	if (dhd == NULL)
		dhd = g_dhd_info;

	for (i = 0; i < MAX_EVENT_CALLBACK; i++) {
		if (dhd->iflist[ifidx].event_callback[i] == cb) {
			dhd->iflist[ifidx].event_callback[i] = NULL;
			break;
		}
	}
}


/* ----------------------------------------------------------------------- */
void
wl_drv_register_event_packet_callback_if(wl_drv_hdl hdl, unsigned int ifidx,
	wl_drv_event_packet_callback cb)
{
	struct dhd_info *dhd = WL_DRV_HDL_TO_DHD_INFO(hdl);
	int i;

	if (ifidx >= MAX_INTERFACES) {
		return;
	}

	if (dhd == NULL)
		dhd = g_dhd_info;

	for (i = 0; i < MAX_EVENT_PACKET_CALLBACK; i++) {
		if (dhd->iflist[ifidx].event_packet_callback[i] == NULL) {
			dhd->iflist[ifidx].event_packet_callback[i] = cb;
			break;
		}
	}
}


/* ----------------------------------------------------------------------- */
void
wl_drv_deregister_event_packet_callback_if(wl_drv_hdl hdl, unsigned int ifidx,
	wl_drv_event_packet_callback cb)
{
	struct dhd_info *dhd = WL_DRV_HDL_TO_DHD_INFO(hdl);
	int i;

	if (ifidx >= MAX_INTERFACES) {
		return;
	}

	if (dhd == NULL)
		dhd = g_dhd_info;

	for (i = 0; i < MAX_EVENT_PACKET_CALLBACK; i++) {
		if (dhd->iflist[ifidx].event_packet_callback[i] == cb) {
			dhd->iflist[ifidx].event_packet_callback[i] = NULL;
			break;
		}
	}
}


/****************************************************************************
* Function:   dhd_open
*
* Purpose:    Start the driver.
*
* Parameters: dhd (mod) Pointer to DHD information structure.
*
* Returns:    0 on success, else error code.
*****************************************************************************
*/
static int
dhd_open(dhd_info_t *dhd)
{
	int ret;

	DHD_TRACE(("%s: Enter\n", __FUNCTION__));

	/* This must be set before the firmware download because it is used
	 * by the file-system access callbacks.
	 */
	g_dhd_info = dhd;

	/* Download image and nvram to the dongle. */
	if (dhd_bus_download_firmware(dhd->pub.bus, dhd->pub.osh, fw_path, nv_path) < 0) {
		DHD_ERROR(("%s: Firmware download failed.\n", __FUNCTION__));
		return -1;
	}


	/* Start the watchdog timer */
	dhd->pub.tickcnt = 0;
	dhd_os_wd_timer(dhd, dhd_watchdog_ms);

	/* Bring up the bus */
	if ((ret = dhd_bus_init(&dhd->pub, TRUE)) != 0)
	{
		DHD_ERROR(("ERROR: dhd_bus_init() failed!\n"));
		return ret;
	}

	/* If bus is not ready, can't come up */
	if (dhd->pub.busstate != DHD_BUS_DATA) {
		DHD_ERROR(("ERROR: wrong bus-state!\n"));
		osl_ext_timer_delete(&dhd->wd_timer);
		dhd->wd_timer_valid = FALSE;
		return -1;
	}

	/* Bus is ready, query any dongle information */
	dhd_sync_with_dongle(&dhd->pub);
	DHD_ERROR(("Dongles MAC address = %02X:%02X:%02X:%02X:%02X:%02X\n",
		dhd->pub.mac.octet[0], dhd->pub.mac.octet[1], dhd->pub.mac.octet[2],
		dhd->pub.mac.octet[3], dhd->pub.mac.octet[4], dhd->pub.mac.octet[5]));

	/* Allow transmit calls */
	dhd->pub.up = 1;

	return 0;
}


/****************************************************************************
* Function:   dhd_stop
*
* Purpose:    Stop the driver.
*
* Parameters: dhd (mod) Pointer to DHD information structure.
*
* Returns:    0 on success, else error code.
*****************************************************************************
*/
static int
dhd_stop(dhd_info_t *dhd)
{
	unsigned int 			ifidx;
	wl_drv_netif_stop_queue		stop_queue_cb;

	DHD_TRACE(("%s: Enter\n", __FUNCTION__));

#ifdef PROP_TXSTATUS
	dhd_wlfc_cleanup(&dhd->pub, NULL, 0);
#endif

	/* Set state and stop OS transmissions */
	dhd->pub.up = 0;

	/* Stop the network interface transmit queue. */
	for (ifidx = 0; ifidx < MAX_INTERFACES; ifidx++) {
		int i;
		for (i = 0; i < MAX_NETIF_CALLBACK; i++) {
			stop_queue_cb = dhd->iflist[ifidx].netif_callbacks[i].stop_queue;
			if (stop_queue_cb != NULL)
				stop_queue_cb();
		}
	}


	/* Stop the protocol module */
	dhd_prot_stop(&dhd->pub);

	/* Stop the bus module */
	dhd_bus_stop(dhd->pub.bus, TRUE);

	if (dhd->wd_timer_valid == TRUE) {
		osl_ext_timer_delete(&dhd->wd_timer);
		dhd->wd_timer_valid = FALSE;
	}

	return 0;
}


/* ----------------------------------------------------------------------- */
osl_t *
dhd_osl_attach(void *pdev, uint32 bustype, uint32 bus_num, uint32 slot_num)
{
	/* Nothing to do - OS layer already created in wl_drv_init(). */
	return (NULL);
}

/* ----------------------------------------------------------------------- */
void
dhd_osl_detach(osl_t *osh)
{
	/* Nothing to do. */
}

/*
 * Generalized timeout mechanism.  Uses spin sleep with exponential back-off.  Usage:
 *
 *      dhd_timeout_start(&tmo, usec);
 *      while (!dhd_timeout_expired(&tmo))
 *              if (poll_something())
 *                      break;
 *      if (dhd_timeout_expired(&tmo))
 *              fatal();
 */

void
dhd_timeout_start(dhd_timeout_t *tmo, uint usec)
{
	tmo->limit = usec;
	tmo->increment = 0;
	tmo->elapsed = 0;
	tmo->tick = 10000;
}

int
dhd_timeout_expired(dhd_timeout_t *tmo)
{
	/* Does nothing the first call */
	if (tmo->increment == 0) {
		tmo->increment = 1;
		return 0;
	}

	if (tmo->elapsed >= tmo->limit)
		return 1;

	/* Add the delay that's about to take place */
	tmo->elapsed += tmo->increment;


	OSL_DELAY(tmo->increment);
	tmo->increment *= 2;
	if (tmo->increment > tmo->tick)
		tmo->increment = tmo->tick;

	return 0;
}

/* ----------------------------------------------------------------------- */
char *
dhd_ifname(dhd_pub_t *dhdp, int ifidx)
{
	dhd_info_t *dhd = (dhd_info_t *)dhdp->info;

	ASSERT(dhd != NULL);

	if (ifidx < 0 || ifidx >= MAX_INTERFACES) {
		DHD_ERROR(("%s: ifidx %d out of range\n", __FUNCTION__, ifidx));
		return "<if_bad>";
	}

	if (dhd->iflist[ifidx].in_use == FALSE) {
		DHD_ERROR(("%s: null i/f %d\n", __FUNCTION__, ifidx));
		return "<if_null>";
	}

	return (dhd->iflist[ifidx].name);
}


/* ----------------------------------------------------------------------- */
int
dhd_ifname2idx(dhd_info_t *dhd, char *name)
{
	int i = MAX_INTERFACES;

	if (dhd == NULL)
		dhd = g_dhd_info;

	ASSERT(dhd != NULL);

	if ((name == NULL) || name[0] == '\0') {
		return (0);
	}

	while (--i > 0) {
		if (dhd->iflist[i].in_use &&
		    !strncmp(dhd->iflist[i].name, name, MAX_IFNAME_SIZE)) {
				break;
		}
	}

	DHD_TRACE(("%s: return idx %d for \"%s\"\n", __FUNCTION__, i, name));

	return i;	/* default - the primary interface */
}

/* ----------------------------------------------------------------------- */
int
dhd_event_ifadd(dhd_info_t *dhd, wl_event_data_if_t *ifevent, char *name, uint8 *mac)
{
	dhd_if_t 	*ifp;
	dhd_if_t	backup_if;
	int 		ifidx;

	ifidx = ifevent->ifidx;
	DHD_TRACE(("%s: idx %d, handle->%p\n", __FUNCTION__, ifidx, handle));

	ASSERT((dhd != NULL) && (ifidx < MAX_INTERFACES));

	ifp = &dhd->iflist[ifidx];

	/* Make a backup of the interface, some callback may be registered prior
	 * to creation.
	 */
	backup_if = *ifp;

	memset(ifp, 0, sizeof(dhd_if_t));
	ifp->info = dhd;
	ifp->idx = ifidx;
	strncpy(ifp->name, name, MAX_IFNAME_SIZE);
	ifp->name[MAX_IFNAME_SIZE-1] = '\0';

	/* Restore callbacks. */
	memcpy(ifp->event_callback, backup_if.event_callback,
	       sizeof(backup_if.event_callback));
	memcpy(ifp->event_packet_callback, backup_if.event_packet_callback,
	       sizeof(backup_if.event_packet_callback));
	memcpy(ifp->netif_callbacks, backup_if.netif_callbacks,
	       sizeof(backup_if.netif_callbacks));
	ifp->bta_callbacks = backup_if.bta_callbacks;

	ifp->in_use = TRUE;

	return (0);
}

/* ----------------------------------------------------------------------- */
void
dhd_event_ifdel(dhd_info_t *dhd, wl_event_data_if_t *ifevent)
{
	dhd_if_t *ifp;
	int 		ifidx;

	ifidx = ifevent->ifidx;
	DHD_TRACE(("%s: idx %d\n", __FUNCTION__, ifidx));

	ASSERT((dhd != NULL) && (ifidx != 0) && (ifidx < MAX_INTERFACES));
	ifp = &dhd->iflist[ifidx];
	memset(ifp, 0, sizeof(dhd_if_t));
}

/* ----------------------------------------------------------------------- */

void
dhd_event(struct dhd_info *dhd, char *evpkt, int evlen, int ifidx)
{
	/*
	 * Not supported
	 */
	return;
}

dhd_pub_t *
dhd_attach(osl_t *osh, struct dhd_bus *bus, uint bus_hdrlen)
{
	dhd_info_t *dhd = NULL;
	osl_ext_status_t status;

	DHD_TRACE(("%s: Enter\n", __FUNCTION__));

	if (!(dhd = MALLOC(osh, sizeof(dhd_info_t)))) {
		DHD_ERROR(("malloc of dhd_info_t failed!\n"));
		goto fail;
	}

	memset(dhd, 0, sizeof(dhd_info_t));
	dhd->pub.osh = osh;

	/* Link to info module */
	dhd->pub.info = dhd;

	/* Link to bus module */
	dhd->pub.bus = bus;
	dhd->pub.hdrlen = bus_hdrlen;

	dhd->ioctl_timeout_msec = IOCTL_RESP_TIMEOUT;

#ifdef PROP_TXSTATUS
	status = osl_ext_mutex_create("dhdwlfc", &dhd->wlfc_mutex);
	ASSERT(status == OSL_EXT_SUCCESS);
#endif

	status = osl_ext_mutex_create("dhdprot", &dhd->proto_mutex);
	ASSERT(status == OSL_EXT_SUCCESS);

	status = osl_ext_mutex_create("dhdsd", &dhd->sd_mutex);
	ASSERT(status == OSL_EXT_SUCCESS);

	status = osl_ext_mutex_create("dhdtxq", &dhd->txq_mutex);
	ASSERT(status == OSL_EXT_SUCCESS);

	status = osl_ext_sem_create("dhdioc", 0, &dhd->ioctl_resp_wait);
	ASSERT(status == OSL_EXT_SUCCESS);

	status = osl_ext_sem_create("dhdtask", 0, &dhd->dhd_task_signal);
	ASSERT(status == OSL_EXT_SUCCESS);

	status = osl_ext_sem_create("wdtask", 0, &dhd->wd_task_signal);
	ASSERT(status == OSL_EXT_SUCCESS);

	status = osl_ext_sem_create("dhdctrl", 0, &dhd->ctrl_wait);
	ASSERT(status == OSL_EXT_SUCCESS);

	status = osl_ext_task_create("dhd",
	                         DHD_TASK_STACK_SIZE_BYTES,
	                         DHD_TASK_PRIORITY,
	                         dhd_task,
	                         (osl_ext_task_arg_t) dhd,
	                         &dhd->dhd_task);
	ASSERT(status == OSL_EXT_SUCCESS);

	status = osl_ext_task_create("wdog",
	                         WDOG_TASK_STACK_SIZE_BYTES,
	                         WDOG_TASK_PRIORITY,
	                         dhd_watchdog_task,
	                         (osl_ext_task_arg_t) dhd,
	                         &dhd->wd_task);
	ASSERT(status == OSL_EXT_SUCCESS);


	/* Attach and link in the protocol */
	if (dhd_prot_attach(&dhd->pub) != 0) {
		DHD_ERROR(("dhd_prot_attach failed\n"));
		goto fail;
	}

	PKTSETHEADROOM(osh, TRUE, bus_hdrlen);

	return (&dhd->pub);
fail:
	if (dhd)
	{
		dhd_detach(&(dhd->pub));
		dhd_free(&(dhd->pub));
	}

	return NULL;
}


/* ----------------------------------------------------------------------- */
void
dhd_detach(dhd_pub_t *dhdp)
{
	dhd_info_t *dhd = (dhd_info_t *)dhdp->info;

	DHD_TRACE(("%s: Enter\n", __FUNCTION__));

	/* Signal tasks to stop running, so that they are in
	 * a known state and do not have a resources allocated, before
	 * deleting them.
	 */
	dhd->exit_dhd_task = TRUE;
	osl_ext_sem_give(&dhd->dhd_task_signal);
	while (!dhd->dhd_task_done) {
		/* 100ms */
		osl_delay(1000 * 100);
	}

	dhd->exit_watchdog_task = TRUE;
	osl_ext_sem_give(&dhd->wd_task_signal);
	while (!dhd->watchdog_task_done) {
		/* 100ms */
		osl_delay(1000 * 100);
	}


	/* Delete operating system resources. */
	osl_ext_task_delete(&dhd->wd_task);
	osl_ext_task_delete(&dhd->dhd_task);

	osl_ext_sem_delete(&dhd->wd_task_signal);
	osl_ext_sem_delete(&dhd->dhd_task_signal);
	osl_ext_sem_delete(&dhd->ioctl_resp_wait);
	osl_ext_sem_delete(&dhd->ctrl_wait);

	osl_ext_mutex_delete(&dhd->txq_mutex);
	osl_ext_mutex_delete(&dhd->sd_mutex);
	osl_ext_mutex_delete(&dhd->proto_mutex);

#ifdef PROP_TXSTATUS
	osl_ext_mutex_delete(&dhd->wlfc_mutex);
#endif

	if (dhdp->prot)
		dhd_prot_detach(&dhd->pub);
}


void
dhd_free(dhd_pub_t *dhdp)
{
	dhd_info_t *dhd = (dhd_info_t *)dhdp->info;
	DHD_TRACE(("%s: Enter\n", __FUNCTION__));
#ifdef CACHE_FW_IMAGES
	if (dhdp->cached_fw) {
		MFREE(dhdp->osh, dhdp->cached_fw, dhdp->bus->ramsize);
		dhdp->cached_fw = NULL;
	}

	if (dhdp->cached_nvram) {
		MFREE(dhdp->osh, dhdp->cached_nvram, MAX_NVRAMBUF_SIZE);
		dhdp->cached_nvram = NULL;
	}
#endif
	if (dhdp->osh)
		MFREE(dhd->pub.osh, dhd, sizeof(dhd_info_t));
}

/* ----------------------------------------------------------------------- */
int
dhd_register_if(dhd_pub_t *dhdp, int ifidx)
{
	dhdp->rxsz = ETHER_MTU + 14 + dhdp->hdrlen;

	return 0;
}


/* ----------------------------------------------------------------------- */
int
dhd_bus_start(dhd_pub_t *dhdp)
{
	/* Nothing to do */
	return 0;
}

int
wl_drv_ioctl(wl_drv_hdl hdl, wl_drv_ioctl_t *ioc)
{
	int 		bcmerror = BCME_ERROR;
	int 		len = 0;
	void 		*buf = NULL;
	dhd_ioctl_t	*dhd_ioc;
	wl_ioctl_t 	*wl_ioc;
	struct dhd_info *dhd = WL_DRV_HDL_TO_DHD_INFO(hdl);
	bool		is_set_key_cmd;

	DHD_TRACE(("%s: Enter\n", __FUNCTION__));

	if (dhd == NULL)
		dhd = g_dhd_info;

	if ((dhd == NULL) || (ioc == NULL)) {
		bcmerror = BCME_BADARG;
		goto done;
	}


	/* Check for local dhd ioctl and handle it */
	if (ioc->d.driver == DHD_IOCTL_MAGIC) {
		dhd_ioc = &ioc->d;
		if (dhd_ioc->buf) {
			len = MIN(dhd_ioc->len, DHD_IOCTL_MAXLEN);
			buf = dhd_ioc->buf;
		}

		bcmerror = dhd_ioctl(&dhd->pub, dhd_ioc, buf, len);
		goto done;
	}
	else {
		wl_ioc = &ioc->w;
		if (wl_ioc->buf) {
			len = MIN(wl_ioc->len, WLC_IOCTL_MAXLEN);
			buf = wl_ioc->buf;
		}
	}


	/* State check before sending to dongle (must be up, and wl) */
	if (!dhd->pub.up || (dhd->pub.busstate != DHD_BUS_DATA)) {
		DHD_TRACE(("DONGLE_DOWN\n"));
		bcmerror = BCME_DONGLE_DOWN;
		goto done;
	}

	if (!dhd->pub.iswl) {
		bcmerror = BCME_DONGLE_DOWN;
		goto done;
	}


	/* Intercept WLC_SET_KEY IOCTL - serialize M4 send and set key IOCTL to
	 * prevent M4 encryption.
	 */
	is_set_key_cmd = ((ioc->w.cmd == WLC_SET_KEY) ||
	                 ((ioc->w.cmd == WLC_SET_VAR) &&
	                        !(strncmp("wsec_key", ioc->w.buf, 9))) ||
	                 ((ioc->w.cmd == WLC_SET_VAR) &&
	                        !(strncmp("bsscfg:wsec_key", ioc->w.buf, 15))));

	if (is_set_key_cmd) {
		dhd_wait_pend8021x(dhd);
	}


	/* Send to dongle... */
	bcmerror = dhd_wl_ioctl(&dhd->pub, ioc->ifidx, wl_ioc, buf, len);

done:
	if ((bcmerror != 0) && (dhd != NULL)) {
		dhd->pub.bcmerror = bcmerror;
	}

	return OSL_ERROR(bcmerror);
}


/* ----------------------------------------------------------------------- */
void
dhd_sched_dpc(dhd_pub_t *dhdp)
{
	/* Kick the main DHD task. */
	osl_ext_sem_give(&dhdp->info->dhd_task_signal);
}


/****************************************************************************
* Function:   dhd_task
*
* Purpose:    DHD task to handle rx/tx of packets from/to dongle.
*
* Parameters: arg (in) Pointer to 'dhd_info_t' struct.
*
* Returns:    None.
*****************************************************************************
*/
static void
dhd_task(osl_ext_task_arg_t arg)
{
	dhd_info_t *dhd = (dhd_info_t *)arg;

	dhd->exit_dhd_task = FALSE;
	dhd->dhd_task_done = FALSE;

	/* Run until signal received */
	while (1) {
		if (osl_ext_sem_take(&dhd->dhd_task_signal,
		                     OSL_EXT_TIME_FOREVER) == OSL_EXT_SUCCESS) {

			/* Check if task has been signaled to exit. */
			if (dhd->exit_dhd_task)
				break;

			/* Call bus dpc unless it indicated down (then clean stop) */
			if (dhd->pub.busstate != DHD_BUS_DOWN) {
				if (dhd_bus_dpc(dhd->pub.bus))
					osl_ext_sem_give(&dhd->dhd_task_signal);
			} else {
				dhd_bus_stop(dhd->pub.bus, TRUE);
			}
		}
		else
			DHD_ERROR(("%s: sem take error!\n", __FUNCTION__));
	}

	DHD_ERROR(("%s: Exiting task!!\n", __FUNCTION__));
	dhd->dhd_task_done = TRUE;
}


/****************************************************************************
* Function:   dhd_watchdog_timer_callback
*
* Purpose:    Callback function invoked periodically by wdog timer.
*
* Parameters: arg (in) Pointer to 'dhd_info_t' struct.
*
* Returns:    None.
*****************************************************************************
*/
static void
dhd_watchdog_timer_callback(osl_ext_timer_arg_t arg)
{
	dhd_info_t *dhd;

	ASSERT(arg);
	dhd = (dhd_info_t *)arg;

	/* Kick the watchdog task. */
	osl_ext_sem_give(&dhd->wd_task_signal);
}


/****************************************************************************
* Function:   dhd_watchdog_task
*
* Purpose:    Task that runs periodically for performing housekeeping tasks such
*             as determining when the bus is idle "long enough" to justify turning
*             off the chip PLL..
*
* Parameters: arge (in) Pointer to 'dhd_info_t' struct.
*
* Returns:    None.
*****************************************************************************
*/
static void
dhd_watchdog_task(osl_ext_task_arg_t arg)
{
	dhd_info_t *dhd;

	dhd = (dhd_info_t *)arg;

	dhd->exit_watchdog_task = FALSE;
	dhd->watchdog_task_done = FALSE;

	while (1) {
		if (osl_ext_sem_take(&dhd->wd_task_signal,
		                     OSL_EXT_TIME_FOREVER) == OSL_EXT_SUCCESS) {

			/* Check if task has been signaled to exit. */
			if (dhd->exit_watchdog_task)
				break;

			/* Call the bus module watchdog */
			dhd_bus_watchdog(&dhd->pub);

			/* Count the tick for reference */
			dhd->pub.tickcnt++;
		}
		else
			DHD_ERROR(("%s: sem take error!!\n", __FUNCTION__));
	}

	DHD_ERROR(("%s: Exiting task!!\n", __FUNCTION__));
	dhd->watchdog_task_done = TRUE;
}


/* ----------------------------------------------------------------------- */
void
dhd_txflowcontrol(dhd_pub_t *dhdp, int ifidx, bool state)
{
	dhd_info_t 			*dhd;
	wl_drv_netif_callbacks_t	*cb;
	int i;

	dhd = dhdp->info;

	DHD_TRACE(("%s: Enter\n", __FUNCTION__));

	dhdp->txoff = state;

	for (i = 0; i < MAX_NETIF_CALLBACK; i++) {
		cb = &dhd->iflist[ifidx].netif_callbacks[i];
		if (state == ON) {
			/* Notify network interface stack (TCP/IP stack) to stop transmitting
			 * packets to the driver and queue them in the stack.
			 */
			if (cb->stop_queue != NULL)
				cb->stop_queue();
		}
		else {
			/* Notify network interface stack (TCP/IP stack) to start transmitting
			 * packets to the driver again.
			 */
			if (cb->start_queue != NULL)
				cb->start_queue();
	}
}
}


/* ----------------------------------------------------------------------- */
void
dhd_rx_frame(dhd_pub_t *dhdp, int ifidx, void *pktbuf, int numpkt, uint8 chan)
{
	uchar 				*ptr;
	uint16 				type;
	wl_event_msg_t			event;
	void 				*data;
	void				*pnext;
	uint 				len;
	int 				i;
	void 				*netif_pkt;
	osl_t				*osh;
	dhd_info_t			*dhd;
	int				bcmerror;
	int				j;
	bool				pkt_consumed;
	wl_drv_netif_rx_pkt		rx_pkt_cb;
	wl_drv_event_callback		evt_cb;
	wl_drv_event_packet_callback	evt_pkt_cb;
	amp_hci_ACL_data_t 		*ACL_data = NULL;

	DHD_TRACE(("%s: Enter\n", __FUNCTION__));

	osh = dhdp->osh;
	dhd = dhdp->info;

	for (i = 0; pktbuf && i < numpkt; i++, pktbuf = pnext) {

		pnext = PKTNEXT(osh, pktbuf);
		PKTSETNEXT(osh, pktbuf, NULL);

		ptr = PKTDATA(osh, pktbuf);
		type  = *(uint16 *)(ptr + ETHER_TYPE_OFFSET);
		len = PKTLEN(osh, pktbuf);

#ifdef PROP_TXSTATUS
		if (dhd_wlfc_is_header_only_pkt(dhdp, pktbuf)) {
			/* WLFC may send header only packet when
			there is an urgent message but no packet to
			piggy-back on
			*/
			PKTFREE(dhdp->osh, pktbuf, TRUE);
			continue;
		}
#endif

		/* Process special event packets and then discard them */
		if (ntoh16(type) == ETHER_TYPE_BRCM) {
			ifidx = 0;		/* single interface for now */
			bcmerror = wl_host_event(dhdp, &ifidx, ptr, 0xFFFF, &event, &data);
			if (bcmerror == BCME_OK) {
				/* Event packet callback */
				for (j = 0; j < MAX_EVENT_PACKET_CALLBACK; j++) {
					evt_pkt_cb = dhd->iflist[ifidx].event_packet_callback[j];
					if (evt_pkt_cb != NULL) {
						evt_pkt_cb((bcm_event_t *)ptr);
					}
				}

				/* Post event to application. */
				wl_event_to_host_order(&event);
				for (j = 0; j < MAX_EVENT_CALLBACK; j++) {
					evt_cb = dhd->iflist[ifidx].event_callback[j];
					if (evt_cb != NULL) {
						evt_cb(&event, data);
					}
				}
			}
			PKTFREE(osh, pktbuf, FALSE);
			continue;
		}

		/* Real rx-frame (non-event) received; convert from driver
		 * packet to network interface stack packet.
		 */
		netif_pkt = PKTTONATIVE(osh, pktbuf);

		dhdp->dstats.rx_bytes += len;
		dhdp->rx_packets++; /* Local count */

		/* Send packet up to network interface. */
		pkt_consumed = FALSE;

		/* Regular network packet. */
		for (i = 0; i < MAX_NETIF_CALLBACK; i++) {
			rx_pkt_cb = dhd->iflist[ifidx].netif_callbacks[i].rx_pkt;
			if (rx_pkt_cb != NULL)
				if (rx_pkt_cb(netif_pkt, len) == 0) {
					pkt_consumed = TRUE;
					break;
				}
		}

		/* Free the driver packet. */
		PKTFREE(osh, pktbuf, FALSE);

		/* Free the network interface (native) packet if it wasn't consumed. */
		if (!pkt_consumed)
			pkt_free_native(osh, netif_pkt);
	}
}

/* ----------------------------------------------------------------------- */
wl_drv_hdl wl_drv_get_handle(void)
{
	return (DHD_INFO_TO_WL_DRV_HDL(g_dhd_info));
}

/* ----------------------------------------------------------------------- */
int
wl_drv_tx_pkt_if(wl_drv_hdl hdl, unsigned int ifidx, wl_drv_netif_pkt netif_pkt,
	unsigned int len)
{
	void	*pktbuf;
	struct dhd_info *dhd = WL_DRV_HDL_TO_DHD_INFO(hdl);

	DHD_TRACE(("%s: Enter\n", __FUNCTION__));

	if (dhd == NULL)
		dhd = g_dhd_info;

	/* Convert network interface packet to driver packet */
	if (!(pktbuf = PKTFRMNATIVE(dhd->pub.osh, netif_pkt, len))) {
		DHD_ERROR(("%s: PKTFRMNATIVE failed\n", dhd_ifname(&dhd->pub, 0)));
		dhd->pub.dstats.tx_dropped++;
		return (-1);
	}

	return (dhd_sendpkt(&dhd->pub, ifidx, pktbuf));
}

#ifdef PROP_TXSTATUS
int
dhd_os_wlfc_block(dhd_pub_t *pub)
{
	dhd_info_t *dhd = (dhd_info_t *)(pub->info);
	ASSERT(dhd != NULL);
	osl_ext_mutex_acquire(&dhd->wlfc_mutex, OSL_EXT_TIME_FOREVER);
	return 1;
}

int
dhd_os_wlfc_unblock(dhd_pub_t *pub)
{
	dhd_info_t *dhd = (dhd_info_t *)(pub->info);
	ASSERT(dhd != NULL);
	osl_ext_mutex_release(&dhd->wlfc_mutex);
	return 1;
}

const uint8 wme_fifo2ac[] = { 0, 1, 2, 3, 1, 1 };
const uint8 prio2fifo[8] = { 1, 0, 0, 1, 2, 2, 3, 3 };
#define WME_PRIO2AC(prio)	wme_fifo2ac[prio2fifo[(prio)]]

#endif /* PROP_TXSTATUS */

/* ----------------------------------------------------------------------- */
int
dhd_sendpkt(dhd_pub_t *dhdp, int ifidx, void *pktbuf)
{
	int ret;
	dhd_info_t *dhd = (dhd_info_t *)(dhdp->info);
	struct ether_header *eh = NULL;

	/* Reject if down */
	if (!dhdp->up || (dhdp->busstate == DHD_BUS_DOWN)) {
		ret = BCME_NOTUP;
		goto done;
	}


	/* Track pending 802.1X packet transmits in order to be able to serialize
	 * M4 send and set key IOCTL to prevent M4 encryption.
	 */
	if (PKTLEN(dhd->pub.osh, pktbuf) >= ETHER_ADDR_LEN) {
		uint8 *pktdata = (uint8 *)PKTDATA(dhd->pub.osh, pktbuf);
		eh = (struct ether_header *)pktdata;

		if (ntoh16(eh->ether_type) == ETHER_TYPE_802_1X) {
			dhd->pend_8021x_cnt++;
		}
	}

	/* Look into the packet and update the packet priority */
	pktsetprio(pktbuf, FALSE);

#ifdef PROP_TXSTATUS
	if (dhd_wlfc_is_supported(dhdp)) {
		/* store the interface ID */
		DHD_PKTTAG_SETIF(PKTTAG(pktbuf), ifidx);

		/* store destination MAC in the tag as well */
		DHD_PKTTAG_SETDSTN(PKTTAG(pktbuf), eh->ether_dhost);

		/* decide which FIFO this packet belongs to */
		if (ETHER_ISMULTI(eh->ether_dhost))
			/* one additional queue index (highest AC + 1) is used for bc/mc queue */
			DHD_PKTTAG_SETFIFO(PKTTAG(pktbuf), AC_COUNT);
		else
			DHD_PKTTAG_SETFIFO(PKTTAG(pktbuf), WME_PRIO2AC(PKTPRIO(pktbuf)));
	} else
#endif /* PROP_TXSTATUS */
	/* If the protocol uses a data header, apply it */
	dhd_prot_hdrpush(dhdp, ifidx, pktbuf);

#ifdef PROP_TXSTATUS
	if (dhd_wlfc_commit_packets(dhdp, (f_commitpkt_t)dhd_bus_txdata,
		dhdp->bus, pktbuf, TRUE) == WLFC_UNSUPPORTED) {
		/* non-proptxstatus way */
		ret = dhd_bus_txdata(dhdp->bus, pktbuf);
	}
#else
	/* Use bus module to send data frame */
	ret = dhd_bus_txdata(dhdp->bus, pktbuf);
#endif /* PROP_TXSTATUS */

done:
	if (ret)
		dhd->pub.dstats.tx_dropped++;
	else
		dhd->pub.tx_packets++;

	return ret;
}


/* ----------------------------------------------------------------------- */
void
wl_drv_register_netif_callbacks_if(wl_drv_hdl hdl, unsigned int ifidx,
                                   wl_drv_netif_callbacks_t *callbacks)
{
	struct dhd_info *dhd = WL_DRV_HDL_TO_DHD_INFO(hdl);
	int i;

	if (ifidx >= MAX_INTERFACES) {
		return;
	}

	if (dhd == NULL)
		dhd = g_dhd_info;

	for (i = 0; i < MAX_NETIF_CALLBACK; i++) {
		if (dhd->iflist[ifidx].netif_callbacks[i].rx_pkt == NULL) {
			dhd->iflist[ifidx].netif_callbacks[i] = *callbacks;
			break;
		}
	}

	/* Allow transmit calls */
	if (callbacks->start_queue != NULL)
		callbacks->start_queue();
}

/* ----------------------------------------------------------------------- */
void
wl_drv_deregister_netif_callbacks_if(wl_drv_hdl hdl, unsigned int ifidx,
	wl_drv_netif_callbacks_t *callbacks)
{
	struct dhd_info *dhd = WL_DRV_HDL_TO_DHD_INFO(hdl);
	int i;

	if (ifidx >= MAX_INTERFACES) {
		return;
	}

	if (dhd == NULL)
		dhd = g_dhd_info;

	for (i = 0; i < MAX_NETIF_CALLBACK; i++) {
		if (dhd->iflist[ifidx].netif_callbacks[i].rx_pkt ==
			callbacks->rx_pkt) {
			if (dhd->iflist[ifidx].netif_callbacks[i].stop_queue != NULL)
				dhd->iflist[ifidx].netif_callbacks[i].stop_queue();
			memset(&dhd->iflist[ifidx].netif_callbacks[i],
				0, sizeof(wl_drv_netif_callbacks_t));
			break;
		}
	}
}

/* ----------------------------------------------------------------------- */
void
dhd_txcomplete(dhd_pub_t *dhdp, void *txp, bool success)
{
	int ifidx;
	dhd_info_t *dhd = (dhd_info_t *)(dhdp->info);
	struct ether_header *eh;
	uint16 type;

	dhd_prot_hdrpull(dhdp, &ifidx, txp, NULL, NULL);

	eh = (struct ether_header *)PKTDATA(dhdp->osh, txp);
	type  = ntoh16(eh->ether_type);


	/* Track pending 802.1X packet transmits in order to be able to serialize
	 * M4 send and set key IOCTL to prevent M4 encryption.
	 */
	if (type == ETHER_TYPE_802_1X) {
		dhd->pend_8021x_cnt--;
	}

}

/* ----------------------------------------------------------------------- */
void
dhd_os_sdlock(dhd_pub_t * pub)
{
	dhd_info_t *dhd = pub->info;

	osl_ext_mutex_acquire(&dhd->sd_mutex, OSL_EXT_TIME_FOREVER);
}

/* ----------------------------------------------------------------------- */
void
dhd_os_sdunlock(dhd_pub_t * pub)
{
	dhd_info_t *dhd = pub->info;

	osl_ext_mutex_release(&dhd->sd_mutex);
}


/* ----------------------------------------------------------------------- */
void
dhd_os_sdlock_txq(dhd_pub_t * pub)
{
	dhd_info_t *dhd = pub->info;

	osl_ext_mutex_acquire(&dhd->txq_mutex, OSL_EXT_TIME_FOREVER);
}


/* ----------------------------------------------------------------------- */
void
dhd_os_sdunlock_txq(dhd_pub_t * pub)
{
	dhd_info_t *dhd = pub->info;

	osl_ext_mutex_release(&dhd->txq_mutex);
}


/* ----------------------------------------------------------------------- */
void
dhd_os_sdlock_rxq(dhd_pub_t * pub)
{
	/* This lock doesn't appear to be required. All calls to this function
	** appear to be called within the context of the same task.
	*/
}


/* ----------------------------------------------------------------------- */
void
dhd_os_sdunlock_rxq(dhd_pub_t * pub)
{
	/* This lock doesn't appear to be required. All calls to this function
	** appear to be called within the context of the same task.
	*/
}

/* ----------------------------------------------------------------------- */
void
dhd_os_wd_timer(void *bus, uint timeout_msec)
{
	dhd_info_t *dhd = bus;
	osl_ext_status_t status;

	/* don't start the wd until fw is loaded */
	if (dhd->pub.busstate == DHD_BUS_DOWN)
		return;

	if (dhd->wd_timer_valid == TRUE) {
		osl_ext_timer_delete(&dhd->wd_timer);
		dhd->wd_timer_valid = FALSE;
	}

	if (timeout_msec != 0) {
		status = osl_ext_timer_create("dhdwdog",
		                          timeout_msec,
		                          OSL_EXT_TIMER_MODE_REPEAT,
		                          dhd_watchdog_timer_callback,
		                          dhd,
		                          &dhd->wd_timer);

		if (status == OSL_EXT_SUCCESS) {
			dhd->wd_timer_valid = TRUE;
		}
		else {
			DHD_ERROR(("%s: create timer failed, status=%u\n",
			           __FUNCTION__, status));
		}
	}
}


/* ----------------------------------------------------------------------- */
void *
dhd_os_open_image(char * filename)
{
	wl_drv_file_hdl file_hdl = NULL;

	if (g_dhd_info->file_callbacks.open != NULL)
		file_hdl = g_dhd_info->file_callbacks.open(filename);

	return (file_hdl);
}


/* ----------------------------------------------------------------------- */
int
dhd_os_get_image_block(char * buf, int len, void * image)
{
	int bytes_read = 0;
	wl_drv_file_hdl file_hdl = (wl_drv_file_hdl) image;

	if (g_dhd_info->file_callbacks.read != NULL)
		bytes_read = g_dhd_info->file_callbacks.read(buf, len, file_hdl);

	return (bytes_read);
}


/* ----------------------------------------------------------------------- */
void
dhd_os_close_image(void * image)
{
	wl_drv_file_hdl file_hdl = (wl_drv_file_hdl) image;

	if (g_dhd_info->file_callbacks.close != NULL)
		g_dhd_info->file_callbacks.close(file_hdl);
}


/* ----------------------------------------------------------------------- */
int
dhd_os_proto_block(dhd_pub_t * pub)
{
	dhd_info_t *dhd = pub->info;
	if (osl_ext_mutex_acquire(&dhd->proto_mutex, OSL_EXT_TIME_FOREVER) == OSL_EXT_SUCCESS)
		return (1);
	else
		return (0);
}


/* ----------------------------------------------------------------------- */
int
dhd_os_proto_unblock(dhd_pub_t * pub)
{
	dhd_info_t *dhd = pub->info;
	if (osl_ext_mutex_release(&dhd->proto_mutex) == OSL_EXT_SUCCESS)
		return (1);
	else
		return (0);
}

/* ----------------------------------------------------------------------- */
void
dhd_os_dhdiovar_lock(dhd_pub_t *pub)
{
	return;
}

/* ----------------------------------------------------------------------- */
void
dhd_os_dhdiovar_unlock(dhd_pub_t *pub)
{
	return;
}

/* ----------------------------------------------------------------------- */
unsigned int
dhd_os_get_ioctl_resp_timeout(void)
{
	return (g_dhd_info->ioctl_timeout_msec);
}


/* ----------------------------------------------------------------------- */
void
dhd_os_set_ioctl_resp_timeout(unsigned int timeout_msec)
{
	g_dhd_info->ioctl_timeout_msec = timeout_msec;
}


/* ----------------------------------------------------------------------- */
int
dhd_os_ioctl_resp_wait(dhd_pub_t *pub, uint *condition)
{
	dhd_info_t *dhd = pub->info;

	if (osl_ext_sem_take(&dhd->ioctl_resp_wait,
	                     g_dhd_info->ioctl_timeout_msec) == OSL_EXT_SUCCESS)
		return (1);
	else
		return (0);
}


/* ----------------------------------------------------------------------- */
int
dhd_os_ioctl_resp_wake(dhd_pub_t *pub)
{
	dhd_info_t *dhd = pub->info;

	if (osl_ext_sem_give(&dhd->ioctl_resp_wait) == OSL_EXT_SUCCESS)
		return (1);
	else
		return (0);
}


/* ----------------------------------------------------------------------- */
void dhd_wait_for_event(dhd_pub_t *dhd, bool *lockvar)
{
	struct dhd_info *dhdinfo = dhd->info;
	osl_ext_status_t status;

	dhdinfo->ctrl_notified = FALSE;
	dhdinfo->ctrl_waiting = TRUE;

	dhd_os_sdunlock(dhd);
	status = osl_ext_sem_take(&dhdinfo->ctrl_wait, 2000);
	dhd_os_sdlock(dhd);

	dhdinfo->ctrl_waiting = FALSE;
	if (status != OSL_EXT_SUCCESS && dhdinfo->ctrl_notified == TRUE) {
		status = osl_ext_sem_take(&dhdinfo->ctrl_wait, 0);
		ASSERT(status == OSL_EXT_SUCCESS);
	}
}


/* ----------------------------------------------------------------------- */
void dhd_wait_event_wakeup(dhd_pub_t*dhd)
{
	struct dhd_info *dhdinfo = dhd->info;

	if (dhdinfo->ctrl_waiting == TRUE) {
		dhdinfo->ctrl_notified = TRUE;
		osl_ext_sem_give(&dhdinfo->ctrl_wait);
	}
}


#define MAX_WAIT_FOR_8021X_TX	10

static int
dhd_wait_pend8021x(dhd_info_t *dhd)
{
	unsigned int timeout_msec = 10;
	unsigned int ntimes = MAX_WAIT_FOR_8021X_TX;
	unsigned int pend = dhd->pend_8021x_cnt;

	while ((ntimes > 0) && (pend > 0)) {
		OSL_DELAY(timeout_msec * 1000);
		ntimes--;

		pend = dhd->pend_8021x_cnt;
	}

	return (pend);
}

/* ----------------------------------------------------------------------- */
/* send up locally generated event */
void
dhd_sendup_event(dhd_pub_t *dhdp, wl_event_msg_t *event, void *data)
{
	dhd_info_t	*dhd = (dhd_info_t *)dhdp->info;
	wl_drv_event_callback	evt_cb;
	int			j;
	int			ifidx = 0;		/* single interface for now */

	/* Post event to application. */
	wl_event_to_host_order(event);
	for (j = 0; j < MAX_EVENT_CALLBACK; j++) {
		evt_cb = dhd->iflist[ifidx].event_callback[j];
		if (evt_cb != NULL) {
			evt_cb(event, data);
		}
	}
}

int dhd_change_mtu(dhd_pub_t *dhdp, int new_mtu, int ifidx)
{
	return BCME_UNSUPPORTED;
}

#ifdef DHD_DEBUG
int
write_to_file(dhd_pub_t *dhd, uint8 *buf, int size)
{
	return 0;
}
#endif /* DHD_DEBUG */

int dhd_preinit_ioctls(dhd_pub_t *dhd)
{
#ifdef PROP_TXSTATUS
	dhd_wlfc_init(dhd);
#endif /* PROP_TXSTATUS */
}
