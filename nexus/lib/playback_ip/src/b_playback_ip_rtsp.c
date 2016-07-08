/***************************************************************************
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
*
* Description: This file implements the RTSP Wrapper to the Live Media RTSP library.
*
***************************************************************************/
#if defined(LINUX) || defined(__vxworks)

#include "b_playback_ip_lib.h"
#include "b_playback_ip_priv.h"
#include "b_playback_ip_utils.h"
#include "b_playback_ip_lm_helper.h"
#include <sys/ioctl.h>
#include <net/if.h>

BDBG_MODULE(b_playback_ip_rtsp);

extern B_PlaybackIpError B_PlaybackIp_RtpSessionOpen(B_PlaybackIpHandle playback_ip, B_PlaybackIpSessionOpenSettings *openSettings, B_PlaybackIpSessionOpenStatus *openStatus);
extern void B_PlaybackIp_RtpSessionClose(B_PlaybackIpHandle playback_ip);
extern B_PlaybackIpError B_PlaybackIp_RtpSessionSetup( B_PlaybackIpHandle playback_ip, B_PlaybackIpSessionSetupSettings *setupSettings, B_PlaybackIpSessionSetupStatus *setupStatus);
extern B_PlaybackIpError B_PlaybackIp_RtpSessionStart(B_PlaybackIpHandle playback_ip, B_PlaybackIpSessionStartSettings *startSettings, B_PlaybackIpSessionStartStatus *startStatus);
extern B_PlaybackIpError B_PlaybackIp_UdpSessionStart(B_PlaybackIpHandle playback_ip, B_PlaybackIpSessionStartSettings *startSettings, B_PlaybackIpSessionStartStatus *startStatus);
extern void B_PlaybackIp_UdpSessionStop(B_PlaybackIpHandle playback_ip);
extern void B_PlaybackIp_UdpSessionClose(B_PlaybackIpHandle playback_ip);
extern void B_PlaybackIp_RtpSessionStop(B_PlaybackIpHandle playback_ip);

#ifdef LIVEMEDIA_SUPPORT
static void
rtsp_cmd_completion(void *appCtx, B_PlaybackIpEventIds eventId)
{
    B_PlaybackIpHandle playback_ip = (B_PlaybackIpHandle)appCtx;

    BDBG_ASSERT(playback_ip);
    BKNI_SetEvent(playback_ip->liveMediaSyncEvent);
    BDBG_MSG(("%s: sent LiveMediaSyncEvent for eventId %d\n", __FUNCTION__, eventId));
}
#endif /* LIVEMEDIA_SUPPORT=y */

#define LIVEMEDIA_LIB_API_WAIT_TIME 2000

void
B_PlaybackIp_RtspSessionClose(
    B_PlaybackIpHandle playback_ip
    )
{
#ifdef LIVEMEDIA_SUPPORT
    if (playback_ip->mediaTransportProtocol == B_PlaybackIpProtocol_eUdp)
        B_PlaybackIp_UdpSessionClose(playback_ip);
    else
        B_PlaybackIp_RtpSessionClose(playback_ip);
    if (playback_ip->trickModeThread) {
        /* destroy thread that was created during 1st non-blocking trickmode call and re-used for all subsequent trickmode calls */
        B_Thread_Destroy(playback_ip->trickModeThread);
        playback_ip->trickModeThread = NULL;
        BDBG_MSG(("%s: destroying temporary thread created during HTTP session setup", __FUNCTION__));
    }
    if (playback_ip->newTrickModeJobEvent) {
        BKNI_DestroyEvent(playback_ip->newTrickModeJobEvent);
        playback_ip->newTrickModeJobEvent = NULL;
    }
#else
    BSTD_UNUSED(playback_ip);
#endif /* LIVEMEDIA_SUPPORT=y */
}

