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
 *****************************************************************************/

 /**
 * Note this file is currently only compiled for LINUX platforms. Even though it
 * has VxWorks & WinCE related hash defines, the code needs more porting
 * work for these OS platforms.
 * If the feature isn't compiled in, then the only functions in this .c file
 * should be the public stubs.
 **/
#if defined(LINUX) || defined(__vxworks)

#include "b_playback_ip_lib.h"
#include "b_playback_ip_priv.h"
#include "b_playback_ip_utils.h"
#include "b_playback_ip_lm_helper.h"
#include "b_playback_ip_psi.h"
#include <sys/ioctl.h>
#include <net/if.h>

BDBG_MODULE(b_playback_ip);

/**
* Playback IP module can receive IP encapsulated AV data from
* network using following 3 compile options:
*       -use normal sockets (true when B_HAS_NETACCEL &
*        B_HAS_PLAYPUMP_IP is not defined)
*       -use legacy IP Playpump (true when B_HAS_PLAYPUMP_IP is
*        defined), this i/f is being deprecated)
*       -use recommended accelerated sockets (which uses
*        Broadcom's Accelerated Sockets i/f) (true when
*        B_HAS_NETACCEL is defined & B_HAS_PLAYPUMP_IP is not
*        defined).
*/

/***************************************************************************
Summary:
Private handle for each Playback IP App Lib context
***************************************************************************/

/* forward declarations: */
#ifndef B_HAS_SMS_GATEWAY
/* When SMS Gateway is configured, IP Applib is used only for receiving Live IP traffic which is mainly using UDP/RTP traffic. So we dont need any of the HTTP module. */
extern B_PlaybackIpError B_PlaybackIp_HttpSessionOpen( B_PlaybackIpHandle playback_ip, B_PlaybackIpSessionOpenSettings *openSettings, B_PlaybackIpSessionOpenStatus *openStatus);
extern B_PlaybackIpError B_PlaybackIp_HttpSessionSetup(B_PlaybackIpHandle playback_ip, B_PlaybackIpSessionSetupSettings *setupSettings, B_PlaybackIpSessionSetupStatus *setupStatus);
extern B_PlaybackIpError B_PlaybackIp_HttpSessionStart(B_PlaybackIpHandle playback_ip, B_PlaybackIpSessionStartSettings *startSettings, B_PlaybackIpSessionStartStatus *startStatus);
extern void B_PlaybackIp_HttpSessionStop(B_PlaybackIpHandle playback_ip);
extern void B_PlaybackIp_HttpSessionClose(B_PlaybackIpHandle playback_ip);
B_PlaybackIpError B_PlaybackIp_HttpGetCurrentPlaybackPosition(B_PlaybackIpHandle playback_ip, NEXUS_PlaybackPosition *currentPosition);
#endif
extern B_PlaybackIpError B_PlaybackIp_UdpSessionOpen( B_PlaybackIpHandle playback_ip, B_PlaybackIpSessionOpenSettings *openSettings, B_PlaybackIpSessionOpenStatus *openStatus);
extern B_PlaybackIpError B_PlaybackIp_UdpSessionStart(B_PlaybackIpHandle playback_ip, B_PlaybackIpSessionStartSettings *startSettings, B_PlaybackIpSessionStartStatus *startStatus);
extern void B_PlaybackIp_UdpSessionClose(B_PlaybackIpHandle playback_ip);

extern B_PlaybackIpError B_PlaybackIp_RtpSessionOpen( B_PlaybackIpHandle playback_ip, B_PlaybackIpSessionOpenSettings *openSettings, B_PlaybackIpSessionOpenStatus *openStatus);
extern B_PlaybackIpError B_PlaybackIp_RtpSessionStart(B_PlaybackIpHandle playback_ip, B_PlaybackIpSessionStartSettings *startSettings, B_PlaybackIpSessionStartStatus *startStatus);
extern void B_PlaybackIp_RtpSessionStop(B_PlaybackIpHandle playback_ip);
extern void B_PlaybackIp_RtpSessionClose(B_PlaybackIpHandle playback_ip);

#ifdef LIVEMEDIA_SUPPORT
extern B_PlaybackIpError B_PlaybackIp_RtspSessionOpen( B_PlaybackIpHandle playback_ip, B_PlaybackIpSessionOpenSettings *openSettings, B_PlaybackIpSessionOpenStatus *openStatus);
extern B_PlaybackIpError B_PlaybackIp_RtspSessionStart(B_PlaybackIpHandle playback_ip, B_PlaybackIpSessionStartSettings *startSettings, B_PlaybackIpSessionStartStatus *startStatus);
extern B_PlaybackIpError B_PlaybackIp_RtspSessionSetup(B_PlaybackIpHandle playback_ip, B_PlaybackIpSessionSetupSettings *setupSettings, B_PlaybackIpSessionSetupStatus *setupStatus);
extern void B_PlaybackIp_RtspSessionStop(B_PlaybackIpHandle playback_ip);
#ifndef SPF_SUPPORT
extern void B_PlaybackIp_RtspSessionClose(B_PlaybackIpHandle playback_ip);
#endif /* SPF_SUPPORT */
#endif
extern B_PlaybackIpError B_PlaybackIp_UdpSessionSetup(B_PlaybackIpHandle playback_ip, B_PlaybackIpSessionSetupSettings *setupSettings, B_PlaybackIpSessionSetupStatus *setupStatus);
extern B_PlaybackIpError B_PlaybackIp_RtpSessionSetup(B_PlaybackIpHandle playback_ip, B_PlaybackIpSessionSetupSettings *setupSettings, B_PlaybackIpSessionSetupStatus *setupStatus);
extern void B_PlaybackIp_HlsStopAlternateRendition(B_PlaybackIpHandle playback_ip);
extern B_Error B_PlaybackIp_HlsStartAlternateRendition(B_PlaybackIpHandle playback_ip, B_PlaybackIpHlsAltAudioRenditionInfo *altAudioRenditionInfo);
extern bool B_PlaybackIp_HlsBoundedStream(B_PlaybackIpHandle playback_ip);

/* TODO: look into using a per IP session context mutext to prevent multiple threads invoking the same/different APIs for a given IP session context */

/***************************************************************************
Summary:
This function initializes an Playback IP channel on the settings selected. A shallow
copy of the B_PlaybackIp_Settings structure is made in this call. The private App Lib
structure is malloc'ed.
***************************************************************************/
B_PlaybackIpHandle B_PlaybackIp_Open(
    const B_PlaybackIpOpenSettings *pSettings
    )
{
    B_PlaybackIpHandle playback_ip = NULL;

    BDBG_MSG(("%s: PKTS_PER_READ %d\n", __FUNCTION__, PKTS_PER_READ));
    BSTD_UNUSED(pSettings);

    B_Os_Init();

    /* allocate playback_ip context */
    playback_ip = (B_PlaybackIpHandle) BKNI_Malloc(sizeof(B_PlaybackIp));
    if (NULL == playback_ip) {
        B_Os_Uninit();
        return NULL;
    }
    BKNI_Memset(playback_ip, 0, sizeof(B_PlaybackIp));

    /* move to open state */
    playback_ip->playback_state = B_PlaybackIpState_eOpened;

    return (B_PlaybackIpHandle) playback_ip;
}

/*
Summary:
    Close an IP playback channel.
    This function de-initializes the Playback IP App Lib. The private App Lib structure is freed.
*/
B_PlaybackIpError
B_PlaybackIp_Close(
    B_PlaybackIpHandle playback_ip
    )
{
    if (!playback_ip) {
        BDBG_ERR(("%s: NULL playback_ip handle\n", __FUNCTION__));
        return B_ERROR_INVALID_PARAMETER;
    }

    BDBG_MSG(("%s: ", __FUNCTION__));

    switch (playback_ip->playback_state) {
    case B_PlaybackIpState_eSessionSetupInProgress:
    case B_PlaybackIpState_eSessionSetup:
    case B_PlaybackIpState_eSessionStartInProgress:
    case B_PlaybackIpState_ePlaying:
    case B_PlaybackIpState_eStopping:
    case B_PlaybackIpState_eBuffering:
    case B_PlaybackIpState_ePaused:
    case B_PlaybackIpState_eEnteringTrickMode:
    case B_PlaybackIpState_eTrickMode:
        B_PlaybackIp_SessionStop(playback_ip);
        /* continue w/ session close cleanup */
    case B_PlaybackIpState_eSessionOpenInProgress:
    case B_PlaybackIpState_eSessionOpened:
    /* case B_PlaybackIpState_eStopped: */
        B_PlaybackIp_SessionClose(playback_ip);
        /* continue w/ playback ip channel cleanup */
    case B_PlaybackIpState_eOpened:
    default:
        break;
        /* do the playback channel related cleanup */
    }

    BKNI_Free(playback_ip);
    B_Os_Uninit();

    return BERR_SUCCESS;
}

