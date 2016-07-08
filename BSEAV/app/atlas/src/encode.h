/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
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
 *****************************************************************************/

#ifndef ENCODE_H__
#define ENCODE_H__

#if NEXUS_HAS_VIDEO_ENCODER

#include "nexus_types.h"
#include "nexus_simple_encoder.h"
#include "nexus_simple_encoder_server.h"
#include "nexus_file.h"
#include "nexus_stc_channel.h"
#include "nexus_stream_mux.h"
#include "nexus_display.h"

#include "resource.h"
#include "pid.h"
#include "mvc.h"
#include "pidmgr.h"
#include "playback.h"
#include "videolist.h"
#include "mlist.h"
#ifdef MPOD_SUPPORT
#include "tspsimgr.h"
#else
#include "tspsimgr2.h"
#endif
#include "mstring.h"
#include "mstringlist.h"
#include "mxmlelement.h"
#include "bmedia_probe.h"
#include "record.h"
#include "audio_decode.h"

#ifdef __cplusplus
extern "C" {
#endif

class CControl;
class CVideo;
class CParserBand;
class CBoardResources;

class CTranscodeData
{
public:
    CTranscodeData(
            const char * strFileName = NULL,
            const char * strPath = NULL,
            CVideo *     video = NULL
            ) :
        _strFileName(strFileName),
        _strPath(strPath),
        _video(video) {}

public:
    MString  _strFileName;
    MString  _strIndexName;
    MString  _strPath;
    CVideo * _video;
};

class CEncode : public CResource
{
public:

    CEncode(
            const char *     name,
            const uint16_t   number,
            CConfiguration * pCfg
            );
    ~CEncode(void);

    eRet                             initialize();
    eRet                             open();
    void                             close();
    eRet                             start();
    eRet                             stop();
    eRet                             parseInfo(const char * filename);
    CRecord *                        getRecord(void)        { return(_pRecord); }
    NEXUS_SimpleEncoderSettings      getSettings(void)      { return(_encoderSettings); }
    NEXUS_SimpleEncoderStartSettings getStartSettings(void) { return(_startSettings); }
    eRet                             setSettings(NEXUS_SimpleEncoderSettings encoderSettings);
    void                             setStartSettings(NEXUS_SimpleEncoderStartSettings encoderStartSettings) { _startSettings = encoderStartSettings; }
    void                             setVideo(CVideo * video)                                                { _currentVideo = video; }
    CVideo *                         getVideo(void)                                                          { return(_currentVideo); }
    void                             dupPidMgr(CPidMgr * pPidMgr);
    CPid *                           getPid(uint16_t index, ePidType type);
    eRet                             createVideo(MString fileName, MString path);
    void                             setBand(CParserBand * pParserBand) { _pDecodeParserBand = pParserBand; }
    bool                             hasIndex()                         { return(_currentVideo->hasIndex()); }
    bool                             isActive(void);
    eRet                             openPids(CPidMgr * pPidMgr, bool record);
    void                             dump(void);
    bool                             isAllocated(void)                                    { return(_allocated); }
    void                             setBoardResources(CBoardResources * pBoardResources) { _pBoardResources = pBoardResources; }
    void                             setPlaybackList(CPlaybackList * pPlaybackList)       { _pPlaybackList = pPlaybackList; }
    void                             setModel(CModel * pModel) { _pModel = pModel; }
protected:
    eRet simple_encoder_create(void);
    void simple_encoder_destroy(void);
    void checkinResources(void);
    eRet createVideo(void);

    bool                              _allocated;
    CVideo *                          _currentVideo;
    CPidMgr                           _pidMgr;
    NEXUS_SimpleEncoderHandle         _encoder;
    NEXUS_SimpleEncoderServerSettings _encoderServerSettings;
    NEXUS_SimpleEncoderStartSettings  _startSettings;
    NEXUS_SimpleEncoderSettings       _encoderSettings;
    NEXUS_AudioMixerHandle            _mixer;
    CParserBand *                     _pDecodeParserBand;
    CParserBand *                     _pRecordParserBand;
    CRecord *                         _pRecord;
    CRecpump *                        _pRecpump;
    CPlaypump *                       _pPlaypump[NEXUS_SIMPLE_ENCODER_NUM_PLAYPUMPS];
    CConfiguration *                  _pCfg;
    CBoardResources *                 _pBoardResources;
    CSimpleVideoDecode *              _pVideoDecode;
    CSimpleAudioDecode *              _pAudioDecode;
    CStc *                            _pStc;
    CStc *                            _pTranscodeStc;
    COutputAudioDummy *               _pOutputAudioDummy;
    CPlaybackList *                   _pPlaybackList;
    CModel *                          _pModel;
};

#ifdef __cplusplus
}
#endif
#endif /* if NEXUS_HAS_VIDEO_ENCODER */
#endif /* EBCODE_H__ */