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

#ifndef PLAYBACK_H__
#define PLAYBACK_H__

#include "nexus_types.h"
#include "nexus_playback.h"
#include "nexus_file.h"
#include "bwidgets.h"
#include "widget_engine.h"

#include "resource.h"
#include "pid.h"
#include "videolist.h"
#include "mvc.h"
#include "pidmgr.h"
#include "mlist.h"
#include "tspsimgr2.h"
#include "mstring.h"
#include "mstringlist.h"
#include "mxmlelement.h"
#include "bmedia_probe.h"
#include "dma.h"
#include "remote.h"

#ifdef __cplusplus
extern "C" {
#endif

class CControl;
class CBoardResources;
class CStc;

typedef enum ePlaybackTrick
{
    ePlaybackTrick_Stop,
    ePlaybackTrick_Play,
    ePlaybackTrick_Pause,
    ePlaybackTrick_Rewind,
    ePlaybackTrick_FastForward,
    ePlaybackTrick_FrameAdvance,
    ePlaybackTrick_FrameRewind,
    ePlaybackTrick_PlayNormal,
    ePlaybackTrick_PlayI,
    ePlaybackTrick_PlaySkipB,
    ePlaybackTrick_PlayIP,
    ePlaybackTrick_PlaySkipP,
    ePlaybackTrick_PlayBrcm,
    ePlaybackTrick_PlayGop,
    ePlaybackTrick_PlayGopIP,
    ePlaybackTrick_TimeSkip,
    ePlaybackTrick_Rate,
    ePlaybackTrick_Host,
    ePlaybackTrick_Seek,
    ePlaybackTrick_SeekRelative,
    ePlaybackTrick_Max
} ePlaybackTrick;

/* Used only by Custom Trick Mode data through LUA interface ONLY*/
class CPlaybackTrickData
{
public:
    CPlaybackTrickData(ePlaybackTrick command = ePlaybackTrick_Stop) :
        _command(command),
        _rate(1.0),
        _trick(NEXUS_PlaybackHostTrickMode_eNone),
        _modeModifier(1),
        _seekPosition(0) {}

public:
    ePlaybackTrick              _command;
    float                       _rate;
    NEXUS_PlaybackHostTrickMode _trick;
    int      _modeModifier;
    uint32_t _seekPosition;
};

class CPlaybackData
{
public:
    CPlaybackData(
            const char * strFileName = "filename.mpg",
            const char * strIndexName = NULL,
            const char * strPath = NULL,
            CVideo *     video = NULL,
            bool         tune = true
            ) :
        _strFileName(strFileName),
        _strIndexName(strIndexName),
        _strPath(strPath),
        _video(video),
        _bTuneLastChannel(tune) {}

public:
    MString  _strFileName;
    MString  _strIndexName;
    MString  _strPath;
    CVideo * _video;
    bool     _bTuneLastChannel;
};

class CPlaypump : public CResource
{
public:
    CPlaypump(
            const char *     name,
            const uint16_t   number,
            CConfiguration * pCfg
            );
    ~CPlaypump(void);

    eRet                       initialize(void);
    eRet                       uninitialize(void);
    eRet                       open();
    eRet                       close();
    eRet                       start();
    eRet                       stop();
    eRet                       play(void);
    eRet                       pause(void);
    eRet                       parseInfo(const char * filename);
    void                       closePids(void);
    NEXUS_PlaypumpHandle       getPlaypump(void) { return(_playpump); }
    eRet                       setSettings(NEXUS_PlaypumpSettings * playpumpSettings);
    NEXUS_PlaypumpOpenSettings getPlaypumpOpenSettings(void)                                { return(_playpumpOpenSettings); }
    void                       setSettings(NEXUS_PlaypumpOpenSettings playpumpOpenSettings) { _playpumpOpenSettings = playpumpOpenSettings; }
    void                       setVideo(CVideo * video)                                     { _currentVideo = video; }
    CVideo *                   getVideo(void)                                               { return(_currentVideo); }
    MString                    getVideoName(void);
    CPid *                     getPid(uint16_t index, ePidType type);
    CPid *                     findPid(uint16_t pidNum, ePidType type);
    void                       printPids(void);
    bool                       hasIndex(void);
    void                       dump(void);
    void                       setIp(bool isIp)  { _isIp = isIp; }
    bool                       isIp(void)        { return(_isIp); }
    bool                       isAllocated(void) { return(_allocated); }
    void                       setResources(
            void *            id,
            CBoardResources * pResources
            ) { BSTD_UNUSED(id); _pBoardResources = pResources; }

protected:
    NEXUS_FilePlayHandle       _file;
    NEXUS_FilePlayHandle       _customFile;
    NEXUS_FilePlayHandle       _stickyFile;
    NEXUS_PlaypumpHandle       _playpump;
    NEXUS_PlaypumpSettings     _playpumpSettings;
    NEXUS_PlaypumpOpenSettings _playpumpOpenSettings;
    CBoardResources *          _pBoardResources;

    bool     _isIp;
    bool     _active;
    bool     _allocated;
    CVideo * _currentVideo;
    int      _trickModeRate;
    MString  _trickModeState;
};

class CPlayback : public CResource
{
public:
    CPlayback(
            const char *     name,
            const uint16_t   number,
            CConfiguration * pCfg
            );
    ~CPlayback(void);

    eRet                   open(CWidgetEngine * pWidgetEngine = NULL);
    eRet                   close(CPidMgr * pPidMgr = NULL);
    eRet                   start(const char * filename, CPlaybackList * pPlaybackList);
    eRet                   start(CVideo * pVideo);
    eRet                   start(void);
    eRet                   start(NEXUS_PlaybackPosition pos);
    eRet                   start(NEXUS_FilePlayHandle file);
    eRet                   stop(CPidMgr * pidMgr = NULL);
    eRet                   play(ePlaybackTrick playMode = ePlaybackTrick_Play, bool bNotify = true);
    eRet                   pause(bool bNotify = true);
    eRet                   pause(bool bNotify, NEXUS_PlaybackPosition * pPosition); /* pause and return position */
    eRet                   seek(NEXUS_PlaybackPosition pos);
    eRet                   setTrickModeRate(float trickModeRate);
    eRet                   trickMode(eKey key);
    eRet                   trickMode(CPlaybackTrickData * pTrickModeData); /*Custom Trick Mode function used only through LUA */
    float                  getTrickModeRate(void) { return(_trickModeRate); }
    eRet                   parseInfo(const char * filename);
    eRet                   setStc(CStc * pStc = NULL);
    void                   dupPidMgr(CPidMgr * pPidMgr);
    void                   closePids(void);
    NEXUS_PlaybackHandle   getPlayback(void) { return(_playback); }
    CPlaypump *            getPlaypump(void) { return(_pPlaypump); }
    NEXUS_PlaybackSettings getSettings(void);
    eRet                   setSettings(NEXUS_PlaybackSettings * pPlaybackSettings);
    void                   setVideo(CVideo * video);
    CVideo *               getVideo(void) { return(_currentVideo); }
    MString                getVideoName(void);
    ePlaybackTrick         getTrickModeState(void) { return(_trickModeState); }
    MString                getTimeString(void);
    uint32_t               getMaxDataRate(void);
    CPid *                 getPid(uint16_t index, ePidType type);
    CPid *                 findPid(uint16_t pidNum, ePidType type);
    void                   printPids(void);
    bool                   hasIndex(void);
    bool                   isActive(void);
    void                   dump(void);
    void                   setIp(bool isIp)  { _isIp = isIp; }
    bool                   isIp(void)        { return(_isIp); }
    bool                   isAllocated(void) { return(_allocated); }
    bool                   isPaused(void)    { return(_trickModeState == ePlaybackTrick_Pause); }
    void                   setTS(void);
    CWidgetEngine *        getWidgetEngine(void) { return(_pWidgetEngine); }
    void                   beginEndOfStreamCallback(void);
    void                   setBeginEndCallbackStatus(int param) { _beginEndOfStreamStatus = param; }
    int                    getBeginEndCallbackStatus(void)      { return(_beginEndOfStreamStatus); }
    void                   setResources(
            void *            id,
            CBoardResources * pResources
            ) { BSTD_UNUSED(id); _pBoardResources = pResources; }

protected:
    CWidgetEngine *                 _pWidgetEngine;
    bwin_io_handle                  _hIoHandle;
    NEXUS_FilePlayHandle            _file;
    NEXUS_FilePlayHandle            _customFile;
    NEXUS_FilePlayHandle            _stickyFile;
    NEXUS_PlaybackHandle            _playback;
    NEXUS_PlaybackSettings          _playbackSettings;
    NEXUS_PlaybackStartSettings     _playbackStartSettings;
    NEXUS_PlaybackTrickModeSettings _trickModeSettings;
    CPlaypump *                     _pPlaypump;
    CBoardResources *               _pBoardResources;
    CStc *         _pStc;
    bool           _isIp;
    bool           _active;
    bool           _allocated;
    CVideo *       _currentVideo;
    float          _trickModeRate;
    float          _trickModeMaxDecodeRate;
    ePlaybackTrick _trickModeState;
    int            _beginEndOfStreamStatus;
    CPidMgr        _pidMgr;
#if (defined NEXUS_HAS_DMA || defined NEXUS_HAS_XPT_DMA)
    CDma * _pDma;
#endif
};

#ifdef __cplusplus
}
#endif

#endif /* PLAYBACK_H__ */