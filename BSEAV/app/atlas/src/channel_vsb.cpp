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

#if NEXUS_HAS_FRONTEND

#include "channel_vsb.h"
#include "channelmgr.h"
#include "nexus_core_utils.h"
#include "tuner_vsb.h"

BDBG_MODULE(atlas_channel_vsb);

const char * g_nexusVsbModes[NEXUS_FrontendVsbMode_eMax] = { /* index: Nexus Vsb Modes */
    "8",
    "16"
};

NEXUS_FrontendVsbMode CChannelVsb::stringToNexusVsbMode(const char * mode)
{
    int i = 0;

    i = atoi(mode);

    switch (i)
    {
    case 8:
        return(NEXUS_FrontendVsbMode_e8);

        break;
    case 16:
        return(NEXUS_FrontendVsbMode_e16);

        break;
    default:
        BDBG_WRN(("Not a valid Mode, set this to MAX"));
        return(NEXUS_FrontendVsbMode_eMax);

        break;
    } /* switch */
}     /* stringToNexusVsbMode */

const char * CChannelVsb::nexusVsbModeToString(NEXUS_FrontendVsbMode mode)
{
    if (mode >= NEXUS_FrontendVsbMode_eMax)
    {
        BDBG_ERR((" Invalid mode %d ", mode));
        return(NULL);
    }

    return(g_nexusVsbModes[mode]);
}

CChannelVsb::CChannelVsb(
        CConfiguration * pCfg,
        CTunerVsb *      pTuner
        ) :
    CChannel("CChannelVsb", eBoardResource_frontendVsb, pCfg)
{
    setTunerRequired(true);

#if NEXUS_HAS_FRONTEND
    setTuner(pTuner);
#endif
    memset(&_settings, 0, sizeof(_settings));
    NEXUS_Frontend_GetDefaultVsbSettings(&_settings);
}

CChannelVsb::CChannelVsb(const CChannelVsb & chVsb) :
    CChannel(chVsb),
    _settings(chVsb._settings)
{
    /* copy constructor will not copy any unique objects like tuner */
}

CChannel * CChannelVsb::createCopy(CChannel * pChannel)
{
    CChannelVsb * pChNew = NULL;

    pChNew = new CChannelVsb(*(CChannelVsb *)pChannel);
    return(pChNew);
}

eRet CChannelVsb::readXML(MXmlElement * xmlElemChannel)
{
    eRet    ret = eRet_Ok;
    MString strBandwidth;
    MString strSymRate;
    MString strAnnex;
    MString strFreq;

    CChannel::readXML(xmlElemChannel);

    setTransportType(NEXUS_TransportType_eTs);

    MString strMode = xmlElemChannel->attrValue(XML_ATT_MODE);
    if (strMode.isEmpty())
    {
        BDBG_WRN(("No Vsb Mode is not set."));
        _settings.mode = NEXUS_FrontendVsbMode_eMax;
        goto error;
    }

    _settings.mode = stringToNexusVsbMode(strMode.s());
    if (_settings.mode == NEXUS_FrontendVsbMode_eMax)
    {
        goto error;
    }

    strSymRate = xmlElemChannel->attrValue(XML_ATT_SYMRATE);
    if (strSymRate.isEmpty())
    {
        BDBG_MSG(("No Symbol rate defined set to default"));
        _settings.symbolRate = 0;
    }
    else
    {
        _settings.symbolRate = strSymRate.toInt();
    }

    strFreq = xmlElemChannel->attrValue(XML_ATT_FREQ);
    if (strFreq.isEmpty())
    {
        BDBG_MSG(("Freq is not set, default to 549Mhz"));
        setFrequency(549 * 1000000);
    }
    else
    {
        setFrequency(strFreq.toInt());
        BDBG_MSG(("Freq is successfully set to %d", _settings.frequency));
    }

    {
        MXmlElement * xmlElemStream = xmlElemChannel->findChild(XML_TAG_STREAM);
        if (NULL != xmlElemStream)
        {
            _pidMgr.readXML(xmlElemStream);
        }
    }

    updateDescription();

error:
    return(ret);
} /* readXML */

