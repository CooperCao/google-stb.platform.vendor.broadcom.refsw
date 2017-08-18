#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include <bcm_ecdsa.h>

#include "bn_utils.h"

static
bn_rand_fn_t bn_rand;


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
void speed_ecdsa(void *s, ecg_t *ecg, int iter)
{
	int k;
	time_t tStart, tEnd;
	char* Msg = "abc";
	int bit_len = ecg_get_bn_param(ecg, ECG_PARAM_BN_BIT_LEN, 0);
	int len_bytes = ecg_get_bn_param(ecg, ECG_PARAM_BN_BYTE_LEN, 0);
	ecg_type_t ecg_type = ecg_get_type(ecg);
	ecdsa_hash_type_t hash_type;

	bn_ctx_t *bnx = bn_ctx_alloc(shalom_alloc, shalom_free, s, bit_len);

	int Ok = 1;
	ecdsa_t *e_ctx;
	ecdsa_signature_t sig;
	bn_t *bn_rkey = get_rand_key(bnx);
	if (!bn_rkey)
		return;

	if (ecg_type == ECG_NIST_P521)
		hash_type = ECDSA_HASH_SHA512;
	else
	if (ecg_type == ECG_NIST_P384)
		hash_type = ECDSA_HASH_SHA384;
	else
		hash_type = ECDSA_HASH_SHA256;

	e_ctx = ecdsa_alloc(ecg, hash_type, shalom_rand, bn_rkey, NULL, bnx);

	printf("\n FUNCTION: %s\n", __FUNCTION__);
	sig.pub = ecg_elt_alloc(ecg);
	sig.r = bn_alloc(bnx, BN_FMT_BE, 0, len_bytes);
	sig.s = bn_alloc(bnx, BN_FMT_BE, 0, len_bytes);

	time(&tStart);
	for (k=0; k < iter && Ok; k++) {

		ecdsa_sign(e_ctx, Msg, 3, &sig);

		Ok = ecdsa_verify(e_ctx, Msg, 3, &sig);

		if (k % 250 == 0)
			printf("\n\t");

		if (k % 25 == 0)
			printf(" %04d", k);
	}
	time(&tEnd);
	printf("\n\n\t ecp_type = %d   iter = %d   time elapsed = %f\n\n",
		ecg_get_type(ecg), iter, difftime(tEnd, tStart));

	set_rand_key(bn_rkey);

	bn_free(&sig.s);
	bn_free(&sig.r);
	ecg_elt_free((ecg_elt_t **)&sig.pub);
	ecdsa_free(&e_ctx);
	bn_free(&bn_rkey);
	bn_ctx_destroy(&bnx);
}


static uint8 s[3000];

int main(int argc, char *argv[])
{
	bool Ok = 1;
	int j, iter;
	time_t t;
	ecg_t *ecg;
	ecg_type_t ecg_type;
	if (argc != 3) {
		printf(" test x  where 'x' = ecg type (1..4), iter=### \n");
		return 0;
	}
	shalom_clear(s, sizeof(s));
	bn_rand = shalom_rand;

	ecg_type = atoi(argv[1]);
	ecg = ecg_alloc(ecg_type, shalom_alloc, shalom_free, s);
	if (!ecg)
		return 0;

	iter = atoi(argv[2]);

	printf("\n\t\t---------------  b e g i n  ----------------------\n\n");

	PrintPoint(" base", ecg_get_base_elt(ecg));
	puts("");

	srand ((unsigned) time(&t));

	speed_ecdsa(&s, ecg, iter);

	ecg_free(&ecg);

	return Ok;
}
