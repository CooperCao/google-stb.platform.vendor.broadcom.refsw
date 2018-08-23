/******************************************************************************
 *  Copyright (C) 2018 Broadcom.
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

#include <string.h>

#include "keymaster_debug.h"
#include "bstd.h"
#include "bkni.h"
#include "nexus_security_client.h"
#include "nexus_memory.h"

#include "keymaster_ids.h"
#include "keymaster_platform.h"
#include "keymaster_tl.h"
#include "keymaster_test.h"
#include "keymaster_crypto_utils.h"
#include "keymaster_key_params.h"


BDBG_MODULE(keymaster_crypto_hmac);

/* Currently test must use a multiple of 3*4K because SRAI block pointers must be 4K aligned */
#define TEST_BLOCK_SIZE (3 * 4096)

/* Allocate for worst case (depends on TAG_MAC_LENGTH which is max 512 bits) */
#define MAX_SIGNATURE_SIZE  64

#define NUM_VECTOR_TESTS 7

/* HMAC tests which map to HW - tests encrypt followed by decrypt of a data block */
static BERR_Code km_crypto_hmac_sign_verify_test(KeymasterTl_Handle handle)
{
    BERR_Code err;
    KM_Tag_ContextHandle key_params = NULL;
    KM_Tag_ContextHandle begin_params = NULL;
    KM_CryptoOperation_Settings settings;
    KeymasterTl_DataBlock key;
    KeymasterTl_DataBlock in_data;
    KeymasterTl_DataBlock out_data;
    KeymasterTl_DataBlock signature_data;
    int i;
    const char *comment;

    memset(&key, 0, sizeof(key));
    memset(&in_data, 0, sizeof(in_data));
    memset(&out_data, 0, sizeof(out_data));
    memset(&signature_data, 0, sizeof(signature_data));

    BDBG_LOG(("----------------------- %s -----------------------", BSTD_FUNCTION));
    for (i = 0; i < 18; i++) {

        KM_CryptoOperation_GetDefaultSettings(&settings);
        settings.handle = handle;

        EXPECT_SUCCESS(KM_Tag_NewContext(&begin_params));

        settings.begin_params = begin_params;
        TEST_TAG_ADD_ENUM(settings.begin_params, SKM_TAG_PADDING, SKM_PAD_NONE);

        switch (i) {
        case 0:
            EXPECT_SUCCESS(km_test_new_params_with_hmac_defaults(&key_params, 160));
            TEST_TAG_ADD_ENUM(key_params, SKM_TAG_DIGEST, SKM_DIGEST_SHA1);
            TEST_TAG_ADD_INTEGER(settings.begin_params, SKM_TAG_MAC_LENGTH, 160);
            TEST_TAG_ADD_ENUM(settings.begin_params, SKM_TAG_DIGEST, SKM_DIGEST_SHA1);
            comment = "HMAC 160 160 SHA1";
            break;
        case 1:
            EXPECT_SUCCESS(km_test_new_params_with_hmac_defaults(&key_params, 224));
            TEST_TAG_ADD_ENUM(key_params, SKM_TAG_DIGEST, SKM_DIGEST_SHA_2_224);
            TEST_TAG_ADD_INTEGER(settings.begin_params, SKM_TAG_MAC_LENGTH, 224);
            TEST_TAG_ADD_ENUM(settings.begin_params, SKM_TAG_DIGEST, SKM_DIGEST_SHA_2_224);
            comment = "HMAC 224 224 SHA 224";
            break;
        case 2:
            EXPECT_SUCCESS(km_test_new_params_with_hmac_defaults(&key_params, 256));
            TEST_TAG_ADD_ENUM(key_params, SKM_TAG_DIGEST, SKM_DIGEST_SHA_2_256);
            TEST_TAG_ADD_INTEGER(settings.begin_params, SKM_TAG_MAC_LENGTH, 256);
            TEST_TAG_ADD_ENUM(settings.begin_params, SKM_TAG_DIGEST, SKM_DIGEST_SHA_2_256);
            comment = "HMAC 256 256 SHA 256";
            break;
        case 3:
            EXPECT_SUCCESS(km_test_new_params_with_hmac_defaults(&key_params, 512));
            TEST_TAG_ADD_ENUM(key_params, SKM_TAG_DIGEST, SKM_DIGEST_SHA1);
            TEST_TAG_ADD_INTEGER(settings.begin_params, SKM_TAG_MAC_LENGTH, 160);
            TEST_TAG_ADD_ENUM(settings.begin_params, SKM_TAG_DIGEST, SKM_DIGEST_SHA1);
            comment = "HMAC 512 160 SHA1";
            break;
        case 4:
            EXPECT_SUCCESS(km_test_new_params_with_hmac_defaults(&key_params, 512));
            TEST_TAG_ADD_ENUM(key_params, SKM_TAG_DIGEST, SKM_DIGEST_SHA_2_224);
            TEST_TAG_ADD_INTEGER(settings.begin_params, SKM_TAG_MAC_LENGTH, 224);
            TEST_TAG_ADD_ENUM(settings.begin_params, SKM_TAG_DIGEST, SKM_DIGEST_SHA_2_224);
            comment = "HMAC 512 224 SHA 224";
            break;
        case 5:
            EXPECT_SUCCESS(km_test_new_params_with_hmac_defaults(&key_params, 512));
            TEST_TAG_ADD_ENUM(key_params, SKM_TAG_DIGEST, SKM_DIGEST_SHA_2_256);
            TEST_TAG_ADD_INTEGER(settings.begin_params, SKM_TAG_MAC_LENGTH, 256);
            TEST_TAG_ADD_ENUM(settings.begin_params, SKM_TAG_DIGEST, SKM_DIGEST_SHA_2_256);
            comment = "HMAC 512 256 SHA 256";
            break;
        case 6:
            EXPECT_SUCCESS(km_test_new_params_with_hmac_defaults(&key_params, 512));
            TEST_TAG_ADD_ENUM(key_params, SKM_TAG_DIGEST, SKM_DIGEST_SHA_2_384);
            TEST_TAG_ADD_INTEGER(settings.begin_params, SKM_TAG_MAC_LENGTH, 384);
            TEST_TAG_ADD_ENUM(settings.begin_params, SKM_TAG_DIGEST, SKM_DIGEST_SHA_2_384);
            comment = "HMAC 512 384 SHA 384";
            break;
        case 7:
            EXPECT_SUCCESS(km_test_new_params_with_hmac_defaults(&key_params, 224));
            TEST_TAG_ADD_ENUM(key_params, SKM_TAG_DIGEST, SKM_DIGEST_SHA_2_512);
            TEST_TAG_ADD_INTEGER(settings.begin_params, SKM_TAG_MAC_LENGTH, 256);
            TEST_TAG_ADD_ENUM(settings.begin_params, SKM_TAG_DIGEST, SKM_DIGEST_SHA_2_512);
            comment = "HMAC 224 SHA 512";
            break;
        case 8:
            EXPECT_SUCCESS(km_test_new_params_with_hmac_defaults(&key_params, 512));
            TEST_TAG_ADD_ENUM(key_params, SKM_TAG_DIGEST, SKM_DIGEST_SHA_2_512);
            TEST_TAG_ADD_INTEGER(settings.begin_params, SKM_TAG_MAC_LENGTH, 512);
            TEST_TAG_ADD_ENUM(settings.begin_params, SKM_TAG_DIGEST, SKM_DIGEST_SHA_2_512);
            comment = "HMAC 512 SHA 512";
            break;
        case 9:
            EXPECT_SUCCESS(km_test_new_params_with_hmac_defaults(&key_params, 224));
            TEST_TAG_ADD_ENUM(key_params, SKM_TAG_DIGEST, SKM_DIGEST_MD5);
            TEST_TAG_ADD_INTEGER(settings.begin_params, SKM_TAG_MAC_LENGTH, 128);
            TEST_TAG_ADD_ENUM(settings.begin_params, SKM_TAG_DIGEST, SKM_DIGEST_MD5);
            comment = "HMAC MD5";
            break;
        case 10:
            EXPECT_SUCCESS(km_test_new_params_with_hmac_defaults(&key_params, 512));
            TEST_TAG_ADD_ENUM(key_params, SKM_TAG_DIGEST, SKM_DIGEST_SHA_2_512);
            TEST_TAG_ADD_INTEGER(settings.begin_params, SKM_TAG_MAC_LENGTH, 64);
            TEST_TAG_ADD_ENUM(settings.begin_params, SKM_TAG_DIGEST, SKM_DIGEST_SHA_2_512);
            comment = "HMAC 512 SHA 512, MAC 64";
            break;
        case 11:
            EXPECT_SUCCESS(km_test_new_params_with_hmac_defaults(&key_params, 64));
            TEST_TAG_ADD_ENUM(key_params, SKM_TAG_DIGEST, SKM_DIGEST_SHA1);
            TEST_TAG_ADD_INTEGER(settings.begin_params, SKM_TAG_MAC_LENGTH, 64);
            TEST_TAG_ADD_ENUM(settings.begin_params, SKM_TAG_DIGEST, SKM_DIGEST_SHA1);
            comment = "HMAC 64 SHA1, MAC 64";
            break;
        case 12:
            EXPECT_SUCCESS(km_test_new_params_with_hmac_defaults(&key_params, 128));
            TEST_TAG_ADD_ENUM(key_params, SKM_TAG_DIGEST, SKM_DIGEST_MD5);
            TEST_TAG_ADD_INTEGER(settings.begin_params, SKM_TAG_MAC_LENGTH, 64);
            TEST_TAG_ADD_ENUM(settings.begin_params, SKM_TAG_DIGEST, SKM_DIGEST_MD5);
            comment = "HMAC 128 MD5, MAC 160";
            break;
        case 13:
            EXPECT_SUCCESS(km_test_new_params_with_hmac_defaults(&key_params, 128));
            TEST_TAG_ADD_ENUM(key_params, SKM_TAG_DIGEST, SKM_DIGEST_SHA1);
            TEST_TAG_ADD_INTEGER(settings.begin_params, SKM_TAG_MAC_LENGTH, 160);
            TEST_TAG_ADD_ENUM(settings.begin_params, SKM_TAG_DIGEST, SKM_DIGEST_SHA1);
            comment = "HMAC 128 SHA1, MAC 160";
            break;
        case 14:
            EXPECT_SUCCESS(km_test_new_params_with_hmac_defaults(&key_params, 128));
            TEST_TAG_ADD_ENUM(key_params, SKM_TAG_DIGEST, SKM_DIGEST_SHA_2_224);
            TEST_TAG_ADD_INTEGER(settings.begin_params, SKM_TAG_MAC_LENGTH, 160);
            TEST_TAG_ADD_ENUM(settings.begin_params, SKM_TAG_DIGEST, SKM_DIGEST_SHA_2_224);
            comment = "HMAC 128 SHA224, MAC 160";
            break;
        case 15:
            EXPECT_SUCCESS(km_test_new_params_with_hmac_defaults(&key_params, 128));
            TEST_TAG_ADD_ENUM(key_params, SKM_TAG_DIGEST, SKM_DIGEST_SHA_2_256);
            TEST_TAG_ADD_INTEGER(settings.begin_params, SKM_TAG_MAC_LENGTH, 160);
            TEST_TAG_ADD_ENUM(settings.begin_params, SKM_TAG_DIGEST, SKM_DIGEST_SHA_2_256);
            comment = "HMAC 128 SHA224, MAC 160";
            break;
        case 16:
            EXPECT_SUCCESS(km_test_new_params_with_hmac_defaults(&key_params, 128));
            TEST_TAG_ADD_ENUM(key_params, SKM_TAG_DIGEST, SKM_DIGEST_SHA_2_384);
            TEST_TAG_ADD_INTEGER(settings.begin_params, SKM_TAG_MAC_LENGTH, 160);
            TEST_TAG_ADD_ENUM(settings.begin_params, SKM_TAG_DIGEST, SKM_DIGEST_SHA_2_384);
            comment = "HMAC 128 SHA384, MAC 160";
            break;
        case 17:
            EXPECT_SUCCESS(km_test_new_params_with_hmac_defaults(&key_params, 128));
            TEST_TAG_ADD_ENUM(key_params, SKM_TAG_DIGEST, SKM_DIGEST_SHA_2_512);
            TEST_TAG_ADD_INTEGER(settings.begin_params, SKM_TAG_MAC_LENGTH, 160);
            TEST_TAG_ADD_ENUM(settings.begin_params, SKM_TAG_DIGEST, SKM_DIGEST_SHA_2_512);
            comment = "HMAC 128 SHA512, MAC 160";
            break;
        default:
            BDBG_ERR(("%s:%d Internal error", BSTD_FUNCTION, __LINE__));
            err = BERR_UNKNOWN;
            goto done;
        }
        BDBG_ERR(("%s : Test - %s", BSTD_FUNCTION, comment));

        EXPECT_SUCCESS(KeymasterTl_GenerateKey(handle, key_params, &key));

        settings.key_params = key_params;
        settings.in_key = key;

        TEST_ALLOCATE_BLOCK(in_data, TEST_BLOCK_SIZE);
        TEST_ALLOCATE_BLOCK(out_data, MAX_SIGNATURE_SIZE);
        memset(in_data.buffer, 0xC3, in_data.size);
        memset(out_data.buffer, 0xFF, out_data.size);
        settings.in_data = in_data;
        settings.out_data = out_data;

        EXPECT_SUCCESS(KM_Crypto_Operation(SKM_PURPOSE_SIGN, &settings));

        BDBG_LOG(("%s: signature %d bytes (%d bits)", BSTD_FUNCTION, settings.out_data.size, settings.out_data.size * 8));
        if (settings.out_data.size == 0) {
            BDBG_ERR(("NO SIGNATURE GENERATED FAILED: %s", comment));
            continue;
        }

        TEST_ALLOCATE_BLOCK(signature_data, settings.out_data.size);
        memcpy(signature_data.buffer, settings.out_data.buffer, signature_data.size);
        settings.signature_data = signature_data;
        settings.out_data.buffer = NULL;
        settings.out_data.size = 0;

        km_test_remove_mac_length(settings.begin_params);

        EXPECT_SUCCESS(KM_Crypto_Operation(SKM_PURPOSE_VERIFY, &settings));

        settings.signature_data.buffer[0]++;
        EXPECT_FAILURE_CODE(KM_Crypto_Operation(SKM_PURPOSE_VERIFY, &settings), BSAGE_ERR_KM_VERIFICATION_FAILED);

        settings.signature_data.buffer[0]--;
        settings.in_data.buffer[0]++;
        EXPECT_FAILURE_CODE(KM_Crypto_Operation(SKM_PURPOSE_VERIFY, &settings), BSAGE_ERR_KM_VERIFICATION_FAILED);

        BDBG_LOG(("%s: %s success", BSTD_FUNCTION, comment));

        TEST_FREE_BLOCK(in_data);
        TEST_FREE_BLOCK(out_data);
        TEST_FREE_BLOCK(signature_data);
        TEST_FREE_BLOCK(key);
        TEST_DELETE_CONTEXT(begin_params);
        TEST_DELETE_CONTEXT(key_params);
    }
    err = BERR_SUCCESS;

done:
    TEST_FREE_BLOCK(in_data);
    TEST_FREE_BLOCK(out_data);
    TEST_FREE_BLOCK(signature_data);
    TEST_FREE_BLOCK(key);
    TEST_DELETE_CONTEXT(begin_params);
    TEST_DELETE_CONTEXT(key_params);
    return err;
}

