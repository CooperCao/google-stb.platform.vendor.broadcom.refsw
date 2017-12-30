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

#include <ftw.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <errno.h>

#include "bstd.h"
#include "bkni.h"

#include "openssl/x509.h"
#include "openssl/x509_vfy.h"
#include "openssl/md5.h"

#include "nexus_platform.h"
#include "nexus_platform_init.h"

#ifdef NXCLIENT_SUPPORT
#include "nxclient.h"
#endif

#include "keymaster_ids.h"
#include "keymaster_platform.h"
#include "keymaster_tl.h"
#include "keymaster_test.h"
#include "keymaster_keygen.h"
#include "keymaster_key_params.h"
#include "keymaster_crypto_aes.h"
#include "keymaster_crypto_rsa.h"
#include "keymaster_crypto_hmac.h"


BDBG_MODULE(keymaster_test);

#define USE_OS_VERSION     1

#define USE_OS_PATCHLEVEL  201603U


#ifndef NXCLIENT_SUPPORT
static BERR_Code SAGE_app_join_nexus(void);
static void SAGE_app_leave_nexus(void);

static NEXUS_Error SAGE_app_join_nexus(void)
{
    NEXUS_Error err = 0;
    NEXUS_PlatformSettings platformSettings;

    BDBG_LOG(("\t*** Bringing up all Nexus modules for platform using default settings\n\n\n"));

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;

    err = NEXUS_Platform_Init(&platformSettings);
    if (err != NEXUS_SUCCESS) {
        BDBG_ERR(("\t### Failed to bring up Nexus\n\n"));
    }
    return err;
}

static void SAGE_app_leave_nexus(void)
{
    NEXUS_Platform_Uninit();
}
#endif

/* Each function needs to be called before configure to be sure it fails */
#define TEST0_KEY_SIZE 256
#define TEST0_DATA_SIZE 128

static BERR_Code km_tests_before_configure(KeymasterTl_Handle handle)
{
    BERR_Code err;
    KM_Tag_ContextHandle params = NULL;
    KeymasterTl_GetKeyCharacteristicsSettings chSettings;
    KeymasterTl_ImportKeySettings impSettings;
    KeymasterTl_ExportKeySettings expSettings;
    KeymasterTl_AttestKeySettings attSettings;
    KeymasterTl_CryptoBeginSettings beginSettings;
    KeymasterTl_CryptoUpdateSettings updateSettings;
    KeymasterTl_CryptoFinishSettings finishSettings;
    KeymasterTl_DataBlock key = { 0 };
    KeymasterTl_DataBlock out_key = { 0 };
    KeymasterTl_DataBlock in_data = { 0 };
    KeymasterTl_DataBlock out_data = { 0 };
    uint8_t dummy_key[TEST0_KEY_SIZE / 8];
    km_operation_handle_t op_handle;

    BDBG_LOG(("----------------------- %s -----------------------", BSTD_FUNCTION));

    /* Add RNG entropy */
    TEST_ALLOCATE_BLOCK(in_data, TEST0_DATA_SIZE);
    EXPECT_FAILURE_CODE(KeymasterTl_AddRngEntropy(handle, &in_data), BSAGE_ERR_KM_KEYMASTER_NOT_CONFIGURED);
    TEST_FREE_BLOCK(in_data);

    /* Generate key */
    EXPECT_SUCCESS(km_test_new_params_with_aes_defaults(&params, TEST0_KEY_SIZE));
    EXPECT_FAILURE_CODE(KeymasterTl_GenerateKey(handle, params, &key), BSAGE_ERR_KM_KEYMASTER_NOT_CONFIGURED);
    TEST_DELETE_CONTEXT(params);

    /* Get characteristics */
    KeymasterTl_GetDefaultKeyCharacteristicsSettings(&chSettings);
    chSettings.in_key_blob.buffer = dummy_key;
    chSettings.in_key_blob.size = sizeof(dummy_key);
    EXPECT_SUCCESS(km_test_new_params_with_aes_defaults(&chSettings.in_params, TEST0_KEY_SIZE));
    EXPECT_FAILURE_CODE(KeymasterTl_GetKeyCharacteristics(handle, &chSettings), BSAGE_ERR_KM_KEYMASTER_NOT_CONFIGURED);
    TEST_DELETE_CONTEXT(chSettings.in_params);
    BDBG_ASSERT(!chSettings.out_hw_enforced);
    BDBG_ASSERT(!chSettings.out_sw_enforced);

    /* Import key */
    KeymasterTl_GetDefaultImportKeySettings(&impSettings);
    EXPECT_SUCCESS(km_test_new_params_with_aes_defaults(&impSettings.in_key_params, TEST0_KEY_SIZE));
    impSettings.in_key_format = KM_KEY_FORMAT_RAW;
    impSettings.in_key_blob.buffer = dummy_key;
    impSettings.in_key_blob.size = sizeof(dummy_key);
    EXPECT_FAILURE_CODE(KeymasterTl_ImportKey(handle, &impSettings), BSAGE_ERR_KM_KEYMASTER_NOT_CONFIGURED);
    TEST_DELETE_CONTEXT(impSettings.in_key_params);
    BDBG_ASSERT(!impSettings.out_key_blob.buffer);

    /* Export key */
    KeymasterTl_GetDefaultExportKeySettings(&expSettings);
    expSettings.in_key_format = KM_KEY_FORMAT_X509;
    expSettings.in_key_blob.buffer = dummy_key;
    expSettings.in_key_blob.size = sizeof(dummy_key);
    EXPECT_SUCCESS(km_test_new_params_with_ec_defaults(&expSettings.in_params, TEST0_KEY_SIZE));
    EXPECT_FAILURE_CODE(KeymasterTl_ExportKey(handle, &expSettings), BSAGE_ERR_KM_KEYMASTER_NOT_CONFIGURED);
    TEST_DELETE_CONTEXT(expSettings.in_params);
    BDBG_ASSERT(!expSettings.out_key_blob.buffer);

    /* Attest key */
    KeymasterTl_GetDefaultAttestKeySettings(&attSettings);
    attSettings.in_key_blob.buffer = dummy_key;
    attSettings.in_key_blob.size = sizeof(dummy_key);
    EXPECT_FAILURE_CODE(KeymasterTl_AttestKey(handle, &attSettings), BSAGE_ERR_KM_KEYMASTER_NOT_CONFIGURED);

    /* Upgrade key */
    key.buffer = dummy_key;
    key.size = sizeof(dummy_key);
    out_key.buffer = NULL;
    out_key.size = 0;
    EXPECT_FAILURE_CODE(KeymasterTl_UpgradeKey(handle, &key, NULL, &out_key), BSAGE_ERR_KM_KEYMASTER_NOT_CONFIGURED);
    BDBG_ASSERT(!out_key.buffer);

    /* Delete key */
    key.buffer = dummy_key;
    key.size = sizeof(dummy_key);
    EXPECT_FAILURE_CODE(KeymasterTl_DeleteKey(handle, &key), BSAGE_ERR_KM_KEYMASTER_NOT_CONFIGURED);
    BDBG_ASSERT(!out_key.buffer);

    /* Delete all keys */
    EXPECT_FAILURE_CODE(KeymasterTl_DeleteAllKeys(handle), BSAGE_ERR_KM_KEYMASTER_NOT_CONFIGURED);

    /* Crypto begin */
    KeymasterTl_GetDefaultCryptoBeginSettings(&beginSettings);
    beginSettings.purpose = KM_PURPOSE_DECRYPT;
    beginSettings.in_key_blob.buffer = dummy_key;
    beginSettings.in_key_blob.size = sizeof(dummy_key);
    EXPECT_FAILURE_CODE(KeymasterTl_CryptoBegin(handle, &beginSettings, &op_handle), BSAGE_ERR_KM_KEYMASTER_NOT_CONFIGURED);

    /* Crypto update - in_data and out_data must be SRAI memory */
    KeymasterTl_GetDefaultCryptoUpdateSettings(&updateSettings);
    TEST_ALLOCATE_BLOCK(updateSettings.in_data, TEST0_DATA_SIZE);
    TEST_ALLOCATE_BLOCK(updateSettings.out_data, TEST0_DATA_SIZE);
    EXPECT_FAILURE_CODE(KeymasterTl_CryptoUpdate(handle, op_handle, &updateSettings), BSAGE_ERR_KM_KEYMASTER_NOT_CONFIGURED);
    TEST_FREE_BLOCK(updateSettings.in_data);
    TEST_FREE_BLOCK(updateSettings.out_data);

    /* Crypto finish */
    KeymasterTl_GetDefaultCryptoFinishSettings(&finishSettings);
    TEST_ALLOCATE_BLOCK(finishSettings.out_data, TEST0_DATA_SIZE);
    EXPECT_FAILURE_CODE(KeymasterTl_CryptoFinish(handle, op_handle, &finishSettings), BSAGE_ERR_KM_KEYMASTER_NOT_CONFIGURED);
    TEST_FREE_BLOCK(finishSettings.out_data);

    /* Crypto abort */
    TEST_ALLOCATE_BLOCK(out_data, TEST0_DATA_SIZE);
    EXPECT_FAILURE_CODE(KeymasterTl_CryptoAbort(handle, op_handle), BSAGE_ERR_KM_KEYMASTER_NOT_CONFIGURED);
    TEST_FREE_BLOCK(out_data);

    BDBG_LOG(("%s: all tests passed", BSTD_FUNCTION));
    err = BERR_SUCCESS;

done:
    TEST_DELETE_CONTEXT(params);
    TEST_FREE_BLOCK(in_data);
    TEST_FREE_BLOCK(out_data);
    return err;
}

