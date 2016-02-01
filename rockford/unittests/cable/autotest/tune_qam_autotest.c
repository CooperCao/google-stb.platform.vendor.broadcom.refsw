/******************************************************************************
 *    (c)2008-2013 Broadcom Corporation
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
/* Example to tune a QAM channel using nexus */

#if NEXUS_HAS_FRONTEND
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
#include "nexus_composite_output.h"
#include "nexus_component_output.h"
#include "nexus_core_utils.h"
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#endif

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

BDBG_MODULE(tune_qam);

/* the following define the input and its characteristics -- these will vary by input stream */
#define VIDEO_PID 0x11
#define AUDIO_PID 0x14

static void lock_changed_callback(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
	/* for autotest, frontend qam locked */ 
    BDBG_WRN(("autotest - Qam Locked Successfully!"));
}

static void async_status_ready_callback(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
}

#include <sys/time.h>
static unsigned b_get_time(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec*1000 + tv.tv_usec/1000;
}

static void print_usage(void)
{
    printf(
        "Usage: tune_ofdm\n"
        "  --help or -h for help\n"
        "  -mode {0,1}   QAM-64=0, QAM-256=1 (default is QAM-64)\n"
        "  -freq   #     frequency in MHz\n"
        "  -vpid   #     video PID\n"
        "  -apid   #     audio PID\n"
        "  -pcrpid #     PCR PID\n"
        "  -vcodec       MPEG2=2, AVC=5 (default is MPEG2)\n"
        "  -acodec       MPEG=1, AAC=3, AACplus=5, AC3=7 (default is AC3)\n"
        );
}

static NEXUS_Error get_status(NEXUS_FrontendHandle frontend, BKNI_EventHandle statusEvent);

