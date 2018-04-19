/*
 * bsd scheme  (Linux)
 *
 * $Copyright Broadcom Corporation$
 *
 * $Id: bsd_default.c $
 */
#include "bsd.h"

/* The default policy for 2.4G and 5G dualband boards
 *   steer STAs to 2.4G RF from 5G RF if 5G is oversubscribed
 *   steer STAs to 5G RF from 2.4G RF if 5G is undersubscribed
 *
 * The STA selection is based on phyrate
 */

#if 0 /* leave here as sample BSD nvram settings */
/* nvram config for eth1(5G), eth2(2.4G) ref. board e.g. BCM94706nr2hmc */
static struct nvram_tuple bsd_5g2g_policy[] = {
	{"bsd_ifnames", "eth1 eth2", 0},
	{"bsd_scheme", "2", 0},
	{"wl0_bsd_steering_policy", "80 5 3 0 0 0x40", 0},
	{"wl1_bsd_steering_policy", "0 5 3 0 0 0x50", 0},
	{"wl0_bsd_sta_select_policy", "0 0 0 0 0 1 0 0 0 0x240", 0},
	{"wl1_bsd_sta_select_policy", "0 0 0 0 0 1 0 0 0 0x240", 0},
	{"wl0_bsd_if_select_policy", "eth2", 0},
	{"wl1_bsd_if_select_policy", "eth1", 0},
	{"wl0_bsd_if_qualify_policy", "20 0x0 -75", 0},
	{"wl1_bsd_if_qualify_policy", "0 0x0 -75", 0},
	{"bsd_bounce_detect", "180 2 3600", 0},
	{0, 0, 0}
};

/* nvram config for eth1(2.4G), eth2(5G) ref. board e.g. BCM94708r */
static struct nvram_tuple bsd_2g5g_policy[] = {
	{"bsd_ifnames", "eth1 eth2", 0},
	{"bsd_scheme", "2", 0},
	{"wl0_bsd_steering_policy", "0 5 3 0 0 0x50", 0},
	{"wl1_bsd_steering_policy", "80 5 3 0 0 0x40", 0},
	{"wl0_bsd_sta_select_policy", "0 0 0 0 0 1 0 0 0 0x240", 0},
	{"wl1_bsd_sta_select_policy", "0 0 0 0 0 1 0 0 0 0x240", 0},
	{"wl0_bsd_if_select_policy", "eth2", 0},
	{"wl1_bsd_if_select_policy", "eth1", 0},
	{"wl0_bsd_if_qualify_policy", "0 0x0 -75", 0},
	{"wl1_bsd_if_qualify_policy", "20 0x0 -75", 0},
	{"bsd_bounce_detect", "180 2 3600", 0},
	{0, 0, 0}
};

/* The default policy for 3 RFs,, eth1(5G Lo), eth2(2.4G), eth3(5G Hi) board
 *   STAs are steered between 5G Low, and 5G High RFs
 *   STAs with phyrate less than 300Mbps are steered to 5G High
 *   STAs with phyrate greater than or equal to 300Mbps are steered to 5G Low
 *
 *   In case of 5G Low oversubscription, BSD steers STAs to 5G High from 5G Low
 *
 * The STA selection is based on phyrate, and is not 4Kbps active
 */
/* e.g. BCM94709acdcrh */
static struct nvram_tuple bsd_5glo_2g_5ghi_policy[] = {
	{"bsd_ifnames", "eth1 eth2 eth3", 0},
	{"bsd_scheme", "2", 0},
	{"wl0_bsd_steering_policy", "80 5 3 0 300 0x40", 0},
	{"wl2_bsd_steering_policy", "0 5 3 0 400 0x60", 0},
	{"wl0_bsd_sta_select_policy", "4 0 300 0 0 1 0 0 0 0x40", 0},
	{"wl2_bsd_sta_select_policy", "4 0 400 0 0 -1 0 0 0 0x60", 0},
	{"wl0_bsd_if_select_policy", "eth3", 0},
	{"wl2_bsd_if_select_policy", "eth1", 0},
	{"wl0_bsd_if_qualify_policy", "60 0x0 -75", 0},
	{"wl2_bsd_if_qualify_policy", "0 0x0 -75", 0},
	{"bsd_bounce_detect", "180 2 3600", 0},
	{0, 0, 0}
};
#endif /* sample BSD nvram settings */
