/******************************************************************************
 *  Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

/* Example to tune a VSB channel using nexus */

#include "nexus_frontend.h"
#include "nexus_platform.h"
#include "nexus_parser_band.h"
#include "nexus_video_decoder.h"
#include "nexus_stc_channel.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_video_input.h"
#include "nexus_audio_dac.h"
#include "nexus_audio_decoder.h"
#include "nexus_audio_output.h"
#include "nexus_audio_input.h"
#include "nexus_platform_frontend_power_management.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>


static int channel_frequency[3] = {429, 435, 441};
static int power_status = 0;    /*0 normal; 1 standby*/


static void lock_callback(void *context, int param)
{
    NEXUS_FrontendHandle frontend = (NEXUS_FrontendHandle)context;
    NEXUS_FrontendQamStatus qamStatus;

    BSTD_UNUSED(param);

    fprintf(stderr, "Lock callback, frontend 0x%08x\n", (unsigned)frontend);

#if 0
    NEXUS_Frontend_GetQamStatus(frontend, &qamStatus);
    fprintf(stderr, "QAM Lock callback, frontend 0x%08x - lock status %d, %d\n", (unsigned)frontend,
            qamStatus.fecLock, qamStatus.receiverLock);

#endif

}

int main(int argc, char **argv)
{
    NEXUS_FrontendUserParameters userParams;
    NEXUS_FrontendHandle frontend=NULL;
    NEXUS_FrontendQamSettings qamSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_ParserBand parserBand;
    NEXUS_ParserBandSettings parserBandSettings;
    NEXUS_StcChannelHandle stcChannel;
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_DisplayHandle display;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_VideoWindowHandle window;
    NEXUS_VideoDecoderHandle videoDecoder;
    NEXUS_AudioDecoderHandle audioDecoder;
    unsigned i;
    /* default freq & qam mode */
    int channel_number = 0;
    unsigned freq = channel_frequency[channel_number];
    NEXUS_FrontendQamMode qamMode = NEXUS_FrontendQamMode_e64;
    char input_char[8];
    int standby_frontend_number, normal_frontend_number;


    /* allow cmdline standby frontend number, normal status the frist operated frontend number & qam mode for simple test */
    if (argc > 2) {
        normal_frontend_number = atoi(argv[1]);
        standby_frontend_number = atoi(argv[2]);
    }else{
        printf("Please specify the first frontend number working under normal status and the one under active standby status:\n ");
        printf("Normal: DS2 - 0, DS1 - 1, DS3 - 2, DS0 - 3\nStandby: DS2 - 0, DS3 - 1, DS1 - 2, DS0 - 3\n");
        printf("Defualt is 0 0, press any key to continue\n");
        getchar();
        normal_frontend_number = 0;
        standby_frontend_number = 0;
    }

    NEXUS_Platform_Init(NULL);
    NEXUS_Platform_GetConfiguration(&platformConfig);

#if 0
    for ( i = 0; i < NEXUS_MAX_FRONTENDS; i++ )
    {
        NEXUS_FrontendCapabilities capabilities;
        frontend = platformConfig.frontend[i];
        if (frontend) {
            NEXUS_Frontend_GetCapabilities(frontend, &capabilities);
            /* Does this frontend support qam? */
            if ( capabilities.qam )
            {
                break;
            }
        }
    }
#else
	frontend = platformConfig.frontend[normal_frontend_number/*0*/];
#endif
    if (NULL == frontend )
    {
        fprintf(stderr, "Unable to find QAM-capable frontend\n");
        return 0;
    }

    NEXUS_Frontend_GetDefaultQamSettings(&qamSettings);
    qamSettings.frequency = freq * 1000000;
    qamSettings.mode = qamMode;
    qamSettings.annex = NEXUS_FrontendQamAnnex_eB;
    qamSettings.lockCallback.callback = lock_callback;
    qamSettings.lockCallback.context = frontend;

    NEXUS_Frontend_GetUserParameters(frontend, &userParams);

    /* Map a parser band to the demod's input band. */
    parserBand = NEXUS_ParserBand_e0;
    NEXUS_ParserBand_GetSettings(parserBand, &parserBandSettings);
    parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
    parserBandSettings.sourceTypeSettings.inputBand = userParams.param1;  /* Platform initializes this to input band */
    parserBandSettings.transportType = NEXUS_TransportType_eTs;
    NEXUS_ParserBand_SetSettings(parserBand, &parserBandSettings);

    NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;
    stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);

    audioDecoder = NEXUS_AudioDecoder_Open(0, NULL);
    NEXUS_AudioOutput_AddInput(
    NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]),
    NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));

    NEXUS_Display_GetDefaultSettings(&displaySettings);
    displaySettings.format = NEXUS_VideoFormat_eNtsc;
    display = NEXUS_Display_Open(0, &displaySettings);
    NEXUS_Display_AddOutput(display, NEXUS_ComponentOutput_GetConnector(platformConfig.outputs.component[0]));

    window = NEXUS_VideoWindow_Open(display, 0);
    videoDecoder = NEXUS_VideoDecoder_Open(0, NULL);
    NEXUS_VideoWindow_AddInput(window, NEXUS_VideoDecoder_GetConnector(videoDecoder));

    while (1) {
        NEXUS_PidChannelHandle videoPidChannel, audioPidChannel;
        NEXUS_VideoDecoderStartSettings videoProgram;
        NEXUS_AudioDecoderStartSettings audioProgram;


        NEXUS_Frontend_GetUserParameters(frontend, &userParams);
        NEXUS_ParserBand_GetSettings(parserBand, &parserBandSettings);
        parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
        parserBandSettings.sourceTypeSettings.inputBand = userParams.param1;  /* Platform initializes this to input band */
        printf("Parseband's input band is %d\n", userParams.param1);
        parserBandSettings.transportType = NEXUS_TransportType_eTs;
        NEXUS_ParserBand_SetSettings(parserBand, &parserBandSettings);


        videoPidChannel = NEXUS_PidChannel_Open(parserBand, 0x11, NULL);
        audioPidChannel = NEXUS_PidChannel_Open(parserBand, 0x14, NULL);

        NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
        videoProgram.codec = NEXUS_VideoCodec_eMpeg2;
        videoProgram.pidChannel = videoPidChannel;
        videoProgram.stcChannel = stcChannel;
        NEXUS_AudioDecoder_GetDefaultStartSettings(&audioProgram);
        audioProgram.codec = NEXUS_AudioCodec_eAc3;
        audioProgram.pidChannel = audioPidChannel;
        audioProgram.stcChannel = stcChannel;

        NEXUS_StcChannel_GetSettings(stcChannel, &stcSettings);
        stcSettings.mode = NEXUS_StcChannelMode_ePcr; /* live */
        stcSettings.modeSettings.pcr.pidChannel = videoPidChannel; /* PCR happens to be on video pid */
        NEXUS_StcChannel_SetSettings(stcChannel, &stcSettings);

        channel_number = channel_number % 3;
        printf("Start to decode channel %d, frontend 0x%x, callback frontend 0x%x\n", channel_number, frontend, qamSettings.lockCallback.context);
        qamSettings.frequency = channel_frequency[channel_number] * 1000000;
        NEXUS_Frontend_TuneQam(frontend, &qamSettings);

        NEXUS_VideoDecoder_Start(videoDecoder, &videoProgram);
        NEXUS_AudioDecoder_Start(audioDecoder, &audioProgram);

input_again:

        printf("Enter s to goto standby; n to goto normal status, any others to change channel:\n");
        scanf("%s", &input_char);
        printf("the input is %c\n", input_char[0]);

        switch(input_char[0]){
            case 'n':
				printf("Processing cmd to goto normal operation start\n");
				{
					NEXUS_PlatformConfiguration platformConfig;
					bool isOperational = false;
					NEXUS_Error rc = NEXUS_NOT_SUPPORTED;
					NEXUS_FrontendHandle * pControlledFrontend;

					if(power_status == 0)
					{
						printf("Already under normal operation status, do nothing\n"); goto input_again;
					}

					{
						NEXUS_PlatformFrontendStandbyMode mode;
						NEXUS_Platform_GetFrontendStandbyMode(&mode);

						if(mode != NEXUS_PlatformFrontendStandbyMode_eFullPower)
						{
                           if(standby_frontend_number == 0 || standby_frontend_number == 1)
                           {
								NEXUS_Frontend_3255_PowerUpFrontendDevices(platformConfig.frontend[0]);
								printf("Will power up frontend devices\n");
                           }
                           else
                               printf("Won't power up frontend devices\n");
						NEXUS_Platform_WakeupCM();
						}

						if(1){
							printf("\n Do system command call here to insmod bcm\n");
							system("insmod /root/bcmscbeth.ko");
							BKNI_Sleep(1000);
							system("ifconfig bcm0:0 192.168.17.10");
							system("/root/rnonvolhost bcm0 192.168.17.10 192.168.17.1 /root/p.bin /root/d.bin &");
						}else
							BKNI_Sleep(500);
					}
					NEXUS_Platform_SetFrontendStandbyMode(NEXUS_PlatformFrontendStandbyMode_eFullPower);
					/*to this point the BNM is really waken up and operational, after this we will transfter the control to BNM*/
					NEXUS_Platform_Frontend_DisableHostControl();
					NEXUS_Platform_GetConfiguration(&platformConfig);
					frontend = NULL;
					NEXUS_Platform_Frontend_GetFrontendInstanceControlled(&frontend, normal_frontend_number);
					printf("\n set back 1 BNM controlled frontend to trinity tuner, frontend 0x%x\n", frontend);
					qamSettings.lockCallback.context = frontend;
					printf("\n set back 2 qamSettings.lockCallback.context, frontend 0x%x\n", qamSettings.lockCallback.context);
					power_status = 0;
				}
				printf("Processing cmd to goto standby end\n");
                    goto input_again;
                break;
            case 's':
				printf("Processing cmd to goto standby start\n");
				{

					NEXUS_PlatformConfiguration platformConfig;
					bool isOperational = false;
					NEXUS_Error rc = NEXUS_NOT_SUPPORTED;
					NEXUS_FrontendHandle * pControlledFrontend;

					if(power_status == 1) {
                                printf("Already under normal status, do nothing\n"); goto input_again;
                            }

					NEXUS_Platform_Frontend_EnableHostControl();
					NEXUS_Platform_GetConfiguration(&platformConfig);

					NEXUS_Platform_SetFrontendStandbyMode(NEXUS_PlatformFrontendStandbyMode_eActive);
					{
						if(1){
							printf("\n Do system command call here to kill nonvol host and rmmod bcm\n");
							system("killall rnonvolhost");
							printf("\n After system command killall rnonvolhost\n");
							system("rmmod bcmscbeth.ko");
							printf("\n After system command rmmod bcmscbeth.ko\n");
						}
						NEXUS_Platform_ShutdownCM();
						if(standby_frontend_number == 0 || standby_frontend_number == 1){
							NEXUS_Frontend_3255_PowerDownFrontendDevices(platformConfig.frontend[0]);
							printf("Will power down frontend devices\n");
						}
						else
							printf("Won't power down frontend devices\n");
					}
					frontend = NULL;
					NEXUS_Platform_Frontend_GetFrontendInstanceControlled(&frontend, standby_frontend_number);
					printf("\n set host controlled frontend to trinity tuner, frontend 0x%x\n", frontend);
					qamSettings.lockCallback.context = frontend;
					power_status = 1;
				}
				printf("Processing cmd to goto standby end\n");
			goto input_again;
			break;


             default:
                printf("Change channel:\n");
                break;

        }


        NEXUS_AudioDecoder_Stop(audioDecoder);
        NEXUS_VideoDecoder_Stop(videoDecoder);
        NEXUS_PidChannel_Close(videoPidChannel);
        NEXUS_PidChannel_Close(audioPidChannel);

        channel_number++;

    }

    return 0;
}
