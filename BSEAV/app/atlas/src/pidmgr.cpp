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

#include "pidmgr.h"
#include "band.h"
#include "convert.h"

#include "nexus_parser_band.h"

BDBG_MODULE(atlas_pidmgr);

CPidMgr::CPidMgr() :
    CMvcModel("CPidMgr"),
    _pParserBand(NULL),
    _pPcrPid(NULL),
    _transportType(NEXUS_TransportType_eTs),
    _caPid(0),
    _pmtPid(0),
    _program(0),
    _pCfg(NULL)
{
    _videoPidList.clear();
    _audioPidList.clear();
    _ancillaryPidList.clear();
}

CPidMgr::CPidMgr(const CPidMgr & src) :
    CMvcModel("CPidMgr"),
    _pParserBand(src._pParserBand),
    _pPcrPid(NULL),
    _transportType(src._transportType),
    _caPid(src._caPid),
    _pmtPid(src._pmtPid),
    _program(0),
    _pCfg(src._pCfg)
{
    /* copy pids */
    MListItr <CPid> itrVideo(&(src._videoPidList));
    MListItr <CPid> itrAudio(&(src._audioPidList));
    MListItr <CPid> itrAncillary(&(src._ancillaryPidList));
    CPid *          pPid = NULL;

    for (pPid = itrVideo.first(); pPid; pPid = itrVideo.next())
    {
        eRet   ret     = eRet_Ok;
        CPid * pNewPid = NULL;

        pNewPid = new CPid(*pPid);
        BDBG_ASSERT(NULL != pNewPid);

        ret = addPid(pNewPid);
        BDBG_ASSERT(eRet_Ok == ret);

        if (pPid == src._pPcrPid)
        {
            /* pcr pid is video pid */
            _pPcrPid = pNewPid;
        }
    }

    for (pPid = itrAudio.first(); pPid; pPid = itrAudio.next())
    {
        eRet   ret     = eRet_Ok;
        CPid * pNewPid = NULL;

        pNewPid = new CPid(*pPid);
        BDBG_ASSERT(NULL != pNewPid);

        ret = addPid(pNewPid);
        BDBG_ASSERT(eRet_Ok == ret);

        if (pPid == src._pPcrPid)
        {
            /* pcr pid is audio pid */
            _pPcrPid = pNewPid;
        }
    }

    for (pPid = itrAncillary.first(); pPid; pPid = itrAncillary.next())
    {
        eRet   ret     = eRet_Ok;
        CPid * pNewPid = NULL;

        pNewPid = new CPid(*pPid);
        BDBG_ASSERT(NULL != pNewPid);

        ret = addPid(pNewPid);
        BDBG_ASSERT(eRet_Ok == ret);

        if (pPid == src._pPcrPid)
        {
            /* pcr pid is ancillary pid */
            _pPcrPid = pNewPid;
        }
    }

    if (NULL != src._pPcrPid)
    {
        if (src._pPcrPid->isUniquePcrPid())
        {
            /* if the pid is a unique pcr pid (i.e. not the same as a video/audio/ancillary pid)
             * then we must explicitly create it. */
            _pPcrPid = new CPid(*(src._pPcrPid));
            BDBG_ASSERT(NULL != _pPcrPid);
        }
    }
}

CPidMgr::~CPidMgr()
{
    if (NULL != _pPcrPid)
    {
        if (_pPcrPid->isUniquePcrPid())
        {
            /* if the pid is a unique pcr pid (i.e. not the same as a video/audio/ancillary pid)
             * then we must explicitly delete it.  otherwise it is stored in one of the other
             * pid type's pid lists. */
            DEL(_pPcrPid);
        }
    }
    _pPcrPid = NULL;

    DEL(_pParserBand);
}

eRet CPidMgr::initialize(CConfiguration * pCfg)
{
    _pCfg = pCfg;
    return(eRet_Ok);
}

