/******************************************************************************
 *    (c)2008-2014 Broadcom Corporation
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
#include <string.h>
#include <stdlib.h>
#include "nexus_platform.h"
#include "bstd.h"
#include "bkni.h"
#include "bdbg.h"
#include "frontend.h"

BDBG_MODULE(frontend);

#if NEXUS_HAS_FRONTEND
static void
usage(void *settings)
{
    BSTD_UNUSED(settings);
    printf("\n  -freq           - tuner frequency in MHz");
    printf("\n  -vsb");
    print_list(g_vsbModeStrs);
    printf("\n  -qam");
    print_list(g_qamModeStrs);
    printf("\n  -ofdm");
    print_list(g_ofdmModeStrs);
    printf("\n  -sat");
    print_list(g_satModeStrs);
    printf("\n  -voltage");
    print_list(g_diseqcVoltageStrs);
    printf("\n  -tone");
    print_list(g_diseqcToneEnabledStrs);
    printf("\n  -networkspec");
    print_list(g_satNetworkSpecStrs);
    printf("\n  -adc         - assign demod to a specific ADC");
    printf("\n  -kufreq      - Ku Band freq in MHz, calculates -freq and -tone automatically");
    printf("\n  -ksyms       - symbol rate in ksyms");
    printf("\n  -bandwidth   - bandwidth in MHz");
    printf("\n  -plpid       - DVB-T2 PLP ID");
    printf("\n  -dvbvt2profile");
    print_list(g_dvbt2ProfileStrs);
}
#endif

#if NEXUS_HAS_FRONTEND
static BKNI_EventHandle lockEvent;
static void lock_callback(void *context, int param)
{
#if 0 /* TODO: for now, assume frontend locked upon entrance to this function */
    NEXUS_FrontendHandle frontend = (NEXUS_FrontendHandle)context;
    RecordSource source = (int)param;
    bool locked = false;

    if (source==RecordSource_eVsb) {
        NEXUS_FrontendVsbStatus status;
        NEXUS_Frontend_GetVsbStatus(frontend, &status);
        locked = status.receiverLock;
    }
    else if (source==RecordSource_eQam) {
        NEXUS_FrontendQamStatus status;
        NEXUS_Frontend_GetQamStatus(frontend, &status);
        locked = status.receiverLock;
    }
    else if (source==RecordSource_eSat) {
        NEXUS_FrontendSatelliteStatus status;
        NEXUS_Frontend_GetSatelliteStatus(frontend, &status);
        locked = status.demodLocked;
    }

    if (locked) {
        BKNI_SetEvent(lockEvent);
    }
#else
    NEXUS_FrontendFastStatus status;
    NEXUS_FrontendHandle frontend = (NEXUS_FrontendHandle)context;
    BSTD_UNUSED(param);
    NEXUS_Frontend_GetFastStatus(frontend, &status);
    if (status.lockStatus == NEXUS_FrontendLockStatus_eLocked) {
        printf("locked callback\n");
        BKNI_SetEvent(lockEvent);
    }
#endif
    return;
}

static BKNI_EventHandle ofdmStatusEvent;
static void
ofdm_status_callback(void *context, int param)
{
    BSTD_UNUSED(context);
    BSTD_UNUSED(param);
    BKNI_SetEvent(ofdmStatusEvent);
}

#endif

int
frontend_selected(FrontendSettings *settings)
{
#if NEXUS_HAS_FRONTEND
    if (settings->opts.vsbMode!=NEXUS_FrontendVsbMode_eMax) {
        settings->source = FrontendSource_eVsb;
        return 1;
    }
    else if (settings->opts.qamMode!=NEXUS_FrontendQamMode_eMax) {
        settings->source = FrontendSource_eQam;
        return 1;
    }
    else if (settings->opts.ofdmMode!=NEXUS_FrontendOfdmMode_eMax) {
        settings->source = FrontendSource_eOfdm;
        return 1;
    }
    else if (settings->opts.satMode!=NEXUS_FrontendSatelliteMode_eMax) {
        settings->source = FrontendSource_eSat;
        return 1;
    }
#endif
    settings->source = FrontendSource_eMax;
    return 0;
}

