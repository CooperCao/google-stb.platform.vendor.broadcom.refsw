#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include <bcm_bn.h>

#include "bn_utils.h"

static
bn_rand_fn_t bn_rand;


static
void Print_Len(char* title, const bn_t *b)
{
	Print(title, b);
	printf("%-10s:  len_bits = %d\n", title, bn_get_len_bits(b));
	printf("%-10s:  len_bytes = %d\n", title, bn_get_len_bytes(b, BN_FMT_BE));
}


#define P256  54

static
const uint8 prime[P256] = {3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 57, 59,
	61, 67, 71, 73, 79, 83, 89, 97, 101, 103, 107, 109, 113, 127, 131, 137, 139, 149, 151,
	157, 163, 167, 173, 179, 181, 191, 193, 197, 199, 211, 223, 227, 229, 233, 239, 241, 251
};


static  /* search for m  that is not divisible by any prime < 256 */
void near_prime(bn_t *m)
{
	int j, k, tt;
	int zero;
	uint8 r[P256], a[P256], two = 2;
	uint8 ans;
	bn_ctx_t *bnx = bn_get_ctx(m);
	bn_t *div = bn_alloc(bnx, BN_FMT_BE, &two, 1);
	bn_t *rem = bn_alloc(bnx, BN_FMT_BE, 0, 1);
	memset (r, 0, P256);
	memset (a, 0, P256);

	bn_mod(rem, m, div);
	bn_get(rem, BN_FMT_BE, &ans, 1);
	if (!ans)
		bn_iadd(m, m, 1, 0);

	for (j = 0; j < P256; j++) {
		bn_set(div, BN_FMT_BE, &prime[j], 1);
		bn_mod(rem, m, div);
		bn_get(rem, BN_FMT_BE, &r[j], 1);
		zero |= !r[j];
	}
	if (!zero)
		return;

	tt = 2;
	j = 0;
	while (j < P256) {
		r[j] = (r[j] + tt - a[j]) % prime[j];
		a[j] = tt;
		if (r[j])
			j++;
		else {
			tt += 2;
			j = 0;
		}
	}
	bn_iadd(m, m, tt, 0);
	bn_free(&rem);
	bn_free(&div);
}


static
void normalize(bn_t* b, bn_t* b1, bn_t* m)
{
	Print(" m", m);
	near_prime(m);
	Print(" m", m);
	Print(" b", b);
	if (bn_cmp(b, m) > 0) {
		bn_mod(b, b, m);
		Print(" b", b);
	}
	Print(" b1", b1);
	if (bn_cmp(b1, m) > 0) {
		bn_mod(b1, b1, m);
		Print(" b1", b1);
	}
	puts(" ");
}


static
void test_shift(void *s, int bit_len)
{
	bn_t *b, *b1;
	bn_ctx_t *bnx = bn_ctx_alloc(shalom_alloc, shalom_free, s, bit_len);
	bn_t *bn_rkey = get_rand_key(bnx);
	if (!bn_rkey)
		return;

	printf("\n FUNCTION: %s\n", __FUNCTION__);
	b = bn_rand(bn_rkey, bnx, bit_len);
	b1 = bn_rand(bn_rkey, bnx, bit_len - 20);
	Print_Len(" b", b);

	bn_shift(b, 19);
	Print_Len(" shift19", b);

	bn_shift(b, -19);
	Print_Len(" unshift", b);

	puts("");
	Print(" b1", b1);
	bn_shift(b1, -17);
	Print_Len(" shift-17", b1);

	bn_truncate(b1, 7);
	Print_Len(" trunc 7", b1);

	bn_shift(b1, +27);
	Print_Len(" shift+27", b1);

	puts("");

	set_rand_key(bn_rkey);

	bn_free(&b1);
	bn_free(&b);
	bn_free(&bn_rkey);
	bn_ctx_destroy(&bnx);
}

