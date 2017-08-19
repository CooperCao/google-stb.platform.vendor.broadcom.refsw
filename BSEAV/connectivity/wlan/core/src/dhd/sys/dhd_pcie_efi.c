/*
 * EFI-specific PCIE portion of
 * Broadcom 802.11abgn Networking Device Driver
 *
 * Copyright (C) 2017, Broadcom. All Rights Reserved.
 * 
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: dhd_pcie_efi.c 552299 2015-12-03 00:56:24Z $
 */


/* include files */
#include <typedefs.h>
#include <osl.h>
#include <hndsoc.h>
#include <dngl_stats.h>
#include <pcie_core.h>
#include <dhd.h>
#include <dhd_bus.h>
#include <dhd_efi.h>
#include <dhd_proto.h>
#include <bcmmsgbuf.h>
#include <dhd_pcie.h>
#include <dhd_dbg.h>
#include <wl_export.h>
#include <bcmendian.h>
#include <pcicfg.h>
#include <hnd_pktq.h>

#ifdef DHD_DEBUG
uint dhd_console_ms = 250;
#else
uint dhd_console_ms = 0;
#endif /* DHD_DEBUG */

uint dhd_intr = TRUE;
/* IOCTL response timeout */
int dhd_ioctl_timeout_msec = IOCTL_RESP_TIMEOUT;

typedef struct dhdpcie_info
{
	dhd_bus_t *bus;
	osl_t *osh;
	volatile char *regs;		/* pci device memory Pa */
	volatile char *tcm;		/* pci device memory Pa */
	uint32 tcm_size;	/* pci device memory size */
	uint16 last_intrstatus;	/* to cache intrstatus */
	int	irq;
	char pciname[32];
	EFI_EVENT ioctl_event;
	EFI_EVENT intr_poll_timer;
	EFI_EVENT wait_list[1];
	CHAR16 fw_path[PATH_MAX];
	CHAR16 nv_path[PATH_MAX];
	CHAR16 clm_path[PATH_MAX];
} dhdpcie_info_t;

void EFIAPI
dhdpcie_intr_poll_timer_expired(IN EFI_EVENT Event,	IN VOID *Context)
{
	dhdpcie_info_t *dhdpcie = NULL;

	dhdpcie = (dhdpcie_info_t *)Context;
	ASSERT(dhdpcie);

	if (!dhdpcie) {
		DHD_ERROR(("%s: Fatal! dhdpcie is NULL !\n", __FUNCTION__));
		return;
	}

	dhdpcie_bus_isr(dhdpcie->bus);

	gBS->SetTimer(dhdpcie->intr_poll_timer, TimerRelative, INTR_POLL_PERIOD);

}

void EFIAPI
dhdpcie_ioctl_event_notify(IN EFI_EVENT Event, IN VOID *Context)
{

}


int
dhdpcie_detach(dhdpcie_info_t *pch)
{
	if (pch) {
		MFREE(pch->osh, pch, sizeof(dhdpcie_info_t));
	}
	return 0;
}

