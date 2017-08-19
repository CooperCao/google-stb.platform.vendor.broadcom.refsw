/*
 * sha2.c
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

#include <stdlib.h>
#include <string.h>

#include <sha2.h>

static const int szBLOCK = 64;

/***************   S H A - 1   **************************** */

static const uint32 sha1_Ka = 0x5A827999;
static const uint32 sha1_Kb = 0x6ED9EBA1;
static const uint32 sha1_Kc = 0x8F1BBCDC;
static const uint32 sha1_Kd = 0xCA62C1D6;

static const uint32 sha1_D[5] = {
	0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476, 0xC3D2E1F0
};

#define Ch(x, y, z)	((x & y) ^ (~x & z))
#define Xor(x, y, z) (x ^ y ^ z)
#define Maj(x, y, z)	((x & y) ^ (x & z) ^ (y & z))
#define ROL5(x)	((x << 5) | (x >> 27))
#define ROL(x)	((x << 1) | (x >> 31))
#define ROR2(x) ((x >> 2) | (x << 30))

static
void sha1_transform(uint32* state, uint8* M)
{
	uint32* hh = &state[85];
	uint32* hLow = hh - 16;

	memcpy(hh, state, 20);
	do {
		hh--;
		hh[0] = ROL5(hh[1]) + Ch(hh[2], hh[3], hh[4]) + hh[5] + sha1_Ka;
		hh[0] += hh[5] = M[0]<<24 | M[1]<<16 | M[2]<<8 | M[3];
		M += 4;
		hh[2] = ROR2(hh[2]);
	} while (hh > hLow);

	hLow -= 4;
	do {
		hh--;
		hh[0] = ROL5(hh[1]) + Ch(hh[2], hh[3], hh[4]) + hh[5] + sha1_Ka;
		hh[0] += hh[5] = ROL((hh[8] ^ hh[13] ^ hh[19] ^ hh[21]));
		hh[2] = ROR2(hh[2]);
	} while (hh > hLow);

	hLow -= 20;
	do {
		hh--;
		hh[0] = ROL5(hh[1]) + Xor(hh[2], hh[3], hh[4]) + hh[5] + sha1_Kb;
		hh[0] += hh[5] = ROL((hh[8] ^ hh[13] ^ hh[19] ^ hh[21]));
		hh[2] = ROR2(hh[2]);
	} while (hh > hLow);

	hLow -= 20;
	do {
		hh--;
		hh[0] = ROL5(hh[1]) + Maj(hh[2], hh[3], hh[4]) + hh[5] + sha1_Kc;
		hh[0] += hh[5] = ROL((hh[8] ^ hh[13] ^ hh[19] ^ hh[21]));
		hh[2] = ROR2(hh[2]);
	} while (hh > hLow);

	hLow -= 20;
	do {
		hh--;
		hh[0] = ROL5(hh[1]) + Xor(hh[2], hh[3], hh[4]) + hh[5] + sha1_Kd;
		hh[0] += hh[5] = ROL((hh[8] ^ hh[13] ^ hh[19] ^ hh[21]));
		hh[2] = ROR2(hh[2]);
	} while (hh > hLow);

	state[0] += hh[0];
	state[1] += hh[1];
	state[2] += hh[2];
	state[3] += hh[3];
	state[4] += hh[4];
}


/***************   S H A - 2 5 6   ************************ */

