/******************************************************************************
 *  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
/* Nexus example app: single live a/v decode from an input band, routed to hdmi output */

#include "nexus_platform.h"
#if NEXUS_NUM_HDMI_OUTPUTS && NEXUS_HAS_AUDIO
#include "nexus_pid_channel.h"
#include "nexus_parser_band.h"
#include "nexus_video_decoder.h"
#include "nexus_stc_channel.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_audio_dac.h"
#include "nexus_audio_decoder.h"
#include "nexus_audio_output.h"
#include "nexus_spdif_output.h"
#include "nexus_hdmi_output.h"
#include "nexus_component_output.h"
#include "nexus_hdmi_output_hdcp.h"
#include "nexus_core_utils.h"
#include <stdio.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <stdlib.h>
#include <string.h>
#include "bstd.h"

#include "bkni.h"

#include "nexus_file.h"
#include "nexus_playback.h"
#include "nexus_playpump.h"

#if 0
#define FILE_NAME "videos/cnnticker.mpg"
#define TRANSPORT_TYPE NEXUS_TransportType_eTs
#define VIDEO_CODEC NEXUS_VideoCodec_eMpeg2
#define AUDIO_CODEC NEXUS_AudioCodec_eMpeg
#define VIDEO_PID 0x21
#define AUDIO_PID 0x22
#else
#define FILE_NAME "videos/cnnticker.mpg"
#define TRANSPORT_TYPE NEXUS_TransportType_eTs
#define VIDEO_CODEC NEXUS_VideoCodec_eMpeg2
#define AUDIO_CODEC NEXUS_AudioCodec_eMpeg
#define VIDEO_PID 0x21
#define AUDIO_PID 0x22
#endif

static const char HDCP2x_DEFAULT_BIN[] =  "./drm.bin";
static const char HDCP1x_DEFAULT_BIN[] = "./hdcp1xKeys.bin";

BDBG_MODULE(hdmi_output_hdcp) ;

static bool hdmiMultichPCM = false;
static bool hdmiCompressedAudio = false, audioStarted = false, hdmiAvMute = false ;
static bool hdmiHdcpEnabled = false ;
static bool complianceTest = false;
static NEXUS_VideoFormat complianceTestFormat = NEXUS_VideoFormat_e480p;
static NEXUS_PlatformConfiguration platformConfig;
static NEXUS_AudioDecoderHandle pcmDecoder, compressedDecoder;
static NEXUS_AudioDecoderStartSettings audioProgram;
static NEXUS_HdmiOutputHdcpVersion version_select = NEXUS_HdmiOutputHdcpVersion_eAuto;

static NEXUS_Error initializeHdmiOutputHdcpSettings(void)  ;


/**************************************/
/* HDCP 1.x Specification Test Key Set    */
/*                                    */
/* NOTE: the default declared Test    */
/* KeySet below is from the HDCP Spec */
/* and it *IS NOT* compatible with    */
/* production devices                 */
/**************************************/
static NEXUS_HdmiOutputHdcpKsv testkey_hdcpTxAksv =
{    {0x14, 0xF7, 0x61, 0x03, 0xB7} };

static NEXUS_HdmiOutputHdcpKey testkey_encryptedTxKeySet[NEXUS_HDMI_OUTPUT_HDCP_NUM_KEYS] =
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

static NEXUS_HdmiOutputHdcpKsv hdcpTxAksv;
static NEXUS_HdmiOutputHdcpKey encryptedTxKeySet[NEXUS_HDMI_OUTPUT_HDCP_NUM_KEYS];

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

typedef struct hotplugCallbackParameters
{
    NEXUS_HdmiOutputHandle hdmi  ;
    NEXUS_DisplayHandle display ;
} hotplugCallbackParameters ;

typedef struct
{
	NEXUS_HdmiOutputHdcpError eError ;
	const char *hdcpErrorText ;
}  hdcpErrorText ;

