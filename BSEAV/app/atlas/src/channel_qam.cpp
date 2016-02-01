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

#include "channel_qam.h"
#include "channelmgr.h"
#include "nexus_core_utils.h"
#include "tuner_qam.h"

BDBG_MODULE(atlas_channel_qam);

#ifdef MPOD_SUPPORT
static NEXUS_FrontendHandle g_frontendHandle;
#endif

STRING_TO_ENUM_INIT_CPP(CChannelQam, stringToNexusQamMode, NEXUS_FrontendQamMode)
STRING_TO_ENUM_START()
STRING_TO_ENUM_ENTRY("auto", NEXUS_FrontendQamMode_eAuto_64_256)
STRING_TO_ENUM_ENTRY("16", NEXUS_FrontendQamMode_e16)
STRING_TO_ENUM_ENTRY("32", NEXUS_FrontendQamMode_e32)
STRING_TO_ENUM_ENTRY("64", NEXUS_FrontendQamMode_e64)
STRING_TO_ENUM_ENTRY("128", NEXUS_FrontendQamMode_e128)
STRING_TO_ENUM_ENTRY("256", NEXUS_FrontendQamMode_e256)
STRING_TO_ENUM_ENTRY("512", NEXUS_FrontendQamMode_e512)
STRING_TO_ENUM_ENTRY("1024", NEXUS_FrontendQamMode_e1024)
STRING_TO_ENUM_ENTRY("2048", NEXUS_FrontendQamMode_e2048)
STRING_TO_ENUM_ENTRY("4096", NEXUS_FrontendQamMode_e4096)
STRING_TO_ENUM_END(NEXUS_FrontendQamMode)

ENUM_TO_MSTRING_INIT_CPP(CChannelQam, nexusQamModeToString, NEXUS_FrontendQamMode)
ENUM_TO_MSTRING_START()
ENUM_TO_MSTRING_ENTRY(NEXUS_FrontendQamMode_eAuto_64_256, "auto")
ENUM_TO_MSTRING_ENTRY(NEXUS_FrontendQamMode_e16, "16")
ENUM_TO_MSTRING_ENTRY(NEXUS_FrontendQamMode_e32, "32")
ENUM_TO_MSTRING_ENTRY(NEXUS_FrontendQamMode_e64, "64")
ENUM_TO_MSTRING_ENTRY(NEXUS_FrontendQamMode_e128, "128")
ENUM_TO_MSTRING_ENTRY(NEXUS_FrontendQamMode_e256, "256")
ENUM_TO_MSTRING_ENTRY(NEXUS_FrontendQamMode_e512, "512")
ENUM_TO_MSTRING_ENTRY(NEXUS_FrontendQamMode_e1024, "1024")
ENUM_TO_MSTRING_ENTRY(NEXUS_FrontendQamMode_e2048, "2048")
ENUM_TO_MSTRING_ENTRY(NEXUS_FrontendQamMode_e4096, "4096")
ENUM_TO_MSTRING_END()

/* todo: fill in defaults for other modes "0" */
ENUM_TO_MSTRING_INIT_CPP(CChannelQam, nexusQamModeToSymbolRate, NEXUS_FrontendQamMode)
ENUM_TO_MSTRING_START()
ENUM_TO_MSTRING_ENTRY(NEXUS_FrontendQamMode_eAuto_64_256, "5360537")
ENUM_TO_MSTRING_ENTRY(NEXUS_FrontendQamMode_e16, "0")
ENUM_TO_MSTRING_ENTRY(NEXUS_FrontendQamMode_e32, "0")
ENUM_TO_MSTRING_ENTRY(NEXUS_FrontendQamMode_e64, "5056900")
ENUM_TO_MSTRING_ENTRY(NEXUS_FrontendQamMode_e128, "0")
ENUM_TO_MSTRING_ENTRY(NEXUS_FrontendQamMode_e256, "5360537")
ENUM_TO_MSTRING_ENTRY(NEXUS_FrontendQamMode_e512, "0")
ENUM_TO_MSTRING_ENTRY(NEXUS_FrontendQamMode_e1024, "0")
ENUM_TO_MSTRING_ENTRY(NEXUS_FrontendQamMode_e2048, "0")
ENUM_TO_MSTRING_ENTRY(NEXUS_FrontendQamMode_e4096, "0")
ENUM_TO_MSTRING_END()

