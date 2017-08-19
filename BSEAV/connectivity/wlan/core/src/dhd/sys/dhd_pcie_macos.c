/*
 * Dongle Host Driver (DHD) OS-specific DHD stubs for MacOSX
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
 * $Id: dhd_pcie_macos.c  $
 */
 /* FILE-CSTYLED */

/* include files */
#include <typedefs.h>
#include "osl.h"
#include <dngl_stats.h>
#include <pcie_core.h>
#include <dhd.h>
#include <dhd_bus.h>
#include <bcmmsgbuf.h>
#include <dhd_pcie.h>
#include <dhd_dbg.h>
#include <sys/vnode.h>
#include <sys/file.h>
#include <sys/proc.h>
#include <wl_export.h>

#ifdef DHD_DEBUG
uint dhd_console_ms = 250;
#else
uint dhd_console_ms = 0;
#endif /* DHD_DEBUG */

uint dhd_intr = TRUE;

int dhd_get_ifidx(struct dhd_info *dhd_info, char *name);

int
dhd_add_if(struct dhd_info *dhd, int ifidx, void *handle, char *name,
	uint8 *mac_addr, uint32 flags, uint8 bssidx)
{
	return 0;
}

int
dhd_event_ifadd(struct dhd_info *dhd, struct wl_event_data_if *ifevent,
	char *name, uint8 *mac)
{
	return 0;
}

int
dhd_event_ifdel(struct dhd_info *dhd, struct wl_event_data_if *ifevent,
	char *name, uint8 *mac)
{
	return 0;
}

int
dhd_register_if(dhd_pub_t *dhdp, int idx, bool need_rtnl_lock)
{
	return 0;
}

void
dhdpcie_free_irq(dhd_bus_t *bus)
{
}

int
dhdpcie_disable_irq_nosync(dhd_bus_t *bus)
{
	/* There is no, _nosync flavor in MAC and we will be
	 * calling this from a workloop context rather than
	 * ISR context in MAC. So calling same dhdpcie_disable_irq
	 * should work
	 */
	return dhdpcie_disable_irq(bus);
}

int
dhdpcie_disable_irq(dhd_bus_t *bus)
{
    if ((bus == NULL) || (bus->dhd == NULL)) {
        return BCME_ERROR;
    }

    return dhd_macos_disable_irq(bus->dhd);
}

int
dhdpcie_enable_irq(dhd_bus_t *bus)
{
    if ((bus == NULL) || (bus->dhd == NULL)) {
        return BCME_ERROR;
    }

    return dhd_macos_enable_irq(bus->dhd);
}

dhd_pub_t*
dhd_attach(osl_t *osh, struct dhd_bus *bus, uint bus_hdrlen)
{
	dhd_pub_t *dhdpub = NULL;

	dhdpub = MALLOC(osh, sizeof(*dhdpub));
	if (!dhdpub) {
		DHD_ERROR(("%s: OOM - alloc dhdpub\n", __FUNCTION__));
		goto fail;
	}
	memset(dhdpub, 0, sizeof(*dhdpub));
	dhdpub->osh = osh;
	dhdpub->bus = bus;
	dhdpub->hdrlen = bus_hdrlen;

	return dhdpub;
fail:
	if (dhdpub)
		MFREE(osh, dhdpub, sizeof(*dhdpub));
	return NULL;
}

void
dhd_detach(dhd_pub_t *dhdp)
{
}

void
dhd_free(dhd_pub_t *dhdpub)
{
	osl_t *osh = dhdpub->osh;
#ifdef CACHE_FW_IMAGES
	if (dhdpub->cached_fw) {
		MFREE(dhdpub->osh, dhdpub->cached_fw, dhdpub->bus->ramsize);
		dhdp->cached_fw = NULL;
	}

	if (dhdp->cached_nvram) {
		MFREE(dhdpub->osh, dhdpub->cached_nvram, MAX_NVRAMBUF_SIZE);
		dhdpub->cached_nvram = NULL;
	}
#endif
	MFREE(osh, dhdpub, sizeof(*dhdpub));
}

int
dhd_change_mtu(dhd_pub_t *dhdp, int new_mtu, int ifidx)
{
	return 0;
}

int
write_to_file(dhd_pub_t *dhd, uint8 *buf, int size)
{
	return 0;
}

void
dhd_del_if(struct dhd_info *dhd, int ifidx)
{
}

void
dhd_event(struct dhd_info *dhd, char *evpkt, int evlen, int ifidx)
{
}

int
dhd_ifname2idx(struct dhd_info *dhd, char *name)
{
	return dhd_get_ifidx(dhd, name);
}

unsigned int
dhd_os_get_ioctl_resp_timeout(void)
{
    return 0;
}

void
dhd_os_set_ioctl_resp_timeout(unsigned int timeout_msec)
{
}

static char wait_channel = 0;

