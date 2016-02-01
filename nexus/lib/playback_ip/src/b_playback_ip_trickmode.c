/***************************************************************************
*     (c)2003-2016 Broadcom Corporation
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
* Description: File contains RTSP & HTTP TrickMode support
*
* Revision History:
*
* $brcm_Log: $
*
***************************************************************************/
#include "b_os_lib.h"
#include "linuxuser/b_os_time.h"
#include "b_playback_ip_lib.h"
#include "b_playback_ip_priv.h"
#include "b_playback_ip_lm_helper.h"
#include "b_playback_ip_utils.h"
#include "b_playback_ip_psi.h"
#include "nexus_playback.h"
#include "nexus_audio_decoder_trick.h"

BDBG_MODULE(b_playback_ip_trickmode);
#if defined(LINUX) || defined(__vxworks)

static B_PlaybackIpError trickModePlay_locked( B_PlaybackIpHandle playback_ip);
static B_PlaybackIpError trickModeTrick_locked(B_PlaybackIpHandle playback_ip, B_PlaybackIpTrickModesSettings *ipTrickModeSettings);
static B_PlaybackIpError trickModeFrameAdvance_locked( B_PlaybackIpHandle playback_ip, bool forward);
static B_PlaybackIpError trickModeSeek_locked( B_PlaybackIpHandle playback_ip, B_PlaybackIpTrickModesSettings *ipTrickModeSettings);
static void trickModeThread( void *data);

extern B_PlaybackIpError http_do_server_trick_modes_socket_transition( B_PlaybackIpHandle playback_ip, double timeSeekRangeBegin, double timeSeekRangeEnd, int rate, char *playSpeedString);
extern void print_av_pipeline_buffering_status(B_PlaybackIpHandle playback_ip);

#ifdef B_HAS_HLS_PROTOCOL_SUPPORT
extern B_PlaybackIpError B_PlaybackIp_PauseHls(B_PlaybackIpHandle playback_ip, B_PlaybackIpState currentState);
extern B_PlaybackIpError B_PlaybackIp_PlayHls( B_PlaybackIpHandle playback_ip, B_PlaybackIpState currentState);
extern B_PlaybackIpError B_PlaybackIp_SeekHls( B_PlaybackIpHandle playback_ip, B_PlaybackIpState currentState, NEXUS_PlaybackPosition seekPosition, bool accurateSeek, bool flushPipeline);
extern B_PlaybackIpError B_PlaybackIp_TrickModeHls(B_PlaybackIpHandle playback_ip, B_PlaybackIpState currentState, B_PlaybackIpTrickModesSettings *ipTrickModeSettings);
#endif /* B_HAS_HLS_PROTOCOL_SUPPORT */

#ifdef B_HAS_MPEG_DASH_PROTOCOL_SUPPORT
extern B_PlaybackIpError B_PlaybackIp_PauseMpegDash(B_PlaybackIpHandle playback_ip, B_PlaybackIpTrickModesSettings *ipTrickModeSettings);
extern B_PlaybackIpError B_PlaybackIp_PlayMpegDash( B_PlaybackIpHandle playback_ip);
extern B_PlaybackIpError B_PlaybackIp_SeekMpegDash( B_PlaybackIpHandle playback_ip, NEXUS_PlaybackPosition seekPosition);
#endif /* B_HAS_HLS_PROTOCOL_SUPPORT */

/* this function gets the pts value corresponding to the next video or audio frame being played */
B_PlaybackIpError
http_get_current_pts(
    B_PlaybackIpHandle playback_ip,
    unsigned int *currentPts
    )
{
#if 0
    /* TODO: make this a comiple option: or set for HLS streams w/ AV & audio only. I am exploring an alternate way of maintaining the time position. */
    /* Use STC instead of AV PTS for measuring the time position. */
    /* Making this change to support the AV -> Audio only -> AV switch during adaptive streaming! */
    if (playback_ip->nexusHandles.stcChannel)
        NEXUS_StcChannel_GetStc(playback_ip->nexusHandles.stcChannel, currentPts);
    else if (playback_ip->nexusHandles.simpleStcChannel)
        NEXUS_SimpleStcChannel_GetStc(playback_ip->nexusHandles.simpleStcChannel, currentPts);
    else
        /* if no other choice, just re-start from begining */
        *currentPts = 0;
#else

    if (playback_ip->nexusHandles.videoDecoder) {
        NEXUS_VideoDecoderStatus status;
        if ((NEXUS_VideoDecoder_GetStatus(playback_ip->nexusHandles.videoDecoder, &status) != NEXUS_SUCCESS) || (status.ptsType == NEXUS_PtsType_eInterpolatedFromInvalidPTS)) {
            BDBG_ERR(("%s: %s", __FUNCTION__, (status.ptsType == NEXUS_PtsType_eInterpolatedFromInvalidPTS) ? "Got NEXUS_PtsType_eInterpolatedFromInvalidPTS, ignore it":"Failed to get current video PTS"));
            return B_ERROR_UNKNOWN;
        }
        *currentPts = status.pts;
        BDBG_MSG_FLOW(("q depth %d, decoded %d, displayed %d, decode errs %d, display errs %d, decode drops %d, display drops %d, display underflow %d, received %d, pts errs %d\n",
            status.queueDepth, status.numDecoded, status.numDisplayed, status.numDecodeErrors,
            status.numDisplayErrors, status.numDecodeDrops, status.numDisplayDrops, status.numDisplayUnderflows, status.numPicturesReceived, status.ptsErrorCount));
    }
#ifdef NEXUS_HAS_SIMPLE_DECODER
    else if (playback_ip->nexusHandles.simpleVideoDecoder) {
        NEXUS_VideoDecoderStatus status;
        if ((NEXUS_SimpleVideoDecoder_GetStatus(playback_ip->nexusHandles.simpleVideoDecoder, &status) != NEXUS_SUCCESS) || (status.ptsType == NEXUS_PtsType_eInterpolatedFromInvalidPTS)) {
            BDBG_ERR(("%s: %s", __FUNCTION__, (status.ptsType == NEXUS_PtsType_eInterpolatedFromInvalidPTS) ? "Got NEXUS_PtsType_eInterpolatedFromInvalidPTS, ignore it":"Failed to get current video PTS"));
            return B_ERROR_UNKNOWN;
        }
        *currentPts = status.pts;
        BDBG_MSG_FLOW(("q depth %d, decoded %d, displayed %d, decode errs %d, display errs %d, decode drops %d, display drops %d, display underflow %d, received %d, pts errs %d\n",
                    status.queueDepth, status.numDecoded, status.numDisplayed, status.numDecodeErrors,
                    status.numDisplayErrors, status.numDecodeDrops, status.numDisplayDrops, status.numDisplayUnderflows, status.numPicturesReceived, status.ptsErrorCount));
    }
#endif

    /* for Audio only stream, get the PTS of the last decoded Sample */
    else if (playback_ip->nexusHandles.primaryAudioDecoder) {
        NEXUS_AudioDecoderStatus audioStatus;
        if ((NEXUS_AudioDecoder_GetStatus(playback_ip->nexusHandles.primaryAudioDecoder, &audioStatus) != NEXUS_SUCCESS) || (audioStatus.ptsType == NEXUS_PtsType_eInterpolatedFromInvalidPTS)) {
            BDBG_ERR(("%s: %s", __FUNCTION__, (audioStatus.ptsType == NEXUS_PtsType_eInterpolatedFromInvalidPTS) ? "Got NEXUS_PtsType_eInterpolatedFromInvalidPTS, ignore it":"Failed to get current video PTS"));
            return B_ERROR_UNKNOWN;
        }
        *currentPts = audioStatus.pts;
    }
    else if (playback_ip->nexusHandles.secondaryAudioDecoder) {
        NEXUS_AudioDecoderStatus audioStatus;
        if ((NEXUS_AudioDecoder_GetStatus(playback_ip->nexusHandles.secondaryAudioDecoder, &audioStatus) != NEXUS_SUCCESS) || (audioStatus.ptsType == NEXUS_PtsType_eInterpolatedFromInvalidPTS)) {
            BDBG_ERR(("%s: %s", __FUNCTION__, (audioStatus.ptsType == NEXUS_PtsType_eInterpolatedFromInvalidPTS) ? "Got NEXUS_PtsType_eInterpolatedFromInvalidPTS, ignore it":"Failed to get current video PTS"));
            return B_ERROR_UNKNOWN;
        }
        *currentPts = audioStatus.pts;
    }
#ifdef NEXUS_HAS_SIMPLE_DECODER
    else if (playback_ip->nexusHandles.simpleAudioDecoder) {
        NEXUS_AudioDecoderStatus audioStatus;
        if ((NEXUS_SimpleAudioDecoder_GetStatus(playback_ip->nexusHandles.simpleAudioDecoder, &audioStatus) != NEXUS_SUCCESS) || (audioStatus.ptsType == NEXUS_PtsType_eInterpolatedFromInvalidPTS)) {
            BDBG_ERR(("%s: %s", __FUNCTION__, (audioStatus.ptsType == NEXUS_PtsType_eInterpolatedFromInvalidPTS) ? "Got NEXUS_PtsType_eInterpolatedFromInvalidPTS, ignore it":"Failed to get current video PTS"));
            return B_ERROR_UNKNOWN;
        }
        *currentPts = audioStatus.pts;
    }
#endif
    else {
        /* if no other choice, just re-start from begining */
        *currentPts = 0;
    }
#endif
    return B_ERROR_SUCCESS;
}

/* this function gets the pts value corresponding to the next video or audio frame being played */
NEXUS_PlaybackPosition
http_calculate_current_position(B_PlaybackIpHandle playback_ip, unsigned currentPts)
{
    NEXUS_PlaybackPosition currentPosition = 0;

    if (!playback_ip->mediaStartTimeNoted)
        /* we haven't yet received 1st pts callback and thus not noted the media start time, so we return time we have accumulated till last discontinuity */
        /* this can happen either during seek (when decoders are flushed) or when sync channel has to flush the decoders (as they are full due to TSM not passing, frames in future due to lost frames) */
        return (playback_ip->streamDurationUntilLastDiscontinuity);

    if (currentPts == 0) {
        /* current PTS is 0 (as DM may have been reset during channel change or DM may have been underflowed (which looks like a bug in DM) */
        BDBG_MSG(("%s: currentPts is 0, using lastPts as currentPts, lastPts %x, firstPts %x", __FUNCTION__, playback_ip->lastUsedPts, playback_ip->firstPts));
        if (playback_ip->getNewPtsAfterDiscontinuity) {
            /* there was a recent pts discontinuity event and thus we needed to get the new currentPts, but we haven't gotten one yet */
            /* so we use the firstPts before the discontinuity in our calculation */
            currentPosition = (playback_ip->lastUsedPts - playback_ip->firstPtsBeforePrevDiscontinuity) / 45; /* in msec: our decoders report PTS in 45Khz rate */
        }
        else {
            currentPosition = (playback_ip->lastUsedPts - playback_ip->firstPts) / 45; /* in msec: our decoders report PTS in 45Khz rate */
        }
        currentPosition += playback_ip->streamDurationUntilLastDiscontinuity;
    }
    else if (playback_ip->getNewPtsAfterDiscontinuity && currentPts == playback_ip->lastUsedPts) {
        /* we had a pts discontinuity, and thus needed to get the new currentPts, */
        /* but the currentPts happens to be same as the lastPts */
        /* This can happen when TSM hasn't yet matured for the new frames after the discontinuity */
        /* so we use the previous firstPts to calculate the current position so far */
        BDBG_MSG(("%s: currentPts hasn't changed since the last discontinuity, lastPts %x, currentPts %x", __FUNCTION__, playback_ip->lastUsedPts, currentPts));
        currentPosition = (playback_ip->lastUsedPts - playback_ip->firstPtsBeforePrevDiscontinuity) / 45; /* in msec: our decoders report PTS in 45Khz rate */
        currentPosition = playback_ip->streamDurationUntilLastDiscontinuity;
    }
    else {
        /* current PTS is valid, current - first PTS gives us the current position (+ streamDurationUntilLastDiscontinuity) */
        /* since these variables are unsigned, unsigned math takes care of the PTS wrap logic as well */
        currentPosition = (currentPts - playback_ip->firstPts) / 45; /* in msec: our decoders report PTS in 45Khz rate */
        currentPosition += playback_ip->streamDurationUntilLastDiscontinuity;
        playback_ip->getNewPtsAfterDiscontinuity = false; /* as we successfully got the new current pts */
        playback_ip->lastUsedPts = currentPts;
        playback_ip->ptsDuringLastDiscontinuity = 0;
        BDBG_MSG(("%s: currentPts %x, firstPts %x, lastPts %x, pos %u streamDurationUntilLastDiscontinuity %u", __FUNCTION__, currentPts, playback_ip->firstPts, playback_ip->lastUsedPts, currentPosition, playback_ip->streamDurationUntilLastDiscontinuity));
    }
    return currentPosition;
}