static const hdcpErrorText  hdcpErrorTextTable[] =
{
	{NEXUS_HdmiOutputHdcpError_eSuccess,		BDBG_STRING("Success")},
	{NEXUS_HdmiOutputHdcpError_eRxBksvError,		BDBG_STRING("HDCP Rx BKsv Error")},
	{NEXUS_HdmiOutputHdcpError_eRxBksvRevoked,	BDBG_STRING("HDCP Rx BKsv/Keyset Revoked")},
	{NEXUS_HdmiOutputHdcpError_eRxBksvI2cReadError,		BDBG_STRING("HDCP I2C Read Error")},
	{NEXUS_HdmiOutputHdcpError_eTxAksvError, 	BDBG_STRING("HDCP Tx Aksv Error")},
	{NEXUS_HdmiOutputHdcpError_eTxAksvI2cWriteError,		BDBG_STRING("HDCP I2C Write Error")},
	{NEXUS_HdmiOutputHdcpError_eReceiverAuthenticationError,	BDBG_STRING("HDCP Receiver Authentication Failure")},
	{NEXUS_HdmiOutputHdcpError_eRepeaterAuthenticationError,		BDBG_STRING("HDCP Repeater Authentication Failure")},
	{NEXUS_HdmiOutputHdcpError_eRxDevicesExceeded,		BDBG_STRING("HDCP Repeater MAX Downstram Devices Exceeded")},
	{NEXUS_HdmiOutputHdcpError_eRepeaterDepthExceeded,	BDBG_STRING("HDCP Repeater MAX Downstram Levels Exceeded")},
	{NEXUS_HdmiOutputHdcpError_eRepeaterFifoNotReady, 	BDBG_STRING("Timeout waiting for Repeater")},
	{NEXUS_HdmiOutputHdcpError_eRepeaterDeviceCount0,	BDBG_STRING("HDCP Repeater Authentication Failure")},
	{NEXUS_HdmiOutputHdcpError_eRepeaterLinkFailure,		BDBG_STRING("HDCP Repeater Authentication Failure")},
	{NEXUS_HdmiOutputHdcpError_eLinkRiFailure,	BDBG_STRING("HDCP Ri Integrity Check Failure")},
	{NEXUS_HdmiOutputHdcpError_eLinkPjFailure,		BDBG_STRING("HDCP Pj Integrity Check Failure")},
	{NEXUS_HdmiOutputHdcpError_eFifoUnderflow,		BDBG_STRING("Video configuration issue - FIFO underflow")},
	{NEXUS_HdmiOutputHdcpError_eFifoOverflow,	BDBG_STRING("Video configuration issue - FIFO overflow")},
	{NEXUS_HdmiOutputHdcpError_eMultipleAnRequest,		BDBG_STRING("Multiple Authentication Request... ")},
	{NEXUS_HdmiOutputHdcpError_eMax, 	BDBG_STRING("Unknown HDCP Authentication Error")},
};

static const char * hdcpErrorToStr(
	NEXUS_HdmiOutputHdcpError eError
)
{
	uint8_t i=0;

	for (i=0; i<sizeof(hdcpErrorTextTable)/sizeof(hdcpErrorText); i++) {
		if (hdcpErrorTextTable[i].eError != eError)
			continue;

		return hdcpErrorTextTable[i].hdcpErrorText;
	}

	return BDBG_STRING("Invalid Authentication Error");
}


static void disconnect_hdmi_audio(NEXUS_HdmiOutputHandle hdmi)
{
    NEXUS_AudioDecoderHandle decoder;

    if ( hdmiCompressedAudio )
    {
        decoder = compressedDecoder;
    }
    else
    {
        decoder = pcmDecoder;
    }

    if ( audioStarted )
        NEXUS_AudioDecoder_Stop(decoder);

    NEXUS_AudioOutput_RemoveAllInputs(NEXUS_HdmiOutput_GetAudioConnector(hdmi));

    if ( audioStarted )
        NEXUS_AudioDecoder_Start(decoder, &audioProgram);
}

static void connect_hdmi_audio(NEXUS_HdmiOutputHandle hdmi)
{
    NEXUS_AudioDecoderHandle decoder;
    NEXUS_AudioInputHandle connector;

    if ( hdmiCompressedAudio )
    {
        decoder = compressedDecoder;
        connector = NEXUS_AudioDecoder_GetConnector(decoder, NEXUS_AudioDecoderConnectorType_eCompressed);
    }
    else
    {
        decoder = pcmDecoder;
        if (hdmiMultichPCM)
        {
            connector = NEXUS_AudioDecoder_GetConnector(decoder, NEXUS_AudioDecoderConnectorType_eMultichannel);
        }
        else
        {
            connector = NEXUS_AudioDecoder_GetConnector(decoder, NEXUS_AudioDecoderConnectorType_eStereo);
        }
    }

    if ( audioStarted )
        NEXUS_AudioDecoder_Stop(decoder);

    NEXUS_AudioOutput_AddInput(NEXUS_HdmiOutput_GetAudioConnector(hdmi), connector);

    if ( audioStarted )
        NEXUS_AudioDecoder_Start(decoder, &audioProgram);
}

