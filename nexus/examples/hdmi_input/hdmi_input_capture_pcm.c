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
#if NEXUS_HAS_HDMI_OUTPUT && NEXUS_HAS_HDMI_INPUT && NEXUS_HAS_AUDIO
#include <getopt.h>

#include "nexus_platform.h"
#include "nexus_core_utils.h"

#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_video_input.h"
#include "nexus_video_decoder.h"

#include "nexus_hdmi_input.h"
#include "nexus_hdmi_input_hdcp.h"
#include "nexus_hdmi_output_hdcp.h"

#include "nexus_audio_input.h"
#include "nexus_audio_input_capture.h"
#include "nexus_audio_decoder.h"
#include "nexus_audio_capture.h"
#include "nexus_audio_output.h"

#include "bstd.h"
#include "bkni.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

BDBG_MODULE(hdmi_input_capture_pcm);

typedef struct pcm_format_s {
    uint16_t num_channels;
    uint32_t sample_rate;
    uint16_t bits_per_sample;
    uint16_t block_align;
} pcm_format_t;

typedef struct {
    int menuEntry;
    NEXUS_VideoFormat format;
    char* name;
} formatTable;

static formatTable testFormats[] = {
    { 0, NEXUS_VideoFormat_e720p,       "720p" },
    { 0, NEXUS_VideoFormat_e480p,       "480p" },
    { 0, NEXUS_VideoFormat_e720p50hz,   "720p 50Hz" },
    { 0, NEXUS_VideoFormat_e1080p24hz,  "1080p 24Hz" },
    { 0, NEXUS_VideoFormat_e1080p30hz,  "1080p 30Hz" },
    { 0, NEXUS_VideoFormat_e1080p60hz,  "1080p 60Hz" },

    /* END of ARRAY */
    { 0, 0, "" }
};

typedef struct app_configuration {
    pcm_format_t audioFormat;
    int audioFormatSet;
    int first_msg;

    char *dumpfilename;
    FILE *pFile;

    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_HdmiInputHandle hdmiInput;
    NEXUS_HdmiOutputHandle hdmiOutput;
    NEXUS_AudioInputCaptureHandle inputCapture;
    NEXUS_AudioInputCaptureStartSettings inputCaptureStartSettings;
    NEXUS_AudioCaptureHandle audioCapture;

    NEXUS_DisplayHandle display;
    NEXUS_VideoWindowHandle window;

} app_configuration;


/* enable the following only if HDCP keys are available for the Input and Output Port */
/* Production source devices connected to hdmiInput will fail if
        1) no keys  have been loaded (see hdmi_input_hdcpkeyloader app for example) or
        2)  if macro HDCP_SUPPORT_ON_HDMI_OUTPUT is 0
*/

#define USE_OUTPUT_CAPTURE          0
#define CAPTURE_TO_MEMORY           1
#define HDCP_SUPPORT_ON_HDMI_INPUT  0
#define HDCP_SUPPORT_ON_HDMI_OUTPUT 0

#if HDCP_SUPPORT_ON_HDMI_INPUT

/*****************************************************/
/* INSERT PRODUCTION KeySet generated by BCrypt HERE */
/*****************************************************/

#endif /* HDCP_SUPPORT_ON_HDMI_INPUT */

/*******************************************************************************
* Function definitions
*/

static void usage(void)
{
    printf("\nBroadcom (c) 2015 - Connecting Everything!\n");
    printf("-------------------------------------------\n");
    printf("Usage: hdmi_input_capture_pcm [options] \n");
    printf("Options:\n");
    printf("   -b                         : Send FEC data before main audio data\n");
    printf("   -c                         : Use WiFi clock for time\n");
    printf("   -d <dump filename>         : Dump captured audio to filename\n");
    printf("   -f <FEC length>            : Enable FEC processing and set the FEC packet length\n");
    printf("   -h <hostname | ipaddress>  : Hostname or IP address of client\n");
    printf("   -m                         : Enable multicast\n");
    printf("   -n <num extra buffers>     : Number of extra packet buffers to allocate\n");
    printf("   -p <portnum>               : RTP port number\n");
}


static __inline__ void printInputCaptureStatus(app_configuration *config)
{
    NEXUS_AudioInputCaptureStatus audioStatus;

    NEXUS_AudioInputCapture_GetStatus(config->inputCapture, &audioStatus);

    fprintf(stderr, "Audio input capture status   SPV: %d,%d,%d, SR: %d, codec %d, channels: %d\n",
            audioStatus.started, audioStatus.inputSignalPresent, audioStatus.inputSignalValid,
            audioStatus.sampleRate, audioStatus.codec, audioStatus.numPcmChannels);
    fflush(stderr);
}


