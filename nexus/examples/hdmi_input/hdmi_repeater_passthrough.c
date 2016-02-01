/***************************************************************************
*     (c)2004-2013 Broadcom Corporation
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
#if NEXUS_HAS_HDMI_OUTPUT && NEXUS_HAS_HDMI_INPUT

#include "nexus_pid_channel.h"
#include "nexus_display.h"
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
#include "nexus_core_utils.h"

#include "bstd.h"
#include "bkni.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

BDBG_MODULE(repeater_passthrough) ;

#if NEXUS_NUM_AUDIO_INPUT_CAPTURES
#include "nexus_audio_input_capture.h"
static NEXUS_AudioInputCaptureHandle inputCapture;
static NEXUS_AudioInputCaptureStartSettings inputCaptureStartSettings;

static bool hdmiCompressedAudio = false ;
static bool hdmiOutputHdcpEnabled = false ;

static    NEXUS_PlatformConfiguration platformConfig;


static NEXUS_DisplayHandle display;
static NEXUS_HdmiInputHandle hdmiInput;
static NEXUS_HdmiOutputHandle hdmiOutput;



static bool force_exit  = false ;
static bool compliance_test  = false ;
/**********************
** NOTE:
	If using the QuantumData 882EA for compliance testing,
		please enable the work around by running the application with option "-qd882"
			i.e.: nexus hdmi_repeater_passthrough -qd882

* This is a work around to pass test 3C-II-06 on the QuantumData 882. In this test,
* the QD sets the device count to max but also does not provide the calculated V' value,
* which leads to the V mismatch error
**********************/
static bool qd882EA_work_around = false;


static void initializeHdmiOutputHdcpSettings(void) ;


/*****************************/
/* NOTE:                     */
/* This app requires the usage of both HDCP Tx and Rx Keysets */
/* The HDCP Tx Keys are declared in this sample APP */
/* The HDCP Rx Keys are declared/loaded by the hdmi_input_hdcp_keyloader.c app */
/* The hdmi_input_hdcp_keyloader.c app needs to be run only once */
/*     after a reset or power cycle */
/* HDCP Tx and Rx Keys are not interchangeable */
/* Both types must be purchased */
/*****************************/

#define USE_PRODUCTION_KEYS 0


#if USE_PRODUCTION_KEYS
/*****************************/
/* INSERT PRODUCTION KeySet HERE */
/*****************************/

#else


/**************************************/
/* HDCP Specification Test Key Set    */
/*                                    */
/* NOTE: the default declared Test    */
/* KeySet below is from the HDCP Spec */
/* and it *IS NOT* compatible with    */
/* production devices                 */
/**************************************/


static NEXUS_HdmiOutputHdcpKsv hdcpTxAksv =
{	{0x14, 0xF7, 0x61, 0x03, 0xB7} };

static NEXUS_HdmiOutputHdcpKey encryptedTxKeySet[NEXUS_HDMI_OUTPUT_HDCP_NUM_KEYS] =
{
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0x691e138f, 0x58a44d00},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0x0950e658, 0x35821f00},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0x0d98b9ab, 0x476a8a00},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0xcac5cb52, 0x1b18f300},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0xb4d89668, 0x7f14fb00},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0x818f4878, 0xc98be000},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0x412c11c8, 0x64d0a000},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0x44202428, 0x5a9db300},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0x6b56adbd, 0xb228b900},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0xf6e46c4a, 0x7ba49100},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0x589d5e20, 0xf8005600},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0xa03fee06, 0xb77f8c00},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0x28bc7c9d, 0x8c2dc000},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0x059f4be5, 0x61125600},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0xcbc1ca8c, 0xdef07400},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0x6adbfc0e, 0xf6b83b00},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0xd72fb216, 0xbb2ba000},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0x98547846, 0x8e2f4800},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0x38472762, 0x25ae6600},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0xf2dd23a3, 0x52493d00},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0x543a7b76, 0x31d2e200},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0x2561e6ed, 0x1a584d00},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0xf7227bbf, 0x82603200},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0x6bce3035, 0x461bf600},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0x6b97d7f0, 0x09043600},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0xf9498d61, 0x05e1a100},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0x063405d1, 0x9d8ec900},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0x90614294, 0x67c32000},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0xc34facce, 0x51449600},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0x8a8ce104, 0x45903e00},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0xfc2d9c57, 0x10002900},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0x80b1e569, 0x3b94d700},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0x437bdd5b, 0xeac75400},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0xba90c787, 0x58fb7400},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0xe01d4e36, 0xfa5c9300},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0xae119a15, 0x5e070300},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0x01fb788a, 0x40d30500},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0xb34da0d7, 0xa5590000},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0x409e2c4a, 0x633b3700},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0x412056b4, 0xbb732500}
} ;

 #endif


