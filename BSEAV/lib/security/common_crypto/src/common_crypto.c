/******************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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

#include "bstd.h"
#include "bkni.h"

#include "nexus_base_mmap.h"
#include "nexus_base_os.h"
#include "nexus_memory.h"

#include "nexus_types.h"
#include "nexus_keyladder.h"
#include "nexus_core_priv.h"

#include "common_crypto.h"


BDBG_MODULE(common_crypto);


typedef struct CommonCryptoHeapInfo
{
    NEXUS_Addr offset;
    uint32_t size;
}CommonCryptoHeapInfo;

typedef struct CommonCrypto
{
    BDBG_OBJECT(CommonCrypto)

    NEXUS_DmaHandle dmaHandle; /* Dma handle*/
    CommonCryptoSettings settings;
    BKNI_EventHandle dmaEvent;

    NEXUS_DmaJobHandle dmaJob;
    NEXUS_DmaJobSettings dmaJobSettings;
    NEXUS_DmaJobBlockSettings *localBlkSettings;
    CommonCryptoHeapInfo videoSecureHeapInfo[2]; /* Retrieve the video secure heap information during initialization */
}CommonCrypto;

static NEXUS_Error CommonCrypto_LoadClearKey_priv(NEXUS_KeySlotHandle keySlot,
                                                  const uint8_t  *pKey,
                                                  uint32_t  keySize,
                                                  NEXUS_SecurityKeyType keySlotType,
                                                  NEXUS_SecurityKeyIVType keyIvType);

static NEXUS_Error CommonCrypto_LoadCipheredKey_priv(NEXUS_KeySlotHandle keySlot,
                                                     NEXUS_SecurityRootKeySrc keySrc,
                                                     NEXUS_SecurityKeyType keySlotType,
                                                     const CommonCryptoKeyLadderSettings   *pKeyLadderInfo);


static void CommonCrypto_P_InitializeVideoSecureHeapInfo(CommonCryptoHandle handle);
static bool CommonCrypto_P_IsInVideoSecureHeap(CommonCryptoHandle handle, NEXUS_Addr offset, uint32_t size);
static NEXUS_Error CommonCrypto_P_FlushCache(CommonCryptoHandle handle, const uint8_t *address, const uint32_t size);


static void CompleteCallback(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
}

void CommonCrypto_GetDefaultSettings(
    CommonCryptoSettings *pSettings
    )
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
}

void CommonCrypto_GetDefaultCipheredKeySettings(
    CommonCryptoCipheredKeySettings *pSettings    /* [out] default settings */
    )
{
    NEXUS_SecurityAlgorithmSettings nexusAlgSettings;
    BDBG_ASSERT(pSettings != NULL);
    BKNI_Memset(pSettings, 0, sizeof(CommonCryptoCipheredKeySettings));
    CommonCrypto_GetDefaultKeyLadderSettings(&pSettings->settings);

    NEXUS_Security_GetDefaultAlgorithmSettings(&nexusAlgSettings);
    pSettings->keySlotType = nexusAlgSettings.keyDestEntryType;
}


void CommonCrypto_GetDefaultClearKeySettings(
    CommonCryptoClearKeySettings *pSettings    /* [out] default settings */
    )
{
    NEXUS_SecurityAlgorithmSettings nexusAlgSettings;
    BDBG_ASSERT(pSettings != NULL);
    BKNI_Memset(pSettings, 0, sizeof(CommonCryptoClearKeySettings));

    NEXUS_Security_GetDefaultAlgorithmSettings(&nexusAlgSettings);
    pSettings->keySlotType = nexusAlgSettings.keyDestEntryType;
}

#if (NEXUS_SECURITY_HAS_ASKM == 1)
#define UINT32_MAX__ (0xFFFFFFFF -1)
static uint32_t strToHex(const char *s)
{
   uint32_t result = 0;
   int c = 0;

   if (NULL == s) {
       BDBG_ERR(("string is empty"));
       return UINT32_MAX__;
   }
   if ('-' == *s) {
       BDBG_ERR(("string starts with -"));
       return UINT32_MAX__;
   }
   if ('0' == *s && 'x' == *(s+1)) {
       s += 2;
   }
   while (*s != '\0') {
          result = result << 4;
          if (c = (*s-'0'),(c>=0 && c <=9))
              result |= c;
          else if (c = (*s-'A'),(c>=0 && c <=5))
              result |= (c+10);
          else if (c = (*s-'a'),(c>=0 && c <=5))
              result |= (c+10);
          else {
              BDBG_ERR(("string contains non-hex character: %c", *s));
              return UINT32_MAX__;
          }
          ++s;
   }
   return result;
}
#endif  /*NEXUS_SECURITY_HAS_ASKM == 1*/

