/******************************************************************************
 * Copyright (C) 2018 Broadcom.
 * The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to
 * the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied),
 * right to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 * THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 * IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 * THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 * OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *****************************************************************************/
#include "blst_queue.h"
#include "bip.h"
#include <fcntl.h>

#if NXCLIENT_SUPPORT
#include "nxclient.h"
#include "nexus_platform_client.h"
#else
#include "nexus_platform.h"
#endif
#if NEXUS_HAS_HDMI_INPUT
#include "nexus_hdmi_input.h"
#include "nexus_hdmi_input_hdcp.h"
#endif

BDBG_MODULE(http_hdmi_input_streamer);

#if NEXUS_HAS_HDMI_INPUT
#define HTTP_SERVER_PORT_STRING "80"
#define HTTP_SERVER_DTCP_IP_AKE_PORT_STRING "8000"
#define MAX_TRACKED_EVENTS 2
#define DTCP_IP_KEY_FORMAT_COMMON_DRM   "commonDrm"
#define DTCP_IP_KEY_FORMAT_TEST         "test"

#define TRANSCODE_PROFILE_720p_AVC "720pAvc"
#define TRANSCODE_PROFILE_480p_AVC "480pAvc"

static const char *filename1x = NULL ;
static const char *filename2x = NULL ;

#define HDCP1X_KEYS_BIN_FILE    "./hdcp1xRxKeys.bin"
#define HDCP2X_KEYS_BIN_FILE    "./hdcp2xKeys.bin"

typedef struct AppCtx
{
    BIP_StringHandle            hPort;
    BIP_StringHandle            hDtcpIpAkePort;
    BIP_StringHandle            hInterfaceName;
    BIP_StringHandle            hDtcpIpKeyFormat;
    bool                        httpServerStarted;
    BIP_HttpServerHandle        hHttpServer;
    B_EventGroupHandle          hHttpEventGroup;
    B_EventHandle               hHttpRequestRcvdEvent;
    B_EventHandle               hHttpStreamerEvent;
    int                         maxTriggeredEvents;
    unsigned                    inactivityTimeoutInMs;
    bool                        disableAvHeadersInsertion;
    bool                        enableDtcpIp;
    bool                        enableSlaveMode;
    bool                        enableXcode;
    bool                        enableHls;
    bool                        enableMpegDash;
    char                        *xcodeProfile;
    bool                        printStatus;
    bool                        enableHwOffload;
    unsigned                    maxDataRate;
    bool                        enableHdcp;
    BIP_StringHandle            hHdcp1xBinFileName;
    BIP_StringHandle            hHdcp2xBinFileName;
    BLST_Q_HEAD( streamerListHead, AppStreamerCtx ) streamerListHead; /* List of Streamer Ctx */
} AppCtx;

typedef struct AppStreamerCtx
{
    BLST_Q_ENTRY( AppStreamerCtx ) streamerListNext;  /* Next Streamer Ctx */
    AppCtx                      *pAppCtx;
    const char                  *pMediaFileAbsolutePathName; /* Absolute URL Path Name pointer */
    const char                  *pMethodName;       /* HTTP Request Method Name */
    bool                        endOfStreamerRcvd;  /* We have received End of Streaming event for this Streamer. */
    BIP_HttpStreamerHandle      hHttpStreamer;        /* Cached Streamer handle */
    bool                        enableDtcpIp;
    bool                        enableSlaveMode;
    bool                        enableXcode;
    bool                        enableHls;
    bool                        enableMpegDash;
    bool                        enableTransportTimestamp;
    bool                        enableHwOffload;
    NEXUS_HdmiInputHandle       hHdmiInput;
    bool                        enableHdcp;
} AppStreamerCtx;

#define USER_INPUT_BUF_SIZE 64
/* function to allow proper exit of server */
int readInput( void )
{
    char    buffer[USER_INPUT_BUF_SIZE];
    size_t  bytesRead;

    BIP_Fd_ReadWithTimeout(STDIN_FILENO, sizeof(buffer)-1, 0 /*timeout*/, buffer, &bytesRead );
    buffer[bytesRead] = '\0';   /* Null-terminate whatever we read. */

    if (strstr(buffer, "q"))
    {
        BDBG_LOG((BIP_MSG_PRE_FMT "Received quit " BIP_MSG_PRE_ARG));
        return 'q';
    }
    else if (strstr(buffer, "s"))
    {
        return 's';
    }
    else return 'c'; /* continue */
    return false;
}

static void endOfStreamCallbackFromBIP(
    void *context,
    int   param
    )
{
    AppStreamerCtx *pAppStreamerCtx = context;

    BSTD_UNUSED( param );

    BDBG_WRN(( BIP_MSG_PRE_FMT " B_Event_Set( pAppCtx->hHttpStreamerEvent )" BIP_MSG_PRE_ARG ));
    pAppStreamerCtx->endOfStreamerRcvd = true;
    B_Event_Set( pAppStreamerCtx->pAppCtx->hHttpStreamerEvent );
}

void destroyAppStreamerCtx(
    AppStreamerCtx *pAppStreamerCtx
    )
{
    if (pAppStreamerCtx == NULL)
        return;

    B_Os_Free(pAppStreamerCtx);
}

void stopAndDestroyStreamer(
    AppStreamerCtx *pAppStreamerCtx
    )
{
    if (!pAppStreamerCtx->hHttpStreamer)
        return;
    BDBG_MSG(( BIP_MSG_PRE_FMT " Stopping Streamer %p" BIP_MSG_PRE_ARG, (void *)pAppStreamerCtx->hHttpStreamer));
    BIP_HttpServer_StopStreamer( pAppStreamerCtx->pAppCtx->hHttpServer, pAppStreamerCtx->hHttpStreamer );

    BDBG_MSG(( BIP_MSG_PRE_FMT " Destroying Streamer %p" BIP_MSG_PRE_ARG, (void *)pAppStreamerCtx->hHttpStreamer));
    BIP_HttpServer_DestroyStreamer( pAppStreamerCtx->pAppCtx->hHttpServer,  pAppStreamerCtx->hHttpStreamer );

#if NEXUS_HAS_HDMI_INPUT
    if (pAppStreamerCtx->hHdmiInput) NEXUS_HdmiInput_Close(pAppStreamerCtx->hHdmiInput);
    pAppStreamerCtx->hHdmiInput = NULL;
#endif

    return;
} /* stopAndDestroyStreamer */

#define HTTP_STREAMER_MAX_INACTIVITY_TIME_IN_MSEC 10000
bool stopAndDestroyInactiveStreamer(
    AppCtx          *pAppCtx
    )
{
    AppStreamerCtx          *pAppStreamerCtx;

    for ( pAppStreamerCtx = BLST_Q_FIRST(&pAppCtx->streamerListHead);
            pAppStreamerCtx;
            pAppStreamerCtx = BLST_Q_NEXT( pAppStreamerCtx, streamerListNext ) )
    {
        BIP_HttpStreamerStatus status;

        if ( BIP_HttpStreamer_GetStatus( pAppStreamerCtx->hHttpStreamer, &status ) == BIP_SUCCESS )
        {
            if ( status.inactivityTimeInMs >= HTTP_STREAMER_MAX_INACTIVITY_TIME_IN_MSEC )
            {
                BDBG_WRN(( BIP_MSG_PRE_FMT "Destroy inactive (=%dmsec) HttpStreamer=%p so as to Free-up Nexus Resources for the next request. " BIP_MSG_PRE_ARG, status.inactivityTimeInMs, (void *)pAppStreamerCtx->hHttpStreamer ));
                BLST_Q_REMOVE( &pAppCtx->streamerListHead, pAppStreamerCtx, streamerListNext );
                stopAndDestroyStreamer( pAppStreamerCtx);
                destroyAppStreamerCtx(pAppStreamerCtx);
                return true;
            }
        }
        else BDBG_ASSERT( NULL );
    }
    BDBG_WRN(( BIP_MSG_PRE_FMT "No streamers inactive at this time (inactivityMaxTime=%d" BIP_MSG_PRE_ARG, HTTP_STREAMER_MAX_INACTIVITY_TIME_IN_MSEC ));
    return false;
} /* stopAndDestroyInactiveStreamer */

