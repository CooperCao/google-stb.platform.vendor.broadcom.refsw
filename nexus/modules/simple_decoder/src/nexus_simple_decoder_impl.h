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
#ifndef NEXUS_SIMPLE_DECODER_IMPL_H__
#define NEXUS_SIMPLE_DECODER_IMPL_H__

#ifdef __cplusplus
#error
#endif

typedef enum NEXUS_SimpleDecoderType
{
    NEXUS_SimpleDecoderType_eVideo,
    NEXUS_SimpleDecoderType_eAudio,
    NEXUS_SimpleDecoderType_eMax
} NEXUS_SimpleDecoderType;

NEXUS_StcChannelHandle NEXUS_SimpleStcChannel_GetServerStcChannel_priv(NEXUS_SimpleStcChannelHandle handle, NEXUS_SimpleDecoderType type);
NEXUS_SimpleStcChannelHandle NEXUS_SimpleVideoDecoder_P_GetStcChannel( NEXUS_SimpleVideoDecoderHandle handle );
NEXUS_SimpleStcChannelHandle NEXUS_SimpleAudioDecoder_P_GetStcChannel( NEXUS_SimpleAudioDecoderHandle handle );

typedef struct NEXUS_SimpleStcChannelEncoderStatus
{
    bool enabled;
    NEXUS_Timebase timebase;
    bool nonRealTime;
} NEXUS_SimpleStcChannelEncoderStatus;

typedef struct NEXUS_SimpleStcChannelDecoderStatus
{
    bool connected; /* is the server decoder resource connected */
    bool primer; /* is this a primer channel (which doesn't have a server decoder resource, but still needs a valid stc channel with autoConfigTimebase = false */
    struct
    {
        int index; /* server preferred stc index */
    } stc;
    NEXUS_SimpleStcChannelEncoderStatus encoder;
    bool hdDviInput;
    bool mainWindow;
} NEXUS_SimpleStcChannelDecoderStatus;

void NEXUS_SimpleVideoDecoder_GetStcStatus_priv(NEXUS_SimpleVideoDecoderHandle handle, NEXUS_SimpleStcChannelDecoderStatus * pStatus);
void NEXUS_SimpleAudioDecoder_GetStcStatus_priv(NEXUS_SimpleAudioDecoderHandle handle, NEXUS_SimpleStcChannelDecoderStatus * pStatus);
void NEXUS_SimpleEncoder_GetStcStatus_priv(NEXUS_SimpleEncoderHandle encoder, NEXUS_SimpleStcChannelEncoderStatus *pStatus);

void NEXUS_SimpleStcChannel_SetVideo_priv(NEXUS_SimpleStcChannelHandle handle, NEXUS_SimpleVideoDecoderHandle video);
void NEXUS_SimpleStcChannel_SetAudio_priv(NEXUS_SimpleStcChannelHandle handle, NEXUS_SimpleAudioDecoderHandle audio);
bool NEXUS_SimpleStcChannel_P_ActiveVideo(NEXUS_SimpleStcChannelHandle handle);
void NEXUS_SimpleAudioDecoder_AdjustAudioPrimerToVideo_priv(NEXUS_SimpleAudioDecoderHandle handle, bool videoActive);

#if NEXUS_HAS_ASTM
NEXUS_Error NEXUS_SimpleStcChannel_SetAstmVideo_priv(NEXUS_SimpleStcChannelHandle handle, NEXUS_VideoDecoderHandle videoDecoder);
NEXUS_Error NEXUS_SimpleStcChannel_AddAstmAudio_priv(NEXUS_SimpleStcChannelHandle handle, NEXUS_AudioDecoderHandle audioDecoder);
void NEXUS_SimpleStcChannel_RemoveAstmAudio_priv(NEXUS_SimpleStcChannelHandle handle, NEXUS_AudioDecoderHandle audioDecoder);
#endif

#if NEXUS_HAS_SYNC_CHANNEL
NEXUS_Error NEXUS_SimpleStcChannel_SetSyncVideo_priv(NEXUS_SimpleStcChannelHandle handle, NEXUS_VideoInput videoInput);
NEXUS_Error NEXUS_SimpleStcChannel_AddSyncAudio_priv(NEXUS_SimpleStcChannelHandle handle, NEXUS_AudioInputHandle audioInput);
void NEXUS_SimpleStcChannel_RemoveSyncAudio_priv(NEXUS_SimpleStcChannelHandle handle, NEXUS_AudioInputHandle audioInput);
#endif

/* simple_video_decoder_impl */
NEXUS_Error nexus_simplevideodecoder_p_add_encoder(NEXUS_SimpleVideoDecoderHandle handle, NEXUS_VideoWindowHandle window, NEXUS_SimpleEncoderHandle encoder);
void nexus_simplevideodecoder_p_remove_encoder(NEXUS_SimpleVideoDecoderHandle handle, NEXUS_VideoWindowHandle window, NEXUS_SimpleEncoderHandle encoder);

/* internal stop if decoders stopped */
void nexus_simpleencoder_p_stop(NEXUS_SimpleEncoderHandle encoder);
bool nexus_simpleencoder_p_nonRealTime(NEXUS_SimpleEncoderHandle encoder);

void NEXUS_SimpleDecoderModule_P_PrintVideoDecoder(void);
void NEXUS_SimpleDecoderModule_P_PrintAudioDecoder(void);
void NEXUS_SimpleDecoderModule_P_PrintEncoder(void);

NEXUS_Error nexus_simpleencoder_p_start_audio(NEXUS_SimpleEncoderHandle handle);
NEXUS_Error nexus_simpleencoder_p_start_video(NEXUS_SimpleEncoderHandle handle);
void nexus_simplevideodecoder_p_remove_settings_from_cache(void);
void nexus_simpleencoder_p_stop_videoencoder(NEXUS_SimpleEncoderHandle handle, bool abort);

void NEXUS_SimpleVideoDecoderModule_P_UnloadDefaultSettings(void);
void NEXUS_SimpleAudioDecoderModule_P_UnloadDefaultSettings(void);

#endif
