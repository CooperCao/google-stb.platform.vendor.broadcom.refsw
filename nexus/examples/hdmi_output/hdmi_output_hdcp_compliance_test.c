/******************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
#if NEXUS_NUM_HDMI_OUTPUTS
#include "nexus_display.h"
#include "nexus_hdmi_output.h"
#include "nexus_hdmi_output_hdcp.h"

#if NEXUS_HAS_PICTURE_DECODER
#include "nexus_core_utils.h"
#include "nexus_graphics2d.h"
#include "nexus_picture_decoder.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include "bstd.h"
#include "nexus_core_utils.h"

#include "bkni.h"

BDBG_MODULE(hdmi_output_hdcp_compliance_test) ;

/* hdmiHdcp is always enabled for HDCP Compliance Testing */
static bool hdmiOutputHdcpEnabled = true ;

static NEXUS_PlatformConfiguration platformConfig;

/* HDCP Compliance Test format must be either 480p or 576p  */
/* update the macro HDCP_COMPLIANCE_TEST_FORMAT to select */
#if 0
NEXUS_VideoFormat_e480p			/* NTSC Progressive (27Mhz) */
NEXUS_VideoFormat_e576p			/* HD PAL Progressive 50hz for Australia */
#endif

#define HDCP_COMPLIANCE_TEST_FORMAT NEXUS_VideoFormat_e480p ;

#define USE_APPLICATION_BUFFER 0

static const char HDCP1x_DEFAULT_BIN[] = "./hdcp1xKeys.bin";
static void initializeHdcpSettings(void)  ;


/**************************************/
/* HDCP Specification Test Key Set    */
/*                                    */
/* NOTE: the default declared Test    */
/* KeySet below is from the HDCP Spec */
/* and it *IS NOT* compatible with    */
/* production devices                 */
/**************************************/

static NEXUS_HdmiOutputHdcpKsv testkey_hdcpTxAksv =
{	{0x14, 0xF7, 0x61, 0x03, 0xB7} };


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


static void hdmiOutputHotplugStateChanged(void *pParam, int iParam)
{
    NEXUS_HdmiOutputStatus status;
    NEXUS_HdmiOutputHandle hdmi ;
    NEXUS_DisplayHandle display ;
    NEXUS_DisplaySettings displaySettings;
    hotplugCallbackParameters *hotPlugCbParams ;

    hotPlugCbParams = (hotplugCallbackParameters *) pParam ;
        hdmi = hotPlugCbParams->hdmi ;
        display = hotPlugCbParams->display ;

    NEXUS_HdmiOutput_GetStatus(hdmi, &status);

    /* the app can choose to switch to the preferred format, but it's not required. */
    if ( !status.connected )
    {
        BDBG_WRN(("No RxDevice Connected")) ;
        return ;
    }

    NEXUS_Display_GetSettings(display, &displaySettings);

        displaySettings.format = HDCP_COMPLIANCE_TEST_FORMAT ;

        /* make sure selected Compliance Test is supported; */
        /* Compliance TE should/must support these */
        if ( !status.videoFormatSupported[displaySettings.format] )
        {
            BDBG_ERR(("Current format not supported by attached monitor. Switching to preferred format %d",
                status.preferredVideoFormat)) ;
            displaySettings.format = status.preferredVideoFormat;
        }

    NEXUS_Display_SetSettings(display, &displaySettings);

    /* restart HDCP if it was previously enabled */
    if (hdmiOutputHdcpEnabled)
    {
        NEXUS_HdmiOutput_StartHdcpAuthentication(platformConfig.outputs.hdmi[0]);
    }
}


