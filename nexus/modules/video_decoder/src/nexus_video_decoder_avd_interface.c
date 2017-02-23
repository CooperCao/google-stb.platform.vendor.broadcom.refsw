/***************************************************************************
 *  Copyright (C) 2007-2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  .  Except as set forth in an Authorized License, Broadcom grants
 *  no license , right to use, or waiver of any kind with respect to the
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
 *  LICENSORS BE LIABLE FOR  CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR  ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 *
 **************************************************************************/
#include "nexus_video_decoder_module.h"
#include "nexus_still_decoder_impl.h"

BDBG_MODULE(nexus_video_decoder_avd_interface);

const NEXUS_VideoDecoder_P_Interface NEXUS_VideoDecoder_P_Interface_Avd = {
    NEXUS_VideoDecoder_P_Close_Avd,
    NEXUS_VideoDecoder_P_GetOpenSettings_Common,
    NEXUS_VideoDecoder_P_GetSettings_Common,
    NEXUS_VideoDecoder_P_SetSettings_Avd,
    NEXUS_VideoDecoder_P_Start_Avd,
    NEXUS_VideoDecoder_P_Stop_Avd,
    NEXUS_VideoDecoder_P_Flush_Avd,
    NEXUS_VideoDecoder_P_GetStatus_Avd,
    NEXUS_VideoDecoder_P_GetConnector_Common,
    NEXUS_VideoDecoder_P_GetStreamInformation_Avd,
    NEXUS_VideoDecoder_P_SetStartPts_Avd,
    NEXUS_VideoDecoder_P_IsCodecSupported_Generic,
    NEXUS_VideoDecoder_P_SetPowerState_Avd,
    NEXUS_VideoDecoder_P_Reset_Avd,
    NEXUS_VideoDecoder_P_GetExtendedStatus_Avd,
    NEXUS_VideoDecoder_P_GetExtendedSettings_Avd,
    NEXUS_VideoDecoder_P_SetExtendedSettings_Avd,
    NEXUS_VideoDecoder_P_CreateStripedSurface_Avd,
    NEXUS_VideoDecoder_P_DestroyStripedSurface_Avd,
    NEXUS_VideoDecoder_P_CreateStripedMosaicSurfaces_Avd,
    NEXUS_VideoDecoder_P_GetMostRecentPts_Avd,
    NEXUS_VideoDecoder_P_GetTrickState_Common,
    NEXUS_VideoDecoder_P_SetTrickState_Avd,
    NEXUS_VideoDecoder_P_FrameAdvance_Avd,
    NEXUS_VideoDecoder_P_GetNextPts_Avd,
    NEXUS_VideoDecoder_P_GetPlaybackSettings_Common,
    NEXUS_VideoDecoder_P_SetPlaybackSettings_Common,
#if NEXUS_NUM_STILL_DECODES
    NEXUS_StillDecoder_P_Open_Avd,
#else
    NULL,
#endif

#if NEXUS_HAS_ASTM
    NEXUS_VideoDecoder_GetAstmSettings_priv_Common,
    NEXUS_VideoDecoder_SetAstmSettings_priv_Avd,
    NEXUS_VideoDecoder_GetAstmStatus_Common_isr,
#endif

    NEXUS_VideoDecoder_GetDisplayConnection_priv_Avd,
    NEXUS_VideoDecoder_SetDisplayConnection_priv_Avd,
    NEXUS_VideoDecoder_GetSourceId_priv_Avd,
    NEXUS_VideoDecoder_GetHeap_priv_Common,

#if NEXUS_HAS_SYNC_CHANNEL
    NEXUS_VideoDecoder_GetSyncSettings_priv_Common,
    NEXUS_VideoDecoder_SetSyncSettings_priv_Avd,
    NEXUS_VideoDecoder_GetSyncStatus_Common_isr,
#endif

    NEXUS_VideoDecoder_UpdateDisplayInformation_priv_Avd,
    NEXUS_VideoDecoder_GetDecodedFrames_Avd,
    NEXUS_VideoDecoder_ReturnDecodedFrames_Avd
};

#if NEXUS_NUM_STILL_DECODES
const NEXUS_StillDecoder_P_Interface NEXUS_StillDecoder_P_Interface_Avd  = {
    NEXUS_StillDecoder_P_Close_Avd,
    NEXUS_StillDecoder_P_Start_Avd,
    NEXUS_StillDecoder_P_Stop_Avd,
    NEXUS_StillDecoder_P_GetStatus_Avd,
    NEXUS_StillDecoder_P_GetStripedSurface_Avd,
    NEXUS_StillDecoder_P_ReleaseStripedSurface_Avd
};
#endif
