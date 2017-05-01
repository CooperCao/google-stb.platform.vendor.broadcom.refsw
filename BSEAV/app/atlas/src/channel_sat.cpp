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

#include "channel_sat.h"
#include "channelmgr.h"
#include "nexus_core_utils.h"
#include "tuner_sat.h"

BDBG_MODULE(atlas_channel_sat);

const char * g_nexusSatModes[NEXUS_FrontendSatelliteMode_eMax] = { /* index: Nexus Satellite Modes */
    "dvb",
    "dss",
    "dcii",
    "qpskturbo",
    "8pskturbo",
    "turbo",
    "8pskldpc",
    "qpskldpc",
    "ldpc",
    "blind"
};

NEXUS_FrontendSatelliteMode CChannelSat::stringToNexusSatMode(const char * mode)
{
    /* Helper Tuner functions */
    if (!strncmp(mode, "", 1))
    {
        BDBG_WRN(("No Satellite Mode set"));
        return(NEXUS_FrontendSatelliteMode_eMax);
    }
    if (!strncmp(mode, "dvb", 3))
    {
        return(NEXUS_FrontendSatelliteMode_eDvb);
    }
    if (!strncmp(mode, "dss", 3))
    {
        return(NEXUS_FrontendSatelliteMode_eDss);
    }
    if (!strncmp(mode, "dcii", 4))
    {
        return(NEXUS_FrontendSatelliteMode_eDcii);
    }
    if (!strncmp(mode, "qpskturbo", 9))
    {
        return(NEXUS_FrontendSatelliteMode_eQpskTurbo);
    }
    if (!strncmp(mode, "8pskturbo", 9))
    {
        return(NEXUS_FrontendSatelliteMode_e8pskTurbo);
    }
    if (!strncmp(mode, "turbo", 5))
    {
        return(NEXUS_FrontendSatelliteMode_eTurbo);
    }
    if (!strncmp(mode, "8pskldpc", 8))
    {
        return(NEXUS_FrontendSatelliteMode_e8pskLdpc);
    }
    if (!strncmp(mode, "qpskldpc", 8))
    {
        return(NEXUS_FrontendSatelliteMode_eQpskLdpc);
    }
    if (!strncmp(mode, "ldpc", 4))
    {
        return(NEXUS_FrontendSatelliteMode_eLdpc);
    }
    if (!strncmp(mode, "blind", 5))
    {
        return(NEXUS_FrontendSatelliteMode_eBlindAcquisition);
    }

    BDBG_ERR(("Cannot find Satellite Mode %s", mode));
    return(NEXUS_FrontendSatelliteMode_eMax);
} /* stringToNexusSatMode */

const char * CChannelSat::nexusSatModeToString(NEXUS_FrontendSatelliteMode mode)
{
    if (mode >= NEXUS_FrontendSatelliteMode_eMax)
    {
        return(NULL);
    }

    return(g_nexusSatModes[(int)mode]);
}

CChannelSat::CChannelSat(
        CConfiguration * pCfg,
        CTunerSat *      pTuner
        ) :
    CChannel("CChannelSat", eBoardResource_frontendSds, pCfg)
{
    setTunerRequired(true);

    memset(&_settings, 0, sizeof(satSettings));

#if NEXUS_HAS_FRONTEND
    setTuner(pTuner);
#endif
    NEXUS_Frontend_GetDefaultSatelliteSettings(&_settings.sat);
}

CChannelSat::~CChannelSat()
{
}

CChannelSat::CChannelSat(const CChannelSat & chSat) :
    CChannel(chSat),
    _settings(chSat._settings)
{
    /* copy constructor will not copy any unique objects like tuner */
}

CChannel * CChannelSat::createCopy(CChannel * pChannel)
{
    CChannelSat * pChNew = NULL;

    pChNew = new CChannelSat(*(CChannelSat *)pChannel);
    return(pChNew);
}