void CChannelVsb::writeXML(MXmlElement * xmlElemChannel)
{
    BDBG_MSG((" Write Vsb channel"));
    CChannel::writeXML(xmlElemChannel);

    xmlElemChannel->addAttr(XML_ATT_TYPE, "vsb");
    xmlElemChannel->addAttr(XML_ATT_FREQ, MString(_settings.frequency));
    xmlElemChannel->addAttr(XML_ATT_MODE, nexusVsbModeToString(_settings.mode));
    xmlElemChannel->addAttr(XML_ATT_SYMRATE, MString(_settings.symbolRate));

    _pidMgr.writeXML(xmlElemChannel);
}

CChannelVsb::~CChannelVsb()
{
}

eRet CChannelVsb::initialize(PROGRAM_INFO_T * pProgramInfo)
{
    eRet ret = eRet_Ok;

    BDBG_ASSERT(NULL != pProgramInfo);

    setProgramNum(pProgramInfo->program_number);
    _pidMgr.clearPids();
    ret = _pidMgr.createPids(pProgramInfo);
    CHECK_ERROR_GOTO("unable to create pids from program info", ret, error);

error:
    return(ret);
}

void CChannelVsb::setFrequency(uint32_t frequency)
{
    _settings.frequency = frequency;
    updateDescription();
}

void CChannelVsb::updateDescription()
{
    CChannel::updateDescription();

    _strDescription  = boardResourceToString(getType());
    _strDescription += MString(" ") + MString(_settings.frequency / 1000000);
    _strDescription += "MHz";

    _strDescriptionShort = _strDescription;

    addMetadata("Description", _strDescription);
    addMetadata("Frequency", MString(_settings.frequency / 1000000) + " MHz");
}

eRet CChannelVsb::tune(
        void *    id,
        CConfig * pConfig,
        bool      bWaitForLock,
        uint16_t  index
        )
{
    eRet                         ret    = eRet_Ok;
    NEXUS_Error                  nerror = NEXUS_SUCCESS;
    NEXUS_FrontendUserParameters userParams;
    CInputBand *                 pInputBand          = NULL;
    CParserBand *                pParserBand         = NULL;
    CTunerVsb *                  pTuner              = NULL;
    CBoardResources *            pBoardResources     = NULL;
    CConfiguration *             pCfg                = NULL;
    bool                         bLocalTunerCheckout = false;

    BDBG_ASSERT(NULL != pConfig);
    pBoardResources = pConfig->getBoardResources();
    pCfg            = pConfig->getCfg();

    pTuner = (CTunerVsb *)getTuner();
    if ((NULL != pTuner) && (index != pTuner->getNumber()) && (ANY_INDEX != index))
    {
        /* given tuner index does not match currently checked out tuner so check in */
        pTuner->release();
        pTuner->close();
        pBoardResources->checkinResource(pTuner);
        pTuner = NULL;
    }

    if (NULL == pTuner)
    {
        pTuner = (CTunerVsb *)pBoardResources->checkoutResource(id, eBoardResource_frontendVsb, index);
        CHECK_PTR_ERROR_GOTO("unable to checkout Vsb tuner", pTuner, ret, eRet_NotAvailable, done);

        ret = pTuner->open(_pWidgetEngine);
        CHECK_ERROR_GOTO("unable to open VSB tuner", ret, done);

        ret = pTuner->acquire();
        CHECK_ERROR_GOTO("unable to acquire VSB tuner", ret, done);

        pTuner->setConfig(pConfig);
        setTuner(pTuner);
        bLocalTunerCheckout = true;
    }

    nerror = NEXUS_Frontend_GetUserParameters(_pTuner->getFrontend(), &userParams);
    CHECK_NEXUS_ERROR_GOTO("unable to get frontend user parameters", ret, nerror, done);

    /* checkout input band based on frontend user parameters (set by nexus platform) */
    pInputBand = (CInputBand *)pBoardResources->checkoutResource(id, eBoardResource_inputBand, userParams.param1);
    CHECK_PTR_ERROR_GOTO("unable to checkout inputband", pInputBand, ret, eRet_NotAvailable, error);
    setInputBand(pInputBand);

    pParserBand = (CParserBand *)pBoardResources->checkoutResource(id, eBoardResource_parserBand);
    CHECK_PTR_ERROR_GOTO("unable to checkout parser band", pParserBand, ret, eRet_NotAvailable, error);
    pParserBand->open();
    setParserBand(pParserBand);

    ret = mapInputBand(pInputBand);
    CHECK_ERROR_GOTO("error mapping input band to parser band", ret, error);

    BDBG_MSG(("CH NUMBER: %d.%d", getMajor(), getMinor()));
    BDBG_MSG(("mode:%d freq:%u iffreq:%u symbolRate:%u terr:%d",
              _settings.mode, _settings.frequency, _settings.ifFrequency, _settings.symbolRate, _settings.terrestrial));

    ret = pTuner->tune(_settings);
    CHECK_ERROR_GOTO("tuning error!", ret, error);

    _tuned = true;

    if (true == bWaitForLock)
    {
        B_Error berror  = B_ERROR_SUCCESS;
        int     timeout = GET_INT(pCfg, TUNE_VSB_TIMEOUT);

        berror = B_Event_Wait(_pTuner->getWaitEvent(), timeout);
        CHECK_BOS_WARN_GOTO("tune failed to lock", ret, berror, error);

        if ((false == pTuner->getLockState()) && (false == pTuner->isAcquireInProgress()))
        {
            /* wait event could have been set if unlocked but tuner is done searching
             * so we will indicate a failed tune in the return code */
            ret = eRet_NotAvailable;
        }

        pTuner->resetStatus();
    }

error:
    if (eRet_Ok != ret)
    {
        if ((NULL != pTuner) && (true == bLocalTunerCheckout))
        {
            pBoardResources->checkinResource(pTuner);
            setTuner(NULL);
        }

        if (NULL != pParserBand)
        {
            pBoardResources->checkinResource(pParserBand);
        }
        if (NULL != pInputBand)
        {
            pBoardResources->checkinResource(pInputBand);
        }
    }

done:
    _tuned = (eRet_Ok == ret) ? true : false;
    return(ret);
} /* tune */

