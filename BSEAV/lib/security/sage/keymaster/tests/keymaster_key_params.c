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

#include "bstd.h"
#include "bkni.h"

#include "keymaster_ids.h"
#include "keymaster_platform.h"
#include "keymaster_tl.h"
#include "keymaster_test.h"
#include "keymaster_key_params.h"


BDBG_MODULE(keymaster_key_params);


#define GCM_MIN_MAC_SIZE   128

#define HMAC_MIN_MAC_SIZE  128

#define DEFAULT_APP_ID     "MyAppId"
#define DEFAULT_APP_DATA   "MyAppData"


BERR_Code km_test_aes_add_default_params(KM_Tag_ContextHandle key_params, uint32_t key_size)
{
    BERR_Code err;
    BDBG_ASSERT(key_params);

    TEST_TAG_ADD_ENUM(key_params, KM_TAG_ALGORITHM, KM_ALGORITHM_AES);
    TEST_TAG_ADD_INTEGER(key_params, KM_TAG_KEY_SIZE, key_size);
    TEST_TAG_ADD_ENUM(key_params, KM_TAG_PURPOSE, KM_PURPOSE_ENCRYPT);
    TEST_TAG_ADD_ENUM(key_params, KM_TAG_PURPOSE, KM_PURPOSE_DECRYPT);
    TEST_TAG_ADD_ENUM(key_params, KM_TAG_PADDING, KM_PAD_NONE);
    TEST_TAG_ADD_ENUM(key_params, KM_TAG_PADDING, KM_PAD_PKCS7);
    TEST_TAG_ADD_ENUM(key_params, KM_TAG_BLOCK_MODE, KM_MODE_ECB);
    TEST_TAG_ADD_ENUM(key_params, KM_TAG_BLOCK_MODE, KM_MODE_CBC);
    TEST_TAG_ADD_ENUM(key_params, KM_TAG_BLOCK_MODE, KM_MODE_CTR);
    TEST_TAG_ADD_ENUM(key_params, KM_TAG_BLOCK_MODE, KM_MODE_GCM);
    TEST_TAG_ADD_INTEGER(key_params, KM_TAG_MIN_MAC_LENGTH, GCM_MIN_MAC_SIZE);
    TEST_TAG_ADD_BOOL(key_params, KM_TAG_CALLER_NONCE, true);

    TEST_TAG_ADD_BOOL(key_params, KM_TAG_ALL_USERS, true);
    TEST_TAG_ADD_BOOL(key_params, KM_TAG_NO_AUTH_REQUIRED, true);
    TEST_TAG_ADD_BOOL(key_params, KM_TAG_ALL_APPLICATIONS, true);
    err = BERR_SUCCESS;

done:
    return err;
}

BERR_Code km_test_hmac_add_default_params(KM_Tag_ContextHandle key_params, uint32_t key_size)
{
    BERR_Code err;
    BDBG_ASSERT(key_params);

    TEST_TAG_ADD_ENUM(key_params, KM_TAG_ALGORITHM, KM_ALGORITHM_HMAC);
    TEST_TAG_ADD_INTEGER(key_params, KM_TAG_KEY_SIZE, key_size);
    TEST_TAG_ADD_ENUM(key_params, KM_TAG_PURPOSE, KM_PURPOSE_SIGN);
    TEST_TAG_ADD_ENUM(key_params, KM_TAG_PURPOSE, KM_PURPOSE_VERIFY);
    TEST_TAG_ADD_ENUM(key_params, KM_TAG_DIGEST, KM_DIGEST_MD5);
    TEST_TAG_ADD_ENUM(key_params, KM_TAG_DIGEST, KM_DIGEST_SHA1);
    TEST_TAG_ADD_ENUM(key_params, KM_TAG_DIGEST, KM_DIGEST_SHA_2_224);
    TEST_TAG_ADD_ENUM(key_params, KM_TAG_DIGEST, KM_DIGEST_SHA_2_256);
    TEST_TAG_ADD_ENUM(key_params, KM_TAG_DIGEST, KM_DIGEST_SHA_2_384);
    TEST_TAG_ADD_ENUM(key_params, KM_TAG_DIGEST, KM_DIGEST_SHA_2_512);
    TEST_TAG_ADD_INTEGER(key_params, KM_TAG_MIN_MAC_LENGTH, HMAC_MIN_MAC_SIZE);

    TEST_TAG_ADD_BOOL(key_params, KM_TAG_ALL_USERS, true);
    TEST_TAG_ADD_BOOL(key_params, KM_TAG_NO_AUTH_REQUIRED, true);
    TEST_TAG_ADD_BOOL(key_params, KM_TAG_ALL_APPLICATIONS, true);
    err = BERR_SUCCESS;

done:
    return err;
}