static void capture_callback(void *pParam, int param)
{
    app_configuration *config = (app_configuration *)pParam;
    NEXUS_Error errCode;
    static int foocount = 0;

    BSTD_UNUSED(param);

    /*
     * We have audio data. Have we informed the streamer about the format yet?
     */
    BDBG_ERR(("Data Ready Callback received"));
    if (!config->audioFormatSet)
    {
        NEXUS_AudioInputCaptureStatus inputStatus;
        #if USE_OUTPUT_CAPTURE
        NEXUS_AudioCaptureSettings captureSettings;
        #else
        NEXUS_AudioInputCaptureSettings captureSettings;
        #endif
        uint16_t bits_per_sample;
        uint16_t block_align;

        NEXUS_AudioInputCapture_GetStatus(config->inputCapture, &inputStatus);
        #if USE_OUTPUT_CAPTURE
        NEXUS_AudioCapture_GetSettings(config->audioCapture, &captureSettings);
        #else
        NEXUS_AudioInputCapture_GetSettings(config->inputCapture, &captureSettings);
        #endif

        if (captureSettings.format == NEXUS_AudioCaptureFormat_e16BitStereo)
        {
            BDBG_ERR(("stereo 16 bit"));
            bits_per_sample = 16;
            block_align     = inputStatus.numPcmChannels * bits_per_sample / 8;
        }
        else if (captureSettings.format == NEXUS_AudioCaptureFormat_e24BitStereo || captureSettings.format == NEXUS_AudioCaptureFormat_e24Bit5_1)
        {
            BDBG_ERR(("multichannel 24 bit"));
            bits_per_sample = 24;
            block_align     = inputStatus.numPcmChannels * 32 / 8;
        }
        else
        {
            BDBG_ERR(("bad capture format"));
            fprintf(stderr, "BAD AUDIO CAPTURE FORMAT\n");
            NEXUS_AudioCapture_Stop(config->audioCapture);
            return;
        }

        config->audioFormat.num_channels    = inputStatus.numPcmChannels;
        config->audioFormat.sample_rate     = inputStatus.sampleRate;
        config->audioFormat.bits_per_sample = bits_per_sample;
        config->audioFormat.block_align     = block_align;

        config->audioFormatSet = 1;
    }

    for ( ;; )
    {
        void *pBuffer;
        size_t bufferSize;

        /* Check available buffer space */
        #if USE_OUTPUT_CAPTURE
        errCode = NEXUS_AudioCapture_GetBuffer(config->audioCapture, (void **)&pBuffer, &bufferSize);
        #else
        errCode = NEXUS_AudioInputCapture_GetBuffer(config->inputCapture, (void **)&pBuffer, &bufferSize);
        #endif
        if ( errCode )
        {
            fprintf(stderr, "Error getting capture buffer\n");
            #if USE_OUTPUT_CAPTURE
            NEXUS_AudioCapture_Stop(config->audioCapture);
            #else
            NEXUS_AudioInputCapture_Stop(config->inputCapture);
            #endif
            return;
        }

        if (bufferSize > 0)
        {
            if (config->first_msg == 0)
            {
                fprintf(stderr, "Data callback - FIRST DATA BUFFER OF %d bytes\n", (int)bufferSize);
                config->first_msg = 1;
            }
            if (bufferSize > (10 * 1024))
            {
                fprintf(stderr, "Data callback - Large audio buffer of %d bytes\n", (int)bufferSize);
            }

            if (config->pFile != NULL)
            {
                /* Write samples to disk */
                if (1 != fwrite(pBuffer, bufferSize, 1, config->pFile))
                {
                    fprintf(stderr, "Error writing to disk\n");
                    #if USE_OUTPUT_CAPTURE
                    NEXUS_AudioCapture_Stop(config->audioCapture);
                    #else
                    NEXUS_AudioInputCapture_Stop(config->inputCapture);
                    #endif
                    return;
                }
            }

            if (0 && (foocount++ % 10) == 0)
            {
                NEXUS_TimebaseStatus status;

                errCode = NEXUS_Timebase_GetStatus(NEXUS_Timebase_e0, &status);
                if (errCode)
                {
                    fprintf(stderr, "Error getting timebase status %d\n", errCode);
                }
                fprintf(stderr, "timebase is %u, lastValue %u, sourceType %d\n", status.timebase, status.lastValue, status.sourceType);
            }

            #if USE_OUTPUT_CAPTURE
            errCode = NEXUS_AudioCapture_ReadComplete(config->audioCapture, bufferSize);
            #else
            errCode = NEXUS_AudioInputCapture_ReadComplete(config->inputCapture, bufferSize);
            #endif
            if (errCode)
            {
                fprintf(stderr, "Error committing capture buffer\n");
                #if USE_OUTPUT_CAPTURE
                NEXUS_AudioCapture_Stop(config->audioCapture);
                #else
                NEXUS_AudioInputCapture_Stop(config->inputCapture);
                #endif
                return;
            }
        }
        else
        {
            break;
        }
    }
}


#if HDCP_SUPPORT_ON_HDMI_INPUT
static void displayKeyLoadStatus(uint8_t success)
{

    BDBG_LOG(("*************************")) ;
    BDBG_LOG(("HDCP Key Loading: %s", success ? "SUCCESS" : " FAILED")) ;
    BDBG_LOG(("*************************")) ;
}
#endif

void audio_source_changed(void *context, int param)
{
    NEXUS_AudioInputCaptureHandle audioHandle;
    NEXUS_AudioInputCaptureStatus audioStatus;
    app_configuration *config = (app_configuration *)context;

    BSTD_UNUSED(param);

    fprintf(stderr, "Audio source changed:\n");

    audioHandle = config->inputCapture;

    NEXUS_AudioInputCapture_GetStatus(audioHandle, &audioStatus);

    printInputCaptureStatus(config);

    if (1 && audioStatus.started && audioStatus.inputSignalPresent && audioStatus.inputSignalValid &&
        (audioStatus.codec != NEXUS_AudioCodec_ePcm || (config->audioFormat.num_channels > 0 && audioStatus.numPcmChannels != config->audioFormat.num_channels)))
    {
        config->audioFormatSet = 0;
        config->first_msg      = 0;
        fprintf(stderr, "******* Reset audio format\n");
    }
}


void hdmi_source_changed(void *context, int param)
{
    NEXUS_HdmiInputHandle hdmiInput;
    NEXUS_HdmiInputStatus hdmiInputStatus;
    NEXUS_DisplaySettings displaySettings;

    app_configuration *config = (app_configuration *)context;
    hdmiInput = config->hdmiInput;

    NEXUS_HdmiInput_GetStatus(hdmiInput, &hdmiInputStatus);
    fprintf(stderr, "HDMI input status (callback):\n");
    fprintf(stderr, "attached %d,  audio valid %d,  Type %d,  wordlength %d,  freq %d\n",
            hdmiInputStatus.deviceAttached, hdmiInputStatus.audio.validSpdifInfo, hdmiInputStatus.audio.streamType, hdmiInputStatus.audio.wordLength, hdmiInputStatus.audio.sampleFreq);

    NEXUS_Display_GetSettings(config->display, &displaySettings);
    if (displaySettings.format != hdmiInputStatus.originalFormat)
    {
        BDBG_WRN(("Video Format Change - Updating to %u", hdmiInputStatus.originalFormat));

        displaySettings.format = hdmiInputStatus.originalFormat;
        NEXUS_Display_SetSettings(config->display, &displaySettings);

        printInputCaptureStatus(config);
    }
}


