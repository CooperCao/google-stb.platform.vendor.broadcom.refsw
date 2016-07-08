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

#include "output.h"
#include "atlas_os.h"
#include "mixer.h"
#include "convert.h"

#define CALLBACK_HDMI_HOTPLUG  "CallbackHdmiHotplug"
#define CALLBACK_HDMI_HDCP     "CallbackHdmiHdcp"
#define CALLBACK_HDMI_CEC      "CallbackHdmiCec"

BDBG_MODULE(atlas_output);

COutput::COutput(
        const char *     name,
        const uint16_t   number,
        eBoardResource   type,
        CConfiguration * pCfg
        ) :
    CResource(name, number, type, pCfg),
    _bConnected(false)
{
}

COutput::~COutput()
{
}

COutputComponent::COutputComponent(
        const char *     name,
        const uint16_t   number,
        CConfiguration * pCfg
        ) :
    COutput(name, number, eBoardResource_outputComponent, pCfg),
    _outputComponent(NULL),
    _desiredType(NEXUS_ComponentOutputType_eRGB)
{
}

COutputComponent::~COutputComponent()
{
}

eRet COutputComponent::initialize()
{
    CPlatform * pPlatformConfig = _pCfg->getPlatformConfig();
    eRet        ret             = eRet_Ok;

    BDBG_ASSERT(NULL != pPlatformConfig);

    ret = COutput::initialize();
    CHECK_ERROR_GOTO("resource initialization failed", ret, error);

    _outputComponent = pPlatformConfig->getOutputComponent(_number);
    _desiredType     = NEXUS_ComponentOutputType_eYPrPb;

error:
    return(ret);
}

NEXUS_VideoOutput COutputComponent::getConnectorV()
{
    BDBG_ASSERT(_outputComponent);
    return(NEXUS_ComponentOutput_GetConnector(_outputComponent));
}

eRet COutputComponent::setColorSpace(NEXUS_ColorSpace colorSpace)
{
    NEXUS_ComponentOutputSettings settings;
    NEXUS_Error                   nError = NEXUS_SUCCESS;
    eRet ret                             = eRet_Ok;

    NEXUS_ComponentOutput_GetSettings(getOutput(), &settings);

    switch (colorSpace)
    {
    case NEXUS_ColorSpace_eAuto:
    case NEXUS_ColorSpace_eYCbCr422:
    case NEXUS_ColorSpace_eYCbCr444:
    case NEXUS_ColorSpace_eYCbCr420:
        settings.type = NEXUS_ComponentOutputType_eYPrPb;
        break;

    case NEXUS_ColorSpace_eRgb:
    default:
        settings.type = NEXUS_ComponentOutputType_eRGB;
        break;
    } /* switch */

    nError = NEXUS_ComponentOutput_SetSettings(getOutput(), &settings);
    CHECK_NEXUS_ERROR_GOTO("error setting component output settings", ret, nError, error);

error:
    return(ret);
} /* setColorSpace */

eRet COutputComponent::setMpaaDecimation(bool bMpaaDecimation)
{
    NEXUS_ComponentOutputSettings settings;
    NEXUS_Error                   nError = NEXUS_SUCCESS;
    eRet ret                             = eRet_Ok;

    NEXUS_ComponentOutput_GetSettings(getOutput(), &settings);
    settings.mpaaDecimationEnabled = bMpaaDecimation;
    nError                         = NEXUS_ComponentOutput_SetSettings(getOutput(), &settings);
    CHECK_NEXUS_ERROR_GOTO("error setting component output settings", ret, nError, error);

error:
    return(ret);
}

bool COutputComponent::isValidVideoFormat(NEXUS_VideoFormat format)
{
    bool bValid = false;

    switch (format)
    {
    case NEXUS_VideoFormat_e1080i:
    case NEXUS_VideoFormat_e1080i50hz: /* European 50hz HD 1080 */
    case NEXUS_VideoFormat_e720p:
    case NEXUS_VideoFormat_e720p50hz: /* HD 720p 50hz for Australia */
    case NEXUS_VideoFormat_e720p24hz: /* HD 720p 24hz */
    case NEXUS_VideoFormat_e720p25hz: /* HD 720p 25hz */
    case NEXUS_VideoFormat_e720p30hz: /* HD 720p 30hz */
    case NEXUS_VideoFormat_e576p:
    case NEXUS_VideoFormat_e1440x576p50hz: /* CEA861B */
    case NEXUS_VideoFormat_e480p:
    case NEXUS_VideoFormat_e1440x480p60hz: /* CEA861B */
    case NEXUS_VideoFormat_eNtsc:
    case NEXUS_VideoFormat_eNtsc443:        /* NTSC encoding with the PAL color carrier frequency. */
    case NEXUS_VideoFormat_eNtscJapan:      /* Japan NTSC: no pedestal level */
    case NEXUS_VideoFormat_e720x482_NTSC:   /* 720x482i NTSC-M for North America */
    case NEXUS_VideoFormat_e720x482_NTSC_J: /* 720x482i Japan */
    case NEXUS_VideoFormat_e720x483p:       /* 720x483p */
    case NEXUS_VideoFormat_ePal:
    case NEXUS_VideoFormat_ePalM:    /* PAL Brazil */
    case NEXUS_VideoFormat_ePalN:    /* PAL_N */
    case NEXUS_VideoFormat_ePalNc:   /* PAL_N: Argentina */
    case NEXUS_VideoFormat_ePalB:    /* Australia */
    case NEXUS_VideoFormat_ePalB1:   /* Hungary */
    case NEXUS_VideoFormat_ePalD:    /* China */
    case NEXUS_VideoFormat_ePalD1:   /* Poland */
    case NEXUS_VideoFormat_ePalDK2:  /* Eastern Europe */
    case NEXUS_VideoFormat_ePalDK3:  /* Eastern Europe */
    case NEXUS_VideoFormat_ePalH:    /* Europe */
    case NEXUS_VideoFormat_ePalK:    /* Europe */
    case NEXUS_VideoFormat_ePalI:    /* U.K. */
    case NEXUS_VideoFormat_ePal60hz: /* 60Hz PAL */
        bValid = true;
        break;

    default:
        break;
    } /* switch */

    return(bValid);
} /* isValidVideoFormat */

COutputSVideo::COutputSVideo(
        const char *     name,
        const uint16_t   number,
        CConfiguration * pCfg
        ) :
    COutput(name, number, eBoardResource_outputSVideo, pCfg),
    _outputSVideo(NULL)
{
}

COutputSVideo::~COutputSVideo()
{
}

