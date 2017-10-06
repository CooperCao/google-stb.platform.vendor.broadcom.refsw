/*
 * bcm_ec.h
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
 *  <<Broadcom-WL-IPTag/Open:>>
 */

/* This header defines the API support elliptic curve crypto. */

#ifndef _BCM_EC_H_
#define _BCM_EC_H_

#include <typedefs.h>
#ifdef BCMDRIVER
#include <osl.h>
#else
#include <stddef.h>
#endif

#include <bcm_bn.h>

/* allocation support */
typedef bn_alloc_fn_t ecg_alloc_fn_t;
typedef bn_free_fn_t ecg_free_fn_t;
typedef bn_rand_fn_t ecg_rand_fn_t;

/* opaque elliptic curve group - can be EC(Prime) or EC(2^N) */
typedef struct ecg ecg_t;

/* groups we understand, but may or may not support. Group abstracts
 * curve parameters etc.
 */
enum {
	ECG_NIST_P256 = 1,	/* FIPS 186-4 - Appendix D */
	ECG_IPSEC_G19 = 1,	/* ipsec-registry - same as NIST P256 */
	ECG_NIST_P384 = 2,	/* FIPS 186-4 - Appendix D */
	ECG_IPSEC_G20 = 2,	/* ipsec-registry - same as NIST P384 */
	ECG_NIST_P521 = 3,	/* FIPS 186-4 - Appendix D */
	ECG_IPSEC_G21 = 3,	/* ipsec-registry - same as NIST P521 */

	/* Curve 25519 */
	ECG_25519	= 4,	/* D.J. Bernstein - p = 2^255 - 19 */

	/* Additional curves above this line */
	ECG_MAX
};
typedef int ecg_type_t;

/* create an ec group */
ecg_t* ecg_alloc(ecg_type_t type, ecg_alloc_fn_t alloc_fn, ecg_free_fn_t free_fn, void *ctx);

/* free an ecg, reset ecg to NULL */
void ecg_free(ecg_t **ecg);

/* many params re big nums - abstract the API to provide those extensibly */
enum {
	ECG_PARAM_BN_PRIME = 1,
	ECG_PARAM_BN_ORDER = 2,
	ECG_PARAM_BN_COFACTOR = 3,
	ECG_PARAM_BN_A = 4,
	ECG_PARAM_BN_B = 5,
	ECG_PARAM_BN_BIT_LEN = 6,
	ECG_PARAM_BN_BYTE_LEN = 7,

	/* add bn parameters above, as necessary */
	ECG_PARAM_BN_MAX
};
typedef int16 ecg_param_type_t;

int ecg_get_bn_param(ecg_t *ecg, ecg_param_type_t type, bn_t *bn);

/* group element */
typedef struct ecg_elt ecg_elt_t;

/* allocate an ec group element */
ecg_elt_t* ecg_elt_alloc(ecg_t *ecg);

/* free an ec group element */
void ecg_elt_free(ecg_elt_t **elt);

/* get elt coordinates. x or y may be NULL */
void ecg_elt_get_xy(const ecg_elt_t *elt, const bn_t **x, const bn_t **y);

/* initialize elt with coordiantes - may return error if not an elt */
int ecg_elt_init(ecg_elt_t *elt, const bn_t *x, const bn_t *y);

/* convenient define */
#define ecg_elt_set_xy(_e, _x, _y) ecg_elt_init(_e, _x, _y)

/* generate a random element in the group */
ecg_elt_t* ecg_elt_rand(ecg_t *ecg, ecg_rand_fn_t rand_fn, void* rand_ctx);

/* element add c = a + b */
void ecg_elt_add(const ecg_elt_t *elt_a, const ecg_elt_t *elt_b, ecg_elt_t *elt_c);

/* elt double b = 2 * a */
void ecg_elt_dbl(const ecg_elt_t *elt_a, ecg_elt_t *elt_b);

/* inverse element */
void ecg_elt_inv(const ecg_elt_t *elt, ecg_elt_t *inv);

/* scalar multiply res = bn * elt */
void ecg_elt_mul(const ecg_elt_t *elt, const bn_t *bn, ecg_elt_t *res);

/* point at infinity */
const ecg_elt_t* ecg_elt_infinity(ecg_t *ecg);

/* copy an element */
void ecg_elt_copy(ecg_elt_t *dst, const ecg_elt_t *src);

/* equality */
bool ecg_elt_eq(const ecg_elt_t *e1, const ecg_elt_t *e2);

/* on curve */
bool ecg_is_member(const ecg_t *ecg, const bn_t *x, const bn_t *y);

/* get base element */
const ecg_elt_t* ecg_get_base_elt(const ecg_t *ecg);

/* get ecg type */
ecg_type_t ecg_get_type(const ecg_t *ecg);

/* get ecg from elt */
ecg_t* ecg_elt_get_group(const ecg_elt_t *elt);

#endif /* _BCM_EC_H_ */
