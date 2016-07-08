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
#ifndef MEDIA_PLAYER_H__
#define MEDIA_PLAYER_H__

#include "nexus_types.h"
#include "nexus_playback.h"
#include "dvr_crypto.h"
#include "nxclient.h"
#if B_REFSW_TR69C_SUPPORT
#include "tr69clib.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
NOTE: This API is only example code. It is subject to change and
is not supported as a standard reference software deliverable.
**/

/**
Summary:
settings for media_player_create
**/
typedef struct media_player_create_settings
{
    bool dtcpEnabled; /* if false, dtcp_ip wont be used */
    struct {
        unsigned surfaceClientId;
        unsigned id;
    } window; /* assign window to control position */
    NEXUS_VideoFormat maxFormat; /* Minimum source resolution from decoder. Takes precedence over maxWidth/maxHeight.
                                    Includes interlaced/progressive and frame rate, but doesn't work for sub-SD sizes. */
    unsigned maxWidth, maxHeight; /* Use if maxFormat not available for sub-SD sizes. Assumes p60. */
} media_player_create_settings;

enum media_player_audio_primers
{
    media_player_audio_primers_none,
    media_player_audio_primers_later,
    media_player_audio_primers_immediate
};

/**
Summary:
Player Settings that can be changed before or after starting the
player.
**/

typedef struct media_player_settings
{
    struct{
        char     *language;             /* This is to select a language specific audio track.
                                           Specifies 3-byte language code defining language of the audio service in ISO 639-2code.
                                           Eg: eng for english, por for portugal.Default this will be set to NULL */
        unsigned ac3_service_type;      /* This is to select a ac3(only) service type specific audio track.
                                           For more details please see ATSC A/52:2012 Digital Audio Compression Standard spec section 5.4.2.2.
                                           Defult this will be set to UINT_MAX */
    }audio;
    bool enableDynamicTrackSelection;    /*If true, Player will detect if a track changes in the middle of the stream & re-select tracks based
                                           on the Track Preferences defined above unless App has defined trackSelectionCallback.
                                           Then, Player will only invoke this callback & wait for App to re-select the tracks.
                                           For more details please look into bip_player.h. */
}media_player_settings;

/**
Summary:
settings for media_player_start
**/
typedef struct media_player_start_settings
{
    const char *stream_url; /* required for dvr. if NULL, then live */
    const char *index_url; /* optional */
    unsigned program;

    NEXUS_PlaybackLoopMode loopMode; /* default is eLoop; ePause will play once and stop; ePlay is used for multiprocess timeshifting */
    void (*eof)(void *context);
    void *context; /* context for any callbacks */
    NxClient_VideoWindowType videoWindowType;
    bool smoothResolutionChange;
    bool startPaused;
    bool stcTrick;
    unsigned dqtFrameReverse;
    bool pacing;
    enum {
        source3d_none, /* 2D or force 2D for MVC */
        source3d_lr,
        source3d_ou,
        source3d_auto  /* autodetect */
    } source3d;

    struct {
        NEXUS_SecurityAlgorithm algo; /* eMax means none */
    } decrypt;
    struct {
        unsigned pid; /* override probe */
        NEXUS_VideoCodec codec; /* override probe */
        unsigned fifoSize;
        bool secure;
    } video;
    NEXUS_TransportType transportType; /* override probe */
    struct {
        unsigned pid; /* override probe */
        NEXUS_AudioCodec codec; /* override probe */
        NEXUS_AudioDecoderDolbyDrcMode dolbyDrcMode; /* applied to ac3, ac3+, aac and aac+ as able */
    } audio;

    bool quiet; /* don't print status */
    NxClient_HdcpLevel hdcp;
    NxClient_HdcpVersion hdcp_version;
    enum media_player_audio_primers audio_primers;
    NEXUS_SimpleStcChannelSyncMode sync; /* sync mode for simple stc channel */
    const char  *additional_headers;/* Additional HTTP headers that app wants to include in the outgoing Get Request. Terminate each header with "\0xd\0xa". */
    NEXUS_PlaybackSettings playbackSettings;
    media_player_settings  mediaPlayerSettings;
} media_player_start_settings;


typedef struct media_player *media_player_t;

/**
Summary:
**/
void media_player_get_default_create_settings(
    media_player_create_settings *psettings
    );

/**
Summary:
**/
media_player_t media_player_create(
    const media_player_create_settings *psettings /* optional */
    );

/**
Summary:
**/
void media_player_get_default_start_settings(
    media_player_start_settings *psettings
    );

/**
Summary:
**/
void media_player_get_default_settings(
    media_player_settings *psettings
    );
/**
Start playback
**/
int media_player_start(
    media_player_t handle,
    const media_player_start_settings *psettings
    );

/**
Summary:
**/
void media_player_get_settings(
    media_player_t player,
    media_player_settings *psettings
    );

/**
Summary:
**/
int media_player_set_settings(
    media_player_t player,
    const media_player_settings *psettings
    );

/**
Summary:
**/
int media_player_ac4_status( media_player_t handle, int action );

/**
Summary:
Perform trick mode
**/
int media_player_trick(
    media_player_t handle,
    int rate /* units are NEXUS_NORMAL_DECODE_RATE. for example:
                0 = pause
                NEXUS_NORMAL_DECODE_RATE = normal play
                2*NEXUS_NORMAL_DECODE_RATE = 2x fast forward
                -3*NEXUS_NORMAL_DECODE_RATE = 3x rewind
                */
    );

/**
Summary:
Seek based on time
**/
int media_player_seek(
    media_player_t handle,
    int offset, /* in milliseconds */
    int whence /* SEEK_SET, SEEK_CUR, SEEK_END */
    );

/**
Summary:
Frame advance or reverse
**/
int media_player_frame_advance(
    media_player_t handle,
    bool forward
    );

/**
Summary:
**/
int media_player_get_playback_status( media_player_t handle, NEXUS_PlaybackStatus *pstatus );

/**
Summary:
**/
void media_player_stop( media_player_t handle );

/**
Summary:
**/
void media_player_destroy( media_player_t handle );

/**
Summary:
Create a set of media players for mosaic decode

Each can be started and stopped independently. Creating in a bundle
allows the underlying NxClient_Alloc and _Connect calls to be batched as required.
**/
int media_player_create_mosaics(
    media_player_t *players, /* array of size num_mosaics. will return NULL for players that can't be created */
    unsigned num_mosaics, /* may create less than requested number */
    const media_player_create_settings *psettings /* optional */
    );

void media_player_destroy_mosaics(
    media_player_t *players, /* array of size num_mosaics. */
    unsigned num_mosaics
    );

/* Start audio decode on a mosaic player */
int media_player_switch_audio(
    media_player_t handle
    );

/**
Summary:
**/
NEXUS_SimpleVideoDecoderHandle media_player_get_video_decoder(media_player_t player);

/**
Summary:
**/
#if B_REFSW_TR69C_SUPPORT
int media_player_get_set_tr69c_info(void *context, enum b_tr69c_type type, union b_tr69c_info *info);
#endif

#ifdef __cplusplus
}
#endif

#endif /* MEDIA_PLAYER_H__ */