int
dhdpcie_get_oly_fwpath_otp(dhd_bus_t *bus, char *fw_path, char *nv_path, char *clm_path)
{
	uint32 val = 0;
	uint16 chip_id = 0;
	uint16 otp_word = 0;
	uint8 otp_data[2];
	char stepping[2];
	char module_name[5];
	char module_vendor;
	char module_rev[4];
	uint8 tuple_id = 0;
	uint8 tuple_len = 0;
	uint32 cur_offset = 0;
	char module_info[64];
	char dirname[32];
	char progname[32];
	bool srom_present = 0;
	uint32 sprom_ctrl = 0;
	uint32 otp_ctrl = 0;
	int i = 0, j = 0, status = BCME_ERROR;

	if (!fw_path || !nv_path || !clm_path || !bus)
		return BCME_ERROR;

	/* read chip id first */
	if (si_backplane_access(bus->sih, SI_ENUM_BASE(bus->sih), 4, &val, TRUE) != BCME_OK) {
		DHD_ERROR(("%s: bkplane access error ! \n", __FUNCTION__));
	} else {
		chip_id = val & 0xffff;
	}

	/* read SpromCtrl register */
	si_backplane_access(bus->sih, SPROM_CTRL_REG_ADDR(bus->sih), 4, &sprom_ctrl, TRUE);
	val = sprom_ctrl;

	/* proceed only if OTP is present - i.e, the 5th bit OtpPresent is set
	* and chip is 4355 or 4364
	*/
	if ((val & 0x20) && (chip_id == 0x4355 || chip_id == 0x4364)) {
		DHD_ERROR(("%s : OTP present\n", __FUNCTION__));
		/* check if the 0th bit, SromPresent is set in spromctrl register */
		if (val & 0x1) {
			DHD_ERROR(("%s : SROM present\n", __FUNCTION__));
			srom_present = 1;
		}
		/* OTP power up sequence */
		/* 1. cache otp ctrl and enable OTP clock through OTPCtrl1 register */
		si_backplane_access(bus->sih, OTP_CTRL1_REG_ADDR(bus->sih), 4, &otp_ctrl, TRUE);
		val = 0xFA0000;
		si_backplane_access(bus->sih, OTP_CTRL1_REG_ADDR(bus->sih), 4, &val, FALSE);

		/* 2. enable OTP power through min res mask register in PMU */
		si_backplane_access(bus->sih, PMU_MINRESMASK_REG_ADDR(bus->sih), 4, &val, TRUE);
		val |= 0xC47;
		si_backplane_access(bus->sih, PMU_MINRESMASK_REG_ADDR(bus->sih), 4, &val, FALSE);

		/* 3. if srom is present, need to set OtpSelect 4th bit
		* in SpromCtrl register to read otp
		*/
		if (srom_present) {
			val = sprom_ctrl | 0x10;
			si_backplane_access(bus->sih,
				SPROM_CTRL_REG_ADDR(bus->sih), 4, &val, FALSE);
		}

		cur_offset = OTP_USER_AREA_ADDR(bus->sih) + 0x40;

		/* read required data from otp to construct FW string name
		* data like - chip info, module info. This is present in the
		* form of a Vendor CIS Tuple whose format is provided by Olympic.
		* The data is in the form of ASCII character strings.
		* The Vendor tuple along with other CIS tuples are present
		* in the OTP user area. A CIS tuple is a TLV format.
		* (T = 1-byte, L = 1-byte, V = n-bytes)
		*/
		/* Find the vendor tuple */
		while (tuple_id != OTP_CIS_REGION_END_TUPLE_ID) {
			si_backplane_access(bus->sih, cur_offset,
					2, (uint *)otp_data, TRUE);
			tuple_id = otp_data[0];
			tuple_len = otp_data[1];
			if (tuple_id == OTP_VENDOR_TUPLE_ID) {
				break;
			}
			/* if its NULL tuple, skip */
			if (tuple_id == 0)
				cur_offset += 1;
			else
				cur_offset += tuple_len + 2;
		}
		/* skip the major, minor ver. numbers, manufacturer and product names */
		cur_offset += 6;

		/* read the chip info */
		si_backplane_access(bus->sih, cur_offset,
				2, (uint *)otp_data, TRUE);
		if (otp_data[0] == 's' && otp_data[1] == '=') {
			/* read the stepping */
			cur_offset += 2;
			si_backplane_access(bus->sih, cur_offset,
					2, (uint *)stepping, TRUE);
			/* read module info */
			memset(module_info, 0, 64);
			cur_offset += 2;
			si_backplane_access(bus->sih, cur_offset,
					2, (uint *)otp_data, TRUE);
			while (otp_data[0] != OTP_CIS_REGION_END_TUPLE_ID &&
					otp_data[1] != OTP_CIS_REGION_END_TUPLE_ID) {
					memcpy(&module_info[i], otp_data, 2);
					i += 2;
					cur_offset += 2;
					si_backplane_access(bus->sih, cur_offset,
							2, (uint *)otp_data, TRUE);
			}
			/* replace any null characters found at the beginning
			* and middle of the string
			*/
			for (j = 0; j < i; ++j) {
				if (module_info[j] == 0)
					module_info[j] = ' ';
			}
			DHD_INFO(("OTP chip_info: s=%c%c; module info: %s \n",
					stepping[0], stepping[1], module_info));
			/* extract the module name, revision and vendor
			* information from the module info string
			*/
			for (i = 0; module_info[i]; i++) {
				if (module_info[i] == 'M' && module_info[i+1] == '=') {
					memcpy(module_name, &module_info[i+2], 4);
					module_name[4] = 0;
					i += 5;
				} else if (module_info[i] == 'm' && module_info[i+1] == '=') {
					memcpy(module_rev, &module_info[i+2], 3);
					module_rev[3] = 0;
					i += 4;
				} else if (module_info[i] == 'V' && module_info[i+1] == '=') {
					module_vendor = module_info[i+2];
					i += 2;
				}
			}

			/* construct the complete file paths to clm, nvram and firmware as per
			* olympic conventions
			*/
			memset(dirname, 0, 32);
			memset(progname, 0, 32);
			sprintf(dirname, "C-%x__s-%c%c", chip_id, stepping[0], stepping[1]);
			if (chip_id == 0x4355)
				sprintf(progname, "simbaab");
			else
				sprintf(progname, PROGRAM_NAME);

			/* firmware file name */
			sprintf(fw_path, "%s\\%s\\%s.trx", WIRELESS_PATH,
					dirname, progname);
			/* clm file name */
			sprintf(clm_path, "%s\\%s\\%s.clmb", WIRELESS_PATH,
					dirname, progname);
			/* nvram file name */
			sprintf(nv_path, "%s\\%s\\P-%s_M-%s_V-%c__m-%s.txt", WIRELESS_PATH,
					dirname, progname, module_name, module_vendor, module_rev);

			DHD_ERROR(("OTP read success. \n FW path = %s\n"
					" NVRAM path = %s\n CLM path = %s \n",
					fw_path, nv_path, clm_path));

			status = BCME_OK;
		}
		if (srom_present) {
			si_backplane_access(bus->sih,
				SPROM_CTRL_REG_ADDR(bus->sih), 4, &sprom_ctrl, FALSE);
			si_backplane_access(bus->sih,
				OTP_CTRL1_REG_ADDR(bus->sih), 4, &otp_ctrl, FALSE);
		}
	}

	return status;

}

