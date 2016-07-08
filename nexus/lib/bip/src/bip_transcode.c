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
 *****************************************************************************/

#include "b_os_lib.h"
#include "bip_priv.h"
#include "bip_transcode_impl.h"
#include "b_playback_ip_lib.h"
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

BDBG_MODULE( bip_transcode );
BDBG_OBJECT_ID( BIP_Transcode );
BIP_SETTINGS_ID(BIP_TranscodeCreateSettings);
BIP_SETTINGS_ID(BIP_TranscodePrepareSettings);
BIP_SETTINGS_ID(BIP_TranscodeStartSettings);
BIP_SETTINGS_ID(BIP_TranscodeNexusHandles);
BIP_SETTINGS_ID(BIP_TranscodeProfile);
BIP_SETTINGS_ID(BIP_TranscodeHandles);

struct BIP_TranscodeStateNames
{
    BIP_TranscodeState state;
    char *pStateName;
}gTranscodeState[] = {
    {BIP_TranscodeState_eUninitialized,      "UnInitialized"},
    {BIP_TranscodeState_eIdle,               "Idle"},                /* Idle state. */
    {BIP_TranscodeState_ePrepared,           "Prepared"},            /* Prepared for Transcoding. */
    {BIP_TranscodeState_eTranscoding,        "Transcoding"},         /* Transcoding is in progress. */
    {BIP_TranscodeState_eMax,                "MaxState"}
};
#define BIP_TRANSCODE_STATE(state) \
    gTranscodeState[state].pStateName

void processTranscodeState( void *jObject, int value, BIP_Arb_ThreadOrigin threadOrigin );

static void printTranscodeStatus(
    BIP_TranscodeHandle hTranscode
    )
{
    NEXUS_Error nrc;

    if ( hTranscode == NULL ) return;

    /* Transcode Status */
    {
        {
            NEXUS_VideoDecoderStatus videoStats;
            nrc = NEXUS_SimpleVideoDecoder_GetStatus( hTranscode->nexusHandles.simple.hVideo, &videoStats );
            BDBG_ASSERT( nrc == NEXUS_SUCCESS );
            BDBG_WRN(("Vdec: started=%s srcSize=%u x %u frameRate=%u interlaced=%s tsm=%s firstPtsPassed=%s",
                        videoStats.started?"Y":"N",
                        videoStats.source.width, videoStats.source.height,
                        videoStats.frameRate, videoStats.interlaced?"Y":"N",
                        videoStats.tsm?"Y":"N", videoStats.firstPtsPassed?"Y":"N"
                        ));
            BDBG_WRN(("Vdec: fifo depth/size=%u/%u decoded=%u displayed=%u picRcvd=%u iFrDisplayed=%u decoderErrs=%u decoderOverflows=%u decoderDrops=%u displayErrs=%u displayDrops=%u displayUnderflows=%u ptsErrs=%u watchdogs=%u bytesDecoded=%llu",
                        videoStats.fifoDepth, videoStats.fifoSize,
                        videoStats.numDecoded, videoStats.numDisplayed,
                        videoStats.numPicturesReceived,
                        videoStats.numIFramesDisplayed,
                        videoStats.numDecodeErrors,
                        videoStats.numDecodeOverflows,
                        videoStats.numDecodeDrops,

                        videoStats.numDisplayErrors,
                        videoStats.numDisplayDrops,
                        videoStats.numDisplayUnderflows,
                        videoStats.ptsErrorCount,
                        videoStats.numWatchdogs,
                        videoStats.numBytesDecoded
                        ));
        }
        {
            NEXUS_AudioDecoderStatus audioStats;
            nrc = NEXUS_SimpleAudioDecoder_GetStatus( hTranscode->nexusHandles.simple.hAudio, &audioStats );
            BDBG_ASSERT( nrc == NEXUS_SUCCESS );
            BDBG_WRN(("Adec: started=%s tsm=%s locked=%s fifoSz=%u/%u sampelRate=%d frDecoded=%u frErrs=%u fifoOverflows=%u fifoUnderflows=%u bytesDecoded=%lld watchdogs=%u",
                        audioStats.started?"Y":"N",
                        audioStats.tsm?"Y":"N", audioStats.locked?"Y":"N",
                        audioStats.fifoDepth, audioStats.fifoSize,
                        audioStats.sampleRate, audioStats.framesDecoded,
                        audioStats.frameErrors, audioStats.numFifoOverflows,
                        audioStats.numFifoUnderflows, audioStats.numBytesDecoded, audioStats.numWatchdogs
                     ));
        }
        {
            NEXUS_SimpleEncoderStatus encoderStatus;
            NEXUS_SimpleEncoder_GetStatus( hTranscode->nexusHandles.simple.hEncoder, &encoderStatus );
            BDBG_WRN(("Enc: vidEncEnabled=%s picRcvd=%u picDropFRC=%u picDropHDR=%u picDropErrs=%u picEncoded=%u picPerSec=%u dataFifo=%u/%u ver=0x%x",
                        encoderStatus.videoEncoder.enabled?"Y":"N",
                        encoderStatus.video.picturesReceived,
                        encoderStatus.video.picturesDroppedFRC,
                        encoderStatus.video.picturesDroppedHRD,
                        encoderStatus.video.picturesDroppedErrors,
                        encoderStatus.video.picturesEncoded,
                        encoderStatus.video.picturesPerSecond,
                        encoderStatus.video.data.fifoDepth,
                        encoderStatus.video.data.fifoSize,
                        encoderStatus.video.version.firmware
                     ));
        }
    }
}

static void
ptsErrorCallback(void *context, int param)
{
    BIP_TranscodeHandle hTranscode = context;
    BSTD_UNUSED( param );
    BDBG_WRN(( BIP_MSG_PRE_FMT "pTranscode=%p" BIP_MSG_PRE_ARG, (void *)hTranscode ));
}

static void
videoFirstPtsCallback(void *context, int param)
{
    BIP_TranscodeHandle hTranscode = context;
    BSTD_UNUSED( param );
    BDBG_WRN(( BIP_MSG_PRE_FMT "pTranscode=%p" BIP_MSG_PRE_ARG, (void *)hTranscode ));
}

static void
videoFirstPtsPassedCallback(void *context, int param)
{
    BIP_TranscodeHandle hTranscode = context;
    BSTD_UNUSED( param );
    BDBG_WRN(( BIP_MSG_PRE_FMT "pTranscode=%p" BIP_MSG_PRE_ARG, (void *)hTranscode ));
}