/*
   Function does following:
   -sdf
   -sdf
*/
B_PlaybackIpError
B_PlaybackIp_RtspSessionOpen(
    B_PlaybackIpHandle playback_ip,
    B_PlaybackIpSessionOpenSettings *openSettings,
    B_PlaybackIpSessionOpenStatus *openStatus /* [out] */
    )
{
    B_PlaybackIpError errorCode = B_ERROR_PROTO;
#ifdef LIVEMEDIA_SUPPORT
    B_PlaybackIpSocketState *socketState;

    if (!playback_ip || !openSettings || !openStatus) {
        BDBG_ERR(("%s: invalid params, playback_ip %p, openSettings %p, openStatus %p\n", __FUNCTION__, (void *)playback_ip, (void *)openSettings, (void *)openStatus));
        return B_ERROR_INVALID_PARAMETER;
    }

    /* if SessionSetup is in progress, return INCOMPLETE */
    if (playback_ip->apiInProgress) {
        return B_ERROR_IN_PROGRESS;
    }

    /* if SessionSetup is completed, return results to app */
    if (playback_ip->apiCompleted) {
        BDBG_WRN(("%s: previously started session open operation completed, playback_ip %p\n", __FUNCTION__, (void *)playback_ip));
        goto done;
    }

    /* Neither SessionSetup is in progress nor it is completed, verify input params and then start work */
    if (openSettings->u.rtsp.additionalHeaders && !strstr(openSettings->u.rtsp.additionalHeaders, "\r\n")) {
        BDBG_ERR(("%s: additional RTSP header is NOT properly terminated (missing \\r\\n), header is %s\n", __FUNCTION__, openSettings->u.rtsp.additionalHeaders));
        return B_ERROR_INVALID_PARAMETER;
    }

    /* input parameters are good, so update the api progress state */
    playback_ip->apiInProgress = true;
    playback_ip->apiCompleted = false;
    socketState = &openStatus->socketState;

    if (!openSettings->nonBlockingMode) {
        /* App wants to do a blocking call, so we will need to define our internal callback to wait for LiveMedia Session Completion */
        /* All RTSP related Live Media calls are non-blocking */
        playback_ip->openSettings.eventCallback = rtsp_cmd_completion;
        playback_ip->openSettings.appCtx = playback_ip;
    }

    /* We pass IP Channel info (IP, Port, RTSP URL) to the Live Media library */
    /* Library is responsible for setting up the initial socket information  & returns the socket fd back */
    BKNI_ResetEvent(playback_ip->liveMediaSyncEvent);
    errorCode = B_PlaybackIp_liveMediaSessionOpen(playback_ip, &playback_ip->openSettings, &playback_ip->lm_context, &socketState->fd);
    if (errorCode == B_ERROR_IN_PROGRESS) {
        BERR_Code rc;
        if (openSettings->nonBlockingMode)
            return B_ERROR_IN_PROGRESS;

        /* app is in the blocking mode, so wait for command completion */
        BDBG_MSG(("%s: waiting for for Live Media Session Open Command Completion event", __FUNCTION__));
        rc = BKNI_WaitForEvent(playback_ip->liveMediaSyncEvent, LIVEMEDIA_LIB_API_WAIT_TIME  /* msec */);
        if (rc == BERR_TIMEOUT) {
            BDBG_WRN(("%s: timed out for Live Media Session Open Command Completion event", __FUNCTION__));
            errorCode = B_ERROR_UNKNOWN;
            goto error;
        } else if (rc!=0) {
            BDBG_WRN(("%s: Got error while waiting for Live Media Session Open Command Completion event", __FUNCTION__));
            errorCode = B_ERROR_UNKNOWN;
            goto error;
        }

        /* session open completed successfully, continue below at done label */
        BDBG_MSG(("%s: Session Open Command completed: socket %d", __FUNCTION__, socketState->fd));
    }
    else if (errorCode != B_ERROR_SUCCESS) {
        goto error;
    }

done:
    /* RTSP command completed, verify response */
    openStatus->u.rtsp = playback_ip->openStatus.u.rtsp;    /* filled in by the liveMediaSessionOpen call */
    if (playback_ip->openStatus.u.rtsp.statusCode != 200) {
        BDBG_ERR(("%s: Live Media Session Open Failed ", __FUNCTION__));
        goto error;
    }
    BDBG_MSG(("%s: successfully received the RTSP Response", __FUNCTION__));
    playback_ip->playback_state = B_PlaybackIpState_eOpened;

    /* Since we dont yet know the media transport protocol, as assume it to be RTP and do the RTP session setup */
    /* RTP session setup is the super set of both RTP & UDP cases, so we should be fine */
    errorCode = B_PlaybackIp_RtpSessionOpen(playback_ip, openSettings, openStatus);

error:
    playback_ip->apiInProgress = false;
    playback_ip->apiCompleted = false;
    if (errorCode != B_ERROR_SUCCESS) {
        B_PlaybackIp_RtpSessionClose(playback_ip);
    }
#else
    BSTD_UNUSED(playback_ip);
    BSTD_UNUSED(openSettings);
    BSTD_UNUSED(openStatus);
#endif /* LIVEMEDIA_SUPPORT=y */
    return errorCode;
}

