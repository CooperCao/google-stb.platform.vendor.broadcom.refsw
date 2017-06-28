/******************************************************************************
 * Broadcom Proprietary and Confidential. (c) 2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 *****************************************************************************/
#ifndef PLATFORM_PRIV_H__
#define PLATFORM_PRIV_H__ 1

#include "platform_types.h"
#include "nxclient.h"
#include "nexus_types.h"
#include "nexus_hdmi_types.h"
#include "nexus_hdmi_output_extra.h"
#include "nexus_video_decoder.h"

typedef struct Platform
{
    PlatformGraphicsHandle gfx;
    PlatformMediaPlayerHandle player;
    PlatformDisplayHandle display;
    PlatformReceiverHandle rx;
    PlatformInputHandle input;
    PlatformSchedulerHandle scheduler;
    NxClient_CallbackThreadSettings callbackThreadSettings;
    NEXUS_VideoDecoderCapabilities videoCaps;
} Platform;

void platform_p_output_dynamic_range_to_nexus(PlatformDynamicRange dynrng, NEXUS_VideoEotf * pEotf, NEXUS_HdmiOutputDolbyVisionMode * pDolbyVision);
PlatformDynamicRange platform_p_input_dynamic_range_from_nexus(NEXUS_VideoEotf nxEotf, NEXUS_VideoDecoderDynamicRangeMetadataType dynamicMetadataType);
PlatformDynamicRange platform_p_output_dynamic_range_from_nexus(NEXUS_VideoEotf nxEotf, NEXUS_HdmiOutputDolbyVisionMode dolbyVision);
NEXUS_MatrixCoefficients platform_p_colorimetry_to_nexus(PlatformColorimetry colorimetry);
PlatformColorimetry platform_p_colorimetry_from_nexus(NEXUS_MatrixCoefficients nxColorimetry);
NEXUS_ColorSpace platform_p_color_space_to_nexus(PlatformColorSpace colorSpace);
PlatformColorSpace platform_p_color_space_from_nexus(NEXUS_ColorSpace nxColorSpace);
unsigned platform_p_frame_rate_from_nexus(NEXUS_VideoFrameRate frameRate);
void platform_p_hotplug_handler(void * context, int param);
void platform_receiver_p_hotplug_handler(PlatformReceiverHandle rx);
bool platform_display_p_is_dolby_vision_supported(PlatformDisplayHandle display);

#endif /* PLATFORM_PRIV_H__ */