B_PlaybackIpError
B_PlaybackIp_SessionOpen(
    B_PlaybackIpHandle playback_ip,
    B_PlaybackIpSessionOpenSettings *openSettings,
    B_PlaybackIpSessionOpenStatus *openStatus /* [out] */
    )
{
    B_PlaybackIpError errorCode = B_ERROR_PROTO;
    B_PlaybackIpSocketState *socketState;
    char *pValue = NULL;

    if (!playback_ip || !openSettings || !openStatus) {
        BDBG_ERR(("%s: invalid params, playback_ip %p, openSettings %p, openStatus %p\n", __FUNCTION__, (void *)playback_ip, (void *)openSettings, (void *)openStatus));
        return B_ERROR_INVALID_PARAMETER;
    }

    /* Validate if app can call this API in the current IP Playback State */
    switch (playback_ip->playback_state) {
    case B_PlaybackIpState_eOpened:
    case B_PlaybackIpState_eSessionOpenInProgress:
        /* continue below with this API */
        break;
    default:
        /* In all other states, app can't call this API */
        BDBG_ERR(("ERROR: Can't call %s() in this state %d\n", __FUNCTION__, playback_ip->playback_state));
        return B_ERROR_NOT_SUPPORTED;
    }

    if (openSettings->nonBlockingMode) {
        if (!openSettings->eventCallback) {
            BDBG_ERR(("%s: invalid param: eventCallback must be specified for nonBlockingMode operation", __FUNCTION__));
            return B_ERROR_INVALID_PARAMETER;
        }

        /* if the API is already in progress, return INCOMPLETE */
        if (playback_ip->apiInProgress) {
            return B_ERROR_IN_PROGRESS;
        }

        /* So Api is not in progress, check if it is completed, them jump to returning results to app */
        if (playback_ip->apiCompleted) {
            BDBG_MSG(("%s: previously started session open operation completed, playback_ip %p", __FUNCTION__, (void *)playback_ip));
            goto apiDone;
        }

        /* else fall below to perform session initialization */
    }

    /* reset playback ip context */
    memset(playback_ip, 0, sizeof(B_PlaybackIp));
    B_PlaybackIp_GetDefaultSettings(&playback_ip->settings);

    pValue = getenv("enableRecording");
    if (pValue)
        playback_ip->enableRecording = true;
    else
        playback_ip->enableRecording = false;

    pValue = getenv("ipVerboseLog");
    if (pValue)
        playback_ip->ipVerboseLog = true;
    else
        playback_ip->ipVerboseLog = false;

    /* now API is progress, update state to reflect that */
    playback_ip->playback_state = B_PlaybackIpState_eSessionOpenInProgress;

    BDBG_MSG(("%s: playback_ip %p, openSettings %p, openStatus %p, state %d, proto %d\n",
                __FUNCTION__, (void *)playback_ip, (void *)openSettings, (void *)openStatus, playback_ip->playback_state, openSettings->socketOpenSettings.protocol));

    playback_ip->openSettings = *openSettings;
    if (openSettings->networkTimeout) {
        playback_ip->settings.networkTimeout = openSettings->networkTimeout;
        playback_ip->networkTimeout = openSettings->networkTimeout;
    }
    else {
        playback_ip->networkTimeout = HTTP_SELECT_TIMEOUT;
    }

    if (openSettings->socketOpenSettings.url != NULL) {
        /* cache a copy of the URL */
        if ((playback_ip->openSettings.socketOpenSettings.url = B_PlaybackIp_UtilsStrdup(openSettings->socketOpenSettings.url)) == NULL) {
            BDBG_ERR(("%s: Failed to duplicate URL string due to out of memory condition", __FUNCTION__));
            goto error;
        }
    }
    socketState = &openStatus->socketState;
    memset(openStatus, 0, sizeof(B_PlaybackIpSessionOpenStatus));

    /* create events */
    if (BKNI_CreateEvent(&playback_ip->playback_halt_event)) {
        BDBG_ERR(("%s: Failed to create an event\n", __FUNCTION__));
        goto error;
    }
    if (BKNI_CreateEvent(&playback_ip->read_callback_event)) {
        BDBG_ERR(("%s: Failed to create an event\n", __FUNCTION__));
        goto error;
    }
    if (BKNI_CreateEvent(&playback_ip->preChargeBufferEvent)) {
        BDBG_ERR(("%s: Failed to create an event\n", __FUNCTION__));
        goto error;
    }
    if (BKNI_CreateEvent(&playback_ip->newJobEvent)) {
        BDBG_ERR(("%s: Failed to create an event\n", __FUNCTION__));
        goto error;
    }
    if (BKNI_CreateEvent(&playback_ip->liveMediaSyncEvent)) {
        BDBG_ERR(("%s: Failed to create an event\n", __FUNCTION__));
        goto error;
    }
    if (BKNI_CreateEvent(&playback_ip->sessionOpenRetryEventHandle)) {
        BDBG_ERR(("%s: Failed to create an event (Session Open Retry Event)\n", __FUNCTION__));
        goto error;
    }
    /* create mutex for serializing flow among different flows invoking a IP playback session */
    if (BKNI_CreateMutex(&playback_ip->lock) != 0) {
        BDBG_ERR(("Failed to create BKNI mutex at %d", __LINE__));
        errorCode = B_ERROR_OUT_OF_MEMORY;
        goto error;
    }


apiDone:
    switch (openSettings->socketOpenSettings.protocol) {
#ifndef B_HAS_SMS_GATEWAY
    case B_PlaybackIpProtocol_eHttp:
        errorCode = B_PlaybackIp_HttpSessionOpen(playback_ip, openSettings, openStatus);
        break;
#endif
    case B_PlaybackIpProtocol_eUdp:
    case B_PlaybackIpProtocol_eRtpFec:
        errorCode = B_PlaybackIp_UdpSessionOpen(playback_ip, openSettings, openStatus);
        break;
    case B_PlaybackIpProtocol_eRtsp:
#ifdef LIVEMEDIA_SUPPORT
        errorCode = B_PlaybackIp_RtspSessionOpen(playback_ip, openSettings, openStatus);
#else
        errorCode = B_ERROR_INVALID_PARAMETER;
        BDBG_ERR(("%s: RTSP protocol requires compilation of Live Media library (build w/ LIVEMEDIA_SUPPORT=y)", __FUNCTION__));
#endif
        break;

    case B_PlaybackIpProtocol_eRtp:
#ifdef LIVEMEDIA_SUPPORT
        errorCode = B_PlaybackIp_RtpSessionOpen(playback_ip, openSettings, openStatus);
#else
        errorCode = B_ERROR_INVALID_PARAMETER;
        BDBG_ERR(("%s: RTP protocol requires compilation of Live Media library (build w/ LIVEMEDIA_SUPPORT=y)", __FUNCTION__));
#endif
        break;
    case B_PlaybackIpProtocol_eRtpNoRtcp:
        errorCode = B_PlaybackIp_RtpSessionOpen(playback_ip, openSettings, openStatus);
        break;

    default:
        errorCode = B_ERROR_INVALID_PARAMETER;
        break;
    }
    playback_ip->protocol = openSettings->socketOpenSettings.protocol;
    if (errorCode == B_ERROR_IN_PROGRESS)
        return B_ERROR_IN_PROGRESS;
    else if (errorCode != B_ERROR_SUCCESS)
        goto error;

    /* now this API is successfully completed, update state to reflect that */
    playback_ip->playback_state = B_PlaybackIpState_eSessionOpened;
    playback_ip->socketState = openStatus->socketState;
    playback_ip->settings.ipMode = openSettings->ipMode;
    BDBG_MSG(("%s: Session Open completed: socket fd %d\n", __FUNCTION__, playback_ip->socketState.fd));
    return B_ERROR_SUCCESS;

error:
    if (playback_ip->openSettings.socketOpenSettings.url)
        BKNI_Free(playback_ip->openSettings.socketOpenSettings.url);
    if (playback_ip->playback_halt_event)
        BKNI_DestroyEvent(playback_ip->playback_halt_event);
    if (playback_ip->preChargeBufferEvent)
        BKNI_DestroyEvent(playback_ip->preChargeBufferEvent);
    if (playback_ip->newJobEvent)
        BKNI_DestroyEvent(playback_ip->newJobEvent);
    if (playback_ip->read_callback_event)
        BKNI_DestroyEvent(playback_ip->read_callback_event);
    if (playback_ip->liveMediaSyncEvent)
        BKNI_DestroyEvent(playback_ip->liveMediaSyncEvent);
    if (playback_ip->sessionOpenRetryEventHandle) {
        BKNI_DestroyEvent(playback_ip->sessionOpenRetryEventHandle);
        playback_ip->sessionOpenRetryEventHandle = NULL;
    }
    if (playback_ip->lock) {
        BKNI_DestroyMutex(playback_ip->lock);
        playback_ip->lock = NULL;
    }

    /* back to Opened state */
    playback_ip->playback_state = B_PlaybackIpState_eOpened;
    playback_ip->openSettings.eventCallback = NULL;
    BDBG_ERR(("%s() ERRRO: playback_ip %p, errorCode %d, fd %d\n", __FUNCTION__, (void *)playback_ip, errorCode, playback_ip->socketState.fd));
    return errorCode;
}

B_PlaybackIpError
B_PlaybackIp_SessionSetup(
    B_PlaybackIpHandle playback_ip,
    B_PlaybackIpSessionSetupSettings *setupSettings,
    B_PlaybackIpSessionSetupStatus *setupStatus /* [out] */
    )
{
    B_PlaybackIpError errorCode = B_ERROR_PROTO;

    BDBG_MSG(("%s: playback_ip %p, setupSettings %p, setupStatus %p\n", __FUNCTION__, (void *)playback_ip, (void *)setupSettings, (void *)setupStatus));
    if (!playback_ip || !setupSettings || !setupStatus) {
        BDBG_ERR(("%s: invalid params, playback_ip %p, setupSettings %p, setupStatus %p\n", __FUNCTION__, (void *)playback_ip, (void *)setupSettings, (void *)setupStatus));
        return B_ERROR_INVALID_PARAMETER;
    }

    /* Validate if app can call this API in the current IP Playback State */
    switch (playback_ip->playback_state) {
    case B_PlaybackIpState_eSessionOpened:
    case B_PlaybackIpState_eSessionSetupInProgress:
        /* continue below with this API */
        break;
    default:
        /* In all other states, app can't call this API */
        BDBG_ERR(("ERROR: Can't call %s() in this state %d\n", __FUNCTION__, playback_ip->playback_state));
        return B_ERROR_NOT_SUPPORTED;
    }

    /* now API is progress, update state to reflect that */
    playback_ip->playback_state = B_PlaybackIpState_eSessionSetupInProgress;

    memset(setupStatus, 0, sizeof(B_PlaybackIpSessionSetupStatus));
    playback_ip->setupSettings = *setupSettings;
    B_PlaybackIp_UtilsTuneNetworkStack(playback_ip->socketState.fd);

    switch (playback_ip->openSettings.socketOpenSettings.protocol) {
#ifndef B_HAS_SMS_GATEWAY
    case B_PlaybackIpProtocol_eHttp:
        errorCode = B_PlaybackIp_HttpSessionSetup(playback_ip, setupSettings, setupStatus);
        break;
#endif
#ifdef LIVEMEDIA_SUPPORT
    case B_PlaybackIpProtocol_eRtsp:
        errorCode = B_PlaybackIp_RtspSessionSetup(playback_ip, setupSettings, setupStatus);
        break;
#endif
    case B_PlaybackIpProtocol_eUdp:
        errorCode = B_PlaybackIp_UdpSessionSetup(playback_ip, setupSettings, setupStatus);
        break;
    case B_PlaybackIpProtocol_eRtp:
        errorCode = B_PlaybackIp_RtpSessionSetup(playback_ip, setupSettings, setupStatus);
        break;
    default:
        /* this function is NOP for rest of the protocols */
        errorCode = B_ERROR_SUCCESS;
        break;
    }

    if (errorCode == B_ERROR_IN_PROGRESS)
        return B_ERROR_IN_PROGRESS;
    else if (errorCode != B_ERROR_SUCCESS)
        goto error;

    /* now this API is successfully completed, update state to reflect that */
    playback_ip->playback_state = B_PlaybackIpState_eSessionSetup;
    playback_ip->sessionSetupCompleted = true;
    BDBG_MSG(("%s: Session Setup completed: socket fd %d\n", __FUNCTION__, playback_ip->socketState.fd));
    return B_ERROR_SUCCESS;

error:
    BDBG_ERR(("%s() ERRRO: playback_ip %p, errorCode %d, fd %d\n", __FUNCTION__, (void *)playback_ip, errorCode, playback_ip->socketState.fd));
    playback_ip->playback_state = B_PlaybackIpState_eSessionOpened;
    return errorCode;
}

extern B_PlaybackIpError http_get_current_pts( B_PlaybackIpHandle playback_ip, unsigned int *currentPts);
static void
ptsErrorCallback(void *context, int param)
{
    B_PlaybackIpHandle playback_ip = (B_PlaybackIpHandle)context;
    uint32_t currentPts = 0;
    uint32_t currentContigousStreamDuration = 0;
    uint32_t stc = 0;
    uint32_t nextPts = 0;
    BSTD_UNUSED(param);

    if (playback_ip == NULL) {
        BDBG_ERR(("%s: ERROR: playback_ip context became NULL", __FUNCTION__));
        goto out;
    }

    /* when Display Manager (DM) detects a PTS discontinuity, nexus updates the STC with the PTS of the new frame with the discontinuity */
    /* and triggers this callback. Here, we obtain the current STC and use that as the firstPts of the new segment */
    /* we also get the currentPts which still points to the last frame before the discontinuity. */
    /* (this is because it takes decoder more time to decode the i-frame after the discontinuity and also it takes */
    /* sometime for its PTS to pass) and thus be available to the DM and thus reflected in the pts value) */
    /* and use the current firstPts and this pts to determine the duration of this segment before the discontinuity */
    /* Also, we accumulate the durations of all contigous segments and maintain them in currentPosition */

#ifdef NEXUS_HAS_SIMPLE_DECODER
    if (playback_ip->nexusHandles.simpleStcChannel)
        NEXUS_SimpleStcChannel_GetStc(playback_ip->nexusHandles.simpleStcChannel, &stc);
    else
#endif
    if (playback_ip->nexusHandles.stcChannel) NEXUS_StcChannel_GetStc(playback_ip->nexusHandles.stcChannel, &stc);
    /* current STC will become new firstPts */
    nextPts = stc;

    /* get pts of the last frame before disontinuity */
    if ( (http_get_current_pts(playback_ip, &currentPts) != B_ERROR_SUCCESS) || currentPts == 0) {
        BDBG_MSG(("%s: DM either failed to return PTS or returned 0 as current PTS, use the lastUsedPts %x as the currentPts", __FUNCTION__, playback_ip->lastUsedPts));
        currentPts = playback_ip->lastUsedPts;
    }

    if (currentPts < playback_ip->firstPts) {
        /* shouldn't happen as currentPts returned by the DM corresponds to the last frame before the discontinuity */
        /* and this whole segment should have been contigous, so something is not as we expect */
        /* note: it is confusing from the decoder status comment, but this currentPts is not the PTS corresponding to the frame after the discontinuity */
        BDBG_ERR(("%s: ERROR: current PTS %x is smaller than firstPTS %x, nextPts %x, streamDurationUntilLastDiscontinuity %d msec", __FUNCTION__, currentPts, playback_ip->firstPts, nextPts, playback_ip->streamDurationUntilLastDiscontinuity));
        currentContigousStreamDuration = 0;
    }
    else if (currentPts == playback_ip->ptsDuringLastDiscontinuity) {
        /* current decoded pts still happens to be same as the one during previous discontinuity, ignore it */
        /* this can happen when we get back to back PTS discontinuities */
        BDBG_MSG(("%s: current PTS %x is same as ptsDuringLastDiscontinuity %x, nextPts %x, streamDurationUntilLastDiscontinuity %d msec", __FUNCTION__, currentPts, playback_ip->ptsDuringLastDiscontinuity, nextPts, playback_ip->streamDurationUntilLastDiscontinuity));
        currentContigousStreamDuration = 0;
    }
    else {
        /* currentPts is good, so calculate the duration of the contigous segment upto this point */
        currentContigousStreamDuration = (currentPts - playback_ip->firstPts) / 45;
    }
    playback_ip->streamDurationUntilLastDiscontinuity += currentContigousStreamDuration;
    BDBG_ERR(("%s: contingous segment's firstPts %x, lastPts %x, nextPts %x, total streamDurationUntilLastDiscontinuity %d msec", __FUNCTION__, playback_ip->firstPts, currentPts, nextPts, playback_ip->streamDurationUntilLastDiscontinuity));
    playback_ip->firstPtsBeforePrevDiscontinuity = playback_ip->firstPts;
    playback_ip->firstPts = nextPts; /* this nextPts becomes the first PTS of segment after discontinuity */
    /* dont reset lastUsedPts to nextPts, instead to the currentPts (which is the last pts before this discontinuity */
    /* this allows us to correctly calculate the position if app asks for it before we have a chance to display the next picture */
    playback_ip->lastUsedPts = currentPts;
    playback_ip->ptsDuringLastDiscontinuity = currentPts;
    playback_ip->getNewPtsAfterDiscontinuity = true;

out:
    if (playback_ip && playback_ip->appPtsError.callback && playback_ip->appPtsError.callback != ptsErrorCallback)
        playback_ip->appPtsError.callback(playback_ip->appPtsError.context, playback_ip->appPtsError.param);
    return;
}