/* Function to process HttpStreamer related events */
static void processHttpStreamerEvent(
    AppCtx *pAppCtx
    )
{
    AppStreamerCtx          *pAppStreamerCtx;
    AppStreamerCtx          *pAppStreamerCtxNext;

    pAppStreamerCtx = BLST_Q_FIRST(&pAppCtx->streamerListHead);
    while ( pAppStreamerCtx )
    {
        if (pAppStreamerCtx->endOfStreamerRcvd)
        {
            /* BIP has indicated end of streaming, so cleanup! */
            pAppStreamerCtxNext = BLST_Q_NEXT( pAppStreamerCtx, streamerListNext );
            BLST_Q_REMOVE( &pAppStreamerCtx->pAppCtx->streamerListHead, pAppStreamerCtx, streamerListNext );
            stopAndDestroyStreamer( pAppStreamerCtx);
            destroyAppStreamerCtx(pAppStreamerCtx);
            pAppStreamerCtx = pAppStreamerCtxNext;
        }
        else
        {
            pAppStreamerCtx = BLST_Q_NEXT( pAppStreamerCtx, streamerListNext );
        }
    }
} /* processHttpStreamerEvent */

static BIP_Status rejectRequestAndSetResponseHeaders(
    AppCtx *pAppCtx,
    BIP_HttpRequestHandle  hHttpRequest,
    BIP_HttpResponseStatus responseStatus
    )
{
    BIP_Status bipStatus;
    BIP_HttpServerRejectRequestSettings rejectSettings;

    BIP_HttpServer_GetDefaultRejectRequestSettings(&rejectSettings);
    rejectSettings.httpStatus = responseStatus;
    rejectSettings.customResponseHeaders = "Custom Header1\r\nCustomHeader2\r\n";
    bipStatus = BIP_HttpServer_RejectRequest( pAppCtx->hHttpServer, hHttpRequest, &rejectSettings );
    BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_HttpServer_RejectRequest Failed" ), error, bipStatus, bipStatus );

    BDBG_ERR(( BIP_MSG_PRE_FMT "BIP_HttpServer_RejectRequest() for hHttpRequest %p" BIP_MSG_PRE_ARG, (void *)hHttpRequest));

error:
    return (bipStatus);
}


#if NXCLIENT_SUPPORT

static void displayKeyLoadStatus(uint8_t success)
{
    BDBG_LOG(("HDCP 1.x Key Loading: %s", success ? "SUCCESS" : " FAILED")) ;
}
/**********************/
/* Load HDCP 1.x Keys */
/**********************/
static NEXUS_Error initializeHdmInputHdcp1xKeys(AppStreamerCtx *pAppStreamerCtx)
{
    static const unsigned char hdcp1xHeader[] = {0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00} ;
    NEXUS_Error errCode = NEXUS_SUCCESS ;
    NEXUS_HdmiInputHdcpKeyset hdmiRxKeyset ;
    NEXUS_HdmiInputHdcpStatus hdcpStatus ;

    int fileFd = -1;
    char tmp[8];
    if(filename1x != NULL)
    {
        fileFd = open(filename1x, O_RDONLY);
    }

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

    errCode = NEXUS_HdmiInput_HdcpSetKeyset(pAppStreamerCtx->hHdmiInput, &hdmiRxKeyset) ;
    if (errCode)
    {
       /* display message informing of result of HDCP Key Load */
       displayKeyLoadStatus(0) ;
       goto done ;
    }

    NEXUS_HdmiInput_HdcpGetStatus(pAppStreamerCtx->hHdmiInput, &hdcpStatus) ;

    /* display message informing of result of HDCP Key Load */
    /* NOTE: use of otpState is overloaded... refers to status of key load */
    if (hdcpStatus.eOtpState != NEXUS_HdmiInputHdcpKeySetOtpState_eCrcMatch)
       displayKeyLoadStatus(0) ;
    else
       displayKeyLoadStatus(1) ;

done:
    if(fileFd != -1)
    {
        close(fileFd);
    }
    return errCode ;
}


