/* Health check module
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * <<Broadcom-WL-IPTag/Proprietary:>>
 *
 * $Id$
 */

/* This has to hold module specifc call back
 */

#ifndef _HEALTH_CHECK_H
#define _HEALTH_CHECK_H

typedef int (*health_check_module_fn)(uint8 *buf, uint16 len,
                void *module_context_ptr, uint16 *bytes_written);

typedef struct health_check_client_info health_check_client_info_t;

extern health_check_info_t* wlc_health_check_attach(wlc_info_t *wlc);
extern void wlc_health_check_detach(health_check_info_t *hc);
extern int wlc_health_check_register_client(health_check_info_t *hc,
		health_check_module_fn fn, void *module_context_ptr,
		int module_id, uint16 buf_len);

extern int wlc_health_check_dregister_clients(health_check_client_info_t *client_info);

#endif /* _HEALTH_CHECK_H */