int
frontend_check_capabilities(FrontendSettings *settings, NEXUS_PlatformConfiguration *platformConfig)
{
#if NEXUS_HAS_FRONTEND
    if (settings->source != FrontendSource_eMax) {
        int i;
        for (i=0; i<NEXUS_MAX_FRONTENDS; i++) {
            NEXUS_FrontendCapabilities capabilities;
            settings->handle = platformConfig->frontend[i];
            if (settings->handle) {
                NEXUS_Frontend_GetCapabilities(settings->handle, &capabilities);
                if (settings->source==FrontendSource_eVsb && capabilities.vsb) { break; }
                else if (settings->source==FrontendSource_eQam && capabilities.qam) { break; }
                else if (settings->source==FrontendSource_eOfdm && capabilities.ofdm) { break; }
                else if (settings->source==FrontendSource_eSat && capabilities.satellite) { break; }
                else {
                    settings->handle = NULL;
                }
            }
        }

        if (!settings->handle) {
            BDBG_ERR(("Unable to find capable frontend"));
            return 0;
        }
        return 1;
    }
#else
    BSTD_UNUSED(settings);
    BSTD_UNUSED(platformConfig);
#endif
    return 0;
}

int
frontend_check_usage(FrontendSettings *settings)
{
#if NEXUS_HAS_FRONTEND
    if (settings->source != FrontendSource_eMax && settings->opts.freq == 0) {
        BDBG_ERR(("For frontend source, you must specify a frequency"));
        return 0;
    }
    return 1;
#else
    BSTD_UNUSED(settings);
    return 1;
#endif
}


void
frontend_set_settings(FrontendSettings *settings, struct common_opts_t *common)
{
#if !NEXUS_HAS_FRONTEND
    BSTD_UNUSED(common);
#endif
    switch (settings->source) {
#if NEXUS_HAS_FRONTEND
    case FrontendSource_eVsb:
        NEXUS_Frontend_GetDefaultVsbSettings(&settings->vsbSettings);
        settings->vsbSettings.frequency = settings->opts.freq * 1000000;
        settings->vsbSettings.mode = settings->opts.vsbMode;
        settings->vsbSettings.lockCallback.callback = lock_callback;
        settings->vsbSettings.lockCallback.context = settings->handle;
        settings->vsbSettings.lockCallback.param = (int)settings->source;
        break;
    case FrontendSource_eQam:
        NEXUS_Frontend_GetDefaultQamSettings(&settings->qamSettings);
        settings->qamSettings.frequency = settings->opts.freq * 1000000;
        settings->qamSettings.mode = settings->opts.qamMode;
        settings->qamSettings.annex = NEXUS_FrontendQamAnnex_eB;
        settings->qamSettings.lockCallback.callback = lock_callback;
        settings->qamSettings.lockCallback.context = settings->handle;
        settings->qamSettings.lockCallback.param = (int)settings->source;
        break;
    case FrontendSource_eOfdm:
        NEXUS_Frontend_GetDefaultOfdmSettings(&settings->ofdmSettings);
        settings->ofdmSettings.frequency = settings->opts.freq * 1000000;
        settings->ofdmSettings.mode = settings->opts.ofdmMode;
        if (settings->opts.bandwidth > 0) {
            settings->ofdmSettings.bandwidth = settings->opts.bandwidth * 1000000;
        }
        if (settings->ofdmSettings.mode == NEXUS_FrontendOfdmMode_eDvbt2) {
            if (settings->opts.plpid >= 0) {
                settings->ofdmSettings.dvbt2Settings.plpMode = 0;
                settings->ofdmSettings.dvbt2Settings.plpId = settings->opts.plpid;
            }
            if (settings->opts.dvbt2Profile != NEXUS_FrontendDvbt2Profile_eMax) {
                settings->ofdmSettings.dvbt2Settings.profile = settings->opts.dvbt2Profile;
            }
        }
        settings->ofdmSettings.lockCallback.callback = lock_callback;
        settings->ofdmSettings.lockCallback.context = settings->handle;
        settings->ofdmSettings.lockCallback.param = (int)settings->source;
        settings->ofdmSettings.asyncStatusReadyCallback.callback = ofdm_status_callback;
        settings->ofdmSettings.asyncStatusReadyCallback.context = settings->handle;
        settings->ofdmSettings.asyncStatusReadyCallback.param = (int)settings->source;
        break;
    case FrontendSource_eSat:
        if (settings->opts.adc != -1) {
            NEXUS_FrontendSatelliteRuntimeSettings rts;
            NEXUS_Frontend_GetSatelliteRuntimeSettings(settings->handle, &rts);
            rts.selectedAdc = settings->opts.adc;
            NEXUS_Frontend_SetSatelliteRuntimeSettings(settings->handle, &rts);
        }
        NEXUS_Frontend_GetDefaultSatelliteSettings(&settings->satSettings);
        settings->satSettings.frequency = settings->opts.freq * 1000000;
        settings->satSettings.mode = settings->opts.satMode;
        settings->satSettings.lockCallback.callback = lock_callback;
        settings->satSettings.lockCallback.context = settings->handle;
        settings->satSettings.lockCallback.param = (int)settings->source;
        settings->satSettings.symbolRate = settings->opts.ksyms * 1000;
        settings->satSettings.networkSpec = settings->opts.satNetworkSpec;
        if (settings->opts.satMode==NEXUS_FrontendSatelliteMode_eDvb &&
            common->transportType!=NEXUS_TransportType_eTs) {
            BDBG_WRN(("mpeg_type for Sat_DVB was not set to TS"));
        }
        if (settings->opts.satMode==NEXUS_FrontendSatelliteMode_eDss &&
            common->transportType!=NEXUS_TransportType_eDssEs) {
            BDBG_WRN(("mpeg_type for Sat_DSS was not set to DSS_ES"));
        }

        /* Adjust pilot behaviour for DVB-S2 */
        switch (settings->opts.satMode) {
        case NEXUS_FrontendSatelliteMode_eQpskLdpc:
            settings->satSettings.ldpcPilot     = false;
            settings->satSettings.ldpcPilotPll  = false;
            settings->satSettings.ldpcPilotScan = true;
            break;
        case NEXUS_FrontendSatelliteMode_e8pskLdpc:
            settings->satSettings.ldpcPilot     = true;
            settings->satSettings.ldpcPilotPll  = true;
            settings->satSettings.ldpcPilotScan = false;
            break;
        default:
            break;
        }

        NEXUS_Frontend_GetDiseqcSettings(settings->handle, &settings->diseqcSettings);
        switch (settings->opts.satMode) {
        case NEXUS_FrontendSatelliteMode_eDvb:
        case NEXUS_FrontendSatelliteMode_eQpskLdpc:
        case NEXUS_FrontendSatelliteMode_e8pskLdpc:
            settings->diseqcSettings.voltage = settings->opts.diseqcVoltage == NEXUS_FrontendDiseqcVoltage_eMax ? NEXUS_FrontendDiseqcVoltage_e13v : settings->opts.diseqcVoltage;
            if (settings->opts.toneEnabled < 0) {
                settings->diseqcSettings.toneEnabled = true;
            }
            else {
                settings->diseqcSettings.toneEnabled = settings->opts.toneEnabled;
            }
            break;
        case NEXUS_FrontendSatelliteMode_eDss:
            if (settings->opts.toneEnabled < 0) {
                settings->diseqcSettings.toneEnabled = false;
            }
            else {
                settings->diseqcSettings.toneEnabled = settings->opts.toneEnabled;
            }
            settings->diseqcSettings.voltage = NEXUS_FrontendDiseqcVoltage_e18v;
            break;
        default:
            break;
        }
        NEXUS_Frontend_SetDiseqcSettings(settings->handle, &settings->diseqcSettings);
        break;
#endif
    default:
        BDBG_ERR(("unknown source: %d", settings->source));
        return;
    }
}

