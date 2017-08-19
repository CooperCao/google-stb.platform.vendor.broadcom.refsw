/******************************************************************************
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
******************************************************************************/
#include "nexus_platform.h"
#if NEXUS_HAS_HDMI_OUTPUT && NEXUS_HAS_HDMI_INPUT && NEXUS_HAS_AUDIO
#include "nexus_core_utils.h"

#include "nexus_display.h"
#include "nexus_graphics2d.h"
#include "nexus_video_window.h"
#include "nexus_video_input.h"
#include "nexus_video_decoder.h"

#include "nexus_hdmi_input.h"
#include "nexus_hdmi_input_info.h"
#include "nexus_hdmi_input_ext.h"
#include "nexus_hdmi_input_hdcp.h"
#include "nexus_hdmi_output_hdcp.h"

#include "nexus_audio_input.h"
#include "nexus_audio_decoder.h"
#include "nexus_audio_dac.h"
#include "nexus_audio_output.h"

#include "bstd.h"
#include "bkni.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

BDBG_MODULE(hdmi_input_to_hdmi_output) ;

static    NEXUS_PlatformConfiguration platformConfig;

#if NEXUS_NUM_AUDIO_INPUT_CAPTURES
#include "nexus_audio_input_capture.h"

static bool decodeAudio = false;
static bool dualAudio = false;
static bool hdmiOutOnly = false;
static NEXUS_AudioDecoderHandle audioDecoder;
static NEXUS_AudioDecoderStartSettings audioProgram;
static NEXUS_AudioInputCaptureHandle inputCapture;
static NEXUS_AudioInputCaptureStartSettings inputCaptureStartSettings;

#define OUTPUT_VIDEO_FORMAT_FOLLOWS_INPUT 1

/* enable the following only if HDCP keys are available for the Input and Output Port */
/* Production source devices connected to hdmiInput will fail if
        1) no keys  have been loaded (see hdmi_input_hdcpkeyloader app for example) or
        2)  if macro HDCP_SUPPORT_ON_HDMI_OUTPUT is 0
*/

#define HDCP_SUPPORT_ON_HDMI_INPUT 0
#define HDCP_SUPPORT_ON_HDMI_OUTPUT 0


    NEXUS_HdmiInputHandle hdmiInput;
    NEXUS_HdmiInputSettings hdmiInputSettings;

    NEXUS_HdmiOutputHandle hdmiOutput;
    NEXUS_HdmiOutputSettings hdmiOutputSettings;


    NEXUS_VideoFormatInfo displayFormatInfo;
    NEXUS_SurfaceCreateSettings createSurfaceSettings;
    NEXUS_SurfaceHandle framebuffer, offscreen;
    NEXUS_Graphics2DFillSettings fillSettings;
    NEXUS_Graphics2DHandle gfx;
    NEXUS_Graphics2DSettings gfxSettings;
    NEXUS_GraphicsSettings graphicsSettings;
    NEXUS_Graphics2DBlitSettings blitSettings;

    bool graphicsOverlayEnabled ;
    bool avEnabled = false ;

    BKNI_EventHandle checkpointEvent, spaceAvailableEvent;

typedef struct hotplugCallbackParameters
{
    NEXUS_HdmiOutputHandle hdmiOutput  ;
    NEXUS_DisplayHandle display ;
} hotplugCallbackParameters ;


typedef struct SourceChangedCallbackParameters
{
    NEXUS_HdmiInputHandle hdmiInput  ;
    NEXUS_DisplayHandle display ;
} SourceChangedCallbackParameters ;


static void source_changed(void *context, int param)
{
#if OUTPUT_VIDEO_FORMAT_FOLLOWS_INPUT
    NEXUS_Error errCode ;
    NEXUS_HdmiInputHandle hdmiInput ;
    NEXUS_HdmiInputStatus hdmiInputStatus ;
    NEXUS_HdmiOutputSettings hdmiOutputSettings ;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_DisplayHandle display ;

    SourceChangedCallbackParameters *sourceChangeCbParams ;

    sourceChangeCbParams = (SourceChangedCallbackParameters *) context ;
        hdmiInput = sourceChangeCbParams->hdmiInput ;
        display = sourceChangeCbParams->display ;

    NEXUS_HdmiInput_GetStatus(hdmiInput, &hdmiInputStatus) ;
    if (!hdmiInputStatus.validHdmiStatus) {
        return ;
    }

    NEXUS_Display_GetSettings(display, &displaySettings);
    if ( displaySettings.format != hdmiInputStatus.originalFormat )
    {
        if (hdmiInputStatus.originalFormat == NEXUS_VideoFormat_eUnknown)
        {
            BDBG_MSG(("Incoming format not determined yet; don't switch output format yet")) ;
            return ;
        }

        BDBG_LOG(("Video Format Change - Updating to %u", hdmiInputStatus.originalFormat));
        displaySettings.format = hdmiInputStatus.originalFormat;
        errCode = NEXUS_Display_SetSettings(display, &displaySettings);
        if (errCode) {
            BDBG_ERR(("Unable to set Display Settings (errCode= %d)", errCode)) ;
            return ;
        }
    }

    NEXUS_HdmiOutput_GetSettings(hdmiOutput, &hdmiOutputSettings) ;
        hdmiOutputSettings.colorDepth = hdmiInputStatus.colorDepth ;
        hdmiOutputSettings.colorSpace = hdmiInputStatus.colorSpace ;
    errCode = NEXUS_HdmiOutput_SetSettings(hdmiOutput, &hdmiOutputSettings) ;
    if (errCode)
    {
        BDBG_ERR(("Unable to set HdmiOutputSettings")) ;
    }
#else
    BSTD_UNUSED(context);
    BSTD_UNUSED(param);
#endif
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
        BDBG_WRN(("avmute_changed callback: Unable to get hdmiInput status\n")) ;
    }
    else
    {
        BDBG_LOG(("avmute_changed callback: %s\n",
            hdmiInputStatus.avMute ? "Set_AvMute" : "Clear_AvMute")) ;
    }
}