static BERR_Code km_add_rng_tests(KeymasterTl_Handle handle)
{
    BERR_Code err;
    KeymasterTl_DataBlock in_data = { 0 };

    EXPECT_FAILURE_CODE(KeymasterTl_AddRngEntropy(handle, &in_data), BSAGE_ERR_KM_UNEXPECTED_NULL_POINTER);

    TEST_ALLOCATE_BLOCK(in_data, TEST0_DATA_SIZE);
    EXPECT_SUCCESS(KeymasterTl_AddRngEntropy(handle, &in_data));
    err = BERR_SUCCESS;

done:
    TEST_FREE_BLOCK(in_data);
    return err;
}

#define MAX_MODIFIERS   5
/* Special value used in table below for "don't care" error code */
#define NOT_BERR_SUCCESS (!0u)

typedef struct {
    const char *name;
    km_key_param_fn fn;
    uint32_t key_size;
    km_key_modifier_fn mod_fn[MAX_MODIFIERS];
    BERR_Code expected_err;
} test_param_data;

static BERR_Code km_generate_tests(KeymasterTl_Handle handle)
{
    BERR_Code err;
    KM_Tag_ContextHandle key_params = NULL;
    KeymasterTl_DataBlock key = { 0 };
    int i;
    int mod;
    test_param_data params[] = {
        {"AES 128",     km_test_new_params_with_aes_defaults,     128,  {0},   BERR_SUCCESS},
        {"AES 192",     km_test_new_params_with_aes_defaults,     192,  {0},   BERR_SUCCESS},
        {"AES 256",     km_test_new_params_with_aes_defaults,     256,  {0},   BERR_SUCCESS},
        {"AES 112",     km_test_new_params_with_aes_defaults,     112,  {0},   BSAGE_ERR_KM_UNSUPPORTED_KEY_SIZE},
        {"AES 272",     km_test_new_params_with_aes_defaults,     272,  {0},   BSAGE_ERR_KM_UNSUPPORTED_KEY_SIZE},
        {"AES 256 -key_size",  km_test_new_params_with_aes_defaults, 256,
                {km_test_remove_key_size, 0}, BSAGE_ERR_KM_UNSUPPORTED_KEY_SIZE},

        {"HMAC 160",   km_test_new_params_with_hmac_defaults,     160,  {0},   BERR_SUCCESS},
        {"HMAC 224",   km_test_new_params_with_hmac_defaults,     224,  {0},   BERR_SUCCESS},
        {"HMAC 256",   km_test_new_params_with_hmac_defaults,     256,  {0},   BERR_SUCCESS},
        {"HMAC 512",   km_test_new_params_with_hmac_defaults,     512,  {0},   BERR_SUCCESS},
        {"HMAC 128",   km_test_new_params_with_hmac_defaults,     128,  {0},   BERR_SUCCESS},
        {"HMAC 140",   km_test_new_params_with_hmac_defaults,     140,  {0},   BSAGE_ERR_KM_UNSUPPORTED_KEY_SIZE},
        {"HMAC 528",   km_test_new_params_with_hmac_defaults,     528,  {0},   BSAGE_ERR_KM_UNSUPPORTED_KEY_SIZE},
        {"HMAC 160 -key_size",   km_test_new_params_with_hmac_defaults, 160,
                {km_test_remove_key_size, 0}, BSAGE_ERR_KM_UNSUPPORTED_KEY_SIZE},
        {"HMAC 128 +sha1",   km_test_new_params_with_hmac_defaults, 128,
                {km_test_add_sha1_digest, 0}, BSAGE_ERR_KM_UNSUPPORTED_DIGEST},
        {"HMAC 128 digest none",   km_test_new_params_with_hmac_defaults, 128,
                {km_test_remove_digest, km_test_add_none_digest, 0}, BSAGE_ERR_KM_UNSUPPORTED_DIGEST},
        {"HMAC 128 min_mac_length 48",   km_test_new_params_with_hmac_defaults, 128,
                {km_test_remove_min_mac_length, km_test_add_min_mac_length_48, 0}, BSAGE_ERR_KM_UNSUPPORTED_MIN_MAC_LENGTH},
        {"HMAC 128 min_mac_length 130",   km_test_new_params_with_hmac_defaults, 128,
                {km_test_remove_min_mac_length, km_test_add_min_mac_length_130, 0}, BSAGE_ERR_KM_UNSUPPORTED_MIN_MAC_LENGTH},
        {"HMAC 128 min_mac_length 384",   km_test_new_params_with_hmac_defaults, 128,
                {km_test_remove_min_mac_length, km_test_add_min_mac_length_384, 0}, BSAGE_ERR_KM_UNSUPPORTED_MIN_MAC_LENGTH},

        {"EC 224",     km_test_new_params_with_ec_defaults,       224,  {0},   BERR_SUCCESS},
        {"EC 256",     km_test_new_params_with_ec_defaults,       256,  {0},   BERR_SUCCESS},
        {"EC 384",     km_test_new_params_with_ec_defaults,       384,  {0},   BERR_SUCCESS},
        {"EC 521",     km_test_new_params_with_ec_defaults,       521,  {0},   BERR_SUCCESS},
        {"EC 521",     km_test_new_params_with_ec_defaults,       512,  {0},   BSAGE_ERR_KM_UNSUPPORTED_KEY_SIZE},
        {"EC 208",     km_test_new_params_with_ec_defaults,       208,  {0},   BSAGE_ERR_KM_UNSUPPORTED_KEY_SIZE},
        {"EC 190",     km_test_new_params_with_ec_defaults,       190,  {0},   BSAGE_ERR_KM_UNSUPPORTED_KEY_SIZE},
        {"EC 224 (curve mismatch)",     km_test_new_params_with_ec_defaults, 224,
                {km_test_remove_curve, km_test_add_256_curve, 0}, BERR_INVALID_PARAMETER},
        {"EC 224 -key_size",    km_test_new_params_with_ec_defaults, 224,
                {km_test_remove_key_size, 0}, BERR_SUCCESS},
        {"EC 224 -curve",       km_test_new_params_with_ec_defaults, 224,
                {km_test_remove_curve, 0},    BERR_SUCCESS},
        {"EC 224 -curve -key",  km_test_new_params_with_ec_defaults, 224,
                {km_test_remove_key_size, km_test_remove_curve, 0}, BSAGE_ERR_KM_UNSUPPORTED_KEY_SIZE},

        {"RSA 1024",    km_test_new_params_with_rsa_defaults,     1024, {0},   BERR_SUCCESS},
        {"RSA 2048",    km_test_new_params_with_rsa_defaults,     2048, {0},   BSAGE_ERR_KM_UNSUPPORTED_KEY_SIZE}, /* TEMP disabled */
        {"RSA 3072",    km_test_new_params_with_rsa_defaults,     3072, {0},   BSAGE_ERR_KM_UNSUPPORTED_KEY_SIZE}, /* TEMP disabled */
        {"RSA 4096",    km_test_new_params_with_rsa_defaults,     4096, {0},   BSAGE_ERR_KM_UNSUPPORTED_KEY_SIZE}, /* TEMP disabled */
        {"RSA 256",     km_test_new_params_with_rsa_defaults,     256,  {0},   BERR_SUCCESS},
        {"RSA 512",     km_test_new_params_with_rsa_defaults,     512,  {0},   BERR_SUCCESS},
        {"RSA 768",     km_test_new_params_with_rsa_defaults,     768,  {0},   BERR_SUCCESS},
        {"RSA 764",     km_test_new_params_with_rsa_defaults,     764,  {0},   BSAGE_ERR_KM_UNSUPPORTED_KEY_SIZE},
        {"RSA 5120",    km_test_new_params_with_rsa_defaults,     5120, {0},   BSAGE_ERR_KM_UNSUPPORTED_KEY_SIZE},
        {"RSA 1024 -key_size",  km_test_new_params_with_rsa_defaults, 1024,
                {km_test_remove_key_size, 0}, BSAGE_ERR_KM_UNSUPPORTED_KEY_SIZE},
        {"RSA 1024 -exp",       km_test_new_params_with_rsa_defaults, 1024,
                {km_test_remove_exponent, 0}, BERR_INVALID_PARAMETER},

        {"AES 256 app_id",      km_test_new_params_with_aes_defaults, 256,
                {km_test_remove_all_apps, km_test_add_app_id, 0}, BERR_SUCCESS},
        {"AES 256 app_id+data", km_test_new_params_with_aes_defaults, 256,
                {km_test_remove_all_apps, km_test_add_app_id, km_test_add_app_data, 0}, BERR_SUCCESS},
        {"AES 256 app_data",    km_test_new_params_with_aes_defaults, 256,
                {km_test_remove_all_apps, km_test_add_app_data, 0}, BERR_INVALID_PARAMETER},
        {"AES 256 app_id+all_apps", km_test_new_params_with_aes_defaults, 256,
                {km_test_add_app_id, 0}, BERR_INVALID_PARAMETER},
        {"AES 256 -all_apps",   km_test_new_params_with_aes_defaults, 256,
                {km_test_remove_all_apps, 0}, BERR_INVALID_PARAMETER},

        {"HMAC app_id",         km_test_new_params_with_hmac_defaults, 256,
                {km_test_remove_all_apps, km_test_add_app_id, 0}, BERR_SUCCESS},
        {"HMAC app_id+data",    km_test_new_params_with_hmac_defaults, 256,
                {km_test_remove_all_apps, km_test_add_app_id, km_test_add_app_data, 0}, BERR_SUCCESS},
        {"HMAC app_data",       km_test_new_params_with_hmac_defaults, 256,
                {km_test_remove_all_apps, km_test_add_app_data, 0}, BERR_INVALID_PARAMETER},
        {"HMAC app_id+all_apps",    km_test_new_params_with_hmac_defaults, 256,
                {km_test_add_app_id, 0}, BERR_INVALID_PARAMETER},
        {"HMAC -all_apps",      km_test_new_params_with_hmac_defaults, 256,
                {km_test_remove_all_apps, 0}, BERR_INVALID_PARAMETER},

        {"EC app_id",           km_test_new_params_with_ec_defaults, 256,
                {km_test_remove_all_apps, km_test_add_app_id, 0}, BERR_SUCCESS},
        {"EC app_id+data",      km_test_new_params_with_ec_defaults, 256,
                {km_test_remove_all_apps, km_test_add_app_id, km_test_add_app_data, 0}, BERR_SUCCESS},
        {"EC app_data",         km_test_new_params_with_ec_defaults, 256,
                {km_test_remove_all_apps, km_test_add_app_data, 0}, BERR_INVALID_PARAMETER},
        {"EC app_id_all_apps",  km_test_new_params_with_ec_defaults, 256,
                {km_test_add_app_id, 0}, BERR_INVALID_PARAMETER},
        {"EC -all_apps",        km_test_new_params_with_ec_defaults, 256,
                {km_test_remove_all_apps, 0}, BERR_INVALID_PARAMETER},

        {"RSA app_id",          km_test_new_params_with_rsa_defaults, 1024,
                {km_test_remove_all_apps, km_test_add_app_id, 0}, BERR_SUCCESS},
        {"RSA app_id+data",     km_test_new_params_with_rsa_defaults, 1024,
                {km_test_remove_all_apps, km_test_add_app_id, km_test_add_app_data, 0}, BERR_SUCCESS},
        {"RSA app_data",        km_test_new_params_with_rsa_defaults, 1024,
                {km_test_remove_all_apps, km_test_add_app_data, 0}, BERR_INVALID_PARAMETER},
        {"RSA app_id+all_apps", km_test_new_params_with_rsa_defaults, 1024,
                {km_test_add_app_id, 0}, BERR_INVALID_PARAMETER},
        {"RSA -all_apps",       km_test_new_params_with_rsa_defaults, 1024,
                {km_test_remove_all_apps, 0}, BERR_INVALID_PARAMETER},
    };

    BDBG_LOG(("----------------------- %s -----------------------", BSTD_FUNCTION));

    EXPECT_SUCCESS(km_test_new_params_with_aes_defaults(&key_params, 128));
    EXPECT_FAILURE_CODE(KeymasterTl_GenerateKey(handle, NULL, &key), BSAGE_ERR_KM_UNEXPECTED_NULL_POINTER);
    EXPECT_FAILURE_CODE(KeymasterTl_GenerateKey(handle, key_params, NULL), BSAGE_ERR_KM_UNEXPECTED_NULL_POINTER);
    TEST_DELETE_CONTEXT(key_params);

    for (i = 0; i < sizeof(params) / sizeof(test_param_data); i++) {
        bool test_failed = true;

        EXPECT_SUCCESS(params[i].fn(&key_params, params[i].key_size));
        if (params[i].name[0] == 'H')
        {
            TEST_TAG_ADD_ENUM(key_params, KM_TAG_DIGEST, KM_DIGEST_SHA_2_256);
        }

        for (mod = 0; mod < MAX_MODIFIERS && params[i].mod_fn[mod]; mod++) {
            EXPECT_SUCCESS(params[i].mod_fn[mod](key_params));
        }
        err = KeymasterTl_GenerateKey(handle, key_params, &key);
        if (params[i].expected_err == BERR_SUCCESS) {
            /* Should have passed */
            test_failed = (err != BERR_SUCCESS);
        } else {
            /* Should have failed */
            if (params[i].expected_err == NOT_BERR_SUCCESS) {
                test_failed = (err == BERR_SUCCESS);
            } else {
                test_failed = (err != params[i].expected_err);
            }
        }
        if (test_failed) {
            BDBG_ERR(("%s:%d iteration %d/%s failed", BSTD_FUNCTION, __LINE__, i, params[i].name));
            BDBG_ERR(("%s: err 0x%x, expected 0x%x", __FUNCTION__, err, params[i].expected_err));
            err = BERR_UNKNOWN;
            goto done;
        }
        BDBG_LOG(("%s:%d success", params[i].name, i));
        BDBG_LOG(("\tReturned blob size %d (%p)", key.size, key.buffer));

        TEST_FREE_BLOCK(key);
        TEST_DELETE_CONTEXT(key_params);
    }
    err = BERR_SUCCESS;

done:
    TEST_FREE_BLOCK(key);
    TEST_DELETE_CONTEXT(key_params);
    return err;
}

