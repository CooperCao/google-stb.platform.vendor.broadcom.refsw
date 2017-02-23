/***************************************************************************
 *  Copyright (C) 2007-2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *
 **************************************************************************/
#include "nexus_video_decoder_module.h"
#include "nexus_still_decoder_impl.h"
BDBG_MODULE(nexus_video_decoder_dispatch);

static void NEXUS_VideoDecoder_P_Finalizer( NEXUS_VideoDecoderHandle handle)
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_VideoDecoder);
    BDBG_ASSERT(handle->intf);
    if(handle->intf->Close) {
        handle->intf->Close(handle);
    } else {
        (void)BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }
}

static void NEXUS_VideoDecoder_P_Release(NEXUS_VideoDecoderHandle videoDecoder)
{
    BDBG_OBJECT_ASSERT(videoDecoder, NEXUS_VideoDecoder);
    NEXUS_OBJECT_UNREGISTER(NEXUS_VideoInput, &videoDecoder->input, Close);
}

NEXUS_OBJECT_CLASS_MAKE_WITH_RELEASE(NEXUS_VideoDecoder, NEXUS_VideoDecoder_Close);

void NEXUS_VideoDecoder_GetOpenSettings( NEXUS_VideoDecoderHandle handle, NEXUS_VideoDecoderOpenSettings *pOpenSettings)
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_VideoDecoder);
    BDBG_ASSERT(handle->intf);
    if(handle->intf->GetOpenSettings) {
        handle->intf->GetOpenSettings(handle, pOpenSettings);
    } else {
        (void)BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }
}

void NEXUS_VideoDecoder_GetSettings( NEXUS_VideoDecoderHandle handle, NEXUS_VideoDecoderSettings *pSettings )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_VideoDecoder);
    BDBG_ASSERT(handle->intf);
    if(handle->intf->GetSettings) {
        handle->intf->GetSettings(handle, pSettings);
    } else {
        (void)BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }
}

NEXUS_Error NEXUS_VideoDecoder_SetSettings( NEXUS_VideoDecoderHandle handle, const NEXUS_VideoDecoderSettings *pSettings)
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_VideoDecoder);
    BDBG_ASSERT(handle->intf);
    if(handle->intf->SetSettings) {
        return handle->intf->SetSettings(handle, pSettings);
    } else {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

NEXUS_Error NEXUS_VideoDecoder_Start( NEXUS_VideoDecoderHandle handle, const NEXUS_VideoDecoderStartSettings *pSettings)
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_VideoDecoder);
    BDBG_ASSERT(handle->intf);
    if(handle->intf->Start) {
        return handle->intf->Start( handle, pSettings);
    } else {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

void NEXUS_VideoDecoder_Stop( NEXUS_VideoDecoderHandle handle)
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_VideoDecoder);
    BDBG_ASSERT(handle->intf);
    if(handle->intf->Stop) {
        handle->intf->Stop( handle);
    } else {
        (void)BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }
}

void NEXUS_VideoDecoder_Flush( NEXUS_VideoDecoderHandle handle)
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_VideoDecoder);
    BDBG_ASSERT(handle->intf);
    if(handle->intf->Flush) {
        handle->intf->Flush(handle);
    } else {
        (void)BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

NEXUS_Error NEXUS_VideoDecoder_GetStatus( NEXUS_VideoDecoderHandle handle, NEXUS_VideoDecoderStatus *pStatus)
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_VideoDecoder);
    BDBG_ASSERT(handle->intf);
    if(handle->intf->GetStatus) {
        return handle->intf->GetStatus(handle, pStatus);
    } else {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

NEXUS_VideoInput NEXUS_VideoDecoder_GetConnector(NEXUS_VideoDecoderHandle handle)
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_VideoDecoder);
    BDBG_ASSERT(handle->intf);
    if(handle->intf->GetConnector) {
        return handle->intf->GetConnector(handle);
    } else {
        (void)BERR_TRACE(BERR_NOT_SUPPORTED);
        return NULL;
    }
}

