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
#include <stdio.h>

#include "bstd.h"
#include "bkni.h"
#include "nexus_security_client.h"
#include "nexus_memory.h"

#include "keymaster_ids.h"
#include "keymaster_platform.h"
#include "keymaster_tl.h"
#include "keymaster_test.h"
#include "keymaster_key_params.h"
#include "keymaster_keygen.h"
#include "keymaster_crypto_utils.h"


BDBG_MODULE(keymaster_crypto_aes);

/* Test and free the keyslot handle */
#define TEST_FREE_KEYSLOT(hndl)               if (hndl) { NEXUS_Security_FreeKeySlot(hndl); hndl = NULL; }

/* Currently test must use a multiple of 3*4K because SRAI block pointers must be 4K aligned */
#define TEST_BLOCK_SIZE (3 * 4096)

static NEXUS_KeySlotHandle km_crypto_aes_allocate_keyslot(void)
{
    NEXUS_SecurityKeySlotSettings keyslotSettings;
    NEXUS_KeySlotHandle keyslot = NULL;

    NEXUS_Security_GetDefaultKeySlotSettings(&keyslotSettings);
    keyslotSettings.client = NEXUS_SecurityClientType_eSage;
    keyslotSettings.keySlotEngine = NEXUS_SecurityEngine_eM2m;
    keyslot = NEXUS_Security_AllocateKeySlot(&keyslotSettings);
    if (!keyslot) {
        BDBG_ERR(("%s: Error allocating keyslot", BSTD_FUNCTION));
    }
    return keyslot;
}

