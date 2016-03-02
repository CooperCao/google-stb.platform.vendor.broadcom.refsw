/***************************************************************************
 * (c) 2002-2016 Broadcom Corporation
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

#ifndef AUDIO_DECODE_H__
#define AUDIO_DECODE_H__

#include "atlas.h"
#include "atlas_cfg.h"
#include "atlas_os.h"
#include "pid.h"
#include "stc.h"
#include "bwidgets.h"
#include "widget_engine.h"
#include "audio_processing.h"
#include "model.h"

#include "nexus_simple_audio_decoder_server.h"
#include "nexus_audio_decoder_types.h"
#include "nexus_audio_decoder.h"
#include "nexus_audio_capture.h"
#include "nexus_ac3_encode.h"
#include "nexus_dts_encode.h"
#include "nexus_auto_volume_level.h"
#include "nexus_dolby_volume.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum eSpdifInput
{
    eSpdifInput_Pcm,
    eSpdifInput_Compressed,
    eSpdifInput_EncodeDts,
    eSpdifInput_EncodeAc3,
    eSpdifInput_None,
    eSpdifInput_Max
} eSpdifInput;

typedef enum eHdmiAudioInput
{
    eHdmiAudioInput_Pcm,
    eHdmiAudioInput_Compressed,
    eHdmiAudioInput_Multichannel,
    eHdmiAudioInput_EncodeDts,
    eHdmiAudioInput_EncodeAc3,
    eHdmiAudioInput_None,
    eHdmiAudioInput_Max
} eHdmiAudioInput;

typedef enum eAudioDownmix
{
    eAudioDownmix_None,
    eAudioDownmix_Left,
    eAudioDownmix_Right,
    eAudioDownmix_Monomix,
    eAudioDownmix_AacMatrix,
    eAudioDownmix_AacArib,
    eAudioDownmix_AacLtRt,
    eAudioDownmix_Ac3Auto,
    eAudioDownmix_Ac3LoRo,
    eAudioDownmix_Ac3LtRt,
    eAudioDownmix_DtsAuto,
    eAudioDownmix_DtsLoRo,
    eAudioDownmix_DtsLtRt,
    eAudioDownmix_Max
} eAudioDownmix;

typedef enum eAudioDualMono
{
    eAudioDualMono_Left,
    eAudioDualMono_Right,
    eAudioDualMono_Stereo,
    eAudioDualMono_Monomix,
    eAudioDualMono_Max
} eAudioDualMono;

typedef enum eDolbyDRC
{
    eDolbyDRC_None,
    eDolbyDRC_Light,
    eDolbyDRC_Medium,
    eDolbyDRC_Heavy,
    eDolbyDRC_Max
} eDolbyDRC;

class CAudioDecode : public CResource
{
public:
    CAudioDecode(
            const char *     name,
            const uint16_t   number,
            CConfiguration * pCfg
            );
    ~CAudioDecode(void);

    virtual eRet                     open(CWidgetEngine * pWidgetEngine, CStc * pStc);
    virtual CStc *                   close(void);
    virtual CPid *                   getPid(void)        { return(_pPid); }
    virtual void                     setPid(CPid * pPid) { _pPid = pPid; }
    virtual CStc *                   getStc(void)        { return(_pStc); }
    virtual bool                     isStarted()         { return(_started); }
    virtual bool                     isOpened()          { return(_decoder ? true : false); }
    virtual NEXUS_AudioDecoderHandle getDecoder(void)    { return(_decoder); }
    virtual eRet                     getStatus(NEXUS_AudioDecoderStatus * pStatus);
    virtual bool                     isCodecSupported(NEXUS_AudioCodec codec);

    CWidgetEngine *  getWidgetEngine(void)                { return(_pWidgetEngine); }
    bool             isSourceChanged()                    { return(_sourceChanged); }
    void             setSourceChanged(bool sourceChanged) { _sourceChanged = sourceChanged; }
    NEXUS_AudioInput getConnector(NEXUS_AudioConnectorType type);

protected:
    NEXUS_AudioDecoderHandle         _decoder;
    NEXUS_AudioDecoderSettings       _decoderSettings;
    NEXUS_SimpleAudioDecoderSettings _simpleDecoderSettings;
    B_SchedulerCallbackHandle        _sourceChangedCallbackHandle;
    CStc *          _pStc;
    CPid *          _pPid;
    CWidgetEngine * _pWidgetEngine;
    bool            _started;
    bool            _sourceChanged;

#if B_HAS_EXTERNAL_ANALOG
    NEXUS_I2sInputHandle _i2sInput;
#endif
#if NEXUS_NUM_AUDIO_CAPTURES
    NEXUS_AudioCaptureHandle _audioCapture;
#endif
};

class CBoardResources;
class COutputSpdif;
class COutputHdmi;
class COutputAudioDac;
class COutputRFM;
class COutputAudioDummy;
class CModel;

class CSimpleAudioDecode : public CAudioDecode
{
public:
    CSimpleAudioDecode(
            const char *     name,
            const uint16_t   number,
            CConfiguration * pCfg
            );
    ~CSimpleAudioDecode(void);

    virtual eRet   open(CWidgetEngine * pWidgetEngine, CStc * pStc);
    virtual CStc * close(void);
    virtual eRet   start(CPid * pPid, CStc * pStc = NULL);
    virtual CPid * stop(void);
    virtual bool   isOpened() { return(_simpleDecoder ? true : false); }

    virtual bool             getMute(void);
    virtual eRet             setMute(bool bMute);
    virtual uint32_t         getVolume(void);
    virtual eRet             setVolume(uint32_t level);
    virtual eRet             setDownmix(eAudioDownmix downmix);
    virtual eAudioDownmix    getDownmix(void);
    virtual eRet             setDualMono(eAudioDualMono dualMono);
    virtual eAudioDualMono   getDualMono(void) { return(_dualMono); }
    virtual eRet             setDolbyDRC(eDolbyDRC dolbyDRC);
    virtual eDolbyDRC        getDolbyDRC(void) { return(_dolbyDRC); }
    virtual eRet             setDolbyDialogNorm(bool dolbyDialogNorm);
    virtual bool             getDolbyDialogNorm(void) { return(_dolbyDialogNorm); }
    virtual bool             isCodecSupported(NEXUS_AudioCodec codec);
    virtual eRet             setHdmiInput(eHdmiAudioInput hdmiInput, NEXUS_SimpleAudioDecoderServerSettings * pSettings = NULL);
    virtual eHdmiAudioInput  getHdmiInput(NEXUS_AudioCodec codec);
    virtual bool             isEncodeSupportedAc3(void)       { return(NULL != _encodeAc3 ? true : false); }
    virtual bool             isEncodeSupportedDts(void)       { return(NULL != _encodeDts ? true : false); }
    virtual bool             isAutoVolumeLevelSupported(void) { return(NULL != _pAutoVolumeLevel ? true : false); }
    virtual bool             isDolbyVolumeSupported(void)     { return(NULL != _pDolbyVolume ? true : false); }
    virtual bool             isTruVolumeSupported(void)       { return(NULL != _pTruVolume ? true : false); }
    virtual eRet             setAudioProcessing(eAudioProcessing audioProcessing);
    virtual eAudioProcessing getAudioProcessing(void) { return(_audioProcessing); }

    void setResources(void * id, CBoardResources * pResources);
    void setOutputHdmi(COutputHdmi * pHdmi)         { _pHdmi = pHdmi; }
    void setOutputSpdif(COutputSpdif * pSpdif)      { _pSpdif = pSpdif; }
    void setOutputDac(COutputAudioDac * pDac)       { _pDac = pDac; }
    void setOutputRFM(COutputRFM * pRFM)            { _pRFM = pRFM; }
    void setOutputDummy(COutputAudioDummy * pDummy) { _pDummy = pDummy; }

    eRet getStatus(NEXUS_AudioDecoderStatus * pStatus);

    /* SW7445-1016 : should return NULL for 14.3*/
    virtual NEXUS_AudioDecoderHandle       getDecoder(void)       { return(_pDecoders[0]->getDecoder()); }
    virtual NEXUS_SimpleAudioDecoderHandle getSimpleDecoder(void) { return(_simpleDecoder); }
    NEXUS_AudioInput                       getConnector(uint8_t num, NEXUS_AudioConnectorType type);

    void getSettings(NEXUS_SimpleAudioDecoderServerSettings * pSettings);
    eRet setSettings(NEXUS_SimpleAudioDecoderServerSettings * pSettings);

    void        connectEncodeAc3(bool bConnect);
    void        connectEncodeDts(bool bConnect);
    void        verifyEncode(NEXUS_AudioCodec codec);
    eRet        setSpdifInput(eSpdifInput spdifInput, NEXUS_SimpleAudioDecoderServerSettings * pSettings = NULL);
    eSpdifInput getSpdifInput(NEXUS_AudioCodec codec);
    eRet        setStc(CStc * pStc);
    CStc *      getStc(void) { return(_pStc); }
    void        setModel(CModel * pModel) { _pModel = pModel; }
    CModel *    getModel(void)            { return(_pModel); }

    COutputHdmi *  getOutputHdmi(void)  { return(_pHdmi); }
    COutputSpdif * getOutputSpdif(void) { return(_pSpdif); }
    void           setWindowType(eWindowType windowType) { _windowType = windowType; }
    eWindowType    getWindowType(void) { return(_windowType); }