eRet COutputSVideo::initialize()
{
    CPlatform * pPlatformConfig = _pCfg->getPlatformConfig();
    eRet        ret             = eRet_Ok;

    BDBG_ASSERT(NULL != pPlatformConfig);

    ret = COutput::initialize();
    CHECK_ERROR_GOTO("resource initialization failed", ret, error);

    _outputSVideo = pPlatformConfig->getOutputSvideo(_number);

error:
    return(ret);
} /* initialize */

NEXUS_VideoOutput COutputSVideo::getConnectorV()
{
    BDBG_ASSERT(_outputSVideo);
    return(NEXUS_SvideoOutput_GetConnector(_outputSVideo));
}

bool COutputSVideo::isValidVideoFormat(NEXUS_VideoFormat format)
{
    bool bValid = false;

    switch (format)
    {
    case NEXUS_VideoFormat_eNtsc:
    case NEXUS_VideoFormat_eNtsc443:        /* NTSC encoding with the PAL color carrier frequency. */
    case NEXUS_VideoFormat_eNtscJapan:      /* Japan NTSC: no pedestal level */
    case NEXUS_VideoFormat_e720x482_NTSC:   /* 720x482i NTSC-M for North America */
    case NEXUS_VideoFormat_e720x482_NTSC_J: /* 720x482i Japan */
    case NEXUS_VideoFormat_e720x483p:       /* 720x483p */
    case NEXUS_VideoFormat_ePal:
    case NEXUS_VideoFormat_ePalM:    /* PAL Brazil */
    case NEXUS_VideoFormat_ePalN:    /* PAL_N */
    case NEXUS_VideoFormat_ePalNc:   /* PAL_N: Argentina */
    case NEXUS_VideoFormat_ePalB:    /* Australia */
    case NEXUS_VideoFormat_ePalB1:   /* Hungary */
    case NEXUS_VideoFormat_ePalD:    /* China */
    case NEXUS_VideoFormat_ePalD1:   /* Poland */
    case NEXUS_VideoFormat_ePalDK2:  /* Eastern Europe */
    case NEXUS_VideoFormat_ePalDK3:  /* Eastern Europe */
    case NEXUS_VideoFormat_ePalH:    /* Europe */
    case NEXUS_VideoFormat_ePalK:    /* Europe */
    case NEXUS_VideoFormat_ePalI:    /* U.K. */
    case NEXUS_VideoFormat_ePal60hz: /* 60Hz PAL */
        bValid = true;
        break;

    default:
        break;
    } /* switch */

    return(bValid);
} /* isValidVideoFormat */

COutputComposite::COutputComposite(
        const char *     name,
        const uint16_t   number,
        CConfiguration * pCfg
        ) :
    COutput(name, number, eBoardResource_outputComposite, pCfg),
    _outputComposite(NULL)
{
}

COutputComposite::~COutputComposite()
{
}

eRet COutputComposite::initialize()
{
    CPlatform * pPlatformConfig = _pCfg->getPlatformConfig();
    eRet        ret             = eRet_Ok;

    BDBG_ASSERT(NULL != pPlatformConfig);

    ret = COutput::initialize();
    CHECK_ERROR_GOTO("resource initialization failed", ret, error);

    _outputComposite = pPlatformConfig->getOutputComposite(_number);

error:
    return(ret);
}

NEXUS_VideoOutput COutputComposite::getConnectorV()
{
    BDBG_ASSERT(_outputComposite);
    return(NEXUS_CompositeOutput_GetConnector(_outputComposite));
}

bool COutputComposite::isValidVideoFormat(NEXUS_VideoFormat format)
{
    bool bValid = false;

    switch (format)
    {
    case NEXUS_VideoFormat_eNtsc:
    case NEXUS_VideoFormat_eNtsc443:        /* NTSC encoding with the PAL color carrier frequency. */
    case NEXUS_VideoFormat_eNtscJapan:      /* Japan NTSC: no pedestal level */
    case NEXUS_VideoFormat_e720x482_NTSC:   /* 720x482i NTSC-M for North America */
    case NEXUS_VideoFormat_e720x482_NTSC_J: /* 720x482i Japan */
    case NEXUS_VideoFormat_e720x483p:       /* 720x483p */
    case NEXUS_VideoFormat_ePal:
    case NEXUS_VideoFormat_ePalM:    /* PAL Brazil */
    case NEXUS_VideoFormat_ePalN:    /* PAL_N */
    case NEXUS_VideoFormat_ePalNc:   /* PAL_N: Argentina */
    case NEXUS_VideoFormat_ePalB:    /* Australia */
    case NEXUS_VideoFormat_ePalB1:   /* Hungary */
    case NEXUS_VideoFormat_ePalD:    /* China */
    case NEXUS_VideoFormat_ePalD1:   /* Poland */
    case NEXUS_VideoFormat_ePalDK2:  /* Eastern Europe */
    case NEXUS_VideoFormat_ePalDK3:  /* Eastern Europe */
    case NEXUS_VideoFormat_ePalH:    /* Europe */
    case NEXUS_VideoFormat_ePalK:    /* Europe */
    case NEXUS_VideoFormat_ePalI:    /* U.K. */
    case NEXUS_VideoFormat_ePal60hz: /* 60Hz PAL */
        bValid = true;
        break;

    default:
        break;
    } /* switch */

    return(bValid);
} /* isValidVideoFormat */

COutputRFM::COutputRFM(
        const char *     name,
        const uint16_t   number,
        CConfiguration * pCfg
        ) :
    COutput(name, number, eBoardResource_outputRFM, pCfg)
#if NEXUS_HAS_RFM
    , _outputRFM(NULL)
#endif
{
}

COutputRFM::~COutputRFM()
{
}

eRet COutputRFM::initialize()
{
    CPlatform * pPlatformConfig = _pCfg->getPlatformConfig();
    eRet        ret             = eRet_Ok;

    BDBG_ASSERT(NULL != pPlatformConfig);

    ret = COutput::initialize();
    CHECK_ERROR_GOTO("resource initialization failed", ret, error);

#if NEXUS_HAS_RFM
    if (NULL != pPlatformConfig->getOutputRFM(_number))
    {
        int outputDelay = 0;

        _outputRFM  = pPlatformConfig->getOutputRFM(_number);
        outputDelay = GET_INT(_pCfg, DAC_OUTPUT_DELAY);
        if (0 < outputDelay)
        {
            NEXUS_AudioOutputSettings outputSettings;
            NEXUS_Error               nError = NEXUS_SUCCESS;

            NEXUS_AudioOutput_GetSettings(NEXUS_Rfm_GetAudioConnector(_outputRFM), &outputSettings);
            outputSettings.additionalDelay = outputDelay;
            BDBG_WRN(("RFM output delay %d", outputSettings.additionalDelay));
            nError = NEXUS_AudioOutput_SetSettings(NEXUS_Rfm_GetAudioConnector(_outputRFM), &outputSettings);
            CHECK_NEXUS_ERROR_ASSERT("error setting audio output settings", nError);
        }
        setVolume(NEXUS_AUDIO_VOLUME_LINEAR_NORMAL);
        setChannel(GET_INT(_pCfg, RFM_CHANNEL_NUM));
    }
#endif /* if NEXUS_HAS_RFM */

error:
    return(ret);
} /* initialize */