static void hotplug_callback(void *pParam, int iParam)
{
    NEXUS_HdmiOutputStatus status;
    NEXUS_HdmiOutputHandle hdmi ;
    NEXUS_DisplayHandle display ;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_HdmiOutputSettings hdmiSettings;

    hotplugCallbackParameters *hotPlugCbParams ;
    NEXUS_Error errCode = NEXUS_SUCCESS;

    BSTD_UNUSED(iParam);
    hotPlugCbParams = (hotplugCallbackParameters *) pParam ;
        hdmi = hotPlugCbParams->hdmi ;
        display = hotPlugCbParams->display ;

    NEXUS_HdmiOutput_GetStatus(hdmi, &status);
    if ( !status.connected )
    {
        BDBG_WRN(("No RxDevice Connected")) ;
        return ;
    }

    /* the app can choose to switch to the preferred format, but it's not required. */
    if (!complianceTest)
    {
        NEXUS_Display_GetSettings(display, &displaySettings);
        if ( !status.videoFormatSupported[displaySettings.format] )
        {
            BDBG_ERR(("Current format not supported by attached monitor. Switching to preferred format %d",
                status.preferredVideoFormat)) ;
            displaySettings.format = status.preferredVideoFormat;
        }
        NEXUS_Display_SetSettings(display, &displaySettings);

        /* force HDMI updates after a hotplug */
        NEXUS_HdmiOutput_GetSettings(hdmi, &hdmiSettings) ;
        NEXUS_HdmiOutput_SetSettings(hdmi, &hdmiSettings) ;
    }

    if (status.rxPowered) {
        errCode = initializeHdmiOutputHdcpSettings() ;
        if (errCode != NEXUS_SUCCESS) {
            BDBG_ERR(("Error InializeHdmiOutputHdcpSettings"));
            BERR_TRACE(errCode);
            return;
        }
    }

    /* restart HDCP if it was previously enabled or running compliance test*/
    if (hdmiHdcpEnabled || complianceTest) {
        NEXUS_HdmiOutput_StartHdcpAuthentication(platformConfig.outputs.hdmi[0]);
    }
}


static void hdmiOutputHdcpStateChanged(void *pContext, int param)
{

    NEXUS_HdmiOutputHandle handle = pContext;
    NEXUS_HdmiOutputStatus hdmiStatus;
    NEXUS_HdmiOutputHdcpStatus hdcpStatus;
    BSTD_UNUSED(param) ;


    /* check if  HDCP state changed due to power down */
    NEXUS_HdmiOutput_GetStatus(handle, &hdmiStatus);
    if (!hdmiStatus.rxPowered)
    {
        BDBG_WRN(("HDMI Rx is powered down; HDCP authentication disabled")) ;
        goto done ;
    }


    NEXUS_HdmiOutput_GetHdcpStatus(handle, &hdcpStatus);
    BDBG_LOG(("%s: state %d -- error %d", BSTD_FUNCTION, hdcpStatus.hdcpState, hdcpStatus.hdcpError));

	switch (hdcpStatus.hdcpState)
	{
	case NEXUS_HdmiOutputHdcpState_eUnpowered:
		BDBG_ERR(("Attached Device is unpowered")) ;
		break;

	case NEXUS_HdmiOutputHdcpState_eUnauthenticated:
		/* Unauthenticated - no hdcp error */
		if (hdcpStatus.hdcpError == NEXUS_HdmiOutputHdcpError_eSuccess)
		{
			BDBG_LOG(("*** HDCP was disabled as requested (NEXUS_HdmiOutput_DisableHdcpAuthentication was called)***"));
			goto done;
		}

		/* Unauthenticated - with hdcp authentication error */
		else {
			BDBG_LOG(("*** HDCP Authentication failed - Error (%d): %s - ***",
				hdcpStatus.hdcpError, hdcpErrorToStr(hdcpStatus.hdcpError)));

		    /* always retry if running compliance test */
	        if (complianceTest) {
	            NEXUS_HdmiOutput_StartHdcpAuthentication(platformConfig.outputs.hdmi[0]);
	        }
		}
		break;


	case NEXUS_HdmiOutputHdcpState_eWaitForValidVideo:
	case NEXUS_HdmiOutputHdcpState_eInitializedAuthentication:
	case NEXUS_HdmiOutputHdcpState_eWaitForReceiverAuthentication:
	case NEXUS_HdmiOutputHdcpState_eReceiverR0Ready:
	case NEXUS_HdmiOutputHdcpState_eReceiverAuthenticated:
	case NEXUS_HdmiOutputHdcpState_eWaitForRepeaterReady:
	case NEXUS_HdmiOutputHdcpState_eCheckForRepeaterReady:
	case NEXUS_HdmiOutputHdcpState_eRepeaterReady:
		/* In process of authenticating with attached Rx/Repeater */
		/* Do nothing */
		BDBG_LOG(("*** Authenticating with attached Receiver/Repeater - current state: %d ***", hdcpStatus.hdcpState));
		break;


	case NEXUS_HdmiOutputHdcpState_eLinkAuthenticated:
    case NEXUS_HdmiOutputHdcpState_eEncryptionEnabled:
		/* HDCP successfully authenticated */
		BDBG_LOG(("*** HDCP Authentication Successful ***\n"));
		hdmiHdcpEnabled = true ;
		break;


	case NEXUS_HdmiOutputHdcpState_eRepeaterAuthenticationFailure:
	case NEXUS_HdmiOutputHdcpState_eRiLinkIntegrityFailure:
	case NEXUS_HdmiOutputHdcpState_ePjLinkIntegrityFailure:
	case NEXUS_HdmiOutputHdcpState_eR0LinkFailure:
		/* HDCP authentication fail - in particular, link integrity check fail */
		BDBG_LOG(("*** HDCP Authentication failed - Error: %s - ***", hdcpErrorToStr(hdcpStatus.hdcpError)));

	    /* always retry if running compliance test */
        if (complianceTest) {
            NEXUS_HdmiOutput_StartHdcpAuthentication(platformConfig.outputs.hdmi[0]);
        }
		break;

	default:
		BDBG_ERR(("*** Invalid HDCP authentication state ***"));
		break;
	}

done:

	return;
}


