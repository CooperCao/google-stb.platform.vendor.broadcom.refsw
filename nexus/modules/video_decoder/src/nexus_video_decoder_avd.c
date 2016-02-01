/***************************************************************************
 *     (c)2007-2013 Broadcom Corporation
 *
 *  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 **************************************************************************/
#include "nexus_video_decoder_module.h"
#include "nexus_still_decoder_impl.h"


NEXUS_VideoDecoderHandle NEXUS_VideoDecoder_Open( unsigned index, const NEXUS_VideoDecoderOpenSettings *pOpenSettings)
{
    return NEXUS_VideoDecoder_P_Open_Avd(index, pOpenSettings);
}

static void NEXUS_VideoDecoder_P_Finalizer( NEXUS_VideoDecoderHandle handle)
{
    NEXUS_VideoDecoder_P_Close_Avd(handle);
    return;
}

static void NEXUS_VideoDecoder_P_Release(NEXUS_VideoDecoderHandle videoDecoder)
{
    BDBG_OBJECT_ASSERT(videoDecoder, NEXUS_VideoDecoder);
    NEXUS_OBJECT_UNREGISTER(NEXUS_VideoInput, &videoDecoder->input, Close);
}

NEXUS_OBJECT_CLASS_MAKE_WITH_RELEASE(NEXUS_VideoDecoder, NEXUS_VideoDecoder_Close);

void NEXUS_VideoDecoder_GetOpenSettings( NEXUS_VideoDecoderHandle handle, NEXUS_VideoDecoderOpenSettings *pOpenSettings)
{
    NEXUS_VideoDecoder_P_GetOpenSettings_Common(handle, pOpenSettings);
}

void NEXUS_VideoDecoder_GetSettings( NEXUS_VideoDecoderHandle handle, NEXUS_VideoDecoderSettings *pSettings )
{
    NEXUS_VideoDecoder_P_GetSettings_Common(handle, pSettings);
}

NEXUS_Error NEXUS_VideoDecoder_SetSettings( NEXUS_VideoDecoderHandle handle, const NEXUS_VideoDecoderSettings *pSettings)
{
    return NEXUS_VideoDecoder_P_SetSettings_Avd( handle, pSettings);
}

NEXUS_Error NEXUS_VideoDecoder_Start( NEXUS_VideoDecoderHandle handle, const NEXUS_VideoDecoderStartSettings *pSettings)
{
    return NEXUS_VideoDecoder_P_Start_Avd( handle, pSettings);

}
void NEXUS_VideoDecoder_Stop( NEXUS_VideoDecoderHandle handle)
{
    NEXUS_VideoDecoder_P_Stop_Avd( handle);
}

void NEXUS_VideoDecoder_Flush( NEXUS_VideoDecoderHandle handle)
{
    NEXUS_VideoDecoder_P_Flush_Avd(handle);
}

NEXUS_Error NEXUS_VideoDecoder_GetStatus( NEXUS_VideoDecoderHandle handle, NEXUS_VideoDecoderStatus *pStatus)
{
    return NEXUS_VideoDecoder_P_GetStatus_Avd( handle, pStatus);
}

NEXUS_VideoInput NEXUS_VideoDecoder_GetConnector(NEXUS_VideoDecoderHandle handle)
{
    return NEXUS_VideoDecoder_P_GetConnector_Common(handle);
}

NEXUS_Error NEXUS_VideoDecoder_GetStreamInformation(NEXUS_VideoDecoderHandle handle, NEXUS_VideoDecoderStreamInformation *pStreamInformation)
{
    return NEXUS_VideoDecoder_P_GetStreamInformation_Avd(handle, pStreamInformation);
}

NEXUS_Error NEXUS_VideoDecoder_SetStartPts( NEXUS_VideoDecoderHandle handle, uint32_t pts)
{
    return NEXUS_VideoDecoder_P_SetStartPts_Avd( handle, pts);
}

void NEXUS_VideoDecoder_IsCodecSupported( NEXUS_VideoDecoderHandle videoDecoder, NEXUS_VideoCodec codec, bool *pSupported)
{
    NEXUS_VideoDecoder_P_IsCodecSupported_Generic( videoDecoder, codec, pSupported);
}

NEXUS_Error NEXUS_VideoDecoder_SetPowerState( NEXUS_VideoDecoderHandle handle, bool powerUp)
{
    return NEXUS_VideoDecoder_P_SetPowerState_Avd( handle, powerUp);
}