B_PlaybackIpError
B_PlaybackIp_HttpGetCurrentPlaybackPosition(
        B_PlaybackIpHandle playback_ip,
        NEXUS_PlaybackPosition *currentPosition
        )
{
    unsigned currentPts = 0;
    NEXUS_PlaybackStatus playbackStatus;

    if (playback_ip->useNexusPlaypump) {
        if (playback_ip->startSettings.mediaPositionUsingWallClockTime) {
            /* Wall Clock based time position algorithm: this method of maintaining time is not as accurate as the default one using PTS. */
            /* But is needed when Simulated STC based trickmodes are used as decoders will generate PTS errors because of STC rate change while TSM is on in STC trickmodes. */
            B_Time currentTime;
            int speedNumerator = 1, speedDenominator = 1;
            unsigned currentPlayed;

            B_Time_Get(&currentTime);
            if (!playback_ip->mediaStartTimeNoted || /* if we haven't started playing yet (mediaStartTime is noted in the firstPtsCallback), or */
                    playback_ip->mediaEndTimeNoted  ||  /* we have reached end of stream & noted the mediaEndTime, or */
                    playback_ip->playback_state == B_PlaybackIpState_ePaused || /* we are currently in paused state, or */
                    playback_ip->playback_state == B_PlaybackIpState_eWaitingToEnterTrickMode || /* we are waiting to enter the trickmode transition stage, or */
                    playback_ip->playback_state == B_PlaybackIpState_eEnteringTrickMode) /* we are in the trickmode transition stage where trickMode API hasn't yet finished */
            {
                /* We use the last noted position */
                *currentPosition = playback_ip->lastPosition;
            }
            else
            {
                /* We are either in the Playing or TrickMode state, */
                /* calculate the current interval, scale it by the trickMode speed, and update the lastPosition */
                BDBG_ASSERT(playback_ip->mediaStartTimeNoted);
                if (playback_ip->playback_state == B_PlaybackIpState_eTrickMode ) {
                    speedNumerator = playback_ip->speedNumerator;
                    speedDenominator = playback_ip->speedDenominator;
                }
                else {
                    /* 1x Playing state */
                    speedNumerator = 1;
                    speedDenominator = 1;
                }

                /* Check if we are underflowing, if so, we can't update our last position with this interval. */
                /* In addition, this helps with the initial start where we only start updating the position when we start displaying pictures. */
                currentPlayed = B_PlaybackIp_UtilsPlayedCount(playback_ip);
                if (currentPlayed == playback_ip->lastPlayed && playback_ip->playback_state == B_PlaybackIpState_ePlaying) {
                    /* Between the current & last sampling interval, count of played picture hasn't changed. */
                    /* It can happen if there was a network timeout condition where we have no new video frames or audio samples to play. */
                    /* In such cases, we dont update our current position and instead wait until this count starts changing again! */
                    /* Note: we do this only in the Playing state as position gets reset to current point anyway when we resume from TrickMode to Play */
                    BDBG_MSG(("%s: displayed picture count didn't change in this interval %d", __FUNCTION__, B_Time_Diff(&currentTime, &playback_ip->mediaStartTime)));
                    *currentPosition = playback_ip->lastPosition;
                }
                else {
                    /* last played count is different than the previous one, so lets update our position */
                    *currentPosition = playback_ip->lastPosition + ((speedNumerator * B_Time_Diff(&currentTime, &playback_ip->mediaStartTime)) / speedDenominator);
                    playback_ip->lastPosition = *currentPosition;
                    playback_ip->lastPlayed = currentPlayed;
                }
                /* Make sure our position is within the allowed bounds */
                if ((int)(*currentPosition) <= 0) {
                    BDBG_MSG(("%s: ###### resetting current position %u to 0", __FUNCTION__, (int)(*currentPosition)));
                    if (playback_ip->settings.enableEndOfStreamLooping && playback_ip->playback_state == B_PlaybackIpState_eTrickMode && speedNumerator < 0) {
                        *currentPosition = playback_ip->psi.duration;
                        playback_ip->lastPosition = *currentPosition;
                    }
                    else {
                        *currentPosition = 0;
                        playback_ip->lastPosition = *currentPosition;
                    }
                }
                else if (playback_ip->psi.duration && *currentPosition >= playback_ip->psi.duration) {
                    BDBG_MSG(("%s: ###### resetting current position %u to stream duration %u, loop %d", __FUNCTION__, *currentPosition, playback_ip->psi.duration, playback_ip->settings.enableEndOfStreamLooping));
                    if (playback_ip->settings.enableEndOfStreamLooping) {
                        *currentPosition = 0;
                        playback_ip->lastPosition = *currentPosition;
                    }
                    else {
                        *currentPosition = playback_ip->psi.duration;
                        playback_ip->lastPosition = *currentPosition;
                    }
                }
                playback_ip->mediaStartTime = currentTime;
            }
            BDBG_MSG_FLOW(("%s: #### currentPosition %u, num %d, denom %d", __FUNCTION__, *currentPosition, speedNumerator, speedDenominator));
        }
#ifdef B_HAS_HLS_PROTOCOL_SUPPORT
        else if (playback_ip->hlsSessionEnabled) {
            if (playback_ip->lastSeekPositionSet) {
                /* a seek operation is currently in progress but we haven't yet received the 1st frame after seeking */
                /* so currentPosition calculation using current PTS will not show the new seeked position. */
                /* To take care of the case, where seek was invoked because user has slided the timebar, sending older timestamp */
                /* will cause the timebar to jump backwards. To take care of this case, we use the lastSeekPosition as the current one */
                *currentPosition = playback_ip->lastSeekPosition;
            }
            else {
                if (http_get_current_pts(playback_ip, &currentPts) != B_ERROR_SUCCESS) {
                    /* if there is an error in getting the current PTS value, we still do the position calculation assuming it is 0 */
                    /* which will basically use the last valid PTS value in the http_calculate_current_position */
                    currentPts = playback_ip->lastUsedPts;
                    BDBG_ERR(("%s: currentPts is 0, setting it to lastUsedPts 0x%x", __FUNCTION__, playback_ip->lastUsedPts));
                }
                *currentPosition = (currentPts - playback_ip->originalFirstPts) / 45; /* in msec: our decoders report PTS in 45Khz rate */
                if (*currentPosition > playback_ip->psi.duration) {
                    /* looks like current position is not valid. */
                    if ( playback_ip->speedNumerator < 0) {
                        /* rewind case, set to start. */
                        BDBG_ERR(("%s: currentPosition %u is > duration, setting it to 0 for rewind case! %u: pts: current 0x%x, lastUsedPts 0x%x", __FUNCTION__,
                                    *currentPosition, playback_ip->psi.duration, currentPts, playback_ip->lastUsedPts));
                        *currentPosition = 0;
                    }
                    else {
                        /* normal or fwd case, set to end. */
                        BDBG_ERR(("%s: currentPosition %u is > duration, setting it to duration %u, pts: current 0x%x, lastUsed 0x%x", __FUNCTION__,
                                    *currentPosition, playback_ip->psi.duration, currentPts, playback_ip->lastUsedPts));
                        *currentPosition = playback_ip->psi.duration;
                    }
                }
                playback_ip->getNewPtsAfterDiscontinuity = false; /* as we successfully got the new current pts */
                playback_ip->lastUsedPts = currentPts;
                playback_ip->ptsDuringLastDiscontinuity = 0;
            }
        }
#endif /* B_HAS_HLS_PROTOCOL_SUPPORT */
        else if (playback_ip->lastSeekPositionSet) {
            /* a seek operation is currently in progress but we haven't yet received the 1st frame after seeking */
            /* so currentPosition calculation using current PTS will not show the new seeked position. */
            /* To take care of the case, where seek was invoked because user has slided the timebar, sending older timestamp */
            /* will cause the timebar to jump backwards. To take care of this case, we use the lastSeekPosition as the current one */
            *currentPosition = playback_ip->lastSeekPosition;
        }
        else {
            if (http_get_current_pts(playback_ip, &currentPts) != B_ERROR_SUCCESS) {
                /* if there is an error in getting the current PTS value, we still do the position calculation assuming it is 0 */
                /* which will basically use the last valid PTS value in the http_calculate_current_position */
                currentPts = 0;
            }
            *currentPosition = http_calculate_current_position(playback_ip, currentPts); /* accounts for check for PTS wrap & discontinuity */
        }
        BDBG_MSG(("%s: current position (using %s method) at %0.3f/%0.3f sec", __FUNCTION__, (playback_ip->startSettings.mediaPositionUsingWallClockTime?"wallClock":"pts"), *currentPosition/1000., playback_ip->psi.duration/1000.));
        playback_ip->pausedByteOffset = *currentPosition * (playback_ip->psi.avgBitRate/8000.0); /* byte position * byte rate in milli-sec */
    }
    else {
        /* not using playpump, so continue below to get position from Nexus playback */
        if (playback_ip->nexusHandles.playback) {
            if (NEXUS_Playback_GetStatus(playback_ip->nexusHandles.playback, &playbackStatus) != NEXUS_SUCCESS) {
                BDBG_ERR(("%s: ERROR: NEXUS_Playback_GetStatus() Failed\n", __FUNCTION__));
                return B_ERROR_UNKNOWN;
            }
            /* this is the position of the last frame that was sent to the vdec */
            *currentPosition = playbackStatus.position;
        }
        else
            *currentPosition = 0;
        playback_ip->pausedByteOffset = *currentPosition * (playback_ip->psi.avgBitRate/8000.0); /* byte position * byte rate in milli-sec */
        if (playback_ip->psi.duration && *currentPosition >= playback_ip->psi.duration) {
            BDBG_MSG(("%s: calculated incorrect current position (%u) as is outside the file duration %d, setting it to file duration\n", __FUNCTION__, *currentPosition, playback_ip->psi.duration));
            *currentPosition = playback_ip->psi.duration;
        }
        BDBG_MSG(("%s: current position: at %0.3f sec, byte offset %lld", __FUNCTION__, *currentPosition/1000., playback_ip->pausedByteOffset));
    }
    return B_ERROR_SUCCESS;
}

static B_PlaybackIpError
updateNexusAudioDecodersState(
    B_PlaybackIpHandle playback_ip,
    B_PlaybackIpTrickModesSettings *ipTrickModeSettings
    )
{
    B_PlaybackIpError rc = B_ERROR_UNKNOWN;
    NEXUS_AudioDecoderTrickState audioTrickSettings;
    float rate;
    int speedNumerator, speedDenominator;

    BSTD_UNUSED(ipTrickModeSettings);
    speedNumerator = playback_ip->speedNumerator;
    speedDenominator = playback_ip->speedDenominator;
    rate = (float) speedNumerator / speedDenominator;
    BDBG_MSG(("%s: speedNumerator %d, speedDenominator %d, rate %.1f", __FUNCTION__, speedNumerator, speedDenominator, rate));

#ifdef NEXUS_HAS_SIMPLE_DECODER
    if (playback_ip->nexusHandles.simpleAudioDecoder)
        NEXUS_SimpleAudioDecoder_GetTrickState(playback_ip->nexusHandles.simpleAudioDecoder, &audioTrickSettings);
    else if (playback_ip->nexusHandles.primaryAudioDecoder)
        NEXUS_AudioDecoder_GetTrickState(playback_ip->nexusHandles.primaryAudioDecoder, &audioTrickSettings);
    else if (playback_ip->nexusHandles.secondaryAudioDecoder)
        NEXUS_AudioDecoder_GetTrickState(playback_ip->nexusHandles.secondaryAudioDecoder, &audioTrickSettings);
    else {
        BDBG_WRN(("%s: No Audio Decoder Handles provided, Audio is not enabled!", __FUNCTION__));
        return (B_ERROR_SUCCESS);
    }
#endif

    if (playback_ip->hwStcTrickMode) {
        /* This is true when transitioning in-n-out of trickmodes using HW STC */
        /* which is true only for +ve trickmodes upto 2x, or pause/play case w/ connx stalling. */
        /* we keep audio on and dont set forceStopped. */
        BDBG_MSG(("%s: hwStcTrickMode settings for rate %0.1f", __FUNCTION__, rate));
        audioTrickSettings.forceStopped  = false;
        /* adjust the audio rate: since speedNumerator is 1 for Play, 0 for Pause, and valid in other HW STC trickModes, this equation works in all cases*/
        audioTrickSettings.rate = (NEXUS_NORMAL_PLAY_SPEED * speedNumerator) / speedDenominator;
        /* mute audio in STC trickmode */
        audioTrickSettings.muted = (rate == 1.0) ? false : true;
        /* let decoder know about HW based STC trickmode */
        audioTrickSettings.stcTrickEnabled = (rate == 1.0) ? false : true;
        audioTrickSettings.tsmEnabled = true;
    }
    else {
        /* We can be here for decoder based Pause or resuming into Play from that pause.Or */
        /* When doing trickmodes, we are here for all -ve trickmodes or > 2x speeds (either for simulated STC mode or decoder based trickmodes ) */
        if (rate == 1.0 || rate == 0.0) {
            /* Resuming to Play or Going to Decoder Pause */
            /* We keep audio on and dont set forceStopped. */
            BDBG_MSG(("%s: decoder settings for rate %0.1f %s", __FUNCTION__, rate, (rate == 1.0)?"Play":"Pause"));
            audioTrickSettings.forceStopped  = false;
            /* Set the audio decoder rate */
            audioTrickSettings.rate = NEXUS_NORMAL_PLAY_SPEED * (int)rate;
            audioTrickSettings.muted = (rate == 1.0) ? false : true; /* dont mute if resuming to play */
            audioTrickSettings.stcTrickEnabled = false;
            audioTrickSettings.tsmEnabled = true;
        }
        else {
            /* We can be using either simulatedStc trickmodes or just video decoder trickmodes. */
            /* Since we are here for all -ve trickmodes or > 2x speeds, audio is not played */
            BDBG_MSG(("%s: %s trickMode settings for rate %0.1f", __FUNCTION__, playback_ip->simulatedStcTrickMode?"Simualted Stc":"Decoder", rate));
            audioTrickSettings.forceStopped  = true;
            audioTrickSettings.muted = true;
            /* rate shouldn't really matter as decoder is stopped */
            audioTrickSettings.rate = NEXUS_NORMAL_DECODE_RATE;
            audioTrickSettings.stcTrickEnabled = false;
            audioTrickSettings.tsmEnabled = false;
        }
    }
    BDBG_MSG(("%s: AudioTrickSettings: forceStopped %s, rate %d, muted %s, stcTrickEnabled %s, tsmEnabled %s", __FUNCTION__,
                audioTrickSettings.forceStopped ? "Y":"N",
                audioTrickSettings.rate,
                audioTrickSettings.muted ? "Y":"N",
                audioTrickSettings.stcTrickEnabled ? "Y":"N",
                audioTrickSettings.tsmEnabled ? "Y":"N"
             ));
#ifdef NEXUS_HAS_SIMPLE_DECODER
    if (playback_ip->nexusHandles.simpleAudioDecoder) {
        if (NEXUS_SimpleAudioDecoder_SetTrickState(playback_ip->nexusHandles.simpleAudioDecoder, &audioTrickSettings) != NEXUS_SUCCESS) {
            BDBG_ERR(("%s: NEXUS_AudioDecoder_SetTrickState() failed for primary audio decoder \n", __FUNCTION__));
            rc = B_ERROR_UNKNOWN;
        }
        else
            rc = B_ERROR_SUCCESS;
    }
    else
#endif
    {
        if (playback_ip->nexusHandles.primaryAudioDecoder) {
            if (NEXUS_AudioDecoder_SetTrickState(playback_ip->nexusHandles.primaryAudioDecoder, &audioTrickSettings) != NEXUS_SUCCESS) {
                BDBG_ERR(("%s: NEXUS_AudioDecoder_SetTrickState() failed for primary audio decoder \n", __FUNCTION__));
                rc = B_ERROR_UNKNOWN;
                goto out;
            }
            else
                rc = B_ERROR_SUCCESS;
        }
        if (playback_ip->nexusHandles.secondaryAudioDecoder) {
            if (NEXUS_AudioDecoder_SetTrickState(playback_ip->nexusHandles.secondaryAudioDecoder, &audioTrickSettings) != NEXUS_SUCCESS) {
                BDBG_ERR(("%s: NEXUS_AudioDecoder_SetTrickState() failed for secondary audio decoder \n", __FUNCTION__));
                rc = B_ERROR_UNKNOWN;
            }
            else
                rc = B_ERROR_SUCCESS;
        }
    }
out:
    return rc;
}

static int
mapNexusFrameRateEnumToNum(NEXUS_VideoFrameRate frameRate)
{
    int frameRateNum;
    switch (frameRate)
    {
        case NEXUS_VideoFrameRate_e23_976:
        case NEXUS_VideoFrameRate_e24:
            frameRateNum = 24;
            break;
        case NEXUS_VideoFrameRate_e25:
            frameRateNum = 25;
            break;
        case NEXUS_VideoFrameRate_e29_97:
        case NEXUS_VideoFrameRate_e30:
            frameRateNum = 30;
            break;
        case NEXUS_VideoFrameRate_e50:
            frameRateNum = 50;
            break;
        case NEXUS_VideoFrameRate_e59_94:
        case NEXUS_VideoFrameRate_e60:
            frameRateNum = 60;
            break;
        case NEXUS_VideoFrameRate_e14_985:
            frameRateNum = 15;
            break;
        case NEXUS_VideoFrameRate_e7_493:
            frameRateNum = 7;
            break;
        default:
            /* not specified, we default frame rate to 30 */
            frameRateNum = 30;
    }
    BDBG_MSG(("frame rate %d, frameRateNum %d", frameRate, frameRateNum));
    return frameRateNum;
}

static B_PlaybackIpError
updateNexusStcSettings(
    B_PlaybackIpHandle playback_ip,
    unsigned increment,
    unsigned prescale
    )
{
#ifdef NEXUS_HAS_SIMPLE_DECODER
    if (playback_ip->nexusHandles.simpleStcChannel) {
        if (NEXUS_SimpleStcChannel_SetRate(playback_ip->nexusHandles.simpleStcChannel, increment, prescale) != NEXUS_SUCCESS) {
            BDBG_ERR(("%s: NEXUS_StcChannel_SetRate() failed \n", __FUNCTION__));
            return B_ERROR_UNKNOWN;
        }

        /* Invalidate STC channel to force the STC to get reloaded. This is required to minimize the loss of */
        /* audio/video data when transitioning from decoder_pause-->play or decoder_pause->seek->play. */
        /* The problem shows up if the time period between transition is long. */
        if (NEXUS_SimpleStcChannel_Invalidate(playback_ip->nexusHandles.simpleStcChannel) != NEXUS_SUCCESS) {
            BDBG_ERR(("%s: NEXUS_StcChannel_Invalidate() failed \n", __FUNCTION__));
            return B_ERROR_UNKNOWN;
        }
#if 0
        if (NEXUS_VideoDecoder_InvalidateSimulatedStc(playback_ip->nexusHandles.videoDecoder) != NEXUS_SUCCESS) {
            BDBG_ERR(("%s: NEXUS_VideoDecoder_InvalidateSimulatedStc() failed \n", __FUNCTION__));
            return B_ERROR_UNKNOWN;
        }
#endif
    }
    else
#endif
    {
        if (playback_ip->nexusHandles.stcChannel) {
            if (NEXUS_StcChannel_SetRate(playback_ip->nexusHandles.stcChannel, increment, prescale) != NEXUS_SUCCESS) {
                BDBG_ERR(("%s: NEXUS_StcChannel_SetRate() failed \n", __FUNCTION__));
                return B_ERROR_UNKNOWN;
            }

            /* Invalidate STC channel to force the STC to get reloaded. This is required to minimize the loss of */
            /* audio/video data when transitioning from decoder_pause-->play or decoder_pause->seek->play. */
            /* The problem shows up if the time period between transition is long. */
            if (NEXUS_StcChannel_Invalidate(playback_ip->nexusHandles.stcChannel) != NEXUS_SUCCESS) {
                BDBG_ERR(("%s: NEXUS_StcChannel_Invalidate() failed \n", __FUNCTION__));
                return B_ERROR_UNKNOWN;
            }
        }
    }
    BDBG_MSG(("%s: STC Trickmode: increment %u, prescale %u", __FUNCTION__, increment, prescale));
    return B_ERROR_SUCCESS;
}

