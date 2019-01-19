/*
 * WBD related include file
 *
 * $Copyright Broadcom Corporation$
 *
 *
 * <<Broadcom-WL-IPTag/Proprietary:>>
 *
 * $Id: bsd_wbd.h 765842 2018-07-18 11:07:25Z sp888952 $
 */

#ifndef _BSD_WBD_H_
#define _BSD_WBD_H_

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>

#include <security_ipc.h>
#include <typedefs.h>

#include "wbd_rc_shared.h"

#define BSD_WBD_REQ_BUFSIZE		512

/* Loopback IP address */
#define BSD_WBD_LOOPBACK_IP		"127.0.0.1"
#define BSD_WBD_SERVERT_PORT		(EAPD_WKSP_WBD_UDP_PORT + 0x103)

/* WiFi Blanket related NVRAMS */
#define BSD_WBD_NVRAM_MAP_MODE		"multiap_mode"
#define BSD_WBD_NVRAM_IFNAMES		"wbd_ifnames"
#define BSD_WBD_NVRAM_WEAK_STA_ALGO	"wbd_wc_algo"
#define BSD_WBD_NVRAM_WEAK_STA_POLICY	"wbd_weak_sta_policy"
#define BSD_WBD_NVRAM_WEAK_STA_CFG	"wbd_weak_sta_cfg"
#define BSD_WBD_NVRAM_MCHAN_SLAVE       "wbd_mchan"

/* STA status flags */
#define BSD_WBD_STA_WEAK_PENDING	0x0001	/* STA is weak and reported to slave */
#define BSD_WBD_STA_WEAK		0x0002	/* STA is weak and accepted by master */
#define BSD_WBD_STA_DWDS		0x0004	/* STA is DWDS STA, no need to check for weak STA */
#define BSD_WBD_STA_IGNORE		0x0008	/* Ignore the STA from steering */
#define BSD_WBD_STA_DWELL		0x0010	/* STA is in dwell period */

/* WBD's error codes */
#define BSDE_WBD_OK			100
#define BSDE_WBD_FAIL			101
#define BSDE_WBD_IGNORE_STA		102
#define BSDE_WBD_NO_SLAVE_TO_STEER	103
#define BSDE_WBD_BOUNCING_STA		104
#define BSDE_WBD_UN_ASCSTA		105

/* MulitAP Modes */
#define MAP_MODE_FLAG_DISABLED		0x0000	/* Disabled */
#define MAP_MODE_FLAG_CONTROLLER	0x0001	/* Controller */
#define MAP_MODE_FLAG_AGENT		0x0002	/* Agent */

#define BSD_WBD_DISABLED(mode) (((mode) <= MAP_MODE_FLAG_DISABLED))

typedef struct bsd_bssinfo bsd_bssinfo_t;
typedef struct bsd_info bsd_info_t;

/* List of bss info on which WiFi Blanket is enabled */
typedef struct bsd_wbd_bss_list {
	uint8 algo;					/* Which find weak STA algorithm to use */
	uint8 policy;					/* Which policy to use to find weak sta */
	wbd_weak_sta_policy_t *weak_sta_cfg;		/* Configuration of the policy chosen */
	bsd_bssinfo_t *bssinfo;				/* Pointer to BSS info structure on which
							 * WBD is enabled
							 */
	int wbd_band_type;				/* WBD Band Type fm Chanspec & Bridge */

	struct bsd_wbd_bss_list *next;
} bsd_wbd_bss_list_t;

/* Information regarding WiFi Blanket */
typedef struct bsd_wbd_info {
	bsd_wbd_bss_list_t *bss_list;	/* List of BSS on which WBD is enabled */
} bsd_wbd_info_t;

/* Extern Declarations */
extern int bsd_wbd_set_ifnames(bsd_info_t *info);
extern int bsd_wbd_init(bsd_info_t *info);
extern void bsd_wbd_reinit(bsd_info_t *info);
extern void bsd_cleanup_wbd(bsd_wbd_info_t *info);
extern void bsd_wbd_check_weak_sta(bsd_info_t *info);
extern void bsd_wbd_update_bss_info(bsd_info_t *info, char *ifname);
#endif /*  _BSD_WBD_H_ */
