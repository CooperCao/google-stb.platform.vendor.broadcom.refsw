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

#ifndef OUTPUT_H__
#define OUTPUT_H__

#include "resource.h"
#include "atlas_cfg.h"
#include "atlas_os.h"
#include "convert.h"
#include "timer.h"

#include "nexus_svideo_output.h"
#include "nexus_component_output.h"
#include "nexus_composite_output.h"

#if NEXUS_HAS_RFM
#include "nexus_rfm.h"
#endif

#include "nexus_spdif_output.h"
#include "nexus_audio_dac.h"
#include "nexus_audio_types.h"

class CAudioVolume
{
public:
    CAudioVolume(void) :
        _volumeType(NEXUS_AudioVolumeType_eLinear),
        _left(NEXUS_AUDIO_VOLUME_LINEAR_NORMAL),
        _right(NEXUS_AUDIO_VOLUME_LINEAR_NORMAL),
        _muted(false) {}

    CAudioVolume(NEXUS_AudioOutputSettings output) :
        _volumeType(output.volumeType),
        _left(output.leftVolume),
        _right(output.rightVolume),
        _muted(output.muted) {}

public:
    NEXUS_AudioVolumeType _volumeType;
    int32_t               _left;
    int32_t               _right;
    bool                  _muted;
};

class COutput : public CResource
{
public:
    COutput(
            const char *         name,
            const uint16_t       number,
            const eBoardResource type,
            CConfiguration *     pCfg
            );
    virtual ~COutput(void);

    virtual eRet initialize(void)   { return(eRet_Ok); }
    virtual void uninitialize(void) {}

    /* video - pure virtual functions only apply to video capable outputs */
    virtual NEXUS_VideoOutput getConnectorV(void)                              = 0;
    virtual bool              isValidVideoFormat(NEXUS_VideoFormat format)     = 0;
    virtual NEXUS_VideoFormat getPreferredVideoFormat(void)                    = 0;
    virtual NEXUS_ColorSpace  getPreferredColorSpace(NEXUS_VideoFormat format) = 0;
    virtual eRet              setColorSpace(NEXUS_ColorSpace colorSpace) { BSTD_UNUSED(colorSpace); return(eRet_Ok); }
    virtual eRet              setMpaaDecimation(bool bMpaaDecimation)    { BSTD_UNUSED(bMpaaDecimation); return(eRet_Ok); }

    /* audio - pure virtual functions only apply to audio capable outputs */
    virtual NEXUS_AudioOutput getConnectorA() = 0;
    virtual eRet              connect(NEXUS_AudioInput input);
    virtual eRet              disconnect(void);
    virtual CAudioVolume      getVolume(void);
    virtual eRet              setVolume(CAudioVolume volume);
    virtual eRet              setVolume(int32_t level, NEXUS_AudioVolumeType type = NEXUS_AudioVolumeType_eLinear);
    virtual bool              getMute(void);
    virtual eRet              setMute(bool bMute);

protected:
    /* audio */
    bool _bConnected;
};

class COutputComponent : public COutput
{
public:
    COutputComponent(
            const char *     name,
            const uint16_t   number,
            CConfiguration * pCfg
            );
    virtual ~COutputComponent(void);

    virtual eRet              initialize(void);
    virtual NEXUS_VideoOutput getConnectorV(void);
    virtual eRet              setColorSpace(NEXUS_ColorSpace colorSpace);
    virtual eRet              setMpaaDecimation(bool bMpaaDecimation);
    virtual bool              isValidVideoFormat(NEXUS_VideoFormat format);
    virtual NEXUS_VideoFormat getPreferredVideoFormat(void)
    {
        return(stringToVideoFormat(GET_STR(_pCfg, PREFERRED_FORMAT_HD)));
    }

    virtual NEXUS_ColorSpace getPreferredColorSpace(NEXUS_VideoFormat format) { BSTD_UNUSED(format); return(NEXUS_ColorSpace_eMax); }

    NEXUS_ComponentOutputHandle getOutput()      { return(_outputComponent); }
    NEXUS_ComponentOutputType   getDesiredType() { return(_desiredType); }

    /* not applicable functions */
    NEXUS_AudioOutput getConnectorA() { return(NULL); }

protected:
    NEXUS_ComponentOutputHandle _outputComponent;
    NEXUS_ComponentOutputType   _desiredType;
};

class COutputSVideo : public COutput
{
public:
    COutputSVideo(
            const char *     name,
            const uint16_t   number,
            CConfiguration * pCfg
            );
    virtual ~COutputSVideo(void);

    virtual eRet              initialize(void);
    virtual NEXUS_VideoOutput getConnectorV(void);
    virtual bool              isValidVideoFormat(NEXUS_VideoFormat format);
    virtual NEXUS_VideoFormat getPreferredVideoFormat(void)
    {
        return(stringToVideoFormat(GET_STR(_pCfg, PREFERRED_FORMAT_SD)));
    }