#if HDCP_SUPPORT_ON_HDMI_INPUT

static void hdmiInputHdcpStateChanged(void *context, int param)
{
    NEXUS_HdmiInputHandle hdmiInput ;
    NEXUS_HdmiInputHdcpStatus hdmiInputHdcpStatus ;

    BSTD_UNUSED(param) ;

    hdmiInput = (NEXUS_HdmiInputHandle) context ;
    NEXUS_HdmiInput_HdcpGetStatus(hdmiInput, &hdmiInputHdcpStatus) ;

    switch (hdmiInputHdcpStatus.eAuthState)
    {
    case NEXUS_HdmiInputHdcpAuthState_eKeysetInitialization :

        BDBG_WRN(( "Change in HDCP Key Set detected\n")) ;
        switch (hdmiInputHdcpStatus.eKeyStorage)
        {
            case NEXUS_HdmiInputHdcpKeyStorage_eOtpROM :
            BDBG_WRN(( "HDCP Keys stored in OTP ROM\n")) ;
            break ;

        case NEXUS_HdmiInputHdcpKeyStorage_eOtpRAM :
            BDBG_WRN(( "HDCP Keys stored in OTP RAM\n")) ;
            break ;

        default :
            BDBG_WRN(( "Unknown Key Storage type %d\n", hdmiInputHdcpStatus.eKeyStorage)) ;
        }

        BDBG_WRN(( "NOTE: EACH DEVICE REQUIRES A UNIQUE HDCP KEY SET; The same KeySet cannot be used in multiple devices\n\n")) ;

        break ;


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
        BDBG_WRN(("Downstream FIFO Request; Start hdmiOutput Authentication...")) ;
        NEXUS_HdmiOutput_StartHdcpAuthentication(hdmiOutput);
        break ;

    default :
        BDBG_WRN(("Unknown State %d", hdmiInputHdcpStatus.eAuthState )) ;
    }

}
#endif

