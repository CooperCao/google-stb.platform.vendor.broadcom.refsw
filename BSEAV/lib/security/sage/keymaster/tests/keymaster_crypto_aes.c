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

#include "keymaster_debug.h"
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

#define TEST_BLOCK_SIZE (3 * 4096)

#define NUM_VECTOR_TESTS 3

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
    KeymasterTl_DataBlock key;
    KeymasterTl_DataBlock in_data;
    KeymasterTl_DataBlock intermediate_data;
    KeymasterTl_DataBlock final_data;
    int i;
    int GcmSize;
    int extra_space;
    const char *comment;
    uint32_t key_size;
    uint8_t aad[7] = "foobar";

    memset(&key, 0, sizeof(key));
    memset(&in_data, 0, sizeof(in_data));
    memset(&intermediate_data, 0, sizeof(intermediate_data));
    memset(&final_data, 0, sizeof(final_data));

    BDBG_LOG(("----------------------- %s -----------------------", BSTD_FUNCTION));

    /* For speed, it is recommended that a single block of data is provided at a time, for enc/dec */
    /* The optimal operation is CryptoBegin and CryptoFinish, with data provided to CryptoFinish */
    for (i = 0; i < 18; i++) {
        extra_space = 0;

        KM_CryptoOperation_GetDefaultSettings(&settings);

        settings.handle = handle;

        EXPECT_SUCCESS(KM_Tag_NewContext(&begin_params));
        EXPECT_SUCCESS(KM_Tag_NewContext(&update_params));
        settings.begin_params = begin_params;
        settings.update_params = update_params;

        switch (i) {
        case 0:
            key_size = 128;
            extra_space = key_size / 8;
            TEST_TAG_ADD_ENUM(settings.begin_params, KM_TAG_PADDING, KM_PAD_NONE);
            TEST_TAG_ADD_ENUM(settings.begin_params, KM_TAG_BLOCK_MODE, KM_MODE_ECB);
            comment = "HW AES 128 ECB mode, no padding";
            break;
        case 1:
            key_size = 128;
            /* OpenSSL requires extra space for enc/dec, so all non-GCM AES modes have been given extra space */
            extra_space = key_size / 8;
            TEST_TAG_ADD_ENUM(settings.begin_params, KM_TAG_PADDING, KM_PAD_PKCS7);
            TEST_TAG_ADD_ENUM(settings.begin_params, KM_TAG_BLOCK_MODE, KM_MODE_ECB);
            comment = "AES 128 ECB mode, PKCS7";
            break;
        case 2:
            key_size = 192;
            extra_space = key_size / 8;
            TEST_TAG_ADD_ENUM(settings.begin_params, KM_TAG_PADDING, KM_PAD_NONE);
            TEST_TAG_ADD_ENUM(settings.begin_params, KM_TAG_BLOCK_MODE, KM_MODE_ECB);
            comment = "AES 192 ECB mode, no padding";
            break;
        case 3:
            key_size = 192;
            extra_space = key_size / 8;
            TEST_TAG_ADD_ENUM(settings.begin_params, KM_TAG_PADDING, KM_PAD_PKCS7);
            TEST_TAG_ADD_ENUM(settings.begin_params, KM_TAG_BLOCK_MODE, KM_MODE_ECB);
            comment = "AES 192 ECB mode, PKCS7";
            break;
        case 4:
            key_size = 256;
            extra_space = key_size / 8;
            TEST_TAG_ADD_ENUM(settings.begin_params, KM_TAG_PADDING, KM_PAD_NONE);
            TEST_TAG_ADD_ENUM(settings.begin_params, KM_TAG_BLOCK_MODE, KM_MODE_ECB);
            comment = "AES 256 ECB mode, no padding";
            break;
        case 5:
            key_size = 256;
            extra_space = key_size / 8;
            TEST_TAG_ADD_ENUM(settings.begin_params, KM_TAG_PADDING, KM_PAD_PKCS7);
            TEST_TAG_ADD_ENUM(settings.begin_params, KM_TAG_BLOCK_MODE, KM_MODE_ECB);
            comment = "AES 256 ECB mode, PKCS7";
            break;
        case 6:
            key_size = 128;
            extra_space = key_size / 8;
            TEST_TAG_ADD_ENUM(settings.begin_params, KM_TAG_PADDING, KM_PAD_NONE);
            TEST_TAG_ADD_ENUM(settings.begin_params, KM_TAG_BLOCK_MODE, KM_MODE_CBC);
            comment = "HW AES 128 CBC mode, no padding";
            break;
        case 7:
            key_size = 128;
            extra_space = key_size / 8;
            TEST_TAG_ADD_ENUM(settings.begin_params, KM_TAG_PADDING, KM_PAD_PKCS7);
            TEST_TAG_ADD_ENUM(settings.begin_params, KM_TAG_BLOCK_MODE, KM_MODE_CBC);
            comment = "AES 128 CBC mode, PKCS7";
            break;
        case 8:
            key_size = 192;
            extra_space = key_size / 8;
            TEST_TAG_ADD_ENUM(settings.begin_params, KM_TAG_PADDING, KM_PAD_NONE);
            TEST_TAG_ADD_ENUM(settings.begin_params, KM_TAG_BLOCK_MODE, KM_MODE_CBC);
            comment = "AES 192 CBC mode, no padding";
            break;
        case 9:
            key_size = 192;
            extra_space = key_size / 8;
            TEST_TAG_ADD_ENUM(settings.begin_params, KM_TAG_PADDING, KM_PAD_PKCS7);
            TEST_TAG_ADD_ENUM(settings.begin_params, KM_TAG_BLOCK_MODE, KM_MODE_CBC);
            comment = "AES 192 CBC mode, PKCS7";
            break;
        case 10:
            key_size = 256;
            extra_space = key_size / 8;
            TEST_TAG_ADD_ENUM(settings.begin_params, KM_TAG_PADDING, KM_PAD_NONE);
            TEST_TAG_ADD_ENUM(settings.begin_params, KM_TAG_BLOCK_MODE, KM_MODE_CBC);
            comment = "AES 256 CBC mode, no padding";
            break;
        case 11:
            key_size = 256;
            extra_space = key_size / 8;
            TEST_TAG_ADD_ENUM(settings.begin_params, KM_TAG_PADDING, KM_PAD_PKCS7);
            TEST_TAG_ADD_ENUM(settings.begin_params, KM_TAG_BLOCK_MODE, KM_MODE_CBC);
            comment = "AES 256 CBC mode, PKCS7";
            break;
        case 12:
            key_size = 128;
            extra_space = key_size / 8;
            TEST_TAG_ADD_ENUM(settings.begin_params, KM_TAG_PADDING, KM_PAD_NONE);
            TEST_TAG_ADD_ENUM(settings.begin_params, KM_TAG_BLOCK_MODE, KM_MODE_CTR);
            comment = "HW AES 128 CTR mode";
            break;
        case 13:
            key_size = 192;
            extra_space = key_size / 8;
            TEST_TAG_ADD_ENUM(settings.begin_params, KM_TAG_PADDING, KM_PAD_NONE);
            TEST_TAG_ADD_ENUM(settings.begin_params, KM_TAG_BLOCK_MODE, KM_MODE_CTR);
            comment = "AES 192 CTR mode";
            break;
        case 14:
            key_size = 256;
            extra_space = key_size / 8;
            TEST_TAG_ADD_ENUM(settings.begin_params, KM_TAG_PADDING, KM_PAD_NONE);
            TEST_TAG_ADD_ENUM(settings.begin_params, KM_TAG_BLOCK_MODE, KM_MODE_CTR);
            comment = "AES 256 CTR mode";
            break;
        case 15: /* GCM modes are currently disabled by the for loop */
            key_size = 128;
            GcmSize = 128;
            /* GCM modes need the extra space for the appended GCM TAG */
            extra_space = key_size / 8;
            TEST_TAG_ADD_ENUM(settings.begin_params, KM_TAG_PADDING, KM_PAD_NONE);
            TEST_TAG_ADD_ENUM(settings.begin_params, KM_TAG_BLOCK_MODE, KM_MODE_GCM);
            TEST_TAG_ADD_INTEGER(settings.begin_params, KM_TAG_MAC_LENGTH, GcmSize);
            TEST_TAG_ADD_BYTES(settings.update_params, KM_TAG_ASSOCIATED_DATA, sizeof(aad), aad);
            comment = "AES 128 GCM mode";
            break;
        case 16:
            key_size = 192;
            GcmSize = 128;
            extra_space = key_size / 8;
            TEST_TAG_ADD_ENUM(settings.begin_params, KM_TAG_PADDING, KM_PAD_NONE);
            TEST_TAG_ADD_ENUM(settings.begin_params, KM_TAG_BLOCK_MODE, KM_MODE_GCM);
            TEST_TAG_ADD_INTEGER(settings.begin_params, KM_TAG_MAC_LENGTH, GcmSize);
            TEST_TAG_ADD_BYTES(settings.update_params, KM_TAG_ASSOCIATED_DATA, sizeof(aad), aad);
            comment = "AES 192 GCM mode";
            break;
        case 17:
            key_size = 256;
            GcmSize = 128;
            extra_space = key_size / 8;
            TEST_TAG_ADD_ENUM(settings.begin_params, KM_TAG_PADDING, KM_PAD_NONE);
            TEST_TAG_ADD_ENUM(settings.begin_params, KM_TAG_BLOCK_MODE, KM_MODE_GCM);
            TEST_TAG_ADD_INTEGER(settings.begin_params, KM_TAG_MAC_LENGTH, GcmSize);
            TEST_TAG_ADD_BYTES(settings.update_params, KM_TAG_ASSOCIATED_DATA, sizeof(aad), aad);
            comment = "AES 256 GCM mode";
            break;
        default:
            BDBG_ERR(("%s:%d Internal error", BSTD_FUNCTION, __LINE__));
            err = BERR_UNKNOWN;
            goto done;
        }

        EXPECT_SUCCESS(km_test_new_params_with_aes_defaults(&key_params, key_size));
        EXPECT_SUCCESS(KeymasterTl_GenerateKey(handle, key_params, &key));
        settings.key_params = key_params;
        settings.in_key = key;

        BDBG_LOG(("%s : Test - %s", BSTD_FUNCTION, comment));

        TEST_ALLOCATE_BLOCK(in_data, TEST_BLOCK_SIZE);
        TEST_ALLOCATE_BLOCK(intermediate_data, TEST_BLOCK_SIZE + extra_space);
        TEST_ALLOCATE_BLOCK(final_data, TEST_BLOCK_SIZE + extra_space);
        memset(in_data.buffer, 0xC3, in_data.size);
        memset(intermediate_data.buffer, 0xFF, intermediate_data.size);
        memset(final_data.buffer, 0xA1, final_data.size);

        settings.in_data = in_data;
        settings.out_data = intermediate_data;

        EXPECT_SUCCESS(KM_Crypto_Operation(KM_PURPOSE_ENCRYPT, &settings));

        if (!memcmp(in_data.buffer, intermediate_data.buffer, settings.out_data.size)) {
            BDBG_ERR(("%s: intermediate data matches", BSTD_FUNCTION));
            err = BERR_UNKNOWN;
            goto done;
        }

        if (settings.out_nonce) {
            /* Move the NONCE into the in_params context */
            TEST_TAG_ADD_BYTES(settings.begin_params, KM_TAG_NONCE, settings.out_nonce->value.blob_data_length, settings.out_nonce->blob_data);
            NEXUS_Memory_Free(settings.out_nonce);
            settings.out_nonce = NULL;
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
    KeymasterTl_DataBlock in_key;
    km_operation_handle_t op_handle = 0;
    NEXUS_KeySlotHandle keyslot = NULL;
    km_tag_value_t *padding;
    km_tag_value_t *block_mode;
    km_tag_value_t *mac;

    memset(&in_key, 0, sizeof(in_key));

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

BERR_Code km_crypto_encrypt_vector_test(KeymasterTl_Handle handle, KeymasterTl_DataBlock in_key, KeymasterTl_DataBlock nonce,
                                        KeymasterTl_DataBlock message, KeymasterTl_DataBlock expected)
{
    BERR_Code err;
    KM_Tag_ContextHandle begin_params = NULL;
    KM_CryptoOperation_Settings settings;
    KeymasterTl_DataBlock out_data;
    KeymasterTl_ImportKeySettings impSettings;

    memset(&out_data, 0, sizeof(out_data));

    BDBG_LOG(("----------------------- %s -----------------------", BSTD_FUNCTION));

    KM_CryptoOperation_GetDefaultSettings(&settings);

    settings.handle = handle;

    KeymasterTl_GetDefaultImportKeySettings(&impSettings);
    EXPECT_SUCCESS(km_test_new_params_with_aes_defaults(&impSettings.in_key_params, in_key.size * 8));
    impSettings.in_key_format = KM_KEY_FORMAT_RAW;
    impSettings.in_key_blob = in_key;
    EXPECT_SUCCESS(KeymasterTl_ImportKey(handle, &impSettings));
    settings.in_key = impSettings.out_key_blob;

    EXPECT_SUCCESS(KM_Tag_NewContext(&begin_params));
    settings.begin_params = begin_params;

    TEST_TAG_ADD_ENUM(settings.begin_params, KM_TAG_PADDING, KM_PAD_NONE);
    TEST_TAG_ADD_ENUM(settings.begin_params, KM_TAG_BLOCK_MODE, KM_MODE_CTR);
    TEST_TAG_ADD_BYTES(settings.begin_params, KM_TAG_NONCE, nonce.size, nonce.buffer);

    TEST_ALLOCATE_BLOCK(out_data, expected.size + in_key.size);
    memset(out_data.buffer, 0xFF, out_data.size);

    settings.in_data = message;
    settings.out_data = out_data;
    settings.key_params = impSettings.in_key_params;

    EXPECT_SUCCESS(KM_Crypto_Operation(KM_PURPOSE_ENCRYPT, &settings));

    if (memcmp(out_data.buffer, expected.buffer, expected.size) != 0) {
        BDBG_ERR(("%s: Out data and expected buffers don't match ", BSTD_FUNCTION));
        err = BERR_UNKNOWN;
        goto done;
    }

    BDBG_LOG(("%s: success", BSTD_FUNCTION));

    TEST_FREE_BLOCK(out_data);
    TEST_DELETE_CONTEXT(begin_params);

    err = BERR_SUCCESS;

done:
    TEST_FREE_BLOCK(out_data);
    TEST_DELETE_CONTEXT(begin_params);
    return err;
}

static BERR_Code km_crypto_aes_vectors(KeymasterTl_Handle handle)
{
    BERR_Code err = BERR_UNKNOWN;
    int i;
    KeymasterTl_DataBlock key_data;
    KeymasterTl_DataBlock nonce_data;
    KeymasterTl_DataBlock message_data;
    KeymasterTl_DataBlock expected_data;

    BDBG_LOG(("----------------------- %s -----------------------", BSTD_FUNCTION));
    uint8_t nonce[] = {
        0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff,
    };
    uint8_t message[] = {
        0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96, 0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a,
        0xae, 0x2d, 0x8a, 0x57, 0x1e, 0x03, 0xac, 0x9c, 0x9e, 0xb7, 0x6f, 0xac, 0x45, 0xaf, 0x8e, 0x51,
        0x30, 0xc8, 0x1c, 0x46, 0xa3, 0x5c, 0xe4, 0x11, 0xe5, 0xfb, 0xc1, 0x19, 0x1a, 0x0a, 0x52, 0xef,
        0xf6, 0x9f, 0x24, 0x45, 0xdf, 0x4f, 0x9b, 0x17, 0xad, 0x2b, 0x41, 0x7b, 0xe6, 0x6c, 0x37, 0x10,
    };

    TEST_ALLOCATE_BLOCK(nonce_data, sizeof(nonce));
    memcpy(nonce_data.buffer, nonce, nonce_data.size);

    TEST_ALLOCATE_BLOCK(message_data, sizeof(message));
    memcpy(message_data.buffer, message, message_data.size);
    for (i = 0; i < NUM_VECTOR_TESTS; i++) {

        switch (i) {
        case 0:
            {
                BDBG_LOG(("----------------------- AES Ctr VectorTestCase 128 -----------------------"));
                uint8_t key[] = {
                    0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c,
                };
                uint8_t expected[] = {
                    0x87, 0x4d, 0x61, 0x91, 0xb6, 0x20, 0xe3, 0x26, 0x1b, 0xef, 0x68, 0x64, 0x99, 0x0d, 0xb6, 0xce,
                    0x98, 0x06, 0xf6, 0x6b, 0x79, 0x70, 0xfd, 0xff, 0x86, 0x17, 0x18, 0x7b, 0xb9, 0xff, 0xfd, 0xff,
                    0x5a, 0xe4, 0xdf, 0x3e, 0xdb, 0xd5, 0xd3, 0x5e, 0x5b, 0x4f, 0x09, 0x02, 0x0d, 0xb0, 0x3e, 0xab,
                    0x1e, 0x03, 0x1d, 0xda, 0x2f, 0xbe, 0x03, 0xd1, 0x79, 0x21, 0x70, 0xa0, 0xf3, 0x00, 0x9c, 0xee,
                };

                TEST_ALLOCATE_BLOCK(key_data, sizeof(key));
                memcpy(key_data.buffer, key, key_data.size);

                TEST_ALLOCATE_BLOCK(expected_data, sizeof(expected));
                memcpy(expected_data.buffer, expected, expected_data.size);
                break;
            }
        case 1:
            {
                BDBG_LOG(("----------------------- AES Ctr VectorTestCase 192 -----------------------"));
                uint8_t key[] = {
                    0x8e, 0x73, 0xb0, 0xf7, 0xda, 0x0e, 0x64, 0x52, 0xc8, 0x10, 0xf3, 0x2b, 0x80, 0x90, 0x79, 0xe5,
                    0x62, 0xf8, 0xea, 0xd2, 0x52, 0x2c, 0x6b, 0x7b,
                };
                uint8_t expected[] = {
                    0x1a, 0xbc, 0x93, 0x24, 0x17, 0x52, 0x1c, 0xa2, 0x4f, 0x2b, 0x04, 0x59, 0xfe, 0x7e, 0x6e, 0x0b,
                    0x09, 0x03, 0x39, 0xec, 0x0a, 0xa6, 0xfa, 0xef, 0xd5, 0xcc, 0xc2, 0xc6, 0xf4, 0xce, 0x8e, 0x94,
                    0x1e, 0x36, 0xb2, 0x6b, 0xd1, 0xeb, 0xc6, 0x70, 0xd1, 0xbd, 0x1d, 0x66, 0x56, 0x20, 0xab, 0xf7,
                    0x4f, 0x78, 0xa7, 0xf6, 0xd2, 0x98, 0x09, 0x58, 0x5a, 0x97, 0xda, 0xec, 0x58, 0xc6, 0xb0, 0x50,
                };

                TEST_ALLOCATE_BLOCK(key_data, sizeof(key));
                memcpy(key_data.buffer, key, key_data.size);

                TEST_ALLOCATE_BLOCK(expected_data, sizeof(expected));
                memcpy(expected_data.buffer, expected, expected_data.size);
                break;
            }
        case 2:
            {
                BDBG_LOG(("----------------------- AES Ctr VectorTestCase 256 -----------------------"));
                uint8_t key[] = {
                    0x60, 0x3d, 0xeb, 0x10, 0x15, 0xca, 0x71, 0xbe, 0x2b, 0x73, 0xae, 0xf0, 0x85, 0x7d, 0x77, 0x81,
                    0x1f, 0x35, 0x2c, 0x07, 0x3b, 0x61, 0x08, 0xd7, 0x2d, 0x98, 0x10, 0xa3, 0x09, 0x14, 0xdf, 0xf4,
                };
                uint8_t expected[] = {
                    0x60, 0x1e, 0xc3, 0x13, 0x77, 0x57, 0x89, 0xa5, 0xb7, 0xa7, 0xf5, 0x04, 0xbb, 0xf3, 0xd2, 0x28,
                    0xf4, 0x43, 0xe3, 0xca, 0x4d, 0x62, 0xb5, 0x9a, 0xca, 0x84, 0xe9, 0x90, 0xca, 0xca, 0xf5, 0xc5,
                    0x2b, 0x09, 0x30, 0xda, 0xa2, 0x3d, 0xe9, 0x4c, 0xe8, 0x70, 0x17, 0xba, 0x2d, 0x84, 0x98, 0x8d,
                    0xdf, 0xc9, 0xc5, 0x8d, 0xb6, 0x7a, 0xad, 0xa6, 0x13, 0xc2, 0xdd, 0x08, 0x45, 0x79, 0x41, 0xa6,
                };

                TEST_ALLOCATE_BLOCK(key_data, sizeof(key));
                memcpy(key_data.buffer, key, key_data.size);

                TEST_ALLOCATE_BLOCK(expected_data, sizeof(expected));
                memcpy(expected_data.buffer, expected, expected_data.size);
                break;
            }
        default:
            BDBG_ERR(("%s:%d Internal error", BSTD_FUNCTION, __LINE__));
            err = BERR_UNKNOWN;
            goto done;
        }
        EXPECT_SUCCESS(km_crypto_encrypt_vector_test(handle, key_data, nonce_data, message_data, expected_data));
        TEST_FREE_BLOCK(expected_data);
        TEST_FREE_BLOCK(key_data);
    }
    err = BERR_SUCCESS;
    TEST_FREE_BLOCK(nonce_data);
    TEST_FREE_BLOCK(message_data);
done:
    TEST_FREE_BLOCK(expected_data);
    TEST_FREE_BLOCK(key_data);
    TEST_FREE_BLOCK(message_data);
    TEST_FREE_BLOCK(nonce_data);
    return err;
}

/* A standard set of tests used in multiple tests below */
static BERR_Code km_crypto_aes_select_test(int test, uint32_t *key_size, KM_Tag_ContextHandle *begin_params, KM_Tag_ContextHandle *update_params, const char **comment)
{
    BERR_Code err = BERR_UNKNOWN;
    int GcmSize = 0;
    uint8_t aad[7] = "foobar";

    *begin_params = NULL;
    *update_params = NULL;

    EXPECT_SUCCESS(KM_Tag_NewContext(begin_params));
    EXPECT_SUCCESS(KM_Tag_NewContext(update_params));

    switch (test) {
    case 0:
        *key_size = 128;
        TEST_TAG_ADD_ENUM(*begin_params, KM_TAG_PADDING, KM_PAD_NONE);
        TEST_TAG_ADD_ENUM(*begin_params, KM_TAG_BLOCK_MODE, KM_MODE_ECB);
        *comment = "HW AES 128 ECB mode, no padding";
        break;
    case 1:
        *key_size = 128;
        TEST_TAG_ADD_ENUM(*begin_params, KM_TAG_PADDING, KM_PAD_PKCS7);
        TEST_TAG_ADD_ENUM(*begin_params, KM_TAG_BLOCK_MODE, KM_MODE_ECB);
        *comment = "AES 128 ECB mode, PKCS7";
        break;
    case 2:
        *key_size = 128;
        TEST_TAG_ADD_ENUM(*begin_params, KM_TAG_PADDING, KM_PAD_NONE);
        TEST_TAG_ADD_ENUM(*begin_params, KM_TAG_BLOCK_MODE, KM_MODE_CBC);
        *comment = "HW AES 128 CBC mode, no padding";
        break;
    case 3:
        *key_size = 128;
        TEST_TAG_ADD_ENUM(*begin_params, KM_TAG_PADDING, KM_PAD_PKCS7);
        TEST_TAG_ADD_ENUM(*begin_params, KM_TAG_BLOCK_MODE, KM_MODE_CBC);
        *comment = "AES 128 CBC mode, PKCS7";
        break;
    case 4:
        *key_size = 128;
        TEST_TAG_ADD_ENUM(*begin_params, KM_TAG_PADDING, KM_PAD_NONE);
        TEST_TAG_ADD_ENUM(*begin_params, KM_TAG_BLOCK_MODE, KM_MODE_CTR);
        *comment = "HW AES 128 CTR mode";
        break;
    case 5:
        *key_size = 192;
        TEST_TAG_ADD_ENUM(*begin_params, KM_TAG_PADDING, KM_PAD_NONE);
        TEST_TAG_ADD_ENUM(*begin_params, KM_TAG_BLOCK_MODE, KM_MODE_CTR);
        *comment = "AES 192 CTR mode";
        break;
    case 6:
        *key_size = 128;
        GcmSize = 128;
        TEST_TAG_ADD_ENUM(*begin_params, KM_TAG_PADDING, KM_PAD_NONE);
        TEST_TAG_ADD_ENUM(*begin_params, KM_TAG_BLOCK_MODE, KM_MODE_GCM);
        TEST_TAG_ADD_INTEGER(*begin_params, KM_TAG_MAC_LENGTH, GcmSize);
        TEST_TAG_ADD_BYTES(*update_params, KM_TAG_ASSOCIATED_DATA, sizeof(aad), aad);
        *comment = "AES 128 GCM mode";
        break;
    default:
        BDBG_ERR(("%s:%d Internal error", BSTD_FUNCTION, __LINE__));
        err = BERR_UNKNOWN;
        goto done;
    }

done:
    if (err != BERR_SUCCESS) {
        TEST_DELETE_CONTEXT(*begin_params);
        TEST_DELETE_CONTEXT(*update_params);
    }
    return err;
}

#define MIN_SECONDS_BETWEEN_OPS     4

static BERR_Code km_crypto_aes_seconds_between_ops(KeymasterTl_Handle handle)
{
    BERR_Code err;
    KM_Tag_ContextHandle key_params = NULL;
    KM_Tag_ContextHandle begin_params = NULL;
    KM_Tag_ContextHandle update_params = NULL;
    KM_CryptoOperation_Settings settings;
    KeymasterTl_DataBlock key;
    KeymasterTl_DataBlock in_data;
    KeymasterTl_DataBlock intermediate_data;
    KeymasterTl_DataBlock final_data;
    int i, mode;
    const char *comment;
    uint32_t key_size;

    memset(&key, 0, sizeof(key));
    memset(&in_data, 0, sizeof(in_data));
    memset(&intermediate_data, 0, sizeof(intermediate_data));
    memset(&final_data, 0, sizeof(final_data));

    BDBG_LOG(("----------------------- %s -----------------------", BSTD_FUNCTION));

    for (i = 0; i < 7; i++) {
        EXPECT_SUCCESS(km_crypto_aes_select_test(i, &key_size, &begin_params, &update_params, &comment));

        EXPECT_SUCCESS(km_test_new_params_with_aes_defaults(&key_params, key_size));
        TEST_TAG_ADD_INTEGER(key_params, KM_TAG_MIN_SECONDS_BETWEEN_OPS, MIN_SECONDS_BETWEEN_OPS);
        EXPECT_SUCCESS(KeymasterTl_GenerateKey(handle, key_params, &key));

        for (mode = 0; mode < 2; mode++) {
            KM_CryptoOperation_GetDefaultSettings(&settings);
            settings.handle = handle;
            settings.key_params = key_params;
            settings.in_key = key;
            settings.begin_params = begin_params;
            settings.update_params = update_params;

            TEST_ALLOCATE_BLOCK(in_data, TEST_BLOCK_SIZE);
            TEST_ALLOCATE_BLOCK(intermediate_data, TEST_BLOCK_SIZE + key_size / 8);
            TEST_ALLOCATE_BLOCK(final_data, TEST_BLOCK_SIZE + key_size / 8);
            memset(in_data.buffer, 0xC3, in_data.size);
            memset(intermediate_data.buffer, 0xFF, intermediate_data.size);
            memset(final_data.buffer, 0xA1, final_data.size);

            settings.in_data = in_data;
            settings.out_data = intermediate_data;

            BDBG_LOG(("%s : Test - %s", BSTD_FUNCTION, comment));

            EXPECT_SUCCESS(KM_Crypto_Operation(KM_PURPOSE_ENCRYPT, &settings));

            if (!memcmp(in_data.buffer, intermediate_data.buffer, settings.out_data.size)) {
                BDBG_ERR(("%s: intermediate data matches", BSTD_FUNCTION));
                err = BERR_UNKNOWN;
                goto done;
            }

            if (settings.out_nonce) {
                /* Move the NONCE into the in_params context */
                TEST_TAG_ADD_BYTES(begin_params, KM_TAG_NONCE, settings.out_nonce->value.blob_data_length, settings.out_nonce->blob_data);
                NEXUS_Memory_Free(settings.out_nonce);
                settings.out_nonce = NULL;
            }

            settings.in_data = settings.out_data;
            settings.out_data = final_data;

            if (mode == 0) {
                BDBG_LOG(("    sleeping %d seconds before calling decrypt", MIN_SECONDS_BETWEEN_OPS));
                sleep(MIN_SECONDS_BETWEEN_OPS);
                EXPECT_SUCCESS(KM_Crypto_Operation(KM_PURPOSE_DECRYPT, &settings));

                if ((settings.out_data.size != in_data.size) || memcmp(in_data.buffer, settings.out_data.buffer, settings.out_data.size)) {
                    BDBG_ERR(("%s: final data does not match", BSTD_FUNCTION));
                    err = BERR_UNKNOWN;
                    goto done;
                }
            } else {
                BDBG_LOG(("    sleeping %d seconds before calling decrypt", MIN_SECONDS_BETWEEN_OPS / 2));
                sleep(MIN_SECONDS_BETWEEN_OPS / 2);
                EXPECT_FAILURE_CODE(KM_Crypto_Operation(KM_PURPOSE_DECRYPT, &settings), BSAGE_ERR_KM_TOO_MANY_OPERATIONS);
            }

            BDBG_LOG(("%s: %s success", BSTD_FUNCTION, comment));

            TEST_FREE_BLOCK(in_data);
            TEST_FREE_BLOCK(intermediate_data);
            TEST_FREE_BLOCK(final_data);

            if (!mode) {
                BDBG_LOG(("    sleeping %d seconds before looping", MIN_SECONDS_BETWEEN_OPS));
                sleep(MIN_SECONDS_BETWEEN_OPS);
            }
        }

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

/* Make this an odd number so the loop below fails on the decrypt */
#define MAX_USES_PER_BOOT   5

static BERR_Code km_crypto_aes_max_uses_per_boot(KeymasterTl_Handle handle)
{
    BERR_Code err;
    KM_Tag_ContextHandle key_params = NULL;
    KM_Tag_ContextHandle begin_params = NULL;
    KM_Tag_ContextHandle update_params = NULL;
    KM_CryptoOperation_Settings settings;
    KeymasterTl_DataBlock key;
    KeymasterTl_DataBlock in_data;
    KeymasterTl_DataBlock intermediate_data;
    KeymasterTl_DataBlock final_data;
    int i, use_count, loop;
    const char *comment;
    uint32_t key_size;

    memset(&key, 0, sizeof(key));
    memset(&in_data, 0, sizeof(in_data));
    memset(&intermediate_data, 0, sizeof(intermediate_data));
    memset(&final_data, 0, sizeof(final_data));

    BDBG_LOG(("----------------------- %s -----------------------", BSTD_FUNCTION));

    for (i = 0; i < 7; i++) {
        EXPECT_SUCCESS(km_crypto_aes_select_test(i, &key_size, &begin_params, &update_params, &comment));

        EXPECT_SUCCESS(km_test_new_params_with_aes_defaults(&key_params, key_size));
        TEST_TAG_ADD_INTEGER(key_params, KM_TAG_MAX_USES_PER_BOOT, MAX_USES_PER_BOOT);
        EXPECT_SUCCESS(KeymasterTl_GenerateKey(handle, key_params, &key));

        BDBG_LOG(("%s : Test - %s", BSTD_FUNCTION, comment));

        for (use_count = 0, loop = 0; loop < (MAX_USES_PER_BOOT + 1) / 2; loop++) {
            KM_CryptoOperation_GetDefaultSettings(&settings);
            settings.handle = handle;
            settings.key_params = key_params;
            settings.in_key = key;
            settings.begin_params = begin_params;
            settings.update_params = update_params;

            TEST_ALLOCATE_BLOCK(in_data, TEST_BLOCK_SIZE);
            TEST_ALLOCATE_BLOCK(intermediate_data, TEST_BLOCK_SIZE + key_size / 8);
            TEST_ALLOCATE_BLOCK(final_data, TEST_BLOCK_SIZE + key_size / 8);
            memset(in_data.buffer, 0xC3, in_data.size);
            memset(intermediate_data.buffer, 0xFF, intermediate_data.size);
            memset(final_data.buffer, 0xA1, final_data.size);

            settings.in_data = in_data;
            settings.out_data = intermediate_data;

            if (++use_count <= MAX_USES_PER_BOOT) {
                EXPECT_SUCCESS(KM_Crypto_Operation(KM_PURPOSE_ENCRYPT, &settings));

                if (!memcmp(in_data.buffer, intermediate_data.buffer, settings.out_data.size)) {
                    BDBG_ERR(("%s: intermediate data matches", BSTD_FUNCTION));
                    err = BERR_UNKNOWN;
                    goto done;
                }

                if (settings.out_nonce) {
                    /* Move the NONCE into the in_params context */
                    TEST_TAG_ADD_BYTES(begin_params, KM_TAG_NONCE, settings.out_nonce->value.blob_data_length, settings.out_nonce->blob_data);
                    NEXUS_Memory_Free(settings.out_nonce);
                    settings.out_nonce = NULL;
                }

                settings.in_data = settings.out_data;
                settings.out_data = final_data;

                if (++use_count <= MAX_USES_PER_BOOT) {
                    EXPECT_SUCCESS(KM_Crypto_Operation(KM_PURPOSE_DECRYPT, &settings));
                    if ((settings.out_data.size != in_data.size) || memcmp(in_data.buffer, settings.out_data.buffer, settings.out_data.size)) {
                        BDBG_ERR(("%s: final data does not match", BSTD_FUNCTION));
                        err = BERR_UNKNOWN;
                        goto done;
                    }
                } else {
                    EXPECT_FAILURE_CODE(KM_Crypto_Operation(KM_PURPOSE_DECRYPT, &settings), BSAGE_ERR_KM_TOO_MANY_OPERATIONS);
                }
            } else {
                EXPECT_FAILURE_CODE(KM_Crypto_Operation(KM_PURPOSE_ENCRYPT, &settings), BSAGE_ERR_KM_TOO_MANY_OPERATIONS);
            }

            TEST_FREE_BLOCK(in_data);
            TEST_FREE_BLOCK(intermediate_data);
            TEST_FREE_BLOCK(final_data);
        }

        BDBG_LOG(("%s: %s success", BSTD_FUNCTION, comment));

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

static BERR_Code km_crypto_aes_purpose_not_authorized_tests(KeymasterTl_Handle handle)
{
    BERR_Code err;
    KM_Tag_ContextHandle key_params = NULL;
    KM_Tag_ContextHandle begin_params = NULL;
    KM_Tag_ContextHandle update_params = NULL;
    KM_CryptoOperation_Settings settings;
    KeymasterTl_DataBlock key;
    KeymasterTl_DataBlock in_data;
    KeymasterTl_DataBlock intermediate_data;
    KeymasterTl_DataBlock final_data;
    int i;
    const char *comment;
    uint32_t key_size;

    memset(&key, 0, sizeof(key));
    memset(&in_data, 0, sizeof(in_data));
    memset(&intermediate_data, 0, sizeof(intermediate_data));
    memset(&final_data, 0, sizeof(final_data));

    BDBG_LOG(("----------------------- %s -----------------------", BSTD_FUNCTION));

    for (i = 0; i < 7; i++) {
        EXPECT_SUCCESS(km_crypto_aes_select_test(i, &key_size, &begin_params, &update_params, &comment));
        BDBG_LOG(("%s : Test - %s", BSTD_FUNCTION, comment));

        TEST_ALLOCATE_BLOCK(in_data, TEST_BLOCK_SIZE);
        TEST_ALLOCATE_BLOCK(intermediate_data, TEST_BLOCK_SIZE + key_size / 8);
        TEST_ALLOCATE_BLOCK(final_data, TEST_BLOCK_SIZE + key_size / 8);
        memset(in_data.buffer, 0xC3, in_data.size);
        memset(intermediate_data.buffer, 0xFF, intermediate_data.size);
        memset(final_data.buffer, 0xA1, final_data.size);

        /* First part is to test failure to encrypt */
        EXPECT_SUCCESS(km_test_new_params_with_aes_defaults(&key_params, key_size));
        TEST_TAG_REMOVE_ENUM(key_params, KM_TAG_PURPOSE, KM_PURPOSE_ENCRYPT);
        EXPECT_SUCCESS(KeymasterTl_GenerateKey(handle, key_params, &key));

        KM_CryptoOperation_GetDefaultSettings(&settings);
        settings.handle = handle;
        settings.key_params = key_params;
        settings.in_key = key;
        settings.begin_params = begin_params;
        settings.update_params = update_params;
        settings.in_data = in_data;
        settings.out_data = intermediate_data;

        EXPECT_FAILURE_CODE(KM_Crypto_Operation(KM_PURPOSE_ENCRYPT, &settings), BSAGE_ERR_KM_INCOMPATIBLE_PURPOSE);
        TEST_DELETE_CONTEXT(key_params);
        TEST_FREE_BLOCK(key);

        /* Second part is to test failure to decrypt */
        EXPECT_SUCCESS(km_test_new_params_with_aes_defaults(&key_params, key_size));
        TEST_TAG_REMOVE_ENUM(key_params, KM_TAG_PURPOSE, KM_PURPOSE_DECRYPT);
        EXPECT_SUCCESS(KeymasterTl_GenerateKey(handle, key_params, &key));

        KM_CryptoOperation_GetDefaultSettings(&settings);
        settings.handle = handle;
        settings.key_params = key_params;
        settings.in_key = key;
        settings.begin_params = begin_params;
        settings.update_params = update_params;

        settings.in_data = in_data;
        settings.out_data = intermediate_data;

        EXPECT_SUCCESS(KM_Crypto_Operation(KM_PURPOSE_ENCRYPT, &settings));
        settings.in_data = settings.out_data;
        settings.out_data = intermediate_data;
        if (settings.out_nonce) {
            /* Move the NONCE into the in_params context */
            TEST_TAG_ADD_BYTES(begin_params, KM_TAG_NONCE, settings.out_nonce->value.blob_data_length, settings.out_nonce->blob_data);
            NEXUS_Memory_Free(settings.out_nonce);
            settings.out_nonce = NULL;
        }
        EXPECT_FAILURE_CODE(KM_Crypto_Operation(KM_PURPOSE_DECRYPT, &settings), BSAGE_ERR_KM_INCOMPATIBLE_PURPOSE);

        BDBG_LOG(("%s: %s success", BSTD_FUNCTION, comment));

        TEST_FREE_BLOCK(key);
        TEST_DELETE_CONTEXT(begin_params);
        TEST_DELETE_CONTEXT(update_params);
        TEST_DELETE_CONTEXT(key_params);
        TEST_FREE_BLOCK(in_data);
        TEST_FREE_BLOCK(intermediate_data);
        TEST_FREE_BLOCK(final_data);
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

#define GCM_NONCE_SIZE 12
#define AES_NONCE_SIZE 16

static BERR_Code km_crypto_aes_nonce_not_authorized_tests(KeymasterTl_Handle handle)
{
    BERR_Code err;
    KM_Tag_ContextHandle key_params = NULL;
    KM_Tag_ContextHandle begin_params = NULL;
    KM_Tag_ContextHandle update_params = NULL;
    KM_CryptoOperation_Settings settings;
    KeymasterTl_DataBlock key;
    KeymasterTl_DataBlock in_data;
    KeymasterTl_DataBlock intermediate_data;
    KeymasterTl_DataBlock final_data;
    KeymasterTl_DataBlock nonce;
    int i, mode;
    const char *comment;
    uint32_t key_size;

    memset(&key, 0, sizeof(key));
    memset(&in_data, 0, sizeof(in_data));
    memset(&intermediate_data, 0, sizeof(intermediate_data));
    memset(&final_data, 0, sizeof(final_data));
    memset(&nonce, 0, sizeof(nonce));

    BDBG_LOG(("----------------------- %s -----------------------", BSTD_FUNCTION));

    for (i = 0; i < 7; i++) {
        for (mode = 0; mode < 2; mode++) {
            EXPECT_SUCCESS(km_crypto_aes_select_test(i, &key_size, &begin_params, &update_params, &comment));

            EXPECT_SUCCESS(km_test_new_params_with_aes_defaults(&key_params, key_size));
            if (mode == 1) {
                TEST_TAG_REMOVE(key_params, KM_TAG_CALLER_NONCE);
            }
            EXPECT_SUCCESS(KeymasterTl_GenerateKey(handle, key_params, &key));

            BDBG_LOG(("%s : Test - %s", BSTD_FUNCTION, comment));

            KM_CryptoOperation_GetDefaultSettings(&settings);
            settings.handle = handle;
            settings.key_params = key_params;
            settings.in_key = key;
            settings.begin_params = begin_params;
            settings.update_params = update_params;

            TEST_ALLOCATE_BLOCK(in_data, TEST_BLOCK_SIZE);
            TEST_ALLOCATE_BLOCK(intermediate_data, TEST_BLOCK_SIZE + key_size / 8);
            TEST_ALLOCATE_BLOCK(final_data, TEST_BLOCK_SIZE + key_size / 8);
            memset(in_data.buffer, 0xC3, in_data.size);
            memset(intermediate_data.buffer, 0xFF, intermediate_data.size);
            memset(final_data.buffer, 0xA1, final_data.size);

            if (KM_Tag_ContainsEnum(begin_params, KM_TAG_BLOCK_MODE, KM_MODE_GCM)) {
                TEST_ALLOCATE_BLOCK(nonce, GCM_NONCE_SIZE);
            } else {
                TEST_ALLOCATE_BLOCK(nonce, AES_NONCE_SIZE);
            }
            memset(nonce.buffer, 0xA4, nonce.size);
            TEST_TAG_ADD_BYTES(begin_params, KM_TAG_NONCE, nonce.size, nonce.buffer);

            settings.in_data = in_data;
            settings.out_data = intermediate_data;

            if (!mode) {
                EXPECT_SUCCESS(KM_Crypto_Operation(KM_PURPOSE_ENCRYPT, &settings));

                if (!memcmp(in_data.buffer, intermediate_data.buffer, settings.out_data.size)) {
                    BDBG_ERR(("%s: intermediate data matches", BSTD_FUNCTION));
                    err = BERR_UNKNOWN;
                    goto done;
                }

                if (settings.out_nonce) {
                    /* Move the NONCE into the in_params context */
                    TEST_TAG_ADD_BYTES(begin_params, KM_TAG_NONCE, settings.out_nonce->value.blob_data_length, settings.out_nonce->blob_data);
                    NEXUS_Memory_Free(settings.out_nonce);
                    settings.out_nonce = NULL;
                }

                settings.in_data = settings.out_data;
                settings.out_data = final_data;

                EXPECT_SUCCESS(KM_Crypto_Operation(KM_PURPOSE_DECRYPT, &settings));

                if ((settings.out_data.size != in_data.size) || memcmp(in_data.buffer, settings.out_data.buffer, settings.out_data.size)) {
                    BDBG_ERR(("%s: final data does not match", BSTD_FUNCTION));
                    err = BERR_UNKNOWN;
                    goto done;
                }
            } else {
                EXPECT_FAILURE_CODE(KM_Crypto_Operation(KM_PURPOSE_ENCRYPT, &settings), BSAGE_ERR_KM_CALLER_NONCE_PROHIBITED);
            }

            BDBG_LOG(("%s: %s success", BSTD_FUNCTION, comment));

            TEST_FREE_BLOCK(nonce);
            TEST_FREE_BLOCK(in_data);
            TEST_FREE_BLOCK(intermediate_data);
            TEST_FREE_BLOCK(final_data);

            TEST_FREE_BLOCK(key);
            TEST_DELETE_CONTEXT(begin_params);
            TEST_DELETE_CONTEXT(update_params);
            TEST_DELETE_CONTEXT(key_params);
        }
    }
    err = BERR_SUCCESS;

done:
    TEST_FREE_BLOCK(nonce);
    TEST_FREE_BLOCK(in_data);
    TEST_FREE_BLOCK(intermediate_data);
    TEST_FREE_BLOCK(final_data);
    TEST_FREE_BLOCK(key);
    TEST_DELETE_CONTEXT(begin_params);
    TEST_DELETE_CONTEXT(update_params);
    TEST_DELETE_CONTEXT(key_params);
    return err;
}

BERR_Code km_crypto_aes_tests(KeymasterTl_Handle handle)
{
    BERR_Code err;

    EXPECT_SUCCESS(km_crypto_aes_encrypt_test(handle));
    EXPECT_SUCCESS(km_crypto_aes_parameter_tests(handle));
    EXPECT_SUCCESS(km_crypto_aes_vectors(handle));
    EXPECT_SUCCESS(km_crypto_aes_seconds_between_ops(handle));
    EXPECT_SUCCESS(km_crypto_aes_max_uses_per_boot(handle));
    EXPECT_SUCCESS(km_crypto_aes_purpose_not_authorized_tests(handle));
    EXPECT_SUCCESS(km_crypto_aes_nonce_not_authorized_tests(handle));

done:
    return err;
}
