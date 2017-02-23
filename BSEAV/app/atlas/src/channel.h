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

#ifndef CHANNEL_H__
#define CHANNEL_H__

#include "atlas_os.h"
#include "config.h"
#include "mvc.h"
#include "pidmgr.h"
#include "mxmlelement.h"
#include "mhash.h"
#include "tuner.h"
#include "playback.h"

class CInputBand;
class CParserBand;
class CRecord;
#if NEXUS_HAS_VIDEO_ENCODER
class CEncode;
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef enum eChannelTrick
{
    eChannelTrick_Stop,
    eChannelTrick_Play,
    eChannelTrick_Pause,
    eChannelTrick_Rewind,
    eChannelTrick_FastForward,
    eChannelTrick_FrameAdvance,
    eChannelTrick_FrameRewind,
    eChannelTrick_PlayNormal,
    eChannelTrick_PlayI,
    eChannelTrick_PlaySkipB,
    eChannelTrick_PlayIP,
    eChannelTrick_PlaySkipP,
    eChannelTrick_PlayBrcm,
    eChannelTrick_PlayGop,
    eChannelTrick_PlayGopIP,
    eChannelTrick_TimeSkip,
    eChannelTrick_Rate,
    eChannelTrick_Host,
    eChannelTrick_Seek,
    eChannelTrick_Max
} eChannelTrick;

ENUM_TO_MSTRING_DECLARE(channelTrickToString, eChannelTrick)

/* tuning will be done 1st if _pChannel != NULL.  otherwise tuning
 * will be based on the given _strChannel channel number */
class CChannelData
{
public:
    CChannelData(const char * strChannel) :
        _strChannel(strChannel),
        _tunerIndex(ANY_INDEX),
        _pChannel(NULL),
        _windowType(eWindowType_Max) {}
public:
    MString     _strChannel;
    uint16_t    _tunerIndex;
    CChannel *  _pChannel;
    eWindowType _windowType;
};

class CChannel : public CMvcModel
{
public:

    CChannel(
            const char *     strName,
            eBoardResource   type,
            CConfiguration * pCfg
            );
    ~CChannel(void);
    CChannel(const CChannel & ch); /* copy constructor */

    virtual CChannel *      createCopy(CChannel * pChannel)           = 0;
    virtual eRet            initialize(PROGRAM_INFO_T * pProgramInfo) = 0;
    virtual eRet            tune(void * id, CConfig * pResourceLibrary, bool bWaitForLock, uint16_t index = ANY_INDEX) = 0;
    virtual eRet            unTune(CConfig * pResourceLibrary, bool bFullUnTune = false, bool bCheckInTuner = true)    = 0;
    virtual eRet            readXML(MXmlElement * xmlElemChannel);
    virtual void            writeXML(MXmlElement * xmlElemChannel);
    virtual void            updateDescription(void);
    virtual eRet            openPids(CSimpleAudioDecode * pAudioDecode = NULL, CSimpleVideoDecode * pVideoDecode = NULL);
    virtual eRet            closePids(void);
    virtual eRet            getChannelInfo(CHANNEL_INFO_T * pChanInfo, bool bScanning);
    virtual int             addPsiPrograms(CTunerScanCallback addChannelCallback, void * context);
    virtual void            setStc(CStc * pStc)                            { _pStc = pStc; }
    virtual CStc *          getStc(void)                                   { return(_pStc); }
    virtual void            setModel(CModel * pModel)                      { _pModel = pModel; }
    virtual CModel *        getModel(void)                                 { return(_pModel); }
    virtual void            setWidgetEngine(CWidgetEngine * pWidgetEngine) { _pWidgetEngine = pWidgetEngine; }
    virtual CWidgetEngine * getWidgetEngine(void)                          { return(_pWidgetEngine); }
    virtual void            setDurationInMsecs(uint64_t durationInMsecs)   { _durationInMsecs = durationInMsecs; }
    virtual uint64_t        getDurationInMsecs(void)                       { return(_durationInMsecs); }
    virtual void            setStopAllowed(bool bStopAllowed)              { _bStopAllowed = bStopAllowed; }
    virtual bool            isStopAllowed(void)                            { return(_bStopAllowed); }
    virtual void            setPipSwapSupported(bool bPipSwapSupported)    { _bPipSwapSupported = bPipSwapSupported; }
    virtual bool            isPipSwapSupported(void)                       { return(_bPipSwapSupported); }
    virtual eRet            start(CSimpleAudioDecode * pAudioDecode = NULL,
            CSimpleVideoDecode *                       pVideoDecode = NULL);
    virtual CPid * getPid(
            uint16_t index,
            ePidType type
            ) { return(_pidMgr.getPid(index, type)); } /* Get Pid */
    virtual CPid * findPid(
            uint16_t pidNum,
            ePidType type
            ) { return(_pidMgr.findPid(pidNum, type)); }
    virtual bool     isRecordEnabled(void)      { return(true); }
    virtual uint16_t getWidth(void)             { return(_width); }
    virtual void     setWidth(uint16_t width)   { _width = width; }
    virtual uint16_t getHeight(void)            { return(_height); }
    virtual void     setHeight(uint16_t height) { _height = height; }
    virtual void     gotoBackGroundRecord(void) { return; }
    virtual void     dump(bool bForce = false);