static NEXUS_Error initializeHdmiOutputHdcpSettings(void)
{
    NEXUS_HdmiOutputHdcpSettings hdmiOutputHdcpSettings;

    int rc = 0;
    int fileFd;
    uint8_t *buffer = NULL;
    NEXUS_Error errCode=NEXUS_SUCCESS;
    size_t fileSize=0;
    off_t seekPos=0;

    fileFd = open(HDCP1x_DEFAULT_BIN, O_RDONLY);
    if (fileFd < 0) {
        BDBG_LOG(("Loading 1.x test keys"));
        BKNI_Memcpy(hdcpTxAksv.data, testkey_hdcpTxAksv.data, NEXUS_HDMI_OUTPUT_HDCP_KSV_LENGTH);
        BKNI_Memcpy(&encryptedTxKeySet, &testkey_encryptedTxKeySet, NEXUS_HDMI_OUTPUT_HDCP_NUM_KEYS * sizeof(NEXUS_HdmiOutputHdcpKey));
    }
    else {
        char tmp[4];
        BDBG_LOG(("Loading 1.x production keys from %s", HDCP1x_DEFAULT_BIN));
        read(fileFd, hdcpTxAksv.data, NEXUS_HDMI_OUTPUT_HDCP_KSV_LENGTH);
        read(fileFd, tmp, 3);
        read(fileFd, &encryptedTxKeySet, NEXUS_HDMI_OUTPUT_HDCP_NUM_KEYS * sizeof(NEXUS_HdmiOutputHdcpKey));
        close(fileFd);
    }

    fileFd = open(HDCP2x_DEFAULT_BIN, O_RDONLY);
    if (fileFd < 0)
    {
        BDBG_ERR(("Unable to open bin file"));
        rc = 1;
        goto end;
    }

    seekPos = lseek(fileFd, 0, SEEK_END);
    if (seekPos < 0)
    {
        BDBG_ERR(("Unable to seek bin file size"));
        rc = 2;
        goto end;
    }
    fileSize = (size_t)seekPos;

    if (lseek(fileFd, 0, SEEK_SET) < 0)
    {
        BDBG_ERR(("Unable to get back to origin"));
        rc = 3;
        goto end;
    }

    buffer = BKNI_Malloc(fileSize);
    if (read(fileFd, (void *)buffer, fileSize) != (ssize_t)fileSize)
    {
        BDBG_ERR(("Unable to read all binfile"));
        rc = 6;
        goto end;
    }

    BDBG_LOG(("%s: buff=%p, size=%u", BSTD_FUNCTION, buffer, (unsigned)fileSize));

    NEXUS_HdmiOutput_GetHdcpSettings(platformConfig.outputs.hdmi[0], &hdmiOutputHdcpSettings);
    hdmiOutputHdcpSettings.hdcp_version = version_select;

    /* copy the encrypted key set and its Aksv here  */
    BKNI_Memcpy(hdmiOutputHdcpSettings.encryptedKeySet, encryptedTxKeySet, NEXUS_HDMI_OUTPUT_HDCP_NUM_KEYS * sizeof(NEXUS_HdmiOutputHdcpKey));
    BKNI_Memcpy(&hdmiOutputHdcpSettings.aksv, &hdcpTxAksv, NEXUS_HDMI_OUTPUT_HDCP_KSV_LENGTH);

#if 0
    /* install HDCP success  callback */
    hdmiOutputHdcpSettings.successCallback.callback = hdmiOutputHdcpStateChanged ;
    hdmiOutputHdcpSettings.successCallback.context = platformConfig.outputs.hdmi[0];

    /* install HDCP failure callback */
    hdmiOutputHdcpSettings.failureCallback.callback = hdmiOutputHdcpStateChanged ;
    hdmiOutputHdcpSettings.failureCallback.context = platformConfig.outputs.hdmi[0];
#else
	/* install hdcp callback */
	hdmiOutputHdcpSettings.stateChangedCallback.callback = hdmiOutputHdcpStateChanged;
	hdmiOutputHdcpSettings.stateChangedCallback.context = platformConfig.outputs.hdmi[0];
#endif

    NEXUS_HdmiOutput_SetHdcpSettings(platformConfig.outputs.hdmi[0], &hdmiOutputHdcpSettings);

    errCode = NEXUS_HdmiOutput_SetHdcp2xBinKeys(platformConfig.outputs.hdmi[0], buffer, (uint32_t)fileSize);
    if (errCode != NEXUS_SUCCESS)
    {
        BDBG_ERR(("Error setting Hdcp2x encrypted keys. HDCP2.x will not work."));
    }

    /* install list of revoked KSVs from SRMs (System Renewability Message) if available */
    NEXUS_HdmiOutput_SetHdcpRevokedKsvs(platformConfig.outputs.hdmi[0],
        RevokedKsvs, NumRevokedKsvs) ;
end:

    if (fileFd)    {
        close(fileFd);
    }

    if (buffer) {
        BKNI_Free(buffer);
    }

    if (rc)
    {
        BDBG_ERR(("%s: error #%d, fileSize=%u, seekPos=%d", BSTD_FUNCTION,  rc, (unsigned)fileSize, (unsigned)seekPos));
        BDBG_ASSERT(false);
    }

    return errCode;
 }