NEXUS_VideoOutput COutputRFM::getConnectorV()
{
    NEXUS_VideoOutput output = NULL;

#if NEXUS_HAS_RFM
    BDBG_ASSERT(_outputRFM);
    output = NEXUS_Rfm_GetVideoConnector(_outputRFM);
#endif

    return(output);
}

NEXUS_AudioOutput COutputRFM::getConnectorA()
{
    NEXUS_AudioOutput output = NULL;

#if NEXUS_HAS_RFM
    BDBG_ASSERT(_outputRFM);
    output = NEXUS_Rfm_GetAudioConnector(_outputRFM);
#endif

    return(output);
}

bool COutputRFM::isValidVideoFormat(NEXUS_VideoFormat format)
{
    bool bValid = false;

    switch (format)
    {
    case NEXUS_VideoFormat_eNtsc:
    case NEXUS_VideoFormat_eNtsc443:        /* NTSC encoding with the PAL color carrier frequency. */
    case NEXUS_VideoFormat_eNtscJapan:      /* Japan NTSC: no pedestal level */
    case NEXUS_VideoFormat_e720x482_NTSC:   /* 720x482i NTSC-M for North America */
    case NEXUS_VideoFormat_e720x482_NTSC_J: /* 720x482i Japan */
    case NEXUS_VideoFormat_e720x483p:       /* 720x483p */
    case NEXUS_VideoFormat_ePal:
    case NEXUS_VideoFormat_ePalM:    /* PAL Brazil */
    case NEXUS_VideoFormat_ePalN:    /* PAL_N */
    case NEXUS_VideoFormat_ePalNc:   /* PAL_N: Argentina */
    case NEXUS_VideoFormat_ePalB:    /* Australia */
    case NEXUS_VideoFormat_ePalB1:   /* Hungary */
    case NEXUS_VideoFormat_ePalD:    /* China */
    case NEXUS_VideoFormat_ePalD1:   /* Poland */
    case NEXUS_VideoFormat_ePalDK2:  /* Eastern Europe */
    case NEXUS_VideoFormat_ePalDK3:  /* Eastern Europe */
    case NEXUS_VideoFormat_ePalH:    /* Europe */
    case NEXUS_VideoFormat_ePalK:    /* Europe */
    case NEXUS_VideoFormat_ePalI:    /* U.K. */
    case NEXUS_VideoFormat_ePal60hz: /* 60Hz PAL */
        bValid = true;
        break;

    default:
        break;
    } /* switch */

    return(bValid);
} /* isValidVideoFormat */

eRet COutputRFM::connect(NEXUS_AudioInput input)
{
    eRet        ret    = eRet_Ok;
    NEXUS_Error nerror = NEXUS_SUCCESS;

    BDBG_ASSERT(NULL != input);

    nerror = NEXUS_AudioOutput_AddInput(getConnectorA(), input);
    CHECK_NEXUS_ERROR_GOTO("error adding input to RFM output", ret, nerror, error);

error:
    return(ret);
}

#if NEXUS_HAS_RFM
bool COutputRFM::getMute()
{
    NEXUS_RfmSettings settings;

    NEXUS_Rfm_GetSettings(getOutput(), &settings);

    return(settings.muted);
}

eRet COutputRFM::setMute(bool bMute)
{
    eRet              ret    = eRet_Ok;
    NEXUS_Error       nerror = NEXUS_SUCCESS;
    NEXUS_RfmSettings settings;

    NEXUS_Rfm_GetSettings(getOutput(), &settings);
    settings.muted = bMute;
    nerror         = NEXUS_Rfm_SetSettings(getOutput(), &settings);
    CHECK_NEXUS_ERROR_GOTO("unable to set RFM settings", ret, nerror, error);

error:
    return(ret);
}

eRet COutputRFM::setChannel(uint32_t channelNum)
{
    NEXUS_Error nerror = NEXUS_SUCCESS;
    eRet        ret    = eRet_Ok;

    if ((2 > channelNum) || (83 < channelNum))
    {
        ret = eRet_InvalidParameter;
        goto error;
    }

    {
        NEXUS_RfmSettings settings;
        NEXUS_Rfm_GetSettings(getOutput(), &settings);
        settings.channel = channelNum;
        NEXUS_Rfm_SetSettings(getOutput(), &settings);
        CHECK_NEXUS_ERROR_GOTO("unable to set RFM channel num", ret, nerror, error);
    }

error:
    return(ret);
} /* setChannel */

#endif /* if NEXUS_HAS_RFM */

static void nexusHdmiHotplugCallback(
        void * context,
        int    param
        )
{
    COutputHdmi * pOutputHdmi = (COutputHdmi *)context;

    BSTD_UNUSED(param);
    BDBG_ASSERT(NULL != pOutputHdmi);

    CWidgetEngine * pWidgetEngine = pOutputHdmi->getWidgetEngine();
    if (NULL != pWidgetEngine)
    {
        pWidgetEngine->syncCallback(pOutputHdmi, CALLBACK_HDMI_HOTPLUG);
    }
}

static void nexusHdcpStatusCallback(
        void * context,
        int    param
        )
{
    COutputHdmi * pOutputHdmi = (COutputHdmi *)context;

    BSTD_UNUSED(param);
    BDBG_ASSERT(NULL != pOutputHdmi);

    CWidgetEngine * pWidgetEngine = pOutputHdmi->getWidgetEngine();

    if (NULL != pWidgetEngine)
    {
        pWidgetEngine->syncCallback(pOutputHdmi, CALLBACK_HDMI_HDCP);
    }
}

static void nexusHdmiCecCallback(
        void * context,
        int    param
        )
{
    COutputHdmi * pOutputHdmi = (COutputHdmi *)context;

    BSTD_UNUSED(param);
    BDBG_ASSERT(NULL != pOutputHdmi);

    CWidgetEngine * pWidgetEngine = pOutputHdmi->getWidgetEngine();

    if (NULL != pWidgetEngine)
    {
        pWidgetEngine->syncCallback(pOutputHdmi, CALLBACK_HDMI_CEC);
    }
}

/* timer callbacks are already synchronized with bwin */
void COutputHdmi::timerCallback(void * pTimer)
{
    if (&_timerHdcpRetry == (CTimer *)pTimer)
    {
        startHdcpAuthentication();
    }
}

