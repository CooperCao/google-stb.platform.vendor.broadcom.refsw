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

#ifndef AUDIO_PLAYBACK_H__
#define AUDIO_PLAYBACK_H__

#include "atlas.h"
#include "atlas_cfg.h"
#include "atlas_os.h"
#include "bwidgets.h"
#include "widget_engine.h"
#include "model.h"
#include "audio_decode.h"
#include "nexus_simple_audio_playback.h"
#include "nexus_file.h"

#ifdef __cplusplus
extern "C" {
#endif

class CBoardResources;
class COutputSpdif;
class COutputHdmi;

class CSimplePcmPlayback : public CResource
{
public:
    CSimplePcmPlayback(
            const char *     name,
            const unsigned   number,
            CConfiguration * pCfg
            );

    virtual eRet                            open(CWidgetEngine * pWidgetEngine);
    virtual void                            close(void);
    virtual eRet                            start(const char * strFilename);
    virtual eRet                            stop(void);
    virtual NEXUS_SimpleAudioPlaybackHandle getAudioPlayback(void)    { return(_simplePlayback); }
    virtual bool                            isOpened(void) { return((NULL != _simplePlayback) ? true : false); }
    virtual eRet                            connect(unsigned index = 0) { return(eRet_Ok); }
    virtual void                            disconnect(void) { return; }
    virtual bool                            isConnected(void) { return((0 == _connectId) ? false : true); }

    void            doPlayback(void);
    void            getSettings(NEXUS_SimpleAudioPlaybackServerSettings * pSettings);
    eRet            setSettings(NEXUS_SimpleAudioPlaybackServerSettings * pSettings);
    void            setOutputSpdif(COutputSpdif * pSpdif) { _pSpdif = pSpdif; }
    COutputSpdif *  getOutputSpdif(void) { return(_pSpdif); }
    void            setOutputHdmi(COutputHdmi * pHdmi) { _pHdmi = pHdmi; }
    COutputHdmi *   getOutputHdmi(void) { return(_pHdmi); }
    void            destroyThreadPlayback(void);
    void            setModel(CModel * pModel)             { _pModel = pModel; }
    CModel *        getModel(void)                        { return(_pModel); }
    CWidgetEngine * getWidgetEngine(void) { return(_pWidgetEngine); }
    void            setResources(void * id, CBoardResources * pResources);
    void            clearPlaybackQueue(void);
    unsigned        playbackJobCount(void);
    void            addPlaybackJob(MString strFilename);
    MString         removePlaybackJob(void);
    void            waitForPcmPlaybackToComplete(long timeout = -1);

protected:
    NEXUS_SimpleAudioPlaybackHandle   _simplePlayback;
    NEXUS_AudioPlaybackHandle         _audioPlayback;
    COutputSpdif *                    _pSpdif;
    COutputHdmi *                     _pHdmi;
    int                               _startOffset;
    B_EventHandle                     _jobReadyEvent;
    B_EventHandle                     _bufferReadyEvent;
    B_ThreadHandle                    _playbackThread_handle;
    void *                            _resourceId;
    unsigned                          _connectId;
    MAutoList<MString>                _pcmSoundList;
    bool                              _bRun;
    bool                              _bLoop;

    B_MutexHandle                     _mutex;
    CBoardResources *                 _pBoardResources;
    CWidgetEngine *                   _pWidgetEngine;
    CModel *                          _pModel;
};

#ifdef __cplusplus
}
#endif

#endif /* AUDIO_PLAYBACK_H__ */