#ifdef LIVEMEDIA_SUPPORT
static B_PlaybackIpError
B_PlaybackIp_RtspLiveMediaSessionStart(B_PlaybackIpHandle playback_ip, bool nonBlockingMode)
{
    B_PlaybackIpError errorCode = B_ERROR_PROTO;

    BKNI_ResetEvent(playback_ip->liveMediaSyncEvent);
    errorCode = B_PlaybackIp_liveMediaSessionStart(playback_ip->lm_context);
    if (errorCode == B_ERROR_IN_PROGRESS) {
        BERR_Code rc;
        if (nonBlockingMode)
            goto out;

        /* app is in the blocking mode, so wait for command completion */
        rc = BKNI_WaitForEvent(playback_ip->liveMediaSyncEvent, LIVEMEDIA_LIB_API_WAIT_TIME /* msec */);
        if (rc == BERR_TIMEOUT) {
            BDBG_WRN(("%s: timed out for Live Media Session Start Command Completion event", __FUNCTION__));
            errorCode = B_ERROR_UNKNOWN;
        } else if (rc!=0) {
            BDBG_WRN(("%s: Got error while waiting for Live Media Session Start Command Completion event", __FUNCTION__));
            errorCode = B_ERROR_UNKNOWN;
        }
        else {
            /* session Start successfully completed */
            errorCode = B_ERROR_SUCCESS;
        }
    }

out:
    BDBG_MSG(("%s: errorCode %d", __FUNCTION__, errorCode));
    return errorCode;
}
#endif /* LIVEMEDIA_SUPPORT=y */

