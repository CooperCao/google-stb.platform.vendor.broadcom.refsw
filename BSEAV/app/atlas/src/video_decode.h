/******************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#ifndef VIDEO_DECODE_H__
#define VIDEO_DECODE_H__

#include "atlas.h"
#include "atlas_cfg.h"
#include "atlas_os.h"
#include "pid.h"
#include "stc.h"
#include "bwidgets.h"
#include "widget_engine.h"
#include "model.h"

#include "nexus_video_input.h"
#include "nexus_video_decoder_types.h"

class CVideoWindow;
class COutput;
class CModel;

#ifdef __cplusplus
extern "C" {
#endif

class CVideoDecode : public CResource
{
public:
    CVideoDecode(
            const char *     name,
            const uint16_t   number,
            CConfiguration * pCfg
            );
    ~CVideoDecode(void);

    virtual eRet   open(CWidgetEngine * pWidgetEngine, CStc * pStc);
    virtual CStc * close(void);
    virtual bool   isCodecSupported(NEXUS_VideoCodec codec);
    virtual void   videoDecodeCallback(void);

    CPid *                   getPid(void)                         { return(_pPid); }
    CStc *                   getStc(void)                         { return(_pStc); }
    bool                     isStarted(void)                      { return(_started); }
    bool                     isOpened(void)                       { return(_decoder ? true : false); }
    NEXUS_VideoDecoderHandle getDecoder(void)                     { return(_decoder); }
    CWidgetEngine *          getWidgetEngine(void)                { return(_pWidgetEngine); }
    bool                     isSourceChanged(void)                { return(_sourceChanged); }
    void                     setSourceChanged(bool sourceChanged) { _sourceChanged = sourceChanged; }
    NEXUS_VideoInputHandle   getConnector(void);
    eRet                     getStatus(NEXUS_VideoDecoderStatus * pStatus);
    eRet                     getStreamInfo(NEXUS_VideoDecoderStreamInformation * pStream);

protected:
    NEXUS_VideoDecoderHandle _decoder;
    CStc *                   _pStc;
    CPid *                   _pPid;
    CWidgetEngine *          _pWidgetEngine;
    bool                     _started;
    bool                     _sourceChanged;
    uint16_t                 _maxWidth;
    uint16_t                 _maxHeight;
};

class CBoardResources;

class CSimpleVideoDecode : public CVideoDecode
{
public:
    CSimpleVideoDecode(
            const char *     name,
            const uint16_t   number,
            CConfiguration * pCfg
            );
    ~CSimpleVideoDecode();

    virtual eRet                           open(CWidgetEngine * pWidgetEngine, CStc * pStc);
    virtual CStc *                         close(void);
    virtual eRet                           start(CPid * pPid, CStc * pStc = NULL);
    virtual CPid *                         stop(void);
    virtual bool                           isOpened()         { return(_simpleDecoder ? true : false); }
    virtual NEXUS_SimpleVideoDecoderHandle getSimpleDecoder() { return(_simpleDecoder); }
    virtual bool                           isCodecSupported(NEXUS_VideoCodec codec);
    virtual void                           videoDecodeCallback(void);

    ENUM_TO_MSTRING_DECLARE(noiseReductionModeToString, NEXUS_VideoWindowFilterMode)

    CVideoDecode *        getVideoDecoder(void) { return(_pDecoder); }
    eRet                  addVideoWindow(CVideoWindow * pVideoWindow);
    void                  removeVideoWindow(CVideoWindow * pVideoWindow);
    void                  swapVideoWindowLists(CSimpleVideoDecode * pVideoDecodeOther);
    MList<CVideoWindow> * getVideoWindowList(void)  { return(&_videoWindowList); }
    void                  setModel(CModel * pModel) { _pModel = pModel; }
    CModel *              getModel(void)            { return(_pModel); }

    void setResources(
            void *            id,
            CBoardResources * pResources
            ) { BSTD_UNUSED(id); _pBoardResources = pResources; }
    eRet                        getStatus(NEXUS_VideoDecoderStatus * pStatus);
    eRet                        getStreamInfo(NEXUS_VideoDecoderStreamInformation * pStream);
    eRet                        getOptimalVideoFormat(COutput * pOutput, NEXUS_VideoFormat * pFormat);
    eRet                        setStc(CStc * pStc);
    eRet                        setMaxSize(uint16_t width, uint16_t height);
    eRet                        setColorDepth(uint8_t depth);
    eRet                        setDnrBlockNoiseMode(NEXUS_VideoWindowFilterMode mode);
    NEXUS_VideoWindowFilterMode getDnrBlockNoiseMode(void);
    eRet                        setDnrMosquitoNoiseMode(NEXUS_VideoWindowFilterMode mode);
    NEXUS_VideoWindowFilterMode getDnrMosquitoNoiseMode(void);
    eRet                        setDnrContourMode(NEXUS_VideoWindowFilterMode mode);
    NEXUS_VideoWindowFilterMode getDnrContourMode(void);
    eRet                        setAnrMode(NEXUS_VideoWindowFilterMode mode);
    NEXUS_VideoWindowFilterMode getAnrMode(void);
    void                        videoDecodeSourceChangedCallback(void);
    void                        setWindowType(eWindowType windowType) { _windowType = windowType; }
    eWindowType                 getWindowType(void)                   { return(_windowType); }

protected:
    NEXUS_SimpleVideoDecoderHandle _simpleDecoder;
    CBoardResources *              _pBoardResources;
    CVideoDecode *                 _pDecoder;
    MList<CVideoWindow>            _videoWindowList;
    CVideoWindow *                 _pVideoWindow;
    eWindowType                    _windowType;
    CModel * _pModel;
};

#ifdef __cplusplus
}
#endif

#endif /* ifndef VIDEO_DECODE_H__ */