B_PlaybackIpError
B_PlaybackIp_ResetVideoPtsCallback(
    B_PlaybackIpHandle playback_ip
    )
{
    B_PlaybackIpError errorCode = B_ERROR_PROTO;

    if (playback_ip->nexusHandles.videoDecoder) {
        NEXUS_VideoDecoderSettings videoDecoderSettings;
        NEXUS_VideoDecoder_GetSettings(playback_ip->nexusHandles.videoDecoder, &videoDecoderSettings);
        if (playback_ip->appFirstPts.callback)
            videoDecoderSettings.firstPts = playback_ip->appFirstPts;
        else
            videoDecoderSettings.firstPts.callback = NULL;
        if (NEXUS_VideoDecoder_SetSettings(playback_ip->nexusHandles.videoDecoder, &videoDecoderSettings) != NEXUS_SUCCESS) {
            BDBG_ERR(("%s: Failed to set the 1st pts callback for video decoder", __FUNCTION__));
            goto error;
        }
    }
    else if (playback_ip->nexusHandles.simpleVideoDecoder) {
        NEXUS_VideoDecoderSettings videoDecoderSettings;
        NEXUS_SimpleVideoDecoder_GetSettings(playback_ip->nexusHandles.simpleVideoDecoder, &videoDecoderSettings);
        if (playback_ip->appFirstPts.callback)
            videoDecoderSettings.firstPts = playback_ip->appFirstPts;
        else
            videoDecoderSettings.firstPts.callback = NULL;
        if (NEXUS_SimpleVideoDecoder_SetSettings(playback_ip->nexusHandles.simpleVideoDecoder, &videoDecoderSettings) != NEXUS_SUCCESS) {
            BDBG_ERR(("%s: Failed to set the 1st pts callback\n", __FUNCTION__));
            goto error;
        }
    }
    BDBG_MSG(("%s: ctx %p: reset video first pts callback!", __FUNCTION__, (void *)playback_ip));
    errorCode = B_ERROR_SUCCESS;
error:
    return errorCode;
}

B_PlaybackIpError
B_PlaybackIp_ResetAudioPtsCallback(
    B_PlaybackIpHandle playback_ip
    )
{
    B_PlaybackIpError errorCode = B_ERROR_PROTO;

    if (playback_ip->nexusHandles.primaryAudioDecoder) {
        NEXUS_AudioDecoderSettings audioDecoderSettings;
        NEXUS_AudioDecoder_GetSettings(playback_ip->nexusHandles.primaryAudioDecoder, &audioDecoderSettings);
        if (playback_ip->streamStatusAvailable.callback)
            audioDecoderSettings.streamStatusAvailable = playback_ip->streamStatusAvailable;
        else
            audioDecoderSettings.streamStatusAvailable.callback = NULL;
        if (NEXUS_AudioDecoder_SetSettings(playback_ip->nexusHandles.primaryAudioDecoder, &audioDecoderSettings) != NEXUS_SUCCESS) {
            BDBG_ERR(("%s: Failed to set the 1st pts callback for audio decoder", __FUNCTION__));
            goto error;
        }
    }
    else if (playback_ip->nexusHandles.simpleAudioDecoder) {
        NEXUS_SimpleAudioDecoderSettings  * pAudioDecoderSettings;
        NEXUS_Error                         errCode = NEXUS_SUCCESS;

        pAudioDecoderSettings = BKNI_Malloc(sizeof(* pAudioDecoderSettings));
         if (!pAudioDecoderSettings) {
             BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
             goto error;
         }
        NEXUS_SimpleAudioDecoder_GetSettings(playback_ip->nexusHandles.simpleAudioDecoder, pAudioDecoderSettings);
        if (playback_ip->streamStatusAvailable.callback)
            pAudioDecoderSettings->primary.streamStatusAvailable = playback_ip->streamStatusAvailable;
        else
            pAudioDecoderSettings->primary.streamStatusAvailable.callback = NULL;

        errCode = NEXUS_SimpleAudioDecoder_SetSettings(playback_ip->nexusHandles.simpleAudioDecoder, pAudioDecoderSettings);
        BKNI_Free(pAudioDecoderSettings);
        if (errCode != NEXUS_SUCCESS) {
            BDBG_ERR(("%s: Failed to unset the 1st pts callback for simple audio decoder", __FUNCTION__));
            goto error;
        }
    }
    BDBG_MSG(("%s: ctx %p: reset audio first pts callback!", __FUNCTION__, (void *)playback_ip));
    errorCode = B_ERROR_SUCCESS;
error:
    return errorCode;
}

/*
   This callback is invoked when 1st frame is decoded:
   - after the initial start of stream playback
   - after seek operation
   - after trickmode operation
   - after resuming from trickmode operation
   It sets the firstPts when this stream is being played for the very 1st time.
   In other cases, it tries to determine the stream discontinuity after seek or trickplay operation
   and accordingly update the firstPts.
*/
static void
firstPtsCallback(void *context, int param)
{
    uint32_t currentPts;
    B_PlaybackIpHandle playback_ip = (B_PlaybackIpHandle)context;

    BSTD_UNUSED(param);
    BDBG_ASSERT(playback_ip);

    /* get pts corresponding to the 1st frame */
    if ((http_get_current_pts(playback_ip, &currentPts) != B_ERROR_SUCCESS) || currentPts == 0) {
        BDBG_MSG(("%s: DM either failed to return PTS or returned 0 as current PTS, not setting the firstPts", __FUNCTION__));
        goto out;
    }

    /* reset the mediaStartTime as this callback can also come after a seek or av decoder flush */
    playback_ip->mediaStartTimeNoted = false;
    if (!playback_ip->lastSeekPositionSet) {
#if 0
        /* in case of Boxee, where stream can have lots of artifacts, AV decoders cat get stuck. */
        /* stcChannel/pcrlib is flushing the decoder fifos and thus 1stPts interrupt is getting called again */
        /* we may need to not reset this value and instead add up the current time upto now into this */
        playback_ip->streamDurationUntilLastDiscontinuity = 0;
#else
        playback_ip->streamDurationUntilLastDiscontinuity += (playback_ip->lastUsedPts-playback_ip->firstPts)/45;
        BDBG_MSG(("%s: first pts 0x%x, last pts 0x%x, additional duration %d, total dur %d", __FUNCTION__,
                    playback_ip->firstPts, playback_ip->lastUsedPts, (playback_ip->lastUsedPts-playback_ip->firstPts)/45, playback_ip->streamDurationUntilLastDiscontinuity));
#endif
        /* if lastSeekPosition is not set, then we haven't yet done either pause/resume w/ disconnect/reconnect method, seek, or a trickplay */
        /* So this has to be the PTS corresponding to the 1st frame of this stream. We note this as firstPts for the current position calculations. */
        playback_ip->firstPts = currentPts;
        playback_ip->lastUsedPts = currentPts;
        BDBG_MSG(("%s: first decoded pts 0x%x", __FUNCTION__, currentPts));
    }
    else {
        /* lastSeekPosition is set, so we have done either pause/resume w/ disconnect/reconnect method, seek, or a trickplay */
        /* we compute the currentPositionUsingPts = currentPts - firstPts; and compare it to the lastSeekPosition */
        /* if currentPositionUsingPts is not way too off from the lastSeekPosition, we consider it more accurate */
        /* (as server may not be able to accurately seek to the lastSeekPosition) and thus dont change the 1st pts */
        /* Otherwise, there may have been PTS discontinuity in the content we skipped over due to trickplay and thus our 1st pts will need to be reset */
        NEXUS_PlaybackPosition currentPositionUsingPts;
        uint32_t positionDelta;
        int i;

        /* calculate the current position using pts */
        currentPositionUsingPts = (currentPts - playback_ip->firstPts)/45;

        if (playback_ip->streamDurationUntilLastDiscontinuity) {
            if (currentPts <= playback_ip->firstPts) {
                /* we had a PTS discontinuity & now new position is before this contigous block, so reset this duration */
                playback_ip->streamDurationUntilLastDiscontinuity = 0;
                /* pts will get reset below as positionDelta would be too big */
            }
            else {
                /* we had a discontinuity, but new position is after the firstPts, so account for this duration until last discontinuity */
                currentPositionUsingPts += playback_ip->streamDurationUntilLastDiscontinuity;
            }
        }

        /* now compare this pts computed position to the we had actually seeked or resumed from */
        i = currentPositionUsingPts - playback_ip->lastSeekPosition;
        positionDelta = abs(i);
#define MAX_POSITION_DELTA 10000
        if (positionDelta > MAX_POSITION_DELTA) {
            /* two positions are differ by over a max, so there must be PTS discontinuity. Reset 1st PTS. */
            playback_ip->streamDurationUntilLastDiscontinuity = playback_ip->lastSeekPosition;
            BDBG_MSG(("%s: PTS discontinuity: Current Position %lu after trickplay using PTS calculation is off from lastSeekedPosition %lu by over %d msec, max position delta %d, reset first pts from 0x%x to 0x%x, streamDurationUntilLastDiscontinuity %d msec",
                        __FUNCTION__, currentPositionUsingPts, playback_ip->lastSeekPosition, positionDelta, MAX_POSITION_DELTA, playback_ip->firstPts, currentPts, playback_ip->streamDurationUntilLastDiscontinuity));
            playback_ip->firstPts = currentPts;
            playback_ip->lastUsedPts = currentPts;
        }
        else {
            BDBG_MSG(("%s: Current Position %lu after trickplay using PTS calculation is more accurate than the one from lastSeekedPosition %lu, not resetting first pts from 0x%x to 0x%x, streamDurationUntilLastDiscontinuity %d msec",
                        __FUNCTION__, currentPositionUsingPts, playback_ip->lastSeekPosition, playback_ip->firstPts, currentPts, playback_ip->streamDurationUntilLastDiscontinuity));
            playback_ip->lastUsedPts = currentPts;
        }
        /* reset lastSeekPosition */
        playback_ip->lastSeekPositionSet = false;
        playback_ip->lastSeekPosition = 0;
    }

    B_Time_Get(&playback_ip->mediaStartTime);
    playback_ip->mediaStartTimeNoted = true;
    if (playback_ip->lastPtsExtrapolated == 0)
        playback_ip->lastPtsExtrapolated = playback_ip->firstPts + playback_ip->psi.duration * 45;
    if (playback_ip->originalFirstPts == 0)
        playback_ip->originalFirstPts = playback_ip->firstPts;

out:
    if (playback_ip) {
        if (playback_ip->appFirstPts.callback && playback_ip->appFirstPts.callback != firstPtsCallback)
            playback_ip->appFirstPts.callback(playback_ip->appFirstPts.context, playback_ip->appFirstPts.param);
        if (playback_ip->streamStatusAvailable.callback && playback_ip->streamStatusAvailable.callback != firstPtsCallback)
            playback_ip->streamStatusAvailable.callback(playback_ip->streamStatusAvailable.context, playback_ip->streamStatusAvailable.param);
#if 0
        /* commenting these out as this change breaks time position in server side trickmode case. */
        B_PlaybackIp_ResetAudioPtsCallback(playback_ip);
        B_PlaybackIp_ResetVideoPtsCallback(playback_ip);
#endif
    }
}

static void
firstPtsPassedCallback(void *context, int param)
{
    uint32_t currentPts;
    B_PlaybackIpHandle playback_ip = (B_PlaybackIpHandle)context;

    BSTD_UNUSED(param);
    BDBG_ASSERT(playback_ip);

    /* get pts corresponding to the 1st frame */
    if ((http_get_current_pts(playback_ip, &currentPts) != B_ERROR_SUCCESS) || currentPts == 0) {
        BDBG_MSG(("%s: DM either failed to return PTS or returned 0 as current PTS, not setting the firstPts", __FUNCTION__));
        goto out;
    }

    playback_ip->firstPtsPassed = true;
#ifdef BDBG_DEBUG_BUILD
    if (playback_ip->ipVerboseLog)
        BDBG_WRN(("%s:%p first passed pts 0x%x", __FUNCTION__, (void *)playback_ip, currentPts));
#endif
out:
    if (playback_ip && playback_ip->appFirstPtsPassed.callback && playback_ip->appFirstPtsPassed.callback != firstPtsPassedCallback)
        playback_ip->appFirstPtsPassed.callback(playback_ip->appFirstPtsPassed.context, playback_ip->appFirstPtsPassed.param);
}

