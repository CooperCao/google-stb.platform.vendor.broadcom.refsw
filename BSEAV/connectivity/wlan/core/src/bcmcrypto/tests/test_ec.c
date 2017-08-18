#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include <bcm_ec.h>

#include "bn_utils.h"


static
void PrintPoint(char* title, const ecg_elt_t* P)
{
	const bn_t *x, *y;
	ecg_t* ecg = ecg_elt_get_group(P);
	ecg_elt_get_xy(P, &x, &y);
	bool Ok = ecg_is_member(ecg, x, y);
	printf(" Point:%s\n", title);
	Print(" x", x);
	Print(" y", y);
	printf(" Ok = %d\n\n", Ok);
}


static
void test_param(void *s, ecg_t* ecg)
{
	bn_ctx_t *bnx = bn_ctx_alloc(shalom_alloc, shalom_free, s,
		ecg_get_bn_param(ecg, ECG_PARAM_BN_BIT_LEN, 0));

	int len_bytes = ecg_get_bn_param(ecg, ECG_PARAM_BN_BYTE_LEN, 0);
	bn_t *bn = bn_alloc(bnx, BN_FMT_BE, 0, len_bytes);

	printf("\n FUNCTION: %s\n", __FUNCTION__);
	ecg_get_bn_param(ecg, ECG_PARAM_BN_PRIME, bn);
	Print(" Prime", bn);
	puts("");

	ecg_get_bn_param(ecg, ECG_PARAM_BN_ORDER, bn);
	Print(" Order", bn);
	puts("");

	printf(" Cofactor : %2d", ecg_get_bn_param(ecg, ECG_PARAM_BN_COFACTOR, 0));
	puts("");

	printf(" A        : %6d", ecg_get_bn_param(ecg, ECG_PARAM_BN_A, 0));
	puts("");

	ecg_get_bn_param(ecg, ECG_PARAM_BN_B, bn);
	Print(" B", bn);
	puts("");

	printf(" Bit Length: %3d", ecg_get_bn_param(ecg, ECG_PARAM_BN_BIT_LEN, 0));
	puts("");

	printf(" Byte Length: %2d", len_bytes);
	puts("");

	bn_free(&bn);
	bn_ctx_destroy(&bnx);
}


static
void test_create_point(void *s, ecg_t *ecg)
{
	int j;
	ecg_elt_t *elt;
	int bit_len = ecg_get_bn_param(ecg, ECG_PARAM_BN_BIT_LEN, 0);
	bn_ctx_t *bnx = bn_ctx_alloc(shalom_alloc, shalom_free, s, bit_len);
	bn_t *key = get_rand_key(bnx);

	printf("\n FUNCTION: %s\n", __FUNCTION__);
	for (j = 0; j < 10; j++) {
		elt = ecg_elt_rand(ecg, shalom_rand, key);
		PrintPoint(" generated", elt);
		ecg_elt_free(&elt);
	}
	set_rand_key(key);
	bn_free(&key);
	bn_ctx_destroy(&bnx);
}


static
void test_double_point(void *s, ecg_t* ecg)
{
	int j;
	int bit_len = ecg_get_bn_param(ecg, ECG_PARAM_BN_BIT_LEN, 0);
	bn_ctx_t *bnx = bn_ctx_alloc(shalom_alloc, shalom_free, s, bit_len);
	bn_t *key = get_rand_key(bnx);

	ecg_type_t ecg_type = ecg_get_type(ecg);
	ecg_elt_t *elt;
	ecg_elt_t *elt1 = ecg_elt_alloc(ecg);
	ecg_elt_t *elt2 = ecg_elt_alloc(ecg);
	printf("\n FUNCTION: %s\n", __FUNCTION__);

	elt = ecg_elt_rand(ecg, shalom_rand, key);
	ecg_elt_copy(elt1, elt);
	ecg_elt_inv(elt1, elt2);
	PrintPoint(" generated", elt);

	for (j=0; j<5; j++) {
		ecg_elt_dbl(elt1, elt1);
		ecg_elt_dbl(elt2, elt2);
	}
	puts("");
	PrintPoint(" 2 ^ 5", elt1);
	PrintPoint(" inv", elt2);
	ecg_elt_free(&elt);
	ecg_elt_free(&elt2);
	ecg_elt_free(&elt1);

	set_rand_key(key);
	bn_free(&key);
	bn_ctx_destroy(&bnx);
}