/**********************/
/* Load HDCP 2.2 Keys */
/**********************/
static NEXUS_Error initializeHdmInputHdcp2xKeys(AppStreamerCtx *pAppStreamerCtx)
{
    NEXUS_Error errCode = NEXUS_SUCCESS ;
#if NEXUS_HAS_SAGE
    int rc = 0;
    int fileFd = -1;

    uint8_t *buffer = NULL;
    size_t fileSize = 0;
    off_t seekPos = 0;

    BDBG_LOG((" Opening Hdcp 2.2 file ============================= %s", filename2x));

    if(filename2x != NULL)
    {
        fileFd = open(filename2x, O_RDONLY);
    }

    if (fileFd < 0) {
        BDBG_ERR(("Open file error <%d> for HDCP 2.x Rx Keys file '%s'; HDCP 2.x devices will not work",
            fileFd, filename2x));
        rc = NEXUS_NOT_AVAILABLE ;
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

    BDBG_LOG(("%s file loaded buff=%p, size=%u", filename2x, buffer, (unsigned)fileSize));
    errCode = NEXUS_HdmiInput_SetHdcp2xBinKeys(pAppStreamerCtx->hHdmiInput, buffer, (uint32_t)fileSize);
    if (errCode != NEXUS_SUCCESS)
    {
        BDBG_ERR(("Error setting Hdcp2x encrypted keys. HDCP2.x devices may not work"));
        goto done ;
    }

    BDBG_LOG(("HDCP 2.2 Key Loading: SUCCESS")) ;

done:
    if (fileFd != -1)
    {
        close(fileFd);
    }

    if (buffer)
    {
        BKNI_Free(buffer);
    }

    if (rc)
    {
        BDBG_ERR(("error #%d, fileSize=%u, seekPos=%d", rc, (unsigned)fileSize, (unsigned)seekPos));
        /*BDBG_ASSERT(false); */
    }
#endif
    return errCode;
}

static NEXUS_Error initializeHdmiInputHdcpSettings(AppStreamerCtx *pAppStreamerCtx)
{
    NEXUS_Error errCode = NEXUS_SUCCESS ;
    NxClient_HdcpVersion        hdcpVersion = NxClient_HdcpVersion_eAuto;

    switch (hdcpVersion)
    {
    case  NxClient_HdcpVersion_eMax :
        errCode = NEXUS_SUCCESS ;
        goto done ;

    case  NxClient_HdcpVersion_eAuto :
        /* *** FALL THROUGH - to higheset HDCP support available  */

    case  NxClient_HdcpVersion_eHdcp22 :

        errCode = initializeHdmInputHdcp2xKeys(pAppStreamerCtx) ;
        if (errCode)
        {
            /* goto done ; TODO:I will enable it later when Hdcp2x is fully supported. */
            BDBG_WRN((BIP_MSG_PRE_FMT " Not able to find Hdcp2.2 bin file, We will not support Hdcp2.2" BIP_MSG_PRE_ARG));
        }

        /* *** FALL THROUGH - to add HDCP 1.x key support */
        /* required when HDCP 2.2 is specified  */

    case  NxClient_HdcpVersion_eHdcp1x :
        errCode = initializeHdmInputHdcp1xKeys(pAppStreamerCtx) ;
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

static NEXUS_HdmiInputHandle       g_hHdmiInput = NULL;

static void hdmiRxHdcpStateChanged(void *context, int param)
{
    NEXUS_HdmiInputHdcpStatus hdmiRxHdcpStatus;
    bool rxAuthenticated = false;
    bool repeaterAuthenticated = false;
    NEXUS_Error rc;

    BSTD_UNUSED(context);
    BSTD_UNUSED(param);

    /* check the authentication state and process accordingly */

    /***********************/
    /* HDMI Rx HDCP status */
    /***********************/
    rc = NEXUS_HdmiInput_HdcpGetStatus(g_hHdmiInput, &hdmiRxHdcpStatus);
    if (rc)
    {
        BDBG_ERR(("%s: Error getting Rx HDCP status", BSTD_FUNCTION)) ;
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

        rc = NxClient_SetHdmiInputRepeater(g_hHdmiInput) ;
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
#endif

static BIP_Status startStreamer(
    AppStreamerCtx *pAppStreamerCtx,
    BIP_HttpRequestHandle hHttpRequest
    )
{
    BIP_Status bipStatus = BIP_SUCCESS;
    BIP_HttpResponseStatus responseStatus = BIP_HttpResponseStatus_e400_BadRequest;

    /* Create HttpStreamer. */
    {
        BIP_HttpServerCreateStreamerSettings createStreamerSettings;

        BIP_HttpServer_GetDefaultCreateStreamerSettings( &createStreamerSettings );
        createStreamerSettings.endOfStreamingCallback.callback = endOfStreamCallbackFromBIP;
        createStreamerSettings.endOfStreamingCallback.context = pAppStreamerCtx;

        pAppStreamerCtx->hHttpStreamer = BIP_HttpServer_CreateStreamer( pAppStreamerCtx->pAppCtx->hHttpServer, &createStreamerSettings );
        responseStatus = BIP_HttpResponseStatus_e500_InternalServerError;
        BIP_CHECK_GOTO(( pAppStreamerCtx->hHttpStreamer ), ( "BIP_HttpServer_CreateStreamer Failed" ), rejectRequest, BIP_ERR_CREATE_FAILED, bipStatus );
        BDBG_MSG(( BIP_MSG_PRE_FMT " BIP_HttpStreamer created: %p" BIP_MSG_PRE_ARG, (void *)pAppStreamerCtx->hHttpStreamer ));
    }

    /* Now provide Hdmi input settings */
    {
        int i;
        for (i=0; i<NEXUS_NUM_HDMI_INPUTS; i++)
        {
            pAppStreamerCtx->hHdmiInput = NEXUS_HdmiInput_Open(i, NULL);
            if (pAppStreamerCtx->hHdmiInput) break;
        }
        responseStatus = BIP_HttpResponseStatus_e500_InternalServerError;
        BIP_CHECK_GOTO(( pAppStreamerCtx->hHdmiInput ), ( "NEXUS_HdmiInput_Open Failed: no free HDMI input available!" ), rejectRequest, BIP_INF_NEXUS_RESOURCE_NOT_AVAILABLE, bipStatus );

        if(pAppStreamerCtx->enableHdcp)
        {
#if NXCLIENT_SUPPORT
            NEXUS_HdmiInputHdcpSettings hdmiInputHdcpSettings;
            NEXUS_Error errCode = NEXUS_SUCCESS ;
            errCode = initializeHdmiInputHdcpSettings(pAppStreamerCtx) ;
            if (errCode)
            {
                BDBG_ERR(("Error (%d) initializing HDCP Settings", errCode)) ;
                responseStatus = BIP_HttpResponseStatus_e500_InternalServerError;
                BIP_CHECK_GOTO(( pAppStreamerCtx->hHdmiInput ), ( "initializeHdmiInputHdcpSettings Failed: not able to enable Hdcp in HDMI input!" ), rejectRequest, BIP_INF_NEXUS_RESOURCE_NOT_AVAILABLE, bipStatus );
            }

            /* Right now this is added after debugging one issue, we found that default maxVersion is set as 1x,
               That is the reason Hdcp2.2 was not working. With this changes  I am able to get Hdcp 2.2 working.
               This can be removed once Hdcp team make changes in Nexus.*/
            NEXUS_HdmiInput_HdcpGetDefaultSettings(pAppStreamerCtx->hHdmiInput, &hdmiInputHdcpSettings);
            /* chips with both hdmi rx and tx cores should always set repeater to TRUE */
            g_hHdmiInput = pAppStreamerCtx->hHdmiInput;
            hdmiInputHdcpSettings.repeater = false;
            hdmiInputHdcpSettings.hdcpRxChanged.callback = hdmiRxHdcpStateChanged;
            hdmiInputHdcpSettings.maxVersion = NEXUS_HdcpVersion_e2x;

            errCode = NEXUS_HdmiInput_HdcpSetSettings(pAppStreamerCtx->hHdmiInput, &hdmiInputHdcpSettings);
            BDBG_ASSERT(!errCode);

#else
            BDBG_WRN((BIP_MSG_PRE_FMT " WARNING:<========================================================>" BIP_MSG_PRE_ARG));
            BDBG_WRN((BIP_MSG_PRE_FMT " WARNING:<========= Hdcp only supported in Nxclient mode =========>" BIP_MSG_PRE_ARG));
            BDBG_WRN((BIP_MSG_PRE_FMT " WARNING:<========================================================>" BIP_MSG_PRE_ARG));
#endif
        }

        /* Set input Settings */
        bipStatus = BIP_HttpStreamer_SetHdmiInputSettings( pAppStreamerCtx->hHttpStreamer, pAppStreamerCtx->hHdmiInput, NULL);
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_HttpStreamer_SetHdmiInputSettings Failed" ), rejectRequest, bipStatus, bipStatus );

        BDBG_MSG(( BIP_MSG_PRE_FMT "Method: %s, Absolute URL: %s, hHdmiInput=%p" BIP_MSG_PRE_ARG,
                    pAppStreamerCtx->pMethodName, pAppStreamerCtx->pMediaFileAbsolutePathName, pAppStreamerCtx->hHdmiInput));
    } /* Input Settings. */

    /* Provide the Streamer output settings */
    {
        BIP_HttpStreamerProtocol streamerProtocol;
        BIP_HttpStreamerOutputSettings streamerOutputSettings;

        BIP_HttpStreamer_GetDefaultOutputSettings( &streamerOutputSettings );
        if (pAppStreamerCtx->enableDtcpIp)
        {
            streamerOutputSettings.enableDtcpIp = true;
            streamerOutputSettings.dtcpIpOutput.pcpPayloadLengthInBytes = (1024*1024);
            streamerOutputSettings.dtcpIpOutput.akeTimeoutInMs = 2000;
            streamerOutputSettings.dtcpIpOutput.copyControlInfo = B_CCI_eCopyNever;
        }

        streamerOutputSettings.streamerSettings.enableHwOffload = pAppStreamerCtx->enableHwOffload;

        streamerProtocol = pAppStreamerCtx->enableHls ? BIP_HttpStreamerProtocol_eHls : BIP_HttpStreamerProtocol_eDirect;
        streamerOutputSettings.streamerSettings.mpeg2Ts.enableTransportTimestamp = pAppStreamerCtx->enableTransportTimestamp;
        streamerOutputSettings.disableAvHeadersInsertion = pAppStreamerCtx->pAppCtx->disableAvHeadersInsertion;

        bipStatus = BIP_HttpStreamer_SetOutputSettings(pAppStreamerCtx->hHttpStreamer, streamerProtocol, &streamerOutputSettings);
        responseStatus = BIP_HttpResponseStatus_e500_InternalServerError;
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_HttpStreamer_SetOutputSettings Failed" ), rejectRequest, bipStatus, bipStatus );
    }

    /* Add Encoder Profile. */
    {
#if NEXUS_HAS_VIDEO_ENCODER
        BIP_TranscodeProfile transcodeProfile;

        if ( pAppStreamerCtx->enableHls )
        {
            if ( strncmp( pAppStreamerCtx->pAppCtx->xcodeProfile, TRANSCODE_PROFILE_480p_AVC, strlen(TRANSCODE_PROFILE_480p_AVC) ) == 0 )
            /* Add a 480p profile. */
            {
                BIP_Transcode_GetDefaultProfileFor_480p30_AVC_AAC_MPEG2_TS( &transcodeProfile );
                BDBG_WRN(("HLS 480pAVC Transcode profile"));
            }
            else
            /* Add a 720p profile. */
            {
                BIP_Transcode_GetDefaultProfileFor_720p30_AVC_AAC_MPEG2_TS( &transcodeProfile );
                BDBG_WRN(("HLS 720pAVC Transcode profile"));
            }
            transcodeProfile.video.startSettings.adaptiveLowDelayMode = false;
            transcodeProfile.video.settings.streamStructure.adaptiveDuration  = false;
            transcodeProfile.video.settings.streamStructure.duration  = 2000; /* 1 sec GOP, 1 Segment/GOP */
            bipStatus = BIP_HttpStreamer_AddTranscodeProfile( pAppStreamerCtx->hHttpStreamer, &transcodeProfile );
            responseStatus = BIP_HttpResponseStatus_e500_InternalServerError;
            BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_HttpStreamer_AddTranscodeProfile Failed" ), rejectRequest, bipStatus, bipStatus );
        }
        else if ( pAppStreamerCtx->enableXcode )
        {
            if ( strncmp( pAppStreamerCtx->pAppCtx->xcodeProfile, TRANSCODE_PROFILE_720p_AVC, strlen(TRANSCODE_PROFILE_720p_AVC) ) == 0 )
            {
                BIP_Transcode_GetDefaultProfileFor_720p30_AVC_AAC_MPEG2_TS( &transcodeProfile );
            }
            else if ( strncmp( pAppStreamerCtx->pAppCtx->xcodeProfile, TRANSCODE_PROFILE_480p_AVC, strlen(TRANSCODE_PROFILE_480p_AVC) ) == 0 )
            {
                BIP_Transcode_GetDefaultProfileFor_480p30_AVC_AAC_MPEG2_TS( &transcodeProfile );
            }
            else
            {
                BIP_Transcode_GetDefaultProfile( &transcodeProfile );
            }
            transcodeProfile.video.startSettings.adaptiveLowDelayMode = false;
            transcodeProfile.video.settings.streamStructure.adaptiveDuration  = false;
            bipStatus = BIP_HttpStreamer_AddTranscodeProfile( pAppStreamerCtx->hHttpStreamer, &transcodeProfile );
            responseStatus = BIP_HttpResponseStatus_e500_InternalServerError;
            BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_HttpStreamer_AddTranscodeProfile Failed" ), rejectRequest, bipStatus, bipStatus );
        }
#else
        {
            BDBG_ERR(( BIP_MSG_PRE_FMT "HDMI streaming can't be supported as NEXUS Video Encoder Feature is not supported for this platform" BIP_MSG_PRE_ARG ));
            goto rejectRequest;
        }
#endif
    }

    /* At this point, we have set sucessfully configured the Input & Output settings of the HttpStreamer. */


    /* Now Set any custom or app specific HTTP Headers that HttpStreamer should include when it sends out the Response Message. */
    {
        bipStatus = BIP_HttpStreamer_SetResponseHeader( pAppStreamerCtx->hHttpStreamer, "transferMode.dlna.org", "Streaming" );
        responseStatus = BIP_HttpResponseStatus_e500_InternalServerError;
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS || bipStatus == BIP_INF_HTTP_MESSAGE_HEADER_ALREADY_SET),
                ( "BIP_HttpStreamer_SetResponseHeader Failed: hHttpStreamer %p", (void *)pAppStreamerCtx->hHttpStreamer ), rejectRequest, bipStatus, bipStatus );

        /* App can add more custom headers here using this example above! */
        bipStatus = BIP_HttpStreamer_SetResponseHeader( pAppStreamerCtx->hHttpStreamer, "Access-Control-Allow-Origin", "*" );
        responseStatus = BIP_HttpResponseStatus_e500_InternalServerError;
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS || bipStatus == BIP_INF_HTTP_MESSAGE_HEADER_ALREADY_SET),
                ( "BIP_HttpStreamer_SetResponseHeader Failed: hHttpStreamer %p", (void *)pAppStreamerCtx->hHttpStreamer ), rejectRequest, bipStatus, bipStatus );
    }

    /* And as a last step, start the HttpStreamer. */
    while ( true )
    {
        BIP_HttpServerStartStreamerSettings startSettings;

        BIP_HttpServer_GetDefaultStartStreamerSettings( &startSettings );
        startSettings.streamerStartSettings.inactivityTimeoutInMs = pAppStreamerCtx->pAppCtx->inactivityTimeoutInMs;
        bipStatus = BIP_HttpServer_StartStreamer( pAppStreamerCtx->pAppCtx->hHttpServer, pAppStreamerCtx->hHttpStreamer, hHttpRequest, &startSettings);
        if ( bipStatus == BIP_INF_NEXUS_RESOURCE_NOT_AVAILABLE )
        {
            /* Since Streamer start failed because a Nexus resource wasn't available. We will try to see if there are any inactive streamers and destroy them. */
            if ( stopAndDestroyInactiveStreamer( pAppStreamerCtx->pAppCtx ) == true )
            {
                BDBG_WRN(( BIP_MSG_PRE_FMT "Successfully Released an active Streamer, Retrying BIP_HttpServer_StartStreamer()" BIP_MSG_PRE_ARG ));
                continue;
            }
            else break; /* No inactive streamers at this time, goto reject request. */
        }
        else break; /* Some other error, so no point in destroying the inactive streamers. */
    }
    responseStatus = BIP_HttpResponseStatus_e500_InternalServerError;
    BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_HttpServer_StartStreamer Failed: hHttpStreamer %p", (void *)pAppStreamerCtx->hHttpStreamer ), rejectRequest, bipStatus, bipStatus );
    return (bipStatus);

rejectRequest:
    /* This label handles the error cases when app encounters an error before BIP_HttpServer_StartStreamer(). */
    /* Thus it needs to reject the current hHttpRequest. */
    rejectRequestAndSetResponseHeaders( pAppStreamerCtx->pAppCtx, hHttpRequest, responseStatus );

#if NEXUS_HAS_HDMI_INPUT
    if (pAppStreamerCtx->hHdmiInput) NEXUS_HdmiInput_Close(pAppStreamerCtx->hHdmiInput);
    pAppStreamerCtx->hHdmiInput = NULL;
#endif
    if (pAppStreamerCtx->hHttpStreamer)
    {
        BDBG_MSG(( BIP_MSG_PRE_FMT " Destroying Streamer %p" BIP_MSG_PRE_ARG, (void *)pAppStreamerCtx->hHttpStreamer));
        /* Note: App doesn't need to destroy the streamer & can maintain a free-list of them. */
        BIP_HttpServer_DestroyStreamer( pAppStreamerCtx->pAppCtx->hHttpServer,  pAppStreamerCtx->hHttpStreamer );
        pAppStreamerCtx->hHttpStreamer = NULL;
    }

    /*
     * This label handles the error cases when app encounters an error during BIP_HttpServer_StartStreamer().
     * In that case, BIP_HttpServer_StartStreamer() has internally send out the error response.
     * Caller will cleanup the AppStreamerCtx.
     */
    return (bipStatus);

} /* startStreamer */

/* Function to process incoming Requests */
static void processHttpRequestEvent(
    AppCtx *pAppCtx
    )
{
    BIP_Status                  bipStatus = BIP_SUCCESS;
    BIP_HttpRequestMethod       method;
    AppStreamerCtx              *pAppStreamerCtx = NULL;
    BIP_HttpRequestHandle       hHttpRequest;
    BIP_HttpResponseStatus      responseStatus = BIP_HttpResponseStatus_e500_InternalServerError;
    BIP_StringHandle            hTarget = NULL;
    BIP_StringHandle            hInfoFileAbsolutePathName = NULL;

    hTarget = BIP_String_Create();
    hInfoFileAbsolutePathName = BIP_String_Create();

    while (true)
    {
        hHttpRequest = NULL;
        /* Check if there is a HTTP Request available for processing! */
        {
            bipStatus = BIP_HttpServer_RecvRequest( pAppCtx->hHttpServer, &hHttpRequest, NULL );
            if ( bipStatus == BIP_INF_TIMEOUT )
            {
                if(hTarget)
                {
                    BIP_String_Destroy(hTarget);
                }

                if(hInfoFileAbsolutePathName)
                {
                    BIP_String_Destroy(hInfoFileAbsolutePathName);
                }

                /* No request available at this time, return! */
                BDBG_MSG(( "No HTTPRequest is currently available, we are done for now! " ));
                return;
            }
            /* Make sure it is not an error while receiving the request! */
            BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_HttpServer_RecvRequest Failed: bipStatus 0x%x", bipStatus ), error, bipStatus, bipStatus );
        }

        /* Successfully Received a Request, create Streamer specific app context */
        {
            pAppStreamerCtx = B_Os_Calloc( 1, sizeof( AppStreamerCtx ));
            responseStatus = BIP_HttpResponseStatus_e500_InternalServerError;
            BIP_CHECK_GOTO(( pAppStreamerCtx ), ( "Memory Allocation Failed" ), rejectRequest, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

            pAppStreamerCtx->pAppCtx = pAppCtx;
            BDBG_MSG(( BIP_MSG_PRE_FMT "AppStreamerCtx %p: Setting up Streamer" BIP_MSG_PRE_ARG, (void *)pAppStreamerCtx));

            /* Copy Global Settings into Streamer instance. */
            pAppStreamerCtx->enableSlaveMode = pAppCtx->enableSlaveMode;
            pAppStreamerCtx->enableXcode = pAppCtx->enableXcode;
            pAppStreamerCtx->enableHls = pAppCtx->enableHls;
            pAppStreamerCtx->enableMpegDash = pAppCtx->enableMpegDash;
            pAppStreamerCtx->enableHwOffload = pAppCtx->enableHwOffload;
            pAppStreamerCtx->enableHdcp = pAppCtx->enableHdcp;
            responseStatus = BIP_HttpResponseStatus_e500_InternalServerError;
        }

        /* Process the received Request, start w/ the Method */
        {
            const char *pTmpUrlPath;

            bipStatus = BIP_HttpRequest_GetMethod( hHttpRequest, &method, &pAppStreamerCtx->pMethodName );
            responseStatus = BIP_HttpResponseStatus_e400_BadRequest;
            BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_HttpRequest_GetMethod Failed" ), rejectRequest, bipStatus, bipStatus );

            /* Validate the Request Method! */
            responseStatus = BIP_HttpResponseStatus_e501_NotImplemented;
            BIP_CHECK_GOTO( (method == BIP_HttpRequestMethod_eHead || method == BIP_HttpRequestMethod_eGet),
                    ( BIP_MSG_PRE_FMT " ERROR: method (%s) is not recognized/supported, only HEAD & GET are supported!" BIP_MSG_PRE_ARG, pAppStreamerCtx->pMethodName ),
                    rejectRequest, BIP_ERR_INVALID_REQUEST_TARGET, bipStatus );

            /* Retrieve the requested URL */
            bipStatus = BIP_HttpRequest_GetTarget( hHttpRequest, &pTmpUrlPath);
            responseStatus = BIP_HttpResponseStatus_e400_BadRequest;
            BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_HttpRequest_GetTarget Failed" ), rejectRequest, bipStatus, bipStatus );

            /* Note: app may retrieve any private/custom app specific Headers from the Request using BIP_HttpRequest_GetHeader(). */

            /* Before we do that, this example also uses some additional file name extensions to indicate if client is request HLS, Xcode, or other stream variations. */
            /*
             * Here are few examples of the supported extensions:
             * -normal coded:           AbcMpeg2HD.mpg
             * -HLS encoded:            AbcMpeg2HD.mpg.m3u8
             * -MPEG DASH encoded:      AbcMpeg2HD.mpg.mpd
             */
            {
                const char *pTmp;

                bipStatus = BIP_String_StrcpyChar(hTarget, pTmpUrlPath);
                BIP_CHECK_GOTO(( bipStatus==BIP_SUCCESS ), ( "BIP_String_StrcpyChar() Failed" ), rejectRequest, BIP_ERR_CREATE_FAILED, bipStatus );

                /* Trim out the extension from the URL String. */
                if ( (pTmp = strstr( BIP_String_GetString(hTarget), ".m3u8" )) != NULL )
                {
                    pAppStreamerCtx->enableHls = true;
                    bipStatus = BIP_String_Trim(hTarget, pTmp, 0 );
                }
                else if ( (pTmp = strstr( BIP_String_GetString(hTarget), ".mpd" )) != NULL )
                {
                    pAppStreamerCtx->enableMpegDash = true;
                    bipStatus = BIP_String_Trim(hTarget, pTmp, 0 );
                }
                else
                {
                    if ( (pTmp = strstr( BIP_String_GetString(hTarget), ".dtcpIp" )) != NULL )
                    {
                        pAppStreamerCtx->enableDtcpIp = true;
                        bipStatus = BIP_String_Trim(hTarget, pTmp, strlen(".dtcpIp") );
                        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_String_Trim Failed" ), rejectRequest, bipStatus, bipStatus );
                    }
                    if ( (pTmp = strstr( BIP_String_GetString(hTarget), ".tts" )) != NULL )
                    {
                        pAppStreamerCtx->enableTransportTimestamp = true;
                        bipStatus = BIP_String_Trim(hTarget, pTmp, strlen(".tts") );
                        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_String_Trim Failed" ), rejectRequest, bipStatus, bipStatus );
                    }
                }
                responseStatus = BIP_HttpResponseStatus_e500_InternalServerError;
                BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_String_Trim Failed" ), rejectRequest, bipStatus, bipStatus );

                /* Now trim any query string from the Request Target until we support parsing & interpretting it. */
                if ( (pTmp = strstr( BIP_String_GetString(hTarget), "?" )) != NULL )
                {
                    bipStatus = BIP_String_Trim(hTarget, pTmp, 0 );
                    responseStatus = BIP_HttpResponseStatus_e500_InternalServerError;
                    BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_String_Trim Failed" ), rejectRequest, bipStatus, bipStatus );
                }
            }
            /* We have a valid request. */
        }

        /* All is well, so Start Streamer */
        {
            bipStatus = startStreamer( pAppStreamerCtx, hHttpRequest);
            BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "startStreamer Failed" ), error, bipStatus, bipStatus );

            /* Streamer is started, now track this streamer context. */
            BLST_Q_INSERT_TAIL( &pAppCtx->streamerListHead, pAppStreamerCtx, streamerListNext );

            BDBG_MSG(( BIP_MSG_PRE_FMT "AppStreamerCtx %p: Streaming is started!" BIP_MSG_PRE_ARG, (void *)pAppStreamerCtx));
        }
        continue;

rejectRequest:
        /* Some error happened, so reject the current hHttpRequest. */
        rejectRequestAndSetResponseHeaders( pAppStreamerCtx->pAppCtx, hHttpRequest, responseStatus );
        destroyAppStreamerCtx(pAppStreamerCtx);
        pAppStreamerCtx = NULL;
        /* continue back to the top of the loop. */
    } /* while */

error:
    if(hTarget)
    {
        BIP_String_Destroy(hTarget);
    }
    if(hInfoFileAbsolutePathName)
    {
        BIP_String_Destroy(hInfoFileAbsolutePathName);
    }
    if (pAppStreamerCtx)
    {
        if (pAppStreamerCtx->hHttpStreamer) stopAndDestroyStreamer( pAppStreamerCtx );
        destroyAppStreamerCtx(pAppStreamerCtx);
    }
    BDBG_ERR(( BIP_MSG_PRE_FMT " Error case: DONE" BIP_MSG_PRE_ARG ));
    return;
} /* processHttpRequestEvent */