static BERR_Code km_crypto_aes_encrypt_test(KeymasterTl_Handle handle)
{
    BERR_Code err;
    KM_Tag_ContextHandle key_params = NULL;
    KM_Tag_ContextHandle begin_params = NULL;
    KM_Tag_ContextHandle update_params = NULL;
    KM_CryptoOperation_Settings settings;
    KeymasterTl_DataBlock key = { 0 };
    KeymasterTl_DataBlock in_data = { 0 };
    KeymasterTl_DataBlock intermediate_data = { 0 };
    KeymasterTl_DataBlock final_data = { 0 };
    int i;
    int GcmSize;
    const char *comment;
    uint8_t aad[7] = "foobar";

    BDBG_LOG(("----------------------- %s -----------------------", BSTD_FUNCTION));

    for (i = 0; i < 3; i++) {
        GcmSize = 0;

        KM_CryptoOperation_GetDefaultSettings(&settings);

        settings.handle = handle;

        EXPECT_SUCCESS(km_test_new_params_with_aes_defaults(&key_params, 128));
        EXPECT_SUCCESS(KeymasterTl_GenerateKey(handle, key_params, &key));
        settings.key_params = key_params;
        settings.in_key = key;

        EXPECT_SUCCESS(KM_Tag_NewContext(&begin_params));
        EXPECT_SUCCESS(KM_Tag_NewContext(&update_params));
        settings.begin_params = begin_params;
        settings.update_params = update_params;
        TEST_TAG_ADD_ENUM(settings.begin_params, KM_TAG_PADDING, KM_PAD_NONE);

        switch (i) {
        case 0:
            TEST_TAG_ADD_ENUM(settings.begin_params, KM_TAG_BLOCK_MODE, KM_MODE_ECB);
            comment = "AES ECB mode, no padding";
            break;
        case 1:
            TEST_TAG_ADD_ENUM(settings.begin_params, KM_TAG_BLOCK_MODE, KM_MODE_CBC);
            comment = "AES CBC mode, no padding";
            break;
        case 2:
            TEST_TAG_ADD_ENUM(settings.begin_params, KM_TAG_BLOCK_MODE, KM_MODE_CTR);
            comment = "AES CTR mode";
            break;
        case 3:
            GcmSize = 128;
            TEST_TAG_ADD_ENUM(settings.begin_params, KM_TAG_BLOCK_MODE, KM_MODE_GCM);
            TEST_TAG_ADD_INTEGER(settings.begin_params, KM_TAG_MAC_LENGTH, GcmSize);
            TEST_TAG_ADD_BYTES(settings.update_params, KM_TAG_ASSOCIATED_DATA, sizeof(aad), aad);
            comment = "AES GCM mode";
            break;
        default:
            BDBG_ERR(("%s:%d Internal error", BSTD_FUNCTION, __LINE__));
            err = BERR_UNKNOWN;
            goto done;
        }
        BDBG_ERR(("%s : Test - %s", BSTD_FUNCTION, comment));

        TEST_ALLOCATE_BLOCK(in_data, TEST_BLOCK_SIZE);
        TEST_ALLOCATE_BLOCK(intermediate_data, TEST_BLOCK_SIZE + GcmSize);
        TEST_ALLOCATE_BLOCK(final_data, TEST_BLOCK_SIZE);
        memset(in_data.buffer, 0xC3, in_data.size);
        memset(intermediate_data.buffer, 0xFF, intermediate_data.size);
        memset(final_data.buffer, 0xA1, final_data.size);

        settings.in_data = in_data;
        settings.out_data = intermediate_data;

        EXPECT_SUCCESS(KM_Crypto_Operation(KM_PURPOSE_ENCRYPT, &settings));

        if (!memcmp(in_data.buffer, intermediate_data.buffer, settings.in_data.size)) {
            BDBG_ERR(("%s: intermediate data matches", BSTD_FUNCTION));
            err = BERR_UNKNOWN;
            goto done;
        }

        if (settings.nonce) {
            /* Move the NONCE into the in_params context */
            TEST_TAG_ADD_BYTES(settings.begin_params, KM_TAG_NONCE, settings.nonce->value.blob_data_length, settings.nonce->blob_data);
            NEXUS_Memory_Free(settings.nonce);
            settings.nonce = NULL;
        }

        settings.in_data = settings.out_data;
        settings.out_data = final_data;

        EXPECT_SUCCESS(KM_Crypto_Operation(KM_PURPOSE_DECRYPT, &settings));

        if ((settings.out_data.size != in_data.size) || memcmp(in_data.buffer, settings.out_data.buffer, settings.out_data.size)) {
            BDBG_ERR(("%s: final data does not match", BSTD_FUNCTION));
            err = BERR_UNKNOWN;
            goto done;
        }

        BDBG_LOG(("%s: %s success", BSTD_FUNCTION, comment));

        TEST_FREE_BLOCK(in_data);
        TEST_FREE_BLOCK(intermediate_data);
        TEST_FREE_BLOCK(final_data);
        TEST_FREE_BLOCK(key);
        TEST_DELETE_CONTEXT(begin_params);
        TEST_DELETE_CONTEXT(update_params);
        TEST_DELETE_CONTEXT(key_params);
    }
    err = BERR_SUCCESS;

done:
    TEST_FREE_BLOCK(in_data);
    TEST_FREE_BLOCK(intermediate_data);
    TEST_FREE_BLOCK(final_data);
    TEST_FREE_BLOCK(key);
    TEST_DELETE_CONTEXT(begin_params);
    TEST_DELETE_CONTEXT(update_params);
    TEST_DELETE_CONTEXT(key_params);
    return err;
}

#define AES_GCM_MIN_TAG_LENGTH  (12 * 8)
#define AES_GCM_MAX_TAG_LENGTH  (16 * 8)