/*
from HDCP Spec:
Table 51 gives the format of the HDCP SRM. All values are stored in big endian format.

Specify KSVs here in big endian;
*/
#define NUM_REVOKED_KSVS 3
static uint8_t NumRevokedKsvs = NUM_REVOKED_KSVS ;
static const NEXUS_HdmiOutputHdcpKsv RevokedKsvs[NUM_REVOKED_KSVS] =
{
    /* MSB ... LSB */
    {{0xa5, 0x1f, 0xb0, 0xc3, 0x72}},
    {{0x65, 0xbf, 0x04, 0x8a, 0x7c}},
    {{0x65, 0x65, 0x1e, 0xd5, 0x64}}
} ;

static uint8_t *hdmiRxPortEdid ;
static uint16_t hdmiRxPortEdidSize ;


void source_changed(void *context, int param)
{
    NEXUS_Error errCode ;
    NEXUS_HdmiInputHandle hdmiInput ;

    NEXUS_HdmiInputStatus hdmiInputStatus ;
    NEXUS_HdmiOutputSettings hdmiOutputSettings ;
    NEXUS_DisplaySettings displaySettings ;
    BSTD_UNUSED(param);

    hdmiInput = (NEXUS_HdmiInputHandle) context ;

    NEXUS_HdmiInput_GetStatus(hdmiInput, &hdmiInputStatus) ;
    if (!hdmiInputStatus.validHdmiStatus)
    {
        if (hdmiOutputHdcpEnabled)
        {
            NEXUS_HdmiOutput_DisableHdcpAuthentication(hdmiOutput);
            hdmiOutputHdcpEnabled = false ;
        }
        return ;
    }

    NEXUS_Display_GetSettings(display, &displaySettings);
    if ( displaySettings.format != hdmiInputStatus.originalFormat )
    {

        BDBG_LOG(("Video Format Change - Updating to %u", hdmiInputStatus.originalFormat));

        displaySettings.format = hdmiInputStatus.originalFormat;
        errCode = NEXUS_Display_SetSettings(display, &displaySettings);
        if (errCode) {
            BDBG_ERR(("Unable to set Display Settings (errCode= %d)", errCode)) ;
            return ;
        }
    }

    NEXUS_HdmiOutput_GetSettings(hdmiOutput, &hdmiOutputSettings) ;
        /* debug messages */
        if (hdmiInputStatus.colorSpace !=  hdmiOutputSettings.colorSpace)
        {
            BDBG_MSG(("hdmiInput Colorspace: %d", hdmiInputStatus.colorSpace)) ;
            BDBG_MSG(("hdmiOutput Colorspace: %d", hdmiOutputSettings.colorSpace)) ;
        }

        if (hdmiInputStatus.colorDepth !=  hdmiOutputSettings.colorDepth)
        {
            BDBG_MSG(("hdmiInput Colordepth: %d", hdmiInputStatus.colorDepth)) ;
            BDBG_MSG(("hdmiOutput Colordepth: %d", hdmiOutputSettings.colorDepth)) ;
        }

        /* update colordepth/colorspace */
        hdmiOutputSettings.colorDepth = hdmiInputStatus.colorDepth ;
        hdmiOutputSettings.colorSpace = hdmiInputStatus.colorSpace ;
    errCode = NEXUS_HdmiOutput_SetSettings(hdmiOutput, &hdmiOutputSettings) ;
    if (errCode)
    {
        BDBG_ERR(("Unable to set HdmiOutputSettings")) ;
    }

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
        BDBG_WRN(("avmute_changed callback: Unable to get hdmiInput status")) ;
        return ;
    }

    BDBG_WRN(("avmute_changed callback: %s\n",
    hdmiInputStatus.avMute ? "Set_AvMute" : "Clear_AvMute")) ;

}


