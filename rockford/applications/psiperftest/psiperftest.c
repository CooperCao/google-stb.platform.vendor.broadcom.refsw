/***************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
 *
 * Module Description:
 *
 **************************************************************************/
/*
 * This app is intended to be used alongside linux 'top' utility to measure CPU
 * overhead of processing PSI messages every 10 ms in both HW and SW filtering
 * modes.  You need to add the line 'NEXUS_USE_SW_FILTER=y' in
 * nexus/modules/transport/transport.inc and then rebuild ('make clean;make') to
 * force nexus to use software-based parsing instead of usual hardware-based
 * parsing (using XPT message filter hardware).  Then compare the results from
 * the 'top' utility of running the app compiled with hardware psi support vs.
 * running the app compiled with software psi support.
 */
#include "nexus_platform.h"
#include "nexus_parser_band.h"
#include "tspsimgr3.h"

#include "bstd.h"
#include "bkni.h"

#include <stdio.h>

BDBG_MODULE(psiperftest);

void scan_callback(void *context)
{
    BKNI_SetEvent((BKNI_EventHandle)context);
}

int main(int argc, char * argv[])
{
    NEXUS_PlatformSettings platformSettings;
    NEXUS_ParserBand parserBand = NEXUS_ParserBand_e0;
    NEXUS_ParserBandSettings parserBandSettings;
    BKNI_EventHandle event;
    tspsimgr_scan_settings scanSettings;
    tspsimgr_scan_results scanResults;
    tspsimgr_t tspsi;
    NEXUS_Error rc;

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    platformSettings.transportModuleSettings.maxDataRate.parserBand[0] = 108000000;
    NEXUS_Platform_Init(&platformSettings);

    NEXUS_ParserBand_GetSettings(parserBand, &parserBandSettings);
    parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
    NEXUS_Platform_GetStreamerInputBand(0, &parserBandSettings.sourceTypeSettings.inputBand);
    NEXUS_ParserBand_SetSettings(parserBand, &parserBandSettings);

    tspsi = tspsimgr_create();

    BKNI_CreateEvent(&event);

    tspsimgr_get_default_start_scan_settings(&scanSettings);
    scanSettings.context = event;
    scanSettings.exclude_ca_pid = true;
    scanSettings.parserBand = parserBand;
    scanSettings.scan_done = scan_callback;

    while (1)
    {
        unsigned i;
        rc = tspsimgr_start_scan(tspsi, &scanSettings);
        if (rc) { BDBG_ERR(("scan failed: %u", rc)); break; }
        rc = BKNI_WaitForEvent(event, 5000); /* wait 5 seconds */
        if (rc) { BDBG_ERR(("WaitForEvent failed: %u", rc)); rc = -1; break; }
        tspsimgr_get_scan_results(tspsi, &scanResults);
        BKNI_Sleep(10);
    }

    tspsimgr_stop_scan(tspsi);
    tspsimgr_destroy(tspsi);
    BKNI_DestroyEvent(event);
    NEXUS_Platform_Uninit();
    return 0;
}