static B_PlaybackIpError
updateNexusPlaypumpDecodersState(
    B_PlaybackIpHandle playback_ip,
    B_PlaybackIpTrickModesSettings *ipTrickModeSettings
    )
{
    bool pausePlaypump;
    NEXUS_VideoDecoderTrickState videoDecoderTrickSettings;
    float rate;
    int speedNumerator, speedDenominator;
    unsigned stcPreScale, stcIncrement;

    speedNumerator = playback_ip->speedNumerator;
    speedDenominator = playback_ip->speedDenominator;
    rate = (float) speedNumerator / speedDenominator;

    BDBG_MSG(("%s: speedNumerator %d, speedDenominator %d, rate %.1f, simulatedStcTrickMode %s, hwStcTrickMode %s", __FUNCTION__,
                speedNumerator, speedDenominator, rate,
                playback_ip->simulatedStcTrickMode ? "Y":"N",
                playback_ip->hwStcTrickMode ? "Y":"N"));

    if (playback_ip->hwStcTrickMode) {
        /* Hardware baed STC trickmodes are used for +ve speeds upto 2x including all slow fwd cases. */
        /* In addition, can also be called for Play resuming from previous HW STC based trickmode or pause */
        /* Assumptions in this mode:
         * numerator == 0 for pause, denominator is ignored.
         * numerator == 1 for play, denomintor is also 1.
         * numerator & denominator are appropriately set for +ve speeds  upto 2x
         */
        stcIncrement = (NEXUS_NORMAL_PLAY_SPEED * speedNumerator) / speedDenominator;
        stcPreScale = NEXUS_NORMAL_PLAY_SPEED-1; /* STC HW adds 1 */
        BDBG_MSG(("%s: Hardware STC based Trick play settings are: increment %d, prescale %d", __FUNCTION__, stcIncrement, stcPreScale));
    }
    else {
        /* non-hw based STC trickmode case, we run hw STC at the normal rate and instead manipulate either simualated STC or decoder rate */
        /* reset the STC rate to normal rate just incase it was modified last time */
        stcIncrement = NEXUS_NORMAL_DECODE_RATE;
        stcPreScale = NEXUS_NORMAL_DECODE_RATE-1; /* STC HW adds 1 */
    }

    /* update set the STC rate */
    if (updateNexusStcSettings(playback_ip, stcIncrement, stcPreScale) != B_ERROR_SUCCESS) {
        BDBG_ERR(("%s: ERROR: failed to set the stc rate to: incremen %u, prescale %u during trickmode transition", __FUNCTION__, stcIncrement, stcPreScale));
        return B_ERROR_UNKNOWN;
    }

    /* update audio decoder state to match the given trickmode: audio decoder trickmode settings are calculated in this function */
    if (updateNexusAudioDecodersState(playback_ip, ipTrickModeSettings) != B_ERROR_SUCCESS) {
        BDBG_ERR(("%s: ERROR: failed to update nexus audio decoder state during trickmode transition\n", __FUNCTION__));
        return B_ERROR_UNKNOWN;
    }

    /* update Nexus Playpump state */
    if (playback_ip->nexusHandles.playpump) {
        bool suspendPacing = false;
        if (ipTrickModeSettings->method == B_PlaybackIpTrickModesMethod_UseRvuSpec) {
            suspendPacing = rate == 1.0? false:true;
            if (B_PlaybackIp_TtsThrottle_SuspendPacing(playback_ip->ttsThrottle, suspendPacing) != B_ERROR_SUCCESS) {
                BDBG_ERR(("%s: B_PlaybackIp_TtsThrottle_SuspendPacing() Failed to %s", __FUNCTION__, suspendPacing ? "suspend pacing":"resume pacing"));
                return B_ERROR_UNKNOWN;
            }
        }
        if (rate == 0.0)
            pausePlaypump = true;
        else
            pausePlaypump = false;
        if (NEXUS_Playpump_SetPause(playback_ip->nexusHandles.playpump, pausePlaypump) != NEXUS_SUCCESS) {
            BDBG_ERR(("%s: NEXUS_Playpump_SetPause() failed to %s", __FUNCTION__, pausePlaypump ? "set pause":"unset pause"));
            return B_ERROR_UNKNOWN;
        }
        if (playback_ip->nexusHandles.playpump2) {
            if (NEXUS_Playpump_SetPause(playback_ip->nexusHandles.playpump2, pausePlaypump) != NEXUS_SUCCESS) {
                BDBG_ERR(("%s: NEXUS_Playpump_SetPause() for playpump2 failed to %s", __FUNCTION__, pausePlaypump ? "set pause":"unset pause"));
                return B_ERROR_UNKNOWN;
            }
        }
        BDBG_MSG(("%s: playpump pause %s, suspend pacing %s", __FUNCTION__, pausePlaypump ? "Y":"N", suspendPacing ? "Y":"N"));
    }

    /* update video decoder settings for streams playing video */
    if (playback_ip->nexusHandles.videoDecoder || playback_ip->nexusHandles.simpleVideoDecoder) {

        /* we can be here due to any of these case:
         * -pause w/ disconnect method, we use the decoder pause.
         * -play w/ after pause w/ disconnect method or resuming from trickmode, we use the decoder play.
         * -we are in fast fwd, slow rewind, or fast rewind trickmode case, we use decoder trickmodes.
         */


        if (playback_ip->nexusHandles.videoDecoder)
            NEXUS_VideoDecoder_GetTrickState(playback_ip->nexusHandles.videoDecoder, &videoDecoderTrickSettings);
#ifdef NEXUS_HAS_SIMPLE_DECODER
        else
            NEXUS_SimpleVideoDecoder_GetTrickState(playback_ip->nexusHandles.simpleVideoDecoder, &videoDecoderTrickSettings);
#endif
        if (playback_ip->hwStcTrickMode) {
            /* for STC trickmodes, set video decoder settings to handle +ve trickmodes upto 2x */
            /* we tell decoder to decode all pictures at the normal decoder rate (but really it is driven by the h/w STC rate) */
            /* also, we keep the TSM on. */
            BDBG_MSG(("%s: programming video decoder for HW STC based %s at rate %0.1f", __FUNCTION__,
                        (rate == 1.0 || rate == 0.0) ? "Play/Pause":"Trickmode", rate));
            videoDecoderTrickSettings.rate = NEXUS_NORMAL_DECODE_RATE; /* decoder rate doesn't matter for pause as STC is frozen for pause */
            videoDecoderTrickSettings.stcTrickEnabled = (rate == 1.0)? false:true; /* allows decoder to properly handle the PTS errors */

            videoDecoderTrickSettings.decodeMode = NEXUS_VideoDecoderDecodeMode_eAll;
            videoDecoderTrickSettings.tsmEnabled = NEXUS_TsmMode_eEnabled;

            videoDecoderTrickSettings.topFieldOnly = false;
            videoDecoderTrickSettings.hostTrickModesEnabled = false;
            videoDecoderTrickSettings.reverseFields = false;
            videoDecoderTrickSettings.forceStopped = false;
        }
        else if (rate == 1.0 || rate == 0.0) {
            /* We are resuming back to normal play mode or going into decoder based pause */
            /* we tell decoder to decode all pictures at the normal decoder rate and re-enable TSM */
            BDBG_MSG(("%s: programming video decoder for decoder based %s ", __FUNCTION__, (rate == 1.0) ? "Play":"Pause"));
            videoDecoderTrickSettings.rate = (rate == 1.0) ? NEXUS_NORMAL_DECODE_RATE : 0;
            videoDecoderTrickSettings.stcTrickEnabled = false;

            videoDecoderTrickSettings.decodeMode = NEXUS_VideoDecoderDecodeMode_eAll;
            videoDecoderTrickSettings.tsmEnabled = NEXUS_TsmMode_eEnabled;

            videoDecoderTrickSettings.topFieldOnly = false;
            videoDecoderTrickSettings.hostTrickModesEnabled = false;
            videoDecoderTrickSettings.reverseFields = false;
            videoDecoderTrickSettings.forceStopped = false;
        }
        else {
            /* We are here for all -ve trickmodes or > 2x speeds */
            /* We can be using either simualtedSTC trickmodes or just video decoder trickmodes. */
            /* First, we setup the settings which differ in two modes: TSM & Decoder Rate */
            if (playback_ip->simulatedStcTrickMode) {
                BDBG_MSG(("%s: programming video decoder for simulatedStcTrickMode", __FUNCTION__));
#if 0
                playback_ip->enableRecording = 1; /* NOTE: this is just for debugging the slow rewind issue */
#endif
                /* we keep the TSM on and run manipulate STC in s/w to match the requested rate. */
                /* Decoder will decode/display frames as their TSM matures and thus display them at the correct time. */
                videoDecoderTrickSettings.tsmEnabled = NEXUS_TsmMode_eSimulated;
                /* tell decoder that STC is being manipulated in software */
                videoDecoderTrickSettings.stcTrickEnabled = false;
                /* set decoder rate */
                videoDecoderTrickSettings.rate = (speedNumerator*NEXUS_NORMAL_DECODE_RATE)/speedDenominator;
            }
            else {
                /* for nonSimulatedStc trick modes (when server provides frameRateInTrickMode) */
                int frameRateNum;
                int frameRateInTrickMode;
                BDBG_MSG(("%s: programming video decoder for decoder rate based trickmodes", __FUNCTION__));
                /* we turn-off the TSM and instead just program decoder rate to decode at that rate. */
                videoDecoderTrickSettings.tsmEnabled = NEXUS_TsmMode_eDisabled;
                videoDecoderTrickSettings.stcTrickEnabled = false;
                /* program the decoder rate */
                if (rate == -1.0 && playback_ip->frameAdvanceApiActive) {
                    videoDecoderTrickSettings.rate = 0;
                }
                else {
                    frameRateInTrickMode = ipTrickModeSettings->frameRateInTrickMode ? ipTrickModeSettings->frameRateInTrickMode : playback_ip->psi.frameRateInTrickMode;
                    frameRateNum = mapNexusFrameRateEnumToNum(playback_ip->frameRate);
                    videoDecoderTrickSettings.rate = (NEXUS_NORMAL_DECODE_RATE/frameRateNum)*frameRateInTrickMode;
                    BDBG_MSG(("%s: frame rate %d, nexus enum %d , frameRateInTrickMode %d", __FUNCTION__, frameRateNum, playback_ip->frameRate, frameRateInTrickMode));
                }
            }
            /* video decoder settings which are common to both Simulated STC or Decoder trickmode case */

            /* We are only going to decode I-frames in the trickmodes. Servers typically will send I-frames in rewind & higher forward trickplay speeds. */
            /* Ideally, servers could/should use the FrameTypesInTrickMode.ochn.org header to indicate the frame type being sent by the servers. */
            videoDecoderTrickSettings.decodeMode = NEXUS_VideoDecoderDecodeMode_eI;

            /* we are using host trick modes as server is manipulating the streams */
            videoDecoderTrickSettings.hostTrickModesEnabled = true;
            /* for rewind or fast fwd trick modes, we send only the top field for the interlaced content. */
            videoDecoderTrickSettings.topFieldOnly = true;

            /* explicitly turn off these settings as well */
            videoDecoderTrickSettings.brcmTrickModesEnabled = false;
            videoDecoderTrickSettings.dqtEnabled = false;

            /* for fast rewind, set reverseFields flag to true */
            if (rate < 0)
                videoDecoderTrickSettings.reverseFields = true;
            else
                videoDecoderTrickSettings.reverseFields = false;
        } /* -ve trickmodes or > 2x speeds */

        if (playback_ip->nexusHandles.videoDecoder) {
            if (NEXUS_VideoDecoder_SetTrickState(playback_ip->nexusHandles.videoDecoder, &videoDecoderTrickSettings) != NEXUS_SUCCESS) {
                BDBG_ERR(("%s: NEXUS_VideoDecoder_SetTrickState() failed \n", __FUNCTION__));
                return B_ERROR_UNKNOWN;
            }
        }
#ifdef NEXUS_HAS_SIMPLE_DECODER
        else {
            if (NEXUS_SimpleVideoDecoder_SetTrickState(playback_ip->nexusHandles.simpleVideoDecoder, &videoDecoderTrickSettings) != NEXUS_SUCCESS) {
                BDBG_ERR(("%s: NEXUS_VideoDecoder_SetTrickState() failed \n", __FUNCTION__));
                return B_ERROR_UNKNOWN;
            }
        }
#endif
#ifdef BDBG_DEBUG_BUILD
        if (playback_ip->ipVerboseLog)
            BDBG_WRN(("Video Decoder Settings: Host %s, TSM %s, TsmSimulated %s, StcTrick %s, Decode Rate %d, TopFieldOnly %s, RevFields %s",
                    videoDecoderTrickSettings.hostTrickModesEnabled? "Y":"N",
                    videoDecoderTrickSettings.tsmEnabled? "Y":"N",
                    videoDecoderTrickSettings.tsmEnabled == NEXUS_TsmMode_eSimulated? "Y":"N",
                    videoDecoderTrickSettings.stcTrickEnabled? "Y":"N",
                    videoDecoderTrickSettings.rate,
                    videoDecoderTrickSettings.topFieldOnly? "Y":"N",
                    videoDecoderTrickSettings.reverseFields? "Y":"N"
                 ));
#endif
#ifdef TODO
        /* I dont think we need to invalidate the simulated STC but will need to look into this later.. */
        if (playback_ip->simulatedStcTrickMode) {
            if (NEXUS_VideoDecoder_InvalidateSimulatedStc(playback_ip->nexusHandles.videoDecoder) != NEXUS_SUCCESS) {
                BDBG_ERR(("%s: NEXUS_VideoDecoder_InvalidateSimulatedStc() failed \n", __FUNCTION__));
                return B_ERROR_UNKNOWN;
            }
        }
#endif

    }

    BDBG_MSG(("%s: Trick play settings are sucessfully applied to Nexus AV decoders and Playpump", __FUNCTION__));
    return B_ERROR_SUCCESS;
}

B_PlaybackIpError
http_send_time_seek_request(
    B_PlaybackIpHandle playback_ip
    )
{
    B_PlaybackIpError rc;
    double timeSeekRangeBegin;
    double timeSeekRangeEnd;

    if (playback_ip->ipTrickModeSettings.rate > 0) {
        timeSeekRangeBegin = playback_ip->initialTimeSeekRangeBegin;
        timeSeekRangeEnd = timeSeekRangeBegin + playback_ip->timeSeekRangeInterval;
        playback_ip->initialTimeSeekRangeBegin = timeSeekRangeEnd;
    }
    else {
        timeSeekRangeEnd = playback_ip->initialTimeSeekRangeBegin;
        timeSeekRangeBegin = timeSeekRangeEnd - playback_ip->timeSeekRangeInterval;
        if (timeSeekRangeBegin < 0)
            timeSeekRangeBegin = 0.;
        playback_ip->initialTimeSeekRangeBegin = timeSeekRangeBegin;
    }

    BDBG_MSG(("%s: initTimeSeekRangeBegin %0.3f, interval %0.3f, timeSeekRangeBegin %0.3f, timeSeekRangeEnd %0.3f, state %d",
                __FUNCTION__, playback_ip->initialTimeSeekRangeBegin, playback_ip->timeSeekRangeInterval, timeSeekRangeBegin, timeSeekRangeEnd, playback_ip->playback_state));

    /* now build & send the new get request so that server will send stream at new speed */
    if ((rc = http_do_server_trick_modes_socket_transition(playback_ip, timeSeekRangeBegin, timeSeekRangeEnd, playback_ip->ipTrickModeSettings.rate, NULL /*playSpeedString */)) != B_ERROR_SUCCESS) {
        BDBG_ERR(("%s: Failed to update server about the trick play transition\n", __FUNCTION__));
        return B_ERROR_UNKNOWN;
    }

    BDBG_ERR(("%s: Done", __FUNCTION__));
    return B_ERROR_SUCCESS;
}

static B_PlaybackIpError
http_do_client_side_trick_modes_using_playback(
        B_PlaybackIpHandle playback_ip,
        B_PlaybackIpTrickModesSettings *ipTrickModeSettings
        )
{
    NEXUS_PlaybackTrickModeSettings playbackTrickModeSettings;
    NEXUS_Playback_GetDefaultTrickModeSettings(&playbackTrickModeSettings);
    int sign;
    int rate;

    playbackTrickModeSettings.mode = NEXUS_PlaybackHostTrickMode_eNone; /* this allows rate to take affect */
    if (ipTrickModeSettings->absoluteRateDefined) {
        playbackTrickModeSettings.rate = ipTrickModeSettings->absoluteRate;
    }
    else {
        if (ipTrickModeSettings->rate < 0) {
            sign = -1;
            rate = sign * ipTrickModeSettings->rate;
        }
        else {
            sign = 1;
            rate = ipTrickModeSettings->rate;
        }
        switch (rate) {
        case 1:
            playbackTrickModeSettings.rate = sign*NEXUS_NORMAL_PLAY_SPEED;
            break;
        case 2:
            playbackTrickModeSettings.rate = sign*2*NEXUS_NORMAL_PLAY_SPEED;
            break;
        case 3:
            playbackTrickModeSettings.rate = sign*5*NEXUS_NORMAL_PLAY_SPEED;
            break;
        case 4:
            playbackTrickModeSettings.rate = sign*10*NEXUS_NORMAL_PLAY_SPEED;
            break;
        case 5:
            playbackTrickModeSettings.rate = sign*25*NEXUS_NORMAL_PLAY_SPEED;
            break;
        case 6:
            playbackTrickModeSettings.rate = sign*50*NEXUS_NORMAL_PLAY_SPEED;
            break;
        case 7:
            playbackTrickModeSettings.rate = sign*100*NEXUS_NORMAL_PLAY_SPEED;
            break;
        default :
            playbackTrickModeSettings.rate = sign*200*NEXUS_NORMAL_PLAY_SPEED;
            break;
        }
    }

    BDBG_MSG(("%s: Calling NEXUS_Playback_TrickMode() nexus rate %d", __FUNCTION__, playbackTrickModeSettings.rate));
    if (NEXUS_Playback_TrickMode(playback_ip->nexusHandles.playback, &playbackTrickModeSettings) != NEXUS_SUCCESS) {
        BDBG_WRN(("%s: NEXUS_Playback_TrickMode() failed for rate %d\n", __FUNCTION__, ipTrickModeSettings->rate));
        return (B_ERROR_UNKNOWN);
    }

    BDBG_MSG(("%s: NEXUS_Playback_TrickMode() successfully updated the trick mode rate to %d, nexus rate %d", __FUNCTION__, ipTrickModeSettings->rate, playbackTrickModeSettings.rate));
    return B_ERROR_SUCCESS;
}

#define B_PlaybackIp_HttpGetDefaultTrickModeSettings B_PlaybackIp_HttpSetDefaultTrickModeSettings
void B_PlaybackIp_HttpSetDefaultTrickModeSettings(
    B_PlaybackIpTrickModesSettings *ipTrickModeSettings
    )
{
    memset(ipTrickModeSettings, 0, sizeof(B_PlaybackIpTrickModesSettings));
    ipTrickModeSettings->method = B_PlaybackIpTrickModesMethod_UseByteRange;    /* default to client side trickmodes */
    ipTrickModeSettings->rate = 1;  /* normal rate unless app sets it otherwise */
    ipTrickModeSettings->frameRepeat = 1;
    ipTrickModeSettings->pauseMethod = B_PlaybackIpPauseMethod_UseDisconnectAndSeek;
    ipTrickModeSettings->seekPosition = 0;  /* beginging of stream */
    ipTrickModeSettings->seekPositionIsRelative = false;  /* assume all seeks positions are absolute unless app specifically sets this flag */
    ipTrickModeSettings->seekBackward = false;  /* assume all relative seeks are in forward direction unless app specifically sets this flag */
    ipTrickModeSettings->decodeMode = NEXUS_VideoDecoderDecodeMode_eAll;  /* assume all relative seeks are in forward direction unless app specifically sets this flag */
    return;
}

