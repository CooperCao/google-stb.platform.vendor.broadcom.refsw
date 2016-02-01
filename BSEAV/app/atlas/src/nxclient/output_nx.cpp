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

#include "output_nx.h"
#include "atlas_os.h"
#include "convert.h"

#define CALLBACK_HDMI_HOTPLUG  "CallbackHdmiHotplug"
#define CALLBACK_HDMI_HDCP     "CallbackHdmiHdcp"
#define CALLBACK_HDMI_CEC      "CallbackHdmiCec"

BDBG_MODULE(atlas_output);

static void nexusHdmiHotplugCallbackNx(
        void * context,
        int    param
        )
{
    eRet                    ret         = eRet_Ok;
    NEXUS_Error             nerror      = NEXUS_SUCCESS;
    COutputHdmiNx *         pOutputHdmi = (COutputHdmiNx *)context;
    NxClient_CallbackStatus statusCallback;

    BSTD_UNUSED(param);
    BDBG_ASSERT(NULL != pOutputHdmi);

    nerror = NxClient_GetCallbackStatus(&statusCallback);
    CHECK_NEXUS_ERROR_GOTO("unable to get nxclient callback status", ret, nerror, error);

    if (true == pOutputHdmi->isValidHotplug(statusCallback.hdmiOutputHotplug))
    {
        CWidgetEngine * pWidgetEngine = pOutputHdmi->getWidgetEngine();
        if (NULL != pWidgetEngine)
        {
            pWidgetEngine->syncCallback(pOutputHdmi, CALLBACK_HDMI_HOTPLUG);
        }
    }

error:
    return;
} /* nexusHdmiHotplugCallbackNx */

static void nexusHdcpStatusCallbackNx(
        void * context,
        int    param
        )
{
    eRet                    ret         = eRet_Ok;
    NEXUS_Error             nerror      = NEXUS_SUCCESS;
    COutputHdmiNx *         pOutputHdmi = (COutputHdmiNx *)context;
    NxClient_CallbackStatus statusCallback;

    BSTD_UNUSED(param);
    BDBG_ASSERT(NULL != pOutputHdmi);

    nerror = NxClient_GetCallbackStatus(&statusCallback);
    CHECK_NEXUS_ERROR_GOTO("unable to get nxclient callback status", ret, nerror, error);

    if (true == pOutputHdmi->isValidHdcp(statusCallback.hdmiOutputHdcpChanged))
    {
        CWidgetEngine * pWidgetEngine = pOutputHdmi->getWidgetEngine();
        if (NULL != pWidgetEngine)
        {
            pWidgetEngine->syncCallback(pOutputHdmi, CALLBACK_HDMI_HDCP);
        }
    }
error:
    return;
} /* nexusHdcpStatusCallbackNx */

#if 0
static void nexusHdmiCecCallbackNx(
        void * context,
        int    param
        )
{
    COutputHdmiNx * pOutputHdmi = (COutputHdmiNx *)context;

    BSTD_UNUSED(param);
    BDBG_ASSERT(NULL != pOutputHdmi);

    CWidgetEngine * pWidgetEngine = pOutputHdmi->getWidgetEngine();

    if (NULL != pWidgetEngine)
    {
        pWidgetEngine->syncCallback(pOutputHdmi, CALLBACK_HDMI_CEC);
    }
}
#endif

/*
 *  Callback from the bwidgets main loop - io trigger is complete and we can
 *  safely handle hot plug callback
 */