B_PlaybackIpError
B_PlaybackIp_RtspSessionSetup(
    B_PlaybackIpHandle playback_ip,
    B_PlaybackIpSessionSetupSettings *setupSettings,
    B_PlaybackIpSessionSetupStatus *setupStatus /* [out] */
    )
{
    B_PlaybackIpError errorCode = B_ERROR_PROTO;
#ifdef LIVEMEDIA_SUPPORT

    if (!playback_ip || !setupSettings || !setupStatus) {
        BDBG_ERR(("%s: invalid params, playback_ip %p, setupSettings %p, setupStatus %p\n", __FUNCTION__, (void *)playback_ip, (void *)setupSettings, (void *)setupStatus));
        return B_ERROR_INVALID_PARAMETER;
    }

    if (setupSettings->u.rtsp.additionalHeaders && !strstr(setupSettings->u.rtsp.additionalHeaders, "\r\n")) {
        BDBG_ERR(("%s: additional RTSP header is NOT properly terminated (missing \\r\\n), header is: %s\n", __FUNCTION__, setupSettings->u.rtsp.additionalHeaders));
        errorCode = B_ERROR_INVALID_PARAMETER;
        goto error;
    }


    /* if SessionSetup is in progress, return INCOMPLETE */
    if (playback_ip->apiInProgress)
        return B_ERROR_IN_PROGRESS;

    /* if SessionSetup is completed, return results to app */
    if (playback_ip->apiCompleted) {
        BDBG_WRN(("%s: previously started session setup operation completed, playback_ip %p\n", __FUNCTION__, (void *)playback_ip));
        /* Note: since this api was run in a separate thread, we defer thread cleanup until the Ip_Start */
        /* as this call to read up the session status may be invoked in the context of this thread via the callback */
        goto done;
    }

    /* Neither SessionSetup is in progress nor it is completed, so start setup */
    playback_ip->apiInProgress = true;
    playback_ip->apiCompleted = false;
    memset(&playback_ip->setupStatus, 0, sizeof(playback_ip->setupStatus));

#ifdef TEST_NW_TIMEOUT
    BDBG_WRN((" ***** pull cable, sleeping for 5 sec ****"));
    sleep(5);
    BDBG_WRN((" ***** after sleeping for 5 sec ****"));
#endif

    BKNI_ResetEvent(playback_ip->liveMediaSyncEvent);
    errorCode = B_PlaybackIp_liveMediaSessionSetup(playback_ip->lm_context);
    if (errorCode == B_ERROR_IN_PROGRESS) {
        BERR_Code rc;
        if (playback_ip->openSettings.nonBlockingMode)
            return B_ERROR_IN_PROGRESS;

        /* app is in the blocking mode, so wait for command completion */
        rc = BKNI_WaitForEvent(playback_ip->liveMediaSyncEvent, LIVEMEDIA_LIB_API_WAIT_TIME /* msec */);
        if (rc == BERR_TIMEOUT) {
            BDBG_WRN(("%s: timed out for Live Media Session Setup Command Completion event", __FUNCTION__));
            errorCode = B_ERROR_UNKNOWN;
            goto error;
        } else if (rc!=0) {
            BDBG_WRN(("%s: Got error while waiting for Live Media Session Setup Command Completion event", __FUNCTION__));
            errorCode = B_ERROR_UNKNOWN;
            goto error;
        }

        /* session setup completed */
        BDBG_MSG(("%s: Live Media Session Setup Command Response\n %s", __FUNCTION__, playback_ip->setupStatus.u.rtsp.responseHeaders));
    }
    else if (errorCode != B_ERROR_SUCCESS) {
        goto error;
    }

done:
    /* LiveMedia (LM) layer is setup, now start it so that we do receive data to do media probe. */
    if (!playback_ip->rtspLmStartDone && !playback_ip->rtspPsiSetupDone)
    {
        /* Check the status of the LM Setup. */
        setupStatus->u.rtsp = playback_ip->setupStatus.u.rtsp;
        if (setupStatus->u.rtsp.responseHeaders != NULL) {
            /* success */
            errorCode = B_ERROR_SUCCESS;
            /* get the optional scale list if provided by server */
            B_PlaybackIp_liveMediaGetScaleList(playback_ip->lm_context, &setupStatus->u.rtsp.scaleListEntries, &setupStatus->u.rtsp.scaleList[0]);
        }
        else {
            errorCode = B_ERROR_PROTO;
            BDBG_ERR(("%s: Live Media Session Setup Failed ", __FUNCTION__));
            goto error;
        }

        /* We are done if caller doesn't want us to probe the media. */
        if (setupSettings->u.rtsp.skipPsiParsing) {
            BDBG_WRN(("%s: App asked us to skip PSI parsing...", __FUNCTION__));
            goto out;
        }

        /* Now continue w/ phase2 of API where we will first start the lm layer. */
        /* Since caller wants us to get the Media Info, we need to internally Start so that server starts sending AV stream. */
        /* Also, note that calls to LmSession layers are all non-blocking. In the case, where app is in the blocking mode */
        /* we set an internal callback here that lmSession layer invokes. Since we want to catch this lmSessionStart completion */
        /* we override the user eventCallback to ours and restore it later */

        /* Save the user callback if we are not in non-blocking mode as in non-blocking mode, Lm layer will invoke the user callback. */
        if (!playback_ip->openSettings.nonBlockingMode) {
            playback_ip->savedUserCallback = playback_ip->openSettings.eventCallback;
            playback_ip->savedUserContext = playback_ip->openSettings.appCtx;
            playback_ip->openSettings.eventCallback = rtsp_cmd_completion;
            playback_ip->openSettings.appCtx = playback_ip;
        }
        else {
            /* non-Blocking API, so again set the flags to indicate it. */
            playback_ip->apiInProgress = true;
            playback_ip->apiCompleted = false;
        }


        BDBG_WRN(("%s: App asked us to do PSI parsing, so internaly starting LM layer", __FUNCTION__));
        /* For us to get the PSI info, we will need to quickly start the session, so that server can start streaming */
        errorCode = B_PlaybackIp_RtspLiveMediaSessionStart(playback_ip, playback_ip->openSettings.nonBlockingMode);
        playback_ip->rtspLmStartDone = true;
        if (errorCode == B_ERROR_IN_PROGRESS)
            return errorCode;
        else if (errorCode != B_ERROR_SUCCESS)
            goto error;
    }
    if (playback_ip->rtspLmStartDone && !playback_ip->rtspPsiSetupDone)
    {
        /* Now continue w/ phase3 of API where we will setup for probing the media. */
        playback_ip->apiInProgress = false;
        playback_ip->apiCompleted = false;

        /* Now RtspLiveMediaSessionStart() is complete, so start the media probe process. This is done inline. */
        BDBG_WRN(("%s: LM layer started, now starting the probe", __FUNCTION__));
#if 0
        B_PlaybackIp_UtilsMediaProbeCreate(playback_ip);
#endif
        errorCode = B_PlaybackIp_RtpSessionSetup(playback_ip, setupSettings, setupStatus);
        playback_ip->rtspPsiSetupDone = true;
        if (errorCode == B_ERROR_IN_PROGRESS)
            return errorCode;
    }

    /* Probe is done, so stop the LmSession. */
    BDBG_MSG(("Probe is done, check results & complete API. "));
    B_PlaybackIp_liveMediaSessionStop(playback_ip);

    /* restore the user context if needed. */
    if (!playback_ip->openSettings.nonBlockingMode) {
        playback_ip->openSettings.eventCallback =   playback_ip->savedUserCallback;
        playback_ip->openSettings.appCtx =   playback_ip->savedUserContext;
    }

    /* check the success of probing function */
    if (!playback_ip->psi.psiValid) {
        BDBG_ERR(("%s: Failed to acquire PSI info via media probe\n", __FUNCTION__));
        errorCode = B_ERROR_UNKNOWN;
        B_PlaybackIp_UtilsMediaProbeDestroy(playback_ip);
        goto error;
    }
    setupStatus->u.rtsp.psi = playback_ip->psi;
    setupStatus->u.rtsp.stream = (void *)playback_ip->stream;
out:
    errorCode = B_ERROR_SUCCESS;

error:
    if (errorCode != B_ERROR_SUCCESS)
    {
        BDBG_WRN(("%s: Failed", __FUNCTION__));
        B_PlaybackIp_RtpSessionClose(playback_ip);
    }
    playback_ip->apiInProgress = false;
    playback_ip->apiCompleted = false;
#else
    BSTD_UNUSED(playback_ip);
    BSTD_UNUSED(setupSettings);
    BSTD_UNUSED(setupStatus);
#endif /* LIVEMEDIA_SUPPORT=y */
    return errorCode;
}