static void
sourceChangedCallback(void *context, int param)
{
    B_PlaybackIpHandle playback_ip = (B_PlaybackIpHandle)context;
    NEXUS_VideoDecoderStatus status;
    BSTD_UNUSED(param);
    if (playback_ip->nexusHandles.videoDecoder) {
        if (NEXUS_VideoDecoder_GetStatus(playback_ip->nexusHandles.videoDecoder, &status) != NEXUS_SUCCESS) {
            BDBG_ERR(("%s: NEXUS_VideoDecoder_GetStatus() Failed", __FUNCTION__));
            goto out;
        }
        BDBG_WRN(("%s: res: source %dx%d, coded %dx%d, display %dx%d, ar %d, fr %d, interlaced %d video format %d, muted %d", __FUNCTION__,
                    status.source.width, status.source.height,
                    status.coded.width, status.coded.height,
                    status.display.width, status.display.height,
                    status.aspectRatio,
                    status.frameRate,
                    status.interlaced,
                    status.format,
                    status.muted
                    ));
        playback_ip->frameRate = status.frameRate;
    }
#ifdef NEXUS_HAS_SIMPLE_DECODER
    else if (playback_ip->nexusHandles.simpleVideoDecoder) {
        if (NEXUS_SimpleVideoDecoder_GetStatus(playback_ip->nexusHandles.simpleVideoDecoder, &status) != NEXUS_SUCCESS) {
            BDBG_ERR(("%s: NEXUS_SimpleVideoDecoder_GetStatus() Failed", __FUNCTION__));
            goto out;
        }
        BDBG_WRN(("%s: res: source %dx%d, coded %dx%d, display %dx%d, ar %d, fr %d, interlaced %d video format %d, muted %d", __FUNCTION__,
                    status.source.width, status.source.height,
                    status.coded.width, status.coded.height,
                    status.display.width, status.display.height,
                    status.aspectRatio,
                    status.frameRate,
                    status.interlaced,
                    status.format,
                    status.muted
                    ));
        playback_ip->frameRate = status.frameRate;
    }
#endif

out:
    /* now invoke this callback for app if it had registered for it */
    if (playback_ip->appSourceChanged.callback && playback_ip->appSourceChanged.callback != sourceChangedCallback)
        playback_ip->appSourceChanged.callback(playback_ip->appSourceChanged.context, playback_ip->appSourceChanged.param);
}

B_PlaybackIpError
B_PlaybackIp_SetVideoPtsCallback(
    B_PlaybackIpHandle playback_ip
    )
{
    B_PlaybackIpError errorCode = B_ERROR_PROTO;

    if (playback_ip->nexusHandles.videoDecoder) {
        NEXUS_VideoDecoderSettings videoDecoderSettings;
        NEXUS_VideoDecoder_GetSettings(playback_ip->nexusHandles.videoDecoder, &videoDecoderSettings);
        videoDecoderSettings.firstPts.callback = firstPtsCallback;
        videoDecoderSettings.firstPts.context = playback_ip;
        if (NEXUS_VideoDecoder_SetSettings(playback_ip->nexusHandles.videoDecoder, &videoDecoderSettings) != NEXUS_SUCCESS) {
            BDBG_ERR(("%s: Failed to set the 1st pts callback for video decoder", __FUNCTION__));
            goto error;
        }
    }
    else if (playback_ip->nexusHandles.simpleVideoDecoder) {
        NEXUS_VideoDecoderSettings videoDecoderSettings;
        NEXUS_SimpleVideoDecoder_GetSettings(playback_ip->nexusHandles.simpleVideoDecoder, &videoDecoderSettings);
        videoDecoderSettings.firstPts.callback = firstPtsCallback;
        videoDecoderSettings.firstPts.context = playback_ip;
        if (NEXUS_SimpleVideoDecoder_SetSettings(playback_ip->nexusHandles.simpleVideoDecoder, &videoDecoderSettings) != NEXUS_SUCCESS) {
            BDBG_ERR(("%s: Failed to set the 1st pts callback\n", __FUNCTION__));
            goto error;
        }
    }
    BDBG_MSG(("%s: ctx %p: enabled the video first pts callback!", __FUNCTION__, (void *)playback_ip));
    errorCode = B_ERROR_SUCCESS;
error:
    return errorCode;
}

B_PlaybackIpError
B_PlaybackIp_SetAudioPtsCallback(
    B_PlaybackIpHandle playback_ip
    )
{
    B_PlaybackIpError errorCode = B_ERROR_PROTO;

    if (playback_ip->nexusHandles.primaryAudioDecoder) {
        NEXUS_AudioDecoderSettings audioDecoderSettings;
        NEXUS_AudioDecoder_GetSettings(playback_ip->nexusHandles.primaryAudioDecoder, &audioDecoderSettings);
        audioDecoderSettings.streamStatusAvailable.callback = firstPtsCallback;
        audioDecoderSettings.streamStatusAvailable.context = playback_ip;
        if (NEXUS_AudioDecoder_SetSettings(playback_ip->nexusHandles.primaryAudioDecoder, &audioDecoderSettings) != NEXUS_SUCCESS) {
            BDBG_ERR(("%s: Failed to set the 1st pts callback for audio decoder", __FUNCTION__));
            goto error;
        }
    }
    else if (playback_ip->nexusHandles.simpleAudioDecoder) {
        NEXUS_SimpleAudioDecoderSettings  * pAudioDecoderSettings;
        NEXUS_Error                         errCode = NEXUS_SUCCESS;

        /* Need to malloc pAudioDecoderSettings because Coverity will complain that
         * NEXUS_SimpleAudioDecoderSettings is too big to go on the stack.*/
        pAudioDecoderSettings = BKNI_Malloc(sizeof(* pAudioDecoderSettings));
        if (!pAudioDecoderSettings) {
            BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
            goto error;
        }
        NEXUS_SimpleAudioDecoder_GetSettings(playback_ip->nexusHandles.simpleAudioDecoder, pAudioDecoderSettings);
        pAudioDecoderSettings->primary.streamStatusAvailable.callback = firstPtsCallback;
        pAudioDecoderSettings->primary.streamStatusAvailable.context = playback_ip;
        errCode = NEXUS_SimpleAudioDecoder_SetSettings(playback_ip->nexusHandles.simpleAudioDecoder, pAudioDecoderSettings);
        BKNI_Free(pAudioDecoderSettings);
        if (errCode != NEXUS_SUCCESS) {
            BDBG_ERR(("%s: Failed to unset the 1st pts callback for simple audio decoder", __FUNCTION__));
            goto error;
        }
    }
    BDBG_MSG(("%s: ctx %p: enabled the audio first pts callback!", __FUNCTION__, (void *)playback_ip));
    errorCode = B_ERROR_SUCCESS;
error:
    return errorCode;
}

B_PlaybackIpError
B_PlaybackIp_SessionStart(
    B_PlaybackIpHandle playback_ip,
    B_PlaybackIpSessionStartSettings *startSettings,
    B_PlaybackIpSessionStartStatus *startStatus /* [out] */
    )
{
    B_PlaybackIpError errorCode = B_ERROR_PROTO;
    NEXUS_VideoDecoderSettings videoDecoderSettings;

    BDBG_MSG(("%s: playback_ip %p, startSettings %p, startStatus %p\n", __FUNCTION__, (void *)playback_ip, (void *)startSettings, (void *)startStatus));
    if (!playback_ip || !startSettings || !startStatus) {
        BDBG_ERR(("%s: invalid params, playback_ip %p, startSettings %p, startStatus %p\n", __FUNCTION__, (void *)playback_ip, (void *)startSettings, (void *)startStatus));
        return B_ERROR_INVALID_PARAMETER;
    }

    /* Validate if app can call this API in the current IP Playback State */
    switch (playback_ip->playback_state) {
    case B_PlaybackIpState_eSessionOpened:
        if (playback_ip->openSettings.socketOpenSettings.protocol == B_PlaybackIpProtocol_eHttp ||
            playback_ip->openSettings.socketOpenSettings.protocol == B_PlaybackIpProtocol_eRtsp) {
            /* SessionSetup needs to be called at least once for the HTTP/RTSP protocols */
            if (!playback_ip->sessionSetupCompleted) {
                BDBG_ERR(("%s: ERROR: Ip_SessionSetup() needs to be called before Ip_SessionStart() for HTTP/RTSP protocols, ip state %d",
                        __FUNCTION__, playback_ip->playback_state));
                return B_ERROR_NOT_SUPPORTED;
            }
            else {
                /* session setup has been completed atleast once for this session, so we can proceed w/ start below */
                break;
            }
        }
        else {
            /* For all other protocols, app can call SessionStart w/o calling SessionSetup */
            /* so continue below with this API */
            break;
        }
    case B_PlaybackIpState_eSessionSetup:
        /* continue below with this API */
        break;
    case B_PlaybackIpState_eSessionStartInProgress:
        goto checkStartStatus;
        break;
    default:
        /* In all other states, app can't call this API */
        BDBG_ERR(("ERROR: Can't call %s() in this state %d\n", __FUNCTION__, playback_ip->playback_state));
        return B_ERROR_NOT_SUPPORTED;
    }

    /* now API is progress, update state to reflect that */
    playback_ip->playback_state = B_PlaybackIpState_eSessionStartInProgress;
    BKNI_ResetEvent(playback_ip->playback_halt_event);

    memset(startStatus, 0, sizeof(B_PlaybackIpSessionStartStatus));
    playback_ip->startSettings = *startSettings;
    if (startSettings->nexusHandlesValid)
        playback_ip->nexusHandles = startSettings->nexusHandles;
    playback_ip->byte_count = 0;
    playback_ip->numRecvTimeouts = 0;

    if (!playback_ip->startSettings.musicChannelWithVideoStills && playback_ip->nexusHandles.videoDecoder) {
        NEXUS_VideoDecoder_GetSettings(playback_ip->nexusHandles.videoDecoder, &videoDecoderSettings);
        /* save app's callback functions so that we can cacade them */
        if (videoDecoderSettings.sourceChanged.callback != sourceChangedCallback) {
            playback_ip->appSourceChanged = videoDecoderSettings.sourceChanged;
        }
        if (videoDecoderSettings.firstPts.callback != firstPtsCallback) {
            playback_ip->appFirstPts = videoDecoderSettings.firstPts;
        }
        if (videoDecoderSettings.firstPtsPassed.callback != firstPtsPassedCallback) {
            playback_ip->appFirstPtsPassed = videoDecoderSettings.firstPtsPassed;
        }
        playback_ip->appPtsError = videoDecoderSettings.ptsError;

        videoDecoderSettings.firstPts.callback = firstPtsCallback;
        videoDecoderSettings.firstPts.context = playback_ip;
        videoDecoderSettings.firstPtsPassed.callback = firstPtsPassedCallback;
        videoDecoderSettings.firstPtsPassed.context = playback_ip;
        videoDecoderSettings.sourceChanged.callback = sourceChangedCallback;
        videoDecoderSettings.sourceChanged.context = playback_ip;
        videoDecoderSettings.ptsError.callback = ptsErrorCallback;
        videoDecoderSettings.ptsError.context = playback_ip;
        if (NEXUS_VideoDecoder_SetSettings(playback_ip->nexusHandles.videoDecoder, &videoDecoderSettings) != NEXUS_SUCCESS) {
            BDBG_ERR(("%s: Failed to set the 1st pts callback\n", __FUNCTION__));
            goto error;
        }
    }
#ifdef NEXUS_HAS_SIMPLE_DECODER
    else if (!playback_ip->startSettings.musicChannelWithVideoStills && playback_ip->nexusHandles.simpleVideoDecoder) {
        NEXUS_SimpleVideoDecoder_GetSettings(playback_ip->nexusHandles.simpleVideoDecoder, &videoDecoderSettings);
        /* save app's callback functions so that we can cacade them */
        if (videoDecoderSettings.sourceChanged.callback != sourceChangedCallback) {
            playback_ip->appSourceChanged = videoDecoderSettings.sourceChanged;
        }
        if (videoDecoderSettings.firstPts.callback != firstPtsCallback) {
            playback_ip->appFirstPts = videoDecoderSettings.firstPts;
        }
        if (videoDecoderSettings.firstPtsPassed.callback != firstPtsPassedCallback) {
            playback_ip->appFirstPtsPassed = videoDecoderSettings.firstPtsPassed;
        }
        playback_ip->appPtsError = videoDecoderSettings.ptsError;

        videoDecoderSettings.firstPts.callback = firstPtsCallback;
        videoDecoderSettings.firstPts.context = playback_ip;
        videoDecoderSettings.firstPtsPassed.callback = firstPtsPassedCallback;
        videoDecoderSettings.firstPtsPassed.context = playback_ip;
        videoDecoderSettings.sourceChanged.callback = sourceChangedCallback;
        videoDecoderSettings.sourceChanged.context = playback_ip;
        videoDecoderSettings.ptsError.callback = ptsErrorCallback;
        videoDecoderSettings.ptsError.context = playback_ip;
        if (NEXUS_SimpleVideoDecoder_SetSettings(playback_ip->nexusHandles.simpleVideoDecoder, &videoDecoderSettings) != NEXUS_SUCCESS) {
            BDBG_ERR(("%s: Failed to set the 1st pts callback\n", __FUNCTION__));
            goto error;
        }
    }
#endif
    else if (playback_ip->nexusHandles.primaryAudioDecoder) {
        NEXUS_AudioDecoderSettings audioDecoderSettings;
        NEXUS_AudioDecoder_GetSettings(playback_ip->nexusHandles.primaryAudioDecoder, &audioDecoderSettings);
        if (audioDecoderSettings.streamStatusAvailable.callback != firstPtsCallback) {
            playback_ip->streamStatusAvailable = audioDecoderSettings.firstPts;
        }
        audioDecoderSettings.streamStatusAvailable.callback = firstPtsCallback;
        audioDecoderSettings.streamStatusAvailable.context = playback_ip;
        if (NEXUS_AudioDecoder_SetSettings(playback_ip->nexusHandles.primaryAudioDecoder, &audioDecoderSettings) != NEXUS_SUCCESS) {
            BDBG_ERR(("%s: Failed to set the 1st pts callback for audio decoder", __FUNCTION__));
            goto error;
        }
    }
#ifdef NEXUS_HAS_SIMPLE_DECODER
    else if (playback_ip->nexusHandles.simpleAudioDecoder) {
        NEXUS_SimpleAudioDecoderSettings  * pAudioDecoderSettings;
        NEXUS_Error                         errCode = NEXUS_SUCCESS;

        /* Need to malloc pAudioDecoderSettings because Coverity will complain that
         * NEXUS_SimpleAudioDecoderSettings is too big to go on the stack.*/
        pAudioDecoderSettings = BKNI_Malloc(sizeof(* pAudioDecoderSettings));
         if (!pAudioDecoderSettings) {
             BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
             goto error;
         }
        NEXUS_SimpleAudioDecoder_GetSettings(playback_ip->nexusHandles.simpleAudioDecoder, pAudioDecoderSettings);
        if (pAudioDecoderSettings->primary.streamStatusAvailable.callback != firstPtsCallback) {
            playback_ip->streamStatusAvailable = pAudioDecoderSettings->primary.streamStatusAvailable;
        }
        pAudioDecoderSettings->primary.streamStatusAvailable.callback = firstPtsCallback;
        pAudioDecoderSettings->primary.streamStatusAvailable.context = playback_ip;
        errCode = NEXUS_SimpleAudioDecoder_SetSettings(playback_ip->nexusHandles.simpleAudioDecoder, pAudioDecoderSettings);
        BKNI_Free(pAudioDecoderSettings);
        if (errCode != NEXUS_SUCCESS) {
            BDBG_ERR(("%s: Failed to unset the 1st pts callback for simple audio decoder", __FUNCTION__));
            goto error;
        }
    }
#endif

checkStartStatus:
    switch (playback_ip->openSettings.socketOpenSettings.protocol) {
#ifndef B_HAS_SMS_GATEWAY
    case B_PlaybackIpProtocol_eHttp:
        errorCode = B_PlaybackIp_HttpSessionStart(playback_ip, startSettings, startStatus);
        break;
#endif
#ifdef LIVEMEDIA_SUPPORT
    case B_PlaybackIpProtocol_eRtsp:
        errorCode = B_PlaybackIp_RtspSessionStart(playback_ip, startSettings, startStatus);
        break;
#endif

    case B_PlaybackIpProtocol_eRtp:
    case B_PlaybackIpProtocol_eRtpNoRtcp:
        errorCode = B_PlaybackIp_RtpSessionStart(playback_ip, startSettings, startStatus);
        break;

    case B_PlaybackIpProtocol_eUdp:
        errorCode = B_PlaybackIp_UdpSessionStart(playback_ip, startSettings, startStatus);
        break;
    case B_PlaybackIpProtocol_eRtpFec:
    default:
        BDBG_MSG(("%s: Session start failed: protocol %d not supported", __FUNCTION__, playback_ip->openSettings.socketOpenSettings.protocol));
        errorCode = B_ERROR_INVALID_PARAMETER;
        break;
    }

    if (errorCode == B_ERROR_IN_PROGRESS)
        return B_ERROR_IN_PROGRESS;
    else if (errorCode != B_ERROR_SUCCESS)
        goto error;

    /* now this API is successfully completed, update state to reflect that */
    if (playback_ip->startSettings.startPaused == true) {
        playback_ip->playback_state = B_PlaybackIpState_ePaused;
    }
    else {
        playback_ip->playback_state = B_PlaybackIpState_ePlaying;
    }
    BDBG_MSG(("%s() completed successfully, playback_ip %p\n", __FUNCTION__, (void *)playback_ip));
    return B_ERROR_SUCCESS;

error:
    playback_ip->playback_state = B_PlaybackIpState_eSessionSetup;
    return errorCode;
}