static BERR_Code km_crypto_hmac_generate_fails(KeymasterTl_Handle handle)
{
    BERR_Code err;
    KM_Tag_ContextHandle key_params = NULL;
    KeymasterTl_DataBlock key;
    int i;
    const char *comment;
    int key_size = 128;
    BERR_Code expected_err;

    memset(&key, 0, sizeof(key));

    BDBG_LOG(("----------------------- %s -----------------------", BSTD_FUNCTION));
    for (i = 0; i < 5; i++) {

        switch (i) {
        case 0:
            EXPECT_SUCCESS(km_test_new_params_with_hmac_defaults(&key_params, key_size));
            TEST_TAG_ADD_ENUM(key_params, SKM_TAG_DIGEST, SKM_DIGEST_SHA1);
            TEST_TAG_ADD_ENUM(key_params, SKM_TAG_DIGEST, SKM_DIGEST_SHA_2_256);
            comment = "HMAC Multi Digest";
            expected_err = BSAGE_ERR_KM_UNSUPPORTED_DIGEST;
            break;
        case 1:
            EXPECT_SUCCESS(km_test_new_params_with_hmac_defaults(&key_params, key_size));
            TEST_TAG_ADD_ENUM(key_params, SKM_TAG_DIGEST, SKM_DIGEST_NONE);
            comment = "HMAC Invalid Digest";
            expected_err = BSAGE_ERR_KM_UNSUPPORTED_DIGEST;
            break;
        case 2:
            EXPECT_SUCCESS(KM_Tag_NewContext(&key_params));
            TEST_TAG_ADD_ENUM(key_params, SKM_TAG_ALGORITHM, SKM_ALGORITHM_HMAC);
            TEST_TAG_ADD_INTEGER(key_params, SKM_TAG_KEY_SIZE, key_size);
            TEST_TAG_ADD_ENUM(key_params, SKM_TAG_PURPOSE, SKM_PURPOSE_SIGN);
            TEST_TAG_ADD_ENUM(key_params, SKM_TAG_PURPOSE, SKM_PURPOSE_VERIFY);
            TEST_TAG_ADD_INTEGER(key_params, SKM_TAG_MIN_MAC_LENGTH, 48);

            TEST_TAG_ADD_BOOL(key_params, SKM_TAG_ALL_USERS, true);
            TEST_TAG_ADD_BOOL(key_params, SKM_TAG_NO_AUTH_REQUIRED, true);
            TEST_TAG_ADD_BOOL(key_params, SKM_TAG_ALL_APPLICATIONS, true);
            TEST_TAG_ADD_ENUM(key_params, SKM_TAG_DIGEST, SKM_DIGEST_SHA_2_256);
            comment = "HMAC Min Mac Size too small";
            expected_err = BSAGE_ERR_KM_UNSUPPORTED_MIN_MAC_LENGTH;
            break;
        case 3:
            EXPECT_SUCCESS(KM_Tag_NewContext(&key_params));
            TEST_TAG_ADD_ENUM(key_params, SKM_TAG_ALGORITHM, SKM_ALGORITHM_HMAC);
            TEST_TAG_ADD_INTEGER(key_params, SKM_TAG_KEY_SIZE, key_size);
            TEST_TAG_ADD_ENUM(key_params, SKM_TAG_PURPOSE, SKM_PURPOSE_SIGN);
            TEST_TAG_ADD_ENUM(key_params, SKM_TAG_PURPOSE, SKM_PURPOSE_VERIFY);
            TEST_TAG_ADD_INTEGER(key_params, SKM_TAG_MIN_MAC_LENGTH, 130);

            TEST_TAG_ADD_BOOL(key_params, SKM_TAG_ALL_USERS, true);
            TEST_TAG_ADD_BOOL(key_params, SKM_TAG_NO_AUTH_REQUIRED, true);
            TEST_TAG_ADD_BOOL(key_params, SKM_TAG_ALL_APPLICATIONS, true);
            TEST_TAG_ADD_ENUM(key_params, SKM_TAG_DIGEST, SKM_DIGEST_SHA_2_256);
            comment = "HMAC Min Mac Size non octet";
            expected_err = BSAGE_ERR_KM_UNSUPPORTED_MIN_MAC_LENGTH;
            break;
        case 4:
            EXPECT_SUCCESS(KM_Tag_NewContext(&key_params));
            TEST_TAG_ADD_ENUM(key_params, SKM_TAG_ALGORITHM, SKM_ALGORITHM_HMAC);
            TEST_TAG_ADD_INTEGER(key_params, SKM_TAG_KEY_SIZE, key_size);
            TEST_TAG_ADD_ENUM(key_params, SKM_TAG_PURPOSE, SKM_PURPOSE_SIGN);
            TEST_TAG_ADD_ENUM(key_params, SKM_TAG_PURPOSE, SKM_PURPOSE_VERIFY);
            TEST_TAG_ADD_INTEGER(key_params, SKM_TAG_MIN_MAC_LENGTH, 384);

            TEST_TAG_ADD_BOOL(key_params, SKM_TAG_ALL_USERS, true);
            TEST_TAG_ADD_BOOL(key_params, SKM_TAG_NO_AUTH_REQUIRED, true);
            TEST_TAG_ADD_BOOL(key_params, SKM_TAG_ALL_APPLICATIONS, true);
            TEST_TAG_ADD_ENUM(key_params, SKM_TAG_DIGEST, SKM_DIGEST_SHA_2_256);
            comment = "HMAC Min Mac Size too long";
            expected_err = BSAGE_ERR_KM_UNSUPPORTED_MIN_MAC_LENGTH;
            break;
        default:
            BDBG_ERR(("%s:%d Internal error", BSTD_FUNCTION, __LINE__));
            err = BERR_UNKNOWN;
            goto done;
        }
        BDBG_LOG(("%s : Test - %s", BSTD_FUNCTION, comment));

        EXPECT_FAILURE_CODE(KeymasterTl_GenerateKey(handle, key_params, &key), expected_err);
        TEST_FREE_BLOCK(key);
        TEST_DELETE_CONTEXT(key_params);
    }
    err = BERR_SUCCESS;

done:
    TEST_FREE_BLOCK(key);
    TEST_DELETE_CONTEXT(key_params);
    return err;
}

