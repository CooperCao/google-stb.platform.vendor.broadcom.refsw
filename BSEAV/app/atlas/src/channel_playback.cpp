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

#include "channel_playback.h"
#include "channelmgr.h"
#include "nexus_core_utils.h"

BDBG_MODULE(atlas_channel_playback);

CChannelPlayback::CChannelPlayback(CConfiguration * pCfg) :
    CChannel("CChannelPlayback", eBoardResource_max, pCfg),
    _pPlayback(NULL),
    _nFrequency(0),
    _nProgram(-1)
{
    setTunerRequired(false);
}

CChannelPlayback::CChannelPlayback(const CChannelPlayback & chPlayback) :
    CChannel(chPlayback),
    _pPlayback(NULL),
    _nFrequency(chPlayback._nFrequency),
    _strType(chPlayback._strType),
    _strFilename(chPlayback._strFilename),
    _nProgram(chPlayback._nProgram)
{
    /* copy constructor will not copy any unique objects like playback object or pids */
}

CChannel * CChannelPlayback::createCopy(CChannel * pChannel)
{
    CChannelPlayback * pChNew = NULL;

    pChNew = new CChannelPlayback(*(CChannelPlayback *)pChannel);
    return(pChNew);
}

eRet CChannelPlayback::readXML(MXmlElement * xmlElemChannel)
{
    eRet    ret = eRet_Ok;
    MString strFreq;
    MString strType;
    MString strProgramNum;

    CChannel::readXML(xmlElemChannel);

    /* setTransportType(NEXUS_TransportType_eTs); */

    strType  = xmlElemChannel->attrValue(XML_ATT_TYPE);
    _strType = (strType.isEmpty()) ? "PB" : strType;

    _strFilename = xmlElemChannel->attrValue(XML_ATT_FILENAME);
    if (_strFilename.isEmpty())
    {
        CHECK_PTR_ERROR_GOTO("playback channel must have a filename", NULL, ret, eRet_InvalidParameter, error);
    }

#if HAS_VID_NL_LUMA_RANGE_ADJ
    /* possibly force dynamic range based on filename */
    if (-1 < _strFilename.find("SDR"))
    {
        setDynamicRange(eDynamicRange_SDR);
    }
    else
    if (-1 < _strFilename.find("HDR"))
    {
        setDynamicRange(eDynamicRange_HDR10);
    }
    else
    if (-1 < _strFilename.find("HLG"))
    {
        setDynamicRange(eDynamicRange_HLG);
    }
    else
    if (-1 < _strFilename.find("DOLBYVISION"))
    {
        setDynamicRange(eDynamicRange_DolbyVision);
    }
#endif /* if HAS_VID_NL_LUMA_RANGE_ADJ */

    strProgramNum = xmlElemChannel->attrValue(XML_ATT_NUMBER);
    if (false == strProgramNum.isEmpty())
    {
        setProgramNum(strProgramNum.toInt());
        BDBG_MSG(("Program number set to %d", getProgramNum()));
    }

    strFreq = xmlElemChannel->attrValue(XML_ATT_FREQ);
    if (strFreq.isEmpty())
    {
        BDBG_MSG(("Freq is not set, default to 0"));
        _nFrequency = 0;
    }
    else
    {
        _nFrequency = strFreq.toInt();
        BDBG_MSG(("Freq is successfully set to %d", getFrequency()));
    }

    updateDescription();

error:
    return(ret);
} /* readXML */

void CChannelPlayback::writeXML(MXmlElement * xmlElemChannel)
{
    BDBG_MSG((" Write Playback channel"));
    CChannel::writeXML(xmlElemChannel);

    xmlElemChannel->addAttr(XML_ATT_TYPE, _strType);
    xmlElemChannel->addAttr(XML_ATT_FILENAME, _strFilename);
} /* writeXML */

CChannelPlayback::~CChannelPlayback()
{
}

eRet CChannelPlayback::initialize(PROGRAM_INFO_T * pProgramInfo)
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

void CChannelPlayback::setFrequency(uint32_t frequency)
{
    _nFrequency = frequency;
    updateDescription();
}

void CChannelPlayback::setStc(CStc * pStc)
{
    if (NULL != _pPlayback)
    {
        _pPlayback->setStc(pStc);
    }
    _pStc = pStc;
}

CPid * CChannelPlayback::getPid(
        unsigned index,
        ePidType type
        )
{
    if (NULL == _pPlayback)
    {
        return(NULL);
    }

    return(_pPlayback->getPid(index, type));
}

CPid * CChannelPlayback::findPid(
        unsigned pidNum,
        ePidType type
        )
{
    if (NULL == _pPlayback)
    {
        return(NULL);
    }

    return(_pPlayback->findPid(pidNum, type));
}