void CommonCrypto_GetDefaultKeyConfigSettings(
    CommonCryptoKeyConfigSettings *pSettings    /* [out] default settings */
    )
{
#if (NEXUS_SECURITY_HAS_ASKM == 1)
    const char* vendorID = NULL;
    const char* stbOwnerID    = NULL;
    const char* maskKey2Select = NULL;
#endif  /*NEXUS_SECURITY_HAS_ASKM == 1*/

    NEXUS_SecurityAlgorithmSettings nexusAlgSettings;
    BDBG_ASSERT(pSettings != NULL);
    BKNI_Memset(pSettings, 0, sizeof(CommonCryptoKeyConfigSettings));

    NEXUS_Security_GetDefaultAlgorithmSettings(&nexusAlgSettings);
    pSettings->settings.keySlotType = nexusAlgSettings.keyDestEntryType;

#if (NEXUS_SECURITY_HAS_ASKM == 1)
    pSettings->settings.maskKey2Select  = NEXUS_SecurityKey2Select_eFixedKey;
    pSettings->settings.caVendorID      = 0x1234;
    pSettings->settings.askmModuleID    = NEXUS_SecurityAskmModuleID_eModuleID_8;
    pSettings->settings.ivMode          = NEXUS_SecurityIVMode_eRegular;
    pSettings->settings.stbOwnerID      = NEXUS_SecurityOtpId_eOneVal;
    pSettings->settings.enableMaskKey2Select  = false;
    if (NULL != (vendorID = NEXUS_GetEnv("COMMON_DRM_CA_VENDOR_ID"))) {
        uint32_t num = strToHex(vendorID);
        if (num == UINT32_MAX__) {
            BDBG_ERR(("could not convert vendorID : %s", vendorID));
        }
        pSettings->settings.caVendorID = num;
        BDBG_MSG(("after getenv---%s vendorID is 0x%x", __FUNCTION__, pSettings->settings.caVendorID));
    }
    if (NULL != (stbOwnerID = NEXUS_GetEnv("COMMON_DRM_STB_OWNER_ID"))) {
        uint32_t num = (NEXUS_SecurityOtpId)strToHex(stbOwnerID);
        if (num == UINT32_MAX__) {
            BDBG_ERR(("could not convert stbOwnerID : %s", stbOwnerID));
        }
        pSettings->settings.stbOwnerID = num;
        BDBG_MSG(("after getenv---%s otpID is %d", __FUNCTION__, pSettings->settings.stbOwnerID));
    }
    if (NULL != (maskKey2Select = NEXUS_GetEnv("COMMON_DRM_MASKKEY2_ID"))) {
        uint32_t num = (NEXUS_SecurityKey2Select)strToHex(maskKey2Select);
        if (num == UINT32_MAX__) {
            BDBG_ERR(("could not convert maskKey2Select : %s", maskKey2Select));
        }
        pSettings->settings.maskKey2Select = num;
        BDBG_MSG(("after getenv---%s maskKey2Select is %d", __FUNCTION__, pSettings->settings.maskKey2Select));
    }
#endif /*(NEXUS_SECURITY_HAS_ASKM == 1)*/
}

void CommonCrypto_GetDefaultKeySettings(
    CommonCryptoKeySettings *pSettings,    /* [out] default settings */
    CommonCryptoKeySrc keySrc
    )
{
    NEXUS_SecurityAlgorithmSettings nexusAlgSettings;
    BDBG_ASSERT(pSettings != NULL);
    BKNI_Memset(pSettings, 0, sizeof(CommonCryptoKeySettings));

    NEXUS_Security_GetDefaultAlgorithmSettings(&nexusAlgSettings);
    pSettings->alg.keySlotType = nexusAlgSettings.keyDestEntryType;

    if(keySrc != CommonCrypto_eClearKey){
        CommonCrypto_GetDefaultKeyLadderSettings((CommonCryptoKeyLadderSettings*)&pSettings->src);
    }
}


CommonCryptoHandle CommonCrypto_Open(
    const CommonCryptoSettings *pSettings
    )
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    BERR_Code magnumRc;
    CommonCryptoHandle  pCrypto;
    CommonCryptoSettings defaultSettings;

    if (!pSettings) {
        CommonCrypto_GetDefaultSettings(&defaultSettings);
        pSettings = &defaultSettings;
    }

    pCrypto = BKNI_Malloc(sizeof(*pCrypto));
    if (!pCrypto) {
        rc = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        goto handle_malloc_error;
    }

    BKNI_Memset(pCrypto, 0, sizeof(*pCrypto));
    magnumRc = BKNI_CreateEvent(&pCrypto->dmaEvent);
    if (magnumRc != BERR_SUCCESS) {
        BDBG_ERR(("BKNI_CreateEvent failure (%d)", magnumRc));
        rc = BERR_TRACE(NEXUS_NOT_INITIALIZED);
        goto handle_error;
    }
    NEXUS_DmaJob_GetDefaultSettings(&pCrypto->dmaJobSettings);
    pCrypto->dmaJobSettings.completionCallback.callback = CompleteCallback;
    pCrypto->dmaJobSettings.completionCallback.context = pCrypto->dmaEvent;
    pCrypto->dmaJobSettings.numBlocks = 0;


    /* DMA is virtualized, hence it can be opened multiple times */
    pCrypto->dmaHandle = NEXUS_Dma_Open(0, &pSettings->dmaSettings);
    if(pCrypto->dmaHandle == NULL)
    {
        rc = BERR_TRACE(NEXUS_NOT_AVAILABLE);
        goto handle_error;
    }

    BKNI_Memcpy(&pCrypto->settings, pSettings, sizeof(*pSettings));

    CommonCrypto_P_InitializeVideoSecureHeapInfo(pCrypto);

handle_error:
    if(rc != NEXUS_SUCCESS)
    {
        CommonCrypto_Close(pCrypto);
        pCrypto = NULL;
    }
