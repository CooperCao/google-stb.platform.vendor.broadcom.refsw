/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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

#ifndef VIDEOLIST_H__
#define VIDEOLIST_H__

#include "nexus_types.h"
#include "nexus_playback.h"
#include "nexus_file.h"

#include "resource.h"
#include "pid.h"
#include "mvc.h"
#include "pidmgr.h"
#include "playback.h"
#include "mlist.h"
#include "crypto.h"
#ifdef MPOD_SUPPORT
#include "tspsimgr.h"
#else
#include "tspsimgr2.h"
#endif
#include "mstring.h"
#include "mstringlist.h"
#include "mxmlelement.h"
#include "bmedia_probe.h"
#ifdef __cplusplus
extern "C" {
#endif

class CControl;
class CVideo;

typedef enum eServerIndexState
{
    eServerIndexState_Missing,
    eServerIndexState_Created,
    eServerIndexState_Failed,
    eServerIndexState_Max
} eServerIndexState;

class CPlaybackList : public CMvcModel
{
public:
    CPlaybackList(CConfiguration * pCfg);
    virtual ~CPlaybackList();

    void     directoryToStringList(const char * strDir, const char * strExtension, MStringList * pStrList);
    void     refreshFromDisk(void);
    void     removeVideo(CVideo * video);
    void     removeVideo(int index);
    void     addVideo(CVideo * video, int index = -1);
    eRet     readXML(MXmlElement * xmlElemInfoFile, const char * strInfoName, const char * strPath);
    eRet     readInfo(const char * infoName, const char * path);
    eRet     createInfo(const char * mediafile, const char * indexfile, const char * path, bool bAddToVideoList = true);
    void     createInfo(CVideo * video = NULL, CPidMgr * pPidMgr = NULL, bool bAddToVideoList = true);
    void     sync(void);
    CVideo * getVideo(uint32_t index);
    MString  getNextAtlasName(void);
    void     generateVideoName(MString path);
    CVideo * getVideo(MString name);
    CVideo * find(const char * name, int16_t nProgram = -1);
    bool     hasIndex(void);
    void     destroyThreadGenerateIndexes(void);
    void     doGenerateIndexes(void);
    eRet     generateIndexes(void);
    void     dump(bool bForce = false);

protected:
    eRet createInfo(MString &mediafile, bmedia_probe_t probe, CVideo * video);
    eRet writeInfo(MList<CVideo> * probedVideos = NULL, CPidMgr * pPidMgr = NULL);

    CConfiguration * _pCfg;
    MList <CVideo>   _videos;
    CVideo *         _lastVideo;
    int              _index;
    int              _nextVideoNumber;
    B_ThreadHandle   _generateIndexesThread_handle;
    volatile bool    _stopGenerateIndexes;
    B_MutexHandle    _mutexPlaylist;
};

class CVideo
{
public:
    CVideo(
            CConfiguration * pCfg,
            const char *     strMedia = NULL,
            const char *     strIndex = NULL,
            const char *     strPathVideos = NULL,
            const char *     strPathInfo = NULL,
            const char *     strPathIndex = NULL
            );
    CVideo(
            const CConfiguration * pCfg,
            const CVideo           & video
            ); /* copy constructor */
    virtual ~CVideo();

    void   closeVideo(void);
    bool   hasIndex();
    eRet   writeXML(MXmlElement * xmlElem);
    eRet   generateInfoName(const char * strName);
    eRet   readInfo(const char * infoName, CVideo * video);
    eRet   readXML(MXmlElement * xmlElem);
    eRet   readInfo(MString infoName);
    CPid * getPid(
            uint16_t index,
            ePidType type
            ) { return(_pidMgr.getPid(index, type)); }
    CPidMgr *         getPidMgr(void) { return(&_pidMgr); }
    void              dupPidMgr(CPidMgr * pPidMgr);
    eRet              updateFileSysInfo(void);
    uint16_t          getProgram(void)                     { return(_pidMgr.getProgram()); }
    void              setProgram(uint16_t program)         { _pidMgr.setProgram(program); }
    MString           getDescription(void)                 { return(_description); }
    void              setDescription(const char * strName) { _description = strName; }
    time_t            getDate(void)                        { return(_date); }
    int64_t           getSize(void)                        { return(_size); }
    void              setSize(int64_t size)                { _size = size; }
    uint32_t          getDuration(void)                    { return(_duration); }
    void              setDuration(uint32_t duration)       { _duration = duration; }
    uint16_t          getMaxBitrate(void)                  { return(_maxBitrate); }
    void              setMaxBitrate(uint16_t maxBitrate)   { _maxBitrate = maxBitrate; }
    MString           getInfoName(void)                    { return(_infoName); }
    MString           getInfoNamePath(void)                { return(_infoPath + MString("/") + _infoName); }
    void              setInfoName(const char * strName)    { _infoName = strName; }
    MString           getVideoName(void)                   { return(_mediaName); }
    MString           getVideoNamePath(void)               { return(_videosPath + MString("/") + _mediaName); }
    MString           getVideoNamePathAbsolute(void);
    void              setVideoName(const char * strName)  { _mediaName = strName; }
    MString           getIndexName(void)                  { return(_indexName); }
    MString           getIndexNamePath(void)              { return(_indexPath + MString("/") + _indexName); }
    void              setIndexName(const char * strName)  { _indexName = strName; }
    MString           getVideosPath(void)                 { return(_videosPath); }
    void              setVideosPath(const char * strName) { _videosPath = strName; }
    MString           getInfoPath(void)                   { return(_infoPath); }
    void              setInfoPath(const char * strName)   { _infoPath = strName; }
    MString           getIndexPath(void)                  { return(_indexPath); }
    void              setIndexPath(const char * strName)  { _indexPath = strName; }
    bool              isTimestampEnabled(void)            { return(_timestamp_enabled); }
    void              enableTimestamp(bool bEnable)       { _timestamp_enabled = bEnable; }
    bool              isForPurchase(void)                 { return(_isForPurchase); }
    void              setForPurchase(bool bPurchase)      { _isForPurchase = bPurchase; }
    bool              isPlaybackActive(void);
    bool              isRecordActive(void)           { return(_isRecordActive); }
    void              setRecordState(bool bActive)   { _isRecordActive = bActive; }
    bool              isIndexRequired(void)          { return(_isIndexRequired); }
    void              setIndexRequired(bool bActive) { _isIndexRequired = bActive; }
    void              inUse(void)                    { _usageCounter++; }
    uint16_t          getWidth(void);
    void              setWidth(uint16_t width);
    uint16_t          getHeight(void);
    void              setHeight(uint16_t height);
    void              dump(bool bForce = false);
    bool              isEncrypted(void);
    eRet              generateIndex(void);
    bool              isIndexGenerationNeeded(void)                           { return(_isIndexGenerationNeeded); }
    void              setIndexGenerationNeeded(bool isIndexGenerationNeeded)  { _isIndexGenerationNeeded = isIndexGenerationNeeded; }
    bool              isAudioOnly(void)                                       { return((NULL == getPid(0, ePidType_Video)) ? true : false); }
    eServerIndexState getServerIndexState(void)                               { return(_serverIndexState); }
    void              setServerIndexState(eServerIndexState serverIndexState) { _serverIndexState = serverIndexState; }
    uint32_t          getTotalStreams(void)                                   { return(_totalStreams); }
    void              setTotalStreams(uint32_t totalStreams)                  { _totalStreams = totalStreams; }
    uint32_t          setThumbTimestamp(uint8_t percent = 5)                  { _thumbTimestamp = getDuration() * percent / 100; return(_thumbTimestamp); }
    uint32_t          incrementThumbTimestamp(uint32_t increment);

protected:
    MString  _description;
    time_t   _date;
    int64_t  _size;
    uint32_t _duration;
    uint16_t _maxBitrate;
    MString  _infoName;
    MString  _mediaName;
    MString  _indexName;
    MString  _videosPath;
    MString  _infoPath;
    MString  _indexPath;
    CPidMgr  _pidMgr;
    bool     _isForPurchase;
    bool     _timestamp_enabled;
    bool     _encrypted;
#if DVR_LIB_SUPPORT
    bool _isTsbActive;
#endif
    bool              _isRecordActive;
    bool              _isIndexRequired;
    bool              _timeShifting;
    int               _position;
    int               _usageCounter;
    eServerIndexState _serverIndexState;
    uint32_t          _totalStreams; /* total streams in file this object originated from */
    bool              _isIndexGenerationNeeded;
    uint32_t          _thumbTimestamp;  /* timestamp used to extract thumbnail image from video */
};

#ifdef __cplusplus
}
#endif

#endif /* VIDEOLIST_H__ */