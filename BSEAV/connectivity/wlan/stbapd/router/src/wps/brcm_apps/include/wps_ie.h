/*
 * WPS IE
 *
 * Copyright (C) 2018, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: wps_ie.h 383928 2013-02-08 04:15:06Z $
 */

#ifndef __WPS_IE_H__
#define __WPS_IE_H__

int wps_ie_default_ssr_info(CTlvSsrIE *ssrmsg, unsigned char *authorizedMacs,
	int authorizedMacs_len, BufferObj *authorizedMacs_Obj, unsigned char *wps_uuid,
	BufferObj *uuid_R_Obj, uint8 scState);
void wps_ie_set(char *wps_ifname, CTlvSsrIE *ssrmsg);
void wps_ie_clear();


#endif	/* __WPS_IE_H__ */