static void hdmiOutputHdcpStateChanged(void *pContext, int param)
{
    bool success = false ;
    NEXUS_HdmiOutputHandle handle = pContext;
    NEXUS_HdmiOutputHdcpStatus hdmiOutputHdcpStatus;

    BSTD_UNUSED(param) ;

    NEXUS_HdmiOutput_GetHdcpStatus(handle, &hdmiOutputHdcpStatus);

    switch (hdmiOutputHdcpStatus.hdcpError)
    {
    case NEXUS_HdmiOutputHdcpError_eSuccess :
        BDBG_WRN(("HDCP Authentication Successful"));
        success = true ;
        break ;

    case NEXUS_HdmiOutputHdcpError_eRxBksvError :
        BDBG_ERR(("HDCP Rx BKsv Error")) ;
        break ;

    case NEXUS_HdmiOutputHdcpError_eRxBksvRevoked :
        BDBG_ERR(("HDCP Rx BKsv/Keyset Revoked")) ;
        break ;

    case NEXUS_HdmiOutputHdcpError_eRxBksvI2cReadError :
    case NEXUS_HdmiOutputHdcpError_eTxAksvI2cWriteError :
        BDBG_ERR(("HDCP I2C Read Error")) ;
        break ;

    case NEXUS_HdmiOutputHdcpError_eTxAksvError :
        BDBG_ERR(("HDCP Tx Aksv Error")) ;
        break ;

    case NEXUS_HdmiOutputHdcpError_eReceiverAuthenticationError :
        BDBG_ERR(("HDCP Receiver Authentication Failure")) ;
        break ;

    case NEXUS_HdmiOutputHdcpError_eRepeaterAuthenticationError :
    case NEXUS_HdmiOutputHdcpError_eRepeaterLinkFailure :    /* Repeater Error; unused */
        BDBG_ERR(("HDCP Repeater Authentication Failure")) ;
        break ;

    case NEXUS_HdmiOutputHdcpError_eRxDevicesExceeded :
        BDBG_ERR(("HDCP Repeater MAX Downstram Devices Exceeded")) ;
        break ;

    case NEXUS_HdmiOutputHdcpError_eRepeaterDepthExceeded :
        BDBG_ERR(("HDCP Repeater MAX Downstram Levels Exceeded")) ;
        break ;

    case NEXUS_HdmiOutputHdcpError_eRepeaterFifoNotReady :
        BDBG_ERR(("Timeout waiting for Repeater")) ;
        break ;

    case NEXUS_HdmiOutputHdcpError_eRepeaterDeviceCount0 : /* unused */
        break ;

    case NEXUS_HdmiOutputHdcpError_eLinkRiFailure :
        BDBG_ERR(("HDCP Ri Integrity Check Failure")) ;
        break ;

    case NEXUS_HdmiOutputHdcpError_eLinkPjFailure :
        BDBG_ERR(("HDCP Pj Integrity Check Failure")) ;
        break ;

    case NEXUS_HdmiOutputHdcpError_eFifoUnderflow :
    case NEXUS_HdmiOutputHdcpError_eFifoOverflow :
        BDBG_ERR(("Video configuration issue")) ;
        break ;

    case NEXUS_HdmiOutputHdcpError_eMultipleAnRequest : /* Should not reach here; but flag if occurs */
        BDBG_WRN(("Multiple Authentication Request... ")) ;

    default :
        BDBG_WRN(("Unknown HDCP Authentication Error %d", hdmiOutputHdcpStatus.hdcpError)) ;
    }

    if (!success )
    {
        BDBG_WRN(("HDCP Authentication Failed.  Current State: %d ; Error: %d",
            hdmiOutputHdcpStatus.hdcpState,
            hdmiOutputHdcpStatus.hdcpError)) ;

       /* always retry */
       NEXUS_HdmiOutput_StartHdcpAuthentication(platformConfig.outputs.hdmi[0]) ;
    }

}


static void initializeHdcpSettings(void)
{
    NEXUS_HdmiOutputHdcpSettings *pHdcpSettings;
    int fileFd;
    size_t fileSize;
    off_t seekPos;

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


    pHdcpSettings = BKNI_Malloc(sizeof(*pHdcpSettings));
    if ( !pHdcpSettings )
    {
        BDBG_ERR(("Out of memory")) ;
        return ;
    }

    NEXUS_HdmiOutput_GetHdcpSettings(platformConfig.outputs.hdmi[0], pHdcpSettings);

        /* copy the encrypted key set and its Aksv here  */
        BKNI_Memcpy(pHdcpSettings->encryptedKeySet, encryptedTxKeySet,
            NEXUS_HDMI_HDCP_NUM_KEYS * sizeof(NEXUS_HdmiOutputHdcpKey)) ;
        BKNI_Memcpy(&pHdcpSettings->aksv, &hdcpTxAksv, NEXUS_HDMI_HDCP_KSV_LENGTH) ;

        /* install HDCP success  callback */
        pHdcpSettings->successCallback.callback = hdmiOutputHdcpStateChanged ;
        pHdcpSettings->successCallback.context = platformConfig.outputs.hdmi[0];
        pHdcpSettings->successCallback.param = true;

        /* install HDCP failure callback */
        pHdcpSettings->failureCallback.callback = hdmiOutputHdcpStateChanged ;
        pHdcpSettings->failureCallback.context = platformConfig.outputs.hdmi[0];
        pHdcpSettings->failureCallback.param = false;

    NEXUS_HdmiOutput_SetHdcpSettings(platformConfig.outputs.hdmi[0], pHdcpSettings);

    /* install list of revoked KSVs from SRMs (System Renewability Message) if available */
    NEXUS_HdmiOutput_SetHdcpRevokedKsvs(platformConfig.outputs.hdmi[0],
        RevokedKsvs, NumRevokedKsvs) ;

    BKNI_Free(pHdcpSettings);
}


