/*
 * sha2x.c
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
 * This header defines the API for 64-bit sha   FIPS 180-4
 */

#include <stdlib.h>
#include <string.h>

#include <sha2x.h>

static const int szSTATE = 64;
static const int szBLOCK = 128;

static const uint64 kk[80] = {
	0x428a2f98d728ae22, 0x7137449123ef65cd, 0xb5c0fbcfec4d3b2f, 0xe9b5dba58189dbbc,
	0x3956c25bf348b538, 0x59f111f1b605d019, 0x923f82a4af194f9b, 0xab1c5ed5da6d8118,
	0xd807aa98a3030242, 0x12835b0145706fbe, 0x243185be4ee4b28c, 0x550c7dc3d5ffb4e2,
	0x72be5d74f27b896f, 0x80deb1fe3b1696b1, 0x9bdc06a725c71235, 0xc19bf174cf692694,
	0xe49b69c19ef14ad2, 0xefbe4786384f25e3, 0x0fc19dc68b8cd5b5, 0x240ca1cc77ac9c65,
	0x2de92c6f592b0275, 0x4a7484aa6ea6e483, 0x5cb0a9dcbd41fbd4, 0x76f988da831153b5,
	0x983e5152ee66dfab, 0xa831c66d2db43210, 0xb00327c898fb213f, 0xbf597fc7beef0ee4,
	0xc6e00bf33da88fc2, 0xd5a79147930aa725, 0x06ca6351e003826f, 0x142929670a0e6e70,
	0x27b70a8546d22ffc, 0x2e1b21385c26c926, 0x4d2c6dfc5ac42aed, 0x53380d139d95b3df,
	0x650a73548baf63de, 0x766a0abb3c77b2a8, 0x81c2c92e47edaee6, 0x92722c851482353b,
	0xa2bfe8a14cf10364, 0xa81a664bbc423001, 0xc24b8b70d0f89791, 0xc76c51a30654be30,
	0xd192e819d6ef5218, 0xd69906245565a910, 0xf40e35855771202a, 0x106aa07032bbd1b8,
	0x19a4c116b8d2d0c8, 0x1e376c085141ab53, 0x2748774cdf8eeb99, 0x34b0bcb5e19b48a8,
	0x391c0cb3c5c95a63, 0x4ed8aa4ae3418acb, 0x5b9cca4f7763e373, 0x682e6ff3d6b2b8a3,
	0x748f82ee5defb2fc, 0x78a5636f43172f60, 0x84c87814a1f0ab72, 0x8cc702081a6439ec,
	0x90befffa23631e28, 0xa4506cebde82bde9, 0xbef9a3f7b2c67915, 0xc67178f2e372532b,
	0xca273eceea26619c, 0xd186b8c721c0c207, 0xeada7dd6cde0eb1e, 0xf57d4f7fee6ed178,
	0x06f067aa72176fba, 0x0a637dc5a2c898a6, 0x113f9804bef90dae, 0x1b710b35131c471b,
	0x28db77f523047d84, 0x32caab7b40c72493, 0x3c9ebe0a15c9bebc, 0x431d67c49c100d4c,
	0x4cc5d4becb3e42b6, 0x597f299cfc657e2a, 0x5fcb6fab3ad6faec, 0x6c44198c4a475817,
};

static const uint64 sha512_224_init[8] = {
	0x8C3D37C819544DA2, 0x73E1996689DCD4D6, 0x1DFAB7AE32FF9C82, 0x679DD514582F9FCF,
	0x0F6D2B697BD44DA8, 0x77E36F7304C48942, 0x3F9D85A86A1D36C8, 0x1112E6AD91D692A1
};

static const uint64 sha512_256_init[8] = {
	0x22312194FC2BF72C, 0x9F555FA3C84C64C2, 0x2393B86B6F53B151, 0x963877195940EABD,
	0x96283EE2A88EFFE3, 0xBE5E1E2553863992, 0x2B0199FC2C85B8AA, 0x0EB72DDC81C52CA2
};

static const uint64 sha384_init[8] = {
	0xcbbb9d5dc1059ed8, 0x629a292a367cd507, 0x9159015a3070dd17, 0x152fecd8f70e5939,
	0x67332667ffc00b31, 0x8eb44a8768581511, 0xdb0c2e0d64f98fa7, 0x47b5481dbefa4fa4
};

static const uint64 sha512_init[8] = {
	0x6A09E667F3BCC908, 0xBB67AE8584CAA73B, 0x3C6EF372FE94F82B, 0xA54FF53A5F1D36F1,
	0x510E527FADE682D1, 0x9B05688C2B3E6C1F, 0x1F83D9ABFB41BD6B, 0x5BE0CD19137E2179
};