/*
Summary:
    Stops IP playback.
Description:
    This function must be called when app no longer wants to tune to a live IP or a playback channel over network.
    Depending on the current state of the playback IP channel, this function stops the IP internal thread
    and does some protocol specific cleanup (stop reading data from socket, etc.). It then changes
    state to eStopped.
Note:
    Socket is not closed in this call. Thus, App can temporarily stop receiving live data and then
    restart receiving data on this socket using the B_PlaybackIp_Start().
    App must call B_PlaybackIp_SessionClose() to free up the socket related resources.
    Also, this is a blocking function call.
*/
B_PlaybackIpError B_PlaybackIp_SessionStop(
    B_PlaybackIpHandle playback_ip
    )
{
    BERR_Code rc;
#ifdef B_HAS_NETACCEL
    STRM_SockSetFilterState_t sockFilterState;
#endif
    B_PlaybackIpState currentState;

    if (!playback_ip) return B_ERROR_INVALID_PARAMETER;

    currentState = playback_ip->playback_state;
    BDBG_MSG(("%s:%p playback ip state %d\n", __FUNCTION__, (void *)playback_ip, playback_ip->playback_state));
    switch (playback_ip->playback_state) {
    case B_PlaybackIpState_eOpened:
    case B_PlaybackIpState_eSessionOpened:
    case B_PlaybackIpState_eStopping:
        BDBG_MSG(("%s: nothing to stop in this state %d... ", __FUNCTION__, playback_ip->playback_state));
        return (B_ERROR_SUCCESS);
    case B_PlaybackIpState_eSessionSetup:
        BDBG_MSG(("%s: nothing to stop in this state %d... ", __FUNCTION__, playback_ip->playback_state));
        goto out; /* Setup is complete, so we just need to go back to the stopped state. */
    case B_PlaybackIpState_eSessionOpenInProgress:
    case B_PlaybackIpState_eSessionSetupInProgress:
        BKNI_ResetEvent(playback_ip->playback_halt_event);
        BDBG_MSG(("%s: need to abort this in progress state %d... ", __FUNCTION__, playback_ip->playback_state));
        break;
    case B_PlaybackIpState_eSessionStartInProgress:
        BKNI_ResetEvent(playback_ip->playback_halt_event);
        break;
    case B_PlaybackIpState_ePlaying:
    case B_PlaybackIpState_eBuffering:
    case B_PlaybackIpState_ePaused:
    case B_PlaybackIpState_eEnteringTrickMode:
    case B_PlaybackIpState_eTrickMode:
        /* Continue below to Stop the IP Session */
        break;
    default:
        BDBG_MSG(("%s: nothing to stop in this state %d... ", __FUNCTION__, playback_ip->playback_state));
        return B_ERROR_SUCCESS;

    }

    /* change to stopping state as stopping the IP thread can take some time */
    playback_ip->playback_state = B_PlaybackIpState_eStopping;
    BDBG_MSG(("%s:%p playback ip state changed to %d\n", __FUNCTION__, (void *)playback_ip, playback_ip->playback_state));

    if (playback_ip->sessionOpenRetryEventHandle != NULL) {
        BKNI_SetEvent(playback_ip->sessionOpenRetryEventHandle);
    }

#ifdef SPF_SUPPORT
    if (playback_ip->openSettings.socketOpenSettings.protocol == B_PlaybackIpProtocol_eRtsp) {
        /* for RTSP, lm scheduler doesn't return until app calls stop */
#ifdef LIVEMEDIA_SUPPORT
        B_PlaybackIp_RtspSessionStop(playback_ip);
#endif
    }
#endif /* SPF_SUPPORT */

    rc = BKNI_WaitForEvent(playback_ip->playback_halt_event, IP_HALT_TASK_TIMEOUT_MSEC);
    if (rc == BERR_TIMEOUT) {
        BDBG_WRN(("%s: playback_halt_event was timed out", __FUNCTION__));
    } else if (rc!=0) {
        BDBG_ERR(("%s: failed to stop the IP thread: playback_halt_event timed out due to error rc = %d", __FUNCTION__, rc));
        goto error;
    }
    if (playback_ip->playbackIpThread)
        B_Thread_Destroy(playback_ip->playbackIpThread);
    BDBG_MSG(("%s: Playback IP thread is stopped \n", __FUNCTION__));

    if (currentState < B_PlaybackIpState_eSessionSetup) {
        BDBG_ERR(("%s:%p Skipping remaining stop processing as session wasn't started, state=%d ", __FUNCTION__, (void *)playback_ip, playback_ip->playback_state));
        goto out;
    }
    switch (playback_ip->openSettings.socketOpenSettings.protocol) {
    case B_PlaybackIpProtocol_eUdp:
    case B_PlaybackIpProtocol_eRtpFec:
        break;
#ifndef B_HAS_SMS_GATEWAY
    case B_PlaybackIpProtocol_eHttp:
        B_PlaybackIp_HttpSessionStop(playback_ip);
        break;
#endif

    case B_PlaybackIpProtocol_eRtp:
    case B_PlaybackIpProtocol_eRtpNoRtcp:
        B_PlaybackIp_RtpSessionStop(playback_ip);
        break;
#ifdef LIVEMEDIA_SUPPORT
    case B_PlaybackIpProtocol_eRtsp:
        B_PlaybackIp_RtspSessionStop(playback_ip);
        break;
#endif

    default:
        BDBG_WRN(("%s: Bad protocol", __FUNCTION__));
        break;
    }

    if (playback_ip->protocol != B_PlaybackIpProtocol_eHttp) {
#ifdef B_HAS_NETACCEL
        /* filter only needs to be disbled for live protocols */
        sockFilterState.filterEnable = 0;
        if (setsockopt(playback_ip->socketState.fd, SOCK_BRCM_DGRAM, STRM_SOCK_SET_FILTER_STATE, &sockFilterState, sizeof(sockFilterState)) != 0)
        {
            BDBG_ERR(("%s: setsockopt() ERROR:", __FUNCTION__));
        }
        BDBG_ERR(("%s: Disabled Net DMA filter\n", __FUNCTION__));
#endif
    }

    if (!playback_ip->startSettings.musicChannelWithVideoStills && playback_ip->nexusHandles.videoDecoder) {
        NEXUS_VideoDecoderSettings videoDecoderSettings;
        NEXUS_VideoDecoder_GetSettings(playback_ip->nexusHandles.videoDecoder, &videoDecoderSettings);
        videoDecoderSettings.firstPts = playback_ip->appFirstPts;
        playback_ip->appFirstPts.callback = NULL;
        videoDecoderSettings.firstPtsPassed = playback_ip->appFirstPtsPassed;
        playback_ip->appFirstPtsPassed.callback = NULL;
        if (playback_ip->appSourceChanged.callback != sourceChangedCallback)
            videoDecoderSettings.sourceChanged = playback_ip->appSourceChanged;
        playback_ip->appSourceChanged.callback = NULL;
        videoDecoderSettings.ptsError = playback_ip->appPtsError;
        playback_ip->appPtsError.callback = NULL;
        if (NEXUS_VideoDecoder_SetSettings(playback_ip->nexusHandles.videoDecoder, &videoDecoderSettings) != NEXUS_SUCCESS) {
            BDBG_ERR(("%s: Failed to unset the 1st pts callback\n", __FUNCTION__));
            goto error;
        }
    }
#ifdef NEXUS_HAS_SIMPLE_DECODER
    else if (!playback_ip->startSettings.musicChannelWithVideoStills && playback_ip->nexusHandles.simpleVideoDecoder) {
        NEXUS_VideoDecoderSettings videoDecoderSettings;
        NEXUS_SimpleVideoDecoder_GetSettings(playback_ip->nexusHandles.simpleVideoDecoder, &videoDecoderSettings);
        videoDecoderSettings.firstPts = playback_ip->appFirstPts;
        playback_ip->appFirstPts.callback = NULL;
        videoDecoderSettings.firstPtsPassed = playback_ip->appFirstPtsPassed;
        playback_ip->appFirstPtsPassed.callback = NULL;
        if (playback_ip->appSourceChanged.callback != sourceChangedCallback)
            videoDecoderSettings.sourceChanged = playback_ip->appSourceChanged;
        playback_ip->appSourceChanged.callback = NULL;
        videoDecoderSettings.ptsError = playback_ip->appPtsError;
        playback_ip->appPtsError.callback = NULL;
        if (NEXUS_SimpleVideoDecoder_SetSettings(playback_ip->nexusHandles.simpleVideoDecoder, &videoDecoderSettings) != NEXUS_SUCCESS) {
            BDBG_ERR(("%s: Failed to unset the 1st pts callback\n", __FUNCTION__));
            goto error;
        }
        NEXUS_SimpleVideoDecoder_GetSettings(playback_ip->nexusHandles.simpleVideoDecoder, &videoDecoderSettings);
    }
#endif
    else if (playback_ip->nexusHandles.primaryAudioDecoder) {
        NEXUS_AudioDecoderSettings audioDecoderSettings;
        NEXUS_AudioDecoder_GetSettings(playback_ip->nexusHandles.primaryAudioDecoder, &audioDecoderSettings);
        audioDecoderSettings.streamStatusAvailable = playback_ip->streamStatusAvailable;
        playback_ip->streamStatusAvailable.callback = NULL;
        if (NEXUS_AudioDecoder_SetSettings(playback_ip->nexusHandles.primaryAudioDecoder, &audioDecoderSettings) != NEXUS_SUCCESS) {
            BDBG_ERR(("%s: Failed to unset the 1st pts callback for audio decoder", __FUNCTION__));
            goto error;
        }
    }
#ifdef NEXUS_HAS_SIMPLE_DECODER
    else if (playback_ip->nexusHandles.simpleAudioDecoder) {
        NEXUS_SimpleAudioDecoderSettings  * pAudioDecoderSettings;
        NEXUS_Error                         errCode = NEXUS_SUCCESS;

        /* Need to malloc pAudioDecoderSettings because Coverity will complain that
         * NEXUS_SimpleAudioDecoderSettings is too big to go on the stack.*/
        pAudioDecoderSettings = BKNI_Malloc(sizeof(* pAudioDecoderSettings));
         if (!pAudioDecoderSettings) {
             rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
             goto error;
         }
        NEXUS_SimpleAudioDecoder_GetSettings(playback_ip->nexusHandles.simpleAudioDecoder, pAudioDecoderSettings);
        pAudioDecoderSettings->primary.streamStatusAvailable = playback_ip->streamStatusAvailable;
        playback_ip->streamStatusAvailable.callback = NULL;
        errCode = NEXUS_SimpleAudioDecoder_SetSettings(playback_ip->nexusHandles.simpleAudioDecoder, pAudioDecoderSettings);
        BKNI_Free(pAudioDecoderSettings);

        if (errCode != NEXUS_SUCCESS) {
            BDBG_ERR(("%s: Failed to unset the 1st pts callback for simple audio decoder", __FUNCTION__));
            goto error;
        }
    }
#endif

out:
    if (currentState == B_PlaybackIpState_eSessionOpenInProgress)
        playback_ip->playback_state = B_PlaybackIpState_eOpened;
    else
        playback_ip->playback_state = B_PlaybackIpState_eStopped;
    return B_ERROR_SUCCESS;

error:
    BDBG_ERR(("%s: error", __FUNCTION__));
    return B_ERROR_UNKNOWN;
}

