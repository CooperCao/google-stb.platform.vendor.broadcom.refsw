/* 
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id$
 *
 * WL common functions -- Implement wl-related methods to communicate with driver,
 *    -- codes are created from linux\wpscli_wlan.c: refer join_network() and
 *       join_network_with_bssid() from linux\wpscli_wlan.c
 *    -- use 'wpsosi_' prefix to indicate the function can be shared across platforms.
 */
#include <ctype.h>
#include "wpscli_osl.h"
#include "bcmutils.h"
#include "wlioctl.h"
#include "wps_wl.h"
#include "tutrace.h"
#include "wpscli_common.h"

/* enable structure packing */
#if defined(__GNUC__)
#define PACKED __attribute__((packed))
#else
#pragma pack(1)
#define	PACKED
#endif

extern BOOL wpscli_iovar_set(const char *iovar, void *param, uint paramlen);
/* from linux's wpscli_wlan.c */
#define WLAN_JOIN_ATTEMPTS	3
#define WLAN_POLLING_JOIN_COMPLETE_ATTEMPTS	20
#define WLAN_POLLING_JOIN_COMPLETE_SLEEP	100
#define WLAN_JOIN_SCAN_DEFAULT_ACTIVE_TIME 20
#define WLAN_JOIN_SCAN_ACTIVE_TIME 60
#define WLAN_JOIN_SCAN_PASSIVE_TIME 150
#define WLAN_JOIN_SCAN_PASSIVE_TIME_LONG 2000

/* --------------------------------------------------------------------
* apply security for a STA
* --------------------------------------------------------------------
*/
int wpsosi_common_apply_sta_security(uint32 wsec)
{
	int auth = 0, infra = 1;
	int wpa_auth = WPA_AUTH_DISABLED;
	int ret = 0;

	/* set infrastructure mode */
	if ((ret = wpscli_wlh_ioctl_set(WLC_SET_INFRA,
		(const char *)&infra, sizeof(int))) < 0)
	{
		TUTRACE((TUTRACE_INFO,
			"wpsosi_common_apply_sta_security, WLC_SET_INFRA (%d) fail, ret=%d\n",
			infra, ret));
		return ret;
	}

	/* set authentication mode */
	if ((ret = wpscli_wlh_ioctl_set(WLC_SET_AUTH,
		(const char *)&auth, sizeof(int))) < 0)
	{
		TUTRACE((TUTRACE_INFO,
			"wpsosi_common_apply_sta_security, WLC_SET_AUTH (%d) fail, ret=%d\n",
			auth, ret));
		return ret;
	}

	/* set wsec mode */
	/*
	 * If wep bit is on,
	 * pick any WPA encryption type to allow association.
	 * Registration traffic itself will be done in clear (eapol).
	*/
	if (wsec)
		wsec = 2; /* TKIP */

	if ((ret = wpscli_wlh_ioctl_set(WLC_SET_WSEC,
		(const char *)&wsec, sizeof(int))) < 0)
	{
		TUTRACE((TUTRACE_INFO,
			"wpsosi_common_apply_sta_security, WLC_SET_WSEC (%d) fail, ret=%d\n",
			wsec, ret));
		return ret;
	}

	/* set WPA_auth mode */
	if ((ret = wpscli_wlh_ioctl_set(WLC_SET_WPA_AUTH,
		(const char *)&wpa_auth, sizeof(wpa_auth))) < 0)
	{
		TUTRACE((TUTRACE_INFO,
			"wpsosi_common_apply_sta_security, WLC_SET_WPA_AUTH(%d) fail, ret=%d\n",
			wpa_auth, ret));
		return ret;
	}

	return 0;

}

