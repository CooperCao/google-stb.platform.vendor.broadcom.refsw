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

#include "channel_mosaic.h"
#include "channelmgr.h"
#include "nexus_core_utils.h"

BDBG_MODULE(atlas_channel_mosaic);

CChannelMosaic::CChannelMosaic(CConfiguration * pCfg) :
    CChannel("CChannelMosaic", eBoardResource_max, pCfg)
{
    /* no pip swap for mosaic channels */
    setPipSwapSupported(false);
}

CChannelMosaic::CChannelMosaic(const CChannelMosaic & chMosaic) :
    CChannel(chMosaic)
{
    eRet       ret     = eRet_Ok;
    CChannel * pMosaic = NULL;

    /* copy constructor will not copy any unique objects like pids */

    /* clear list also deletes contents automatically */
    _mosaicList.clear();

error:
    return;
}

CChannel * CChannelMosaic::createCopy(CChannel * pChannel)
{
    CChannelMosaic * pChannelMosaic = (CChannelMosaic *)pChannel;
    CChannel *       pMosaic        = NULL;
    CChannelMosaic * pChNew         = NULL;
    eRet             ret            = eRet_Ok;

    pChNew = new CChannelMosaic(*(CChannelMosaic *)pChannel);
    CHECK_PTR_ERROR_GOTO("unable to allocate CChannelMosaic", pChNew, ret, eRet_OutOfMemory, error);

    /* copy list of mosaics */
    for (pMosaic = pChannelMosaic->getMosaicList()->first(); pMosaic; pMosaic = pChannelMosaic->getMosaicList()->next())
    {
        CChannel * pMosaicNew = pMosaic->createCopy(pMosaic);
        pChNew->addChannel(pMosaicNew);
    }

error:
    return(pChNew);
} /* createCopy */

eRet CChannelMosaic::registerObserver(
        CObserver *   pObserver,
        eNotification notification
        )
{
    eRet       ret     = eRet_Ok;
    CChannel * pMosaic = NULL;

    CChannel::registerObserver(pObserver, notification);

    for (pMosaic = _mosaicList.first(); pMosaic; pMosaic = _mosaicList.next())
    {
        /* register observer for mosaic subchannels */
        pMosaic->registerObserver(pObserver, notification);
    }

    return(ret);
} /* registerObserver */

extern CChannel * createChannel(CConfiguration * pCfg, MXmlElement * xmlElemChannel);

eRet CChannelMosaic::readXML(MXmlElement * xmlElemChannel)
{
    eRet    ret = eRet_Ok;
    MString strFreq;
    MString strType;
    MString strProgramNum;

    CChannel::readXML(xmlElemChannel);

    MXmlElement * xmlElemCh = NULL;

    for (xmlElemCh = xmlElemChannel->firstChild(); xmlElemCh; xmlElemCh = xmlElemChannel->nextChild())
    {
        CChannel * pCh = NULL;

        pCh = createChannel(_pCfg, xmlElemCh);

        if (NULL != pCh)
        {
            BDBG_MSG(("create mosaic channel:%s", pCh->getName().s()));
            pCh->readXML(xmlElemCh);
            addChannel(pCh);
        }
    }

    updateDescription();

error:
    return(ret);
} /* readXML */

void CChannelMosaic::writeXML(MXmlElement * xmlElemChannel)
{
    MXmlElement * xmlElemSubChannel = NULL;
    CChannel *    pMosaic           = NULL;
    eRet          ret               = eRet_Ok;

    BDBG_MSG((" Write Mosaic channel"));
    CChannel::writeXML(xmlElemChannel);

    /* write all mosaic sub channels */
    for (pMosaic = _mosaicList.first(); pMosaic; pMosaic = _mosaicList.next())
    {
        xmlElemSubChannel = new MXmlElement(xmlElemChannel, XML_TAG_CHANNEL);
        CHECK_PTR_ERROR_GOTO("unable to allocate xml element", xmlElemSubChannel, ret, eRet_OutOfMemory, error);

        pMosaic->writeXML(xmlElemSubChannel);
    }

    xmlElemChannel->addAttr(XML_ATT_MAJOR, MString(getMajor()));
error:
    return;
} /* writeXML */

