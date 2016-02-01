/***************************************************************************
 * (c) 2002-2015 Broadcom Corporation
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

#if NEXUS_HAS_FRONTEND

#include "channel_ofdm.h"
#include "channelmgr.h"
#include "nexus_core_utils.h"
#include "tuner_ofdm.h"

#include "band.h"

#include "nexus_parser_band.h"

BDBG_MODULE(atlas_channel_ofdm);

ENUM_TO_MSTRING_INIT_CPP(CChannelOfdm, nexusOfdmModeToString, NEXUS_FrontendOfdmMode)
ENUM_TO_MSTRING_START()
ENUM_TO_MSTRING_ENTRY(NEXUS_FrontendOfdmMode_eDvbt2, "dvbt2")
ENUM_TO_MSTRING_ENTRY(NEXUS_FrontendOfdmMode_eDvbt, "dvbt")
ENUM_TO_MSTRING_ENTRY(NEXUS_FrontendOfdmMode_eDvbc2, "dvbc2")
ENUM_TO_MSTRING_ENTRY(NEXUS_FrontendOfdmMode_eIsdbt, "isdbt")
ENUM_TO_MSTRING_END()

STRING_TO_ENUM_INIT_CPP(CChannelOfdm, stringToNexusOfdmMode, NEXUS_FrontendOfdmMode)
STRING_TO_ENUM_START()
STRING_TO_ENUM_ENTRY("dvbt2", NEXUS_FrontendOfdmMode_eDvbt2)
STRING_TO_ENUM_ENTRY("dvbt", NEXUS_FrontendOfdmMode_eDvbt)
STRING_TO_ENUM_ENTRY("dvbc2", NEXUS_FrontendOfdmMode_eDvbc2)
STRING_TO_ENUM_ENTRY("isdbt", NEXUS_FrontendOfdmMode_eIsdbt)
STRING_TO_ENUM_END(NEXUS_FrontendOfdmMode)

CChannelOfdm::CChannelOfdm(
        CConfiguration * pCfg,
        CTunerOfdm *     pTuner
        ) :
    CChannel("CChannelOfdm", eBoardResource_frontendOfdm, pCfg)
{
    setTunerRequired(true);

#if NEXUS_HAS_FRONTEND
    setTuner(pTuner);
#endif
    memset(&_settings, 0, sizeof(_settings));
    NEXUS_Frontend_GetDefaultOfdmSettings(&_settings);
}

CChannelOfdm::CChannelOfdm(const CChannelOfdm & chOfdm) :
    CChannel(chOfdm),
    _settings(chOfdm._settings)
{
    /* copy constructor will not copy any unique objects like tuner */
}

CChannel * CChannelOfdm::createCopy(CChannel * pChannel)
{
    CChannelOfdm * pChNew = NULL;

    pChNew = new CChannelOfdm(*(CChannelOfdm *)pChannel);
    return(pChNew);
}

