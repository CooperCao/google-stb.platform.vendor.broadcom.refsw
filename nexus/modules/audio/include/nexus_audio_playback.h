/***************************************************************************
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
*
* API Description:
*   API name: AudioPlayback
*    Specific APIs related to PCM audio playback.  This supports playback
*    of data from memory.  It can be routed either to a mixer or directly
*    to output devices.
*
***************************************************************************/
#ifndef NEXUS_AUDIO_PLAYBACK_H__
#define NEXUS_AUDIO_PLAYBACK_H__

#include "nexus_types.h"
#include "nexus_audio_types.h"
#include "nexus_timebase.h"

/*=************************************
Interface: AudioPlayback

Header file: nexus_audio_playback.h

Module: Audio

Description: Playback audio PCM data

**************************************/

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
Summary:
Handle for audio playback
***************************************************************************/
typedef struct NEXUS_AudioPlayback *NEXUS_AudioPlaybackHandle;

/***************************************************************************
Summary:
Audio Playback Open Settings

Description:
Audio Playback Open Settings

See Also:
NEXUS_AudioPlaybackSettings
***************************************************************************/
typedef struct NEXUS_AudioPlaybackOpenSettings
{
    size_t fifoSize;        /* FIFO size in bytes */
    size_t threshold;       /* FIFO data callback threshold in bytes.  When the amount of data
                               remaining in the FIFO drops below this level, the dataCallback
                               routine provided in NEXUS_AudioPlaybackSettings will be called.
                               If this value is 0, a default threshold percentage will be used. */
    NEXUS_HeapHandle heap;  /* Optional heap for fifo allocation */                               
} NEXUS_AudioPlaybackOpenSettings;

/***************************************************************************
Summary:
Get default settings for an audio playback channel
***************************************************************************/
void NEXUS_AudioPlayback_GetDefaultOpenSettings(
    NEXUS_AudioPlaybackOpenSettings *pSettings      /* [out] default settings */
    );

/***************************************************************************
Summary:
Open an audio playback channel
***************************************************************************/
NEXUS_AudioPlaybackHandle NEXUS_AudioPlayback_Open(     /* attr{destructor=NEXUS_AudioPlayback_Close}  */
    unsigned index,
    const NEXUS_AudioPlaybackOpenSettings *pSettings    /* attr{null_allowed=y} */
    );

/***************************************************************************
Summary:
Close an audio playback channel
***************************************************************************/
void NEXUS_AudioPlayback_Close(
    NEXUS_AudioPlaybackHandle handle
    );

/***************************************************************************
Summary:
Settings of an audio playback channel
***************************************************************************/
typedef struct NEXUS_AudioPlaybackStartSettings
{
    NEXUS_Timebase timebase;
    unsigned sampleRate;                /* In Hz.  Pass 0 to use the value in NEXUS_AudioPlaybackSettings.sampleRate instead. */
    unsigned bitsPerSample;             /* Currently supports 8, 16, and 24. */
    size_t startThreshold;              /* Starting threshold in bytes.  If set, the hardware will wait until the number of bytes
                                           specified has been buffered before starting to consume the data. */
    bool stereo;                        /* If true, data will be treated as stereo data.  If false, data will be treated as mono. */
    bool signedData;                    /* If true, data will be treated as signed.  If false, data will be treated as unsigned. */
    bool loopAround;                    /* If true, the buffer will loop around when empty instead of outputting zero samples. */
    bool compressed;                    /* If true, IEC-61937 samples will be passed instead of PCM samples.  bitsPerSample must be 16. */
    NEXUS_CallbackDesc dataCallback;    /* Callback when space becomes available. User should call NEXUS_AudioPlayback_GetBuffer.
                                           You will not receive another callback until NEXUS_AudioPlayback_GetBuffer is called. */
    NEXUS_EndianMode endian;            /* Endian of the pcm data being fed for playback */
} NEXUS_AudioPlaybackStartSettings;

/***************************************************************************
Summary:
Get default settings for an audio playback channel
***************************************************************************/
void NEXUS_AudioPlayback_GetDefaultStartSettings(
    NEXUS_AudioPlaybackStartSettings *pSettings  /* [out] Default Settings */
    );

