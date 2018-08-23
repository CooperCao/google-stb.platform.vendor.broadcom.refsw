/*
 * Wireless Application Event Service
 * shared header file
 *
 * Copyright (C) 2018, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 * $Id: $
 */

#ifndef _appeventd_h_
#define _appeventd_h_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <typedefs.h>
#include <bcmnvram.h>
#include <bcmutils.h>
#include <bcmtimer.h>
#include <bcmendian.h>
#include <shutils.h>
#include <security_ipc.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <appevent_hdr.h>

extern int appeventd_debug_level;

#define APPEVENTD_DEBUG_ERROR	0x0001
#define APPEVENTD_DEBUG_WARNING	0x0002
#define APPEVENTD_DEBUG_INFO	0x0004
#define APPEVENTD_DEBUG_DETAIL	0x0008

#define APPEVENTD_ERROR(fmt, arg...) \
		do { if (appeventd_debug_level & APPEVENTD_DEBUG_ERROR) \
			printf("APPEVENTD >> "fmt, ##arg); } while (0)

#define APPEVENTD_WARNING(fmt, arg...) \
		do { if (appeventd_debug_level & APPEVENTD_DEBUG_WARNING) \
			printf("APPEVENTD >> "fmt, ##arg); } while (0)

#define APPEVENTD_INFO(fmt, arg...) \
		do { if (appeventd_debug_level & APPEVENTD_DEBUG_INFO) \
			printf("APPEVENTD >> "fmt, ##arg); } while (0)

#define APPEVENTD_DEBUG(fmt, arg...) \
		do { if (appeventd_debug_level & APPEVENTD_DEBUG_DETAIL) \
			printf("APPEVENTD >> "fmt, ##arg); } while (0)

#define APPEVENTD_BUFSIZE	2048

#define APPEVENTD_OK	0
#define APPEVENTD_FAIL -1

/* WiFi Application Event ID */
#define APP_E_BSD_STEER_START 1  /* status: STEERING */
#define APP_E_BSD_STEER_END   2  /* status: SUCC/FAIL */
#define APP_E_BSD_STATS_QUERY 3  /* status: STA/RADIO */
#define APP_E_WBD_SLAVE_WEAK_CLIENT 4  /* status: SUCC */
#define APP_E_WBD_SLAVE_STEER_START 5  /* status: SUCC */
#define APP_E_WBD_SLAVE_STEER_RESP  6  /* status: SUCC */
#define APP_E_WBD_MASTER_WEAK_CLIENT 7  /* status: SUCC */
#define APP_E_WBD_MASTER_STEER_START 8  /* status: SUCC */
#define APP_E_WBD_MASTER_STEER_RESP  9  /* status: SUCC */
#define APP_E_WBD_MASTER_STEER_END   10  /* status: SUCC */
#endif /* _appeventd_h_ */