static void bwinHdmiHotplugCallbackNx(
        void *       pObject,
        const char * strCallback
        )
{
    COutputHdmiNx *        pOutputHdmi = (COutputHdmiNx *)pObject;
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
static void bwinHdmiHdcpCallbackNx(
        void *       pObject,
        const char * strCallback
        )
{
    COutputHdmiNx * pOutputHdmi = (COutputHdmiNx *)pObject;

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
static void bwinHdmiCecCallbackNx(
        void *       pObject,
        const char * strCallback
        )
{
    BSTD_UNUSED(pObject);
    BSTD_UNUSED(strCallback);

    BDBG_WRN(("unhandled HDMI CEC callback"));
}

COutputHdmiNx::COutputHdmiNx(
        const char *     name,
        const uint16_t   number,
        CConfiguration * pCfg
        ) :
    COutputHdmi(name, number, pCfg),
    _countHotplug(0),
    _countHdcp(0)
{
    memset(&_statusCallback, 0, sizeof(_statusCallback));
}

COutputHdmiNx::~COutputHdmiNx()
{
}

eRet COutputHdmiNx::initialize()
{
    eRet        ret             = eRet_Ok;
    CPlatform * pPlatformConfig = _pCfg->getPlatformConfig();
    NEXUS_Error nError;

    BDBG_ASSERT(NULL != pPlatformConfig);
    ret = COutput::initialize();
    CHECK_ERROR_GOTO("resource initialization failed", ret, error);

    _outputHdmi = NEXUS_HdmiOutput_Open(NEXUS_ALIAS_ID + 0, NULL);
    CHECK_PTR_ERROR_GOTO("unable to open read only hdmi output handle", _outputHdmi, ret, eRet_NotAvailable, error);

    /* setup hdmi callback thread */
    {
        NxClient_CallbackThreadSettings settingsCallback;
        NxClient_GetDefaultCallbackThreadSettings(&settingsCallback);

        settingsCallback.hdmiOutputHotplug.callback     = nexusHdmiHotplugCallbackNx;
        settingsCallback.hdmiOutputHotplug.context      = this;
        settingsCallback.hdmiOutputHdcpChanged.callback = nexusHdcpStatusCallbackNx;
        settingsCallback.hdmiOutputHdcpChanged.context  = this;
        nError = NxClient_StartCallbackThread(&settingsCallback);
        CHECK_NEXUS_ERROR_GOTO("unable to start nxclient hdmi callback thread", ret, nError, error);
    }

    /* manually  hotplug event to initialize */
    bwinHdmiHotplugCallbackNx(this, NULL);

error:
    return(ret);
} /* initialize */

void COutputHdmiNx::uninitialize()
{
    setWidgetEngine(NULL);
    _timerHdcpRetry.stop();

    NxClient_StopCallbackThread();
} /* uninitialize */

void COutputHdmiNx::setWidgetEngine(CWidgetEngine * pWidgetEngine)
{
    eRet ret = eRet_Ok;

    if (NULL != pWidgetEngine)
    {
        ret = pWidgetEngine->addCallback(this, CALLBACK_HDMI_HOTPLUG, bwinHdmiHotplugCallbackNx);
        CHECK_ERROR("unable to add callback to widget engine", ret);
        ret = pWidgetEngine->addCallback(this, CALLBACK_HDMI_HDCP, bwinHdmiHdcpCallbackNx);
        CHECK_ERROR("unable to add callback to widget engine", ret);
        ret = pWidgetEngine->addCallback(this, CALLBACK_HDMI_CEC, bwinHdmiCecCallbackNx);
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

bool COutputHdmiNx::isValidHotplug(uint16_t count)
{
    bool bChange = false;

    if (_countHotplug != count)
    {
        _countHotplug = count;
        bChange       = true;
    }

    return(bChange);
}

bool COutputHdmiNx::isValidHdcp(uint16_t count)
{
    bool bChange = false;

    if (_countHdcp != count)
    {
        _countHdcp = count;
        bChange    = true;
    }

    return(bChange);
}

eRet COutputHdmiNx::setColorSpace(NEXUS_ColorSpace colorSpace)
{
    BSTD_UNUSED(colorSpace);
    BDBG_WRN(("colorspace cannot be set using the hdmi output in nxclient mode"));
    return(eRet_NotAvailable);
}

CAudioVolume COutputHdmiNx::getVolume()
{
    NxClient_AudioSettings settings;
    CAudioVolume           volume;

    settings.volumeType = NEXUS_AudioVolumeType_eLinear;
    NxClient_GetAudioSettings(&settings);
    volume._volumeType = settings.hdmi.volumeType;
    volume._left       = settings.hdmi.leftVolume;
    volume._right      = settings.hdmi.rightVolume;
    volume._muted      = settings.hdmi.muted;

    return(volume);
}

eRet COutputHdmiNx::setVolume(CAudioVolume volume)
{
    NxClient_AudioSettings settings;
    NEXUS_Error            nerror = NEXUS_SUCCESS;
    eRet                   ret    = eRet_Ok;

    NxClient_GetAudioSettings(&settings);
    settings.hdmi.volumeType  = volume._volumeType;
    settings.hdmi.leftVolume  = volume._left;
    settings.hdmi.rightVolume = volume._right;
    settings.hdmi.muted       = volume._muted;

    nerror = NxClient_SetAudioSettings(&settings);
    CHECK_NEXUS_ERROR_GOTO("unable to set nxclient audio settings", ret, nerror, error);

    notifyObservers(eNotify_VolumeChanged, &volume);
error:
    return(ret);
} /* setVolume */

eRet COutputHdmiNx::setVolume(
        int32_t               level,
        NEXUS_AudioVolumeType type
        )
{
    CAudioVolume volume;

    volume._volumeType = type;
    volume._left       = level;
    volume._right      = level;

    return(setVolume(volume));
}

bool COutputHdmiNx::getMute()
{
    return(false);
}

eRet COutputHdmiNx::setMute(bool bMute)
{
    CAudioVolume volume = getVolume();
    eRet         ret    = eRet_Ok;

    volume._muted = bMute;

    ret = setVolume(volume);
    notifyObservers(eNotify_MuteChanged, &volume);

    return(ret);
}