static BIP_Status prepareTranscodePath(
    BIP_TranscodeHandle     hTranscode
    )
{
    BIP_Status bipStatus = BIP_ERR_INTERNAL;
    NEXUS_Error nrc = NEXUS_UNKNOWN;

    BDBG_WRN(( "hTranscode=%p type=%s video=%s vTrackId=%u size=%ux%u codec=%s profile=%s level=%s refreshRate=%u bps=%u frameRate=%s lowDelayPipeline=%s adaptiveLowDelayMode=%s nonRealTime=%s rateBufferDelay=%u gopDuration=%u adaptiveDuration=%s, #framesP=%u #framesB=%u",
                (void *)hTranscode,
                BIP_ToStr_NEXUS_TransportType( hTranscode->transcodeProfile.containerType ),
                hTranscode->transcodeProfile.disableVideo? "N":"Y",
                hTranscode->transcodeProfile.video.trackId,
                hTranscode->transcodeProfile.video.width, hTranscode->transcodeProfile.video.height,
                BIP_ToStr_NEXUS_VideoCodec( hTranscode->transcodeProfile.video.startSettings.codec ),
                BIP_ToStr_NEXUS_VideoCodecProfile (hTranscode->transcodeProfile.video.startSettings.profile ),
                BIP_ToStr_NEXUS_VideoCodecLevel (hTranscode->transcodeProfile.video.startSettings.level ),
                hTranscode->transcodeProfile.video.refreshRate,
                hTranscode->transcodeProfile.video.settings.bitrateMax,
                BIP_ToStr_NEXUS_VideoFrameRate( hTranscode->transcodeProfile.video.settings.frameRate),
                hTranscode->transcodeProfile.video.startSettings.lowDelayPipeline?"Y":"N",
                hTranscode->transcodeProfile.video.startSettings.adaptiveLowDelayMode?"Y":"N",
                hTranscode->transcodeProfile.video.startSettings.nonRealTime?"Y":"N",
                hTranscode->transcodeProfile.video.startSettings.rateBufferDelay,
                hTranscode->transcodeProfile.video.settings.streamStructure.duration,
                hTranscode->transcodeProfile.video.settings.streamStructure.adaptiveDuration ? "Y":"N",
                hTranscode->transcodeProfile.video.settings.streamStructure.framesP,
                hTranscode->transcodeProfile.video.settings.streamStructure.framesB
             ));
    /* Set StcChannel Settings: transportType, mode (auto/pcr/etc.), etc. */
    {
        NEXUS_SimpleStcChannelSettings stcSettings;

        NEXUS_SimpleStcChannel_GetSettings( hTranscode->nexusHandles.simple.hStcChannel, &stcSettings );
        stcSettings.modeSettings.Auto.transportType = hTranscode->prepareApi.transcodeStreamInfo.transportType;
        if ( hTranscode->prepareApi.nonRealTime )
        {
            stcSettings.mode = NEXUS_StcChannelMode_eAuto;
        }
        else
        {
            stcSettings.mode = NEXUS_StcChannelMode_ePcr;
            BDBG_ASSERT( hTranscode->prepareApi.settings.hPcrPidChannel );
            stcSettings.modeSettings.pcr.pidChannel = hTranscode->prepareApi.settings.hPcrPidChannel;
        }
        nrc = NEXUS_SimpleStcChannel_SetSettings( hTranscode->nexusHandles.simple.hStcChannel, &stcSettings );
        BIP_CHECK_GOTO(( nrc == NEXUS_SUCCESS ), ( "NEXUS_SimpleStcChannel_SetSettings Failed!" ), error, BIP_ERR_NEXUS, bipStatus );
        BDBG_MSG(( BIP_MSG_PRE_FMT "hStcChannel=%p mode=%d" BIP_MSG_PRE_ARG, (void *)hTranscode->nexusHandles.simple.hStcChannel, stcSettings.mode ));
    }

    /* Setup Nexus Playback w/ Stc Channel. */
    if ( hTranscode->prepareApi.settings.hPlayback )
    {
        NEXUS_PlaybackSettings playbackSettings;
        NEXUS_PlaybackPidChannelSettings playbackPidSettings;

        NEXUS_Playback_GetSettings( hTranscode->prepareApi.settings.hPlayback , &playbackSettings );
        playbackSettings.simpleStcChannel = hTranscode->nexusHandles.simple.hStcChannel;
        nrc = NEXUS_Playback_SetSettings( hTranscode->prepareApi.settings.hPlayback, &playbackSettings );
        BIP_CHECK_GOTO(( nrc == NEXUS_SUCCESS ), ( "NEXUS_Playback_SetSettings Failed!" ), error, BIP_ERR_NEXUS, bipStatus );

        if ( hTranscode->prepareApi.settings.hVideoPidChannel ||  hTranscode->prepareApi.settings.hAudioPidChannel )
        {
            nrc = NEXUS_Playback_GetPidChannelSettings( hTranscode->prepareApi.settings.hPlayback, hTranscode->prepareApi.settings.hVideoPidChannel, &playbackPidSettings );
            BIP_CHECK_GOTO(( nrc == NEXUS_SUCCESS ), ( "NEXUS_Playback_GetPidChannelSettings Failed!" ), error, BIP_ERR_NEXUS, bipStatus );
            playbackPidSettings.pidTypeSettings.video.simpleDecoder = hTranscode->nexusHandles.simple.hVideo;
            playbackPidSettings.pidTypeSettings.audio.simpleDecoder = hTranscode->nexusHandles.simple.hAudio;
            nrc = NEXUS_Playback_SetPidChannelSettings( hTranscode->prepareApi.settings.hPlayback, hTranscode->prepareApi.settings.hVideoPidChannel, &playbackPidSettings );
            BIP_CHECK_GOTO(( nrc == NEXUS_SUCCESS ), ( "NEXUS_Playback_SetPidChannelSettings Failed!" ), error, BIP_ERR_NEXUS, bipStatus );
        }
    }

    /* Decoders Settings. */
    {
        bool videoDisabled = false, audioDisabled = false;
        NEXUS_VideoDecoderSettings settings;

        if ( !hTranscode->transcodeProfile.disableVideo && hTranscode->prepareApi.settings.hVideoPidChannel )
        {
            nrc = NEXUS_SimpleVideoDecoder_SetStcChannel( hTranscode->nexusHandles.simple.hVideo, hTranscode->nexusHandles.simple.hStcChannel );
            BIP_CHECK_GOTO(( nrc == NEXUS_SUCCESS ), ( "NEXUS_SimpleVideoDecoder_SetStcChannel Failed!" ), error, BIP_ERR_NEXUS, bipStatus );
            BDBG_MSG(( BIP_MSG_PRE_FMT "hVideoDecoder=%p hStcChannel=%p linked" BIP_MSG_PRE_ARG, (void *)hTranscode->nexusHandles.simple.hVideo, (void *)hTranscode->nexusHandles.simple.hStcChannel));

            NEXUS_SimpleVideoDecoder_GetDefaultStartSettings( &hTranscode->videoProgram );
            hTranscode->videoProgram.settings.pidChannel = hTranscode->prepareApi.settings.hVideoPidChannel;
            hTranscode->videoProgram.settings.codec = hTranscode->prepareApi.settings.videoTrack.info.video.codec;
            hTranscode->videoProgram.maxWidth = hTranscode->prepareApi.settings.videoTrack.info.video.width;
            hTranscode->videoProgram.maxHeight = hTranscode->prepareApi.settings.videoTrack.info.video.height;
            BIP_MSG_SUM(( BIP_MSG_PRE_FMT "hVideoDecoder=%p codec=%d maxWidth=%d maxHeight=%d pidChannel=%p" BIP_MSG_PRE_ARG,
                        (void *)hTranscode->nexusHandles.simple.hVideo, hTranscode->videoProgram.settings.codec,
                        hTranscode->videoProgram.maxWidth, hTranscode->videoProgram.maxHeight, (void *)hTranscode->prepareApi.settings.hVideoPidChannel ));

            NEXUS_SimpleVideoDecoder_GetSettings( hTranscode->nexusHandles.simple.hVideo, &settings );
            if ( hTranscode->prepareApi.settings.videoTrack.info.video.colorDepth > 8 )
            {

                settings.colorDepth = hTranscode->prepareApi.settings.videoTrack.info.video.colorDepth;
            }
            settings.firstPts.callback = videoFirstPtsCallback;
            settings.firstPts.context = hTranscode;
            settings.firstPts.param = 0;
            settings.firstPtsPassed.callback = videoFirstPtsPassedCallback;
            settings.firstPtsPassed.context = hTranscode;
            settings.firstPtsPassed.param = 0;
            settings.ptsError.callback = ptsErrorCallback;
            settings.ptsError.context = hTranscode;
            settings.ptsError.param = 0;
            settings.supportedCodecs[NEXUS_VideoCodec_eH264_Svc] = false; /* xcode TODO */
            settings.supportedCodecs[NEXUS_VideoCodec_eH264_Mvc] = false; /* xcode TODO */
            nrc = NEXUS_SimpleVideoDecoder_SetSettings( hTranscode->nexusHandles.simple.hVideo, &settings );
            BIP_CHECK_GOTO(( nrc == NEXUS_SUCCESS ), ( "NEXUS_SimpleVideoDecoder_SetSettings Failed!" ), error, BIP_ERR_NEXUS, bipStatus );
        }
        else
        {
            /* Either Transcode profile disabled the video or caller didn't add a video track as it may not be present in the stream. */
            videoDisabled = true;
        }
        if ( !hTranscode->transcodeProfile.disableAudio && hTranscode->prepareApi.settings.hAudioPidChannel )
        {
            NEXUS_SimpleAudioDecoder_GetDefaultStartSettings( &hTranscode->audioProgram );
            hTranscode->audioProgram.primary.pidChannel = hTranscode->prepareApi.settings.hAudioPidChannel;
            hTranscode->audioProgram.primary.codec = hTranscode->prepareApi.settings.audioTrack.info.audio.codec;

            nrc = NEXUS_SimpleAudioDecoder_SetStcChannel( hTranscode->nexusHandles.simple.hAudio, hTranscode->nexusHandles.simple.hStcChannel );
            BIP_CHECK_GOTO(( nrc == NEXUS_SUCCESS ), ( "NEXUS_SimpleAudioDecoder_SetStcChannel Failed!" ), error, BIP_ERR_NEXUS, bipStatus );

            BDBG_MSG(( BIP_MSG_PRE_FMT "hAudioDecoder=%p codec=%d" BIP_MSG_PRE_ARG, (void *)hTranscode->nexusHandles.simple.hAudio, hTranscode->audioProgram.primary.codec ));
        }
        else
        {
            /* Either Transcode profile disabled the audio or app didn't add a audio track as it may not be present in the stream. */
            audioDisabled = true;
        }

        if ( videoDisabled && audioDisabled )
        {
            BDBG_ERR(( BIP_MSG_PRE_FMT "hTranscodeCtx=%p: Neither Audio nor Video Tracks are present in the stream!" BIP_MSG_PRE_ARG, (void *)hTranscode));
            bipStatus = BIP_ERR_INVALID_PARAMETER;
            goto error;
        }
    }

    /* Encoder Settings. */
    {
        {
            NEXUS_SimpleEncoderSettings encoderSettings;

            NEXUS_SimpleEncoder_GetSettings( hTranscode->nexusHandles.simple.hEncoder, &encoderSettings );
            if ( !hTranscode->transcodeProfile.disableVideo )
            {
                encoderSettings.video.width = hTranscode->transcodeProfile.video.width;
                encoderSettings.video.height = hTranscode->transcodeProfile.video.height;
                encoderSettings.video.interlaced = hTranscode->transcodeProfile.video.startSettings.interlaced;
                encoderSettings.video.refreshRate = hTranscode->transcodeProfile.video.refreshRate;
                encoderSettings.video.refreshRate = 30000; /* xcode TODO */
                encoderSettings.videoEncoder = hTranscode->transcodeProfile.video.settings;
                encoderSettings.videoEncoder.enableFieldPairing = false; /* xcode TODO */
                encoderSettings.videoEncoder.variableFrameRate = false; /* xcode TODO */
            }
            if ( !hTranscode->transcodeProfile.disableAudio )
            {
                encoderSettings.audioEncoder.codec = hTranscode->transcodeProfile.audio.audioCodec;
                /* TODO: may need to set the audioCodec settings based on the audio codec. */
                /* encoderSettings.audioEncoder.codecSettings.<audioCodec> = hTranscode->transcodeProfile.audio.settings; */
            }
            nrc = NEXUS_SimpleEncoder_SetSettings( hTranscode->nexusHandles.simple.hEncoder, &encoderSettings );
#ifdef UNIT_TEST
            nrc = NEXUS_UNKNOWN;
#endif
            BIP_CHECK_GOTO(( nrc == NEXUS_SUCCESS ), ( "NEXUS_SimpleEncoder_SetSettings Failed!" ), error, BIP_ERR_NEXUS, bipStatus );
            BIP_MSG_SUM(( BIP_MSG_PRE_FMT "hTranscode=%p width=%d height=%d interlaced=%d refreshRate=%d, GOP duration=%u adaptiveDuration=%s" BIP_MSG_PRE_ARG, (void *)hTranscode->nexusHandles.simple.hEncoder,
                        encoderSettings.video.width, encoderSettings.video.height,
                        encoderSettings.video.interlaced, encoderSettings.video.refreshRate,
                        encoderSettings.videoEncoder.streamStructure.duration, encoderSettings.videoEncoder.streamStructure.adaptiveDuration?"Y":"N"
                        ));
            BIP_MSG_SUM(( BIP_MSG_PRE_FMT "bitrateTarget=%d bitrateMax=%d encoderDelay=%d frameRate=%s variableFrameRate=%s" BIP_MSG_PRE_ARG,
                        encoderSettings.videoEncoder.bitrateTarget, encoderSettings.videoEncoder.bitrateMax,
                        encoderSettings.videoEncoder.encoderDelay, BIP_ToStr_NEXUS_VideoFrameRate( encoderSettings.videoEncoder.frameRate ),
                        encoderSettings.videoEncoder.variableFrameRate?"Y":"N"
                     ));
        }

        {
            NEXUS_SimpleEncoderStartSettings startSettings;

            NEXUS_SimpleEncoder_GetDefaultStartSettings( &startSettings );
            startSettings.recpump = hTranscode->prepareApi.hRecpump;
            if ( !hTranscode->transcodeProfile.disableVideo )
            {
                startSettings.input.video = hTranscode->nexusHandles.simple.hVideo;
                startSettings.output.video.settings = hTranscode->transcodeProfile.video.startSettings;
                startSettings.output.video.index = false;
                startSettings.output.video.raiIndex = hTranscode->prepareApi.settings.enableRaiIndex;
                startSettings.output.video.pid = hTranscode->transcodeProfile.video.trackId;
#if 1
                /* Move these to the bip_transcode.h ? */
                startSettings.output.video.settings.bounds.outputFrameRate.min = NEXUS_VideoFrameRate_e25;
                startSettings.output.video.settings.bounds.outputFrameRate.max = NEXUS_VideoFrameRate_e60;
                startSettings.output.video.settings.bounds.inputFrameRate.min = NEXUS_VideoFrameRate_e25;
                startSettings.output.video.settings.bounds.inputDimension.max.width = 1280;
                startSettings.output.video.settings.bounds.inputDimension.max.height = 720;
#endif
            }
            if ( !hTranscode->transcodeProfile.disableAudio )
            {
                startSettings.input.audio = hTranscode->nexusHandles.simple.hAudio;
                startSettings.output.audio.pid = hTranscode->transcodeProfile.audio.trackId;
                startSettings.output.audio.passthrough = hTranscode->transcodeProfile.audio.passthrough;
                if ( hTranscode->transcodeProfile.audio.passthrough == true )
                {
                    startSettings.output.audio.codec = hTranscode->prepareApi.settings.audioTrack.info.audio.codec;
                }
                else
                {
                    startSettings.output.audio.codec = hTranscode->transcodeProfile.audio.audioCodec;
                }
            }

            if ( hTranscode->transcodeProfile.containerType == NEXUS_TransportType_eTs )
            {
                startSettings.output.transport.pmtPid       = hTranscode->transcodeProfile.mpeg2Ts.pmtPid;
                startSettings.output.transport.interval     = hTranscode->transcodeProfile.mpeg2Ts.pmtIntervalInMs;
                startSettings.output.transport.pcrPid       = hTranscode->transcodeProfile.mpeg2Ts.pcrPid;
            }
            {
                startSettings.output.video.settings.nonRealTime = hTranscode->prepareApi.nonRealTime;
            }

            BIP_MSG_SUM(( BIP_MSG_PRE_FMT "hVideo=%p hAudio=%p hRecpump=%p" BIP_MSG_PRE_ARG,
                        (void *)startSettings.input.video, (void *)startSettings.input.audio, (void *)startSettings.recpump
                     ));
            BIP_MSG_SUM(( BIP_MSG_PRE_FMT "vCodec=%d level=%d profile=%d interlaced=%s rateBufferDel=%u NRT=%s adaptiveLowDelay=%s lowDelay=%s"
                        BIP_MSG_PRE_ARG,
                        startSettings.output.video.settings.codec,
                        startSettings.output.video.settings.level,
                        startSettings.output.video.settings.profile,
                        startSettings.output.video.settings.interlaced?"Y":"N",
                        startSettings.output.video.settings.rateBufferDelay,
                        startSettings.output.video.settings.nonRealTime?"Y":"N",
                        startSettings.output.video.settings.adaptiveLowDelayMode?"Y":"N",
                        startSettings.output.video.settings.lowDelayPipeline?"Y":"N"
                     ));
            BIP_MSG_SUM(( BIP_MSG_PRE_FMT "transportType=%d pcrPid=%d pmtPid=%d interval=%d" BIP_MSG_PRE_ARG,
                        startSettings.output.transport.type, startSettings.output.transport.pcrPid,
                        startSettings.output.transport.pmtPid, startSettings.output.transport.interval
                        ));

            /* Save the start settings for starting the encoder later during start. */
            hTranscode->encoderStartSettings = startSettings;
        }
    }
    bipStatus = BIP_SUCCESS;

error:
    return ( bipStatus );
} /* prepareTranscodePath */