CChannelMosaic::~CChannelMosaic()
{
}

eRet CChannelMosaic::initialize(PROGRAM_INFO_T * pProgramInfo)
{
    /* Do nothing with this function at this time */
    BSTD_UNUSED(pProgramInfo);
    return(eRet_Ok);
}

eRet CChannelMosaic::addChannel(CChannel * pChannel)
{
    eRet ret = eRet_Ok;

    _mosaicList.add(pChannel);
    pChannel->setParent(this);
    _numSubChannels++;

    return(ret);
}

void CChannelMosaic::updateDescription()
{
    CChannel::updateDescription();

    _strDescription = "";

    /* addMetadata("Filename", _strFilename); */
}

eRet CChannelMosaic::tune(
        void *    id,
        CConfig * pConfig,
        bool      bWaitForLock,
        uint16_t  index
        )
{
    eRet       ret        = eRet_Ok;
    CChannel * pMosaic    = NULL;
    uint32_t   windowType = eWindowType_Mosaic1;

    for (pMosaic = _mosaicList.first(); pMosaic; pMosaic = _mosaicList.next())
    {
        CChannelData channelData("");

        channelData._pChannel   = pMosaic;
        channelData._windowType = (eWindowType)windowType;

        BDBG_MSG(("tuning mosaic:%s", pMosaic->getName().s()));
        ret = notifyObservers(eNotify_Tune, &channelData);
        CHECK_ERROR_GOTO("error notifying observers", ret, error);

        /* each mosaic channel in list corresponds to a mosaic window type */
        windowType++;
    }

error:
    _tuned = (eRet_Ok == ret) ? true : false;
    return(ret);
} /* tune */

eRet CChannelMosaic::unTune(
        CConfig * pConfig,
        bool      bFullUnTune,
        bool      bCheckInTuner
        )
{
    eRet       ret     = eRet_Ok;
    CChannel * pMosaic = NULL;

    BSTD_UNUSED(bFullUnTune);
    BSTD_UNUSED(bCheckInTuner);
    BDBG_ASSERT(NULL != _pModel);

    for (pMosaic = _mosaicList.first(); pMosaic; pMosaic = _mosaicList.next())
    {
        BDBG_MSG(("UNtuning mosaic:%s", pMosaic->getName().s()));
        ret = pMosaic->unTune(pConfig, bFullUnTune, bCheckInTuner);
        CHECK_ERROR_GOTO("unable to untune mosaic", ret, error);
    }

error:
    _tuned = false;

    return(ret);
} /* unTune */

bool CChannelMosaic::operator ==(CChannel &other)
{
    CChannelMosaic * pOtherMosaic = (CChannelMosaic *)&other;
    bool             bMatch       = false;

    /* check base class equivalency first */
    bMatch = CChannel::operator ==(other);
    if (true == bMatch)
    {
        CChannel * pChannel      = NULL;
        CChannel * pChannelOther = NULL;

        for (pChannel = _mosaicList.first(), pChannelOther = pOtherMosaic->getMosaicList()->first();
             pChannel && pChannelOther;
             pChannel = _mosaicList.next(), pChannelOther = pOtherMosaic->getMosaicList()->next())
        {
            bMatch &= (*pChannel == *pChannelOther);
        }
    }

    return(bMatch);
} /* == */

eRet CChannelMosaic::closePids()
{
    eRet       ret     = eRet_Ok;
    CChannel * pMosaic = NULL;

    for (pMosaic = _mosaicList.first(); pMosaic; pMosaic = _mosaicList.next())
    {
        ret = pMosaic->closePids();
        CHECK_ERROR_GOTO("error closing mosiac channel pids", ret, error);
    }
error:
    return(ret);
}

bool CChannelMosaic::isTunerRequired()
{
    CChannel * pMosaic        = NULL;
    bool       bTunerRequired = false;

    for (pMosaic = _mosaicList.first(); pMosaic; pMosaic = _mosaicList.next())
    {
        bTunerRequired |= pMosaic->isTunerRequired();
    }

    /* since we've calculated whether a tuner is required, might as well
     * update our base class member variable */
    _bTunerRequired = bTunerRequired;

    return(bTunerRequired);
}