B_PlaybackIpError
B_PlaybackIp_GetTrickModeSettings(
    B_PlaybackIpHandle playback_ip,
    B_PlaybackIpTrickModesSettings *ipTrickModeSettings
    )
{
    if (!playback_ip || !ipTrickModeSettings) {
        BDBG_ERR(("%s: ERROR: incorrect params: playback_ip %p, trickMode %p\n", __FUNCTION__, playback_ip, ipTrickModeSettings));
        return B_ERROR_INVALID_PARAMETER;
    }
    B_PlaybackIp_HttpSetDefaultTrickModeSettings(&playback_ip->ipTrickModeSettings);
    *ipTrickModeSettings = playback_ip->ipTrickModeSettings;
    if (playback_ip->useNexusPlaypump)
        ipTrickModeSettings->method = B_PlaybackIpTrickModesMethod_UsePlaySpeed;
    return B_ERROR_SUCCESS;
}

static void unlock_ip_session(
    B_PlaybackIpHandle playback_ip
    )
{
    BKNI_ReleaseMutex(playback_ip->lock);
}

static B_PlaybackIpError lock_ip_session(
    B_PlaybackIpHandle playback_ip
    )
{
    B_PlaybackIpError rc = B_ERROR_SUCCESS;
    B_PlaybackIpState currentState;

    /* set state to waiting to enter trickmodes, this allows Nexus IO & HTTP thread to finish its work ASAP and release the mutex */
    currentState = playback_ip->playback_state;
    playback_ip->playback_state = B_PlaybackIpState_eWaitingToEnterTrickMode;

    /* BIP TODO: should we use the TryLock here and use a while loop w/ 10msec sleep interval and some upper limit on the loop count, this will avoid a mutex lockup */
    /* now wait the lock to be released by other threads (Nexus IO or HTTP main thread) */
    if (BKNI_AcquireMutex(playback_ip->lock) != B_ERROR_SUCCESS) {
        rc = B_ERROR_UNKNOWN;
    }
    else {
        rc = B_ERROR_SUCCESS;
    }

    /* now restore the current state */
    playback_ip->playback_state = currentState;

    return rc;
}

B_PlaybackIpError
B_PlaybackIp_PauseRvu(
    B_PlaybackIpHandle playback_ip,
    B_PlaybackIpTrickModesSettings *ipTrickModeSettings
    )
{
    BSTD_UNUSED(playback_ip);
    BSTD_UNUSED(ipTrickModeSettings);
    BDBG_ERR(("%s: ERROR: Rvu Pause is not yet supported", __FUNCTION__));
    return B_ERROR_SUCCESS;
}

#ifdef LIVEMEDIA_SUPPORT
B_PlaybackIpError
B_PlaybackIp_PauseRtsp(
    B_PlaybackIpHandle playback_ip,
    B_PlaybackIpTrickModesSettings *ipTrickModeSettings
    )
{
#if 0
    B_PlaybackIpError rc;
    B_PlaybackIp_RtspCtrlCmd cmd;

    /* send Pause command to the server */
    cmd.type = B_PlaybackIp_RtspCtrlCmdType_Pause;
    B_PlaybackIp_liveMediaSessionRTSPControl(playback_ip->lm_context, &cmd);

    /* disable the live mode so that feeder thread doesn't detect any discontinuities introduced due to longer n/w latency */
    playback_ip->disableLiveMode = true;

    /* update the decoder state */
    ipTrickModeSettings->rate = 0;
    ipTrickModeSettings->frameRepeat = 0;
    ipTrickModeSettings->decodeMode = NEXUS_VideoDecoderDecodeMode_eAll;
    if ((rc = updateNexusPlaypumpDecodersState(playback_ip, ipTrickModeSettings)) != B_ERROR_SUCCESS) {
        BDBG_ERR(("%s: ERROR: failed to update nexus av decoder state during pause transition\n", __FUNCTION__));
        return rc;
    }
    return B_ERROR_SUCCESS;
#else
    BSTD_UNUSED(playback_ip);
    BSTD_UNUSED(ipTrickModeSettings);
    /* Until RTSP-ES support is complete, we are going to disable RTSP trickmodes */
    BDBG_ERR(("%s: Until RTSP-ES support is complete, we are going to disable RTSP trickmodes", __FUNCTION__));
    return B_ERROR_UNKNOWN;
#endif
}
#endif /* LIVEMEDIA_SUPPORT */

/* handles pause for non-HLS, non-RVU HTTP sessions: includes sessions from other DLNA Servers */
/* For such HTTP session, either Nexus Playpump or Playback can be used so we have to be careful in choosing the correct interface */
/* Plus, we have to consider the pause method that app wants us to use: connx stalling or disconnect & seek */
B_PlaybackIpError
B_PlaybackIp_PauseHttp(
    B_PlaybackIpHandle playback_ip,
    B_PlaybackIpTrickModesSettings *ipTrickModeSettings
    )
{
    B_PlaybackIpError rc = B_ERROR_SUCCESS;

    if (!playback_ip->useNexusPlaypump) {
        /* pause Nexus Playback and it takes care of pausing decoders & playpump */
        if (NEXUS_Playback_Pause(playback_ip->nexusHandles.playback) != NEXUS_SUCCESS) {
            BDBG_ERR(("%s: ERROR: Failed to Pause Nexus Playback", __FUNCTION__ ));
            rc = B_ERROR_UNKNOWN;
            goto error;
        }
    }
    else {
        /* using Nexus Playpump, so we will need to inform both Decoders & Playpump about Pause */
        ipTrickModeSettings->rate = 0;
        ipTrickModeSettings->frameRepeat = 0;
        ipTrickModeSettings->decodeMode = NEXUS_VideoDecoderDecodeMode_eAll;
        if (ipTrickModeSettings->pauseMethod == B_PlaybackIpPauseMethod_UseConnectionStalling) {
            /* in connection stalling case, we could use STC trickmode to freeze the STC */
            /* however, this current breaks this case: Playing -> Pause -> Seek ==> should update to new position & Pause there */
            /* Using STC based pause doesn't allow decoder to just advance one frame */

            /* Using decoder pause allows us to seek to new position and then do a frame advance to show the next position */
            /* So we are not using STC pause & instead using decoder based pause!!! */

            /* TODo: Also, note, after bit more testing, these 3 if conditions dont do anything different and thus can be combined! */
            playback_ip->hwStcTrickMode = false;
            playback_ip->speedNumerator = 0;
            playback_ip->speedDenominator = 1;
        }
        else if (ipTrickModeSettings->pauseMethod == B_PlaybackIpPauseMethod_UseDisconnectAndSeek) {
            /* since we have to disconnect the connection and reconnect to the server at the current location, */
            /* assuming that server may not be able to send the stream from the same point, so we are currently flushing */
            /* in that case, we can't do stc trickmode and rather rely on decoders to pause & resume. */
            playback_ip->hwStcTrickMode = false;
            playback_ip->speedNumerator = 0;
            playback_ip->speedDenominator = 1;
        }
        else {
            /* use decoder pause for any other pause method case */
            playback_ip->hwStcTrickMode = false;
            playback_ip->speedNumerator = 0;
            playback_ip->speedDenominator = 1;
        }

        BDBG_MSG(("%s: Pause Method: %s, Pause using %s, speedNumerator %d, speedDenominator %d", __FUNCTION__,
                    (ipTrickModeSettings->pauseMethod == B_PlaybackIpPauseMethod_UseConnectionStalling) ? "Connection Stalling":"Disconnection & Seek",
                    playback_ip->hwStcTrickMode?"STC Pause":"Decoders Pause", playback_ip->speedNumerator, playback_ip->speedDenominator));
        if ((rc = updateNexusPlaypumpDecodersState(playback_ip, ipTrickModeSettings)) != B_ERROR_SUCCESS) {
            BDBG_ERR(("%s: ERROR: failed to update nexus av decoder state during pause transition", __FUNCTION__));
            goto error;
        }
    }

    if (ipTrickModeSettings->pauseMethod == B_PlaybackIpPauseMethod_UseDisconnectAndSeek) {
        /* if app is telling us to pause using disconnect & seek method, then just pausing nexus playback/playpump is NOT enough. */
        /* in addition, we need to close the current socket and store some state to resume playback later from this point */

        /* do the ground work to enable cloning the security handle */
        /* if security layer has defined the shutdown callback, we call it to start the shutdown of the security handle */
        /* this basically tells security module to free up an security context w/o destroying the ability to clone it */
        if (playback_ip->netIo.shutdown && !playback_ip->serverClosed)
            playback_ip->netIo.shutdown(playback_ip->securityHandle, 0);

        /* now disconnect: close the current socket to meet DLNA requirement of pause using disconnecting socket */
        close(playback_ip->socketState.fd);
        playback_ip->socketState.fd = -1;
        if (!playback_ip->serverClosed) {
            /* since server hasn't closed (meaning it hasn't yet sent the full content), set a flag to re-open the socket */
            playback_ip->reOpenSocket = true; /* this flags tells HTTP layer to reopen the connection */
            playback_ip->reOpenIndexSocket = true; /* this flags tells HTTP layer to reopen the connection if index reading flow is continued */
        }
        /* set a flag to prevent this socket from being closed again in NetRange function */
        playback_ip->socketClosed = true;
    }
    else {
        /* if app is telling us to pause using connection stalling, or we are pausing a live channel */
        /* then just pausing nexus playback/playpump is enough. */
        /* Since decoders are in pause state, this will eventually cause all buffers in pipleline to fill up and */
        /* thus read io thread will stop reading from the socket. This will cause TCP buffers to fill and thus */
        /* TCP flow control will send 0 TCP window to server and thus make it stop sending the content */
        BDBG_MSG(("%s: Paused IP Playback using Connection Stalling Method", __FUNCTION__));
    }
    rc = B_ERROR_SUCCESS;
    return rc;

error:
    return rc;
}

static B_PlaybackIpError
trickModePause_locked(
    B_PlaybackIpHandle playback_ip,
    B_PlaybackIpTrickModesSettings *ipTrickModeSettings
    )
{
    B_PlaybackIpError rc = B_ERROR_SUCCESS;
    B_PlaybackIpState currentState;
    NEXUS_PlaybackPosition currentPosition;

    if (!playback_ip || !ipTrickModeSettings) {
        BDBG_ERR(("%s: ERROR: incorrect params: playback_ip %p, trickMode %p", __FUNCTION__, playback_ip, ipTrickModeSettings));
        return B_ERROR_INVALID_PARAMETER;
    }

    BDBG_MSG(("%s: Pausing playback_ip %p, method %d", __FUNCTION__, playback_ip, ipTrickModeSettings->pauseMethod, playback_ip->socketState.fd));
    if (playback_ip->playback_state != B_PlaybackIpState_ePaused &&
            playback_ip->playback_state != B_PlaybackIpState_ePlaying &&
            playback_ip->playback_state != B_PlaybackIpState_eTrickMode) {
        BDBG_ERR(("%s: ERROR: Can't do Pause command in this state %d", __FUNCTION__, playback_ip->playback_state));
        return B_ERROR_INVALID_PARAMETER;
    }

    if (playback_ip->psi.liveChannel) {
        BDBG_WRN(("%s: Can't do Pause for liveChannels (psi.liveChannel flag %d), Ignoring the command", __FUNCTION__, playback_ip->psi.liveChannel));
        return B_ERROR_SUCCESS;
    }

    if (playback_ip->playback_state == B_PlaybackIpState_ePaused) {
        /* already in pause state, so resume */
        if (trickModePlay_locked(playback_ip)) {
            BDBG_ERR(("%s: ERROR: Failed to resume from pause", __FUNCTION__));
            rc = B_ERROR_UNKNOWN;
            return rc;
        }
        BDBG_MSG(("%s: resuming from pause", __FUNCTION__));
        return B_ERROR_SUCCESS;
    }

    if (B_PlaybackIp_HttpGetCurrentPlaybackPosition(playback_ip, &currentPosition) != B_ERROR_SUCCESS) {
        BDBG_WRN(("%s: Failed to determine the current playback position", __FUNCTION__));
        return B_ERROR_UNKNOWN;
    }
    playback_ip->lastPosition = currentPosition; /* this is the last position upto which we have played the media so far! */
    BDBG_MSG(("%s: ##### lastPosition played %u until Pause", __FUNCTION__, playback_ip->lastPosition));

    /* now change the state to EnteringTrickMode until we finish the trickmode work */
    currentState = playback_ip->playback_state;
    playback_ip->playback_state = B_PlaybackIpState_eEnteringTrickMode;

#ifdef B_HAS_HLS_PROTOCOL_SUPPORT
    if (playback_ip->hlsSessionEnabled)
        rc = B_PlaybackIp_PauseHls(playback_ip, currentState);
    else
#endif
#ifdef B_HAS_MPEG_DASH_PROTOCOL_SUPPORT
    if (playback_ip->mpegDashSessionEnabled)
        rc = B_PlaybackIp_PauseMpegDash(playback_ip, ipTrickModeSettings);
    else
#endif
    if (playback_ip->protocol == B_PlaybackIpProtocol_eHttp)
        rc = B_PlaybackIp_PauseHttp(playback_ip, ipTrickModeSettings);
#ifdef LIVEMEDIA_SUPPORT
    else if (playback_ip->protocol == B_PlaybackIpProtocol_eRtsp)
        rc = B_PlaybackIp_PauseRtsp(playback_ip, ipTrickModeSettings);
#endif
    else if (ipTrickModeSettings->method == B_PlaybackIpTrickModesMethod_UseRvuSpec)
        rc = B_PlaybackIp_PauseRvu(playback_ip, ipTrickModeSettings);
    else {
        BDBG_ERR(("%s: ERROR: Pause not supported for IP protocol %d", __FUNCTION__, playback_ip->protocol));
        rc = B_ERROR_NOT_SUPPORTED;
    }

    if (rc == B_ERROR_SUCCESS) {
        playback_ip->playback_state = B_PlaybackIpState_ePaused;
        playback_ip->ipTrickModeSettings = *ipTrickModeSettings;
        BDBG_MSG(("%s: Successfully Paused IP Playback, ip state %d", __FUNCTION__, playback_ip->playback_state));
    }
    else {
        /* in case of error, reset the state back to current state */
        playback_ip->playback_state = currentState;
    }
    return rc;
}

B_PlaybackIpError
B_PlaybackIp_Pause(
    B_PlaybackIpHandle playback_ip,
    B_PlaybackIpTrickModesSettings *ipTrickModeSettings
    )
{
    B_PlaybackIpError rc = B_ERROR_SUCCESS;
    lock_ip_session(playback_ip);
    rc = trickModePause_locked(playback_ip, ipTrickModeSettings);
    /* Pause is a non-blocking API, so we always unlock before returning. */
    unlock_ip_session(playback_ip);
    return (rc);
}

#ifdef LIVEMEDIA_SUPPORT
B_PlaybackIpError
B_PlaybackIp_PlayRtsp(
    B_PlaybackIpHandle playback_ip
    )
{
#if 0
    B_PlaybackIpError rc;
    B_PlaybackIpTrickModesSettings ipTrickModeSettings;
    B_PlaybackIp_RtspCtrlCmd cmd;

    /* send resume command to RTSP server */
    cmd.type = B_PlaybackIp_RtspCtrlCmdType_Resume;
    /* (Note: start=-1 means 'resume'; end=-1 means 'play to end') */
    cmd.params.end = -1;
    cmd.params.scale = 1;
    B_PlaybackIp_liveMediaSessionRTSPControl(playback_ip->lm_context, &cmd);

    /* re-enable the live mode so that feeder thread can detect any discontinuities introduced due to longer n/w latency */
    playback_ip->disableLiveMode = false;

    /* program decoders to resume normal play */
    ipTrickModeSettings.rate = 1;
    ipTrickModeSettings.frameRepeat = 0;
    ipTrickModeSettings.decodeMode = NEXUS_VideoDecoderDecodeMode_eAll;
    if ((rc = updateNexusPlaypumpDecodersState(playback_ip, &ipTrickModeSettings)) != B_ERROR_SUCCESS) {
        BDBG_ERR(("%s: ERROR: failed to update nexus av decoder state during trickmode transition\n", __FUNCTION__));
        return rc;
    }
    return B_ERROR_SUCCESS;
#else
    BSTD_UNUSED(playback_ip);
    /* Until RTSP-ES support is complete, we are going to disable RTSP trickmodes */
    BDBG_ERR(("%s: Until RTSP-ES support is complete, we are going to disable RTSP trickmodes", __FUNCTION__));
    return B_ERROR_UNKNOWN;
#endif
}
#endif /* LIVEMEDIA_SUPPORT */

