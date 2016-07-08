/******************************************************************************
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
 ******************************************************************************/
/* Example to tune a OFDM channel using nexus */
#if NEXUS_HAS_FRONTEND
#include "nexus_frontend.h"
#include "nexus_platform.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

BDBG_MODULE(tune_ofdm);

/* Accumulate frequency is in milli-seconds. This has to be less tha three seconds else the 32 bit counters in the hardware will overflow. 
    Hence we might get errenous results. */
#define STATUS_FREQUENCY 2000

#define STATUS_TIMEOUT 1000

/* Error counters will cccumulate every STATUS_FREQUENCY*ACCUMULATE_COUNT seconds.*/
#define ACCUMULATE_COUNT 2

static void lock_callback(void *context, int param)
{
    NEXUS_FrontendHandle frontend = (NEXUS_FrontendHandle)context;
    NEXUS_FrontendFastStatus status;

    BSTD_UNUSED(param);
    NEXUS_Frontend_GetFastStatus(frontend, &status);
    if(status.lockStatus == NEXUS_FrontendLockStatus_eUnlocked)
        printf("OFDM Lock callback: Fast lock status = Unlocked.\n");
    else if(status.lockStatus == NEXUS_FrontendLockStatus_eLocked)
        printf("OFDM Lock callback: Fast lock status = Locked.\n");
    else if(status.lockStatus == NEXUS_FrontendLockStatus_eNoSignal)
        printf("OFDM Lock callback: Fast lock status = NoSignal.\n");
    else
        printf("OFDM Lock callback: Fast lock status = Unknown.\n");
}

static void async_status_ready_callback(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
}