NEXUS_VideoDecoderHandle NEXUS_VideoDecoder_OpenMosaic( unsigned parentIndex, unsigned index, const NEXUS_VideoDecoderOpenMosaicSettings *pOpenSettings )
{
    return NEXUS_VideoDecoder_P_OpenMosaic_Avd( parentIndex, index, pOpenSettings );
}


void NEXUS_VideoDecoder_Reset( NEXUS_VideoDecoderHandle handle)
{
    NEXUS_VideoDecoder_P_Reset_Avd( handle);
}

NEXUS_Error NEXUS_VideoDecoder_GetExtendedStatus( NEXUS_VideoDecoderHandle handle, NEXUS_VideoDecoderExtendedStatus *pStatus)
{
    return NEXUS_VideoDecoder_P_GetExtendedStatus_Avd( handle, pStatus);
}

void NEXUS_VideoDecoder_GetExtendedSettings( NEXUS_VideoDecoderHandle handle, NEXUS_VideoDecoderExtendedSettings *pSettings)
{
    NEXUS_VideoDecoder_P_GetExtendedSettings_Avd( handle, pSettings);
}

NEXUS_Error NEXUS_VideoDecoder_SetExtendedSettings( NEXUS_VideoDecoderHandle handle, const NEXUS_VideoDecoderExtendedSettings *pSettings)
{
    return NEXUS_VideoDecoder_P_SetExtendedSettings_Avd( handle, pSettings);
}

NEXUS_StripedSurfaceHandle NEXUS_VideoDecoder_CreateStripedSurface( NEXUS_VideoDecoderHandle handle)
{
    return NEXUS_VideoDecoder_P_CreateStripedSurface_Avd(handle);
}

void NEXUS_VideoDecoder_DestroyStripedSurface( NEXUS_VideoDecoderHandle videoDecoder, NEXUS_StripedSurfaceHandle stripedSurface)
{
    NEXUS_VideoDecoder_P_DestroyStripedSurface_Avd( videoDecoder, stripedSurface);
}

NEXUS_Error NEXUS_VideoDecoder_CreateStripedMosaicSurfaces( NEXUS_VideoDecoderHandle videoDecoder, NEXUS_StripedSurfaceHandle *pStripedSurfaces, unsigned int maxSurfaces, unsigned int *pSurfaceCount)
{
    return NEXUS_VideoDecoder_P_CreateStripedMosaicSurfaces_Avd( videoDecoder, pStripedSurfaces, maxSurfaces, pSurfaceCount);
}

void NEXUS_VideoDecoder_DestroyStripedMosaicSurfaces( NEXUS_VideoDecoderHandle videoDecoder, const NEXUS_StripedSurfaceHandle *pStripedSurfaces, unsigned int surfaceCount)
{
    NEXUS_VideoDecoder_P_DestroyStripedMosaicSurfaces_Avd( videoDecoder, pStripedSurfaces, surfaceCount);
}

NEXUS_Error NEXUS_VideoDecoder_GetMostRecentPts( NEXUS_VideoDecoderHandle handle, uint32_t *pPts)
{
    return NEXUS_VideoDecoder_P_GetMostRecentPts_Avd( handle, pPts);
}

void NEXUS_VideoDecoder_GetTrickState( NEXUS_VideoDecoderHandle handle, NEXUS_VideoDecoderTrickState *pState)
{
    NEXUS_VideoDecoder_P_GetTrickState_Common( handle, pState);
}

NEXUS_Error NEXUS_VideoDecoder_SetTrickState( NEXUS_VideoDecoderHandle handle, const NEXUS_VideoDecoderTrickState *pState)
{
    return NEXUS_VideoDecoder_P_SetTrickState_Avd( handle, pState);
}

NEXUS_Error NEXUS_VideoDecoder_FrameAdvance(NEXUS_VideoDecoderHandle videoDecoder)
{
    return NEXUS_VideoDecoder_P_FrameAdvance_Avd(videoDecoder);
}

NEXUS_Error NEXUS_VideoDecoder_GetNextPts( NEXUS_VideoDecoderHandle handle, uint32_t *pNextPts)
{
    return NEXUS_VideoDecoder_P_GetNextPts_Avd( handle, pNextPts);
}

