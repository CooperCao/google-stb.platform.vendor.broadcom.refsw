/*
 * bcm_rand.h
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: $
 *
 *   <<Broadcom-WL-IPTag/Open:>>
 */

#ifndef _BCM_RAND_H_
#define _BCM_RAND_H_

#include <typedefs.h>
#ifdef BCMDRIVER
#include <osl.h>
#else
#include <stddef.h>
#endif

/* crypto strengith rng */
void randomize(uint16* buf, int buf_len, uint16* key, int key_len, int (*rng)(void *ctx));

#endif /* _BCM_RAND_H_ */
