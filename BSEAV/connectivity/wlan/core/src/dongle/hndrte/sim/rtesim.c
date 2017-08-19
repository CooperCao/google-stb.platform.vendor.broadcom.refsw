/*
 * Initialization and support routines for RTE simulations.
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
#include <siutils.h>
#include <wlioctl.h>
#include <bcmdevs.h>
#include <epivers.h>
#include <bcmutils.h>
#include <bcmnvram.h>
#include <hndsoc.h>
#include <sbchipc.h>
#include <rte_dev.h>
#include <rte.h>


static si_t *sih;
static ctimeout_t state_timer;

/* WL device struct */
extern hnd_dev_t bcmwl;

/* SDIO device struct */
extern hnd_dev_t usbdev_dev;
extern hnd_dev_t sdpcmd_dev;

#ifndef EXT_CBALL
extern void chip_init(void);
extern void sb_runcores(void);
#endif

/* armulatorsim.c */
extern int connect_usb_host(void);
extern int connect_sdio_host(void);
extern int generate_d11_frame(uint nbytes);

#ifdef AMPDU_CYCLES_MEASUREMENT
extern int generate_d11_addbr_res_frame(uchar* bss_id);
extern int generate_d11_ampdu_ba_frame();
#endif

extern int generate_usb_frame(uint nbytes);
extern uint64 cball_core_cycle(void);
extern int cball_core_clock(void);
extern void cball_dn_enable(void);

static void simulate_d11_handler(void *unused);

/* Forward decls */
#ifdef USB_DNGL
static void simulate_usb_handler(void *unused);
#endif

#ifdef SDIO_DNGL
extern int generate_sdio_frame(uint nbytes);

static void simulate_sdio_handler(void *nused);
#endif

static void associate_handler(void *unused);
static void connect_host_handler(void *unused);
static void conf_wl_handler(void *unused);

static int frame_size_array[] = {1500, 512, 64};

#ifdef AMPDU_CYCLES_MEASUREMENT
uint32 __gcycles = 0, __gtimer = 0, __gcaladj = 0;
#endif


#ifdef USB_DNGL
static void
simulate_usb_handler(void *unused)
{
#ifdef AMPDU_CYCLES_MEASUREMENT
	int frame_size;
	static int frame_size_idx = 0;

	static int i = 0;
	int ioval = 385;
	static  char params[16];
	int j;

	if (i == 0) {
		/*
		PROF_START();
		*/
		/* enable N rate */
		bzero(params, 16);
		strcpy(params, "nrate");
		bcopy((char *)&ioval, &params[strlen("nrate")+1], sizeof(int));
		bcmwl.ops->ioctl(&bcmwl, WLC_SET_VAR, params, 10, NULL, NULL, TRUE);
	}
	frame_size = frame_size_array[frame_size_idx++];
	frame_size_idx %= (sizeof(frame_size_array) / sizeof(frame_size_array[0]));

	/*
	PROF_INTERIM(" simulate_usb_handler() START");
	*/

	if (i == 0)
		generate_usb_frame(frame_size);

	if (i == 1)
		for (j = 0; j < 5; j++) {
			generate_usb_frame(frame_size);
			frame_size = frame_size_array[frame_size_idx++];
			frame_size_idx %= (sizeof(frame_size_array) / sizeof(frame_size_array[0]));
		}
	i++;

	printf("Generate USB frame (%d)\n", frame_size);

	/* Simulate receipt of USB frame */
	generate_usb_frame(frame_size);
#else
#ifdef TEST_AMSDU

	{
		char params[16];

		/* enable amsdu-tx-chain path */
		bzero(params, 16);
		strcpy(params, "amsdu_sim");
		strcpy(&params[strlen("amsdu_sim")+1], "1");
		printf("AMSDU args=%s %s\n", params, &params[strlen("amsdu_sim")+1]);
		bcmwl.ops->ioctl(&bcmwl, WLC_SET_VAR, params, strlen(params) + 1 + 4,
		                   NULL, NULL, TRUE);
	}

#endif /* TEST_AMSDU */
#endif /*  AMPDU_CYCLES_MEASUREMENT */

	/* Schedule d11 frame */
	hndrte_timeout(&state_timer, 10, simulate_d11_handler, NULL);
}
#endif /* USB_DNGL */

