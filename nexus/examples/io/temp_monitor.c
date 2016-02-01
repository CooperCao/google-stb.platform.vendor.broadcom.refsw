/******************************************************************************
 *    (c)2008-2012 Broadcom Corporation
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
#include "nexus_platform.h"
#include <stdio.h>
#if NEXUS_HAS_TEMP_MONITOR
#include "nexus_temp_monitor.h"

#ifndef NEXUS_NUM_TEMP_MONITORS
#define NEXUS_NUM_TEMP_MONITORS 2
#endif

static void data_ready(void *context, int param)
{
    int rc;
    NEXUS_TempMonitorStatus status;
    NEXUS_TempMonitorHandle tempMonitor = context;

    rc = NEXUS_TempMonitor_GetStatus(tempMonitor, &status);
    BDBG_ASSERT(!rc);
    if (!status.valid) {
        printf("temp_monitor %d: status not valid\n", param);
    }
    else {
        printf("temp_monitor %d: %#x,%#x,%#x; %d degrees C\n",
            param,
            status.top,
            status.middle,
            status.bottom,
            status.onChipTemperature);
    }
}

int main(void)
{
    unsigned i;
    NEXUS_Error rc;
    NEXUS_TempMonitorHandle tempMonitor[NEXUS_NUM_TEMP_MONITORS];

    rc = NEXUS_Platform_Init(NULL);
    if (rc) return -1;

    for (i=0;i<NEXUS_NUM_TEMP_MONITORS;i++) {
        NEXUS_TempMonitorSettings settings;
        
        printf("opening temp monitor %d\n", i);
        tempMonitor[i] = NEXUS_TempMonitor_Open(i, NULL);
        BDBG_ASSERT(tempMonitor[i]);

        NEXUS_TempMonitor_GetSettings(tempMonitor[i], &settings);
        settings.enabled = true;
        settings.dataReady.callback = data_ready;
        settings.dataReady.context = tempMonitor[i];
        settings.dataReady.param = i;
        settings.measureInterval = 2000; /* 2 seconds */
        rc = NEXUS_TempMonitor_SetSettings(tempMonitor[i], &settings);
        BDBG_ASSERT(!rc);
    }
    
    printf("press ENTER to exit\n");
    getchar();

    for (i=0;i<NEXUS_NUM_TEMP_MONITORS;i++) {
        NEXUS_TempMonitor_Close(tempMonitor[i]);
    }

    NEXUS_Platform_Uninit();
    return 0;
}
#else
int main(void) {printf("no temp_monitor support\n");return 0;}
#endif