static void stopTranscodePath(
    BIP_Transcode *hTranscode
    )
{
    /* Stop Transcode. */
    if ( hTranscode->encoderStarted )
    {
        NEXUS_SimpleEncoder_Stop( hTranscode->nexusHandles.simple.hEncoder );
        BDBG_MSG(( BIP_MSG_PRE_FMT "Transcode (%p) Stopped" BIP_MSG_PRE_ARG, (void *)hTranscode->nexusHandles.simple.hEncoder ));
        hTranscode->encoderStarted = false;
    }

    /* Stop Video Decoder. */
    if ( hTranscode->videoDecoderStarted )
    {
        NEXUS_SimpleVideoDecoder_Stop( hTranscode->nexusHandles.simple.hVideo );
        BDBG_MSG(( BIP_MSG_PRE_FMT "Video Decoder (%p) Stopped" BIP_MSG_PRE_ARG, (void *)hTranscode->nexusHandles.simple.hVideo ));
        hTranscode->videoDecoderStarted = false;
    }

    /* Stop Audio Decoder. */
    if ( hTranscode->audioDecoderStarted )
    {
        NEXUS_SimpleAudioDecoder_Stop( hTranscode->nexusHandles.simple.hAudio );
        BDBG_MSG(( BIP_MSG_PRE_FMT "Audio Decoder (%p) Stopped" BIP_MSG_PRE_ARG, (void *)hTranscode->nexusHandles.simple.hAudio ));
        hTranscode->audioDecoderStarted = false;
    }

    if ( hTranscode->nexusHandles.simple.hStcChannel )
    {
        if (NEXUS_SimpleStcChannel_Invalidate( hTranscode->nexusHandles.simple.hStcChannel ) != NEXUS_SUCCESS)
            BDBG_WRN(( BIP_MSG_PRE_FMT "NEXUS_SimpleStcChannel_Invalidate Failed!" BIP_MSG_PRE_ARG ));
    }

} /* stopTranscodePath */

static BIP_Status startTranscodePath(
    BIP_TranscodeHandle     hTranscode
    )
{
    BIP_Status bipStatus = BIP_ERR_INTERNAL;
    NEXUS_Error nrc;

    /* Start Transcode. */
    nrc = NEXUS_SimpleEncoder_Start( hTranscode->nexusHandles.simple.hEncoder, &hTranscode->encoderStartSettings );
    BIP_CHECK_GOTO(( nrc == NEXUS_SUCCESS ), ( "NEXUS_SimpleEncoder_Start Failed!" ), error, BIP_ERR_NEXUS, bipStatus );
    hTranscode->encoderStarted = true;
    BDBG_MSG(( BIP_MSG_PRE_FMT "Transcode (%p) Started" BIP_MSG_PRE_ARG, (void *)hTranscode->nexusHandles.simple.hEncoder ));

    /* Start Video Decoder. */
    if ( !hTranscode->transcodeProfile.disableVideo )
    {
        nrc = NEXUS_SimpleVideoDecoder_Start( hTranscode->nexusHandles.simple.hVideo, &hTranscode->videoProgram );
        BIP_CHECK_GOTO(( nrc == NEXUS_SUCCESS ), ( "NEXUS_SimpleVideoDecoder_Start Failed!" ), error, BIP_ERR_NEXUS, bipStatus );
        hTranscode->videoDecoderStarted = true;
        BDBG_MSG(( BIP_MSG_PRE_FMT "Video Decoder (%p) Started" BIP_MSG_PRE_ARG, (void *)hTranscode->nexusHandles.simple.hVideo ));
    }

    /* Start Audio Decoder. */
    if ( !hTranscode->transcodeProfile.disableAudio )
    {
        nrc = NEXUS_SimpleAudioDecoder_Start( hTranscode->nexusHandles.simple.hAudio, &hTranscode->audioProgram );
        BIP_CHECK_GOTO(( nrc == NEXUS_SUCCESS ), ( "NEXUS_SimpleAudioDecoder_Start Failed!" ), error, BIP_ERR_NEXUS, bipStatus );
        hTranscode->audioDecoderStarted = true;
        BDBG_MSG(( BIP_MSG_PRE_FMT "Audio Decoder (%p) Started" BIP_MSG_PRE_ARG, (void *)hTranscode->nexusHandles.simple.hAudio ));
    }
    bipStatus = BIP_SUCCESS;
error:
    if ( bipStatus != BIP_SUCCESS ) stopTranscodePath( hTranscode );
#ifdef UNIT_TEST
    bipStatus = BIP_ERR_INTERNAL;
#endif
    return ( bipStatus );
} /* startTranscodePath */

