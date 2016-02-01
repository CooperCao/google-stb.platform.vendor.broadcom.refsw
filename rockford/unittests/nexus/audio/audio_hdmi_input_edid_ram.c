/***************************************************************************
*     (c)2004-2014 Broadcom Corporation
*
*  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
* $brcm_Workfile: $
* $brcm_Revision: $
* $brcm_Date: $
*
* API Description:
*   API name: Platform
*    Specific APIs to initialze the a board.
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/

#include "nexus_platform.h"
#include "nexus_pid_channel.h"
#include "nexus_stc_channel.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_composite_output.h"
#include "nexus_component_output.h"
#include "nexus_video_input.h"
#if NEXUS_NUM_HDMI_INPUTS
#include "nexus_hdmi_input.h"
#endif
#include "nexus_audio_input.h"
#include "nexus_audio_decoder.h"
#include "nexus_audio_dac.h"
#include "nexus_audio_output.h"
#include "bstd.h"
#include "bkni.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if NEXUS_DTV_PLATFORM
#include "nexus_panel_output.h"
#include "nexus_platform_boardcfg.h"
#endif

BDBG_MODULE(audio_hdmi_input_edid_ram);

#if NEXUS_NUM_HDMI_INPUTS

static uint8_t SampleEDID[] = 
{
/*
 *          Gary's EDID with Full Audio Support
 * 
 *   Created by the Lightware Matrix Controller 3.3.4 EDID Editor
 *     Available from http://lightwareusa.com/support-/downloads/668-matrix-controller-software
 * 
 *   This EDID indicates the following audio support:
 *   Supported audio formats:
 *
 *            * LPCM 8 Ch
 *                  192kHz 176.4kHz 96kHz 88.2kHz 48kHz 44.1kHz 32kHz 
 *            * AC-3 8 Ch
 *                  192kHz 176.4kHz 96kHz 88.2kHz 48kHz 44.1kHz 32kHz 
 *            * AAC 8 Ch
 *                  192kHz 176.4kHz 96kHz 88.2kHz 48kHz 44.1kHz 32kHz 
 *            * DTS 8 Ch
 *                  192kHz 176.4kHz 96kHz 88.2kHz 48kHz 44.1kHz 32kHz 
 *            * MPEG1 2 Ch
 *                  48kHz 44.1kHz 32kHz 
 *            * MP3 2 Ch
 *                  48kHz 44.1kHz 32kHz 
 *            * Dolby + 8 Ch
 *                  48kHz 44.1kHz 32kHz 
 *            * DTS-HD 8 Ch
 *                  192kHz 176.4kHz 96kHz 88.2kHz 48kHz 44.1kHz 32kHz 
 *            * MLP 8 Ch
 *                  192kHz 176.4kHz 96kHz 88.2kHz 48kHz 44.1kHz 32kHz 
 * 
 * 
 *   Available speakers
 *         
 *         * Front Center
 *         * Front Left,        Front Right
 *         * Rear Left,         Rear Right
 *         * Rear Left Center,  Rear Right Center
 *         * Low Frequency Effect
 *
 */