    virtual NEXUS_ColorSpace getPreferredColorSpace(NEXUS_VideoFormat format) { BSTD_UNUSED(format); return(NEXUS_ColorSpace_eMax); }

    /* not applicable functions */
    NEXUS_AudioOutput getConnectorA() { return(NULL); }

protected:
    NEXUS_SvideoOutputHandle _outputSVideo;
};

class COutputComposite : public COutput
{
public:
    COutputComposite(
            const char *     name,
            const uint16_t   number,
            CConfiguration * pCfg
            );
    virtual ~COutputComposite(void);

    virtual eRet              initialize(void);
    virtual NEXUS_VideoOutput getConnectorV(void);
    virtual bool              isValidVideoFormat(NEXUS_VideoFormat format);
    virtual NEXUS_VideoFormat getPreferredVideoFormat(void)
    {
        return(stringToVideoFormat(GET_STR(_pCfg, PREFERRED_FORMAT_SD)));
    }

    virtual NEXUS_ColorSpace getPreferredColorSpace(NEXUS_VideoFormat format) { BSTD_UNUSED(format); return(NEXUS_ColorSpace_eMax); }

    NEXUS_CompositeOutputHandle getOutput() { return(_outputComposite); }

    /* not applicable functions */
    NEXUS_AudioOutput getConnectorA() { return(NULL); }

protected:
    NEXUS_CompositeOutputHandle _outputComposite;
};

class COutputRFM : public COutput
{
public:
    COutputRFM(
            const char *     name,
            const uint16_t   number,
            CConfiguration * pCfg
            );
    virtual ~COutputRFM(void);

    virtual eRet              initialize(void);
    virtual NEXUS_VideoOutput getConnectorV(void);
    virtual NEXUS_AudioOutput getConnectorA(void);
    virtual bool              isValidVideoFormat(NEXUS_VideoFormat format);
    virtual NEXUS_VideoFormat getPreferredVideoFormat(void)
    {
        return(stringToVideoFormat(GET_STR(_pCfg, PREFERRED_FORMAT_SD)));
    }

    virtual NEXUS_ColorSpace getPreferredColorSpace(NEXUS_VideoFormat format) { BSTD_UNUSED(format); return(NEXUS_ColorSpace_eMax); }

    virtual eRet connect(NEXUS_AudioInput input);

#if NEXUS_HAS_RFM
    virtual bool    getMute(void);
    virtual eRet    setMute(bool bMute);
    NEXUS_RfmHandle getOutput(void) { return(_outputRFM); }
    eRet            setChannel(uint32_t channelNum);
#endif /* if NEXUS_HAS_RFM */

protected:
#if NEXUS_HAS_RFM
    NEXUS_RfmHandle _outputRFM;
#endif
};

#include "nexus_hdmi_output_hdcp.h"

class COutputHdmi : public COutput
{
public:
    COutputHdmi(
            const char *     name,
            const uint16_t   number,
            CConfiguration * pCfg
            );
    virtual ~COutputHdmi(void);

    virtual eRet              initialize(void);
    virtual void              uninitialize(void);
    void                      timerCallback(void * pTimer);
    virtual NEXUS_VideoOutput getConnectorV(void);
    virtual NEXUS_AudioOutput getConnectorA(void);
    virtual eRet              setColorSpace(NEXUS_ColorSpace colorSpace);
    virtual bool              isValidVideoFormat(NEXUS_VideoFormat format, NEXUS_HdmiOutputStatus * pStatus);
    virtual bool              isValidVideoFormat(NEXUS_VideoFormat format) { return(isValidVideoFormat(format, NULL)); }
    virtual bool              isValidAudioCodec(NEXUS_AudioCodec codec, NEXUS_HdmiOutputStatus * pStatus);
    virtual bool              isValidAudioCodec(NEXUS_AudioCodec codec) { return(isValidAudioCodec(codec, NULL)); }
    virtual bool              isValidAudioCompressed(NEXUS_AudioCodec codec);
    virtual bool              isValidAudioMultichannel(NEXUS_AudioCodec codec);
    virtual uint8_t           getMaxAudioPcmChannels(void);
    virtual NEXUS_VideoFormat getPreferredVideoFormat(void)                     { return(_preferredVideoFormat); }
    virtual void              setPreferredVideoFormat(NEXUS_VideoFormat format) { _preferredVideoFormat = format; }
    virtual NEXUS_ColorSpace  getPreferredColorSpace(NEXUS_VideoFormat format);
    virtual void              setWidgetEngine(CWidgetEngine * pWidgetEngine);