/*
 *  Callback from the bwidgets main loop - io trigger is complete and we can
 *  safely handle hot plug callback
 */
static void bwinHdmiHotplugCallback(
        void *       pObject,
        const char * strCallback
        )
{
    COutputHdmi *          pOutputHdmi = (COutputHdmi *)pObject;
    NEXUS_HdmiOutputStatus status;

    BSTD_UNUSED(strCallback);

    BDBG_ASSERT(NULL != pOutputHdmi);
    BSTD_UNUSED(strCallback);

    NEXUS_HdmiOutput_GetStatus(pOutputHdmi->getOutput(), &status);
    BDBG_MSG(("HDMI Hotplug Event status %d connected %d", status.connected, pOutputHdmi->isConnected()));

    pOutputHdmi->processHotplugStatus(&status);
}

/*
 *  Callback from the bwidgets main loop - io trigger is complete and we can
 *  safely handle HDCP callback
 */
static void bwinHdmiHdcpCallback(
        void *       pObject,
        const char * strCallback
        )
{
    COutputHdmi * pOutputHdmi = (COutputHdmi *)pObject;

    BSTD_UNUSED(strCallback);

    if (true == pOutputHdmi->isHdcpPreferred())
    {
        NEXUS_HdmiOutputHdcpStatus hdcpStatus;

        NEXUS_HdmiOutput_GetHdcpStatus(pOutputHdmi->getOutput(), &hdcpStatus);
        pOutputHdmi->processHdcpStatus(&hdcpStatus);

        /* TODO: DTT - notify registered observers (display!) of hdcp status change event (see bsettop_dislay)
         * display has to handle the setting/clearing of bluescreen depending on hdcp status */
    }
}

/*
 *  Callback from the bwidgets main loop - io trigger is complete and we can
 *  safely handle CEC callback
 */
static void bwinHdmiCecCallback(
        void *       pObject,
        const char * strCallback
        )
{
    BSTD_UNUSED(pObject);
    BSTD_UNUSED(strCallback);

    BDBG_WRN(("unhandled HDMI CEC callback"));
}

COutputHdmi::COutputHdmi(
        const char *     name,
        const uint16_t   number,
        CConfiguration * pCfg
        ) :

    COutput(name, number, eBoardResource_outputHdmi, pCfg),
    _pWidgetEngine(NULL),
    _connected(false),
    _outputHdmi(NULL),
    _preferredVideoFormat(NEXUS_VideoFormat_eUnknown),
    _preferredHdcp(false),
    _handleHdcpFailures(true),
    _hdcpFailureRetryDelay(500),
    _timerHdcpRetry((CSubject *)this)
{
    _timerHdcpRetry.setName("HDCP retry timer");
}

COutputHdmi::~COutputHdmi()
{
}

void COutputHdmi::setWidgetEngine(CWidgetEngine * pWidgetEngine)
{
    eRet ret = eRet_Ok;

    if (NULL != pWidgetEngine)
    {
        ret = pWidgetEngine->addCallback(this, CALLBACK_HDMI_HOTPLUG, bwinHdmiHotplugCallback);
        CHECK_ERROR("unable to add callback to widget engine", ret);
        ret = pWidgetEngine->addCallback(this, CALLBACK_HDMI_HDCP, bwinHdmiHdcpCallback);
        CHECK_ERROR("unable to add callback to widget engine", ret);
        ret = pWidgetEngine->addCallback(this, CALLBACK_HDMI_CEC, bwinHdmiCecCallback);
        CHECK_ERROR("unable to add callback to widget engine", ret);
    }
    else
    {
        _pWidgetEngine->removeCallback(this, CALLBACK_HDMI_HOTPLUG);
        _pWidgetEngine->removeCallback(this, CALLBACK_HDMI_HDCP);
        _pWidgetEngine->removeCallback(this, CALLBACK_HDMI_CEC);
    }

    _pWidgetEngine = pWidgetEngine;

    _timerHdcpRetry.setWidgetEngine(_pWidgetEngine);
} /* setWidgetEngine */

/* returns true if hdmi receiver device can decode given video format */
bool COutputHdmi::isValidVideoFormat(
        NEXUS_VideoFormat        format,
        NEXUS_HdmiOutputStatus * pStatus
        )
{
    NEXUS_Error            nerror = NEXUS_SUCCESS;
    eRet                   ret    = eRet_Ok;
    NEXUS_HdmiOutputStatus status;

    if (NULL == pStatus)
    {
        nerror = NEXUS_HdmiOutput_GetStatus(getOutput(), &status);
        CHECK_NEXUS_ERROR_GOTO("unable to get HDMI status", ret, nerror, error);
        pStatus = &status;
    }

    return(pStatus->videoFormatSupported[format]);

error:
    BDBG_WRN(("error checking valid hdmi video format - assume false"));
    return(false);
} /* isValidVideoFormat */

/* returns true if hdmi receiver device can decode given audio codec */
bool COutputHdmi::isValidAudioCodec(
        NEXUS_AudioCodec         codec,
        NEXUS_HdmiOutputStatus * pStatus
        )
{
    NEXUS_Error            nerror = NEXUS_SUCCESS;
    eRet                   ret    = eRet_Ok;
    NEXUS_HdmiOutputStatus status;

    if (NULL == pStatus)
    {
        nerror = NEXUS_HdmiOutput_GetStatus(getOutput(), &status);
        CHECK_NEXUS_ERROR_GOTO("unable to get HDMI status", ret, nerror, error);
        pStatus = &status;
    }

    /* always disable AAC */
    pStatus->audioCodecSupported[NEXUS_AudioCodec_eAac]         = false;
    pStatus->audioCodecSupported[NEXUS_AudioCodec_eAacLoas]     = false;
    pStatus->audioCodecSupported[NEXUS_AudioCodec_eAacPlus]     = false;
    pStatus->audioCodecSupported[NEXUS_AudioCodec_eAacPlusAdts] = false;

    return(pStatus->audioCodecSupported[codec]);

error:
    BDBG_WRN(("error checking valid hdmi audio codec - assume false"));
    return(false);
} /* isValidAudioCodec */

uint8_t COutputHdmi::getMaxAudioPcmChannels()
{
    NEXUS_Error            nerror = NEXUS_SUCCESS;
    eRet                   ret    = eRet_Ok;
    NEXUS_HdmiOutputStatus status;

    nerror = NEXUS_HdmiOutput_GetStatus(getOutput(), &status);
    CHECK_NEXUS_ERROR_GOTO("unable to get HDMI status", ret, nerror, error);

    return(status.maxAudioPcmChannels);

error:
    BDBG_WRN(("error checking valid hdmi video format - assume false"));
    return(0);
}