eRet CPidMgr::addPid(CPid * pPid)
{
    eRet ret = eRet_NotAvailable; /* assume error */

    BDBG_ASSERT(pPid);

    if (pPid->isVideoPid())
    {
        if (ATLAS_MAX_PROGRAMS >= _videoPidList.total())
        {
            _videoPidList.add(pPid);
            ret = eRet_Ok;
        }
    }
    else
    if (pPid->isAudioPid())
    {
        if (ATLAS_MAX_PROGRAMS >= _audioPidList.total())
        {
            _audioPidList.add(pPid);
            ret = eRet_Ok;
        }
    }
    else
    if (pPid->isAncillaryPid())
    {
        if (ATLAS_MAX_PROGRAMS >= _ancillaryPidList.total())
        {
            _ancillaryPidList.add(pPid);
            ret = eRet_Ok;
        }
    }

    if (pPid->isPcrPid())
    {
        _pPcrPid = pPid;
        ret      = eRet_Ok;
    }

    return(ret);
} /* addPid */

eRet CPidMgr::removePid(CPid * pPid)
{
    eRet ret = eRet_NotAvailable; /* assume error */

    BDBG_ASSERT(pPid);

    if (pPid->isVideoPid())
    {
        if (ATLAS_MAX_PROGRAMS >= _videoPidList.total())
        {
            _videoPidList.remove(pPid);
            ret = eRet_Ok;
        }
    }
    else
    if (pPid->isAudioPid())
    {
        if (ATLAS_MAX_PROGRAMS >= _audioPidList.total())
        {
            _audioPidList.remove(pPid);
            ret = eRet_Ok;
        }
    }
    else
    if (pPid->isAncillaryPid())
    {
        if (ATLAS_MAX_PROGRAMS >= _ancillaryPidList.total())
        {
            _ancillaryPidList.remove(pPid);
            ret = eRet_Ok;
        }
    }

    if (pPid->isPcrPid())
    {
        _pPcrPid = pPid;
        ret      = eRet_Ok;
    }

    return(ret);
} /* removePid */

CPid * CPidMgr::findPid(
        uint16_t pidNum,
        ePidType type
        )
{
    CPid * pPidFound = NULL;

    switch (type)
    {
    case ePidType_Video:
    {
        CPid *          pPid = NULL;
        MListItr <CPid> itr(&_videoPidList);

        for (pPid = itr.first(); NULL != pPid; pPid = itr.next())
        {
            if (pidNum == pPid->getPid())
            {
                /* found match */
                pPidFound = pPid;
                break;
            }
        }
    }
    break;

    case ePidType_Audio:
    {
        CPid *          pPid = NULL;
        MListItr <CPid> itr(&_audioPidList);

        for (pPid = itr.first(); NULL != pPid; pPid = itr.next())
        {
            if (pidNum == pPid->getPid())
            {
                /* found match */
                pPidFound = pPid;
                break;
            }
        }
    }
    break;

    case ePidType_Ancillary:
    {
        CPid *          pPid = NULL;
        MListItr <CPid> itr(&_ancillaryPidList);

        for (pPid = itr.first(); NULL != pPid; pPid = itr.next())
        {
            if (pidNum == pPid->getPid())
            {
                /* found match */
                pPidFound = pPid;
                break;
            }
        }
    }
    break;

    case ePidType_Pcr:
        if (pidNum == _pPcrPid->getPid())
        {
            pPidFound = _pPcrPid;
        }
        break;

    default:
        break;
    } /* switch */

    return(pPidFound);
} /* findPid */