BERR_Code km_test_rsa_add_default_params(KM_Tag_ContextHandle key_params, uint32_t key_size)
{
    BERR_Code err;
    BDBG_ASSERT(key_params);

    TEST_TAG_ADD_ENUM(key_params, KM_TAG_ALGORITHM, KM_ALGORITHM_RSA);
    TEST_TAG_ADD_INTEGER(key_params, KM_TAG_KEY_SIZE, key_size);
    TEST_TAG_ADD_LONG_INTEGER(key_params, KM_TAG_RSA_PUBLIC_EXPONENT, 65537ULL);
    TEST_TAG_ADD_ENUM(key_params, KM_TAG_PURPOSE, KM_PURPOSE_SIGN);
    TEST_TAG_ADD_ENUM(key_params, KM_TAG_PURPOSE, KM_PURPOSE_VERIFY);
    TEST_TAG_ADD_ENUM(key_params, KM_TAG_PURPOSE, KM_PURPOSE_ENCRYPT);
    TEST_TAG_ADD_ENUM(key_params, KM_TAG_PURPOSE, KM_PURPOSE_DECRYPT);
    TEST_TAG_ADD_ENUM(key_params, KM_TAG_DIGEST, KM_DIGEST_MD5);
    TEST_TAG_ADD_ENUM(key_params, KM_TAG_DIGEST, KM_DIGEST_SHA1);
    TEST_TAG_ADD_ENUM(key_params, KM_TAG_DIGEST, KM_DIGEST_SHA_2_224);
    TEST_TAG_ADD_ENUM(key_params, KM_TAG_DIGEST, KM_DIGEST_SHA_2_256);
    TEST_TAG_ADD_ENUM(key_params, KM_TAG_DIGEST, KM_DIGEST_SHA_2_384);
    TEST_TAG_ADD_ENUM(key_params, KM_TAG_DIGEST, KM_DIGEST_SHA_2_512);
    TEST_TAG_ADD_ENUM(key_params, KM_TAG_PADDING, KM_PAD_NONE);
    TEST_TAG_ADD_ENUM(key_params, KM_TAG_PADDING, KM_PAD_RSA_OAEP);
    TEST_TAG_ADD_ENUM(key_params, KM_TAG_PADDING, KM_PAD_RSA_PKCS1_1_5_SIGN);
    TEST_TAG_ADD_ENUM(key_params, KM_TAG_PADDING, KM_PAD_RSA_PKCS1_1_5_ENCRYPT);
    TEST_TAG_ADD_ENUM(key_params, KM_TAG_PADDING, KM_PAD_RSA_PSS);

    TEST_TAG_ADD_BOOL(key_params, KM_TAG_ALL_USERS, true);
    TEST_TAG_ADD_BOOL(key_params, KM_TAG_NO_AUTH_REQUIRED, true);
    TEST_TAG_ADD_BOOL(key_params, KM_TAG_ALL_APPLICATIONS, true);
    err = BERR_SUCCESS;

done:
    return err;
}

BERR_Code km_test_ec_add_default_params(KM_Tag_ContextHandle key_params, uint32_t key_size)
{
    BERR_Code err;
    BDBG_ASSERT(key_params);

    TEST_TAG_ADD_ENUM(key_params, KM_TAG_ALGORITHM, KM_ALGORITHM_EC);
    TEST_TAG_ADD_INTEGER(key_params, KM_TAG_KEY_SIZE, key_size);
    switch (key_size) {
    case 224:
        TEST_TAG_ADD_ENUM(key_params, KM_TAG_EC_CURVE, KM_EC_CURVE_P_224);
        break;
    case 256:
        TEST_TAG_ADD_ENUM(key_params, KM_TAG_EC_CURVE, KM_EC_CURVE_P_256);
        break;
    case 384:
        TEST_TAG_ADD_ENUM(key_params, KM_TAG_EC_CURVE, KM_EC_CURVE_P_384);
        break;
    case 521:
        TEST_TAG_ADD_ENUM(key_params, KM_TAG_EC_CURVE, KM_EC_CURVE_P_521);
        break;
    default:
        /* Don't error, just don't put in the curve */
        break;
    }
    TEST_TAG_ADD_ENUM(key_params, KM_TAG_PURPOSE, KM_PURPOSE_SIGN);
    TEST_TAG_ADD_ENUM(key_params, KM_TAG_PURPOSE, KM_PURPOSE_VERIFY);
    TEST_TAG_ADD_ENUM(key_params, KM_TAG_DIGEST, KM_DIGEST_MD5);
    TEST_TAG_ADD_ENUM(key_params, KM_TAG_DIGEST, KM_DIGEST_SHA1);
    TEST_TAG_ADD_ENUM(key_params, KM_TAG_DIGEST, KM_DIGEST_SHA_2_224);
    TEST_TAG_ADD_ENUM(key_params, KM_TAG_DIGEST, KM_DIGEST_SHA_2_256);
    TEST_TAG_ADD_ENUM(key_params, KM_TAG_DIGEST, KM_DIGEST_SHA_2_384);
    TEST_TAG_ADD_ENUM(key_params, KM_TAG_DIGEST, KM_DIGEST_SHA_2_512);

    TEST_TAG_ADD_BOOL(key_params, KM_TAG_ALL_USERS, true);
    TEST_TAG_ADD_BOOL(key_params, KM_TAG_NO_AUTH_REQUIRED, true);
    TEST_TAG_ADD_BOOL(key_params, KM_TAG_ALL_APPLICATIONS, true);
    err = BERR_SUCCESS;

done:
    return err;
}