STRING_TO_ENUM_INIT_CPP(CChannelQam, stringToNexusQamAnnex, NEXUS_FrontendQamAnnex)
STRING_TO_ENUM_START()
STRING_TO_ENUM_ENTRY("A", NEXUS_FrontendQamAnnex_eA)
STRING_TO_ENUM_ENTRY("B", NEXUS_FrontendQamAnnex_eB)
STRING_TO_ENUM_ENTRY("C", NEXUS_FrontendQamAnnex_eC)
STRING_TO_ENUM_END(NEXUS_FrontendQamAnnex)

ENUM_TO_MSTRING_INIT_CPP(CChannelQam, nexusQamAnnexToString, NEXUS_FrontendQamAnnex)
ENUM_TO_MSTRING_START()
ENUM_TO_MSTRING_ENTRY(NEXUS_FrontendQamAnnex_eA, "A")
ENUM_TO_MSTRING_ENTRY(NEXUS_FrontendQamAnnex_eB, "B")
ENUM_TO_MSTRING_ENTRY(NEXUS_FrontendQamAnnex_eC, "C")
ENUM_TO_MSTRING_END()

CChannelQam::CChannelQam(
        CConfiguration * pCfg,
        CTunerQam *      pTuner
        ) :
    CChannel("CChannelQam", eBoardResource_frontendQam, pCfg)
{
    setTunerRequired(true);

#if NEXUS_HAS_FRONTEND
    setTuner(pTuner);
#endif
    memset(&_settings, 0, sizeof(_settings));
    NEXUS_Frontend_GetDefaultQamSettings(&_settings);
}

CChannelQam::CChannelQam(const CChannelQam & chQam) :
    CChannel(chQam),
    _settings(chQam._settings)
{
    /* copy constructor will not copy any unique objects like tuner */
}

CChannel * CChannelQam::createCopy(CChannel * pChannel)
{
    CChannelQam * pChNew = NULL;

    pChNew = new CChannelQam(*(CChannelQam *)pChannel);
    return(pChNew);
}