#if HDCP_SUPPORT_ON_HDMI_OUTPUT
static void hdmiOutputHdcpStateChanged(void *pContext, int param)
{
    bool success = (bool)param;
    NEXUS_HdmiOutputHandle handle = pContext;

    NEXUS_HdmiOutputHdcpStatus hdmiOutputHdcpStatus;
    NEXUS_HdmiHdcpDownStreamInfo downStream  ;
    NEXUS_HdmiHdcpKsv *pKsvs ;

    unsigned returnedDevices ;
    uint8_t i ;

    NEXUS_HdmiOutput_GetHdcpStatus(handle, &hdmiOutputHdcpStatus);

    if (!success )
    {
        BDBG_WRN(("HDCP Authentication Failed.  Current State %d",
            hdmiOutputHdcpStatus.hdcpState);
        return ;
    }

    BDBG_WRN(("HDCP Tx Authentication Successful")) ;

    NEXUS_HdmiOutput_HdcpGetDownstreamInfo(handle, &downStream) ;

    /* allocate space to hold ksvs for the downstream devices */
    pKsvs =
        BKNI_Malloc(downStream.devices * NEXUS_HDMI_HDCP_KSV_LENGTH) ;

    NEXUS_HdmiOutput_HdcpGetDownstreamKsvs(handle,
        pKsvs, downStream.devices, &returnedDevices) ;

    BDBG_WRN(("hdmiOutput Downstream Levels:  %d  Devices: %d",
        downStream.depth, downStream.devices)) ;

    /* display the downstream device KSVs */
    for (i = 0 ; i < downStream.devices; i++)
   {
        BDBG_WRN(("Device %02d BKsv: %02X %02X %02X %02X %02X",
            i + 1,
            *(pKsvs->data + i + 4), *(pKsvs->data + i + 3),
            *(pKsvs->data + i + 2), *(pKsvs->data + i + 1),
            *(pKsvs->data + i ))) ;
    }
    NEXUS_HdmiInput_HdcpLoadKsvFifo(hdmiInput, &downStream, pKsvs, downStream.devices) ;

    BKNI_Free(pKsvs) ;
}
#endif

static void disable_audio(NEXUS_HdmiOutputHandle hdmiOutput)
{
    BSTD_UNUSED(hdmiOutput);

    if ( decodeAudio || dualAudio )
    {
        BDBG_ERR(("Stopping Audio Decode\n"));
        NEXUS_AudioDecoder_Stop(audioDecoder);
    }

    if ( !decodeAudio || dualAudio )
    {
        BDBG_ERR(("Stopping HDMI bypass passthrough Audio\n"));
        NEXUS_AudioInputCapture_Stop(inputCapture);
    }
}

static void enable_audio(NEXUS_HdmiOutputHandle hdmiOutput)
{
    BSTD_UNUSED(hdmiOutput);

    if ( decodeAudio || dualAudio )
    {
        BDBG_LOG(("Starting Audio Decode\n"));
        NEXUS_AudioDecoder_Start(audioDecoder, &audioProgram);
    }

    if ( !decodeAudio || dualAudio )
    {
        BDBG_LOG(("Starting HDMI bypass passthrough Audio\n"));
        NEXUS_AudioInputCapture_Start(inputCapture, &inputCaptureStartSettings);
    }
}

static void mute_audio(NEXUS_HdmiOutputHandle hdmiOutput)
{
    NEXUS_AudioOutputSettings outputSettings;

    NEXUS_AudioOutput_GetSettings(NEXUS_HdmiOutput_GetAudioConnector(hdmiOutput), &outputSettings);
    outputSettings.muted = !outputSettings.muted;
    NEXUS_AudioOutput_SetSettings(NEXUS_HdmiOutput_GetAudioConnector(hdmiOutput), &outputSettings);
}

static void hotplug_callback(void *pParam, int iParam)
{
    NEXUS_HdmiOutputStatus status;
    NEXUS_HdmiOutputHandle hdmiOutput ;
    NEXUS_DisplayHandle display ;
    hotplugCallbackParameters *hotPlugCbParams ;

    hotPlugCbParams = (hotplugCallbackParameters *) pParam ;
    hdmiOutput = hotPlugCbParams->hdmiOutput ;
    display = hotPlugCbParams->display ;

    NEXUS_HdmiOutput_GetStatus(hdmiOutput, &status);
    BDBG_LOG(("hotplug_callback: %s\n", status.connected ?
        "DEVICE CONNECTED" : "DEVICE REMOVED")) ;

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

void hdmi_input_status(void )
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

    BDBG_GetModuleLevel("hdmi_input_to_hdmi_output", &saveLevel) ;

    BDBG_SetModuleLevel("hdmi_input_to_hdmi_output", BDBG_eMsg) ;

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

    {
        NEXUS_Error errCode;
        NEXUS_AudioInputStatus audioStatus;

        errCode = NEXUS_AudioInput_GetInputStatus(NEXUS_HdmiInput_GetAudioConnector(hdmiInput), &audioStatus);

        if ( errCode == NEXUS_SUCCESS && audioStatus.valid )
        {
            BDBG_MSG(("Audio Codec: %d (%s)", audioStatus.codec, audioStatus.codec == NEXUS_AudioCodec_ePcm?"PCM":"Compressed"));
            if ( audioStatus.codec == NEXUS_AudioCodec_ePcm )
            {
                BDBG_MSG(("  Audio num chs: %d", audioStatus.numPcmChannels));
            }
            BDBG_MSG(("Audio Samplerate: %d", audioStatus.sampleRate));
        }
        else
        {
            BDBG_MSG(("Audio not initialized"));
        }


    }

    /* restore debug level */
    BDBG_SetModuleLevel("hdmi_input_to_hdmi_output", saveLevel) ;

}

void hdmi_output_status(void )
{
    NEXUS_HdmiOutputStatus status ;
    BDBG_Level saveLevel ;

    BDBG_GetModuleLevel("hdmi_input_to_hdmi_output", &saveLevel) ;

    BDBG_SetModuleLevel("hdmi_input_to_hdmi_output", BDBG_eMsg) ;

    NEXUS_HdmiOutput_GetStatus(hdmiOutput, &status) ;
    if (!status.connected)
    {
        BDBG_WRN(("Cannot determine output status...")) ;
        return  ;
    }

    BDBG_MSG(("Monitor <%s> (Nexus) hdmiOutput Format: %d",
        status.monitorName, status.videoFormat)) ;

    BDBG_MSG(("Audio Format: %d; Sample Rate: %dHz;  Sample Size: %d Channels: %d",
        status.audioFormat,
        status.audioSamplingRate, /* in units of Hz */
        status.audioSamplingSize, status.audioChannelCount)) ;

    BDBG_MSG(("HDMI Index: %d", status.index)) ;

    /* restore debug level */
    BDBG_SetModuleLevel("hdmi_input_to_hdmi_output", saveLevel) ;

}

void graphicsComplete(void *data, int unused)
{
    BSTD_UNUSED(unused);
    BKNI_SetEvent((BKNI_EventHandle) data);
}

void setupGraphics(
    NEXUS_DisplayHandle display, NEXUS_DisplaySettings *displaySettings)
{

    NEXUS_Error rc;

    NEXUS_VideoFormat_GetInfo(displaySettings->format, &displayFormatInfo);

    /* create actual framebuffer */
    NEXUS_Surface_GetDefaultCreateSettings(&createSurfaceSettings);
        /* older platforms cannot support 32bpp pixel format on a large framebuffer due to bandwidth issues.
           you can either use a 16bpp pixel format, or reduce the framebuffer width */
        createSurfaceSettings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
#define FRAMEBUFFER_WIDTH 1440
        createSurfaceSettings.width = FRAMEBUFFER_WIDTH;
        createSurfaceSettings.height = 1080;
        createSurfaceSettings.heap = NEXUS_Platform_GetFramebufferHeap(0);
    framebuffer = NEXUS_Surface_Create(&createSurfaceSettings);

    /* create app offscreen buffer */
    NEXUS_Surface_GetDefaultCreateSettings(&createSurfaceSettings);
        createSurfaceSettings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
        createSurfaceSettings.width = 720;
        createSurfaceSettings.height = 480;
    offscreen = NEXUS_Surface_Create(&createSurfaceSettings);

    BKNI_CreateEvent(&checkpointEvent);
    BKNI_CreateEvent(&spaceAvailableEvent);

    gfx = NEXUS_Graphics2D_Open(0, NULL);

    NEXUS_Graphics2D_GetSettings(gfx, &gfxSettings);
    gfxSettings.checkpointCallback.callback = graphicsComplete;
    gfxSettings.checkpointCallback.context = checkpointEvent ;
    gfxSettings.packetSpaceAvailable.callback = graphicsComplete;
    gfxSettings.packetSpaceAvailable.context = spaceAvailableEvent;
    NEXUS_Graphics2D_SetSettings(gfx, &gfxSettings);


    /* build the offscreen buffer */
    NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
        fillSettings.surface = offscreen;
        fillSettings.color = 0x00000000; /* transparent black */
    rc = NEXUS_Graphics2D_Fill(gfx, &fillSettings);
    BDBG_ASSERT(!rc);

    NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
        fillSettings.surface = offscreen;
        fillSettings.rect.x = 100;
        fillSettings.rect.y = 100;
        fillSettings.rect.width = 100;
        fillSettings.rect.height = 100;
        fillSettings.color = 0xFFFFFFFF; /* opaque white */
    rc = NEXUS_Graphics2D_Fill(gfx, &fillSettings);
    BDBG_ASSERT(!rc);

    NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
        fillSettings.surface = framebuffer;
        fillSettings.color = 0x00000000; /* transparent black */
    rc = NEXUS_Graphics2D_Fill(gfx, &fillSettings);
    BDBG_ASSERT(!rc);

    NEXUS_Display_GetGraphicsSettings(display, &graphicsSettings);
        graphicsSettings.enabled = true;
        graphicsSettings.clip.width = FRAMEBUFFER_WIDTH;
        graphicsSettings.clip.height = displayFormatInfo.height;
    rc = NEXUS_Display_SetGraphicsSettings(display, &graphicsSettings);
    BDBG_ASSERT(!rc);


    rc = NEXUS_Display_SetGraphicsFramebuffer(display, framebuffer);
    BDBG_ASSERT(!rc);

    /* blit can fail because an internal queue is full. wait for all blits to complete, then continue. */
    rc = NEXUS_Graphics2D_Checkpoint(gfx, NULL);
    if (rc == NEXUS_GRAPHICS2D_QUEUED)
    {
        BKNI_WaitForEvent(checkpointEvent, 0xffffffff);
    }
}

void updateGraphics(void)
{
    NEXUS_Error rc;
    static uint8_t index = 0 ;

#define NUM_TEST_MODES 14
struct {
    bool sbs;
    int zoffset;
} g_testmode[NUM_TEST_MODES] =
{
    {true, 0},
    {true, -5},
    {true, -10},
    {true, 0},
    {true, 5},
    {true, 10},
    {true, 0},
    {false, 0},
    {false, -5},
    {false, -10},
    {false, 0},
    {false, 5},
    {false, 10},
    {false, 0}
};

    if (index == NUM_TEST_MODES) index = 0;

#if NO_ZNORM
/**
The zoffset == 0 case is included to keep the example code simple.
**/
    if (g_testmode[i].sbs)
    {
        printf("mode: L/R half\n");
        NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);
        blitSettings.source.surface = offscreen;
        blitSettings.output.surface = framebuffer;
        blitSettings.output.rect.x = 0;
        blitSettings.output.rect.y = 0;
        blitSettings.output.rect.width = FRAMEBUFFER_WIDTH/2;
        blitSettings.output.rect.height = displayFormatInfo.height;
        rc = NEXUS_Graphics2D_Blit(gfx, &blitSettings);
        BDBG_ASSERT(!rc);

        NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);
        blitSettings.source.surface = offscreen;
        blitSettings.output.surface = framebuffer;
        blitSettings.output.rect.x = FRAMEBUFFER_WIDTH/2;
        blitSettings.output.rect.y = 0;
        blitSettings.output.rect.width = FRAMEBUFFER_WIDTH/2;
        blitSettings.output.rect.height = displayFormatInfo.height;
        rc = NEXUS_Graphics2D_Blit(gfx, &blitSettings);
        BDBG_ASSERT(!rc);
    }
    else
    {
        printf("mode: O/U half\n");
        NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);
        blitSettings.source.surface = offscreen;
        blitSettings.output.surface = framebuffer;
        blitSettings.output.rect.x = 0;
        blitSettings.output.rect.y = 0;
        blitSettings.output.rect.width = FRAMEBUFFER_WIDTH;
        blitSettings.output.rect.height = displayFormatInfo.height/2;
        rc = NEXUS_Graphics2D_Blit(gfx, &blitSettings);
        BDBG_ASSERT(!rc);

        NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);
        blitSettings.source.surface = offscreen;
        blitSettings.output.surface = framebuffer;
        blitSettings.output.rect.x = 0;
        blitSettings.output.rect.y = displayFormatInfo.height/2;
        blitSettings.output.rect.width = FRAMEBUFFER_WIDTH;
        blitSettings.output.rect.height = displayFormatInfo.height/2;
        rc = NEXUS_Graphics2D_Blit(gfx, &blitSettings);
        BDBG_ASSERT(!rc);
    }
