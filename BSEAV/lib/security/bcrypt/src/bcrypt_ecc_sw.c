/******************************************************************************
 *  Copyright (C) 2019 Broadcom.
 *  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to
 *  the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied),
 *  right to use, or waiver of any kind with respect to the Software, and
 *  Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 *  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 *  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization,
 *  constitutes the valuable trade secrets of Broadcom, and you shall use all
 *  reasonable efforts to protect the confidentiality thereof, and to use this
 *  information only in connection with your use of Broadcom integrated circuit
 *  products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 *  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 *  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 *  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 *  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 *  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 *  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 *  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/

/* General purpose elliptic curve routines. */
#include "bstd.h"
#include "berr.h"
#include "bkni.h"
#include "bdbg.h"

#include <stdio.h>
#include <stdlib.h>
#include "bcrypt_ecc_sw.h"

BDBG_MODULE(BCRYPT);

static int bcrypt_Is_neg(BIGNUM * p)
{
#if (OPENSSL_VERSION_NUMBER > 0x10100000L)
    if(BN_is_negative(p))
#else
    if (p->neg)
#endif
    {
        return 1;
    }
    else
    {
        return 0;
     }
}
/*-jfu check 4*a^3+27*b^2<>0 */
static int ECC_param_ok(BIGNUM * p, BIGNUM * a, BIGNUM * b)
{
    BIGNUM *tmp1, *tmp2;
    BN_CTX *ctx;
    int retval;

    if ((p == NULL) || (a == NULL) || (b == NULL))
        return 0;

    ctx = BN_CTX_new();
    if(ctx == NULL) {
        BDBG_ERR(("NULL context\n"));
        return 0;
    }
    tmp1 = BN_new();
    tmp2 = BN_new();

    BN_mod_mul(tmp1, a, a, p, ctx);
    BN_mod_mul(tmp1, tmp1, a, p, ctx);
    BN_lshift(tmp1, tmp1, 2);
    BN_mod(tmp1, tmp1, p, ctx);

        /* Removed by Abbas to fix the memory leak (Removed the extra tmp2 = BN_new())   */
    BN_set_word(tmp2, 27);
    BN_mod_mul(tmp2, tmp2, b, p, ctx);
    BN_mod_mul(tmp2, tmp2, b, p, ctx);

    BN_add(tmp2, tmp1, tmp2);
    BN_mod(tmp2, tmp2, p, ctx);

    retval = !BN_is_zero(tmp2);

    BN_CTX_free(ctx);
    BN_free(tmp1);
    BN_free(tmp2);

    return retval;
}


ECC *ECC_new_set(BIGNUM * p, BIGNUM * a, BIGNUM * b, ECCpt g, BIGNUM *n)
{
    ECC *ecc;

    if (!ECC_param_ok(p, a, b))
        return NULL;

    ecc = malloc(sizeof(ECC));
    if (ecc != NULL) {
        ecc->modulus = BN_dup(p);
        ecc->a = BN_dup(a);
        ecc->b = BN_dup(b);
        ecc->generator.x = BN_dup(g.x);
        ecc->generator.y = BN_dup(g.y);
        ecc->pubkey.x = ecc->pubkey.y = NULL;
        ecc->privkey = NULL;
    ecc->n = BN_dup(n);
    }

    return ecc;
}


void ECC_free(ECC * ecc)
{
    if (ecc != NULL) {
        BN_free(ecc->modulus);
        ecc->modulus = NULL;
        BN_free(ecc->a);
        ecc->a = NULL;
        BN_free(ecc->b);
        ecc->b = NULL;
        BN_free(ecc->generator.x);
        ecc->generator.x = NULL;
        BN_free(ecc->generator.y);
        ecc->generator.y = NULL;
    BN_free(ecc->n);
    ecc->n=NULL;
        if (ecc->pubkey.x != NULL) {
            BN_free(ecc->pubkey.x);
            ecc->pubkey.x = NULL;
            BN_free(ecc->pubkey.y);
            ecc->pubkey.y = NULL;
        }
        if (ecc->privkey != NULL) {
            BN_free(ecc->privkey);
            ecc->privkey = NULL;
        }
        free(ecc);
    }
}


void ECCpt_init(ECCpt * pt)
{
    pt->x = BN_new();
    pt->y = BN_new();
}


void ECCpt_free(ECCpt * pt)
{
    BN_free(pt->x);
    pt->x = NULL;
    BN_free(pt->y);
    pt->y = NULL;
}


int ECCpt_is_valid_pt(ECCpt * a, ECC * ecc)
{
    /*  check that y^2 = x^3 + a x + b  */
    BIGNUM *tmp1, *tmp2;
    BN_CTX *ctx;
    int retval;

    ctx = BN_CTX_new();
    if(ctx == NULL) {
        BDBG_ERR(("NULL context\n"));
        return 0;
    }
    tmp1 = BN_dup(a->x);
    BN_mod_mul(tmp1, tmp1, tmp1, ecc->modulus, ctx);
    BN_add(tmp1, tmp1, ecc->a);
    BN_mod_mul(tmp1, tmp1, a->x, ecc->modulus, ctx);
    BN_add(tmp1, tmp1, ecc->b);
    if (BN_cmp(tmp1, ecc->modulus) >= 0)
        BN_sub(tmp1, tmp1, ecc->modulus);

    tmp2 = BN_dup(a->y);
    BN_mod_mul(tmp2, tmp2, tmp2, ecc->modulus, ctx);

    retval = (BN_cmp(tmp1, tmp2) == 0);
    BN_free(tmp1);
    BN_free(tmp2);
    BN_CTX_free(ctx);
    return retval;
}