void hdmi_output_status(void )
{
    NEXUS_HdmiOutputStatus status ;
    BDBG_Level saveLevel ;

    BDBG_GetModuleLevel("hdmi_output_hdcp_compliance_test", &saveLevel) ;

    BDBG_SetModuleLevel("hdmi_output_hdcp_compliance_test", BDBG_eMsg) ;

    NEXUS_HdmiOutput_GetStatus(platformConfig.outputs.hdmi[0], &status) ;
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

    /* restore debug level */
    BDBG_SetModuleLevel("hdmi_output_hdcp_compliance_test", saveLevel) ;

}


int main(void)
{
#if NEXUS_HAS_PICTURE_DECODER
	NEXUS_SurfaceHandle framebuffer;
	NEXUS_SurfaceCreateSettings createSettings;
	NEXUS_GraphicsSettings graphicsSettings;
	NEXUS_Graphics2DHandle gfx;
	NEXUS_PictureDecoderStartSettings pictureSettings;
	NEXUS_PictureDecoderStatus pictureStatus;
	NEXUS_Graphics2DBlitSettings blitSettings;
	NEXUS_PictureDecoderOpenSettings decoderSettings;
	NEXUS_VideoFormatInfo videoFormatInfo;
	int pictureBufferSize;
	int multiScanBufferSize;
	void *pPictureBuffer;
	void *pMultiScanBuffer;
#if USE_APPLICATION_BUFFER
	NEXUS_MemoryAllocationSettings allocationSettings;
#endif
	void *buffer;
	size_t size,file_size;
	int rc;
	FILE *fin;
	const char *pictureFilename = "videos/broadcom.jpg";
	NEXUS_PictureDecoderHandle pictureDecoder = NULL;
	NEXUS_SurfaceHandle picture = NULL;
	unsigned cnt;
	size_t bytesRemain =0;
#endif

	NEXUS_DisplayHandle display;
	NEXUS_PlatformSettings platformSettings;
	NEXUS_DisplaySettings displaySettings;
	NEXUS_HdmiOutputSettings hdmiSettings;
	bool done = false;
	BERR_Code brc ;

    hotplugCallbackParameters hotPlugCbParams ;

#if 1
    printf("********************************************************************************************\n");
    printf("* This example app should no longer be used. It will be deprecated in the next URSR release\n");
    printf("* Please use hdmi_output_hdcp.c app instead. \n");
    printf("* Use runtime option '-compliance' when running HDCP compliance test\n");
    printf("********************************************************************************************\n");
    return 0;
#endif


	/* Bring up all modules for a platform in a default configuration for this platform */
	NEXUS_Platform_GetDefaultSettings(&platformSettings);
	platformSettings.openFrontend = false;
	NEXUS_Platform_Init(&platformSettings);
	NEXUS_Platform_GetConfiguration(&platformConfig);

	NEXUS_Display_GetDefaultSettings(&displaySettings);
	displaySettings.displayType = NEXUS_DisplayType_eAuto;
	displaySettings.format = HDCP_COMPLIANCE_TEST_FORMAT;
	display = NEXUS_Display_Open(0, &displaySettings);

	NEXUS_Display_AddOutput(display, NEXUS_HdmiOutput_GetVideoConnector(platformConfig.outputs.hdmi[0]));

	/* Install hotplug callback */
	NEXUS_HdmiOutput_GetSettings(platformConfig.outputs.hdmi[0], &hdmiSettings);
            hdmiSettings.hotplugCallback.callback = hdmiOutputHotplugStateChanged;

            hotPlugCbParams.hdmi = platformConfig.outputs.hdmi[0] ;
            hotPlugCbParams.display = display ;
            hdmiSettings.hotplugCallback.context = &hotPlugCbParams ;

	NEXUS_HdmiOutput_SetSettings(platformConfig.outputs.hdmi[0], &hdmiSettings);

	/* turn on BHDM_HDCP messages so message updates are seen */
	brc = BDBG_SetModuleLevel("BHDM_HDCP", BDBG_eMsg) ;


#if NEXUS_HAS_PICTURE_DECODER
	/* allocate framebuffer */
	NEXUS_VideoFormat_GetInfo(displaySettings.format, &videoFormatInfo);
	NEXUS_Surface_GetDefaultCreateSettings(&createSettings);
	createSettings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
	createSettings.width = videoFormatInfo.width;
	createSettings.height = videoFormatInfo.height;
	createSettings.heap = NEXUS_Platform_GetFramebufferHeap(0);
	framebuffer = NEXUS_Surface_Create(&createSettings);

	/* use graphics to fit image into the display framebuffer */
	gfx = NEXUS_Graphics2D_Open(0, NULL);

	BDBG_WRN(("decoding %s...", pictureFilename));
	fin = fopen(pictureFilename,"rb");
	if(!fin) {
		BDBG_ERR(("****************************************************"));
		BDBG_ERR(("** Error opening file %s **", pictureFilename));
		BDBG_ERR(("** Make sure file %s exist **", pictureFilename));
		BDBG_ERR(("****************************************************"));
		goto error;
	}

	multiScanBufferSize = 16*1024*1024;
	pictureBufferSize = 128*1024;
#if USE_APPLICATION_BUFFER
	printf(" Using application provided buffers for PictureDecoder...\n");
	/* Get heap from system */
	/* Allocate memory from heap, this memory will be used by Picture Decoder */
	NEXUS_Memory_GetDefaultAllocationSettings(&allocationSettings);
	/* Use the default heap to allocate memory from */
	rc = NEXUS_Memory_Allocate(multiScanBufferSize, &allocationSettings, &pMultiScanBuffer);
	BDBG_ASSERT(!rc);
	/* Reuse alloctionSettings from available */
	rc = NEXUS_Memory_Allocate(pictureBufferSize, &allocationSettings, &pPictureBuffer);
	BDBG_ASSERT(!rc);
#else
	/* Let PictureDecoder allocation memory */
	printf(" Using PictureDecoder provided buffers...\n");
	pMultiScanBuffer = NULL;
	pPictureBuffer = NULL;
#endif

	NEXUS_PictureDecoder_GetDefaultOpenSettings(&decoderSettings);
	decoderSettings.multiScanBufferSize = multiScanBufferSize;
	decoderSettings.bufferSize = pictureBufferSize;
	decoderSettings.multiScanBufferPtr = pMultiScanBuffer;
	decoderSettings.bufferPtr = pPictureBuffer;
	pictureDecoder = NEXUS_PictureDecoder_Open(0, &decoderSettings);

	/* Determine file size of image to display */
	fseek(fin, 0, SEEK_END);
	file_size = ftell(fin);
	fseek(fin, 0, SEEK_SET);

	NEXUS_PictureDecoder_GetDefaultStartSettings(&pictureSettings);
	if (strcasestr(pictureFilename, ".mpg") || strcasestr(pictureFilename, ".es")) {
		pictureSettings.format = NEXUS_PictureFormat_eMpeg;
	}
	else if (strcasestr(pictureFilename, ".png")) {
		pictureSettings.format = NEXUS_PictureFormat_ePng;
	}
	else if (strcasestr(pictureFilename, ".gif")) {
		pictureSettings.format = NEXUS_PictureFormat_eGif;
	}
	else if (strcasestr(pictureFilename, ".bmp")) {
		BDBG_ERR(("BMP is not supported"));
		goto error;
	}
	else {
		pictureSettings.format = NEXUS_PictureFormat_eJpeg;
	}
	pictureSettings.imageSize = file_size;

	NEXUS_PictureDecoder_GetBuffer(pictureDecoder, &buffer, &size); /* get location and size of the decoder's buffer */
	rc = fread(buffer, 1, size, fin);								/* read file into the decoder's buffer */
	if(rc<0) {
		perror(pictureFilename);
		goto error;
	}
	else if(rc==(int)size) {
		if( (unsigned)file_size > size && pictureSettings.format == NEXUS_PictureFormat_eMpeg ) {
			fprintf(stderr, "Picture file size %u is larger then buffer size %u, not supported for MPEG still frames\n",
				(unsigned)file_size, (unsigned)size);
			goto error;
		}
	}
	NEXUS_PictureDecoder_ReadComplete(pictureDecoder, 0, rc); /* commit data to the decoder */
	bytesRemain = pictureSettings.imageSize - size ;

	NEXUS_PictureDecoder_Start(pictureDecoder, &pictureSettings);

	cnt = 0;
	do {
		NEXUS_PictureDecoder_GetStatus(pictureDecoder, &pictureStatus);
		if(pictureStatus.state==NEXUS_PictureDecoderState_eError) {
			fprintf(stderr, "decoding failed\n");
			goto error;
		} else if ( pictureStatus.state==NEXUS_PictureDecoderState_eMoreData ) {
			rc = NEXUS_PictureDecoder_GetBuffer(pictureDecoder, &buffer, &size); /* get location and size of the decoder's buffer */
			if ( size > bytesRemain ) size = bytesRemain;

			if ( rc == NEXUS_SUCCESS ) {
				rc = fread(buffer, 1, size, fin);							  /* read file into the decoder's buffer */

				if(rc) {
					NEXUS_PictureDecoder_ReadComplete(pictureDecoder, 0, rc); /* commit data to the decoder */
					bytesRemain -= size;
				}
				else {
					BDBG_MSG(( "COULDN'T READ IN any more data !!! rc=%d bytesRemain=%d\n" , rc, bytesRemain  ));
					cnt = 999;
				}
			}
		}
		usleep(1000);
		if (++cnt == 1000) { /* 1 second */
			BDBG_ERR(("unable to read header. aborting."));
			goto error;
		}
	} while(!pictureStatus.headerValid); /* wait for picture dimensions */

	/* create picture that could handle complete picture */
	NEXUS_Surface_GetDefaultCreateSettings(&createSettings);
	createSettings.pixelFormat = pictureStatus.header.format;
	createSettings.width = pictureStatus.header.surface.width;
	createSettings.height = pictureStatus.header.surface.height;
	createSettings.alignment = 2;
    {
        NEXUS_PixelFormatInfo pixelFormatInfo;
        NEXUS_PixelFormat_GetInfo(createSettings.pixelFormat, &pixelFormatInfo);
        /* can't rely on default pitch 0 because we must round up to 4 byte alignment for SID */
        createSettings.pitch = (createSettings.width*pixelFormatInfo.bpp + 7) / 8;
        createSettings.pitch = ((createSettings.pitch + 3) & ~3);
        BDBG_ASSERT(createSettings.pitch % 4 == 0);
    }
	picture = NEXUS_Surface_Create(&createSettings);
	BDBG_WRN(("creating surface: format %d, %dx%d, pitch %d", createSettings.pixelFormat, createSettings.width, createSettings.height, createSettings.pitch));

	/* copy palette if needed */
	if (NEXUS_PIXEL_FORMAT_IS_PALETTE(createSettings.pixelFormat)) {
		NEXUS_PictureDecoderPalette sidPalette;
		NEXUS_SurfaceMemory mem;

		rc = NEXUS_PictureDecoder_GetPalette(pictureDecoder, &sidPalette);
		BDBG_ASSERT(!rc);
		NEXUS_Surface_GetMemory(picture, &mem),
		BKNI_Memcpy(mem.palette, sidPalette.palette, mem.numPaletteEntries*sizeof(NEXUS_PixelFormat));
		NEXUS_Surface_Flush(picture);
	}

	if ( file_size > (unsigned)pictureBufferSize ) {
		fprintf(stderr, "File requires multi part read, SID fw requires read from start\n");
		fseek(fin, 0, SEEK_SET);
		NEXUS_PictureDecoder_GetBuffer(pictureDecoder, &buffer, &size); /* get location and size of the decoder's buffer */
		rc = fread(buffer, 1, size, fin);								/* read file into the decoder's buffer */

		if(rc) {
			   NEXUS_PictureDecoder_ReadComplete(pictureDecoder, 0, rc); /* commit data to the decoder */
		}
		else {
			fprintf(stderr, "failed to read in data for %s \n" , pictureFilename );
			goto error;
		}
	}

	/* start decoding */
	NEXUS_PictureDecoder_DecodeSegment(pictureDecoder, picture, NULL);
	do {
		NEXUS_PictureDecoder_GetStatus(pictureDecoder, &pictureStatus);
		if(pictureStatus.state==NEXUS_PictureDecoderState_eError) {
			fprintf(stderr, "decoding failed\n");
			goto error;
		} else if ( pictureStatus.state==NEXUS_PictureDecoderState_eMoreData ) {
			rc = NEXUS_PictureDecoder_GetBuffer(pictureDecoder, &buffer, &size); /* get location and size of the decoder's buffer */
			if ( rc == NEXUS_SUCCESS ) {
				rc = fread(buffer, 1, size, fin);							  /* read file into the decoder's buffer */
				if ( size > bytesRemain ) size = bytesRemain;

				if(rc) {
					NEXUS_PictureDecoder_ReadComplete(pictureDecoder, 0, rc); /* commit data to the decoder */
					bytesRemain -= size;
				}
				else {
					fprintf( stderr, "rc=%d size=%d ..COULDN'T READ IN any more data !!! \n", rc, size );
					goto error;
				}
			}
			else {
				BDBG_MSG(("failed to get buffer to read data into\n" ));
			}
		}
		usleep(1000);
	} while(pictureStatus.state!=NEXUS_PictureDecoderState_eSegmentDone);	/* wait for picture to decode */

	NEXUS_PictureDecoder_Stop(pictureDecoder);

	NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);
	blitSettings.source.surface = picture;
	blitSettings.source.rect.x = 0;
	blitSettings.source.rect.y = 0;
	blitSettings.source.rect.width = pictureStatus.header.width;
	if(NEXUS_PIXEL_FORMAT_IS_YCRCB(pictureStatus.header.format)) {
		blitSettings.source.rect.width += blitSettings.source.rect.width%2; /* YCrCb single pixel has width of 2 */
	}

	blitSettings.source.rect.height = pictureStatus.header.height;
	blitSettings.colorOp = NEXUS_BlitColorOp_eCopySource;
	blitSettings.alphaOp = NEXUS_BlitAlphaOp_eCopySource;

	blitSettings.output.surface = framebuffer;
	blitSettings.output.rect.x = 0;
	blitSettings.output.rect.y = 0;
	blitSettings.output.rect.width = videoFormatInfo.width; /* fill to fit entire screen */
	blitSettings.output.rect.height = videoFormatInfo.height;

	NEXUS_Graphics2D_Blit(gfx, &blitSettings);				/* don't wait for blit to complete */
	NEXUS_Graphics2D_Checkpoint(gfx, NULL);

	NEXUS_Display_GetGraphicsSettings(display, &graphicsSettings);
	graphicsSettings.enabled = true;
	NEXUS_Display_SetGraphicsSettings(display, &graphicsSettings);
	NEXUS_Display_SetGraphicsFramebuffer(display, framebuffer);
