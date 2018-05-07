/******************************************************************************
 *  Copyright (C) 2018 Broadcom.
 *  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to
 *  the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied),
 *  right to use, or waiver of any kind with respect to the Software, and
 *  Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 *  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 *  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization,
 *  constitutes the valuable trade secrets of Broadcom, and you shall use all
 *  reasonable efforts to protect the confidentiality thereof, and to use this
 *  information only in connection with your use of Broadcom integrated circuit
 *  products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 *  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 *  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 *  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 *  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 *  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 *  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 *  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/

#include "bstd.h"
#include "string.h"
#include "nexus_platform.h"
#include "nexus_display.h"
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#include "cmdline_args.h"

struct hotplug_context {
    NEXUS_HdmiOutputHandle hdmi;
    NEXUS_DisplayHandle display;
    bool ignore_edid;
    bool set_initial_preferred_format;
};

/* registered HDMI hotplug handler -- changes the format (to monitor's default) if monitor doesn't support current format */
static void hotplug_callback(void *pParam, int iParam)
{
    struct hotplug_context *context = pParam;
    NEXUS_HdmiOutputStatus hdmiStatus;
    NEXUS_HdmiOutputHandle hdmi = context->hdmi;
    NEXUS_DisplayHandle display = context->display;
    const char * nexusEnv ;
    NEXUS_Error rc;

    BSTD_UNUSED(iParam);

    rc = NEXUS_HdmiOutput_GetStatus(hdmi, &hdmiStatus);
    BDBG_MSG(("HDMI hotplug event: %s", hdmiStatus.connected?"connected":"not connected"));

    /* the app can choose to switch to the preferred format, but it's not required. */
    if ( !rc && hdmiStatus.connected && !context->ignore_edid)
    {
        NEXUS_DisplaySettings displaySettings;
        bool change;

        NEXUS_Display_GetSettings(display, &displaySettings);
        if (context->set_initial_preferred_format) {
            context->set_initial_preferred_format = false;
            change = true;
        }
        else if ( !hdmiStatus.videoFormatSupported[displaySettings.format]) {
            BDBG_WRN(("Current format (%d) not supported by attached monitor; Use preferred format (%d)",
                displaySettings.format, hdmiStatus.preferredVideoFormat)) ;
            change = true;
        }
        else {
            change = false;
        }

        if (change) {
            NEXUS_HdmiOutputSettings hdmiSettings;

            NEXUS_HdmiOutput_GetSettings(hdmi, &hdmiSettings);
            hdmiSettings.colorSpace = NEXUS_ColorSpace_eAuto;
            hdmiSettings.colorDepth = 0;
            rc = NEXUS_HdmiOutput_SetSettings(hdmi, &hdmiSettings);
            if (rc) BERR_TRACE(rc); /* continue */

            /* current display format is not supported by attached Rx */
            /* use the preferred format unless hdmiformatoverride is set */
            displaySettings.format = hdmiStatus.preferredVideoFormat;
            nexusEnv = NEXUS_GetEnv("hdmiformatoverride") ;
            if (nexusEnv) {
                if ((nexusEnv[0] == 'y') || (nexusEnv[0] == 'Y')) {
                    BDBG_WRN(("hdmiformatoverride will FORCE format (%d) even though not supported",
                        displaySettings.format)) ;
                }
            }

            if (hdmiStatus.preferredVideoFormat >= NEXUS_VideoFormat_e480p) {
                BDBG_WRN(("Warning: This format may disable composite output!"));
            }
            NEXUS_Display_SetSettings(display, &displaySettings);
        }
    }
}
#endif
