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


BDBG_MODULE(keymaster_crypto_ec);

/* Key size of 521 requires 141 bytes, so just allocate that for all */
#define EC_MAX_SIGNATURE_SIZE   141

static BERR_Code km_crypto_ec_sign_verify_test(KeymasterTl_Handle handle)
{
    BERR_Code err;
    KM_Tag_ContextHandle key_params = NULL;
    KM_Tag_ContextHandle begin_params = NULL;
    KM_CryptoOperation_Settings settings;
    KeymasterTl_DataBlock key;
    KeymasterTl_DataBlock in_data;
    KeymasterTl_DataBlock out_data;
    KeymasterTl_DataBlock signature_data;
    uint32_t data_size;
    int i;
    const char *comment;

    memset(&key, 0, sizeof(key));
    memset(&in_data, 0, sizeof(in_data));
    memset(&out_data, 0, sizeof(out_data));
    memset(&signature_data, 0, sizeof(signature_data));

    BDBG_LOG(("----------------------- %s -----------------------", BSTD_FUNCTION));
    for (i = 0; i < 24; i++) {

        KM_CryptoOperation_GetDefaultSettings(&settings);
        settings.handle = handle;

        EXPECT_SUCCESS(KM_Tag_NewContext(&begin_params));

        settings.begin_params = begin_params;
        TEST_TAG_ADD_ENUM(settings.begin_params, KM_TAG_PADDING, KM_PAD_NONE);

        switch (i) {
        case 0:
            EXPECT_SUCCESS(km_test_new_params_with_ec_defaults(&key_params, 224));
            TEST_TAG_ADD_ENUM(settings.begin_params, KM_TAG_DIGEST, KM_DIGEST_NONE);
            comment = "EC 224 NONE";
            data_size = 224 / 8;
            break;
        case 1:
            EXPECT_SUCCESS(km_test_new_params_with_ec_defaults(&key_params, 224));
            TEST_TAG_ADD_ENUM(settings.begin_params, KM_TAG_DIGEST, KM_DIGEST_SHA_2_256);
            comment = "EC 224 SHA256";
            data_size = 1024;
            break;
        case 2:
            EXPECT_SUCCESS(km_test_new_params_with_ec_defaults(&key_params, 224));
            TEST_TAG_ADD_ENUM(settings.begin_params, KM_TAG_DIGEST, KM_DIGEST_SHA_2_384);
            comment = "EC 224 SHA384";
            data_size = 1024;
            break;
        case 3:
            EXPECT_SUCCESS(km_test_new_params_with_ec_defaults(&key_params, 224));
            TEST_TAG_ADD_ENUM(settings.begin_params, KM_TAG_DIGEST, KM_DIGEST_NONE);
            comment = "EC 224 NONE, large data";
            data_size = 64 * 1024;
            break;
        case 4:
            EXPECT_SUCCESS(km_test_new_params_with_ec_defaults(&key_params, 224));
            TEST_TAG_ADD_ENUM(settings.begin_params, KM_TAG_DIGEST, KM_DIGEST_SHA1);
            comment = "EC 224 SHA1";
            data_size = 1024;
            break;
        case 5:
            EXPECT_SUCCESS(km_test_new_params_with_ec_defaults(&key_params, 224));
            TEST_TAG_ADD_ENUM(settings.begin_params, KM_TAG_DIGEST, KM_DIGEST_SHA_2_224);
            comment = "EC 224 SHA224";
            data_size = 1024;
            break;
        case 6:
            EXPECT_SUCCESS(km_test_new_params_with_ec_defaults(&key_params, 224));
            TEST_TAG_ADD_ENUM(settings.begin_params, KM_TAG_DIGEST, KM_DIGEST_SHA_2_256);
            comment = "EC 224 SHA256";
            data_size = 1024;
            break;
        case 7:
            EXPECT_SUCCESS(km_test_new_params_with_ec_defaults(&key_params, 224));
            TEST_TAG_ADD_ENUM(settings.begin_params, KM_TAG_DIGEST, KM_DIGEST_SHA_2_384);
            comment = "EC 224 SHA384";
            data_size = 1024;
            break;
        case 8:
            EXPECT_SUCCESS(km_test_new_params_with_ec_defaults(&key_params, 224));
            TEST_TAG_ADD_ENUM(settings.begin_params, KM_TAG_DIGEST, KM_DIGEST_SHA_2_512);
            comment = "EC 224 SHA512";
            data_size = 1024;
            break;
        case 9:
            EXPECT_SUCCESS(km_test_new_params_with_ec_defaults(&key_params, 256));
            TEST_TAG_ADD_ENUM(settings.begin_params, KM_TAG_DIGEST, KM_DIGEST_SHA1);
            comment = "EC 256 SHA1";
            data_size = 1024;
            break;
        case 10:
            EXPECT_SUCCESS(km_test_new_params_with_ec_defaults(&key_params, 256));
            TEST_TAG_ADD_ENUM(settings.begin_params, KM_TAG_DIGEST, KM_DIGEST_SHA_2_224);
            comment = "EC 256 SHA224";
            data_size = 1024;
            break;
        case 11:
            EXPECT_SUCCESS(km_test_new_params_with_ec_defaults(&key_params, 256));
            TEST_TAG_ADD_ENUM(settings.begin_params, KM_TAG_DIGEST, KM_DIGEST_SHA_2_256);
            comment = "EC 256 SHA256";
            data_size = 1024;
            break;
        case 12:
            EXPECT_SUCCESS(km_test_new_params_with_ec_defaults(&key_params, 256));
            TEST_TAG_ADD_ENUM(settings.begin_params, KM_TAG_DIGEST, KM_DIGEST_SHA_2_384);
            comment = "EC 256 SHA384";
            data_size = 1024;
            break;
        case 13:
            EXPECT_SUCCESS(km_test_new_params_with_ec_defaults(&key_params, 256));
            TEST_TAG_ADD_ENUM(settings.begin_params, KM_TAG_DIGEST, KM_DIGEST_SHA_2_512);
            comment = "EC 256 SHA512";
            data_size = 1024;
            break;
        case 14:
            EXPECT_SUCCESS(km_test_new_params_with_ec_defaults(&key_params, 384));
            TEST_TAG_ADD_ENUM(settings.begin_params, KM_TAG_DIGEST, KM_DIGEST_SHA1);
            comment = "EC 384 SHA1";
            data_size = 1024;
            break;
        case 15:
            EXPECT_SUCCESS(km_test_new_params_with_ec_defaults(&key_params, 384));
            TEST_TAG_ADD_ENUM(settings.begin_params, KM_TAG_DIGEST, KM_DIGEST_SHA_2_224);
            comment = "EC 384 SHA224";
            data_size = 1024;
            break;
        case 16:
            EXPECT_SUCCESS(km_test_new_params_with_ec_defaults(&key_params, 384));
            TEST_TAG_ADD_ENUM(settings.begin_params, KM_TAG_DIGEST, KM_DIGEST_SHA_2_256);
            comment = "EC 384 SHA256";
            data_size = 1024;
            break;
        case 17:
            EXPECT_SUCCESS(km_test_new_params_with_ec_defaults(&key_params, 384));
            TEST_TAG_ADD_ENUM(settings.begin_params, KM_TAG_DIGEST, KM_DIGEST_SHA_2_384);
            comment = "EC 384 SHA384";
            data_size = 1024;
            break;
        case 18:
            EXPECT_SUCCESS(km_test_new_params_with_ec_defaults(&key_params, 384));
            TEST_TAG_ADD_ENUM(settings.begin_params, KM_TAG_DIGEST, KM_DIGEST_SHA_2_512);
            comment = "EC 384 SHA512";
            data_size = 1024;
            break;
        case 19:
            EXPECT_SUCCESS(km_test_new_params_with_ec_defaults(&key_params, 521));
            TEST_TAG_ADD_ENUM(settings.begin_params, KM_TAG_DIGEST, KM_DIGEST_SHA1);
            comment = "EC 521 SHA1";
            data_size = 1024;
            break;
        case 20:
            EXPECT_SUCCESS(km_test_new_params_with_ec_defaults(&key_params, 521));
            TEST_TAG_ADD_ENUM(settings.begin_params, KM_TAG_DIGEST, KM_DIGEST_SHA_2_224);
            comment = "EC 521 SHA224";
            data_size = 1024;
            break;
        case 21:
            EXPECT_SUCCESS(km_test_new_params_with_ec_defaults(&key_params, 521));
            TEST_TAG_ADD_ENUM(settings.begin_params, KM_TAG_DIGEST, KM_DIGEST_SHA_2_256);
            comment = "EC 521 SHA256";
            data_size = 1024;
            break;
        case 22:
            EXPECT_SUCCESS(km_test_new_params_with_ec_defaults(&key_params, 521));
            TEST_TAG_ADD_ENUM(settings.begin_params, KM_TAG_DIGEST, KM_DIGEST_SHA_2_384);
            comment = "EC 521 SHA384";
            data_size = 1024;
            break;
        case 23:
            EXPECT_SUCCESS(km_test_new_params_with_ec_defaults(&key_params, 521));
            TEST_TAG_ADD_ENUM(settings.begin_params, KM_TAG_DIGEST, KM_DIGEST_SHA_2_512);
            comment = "EC 521 SHA512";
            data_size = 1024;
            break;
        default:
            BDBG_ERR(("%s:%d Internal error", BSTD_FUNCTION, __LINE__));
            err = BERR_UNKNOWN;
            goto done;
        }
        BDBG_LOG(("%s : Test - %s", BSTD_FUNCTION, comment));

        EXPECT_SUCCESS(KeymasterTl_GenerateKey(handle, key_params, &key));

        settings.key_params = key_params;
        settings.in_key = key;

        TEST_ALLOCATE_BLOCK(in_data, data_size);
        TEST_ALLOCATE_BLOCK(out_data, EC_MAX_SIGNATURE_SIZE);
        memset(in_data.buffer, 'a', in_data.size);
        memset(out_data.buffer, 0xFF, out_data.size);
        settings.in_data = in_data;
        settings.out_data = out_data;

        EXPECT_SUCCESS(KM_Crypto_Operation(KM_PURPOSE_SIGN, &settings));

        if (settings.out_data.size == 0) {
            BDBG_ERR(("NO SIGNATURE GENERATED FAILED: %s", comment));
            continue;
        }

        TEST_ALLOCATE_BLOCK(signature_data, settings.out_data.size);
        memcpy(signature_data.buffer, settings.out_data.buffer, signature_data.size);
        settings.out_data.buffer = NULL;
        settings.out_data.size = 0;
        settings.signature_data = signature_data;

        EXPECT_SUCCESS(KM_Crypto_Operation(KM_PURPOSE_VERIFY, &settings));

        settings.signature_data.buffer[0]++;
        EXPECT_FAILURE_CODE(KM_Crypto_Operation(KM_PURPOSE_VERIFY, &settings), BSAGE_ERR_KM_VERIFICATION_FAILED);

        settings.signature_data.buffer[0]--;
        settings.in_data.buffer[0]++;
        EXPECT_FAILURE_CODE(KM_Crypto_Operation(KM_PURPOSE_VERIFY, &settings), BSAGE_ERR_KM_VERIFICATION_FAILED);

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

static BERR_Code km_crypto_ec_generate_fails(KeymasterTl_Handle handle)
{
    BERR_Code err;
    KM_Tag_ContextHandle key_params = NULL;
    KeymasterTl_DataBlock key;
    int i;
    const char *comment;
    BERR_Code expected_err;

    memset(&key, 0, sizeof(key));

    BDBG_LOG(("----------------------- %s -----------------------", BSTD_FUNCTION));
    for (i = 0; i < 3; i++) {

        switch (i) {
        case 0:
            EXPECT_SUCCESS(km_test_new_params_with_ec_defaults(&key_params, 224));
            km_test_remove_key_size(key_params);
            km_test_remove_curve(key_params);
            comment = "EC No key size";
            expected_err = BSAGE_ERR_KM_UNSUPPORTED_KEY_SIZE;
            break;
        case 1:
            EXPECT_SUCCESS(KM_Tag_NewContext(&key_params));
            TEST_TAG_ADD_ENUM(key_params, KM_TAG_ALGORITHM, KM_ALGORITHM_EC);
            TEST_TAG_ADD_INTEGER(key_params, KM_TAG_KEY_SIZE, 190);
            TEST_TAG_ADD_ENUM(key_params, KM_TAG_PURPOSE, KM_PURPOSE_SIGN);
            TEST_TAG_ADD_ENUM(key_params, KM_TAG_PURPOSE, KM_PURPOSE_VERIFY);
            TEST_TAG_ADD_ENUM(key_params, KM_TAG_DIGEST, KM_DIGEST_NONE);
            TEST_TAG_ADD_ENUM(key_params, KM_TAG_DIGEST, KM_DIGEST_MD5);
            TEST_TAG_ADD_ENUM(key_params, KM_TAG_DIGEST, KM_DIGEST_SHA1);
            TEST_TAG_ADD_ENUM(key_params, KM_TAG_DIGEST, KM_DIGEST_SHA_2_224);
            TEST_TAG_ADD_ENUM(key_params, KM_TAG_DIGEST, KM_DIGEST_SHA_2_256);
            TEST_TAG_ADD_ENUM(key_params, KM_TAG_DIGEST, KM_DIGEST_SHA_2_384);
            TEST_TAG_ADD_ENUM(key_params, KM_TAG_DIGEST, KM_DIGEST_SHA_2_512);
            TEST_TAG_ADD_BOOL(key_params, KM_TAG_ALL_USERS, true);
            TEST_TAG_ADD_BOOL(key_params, KM_TAG_NO_AUTH_REQUIRED, true);
            TEST_TAG_ADD_BOOL(key_params, KM_TAG_ALL_APPLICATIONS, true);
            comment = "EC Invalid key size";
            expected_err = BSAGE_ERR_KM_UNSUPPORTED_KEY_SIZE;
            break;
        case 2:
            EXPECT_SUCCESS(km_test_new_params_with_ec_defaults(&key_params, 224));
            TEST_TAG_ADD_ENUM(key_params, KM_TAG_DIGEST, KM_DIGEST_NONE);
            km_test_remove_curve(key_params);
            TEST_TAG_ADD_ENUM(key_params, KM_TAG_EC_CURVE, KM_EC_CURVE_P_256);
            comment = "EC Mismatched key size";
            expected_err = BERR_INVALID_PARAMETER;
            break;
        default:
            BDBG_ERR(("%s:%d Internal error", BSTD_FUNCTION, __LINE__));
            err = BERR_UNKNOWN;
            goto done;
        }
        BDBG_LOG(("%s : Test - %s", BSTD_FUNCTION, comment));

        EXPECT_FAILURE_CODE(KeymasterTl_GenerateKey(handle, key_params, &key), expected_err);
        TEST_DELETE_CONTEXT(key_params);

        BDBG_LOG(("%s: %s success", BSTD_FUNCTION, comment));
    }
    err = BERR_SUCCESS;

done:
    TEST_FREE_BLOCK(key);
    TEST_DELETE_CONTEXT(key_params);
    return err;
}

BERR_Code km_crypto_ec_tests(KeymasterTl_Handle handle)
{
    BERR_Code err;

    EXPECT_SUCCESS(km_crypto_ec_sign_verify_test(handle));
    EXPECT_SUCCESS(km_crypto_ec_generate_fails(handle));

done:
    return err;
}