/*
Summary:
    Close a IP playback socket.
Description:
    Close a IP playback socket opened by bplayback_socketState_open(). Playback
    must be stopped BEFORE socket is closed
*/
B_PlaybackIpError B_PlaybackIp_SessionClose(
    B_PlaybackIpHandle playback_ip /* Handle returned by bplayback_ip_open */
    )
{
    BERR_Code rc;

    /* TODO: if state is SessionOpenInProgress, need to cancel that action */

    switch (playback_ip->playback_state) {
    case B_PlaybackIpState_eOpened:
        /* since session is not yet started, there is nothing to close, return */
        return B_ERROR_SUCCESS;

    case B_PlaybackIpState_eSessionStartInProgress:
    case B_PlaybackIpState_ePlaying:
    case B_PlaybackIpState_eBuffering:
    case B_PlaybackIpState_ePaused:
    case B_PlaybackIpState_eEnteringTrickMode:
    case B_PlaybackIpState_eTrickMode:
        B_PlaybackIp_SessionStop(playback_ip);
        /* continue below w/ session close cleanup */

    case B_PlaybackIpState_eSessionOpenInProgress:
    case B_PlaybackIpState_eSessionSetupInProgress:
        /* temporarily change state to Stopping to allow media probing to fail & return */
        BKNI_ResetEvent(playback_ip->playback_halt_event);
        playback_ip->playback_state = B_PlaybackIpState_eStopping;
        rc = BKNI_WaitForEvent(playback_ip->playback_halt_event, IP_HALT_TASK_TIMEOUT_MSEC);
        if (rc == BERR_TIMEOUT) {
            BDBG_WRN(("%s: playback_halt_event was timed out", __FUNCTION__));
        } else if (rc!=0) {
            BDBG_ERR(("%s: failed to stop the media probe thread: playback_halt_event timed out due to error rc = %d", __FUNCTION__, rc));
        }
    case B_PlaybackIpState_eSessionSetup:
    case B_PlaybackIpState_eStopped:
    /* case B_PlaybackIpState_eSessionOpened: */
        /* continue below w/ session close cleanup */
        break;
    default:
    case B_PlaybackIpState_eStopping:
        /* error */
        BDBG_ERR(("ERROR: can't call %s() in this state %d as another thread is concurrently calling Ip_SessionStop()", __FUNCTION__, playback_ip->playback_state));
        return B_ERROR_NOT_SUPPORTED;
    }

    BDBG_MSG(("%s: ip state %d proto %d", __FUNCTION__, playback_ip->playback_state, playback_ip->protocol));

    switch (playback_ip->protocol) {
#ifndef B_HAS_SMS_GATEWAY
    case B_PlaybackIpProtocol_eHttp:
        B_PlaybackIp_HttpSessionClose(playback_ip);
        break;
#endif
    case B_PlaybackIpProtocol_eUdp:
        B_PlaybackIp_UdpSessionClose(playback_ip);
        break;

    case B_PlaybackIpProtocol_eRtp:
    case B_PlaybackIpProtocol_eRtpNoRtcp:
        B_PlaybackIp_RtpSessionClose(playback_ip);
        break;
#ifdef LIVEMEDIA_SUPPORT
    case B_PlaybackIpProtocol_eRtsp:
#ifndef SPF_SUPPORT
        B_PlaybackIp_RtspSessionClose(playback_ip);
#endif /* SPF_SUPPORT */
        break;
#endif

    default:
        break;
    }

    /* do socket related cleanup */
    if (playback_ip->socketState.fd > 0)
        close(playback_ip->socketState.fd);
    memset(&playback_ip->socketState, 0, sizeof(playback_ip->socketState));
    BDBG_MSG(("%s: closed fd %d", __FUNCTION__, playback_ip->socketState.fd ));
    playback_ip->socketState.fd = 0;

    /* switch to the opened state, this channel can be used for another session */
    playback_ip->playback_state = B_PlaybackIpState_eOpened;

    if (playback_ip->openSettings.socketOpenSettings.url) {
        BKNI_Free(playback_ip->openSettings.socketOpenSettings.url);
        playback_ip->openSettings.socketOpenSettings.url = NULL;
    }
    BKNI_DestroyEvent(playback_ip->playback_halt_event);
    playback_ip->playback_halt_event = NULL;
    BKNI_DestroyEvent(playback_ip->preChargeBufferEvent);
    BKNI_DestroyEvent(playback_ip->newJobEvent);
    BKNI_DestroyEvent(playback_ip->read_callback_event);
    BKNI_DestroyEvent(playback_ip->liveMediaSyncEvent);
    if (playback_ip->sessionOpenRetryEventHandle) {
        BKNI_DestroyEvent(playback_ip->sessionOpenRetryEventHandle);
        playback_ip->sessionOpenRetryEventHandle = NULL;
    }
    if (playback_ip->lock) {
        BKNI_DestroyMutex(playback_ip->lock);
        playback_ip->lock = NULL;
    }
    return B_ERROR_SUCCESS;
}

/* routine to return the Playback IP Status */
B_PlaybackIpError
B_PlaybackIp_GetStatus(
    B_PlaybackIpHandle playback_ip,
    B_PlaybackIpStatus *ipStatus
    )
{
    B_PlaybackIpError rc = B_ERROR_SUCCESS;
    int cacheIndex;
    if (!ipStatus || !playback_ip) {
        return B_ERROR_INVALID_PARAMETER;
    }
    BKNI_Memset(ipStatus, 0, sizeof(*ipStatus));
    ipStatus->ipState = playback_ip->playback_state;
#ifndef B_HAS_SMS_GATEWAY
    cacheIndex = playback_ip->lastUsedCacheIndex;
    if (playback_ip->dataCache[cacheIndex].inUse && playback_ip->psi.avgBitRate) {
        ipStatus->maxBufferDuration = ((playback_ip->dataCache[cacheIndex].size - playback_ip->cacheDepthFudgeFactor)*8) / playback_ip->psi.avgBitRate;  /* in sec */
        ipStatus->maxBufferDuration *= 1000; /* in msec */
        if (playback_ip->lastSeekOffset < playback_ip->dataCache[cacheIndex].startOffset || playback_ip->lastSeekOffset > playback_ip->dataCache[cacheIndex].endOffset) {
            /* we have seeked or are trying to play from outside the current cache range, so our buffer duration is 0 */
            ipStatus->curBufferDuration = 0;
        }
        else {
            /* seek/play point is in the cache */
            ipStatus->curBufferDuration = ((playback_ip->dataCache[cacheIndex].endOffset - playback_ip->lastSeekOffset)*8) / playback_ip->psi.avgBitRate;  /* in sec */
            ipStatus->curBufferDuration *= 1000; /* in msec */
        }
    }
    ipStatus->numRecvTimeouts = playback_ip->numRecvTimeouts;
    ipStatus->serverClosed = playback_ip->serverClosed;
    ipStatus->httpStatusCode = playback_ip->statusCode;

    if (B_PlaybackIp_HttpGetCurrentPlaybackPosition(playback_ip, &ipStatus->position) != B_ERROR_SUCCESS) {
        BDBG_MSG(("%s: Failed to determine the current playback position, setting it to 0\n", __FUNCTION__));
        ipStatus->position = 0;
    }

    if (playback_ip->psi.duration)
        ipStatus->last = playback_ip->psi.duration;

#if 0
    BDBG_MSG(("%s: buffer duration: cur %d, max %d, state %d, closed %d, offsets: last seeked %lld, end %lld, start %lld, current position %u\n",
                __FUNCTION__, ipStatus->curBufferDuration ,ipStatus->maxBufferDuration, ipStatus->ipState, playback_ip->serverClosed, playback_ip->lastSeekOffset, playback_ip->dataCache[cacheIndex].endOffset, playback_ip->dataCache[cacheIndex].startOffset, ipStatus->position));
#endif
#else
    BSTD_UNUSED(cacheIndex);
#endif

#ifdef B_HAS_HLS_PROTOCOL_SUPPORT
    if (playback_ip->hlsSessionEnabled) {
        ipStatus->first = 0;
        if (playback_ip->hlsSessionState && playback_ip->hlsSessionState->currentPlaylistFile) {
            ipStatus->last = playback_ip->hlsSessionState->currentPlaylistFile->totalDuration;
        }
        if (playback_ip->hlsSessionState) {
            ipStatus->hlsStats.bounded = B_PlaybackIp_HlsBoundedStream(playback_ip);
            ipStatus->hlsStats.lastSegmentDownloadTime = playback_ip->hlsSessionState->lastSegmentDownloadTime;
            ipStatus->hlsStats.lastSegmentBitrate = playback_ip->hlsSessionState->lastSegmentBitrate;
            ipStatus->hlsStats.lastSegmentDuration = playback_ip->hlsSessionState->lastSegmentDuration;
            ipStatus->hlsStats.lastSegmentUrl = playback_ip->hlsSessionState->lastSegmentUrl;
            ipStatus->hlsStats.lastSegmentSequence = playback_ip->hlsSessionState->lastSegmentSequence;
        }
    }
#endif /* B_HAS_HLS_PROTOCOL_SUPPORT */

#ifdef B_HAS_MPEG_DASH_PROTOCOL_SUPPORT
    if (playback_ip->mpegDashSessionEnabled) {
        ipStatus->first = 0;
        if (playback_ip->mpegDashSessionState && playback_ip->mpegDashSessionState->currentRepresentation)
            ipStatus->last = playback_ip->mpegDashSessionState->currentRepresentation->totalDuration;
    }
#endif /* B_HAS_MPEG_DASH_PROTOCOL_SUPPORT */

#ifdef LIVEMEDIA_SUPPORT
    if(playback_ip->protocol == B_PlaybackIpProtocol_eRtp
#ifndef SPF_SUPPORT
|| playback_ip->protocol == B_PlaybackIpProtocol_eRtsp
#endif /* SPF_SUPPORT */
        ) {
        B_PlaybackIp_liveMediaSessionRange(playback_ip->lm_context, NULL, &ipStatus->last);
        ipStatus->first = 0;
    }
#endif
    if (playback_ip->protocol == B_PlaybackIpProtocol_eRtp && playback_ip->rtp) {
        brtp_stats stats;
        brtp_get_stats(playback_ip->rtp, &stats);
        ipStatus->rtpStats.packetsReceived = stats.packetsReceived;
        ipStatus->rtpStats.bytesReceived = stats.bytesReceived;
        ipStatus->rtpStats.packetsDiscarded = stats.packetsDiscarded;
        ipStatus->rtpStats.packetsOutOfSequence = stats.packetsOutOfSequence;
        ipStatus->rtpStats.packetsLost = stats.packetsLost;
        ipStatus->rtpStats.packetsLostBeforeErrorCorrection = stats.packetsLostBeforeErrorCorrection;
        ipStatus->rtpStats.lossEvents = stats.lossEvents;
        ipStatus->rtpStats.lossEventsBeforeErrorCorrection = stats.lossEventsBeforeErrorCorrection;
    }
    ipStatus->totalConsumed = playback_ip->totalConsumed;
    if (playback_ip->useNexusPlaypump == false && playback_ip->nexusHandles.playback) {
        NEXUS_Error rc;
        NEXUS_PlaybackStatus status;
        rc = NEXUS_Playback_GetStatus(playback_ip->nexusHandles.playback, &status);
        if (!rc) {
            ipStatus->first = status.first;
            ipStatus->last = status.last;
            ipStatus->position = status.position;
        }
    }

    /* IP Session related info */
    ipStatus->sessionInfo.protocol = playback_ip->protocol;
    ipStatus->sessionInfo.securityProtocol = playback_ip->openSettings.security.securityProtocol;
    ipStatus->sessionInfo.hlsSessionEnabled = playback_ip->psi.hlsSessionEnabled;
    ipStatus->sessionInfo.mpegDashSessionEnabled = playback_ip->psi.mpegDashSessionEnabled;
    ipStatus->sessionInfo.port = playback_ip->openSettings.socketOpenSettings.port;
    ipStatus->sessionInfo.url = (const char *)playback_ip->openSettings.socketOpenSettings.url;
    ipStatus->sessionInfo.ipAddr = (const char *)playback_ip->openSettings.socketOpenSettings.ipAddr;

    BDBG_MSG(("%s:%p monitoPsi = %d", __FUNCTION__, (void *)playback_ip, playback_ip->startSettings.monitorPsi));
    B_PlaybackIp_GetPsiStreamState(playback_ip->pPsiState, &ipStatus->stream);
    rc = B_ERROR_SUCCESS;
    return rc;
}

