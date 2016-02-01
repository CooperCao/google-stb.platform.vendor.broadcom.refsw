/******************************************************************************
 *    (c)2008-2011 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 *
 *****************************************************************************/

#include "bstd.h"
#include "bkni.h"
#include "nexus_security.h"

/* SAGE-related utility/key_loader layer */
#include "key_loader_tl.h"

#include <stdio.h>

static KeyLoaderTl_Handle hKeyLoader = NULL;

int prepare_keyslots(int playback, NEXUS_KeySlotHandle *pM2mKeySlotHandle, NEXUS_KeySlotHandle *pCpKeySlotHandle, NEXUS_KeySlotHandle *pVideoCaKeySlotHandle, NEXUS_KeySlotHandle *pAudioCaKeySlotHandle)
{
    int ret_code = -1;
    {
        NEXUS_Error rc;
        KeyLoaderTlSettings keyLoaderModuleSettings;
        KeyLoaderTl_GetDefaultSettings(&keyLoaderModuleSettings);
        rc = KeyLoaderTl_Init(&hKeyLoader, &keyLoaderModuleSettings);
        BDBG_ASSERT(rc == BERR_SUCCESS);
    }

    /* Allocate and configure M2M KeySlot */
    {
        KeyLoader_KeySlotConfigSettings keySlotConfigSettings;
        KeyLoader_WrappedKeySettings wrappedKeySettings;
        BERR_Code rc;

        /* M2M keyladder information */
        static const uint8_t m2m_procInForKey3[] = {0xAA, 0xCC, 0xEE, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
        static const uint8_t m2m_procInForKey4[] = {0xBB, 0xDD, 0xFF, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};

        KeyLoader_GetDefaultConfigKeySlotSettings(&keySlotConfigSettings);
        keySlotConfigSettings.engine          = BSAGElib_Crypto_Engine_eM2m;
        keySlotConfigSettings.algorithm       = BSAGElib_Crypto_Algorithm_eAes;
        keySlotConfigSettings.algorithmVar    = BSAGElib_Crypto_AlgorithmVariant_eCbc;
        keySlotConfigSettings.terminationMode = BSAGElib_Crypto_TerminationMode_eBlock;
        keySlotConfigSettings.solitarySelect  = BSAGElib_Crypto_SolitaryMode_eClear;
        if (playback) {
            keySlotConfigSettings.operation   = BSAGElib_Crypto_Operation_eDecrypt;
        }
        else {
            keySlotConfigSettings.operation   = BSAGElib_Crypto_Operation_eEncrypt;
        }
        keySlotConfigSettings.profileIndex    = 2;
        keySlotConfigSettings.custSubMode     = BSAGElib_Crypto_CustomerSubMode_eGeneric_CP_128_4;
        keySlotConfigSettings.keyType         = BSAGElib_Crypto_KeyType_eClear;

        rc = KeyLoader_AllocAndConfigKeySlot(hKeyLoader, pM2mKeySlotHandle, &keySlotConfigSettings);
        if (rc != BERR_SUCCESS)
        {
            fprintf(stderr, "Cannot Config and Allocate M2M Keyslot\n");
            goto error;
        }

        KeyLoader_GetDefaultWrappedKeySettings(&wrappedKeySettings);

        wrappedKeySettings.keyladderAlg = BSAGElib_Crypto_Algorithm_eAes;
        wrappedKeySettings.keyladderDepth = BSAGElib_Crypto_KeyLadderLevel_eKey4;
        BKNI_Memcpy(wrappedKeySettings.procInForKey3, m2m_procInForKey3, 16);
        BKNI_Memcpy(wrappedKeySettings.procInForKey4, m2m_procInForKey4, 16);
        wrappedKeySettings.keyLength = 16;
        wrappedKeySettings.ivLength = 16;
        BKNI_Memset(wrappedKeySettings.iv, 0, sizeof(wrappedKeySettings.iv));

        wrappedKeySettings.keyType = BSAGElib_Crypto_KeyType_eClear;
        rc = KeyLoader_LoadWrappedKey(hKeyLoader, *pM2mKeySlotHandle, &wrappedKeySettings);
        if (rc != BERR_SUCCESS)
        {
            fprintf(stderr, "KeyLoader_LoadWrappedKey FAILED for M2M\n");
            goto error;
        }
    }

    /* Allocate and configure CPS KeySlot */
    {
        KeyLoader_KeySlotConfigSettings keySlotConfigSettings;
        KeyLoader_WrappedKeySettings wrappedKeySettings;
        BERR_Code rc;

        /* CP keyladder information */
        static const uint8_t cp_procInForKey3[] = {0xAB, 0xCD, 0xEF, 0x34, 0x45, 0x56, 0x67, 0x78, 0x89, 0x9A, 0xAB, 0xBC, 0xCD, 0xDE, 0xEF, 0xF0};
        static const uint8_t cp_procInForKey4[] = {0xCB, 0xED, 0x0F, 0x43, 0x54, 0x65, 0x76, 0x87, 0x98, 0xA9, 0xBA, 0xCB, 0xDC, 0xED, 0xFE, 0x0F};

        KeyLoader_GetDefaultConfigKeySlotSettings(&keySlotConfigSettings);
        keySlotConfigSettings.engine          = BSAGElib_Crypto_Engine_eCaCp;
        keySlotConfigSettings.algorithm       = BSAGElib_Crypto_Algorithm_eAes;
        keySlotConfigSettings.algorithmVar    = BSAGElib_Crypto_AlgorithmVariant_eCbc;
        keySlotConfigSettings.terminationMode = BSAGElib_Crypto_TerminationMode_eClear;
        keySlotConfigSettings.solitarySelect  = BSAGElib_Crypto_SolitaryMode_eClear;
        if (playback) {
            keySlotConfigSettings.operation   = BSAGElib_Crypto_Operation_eDecrypt;
        }
        else {
            keySlotConfigSettings.operation   = BSAGElib_Crypto_Operation_eEncrypt;
        }
        keySlotConfigSettings.profileIndex    = 1;
        keySlotConfigSettings.custSubMode     = BSAGElib_Crypto_CustomerSubMode_eGeneric_CP_128_4;
        keySlotConfigSettings.keyType         = BSAGElib_Crypto_KeyType_eOddAndEven;

        rc = KeyLoader_AllocAndConfigKeySlot(hKeyLoader, pCpKeySlotHandle, &keySlotConfigSettings);
        if (rc != BERR_SUCCESS)
        {
            fprintf(stderr, "Cannot Config and Allocate CPS Keyslot\n");
            goto error;
        }

        KeyLoader_GetDefaultWrappedKeySettings(&wrappedKeySettings);

        wrappedKeySettings.keyladderAlg = BSAGElib_Crypto_Algorithm_eAes;
        wrappedKeySettings.keyladderDepth = BSAGElib_Crypto_KeyLadderLevel_eKey4;
        BKNI_Memcpy(wrappedKeySettings.procInForKey3, cp_procInForKey3, 16);
        BKNI_Memcpy(wrappedKeySettings.procInForKey4, cp_procInForKey4, 16);
        wrappedKeySettings.keyLength = 16;
        wrappedKeySettings.ivLength = 16;
        BKNI_Memset(wrappedKeySettings.iv, 0, sizeof(wrappedKeySettings.iv));

        wrappedKeySettings.keyType = BSAGElib_Crypto_KeyType_eEven;
        rc = KeyLoader_LoadWrappedKey(hKeyLoader, *pCpKeySlotHandle, &wrappedKeySettings);
        if (rc != BERR_SUCCESS)
        {
            fprintf(stderr, "KeyLoader_LoadWrappedKey FAILED for CPS Even\n");
            goto error;
        }
        wrappedKeySettings.keyType = BSAGElib_Crypto_KeyType_eOdd;
        rc = KeyLoader_LoadWrappedKey(hKeyLoader, *pCpKeySlotHandle, &wrappedKeySettings);
        if (rc != BERR_SUCCESS)
        {
            fprintf(stderr, "KeyLoader_LoadWrappedKey FAILED for CPS Odd\n");
            goto error;
        }
    }

    /* Allocate and configure CA KeySlots */
    if (pVideoCaKeySlotHandle && pAudioCaKeySlotHandle && !playback /* only decrypt CA streams on recorder app */)
    {
        KeyLoader_KeySlotConfigSettings keySlotConfigSettings;
        KeyLoader_WrappedKeySettings wrappedKeySettings;
        BERR_Code rc;

        /* CA keyladder information */
        uint8_t ca_procInForKey3[] = {0xca, 0x1b, 0x36, 0x2c, 0x43, 0x75, 0xf8, 0x82, 0x7a, 0x03, 0xf5, 0xd8, 0x1d, 0x2f, 0xe9, 0xa4};
        uint8_t ca_procInForKey4[] = {0xf5, 0x31, 0x33, 0x94, 0xd0, 0x9a, 0x3f, 0x3e, 0x18, 0x2a, 0x0b, 0x1f, 0x0e, 0x93, 0xc0, 0x3c};
        uint8_t ca_procInForKey5[] = {0x6c, 0x8e, 0xfa, 0xa4, 0xe0, 0x7b, 0x89, 0x51, 0x2a, 0x3c, 0x10, 0xf6, 0x50, 0x93, 0xaf, 0x4f};

        KeyLoader_GetDefaultConfigKeySlotSettings(&keySlotConfigSettings);
        keySlotConfigSettings.engine          = BSAGElib_Crypto_Engine_eCa;
        keySlotConfigSettings.algorithm       = BSAGElib_Crypto_Algorithm_eAes;
        keySlotConfigSettings.algorithmVar    = BSAGElib_Crypto_AlgorithmVariant_eCbc;
        keySlotConfigSettings.terminationMode = BSAGElib_Crypto_TerminationMode_eBlock;
        keySlotConfigSettings.solitarySelect  = BSAGElib_Crypto_SolitaryMode_eClear;
        keySlotConfigSettings.operation       = BSAGElib_Crypto_Operation_eDecrypt;
        keySlotConfigSettings.profileIndex    = 0;
        keySlotConfigSettings.custSubMode     = BSAGElib_Crypto_CustomerSubMode_eGeneric_CA_128_5;
        keySlotConfigSettings.keyType         = BSAGElib_Crypto_KeyType_eOddAndEven;

        rc = KeyLoader_AllocAndConfigKeySlot(hKeyLoader, pVideoCaKeySlotHandle, &keySlotConfigSettings);
        if (rc != BERR_SUCCESS)
        {
            fprintf(stderr, "Cannot Config and Allocate CA Keyslot for Video\n");
            goto error;
        }

        rc = KeyLoader_AllocAndConfigKeySlot(hKeyLoader, pAudioCaKeySlotHandle, &keySlotConfigSettings);
        if (rc != BERR_SUCCESS)
        {
            fprintf(stderr, "Cannot Config and Allocate CA Keyslot for Audio\n");
            goto error;
        }

        KeyLoader_GetDefaultWrappedKeySettings(&wrappedKeySettings);

        wrappedKeySettings.keyladderAlg = BSAGElib_Crypto_Algorithm_eAes;
        wrappedKeySettings.keyladderDepth = BSAGElib_Crypto_KeyLadderLevel_eKey5;
        BKNI_Memcpy(wrappedKeySettings.procInForKey3, ca_procInForKey3, 16);
        BKNI_Memcpy(wrappedKeySettings.procInForKey4, ca_procInForKey4, 16);
        BKNI_Memcpy(wrappedKeySettings.procInForKey5, ca_procInForKey5, 16);
        wrappedKeySettings.keyLength = 16;
        wrappedKeySettings.ivLength = 16;
        BKNI_Memset(wrappedKeySettings.iv, 0, sizeof(wrappedKeySettings.iv));

        wrappedKeySettings.keyType = BSAGElib_Crypto_KeyType_eEven;
        rc = KeyLoader_LoadWrappedKey(hKeyLoader, *pVideoCaKeySlotHandle, &wrappedKeySettings);
        if (rc != BERR_SUCCESS)
        {
            fprintf(stderr, "KeyLoader_LoadWrappedKey FAILED for CA EVEN (video)\n");
            goto error;
        }

        wrappedKeySettings.keyType = BSAGElib_Crypto_KeyType_eOdd;
        rc = KeyLoader_LoadWrappedKey(hKeyLoader, *pVideoCaKeySlotHandle, &wrappedKeySettings);
        if (rc != BERR_SUCCESS)
        {
            fprintf(stderr, "KeyLoader_LoadWrappedKey FAILED for CA ODD (video)\n");
            goto error;
        }
        wrappedKeySettings.keyType = BSAGElib_Crypto_KeyType_eEven;
        rc = KeyLoader_LoadWrappedKey(hKeyLoader, *pAudioCaKeySlotHandle, &wrappedKeySettings);
        if (rc != BERR_SUCCESS)
        {
            fprintf(stderr, "KeyLoader_LoadWrappedKey FAILED for CA EVEN (audio)\n");
            goto error;
        }

        wrappedKeySettings.keyType = BSAGElib_Crypto_KeyType_eOdd;
        rc = KeyLoader_LoadWrappedKey(hKeyLoader, *pAudioCaKeySlotHandle, &wrappedKeySettings);
        if (rc != BERR_SUCCESS)
        {
            fprintf(stderr, "KeyLoader_LoadWrappedKey FAILED for CA ODD (audio)\n");
            goto error;
        }
    }

    ret_code = 0;

error:
    return ret_code;
}


void clean_keyslots(NEXUS_KeySlotHandle m2mKeySlotHandle, NEXUS_KeySlotHandle cpKeySlotHandle, NEXUS_KeySlotHandle videoCaKeySlotHandle, NEXUS_KeySlotHandle audioCaKeySlotHandle)
{
    if (hKeyLoader) {
        if(m2mKeySlotHandle != NULL)
        {
            KeyLoader_FreeKeySlot(hKeyLoader, m2mKeySlotHandle);
            m2mKeySlotHandle = NULL;
        }

        if(cpKeySlotHandle != NULL)
        {
            KeyLoader_FreeKeySlot(hKeyLoader, cpKeySlotHandle);
            cpKeySlotHandle = NULL;
        }

        if(videoCaKeySlotHandle != NULL)
        {
            KeyLoader_FreeKeySlot(hKeyLoader, videoCaKeySlotHandle);
            videoCaKeySlotHandle = NULL;
        }

        if(audioCaKeySlotHandle != NULL)
        {
            KeyLoader_FreeKeySlot(hKeyLoader, audioCaKeySlotHandle);
            audioCaKeySlotHandle = NULL;
        }

        KeyLoaderTl_Uninit(hKeyLoader);
        hKeyLoader = NULL;
    }
    return;
}