void
frontend_set_parserbandsettings(FrontendSettings *settings, NEXUS_ParserBandSettings *parserBandSettings)
{
#if NEXUS_HAS_FRONTEND
    NEXUS_FrontendUserParameters userParams;
    NEXUS_Error rc = NEXUS_Frontend_GetUserParameters(settings->handle, &userParams);
    BDBG_ASSERT(!rc);
    if (userParams.isMtsif) {
        parserBandSettings->sourceType = NEXUS_ParserBandSourceType_eMtsif;
        parserBandSettings->sourceTypeSettings.mtsif = NEXUS_Frontend_GetConnector(settings->handle); /* NEXUS_Frontend_TuneXyz() will connect this frontend to this parser band */
    }
    else {
        parserBandSettings->sourceType = NEXUS_ParserBandSourceType_eInputBand;
        parserBandSettings->sourceTypeSettings.inputBand = userParams.param1;  /* Platform initializes this to input band */
    }
#else
    BSTD_UNUSED(settings);
    BSTD_UNUSED(parserBandSettings);
#endif
}

int
frontend_tune(FrontendSettings *settings)
{
#if NEXUS_HAS_FRONTEND
    switch (settings->source) {
    case FrontendSource_eVsb:
        NEXUS_Frontend_TuneVsb(settings->handle, &settings->vsbSettings);
        break;
    case FrontendSource_eQam:
        NEXUS_Frontend_TuneQam(settings->handle, &settings->qamSettings);
        break;
    case FrontendSource_eOfdm:
        NEXUS_Frontend_TuneOfdm(settings->handle, &settings->ofdmSettings);
        break;
    case FrontendSource_eSat:
        NEXUS_Frontend_TuneSatellite(settings->handle, &settings->satSettings);
        break;
    default:
        return 0;
    }

    BDBG_WRN(("Waiting for frontend lock..."));
    if (BKNI_WaitForEvent(lockEvent, 5000)) {
        BDBG_ERR(("Failed"));
        return 0;
    }
    BDBG_WRN(("Locked"));
    switch (settings->source) {
    case FrontendSource_eVsb:
        break;
    case FrontendSource_eQam:
        break;
    case FrontendSource_eOfdm:
        break;
    case FrontendSource_eSat: {
        NEXUS_FrontendSatelliteStatus satStatus;
        NEXUS_Frontend_GetSatelliteStatus(settings->handle, &satStatus);
        printf("mode: %d\n", satStatus.mode);
        printf("codeRate: %d/%d/%d\n", satStatus.codeRate.numerator, satStatus.codeRate.denominator, satStatus.codeRate.bitsPerSymbol);
#if 0
            NEXUS_FrontendSatelliteMode mode;                       /* Mode */
    NEXUS_FrontendSatelliteCodeRate codeRate;               /* Code rate detected */
    NEXUS_FrontendSatelliteInversion spectralInversion;     /* Spectral inversion status */
    unsigned frequency;         /* actual tuner frequency */

    bool tunerLocked;           /* true if the tuner is locked */
    bool demodLocked;           /* true if the demodulator is locked */
    bool bertLocked;            /* true if the BER tester is locked.  If so, see berEstimate. */
                                /* Also, see BERT notes under NEXUS_FrontendSatelliteSettings. */

    unsigned channel;           /* Channel number */
    unsigned symbolRate;        /* In baud */
    int symbolRateError;        /* In baud */

    int carrierOffset;          /* In Hz */
    int carrierError;           /* In Hz */
    unsigned sampleClock;       /* In Hz */
    unsigned outputBitRate;     /* Output bit rate in bps */

    unsigned ifAgcLevel;        /* IF AGC level in units of 1/10 percent */
    unsigned rfAgcLevel;        /* tuner AGC level in units of 1/10 percent */
    unsigned intAgcLevel;       /* Internal AGC level in units of 1/10 percent */
    unsigned snrEstimate;       /* SNR in 1/100 dB */
    unsigned berEstimate;       /* Bit error rate as log10 of 0.0-1.0 range.
                                    1.0  => 100% => 0
                                    0.1  => 10%  => -1
                                    0.01 => 1%   => -2
                                    0    => 0%   => 1 (special value for NONE)
                                    If bertLocked == false, it's set to 1. */

    unsigned fecPhase;          /* 0, 90, 180, 270 */
    unsigned fecCorrected;      /* cumulative block correctable errors */
    unsigned fecUncorrected;    /* cumulative block uncorrectable errors */
    unsigned fecClean;          /* cumulative clean block count */
    unsigned bitErrCorrected;   /* cumulative bit correctable errors */
    unsigned reacquireCount;    /* cumulative reacquisition count */
    unsigned berErrorCount;     /* BER error count - only valid when using BERT and when bertLocked is true.  Also, see BERT notes under NEXUS_FrontendSatelliteSettings. */
    unsigned preViterbiErrorCount;    /* accumulated pre-Viterbi error count */
    unsigned mpegErrors;        /* mpeg frame error count */
    unsigned mpegCount;         /* total mpeg frame count */
    unsigned ifAgc;             /* if agc value from hw unmodified */
    unsigned rfAgc;             /* rf agc value from hw unmodified */
    int agf;                    /* AGF integrator value */
#endif
        break;
    }
    default:
        return 0;
    }
#else
    BSTD_UNUSED(settings);
#endif
    return 1;
}