/* decided whether given codec can be output as 'compressed' over hdmi */
bool COutputHdmi::isValidAudioCompressed(NEXUS_AudioCodec codec)
{
    bool bValid = false;

    switch (codec)
    {
    case NEXUS_AudioCodec_eLpcmDvd:     /* LPCM, DVD mode */
    case NEXUS_AudioCodec_eLpcmHdDvd:   /* LPCM, HD-DVD mode */
    case NEXUS_AudioCodec_eLpcmBluRay:  /* LPCM, Blu-Ray mode */
    case NEXUS_AudioCodec_ePcm:         /* PCM audio - Generally used only with inputs such as SPDIF or HDMI. */
    case NEXUS_AudioCodec_ePcmWav:      /* PCM audio with Wave header - Used with streams containing PCM audio */
    case NEXUS_AudioCodec_eLpcm1394:    /* IEEE-1394 LPCM audio  */
    case NEXUS_AudioCodec_eAac:         /* Advanced audio coding with ADTS (Audio Data Transport Format) sync */
    case NEXUS_AudioCodec_eAacLoas:     /* Advanced audio coding with LOAS (Low Overhead Audio Stream) sync and LATM mux */
    case NEXUS_AudioCodec_eAacPlus:     /* AAC plus SBR. aka MPEG-4 High Efficiency (AAC-HE): with LOAS (Low Overhead Audio Stream) sync and LATM mux */
    case NEXUS_AudioCodec_eAacPlusAdts: /* AAC plus SBR. aka MPEG-4 High Efficiency (AAC-HE), with ADTS (Audio Data Transport Format) sync */
        /* compressed not allowed */
        break;

    case NEXUS_AudioCodec_eAc3Plus:
        if (true == isValidAudioCodec(NEXUS_AudioCodec_eAc3Plus))
        {
            bValid = true;
        }
        else
        {
            /* edid says Ac3+ is UNsupported */
            if (true == isValidAudioCodec(NEXUS_AudioCodec_eAc3))
            {
                /* edid say regular Ac3 is supported - we can use decoder0 to convert to ac3 */
                bValid = true;
            }
        }
        break;
    case NEXUS_AudioCodec_eUnknown:
    default:
        if (true == isValidAudioCodec(codec))
        {
            /* compressed allowed */
            bValid = true;
        }
        break;
    } /* switch */

    return(bValid);
} /* isValidAudioCompressed */

/* decided whether given codec can be output as 'multichannel' over hdmi */
bool COutputHdmi::isValidAudioMultichannel(NEXUS_AudioCodec codec)
{
    bool bValid = false;

    if (6 <= getMaxAudioPcmChannels())
    {
        /* hdmi receiver can do multichannel */
        if (6 <= audioCodecToNumChannelsString(codec).toInt())
        {
            /* given codec is multichannel capable */
            bValid = true;
        }
    }

    return(bValid);
} /* isValidAudioMultichannel */

eRet COutputHdmi::processHotplugStatus(
        NEXUS_HdmiOutputStatus * pStatus,
        bool                     bForce
        )
{
    eRet        ret    = eRet_Ok;
    NEXUS_Error nError = NEXUS_SUCCESS;

    BDBG_ASSERT(NULL != pStatus);

    if (true == GET_BOOL(_pCfg, HDMI_IGNORE_HOTPLUG))
    {
        return(ret);
    }

    if (false == bForce)
    {
        if (_connected == pStatus->connected)
        {
            /* duplicate hotplug status - ignore */
            goto done;
        }
    }

    dump();

    if (pStatus->connected)
    {
        setPreferredVideoFormat(pStatus->preferredVideoFormat);
        BDBG_MSG(("HDMI EDID preferred video format:%s", videoFormatToString(pStatus->preferredVideoFormat).s()));
    }
    else
    {
        if (true == _preferredHdcp)
        {
            nError = NEXUS_HdmiOutput_DisableHdcpAuthentication(_outputHdmi);
            ret    = CHECK_NEXUS_ERROR("error disabling hdcp authentication", nError);
        }
    }

    _connected = pStatus->connected;

done:
    notifyObservers(eNotify_HdmiHotplugEvent, this);

    return(ret);
} /* processHotplugStatus */

/* artificially trigger an HDMI hotplug event - used at startup */
void COutputHdmi::triggerHotPlug(NEXUS_VideoFormat formatOverride)
{
    NEXUS_HdmiOutputStatus status;

    NEXUS_HdmiOutput_GetStatus(getOutput(), &status);
    BDBG_MSG(("FORCE HDMI Hotplug Event status %d connected %d", status.connected, isConnected()));

    if (NEXUS_VideoFormat_eUnknown != formatOverride)
    {
        BDBG_MSG(("trigger hot plug with format:%s connected:%d", videoFormatToString(formatOverride).s(), status.connected));
        status.preferredVideoFormat = formatOverride;
    }

    /* force hotplug event (atlas.cfg HDMI_IGNORE_HOTPLUG will still override this) */
    processHotplugStatus(&status, true);
}

NEXUS_ColorSpace COutputHdmi::getPreferredColorSpace(NEXUS_VideoFormat format)
{
    eRet             ret        = eRet_Ok;
    NEXUS_Error      nError     = NEXUS_SUCCESS;
    NEXUS_ColorSpace colorSpace = NEXUS_ColorSpace_eMax;
    NEXUS_HdmiOutputEdidVideoFormatSupport outputFormatSupport;

    if ((NEXUS_VideoFormat_e4096x2160p50hz == format) ||
        (NEXUS_VideoFormat_e4096x2160p60hz == format) ||
        (NEXUS_VideoFormat_e3840x2160p50hz == format) ||
        (NEXUS_VideoFormat_e3840x2160p60hz == format))
    {
        /* force 4:2:0 for 2160p 50Hz/60Hz */
        colorSpace = NEXUS_ColorSpace_eYCbCr420;
    }
    else
    {
        nError = NEXUS_HdmiOutput_GetVideoFormatSupport(_outputHdmi, format, &outputFormatSupport);
        CHECK_NEXUS_ERROR_GOTO("unable to retrieve hdmi output video format support", ret, nError, error);

        if (true == outputFormatSupport.yCbCr444rgb444)
        {
            colorSpace = NEXUS_ColorSpace_eYCbCr444;
        }
        else
        if (true == outputFormatSupport.yCbCr420)
        {
            colorSpace = NEXUS_ColorSpace_eYCbCr420;
        }
    }
    BDBG_MSG(("get preferred colorspace for format %s: %s",
              videoFormatToString(format).s(), colorSpaceToString(colorSpace).s()));

error:
    return(colorSpace);
} /* getPreferredColorSpace */