void CChannelPlayback::updateDescription()
{
    CChannel::updateDescription();

    _strDescription = "";

    if (0 < _nFrequency)
    {
        _strDescription  = MString(_nFrequency / 1000000);
        _strDescription += "MHz";
        addMetadata("Description", _strDescription);
    }

    addMetadata("Filename", _strFilename);
    addMetadata("Playback Type", _strType);
}

eRet CChannelPlayback::openPids(
        CSimpleAudioDecode * pAudioDecode,
        CSimpleVideoDecode * pVideoDecode
        )
{
    CPid * pVideoPid = NULL;
    CPid * pAudioPid = NULL;
    CPid * pPcrPid   = NULL;
    eRet   ret       = eRet_Ok;

    if (NULL == _pPlayback)
    {
        return(eRet_NotAvailable);
    }

    pVideoPid = _pPlayback->getPid(0, ePidType_Video);
    pAudioPid = _pPlayback->getPid(0, ePidType_Audio);
    pPcrPid   = _pPlayback->getPid(0, ePidType_Pcr);

    if ((NULL != pVideoPid) && (false == pVideoPid->isOpen()))
    {
        CHECK_PTR_ERROR_GOTO("missing playback - playback failed", _pPlayback, ret, eRet_InvalidState, error);
        pVideoPid->setVideoDecoder(pVideoDecode);
        ret = pVideoPid->open(_pPlayback);
        CHECK_ERROR_GOTO("open playback video pid channel failed", ret, error);
    }

    if ((NULL != pAudioPid) && (false == pAudioPid->isOpen()))
    {
        CHECK_PTR_ERROR_GOTO("missing playback - playback failed", _pPlayback, ret, eRet_InvalidState, error);
        pAudioPid->setAudioDecoder(pAudioDecode);
        ret = pAudioPid->open(_pPlayback);
        CHECK_ERROR_GOTO("open playback video pid channel failed", ret, error);
    }

    if ((NULL != pPcrPid) && (false == pPcrPid->isOpen()))
    {
        /* only open pcr pid channel if it is different from audio/video pids */
        if (false == pPcrPid->isOpen())
        {
            CHECK_PTR_ERROR_GOTO("missing playback - playback failed", _pPlayback, ret, eRet_InvalidState, error);
            ret = pPcrPid->open(_pPlayback);
            CHECK_ERROR_GOTO("open playback video pid channel failed", ret, error);
        }
    }

error:
    return(ret);
} /* openPids */

eRet CChannelPlayback::closePids()
{
    eRet ret = eRet_Ok;

    if (NULL != _pPlayback)
    {
        _pidMgr.closePlaybackPids(_pPlayback);
    }

    return(ret);
}

eRet CChannelPlayback::start(
        CSimpleAudioDecode * pAudioDecode,
        CSimpleVideoDecode * pVideoDecode
        )
{
    eRet ret = eRet_Ok;

    BSTD_UNUSED(pAudioDecode);
    BSTD_UNUSED(pVideoDecode);

    if (NULL == _pPlayback)
    {
        return(eRet_InvalidState);
    }

    ret = _pPlayback->start();

    CChannel::start(pAudioDecode, pVideoDecode);
    return(ret);
} /* start */