void hdmiInputHdcpStateChanged(void *context, int param)
{
	NEXUS_HdmiInputHandle hdmiInput ;
	NEXUS_HdmiInputHdcpStatus hdmiInputHdcpStatus ;
	NEXUS_HdmiOutputHdcpStatus outputHdcpStatus;
    NEXUS_Error rc ;

	BSTD_UNUSED(param) ;

	hdmiInput = (NEXUS_HdmiInputHandle) context ;

	/* check the authentication state and process accordingly */
	NEXUS_HdmiInput_HdcpGetStatus(hdmiInput, &hdmiInputHdcpStatus) ;

        switch (hdmiInputHdcpStatus.eAuthState)
        {
        case NEXUS_HdmiInputHdcpAuthState_eKeysetInitialization :

            BDBG_WRN(("Change in HDCP Key Set detected")) ;
            switch (hdmiInputHdcpStatus.eKeyStorage)
            {
            case NEXUS_HdmiInputHdcpKeyStorage_eOtpROM :
                BDBG_WRN(("HDCP KeySet stored in OTP ROM")) ;
                break ;

            case NEXUS_HdmiInputHdcpKeyStorage_eOtpRAM :
                BDBG_WRN(("HDCP Keys stored in OTP RAM")) ;
                break ;

            default :
                BDBG_WRN(("Unknown Key Storage type %d", hdmiInputHdcpStatus.eKeyStorage));
            }

            BDBG_WRN(("NOTE: EACH DEVICE REQUIRES A UNIQUE HDCP KEY SET; The same KeySet cannot be used in multiple devices\n\n")) ;

		break ;

        case NEXUS_HdmiInputHdcpAuthState_eWaitForKeyloading :
            BDBG_WRN(("Upstream HDCP Authentication request ...")) ;
        rc = NEXUS_HdmiOutput_StartHdcpAuthentication(hdmiOutput);
        if (rc) BERR_TRACE(rc) ;
            break ;

        case NEXUS_HdmiInputHdcpAuthState_eWaitForDownstreamKsvs :
		BDBG_WRN(("Downstream FIFO Request; Start hdmiOutput Authentication...")) ;

		NEXUS_HdmiOutput_GetHdcpStatus(platformConfig.outputs.hdmi[0], &outputHdcpStatus);
		if ((outputHdcpStatus.hdcpState != NEXUS_HdmiOutputHdcpState_eWaitForRepeaterReady)
			&& (outputHdcpStatus.hdcpState != NEXUS_HdmiOutputHdcpState_eCheckForRepeaterReady))
		{
            rc = NEXUS_HdmiOutput_StartHdcpAuthentication(hdmiOutput);
            if (rc) BERR_TRACE(rc) ;
		}
		break ;

        default :
            BDBG_WRN(("Unknown State %d", hdmiInputHdcpStatus.eAuthState )) ;
	}
}