#if !defined(B_HAS_SMS_GATEWAY) || defined(SMS93383_SUPPORT)
void print_av_pipeline_buffering_status(
    B_PlaybackIpHandle playback_ip
    )
{
    NEXUS_VideoDecoderStatus videoStatus;
    NEXUS_AudioDecoderStatus audioStatus;
    NEXUS_PlaybackStatus playbackStatus;

    if (playback_ip->nexusHandles.videoDecoder) {
        if (NEXUS_VideoDecoder_GetStatus(playback_ip->nexusHandles.videoDecoder, &videoStatus) == NEXUS_SUCCESS) {
            BDBG_WRN(("Video Decoder Status: pts %u, fifoSize %d, fifoDepth %d, fullness %d%%",
                videoStatus.pts, videoStatus.fifoSize, videoStatus.fifoDepth, videoStatus.fifoSize?(videoStatus.fifoDepth*100)/videoStatus.fifoSize:0));
        }
    }
    if (playback_ip->nexusHandles.primaryAudioDecoder) {
        if (NEXUS_AudioDecoder_GetStatus(playback_ip->nexusHandles.primaryAudioDecoder, &audioStatus) == NEXUS_SUCCESS) {
            BDBG_WRN(("Audio Decoder Status: pts %u, fifoSize %d, fifoDepth %d, fullness %d%%",
                audioStatus.pts, audioStatus.fifoSize, audioStatus.fifoDepth, audioStatus.fifoSize?(audioStatus.fifoDepth*100)/audioStatus.fifoSize:0));
        }
    }
    if (playback_ip->nexusHandles.playback) {
        if (NEXUS_Playback_GetStatus(playback_ip->nexusHandles.playback, &playbackStatus) == NEXUS_SUCCESS) {
            BDBG_WRN(("Playback Status: PB buffer depth %d, size %d, fullness %d%%, played bytes %" PRIu64,
                    playbackStatus.fifoDepth, playbackStatus.fifoSize, (playbackStatus.fifoDepth*100)/playbackStatus.fifoSize, playbackStatus.bytesPlayed));
        }
    }
}
#endif

/***************************************************************************
Summary:
This function returns the default and recommended values for the Playback IP App Lib
public settings. A pointer to a valid B_PlaybackIpSettings structure must be
provide or an error will be returned.
***************************************************************************/
B_PlaybackIpError B_PlaybackIp_GetDefaultSettings(
    B_PlaybackIpSettings *pSettings)
{
    if (NULL == pSettings)
        return B_ERROR_INVALID_PARAMETER;
    BKNI_Memset( pSettings, 0, sizeof(B_PlaybackIpSettings) );

    pSettings->ipMode = B_PlaybackIpClockRecoveryMode_ePushWithPcrSyncSlip;
    pSettings->maxNetworkJitter = 300;
    pSettings->preChargeBuffer = false;
    pSettings->networkTimeout = HTTP_SELECT_TIMEOUT;
    pSettings->networkBufferSize = HTTP_DATA_CACHE_SIZE;
    pSettings->useNexusPlaypump = false;
    B_PlaybackIp_TtsThrottle_ParamsInit(&pSettings->ttsParams.throttleParams);
    pSettings->ttsParams.pacingMaxError = 2636;
    return B_ERROR_SUCCESS;
}

B_PlaybackIpError B_PlaybackIp_GetDefaultSessionOpenSettings(
    B_PlaybackIpSessionOpenSettings *openSettings
    )
{
    if (NULL == openSettings)
        return B_ERROR_INVALID_PARAMETER;
    BKNI_Memset(openSettings, 0, sizeof(B_PlaybackIpSessionOpenSettings) );
    openSettings->ipMode = B_PlaybackIpClockRecoveryMode_ePushWithPcrSyncSlip;
    openSettings->maxNetworkJitter = 300; /* 300msec */
    openSettings->networkTimeout = HTTP_SELECT_TIMEOUT; /* 5 sec */
    openSettings->nonBlockingMode = false;
    return B_ERROR_SUCCESS;
}

B_PlaybackIpError B_PlaybackIp_GetDefaultSessionSetupSettings(
    B_PlaybackIpSessionSetupSettings *setupSettings
    )
{
    if (NULL == setupSettings)
        return B_ERROR_INVALID_PARAMETER;
    BKNI_Memset(setupSettings, 0, sizeof(B_PlaybackIpSessionSetupSettings) );
    setupSettings->u.http.readTimeout = 100; /* 100msec */
    return B_ERROR_SUCCESS;
}

/***************************************************************************
Summary:
This function returns the current values for the Playback IP App Lib
public settings.
***************************************************************************/
B_PlaybackIpError B_PlaybackIp_GetSettings(
    B_PlaybackIpHandle playback_ip,
    B_PlaybackIpSettings *pSettings)
{
    if (NULL == pSettings)
        return B_ERROR_INVALID_PARAMETER;

    BKNI_Memcpy(pSettings, &playback_ip->settings, sizeof(B_PlaybackIpSettings) );
    pSettings->nexusHandles = playback_ip->nexusHandles;
    pSettings->playPositionOffsetValid = false;
    return B_ERROR_SUCCESS;
}

