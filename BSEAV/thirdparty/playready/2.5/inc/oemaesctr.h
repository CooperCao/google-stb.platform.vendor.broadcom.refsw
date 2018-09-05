/**************************************************************************
///    Copyright (c)201, Broadcom Corporation
///    All Rights Reserved
///    Confidential Property of Broadcom Corporation
///
/// THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
/// AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
/// EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
///
/// @brief Header file containing hardware specific functions for the OEM module
///
*****************************************************************************/

#include <oem.h>

#ifndef __OEMAESCTR_H__
#define __OEMAESCTR_H__


ENTER_PK_NAMESPACE

DRM_API Oem_AesHwHandle DRM_CALL Oem_AesCtr_Init(
#if (NEXUS_SECURITY_API_VERSION==1)
        NEXUS_SecurityOperation opType
#else
        NEXUS_CryptographicOperation opType
#endif
#if (PLAYREADY_STANDALONE_IMPL==1)
,   DRM_BOOL isContentKeyProtected
#endif
);

DRM_API DRM_RESULT DRM_CALL Oem_AesCtr_Uninit(
    Oem_AesHwHandle aesHwHandle
);

DRM_API DRM_RESULT DRM_CALL Oem_AesCtr_IncrementRefCount(
    Oem_AesHwHandle aesHwHandle
);

#if (PLAYREADY_HOST_IMPL==1)
NEXUS_Error Oem_AesCtr_ProcessData(
    Oem_AesHwHandle aesHwHandle,
    uint64_t nonce,
    uint64_t blockCounter,
    size_t  byteOffset,
    const NEXUS_DmaJobBlockSettings *pBlks,
    uint32_t nDmaBlocks,
    uint8_t *pBTP
);
#elif (PLAYREADY_STANDALONE_IMPL==1)
NEXUS_Error Oem_AesCtr_ProcessData(
    Oem_AesHwHandle aesHwHandle,
    uint64_t nonce,
    uint64_t blockCounter,
    size_t  byteOffset,
    const NEXUS_DmaJobBlockSettings *pBlks,
    uint32_t nDmaBlocks,
    DRM_BOOL isContentKeyProtected
);
#endif

NEXUS_Error Oem_AesCtr_LoadKey(
    Oem_AesHwHandle aesHwHandle,
    uint8_t *pKey,
    size_t   keySize
);

NEXUS_KeySlotHandle Oem_AesCtr_KeySlotHandle(
    Oem_AesHwHandle aesHwHandle
);

NEXUS_Error Oem_AesCtr_LoadCipheredKey(
    Oem_AesHwHandle aesHwHandle,
    const CommonCryptoCipheredKeySettings *pSettings
);

EXIT_PK_NAMESPACE

#endif /* __OEMAESCTR_H__ */
