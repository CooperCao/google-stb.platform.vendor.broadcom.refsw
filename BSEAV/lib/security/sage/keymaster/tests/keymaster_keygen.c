/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.

 ******************************************************************************/

#include "keymaster_debug.h"
#include "keymaster_keygen.h"
#include "bstd.h"
#include <openssl/md5.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/rsa.h>
#include <openssl/bio.h>

BDBG_MODULE(keymaster_keygen);

#define PRINT_KEY_MD5 0

#if PRINT_KEY_MD5
static void km_print_cert_md5_sum(EVP_PKEY *pkey)
{
    uint8_t *blob;
    uint8_t *tmp;
    int len;
    uint8_t hash[16];
    int i;

    len = i2d_PrivateKey(pkey, NULL);
    blob = (uint8_t *)malloc(len);
    tmp = blob;
    len = i2d_PrivateKey(pkey, &tmp);

    MD5(blob, len, hash);
    printf("\n\n");
    for (i = 0; i < 16; i++) {
        printf("%02x", hash[i]);
    }
    printf("\n\n");
    free(blob);
}
#endif

BERR_Code rsa_gen_keys(KeymasterTl_DataBlock *data_block, uint32_t key_size)
{
    BERR_Code err = BERR_SUCCESS;

    RSA *key_pair = RSA_new();
    BIGNUM *bn = BN_new();
    EVP_PKEY *pkey = EVP_PKEY_new();
    BIO *bio = BIO_new(BIO_s_mem());

    BN_set_word(bn, RSA_F4);

    BDBG_MSG(("%s: creating key size %d", BSTD_FUNCTION, key_size));
    if (!RSA_generate_key_ex(key_pair, key_size, bn, NULL)) {
        err = BERR_NOT_INITIALIZED;
        goto done;
    }

    // Assign the key pair to the private key object
    if (!EVP_PKEY_set1_RSA(pkey, key_pair)) {
        goto done;
    }

    i2d_PKCS8PrivateKey_bio(bio, pkey, NULL, NULL, 0, NULL, NULL);

    data_block->size = BIO_pending(bio);
    BDBG_MSG(("%s: block size %d", BSTD_FUNCTION, data_block->size));
    if (data_block->size <= 0) {
        BDBG_ERR(("%s: failed to get pkey size", __FUNCTION__));
        err = BERR_NOT_INITIALIZED;
        goto done;
    }
    data_block->buffer = (uint8_t *)SRAI_Memory_Allocate(data_block->size, SRAI_MemoryType_Shared);
    if (!data_block->buffer) {
        BDBG_ERR(("%s: Couldn't allocate data_block", BSTD_FUNCTION));
        goto done;
    }
    BIO_read(bio, data_block->buffer, data_block->size);

done:
    /* the big number is no longer used */
    EVP_PKEY_free(pkey);
    BN_free(bn);
    RSA_free(key_pair);
    return err;

}

BERR_Code ec_gen_keys(KeymasterTl_DataBlock *data_block, uint32_t key_size)
{
    BERR_Code err = BERR_SUCCESS;

    EVP_PKEY *pkey = EVP_PKEY_new();
    BIO *bio = BIO_new(BIO_s_mem());
    int curve = 0;

    switch (key_size) {
    case 224 :
        curve = NID_secp224r1;
        break;
    case 256 :
        curve = NID_X9_62_prime256v1;
        break;
    case 384 :
        curve = NID_secp384r1;
        break;
    case 521 :
        curve = NID_secp521r1;
        break;
    default :
        BDBG_ERR(("%s: INVALID KEYSIZE, SETTING BAD CURVE FOR TESTING", BSTD_FUNCTION));
        curve = NID_secp224r1;
    }

    EC_KEY *ec_key = EC_KEY_new_by_curve_name(curve);

    if (!EC_KEY_generate_key(ec_key)) {
        BDBG_ERR(("%s: FAILED EC KEY GEN", BSTD_FUNCTION));
        err = BERR_NOT_INITIALIZED;
        goto done;
    }

    // Assign the key pair to the private key object
    if (!EVP_PKEY_set1_EC_KEY(pkey, ec_key)) {
        BDBG_ERR(("%s: FAILED EVP PKEY ASSIGN", BSTD_FUNCTION));
        goto done;
    }

    i2d_PKCS8PrivateKey_bio(bio, pkey, NULL, NULL, 0, NULL, NULL);
#if PRINT_KEY_MD5
    km_print_cert_md5_sum(pkey);
#endif
    data_block->size = BIO_pending(bio);
    if (data_block->size <= 0) {
        BDBG_ERR(("%s: failed to get pkey size", BSTD_FUNCTION));
        err = BERR_NOT_INITIALIZED;
        goto done;
    }
    data_block->buffer = (uint8_t *)SRAI_Memory_Allocate(data_block->size, SRAI_MemoryType_Shared);
    if (!data_block->buffer) {
        BDBG_ERR(("%s: Couldn't allocate data_block", BSTD_FUNCTION));
        goto done;
    }
    BIO_read(bio, data_block->buffer, data_block->size);

done:
    /* the big number is no longer used */
    if (err != BERR_SUCCESS) {
        BDBG_ERR(("%s: EC Keys Not Created", BSTD_FUNCTION));
    }
    EVP_PKEY_free(pkey);
    EC_KEY_free(ec_key);
    return err;
}
