/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#ifndef CONTROL2_H__
#define CONTROL2_H__

#include "atlas_cfg.h"
#include "view.h"
#include "config.h"
#include "model.h"
#include "timer.h"
#include "controller.h"
#include "channelmgr.h"
#if DVR_LIB_SUPPORT
#include "dvrmgr.h"
#endif
#include "channel.h"
#include "power.h"
#include "audio_decode.h"

class CPlayback;
class CRecord;

#ifdef __cplusplus
extern "C" {
#endif

class CControl : public CController
{
public:
    CControl(const char * strName);
    virtual ~CControl(void);

    virtual eRet       setOptimalVideoFormat(CDisplay * pDisplay, CSimpleVideoDecode * pVideoDecode);
    virtual eRet       setComponentDisplay(NEXUS_VideoFormat vFormat);
    virtual eRet       setVideoFormat(NEXUS_VideoFormat videoFormat);
    virtual eRet       setDeinterlacer(bool bDeinterlacer);
    virtual eRet       setMpaaDecimation(bool bMpaaDecimation);
    virtual eRet       setContentMode(NEXUS_VideoWindowContentMode contentMode);
    virtual int32_t    getVolume(void);
    virtual eRet       setVolume(int32_t level);
    virtual bool       getMute(void);
    virtual eRet       setMute(bool muted);
    virtual eRet       applyVbiSettings(uint32_t nDisplayIndex = 1);
    virtual eRet       showPip(bool bShow = true);
    virtual eRet       swapPip(void);
    virtual eRet       setPowerMode(ePowerMode mode);
    virtual ePowerMode getPowerMode(void);
    virtual eRet       setWindowGeometry(void);

    virtual eRet    connectDecoders(
            CSimpleVideoDecode * pVideoDecode,
            CSimpleAudioDecode * pAudioDecode,
            uint32_t             width,
            uint32_t             height,
            CPid *               pVideoPid,
            eWindowType          windowType);
    virtual void disconnectDecoders(eWindowType winType);
    virtual eRet startDecoders(
            CSimpleVideoDecode * pVideoDecode,
            CPid *               pVideoPid,
            CSimpleAudioDecode * pAudioDecode,
            CPid *               pAudioPid,
            CStc *               pStc);
    virtual eRet stopDecoders(CSimpleVideoDecode * pVideoDecode, CSimpleAudioDecode * pAudioDecode);
    virtual bool checkPower(void) {return false;}

    eRet            initialize(void * id, CConfig * pConfig, CChannelMgr * pChannelMgr, CWidgetEngine * pWidgetEngine);
    eRet            uninitialize();
    void            processNotification(CNotification & notification);
    eRet            processKeyEvent(CRemoteEvent * pRemoteEvent);
    void            onIdle(void);
    CWidgetEngine * getWidgetEngine(void) { return(_pWidgetEngine); }
    eRet            tuneChannel(CChannel * pChannel = NULL, eWindowType windowType = eWindowType_Max, unsigned tunerIndex = ANY_INDEX);
    eRet            unTuneChannel(CChannel * pChannel = NULL, bool bFullUnTune = false, eWindowType windowType = eWindowType_Max);
    eRet            unTuneAllChannels(void);
    eRet            decodeChannel(CChannel * pChannel, eWindowType windowType);
    eRet            stopAllPlaybacks(void);
    eRet            stopAllRecordings(void);
    eRet            stopAllEncodings(void);
    eRet            tuneDeferredChannel(eWindowType windowType = eWindowType_Max);
    eRet            tuneLastChannel(void);
    eRet            addChannelToChList(CChannel * pChannel);
    eRet            getChannelStats(void);
    eRet            channelUp(void);
    eRet            channelDown(void);
    eRet tenKey(eKey key);
    eRet playbackStart(const char * fileName, const char * indexName, const char * path, eWindowType windowType = eWindowType_Max);
    eRet playbackStart(CVideo * pVideo, eWindowType windowType = eWindowType_Max);
    eRet playbackStop(const char * MediaName = NULL, eWindowType windowType = eWindowType_Max, bool bTuneLast = true);
    eRet recordStart(CRecordData * pRecordData = NULL);
    eRet recordStop(CChannel * pChannel = NULL);
    eRet encodeStart(const char * fileName = NULL, const char * path = NULL);
    eRet encodeStop(CChannel * pChannel = NULL);
    eRet setAudioProgram(unsigned pid);
    eRet setAudioProcessing(eAudioProcessing audioProcessing);
    eRet          ipServerStart(void);
    eRet          ipServerStop(void);
    eRet          setSpdifInput(eSpdifInput spdifInput);
    eRet          setHdmiAudioInput(eHdmiAudioInput hdmiInput);
    eRet          setAudioDownmix(eAudioDownmix audioDownmix);
    eRet          setAudioDualMono(eAudioDualMono audioDualMono);
    eRet          setDolbyDRC(eDolbyDRC dolbyDRC);
    eRet          setDolbyDialogNorm(bool dolbyDialogNorm);
    eRet          setColorSpace(NEXUS_ColorSpace colorSpace);
    eRet          setColorDepth(uint8_t colorDepth);
    eRet          setBoxDetect(bool bBoxDetect);
    eRet          setAspectRatio(NEXUS_DisplayAspectRatio aspectRatio);
    eRet          setAutoVideoFormat(bool bAutoVideoFormat);
    eRet          showWindowType(eWindowType windowType, bool bShow = true);
    void          setModel(CModel * pModel)                { _pModel = pModel; }
    void          setChannelMgr(CChannelMgr * pChannelMgr) { _pChannelMgr = pChannelMgr; }
    CChannelMgr * getChannelMgr(void)                      { return(_pChannelMgr); }
    CConfiguration * getCfg(void) { return(_pCfg); }
    void             updatePlaybackList();
    void       dumpPlaybackList(bool bForce = false);
    void       addView(CView * pView, const char * name);
    CView *    findView(const char * name);
    void       removeView(CView * pView);
    bool       validateNotification(CNotification & notification, eMode mode);
#ifdef CPUTEST_SUPPORT
    eRet setCpuTestLevel(int nLevel);
#endif
#if RF4CE_SUPPORT
    eRet displayRf4ceRemotes(void);
    eRet addRf4ceRemote(const char * remote_name);
    eRet removeRf4ceRemote(int pairingRefNum);
#endif
#if DVR_LIB_SUPPORT
    void      setDvrMgr(CDvrMgr * pDvrMgr) { _pDvrMgr = pDvrMgr; }
    CDvrMgr * getDvrMgr(void)              { return(_pDvrMgr); }
#endif
#if NEXUS_HAS_FRONTEND
    void initializeTuners();
    eRet scanTuner(CTunerScanData * pScanData);
#endif
#if HAS_GFX_NL_LUMA_RANGE_ADJ
    eRet setGraphicsDynamicRange(CChannel * pChannel);
#endif
#if HAS_VID_NL_LUMA_RANGE_ADJ
    eRet setHdmiOutputDynamicRange(eDynamicRange dynamicRange = eDynamicRange_SDR);
    eRet videoDecodeUpdatePlm(CSimpleVideoDecode * pVideoDecode);
#endif

protected:
    class CViewListNode
    {
public:
        CViewListNode(
                CView *      pView,
                const char * name
                ) :
            _pView(pView),
            _strName(name)
        {}

public:
        CView * _pView;
        MString _strName;
    };

protected:
    /* Id belongs to the Atlas instance */
    void *        _id;
    CModel *      _pModel;
    CConfig *     _pConfig;
    CChannelMgr * _pChannelMgr;
#if DVR_LIB_SUPPORT
    CDvrMgr * _pDvrMgr;
#endif
    CConfiguration *      _pCfg;
    CWidgetEngine *       _pWidgetEngine;
    CTimer                _deferredChannelUpDownTimer;
    CTimer                _deferredChannel10KeyTimer;
    CTimer                _deferredChannelPipTimer;
    CTimer                _tunerLockCheckTimer;
    CTimer                _powerOnTimer;
    MList <CViewListNode> _viewList;
    MList <CChannel>      _recordingChannels;
    MList <CChannel>      _encodingChannels;
    bool                  _bIgnoreNextKeypress;
#if HAS_VID_NL_LUMA_RANGE_ADJ
    MList <CSimpleVideoDecode> _plmVideoDecodeList;
    CTimer _timerPlmVerify;
#endif
};

#ifdef __cplusplus
}
#endif

#endif /* CONTROL2_H__ */