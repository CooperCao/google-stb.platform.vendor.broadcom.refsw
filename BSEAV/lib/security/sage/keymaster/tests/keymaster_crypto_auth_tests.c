/******************************************************************************
 *  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#include "gatekeeper_tl.h"
#include "keymaster_test.h"
#include "keymaster_crypto_utils.h"
#include "keymaster_crypto_auth_tests.h"
#include "keymaster_key_params.h"


BDBG_MODULE(keymaster_crypto_auth_tests);

#define AUTH_TIMEOUT_1_SECOND (1)
#define TEST_BLOCK_SIZE (4096)

/* Allocate for worst case (depends on TAG_MAC_LENGTH which is max 512 bits) */
#define MAX_SIGNATURE_SIZE  64

static BERR_Code km_crypto_hmac_auth_test(KeymasterTl_Handle handle, GatekeeperTl_Handle gkHandle)
{
    BERR_Code err;
    KM_Tag_ContextHandle key_params = NULL;
    KM_Tag_ContextHandle begin_params = NULL;
    KM_Tag_ContextHandle begin_params2 = NULL;
    KM_CryptoOperation_Settings settings;
    KeymasterTl_DataBlock key;
    KeymasterTl_DataBlock in_data;
    KeymasterTl_DataBlock out_data;
    KeymasterTl_DataBlock signature_data;
    int i, mode;
    const char *comment;
    gk_password_handle_t pass_handle;
    GatekeeperTl_Password pass;
    char password[] = "good password";
    uint32_t uid = 1;
    km_hw_auth_token_t auth_token;

    memset(&pass, 0, sizeof(pass));
    memset(&key, 0, sizeof(key));
    memset(&in_data, 0, sizeof(in_data));
    memset(&out_data, 0, sizeof(out_data));
    memset(&signature_data, 0, sizeof(signature_data));

    TEST_ALLOCATE_BLOCK(pass, strlen(password));
    memcpy(pass.buffer, password, strlen(password));

    BDBG_LOG(("----------------------- %s -----------------------", BSTD_FUNCTION));

    /* Enroll a password used for all these tests */
    EXPECT_SUCCESS(GatekeeperTl_Enroll(gkHandle, uid, NULL, NULL, &pass, NULL, &pass_handle));
    BDBG_LOG(("%s: password handle UID: %llx", BSTD_FUNCTION, (unsigned long long)pass_handle.user_id));

    for (mode = 0; mode < 8; mode++) {
        switch (mode) {
        case 0:
            BDBG_LOG(("-------------- %s: Standard tests ---------------", BSTD_FUNCTION));
            break;
        case 1:
            BDBG_LOG(("-------------- %s: Token timeout tests ---------------", BSTD_FUNCTION));
            break;
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
            BDBG_LOG(("-------------- %s: Token corruption tests ---------------", BSTD_FUNCTION));
            break;
        }

        for (i = 0; i < 5; i++) {

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
            default:
                BDBG_ERR(("%s:%d Internal error", BSTD_FUNCTION, __LINE__));
                err = BERR_UNKNOWN;
                goto done;
            }
            BDBG_LOG(("%s : Test - %s", BSTD_FUNCTION, comment));

            TEST_TAG_REMOVE(key_params, SKM_TAG_ALL_USERS);
            TEST_TAG_REMOVE(key_params, SKM_TAG_USER_ID);
            TEST_TAG_REMOVE(key_params, SKM_TAG_NO_AUTH_REQUIRED);

            /* For auth, we need to add auth_type and secure_id with optional auth_timeout */
            TEST_TAG_ADD_ENUM(key_params, SKM_TAG_USER_AUTH_TYPE, SKM_HW_AUTH_PASSWORD);
            TEST_TAG_ADD_LONG_INTEGER(key_params, SKM_TAG_USER_SECURE_ID, pass_handle.user_id);
            /*
             * An odd side-effect of setting auth_timeout is that update and finish operations
             * are not authorized, meaning that there is no need to add auth tokens to the update
             * parameters. So for simplicity of this test, we specify the timeout.
             */
            TEST_TAG_ADD_INTEGER(key_params, SKM_TAG_AUTH_TIMEOUT, AUTH_TIMEOUT_1_SECOND);

            EXPECT_SUCCESS(KeymasterTl_GenerateKey(handle, key_params, &key));

            settings.key_params = key_params;
            settings.in_key = key;

            TEST_ALLOCATE_BLOCK(in_data, TEST_BLOCK_SIZE);
            TEST_ALLOCATE_BLOCK(out_data, MAX_SIGNATURE_SIZE);
            memset(in_data.buffer, 0xC3, in_data.size);
            memset(out_data.buffer, 0xFF, out_data.size);
            settings.in_data = in_data;
            settings.out_data = out_data;

            /* Duplicate begin params before we add the auth token */
            begin_params2 = KM_Tag_DupContext(begin_params);

            /* auth_token required in begin parameters */
            EXPECT_SUCCESS(GatekeeperTl_Verify(gkHandle, uid, 0ull, &pass_handle, &pass, NULL, &auth_token));
            TEST_TAG_ADD_BYTES(settings.begin_params, SKM_TAG_AUTH_TOKEN, sizeof(auth_token), (uint8_t *)&auth_token);

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

            /* Mac length cannot be specified for verify operation */
            km_test_remove_mac_length(settings.begin_params);

            switch (mode) {
            case 0:
                /* STD test of success */
                EXPECT_SUCCESS(KM_Crypto_Operation(SKM_PURPOSE_VERIFY, &settings));
                break;
            case 1:
                /* Test token has timed out */
                BDBG_LOG(("%s: waiting %d seconds before verify", BSTD_FUNCTION, AUTH_TIMEOUT_1_SECOND + 1));
                sleep(AUTH_TIMEOUT_1_SECOND + 1);
                EXPECT_FAILURE_CODE(KM_Crypto_Operation(SKM_PURPOSE_VERIFY, &settings), BSAGE_ERR_KM_KEY_USER_NOT_AUTHENTICATED);
                break;
            case 2:
            case 3:
            case 4:
            case 5:
            case 6:
            case 7:
                /* Test corruption of token */
                switch (mode) {
                case 2:
                    auth_token.challenge++;
                    break;
                case 3:
                    auth_token.user_id++;;
                    break;
                case 4:
                    auth_token.authenticator_id++;;
                    break;
                case 5:
                    auth_token.authenticator_type++;;
                    break;
                case 6:
                    auth_token.timestamp++;;
                    break;
                case 7:
                    auth_token.hmac[0]++;;
                    break;
                default:
                    BDBG_ERR(("%s: invalid mode %d", BSTD_FUNCTION, mode));
                    err = BERR_UNKNOWN;
                    goto done;
                }
                TEST_TAG_ADD_BYTES(begin_params2, SKM_TAG_AUTH_TOKEN, sizeof(auth_token), (uint8_t *)&auth_token);
                settings.begin_params = begin_params2;
                km_test_remove_mac_length(settings.begin_params);
                //EXPECT_FAILURE_CODE(KM_Crypto_Operation(KM_PURPOSE_VERIFY, &settings), BSAGE_ERR_KM_KEY_USER_NOT_AUTHENTICATED);
                break;
            default:
                BDBG_ERR(("%s: invalid mode %d", BSTD_FUNCTION, mode));
                err = BERR_UNKNOWN;
                goto done;
            }

            BDBG_LOG(("%s: %s success", BSTD_FUNCTION, comment));

            TEST_FREE_BLOCK(in_data);
            TEST_FREE_BLOCK(out_data);
            TEST_FREE_BLOCK(signature_data);
            TEST_FREE_BLOCK(key);
            TEST_DELETE_CONTEXT(begin_params);
            TEST_DELETE_CONTEXT(begin_params2);
            TEST_DELETE_CONTEXT(key_params);
        }
    }
    err = BERR_SUCCESS;