B_PlaybackIpError
B_PlaybackIp_PlayHttp(
    B_PlaybackIpHandle playback_ip,
    B_PlaybackIpState currentState
    )
{
    B_PlaybackIpError rc = B_ERROR_UNKNOWN;
    B_PlaybackIpTrickModesSettings ipTrickModeSettings =
    {   B_PlaybackIpTrickModesMethod_UseByteRange, 1, false, NEXUS_NORMAL_DECODE_RATE, 1, 0,
        B_PlaybackIpPauseMethod_UseConnectionStalling, 0, false, 0, false
        #ifndef DMS_CROSS_PLATFORMS
        ,NEXUS_VideoDecoderDecodeMode_eAll
        #endif
        ,false,
        NULL, false, false, B_PlaybackIpAdaptiveStreamingTrickModeMethod_eUseSegmentWithLowestBandwidth
    };

    double timeSeekRangeBegin;
    double timeSeekRangeEnd;
    NEXUS_PlaybackPosition playPosition;
    BSTD_UNUSED(currentState);

    if (!playback_ip->useNexusPlaypump) {
        /* resuming from Paused or Trickmode state: we dont need to worry about re-opening the HTTP session to the server in case pause/trickmode had disconnected the socket */
        /* this is because HTTP playback module will reconnect to the server & ask for correct byte offset when Nexus playback will call it for data and it is missing in HTTP cache */
        /* (only happens when IP data is pulled by Nexus Playback) */
        /* TODO: not sure what state was being set here in the old code, may need to experiment w/ different format playback when taking it out */
        playback_ip->playback_state = B_PlaybackIpState_ePlaying;
        BDBG_MSG(("%s: Calling Nexus Play", __FUNCTION__));
        if (NEXUS_Playback_Play(playback_ip->nexusHandles.playback) != NEXUS_SUCCESS) {
            BDBG_ERR(("%s: ERROR: Failed to Resume Nexus Playback", __FUNCTION__));
            rc = B_ERROR_UNKNOWN;
            goto error;
        }
#ifdef BDBG_DEBUG_BUILD
        if (playback_ip->ipVerboseLog)
            print_av_pipeline_buffering_status(playback_ip);
#endif
    }
    else {
        /* using Nexus Playpump, so this session must be using server side trickmodes (timeSeek and/or playspeed) */
        if (playback_ip->startSettings.mediaPositionUsingWallClockTime) {
            if (playback_ip->lastPosition > 0)
                playback_ip->lastPosition -= 1; /* go back by another sec so as to not miss any frames during play resume incase our position is not accurate */
            playPosition = playback_ip->lastPosition;
        }
        else if (playback_ip->useSeekPosition == true) {
            /* seekPosition is already set, use this position (it is the new seek point) for normal play */
            playPosition = playback_ip->seekPosition;
        }
        else {
            /* seekPosition is not set */
            /* we are resuming from either trickplay or paused state, so note the current position, so that we can resume normal play from that point */
            if (B_PlaybackIp_HttpGetCurrentPlaybackPosition(playback_ip, &playPosition) != B_ERROR_SUCCESS) {
                BDBG_ERR(("%s: Failed to determine the current playback position", __FUNCTION__));
                goto error;
            }
        }
        BDBG_MSG(("%s: resuming position %0.3f, reOpenSocket %d", __FUNCTION__, playPosition/1000., playback_ip->reOpenSocket));

        if (playback_ip->reOpenSocket == true || playback_ip->ipTrickModeSettings.method == B_PlaybackIpTrickModesMethod_UseRvuSpec || playback_ip->frameRewindCalled == true)
        {
            playback_ip->frameRewindCalled = false;
            /* reconnect to server for both trickmode case (other than when rate is 2) & pause w/ disconnect case */
            timeSeekRangeBegin = playPosition/1000.;
            if (playback_ip->ipTrickModeSettings.method == B_PlaybackIpTrickModesMethod_UseRvuSpec &&
                playback_ip->psi.psiValid && playback_ip->psi.duration > 0
               )
                timeSeekRangeEnd = playback_ip->psi.duration/1000.; /* not required to specify rangeEnd in non-Rvu cases */
            else
                timeSeekRangeEnd = 0.;
            /* build & send the new HTTP Get request to server so that it will send stream at new speed & time position */
            if ((rc = http_do_server_trick_modes_socket_transition(playback_ip, timeSeekRangeBegin, timeSeekRangeEnd, 1 /* play speed */, NULL /*playSpeedString */)) != B_ERROR_SUCCESS) {
                BDBG_ERR(("%s: Failed to update server about the trick play transition\n", __FUNCTION__));
                goto error;
            }
            playback_ip->reOpenSocket = false;

            /* flush the AV decoders & Nexus playpump buffers so that we cleanly start decode from the new stream position */
            if (B_PlaybackIp_UtilsFlushAvPipeline(playback_ip)) {
                BDBG_ERR(("%s: ERROR: Failed to flush the AV pipeline\n", __FUNCTION__));
                rc = B_ERROR_UNKNOWN;
                goto error;
            }
            /* since decoders are flushed, we can't return to play position using STC normal rate only, we will also need to let decoders know. */
            playback_ip->hwStcTrickMode = false;

            /* we reset the media start time flag as it will get set again during the 1st PTS interrupt */
            playback_ip->mediaStartTimeNoted = false;

            /* store last position we seek/resume from as this will be used in currentPosition calculation when we receive 1st PTS from this new stream point */
            playback_ip->lastSeekPosition = playPosition;
            playback_ip->lastSeekPositionSet = true;
        }

        /* now resume to normal play: the function below will decide either to just resume the STC or to also let decoders know. */
        /* this is done based on the hwStcTrickMode flag which would be set in the previous operation */
        /* when we had either paused w/ connection stalling or +ve trickplay upto 2x */
        ipTrickModeSettings.rate = 1; /* normal speed */
        ipTrickModeSettings.method = playback_ip->ipTrickModeSettings.method;
        playback_ip->speedNumerator = 1;
        playback_ip->speedDenominator = 1;
        if ((rc = updateNexusPlaypumpDecodersState(playback_ip, &ipTrickModeSettings)) != B_ERROR_SUCCESS) {
            BDBG_ERR(("%s: ERROR: failed to update nexus av decoder state during trickmode transition\n", __FUNCTION__));
            goto error;
        }
        playback_ip->ipTrickModeSettings = ipTrickModeSettings;

        /* reset flag indicating stc trickmode */
        playback_ip->hwStcTrickMode = false;
        playback_ip->simulatedStcTrickMode = false;
        playback_ip->stcRate = NEXUS_NORMAL_PLAY_SPEED;
    }
    BDBG_MSG(("%s: Successfully Resumed IP Playback, ip state %d, fd %d", __FUNCTION__, playback_ip->playback_state, playback_ip->socketState.fd));
    rc = B_ERROR_SUCCESS;
error:
    return rc;
}

/*
Summary:
Play/Resume playback.
 */
static B_PlaybackIpError
trickModePlay_locked(
    B_PlaybackIpHandle playback_ip
    )
{
    B_PlaybackIpError rc = B_ERROR_UNKNOWN;
    B_PlaybackIpState currentState;

    currentState = playback_ip->playback_state;
    playback_ip->speedDenominator = 1;
    playback_ip->speedNumerator = 1;
    playback_ip->mediaEndTimeNoted = false;

#ifdef B_HAS_HLS_PROTOCOL_SUPPORT
    if (playback_ip->hlsSessionEnabled)
        rc = B_PlaybackIp_PlayHls(playback_ip, currentState);
    else
#endif
#ifdef B_HAS_MPEG_DASH_PROTOCOL_SUPPORT
    if (playback_ip->mpegDashSessionEnabled)
        rc = B_PlaybackIp_PlayMpegDash(playback_ip);
    else
#endif
    if (playback_ip->protocol == B_PlaybackIpProtocol_eHttp)
        rc = B_PlaybackIp_PlayHttp(playback_ip, currentState);
#ifdef LIVEMEDIA_SUPPORT
    else if (playback_ip->protocol == B_PlaybackIpProtocol_eRtsp)
        rc = B_PlaybackIp_PlayRtsp(playback_ip);
#endif
    else {
        BDBG_ERR(("%s: ERROR: Play not supported for IP protocol %d", __FUNCTION__, playback_ip->protocol));
        rc = B_ERROR_NOT_SUPPORTED;
    }
    /* reset the paused position */
    playback_ip->seekPosition = 0;
    playback_ip->useSeekPosition = false;

    /* reset the mediaStartTime as we resume the Play from this position. lastPosition variable already contains the position we had played until last pause/trickmode/play event */
    B_Time_Get(&playback_ip->mediaStartTime);
    if (rc == B_ERROR_SUCCESS) {
        playback_ip->playback_state = B_PlaybackIpState_ePlaying;
        playback_ip->ipTrickModeSettings.rate = 1;
    }
    else {
        playback_ip->playback_state = currentState;
    }
    BDBG_MSG(("%s: Resuming to Play state, ip state %d", __FUNCTION__, playback_ip->playback_state));
    playback_ip->playApiActive = false;
    return rc;
}

static B_PlaybackIpError
B_PlaybackIp_PlayImpl(
    B_PlaybackIpHandle playback_ip,
    bool nonBlockingMode                 /* if set, API is run in the non-blocking mode and its completion is provided via B_PlaybackIpSessionOpenSettings.eventCallback. */
    )
{
    B_PlaybackIpError rc = B_ERROR_UNKNOWN;

    if (!playback_ip) {
        BDBG_ERR(("%s: NULL params playback_ip", __FUNCTION__));
        return B_ERROR_INVALID_PARAMETER;
    }

    BDBG_MSG(("%s: Resuming plaback_ip %p, current state %d, pause method %d, fd %d", __FUNCTION__, playback_ip, playback_ip->playback_state, playback_ip->ipTrickModeSettings.pauseMethod, playback_ip->socketState.fd));

    if (playback_ip->playback_state != B_PlaybackIpState_ePaused &&
            playback_ip->playback_state != B_PlaybackIpState_ePlaying &&
            playback_ip->playback_state != B_PlaybackIpState_eTrickMode) {
        BDBG_ERR(("%s: ERROR: Can't do Play command in this state %d\n", __FUNCTION__, playback_ip->playback_state));
        return B_ERROR_INVALID_PARAMETER;
    }

    /* if API is in progress, return INCOMPLETE */
    if (playback_ip->apiInProgress) {
        rc = B_ERROR_IN_PROGRESS;
        BDBG_MSG(("%s: previously started Play operation still in progress, playback_ip %p, status %d", __FUNCTION__, playback_ip, rc));
        return (rc);
    }

    /* if API is completed, return results to app */
    if (playback_ip->apiCompleted) {
        rc = playback_ip->trickModeStatus;
        playback_ip->apiCompleted = false;
        BDBG_MSG(("%s: previously started Play operation completed, playback_ip %p, status %d", __FUNCTION__, playback_ip, rc));
        goto out;
    }

    if (playback_ip->playback_state == B_PlaybackIpState_ePlaying) {
        BDBG_MSG(("%s playbackIp=%p: Ignoring B_PlaybackIp_Play() in the Playing State!", __FUNCTION__, playback_ip));
        return B_ERROR_SUCCESS;
    }

    lock_ip_session(playback_ip);
    BDBG_MSG(("%s playback_ip=%p: nonBlockingMode=%s", __FUNCTION__, playback_ip, nonBlockingMode?"Y":"N"));

    playback_ip->playApiActive = true;
    playback_ip->frameAdvanceApiActive = false;
    playback_ip->trickmodeApiActive = false;
    playback_ip->simulatedStcTrickMode = false;
    playback_ip->hwStcTrickMode = false;
    /* Check if app wants to do the trickplay in non-blocking mode. */
    if (nonBlockingMode) {
        /* Neither Play API is currently in progress nor it is completed, so start Play work */
        playback_ip->apiInProgress = true;
        playback_ip->apiCompleted = false;
        /* start the thread to carry out the any trickmode function if not already created */
        if (playback_ip->trickModeThread == NULL) {

            /* Be sure to create this event before starting the thread,
             * because the first thing the thread does is to wait on the
             * event. */
            if (BKNI_CreateEvent(&playback_ip->newTrickModeJobEvent)) {
                BDBG_ERR(("%s: Failed to create an event at %d", __FUNCTION__, __LINE__));
                rc = B_ERROR_UNKNOWN;
                goto error;
            }

            playback_ip->trickModeThread = B_Thread_Create("trickModeThread", (B_ThreadFunc)trickModeThread, (void *)playback_ip, NULL/*threadSettings*/);
            if (playback_ip->trickModeThread == NULL) {
                BDBG_ERR(("%s: Failed to create thread for trickmode operation during trickmode", __FUNCTION__));
                rc = B_ERROR_UNKNOWN;
                goto error;
            }
            BDBG_MSG(("%s: Successfully created thread & event for trickmode operations ", __FUNCTION__));
        }
        /* trickModeThread is either already running or we just started it, wake it up to start the seek work */
        BKNI_SetEvent(playback_ip->newTrickModeJobEvent);
        rc = B_ERROR_IN_PROGRESS;
    }
    else {
        /* Play is being called in the non-blocking mode, just call the function to do the actual work. */
        rc = trickModePlay_locked(playback_ip);
    }

out:
error:
    if (rc != B_ERROR_IN_PROGRESS) {
        unlock_ip_session(playback_ip);
        if (rc == B_ERROR_SUCCESS) {
            B_PlaybackIp_ResetPsiState(playback_ip->pPsiState);
        }
    }
    BDBG_MSG(("%s: returning rc =%d", __FUNCTION__, rc));
    return rc;
}

B_PlaybackIpError
B_PlaybackIp_PlayAsync(
    B_PlaybackIpHandle playback_ip
    )
{
    return (B_PlaybackIp_PlayImpl(playback_ip, true /* async */));
}

B_PlaybackIpError
B_PlaybackIp_Play(
    B_PlaybackIpHandle playback_ip
    )
{
    return (B_PlaybackIp_PlayImpl(playback_ip, false /* async */));
}

B_PlaybackIpError
B_PlaybackIp_SeekRvu(
    B_PlaybackIpHandle playback_ip,
    B_PlaybackIpTrickModesSettings *ipTrickModeSettings
    )
{
    BSTD_UNUSED(playback_ip);
    BSTD_UNUSED(ipTrickModeSettings);
    BDBG_ERR(("%s: ERROR: Rvu Seek is not yet supported", __FUNCTION__));
    return B_ERROR_SUCCESS;
}

B_PlaybackIpError
B_PlaybackIp_SeekRtsp(
    B_PlaybackIpHandle playback_ip,
    B_PlaybackIpTrickModesSettings *ipTrickModeSettings
    )
{
    BSTD_UNUSED(playback_ip);
    BSTD_UNUSED(ipTrickModeSettings);
    BDBG_ERR(("%s: ERROR: RTSP Seek is not yet supported", __FUNCTION__));
    return B_ERROR_SUCCESS;
}

