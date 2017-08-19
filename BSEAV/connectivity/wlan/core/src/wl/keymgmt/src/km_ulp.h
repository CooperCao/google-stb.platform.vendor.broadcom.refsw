/**
 * @file
 * @brief
 * Keymgmt ULP  module interface.
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
 * $Id: km_ulp.h 564097 2015-06-16 14:00:38Z $
 */

#ifndef _km_ulp_h_
#define _km_ulp_h_

#include <km_key_pvt.h>
extern int km_ulp_p1_module_register(keymgmt_t *km);
extern int km_ulp_preattach(void);

#endif /* _KM_ULP_  */
