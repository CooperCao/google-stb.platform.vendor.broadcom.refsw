/*
 * bcm_ecdsa.h
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
 *	<<Broadcom-WL-IPTag/Open:>>
 */

/*
 * This header defines the API support elliptic curve dsa FIPS 186-4/ANS X9.62
 * Also see  - http://cs.ucsb.edu/~koc/ccs130h/notes/ecdsa-cert.pdf
 */

#ifndef _BCM_EC_DSA_H_
#define _BCM_EC_DSA_H_

#include <typedefs.h>
#ifdef BCMDRIVER
#include <osl.h>
#else
#include <stddef.h>
#endif

#include <bcm_ec.h>

typedef struct ecdsa ecdsa_t;


/* allocation support */
typedef bn_alloc_fn_t ecdsa_alloc_fn_t;
typedef bn_free_fn_t ecdsa_free_fn_t;

/* RNG support - note: must be approved RNG if FIPS - used for key pair
 * generation and per-message random
 */
typedef bn_rand_fn_t ecdsa_rand_fn_t;

/* hash function support; this is explicit rather than a callback since
 * ecdsa restricts hash functions that may be used. hash function
 * typically is (equivalent to)
 *		bn_t* (*ecdsa_hash_fn_t)(bn_ctx_t *ctx, uint8 msg, int msg_len)
 * See FIPS 180-4
 */
enum {
	ECDSA_HASH_SHA1		= 1,
	ECDSA_HASH_SHA256	= 2,
	ECDSA_HASH_SHA384	= 3,
	ECDSA_HASH_SHA512	= 4,

	ECDSA_HASH_SHA512_224	= 5,
	ECDSA_HASH_SHA512_256	= 6,

	/* add others */
	ECDSA_HASH_MAX
};
typedef int8 ecdsa_hash_type_t;

/* signature includes public key, per message random and verifier */
struct ecdsa_signature {
	ecg_elt_t *pub;
	bn_t *r;
	bn_t *s;
};
typedef struct ecdsa_signature ecdsa_signature_t;

/* allocate an ecdsa context */
ecdsa_t* ecdsa_alloc(ecg_t *ecg, ecdsa_hash_type_t hash_type,
	ecdsa_rand_fn_t rand_fn, void *rand_ctx, bn_t *priv /* optional */,
	bn_ctx_t *bn_ctx);

/* free an ecdsa context and clear arg and any keys */
void ecdsa_free(ecdsa_t **ecdsa);

/* generate key pair - see FIPS 186-4 Appendix B.4
 * if *priv is NULL, a random key is generated, returns pub, public key
 */
int ecdsa_gen_keys(ecdsa_t *ecdsa, bn_t **priv, ecg_elt_t *pub);

/* sign a message of given length in bytes, r, s must already be allocated  */
int ecdsa_sign(ecdsa_t *ecdsa, const uint8 *msg, int msg_len, ecdsa_signature_t *signture);

/* verify a message of given length in bytes  - BCME_OK on valid signature */
int ecdsa_verify(ecdsa_t *ecdsa, const uint8 *msg, int msg_len,
	const ecdsa_signature_t *signature);

/* no encryption w/ ecdsa */

/* no support for exporting private key */

#endif /* _BCM_EC_DSA_H_ */