B_PlaybackIpError
B_PlaybackIp_SeekHttp(
    B_PlaybackIpHandle playback_ip,
    NEXUS_PlaybackPosition seekPosition,
    B_PlaybackIpState currentState,
    B_PlaybackIpTrickModesSettings *ipTrickModeSettings
    )
{
    B_PlaybackIpError rc = B_ERROR_SUCCESS;
    NEXUS_Error nrc;

    if (!playback_ip->useNexusPlaypump) {
        if (playback_ip->psi.duration && seekPosition >= playback_ip->psi.duration) {
            BDBG_WRN(("%s: asked position (relative flag %d, seek backward %d, pos relative %u, absolute %d) is outside the file duration %u",
                        __FUNCTION__, ipTrickModeSettings->seekPositionIsRelative, ipTrickModeSettings->seekBackward, ipTrickModeSettings->seekPosition, seekPosition, playback_ip->psi.duration));
            seekPosition = 0;
        }
        BDBG_MSG(("%s: Calling Nexus Seek\n", __FUNCTION__));
        /* we need to unlock here, this is because for some streams that dont have index data, Nexus Seek */
        /* will invoke data callback of Http module. If we didn't release the lock here, the data callback wont */
        /* run as this lock serializes either this thread or data callback thread and thus seek would fail */

        unlock_ip_session(playback_ip);
        if ( (nrc = NEXUS_Playback_Seek(playback_ip->nexusHandles.playback, seekPosition)) != NEXUS_SUCCESS) {
            BDBG_ERR(("%s: ERROR: Failed to seek using NEXUS_Playback_Seek, rc %d", __FUNCTION__, nrc));
            rc = B_ERROR_UNKNOWN;
            goto error;
        }
        lock_ip_session(playback_ip);
        rc = B_ERROR_SUCCESS;
    }
    else {
        /* using Nexus Playpump to feed data, so server must support time seek */
        BDBG_MSG(("%s: currentState %d, seekPosition %d", __FUNCTION__, currentState, seekPosition));
        if (currentState == B_PlaybackIpState_ePaused) {
            double timeSeekRangeBegin;
            double timeSeekRangeEnd;
            /* logic for seeking during paused state */
            timeSeekRangeBegin = seekPosition/1000.;
            /* NOTE: we dont send end time anymore, as it confuses some servers if our time is slightly larger than the actual duration */
            timeSeekRangeEnd = 0.;
            /* build & send the new HTTP Get request to server so that it will send data from new time position */
            if ((rc = http_do_server_trick_modes_socket_transition(playback_ip, timeSeekRangeBegin, timeSeekRangeEnd, 1 /* play speed */, NULL /*playSpeedString */)) != B_ERROR_SUCCESS) {
                BDBG_ERR(("%s: Failed to update server about the trick play transition\n", __FUNCTION__));
                goto error;
            }
            /* clear the seek position, so that next play will start from the already seeked point */
            playback_ip->seekPosition = 0;

            /* flush the current pipeline so frame advance will show new frame */
            if (B_PlaybackIp_UtilsFlushAvPipeline(playback_ip)) {
                BDBG_ERR(("%s: ERROR: Failed to flush the AV pipeline\n", __FUNCTION__));
                rc = B_ERROR_UNKNOWN;
                goto error;
            }

            /* we reset the media start time flag as it will get set again during the 1st PTS interrupt */
            playback_ip->mediaStartTimeNoted = false;

            /* since we are in paused state, we need to resume playpump so that new data can be feed to the decoders */
            if (NEXUS_Playpump_SetPause(playback_ip->nexusHandles.playpump, false) != NEXUS_SUCCESS) {
                BDBG_ERR(("%s: NEXUS_Playpump_SetPause() failed to enable pause", __FUNCTION__));
                rc = B_ERROR_UNKNOWN;
                goto error;
            }
            if (playback_ip->nexusHandles.playpump2) {
                if (NEXUS_Playpump_SetPause(playback_ip->nexusHandles.playpump2, false) != NEXUS_SUCCESS) {
                    BDBG_ERR(("%s: NEXUS_Playpump_SetPause() for playpump2 failed to enable pause", __FUNCTION__));
                    rc = B_ERROR_UNKNOWN;
                    goto error;
                }
            }
            /* allow Ip_PlayPump thread to feed some more data from the socket */
            playback_ip->playback_state = B_PlaybackIpState_ePlaying;
            unlock_ip_session(playback_ip);
            /* TODO: this sleep seems too long, look into this */
            BKNI_Sleep(1000);
            /* data for 1 frame worth should be fed to decoder by now, so get the control back from Feeder thread */
            lock_ip_session(playback_ip);
            playback_ip->playback_state = B_PlaybackIpState_ePaused;

            if (playback_ip->nexusHandles.videoDecoder) {
                /* advance one frame to show the new seeked position */
                if (NEXUS_VideoDecoder_FrameAdvance(playback_ip->nexusHandles.videoDecoder) != NEXUS_SUCCESS) {
                    BDBG_ERR(("%s: ERROR: NEXUS_VideoDecoder_FrameAdvance() Failed \n", __FUNCTION__));
                    rc = B_ERROR_UNKNOWN;
                    goto error;
                }
                BDBG_MSG(("%s: Displayed the frame from the seeked position using NEXUS_VideoDecoder_FrameAdvance()", __FUNCTION__));
            }
            /* re-pause the playpump so that it doesn't expect or feed anymore data */
            if (NEXUS_Playpump_SetPause(playback_ip->nexusHandles.playpump, true) != NEXUS_SUCCESS) {
                BDBG_ERR(("%s: NEXUS_Playpump_SetPause() failed to disable pause", __FUNCTION__));
                rc = B_ERROR_UNKNOWN;
                goto error;
            }
            if (playback_ip->nexusHandles.playpump2) {
                if (NEXUS_Playpump_SetPause(playback_ip->nexusHandles.playpump2, true) != NEXUS_SUCCESS) {
                    BDBG_ERR(("%s: NEXUS_Playpump_SetPause() for playpump2 failed to disable pause", __FUNCTION__));
                    rc = B_ERROR_UNKNOWN;
                    goto error;
                }
            }

            /* close the current socket */
            if (playback_ip->netIo.shutdown && !playback_ip->serverClosed)
                playback_ip->netIo.shutdown(playback_ip->securityHandle, 0);
            /* set flag to reconnect at the pause time */
            close(playback_ip->socketState.fd);
            playback_ip->socketState.fd = -1;
            playback_ip->reOpenSocket = true;
            BDBG_MSG(("%s: Completed Seek During Pause", __FUNCTION__));
        }
        rc = B_ERROR_SUCCESS;
    }
error:
    return rc;
}

static B_PlaybackIpError
trickModeSeek_locked(
    B_PlaybackIpHandle playback_ip,
    B_PlaybackIpTrickModesSettings *ipTrickModeSettings
    )
{
    B_PlaybackIpError rc = B_ERROR_UNKNOWN;
    NEXUS_PlaybackPosition seekPosition; /* absolute position in msec */
    B_PlaybackIpState currentState;

    /* note: caller has already locked the session */
    /* currently assumes only seek call are invoked thru this function */
    BDBG_MSG(("%s: Seeking (%s) to %u msec of %s position in %s direction, fd %d, ip state %d\n", __FUNCTION__,
                ipTrickModeSettings->nonBlockingMode? "non-blocking mode":"blocking mode",
                ipTrickModeSettings->seekPosition,
                ipTrickModeSettings->seekPositionIsRelative? "relative":"absolute",
                ipTrickModeSettings->seekBackward? "backward":"forward", playback_ip->socketState.fd, playback_ip->playback_state));
    currentState = playback_ip->playback_state;
    if (playback_ip->playback_state != B_PlaybackIpState_ePaused) {
        /* Pause the playback only if we are currently not in the paused state */
        /* we are forcing to use the DisconnectAndSeek method of pausing as seek will anyway require reconnecting at a different offset */
        ipTrickModeSettings->pauseMethod = B_PlaybackIpPauseMethod_UseDisconnectAndSeek;
        if (trickModePause_locked(playback_ip, ipTrickModeSettings)) {
            BDBG_ERR(("%s: ERROR: Failed to pause Ip playback during Seek operation", __FUNCTION__));
            return B_ERROR_UNKNOWN;
        }
    }

    if (playback_ip->serverClosed) {
        /* reset this flag incase server was done sending the complete file as we may be seeking back */
        /* if server had not closed, Ip_Pause() will set the reOpenSocket flag */
        playback_ip->serverClosed = false;
        /* since server has closed, set a flag to re-open the socket */
        playback_ip->reOpenSocket = true; /* this flags tells HTTP layer to reopen the connection */
    }

    /* determine absolute seek position if user specified relative position */
    if (ipTrickModeSettings->seekPositionIsRelative) {
        /* get the current position */
        NEXUS_PlaybackPosition currentPosition;
        if (B_PlaybackIp_HttpGetCurrentPlaybackPosition(playback_ip, &currentPosition) != B_ERROR_SUCCESS) {
            BDBG_ERR(("%s: Failed to determine the current playback position\n", __FUNCTION__));
            goto error;
        }
        BDBG_MSG(("%s: current position %0.3f", __FUNCTION__, currentPosition/1000.));

        /* adjust relative value into the current position determined by the Pause() call above */
        if (ipTrickModeSettings->seekBackward)
            seekPosition = currentPosition - ipTrickModeSettings->seekPosition;
        else
            seekPosition = currentPosition + ipTrickModeSettings->seekPosition;
    }
    else {
        seekPosition = ipTrickModeSettings->seekPosition;
    }
    BDBG_MSG(("%s: updated seek position from %d to %d (in msec) \n", __FUNCTION__, ipTrickModeSettings->seekPosition, seekPosition));

    /* Now seek to the new position */
#ifdef B_HAS_HLS_PROTOCOL_SUPPORT
    if (playback_ip->hlsSessionEnabled)
        rc = B_PlaybackIp_SeekHls(playback_ip, currentState, seekPosition, ipTrickModeSettings->enableAccurateSeek, true /* flushPipeline */);
    else
#endif
#ifdef B_HAS_MPEG_DASH_PROTOCOL_SUPPORT
    if (playback_ip->mpegDashSessionEnabled)
        rc = B_PlaybackIp_SeekMpegDash(playback_ip, seekPosition);
    else
#endif
    if (playback_ip->protocol == B_PlaybackIpProtocol_eHttp)
        rc = B_PlaybackIp_SeekHttp(playback_ip, seekPosition, currentState, ipTrickModeSettings);
    else if (playback_ip->protocol == B_PlaybackIpProtocol_eRtsp)
        rc = B_PlaybackIp_SeekRtsp(playback_ip, ipTrickModeSettings);
    else if (playback_ip->ipTrickModeSettings.method == B_PlaybackIpTrickModesMethod_UseRvuSpec)
        rc = B_PlaybackIp_SeekRvu(playback_ip, ipTrickModeSettings);
    else {
        BDBG_ERR(("%s: ERROR: Seek not supported for IP protocol %d", __FUNCTION__, playback_ip->protocol));
        rc = B_ERROR_NOT_SUPPORTED;
    }

    if (rc == B_ERROR_SUCCESS) {
        /* store the position to seek to, it will be used in the Ip_Play() */
        playback_ip->seekPosition = seekPosition;
        /* store last position we seek/resume from as this will be used in currentPosition calculation when we receive 1st PTS from this new stream point */
        playback_ip->lastSeekPosition = seekPosition;
        playback_ip->lastSeekPositionSet = true;
        playback_ip->useSeekPosition = true;
        playback_ip->lastPosition = seekPosition;
    }
    else {
        goto error;
    }

    if (currentState == B_PlaybackIpState_ePlaying || currentState == B_PlaybackIpState_eTrickMode) {
        /* We resume to play state if we were originally in Playing or TrickMode state */
        /* Note: we do so even though seek may have failed as we had paused the playback, so we continue playing igroring the seek affect */
        BDBG_MSG(("%s: %p: Calling trickModePlay_locked", __FUNCTION__, playback_ip));
        if (trickModePlay_locked(playback_ip)) {
            BDBG_ERR(("%s: ERROR: Failed to play\n", __FUNCTION__));
            rc = B_ERROR_UNKNOWN;
            goto error;
        }
    }
    rc = B_ERROR_SUCCESS;
    BDBG_MSG(("%s: Seeked to position %u msec, fd %d", __FUNCTION__,  seekPosition, playback_ip->socketState.fd));
    return rc;
error:
    BDBG_ERR(("%s: Seek failed for playback_ip session %p, fd %d", __FUNCTION__,  playback_ip, playback_ip->socketState.fd));
    return rc;
}

static void
trickModeThread(
    void *data
    )
{
    B_PlaybackIpHandle playback_ip = (B_PlaybackIpHandle)data;
    BERR_Code rc;

    BDBG_MSG(("%s: started, playback_ip %p", __FUNCTION__, playback_ip));

    while (true) {
        if ((rc = BKNI_WaitForEvent(playback_ip->newTrickModeJobEvent, 1000)) != BERR_SUCCESS && rc != BERR_TIMEOUT) {
            BDBG_ERR(("%s: got error while waiting for new trickMode job event, rc %d", __FUNCTION__, rc));
            break;
        }
        if (playback_ip->playback_state == B_PlaybackIpState_eStopping || playback_ip->playback_state == B_PlaybackIpState_eStopped) {
            /* user changed the channel, so return */
            BDBG_MSG(("%s: breaking out of trickMode thread due to state (%d) change", __FUNCTION__, playback_ip->playback_state));
            break;
        }
        if (rc == BERR_TIMEOUT) {
            continue;
        }

        /* We are here that means user wants us to do a trickmode job */
        BDBG_MSG(("%s: calling func to handle %s ", __FUNCTION__, playback_ip->trickmodeApiActive? "TrickMode API" : playback_ip->frameAdvanceApiActive? "FrameAdvance API": playback_ip->playApiActive? "Play API":"Seek API"));
        if (playback_ip->trickmodeApiActive == true) {
            playback_ip->trickModeStatus = trickModeTrick_locked(playback_ip, &playback_ip->ipTrickModeSettings);
        }
        else if (playback_ip->frameAdvanceApiActive == true) {
            playback_ip->trickModeStatus = trickModeFrameAdvance_locked(playback_ip, playback_ip->forward);
        }
        else if (playback_ip->playApiActive == true) {
            playback_ip->trickModeStatus = trickModePlay_locked(playback_ip);
        }
        else {
            playback_ip->trickModeStatus = trickModeSeek_locked(playback_ip, &playback_ip->ipTrickModeSettings);
        }

        playback_ip->apiInProgress = false;
        playback_ip->apiCompleted = true;
        if (playback_ip->openSettings.eventCallback && playback_ip->playback_state != B_PlaybackIpState_eStopping && playback_ip->playback_state != B_PlaybackIpState_eStopped)
            playback_ip->openSettings.eventCallback(playback_ip->openSettings.appCtx, B_PlaybackIpEvent_eSeekComplete);
    }

    playback_ip->trickModeThreadDone = true;
    BDBG_MSG(("%s: done", __FUNCTION__));
}

B_PlaybackIpError
B_PlaybackIp_Seek(
    B_PlaybackIpHandle playback_ip,
    B_PlaybackIpTrickModesSettings *ipTrickModeSettings
    )
{
    B_PlaybackIpError rc;

    if (!playback_ip || !ipTrickModeSettings) {
        BDBG_ERR(("%s: ERROR: incorrect params: playback_ip %p, trickMode %p\n", __FUNCTION__, playback_ip, ipTrickModeSettings));
        return B_ERROR_INVALID_PARAMETER;
    }

    if (playback_ip->psi.liveChannel) {
        /* TODO: take this check out once trickmodes on live channels are supported */
        BDBG_WRN(("%s: ERROR: Seek not supported for LIVE Channels yet, ignoring seek command", __FUNCTION__));
        return B_ERROR_SUCCESS;
    }

    if (playback_ip->protocol != B_PlaybackIpProtocol_eHttp) {
        /* TODO: take this check out once RTSP & other protocols are supported */
        BDBG_ERR(("%s: ERROR: Seek not supported for IP protocol %d, ignoring seek command", __FUNCTION__, playback_ip->protocol));
        return B_ERROR_SUCCESS;
    }

    if (playback_ip->psi.duration && ipTrickModeSettings->seekPosition >= playback_ip->psi.duration) {
        BDBG_ERR(("%s: Ignoring Seek to %d msec as it is beyond the stream duration %d", __FUNCTION__,
                    ipTrickModeSettings->seekPosition, playback_ip->psi.duration));
        return B_ERROR_SUCCESS;
    }
    /* if seek is in progress, return INCOMPLETE */
    if (playback_ip->apiInProgress)
        return B_ERROR_IN_PROGRESS;

    /* if seek is completed, return results to app */
    if (playback_ip->apiCompleted) {
        rc = playback_ip->trickModeStatus;
        playback_ip->apiCompleted = false;
        BDBG_MSG(("%s: previously started seek operation completed, playback_ip %p, trickModeStatus %d", __FUNCTION__, playback_ip, rc));
        goto out;
    }

    /* ensure no other threads are working on this playback_ip session */
    lock_ip_session(playback_ip);

    playback_ip->trickmodeApiActive = false;
    playback_ip->playApiActive = false;
    playback_ip->frameAdvanceApiActive = false;
    playback_ip->mediaEndTimeNoted = false; /* just in case trickplay started when we were at the end */
    if (ipTrickModeSettings->nonBlockingMode) {
        /* Neither seek is in progress nor it is completed, so start seek work */
        playback_ip->apiInProgress = true;
        playback_ip->apiCompleted = false;
        /* start the thread to carry out the seek function if not already created */
        if (playback_ip->trickModeThread == NULL) {

            /* Be sure to create this event before starting the thread,
             * because the first thing the thread does is to wait on the
             * event. */
            if (BKNI_CreateEvent(&playback_ip->newTrickModeJobEvent)) {
                BDBG_ERR(("%s: Failed to create an event at %d", __FUNCTION__, __LINE__));
                rc = B_ERROR_UNKNOWN;
                goto error;
            }

            playback_ip->trickModeThread = B_Thread_Create("trickModeThread", (B_ThreadFunc)trickModeThread, (void *)playback_ip, NULL/*threadSettings*/);
            if (playback_ip->trickModeThread == NULL) {
                BDBG_ERR(("%s: Failed to create thread for trickmode operation during seek", __FUNCTION__));
                rc = B_ERROR_UNKNOWN;
                goto error;
            }
            BDBG_MSG(("%s: Successfully created thread & event for trickmode operations ", __FUNCTION__));
        }
        /* save the settings so that thread function can use it */
        playback_ip->ipTrickModeSettings = *ipTrickModeSettings;
        /* trickModeThread is either already running or we just started it, wake it up to start the seek work */
        BKNI_SetEvent(playback_ip->newTrickModeJobEvent);
        rc = B_ERROR_IN_PROGRESS;
        BDBG_MSG(("%s: started seek processing in non-blocking mode (playback_ip lock is held)", __FUNCTION__));
    }
    else {
        /* seek function is being called in the non-blocking mode, just call the function to do the seek work */
        rc = trickModeSeek_locked(playback_ip, ipTrickModeSettings);
    }

out:
error:
    if (rc != B_ERROR_IN_PROGRESS) {
        unlock_ip_session(playback_ip);
        if (rc == B_ERROR_SUCCESS) {
            B_PlaybackIp_ResetPsiState(playback_ip->pPsiState);
        }
    }
    return rc;
}

B_PlaybackIpError B_PlaybackIp_GetTrickModeByRate(
    B_PlaybackIpHandle playback_ip,
    int rate,
    B_PlaybackIpTrickModesSettings *trickMode
    )
{
    /* bool allow_brcm_trick_modes = bsettop_get_config("no_brcm_trick_modes") == NULL; */
    BSTD_UNUSED(playback_ip);
    BSTD_UNUSED(trickMode);
    BDBG_MSG(("%s: rate: %d", __FUNCTION__, rate));

    /* this function is supposed to return the trickmode parameters for a given rate. However, it doesn't make sense for IP trickmodes */
    /* as IP trickmodes parameters are quite high level at this time (e.g. take only rate and trick mode type as input) */
    return B_ERROR_SUCCESS;
}

