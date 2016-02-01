/***************************************************************************
 * (c) 2002-2015 Broadcom Corporation
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

#ifndef CHANNEL_BIP_H__
#define CHANNEL_BIP_H__

#ifdef PLAYBACK_IP_SUPPORT

#define CALLBACK_TUNER_LOCK_STATUS_BIP  "CallbackTunerLockStatusBip"

#include "config.h"
#include "channel.h"
#include "playback.h"
#include "timebase.h"
#include "bip.h"
#include "bip_player.h"
#include "bip_status.h"
#include "b_os_lib.h"

/*
 *  CModelChannelHTTP encapulates all the functionality specific to an HTTP/UDP/RTP/RTSP channel.
 *  This includes tune/untune from deveral IP sources.
 */

#ifdef __cplusplus
extern "C" {
#endif
typedef enum BMediaPlayerState
{
    BMediaPlayerState_eDisconnected,
    BMediaPlayerState_eWaitingForConnect,
    BMediaPlayerState_eWaitingForProbe,
    BMediaPlayerState_eWaitingForPrepare,
    BMediaPlayerState_eWaitingForStart,
    BMediaPlayerState_eStarted,
    BMediaPlayerState_ePaused,
    BMediaPlayerState_eMax
} BMediaPlayerState;

typedef enum BMediaPlayerAction
{
    BMediaPlayerAction_eEof,
    BMediaPlayerAction_eRestart,
    BMediaPlayerAction_ePlay,
    BMediaPlayerAction_eStop,
    BMediaPlayerAction_ePause,
    BMediaPlayerAction_eFf,
    BMediaPlayerAction_eRew,
    BMediaPlayerAction_ePlayAtRate,
    BMediaPlayerAction_eSeek,
    BMediaPlayerAction_eUnTune,
    BMediaPlayerAction_eStart,
    BMediaPlayerAction_eFrameAdvance,
    BMediaPlayerAction_eFrameRewind,
    BMediaPlayerAction_ePrepare,
    BMediaPlayerAction_eError,
    BMediaPlayerAction_eMax
} BMediaPlayerAction;

class CChannelBip : public CChannel
{
public:

    CChannelBip(
            const char *     strName,
            eBoardResource   type,
            CConfiguration * pCfg
            );
    CChannelBip(void);
    CChannelBip(CConfiguration * pCfg);
    CChannelBip(const CChannelBip & chHttp); /* copy constructor */
    CChannel * createCopy(CChannel * pChannel);
    ~CChannelBip(void);

    virtual eRet    tune(void * id, CConfig * pConfig, bool bWaitForLock, uint16_t index = ANY_INDEX);
    virtual eRet    unTune(CConfig * pResourceLibrary, bool bFullUnTune = false, bool bCheckInTuner = true);
    virtual eRet    readXML(MXmlElement * xmlElemChannel);
    virtual void    writeXML(MXmlElement * xmlElemChannel);
    virtual eRet    sessionOpen(void)  { return((eRet)0); }
    virtual eRet    sessionSetup(void) { return((eRet)0); }
    virtual MString getTimeString(void);

#if 0
    eRet readCompleteUrl(const char * pCompleteUrl);
    eRet writeCompleteUrl(MString &completeUrl);
#endif

    virtual bool operator ==(CChannel &other);

    eRet              initialize(PROGRAM_INFO_T * pProgramInfo);
    eRet              initialize(void);
    void              updateDescription(void);
    eRet              getChannelInfo(CHANNEL_INFO_T * pChanInfo, bool bScanning);
    eRet              getPsiInfo(void);
    eRet              openPids(CSimpleAudioDecode * pAudioDecode, CSimpleVideoDecode * pVideoDecode);
    eRet              start(CSimpleAudioDecode * pAudioDecode, CSimpleVideoDecode * pVideoDecode);
    eRet              play(void);
    eRet              pause(void);
    eRet              stop(void);
    eRet              playAtRate(void);
    eRet              frameAdvance(void);
    eRet              frameRewind(void);
    eRet              seek(bool relative, long int seekTime);
    eRet              applyTrickMode(void);
    eRet              trickmode(CPlaybackTrickData * pTrickModeData);
    eRet              setTrickModeRate(int trickModeRate);
    eRet              setTrickMode(bool fastFoward);
    bool              timelineSupport() { return(true); }
    unsigned int      getLastPosition(void);
    unsigned int      getCurrentPosition(void);
    void              setUrl(const char * pString);
    MString           getUrl()                                   { return(_url); }
    void              setAction(BMediaPlayerAction playerAction) { _playerCallbackAction = playerAction; }
    eRet              setState(BMediaPlayerState state);
    BMediaPlayerState getState(void)   { return(_playerState); }
    uint16_t          getProgram(void) { return(_programNumber); }
    void              setProgram(const char * str); /* program number can be set from url */
    void              setProgram(uint16_t program)
    {
        /* program number can be set locally.*/
        _programNumberValid = true;
        _programNumber      = program;
    }

    void       gotoBackGroundRecord(void);
    MString    getUrlPath(void);
    MString    getUrlQuery(void);
    void       setHost(const char * pString);
    MString    getHost(void);
    void       setPort(uint16_t nPort);
    uint16_t   getPort(void);
    BIP_Status mediaStateMachine(BMediaPlayerAction playerAction);
    void       dump(void);
    void       close(void);

protected:
    BIP_Status restartDecode(void);

    /* Command line options. */
    BIP_StringHandle   _pInterfaceName;
    BIP_StringHandle   _pUrl;
    MString            _url;
    BMediaPlayerAction _playerCallbackAction; /* only used when Start is invoked */

    /* Cached Handles. */
    BIP_PlayerHandle _pPlayer;

    BMediaPlayerState   _playerState;
    BIP_Status          _asyncApiCompletionStatus;
    BIP_CallbackDesc    _asyncCallbackDesc;  /* async completion callback. */
    BIP_MediaInfoHandle _pMediaInfo;

    bool     _programNumberValid;
    uint16_t _programNumber;

    /* needed for prepare */
    CSimpleVideoDecode * _pVideoDecode;
    CSimpleAudioDecode * _pAudioDecode;
};

#ifdef __cplusplus
}
#endif

#endif /* PLAYBACK_IP_SUPPORT */
#endif /* CHANNEL_BIP_H__ */