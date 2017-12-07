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

#include "nexus_platform.h"
#include "nexus_platform_init.h"

#ifdef NXCLIENT_SUPPORT
#include "nxclient.h"
#endif

#include "keymaster_ids.h"
#include "keymaster_platform.h"
#include "keymaster_tl.h"
#include "keymaster_test.h"
#include "keymaster_key_params.h"
#include "keymaster_crypto_aes.h"


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

    /* Add RNG entropy */
    TEST_ALLOCATE_BLOCK(in_data, TEST0_DATA_SIZE);
    EXPECT_FAILURE_CODE(KeymasterTl_AddRngEntropy(handle, &in_data), BSAGE_ERR_KM_KEYMASTER_NOT_CONFIGURED);
    TEST_FREE_BLOCK(in_data);

    /* Generate key */
    EXPECT_SUCCESS(KM_Tag_NewContext(&params));
    EXPECT_SUCCESS(km_test_aes_add_default_params(params, TEST0_KEY_SIZE));
    EXPECT_FAILURE_CODE(KeymasterTl_GenerateKey(handle, params, &key), BSAGE_ERR_KM_KEYMASTER_NOT_CONFIGURED);
    TEST_DELETE_CONTEXT(params);

    /* Get characteristics */
    KeymasterTl_GetDefaultKeyCharacteristicsSettings(&chSettings);
    chSettings.in_key_blob.buffer = dummy_key;
    chSettings.in_key_blob.size = sizeof(dummy_key);
    EXPECT_SUCCESS(KM_Tag_NewContext(&chSettings.in_params));
    EXPECT_SUCCESS(km_test_aes_add_default_params(chSettings.in_params, TEST0_KEY_SIZE));
    EXPECT_FAILURE_CODE(KeymasterTl_GetKeyCharacteristics(handle, &chSettings), BSAGE_ERR_KM_KEYMASTER_NOT_CONFIGURED);
    TEST_DELETE_CONTEXT(chSettings.in_params);
    BDBG_ASSERT(!chSettings.out_hw_enforced);
    BDBG_ASSERT(!chSettings.out_sw_enforced);

    /* Import key */
    KeymasterTl_GetDefaultImportKeySettings(&impSettings);
    EXPECT_SUCCESS(KM_Tag_NewContext(&impSettings.in_key_params));
    EXPECT_SUCCESS(km_test_aes_add_default_params(impSettings.in_key_params, TEST0_KEY_SIZE));
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
    EXPECT_SUCCESS(KM_Tag_NewContext(&expSettings.in_params));
    EXPECT_SUCCESS(km_test_ec_add_default_params(expSettings.in_params, TEST0_KEY_SIZE));
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

    EXPECT_FAILURE_CODE(KeymasterTl_AddRngEntropy(handle, &in_data), BERR_INVALID_PARAMETER);

    TEST_ALLOCATE_BLOCK(in_data, TEST0_DATA_SIZE);
    EXPECT_SUCCESS(KeymasterTl_AddRngEntropy(handle, &in_data));
    err = BERR_SUCCESS;

done:
    TEST_FREE_BLOCK(in_data);
    return err;
}

#define MAX_MODIFIERS   5

typedef struct {
    const char *name;
    km_key_param_fn fn;
    uint32_t key_size;
    km_key_modifier_fn mod_fn[MAX_MODIFIERS];
    bool positive_test;
} test_param_data;

