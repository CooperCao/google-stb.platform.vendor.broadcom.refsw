/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#if NEXUS_HAS_HDMI_INPUT && NEXUS_HAS_SIMPLE_DECODER
#include "nxclient.h"
#include "nexus_surface_client.h"
#include "nexus_simple_video_decoder.h"
#include "nexus_simple_audio_decoder.h"
#include "nexus_hdmi_input.h"
#include "nexus_hdmi_input_hdcp.h"
#include "nexus_hdmi_output_hdcp.h"
#include "nexus_display.h"

#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <sys/time.h>
#include <fcntl.h>

#include "bstd.h"
#include "bkni.h"
#include "nxapps_cmdline.h"
#include "namevalue.h"


static NxClient_HdcpVersion hdcpVersion = NxClient_HdcpVersion_eMax ; /* maximum HDCP version to support */
                                                                   /* eMax = No HDCP support */
static const char *filename1x = "./hdcp1xRxKeys.bin" ;
static const char *filename2x = "./drm.bin" ;


BDBG_MODULE(hdmi_input);

#define appHdmiInputHpChangedCbId 0
#define appHdmiOutputHdcpChangedCbId 2
#define appDisplaySettingsChangedCbId 1


static void print_usage(const struct nxapps_cmdline *cmdline)
{
    printf(
        "Usage: hdmi_input\n"
        "  --help or -h for help\n"
        "  -index #\n"
        "  -prompt\n"
        "  -secure\n"
        "  -pip                     sets -rect and -zorder for picture-in-picture\n"
        "  -track_source            change display format to match HDMI input format\n"
        "  -audio off\n"
        "  -video off\n"
        ) ;
    printf(
        "  -hdcp_version {auto|hdcp1x|hdcp22}\n"
        "      auto   - Indicate support for the highest HDCP version supported by this platform\n"
        "      hdcp22 - Indicate support for HDCP 2.2 and 1.x\n"
        "      hdcp1x - Indicate support for HDCP 1.x only\n"
        "  -hdcp2x_keys BINFILE \tspecify location of Hdcp2.x bin file\n"
        "  -hdcp1x_keys BINFILE \tspecify location of Hdcp1.x bin file\n"
        );
    nxapps_cmdline_print_usage(cmdline);
}

static struct {
    NEXUS_HdmiInputHandle hdmiInput;
    NEXUS_HdmiOutputHandle hdmiOutput;
    bool secureVideo;
} g_app;


static void displayKeyLoadStatus(uint8_t success)
{
    BDBG_LOG(("HDCP 1.x Key Loading: %s", success ? "SUCCESS" : " FAILED")) ;
}


/**********************/
/* Load HDCP 1.x Keys */
/**********************/
static NEXUS_Error initializeHdmInputHdcp1xKeys(void)
{
    static const unsigned char hdcp1xHeader[] = {0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00} ;
    NEXUS_Error errCode = NEXUS_SUCCESS ;
    NEXUS_HdmiInputHdcpKeyset hdmiRxKeyset ;
    NEXUS_HdmiInputHdcpStatus hdcpStatus ;

    int fileFd;
    char tmp[8];

    fileFd = open(filename1x, O_RDONLY);
    if (fileFd < 0)
    {
        BDBG_ERR(("Open file error <%d> for HDCP 1.x Rx Keys file '%s'; HDCP 1.x Tx devices may not work",
            fileFd, filename1x));
        errCode = NEXUS_NOT_AVAILABLE;
        goto done ;
    }

    read(fileFd, tmp, 8) ;
    if (BKNI_Memcmp(hdcp1xHeader, tmp, 8))
    {
        BDBG_WRN(("Invalid Header in 1.x HDCP Rx Key file")) ;
    }

    read(fileFd, tmp, 1) ;
    read(fileFd, tmp, 1) ;
    read(fileFd, (uint8_t *) &hdmiRxKeyset.alg, 1) ;

    read(fileFd, &hdmiRxKeyset.custKeyVarL, 1) ;
    read(fileFd, &hdmiRxKeyset.custKeyVarH, 1) ;
    read(fileFd, &hdmiRxKeyset.custKeySel, 1) ;

    read(fileFd, hdmiRxKeyset.rxBksv.data, NEXUS_HDMI_HDCP_KSV_LENGTH) ;
    read(fileFd, tmp, 3) ;

    read(fileFd, &hdmiRxKeyset.privateKey, NEXUS_HDMI_HDCP_NUM_KEYS * sizeof(NEXUS_HdmiInputHdcpKey));
    close(fileFd);

    errCode = NEXUS_HdmiInput_HdcpSetKeyset(g_app.hdmiInput, &hdmiRxKeyset) ;
    if (errCode)
    {
       /* display message informing of result of HDCP Key Load */
       displayKeyLoadStatus(0) ;
	   goto done ;
    }

    NEXUS_HdmiInput_HdcpGetStatus(g_app.hdmiInput, &hdcpStatus) ;

    /* display message informing of result of HDCP Key Load */
    /* NOTE: use of otpState is overloaded... refers to status of key load */
    if (hdcpStatus.eOtpState != NEXUS_HdmiInputHdcpKeySetOtpState_eCrcMatch)
       displayKeyLoadStatus(0) ;
    else
       displayKeyLoadStatus(1) ;

done:
    return errCode ;
}