protected:
    NEXUS_SimpleAudioDecoderHandle _simpleDecoder;
    NEXUS_Ac3EncodeHandle          _encodeAc3;
    NEXUS_DtsEncodeHandle          _encodeDts;
    CAutoVolumeLevel *             _pAutoVolumeLevel;
    CDolbyVolume *                 _pDolbyVolume;
    CTruVolume *                   _pTruVolume;
    CBoardResources *              _pBoardResources;
    CAudioDecode *                 _pDecoders[2];
    COutputSpdif *                 _pSpdif;            /* DTT TODO: should be a list? */
    COutputHdmi *                  _pHdmi;             /* DTT TODO: should be a list? */
    COutputAudioDac *              _pDac;              /* DTT TODO: should be a list? */
    COutputRFM *                   _pRFM;              /* DTT TODO: should be a list? */
    COutputAudioDummy *            _pDummy;            /* DTT TODO: should be a list? */
    uint16_t         _numSpdif;
    uint16_t         _numHdmi;
    void *           _resourceId;
    eAudioDownmix    _downmix;
    eAudioDownmix    _downmixAc3;
    eAudioDownmix    _downmixDts;
    eAudioDownmix    _downmixAac;
    eAudioDualMono   _dualMono;
    eDolbyDRC        _dolbyDRC;
    bool             _dolbyDialogNorm;
    eAudioProcessing _audioProcessing;
    NEXUS_AudioInput _stereoInput;
    bool             _bEncodeConnectedDts;
    bool             _bEncodeConnectedAc3;
    eWindowType      _windowType;
    CModel *         _pModel;
};

#ifdef __cplusplus
}
#endif

#endif /* AUDIO_DECODE_H__ */