/*          0   1   2   3   4   5   6   7   8   9  */
/*       ________________________________________  */
/*    0  | */  0x00,  0xFF,  0xFF,  0xFF,  0xFF,  0xFF,  0xFF,  0x00,  0x34,  0xA9,
/*   10  | */  0xAF,  0xA0,  0x01,  0x01,  0x01,  0x01,  0x00,  0x14,  0x01,  0x03,
/*   20  | */  0x80,  0x00,  0x00,  0x78,  0x0A,  0xDA,  0xFF,  0xA3,  0x58,  0x4A,
/*   30  | */  0xA2,  0x29,  0x17,  0x49,  0x4B,  0x00,  0x00,  0x00,  0x01,  0x01,
/*   40  | */  0x01,  0x01,  0x01,  0x01,  0x01,  0x01,  0x01,  0x01,  0x01,  0x01,
/*   50  | */  0x01,  0x01,  0x01,  0x01,  0x02,  0x3A,  0x80,  0x18,  0x71,  0x38,
/*   60  | */  0x2D,  0x40,  0x58,  0x2C,  0x45,  0x00,  0xBA,  0x88,  0x21,  0x00,
/*   70  | */  0x00,  0x1E,  0x01,  0x1D,  0x80,  0x18,  0x71,  0x1C,  0x16,  0x20,
/*   80  | */  0x58,  0x2C,  0x25,  0x00,  0xBA,  0x88,  0x21,  0x00,  0x00,  0x9E,
/*   90  | */  0x00,  0x00,  0x00,  0xFC,  0x00,  0x50,  0x61,  0x6E,  0x61,  0x73,
/*  100  | */  0x6F,  0x6E,  0x69,  0x63,  0x54,  0x56,  0x30,  0x0A,  0x00,  0x00,
/*  110  | */  0x00,  0xFD,  0x00,  0x17,  0x3D,  0x0F,  0x44,  0x0F,  0x00,  0x0A,
/*  120  | */  0x20,  0x20,  0x20,  0x20,  0x20,  0x20,  0x01,  0xC7,  0x02,  0x03,
/*  130  | */  0x3A,  0x70,  0x49,  0x90,  0x05,  0x20,  0x04,  0x03,  0x02,  0x07,
/*  140  | */  0x06,  0x01,  0x3B,  0x17,  0x7F,  0x18,  0x37,  0x7F,  0x18,  0x3F,
/*  150  | */  0x7F,  0x18,  0x0F,  0x7F,  0x40,  0x19,  0x07,  0x28,  0x21,  0x07,
/*  160  | */  0x28,  0x57,  0x07,  0x00,  0x5F,  0x7F,  0x00,  0x67,  0x7F,  0x00,
/*  170  | */  0x83,  0x4F,  0x00,  0x00,  0x67,  0x03,  0x0C,  0x00,  0x40,  0x00,
/*  180  | */  0xB8,  0x2D,  0xE3,  0x05,  0x1F,  0x01,  0x00,  0x00,  0x00,  0x00,
/*  190  | */  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,
/*  200  | */  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,
/*  210  | */  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,
/*  220  | */  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,
/*  230  | */  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,
/*  240  | */  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,
/*  250  | */  0x00,  0x00,  0x00,  0x00,  0xF1,  0xC1

};


void source_changed(void *context, int param)
{
    BSTD_UNUSED(context);
    BSTD_UNUSED(param);
    printf("source changed\n");
}

void avmute_changed(void *context, int param)
{
    NEXUS_HdmiInputHandle hdmiInput ;
    NEXUS_HdmiInputStatus hdmiInputStatus ;
    BSTD_UNUSED(param);

    hdmiInput = (NEXUS_HdmiInputHandle) context ;
    NEXUS_HdmiInput_GetStatus(hdmiInput, &hdmiInputStatus) ;

    if (!hdmiInputStatus.validHdmiStatus)
    {
        printf("avmute_changed callback: Unable to get hdmiInput status\n") ;
    }
    else
    {
        printf("avmute_changed callback: %s\n", 
            hdmiInputStatus.avMute ? "Set_AvMute" : "Clear_AvMute") ;
    }
}
#endif

