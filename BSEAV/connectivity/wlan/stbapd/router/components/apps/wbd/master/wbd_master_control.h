/*
 * WBD Steering Policy declarations
 *
 * $ Copyright Broadcom Corporation $
 *
 *
 * <<Broadcom-WL-IPTag/Proprietary:>>
 *
 * $Id: wbd_master_control.h 765398 2018-07-02 05:34:49Z pn952104 $
 */

#ifndef _WBD_MASTER_CONTROL_H_
#define _WBD_MASTER_CONTROL_H_

#include "wbd.h"
#include "wbd_shared.h"
#include "wbd_ds.h"

/* Get the number of weightage policies defined */
extern int wbd_get_max_tbss_wght();

/* Get the number of threshold policies defined for 2G */
extern int wbd_get_max_tbss_thld_2g();

/* Get the number of threshold policies defined for 5G */
extern int wbd_get_max_tbss_thld_5g();

/* Get the number of schemes defined */
extern int wbd_get_max_tbss_algo();

/* Get the weightage policy based on index */
extern wbd_tbss_wght_t *wbd_get_tbss_wght(wbd_master_info_t *master_info);

/* Get the threshold policy based on index for 2G */
extern wbd_tbss_thld_t *wbd_get_tbss_thld_2g(wbd_master_info_t *master_info);

/* Get the threshold policy based on index for 5G */
extern wbd_tbss_thld_t *wbd_get_tbss_thld_5g(wbd_master_info_t *master_info);

/* Create the timer to identify the target BSS for weak STA */
int wbd_master_create_identify_target_bss_timer(wbd_master_info_t *master_info,
	wbd_tbss_timer_args_t *args);

/* Send steer req msg */
void wbd_master_send_steer_req(unsigned char *al_mac, unsigned char *sta_mac,
	unsigned char *bssid, i5_dm_bss_type *tbss);
#endif /* _WBD_MASTER_CONTROL_H_ */
