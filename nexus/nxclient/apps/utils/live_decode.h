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
#ifndef LIVE_DECODE_H__
#define LIVE_DECODE_H__

#include "nxclient.h"
#include "nexus_types.h"
#include "nexus_parser_band.h"
#include "nexus_simple_video_decoder.h"
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
settings for live_decode_create
**/
typedef struct live_decode_create_settings
{
    bool primed; /* if false, no priming will take place. */
    struct {
        NEXUS_VideoDecoder_ChannelChangeMode channelChangeMode;
    } video;
    struct {
        unsigned surfaceClientId;
        unsigned id;
    } window; /* assign window to control position */
    NEXUS_SimpleStcChannelSyncMode sync;
} live_decode_create_settings;

typedef struct live_decode *live_decode_t;
typedef struct live_decode_channel *live_decode_channel_t;

void live_decode_get_default_create_settings( 
    live_decode_create_settings *psettings 
    );

live_decode_t live_decode_create( 
    const live_decode_create_settings *psettings /* optional */
    );

void live_decode_destroy( live_decode_t handle );

live_decode_channel_t live_decode_create_channel( 
    live_decode_t handle
    );
    
void live_decode_destroy_channel( 
    live_decode_channel_t handle
    );
    
/**
Summary:
settings for live_decode_start
**/
typedef struct live_decode_start_settings
{
    NEXUS_ParserBand parserBand;
    struct {
        unsigned pid;
        NEXUS_VideoCodec codec;
        NEXUS_VideoFormat maxFormat; /* Minimum source resolution from decoder. Takes precedence over maxWidth/maxHeight.
                                        Includes interlaced/progressive and frame rate, but doesn't work for sub-SD sizes. */
        unsigned maxWidth, maxHeight; /* Use if maxFormat not available for sub-SD sizes. Assumes p60. */
        NxClient_VideoWindowType videoWindowType;
        bool smoothResolutionChange;
        unsigned prerollRate;
        unsigned colorDepth;
    } video;
    struct {
        unsigned pid;
        NEXUS_AudioCodec codec;
    } audio;
    unsigned pcr_pid;
} live_decode_start_settings;

void live_decode_get_default_start_settings(
    live_decode_start_settings *psettings
    );

/* start priming a channel */
int live_decode_start_channel( 
    live_decode_channel_t handle,
    const live_decode_start_settings *psettings 
    );
    
/* switch from priming to decoding */
int live_decode_activate( 
    live_decode_channel_t channel
    );

/* stop decoding or priming this channel */
void live_decode_stop_channel( 
    live_decode_channel_t handle 
    );

/**
Summary:
**/
NEXUS_SimpleVideoDecoderHandle live_decode_get_video_decoder(live_decode_channel_t channel);

/**
Summary:
**/
#if B_REFSW_TR69C_SUPPORT
int live_decode_get_set_tr69c_info(void *context, enum b_tr69c_type type, union b_tr69c_info *info);
#endif

#ifdef __cplusplus
}
#endif

#endif /* LIVE_DECODE_H__ */
