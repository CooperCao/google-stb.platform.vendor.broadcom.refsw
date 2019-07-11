/*
 * Shell-like utility functions
 *
 * Copyright (C) 2018, Broadcom Corporation. All Rights Reserved.
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
 * $Id: wlif_utils.h 669425 2016-11-09 12:26:48Z $
 */

#ifndef _wlif_utils_h_
#define _wlif_utils_h_

#include "bcmwifi_channels.h"
#include "proto/ethernet.h"
#include <wlioctl.h>

#ifndef IFNAMSIZ
#define IFNAMSIZ 16
#endif

#ifndef ETHER_ADDR_LEN
#define ETHER_ADDR_LEN 6
#endif

#define WLIFU_MAX_NO_BRIDGE		2
#define WLIFU_MAX_NO_WAN		2

#define MAX_USER_KEY_LEN	80			/* same as NAS_WKSP_MAX_USER_KEY_LEN */
#define MAX_SSID_LEN		32			/* same as DOT11_MAX_SSID_LEN */

typedef struct wsec_info_s {
	int unit;					/* interface unit */
	int ibss;					/* IBSS vs. Infrastructure mode */
	int gtk_rekey_secs;		/* group key rekey interval */
	int wep_index;			/* wep key index */
	int ssn_to;				/* ssn timeout value */
	int debug;				/* verbose - 0:no | others:yes */
	int preauth;				/* preauth */
	int auth_blockout_time;	/* update auth blockout retry interval */
	unsigned int auth;	/* shared key authentication optional (0) or required (1) */
	unsigned int akm;			/* authentication mode */
	unsigned int wsec;			/* encryption */
	unsigned int mfp;			/* mfp */
	unsigned int flags;			/* flags */
	char osifname[IFNAMSIZ];	/* interface name */
	unsigned char ea[ETHER_ADDR_LEN];			/* interface hw address */
	unsigned char remote[ETHER_ADDR_LEN];	/* wds remote address */
	unsigned short radius_port;				/* radius server port number */
	char ssid[MAX_SSID_LEN + 1];				/* ssid info */
	char psk[MAX_USER_KEY_LEN + 1];			/* user-supplied psk passphrase */
	char *secret;				/* user-supplied radius secret */
	char *wep_key;			/* user-supplied wep key */
	char *radius_addr;		/* radius server address */
	char *nas_id;			/* nas mac address */
} wsec_info_t;

#define WLIFU_WSEC_SUPPL			0x00000001	/* role is supplicant */
#define WLIFU_WSEC_AUTH			0x00000002	/* role is authenticator */
#define WLIFU_WSEC_WDS			0x00000004	/* WDS mode */

#define WLIFU_AUTH_RADIUS			0x20	/* same as nas_mode_t RADIUS in nas.h */

/* get wsec return code */
#define WLIFU_WSEC_SUCCESS			0
#define WLIFU_ERR_INVALID_PARAMETER	1
#define WLIFU_ERR_NOT_WL_INTERFACE	2
#define WLIFU_ERR_NOT_SUPPORT_MODE	4
#define WLIFU_ERR_WL_REMOTE_HWADDR	3
#define WLIFU_ERR_WL_WPA_ROLE		5

extern int get_wlname_by_mac(unsigned char *mac, char *wlname);
extern char *get_ifname_by_wlmac(unsigned char *mac, char *name);
extern int get_wsec(wsec_info_t *info, unsigned char *mac, char *osifname);
extern bool wl_wlif_is_psta(char *ifname);
extern bool wl_wlif_is_dwds(char *ifname);

/*
 * Routine for getting the nss value.
 * Params:
 * @bi: BSS info.
 */
extern int wl_wlif_get_max_nss(wl_bss_info_t* bi);
extern int get_bridge_by_ifname(char *ifname, char **brname);
extern int wl_wlif_wds_ap_ifname(char *ifname, char *apname);

#define WLIF_STRNCPY(dst, src, count) \
	do { \
		strncpy(dst, src, count - 1); \
		dst[count - 1] = '\0'; \
	} while (0)

#define WLIF_MAX_BUF			256

