/******************************************************************************
 *    (c)2008-2009 Broadcom Corporation
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

#include "nexus_frontend.h"
#include "nexus_platform.h"
#include "nexus_frontend_customer_extension.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void lock_callback(void *context, int param)
{
    NEXUS_FrontendHandle frontend = (NEXUS_FrontendHandle)context;
    NEXUS_FrontendSatelliteStatus status;
    NEXUS_FrontendDiseqcStatus disqecStatus;

    BSTD_UNUSED(param);

    fprintf(stderr, "Frontend(%p) - lock callback\n", (void*)frontend);

    NEXUS_Frontend_GetSatelliteStatus(frontend, &status);
    fprintf(stderr, "  demodLocked = %d\n", status.demodLocked);
    NEXUS_Frontend_GetDiseqcStatus(frontend, &disqecStatus);
    fprintf(stderr, "  diseqc tone = %d, voltage = %d\n", disqecStatus.toneEnabled, disqecStatus.voltage);
}

static void data_ready(void *context, int param)
{
    BSTD_UNUSED(context);
    BSTD_UNUSED(param);
    printf("FTM data ready\n");
}

int main(void)
{
    NEXUS_FrontendHandle frontend=NULL;
    NEXUS_FrontendSatelliteSettings satSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_Error rc;
    NEXUS_FrontendFtmSettings ftmSettings;
    uint8_t buf[256];
    size_t n;
    unsigned char ldpc_scrambling_buf[16];
    NEXUS_FrontendCapabilities capabilities;

    NEXUS_Platform_Init(NULL);
    NEXUS_Platform_GetConfiguration(&platformConfig);

    frontend = platformConfig.frontend[0];

    NEXUS_Frontend_GetCapabilities(platformConfig.frontend[0], &capabilities);
    BDBG_ASSERT(capabilities.satellite);
    NEXUS_Frontend_GetCapabilities(platformConfig.frontend[1], &capabilities);
    BDBG_ASSERT(capabilities.satellite);

    NEXUS_Frontend_GetDefaultSatelliteSettings(&satSettings);
    satSettings.frequency = 1119000000;
    satSettings.mode = NEXUS_FrontendSatelliteMode_eDvb;
    satSettings.lockCallback.callback = lock_callback;
    satSettings.lockCallback.context = frontend;

    rc = NEXUS_Frontend_TuneSatellite(frontend, &satSettings);
    BDBG_ASSERT(!rc);

    NEXUS_Frontend_DirecTV_GetFTMSettings(frontend, &ftmSettings);
    ftmSettings.enabled = true;
    ftmSettings.dataReady.callback = data_ready;
    ftmSettings.dataReady.context = frontend;
    rc = NEXUS_Frontend_DirecTV_SetFTMSettings(frontend, &ftmSettings);
    BDBG_ASSERT(!rc);

    /* open 2nd channel, just so we test the FTM event support */
    NEXUS_Frontend_DirecTV_GetFTMSettings(platformConfig.frontend[1], &ftmSettings);
    ftmSettings.enabled = true;
    ftmSettings.dataReady.callback = data_ready;
    ftmSettings.dataReady.context = platformConfig.frontend[1];
    rc = NEXUS_Frontend_DirecTV_SetFTMSettings(platformConfig.frontend[1], &ftmSettings);
    BDBG_ASSERT(!rc);

    /* TODO: add real FTM code */
    rc = NEXUS_Frontend_DirecTV_SendFTMMessage(frontend, buf, 1);
    BDBG_ASSERT(!rc);

    rc = NEXUS_Frontend_DirecTV_ReceiveFTMMessage(frontend, buf, 256, &n);
    BDBG_ASSERT(!rc);

    rc = NEXUS_Frontend_DirecTV_ResetFTM(frontend);
    BDBG_ASSERT(!rc);

    NEXUS_Frontend_DirecTV_GetFTMSettings(frontend, &ftmSettings);
    ftmSettings.enabled = false;
    rc = NEXUS_Frontend_DirecTV_SetFTMSettings(frontend, &ftmSettings);
    BDBG_ASSERT(!rc);

    rc = NEXUS_Frontend_DirecTV_SetLdpcScramblingSequence(frontend, ldpc_scrambling_buf, 16);
    BDBG_ASSERT(!rc);

    rc = NEXUS_Frontend_DirecTV_SetFskChannel(frontend, NEXUS_DirecTV_FskChannelConfig_eCh0Tx_Ch0Rx);
    BDBG_ASSERT(!rc);

    /* this will close the extension */
    NEXUS_Platform_Uninit();

    return 0;
}