void avmute_changed(void *context, int param)
{
    NEXUS_HdmiInputHandle hdmiInput;
    NEXUS_HdmiInputStatus hdmiInputStatus;
    app_configuration *config = (app_configuration *)context;

    hdmiInput = config->hdmiInput;
    NEXUS_HdmiInput_GetStatus(hdmiInput, &hdmiInputStatus);

    if (!hdmiInputStatus.validHdmiStatus)
    {
        printf("avmute_changed callback: Unable to get hdmiInput status\n") ;
    }
    else
    {
        printf("avmute_changed callback: %s\n", hdmiInputStatus.avMute ? "Set_AvMute" : "Clear_AvMute") ;
    }
    printInputCaptureStatus(config);
}

#if HDCP_SUPPORT_ON_HDMI_INPUT

static void hdmiInputHdcpStateChanged(void *context, int param)
{
    NEXUS_HdmiInputHandle hdmiInput;
    NEXUS_HdmiInputHdcpStatus hdmiInputHdcpStatus;
    app_configuration *config = (app_configuration *)param;

    hdmiInput = (NEXUS_HdmiInputHandle)context;
    NEXUS_HdmiInput_HdcpGetStatus(hdmiInput, &hdmiInputHdcpStatus);

    printf("HDCP State change begin\n");
    switch (hdmiInputHdcpStatus.eAuthState)
    {
        case NEXUS_HdmiInputHdcpAuthState_eKeysetInitialization:
            BDBG_WRN(("Change in HDCP Key Set detected\n")) ;
            switch (hdmiInputHdcpStatus.eKeyStorage)
            {
                case NEXUS_HdmiInputHdcpKeyStorage_eOtpROM:
                    BDBG_WRN(("HDCP Keys stored in OTP ROM\n"));
                    break ;

                case NEXUS_HdmiInputHdcpKeyStorage_eOtpRAM :
                    BDBG_WRN(("HDCP Keys stored in OTP RAM\n"));
                    break ;

                default :
                    BDBG_WRN(("Unknown Key Storage type %d\n", hdmiInputHdcpStatus.eKeyStorage)) ;
            }

            BDBG_WRN(("NOTE: EACH DEVICE REQUIRES A UNIQUE HDCP KEY SET; The same KeySet cannot be used in multiple devices\n\n"));
            break;


        case NEXUS_HdmiInputHdcpAuthState_eWaitForKeyloading :
            BDBG_WRN(("Upstream HDCP Authentication request ...")) ;

            BDBG_WRN(("Aksv: %02x %02x %02x %02x %02x",
                hdmiInputHdcpStatus.aksvValue[4], hdmiInputHdcpStatus.aksvValue[3],
                hdmiInputHdcpStatus.aksvValue[2], hdmiInputHdcpStatus.aksvValue[1],
                hdmiInputHdcpStatus.aksvValue[0])) ;

            BDBG_WRN(("An:   %02x %02x %02x %02x %02x %02x %02x %02x ",
                hdmiInputHdcpStatus.anValue[7], hdmiInputHdcpStatus.anValue[6],
                hdmiInputHdcpStatus.anValue[5], hdmiInputHdcpStatus.anValue[4],
                hdmiInputHdcpStatus.anValue[3], hdmiInputHdcpStatus.anValue[2],
                hdmiInputHdcpStatus.anValue[1], hdmiInputHdcpStatus.anValue[0])) ;

            break ;

        case NEXUS_HdmiInputHdcpAuthState_eWaitForDownstreamKsvs :
            BDBG_WRN(("Downstream FIFO Request; Start hdmiOuput Authentication...")) ;
            NEXUS_HdmiOutput_StartHdcpAuthentication(config->hdmiOutput);
            break ;

        default :
		BDBG_WRN(("Unknown State %d", hdmiInputHdcpStatus.eAuthState )) ;
    }

    printf("HDCP State change complete\n");
}
#endif

#if HDCP_SUPPORT_ON_HDMI_OUTPUT
static void hdmiOutputHdcpStateChanged(void *pContext, int param)
{
    bool success = (bool)param;
    app_configuration *config = (app_configuration *)pContext;

    NEXUS_HdmiOutputHdcpStatus hdmiOutputHdcpStatus;
    NEXUS_HdmiHdcpDownStreamInfo downStream  ;
    NEXUS_HdmiHdcpKsv *pKsvs ;

    unsigned returnedDevices ;
    uint8_t i ;

    NEXUS_HdmiOutput_GetHdcpStatus(config->hdmiOutput, &hdmiOutputHdcpStatus);

    if (!success )
    {
        BDBG_WRN(("HDCP Authentication Failed.  Current State %d", hdmiOutputHdcpStatus.hdcpState));
        return ;
    }

    BDBG_WRN(("HDCP Tx Authentication Successful")) ;

    NEXUS_HdmiOuput_HdcpGetDownstreamInfo(config->hdmiOutput, &downStream) ;

    /* allocate space to hold ksvs for the downstream devices */
    pKsvs = BKNI_Malloc(downStream.devices * NEXUS_HDMI_HDCP_KSV_LENGTH) ;

    NEXUS_HdmiOuput_HdcpGetDownstreamKsvs(config->hdmiOutput,
        pKsvs, downStream.devices, &returnedDevices) ;

    BDBG_WRN(("hdmiOutput Downstream Levels:  %d  Devices: %d", downStream.depth, downStream.devices)) ;

    /* display the downstream device KSVs */
    for (i = 0 ; i < downStream.devices; i++)
    {
        BDBG_WRN(("Device %02d BKsv: %02X %02X %02X %02X %02X",
            i + 1,
            *(pKsvs->data + i + 4), *(pKsvs->data + i + 3),
            *(pKsvs->data + i + 2), *(pKsvs->data + i + 1),
            *(pKsvs->data + i ))) ;
    }
    NEXUS_HdmiInput_HdcpLoadKsvFifo(config->hdmiInput, &downStream, pKsvs, downStream.devices);

    BKNI_Free(pKsvs);
}
#endif


static void disable_audio(app_configuration *config)
{
    NEXUS_AudioInputCapture_Stop(config->inputCapture);
}


