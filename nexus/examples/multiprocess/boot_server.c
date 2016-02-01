/******************************************************************************
 *    (c)2008-2013 Broadcom Corporation
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
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *****************************************************************************/
#include <stdio.h>
#include "bstd.h"
#include "nexus_types.h"
#include "nexus_platform.h"

BDBG_MODULE(boot);

int main(void)
{
    NEXUS_Error rc;
    NEXUS_PlatformStartServerSettings serverSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_PlatformSettings platformSettings;

    while (1) {
        NEXUS_PlatformStatus platformStatus;
        char versionString[256];

        /* NOTE: can't use KNI or DBG before NEXUS_Platform_Init or after NEXUS_Platform_Uninit. Nexus/magnum stack must be up. */
        fprintf(stderr, "Ready to initialize\n");

        NEXUS_Platform_GetDefaultSettings(&platformSettings);
        platformSettings.openInputs = false;
        platformSettings.openOutputs = false;
        platformSettings.openFrontend = false;

        rc = NEXUS_Platform_Init(&platformSettings);
        if ( rc ) {
            fprintf(stderr, "Unable to initialize nexus: %d\n", rc);
            return -1;
        }
        NEXUS_Platform_GetConfiguration(&platformConfig);

        NEXUS_Platform_GetDefaultStartServerSettings(&serverSettings);
        serverSettings.allowUnauthenticatedClients = true;
        serverSettings.unauthenticatedConfiguration.mode = NEXUS_ClientMode_eProtected;
        serverSettings.unauthenticatedConfiguration.heap[0] = NEXUS_Platform_GetFramebufferHeap(NEXUS_OFFSCREEN_SURFACE); /* default heap for client. VC4 usable, eApplication mapping. */
        serverSettings.unauthenticatedConfiguration.heap[1] = platformConfig.heap[0]; /* for packet blit and playpump */
        rc = NEXUS_Platform_StartServer(&serverSettings);

        rc = NEXUS_Platform_GetStatus(&platformStatus);
        BDBG_ASSERT(!rc);
        NEXUS_Platform_GetReleaseVersion(versionString, 256);

        BDBG_WRN(("Initialization of BCM%04x Nexus, release %s, complete. Press ENTER to uninit.", platformStatus.chipId, versionString));
        getchar();

        NEXUS_Platform_Uninit();

        fprintf(stderr, "Uninit complete. Press ENTER to init again.\n");
        getchar();
    }

    return 0;
}

