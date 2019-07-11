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

#include <ftw.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>

#include "keymaster_debug.h"
#include "bstd.h"
#include "bkni.h"

#include "openssl/x509.h"
#include "openssl/x509_vfy.h"
#include "openssl/md5.h"

#include "nexus_platform.h"
#include "nexus_platform_init.h"
#include "nexus_security_client.h"

#ifdef NXCLIENT_SUPPORT
#include "nxclient.h"
#endif

#include "keymaster_ids.h"
#include "keymaster_platform.h"
#include "keymaster_tl.h"
#include "keymaster_test.h"
#include "keymaster_keygen.h"
#include "keymaster_key_params.h"
#include "wrapped_key_data.h"


BDBG_MODULE(kemaster_4_test);

#define USE_OS_VERSION     1
#define USE_OS_PATCHLEVEL  201603U

#define EXPECT_SUCCESS_UNLOCK_ON_ERR(op)  err = op; if (err != BERR_SUCCESS) { BDBG_ERR(("%s:%d  %s failed (err %x)", BSTD_FUNCTION, __LINE__, #op, err)); \
                                                                               pthread_mutex_unlock(state->mutex); goto done; }

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

BERR_Code HmacSharingTest(KeymasterTl_Handle handle)
{
    BERR_Code err = BERR_UNKNOWN;
    KeymasterTl_GetHmacSharingParamsSettings paramSettings;
    KeymasterTl_GetComputeSharedHmacSettings hmacSettings;
    KeymasterTl_VerifyAuthorizationSettings verifySettings;
    km_verification_token_t verificationToken = { 0 };
    uint32_t i;
    uint64_t challenge1 = 0x1664689e0fff8159ull;
    uint64_t challenge2 = 0x0151f68f0e3ca087ull;
    uint64_t timestamp;

    KeymasterTl_GetDefaultHmacSharingParamsSettings(&paramSettings);
    KeymasterTl_GetDefaultComputeSharedHmacSettings(&hmacSettings);
    KeymasterTl_GetDefaultVerifyAuthorizationSettings(&verifySettings);

    EXPECT_SUCCESS(KeymasterTl_GetHmacSharingParams(handle, &paramSettings));
    if ((paramSettings.out_seed.size != SKM_SHA256_DIGEST_SIZE) ||
        (paramSettings.out_nonce.size != SKM_SHA256_DIGEST_SIZE)) {
        BDBG_ERR(("%s: Unexpected size returned from KeymasterTl_GetHmacSharingParams (%u, %u)",
                        BSTD_FUNCTION, paramSettings.out_seed.size, paramSettings.out_nonce.size));
        err = BERR_UNKNOWN;
        goto done;
    }

    hmacSettings.in_num_params = 1;
    memcpy(&hmacSettings.in_sharing_params[0].seed[0], paramSettings.out_seed.buffer, SKM_SHA256_DIGEST_SIZE);
    memcpy(&hmacSettings.in_sharing_params[0].nonce[0], paramSettings.out_nonce.buffer, SKM_SHA256_DIGEST_SIZE);
    EXPECT_SUCCESS(KeymasterTl_ComputeSharedHmac(handle, &hmacSettings));

    verifySettings.in_challenge = challenge1;  /* Just a random challenge */
    /* Not filling in in_params or in_auth_token */
    verifySettings.out_verification_token = &verificationToken;
    EXPECT_SUCCESS(KeymasterTl_VerifyAuthorization(handle, &verifySettings));

    if (verificationToken.challenge != challenge1) {
        BDBG_ERR(("%s: returned challenge incorrect", BSTD_FUNCTION));
        err = BERR_UNKNOWN;
        goto done;
    }
    if (!verificationToken.timestamp) {
        BDBG_ERR(("%s: returned timestamp is zero", BSTD_FUNCTION));
        err = BERR_UNKNOWN;
        goto done;
    }
    if (verificationToken.security_level != SKM_SECURITY_LEVEL_TRUSTED_ENVIRONMENT) {
        BDBG_ERR(("%s: returned security level incorrect (%x)", BSTD_FUNCTION, verificationToken.security_level));
        err = BERR_UNKNOWN;
        goto done;
    }
    for (i = 0; i < SKM_SHA256_DIGEST_SIZE; i++) {
        if (verificationToken.mac[i])
            break;
    }
    if (i == SKM_SHA256_DIGEST_SIZE) {
        BDBG_ERR(("%s: returned mac is null", BSTD_FUNCTION));
        err = BERR_UNKNOWN;
        goto done;
    }
    timestamp = verificationToken.timestamp;
    sleep(1);

    verifySettings.in_challenge = challenge2;  /* Just a random challenge */
    /* Not filling in in_params or in_auth_token */
    verifySettings.out_verification_token = &verificationToken;
    EXPECT_SUCCESS(KeymasterTl_VerifyAuthorization(handle, &verifySettings));

    if (verificationToken.challenge != challenge2) {
        BDBG_ERR(("%s: returned challenge incorrect", BSTD_FUNCTION));
        err = BERR_UNKNOWN;
        goto done;
    }
    if (!verificationToken.timestamp) {
        BDBG_ERR(("%s: returned timestamp is zero", BSTD_FUNCTION));
        err = BERR_UNKNOWN;
        goto done;
    }
    if (verificationToken.security_level != SKM_SECURITY_LEVEL_TRUSTED_ENVIRONMENT) {
        BDBG_ERR(("%s: returned security level incorrect (%x)", BSTD_FUNCTION, verificationToken.security_level));
        err = BERR_UNKNOWN;
        goto done;
    }
    for (i = 0; i < SKM_SHA256_DIGEST_SIZE; i++) {
        if (verificationToken.mac[i])
            break;
    }
    if (i == SKM_SHA256_DIGEST_SIZE) {
        BDBG_ERR(("%s: returned mac is null", BSTD_FUNCTION));
        err = BERR_UNKNOWN;
        goto done;
    }
    if (verificationToken.timestamp - timestamp < 1000) {
        BDBG_ERR(("%s: Timestamp diff %llu", BSTD_FUNCTION, (long long unsigned) (verificationToken.timestamp - timestamp)));
        err = BERR_UNKNOWN;
        goto done;
    }

    err = BERR_SUCCESS;

done:
    TEST_FREE_BLOCK(paramSettings.out_seed);
    TEST_FREE_BLOCK(paramSettings.out_nonce);
    TEST_FREE_BLOCK(hmacSettings.out_sharing_check);
    return err;
}