static BERR_Code km_get_characteristics_tests(KeymasterTl_Handle handle)
{
    BERR_Code err;
    KM_Tag_ContextHandle key_params = NULL;
    KM_Tag_ContextHandle in_params = NULL;
    KeymasterTl_GetKeyCharacteristicsSettings chSettings;
    KeymasterTl_DataBlock key = { 0 };
    int i;
    int mod;
    test_param_data params[] = {
        {"AES 128",     km_test_new_params_with_aes_defaults,     128,  {0},   BERR_SUCCESS},
        {"AES 192",     km_test_new_params_with_aes_defaults,     192,  {0},   BERR_SUCCESS},
        {"AES 256",     km_test_new_params_with_aes_defaults,     256,  {0},   BERR_SUCCESS},

        {"HMAC 160",   km_test_new_params_with_hmac_defaults,     160,  {0},   BERR_SUCCESS},
        {"HMAC 224",   km_test_new_params_with_hmac_defaults,     224,  {0},   BERR_SUCCESS},
        {"HMAC 256",   km_test_new_params_with_hmac_defaults,     256,  {0},   BERR_SUCCESS},
        {"HMAC 512",   km_test_new_params_with_hmac_defaults,     512,  {0},   BERR_SUCCESS},

        {"EC 224",     km_test_new_params_with_ec_defaults,       224,  {0},   BERR_SUCCESS},
        {"EC 256",     km_test_new_params_with_ec_defaults,       256,  {0},   BERR_SUCCESS},
        {"EC 384",     km_test_new_params_with_ec_defaults,       384,  {0},   BERR_SUCCESS},
        {"EC 521",     km_test_new_params_with_ec_defaults,       521,  {0},   BERR_SUCCESS},
        {"EC 224 -key_size",    km_test_new_params_with_ec_defaults, 224,
                {km_test_remove_key_size, 0}, BERR_SUCCESS},
        {"EC 224 -curve",       km_test_new_params_with_ec_defaults, 224,
                {km_test_remove_curve, 0},    BERR_SUCCESS},

        {"RSA 1024",    km_test_new_params_with_rsa_defaults,     1024, {0},   BERR_SUCCESS},

        {"AES 256 app_id",      km_test_new_params_with_aes_defaults, 256,
                {km_test_remove_all_apps, km_test_add_app_id, 0}, BERR_SUCCESS},
        {"AES 256 app_id+data", km_test_new_params_with_aes_defaults, 256,
                {km_test_remove_all_apps, km_test_add_app_id, km_test_add_app_data, 0}, BERR_SUCCESS},

        {"HMAC app_id",         km_test_new_params_with_hmac_defaults, 256,
                {km_test_remove_all_apps, km_test_add_app_id, 0}, BERR_SUCCESS},
        {"HMAC app_id+data",    km_test_new_params_with_hmac_defaults, 256,
                {km_test_remove_all_apps, km_test_add_app_id, km_test_add_app_data, 0}, BERR_SUCCESS},

        {"EC app_id",           km_test_new_params_with_ec_defaults, 256,
                {km_test_remove_all_apps, km_test_add_app_id, 0}, BERR_SUCCESS},
        {"EC app_id+data",      km_test_new_params_with_ec_defaults, 256,
                {km_test_remove_all_apps, km_test_add_app_id, km_test_add_app_data, 0}, BERR_SUCCESS},

        {"RSA app_id",          km_test_new_params_with_rsa_defaults, 1024,
                {km_test_remove_all_apps, km_test_add_app_id, 0}, BERR_SUCCESS},
        {"RSA app_id+data",     km_test_new_params_with_rsa_defaults, 1024,
                {km_test_remove_all_apps, km_test_add_app_id, km_test_add_app_data, 0}, BERR_SUCCESS},
    };

    BDBG_LOG(("----------------------- %s -----------------------", BSTD_FUNCTION));

    /* Check params: the only thing that can fail is not passing in a key blob */
    KeymasterTl_GetDefaultKeyCharacteristicsSettings(&chSettings);
    EXPECT_FAILURE_CODE(KeymasterTl_GetKeyCharacteristics(handle, &chSettings), BSAGE_ERR_KM_UNEXPECTED_NULL_POINTER);

    for (i = 0; i < sizeof(params) / sizeof(test_param_data); i++) {
        EXPECT_SUCCESS(params[i].fn(&key_params, params[i].key_size));
        if (params[i].name[0] == 'H')
        {
            TEST_TAG_ADD_ENUM(key_params, KM_TAG_DIGEST, KM_DIGEST_SHA_2_256);
        }
        for (mod = 0; mod < MAX_MODIFIERS && params[i].mod_fn[mod]; mod++) {
            EXPECT_SUCCESS(params[i].mod_fn[mod](key_params));
        }
        EXPECT_SUCCESS(KeymasterTl_GenerateKey(handle, key_params, &key));
        EXPECT_SUCCESS(km_test_copy_app_id_and_data(key_params, &in_params));
        KeymasterTl_GetDefaultKeyCharacteristicsSettings(&chSettings);
        chSettings.in_key_blob = key;
        chSettings.in_params = in_params;
        EXPECT_SUCCESS(KeymasterTl_GetKeyCharacteristics(handle, &chSettings));
        BDBG_LOG(("\tReturned HW enforced count %d, SW enforced count %d", KM_Tag_GetNumPairs(chSettings.out_hw_enforced), KM_Tag_GetNumPairs(chSettings.out_sw_enforced)));
        EXPECT_TRUE(KM_Tag_ContainsEnum(chSettings.out_hw_enforced, KM_TAG_ORIGIN, KM_ORIGIN_GENERATED));
        TEST_DELETE_CONTEXT(chSettings.out_hw_enforced);
        TEST_DELETE_CONTEXT(chSettings.out_sw_enforced);

        if (KM_Tag_GetNumPairs(in_params)) {
            /* If we have entries in in_params, test for failure to decrypt blob if we omit it */
            KeymasterTl_GetDefaultKeyCharacteristicsSettings(&chSettings);
            chSettings.in_key_blob = key;
            EXPECT_FAILURE_CODE(KeymasterTl_GetKeyCharacteristics(handle, &chSettings), BSAGE_ERR_KM_INVALID_KEY_BLOB);
        }

        BDBG_LOG(("%s:%d success", params[i].name, i));

        TEST_FREE_BLOCK(key);
        TEST_DELETE_CONTEXT(key_params);
        TEST_DELETE_CONTEXT(in_params);
    }
    err = BERR_SUCCESS;

done:
    TEST_FREE_BLOCK(key);
    TEST_DELETE_CONTEXT(key_params);
    TEST_DELETE_CONTEXT(in_params);
    return err;
}