static
void test_bn_add(void *s, int bit_len)
{
	int k;
	bn_t *m, *b, *b1;
	bn_ctx_t *bnx = bn_ctx_alloc(shalom_alloc, shalom_free, s, bit_len);
	bn_t *bn_rkey = get_rand_key(bnx);
	if (!bn_rkey)
		return;

	printf("\n FUNCTION: %s\n", __FUNCTION__);
	m = bn_rand(bn_rkey, bnx, bit_len);
	b = bn_rand(bn_rkey, bnx, bit_len);
	b1 = bn_rand(bn_rkey, bnx, bit_len);

	normalize(b, b1, m);

	for (k=0; k<4; k++) {
		bn_add(b, b, b1, m);
		Print(" b+=b1", b);
		bn_add(b1, b1, b, m);
		Print(" b1+=b", b1);
	}
	puts("");
	for (k=0; k<4; k++) {
		bn_sub(b1, b1, b, m);
		Print(" b1-=b", b1);
		bn_sub(b, b, b1, m);
		Print(" b-=b1", b);
	}
	puts("");

	set_rand_key(bn_rkey);

	bn_free(&b1);
	bn_free(&b);
	bn_free(&m);
	bn_free(&bn_rkey);
	bn_ctx_destroy(&bnx);
}


static
void test_bn_imul(void *s, int bit_len)
{
	bn_t *b, *b1, *r, *r1, *m;
	int SZ = (bit_len + 7) >> 3;
	bn_ctx_t *bnx = bn_ctx_alloc(shalom_alloc, shalom_free, s, bit_len);
	bn_t *bn_rkey = get_rand_key(bnx);
	if (!bn_rkey)
		return;


	printf("\n FUNCTION: %s\n", __FUNCTION__);
	m = bn_rand(bn_rkey, bnx, bit_len);
	b = bn_rand(bn_rkey, bnx, bit_len);
	b1 = bn_rand(bn_rkey, bnx, bit_len);
	r = bn_alloc(bnx, BN_FMT_BE, 0, SZ);
	r1= bn_alloc(bnx, BN_FMT_BE, 0, SZ);

	normalize(b, b1, m);

	bn_imul(r, b, 23456, m);
	Print(" b*23456",r);

	bn_inv(b, m, r1);
	Print(" inv(b)", r1);

	bn_mul(r, r, r1, m);
	Print(" 23456", r);

	puts("");

	bn_imul(r, b1, 3323456, m);
	Print(" b1 * int", r);

	bn_inv(b1, m, r1);
	Print(" inv(b1)", r1);

	bn_mul(r, r, r1, m);
	Print(" 3323456", r);

	puts("");

	set_rand_key(bn_rkey);

	bn_free(&r1);
	bn_free(&r);
	bn_free(&b1);
	bn_free(&b);
	bn_free(&m);
	bn_free(&bn_rkey);
	bn_ctx_destroy(&bnx);
}

static
void test_bn_mult(void *s, int bit_len)
{
	bn_t *b, *b1, *r, *r1, *m;
	int SZ = (bit_len + 7) >> 3;
	bn_ctx_t *bnx = bn_ctx_alloc(shalom_alloc, shalom_free, s, bit_len);
	bn_t *bn_rkey = get_rand_key(bnx);
	if (!bn_rkey)
		return;

	printf("\n FUNCTION: %s\n", __FUNCTION__);
	m = bn_rand(bn_rkey, bnx, bit_len);
	b = bn_rand(bn_rkey, bnx, bit_len);
	b1 = bn_rand(bn_rkey, bnx, bit_len);
	r = bn_alloc(bnx, BN_FMT_BE, 0, SZ);
	r1= bn_alloc(bnx, BN_FMT_BE, 0, SZ);

	normalize(b, b1, m);

	bn_mul(r, b, b1, m);
	Print(" r=b*b1", r);

	bn_sqr(r, r, m);
	Print(" r^2", r);
	puts("");

	bn_sqr(r, b, m);
	Print(" b^2", r);

	bn_sqr(r1, b1, m);
	Print(" b1^2", r1);

	bn_mul(r1, r1, r, m);
	Print(" b^2*b1^2", r1);
	puts("");

	set_rand_key(bn_rkey);

	bn_free(&r1);
	bn_free(&r);
	bn_free(&b1);
	bn_free(&b);
	bn_free(&m);
	bn_free(&bn_rkey);
	bn_ctx_destroy(&bnx);
}


