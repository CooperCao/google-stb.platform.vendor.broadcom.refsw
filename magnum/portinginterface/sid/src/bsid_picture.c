/******************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/

/*
    BSID Picture Interface for XDM
*/
#include "bstd.h"
#include "bdbg.h"

#include "bsid_picture.h"
#include "bsid_priv.h"
#include "bsid_msg.h"

BDBG_MODULE(BSID_PICTURE);


/******************************************************************************
* Function name: BSID_P_GetPictureCount_isr
*   -
* Comments:
*   -
******************************************************************************/
BERR_Code BSID_P_GetPictureCount_isr(void *pvHandle, uint32_t *puiPictureCount)
{
    BERR_Code retCode = BERR_SUCCESS;
    BSID_ChannelHandle  hSidCh = (BSID_ChannelHandle)pvHandle;

    BDBG_ENTER(BSID_P_GetPictureCount_isr);
    *puiPictureCount = 0;

#ifdef BSID_P_DEBUG_SAVE_BUFFER
    return BERR_TRACE(retCode); /* output from SID goes to file */
#endif

    if (hSidCh->e_ChannelState == BSID_ChannelState_eDecode)
    {
        *puiPictureCount = hSidCh->sPlaybackQueue.ui32_DisplayQueueFullnessCount;

#ifdef BSID_P_DEBUG_TRACE_PLAYBACK
        BKNI_Printf("[0x%08x] pr (%d)\n", hSidCh->stDebugPlaybackTrace.vsync_index, *puiPictureCount);
#endif
    }

    BDBG_ENTER(BSID_P_GetPictureCount_isr);

    return BERR_TRACE(retCode);
}

/******************************************************************************
* Function name: BSID_P_PeekAtPicture_isr
*   -
* Comments:
*   -
******************************************************************************/
BERR_Code BSID_P_PeekAtPicture_isr(void *pvHandle, uint32_t uiIndex, const BXDM_Picture **pUnifiedPicture)
{
    BERR_Code retCode = BERR_SUCCESS;
    BSID_ChannelHandle  hSidCh = (BSID_ChannelHandle)pvHandle;
#ifndef BSID_P_DEBUG_TRACE_PLAYBACK
    BSTD_UNUSED(uiIndex);
#endif

    BDBG_ENTER(BSID_P_PeekAtPicture_isr);

    *pUnifiedPicture = NULL;

    if (hSidCh->e_ChannelState == BSID_ChannelState_eDecode)
    {
        BSID_PlaybackQueueState *psPbStateQueue = (BSID_PlaybackQueueState *)hSidCh->sPlaybackQueue.sStateBuf.pv_CachedAddr;
        BMMA_Block_Handle hPbStateQueueBlock = hSidCh->sPlaybackQueue.sStateBuf.hBlock;
        BSID_PlaybackQueueState *psPbStateQueueEntry;

        BMMA_FlushCache(hPbStateQueueBlock, (void *)psPbStateQueue, sizeof(BSID_PlaybackQueueState));

        psPbStateQueueEntry = &psPbStateQueue[hSidCh->sPlaybackQueue.ui32_DisplayReadIndex];
        BMMA_FlushCache(hPbStateQueueBlock, (void *)psPbStateQueueEntry, sizeof(BSID_PlaybackQueueState));

        if (psPbStateQueueEntry->ps_UnifiedPicture)
        {
            *pUnifiedPicture = psPbStateQueueEntry->ps_UnifiedPicture;
        }

#ifdef BSID_P_DEBUG_TRACE_PLAYBACK
        BKNI_Printf("[0x%08x] pa (%d) %p\n", hSidCh->stDebugPlaybackTrace.vsync_index, uiIndex, *pUnifiedPicture);
#endif
    }

    BDBG_ENTER(BSID_P_PeekAtPicture_isr);

    return BERR_TRACE(retCode);
}

/******************************************************************************
* Function name: BSID_P_GetNextPicture_isr
*   -
* Comments:
*   -
******************************************************************************/
BERR_Code BSID_P_GetNextPicture_isr(void *pvHandle, const BXDM_Picture **pUnifiedPicture)
{
    BERR_Code retCode = BERR_SUCCESS;
    BSID_ChannelHandle  hSidCh = (BSID_ChannelHandle)pvHandle;
    unsigned int ui32_QueueSize = hSidCh->sChSettings.u_ChannelSpecific.motion.ui32_OutputBuffersNumber;

    BDBG_ENTER(BSID_P_GetNextPicture_isr);
    *pUnifiedPicture = NULL;

    if (hSidCh->e_ChannelState == BSID_ChannelState_eDecode)
    {
        BSID_PlaybackQueueState *psPbStateQueue = (BSID_PlaybackQueueState *)hSidCh->sPlaybackQueue.sStateBuf.pv_CachedAddr;
        BMMA_Block_Handle hPbStateQueueBlock = hSidCh->sPlaybackQueue.sStateBuf.hBlock;
        BSID_PlaybackQueueState *psPbStateQueueEntry;

        BMMA_FlushCache(hPbStateQueueBlock, (void *)psPbStateQueue, sizeof(BSID_PlaybackQueueState));

        psPbStateQueueEntry = &psPbStateQueue[hSidCh->sPlaybackQueue.ui32_DisplayReadIndex];
        BMMA_FlushCache(hPbStateQueueBlock, (void *)psPbStateQueueEntry, sizeof(BSID_PlaybackQueueState));

        if (psPbStateQueueEntry->ps_UnifiedPicture)
        {
            *pUnifiedPicture = ((BXDM_Picture *)psPbStateQueueEntry->ps_UnifiedPicture);
            psPbStateQueueEntry->ui32_OutputState = BSID_OutputBufferState_eDisplay;
            hSidCh->sPlaybackQueue.ui32_DisplayQueueFullnessCount--;

            BDBG_MSG(("next rdi=%d uni=%p pts = 0x%x",
            hSidCh->sPlaybackQueue.ui32_DisplayReadIndex,
            (void *)(psPbStateQueueEntry->ps_UnifiedPicture),
            psPbStateQueueEntry->ps_UnifiedPicture->stPTS.uiValue ));

            BSID_INCR_INDEX(hSidCh->sPlaybackQueue.ui32_DisplayReadIndex, ui32_QueueSize);
        }
    }

    BDBG_ENTER(BSID_P_GetNextPicture_isr);

    return BERR_TRACE(retCode);
}