static BERR_Code km_key_blob_tests(KeymasterTl_Handle handle)
{
    BERR_Code err;
    KeymasterTl_GetKeyCharacteristicsSettings chSettings;
    KM_Tag_ContextHandle key_params = NULL;
    KM_Tag_ContextHandle hw = NULL;
    KM_Tag_ContextHandle sw = NULL;
    KeymasterTl_DataBlock key = { 0 };
    KeymasterTl_DataBlock key_copy = { 0 };
    uint8_t *key_buffer = NULL;
    km_key_blob_t *blob;
    int i;
    const char *comment;

    BDBG_LOG(("----------------------- %s -----------------------", BSTD_FUNCTION));

    EXPECT_SUCCESS(km_test_new_params_with_aes_defaults(&key_params, 128));
    EXPECT_SUCCESS(KeymasterTl_GenerateKey(handle, key_params, &key));
    KeymasterTl_GetDefaultKeyCharacteristicsSettings(&chSettings);
    chSettings.in_key_blob = key;
    EXPECT_SUCCESS(KeymasterTl_GetKeyCharacteristics(handle, &chSettings));
    TEST_DELETE_CONTEXT(chSettings.out_hw_enforced);
    TEST_DELETE_CONTEXT(chSettings.out_sw_enforced);

    BDBG_LOG(("\tReturned blob size %d (%p)", key.size, key.buffer));

    if (NEXUS_Memory_Allocate(key.size, NULL, (void**)&key_buffer) != NEXUS_SUCCESS) {
        BDBG_ERR(("%s: failed to allocate key", BSTD_FUNCTION));
        goto done;
    }

    for (i = 0; i < 8; i++) {
        key_copy.size = key.size;
        key_copy.buffer = key_buffer;
        blob = (km_key_blob_t *)key_copy.buffer;

        memcpy(key_copy.buffer, key.buffer, key.size);

        BDBG_LOG(("%s: running %d", BSTD_FUNCTION, i));
        /* Poke with various parts of the key blob */
        switch (i) {
        case 0:
            comment = "Magic changed";
            blob->magic++;
            break;
        case 1:
            comment = "header version changed";
            blob->header_version++;
            break;
        case 2:
            comment = "Keymaster version changed";
            blob->keymaster_version++;
            break;
        case 3:
            comment = "Blob length changed";
            blob->blob_length++;
            break;
        case 4:
            comment = "Blob length (2) changed";
            blob->blob_length--;
            break;
        case 5:
            comment = "IV changed";
            blob->iv[0]++;
            break;
        case 6:
            /* Change first byte of data */
            comment = "Data changed";
            blob->blob[0]++;
            break;
        case 7:
            /* Change the HASH */
            comment = "HASH changed";
            blob->blob[blob->blob_length]++;
            break;
        default:
            BDBG_ERR(("%s: unknown test", BSTD_FUNCTION));
            err = BERR_UNKNOWN;
            goto done;
        }

        KeymasterTl_GetDefaultKeyCharacteristicsSettings(&chSettings);
        chSettings.in_key_blob = key_copy;
        EXPECT_FAILURE(KeymasterTl_GetKeyCharacteristics(handle, &chSettings));
        BDBG_LOG(("%s:%d success", comment, i));
    }

    err = BERR_SUCCESS;

done:
    TEST_FREE_BLOCK(key);
    if (key_buffer) {
        NEXUS_Memory_Free(key_buffer);
    }
    TEST_DELETE_CONTEXT(key_params);
    TEST_DELETE_CONTEXT(hw);
    TEST_DELETE_CONTEXT(sw);
    return err;
}

static BERR_Code km_export_tests(KeymasterTl_Handle handle)
{
    BERR_Code err;
    KeymasterTl_ExportKeySettings expSettings;
    KM_Tag_ContextHandle key_params = NULL;
    KM_Tag_ContextHandle in_params = NULL;
    KeymasterTl_DataBlock in_key = { 0 };
    int i;
    int mod;
    test_param_data params[] = {
        {"AES 128",     km_test_new_params_with_aes_defaults,     128,  {0},   BSAGE_ERR_KM_UNSUPPORTED_KEY_FORMAT},
        {"AES 192",     km_test_new_params_with_aes_defaults,     192,  {0},   BSAGE_ERR_KM_UNSUPPORTED_KEY_FORMAT},
        {"AES 256",     km_test_new_params_with_aes_defaults,     256,  {0},   BSAGE_ERR_KM_UNSUPPORTED_KEY_FORMAT},

        {"HMAC 160",   km_test_new_params_with_hmac_defaults,     160,  {0},   BSAGE_ERR_KM_UNSUPPORTED_KEY_FORMAT},
        {"HMAC 224",   km_test_new_params_with_hmac_defaults,     224,  {0},   BSAGE_ERR_KM_UNSUPPORTED_KEY_FORMAT},
        {"HMAC 256",   km_test_new_params_with_hmac_defaults,     256,  {0},   BSAGE_ERR_KM_UNSUPPORTED_KEY_FORMAT},
        {"HMAC 512",   km_test_new_params_with_hmac_defaults,     512,  {0},   BSAGE_ERR_KM_UNSUPPORTED_KEY_FORMAT},

        {"EC 224",     km_test_new_params_with_ec_defaults,       224,  {0},   BERR_SUCCESS},
        {"EC 256",     km_test_new_params_with_ec_defaults,       256,  {0},   BERR_SUCCESS},
        {"EC 384",     km_test_new_params_with_ec_defaults,       384,  {0},   BERR_SUCCESS},
        {"EC 521",     km_test_new_params_with_ec_defaults,       521,  {0},   BERR_SUCCESS},
        {"EC 224 -key_size",    km_test_new_params_with_ec_defaults, 224,
                {km_test_remove_key_size, 0}, BERR_SUCCESS},
        {"EC 224 -curve",       km_test_new_params_with_ec_defaults, 224,
                {km_test_remove_curve, 0},    BERR_SUCCESS},

        {"RSA 256",     km_test_new_params_with_rsa_defaults,     256,  {0},   BERR_SUCCESS},
        {"RSA 1024",    km_test_new_params_with_rsa_defaults,     1024, {0},   BERR_SUCCESS},

        {"EC app_id",           km_test_new_params_with_ec_defaults, 256,
                {km_test_remove_all_apps, km_test_add_app_id, 0}, BERR_SUCCESS},
        {"EC app_id+data",      km_test_new_params_with_ec_defaults, 256,
                {km_test_remove_all_apps, km_test_add_app_id, km_test_add_app_data, 0}, BERR_SUCCESS},

        {"RSA app_id",          km_test_new_params_with_rsa_defaults, 1024,
                {km_test_remove_all_apps, km_test_add_app_id, 0}, BERR_SUCCESS},
        {"RSA app_id+data",     km_test_new_params_with_rsa_defaults, 1024,
                {km_test_remove_all_apps, km_test_add_app_id, km_test_add_app_data, 0}, BERR_SUCCESS},
    };

    BDBG_LOG(("----------------------- %s -----------------------", BSTD_FUNCTION));

    for (i = 0; i < 4; i++) {
        switch (i) {
        case 0:
            EXPECT_SUCCESS(km_test_new_params_with_ec_defaults(&key_params, 256));
            break;
        case 1:
            EXPECT_SUCCESS(km_test_new_params_with_rsa_defaults(&key_params, 256));
            break;
        case 2:
            EXPECT_SUCCESS(km_test_new_params_with_aes_defaults(&key_params, 128));
            break;
        case 3:
            EXPECT_SUCCESS(km_test_new_params_with_hmac_defaults(&key_params, 256));
            TEST_TAG_ADD_ENUM(key_params, KM_TAG_DIGEST, KM_DIGEST_SHA_2_256);
            break;
        default:
            BDBG_ASSERT(0);
        }

        EXPECT_SUCCESS(KeymasterTl_GenerateKey(handle, key_params, &in_key));
        KeymasterTl_GetDefaultExportKeySettings(&expSettings);
        expSettings.in_key_format = KM_KEY_FORMAT_RAW;
        expSettings.in_key_blob = in_key;
        EXPECT_FAILURE_CODE(KeymasterTl_ExportKey(handle, &expSettings), BSAGE_ERR_KM_UNSUPPORTED_KEY_FORMAT);
        KeymasterTl_GetDefaultExportKeySettings(&expSettings);
        expSettings.in_key_format = KM_KEY_FORMAT_PKCS8;
        expSettings.in_key_blob = in_key;
        EXPECT_FAILURE_CODE(KeymasterTl_ExportKey(handle, &expSettings), BSAGE_ERR_KM_UNSUPPORTED_KEY_FORMAT);
        KeymasterTl_GetDefaultExportKeySettings(&expSettings);
        expSettings.in_key_format = KM_KEY_FORMAT_X509;
        EXPECT_FAILURE_CODE(KeymasterTl_ExportKey(handle, &expSettings), BSAGE_ERR_KM_UNEXPECTED_NULL_POINTER);
        TEST_FREE_BLOCK(in_key);
        TEST_DELETE_CONTEXT(key_params);
    }

    for (i = 0; i < sizeof(params) / sizeof(test_param_data); i++) {
        bool test_failed = true;

        EXPECT_SUCCESS(params[i].fn(&key_params, params[i].key_size));
        for (mod = 0; mod < MAX_MODIFIERS && params[i].mod_fn[mod]; mod++) {
            EXPECT_SUCCESS(params[i].mod_fn[mod](key_params));
        }
        if (params[i].name[0] == 'H')
        {
            TEST_TAG_ADD_ENUM(key_params, KM_TAG_DIGEST, KM_DIGEST_SHA_2_256);
        }
        EXPECT_SUCCESS(KeymasterTl_GenerateKey(handle, key_params, &in_key));
        EXPECT_SUCCESS(km_test_copy_app_id_and_data(key_params, &in_params));

        KeymasterTl_GetDefaultExportKeySettings(&expSettings);
        expSettings.in_key_format = KM_KEY_FORMAT_X509;
        expSettings.in_key_blob = in_key;
        expSettings.in_params = in_params;
        err = KeymasterTl_ExportKey(handle, &expSettings);
        if (params[i].expected_err == BERR_SUCCESS) {
            /* Should have passed */
            test_failed = (err != BERR_SUCCESS);
        } else {
            /* Should have failed */
            if (params[i].expected_err == NOT_BERR_SUCCESS) {
                test_failed = (err == BERR_SUCCESS);
            } else {
                test_failed = (err != params[i].expected_err);
            }
        }
        if (test_failed) {
            BDBG_ERR(("%s:%d iteration %d/%s failed", BSTD_FUNCTION, __LINE__, i, params[i].name));
            BDBG_ERR(("%s: err 0x%x, expected 0x%x", __FUNCTION__, err, params[i].expected_err));
            TEST_FREE_BLOCK(expSettings.out_key_blob);
            err = BERR_UNKNOWN;
            goto done;
        }

        BDBG_LOG(("%s:%d success", params[i].name, i));
        BDBG_LOG(("\tReturned blob size %d (%p)", expSettings.out_key_blob.size, expSettings.out_key_blob.buffer));

        TEST_FREE_BLOCK(expSettings.out_key_blob);
        TEST_FREE_BLOCK(in_key);
        TEST_DELETE_CONTEXT(key_params);
        TEST_DELETE_CONTEXT(in_params);
    }
    err = BERR_SUCCESS;

done:
    TEST_FREE_BLOCK(in_key);
    TEST_DELETE_CONTEXT(key_params);
    TEST_DELETE_CONTEXT(in_params);
    return err;
}