    virtual bool operator ==(CChannel &other);

    /* Playback IP Channels support */
    virtual eRet         setAudioProgram(uint16_t pid)         { return(eRet_NotSupported); }
    virtual MString      getTimeString(void)                   { return(MString("")); }
    virtual bool         timelineSupport(void)                 { return(false); }
    virtual unsigned int getLastPosition(void)                 { return(0); }
    virtual unsigned int getCurrentPosition(void)              { return(0); }
    virtual eRet         getPsiInfo(void)                      { return(eRet_NotSupported); }
    virtual bool         isTunerRequired(void)                 { return(_bTunerRequired); }
    virtual void         setTunerRequired(bool bTunerRequired) { _bTunerRequired = bTunerRequired; }
    virtual eRet         play(void)                            { return(eRet_NotSupported); }
    virtual eRet         pause(void)                           { return(eRet_NotSupported); }
    virtual eRet         stop(void)                            { return(eRet_NotSupported); }
    virtual eRet         seek(
            bool,
            long int
            ) { return(eRet_NotSupported); }
    virtual eRet    applyTrickMode(void)                           { return(eRet_NotSupported); }
    virtual eRet    trickmode(CPlaybackTrickData * pTrickModeData) { BSTD_UNUSED(pTrickModeData); return(eRet_NotSupported); }
    virtual int     getTrickModeRate(void)                         { return(_trickModeRate); }
    virtual eRet    setTrickModeRate(int trickModeRate)            { BSTD_UNUSED(trickModeRate); return(eRet_NotSupported); }
    virtual eRet    setTrickMode(bool fastFoward)                  { BSTD_UNUSED(fastFoward); return(eRet_NotSupported); }
    virtual void    setHost(const char * pString)                  { BSTD_UNUSED(pString); }
    virtual MString getHost(void)                                  { return(""); }
    eBoardResource  getType(void)                                  { return(_type); }
    void            setType(eBoardResource resourceType)           { _type = resourceType; }
    void            setMajor(uint16_t major)                       { _major = major; }
    uint16_t        getMajor(void)                                 { return(_major); }
    void            setMinor(uint16_t minor)                       { _minor = minor; }
    uint16_t        getMinor(void)                                 { return(_minor); }
    MString         getDescription(void)                           { return(_strDescription); }
    MString         getDescriptionLong(void)                       { return(_strDescriptionLong); }
    MString         getDescriptionShort(void)                      { return(_strDescriptionShort); }
    void            setProgramNum(uint16_t programNum)             { _programNum = programNum; }
    uint16_t        getprogramNum(void)                            { return(_programNum); }
    MString         getChannelNum(void)                            { return(MString(_major) + MString(".") + MString(_minor)); }
    void            setInputBand(CInputBand * pInputBand)          { _pInputBand = pInputBand; }
    CInputBand *    getInputBand()                                 { return(_pInputBand); }
    bool            isTuned(void)                                  { return(_tuned); }
    void            setTransportType(NEXUS_TransportType type)     { _transportType = type; }      /* move */
    void            setParserBand(CParserBand * pParserBand)       { _pParserBand = pParserBand; } /* move */
    CParserBand *   getParserBand(void)                            { return(_pParserBand); }       /* move */
    CPidMgr *       getPidMgr(void)                                { return(&_pidMgr); }
    bool            isEncrypted(void)                              { return(_pidMgr.isEncrypted()); }  /* Encrypted */
    void            setRecord(CRecord * pRecord)                   { _pRecord = pRecord; }
    CRecord *       getRecord(void)                                { return(_pRecord); }
    CPlaypump *     getPlayback(void)                              { return(_pPlaypump); }
    bool            isRecording(void);
    bool            isEncoding(void);
    eRet            mapInputBand(CInputBand * pInputBand, CParserBand * pParserBand = NULL); /* used my channels */
    virtual eRet    dupParserBand(CParserBand * pParserBand);
    int             totalMetadata(void) { return(_metadata.total()); }
    const char *    getMetadataTag(int index);
    const char *    getMetadataValue(int index);
    eChannelTrick   getTrickModeState(void)       { return(_trickModeState); }
    uint32_t        getNumSubChannels(void)       { return(_numSubChannels); }
    CChannel *      getParent(void)               { return(_pParent); }
    void            setParent(CChannel * pParent) { _pParent = pParent; }