void
B_PlaybackIp_RtspSessionStop(
    B_PlaybackIpHandle playback_ip
    )
{
#ifdef LIVEMEDIA_SUPPORT
    if (playback_ip->mediaTransportProtocol == B_PlaybackIpProtocol_eUdp)
        B_PlaybackIp_UdpSessionStop(playback_ip);
    else
        B_PlaybackIp_RtpSessionStop(playback_ip);
#else
    BSTD_UNUSED(playback_ip);
#endif /* LIVEMEDIA_SUPPORT=y */
}

B_PlaybackIpError
B_PlaybackIp_RtspSessionStart(
    B_PlaybackIpHandle playback_ip,
    B_PlaybackIpSessionStartSettings *startSettings,
    B_PlaybackIpSessionStartStatus *startStatus /* [out] */
    )
{
    B_PlaybackIpError errorCode = B_ERROR_PROTO;

#ifdef LIVEMEDIA_SUPPORT
    if (!playback_ip || !startSettings || !startStatus) {
        BDBG_ERR(("%s: invalid params, playback_ip %p, startSettings %p, startStatus %p\n", __FUNCTION__, (void *)playback_ip, (void *)startSettings, (void *)startStatus));
        return B_ERROR_INVALID_PARAMETER;
    }

    /* if SessionStart is in progress, return INCOMPLETE */
    if (playback_ip->apiInProgress)
        return B_ERROR_IN_PROGRESS;

    /* if SessionStart is completed, return results to app */
    if (playback_ip->apiCompleted) {
        BDBG_WRN(("%s: previously started session start operation completed, playback_ip %p\n", __FUNCTION__, (void *)playback_ip));
        /* Note: since this api was run in a separate thread, we defer thread cleanup until the Ip_Start */
        /* as this call to read up the session status may be invoked in the context of this thread via the callback */
        goto done;
    }

    BDBG_MSG(("%s: RTSP Media Transport Protocol: %s, position start %d, end %d, keepAliveInterval %d",
                __FUNCTION__, startSettings->u.rtsp.mediaTransportProtocol == B_PlaybackIpProtocol_eUdp ? "UDP" : "RTP",
                (int)startSettings->u.rtsp.start,
                (int)startSettings->u.rtsp.end,
                startSettings->u.rtsp.keepAliveInterval
                ));
    /* Neither SessionStart is in progress nor it is completed, so start session */
    playback_ip->apiInProgress = true;
    playback_ip->apiCompleted = false;
    memset(&playback_ip->startStatus, 0, sizeof(playback_ip->startStatus));

    errorCode = B_PlaybackIp_RtspLiveMediaSessionStart(playback_ip, playback_ip->openSettings.nonBlockingMode);
    if (errorCode == B_ERROR_IN_PROGRESS)
        return errorCode;
    else if (errorCode != B_ERROR_SUCCESS)
        goto error;

done:
    BDBG_WRN(("%s: Live Media Session Start Command Response\n %s", __FUNCTION__, playback_ip->startStatus.u.rtsp.responseHeaders));
    startStatus->u.rtsp = playback_ip->startStatus.u.rtsp;
    if (startStatus->u.rtsp.responseHeaders == NULL) {
        errorCode = B_ERROR_PROTO;
        goto error;
    }

    /* Now setup the media transport protocol for RTSP: either UDP or RTP */
    if (playback_ip->mediaTransportProtocol == B_PlaybackIpProtocol_eUdp)
        errorCode = B_PlaybackIp_UdpSessionStart(playback_ip, startSettings, startStatus);
    else
        errorCode = B_PlaybackIp_RtpSessionStart(playback_ip, startSettings, startStatus);

error:
    playback_ip->apiInProgress = false;
    playback_ip->apiCompleted = false;
    if (errorCode != B_ERROR_SUCCESS) {
        B_PlaybackIp_liveMediaSessionStop(playback_ip);
    }
#else
    BSTD_UNUSED(playback_ip);
    BSTD_UNUSED(startSettings);
    BSTD_UNUSED(startStatus);
#endif /* LIVEMEDIA_SUPPORT=y */
    return errorCode;
}

#endif /* LINUX || VxWorks */