/**********************/
/* Load HDCP 2.2 Keys */
/**********************/
static NEXUS_Error initializeHdmInputHdcp2xKeys(void)
{
    NEXUS_Error errCode = NEXUS_SUCCESS ;
#if NEXUS_HAS_SAGE
    int rc = 0;
    int fileFd;

    uint8_t *buffer = NULL;
    size_t fileSize;
    off_t seekPos;

    fileFd = open(filename2x, O_RDONLY);
    if (fileFd < 0) {
        BDBG_ERR(("Open file error <%d> for HDCP 2.x Rx Keys file '%s'; HDCP 2.x devices may not work",
            fileFd, filename2x));
        rc = NEXUS_SUCCESS ;
        goto done ;
    }


    fileFd = open(filename2x, O_RDONLY);
    if (fileFd < 0)
    {
        BDBG_ERR(("Unable to open bin file"));
        rc = 1;
        goto done;
    }

    seekPos = lseek(fileFd, 0, SEEK_END);
    if (seekPos < 0)
    {
        BDBG_ERR(("Unable to seek bin file size"));
        rc = 2;
        goto done;
    }
    fileSize = (size_t)seekPos;

    if (lseek(fileFd, 0, SEEK_SET) < 0)
    {
        BDBG_ERR(("Unable to get back to origin"));
        rc = 3;
        goto done;
    }

    buffer = BKNI_Malloc(fileSize);
    if (read(fileFd, (void *)buffer, fileSize) != (ssize_t)fileSize)
    {
        BDBG_ERR(("Unable to read all binfile"));
        rc = 6;
        goto done;
    }

    BDBG_LOG(("drm.bin file loaded buff=%p, size=%u", buffer, (unsigned)fileSize));
    errCode = NEXUS_HdmiInput_SetHdcp2xBinKeys(g_app.hdmiInput, buffer, (uint32_t)fileSize);
    if (errCode != NEXUS_SUCCESS)
    {
        BDBG_ERR(("Error setting Hdcp2x encrypted keys. HDCP2.x devices may not work"));
        goto done ;
    }

    BDBG_LOG(("HDCP 2.2 Key Loading: SUCCESS")) ;


done:
    if (fileFd)    {
        close(fileFd);
    }

    if (buffer) {
        BKNI_Free(buffer);
    }

    if (rc)
    {
        BDBG_ERR(("error #%d, fileSize=%u, seekPos=%d", rc, (unsigned)fileSize, (unsigned)seekPos));
        BDBG_ASSERT(false);
    }
#endif
    return errCode;

}


static NEXUS_Error initializeHdmiInputHdcpSettings(void)
{
    NEXUS_Error errCode = NEXUS_SUCCESS ;

    switch (hdcpVersion)
    {
    case  NxClient_HdcpVersion_eMax :
        errCode = NEXUS_SUCCESS ;
        goto done ;

    case  NxClient_HdcpVersion_eAuto :
        /* *** FALL THROUGH - to higheset HDCP support available  */

    case  NxClient_HdcpVersion_eHdcp22 :

        errCode = initializeHdmInputHdcp2xKeys() ;
        if (errCode)
        {
            goto done ;
        }

        /* *** FALL THROUGH - to add HDCP 1.x key support */
        /* required when HDCP 2.2 is specified  */

    case  NxClient_HdcpVersion_eHdcp1x :
        errCode = initializeHdmInputHdcp1xKeys() ;
        if (errCode)
        {
            goto done ;
        }
        break ;

    default :
        BDBG_WRN(("Unsupported NxClient HDCP level %d", hdcpVersion)) ;
        errCode = NEXUS_UNKNOWN ;
    }


done :
    return errCode ;
 }


/* changing output params to match input params is not required */
static void source_changed(void *context, int param)
{
    NEXUS_Error rc;
    NEXUS_HdmiInputStatus hdmiInputStatus;
    NxClient_DisplaySettings displaySettings;

    BSTD_UNUSED(context);
    BSTD_UNUSED(param);

    NxClient_GetDisplaySettings(&displaySettings);
    NEXUS_HdmiInput_GetStatus(g_app.hdmiInput, &hdmiInputStatus);
    if (!hdmiInputStatus.validHdmiStatus) {
        displaySettings.hdmiPreferences.hdcp = NxClient_HdcpLevel_eNone;
    }
    else {
        NEXUS_HdmiOutputStatus hdmiOutputStatus;
        NEXUS_HdmiOutput_GetStatus(g_app.hdmiOutput, &hdmiOutputStatus);
        if (displaySettings.format != hdmiInputStatus.originalFormat && hdmiOutputStatus.videoFormatSupported[hdmiInputStatus.originalFormat]) {
            BDBG_WRN(("video format %s to %s", lookup_name(g_videoFormatStrs, displaySettings.format),
                lookup_name(g_videoFormatStrs, hdmiInputStatus.originalFormat)));
            displaySettings.format = hdmiInputStatus.originalFormat;
        }

        if (hdmiInputStatus.colorSpace != displaySettings.hdmiPreferences.colorSpace)
        {
            BDBG_WRN(("color space %s -> %s", lookup_name(g_colorSpaceStrs, displaySettings.hdmiPreferences.colorSpace),
                lookup_name(g_colorSpaceStrs, hdmiInputStatus.colorSpace)));
            displaySettings.hdmiPreferences.colorSpace = hdmiInputStatus.colorSpace;
        }
        if (hdmiInputStatus.colorDepth != displaySettings.hdmiPreferences.colorDepth) {
            BDBG_WRN(("color depth %u -> %u", displaySettings.hdmiPreferences.colorDepth, hdmiInputStatus.colorDepth));
            displaySettings.hdmiPreferences.colorDepth = hdmiInputStatus.colorDepth;
        }
    }

    rc = NxClient_SetDisplaySettings(&displaySettings);
    if (rc) {
        BDBG_ERR(("Unable to set Display Settings (errCode= %d)", rc));
    }
}


