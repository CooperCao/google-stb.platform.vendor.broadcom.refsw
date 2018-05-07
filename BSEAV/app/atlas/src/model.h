/******************************************************************************
 * Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#ifndef MODEL_H__
#define MODEL_H__

#include "model_types.h"
#include "mvc.h"
#include "nexus_ir_input.h"
#if NEXUS_HAS_UHF_INPUT
#include "nexus_uhf_input.h"
#endif
#include "remote.h"
#include "cec_remote.h"
#include "nexus_simple_video_decoder_server.h"
#include "nexus_simple_audio_decoder_server.h"
#include "nexus_simple_audio_playback_server.h"
#include "nexus_simple_encoder_server.h"
#include "video_decode_types.h"

class CChannelMgr;
#if DVR_LIB_SUPPORT
class CDvrMgr;
class CTsb;
#endif
class CDisplay;
class CGraphics;
class CStc;
class CVideoDecode;
class CStillDecode;
class CThumb;
class CSimpleVideoDecode;
class CAudioDecode;
class CSimpleAudioDecode;
class CSimplePcmPlayback;
class CIrRemote;
#if NEXUS_HAS_CEC
class CCecRemote;
#endif
class CPlayback;
class CRecord;
class CPower;
#if NEXUS_HAS_VIDEO_ENCODER
class CEncode;
#endif
class CPlaybackList;
class CChannel;
class COutput;
#ifdef MPOD_SUPPORT
class CCablecard;
#endif
#ifdef DCC_SUPPORT
class CClosedCaption;
#endif
class CServerMgr;
class CAutoDiscoveryServer;
class CAutoDiscoveryClient;
class CPlaylistDb;
class CAudioCapture;
#ifdef SNMP_SUPPORT
class CSnmp;
#endif
class MXmlElement;
#ifdef CPUTEST_SUPPORT
class CCpuTest;
#endif
#ifdef WPA_SUPPLICANT_SUPPORT
class CWifi;
#endif
class CConfig;

#ifdef __cplusplus
extern "C" {
#endif

class CModel : public CMvcModel
{
public:
    CModel(const char * strName);
    ~CModel();

    void                                 setConfig(CConfig * pConfig) { _pConfig = pConfig; }
    CConfig *                            getConfig(void) { return(_pConfig); }
    void                                 setChannelMgr(CChannelMgr * pChannelMgr) { _pChannelMgr = pChannelMgr; }
    CChannelMgr *                        getChannelMgr(void) { return(_pChannelMgr); }
    void                                 addDisplay(CDisplay * pDisplay);
    void                                 removeDisplay(CDisplay * pDisplay);
    uint32_t                             numDisplays(void) { return(_displayList.total()); }
    CDisplay *                           getDisplay(uint32_t num = 0);
    MList<CDisplay> *                    getDisplayList(void) { return(&_displayList); }
    void                                 addGraphics(CGraphics * pGraphics) { _pGraphics = pGraphics; }
    void                                 removeGraphics(CGraphics * pGraphics) { BSTD_UNUSED(pGraphics); _pGraphics = NULL; }
    CGraphics *                          getGraphics(void) { return(_pGraphics); }
    void                                 addStc(CStc * pStc, eWindowType windowType = eWindowType_Main ) { _pStc[windowType] = pStc; }
    void                                 removeStc(eWindowType windowType = eWindowType_Main) { _pStc[windowType] = NULL; }
    CStc *                               getStc(eWindowType windowType = eWindowType_Max) { return((eWindowType_Max == windowType) ? _pStc[_fullScreenWindowType] : _pStc[windowType]); }
#if NEXUS_HAS_CEC
	void                                 addCecRemote(CCecRemote * pCecRemote) { _pCecRemote = pCecRemote; }
    void                                 removeCecRemote(CCecRemote * pCecRemote) { BSTD_UNUSED(pCecRemote); _pCecRemote = NULL; }
    CCecRemote *                         getCecRemote(void) { return(_pCecRemote); }
#endif
    void                                 addVideoDecode(CVideoDecode * pVideoDecode) { _pVideoDecode = pVideoDecode; }
    void                                 removeVideoDecode(CVideoDecode * pVideoDecode) { BSTD_UNUSED(pVideoDecode); _pVideoDecode = NULL; }
    CVideoDecode *                       getVideoDecode(void) { return(_pVideoDecode); }
    void                                 addStillDecode(CStillDecode * pStillDecode) { _pStillDecode = pStillDecode; }
    void                                 removeStillDecode(CStillDecode * pStillDecode) { BSTD_UNUSED(pStillDecode); _pStillDecode = NULL; }
    CStillDecode *                       getStillDecode(void) { return(_pStillDecode); }
    void                                 setThumbExtractor(CThumb * pThumbExtractor) { _pThumbExtractor = pThumbExtractor; }
    CThumb *                             getThumbExtractor(void) { return(_pThumbExtractor); }
    void                                 addSimpleVideoDecode(CSimpleVideoDecode * pSimpleVideoDecode, eWindowType windowType = eWindowType_Main ) { _pSimpleVideoDecode[windowType] = pSimpleVideoDecode; }
    void                                 removeSimpleVideoDecode(eWindowType windowType = eWindowType_Main) { _pSimpleVideoDecode[windowType] = NULL;  }
    CSimpleVideoDecode *                 getSimpleVideoDecode(eWindowType windowType = eWindowType_Max) { return((eWindowType_Max == windowType) ? _pSimpleVideoDecode[_fullScreenWindowType] : _pSimpleVideoDecode[windowType]); }
    void                                 addAudioDecode(CAudioDecode * pAudioDecode) { _pAudioDecode = pAudioDecode; }
    void                                 removeAudioDecode(CAudioDecode * pAudioDecode) { BSTD_UNUSED(pAudioDecode); _pAudioDecode = NULL; }
    CAudioDecode *                       getAudioDecode(void) { return(_pAudioDecode); }
    void                                 addSimpleAudioDecode(CSimpleAudioDecode * pSimpleAudioDecode, eWindowType windowType = eWindowType_Main ) { _pSimpleAudioDecode[windowType] = pSimpleAudioDecode; }
    void                                 removeSimpleAudioDecode(eWindowType windowType = eWindowType_Main) { _pSimpleAudioDecode[windowType] = NULL; }
    CSimpleAudioDecode *                 getSimpleAudioDecode(eWindowType windowType = eWindowType_Max) { return((eWindowType_Max == windowType) ? _pSimpleAudioDecode[_fullScreenWindowType] : _pSimpleAudioDecode[windowType]); }
    void                                 addSimplePcmPlayback(CSimplePcmPlayback * pSimplePcmPlayback) { _pSimplePcmPlayback = pSimplePcmPlayback; }
    void                                 removeSimplePcmPlayback(void) { _pSimplePcmPlayback = NULL; }
    CSimplePcmPlayback *                 getSimplePcmPlayback(void) { return(_pSimplePcmPlayback); }
    void                                 swapDecodeVideoWindows(void);
    void                                 swapSimpleAudioDecode(void);
    void                                 addAudioOutput(COutput * pAudioOutput);
    COutput *                            getAudioOutput(eBoardResource resourceType);
    void                                 removeAudioOutput(COutput * pAudioOutput);
    MList<COutput> *                     getAudioOutputList(void) { return(&_audioOutputList); }
    void                                 addIrRemote(CIrRemote * pIrRemote);
    void                                 removeIrRemote(CIrRemote * pIrRemote);
    CIrRemote *                          getIrRemote(NEXUS_IrInputMode mode = NEXUS_IrInputMode_eMax);
    CPlayback *                          getPlayback(eWindowType windowType = eWindowType_Max) { return((eWindowType_Max == windowType) ? _currentPlayback[_fullScreenWindowType] : _currentPlayback[windowType]); }
    void                                 setPlayback(CPlayback * pPlayback = NULL, eWindowType windowType = eWindowType_Max);
    void                                 addRecord(CRecord * pRecord) { _pRecord = pRecord; }
    void                                 removeRecord(CRecord * pRecord) { BSTD_UNUSED(pRecord); _pRecord = NULL; }
    CRecord *                            getRecord(void) { return(_pRecord); }
    void                                 setPlaybackList(CPlaybackList * pPlaybackList) { _pPlaybackList = pPlaybackList; }
    CPlaybackList *                      getPlaybackList(void) { return(_pPlaybackList); }
    eMode                                getMode(eWindowType windowType = eWindowType_Max) { return((eWindowType_Max == windowType) ? _mode[_fullScreenWindowType] : _mode[windowType]); }
    void                                 setMode(eMode mode = eMode_Max, eWindowType windowType = eWindowType_Max);
    void                                 setId(void * id) { _id = id; }
    void *                               getId(void) { return(_id); }
    bool                                 getPipState(void) { return(_bPip); }
    void                                 setPipState(bool bPip);
    bool                                 getPipEnabled(void) { return(_bPipEnabled); }
    void                                 setPipEnabled(bool bEnabled) { _bPipEnabled = bEnabled; }
    bool                                 getIpClientTranscodeEnabled(void) { return(_bipTranscodeEnabled); }
    void                                 setIpClientTranscodeEnabled(bool bEnabled) { _bipTranscodeEnabled = bEnabled; }
    int                                  getIpClientTranscodeProfile(void) { return(_ipTranscodeProfile); }
    void                                 setIpClientTranscodeProfile(int xcodeProfile) { _ipTranscodeProfile = xcodeProfile; }
    bool                                 getPipSwapState(void) { return(_bPipSwapped); }
    void                                 setPipSwapState(bool bPipSwapped) { _bPipSwapped = bPipSwapped; }
    bool                                 getScanStartState(void) { return(_bScanStarted); }
    void                                 setScanStartState(bool bStarted);
    bool                                 getScanSaveOffer(void) { return(_bScanSaveOffer); }
    void                                 setScanSaveOffer(bool bOffer) { _bScanSaveOffer = bOffer; }
    CChannel *                           getCurrentChannel(eWindowType windowType = eWindowType_Max) { return((eWindowType_Max == windowType) ? _currentChannel[_fullScreenWindowType] : _currentChannel[windowType]); }
    void                                 setCurrentChannel(CChannel * pChannel = NULL, eWindowType windowType = eWindowType_Max);
    CChannel *                           getChannelTuneInProgress(eWindowType windowType = eWindowType_Max) { return((eWindowType_Max == windowType) ? _channelTuneInProgress[_fullScreenWindowType] : _channelTuneInProgress[windowType]); }
    void                                 setChannelTuneInProgress(CChannel * pChannel = NULL, eWindowType windowType = eWindowType_Max);
    CChannel *                           getLastTunedChannel(eWindowType windowType = eWindowType_Max) { return((eWindowType_Max == windowType) ? _lastChannel[_fullScreenWindowType] : _lastChannel[windowType]); }
    void                                 restoreLastTunedChannelPowerSave(void) { _lastChannel[_fullScreenWindowType] = _lastChannelPowerSave; }
    void                                 saveLastTunedChannelPowerSave(void) { _lastChannelPowerSave = _lastChannel[_fullScreenWindowType]; }
    void                                 resetChannelHistory(void);
    unsigned                             numMatchingCurrentChannels(CChannel * pChannel, eWindowType excluded = eWindowType_Max);
    eWindowType                          getFullScreenWindowType(void) { return(_fullScreenWindowType); }
    void                                 setFullScreenWindowType(eWindowType windowType);
    eWindowType                          getPipScreenWindowType(void);
    MString                              getDeferredChannelNum(eWindowType windowType = eWindowType_Max) { return((eWindowType_Max == windowType) ? _deferredChannelNum[_fullScreenWindowType] : _deferredChannelNum[windowType]); }
    CChannel *                           getDeferredChannel(eWindowType windowType = eWindowType_Max) { return((eWindowType_Max == windowType) ? _pDeferredChannel[_fullScreenWindowType] : _pDeferredChannel[windowType]); }
    void                                 setDeferredChannelNum(const char * strChNum, CChannel * pCh = NULL, eWindowType windowType = eWindowType_Max);
    eRet                                 sendGlobalKeyDown(eKey * pKey);
    void                                 setServerMgr(CServerMgr * pServerMgr) { _pServerMgr = pServerMgr; }
    CServerMgr *                         getServerMgr(void) { return(_pServerMgr); }
    void                                 setAutoDiscoveryServer(CAutoDiscoveryServer * pAutoDiscoveryServer) { _pAutoDiscoveryServer = pAutoDiscoveryServer; }
    CAutoDiscoveryServer *               getAutoDiscoveryServer(void) { return(_pAutoDiscoveryServer); }
    void                                 setAutoDiscoveryClient(CAutoDiscoveryClient * pAutoDiscoveryClient) { _pAutoDiscoveryClient = pAutoDiscoveryClient; }
    CAutoDiscoveryClient *               getAutoDiscoveryClient(void) { return(_pAutoDiscoveryClient); }
    void                                 setPlaylistDb(CPlaylistDb * pPlaylistDb) { _pPlaylistDb = pPlaylistDb; }
    CPlaylistDb *                        getPlaylistDb(void) { return(_pPlaylistDb); }
    void                                 setAudioCapture(CAudioCapture * pAudioCapture) { _pAudioCapture = pAudioCapture; }
    CAudioCapture *                      getAudioCapture(void) { return(_pAudioCapture); }
    unsigned                             getConnectId(eWindowType windowType = eWindowType_Max) { return((eWindowType_Max == windowType) ? _connectId[_fullScreenWindowType] : _connectId[windowType]); }
    void                                 setConnectId(uint32_t connectId, eWindowType windowType = eWindowType_Max);
    NEXUS_SimpleVideoDecoderServerHandle getSimpleVideoDecoderServer(void) { return(_simpleVideoDecoderServer); }
    NEXUS_SimpleAudioDecoderServerHandle getSimpleAudioDecoderServer(void) { return(_simpleAudioDecoderServer); }
    NEXUS_SimpleAudioPlaybackServerHandle getSimpleAudioPlaybackServer(void) { return(_simpleAudioPlaybackServer); }
    NEXUS_SimpleEncoderServerHandle      getSimpleEncoderServer(void) { return(_simpleEncoderServer); }
    eDynamicRange                        getLastDynamicRange(void) { return(_dynamicRangeLast); }
    void                                 setLastDynamicRange(eDynamicRange dynamicRange) { _dynamicRangeLast = dynamicRange; }

#ifdef WPA_SUPPLICANT_SUPPORT
    CWifi *                              getWifi(void) { return(_pWifi); }
    void                                 setWifi(CWifi * pWifi) { _pWifi = pWifi; }
#endif
#ifdef CPUTEST_SUPPORT
    CCpuTest *                           getCpuTest(void) { return(_pCpuTest); }
    void                                 setCpuTest(CCpuTest * pCpuTest) { _pCpuTest = pCpuTest; }
#endif
#if RF4CE_SUPPORT
    void                                 addRf4ceRemote(CRf4ceRemote * pRf4ceRemote);
    void                                 removeRf4ceRemote(CRf4ceRemote * pRf4ceRemote);
    CRf4ceRemote *                       getRf4ceRemote(void);
#endif
#if NETAPP_SUPPORT
    void                                 addBluetoothRemote(CBluetoothRemote * pBluetoothRemote);
    void                                 removeBluetoothRemote(CBluetoothRemote * pBluetoothRemote);
    CBluetoothRemote *                   getBluetoothRemote(void);
#endif
#if NEXUS_HAS_UHF_INPUT
    void                                 addUhfRemote(CUhfRemote * pUhfRemote);
    void                                 removeUhfRemote(CUhfRemote * pUhfRemote);
    CUhfRemote *                         getUhfRemote(void);
#endif
#if NEXUS_HAS_VIDEO_ENCODER
    void                                 addEncode(CEncode * pEncode) { _pEncode = pEncode; }
    void                                 removeEncode(CEncode * pEncode) { BSTD_UNUSED(pEncode); _pEncode = NULL; }
    CEncode *                            getEncode(void) { return(_pEncode); }
#endif
#ifdef MPOD_SUPPORT
    void                                 addCableCard(CCablecard * pCableCard) { _pCableCard = pCableCard; }
    void                                 removeCableCard(CCablecard * pCableCard) { BSTD_UNUSED(pCableCard); _pCableCard = NULL; }
    CCablecard *                         getCableCard(void) { return(_pCableCard); }
#endif
#ifdef DCC_SUPPORT
    void                                 addClosedCaption(CClosedCaption * pClosedCaption) { _pClosedCaption = pClosedCaption; }
    void                                 removeClosedCaption(CClosedCaption * pClosedCaption) { BSTD_UNUSED(pClosedCaption); _pClosedCaption = NULL; }
    CClosedCaption *                     getClosedCaption(void) { return(_pClosedCaption); }
#endif
#ifdef SNMP_SUPPORT
    void                                 addSnmp(CSnmp * pSnmp) { _pSnmp = pSnmp; }
    void                                 removeSnmp(CSnmp * pSnmp) { BSTD_UNUSED(pSnmp); _pSnmp = NULL; }
    CSnmp *                              getSnmp(void) { return(_pSnmp); }
#endif
#if DVR_LIB_SUPPORT
    void                                 setDvrMgr(CDvrMgr * pDvrMgr) { _pDvrMgr = pDvrMgr; }
    CDvrMgr *                            getDvrMgr(void) { return(_pDvrMgr); }
    CTsb *                               getTsb(eWindowType windowType = eWindowType_Max) { return((eWindowType_Max == windowType) ? _currentTsb[_fullScreenWindowType] : _currentTsb[windowType]); }
    void                                 setTsb(CTsb * pTsb = NULL, eWindowType windowType = eWindowType_Max);
#endif /* if DVR_LIB_SUPPORT */