int main(int argc, char **argv)
{
    NEXUS_PlatformSettings platformSettings;
    NEXUS_FrontendUserParameters userParams;
    NEXUS_FrontendHandle frontend = NULL;
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
    NEXUS_SurfaceHandle surface;
    NEXUS_SurfaceCreateSettings createSettings;
    NEXUS_VideoFormatInfo videoFormatInfo;
    NEXUS_SurfaceMemory mem;
    NEXUS_GraphicsSettings graphicsSettings;
    NEXUS_Error rc;
    /* default freq & qam mode */
    unsigned freq[4] = {429,435,441,447};
    BKNI_EventHandle statusEvent, lockChangedEvent;
    bool done = false;
    int curarg = 1;
    unsigned  mode = 64;
    unsigned int videoCodec = NEXUS_VideoCodec_eMpeg2;
    unsigned int audioCodec = NEXUS_AudioCodec_eAc3;
    int videoPid = 17, audioPid = 20, pcrPid = -1;
    int successVideoChannel = 0;
    unsigned ii;

    /* Read command line for freqency, pid and codec information */

    while (curarg < argc) {
        if (!strcmp(argv[curarg], "-h") || !strcmp(argv[curarg], "--help")) {
            print_usage();
            return 0;
        }
        else if (!strcmp(argv[curarg], "-mode")) {
            mode = atoi(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-freq") && argc>curarg+1) {
            freq[0] = atoi(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-vpid")) {
            if((!strncmp(argv[++curarg], "0X", 2)) || (!strncmp(argv[curarg], "0x", 2))){
                videoPid = strtol(argv[curarg], NULL, 0);
            }
            else{
                videoPid = atoi(argv[curarg]);
            }
        }
        else if (!strcmp(argv[curarg], "-apid")) {
            if((!strncmp(argv[++curarg], "0X", 2)) || (!strncmp(argv[curarg], "0x", 2))){
                audioPid = strtol(argv[curarg], NULL, 0);
            }
            else{
                audioPid = atoi(argv[curarg]);
            }
        }
        else if (!strcmp(argv[curarg], "-pcrpid")) {
            if((!strncmp(argv[++curarg], "0X", 2)) || (!strncmp(argv[curarg], "0x", 2))){
                pcrPid = strtol(argv[curarg], NULL, 0);
            }
            else{
                pcrPid = atoi(argv[curarg]);
            }
        }
        else if (!strcmp(argv[curarg], "-vcodec")) {
            videoCodec = atoi(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-acodec")) {
            audioCodec = atoi(argv[++curarg]);
        }
        else {
            print_usage();
            return 1;
        }
        curarg++;
    }
    /* If the PCR PID hasn't been set but the video PID has then assume PCR is on the video */
    if ((pcrPid == -1) && (videoPid != -1)) {
        pcrPid = videoPid;
    }

    /* don't let NEXUS_Platform_Init open frontend because it is very slow.
    we need to get the display open and a splash screen up first. then we can open the frontend. */
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    platformSettings.videoDecoderModuleSettings.deferInit = true;
#if NEXUS_HAS_VIDEO_ENCODER
    platformSettings.videoEncoderSettings.deferInit = true;
#endif
    rc = NEXUS_Platform_Init(&platformSettings);
    if (rc) return -1;

	/* for autotest, platform initilized */ 
    BDBG_WRN(("autotest - Nexus Platform Initialized Successfully!"));
	
    /* bring up display first for splash screen */
    NEXUS_Platform_GetConfiguration(&platformConfig);
    NEXUS_Display_GetDefaultSettings(&displaySettings);
    displaySettings.format = NEXUS_VideoFormat_eNtsc;
    display = NEXUS_Display_Open(0, &displaySettings);
#if NEXUS_NUM_COMPOSITE_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_CompositeOutput_GetConnector(platformConfig.outputs.composite[0]));
#endif
#if NEXUS_NUM_COMPONENT_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_ComponentOutput_GetConnector(platformConfig.outputs.component[0]));
#endif

    NEXUS_VideoFormat_GetInfo(displaySettings.format, &videoFormatInfo);
    NEXUS_Surface_GetDefaultCreateSettings(&createSettings);
    createSettings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
    createSettings.width = 720;
    createSettings.height = videoFormatInfo.height;
    createSettings.heap = NEXUS_Platform_GetFramebufferHeap(0);
    surface = NEXUS_Surface_Create(&createSettings);
    NEXUS_Surface_GetMemory(surface, &mem);

    /* simple blue splash screen */
    {
        unsigned x, y;
        for (y=0;y<createSettings.height;y++) {
            uint32_t *ptr = (uint32_t *)((unsigned)mem.buffer + y*mem.pitch);
            for (x=0;x<createSettings.width;x++) {
                ptr[x] = 0xFF0000FF;
            }
        }
    }
    NEXUS_Surface_Flush(surface);

    NEXUS_Display_GetGraphicsSettings(display, &graphicsSettings);
    graphicsSettings.enabled = true;
    graphicsSettings.clip.width = createSettings.width;
    graphicsSettings.clip.height = createSettings.height;
    rc = NEXUS_Display_SetGraphicsSettings(display, &graphicsSettings);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    rc = NEXUS_Display_SetGraphicsFramebuffer(display, surface);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    BDBG_WRN(("splash is up"));

    /* bring up HDMI after splash because it's slower. this allows analog outputs to show something a little faster. */
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_HdmiOutput_GetVideoConnector(platformConfig.outputs.hdmi[0]));
    /* Don't check connected status. This requires more time and should be handled in a hotplug handler. */
#endif

    /* now that GUI is up, we can init frontend. for a more sophisticated app, InitFrontend could be called from a thread so that
    the main thread could update the splash GUI. */
    rc = NEXUS_Platform_InitFrontend();
    if(rc){rc = BERR_TRACE(rc); goto done;}
#if 0
    {
        NEXUS_FrontendAcquireSettings settings;
        NEXUS_Frontend_GetDefaultAcquireSettings(&settings);
        settings.capabilities.qam = true;
        frontend = NEXUS_Frontend_Acquire(&settings);
        if (!frontend) {
            BDBG_ERR(("Unable to find QAM-capable frontend"));
            return -1;
        }
    }
#endif
	/* for autotest, frontend initialized */ 
    BDBG_WRN(("autotest - Nexus Frontend Initialized Successfully!"));
	
    BKNI_CreateEvent(&statusEvent);
    BKNI_CreateEvent(&lockChangedEvent);

    NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;
    stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);

    audioDecoder = NEXUS_AudioDecoder_Open(0, NULL);
    NEXUS_AudioOutput_AddInput(
        NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]),
        NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_AudioOutput_AddInput(
        NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]),
        NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
#endif

    window = NEXUS_VideoWindow_Open(display, 0);
    videoDecoder = NEXUS_VideoDecoder_Open(0, NULL);
    NEXUS_VideoWindow_AddInput(window, NEXUS_VideoDecoder_GetConnector(videoDecoder));