eRet CChannelQam::readXML(MXmlElement * xmlElemChannel)
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
        BDBG_WRN(("No QAM Mode is not set."));
        _settings.mode = NEXUS_FrontendQamMode_eMax;
        goto error;
    }

    _settings.mode = stringToNexusQamMode(strMode.s());
    if (_settings.mode == NEXUS_FrontendQamMode_eMax)
    {
        BDBG_ERR(("channel mode is incorrect!"));
        goto error;
    }

    strBandwidth = xmlElemChannel->attrValue(XML_ATT_BANDWIDTH);
    if (strBandwidth.isEmpty())
    {
        BDBG_MSG(("No Symbol rate defined set to default"));
        _settings.bandwidth = 0;
    }
    else
    {
        _settings.bandwidth = strBandwidth.toInt();
    }

    strSymRate = xmlElemChannel->attrValue(XML_ATT_SYMRATE);
    if (strSymRate.isEmpty())
    {
        BDBG_MSG(("No Symbol rate defined set to default"));
        _settings.symbolRate = nexusQamModeToSymbolRate(_settings.mode).toLong();
    }
    else
    {
        _settings.symbolRate = strSymRate.toInt();
    }

    strAnnex = xmlElemChannel->attrValue(XML_ATT_ANNEX);
    if (strAnnex.isEmpty())
    {
        BDBG_MSG(("Annex is not defined, Default to Annex B"));
        _settings.annex = NEXUS_FrontendQamAnnex_eB;
    }
    else
    {
        /* set to Default Value First */
        _settings.annex = stringToNexusQamAnnex(strAnnex);
        BDBG_MSG(("Annex is set to %s", strAnnex.s()));
    }

    strFreq = xmlElemChannel->attrValue(XML_ATT_FREQ);
    if (strFreq.isEmpty())
    {
        BDBG_MSG(("Freq is not set, default to 549Mhz"));
        _settings.frequency = (549 * 1000000);
    }
    else
    {
        _settings.frequency = strFreq.toInt();
        BDBG_MSG(("Freq is successfully set to %d", getFrequency()));
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

void CChannelQam::writeXML(MXmlElement * xmlElemChannel)
{
    BDBG_MSG((" Write QAM channel"));
    CChannel::writeXML(xmlElemChannel);

    xmlElemChannel->addAttr(XML_ATT_TYPE, "qam");
    xmlElemChannel->addAttr(XML_ATT_FREQ, MString(_settings.frequency));
    xmlElemChannel->addAttr(XML_ATT_MODE, nexusQamModeToString(_settings.mode));
    xmlElemChannel->addAttr(XML_ATT_BANDWIDTH, MString(_settings.bandwidth));
    xmlElemChannel->addAttr(XML_ATT_SYMRATE, MString(_settings.symbolRate));
    xmlElemChannel->addAttr(XML_ATT_ANNEX, nexusQamAnnexToString(_settings.annex));

    _pidMgr.writeXML(xmlElemChannel);
} /* writeXML */

CChannelQam::~CChannelQam()
{
}

eRet CChannelQam::initialize(PROGRAM_INFO_T * pProgramInfo)
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

void CChannelQam::setFrequency(uint32_t frequency)
{
    _settings.frequency = frequency;
    updateDescription();
}

void CChannelQam::updateDescription()
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
eRet CChannelQam::getChannelInfo(
        CHANNEL_INFO_T * pChanInfo,
        bool             bScanning
        )
{
    BERR_Code     err            = BERR_SUCCESS;
    CParserBand * pBand          = getParserBand();
    int           patTimeout     = GET_INT(_pCfg, TUNE_QAM_PAT_TIMEOUT)/10; /* in tsPsi_setTimeout2() this is 500msecs */
    int           patTimeoutOrig = 0;
    int           pmtTimeout     = GET_INT(_pCfg, TUNE_QAM_PMT_TIMEOUT)/10; /* in tsPsi_setTimeout2() this is 500msecs */
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
#else /* ifndef MPOD_SUPPORT */
    if (true == bScanning)
    {
        patTimeout = 800;
        pmtTimeout = 800;
        /* adjust pat/pmt timeouts for faster scanning */
        tsPsi_getTimeout(&patTimeoutOrig, &pmtTimeoutOrig);
        tsPsi_setTimeout(patTimeout, pmtTimeout);
    }
    err = tsPsi_getChannelInfo(pChanInfo, pBand->getBand());

    if (true == bScanning)
    {
        /* restore default pat/pmt timeouts */
        tsPsi_setTimeout(patTimeoutOrig, pmtTimeoutOrig);
    }
#endif /* ifndef MPOD_SUPPORT */
    return((BERR_SUCCESS == err) ? eRet_Ok : eRet_ExternalError);
} /* getChannelInfo */

eRet CChannelQam::tune(
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
    CTunerQam *                  pTuner              = NULL;
    CBoardResources *            pBoardResources     = NULL;
    CConfiguration *             pCfg                = NULL;
    bool                         bLocalTunerCheckout = false;

    BDBG_ASSERT(NULL != pConfig);
    pBoardResources = pConfig->getBoardResources();
    pCfg            = pConfig->getCfg();

    pTuner = (CTunerQam *)getTuner();
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
        pTuner = (CTunerQam *)pBoardResources->checkoutResource(id, eBoardResource_frontendQam, index);
        CHECK_PTR_ERROR_GOTO("unable to checkout QAM tuner", pTuner, ret, eRet_NotAvailable, done);

        ret = pTuner->open(_pWidgetEngine);
        CHECK_ERROR_GOTO("unable to open QAM tuner", ret, done);

        ret = pTuner->acquire();
        CHECK_ERROR_GOTO("unable to acquire QAM tuner", ret, done);

        pTuner->setConfig(pConfig);
        setTuner(pTuner);
        bLocalTunerCheckout = true;
    }

    nerror = NEXUS_Frontend_GetUserParameters(_pTuner->getFrontend(), &userParams);
    CHECK_NEXUS_ERROR_GOTO("unable to get frontend user parameters", ret, nerror, done);

#ifdef MPOD_SUPPORT
    g_frontendHandle = _pTuner->getFrontend();
#endif
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

    ret = pTuner->tune(_settings);
    CHECK_ERROR_GOTO("tuning error!", ret, error);

    if (true == bWaitForLock)
    {
        B_Error berror  = B_ERROR_SUCCESS;
        int     timeout = GET_INT(pCfg, TUNE_QAM_TIMEOUT);

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

eRet CChannelQam::unTune(
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

bool CChannelQam::operator ==(CChannel &other)
{
    CChannelQam * pOtherQam = (CChannelQam *)&other;

    /* check base class equivalency first */
    if (true == CChannel::operator ==(other))
    {
        if ((pOtherQam->getFrequency() == getFrequency()) &&
            (pOtherQam->getMode() == getMode()) &&
            (pOtherQam->getAnnex() == getAnnex()))
        {
            return(true);
        }
    }

    return(false);
} /* == */

#endif /* NEXUS_HAS_FRONTEND */