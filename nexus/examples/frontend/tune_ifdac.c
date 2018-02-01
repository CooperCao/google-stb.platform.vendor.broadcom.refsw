/******************************************************************************
 * Copyright (C) 2013-2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#if NEXUS_HAS_FRONTEND
#include "nexus_platform.h"
#include "nexus_core_utils.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

BDBG_MODULE(tune_ifdac);

static void tune_complete_callback(void *context, int param)
{
    BSTD_UNUSED(param);
    BSTD_UNUSED(context);

    BDBG_MSG(("tune_complete_callback received."));
    /*BKNI_SetEvent((BKNI_EventHandle)context);*/
}

static void async_status_ready_callback(void *context, int param)
{
    BSTD_UNUSED(param);

    BDBG_MSG(("async_status_ready_callback called."));
    BKNI_SetEvent((BKNI_EventHandle)context);
}

int main(int argc, char **argv)
{
    NEXUS_PlatformSettings platformSettings;
    NEXUS_Error rc;
    /* default freq & qam mode */
    unsigned freq = 765;
    NEXUS_FrontendQamMode qamMode = NEXUS_FrontendQamMode_e64;
    NEXUS_TunerHandle tunerHandle = NULL;
    NEXUS_TunerTuneSettings tunerTuneSettings;
    NEXUS_TunerSettings tunerSettings;
    NEXUS_TunerStatus tunerStatus;
    NEXUS_TunerAcquireSettings settings;
    BKNI_EventHandle statusEvent;

    /* allow cmdline freq & qam mode for simple test */
    if (argc > 1) {
        freq = atoi(argv[1]);
    }
    if (argc > 2) {
        unsigned mode = atoi(argv[2]);
        switch (mode) {
        case 64: qamMode = NEXUS_FrontendQamMode_e64; break;
        case 256: qamMode = NEXUS_FrontendQamMode_e256; break;
        case 1024: qamMode = NEXUS_FrontendQamMode_e1024; break;
        default: printf("unknown qam mode %d\n", mode); return -1;
        }
    }

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    rc = NEXUS_Platform_Init(&platformSettings);
    if (rc) return -1;

    BKNI_CreateEvent(&statusEvent);

    rc = NEXUS_Platform_InitFrontend();
    if(rc){rc = BERR_TRACE(rc); goto done;}

    NEXUS_Tuner_GetDefaultAcquireSettings(&settings);
    settings.index = 0;
    tunerHandle = NEXUS_Tuner_Acquire(&settings);
    if (!tunerHandle) {
        BDBG_ERR(("Unable to find IFDAC-capable tuner."));
        return -1;
    }

    NEXUS_Tuner_GetSettings(tunerHandle, &tunerSettings);

    tunerSettings.bandwidth = NEXUS_FrontendQamBandwidth_e6Mhz;
    tunerSettings.ifFrequency = 4000000;
    tunerSettings.dacAttenuation = 2*256;

    rc = NEXUS_Tuner_SetSettings(tunerHandle, &tunerSettings);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    NEXUS_Tuner_GetDefaultTuneSettings(NEXUS_TunerMode_eQam, &tunerTuneSettings);
    tunerTuneSettings.mode = NEXUS_TunerMode_eQam;
    tunerTuneSettings.modeSettings.qam.annex = NEXUS_FrontendQamAnnex_eB;
    tunerTuneSettings.modeSettings.qam.qamMode = qamMode;
    tunerTuneSettings.frequency = freq * 1000000;
    tunerTuneSettings.tuneCompleteCallback.callback = tune_complete_callback;
    tunerTuneSettings.asyncStatusReadyCallback.callback = async_status_ready_callback;
    tunerTuneSettings.asyncStatusReadyCallback.context = statusEvent;
    
    BDBG_WRN(("tuning %d MHz...", tunerTuneSettings.frequency));
    rc = NEXUS_Tuner_Tune(tunerHandle, &tunerTuneSettings);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    BDBG_ERR(("Press ENTER to get status."));
    getchar();

    rc = NEXUS_Tuner_RequestAsyncStatus(tunerHandle);
	if(rc){rc = BERR_TRACE(rc); goto done;}

	rc = BKNI_WaitForEvent(statusEvent, 1000);
	if(rc){rc = BERR_TRACE(rc); goto done;}

	rc = NEXUS_Tuner_GetAsyncStatus(tunerHandle, &tunerStatus);
    if(rc){rc = BERR_TRACE(rc); goto done;}
	
	BDBG_WRN(("rssi = %d", tunerStatus.rssi));
	BDBG_WRN(("tuneComplete = %d", tunerStatus.tuneComplete));

    BDBG_ERR(("Press ENTER to exit."));
    getchar();
done:
	if (statusEvent) {
        BKNI_DestroyEvent(statusEvent);
    }
    if(tunerHandle) {
        NEXUS_Tuner_Release(tunerHandle);
    }
    NEXUS_Platform_Uninit();
    return 0;
}
#else  /* if NEXUS_HAS_FRONTEND */
#include <stdio.h>
int main(void)
{
    printf("ERROR: This platform doesn't include frontend.inc \n");
    return -1;
}
#endif /* if NEXUS_HAS_FRONTEND */

