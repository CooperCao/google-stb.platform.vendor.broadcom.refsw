/*
 * NOA based Coex related header file
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * <<Broadcom-WL-IPTag/Proprietary:>>
 * $Id$
*/

#ifndef _wlc_cxnoa_h_
#define _wlc_cxnoa_h_


#include <wlc_types.h>
#ifdef BCMCOEXNOA
/* APIs */
extern wlc_cxnoa_info_t *wlc_cxnoa_attach(wlc_info_t *wlc);
extern void wlc_cxnoa_detach(wlc_cxnoa_info_t *cxnoa);
#endif /* BCMCOEXNOA */
#endif /* _wlc_cxnoa_h_ */