#ifdef CONFIG_HOSTAPD
// WPS states to update the UI
typedef enum wlif_wps_ui_status_code_id_ {
	WLIF_WPS_UI_INIT	= 0,
	WLIF_WPS_UI_ASSOCIATED	= 1,
	WLIF_WPS_UI_OK		= 2,
	WLIF_WPS_UI_ERR		= 3,
	WLIF_WPS_UI_TIMEOUT	= 4,
	WLIF_WPS_UI_PBCOVERLAP	= 5,
	WLIF_WPS_UI_FIND_PBC_AP	= 6
} wlif_wps_ui_status_code_id_t;

/* Flags to indicate the wps states */
#define WLIF_WPS_STARTED		0x1
#define WLIF_WPS_COMPLETED		0x2
#define WLIF_WPS_SUCCESS		0x4
#define WLIF_WPS_FAILED			0x8
#define WLIF_WPS_PBCOVERLAP		0x10
#define	WLIF_IS_WPS_STARTED(flags)	(flags & WLIF_WPS_STARTED)
#define	WLIF_IS_WPS_COMPLETED(flags)	(flags & WLIF_WPS_COMPLETED)
#define	WLIF_IS_WPS_SUCCEED(flags)	(flags & WLIF_WPS_SUCCESS)
#define	WLIF_IS_WPS_FAILED(flags)	(flags & WLIF_WPS_FAILED)
#define	WLIF_IS_WPS_PBCOVERLAP(flags)	(flags & WLIF_WPS_PBCOVERLAP)

/* Gets the status code from the wps_proc_status nvram value */
int wl_wlif_get_wps_status_code();
/* Compares the hostapd wps status str and updates the wps state flags */
int wl_wlif_hapd_wps_status(char *status_str, int sz, uint16 *flags);
/* Compares the wpa supplicant wps status str and updates the wps state flags */
int wl_wlif_supp_wps_status(char *status_str, int sz, uint16 *flags);

/* Different types of wps led blink types based on wps process state */
typedef enum wlif_wps_blinktype {
	WLIF_WPS_BLINKTYPE_INPROGRESS	= 0,
	WLIF_WPS_BLINKTYPE_ERROR	= 1,
	WLIF_WPS_BLINKTYPE_OVERLAP	= 2,
	WLIF_WPS_BLINKTYPE_SUCCESS	= 3,
	WLIF_WPS_BLINKTYPE_STOP		= 4
} wlif_wps_blinktype_t;

/* Different types of possible wps operations from UI like start, stop etc. */
typedef enum wlif_wps_op_type {
	WLIF_WPS_IDLE		= 0,
	WLIF_WPS_START		= 1,
	WLIF_WPS_RESTART	= 2,
	WLIF_WPS_STOP		= 3
} wlif_wps_op_type_t;

/* Different types of wps operating modes */
typedef enum wlif_wps_mode {
	WLIF_WPS_INVALID	= 0,
	WLIF_WPS_ENROLLEE	= 1,
	WLIF_WPS_REGISTRAR	= 2
} wlif_wps_mode_t;

/* Wps data */
typedef struct wlif_wps_ {
	char prefix[IFNAMSIZ];	/* Interface prefix of type wlx */
	char ifname[IFNAMSIZ];	/* Interface name of type ethx */
	char cmd[WLIF_MAX_BUF];	/* Hostapd/Wpa-Supplicant cli cmd */
	time_t start;		/* Wps start time */
	int board_fp;		/* Wps gpio file descriptor */
	wlif_wps_op_type_t op;	/* Wps operation like START/STOP */
	wlif_wps_mode_t mode;	/* Wps mode ENROLEE/REGISTRAR */
	int wait_thread;
} wlif_wps_t;

int wl_wlif_wps_check_status(wlif_wps_t *wps);
/* wps session timeout */
#define WLIF_WPS_TIMEOUT		120

#ifdef BCA_HNDROUTER
/* Routine for changing wps led status */
void wl_wlif_wps_gpio_led_blink(int board_fp, wlif_wps_blinktype_t blinktype);
int wl_wlif_wps_gpio_init();
void wl_wlif_wps_gpio_cleanup(int board_fp);
#else
#define	wl_wlif_wps_gpio_led_blink(a, b)	do {} while (0)
#define	wl_wlif_wps_gpio_init()		(0)
#define wl_wlif_wps_gpio_cleanup(a)		do {} while (0)
#endif	/* BCA_HNDROUTER */
#endif	/* CONFIG_HOSTAPD */

#endif /* _wlif_utils_h_ */
