/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#include "nexus_platform_client.h"
#include "nxclient.h"
#include "namevalue.h"
#include "nxapps_cmdline.h"
#include "platform.h"
#include "platform_priv.h"
#include "bkni.h"
#include "bdbg.h"
#include "platform_hdmi_receiver_priv.h"

BDBG_MODULE(platform_hdmi_receiver);

void platform_hdmi_receiver_p_hotplug_handler(PlatformHdmiReceiverHandle rx)
{
    BDBG_ASSERT(rx);

    rx->connected = platform_display_hdmi_is_connected(rx->platform->display.handle);
#if NEXUS_HAS_HDMI_OUTPUT
    if (rx->connected)
    {
        NEXUS_HdmiOutputHandle hdmiOutput;
        BDBG_MSG(("receiver generated hotplug; re-reading hdrdb"));
        hdmiOutput = NEXUS_HdmiOutput_Open(NEXUS_ALIAS_ID + 0, NULL);
        if (hdmiOutput) {
            NEXUS_Error rc = NEXUS_SUCCESS;
            rc = NEXUS_HdmiOutput_GetEdidData(hdmiOutput, &rx->edid);
            NEXUS_HdmiOutput_Close(hdmiOutput);
            if (!rc) rx->edid.valid = true;

            if (rx->hotplug.callback)
            {
                rx->hotplug.callback(rx->hotplug.context, 0);
            }
        }
        else
        {
            BDBG_WRN(("Could not open HDMI alias; app will not function properly"));
        }
    }
#endif
}

void platform_hdmi_receiver_close(PlatformHdmiReceiverHandle rx)
{
    if (!rx) return;
    rx->platform->rx = NULL;
    BKNI_Free(rx);
}

PlatformHdmiReceiverHandle platform_hdmi_receiver_open(PlatformHandle platform, PlatformCallback callback, void * context)
{
    PlatformHdmiReceiverHandle rx;
    rx = BKNI_Malloc(sizeof(*rx));
    BDBG_ASSERT(rx);
    BKNI_Memset(rx, 0, sizeof(*rx));
    platform->rx = rx;
    rx->platform = platform;
    rx->hotplug.callback = callback;
    rx->hotplug.context = context;
    return rx;
}

void platform_hdmi_receiver_start(PlatformHdmiReceiverHandle rx)
{
    BDBG_ASSERT(rx);
    platform_hdmi_receiver_p_hotplug_handler(rx);
}

const PlatformHdmiReceiverModel * platform_hdmi_receiver_supports_picture(PlatformHdmiReceiverHandle rx, const PlatformPictureInfo * pInfo)
{
    BDBG_ASSERT(rx);
#if NEXUS_HAS_HDMI_OUTPUT
    rx->model.dynrng = platform_hdmi_receiver_supports_dynamic_range(rx, pInfo->dynrng);
#else
    rx->model.dynrng = PlatformCapability_eUnknown;
#endif
    return &rx->model;
}

PlatformCapability platform_hdmi_receiver_supports_dynamic_range(PlatformHdmiReceiverHandle rx, PlatformDynamicRange dynrng)
{
#if NEXUS_HAS_HDMI_OUTPUT
    BDBG_ASSERT(rx);
    if (rx->edid.valid)
    {
        NEXUS_VideoEotf eotf;
        NEXUS_HdmiOutputDolbyVisionMode dolbyVision;
        platform_p_output_dynamic_range_to_nexus(dynrng, &eotf, &dolbyVision);
        if ((dynrng == PlatformDynamicRange_eDolbyVision && platform_display_p_is_dolby_vision_supported(rx->platform->display.handle))
            || (rx->edid.hdrdb.valid && rx->edid.hdrdb.eotfSupported[eotf]))
        {
            return PlatformCapability_eSupported;
        }
        else
        {
            return PlatformCapability_eUnsupported;
        }
    }
    else
    {
        return PlatformCapability_eUnknown;
    }
#else
    return PlatformCapability_eUnsupported;
#endif
}