void FillInWrappedKeySettings(KM_Tag_ContextHandle key_params,
                              const km_key_format_t key_format,
                              const uint8_t *wrapped_key, uint32_t wrapped_key_size,
                              const uint8_t *tag_data, uint32_t tag_data_size,
                              KeymasterTl_ImportWrappedKeySettings *settings)
{
    settings->version = 0;
    settings->in_transit_key.size = sizeof(TRANSIT_KEY_ENC);
    settings->in_transit_key.buffer = (uint8_t *)TRANSIT_KEY_ENC;
    settings->in_iv.size = sizeof(IV);
    settings->in_iv.buffer = (uint8_t *)IV;
    settings->in_key_format = key_format;
    settings->in_key_params = key_params;
    settings->in_secure_key.size = wrapped_key_size;
    settings->in_secure_key.buffer = (uint8_t *)wrapped_key;
    settings->in_tag_data.size = tag_data_size;
    settings->in_tag_data.buffer = (uint8_t *)tag_data;
    settings->in_description.size = sizeof(DESC);
    settings->in_description.buffer = (uint8_t *)DESC;
}

BERR_Code ImportWrappedKeyTest(KeymasterTl_Handle handle)
{
    BERR_Code err = BERR_UNKNOWN;
    KM_Tag_ContextHandle import_key_params = NULL;
    KeymasterTl_DataBlock key_blob = { 0 };
    KeymasterTl_DataBlock imported_key_blob = { 0 };
    KM_Tag_ContextHandle key_params = NULL;
    KeymasterTl_ImportKeySettings impSettings;
    KeymasterTl_ImportWrappedKeySettings settings;
    KeymasterTl_DataBlock resulting_key_blob = { 0 };
    KeymasterTl_GetKeyCharacteristicsSettings chSettings;
    int i;

    /* We first import the RSA key which is used to wrap the transit key */
    EXPECT_SUCCESS(km_test_new_params_with_rsa_defaults(&import_key_params, RSA_KEY_SIZE));
    /* Add the wrap purpose to the key, or we won't be able to import the wrapped key */
    TEST_TAG_ADD_ENUM(import_key_params, SKM_TAG_PURPOSE, SKM_PURPOSE_WRAP);
    KeymasterTl_GetDefaultImportKeySettings(&impSettings);
    impSettings.in_key_format = SKM_KEY_FORMAT_PKCS8;
    TEST_ALLOCATE_BLOCK(key_blob, sizeof(RSA_PKCS8_STRING));
    memcpy(key_blob.buffer, &RSA_PKCS8_STRING[0], sizeof(RSA_PKCS8_STRING));
    impSettings.in_key_blob = key_blob;
    impSettings.in_key_params = import_key_params;
    EXPECT_SUCCESS(KeymasterTl_ImportKey(handle, &impSettings));
    imported_key_blob = impSettings.out_key_blob;

    for (i = 0; i < 2; i++) {
        KeymasterTl_GetDefaultImportWrappedKeySettings(&settings);

        switch (i) {
        case 0:
            /* Test wrapped AES key import */
            BDBG_LOG(("%s: testing AES key import", BSTD_FUNCTION));
            EXPECT_SUCCESS(km_test_new_params_with_aes_defaults(&key_params, AES_CLEAR_KEY_SIZE));
            FillInWrappedKeySettings(key_params, AES_KEY_FORMAT, AES_ENC_WRAPPED_KEY, sizeof(AES_ENC_WRAPPED_KEY), AES_TAG_DATA, sizeof(AES_TAG_DATA), &settings);
            break;
        case 1:
            /* Test wrapped RSA key import */
            BDBG_LOG(("%s: testing RSA key import", BSTD_FUNCTION));
            EXPECT_SUCCESS(km_test_new_params_with_rsa_defaults(&key_params, RSA_CLEAR_KEY_SIZE));
            FillInWrappedKeySettings(key_params, RSA_KEY_FORMAT, RSA_ENC_WRAPPED_KEY, sizeof(RSA_ENC_WRAPPED_KEY), RSA_TAG_DATA, sizeof(RSA_TAG_DATA), &settings);
            break;
        default:
            BDBG_ERR(("Unknown import stage"));
            goto done;
        }

        settings.in_wrapping_key = imported_key_blob;
        settings.in_masking_key.size = sizeof(MASK);
        settings.in_masking_key.buffer = (uint8_t *)MASK;
        settings.in_params = NULL;    /* Nothing required to auth the use of RSA key */
        EXPECT_SUCCESS(KeymasterTl_ImportWrappedKey(handle, &settings));
        resulting_key_blob = settings.out_key_blob;
        BDBG_LOG(("%s: key import completed okay", BSTD_FUNCTION));

        /* Check that we can get the characteristics of the imported key */
        KeymasterTl_GetDefaultKeyCharacteristicsSettings(&chSettings);
        chSettings.in_key_blob = resulting_key_blob;
        chSettings.in_params = NULL;
        EXPECT_SUCCESS(KeymasterTl_GetKeyCharacteristics(handle, &chSettings));
        EXPECT_TRUE(KM_Tag_ContainsEnum(chSettings.out_hw_enforced, SKM_TAG_ORIGIN, SKM_ORIGIN_IMPORTED));
        TEST_DELETE_CONTEXT(chSettings.out_hw_enforced);
        TEST_DELETE_CONTEXT(chSettings.out_sw_enforced);
        BDBG_LOG(("%s: get key characteristics completed okay", BSTD_FUNCTION));

        TEST_DELETE_CONTEXT(key_params);
        TEST_FREE_BLOCK(resulting_key_blob);
    }

    EXPECT_SUCCESS(KeymasterTl_DeleteAllKeys(handle));

done:
    TEST_DELETE_CONTEXT(import_key_params);
    TEST_FREE_BLOCK(key_blob);
    TEST_FREE_BLOCK(imported_key_blob);
    TEST_DELETE_CONTEXT(key_params);
    TEST_FREE_BLOCK(resulting_key_blob);
    return err;
}