static void releaseTranscodeHandles(
    BIP_TranscodeHandle hTranscode
    )
{
#if NXCLIENT_SUPPORT
    if ( !hTranscode->openedNexusHandles ) return;

    BDBG_MSG(( BIP_MSG_PRE_FMT " Release Xcode related handles: hVideo=%p hAudio=%p hEncoder=%p hStcChannel=%p" BIP_MSG_PRE_ARG,
                (void *)hTranscode->nexusHandles.simple.hVideo,
                (void *)hTranscode->nexusHandles.simple.hAudio,
                (void *)hTranscode->nexusHandles.simple.hEncoder,
                (void *)hTranscode->nexusHandles.simple.hStcChannel
             ));

    if ( hTranscode->nexusHandles.simple.hEncoder )
        NEXUS_SimpleEncoder_Release( hTranscode->nexusHandles.simple.hEncoder );
    hTranscode->nexusHandles.simple.hEncoder = NULL;

    if ( hTranscode->nexusHandles.simple.hVideo )
        NEXUS_SimpleVideoDecoder_Release( hTranscode->nexusHandles.simple.hVideo );
    hTranscode->nexusHandles.simple.hVideo = NULL;

    if ( hTranscode->nexusHandles.simple.hAudio )
        NEXUS_SimpleAudioDecoder_Release( hTranscode->nexusHandles.simple.hAudio );
    hTranscode->nexusHandles.simple.hAudio = NULL;

    if ( hTranscode->nexusHandles.simple.hStcChannel )
        NEXUS_SimpleStcChannel_Destroy( hTranscode->nexusHandles.simple.hStcChannel );
    hTranscode->nexusHandles.simple.hStcChannel = NULL;

    if ( hTranscode->nxClientConnected ) NxClient_Disconnect( hTranscode->connectId );
    hTranscode->nxClientConnected = false;

#else /* NXCLIENT_SUPPORT */
    BSTD_UNUSED( hTranscode );
#endif
} /* releaseTranscodeHandles */

static BIP_Status acquireTranscodeHandles(
    BIP_TranscodeHandle     hTranscode
    )
{
    BIP_Status bipStatus;
#if NXCLIENT_SUPPORT
    NEXUS_Error                 nrc;
    NxClient_AllocSettings      allocSettings;
    NxClient_AllocResults       allocResults;
    NxClient_ConnectSettings    connectSettings;

    if ( hTranscode->prepareApi.settings.pNexusHandles )
    {
        /* Caller already provided the Nexus Handles, so we cache them!. */
        hTranscode->nexusHandles = *hTranscode->prepareApi.settings.pNexusHandles;
        hTranscode->openedNexusHandles = false; /* since we didn't open the handles, we wont need to close them!. */
        BDBG_MSG(( BIP_MSG_PRE_FMT "hTranscode %p: Using App provided NexusHandles" BIP_MSG_PRE_ARG, (void *)hTranscode ));
        return ( BIP_SUCCESS );
    }

    /* Caller didn't provide the Nexus Handles, we will open/acquire them as needed. */
    BIP_Transcode_GetDefaultNexusHandles( &hTranscode->nexusHandles );

    NxClient_GetDefaultAllocSettings(&allocSettings);
    allocSettings.simpleVideoDecoder = hTranscode->prepareApi.settings.hVideoPidChannel ? true:false;
    allocSettings.simpleAudioDecoder = hTranscode->prepareApi.settings.hAudioPidChannel ? true:false;
    allocSettings.simpleEncoder = true;
    nrc = NxClient_Alloc( &allocSettings, &allocResults );
    BIP_CHECK_GOTO(( nrc == NEXUS_SUCCESS ), ( "NxClient_Alloc Failed" ), error, BIP_INF_NEXUS_RESOURCE_NOT_AVAILABLE, bipStatus );

    NxClient_GetDefaultConnectSettings( &connectSettings );
    if ( hTranscode->prepareApi.settings.hVideoPidChannel )
    {
        connectSettings.simpleVideoDecoder[0].id = allocResults.simpleVideoDecoder[0].id;
        connectSettings.simpleVideoDecoder[0].windowCapabilities.encoder = true;
        connectSettings.simpleVideoDecoder[0].decoderCapabilities.maxWidth = hTranscode->prepareApi.settings.videoTrack.info.video.width;
        connectSettings.simpleVideoDecoder[0].decoderCapabilities.maxHeight = hTranscode->prepareApi.settings.videoTrack.info.video.height;
        connectSettings.simpleVideoDecoder[0].decoderCapabilities.supportedCodecs[hTranscode->prepareApi.settings.videoTrack.info.video.codec] = true;
    }
    if ( hTranscode->prepareApi.settings.hAudioPidChannel )
    {
        connectSettings.simpleAudioDecoder.id = allocResults.simpleAudioDecoder.id;
    }

    connectSettings.simpleEncoder[0].id = allocResults.simpleEncoder[0].id;
    connectSettings.simpleEncoder[0].nonRealTime = hTranscode->prepareApi.nonRealTime;

    nrc = NxClient_Connect(&connectSettings, &hTranscode->connectId);
    BIP_CHECK_GOTO(( nrc == NEXUS_SUCCESS ), ( "NxClient_Connect Failed" ), error, BIP_INF_NEXUS_RESOURCE_NOT_AVAILABLE, bipStatus );

    hTranscode->nxClientConnected = true;

    /* Successfully connected to the NxServer. Acquire AV Decoder & Transcode Handles. */
    if (allocResults.simpleVideoDecoder[0].id)
    {
        hTranscode->nexusHandles.simple.hVideo = NEXUS_SimpleVideoDecoder_Acquire(allocResults.simpleVideoDecoder[0].id);
        BIP_CHECK_GOTO(( hTranscode->nexusHandles.simple.hVideo ), ( "NEXUS_SimpleVideoDecoder_Acquire Failed" ), error, BIP_INF_NEXUS_RESOURCE_NOT_AVAILABLE, bipStatus );
    }
    if (allocResults.simpleAudioDecoder.id)
    {
        hTranscode->nexusHandles.simple.hAudio = NEXUS_SimpleAudioDecoder_Acquire(allocResults.simpleAudioDecoder.id);
        BIP_CHECK_GOTO(( hTranscode->nexusHandles.simple.hAudio ), ( "NEXUS_SimpleAudioDecoder_Acquire Failed" ), error, BIP_INF_NEXUS_RESOURCE_NOT_AVAILABLE, bipStatus );
    }
    hTranscode->nexusHandles.simple.hEncoder = NEXUS_SimpleEncoder_Acquire(allocResults.simpleEncoder[0].id);
    BIP_CHECK_GOTO(( hTranscode->nexusHandles.simple.hEncoder ), ( "NEXUS_SimpleEncoder_Acquire Failed" ), error, BIP_INF_NEXUS_RESOURCE_NOT_AVAILABLE, bipStatus );

    hTranscode->nexusHandles.simple.hStcChannel = NEXUS_SimpleStcChannel_Create(NULL);
    BIP_CHECK_GOTO(( hTranscode->nexusHandles.simple.hStcChannel ), ( "NEXUS_SimpleStcChannel_Create Failed" ), error, BIP_INF_NEXUS_RESOURCE_NOT_AVAILABLE, bipStatus );

    /* We have successfully acquired all needed resources. */
    hTranscode->nexusHandles.useSimpleHandles = true;
    hTranscode->openedNexusHandles = true;

    bipStatus = BIP_SUCCESS;
    BDBG_MSG(( BIP_MSG_PRE_FMT " Successfully Acquired Xcode related handles: hVideo=%p hAudio=%p hEncoder=%p hStcChannel=%p" BIP_MSG_PRE_ARG,
                (void *)hTranscode->nexusHandles.simple.hVideo,
                (void *)hTranscode->nexusHandles.simple.hAudio,
                (void *)hTranscode->nexusHandles.simple.hEncoder,
                (void *)hTranscode->nexusHandles.simple.hStcChannel
                ));

error:
    if ( bipStatus != BIP_SUCCESS ) releaseTranscodeHandles( hTranscode );
#else
    BSTD_UNUSED( hTranscode );
    BSTD_UNUSED( releaseTranscodeHandles );
    BDBG_WRN(( BIP_MSG_PRE_FMT "BIP currently only supports Xcode using NxClient. Please comiple it with NXCLIENT_SUPPORT=y" BIP_MSG_PRE_ARG ));
    bipStatus = BIP_ERR_NOT_AVAILABLE;
#endif

    return ( bipStatus );
} /* acquireTranscodeHandles */