/* Callback from BIP_Server: indicates that one or more HttpRequest are ready for app to receive! */
static void requestReceivedCallbackFromBip(
    void *context,
    int   param
    )
{
    AppCtx *pAppCtx = context;
    BSTD_UNUSED( param );

    BDBG_MSG(( BIP_MSG_PRE_FMT " B_Event_Set( pAppCtx->hHttpRequestRcvdEvent )" BIP_MSG_PRE_ARG ));
    B_Event_Set( pAppCtx->hHttpRequestRcvdEvent );
} /* requestReceivedCallbackFromBip */

static void unInitHttpServer(
    AppCtx *pAppCtx
    )
{
    AppStreamerCtx *pAppStreamerCtx;

    if (!pAppCtx)
        return;
    BDBG_MSG(( BIP_MSG_PRE_FMT " appCtx %p" BIP_MSG_PRE_ARG, (void *)pAppCtx ));

    /* Stop the Server first, so that it doesn't accept any new connections. */
    if (pAppCtx->httpServerStarted)
        BIP_HttpServer_Stop( pAppCtx->hHttpServer );
    pAppCtx->httpServerStarted = false;

    /* Now Stop & Destroy any currently active streamers! */
    for (
         pAppStreamerCtx = BLST_Q_FIRST(&pAppCtx->streamerListHead);
         pAppStreamerCtx;
         pAppStreamerCtx = BLST_Q_FIRST(&pAppCtx->streamerListHead)
        )
    {
        BLST_Q_REMOVE( &pAppStreamerCtx->pAppCtx->streamerListHead, pAppStreamerCtx, streamerListNext );
        stopAndDestroyStreamer( pAppStreamerCtx );
        destroyAppStreamerCtx(pAppStreamerCtx);
    }

    if (pAppCtx->hHttpServer)
        BIP_HttpServer_Destroy( pAppCtx->hHttpServer );
    pAppCtx->hHttpServer = NULL;

    if (pAppCtx->hHttpRequestRcvdEvent) {B_EventGroup_RemoveEvent( pAppCtx->hHttpEventGroup, pAppCtx->hHttpRequestRcvdEvent ); }
    if (pAppCtx->hHttpStreamerEvent) {B_EventGroup_RemoveEvent( pAppCtx->hHttpEventGroup, pAppCtx->hHttpStreamerEvent ); }
    if (pAppCtx->hHttpRequestRcvdEvent) {B_Event_Destroy( pAppCtx->hHttpRequestRcvdEvent ); }
    if (pAppCtx->hHttpStreamerEvent) {B_Event_Destroy( pAppCtx->hHttpStreamerEvent ); }
    if (pAppCtx->hHttpEventGroup) {B_EventGroup_Destroy( pAppCtx->hHttpEventGroup ); }

} /* unInitHttpServer */