int
dhdpcie_init(void *controller, void *pcie_dev, struct spktq *q_txdone,
		char *clmblob_path)
{
	osl_t *osh = NULL;
	dhd_bus_t *bus = NULL;
	dhdpcie_info_t *dhdpcie_info =  NULL;
	char fwpath[PATH_MAX];
	char nvpath[PATH_MAX];
	char clmpath[PATH_MAX];
	uint8 otp_path_set = 0;
	uint32 bar0_addr = 0, bar1_addr = 0;
	uint32 cmd = 0, val = 0;
	EFI_STATUS status;
	int ret = BCME_OK;

	do {
		/* read bar0 and bar1 addresses
		*  EFI is non-virtual memory environment. The base address of the device
		* is the same as virtual address for device reg map.
		* Register reads don't require the virtual address as it's offset relative to
		* Bar index, but some sb API don't like Vaddr to be 0
		*/
		((EFI_PCI_IO_PROTOCOL *)pcie_dev)->Pci.Read((EFI_PCI_IO_PROTOCOL *)pcie_dev,
				EfiPciIoWidthUint32,
				PCI_CFG_BAR0,
				1,
				&bar0_addr);
		bar0_addr = bar0_addr & PCIBAR_MEM32_MASK;

		/* OSL ATTACH */
		/* piomode turned off for now */
		DHD_ERROR(("%s: osl_attach %d %d\n", __FUNCTION__, NTXBUF, NRXBUF));
		osh = osl_attach((EFI_HANDLE)controller,
				(uint8 *)bar0_addr,
				(EFI_PCI_IO_PROTOCOL *)pcie_dev,
		        NTXBUF,
		        NRXBUF,
		        0,
		        q_txdone);
		if (!osh) {
			DHD_ERROR(("%s: osl_attach failed\n", __FUNCTION__));
			break;
		}

		bar1_addr = osl_pci_read_config(osh, PCI_CFG_BAR1, sizeof(bar1_addr));
		bar1_addr = bar1_addr & PCIBAR_MEM32_MASK;
		DHD_ERROR(("%s: BAR0 addr = %X; BAR1 addr = %X \n",
				__FUNCTION__, bar0_addr, bar1_addr));

		/*	allocate efi specific pcie structure here */
		if (!(dhdpcie_info = MALLOCZ(osh, sizeof(dhdpcie_info_t)))) {
			DHD_ERROR(("%s: MALLOC of dhdpcie_info_t failed !\n", __FUNCTION__));
			break;
		}

		dhdpcie_info->osh = osh;
		dhdpcie_info->regs = (volatile char *)bar0_addr;
		dhdpcie_info->tcm = (volatile char *)bar1_addr;
		dhdpcie_info->tcm_size = DONGLE_TCM_MAP_SIZE;

		/* Enable the PCI Master */
		/*
		 * Our device support 64-bit DMA.  Let's enable DAC support to take
		 * advantage of it if chipset supports it.  If it's supported, then
		 * buffer copy is eliminated during mapping.
		 */
		if (osl_pci_set_attributes(osh, TRUE,  EFI_PCI_IO_ATTRIBUTE_MEMORY |
				EFI_PCI_IO_ATTRIBUTE_BUS_MASTER, NULL) != BCME_OK) {
			cmd = 0x6;
			DHD_ERROR(("Setting PCI Attribute failed ! \n"));
			osl_pci_write_config(osh, PCI_COMMAND, sizeof(cmd), cmd);
		}

		/* Bus initialization */
		bus = dhdpcie_bus_attach(osh, dhdpcie_info->regs, dhdpcie_info->tcm, NULL);
		if (!bus) {
			DHD_ERROR(("%s:dhdpcie_bus_attach() failed !\n", __FUNCTION__));
			break;
		}
		dhdpcie_info->bus = bus;
		bus->pcie_dev = (void *)dhdpcie_info;

		memset(fwpath, 0, PATH_MAX);
		memset(nvpath, 0, PATH_MAX);
		memset(clmpath, 0, PATH_MAX);
		if (dhdpcie_get_oly_fwpath_otp(bus, fwpath, nvpath, clmpath) != BCME_OK) {
			/* use default fw/nvram/clm paths */
			strcpy(fwpath, FW_PATH);
			strcpy(nvpath, NV_PATH);
			strcpy(clmpath, CLM_PATH);
		} else {
			otp_path_set = 1;
		}

		if (clmblob_path)
			strcpy(clmblob_path, clmpath);

		/* firmware download */
#ifndef BCMEMBEDIMAGE
		if (dhd_bus_download_firmware(bus, osh, fwpath, nvpath) < 0) {
			/* if OTP path FW is not found, fall back to default FW path */
			if (otp_path_set) {
				strcpy(fwpath, FW_PATH);
				strcpy(nvpath, NV_PATH);
				DHD_ERROR(("%s: failed to download firmware !"
						" Trying default FW/NVRAM path\n",
						__FUNCTION__));
				if (dhd_bus_download_firmware(bus, osh, fwpath, nvpath) < 0) {
					DHD_ERROR(("%s: failed to download default firmware !"
							" abort.\n", __FUNCTION__));
					ret = BCME_DONGLE_DOWN;
				}
			}
			else {
				/* default FW download itself failed */
				DHD_ERROR(("%s: failed to download default firmware ! abort.\n",
						__FUNCTION__));
				ret = BCME_DONGLE_DOWN;
			}
		}

#else
		if (dhd_bus_download_firmware(bus, osh, NULL, NULL) < 0) {
			bus->nv_path = NULL;
			bus->fw_path = NULL;
			DHD_ERROR(("%s: failed to download firmware !\n", __FUNCTION__));
			ret = BCME_DONGLE_DOWN;
		}

#endif /* BCMEMBEDIMAGE */

		if (ret == BCME_OK)
			DHD_ERROR(("%s: download firmware success. \n", __FUNCTION__));

		/* create timers and events */
		status = gBS->CreateEvent(
			EFI_EVENT_NOTIFY_SIGNAL | EFI_EVENT_TIMER,
			DHD_LOCK_LEVEL_HIGH,
			dhdpcie_intr_poll_timer_expired,
			(VOID *)dhdpcie_info,
			&dhdpcie_info->intr_poll_timer);
		if (EFI_ERROR (status)) {
			DHD_ERROR(("%s: timer event creation failed !\n", __FUNCTION__));
			break;
		}

		status = gBS->CreateEvent(
			EFI_EVENT_NOTIFY_WAIT,
			DHD_LOCK_LEVEL_LOW,
			dhdpcie_ioctl_event_notify,
			(VOID *)dhdpcie_info,
			&dhdpcie_info->ioctl_event);
		if (EFI_ERROR (status)) {
			DHD_ERROR(("%s: ioctl event creation failed !\n", __FUNCTION__));
			gBS->CloseEvent(dhdpcie_info->intr_poll_timer);
			break;
		}

		/* start the periodic interrupt poll timer */
		gBS->SetTimer(dhdpcie_info->intr_poll_timer, TimerRelative, INTR_POLL_PERIOD);

		return ret;
	} while (0);


	if (dhdpcie_info->intr_poll_timer) {
		gBS->SetTimer(dhdpcie_info->intr_poll_timer, TimerCancel, 0);
		gBS->CloseEvent(dhdpcie_info->intr_poll_timer);
	}

	if (dhdpcie_info->ioctl_event)
		gBS->CloseEvent(dhdpcie_info->ioctl_event);

	if (bus)
		dhdpcie_bus_release(bus);

	if (dhdpcie_info)
		MFREE(osh, dhdpcie_info, sizeof(dhdpcie_info_t));

	if (osh)
		osl_detach(osh);
	return BCME_ERROR;

}