static const uint32 sha256_K[64] = {
	0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
	0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
	0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
	0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
	0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
	0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
	0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
	0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
	0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
	0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
	0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
	0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
	0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
	0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
	0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
	0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

static const uint32 sha256_D[8] = {
	0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
	0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
};

#define Rot(x, n)	((x >> n) | (x << (32-n)))
#define Sum0(x)		(Rot(x, 2) ^ Rot(x, 13) ^ Rot(x, 22))
#define Sum1(x)		(Rot(x, 6) ^ Rot(x, 11) ^ Rot(x, 25))
#define Sigma0(x)	(Rot(x, 7) ^ Rot(x, 18) ^ (x >> 3))
#define Sigma1(x)	(Rot(x, 17)^ Rot(x, 19) ^ (x >> 10))

static
void sha256_transform(uint32* state, uint8* M)
{
	uint32 T1;
	uint32* hh = &state[72];
	int j = 0;

	memcpy(hh, state, 32);
	while (j < 16) {
		hh--;
		T1 = hh[8] + Sum1(hh[5]) + Ch(hh[5], hh[6], hh[7]) + sha256_K[j++];
		T1 += hh[8] = M[0]<<24 | M[1]<<16 | M[2]<<8 | M[3];
		M += 4;
		hh[4] += T1;
		hh[0] = T1 + Sum0(hh[1]) + Maj(hh[1], hh[2], hh[3]);
	}

	while (j < 64) {
		hh--;
		T1 = hh[8] + Sum1(hh[5]) + Ch(hh[5], hh[6], hh[7]) + sha256_K[j++];
		T1 += hh[8] = Sigma1(hh[10]) + hh[15] + Sigma0(hh[23]) + hh[24];
		hh[4] += T1;
		hh[0] = T1 + Sum0(hh[1]) + Maj(hh[1], hh[2], hh[3]);
	}

	state[0] += hh[0];
	state[1] += hh[1];
	state[2] += hh[2];
	state[3] += hh[3];
	state[4] += hh[4];
	state[5] += hh[5];
	state[6] += hh[6];
	state[7] += hh[7];
}

static
void Int2uint8(uint8* b, uint32* c, int count)
{
	int j, k;
	for (j = 0; j < count; j++) {
		k = c[j];
		b[0] = (uint8)(k >> 24);
		b[1] = (uint8)(k >> 16);
		b[2] = (uint8)(k >> 8);
		b[3] = (uint8)k;
		b += 4;
	}
}


void sha2_update(sha2_context* ctx, uint8* data, uint32 lenData)
{
	ctx->ctr[1] += lenData;
	if (ctx->ctr[1] < lenData)
		ctx->ctr[0]++;

	if (ctx->len + lenData < szBLOCK) {
		memcpy(ctx->block + ctx->len, data, lenData);
		ctx->len += lenData;
	} else {
		if (ctx->len > 0) {
			memcpy(ctx->block + ctx->len, data, szBLOCK - ctx->len);
			ctx->sha2_transform(ctx->state, ctx->block);
			data += szBLOCK - ctx->len;
			lenData -= szBLOCK - ctx->len;
		}
		while (lenData >= szBLOCK) {
			ctx->sha2_transform(ctx->state, data);
			data += szBLOCK;
			lenData -= szBLOCK;
		}
		memcpy(ctx->block, data, lenData);
		ctx->len = lenData;
	}
}


void sha2_final(sha2_context* ctx, uint8* digest, int* digestLen)
{
	*digestLen = (ctx->sha2_hash_type == HASH_SHA1) ? 20 : 32;
	ctx->block[ctx->len++] = 0x80;
	if (ctx->len > 56) {
		memset(ctx->block + ctx->len, 0, szBLOCK - ctx->len);
		ctx->sha2_transform(ctx->state, ctx->block);
		ctx->len = 0;
	}
	memset(ctx->block + ctx->len, 0, 56 - ctx->len);
	ctx->ctr[0] = ctx->ctr[0]<<3 | ctx->ctr[1]>>29;
	ctx->ctr[1] <<= 3;
	Int2uint8(ctx->block+56, ctx->ctr, 2);
	ctx->sha2_transform(ctx->state, ctx->block);

	Int2uint8(digest, ctx->state, *digestLen / 4);
}


void sha2_init(sha2_context* ctx)
{
	if (ctx->sha2_hash_type == HASH_SHA1) {
		ctx->sha2_transform = sha1_transform;
		memcpy(ctx->state, sha1_D, 20);
	} else {
		ctx->sha2_transform = sha256_transform;
		memcpy(ctx->state, sha256_D, 32);
	}
	ctx->len = 0;
	ctx->ctr[0] = 0;
	ctx->ctr[1] = 0;
}


void sha2_compute(sha2_hash_type_t hash_t, uint8* digest, int* digestLen,
		uint8* data, uint32 dataLen)
{
	sha2_context ctx;
	ctx.sha2_hash_type = hash_t;
	sha2_init(&ctx);
	sha2_update(&ctx, data, dataLen);
	sha2_final(&ctx, digest, digestLen);
}
