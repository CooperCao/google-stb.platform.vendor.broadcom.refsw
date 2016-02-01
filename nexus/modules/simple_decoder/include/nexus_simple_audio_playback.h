/***************************************************************************
 *     (c)2010-2012 Broadcom Corporation
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
#ifndef NEXUS_SIMPLE_AUDIO_PLAYBACK_H__
#define NEXUS_SIMPLE_AUDIO_PLAYBACK_H__

#include "nexus_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
Summary:
**/
typedef struct NEXUS_SimpleAudioPlayback *NEXUS_SimpleAudioPlaybackHandle;

/**
Summary:
**/
NEXUS_SimpleAudioPlaybackHandle NEXUS_SimpleAudioPlayback_Acquire( /* attr{release=NEXUS_SimpleAudioPlayback_Release}  */
    unsigned index
    );

/**
Summary:
**/
void NEXUS_SimpleAudioPlayback_Release(
    NEXUS_SimpleAudioPlaybackHandle handle
    );

/**
Summary:
**/
typedef struct NEXUS_SimpleAudioPlaybackStartSettings
{
    unsigned sampleRate;                /* In Hz.  Pass 0 to use the value in NEXUS_AudioPlaybackSettings.sampleRate instead. */
    unsigned bitsPerSample;             /* Currently supports 8 and 16. */
    size_t startThreshold;              /* Starting threshold in bytes.  If set, the hardware will wait until the number of bytes
                                           specified has been buffered before starting to consume the data. */
    bool stereo;                        /* If true, data will be treated as stereo data.  If false, data will be treated as mono. */
    bool signedData;                    /* If true, data will be treated as signed.  If false, data will be treated as unsigned. */
    bool loopAround;                    /* If true, the buffer will loop around when empty instead of outputting zero samples. */
    bool compressed;                    /* If true, IEC-61937 samples will be passed instead of PCM samples.  bitsPerSample must be 16. */
    NEXUS_CallbackDesc dataCallback;    /* Callback when space becomes available. User should call NEXUS_AudioPlayback_GetBuffer.
                                           You will not receive another callback until NEXUS_AudioPlayback_GetBuffer is called. */
    NEXUS_Timebase timebase;            /* the DPCR channel that drives the outputs */
} NEXUS_SimpleAudioPlaybackStartSettings;

/**
Summary:
**/
void NEXUS_SimpleAudioPlayback_GetDefaultStartSettings(
    NEXUS_SimpleAudioPlaybackStartSettings *pSettings /* [out] */
    );

/**
Summary:
**/
NEXUS_Error NEXUS_SimpleAudioPlayback_Start(
    NEXUS_SimpleAudioPlaybackHandle handle,
    const NEXUS_SimpleAudioPlaybackStartSettings *pSettings
    );

/**
Summary:
**/
void NEXUS_SimpleAudioPlayback_Stop(
    NEXUS_SimpleAudioPlaybackHandle handle
    );

/**
Summary:
Run-time settings

Description:
Changing some of these settings after start may require a stop/start. Nexus will handle this automatically.

pcmOutput will result in an internal restart.
**/
typedef struct NEXUS_SimpleAudioPlaybackSettings
{
    int32_t leftVolume;         /* Linear volume level of left channel output */
    int32_t rightVolume;        /* Linear volume level of right channel output */
    bool muted;
    unsigned sampleRate;        /* In Hz.  This value is ignored unless
                                   NEXUS_AudioPlaybackStartSettings.sampleRate is
                                   set to 0. */

    unsigned contentReferenceLevel;/* Specify the reference level of the content to
                                      be fed to this playback channel. This level
                                      describes the peak level expected to be played.
                                      Valid values are 0-31, correlating to -dB peak level
                                      Default level is 20. */

} NEXUS_SimpleAudioPlaybackSettings;

/**
Summary:
**/
void NEXUS_SimpleAudioPlayback_GetSettings(
    NEXUS_SimpleAudioPlaybackHandle handle,
    NEXUS_SimpleAudioPlaybackSettings *pSettings /* [out] */
    );

/**
Summary:
**/
NEXUS_Error NEXUS_SimpleAudioPlayback_SetSettings(
    NEXUS_SimpleAudioPlaybackHandle handle,
    const NEXUS_SimpleAudioPlaybackSettings *pSettings
    );

/**
Summary:
**/typedef struct NEXUS_SimpleAudioPlaybackStatus
{
    bool started;
    size_t queuedBytes;
    size_t fifoSize;
    size_t playedBytes;
} NEXUS_SimpleAudioPlaybackStatus;

/**
Summary:
**/
NEXUS_Error NEXUS_SimpleAudioPlayback_GetStatus(
    NEXUS_SimpleAudioPlaybackHandle handle,
    NEXUS_SimpleAudioPlaybackStatus *pStatus   /* [out] Note that the regular NEXUS_AudioPlayback structure is used */
    );

/**
Summary:
**/
NEXUS_Error NEXUS_SimpleAudioPlayback_GetBuffer(
    NEXUS_SimpleAudioPlaybackHandle handle,
    void **pBuffer, /* [out] attr{memory=cached} pointer to memory mapped region that is ready for playback data */
    size_t *pSize   /* [out] total number of writeable, contiguous bytes which buffer is pointing to */
    );

/**
Summary:
**/
NEXUS_Error NEXUS_SimpleAudioPlayback_WriteComplete(
    NEXUS_SimpleAudioPlaybackHandle handle,
    size_t amountWritten            /* The number of bytes written to the buffer */
    );

/**
Summary:
**/
void NEXUS_SimpleAudioPlayback_Flush(
    NEXUS_SimpleAudioPlaybackHandle handle
    );

#ifdef __cplusplus
}
#endif

#endif