eRet CChannelOfdm::readXML(MXmlElement * xmlElemChannel)
{
    eRet    ret = eRet_Ok;
    MString strBandwidth;
    MString strFreq;

    CChannel::readXML(xmlElemChannel);

    setTransportType(NEXUS_TransportType_eTs);

    NEXUS_Frontend_GetDefaultOfdmSettings(&_settings);
    MString strMode = xmlElemChannel->attrValue(XML_ATT_MODE);
    if (strMode.isEmpty())
    {
        BDBG_WRN(("No Ofdm Mode is not set."));
        _settings.mode = NEXUS_FrontendOfdmMode_eMax;
        goto error;
    }

    _settings.mode = stringToNexusOfdmMode(strMode.s());
    if (_settings.mode == NEXUS_FrontendOfdmMode_eMax)
    {
        goto error;
    }

    strBandwidth = xmlElemChannel->attrValue(XML_ATT_BANDWIDTH);
    if (strBandwidth.isEmpty())
    {
        BDBG_MSG(("No bandwidth defined set to default"));
        _settings.bandwidth = 6000000;
    }
    else
    {
        _settings.bandwidth = strBandwidth.toInt()*1000000;
    }

    strFreq = xmlElemChannel->attrValue(XML_ATT_FREQ);
    if (strFreq.isEmpty())
    {
        BDBG_MSG(("Freq is not set, default to 549Mhz"));
        _settings.frequency = (549 * 1000000);
    }
    else
    {
        _settings.frequency = strFreq.toInt() * 1000000;
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

void CChannelOfdm::writeXML(MXmlElement * xmlElemChannel)
{
    BDBG_MSG((" Write Ofdm channel"));
    CChannel::writeXML(xmlElemChannel);

    xmlElemChannel->addAttr(XML_ATT_TYPE, "ofdm");
    xmlElemChannel->addAttr(XML_ATT_FREQ, MString(_settings.frequency/1000000));
    xmlElemChannel->addAttr(XML_ATT_MODE, nexusOfdmModeToString(_settings.mode));
    xmlElemChannel->addAttr(XML_ATT_BANDWIDTH, MString(_settings.bandwidth/1000000));

    _pidMgr.writeXML(xmlElemChannel);
}

CChannelOfdm::~CChannelOfdm()
{
}

eRet CChannelOfdm::initialize(PROGRAM_INFO_T * pProgramInfo)
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

void CChannelOfdm::setFrequency(uint32_t frequency)
{
    _settings.frequency = frequency;
    updateDescription();
}

void CChannelOfdm::updateDescription()
{
    CChannel::updateDescription();

    _strDescription  = boardResourceToString(getType());
    _strDescription += MString(" ") + MString(_settings.frequency / 1000000);
    _strDescription += "MHz";

    _strDescriptionShort = _strDescription;

    addMetadata("Description", _strDescription);
    addMetadata("Frequency", MString(_settings.frequency / 1000000) + " MHz");
}

/* get PSI channel info for current tuned channel */
eRet CChannelOfdm::getChannelInfo(
        CHANNEL_INFO_T * pChanInfo,
        bool             bScanning
        )
{
    BERR_Code     err            = BERR_SUCCESS;
    CParserBand * pBand          = getParserBand();
    int           patTimeout     = GET_INT(_pCfg, TUNE_OFDM_PAT_TIMEOUT)/10; /* in tsPsi_setTimeout2() this is 500msecs */
    int           patTimeoutOrig = 0;
    int           pmtTimeout     = GET_INT(_pCfg, TUNE_OFDM_PMT_TIMEOUT)/10; /* in tsPsi_setTimeout2() this is 500msecs */
    int           pmtTimeoutOrig = 0;

#ifndef MPOD_SUPPORT
    if (true == bScanning)
    {
        /* adjust pat/pmt timeouts for faster scanning */
        tsPsi_getTimeout2(&patTimeoutOrig, &pmtTimeoutOrig);
        tsPsi_setTimeout2(patTimeout, pmtTimeout);
    }

    err = tsPsi_getChannelInfo2(pChanInfo, pBand->getBand());

    if (true == bScanning)
    {
        /* restore default pat/pmt timeouts */
        tsPsi_setTimeout2(patTimeoutOrig, pmtTimeoutOrig);
    }
#endif /* ifndef MPOD_SUPPORT */
    return((BERR_SUCCESS == err) ? eRet_Ok : eRet_ExternalError);
} /* getChannelInfo */

eRet CChannelOfdm::tune(
        void *    id,
        CConfig * pConfig,
        bool      bWaitForLock,
        uint16_t  index
        )
{
    eRet                         ret    = eRet_Ok;
    NEXUS_Error                  nerror = NEXUS_SUCCESS;
    NEXUS_FrontendUserParameters userParams;
    CInputBand *                 pInputBand      = NULL;
    CParserBand *                pParserBand     = NULL;
    CTunerOfdm *                 pTuner          = NULL;
    CBoardResources *            pBoardResources = NULL;
    CConfiguration *             pCfg            = NULL;
    NEXUS_FrontendOfdmSettings   newSettings;
    bool                         bLocalTunerCheckout = false;

    BDBG_ASSERT(NULL != pConfig);
    pBoardResources = pConfig->getBoardResources();
    pCfg            = pConfig->getCfg();

    pTuner = (CTunerOfdm *)getTuner();
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
        pTuner = (CTunerOfdm *)pBoardResources->checkoutResource(id, eBoardResource_frontendOfdm, index);
        CHECK_PTR_ERROR_GOTO("unable to checkout Ofdm tuner", pTuner, ret, eRet_NotAvailable, done);

        ret = pTuner->open(_pWidgetEngine);
        CHECK_ERROR_GOTO("unable to open OFDM tuner", ret, done);

        ret = pTuner->acquire();
        CHECK_ERROR_GOTO("unable to acquire OFDM tuner", ret, done);

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

    /* Default Settings to reset a couple of fields if OFDM Changes */
    NEXUS_Frontend_GetDefaultOfdmSettings(&newSettings);

    /* These should be set for all modes */
    _settings.acquisitionMode = NEXUS_FrontendOfdmAcquisitionMode_eAuto;
    _settings.spectrum        = NEXUS_FrontendOfdmSpectrum_eAuto;

    /* adjust settings based on mode */
    if (_settings.mode == NEXUS_FrontendOfdmMode_eDvbt)
    {
        _settings.terrestrial           = true;
        _settings.manualTpsSettings     = false;
        _settings.pullInRange           = NEXUS_FrontendOfdmPullInRange_eWide;
        _settings.cciMode               = NEXUS_FrontendOfdmCciMode_eNone;
        _settings.dvbt2Settings.plpMode = newSettings.dvbt2Settings.plpMode;
        _settings.dvbt2Settings.plpId   = newSettings.dvbt2Settings.plpId;
    }
    else
    if (_settings.mode == NEXUS_FrontendOfdmMode_eDvbt2)
    {
        _settings.terrestrial           = true;
        _settings.manualTpsSettings     = newSettings.manualTpsSettings;
        _settings.pullInRange           = newSettings.pullInRange;
        _settings.cciMode               = newSettings.cciMode;
        _settings.dvbt2Settings.plpMode = false;
        _settings.dvbt2Settings.plpId   = 0;
    }
    else
    if (_settings.mode == NEXUS_FrontendOfdmMode_eDvbc2)
    {
        _settings.terrestrial           = false;
        _settings.manualTpsSettings     = newSettings.manualTpsSettings;
        _settings.pullInRange           = newSettings.pullInRange;
        _settings.cciMode               = newSettings.cciMode;
        _settings.dvbc2Settings.plpMode = true;
        _settings.dvbc2Settings.plpId   = 0;
    }
    else
    if (_settings.mode == NEXUS_FrontendOfdmMode_eIsdbt)
    {
        _settings.terrestrial           = true;
        _settings.manualTpsSettings     = newSettings.manualTpsSettings;
        _settings.pullInRange           = newSettings.pullInRange;
        _settings.cciMode               = newSettings.cciMode;
        _settings.dvbt2Settings.plpMode = newSettings.dvbt2Settings.plpMode;
        _settings.dvbt2Settings.plpId   = newSettings.dvbt2Settings.plpId;
    }

    BDBG_MSG(("CH NUMBER: %d.%d", getMajor(), getMinor()));
    BDBG_MSG(("mode:%d freq:%u bandwidth:%d",
              _settings.mode, _settings.frequency, _settings.bandwidth));

    ret = pTuner->tune(&_settings);
    CHECK_ERROR_GOTO("tuning error!", ret, error);

    _tuned = true;

    if (true == bWaitForLock)
    {
        B_Error berror  = B_ERROR_SUCCESS;
        int     timeout = GET_INT(pCfg, TUNE_OFDM_TIMEOUT);

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
            pParserBand->close();
            pBoardResources->checkinResource(pParserBand);
            pParserBand = NULL;
        }
        if (NULL != pInputBand)
        {
            pBoardResources->checkinResource(pInputBand);
            pInputBand = NULL;
        }
    }

done:
    _tuned = (eRet_Ok == ret) ? true : false;
    return(ret);
} /* tune */

eRet CChannelOfdm::unTune(
        CConfig * pConfig,
        bool      bFullUnTune,
        bool      bCheckInTuner
        )
{
    eRet              ret             = eRet_Ok;
    CBoardResources * pBoardResources = NULL;

    BDBG_ASSERT(pConfig);
    pBoardResources = pConfig->getBoardResources();

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

bool CChannelOfdm::operator ==(CChannel &other)
{
    CChannelOfdm * pOtherOfdm = (CChannelOfdm *)&other;

    /* check base class equivalency first */
    if (true == CChannel::operator ==(* pOtherOfdm))
    {
        if ((pOtherOfdm->getFrequency() == getFrequency()) &&
            (pOtherOfdm->getMode() == getMode()))
        {
            return(true);
        }
    }

    return(false);
}

#endif /* NEXUS_HAS_FRONTEND */