/* --------------------------------------------------------------------
 * Dump debug info for tracking a P2P-connect
 *
 * Input:
 *      funcName -- specify a caller-function.
 *      ssid     -- network's SSID
 *      wsec     -- security setting
 *      bssid    -- network's BSSID
 *      num_chanspecs -- number of chanspecs to scan for join the network
 *      chanspecss     -- an array of 'num_chanspecs'-entry chanspecs
 * --------------------------------------------------------------------
*/
static void trace_join(char *funcName, const char* ssid, uint32 wsec, const char *bssid,
	int num_chanspecs, chanspec_t *chanspecs)
{
	int i;
	TUTRACE((TUTRACE_INFO,
		"Entered: %s. ssid=[%s] wsec=%d #ch=%d\n",
		funcName, ssid ? ssid : "", wsec, num_chanspecs));

	/* show chanspecs */
	for (i = 0; i < num_chanspecs; i++)
	{
		if (i % 10 == 0)
			TUTRACE((TUTRACE_INFO, "\n"));
		TUTRACE((TUTRACE_INFO, "channel[%d]=0x%04x ", i, chanspecs[i]));
	}

	if (bssid)
	{
		TUTRACE((TUTRACE_INFO,
			"\nbssid=%02x:%02x:%02x:%02x:%02x:%02x",
			(uint8) bssid[0], (uint8) bssid[1], (uint8) bssid[2],
			(uint8) bssid[3], (uint8) bssid[4], (uint8) bssid[5]));
	}

	TUTRACE((TUTRACE_INFO, "\nExit trace_join()\n"));
}

/*
 * Manage WL parameters
*/

/* --------------------------------------------------------------------
 * setup 'bssid' & 'channel spec's field in join-param block
 * --------------------------------------------------------------------
*/
static void setup_join_assoc_params(const char *bssid, int num_chanspecs, chanspec_t *chanspecs,
	wl_join_assoc_params_t *params_t)
{
	int i;

	memset(params_t, 0, sizeof(*params_t));

	/* bssid (if any) */
	if (bssid)
		memcpy(&params_t->bssid, bssid, ETHER_ADDR_LEN);
	else
		memcpy(&params_t->bssid, &ether_bcast, ETHER_ADDR_LEN);

	/* channel spec */
	params_t->chanspec_num = num_chanspecs;
	for (i = 0; i < params_t->chanspec_num; i++) {
		params_t->chanspec_list[i] = chanspecs[i];
	}

	return;

}

/* --------------------------------------------------------------------
 * allocate and initialize a fixed ('non-extended') join_params block for 
 *      using WLC_SET_SSID ioctl to join a BSSID network.
 * Output:
 *   If succeeds, return a pointer of a join-params block allocated by 
 *          this function and the 'size of the buffer' in bytes. 
 *          Caller should 'free' this buffer once it is no longer needed.
 *   If failed, return a NULL pointer.
 * --------------------------------------------------------------------
*/
static wl_join_params_t *init_fixedJoin_params(const char* ssid, const uint8 *bssid,
	int num_chanspecs, chanspec_t *chanspecs,
	int *retJoinParamsSize)
{
	wl_join_params_t *join_params;
	int join_params_size;

	/* initialize */
	*retJoinParamsSize = 0;

	/* allocate a join-params block */
	join_params_size = WL_JOIN_PARAMS_FIXED_SIZE + num_chanspecs * sizeof(chanspec_t);
	if ((join_params = (wl_join_params_t *) malloc(join_params_size)) == NULL) {
		TUTRACE((TUTRACE_INFO, "init_fixedJoin_params: malloc failed"));
		return NULL;
	}
	memset(join_params, 0, join_params_size);

	/* setup 'ssid' wlc_ssid_t join-parameter */
	join_params->ssid.SSID_len = strlen(ssid);
	strncpy((char *)join_params->ssid.SSID, ssid, join_params->ssid.SSID_len);

	/* setup join-assoc-params (bssid, channel spec) */
	setup_join_assoc_params(bssid, num_chanspecs, chanspecs, &join_params->params);

	*retJoinParamsSize = join_params_size;	/* return 'size' in bytes */
	return join_params;
}

