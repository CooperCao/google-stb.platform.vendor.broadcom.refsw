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

#include "standby.h"

BDBG_MODULE(frontend);

extern B_StandbyNexusHandles g_StandbyNexusHandles;
extern B_DeviceState g_DeviceState;

#if NEXUS_HAS_FRONTEND
void untune_frontend(unsigned id)
{
    if(!g_DeviceState.frontend_tuned[id])
        return;

    NEXUS_Frontend_Release(g_StandbyNexusHandles.frontend[id]);

    g_DeviceState.frontend_tuned[id] = false;
}

static void qam_lock_callback(void *context, int param)
{
    NEXUS_FrontendHandle frontend = (NEXUS_FrontendHandle)context;
    NEXUS_FrontendFastStatus status;

    BSTD_UNUSED(param);

    fprintf(stderr, "Qam Frontend(%p) Lock callback\n", (void*)frontend);

    NEXUS_Frontend_GetFastStatus(frontend, &status);
    if(status.lockStatus == NEXUS_FrontendLockStatus_eUnlocked)
        printf("QAM Lock callback: Fast lock status = Unlocked.\n");
    else if(status.lockStatus == NEXUS_FrontendLockStatus_eLocked){
        printf("QAM Lock callback: Fast lock status = Locked.\n");
        BKNI_SetEvent(g_StandbyNexusHandles.signalLockedEvent);
    }
    else if(status.lockStatus == NEXUS_FrontendLockStatus_eNoSignal)
        printf("QAM Lock callback: Fast lock status = NoSignal.\n");
    else
        printf("QAM Lock callback: Fast lock status = Unknown.\n");
}

int tune_qam(unsigned id)
{
    NEXUS_FrontendAcquireSettings settings;
    NEXUS_FrontendQamSettings qamSettings;
    NEXUS_FrontendUserParameters userParams;
    NEXUS_ParserBandSettings parserBandSettings;
    NEXUS_Error rc=0;

    if(g_DeviceState.frontend_tuned[id])
        return rc;

    NEXUS_Frontend_GetDefaultAcquireSettings(&settings);
    settings.capabilities.qam = true;
    g_StandbyNexusHandles.frontend[id] = NEXUS_Frontend_Acquire(&settings);
    if (!g_StandbyNexusHandles.frontend[id]) {
        BDBG_ERR(("Unable to find QAM-capable frontend"));
        rc = NEXUS_NOT_AVAILABLE;
        return rc;
    }

    NEXUS_Frontend_GetDefaultQamSettings(&qamSettings);
    qamSettings.frequency = g_DeviceState.frontend[id].freq * 1000000;
    switch (g_DeviceState.frontend[id].qammode) {
        default:
        case 64: qamSettings.mode = NEXUS_FrontendQamMode_e64; qamSettings.symbolRate = 5056900; break;
        case 256 : qamSettings.mode = NEXUS_FrontendQamMode_e256; qamSettings.symbolRate = 5360537; break;
        case 1024: qamSettings.mode = NEXUS_FrontendQamMode_e1024; qamSettings.symbolRate = 0; /* TODO */break;
    }
    qamSettings.annex = NEXUS_FrontendQamAnnex_eB;
    qamSettings.bandwidth = NEXUS_FrontendQamBandwidth_e6Mhz;
    qamSettings.lockCallback.callback = qam_lock_callback;
    qamSettings.lockCallback.context = g_StandbyNexusHandles.frontend[id];

    NEXUS_Frontend_GetUserParameters(g_StandbyNexusHandles.frontend[id], &userParams);

    NEXUS_ParserBand_GetSettings(g_StandbyNexusHandles.parserBand[id], &parserBandSettings);
    if (userParams.isMtsif) {
        parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eMtsif;
        parserBandSettings.sourceTypeSettings.mtsif = NEXUS_Frontend_GetConnector(g_StandbyNexusHandles.frontend[id]);
    }
    else {
        parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
        parserBandSettings.sourceTypeSettings.inputBand = userParams.param1;  /* Platform initializes this to input band */
    }
    parserBandSettings.transportType = NEXUS_TransportType_eTs;
    NEXUS_ParserBand_SetSettings(g_StandbyNexusHandles.parserBand[id], &parserBandSettings);

    rc = NEXUS_Frontend_TuneQam(g_StandbyNexusHandles.frontend[id], &qamSettings);
    if (rc) { BERR_TRACE(rc);}

    g_DeviceState.frontend_tuned[id] = true;

    return rc;
}

void untune_qam(unsigned id)
{
   untune_frontend(id);
}