static void hdmiRxHdcpStateChanged(void *context, int param)
{
    NEXUS_HdmiInputHdcpStatus hdmiRxHdcpStatus;
    NEXUS_HdmiOutputHdcpStatus hdmiTxHdcpStatus;
    bool rxAuthenticated ;
    bool repeaterAuthenticated ;
    NEXUS_Error rc;

    BSTD_UNUSED(context);
    BSTD_UNUSED(param);

    /* check the authentication state and process accordingly */

    /***********************/
    /* HDMI Rx HDCP status */
    /***********************/
    rc = NEXUS_HdmiInput_HdcpGetStatus(g_app.hdmiInput, &hdmiRxHdcpStatus);
    if (rc)
    {
        BDBG_ERR(("%s: Error getting Rx HDCP status", BSTD_FUNCTION)) ;
        BERR_TRACE(rc) ;
        return ;
    }


    /***********************/
    /* HDMI Tx HDCP status */
    /***********************/
    rc = NEXUS_HdmiOutput_GetHdcpStatus(g_app.hdmiOutput, &hdmiTxHdcpStatus);
    if (rc)
    {
        BDBG_ERR(("%s: Error getting Tx HDCP status", BSTD_FUNCTION)) ;
        BERR_TRACE(rc) ;
        return ;
    }


    /**************************************/
    /*  Rx HDCP 2.x Authentication Status */
    /**************************************/
    if (hdmiRxHdcpStatus.version == NEXUS_HdcpVersion_e2x)
    {
        rxAuthenticated =
            hdmiRxHdcpStatus.hdcpState == NEXUS_HdmiInputHdcpState_eAuthenticated ;
        repeaterAuthenticated =
            hdmiRxHdcpStatus.hdcpState == NEXUS_HdmiInputHdcpState_eRepeaterAuthenticated ;

        BDBG_LOG(("%s: [-->Rx-STB-Tx] HDCP Ver: %s; Content Stream Protection: %d; Upstream Status: %s Authenticated ",
            BSTD_FUNCTION,
            (hdmiRxHdcpStatus.version == NEXUS_HdcpVersion_e2x) ? "2.2" : "1.x",
            hdmiRxHdcpStatus.hdcp2xContentStreamControl,
            rxAuthenticated ? "Rx" :
            repeaterAuthenticated ? "Repeater" : "FAILED"));

        BDBG_LOG(("%s: [Rx-STB-Tx-->] HDCP 2.2 support downstream: %s; HDCP 1.x device downstream: %s",
            BSTD_FUNCTION,
            hdmiTxHdcpStatus.hdcp2_2Features ? "Yes" : "No",
            !hdmiTxHdcpStatus.hdcp2_2Features ? "Yes" :
                hdmiTxHdcpStatus.hdcp2_2RxInfo.hdcp1_xDeviceDownstream ? "Yes" : "No")) ;

        if ((hdmiRxHdcpStatus.hdcpState != NEXUS_HdmiInputHdcpState_eAuthenticated)
        && (hdmiRxHdcpStatus.hdcpState != NEXUS_HdmiInputHdcpState_eRepeaterAuthenticated))
        {
            BDBG_WRN(("%s: HDCP2.2 Auth from upstream Tx: FAILED", BSTD_FUNCTION));
            return;
        }

        if (hdmiRxHdcpStatus.hdcp2xContentStreamControl == NEXUS_Hdcp2xContentStream_eType1)
        {
            /* upstream auth requires HDCP 2.2; Start Tx auth only if connected to HDCP 2.2 device */
            BDBG_LOG(("Upstream HDCP Content Stream Control is Type 1")) ;
            if (!hdmiTxHdcpStatus.hdcp2_2Features)
            {
               /*** Rx does not support HDCP 2.2; CONTENT MUST BE BLOCKED ***/
                BDBG_WRN(("Attached device does not support HDCP 2.2; Downstream authentication blocked")) ;
                return ;
            }

            if (hdmiTxHdcpStatus.hdcp2_2RxInfo.hdcp1_xDeviceDownstream)
            {
                /* upstream auth also requires no HDCP 1.x devices downstream; CONTENT MUST BE BLOCKED ***/
                BDBG_WRN(("Attached 1.x devices downstream;  Downstream authentication blocked")) ;
                return ;
            }
        }
    }

    /*******************************/
    /*  Rx HDCP 1.x Authentication */
    /*******************************/
    else
    {

        BDBG_MSG(("%s: HDCP Authentication State: %d",
            BSTD_FUNCTION, hdmiRxHdcpStatus.eAuthState)) ;

        switch (hdmiRxHdcpStatus.eAuthState) {
        case NEXUS_HdmiInputHdcpAuthState_eKeysetInitialization :
            BDBG_LOG(("%s Change in HDCP Key Set detected: %u",
                BSTD_FUNCTION, hdmiRxHdcpStatus.eKeyStorage));
            break;

        case NEXUS_HdmiInputHdcpAuthState_eWaitForKeyloading :
            BDBG_LOG(("%s: Upstream HDCP 1.x Authentication request ...", BSTD_FUNCTION));
            break;

        case NEXUS_HdmiInputHdcpAuthState_eWaitForDownstreamKsvs :
            /* Repeater Rx is considered authenticated when upstream requests KSvs */
            BDBG_LOG(("%s KSV FIFO Request; Start hdmiOutput Authentication...", BSTD_FUNCTION));

            if ((hdmiTxHdcpStatus.hdcpState != NEXUS_HdmiOutputHdcpState_eWaitForRepeaterReady)
            && (hdmiTxHdcpStatus.hdcpState != NEXUS_HdmiOutputHdcpState_eCheckForRepeaterReady))
            {
                rxAuthenticated = true ;
            }
            break;

        case NEXUS_HdmiInputHdcpAuthState_eKsvFifoReady :
            BDBG_LOG(("%s KSV FIFO Ready...", BSTD_FUNCTION));
            break;

        case NEXUS_HdmiInputHdcpAuthState_eIdle:
            BDBG_MSG(("%s: Auth State: Idle...", BSTD_FUNCTION));
            goto done ;
            break;

        default:
            BDBG_WRN(("%s: Unknown State: %d", BSTD_FUNCTION,  hdmiRxHdcpStatus.eAuthState ));
            goto done ;
            break;
        }
    }

    BDBG_LOG(("RxAuthenticated: %d", rxAuthenticated)) ;
    if (rxAuthenticated)
    {
        BDBG_LOG(("%s: Repeater's Rx is authenticated. Start Repeater's Tx downstream authentication",
            BSTD_FUNCTION)) ;

        rc = NxClient_SetHdmiInputRepeater(g_app.hdmiInput) ;
        if (rc != NEXUS_SUCCESS)
        {
            BDBG_ERR(("%s: Error %d starting HDCP downstream authentication",
                    BSTD_FUNCTION, rc)) ;
            rc = BERR_TRACE(rc) ;
        }
    }
    else if (repeaterAuthenticated)
    {
        BDBG_LOG(("%s: Repeater Tx downstream authentication completed", BSTD_FUNCTION)) ;
    }
    else
    {
        BDBG_LOG(("%s: HDCP2.2 Upstream Authentication status - Current state: [%d]", BSTD_FUNCTION, hdmiRxHdcpStatus.hdcpState));
    }

done:
    return ;
}