eRet CChannelPlayback::tune(
        void *    id,
        CConfig * pConfig,
        bool      bWaitForLock,
        unsigned  index
        )
{
    eRet              ret             = eRet_Ok;
    CBoardResources * pBoardResources = NULL;
    CConfiguration *  pCfg            = NULL;
    CModel *          pModel          = getModel();

    BSTD_UNUSED(bWaitForLock);
    BSTD_UNUSED(index);
    BDBG_ASSERT(NULL != pConfig);
    BDBG_ASSERT(NULL != pModel);

    pBoardResources = pConfig->getBoardResources();
    pCfg            = pConfig->getCfg();

    if (NULL == _pPlayback)
    {
        _pPlayback = (CPlayback *)pBoardResources->checkoutResource(id, eBoardResource_playback);
        CHECK_PTR_ERROR_GOTO("unable to checkout Playback ", _pPlayback, ret, eRet_NotAvailable, done);
        /* need to pass in the resources*/
        _pPlayback->setResources(NULL, pBoardResources);
        ret = _pPlayback->open(); /* no widgetengine given, so no end-of-stream playback handling */
        CHECK_ERROR_GOTO("Cannot open Playback", ret, error);
    }

    if (0 < GET_INT(pCfg, MAXDATARATE_PLAYBACK))
    {
        /* playback max data rate has been changed in atlas.cfg so print out value so it is obvious
         * which playback is in use as well as what the max data rate is set to */
        BDBG_WRN(("Playback #%d Max Data Rate:%d", _pPlayback->getNumber(), _pPlayback->getMaxDataRate()));
    }

    {
        CPid *          pVideoPid     = NULL;
        CPid *          pAudioPid     = NULL;
        CPid *          pPcrPid       = NULL;
        CVideo *        pVideo        = NULL;
        CPlaybackList * pPlaybackList = _pModel->getPlaybackList();
        CHECK_PTR_ERROR_GOTO("unable to get playback list", pPlaybackList, ret, eRet_NotAvailable, error);

        pVideo = pPlaybackList->find(_strFilename, getProgramNum());
        CHECK_PTR_ERROR_GOTO("unable to find video in playback list", pVideo, ret, eRet_NotAvailable, error);

        setWidth(pVideo->getWidth());
        setHeight(pVideo->getHeight());

        _pPlayback->setVideo(pVideo);

        /* copy videos pidmgr - the pids we want to use to record may
         * already be opened and ready for use. */
        _pPlayback->dupPidMgr(pVideo->getPidMgr());

        /* Populate pids */
        pVideoPid = _pPlayback->getPid(0, ePidType_Video);
        if (pVideoPid && pVideoPid->isDVREncryption())
        {
            BDBG_MSG((" Decrypting  Video PID"));
            pVideoPid->encrypt();
        }

        pAudioPid = _pPlayback->getPid(0, ePidType_Audio);
        if (pAudioPid && pAudioPid->isDVREncryption())
        {
            pAudioPid->encrypt();
        }

        pPcrPid = _pPlayback->getPid(0, ePidType_Pcr);
        if (pPcrPid == NULL)
        {
            /* This is normal for Playback */
            BDBG_MSG(("NULL PCRPID"));
        }
        else
        if (pPcrPid->isUniquePcrPid() && pPcrPid->isDVREncryption())
        {
            pPcrPid->encrypt();
        }
    }

    goto done;
error:
    if (NULL != _pPlayback)
    {
        _pPlayback->close();
        pBoardResources->checkinResource(_pPlayback);
        _pPlayback = NULL;
    }
done:
    _tuned = (eRet_Ok == ret) ? true : false;
    notifyObservers(eNotify_NonTunerLockStatus, this);
    return(ret);
} /* tune */

eRet CChannelPlayback::unTune(
        CConfig * pConfig,
        bool      bFullUnTune,
        bool      bCheckInTuner
        )
{
    eRet ret = eRet_Ok;

    BSTD_UNUSED(bFullUnTune);
    BSTD_UNUSED(bCheckInTuner);
    BDBG_ASSERT(NULL != _pModel);

    /* we must reset STC to ref count of pids when we do an untune */
    if (_pStc != NULL)
    {
        NEXUS_SimpleStcChannelSettings settings;
        _pStc->getDefaultSettings(&settings);
        _pStc->setSettings(&settings);
    }

    /*
     * _pPlayback->dump();
     * _pPlayback->printPids();
     */

    if (NULL != _pPlayback)
    {
        ret = _pPlayback->close(); /* stops then closes */
        CHECK_ERROR_GOTO("playback error!", ret, error);

        /* check in playback! */
        {
            CBoardResources * pBoardResources = pConfig->getBoardResources();

            ret = pBoardResources->checkinResource(_pPlayback);
            CHECK_ERROR_GOTO("unable to checkin Playback ", ret, error);
            _pPlayback = NULL;
        }
    }

error:
    _tuned = false;

    return(ret);
} /* unTune */

bool CChannelPlayback::operator ==(CChannel &other)
{
    CChannelPlayback * pOtherQam      = (CChannelPlayback *)&other;
    CPid *             pOtherPidVideo = pOtherQam->getPid(0, ePidType_Video);
    CPid *             pPidVideo      = getPid(0, ePidType_Video);
    CPid *             pOtherPidAudio = pOtherQam->getPid(0, ePidType_Audio);
    CPid *             pPidAudio      = getPid(0, ePidType_Audio);
    bool               bMatch         = false;

    /* check base class equivalency first */
    bMatch = CChannel::operator ==(other);
    if (true == bMatch)
    {
        bMatch &= (pOtherQam->getFrequency() == getFrequency());
        if (true == bMatch)
        {
            /* both NULL or both non-NULL */
            bMatch &= !((NULL == pOtherPidVideo) ^ (NULL == pPidVideo));

            if ((NULL != pOtherPidVideo) && (NULL != pPidVideo))
            {
                /* compare pids since they both exist */
                bMatch &= (*pOtherPidVideo == *pPidVideo);
            }

            /* both NULL or both non-NULL */
            bMatch &= !((NULL == pOtherPidAudio) ^ (NULL == pPidAudio));

            if ((NULL != pOtherPidAudio) && (NULL != pPidAudio))
            {
                /* compare pids since they both exist */
                bMatch &= (*pOtherPidAudio == *pPidAudio);
            }
        }
    }

    return(bMatch);
} /* == */