static void hdmiOutputHdcpStateChanged(void *pContext, int param)
{
    bool success = (bool)param;
    NEXUS_HdmiOutputHandle handle = pContext;

    NEXUS_HdmiOutputHdcpStatus hdmiOutputHdcpStatus;
    NEXUS_HdmiHdcpDownStreamInfo downStream  ;
    NEXUS_HdmiHdcpKsv *pKsvs ;
    NEXUS_Error rc ;

    unsigned returnedDevices ;
    uint8_t i ;

    NEXUS_HdmiOutput_GetHdcpStatus(handle, &hdmiOutputHdcpStatus);

    if (!success )
    {
        BDBG_WRN(("\nHDCP Authentication Failed.  Current State %d, last error =%d\n",
            hdmiOutputHdcpStatus.hdcpState, hdmiOutputHdcpStatus.hdcpError)) ;

		switch (hdmiOutputHdcpStatus.hdcpError)
		{

		case NEXUS_HdmiOutputHdcpError_eRxDevicesExceeded:
		case NEXUS_HdmiOutputHdcpError_eRepeaterDepthExceeded:
			goto uploadDownstreamInfo;
			break;

		default:
			break;
		}

    	goto retryAuthentication;
    }


    BDBG_WRN(("HDCP Tx Authentication Successful")) ;
    hdmiOutputHdcpEnabled = true ;

/* Load Rx KSV FIFO for upstream device */
uploadDownstreamInfo:

    NEXUS_HdmiOutput_HdcpGetDownstreamInfo(handle, &downStream) ;

    /* allocate space to hold ksvs for the downstream devices */
    pKsvs =
	 BKNI_Malloc((downStream.devices) * NEXUS_HDMI_HDCP_KSV_LENGTH) ;

    NEXUS_HdmiOutput_HdcpGetDownstreamKsvs(handle,
        pKsvs, downStream.devices, &returnedDevices) ;

    BDBG_WRN(("hdmiOutput Downstream Levels:  %d  Devices: %d",
        downStream.depth, downStream.devices)) ;

    /* display the downstream device KSVs */
	for (i = 0 ; i <= downStream.devices; i++)
   {
        BDBG_MSG(("Device %02d BKsv: %02X %02X %02X %02X %02X",
            i + 1,
            *(pKsvs->data + i + 4), *(pKsvs->data + i + 3),
            *(pKsvs->data + i + 2), *(pKsvs->data + i + 1),
            *(pKsvs->data + i ))) ;
    }

    NEXUS_HdmiInput_HdcpLoadKsvFifo(hdmiInput,
		&downStream, pKsvs, downStream.devices) ;

    BKNI_Free(pKsvs) ;


retryAuthentication:
	/* when running compliance tests, always retry on failures */
	if (!success && compliance_test)
	{
	    rc = NEXUS_HdmiOutput_StartHdcpAuthentication(hdmiOutput) ;
	    if (rc) BERR_TRACE(rc) ;
	}

	return ;

}


static void disable_audio(NEXUS_HdmiOutputHandle hdmiOutput)
{
    NEXUS_AudioInputCapture_Stop(inputCapture);
    NEXUS_AudioOutput_RemoveAllInputs(NEXUS_HdmiOutput_GetAudioConnector(hdmiOutput));
}

static void enable_audio(NEXUS_HdmiOutputHandle hdmiOutput)
{
    NEXUS_AudioInput connector;

    NEXUS_AudioOutput_RemoveAllInputs(NEXUS_HdmiOutput_GetAudioConnector(hdmiOutput));

    connector = NEXUS_AudioInputCapture_GetConnector(inputCapture);

    NEXUS_AudioOutput_AddInput(NEXUS_HdmiOutput_GetAudioConnector(hdmiOutput), connector);

    NEXUS_AudioInputCapture_Start(inputCapture, &inputCaptureStartSettings);
}