#if 0
    while (!done) {
#else
    for(ii=0; ii<32; ii++) {
        NEXUS_FrontendAcquireSettings settings;
        NEXUS_Frontend_GetDefaultAcquireSettings(&settings);
        settings.capabilities.qam = true;
        frontend = NEXUS_Frontend_Acquire(&settings);
        if (!frontend) {
            BDBG_ERR(("\n")); BDBG_ERR(("\n")); BDBG_ERR(("\n"));
            BDBG_WRN(("autotest - Total Inband QAM channels %d. Success Video Decoding Channels %d.", ii, successVideoChannel));
            BDBG_ERR(("\n")); BDBG_ERR(("\n")); BDBG_ERR(("\n"));
            goto done;
        }
        else {
            BDBG_ERR(("\n")); BDBG_ERR(("\n")); BDBG_ERR(("\n"));
            BDBG_ERR(("QamTuner[%d] found, handle=0x%x", ii, frontend));
        }
#endif
        NEXUS_PidChannelHandle videoPidChannel, audioPidChannel, pcrPidChannel;
        NEXUS_VideoDecoderStartSettings videoProgram;
        NEXUS_AudioDecoderStartSettings audioProgram;
        bool acquired = false;

        NEXUS_Frontend_GetDefaultQamSettings(&qamSettings);
        qamSettings.frequency = freq[ii%2] * 1000000;
        switch (mode) {
        default:
            BDBG_ERR(("Incorrect mode %d specified. Defaulting to 64(NEXUS_FrontendQamMode_e64)", mode));
        case 64:
            qamSettings.mode = NEXUS_FrontendQamMode_e64;
            qamSettings.symbolRate = 5056900;
            break;
        case 256:
            qamSettings.mode = NEXUS_FrontendQamMode_e256;
            qamSettings.symbolRate = 5360537;
            break;
        }
        qamSettings.annex = NEXUS_FrontendQamAnnex_eB;
        qamSettings.bandwidth = NEXUS_FrontendQamBandwidth_e6Mhz;
        qamSettings.lockCallback.callback = lock_changed_callback;
        qamSettings.lockCallback.context = lockChangedEvent;
        qamSettings.asyncStatusReadyCallback.callback = async_status_ready_callback;
        qamSettings.asyncStatusReadyCallback.context = statusEvent;
        qamSettings.autoAcquire = true;
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

        videoPidChannel = NEXUS_PidChannel_Open(parserBand, videoPid, NULL);
        audioPidChannel = NEXUS_PidChannel_Open(parserBand, audioPid, NULL);
        pcrPidChannel = NEXUS_PidChannel_Open(parserBand, pcrPid, NULL);

        NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
        switch (videoCodec) {
        case NEXUS_VideoCodec_eMpeg2:
        case NEXUS_VideoCodec_eH264:
            videoProgram.codec = videoCodec;
            break;
        default:
            BDBG_ERR(("Unsupported video codec %d specified. Defaulting to 2(NEXUS_VideoCodec_eMpeg2)", videoCodec));
            videoProgram.codec = NEXUS_VideoCodec_eMpeg2;
        }
        videoProgram.pidChannel = videoPidChannel;
        videoProgram.stcChannel = stcChannel;
        NEXUS_AudioDecoder_GetDefaultStartSettings(&audioProgram);
        switch (audioCodec) {
        case NEXUS_AudioCodec_eMpeg:
        case NEXUS_AudioCodec_eAac:
        case NEXUS_AudioCodec_eAc3:
        case NEXUS_AudioCodec_eAacPlus:
            audioProgram.codec = audioCodec;
            break;
        default:
            BDBG_ERR(("Unsupported audio codec %d specified. Defaulting to 7(NEXUS_AudioCodec_eAc3)", audioCodec));
            audioProgram.codec = NEXUS_AudioCodec_eAc3;
        }
        audioProgram.pidChannel = audioPidChannel;
        audioProgram.stcChannel = stcChannel;

        NEXUS_StcChannel_GetSettings(stcChannel, &stcSettings);
        stcSettings.mode = NEXUS_StcChannelMode_ePcr; /* live */
        stcSettings.modeSettings.pcr.pidChannel = pcrPidChannel;
        rc = NEXUS_StcChannel_SetSettings(stcChannel, &stcSettings);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        BKNI_ResetEvent(lockChangedEvent);

        BDBG_WRN(("tuning %d MHz...", freq[ii%2]));
        rc = NEXUS_Frontend_TuneQam(frontend, &qamSettings);
        if(rc){rc = BERR_TRACE(rc); goto done;}


        /* in a real-world app, we don't start decode until we are locked and have scanned for PSI */
        {
            unsigned start_time = b_get_time();
            while (1) {
                unsigned current_time = b_get_time();
                NEXUS_FrontendFastStatus status;

/* MAX_ACQUIRE_TIME is the application timeout. Increased to 20 seconds to enable testing of plugging and unplugging the cable while autoacquire is enabled. */
#define MAX_ACQUIRE_TIME 20000
                if (current_time - start_time > MAX_ACQUIRE_TIME) {
                    BDBG_WRN(("application timeout. cannot acquire."));
                    break;
                }
                rc = BKNI_WaitForEvent(lockChangedEvent, MAX_ACQUIRE_TIME - (current_time - start_time));
                if (rc == BERR_TIMEOUT) {
                    BDBG_WRN(("application timeout. cannot acquire."));
                    break;
                }
                BDBG_ASSERT(!rc);

                rc = NEXUS_Frontend_GetFastStatus(frontend, &status);
                if (rc == NEXUS_SUCCESS) {
                    if (status.lockStatus == NEXUS_FrontendLockStatus_eLocked) {
                        BDBG_WRN(("frontend locked"));
                        acquired = true;
                        break;
                    }
                    else if (status.lockStatus == NEXUS_FrontendLockStatus_eUnlocked) {
                        BDBG_WRN(("frontend unlocked (acquireInProgress=%d)", status.acquireInProgress));
                         /* Wait for  MAX_ACQUIRE_TIME when unlocked*/
                    }
                    else if (status.lockStatus == NEXUS_FrontendLockStatus_eNoSignal) {
                        if(!qamSettings.autoAcquire) {
                            BDBG_WRN(("No signal at the tuned frequency."));
                            break;
                        }
                        else
                            BDBG_WRN(("No signal at the tuned frequency. Waiting till the application timesout to allow auto acquire to try again."));
                    }
                    else {
                        BDBG_ERR(("unknown status: %d", status.lockStatus));
                    }
                }
                else if (rc == NEXUS_NOT_SUPPORTED) {
                    NEXUS_FrontendQamStatus qamStatus;
                    rc = NEXUS_Frontend_GetQamStatus(frontend, &qamStatus);
                    if (rc) {rc = BERR_TRACE(rc); return false;}

                    BDBG_WRN(("frontend %s (fecLock=%d)", qamStatus.receiverLock?"locked":"unlocked", qamStatus.fecLock));
                    if (qamStatus.receiverLock) {
                        acquired = true;
                        break;
                    }
                    /* we can't conclude no lock until application timeout */
                }
                else {
                     BERR_TRACE(rc);
                }
            }
        }

        if (acquired) {
            rc = NEXUS_VideoDecoder_Start(videoDecoder, &videoProgram);
            if(rc){rc = BERR_TRACE(rc); goto done;}
            rc = NEXUS_AudioDecoder_Start(audioDecoder, &audioProgram);
            if(rc){rc = BERR_TRACE(rc); goto done;}

#if 0
			/* for autotest, video and audio decoders started */ 
			BDBG_WRN(("autotest: Video and Audio Decoder Started Successfully!"));
#endif
        }
        else {
            BDBG_WRN(("not starting decode because frontend not acquired"));
        }

        /* disable splash */
        NEXUS_Display_GetGraphicsSettings(display, &graphicsSettings);
        if (graphicsSettings.enabled) {
            graphicsSettings.enabled = false;
            rc = NEXUS_Display_SetGraphicsSettings(display, &graphicsSettings);
            if(rc){rc = BERR_TRACE(rc); goto done;}
        }

        /* for autotest, running 1 minute and read back video decoder status */
        sleep(10);
        NEXUS_VideoDecoderStatus status;
        NEXUS_VideoDecoder_GetStatus(videoDecoder, &status);
        BDBG_WRN(("autotest - Total Decoded Pictures %d.", status.numDecoded));
        BDBG_WRN(("autotest - Total Decoder Errors %d.", status.numDecodeErrors));
        BDBG_WRN(("autotest - Total Displayed Pictures %d.", status.numDisplayed));
        BDBG_WRN(("autotest - Total Network Packets Percentage Error = %d%%.", status.numDecodeErrors*100/status.numDecoded));
        if (status.numDecodeErrors*100/status.numDecoded > 1)
            BDBG_WRN(("autotest - Video Decoding Failure!"));
        else
        {
            successVideoChannel++;
            BDBG_WRN(("autotest - Video Decoding Successfully!"));
        }

        {
            char buf[64];

#if 0  /* for autotest, just run tune_qam once without loop */
again:
            BDBG_WRN(("Enter frequency to tune again (st for status, ENTER to repeat, 0 to exit)"));

            fgets(buf, 64, stdin);
            if (buf[0] != '\n') {
                if (strstr(buf, "st") == buf) {
                    get_status(frontend, statusEvent);
                    goto again;
                }
                else {
                    freq = atoi(buf);
                    if (!freq) done = true;
                }
            }
#else            
	        done = true;
#endif	
        }
        NEXUS_Frontend_Untune(frontend);
        NEXUS_AudioDecoder_Stop(audioDecoder);
        NEXUS_VideoDecoder_Stop(videoDecoder);
        NEXUS_PidChannel_Close(videoPidChannel);
        NEXUS_PidChannel_Close(audioPidChannel);
        NEXUS_PidChannel_Close(pcrPidChannel);
    }

#if 0
    /* for autotest, tune_qam finished */ 
    BDBG_WRN(("autotest - Tune_qam Finished Successfully!"));
#endif

done:
    NEXUS_VideoWindow_RemoveAllInputs(window);
    NEXUS_VideoWindow_Close(window);
	NEXUS_Display_Close(display);
    NEXUS_Surface_Destroy(surface);
    NEXUS_VideoDecoder_Close(videoDecoder);
    NEXUS_AudioDecoder_Close(audioDecoder);
    BKNI_DestroyEvent(statusEvent);
    BKNI_DestroyEvent(lockChangedEvent);
    NEXUS_Platform_Uninit();
    return 0;
}