BIP_Status updateTranscodeSettings(
    BIP_TranscodeHandle hTranscode,
    BIP_TranscodeSettings *pSettings
    )
{
    BIP_Status bipStatus = BIP_SUCCESS;
    NEXUS_Error nrc;

    if ( pSettings->pTranscodeProfile )
    {
        /* Caller wants us to use this transcodeProfile, update our current one to this. */
        hTranscode->transcodeProfile = *pSettings->pTranscodeProfile;
    }
    if ( hTranscode->state != BIP_TranscodeState_eTranscoding ) return ( BIP_SUCCESS ); /* Settings are saved, we are done! */

    if ( pSettings->flush )
    {
        BDBG_MSG(( BIP_MSG_PRE_FMT "hTranscode=%p: flush started" BIP_MSG_PRE_ARG, (void *)hTranscode ));

        /* We need to stop & start the xcode pipe. */
        stopTranscodePath( hTranscode );

        NEXUS_Recpump_Stop( hTranscode->prepareApi.hRecpump );

        bipStatus = prepareTranscodePath( hTranscode );
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "prepareTranscodePath Failed" ), error, bipStatus, bipStatus );

        bipStatus = startTranscodePath( hTranscode );
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "startTranscodePath Failed" ), error, bipStatus, bipStatus );

        BDBG_MSG(( BIP_MSG_PRE_FMT "hTranscode=%p: flush completed!" BIP_MSG_PRE_ARG, (void *)hTranscode ));
    }
    else
    {
        NEXUS_SimpleEncoderSettings encoderSettings;

        /* Not flushing the xcode, so just need to adjust the Encoder runtime settings using provided transcode profile. */
        NEXUS_SimpleEncoder_GetSettings( hTranscode->nexusHandles.simple.hEncoder, &encoderSettings );
        encoderSettings.videoEncoder.bitrateMax = hTranscode->transcodeProfile.video.settings.bitrateMax;
        encoderSettings.videoEncoder.frameRate = hTranscode->transcodeProfile.video.settings.frameRate;
        encoderSettings.video.width = hTranscode->transcodeProfile.video.width;
        encoderSettings.video.height = hTranscode->transcodeProfile.video.height;
        nrc = NEXUS_SimpleEncoder_SetSettings( hTranscode->nexusHandles.simple.hEncoder, &encoderSettings );
        BIP_CHECK_GOTO(( nrc == NEXUS_SUCCESS ), ( "NEXUS_SimpleEncoder_SetSettings Failed to update the runtime transcode profile!" ), error, BIP_ERR_NEXUS, bipStatus );
        bipStatus = BIP_SUCCESS;
        BDBG_MSG(( BIP_MSG_PRE_FMT "hTranscode=%p: Successfully updated the runtime transcode settings: bitrate=%u wxh=%ux%u!" BIP_MSG_PRE_ARG,
                    (void *)hTranscode, encoderSettings.videoEncoder.bitrateMax, encoderSettings.video.width, encoderSettings.video.height ));
    }

error:
    return ( bipStatus );
}

void processTranscodeState(
    void *hObject,
    int value,
    BIP_Arb_ThreadOrigin threadOrigin
    )
{
    BIP_TranscodeHandle hTranscode = hObject;               /* Transcode object handle */
    BIP_ArbHandle           hArb;
    BIP_Status              brc = BIP_ERR_INTERNAL;
    BIP_Status              completionStatus = BIP_ERR_INTERNAL;

    BSTD_UNUSED(value);

    BDBG_ASSERT(hTranscode);
    BDBG_OBJECT_ASSERT( hTranscode, BIP_Transcode);

    /*
     ***************************************************************************************************************
     * Transcode State Machine Processing:
     *
     * Note: Transcode Settings related APIs are required to be called before the _Transcode_Start().
     * These are _Set*Input, _SetOutput, _AddTracks, _SetProgram, _SetResponseHeaders, etc.
     * In these APIs, we will just cache the caller provided settings but not acquire any
     * Nexus Resources needed for streaming. Once _Start() is called, then we will acquire & setup
     * the required resources for streaming from a input to a particular output method.
     *
     ***************************************************************************************************************
     */

    B_Mutex_Lock( hTranscode->hStateMutex );
    BDBG_MSG(( BIP_MSG_PRE_FMT "ENTRY ---> hTranscode %p: state %s"
                BIP_MSG_PRE_ARG, (void *)hTranscode, BIP_TRANSCODE_STATE(hTranscode->state) ));

    if (BIP_Arb_IsNew(hArb = hTranscode->getSettingsApi.hArb))
    {
        /* App is request current Transcode settings. */
        BIP_Arb_AcceptRequest(hArb);

        /* Return the current cached settings. */
        if ( hTranscode->getSettingsApi.pSettings->pTranscodeProfile )
            *hTranscode->getSettingsApi.pSettings->pTranscodeProfile = hTranscode->transcodeProfile;
        hTranscode->getSettingsApi.pSettings->flush  = false;

        /* We are done this API Arb, so set its completion status. */
        hTranscode->completionStatus = BIP_SUCCESS;
        BDBG_MSG(( BIP_MSG_PRE_FMT "hTranscode %p: GetSettings Arb request is complete: state %s!"
                    BIP_MSG_PRE_ARG, (void *)hTranscode, BIP_TRANSCODE_STATE(hTranscode->state) ));
        BIP_Arb_CompleteRequest( hArb, hTranscode->completionStatus);
    }
    else if (BIP_Arb_IsNew(hArb = hTranscode->getStatusApi.hArb))
    {
        /* App is request current Transcode settings. */
        BIP_Arb_AcceptRequest(hArb);

        /* Return the current status. */
#if 0
        hTranscode->getStatusApi.pStatus->transcodeActive = hTranscode->state == BIP_TranscodeState_eTranscoding ? true: false;
        hTranscode->getStatusApi.pStatus->stats = hTranscode->stats;

        /* We are done this API Arb, so set its completion status. */
#endif
        hTranscode->completionStatus = BIP_SUCCESS;
        BDBG_MSG(( BIP_MSG_PRE_FMT "hTranscode %p: GetStatus Arb request is complete: state %s!"
                    BIP_MSG_PRE_ARG, (void *)hTranscode, BIP_TRANSCODE_STATE(hTranscode->state) ));
        BIP_Arb_CompleteRequest( hArb, hTranscode->completionStatus);
    }
    else if (BIP_Arb_IsNew(hArb = hTranscode->printStatusApi.hArb))
    {
        /* App is request to print Transcode stats. */
        BIP_Arb_AcceptRequest(hArb);

        printTranscodeStatus( hTranscode );

        /* We are done this API Arb, so set its completion status. */
        hTranscode->completionStatus = BIP_SUCCESS;
        BDBG_MSG(( BIP_MSG_PRE_FMT "hTranscode %p: printStatus Arb request is complete: state %s!"
                    BIP_MSG_PRE_ARG, (void *)hTranscode, BIP_TRANSCODE_STATE(hTranscode->state) ));
        BIP_Arb_CompleteRequest( hArb, hTranscode->completionStatus);
    }
    else if (BIP_Arb_IsNew(hArb = hTranscode->setSettingsApi.hArb))
    {
        BIP_Arb_AcceptRequest(hArb);

        hTranscode->completionStatus = updateTranscodeSettings( hTranscode, hTranscode->setSettingsApi.pSettings );

        BDBG_MSG(( BIP_MSG_PRE_FMT "hTranscode %p: SetSettings Arb request is complete : state %s, status=%s"
                    BIP_MSG_PRE_ARG, (void *)hTranscode, BIP_TRANSCODE_STATE(hTranscode->state), BIP_StatusGetText( hTranscode->completionStatus) ));
        BIP_Arb_CompleteRequest( hArb, hTranscode->completionStatus);
    }
    else if (BIP_Arb_IsNew(hArb = hTranscode->prepareApi.hArb))
    {
        /* App is starting the Server, make sure we are in the correct state to do so! */
        if (hTranscode->state != BIP_TranscodeState_eIdle)
        {
            BDBG_ERR(( BIP_MSG_PRE_FMT "hTranscode %p: BIP_Transcode_Start() is only allowed in Idle state, current state: %s"
                        BIP_MSG_PRE_ARG, (void *)hTranscode, BIP_TRANSCODE_STATE(hTranscode->state)));
            hTranscode->completionStatus = BIP_ERR_INVALID_API_SEQUENCE;
            BIP_Arb_RejectRequest(hArb, hTranscode->completionStatus);
        }
        else
        {
            BIP_Arb_AcceptRequest(hArb);
            hTranscode->completionStatus = BIP_INF_IN_PROGRESS;

            hTranscode->transcodeProfile = *hTranscode->prepareApi.pTranscodeProfile;
            hTranscode->completionStatus = acquireTranscodeHandles( hTranscode );
            if ( hTranscode->completionStatus == BIP_SUCCESS )
            {
                hTranscode->completionStatus = prepareTranscodePath( hTranscode );
                BDBG_MSG(( BIP_MSG_PRE_FMT "hTranscode %p: BIP_Transcode_Prepare successful: state %s!" BIP_MSG_PRE_ARG, (void *)hTranscode, BIP_TRANSCODE_STATE(hTranscode->state) ));
            }
            if ( hTranscode->completionStatus == BIP_SUCCESS )
            {
                hTranscode->state = BIP_TranscodeState_ePrepared;
                BDBG_MSG(( BIP_MSG_PRE_FMT "hTranscode %p: BIP_Transcode_Prepare successful: state %s!" BIP_MSG_PRE_ARG, (void *)hTranscode, BIP_TRANSCODE_STATE(hTranscode->state) ));
            }
            else
            {
                BDBG_ERR(( BIP_MSG_PRE_FMT "hTranscode %p: BIP_Transcode_Prepare() Failed: current state: %s" BIP_MSG_PRE_ARG, (void *)hTranscode, BIP_TRANSCODE_STATE(hTranscode->state)));
                releaseTranscodeHandles( hTranscode );
            }
            BIP_Arb_CompleteRequest(hArb, hTranscode->completionStatus);
        }
    }
    else if (BIP_Arb_IsNew(hArb = hTranscode->startApi.hArb))
    {
        /* Caller is starting transcode, we must be in the Prepared state. */
        if (hTranscode->state != BIP_TranscodeState_ePrepared)
        {
            BDBG_ERR(( BIP_MSG_PRE_FMT "hTranscode %p: BIP_Transcode_Start() is only allowed in Prepared state, current state: %s" BIP_MSG_PRE_ARG, (void *)hTranscode, BIP_TRANSCODE_STATE(hTranscode->state)));
            hTranscode->completionStatus = BIP_ERR_INVALID_API_SEQUENCE;
            BIP_Arb_RejectRequest(hArb, hTranscode->completionStatus);
        }
        else
        {
            BIP_Arb_AcceptRequest(hArb);
            hTranscode->completionStatus = BIP_INF_IN_PROGRESS;

            /* We have confirmed that all streaming related states are valid and thus their settings are in place. */
            /* All streaming resources are already setup in the prepared state, so lets start transcode. */
            hTranscode->startSettings = *hTranscode->startApi.pSettings;

            hTranscode->completionStatus = startTranscodePath( hTranscode );
            if ( hTranscode->completionStatus == BIP_SUCCESS )
            {
                /* NOTE: we change to a temporary SetupComplete state (w/o completing the ARB) */
                /* This way we let the per streaming protocol state logic below do any streaming start related processing. */
                hTranscode->state = BIP_TranscodeState_eTranscoding;
                BIP_MSG_SUM(( BIP_MSG_PRE_FMT "TranscodeStarted: " BIP_TRANSCODE_PRINTF_FMT
                            BIP_MSG_PRE_ARG, BIP_TRANSCODE_PRINTF_ARG(hTranscode)));
            }
            else
            {
                BDBG_ERR(( BIP_MSG_PRE_FMT "hTranscode %p: BIP_Transcode_Start() Failed: current state: %s" BIP_MSG_PRE_ARG, (void *)hTranscode, BIP_TRANSCODE_STATE(hTranscode->state)));
            }
            BIP_Arb_CompleteRequest(hArb, hTranscode->completionStatus);
        }
    }
    else if (BIP_Arb_IsNew(hArb = hTranscode->stopApi.hArb))
    {
        {
            BIP_Arb_AcceptRequest(hArb);
            hTranscode->completionStatus = BIP_INF_IN_PROGRESS;

            /* Note: we also release Nexus xcode resources during Transcode_Stop() as these resources were allocated during Transcode_Prepare(), before the Transcode_Start */
            stopTranscodePath( hTranscode );
            releaseTranscodeHandles( hTranscode );
            hTranscode->completionStatus = BIP_SUCCESS;

            hTranscode->state = BIP_TranscodeState_eIdle;
            BIP_Arb_CompleteRequest(hArb, hTranscode->completionStatus);
            BDBG_MSG(( BIP_MSG_PRE_FMT "hTranscode %p: BIP_Transcode_Stop complete: state %s!"
                        BIP_MSG_PRE_ARG, (void *)hTranscode, BIP_TRANSCODE_STATE(hTranscode->state) ));

        }
    }
    else if (BIP_Arb_IsNew(hArb = hTranscode->destroyApi.hArb))
    {
        BIP_Arb_AcceptRequest(hArb);
        hTranscode->completionStatus = BIP_INF_IN_PROGRESS;

        BDBG_MSG(( BIP_MSG_PRE_FMT "hTranscode %p: Accepted _Destroy Arb: state %s!"
                    BIP_MSG_PRE_ARG, (void *)hTranscode, BIP_TRANSCODE_STATE(hTranscode->state) ));

        if ( hTranscode->state != BIP_TranscodeState_eIdle )
        {
            /* Transcode_Stop() wasn't called. */
            stopTranscodePath( hTranscode );
            releaseTranscodeHandles( hTranscode );
        }
        BIP_Arb_CompleteRequest(hArb, BIP_SUCCESS);
    }

    /*
     * Done with state processing. We have to unlock state machine before asking Arb to do any deferred callbacks!
     */
    B_Mutex_Unlock( hTranscode->hStateMutex );

    BDBG_MSG(( BIP_MSG_PRE_FMT "Finished Processing  State for hTranscode %p: state %s, before issuing the callbacks with completionStatus 0x%x"
            BIP_MSG_PRE_ARG, (void *)hTranscode, BIP_TRANSCODE_STATE(hTranscode->state), completionStatus ));

    /* Tell ARB to do any deferred work. */
    brc = BIP_Arb_DoDeferred( hTranscode->startApi.hArb, threadOrigin );
    BDBG_ASSERT( brc == BIP_SUCCESS );


    BDBG_MSG(( BIP_MSG_PRE_FMT "EXIT <--- hTranscode %p: state %s"
                BIP_MSG_PRE_ARG, (void *)hTranscode, BIP_TRANSCODE_STATE(hTranscode->state)));
    return;
} /* processTranscodeState */