NEXUS_Error NEXUS_VideoDecoder_GetStreamInformation(NEXUS_VideoDecoderHandle handle, NEXUS_VideoDecoderStreamInformation *pStreamInformation)
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_VideoDecoder);
    BDBG_ASSERT(handle->intf);
    if(handle->intf->GetStreamInformation) {
       return handle->intf->GetStreamInformation(handle, pStreamInformation);
    } else {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

NEXUS_Error NEXUS_VideoDecoder_SetStartPts( NEXUS_VideoDecoderHandle handle, uint32_t pts)
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_VideoDecoder);
    BDBG_ASSERT(handle->intf);
    if(handle->intf->SetStartPts) {
       return handle->intf->SetStartPts( handle, pts);
    } else {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

void NEXUS_VideoDecoder_IsCodecSupported( NEXUS_VideoDecoderHandle handle, NEXUS_VideoCodec codec, bool *pSupported)
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_VideoDecoder);
    BDBG_ASSERT(handle->intf);
    if(handle->intf->IsCodecSupported) {
        handle->intf->IsCodecSupported( handle, codec, pSupported);
    } else {
        *pSupported = false;
        (void)BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }
}

NEXUS_Error NEXUS_VideoDecoder_SetPowerState( NEXUS_VideoDecoderHandle handle, bool powerUp)
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_VideoDecoder);
    BDBG_ASSERT(handle->intf);
    if(handle->intf->SetPowerState ) {
        return handle->intf->SetPowerState( handle, powerUp);
    } else {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}


void NEXUS_VideoDecoder_Reset( NEXUS_VideoDecoderHandle handle)
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_VideoDecoder);
    BDBG_ASSERT(handle->intf);
    if(handle->intf->Reset) {
        handle->intf->Reset(handle);
    } else {
        (void)BERR_TRACE(BERR_NOT_SUPPORTED);
        return;
    }
}

NEXUS_Error NEXUS_VideoDecoder_GetExtendedStatus( NEXUS_VideoDecoderHandle handle, NEXUS_VideoDecoderExtendedStatus *pStatus)
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_VideoDecoder);
    BDBG_ASSERT(handle->intf);
    if(handle->intf->GetExtendedStatus) {
        return handle->intf->GetExtendedStatus( handle, pStatus);
    } else {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

void NEXUS_VideoDecoder_GetExtendedSettings( NEXUS_VideoDecoderHandle handle, NEXUS_VideoDecoderExtendedSettings *pSettings)
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_VideoDecoder);
    BDBG_ASSERT(handle->intf);
    if(handle->intf->GetExtendedSettings) {
        handle->intf->GetExtendedSettings( handle, pSettings);
    } else {
        (void)BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

NEXUS_Error NEXUS_VideoDecoder_SetExtendedSettings( NEXUS_VideoDecoderHandle handle, const NEXUS_VideoDecoderExtendedSettings *pSettings)
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_VideoDecoder);
    BDBG_ASSERT(handle->intf);
    if(handle->intf->SetExtendedSettings) {
        return handle->intf->SetExtendedSettings( handle, pSettings);
    } else {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

NEXUS_StripedSurfaceHandle NEXUS_VideoDecoder_CreateStripedSurface( NEXUS_VideoDecoderHandle handle)
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_VideoDecoder);
    BDBG_ASSERT(handle->intf);
    if(handle->intf->CreateStripedSurface) {
        return handle->intf->CreateStripedSurface(handle);
    } else {
        (void)BERR_TRACE(BERR_NOT_SUPPORTED);
        return NULL;
    }
}