void NEXUS_VideoDecoder_GetPlaybackSettings( NEXUS_VideoDecoderHandle handle, NEXUS_VideoDecoderPlaybackSettings *pSettings)
{
    NEXUS_VideoDecoder_P_GetPlaybackSettings_Common( handle, pSettings);
}

NEXUS_Error NEXUS_VideoDecoder_SetPlaybackSettings( NEXUS_VideoDecoderHandle handle, const NEXUS_VideoDecoderPlaybackSettings *pSettings)
{
    return NEXUS_VideoDecoder_P_SetPlaybackSettings_Common( handle, pSettings);
}

NEXUS_StillDecoderHandle NEXUS_StillDecoder_Open( NEXUS_VideoDecoderHandle parentDecoder, unsigned index, const NEXUS_StillDecoderOpenSettings *pSettings)
{
#if NEXUS_NUM_STILL_DECODES
    return NEXUS_StillDecoder_P_Open_Avd( parentDecoder, index, pSettings);
#else
    BSTD_UNUSED(parentDecoder);
    BSTD_UNUSED(index);
    BSTD_UNUSED(pSettings);
    BERR_TRACE(NEXUS_NOT_SUPPORTED);
    return NULL;
#endif
}

static void NEXUS_StillDecoder_P_Finalizer( NEXUS_StillDecoderHandle handle)
{
#if NEXUS_NUM_STILL_DECODES
    NEXUS_StillDecoder_P_Close_Avd(handle);
#else
    BSTD_UNUSED(handle);
#endif
}

NEXUS_OBJECT_CLASS_MAKE(NEXUS_StillDecoder, NEXUS_StillDecoder_Close);

NEXUS_Error NEXUS_StillDecoder_Start( NEXUS_StillDecoderHandle handle, const NEXUS_StillDecoderStartSettings *pSettings)
{
#if NEXUS_NUM_STILL_DECODES
    return NEXUS_StillDecoder_P_Start_Avd( handle, pSettings);
#else
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
#endif
}

void NEXUS_StillDecoder_Stop( NEXUS_StillDecoderHandle handle)
{
#if NEXUS_NUM_STILL_DECODES
    NEXUS_StillDecoder_P_Stop_Avd(handle);
#else
    BSTD_UNUSED(handle);
#endif
}

NEXUS_Error NEXUS_StillDecoder_GetStatus( NEXUS_StillDecoderHandle handle, NEXUS_StillDecoderStatus *pStatus )
{
#if NEXUS_NUM_STILL_DECODES
    return NEXUS_StillDecoder_P_GetStatus_Avd( handle, pStatus );
#else
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pStatus);
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
#endif
}

NEXUS_Error NEXUS_StillDecoder_GetStripedSurface( NEXUS_StillDecoderHandle handle, NEXUS_StripedSurfaceHandle *pStripedSurface)
{
#if NEXUS_NUM_STILL_DECODES
    return NEXUS_StillDecoder_P_GetStripedSurface_Avd( handle, pStripedSurface);
#else
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pStripedSurface);
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
#endif
}

void NEXUS_StillDecoder_ReleaseStripedSurface( NEXUS_StillDecoderHandle handle, NEXUS_StripedSurfaceHandle stripedSurface )
{
#if NEXUS_NUM_STILL_DECODES
    NEXUS_StillDecoder_P_ReleaseStripedSurface_Avd( handle, stripedSurface);
#else
    BSTD_UNUSED(handle);
    BSTD_UNUSED(stripedSurface);
#endif
}

NEXUS_ModuleHandle NEXUS_VideoDecoderModule_Init(const NEXUS_VideoDecoderModuleSettings *pSettings)
{
    return NEXUS_VideoDecoderModule_P_Init_Avd(pSettings);
}

void NEXUS_VideoDecoderModule_Uninit(void)
{
    NEXUS_VideoDecoderModule_P_Uninit_Avd();
}

void NEXUS_VideoDecoder_GetDisplayConnection_priv( NEXUS_VideoDecoderHandle handle, NEXUS_VideoDecoderDisplayConnection *pConnection)
{
    NEXUS_VideoDecoder_GetDisplayConnection_priv_Avd(handle, pConnection);
}

NEXUS_Error NEXUS_VideoDecoder_SetDisplayConnection_priv( NEXUS_VideoDecoderHandle handle, const NEXUS_VideoDecoderDisplayConnection *pConnection)
{
    return NEXUS_VideoDecoder_SetDisplayConnection_priv_Avd(handle, pConnection);
}