B_PlaybackIpError
B_PlaybackIp_TrickModeHttp(
    B_PlaybackIpHandle playback_ip,
    B_PlaybackIpTrickModesSettings *ipTrickModeSettings,
    B_PlaybackIpState currentState
    )
{
    B_PlaybackIpError rc = B_ERROR_UNKNOWN;
    double timeSeekRangeBegin;
    double timeSeekRangeEnd;
    float rate;
    NEXUS_PlaybackPosition currentPosition;

    if (!playback_ip->useNexusPlaypump) {
        if ((rc = http_do_client_side_trick_modes_using_playback(playback_ip, ipTrickModeSettings)) != B_ERROR_SUCCESS) {
            BDBG_ERR(("%s: ERROR: Trick mode transition failed \n", __FUNCTION__));
            goto error;
        }
#ifdef BDBG_DEBUG_BUILD
        if (playback_ip->ipVerboseLog)
            print_av_pipeline_buffering_status(playback_ip);
#endif
    }
    else {
        /*
         * Since the caller has already parsed the app provided trickmode parameters into speed numberator & denominator,
         * we choose the appropriate trickmodes based on these values as follows:
         *
         * -HW STC based for +ve speeds upto 2x including +ve slow fwd speeds
         * -Simulated STC based for >2x +ve & all -ve speeds when FrameRateInTrickMode header is not provided by Server or App.
         * -Decoder trickmode for pause & rest of trickmodes when above conditions are not met!
         */

        /* Now check what type of trickmode we should perform */
        rate = (float) playback_ip->speedNumerator / playback_ip->speedDenominator;
        if (rate > 0.0 && rate <= 2.0) {
            /* +ve speeds upto 2x including slow fwd: */
            /* we will just manipulate Hardware STC to match the requested speed. */
            playback_ip->hwStcTrickMode = true;
            playback_ip->simulatedStcTrickMode = false;
        }
        else {
            /* we either have -ve speed trickmode (including slow rewind) or > 2x +ve fast fwd trickmode */
            if (!playback_ip->psi.frameRateInTrickMode && !ipTrickModeSettings->frameRateInTrickMode) {
            /* Server didn't provide the trickmode frame rate & app doesn't know a suitable value either. */
            /* In this case, we use simualted STC based trickmodes! */
            /* We change software STC rate (maintained by decoder) to match the speed. */
            /* Audio decoder is muted & stopped and video decoder will use TSM to decode I-frames at correct PTS interval. */
                playback_ip->simulatedStcTrickMode = true;
                playback_ip->hwStcTrickMode = false;
            }
            else {
                /* we will program the decoder rate to match the frame rate in trickmode and disable TSM. */
                playback_ip->simulatedStcTrickMode = false;
                playback_ip->hwStcTrickMode = false;
            }
        }

        {
            /* Since server side trickmode is being requested, then we must send the new playSpeed request to the server */

            if (B_PlaybackIp_HttpGetCurrentPlaybackPosition(playback_ip, &currentPosition) != B_ERROR_SUCCESS) {
                BDBG_WRN(("%s: Failed to determine the current playback position", __FUNCTION__));
                goto error;
            }
            BDBG_MSG(("%s: new trickplay starting position %0.3f", __FUNCTION__, currentPosition/1000.));

            /* build & send the new HTTP Get request to server so that it will send stream at new speed & time position */
            timeSeekRangeBegin = currentPosition/1000.;
            if (ipTrickModeSettings->method == B_PlaybackIpTrickModesMethod_UseRvuSpec || ipTrickModeSettings->method == B_PlaybackIpTrickModesMethod_UseTimeSeekRange) {
#define FRAMES_DISPLAYED_PER_SEC 5.0
                if (ipTrickModeSettings->rate > 0) {
                    playback_ip->timeSeekRangeInterval = ipTrickModeSettings->rate*1.0 / FRAMES_DISPLAYED_PER_SEC;
                    timeSeekRangeEnd = timeSeekRangeBegin + playback_ip->timeSeekRangeInterval;
                    playback_ip->initialTimeSeekRangeBegin = timeSeekRangeEnd;
                }
                else {
                    playback_ip->timeSeekRangeInterval = ipTrickModeSettings->rate*-1.0 / FRAMES_DISPLAYED_PER_SEC;
                    timeSeekRangeEnd = timeSeekRangeBegin;
                    timeSeekRangeBegin -= playback_ip->timeSeekRangeInterval;
                    playback_ip->initialTimeSeekRangeBegin = timeSeekRangeBegin;
                }
            }
            else {
                if (ipTrickModeSettings->dontUseTimeSeekRangeInPlaySpeed)
                    /* Setting the start value to 0 will make us not include the TimeSeekRange Headers in the outgoing requests. */
                    timeSeekRangeBegin = 0.;
                timeSeekRangeEnd = 0.;
            }

            BDBG_MSG(("%s: timeSeekRangeBegin %0.3f, timeSeekRangeEnd %0.3f, interval %0.3f, state %d, playSpeedString %s",
                        __FUNCTION__, timeSeekRangeBegin, timeSeekRangeEnd, playback_ip->timeSeekRangeInterval, playback_ip->playback_state,   ipTrickModeSettings->playSpeedStringDefined ? ipTrickModeSettings->playSpeedString : NULL));
            if ((rc = http_do_server_trick_modes_socket_transition(playback_ip, timeSeekRangeBegin, timeSeekRangeEnd, playback_ip->speedNumerator/playback_ip->speedDenominator /* play speed */,
                            ipTrickModeSettings->playSpeedStringDefined ? ipTrickModeSettings->playSpeedString : NULL)) != B_ERROR_SUCCESS) {
                BDBG_ERR(("%s: Failed to update server about the trick play transition\n", __FUNCTION__));
                goto error;
            }
            /* store last position we start trickmode from as this will be used in currentPosition calculation when we receive 1st PTS from this new stream point */
            playback_ip->lastSeekPosition = currentPosition;
            playback_ip->lastSeekPositionSet = true;

            /* flush the AV decoders & Nexus playpump buffers so that we cleanly start decode from the new stream position */
#if 0
            /* TODO: need to see if there is a lag because of this flush during trickmode transition! */
            if (currentState != B_PlaybackIpState_eTrickMode)
#else
            BSTD_UNUSED(currentState);
#endif
            {
                /* we dont flush pipeline if we are already in the trick mode state */
                if (B_PlaybackIp_UtilsFlushAvPipeline(playback_ip)) {
                    BDBG_ERR(("%s: ERROR: Failed to flush the AV pipeline\n", __FUNCTION__));
                    rc = B_ERROR_UNKNOWN;
                    goto error;
                }

                /* we reset the media start time flag as it will get set again during the 1st PTS interrupt */
                playback_ip->mediaStartTimeNoted = false;
            }
            /* set a flag to indicate to Ip_Play() to reopen the connection with normal speed when play is resumed */
            playback_ip->reOpenSocket = true;
        }

        /* now tell decoder & playpump to resume */
        if ((rc = updateNexusPlaypumpDecodersState(playback_ip, ipTrickModeSettings)) != B_ERROR_SUCCESS) {
            BDBG_ERR(("%s: ERROR: failed to update nexus av decoder state during trickmode transition\n", __FUNCTION__));
            goto error;
        }
    }
    playback_ip->frameRewindCalled = false;
    return B_ERROR_SUCCESS;
error:
    return rc;
}

#ifdef LIVEMEDIA_SUPPORT
B_PlaybackIpError
B_PlaybackIp_TrickModeRtsp(
    B_PlaybackIpHandle playback_ip,
    B_PlaybackIpTrickModesSettings *ipTrickModeSettings
    )
{

    BSTD_UNUSED(playback_ip);
    BSTD_UNUSED(ipTrickModeSettings);
    /* Until RTSP-ES support is complete, we are going to disable RTSP trickmodes */
    BDBG_ERR(("%s: Until RTSP-ES support is complete, we are going to disable RTSP trickmodes", __FUNCTION__));
    return B_ERROR_UNKNOWN;

#ifdef TODO
    B_PlaybackIp_RtspCtrlCmd cmd;
    /* send command to server to start trick play at given rate */
    cmd.type = B_PlaybackIp_RtspCtrlCmdType_Resume;
    /* (Note: start=-1 means 'resume'; end=-1 means 'play to end') */
    cmd.params.start = -1;
    cmd.params.end = -1;
    cmd.params.scale = ipTrickModeSettings->rate;
    B_PlaybackIp_liveMediaSessionRTSPControl(playback_ip->lm_context, &cmd);

    /* disable the live mode so that feeder thread doesn't detect any discontinuities introduced due to longer n/w latency */
    playback_ip->disableLiveMode = true;

    /* update decoder settings */
    ipTrickModeSettings->frameRepeat = 0;
    if (ipTrickModeSettings->decodeMode > NEXUS_VideoDecoderDecodeMode_eI) {
        /* user specified invalid decodeMode, force to decode all pictures */
        ipTrickModeSettings->decodeMode = NEXUS_VideoDecoderDecodeMode_eAll;
    }
    if (updateNexusPlaypumpDecodersState(playback_ip, ipTrickModeSettings) != B_ERROR_SUCCESS) {
        BDBG_ERR(("%s: ERROR: failed to update nexus av decoder state during trickmode transition\n", __FUNCTION__));
        return B_ERROR_UNKNOWN;
    }
    return B_ERROR_SUCCESS;
#endif
}
#endif /* LIVEMEDIA_SUPPORT */

static B_PlaybackIpError
trickModeTrick_locked(
    B_PlaybackIpHandle playback_ip,
    B_PlaybackIpTrickModesSettings *ipTrickModeSettings
    )
{
    B_PlaybackIpError rc = B_ERROR_UNKNOWN;
    B_PlaybackIpState currentState;

    /* note: caller has already locked the session */

    BDBG_MSG(("%s: Starting trickmode work: speedNumerator %d, speedDenominator %d", __FUNCTION__,
                ipTrickModeSettings->rate, playback_ip->speedNumerator, playback_ip->speedDenominator));

    /* change the state to EnteringTrickMode until we finish the trickmode work */
    currentState = playback_ip->playback_state;
    playback_ip->playback_state = B_PlaybackIpState_eEnteringTrickMode;

#ifdef B_HAS_HLS_PROTOCOL_SUPPORT
    if (playback_ip->hlsSessionEnabled && (playback_ip->speedNumerator >= 2 || playback_ip->speedNumerator < 0))
        /*
         * NOTE: FFwd or Slow/Fast Rewind cases only need HLS Specific processing,
         * Slow Fwd is handled below along side the regular Http case.
         */
        rc = B_PlaybackIp_TrickModeHls(playback_ip, currentState, ipTrickModeSettings);
    else
#endif
    if (playback_ip->protocol == B_PlaybackIpProtocol_eHttp)
        rc = B_PlaybackIp_TrickModeHttp(playback_ip, ipTrickModeSettings, currentState);
#ifdef LIVEMEDIA_SUPPORT
    else if (playback_ip->protocol == B_PlaybackIpProtocol_eRtsp)
        rc = B_PlaybackIp_TrickModeRtsp(playback_ip, ipTrickModeSettings);
#endif
    else {
        BDBG_ERR(("%s: ERROR: TrickMode not supported for IP protocol %d", __FUNCTION__, playback_ip->protocol));
        rc = B_ERROR_NOT_SUPPORTED;
    }

    if (rc == B_ERROR_SUCCESS) {
        playback_ip->playback_state = B_PlaybackIpState_eTrickMode;
        playback_ip->ipTrickModeSettings = *ipTrickModeSettings;
        if (0)
        {
            /* I dont think I need to update the lastPosition again as it is done when getting currentPosition which was sent to server in the playSpeed request */
            B_Time currentTime;
            B_Time_Get(&currentTime);
            playback_ip->lastPosition += B_Time_Diff(&currentTime, &playback_ip->mediaStartTime);
            playback_ip->mediaStartTime = currentTime;
            playback_ip->mediaEndTimeNoted = false; /* just in case trickplay started when we were at the end */
            BDBG_MSG(("%s: ##### time played %u until trickmode", __FUNCTION__, playback_ip->lastPosition));
        }
    }
    else {
        playback_ip->playback_state = currentState;
    }
    playback_ip->trickmodeApiActive = false;
    return rc;
}

B_PlaybackIpError B_PlaybackIp_TrickMode(
    B_PlaybackIpHandle playback_ip,
    B_PlaybackIpTrickModesSettings *ipTrickModeSettings
    )
{
    B_PlaybackIpError rc = B_ERROR_UNKNOWN;

    if (!playback_ip || !ipTrickModeSettings) {
        BDBG_ERR(("%s: NULL params (playback_ip %p, trickModeSettings %p)\n", __FUNCTION__, playback_ip, ipTrickModeSettings));
        return B_ERROR_INVALID_PARAMETER;
    }

    if (playback_ip->psi.liveChannel) {
        /* TODO: take this check out once trickmodes on live channels are supported */
        BDBG_WRN(("%s: ERROR: Seek not supported for LIVE Channels yet, ignoring seek command", __FUNCTION__));
        return B_ERROR_SUCCESS;
    }

    if ((ipTrickModeSettings->method == B_PlaybackIpTrickModesMethod_UseTimeSeekRange ||
         ipTrickModeSettings->method == B_PlaybackIpTrickModesMethod_UsePlaySpeed ||
         ipTrickModeSettings->method == B_PlaybackIpTrickModesMethod_UseRvuSpec) && !playback_ip->useNexusPlaypump) {
        BDBG_ERR(("%s: trick mode method %d requires App to configure IP Session using Nexus Plapump, ignoring the command", __FUNCTION__, ipTrickModeSettings->method));
        return B_ERROR_SUCCESS;
    }
    if (ipTrickModeSettings->method == B_PlaybackIpTrickModesMethod_UseByteRange && playback_ip->useNexusPlaypump) {
        BDBG_ERR(("%s: trick mode method %d requires App to configure IP Session using Nexus Playback, ignoring the command", __FUNCTION__, ipTrickModeSettings->method));
        return B_ERROR_SUCCESS;
    }
    BDBG_MSG(("%s: trick mode method %d, rate %d, playSpeedString %s", __FUNCTION__, ipTrickModeSettings->method, ipTrickModeSettings->rate, ipTrickModeSettings->playSpeedStringDefined?ipTrickModeSettings->playSpeedString:""));

    /* App can specific playSpeed using 3 ways:
     * -playSpeedString (allows any speed value),
     * -rate (allows integer speed calues),
     * -absoluteRate (any speed value normalized over NEXUS_NORMAL_DECODE_RATE).
     *
     * Our goal is to initialize speedNumerator & speedDenominator using values specified by any of these methods.
     */
    if (ipTrickModeSettings->playSpeedStringDefined) {
        /* App is specifiying Server side trickmodes using playSpeedString, so parse it. */
        if (B_PlaybackIp_UtilsParsePlaySpeedString(ipTrickModeSettings->playSpeedString, &playback_ip->speedNumerator, &playback_ip->speedDenominator, NULL) < 0) {
            BDBG_ERR(("%s: Failed to parse the playSpeedString %s", __FUNCTION__, ipTrickModeSettings->playSpeedString));
            return (B_ERROR_INVALID_PARAMETER);
        }
        BDBG_MSG(("%s: playSpeedSring %s, speedNumerator %d, speedDenominator %d", __FUNCTION__,
                    ipTrickModeSettings->playSpeedString, playback_ip->speedNumerator, playback_ip->speedDenominator));
    }
    else if (ipTrickModeSettings->absoluteRateDefined) {
        if (abs(ipTrickModeSettings->absoluteRate) >= NEXUS_NORMAL_DECODE_RATE) {
            /* fast fwd or fast rewind case */
            playback_ip->speedNumerator = ipTrickModeSettings->absoluteRate / NEXUS_NORMAL_DECODE_RATE;
            playback_ip->speedDenominator = 1;
        }
        else {
            /* slow fwd or slow rewind case */
            playback_ip->speedNumerator = 1 * (ipTrickModeSettings->absoluteRate < 0 ? -1: 1);
            playback_ip->speedDenominator = NEXUS_NORMAL_DECODE_RATE / abs(ipTrickModeSettings->absoluteRate) ;
            if (ipTrickModeSettings->method == B_PlaybackIpTrickModesMethod_UsePlaySpeed) {
                /* build a playSpeed string for this fractional speed case. */
                memset(playback_ip->playSpeedString, 0, sizeof(playback_ip->playSpeedString));
                snprintf(playback_ip->playSpeedString,  sizeof(playback_ip->playSpeedString)-1, "%d/%d", playback_ip->speedNumerator, playback_ip->speedDenominator);
                BDBG_MSG(("%s: rate %d, speedNumerator %d, speedDenominator %d, playSpeedString %s", __FUNCTION__,
                    ipTrickModeSettings->absoluteRate, playback_ip->speedNumerator, playback_ip->speedDenominator, playback_ip->playSpeedString ));
                ipTrickModeSettings->playSpeedStringDefined = true;
                ipTrickModeSettings->playSpeedString = playback_ip->playSpeedString;
            }
        }
        BDBG_MSG(("%s: rate %d, speedNumerator %d, speedDenominator %d", __FUNCTION__,
                    ipTrickModeSettings->absoluteRate, playback_ip->speedNumerator, playback_ip->speedDenominator));
    }
    else {
        /* playSpeed is specified using the rate variable, which only takes the integer values, so fractional speeds! */
        playback_ip->speedNumerator = ipTrickModeSettings->rate;
        playback_ip->speedDenominator = 1;
        BDBG_MSG(("%s: rate %d, speedNumerator %d, speedDenominator %d", __FUNCTION__,
                    ipTrickModeSettings->rate, playback_ip->speedNumerator, playback_ip->speedDenominator));
    }

    if ( (playback_ip->speedNumerator == playback_ip->speedDenominator && playback_ip->playback_state == B_PlaybackIpState_ePlaying) ||
        playback_ip->speedNumerator == 0 || playback_ip->speedDenominator == 0) {
        BDBG_WRN(("%s: Ignoring TrickMode command as requested rate is 1x: speedNumerator %d, speedDenominator %d, currentState=%d", __FUNCTION__,
                    playback_ip->speedNumerator, playback_ip->speedDenominator, playback_ip->playback_state));
        return (B_ERROR_SUCCESS);
    }

    /* if trickmode API is in progress, return INCOMPLETE */
    if (playback_ip->apiInProgress)
        return B_ERROR_IN_PROGRESS;

    /* if trickmode is completed, return results to app */
    if (playback_ip->apiCompleted) {
        rc = playback_ip->trickModeStatus;
        playback_ip->apiCompleted = false;
        BDBG_MSG(("%s: previously started trickmode operation completed, playback_ip %p, trickModeStatus %d", __FUNCTION__, playback_ip, rc));
        goto out;
    }

    lock_ip_session(playback_ip);

    playback_ip->trickmodeApiActive = true;
    playback_ip->frameAdvanceApiActive = false;
    playback_ip->playApiActive = false;
    /* Check if app wants to do the trickplay in non-blocking mode. */
    if (ipTrickModeSettings->nonBlockingMode) {
        /* Neither trickmode is in progress nor it is completed, so start trickmode work */
        playback_ip->apiInProgress = true;
        playback_ip->apiCompleted = false;
        /* start the thread to carry out the trickmode/seek function if not already created */
        if (playback_ip->trickModeThread == NULL) {

            /* Be sure to create this event before starting the thread,
             * because the first thing the thread does is to wait on the
             * event. */
            if (BKNI_CreateEvent(&playback_ip->newTrickModeJobEvent)) {
                BDBG_ERR(("%s: Failed to create an event at %d", __FUNCTION__, __LINE__));
                rc = B_ERROR_UNKNOWN;
                goto error;
            }

            playback_ip->trickModeThread = B_Thread_Create("trickModeThread", (B_ThreadFunc)trickModeThread, (void *)playback_ip, NULL/*threadSettings*/);
            if (playback_ip->trickModeThread == NULL) {
                BDBG_ERR(("%s: Failed to create thread for trickmode operation during trickmode", __FUNCTION__));
                rc = B_ERROR_UNKNOWN;
                goto error;
            }
            BDBG_MSG(("%s: Successfully created thread & event for trickmode operations ", __FUNCTION__));
        }
        /* save the settings so that thread function can use it */
        playback_ip->ipTrickModeSettings = *ipTrickModeSettings;
        /* trickModeThread is either already running or we just started it, wake it up to start the seek work */
        BKNI_SetEvent(playback_ip->newTrickModeJobEvent);
        rc = B_ERROR_IN_PROGRESS;
    }
    else {
        /* trickmode function is being called in the non-blocking mode, just call the function to do the trickmode work */
        rc = trickModeTrick_locked(playback_ip, ipTrickModeSettings);
    }

out:
error:
    if (rc != B_ERROR_IN_PROGRESS) {
        unlock_ip_session(playback_ip);
        if (rc == B_ERROR_SUCCESS) {
            B_PlaybackIp_ResetPsiState(playback_ip->pPsiState);
        }
    }
    return rc;
}

