/******************************************************************************
 * (c) 2014 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 *****************************************************************************/
#ifndef BIP_RTSP_LIVEMEDIA_SESSION_H
#define BIP_RTSP_LIVEMEDIA_SESSION_H
#ifdef __cplusplus
extern "C" {
#endif

#include "bip_rtsp_response.h"
#include "bip_rtsp_session.h"

typedef struct BIP_RtspLiveMediaSession *BIP_RtspLiveMediaSessionHandle;

/**
 * Summary:
 * Get Default RtspLiveMediaSession settings
 **/
typedef struct BIP_RtspLiveMediaSessionCreateSettings
{
    int unused;
} BIP_RtspLiveMediaSessionCreateSettings;
void BIP_RtspLiveMediaSession_GetDefaultCreateSettings( BIP_RtspLiveMediaSessionCreateSettings *pSettings );

/**
 * Summary:
 * C Wrapper to LiveMedia BIP_RtspClient object
 *
 * Description:
 *
 **/
BIP_RtspLiveMediaSessionHandle BIP_RtspLiveMediaSession_CreateFromRequest( char *requestStr, void *lmRtspClientConnectionPtr, BIP_RtspLiveMediaSessionCreateSettings *pSettings );

#if 0
/* TODO: see if this API is needed for the client side */
BIP_RtspLiveMediaSessionHandle BIP_RtspLiveMediaSession_CreateFromUrl( char *url, BIP_RtspLiveMediaSessionCreateSettings *pSettings );
#endif

/**
 * Summary:
 * Rtsp Session settings to customize the RTSP Session
 **/
typedef struct BIP_RtspLiveMediaSessionSettings
{
    BIP_CallbackDesc messageReceivedCallback; /* callback to indicate the next RTSP message from peer */
    BIP_CallbackDesc igmpMembershipReportCallback; /* callback to indicate the an igmpMembershipReport callback from peer */
    /* impl. note: BIP eventLoop reads the full RTSP Request Message from the RtspLiveMediaSession and then invokes this messageReceivedCallback callback */
    BIP_CallbackDesc errorCallback;          /* callback to indicate on RTSP socket */
    BIP_CallbackDesc endOfStreamingCallback; /* callback to indicate that we have reached end of file on the file being streamed on this RtspLiveMediaSession */
    BIP_CallbackDesc peerClosedCallback;     /* callback to indicate peer has done a normal close on RTSP socket */
} BIP_RtspLiveMediaSessionSettings;

void      BIP_RtspLiveMediaSession_GetSettings( BIP_RtspLiveMediaSessionHandle socket, BIP_RtspLiveMediaSessionSettings *pSettings );
BIP_Status BIP_RtspLiveMediaSession_SetSettings( BIP_RtspLiveMediaSessionHandle socket, BIP_RtspLiveMediaSessionSettings *pSettings );
BIP_Status BIP_RtspLiveMediaSession_SendResponse( BIP_RtspLiveMediaSessionHandle rtspLmSession, BIP_RtspResponseStatus rtspStatus );
BIP_Status BIP_RtspLiveMediaSession_SendResponseUsingRequest( BIP_RtspLiveMediaSessionHandle rtspLmSession, BIP_RtspResponseStatus rtspStatus, char *pRequestBuffer );
BIP_Status BIP_RtspLiveMediaSession_ReportLockStatus( BIP_RtspLiveMediaSessionHandle hRtspLmSession, bool bLockStatus );

#if 0
/**
 * Summary:
 * Rtsp Session API to recv a RTSP Message on RTSP Session
 **/
BIP_Status BIP_RtspLiveMediaSession_RecvRequest( BIP_RtspLiveMediaSessionHandle socket, BIP_RtspRequestHandle rtspRequest );
BIP_Status BIP_RtspLiveMediaSession_RecvResponse( BIP_RtspLiveMediaSessionHandle socket, BIP_RtspResponseHandle rtspResponse );

/**
 * Summary:
 * Rtsp Session API to send a RTSP Message on RTSP Session
 **/
BIP_Status BIP_RtspLiveMediaSession_SendRequest( BIP_RtspLiveMediaSessionHandle socket, BIP_RtspRequestHandle rtspRequest );
#endif

/**
 * Summary:
 * Destroy socket
 *
 * Description:
 **/
void BIP_RtspLiveMediaSession_Destroy( BIP_RtspLiveMediaSessionHandle rtspLmSession );
void BIP_RtspLiveMediaRequest_Parse( char *pBuffer, unsigned bufferLength );

/**
 * Summary:
 * Copy current RTSP Message
 *
 * Description:
 *
 * BIP_RtspSession class invokes this API to copy the currently received RTSP Message into its buffer so that it can
 * queue up this message request for app's consumption.
 *
 **/
void BIP_RtspLiveMediaSession_CopyMessage( BIP_RtspLiveMediaSessionHandle rtspLmSession, char *pBuffer );

#if NEXUS_HAS_FRONTEND
#include "nexus_frontend.h"

/**
 * Summary:
 * Parse Satellite Settings
 *
 * Description:
 *
 **/
BIP_Status
BIP_RtspLiveMediaSession_ParseSatelliteSettings(
    BIP_RtspLiveMediaSessionHandle   hRtspLmSession,
    NEXUS_FrontendSatelliteSettings *pSatelliteSettings,
    NEXUS_FrontendDiseqcSettings *pDiseqcSettings,
    BIP_AdditionalSatelliteSettings *pAddSatelliteSettings
    );
#endif

#define BIP_MAX_PIDS_PER_PROGRAM 50

typedef struct BIP_RtspLiveMediaSessionSatelliteSettings {
    int               streamId;
    unsigned long int owner; /* sessionId */
    unsigned          freq;   /*in mhz*/
    int               src;
    int               fe;
    unsigned          fec;
    unsigned          sr;  /* in kSymb/s */
    char              pol[2];  /* polarisation: (h)orizontal, (v)ertical, (l)eft, (r)ight  */
    char              msys[6];  /*modulation system: dvbs or dvbs2 */
    char              mtype[5]; /*modullation type:  qpsk, 8psk.  Not required for DVB-S*/
    float             ro;       /* Roll-off: "0.35", "0.25", "0.20". Not required for DVB-S */
    char              plts[4];   /*pilot tones: on,off.  Not required for DVB-S*/
    int               pidListCount;
    int               pidList[BIP_MAX_PIDS_PER_PROGRAM]; /*ie.  PAT, PMT, VID, AUD */
    int               addPidListCount;
    int               addPidList[BIP_MAX_PIDS_PER_PROGRAM];
    int               delPidListCount;
    int               delPidList[BIP_MAX_PIDS_PER_PROGRAM];
    int               ttl;
    int               level;
    int               lock;
    int               quality;
    int               pilots;
    int               pol_int;
    bool pilotToneSpecified; /* true if client specified a value for the pilotTone */
    bool pilotTone;          /* client can set this to be on or off */
} BIP_RtspLiveMediaSessionSatelliteSettings;

int BIP_RtspLiveMediaSession_GetSatelliteSettings( BIP_RtspLiveMediaSessionHandle hRtspLmSession,
    BIP_RtspLiveMediaSessionSatelliteSettings                    *pSettings );

BIP_Status BIP_RtspLiveMediaSession_GetTransportStatus( BIP_RtspLiveMediaSessionHandle hRtspLmSession,
    BIP_RtspTransportStatus                                  *pSettings );
BIP_Status  BIP_RtspLiveMediaSession_GetPids(BIP_RtspLiveMediaSessionHandle hRtspLmSession,  BIP_PidInfo* pPids);
#ifdef __cplusplus
}
#endif

#endif /* BIP_RTSP_LIVEMEDIA_SESSION_H */