/* Initialize HttpServer */
static BIP_Status initHttpServer(
    AppCtx *pAppCtx
    )
{
    B_Error rc = B_ERROR_UNKNOWN;
    BIP_Status bipStatus = BIP_ERR_INTERNAL;

    /* Create events for callbacks */
    {
        /* Create Event group */
        pAppCtx->hHttpEventGroup = B_EventGroup_Create( NULL );
        BIP_CHECK_GOTO(( pAppCtx->hHttpEventGroup ), ( "Event Group Creation Failed" ), error, BIP_ERR_CREATE_FAILED, bipStatus );

        /* Create an event to indicate a callback on the listener */
        pAppCtx->hHttpRequestRcvdEvent = B_Event_Create( NULL );
        BIP_CHECK_GOTO(( pAppCtx->hHttpRequestRcvdEvent ), ( "Event Creation Failed" ), error, BIP_ERR_CREATE_FAILED, bipStatus );
        pAppCtx->maxTriggeredEvents++;
        /* Add this event to the event group */
        rc = B_EventGroup_AddEvent( pAppCtx->hHttpEventGroup, pAppCtx->hHttpRequestRcvdEvent );
        BIP_CHECK_GOTO(( !rc ), ( "Failed to Add event to group" ), error, BIP_ERR_INTERNAL, bipStatus );

        /* Create an event to indicate a callback on the session */
        pAppCtx->hHttpStreamerEvent = B_Event_Create( NULL );
        BIP_CHECK_GOTO(( pAppCtx->hHttpStreamerEvent ), ( "Event Creation Failed" ), error, BIP_ERR_CREATE_FAILED, bipStatus );
        pAppCtx->maxTriggeredEvents++;
        /* Add this event to the event group */
        rc = B_EventGroup_AddEvent( pAppCtx->hHttpEventGroup, pAppCtx->hHttpStreamerEvent );
        BIP_CHECK_GOTO(( !rc ), ( "Failed to Add event to group" ), error, BIP_ERR_INTERNAL, bipStatus );

        BIP_CHECK_GOTO( !( pAppCtx->maxTriggeredEvents > MAX_TRACKED_EVENTS ), ( "need to increase the MAX_TRACKED_EVENTS to %d", pAppCtx->maxTriggeredEvents ), error, BIP_ERR_INTERNAL, bipStatus );
    }

    /* Create HTTP Server */
    {
        pAppCtx->hHttpServer = BIP_HttpServer_Create( NULL ); /* Use default settings. */
        BIP_CHECK_GOTO(( pAppCtx->hHttpServer ), ( "BIP_HttpServer_Create Failed" ), error, BIP_ERR_CREATE_FAILED, bipStatus );
    }

    /* Set Server settings: Listening port & Callbacks */
    {
        BIP_HttpServerSettings httpServerSettings;
        BIP_HttpServer_GetSettings( pAppCtx->hHttpServer, &httpServerSettings );
        httpServerSettings.requestReceivedCallback.callback = requestReceivedCallbackFromBip;
        httpServerSettings.requestReceivedCallback.context = pAppCtx;
        bipStatus = BIP_HttpServer_SetSettings( pAppCtx->hHttpServer, &httpServerSettings );
        BIP_CHECK_GOTO(( !bipStatus ), ( "BIP_HttpServer_SetSettings Failed" ), error, bipStatus, bipStatus );
    }

    /* Start the Server */
    {
        BIP_HttpServerStartSettings httpServerStartSettings;

        BIP_HttpServer_GetDefaultStartSettings(&httpServerStartSettings);
        BDBG_MSG(("%s: Starting HttpServer...", BSTD_FUNCTION));

        httpServerStartSettings.pPort = BIP_String_GetString(pAppCtx->hPort);
        httpServerStartSettings.pInterfaceName = BIP_String_GetString(pAppCtx->hInterfaceName);

        if (pAppCtx->enableDtcpIp)
        {
            httpServerStartSettings.enableDtcpIp = true;
            BIP_DtcpIpServer_GetDefaultStartSettings( &httpServerStartSettings.dtcpIpServer );
            httpServerStartSettings.dtcpIpServer.pAkePort = BIP_String_GetString(pAppCtx->hDtcpIpAkePort);
            if (strcasecmp(BIP_String_GetString(pAppCtx->hDtcpIpKeyFormat), DTCP_IP_KEY_FORMAT_COMMON_DRM ) == 0)
            {
                httpServerStartSettings.dtcpIpServer.keyFormat = B_DTCP_KeyFormat_eCommonDRM;
            }
            else
            {
                httpServerStartSettings.dtcpIpServer.keyFormat = B_DTCP_KeyFormat_eTest;
            }
        }
        bipStatus = BIP_HttpServer_Start( pAppCtx->hHttpServer, &httpServerStartSettings );
        BIP_CHECK_GOTO(( bipStatus==BIP_SUCCESS ), ( "BIP_HttpServer_Start Failed" ), error, bipStatus, bipStatus );
    }

    pAppCtx->httpServerStarted = true;
    bipStatus = BIP_SUCCESS;

error:
    return ( bipStatus );
} /* initHttpServer */