eRet COutputHdmi::processHdcpStatus(NEXUS_HdmiOutputHdcpStatus * pStatus)
{
    eRet                     ret    = eRet_Ok;
    NEXUS_Error              nError = NEXUS_SUCCESS;
    NEXUS_HdmiOutputSettings hdmiOutputSettings;

    BDBG_ASSERT(NULL != pStatus);

    NEXUS_HdmiOutput_GetSettings(_outputHdmi, &hdmiOutputSettings);

    switch (pStatus->hdcpState)
    {
    case NEXUS_HdmiOutputHdcpState_eUnauthenticated:
        if (true == _handleHdcpFailures)
        {
            NEXUS_HdmiOutput_GetSettings(_outputHdmi, &hdmiOutputSettings);
            hdmiOutputSettings.syncOnly = true;
            nError                      = NEXUS_HdmiOutput_SetSettings(_outputHdmi, &hdmiOutputSettings);
            CHECK_NEXUS_ERROR_ASSERT("error setting hdmi output settings", nError);

            setMute(true);
        }
        break;

    case NEXUS_HdmiOutputHdcpState_eR0LinkFailure:
    case NEXUS_HdmiOutputHdcpState_ePjLinkIntegrityFailure:
    case NEXUS_HdmiOutputHdcpState_eRiLinkIntegrityFailure:
    case NEXUS_HdmiOutputHdcpState_eRepeaterAuthenticationFailure:
        if (true == _handleHdcpFailures)
        {
            NEXUS_HdmiOutput_GetSettings(_outputHdmi, &hdmiOutputSettings);
            hdmiOutputSettings.syncOnly = true;
            nError                      = NEXUS_HdmiOutput_SetSettings(_outputHdmi, &hdmiOutputSettings);
            CHECK_NEXUS_ERROR_ASSERT("error setting hdmi output settings", nError);

            setMute(true);
        }

        /* Retry authorization if requested */
        if (0 < _hdcpFailureRetryDelay)
        {
            _timerHdcpRetry.start(_hdcpFailureRetryDelay);
        }
        break;

    case NEXUS_HdmiOutputHdcpState_eLinkAuthenticated:
    case NEXUS_HdmiOutputHdcpState_eEncryptionEnabled:
        /* dtt - shouldn't we always handle this case? */
        if (true == _handleHdcpFailures)
        {
            NEXUS_HdmiOutput_GetSettings(_outputHdmi, &hdmiOutputSettings);
            hdmiOutputSettings.syncOnly = false;
            nError                      = NEXUS_HdmiOutput_SetSettings(_outputHdmi, &hdmiOutputSettings);
            CHECK_NEXUS_ERROR_ASSERT("error setting hdmi output settings", nError);

            setMute(false);
        }
        break;

    default:
        break;
    } /* switch */

    return(ret);
} /* processHdcpStatus */

eRet COutputHdmi::startHdcpAuthentication()
{
    eRet        ret    = eRet_Ok;
    NEXUS_Error nError = NEXUS_SUCCESS;

    if (false == _preferredHdcp)
    {
        return(ret);
    }

    BDBG_WRN(("Starting HDCP authentication"));
    nError = NEXUS_HdmiOutput_StartHdcpAuthentication(_outputHdmi);
    ret    = CHECK_NEXUS_ERROR("error starting hdcp authentication", nError);

    return(ret);
}

eRet COutputHdmi::initialize()
{
    eRet                     ret             = eRet_Ok;
    CPlatform *              pPlatformConfig = _pCfg->getPlatformConfig();
    NEXUS_Error              nError;
    NEXUS_HdmiOutputSettings hdmiSettings;
    /* coverity[stack_use_local_overflow] */
    NEXUS_HdmiOutputHdcpSettings hdcpSettings;

    BDBG_ASSERT(NULL != pPlatformConfig);

    ret = COutput::initialize();
    CHECK_ERROR_GOTO("resource initialization failed", ret, error);

    _outputHdmi = pPlatformConfig->getOutputHdmi(_number);

    /* register nexus callbacks for hdmi events */
    NEXUS_HdmiOutput_GetSettings(_outputHdmi, &hdmiSettings);
    hdmiSettings.hotplugCallback.callback = nexusHdmiHotplugCallback;
    hdmiSettings.hotplugCallback.context  = this;
    hdmiSettings.cecCallback.callback     = nexusHdmiCecCallback;
    hdmiSettings.cecCallback.context      = this;

    nError = NEXUS_HdmiOutput_SetSettings(_outputHdmi, &hdmiSettings);
    CHECK_NEXUS_ERROR_ASSERT("error setting hdmi output settings", nError);

    /* register nexus callbacks for hdcp events */
    NEXUS_HdmiOutput_GetHdcpSettings(_outputHdmi, &hdcpSettings);
    hdcpSettings.stateChangedCallback.callback = nexusHdcpStatusCallback;
    hdcpSettings.stateChangedCallback.context  = this;
    hdcpSettings.failureCallback.callback      = nexusHdcpStatusCallback;
    hdcpSettings.failureCallback.context       = this;
    /* TO DO: Add successCallback */
    nError = NEXUS_HdmiOutput_SetHdcpSettings(_outputHdmi, &hdcpSettings);
    CHECK_NEXUS_ERROR_ASSERT("error setting hdcp output settings", nError);

    /* add audio delay if specified by atlas configuration option */
    if (0 < GET_INT(_pCfg, HDMI_OUTPUT_DELAY))
    {
        NEXUS_AudioOutputSettings nOutputSettings;
        NEXUS_AudioOutput_GetSettings(NEXUS_HdmiOutput_GetAudioConnector(_outputHdmi), &nOutputSettings);
        nOutputSettings.additionalDelay = GET_INT(_pCfg, HDMI_OUTPUT_DELAY);
        BDBG_WRN(("HDMI output delay %d", nOutputSettings.additionalDelay));
        nError = NEXUS_AudioOutput_SetSettings(NEXUS_HdmiOutput_GetAudioConnector(_outputHdmi), &nOutputSettings);
        CHECK_NEXUS_ERROR_ASSERT("error setting audio output settings", nError);
    }

    setVolume(NEXUS_AUDIO_VOLUME_LINEAR_NORMAL);

    /* manually trigger hotplug event to initialize */
    bwinHdmiHotplugCallback(this, NULL);

error:
    return(ret);
} /* initialize */

void COutputHdmi::uninitialize()
{
    setWidgetEngine(NULL);
    _timerHdcpRetry.stop();
} /* uninitialize */

NEXUS_VideoOutput COutputHdmi::getConnectorV()
{
    BDBG_ASSERT(_outputHdmi);
    return(NEXUS_HdmiOutput_GetVideoConnector(_outputHdmi));
}

