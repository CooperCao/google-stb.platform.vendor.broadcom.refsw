/*
 * bcm_ecdh.h
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
 * <<Broadcom-WL-IPTag/Open:>>
 */

/* This header defines the API support elliptic curve diffie-hellman FIPS SP800-56Ar2 */

#ifndef _BCM_ECDH_H_
#define _BCM_ECDH_H_

#include <typedefs.h>
#ifdef BCMDRIVER
#include <osl.h>
#else
#include <stddef.h>
#endif

#include <bcm_ec.h>

/* allocation support */
typedef bn_rand_fn_t ecdh_rand_fn_t;

/* generate ecdh ephemeral keys */
int ecdh_gen_keys(ecg_t *ecg,  ecg_elt_t *pub, bn_t **priv,
	ecdh_rand_fn_t rand_fn, void* rand_ctx, bn_ctx_t *bnx);

/* generate a shared secret using local private key and peer public key */
int ecdh_gen_secret(ecg_t *ecg, const bn_t *priv,
	const ecg_elt_t *r_pub, bn_t *secret);

#endif /* _BCM_ECDH_H_ */