B_PlaybackIpError trickModeFrameAdvance_locked(
    B_PlaybackIpHandle playback_ip,
    bool forward
    )
{
    B_PlaybackIpError rc = B_ERROR_UNKNOWN;
    B_PlaybackIpState currentState;
    double timeSeekRangeBegin;
    double timeSeekRangeEnd;
    NEXUS_PlaybackPosition currentPosition;
    bool reOpenedSocket = false;

    /* note: caller has already locked the session */

    BDBG_MSG(("%s: Starting frameAdvance work: forward %s", __FUNCTION__, forward ? "Y" : "N"));

    /* change the state to EnteringTrickMode until we finish the trickmode work */
    currentState = playback_ip->playback_state;
    playback_ip->playback_state = B_PlaybackIpState_eEnteringTrickMode;

    /* Check if we need to re-establish connection w/ the server. */
    if (playback_ip->reOpenSocket == true ||  /* Previous Player operation may have closed the connection (such as Pause w/ Disconnect), */
            forward == false ||               /* Forward is false, meaning app wants us to display a frame in the backward direction. */
            playback_ip->frameRewindCalled )  /* Or, forward is not false & previous operation was Frame Rewind, so we will need to re-connect to go in the forward direction. */
    {
        /* Need to re-connect to the server, determine the exact position that we need to use. */
        if (B_PlaybackIp_HttpGetCurrentPlaybackPosition(playback_ip, &currentPosition) != B_ERROR_SUCCESS) {
            BDBG_WRN(("%s: Failed to determine the current playback position", __FUNCTION__));
            goto error;
        }
        BDBG_MSG(("%s: current position %0.3f", __FUNCTION__, currentPosition/1000.));

        /* Adjust the currentPosition for frame reverse case. */
        if (!forward) {
            unsigned gopDurationInMs = 1000;
            if (playback_ip->psi.psiValid && playback_ip->psi.videoCodec == NEXUS_VideoCodec_eMpeg2) {
                gopDurationInMs = 1000;
                BDBG_MSG(("%s: For MPEG2 frame reverse, assuming GOP duration of %u, adjusted current position %u", __FUNCTION__, gopDurationInMs, currentPosition-gopDurationInMs));
            }
            else {
                currentPosition -= 2000;
                BDBG_MSG(("%s: For non-MPEG2 frame reverse, assuming GOP duration of %u, adjusted current position %u", __FUNCTION__, gopDurationInMs, currentPosition-gopDurationInMs));
            }
            if (currentPosition > gopDurationInMs ) currentPosition -= gopDurationInMs;
            /* Also update the lastPosition to this updated value. */
            playback_ip->lastPosition = currentPosition;
        }

        /* build & send the new HTTP Get request to server so that it will send stream at new time position */
        timeSeekRangeBegin = currentPosition/1000.;
        timeSeekRangeEnd = 0.;
        BDBG_MSG(("%s: reOpenSocket because %s", __FUNCTION__, (playback_ip->reOpenSocket ? "it was closed" : !forward? "we are doing frame reverse":"Previous Operation was Frame Reverse & Now it is Frame Advance" ) ));
        if ((rc = http_do_server_trick_modes_socket_transition(playback_ip, timeSeekRangeBegin, timeSeekRangeEnd, 1 /* play speed */, forward?"1":"-1" /*playSpeedString */)) != B_ERROR_SUCCESS) {
            BDBG_ERR(("%s: Failed to update server about the trick play transition\n", __FUNCTION__));
            goto error;
        }
        playback_ip->reOpenSocket = false;
        reOpenedSocket = true;
    }

    /* Check if we should flush the current playpump & decoder buffers. */
    if (!forward || playback_ip->frameRewindCalled) {
        /* For going back a frame or doing a frame advance from the previous frame reverse, */
        /* we need to flush the AV decoders & Nexus playpump buffers so that we cleanly play a frame from the current position. */
        if (B_PlaybackIp_UtilsFlushAvPipeline(playback_ip)) {
            BDBG_ERR(("%s: ERROR: Failed to flush the AV pipeline\n", __FUNCTION__));
            rc = B_ERROR_UNKNOWN;
            goto error;
        }
        BDBG_MSG(("%s: Flushed Playpump & AV Decoders for Frame Reverse mode!", __FUNCTION__));
    }

    /* For reverse or switching back to forward from reverse case, we will need to tell decoder to move in the reverse direction. */
    if (!forward || playback_ip->frameRewindCalled) {
        B_PlaybackIpTrickModesSettings ipTrickModeSettings =
        {   B_PlaybackIpTrickModesMethod_UsePlaySpeed, 1, false, NEXUS_NORMAL_DECODE_RATE, 1, 0,
            B_PlaybackIpPauseMethod_UseConnectionStalling, 0, false, 0, false
                ,NEXUS_VideoDecoderDecodeMode_eI
                ,false,
            NULL, false, false, B_PlaybackIpAdaptiveStreamingTrickModeMethod_eUseSegmentWithLowestBandwidth
        };
        if (!forward) {
            ipTrickModeSettings.rate = -1; /* normal speed */
            playback_ip->speedNumerator = -1;
        }
        else {
            ipTrickModeSettings.rate = 1;
            playback_ip->speedNumerator = 1;
        }
        playback_ip->speedDenominator = 1;
        if ((rc = updateNexusPlaypumpDecodersState(playback_ip, &ipTrickModeSettings)) != B_ERROR_SUCCESS) {
            BDBG_ERR(("%s: ERROR: failed to update nexus av decoder state during trickmode transition\n", __FUNCTION__));
            rc = B_ERROR_UNKNOWN;
            goto error;
        }
    }

    if (forward) {
        NEXUS_VideoDecoderTrickState videoDecoderTrickSettings;
        BDBG_MSG(("%s: Reset VideoDecoder configuration to correctly play a frame in forward direction", __FUNCTION__));
        if (playback_ip->nexusHandles.videoDecoder)
            NEXUS_VideoDecoder_GetTrickState(playback_ip->nexusHandles.videoDecoder, &videoDecoderTrickSettings);
        else
            NEXUS_SimpleVideoDecoder_GetTrickState(playback_ip->nexusHandles.simpleVideoDecoder, &videoDecoderTrickSettings);
        videoDecoderTrickSettings.rate = 0;
        videoDecoderTrickSettings.stcTrickEnabled = false;
        videoDecoderTrickSettings.decodeMode = NEXUS_VideoDecoderDecodeMode_eAll;
        videoDecoderTrickSettings.tsmEnabled = NEXUS_TsmMode_eDisabled;
        videoDecoderTrickSettings.topFieldOnly = false;
        videoDecoderTrickSettings.hostTrickModesEnabled = false;
        videoDecoderTrickSettings.reverseFields = false;
        videoDecoderTrickSettings.forceStopped = false;
        if (playback_ip->nexusHandles.videoDecoder) {
            if (NEXUS_VideoDecoder_SetTrickState(playback_ip->nexusHandles.videoDecoder, &videoDecoderTrickSettings) != NEXUS_SUCCESS) {
                BDBG_ERR(("%s: NEXUS_VideoDecoder_SetTrickState() failed \n", __FUNCTION__));
                rc = B_ERROR_UNKNOWN;
                goto error;
            }
        }
        else {
            if (NEXUS_SimpleVideoDecoder_SetTrickState(playback_ip->nexusHandles.simpleVideoDecoder, &videoDecoderTrickSettings) != NEXUS_SUCCESS) {
                BDBG_ERR(("%s: NEXUS_SimpleVideoDecoder_SetTrickState() failed \n", __FUNCTION__));
                rc = B_ERROR_UNKNOWN;
                goto error;
            }
        }
        BDBG_MSG(("%s: Successfully configured video decoder!", __FUNCTION__));
    }

    /* Check if we need to feed little bit of data to the decoders. */
    if (NEXUS_Playpump_SetPause(playback_ip->nexusHandles.playpump, false) != NEXUS_SUCCESS) {
        BDBG_ERR(("%s: NEXUS_Playpump_SetPause() failed to un-pause it!", __FUNCTION__));
        rc = B_ERROR_UNKNOWN;
        goto error;
    }
    if (reOpenedSocket) {
        /* If we had re-opened the socket, we sleep a bit here to allow Ip_PlayPump thread to feed some more data from the socket. */
        /* This way decoders can then either do a frame advance or reverse on this stream. */
        playback_ip->playback_state = B_PlaybackIpState_ePlaying;
        unlock_ip_session(playback_ip);
        BKNI_Sleep(1000);
        /* data for 1 frame worth should be fed to decoder by now, so get the control back from Feeder thread */
        lock_ip_session(playback_ip);
        playback_ip->playback_state = B_PlaybackIpState_eEnteringTrickMode;
    }

    /* And finally, tell the video decoder to play 1 frame. */
    if (playback_ip->nexusHandles.videoDecoder) {
        /* advance one frame to show the new seeked position */
        if (NEXUS_VideoDecoder_FrameAdvance(playback_ip->nexusHandles.videoDecoder) != NEXUS_SUCCESS) {
            BDBG_ERR(("%s: ERROR: NEXUS_VideoDecoder_FrameAdvance() Failed \n", __FUNCTION__));
            rc = B_ERROR_UNKNOWN;
            goto error;
        }
        BDBG_MSG(("%s: Displayed the frame from current position NEXUS_VideoDecoder_FrameAdvance()", __FUNCTION__));
    }
    else if (playback_ip->nexusHandles.simpleVideoDecoder) {
        /* advance one frame to show the new seeked position */
        if (NEXUS_SimpleVideoDecoder_FrameAdvance(playback_ip->nexusHandles.simpleVideoDecoder) != NEXUS_SUCCESS) {
            BDBG_ERR(("%s: ERROR: NEXUS_SimpleVideoDecoder_FrameAdvance() Failed \n", __FUNCTION__));
            rc = B_ERROR_UNKNOWN;
            goto error;
        }
        BDBG_MSG(("%s: Displayed the frame from current position NEXUS_SimpleVideoDecoder_FrameAdvance()", __FUNCTION__));
    }
    if (!forward) {
        playback_ip->frameRewindCalled = true;
    }
    else {
        playback_ip->frameRewindCalled = false;
    }
    rc = B_ERROR_SUCCESS;

error:
    if (NEXUS_Playpump_SetPause(playback_ip->nexusHandles.playpump, true) != NEXUS_SUCCESS) {
        BDBG_ERR(("%s: NEXUS_Playpump_SetPause() failed to pause it!", __FUNCTION__));
        rc = B_ERROR_UNKNOWN;
    }
    playback_ip->playback_state = currentState;
    playback_ip->frameAdvanceApiActive = false;
    return rc;
}

B_PlaybackIpError B_PlaybackIp_FrameAdvance(
    B_PlaybackIpHandle playback_ip,
    bool forward,                        /* if true, one frame is played in forward direction, otherwise, one frame is played in the backward direction. */
    bool nonBlockingMode                 /* if set, API is run in the non-blocking mode and its completion is provided via B_PlaybackIpSessionOpenSettings.eventCallback. */
    )
{
    B_PlaybackIpError rc = B_ERROR_UNKNOWN;

    if (!playback_ip) {
        BDBG_ERR(("%s: NULL params playback_ip", __FUNCTION__));
        return B_ERROR_INVALID_PARAMETER;
    }

    if (playback_ip->playback_state != B_PlaybackIpState_ePaused &&  playback_ip->frameAdvanceApiActive == false ) {
        BDBG_ERR(("%s: Can't call FrameAdvance API w/o calling Pause first!", __FUNCTION__));
        return B_ERROR_INVALID_PARAMETER;
    }

    /* if frameAdvance API is in progress, return INCOMPLETE */
    if (playback_ip->apiInProgress)
        return B_ERROR_IN_PROGRESS;

    /* if trickmode is completed, return results to app */
    if (playback_ip->apiCompleted) {
        rc = playback_ip->trickModeStatus;
        playback_ip->apiCompleted = false;
        BDBG_MSG(("%s: previously started frameAdvance operation completed, playback_ip %p, trickModeStatus %d", __FUNCTION__, playback_ip, rc));
        goto out;
    }

    lock_ip_session(playback_ip);
    BDBG_MSG(("%s playback_ip=%p: forward=%s nonBlockingMode=%s", __FUNCTION__, playback_ip, forward?"Y":"N", nonBlockingMode?"Y":"N"));

    playback_ip->frameAdvanceApiActive = true;
    playback_ip->playApiActive = false;
    playback_ip->trickmodeApiActive = false;
    playback_ip->simulatedStcTrickMode = false;
    playback_ip->hwStcTrickMode = false;
    playback_ip->forward = forward;
    /* Check if app wants to do the trickplay in non-blocking mode. */
    if (nonBlockingMode) {
        /* Neither frameAdavance is in progress nor it is completed, so start frameAdavance work */
        playback_ip->apiInProgress = true;
        playback_ip->apiCompleted = false;
        /* start the thread to carry out the frameAdvancetrickmode/seek function if not already created */
        if (playback_ip->trickModeThread == NULL) {

            /* Be sure to create this event before starting the thread,
             * because the first thing the thread does is to wait on the
             * event. */
            if (BKNI_CreateEvent(&playback_ip->newTrickModeJobEvent)) {
                BDBG_ERR(("%s: Failed to create an event at %d", __FUNCTION__, __LINE__));
                rc = B_ERROR_UNKNOWN;
                goto error;
            }

            playback_ip->trickModeThread = B_Thread_Create("trickModeThread", (B_ThreadFunc)trickModeThread, (void *)playback_ip, NULL/*threadSettings*/);
            if (playback_ip->trickModeThread == NULL) {
                BDBG_ERR(("%s: Failed to create thread for trickmode operation during trickmode", __FUNCTION__));
                rc = B_ERROR_UNKNOWN;
                goto error;
            }
            BDBG_MSG(("%s: Successfully created thread & event for trickmode operations ", __FUNCTION__));
        }
        /* trickModeThread is either already running or we just started it, wake it up to start the seek work */
        BKNI_SetEvent(playback_ip->newTrickModeJobEvent);
        rc = B_ERROR_IN_PROGRESS;
    }
    else {
        /* frameAdvance function is being called in the non-blocking mode, just call the function to do the actual work. */
        rc = trickModeFrameAdvance_locked(playback_ip, playback_ip->forward);
    }

out:
error:
    if (rc != B_ERROR_IN_PROGRESS) {
        unlock_ip_session(playback_ip);
        if (rc == B_ERROR_SUCCESS) {
            B_PlaybackIp_ResetPsiState(playback_ip->pPsiState);
        }
    }
    BDBG_MSG(("%s: returning rc =%d", __FUNCTION__, rc));
    return rc;
}

#endif /* LINUX || VxWorks */
