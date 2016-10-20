/******************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#include "nxclient.h"
#include "nexus_platform.h"
#include "nexus_component_output.h"
#include "nexus_composite_output.h"
#include "nexus_video_output.h"
#include <stdio.h>
#include <assert.h>
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"

BDBG_MODULE(getvdacstatus);

int main(int argc, char **argv)
{
    NxClient_JoinSettings joinSettings;
    NEXUS_Error rc;
    NEXUS_PlatformConfiguration platformConfig;
    NxClient_DisplaySettings displaySettings;
    NEXUS_VideoOutputHandle hComponent;
    NEXUS_VideoOutputHandle hComposite;
    NEXUS_VideoOutputStatus videoDacStatus;

    BSTD_UNUSED(argc);
    NxClient_GetDefaultJoinSettings(&joinSettings);
    snprintf(joinSettings.name, NXCLIENT_MAX_NAME, "%s", argv[0]);
    joinSettings.mode = NEXUS_ClientMode_eVerified;
    rc = NxClient_Join(&joinSettings);
    if (rc) { printf("Failed to join\n"); return -1; }

    BDBG_WRN(("app requires nxserver is run with -videoDacDetection command line option"));
    NEXUS_Platform_GetConfiguration(&platformConfig);
    hComponent = NEXUS_ComponentOutput_GetConnector(platformConfig.outputs.component[0]);
    hComposite = NEXUS_CompositeOutput_GetConnector(platformConfig.outputs.composite[0]);
    NxClient_GetDisplaySettings(&displaySettings);
    if (hComponent && displaySettings.componentPreferences.enabled)
    {
        NEXUS_VideoOutput_GetStatus(hComponent,&videoDacStatus);
        BDBG_WRN(("Component DAC enabled in SoC"));
        if (videoDacStatus.connectionState == NEXUS_VideoOutputConnectionState_eConnected)
        {
            BDBG_WRN(("Component video cable connected between SoC output and TV"));
        }
        else
        {
            BDBG_WRN(("Component video cable not connected between SoC output and TV"));
        }
    }
    else
    {
        BDBG_WRN(("Component DAC not enabled in SoC"));
    }


    if (hComposite && displaySettings.compositePreferences.enabled)
    {
        NEXUS_VideoOutput_GetStatus(hComposite,&videoDacStatus);
        BDBG_WRN(("Composite DAC enabled in SoC"));
        if (videoDacStatus.connectionState == NEXUS_VideoOutputConnectionState_eConnected)
        {
            BDBG_WRN(("Composite video cable connected between SoC output and TV"));
        }
        else
        {
            BDBG_WRN(("Composite video cable not connected between SoC output and TV"));
        }
    }
    else
    {
        BDBG_WRN(("Composite DAC not enabled in SoC"));
    }

    NxClient_Uninit();

    return 0;
}