int main(int argc, char **argv)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_FrontendHandle frontend=NULL;
    NEXUS_FrontendOfdmSettings ofdmSettings;
    NEXUS_FrontendOfdmMode mode = NEXUS_FrontendOfdmMode_eIsdbt;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_FrontendAcquireSettings settings;
    NEXUS_FrontendIsdbtStatusReady isdbtStatusReady;
    NEXUS_FrontendIsdbtStatus isdbtStatus;
    /* default freq & qam mode */
    unsigned freq = 0;
    BKNI_EventHandle statusEvent;
    NEXUS_FrontendOfdmBertHeader header = NEXUS_FrontendOfdmBertHeader_e4Byte;
    NEXUS_FrontendOfdmBertPolynomial polynomial = NEXUS_FrontendOfdmBertPolynomial_e23; 
    NEXUS_FrontendErrorRate accLayerARate;
    NEXUS_FrontendErrorRate accLayerBRate;
    NEXUS_FrontendErrorRate accLayerCRate;
    unsigned count=ACCUMULATE_COUNT;

    /* allow cmdline freq & qam mode for simple test */
     if (argc > 1) {
        mode = atoi(argv[1]);
    }
    if (argc > 2) {
        freq = atoi(argv[2]);
        if (argc > 3) {
            header = atoi(argv[3]);
            if (argc > 4) {
                polynomial = atoi(argv[4]);
            }
        }
    }

    NEXUS_Platform_Init(NULL);
    NEXUS_Platform_GetConfiguration(&platformConfig);

    NEXUS_Frontend_GetDefaultAcquireSettings(&settings);
    settings.capabilities.ofdm = true;
    frontend = NEXUS_Frontend_Acquire(&settings);
    if (!frontend) {
        fprintf(stderr, "Unable to find OFDM-capable frontend\n");
        return -1;
    }

    BKNI_CreateEvent(&statusEvent);

    NEXUS_Frontend_GetDefaultOfdmSettings(&ofdmSettings);
    
    if(mode == NEXUS_FrontendOfdmMode_eIsdbt){
        ofdmSettings.bandwidth = 6000000;
    }
    else{
        fprintf(stderr, "Unsupported mode.\n");
        goto done;
    }

    ofdmSettings.frequency = freq;
    ofdmSettings.acquisitionMode = NEXUS_FrontendOfdmAcquisitionMode_eAuto;
    ofdmSettings.terrestrial = true;
    ofdmSettings.spectrum = NEXUS_FrontendOfdmSpectrum_eAuto;
    ofdmSettings.mode = mode;
    ofdmSettings.lockCallback.callback = lock_callback;
    ofdmSettings.lockCallback.context = frontend;
    ofdmSettings.asyncStatusReadyCallback.callback = async_status_ready_callback;
    ofdmSettings.asyncStatusReadyCallback.context = statusEvent;

    ofdmSettings.bert.enabled = true;
    ofdmSettings.bert.header = header;
    ofdmSettings.bert.polynomial = polynomial;

    NEXUS_Frontend_TuneOfdm(frontend, &ofdmSettings);
    BKNI_Memset(&accLayerARate, 0, sizeof(accLayerARate));
    BKNI_Memset(&accLayerBRate, 0, sizeof(accLayerBRate));
    BKNI_Memset(&accLayerCRate, 0, sizeof(accLayerCRate));

    while(1){
        if(count--){
            rc = NEXUS_Frontend_RequestIsdbtAsyncStatus(frontend, NEXUS_FrontendIsdbtStatusType_eBasic);
            if(rc){rc = BERR_TRACE(rc); goto done;}

            rc = BKNI_WaitForEvent(statusEvent, STATUS_TIMEOUT);
            if(rc){rc = BERR_TRACE(rc); goto done;}

            rc =  NEXUS_Frontend_GetIsdbtAsyncStatusReady(frontend, &isdbtStatusReady);
            if(rc){rc = BERR_TRACE(rc); goto done;}
            
            if(isdbtStatusReady.type[NEXUS_FrontendIsdbtStatusType_eBasic]){
                BDBG_WRN(("Status received."));
                rc = NEXUS_Frontend_GetIsdbtAsyncStatus(frontend, NEXUS_FrontendIsdbtStatusType_eBasic, &isdbtStatus);
                if(rc){rc = BERR_TRACE(rc); goto done;}
            }

            BKNI_Sleep(STATUS_FREQUENCY);

            accLayerARate.count += isdbtStatus.status.basic.layerA.bert.errorRate.count;
            accLayerARate.total += isdbtStatus.status.basic.layerA.bert.errorRate.total;
            accLayerARate.rate += isdbtStatus.status.basic.layerA.bert.errorRate.rate;

            accLayerBRate.count += isdbtStatus.status.basic.layerB.bert.errorRate.count;
            accLayerBRate.total += isdbtStatus.status.basic.layerB.bert.errorRate.total;
            accLayerBRate.rate += isdbtStatus.status.basic.layerB.bert.errorRate.rate;

            accLayerCRate.count += isdbtStatus.status.basic.layerC.bert.errorRate.count;
            accLayerCRate.total += isdbtStatus.status.basic.layerC.bert.errorRate.total;
            accLayerCRate.rate += isdbtStatus.status.basic.layerC.bert.errorRate.rate;
            
            BDBG_MSG(("Layer A locked = %d", isdbtStatus.status.basic.layerA.bert.locked));
            BDBG_MSG(("Layer A count = %u", (unsigned)isdbtStatus.status.basic.layerA.bert.errorRate.count));
            BDBG_MSG(("Layer A total = %u", (unsigned)isdbtStatus.status.basic.layerA.bert.errorRate.total));

            BDBG_MSG(("Layer B locked = %d", isdbtStatus.status.basic.layerB.bert.locked));
            BDBG_MSG(("Layer B count = %u", (unsigned)isdbtStatus.status.basic.layerB.bert.errorRate.count));
            BDBG_MSG(("Layer B total = %u", (unsigned)isdbtStatus.status.basic.layerB.bert.errorRate.total));

            BDBG_MSG(("Layer C locked = %d", isdbtStatus.status.basic.layerC.bert.locked));
            BDBG_MSG(("Layer C count = %u", (unsigned)isdbtStatus.status.basic.layerC.bert.errorRate.count));
            BDBG_MSG(("Layer C total = %u", (unsigned)isdbtStatus.status.basic.layerC.bert.errorRate.total));
        }
        else {
            count=ACCUMULATE_COUNT;
            BDBG_WRN(("Accumulated Layer A count = %u", (unsigned)accLayerARate.count));
            BDBG_WRN(("Accumulated Layer A total = %u", (unsigned)accLayerARate.total));
            BDBG_WRN(("Accumulated Layer A rate = %d", accLayerARate.rate));

            BDBG_WRN(("Accumulated Layer B count = %u", (unsigned)accLayerBRate.count));
            BDBG_WRN(("Accumulated Layer B total = %u", (unsigned)accLayerBRate.total));
            BDBG_WRN(("Accumulated Layer B rate = %d", accLayerBRate.rate));

            BDBG_WRN(("Accumulated Layer C count = %u", (unsigned)accLayerCRate.count));
            BDBG_WRN(("Accumulated Layer C total = %u", (unsigned)accLayerCRate.total));
            BDBG_WRN(("Accumulated Layer C rate = %d", accLayerCRate.rate));

            BKNI_Memset(&accLayerARate, 0, sizeof(accLayerARate));
            BKNI_Memset(&accLayerBRate, 0, sizeof(accLayerBRate));
            BKNI_Memset(&accLayerCRate, 0, sizeof(accLayerCRate));
        }
    }

done:
    /* example shutdown */
    NEXUS_Frontend_Untune(frontend);
    if (statusEvent) {
        BKNI_DestroyEvent(statusEvent);
    }

    NEXUS_Frontend_Release(frontend);
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

