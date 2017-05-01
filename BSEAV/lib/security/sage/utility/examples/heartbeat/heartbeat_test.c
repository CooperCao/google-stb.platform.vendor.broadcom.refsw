/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
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

 ******************************************************************************/

#include "bstd.h"
#include "bkni.h"
#include "utility_platform.h"
#include "utility_ids.h"
#include "heartbeat_tl.h"
#include "nexus_platform.h"
#include "nexus_platform_init.h"
#include <stdio.h>

#ifdef NXCLIENT_SUPPORT
#include "nxclient.h"
#endif

static int SAGE_app_join_nexus(void);
static void SAGE_app_leave_nexus(void);

BDBG_MODULE(heartbeat);

#ifdef NXCLIENT_SUPPORT
static NxClient_AllocResults allocResults;
#endif

int main(void)
{
    BERR_Code rc = BERR_SUCCESS;
    HeartbeatTlSettings dummySettings;

    SAGE_app_join_nexus();

    HeartbeatTl_GetDefaultSettings(&dummySettings);

    rc = HeartbeatTl_Init(&dummySettings);
    if(rc != BERR_SUCCESS){
        BDBG_ERR(("\tError initializing Heartbeat module"));
        return -1;
    }

    rc = HeartbeatTl_ProcessCommand(Heartbeat_CommandId_eTakePulse);
    if (rc != BERR_SUCCESS) {
        BDBG_ERR(("\tError processing Heartbeat command"));
        return -1;
    }
    BDBG_LOG(("Exiting Heartbeat test application\n"));

    HeartbeatTl_Uninit();
    SAGE_app_leave_nexus();
    return 0;
}

static int SAGE_app_join_nexus(void)
{
    int rc = 0;

#ifdef NXCLIENT_SUPPORT
    NxClient_JoinSettings joinSettings;
    NxClient_AllocSettings nxAllocSettings;

    BDBG_LOG(("\tBringing up nxclient\n\n"));
    NxClient_GetDefaultJoinSettings(&joinSettings);
    snprintf(joinSettings.name, NXCLIENT_MAX_NAME, "heartbeat");
    joinSettings.ignoreStandbyRequest = true;

    rc = NxClient_Join(&joinSettings);
    if (rc) return -1;
    NxClient_GetDefaultAllocSettings(&nxAllocSettings);
    rc = NxClient_Alloc(&nxAllocSettings, &allocResults);
    if (rc)
        return BERR_TRACE(rc);
    BDBG_LOG(("\tnxclient has joined successfully\n"));
#else
    NEXUS_PlatformSettings platformSettings;
    BDBG_LOG(("\tBringing up all Nexus modules for platform using default settings\n\n"));

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    if(NEXUS_Platform_Init(&platformSettings) == NEXUS_SUCCESS)
    {
        BDBG_LOG(("\tNexus has initialized successfully\n"));
        rc = 0;
    }
    else
    {
        BDBG_ERR(("\tFailed to bring up Nexus\n"));
        rc = 1;
    }
#endif

    return rc;
}

static void SAGE_app_leave_nexus(void)
{
#ifdef NXCLIENT_SUPPORT
    NxClient_Free(&allocResults);
    NxClient_Uninit();
#else
    NEXUS_Platform_Uninit();
#endif
}