static
void test_bn_inv(void *s, int bit_len)
{
	bn_t *b, *b1, *r, *m;
	int SZ = (bit_len + 7) >> 3;
	bn_ctx_t *bnx = bn_ctx_alloc(shalom_alloc, shalom_free, s, bit_len);
	bn_t *bn_rkey = get_rand_key(bnx);
	if (!bn_rkey)
		return;

	printf("\n FUNCTION: %s\n", __FUNCTION__);
	m = bn_rand(bn_rkey, bnx, bit_len);
	b = bn_rand(bn_rkey, bnx, bit_len);
	b1 = bn_rand(bn_rkey, bnx, bit_len);
	r = bn_alloc(bnx, BN_FMT_BE, 0, SZ);

	normalize(b, b1, m);

	bn_inv(b, m, r);
	Print(" inv(b)", r);
	bn_mul(r, r, b, m);
	Print(" b*inv", r);
	puts("");

	bn_inv(b1, m, r);
	Print(" inv(b1)", r);
	bn_mul(r, r, b1, m);
	Print(" b1*inv", r);
	puts("");

	set_rand_key(bn_rkey);

	bn_free(&r);
	bn_free(&b1);
	bn_free(&b);
	bn_free(&m);
	bn_free(&bn_rkey);
	bn_ctx_destroy(&bnx);
}


static
void test_bn_mod(void *s, int bit_len)
{
	int k;
	bn_t *b, *b1, *c, *c1, *b2, *c2, *t;
	int SZ = (bit_len + 7) >> 3;
	bn_ctx_t *bnx = bn_ctx_alloc(shalom_alloc, shalom_free, s, bit_len);
	bn_t *bn_rkey = get_rand_key(bnx);
	if (!bn_rkey)
		return;

	printf("\n FUNCTION: %s\n", __FUNCTION__);
	t = bn_alloc(bnx, BN_FMT_BE, 0, 2*SZ);
	b1 = bn_alloc(bnx, BN_FMT_BE, 0, SZ);
	c1 = bn_alloc(bnx, BN_FMT_BE, 0, SZ);
	b2 = bn_alloc(bnx, BN_FMT_BE, 0, SZ);
	c2 = bn_alloc(bnx, BN_FMT_BE, 0, SZ);

	//for (k=0; k<1000000; k++)
	for (k=0; k<100000; k++)
	{
	//if (k % 100000 == 0)
	if (k % 10000 == 0)
		printf(" k = %ld\n", k);

	b = bn_rand(bn_rkey, bnx, bit_len);
	c = bn_rand(bn_rkey, bnx, bit_len);

	bn_mul(t, b, c, 0);
	bn_add(t, t, b, 0);
	bn_add(t, t, c, 0);
	bn_mod(b1, b, c);
	bn_mod(c1, c, b);

	bn_mod(b2, t, b);
	bn_mod(c2, t, c);
	if (bn_cmp(b2, c1) || bn_cmp(c2, b1)) {
		printf(" Error -- mod\n");
		Print(" t", t);
		Print(" b", b);
		Print(" c", c);
		Print(" b1", b1);
		Print(" c1", c1);
		Print(" b2", b2);
		Print(" c2", c2);
	}
	bn_free(&c);
	bn_free(&b);
	}

	bn_free(&c2);
	bn_free(&b2);
	bn_free(&c1);
	bn_free(&b1);
	bn_free(&t);
}


static uint8  s[3000];

int main(int argc, char** argv)
{
	int bit_len;
	time_t t;

	srand ((unsigned) time(&t));

	bn_rand = shalom_rand;

	shalom_clear(s, sizeof(s));
	if (argc != 2) {
		printf(" test_bn 'x'  where  'x' = bit length of BN\n");
		return 0;
	}
	bit_len = atoi(argv[1]);

	printf("\n\t\t---------------  b e g i n  ----------------------\n\n");

	printf("\t\t bit_len = %d\n\n", bit_len);

	test_shift(s, bit_len);

	test_bn_add(s, bit_len);

	test_bn_imul(s, bit_len );

	test_bn_mult(s, bit_len);

	test_bn_inv(s, bit_len);

	test_bn_mod(s, bit_len);

	printf("\n\n\t\t---------------  e n d  ----------------------\n");

	return 1;
}