#if 0
static BERR_Code km_load_data_file(const char *filename, KeymasterTl_DataBlock *data_block)
{
    BERR_Code err = BERR_SUCCESS;
    FILE *fd = NULL;
    int pos;
    size_t actual_read;

    BDBG_ASSERT(filename);
    BDBG_ASSERT(data_block);

    data_block->buffer = NULL;
    data_block->size = 0;

    fd = fopen(filename, "rb");
    if (!fd) {
        err = BERR_INVALID_PARAMETER;
        BDBG_ERR(("%s: failed to open key file %s", __FUNCTION__, filename));
        goto done;
    }
    pos = fseek(fd, 0, SEEK_END);
    if (pos == -1) {
        BDBG_ERR(("%s: Error seeking to end of file '%s'.  (%s)", __FUNCTION__, filename, strerror(errno)));
        err = BERR_OS_ERROR;
        goto done;
    }
    pos = ftell(fd);
    if (pos == -1) {
        BDBG_ERR(("%s: Error determining position of file pointer of file '%s'.  (%s)", __FUNCTION__, filename, strerror(errno)));
        err = BERR_OS_ERROR;
        goto done;
    }
    TEST_ALLOCATE_BLOCK(*data_block, pos);
    data_block->size = pos;

    pos = fseek(fd, 0, SEEK_SET);
    if (pos == -1) {
        BDBG_ERR(("%s: Error seeking to beginning of file '%s'.  (%s)", __FUNCTION__, filename, strerror(errno)));
        err = BERR_OS_ERROR;
        goto done;
    }
    actual_read = fread(data_block->buffer, 1, data_block->size, fd);
    if (actual_read != data_block->size) {
        BDBG_ERR(("%s: Read error on '%s'.  Expected %d, read %d", __FUNCTION__, filename, data_block->size,(int)actual_read));
        err = BERR_OS_ERROR;
        goto done;
    }

done:
    if (err != BERR_SUCCESS) {
        TEST_FREE_BLOCK(*data_block);
    }
    if (fd) {
        fclose(fd);
    }
    return err;
}
#endif

static BERR_Code km_create_key_blob(km_algorithm_t algorithm, uint32_t key_size, KeymasterTl_DataBlock *data_block)
{
    BDBG_ASSERT(data_block);

    BERR_Code err = BERR_SUCCESS;
    int i;

    data_block->buffer = NULL;
    data_block->size = 0;

    switch (algorithm) {
    case KM_ALGORITHM_RSA:
        rsa_gen_keys(data_block, key_size);
        break;
    case KM_ALGORITHM_EC:
        ec_gen_keys(data_block, key_size);
        break;
    case KM_ALGORITHM_AES:
    case KM_ALGORITHM_HMAC:
        TEST_ALLOCATE_BLOCK(*data_block, key_size / 8);
        data_block->size = key_size / 8;

        for (i = 0; i < data_block->size; i++) {
            data_block->buffer[i] = rand();
        }
        break;
    default:
        BDBG_ERR(("%s: invalid algorithm", __FUNCTION__));
        err = BERR_INVALID_PARAMETER;
        break;
    }

done:
    if (err != BERR_SUCCESS) {
        TEST_FREE_BLOCK(*data_block);
    }
    return err;
}

typedef struct {
    const char *name;
    km_key_param_fn fn;
    uint32_t key_size;
    km_key_format_t format;
    km_key_modifier_fn mod_fn[MAX_MODIFIERS];
    BERR_Code expected_err;
} import_test_param_data;