handle_malloc_error:
    return pCrypto;
}

static void CommonCrypto_FreeDmaJobResources_priv(CommonCryptoHandle handle);

void CommonCrypto_Close(
    CommonCryptoHandle handle
    )
{
    if(handle != NULL)
    {
        CommonCrypto_FreeDmaJobResources_priv(handle);
        if(handle->dmaHandle != NULL)
        {
            NEXUS_Dma_Close(handle->dmaHandle);
        }
        if(handle->dmaEvent != NULL)
        {
            BKNI_DestroyEvent(handle->dmaEvent);
        }
        BKNI_Free(handle);
    }
}

/***************************************************************************
Summary:
Get default key ladder operation settings.

Description:
This function is used to initialize a CommonCryptoKeyLadderOperationStruct structure
with default settings used during the key3 and key4 generation.

***************************************************************************/
void CommonCrypto_GetDefaultKeyLadderSettings(
        CommonCryptoKeyLadderSettings *pSettings)
{
    BDBG_MSG(("%s - Entered function", __FUNCTION__));
    pSettings->overwriteKeyLadderOperation = false;
    pSettings->overwriteVKLSettings = false;
    pSettings->KeyLadderOpStruct.SessionKeyOperation = NEXUS_SecurityOperation_eDecrypt;
    pSettings->KeyLadderOpStruct.SessionKeyOperationKey2 = NEXUS_SecurityOperation_eEncrypt;
    pSettings->KeyLadderOpStruct.ControlWordKeyOperation = NEXUS_SecurityOperation_eDecrypt;
    pSettings->VirtualKeyLadderSettings.CustSubMode = NEXUS_SecurityCustomerSubMode_eGeneric_CP_128_4;
    pSettings->VirtualKeyLadderSettings.VklValue = NEXUS_SecurityVirtualKeyladderID_eVKL4;

    /* SWSECURITY-195 */
    pSettings->keyladderID = NEXUS_SecurityKeyladderID_eA;
    pSettings->keyladderType = NEXUS_SecurityKeyladderType_e3Des;
    pSettings->swizzleType = NEXUS_SecuritySwizzleType_eSwizzle0;
    pSettings->askmSupport = false;
    pSettings->aesKeySwap = false;
    pSettings->key4Size = COMMON_CRYPTO_PROC_SIZE;
    pSettings->key3Size = COMMON_CRYPTO_PROC_SIZE;

    BDBG_MSG(("%s - Exiting function", __FUNCTION__));
    return;
}



void CommonCrypto_GetSettings(
    CommonCryptoHandle handle,
    CommonCryptoSettings *pSettings
    )
{
    BDBG_ASSERT(pSettings != NULL);

    *pSettings = handle->settings;
    return;
}

NEXUS_Error CommonCrypto_SetSettings(
    CommonCryptoHandle handle,
    const CommonCryptoSettings *pSettings
    )
{
    NEXUS_Error rc;

    BDBG_ASSERT(pSettings != NULL);

    rc =  NEXUS_Dma_SetSettings(handle->dmaHandle, &pSettings->dmaSettings);

    BKNI_Memcpy(&(handle->settings), pSettings, sizeof(CommonCryptoSettings));

    /* Need to update the video secure heap information. */
    CommonCrypto_P_InitializeVideoSecureHeapInfo(handle);

    return rc;
}


void CommonCrypto_GetDefaultJobSettings(
    CommonCryptoJobSettings *pSettings /* [out] */
    )
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->dataFormat = NEXUS_DmaDataFormat_eBlock;

    return;
}

static void CommonCrypto_FreeDmaJobResources_priv(
    CommonCryptoHandle handle
    )
{
    if (handle->dmaJobSettings.numBlocks != 0)
    {
        NEXUS_DmaJob_Destroy(handle->dmaJob);
        BKNI_Free(handle->localBlkSettings);
    }
}

/* Returns success? */
static bool CommonCrypto_EnsureDmaJobResources_priv(
    CommonCryptoHandle handle,
    const CommonCryptoJobSettings *pJobSettings,
    unsigned nBlocks
    )
{
    if (nBlocks <= handle->dmaJobSettings.numBlocks
        && handle->dmaJobSettings.keySlot == pJobSettings->keySlot
        && handle->dmaJobSettings.dataFormat == pJobSettings->dataFormat)
        return true;

    CommonCrypto_FreeDmaJobResources_priv(handle);

#define MINIMUM_DESCRIPTORS 16
    if (nBlocks < MINIMUM_DESCRIPTORS)
        nBlocks = MINIMUM_DESCRIPTORS;

    handle->dmaJobSettings.numBlocks = nBlocks;
    handle->dmaJobSettings.keySlot = pJobSettings->keySlot;
    handle->dmaJobSettings.dataFormat = pJobSettings->dataFormat;
    handle->dmaJob = NEXUS_DmaJob_Create(handle->dmaHandle, &handle->dmaJobSettings);
    if(handle->dmaJob == NULL)
    {
        handle->dmaJobSettings.numBlocks = 0;
        return false;
    }

    /* Since we handle the cache flush locally, we need a private copy of the block settings (we need to modify the settings). */
    handle->localBlkSettings = BKNI_Malloc(nBlocks * sizeof(NEXUS_DmaJobBlockSettings));
    if (handle->localBlkSettings == NULL)
    {
        handle->dmaJobSettings.numBlocks = 0;
        NEXUS_DmaJob_Destroy(handle->dmaJob);
        return false;
    }
    return true;
}