#endif

    /* initalize HDCP settings, keys, etc. */
    initializeHdcpSettings() ;

    while (!done)
    {
        int tmp;

        printf("Main Menu\n");
        printf("0) Exit\n");
        printf("4) hdmiOutput Status\n");
        printf("Enter Selection: ");
        scanf("%d", &tmp);
        switch ( tmp )
        {
        case 0:
            done = true;
            break;

        case 4:
            hdmi_output_status() ;
            break;

        default:
            printf("Unsupported Menu Choice %d", tmp) ;
            break;
        }
    }


#if NEXUS_HAS_PICTURE_DECODER

error:
	/* Now clean up in reverse order */
	if (picture) {
		NEXUS_Surface_Destroy(picture);
	}
	if (pictureDecoder) {
		NEXUS_PictureDecoder_Close(pictureDecoder);
	}
	if (pPictureBuffer) {
		NEXUS_Memory_Free(pPictureBuffer);
	}
	if (pMultiScanBuffer) {
		NEXUS_Memory_Free(pMultiScanBuffer);
	}

	NEXUS_Surface_Destroy(framebuffer);
	NEXUS_Graphics2D_Close(gfx);
#endif

    /* stop/remove HDMI callbacks associated with display,
    so those callbacks do not access display once it is removed */
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