int main(int argc, char **argv)
{
#if NEXUS_HAS_HDMI_INPUT
    NEXUS_DisplayHandle display;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_VideoWindowHandle window;
    NEXUS_HdmiInputHandle hdmiInput;
    NEXUS_HdmiInputSettings hdmiInputSettings;
    NEXUS_TimebaseSettings timebaseSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_AudioDecoderHandle audioDecoder;
    NEXUS_AudioDecoderStartSettings audioProgram;
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_PlatformSettings platformSettings ;
    
    unsigned long  testCycle;
    unsigned hdmiInputIndex = 0;
    unsigned refreshHz = 120 ;
    unsigned long  loopCount = 2000000000;  /* default is to run almost forever */
    bool equalization = true;
    
    int curarg = 1;

#if (BCHP_CHIP != 35230) && (BCHP_CHIP != 35233) && BCHP_CHIP != 7425
    refreshHz = 60 ;
#else
    refreshHz = 120 ;
#endif


    while (curarg < argc) {
        if (!strcmp(argv[curarg], "-sixty")) {
            refreshHz = 60 ;
        }
        else if (!strcmp(argv[curarg], "-port")) {
            hdmiInputIndex = atoi(argv[++curarg]) ;
        }
        else if (!strcmp(argv[curarg], "-no_equalization")) {
            equalization = false;
        }
        else if (!strcmp(argv[curarg], "-loop")) {
            loopCount = atoi(argv[++curarg]) ;
        }
        curarg++;
    }

    /* Bring up all modules for a platform in a default configuration for this platform */

    NEXUS_Platform_GetDefaultSettings(&platformSettings);

#if (BCHP_CHIP == 35230) ||  (BCHP_CHIP  == 35125) || BCHP_CHIP == 35233 || BCHP_CHIP == 35126
    /* assume 120 unless overwritten */ 
    platformSettings.displayModuleSettings.panel.lvds.dvoLinkMode = NEXUS_PanelOutputLinkMode_eQuad;                    
    platformSettings.displayModuleSettings.panel.lvds.lvdsColorMode = NEXUS_LvdsColorMode_e10Bit ;
    if (refreshHz == 60)
    {
        platformSettings.displayModuleSettings.panel.lvds.dvoLinkMode = NEXUS_PanelOutputLinkMode_eDualChannel1;                    
        platformSettings.displayModuleSettings.panel.lvds.lvdsColorMode = NEXUS_LvdsColorMode_e8Bit ;
    }

    fprintf(stderr, "hdmiInput Port: %d\n", hdmiInputIndex) ;
    fprintf(stderr, "Display Refresh Rate : %d\n", refreshHz) ;
    fprintf(stderr, "dvoLinkMode : %d\n", 
		platformSettings.displayModuleSettings.panel.lvds.dvoLinkMode) ;
    fprintf(stderr, "Exit after : %d seconds\n", loopCount) ;
#endif  
    
    
    NEXUS_Platform_Init(&platformSettings); 

    NEXUS_Platform_GetConfiguration(&platformConfig);

    NEXUS_Timebase_GetSettings(NEXUS_Timebase_e0, &timebaseSettings);
    timebaseSettings.sourceType = NEXUS_TimebaseSourceType_eHdDviIn;
    NEXUS_Timebase_SetSettings(NEXUS_Timebase_e0, &timebaseSettings);

    NEXUS_HdmiInput_GetDefaultSettings(&hdmiInputSettings);

    /* set hpdDisconnected to true if a HDMI switch is in front of the Broadcom HDMI Rx.  
    -- The NEXUS_HdmiInput_ConfigureAfterHotPlug should be called to inform the hw of 
    -- the current state,  the Broadcom SV reference boards have no switch so 
    -- the value should always be false 
    */
    hdmiInputSettings.frontend.hpdDisconnected = false ;
    hdmiInputSettings.frontend.equalizationEnabled = equalization ;
    
    hdmiInputSettings.timebase = NEXUS_Timebase_e0;
    hdmiInputSettings.sourceChanged.callback = source_changed;

    hdmiInputSettings.useInternalEdid = true ;
    hdmiInput = NEXUS_HdmiInput_OpenWithEdid(hdmiInputIndex, &hdmiInputSettings, 
        &SampleEDID[0], (uint16_t) sizeof(SampleEDID));

    if (!hdmiInput) 
    {
        fprintf(stderr, "Can't get hdmi input\n");
        return -1;
    }
    NEXUS_HdmiInput_GetSettings(hdmiInput, &hdmiInputSettings) ;
    hdmiInputSettings.avMuteChanged.callback = avmute_changed;
    hdmiInputSettings.avMuteChanged.context = hdmiInput ;
    NEXUS_HdmiInput_SetSettings(hdmiInput, &hdmiInputSettings) ;

#if NEXUS_DTV_PLATFORM
    /* bring up display */
    NEXUS_Display_GetDefaultSettings(&displaySettings);
    displaySettings.format = NEXUS_VideoFormat_e1080p;
    displaySettings.background = 0xFF00FF00;
    display = NEXUS_Display_Open(0, &displaySettings);
    NEXUS_Display_AddOutput(display, NEXUS_PanelOutput_GetConnector(platformConfig.outputs.panel[0]));
    NEXUS_BoardCfg_ConfigurePanel(true, true, true);
    fprintf(stderr, "Panel output ready\n");
#else
    NEXUS_Display_GetDefaultSettings(&displaySettings);
    displaySettings.format = NEXUS_VideoFormat_e720p;
    displaySettings.background = 0xFF00FF00;
    display = NEXUS_Display_Open(0, &displaySettings);
    NEXUS_Display_AddOutput(display, NEXUS_ComponentOutput_GetConnector(platformConfig.outputs.component[0]));
#endif

    audioDecoder = NEXUS_AudioDecoder_Open(0, NULL);
    NEXUS_AudioDecoder_GetDefaultStartSettings(&audioProgram);
    audioProgram.input = NEXUS_HdmiInput_GetAudioConnector(hdmiInput);
    NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;
    stcSettings.autoConfigTimebase = false;
    audioProgram.stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);
#if NEXUS_NUM_AUDIO_DACS
    NEXUS_AudioOutput_AddInput(NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]),
                               NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
#endif
#if NEXUS_NUM_SPDIF_OUTPUTS
    NEXUS_AudioOutput_AddInput(NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]),
                               NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eCompressed));