NEXUS_Error CommonCrypto_DmaXfer(
    CommonCryptoHandle handle,
    const CommonCryptoJobSettings *pJobSettings,
    const NEXUS_DmaJobBlockSettings *pBlkSettings,
    unsigned nBlocks
    )
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    NEXUS_DmaJobStatus jobStatus;

    unsigned ii = 0;

    BDBG_ASSERT(pBlkSettings != NULL);
    BDBG_ASSERT(pJobSettings != NULL);

    BDBG_MSG(("CommonCrypto_DmaXfer: Enter"));
    BDBG_MSG(("%s - Entered function using keySlot '%p'  nBlocks = '%u'", __FUNCTION__, (void*)pJobSettings->keySlot, nBlocks));

    if (!CommonCrypto_EnsureDmaJobResources_priv(handle, pJobSettings, nBlocks))
    {
        BDBG_ERR(("%s - NEXUS_DmaJob_Create failed", __FUNCTION__));
        rc = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        goto errorExit;
    }

    BKNI_Memcpy(handle->localBlkSettings, pBlkSettings, nBlocks * sizeof(NEXUS_DmaJobBlockSettings));

    for(ii = 0; ii < nBlocks; ii++)
    {
        if(handle->localBlkSettings[ii].pSrcAddr == NULL ||
           handle->localBlkSettings[ii].pDestAddr == NULL)
        {
            BDBG_ERR(("%s - pSrcAddr %p or pDestAddr %p is invalid", __FUNCTION__, handle->localBlkSettings[ii].pSrcAddr, handle->localBlkSettings[ii].pDestAddr));
            rc = NEXUS_INVALID_PARAMETER;
            goto errorExit;
        }

        /* We have to handle the cache flush locally because we want to skip the cache flush for the blocks located */
        /* in the SAGE video secure heap.                                                                           */
        if(handle->localBlkSettings[ii].cached)
        {
            /* Flush cache for the source buffer. */
            rc = CommonCrypto_P_FlushCache(handle, handle->localBlkSettings[ii].pSrcAddr, handle->localBlkSettings[ii].blockSize);
            if(rc != NEXUS_SUCCESS)
            {
                BDBG_ERR(("%s - Failure to flush cache for source buffer.", __FUNCTION__));
                goto errorExit;
            }

            if(handle->localBlkSettings[ii].pDestAddr != handle->localBlkSettings[ii].pSrcAddr)
            {
                /* Flush cache for the destination buffer. */
                rc = CommonCrypto_P_FlushCache(handle, handle->localBlkSettings[ii].pDestAddr, handle->localBlkSettings[ii].blockSize);
                if(rc != NEXUS_SUCCESS)
                {
                    BDBG_ERR(("%s - Failure to flush cache for destination buffer.", __FUNCTION__));
                    goto errorExit;
                }
            }

            /* Cache flush is completed. Therefore, clear the cache flag. */
            handle->localBlkSettings[ii].cached = false;
        }
    }


    /* We can save one context switch by calling NEXUS_DmaJob_ProcessBlocks_priv instead of NEXUS_DmaJob_ProcessBlocks. */
    rc = NEXUS_DmaJob_ProcessBlocks(handle->dmaJob, handle->localBlkSettings, nBlocks);
    if (rc == NEXUS_DMA_QUEUED) {
        BERR_Code rc2;
        rc2 = BKNI_WaitForEvent(handle->dmaEvent, BKNI_INFINITE);
        if ( rc2 != BERR_SUCCESS ) {
            BDBG_ERR(("%s - BKNI_WaitForEvent failed, rc2 = %d", __FUNCTION__, rc2));
            rc = NEXUS_UNKNOWN;
            goto errorExit;
        }
        rc = NEXUS_DmaJob_GetStatus(handle->dmaJob, &jobStatus);
        if (rc || (jobStatus.currentState != NEXUS_DmaJobState_eComplete)) {
            BDBG_ERR(("%s - NEXUS_DmaJob_ProcessBlocks failed, rc = %d", __FUNCTION__, rc));
            goto errorExit;
        }
    }
    else if (rc != NEXUS_SUCCESS) {
        BDBG_ERR(("%s - NEXUS_DmaJob_ProcessBlocks failed, rc = %d", __FUNCTION__, rc));
        goto errorExit;
    }

    for(ii = 0; ii < nBlocks; ii++)
    {
        /* Flushing destination address after DMA operation only if memmory is cached and not found */
        /* in the SAGE video secure heap.                                                           */
        if(pBlkSettings[ii].cached)
        {
            /* Flush cache for the source buffer. */
            rc = CommonCrypto_P_FlushCache(handle, pBlkSettings[ii].pDestAddr, pBlkSettings[ii].blockSize);
            if(rc != NEXUS_SUCCESS)
            {
                BDBG_ERR(("%s - Failure to flush cache for destination buffer after DMA operation.", __FUNCTION__));
                goto errorExit;
            }
        }
    }

errorExit:
    BDBG_MSG(("%s - Exiting function", __FUNCTION__));
    return rc;
}