protected:
    CConfig *                            _pConfig;
    CChannelMgr *                        _pChannelMgr;
#if DVR_LIB_SUPPORT
    CDvrMgr *                            _pDvrMgr;
#endif
    MList<CDisplay>                      _displayList;
    CGraphics *                          _pGraphics;
#if NEXUS_HAS_CEC
	CCecRemote *						 _pCecRemote;
#endif
    eWindowType                          _fullScreenWindowType;
    CVideoDecode *                       _pVideoDecode;
    CStillDecode *                       _pStillDecode;
    CThumb *                             _pThumbExtractor;
    CAudioDecode *                       _pAudioDecode;
    MList<COutput>                       _audioOutputList;
    MList<CIrRemote>                     _irRemoteList;
#if RF4CE_SUPPORT
    MList<CRf4ceRemote>                  _rf4ceRemoteList;
#endif
#if NETAPP_SUPPORT
    MList<CBluetoothRemote>              _bluetoothRemoteList;
#endif
#if NEXUS_HAS_UHF_INPUT
    MList<CUhfRemote>                    _uhfRemoteList;
#endif
    CRecord *                            _pRecord;
#if NEXUS_HAS_VIDEO_ENCODER
    CEncode *                            _pEncode;
#endif
    CPlaybackList *                      _pPlaybackList;
    void *                               _id;
    MString                              _deferredChannelNum[eWindowType_Max];
    CChannel *                           _pDeferredChannel[eWindowType_Max];
    bool                                 _bPip;
    bool                                 _bPipEnabled;
    bool                                 _bPipSwapped;
    bool                                 _bScanSaveOffer;
    bool                                 _bipTranscodeEnabled;
    int                                  _ipTranscodeProfile;
    eMode                                _mode[eWindowType_Max];
    CSimpleVideoDecode *                 _pSimpleVideoDecode[eWindowType_Max];
    CSimpleAudioDecode *                 _pSimpleAudioDecode[eWindowType_Max];
    CSimplePcmPlayback *                 _pSimplePcmPlayback;
    CStc *                               _pStc[eWindowType_Max];
    CChannel *                           _currentChannel[eWindowType_Max];
    CChannel *                           _channelTuneInProgress[eWindowType_Max];
    CChannel *                           _lastChannel[eWindowType_Max];
    CChannel *                           _lastChannelPowerSave;
    CPlayback *                          _currentPlayback[eWindowType_Max];
    CPower *                             _pPower;