done:
    TEST_FREE_BLOCK(in_data);
    TEST_FREE_BLOCK(out_data);
    TEST_FREE_BLOCK(signature_data);
    TEST_FREE_BLOCK(key);
    TEST_DELETE_CONTEXT(begin_params);
    TEST_DELETE_CONTEXT(begin_params2);
    TEST_DELETE_CONTEXT(key_params);
    return err;
}

static BERR_Code km_crypto_hmac_auth_test_2(KeymasterTl_Handle handle, GatekeeperTl_Handle gkHandle)
{
    BERR_Code err;
    KM_Tag_ContextHandle key_params = NULL;
    KM_Tag_ContextHandle begin_params = NULL;
    KM_CryptoOperation_Settings settings;
    KeymasterTl_DataBlock key;
    KeymasterTl_DataBlock in_data;
    KeymasterTl_DataBlock out_data;
    KeymasterTl_DataBlock signature_data;
    int i, mode;
    const char *comment;
    gk_password_handle_t pass_handle;
    GatekeeperTl_Password pass;
    char password[] = "good password";
    uint32_t uid = 1;
    km_hw_auth_token_t auth_token;

    memset(&pass, 0, sizeof(pass));
    memset(&key, 0, sizeof(key));
    memset(&in_data, 0, sizeof(in_data));
    memset(&out_data, 0, sizeof(out_data));
    memset(&signature_data, 0, sizeof(signature_data));

    TEST_ALLOCATE_BLOCK(pass, strlen(password));
    memcpy(pass.buffer, password, strlen(password));

    BDBG_LOG(("----------------------- %s -----------------------", BSTD_FUNCTION));

    /* Enroll a password used for all these tests */
    EXPECT_SUCCESS(GatekeeperTl_Enroll(gkHandle, uid, NULL, NULL, &pass, NULL, &pass_handle));
    BDBG_LOG(("%s: password handle UID: %llx", BSTD_FUNCTION, (unsigned long long)pass_handle.user_id));

    for (mode = 0; mode < 5; mode++) {
        switch (mode) {
        case 0:
            BDBG_LOG(("-------------- %s: Standard tests ---------------", BSTD_FUNCTION));
            break;
        case 1:
            BDBG_LOG(("-------------- %s: Token timeout tests ---------------", BSTD_FUNCTION));
            break;
        case 2:
            BDBG_LOG(("-------------- %s: Wrong challenge ---------------", BSTD_FUNCTION));
            break;
        case 3:
            BDBG_LOG(("-------------- %s: Password auth type not allowed ---------------", BSTD_FUNCTION));
            break;
        case 4:
            BDBG_LOG(("-------------- %s: Password auth type missing ---------------", BSTD_FUNCTION));
            break;
        default:
            break;
        }

        for (i = 0; i < 5; i++) {

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
            default:
                BDBG_ERR(("%s:%d Internal error", BSTD_FUNCTION, __LINE__));
                err = BERR_UNKNOWN;
                goto done;
            }
            BDBG_LOG(("%s : Test - %s", BSTD_FUNCTION, comment));

            TEST_TAG_REMOVE(key_params, SKM_TAG_ALL_USERS);
            TEST_TAG_REMOVE(key_params, SKM_TAG_USER_ID);
            TEST_TAG_REMOVE(key_params, SKM_TAG_NO_AUTH_REQUIRED);

            /* For auth, we need to add auth_type and secure_id without optional auth_timeout */
            switch (mode) {
            case 3:
                /* Don't put in password auth type */
                TEST_TAG_ADD_ENUM(key_params, SKM_TAG_USER_AUTH_TYPE, SKM_HW_AUTH_NONE);
                break;
            case 4:
                /* Don't put in an auth type */
                break;
            default:
                TEST_TAG_ADD_ENUM(key_params, SKM_TAG_USER_AUTH_TYPE, SKM_HW_AUTH_PASSWORD);
                break;
            }
            TEST_TAG_ADD_LONG_INTEGER(key_params, SKM_TAG_USER_SECURE_ID, pass_handle.user_id);

            EXPECT_SUCCESS(KeymasterTl_GenerateKey(handle, key_params, &key));

            settings.key_params = key_params;
            settings.in_key = key;

            TEST_ALLOCATE_BLOCK(in_data, TEST_BLOCK_SIZE);
            TEST_ALLOCATE_BLOCK(out_data, MAX_SIGNATURE_SIZE);
            memset(in_data.buffer, 0xC3, in_data.size);
            memset(out_data.buffer, 0xFF, out_data.size);
            settings.in_data = in_data;
            settings.out_data = out_data;

            /* auth_token required in begin parameters */
            EXPECT_SUCCESS(GatekeeperTl_Verify(gkHandle, uid, 0ull, &pass_handle, &pass, NULL, &auth_token));
            TEST_TAG_ADD_BYTES(settings.begin_params, SKM_TAG_AUTH_TOKEN, sizeof(auth_token), (uint8_t *)&auth_token);

            /* When auth_timeout is not specified in the key, we need to generate a token for crypto update and finish */
            settings.gkHandle = gkHandle;
            settings.uid = uid;
            settings.pass_handle = &pass_handle;
            settings.password = &pass;

            switch (mode) {
            case 3:
            case 4:
                /* Auth type not allowed or auth type missing */
                EXPECT_FAILURE_CODE(KM_Crypto_Operation(SKM_PURPOSE_SIGN, &settings), BSAGE_ERR_KM_KEY_USER_NOT_AUTHENTICATED);
                break;
            default:
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

                /* Mac length cannot be specified for verify operation */
                km_test_remove_mac_length(settings.begin_params);

                switch (mode) {
                case 0:
                case 1:
                    /* First check that op fails if we don't give gkHandle, etc. */
                    settings.gkHandle = NULL;
                    settings.uid = 0;
                    settings.pass_handle = NULL;
                    settings.password = NULL;

                    EXPECT_FAILURE_CODE(KM_Crypto_Operation(SKM_PURPOSE_VERIFY, &settings), BSAGE_ERR_KM_KEY_USER_NOT_AUTHENTICATED);

                    settings.gkHandle = gkHandle;
                    settings.uid = uid;
                    settings.pass_handle = &pass_handle;
                    settings.password = &pass;

                    if (mode) {
                        BDBG_LOG(("%s: waiting %d seconds before verify", BSTD_FUNCTION, AUTH_TIMEOUT_1_SECOND + 1));
                        sleep(AUTH_TIMEOUT_1_SECOND + 1);
                    }
                    EXPECT_SUCCESS(KM_Crypto_Operation(SKM_PURPOSE_VERIFY, &settings));
                    break;
                case 2:
                    /* Purposefully generate wrong challenge for test */
                    settings.testWrongChallenge = true;
                    EXPECT_FAILURE_CODE(KM_Crypto_Operation(SKM_PURPOSE_VERIFY, &settings), BSAGE_ERR_KM_KEY_USER_NOT_AUTHENTICATED);
                    break;
                default:
                    BDBG_ERR(("%s: invalid mode %d", BSTD_FUNCTION, mode));
                    err = BERR_UNKNOWN;
                    goto done;
                }
                break;
            }

            BDBG_LOG(("%s: %s success", BSTD_FUNCTION, comment));

            TEST_FREE_BLOCK(in_data);
            TEST_FREE_BLOCK(out_data);
            TEST_FREE_BLOCK(signature_data);
            TEST_FREE_BLOCK(key);
            TEST_DELETE_CONTEXT(begin_params);
            TEST_DELETE_CONTEXT(key_params);
        }
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

BERR_Code km_crypto_auth_tests(KeymasterTl_Handle handle)
{
    BERR_Code err;
    GatekeeperTl_InitSettings initSettings;
    GatekeeperTl_Handle gkHandle = NULL;

    GatekeeperTl_GetDefaultInitSettings(&initSettings);
    EXPECT_SUCCESS(GatekeeperTl_Init(&gkHandle, &initSettings));

    EXPECT_SUCCESS(km_crypto_hmac_auth_test(handle, gkHandle));
    EXPECT_SUCCESS(km_crypto_hmac_auth_test_2(handle, gkHandle));

done:
    if (handle) {
        GatekeeperTl_Uninit(gkHandle);
    }
    return err;
}