NEXUS_Error CommonCrypto_P_LoadKeyConfig(
    CommonCryptoHandle handle,
    NEXUS_KeySlotHandle keySlot,
    const CommonCryptoAlgorithmSettings *pSettings
    )
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_SecurityAlgorithmSettings     nexusConfig;

    BDBG_MSG(("%s - Entered function ^^^^^^^^^****************", __FUNCTION__));

    if(handle->settings.dmaSettings.coreType != NEXUS_DmaCoreType_eSharf){

        /* Set up key */
        NEXUS_Security_GetDefaultAlgorithmSettings(&nexusConfig);

        /* Config Algrithm */
        nexusConfig.algorithm           = pSettings->algType;
        nexusConfig.algorithmVar        = pSettings->algVariant;
        nexusConfig.operation           = pSettings->opType;
        nexusConfig.keyDestEntryType    = pSettings->keySlotType;
        nexusConfig.terminationMode     = pSettings->termMode;
        nexusConfig.enableExtKey        = pSettings->enableExtKey;
        nexusConfig.enableExtIv         = pSettings->enableExtIv;
        nexusConfig.aesCounterSize      = pSettings->aesCounterSize;
        nexusConfig.aesCounterMode      = pSettings->aesCounterMode;
        nexusConfig.solitarySelect		= pSettings->solitaryMode;

#if (NEXUS_SECURITY_HAS_ASKM == 1)
        nexusConfig.key2Select          = pSettings->maskKey2Select;
        nexusConfig.caVendorID          = pSettings->caVendorID;
        nexusConfig.askmModuleID        = pSettings->askmModuleID;
        nexusConfig.ivMode              = pSettings->ivMode;
        nexusConfig.otpId               = pSettings->stbOwnerID;
        nexusConfig.testKey2Select      = pSettings->enableMaskKey2Select;
#endif

        /* Configure the keyslot for the particular operation (except when using Sharf Engine) */
        rc = NEXUS_Security_ConfigAlgorithm(keySlot, &nexusConfig);
        if(rc != NEXUS_SUCCESS){
            return BERR_TRACE(rc);
        }

    }
    else
    {
        rc = BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }

    return rc;
}

NEXUS_Error CommonCrypto_LoadKeyConfig(
    CommonCryptoHandle handle,
    const CommonCryptoKeyConfigSettings *pSettings
    )
{
    NEXUS_Error rc;

    BDBG_ASSERT(pSettings != NULL);
    BDBG_ASSERT(pSettings->keySlot != NULL);

    rc = CommonCrypto_P_LoadKeyConfig(handle, pSettings->keySlot, &pSettings->settings);

    return rc;
}



NEXUS_Error CommonCrypto_P_LoadClearKeyIv(
    CommonCryptoHandle handle,
    NEXUS_KeySlotHandle keySlot,
    NEXUS_SecurityKeyType keySlotType,
    const CommonCryptoKeyIvSettings *pKeyIvStruct
    )
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    if(pKeyIvStruct->keySize > 0 )
    {
        rc = CommonCrypto_LoadClearKey_priv(keySlot,
                                pKeyIvStruct->key,
                                pKeyIvStruct->keySize,
                                keySlotType,
                                NEXUS_SecurityKeyIVType_eNoIV);
        if(rc != NEXUS_SUCCESS){
            return BERR_TRACE(rc);
        }
    }

    if(handle->settings.dmaSettings.coreType != NEXUS_DmaCoreType_eSharf){
        if(pKeyIvStruct->ivSize > 0)
        {
            rc = CommonCrypto_LoadClearKey_priv(keySlot,
                                    pKeyIvStruct->iv,
                                    pKeyIvStruct->ivSize,
                                    keySlotType,
                                    NEXUS_SecurityKeyIVType_eIV);
            if(rc != NEXUS_SUCCESS){
                return BERR_TRACE(rc);
            }
        }
    }
    else {
        /* When using the Sharf, the IV must be specified in dma descriptor */
        rc = BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }

   return rc;
}

NEXUS_Error CommonCrypto_LoadClearKeyIv(
    CommonCryptoHandle handle,
    const CommonCryptoClearKeySettings *pSettings
    )
{
    NEXUS_Error rc;

    BDBG_ASSERT(pSettings != NULL);
    BDBG_ASSERT(pSettings->keySlot != NULL);

    rc = CommonCrypto_P_LoadClearKeyIv(handle, pSettings->keySlot, pSettings->keySlotType, &pSettings->settings);

    return rc;
}

static NEXUS_Error CommonCrypto_P_LoadCipheredKey(
    NEXUS_KeySlotHandle keySlot,
    CommonCryptoKeySrc keySrc,
    NEXUS_SecurityKeyType keySlotType,
    const CommonCryptoKeyLadderSettings *pKeyLadderInfo
    )
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_SecurityRootKeySrc  rootKeySrc;

    switch(keySrc)
    {
        case CommonCrypto_eCustKey:
            rootKeySrc = NEXUS_SecurityRootKeySrc_eCuskey;
            break;
        case CommonCrypto_eOtpKey:
            rootKeySrc = NEXUS_SecurityRootKeySrc_eOtpKeyA;
            break;
        case CommonCrypto_eOtpKeyB:
            rootKeySrc = NEXUS_SecurityRootKeySrc_eOtpKeyB;
            break;
        case CommonCrypto_eOtpKeyC:
            rootKeySrc = NEXUS_SecurityRootKeySrc_eOtpKeyC;
            break;
        default:
            return BERR_TRACE(NEXUS_INVALID_PARAMETER);
            break;
    }

    rc = CommonCrypto_LoadCipheredKey_priv(keySlot, rootKeySrc, keySlotType, pKeyLadderInfo);

    return rc;
}