#ifdef SDIO_DNGL
static void
simulate_sdio_handler(void *unused)
{
	static int frame_size_idx = 0;
	int frame_size;

	frame_size = frame_size_array[frame_size_idx++];
	frame_size_idx %= (sizeof(frame_size_array) / sizeof(frame_size_array[0]));

	printf("Generate SDIO frame (%d)\n", frame_size);

	/* Simulate receipt of SDIO frame */
	generate_sdio_frame(frame_size);

	/* Schedule d11 frame */
	hndrte_timeout(&state_timer, 10, simulate_d11_handler, NULL);
}
#endif /* SDIO_DNGL */

static void
simulate_d11_handler(void *unused)
{
	static int frame_size_idx = 0;

	int frame_size;

#ifdef AMPDU_CYCLES_MEASUREMENT

	static int i = 0x00;

	uint8 bssid[6];

	bcmwl.ops->ioctl(&bcmwl, WLC_GET_BSSID, &bssid, sizeof(bssid),
	                   NULL, NULL, FALSE);

	printf("Associated to %.2X:%.2X:%.2X:%.2X:%.2X:%.2X\n",
	bssid[0], bssid[1], bssid[2], bssid[3], bssid[4], bssid[5]);

	/*
	PROF_INTERIM(" simulate_d11_handler() START");
	*/

	if (i == 0)
		generate_d11_addbr_res_frame(bssid);
	if (i == 2)
		generate_d11_ampdu_ba_frame();
	i++;
#else

	frame_size = frame_size_array[frame_size_idx++];
	frame_size_idx %= (sizeof(frame_size_array) / sizeof(frame_size_array[0]));

	printf("Generate D11 frame (%d)\n", frame_size);

	/* Simulate receipt of d11 frame */
	generate_d11_frame(frame_size);
#endif /*  AMPDU_CYCLES_MEASUREMENT */

#ifdef USB_DNGL
	/* Schedule usb frame */
	hndrte_timeout(&state_timer, 10, simulate_usb_handler, NULL);
	hndrte_timeout(&state_timer, 10, simulate_usb_handler, NULL);
#endif

#ifdef SDIO_DNGL
	/* Schedule sdio frame */
	hndrte_timeout(&state_timer, 10, simulate_sdio_handler, NULL);
#endif
}

static uint apmode = 2;

static int
wl_iovar_setint(char *var, uint val)
{
	int status;
	char data[32];
	uint t = strlen(var);
	strcpy(data, var);
	memcpy(data + t + 1, &val, sizeof(val));
	status = bcmwl.ops->ioctl(&bcmwl, WLC_SET_VAR, data, sizeof(data),
	                            NULL, NULL, TRUE);
	if (status)
		printf("wl_iovar_setint: %s failed %d\n", var, status);
	return status;
}

static int
wl_ioctl_setint(uint ioctl, uint val)
{
	int status;
	status = bcmwl.ops->ioctl(&bcmwl, ioctl, &val, sizeof(val), NULL, NULL, TRUE);
	if (status)
		printf("wl_ioctl_setint: 0x%x failed %d\n", ioctl, status);
	return status;
}


static void
wl_dump(char *what)
{
	static char dump_buf[2048];
	int status;

	strcpy(dump_buf, "dump");
	strcpy(&dump_buf[5], what);
	strcat(&dump_buf[5], " ");
	status = bcmwl.ops->ioctl(&bcmwl, WLC_GET_VAR, dump_buf, sizeof(dump_buf),
	                            NULL, NULL, TRUE);
	if (status)
		printf("wl_dump: %s failed %d\n", what, status);
	else
		printf("%s", dump_buf);
}

static void
associate_handler(void *unused)
{
	uint8 bssid[6];
	int ret;

	/* report status */
	ret = bcmwl.ops->ioctl(&bcmwl, WLC_GET_BSSID, &bssid, sizeof(bssid),
	                         NULL, NULL, FALSE);

	if (ret) {
		printf("Not associated\n");
		/* Try again later */
		hndrte_timeout(&state_timer, 500, associate_handler, NULL);
	} else {
		printf("Associated to %.2X:%.2X:%.2X:%.2X:%.2X:%.2X\n",
		       bssid[0], bssid[1], bssid[2], bssid[3], bssid[4], bssid[5]);

#ifdef AMPDU_CYCLES_MEASUREMENT
		simulate_usb_handler(unused);
#else
#ifndef RSOCK
		/* Simulate d11 frame */
		simulate_d11_handler(unused);
#endif
#endif /*  AMPDU_CYCLES_MEASUREMENT */
		if (apmode == 0)
			wl_dump("scb");
	}
}