/***************************************************************************
Summary:
Start playing data data from an audio playback channel
***************************************************************************/
NEXUS_Error NEXUS_AudioPlayback_Start(
    NEXUS_AudioPlaybackHandle handle,
    const NEXUS_AudioPlaybackStartSettings *pSettings
    );

/***************************************************************************
Summary:
Stop playing data from an audio playback channel.

Description:
This will stop the channel and flush all data from the FIFO.
***************************************************************************/
void NEXUS_AudioPlayback_Stop(
    NEXUS_AudioPlaybackHandle handle
    );

/***************************************************************************
Summary:
Stop playing data from an audio playback channel without flushing.

Description:
This will stop the channel.
***************************************************************************/
NEXUS_Error NEXUS_AudioPlayback_Suspend(
    NEXUS_AudioPlaybackHandle handle
    );

/***************************************************************************
Summary:
Resume playing data from an audio playback channel.

Description:
This will restart the channel.
***************************************************************************/
NEXUS_Error NEXUS_AudioPlayback_Resume(
    NEXUS_AudioPlaybackHandle handle
    );

/***************************************************************************
Summary:
Flush all data from the playback FIFO.

Description:
This will flush all data from the FIFO without stopping the channel.
***************************************************************************/
void NEXUS_AudioPlayback_Flush(
    NEXUS_AudioPlaybackHandle handle
    );

/***************************************************************************
Summary:
Get a pointer and size for the next location in the buffer which can accept data

Description:
NEXUS_AudioPlayback_GetBuffer is non-destructive. You can safely call it multiple times.
***************************************************************************/
NEXUS_Error NEXUS_AudioPlayback_GetBuffer(
    NEXUS_AudioPlaybackHandle handle,
    void **pBuffer, /* [out] attr{memory=cached} pointer to memory mapped region that is ready for playback data */
    size_t *pSize   /* [out] total number of writeable, contiguous bytes which buffer is pointing to */
    );

/***************************************************************************
Summary:
Notify AudioPlayback how much data was added into the buffer.

Description:
You can only call NEXUS_AudioPlayback_WriteComplete once after a
NEXUS_AudioPlayback_GetBuffer call.  After calling it, you must call
NEXUS_AudioPlayback_GetBuffer before adding more data.
***************************************************************************/
NEXUS_Error NEXUS_AudioPlayback_WriteComplete(
    NEXUS_AudioPlaybackHandle handle,
    size_t amountWritten            /* The number of bytes written to the buffer */
    );

/* backward compatibility */
#define NEXUS_AudioPlayback_ReadComplete NEXUS_AudioPlayback_WriteComplete

/***************************************************************************
Summary:
Audio playback status
***************************************************************************/
typedef struct NEXUS_AudioPlaybackStatus
{
    NEXUS_AudioPlaybackStartSettings startSettings;
    bool started;
    size_t queuedBytes;
    size_t fifoSize;
    size_t playedBytes;
} NEXUS_AudioPlaybackStatus;

/***************************************************************************
Summary:
Get current status of the audio playback channel
***************************************************************************/
void NEXUS_AudioPlayback_GetStatus(
    NEXUS_AudioPlaybackHandle handle,
    NEXUS_AudioPlaybackStatus *pStatus      /* [out] Current Status */
    );

/***************************************************************************
Summary:
Audio playback settings
***************************************************************************/
typedef struct NEXUS_AudioPlaybackSettings
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
} NEXUS_AudioPlaybackSettings;

/***************************************************************************
Summary:
Get current setting of the audio playback channel
***************************************************************************/
void NEXUS_AudioPlayback_GetSettings(
    NEXUS_AudioPlaybackHandle handle,
    NEXUS_AudioPlaybackSettings *pSettings  /* [out] Current settings */
    );

/***************************************************************************
Summary:
Set current setting of the audio playback channel
***************************************************************************/
NEXUS_Error NEXUS_AudioPlayback_SetSettings(
    NEXUS_AudioPlaybackHandle handle,
    const NEXUS_AudioPlaybackSettings *pSettings
    );

/***************************************************************************
Summary:
Get an audio connector for use with downstream components.
**************************************************************************/
NEXUS_AudioInputHandle NEXUS_AudioPlayback_GetConnector(
    NEXUS_AudioPlaybackHandle handle
    );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef NEXUS_AUDIO_PLAYBACK_H__ */
