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

#include "channel_streamer.h"
#include "band.h"

#include "nexus_parser_band.h"

BDBG_MODULE(atlas_channel_streamer);

CChannelStreamer::CChannelStreamer(CConfiguration * pCfg) :
    CChannel("CChannelStreamer", eBoardResource_streamer, pCfg)
{
    setTunerRequired(false);
}

CChannelStreamer::CChannelStreamer(const CChannelStreamer & chStreamer) :
    CChannel(chStreamer)
{
    /* copy constructor will not copy any unique objects like tuner */
}

CChannelStreamer::~CChannelStreamer()
{
}

CChannel * CChannelStreamer::createCopy(CChannel * pChannel)
{
    CChannelStreamer * pChNew = NULL;

    pChNew = new CChannelStreamer(*(CChannelStreamer *)pChannel);
    return(pChNew);
}

eRet CChannelStreamer::readXML(MXmlElement * xmlElemChannel)
{
    eRet ret = eRet_Ok;

    CChannel::readXML(xmlElemChannel);

    setTransportType(NEXUS_TransportType_eTs);

    {
        MXmlElement * xmlElemStream = xmlElemChannel->findChild(XML_TAG_STREAM);
        if (NULL != xmlElemStream)
        {
            _pidMgr.readXML(xmlElemStream);
        }
    }

    updateDescription();

    return(ret);
} /* readXML */

void CChannelStreamer::writeXML(MXmlElement * xmlElemChannel)
{
    BDBG_MSG((" Add streamer channel"));

    CChannel::writeXML(xmlElemChannel);

    /* Create Child PIDS */
    xmlElemChannel->addAttr(XML_ATT_TYPE, "streamer");

    _pidMgr.writeXML(xmlElemChannel);
}

eRet CChannelStreamer::initialize(PROGRAM_INFO_T * pProgramInfo)
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

eRet CChannelStreamer::dupParserBand(CParserBand * pParserBand)
{
    eRet ret = eRet_Ok;

    BDBG_ASSERT(NULL != pParserBand);
    if (_tuned == false)
    {
        BDBG_ERR((" Channel Is not Tuned, Cannot duplicate ParserBand"));
        ret = eRet_ExternalError;
        goto error;
    }

    ret = mapInputBand(_pInputBand, pParserBand);
    CHECK_ERROR_GOTO("error mapping input band to parser band", ret, error);

    BDBG_MSG(("Successfully duplicated the parser band"));
    BDBG_MSG(("FOR CH NUMBER: %d.%d", getMajor(), getMinor()));
error:
    return(ret);
} /* dupParserBand */

eRet CChannelStreamer::mapInputBand(
        CInputBand *  pInputBand,
        CParserBand * pParserBand
        )
{
    eRet                     ret    = eRet_Ok;
    NEXUS_Error              nerror = NEXUS_SUCCESS;
    NEXUS_ParserBandSettings settings;

    NEXUS_ParserBand band;

    BDBG_ASSERT(NULL != _pParserBand);
    BDBG_ASSERT(NULL != pInputBand);

    if (pParserBand == NULL)
    {
        band = _pParserBand->getBand();
    }
    else
    {
        band = pParserBand->getBand();
    }

    NEXUS_ParserBand_GetSettings(band, &settings);
    settings.sourceType                   = NEXUS_ParserBandSourceType_eInputBand;
    settings.sourceTypeSettings.inputBand = pInputBand->getBand();
    BDBG_ASSERT((NEXUS_TransportType_eTs == _transportType) ||
            (NEXUS_TransportType_eDssEs == _transportType) ||
            (NEXUS_TransportType_eDssPes == _transportType));
    settings.transportType = _transportType;
    nerror                 = NEXUS_ParserBand_SetSettings(band, &settings);
    CHECK_NEXUS_ERROR_GOTO("error setting parser band settings", ret, nerror, error);

error:
    return(ret);
} /* mapInputBand */

#if 0
eRet CChannelStreamer::dupParserBand(CParserBand * pParserBand)
{
    eRet ret = eRet_Ok;

    BDBG_ASSERT(NULL != pParserBand);
    if (_tuned == false)
    {
        BDBG_ERR((" Channel Is not Tuned, Cannot duplicate ParserBand"));
        ret = eRet_ExternalError;
        goto error;
    }

    ret = mapInputBand(pInputBand, CParserBand);
    CHECK_ERROR_GOTO("error mapping input band to parser band", ret, error);

    BDBG_MSG(("Successfully duplicated the parser band"));
    BDBG_MSG(("FOR CH NUMBER: %d.%d", getMajor(), getMinor()));
error:
    return(ret);
} /* dupParserBand */

#endif /* if 0 */
eRet CChannelStreamer::tune(
        void *    id,
        CConfig * pConfig,
        bool      bWaitForLock,
        uint16_t  index
        )
{
    eRet              ret             = eRet_Ok;
    NEXUS_Error       nerror          = NEXUS_SUCCESS;
    NEXUS_InputBand   inputBandIndex  = 0;
    CInputBand *      pInputBand      = NULL;
    CParserBand *     pParserBand     = NULL;
    CBoardResources * pBoardResources = NULL;

    BSTD_UNUSED(bWaitForLock);
    BDBG_ASSERT(NULL != pConfig);
    pBoardResources = pConfig->getBoardResources();

    nerror = NEXUS_Platform_GetStreamerInputBand((ANY_INDEX == index) ? 0 : index, &inputBandIndex);
    CHECK_NEXUS_ERROR_GOTO("unable to get streamer input band", ret, nerror, errorInputBand);

    pInputBand = (CInputBand *)pBoardResources->checkoutResource(id, eBoardResource_inputBand, inputBandIndex);
    CHECK_PTR_ERROR_GOTO("unable to checkout inputband", pInputBand, ret, eRet_NotAvailable, errorInputBand);
    setInputBand(pInputBand);

    pParserBand = (CParserBand *)pBoardResources->checkoutResource(id, eBoardResource_parserBand);
    CHECK_PTR_ERROR_GOTO("unable to checkout parser band", pParserBand, ret, eRet_NotAvailable, errorParserBand);
    pParserBand->open();
    setParserBand(pParserBand);

    ret = mapInputBand(pInputBand);
    CHECK_ERROR_GOTO("error mapping input band to parser band", ret, errorMapping);

    BDBG_MSG(("CH NUMBER: %d.%d", getMajor(), getMinor()));
    _tuned = true;

    goto done;

errorMapping:
    pBoardResources->checkinResource(pParserBand);

errorParserBand:
    pBoardResources->checkinResource(pInputBand);

errorInputBand:
done:
    notifyObservers(eNotify_NonTunerLockStatus, this);
    return(ret);
} /* tune */

eRet CChannelStreamer::unTune(
        CConfig * pConfig,
        bool      bFullUnTune,
        bool      bCheckInTuner
        )
{
    eRet              ret             = eRet_Ok;
    CBoardResources * pBoardResources = NULL;

    BSTD_UNUSED(bFullUnTune);
    BSTD_UNUSED(bCheckInTuner);
    BDBG_ASSERT(NULL != pConfig);

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

    _tuned = false;

    return(ret);
} /* unTune */