NEXUS_AudioOutput COutputHdmi::getConnectorA()
{
    BDBG_ASSERT(_outputHdmi);
    return(NEXUS_HdmiOutput_GetAudioConnector(_outputHdmi));
}

eRet COutputHdmi::setColorSpace(NEXUS_ColorSpace colorSpace)
{
    eRet                     ret    = eRet_Ok;
    NEXUS_Error              nError = NEXUS_SUCCESS;
    NEXUS_HdmiOutputSettings settings;

    BDBG_ASSERT(NEXUS_ColorSpace_eMax > colorSpace);

    NEXUS_HdmiOutput_GetSettings(getOutput(), &settings);
    settings.colorSpace = colorSpace;
    nError              = NEXUS_HdmiOutput_SetSettings(getOutput(), &settings);
    CHECK_NEXUS_ERROR_GOTO("error setting hdmi color space settings", ret, nError, error);

error:
    return(ret);
} /* setColorSpace */

eRet COutputHdmi::connect(NEXUS_AudioInput input)
{
    eRet        ret    = eRet_Ok;
    NEXUS_Error nerror = NEXUS_SUCCESS;

    BDBG_ASSERT(NULL != input);

    nerror = NEXUS_AudioOutput_AddInput(getConnectorA(), input);
    CHECK_NEXUS_ERROR_GOTO("error adding input to hdmi output", ret, nerror, error);

error:
    return(ret);
}

void COutputHdmi::dump(bool bForce)
{
    BDBG_Level level;

    if (true == bForce)
    {
        BDBG_GetModuleLevel("atlas_output", &level);
        BDBG_SetModuleLevel("atlas_output", BDBG_eMsg);
    }

    /* debug */
    BDBG_MSG(("List supported HDMI VIDEO formats:"));
    {
        NEXUS_Error            nerror = NEXUS_SUCCESS;
        NEXUS_HdmiOutputStatus status;

        nerror = NEXUS_HdmiOutput_GetStatus(getOutput(), &status);
        if (NEXUS_SUCCESS == nerror)
        {
            for (int i = 0; i < NEXUS_VideoFormat_eMax; i++)
            {
                if (true == isValidVideoFormat((NEXUS_VideoFormat)i, &status))
                {
                    BDBG_MSG(("     supported HDMI VIDEO format:%s", videoFormatToString((NEXUS_VideoFormat)i).s()));
                }
            }
        }
    }

    BDBG_MSG(("List supported HDMI AUDIO codecs:"));
    {
        NEXUS_Error            nerror = NEXUS_SUCCESS;
        NEXUS_HdmiOutputStatus status;

        nerror = NEXUS_HdmiOutput_GetStatus(getOutput(), &status);
        if (NEXUS_SUCCESS == nerror)
        {
            for (int i = 0; i < NEXUS_AudioCodec_eMax; i++)
            {
                if (true == isValidAudioCodec((NEXUS_AudioCodec)i, &status))
                {
                    BDBG_MSG(("     supported HDMI AUDIO codec:%s", audioCodecToString((NEXUS_AudioCodec)i).s()));
                }
            }
        }
    }

    if (true == bForce)
    {
        BDBG_SetModuleLevel("atlas_output", level);
    }
} /* dump */

eRet COutput::connect(NEXUS_AudioInput input)
{
    eRet        ret    = eRet_Ok;
    NEXUS_Error nerror = NEXUS_SUCCESS;

    BDBG_ASSERT(NULL != input);

    nerror = NEXUS_AudioOutput_AddInput(getConnectorA(), input);
    CHECK_NEXUS_ERROR_GOTO("error adding input to audio output", ret, nerror, error);

    _bConnected = true;

error:
    return(ret);
}

eRet COutput::disconnect()
{
    eRet        ret    = eRet_Ok;
    NEXUS_Error nerror = NEXUS_SUCCESS;

    if (false == _bConnected)
    {
        return(ret);
    }

    nerror = NEXUS_AudioOutput_RemoveAllInputs(getConnectorA());
    CHECK_NEXUS_ERROR_GOTO("error removing all inputs from audio output", ret, nerror, error);

    _bConnected = false;

error:
    return(ret);
} /* disconnect */

CAudioVolume COutput::getVolume()
{
    NEXUS_AudioOutputSettings settings;

    NEXUS_AudioOutput_GetSettings(getConnectorA(), &settings);
    CAudioVolume volume(settings);

    return(volume);
}

eRet COutput::setVolume(CAudioVolume volume)
{
    NEXUS_AudioOutputSettings settings;
    NEXUS_Error               nerror = NEXUS_SUCCESS;
    eRet                      ret    = eRet_Ok;

    NEXUS_AudioOutput_GetSettings(getConnectorA(), &settings);
    settings.volumeType  = volume._volumeType;
    settings.leftVolume  = volume._left;
    settings.rightVolume = volume._right;
    settings.muted       = volume._muted;
    nerror               = NEXUS_AudioOutput_SetSettings(getConnectorA(), &settings);
    CHECK_NEXUS_ERROR_GOTO("unable to set audio output", ret, nerror, error);

    notifyObservers(eNotify_VolumeChanged, &volume);
error:
    return(ret);
} /* setVolume */

eRet COutput::setVolume(
        int32_t               level,
        NEXUS_AudioVolumeType type
        )
{
    CAudioVolume volume = getVolume();

    volume._volumeType = type;
    volume._left       = level;
    volume._right      = level;

    return(setVolume(volume));
}

bool COutput::getMute()
{
    NEXUS_AudioOutputSettings settings;

    NEXUS_AudioOutput_GetSettings(getConnectorA(), &settings);
    CAudioVolume volume(settings);

    return(volume._muted);
}

eRet COutput::setMute(bool bMute)
{
    CAudioVolume volume = getVolume();
    eRet         ret    = eRet_Ok;

    volume._muted = bMute;

    ret = setVolume(volume);
    notifyObservers(eNotify_MuteChanged, &volume);

    return(ret);
}

COutputSpdif::COutputSpdif(
        const char *     name,
        const uint16_t   number,
        CConfiguration * pCfg
        ) :
    COutput(name, number, eBoardResource_outputSpdif, pCfg),
    _outputSpdif(NULL)
{
}

COutputSpdif::~COutputSpdif()
{
}