static void hdmiTxHotplugCallback(void)
{
    NEXUS_HdmiOutputStatus hdmiTxStatus ;
    NEXUS_HdmiOutputBasicEdidData hdmiOutputBasicEdidData;
    NEXUS_HdmiOutputEdidBlock edidBlock;
    uint8_t *attachedRxEdid = NULL ;
    uint16_t attachedRxEdidSize = 0 ;
    unsigned i, j;
    NEXUS_Error rc = NEXUS_SUCCESS;

    NEXUS_HdmiOutput_GetStatus(g_app.hdmiOutput, &hdmiTxStatus) ;

    if ( !hdmiTxStatus.connected )
    {
        /* device disconnected. Load internal EDID. */
        rc = NEXUS_HdmiInput_LoadEdidData(g_app.hdmiInput, attachedRxEdid, attachedRxEdidSize) ;
        if (rc) BERR_TRACE(rc);
        goto done ;
    }


    /* Get EDID of attached receiver */
    rc = NEXUS_HdmiOutput_GetBasicEdidData(g_app.hdmiOutput, &hdmiOutputBasicEdidData);
    if (rc) {
        BDBG_ERR(("Unable to get downstream EDID; Use declared EDID in app for repeater's EDID"));
        BERR_TRACE(rc) ;
        goto load_edid;
    }

    /* allocate space to hold the EDID blocks */
    attachedRxEdidSize = (hdmiOutputBasicEdidData.extensions + 1) * sizeof(edidBlock.data);
    attachedRxEdid = BKNI_Malloc(attachedRxEdidSize);
    if (!attachedRxEdid) {
        BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        attachedRxEdid = NULL ;
        attachedRxEdidSize = 0 ;
        goto load_edid;
    }

    /* copy Attached EDID for presentation to the upstream Tx */
    for (i = 0; i <= hdmiOutputBasicEdidData.extensions; i++)
    {
        rc = NEXUS_HdmiOutput_GetEdidBlock(g_app.hdmiOutput, i, &edidBlock);
        if (rc)
        {
            BDBG_ERR(("%s: Error retrieving EDID Block %d from attached receiver;", BSTD_FUNCTION, i));
            attachedRxEdidSize = 0 ;
            goto load_edid;
        }

        for (j=0; j < sizeof(edidBlock.data); j++) {
            attachedRxEdid[i*sizeof(edidBlock.data)+j] = edidBlock.data[j];
        }
    }

load_edid:
    /* TODO: manipulate EDID to add/remove capabilities */
    rc = NEXUS_HdmiInput_LoadEdidData(g_app.hdmiInput, attachedRxEdid, attachedRxEdidSize);
    if (rc) BERR_TRACE(rc) ;

    if (attachedRxEdid)
    {
        BKNI_Free(attachedRxEdid);
    }

    if (hdmiTxStatus.rxPowered)
    {
        rc  = initializeHdmiInputHdcpSettings() ;
        if (rc != NEXUS_SUCCESS)
        {
            BDBG_ERR(("%s: Error InitializeHdmiInputHdcpSettings", BSTD_FUNCTION));
            rc = BERR_TRACE(rc) ;
            goto done ;
        }
    }

done:
    BDBG_LOG(("%s: device %s; toggle Rx HPD to notify upstream device...",
        BSTD_FUNCTION, hdmiTxStatus.connected ? "connected" : "removed"));

    NEXUS_HdmiInput_ToggleHotPlug(g_app.hdmiInput);
}