static BERR_Code km_crypto_hmac_sign_fails(KeymasterTl_Handle handle)
{
    BERR_Code err;
    KM_Tag_ContextHandle key_params = NULL;
    KM_Tag_ContextHandle begin_params = NULL;
    KM_CryptoOperation_Settings settings;
    KeymasterTl_DataBlock key;
    KeymasterTl_DataBlock in_data;
    KeymasterTl_DataBlock out_data;
    km_purpose_t purpose;
    BERR_Code expected_err;
    int i;
    const char *comment;

    memset(&key, 0, sizeof(key));
    memset(&in_data, 0, sizeof(in_data));
    memset(&out_data, 0, sizeof(out_data));

    BDBG_LOG(("----------------------- %s -----------------------", BSTD_FUNCTION));
    for (i = 0; i < 4; i++) {

        KM_CryptoOperation_GetDefaultSettings(&settings);
        settings.handle = handle;

        EXPECT_SUCCESS(KM_Tag_NewContext(&begin_params));

        settings.begin_params = begin_params;
        TEST_TAG_ADD_ENUM(settings.begin_params, SKM_TAG_PADDING, SKM_PAD_NONE);

        switch (i) {
        case 0:
            EXPECT_SUCCESS(km_test_new_params_with_hmac_defaults(&key_params, 128));
            TEST_TAG_ADD_ENUM(key_params, SKM_TAG_DIGEST, SKM_DIGEST_SHA_2_256);
            TEST_TAG_ADD_INTEGER(settings.begin_params, SKM_TAG_MAC_LENGTH, 264);
            TEST_TAG_ADD_ENUM(settings.begin_params, SKM_TAG_DIGEST, SKM_DIGEST_SHA_2_256);
            purpose = SKM_PURPOSE_SIGN;
            comment = "HMAC mac length too large";
            expected_err = BSAGE_ERR_KM_UNSUPPORTED_MAC_LENGTH;
            break;
        case 1:
            EXPECT_SUCCESS(km_test_new_params_with_hmac_defaults(&key_params, 128));
            TEST_TAG_ADD_ENUM(key_params, SKM_TAG_DIGEST, SKM_DIGEST_SHA_2_256);
            TEST_TAG_ADD_INTEGER(settings.begin_params, SKM_TAG_MAC_LENGTH, 30);
            TEST_TAG_ADD_ENUM(settings.begin_params, SKM_TAG_DIGEST, SKM_DIGEST_SHA_2_256);
            comment = "HMAC mac length too small";
            purpose = SKM_PURPOSE_SIGN;
            expected_err = BSAGE_ERR_KM_UNSUPPORTED_MAC_LENGTH;
            break;
        case 2:
            EXPECT_SUCCESS(km_test_new_params_with_hmac_defaults(&key_params, 256));
            TEST_TAG_ADD_ENUM(key_params, SKM_TAG_DIGEST, SKM_DIGEST_SHA_2_256);
            TEST_TAG_ADD_INTEGER(settings.begin_params, SKM_TAG_MAC_LENGTH, 256);
            TEST_TAG_ADD_ENUM(settings.begin_params, SKM_TAG_DIGEST, SKM_DIGEST_SHA_2_256);
            comment = "HMAC encrypt";
            purpose = SKM_PURPOSE_ENCRYPT;
            expected_err = BSAGE_ERR_KM_UNSUPPORTED_PURPOSE;
            break;
        case 3:
            EXPECT_SUCCESS(km_test_new_params_with_hmac_defaults(&key_params, 128));
            TEST_TAG_ADD_ENUM(key_params, SKM_TAG_DIGEST, SKM_DIGEST_SHA_2_256);
            TEST_TAG_ADD_INTEGER(settings.begin_params, SKM_TAG_MAC_LENGTH, 65);
            TEST_TAG_ADD_ENUM(settings.begin_params, SKM_TAG_DIGEST, SKM_DIGEST_SHA_2_256);
            comment = "HMAC invalid mac length";
            purpose = SKM_PURPOSE_SIGN;
            expected_err = BSAGE_ERR_KM_UNSUPPORTED_MAC_LENGTH;
            break;
        default:

            BDBG_ERR(("%s:%d Internal error", BSTD_FUNCTION, __LINE__));
            err = BERR_UNKNOWN;
            goto done;
        }
        BDBG_ERR(("%s : Test - %s", BSTD_FUNCTION, comment));

        EXPECT_SUCCESS(KeymasterTl_GenerateKey(handle, key_params, &key));

        settings.key_params = key_params;
        settings.in_key = key;

        TEST_ALLOCATE_BLOCK(in_data, TEST_BLOCK_SIZE);
        TEST_ALLOCATE_BLOCK(out_data, MAX_SIGNATURE_SIZE);
        memset(in_data.buffer, 0xC3, in_data.size);
        memset(out_data.buffer, 0xFF, out_data.size);
        settings.in_data = in_data;
        settings.out_data = out_data;

        EXPECT_FAILURE_CODE(KM_Crypto_Operation(purpose, &settings), expected_err);

        BDBG_LOG(("%s: %s success", BSTD_FUNCTION, comment));

        TEST_FREE_BLOCK(in_data);
        TEST_FREE_BLOCK(out_data);
        TEST_FREE_BLOCK(key);
        TEST_DELETE_CONTEXT(begin_params);
        TEST_DELETE_CONTEXT(key_params);
    }
    err = BERR_SUCCESS;

done:
    TEST_FREE_BLOCK(in_data);
    TEST_FREE_BLOCK(out_data);
    TEST_FREE_BLOCK(key);
    TEST_DELETE_CONTEXT(begin_params);
    TEST_DELETE_CONTEXT(key_params);
    return err;
}