#else

#define abs(X) ((X)>0?(X):-(X))

/* This code handles positive, negative and zero zoffset. */
    if (g_testmode[index].sbs)
    {
        int zoffset = g_testmode[index].zoffset;
        int scaled_zoffset = zoffset * (int)createSurfaceSettings.width / (int)(FRAMEBUFFER_WIDTH/2);

        printf("mode: L/R half with %d zoffset\n", zoffset);
        NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);
            blitSettings.source.surface = offscreen;
            blitSettings.source.rect.x = scaled_zoffset<0?-scaled_zoffset:0;
            blitSettings.source.rect.y = 0;
            blitSettings.source.rect.width = createSurfaceSettings.width - abs(scaled_zoffset);
            blitSettings.source.rect.height = createSurfaceSettings.height;
            blitSettings.output.surface = framebuffer;
            blitSettings.output.rect.x = zoffset>0?zoffset:0;
            blitSettings.output.rect.y = 0;
            blitSettings.output.rect.width = FRAMEBUFFER_WIDTH/2 - abs(zoffset);
            blitSettings.output.rect.height = displayFormatInfo.height;
        rc = NEXUS_Graphics2D_Blit(gfx, &blitSettings);
        BDBG_ASSERT(!rc);

        NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);
            blitSettings.source.surface = offscreen;
            blitSettings.source.rect.x = scaled_zoffset>0?scaled_zoffset:0;
            blitSettings.source.rect.y = 0;
            blitSettings.source.rect.width = createSurfaceSettings.width - abs(scaled_zoffset);
            blitSettings.source.rect.height = createSurfaceSettings.height;
            blitSettings.output.surface = framebuffer;
            blitSettings.output.rect.x = FRAMEBUFFER_WIDTH/2 - (zoffset<0?zoffset:0);
            blitSettings.output.rect.y = 0;
            blitSettings.output.rect.width = FRAMEBUFFER_WIDTH/2 - abs(zoffset);
            blitSettings.output.rect.height = displayFormatInfo.height;
        rc = NEXUS_Graphics2D_Blit(gfx, &blitSettings);
        BDBG_ASSERT(!rc);
        }
    else
    {
        int zoffset = g_testmode[index].zoffset;
        int scaled_zoffset = zoffset; /* no scaling in horizontal direction */

        printf("mode: O/U half with %d zoffset\n", zoffset);
        NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);
            blitSettings.source.surface = offscreen;
            blitSettings.source.rect.x = scaled_zoffset<0?-scaled_zoffset:0;
            blitSettings.source.rect.y = 0;
            blitSettings.source.rect.width = createSurfaceSettings.width - abs(scaled_zoffset);
            blitSettings.source.rect.height = createSurfaceSettings.height;
            blitSettings.output.surface = framebuffer;
            blitSettings.output.rect.x = zoffset>0?zoffset:0;
            blitSettings.output.rect.y = 0;
            blitSettings.output.rect.width = FRAMEBUFFER_WIDTH - abs(zoffset);
            blitSettings.output.rect.height = displayFormatInfo.height/2;
        rc = NEXUS_Graphics2D_Blit(gfx, &blitSettings);
        BDBG_ASSERT(!rc);

        NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);
            blitSettings.source.surface = offscreen;
            blitSettings.source.rect.x = scaled_zoffset>0?scaled_zoffset:0;
            blitSettings.source.rect.y = 0;
            blitSettings.source.rect.width = createSurfaceSettings.width - abs(scaled_zoffset);
            blitSettings.source.rect.height = createSurfaceSettings.height;
            blitSettings.output.surface = framebuffer;
            blitSettings.output.rect.x = zoffset<0?-zoffset:0;
            blitSettings.output.rect.y = displayFormatInfo.height/2;
            blitSettings.output.rect.width = FRAMEBUFFER_WIDTH - abs(zoffset);
            blitSettings.output.rect.height = displayFormatInfo.height/2;
        rc = NEXUS_Graphics2D_Blit(gfx, &blitSettings);
        BDBG_ASSERT(!rc);
        }