static void printUsage(
    char   *pCmdName
    )
{
    printf( "Usage: %s\n", pCmdName);
    printf(
            "  --help or -h for help\n"
            "  -port            #   Server Port (default is port 80\n"
            "  -interface       #   Optional Interface name to bind the server to\n"
          );
    printf(
            "  -dtcpIp          #   Start DTCP/IP Server\n"
            "  -akePort         #   DTCP/IP Ake Port# (default 8000)\n"
            "  -dtcpIpKeyFormat <keyFormat> # keyFormat values are: [commonDrm | test]. Default is commonDrm \n"
            "  -slave           #   Start Server in slave mode (Client in Nexus Multi-Process)\n"
          );
    printf(
            "  -xcodeProfile    #   Pre-defined xcode profile string: 720pAvc (default), 480pAvc, \n"
            "  -stats           #   Print Periodic stats. \n"
            "  -hls             #   Enable HLS Output. \n"
            "  -enableHwOffload #   Enable streaming using ASP HW Offload Engine. \n"
          );
    printf(
            "  -inactivityTimeoutInMs <num> # Timeout in msec (default=60000) after which streamer will Stop/Close streaming context if there is no activity for that long! \n"
            "  -maxDataRate <num>           # Maximum data rate for the playback parser band in units of bits per second (default 40000000 (40Mpbs))! \n"
          );
    printf(
            "  -enableHdcp      #   Enbale Hdcp encrypted Hdmi input support. It is only supported in NXCLIENT(NXCLIENT_SUPPORT=y) mode.\n"
            "  -hdcp1xKeys  <binfile>  # specify location of Hdcp1.x bin file(default is ./hdcp1xRxKeys.bin). \n"
            "  -hdcp2xKeys  <binfile>  # specify location of Hdcp2.x bin file(default is ./hdcp2xKeys.bin). \n"
          );
    printf( "To enable some of the above options at runtime via the URL Request, add following suffix extension to the URL: \n");
    printf(
            "  .xcode           #   For enabling xcode. .e.g AbcMpeg2HD.mpg.xcode \n"
            "  .m3u8            #   For enabling HLS Streamer Protocol. e.g. AbcMpeg2HD.m3u8 \n"
            "  .dtcpIp          #   For enabling DTCP/IP Encryption. e.g. AbcMpeg2HD.dtcpIp \n"
          );
    exit(0);
} /* printUsage */