#if DVR_LIB_SUPPORT
    CTsb *                               _currentTsb[eWindowType_Max];
#endif
#ifdef MPOD_SUPPORT
    CCablecard *                         _pCableCard;
#endif
#ifdef DCC_SUPPORT
    CClosedCaption *                     _pClosedCaption;
#endif
#ifdef SNMP_SUPPORT
    CSnmp *                              _pSnmp;
#endif
    CServerMgr *                         _pServerMgr;
    CAutoDiscoveryServer *               _pAutoDiscoveryServer;
    CAutoDiscoveryClient *               _pAutoDiscoveryClient;
    CPlaylistDb *                        _pPlaylistDb;
    CAudioCapture *                      _pAudioCapture;
    unsigned                             _connectId[eWindowType_Max];
    NEXUS_SimpleVideoDecoderServerHandle _simpleVideoDecoderServer;
    NEXUS_SimpleAudioDecoderServerHandle _simpleAudioDecoderServer;
    NEXUS_SimpleAudioPlaybackServerHandle _simpleAudioPlaybackServer;
    NEXUS_SimpleEncoderServerHandle      _simpleEncoderServer;
    eDynamicRange                        _dynamicRangeLast;
    bool                                 _bScanStarted;
#ifdef CPUTEST_SUPPORT
    CCpuTest *                           _pCpuTest;
#endif
#ifdef WPA_SUPPLICANT_SUPPORT
    CWifi *                              _pWifi;
#endif
};

#ifdef __cplusplus
}
#endif

#endif /* MODEL_H__ */