CPid * CPidMgr::getPid(
        uint16_t index,
        ePidType type
        )
{
    CPid * pPid = NULL;

    BDBG_ASSERT(NULL != _pCfg);
    /* we are going to disallow turning off BOTH audio and video decode */
    BDBG_ASSERT((true == GET_BOOL(_pCfg, VIDEODECODE_ENABLED)) || (true == GET_BOOL(_pCfg, AUDIODECODE_ENABLED)));

    switch (type)
    {
    case ePidType_Video:
        if (true == GET_BOOL(_pCfg, VIDEODECODE_ENABLED))
        {
            if (index < _videoPidList.total())
            {
                pPid = _videoPidList.get(index);
            }
        }
        break;

    case ePidType_Audio:
        if (true == GET_BOOL(_pCfg, AUDIODECODE_ENABLED))
        {
            if (index < _audioPidList.total())
            {
                pPid = _audioPidList.get(index);
            }
        }
        break;

    case ePidType_Ancillary:
        if (index < _ancillaryPidList.total())
        {
            pPid = _ancillaryPidList.get(index);
        }
        break;

    case ePidType_Pcr:
        pPid = _pPcrPid;
        break;

    default:
        break;
    } /* switch */

    return(pPid);
} /* getPid */

/* Live PIDS */
eRet CPidMgr::createPids(PROGRAM_INFO_T * pProgramInfo)
{
    eRet ret        = eRet_Ok;
    int  j          = 0;
    bool bUniquePcr = true;

    setPmtPid(pProgramInfo->map_pid);

    /* save video pids */
    for (j = 0; j < pProgramInfo->num_video_pids; j++)
    {
        CPid * pPid = NULL;
        pPid = new CPid(pProgramInfo->video_pids[j].pid,
                (NEXUS_VideoCodec)pProgramInfo->video_pids[j].streamType);
        CHECK_PTR_ERROR_GOTO("unable to create pid", pPid, ret, eRet_OutOfMemory, error);

        if (pProgramInfo->pcr_pid == pPid->getPid())
        {
            pPid->setPcrType(true);
            bUniquePcr = false;
        }

        pPid->setCaPid(pProgramInfo->video_pids[j].ca_pid);
        addPid(pPid);
    }

    /* save audio pids */
    for (j = 0; j < pProgramInfo->num_audio_pids; j++)
    {
        CPid * pPid = NULL;
        pPid = new CPid(pProgramInfo->audio_pids[j].pid,
                (NEXUS_AudioCodec)pProgramInfo->audio_pids[j].streamType);
        CHECK_PTR_ERROR_GOTO("unable to create pid", pPid, ret, eRet_OutOfMemory, error);

        pPid->setCaPid(pProgramInfo->audio_pids[j].ca_pid);
        addPid(pPid);
    }

    /* save ancillary pids */
    for (j = 0; j < pProgramInfo->num_other_pids; j++)
    {
        CPid * pPid = NULL;
        pPid = new CPid(pProgramInfo->other_pids[j].pid,
                ePidType_Ancillary);
        CHECK_PTR_ERROR_GOTO("unable to create pid", pPid, ret, eRet_OutOfMemory, error);

        pPid->setCaPid(pProgramInfo->other_pids[j].ca_pid);
        addPid(pPid);
    }

    /* save pcr pid if not duplicate */
    if (true == bUniquePcr)
    {
        CPid * pPid = NULL;
        pPid = new CPid(pProgramInfo->pcr_pid,
                ePidType_Pcr);
        CHECK_PTR_ERROR_GOTO("unable to create pid", pPid, ret, eRet_OutOfMemory, error);

        pPid->setPcrType(true);
        addPid(pPid);
    }

    /* save program ca pid if valid */
    setCaPid(pProgramInfo->ca_pid);

error:
    return(ret);
} /* createPids */

void CPidMgr::clearPids()
{
    /* must clear first in case it points to a video/audio pid */
    clearPcrPid();

    _videoPidList.clear();
    _audioPidList.clear();
    _ancillaryPidList.clear();

    _caPid = 0;
    setPmtPid(0);
}

void CPidMgr::clearPcrPid()
{
    if (NULL != _pPcrPid)
    {
        if (_pPcrPid->isUniquePcrPid())
        {
            /* if the pid is a unique pcr pid (i.e. not the same as a video/audio/ancillary pid)
             * then we must explicitly delete it.  otherwise it is stored in one of the other
             * pid type's pid lists. */
            DEL(_pPcrPid);
        }
    }

    _pPcrPid = NULL;
}