static BERR_Code km_import_tests(KeymasterTl_Handle handle)
{
    BERR_Code err;
    KeymasterTl_ImportKeySettings impSettings = { 0 };
    KeymasterTl_GetKeyCharacteristicsSettings chSettings = { 0 };
    KM_Tag_ContextHandle key_params = NULL;
    KM_Tag_ContextHandle in_params = NULL;
    KeymasterTl_DataBlock in_key = { 0 };
    km_algorithm_t algorithm;
    int i;
    int mod;
    import_test_param_data params[] = {
        {"AES 128",   km_test_new_params_with_aes_defaults,  128,  KM_KEY_FORMAT_RAW, {0},   BERR_SUCCESS},
        {"AES 192",   km_test_new_params_with_aes_defaults,  192,  KM_KEY_FORMAT_RAW, {0},   BERR_SUCCESS},
        {"AES 256",   km_test_new_params_with_aes_defaults,  256,  KM_KEY_FORMAT_RAW, {0},   BERR_SUCCESS},
        {"AES 112",   km_test_new_params_with_aes_defaults,  112,  KM_KEY_FORMAT_RAW, {0},   BSAGE_ERR_KM_UNSUPPORTED_KEY_SIZE},
        {"AES 272",   km_test_new_params_with_aes_defaults,  272,  KM_KEY_FORMAT_RAW, {0},   BSAGE_ERR_KM_UNSUPPORTED_KEY_SIZE},
        {"AES wrong format 1",   km_test_new_params_with_aes_defaults,  256,  KM_KEY_FORMAT_PKCS8, {0}, BSAGE_ERR_KM_UNSUPPORTED_KEY_FORMAT},
        {"AES wrong format 2",   km_test_new_params_with_aes_defaults,  256,  KM_KEY_FORMAT_X509, {0},  BSAGE_ERR_KM_UNSUPPORTED_KEY_FORMAT},
        {"HMAC 160",  km_test_new_params_with_hmac_defaults, 160,  KM_KEY_FORMAT_RAW, {0},   BERR_SUCCESS},
        {"HMAC 224",  km_test_new_params_with_hmac_defaults, 224,  KM_KEY_FORMAT_RAW, {0},   BERR_SUCCESS},
        {"HMAC 256",  km_test_new_params_with_hmac_defaults, 256,  KM_KEY_FORMAT_RAW, {0},   BERR_SUCCESS},
        {"HMAC 512",  km_test_new_params_with_hmac_defaults, 512,  KM_KEY_FORMAT_RAW, {0},   BERR_SUCCESS},
        {"HMAC 521",  km_test_new_params_with_hmac_defaults, 521,  KM_KEY_FORMAT_RAW, {0},   BSAGE_ERR_KM_UNSUPPORTED_KEY_SIZE},
        {"HMAC wrong format 1",  km_test_new_params_with_hmac_defaults, 160,  KM_KEY_FORMAT_PKCS8, {0}, BSAGE_ERR_KM_UNSUPPORTED_KEY_FORMAT},
        {"HMAC wrong format 2",  km_test_new_params_with_hmac_defaults, 160,  KM_KEY_FORMAT_X509, {0},  BSAGE_ERR_KM_UNSUPPORTED_KEY_FORMAT},
        {"EC 224",    km_test_new_params_with_ec_defaults,   224,  KM_KEY_FORMAT_PKCS8, {0}, BERR_SUCCESS},
        {"EC 256",    km_test_new_params_with_ec_defaults,   256,  KM_KEY_FORMAT_PKCS8, {0}, BERR_SUCCESS},
        {"EC 384",    km_test_new_params_with_ec_defaults,   384,  KM_KEY_FORMAT_PKCS8, {0}, BERR_SUCCESS},
        {"EC 521",    km_test_new_params_with_ec_defaults,   521,  KM_KEY_FORMAT_PKCS8, {0}, BERR_SUCCESS},
        {"EC 224",    km_test_new_params_with_ec_defaults,   224,  KM_KEY_FORMAT_PKCS8, {0}, BERR_SUCCESS},
        {"EC wrong format 1",    km_test_new_params_with_ec_defaults,   224,  KM_KEY_FORMAT_RAW, {0},  BSAGE_ERR_KM_UNSUPPORTED_KEY_FORMAT},
        {"EC wrong format 2",    km_test_new_params_with_ec_defaults,   224,  KM_KEY_FORMAT_X509, {0}, BSAGE_ERR_KM_UNSUPPORTED_KEY_FORMAT},
        {"EC 224 -key_size",    km_test_new_params_with_ec_defaults, 224, KM_KEY_FORMAT_PKCS8,
                {km_test_remove_key_size, 0}, BERR_SUCCESS},
        {"EC 224 -curve",       km_test_new_params_with_ec_defaults, 224, KM_KEY_FORMAT_PKCS8,
                {km_test_remove_curve, 0},    BERR_SUCCESS},
        {"EC keysize mismatch", km_test_new_params_with_ec_defaults, 224, KM_KEY_FORMAT_PKCS8,
                {km_test_remove_key_size, km_test_add_key_size_256, 0}, BSAGE_ERR_KM_IMPORT_PARAMETER_MISMATCH},
        {"EC app_id+data",      km_test_new_params_with_ec_defaults, 256, KM_KEY_FORMAT_PKCS8,
                {km_test_remove_all_apps, km_test_add_app_id, km_test_add_app_data, 0}, BERR_SUCCESS},
        {"EC app_id",           km_test_new_params_with_ec_defaults, 256, KM_KEY_FORMAT_PKCS8,
                {km_test_remove_all_apps, km_test_add_app_id, 0}, BERR_SUCCESS},
        {"EC app_data",         km_test_new_params_with_ec_defaults, 256, KM_KEY_FORMAT_PKCS8,
                {km_test_remove_all_apps, km_test_add_app_data, 0}, BERR_INVALID_PARAMETER},
        {"RSA 1024", km_test_new_params_with_rsa_defaults,  1024, KM_KEY_FORMAT_PKCS8, {0},   BERR_SUCCESS},
        {"RSA 768",  km_test_new_params_with_rsa_defaults,  768,  KM_KEY_FORMAT_PKCS8, {0},   BERR_SUCCESS},
        {"RSA 3072", km_test_new_params_with_rsa_defaults,  3072, KM_KEY_FORMAT_PKCS8, {0},   BERR_SUCCESS},
        {"RSA 4096", km_test_new_params_with_rsa_defaults,  4096, KM_KEY_FORMAT_PKCS8, {0},   BERR_SUCCESS},
        {"RSA wrong format 1",  km_test_new_params_with_rsa_defaults,  768,  KM_KEY_FORMAT_RAW, {0},   BSAGE_ERR_KM_UNSUPPORTED_KEY_FORMAT},
        {"RSA wrong format 2",  km_test_new_params_with_rsa_defaults,  768,  KM_KEY_FORMAT_X509, {0},  BSAGE_ERR_KM_UNSUPPORTED_KEY_FORMAT},
        {"RSA keysize mismatch", km_test_new_params_with_rsa_defaults, 768, KM_KEY_FORMAT_PKCS8,
                {km_test_remove_key_size, km_test_add_key_size_1024, 0}, BSAGE_ERR_KM_IMPORT_PARAMETER_MISMATCH},
        {"RSA exponent mismatch", km_test_new_params_with_rsa_defaults, 768, KM_KEY_FORMAT_PKCS8,
                {km_test_remove_exponent, km_test_add_exponent_3, 0}, BSAGE_ERR_KM_IMPORT_PARAMETER_MISMATCH},
        {"RSA app_id",          km_test_new_params_with_rsa_defaults, 1024, KM_KEY_FORMAT_PKCS8,
                {km_test_remove_all_apps, km_test_add_app_id, 0}, BERR_SUCCESS},
        {"RSA app_id+data",     km_test_new_params_with_rsa_defaults, 1024, KM_KEY_FORMAT_PKCS8,
                {km_test_remove_all_apps, km_test_add_app_id, km_test_add_app_data, 0}, BERR_SUCCESS},
        {"RSA app_data",        km_test_new_params_with_rsa_defaults, 1024, KM_KEY_FORMAT_PKCS8,
                {km_test_remove_all_apps, km_test_add_app_data, 0}, BERR_INVALID_PARAMETER},

    };

    BDBG_LOG(("----------------------- %s -----------------------", BSTD_FUNCTION));

    for (i = 0; i < 4; i++) {
        km_algorithm_t algorithm;
        uint32_t key_size;
        km_key_format_t format;

        switch (i) {
        case 0:
            algorithm = KM_ALGORITHM_AES;
            key_size = 128;
            format = KM_KEY_FORMAT_RAW;
            break;
        case 1:
            algorithm = KM_ALGORITHM_HMAC;
            key_size = 160;
            format = KM_KEY_FORMAT_RAW;
            break;
        case 2:
            algorithm = KM_ALGORITHM_EC;
            key_size = 224;
            format = KM_KEY_FORMAT_PKCS8;
            break;
        case 3:
            algorithm = KM_ALGORITHM_RSA;
            key_size = 768;
            format = KM_KEY_FORMAT_PKCS8;
            break;
        default:
            err = BERR_UNKNOWN;
            goto done;
        }

        EXPECT_SUCCESS(km_test_new_params_with_aes_defaults(&key_params, key_size));
        EXPECT_SUCCESS(km_create_key_blob(algorithm, key_size, &in_key));

        /* No in_key_params */
        KeymasterTl_GetDefaultImportKeySettings(&impSettings);
        impSettings.in_key_format = format;
        impSettings.in_key_blob = in_key;
        EXPECT_FAILURE_CODE(KeymasterTl_ImportKey(handle, &impSettings), BSAGE_ERR_KM_UNEXPECTED_NULL_POINTER);
        /* Empty in_key_params */
        KeymasterTl_GetDefaultImportKeySettings(&impSettings);
        EXPECT_SUCCESS(KM_Tag_NewContext(&impSettings.in_key_params));
        impSettings.in_key_format = format;
        impSettings.in_key_blob = in_key;
        EXPECT_FAILURE_CODE(KeymasterTl_ImportKey(handle, &impSettings), BERR_INVALID_PARAMETER);
        TEST_DELETE_CONTEXT(impSettings.in_key_params);
        /* No in_key_blob */
        KeymasterTl_GetDefaultImportKeySettings(&impSettings);
        impSettings.in_key_params = key_params;
        impSettings.in_key_format = format;
        EXPECT_FAILURE_CODE(KeymasterTl_ImportKey(handle, &impSettings), BSAGE_ERR_KM_UNEXPECTED_NULL_POINTER);

        TEST_DELETE_CONTEXT(key_params);
        TEST_FREE_BLOCK(in_key);
    }

    for (i = 0; i < sizeof(params) / sizeof(test_param_data); i++) {
        bool test_failed = true;

        KeymasterTl_GetDefaultImportKeySettings(&impSettings);
        if (params[i].name[0] == 'R') {
            algorithm = KM_ALGORITHM_RSA;
        } else if (params[i].name[0] == 'E') {
            algorithm = KM_ALGORITHM_EC;
        } else if (params[i].name[0] == 'A') {
            algorithm = KM_ALGORITHM_AES;
        } else if (params[i].name[0] == 'H') {
            algorithm = KM_ALGORITHM_HMAC;
        } else {
            BDBG_ERR(("%s: malformed test name", BSTD_FUNCTION));
            err = BERR_INVALID_PARAMETER;
            goto done;
        }
        EXPECT_SUCCESS(km_create_key_blob(algorithm, params[i].key_size, &in_key));

        EXPECT_SUCCESS(params[i].fn(&key_params, params[i].key_size));
        impSettings.in_key_format = params[i].format;
        for (mod = 0; mod < MAX_MODIFIERS && params[i].mod_fn[mod]; mod++) {
            EXPECT_SUCCESS(params[i].mod_fn[mod](key_params));
        }

        impSettings.in_key_blob = in_key;
        impSettings.in_key_params = key_params;

        err = KeymasterTl_ImportKey(handle, &impSettings);
        if (params[i].expected_err == BERR_SUCCESS) {
            /* Should have passed */
            test_failed = (err != BERR_SUCCESS);
        } else {
            /* Should have failed */
            if (params[i].expected_err == NOT_BERR_SUCCESS) {
                test_failed = (err == BERR_SUCCESS);
            } else {
                test_failed = (err != params[i].expected_err);
            }
        }
        if (test_failed) {
            BDBG_ERR(("%s:%d iteration %d/%s failed", BSTD_FUNCTION, __LINE__, i, params[i].name));
            BDBG_ERR(("%s: err 0x%x, expected 0x%x", __FUNCTION__, err, params[i].expected_err));
            TEST_FREE_BLOCK(impSettings.out_key_blob);
            err = BERR_UNKNOWN;
            goto done;
        }

        if (params[i].expected_err == BERR_SUCCESS) {
            /* For completeness, check we can open the resulting key blob */
            EXPECT_SUCCESS(km_test_copy_app_id_and_data(key_params, &in_params));
            KeymasterTl_GetDefaultKeyCharacteristicsSettings(&chSettings);
            chSettings.in_key_blob = impSettings.out_key_blob;
            chSettings.in_params = in_params;
            EXPECT_SUCCESS(KeymasterTl_GetKeyCharacteristics(handle, &chSettings));
            EXPECT_TRUE(KM_Tag_ContainsEnum(chSettings.out_hw_enforced, KM_TAG_ORIGIN, KM_ORIGIN_IMPORTED));
            TEST_DELETE_CONTEXT(chSettings.out_hw_enforced);
            TEST_DELETE_CONTEXT(chSettings.out_sw_enforced);

            if (KM_Tag_GetNumPairs(in_params)) {
                /* If we have entries in in_params, test for failure to decrypt blob if we omit it */
                KeymasterTl_GetDefaultKeyCharacteristicsSettings(&chSettings);
                chSettings.in_key_blob = impSettings.out_key_blob;
                EXPECT_FAILURE_CODE(KeymasterTl_GetKeyCharacteristics(handle, &chSettings), BSAGE_ERR_KM_INVALID_KEY_BLOB);
            }
        }

        BDBG_LOG(("%s:%d success", params[i].name, i));
        BDBG_LOG(("\tReturned blob size %d (%p)", in_key.size, in_key.buffer));

        TEST_FREE_BLOCK(impSettings.out_key_blob);
        TEST_FREE_BLOCK(in_key);
        TEST_DELETE_CONTEXT(in_params);
        TEST_DELETE_CONTEXT(key_params);
    }
    err = BERR_SUCCESS;

done:
    TEST_FREE_BLOCK(impSettings.out_key_blob);
    TEST_FREE_BLOCK(in_key);
    TEST_DELETE_CONTEXT(in_params);
    TEST_DELETE_CONTEXT(key_params);
    return err;
}