BIP_Status parseOptions(
    int    argc,
    char   *argv[],
    AppCtx *pAppCtx
    )
{
    int i;
    BIP_Status bipStatus = BIP_ERR_INTERNAL;

    pAppCtx->maxDataRate = 40*1000*1000;
    pAppCtx->enableXcode = true;

    for (i=1; i<argc; i++)
    {
        if ( !strcmp(argv[i], "-h") || !strcmp(argv[i], "--help") )
        {
            printUsage(argv[0]);
        }
        else if ( !strcmp(argv[i], "-port") && i+1<argc )
        {
            BIP_String_StrcpyChar( pAppCtx->hPort, argv[++i] );
        }
        else if ( !strcmp(argv[i], "-interface") && i+1<argc )
        {
            BIP_String_StrcpyChar( pAppCtx->hInterfaceName, argv[++i] );
        }
        else if ( !strcmp(argv[i], "-inactivityTimeoutInMs") && i+1<argc )
        {
            pAppCtx->inactivityTimeoutInMs = strtoul(argv[++i], NULL, 0);
        }
        else if ( !strcmp(argv[i], "-maxDataRate") && i+1<argc )
        {
            pAppCtx->maxDataRate = strtoul(argv[++i], NULL, 0);
        }
        else if ( !strcmp(argv[i], "-dtcpIp") )
        {
            pAppCtx->enableDtcpIp = true;
        }
        else if ( !strcmp(argv[i], "-akePort") && i+1<argc )
        {
            BIP_String_StrcpyChar( pAppCtx->hDtcpIpAkePort, argv[++i] );
        }
        else if ( !strcmp(argv[i], "-dtcpIpKeyFormat") && i+1<argc )
        {
            BIP_String_StrcpyChar( pAppCtx->hDtcpIpKeyFormat, argv[++i] );
        }
        else if ( !strcmp(argv[i], "-xcodeProfile") )
        {
            pAppCtx->xcodeProfile = argv[++i];
        }
        else if ( !strcmp(argv[i], "-stats") )
        {
            pAppCtx->printStatus = true;
        }
        else if ( !strcmp(argv[i], "-hls") )
        {
            pAppCtx->enableHls = true;
        }
        else if ( !strcmp(argv[i], "-enableHwOffload") )
        {
            pAppCtx->enableHwOffload = true;
        }
        else if ( !strcmp(argv[i], "-dontAddAvInfo") )
        {
            pAppCtx->disableAvHeadersInsertion = true;
        }
        else if ( !strcmp(argv[i], "-enableHdcp") )
        {
            pAppCtx->enableHdcp = true;
        }
        else if ( !strcmp(argv[i], "-hdcp1xKeys") )
        {
            BIP_String_StrcpyChar( pAppCtx->hHdcp1xBinFileName, argv[++i] );
        }
        else if ( !strcmp(argv[i], "-hdcp2xKeys") )
        {
            BIP_String_StrcpyChar( pAppCtx->hHdcp2xBinFileName, argv[++i] );
        }
        else
        {
            printf("Error: incorrect or unsupported option: %s\n", argv[i]);
            printUsage(argv[0]);
        }
    }
    if( pAppCtx->enableHdcp )
    {
        /* Initialize the global char pointers from which core function will read the file name. */
        if(pAppCtx->hHdcp1xBinFileName)
        {
            filename1x = BIP_String_GetString(pAppCtx->hHdcp1xBinFileName);
        }

        if(pAppCtx->hHdcp2xBinFileName)
        {
            filename2x = BIP_String_GetString(pAppCtx->hHdcp2xBinFileName);
        }
    }


    if ( !pAppCtx->xcodeProfile ) pAppCtx->xcodeProfile = TRANSCODE_PROFILE_720p_AVC;
    if (pAppCtx->inactivityTimeoutInMs == 0) pAppCtx->inactivityTimeoutInMs = 60000; /* 60sec default. */
    bipStatus = BIP_SUCCESS;
    BDBG_LOG(( BIP_MSG_PRE_FMT " port %s, interface %s, DTCP/IP %s: AKE Port %s, dtcpIpKeyFormat=%s Xcode %s, inactivityTimeoutInMs=%u, maxDataRate=%u" BIP_MSG_PRE_ARG,
                BIP_String_GetString( pAppCtx->hPort ),
                BIP_String_GetString( pAppCtx->hInterfaceName ),
                pAppCtx->enableDtcpIp ? "Y":"N",
                BIP_String_GetString( pAppCtx->hDtcpIpAkePort ),
                BIP_String_GetString( pAppCtx->hDtcpIpKeyFormat ),
                pAppCtx->enableXcode ? pAppCtx->xcodeProfile : "N",
                pAppCtx->inactivityTimeoutInMs,
                pAppCtx->maxDataRate
             ));
    return ( bipStatus );
} /* parseOptions */

void unInitAppCtx(
    AppCtx *pAppCtx
    )
{
    if (!pAppCtx) return;
    if (pAppCtx->hPort) BIP_String_Destroy( pAppCtx->hPort);
    if (pAppCtx->hDtcpIpAkePort) BIP_String_Destroy( pAppCtx->hDtcpIpAkePort);
    if (pAppCtx->hInterfaceName) BIP_String_Destroy( pAppCtx->hInterfaceName);
    if (pAppCtx->hDtcpIpKeyFormat) BIP_String_Destroy( pAppCtx->hDtcpIpKeyFormat);
    if (pAppCtx->hHdcp1xBinFileName) BIP_String_Destroy( pAppCtx->hHdcp1xBinFileName);
    if (pAppCtx->hHdcp2xBinFileName) BIP_String_Destroy( pAppCtx->hHdcp2xBinFileName);
    if (pAppCtx) B_Os_Free( pAppCtx );
} /* unInitAppCtx */

