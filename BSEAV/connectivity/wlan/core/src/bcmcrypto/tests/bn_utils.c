/* bn_utils.c
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include <bcm_rand.h>

#include "bn_utils.h"

static
void printScalar(char* label, uint8* a, int len_bytes)
{
	int k;
	printf("%-10s: ", label);
	for (k=0; k<len_bytes; k++)
		printf(" %02x", a[k]);
	puts("");
}


void Print(char* title, const bn_t* b)
{
	uint8 out[150];
	int SZ = bn_get_len_bytes(b, BN_FMT_LE);
	bn_get(b, BN_FMT_BE, out, SZ);
	printScalar(title, out, SZ);
}


bn_t* get_rand_key(bn_ctx_t* bnx)
{
	int j, k, length;
	char line[120];
	uint8 buf[32];
	bn_t* rKey;
	FILE* fKey = fopen("test_bn.key","r");
	if (!fKey)
		return 0;

	fgets(line, 120, fKey);

	sscanf(line, "%d", &length);
	for (j=0,k=3; j<length; j++, k+=3) {
		sscanf(line+k, "%x", (int *)(buf+j));
	}
	fclose(fKey);

	rKey = bn_alloc(bnx, BN_FMT_BE, buf, length);
	//Print(" rand key", rKey);
	return rKey;
}


void set_rand_key(bn_t* rKey)
{
	int j, length;
	uint8 buf[32];
	FILE* fKey = fopen("test_bn.key","w");
	if (!fKey)
		return ;

	length = bn_get_len_bytes(rKey, BN_FMT_LE);
	bn_get(rKey, BN_FMT_BE, buf, length);

	fprintf(fKey, "%02d", length);
	for (j=0; j<length; j++)
		fprintf(fKey, " %02x", buf[j]);
	fputs("\n", fKey);
	fclose(fKey);
	//Print(" rand key", rKey);
}


void shalom_clear(uint8 *s, int size)
{
	memset(s, 0, size);
	*(int *)s = size;
	*(int *)&s[4] = 8; // offset
}


void* shalom_alloc(void* base, int size)
{
	uint8* s = (uint8 *)base;
	uint8 *b;
	int max = *(int*)s;
	int *offset = (int *)&s[4];
	if (*offset + size > max) {
		printf( "Error -- space needed: %d  space available: %d\n", size, max - *offset);
		return 0;
	}
	b = base + *offset;
	*offset += size;
	return b;
}


void shalom_free(void* ctx, void *ptr, int size)
{
	uint8 *s = (uint8 *)ctx;
	int *offset = (int *)&s[4];

	if (ptr + size != s + *offset) {
		printf( "Error -- top block not freed, blocks may be out of sequence\n");
		return;
	}

	memset(ptr, 0, size);
	*offset -= size;
}

/*
const uint32 mPrime = 0xfffffffb; // 2^32 - 5

uint32 m32_inverse(uint32 a)
{
	uint32 k, k2 = mPrime;
	uint32 k1 = a;
	uint32 j, j1 = 0, j2 = 1;
	while(1) {
		k = k2 / k1;
		k2 = k2 % k1;
		j1 += j2 * k;
		if (k2 <= 1)
			return mPrime - j1;

		k = k1 / k2;
		k1 = k1 % k2;
		j2 += j1 * k;
		if (k1 <= 1)
			return j2;
	}
}
*/

int m32_rand(void *ctx)
{
	uint16* b = (uint16 *)ctx;
	uint32 c, s1, s2;
	uint32 c1 = b[0] * 62707 + b[1] * 561 + rand();
	uint32 c2 = b[0] * 51737 + b[1] * 7343 + rand();
	s1 = 1 + c1 % 15;
	s2 = 1 + c2 % 15;
	c = (c1 << s1 | c1 >> (32 - s1)) ^ (c2 << s2 | c2 >> (32 - s2));
	return c;
}

static
int key_len_last = 32;

bn_t* shalom_rand(void* ctx, bn_ctx_t *bnx, int bit_len)
{
	bn_t *bn_out;
	bn_t *bn_key = (bn_t *)ctx;
	uint8 key[70], out[70];
	int key_len = bn_get_len_bytes(bn_key, BN_FMT_LE);
	int out_len = ((bit_len + 15) >> 4) << 1;
	int trunc_len;
	if (key_len < key_len_last) {
		printf(" error  key_len = %d\n", key_len);
		Print(" bn_key", bn_key);
		key_len_last = key_len;
	}
	bn_get(bn_key, BN_FMT_BE, key, key_len);
	randomize((uint16 *)out, out_len, (uint16 *)key, key_len, m32_rand);
	bn_set(bn_key, BN_FMT_BE, key, key_len);
	bn_out = bn_alloc(bnx, BN_FMT_BE, out, out_len);
	trunc_len = out_len * 8 - bit_len;
	if (trunc_len > 0)
		bn_truncate(bn_out, trunc_len);
	return bn_out;
}