int ECCpt_is_equal(ECCpt * a, ECCpt * b)
{
    if (bcrypt_Is_neg(a->x) && bcrypt_Is_neg(b->x))
    {
        return 1;
    }
    return ((BN_cmp(a->x, b->x) == 0) && (BN_cmp(a->y, b->y) == 0));
}


void ECCpt_add(ECCpt * r, ECCpt * a, ECCpt * b, ECC * ecc)
{
    BN_CTX *ctx;
    BIGNUM *tmp1, *tmp2;
    BIGNUM *lambda;

    if (bcrypt_Is_neg(a->x)) {
        BN_copy(r->x, b->x);
        BN_copy(r->y, b->y);
        return;
    }

    if (bcrypt_Is_neg(b->x)) {
        BN_copy(r->x, a->x);
        BN_copy(r->y, a->y);
        return;
    }

    tmp1 = BN_new();
    if (BN_cmp(a->x, b->x) == 0) {
        BN_add(tmp1, a->y, b->y);
        if (BN_cmp(tmp1, ecc->modulus) == 0) {
            BN_free(tmp1);
            BN_set_negative(r->x,1);    /*  Set to identity  */
            return;
        }
    }

    ctx = BN_CTX_new();
    if(ctx == NULL) {
        BDBG_ERR(("NULL context\n"));
        goto out_free;
    }
    tmp2 = BN_new();
    lambda = BN_new();
    if (ECCpt_is_equal(a, b)) {
        BN_set_word(tmp1, 3);
        BN_mod_mul(tmp1, tmp1, a->x, ecc->modulus, ctx);
        BN_mod_mul(tmp1, tmp1, a->x, ecc->modulus, ctx);
        BN_add(tmp1, tmp1, ecc->a);
        BN_mod(tmp1, tmp1, ecc->modulus, ctx);
        BN_lshift1(tmp2, a->y);
        BN_mod_inverse(tmp2, tmp2, ecc->modulus, ctx);
        BN_mod_mul(lambda, tmp1, tmp2, ecc->modulus, ctx);
    } else {
        BN_sub(tmp1, b->x, a->x);
        if (bcrypt_Is_neg(tmp1))
            BN_add(tmp1, ecc->modulus, tmp1);
        BN_free(tmp2);  /* Abbas fix for memory leak fix, */
                                /* should free tmp2 before calling the BN_mod_inverse with NULL parameter */
                        /* a memory will be allocated for tmp2 by BN_mod_inverse */
        tmp2 = BN_mod_inverse(NULL, tmp1, ecc->modulus, ctx);
        /* check return value*/
        if (!tmp2) {
            BDBG_ERR(("BN_mod_inverse failed\n"));
            goto out_free;
        }
        BN_sub(tmp1, b->y, a->y);
        if (bcrypt_Is_neg(tmp1))
            BN_add(tmp1, ecc->modulus, tmp1);
        BN_mod_mul(lambda, tmp1, tmp2, ecc->modulus, ctx);
    }

    BN_mod_mul(tmp1, lambda, lambda, ecc->modulus, ctx);
    BN_sub(tmp1, tmp1, a->x);
    if (bcrypt_Is_neg(tmp1))
        BN_add(tmp1, ecc->modulus, tmp1);
    BN_sub(tmp2, tmp1, b->x);
    if (bcrypt_Is_neg(tmp2))
        BN_add(tmp2, ecc->modulus, tmp2);

    BN_sub(tmp1, a->x, tmp2);
    if (bcrypt_Is_neg(tmp1))
        BN_add(tmp1, ecc->modulus, tmp1);
    BN_mod_mul(tmp1, lambda, tmp1, ecc->modulus, ctx);
    BN_sub(r->y, tmp1, a->y);
    if (bcrypt_Is_neg(r->y))
        BN_add(r->y, ecc->modulus, r->y);

    BN_free(r->x);
    r->x = tmp2;
    tmp2 = NULL;

    BN_CTX_free(ctx);
    BN_free(lambda);
out_free:
    BN_free(tmp1);
}


void ECCpt_mul(ECCpt * r, ECCpt * a, BIGNUM * n, ECC * ecc)
{
    ECCpt tmp;
    int numbits, i, negnum=-1;
    BIGNUM * negtmp=NULL;
    BN_CTX *ctx;

    tmp.x = BN_dup(a->x);
    tmp.y = BN_dup(a->y);

#if (OPENSSL_VERSION_NUMBER > 0x10100000L)
    BN_set_negative(r->x,1);
#else
    r->x->neg = 1;
#endif

    if (!bcrypt_Is_neg(r->x))
    {
        ctx = BN_CTX_new();
        if(ctx == NULL) {
            BDBG_ERR(("NULL context\n"));
            goto out_free;
        }
        negtmp = BN_new();
        negtmp = BN_bin2bn((const unsigned char *)&negnum, sizeof(negnum), negtmp);
        BN_set_negative(negtmp,1);    /*  Set to identity  */
        BN_copy(r->x, negtmp);
        BN_CTX_free(ctx);
        BN_free(negtmp);
    }

    numbits = BN_num_bits(n);

    for (i = numbits - 1; i >= 0; i--) {
        if (BN_is_bit_set(n, i))
            ECCpt_add(r, r, &tmp, ecc);
        if (i > 0)
            ECCpt_add(r, r, r, ecc);
    }

    /* Added by Alireza to fix the memory leak */
    ECCpt_free(&tmp);

out_free:
    return;
}