eRet CChannelSat::readXML(MXmlElement * xmlElemChannel)
{
    eRet    ret = eRet_Ok;
    MString strMode;
    MString strSymRate;
    MString strDiseqc;
    MString strFreq;
    MString strTone;
    MString strAdc;

    CChannel::readXML(xmlElemChannel);

    /* Maybe overwritten below when stream element is read. tspsi cannot
     * read anything other then TS */
    setTransportType(NEXUS_TransportType_eTs);

    strMode            = xmlElemChannel->attrValue(XML_ATT_MODE);
    _settings.sat.mode = stringToNexusSatMode(strMode.s());
    if (_settings.sat.mode == NEXUS_FrontendSatelliteMode_eMax)
    {
        goto error;
    }

    BDBG_MSG(("Mode is %d ", _settings.sat.mode));

    strSymRate = xmlElemChannel->attrValue(XML_ATT_SYMRATE);
    if (strSymRate.isEmpty())
    {
        BDBG_MSG(("No SymRate is set, Default to 20M"));
        _settings.sat.symbolRate = 20000000;
    }
    else
    {
        _settings.sat.symbolRate = strSymRate.toFloat()* 1000000;
        BDBG_MSG(("SYMBOLRATE is successfully set to %d", _settings.sat.symbolRate));
    }

    strDiseqc = xmlElemChannel->attrValue(XML_ATT_DISEQC);
    if (strDiseqc.isEmpty())
    {
        BDBG_MSG(("Diseqc is not set, default to 13V"));
        _settings.diseqc.voltage = NEXUS_FrontendDiseqcVoltage_e13v;
    }
    else
    {
        switch (strDiseqc.toInt())
        {
        case 13:
            _settings.diseqc.voltage = NEXUS_FrontendDiseqcVoltage_e13v;
            break;
        case 18:
            _settings.diseqc.voltage = NEXUS_FrontendDiseqcVoltage_e18v;
            break;
        default:
            _settings.diseqc.voltage = NEXUS_FrontendDiseqcVoltage_e13v;
            BDBG_ERR(("Not Supported Voltage %d, set to Default 13V", strDiseqc.toInt()));
            break;
        } /* switch */
        BDBG_MSG(("Diseqc Voltage set to %dV", strDiseqc.toInt()));
    }

    strFreq = xmlElemChannel->attrValue(XML_ATT_FREQ);
    if (strFreq.isEmpty())
    {
        BDBG_ERR(("Freq is not set, This channel will Fail"));
        goto error;
    }
    else
    {
        _settings.sat.frequency = (strFreq.toFloat() * 1000000);
        BDBG_MSG(("Freq is successfully set to %d", _settings.sat.frequency));
    }

    strTone = xmlElemChannel->attrValue(XML_ATT_TONE);
    if ((strTone == "true") || (strTone == "on"))
    {
        _settings.diseqc.toneEnabled = true;
    }
    else
    if ((strTone == "false") || (strTone == "off"))
    {
        _settings.diseqc.toneEnabled = false;
    }
    else
    {
        BDBG_ERR(("Must be fixed for Sat Channel to work!"));
        BDBG_ERR(("Tone is not a correct value: %s", strTone.s()));
        goto error;
    }

    strAdc = xmlElemChannel->attrValue(XML_ATT_ADC);
    if (0 <= strAdc.toInt())
    {
        _settings.runSettings.selectedAdc = strAdc.toInt();
    }
    else
    {
        _settings.runSettings.selectedAdc = 0;
        BDBG_ERR(("ADC must be >= 0 - default to 0"));
    }

    {
        MXmlElement * xmlElemStream = xmlElemChannel->findChild(XML_TAG_STREAM);
        if (NULL != xmlElemStream)
        {
            _pidMgr.readXML(xmlElemStream);
            MString transport = xmlElemStream->attrValue(XML_ATT_TYPE);
            if (!transport.isEmpty())
            {
                BDBG_MSG((" transport is set to %s", transport.s()));
                BDBG_MSG((" If transport is set incorrectly convert will\n "
                          " \tfail and crash!"));
                _pidMgr.setTransportType(stringToTransportType(transport));
                setTransportType(stringToTransportType(transport));
            }
        }
    }
    updateDescription();

    return(ret);

error:
    BDBG_ERR(("Cannot add Satellite Channel"));
    return(eRet_InvalidParameter);
} /* readXML */

void CChannelSat::writeXML(MXmlElement * xmlElemChannel)
{
    BDBG_MSG((" Add SDS channel"));

    CChannel::writeXML(xmlElemChannel);

    xmlElemChannel->addAttr(XML_ATT_TYPE, "sds");
    xmlElemChannel->addAttr(XML_ATT_FREQ, MString(_settings.sat.frequency/1000000));
    xmlElemChannel->addAttr(XML_ATT_MODE, nexusSatModeToString(_settings.sat.mode));
    xmlElemChannel->addAttr(XML_ATT_SYMRATE, MString(_settings.sat.symbolRate/1000000));

    switch (_settings.diseqc.voltage)
    {
    case 0:
        xmlElemChannel->addAttr(XML_ATT_DISEQC, "13");
        break;
    default:
        xmlElemChannel->addAttr(XML_ATT_DISEQC, "18");
        break;
    }

    if (_settings.diseqc.toneEnabled)
    {
        xmlElemChannel->addAttr(XML_ATT_TONE, "true");
    }
    else
    {
        xmlElemChannel->addAttr(XML_ATT_TONE, "false");
    }

    xmlElemChannel->addAttr(XML_ATT_ADC, MString(_settings.runSettings.selectedAdc));

    _pidMgr.writeXML(xmlElemChannel);
} /* writeXML */

