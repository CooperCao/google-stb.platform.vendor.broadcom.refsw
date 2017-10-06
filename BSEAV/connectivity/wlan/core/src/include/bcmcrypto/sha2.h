/*
 * sha2.h
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

/*
 * This header defines the API for sha1 and sha256   FIPS 180-2
 */

#ifndef _SHA2_H_
#define _SHA2_H_

#include <typedefs.h>
#ifdef BCMDRIVER
#include <osl.h>
#else
#include <stddef.h>
#endif


typedef enum {
	HASH_SHA1   = 1,
	HASH_SHA256   = 2,
	HASH_SHA384   = 3,
	HASH_SHA512   = 4,
	HASH_SHA512_224   = 5,
	HASH_SHA512_256   = 6
} sha2_hash_type_t;


typedef struct {
	void (*sha2_transform)(uint32 *state, uint8 *block);
	sha2_hash_type_t sha2_hash_type;
	uint32 state[90];
	uint8 block[64];
	uint32 len;
	uint32 ctr[2];
} sha2_context;

void sha2_update(sha2_context* ctx, uint8* data, uint32 dataLen);

void sha2_init(sha2_context* ctx);

void sha2_final(sha2_context* ctx, uint8* digest, int *digestLen);

void sha2_compute(sha2_hash_type_t hash_t, uint8* digest, int *digestLen,
		uint8* data, uint32 dataLen);

#endif /* _SHA2_H_ */
