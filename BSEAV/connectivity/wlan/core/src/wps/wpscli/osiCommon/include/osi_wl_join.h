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
 *
 */
#ifndef _OSI_WL_JOIN_H_
#define _OSI_WL_JOIN_H_

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------
 *
 *
 *--------------------------------------------------------------------
*/
int wpsosi_common_apply_sta_security(uint32 wsec);


/*--------------------------------------------------------------------
 *
 *
 *--------------------------------------------------------------------
*/
brcm_wpscli_status wpsosi_join_network(char* ssid, uint32 wsec);


/*--------------------------------------------------------------------
 *
 *
 *--------------------------------------------------------------------
*/
brcm_wpscli_status wpsosi_join_network_with_bssid(const char* ssid, uint32 wsec, const uint8 *bssid,
	int num_chanspec, chanspec_t *chanspec);

/*--------------------------------------------------------------------
 *
 *
 *--------------------------------------------------------------------
*/
brcm_wpscli_status wpsosi_leave_network(void);

#ifdef __cplusplus
} // extern "C" {
#endif

#endif /* _OSI_WL_JOIN_H_ */