NEXUS_Error CommonCrypto_LoadCipheredKey(
    CommonCryptoHandle handle,
    const CommonCryptoCipheredKeySettings *pSettings
    )
{
    NEXUS_Error rc;

    BDBG_ASSERT(pSettings != NULL);
    BDBG_ASSERT(pSettings->keySlot != NULL);
    BSTD_UNUSED(handle);

    rc = CommonCrypto_P_LoadCipheredKey(pSettings->keySlot, pSettings->keySrc, pSettings->keySlotType, &pSettings->settings);

    return rc;
}

NEXUS_Error CommonCrypto_SetupKey(
    CommonCryptoHandle handle,
    const CommonCryptoKeySettings *pSettings
    )
{
    NEXUS_Error rc  = NEXUS_SUCCESS;

    BDBG_ASSERT(pSettings != NULL);
    BDBG_ASSERT(pSettings->keySlot != NULL);

    BDBG_MSG(("%s - Entered function", __FUNCTION__));

    rc = CommonCrypto_P_LoadKeyConfig(handle,
                            pSettings->keySlot,
                            &pSettings->alg);
    if(rc != NEXUS_SUCCESS) {
        return BERR_TRACE(rc);
    }

     switch(pSettings->keySrc){
        case CommonCrypto_eClearKey:

            rc = CommonCrypto_P_LoadClearKeyIv(handle,
                            pSettings->keySlot,
                            pSettings->alg.keySlotType,
                            &pSettings->src.keyIvInfo);
            break;
        case CommonCrypto_eCustKey:
        case CommonCrypto_eOtpKey:
            rc = CommonCrypto_P_LoadCipheredKey(
                            pSettings->keySlot,
                            pSettings->keySrc,
                            pSettings->alg.keySlotType,
                            &pSettings->src.keyLadderInfo);
            break;
        default:
            return BERR_TRACE(NEXUS_INVALID_PARAMETER);
            break;
    }

    BDBG_MSG(("%s - Exiting function", __FUNCTION__));
    return rc;
}

static NEXUS_Error CommonCrypto_LoadClearKey_priv(
    NEXUS_KeySlotHandle keySlot,
    const uint8_t *pKey,
    uint32_t keySize,
    NEXUS_SecurityKeyType keySlotType,
    NEXUS_SecurityKeyIVType keyIvType
    )
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_SecurityClearKey swKey;

    BDBG_ASSERT(pKey != NULL);
    BDBG_ASSERT(keySlot != NULL);

    BDBG_MSG(("%s - Loading sw key or iv... (keyIvType = %u) keySlotType = %u", __FUNCTION__, keyIvType, keySlotType));

    if(keySize > sizeof(swKey.keyData)){
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    if(keySize > COMMON_CRYPTO_KEY_SIZE)
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);

    NEXUS_Security_GetDefaultClearKey(&swKey);

    swKey.keyEntryType = keySlotType;
    swKey.keySize      = keySize;
    swKey.keyIVType    = keyIvType;

    BKNI_Memcpy(swKey.keyData, pKey, keySize);

    rc = NEXUS_Security_LoadClearKey(keySlot, &swKey);
    if(rc != NEXUS_SUCCESS)
    {
        return BERR_TRACE(rc);
    }

    return rc;
}