void NEXUS_VideoDecoder_DestroyStripedSurface( NEXUS_VideoDecoderHandle handle, NEXUS_StripedSurfaceHandle stripedSurface)
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_VideoDecoder);
    BDBG_ASSERT(handle->intf);
    if(handle->intf->DestroyStripedSurface) {
        handle->intf->DestroyStripedSurface( handle, stripedSurface);
    } else {
        (void)BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

NEXUS_Error NEXUS_VideoDecoder_CreateStripedMosaicSurfaces( NEXUS_VideoDecoderHandle handle, NEXUS_StripedSurfaceHandle *pStripedSurfaces, unsigned int maxSurfaces, unsigned int *pSurfaceCount)
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_VideoDecoder);
    BDBG_ASSERT(handle->intf);
    if(handle->intf->CreateStripedMosaicSurfaces) {
        return handle->intf->CreateStripedMosaicSurfaces( handle, pStripedSurfaces, maxSurfaces, pSurfaceCount);
    } else {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

NEXUS_Error NEXUS_VideoDecoder_GetMostRecentPts( NEXUS_VideoDecoderHandle handle, uint32_t *pPts)
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_VideoDecoder);
    BDBG_ASSERT(handle->intf);
    if(handle->intf->GetMostRecentPts) {
        return handle->intf->GetMostRecentPts( handle, pPts);
    } else {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

void NEXUS_VideoDecoder_GetTrickState( NEXUS_VideoDecoderHandle handle, NEXUS_VideoDecoderTrickState *pState)
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_VideoDecoder);
    BDBG_ASSERT(handle->intf);
    if(handle->intf->GetTrickState) {
        handle->intf->GetTrickState( handle, pState);
    } else {
        (void)BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }
}

NEXUS_Error NEXUS_VideoDecoder_SetTrickState( NEXUS_VideoDecoderHandle handle, const NEXUS_VideoDecoderTrickState *pState)
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_VideoDecoder);
    BDBG_ASSERT(handle->intf);
    if(handle->intf->SetTrickState) {
        return handle->intf->SetTrickState( handle, pState);
    } else {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

NEXUS_Error NEXUS_VideoDecoder_FrameAdvance(NEXUS_VideoDecoderHandle handle)
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_VideoDecoder);
    BDBG_ASSERT(handle->intf);
    if(handle->intf->FrameAdvance) {
        return handle->intf->FrameAdvance(handle);
    } else {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

NEXUS_Error NEXUS_VideoDecoder_GetNextPts( NEXUS_VideoDecoderHandle handle, uint32_t *pNextPts)
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_VideoDecoder);
    BDBG_ASSERT(handle->intf);
    if(handle->intf->GetNextPts) {
        return handle->intf->GetNextPts( handle, pNextPts);
    } else {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

void NEXUS_VideoDecoder_GetPlaybackSettings( NEXUS_VideoDecoderHandle handle, NEXUS_VideoDecoderPlaybackSettings *pSettings)
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_VideoDecoder);
    BDBG_ASSERT(handle->intf);
    if(handle->intf->GetPlaybackSettings) {
        handle->intf->GetPlaybackSettings( handle, pSettings);
    } else {
        (void)BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }
}

NEXUS_Error NEXUS_VideoDecoder_SetPlaybackSettings( NEXUS_VideoDecoderHandle handle, const NEXUS_VideoDecoderPlaybackSettings *pSettings)
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_VideoDecoder);
    BDBG_ASSERT(handle->intf);
    if(handle->intf->SetPlaybackSettings) {
        return handle->intf->SetPlaybackSettings( handle, pSettings);
    } else {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

void NEXUS_VideoDecoder_GetDisplayConnection_priv( NEXUS_VideoDecoderHandle handle, NEXUS_VideoDecoderDisplayConnection *pConnection)
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_VideoDecoder);
    BDBG_ASSERT(handle->intf);
    if(handle->intf->GetDisplayConnection_priv) {
        handle->intf->GetDisplayConnection_priv( handle, pConnection);
    } else {
        (void)BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }
}

NEXUS_Error NEXUS_VideoDecoder_SetDisplayConnection_priv( NEXUS_VideoDecoderHandle handle, const NEXUS_VideoDecoderDisplayConnection *pConnection)
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_VideoDecoder);
    BDBG_ASSERT(handle->intf);
    if(handle->intf->SetDisplayConnection_priv) {
        return handle->intf->SetDisplayConnection_priv( handle, pConnection);
    } else {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

void NEXUS_VideoDecoder_GetSourceId_priv( NEXUS_VideoDecoderHandle handle, BAVC_SourceId *pSource)
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_VideoDecoder);
    BDBG_ASSERT(handle->intf);
    if(handle->intf->GetSourceId_priv) {
        handle->intf->GetSourceId_priv( handle, pSource);
    } else {
        (void)BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }
}

