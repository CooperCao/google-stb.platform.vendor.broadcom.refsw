/*
 * Miscellaneous module interface (to other PHY modules).
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

#ifndef _phy_misc_h_
#define _phy_misc_h_

#include <phy_api.h>
#include <phy_dbg.h>

/* WAR to address low/mid band RSSI bias for X14 after Rx gain error calibration */
#define X14_5G_LOWBAND_RSSI_OFFSET 3
#define X14_5G_MIDBAND_RSSI_OFFSET 3

/* forward declaration */
typedef struct phy_misc_info phy_misc_info_t;

/* attach/detach */
phy_misc_info_t *phy_misc_attach(phy_info_t *pi);
void phy_misc_detach(phy_misc_info_t *cmn_info);

/* up/down */
int phy_misc_init(phy_misc_info_t *cmn_info);
int phy_misc_down(phy_misc_info_t *cmn_info);

/* Inter-module interfaces and downward interfaces to PHY type specific implemenation */
#if defined(BCMDBG) || defined(WLTEST)
int wlc_phy_test_init(phy_info_t *pi, int channel, bool txpkt);
int wlc_phy_test_stop(phy_info_t *pi);
int wlc_phy_test_freq_accuracy(phy_info_t *pi, int channel);
#endif	/* defined(BCMDBG) || defined(WLTEST) */
uint32 wlc_phy_rx_iq_est(phy_info_t *pi, uint8 samples, uint8 antsel, uint8 resolution,
	uint8 lpf_hpc, uint8 dig_lpf, uint8 gain_correct,
                                uint8 extra_gain_3dB, uint8 wait_for_crs, uint8 force_gain_type);
#if defined(WLTEST) || defined(ATE_BUILD)
void wlc_phy_iovar_tx_tone(phy_info_t *pi, int32 int_val);
void wlc_phy_iovar_txlo_tone(phy_info_t *pi);
#endif 
int wlc_phy_iovar_get_rx_iq_est(phy_info_t *pi, int32 *ret_int_ptr, int32 int_val, int err);
int wlc_phy_iovar_set_rx_iq_est(phy_info_t *pi, void *p, int32 plen, int err);
int wlc_phy_iovar_get_rx_iq_est_sweep(phy_info_t *pi, void *p, int32 plen, void *a, int32 alen,
	struct wlc_if *wlcif, int err);
#ifdef PHY_DUMP_BINARY
int phy_misc_getlistandsize(phy_info_t *pi, phyradregs_list_t **phyreglist, uint16 *phyreglist_sz);
#endif
bool wlc_phy_get_rxgainerr_phy(phy_info_t *pi, int16 *gainerr);
#endif /* _phy_misc_h_ */