void toggle_menu(void)
{
    bool done = false;

    while (!done)
    {
        int tmp;

        /* Display Menu */
        printf("Toggle Menu\n");
        printf(" 1) Toggle PCM/Compressed audio (currently %s)\n",
            hdmiCompressedAudio ? "Compressed" : "PCM");
        printf(" 2) Toggle AvMute (currently %s)\n",
        hdmiAvMute ? "Set_AvMute" : "Clear_AvMute");
        printf(" 3) Toggle 2CH_PCM/5.1CH_PCM (currently %s)\n", hdmiMultichPCM?"5.1ch PCM":" Stereo PCM");
        printf(" 0) Exit\n");
        printf("Enter Selection: ");
        scanf("%d", &tmp);
        switch ( tmp )
        {
        case 0:
         done = true ;
            break;

        case 1:
            disconnect_hdmi_audio(platformConfig.outputs.hdmi[0]);
            hdmiCompressedAudio = !hdmiCompressedAudio;
            connect_hdmi_audio(platformConfig.outputs.hdmi[0]);
            break;

        case 2:
          hdmiAvMute = !hdmiAvMute  ;
            NEXUS_HdmiOutput_SetAVMute(platformConfig.outputs.hdmi[0], hdmiAvMute) ;
            break;

        case 3:
            if (!hdmiCompressedAudio)
            {
                disconnect_hdmi_audio(platformConfig.outputs.hdmi[0]);
                hdmiMultichPCM = !hdmiMultichPCM;
                connect_hdmi_audio(platformConfig.outputs.hdmi[0]);
            }
            else
            {
                hdmiMultichPCM = !hdmiMultichPCM;
            }
            break;
        }
    }
}