#define Ch(x, y, z)	((x & y) ^ (~x & z))
#define Maj(x, y, z)	((x & y) ^ (x & z) ^ (y & z))
#define Rot(x, n)	((x >> n) | (x << (64-n)))
#define Sum0(x)		(Rot(x, 28) ^ Rot(x, 34) ^ Rot(x, 39))
#define Sum1(x)		(Rot(x, 14) ^ Rot(x, 18) ^ Rot(x, 41))
#define Sigma0(x)	(Rot(x, 1) ^ Rot(x, 8) ^ (x >> 7))
#define Sigma1(x)	(Rot(x, 19)^ Rot(x, 61) ^ (x >> 6))

static
void sha2_transform(uint64* state, uint8* M)
{
	uint64 T1;
	uint64* hh = &state[88];
	int j = 0;

	memcpy(hh, state, 64);
	while (j < 16) {
		hh--;
		T1 = hh[8] + Sum1(hh[5]) + Ch(hh[5], hh[6], hh[7]) + kk[j++];
		T1 += hh[8] = ((uint64)(M[0]<< 24 | M[1]<< 16 | M[2]<< 8 | M[3]) << 32) |
				M[4]<< 24 | M[5]<< 16 | M[6]<< 8 | M[7];
		M += 8;
		hh[4] += T1;
		hh[0] = T1 + Sum0(hh[1]) + Maj(hh[1], hh[2], hh[3]);
	}
	while (j < 80) {
		hh--;
		T1 = hh[8] + Sum1(hh[5]) + Ch(hh[5], hh[6], hh[7]) + kk[j++];
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
void Int2uint8(uint8* b, uint64* c, int count)
{
	int j, k, n = count >> 1;
	for (j = 0; j < n; j++) {
		k = c[j] >> 32;
		b[0] = (uint8)(k >> 24);
		b[1] = (uint8)(k >> 16);
		b[2] = (uint8)(k >> 8);
		b[3] = (uint8)k;
		k = c[j];
		b[4] = (uint8)(k >> 24);
		b[5] = (uint8)(k >> 16);
		b[6] = (uint8)(k >> 8);
		b[7] = (uint8)k;
		b += 8;
	}
	if (count & 1) {
		k = c[j] >> 32;
		b[0] = (uint8)(k >> 24);
		b[1] = (uint8)(k >> 16);
		b[2] = (uint8)(k >> 8);
		b[3] = (uint8)k;
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
			sha2_transform(ctx->state, ctx->block);
			data += szBLOCK - ctx->len;
			lenData -= szBLOCK - ctx->len;
		}
		while (lenData >= szBLOCK) {
			sha2_transform(ctx->state, data);
			data += szBLOCK;
			lenData -= szBLOCK;
		}
		memcpy(ctx->block, data, lenData);
		ctx->len = lenData;
	}
}


void sha2_final(sha2_context* ctx, uint8* digest, int* digestLen)
{
	if (ctx->sha2_hash_type == HASH_SHA512_224)
		*digestLen = 28;
	else
	if (ctx->sha2_hash_type == HASH_SHA512_256)
		*digestLen = 32;
	else
	if (ctx->sha2_hash_type == HASH_SHA384)
		*digestLen = 48;
	else
	if (ctx->sha2_hash_type == HASH_SHA512)
		*digestLen = 64;

	ctx->block[ctx->len++] = 0x80;
	if (ctx->len > 112) {
		memset(ctx->block + ctx->len, 0, szBLOCK - ctx->len);
		sha2_transform(ctx->state, ctx->block);
		ctx->len = 0;
	}
	memset(ctx->block + ctx->len, 0, 112 - ctx->len);
	ctx->ctr[0] = ctx->ctr[0]<<3 | ctx->ctr[1]>>61;
	ctx->ctr[1] <<= 3;
	Int2uint8(ctx->block+112, ctx->ctr, 4);
	sha2_transform(ctx->state, ctx->block);

	Int2uint8(digest, ctx->state, *digestLen >> 2);
}


void sha2_init(sha2_context* ctx)
{
	int sz = 64;
	if (ctx->sha2_hash_type == HASH_SHA512_224)
		memcpy(ctx->state, sha512_224_init, sz);
	else
	if (ctx->sha2_hash_type == HASH_SHA512_256)
		memcpy(ctx->state, sha512_256_init, sz);
	else
	if (ctx->sha2_hash_type == HASH_SHA384)
		memcpy(ctx->state, sha384_init, sz);
	else
	if (ctx->sha2_hash_type == HASH_SHA512)
		memcpy(ctx->state, sha512_init, sz);

	ctx->len = 0;
	ctx->ctr[0] = 0;
	ctx->ctr[1] = 0;
}


void sha2_compute(sha2_hash_type_t hash_t, uint8* digest, int* digestLen,
		uint8* data, uint64 dataLen)
{
	sha2_context ctx;
	ctx.sha2_hash_type = hash_t;
	sha2_init(&ctx);
	sha2_update(&ctx, data, dataLen);
	sha2_final(&ctx, digest, digestLen);
}
