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


BDBG_MODULE(keymaster_crypto_utils);

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

void KM_CryptoOperation_GetDefaultSettings(KM_CryptoOperation_Settings *settings)
{
    BDBG_ASSERT(settings);
    memset(settings, 0, sizeof(KM_CryptoOperation_Settings));
    settings->in_data_process_blocks = 1;
}

BERR_Code KM_Crypto_Operation(km_purpose_t operation, KM_CryptoOperation_Settings *settings)
{
    BERR_Code err;
    KeymasterTl_CryptoBeginSettings beginSettings;
    KeymasterTl_CryptoUpdateSettings updateSettings;
    KeymasterTl_CryptoFinishSettings finishSettings;
    km_operation_handle_t op_handle = 0;
    NEXUS_KeySlotHandle keyslot = NULL;
    km_algorithm_t algorithm;
    km_tag_value_t *tag;
    uint8_t *out_data;
    uint32_t available_space;
    uint32_t block;
    uint32_t block_size;
    bool updateCalled = false;
    uint32_t data_available;
    uint8_t *in_data;

    if (!settings || !settings->in_data_process_blocks ||
        ((operation != KM_PURPOSE_ENCRYPT) && (operation != KM_PURPOSE_DECRYPT) &&
         (operation != KM_PURPOSE_SIGN) && (operation != KM_PURPOSE_VERIFY))) {
        BDBG_ERR(("%s: Invalid parameter to function", BSTD_FUNCTION));
        err = BERR_UNKNOWN;
        goto done;
    }

    TEST_FIND_TAG(tag, settings->key_params, KM_TAG_ALGORITHM);
    algorithm = (km_algorithm_t)tag->value.enumerated;
    switch (algorithm) {
    case KM_ALGORITHM_AES:
        {
            keyslot = km_crypto_aes_allocate_keyslot();
            if (!keyslot) {
                err = BERR_OUT_OF_DEVICE_MEMORY;
                goto done;
            }
        }
        break;
    default:
        /* Nothing to do */
        break;
    }

    KeymasterTl_GetDefaultCryptoBeginSettings(&beginSettings);
    beginSettings.purpose = operation;
    beginSettings.in_key_blob = settings->in_key;
    beginSettings.hKeyslot = keyslot;
    beginSettings.in_params = settings->begin_params;
    EXPECT_SUCCESS_SILENT(KeymasterTl_CryptoBegin(settings->handle, &beginSettings, &op_handle));
    if ((operation == KM_PURPOSE_ENCRYPT) && (!settings->out_nonce)) {
        /* No incoming nonce, look to save outgoing nonce */
        tag = KM_Tag_FindFirst(beginSettings.out_params, KM_TAG_NONCE);
        if (tag) {
            settings->out_nonce = KM_Tag_Dup(tag);
        }
    }
    TEST_DELETE_CONTEXT(beginSettings.out_params);

    out_data = settings->out_data.buffer;
    available_space = settings->out_data.size;
    block_size = settings->in_data.size / settings->in_data_process_blocks;
    data_available = settings->in_data.size;
    in_data = settings->in_data.buffer;

    for (block = 0; block < settings->in_data_process_blocks - 1; block++) {
        KeymasterTl_GetDefaultCryptoUpdateSettings(&updateSettings);
        if (!updateCalled && KM_Tag_GetNumPairs(settings->update_params)) {
            /* Only first update gets the params (GCM needs AAD data on first update) */
            updateSettings.in_params = settings->update_params;
        }
        updateSettings.in_data.buffer = in_data;
        updateSettings.in_data.size = block_size;
        updateSettings.out_data.size = available_space;
        updateSettings.out_data.buffer = out_data;
        err = KeymasterTl_CryptoUpdate(settings->handle, op_handle, &updateSettings);
        if (err == BERR_NOT_SUPPORTED) {
            /* This is allowed - data can only be provided to the finish call */
            break;
        } else {
            EXPECT_SUCCESS_SILENT(err);
            updateCalled = true;
            out_data += updateSettings.out_data_size;
            available_space -= updateSettings.out_data_size;
            in_data += block_size;
            data_available -= block_size;
        }
        TEST_DELETE_CONTEXT(updateSettings.out_params);
    }

    KeymasterTl_GetDefaultCryptoFinishSettings(&finishSettings);
    if (data_available) {
        if (!updateCalled && KM_Tag_GetNumPairs(settings->update_params)) {
            /* If update was not called yet, add update parameters to finish */
            finishSettings.in_params = settings->update_params;
        }
        finishSettings.in_data.buffer = in_data;
        finishSettings.in_data.size = data_available;
    }
    finishSettings.out_data.size = available_space;
    finishSettings.out_data.buffer = out_data;
    finishSettings.in_signature = settings->signature_data;
    EXPECT_SUCCESS_SILENT(KeymasterTl_CryptoFinish(settings->handle, op_handle, &finishSettings));
    op_handle = 0;
    TEST_DELETE_CONTEXT(finishSettings.out_params);
    BDBG_ASSERT(finishSettings.out_data_size <= available_space);
    available_space -= finishSettings.out_data_size;
    /* Reduce the out_data.size by how much space is free in the buffer */
    settings->out_data.size -= available_space;
    err = BERR_SUCCESS;

done:
    if (op_handle) {
        /* If crypto finish fails, this will throw an error too */
        (void)KeymasterTl_CryptoAbort(settings->handle, op_handle);
    }
    TEST_FREE_KEYSLOT(keyslot);
    return err;
}