static void enable_audio(app_configuration *config)
{
    printf("Input audio source status before enable:\n");
    printInputCaptureStatus(config);

    fprintf(stderr, "Enable audio input capture\n");
    NEXUS_AudioInputCapture_Start(config->inputCapture, &config->inputCaptureStartSettings);
    fprintf(stderr, "Enable audio complete\n");

    printf("Input audio source status after enable:\n");
    printInputCaptureStatus(config);
}


static void mute_audio(NEXUS_HdmiOutputHandle hdmiOutput)
{
    NEXUS_AudioOutputSettings outputSettings;

    NEXUS_AudioOutput_GetSettings(NEXUS_HdmiOutput_GetAudioConnector(hdmiOutput), &outputSettings);
    outputSettings.muted = !outputSettings.muted;
    NEXUS_AudioOutput_SetSettings(NEXUS_HdmiOutput_GetAudioConnector(hdmiOutput), &outputSettings);
}


static void hotplug_callback(void *pContext, int iParam)
{
    NEXUS_HdmiOutputStatus status;
    NEXUS_HdmiOutputHandle hdmiOutput ;
    NEXUS_DisplayHandle display ;

    app_configuration *config = (app_configuration *)pContext;
    hdmiOutput = config->hdmiOutput ;
    display = config->display ;

    NEXUS_HdmiOutput_GetStatus(hdmiOutput, &status);
    BDBG_LOG(("hotplug_callback: %s\n", status.connected ? "DEVICE CONNECTED" : "DEVICE REMOVED")) ;

    if ( status.connected )
    {
        /* app can update the board's hdmiRxEdid port if new device has been attached */
        BDBG_MSG(("app can update the board's hdmiRxEdid port if new device has been attached")) ;
    }
    else
    {
        /* app can use the hdmiRx internal EDID if no device is to the Tx */
        BDBG_MSG(("app can use the hdmiRx internal EDID if no device is to the Tx")) ;
    }

    /* the app can choose to switch to the preferred format, but it's not required. */
    if ( status.connected )
    {
        NEXUS_DisplaySettings displaySettings;
        NEXUS_Display_GetSettings(display, &displaySettings);
        if ( !status.videoFormatSupported[displaySettings.format] )
        {
            BDBG_WRN(("Current format not supported by attached monitor. Switching to preferred format %d",
                status.preferredVideoFormat)) ;
            displaySettings.format = status.preferredVideoFormat;
            NEXUS_Display_SetSettings(display, &displaySettings);
        }
    }
}

void hdmi_input_status(NEXUS_HdmiInputHandle hdmiInput)
{
    NEXUS_HdmiInputStatus hdmiInputStatus ;
    BDBG_Level saveLevel ;
    static const char *textColorSpace[] =
    {
        "RGB ",
        "YCbCr 4:2:2",
        "YCbCr 4:4:4",
        "Max",
    } ;

    BDBG_GetModuleLevel("config", &saveLevel) ;

    BDBG_SetModuleLevel("config", BDBG_eMsg) ;

    NEXUS_HdmiInput_GetStatus(hdmiInput, &hdmiInputStatus) ;
    if (!hdmiInputStatus.validHdmiStatus)
    {
        BDBG_WRN(("Cannot determine input status...")) ;
        return  ;
    }

    BDBG_MSG(("hdmiInput Mode  : %s", hdmiInputStatus.hdmiMode ? "HDMI" : "DVI")) ;
    BDBG_MSG(("hdmiInput Format: %d x %d %c %s",
        hdmiInputStatus.avWidth, hdmiInputStatus.avHeight,
        hdmiInputStatus.interlaced ? 'i' : 'p',
        textColorSpace[hdmiInputStatus.colorSpace])) ;

    BDBG_MSG(("hdmiInput Clock : %d", hdmiInputStatus.lineClock)) ;
    BDBG_MSG(("HDCP Enabled    : %s", hdmiInputStatus.hdcpRiUpdating ? "Yes" : "No")) ;

    /* restore debug level */
    BDBG_SetModuleLevel("config", saveLevel) ;
}

void hdmi_output_status(NEXUS_HdmiOutputHandle hdmiOutput)
{
    NEXUS_HdmiOutputStatus status;
    BDBG_Level saveLevel;

    BDBG_GetModuleLevel("config", &saveLevel);

    BDBG_SetModuleLevel("config", BDBG_eMsg);

    NEXUS_HdmiOutput_GetStatus(hdmiOutput, &status) ;
    if (!status.connected)
    {
        BDBG_WRN(("Cannot determine output status...")) ;
        return  ;
    }

    BDBG_MSG(("Monitor <%s> (Nexus) hdmiOutput Format: %d",
        status.monitorName, status.videoFormat));

    BDBG_MSG(("Audio Format: %d; Sample Rate: %dHz;  Sample Size: %d Channels: %d",
        status.audioFormat,
        status.audioSamplingRate, /* in units of Hz */
        status.audioSamplingSize, status.audioChannelCount));

    BDBG_MSG(("HDMI Index: %d", status.index));

    /* restore debug level */
    BDBG_SetModuleLevel("config", saveLevel);
}