void BIP_Transcode_GetDefaultSettings(
    BIP_TranscodeSettings *pSettings
    )
{
    BKNI_Memset( pSettings, 0, sizeof( BIP_TranscodeSettings ));
}

static void transcodeDestroy(
    BIP_TranscodeHandle hTranscode
    )
{
    if (!hTranscode) return;
    BDBG_MSG(( BIP_MSG_PRE_FMT "Destroying hTranscode %p" BIP_MSG_PRE_ARG, (void *)hTranscode ));

    if (hTranscode->getStatusApi.hArb) BIP_Arb_Destroy(hTranscode->getStatusApi.hArb);
    if (hTranscode->printStatusApi.hArb) BIP_Arb_Destroy(hTranscode->printStatusApi.hArb);
    if (hTranscode->setSettingsApi.hArb) BIP_Arb_Destroy(hTranscode->setSettingsApi.hArb);
    if (hTranscode->getSettingsApi.hArb) BIP_Arb_Destroy(hTranscode->getSettingsApi.hArb);
    if (hTranscode->startApi.hArb) BIP_Arb_Destroy(hTranscode->startApi.hArb);
    if (hTranscode->prepareApi.hArb) BIP_Arb_Destroy(hTranscode->prepareApi.hArb);
    if (hTranscode->stopApi.hArb) BIP_Arb_Destroy(hTranscode->stopApi.hArb);
    if (hTranscode->destroyApi.hArb) BIP_Arb_Destroy(hTranscode->destroyApi.hArb);

    if (hTranscode->hStateMutex) B_Mutex_Destroy( hTranscode->hStateMutex );

    BDBG_MSG(( BIP_MSG_PRE_FMT "hTranscode %p: Destroyed" BIP_MSG_PRE_ARG, (void *)hTranscode ));
    BDBG_OBJECT_DESTROY( hTranscode, BIP_Transcode );

    B_Os_Free( hTranscode );

} /* transcodeDestroy */