static int _ac = 0;
static char **_av = NULL;

static void
conf_wl_handler(void *unused)
{
	wlc_ssid_t ssid;
	const char *ssid_str;
	int i;

	printf("Configuring wl\n");

	/* wl conf */
	wl_ioctl_setint(WLC_DOWN, 0);
	for (i = 1; i < _ac; i ++) {
		char *eq = strstr(_av[i], "=");
		if (!eq)
			printf("option %s missing value\n", _av[i]);
		else if (!strncmp(_av[i], "msglevel", eq - _av[i]))
			wl_ioctl_setint(WLC_SET_MSGLEVEL, strtoul(eq + 1, NULL, 0));
		else if (!strncmp(_av[i], "apmode", eq - _av[i])) {
			uint apmode = strtoul(eq + 1, NULL, 0);
			int val;
			if (apmode == 2)
				val = 0;
			else
				val = 1;
			wl_ioctl_setint(WLC_SET_INFRA, val);
			if (apmode == 2)
				val = 0;
			else
				val = apmode;
			wl_ioctl_setint(WLC_SET_AP, val);
		}
		else if (!strncmp(_av[i], "bcnprd", eq - _av[i]))
			wl_ioctl_setint(WLC_SET_BCNPRD, strtoul(eq + 1, NULL, 0));
		else if (eq - _av[i] >= 16)
			printf("variable %s name too long\n", _av[i]);
		else {
			char var[16];
			strncpy(var, _av[i], eq - _av[i]);
			var[eq - _av[i]] = 0;
			wl_iovar_setint(var, strtoul(eq + 1, NULL, 0));
		}
	}
	wl_ioctl_setint(WLC_UP, 0);

	/* join/create a BSS */
	if ((ssid_str = nvram_get("wl0_ssid")) == NULL)
		ssid_str = "noname";

	ssid.SSID_len = strlen(ssid_str);
	strncpy((char *)ssid.SSID, ssid_str, ssid.SSID_len);

	bcmwl.ops->ioctl(&bcmwl, WLC_SET_SSID, &ssid, sizeof(wlc_ssid_t),
	                   NULL, NULL, TRUE);

	/* Wait for association */
	hndrte_timeout(&state_timer, 10, associate_handler, NULL);
}

static void
connect_host_handler(void *unused)
{
	printf("Connecting host bus\n");

	/* Simulate host bus attach */
#ifdef USB_DNGL
	connect_usb_host();
#endif
#ifdef SDIO_DNGL
	connect_sdio_host();
#endif

	/* Add a state machine timer */
	hndrte_timeout(&state_timer, 10, conf_wl_handler, NULL);
}


/* globals used for profiling the code */
uint32 __gcycles = 0, __gtimer = 0, __gcaladj = 0;

int
main(int ac, char *av[])
{
	int err = 0;
	uint16 id;
	char chn[8];

	/*
	ENABLE_TIMER();
	PROF_START();
	__gcaladj = PROF_SUSPEND() + 3;

	PROF_START();
	PROF_SUSPEND();
	PROF_RESUME();
	PROF_INTERIM("Zero calibration");
	PROF_FINISH("Zero calibration");
	*/

	_ac = ac;
	_av = av;

#ifndef EXT_CBALL
	/* initialize the chip and sprom */
	chip_init();
#endif

	/* Basic initialization */
	sih = hnd_init();

#ifdef EXT_CBALL
	cball_dn_enable();
#endif


	printf("\nRTE Simulation v%s running on a %s Rev. %d\n\n",
		EPI_VERSION_STR, bcm_chipname(sih->chip, chn, 8), sih->chiprev);

	/* Add the WL device */
	if ((id = getintvar(NULL, "wl0id")) == 0)
		id = BCM4318_D11G_ID;

	if (hnd_add_device(sih, &bcmwl, D11_CORE_ID, id) != 0) {
		printf("WL device add failed\n");
		return -1;
	}

	/* Open wl device */
	if ((err = bcmwl.ops->open(&bcmwl)) != 0) {
		printf("WL device open failed, code %d\n", err);
		return -1;
	}

#ifdef USB_DNGL
	/* Add the USB device. */
	if (hnd_add_device(sih, &usbdev_dev, USB_CORE_ID, BCM47XX_USBD_ID) != 0 &&
	    hnd_add_device(sih, &usbdev_dev, USB11D_CORE_ID, BCM47XX_USBD_ID) != 0 &&
	    hnd_add_device(sih, &usbdev_dev, USB20D_CORE_ID, BCM47XX_USB20D_ID) != 0) {
		printf("USB device add failed\n");
		return -1;
	}

	/* Open USB device  */
	if ((err = usbdev_dev.ops->open(&usbdev_dev)) != 0) {
		printf("USB device open failed, code %d\n", err);
		return -1;
	}
#endif

#ifdef SDIO_DNGL
	/* Add the SDIO device. */
	if ((err = hnd_add_device(sih, &sdpcmd_dev, PCMCIA_CORE_ID, SDIOD_FPGA_ID)) != 0) {
		printf("SDIO device add failed, code %d\n", err);
		return -1;
	}

	/* Open SDIO device */
	if ((err = sdpcmd_dev.ops->open(&sdpcmd_dev)) != 0) {
		printf("SDIO device open failed, code %d\n", err);
		return -1;
	}
#endif

#ifndef RTE_POLL
	/* Begin accepting interrupts */
	hnd_enable_interrupts();
#endif


	/* Add a state machine timer */
	hndrte_timeout(&state_timer, 10, connect_host_handler, NULL);

	while (1) {
#ifndef EXT_CBALL
		sb_runcores();
#endif
		hnd_poll(sih);
	}

	return 0;
}