void CPidMgr::readXML(MXmlElement * xmlElem)
{
    MXmlElement * xmlElemPid = NULL;
    CPid *        pPid       = NULL;

    MListItr <CPid> itrVideo(&_videoPidList);
    MListItr <CPid> itrAudio(&_audioPidList);

    if (xmlElem->tag() != XML_TAG_STREAM)
    {
        return;
    }

    setPmtPid(xmlElem->attrInt(XML_ATT_PMT));

    {
        MString strTransportType = xmlElem->attrValue(XML_ATT_TYPE);
        if (strTransportType.isEmpty())
        {
            BDBG_MSG(("No TransportType Preset, default to TS"));
            _transportType = NEXUS_TransportType_eTs;
        }
        else
        {
            _transportType = stringToTransportType(strTransportType);
            BDBG_MSG(("Transport Type %s", strTransportType.s()));
        }
    }

    _program = xmlElem->attrInt(XML_ATT_NUMBER);

    /* look for pids */
    for (xmlElemPid = xmlElem->firstChild(); xmlElemPid; xmlElemPid = xmlElem->nextChild())
    {
        if (xmlElemPid->tag() != XML_TAG_PID)
        {
            BDBG_WRN((" Tag is not PID its %s", xmlElem->tag().s()));
            continue;
        }

        /* Function will parse the XML element  */
        pPid = new CPid(xmlElemPid);
        if (pPid->getPidType() == ePidType_Unknown)
        {
            BDBG_WRN((" Unknown PID entry "));
            DEL(pPid);
            continue;
        }

        addPid(pPid);
        BDBG_MSG((" Added Pid %s ", xmlElem->tag().s()));
    }

    /* Check for duplicate PCR pids. Can be Audio or Video */
    for (pPid = itrVideo.first(); pPid; pPid = itrVideo.next())
    {
        if ((_pPcrPid != NULL) && (pPid->getPid() == _pPcrPid->getPid()))
        {
            BDBG_MSG((" Pids are the same delete PCR pid"));
            clearPcrPid();
            break;
        }
    }

    for (pPid = itrAudio.first(); pPid; pPid = itrAudio.next())
    {
        if ((_pPcrPid != NULL) && (pPid->getPid() == _pPcrPid->getPid()))
        {
            BDBG_MSG((" Pids are the same delete PCR pid"));
            clearPcrPid();
            break;
        }
    }

    if (_pPcrPid != NULL)
    {
        BDBG_MSG((" PCR PID is Valid return "));
        return;
    }

    BDBG_MSG((" NO PCR pid set it to first video or audio pid"));
    /* Cannot find a pcr pid */
    pPid = itrVideo.first();
    if (NULL != pPid)
    {
        pPid->setPcrType(true);
        _pPcrPid = pPid;
    }
    else
    {
        pPid = itrAudio.first();
        if (NULL != pPid)
        {
            pPid->setPcrType(true);
            _pPcrPid = pPid;
        }
    }
} /* readXML */

MXmlElement * CPidMgr::writeXML(MXmlElement * xmlElem)
{
    CPid * pPid       = NULL;
    bool   bUniquePcr = true;

    MListItr <CPid> itrVideo(&_videoPidList);
    MListItr <CPid> itrAudio(&_audioPidList);
    MListItr <CPid> itrAncillary(&_ancillaryPidList);
    MXmlElement *   xmlElemTransport = new MXmlElement(xmlElem, XML_TAG_STREAM);

    /* Iterate through all Pid list*/
    for (pPid = itrVideo.first(); pPid; pPid = itrVideo.next())
    {
        pPid->writeXML(xmlElemTransport);
        if (_pPcrPid != NULL)
        {
            if (_pPcrPid == pPid)
            {
                bUniquePcr = false;
            }
        }
    }

    for (pPid = itrAudio.first(); pPid; pPid = itrAudio.next())
    {
        pPid->writeXML(xmlElemTransport);
    }

    for (pPid = itrAncillary.first(); pPid; pPid = itrAncillary.next())
    {
        pPid->writeXML(xmlElemTransport);
    }

    if ((_pPcrPid != NULL) && (bUniquePcr == true))
    {
        _pPcrPid->writeXML(xmlElemTransport);
    }

    /* Add Transport type to Channel Entry */
    xmlElemTransport->addAttr(XML_ATT_NUMBER, MString(_program).s());
    xmlElemTransport->addAttr(XML_ATT_TYPE, transportTypeToString(_transportType));
    xmlElemTransport->addAttr(XML_ATT_PMT, MString(getPmtPid()));

    BDBG_MSG(("Done write Pids"));

    return(xmlElemTransport);
} /* writeXML */

