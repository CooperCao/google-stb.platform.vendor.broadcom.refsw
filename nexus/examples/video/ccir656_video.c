/***************************************************************************
*     (c)2004-2012 Broadcom Corporation
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
***************************************************************************/

#include "nexus_platform.h"
#include <stdio.h>
#if NEXUS_HAS_DISPLAY
#include "nexus_display.h"
#include "nexus_video_input.h"
#include "nexus_ccir656_input.h"
#include "nexus_component_output.h"
#include "nexus_composite_output.h"
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#endif
#include "nexus_video_window.h"

#include "bstd.h"
#include "bkni.h"
#include <stdlib.h>

BDBG_MODULE(ccir656_video);

void data_ready(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
}

int main(void)
{
    NEXUS_DisplayHandle display;
    NEXUS_VideoWindowHandle window;
    NEXUS_Ccir656InputHandle ccir656Input;
    NEXUS_Ccir656InputSettings ccir656InputSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_PlatformSettings platformSettings;
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputStatus hdmiStatus;
    NEXUS_DisplaySettings displaySettings;
#endif
    NEXUS_Error rc;
    BKNI_EventHandle event;

    /* Bring up all modules for a platform in a default configuration for this platform */
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);

    BKNI_CreateEvent(&event);

    display = NEXUS_Display_Open(0, NULL);
#if NEXUS_NUM_COMPONENT_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_ComponentOutput_GetConnector(platformConfig.outputs.component[0]));
#endif 
#if NEXUS_NUM_COMPOSITE_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_CompositeOutput_GetConnector(platformConfig.outputs.composite[0]));
#endif
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_HdmiOutput_GetVideoConnector(platformConfig.outputs.hdmi[0]));
    rc = NEXUS_HdmiOutput_GetStatus(platformConfig.outputs.hdmi[0], &hdmiStatus);
    if ( !rc && hdmiStatus.connected )
    {
        /* If current display format is not supported by monitor, switch to monitor's preferred format. 
           If other connected outputs do not support the preferred format, a harmless error will occur. */
        NEXUS_Display_GetSettings(display, &displaySettings);
        if ( !hdmiStatus.videoFormatSupported[displaySettings.format] ) {
            displaySettings.format = hdmiStatus.preferredVideoFormat;
            NEXUS_Display_SetSettings(display, &displaySettings);
		}
    }
#endif

    /* Bring up CCIR656 and connect to the VideoWindow */
    NEXUS_Ccir656Input_GetDefaultSettings(&ccir656InputSettings);
    ccir656InputSettings.vbi.dataReady.callback = data_ready;
    ccir656InputSettings.vbi.dataReady.context = event;
    ccir656InputSettings.vbi.bufferSize = 200;
    ccir656Input = NEXUS_Ccir656Input_Open(0, &ccir656InputSettings);

    window = NEXUS_VideoWindow_Open(display, 0);
    rc = NEXUS_VideoWindow_AddInput(window, NEXUS_Ccir656Input_GetConnector(ccir656Input));
    BDBG_ASSERT(!rc);

    /* read 656 ancillary data.
    See #define TEST_656_VBI in nexus_ccir656_input.c for an artifical internal data feed. */
    while (1) {
        void *buffer;
        unsigned size, size2;
        NEXUS_Ccir656InputVbiHeader vbiHeader;

        if (NEXUS_Ccir656Input_GetBuffer(ccir656Input, &buffer, &size) || !size) {
            /* block until we get a callback */
            BKNI_WaitForEvent(event, 2000);
            continue;
        }

        /* This code illustrates how to parse out the header and payload, handling wrap in either case. */
        if (size > sizeof(vbiHeader)) {
            BKNI_Memcpy(&vbiHeader, buffer, sizeof(vbiHeader));
            if (vbiHeader.payload <= size - sizeof(vbiHeader)) {
                /* we got the whole thing in one */
                NEXUS_Ccir656Input_ReadComplete(ccir656Input, sizeof(vbiHeader) + vbiHeader.payload);
            }
            else {
                /* the payload wrapped */
                NEXUS_Ccir656Input_ReadComplete(ccir656Input, size);
                NEXUS_Ccir656Input_GetBuffer(ccir656Input, &buffer, &size2);
                BDBG_ASSERT(size2 >= sizeof(vbiHeader)+vbiHeader.payload-size);
                NEXUS_Ccir656Input_ReadComplete(ccir656Input, sizeof(vbiHeader)+vbiHeader.payload-size);
            }
        }
        else {
            /* the header wrapped */
            BKNI_Memcpy(&vbiHeader, buffer, size);
            NEXUS_Ccir656Input_ReadComplete(ccir656Input, size);
            NEXUS_Ccir656Input_GetBuffer(ccir656Input, &buffer, &size2);
            BKNI_Memcpy(&((uint8_t*)&vbiHeader)[size], buffer, sizeof(vbiHeader)-size);
            /* complete the rest of this message */
            BDBG_ASSERT(size2 >= sizeof(vbiHeader)+vbiHeader.payload-size);
            NEXUS_Ccir656Input_ReadComplete(ccir656Input, sizeof(vbiHeader)+vbiHeader.payload-size);
        }

        printf("%d bytes of 656 ancillary data\n", vbiHeader.payload);
    }

    return 0;
}
#else
int main(void)
{
    printf("This application is not supported on this platform\n");
    return 0;
}
#endif