BIP_TranscodeHandle BIP_Transcode_Create(
    const BIP_TranscodeCreateSettings *pCreateSettings
    )
{
    BIP_Status                      bipStatus;
    BIP_TranscodeHandle             hTranscode = NULL;
    BIP_TranscodeCreateSettings     defaultSettings;

    BIP_SETTINGS_ASSERT(pCreateSettings, BIP_TranscodeCreateSettings);

    /* Create the transcode object */
    hTranscode = B_Os_Calloc( 1, sizeof( BIP_Transcode ));
    BIP_CHECK_GOTO(( hTranscode != NULL ), ( "Failed to allocate memory (%d bytes) for Transcode Object", sizeof(BIP_Transcode) ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    BDBG_OBJECT_SET( hTranscode, BIP_Transcode );

    /* Create mutex to synchronize state machine from being run via callbacks & Caller calling APIs. */
    hTranscode->hStateMutex = B_Mutex_Create(NULL);
    BIP_CHECK_GOTO(( hTranscode->hStateMutex ), ( "B_Mutex_Create Failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    if (NULL == pCreateSettings)
    {
        BIP_Transcode_GetDefaultCreateSettings( &defaultSettings );
        pCreateSettings = &defaultSettings;
    }
    hTranscode->createSettings = *pCreateSettings;

    BIP_Transcode_GetDefaultSettings(&hTranscode->settings);

    /* Create API ARBs: one per API */
    hTranscode->getSettingsApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hTranscode->getSettingsApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    hTranscode->setSettingsApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hTranscode->setSettingsApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    hTranscode->getStatusApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hTranscode->getStatusApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    hTranscode->printStatusApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hTranscode->printStatusApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    hTranscode->startApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hTranscode->startApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    hTranscode->prepareApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hTranscode->prepareApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    hTranscode->stopApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hTranscode->stopApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    hTranscode->destroyApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hTranscode->destroyApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    hTranscode->state = BIP_TranscodeState_eIdle;

    bipStatus = BIP_SUCCESS;

    BDBG_MSG((    BIP_MSG_PRE_FMT "Created: " BIP_TRANSCODE_PRINTF_FMT
                  BIP_MSG_PRE_ARG, BIP_TRANSCODE_PRINTF_ARG(hTranscode)));
    BIP_MSG_TRC(( BIP_MSG_PRE_FMT "Created: " BIP_TRANSCODE_PRINTF_FMT
                  BIP_MSG_PRE_ARG, BIP_TRANSCODE_PRINTF_ARG(hTranscode)));

    return ( hTranscode );

error:
    return ( NULL );
} /* BIP_Transcode_Create */

/**
 * Summary:
 * Destroy http socket
 *
 * Description:
 **/
void BIP_Transcode_Destroy(
    BIP_TranscodeHandle     hTranscode
    )
{
    BIP_Status bipStatus = BIP_SUCCESS;
    BIP_ArbHandle hArb;
    BIP_ArbSubmitSettings arbSettings;

    BDBG_OBJECT_ASSERT( hTranscode, BIP_Transcode );

    BIP_MSG_TRC(( BIP_MSG_PRE_FMT "Destroying: " BIP_TRANSCODE_PRINTF_FMT
                  BIP_MSG_PRE_ARG, BIP_TRANSCODE_PRINTF_ARG(hTranscode)));

    hArb = hTranscode->destroyApi.hArb;
    bipStatus = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error, bipStatus, bipStatus );

    /* Get ready to run the state machine. */
    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hTranscode;
    arbSettings.arbProcessor = processTranscodeState;
    arbSettings.waitIfBusy = true;;

    /* Invoke state machine via the Arb Submit API */
    bipStatus = BIP_Arb_Submit(hArb, &arbSettings, NULL);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_SubmitRequest() Failed for BIP_Transcode_Destroy" ), error, bipStatus, bipStatus );

error:
    transcodeDestroy( hTranscode );

} /* BIP_Transcode_Destroy */

void BIP_Transcode_GetSettings(
        BIP_TranscodeHandle    hTranscode,
        BIP_TranscodeSettings *pSettings
    )
{
    BIP_Status bipStatus = BIP_SUCCESS;
    BIP_ArbHandle hArb;
    BIP_ArbSubmitSettings arbSettings;

    BDBG_OBJECT_ASSERT( hTranscode, BIP_Transcode );

    BDBG_MSG(( BIP_MSG_PRE_FMT "Enter: hTranscode %p: --------------------->" BIP_MSG_PRE_ARG, (void *)hTranscode));

    BIP_CHECK_GOTO(( hTranscode ), ( "hTranscode pointer can't be NULL" ), error, BIP_ERR_INVALID_PARAMETER, bipStatus );
    BIP_CHECK_GOTO(( pSettings ), ( "pSettings can't be NULL" ), error, BIP_ERR_INVALID_PARAMETER, bipStatus );

    /* Serialize access to Settings state among another thread calling the same _GetSettings API. */
    hArb = hTranscode->getSettingsApi.hArb;
    bipStatus = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error, bipStatus, bipStatus );

    /* Move the API arguments into it's argument list so the state machine can find them. */
    hTranscode->getSettingsApi.pSettings = pSettings;

    /* Get ready to run the state machine. */
    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hTranscode;
    arbSettings.arbProcessor = processTranscodeState;
    arbSettings.waitIfBusy = true;;

    /* Invoke state machine via the Arb Submit API */
    bipStatus = BIP_Arb_Submit(hArb, &arbSettings, NULL);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_SubmitRequest() Failed for BIP_Transcode_GetSettings" ), error, bipStatus, bipStatus );

error:
    BDBG_MSG(( BIP_MSG_PRE_FMT "Exit: hTranscode %p: completionStatus 0x%x <--------------------- " BIP_MSG_PRE_ARG, (void *)hTranscode, bipStatus ));

    return;
}

BIP_Status BIP_Transcode_SetSettings(
    BIP_TranscodeHandle    hTranscode,
    BIP_TranscodeSettings  *pSettings
    )
{
    BIP_Status bipStatus = BIP_SUCCESS;
    BIP_ArbHandle hArb;
    BIP_ArbSubmitSettings arbSettings;

    BDBG_OBJECT_ASSERT( hTranscode, BIP_Transcode );
    BDBG_ASSERT( pSettings );

    BDBG_MSG(( BIP_MSG_PRE_FMT "Enter: hTranscode %p: --------------------->" BIP_MSG_PRE_ARG, (void *)hTranscode));

    BIP_CHECK_GOTO(( hTranscode ), ( "hTranscode pointer can't be NULL" ), error, BIP_ERR_INVALID_PARAMETER, bipStatus );
    BIP_CHECK_GOTO(( pSettings ), ( "pSettings can't be NULL" ), error, BIP_ERR_INVALID_PARAMETER, bipStatus );

    /* Serialize access to Settings state among another thread calling the same _SetSettings API. */
    hArb = hTranscode->setSettingsApi.hArb;
    bipStatus = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error, bipStatus, bipStatus );

    /* Move the API arguments into it's argument list so the state machine can find them. */
    hTranscode->setSettingsApi.pSettings = pSettings;

    /* Get ready to run the state machine. */
    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hTranscode;
    arbSettings.arbProcessor = processTranscodeState;
    arbSettings.waitIfBusy = true;;

    /* Invoke state machine via the Arb Submit API */
    bipStatus = BIP_Arb_Submit(hArb, &arbSettings, NULL);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_SubmitRequest() Failed for BIP_Transcode_SetSettings" ), error, bipStatus, bipStatus );

error:
    BDBG_MSG(( BIP_MSG_PRE_FMT "Exit: hTranscode %p: completionStatus 0x%x <--------------------- " BIP_MSG_PRE_ARG, (void *)hTranscode, bipStatus ));

    return( bipStatus );
} /* BIP_Transcode_SetSettings */


BIP_Status BIP_Transcode_Prepare(
    BIP_TranscodeHandle             hTranscode,
    BIP_TranscodeStreamInfo         *pTranscodeStreamInfo,
    BIP_TranscodeProfile            *pTranscodeProfile,
    NEXUS_RecpumpHandle             hRecpump,
    bool                            nonRealTime,
    BIP_TranscodePrepareSettings    *pSettings
    )
{
    BIP_Status bipStatus = BIP_SUCCESS;
    BIP_ArbHandle hArb;
    BIP_ArbSubmitSettings arbSettings;
    BIP_TranscodePrepareSettings defaultPrepareSettings;

    BDBG_OBJECT_ASSERT( hTranscode, BIP_Transcode );
    BIP_SETTINGS_ASSERT(pSettings, BIP_TranscodePrepareSettings);

    BDBG_MSG(( BIP_MSG_PRE_FMT "Enter: hTranscode %p: --------------------->" BIP_MSG_PRE_ARG, (void *)hTranscode));

    BIP_CHECK_GOTO(( hTranscode ), ( "hTranscode pointer can't be NULL" ), error, BIP_ERR_INVALID_PARAMETER, bipStatus );
    BIP_CHECK_GOTO(( pTranscodeStreamInfo ), ( "pTranscodeStreamInfo pointer can't be NULL" ), error, BIP_ERR_INVALID_PARAMETER, bipStatus );
    BIP_CHECK_GOTO(( pTranscodeProfile ), ( "pTranscodeProfile pointer can't be NULL" ), error, BIP_ERR_INVALID_PARAMETER, bipStatus );
    BIP_CHECK_GOTO(( hRecpump ), ( "hRecpump handle can't be NULL" ), error, BIP_ERR_INVALID_PARAMETER, bipStatus );

    if ( !pTranscodeProfile->disableVideo )
    {
        BIP_CHECK_GOTO(( pSettings ), ( "pSettings can't be NULL if video is enabled" ), error, BIP_ERR_INVALID_PARAMETER, bipStatus );
        BIP_CHECK_GOTO(( pSettings->hVideoPidChannel ), ( "hVideoPidChannel && pVideoTrack can't be NULL for encoding Video." ), error, BIP_ERR_INVALID_PARAMETER, bipStatus );
    }
    if ( !pTranscodeProfile->disableAudio )
    {
        BIP_CHECK_GOTO(( pSettings ), ( "pSettings can't be NULL if audio is enabled" ), error, BIP_ERR_INVALID_PARAMETER, bipStatus );
        BIP_CHECK_GOTO(( pSettings->hAudioPidChannel ), ( "hAudioPidChannel && pAudioTrack can't be NULL for encoding Audio." ), error, BIP_ERR_INVALID_PARAMETER, bipStatus );
    }
    if ( pSettings )
    {
        if ( pSettings->pNexusHandles )
        {
            BIP_CHECK_GOTO(( pSettings->pNexusHandles->useSimpleHandles ), ( "Currently Transcode feature is only supported via useSimpleHandles." ), error, BIP_ERR_INVALID_PARAMETER, bipStatus );
            BIP_CHECK_GOTO(( pSettings->pNexusHandles->simple.hEncoder ), ( "Caller has set the useSimpleHandles flag but not provided the hEncoder handle." ), error, BIP_ERR_INVALID_PARAMETER, bipStatus );
            BIP_CHECK_GOTO(( pSettings->pNexusHandles->simple.hVideo ), ( "Caller has set the useSimpleHandles flag but not provided the hVideo handle." ), error, BIP_ERR_INVALID_PARAMETER, bipStatus );
            BIP_CHECK_GOTO(( pSettings->pNexusHandles->simple.hAudio ), ( "Caller has set the useSimpleHandles flag but not provided the hAudio handle." ), error, BIP_ERR_INVALID_PARAMETER, bipStatus );
            BIP_CHECK_GOTO(( pSettings->pNexusHandles->simple.hStcChannel ), ( "Caller has set the useSimpleHandles flag but not provided the hStcChannel handle." ), error, BIP_ERR_INVALID_PARAMETER, bipStatus );
        }
    }
    else
    {
        BIP_Transcode_GetDefaultPrepareSettings( &defaultPrepareSettings );
        pSettings = &defaultPrepareSettings;
    }

    /* Serialize access to Settings state among another thread calling the same _SetSettings API. */
    hArb = hTranscode->prepareApi.hArb;
    bipStatus = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error, bipStatus, bipStatus );

    /* Move the API arguments into it's argument list so the state machine can find them. */
    hTranscode->prepareApi.settings = *pSettings;
    hTranscode->prepareApi.transcodeStreamInfo = *pTranscodeStreamInfo;
    hTranscode->prepareApi.pTranscodeProfile = pTranscodeProfile;
    hTranscode->prepareApi.hRecpump = hRecpump;
    hTranscode->prepareApi.nonRealTime = nonRealTime;

    /* Get ready to run the state machine. */
    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hTranscode;
    arbSettings.arbProcessor = processTranscodeState;
    arbSettings.waitIfBusy = true;;

    /* Invoke state machine via the Arb Submit API */
    bipStatus = BIP_Arb_Submit(hArb, &arbSettings, NULL);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_SubmitRequest() Failed for BIP_Transcode_Prepare" ), error, bipStatus, bipStatus );

    BDBG_MSG(( BIP_MSG_PRE_FMT "hTranscode %p: Transcode Prepareed: completionStatus: %s" BIP_MSG_PRE_ARG, (void *)hTranscode, BIP_StatusGetText(bipStatus) ));

error:
    BDBG_MSG(( BIP_MSG_PRE_FMT "Exit: hTranscode %p: completionStatus 0x%x <--------------------- " BIP_MSG_PRE_ARG, (void *)hTranscode, bipStatus ));

    return ( bipStatus );
} /* BIP_Transcode_Prepare */

