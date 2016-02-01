/***************************************************************************
 *     Copyright (c) 2003-2013, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
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
