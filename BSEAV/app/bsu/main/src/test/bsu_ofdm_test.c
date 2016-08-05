/***************************************************************************
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
 *
 * Module Description:
 *
 ***************************************************************************/
#include <stdio.h>
#include <string.h>
#include "bsu_prompt.h"

#include "nexus_frontend.h"
#include "nexus_platform.h"
#include "nexus_parser_band.h"

/***********************************************************************
 *                      Function Prototypes
 ***********************************************************************/
#if __cplusplus
extern "C" {
#endif

#if __cplusplus
}
#endif

/***********************************************************************
 *                      Global variables
 ***********************************************************************/

BKNI_EventHandle lockEvent;
extern bool frontend_already_opened;

static void lock_callback(void *context, int param)
{
    NEXUS_FrontendHandle frontend = (NEXUS_FrontendHandle)context;
    NEXUS_FrontendFastStatus status;

    BSTD_UNUSED(param);
    NEXUS_Frontend_GetFastStatus(frontend, &status);
    if(status.lockStatus == NEXUS_FrontendLockStatus_eUnlocked)
        printf("OFDM Lock callback: Fast lock status = Unlocked.\n");
    else if(status.lockStatus == NEXUS_FrontendLockStatus_eLocked) {
        printf("OFDM Lock callback: Fast lock status = Locked.\n");
        BKNI_SetEvent(lockEvent);
    }
    else if(status.lockStatus == NEXUS_FrontendLockStatus_eNoSignal)
        printf("OFDM Lock callback: Fast lock status = NoSignal.\n");
    else
        printf("OFDM Lock callback: Fast lock status = Unknown.\n");
}

static void async_status_ready_callback(void *context, int param)
{
    BSTD_UNUSED(param);

    fprintf(stderr, "async_status_ready_callback\n");
    BKNI_SetEvent((BKNI_EventHandle)context);
}

