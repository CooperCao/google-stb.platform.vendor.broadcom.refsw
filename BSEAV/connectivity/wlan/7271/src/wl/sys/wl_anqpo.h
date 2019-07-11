/*
 * ANQP Offload
 *
 * $Copyright Broadcom Corporation$
 *
 * $Id: wl_anqpo.h 708017 2017-06-29 14:11:45Z pkethini $
 */

/**
 *
 * ANQP Offload is a feature which allows the dongle to perform ANQP queries (using 802.11u GAS) to
 * devices and have the ANQP response returned to the host using an event notification. Any query
 * using ANQP such as hotspot and service discovery may be sent using the offload.
 *
 * Twiki: [OffloadsPhase2]
 */

#ifndef _wl_anqpo_h_
#define _wl_anqpo_h_

#include <wlc_cfg.h>
#include <d11.h>
#include <wlc_types.h>
#include <wl_gas.h>

typedef struct wl_anqpo_info wl_anqpo_info_t;

/* Bit definitions to figure out which addr has been programmed to amt */
#define	MAC_ADDR_RANDOM		(1 << 0)
#define MAC_ADDR_DEFAULT	(1 << 1)

/*
 * Initialize anqpo private context.
 * Returns a pointer to the anqpo private context, NULL on failure.
 */
extern wl_anqpo_info_t *wl_anqpo_attach(wlc_info_t *wlc, wl_gas_info_t *gas);

/* Cleanup anqpo private context */
extern void wl_anqpo_detach(wl_anqpo_info_t *anqpo);

/* initialize on scan start */
extern void wl_anqpo_scan_start(wl_anqpo_info_t *anqpo);

/* deinitialize on scan stop */
extern void wl_anqpo_scan_stop(wl_anqpo_info_t *anqpo);

/* process scan result */
extern void wl_anqpo_process_scan_result(wl_anqpo_info_t *anqpo,
	wlc_bss_info_t *bi, uint8 *ie, uint32 ie_len, int8 cfg_idx);

extern struct ether_addr * wl_anqpo_get_mac(wl_anqpo_info_t *anqpo);

bool wl_anqpo_is_anqp_active(wl_anqpo_info_t *anqpo);

void wl_anqpo_update_mac_addr_mask(wl_anqpo_info_t *anqpo,
	uint8 mac_addr_mask, bool set);

void wl_anqpo_update_src_mac(wl_anqpo_info_t *anqpo, struct ether_addr *src_mac);

bool
wl_anqpo_auto_hotspot_enabled(wl_anqpo_info_t *anqpo);

extern bool wl_anqpo_bssid_wildcard_isset(wl_anqpo_info_t *anqpo,
	struct ether_addr *peer_addr);
#endif /* _wl_anqpo_h_ */