    void addMetadata(
            const char * strTag,
            const char * strValue
            ) { _metadata.add(strTag, new MString(strValue)); }

#if NEXUS_HAS_VIDEO_ENCODER
    void      setEncode(CEncode * pEncode) { _pEncode = pEncode; }
    CEncode * getEncode(void)              { return(_pEncode); }
#endif
#if NEXUS_HAS_FRONTEND
    CTuner * getTuner(void)            { return(_pTuner); }
    void     setTuner(CTuner * pTuner) { _pTuner = pTuner; }
#endif
#ifdef MPOD_SUPPORT
    void     setSourceId(uint32_t sourceId) { _sourceId = sourceId; }
    uint32_t getSourceId(void)              { return(_sourceId); }
#ifdef NEXUS_HAS_SECURITY
    void                setKeySlot(NEXUS_KeySlotHandle keySlot) { _keySlot = keySlot; }
    NEXUS_KeySlotHandle getKeySlot(void)                        { return(_keySlot); }
#endif
#endif /* ifdef MPOD_SUPPORT */
#ifdef SNMP_SUPPORT
    void    countCcError(void)  { _ccError++; }
    int     getCcError(void)    { return(_ccError); }
    void    countTeiError(void) { _teiError++; }
    int     getTeiError(void)   { return(_teiError); }
    void    setCci(uint8_t cci) { _cci = cci; }
    uint8_t getCci(void)        { return(_cci); }
#endif /* ifdef SNMP_SUPPORT */

protected:
    eBoardResource _type;
    uint16_t       _major;
    uint16_t       _minor;
    MString        _strDescription;
    MString        _strDescriptionLong;
    MString        _strDescriptionShort;
    uint16_t       _programNum;
#ifdef MPOD_SUPPORT
    uint32_t _sourceId;
#ifdef NEXUS_HAS_SECURITY
    NEXUS_KeySlotHandle _keySlot;
#endif
#endif /* ifdef MPOD_SUPPORT */
#ifdef SNMP_SUPPORT
    int     _ccError;
    int     _teiError;
    uint8_t _cci;
#endif
    CInputBand *  _pInputBand;
    bool          _tuned;
    CParserBand * _pParserBand;
    CPlayback *   _pPlayback;
    CPlaypump *   _pPlaypump;
#if NEXUS_HAS_VIDEO_ENCODER
    CEncode * _pEncode;
#endif
    NEXUS_TransportType _transportType;
    CPidMgr             _pidMgr;
#if NEXUS_HAS_FRONTEND
    CTuner * _pTuner;
#endif
    CRecord *        _pRecord;
    CConfiguration * _pCfg;
    CStc *           _pStc;
    int              _trickModeRate;
    eChannelTrick    _trickModeState;
    bool             _bTunerRequired;
    uint16_t         _width;
    uint16_t         _height;
    uint64_t         _durationInMsecs;
    bool             _bStopAllowed;
    bool             _bPipSwapSupported;
    uint32_t         _numSubChannels;
    CChannel *       _pParent;
    CModel *         _pModel;
    CWidgetEngine *  _pWidgetEngine;
    MHash<MString>   _metadata;
};

#ifdef __cplusplus
}
#endif

#endif /* CHANNEL_H__ */