void NEXUS_VideoDecoder_GetSourceId_priv( NEXUS_VideoDecoderHandle handle, BAVC_SourceId *pSource)
{
    NEXUS_VideoDecoder_GetSourceId_priv_Avd(handle, pSource);
}

void NEXUS_VideoDecoder_GetHeap_priv( NEXUS_VideoDecoderHandle handle, NEXUS_HeapHandle *pHeap)
{
    NEXUS_VideoDecoder_GetHeap_priv_Common(handle, pHeap);
}

void NEXUS_VideoDecoder_GetSyncSettings_priv(NEXUS_VideoDecoderHandle videoDecoder, NEXUS_VideoInputSyncSettings *pSyncSettings)
{
    NEXUS_VideoDecoder_GetSyncSettings_priv_Common(videoDecoder, pSyncSettings);
}

NEXUS_Error NEXUS_VideoDecoder_SetSyncSettings_priv(NEXUS_VideoDecoderHandle videoDecoder, const NEXUS_VideoInputSyncSettings *pSyncSettings)
{
    return NEXUS_VideoDecoder_SetSyncSettings_priv_Avd(videoDecoder, pSyncSettings);
}

NEXUS_Error NEXUS_VideoDecoder_GetSyncStatus_isr(NEXUS_VideoDecoderHandle videoDecoder, NEXUS_VideoInputSyncStatus *pSyncStatus )
{
    return NEXUS_VideoDecoder_GetSyncStatus_Common_isr(videoDecoder, pSyncStatus);
}

void NEXUS_VideoDecoder_UpdateDisplayInformation_priv( NEXUS_VideoDecoderHandle videoDecoder, const NEXUS_VideoDecoder_DisplayInformation *displayInformation)
{
    NEXUS_VideoDecoder_UpdateDisplayInformation_priv_Avd( videoDecoder, displayInformation);
}

#if NEXUS_HAS_ASTM
void NEXUS_VideoDecoder_GetAstmSettings_priv( NEXUS_VideoDecoderHandle videoDecoder, NEXUS_VideoDecoderAstmSettings * pAstmSettings)
{
    NEXUS_VideoDecoder_GetAstmSettings_priv_Common(videoDecoder, pAstmSettings);
}

NEXUS_Error NEXUS_VideoDecoder_SetAstmSettings_priv(NEXUS_VideoDecoderHandle videoDecoder, const NEXUS_VideoDecoderAstmSettings * pAstmSettings)
{
    return NEXUS_VideoDecoder_SetAstmSettings_priv_Avd(videoDecoder,  pAstmSettings);
}

NEXUS_Error NEXUS_VideoDecoder_GetAstmStatus_isr( NEXUS_VideoDecoderHandle videoDecoder, NEXUS_VideoDecoderAstmStatus * pAstmStatus)
{
    return NEXUS_VideoDecoder_GetAstmStatus_Common_isr( videoDecoder,  pAstmStatus);
}
#endif /* NEXUS_HAS_ASTM */

NEXUS_Error NEXUS_VideoDecoder_GetDecodedFrames(
    NEXUS_VideoDecoderHandle handle,
    NEXUS_VideoDecoderFrameStatus *pStatus,  /* attr{nelem=numEntries;nelem_out=pNumEntriesReturned;null_allowed=y} [out] */
    unsigned numEntries,
    unsigned *pNumEntriesReturned /* [out] */
    )
{
    return NEXUS_VideoDecoder_GetDecodedFrames_Avd(handle, pStatus, numEntries, pNumEntriesReturned);
}

NEXUS_Error NEXUS_VideoDecoder_ReturnDecodedFrames(
    NEXUS_VideoDecoderHandle handle,
    const NEXUS_VideoDecoderReturnFrameSettings *pSettings, /* attr{nelem=numFrames;null_allowed=y} Settings for each returned frame.  Pass NULL for defaults. */
    unsigned numFrames                                      /* Number of frames to return to the decoder */
    )
{
    return NEXUS_VideoDecoder_ReturnDecodedFrames_Avd(handle, pSettings, numFrames);
}

NEXUS_Error NEXUS_VideoDecoder_VideoAsGraphicsSupported_priv(
    NEXUS_VideoDecoderHandle handle
    )
{
    BSTD_UNUSED(handle);
    return NEXUS_SUCCESS;
}