int main(int argc, char *argv[])
{
    KeymasterTl_Handle handle = NULL;
    KeymasterTl_InitSettings initSettings;
    NEXUS_Error err = NEXUS_SUCCESS;
    BERR_Code berr;
    int rc = 0;
    KM_Tag_ContextHandle params = NULL;
    uint32_t vendor_patchlevel = 20190101U;
#ifdef NXCLIENT_SUPPORT
    NxClient_JoinSettings joinSettings;
#endif

    if (argc > 2) {
        BDBG_ERR(("\tInvalid number of command line arguments (%u)\n", argc));
        return (-1);
    }

#ifdef NXCLIENT_SUPPORT
    NxClient_GetDefaultJoinSettings(&joinSettings);
    snprintf(joinSettings.name, NXCLIENT_MAX_NAME, "keymaster_4_test");
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

    initSettings.version = SKM_VERSION_4;
    berr = KeymasterTl_Init(&handle, &initSettings);
    if (berr != BERR_SUCCESS) {
        BDBG_ERR(("### Keymaster init failed (%x)\n", berr));
        rc = -1;
        goto done;
    }

    /* Before configure, we call the HMAC sharing API */
    EXPECT_SUCCESS(HmacSharingTest(handle));

    EXPECT_SUCCESS(KM_Tag_NewContext(&params));
    TEST_TAG_ADD_INTEGER(params, SKM_TAG_OS_VERSION, USE_OS_VERSION);
    TEST_TAG_ADD_INTEGER(params, SKM_TAG_OS_PATCHLEVEL, USE_OS_PATCHLEVEL);
    EXPECT_SUCCESS(KeymasterTl_Configure(handle, vendor_patchlevel, params));

    EXPECT_SUCCESS(ImportWrappedKeyTest(handle));

    err = BERR_SUCCESS;

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

    if (err != BERR_SUCCESS) {
        rc = -1;
    }

    if (rc) {
        BDBG_ERR(("Failure in SAGE Keymaster test\n"));
    }

    return rc;
}