#if NEXUS_HAS_FRONTEND
static int parse(int *argc, const char *argv[], void *param)
{
    FrontendSettings *settings = param;
    int i = *argc;
    if (!strcmp(argv[i], "-vsb")) {
        settings->opts.vsbMode = lookup(g_vsbModeStrs, argv[++i]);
    }
    else if (!strcmp(argv[i], "-qam")) {
        settings->opts.qamMode = lookup(g_qamModeStrs, argv[++i]);
    }
    else if (!strcmp(argv[i], "-ofdm")) {
        settings->opts.ofdmMode = lookup(g_ofdmModeStrs, argv[++i]);
    }
    else if (!strcmp(argv[i], "-sat")) {
        settings->opts.satMode = lookup(g_satModeStrs, argv[++i]);
    }
    else if (!strcmp(argv[i], "-voltage")) {
        settings->opts.diseqcVoltage = lookup(g_diseqcVoltageStrs, argv[++i]);
    }
    else if (!strcmp(argv[i], "-adc")) {
        settings->opts.adc = strtoul(argv[++i], NULL, 0);
    }
    else if (!strcmp(argv[i], "-freq")) {
        settings->opts.freq = strtoul(argv[++i], NULL, 0);
    }
    else if (!strcmp(argv[i], "-ksyms")) {
        settings->opts.ksyms = strtoul(argv[++i], NULL, 0);
    }
    else if (!strcmp(argv[i], "-bandwidth")) {
        settings->opts.bandwidth = strtoul(argv[++i], NULL, 0);
    }
    else if (!strcmp(argv[i], "-plpid")) {
        settings->opts.plpid = strtoul(argv[++i], NULL, 0);
    }
    else if (!strcmp(argv[i], "-dvbt2profile")) {
        settings->opts.dvbt2Profile = lookup(g_dvbt2ProfileStrs, argv[++i]);
    }
    else if (!strcmp(argv[i], "-tone")) {
        settings->opts.toneEnabled = lookup(g_diseqcToneEnabledStrs, argv[++i]);
    }
    else if (!strcmp(argv[i], "-networkspec")) {
        settings->opts.satNetworkSpec = lookup(g_satNetworkSpecStrs, argv[++i]);
    }
    else if (!strcmp(argv[i], "-kufreq")) {
        unsigned freq = strtoul(argv[++i], NULL, 0);
        if (freq < 11700) {
            /* low band */
            settings->opts.freq = freq - 9750;
            settings->opts.toneEnabled = 0;
        }
        else {
            /* high band */
            settings->opts.freq = freq - 10600;
            settings->opts.toneEnabled = 1;
        }
    }
    else {
        return 0;
    }
    *argc = i;
    return 1;
}
#endif