void bsu_ofdm_test(void)
{
    NEXUS_FrontendUserParameters userParams;
    NEXUS_FrontendHandle frontend=NULL;
    NEXUS_FrontendOfdmSettings ofdmSettings;
    NEXUS_FrontendOfdmMode mode = NEXUS_FrontendOfdmMode_eDvbt;
    NEXUS_FrontendAcquireSettings settings;
    NEXUS_ParserBand parserBand;
    NEXUS_ParserBandSettings parserBandSettings;
    unsigned int statusMax = 0;
    BKNI_EventHandle statusEvent;
    NEXUS_Error rc;

    bool dvbT2Lite = false;
    unsigned int freq = 0;

    NEXUS_Error (*RequestAsyncStatus)(NEXUS_FrontendHandle, unsigned int);

    if (frontend_already_opened==false) {
        rc = NEXUS_Platform_InitFrontend();
        if(rc){rc = BERR_TRACE(rc); goto done;}
    }

    NEXUS_Frontend_GetDefaultAcquireSettings(&settings);
    settings.capabilities.ofdm = true;
    frontend = NEXUS_Frontend_Acquire(&settings);
    if (!frontend) {
        fprintf(stderr, "Unable to find OFDM-capable frontend\n");
        return;
    }

    BKNI_CreateEvent(&statusEvent);
    BKNI_CreateEvent(&lockEvent);

    NEXUS_Frontend_GetDefaultOfdmSettings(&ofdmSettings);

    if (mode == NEXUS_FrontendOfdmMode_eDvbt) {
         /* If freq and video PID not set on command line use defaults */
         if ((freq == 0)) {
             printf("Using built in default tune parameters for DVB-T\n");
             freq = 578;
         }

         ofdmSettings.bandwidth = 8000000;
         ofdmSettings.manualTpsSettings = false;
         ofdmSettings.pullInRange = NEXUS_FrontendOfdmPullInRange_eWide;
         ofdmSettings.cciMode = NEXUS_FrontendOfdmCciMode_eNone;

         statusMax = NEXUS_FrontendDvbtStatusType_eMax;
         RequestAsyncStatus = NEXUS_Frontend_RequestDvbtAsyncStatus;

     }
     else if (mode == NEXUS_FrontendOfdmMode_eDvbt2) {
         /* If freq and video PID not set on command line use defaults */
         if ((freq == 0)) {
             printf("Using built in default tune parameters for DVB-T2\n");
             freq = 602;
         }

         ofdmSettings.bandwidth = 8000000;
         ofdmSettings.dvbt2Settings.plpMode = true;
         ofdmSettings.dvbt2Settings.plpId = 0;

         if (dvbT2Lite) {
             ofdmSettings.dvbt2Settings.profile = NEXUS_FrontendDvbt2Profile_eLite;
         } else {
             ofdmSettings.dvbt2Settings.profile = NEXUS_FrontendDvbt2Profile_eBase;
         }

         statusMax = NEXUS_FrontendDvbt2StatusType_eMax;
         RequestAsyncStatus = NEXUS_Frontend_RequestDvbt2AsyncStatus;

    }
    else if(mode == NEXUS_FrontendOfdmMode_eIsdbt) {
        /* If freq and video PID not set on command line use defaults */
        if ((freq == 0)) {
            printf("Using built in default tune parameters for ISDB-T\n");
            freq = 473;
        }

        ofdmSettings.bandwidth = 6000000;

        statusMax = NEXUS_FrontendIsdbtStatusType_eMax;
        RequestAsyncStatus = NEXUS_Frontend_RequestIsdbtAsyncStatus;
   }

/*
    printf("\nTuning Parameters: mode: %s frequency: %d videoPid: %d audioPid: %d videoCodec: %s audioCodec: %s \n\n",
           (mode == NEXUS_FrontendOfdmMode_eDvbt2) ? "DVB-T2" : (mode == NEXUS_FrontendOfdmMode_eDvbt) ? "DVB-T" : "ISDB-T",
           freq,videoPid, audioPid,
           (videoCodec == 2)? "MPEG2" : (videoCodec == 5)? "AVC" : "Unknown",
           (audioCodec == 1)? "MPEG"  : (audioCodec == 3)? "AAC" : (audioCodec == 5) ? "AACplus" : "Unknown");
*/

    ofdmSettings.frequency = freq * 1000000;
    ofdmSettings.acquisitionMode = NEXUS_FrontendOfdmAcquisitionMode_eAuto;
    ofdmSettings.terrestrial = true;
    ofdmSettings.spectrum = NEXUS_FrontendOfdmSpectrum_eAuto;
    ofdmSettings.mode = mode;
    ofdmSettings.lockCallback.callback = lock_callback;
    ofdmSettings.lockCallback.context = frontend;
    ofdmSettings.asyncStatusReadyCallback.callback = async_status_ready_callback;
    ofdmSettings.asyncStatusReadyCallback.context = statusEvent;

    NEXUS_Frontend_GetUserParameters(frontend, &userParams);

    /* Map a parser band to the demod's input band. */
    parserBand = NEXUS_ParserBand_e0;
    NEXUS_ParserBand_GetSettings(parserBand, &parserBandSettings);
    if (userParams.isMtsif) {
        parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eMtsif;
        parserBandSettings.sourceTypeSettings.mtsif = NEXUS_Frontend_GetConnector(frontend); /* NEXUS_Frontend_TuneXyz() will connect this frontend to this parser band */
    }
    else {
        parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
        parserBandSettings.sourceTypeSettings.inputBand = userParams.param1;  /* Platform initializes this to input band */
    }
    parserBandSettings.transportType = NEXUS_TransportType_eTs;
    NEXUS_ParserBand_SetSettings(parserBand, &parserBandSettings);

    NEXUS_Frontend_TuneOfdm(frontend, &ofdmSettings);

    printf("waiting for lock status...\n");
    BKNI_WaitForEvent(lockEvent, 1000);
    BKNI_DestroyEvent(lockEvent);
//    NEXUS_Frontend_Release(frontend);
done:
}