eRet CChannelSat::initialize(PROGRAM_INFO_T * pProgramInfo)
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

void CChannelSat::setFrequency(uint32_t frequency)
{
    _settings.sat.frequency = frequency;
    updateDescription();
}

void CChannelSat::updateDescription()
{
    char strFreq[16];
    CChannel::updateDescription();

    snprintf(strFreq, 16, "%g", (_settings.sat.frequency / 1000000.0));

    _strDescription  = boardResourceToString(getType());
    _strDescription += MString(" ") + MString(strFreq);
    _strDescription += "MHz";

    _strDescriptionShort = _strDescription;

    addMetadata("Description", _strDescription);
    addMetadata("Frequency", MString(strFreq) + " MHz");
    addMetadata("Mode", nexusSatModeToString(getMode()));
} /* updateDescription */

eRet CChannelSat::tune(
        void *    id,
        CConfig * pConfig,
        bool      bWaitForLock,
        uint16_t  index
        )
{
    eRet                                   ret    = eRet_Ok;
    NEXUS_Error                            nerror = NEXUS_SUCCESS;
    NEXUS_FrontendUserParameters           userParams;
    CInputBand *                           pInputBand          = NULL;
    CParserBand *                          pParserBand         = NULL;
    CTunerSat *                            pTuner              = NULL;
    CBoardResources *                      pBoardResources     = NULL;
    CConfiguration *                       pCfg                = NULL;
    bool                                   bLocalTunerCheckout = false;
    NEXUS_FrontendDiseqcSettings           diseqcSettings;
    NEXUS_FrontendSatelliteRuntimeSettings runSettings;

    BDBG_ASSERT(NULL != pConfig);
    pBoardResources = pConfig->getBoardResources();
    pCfg            = pConfig->getCfg();

    pTuner = (CTunerSat *)getTuner();
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
        pTuner = (CTunerSat *)pBoardResources->checkoutResource(id, eBoardResource_frontendSds, index);
        CHECK_PTR_ERROR_GOTO("unable to checkout SAT tuner", pTuner, ret, eRet_NotAvailable, done);

        ret = pTuner->open(_pWidgetEngine);
        CHECK_ERROR_GOTO("unable to open SAT tuner", ret, done);

        ret = pTuner->acquire();
        CHECK_ERROR_GOTO("unable to acquire SAT tuner", ret, done);

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

    diseqcSettings.toneEnabled = _settings.diseqc.toneEnabled;
    diseqcSettings.voltage     = _settings.diseqc.voltage;

    NEXUS_Frontend_GetDiseqcSettings(pTuner->getFrontend(), &_settings.diseqc);
    _settings.diseqc.enabled     = pTuner->isDiseqcSupported();
    _settings.diseqc.toneEnabled = diseqcSettings.toneEnabled;
    _settings.diseqc.voltage     = diseqcSettings.voltage;

    NEXUS_FrontendDevice_GetSatelliteCapabilities(NEXUS_Frontend_GetDevice(pTuner->getFrontend()), &_settings.satCapabilities);
    if (_settings.satCapabilities.numAdc > 0)
    {
        runSettings.selectedAdc = _settings.runSettings.selectedAdc;
        NEXUS_Frontend_GetSatelliteRuntimeSettings(pTuner->getFrontend(), &_settings.runSettings);
        _settings.runSettings.selectedAdc = runSettings.selectedAdc;
    }

    BDBG_MSG(("CH NUMBER: %d.%d", getMajor(), getMinor()));
    BDBG_MSG(("diseqc:%s mode:%d freq:%u  symbolRate:%u voltage:%s toneEnabled:%s",
              (_settings.diseqc.enabled ? "true" : "false"), _settings.sat.mode, _settings.sat.frequency, _settings.sat.symbolRate, (_settings.diseqc.voltage) ? "18V" : "13V", (_settings.diseqc.toneEnabled) ? "yes" : "no"));

    ret = pTuner->tune(_settings);
    CHECK_ERROR_GOTO("tuning error!", ret, error);

    _tuned = true;

    if (true == bWaitForLock)
    {
        B_Error berror  = B_ERROR_SUCCESS;
        int     timeout = GET_INT(pCfg, TUNE_SAT_TIMEOUT);

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

eRet CChannelSat::unTune(
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

bool CChannelSat::operator ==(CChannel &other)
{
    CChannelSat * pOtherSat = (CChannelSat *)&other;

    /* check base class equivalency first */
    if (true == CChannel::operator ==(* pOtherSat))
    {
        if ((pOtherSat->getFrequency() == getFrequency()) &&
            (pOtherSat->getMode() == getMode()))
        {
            return(true);
        }
    }

    return(false);
}

#endif /* NEXUS_HAS_FRONTEND */