int
dhd_os_ioctl_resp_wait(dhd_pub_t *pub, uint *condition)
{
	int ret;
	struct timespec ts;
	ts.tv_sec = 2;
	ts.tv_nsec = 0;
	ret = msleep(&wait_channel, NULL, 0, "dhd_os_ioctl_resp_wait", &ts);

	return 1;
}

int
dhd_os_ioctl_resp_wake(dhd_pub_t *pub)
{
	wakeup_one(&wait_channel);
	return 0;
}

void
dhd_os_wd_timer(void *bus, uint wdtick)
{
}

int
dhd_preinit_ioctls(dhd_pub_t *dhd)
{
	return 0;
}

void
dhd_sched_dpc(dhd_pub_t *dhdp)
{
	while (dhd_bus_dpc(dhdp->bus));
}

void
dhd_sendup_event(dhd_pub_t *dhdp, wl_event_msg_t *event, void *data)
{
	DHD_ERROR(("dhd_sendup_event \n"));
}

int
dhd_timeout_expired(dhd_timeout_t *tmo)
{
	if (tmo->increment == 0) {
		tmo->increment = 1;
		return 0;
	}

	if (tmo->elapsed > tmo->limit)
		return 1;
	OSL_DELAY(tmo->increment);
	tmo->increment = (tmo->increment > tmo->tick) ? tmo->tick : (tmo->increment * 2);

	tmo->elapsed += tmo->tick;
	return 0;
}

void
dhd_timeout_start(dhd_timeout_t *tmo, uint usec)
{
	memset(tmo, 0, sizeof(dhd_timeout_t));
	tmo->limit = usec;
	tmo->elapsed = 0;
	tmo->increment = 0;
	/* constant tick of 1000us */
	tmo->tick = 1000;
}

int
dhdpcie_bus_register(void)
{
	return 0;
}

void
dhdpcie_bus_unregister(void)
{
}

void
dhd_txflowcontrol(dhd_pub_t *dhdp, int ifidx, bool state)
{
}

void
dhd_os_sdlock(dhd_pub_t *pub)
{
}

void
dhd_os_sdunlock(dhd_pub_t *pub)
{
}

void *
dhd_os_spin_lock_init(osl_t *osh)
{
	return osl_spin_alloc_init();
}

void
dhd_os_spin_lock_deinit(osl_t *osh, void *lock)
{
	osl_spin_free(lock);
}

unsigned long
dhd_os_spin_lock(void *lock)
{
	osl_spin_lock(lock);
	return 0;
}

void
dhd_os_spin_unlock(void *lock, unsigned long flags)
{
	osl_spin_unlock(lock);
}

int
dhdpcie_pci_suspend_resume(struct dhd_bus *bus, bool state)
{
	return -1;
}

static struct {
	vfs_context_t ctx;
	struct vnode *vp;
	int offset;
} fw_ctx;

void *
dhd_os_open_image(char * filename)
{
	int error;

	memset(&fw_ctx, 0, sizeof(fw_ctx));
	fw_ctx.ctx = vfs_context_create(NULL);
	error = vnode_open(filename, (O_CREAT | FWRITE), (0), 0, &fw_ctx.vp, fw_ctx.ctx);
	if(error)
	{
		DHD_ERROR(("%s: vnode_open failed with %d\n", __FUNCTION__, error));
		vfs_context_rele(fw_ctx.ctx);
		fw_ctx.ctx = NULL;
	}
	return &fw_ctx;
}

int
dhd_os_get_image_block(char *buf, int len, void *image)
{
	int resid;
	int error;
	if (!image) {
		DHD_ERROR(("%s: No FW/NVRAM to download\n", __func__));
		return 0;
	}
	error = vn_rdwr(UIO_READ, fw_ctx.vp,
			buf, len, fw_ctx.offset,
			UIO_SYSSPACE, IO_SYNC|IO_NODELOCKED|IO_UNIT,
			vfs_context_ucred(fw_ctx.ctx), &resid,
			vfs_context_proc(fw_ctx.ctx));
	if(error || (resid == len)) {
		DHD_ERROR(("%s: vn_rdwr failed with %d, resid %d, len %d\n", __FUNCTION__, error, resid, len));
	}
	fw_ctx.offset += (len - resid);
	return (len - resid);
}

void
dhd_os_close_image(void *image)
{
	int error;
	if (image)
		error = vnode_close(fw_ctx.vp, FWRITE, fw_ctx.ctx);
	vfs_context_rele(fw_ctx.ctx);
	memset(&fw_ctx, 0, sizeof(fw_ctx));
}

#ifdef DHD_FW_COREDUMP
void dhd_schedule_memdump(dhd_pub_t *dhdp, uint8 *buf, uint32 size)
{
	wl_dump_mem((char *)buf, (int) size, WL_DUMP_MEM_SOCRAM);
}
#endif /* DHD_FW_COREDUMP */