/* --------------------------------------------------------------------
 * allocate and initialize an 'extended' join_params block for using 'join' iovar 
 * to join the network.
 * Input:
 *       bPassive -- set to TRUE to allocate an extended join_params for a 'passive' join scan
 *                   set to FALSE to allocate an extended join_params for an 'active' join scan
 * Output:
 *   If succeeds, return a pointer of an extended join-params block allocated by this function and
 *          the 'size of the buffer' in bytes.
 *          Caller should 'free' this buffer once it is no longer needed.
 *   If failed, return a NULL pointer.
 * --------------------------------------------------------------------
*/
static wl_extjoin_params_t *init_extjoin_params(const char* ssid, const uint8 *bssid,
	int num_chanspecs, chanspec_t *chanspecs,
	BOOL bPassive, int *retJoinParamsSize)
{
	wl_extjoin_params_t *join_params;
	int join_params_size;
	wl_join_scan_params_t *scan_t;

	/* initialize */
	*retJoinParamsSize = 0;

	/* allocate an 'extended' join-params block */
	join_params_size = WL_EXTJOIN_PARAMS_FIXED_SIZE + num_chanspecs * sizeof(chanspec_t);
	if ((join_params = (wl_extjoin_params_t *) malloc(join_params_size)) == NULL) {
		TUTRACE((TUTRACE_INFO, "init_extjoin_params: malloc failed"));
		return NULL;
	}
	memset((void *)join_params, 0, join_params_size);

	/* setup 'ssid' wlc_ssid_t join-parameter */
	join_params->ssid.SSID_len = strlen(ssid);
	strncpy((char *)join_params->ssid.SSID, ssid, join_params->ssid.SSID_len);

	/* setup join-scan-parameter wl_join_scan_params_t */
	scan_t = &join_params->scan;
	scan_t->scan_type = bPassive ? 1 : DOT11_SCANTYPE_ACTIVE;
	scan_t->nprobes = -1;
	scan_t->active_time = bPassive ? -1 :  WLAN_JOIN_SCAN_ACTIVE_TIME;
	if (bPassive)
		scan_t->passive_time = (num_chanspecs == 1) ?
			WLAN_JOIN_SCAN_PASSIVE_TIME_LONG : WLAN_JOIN_SCAN_PASSIVE_TIME;
	else
		scan_t->passive_time = -1;
	scan_t->home_time = -1;

	/* setup join-assoc-params (bssid, channel spec) */
	setup_join_assoc_params(bssid, num_chanspecs, chanspecs, &join_params->assoc);

	*retJoinParamsSize = join_params_size;	/* return 'size' in bytes */
	return join_params;
}

/*
 * WL operations
*/

/* --------------------------------------------------------------------
 * Join a 'BSSID' use 'join' iovar with a join_params.
 *
 * Input:
 *      p_join_params -- caller-provided formated 'wlc_extjoin_params_t" block
 *                       used for 'join' iovar.
 *      join_params_size -- size of 'p_join_params' block in bytes.
 *      num_channels    -- number of channels.
 *
 * return 0 if succeeds, otherwise return -1.  
 * Note: Caller should have applied security settings before this function is called.
 * --------------------------------------------------------------------
*/
static int do_join_iovar(wl_extjoin_params_t *p_join_params, int join_params_size)
{
	/* use 'join' iovar */
	if (!wpscli_iovar_set("join", p_join_params, join_params_size))
	{
		TUTRACE((TUTRACE_INFO,
			"do_join_iovar: join network using join-iovar failed, buf_size=%d\n",
			join_params_size));
		return -1;	/* failed */

	}

	return 0;	/* success */
}

/* --------------------------------------------------------------------
 * Join a 'SSID' or a 'BSSID' using WLC_SET_SSID ioctl.
 *
 * return 0 if succeeds, otherwise return an error code
 *
 * Input:
 *      p_ioctl_buf -- caller-provided formated 'wlc_ssid_t" or "wl_join_params_t'
 *                     block for WLC_SET_SSID..
 *      ioctl_buf_size -- size of p_ioctl_buf in bytes

 *         to join using 'ssid'
 *            p_ioctl_buf -- points to a caller-formated 'wlc_ssid_t'
 *            (caller must set SSID and SSID_len properly).
 *            ioctl_buf_size -- size of 'p_ioctl_buf' in bytes (i.e. sizeof(wlc_ssid_t))
 *         to join using 'bssid',
 *            p_ioctl_buf -- points to a caller-formated non-extended 'wl_join_params_t' block
 *            ioctl_buf_size -- size of 'p_ioctl_buf' in bytes (i.e. = sizeof(wl_join_params_t))
 *
 * return 0 if succeeds, otherwise return an error code (< 0).       
 * Note: Caller should have applied security settings before this function is called.
 * --------------------------------------------------------------------
*/
static int do_join_ioctl(const char*p_ioctl_buf, int ioctl_buf_size)
{
	int ret = wpscli_wlh_ioctl_set(WLC_SET_SSID, p_ioctl_buf, ioctl_buf_size);

	if (ret < 0) {
		TUTRACE((TUTRACE_INFO,
			"do_join_ioctl: WLC_SET_SSID failed, p_ioctl_buf=%p, size=%d, errCode=%d\n",
			p_ioctl_buf, ioctl_buf_size, ret));

		return ret;
	}

	return ret;

}