static NEXUS_Error CommonCrypto_LoadCipheredKey_priv(
    NEXUS_KeySlotHandle keySlot,
    NEXUS_SecurityRootKeySrc keySrc,
    NEXUS_SecurityKeyType keySlotType,
    const CommonCryptoKeyLadderSettings *pKeyLadderInfo
    )
{
    NEXUS_Error rc = NEXUS_SUCCESS;
/*SWSECDRM-1165 : use dynamic vkl allocation for all Zeus version*/

    NEXUS_SecurityVKLSettings    vklSettings;
    NEXUS_VirtualKeyLadderHandle vkl = NULL;
    NEXUS_VirtualKeyLadderInfo   vklInfo;


    NEXUS_SecurityEncryptedSessionKey encryptedSessionkey;
    NEXUS_SecurityEncryptedControlWord encrytedCW;

    BDBG_ASSERT(pKeyLadderInfo != NULL);
    BDBG_ASSERT(keySlot != NULL);

    /*SWSECDRM-1165 : use dynamic vkl allocation for all Zeus version*/

    /* Allocate a VKL */
    NEXUS_Security_GetDefaultVKLSettings(&vklSettings);
    vklSettings.custSubMode = pKeyLadderInfo->VirtualKeyLadderSettings.CustSubMode;
    vklSettings.client      = NEXUS_SecurityClientType_eHost;
    vkl = NEXUS_Security_AllocateVKL(&vklSettings);
    if(vkl == NULL)
    {
        BDBG_ERR(("%s - Failure to allocate a VKL.", __FUNCTION__));
        rc = NEXUS_NOT_AVAILABLE;
        goto handle_error;
    }

    /* Retrieve VKL info */
    NEXUS_Security_GetVKLInfo(vkl, &vklInfo);


    NEXUS_Security_GetDefaultSessionKeySettings(&encryptedSessionkey);
    NEXUS_Security_GetDefaultControlWordSettings(&encrytedCW);

    encryptedSessionkey.keyladderID     = pKeyLadderInfo->keyladderID;
    encryptedSessionkey.keyladderType	= pKeyLadderInfo->keyladderType;
    encryptedSessionkey.swizzleType		= pKeyLadderInfo->swizzleType;

    encryptedSessionkey.rootKeySrc      = keySrc;
    encryptedSessionkey.bRouteKey       = false;
    encryptedSessionkey.operation       = pKeyLadderInfo->KeyLadderOpStruct.SessionKeyOperation;/* Default: NEXUS_SecurityOperation_eDecrypt;*/
    encryptedSessionkey.operationKey2   = pKeyLadderInfo->KeyLadderOpStruct.SessionKeyOperationKey2;/* Default: NEXUS_SecurityOperation_eEncrypt;*/
    encryptedSessionkey.keyEntryType    = keySlotType;

    if(keySrc == NEXUS_SecurityRootKeySrc_eCuskey)
    {
        BDBG_MSG(("%s - Setting CustKey values", __FUNCTION__));
        encryptedSessionkey.cusKeyL         = pKeyLadderInfo->custKeySelect;
        encryptedSessionkey.cusKeyH         = pKeyLadderInfo->custKeySelect;
        encryptedSessionkey.cusKeyVarL      = pKeyLadderInfo->keyVarLow;
        encryptedSessionkey.cusKeyVarH      = pKeyLadderInfo->keyVarHigh;
    }
    BKNI_Memcpy(encryptedSessionkey.keyData, pKeyLadderInfo->procInForKey3, pKeyLadderInfo->key3Size);

#if (NEXUS_SECURITY_HAS_ASKM == 1)
    BDBG_MSG(("%s - Session key (40nm and after) *****************", __FUNCTION__));
    encryptedSessionkey.keyGenCmdID = NEXUS_SecurityKeyGenCmdID_eKeyGen;
    encryptedSessionkey.sessionKeyOp =  NEXUS_SecuritySessionKeyOp_eNoProcess;
    encryptedSessionkey.bASKMMode = pKeyLadderInfo->askmSupport;
    encryptedSessionkey.bSwapAESKey = pKeyLadderInfo->aesKeySwap;
    encryptedSessionkey.keyDestIVType = NEXUS_SecurityKeyIVType_eNoIV;
    /*SWSECDRM-1165 : use dynamic vkl allocation for all Zeus version*/
    /* Use dynamically allocated VKL. */
    BDBG_MSG(("%s - Session key (Zeus 3.0 and after) *****************", __FUNCTION__));
    encryptedSessionkey.custSubMode        = vklInfo.custSubMode;
    encryptedSessionkey.virtualKeyLadderID = vklInfo.vkl;
#endif /*NEXUS_SECURITY_HAS_ASKM*/
    encryptedSessionkey.keyMode = NEXUS_SecurityKeyMode_eRegular;

    BDBG_MSG(("%s - Calling 'NEXUS_Security_GenerateSessionKey'", __FUNCTION__));
    rc = NEXUS_Security_GenerateSessionKey(keySlot, &encryptedSessionkey);
    if(rc == NEXUS_SUCCESS){
        /* Load CW */
        encrytedCW.keyladderID = pKeyLadderInfo->keyladderID;
        encrytedCW.keyladderType = pKeyLadderInfo->keyladderType;
        encrytedCW.swizzleType = pKeyLadderInfo->swizzleType;
        encrytedCW.keySize = pKeyLadderInfo->key4Size;
        encrytedCW.keyEntryType = keySlotType;
        encrytedCW.operation = pKeyLadderInfo->KeyLadderOpStruct.ControlWordKeyOperation;/* Default: NEXUS_SecurityOperation_eDecrypt;*/
        encrytedCW.bRouteKey = true;
        encrytedCW.rootKeySrc = keySrc;

        BKNI_Memcpy(encrytedCW.keyData, pKeyLadderInfo->procInForKey4, sizeof(pKeyLadderInfo->procInForKey4));

#if (NEXUS_SECURITY_HAS_ASKM == 1)
        BDBG_MSG(("%s - Control Word (40nm and after) *****************", __FUNCTION__));
        encrytedCW.keyDestIVType = NEXUS_SecurityKeyIVType_eNoIV;
        encrytedCW.keyGenCmdID = NEXUS_SecurityKeyGenCmdID_eKeyGen;
        encrytedCW.bSwapAESKey = pKeyLadderInfo->aesKeySwap;
        /*SWSECDRM-1165 : use dynamic vkl allocation for all Zeus version*/
        /* Use dynamically allocated VKL. */
        BDBG_MSG(("%s - Control Word (Zeus 3.0 and after) *****************", __FUNCTION__));
        encrytedCW.custSubMode        = vklInfo.custSubMode;
        encrytedCW.virtualKeyLadderID = vklInfo.vkl;

#endif /*NEXUS_SECURITY_HAS_ASKM*/

        encrytedCW.keyMode = NEXUS_SecurityKeyMode_eRegular;
        rc = NEXUS_Security_GenerateControlWord(keySlot, &encrytedCW);
        if(rc != NEXUS_SUCCESS)
        {
            BDBG_ERR(("%s - Error generating Control Word (key4)", __FUNCTION__));
        }
    }
    else
    {
        BDBG_ERR(("%s - Error generating Session key (key3)", __FUNCTION__));
    }
    /*SWSECDRM-1165 : use dynamic vkl allocation for all Zeus version, hence free vkl*/

handle_error:
    if(vkl != NULL)
    {
        NEXUS_Security_FreeVKL(vkl);
        vkl = NULL;
    }


    BDBG_MSG(("%s - Exiting function", __FUNCTION__));
    return rc;
}



