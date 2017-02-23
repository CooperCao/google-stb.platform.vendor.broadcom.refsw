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

#ifndef TSB_H__
#define TSB_H__

#include "resource.h"
#include "pid.h"
#include "mvc.h"
#include "pidmgr.h"
#include "playback.h"
#include "videolist.h"
#include "mlist.h"
#include "mstring.h"
#include "mstringlist.h"
#include "mxmlelement.h"
#include "audio_decode.h"

#include "b_dvr_manager.h"
#include "b_dvr_mediastorage.h"
#include "b_dvr_datainjectionservice.h"
#include "b_dvr_recordservice.h"
#include "b_dvr_tsbservice.h"
#include "b_dvr_mediafile.h"

#ifdef __cplusplus
extern "C" {
#endif

class CDvrMgr;
class CControl;
class CVideo;
class CParserBand;
class CBoardResources;
class CSimpleVideoDecode;
class CSimpleAudioDecode;
class CStc;

class CTsb : public CResource
{
public:

    CTsb(
            const char *     name,
            const uint16_t   number,
            CConfiguration * pCfg
            );
    ~CTsb(void);
    static void   tsb_callback(void * context, int index, B_DVR_Event event, B_DVR_Service service);
    eRet          start();
    eRet          stop();
    eRet          play();
    eRet          pause();
    eRet          forward();
    eRet          rewind();
    eRet          slowForward();
    eRet          slowRewind();
    eRet          getStatus(B_DVR_TSBServiceStatus * pStatus);
    void          setPidMgr(CPidMgr * pPidMgr)                                  { _pPidMgr = pPidMgr; }
    CParserBand * getBand(void)                                                 { return(_pParserBand); }
    void          setBand(CParserBand * pParserBand)                            { _pParserBand = pParserBand; }
    void          setSimpleVideoDecode(CSimpleVideoDecode * pSimpleVideoDecode) { _pSimpleVideoDecode = pSimpleVideoDecode;  }
    void          setSimpleAudioDecode(CSimpleAudioDecode * pSimpleAudioDecode);
    CPid *        getTsbPlaybackAudioPid()              { return(_pAudioPid);    }
    void          setStc(CStc * pStc)                   { _pStc = pStc; }
    CStc *        getStc(void)                          { return(_pStc); }
    void          setWindowType(eWindowType windowType) { _windowType = windowType; }
    void          setDvrMgr(CDvrMgr * pDvrMgr)          { _pDvrMgr = pDvrMgr; }
    MString       getTimeString(void);
    MString       getTsbState(void) { return(_tsbState); }
    bool          isActive(void)    { return(_enabled); }
    bool          isTsbDecode(void) { return(_tsbDecode); }
protected:
    CParserBand *          _pParserBand;
    CPidMgr *              _pPidMgr;
    CBoardResources *      _pBoardResources;
    B_DVR_TSBServiceHandle _hTsbService;
    B_DVR_ServiceCallback  _serviceCallback;
    CDvrMgr *              _pDvrMgr;
    CSimpleVideoDecode *   _pSimpleVideoDecode;
    CSimpleAudioDecode *   _pSimpleAudioDecode;
    CStc *                 _pStc;
    eWindowType            _windowType;
    CPid *                 _pVideoPid;
    CPid *                 _pAudioPid;
    unsigned               _index;
    NEXUS_PlaybackHandle   _tsbPlayback;
    bool                   _tsbDecode;
    bool                   _enabled;
    eRet tsbDecode();
    eRet liveDecode();
    eRet decodeStart();
    eRet decodeStop();
    MString _tsbState;
};

#ifdef __cplusplus
}
#endif

#endif /* TSB_H__ */