static
void test_add_point(void *s, ecg_t *ecg)
{
	int j;
	int bit_len = ecg_get_bn_param(ecg, ECG_PARAM_BN_BIT_LEN, 0);
	bn_ctx_t *bnx = bn_ctx_alloc(shalom_alloc, shalom_free, s, bit_len);
	bn_t *key = get_rand_key(bnx);

	ecg_type_t ecg_type = ecg_get_type(ecg);
	ecg_elt_t *elt;
	ecg_elt_t *elt1 = ecg_elt_alloc(ecg);
	ecg_elt_t *elt2 = ecg_elt_alloc(ecg);
	elt = ecg_elt_rand(ecg, shalom_rand, key);
	printf("\n FUNCTION: %s\n", __FUNCTION__);
	PrintPoint(" generated", elt);
	ecg_elt_copy(elt2, elt);
	ecg_elt_dbl(elt, elt1);
	PrintPoint(" double", elt1);

	for (j=0; j<3; j++) {
		ecg_elt_add(elt, elt1, elt1);
		PrintPoint(" add", elt1);
	}
	ecg_elt_inv(elt, elt2);
	for (j=0; j<3; j++) {
		ecg_elt_add(elt2, elt1, elt1);
		PrintPoint(" sub", elt1);
	}
	ecg_elt_free(&elt);
	ecg_elt_free(&elt2);
	ecg_elt_free(&elt1);

	set_rand_key(key);
	bn_free(&key);
	bn_ctx_destroy(&bnx);
}


static
void test_fibo(void *s, ecg_t *ecg)
{
	int n, j, k;
	int K[3] = {5, 0, 2000};
	char str[10];
	int bit_len = ecg_get_bn_param(ecg, ECG_PARAM_BN_BIT_LEN, 0);
	bn_ctx_t *bnx = bn_ctx_alloc(shalom_alloc, shalom_free, s, bit_len);
	bn_t *key = get_rand_key(bnx);

	bn_t *b1, *b2;
	bn_t *Order;
	uint16 len_bytes = ecg_get_bn_param(ecg, ECG_PARAM_BN_BYTE_LEN, 0);
	ecg_elt_t *elt = ecg_elt_rand(ecg, shalom_rand, key);

	ecg_elt_t *elt1 = ecg_elt_alloc(ecg);
	ecg_elt_t *elt2 = ecg_elt_alloc(ecg);
	ecg_elt_t *elt3 = ecg_elt_alloc(ecg);

	printf("\n FUNCTION: %s\n", __FUNCTION__);
	K[1] = rand() % 0x00ff;
	if (K[1] < 50)
		K[1] += 50;

	Order = bn_alloc(bnx, BN_FMT_BE, 0, len_bytes);
	ecg_get_bn_param(ecg, ECG_PARAM_BN_ORDER, Order);

	b1 = bn_alloc(bnx, BN_FMT_BE, 0, len_bytes);
	b2 = bn_alloc(bnx, BN_FMT_BE, 0, len_bytes);

	ecg_elt_copy(elt1, elt);
	PrintPoint(" generated", elt1);
	ecg_elt_dbl(elt1, elt2);
	PrintPoint(" double", elt2);
	bn_iadd(b1, b1, 1, 0);
	bn_iadd(b2, b2, 2, 0);
	n = 2;
	for (k = 0; k < 3; k++) {
		for (j = 0; j < K[k]; j++) {
			ecg_elt_add(elt2, elt1, elt1);
			bn_add(b1, b1, b2, Order);
			ecg_elt_add(elt1, elt2, elt2);
			bn_add(b2, b2, b1, Order);
		}
		n += j << 1;
		sprintf(str, " F(%d)", n);
		Print(str, b2);
		PrintPoint(" add", elt2);
		ecg_elt_mul(elt, b2, elt3);
		PrintPoint(" mult", elt3);
	}
	bn_free(&b2);
	bn_free(&b1);
	bn_free(&Order);

	ecg_elt_free(&elt3);
	ecg_elt_free(&elt2);
	ecg_elt_free(&elt1);
	ecg_elt_free(&elt);

	set_rand_key(key);
	bn_free(&key);
	bn_ctx_destroy(&bnx);
}


static uint8 s[3000];

int main(int argc, char *argv[])
{
	time_t t;
	ecg_t *ecg;
	ecg_type_t ecg_type;
	if (argc != 2) {
		printf(" test x  where 'x' = ecg type (1..4)\n");
		return 0;
	}
	shalom_clear(s, sizeof(s));

	ecg_type = atoi(argv[1]);
	ecg = ecg_alloc(ecg_type, shalom_alloc, shalom_free, s);
	if (!ecg) {
		if (ecg_type < 1 || ecg_type > 4)
			printf(" ecg_type = %d  not legal  ecg_type = {1..3}\n", ecg_type);
		else
			printf(" init_ecg failed although type was legal: ecg_type = %d\n", ecg_type);
		return 0;
	}

	printf("\n\t\t---------------  b e g i n  ----------------------\n\n");

	srand ((unsigned) time(&t));

	PrintPoint(" base", ecg_get_base_elt(ecg));

	test_param(s, ecg);

	test_create_point(s, ecg);

	test_double_point(s, ecg);

	test_add_point(s, ecg);

	test_fibo(s, ecg);

	printf("\n\n\t\t---------------  e n d  ----------------------\n");

	ecg_free(&ecg);

	return 1;
}