static void mangleEdid(uint8_t *edid, uint16_t edidBytes)
{
    int dtdBegin;
    int idx = 128;
    int sum;
    int i;

    if (edidBytes < 128)
    {
        printf("No EDID extension blocks present\n");
        return;
    }

    /*
     * Make sure that we have the CEA EDID block.
     */

    if (edid[idx] != 0x02)
    {
        printf("Not a CEA EDID extension block (0x%02x)\n", edid[idx]);
        return;
    }

    dtdBegin = edid[idx + 2];
    if (dtdBegin == 0x04)
    {
        printf("No Data Block Collection in EDID\n");
        return;
    }

    dtdBegin += 128;
    idx += 4;

    while (idx < edidBytes && idx < dtdBegin)
    {
        uint8_t blockType;
        uint8_t blockBytes;

        /*
         * Look for an audio data block.
         */

        blockType  = (edid[idx] & 0xE0) >> 5;
        blockBytes = (edid[idx] & 0x1F);

        if (blockType == 1)
        {
            idx++;
            while (blockBytes > 0)
            {
                printf("Audio block found:\n");
                printf("   Codecs: %d\n", (edid[idx] & 0x78) >> 3);
                printf("   Channels: %d\n", (edid[idx] & 0x07) + 1);
                printf("   Sample Rates: 0x%02x\n", edid[idx + 1]);
                printf("   Bit Rates: 0x%02x\n", (edid[idx + 2] & 0x03));

                if (1)
                {
                    printf("Audio format overwrite...\n");
                    edid[idx]     = (1 << 3) | 5;
                    edid[idx + 1] = 0x04;
                    edid[idx + 2] = 0x01;
                    printf("   Codecs: %d\n", (edid[idx] & 0x78) >> 3);
                    printf("   Channels: %d\n", (edid[idx] & 0x07) + 1);
                    printf("   Sample Rates: 0x%02x\n", edid[idx + 1]);
                    printf("   Bit Rates: 0x%02x\n", (edid[idx + 2] & 0x03));

                    /*
                     * Don't forget to update the checksum.
                     */

                    for (i = 128, sum = 0; i < 255; i++)
                    {
                        sum += edid[i];
                    }
                    sum = 256 - (sum & 0xff);
                    edid[255] = sum;
                }

                blockBytes -= 3;
                idx += 3;
            }
        }
        else
        {
            idx += blockBytes + 1;
        }
    }
}


static int parse_options(app_configuration *config, int argc, char **argv)
{
    int opt;
    int option_index;
    static struct option long_options[] = {
        {"clock",     no_argument,       0, 'c'},
        {"help",      no_argument,       0,  0 },
        {"hostname",  required_argument, 0, 'h'},
        {"multicast", no_argument,       0, 'm'},
        {"port",      required_argument, 0, 'p'},
        {0,           0,                 0,  0 }
    };

    opterr = 0; /* No automatic error messages from getopt */
    option_index = 0;
    while (1)
    {
        opt = getopt_long(argc, argv, ":bcd:f:h:mn:p:", long_options, &option_index);

        if (opt == -1)
            break;

        switch (opt)
        {
            case 0:
                usage();
                break;

            case 'b':
                break;

            case 'c':
                break;

            case 'd':
                if (config->dumpfilename)
                {
                    free(config->dumpfilename);
                }
                config->dumpfilename = strdup(optarg);
                break;

            case 'f':
                break;

            case 'h':
                break;

            case 'm':
                break;

            case 'n':
                break;

            case 'p':
                break;

            default:    /* ? */
                if (opt == ':')
                {
                    printf("Missing argument for option: %c\n", optopt);
                }
                else
                {
                    printf("Unrecognized option: %c\n", optopt);
                }
                usage();
                break;
        }
    }

    return 0;
}


static void config_shutdown(app_configuration *config)
{
    if (config == NULL)
    {
        return;
    }

    disable_audio(config);

    /* stop/remove HDMI callbacks associated with display, so those callbacks do not access display once it is removed */
    NEXUS_StopCallbacks(config->hdmiOutput);
    NEXUS_StopCallbacks(config->hdmiInput);

    /* Shutdown the video pipeline. */
    NEXUS_VideoWindow_RemoveInput(config->window, NEXUS_HdmiInput_GetVideoConnector(config->hdmiInput));
    NEXUS_VideoWindow_Close(config->window);
    NEXUS_Display_RemoveOutput(config->display, NEXUS_HdmiOutput_GetVideoConnector(config->hdmiOutput));
    NEXUS_Display_Close(config->display);

    /* Now shutdown the audio pipline. */
    NEXUS_AudioOutput_RemoveAllInputs(NEXUS_HdmiOutput_GetAudioConnector(config->hdmiOutput));

    NEXUS_AudioInput_Shutdown(NEXUS_AudioInputCapture_GetConnector(config->inputCapture));
    NEXUS_AudioInput_Shutdown(NEXUS_HdmiInput_GetAudioConnector(config->hdmiInput));
    NEXUS_AudioInputCapture_Close(config->inputCapture);

    #if USE_OUTPUT_CAPTURE
    NEXUS_AudioCapture_Close(config->audioCapture);
    #endif

    NEXUS_HdmiInput_Close(config->hdmiInput);

    /*
     * And free any miscellanous bits of memory.
     */

    if (config->pFile != NULL)
    {
        fclose(config->pFile);
    }

    if (config->dumpfilename)
    {
        free(config->dumpfilename);
    }

    NEXUS_Platform_Uninit();
}