static void hotplug_callback(void *pParam, int iParam)
{
    NEXUS_HdmiOutputStatus status;
    NEXUS_HdmiOutputHandle hdmiOutput = pParam;
    NEXUS_DisplayHandle display = (NEXUS_DisplayHandle)iParam;
    NEXUS_HdmiOutputBasicEdidData hdmiOutputBasicEdidData;
    NEXUS_HdmiOutputEdidBlock edidBlock;
    uint8_t *attachedRxEdid = NULL ;
    uint16_t attachedRxEdidSize ;
    uint8_t i, j;

    NEXUS_Error rc = NEXUS_SUCCESS;

    NEXUS_HdmiOutput_GetStatus(hdmiOutput, &status);
    BDBG_LOG(("hotplug_callback: %s\n", status.connected ?
		"DEVICE CONNECTED" : "DEVICE REMOVED")) ;

    if ( status.connected )
    {
        /* app can update the Rx EDID if a new device has been attached */
    }
    else
    {
        /* app can restore the Rx internal EDID if device is no longer connected to the Tx */
    }

    /* the app can choose to switch to the preferred format, but it's not required. */
    if ( status.connected )
    {
        /* Get EDID of attached receiver*/
        rc = NEXUS_HdmiOutput_GetBasicEdidData(hdmiOutput, &hdmiOutputBasicEdidData);
        if (rc)
        {
            BDBG_ERR(("Unable to get downstream EDID; Use declared EDID in app for repeater's EDID")) ;
            /* error getting EDID of attached Rx; use the internal EDID instead */
            hdmiRxPortEdid = NULL ;
            hdmiRxPortEdidSize = 0 ;

            goto load_edid ;
        }

        /* allocate space to hold the EDID blocks */
        attachedRxEdidSize = (hdmiOutputBasicEdidData.extensions + 1) * sizeof(edidBlock.data) ;
        attachedRxEdid = BKNI_Malloc(attachedRxEdidSize) ;

        for (i = 0; i <= hdmiOutputBasicEdidData.extensions ; i++)
        {
            rc = NEXUS_HdmiOutput_GetEdidBlock(hdmiOutput, i, &edidBlock);
            if (rc)
            {
                BDBG_ERR(("Error retrieving EDID Block %d from attached receiver;", i));
                /* error getting EDID of attached Rx; use the internal EDID instead */
                hdmiRxPortEdid = NULL ;
                hdmiRxPortEdidSize = 0 ;

                BKNI_Free(attachedRxEdid) ;

                goto load_edid ;
            }

            /* copy EDID data */
            for (j=0; j < sizeof(edidBlock.data); j++)
                attachedRxEdid[i*sizeof(edidBlock.data)+j] = edidBlock.data[j];
        }

        hdmiRxPortEdid = attachedRxEdid ;
        hdmiRxPortEdidSize = attachedRxEdidSize;

  load_edid:

        /********************************************************/
        /* manipulation of EDID to add/remove capabilities can be done here        */
        /********************************************************/

        /*
        -- load the repeater's EDID with the selected EDID
        -- either from the attached device, the internally declared EDID, or a combination of the two)
        */
        rc = NEXUS_HdmiInput_LoadEdidData(hdmiInput, hdmiRxPortEdid, hdmiRxPortEdidSize);
        if (rc) BERR_TRACE(rc);

        if (attachedRxEdid)
        {
            /* release memory resources */
            BKNI_Free(attachedRxEdid);
        }


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

        BDBG_WRN(("Toggle Rx HOT PLUG to force upstream re-authentication...")) ;
        NEXUS_HdmiInput_ToggleHotPlug(hdmiInput) ;

    }
    else    /* device disconnected. Load internal EDID */
    {
        hdmiRxPortEdid = NULL ;
        rc = NEXUS_HdmiInput_LoadEdidData(hdmiInput, hdmiRxPortEdid, 0);
        if (rc) BERR_TRACE(rc);
    }
}

void hdmi_input_status(void )
{
    NEXUS_HdmiInputStatus status ;
    BDBG_Level saveLevel ;
    static const char *textColorSpace[] =
    {
        "RGB ",
        "YCbCr 4:2:2",
        "YCbCr 4:4:4",
        "Max",
    } ;

    BDBG_GetModuleLevel("repeater_passthrough", &saveLevel) ;

    BDBG_SetModuleLevel("repeater_passthrough", BDBG_eMsg) ;

    NEXUS_HdmiInput_GetStatus(hdmiInput, &status) ;
    if (!status.validHdmiStatus)
    {
        BDBG_WRN(("Cannot determine input status...")) ;
        return  ;
    }

    BDBG_MSG(("hdmiInput Mode  : %s", status.hdmiMode ? "HDMI" : "DVI")) ;
    BDBG_MSG(("hdmiInput Format: %d x %d %c %s",
		status.avWidth, status.avHeight, status.interlaced ? 'i' : 'p',
		textColorSpace[status.colorSpace])) ;


    BDBG_MSG(("hdmiInput Clock : %d", status.lineClock)) ;
    BDBG_MSG(("HDCP Enabled    : %s", status.hdcpRiUpdating ? "Yes" : "No")) ;

    /* restore debug level */
    BDBG_SetModuleLevel("repeater_passthrough", saveLevel) ;

}


static void initializeHdmiOutputHdcpSettings(void)
{
    NEXUS_HdmiOutputHdcpSettings hdmiOutputHdcpSettings;

    NEXUS_HdmiOutput_GetHdcpSettings(hdmiOutput, &hdmiOutputHdcpSettings);

        /* copy the encrypted key set and its Aksv here  */
        BKNI_Memcpy(hdmiOutputHdcpSettings.encryptedKeySet, encryptedTxKeySet,
            NEXUS_HDMI_OUTPUT_HDCP_NUM_KEYS * sizeof(NEXUS_HdmiOutputHdcpKey)) ;
        BKNI_Memcpy(&hdmiOutputHdcpSettings.aksv, &hdcpTxAksv,
            NEXUS_HDMI_OUTPUT_HDCP_KSV_LENGTH) ;

        /* install HDCP success  callback */
        hdmiOutputHdcpSettings.successCallback.callback = hdmiOutputHdcpStateChanged ;
        hdmiOutputHdcpSettings.successCallback.context =  hdmiOutput ;
        hdmiOutputHdcpSettings.successCallback.param = true;

        /* install HDCP failure callback */
        hdmiOutputHdcpSettings.failureCallback.callback = hdmiOutputHdcpStateChanged ;
        hdmiOutputHdcpSettings.failureCallback.context = hdmiOutput ;
        hdmiOutputHdcpSettings.failureCallback.param = false;

    NEXUS_HdmiOutput_SetHdcpSettings(hdmiOutput, &hdmiOutputHdcpSettings);

    /* install list of revoked KSVs from SRMs (System Renewability Message) if available */
    NEXUS_HdmiOutput_SetHdcpRevokedKsvs(hdmiOutput,
        RevokedKsvs, NumRevokedKsvs) ;

 }