    CWidgetEngine *        getWidgetEngine(void) { return(_pWidgetEngine); }
    NEXUS_HdmiOutputHandle getOutput(void)       { return(_outputHdmi); }
    eRet                   connect(NEXUS_AudioInput input);
    bool                   isConnected(void)     { return(_connected); }
    bool                   isHdcpPreferred(void) { return(_preferredHdcp); }
    eRet                   processHotplugStatus(NEXUS_HdmiOutputStatus * pStatus, bool bForce = false);
    eRet                   processHdcpStatus(NEXUS_HdmiOutputHdcpStatus * pStatus);
    eRet                   startHdcpAuthentication(void);
    void                   triggerHotPlug(NEXUS_VideoFormat formatOverride = NEXUS_VideoFormat_eUnknown);
    void                   dump(bool bForce = false);

protected:
    CWidgetEngine *        _pWidgetEngine;
    bool                   _connected;
    NEXUS_HdmiOutputHandle _outputHdmi;
    NEXUS_VideoFormat      _preferredVideoFormat;
    bool                   _preferredHdcp;
    bool                   _handleHdcpFailures;
    uint16_t               _hdcpFailureRetryDelay;
    CTimer                 _timerHdcpRetry;
};

class COutputSpdif : public COutput
{
public:
    COutputSpdif(
            const char *     name,
            const uint16_t   number,
            CConfiguration * pCfg
            );
    virtual ~COutputSpdif(void);

    virtual eRet              initialize(void);
    virtual NEXUS_AudioOutput getConnectorA(void);
    NEXUS_SpdifOutputHandle   getOutput(void) { return(_outputSpdif); }

    /* not applicable functions */
    NEXUS_VideoOutput getConnectorV(void)                              { return(NULL); }
    bool              isValidVideoFormat(NEXUS_VideoFormat format)     { BSTD_UNUSED(format); return(false); }
    NEXUS_VideoFormat getPreferredVideoFormat(void)                    { return(NEXUS_VideoFormat_eUnknown); }
    NEXUS_ColorSpace  getPreferredColorSpace(NEXUS_VideoFormat format) { BSTD_UNUSED(format); return(NEXUS_ColorSpace_eMax); }

protected:
    NEXUS_SpdifOutputHandle _outputSpdif;
};

class COutputAudioDac : public COutput
{
public:
    COutputAudioDac(
            const char *         name,
            const uint16_t       number,
            const eBoardResource type,
            CConfiguration *     pCfg
            );
    virtual ~COutputAudioDac(void);

    virtual eRet              initialize(void);
    virtual NEXUS_AudioOutput getConnectorA();

    /* not applicable functions */
    NEXUS_VideoOutput getConnectorV(void)                              { return(NULL); }
    bool              isValidVideoFormat(NEXUS_VideoFormat format)     { BSTD_UNUSED(format); return(false); }
    NEXUS_VideoFormat getPreferredVideoFormat(void)                    { return(NEXUS_VideoFormat_eUnknown); }
    NEXUS_ColorSpace  getPreferredColorSpace(NEXUS_VideoFormat format) { BSTD_UNUSED(format); return(NEXUS_ColorSpace_eMax); }

protected:
    NEXUS_AudioDacHandle _outputAudioDac;
};

class COutputAudioDacI2s : public COutputAudioDac
{
public:
    COutputAudioDacI2s(
            const char *     name,
            const uint16_t   number,
            CConfiguration * pCfg
            );
    virtual ~COutputAudioDacI2s(void);

    virtual eRet              initialize(void);
    virtual NEXUS_AudioOutput getConnectorA();

    /* not applicable functions */
    NEXUS_VideoOutput getConnectorV(void)                              { return(NULL); }
    bool              isValidVideoFormat(NEXUS_VideoFormat format)     { BSTD_UNUSED(format); return(false); }
    NEXUS_VideoFormat getPreferredVideoFormat(void)                    { return(NEXUS_VideoFormat_eUnknown); }
    NEXUS_ColorSpace  getPreferredColorSpace(NEXUS_VideoFormat format) { BSTD_UNUSED(format); return(NEXUS_ColorSpace_eMax); }

protected:
    NEXUS_I2sOutputHandle _outputI2s;
};

#include "nexus_audio_dummy_output.h"

class COutputAudioDummy : public COutput
{
public:
    COutputAudioDummy(
            const char *     name,
            const uint16_t   number,
            CConfiguration * pCfg
            );
    virtual ~COutputAudioDummy(void);

    virtual eRet                 initialize(void);
    virtual NEXUS_AudioOutput    getConnectorA();
    NEXUS_AudioDummyOutputHandle getOutput(void) { return(_outputAudioDummy); }

    /* not applicable functions */
    NEXUS_VideoOutput getConnectorV(void)                              { return(NULL); }
    bool              isValidVideoFormat(NEXUS_VideoFormat format)     { BSTD_UNUSED(format); return(false); }
    NEXUS_VideoFormat getPreferredVideoFormat(void)                    { return(NEXUS_VideoFormat_eUnknown); }
    NEXUS_ColorSpace  getPreferredColorSpace(NEXUS_VideoFormat format) { BSTD_UNUSED(format); return(NEXUS_ColorSpace_eMax); }

protected:
    NEXUS_AudioDummyOutputHandle _outputAudioDummy;
};

#endif /* OUTPUT_H__ */