static app_configuration *config_init(int argc, char **argv)
{
    NEXUS_PlatformSettings platformSettings;
    NEXUS_TimebaseSettings timebaseSettings;
    NEXUS_HdmiOutputStatus hdmiOutputStatus;
    NEXUS_HdmiOutputSettings hdmiOutputSettings;
    NEXUS_HdmiInputSettings hdmiInputSettings;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_AudioMultichannelFormat audioFormat;
    unsigned numChPairs = 1;
    NEXUS_AudioInputCaptureSettings inputCaptureSettings;
    NEXUS_AudioInputCaptureOpenSettings inputCaptureOpenSettings;
    #if USE_OUTPUT_CAPTURE
    NEXUS_AudioCaptureOpenSettings audioCaptureOpenSettings;
    NEXUS_AudioCaptureStartSettings audioCaptureStartSettings;
    NEXUS_AudioCaptureSettings audioCaptureSettings;
    #endif
    unsigned hdmiInputIndex = 0;
    unsigned timeout = 4;
    app_configuration *config;
    NEXUS_Error rc;

    /*
     * Allocate the main application structure.
     */

    if ((config = calloc(1, sizeof(app_configuration))) == NULL)
        return NULL;

    parse_options(config, argc, argv);

    /* Bring up all modules for a platform in a default configuration for this platform */
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&config->platformConfig);

    NEXUS_Timebase_GetSettings(NEXUS_Timebase_e0, &timebaseSettings);
    timebaseSettings.sourceType = NEXUS_TimebaseSourceType_eHdDviIn;
    NEXUS_Timebase_SetSettings(NEXUS_Timebase_e0, &timebaseSettings);

    NEXUS_HdmiInput_GetDefaultSettings(&hdmiInputSettings);
    hdmiInputSettings.timebase = NEXUS_Timebase_e0;

    /* set hpdDisconnected to true if a HDMI switch is in front of the Broadcom HDMI Rx.
         -- The NEXUS_HdmiInput_ConfigureAfterHotPlug should be called to inform the hw of
         -- the current state,  the Broadcom SV reference boards have no switch so
         -- the value should always be false
         */
    hdmiInputSettings.frontend.hpdDisconnected = false;

    config->hdmiOutput = config->platformConfig.outputs.hdmi[0];
    do
    {
        /* check for connected downstream device */
        rc = NEXUS_HdmiOutput_GetStatus(config->hdmiOutput, &hdmiOutputStatus);
        if (rc) BERR_TRACE(rc);
        if (hdmiOutputStatus.connected)
        {
            break;
        }
        BDBG_WRN(("Waiting for HDMI Tx Device"));
        BKNI_Sleep(250);
    } while (timeout-- > 0);

    if (hdmiOutputStatus.connected)
    {
        NEXUS_HdmiOutputEdidBlock edidBlock;
        NEXUS_HdmiOutputBasicEdidData hdmiOutputBasicEdidData;
        uint8_t *attachedRxEdid;
        uint16_t rxEdidBytes;
        unsigned i, j;

        /* Get EDID of attached receiver*/
        rc = NEXUS_HdmiOutput_GetBasicEdidData(config->hdmiOutput, &hdmiOutputBasicEdidData);
        if (rc)
        {
            BDBG_ERR(("Error retrieving EDID from attached receiver"));
            goto error_exit;
        }

        /* allocate space to hold the EDID blocks */
        rxEdidBytes = (hdmiOutputBasicEdidData.extensions+1) * sizeof(edidBlock.data) ;
        attachedRxEdid = BKNI_Malloc(rxEdidBytes);

        for (i = 0; i <= hdmiOutputBasicEdidData.extensions; i++)
        {
            rc = NEXUS_HdmiOutput_GetEdidBlock(config->hdmiOutput, i, &edidBlock);
            if (rc)
            {
                BDBG_ERR(("Error retrieving EDID from attached receiver"));
                attachedRxEdid = 0;
                rxEdidBytes = 0;
                break;
            }

            for (j = 0; j < sizeof(edidBlock.data); j++)
            {
                attachedRxEdid[i * sizeof(edidBlock.data) + j] = edidBlock.data[j];
            }
        }

        mangleEdid(attachedRxEdid, rxEdidBytes);

        /* Load hdmiInput EDID RAM with the EDID from the Rx attached to the hdmiOutput (Tx) . */
        config->hdmiInput = NEXUS_HdmiInput_OpenWithEdid(hdmiInputIndex, &hdmiInputSettings, attachedRxEdid, rxEdidBytes);

        /* release memory resources */
        if (attachedRxEdid)
            BKNI_Free(attachedRxEdid);
    }

    if (!config->hdmiInput)
    {
        fprintf(stderr, "Can't get hdmi input\n");
        goto error_exit;
    }

    NEXUS_Display_GetDefaultSettings(&displaySettings);
    displaySettings.format = NEXUS_VideoFormat_e1080p30hz;
    config->display = NEXUS_Display_Open(0, &displaySettings);

    NEXUS_HdmiInput_GetSettings(config->hdmiInput, &hdmiInputSettings);

    hdmiInputSettings.avMuteChanged.callback = avmute_changed;
    hdmiInputSettings.avMuteChanged.context  = config;
    hdmiInputSettings.avMuteChanged.param    = 0;

    hdmiInputSettings.sourceChanged.callback = hdmi_source_changed;
    hdmiInputSettings.sourceChanged.context  = config ;
    hdmiInputSettings.sourceChanged.param = 0 ;
    NEXUS_HdmiInput_SetSettings(config->hdmiInput, &hdmiInputSettings);

    if (config->dumpfilename)
    {
        /*
         * Open a file for our capture output.
         */

        config->pFile = fopen(config->dumpfilename, "wb+");
        if (NULL == config->pFile)
        {
            fprintf(stderr, "Unable to open file '%s' for writing\n", config->dumpfilename);
        }
    }

#if 0 /* HDCP_SUPPORT_ON_HDMI_INPUT */
    {
        NEXUS_HdmiInputHdcpSettings hdmiInputHdcpSettings;
        NEXUS_HdmiInputHdcpStatus hdmiInputHdcpStatus;
        NEXUS_HdmiInputHdcpKeyset hdmiInputKeyset ;

        NEXUS_HdmiInput_HdcpGetDefaultSettings(config->hdmiInput, &hdmiInputHdcpSettings);

        /*
         -- for this sample app override the HDCP repeater functionality
         -- so hdmi in to hdmi out path can be tested
         --
         -- in a normal application, overriding the HDCP Repeater is a violation of the HDCP license
         -- See hdmi_repeater_passthrough app for full HDCP support.
        */

        hdmiInputHdcpSettings.repeater = false;

        hdmiInputHdcpSettings.hdcpRxChanged.callback = hdmiInputHdcpStateChanged;
        hdmiInputHdcpSettings.hdcpRxChanged.context  = config->hdmiInput;
        hdmiInputHdcpSettings.hdcpRxChanged.param    = (int)config;

        NEXUS_HdmiInput_HdcpSetSettings(config->hdmiInput, &hdmiInputHdcpSettings);

        /*
         * Load the key set.
         */

        NEXUS_HdmiInput_HdcpGetDefaultKeyset(config->hdmiInput, &hdmiInputKeyset);

        /* Initialize/Load HDCP Key Set	*/
        hdmiInputKeyset.alg         = encryptedRxKeySetAlg;
        hdmiInputKeyset.custKeyVarL = encryptedRxKeySetKeyVar1;
        hdmiInputKeyset.custKeyVarH = encryptedRxKeySetKeyVar2;
        hdmiInputKeyset.custKeySel  = encryptedRxKeySetCusKey;

        BKNI_Memcpy(&hdmiInputKeyset.rxBksv, &hdcpRxBksv, NEXUS_HDMI_HDCP_KSV_LENGTH);

        BKNI_Memcpy(&hdmiInputKeyset.privateKey, &encryptedRxKeySet, sizeof(NEXUS_HdmiInputHdcpKey) * NEXUS_HDMI_HDCP_NUM_KEYS);

        rc = NEXUS_HdmiInput_HdcpSetKeyset(config->hdmiInput, &hdmiInputKeyset);
        if (rc)
        {
            /* display message informing of result of HDCP Key Load */
            displayKeyLoadStatus(0) ;
            BDBG_ERR(("Error setting HDCP Keys"));
            goto error_exit;
        }

        NEXUS_HdmiInput_HdcpGetStatus(config->hdmiInput, &hdmiInputHdcpStatus);

        /* display message informing of result of HDCP Key Load */
        /* NOTE: use of otpState is overloaded... refers to status of key load */
        if (hdmiInputHdcpStatus.eOtpState != NEXUS_HdmiInputHdcpKeySetOtpState_eCrcMatch)
            displayKeyLoadStatus(0);
        else
            displayKeyLoadStatus(1);
    }
