/******************************************************************************
 * Copyright (C) 2018 Broadcom.
 * The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to
 * the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied),
 * right to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 * THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 * IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 * THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 * OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *****************************************************************************/
#ifndef PLATFORM_PRIV_H__
#define PLATFORM_PRIV_H__ 1

#include "platform_types.h"
#include "nxclient.h"
#include "nexus_types.h"
#include "nexus_hdmi_types.h"
#include "nexus_hdmi_output_extra.h"
#include "nexus_video_decoder.h"
#include "nexus_simple_video_decoder.h"
#include "nexus_display.h"

#define NUM_CAPTURE_SURFACES 2

typedef struct Platform
{
    PlatformGraphicsHandle gfx;
    PlatformHdmiReceiverHandle rx;
    PlatformInputHandle input;
    PlatformSchedulerHandle schedulers[PLATFORM_SCHEDULER_COUNT];
    NxClient_CallbackThreadSettings callbackThreadSettings;
    struct
    {
        NEXUS_VideoDecoderCapabilities videoCaps;
        PlatformMediaPlayerHandle players[MAX_STREAMS];
        struct
        {
            NEXUS_SurfaceHandle surfaces[NEXUS_SIMPLE_DECODER_MAX_SURFACES];
            NEXUS_SurfaceHandle captures[NUM_CAPTURE_SURFACES];
            unsigned validCaptures;
            unsigned lastValidCaptures;
            struct
            {
                unsigned startTime;
                unsigned now;
                unsigned frames;
            } performance;
            PlatformMediaPlayerHandle player;
        } streams[MAX_STREAMS];
        unsigned maxStreams; /* total number of simul streams supported on this platform (does not care 4+0, 3+1, 1+1, 1+0, etc.) */
        struct
        {
            unsigned maxStreams; /* total number of simultaneous decode output streams supported per decoder (partially determines 4+0, 3+1, 1+1, 1+0, etc.) */
        } video[NEXUS_MAX_VIDEO_DECODERS];
    } media;
    struct
    {
        NEXUS_DisplayCapabilities caps;
        PlatformDisplayHandle handle;
        unsigned maxWindows; /* total number of windows supported for the display (partially determines 4+0, 3+1, 1+1, 1+0, etc.) */
    } display;
    struct
    {
        unsigned main;
        unsigned pip;
    } streamId; /* this is not the display window index, but rather a platform stream index including mosaic streams */
} Platform;

NEXUS_VideoDynamicRangeMode platform_p_output_dynamic_range_to_nexus(PlatformDynamicRange dynrng);
PlatformDynamicRange platform_p_input_dynamic_range_from_nexus(NEXUS_VideoEotf nxEotf, NEXUS_VideoDecoderDynamicRangeMetadataType dynamicMetadataType);
PlatformDynamicRange platform_p_output_dynamic_range_from_nexus(NEXUS_VideoDynamicRangeMode dynrngMode);
NEXUS_MatrixCoefficients platform_p_colorimetry_to_nexus(PlatformColorimetry colorimetry);
PlatformColorimetry platform_p_colorimetry_from_nexus(NEXUS_MatrixCoefficients nxColorimetry);
NEXUS_ColorSpace platform_p_color_space_and_sampling_to_nexus(PlatformColorSpace colorSpace, int colorSampling);
PlatformColorSpace platform_p_color_space_from_nexus(NEXUS_ColorSpace nxColorSpace);
int platform_p_color_sampling_from_nexus(NEXUS_ColorSpace nxColorSpace);
unsigned platform_p_frame_rate_from_nexus(NEXUS_VideoFrameRate frameRate);
bool platform_p_drop_frame_from_nexus(NEXUS_VideoFrameRate frameRate);
void platform_p_picture_format_from_nexus(NEXUS_VideoFormat format, PlatformPictureFormat * pFormat);
NEXUS_VideoFormat platform_p_picture_format_to_nexus(const PlatformPictureFormat * pFormat);
void platform_p_aspect_ratio_from_nexus(PlatformAspectRatio * pAr, NEXUS_DisplayAspectRatio ar, unsigned x, unsigned y);
NEXUS_DisplayAspectRatio platform_p_aspect_ratio_to_nexus(const PlatformAspectRatio * pAr, unsigned * pX, unsigned * pY);
PlatformRenderingPriority platform_p_rendering_priority_from_nexus(NEXUS_DisplayPriority renderingPriority);
NEXUS_DisplayPriority platform_p_rendering_priority_to_nexus(PlatformRenderingPriority renderingPriority);

void platform_p_hotplug_handler(void * context, int param);
void platform_hdmi_receiver_p_hotplug_handler(PlatformHdmiReceiverHandle rx);

bool platform_display_p_is_dynamic_range_supported(PlatformDisplayHandle display, PlatformDynamicRange dynrng);
void platform_media_player_p_capture_video(PlatformMediaPlayerHandle player);
void platform_media_player_p_recycle_video(PlatformMediaPlayerHandle player);
unsigned platform_p_get_time(void);

#endif /* PLATFORM_PRIV_H__ */
