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
#include "bdbg.h"
#include "bkni.h"

#include "drm_playback_tl.h"
#include "drm_common_tl.h"
#include "drm_common.h"
#include "drm_data.h"

#include "nexus_memory.h"
#include "nexus_parser_band.h"
#include "nexus_pid_channel.h"
#include "bsagelib_types.h"
#include "sage_srai.h"

BDBG_MODULE(playback_tl);

typedef enum Playback_CommandId_e
{
    PLAYBACK_STOPPED = 0,
    PLAYBACK_COMMANDID_MAX
}Playback_CommandId_e;

typedef struct DrmPlaybackContext_s
{
    SRAI_ModuleHandle        playbackSageHandle;
}DrmPlaybackContext_t;

DrmRC DRM_Playback_Initialize(DrmPlaybackHandle_t  *playbackHandle)
{

    NEXUS_MemoryAllocationSettings  allocSettings;
    NEXUS_Error                     nrc = NEXUS_SUCCESS;
    DrmRC                           rc = Drm_Success;
    BSAGElib_InOutContainer        *container = NULL;
    DrmPlaybackContext_t           *pContext=NULL;

    BDBG_MSG(("%s - Entered function", __FUNCTION__));

    NEXUS_Memory_GetDefaultAllocationSettings(&allocSettings);
    nrc = NEXUS_Memory_Allocate(sizeof(DrmPlaybackContext_t), &allocSettings, (void *)&pContext);
    if(nrc != NEXUS_SUCCESS) {
        BDBG_ERR(("%s - NEXUS_Memory_Allocate failed for Playback handle, rc = %d\n", __FUNCTION__, nrc));
        (void)BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        rc = nrc;
        goto ErrorExit;
    }
    if ( NULL == pContext ) {
        (void)BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto ErrorExit;
    }
    BKNI_Memset(pContext, 0, sizeof(DrmPlaybackContext_t));

    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        BDBG_ERR(("%s - Error to allocate SRAI Container", __FUNCTION__));
        rc = Drm_Err;
        goto ErrorExit;
    }

    rc = DRM_Common_TL_ModuleInitialize(DrmCommon_ModuleId_ePlayback, NULL, container, &(pContext->playbackSageHandle));
    if(rc != Drm_Success)
    {
        BDBG_ERR(("%s - Error initializing module (0x%08x)", __FUNCTION__, container->basicOut[0]));
        goto ErrorExit;
    }

    *playbackHandle = (DrmPlaybackHandle_t) pContext;

    if(container) SRAI_Container_Free(container);

    return Drm_Success;

ErrorExit:
    if(container)SRAI_Container_Free(container);
    if( pContext != NULL) NEXUS_Memory_Free(pContext);
    BDBG_MSG(("%s - Exiting function", __FUNCTION__));
    return rc;
}

DrmRC DRM_Playback_Finalize(DrmPlaybackHandle_t pHandle)
{
    DrmPlaybackContext_t *pCtx = (DrmPlaybackContext_t *)pHandle;
    DrmRC rc = Drm_Success;

    if(pCtx != NULL) {
        rc = DRM_Common_TL_ModuleFinalize(pCtx->playbackSageHandle);
        NEXUS_Memory_Free(pCtx);
    }

    return rc;
}

DrmRC DRM_Playback_Stop(DrmPlaybackHandle_t pHandle)
{
    DrmRC rc = Drm_Success;
    BERR_Code sage_rc = BERR_SUCCESS;
    BSAGElib_InOutContainer *container = NULL;
    DrmPlaybackContext_t *pCtx = (DrmPlaybackContext_t *)pHandle;
    NEXUS_SecurityKeySlotSettings keySlotSettings;
    NEXUS_SecurityKeySlotInfo  keyslotInfo;
    NEXUS_KeySlotHandle scrubbingKeyHandle = 0;
    uint8_t *pDmaMemoryPool = NULL;
    unsigned size = 4*1024;

    if(pCtx == NULL){
        rc = Drm_Err;
        goto ErrorExit;
    }

    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        rc = Drm_MemErr;
        BDBG_ERR(("%s - Error allocating SRAI container", __FUNCTION__));
        goto ErrorExit;
    }

    NEXUS_Security_GetDefaultKeySlotSettings(&keySlotSettings);
    keySlotSettings.keySlotEngine = NEXUS_SecurityEngine_eM2m;
    keySlotSettings.client = NEXUS_SecurityClientType_eSage;
    scrubbingKeyHandle = NEXUS_Security_AllocateKeySlot(&keySlotSettings);
    if(scrubbingKeyHandle == NULL)
    {
        rc = Drm_MemErr;
        BDBG_ERR(("NEXUS_Security_AllocateKeySlot() failure"));
        goto ErrorExit;
    }

    NEXUS_Security_GetKeySlotInfo(scrubbingKeyHandle, &keyslotInfo);
    container->basicIn[0] = (int32_t)keyslotInfo.keySlotNumber;
    BDBG_MSG(("%s - keyslotInfo.keySlotNumber %d\n", __FUNCTION__, keyslotInfo.keySlotNumber));

    /* Allocate enough memory for DMA descriptors */
    pDmaMemoryPool = SRAI_Memory_Allocate(size, SRAI_MemoryType_SagePrivate);
    if(pDmaMemoryPool == NULL)
    {
        rc = Drm_MemErr;
        BDBG_ERR(("%s - Error calling SRAI_Memory_Allocate()", __FUNCTION__));
        goto ErrorExit;
    }
    container->blocks[0].data.ptr  = pDmaMemoryPool;
    container->blocks[0].len = size;

    sage_rc = SRAI_Module_ProcessCommand(pCtx->playbackSageHandle, PLAYBACK_STOPPED, container);
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error sending command to SAGE", __FUNCTION__));
        rc = Drm_Err;
        goto ErrorExit;
    }

    sage_rc = container->basicOut[0];
    if(sage_rc != BERR_SUCCESS)
    {
        rc = Drm_Err;
        BDBG_ERR(("%s - Command sent to SAGE successfully but error fetching decryption certificate", __FUNCTION__));
        goto ErrorExit;
    }

ErrorExit:
    if(pDmaMemoryPool) SRAI_Memory_Free(pDmaMemoryPool);
    if(scrubbingKeyHandle) NEXUS_Security_FreeKeySlot(scrubbingKeyHandle);
    if(container) SRAI_Container_Free(container);
    return rc;
}