#ifdef EXT_CBALL
/*
 * Rate Statistics
 */

#define RSTATS_MAX		8

struct ratestat {
	char *module_name;
	char *stat_name;
	uint64 cycle_start;
	uint64 packet_count;
	uint64 byte_count;
} rstats[RSTATS_MAX];

static int rstats_triggered;
static int rstats_core_clock;

#define RSTATS_REPORT_CYCLE_INTERVAL	5000000

static uint64 rstats_report_cycle;

void
RateStatShow(void)
{
	uint64 now;
	int i;

	now = cball_core_cycle();

	printf("+----------------+---------+----------+--------+----------+--------+\n"
	       "| Stat           | Packets |    Bytes | AvSize |   Byte/s | Mbit/s |\n"
	       "+----------------+---------+----------+--------+----------+--------+\n");

	for (i = 0; i < RSTATS_MAX; i++) {
		struct ratestat *r = &rstats[i];
		int avsize, bytes_s, bits_s;
		uint64 total_cycles;
		char name[64];

		if (r->stat_name == NULL)
			break;

		sprintf(name, "%s.%s", r->module_name, r->stat_name);
		total_cycles = now - r->cycle_start;
		avsize = (r->packet_count > 0) ? (int)(r->byte_count / r->packet_count) : 0;
		bytes_s = (int)((uint64)r->byte_count * rstats_core_clock / total_cycles);
		bits_s = 8 * bytes_s;

		printf("| %-14s | %7d | %8d | %6d | %8d | %3d.%d%d |\n",
		       name,
		       (int)r->packet_count,
		       (int)r->byte_count,
		       avsize,
		       bytes_s,
		       bits_s / 1000000, (bits_s / 100000) % 10, (bits_s / 10000) % 10);
	}

	printf("+----------------+---------+----------+--------+----------+--------+\n\n");
}

void
RateStat(char *module_name, char *stat_name, int bytes)
{
	struct ratestat *r;
	uint64 now;
	int i;

	if (!rstats_triggered)
		return;

	now = cball_core_cycle();

	for (i = 0; i < RSTATS_MAX; i++) {
		r = &rstats[i];

		/* Add new stat */
		if (r->module_name == NULL) {
			r->module_name = module_name;
			r->stat_name = stat_name;
			r->cycle_start = now;
			r->packet_count = 1;
			r->byte_count = bytes;
			break;
		}

		/* Update existing stat */
		if (r->module_name == module_name && r->stat_name == stat_name) {
			r->packet_count++;
			r->byte_count += bytes;
			break;
		}
	}

	if (now >= rstats_report_cycle) {
		RateStatShow();
		rstats_report_cycle = now + RSTATS_REPORT_CYCLE_INTERVAL;
	}
}

void
RateStatTrigger(void)
{
	if (!rstats_triggered) {
		rstats_core_clock = cball_core_clock();
		rstats_report_cycle = cball_core_cycle() + RSTATS_REPORT_CYCLE_INTERVAL;
		rstats_triggered = 1;
	}
}
#endif /* EXT_CBALL */