/* Retrieve the information for the video secure heap and save them into the Common Crypto context. */
/* If the Common Crypto settings contain an invalid index for the video secure heap, search in all  */
/* client heaps for a secure heap.                                                                  */
static void CommonCrypto_P_InitializeVideoSecureHeapInfo(CommonCryptoHandle handle)
{
    NEXUS_HeapHandle hHeap;
    NEXUS_MemoryStatus status;
    NEXUS_Error rc;

    BDBG_ASSERT(handle != NULL);

    hHeap = NEXUS_Heap_Lookup(NEXUS_HeapLookupType_eCompressedRegion);
    if(hHeap)
    {
        rc = NEXUS_Heap_GetStatus(hHeap, &status);

        if (rc != NEXUS_SUCCESS)
        {
            BDBG_ERR(("NEXUS_Heap_GetStatus(%p) failure (%d) ", (void *)hHeap, rc));
            return;
        }
        else
        {
            handle->videoSecureHeapInfo[0].offset = status.offset;
            handle->videoSecureHeapInfo[0].size = status.size;
        }
    }

    hHeap = NEXUS_Heap_Lookup(NEXUS_HeapLookupType_eExportRegion);
    if(hHeap)
    {
        rc = NEXUS_Heap_GetStatus(hHeap, &status);
        if (rc != NEXUS_SUCCESS)
        {
            BDBG_ERR(("NEXUS_Heap_GetStatus(%p) failure (%d) ", (void *)hHeap, rc));
            return;
        }
        else
        {
            handle->videoSecureHeapInfo[1].offset = status.offset;
            handle->videoSecureHeapInfo[1].size = status.size;
        }
    }

    if (handle->videoSecureHeapInfo[0].size)
    {
        BDBG_MSG(("%s: CRR heap information: offset: 0x%08x, size: %u.", __FUNCTION__, (unsigned)handle->videoSecureHeapInfo[0].offset, handle->videoSecureHeapInfo[0].size));
    }
    else
    {
        BDBG_MSG(("%s: Cannot find a CRR heap in client heaps.", __FUNCTION__));
    }

    if (handle->videoSecureHeapInfo[1].size)
    {
        BDBG_MSG(("%s: XRR heap information: offset: 0x%08x, size: %u.", __FUNCTION__, (unsigned)handle->videoSecureHeapInfo[1].offset, handle->videoSecureHeapInfo[1].size));
    }
    else
    {
        BDBG_MSG(("%s: Cannot find an XRR heap in client heaps.", __FUNCTION__));
    }
    return;
}


/* Is the block within the video secure heap? */
static bool CommonCrypto_P_IsInVideoSecureHeap(CommonCryptoHandle handle, NEXUS_Addr offset, uint32_t size)
{
    /* When there is no video secure heap, the heap offset and size will */
    /* be zero. Therefore, this function will always return false.       */
    return ( ((offset >= handle->videoSecureHeapInfo[0].offset) &&
            ((offset + size) <= (handle->videoSecureHeapInfo[0].offset + handle->videoSecureHeapInfo[0].size)))
            || ((offset >= handle->videoSecureHeapInfo[1].offset) &&
            ((offset + size) <= (handle->videoSecureHeapInfo[1].offset + handle->videoSecureHeapInfo[1].size))) );
}


/* Flush cache for the specified block. The function will skip the flush for blocks located in the video secure heap */
/* because the memory is not CPU accessible.                                                                         */
static NEXUS_Error CommonCrypto_P_FlushCache(CommonCryptoHandle handle, const uint8_t *address, const uint32_t size)
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    NEXUS_Addr offset;

    /* Verify parameters */
    if((handle == NULL) || (address == NULL) || (size == 0))
    {
        BDBG_ERR(("%s - Invalid parameter.", __FUNCTION__));
        rc = NEXUS_INVALID_PARAMETER;
        goto errorExit;
    }

    /* Get the offset */
    offset = NEXUS_AddrToOffset(address);
    if(offset == 0)
    {
        BDBG_ERR(("%s - Cannot retrieve offset from address %p.", __FUNCTION__, address));
        rc = NEXUS_INVALID_PARAMETER;
        goto errorExit;
    }

    /* Offsets in the video secure heap are not CPU accessible. Therefore, they shall not be flushed. */
    if(!CommonCrypto_P_IsInVideoSecureHeap(handle, offset, size))
    {
        NEXUS_FlushCache(address, size);
    }

errorExit:
    return rc;
}