/* --------------------------------------------------------------------
 * Join a SSID/BSSID using the WLC_SET_SSID ioctl or 'join' iovar, 
 * poll for the results until we got BSSID.
 *
 * return 0 if succeeds, otherwise return an error code
 *
 * Input:
 *      b_use_ioctl -- set to TRUE to join a SSID/BSSID using 'WLC_SET_SSID' ioctl,
 *                     set to FALSE ot use 'join' iovar.
 *
 *      1. if using 'WLC_SET_SSID" ioctl,
 *         a. to join using 'ssid'
 *            p_buffer -- points to a caller-formated 'wlc_ssid_t'
 *            (caller must set SSID and SSID_len properly).
 *            buf_size -- size of 'p_buffer' in bytes (i.e. sizeof(wlc_ssid_t))
 *         b. to join using 'bssid',
 *            p_buffer -- points to a caller-formated non-extended 'wl_join_params_t' block
 *            buf_size -- size of 'p_buffer' in bytes (i.e. = sizeof(wl_join_params_t))
 *
 *      2. if using 'join' iovar
 *            p_buffer -- points to a caller-formated extended 'wl_extjoin_params_t' block
 *            buf_size -- size of 'p_buffer' in bytes (i.e. = sizeof(wl_extjoin_params_t))
 *         
 *      join_scan_time: if specified (i.e. non-zero), the function will sleep for this time interval
 *            after requests the driver to perform a 'join' operation and then poll for the results
 *            to see if 'join' is completed. This only affects 'join' iovar.
 *
 * Note: Caller should have applied security settings before this function is called.
 * --------------------------------------------------------------------
*/
static int do_join_ioctl_or_iovar(BOOL b_use_ioctl, void *p_buffer, int buf_size,
	int join_scan_time)
{
	int ret = 0;
	char associated_bssid[6];
	int i, j;

#ifndef WL_EXTJOIN_PARAMS_FIXED_SIZE  /* if driver does not have "join" iovar */
	if (!b_use_ioctl)
	{
		TUTRACE((TUTRACE_INFO,
			"do_join_ioctl_or_iovar: failed, driver does not support join-iovar\n"));
		return -1;
	}
#endif

	for (i = 0; i < WLAN_JOIN_ATTEMPTS; i++) {
		/* Start the join */
		TUTRACE((TUTRACE_INFO,
			"do_join_ioctl_or_iovar join_network: use %s join-attempt=%d\n",
			b_use_ioctl ? "WLC_SET_SSID" : "iovar", i + 1));

		if (b_use_ioctl)
			ret = do_join_ioctl((void *) p_buffer, buf_size);
		else /* use 'join' iovar */
			ret = do_join_iovar((wl_extjoin_params_t *)p_buffer, buf_size);

		if (ret < 0)
			break;	/* abort if error */

		/* wait for the join scan time if use 'join' iovar */
		if (!b_use_ioctl && join_scan_time > 0)
		{
			TUTRACE((TUTRACE_INFO,
				"do_join_ioctl_or_iovar: sleep %d ms (join_scan_time)\n",
				join_scan_time));

			wpscli_sleep(join_scan_time);

			TUTRACE((TUTRACE_INFO,
				"do_join_ioctl_or_iovar: resume from join_scan_time\n"));
		}

		/* poll for the results until we got BSSID */
		for (j = 0; j < WLAN_POLLING_JOIN_COMPLETE_ATTEMPTS; j++) { /* max 20 times */

			TUTRACE((TUTRACE_INFO,
				"do_join_ioctl_or_iovar: sleep %d, poll for the result(loop=%d)\n",
				WLAN_POLLING_JOIN_COMPLETE_SLEEP, j));

			/* join time */
			wpscli_sleep(WLAN_POLLING_JOIN_COMPLETE_SLEEP);		/* sleep 100ms */

			TUTRACE((TUTRACE_INFO,
				"do_join_ioctl_or_iovar: get the result(WLC_GET_BSSID,loop=%d)\n",
				j));

			ret = wpscli_wlh_ioctl_get(WLC_GET_BSSID, associated_bssid, 6);

			/* exit if associated */
			if (ret == 0)
			{
				TUTRACE((TUTRACE_INFO,
					"do_join_ioctl_or_iovar: confirmed associated(loop=%d)\n",
					j));

				goto exit;
			}
			TUTRACE((TUTRACE_INFO,
				"do_join_ioctl_or_iovar: WLC_GET_BSSID ret=%d (not associated)\n",
				ret));
		}

	TUTRACE((TUTRACE_INFO,
		"do_join_ioctl_or_iovar: still not associated, ret=%d(not reset)\n", ret));
	}

exit:
	TUTRACE((TUTRACE_INFO, "do_join_ioctl_or_iovar: Exiting. ret=%d\n", ret));
	return ret;

}