eRet CChannelVsb::unTune(
        CConfig * pConfig,
        bool      bFullUnTune,
        bool      bCheckInTuner
        )
{
    eRet              ret             = eRet_Ok;
    CBoardResources * pBoardResources = NULL;

    BDBG_ASSERT(pConfig);
    pBoardResources = pConfig->getBoardResources();

    /* we must reset STC to ref count of pids when we do an untune */
    if(_pStc != NULL )
    {
        NEXUS_SimpleStcChannelSettings settings;
        _pStc->getDefaultSettings(&settings);
        _pStc->setSettings(&settings);
    }

    if ((NULL != _pParserBand) && (_pParserBand->isCheckedOut()))
    {
        _pParserBand->close();
        pBoardResources->checkinResource(_pParserBand);
        setParserBand(NULL);
    }

    if ((NULL != _pInputBand) && (_pInputBand->isCheckedOut()))
    {
        pBoardResources->checkinResource(_pInputBand);
        setInputBand(NULL);
    }

    if (NULL != _pTuner)
    {
        _pTuner->unTune(bFullUnTune);

        if ((true == bCheckInTuner) && (true == _pTuner->isCheckedOut()))
        {
            _pTuner->release();
            _pTuner->close();
            pBoardResources->checkinResource(_pTuner);
            setTuner(NULL);
        }
    }

    _tuned = false;

    return(ret);
} /* unTune */

bool CChannelVsb::operator ==(CChannel &other)
{
    CChannelVsb * pOtherVsb = (CChannelVsb *)&other;

    /* check base class equivalency first */
    if (true == CChannel::operator ==(* pOtherVsb))
    {
        if ((pOtherVsb->getFrequency() == getFrequency()) &&
            (pOtherVsb->getMode() == getMode()))
        {
            return(true);
        }
    }

    return(false);
}

#endif /* NEXUS_HAS_FRONTEND */