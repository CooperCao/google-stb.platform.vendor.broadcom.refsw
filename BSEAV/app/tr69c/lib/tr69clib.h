/******************************************************************************
 *    (c)2014 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 *****************************************************************************/
#ifndef __TR69CLIB_H__
#define __TR69CLIB_H__

#include "nexus_types.h"
#include "nexus_video_decoder.h"
#include "nexus_audio_decoder.h"
#include "nexus_video_decoder_types.h"
#include "nexus_audio_decoder_types.h"
#include "nexus_playback.h"
#include "nxclient.h"
#if PLAYBACK_IP_SUPPORT
#include "b_playback_ip_lib.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct b_tr69c *b_tr69c_t;

enum b_tr69c_type
{
    b_tr69c_type_get_playback_ip_status,
    b_tr69c_type_get_playback_status,
    b_tr69c_type_get_video_decoder_status,
    b_tr69c_type_get_video_decoder_start_settings,
    b_tr69c_type_get_audio_decoder_status,
    b_tr69c_type_get_audio_decoder_settings,

    b_tr69c_type_set_video_decoder_mute,
    b_tr69c_type_set_audio_decoder_mute,

    b_tr69c_type_max
};

struct b_tr69c_video_start_settings
{
    NEXUS_VideoCodec codec;
    NxClient_VideoWindowType videoWindowType;
};

union b_tr69c_info
{
#if PLAYBACK_IP_SUPPORT
    B_PlaybackIpStatus playback_ip_status;
#endif
    NEXUS_PlaybackStatus playback_status;
    NEXUS_VideoDecoderStatus video_decoder_status;
    struct b_tr69c_video_start_settings video_start_settings;
    NEXUS_AudioDecoderStatus audio_decoder_status;
    NEXUS_SimpleAudioDecoderSettings audio_decoder_settings;
    bool video_decoder_mute;
    bool audio_decoder_mute;
};

/* return zero if you can get the status. return non-zero if you can't. */
typedef int (*tr69c_callback)(void *context, enum b_tr69c_type type, union b_tr69c_info *info);

b_tr69c_t b_tr69c_init(
    tr69c_callback callback, /* universal callback to get status/settings and set settings */
    void *context /* pass into callback for the caller's use */
    );

void b_tr69c_uninit(
    b_tr69c_t tr69c
    );

#ifdef __cplusplus
}
#endif

#endif /* __TR69CLIB_H__ */