/* --------------------------------------------------------------------
 * Join a BSSID using the WLC_SET_SSID ioctl, poll for the results until we got BSSID.
 *
 * return 0 if succeeds, otherwise return an error code
 * Note: Caller should have applied security settings before this function is called.
 * --------------------------------------------------------------------
*/
static int join_with_bssid_ioctl(const char* ssid, uint32 wsec, const char *bssid,
	int num_chanspecs, chanspec_t *chanspecs)
{
	int ret = 0;
	int join_params_size;
	wl_join_params_t *join_params;
	int join_scan_time;

	/* output log */
	trace_join("join_with_bssid_ioctl", ssid, wsec, bssid, num_chanspecs, chanspecs);

	/*
	 * allocate and intialize a 'non-extended' join_params block (either set 'ssid' or 'bssid')
	 */
	join_params_size = 0;
	if ((join_params = init_fixedJoin_params(ssid, bssid, num_chanspecs, chanspecs,
		&join_params_size)) == NULL) {
		TUTRACE((TUTRACE_INFO, "Exit: join_with_bssid_ioctl: malloc failed"));
		return -1;
	}

	/* Use the current secuirty settings set and use WLC_SET_SSID to join a BSSID */
	join_scan_time = 40 * num_chanspecs;
	ret = do_join_ioctl_or_iovar(TRUE, (void *) join_params, join_params_size, join_scan_time);

	free(join_params);

	TUTRACE((TUTRACE_INFO, "Exit: join_with_bssid_ioctl: WLC_SET_SSID ret=%d\n", ret));
	return ret;
}

/* --------------------------------------------------------------------
 * Join a BSSID using an 'active' or 'passive' join scan, poll for the results until we got BSSID.
 *
 * First tries using the "join" iovar.  If that is unsupported by the driver
 * then use the WLC_SET_SSID ioctl.
 *
 * Input:
 *      bPassiveJoin -- specify 'passive' or 'active' join scan used for
 *                  joining a BSSID using 'join' iovar if 'join' iovar is supported.
 *                  set to TRUE to use 'passive' join scan, set to FALSE to use 'active' join scan
 *              
 * return 0 if succeeds, otherwise return an error code
 * Note: Caller should have applied security settings before this function is called.
 * --------------------------------------------------------------------
 */
