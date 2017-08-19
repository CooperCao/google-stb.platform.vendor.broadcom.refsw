#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include <bcm_ecdh.h>

#include <bcm_rand.h>

#include "bn_utils.h"

static
bn_rand_fn_t bn_rand;


static
void PrintPoint(char* title, const ecg_elt_t* P)
{
	const bn_t *x, *y;
	ecg_t* ecg = ecg_elt_get_group(P);
	ecg_elt_get_xy(P, &x, &y);
	printf(" Point:%s\n", title);
	Print(" x", x);
	Print(" y", y);
	printf(" Ok = %d\n\n", ecg_is_member(ecg, x, y));
}


static
int speed_ecdh(void *s, ecg_t *ecg, int iter)
{
	int k;
	time_t tStart, tEnd;
	bn_t *b, *b1;
	int bit_len = ecg_get_bn_param(ecg, ECG_PARAM_BN_BIT_LEN, 0);
	int len_bytes = ecg_get_bn_param(ecg, ECG_PARAM_BN_BYTE_LEN, 0);
	bn_ctx_t *bnx = bn_ctx_alloc(shalom_alloc, shalom_free, s, bit_len);

	bn_t *bn_rkey = get_rand_key(bnx);
	bn_t *priv1 = NULL;
	bn_t *priv2 = NULL;

	int Ok = 1;

	bn_t *secret1, *secret2;
	ecg_elt_t *pub1, *pub2;
	if (!bn_rkey)
		return 0;

	pub1 = ecg_elt_alloc(ecg);
	pub2 = ecg_elt_alloc(ecg);
	secret1 = bn_alloc(bnx, BN_FMT_BE, 0, len_bytes);
	secret2 = bn_alloc(bnx, BN_FMT_BE, 0, len_bytes);

	time(&tStart);
	for (k=0; k < iter && Ok; k++) {

		ecdh_gen_keys(ecg, pub1, &priv1, bn_rand, bn_rkey, bnx);

		ecdh_gen_keys(ecg, pub2, &priv2, bn_rand, bn_rkey, bnx);

		ecdh_gen_secret(ecg, priv1, pub2, secret1);

		ecdh_gen_secret(ecg, priv2, pub1, secret2);

		if (bn_cmp(secret1, secret2)) {
			printf(" secrets don't match in iter = %d\n", iter);
			return 0;
		}

		if (k % 250 == 0)
			printf("\n\t");

		if (k % 25 == 0)
			printf(" %04d", k);

		bn_free(&priv2);
		bn_free(&priv1);
	}
	time(&tEnd);
	printf("\n\n\t ecp_type = %d   iter = %d   time elapsed = %f\n\n",
		ecg_get_type(ecg), iter, difftime(tEnd, tStart));

	set_rand_key(bn_rkey);

	bn_free(&secret2);
	bn_free(&secret1);
	ecg_elt_free(&pub2);
	ecg_elt_free(&pub1);
	bn_free(&bn_rkey);

	bn_ctx_destroy(&bnx);
	return Ok;
}


static uint8 s[3000];

int main(int argc, char *argv[])
{
	int Ok;
	int j, iter;
	time_t t;
	ecg_t *ecg;
	ecg_type_t ecg_type;
	if (argc != 3) {
		printf(" test x  where 'x' = ecg type (1..4), iter=### \n");
		return 0;
	}
	bn_rand = shalom_rand;
	shalom_clear(s, sizeof(s));

	ecg_type = atoi(argv[1]);
	ecg = ecg_alloc(ecg_type, shalom_alloc, shalom_free, &s);
	if (!ecg)
		return 0;

	iter = atoi(argv[2]);

	printf("\n\t\t---------------  b e g i n  ----------------------\n\n");

	PrintPoint(" base", ecg_get_base_elt(ecg));
	puts("");

	srand ((unsigned) time(&t));

	Ok = speed_ecdh(&s, ecg, iter);

	ecg_free(&ecg);

	return Ok;
}