void NEXUS_VideoDecoder_GetHeap_priv( NEXUS_VideoDecoderHandle handle, NEXUS_HeapHandle *pHeap)
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_VideoDecoder);
    BDBG_ASSERT(handle->intf);
    if(handle->intf->GetHeap_priv) {
        handle->intf->GetHeap_priv( handle, pHeap);
    } else {
        (void)BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }
}

#if NEXUS_HAS_SYNC_CHANNEL
void NEXUS_VideoDecoder_GetSyncSettings_priv(NEXUS_VideoDecoderHandle handle, NEXUS_VideoInputSyncSettings *pSyncSettings)
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_VideoDecoder);
    BDBG_ASSERT(handle->intf);
    if(handle->intf->GetSyncSettings_priv) {
        handle->intf->GetSyncSettings_priv( handle, pSyncSettings);
    } else {
        (void)BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }
}

NEXUS_Error NEXUS_VideoDecoder_SetSyncSettings_priv(NEXUS_VideoDecoderHandle handle, const NEXUS_VideoInputSyncSettings *pSyncSettings)
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_VideoDecoder);
    BDBG_ASSERT(handle->intf);
    if(handle->intf->SetSyncSettings_priv) {
        return handle->intf->SetSyncSettings_priv( handle, pSyncSettings);
    } else {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

NEXUS_Error NEXUS_VideoDecoder_GetSyncStatus_isr(NEXUS_VideoDecoderHandle handle, NEXUS_VideoInputSyncStatus *pSyncStatus )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_VideoDecoder);
    BDBG_ASSERT(handle->intf);
    if(handle->intf->GetSyncStatus_isr) {
        return handle->intf->GetSyncStatus_isr( handle, pSyncStatus);
    } else {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}
#endif

void NEXUS_VideoDecoder_UpdateDisplayInformation_priv( NEXUS_VideoDecoderHandle handle, const NEXUS_VideoDecoder_DisplayInformation *displayInformation)
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_VideoDecoder);
    BDBG_ASSERT(handle->intf);
    if(handle->intf->UpdateDisplayInformation_priv) {
        handle->intf->UpdateDisplayInformation_priv( handle, displayInformation);
    } else {
        (void)BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }
}

#if NEXUS_HAS_ASTM
void NEXUS_VideoDecoder_GetAstmSettings_priv( NEXUS_VideoDecoderHandle handle, NEXUS_VideoDecoderAstmSettings * pAstmSettings)
{
    BDBG_ASSERT(handle->intf);
    if(handle->intf->GetAstmSettings_priv) {
        handle->intf->GetAstmSettings_priv( handle, pAstmSettings);
    } else {
        (void)BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }
}

NEXUS_Error NEXUS_VideoDecoder_SetAstmSettings_priv(NEXUS_VideoDecoderHandle handle, const NEXUS_VideoDecoderAstmSettings * pAstmSettings)
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_VideoDecoder);
    BDBG_ASSERT(handle->intf);
    if(handle->intf->SetAstmSettings_priv) {
        return handle->intf->SetAstmSettings_priv( handle, pAstmSettings);
    } else {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

NEXUS_Error NEXUS_VideoDecoder_GetAstmStatus_isr( NEXUS_VideoDecoderHandle handle, NEXUS_VideoDecoderAstmStatus * pAstmStatus)
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_VideoDecoder);
    BDBG_ASSERT(handle->intf);
    if(handle->intf->GetAstmStatus_isr) {
        return handle->intf->GetAstmStatus_isr( handle, pAstmStatus);
    } else {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}
#endif /* NEXUS_HAS_ASTM */


NEXUS_StillDecoderHandle NEXUS_StillDecoder_Open( NEXUS_VideoDecoderHandle handle, unsigned index, const NEXUS_StillDecoderOpenSettings *pSettings )
{
#if NEXUS_NUM_STILL_DECODES
    if (!handle) {
        return NEXUS_StillDecoder_P_Open_Avd( NULL, index, pSettings );
    }
#endif
    BDBG_OBJECT_ASSERT(handle, NEXUS_VideoDecoder);
    BDBG_ASSERT(handle->intf);
    if(handle->intf->StillDecoder_Open) {
        return handle->intf->StillDecoder_Open( handle, index, pSettings);
    } else {
        (void)BERR_TRACE(BERR_NOT_SUPPORTED);
        return NULL;
    }
}

