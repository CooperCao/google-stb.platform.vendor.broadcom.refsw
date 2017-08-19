/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *****************************************************************************/
#if NEXUS_HAS_TRANSPORT
#include "nexus_types.h"
#include "dvr_crypto.h"
#include "bstd.h"
#include "bkni.h"
#include "bdbg.h"

#if NEXUS_HAS_SECURITY && NEXUS_SECURITY_API_VERSION==1
#include "nexus_security_client.h"
#include <string.h>
#include <stdlib.h>

BDBG_MODULE(dvr_crypto);

static struct dvr_crypto_settings g_dummy_settings;
#define MAX_PIDS (sizeof(g_dummy_settings.pid)/sizeof(g_dummy_settings.pid[0]))

struct dvr_crypto
{
    NEXUS_KeySlotHandle keyslot;
    struct dvr_crypto_settings settings;
};

void dvr_crypto_get_default_settings(struct dvr_crypto_settings *psettings)
{
    static const uint8_t key[] = { 0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0, 0x12, 0x34, 0xfe, 0xed, 0xba, 0xbe, 0xbe, 0xef  };
    memset(psettings, 0, sizeof(*psettings));
    psettings->key.data = (uint8_t *)key;
    psettings->key.size = sizeof(key);
    psettings->algo = NEXUS_SecurityAlgorithm_eAes;
}

dvr_crypto_t dvr_crypto_create(const struct dvr_crypto_settings *psettings)
{
    NEXUS_SecurityKeySlotSettings keySlotSettings;
    NEXUS_SecurityClearKey key;
    NEXUS_SecurityAlgorithmSettings AlgConfig;
    dvr_crypto_t handle;
    unsigned i;
    int rc;

    if (psettings->key.size > sizeof(key.keyData)) {
        BERR_TRACE(NEXUS_INVALID_PARAMETER);
        return NULL;
    }

    handle = BKNI_Malloc(sizeof(*handle));
    memset(handle, 0, sizeof(*handle));
    handle->settings = *psettings;
    psettings = &handle->settings;

    /* scrub out duplicate pids */
    for (i=1;i<MAX_PIDS;i++) {
        unsigned j;
        for (j=0;j<i;j++) {
            if (handle->settings.pid[i] == handle->settings.pid[j]) {
                handle->settings.pid[i] = 0;
                break;
            }
        }
    }

    NEXUS_Security_GetDefaultKeySlotSettings(&keySlotSettings);
    keySlotSettings.keySlotEngine = psettings->encrypt?NEXUS_SecurityEngine_eCaCp:NEXUS_SecurityEngine_eCa;
    handle->keyslot = NEXUS_Security_AllocateKeySlot(&keySlotSettings);
    BDBG_MSG(("keyslot %p %s", (void*)handle->keyslot, psettings->encrypt?"encrypt":"decrypt"));


    /* Config AV algorithms */
    NEXUS_Security_GetDefaultAlgorithmSettings(&AlgConfig);
    AlgConfig.algorithm = psettings->algo;
    AlgConfig.algorithmVar        = NEXUS_SecurityAlgorithmVariant_eEcb;
    AlgConfig.terminationMode     = NEXUS_SecurityTerminationMode_eClear;
    AlgConfig.ivMode              = NEXUS_SecurityIVMode_eRegular;
    AlgConfig.solitarySelect      = NEXUS_SecuritySolitarySelect_eClear;
    AlgConfig.caVendorID          = 0x1234;
    AlgConfig.askmModuleID        = NEXUS_SecurityAskmModuleID_eModuleID_4;
    AlgConfig.otpId               = NEXUS_SecurityOtpId_eOtpVal;
    AlgConfig.key2Select          = NEXUS_SecurityKey2Select_eReserved1;
    AlgConfig.dest                = psettings->encrypt?NEXUS_SecurityAlgorithmConfigDestination_eCps:NEXUS_SecurityAlgorithmConfigDestination_eCa;
    AlgConfig.operation           = psettings->encrypt?NEXUS_SecurityOperation_eEncrypt:NEXUS_SecurityOperation_eDecrypt;
    if (psettings->encrypt) {
        AlgConfig.bRestrictEnable     = false;
        AlgConfig.bEncryptBeforeRave  = false;
        AlgConfig.bGlobalEnable       = true;
        AlgConfig.modifyScValue[NEXUS_SecurityPacketType_eRestricted] = true;
        AlgConfig.modifyScValue[NEXUS_SecurityPacketType_eGlobal]     = true;
        AlgConfig.keyDestEntryType = NEXUS_SecurityKeyType_eClear;
    }

    if (psettings->encrypt) {
        AlgConfig.scValue[NEXUS_SecurityPacketType_eRestricted] =
        AlgConfig.scValue[NEXUS_SecurityPacketType_eGlobal]     = NEXUS_SecurityAlgorithmScPolarity_eOdd;
        rc = NEXUS_Security_ConfigAlgorithm(handle->keyslot, &AlgConfig);
        BDBG_ASSERT(!rc);
    }
    else {
        rc = NEXUS_Security_ConfigAlgorithm(handle->keyslot, &AlgConfig);
        BDBG_ASSERT(!rc);
    }

    NEXUS_Security_GetDefaultClearKey(&key);
    key.dest = psettings->encrypt?NEXUS_SecurityAlgorithmConfigDestination_eCps:NEXUS_SecurityAlgorithmConfigDestination_eCa;
    key.keyEntryType = psettings->encrypt?NEXUS_SecurityKeyType_eClear:NEXUS_SecurityKeyType_eOddAndEven;
    key.keySize = psettings->key.size;
    key.keyIVType = NEXUS_SecurityKeyIVType_eNoIV;
    memcpy(key.keyData,psettings->key.data,psettings->key.size);
    BDBG_MSG(("keysize %d", key.keySize));
    rc = NEXUS_Security_LoadClearKey(handle->keyslot, &key);
    BDBG_ASSERT(!rc);

    for (i=0;i<MAX_PIDS;i++) {
        if (!psettings->pid[i]) break;
        rc = NEXUS_KeySlot_AddPidChannel(handle->keyslot, psettings->pid[i]);
        BDBG_ASSERT(!rc);
    }

    return handle;
}

void dvr_crypto_destroy(dvr_crypto_t handle)
{
    unsigned i;
    struct dvr_crypto_settings *psettings = &handle->settings;

    for (i=0;i<MAX_PIDS;i++) {
        if (!psettings->pid[i]) break;
        NEXUS_KeySlot_RemovePidChannel(handle->keyslot, psettings->pid[i]);
    }
    NEXUS_Security_FreeKeySlot(handle->keyslot);
    BKNI_Free(handle);
}

NEXUS_KeySlotHandle dvr_crypto_keyslot(dvr_crypto_t handle)
{
    return handle->keyslot;
}
#else
void dvr_crypto_get_default_settings(struct dvr_crypto_settings *psettings)
{
    BSTD_UNUSED(psettings);
}
dvr_crypto_t dvr_crypto_create(const struct dvr_crypto_settings *psettings)
{
    BSTD_UNUSED(psettings);
    BERR_TRACE(NEXUS_NOT_SUPPORTED);
    return NULL;
}
void dvr_crypto_destroy(dvr_crypto_t handle)
{
    BSTD_UNUSED(handle);
}
NEXUS_KeySlotHandle dvr_crypto_keyslot(dvr_crypto_t handle)
{
    BSTD_UNUSED(handle);
    return NULL;
}
#endif
#else
#include <stdio.h>
int main(void)
{
    printf("This application is not supported on this platform (needs transport)!\n");
    return 0;
}
#endif