int main(int argc, char **argv)
{
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_DisplayHandle display;
    NEXUS_VideoWindowHandle window;
    NEXUS_HdmiOutputStatus hdmiStatus;
    NEXUS_HdmiOutputSettings hdmiSettings;
    bool done = false;
    NEXUS_Error rc;
    unsigned i, menuIndex ;

    NEXUS_PlatformSettings platformSettings;
    NEXUS_StcChannelHandle stcChannel;
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_PidChannelHandle videoPidChannel, audioPidChannel;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_VideoDecoderHandle videoDecoder;
    NEXUS_FilePlayHandle file;
    NEXUS_PlaypumpHandle playpump;
    NEXUS_PlaybackHandle playback;
    NEXUS_PlaybackSettings playbackSettings;
    NEXUS_PlaybackPidChannelSettings playbackPidSettings;
    const char *fname = FILE_NAME;
    NEXUS_HdmiOutputStatus pStatus;
    hotplugCallbackParameters hotPlugCbParams ;


    typedef struct {
        int menuEntry;
        NEXUS_VideoFormat format;
        char* name;
    } formatTable;

    static formatTable testFormats[] = {

		{ 0, NEXUS_VideoFormat_e3840x2160p24hz,	"3840x2160p24" },
		{ 0, NEXUS_VideoFormat_e3840x2160p25hz,	"3840x2160p25" },
		{ 0, NEXUS_VideoFormat_e3840x2160p30hz,	"3840x2160p30" },
		{ 0, NEXUS_VideoFormat_e3840x2160p50hz,	"3840x2160p50" },
		{ 0, NEXUS_VideoFormat_e3840x2160p60hz, "3840x2160p60" },
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

        /* END of ARRAY */
        { 0, 0, "" }
    };

    int curarg = 1;
    while (curarg < argc) {
        if (!strcmp(argv[curarg], "-compliance")) {
            complianceTest = true  ;
        }
        else if (!strcmp(argv[curarg], "-480p")) {
            complianceTestFormat = NEXUS_VideoFormat_e480p;
        }
        else if (!strcmp(argv[curarg], "-720p")) {
            complianceTestFormat = NEXUS_VideoFormat_e720p;
        }
        else if (!strcmp(argv[curarg], "-1080p")) {
            complianceTestFormat = NEXUS_VideoFormat_e1080p60hz;
        }

       curarg++;
    }

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    rc = NEXUS_Platform_Init(&platformSettings);
    if (rc) return -1;
    NEXUS_Platform_GetConfiguration(&platformConfig);

    playpump = NEXUS_Playpump_Open(NEXUS_ANY_ID, NULL);
    BDBG_ASSERT(playpump);
    playback = NEXUS_Playback_Create();
    BDBG_ASSERT(playback);

    file = NEXUS_FilePlay_OpenPosix(fname, NULL);
    if (!file) {
        fprintf(stderr, "can't open file:%s\n", fname);
        return -1;
    }

    NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;
    stcSettings.mode = NEXUS_StcChannelMode_eAuto;
    stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);

    NEXUS_Playback_GetSettings(playback, &playbackSettings);
    playbackSettings.playpump = playpump;
    /* set a stream format, it could be any audio video transport type or file format, i.e NEXUS_TransportType_eMp4, NEXUS_TransportType_eAvi ... */
    playbackSettings.playpumpSettings.transportType = TRANSPORT_TYPE;
    playbackSettings.stcChannel = stcChannel;
    NEXUS_Playback_SetSettings(playback, &playbackSettings);

    /* Bring up audio decoders and outputs */
    pcmDecoder = NEXUS_AudioDecoder_Open(0, NULL);
    NEXUS_AudioOutput_AddInput(
        NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]),
        NEXUS_AudioDecoder_GetConnector(pcmDecoder, NEXUS_AudioDecoderConnectorType_eStereo));


    /* Bring up video display and outputs */
    NEXUS_Display_GetDefaultSettings(&displaySettings);