static int join_with_bssid_specific_scan(const char* ssid, uint32 wsec,
	const char *bssid, int num_chanspecs, chanspec_t *chanspecs,
	BOOL bPassive)
{
#ifdef WL_EXTJOIN_PARAMS_FIXED_SIZE  /* if driver has "join" iovar */
	int ret = 0;
	int join_params_size;
	wl_extjoin_params_t *join_params;
	BOOL bTryIoctl = FALSE;
	int join_scan_time;

	trace_join("join_with_bssid_specific_scan(use 'join' iovar)", ssid, wsec, bssid,
		num_chanspecs, chanspecs);

	/* allocate and intialize an 'extended' join_params block for a passive/active join scan */
	if ((join_params = init_extjoin_params(ssid, bssid, num_chanspecs, chanspecs,
		bPassive, &join_params_size)) == NULL) {
		TUTRACE((TUTRACE_INFO, "Exit: join_with_bssid_specific_scan: malloc failed"));
		return -1;
	}

	/* Use the current secuirty settings */
	/* adjust join scan params */
	join_scan_time = WLAN_JOIN_SCAN_DEFAULT_ACTIVE_TIME;
	if (bPassive)
		join_scan_time += join_params->scan.passive_time;
	else
		join_scan_time += join_params->scan.active_time;
	join_scan_time *= num_chanspecs;

	/* do join BSSID using 'join' iovar */
	ret = do_join_ioctl_or_iovar(FALSE, (void *) join_params, join_params_size, join_scan_time);

	/* If the "join" iovar is unsupported by the driver, 
	 * retry the join using the WLC_SET_SSID ioctl.
	*/
	if (ret == BCME_UNSUPPORTED) /* brcm_wpscli_ioctl_err */
		bTryIoctl = TRUE;

	free(join_params);

	if (!bTryIoctl)
	{
		TUTRACE((TUTRACE_INFO, "Exit: join_with_bssid_specific_scan: ret=%d\n", ret));
		return ret;
	}
#endif /* WL_EXTJOIN_PARAMS_FIXED_SIZE */

	trace_join("join_with_bssid_specific_scan(use ioctl)", ssid, wsec, bssid,
		num_chanspecs, chanspecs);

	/* no "join" iovar */
	return join_with_bssid_ioctl(ssid, wsec, bssid, num_chanspecs, chanspecs);

}

/* --------------------------------------------------------------------
 * Join a BSSID using a passive join scan, poll for the results until we got BSSID.
 *
 * First tries using the "join" iovar.  If that is unsupported by the driver
 * then use the WLC_SET_SSID ioctl.
 *
 * return 0 if succeeds, otherwise return an error code
 * Note: Caller should have applied security settings before this function is called.
 * --------------------------------------------------------------------
*/
static int join_with_bssid_passive(const char* ssid, uint32 wsec, const char *bssid,
	int num_chanspecs, chanspec_t *chanspecs)
{
	/* use passive-scan */
	return join_with_bssid_specific_scan(ssid, wsec, bssid, num_chanspecs, chanspecs, TRUE);
}

/* --------------------------------------------------------------------
 * Join a BSSID using an 'active' join scan, poll for the results until we got BSSID.
 *
 * First tries using the "join" iovar.  If that is unsupported by the driver
 * then use the WLC_SET_SSID ioctl.
 *
 * return 0 if succeeds, otherwise return an error code
 * Note: Caller should have applied security settings before this function is called.
 * --------------------------------------------------------------------
*/
static int join_with_bssid_active(const char* ssid, uint32 wsec,
	const char *bssid, int num_chanspecs, chanspec_t *chanspecs)
{
	/* use active-scan */
	return join_with_bssid_specific_scan(ssid, wsec, bssid, num_chanspecs, chanspecs, FALSE);
}

/* --------------------------------------------------------------------
 * Applies security settings and join a BSSID using the 'join' iovar,
 * poll for the results until we got BSSID.
 *
 * Return
 *   1. -1 if failed to apply security settings.
 *   2. WPS_STATUS_SUCCESS if succeeds
 *   3. WPS_STATUS_WLAN_CONNECTION_ATTEMPT_FAIL if failed
 * --------------------------------------------------------------------
*/
/* #if defined(WL_ASSOC_PARAMS_FIXED_SIZE) && defined(WL_JOIN_PARAMS_FIXED_SIZE) */
static int join_with_bssid_iovar(const char* ssid, uint32 wsec, const char *bssid,
	int num_chanspecs, chanspec_t *chanspecs)
{
	int ret;

	/* output log */
	trace_join("join_with_bssid_iovar", ssid, wsec, bssid, num_chanspecs, chanspecs);

	/* apply security settings */
	if ((ret = wpsosi_common_apply_sta_security(wsec)) != 0)
	{
		TUTRACE((TUTRACE_INFO, "join_with_bssid_iovar: apply_sta_security failed\n"));
		return -1;
	}

	/* attempt join the network with first channel */
	ret = join_with_bssid_active(ssid, wsec, bssid, num_chanspecs >= 1 ? 1 : 0, chanspecs);
	if (ret == WPS_STATUS_SUCCESS)
		return ret;

	ret = join_with_bssid_passive(ssid, wsec, bssid, num_chanspecs >= 1 ? 1 : 0, chanspecs);
	if (ret == WPS_STATUS_SUCCESS)
		return ret;

	/* attempt join the network with remaining chanspecs */
	if (num_chanspecs > 1 && chanspecs != NULL) {
		ret = join_with_bssid_active(ssid, wsec, bssid, num_chanspecs - 1, &chanspecs[1]);
		if (ret == WPS_STATUS_SUCCESS)
			return ret;

		ret = join_with_bssid_passive(ssid, wsec, bssid, num_chanspecs - 1, &chanspecs[1]);
		if (ret == WPS_STATUS_SUCCESS)
			return ret;
	}

	TUTRACE((TUTRACE_INFO, "Exit: join_with_bssid_iovar: failed\n"));
	return WPS_STATUS_WLAN_CONNECTION_ATTEMPT_FAIL;
}
/* #endif defined(WL_ASSOC_PARAMS_FIXED_SIZE) && defined(WL_JOIN_PARAMS_FIXED_SIZE) */