#define ENABLE_CERT_VERIFY 0

#if ENABLE_CERT_VERIFY
static void km_print_cert_md5_sum(X509 *cert)
{
    uint8_t *blob;
    uint8_t *tmp;
    int len;
    uint8_t hash[MD5_DIGEST_LENGTH];
    int i;

    len = i2d_X509(cert, NULL);
    blob = (uint8_t *)malloc(len);
    tmp = blob;
    len = i2d_X509(cert, &tmp);

    MD5(blob, len, hash);
    for (i = 0; i < MD5_DIGEST_LENGTH; i++) {
        printf("%02x", hash[i]);
    }
    free(blob);
}

static int km_verify_callback(int ok, X509_STORE_CTX *ctx)
{
    int error = X509_STORE_CTX_get_error(ctx);
    X509 *cert = X509_STORE_CTX_get_current_cert(ctx);

    if (!ok) {
        if (cert) {

            X509_NAME_print_ex_fp(stdout,
                                  X509_get_subject_name(cert),
                                  0, XN_FLAG_ONELINE);
            printf("\n");

            printf("MD5 digest of cert: ");
            km_print_cert_md5_sum(cert);
            printf("\n");
        }
        printf("%serror %d at depth %d: '%s'\n",
               X509_STORE_CTX_get0_parent_ctx(ctx) ? "[CRL path]" : "",
               error,
               X509_STORE_CTX_get_error_depth(ctx),
               X509_verify_cert_error_string(error));

        ok = 1;

        return ok;

    }
    return (ok);
}
#endif

#define ATTEST_CHALLENGE    "km_test"

