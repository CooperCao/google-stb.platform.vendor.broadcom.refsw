/*
 * WBD related include file
 *
 * $Copyright Broadcom Corporation$
 *
 *
 * <<Broadcom-WL-IPTag/Proprietary:>>
 *
 * $Id: bsd_wbd.h 642646 2016-06-09 12:13:08Z spalanga $
 */

#ifndef _BSD_WBD_H_
#define _BSD_WBD_H_

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>

#include <security_ipc.h>
#include <typedefs.h>

#define BSD_WBD_REQ_BUFSIZE	256

/* Loopback IP address */
#define BSD_WBD_LOOPBACK_IP	"127.0.0.1"
#define BSD_WBD_SERVERT_PORT	(EAPD_WKSP_WBD_UDP_PORT + 0x103)

/* WiFi Blanket related NVRAMS */
#define BSD_WBD_NVRAM_MODE		"wbd_mode"
#define BSD_WBD_NVRAM_IFNAMES		"wbd_ifnames"
#define BSD_WBD_NVRAM_WEAK_STA_ALGO	"wbd_wc_algo"
#define BSD_WBD_NVRAM_WEAK_STA_POLICY	"wbd_weak_sta_policy"
#define BSD_WBD_NVRAM_WEAK_STA_CFG	"wbd_weak_sta_cfg"

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

/* WiFi Blanket Mode (Master, Slave or Disabled) */
typedef enum {
	WBD_MODE_UNDEFINED = -1,
	WBD_MODE_DISABLED = 0,
	WBD_MODE_MASTER,
	WBD_MODE_SLAVE
} bsd_wbd_mode_t;

#define BSD_WBD_DISABLED(mode) (((mode) == WBD_MODE_DISABLED) || ((mode) == WBD_MODE_UNDEFINED))

/* bit masks for WBD weak_sta_policy flags */
#define BSD_WBD_WEAK_STA_POLICY_FLAG_RULE		0x00000001	/* logic AND chk */
#define BSD_WBD_WEAK_STA_POLICY_FLAG_ACTIVE_STA		0x00000002	/* Active STA */
#define BSD_WBD_WEAK_STA_POLICY_FLAG_RSSI		0x00000004	/* RSSI */
#define BSD_WBD_WEAK_STA_POLICY_FLAG_PHYRATE		0x00000008	/* Phyrate */
#define BSD_WBD_WEAK_STA_POLICY_FLAG_TXFAILURE		0x00000010	/* Tx Failure */

typedef struct bsd_bssinfo bsd_bssinfo_t;
typedef struct bsd_info bsd_info_t;

/* Rules and Threshold parameters for finding weak clients */
typedef struct bsd_wbd_weak_sta_policy {
	int idle_rate;		/* data rate threshold to measure STA is idle */
	int rssi;		/* rssi threshold */
	uint32 phyrate;		/* phyrate threshold in Mbps */
	int tx_failures;	/* threshold for tx retry */
	uint32 flags;		/* extension flags (Rules) */
} bsd_wbd_weak_sta_policy_t;

/* List of bss info on which WiFi Blanket is enabled */
typedef struct bsd_wbd_bss_list {
	uint8 algo;					/* Which find weak STA algorithm to use */
	uint8 policy;					/* Which policy to use to find weak sta */
	bsd_wbd_weak_sta_policy_t *weak_sta_cfg;	/* Configuration of the policy chosen */
	bsd_bssinfo_t *bssinfo;				/* Pointer to BSS info structure on which
							 * WBD is enabled
							 */
	struct bsd_wbd_bss_list *next;
} bsd_wbd_bss_list_t;

/* Information regarding WiFi Blanket */
typedef struct bsd_wbd_info {
	bsd_wbd_bss_list_t *bss_list;	/* List of BSS on which WBD is enabled */
} bsd_wbd_info_t;

/* Extern Declarations */
extern int bsd_wbd_set_ifnames(bsd_info_t *info);
extern int bsd_wbd_init(bsd_info_t *info);
extern void bsd_cleanup_wbd(bsd_wbd_info_t *info);
extern void bsd_wbd_check_weak_sta(bsd_info_t *info);

#endif /*  _BSD_WBD_H_ */