#endif

#if HDCP_SUPPORT_ON_HDMI_OUTPUT
    {
        NEXUS_HdmiOutputHdcpSettings hdmiOutputHdcpSettings;

        NEXUS_HdmiOutput_GetHdcpSettings(config->hdmiOutput, &hdmiOutputHdcpSettings);
        /* If you want to use a custom key, insert that here */
        hdmiOutputHdcpSettings.successCallback.callback = hdmiOutputHdcpStateChanged;
        hdmiOutputHdcpSettings.successCallback.context  = config;
        hdmiOutputHdcpSettings.successCallback.param    = true;

        hdmiOutputHdcpSettings.failureCallback.callback = hdmiOutputHdcpStateChanged;
        hdmiOutputHdcpSettings.failureCallback.context  = config;
        hdmiOutputHdcpSettings.failureCallback.param    = false;
        NEXUS_HdmiOutput_SetHdcpSettings(config->hdmiOutput, &hdmiOutputHdcpSettings);

        NEXUS_HdmiOutput_SetHdcpRevokedKsvs(config->hdmiOutput, RevokedKsvs, NumRevokedKsvs) ;
    }
#endif

    config->window = NEXUS_VideoWindow_Open(config->display, 0);
    NEXUS_Display_AddOutput(config->display, NEXUS_HdmiOutput_GetVideoConnector(config->hdmiOutput));

    /* Install hotplug callback -- video only for now */
    NEXUS_HdmiOutput_GetSettings(config->hdmiOutput, &hdmiOutputSettings);
        hdmiOutputSettings.hotplugCallback.callback = hotplug_callback;
        hdmiOutputSettings.hotplugCallback.context = config ;
    NEXUS_HdmiOutput_SetSettings(config->hdmiOutput, &hdmiOutputSettings);

#if 1
    audioFormat = NEXUS_AudioMultichannelFormat_e5_1;
    numChPairs = 3;
#else
    audioFormat = NEXUS_AudioMultichannelFormat_eStereo;
    numChPairs = 1;
#endif

    /*
     * Connect the audio input capture to the HDMI input.
     */
    NEXUS_AudioInputCapture_GetDefaultOpenSettings(&inputCaptureOpenSettings);
    #if !USE_OUTPUT_CAPTURE
    inputCaptureOpenSettings.multichannelFormat = audioFormat;
    inputCaptureOpenSettings.fifoSize = numChPairs * 128 * 1024; /* 128KB per PCM channel pair */
    inputCaptureOpenSettings.format = (audioFormat == NEXUS_AudioMultichannelFormat_e5_1) ? NEXUS_AudioCaptureFormat_e24Bit5_1 : NEXUS_AudioCaptureFormat_e16BitStereo;
    inputCaptureOpenSettings.threshold = inputCaptureOpenSettings.fifoSize / 4;
    BDBG_ERR(("Opening in Memory Capture Mode"));
    #endif
    config->inputCapture = NEXUS_AudioInputCapture_Open(0, &inputCaptureOpenSettings);
    NEXUS_AudioInputCapture_GetDefaultStartSettings(&config->inputCaptureStartSettings);
    config->inputCaptureStartSettings.input = NEXUS_HdmiInput_GetAudioConnector(config->hdmiInput);

    NEXUS_AudioInputCapture_GetSettings(config->inputCapture, &inputCaptureSettings);
    inputCaptureSettings.sourceChanged.callback = audio_source_changed;
    inputCaptureSettings.sourceChanged.context  = config;
    inputCaptureSettings.sourceChanged.param    = 0;
    #if !USE_OUTPUT_CAPTURE
    inputCaptureSettings.dataCallback.callback = capture_callback;
    inputCaptureSettings.dataCallback.context = config;
    inputCaptureSettings.dataCallback.param = 0;
    #endif
    NEXUS_AudioInputCapture_SetSettings(config->inputCapture, &inputCaptureSettings);

    #if USE_OUTPUT_CAPTURE
    NEXUS_AudioCapture_GetDefaultOpenSettings(&audioCaptureOpenSettings);
    audioCaptureOpenSettings.multichannelFormat = audioFormat;
    audioCaptureOpenSettings.fifoSize *= 3;  /* Increase FIFO for 6 channels instead of stereo */
    audioCaptureOpenSettings.threshold = 1024;
    fprintf(stderr, "Open Capture (mcformat %d, format: %d)\n", audioCaptureOpenSettings.multichannelFormat, audioCaptureOpenSettings.format);
    config->audioCapture = NEXUS_AudioCapture_Open(0, &audioCaptureOpenSettings);
    if (NULL == config->audioCapture)
    {
        fprintf(stderr, "Unable to open capture channel\n");
        goto error_exit;
    }

    /* Set to multichannel mode */
    NEXUS_AudioCapture_GetSettings(config->audioCapture, &audioCaptureSettings);