#endif

int main(int argc, char **argv)
{
#if NEXUS_HAS_HDMI_INPUT
    NEXUS_Error rc;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_VideoWindowHandle window;
    NEXUS_HdmiInputSettings hdmiInputSettings;
    NEXUS_HdmiOutputSettings hdmiOutputSettings;
    NEXUS_HdmiOutputStatus hdmiOutputStatus;

    NEXUS_TimebaseSettings timebaseSettings;
    NEXUS_PlatformSettings platformSettings ;

    unsigned hdmiInputIndex = 0;

    NEXUS_HdmiOutputBasicEdidData hdmiOutputBasicEdidData;
    uint8_t *attachedRxEdid;
    uint16_t attachedRxEdidSize ;
    unsigned timeout=4;

    BDBG_Level debugLevel ;

    int curarg = 1;


    while (curarg < argc) {
        if (!strcmp(argv[curarg], "-exit")) {
            force_exit = true  ;
        }
        else if (!strcmp(argv[curarg], "-compliance")) {
            compliance_test = true  ;
        }
		else if (!strcmp(argv[curarg], "-qd882")) {
			qd882EA_work_around = true;
		}
       curarg++;
    }

    /* Bring up all modules for a platform in a default configuration for this platform */

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
        platformSettings.openFrontend = false;
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

    /* check for connected downstream device */
    do
    {
        /* check for connected downstream device */
        rc = NEXUS_HdmiOutput_GetStatus(hdmiOutput, &hdmiOutputStatus);
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

    if (hdmiOutputStatus.connected)
    {
        NEXUS_HdmiOutputEdidBlock edidBlock;
        unsigned i, j;

        /* Get EDID of attached receiver*/
        rc = NEXUS_HdmiOutput_GetBasicEdidData(hdmiOutput, &hdmiOutputBasicEdidData);
        if (rc)
        {
            BDBG_ERR(("Unable to get downstream EDID; Use declared EDID in app for repeater's EDID")) ;
            goto use_internal_edid ;
        }

        /* allocate space to hold the EDID blocks */
        attachedRxEdidSize = (hdmiOutputBasicEdidData.extensions + 1) * sizeof(edidBlock.data) ;
        attachedRxEdid = BKNI_Malloc(attachedRxEdidSize) ;

        for (i = 0; i <= hdmiOutputBasicEdidData.extensions ; i++)
        {
            rc = NEXUS_HdmiOutput_GetEdidBlock(hdmiOutput, i, &edidBlock);
            if (rc)
            {
                BDBG_ERR(("Error retrieving EDID Block %d from attached receiver;", i));
                        BKNI_Free(attachedRxEdid) ;

                goto use_internal_edid ;
            }

            /* copy EDID data */
            for (j = 0 ; j < sizeof(edidBlock.data) ; j++)
                attachedRxEdid[i*sizeof(edidBlock.data)+j] = edidBlock.data[j];
        }

        hdmiRxPortEdid = attachedRxEdid ;
        hdmiRxPortEdidSize = attachedRxEdidSize;

        goto open_with_edid ;

    }

use_internal_edid:
     hdmiRxPortEdid = NULL ;
     hdmiRxPortEdidSize = 0 ;

    /* fall through and open with sample EDID */

open_with_edid:

    /********************************************************/
    /* manipulation of EDID to add/remove capabilities can be done here        */
    /********************************************************/

    hdmiInput = NEXUS_HdmiInput_OpenWithEdid(hdmiInputIndex,
		&hdmiInputSettings, hdmiRxPortEdid, (uint16_t) hdmiRxPortEdidSize);

    if (hdmiRxPortEdid)
    {
        /* release memory resources */
        BKNI_Free(hdmiRxPortEdid);
    }

    if (!hdmiInput)
    {
        BDBG_WRN(("Can't get hdmi input\n")) ;
        return -1;
    }

    NEXUS_HdmiInput_GetSettings(hdmiInput, &hdmiInputSettings) ;
        hdmiInputSettings.avMuteChanged.callback = avmute_changed;
        hdmiInputSettings.avMuteChanged.context = hdmiInput ;

        hdmiInputSettings.sourceChanged.callback = source_changed;
        hdmiInputSettings.sourceChanged.context = hdmiInput ;
    NEXUS_HdmiInput_SetSettings(hdmiInput, &hdmiInputSettings) ;

    {
        NEXUS_HdmiInputHdcpSettings hdmiInputHdcpSettings ;

        NEXUS_HdmiInput_HdcpGetDefaultSettings(hdmiInput, &hdmiInputHdcpSettings) ;

            /* chips with both hdmi rx and tx cores should always set repeater to TRUE */
            BDBG_WRN(("HDCP default Repeater Setting: %d",
                hdmiInputHdcpSettings.repeater)) ;
            hdmiInputHdcpSettings.repeater = true ;

			/* Set Max downstream device supported to 16 as a work around for QuantumData
				issue on test 3C-II-06 */
			if (qd882EA_work_around) {
				hdmiInputHdcpSettings.maxDeviceCountSupported = 16;
			}

            hdmiInputHdcpSettings.hdcpRxChanged.callback = hdmiInputHdcpStateChanged ;
            hdmiInputHdcpSettings.hdcpRxChanged.context = hdmiInput ;
            hdmiInputHdcpSettings.hdcpRxChanged.param = 0 ;

        NEXUS_HdmiInput_HdcpSetSettings(hdmiInput, &hdmiInputHdcpSettings) ;
    }

    /* Initialize HDCP settings / keys */
    initializeHdmiOutputHdcpSettings() ;


    NEXUS_Display_GetDefaultSettings(&displaySettings);
    display = NEXUS_Display_Open(0, &displaySettings);

    window = NEXUS_VideoWindow_Open(display, 0);
    NEXUS_Display_AddOutput(display, NEXUS_HdmiOutput_GetVideoConnector(hdmiOutput));

    /* Install hotplug callback -- video only for now */
    NEXUS_HdmiOutput_GetSettings(hdmiOutput, &hdmiOutputSettings);
    hdmiOutputSettings.hotplugCallback.callback = hotplug_callback;
    hdmiOutputSettings.hotplugCallback.context = hdmiOutput;
    hdmiOutputSettings.hotplugCallback.param = (int)display;
    NEXUS_HdmiOutput_SetSettings(hdmiOutput, &hdmiOutputSettings);


    /* add audio support */
    inputCapture = NEXUS_AudioInputCapture_Open(0, NULL);
    NEXUS_AudioInputCapture_GetDefaultStartSettings(&inputCaptureStartSettings);
    inputCaptureStartSettings.input = NEXUS_HdmiInput_GetAudioConnector(hdmiInput);


    NEXUS_VideoWindow_AddInput(window, NEXUS_HdmiInput_GetVideoConnector(hdmiInput));
    enable_audio(hdmiOutput) ;

    if (force_exit)
    {
        BDBG_WRN(("\n\n\n\n")) ;
        return 0 ;
    }

    for ( ;; )
    {
        int tmp;

        BDBG_GetModuleLevel("repeater_passthrough",  &debugLevel) ;
        BDBG_SetModuleLevel("repeater_passthrough",  BDBG_eMsg) ;

			BDBG_MSG(("**************************************************"));
			BDBG_MSG(("* Please note the following run time options"));
			BDBG_MSG(("*      -compliance : required to run HDCP compliance test"));
			BDBG_MSG(("*      -qd882 : required if run HDCP compliance test on the QuantumData 882EA"));
			BDBG_MSG(("*******************************************************\n"));

            BDBG_MSG(("Main Menu"));
            BDBG_MSG(("1) Change Video Format")) ;
            BDBG_MSG(("2) Toggle PCM/Compressed audio (currently %s)",
    			hdmiCompressedAudio?"Compressed":"PCM")) ;
            BDBG_MSG(("3) hdmiInput Status")) ;

            BDBG_MSG(("0) Exit\n")) ;
            BDBG_MSG(("Enter Selection: ")) ;

        BDBG_SetModuleLevel("repeater_passthrough", debugLevel) ;


        scanf("%d", &tmp);
        switch ( tmp )
        {
        case 0:
            goto exit_menu ;
        case 1:
            NEXUS_Display_GetSettings(display, &displaySettings);
            switch ( displaySettings.format )
            {
            case NEXUS_VideoFormat_e1080p: tmp=0; break;
            case NEXUS_VideoFormat_e1080i: tmp=1; break;
            case NEXUS_VideoFormat_e720p: tmp=2; break;
            case NEXUS_VideoFormat_e480p: tmp=3; break;
            default: tmp=4; break;
            }

            BDBG_GetModuleLevel("repeater_passthrough",  &debugLevel) ;
            BDBG_SetModuleLevel("repeater_passthrough",  BDBG_eMsg) ;

                BDBG_MSG(("Current format is %d", tmp)) ;
                BDBG_MSG(("Enter new format (0=1080p 1=1080i 2=720p 3=480p 4=NTSC): ", tmp)) ;

            BDBG_SetModuleLevel("repeater_passthrough", debugLevel) ;

            scanf("%d", &tmp);
            switch ( tmp )
            {
            case 0: displaySettings.format = NEXUS_VideoFormat_e1080p; break;
            case 1: displaySettings.format = NEXUS_VideoFormat_e1080i; break;
            case 2: displaySettings.format = NEXUS_VideoFormat_e720p; break;
            case 3: displaySettings.format = NEXUS_VideoFormat_e480p; break;
            default: displaySettings.format = NEXUS_VideoFormat_eNtsc; break;
            }
            NEXUS_Display_SetSettings(display, &displaySettings);
            break;
        case 2:
            disable_audio(hdmiOutput);
            enable_audio(hdmiOutput);
            break;

        case 3:
            hdmi_input_status() ;

            break;


        default:
            break;
        }
    }

exit_menu:

    disable_audio(hdmiOutput) ;

    NEXUS_VideoWindow_RemoveInput(window, NEXUS_HdmiInput_GetVideoConnector(hdmiInput));
    NEXUS_VideoWindow_Close(window);
    NEXUS_Display_RemoveOutput(display, NEXUS_HdmiOutput_GetVideoConnector(hdmiOutput));


	/* stop/remove HDMI callbacks associated with display,
	so those callbacks do not access display once it is removed */
    NEXUS_StopCallbacks(hdmiOutput);
    NEXUS_StopCallbacks(hdmiInput);
    NEXUS_Display_Close(display) ;

    NEXUS_AudioOutput_RemoveAllInputs(NEXUS_HdmiOutput_GetAudioConnector(hdmiOutput));
    NEXUS_AudioInput_Shutdown(NEXUS_AudioInputCapture_GetConnector(inputCapture));
    NEXUS_AudioInput_Shutdown(NEXUS_HdmiInput_GetAudioConnector(hdmiInput));
    NEXUS_AudioInputCapture_Close(inputCapture);

    NEXUS_HdmiInput_Close(hdmiInput) ;

    NEXUS_Platform_Uninit();

#else
    BSTD_UNUSED(argc);
    printf("%s not supported on this platform", argv[0]) ;
#endif

    return 0;
}
#else
#include "bstd.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef NEXUS_NUM_HDMI_INPUTS
#define NEXUS_NUM_HDMI_INPUTS 0
#endif

#ifndef NEXUS_NUM_HDMI_OUTPUTS
#define NEXUS_NUM_HDMI_OUTPUTS 0
#endif

int main(int argc, char **argv)
{
    BSTD_UNUSED(argc);
	printf("%d Platform has %d HDMI Inputs and %d HDMI Outputs; App requires one of each.\n",NEXUS_PLATFORM,
           NEXUS_NUM_HDMI_INPUTS, NEXUS_NUM_HDMI_OUTPUTS);

    printf("%s not supported on the %d platform.\n", argv[0], BCHP_CHIP) ;
    return 0 ;
}
#endif