static BERR_Code km_crypto_aes_parameter_tests(KeymasterTl_Handle handle)
{
    BERR_Code err;
    KeymasterTl_CryptoBeginSettings beginSettings;
    KM_Tag_ContextHandle key_params = NULL;
    KM_Tag_ContextHandle in_params = NULL;
    KeymasterTl_DataBlock in_key = { 0 };
    km_operation_handle_t op_handle = 0;
    NEXUS_KeySlotHandle keyslot = NULL;
    km_tag_value_t *padding;
    km_tag_value_t *block_mode;
    km_tag_value_t *mac;

    BDBG_LOG(("----------------------- %s -----------------------", BSTD_FUNCTION));

    EXPECT_SUCCESS(km_test_new_params_with_aes_defaults(&key_params, 128));
    EXPECT_SUCCESS(KeymasterTl_GenerateKey(handle, key_params, &in_key));
    keyslot = km_crypto_aes_allocate_keyslot();
    if (!keyslot) {
        err = BERR_OUT_OF_DEVICE_MEMORY;
        goto done;
    }

    EXPECT_SUCCESS(KM_Tag_NewContext(&in_params));
    /* Omit block mode for first test */

    KeymasterTl_GetDefaultCryptoBeginSettings(&beginSettings);
    beginSettings.purpose = KM_PURPOSE_ENCRYPT;
    beginSettings.in_key_blob = in_key;
    beginSettings.hKeyslot = keyslot;
    beginSettings.in_params = in_params;
    EXPECT_FAILURE_CODE(KeymasterTl_CryptoBegin(handle, &beginSettings, &op_handle), BSAGE_ERR_KM_UNSUPPORTED_BLOCK_MODE);

    TEST_TAG_ADD_ENUM(in_params, KM_TAG_BLOCK_MODE, KM_MODE_ECB);
    /* Test with default padding NONE, which should succeed */
    EXPECT_SUCCESS(KeymasterTl_CryptoBegin(handle, &beginSettings, &op_handle));
    TEST_DELETE_CONTEXT(beginSettings.out_params);
    EXPECT_SUCCESS(KeymasterTl_CryptoAbort(handle, op_handle));
    op_handle = 0;
    TEST_TAG_ADD_ENUM(in_params, KM_TAG_PADDING, KM_PAD_RSA_OAEP);
    EXPECT_FAILURE_CODE(KeymasterTl_CryptoBegin(handle, &beginSettings, &op_handle), BSAGE_ERR_KM_UNSUPPORTED_PADDING_MODE);

    /* Change the padding mode to test ECB combinations */
    padding = KM_Tag_FindFirst(in_params, KM_TAG_PADDING);
    BDBG_ASSERT(padding);
    padding->value.enumerated = (uint32_t)KM_PAD_RSA_PKCS1_1_5_ENCRYPT;
    EXPECT_FAILURE_CODE(KeymasterTl_CryptoBegin(handle, &beginSettings, &op_handle), BSAGE_ERR_KM_UNSUPPORTED_PADDING_MODE);
    padding->value.enumerated = (uint32_t)KM_PAD_RSA_PKCS1_1_5_SIGN;
    EXPECT_FAILURE_CODE(KeymasterTl_CryptoBegin(handle, &beginSettings, &op_handle), BSAGE_ERR_KM_UNSUPPORTED_PADDING_MODE);
    padding->value.enumerated = (uint32_t)KM_PAD_NONE;
    beginSettings.hKeyslot = NULL;
    EXPECT_FAILURE_CODE(KeymasterTl_CryptoBegin(handle, &beginSettings, &op_handle), BERR_INVALID_PARAMETER);

    /* Change the block mode to check CBC padding combinations */
    block_mode = KM_Tag_FindFirst(in_params, KM_TAG_BLOCK_MODE);
    BDBG_ASSERT(block_mode);
    block_mode->value.enumerated = (uint32_t)KM_MODE_CBC;
    padding->value.enumerated = (uint32_t)KM_PAD_RSA_OAEP;
    beginSettings.hKeyslot = keyslot;
    EXPECT_FAILURE_CODE(KeymasterTl_CryptoBegin(handle, &beginSettings, &op_handle), BSAGE_ERR_KM_UNSUPPORTED_PADDING_MODE);
    padding->value.enumerated = (uint32_t)KM_PAD_RSA_PKCS1_1_5_ENCRYPT;
    EXPECT_FAILURE_CODE(KeymasterTl_CryptoBegin(handle, &beginSettings, &op_handle), BSAGE_ERR_KM_UNSUPPORTED_PADDING_MODE);
    padding->value.enumerated = (uint32_t)KM_PAD_RSA_PKCS1_1_5_SIGN;
    EXPECT_FAILURE_CODE(KeymasterTl_CryptoBegin(handle, &beginSettings, &op_handle), BSAGE_ERR_KM_UNSUPPORTED_PADDING_MODE);
    padding->value.enumerated = (uint32_t)KM_PAD_NONE;
    beginSettings.hKeyslot = NULL;
    EXPECT_FAILURE_CODE(KeymasterTl_CryptoBegin(handle, &beginSettings, &op_handle), BERR_INVALID_PARAMETER);

    /* Check CTR mode padding combinations */
    block_mode->value.enumerated = (uint32_t)KM_MODE_CTR;
    padding->value.enumerated = (uint32_t)KM_PAD_PKCS7;
    EXPECT_FAILURE_CODE(KeymasterTl_CryptoBegin(handle, &beginSettings, &op_handle), BSAGE_ERR_KM_UNSUPPORTED_PADDING_MODE);
    padding->value.enumerated = (uint32_t)KM_PAD_RSA_OAEP;
    EXPECT_FAILURE_CODE(KeymasterTl_CryptoBegin(handle, &beginSettings, &op_handle), BSAGE_ERR_KM_UNSUPPORTED_PADDING_MODE);
    padding->value.enumerated = (uint32_t)KM_PAD_RSA_PKCS1_1_5_ENCRYPT;
    EXPECT_FAILURE_CODE(KeymasterTl_CryptoBegin(handle, &beginSettings, &op_handle), BSAGE_ERR_KM_UNSUPPORTED_PADDING_MODE);
    padding->value.enumerated = (uint32_t)KM_PAD_RSA_PKCS1_1_5_SIGN;
    EXPECT_FAILURE_CODE(KeymasterTl_CryptoBegin(handle, &beginSettings, &op_handle), BSAGE_ERR_KM_UNSUPPORTED_PADDING_MODE);
    padding->value.enumerated = (uint32_t)KM_PAD_NONE;
    beginSettings.hKeyslot = NULL;
    EXPECT_FAILURE_CODE(KeymasterTl_CryptoBegin(handle, &beginSettings, &op_handle), BERR_INVALID_PARAMETER);

    /* Check GCM mode padding combinations */
    block_mode->value.enumerated = (uint32_t)KM_MODE_GCM;
    padding->value.enumerated = (uint32_t)KM_PAD_PKCS7;
    EXPECT_FAILURE_CODE(KeymasterTl_CryptoBegin(handle, &beginSettings, &op_handle), BSAGE_ERR_KM_UNSUPPORTED_PADDING_MODE);
    padding->value.enumerated = (uint32_t)KM_PAD_RSA_OAEP;
    EXPECT_FAILURE_CODE(KeymasterTl_CryptoBegin(handle, &beginSettings, &op_handle), BSAGE_ERR_KM_UNSUPPORTED_PADDING_MODE);
    padding->value.enumerated = (uint32_t)KM_PAD_RSA_PKCS1_1_5_ENCRYPT;
    EXPECT_FAILURE_CODE(KeymasterTl_CryptoBegin(handle, &beginSettings, &op_handle), BSAGE_ERR_KM_UNSUPPORTED_PADDING_MODE);
    padding->value.enumerated = (uint32_t)KM_PAD_RSA_PKCS1_1_5_SIGN;
    EXPECT_FAILURE_CODE(KeymasterTl_CryptoBegin(handle, &beginSettings, &op_handle), BSAGE_ERR_KM_UNSUPPORTED_PADDING_MODE);

    /* Check GCM TAG length */
    padding->value.enumerated = (uint32_t)KM_PAD_NONE;
    EXPECT_FAILURE_CODE(KeymasterTl_CryptoBegin(handle, &beginSettings, &op_handle), BSAGE_ERR_KM_MISSING_MAC_LENGTH);
    TEST_TAG_ADD_INTEGER(in_params, KM_TAG_MAC_LENGTH, AES_GCM_MIN_TAG_LENGTH - 8);
    EXPECT_FAILURE_CODE(KeymasterTl_CryptoBegin(handle, &beginSettings, &op_handle), BSAGE_ERR_KM_INVALID_MAC_LENGTH);
    mac = KM_Tag_FindFirst(in_params, KM_TAG_MAC_LENGTH);
    BDBG_ASSERT(mac);
    mac->value.integer = AES_GCM_MAX_TAG_LENGTH + 8;
    EXPECT_FAILURE_CODE(KeymasterTl_CryptoBegin(handle, &beginSettings, &op_handle), BSAGE_ERR_KM_INVALID_MAC_LENGTH);

    BDBG_LOG(("%s: success", BSTD_FUNCTION));
    err = BERR_SUCCESS;

done:
    if (op_handle) {
        /* If crypto finish fails, this will throw an error too */
        (void)KeymasterTl_CryptoAbort(handle, op_handle);
    }
    TEST_FREE_KEYSLOT(keyslot);
    TEST_FREE_BLOCK(in_key);
    TEST_DELETE_CONTEXT(key_params);
    TEST_DELETE_CONTEXT(in_params);
    return err;
}

BERR_Code km_crypto_aes_tests(KeymasterTl_Handle handle)
{
    BERR_Code err;

    EXPECT_SUCCESS(km_crypto_aes_encrypt_test(handle));
    EXPECT_SUCCESS(km_crypto_aes_parameter_tests(handle));

done:
    return err;
}
