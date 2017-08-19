/**
 * @file
 * @brief
 * ULP module header file
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

#ifndef __wlc_ulp_h__
#define __wlc_ulp_h__
#include <ulp.h>

#define ULP_CFG_IDX_INVALID (-1)
#define WL_ULP_DBG(args)
#define WL_ULP_ERR(args)	printf args

/* ULP Enter Timer Related enums  */
enum {
	ULP_TIMER_START = 1,
	ULP_TIMER_STOP	= 2,
	};

/* ulp extended info, used to pass to ulp enter, to calculate cubby sizes and
 * to aid for assoc recreate.
 */
struct ulp_ext_info {
	int wowl_cfg_ID; /* id of cfg which is triggering wowl/ulp mode */
};
extern wlc_ulp_info_t *wlc_ulp_attach(wlc_info_t *wlc);
extern void wlc_ulp_detach(wlc_ulp_info_t *ulp_info);
extern int wlc_ulp_preattach(void);
extern bool wlc_is_ulp_pending(wlc_ulp_info_t *ulp_info);
extern int wlc_ulp_wowl_check(wlc_ulp_info_t *ulp_info, uint16 id);
extern int wlc_ulp_enter_wowl_trigger(wlc_ulp_info_t *ulp_info, wlc_bsscfg_t *cfg);
extern int wlc_ulp_assoc_recreate(wlc_ulp_info_t *ulp_info);
extern int wlc_ulp_pre_ulpucode_switch(wlc_ulp_info_t *ulp_info, wlc_bsscfg_t *cfg);
extern int wlc_ulp_post_ulpucode_switch(wlc_ulp_info_t *ulp_info, wlc_bsscfg_t *cfg);
extern void wlc_ulp_configure_ulp_features(wlc_ulp_info_t *ulp_info, uint32 val, uint32 enable);
extern int wlc_ulp_perform_sleep_ctrl_action(wlc_ulp_info_t *ui, uint8 action);
extern bool wlc_ulp_allow_wowl_force(wlc_ulp_info_t *ui);

#endif /* _wlc_ulp_h_ */
