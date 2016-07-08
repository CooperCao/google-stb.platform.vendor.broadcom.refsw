/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
 **************************************************************************/
#include "nexus_playback.h"

struct {
    const char *filename;
    unsigned short video_pid;
    NEXUS_VideoCodec video_codec;
    unsigned short audio_pid;
    NEXUS_AudioCodec audio_codec;
} g_teststreams[] =
{
    {"videos/discoveryAvcHD.mpg", 0x1522, NEXUS_VideoCodec_eH264, 0x1523, NEXUS_AudioCodec_eAc3},
    {"videos/cnnticker.mpg", 0x21, NEXUS_VideoCodec_eMpeg2, 0x22, NEXUS_AudioCodec_eMpeg},
    {"videos/herbie1AvcHD.mpg", 0x1222, NEXUS_VideoCodec_eH264, 0x1223, NEXUS_AudioCodec_eAc3},
    {"videos/riddick_avc_720p.mpg", 0x1422, NEXUS_VideoCodec_eH264, 0x1423, NEXUS_AudioCodec_eAc3},
    {NULL, 0, NEXUS_VideoCodec_eUnknown, 0, NEXUS_AudioCodec_eUnknown}
};

struct b_video_decoder
{
    NEXUS_VideoDecoderHandle videoDecoder;
    NEXUS_StcChannelHandle stcChannel;
    NEXUS_PidChannelHandle videoPidChannel;
    NEXUS_FilePlayHandle file;
    NEXUS_PlaypumpHandle playpump;
    NEXUS_PlaybackHandle playback;
};

int b_video_decoder_open(struct b_video_decoder *decoder, unsigned index)
{
    NEXUS_StcChannelSettings stcSettings;

    BKNI_Memset(decoder, 0, sizeof(*decoder));
    decoder->playpump = NEXUS_Playpump_Open(NEXUS_ANY_ID, NULL);
    if (!decoder->playpump) return BERR_TRACE(-1);

    decoder->playback = NEXUS_Playback_Create();
    if (!decoder->playback) return BERR_TRACE(-1);

    NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;
    stcSettings.mode = NEXUS_StcChannelMode_eAuto;
    decoder->stcChannel = NEXUS_StcChannel_Open(index, &stcSettings);
    if (!decoder->stcChannel) return BERR_TRACE(-1);

    decoder->videoDecoder = NEXUS_VideoDecoder_Open(index, NULL); /* take default capabilities */
    if (!decoder->videoDecoder) return BERR_TRACE(-1);
    return 0;
}

void b_video_decoder_close(struct b_video_decoder *decoder)
{
    NEXUS_VideoDecoder_Close(decoder->videoDecoder);
    NEXUS_StcChannel_Close(decoder->stcChannel);
    NEXUS_Playback_Destroy(decoder->playback);
    NEXUS_Playpump_Close(decoder->playpump);
}

int b_video_decoder_start_playback(struct b_video_decoder *decoder, const char *filename, unsigned pid, NEXUS_VideoCodec codec)
{
    NEXUS_PlaybackSettings playbackSettings;
    NEXUS_PlaybackPidChannelSettings playbackPidSettings;
    NEXUS_VideoDecoderStartSettings videoProgram;
    int rc;

    decoder->file = NEXUS_FilePlay_OpenPosix(filename, NULL);
    if (!decoder->file) {
        BDBG_ERR(("can't open file:%s", filename));
        return BERR_TRACE(-1);
    }

    NEXUS_Playback_GetSettings(decoder->playback, &playbackSettings);
    playbackSettings.playpump = decoder->playpump;
    playbackSettings.playpumpSettings.transportType = NEXUS_TransportType_eTs; /* TODO */
    playbackSettings.stcChannel = decoder->stcChannel;
    rc = NEXUS_Playback_SetSettings(decoder->playback, &playbackSettings);
    if (rc) return BERR_TRACE(rc);

    NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
    playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
    playbackPidSettings.pidTypeSettings.video.codec = codec;
    playbackPidSettings.pidTypeSettings.video.index = true;
    playbackPidSettings.pidTypeSettings.video.decoder = decoder->videoDecoder;
    decoder->videoPidChannel = NEXUS_Playback_OpenPidChannel(decoder->playback, pid, &playbackPidSettings);
    if (!decoder->videoPidChannel) return BERR_TRACE(-1);

    NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
    videoProgram.codec = codec;
    videoProgram.pidChannel = decoder->videoPidChannel;
    videoProgram.stcChannel = decoder->stcChannel;
    rc = NEXUS_VideoDecoder_Start(decoder->videoDecoder, &videoProgram);
    if (rc) return BERR_TRACE(rc);

    rc = NEXUS_Playback_Start(decoder->playback, decoder->file, NULL);
    if (rc) return BERR_TRACE(rc);

    return 0;
}

void b_video_decoder_stop_playback(struct b_video_decoder *decoder)
{
    NEXUS_Playback_Stop(decoder->playback);
    NEXUS_VideoDecoder_Stop(decoder->videoDecoder);
    NEXUS_Playback_ClosePidChannel(decoder->playback, decoder->videoPidChannel);
    NEXUS_FilePlay_Close(decoder->file);
}