/***************************************************************************
Summary:
This function updates the current values for the Playback IP App Lib
public settings.
***************************************************************************/
B_PlaybackIpError B_PlaybackIp_SetSettings(
    B_PlaybackIpHandle playback_ip,
    B_PlaybackIpSettings *pSettings)
{
    BERR_Code rc;

    BDBG_ASSERT(playback_ip);
    if (NULL == pSettings)
        return B_ERROR_INVALID_PARAMETER;

    /* update Nexus Handles if app has supplied them */
    if (pSettings->nexusHandlesValid) {
        if (pSettings->nexusHandles.playpump)
            playback_ip->nexusHandles.playpump = pSettings->nexusHandles.playpump;
        if (pSettings->nexusHandles.playpump2)
            playback_ip->nexusHandles.playpump2 = pSettings->nexusHandles.playpump2;
        if (pSettings->nexusHandles.playback)
            playback_ip->nexusHandles.playback = pSettings->nexusHandles.playback;
        if (pSettings->nexusHandles.videoDecoder)
            playback_ip->nexusHandles.videoDecoder = pSettings->nexusHandles.videoDecoder;
        if (pSettings->nexusHandles.simpleVideoDecoder)
            playback_ip->nexusHandles.simpleVideoDecoder = pSettings->nexusHandles.simpleVideoDecoder;
        if (pSettings->nexusHandles.simpleAudioDecoder)
            playback_ip->nexusHandles.simpleAudioDecoder = pSettings->nexusHandles.simpleAudioDecoder;
        if (pSettings->nexusHandles.primaryAudioDecoder)
            playback_ip->nexusHandles.primaryAudioDecoder = pSettings->nexusHandles.primaryAudioDecoder;
        if (pSettings->nexusHandles.secondaryAudioDecoder)
            playback_ip->nexusHandles.secondaryAudioDecoder = pSettings->nexusHandles.secondaryAudioDecoder;
        if (pSettings->nexusHandles.stcChannel)
            playback_ip->nexusHandles.stcChannel = pSettings->nexusHandles.stcChannel;
        if (pSettings->nexusHandles.simpleStcChannel)
            playback_ip->nexusHandles.simpleStcChannel = pSettings->nexusHandles.simpleStcChannel;
    }
    if (pSettings->playPositionOffsetValid) {
        BDBG_MSG(("%s:%p: Update current position=%lu to new=%lu", __FUNCTION__, (void *)playback_ip, playback_ip->lastPosition , pSettings->playPositionOffsetInMs));
        playback_ip->lastPosition = pSettings->playPositionOffsetInMs;
        playback_ip->reOpenSocket = true;
    }

#ifdef B_HAS_HLS_PROTOCOL_SUPPORT
    if (pSettings->stopAlternateAudio) {
        if (playback_ip->openSettings.socketOpenSettings.protocol == B_PlaybackIpProtocol_eHttp && playback_ip->hlsSessionEnabled) {
            BDBG_MSG(("%s:%p: Calling B_PlaybackIp_HlsStopAlternateRendition", __FUNCTION__, (void *)playback_ip));
            B_PlaybackIp_HlsStopAlternateRendition(playback_ip);
        }
        rc = B_ERROR_SUCCESS;
    }
    else if (pSettings->startAlternateAudio) {
        if (pSettings->alternateAudio.pid == 0) {
            BDBG_ERR(("%s: alternateAudio.pid can't be 0 if alternate audio is enabled.", __FUNCTION__));
            return B_ERROR_INVALID_PARAMETER;
        }
        if (pSettings->alternateAudio.containerType == NEXUS_TransportType_eUnknown) {
            BDBG_ERR(("%s: alternateAudio.containerType can't be unknown if alternate audio is enabled.", __FUNCTION__));
            return B_ERROR_INVALID_PARAMETER;
        }
        if (pSettings->alternateAudio.groupId == NULL) {
            BDBG_ERR(("%s: alternateAudio.groupId can't be NULL if alternate audio is enabled.", __FUNCTION__));
            return B_ERROR_INVALID_PARAMETER;
        }
        if (pSettings->alternateAudio.language == NULL) {
            BDBG_ERR(("%s: alternateAudio.language can't be NULL if alternate audio is enabled.", __FUNCTION__));
            return B_ERROR_INVALID_PARAMETER;
        }
        if ((pSettings->nexusHandlesValid && pSettings->nexusHandles.playpump2 == NULL) || (playback_ip->nexusHandles.playpump2 == NULL)) {
            BDBG_ERR(("%s: playpump2 handle is NULL, it must be set for playing the alternate audio!", __FUNCTION__));
            return B_ERROR_INVALID_PARAMETER;
        }
        BDBG_MSG(("%s: Enabling AlternateAudio: language=%s groupId=%s pid=%d containerType=%d", __FUNCTION__,
                    pSettings->alternateAudio.language, pSettings->alternateAudio.groupId, pSettings->alternateAudio.pid, pSettings->alternateAudio.containerType));
        /* coverity[stack_use_local_overflow] */
        /* coverity[stack_use_overflow] */
        if (playback_ip->openSettings.socketOpenSettings.protocol == B_PlaybackIpProtocol_eHttp && playback_ip->hlsSessionEnabled) {
            if ((rc=B_PlaybackIp_HlsStartAlternateRendition(playback_ip, &pSettings->alternateAudio)) != B_ERROR_SUCCESS) {
                BDBG_ERR(("%s: Failed to enable Alternate Rendition for HLS protocol: rc=%d", __FUNCTION__, rc));
                return (rc);
            }
        }
        else {
            BDBG_ERR(("%s: Alternate Audio is only supported for HLS protocol: requestProtocol=%d", __FUNCTION__, playback_ip->openSettings.socketOpenSettings.protocol));
            return B_ERROR_INVALID_PARAMETER;
        }
    }
#endif
    BDBG_MSG(("%s:%p resumePsiMonitoring = %d", __FUNCTION__, (void *)playback_ip, pSettings->resumePsiMonitoring));
    if (pSettings->resumePsiMonitoring) {
        B_PlaybackIp_ResumePsiParsing(playback_ip->pPsiState);
    }
    BDBG_MSG(("%s: preChargeBuffer %d, ipState %d", __FUNCTION__, pSettings->preChargeBuffer, playback_ip->playback_state));
    switch (playback_ip->playback_state) {
    case B_PlaybackIpState_ePlaying:
        if (pSettings->preChargeBuffer) {
            if (!playback_ip->psi.avgBitRate) {
                BDBG_ERR(("%s: Can't enable Runtime Buffering since we dont know the avg stream bitrate", __FUNCTION__));
                return B_ERROR_INVALID_PARAMETER;
            }

            /* user set the preChargeBuffer flag, tell Http thread to start pre-charging work */
            playback_ip->preChargeBuffer = true;
            /* send event to let http thread know app would like it to start buffering */
            BKNI_SetEvent(playback_ip->newJobEvent);

            /* now wait for http thread to acknowledge start of buffering */
            rc = BKNI_WaitForEvent(playback_ip->preChargeBufferEvent, HTTP_PRE_CHARGE_EVENT_TIMEOUT);
            if (rc == BERR_TIMEOUT) {
                BDBG_WRN(("%s: timed out for pre-charging complete event", __FUNCTION__));
                return B_ERROR_UNKNOWN;
            } else if (rc!=0) {
                BDBG_WRN(("%s: got error while trying to wait for pre-charging complete event, rc %d", __FUNCTION__, rc));
                return B_ERROR_UNKNOWN;
            }
            BDBG_MSG(("%s: Enabled pre-charging of network buffer", __FUNCTION__));
        }
        /* can't change any settings in playing state, we ignore the settings */
        return B_ERROR_SUCCESS;
    case B_PlaybackIpState_eBuffering:
        if (!pSettings->preChargeBuffer) {
            /* we are currently pre-charging and user wants us to stop pre-charging */
            playback_ip->preChargeBuffer = false;
            rc = BKNI_WaitForEvent(playback_ip->preChargeBufferEvent, HTTP_PRE_CHARGE_EVENT_TIMEOUT);
            if (rc == BERR_TIMEOUT) {
                BDBG_WRN(("%s: timed out for pre-charging complete event", __FUNCTION__));
                return B_ERROR_UNKNOWN;
            } else if (rc!=0) {
                BDBG_WRN(("%s: got error while trying to wait for pre-charging complete event, rc %d", __FUNCTION__, rc));
                return B_ERROR_UNKNOWN;
            }
            BDBG_MSG(("%s: Stopped pre-charging of network buffer", __FUNCTION__));
        }
        /* can't change any settings in buffering state, we ignore the settings */
        return B_ERROR_SUCCESS;
    break;
    default:
        if (pSettings->preChargeBuffer) {
            BDBG_ERR(("%s: IP Playback Channel is not yet setup to be able to pre-charge network buffer (current state %d)\n", __FUNCTION__, playback_ip->playback_state));
            return B_ERROR_INVALID_PARAMETER;
        }
        /* states other than ePlaying or eBuffering, update settings */
        if (pSettings->useNexusPlaypump)
            playback_ip->useNexusPlaypump = true;
        BKNI_Memcpy(&playback_ip->settings, pSettings, sizeof(B_PlaybackIpSettings) );
        return B_ERROR_SUCCESS;
    }
}

/***************************************************************************
Summary:
This function detects if the stream is TTS or not
***************************************************************************/
B_PlaybackIpError B_PlaybackIp_DetectTts(
    B_PlaybackIpHandle playback_ip,
    bool *isTts
    )
{
    int i;
    int ret;
    fd_set fds;
    struct timeval socketTimeout;
    struct sockaddr *from;
    socklen_t   fromLen;
    int bytesRecv = 0;
    B_PlaybackIpError rc = B_ERROR_SUCCESS;
    int offset = 0;
    int pktCount = 0;
    int tsHdrCount = 0;
    char *buf = &playback_ip->temp_buf[0];

#ifdef B_HAS_NETACCEL
    STRM_SockRecvParams_t sockRecvParams;
    STRM_SockSetFilterState_t filterState;

    memset(&sockRecvParams, 0, sizeof(sockRecvParams));
    sockRecvParams.pktsPerRecv = 1;

    switch(playback_ip->protocol) {
    case B_PlaybackIpProtocol_eUdp:
    sockRecvParams.hdrOffset = sizeof(struct udp_hdr);
        break;
    case B_PlaybackIpProtocol_eRtp:
    case B_PlaybackIpProtocol_eRtpNoRtcp:
        sockRecvParams.hdrOffset = 0;
        buf = &playback_ip->temp_buf[8];
        break;
    default:
        sockRecvParams.hdrOffset = 0;
        break;
    }
    if (setsockopt(playback_ip->socketState.fd, SOCK_BRCM_DGRAM, STRM_SOCK_RECV_PARAMS, &sockRecvParams, sizeof(sockRecvParams)))
    {
        BDBG_ERR(("%s: setsockopt() ERROR:", __FUNCTION__));
        return B_ERROR_OS_ERROR;
    }
#endif

    BDBG_MSG(("%s: entering: %p", __FUNCTION__, (void *)playback_ip));

    if( IN_MULTICAST(ntohl(playback_ip->socketState.local_addr.sin_addr.s_addr)) ) {
        from = (struct sockaddr *) &playback_ip->socketState.remote_addr;
        fromLen = sizeof(struct sockaddr);
    }
    else {
        from = NULL;
        fromLen = 0;
    }

    for(i=0; ; i++) {
        if  (playback_ip->playback_state == B_PlaybackIpState_eStopping || playback_ip->playback_state == B_PlaybackIpState_eStopped) {
            /* user changed the channel, so return */
            BDBG_WRN(("%s: breaking out of main pre-charging loop due to state (%d) change", __FUNCTION__, playback_ip->playback_state));
            break;
        }
        if(i>=20) {
            rc = B_ERROR_UNKNOWN;
            BDBG_ERR(("%s: Failed to receive any data on this Live IP channel (playback_ip %p)", __FUNCTION__, (void *)playback_ip));
            goto error;
        }

        bytesRecv = recvfrom(playback_ip->socketState.fd,
                              (void *)playback_ip->temp_buf,
                              IP_MAX_PKT_SIZE,
                              0,
                              from,
                              &fromLen);

        if (bytesRecv==0) {
            BDBG_WRN(("server has closed connection"));
            goto wait;
        } else if (bytesRecv > 0) {
            /* got some data */
            goto done;
        } /* bytesRecv < 0 */

        if (errno == EINTR) {
            continue;
        }
#if defined(__vxworks)
        else if(errno != EWOULDBLOCK) {
            BDBG_ERR(("recvfrom error"));
        }
#else
        else if (errno != EAGAIN) {
            BDBG_ERR(("recvfrom error"));
            /* JJ - removed code: this is a normal condition that can occur when the server is overloaded */
        }
#endif

wait:
        /* no data received: wait for some */
        FD_ZERO(&fds);
        FD_SET(playback_ip->socketState.fd, &fds);
        socketTimeout.tv_sec = 0;
        socketTimeout.tv_usec = IP_RECV_TIMEOUT_USEC;
        /* BDBG_MSG_FLOW(("select")); */
        ret = select(playback_ip->socketState.fd+1, &fds, NULL, NULL, &socketTimeout);

        if (ret == 0) {
            BDBG_WRN(("%s: Receive timeout", __FUNCTION__));
        } else if (ret<0 && errno != EINTR) {
            BDBG_ERR(("select error"));
            rc = B_ERROR_UNKNOWN;
            goto error;
        }
    }

done:

/* JJ - debug */
#if 0
    BDBG_WRN(("0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x",
              buf[0],buf[1],buf[2],buf[3],buf[4],buf[5],buf[6],buf[7],buf[8],buf[9],buf[10],buf[11],buf[12],buf[13],buf[14],buf[15]));
#endif

    switch(playback_ip->protocol) {
    case B_PlaybackIpProtocol_eUdp:
        offset = 0;
        break;
    case B_PlaybackIpProtocol_eRtp:
    case B_PlaybackIpProtocol_eRtpNoRtcp:
    case B_PlaybackIpProtocol_eRtsp:
        {
            int cc;
            int ext_len = 0;
            bool ext = false;

            /* b_play_ip_parse_rtp_header(playback_ip->temp_buf); */
            if(((buf[0] & 0xC0) >> 6) != 2) goto error;
            if(buf[0] & 0x10) ext = true;
            cc = buf[0] & 0x0F;
            offset = 12+(cc*4);
            if(ext) {
                ext_len = ((buf[offset+2]<<8)+buf[offset+3]*4)+4;
                offset += ext_len;
            }
            BDBG_MSG(("RTP header size: %d", offset));
        }
        break;
    default:
        BDBG_WRN(("Bad protocol"));
        break;
    }

#ifdef B_HAS_NETACCEL
    switch(playback_ip->protocol) {
    case B_PlaybackIpProtocol_eUdp:
        break;
    case B_PlaybackIpProtocol_eRtp:
    case B_PlaybackIpProtocol_eRtpNoRtcp:
        bytesRecv -= 8;
        break;
    default:
        break;
    }
#endif
    playback_ip->rtpHeaderLength = offset;

    for(i=offset; i<bytesRecv; i+=188) {
        pktCount++;
        if(buf[i] == 0x47) {
            tsHdrCount++;
        }
    }
    if(tsHdrCount==pktCount) {
        /* it's a TS stream */
        BDBG_MSG(("it's a TS stream"));
        *isTts = false;
    }
    else {
        pktCount = 0;
        tsHdrCount = 0;
        for(i=offset; i<bytesRecv; i+=192) {
            pktCount++;
            if(buf[i+4] == 0x47) {
                tsHdrCount++;
            }
        }
        if(tsHdrCount==pktCount) {
            /* it's a TTS stream */
            BDBG_MSG(("it's a TTS stream"));
            *isTts = true;
        }
        else {
            BDBG_WRN(("TS/TTS auto-detect fail!"));
            *isTts = false;
        }
    }

error:

#ifdef B_HAS_NETACCEL
    /* now disable the net dma filter */
    memset(&filterState, 0, sizeof(filterState));
    filterState.filterEnable = 0;
    sockRecvParams.pktsPerRecv = 1;

    if (setsockopt(playback_ip->socketState.fd, SOCK_BRCM_DGRAM, STRM_SOCK_SET_FILTER_STATE, &filterState, sizeof(filterState)))
    {
        BDBG_ERR(("%s: setsockopt() ERROR:", __FUNCTION__));
        return B_ERROR_OS_ERROR;
    }

    /* TODO: may need to drain any remaining packets from the recvq or */
    /* throw away 1 queue size worth (~256) initial packets in B_PlaybackIp_RtpProcessing() */
#endif

    BDBG_MSG(("%s: TTS %d, rtpHeaderLength %d", __FUNCTION__, *isTts, playback_ip->rtpHeaderLength));
    return rc;
}

#endif /* LINUX || VxWorks */