static void ofdm_lock_callback(void *context, int param)
{
    NEXUS_FrontendHandle frontend = (NEXUS_FrontendHandle)context;
    NEXUS_FrontendFastStatus status;

    BSTD_UNUSED(param);

    fprintf(stderr, "Ofdm Frontend(%p) Lock callback\n", (void*)frontend);

    NEXUS_Frontend_GetFastStatus(frontend, &status);
    if(status.lockStatus == NEXUS_FrontendLockStatus_eUnlocked)
        printf("OFDM Lock callback: Fast lock status = Unlocked.\n");
    else if(status.lockStatus == NEXUS_FrontendLockStatus_eLocked){
        printf("OFDM Lock callback: Fast lock status = Locked.\n");
        BKNI_SetEvent(g_StandbyNexusHandles.signalLockedEvent);
    }
    else if(status.lockStatus == NEXUS_FrontendLockStatus_eNoSignal)
        printf("OFDM Lock callback: Fast lock status = NoSignal.\n");
    else
        printf("OFDM Lock callback: Fast lock status = Unknown.\n");
}

int tune_ofdm(unsigned id)
{
    NEXUS_FrontendOfdmSettings ofdmSettings;
    NEXUS_FrontendAcquireSettings settings;
    NEXUS_FrontendUserParameters userParams;
    NEXUS_ParserBandSettings parserBandSettings;
    NEXUS_Error rc=0;

    if(g_DeviceState.frontend_tuned[id])
        return rc;

    if(!g_StandbyNexusHandles.frontend[id]){
        NEXUS_Frontend_GetDefaultAcquireSettings(&settings);
        settings.capabilities.ofdm = true;
        g_StandbyNexusHandles.frontend[id] = NEXUS_Frontend_Acquire(&settings);
        if (!g_StandbyNexusHandles.frontend[id]) {
            fprintf(stderr, "Unable to find OFDM-capable frontend\n");
            rc = NEXUS_NOT_AVAILABLE;
            return rc;
        }
    }

    NEXUS_Frontend_GetDefaultOfdmSettings(&ofdmSettings);

    ofdmSettings.frequency = g_DeviceState.frontend[id].freq * 1000000;
    ofdmSettings.acquisitionMode = NEXUS_FrontendOfdmAcquisitionMode_eAuto;
    ofdmSettings.spectrum = NEXUS_FrontendOfdmSpectrum_eAuto;

    switch (g_DeviceState.frontend[id].ofdmmode) {
        default:
        case 0:
            ofdmSettings.mode = NEXUS_FrontendOfdmMode_eDvbt;
            ofdmSettings.bandwidth = 8000000;
            ofdmSettings.manualTpsSettings = false;
            ofdmSettings.pullInRange = NEXUS_FrontendOfdmPullInRange_eWide;
            ofdmSettings.cciMode = NEXUS_FrontendOfdmCciMode_eNone;
            ofdmSettings.terrestrial = true;
            break;
        case 1:
            ofdmSettings.mode = NEXUS_FrontendOfdmMode_eDvbt2;
            ofdmSettings.bandwidth = 8000000;
            ofdmSettings.dvbt2Settings.plpMode = true;
            ofdmSettings.dvbt2Settings.plpId = 0;
            ofdmSettings.dvbt2Settings.profile = NEXUS_FrontendDvbt2Profile_eBase;
            ofdmSettings.terrestrial = true;
            break;
        case 2: ofdmSettings.mode = NEXUS_FrontendOfdmMode_eDvbc2; break;
        case 3: ofdmSettings.mode = NEXUS_FrontendOfdmMode_eIsdbt; ofdmSettings.bandwidth = 6000000; break;
    }

    ofdmSettings.lockCallback.callback = ofdm_lock_callback;
    ofdmSettings.lockCallback.context = g_StandbyNexusHandles.frontend[id];

    NEXUS_Frontend_GetUserParameters(g_StandbyNexusHandles.frontend[id], &userParams);

    NEXUS_ParserBand_GetSettings(g_StandbyNexusHandles.parserBand[id], &parserBandSettings);
    if (userParams.isMtsif) {
        parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eMtsif;
        parserBandSettings.sourceTypeSettings.mtsif = NEXUS_Frontend_GetConnector(g_StandbyNexusHandles.frontend[id]); /* NEXUS_Frontend_TuneXyz() will connect this frontend to this parser band */
    }
    else {
        parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
        parserBandSettings.sourceTypeSettings.inputBand = userParams.param1;  /* Platform initializes this to input band */
    }
    parserBandSettings.transportType = NEXUS_TransportType_eTs;
    NEXUS_ParserBand_SetSettings(g_StandbyNexusHandles.parserBand[id], &parserBandSettings);

    rc = NEXUS_Frontend_TuneOfdm(g_StandbyNexusHandles.frontend[id], &ofdmSettings);
    if (rc) { BERR_TRACE(rc);}

    g_DeviceState.frontend_tuned[id] = true;

    return rc;
}

void untune_ofdm(unsigned id)
{
    untune_frontend(id);
}