static void hdmiTxHdcpStateChanged(void)
{
    NEXUS_Error rc ;
    NEXUS_HdmiOutputHdcpStatus hdmiTxHdcpStatus;
    NEXUS_HdmiOutputHdcpSettings hdmiTxHdcpSettings ;
    NEXUS_HdmiHdcpDownStreamInfo downStream  ;
    NEXUS_HdmiHdcpKsv *pKsvs ;
    unsigned returnedDevices ;
    uint8_t i ;

    rc = NEXUS_HdmiOutput_GetHdcpStatus(g_app.hdmiOutput, &hdmiTxHdcpStatus);
    if (rc) BERR_TRACE(rc);

    switch (hdmiTxHdcpStatus.hdcpState)
    {
    case NEXUS_HdmiOutputHdcpState_eUnpowered:
        BDBG_ERR(("Attached Device is unpowered")) ;
        goto done;
        break;

    case NEXUS_HdmiOutputHdcpState_eUnauthenticated:
        /* Unauthenticated - no hdcp error */
        if (hdmiTxHdcpStatus.hdcpError == NEXUS_HdmiOutputHdcpError_eSuccess)
        {
            BDBG_MSG(("*** HDCP was disabled as requested (NEXUS_HdmiOutput_DisableHdcpAuthentication was called)***"));
            goto done;
        }

        /* Unauthenticated - with hdcp authentication error */
        else {
            BDBG_LOG(("*** HDCP Authentication failed - Error: %d - ***", hdmiTxHdcpStatus.hdcpError));
            /* nxserver already retry downstream authentication */
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
        BDBG_LOG(("*** Authenticating with attached Receiver/Repeater - current state: %d ***", hdmiTxHdcpStatus.hdcpState));
        goto done;
        break;


    case NEXUS_HdmiOutputHdcpState_eLinkAuthenticated:
    case NEXUS_HdmiOutputHdcpState_eEncryptionEnabled:
        /* HDCP successfully authenticated */
        BDBG_LOG(("*** HDCP Authentication Successful ***\n"));
        goto uploadDownstreamInfo;
        break;


    case NEXUS_HdmiOutputHdcpState_eRepeaterAuthenticationFailure:
    case NEXUS_HdmiOutputHdcpState_eRiLinkIntegrityFailure:
    case NEXUS_HdmiOutputHdcpState_ePjLinkIntegrityFailure:
    case NEXUS_HdmiOutputHdcpState_eR0LinkFailure:
        /* HDCP authentication fail - in particular, link integrity check fail */
        BDBG_LOG(("*** HDCP Authentication failed - Error: %d - ***", hdmiTxHdcpStatus.hdcpError));

        NEXUS_HdmiOutput_GetHdcpSettings(g_app.hdmiOutput, &hdmiTxHdcpSettings);
        if ((hdmiTxHdcpSettings.hdcp_version == NEXUS_HdmiOutputHdcpVersion_e1_x)
        || ((!hdmiTxHdcpStatus.hdcp2_2Features)
            && (hdmiTxHdcpSettings.hdcp_version == NEXUS_HdmiOutputHdcpVersion_eAuto)))
        {
            switch (hdmiTxHdcpStatus.hdcpError)
            {
            case NEXUS_HdmiOutputHdcpError_eRxDevicesExceeded:
            case NEXUS_HdmiOutputHdcpError_eRepeaterDepthExceeded:
                goto uploadDownstreamInfo;
                break;
            default:
                break;
            }
        }
        /* nxserver already retry downstream authentication */
        break;

    default:
        BDBG_ERR(("*** Invalid HDCP authentication state ***"));
        break;
    }


uploadDownstreamInfo:
    BDBG_LOG(("%s Uploading downstream info...", BSTD_FUNCTION));

    /* Load Rx KSV FIFO for upstream device */
    NEXUS_HdmiOutput_GetHdcpSettings(g_app.hdmiOutput, &hdmiTxHdcpSettings);

    /* If HDCP 2.2 version was selected or AUTO mode was selected AND the Rx support HDCP 2.2 */
    if (hdmiTxHdcpStatus.hdcp2_2Features
    && ((hdmiTxHdcpSettings.hdcp_version == NEXUS_HdmiOutputHdcpVersion_e2_2)
    ||  (hdmiTxHdcpSettings.hdcp_version == NEXUS_HdmiOutputHdcpVersion_eAuto)))
    {
        BSTD_UNUSED(pKsvs);
        BSTD_UNUSED(downStream);
    }
    else
    {
        /* HDCP 1.x */
        NEXUS_HdmiOutput_HdcpGetDownstreamInfo(g_app.hdmiOutput, &downStream) ;

        /* allocate space to hold ksvs for the downstream devices */
        pKsvs = BKNI_Malloc((downStream.devices) * NEXUS_HDMI_HDCP_KSV_LENGTH) ;
        rc = NEXUS_HdmiOutput_HdcpGetDownstreamKsvs(g_app.hdmiOutput,
            pKsvs, downStream.devices, &returnedDevices) ;
        if (rc) {rc = BERR_TRACE(rc) ;}

        BDBG_LOG(("hdmiOutput Downstream Levels:  %d  Devices: %d",
            downStream.depth, downStream.devices)) ;

        /* display the downstream device KSVs */
        for (i = 0 ; i < downStream.devices; i++)
        {
            BDBG_LOG(("Device %02d BKsv: %02X %02X %02X %02X %02X",
                i + 1,
                *(pKsvs->data + i + 4), *(pKsvs->data + i + 3),
                *(pKsvs->data + i + 2), *(pKsvs->data + i + 1),
                *(pKsvs->data + i ))) ;
        }

        rc = NEXUS_HdmiInput_HdcpLoadKsvFifo(g_app.hdmiInput,
            &downStream, pKsvs, downStream.devices) ;
        if (rc) {rc = BERR_TRACE(rc) ;}

        BKNI_Free(pKsvs) ;
    }
    /* Dowstream device IDs/KSVs have been uploaded */
    goto done ;

done:
    return ;

}


void nxclient_callback(void *context, int param)
{
    BSTD_UNUSED(context);
    BDBG_MSG(("NxClient Callback ID: %d", param)) ;

    switch (param) {
    case appHdmiInputHpChangedCbId :
        hdmiTxHotplugCallback();
        break;

    case appHdmiOutputHdcpChangedCbId :
        hdmiTxHdcpStateChanged() ;
        break;

    case appDisplaySettingsChangedCbId :
        break;

    default :
        BDBG_WRN(("Unsupported callback id: %d", param)) ;
        break;
    }
}

int main(int argc, const char **argv)  {
	NEXUS_Error errCode ;
    NxClient_JoinSettings joinSettings;
    NxClient_AllocSettings allocSettings;
    NxClient_AllocResults allocResults;
    NxClient_ConnectSettings connectSettings;
    NEXUS_SurfaceClientHandle surfaceClient, videoSurfaceClient;
    NEXUS_SimpleVideoDecoderHandle videoDecoder = NULL;
    NEXUS_SimpleAudioDecoderHandle audioDecoder = NULL;
    NEXUS_SimpleStcChannelHandle stcChannel;
    NEXUS_HdmiInputSettings hdmiInputSettings;
    NEXUS_HdmiInputHdcpSettings hdmiInputHdcpSettings;
    NxClient_CallbackThreadSettings callbackThreadSettings;
    unsigned connectId;
    int curarg = 1;
    int rc;
    bool prompt = false;
    unsigned index = 0;
    struct nxapps_cmdline cmdline;
    int n;
    NxClient_VideoWindowType videoWindowType = NxClient_VideoWindowType_eMain;
    bool track_source = false;
    struct stat buf;
    bool video = true, audio = true;

    nxapps_cmdline_init(&cmdline);
    nxapps_cmdline_allow(&cmdline, nxapps_cmdline_type_SurfaceComposition);

    while (curarg < argc) {
        if (!strcmp(argv[curarg], "-h") || !strcmp(argv[curarg], "--help")) {
            print_usage(&cmdline);
            return 0;
        }
        else if (!strcmp(argv[curarg], "-secure")) {
            g_app.secureVideo = true;
        }
        else if (!strcmp(argv[curarg], "-prompt")) {
            prompt = true;
        }
        else if (!strcmp(argv[curarg], "-index") && curarg+1 < argc) {
            index = atoi(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-pip")) {
            const char *argv[] = {"-rect","960,0,960,540","-zorder","1"};
            nxapps_cmdline_parse(0, 2, argv, &cmdline);
            nxapps_cmdline_parse(2, 4, argv, &cmdline);
            videoWindowType = NxClient_VideoWindowType_ePip;
        }
        else if (!strcmp(argv[curarg], "-track_source")) {
            track_source = true;
        }
        else if (!strcmp(argv[curarg], "-hdcp_version") && argc>curarg+1) {
            curarg++;

            /* for -hdcp_version auto, present the highest HDCP version supported */
            if      (!strcmp(argv[curarg],"auto"  ))  hdcpVersion = NxClient_HdcpVersion_eAuto;
            else if (!strcmp(argv[curarg],"hdcp1x"))  hdcpVersion = NxClient_HdcpVersion_eHdcp1x;
            else if (!strcmp(argv[curarg],"hdcp22"))  hdcpVersion = NxClient_HdcpVersion_eHdcp22;
            else {
                BDBG_ERR(("Incorrectly specified -hdcp_version option")) ;
                printf("\n\n") ;
                print_usage(&cmdline);
                return -1;
            }
        }
        else if (!strcmp(argv[curarg], "-hdcp2x_keys")) {
            filename2x = argv[++curarg] ;
        }
        else if (!strcmp(argv[curarg], "-hdcp1x_keys")) {
            filename1x = argv[++curarg] ;
        }
        else if (!strcmp(argv[curarg], "-video") && curarg+1<argc) {
            video = strcmp(argv[++curarg], "off");
        }
        else if (!strcmp(argv[curarg], "-audio") && curarg+1<argc) {
            audio = strcmp(argv[++curarg], "off");
        }
        else if ((n = nxapps_cmdline_parse(curarg, argc, argv, &cmdline))) {
            if (n < 0) {
                print_usage(&cmdline);
                return -1;
            }
            curarg += n;
        }
        else {
            print_usage(&cmdline);
            return -1;
        }
        curarg++;
    }

    switch (hdcpVersion)
    {
    case NxClient_HdcpVersion_eAuto :
#if NEXUS_HAS_SAGE
    case NxClient_HdcpVersion_eHdcp22 :
        if (stat(filename2x, &buf) != 0)
        {
            BDBG_ERR(("Unable to find HDCP 2.2 bin file <%s>\n", filename2x)) ;
            return -1 ;
        }

        /* FALL THROUGH - If HDCP 2.2 is specified, HDCP 1.x must also be available */
#endif

    case NxClient_HdcpVersion_eHdcp1x :
        if (stat(filename1x, &buf) != 0)
        {
            BDBG_ERR(("Unable to find HDCP 1.x bin file <%s>\n", filename1x)) ;
            return -1 ;
        }
        break ;

	case NxClient_HdcpVersion_eMax :
        /* no HDCP support */
        break ;

    default :
        BDBG_WRN(("Unsupported NxClient HDCP level %d", hdcpVersion)) ;
        break ;
    }

    NxClient_GetDefaultJoinSettings(&joinSettings);
    snprintf(joinSettings.name, NXCLIENT_MAX_NAME, "%s", argv[0]);
    rc = NxClient_Join(&joinSettings);
    if (rc) {
        printf("cannot join: %d\n", rc);
        return -1;
    }

    NxClient_GetDefaultAllocSettings(&allocSettings);
    allocSettings.simpleVideoDecoder = video?1:0;
    allocSettings.simpleAudioDecoder = audio?1:0;
    allocSettings.surfaceClient = video?1:0; /* surface client required for video window */
    rc = NxClient_Alloc(&allocSettings, &allocResults);
    if (rc) {BDBG_WRN(("unable to alloc transcode resources")); return -1;}

    if (allocResults.simpleVideoDecoder[0].id) {
        videoDecoder = NEXUS_SimpleVideoDecoder_Acquire(allocResults.simpleVideoDecoder[0].id);
    }
    if (allocResults.simpleAudioDecoder.id) {
        audioDecoder = NEXUS_SimpleAudioDecoder_Acquire(allocResults.simpleAudioDecoder.id);
    }
    if (allocResults.surfaceClient[0].id) {
        /* surfaceClient is the top-level graphics window in which video will fit.
        videoSurfaceClient must be "acquired" to associate the video window with surface compositor.
        Graphics do not have to be submitted to surfaceClient for video to work, but at least an
        "alpha hole" surface must be submitted to punch video through other client's graphics.
        Also, the top-level surfaceClient ID must be submitted to NxClient_ConnectSettings below. */
        surfaceClient = NEXUS_SurfaceClient_Acquire(allocResults.surfaceClient[0].id);
        videoSurfaceClient = NEXUS_SurfaceClient_AcquireVideoWindow(surfaceClient, 0);
        BSTD_UNUSED(videoSurfaceClient);

        if (nxapps_cmdline_is_set(&cmdline, nxapps_cmdline_type_SurfaceComposition)) {
            NEXUS_SurfaceComposition comp;
            NxClient_GetSurfaceClientComposition(allocResults.surfaceClient[0].id, &comp);
            nxapps_cmdline_apply_SurfaceComposition(&cmdline, &comp);
            NxClient_SetSurfaceClientComposition(allocResults.surfaceClient[0].id, &comp);
        }
    }

    NxClient_GetDefaultConnectSettings(&connectSettings);
    connectSettings.simpleVideoDecoder[0].id = allocResults.simpleVideoDecoder[0].id;
    connectSettings.simpleVideoDecoder[0].surfaceClientId = allocResults.surfaceClient[0].id;
    connectSettings.simpleVideoDecoder[0].decoderCapabilities.connectType = NxClient_VideoDecoderConnectType_eWindowOnly;
    connectSettings.simpleVideoDecoder[0].windowCapabilities.type = videoWindowType;
    connectSettings.simpleAudioDecoder.id = allocResults.simpleAudioDecoder.id;
    rc = NxClient_Connect(&connectSettings, &connectId);
    if (rc) return BERR_TRACE(rc);

    NEXUS_HdmiInput_GetDefaultSettings(&hdmiInputSettings);
    hdmiInputSettings.frontend.hpdDisconnected = false;
    hdmiInputSettings.secureVideo = g_app.secureVideo;
    g_app.hdmiInput = NEXUS_HdmiInput_Open(index, &hdmiInputSettings);
    if (!g_app.hdmiInput) {
        BDBG_ERR(("HdmiInput %d not available", index));
        return -1;
    }

    /* open a read-only alias to get EDID. any changes must go through nxclient. */
    g_app.hdmiOutput = NEXUS_HdmiOutput_Open(0 + NEXUS_ALIAS_ID, NULL);
    if (!g_app.hdmiOutput) {
        BDBG_WRN(("Can't get hdmi output read-only alias\n"));
        return -1;
    }

    errCode = initializeHdmiInputHdcpSettings() ;
    if (errCode)
    {
        BDBG_ERR(("Error (%d) initializing HDCP Settings", errCode)) ;
        return NEXUS_UNKNOWN ;
    }

    if (track_source) {
        NEXUS_HdmiInput_GetSettings(g_app.hdmiInput, &hdmiInputSettings);
        hdmiInputSettings.sourceChanged.callback = source_changed;
        rc = NEXUS_HdmiInput_SetSettings(g_app.hdmiInput, &hdmiInputSettings);
        BDBG_ASSERT(!rc);
    }

    NEXUS_HdmiInput_HdcpGetDefaultSettings(g_app.hdmiInput, &hdmiInputHdcpSettings);
    /* chips with both hdmi rx and tx cores should always set repeater to TRUE */
    hdmiInputHdcpSettings.repeater = true;
    hdmiInputHdcpSettings.hdcpRxChanged.callback = hdmiRxHdcpStateChanged;
    rc = NEXUS_HdmiInput_HdcpSetSettings(g_app.hdmiInput, &hdmiInputHdcpSettings);
    BDBG_ASSERT(!rc);

    NxClient_GetDefaultCallbackThreadSettings(&callbackThreadSettings);
        callbackThreadSettings.hdmiOutputHotplug.callback = nxclient_callback;
        callbackThreadSettings.hdmiOutputHotplug.param = appHdmiInputHpChangedCbId ;

        callbackThreadSettings.hdmiOutputHdcpChanged.callback = nxclient_callback;
        callbackThreadSettings.hdmiOutputHdcpChanged.param = appHdmiOutputHdcpChangedCbId ;

        callbackThreadSettings.displaySettingsChanged.callback = nxclient_callback;
        callbackThreadSettings.displaySettingsChanged.param = appDisplaySettingsChangedCbId ;
    rc = NxClient_StartCallbackThread(&callbackThreadSettings);
    BDBG_ASSERT(!rc);

    stcChannel = NEXUS_SimpleStcChannel_Create(NULL);
    if (videoDecoder) {
        NEXUS_SimpleVideoDecoder_SetStcChannel(videoDecoder, stcChannel);
    }
    if (audioDecoder) {
        NEXUS_SimpleAudioDecoder_SetStcChannel(audioDecoder, stcChannel);
    }

    if (videoDecoder) {
        rc = NEXUS_SimpleVideoDecoder_StartHdmiInput(videoDecoder, g_app.hdmiInput, NULL);
        BDBG_ASSERT(!rc);
    }
    if (audioDecoder) {
        rc = NEXUS_SimpleAudioDecoder_StartHdmiInput(audioDecoder, g_app.hdmiInput, NULL);
        BDBG_ASSERT(!rc);
    }

    BDBG_LOG(("HdmiInput %d is ACTIVE", index));
    NEXUS_HdmiInput_ToggleHotPlug(g_app.hdmiInput);

    if (prompt) {
        BDBG_WRN(("Press ENTER to exit"));
        getchar();
    }
    else {
        while (1) BKNI_Sleep(1000);
    }

    NxClient_Disconnect(connectId);
    NxClient_Free(&allocResults);
    NxClient_Uninit();
    return rc;
}
#else
#include <stdio.h>
int main(void)
{
    printf("This application is not supported on this platform (needs hdmi_input and simple_decoder)!\n");
    return 0;
}
#endif