#endif
        /* blit can fail because an internal queue is full. wait for all blits to complete, then continue. */
        rc = NEXUS_Graphics2D_Checkpoint(gfx, NULL);
        if (rc == NEXUS_GRAPHICS2D_QUEUED)
        {
            BKNI_WaitForEvent(checkpointEvent, 0xffffffff);
        }

        index++ ;
    }
#endif

void shutdownGraphics(void)
{
    if (!graphicsOverlayEnabled)
        return ;

    NEXUS_Graphics2D_Close(gfx) ;

    if (spaceAvailableEvent)
        BKNI_DestroyEvent(spaceAvailableEvent) ;

    if (checkpointEvent)
        BKNI_DestroyEvent(checkpointEvent) ;

    NEXUS_Surface_Destroy(offscreen) ;
    NEXUS_Surface_Destroy(framebuffer) ;
}


int main(int argc, char **argv)
{
    NEXUS_DisplayHandle display;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_VideoWindowHandle window;

    NEXUS_TimebaseSettings timebaseSettings;
    NEXUS_PlatformSettings platformSettings ;

    NEXUS_HdmiOutputStatus hdmiOutputStatus;
    SourceChangedCallbackParameters sourceChangeCbParams ;
    hotplugCallbackParameters hotPlugCbParams ;

    NEXUS_Error rc;
    bool exitTest = false ;
    bool useAttachedRxEdid = true ;

    unsigned i, j, menuIndex;
    unsigned timeout=4;
    unsigned hdmiInputIndex = 0;
    int curarg = 0;
    int waitTime ;
    bool connectVideoWindow = true;

    typedef struct {
        int menuEntry;
        NEXUS_VideoFormat format;
        char* name;
    } formatTable;

    static formatTable testFormats[] = {

        { 0, NEXUS_VideoFormat_e1080i, "1080i" },
        { 0, NEXUS_VideoFormat_e720p, "720p" },
        { 0, NEXUS_VideoFormat_e480p, "480p" },
        { 0, NEXUS_VideoFormat_eNtsc, "480i (NTSC)" },
        { 0, NEXUS_VideoFormat_e720p50hz, "720p 50Hz" },
        { 0, NEXUS_VideoFormat_e1080p24hz, "1080p 24Hz" },
        { 0, NEXUS_VideoFormat_e1080i50hz, "1080i 50Hz" },
        { 0, NEXUS_VideoFormat_e1080p50hz, "1080p 50Hz" },
        { 0, NEXUS_VideoFormat_e1080p60hz, "1080p 60Hz" },
        { 0, NEXUS_VideoFormat_ePal, "576i (PAL)" },
        { 0, NEXUS_VideoFormat_e576p, "576p" },
        { 0, NEXUS_VideoFormat_e3840x2160p24hz, "3840x2160 24Hz" },
        { 0, NEXUS_VideoFormat_e3840x2160p25hz, "3840x2160 25Hz" },
        { 0, NEXUS_VideoFormat_e3840x2160p30hz, "3840x2160 30Hz" },
        { 0, NEXUS_VideoFormat_e3840x2160p50hz, "3840x2160 50Hz" },
        { 0, NEXUS_VideoFormat_e3840x2160p60hz, "3840x2160 60Hz" },

        /* END of ARRAY */
        { 0, 0, "" }
    };

    while (curarg < argc) {
        if (!strcmp(argv[curarg], "-exit")) {
            exitTest = 1 ;
        }
        else if (!strcmp(argv[curarg], "-rxedid"))
        {
            BDBG_WRN(("Attached Rx EDID is copied/used by the board's HDMI Rx port by default")) ;
            BDBG_WRN(("Force usage of internally/declared EDID with the -defaultEdid option")) ;
            useAttachedRxEdid = true ;
        }
        else if (!strcmp(argv[curarg], "-defaultEdid"))
        {
            useAttachedRxEdid = false ;
        }
        else if (!strcmp(argv[curarg], "-decodeAudio"))
        {
            decodeAudio = true ;
        }
        else if (!strcmp(argv[curarg], "-dualAudio"))
        {
            dualAudio = true ;
        }
        else if (!strcmp(argv[curarg], "-hdmiOutOnly"))
        {
            hdmiOutOnly = true ;
        }

        curarg++;
    }

    /* Bring up all modules for a platform in a default configuration for this platform */
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
        platformSettings.openFrontend = false;
        platformSettings.audioModuleSettings.numPcmBuffers += 3; /* allow support for up to 7.1ch pcm input data */
        platformSettings.audioModuleSettings.numCompressed4xBuffers += 1; /* for DDP dual audio mode */
        platformSettings.audioModuleSettings.numCompressed16xBuffers += 1; /* for HBR dual audio mode */
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);

    hdmiOutput = platformConfig.outputs.hdmi[0];

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
       hdmiInputSettings.frontend.hpdDisconnected = false ;


    do
    {
        /* check for connected downstream device */
        rc = NEXUS_HdmiOutput_GetStatus(platformConfig.outputs.hdmi[0], &hdmiOutputStatus);
        if (rc) BERR_TRACE(rc);
        if ( !hdmiOutputStatus.connected )
        {
            BDBG_WRN(("Waiting for HDMI Tx Device"));
            BKNI_Sleep(250);
        }
        else
        {
            break;
        }
    } while ( timeout-- > 0 );

    if ((hdmiOutputStatus.connected) && (useAttachedRxEdid))
    {
        NEXUS_HdmiOutputEdidBlock edidBlock;
        NEXUS_HdmiOutputBasicEdidData hdmiOutputBasicEdidData;
        uint8_t *attachedRxEdid;
        uint16_t rxEdidBytes ;
        unsigned i, j;

        /* Get EDID of attached receiver*/
        rc = NEXUS_HdmiOutput_GetBasicEdidData(platformConfig.outputs.hdmi[0], &hdmiOutputBasicEdidData);
        if (rc)
        {
            BDBG_ERR(("Error retrieving EDID from attached receiver"));
            return -1;
        }

        /* allocate space to hold the EDID blocks */
        rxEdidBytes = (hdmiOutputBasicEdidData.extensions+1) * sizeof(edidBlock.data) ;
        attachedRxEdid = BKNI_Malloc(rxEdidBytes) ;


        for (i=0; i<= hdmiOutputBasicEdidData.extensions; i++)
        {
            rc = NEXUS_HdmiOutput_GetEdidBlock(platformConfig.outputs.hdmi[0], i, &edidBlock);
            if (rc)
            {
                BDBG_ERR(("Error retrieving EDID from attached receiver"));
                attachedRxEdid = 0 ;
                rxEdidBytes = 0 ;
                break ;
            }

            for (j=0; j < sizeof(edidBlock.data); j++) {
                attachedRxEdid[i*sizeof(edidBlock.data)+j] = edidBlock.data[j];
            }
        }

        /* Load hdmiInput EDID RAM with the EDID from the Rx attached to the hdmiOutput (Tx) . */
        hdmiInput = NEXUS_HdmiInput_OpenWithEdid(hdmiInputIndex,
            &hdmiInputSettings, attachedRxEdid, rxEdidBytes) ;

        /* release memory resources */
        if (useAttachedRxEdid)
            BKNI_Free(attachedRxEdid);
    }
    else  /* No Attached Rx or user requested the internal EDID */
    {
        /* Load EDID RAM with the internal declared EDID. */
        hdmiInput = NEXUS_HdmiInput_Open(hdmiInputIndex, &hdmiInputSettings) ;
    }

    if (!hdmiInput)
    {
        fprintf(stderr, "Can't get hdmi input\n");
        return -1;
    }

    NEXUS_Display_GetDefaultSettings(&displaySettings);
    displaySettings.format = NEXUS_VideoFormat_e1080i;
    display = NEXUS_Display_Open(0, &displaySettings);

    NEXUS_HdmiInput_GetSettings(hdmiInput, &hdmiInputSettings) ;
        hdmiInputSettings.avMuteChanged.callback = avmute_changed;
        hdmiInputSettings.avMuteChanged.context = hdmiInput ;

        hdmiInputSettings.sourceChanged.callback = source_changed;
            sourceChangeCbParams.hdmiInput = hdmiInput ;
            sourceChangeCbParams.display = display ;
        hdmiInputSettings.sourceChanged.context = &sourceChangeCbParams ;
    NEXUS_HdmiInput_SetSettings(hdmiInput, &hdmiInputSettings) ;

    {
        NEXUS_HdmiInputHdcpSettings hdmiInputHdcpSettings ;
#if HDCP_SUPPORT_ON_HDMI_INPUT
        NEXUS_HdmiInputHdcpStatus hdmiInputHdcpStatus ;
#endif

        NEXUS_HdmiInput_HdcpGetDefaultSettings(hdmiInput, &hdmiInputHdcpSettings) ;
            /*
            -- for this sample app override the HDCP repeater functionality
            -- so hdmi in to hdmi out path can be tested
            --
            -- in a normal application, overriding the HDCP Repeater is a violation of the HDCP license
            -- See hdmi_repeater_passthrough app for full HDCP support.
            */
            hdmiInputHdcpSettings.repeater = false  ;

#if HDCP_SUPPORT_ON_HDMI_INPUT
            hdmiInputHdcpSettings.hdcpRxChanged.callback = hdmiInputHdcpStateChanged ;
            hdmiInputHdcpSettings.hdcpRxChanged.context = hdmiInput ;
            hdmiInputHdcpSettings.hdcpRxChanged.param = 0 ;
 #endif

        NEXUS_HdmiInput_HdcpSetSettings(hdmiInput, &hdmiInputHdcpSettings) ;

#if HDCP_SUPPORT_ON_HDMI_INPUT
        /* get status of HDCP Keys stored */
        NEXUS_HdmiInput_HdcpGetStatus(hdmiInput, &hdmiInputHdcpStatus) ;
        if (hdmiInputHdcpStatus.eOtpState != NEXUS_HdmiInputHdcpKeySetOtpState_eCrcMatch)
        {
            BDBG_ERR(("HDCP Keys are not available")) ;
            goto shutdown ;
        }
#endif
    }