BIP_Status BIP_Transcode_Start(
    BIP_TranscodeHandle          hTranscode,
    BIP_TranscodeStartSettings   *pSettings
    )
{
    BIP_Status bipStatus = BIP_SUCCESS;
    BIP_ArbHandle hArb;
    BIP_ArbSubmitSettings arbSettings;
    BIP_TranscodeStartSettings defaultSettings;

    BDBG_OBJECT_ASSERT( hTranscode, BIP_Transcode );
    BIP_SETTINGS_ASSERT(pSettings, BIP_TranscodeStartSettings);

    BDBG_MSG(( BIP_MSG_PRE_FMT "Enter: hTranscode %p: --------------------->" BIP_MSG_PRE_ARG, (void *)hTranscode));

    BIP_CHECK_GOTO(( hTranscode ), ( "hTranscode pointer can't be NULL" ), error, BIP_ERR_INVALID_PARAMETER, bipStatus );

    if ( pSettings == NULL )
    {
        BIP_Transcode_GetDefaultStartSettings( &defaultSettings );
        pSettings = &defaultSettings;
    }
    /* Serialize access to Settings state among another thread calling the same _SetSettings API. */
    hArb = hTranscode->startApi.hArb;
    bipStatus = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error, bipStatus, bipStatus );

    /* Move the API arguments into it's argument list so the state machine can find them. */
    hTranscode->startApi.pSettings = pSettings;

    /* Get ready to run the state machine. */
    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hTranscode;
    arbSettings.arbProcessor = processTranscodeState;
    arbSettings.waitIfBusy = true;;

    /* Invoke state machine via the Arb Submit API */
    bipStatus = BIP_Arb_Submit(hArb, &arbSettings, NULL);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_SubmitRequest() Failed for BIP_Transcode_Start" ), error, bipStatus, bipStatus );

    BDBG_MSG(( BIP_MSG_PRE_FMT "hTranscode %p: Transcode Started: completionStatus: %s" BIP_MSG_PRE_ARG, (void *)hTranscode, BIP_StatusGetText(bipStatus) ));

error:
    BDBG_MSG(( BIP_MSG_PRE_FMT "Exit: hTranscode %p: completionStatus 0x%x <--------------------- " BIP_MSG_PRE_ARG, (void *)hTranscode, bipStatus ));

    return ( bipStatus );
} /* BIP_Transcode_Start */

BIP_Status BIP_Transcode_Stop(
    BIP_TranscodeHandle    hTranscode
    )
{
    BIP_Status bipStatus = BIP_SUCCESS;
    BIP_ArbHandle hArb;
    BIP_ArbSubmitSettings arbSettings;

    BDBG_OBJECT_ASSERT( hTranscode, BIP_Transcode );

    BIP_MSG_TRC(( BIP_MSG_PRE_FMT "Stopping: " BIP_TRANSCODE_PRINTF_FMT
                  BIP_MSG_PRE_ARG, BIP_TRANSCODE_PRINTF_ARG(hTranscode)));
    BDBG_MSG((    BIP_MSG_PRE_FMT "Stopping: " BIP_TRANSCODE_PRINTF_FMT
                  BIP_MSG_PRE_ARG, BIP_TRANSCODE_PRINTF_ARG(hTranscode)));

    BIP_CHECK_GOTO(( hTranscode ), ( "hTranscode pointer can't be NULL" ), error, BIP_ERR_INVALID_PARAMETER, bipStatus );

    /* Serialize access to Settings state among another thread calling the same _SetSettings API. */
    hArb = hTranscode->stopApi.hArb;
    bipStatus = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error, bipStatus, bipStatus );

    /* Get ready to run the state machine. */
    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hTranscode;
    arbSettings.arbProcessor = processTranscodeState;
    arbSettings.waitIfBusy = true;;

    /* Invoke state machine via the Arb Submit API */
    bipStatus = BIP_Arb_Submit(hArb, &arbSettings, NULL);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_SubmitRequest() Failed for BIP_Transcode_Stop" ), error, bipStatus, bipStatus );

error:
    BDBG_MSG(( BIP_MSG_PRE_FMT "Exit: hTranscode %p: completionStatus 0x%x <--------------------- " BIP_MSG_PRE_ARG, (void *)hTranscode, bipStatus ));

    return( bipStatus );
} /* BIP_Transcode_Stop */

BIP_Status  BIP_Transcode_GetStatus(
        BIP_TranscodeHandle  hTranscode,
        BIP_TranscodeStatus  *pStatus
    )
{
    BIP_Status bipStatus = BIP_SUCCESS;
    BIP_ArbHandle hArb;
    BIP_ArbSubmitSettings arbSettings;

    BDBG_OBJECT_ASSERT( hTranscode, BIP_Transcode );

    BDBG_MSG(( BIP_MSG_PRE_FMT "Enter: hTranscode %p: --------------------->" BIP_MSG_PRE_ARG, (void *)hTranscode));

    BIP_CHECK_GOTO(( hTranscode ), ( "hTranscode pointer can't be NULL" ), error, BIP_ERR_INVALID_PARAMETER, bipStatus );
    BIP_CHECK_GOTO(( pStatus ), ( "pStatus can't be NULL" ), error, BIP_ERR_INVALID_PARAMETER, bipStatus );

    /* Serialize access to Settings state among another thread calling the same _GetStatus API. */
    hArb = hTranscode->getStatusApi.hArb;
    bipStatus = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error, bipStatus, bipStatus );

    /* Move the API arguments into it's argument list so the state machine can find them. */
    hTranscode->getStatusApi.pStatus = pStatus;

    /* Get ready to run the state machine. */
    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hTranscode;
    arbSettings.arbProcessor = processTranscodeState;
    arbSettings.waitIfBusy = true;;

    /* Invoke state machine via the Arb Submit API */
    bipStatus = BIP_Arb_Submit(hArb, &arbSettings, NULL);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_SubmitRequest() Failed for BIP_Transcode_GetStatus" ), error, bipStatus, bipStatus );

error:
    BDBG_MSG(( BIP_MSG_PRE_FMT "Exit: hTranscode %p: completionStatus 0x%x <--------------------- " BIP_MSG_PRE_ARG, (void *)hTranscode, bipStatus ));

    return ( bipStatus );
}

void BIP_Transcode_PrintStatus(
    BIP_TranscodeHandle hTranscode
    )
{
    BIP_Status bipStatus;
    BIP_ArbHandle hArb;
    BIP_ArbSubmitSettings arbSettings;

    BDBG_ASSERT( hTranscode );
    BDBG_OBJECT_ASSERT( hTranscode, BIP_Transcode );

    if ( !hTranscode ) return;

    /* Serialize access to Settings state among another thread calling the same _GetSettings API. */
    hArb = hTranscode->printStatusApi.hArb;
    bipStatus = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error, bipStatus, bipStatus );

    /* Get ready to run the state machine. */
    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hTranscode;
    arbSettings.arbProcessor = processTranscodeState;
    arbSettings.waitIfBusy = true;;

    bipStatus = BIP_Arb_Submit(hArb, &arbSettings, NULL);
    BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ( "BIP_Arb_SubmitRequest() Failed" ), error, bipStatus, bipStatus );

error:
    return;
} /* BIP_Transcode_PrintStats */