bool CPidMgr::isEncrypted(void)
{
    if (true == VALID_PID(getCaPid()))
    {
        return(true);
    }

    /* also have to check ca pid values for each video pid object */
    MListItr <CPid> itrVideo(&_videoPidList);
    CPid *          pPid = NULL;

    for (pPid = itrVideo.first(); pPid; pPid = itrVideo.next())
    {
        if (pPid->isEncrypted())
        {
            return(true);
        }
    }

    /* also have to check ca pid values for each audio pid object */
    MListItr <CPid> itrAudio(&_audioPidList);
    for (pPid = itrAudio.first(); pPid; pPid = itrAudio.next())
    {
        if (pPid->isEncrypted())
        {
            return(true);
        }
    }

    return(false);
} /* isEncrypted */

eRet CPidMgr::mapInputBand(CInputBand * pInputBand)
{
    eRet                     ret    = eRet_Ok;
    NEXUS_Error              nerror = NEXUS_SUCCESS;
    NEXUS_ParserBandSettings settings;
    NEXUS_ParserBand         band;

    BDBG_ASSERT(NULL != _pParserBand);
    BDBG_ASSERT(NULL != pInputBand);

    band = _pParserBand->getBand();

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

bool CPidMgr::operator ==(CPidMgr &other)
{
    CPid * pVideoPid      = getPid(0, ePidType_Video);
    CPid * pOtherVideoPid = other.getPid(0, ePidType_Video);

    CPid * pAudioPid      = getPid(0, ePidType_Audio);
    CPid * pOtherAudioPid = other.getPid(0, ePidType_Audio);

    if (_transportType != other._transportType)
    {
        return(false);
    }

    if ((NULL != _pPcrPid) && (NULL != other._pPcrPid))
    {
        if (!(*_pPcrPid == *(other._pPcrPid)))
        {
            return(false);
        }
    }
    else
    if ((NULL == _pPcrPid) ^ (NULL == other._pPcrPid))
    {
        /* XOR */
        return(false);
    }

    /* note: we will only check the first video and audio pid - technically we should
     * check all the pids but this is sufficient in all but the rarest of cases. */
    if ((NULL != pVideoPid) && (NULL != pOtherVideoPid))
    {
        if (!(*pVideoPid == *pOtherVideoPid))
        {
            return(false);
        }
    }
    else
    if ((NULL == pVideoPid) ^ (NULL == pOtherVideoPid))
    {
        /* XOR */
        return(false);
    }

    if ((NULL != pAudioPid) && (NULL != pOtherAudioPid))
    {
        if (!(*pAudioPid == *pOtherAudioPid))
        {
            return(false);
        }
    }
    else
    if ((NULL == pAudioPid) ^ (NULL == pOtherAudioPid))
    {
        /* XOR */
        return(false);
    }

    if (other.getCaPid() != getCaPid())
    {
        return(false);
    }

    return(true);
} /* == */

void CPidMgr::operator =(CPidMgr &other)
{
    MListItr <CPid> itrVideo(&(other._videoPidList));
    MListItr <CPid> itrAudio(&(other._audioPidList));
    MListItr <CPid> itrAncillary(&(other._ancillaryPidList));
    CPid *          pPid = NULL;

    _transportType = other._transportType;
    _program       = other._program;

    clearPids();

    for (pPid = itrVideo.first(); pPid; pPid = itrVideo.next())
    {
        eRet   ret     = eRet_Ok;
        CPid * pNewPid = NULL;

        pNewPid = new CPid(*pPid);
        BDBG_ASSERT(NULL != pNewPid);

        ret = addPid(pNewPid);
        BDBG_ASSERT(eRet_Ok == ret);

        if (pPid == other._pPcrPid)
        {
            /* pcr pid is video pid */
            _pPcrPid = pNewPid;
        }
    }

    for (pPid = itrAudio.first(); pPid; pPid = itrAudio.next())
    {
        eRet   ret     = eRet_Ok;
        CPid * pNewPid = NULL;

        pNewPid = new CPid(*pPid);
        BDBG_ASSERT(NULL != pNewPid);

        ret = addPid(pNewPid);
        BDBG_ASSERT(eRet_Ok == ret);

        if (pPid == other._pPcrPid)
        {
            /* pcr pid is audio pid */
            _pPcrPid = pNewPid;
        }
    }

    for (pPid = itrAncillary.first(); pPid; pPid = itrAncillary.next())
    {
        eRet   ret     = eRet_Ok;
        CPid * pNewPid = NULL;

        pNewPid = new CPid(*pPid);
        BDBG_ASSERT(NULL != pNewPid);

        ret = addPid(pNewPid);
        BDBG_ASSERT(eRet_Ok == ret);

        if (pPid == other._pPcrPid)
        {
            /* pcr pid is ancillary pid */
            _pPcrPid = pNewPid;
        }
    }

    if (NULL != other._pPcrPid)
    {
        if (other._pPcrPid->isUniquePcrPid())
        {
            /* if the pid is a unique pcr pid (i.e. not the same as a video/audio/ancillary pid)
             * then we must explicitly create it. */
            _pPcrPid = new CPid(*(other._pPcrPid));
            BDBG_ASSERT(NULL != _pPcrPid);
        }
    }
} /* = */

/* This Function does delete the playback pids */
void CPidMgr::closePlaybackPids(CPlayback * pPlayback)
{
    MListItr <CPid> itrVideo(&_videoPidList);
    CPid *          pPid = NULL;

    for (pPid = itrVideo.first(); pPid; pPid = itrVideo.next())
    {
        pPid->close(pPlayback);
        BDBG_MSG(("Closing VIDEO PID"));
        /* pPid->print();*/
    }

    _videoPidList.clear();

    /* also have to check ca pid values for each audio pid object */
    MListItr <CPid> itrAudio(&_audioPidList);
    for (pPid = itrAudio.first(); pPid; pPid = itrAudio.next())
    {
        pPid->close(pPlayback);
        BDBG_MSG(("Closing AUDIO PID"));
        /* pPid->print(); */
    }
    _audioPidList.clear();

    /* also have to check ca pid values for each audio pid object */
    MListItr <CPid> itrAncillary(&_ancillaryPidList);
    for (pPid = itrAncillary.first(); pPid; pPid = itrAncillary.next())
    {
        pPid->close(pPlayback);
        BDBG_MSG(("Closing AUDIO PID"));
        /* pPid->print(); */
    }

    _ancillaryPidList.clear();

    if (_pPcrPid && _pPcrPid->isUniquePcrPid())
    {
        _pPcrPid->close(pPlayback);
        BDBG_MSG(("Closing PCR pid"));
    }

    _caPid = 0;
    setPmtPid(0);

    _pPcrPid = NULL;
} /* closePlaybackPids */

/* This Function closes Playback pid Channel */
void CPidMgr::closePlaybackPidChannels(CPlayback * pPlayback)
{
    MListItr <CPid> itrVideo(&_videoPidList);
    CPid *          pPid = NULL;

    for (pPid = itrVideo.first(); pPid; pPid = itrVideo.next())
    {
        pPid->close(pPlayback);
        BDBG_MSG(("Closing VIDEO PID CHANNEL "));
        /* pPid->print();*/
    }

    /* also have to check ca pid values for each audio pid object */
    MListItr <CPid> itrAudio(&_audioPidList);
    for (pPid = itrAudio.first(); pPid; pPid = itrAudio.next())
    {
        pPid->close(pPlayback);
        BDBG_MSG(("Closing AUDIO PID CHANNEL"));
        /* pPid->print(); */
    }

    /* also have to check ca pid values for each audio pid object */
    MListItr <CPid> itrAncillary(&_ancillaryPidList);
    for (pPid = itrAncillary.first(); pPid; pPid = itrAncillary.next())
    {
        pPid->close(pPlayback);
        BDBG_MSG(("Closing AUDIO PID CHANNEL"));
        /* pPid->print(); */
    }

    if (_pPcrPid && _pPcrPid->isUniquePcrPid())
    {
        _pPcrPid->close(pPlayback);
        BDBG_MSG(("Closing PCR pid"));
    }
} /* closePlaybackPidChannels */

/* This Function closes  pid Channels */
void CPidMgr::closePidChannels()
{
    MListItr <CPid> itrVideo(&_videoPidList);
    CPid *          pPid = NULL;

    for (pPid = itrVideo.first(); pPid; pPid = itrVideo.next())
    {
        pPid->close();
        BDBG_MSG(("Closing VIDEO PID CHANNEL "));
        /* pPid->print();*/
    }

    /* also have to check ca pid values for each audio pid object */
    MListItr <CPid> itrAudio(&_audioPidList);
    for (pPid = itrAudio.first(); pPid; pPid = itrAudio.next())
    {
        pPid->close();
        BDBG_MSG(("Playpump:Closing AUDIO PID CHANNEL"));
        /* pPid->print(); */
    }

    /* also have to check ca pid values for each audio pid object */
    MListItr <CPid> itrAncillary(&_ancillaryPidList);
    for (pPid = itrAncillary.first(); pPid; pPid = itrAncillary.next())
    {
        pPid->close();
        BDBG_MSG(("Closing Ancillary CHANNEL"));
        /* pPid->print(); */
    }

    if (_pPcrPid && _pPcrPid->isUniquePcrPid())
    {
        _pPcrPid->close();
        BDBG_MSG(("Closing PCR pid"));
    }
} /* closePidChannels */

/* This Function closes Playback pid Channel */
void CPidMgr::closePidChannels(CPlaypump * pPlaypump)
{
    MListItr <CPid> itrVideo(&_videoPidList);
    CPid *          pPid = NULL;

    for (pPid = itrVideo.first(); pPid; pPid = itrVideo.next())
    {
        pPid->close(pPlaypump);
        BDBG_MSG(("Playpump:Closing VIDEO PID CHANNEL "));
        /* pPid->print();*/
    }

    /* also have to check ca pid values for each audio pid object */
    MListItr <CPid> itrAudio(&_audioPidList);
    for (pPid = itrAudio.first(); pPid; pPid = itrAudio.next())
    {
        pPid->close(pPlaypump);
        BDBG_MSG(("Playpump:Closing AUDIO PID CHANNEL"));
        /* pPid->print(); */
    }

    /* also have to check ca pid values for each audio pid object */
    MListItr <CPid> itrAncillary(&_ancillaryPidList);
    for (pPid = itrAncillary.first(); pPid; pPid = itrAncillary.next())
    {
        pPid->close(pPlaypump);
        BDBG_MSG(("Playpump:Closing Ancillary CHANNEL"));
        /* pPid->print(); */
    }

    if (_pPcrPid && _pPcrPid->isUniquePcrPid())
    {
        _pPcrPid->close(pPlaypump);
        BDBG_MSG(("Closing PCR pid"));
    }
} /* closePlaybackPidChannels */

/* This Function does delete the record pids */
void CPidMgr::closeRecordPids(CRecord * pRecord)
{
    MListItr <CPid> itrVideo(&_videoPidList);
    CPid *          pPid = NULL;

    for (pPid = itrVideo.first(); pPid; pPid = itrVideo.next())
    {
        pPid->close(pRecord);
        BDBG_MSG(("Closing VIDEO PID"));
        /* pPid->print();*/
    }

    _videoPidList.clear();

    /* also have to check ca pid values for each audio pid object */
    MListItr <CPid> itrAudio(&_audioPidList);
    for (pPid = itrAudio.first(); pPid; pPid = itrAudio.next())
    {
        pPid->close(pRecord);
        BDBG_MSG(("Closing AUDIO PID"));
        /* pPid->print(); */
    }
    _audioPidList.clear();

    /* also have to check ca pid values for each audio pid object */
    MListItr <CPid> itrAncillary(&_ancillaryPidList);
    for (pPid = itrAncillary.first(); pPid; pPid = itrAncillary.next())
    {
        pPid->close(pRecord);
        BDBG_MSG(("Closing AUDIO PID"));
        /* pPid->print(); */
    }

    _ancillaryPidList.clear();

    _caPid = 0;
    setPmtPid(0);

    _pPcrPid = NULL;
} /* closeRecordPids */

/* This Function closes Playback pid Channel but does not delete it */
void CPidMgr::closeRecordPidChannels(CRecord * pRecord)
{
    MListItr <CPid> itrVideo(&_videoPidList);
    CPid *          pPid = NULL;

    for (pPid = itrVideo.first(); pPid; pPid = itrVideo.next())
    {
        pPid->close(pRecord);
        BDBG_MSG(("Closing VIDEO PID CHANNEL "));
        /* pPid->print();*/
    }

    /* also have to check ca pid values for each audio pid object */
    MListItr <CPid> itrAudio(&_audioPidList);
    for (pPid = itrAudio.first(); pPid; pPid = itrAudio.next())
    {
        pPid->close(pRecord);
        BDBG_MSG(("Closing AUDIO PID CHANNEL"));
        /* pPid->print(); */
    }

    /* also have to check ca pid values for each audio pid object */
    MListItr <CPid> itrAncillary(&_ancillaryPidList);
    for (pPid = itrAncillary.first(); pPid; pPid = itrAncillary.next())
    {
        pPid->close(pRecord);
        BDBG_MSG(("Closing AUDIO PID CHANNEL"));
        /* pPid->print(); */
    }
} /* closeRecordPidChannels */

void CPidMgr::print(bool bForce)
{
    MListItr <CPid> itrVideo(&_videoPidList);
    CPid *          pPid = NULL;
    BDBG_Level      level;

    if (true == bForce)
    {
        BDBG_GetModuleLevel("atlas_pidmgr", &level);
        BDBG_SetModuleLevel("atlas_pidmgr", BDBG_eMsg);
    }

    BDBG_MSG(("PMT pid:%d", getPmtPid()));

    for (pPid = itrVideo.first(); pPid; pPid = itrVideo.next())
    {
        pPid->print(bForce);
    }

    /* also have to check ca pid values for each audio pid object */
    MListItr <CPid> itrAudio(&_audioPidList);
    for (pPid = itrAudio.first(); pPid; pPid = itrAudio.next())
    {
        pPid->print(bForce);
    }

    /* also have to check ca pid values for each audio pid object */
    MListItr <CPid> itrAncillary(&_ancillaryPidList);
    for (pPid = itrAncillary.first(); pPid; pPid = itrAncillary.next())
    {
        pPid->print(bForce);
    }

    if (true == bForce)
    {
        BDBG_SetModuleLevel("atlas_pidmgr", level);
    }
} /* print */

bool CPidMgr::isEmpty(void)
{
    if (0 < _videoPidList.total())
    {
        return(false);
    }

    if (0 < _audioPidList.total())
    {
        return(false);
    }

    /* also have to check ca pid values for each audio pid object */
#if 0
    if (0 < _ancillaryPidList.total())
    {
        return(false);
    }
#endif /* if 0 */

    return(true);
} /* isEmpty */