#if HDCP_SUPPORT_ON_HDMI_OUTPUT
    {
        NEXUS_HdmiOutputHdcpSettings hdmiOutputHdcpSettings;

        NEXUS_HdmiOutput_GetHdcpSettings(hdmiOutput, &hdmiOutputHdcpSettings);
            /* If you want to use a custom key, insert that here */
            hdmiOutputHdcpSettings.successCallback.callback = hdmiOutputHdcpStateChanged;
            hdmiOutputHdcpSettings.successCallback.context = hdmiOutput;
            hdmiOutputHdcpSettings.successCallback.param = true;

            hdmiOutputHdcpSettings.failureCallback.callback = hdmiOutputHdcpStateChanged;
            hdmiOutputHdcpSettings.failureCallback.context = hdmiOutput;
            hdmiOutputHdcpSettings.failureCallback.param = false;
        NEXUS_HdmiOutput_SetHdcpSettings(hdmiOutput, &hdmiOutputHdcpSettings);

        NEXUS_HdmiOutput_SetHdcpRevokedKsvs(hdmiOutput,
            RevokedKsvs, NumRevokedKsvs) ;
    }
#endif

    window = NEXUS_VideoWindow_Open(display, 0);
    NEXUS_Display_AddOutput(display, NEXUS_HdmiOutput_GetVideoConnector(hdmiOutput));

    /* Install hotplug callback -- video only for now */
    NEXUS_HdmiOutput_GetSettings(hdmiOutput, &hdmiOutputSettings);
        hdmiOutputSettings.hotplugCallback.callback = hotplug_callback;
            hotPlugCbParams.hdmiOutput = hdmiOutput ;
            hotPlugCbParams.display = display ;
        hdmiOutputSettings.hotplugCallback.context = &hotPlugCbParams ;
    NEXUS_HdmiOutput_SetSettings(hdmiOutput, &hdmiOutputSettings);

    /* add audio support */
    if ( decodeAudio || dualAudio )
    {
        audioDecoder = NEXUS_AudioDecoder_Open(0, NULL);
        NEXUS_AudioDecoder_GetDefaultStartSettings(&audioProgram);
        audioProgram.input = NEXUS_HdmiInput_GetAudioConnector(hdmiInput);
        audioProgram.latencyMode = NEXUS_AudioDecoderLatencyMode_eLowest;
        /* TSM no longer required for input -> decode
        {                                                                                   ;
        NEXUS_StcChannelSettings stcSettings;
        NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
        stcSettings.timebase = NEXUS_Timebase_e0;
        stcSettings.autoConfigTimebase = false;
        stcSettings.mode = NEXUS_StcChannelMode_eAuto;
        stcSettings.modeSettings.Auto.behavior = NEXUS_StcChannelAutoModeBehavior_eAudioMaster;
        stcSettings.modeSettings.Auto.transportType = NEXUS_TransportType_eMpeg2Pes;
        audioProgram.stcChannel = NEXUS_StcChannel_Open(NEXUS_ANY_ID, &stcSettings);
        }
        */
        if ( !dualAudio )
        {
            NEXUS_AudioOutput_AddInput(NEXUS_HdmiOutput_GetAudioConnector(hdmiOutput), NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioConnectorType_eMultichannel));
        }
        if ( !hdmiOutOnly )
        {
            #if NEXUS_NUM_SPDIF_OUTPUTS
            NEXUS_AudioOutput_AddInput(NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]), NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioConnectorType_eStereo));
            #endif
            #if NEXUS_NUM_AUDIO_DACS
            NEXUS_AudioOutput_AddInput(NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]), NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioConnectorType_eStereo));
            #endif
        }
    }

    if ( !decodeAudio || dualAudio )
    {
        inputCapture = NEXUS_AudioInputCapture_Open(0, NULL);
        NEXUS_AudioInputCapture_GetDefaultStartSettings(&inputCaptureStartSettings);
        inputCaptureStartSettings.input = NEXUS_HdmiInput_GetAudioConnector(hdmiInput);
        NEXUS_AudioOutput_AddInput(NEXUS_HdmiOutput_GetAudioConnector(hdmiOutput), NEXUS_AudioInputCapture_GetConnector(inputCapture));
    }

    NEXUS_VideoWindow_AddInput(window, NEXUS_HdmiInput_GetVideoConnector(hdmiInput));
    enable_audio(hdmiOutput);

    graphicsOverlayEnabled  = false ;
    avEnabled = true ;


    if (exitTest)
    {
        srand(time(NULL)) ;

        waitTime = rand() % 9 + 1;
        BDBG_WRN(("Randomly wait %d seconds", waitTime)) ;
        BKNI_Sleep(waitTime * 1000) ;
        goto exit_menu ;
    }

    for (i=0;;i++)
    {
        unsigned char chr ;
        unsigned int menuSelection;
        unsigned int formatSelection;

        /* Find Current Format */
        NEXUS_Display_GetSettings(display, &displaySettings);
        for ( j = 0; testFormats[j].format ; j++)
        {
            if ( displaySettings.format == testFormats[j].format ) { break; }
        }

        printf("Main Menu\n");

        printf("1) Change Video Format %s%s\n", i ? "-> Currently " : "" ,i ? testFormats[j].name : "" );
        printf("2) Restart Audio\n");
        printf("3) hdmiInput Status\n");
        printf("4) hdmiOutput Status\n");
        printf("5) Enable/Update Graphics\n");
        printf("6) Toggle Audio Mute\n");
        printf("7) Toggle Video Window\n");

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
            NEXUS_HdmiOutput_GetStatus(platformConfig.outputs.hdmi[0], &hdmiOutputStatus);

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
                    NEXUS_Display_GetSettings(display, &displaySettings);
                        displaySettings.format = testFormats[j].format;
                    rc = NEXUS_Display_SetSettings(display, &displaySettings);
                    if (rc)
                    {
                        printf("ERROR changing format\n") ;
                    }
                    break;
                }
            }
            break;

        case 2:
            disable_audio(hdmiOutput);
            enable_audio(hdmiOutput);
            break;
        case 3:  hdmi_input_status() ;  break ;
        case 4:  hdmi_output_status() ;  break;

        case 5:
            if (!graphicsOverlayEnabled)
            {
               /* enable graphics overlay */
                setupGraphics(display, &displaySettings) ;
                graphicsOverlayEnabled = true ;
            }
            else
                updateGraphics() ;
            break;

        case 6:
            mute_audio(hdmiOutput);
            break;

        case 7:
            if(connectVideoWindow)
            {
                NEXUS_VideoWindow_RemoveInput(window, NEXUS_HdmiInput_GetVideoConnector(hdmiInput));
                NEXUS_VideoWindow_Close(window);
            }
            else
            {
                window = NEXUS_VideoWindow_Open(display, 0);
                NEXUS_VideoWindow_AddInput(window, NEXUS_HdmiInput_GetVideoConnector(hdmiInput));
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
    if (exitTest)
    {
        waitTime = rand() % 2 + 1;
        if (waitTime == 2)
        {
            BDBG_WRN(("\n\n\n\n\nExit without formal NEXUS shutdown (%d)", waitTime)) ;
            return 0 ;
        }
    }

#if HDCP_SUPPORT_ON_HDMI_INPUT
shutdown :
#endif

    if (avEnabled)
    {
        disable_audio(hdmiOutput);

        NEXUS_VideoWindow_RemoveInput(window, NEXUS_HdmiInput_GetVideoConnector(hdmiInput));
        NEXUS_VideoWindow_Close(window);
        NEXUS_Display_RemoveOutput(display, NEXUS_HdmiOutput_GetVideoConnector(hdmiOutput));

        /* stop/remove HDMI callbacks associated with display,
        so those callbacks do not access display once it is removed */
        NEXUS_StopCallbacks(hdmiOutput);
        NEXUS_StopCallbacks(hdmiInput);

        NEXUS_Display_Close(display) ;

        NEXUS_AudioOutput_RemoveAllInputs(NEXUS_HdmiOutput_GetAudioConnector(hdmiOutput));
        if ( decodeAudio || dualAudio )
        {
            NEXUS_AudioInput_Shutdown(NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioConnectorType_eStereo));
            NEXUS_AudioInput_Shutdown(NEXUS_HdmiInput_GetAudioConnector(hdmiInput));
            NEXUS_AudioDecoder_Close(audioDecoder);
            if ( audioProgram.stcChannel )
            {
            NEXUS_StcChannel_Close(audioProgram.stcChannel);
            }
        }

        if ( !decodeAudio || dualAudio )
        {
            NEXUS_AudioInput_Shutdown(NEXUS_AudioInputCapture_GetConnector(inputCapture));
            NEXUS_AudioInput_Shutdown(NEXUS_HdmiInput_GetAudioConnector(hdmiInput));
            NEXUS_AudioInputCapture_Close(inputCapture);
        }
    }

    shutdownGraphics() ;

    NEXUS_HdmiInput_Close(hdmiInput) ;

    NEXUS_Platform_Uninit();
    return 0;
}
#else
#include <stdio.h>
int main(void)
{
    printf("This application is not supported on this platform\n");
    return 0;
}
#endif