/*        displaySettings.format = NEXUS_VideoFormat_e3840x2160p30hz;*/
    displaySettings.format = NEXUS_VideoFormat_e1080p60hz;
    display = NEXUS_Display_Open(0, &displaySettings);
    window = NEXUS_VideoWindow_Open(display, 0);

    NEXUS_Display_AddOutput(display, NEXUS_HdmiOutput_GetVideoConnector(platformConfig.outputs.hdmi[0]));
    rc = NEXUS_HdmiOutput_GetStatus(platformConfig.outputs.hdmi[0], &hdmiStatus);
    if ( !rc && hdmiStatus.connected )
    {
        /* If current display format is not supported by monitor, switch to monitor's preferred format.
           If other connected outputs do not support the preferred format, a harmless error will occur. */
        NEXUS_Display_GetSettings(display, &displaySettings);
        if ( !hdmiStatus.videoFormatSupported[displaySettings.format] ) {
            displaySettings.format = hdmiStatus.preferredVideoFormat;
            NEXUS_Display_SetSettings(display, &displaySettings);
        }
    }

    if (complianceTest) {
        NEXUS_Display_GetSettings(display, &displaySettings);
            displaySettings.format = complianceTestFormat;
            NEXUS_HdmiOutput_GetSettings(platformConfig.outputs.hdmi[0], &hdmiSettings);
            hdmiSettings.colorSpace = NEXUS_ColorSpace_eRgb;
            NEXUS_HdmiOutput_SetSettings(platformConfig.outputs.hdmi[0], &hdmiSettings);
        NEXUS_Display_SetSettings(display, &displaySettings);
    }

    /* bring up decoder and connect to display */
    videoDecoder = NEXUS_VideoDecoder_Open(0, NULL); /* take default capabilities */
    NEXUS_VideoWindow_AddInput(window, NEXUS_VideoDecoder_GetConnector(videoDecoder));

    /* Open the audio and video pid channels */
    NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
    playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
    playbackPidSettings.pidTypeSettings.video.codec = VIDEO_CODEC; /* must be told codec for correct handling */
    playbackPidSettings.pidTypeSettings.video.index = true;
    playbackPidSettings.pidTypeSettings.video.decoder = videoDecoder;
    videoPidChannel = NEXUS_Playback_OpenPidChannel(playback, VIDEO_PID, &playbackPidSettings);

    NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
    playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eAudio;
    playbackPidSettings.pidTypeSettings.audio.primary = pcmDecoder;
    audioPidChannel = NEXUS_Playback_OpenPidChannel(playback, AUDIO_PID, &playbackPidSettings);


    /* Set up decoder Start structures now. We need to know the audio codec to properly set up the audio outputs. */
    NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
    videoProgram.codec = VIDEO_CODEC;
    videoProgram.pidChannel = videoPidChannel;
    videoProgram.stcChannel = stcChannel;
    videoProgram.frameRate = NEXUS_VideoFrameRate_e30;


    NEXUS_AudioDecoder_GetDefaultStartSettings(&audioProgram);
    audioProgram.codec = AUDIO_CODEC;
    audioProgram.pidChannel = audioPidChannel;
    audioProgram.stcChannel = stcChannel;


    /* Start decoders */
    NEXUS_VideoDecoder_Start(videoDecoder, &videoProgram);
    NEXUS_AudioDecoder_Start(pcmDecoder, &audioProgram);

    /* Start playback */
    NEXUS_Playback_Start(playback, file, NULL);


    /* Install hotplug callback -- video only for now */
    NEXUS_HdmiOutput_GetSettings(platformConfig.outputs.hdmi[0], &hdmiSettings);
        hdmiSettings.hotplugCallback.callback = hotplug_callback;

        hotPlugCbParams.hdmi = platformConfig.outputs.hdmi[0] ;
        hotPlugCbParams.display = display ;
        hdmiSettings.hotplugCallback.context = &hotPlugCbParams ;

    NEXUS_HdmiOutput_SetSettings(platformConfig.outputs.hdmi[0], &hdmiSettings);

    /* initialize HDCP settings, keys, etc. */
    NEXUS_HdmiOutput_GetStatus(platformConfig.outputs.hdmi[0], &pStatus) ;
    if (!pStatus.connected || !pStatus.rxPowered) {
        BDBG_WRN(("No Receiver Device Available or Receiver is powered off - Skip InitializeHdcpSettings")) ;
    }
    else {
        rc = initializeHdmiOutputHdcpSettings() ;
    }

    /* Force a hotplug to switch to preferred format */
    if (!complianceTest) {
       hotplug_callback(&hotPlugCbParams, 0);
    }

    while (!done)
    {
        int tmp;
        NEXUS_HdmiOutputHdcpSettings hdmiOutputHdcpSettings;

        /* Find Current Format */
        NEXUS_Display_GetSettings(display, &displaySettings);
        for ( i = 0; testFormats[i].format ; i++)
        {
            if ( displaySettings.format == testFormats[i].format )
                break ;
        }


        /* Display Menu */
        printf("Main Menu\n");
        printf(" 1) Change Video Format (currently %s) \n", testFormats[i].name );
        printf(" 2) Toggle Menu\n") ;
        printf(" 3) Print decode status\n");
        printf(" 4) Enable HDCP\n");
        printf(" 5) Disable HDCP\n");
        printf(" 6) Select HDCP version: auto\n");
        printf(" 7) Select HDCP version: 2_2 only\n");
        printf(" 8) Select HDCP version: 1_x only\n");
        printf("10) Permit all hdcp streams to downstream devices\n");
        printf("11) Block all hdcp streams to downstream 1.x devices\n");
        printf(" 0) Exit\n");
        printf("Enter Selection: ");
        scanf("%d", &tmp);
        switch ( tmp )
        {
        case 0:
            done = true;
            break;
        case 1:

            /* get/build a list of formats supported by attached Receiver */
            NEXUS_HdmiOutput_GetStatus(platformConfig.outputs.hdmi[0], &hdmiStatus);

            /* Display available formats */
            menuIndex = 0;
            printf("\n\n 0) Return to Main Menu\n");

            i = 0 ;
            while ( testFormats[i].format )
            {
                /* if format is supported; add to menu */
                if ( hdmiStatus.videoFormatSupported[testFormats[i].format] )
                {
                    testFormats[i].menuEntry = ++ menuIndex ;
                    printf("%2d) %s\n", menuIndex, testFormats[i].name );
                }
                i++;
            }

            /* Read user input for desired format */
            printf("\nEnter new format-> ");
            scanf("%d", &tmp);

            if( !tmp )  /* 0 - Exit */
                break;  /* Exit to Main Menu*/

            if ( (uint8_t) tmp > menuIndex) /* selection not listed */
            {
                printf("\n'%d' is an invalid choice\n\n", tmp);
                break;
            }

            for ( i = 0; ; i++)
            {
                if ( tmp != testFormats[i].menuEntry ) /* search for match */
                    continue ;

                /* Update display to selected format */
                NEXUS_Display_GetSettings(display, &displaySettings);
                    displaySettings.format = testFormats[i].format;
                rc = NEXUS_Display_SetSettings(display, &displaySettings);
                if (rc)
                {
                    printf("ERROR changing format\n") ;
                }

                break;
            }

            break;
        case 2:
            toggle_menu() ;
            break;

        case 3:
            {
                NEXUS_VideoDecoderStatus status;
                NEXUS_AudioDecoderStatus audioStatus;
                uint32_t stc;

                NEXUS_VideoDecoder_GetStatus(videoDecoder, &status);
                NEXUS_StcChannel_GetStc(videoProgram.stcChannel, &stc);
                printf("decode %.4dx%.4d, pts %#x, stc %#x (diff %d) fifo=%d%%\n",
                    status.source.width, status.source.height, status.pts, stc, status.ptsStcDifference, status.fifoSize?(status.fifoDepth*100)/status.fifoSize:0);
                NEXUS_AudioDecoder_GetStatus(pcmDecoder, &audioStatus);
                printf("audio0            pts %#x, stc %#x (diff %d) fifo=%d%%\n",
                    audioStatus.pts, stc, audioStatus.ptsStcDifference, audioStatus.fifoSize?(audioStatus.fifoDepth*100)/audioStatus.fifoSize:0);
                NEXUS_AudioDecoder_GetStatus(compressedDecoder, &audioStatus);
                if ( audioStatus.started )
                {
                    printf("audio1            pts %#x, stc %#x (diff %d) fifo=%d%%\n",
                        audioStatus.pts, stc, audioStatus.ptsStcDifference, audioStatus.fifoSize?(audioStatus.fifoDepth*100)/audioStatus.fifoSize:0);
                }
            }
            break;
        case 4:
            NEXUS_HdmiOutput_GetStatus(platformConfig.outputs.hdmi[0], &pStatus);
            if (!pStatus.connected || !pStatus.rxPowered) {
                BDBG_WRN(("No Receiver Device Available or Receiver powered off"));
                break ;
            }

            NEXUS_HdmiOutput_StartHdcpAuthentication(platformConfig.outputs.hdmi[0]);
            break;
        case 5:
            NEXUS_HdmiOutput_DisableHdcpAuthentication(platformConfig.outputs.hdmi[0]);
            hdmiHdcpEnabled = false ;
            break;

        case 6:
            version_select = NEXUS_HdmiOutputHdcpVersion_eAuto;
            initializeHdmiOutputHdcpSettings();
            break;
        case 7:
            version_select = NEXUS_HdmiOutputHdcpVersion_e2_2;
            initializeHdmiOutputHdcpSettings();
            break;
        case 8:
            version_select = NEXUS_HdmiOutputHdcpVersion_e1_x;
            initializeHdmiOutputHdcpSettings();
            break;

        case 10:
            NEXUS_HdmiOutput_GetHdcpSettings(platformConfig.outputs.hdmi[0], &hdmiOutputHdcpSettings);
            hdmiOutputHdcpSettings.hdcp2xContentStreamControl = NEXUS_Hdcp2xContentStream_eType0;
            NEXUS_HdmiOutput_SetHdcpSettings(platformConfig.outputs.hdmi[0], &hdmiOutputHdcpSettings);
            break;
        case 11:
            NEXUS_HdmiOutput_GetHdcpSettings(platformConfig.outputs.hdmi[0], &hdmiOutputHdcpSettings);
            hdmiOutputHdcpSettings.hdcp2xContentStreamControl = NEXUS_Hdcp2xContentStream_eType1;
            NEXUS_HdmiOutput_SetHdcpSettings(platformConfig.outputs.hdmi[0], &hdmiOutputHdcpSettings);
            break;
        default:
            break;
        }
    }


    NEXUS_VideoDecoder_Stop(videoDecoder);
    NEXUS_Playback_ClosePidChannel(playback, videoPidChannel);

    if (pcmDecoder) {
        NEXUS_AudioDecoder_Stop(pcmDecoder);
        NEXUS_AudioDecoder_Close(pcmDecoder);
    }

    if (compressedDecoder) {
        NEXUS_AudioDecoder_Stop(compressedDecoder);
        NEXUS_AudioDecoder_Close(compressedDecoder);
    }
    NEXUS_Playback_ClosePidChannel(playback, audioPidChannel);

    NEXUS_Playback_Stop(playback);
    NEXUS_FilePlay_Close(file);
    NEXUS_Playback_Destroy(playback);
    NEXUS_Playpump_Close(playpump);

    NEXUS_VideoDecoder_Close(videoDecoder);

    NEXUS_StcChannel_Close(stcChannel);
    NEXUS_VideoWindow_Close(window);

    NEXUS_StopCallbacks(platformConfig.outputs.hdmi[0]);
    NEXUS_Display_Close(display);


    NEXUS_Platform_Uninit();

    return 0;
}

#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bstd.h"
int main(int argc, char **argv)
{
    BSTD_UNUSED(argc) ;
    fprintf(stderr, "%s not supported on this platform\n", argv[0]);
    return 0;
}
#endif