eRet COutputSpdif::initialize()
{
    CPlatform * pPlatformConfig = _pCfg->getPlatformConfig();
    eRet        ret             = eRet_Ok;
    NEXUS_Error nError          = NEXUS_SUCCESS;

    BDBG_ASSERT(NULL != pPlatformConfig);

    ret = COutput::initialize();
    CHECK_ERROR_GOTO("resource initialization failed", ret, error);

    if (NULL != pPlatformConfig->getOutputSpdif(_number))
    {
        int outputDelay = 0;

        _outputSpdif = pPlatformConfig->getOutputSpdif(_number);
        outputDelay  = GET_INT(_pCfg, SPDIF_OUTPUT_DELAY);
        if (0 < outputDelay)
        {
            NEXUS_AudioOutputSettings outputSettings;
            NEXUS_AudioOutput_GetSettings(NEXUS_SpdifOutput_GetConnector(_outputSpdif), &outputSettings);
            outputSettings.additionalDelay = outputDelay;
            BDBG_WRN(("SPDIF output delay %d", outputSettings.additionalDelay));
            nError = NEXUS_AudioOutput_SetSettings(NEXUS_SpdifOutput_GetConnector(_outputSpdif), &outputSettings);
            CHECK_NEXUS_ERROR_ASSERT("error setting audio out settings", nError);
        }
        setVolume(NEXUS_AUDIO_VOLUME_LINEAR_NORMAL);
    }

error:
    return(ret);
} /* initialize */

NEXUS_AudioOutput COutputSpdif::getConnectorA()
{
    BDBG_ASSERT(_outputSpdif);
    return(NEXUS_SpdifOutput_GetConnector(_outputSpdif));
}

COutputAudioDac::COutputAudioDac(
        const char *         name,
        const uint16_t       number,
        const eBoardResource type,
        CConfiguration *     pCfg
        ) :
    COutput(name, number, type, pCfg),
    _outputAudioDac(NULL)
{
}

COutputAudioDac::~COutputAudioDac()
{
    disconnect();
}

eRet COutputAudioDac::initialize()
{
    CPlatform * pPlatformConfig = _pCfg->getPlatformConfig();
    eRet        ret             = eRet_Ok;
    NEXUS_Error nError          = NEXUS_SUCCESS;

    BDBG_ASSERT(NULL != pPlatformConfig);

    ret = COutput::initialize();
    CHECK_ERROR_GOTO("resource initialization failed", ret, error);

    if (NULL != pPlatformConfig->getOutputAudioDac(_number))
    {
        int outputDelay = 0;

        _outputAudioDac = pPlatformConfig->getOutputAudioDac(_number);
        outputDelay     = GET_INT(_pCfg, DAC_OUTPUT_DELAY);
        if (0 < outputDelay)
        {
            NEXUS_AudioOutputSettings outputSettings;
            NEXUS_AudioOutput_GetSettings(NEXUS_AudioDac_GetConnector(_outputAudioDac), &outputSettings);
            outputSettings.additionalDelay = outputDelay;
            BDBG_WRN(("Audio DAC output delay %d", outputSettings.additionalDelay));
            nError = NEXUS_AudioOutput_SetSettings(NEXUS_AudioDac_GetConnector(_outputAudioDac), &outputSettings);
            CHECK_NEXUS_ERROR_ASSERT("error setting audio output settings", nError);
        }
        setVolume(NEXUS_AUDIO_VOLUME_LINEAR_NORMAL);
    }

error:
    return(ret);
} /* initialize */

NEXUS_AudioOutput COutputAudioDac::getConnectorA()
{
    BDBG_ASSERT(_outputAudioDac);
    return(NEXUS_AudioDac_GetConnector(_outputAudioDac));
}

COutputAudioDacI2s::COutputAudioDacI2s(
        const char *     name,
        const uint16_t   number,
        CConfiguration * pCfg
        ) :
    COutputAudioDac(name, number, eBoardResource_outputAudioDacI2s, pCfg),
    _outputI2s(NULL)
{
}

COutputAudioDacI2s::~COutputAudioDacI2s()
{
    disconnect();
}

eRet COutputAudioDacI2s::initialize()
{
    CPlatform * pPlatformConfig = _pCfg->getPlatformConfig();
    eRet        ret             = eRet_Ok;
    NEXUS_Error nError          = NEXUS_SUCCESS;

    BDBG_ASSERT(NULL != pPlatformConfig);

    ret = COutput::initialize();
    CHECK_ERROR_GOTO("resource initialization failed", ret, error);

    if (NULL != pPlatformConfig->getOutputI2sDac(_number))
    {
        int outputDelay = 0;

        _outputI2s  = pPlatformConfig->getOutputI2sDac(_number);
        outputDelay = GET_INT(_pCfg, DAC_OUTPUT_DELAY);
        if (0 < outputDelay)
        {
            NEXUS_AudioOutputSettings outputSettings;
            NEXUS_AudioOutput_GetSettings(NEXUS_I2sOutput_GetConnector(_outputI2s), &outputSettings);
            outputSettings.additionalDelay = outputDelay;
            BDBG_WRN(("Audio DAC output delay %d", outputSettings.additionalDelay));
            nError = NEXUS_AudioOutput_SetSettings(NEXUS_I2sOutput_GetConnector(_outputI2s), &outputSettings);
            CHECK_NEXUS_ERROR_ASSERT("error setting audio output settings", nError);
        }
        setVolume(NEXUS_AUDIO_VOLUME_LINEAR_NORMAL);
    }
error:
    return(ret);
} /* initialize */

NEXUS_AudioOutput COutputAudioDacI2s::getConnectorA()
{
    BDBG_ASSERT(_outputI2s);
    return(NEXUS_I2sOutput_GetConnector(_outputI2s));
}

COutputAudioDummy::COutputAudioDummy(
        const char *     name,
        const uint16_t   number,
        CConfiguration * pCfg
        ) :
    COutput(name, number, eBoardResource_outputAudioDummy, pCfg),
    _outputAudioDummy(NULL)
{
}

COutputAudioDummy::~COutputAudioDummy()
{
    disconnect();
}

eRet COutputAudioDummy::initialize()
{
    CPlatform * pPlatformConfig = _pCfg->getPlatformConfig();
    eRet        ret             = eRet_Ok;

    BDBG_ASSERT(NULL != pPlatformConfig);

    ret = COutput::initialize();
    CHECK_ERROR_GOTO("resource initialization failed", ret, error);

    BDBG_MSG(("Audio Output Dummy output number %d", _number));
#if NEXUS_NUM_AUDIO_DUMMY_OUTPUTS
    CHECK_PTR_ERROR_GOTO("Dummy output is Null", pPlatformConfig->getOutputAudioDummy(_number), ret, eRet_NotAvailable, error);
    _outputAudioDummy = pPlatformConfig->getOutputAudioDummy(_number);
#else
    ret = eRet_NotSupported;
#endif

error:
    return(ret);
} /* init Audio Dummy */

NEXUS_AudioOutput COutputAudioDummy::getConnectorA()
{
    BDBG_ASSERT(_outputAudioDummy);
    return(NEXUS_AudioDummyOutput_GetConnector(_outputAudioDummy));
}