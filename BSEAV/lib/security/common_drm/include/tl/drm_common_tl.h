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
#ifndef __DRM_COMMON_H__
#define __DRM_COMMON_H__

#include <openssl/x509.h>

#include "bstd.h"
#include "bdbg.h"
#include "bkni.h"

#include "bsagelib_types.h"
#include "sage_srai.h"

#include "drm_types.h"
#include "common_crypto.h"
#include "drm_common.h"
#include "drm_common_module_ids.h"

#ifdef __cplusplus
extern "C" {
#endif


#define COMMON_DRM_TL_VERSION "CMNDRM_TL 1.6"

#define DRM_MSG_PRINT_BUF(buffer_name, buffer_ptr, buffer_size)                                     \
do {                                                                                                \
    uint32_t _word_no_ = 0;                                                                         \
    uint32_t _word_count_ = 0;                                                                      \
    uint32_t _unaligned_data_size_ = 0;                                                             \
                                                                                                    \
    _unaligned_data_size_ = (buffer_size % 4);                                                      \
    _word_count_ = (buffer_size - _unaligned_data_size_) >> 2;                                      \
                                                                                                    \
    BDBG_MSG(("%s: %s: ", __FUNCTION__, buffer_name));                                              \
    for(_word_no_ = 0; _word_no_ < _word_count_; _word_no_++)                                       \
    {                                                                                               \
        BDBG_MSG(("%02x %02x %02x %02x", ((uint8_t *)(buffer_ptr))[4 * _word_no_],                  \
                                         ((uint8_t *)(buffer_ptr))[4 * _word_no_ + 1],              \
                                         ((uint8_t *)(buffer_ptr))[4 * _word_no_ + 2],              \
                                         ((uint8_t *)(buffer_ptr))[4 * _word_no_ + 3]));            \
    }                                                                                               \
                                                                                                    \
    if(_unaligned_data_size_ == 1)                                                                  \
    {                                                                                               \
        BDBG_MSG(("%02x", ((uint8_t *)(buffer_ptr))[4 * _word_count_]));                            \
    }                                                                                               \
    else if(_unaligned_data_size_ == 2)                                                             \
    {                                                                                               \
        BDBG_MSG(("%02x %02x", ((uint8_t *)(buffer_ptr))[4 * _word_count_],                         \
                               ((uint8_t *)(buffer_ptr))[4 * _word_count_ + 1]));                   \
    }                                                                                               \
    else if(_unaligned_data_size_ == 3)                                                             \
    {                                                                                               \
        BDBG_MSG(("%02x %02x %02x", ((uint8_t *)(buffer_ptr))[4 * _word_count_],                    \
                                    ((uint8_t *)(buffer_ptr))[4 * _word_count_ + 1],                \
                                    ((uint8_t *)(buffer_ptr))[4 * _word_count_ + 2]));              \
    }                                                                                               \
                                                                                                    \
    BDBG_MSG(("\n"));                                                                               \
} while(0)

#define GET_UINT32_FROM_BUF(pBuf) \
    (((uint32_t)(((uint8_t*)(pBuf))[0]) << 24) | \
     ((uint32_t)(((uint8_t*)(pBuf))[1]) << 16) | \
     ((uint32_t)(((uint8_t*)(pBuf))[2]) << 8)  | \
     ((uint8_t *)(pBuf))[3])

/* Number of DMA blocks supported */
#define DRM_COMMON_TL_MAX_DMA_BLOCKS 32

/* Chipset type */
typedef enum
{
    ChipType_eZS = 0,
    ChipType_eZB,
    ChipType_eCustomer,
    ChipType_eCustomer1
} ChipType_e;


/******************************************************************************
** FUNCTION
**   DRM_Common_Initialize
**
** DESCRIPTION:
**    Initializes the common DRM interface module
**
** PARAMETERS:
**    N/A
**
** RETURNS:
**   Drm_Success when the operation is successful or an error.
**
******************************************************************************/
DrmRC DRM_Common_TL_Initialize(DrmCommonInit_t *pCommonTLSettings);

DrmRC DRM_Common_TL_Finalize(void);

DrmRC DRM_Common_TL_ModuleInitialize(DrmCommon_ModuleId_e module_id,
                                 char * drm_bin_filename,
                                 BSAGElib_InOutContainer *container,
							     SRAI_ModuleHandle *moduleHandle);

DrmRC DRM_Common_TL_ModuleFinalize(SRAI_ModuleHandle moduleHandle);

/******************************************************************************
 * * FUNCTION
 **   DRM_Common_AllocKeySlot
 **
 ** DESCRIPTION:
 **    Helper function to allocate a Nexus key slot (and at the same time hide
 **         Nexus details from higher level callers.
 **
 ** PARAMETERS:
 **    NEXUS_KeySlotHandle* keySlotHandle - address of a key slot handle
 **                                         
 ** RETURNS:
 **   Drm_Success when the operation is successful, Drm_NexusErr error otherwise.
 **
 ******************************************************************************/
DrmRC DRM_Common_TL_AllocKeySlot(NEXUS_SecurityEngine securityEngine,
                              NEXUS_KeySlotHandle* keySlotHandle);

/******************************************************************************
 * * FUNCTION
 **   DRM_Common_FreeKeySlot
 **
 ** DESCRIPTION:
 **    The DRM_Common_AllocKeySlot counterpart - see above
 **
 ******************************************************************************/
void DRM_Common_TL_FreeKeySlot(NEXUS_KeySlotHandle keySlotHandle);


/******************************************************************************
 * * FUNCTION
 **   DRM_Common_P_GetFileSize
 **
 ** DESCRIPTION:
 **    Fetch the file size on the rootfs
 **
 ******************************************************************************/
DrmRC DRM_Common_P_GetFileSize(char * filename, uint32_t *filesize);


/******************************************************************************
 * * FUNCTION
 **   DRM_Common_TL_M2mOperation

 **
 ** DESCRIPTION:
 **    Performs the DMA M2M operation given the settings of the
 **    'DrmCommonOperationStruct_t' structure passed the function.
 **    This function controls the flush operation by another given parameter.
 **
 ** PARAMETERS:
 **    DrmCommonOperationStruct_t *pDrmCommonOpStruct - structure containing key
 **     source and cryptographic operation to perform
 **    bool bSkipCacheFlush - whether to skip NEXUS_FlushCache
 **    bool bExternalIV - whether first dma block contains BTP external IV data
 **
 ** RETURNS:
 **   Drm_Success when the operation is successful, Drm_NexusErr error otherwise.
 **
 ******************************************************************************/
DrmRC DRM_Common_TL_M2mOperation(DrmCommonOperationStruct_t *pDrmCommonOpStruct,
    bool bSkipCacheFlush,
    bool bExternalIV );


/******************************************************************************
 * * FUNCTION
 **   DRM_Common_P_TA_Install
 **
 ** DESCRIPTION:
 **    Install TA
 **
 ******************************************************************************/
DrmRC DRM_Common_P_TA_Install(char * taBinFileName);

/******************************************************************************
 * * FUNCTION
 **   DRM_Common_GetChipType
 **
 ** DESCRIPTION:
 **    Get the Chip Type
 **
 ******************************************************************************/
ChipType_e DRM_Common_GetChipType();


#ifdef __cplusplus
}
#endif

#endif /*DRM_COMMON_H_*/
