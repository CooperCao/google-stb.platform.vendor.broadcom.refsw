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
/* Example to tune a satellite channel using nexus */

#include "nexus_frontend.h"
#include "nexus_platform.h"
#include "nexus_parser_band.h"
#include "nexus_stc_channel.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_video_input.h"
#include "nexus_composite_output.h"
#include "nexus_component_output.h"
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#endif

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bsu-api.h"
#include "bsu-api2.h"

#define USE_DVB 1 /* else use DSS */

/* the following define the input and its characteristics -- these will vary by input type */
#ifdef USE_DVB
#define TRANSPORT_TYPE NEXUS_TransportType_eTs
#define VIDEO_CODEC NEXUS_VideoCodec_eMpeg2
#define AUDIO_CODEC NEXUS_AudioCodec_eAc3
#define VIDEO_PID 0x31
#define AUDIO_PID 0x34
#define FREQ 1119000000
#define SATELLITE_MODE NEXUS_FrontendSatelliteMode_eDvb
#define TONE_MODE true
#define VOLTAGE NEXUS_FrontendDiseqcVoltage_e13v
#else
#define TRANSPORT_TYPE NEXUS_TransportType_eDssEs
#define VIDEO_CODEC NEXUS_VideoCodec_eMpeg2
#define AUDIO_CODEC NEXUS_AudioCodec_eMpeg
#define VIDEO_PID 0x78
#define AUDIO_PID 0x79
#define FREQ 1396820000
#define SATELLITE_MODE NEXUS_FrontendSatelliteMode_eDss
#define TONE_MODE false
#define VOLTAGE NEXUS_FrontendDiseqcVoltage_e18v
#endif

extern bool frontend_already_opened;

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

void bsu_satellite_test(void)
{
    NEXUS_FrontendUserParameters userParams;
    NEXUS_FrontendHandle frontend=NULL;
    NEXUS_FrontendSatelliteSettings satSettings;
    NEXUS_FrontendDiseqcSettings diseqcSettings;
    NEXUS_FrontendAcquireSettings settings;
    NEXUS_ParserBand parserBand;
    NEXUS_ParserBandSettings parserBandSettings;
    NEXUS_Error rc;

    if (frontend_already_opened==false) {
        rc = NEXUS_Platform_InitFrontend();
        if(rc){rc = BERR_TRACE(rc); goto done;}
    }

    NEXUS_Frontend_GetDefaultAcquireSettings(&settings);
    settings.capabilities.satellite = true;
    frontend = NEXUS_Frontend_Acquire(&settings);
    if (!frontend) {
        fprintf(stderr, "Unable to find satellite-capable frontend\n");
        return;
    }

    NEXUS_Frontend_GetDefaultSatelliteSettings(&satSettings);
    satSettings.frequency = FREQ;
    satSettings.mode = SATELLITE_MODE;
    satSettings.lockCallback.callback = lock_callback;
    satSettings.lockCallback.context = frontend;
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
    parserBandSettings.transportType = TRANSPORT_TYPE;
    NEXUS_ParserBand_SetSettings(parserBand, &parserBandSettings);

    NEXUS_Frontend_GetDiseqcSettings(frontend, &diseqcSettings);
    diseqcSettings.toneEnabled = TONE_MODE;
    diseqcSettings.voltage = VOLTAGE;
    NEXUS_Frontend_SetDiseqcSettings(frontend, &diseqcSettings);
    printf("Set DiseqcSettings\n");

    rc = NEXUS_Frontend_TuneSatellite(frontend, &satSettings);
    BDBG_ASSERT(!rc);

    while (1)
    {
        NEXUS_FrontendSatelliteStatus satStatus;
        rc = NEXUS_Frontend_GetSatelliteStatus(frontend, &satStatus);
        if (rc) {
            printf("unable to read status\n");
        }
        else {
            printf(
                "Sat Status\n"
                "  symbolRate %d\n"
                "  locked  %d\n",
                    satStatus.settings.symbolRate,
                    satStatus.demodLocked);
            if(satStatus.demodLocked)
                break;
        }
        BKNI_Sleep(1000);
    }

done:
    return;
}
