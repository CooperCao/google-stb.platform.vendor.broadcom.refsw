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
 ******************************************************************************/
#ifndef NXSERVERLIB_DYNRNG_IMPL_H__
#define NXSERVERLIB_DYNRNG_IMPL_H__

#include "nxserverlib.h"
#include "nexus_types.h"
/*#include "nexus_platform.h"*/
/*#include "nexus_platform_server.h"*/
/*#include "nexus_core_compat.h"*/
#if NEXUS_HAS_VIDEO_DECODER
#include "nexus_video_decoder_types.h"
#endif
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#endif
/*#include "nexus_core_utils.h"*/
/*#include "namevalue.h"*/
#include "nexus_types.h"
#include "nxclient.h"

#include <stdlib.h>
#include <string.h>

#if NEXUS_HAS_HDMI_OUTPUT
enum b_video_dynrng_source {
    b_video_dynrng_source_input,
    b_video_dynrng_source_user,
    b_video_dynrng_source_max
};

struct b_video_dynrng_selector
{
    enum b_video_dynrng_source eotf;
    struct
    {
        enum b_video_dynrng_source max;
        enum b_video_dynrng_source maxFrameAverage;
    } cll;
    struct
    {
        struct
        {
            enum b_video_dynrng_source red;
            enum b_video_dynrng_source green;
            enum b_video_dynrng_source blue;
        } primaries;
        enum b_video_dynrng_source whitePoint;
        struct
        {
            enum b_video_dynrng_source max;
            enum b_video_dynrng_source min;
        } luminance;
    } mdcv;
};

struct b_video_dynrng
{
    struct b_session * session;
    struct b_video_dynrng_selector selector;
    bool eotfValid;
    bool smdValid;
    NEXUS_HdmiDynamicRangeMasteringInfoFrame input;
#if NEXUS_HAS_VIDEO_DECODER
    NEXUS_VideoDecoderHandle mainDecoder;
    NEXUS_VideoDecoderDynamicRangeMetadataType dynamicMetadataType;
#endif
    NEXUS_VideoDynamicRangeMode dynamicRangeMode;
    NEXUS_HdmiOutputExtraStatus extraStatus;
    NEXUS_VideoEotf lastInputEotf;
};

bool nxserverlib_dynrng_p_is_dolby_vision_active(struct b_session *session);
int nxserverlib_dynrng_p_session_initialized(struct b_session * session);
int nxserverlib_dynrng_p_video_decoder_acquired(struct b_video_dynrng *dynrng, NEXUS_VideoDecoderHandle handle, NxClient_VideoWindowType type);
void nxserverlib_dynrng_p_video_decoder_released(struct b_video_dynrng *dynrng, NEXUS_VideoDecoderHandle handle);
void nxserverlib_dynrng_p_hotplug_callback_locked(struct b_video_dynrng *dynrng);
void nxserverlib_dyrnng_p_get_default_settings(NxClient_DisplaySettings * pSettings);
NEXUS_Error nxserverlib_dynrng_p_set_drmif(struct b_video_dynrng *dynrng, const NxClient_DisplaySettings *pSettings);
NEXUS_Error nxserverlib_dynrng_p_set_mode(struct b_video_dynrng *dynrng, const NxClient_DisplaySettings *pSettings);
#endif /* NEXUS_HAS_HDMI_OUTPUT || NEXUS_HAS_DISPLAY */

#endif /* NXSERVERLIB_DYNRNG_IMPL_H__ */