static BERR_Code km_crypto_sign_vector_test(KeymasterTl_Handle handle, KeymasterTl_DataBlock in_key,
                                            KeymasterTl_DataBlock message, km_digest_t digest, KeymasterTl_DataBlock expected_mac)
{

    BERR_Code err;
    KM_Tag_ContextHandle begin_params = NULL;
    KM_CryptoOperation_Settings settings;
    KeymasterTl_DataBlock out_data;
    KeymasterTl_ImportKeySettings impSettings;

    memset(&out_data, 0, sizeof(out_data));
    memset(&impSettings, 0, sizeof(impSettings));

    BDBG_LOG(("----------------------- %s -----------------------", BSTD_FUNCTION));
    KM_CryptoOperation_GetDefaultSettings(&settings);
    settings.handle = handle;


    KeymasterTl_GetDefaultImportKeySettings(&impSettings);
    EXPECT_SUCCESS(km_test_new_params_with_hmac_defaults(&impSettings.in_key_params, in_key.size * 8));
    impSettings.in_key_format = SKM_KEY_FORMAT_RAW;
    impSettings.in_key_blob = in_key;
    TEST_TAG_ADD_ENUM(impSettings.in_key_params, SKM_TAG_DIGEST, digest);
    EXPECT_SUCCESS(KeymasterTl_ImportKey(handle, &impSettings));


    settings.in_key = impSettings.out_key_blob;

    EXPECT_SUCCESS(KM_Tag_NewContext(&begin_params));
    TEST_TAG_ADD_ENUM(begin_params, SKM_TAG_PADDING, SKM_PAD_NONE);
    TEST_TAG_ADD_INTEGER(begin_params, SKM_TAG_MAC_LENGTH, expected_mac.size * 8);
    TEST_TAG_ADD_ENUM(begin_params, SKM_TAG_DIGEST, digest);
    settings.begin_params = begin_params;

    settings.key_params = impSettings.in_key_params;

    TEST_ALLOCATE_BLOCK(out_data, expected_mac.size);
    memset(out_data.buffer, 0xFF, out_data.size);
    settings.in_data = message;
    settings.out_data = out_data;

    EXPECT_SUCCESS(KM_Crypto_Operation(SKM_PURPOSE_SIGN, &settings));


    if (memcmp(out_data.buffer, expected_mac.buffer, expected_mac.size) != 0) {
        BDBG_ERR(("Expected and outdata do not match"));
        err = BERR_UNKNOWN;
        goto done;
    }

    BDBG_LOG(("%s: success", BSTD_FUNCTION));

    err = BERR_SUCCESS;

done:
    TEST_FREE_BLOCK(impSettings.out_key_blob);
    TEST_DELETE_CONTEXT(impSettings.in_key_params);
    TEST_FREE_BLOCK(out_data);
    TEST_DELETE_CONTEXT(begin_params);
    return err;
}