static BERR_Code km_generate_tests(KeymasterTl_Handle handle)
{
    BERR_Code err;
    KM_Tag_ContextHandle key_params = NULL;
    KeymasterTl_DataBlock key = { 0 };
    int i;
    int mod;
    test_param_data params[] = {
        {"AES 128",     km_test_aes_add_default_params,     128,  {0},   true},
        {"AES 192",     km_test_aes_add_default_params,     192,  {0},   true},
        {"AES 256",     km_test_aes_add_default_params,     256,  {0},   true},
        {"AES 112",     km_test_aes_add_default_params,     112,  {0},   false},
        {"AES 272",     km_test_aes_add_default_params,     272,  {0},   false},
        {"AES 256 -key_size",  km_test_aes_add_default_params, 256,
                {km_test_remove_key_size, 0},   false},

        {"HMAC 160",   km_test_hmac_add_default_params,     160,  {0},   true},
        {"HMAC 224",   km_test_hmac_add_default_params,     224,  {0},   true},
        {"HMAC 256",   km_test_hmac_add_default_params,     256,  {0},   true},
        {"HMAC 512",   km_test_hmac_add_default_params,     512,  {0},   true},
        {"HMAC 144",   km_test_hmac_add_default_params,     144,  {0},   false},
        {"HMAC 168",   km_test_hmac_add_default_params,     168,  {0},   false},
        {"HMAC 528",   km_test_hmac_add_default_params,     528,  {0},   false},
        {"HMAC 160 -key_size",   km_test_hmac_add_default_params, 160,
                {km_test_remove_key_size, 0},   false},

        {"EC 224",     km_test_ec_add_default_params,       224,  {0},   true},
        {"EC 256",     km_test_ec_add_default_params,       256,  {0},   true},
        {"EC 384",     km_test_ec_add_default_params,       384,  {0},   true},
        {"EC 521",     km_test_ec_add_default_params,       521,  {0},   true},
        {"EC 521",     km_test_ec_add_default_params,       512,  {0},   false},
        {"EC 512",     km_test_ec_add_default_params,       208,  {0},   false},
        {"EC 224 -key_size",    km_test_ec_add_default_params, 224,
                {km_test_remove_key_size, 0}, true},
        {"EC 224 -curve",       km_test_ec_add_default_params, 224,
                {km_test_remove_curve, 0},    true},
        {"EC 224 -curve -key",  km_test_ec_add_default_params, 224,
                {km_test_remove_key_size, km_test_remove_curve, 0}, false},

        {"RSA 1024",    km_test_rsa_add_default_params,     1024, {0},   true},
        {"RSA 2048",    km_test_rsa_add_default_params,     2048, {0},   false}, /* TEMP disabled */
        {"RSA 3072",    km_test_rsa_add_default_params,     3072, {0},   false}, /* TEMP disabled */
        {"RSA 4096",    km_test_rsa_add_default_params,     4096, {0},   false}, /* TEMP disabled */
        {"RSA 512",     km_test_rsa_add_default_params,     512,  {0},   false},
        {"RSA 5120",    km_test_rsa_add_default_params,     5120, {0},   false},
        {"RSA 1024 -key_size",  km_test_rsa_add_default_params, 1024,
                {km_test_remove_key_size, 0}, false},
        {"RSA 1024 -exp",       km_test_rsa_add_default_params, 1024,
                {km_test_remove_exponent, 0}, false},

        {"AES 256 app_id",      km_test_aes_add_default_params, 256,
                {km_test_remove_all_apps, km_test_add_app_id, 0}, true},
        {"AES 256 app_id+data", km_test_aes_add_default_params, 256,
                {km_test_remove_all_apps, km_test_add_app_id, km_test_add_app_data, 0}, true},
        {"AES 256 app_data",    km_test_aes_add_default_params, 256,
                {km_test_remove_all_apps, km_test_add_app_data, 0}, false},
        {"AES 256 app_id+all_apps", km_test_aes_add_default_params, 256,
                {km_test_add_app_id, 0}, false},
        {"AES 256 -all_apps",   km_test_aes_add_default_params, 256,
                {km_test_remove_all_apps, 0}, false},

        {"HMAC app_id",         km_test_hmac_add_default_params, 256,
                {km_test_remove_all_apps, km_test_add_app_id, 0}, true},
        {"HMAC app_id+data",    km_test_hmac_add_default_params, 256,
                {km_test_remove_all_apps, km_test_add_app_id, km_test_add_app_data, 0}, true},
        {"HMAC app_data",       km_test_hmac_add_default_params, 256,
                {km_test_remove_all_apps, km_test_add_app_data, 0}, false},
        {"HMAC app_id+all_apps",    km_test_hmac_add_default_params, 256,
                {km_test_add_app_id, 0}, false},
        {"HMAC -all_apps",      km_test_hmac_add_default_params, 256,
                {km_test_remove_all_apps, 0}, false},

        {"EC app_id",           km_test_ec_add_default_params, 256,
                {km_test_remove_all_apps, km_test_add_app_id, 0}, true},
        {"EC app_id+data",      km_test_ec_add_default_params, 256,
                {km_test_remove_all_apps, km_test_add_app_id, km_test_add_app_data, 0}, true},
        {"EC app_data",         km_test_ec_add_default_params, 256,
                {km_test_remove_all_apps, km_test_add_app_data, 0}, false},
        {"EC app_id_all_apps",  km_test_ec_add_default_params, 256,
                {km_test_add_app_id, 0}, false},
        {"EC -all_apps",        km_test_ec_add_default_params, 256,
                {km_test_remove_all_apps, 0}, false},

        {"RSA app_id",          km_test_rsa_add_default_params, 1024,
                {km_test_remove_all_apps, km_test_add_app_id, 0}, true},
        {"RSA app_id+data",     km_test_rsa_add_default_params, 1024,
                {km_test_remove_all_apps, km_test_add_app_id, km_test_add_app_data, 0}, true},
        {"RSA app_data",        km_test_rsa_add_default_params, 1024,
                {km_test_remove_all_apps, km_test_add_app_data, 0}, false},
        {"RSA app_id+all_apps", km_test_rsa_add_default_params, 1024,
                {km_test_add_app_id, 0}, false},
        {"RSA -all_apps",       km_test_rsa_add_default_params, 1024,
                {km_test_remove_all_apps, 0}, false},
    };

    EXPECT_SUCCESS(KM_Tag_NewContext(&key_params));
    EXPECT_SUCCESS(km_test_aes_add_default_params(key_params, 128));
    EXPECT_FAILURE_CODE(KeymasterTl_GenerateKey(handle, NULL, &key), BERR_INVALID_PARAMETER);
    EXPECT_FAILURE_CODE(KeymasterTl_GenerateKey(handle, key_params, NULL), BERR_INVALID_PARAMETER);
    TEST_DELETE_CONTEXT(key_params);

    for (i = 0; i < sizeof(params) / sizeof(test_param_data); i++) {
        EXPECT_SUCCESS(KM_Tag_NewContext(&key_params));
        EXPECT_SUCCESS(params[i].fn(key_params, params[i].key_size));
        for (mod = 0; mod < MAX_MODIFIERS && params[i].mod_fn[mod]; mod++) {
            EXPECT_SUCCESS(params[i].mod_fn[mod](key_params));
        }
        err = KeymasterTl_GenerateKey(handle, key_params, &key);
        if (params[i].positive_test && (err != BERR_SUCCESS)) {
            BDBG_ERR(("%s:%d iteration %d/%s failed", BSTD_FUNCTION, __LINE__, i, params[i].name));
            goto done;
        }
        if (!params[i].positive_test && (err == BERR_SUCCESS)) {
            BDBG_ERR(("%s:%d iteration %d/%s failed", BSTD_FUNCTION, __LINE__, i, params[i].name));
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
        {"AES 128",     km_test_aes_add_default_params,     128,  {0},   true},
        {"AES 192",     km_test_aes_add_default_params,     192,  {0},   true},
        {"AES 256",     km_test_aes_add_default_params,     256,  {0},   true},

        {"HMAC 160",   km_test_hmac_add_default_params,     160,  {0},   true},
        {"HMAC 224",   km_test_hmac_add_default_params,     224,  {0},   true},
        {"HMAC 256",   km_test_hmac_add_default_params,     256,  {0},   true},
        {"HMAC 512",   km_test_hmac_add_default_params,     512,  {0},   true},

        {"EC 224",     km_test_ec_add_default_params,       224,  {0},   true},
        {"EC 256",     km_test_ec_add_default_params,       256,  {0},   true},
        {"EC 384",     km_test_ec_add_default_params,       384,  {0},   true},
        {"EC 521",     km_test_ec_add_default_params,       521,  {0},   true},
        {"EC 224 -key_size",    km_test_ec_add_default_params, 224,
                {km_test_remove_key_size, 0}, true},
        {"EC 224 -curve",       km_test_ec_add_default_params, 224,
                {km_test_remove_curve, 0},    true},

        {"RSA 1024",    km_test_rsa_add_default_params,     1024, {0},   true},

        {"AES 256 app_id",      km_test_aes_add_default_params, 256,
                {km_test_remove_all_apps, km_test_add_app_id, 0}, true},
        {"AES 256 app_id+data", km_test_aes_add_default_params, 256,
                {km_test_remove_all_apps, km_test_add_app_id, km_test_add_app_data, 0}, true},

        {"HMAC app_id",         km_test_hmac_add_default_params, 256,
                {km_test_remove_all_apps, km_test_add_app_id, 0}, true},
        {"HMAC app_id+data",    km_test_hmac_add_default_params, 256,
                {km_test_remove_all_apps, km_test_add_app_id, km_test_add_app_data, 0}, true},

        {"EC app_id",           km_test_ec_add_default_params, 256,
                {km_test_remove_all_apps, km_test_add_app_id, 0}, true},
        {"EC app_id+data",      km_test_ec_add_default_params, 256,
                {km_test_remove_all_apps, km_test_add_app_id, km_test_add_app_data, 0}, true},

        {"RSA app_id",          km_test_rsa_add_default_params, 1024,
                {km_test_remove_all_apps, km_test_add_app_id, 0}, true},
        {"RSA app_id+data",     km_test_rsa_add_default_params, 1024,
                {km_test_remove_all_apps, km_test_add_app_id, km_test_add_app_data, 0}, true},
    };

    /* Check params: the only thing that can fail is not passing in a key blob */
    KeymasterTl_GetDefaultKeyCharacteristicsSettings(&chSettings);
    EXPECT_FAILURE_CODE(KeymasterTl_GetKeyCharacteristics(handle, &chSettings), BERR_INVALID_PARAMETER);

    for (i = 0; i < sizeof(params) / sizeof(test_param_data); i++) {
        EXPECT_SUCCESS(KM_Tag_NewContext(&key_params));
        EXPECT_SUCCESS(params[i].fn(key_params, params[i].key_size));
        for (mod = 0; mod < MAX_MODIFIERS && params[i].mod_fn[mod]; mod++) {
            EXPECT_SUCCESS(params[i].mod_fn[mod](key_params));
        }
        EXPECT_SUCCESS(KeymasterTl_GenerateKey(handle, key_params, &key));
        EXPECT_SUCCESS(km_test_copy_app_id_and_data(key_params, &in_params));
        KeymasterTl_GetDefaultKeyCharacteristicsSettings(&chSettings);
        chSettings.in_key_blob = key;
        chSettings.in_params = in_params;
        EXPECT_SUCCESS(KeymasterTl_GetKeyCharacteristics(handle, &chSettings));
        TEST_DELETE_CONTEXT(chSettings.out_hw_enforced);
        TEST_DELETE_CONTEXT(chSettings.out_sw_enforced);

        if (KM_Tag_GetNumPairs(in_params)) {
            /* If we have entries in in_params, test for failure to decrypt blob if we omit it */
            KeymasterTl_GetDefaultKeyCharacteristicsSettings(&chSettings);
            chSettings.in_key_blob = key;
            EXPECT_FAILURE_CODE(KeymasterTl_GetKeyCharacteristics(handle, &chSettings), BSAGE_ERR_KM_INVALID_KEY_BLOB);
            TEST_DELETE_CONTEXT(chSettings.out_hw_enforced);
            TEST_DELETE_CONTEXT(chSettings.out_sw_enforced);
        }

        BDBG_LOG(("%s:%d success", params[i].name, i));
        BDBG_LOG(("\tReturned blob size %d (%p)", key.size, key.buffer));

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

    EXPECT_SUCCESS(KM_Tag_NewContext(&key_params));
    EXPECT_SUCCESS(km_test_aes_add_default_params(key_params, 128));
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
        {"AES 128",     km_test_aes_add_default_params,     128,  {0},   false},
        {"AES 192",     km_test_aes_add_default_params,     192,  {0},   false},
        {"AES 256",     km_test_aes_add_default_params,     256,  {0},   false},

        {"HMAC 160",   km_test_hmac_add_default_params,     160,  {0},   false},
        {"HMAC 224",   km_test_hmac_add_default_params,     224,  {0},   false},
        {"HMAC 256",   km_test_hmac_add_default_params,     256,  {0},   false},
        {"HMAC 512",   km_test_hmac_add_default_params,     512,  {0},   false},

        {"EC 224",     km_test_ec_add_default_params,       224,  {0},   true},
        {"EC 256",     km_test_ec_add_default_params,       256,  {0},   true},
        {"EC 384",     km_test_ec_add_default_params,       384,  {0},   true},
        {"EC 521",     km_test_ec_add_default_params,       521,  {0},   true},
        {"EC 224 -key_size",    km_test_ec_add_default_params, 224,
                {km_test_remove_key_size, 0}, true},
        {"EC 224 -curve",       km_test_ec_add_default_params, 224,
                {km_test_remove_curve, 0},    true},

        {"RSA 1024",    km_test_rsa_add_default_params,     1024, {0},   true},

        {"EC app_id",           km_test_ec_add_default_params, 256,
                {km_test_remove_all_apps, km_test_add_app_id, 0}, true},
        {"EC app_id+data",      km_test_ec_add_default_params, 256,
                {km_test_remove_all_apps, km_test_add_app_id, km_test_add_app_data, 0}, true},

        {"RSA app_id",          km_test_rsa_add_default_params, 1024,
                {km_test_remove_all_apps, km_test_add_app_id, 0}, true},
        {"RSA app_id+data",     km_test_rsa_add_default_params, 1024,
                {km_test_remove_all_apps, km_test_add_app_id, km_test_add_app_data, 0}, true},
    };

    EXPECT_SUCCESS(KM_Tag_NewContext(&key_params));
    EXPECT_SUCCESS(km_test_ec_add_default_params(key_params, 256));
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
    EXPECT_FAILURE_CODE(KeymasterTl_ExportKey(handle, &expSettings), BERR_INVALID_PARAMETER);
    TEST_FREE_BLOCK(in_key);
    TEST_DELETE_CONTEXT(key_params);

    for (i = 0; i < sizeof(params) / sizeof(test_param_data); i++) {
        EXPECT_SUCCESS(KM_Tag_NewContext(&key_params));
        EXPECT_SUCCESS(params[i].fn(key_params, params[i].key_size));
        for (mod = 0; mod < MAX_MODIFIERS && params[i].mod_fn[mod]; mod++) {
            EXPECT_SUCCESS(params[i].mod_fn[mod](key_params));
        }
        EXPECT_SUCCESS(KeymasterTl_GenerateKey(handle, key_params, &in_key));
        EXPECT_SUCCESS(km_test_copy_app_id_and_data(key_params, &in_params));

        KeymasterTl_GetDefaultExportKeySettings(&expSettings);
        expSettings.in_key_format = KM_KEY_FORMAT_X509;
        expSettings.in_key_blob = in_key;
        expSettings.in_params = in_params;
        err = KeymasterTl_ExportKey(handle, &expSettings);
        if (params[i].positive_test && (err != BERR_SUCCESS)) {
            BDBG_ERR(("%s:%d iteration %d/%s failed", BSTD_FUNCTION, __LINE__, i, params[i].name));
            goto done;
        }
        if (!params[i].positive_test && (err == BERR_SUCCESS)) {
            TEST_FREE_BLOCK(expSettings.out_key_blob);
            BDBG_ERR(("%s:%d iteration %d/%s failed", BSTD_FUNCTION, __LINE__, i, params[i].name));
            goto done;
        }

        BDBG_LOG(("%s:%d success", params[i].name, i));
        BDBG_LOG(("\tReturned blob size %d (%p)", in_key.size, in_key.buffer));

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
    EXPECT_SUCCESS(km_export_tests(handle));
    EXPECT_SUCCESS(km_crypto_aes_tests(handle));

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
