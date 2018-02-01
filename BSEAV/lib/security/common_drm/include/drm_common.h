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
#ifndef DRM_COMMON_H_
#define DRM_COMMON_H_

#include <openssl/x509.h>

#include "drm_types.h"
#include "drm_common_swcrypto_types.h"
#include "common_crypto.h"

#ifdef __cplusplus
extern "C" {
#endif

#define COMMON_DRM_VERSION "CMNDRM 1.29"

#define DRM_MSG_PRINT_BUF(buffer_name, buffer_ptr, buffer_size)         \
do {                                                                \
    uint32_t _word_no_ = 0;                                         \
    uint32_t _word_count_ = 0;                                      \
    uint32_t _unaligned_data_size_ = 0;                             \
                                                                    \
    _unaligned_data_size_ = (buffer_size % 4);                      \
    _word_count_ = (buffer_size - _unaligned_data_size_) >> 2;      \
                                                                    \
    BDBG_MSG(("%s: %s: ", BSTD_FUNCTION, buffer_name));             \
    for(_word_no_ = 0; _word_no_ < _word_count_; _word_no_++)       \
    {                                                               \
        BDBG_MSG(("%02x %02x %02x %02x", ((uint8_t *)(buffer_ptr))[4 * _word_no_], \
                  ((uint8_t *)(buffer_ptr))[4 * _word_no_ + 1],     \
                  ((uint8_t *)(buffer_ptr))[4 * _word_no_ + 2],     \
                  ((uint8_t *)(buffer_ptr))[4 * _word_no_ + 3]));   \
    }                                                               \
                                                                    \
    if(_unaligned_data_size_ == 1)                                  \
    {                                                               \
        BDBG_MSG(("%02x", ((uint8_t *)(buffer_ptr))[4 * _word_count_])); \
    }                                                               \
    else if(_unaligned_data_size_ == 2)                             \
    {                                                               \
        BDBG_MSG(("%02x %02x", ((uint8_t *)(buffer_ptr))[4 * _word_count_], \
                  ((uint8_t *)(buffer_ptr))[4 * _word_count_ + 1])); \
    }                                                               \
    else if(_unaligned_data_size_ == 3)                             \
    {                                                               \
        BDBG_MSG(("%02x %02x %02x", ((uint8_t *)(buffer_ptr))[4 * _word_count_], \
                  ((uint8_t *)(buffer_ptr))[4 * _word_count_ + 1],  \
                  ((uint8_t *)(buffer_ptr))[4 * _word_count_ + 2])); \
    }                                                               \
                                                                    \
    BDBG_MSG(("\n"));                                               \
} while(0)

#define GET_UINT32_FROM_BUF(pBuf)                   \
    (((uint32_t)(((uint8_t*)(pBuf))[0]) << 24) |    \
    ((uint32_t)(((uint8_t*)(pBuf))[1]) << 16) |     \
    ((uint32_t)(((uint8_t*)(pBuf))[2]) << 8)  |     \
    ((uint8_t *)(pBuf))[3])


/* Structure Definitions */
typedef struct DrmCommonOperationStruct_t
{
    CommonCryptoKeyConfigSettings keyConfigSettings;
    CommonCryptoKeySrc keySrc;
    CommonCryptoKeyIvSettings keyIvSettings;
    CommonCryptoKeyLadderSettings* pKeyLadderInfo;
    DmaBlockInfo_t* pDmaBlock;
    uint32_t    num_dma_block;
    bool byPassKeyConfig;
}DrmCommonOperationStruct_t;

typedef struct drm_chip_info_t
{
    uint8_t devIdA[8];
    uint8_t devIdB[8];
}drm_chip_info_t;

/* Function Definitions */
DrmRC DrmAssertParam(
    char* param_string,
    uint32_t value,
    uint32_t max);

/******************************************************************************
 ** FUNCTION
 **   DRM_Common_BasicInitialize
 **
 ** DESCRIPTION:
 **   Initialize the basic Common DRM functionality (no DRM bin file required)
 **
 ** PARAMETERS:
 **   pCommonDrmSettings - Contains a NEXUS_HeapHandle to be used during memory
 **                        allocation operations
 **
 ** RETURNS:
 **   Drm_Success or other
 **
 ******************************************************************************/
DrmRC DRM_Common_BasicInitialize(
    DrmCommonInit_t *pCommonDrmSettings);

/******************************************************************************
 ** FUNCTION
 **   DRM_Common_Finalize
 **
 ** DESCRIPTION:
 **   Close the DRM_Common module
 **
 ** PARAMETERS:
 **   void
 **
 ** RETURNS:
 **   Drm_Success or other
 **
 ******************************************************************************/
DrmRC DRM_Common_Finalize(void);

/******************************************************************************
 ** FUNCTION
 **   DRM_Common_AllocKeySlot
 **
 ** DESCRIPTION:
 **   Helper function to allocate a Nexus keyslot and at the same time hide
 **         Nexus details from higher level callers
 **
 ** PARAMETERS:
 **   NEXUS_SecurityEngine securityEngine - security engine for keyslot allocation
 **   NEXUS_KeySlotHandle* keySlotHandle - address of a keyslot handle
 **
 ** RETURNS:
 **   Drm_Success when the operation is successful, Drm_NexusErr error otherwise.
 **
 ******************************************************************************/
DrmRC DRM_Common_AllocKeySlot(NEXUS_SecurityEngine securityEngine,
    NEXUS_KeySlotHandle* keySlotHandle);

/******************************************************************************
 ** FUNCTION
 **   DRM_Common_FreeKeySlot
 **
 ** DESCRIPTION:
 **   The DRM_Common_AllocKeySlot counterpart - see above
 **
 ******************************************************************************/
void DRM_Common_FreeKeySlot(NEXUS_KeySlotHandle keySlotHandle);

/******************************************************************************
 ** FUNCTION
 **   DRM_Common_GetDefaultStructSettings
 **
 ** DESCRIPTION:
 **   Retrieve default settings for the DrmCommonOperationStruct_t structure
 **
 ******************************************************************************/
void DRM_Common_GetDefaultStructSettings(DrmCommonOperationStruct_t *pDrmCommonOpStruct);

/******************************************************************************
 ** FUNCTION
 **   DRM_Common_OperationDma
 **
 ** DESCRIPTION:
 **   Performs both the key loading and DMA operation given the settings of the
 **   'DrmCommonOperationStruct_t' structure passed the function. Unlike
 **   'DRM_Common_KeyConfigOperation' and 'DRM_Common_M2mOperation', this function
 **   should be used in the case where the caller does not care about having access
 **   to the 'NEXUS_KeySlotHandle'.
 **
 ** PARAMETERS:
 **   pDrmCommonOpStruct* [in/out] - Structure containing key source and
 **                                  cryptographic operation to perform
 **
 ** RETURNS:
 **   Drm_Success (Success) or any other error code (Failure)
 **
 ******************************************************************************/
DrmRC DRM_Common_OperationDma(DrmCommonOperationStruct_t *pDrmCommonOpStruct);

/******************************************************************************
 ** FUNCTION
 **   DRM_Common_KeyConfigOperation
 **
 ** DESCRIPTION:
 **   Performs the key loading operation given the settings of the
 **   'DrmCommonOperationStruct_t' structure passed the function. Unlike
 **   'DRM_Common_OperationDma', this function should be used in the
 **   case where the caller would like to have access to the
 **   to the 'NEXUS_KeySlotHandle'.
 **
 ** PARAMETERS:
 **   pDrmCommonOpStruct* [in/out] - Structure containing key source and
 **                                  cryptographic operation to perform
 **
 ** RETURNS:
 **   Drm_Success (Success) or any other error code (Failure)
 **
 ******************************************************************************/
DrmRC DRM_Common_KeyConfigOperation(DrmCommonOperationStruct_t *pDrmCommonOpStruct);

/******************************************************************************
 ** FUNCTION
 **   DRM_Common_M2mOperation
 **
 ** DESCRIPTION:
 **   Performs the DMA M2M operation given the settings of the
 **   'DrmCommonOperationStruct_t' structure passed the function. Unlike
 **   'DRM_Common_OperationDma', this function should be used in the
 **   case where the caller would like to have access to the
 **   to the 'NEXUS_KeySlotHandle'.
 **
 **   Note: The keyhandle is inside DrmCommonOpStruct.
 **
 ** PARAMETERS:
 **   pDrmCommonOpStruct* [in/out] - Structure containing key source and
 **                                  cryptographic operation to perform
 **
 ** RETURNS:
 **   Drm_Success (Success) or any other error code (Failure)
 **
 ******************************************************************************/
DrmRC DRM_Common_M2mOperation(DrmCommonOperationStruct_t *pDrmCommonOpStruct);

/******************************************************************************
 ** FUNCTION
 **   DRM_Common_GenerateRandomNumber
 **
 ** DESCRIPTION:
 **   Generate a number of random bytes and return them to the caller
 **
 ** PARAMETERS:
 **   numberOfBytes[in] - Number of bytes to generate.  Hardware limitation is 360.
 **   pBuffer[in/out]   - Pointer to buffer that will contain the random bytes.
 **                       Must be large enough to contain numberOfBytes of data.
 **
 ** RETURNS:
 **   Drm_Success when the operation is successful or an error
 **
 ******************************************************************************/
DrmRC DRM_Common_GenerateRandomNumber(
    uint32_t numberOfBytes,
    uint8_t *pBuffer);

/******************************************************************************
 ** FUNCTION
 **   DRM_Common_MemoryAllocate
 **
 ** DESCRIPTION:
 **   Allocates an aligned contiguous block of memory
 **
 **   Example:
 **   uint8_t *pBuf = NULL;
 **   DRM_Common_MemoryAllocate(&pBuf, 32);
 **
 ** PARAMETERS:
 **   pBuffer     - address of a pointer
 **   buffer_size - size of the buffer to allocate
 **
 ** RETURNS:
 **   Drm_Success when the operation is successful or an error
 **
 ******************************************************************************/
DrmRC DRM_Common_MemoryAllocate(
    uint8_t **pBuffer,
    uint32_t buffer_size);

/******************************************************************************
 ** FUNCTION
 **   DRM_Common_MemoryFree
 **
 ** DESCRIPTION:
 **   Free a block of memory
 **
 ** PARAMETERS:
 **   pBuffer - address of memory to free
 **
 ** RETURNS:
 **   Drm_Success when the operation is successful or an error
 **
 ******************************************************************************/
DrmRC DRM_Common_MemoryFree(
    uint8_t *pBuffer);

/**********************************************************
** FUNCTION:
**  DRM_Common_AttachPidChannel
**
** DESCRIPTION:
**   Attach a pid channel to the content key
**
** RETURN:
** 	Drm_Success or other
***********************************************************/

#if (NEXUS_SECURITY_API_VERSION==1)
DrmRC DRM_Common_AttachPidChannel(
    NEXUS_KeySlotHandle keySlot,
    uint32_t pidChannel
);
#else
DrmRC DRM_Common_AttachPidChannel(
    NEXUS_KeySlotHandle keySlot,
    NEXUS_PidChannelHandle pidChannel);
#endif


/******************************************************************************
** FUNCTION:
**  DRM_Common_DetachPidChannel
**
** DESCRIPTION:
**  Detach the pid channel from the content key keyslot
******************************************************************************/
#if (NEXUS_SECURITY_API_VERSION==1)
DrmRC DRM_Common_DetachPidChannel(
    NEXUS_KeySlotHandle keySlot,
    uint32_t pidChannel
);
#else
DrmRC DRM_Common_DetachPidChannel(
    NEXUS_KeySlotHandle keySlot,
    NEXUS_PidChannelHandle pidChannel);
#endif

/******************************************************************************
** FUNCTION:
**   DRM_Common_FetchDeviceIds
**
** DESCRIPTION:
**   Retrieve the OTP IDs
**
** RETURNS:
**   Success -- Drm_Success
**   Failure -- Other
**
******************************************************************************/
DrmRC DRM_Common_FetchDeviceIds(
    drm_chip_info_t *pStruct);

#ifdef __cplusplus
}
#endif

#endif /*DRM_COMMON_H_*/