static NEXUS_Error get_status(NEXUS_FrontendHandle frontend, BKNI_EventHandle statusEvent)
{
    NEXUS_FrontendQamStatus qamStatus;
    int rc;
    NEXUS_FrontendDeviceHandle deviceHandle;
    NEXUS_FrontendDeviceStatus deviceStatus;

    deviceHandle = NEXUS_Frontend_GetDevice(frontend);
    if (!deviceHandle) {
        fprintf(stderr, "Unable to retrieve frontend device handle.\n");
    }

    rc = NEXUS_Frontend_RequestQamAsyncStatus(frontend);
    if(rc == NEXUS_SUCCESS){
        rc = BKNI_WaitForEvent(statusEvent, 1000);
        if (rc) {
            printf("Status not returned\n");
            return BERR_TRACE(rc);
        }
        NEXUS_Frontend_GetQamAsyncStatus(frontend , &qamStatus);

        if (deviceHandle) {
            rc = NEXUS_FrontendDevice_GetStatus(deviceHandle, &deviceStatus);
            if(rc) return BERR_TRACE(rc);
        }
    }
    else if(rc == NEXUS_NOT_SUPPORTED){
        rc = NEXUS_Frontend_GetQamStatus(frontend, &qamStatus);
        if (rc) return BERR_TRACE(rc);
    }
    else {
        return BERR_TRACE(rc);
    }

    printf("\nDownstream lock = %d\n", qamStatus.fecLock);
    printf("Frequency = %d\n", qamStatus.settings.frequency);
    if(qamStatus.settings.mode == NEXUS_FrontendQamMode_e64)
        printf("Mode = NEXUS_FrontendQamMode_e64\n");
    else if(qamStatus.settings.mode == NEXUS_FrontendQamMode_e256)
        printf("Mode  = NEXUS_FrontendQamMode_e256\n");
    else
        printf("Mode = %d\n", qamStatus.settings.mode);
    if(qamStatus.settings.annex == NEXUS_FrontendQamAnnex_eA)
        printf("Annex = NEXUS_FrontendQamAnnex_eA\n");
    else if(qamStatus.settings.annex == NEXUS_FrontendQamAnnex_eB)
        printf("Annex  = NEXUS_FrontendQamAnnex_eB\n");
    else
        printf("Annex = %d\n", qamStatus.settings.annex);
    printf("Symbol rate = %d\n", qamStatus.symbolRate);
    printf("Snr estimate = %d\n", qamStatus.snrEstimate/100 );
    printf("FecCorrected = %d\n", qamStatus.fecCorrected);
    printf("FecUncorrected = %d\n", qamStatus.fecUncorrected);
    printf("DsChannelPower in dBmV = %d\n", qamStatus.dsChannelPower/10);
    printf("DsChannelPower in dBm = %d\n", qamStatus.dsChannelPower/10 - 48);
    if (deviceHandle) {
        printf("AVS enabled = %d\n", deviceStatus.avs.enabled);
        printf("AVS voltage = %d\n", deviceStatus.avs.voltage);
        printf("Device temperature = %d\n", deviceStatus.temperature);
    }
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