#if 0
    audioCaptureSettings.format = NEXUS_AudioCaptureFormat_e24Bit5_1;
#endif
    NEXUS_AudioCapture_SetSettings(config->audioCapture, &audioCaptureSettings);

    /* Connect capture to decoder */
    fprintf(stderr, "Connect input capture to audio capture\n");
    NEXUS_AudioOutput_AddInput(NEXUS_AudioCapture_GetConnector(config->audioCapture), NEXUS_AudioInputCapture_GetConnector(config->inputCapture));

    /* Start the capture -- no data will be received until the input capture starts */
    NEXUS_AudioCapture_GetDefaultStartSettings(&audioCaptureStartSettings);
    audioCaptureStartSettings.dataCallback.callback = capture_callback;
    audioCaptureStartSettings.dataCallback.context  = config;
    audioCaptureStartSettings.dataCallback.param    = 0;
    fprintf(stderr, "Start capture\n");
    NEXUS_AudioCapture_Start(config->audioCapture, &audioCaptureStartSettings);
    #elif !CAPTURE_TO_MEMORY
    NEXUS_AudioOutput_AddInput(NEXUS_HdmiOutput_GetAudioConnector(config->hdmiOutput), NEXUS_AudioInputCapture_GetConnector(config->inputCapture));
    NEXUS_AudioOutput_AddInput(NEXUS_SpdifOutput_GetConnector(config->platformConfig.outputs.spdif[0]), NEXUS_AudioInputCapture_GetConnector(config->inputCapture));
    #endif

    NEXUS_VideoWindow_AddInput(config->window, NEXUS_HdmiInput_GetVideoConnector(config->hdmiInput));

    return config;

error_exit:
    if (config)
    {
        free(config);
    }

    return NULL;
}


int main(int argc, char **argv)
{
    app_configuration *config;
    NEXUS_Error rc;
    unsigned i, j, menuIndex;
    bool connectVideoWindow = true;

    /*
     * Main initialization.
     */

    if ((config = config_init(argc, argv)) == NULL)
    {
        return -1;
    }

    enable_audio(config);

    for (i = 0; ; i++)
    {
        NEXUS_DisplaySettings displaySettings;
        NEXUS_HdmiOutputStatus hdmiOutputStatus;
        unsigned char chr ;
        unsigned int menuSelection;
        unsigned int formatSelection;

        /* Find Current Format */
        NEXUS_Display_GetSettings(config->display, &displaySettings);
        for ( j = 0; testFormats[j].format ; j++)
        {
            if ( displaySettings.format == testFormats[j].format ) { break; }
        }

        printf("Main Menu\n");

        printf("1) Change Video Format %s%s\n", i ? "-> Currently " : "" ,i ? testFormats[j].name : "" );
        printf("2) Restart Audio\n");
        printf("3) hdmiInput Status\n");
        printf("4) hdmiOutput Status\n");
        printf("5) Toggle Audio Mute\n");
        printf("6) Toggle Video Window\n");

        printf("0) Exit\n");
        printf("Enter Selection: ");

        if (!scanf("%u", &menuSelection))
        {
           printf("Invalid Menu Selection\n\n") ;
           menuSelection = 255 ;  /* non-numeric scanned */
        }
        else
            printf("Menu Selection %d\n", menuSelection) ;

        switch ( menuSelection )
        {
        case 0:
            goto exit_menu;

        case 1:
            /* get/build a list of formats supported by attached Receiver */
            NEXUS_HdmiOutput_GetStatus(config->hdmiOutput, &hdmiOutputStatus);

            /* Display available formats */
            menuIndex = 0;
            printf("\n\n 0) Return to Main Menu\n");

            j = 0 ;
            while ( testFormats[j].format )
            {
                /* if format is supported; add to menu */
                if ( hdmiOutputStatus.videoFormatSupported[testFormats[j].format] )
                {
                    testFormats[j].menuEntry = ++ menuIndex ;
                    printf("%2d) %s\n", menuIndex, testFormats[j].name );
                }
                j++;
            }

            /* Read user input for desired format */
            printf("\nEnter new format-> ");
            scanf("%u", &formatSelection);

            if( !formatSelection )  /* 0 - Exit */
                break;  /* Exit to Main Menu*/

            if ( (uint8_t) formatSelection > menuIndex)
            {
                scanf("%c", &chr); /* flush buffer */
                printf("Invalid Video Format choice:\n\n");
                break;
            }

            /* confirm valid format selection */
            for ( j = 0; ; j++)
            {
                if ( (int) formatSelection == testFormats[j].menuEntry )
                {
                    /* Set Format */
                    NEXUS_Display_GetSettings(config->display, &displaySettings);
                    displaySettings.format = testFormats[j].format;
                    rc = NEXUS_Display_SetSettings(config->display, &displaySettings);
                    if (rc)
                    {
                        printf("ERROR changing format\n") ;
                    }
                    break;
                }
            }
            break;

        case 2:
            disable_audio(config);
            enable_audio(config);
            break;

        case 3: hdmi_input_status(config->hdmiInput);   break;
        case 4: hdmi_output_status(config->hdmiOutput); break;

        case 5:
            mute_audio(config->hdmiOutput);
            break;

        case 6:
            if(connectVideoWindow)
            {
                NEXUS_VideoWindow_RemoveInput(config->window, NEXUS_HdmiInput_GetVideoConnector(config->hdmiInput));
                NEXUS_VideoWindow_Close(config->window);
            }
            else
            {
                config->window = NEXUS_VideoWindow_Open(config->display, 0);
                NEXUS_VideoWindow_AddInput(config->window, NEXUS_HdmiInput_GetVideoConnector(config->hdmiInput));
            }
            connectVideoWindow = !connectVideoWindow;
            printf("\nVideo window is %s\n\n", connectVideoWindow ? "connected" : "disconnected");
            break;

        case 255 :
        default:
           scanf("%c", &chr); /* clear buffer */
            break;
        }
    }

exit_menu:

    config_shutdown(config);

    return 0;
}
#else
#include <stdio.h>
int main(void)
{
    printf("This application is not supported on this platform.\n");
    return 0;
}
#endif
