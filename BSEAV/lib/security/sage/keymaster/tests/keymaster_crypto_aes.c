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
#include "nexus_security_client.h"
#include "nexus_memory.h"

#include "keymaster_ids.h"
#include "keymaster_platform.h"
#include "keymaster_tl.h"
#include "keymaster_test.h"
#include "keymaster_key_params.h"


BDBG_MODULE(keymaster_crypto_aes);

/* Test and free the keyslot handle */
#define TEST_FREE_KEYSLOT(hndl)               if (hndl) { NEXUS_Security_FreeKeySlot(hndl); hndl = NULL; }


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

/* Currently test must use a multiple of 3*4K because SRAI block pointers must be 4K aligned */
#define TEST_BLOCK_SIZE (3 * 4096)

/* AES tests which map to HW - tests encrypt followed by decrypt of a data block */
static BERR_Code km_crypto_aes_hw_encrypt_test(KeymasterTl_Handle handle)
{
    BERR_Code err;
    KeymasterTl_CryptoBeginSettings beginSettings;
    KeymasterTl_CryptoUpdateSettings updateSettings;
    KeymasterTl_CryptoFinishSettings finishSettings;
    KM_Tag_ContextHandle key_params = NULL;
    KM_Tag_ContextHandle in_params = NULL;
    KeymasterTl_DataBlock in_key = { 0 };
    KeymasterTl_DataBlock in_data = { 0 };
    KeymasterTl_DataBlock out_data = { 0 };
    KeymasterTl_DataBlock final_data = { 0 };
    km_operation_handle_t op_handle = 0;
    NEXUS_KeySlotHandle keyslot = NULL;
    int i, block;
    bool has_iv;
    km_tag_value_t *tag;
    km_tag_value_t *nonce = NULL;
    const char *comment;

    for (i = 0; i < 3; i++) {
        EXPECT_SUCCESS(KM_Tag_NewContext(&key_params));
        EXPECT_SUCCESS(km_test_aes_add_default_params(key_params, 128));
        EXPECT_SUCCESS(KeymasterTl_GenerateKey(handle, key_params, &in_key));
        keyslot = km_crypto_aes_allocate_keyslot();
        if (!keyslot) {
            err = BERR_OUT_OF_DEVICE_MEMORY;
            goto done;
        }

        EXPECT_SUCCESS(KM_Tag_NewContext(&in_params));
        TEST_TAG_ADD_ENUM(in_params, KM_TAG_PADDING, KM_PAD_NONE);
        switch (i) {
        case 0:
            TEST_TAG_ADD_ENUM(in_params, KM_TAG_BLOCK_MODE, KM_MODE_ECB);
            has_iv = false;
            comment = "AES ECB mode, no padding";
            break;
        case 1:
            TEST_TAG_ADD_ENUM(in_params, KM_TAG_BLOCK_MODE, KM_MODE_CBC);
            has_iv = true;
            comment = "AES CBC mode, no padding";
            break;
        case 2:
            TEST_TAG_ADD_ENUM(in_params, KM_TAG_BLOCK_MODE, KM_MODE_CTR);
            has_iv = true;
            comment = "AES CTR mode";
            break;
        default:
            BDBG_ERR(("%s:%d Internal error", BSTD_FUNCTION, __LINE__));
            err = BERR_UNKNOWN;
            goto done;
        }

        TEST_ALLOCATE_BLOCK(in_data, TEST_BLOCK_SIZE);
        TEST_ALLOCATE_BLOCK(out_data, TEST_BLOCK_SIZE);
        TEST_ALLOCATE_BLOCK(final_data, TEST_BLOCK_SIZE);
        memset(in_data.buffer, 0xC3, in_data.size);
        memset(out_data.buffer, 0xFF, in_data.size);
        memset(final_data.buffer, 0xA1, in_data.size);

        KeymasterTl_GetDefaultCryptoBeginSettings(&beginSettings);
        beginSettings.purpose = KM_PURPOSE_ENCRYPT;
        beginSettings.in_key_blob = in_key;
        beginSettings.hKeyslot = keyslot;
        beginSettings.in_params = in_params;
        EXPECT_SUCCESS(KeymasterTl_CryptoBegin(handle, &beginSettings, &op_handle));
        tag = KM_Tag_FindFirst(beginSettings.out_params, KM_TAG_NONCE);
        if (tag && !has_iv) {
            TEST_DELETE_CONTEXT(beginSettings.out_params);
            BDBG_ERR(("%s: NONCE should not be returned", BSTD_FUNCTION));
            err = BERR_UNKNOWN;
            goto done;
        }
        if (tag) {
            nonce = KM_Tag_Dup(tag);
        }
        TEST_DELETE_CONTEXT(beginSettings.out_params);
        if (has_iv && !nonce) {
            BDBG_ERR(("%s: Missing NONCE", BSTD_FUNCTION));
            err = BERR_UNKNOWN;
            goto done;
        }

        /* First 2 blocks done through update and final through finish */
        for (block = 0; block < 2; block++) {
            KeymasterTl_GetDefaultCryptoUpdateSettings(&updateSettings);
            updateSettings.in_data.size = in_data.size / 3;
            updateSettings.in_data.buffer = (!block) ? in_data.buffer : (in_data.buffer + in_data.size / 3);
            updateSettings.out_data.size = out_data.size / 3;
            updateSettings.out_data.buffer = (!block) ? out_data.buffer : (out_data.buffer + out_data.size / 3);
            EXPECT_SUCCESS(KeymasterTl_CryptoUpdate(handle, op_handle, &updateSettings));
            TEST_DELETE_CONTEXT(updateSettings.out_params);
        }

        KeymasterTl_GetDefaultCryptoFinishSettings(&finishSettings);
        finishSettings.in_data.size = in_data.size / 3;
        finishSettings.in_data.buffer = in_data.buffer + 2 * (in_data.size / 3);
        finishSettings.out_data.size = out_data.size / 3;
        finishSettings.out_data.buffer = out_data.buffer + 2 * (out_data.size / 3);
        EXPECT_SUCCESS(KeymasterTl_CryptoFinish(handle, op_handle, &finishSettings));
        TEST_DELETE_CONTEXT(finishSettings.out_params);
        op_handle = 0;

        if (!memcmp(in_data.buffer, out_data.buffer, in_data.size)) {
            BDBG_ERR(("%s: intermediate data matches", BSTD_FUNCTION));
            err = BERR_UNKNOWN;
            goto done;
        }

        if (nonce) {
            TEST_TAG_ADD_BYTES(in_params, KM_TAG_NONCE, nonce->value.blob_data_length, nonce->blob_data);
        }

        KeymasterTl_GetDefaultCryptoBeginSettings(&beginSettings);
        beginSettings.purpose = KM_PURPOSE_DECRYPT;
        beginSettings.in_key_blob = in_key;
        beginSettings.hKeyslot = keyslot;
        beginSettings.in_params = in_params;
        EXPECT_SUCCESS(KeymasterTl_CryptoBegin(handle, &beginSettings, &op_handle));
        TEST_DELETE_CONTEXT(beginSettings.out_params);

        /* First 2 blocks done through update and final through finish */
        for (block = 0; block < 2; block++) {
            KeymasterTl_GetDefaultCryptoUpdateSettings(&updateSettings);
            updateSettings.in_data.size = out_data.size / 3;
            updateSettings.in_data.buffer = (!block) ? out_data.buffer : (out_data.buffer + out_data.size / 3);
            updateSettings.out_data.size = final_data.size / 3;
            updateSettings.out_data.buffer = (!block) ? final_data.buffer : (final_data.buffer + final_data.size / 3);
            EXPECT_SUCCESS(KeymasterTl_CryptoUpdate(handle, op_handle, &updateSettings));
            TEST_DELETE_CONTEXT(updateSettings.out_params);
        }

        KeymasterTl_GetDefaultCryptoFinishSettings(&finishSettings);
        finishSettings.in_data.size = out_data.size / 3;
        finishSettings.in_data.buffer = out_data.buffer + 2 * (out_data.size / 3);
        finishSettings.out_data.size = final_data.size / 3;
        finishSettings.out_data.buffer = final_data.buffer + 2 * (final_data.size / 3);
        EXPECT_SUCCESS(KeymasterTl_CryptoFinish(handle, op_handle, &finishSettings));
        TEST_DELETE_CONTEXT(finishSettings.out_params);
        op_handle = 0;

        if (memcmp(in_data.buffer, final_data.buffer, in_data.size)) {
            BDBG_ERR(("%s: final data does not match", BSTD_FUNCTION));
            err = BERR_UNKNOWN;
            goto done;
        }
        if (nonce) {
            NEXUS_Memory_Free(nonce);
            nonce = NULL;
        }

        BDBG_LOG(("%s: %s success", BSTD_FUNCTION, comment));
    }
    err = BERR_SUCCESS;

done:
    if (op_handle) {
        /* If crypto finish fails, this will throw an error too */
        (void)KeymasterTl_CryptoAbort(handle, op_handle);
    }
    if (nonce) {
        NEXUS_Memory_Free(nonce);
    }
    TEST_FREE_KEYSLOT(keyslot);
    TEST_FREE_BLOCK(in_key);
    TEST_FREE_BLOCK(in_data);
    TEST_FREE_BLOCK(out_data);
    TEST_FREE_BLOCK(final_data);
    TEST_DELETE_CONTEXT(key_params);
    TEST_DELETE_CONTEXT(in_params);
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

    EXPECT_SUCCESS(KM_Tag_NewContext(&key_params));
    EXPECT_SUCCESS(km_test_aes_add_default_params(key_params, 128));
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

    EXPECT_SUCCESS(km_crypto_aes_hw_encrypt_test(handle));
    EXPECT_SUCCESS(km_crypto_aes_parameter_tests(handle));

done:
    return err;
}