static void NEXUS_StillDecoder_P_Finalizer( NEXUS_StillDecoderHandle handle)
{
    BDBG_ASSERT(handle->intf);
    if(handle->intf->Close) {
        handle->intf->Close(handle);
    } else {
        (void)BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    return;
}

NEXUS_OBJECT_CLASS_MAKE(NEXUS_StillDecoder, NEXUS_StillDecoder_Close);

NEXUS_Error NEXUS_StillDecoder_Start( NEXUS_StillDecoderHandle handle, const NEXUS_StillDecoderStartSettings *pSettings)
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_StillDecoder);
    BDBG_ASSERT(handle->intf);
    if(handle->intf->Start) {
        return handle->intf->Start( handle, pSettings);
    } else {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

void NEXUS_StillDecoder_Stop( NEXUS_StillDecoderHandle handle)
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_StillDecoder);
    BDBG_ASSERT(handle->intf);
    if(handle->intf->Stop) {
        handle->intf->Stop(handle);
    } else {
        (void)BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

NEXUS_Error NEXUS_StillDecoder_GetStatus( NEXUS_StillDecoderHandle handle, NEXUS_StillDecoderStatus *pStatus )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_StillDecoder);
    BDBG_ASSERT(handle->intf);
    if(handle->intf->GetStatus) {
        return handle->intf->GetStatus( handle, pStatus );
    } else {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

NEXUS_Error NEXUS_StillDecoder_GetStripedSurface( NEXUS_StillDecoderHandle handle, NEXUS_StripedSurfaceHandle *pStripedSurface)
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_StillDecoder);
    BDBG_ASSERT(handle->intf);
    if(handle->intf->GetStripedSurface) {
        return handle->intf->GetStripedSurface( handle, pStripedSurface);
    } else {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

void NEXUS_StillDecoder_ReleaseStripedSurface( NEXUS_StillDecoderHandle handle, NEXUS_StripedSurfaceHandle stripedSurface )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_StillDecoder);
    BDBG_ASSERT(handle->intf);
    if(handle->intf->GetStripedSurface) {
        handle->intf->ReleaseStripedSurface( handle, stripedSurface);
    }
}

NEXUS_Error NEXUS_VideoDecoder_GetDecodedFrames(
    NEXUS_VideoDecoderHandle handle,
    NEXUS_VideoDecoderFrameStatus *pStatus,  /* attr{nelem=numEntries;nelem_out=pNumEntriesReturned;null_allowed=y} [out] */
    unsigned numEntries,
    unsigned *pNumEntriesReturned /* [out] */
    )
{
    NEXUS_OBJECT_ASSERT(NEXUS_VideoDecoder, handle);
    BDBG_ASSERT(handle->intf);
    BDBG_ASSERT(NULL != pNumEntriesReturned);
    if ( handle->intf->GetDecodedFrames ) {
        return handle->intf->GetDecodedFrames(handle, pStatus, numEntries, pNumEntriesReturned);
    }
    else {
        *pNumEntriesReturned = 0;
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

NEXUS_Error NEXUS_VideoDecoder_ReturnDecodedFrames(
    NEXUS_VideoDecoderHandle handle,
    const NEXUS_VideoDecoderReturnFrameSettings *pSettings,
    unsigned numFrames
    )
{
    NEXUS_OBJECT_ASSERT(NEXUS_VideoDecoder, handle);
    BDBG_ASSERT(handle->intf);
    if ( handle->intf->ReturnDecodedFrames ) {
        return handle->intf->ReturnDecodedFrames(handle, pSettings, numFrames);
    }
    else {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

NEXUS_Error NEXUS_VideoDecoder_VideoAsGraphicsSupported_priv(
    NEXUS_VideoDecoderHandle handle
    )
{
    if ( handle->intf->GetDecodedFrames ) {
        return NEXUS_SUCCESS;
    }
    else {
        return NEXUS_NOT_SUPPORTED;
    }
}