/* Modifiers */

BERR_Code km_test_remove_key_size(KM_Tag_ContextHandle key_params)
{
    BERR_Code err;
    BDBG_ASSERT(key_params);
    TEST_TAG_REMOVE(key_params, KM_TAG_KEY_SIZE);

done:
    return err;
}

BERR_Code km_test_remove_curve(KM_Tag_ContextHandle key_params)
{
    BERR_Code err;
    BDBG_ASSERT(key_params);
    TEST_TAG_REMOVE(key_params, KM_TAG_EC_CURVE);

done:
    return err;
}

BERR_Code km_test_remove_exponent(KM_Tag_ContextHandle key_params)
{
    BERR_Code err;
    BDBG_ASSERT(key_params);
    TEST_TAG_REMOVE(key_params, KM_TAG_RSA_PUBLIC_EXPONENT);

done:
    return err;
}

BERR_Code km_test_remove_all_apps(KM_Tag_ContextHandle key_params)
{
    BERR_Code err;
    BDBG_ASSERT(key_params);
    TEST_TAG_REMOVE(key_params, KM_TAG_ALL_APPLICATIONS);

done:
    return err;
}

BERR_Code km_test_add_app_id(KM_Tag_ContextHandle key_params)
{
    BERR_Code err;
    BDBG_ASSERT(key_params);
    TEST_TAG_ADD_BYTES(key_params, KM_TAG_APPLICATION_ID, strlen(DEFAULT_APP_ID), (uint8_t*)DEFAULT_APP_ID);

done:
    return err;
}

BERR_Code km_test_add_app_data(KM_Tag_ContextHandle key_params)
{
    BERR_Code err;
    BDBG_ASSERT(key_params);
    TEST_TAG_ADD_BYTES(key_params, KM_TAG_APPLICATION_DATA, strlen(DEFAULT_APP_DATA), (uint8_t*)DEFAULT_APP_DATA);

done:
    return err;
}

BERR_Code km_test_update_with_secure_id(KM_Tag_ContextHandle key_params)
{
    /* TODO: add KM_TAG_USER_SECURE_ID, KM_TAG_USER_AUTH_TYPE, KM_TAG_AUTH_TIMEOUT */
    return BERR_UNKNOWN;
}

BERR_Code km_test_copy_app_id_and_data(KM_Tag_ContextHandle key_params, KM_Tag_ContextHandle *params)
{
    BERR_Code err;
    km_tag_value_t *tag;

    BDBG_ASSERT(key_params);
    BDBG_ASSERT(params);

    err = KM_Tag_NewContext(params);
    if (err != BERR_SUCCESS) {
        goto done;
    }

    tag = KM_Tag_FindFirst(key_params, KM_TAG_APPLICATION_ID);
    if (tag) {
        err = KM_Tag_Add(*params, tag);
        if (err != BERR_SUCCESS) {
            goto done;
        }
    }
    tag = KM_Tag_FindFirst(key_params, KM_TAG_APPLICATION_DATA);
    if (tag) {
        err = KM_Tag_Add(*params, tag);
        if (err != BERR_SUCCESS) {
            goto done;
        }
    }
    err = BERR_SUCCESS;

done:
    if (err != BERR_SUCCESS) {
        KM_Tag_DeleteContext(*params);
        *params = NULL;
    }
    return err;
}