static BERR_Code km_crypto_hmac_vectors(KeymasterTl_Handle handle)
{
    BERR_Code err = BERR_UNKNOWN;
    int i, j;
    KeymasterTl_DataBlock key_data;
    KeymasterTl_DataBlock message;
    KeymasterTl_DataBlock expected;
    km_digest_t digest;

    BDBG_LOG(("----------------------- %s -----------------------", BSTD_FUNCTION));
    for (i = 0; i < NUM_VECTOR_TESTS; i++) {
        BDBG_LOG(("----------------------- HmacRfc4231TestCase %d -----------------------", i));
        switch (i) {
        case 0:
            {
                uint8_t key[] = {
                    0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b,
                    0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b,
                };
                char *message_string = "Hi There";
                uint8_t sha_224_expected[] = {
                    0x89, 0x6f, 0xb1, 0x12, 0x8a, 0xbb, 0xdf, 0x19, 0x68, 0x32, 0x10, 0x7c, 0xd4, 0x9d,
                    0xf3, 0x3f, 0x47, 0xb4, 0xb1, 0x16, 0x99, 0x12, 0xba, 0x4f, 0x53, 0x68, 0x4b, 0x22,
                };
                uint8_t sha_256_expected[] = {
                    0xb0, 0x34, 0x4c, 0x61, 0xd8, 0xdb, 0x38, 0x53, 0x5c, 0xa8, 0xaf,
                    0xce, 0xaf, 0x0b, 0xf1, 0x2b, 0x88, 0x1d, 0xc2, 0x00, 0xc9, 0x83,
                    0x3d, 0xa7, 0x26, 0xe9, 0x37, 0x6c, 0x2e, 0x32, 0xcf, 0xf7,
                };
                uint8_t sha_384_expected[] = {
                    0xaf, 0xd0, 0x39, 0x44, 0xd8, 0x48, 0x95, 0x62, 0x6b, 0x08, 0x25, 0xf4,
                    0xab, 0x46, 0x90, 0x7f, 0x15, 0xf9, 0xda, 0xdb, 0xe4, 0x10, 0x1e, 0xc6,
                    0x82, 0xaa, 0x03, 0x4c, 0x7c, 0xeb, 0xc5, 0x9c, 0xfa, 0xea, 0x9e, 0xa9,
                    0x07, 0x6e, 0xde, 0x7f, 0x4a, 0xf1, 0x52, 0xe8, 0xb2, 0xfa, 0x9c, 0xb6,
                };
                uint8_t sha_512_expected[] = {
                    0x87, 0xaa, 0x7c, 0xde, 0xa5, 0xef, 0x61, 0x9d, 0x4f, 0xf0, 0xb4, 0x24, 0x1a,
                    0x1d, 0x6c, 0xb0, 0x23, 0x79, 0xf4, 0xe2, 0xce, 0x4e, 0xc2, 0x78, 0x7a, 0xd0,
                    0xb3, 0x05, 0x45, 0xe1, 0x7c, 0xde, 0xda, 0xa8, 0x33, 0xb7, 0xd6, 0xb8, 0xa7,
                    0x02, 0x03, 0x8b, 0x27, 0x4e, 0xae, 0xa3, 0xf4, 0xe4, 0xbe, 0x9d, 0x91, 0x4e,
                    0xeb, 0x61, 0xf1, 0x70, 0x2e, 0x69, 0x6c, 0x20, 0x3a, 0x12, 0x68, 0x54,
                };

                TEST_ALLOCATE_BLOCK(key_data, sizeof(key));
                memcpy(key_data.buffer, key, key_data.size);

                TEST_ALLOCATE_BLOCK(message, strlen(message_string));
                memcpy(message.buffer, message_string, message.size);

                for (j = 0; j < 4; j++) {
                    switch (j) {

                    case 0 :
                        TEST_ALLOCATE_BLOCK(expected, sizeof(sha_224_expected));
                        memcpy(expected.buffer, sha_224_expected, expected.size);
                        digest = SKM_DIGEST_SHA_2_224;
                        break;
                    case 1 :
                        TEST_ALLOCATE_BLOCK(expected, sizeof(sha_256_expected));
                        memcpy(expected.buffer, sha_256_expected, expected.size);
                        digest = SKM_DIGEST_SHA_2_256;
                        break;
                    case 2 :
                        TEST_ALLOCATE_BLOCK(expected, sizeof(sha_384_expected));
                        memcpy(expected.buffer, sha_384_expected, expected.size);
                        digest = SKM_DIGEST_SHA_2_384;
                        break;
                    case 3 :
                        TEST_ALLOCATE_BLOCK(expected, sizeof(sha_512_expected));
                        memcpy(expected.buffer, sha_512_expected, expected.size);
                        digest = SKM_DIGEST_SHA_2_512;
                        break;
                    }
                    EXPECT_SUCCESS(km_crypto_sign_vector_test(handle, key_data, message, digest, expected));

                    TEST_FREE_BLOCK(expected);
                }
                break;
            }
        case 1:
            {
                /* No longer valid as minimum key size is 64 bytes */
                break;
                char *key = "Jefe";
                char *message_string = "what do ya want for nothing?";
                uint8_t sha_224_expected[] = {
                    0xa3, 0x0e, 0x01, 0x09, 0x8b, 0xc6, 0xdb, 0xbf, 0x45, 0x69, 0x0f, 0x3a, 0x7e, 0x9e,
                    0x6d, 0x0f, 0x8b, 0xbe, 0xa2, 0xa3, 0x9e, 0x61, 0x48, 0x00, 0x8f, 0xd0, 0x5e, 0x44,
                };
                uint8_t sha_256_expected[] = {
                    0x5b, 0xdc, 0xc1, 0x46, 0xbf, 0x60, 0x75, 0x4e, 0x6a, 0x04, 0x24,
                    0x26, 0x08, 0x95, 0x75, 0xc7, 0x5a, 0x00, 0x3f, 0x08, 0x9d, 0x27,
                    0x39, 0x83, 0x9d, 0xec, 0x58, 0xb9, 0x64, 0xec, 0x38, 0x43,
                };
                uint8_t sha_384_expected[] = {
                    0xaf, 0x45, 0xd2, 0xe3, 0x76, 0x48, 0x40, 0x31, 0x61, 0x7f, 0x78, 0xd2,
                    0xb5, 0x8a, 0x6b, 0x1b, 0x9c, 0x7e, 0xf4, 0x64, 0xf5, 0xa0, 0x1b, 0x47,
                    0xe4, 0x2e, 0xc3, 0x73, 0x63, 0x22, 0x44, 0x5e, 0x8e, 0x22, 0x40, 0xca,
                    0x5e, 0x69, 0xe2, 0xc7, 0x8b, 0x32, 0x39, 0xec, 0xfa, 0xb2, 0x16, 0x49,
                };
                uint8_t sha_512_expected[] = {
                    0x16, 0x4b, 0x7a, 0x7b, 0xfc, 0xf8, 0x19, 0xe2, 0xe3, 0x95, 0xfb, 0xe7, 0x3b,
                    0x56, 0xe0, 0xa3, 0x87, 0xbd, 0x64, 0x22, 0x2e, 0x83, 0x1f, 0xd6, 0x10, 0x27,
                    0x0c, 0xd7, 0xea, 0x25, 0x05, 0x54, 0x97, 0x58, 0xbf, 0x75, 0xc0, 0x5a, 0x99,
                    0x4a, 0x6d, 0x03, 0x4f, 0x65, 0xf8, 0xf0, 0xe6, 0xfd, 0xca, 0xea, 0xb1, 0xa3,
                    0x4d, 0x4a, 0x6b, 0x4b, 0x63, 0x6e, 0x07, 0x0a, 0x38, 0xbc, 0xe7, 0x37,
                };

                TEST_ALLOCATE_BLOCK(key_data, strlen(key));
                memcpy(key_data.buffer, key, key_data.size);

                TEST_ALLOCATE_BLOCK(message, strlen(message_string));
                memcpy(message.buffer, message_string, message.size);

                for (j = 0; j < 4; j++) {
                    switch (j) {
                    case 0 :
                        TEST_ALLOCATE_BLOCK(expected, sizeof(sha_224_expected));
                        memcpy(expected.buffer, sha_224_expected, expected.size);
                        digest = SKM_DIGEST_SHA_2_224;
                        break;
                    case 1 :
                        TEST_ALLOCATE_BLOCK(expected, sizeof(sha_256_expected));
                        memcpy(expected.buffer, sha_256_expected, expected.size);
                        digest = SKM_DIGEST_SHA_2_256;
                        break;
                    case 2 :
                        TEST_ALLOCATE_BLOCK(expected, sizeof(sha_384_expected));
                        memcpy(expected.buffer, sha_384_expected, expected.size);
                        digest = SKM_DIGEST_SHA_2_384;
                        break;
                    case 3 :
                        TEST_ALLOCATE_BLOCK(expected, sizeof(sha_512_expected));
                        memcpy(expected.buffer, sha_512_expected, expected.size);
                        digest = SKM_DIGEST_SHA_2_512;
                        break;
                    }
                    EXPECT_SUCCESS(km_crypto_sign_vector_test(handle, key_data, message, digest, expected));

                    TEST_FREE_BLOCK(expected);
                }
                break;
            }
        case 2:
            {
                uint8_t sha_224_expected[] = {
                    0x7f, 0xb3, 0xcb, 0x35, 0x88, 0xc6, 0xc1, 0xf6, 0xff, 0xa9, 0x69, 0x4d, 0x7d, 0x6a,
                    0xd2, 0x64, 0x93, 0x65, 0xb0, 0xc1, 0xf6, 0x5d, 0x69, 0xd1, 0xec, 0x83, 0x33, 0xea,
                };
                uint8_t sha_256_expected[] = {
                    0x77, 0x3e, 0xa9, 0x1e, 0x36, 0x80, 0x0e, 0x46, 0x85, 0x4d, 0xb8,
                    0xeb, 0xd0, 0x91, 0x81, 0xa7, 0x29, 0x59, 0x09, 0x8b, 0x3e, 0xf8,
                    0xc1, 0x22, 0xd9, 0x63, 0x55, 0x14, 0xce, 0xd5, 0x65, 0xfe,
                };
                uint8_t sha_384_expected[] = {
                    0x88, 0x06, 0x26, 0x08, 0xd3, 0xe6, 0xad, 0x8a, 0x0a, 0xa2, 0xac, 0xe0,
                    0x14, 0xc8, 0xa8, 0x6f, 0x0a, 0xa6, 0x35, 0xd9, 0x47, 0xac, 0x9f, 0xeb,
                    0xe8, 0x3e, 0xf4, 0xe5, 0x59, 0x66, 0x14, 0x4b, 0x2a, 0x5a, 0xb3, 0x9d,
                    0xc1, 0x38, 0x14, 0xb9, 0x4e, 0x3a, 0xb6, 0xe1, 0x01, 0xa3, 0x4f, 0x27,
                };
                uint8_t sha_512_expected[] = {
                    0xfa, 0x73, 0xb0, 0x08, 0x9d, 0x56, 0xa2, 0x84, 0xef, 0xb0, 0xf0, 0x75, 0x6c,
                    0x89, 0x0b, 0xe9, 0xb1, 0xb5, 0xdb, 0xdd, 0x8e, 0xe8, 0x1a, 0x36, 0x55, 0xf8,
                    0x3e, 0x33, 0xb2, 0x27, 0x9d, 0x39, 0xbf, 0x3e, 0x84, 0x82, 0x79, 0xa7, 0x22,
                    0xc8, 0x06, 0xb4, 0x85, 0xa4, 0x7e, 0x67, 0xc8, 0x07, 0xb9, 0x46, 0xa3, 0x37,
                    0xbe, 0xe8, 0x94, 0x26, 0x74, 0x27, 0x88, 0x59, 0xe1, 0x32, 0x92, 0xfb,
                };

                TEST_ALLOCATE_BLOCK(key_data, 20);
                memset(key_data.buffer, 0xaa, key_data.size);

                TEST_ALLOCATE_BLOCK(message, 50);
                memset(message.buffer, 0xdd, message.size);

                for (j = 0; j < 4; j++) {
                    switch (j) {
                    case 0 :
                        TEST_ALLOCATE_BLOCK(expected, sizeof(sha_224_expected));
                        memcpy(expected.buffer, sha_224_expected, expected.size);
                        digest = SKM_DIGEST_SHA_2_224;
                        break;
                    case 1 :
                        TEST_ALLOCATE_BLOCK(expected, sizeof(sha_256_expected));
                        memcpy(expected.buffer, sha_256_expected, expected.size);
                        digest = SKM_DIGEST_SHA_2_256;
                        break;
                    case 2 :
                        TEST_ALLOCATE_BLOCK(expected, sizeof(sha_384_expected));
                        memcpy(expected.buffer, sha_384_expected, expected.size);
                        digest = SKM_DIGEST_SHA_2_384;
                        break;
                    case 3 :
                        TEST_ALLOCATE_BLOCK(expected, sizeof(sha_512_expected));
                        memcpy(expected.buffer, sha_512_expected, expected.size);
                        digest = SKM_DIGEST_SHA_2_512;
                        break;
                    }
                    EXPECT_SUCCESS(km_crypto_sign_vector_test(handle, key_data, message, digest, expected));

                    TEST_FREE_BLOCK(expected);
                }
                break;
            }
        case 3:
            {
                uint8_t key[25] = {
                    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d,
                    0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19,
                };
                uint8_t sha_224_expected[] = {
                    0x6c, 0x11, 0x50, 0x68, 0x74, 0x01, 0x3c, 0xac, 0x6a, 0x2a, 0xbc, 0x1b, 0xb3, 0x82,
                    0x62, 0x7c, 0xec, 0x6a, 0x90, 0xd8, 0x6e, 0xfc, 0x01, 0x2d, 0xe7, 0xaf, 0xec, 0x5a,
                };
                uint8_t sha_256_expected[] = {
                    0x82, 0x55, 0x8a, 0x38, 0x9a, 0x44, 0x3c, 0x0e, 0xa4, 0xcc, 0x81,
                    0x98, 0x99, 0xf2, 0x08, 0x3a, 0x85, 0xf0, 0xfa, 0xa3, 0xe5, 0x78,
                    0xf8, 0x07, 0x7a, 0x2e, 0x3f, 0xf4, 0x67, 0x29, 0x66, 0x5b,
                };
                uint8_t sha_384_expected[] = {
                    0x3e, 0x8a, 0x69, 0xb7, 0x78, 0x3c, 0x25, 0x85, 0x19, 0x33, 0xab, 0x62,
                    0x90, 0xaf, 0x6c, 0xa7, 0x7a, 0x99, 0x81, 0x48, 0x08, 0x50, 0x00, 0x9c,
                    0xc5, 0x57, 0x7c, 0x6e, 0x1f, 0x57, 0x3b, 0x4e, 0x68, 0x01, 0xdd, 0x23,
                    0xc4, 0xa7, 0xd6, 0x79, 0xcc, 0xf8, 0xa3, 0x86, 0xc6, 0x74, 0xcf, 0xfb,
                };
                uint8_t sha_512_expected[] = {
                    0xb0, 0xba, 0x46, 0x56, 0x37, 0x45, 0x8c, 0x69, 0x90, 0xe5, 0xa8, 0xc5, 0xf6,
                    0x1d, 0x4a, 0xf7, 0xe5, 0x76, 0xd9, 0x7f, 0xf9, 0x4b, 0x87, 0x2d, 0xe7, 0x6f,
                    0x80, 0x50, 0x36, 0x1e, 0xe3, 0xdb, 0xa9, 0x1c, 0xa5, 0xc1, 0x1a, 0xa2, 0x5e,
                    0xb4, 0xd6, 0x79, 0x27, 0x5c, 0xc5, 0x78, 0x80, 0x63, 0xa5, 0xf1, 0x97, 0x41,
                    0x12, 0x0c, 0x4f, 0x2d, 0xe2, 0xad, 0xeb, 0xeb, 0x10, 0xa2, 0x98, 0xdd,
                };

                TEST_ALLOCATE_BLOCK(key_data, sizeof(key));
                memcpy(key_data.buffer, key, key_data.size);

                TEST_ALLOCATE_BLOCK(message, 50);
                memset(message.buffer, 0xcd, message.size);

                for (j = 0; j < 4; j++) {
                    switch (j) {
                    case 0 :
                        TEST_ALLOCATE_BLOCK(expected, sizeof(sha_224_expected));
                        memcpy(expected.buffer, sha_224_expected, expected.size);
                        digest = SKM_DIGEST_SHA_2_224;
                        break;
                    case 1 :
                        TEST_ALLOCATE_BLOCK(expected, sizeof(sha_256_expected));
                        memcpy(expected.buffer, sha_256_expected, expected.size);
                        digest = SKM_DIGEST_SHA_2_256;
                        break;
                    case 2 :
                        TEST_ALLOCATE_BLOCK(expected, sizeof(sha_384_expected));
                        memcpy(expected.buffer, sha_384_expected, expected.size);
                        digest = SKM_DIGEST_SHA_2_384;
                        break;
                    case 3 :
                        TEST_ALLOCATE_BLOCK(expected, sizeof(sha_512_expected));
                        memcpy(expected.buffer, sha_512_expected, expected.size);
                        digest = SKM_DIGEST_SHA_2_512;
                        break;
                    }
                    EXPECT_SUCCESS(km_crypto_sign_vector_test(handle, key_data, message, digest, expected));

                    TEST_FREE_BLOCK(expected);
                }
                break;
            }
        case 4:
            {
                char *message_string = "Test With Truncation";

                uint8_t sha_224_expected[] = {
                    0x0e, 0x2a, 0xea, 0x68, 0xa9, 0x0c, 0x8d, 0x37,
                    0xc9, 0x88, 0xbc, 0xdb, 0x9f, 0xca, 0x6f, 0xa8,
                };
                uint8_t sha_256_expected[] = {
                    0xa3, 0xb6, 0x16, 0x74, 0x73, 0x10, 0x0e, 0xe0,
                    0x6e, 0x0c, 0x79, 0x6c, 0x29, 0x55, 0x55, 0x2b,
                };
                uint8_t sha_384_expected[] = {
                    0x3a, 0xbf, 0x34, 0xc3, 0x50, 0x3b, 0x2a, 0x23,
                    0xa4, 0x6e, 0xfc, 0x61, 0x9b, 0xae, 0xf8, 0x97,
                };
                uint8_t sha_512_expected[] = {
                    0x41, 0x5f, 0xad, 0x62, 0x71, 0x58, 0x0a, 0x53,
                    0x1d, 0x41, 0x79, 0xbc, 0x89, 0x1d, 0x87, 0xa6,
                };

                TEST_ALLOCATE_BLOCK(key_data, 20);
                memset(key_data.buffer, 0x0c, key_data.size);

                TEST_ALLOCATE_BLOCK(message, strlen(message_string));
                memcpy(message.buffer, message_string, message.size);
                TEST_FREE_BLOCK(expected);
                for (j = 0; j < 4; j++) {
                    switch (j) {
                    case 0 :
                        TEST_ALLOCATE_BLOCK(expected, sizeof(sha_224_expected));
                        memcpy(expected.buffer, sha_224_expected, expected.size);
                        digest = SKM_DIGEST_SHA_2_224;
                        break;
                    case 1 :
                        TEST_ALLOCATE_BLOCK(expected, sizeof(sha_256_expected));
                        memcpy(expected.buffer, sha_256_expected, expected.size);
                        digest = SKM_DIGEST_SHA_2_256;
                        break;
                    case 2 :
                        TEST_ALLOCATE_BLOCK(expected, sizeof(sha_384_expected));
                        memcpy(expected.buffer, sha_384_expected, expected.size);
                        digest = SKM_DIGEST_SHA_2_384;
                        break;
                    case 3 :
                        TEST_ALLOCATE_BLOCK(expected, sizeof(sha_512_expected));
                        memcpy(expected.buffer, sha_512_expected, expected.size);
                        digest = SKM_DIGEST_SHA_2_512;
                        break;
                    }
                    EXPECT_SUCCESS(km_crypto_sign_vector_test(handle, key_data, message, digest, expected));

                    TEST_FREE_BLOCK(expected);
                }
                break;
            }
        case 5:
            {
                char *message_string = "Test Using Larger Than Block-Size Key - Hash Key First";

                uint8_t sha_224_expected[] = {
                    0x05, 0xb0, 0x1d, 0xf0, 0x41, 0x6b, 0x54, 0xb6, 0x68, 0xec, 0xbd, 0x5f, 0x3f,
                    0x2f, 0x9f, 0x65, 0x22, 0x00, 0xe0, 0x44, 0x58, 0xdf, 0xa6, 0xe2, 0x1f, 0x25,
                    0x3b, 0x81,
                };
                uint8_t sha_256_expected[] = {
                    0x84, 0x33, 0x2a, 0x75, 0x80, 0xed, 0x3c, 0xf7, 0x5d, 0xe8, 0x3c, 0x64, 0x4c,
                    0x8d, 0x2c, 0x1c, 0x26, 0x2a, 0xd9, 0x0e, 0x01, 0x90, 0xe5, 0xc5, 0xae, 0x4b,
                    0x82, 0xb2, 0x10, 0x2e, 0x8e, 0x75,
                };
                uint8_t sha_384_expected[] = {
                    0x30, 0x67, 0x92, 0xdd, 0x87, 0xe9, 0x9a, 0x24, 0xd7, 0xfd, 0x2f, 0x8f, 0xee,
                    0x29, 0x50, 0xf9, 0x72, 0x5f, 0x6b, 0xb1, 0xc7, 0x63, 0x63, 0xdc, 0x02, 0x77,
                    0x65, 0xab, 0x8e, 0xcf, 0x0e, 0x22, 0x49, 0x8c, 0xbc, 0x4d, 0x61, 0x8f, 0x8b,
                    0x7f, 0x1e, 0xde, 0xe0, 0x8c, 0x37, 0x52, 0x2c, 0xc7,
                };
                uint8_t sha_512_expected[] = {
                    0xc8, 0x75, 0xfd, 0x3b, 0xda, 0xed, 0xf8, 0x41, 0xdb, 0xa9, 0x29, 0x22, 0x3c,
                    0xfe, 0x23, 0xef, 0xcb, 0x3a, 0x3c, 0xc8, 0x6c, 0x07, 0x91, 0x58, 0x90, 0x2b,
                    0xaf, 0xe2, 0x81, 0x29, 0x07, 0x5c, 0xc4, 0x32, 0x65, 0x10, 0x04, 0xe4, 0xcc,
                    0x5d, 0x17, 0x54, 0x26, 0x51, 0x67, 0x4d, 0xfc, 0xe0, 0xfc, 0x29, 0xbf, 0x0a,
                    0x01, 0x06, 0x28, 0xd9, 0x9a, 0x62, 0xc3, 0x1b, 0x0c, 0x91, 0x54, 0x98,
                };

                TEST_ALLOCATE_BLOCK(key_data, 64);
                memset(key_data.buffer, 0xaa, key_data.size);

                TEST_ALLOCATE_BLOCK(message, strlen(message_string));
                memcpy(message.buffer, message_string, message.size);

                for (j = 0; j < 4; j++) {
                    switch (j) {
                    case 0 :
                        TEST_ALLOCATE_BLOCK(expected, sizeof(sha_224_expected));
                        memcpy(expected.buffer, sha_224_expected, expected.size);
                        digest = SKM_DIGEST_SHA_2_224;
                        break;
                    case 1 :
                        TEST_ALLOCATE_BLOCK(expected, sizeof(sha_256_expected));
                        memcpy(expected.buffer, sha_256_expected, expected.size);
                        digest = SKM_DIGEST_SHA_2_256;
                        break;
                    case 2 :
                        TEST_ALLOCATE_BLOCK(expected, sizeof(sha_384_expected));
                        memcpy(expected.buffer, sha_384_expected, expected.size);
                        digest = SKM_DIGEST_SHA_2_384;
                        break;
                    case 3 :
                        TEST_ALLOCATE_BLOCK(expected, sizeof(sha_512_expected));
                        memcpy(expected.buffer, sha_512_expected, expected.size);
                        digest = SKM_DIGEST_SHA_2_512;
                        break;
                    }
                    EXPECT_SUCCESS(km_crypto_sign_vector_test(handle, key_data, message, digest, expected));

                    TEST_FREE_BLOCK(expected);
                }
                break;
            }
        case 6:
            {
                char *message_string = "This is a test using a larger than block-size key and a larger than block-size data. The key needs to be hashed before being used by the HMAC algorithm.";

                uint8_t sha_224_expected[] = {
                    0x4b, 0x3a, 0x1f, 0x11, 0x9a, 0x4e, 0x05, 0x69, 0x10, 0x49, 0x15,
                    0x79, 0xcb, 0x9b, 0x05, 0x53, 0x1f, 0x6d, 0xe5, 0x98, 0xee, 0x0b,
                    0xb5, 0xa3, 0x82, 0xc0, 0xd9, 0x4a,
                };
                uint8_t sha_256_expected[] = {
                    0x20, 0xce, 0xaf, 0xe1, 0xd2, 0xf6, 0xc4, 0xb4, 0x9f, 0xe4, 0xaa,
                    0xb5, 0x8a, 0xd8, 0xaf, 0x4d, 0x16, 0xda, 0x60, 0xac, 0x09, 0x6d,
                    0xd0, 0x8f, 0x0f, 0xd7, 0x45, 0xe8, 0x97, 0x60, 0x1f, 0x52,
                };
                uint8_t sha_384_expected[] = {
                    0xc4, 0x97, 0x34, 0xb6, 0xbd, 0x8c, 0xd5, 0x36, 0xeb, 0x59, 0x96,
                    0xea, 0x25, 0x07, 0xff, 0x3e, 0x87, 0xe8, 0x99, 0xe0, 0x1c, 0xa4,
                    0xf9, 0x7b, 0xa1, 0xe6, 0xd2, 0x19, 0xcd, 0x67, 0x5c, 0x0a, 0xff,
                    0x34, 0xc6, 0xec, 0xae, 0x46, 0x59, 0x7e, 0x3f, 0xf9, 0x89, 0xae,
                    0xaf, 0xe2, 0xd4, 0xd0,
                };
                uint8_t sha_512_expected[] = {
                    0xc8, 0xbe, 0xf9, 0xc1, 0x11, 0x33, 0x85, 0x76, 0x9a, 0xea, 0xb7,
                    0x73, 0x69, 0x2d, 0xb9, 0x81, 0x2d, 0x82, 0xb2, 0x74, 0x2e, 0x23,
                    0x97, 0xcf, 0x12, 0x1f, 0x92, 0x92, 0xd2, 0x50, 0x20, 0xda, 0xbd,
                    0x3f, 0xff, 0x50, 0xaf, 0x1a, 0xc3, 0xcc, 0xd0, 0xed, 0x74, 0x7d,
                    0xd2, 0x4c, 0x0e, 0xf3, 0x86, 0xc6, 0xd9, 0x68, 0x31, 0x6b, 0xc2,
                    0x7f, 0x1d, 0x28, 0x60, 0x47, 0x35, 0x2e, 0x81, 0xba,
                };

                TEST_ALLOCATE_BLOCK(key_data, 64);
                memset(key_data.buffer, 0xaa, key_data.size);

                TEST_ALLOCATE_BLOCK(message, strlen(message_string));
                memcpy(message.buffer, message_string, message.size);

                for (j = 0; j < 4; j++) {
                    switch (j) {
                    case 0 :
                        TEST_ALLOCATE_BLOCK(expected, sizeof(sha_224_expected));
                        memcpy(expected.buffer, sha_224_expected, expected.size);
                        digest = SKM_DIGEST_SHA_2_224;
                        break;
                    case 1 :
                        TEST_ALLOCATE_BLOCK(expected, sizeof(sha_256_expected));
                        memcpy(expected.buffer, sha_256_expected, expected.size);
                        digest = SKM_DIGEST_SHA_2_256;
                        break;
                    case 2 :
                        TEST_ALLOCATE_BLOCK(expected, sizeof(sha_384_expected));
                        memcpy(expected.buffer, sha_384_expected, expected.size);
                        digest = SKM_DIGEST_SHA_2_384;
                        break;
                    case 3 :
                        TEST_ALLOCATE_BLOCK(expected, sizeof(sha_512_expected));
                        memcpy(expected.buffer, sha_512_expected, expected.size);
                        digest = SKM_DIGEST_SHA_2_512;
                        break;
                    }
                    EXPECT_SUCCESS(km_crypto_sign_vector_test(handle, key_data, message, digest, expected));

                    TEST_FREE_BLOCK(expected);
                }
                break;
            }
        default:
            BDBG_ERR(("%s:%d Internal error", BSTD_FUNCTION, __LINE__));
            err = BERR_UNKNOWN;
            goto done;
        }
        TEST_FREE_BLOCK(key_data);
        TEST_FREE_BLOCK(message);


    }
    err = BERR_SUCCESS;
done:
    TEST_FREE_BLOCK(expected);
    TEST_FREE_BLOCK(key_data);
    TEST_FREE_BLOCK(message);
    return err;
}

BERR_Code km_crypto_hmac_tests(KeymasterTl_Handle handle)
{
    BERR_Code err;

    EXPECT_SUCCESS(km_crypto_hmac_sign_verify_test(handle));
    EXPECT_SUCCESS(km_crypto_hmac_generate_fails(handle));
    EXPECT_SUCCESS(km_crypto_hmac_sign_fails(handle));
    EXPECT_SUCCESS(km_crypto_hmac_vectors(handle));

done:
    return err;
}