AppCtx *initAppCtx( void )
{
    AppCtx *pAppCtx = NULL;
    BIP_Status bipStatus = BIP_ERR_INTERNAL;

    pAppCtx = B_Os_Calloc( 1, sizeof( AppCtx ));
    BIP_CHECK_GOTO(( pAppCtx ), ( "Memory Allocation Failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    /* Setup default values for global App settings */
    pAppCtx->hPort = BIP_String_CreateFromChar( HTTP_SERVER_PORT_STRING );
    BIP_CHECK_GOTO( (pAppCtx->hPort), ("BIP_String_CreateFromChar() Failed"), error, BIP_ERR_CREATE_FAILED, bipStatus);

    /* Setup default values for global App settings */
    pAppCtx->hDtcpIpAkePort = BIP_String_CreateFromChar( HTTP_SERVER_DTCP_IP_AKE_PORT_STRING );
    BIP_CHECK_GOTO( (pAppCtx->hDtcpIpAkePort), ("BIP_String_CreateFromChar() Failed"), error, BIP_ERR_CREATE_FAILED, bipStatus);

    pAppCtx->hInterfaceName = BIP_String_Create();
    BIP_CHECK_GOTO( (pAppCtx->hInterfaceName), ("BIP_String_Create() Failed"), error, BIP_ERR_CREATE_FAILED, bipStatus);

    pAppCtx->hDtcpIpKeyFormat = BIP_String_CreateFromChar(DTCP_IP_KEY_FORMAT_COMMON_DRM);
    BIP_CHECK_GOTO( (pAppCtx->hDtcpIpKeyFormat), ("BIP_String_Create() Failed"), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus);

    pAppCtx->hHdcp1xBinFileName = BIP_String_CreateFromChar(HDCP1X_KEYS_BIN_FILE);
    BIP_CHECK_GOTO( (pAppCtx->hHdcp1xBinFileName), ("BIP_String_Create() Failed"), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus);

    pAppCtx->hHdcp2xBinFileName = BIP_String_CreateFromChar(HDCP2X_KEYS_BIN_FILE);
    BIP_CHECK_GOTO( (pAppCtx->hHdcp2xBinFileName), ("BIP_String_Create() Failed"), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus);

error:
    return pAppCtx;
} /* initAppCtx */

int main(
    int    argc,
    char *argv[]
    )
{
    BIP_Status bipStatus;
    NEXUS_Error nrc;
    AppCtx *pAppCtx = NULL;

    /* Initialize BIP */
    bipStatus = BIP_Init(NULL);
    BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_Init Failed" ), errorBipInit, bipStatus, bipStatus );

    /* Allocate & initialize App Ctx */
    pAppCtx = initAppCtx();
    BIP_CHECK_GOTO(( pAppCtx ), ( "initAppCtx Failed" ), errorBipInit, BIP_ERR_CREATE_FAILED, bipStatus );

    /* Parse commandline options */
    bipStatus = parseOptions( argc, argv, pAppCtx );
    BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "parseOptions Failed" ), errorParseOptions, bipStatus, bipStatus );

    BDBG_MSG((BIP_MSG_PRE_FMT"http_file_streamer started ------------------->"BIP_MSG_PRE_ARG));

    /* Initialize NEXUS */
#if NXCLIENT_SUPPORT
    {
        NxClient_JoinSettings joinSettings;

        NxClient_GetDefaultJoinSettings(&joinSettings);
        snprintf(joinSettings.name, NXCLIENT_MAX_NAME, "%s", argv[0]);
        nrc = NxClient_Join(&joinSettings);
        BIP_CHECK_GOTO(( nrc == NEXUS_SUCCESS ), ( "NxClient_Join Failed" ), error, BIP_ERR_INTERNAL, bipStatus );
    }
#else /* !NXCLIENT_SUPPORT */
    {
        NEXUS_PlatformSettings platformSettings;

        if (pAppCtx->enableSlaveMode)
        {
            nrc = NEXUS_Platform_AuthenticatedJoin(NULL);
            BIP_CHECK_GOTO(( nrc == NEXUS_SUCCESS ), ( "NEXUS_Platform_Join Failed" ), error, BIP_ERR_INTERNAL, bipStatus );
        }
        else
        {
            NEXUS_PlatformStartServerSettings serverSettings;
            NEXUS_PlatformConfiguration platformConfig;

            NEXUS_Platform_GetDefaultSettings(&platformSettings);
            platformSettings.mode = NEXUS_ClientMode_eVerified;
            /*
             * Due to latest SAGE restrictions EXPORT_HEAP needs to be initialized even if we are not using SVP/EXPORT_HEAP(XRR).
             * It could be any small size heap. Configure export heap since it's not allocated by nexus by default.
             */
            platformSettings.heap[NEXUS_EXPORT_HEAP].size = 16*1024*1024;
            nrc = NEXUS_Platform_Init(&platformSettings);
            BIP_CHECK_GOTO(( nrc == NEXUS_SUCCESS ), ( "NEXUS_Platform_Init Failed" ), error, BIP_ERR_INTERNAL, bipStatus );

            NEXUS_Platform_GetConfiguration(&platformConfig);

            NEXUS_Platform_GetDefaultStartServerSettings(&serverSettings);
            serverSettings.allowUnauthenticatedClients = true; /* client is written this way */
            serverSettings.unauthenticatedConfiguration.mode = NEXUS_ClientMode_eVerified;
            serverSettings.unauthenticatedConfiguration.heap[1] = platformConfig.heap[0]; /* for purposes of example, allow access to main heap */
            nrc = NEXUS_Platform_StartServer(&serverSettings);
            BIP_CHECK_GOTO(( nrc == NEXUS_SUCCESS ), ( "NEXUS_Platform_StartServer Failed" ), error, BIP_ERR_INTERNAL, bipStatus );
        }
    }
#endif /* NXCLIENT_SUPPORT */

    /* Initialize HttpServer */
    bipStatus = initHttpServer( pAppCtx );
    BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "initHttpServer Failed" ), error, bipStatus, bipStatus );

    /*
     * At this time, BIP has started listening for incoming requests from clients.
     * When new request comes from client, app gets the callback from BIP.
     * Callback function sends event to allow this main thread to process the events.
     */

    /* Wait on Event group and process events as they come */
    BDBG_LOG(( BIP_MSG_PRE_FMT
                " Type 'q' followed by ENTER to exit gracefully\n"
                " Type 's' followed by ENTER to print stats\n"
                BIP_MSG_PRE_ARG ));
    while ( true )
    {
        unsigned      userInput;
        unsigned      i;
        unsigned      numTriggeredEvents;
        B_EventHandle triggeredEvents[MAX_TRACKED_EVENTS];

        if ( (userInput = readInput()) == 'q' ) break;

        B_EventGroup_Wait( pAppCtx->hHttpEventGroup, 1000/*1sec*/, triggeredEvents, pAppCtx->maxTriggeredEvents, &numTriggeredEvents );
        for (i = 0; i < numTriggeredEvents; i++)
        {
            if (triggeredEvents[i] == pAppCtx->hHttpRequestRcvdEvent)
            {
                BDBG_MSG(( BIP_MSG_PRE_FMT " Process HttpRequestEvent[i = %d]" BIP_MSG_PRE_ARG, i));
                /* process all events on the HttpServer */
                processHttpRequestEvent( pAppCtx );
                /* Continue to process next event */
                continue;
            }
            if (triggeredEvents[i] == pAppCtx->hHttpStreamerEvent)
            {
                BDBG_MSG(( BIP_MSG_PRE_FMT " Process events on all HttpStreamers" BIP_MSG_PRE_ARG ));
                processHttpStreamerEvent( pAppCtx );
                continue;
            }
            BDBG_WRN(( BIP_MSG_PRE_FMT " didn't process event (i %d)" BIP_MSG_PRE_ARG, i ));
        }
        if ( userInput == 's' || pAppCtx->printStatus )
        {
            AppStreamerCtx *pAppStreamerCtx;
            for ( pAppStreamerCtx = BLST_Q_FIRST(&pAppCtx->streamerListHead);
                  pAppStreamerCtx;
                  pAppStreamerCtx = BLST_Q_NEXT( pAppStreamerCtx, streamerListNext ) )
            {
                BIP_HttpStreamer_PrintStatus( pAppStreamerCtx->hHttpStreamer );
            }
        }
    }

    BDBG_LOG(( BIP_MSG_PRE_FMT " Shutting HttpServer down..." BIP_MSG_PRE_ARG ));

error:
    unInitHttpServer( pAppCtx );
#if NXCLIENT_SUPPORT
    NxClient_Uninit();
#else /* !NXCLIENT_SUPPORT */
    if (!pAppCtx->enableSlaveMode)
        NEXUS_Platform_StopServer();
    NEXUS_Platform_Uninit();
#endif /* NXCLIENT_SUPPORT */
errorParseOptions:
    unInitAppCtx( pAppCtx );
errorBipInit:
    BIP_Uninit();

    BDBG_LOG(( "All done!" ));
    return (0); /* main */
}
#else
int main()
{
    BDBG_ERR((BIP_MSG_PRE_FMT"http_hdmi_input_streamer is only supported on platforms w/ NEXUS_HAS_HDMI_INPUT is defined!" BIP_MSG_PRE_ARG));
}
#endif