int dhdpcie_deinit(dhd_pub_t *pub)
{
	osl_t *osh = NULL;
	dhdpcie_info_t *pch = NULL;
	dhd_bus_t *bus = NULL;
	int err = 0;

	osh = pub->osh;
	bus = pub->bus;
	ASSERT(bus);
	ASSERT(osh);

	pch = (dhdpcie_info_t *)bus->pcie_dev;
	ASSERT(pch);

	err = osl_pci_set_attributes(osh, FALSE,  EFI_PCI_IO_ATTRIBUTE_MEMORY |
			EFI_PCI_IO_ATTRIBUTE_BUS_MASTER,
			NULL);

	if (pch->intr_poll_timer) {
		gBS->SetTimer(pch->intr_poll_timer, TimerCancel, 0);
		gBS->CloseEvent(pch->intr_poll_timer);
	}

	if (pch->ioctl_event)
		gBS->CloseEvent(pch->ioctl_event);

	dhdpcie_bus_release(bus);

	dhdpcie_detach(pch);

	osl_detach(osh);

	return err;
}

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
dhdpcie_disable_irq(dhd_bus_t *bus)
{
	return BCME_OK;
}

int
dhdpcie_enable_irq(dhd_bus_t *bus)
{
	return BCME_OK;
}


void
dhd_free(dhd_pub_t *dhdpub)
{
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
	return 0; //return dhd_get_ifidx(dhd, name);
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

int
dhdpcie_pci_suspend_resume(struct dhd_bus *bus, bool state)
{
	return -1;
}

unsigned int
dhd_os_get_ioctl_resp_timeout(void)
{
	return ((unsigned int)dhd_ioctl_timeout_msec);
}

void
dhd_os_set_ioctl_resp_timeout(unsigned int timeout_msec)
{
	dhd_ioctl_timeout_msec = (int)timeout_msec;
}

int
dhd_os_ioctl_resp_wait(dhd_pub_t *pub, uint *condition)
{
	dhdpcie_info_t *dhdpcie = NULL;
	EFI_STATUS status;
	UINTN idx = 0;
	uint32 elapsed_time = 0;
	uint32 jiffies_ms = WAIT_EVENT_STALL_PERIOD/1000; /* 10ms in units of u-sec */

	ASSERT(pub);
	ASSERT(pub->bus);
	ASSERT(pub->bus->pcie_dev);

	if (!pub || !pub->bus || !pub->bus->pcie_dev) {
		DHD_ERROR(("%s: Fatal! pub/bus/pcie_dev is NULL !\n", __FUNCTION__));
		return BCME_ERROR;
	}

	dhdpcie = (dhdpcie_info_t *)pub->bus->pcie_dev;
	ASSERT(dhdpcie->ioctl_event);
	if (!dhdpcie) {
		DHD_ERROR(("%s: Fatal! dhdpcie is NULL !\n", __FUNCTION__));
		return BCME_ERROR;
	}

	while (1) {
		status = gBS->CheckEvent(dhdpcie->ioctl_event);
		if (status == EFI_INVALID_PARAMETER) {
			DHD_ERROR(("%s: error waiting on ioctl_event !\n", __FUNCTION__));
			return BCME_ERROR;
		} else if (status == EFI_SUCCESS) {
			break;
		}
		OSL_DELAY(WAIT_EVENT_STALL_PERIOD);
		elapsed_time += jiffies_ms;
		if (elapsed_time >= dhd_ioctl_timeout_msec)
			return 0;
	}

	return 1;
}

int
dhd_os_ioctl_resp_wake(dhd_pub_t *pub)
{
	dhdpcie_info_t *dhdpcie = NULL;
	EFI_STATUS status;

	ASSERT(pub);
	ASSERT(pub->bus);
	ASSERT(pub->bus->pcie_dev);

	if (!pub || !pub->bus || !pub->bus->pcie_dev) {
		DHD_ERROR(("%s: Fatal! pub/bus/pcie_dev is NULL !\n", __FUNCTION__));
		return BCME_ERROR;
	}

	dhdpcie = (dhdpcie_info_t *)pub->bus->pcie_dev;
	ASSERT(dhdpcie->ioctl_event);
	if (!dhdpcie) {
		DHD_ERROR(("%s: Fatal! dhdpcie is NULL !\n", __FUNCTION__));
		return BCME_ERROR;
	}

	status = gBS->SignalEvent(dhdpcie->ioctl_event);
	if (EFI_ERROR (status)) {
		DHD_ERROR(("%s: error signalling ioctl_event !\n", __FUNCTION__));
		return BCME_ERROR;
	}

	return 0;
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