#endif

    window = NEXUS_VideoWindow_Open(display, 0);
    NEXUS_VideoWindow_AddInput(window, NEXUS_HdmiInput_GetVideoConnector(hdmiInput));

    for (testCycle=0 ; testCycle<loopCount ; testCycle++)
    {
        NEXUS_HdmiInputStatus status, previous_status ;
        unsigned long       seconds = 0;
        BERR_Code       errCode;


        BDBG_LOG(("Starting Audio Decoder... "));
        errCode = NEXUS_AudioDecoder_Start(audioDecoder, &audioProgram);
        if ( errCode ) return BERR_TRACE(errCode);

        BDBG_LOG((""));
        BDBG_LOG(("Beginning Test Cycle %lu... ", testCycle));
        BDBG_LOG((""));

        for (seconds=0 ; seconds<9999999  ; seconds++)
        {

            NEXUS_HdmiInput_GetStatus(hdmiInput, &status);

            if (!BKNI_Memcmp(&status, &previous_status, sizeof (NEXUS_HdmiInputStatus)))
            {

                printf(
                       "HdmiInput Status\n"
                       "  original format: %d\n"
                       "  interlaced:      %c\n"
                       "  noSignal:        %c\n",
                       status.originalFormat,
                       status.interlaced?'y':'n',
                       status.noSignal?'y':'n');
                previous_status = status ;
            }
            BKNI_Sleep(1000);
        }

        /* printf("Press <Enter>...");   getchar(); */

        BDBG_LOG(("Stopping Audio Decoder... "));
        NEXUS_AudioDecoder_Stop(audioDecoder) ;

        #ifdef  BCHP_PWR_SUPPORT
        {
                NEXUS_PlatformStandbySettings nexusStandbySettings;

                NEXUS_Platform_GetStandbySettings(&nexusStandbySettings);
                nexusStandbySettings.mode = NEXUS_PlatformStandbyMode_eActive;
                BDBG_LOG(("Going to Active Standby (StandbyMode: NEXUS_PlatformStandbyMode_eActive)"));
                errCode = NEXUS_Platform_SetStandbySettings(&nexusStandbySettings);
                if ( errCode ) return BERR_TRACE(errCode);
        }
        #endif /*  BCHP_PWR_SUPPORT */

        BDBG_LOG(("Sleeping..."));

        BKNI_Sleep(2000);

        #ifdef  BCHP_PWR_SUPPORT
        {
            NEXUS_PlatformStandbySettings nexusStandbySettings;

            NEXUS_Platform_GetStandbySettings(&nexusStandbySettings);
            nexusStandbySettings.mode = NEXUS_PlatformStandbyMode_eOn;
            BDBG_LOG(("Resuming from Standby (StandbyMode: NEXUS_PlatformStandbyMode_eOn)"));
            errCode = NEXUS_Platform_SetStandbySettings(&nexusStandbySettings);
            if ( errCode ) return BERR_TRACE(errCode);
        }
        #endif /*  BCHP_PWR_SUPPORT */

        /* printf("Press <Enter>...");   getchar(); */
    }

    NEXUS_VideoWindow_RemoveInput(window, NEXUS_HdmiInput_GetVideoConnector(hdmiInput));

    NEXUS_VideoWindow_Close(window);

#if NEXUS_NUM_AUDIO_DACS
    NEXUS_AudioOutput_RemoveInput(NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]),
                   NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
#endif
#if NEXUS_NUM_SPDIF_OUTPUTS
    NEXUS_AudioOutput_RemoveInput(NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]),
                   NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eCompressed));
#endif
    NEXUS_AudioInput_Shutdown(NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
    NEXUS_AudioInput_Shutdown(NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eCompressed));
    NEXUS_AudioInput_Shutdown(NEXUS_HdmiInput_GetAudioConnector(hdmiInput));

#if NEXUS_DTV_PLATFORM
    NEXUS_Display_RemoveOutput(display, 
        NEXUS_PanelOutput_GetConnector(platformConfig.outputs.panel[0])) ;

    NEXUS_VideoInput_Shutdown(NEXUS_HdmiInput_GetVideoConnector(hdmiInput)) ;
#endif

    NEXUS_HdmiInput_Close(hdmiInput) ;  
    
#if NEXUS_DTV_PLATFORM
    NEXUS_Display_Close(display);
#endif

    NEXUS_Platform_Uninit(); 

#else
    BSTD_UNUSED(argc);
    printf("%s not supported on this chip", argv[0]) ;
#endif
    return 0;
}