/******************************************************************************
* Function name: BSID_P_ReleasePicture_isr
*   -
* Comments:
*   -
******************************************************************************/
BERR_Code BSID_P_ReleasePicture_isr(void *pvHandle, const BXDM_Picture *pUnifiedPicture, const BXDM_Decoder_ReleasePictureInfo *pReleasePictureInfo)
{
    BERR_Code retCode = BERR_SUCCESS;
    BSID_ChannelHandle  hSidCh = (BSID_ChannelHandle)pvHandle;

    BSTD_UNUSED(pReleasePictureInfo);

    BDBG_ENTER(BSID_P_ReleasePicture_isr);

    if (hSidCh->e_ChannelState == BSID_ChannelState_eDecode)
    {
        unsigned int ui32_QueueSize = hSidCh->sChSettings.u_ChannelSpecific.motion.ui32_OutputBuffersNumber;
        unsigned int bufferIndex = 0;

        BSID_PlaybackQueueState *psPbStateQueue = (BSID_PlaybackQueueState *)hSidCh->sPlaybackQueue.sStateBuf.pv_CachedAddr;
        BMMA_Block_Handle hPbStateQueueBlock = hSidCh->sPlaybackQueue.sStateBuf.hBlock;
        BSID_PlaybackQueueState *psPbStateQueueEntry;

        for (bufferIndex = 0; bufferIndex < ui32_QueueSize; bufferIndex++)
        {
            psPbStateQueueEntry = &psPbStateQueue[bufferIndex];
            BMMA_FlushCache(hPbStateQueueBlock, (void *)psPbStateQueueEntry, sizeof(BSID_PlaybackQueueState));

            if (psPbStateQueueEntry->ps_UnifiedPicture == pUnifiedPicture)
            {
                psPbStateQueueEntry->ps_UnifiedPicture = NULL;
                psPbStateQueueEntry->ui32_OutputState = BSID_OutputBufferState_eIdle;
                break;
            }
        }
    }

    BDBG_ENTER(BSID_P_ReleasePicture_isr);

    return BERR_TRACE(retCode);
}


/******************************************************************************
* Function name: BSID_P_DisplayInterruptEvent_isr
*   -
* Comments:
*   -
******************************************************************************/
BERR_Code BSID_P_DisplayInterruptEvent_isr(void *pvHandle)
{
    BERR_Code retCode = BERR_SUCCESS;
    BSID_ChannelHandle  hSidCh = (BSID_ChannelHandle)pvHandle;
#ifdef BSID_P_DEBUG_SAVE_BUFFER
    BSTD_UNUSED(hSidCh);
#endif

#ifdef BSID_P_DEBUG_TRACE_PLAYBACK
    hSidCh->stDebugPlaybackTrace.vsync_index++;
#endif

    BDBG_ENTER(BSID_P_DisplayInterruptEvent_isr);
#ifndef BSID_P_DEBUG_SAVE_BUFFER
    /* If we're saving the picture buffer, then this is called from a background thread instead */
    BSID_P_UpdatePlaybackInfo_isr(hSidCh);
#endif
    BDBG_ENTER(BSID_P_DisplayInterruptEvent_isr);

    return BERR_TRACE(retCode);
}

#if !B_REFSW_MINIMAL
/* not used for now */
/******************************************************************************
* Function name: BSID_P_GetPictureDropPendingCount_isr
*   -
* Comments:
*   -
******************************************************************************/
BERR_Code BSID_P_GetPictureDropPendingCount_isr(void *pvHandle, uint32_t *puiPictureDropPendingCount)
{
    BERR_Code retCode = BERR_SUCCESS;
    BSID_ChannelHandle  hSidCh = (BSID_ChannelHandle)pvHandle;

    BSTD_UNUSED(puiPictureDropPendingCount);
    BSTD_UNUSED(hSidCh);

    BDBG_ENTER(BSID_P_GetPictureDropPendingCount_isr);

    BDBG_ENTER(BSID_P_GetPictureDropPendingCount_isr);

    return BERR_TRACE(retCode);
}

/******************************************************************************
* Function name: BSID_P_RequestPictureDrop_isr
*   -
* Comments:
*   -
******************************************************************************/
BERR_Code BSID_P_RequestPictureDrop_isr(void *pvHandle, uint32_t *puiPictureDropRequestCount)
{
    BERR_Code retCode = BERR_SUCCESS;
    BSID_ChannelHandle  hSidCh = (BSID_ChannelHandle)pvHandle;

    BSTD_UNUSED(puiPictureDropRequestCount);
    BSTD_UNUSED(hSidCh);

    BDBG_ENTER(BSID_P_RequestPictureDrop_isr);

    BDBG_ENTER(BSID_P_RequestPictureDrop_isr);

    return BERR_TRACE(retCode);
}
#endif


/*
  End of File
*/