/* --------------------------------------------------------------------
 * Join the network
 * Applies security settings and join a SSID using WLC_SET_SSID ioctl,
 * poll for the results until we got BSSID.
 * Return
 *   1. -1 if failed to apply security settings.
 *   2. WPS_STATUS_SUCCESS if succeeds
 *   3. WPS_STATUS_WLAN_CONNECTION_ATTEMPT_FAIL if failed
 * --------------------------------------------------------------------
*/
brcm_wpscli_status wpsosi_join_network(char* ssid, uint32 wsec)
{
	int ret = 0;
	wlc_ssid_t ssid_t;

	TUTRACE((TUTRACE_INFO, "Entered: wpsosi_join_network. ssid=[%s] wsec=%d\n", ssid, wsec));

	if ((ret = wpsosi_common_apply_sta_security(wsec)) != 0)
	{
		TUTRACE((TUTRACE_INFO, "wpsosi_join_network: apply_sta_security failed\n"));
		return -1;
	}

	/* setup 'ssid' parameter */
	memset(&ssid_t, 0, sizeof(ssid_t));
	ssid_t.SSID_len = strlen(ssid);
	strncpy((char *)ssid_t.SSID, ssid, ssid_t.SSID_len);

	/* use WLC_SET_SSID ioctl to join a network (& ignore join-scan-time) */
	/* Poll for the results once a second until we got BSSID */
	ret = do_join_ioctl_or_iovar(TRUE, (void *) &ssid_t, sizeof(ssid_t), 0);

	TUTRACE((TUTRACE_INFO, "wpsosi_join_network: Exiting. ret=%d\n", ret));
	return ret;
}

/* --------------------------------------------------------------------
 * Join the network:
 *   Applies security settings and join a SSID/BSSID network using
 *   IOCTL or using the 'join' iovar, poll for the results until we got BSSID.
 * Return
 *   1. -1 if failed to apply security settings.
 *   2. WPS_STATUS_SUCCESS if succeeds
 *   3. WPS_STATUS_WLAN_CONNECTION_ATTEMPT_FAIL if failed
 * --------------------------------------------------------------------
*/
brcm_wpscli_status wpsosi_join_network_with_bssid(const char* ssid, uint32 wsec, const uint8 *bssid,
	int num_chanspecs, chanspec_t *chanspecs)
{
#if !defined(WL_ASSOC_PARAMS_FIXED_SIZE) || !defined(WL_JOIN_PARAMS_FIXED_SIZE)
	return (wpsosi_join_network(ssid, wsec));
#else
	return join_with_bssid_iovar(ssid, wsec, bssid, num_chanspecs, chanspecs);
#endif /* !defined(WL_ASSOC_PARAMS_FIXED_SIZE) || !defined(WL_JOIN_PARAMS_FIXED_SIZE) */

}

/* --------------------------------------------------------------------
 * disconnect wlan connection
 * ---------------------------------------------------------------------
*/
brcm_wpscli_status wpsosi_leave_network(void)
{
	brcm_wpscli_status status = wpscli_wlh_ioctl_set(WLC_DISASSOC, NULL, 0);

	TUTRACE((TUTRACE_INFO, "wpsosi_leave_network, STATUS=%d\n", status));

	return status;

}