static BERR_Code km_attest_tests(KeymasterTl_Handle handle)
{
    BERR_Code err;
    KeymasterTl_ImportKeySettings impSettings = { 0 };
    KeymasterTl_AttestKeySettings attSettings = { 0 };
    KM_Tag_ContextHandle key_params = NULL;
    KM_Tag_ContextHandle in_params = NULL;
    KeymasterTl_DataBlock in_key = { 0 };
    km_algorithm_t algorithm;
    km_key_format_t format;
#if ENABLE_CERT_VERIFY
    X509_STORE *store = NULL;
    X509_STORE_CTX *ctx = NULL;
#endif
    int i;
    int method;
    int mod;
    test_param_data params[] = {
        {"AES 128",     km_test_new_params_with_aes_defaults,     128,  {0},   BSAGE_ERR_KM_INCOMPATIBLE_ALGORITHM},
        {"AES 192",     km_test_new_params_with_aes_defaults,     192,  {0},   BSAGE_ERR_KM_INCOMPATIBLE_ALGORITHM},
        {"AES 256",     km_test_new_params_with_aes_defaults,     256,  {0},   BSAGE_ERR_KM_INCOMPATIBLE_ALGORITHM},

        {"HMAC 160",   km_test_new_params_with_hmac_defaults,     160,  {0},   BSAGE_ERR_KM_INCOMPATIBLE_ALGORITHM},
        {"HMAC 224",   km_test_new_params_with_hmac_defaults,     224,  {0},   BSAGE_ERR_KM_INCOMPATIBLE_ALGORITHM},
        {"HMAC 256",   km_test_new_params_with_hmac_defaults,     256,  {0},   BSAGE_ERR_KM_INCOMPATIBLE_ALGORITHM},
        {"HMAC 512",   km_test_new_params_with_hmac_defaults,     512,  {0},   BSAGE_ERR_KM_INCOMPATIBLE_ALGORITHM},

        {"EC 224",     km_test_new_params_with_ec_defaults,       224,  {0},   BERR_SUCCESS},
        {"EC 256",     km_test_new_params_with_ec_defaults,       256,  {0},   BERR_SUCCESS},
        {"EC 384",     km_test_new_params_with_ec_defaults,       384,  {0},   BERR_SUCCESS},
        {"EC 521",     km_test_new_params_with_ec_defaults,       521,  {0},   BERR_SUCCESS},
        {"EC 224 -key_size",    km_test_new_params_with_ec_defaults, 224,
                {km_test_remove_key_size, 0}, BERR_SUCCESS},
        {"EC 224 -curve",       km_test_new_params_with_ec_defaults, 224,
                {km_test_remove_curve, 0},    BERR_SUCCESS},

        {"RSA 1024",    km_test_new_params_with_rsa_defaults,     1024, {0},   BERR_SUCCESS},

        {"EC app_id",           km_test_new_params_with_ec_defaults, 256,
                {km_test_remove_all_apps, km_test_add_app_id, 0}, BERR_SUCCESS},
        {"EC app_id+data",      km_test_new_params_with_ec_defaults, 256,
                {km_test_remove_all_apps, km_test_add_app_id, km_test_add_app_data, 0}, BERR_SUCCESS},

        {"RSA app_id",          km_test_new_params_with_rsa_defaults, 1024,
                {km_test_remove_all_apps, km_test_add_app_id, 0}, BERR_SUCCESS},
        {"RSA app_id+data",     km_test_new_params_with_rsa_defaults, 1024,
                {km_test_remove_all_apps, km_test_add_app_id, km_test_add_app_data, 0}, BERR_SUCCESS},
    };

    BDBG_LOG(("----------------------- %s -----------------------", BSTD_FUNCTION));

    /* Only thing that can be missing is the key blob */
    KeymasterTl_GetDefaultAttestKeySettings(&attSettings);
    EXPECT_FAILURE_CODE(KeymasterTl_AttestKey(handle, &attSettings), BSAGE_ERR_KM_UNEXPECTED_NULL_POINTER);

    /* The outer loop is whether we do a key generate or key import for attestation */
    for (method = 0; method < 2; method++) {
        for (i = 0; i < sizeof(params) / sizeof(test_param_data); i++) {
            bool test_failed = true;

            EXPECT_SUCCESS(params[i].fn(&key_params, params[i].key_size));
            for (mod = 0; mod < MAX_MODIFIERS && params[i].mod_fn[mod]; mod++) {
                EXPECT_SUCCESS(params[i].mod_fn[mod](key_params));
            }
            if (method == 0) {
                /* Generate the key */
                if (params[i].name[0] == 'H') {
                    TEST_TAG_ADD_ENUM(key_params, KM_TAG_DIGEST, KM_DIGEST_SHA_2_256);
                }
                EXPECT_SUCCESS(KeymasterTl_GenerateKey(handle, key_params, &in_key));
            } else {
                /* Create a key on the host and import the key */
                KeymasterTl_GetDefaultImportKeySettings(&impSettings);
                if (params[i].name[0] == 'R') {
                    algorithm = KM_ALGORITHM_RSA;
                    format = KM_KEY_FORMAT_PKCS8;
                } else if (params[i].name[0] == 'E') {
                    algorithm = KM_ALGORITHM_EC;
                    format = KM_KEY_FORMAT_PKCS8;
                } else if (params[i].name[0] == 'A') {
                    algorithm = KM_ALGORITHM_AES;
                    format = KM_KEY_FORMAT_RAW;
                } else if (params[i].name[0] == 'H') {
                    algorithm = KM_ALGORITHM_HMAC;
                    format = KM_KEY_FORMAT_RAW;
                    TEST_TAG_ADD_ENUM(key_params, KM_TAG_DIGEST, KM_DIGEST_SHA_2_256);
                } else {
                    BDBG_ERR(("%s: malformed test name", BSTD_FUNCTION));
                    err = BERR_INVALID_PARAMETER;
                    goto done;
                }
                EXPECT_SUCCESS(km_create_key_blob(algorithm, params[i].key_size, &in_key));

                impSettings.in_key_format = format;
                impSettings.in_key_blob = in_key;
                impSettings.in_key_params = key_params;

                EXPECT_SUCCESS(KeymasterTl_ImportKey(handle, &impSettings));

                /* Free the host-side blob and replace it with the imported blob */
                TEST_FREE_BLOCK(in_key);
                in_key = impSettings.out_key_blob;
            }
            EXPECT_SUCCESS(km_test_copy_app_id_and_data(key_params, &in_params));

            TEST_TAG_ADD_BYTES(in_params, KM_TAG_ATTESTATION_CHALLENGE, strlen(ATTEST_CHALLENGE), (uint8_t *)ATTEST_CHALLENGE);

            KeymasterTl_GetDefaultAttestKeySettings(&attSettings);
            attSettings.in_key_blob = in_key;
            attSettings.in_params = in_params;
            err = KeymasterTl_AttestKey(handle, &attSettings);
            if (params[i].expected_err == BERR_SUCCESS) {
                /* Should have passed */
                test_failed = (err != BERR_SUCCESS);
            } else {
                /* Should have failed */
                if (params[i].expected_err == NOT_BERR_SUCCESS) {
                    test_failed = (err == BERR_SUCCESS);
                } else {
                    test_failed = (err != params[i].expected_err);
                }
            }
            if (test_failed) {
                BDBG_ERR(("%s:%d method %s iteration %d/%s failed", BSTD_FUNCTION, __LINE__, method ? "IMPORT" : "GENERATE", i, params[i].name));
                BDBG_ERR(("%s: err 0x%x, expected 0x%x", __FUNCTION__, err, params[i].expected_err));
                err = BERR_UNKNOWN;
                goto done;
            }
            /* You need at least a leaf, an intermediate and a root */
            if ((params[i].expected_err == BERR_SUCCESS) && (attSettings.out_cert_chain.num < 3)) {
                BDBG_ERR(("%s: Insufficient cert data returned", BSTD_FUNCTION));
                err = BERR_UNKNOWN;
                goto done;
            }

            BDBG_LOG(("Method %s/%s:%d success", method ? "IMPORT" : "GENERATE", params[i].name, i));
            BDBG_LOG(("\tReturned certificate %d bytes, num %d", attSettings.out_cert_chain_buffer.size, attSettings.out_cert_chain.num));

    #if ENABLE_CERT_VERIFY
            /* If we got a cert chain back, verify it */
            if (attSettings.out_cert_chain.num > 0) {
                X509 *certificate[KM_CERTIFICATES_NUM_MAX] = { 0 };
                int cert;
                const uint8_t *p;

                store = X509_STORE_new();
                if (!store) {
                    BDBG_ERR(("%s: failed to allocate cert store", BSTD_FUNCTION));
                    err = BERR_UNKNOWN;
                    goto done;
                }
                X509_STORE_set_verify_cb(store, km_verify_callback);

                for (cert = attSettings.out_cert_chain.num - 1; cert > 0; cert--) {
                    p = &attSettings.out_cert_chain_buffer.buffer[attSettings.out_cert_chain.certificates[cert].offset];
                    certificate[cert] = d2i_X509(NULL, &p, attSettings.out_cert_chain.certificates[cert].length);

                    printf("Adding cert[%d] to store MD5: ", cert);
                    km_print_cert_md5_sum(certificate[cert]);
                    printf("\n");

                    if (X509_STORE_add_cert(store, certificate[cert]) < 0) {
                        BDBG_ERR(("%s: failed to add certificate", BSTD_FUNCTION));
                        err = BERR_UNKNOWN;
                        goto done;
                    }
                }

                p = &attSettings.out_cert_chain_buffer.buffer[attSettings.out_cert_chain.certificates[0].offset];
                certificate[0] = d2i_X509(NULL, &p, attSettings.out_cert_chain.certificates[0].length);

                ctx = X509_STORE_CTX_new();

                printf("Store init with cert[0] MD5: ");
                km_print_cert_md5_sum(certificate[0]);
                printf("\n");

                if (X509_STORE_CTX_init(ctx, store, certificate[1], NULL) < 0) {
                    BDBG_ERR(("%s: failed to init cert context", BSTD_FUNCTION));
                    err = BERR_UNKNOWN;
                    goto done;
                }

                if (X509_verify_cert(ctx) != 1) {
                    BDBG_ERR(("%s: failed to verify certificate", BSTD_FUNCTION));
                    err = BERR_UNKNOWN;
                    goto done;
                }

                X509_STORE_CTX_cleanup(ctx);
                X509_STORE_CTX_free(ctx);
                ctx = NULL;
                X509_STORE_free(store);
                store = NULL;
            }
    #endif

            TEST_FREE_BLOCK(attSettings.out_cert_chain_buffer);
            TEST_FREE_BLOCK(in_key);
            TEST_DELETE_CONTEXT(key_params);
            TEST_DELETE_CONTEXT(in_params);
        }
    }

    err = BERR_SUCCESS;

done:
#if ENABLE_CERT_VERIFY
    if (ctx) {
        X509_STORE_CTX_cleanup(ctx);
        X509_STORE_CTX_free(ctx);
    }
    if (store) {
        X509_STORE_free(store);
    }
#endif
    TEST_FREE_BLOCK(attSettings.out_cert_chain_buffer);
    TEST_FREE_BLOCK(in_key);
    TEST_DELETE_CONTEXT(key_params);
    TEST_DELETE_CONTEXT(in_params);
    return err;
}

int main(int argc, char *argv[])
{
    KeymasterTl_Handle handle = NULL;
    KeymasterTl_InitSettings initSettings;
    NEXUS_Error err = NEXUS_SUCCESS;
    BERR_Code berr;
    int rc;
    KM_Tag_ContextHandle params = NULL;

#ifdef NXCLIENT_SUPPORT
    NxClient_JoinSettings joinSettings;
#endif

    if (argc > 2) {
        BDBG_ERR(("\tInvalid number of command line arguments (%u)\n", argc));
        return (-1);
    }

#ifdef NXCLIENT_SUPPORT
    NxClient_GetDefaultJoinSettings(&joinSettings);
    snprintf(joinSettings.name, NXCLIENT_MAX_NAME, "keymaster_test");
    joinSettings.ignoreStandbyRequest = true;
    err = NxClient_Join(&joinSettings);
    if (err) return (-1);
#else
    err = SAGE_app_join_nexus();
    if (err) return (-1);
#endif

    KeymasterTl_GetDefaultInitSettings(&initSettings);

    if (argv[1] != NULL) {
        if (strlen(argv[1]) > sizeof(initSettings.drm_binfile_path)) {
            BDBG_ERR(("\tString length of argument '%s' is too large (%u bytes)\n", argv[1], (uint32_t)strlen(argv[1])));
            rc = -1;
            goto done;
        }
        memcpy(initSettings.drm_binfile_path, argv[1], strlen(argv[1]));
    }

    berr = KeymasterTl_Init(&handle, &initSettings);
    if (berr != BERR_SUCCESS) {
        BDBG_ERR(("### Keymaster init failed (%x)\n", berr));
        rc = -1;
        goto done;
    }

    EXPECT_SUCCESS(km_tests_before_configure(handle));

    EXPECT_SUCCESS(KM_Tag_NewContext(&params));
    TEST_TAG_ADD_INTEGER(params, KM_TAG_OS_VERSION, USE_OS_VERSION);
    TEST_TAG_ADD_INTEGER(params, KM_TAG_OS_PATCHLEVEL, USE_OS_PATCHLEVEL);
    EXPECT_SUCCESS(KeymasterTl_Configure(handle, params));

    EXPECT_SUCCESS(km_add_rng_tests(handle));
    EXPECT_SUCCESS(km_generate_tests(handle));
    EXPECT_SUCCESS(km_get_characteristics_tests(handle));
    EXPECT_SUCCESS(km_key_blob_tests(handle));
    EXPECT_SUCCESS(km_import_tests(handle));
    EXPECT_SUCCESS(km_export_tests(handle));
    EXPECT_SUCCESS(km_attest_tests(handle));
    EXPECT_SUCCESS(km_crypto_aes_tests(handle));
    EXPECT_SUCCESS(km_crypto_rsa_tests(handle));
    EXPECT_SUCCESS(km_crypto_hmac_tests(handle));

    BDBG_LOG(("%s: ***** ALL TESTS PASSED *****", BSTD_FUNCTION));

done:
    TEST_DELETE_CONTEXT(params);
    if (handle) {
        KeymasterTl_Uninit(handle);
    }
#ifdef NXCLIENT_SUPPORT
    NxClient_Uninit();
#else
    SAGE_app_leave_nexus();
#endif

    if (rc) {
        BDBG_ERR(("Failure in SAGE Keymaster test\n"));
    }

    return rc;
}