static void sat_lock_callback(void *context, int param)
{
    NEXUS_FrontendHandle frontend = (NEXUS_FrontendHandle)context;
    NEXUS_FrontendSatelliteStatus status;

    BSTD_UNUSED(param);

    fprintf(stderr, "Sat Frontend(%p) Lock callback\n", (void*)frontend);

    NEXUS_Frontend_GetSatelliteStatus(frontend, &status);
    fprintf(stderr, "  demodLocked = %d\n", status.demodLocked);

    BKNI_SetEvent(g_StandbyNexusHandles.signalLockedEvent);
}

int tune_sat(unsigned id)
{
    NEXUS_FrontendAcquireSettings settings;
    NEXUS_FrontendSatelliteSettings satSettings;
    NEXUS_FrontendDiseqcSettings diseqcSettings;
    NEXUS_FrontendUserParameters userParams;
    NEXUS_ParserBandSettings parserBandSettings;
    NEXUS_Error rc=0;

    if(g_DeviceState.frontend_tuned[id])
        return rc;

    NEXUS_Frontend_GetDefaultAcquireSettings(&settings);
    settings.capabilities.satellite = true;
    g_StandbyNexusHandles.frontend[id] = NEXUS_Frontend_Acquire(&settings);
    if (!g_StandbyNexusHandles.frontend[id]) {
        fprintf(stderr, "Unable to find satellite-capable frontend\n");
        rc = NEXUS_NOT_AVAILABLE;
        return rc;
    }

    NEXUS_Frontend_GetDefaultSatelliteSettings(&satSettings);
    satSettings.frequency = g_DeviceState.frontend[id].freq * 1000000;
    satSettings.mode = g_DeviceState.frontend[id].satmode;
    satSettings.lockCallback.callback = sat_lock_callback;
    satSettings.lockCallback.context = g_StandbyNexusHandles.frontend[id];

    NEXUS_Frontend_GetUserParameters(g_StandbyNexusHandles.frontend[id], &userParams);

    /* Map a parser band to the demod's input band. */
    NEXUS_ParserBand_GetSettings(g_StandbyNexusHandles.parserBand[id], &parserBandSettings);
    if (userParams.isMtsif) {
        parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eMtsif;
        parserBandSettings.sourceTypeSettings.mtsif = NEXUS_Frontend_GetConnector(g_StandbyNexusHandles.frontend[id]);
    } else {
        parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
        parserBandSettings.sourceTypeSettings.inputBand = userParams.param1;  /* Platform initializes this to input band */
    }
    parserBandSettings.transportType = NEXUS_TransportType_eTs;
    NEXUS_ParserBand_SetSettings(g_StandbyNexusHandles.parserBand[id], &parserBandSettings);

    NEXUS_Frontend_GetDiseqcSettings(g_StandbyNexusHandles.frontend[id], &diseqcSettings);
    diseqcSettings.toneEnabled = true;
    diseqcSettings.voltage = NEXUS_FrontendDiseqcVoltage_e13v;
    NEXUS_Frontend_SetDiseqcSettings(g_StandbyNexusHandles.frontend[id], &diseqcSettings);
    printf("Set DiseqcSettings\n");

    if (g_DeviceState.frontend[id].adc != 0) {
        NEXUS_FrontendSatelliteRuntimeSettings settings;
        NEXUS_Frontend_GetSatelliteRuntimeSettings(g_StandbyNexusHandles.frontend[id], &settings);
        settings.selectedAdc = g_DeviceState.frontend[id].adc;
        NEXUS_Frontend_SetSatelliteRuntimeSettings(g_StandbyNexusHandles.frontend[id], &settings);
    }

    rc = NEXUS_Frontend_TuneSatellite(g_StandbyNexusHandles.frontend[id], &satSettings);
    if (rc) { BERR_TRACE(rc);}

    g_DeviceState.frontend_tuned[id] = true;

    return rc;
}

void untune_sat(unsigned id)
{
    untune_frontend(id);
}
#endif

int streamer_start(unsigned id)
{
    NEXUS_ParserBandSettings parserBandSettings;

    if(g_DeviceState.frontend_tuned[id])
        return 0;

    NEXUS_ParserBand_GetSettings(g_StandbyNexusHandles.parserBand[id], &parserBandSettings);
    parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
    NEXUS_Platform_GetStreamerInputBand(0, &parserBandSettings.sourceTypeSettings.inputBand);
    parserBandSettings.transportType = NEXUS_TransportType_eTs;
    NEXUS_ParserBand_SetSettings(g_StandbyNexusHandles.parserBand[id], &parserBandSettings);

    return 0;
}

void streamer_stop(unsigned id)
{
    BSTD_UNUSED(id);
    /* Nothing to untune for streamer */
    return;
}