int
frontend_getStrength(FrontendSettings *settings, int *pStrength, unsigned *pLevelPercent, unsigned *pQualityPercent)
{
#if NEXUS_HAS_FRONTEND
    switch (settings->source) {
    case FrontendSource_eOfdm: {
        NEXUS_FrontendOfdmStatus status;
        BKNI_ResetEvent(ofdmStatusEvent);
        if (NEXUS_Frontend_RequestOfdmAsyncStatus(settings->handle) != NEXUS_SUCCESS) {
            BDBG_ERR(("Request"));
            return 0;
        }
        if (BKNI_WaitForEvent(ofdmStatusEvent, 5000) == NEXUS_TIMEOUT) {
            BDBG_ERR(("Timed out"));
            return 0;
        }
        if (NEXUS_Frontend_GetOfdmAsyncStatus(settings->handle, &status) != NEXUS_SUCCESS) {
            BDBG_ERR(("Get"));
        }
        *pStrength = status.signalStrength;
        *pLevelPercent = status.signalLevelPercent;
        *pQualityPercent = status.signalQualityPercent;
        return 1;
    }
    default:
        break;
    }
#else
    BSTD_UNUSED(settings);
    BSTD_UNUSED(pStrength);
    BSTD_UNUSED(pLevelPercent);
    BSTD_UNUSED(pQualityPercent);
#endif
    return 0;
}

void
frontend_init(FrontendSettings *settings)
{
    settings->source = FrontendSource_eMax;
#if NEXUS_HAS_FRONTEND
    settings->handle = 0;
    cmdline_register_module(parse, usage, settings);
    settings->opts.vsbMode = NEXUS_FrontendVsbMode_eMax;
    settings->opts.qamMode = NEXUS_FrontendQamMode_eMax;
    settings->opts.satMode = NEXUS_FrontendSatelliteMode_eMax;
    settings->opts.toneEnabled = -1;
    settings->opts.satNetworkSpec = NEXUS_FrontendSatelliteNetworkSpec_eDefault;
    settings->opts.diseqcVoltage = NEXUS_FrontendDiseqcVoltage_eMax;
    settings->opts.bandwidth = 0;
    settings->opts.ofdmMode = NEXUS_FrontendOfdmMode_eMax;
    settings->opts.plpid = -1;
    settings->opts.dvbt2Profile = NEXUS_FrontendDvbt2Profile_eMax;
#if NEXUS_USE_7445_DBS
    settings->opts.adc = 1;
#else
    settings->opts.adc = -1;
#endif
    BKNI_CreateEvent(&lockEvent);
    BKNI_CreateEvent(&ofdmStatusEvent);
#endif
}

void
frontend_shutdown(FrontendSettings *settings)
{
    BSTD_UNUSED(settings);
#if NEXUS_HAS_FRONTEND
    BKNI_DestroyEvent(lockEvent);
    BKNI_DestroyEvent(ofdmStatusEvent);
#endif
}
