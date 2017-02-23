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

#include "channel.h"
#include "band.h"
#include "record.h"
#include "stc.h"

#include "nexus_parser_band.h"

BDBG_MODULE(atlas_channel);

ENUM_TO_MSTRING_INIT_C(channelTrickToString, eChannelTrick)
ENUM_TO_MSTRING_START()
ENUM_TO_MSTRING_ENTRY(eChannelTrick_Stop, "Stop")
ENUM_TO_MSTRING_ENTRY(eChannelTrick_Play, "Play")
ENUM_TO_MSTRING_ENTRY(eChannelTrick_Pause, "Pause")
ENUM_TO_MSTRING_ENTRY(eChannelTrick_Rewind, "Rewind")
ENUM_TO_MSTRING_ENTRY(eChannelTrick_FastForward, "Forward")
ENUM_TO_MSTRING_ENTRY(eChannelTrick_FrameAdvance, "FrameAdv")
ENUM_TO_MSTRING_ENTRY(eChannelTrick_FrameRewind, "FrameRew")
ENUM_TO_MSTRING_ENTRY(eChannelTrick_PlayNormal, "Play")
ENUM_TO_MSTRING_ENTRY(eChannelTrick_PlayI, "Play-I")
ENUM_TO_MSTRING_ENTRY(eChannelTrick_PlaySkipB, "Play-SkipB")
ENUM_TO_MSTRING_ENTRY(eChannelTrick_PlayIP, "Play-IP")
ENUM_TO_MSTRING_ENTRY(eChannelTrick_PlaySkipP, "Play-SkipP")
ENUM_TO_MSTRING_ENTRY(eChannelTrick_PlayBrcm, "Play-Brcm")
ENUM_TO_MSTRING_ENTRY(eChannelTrick_PlayGop, "Play-Gop")
ENUM_TO_MSTRING_ENTRY(eChannelTrick_PlayGopIP, "Play-GopIP")
ENUM_TO_MSTRING_ENTRY(eChannelTrick_TimeSkip, "TimeSkip")
ENUM_TO_MSTRING_ENTRY(eChannelTrick_Rate, "Rate")
ENUM_TO_MSTRING_ENTRY(eChannelTrick_Host, "Host")
ENUM_TO_MSTRING_ENTRY(eChannelTrick_Seek, "Seek")
ENUM_TO_MSTRING_ENTRY(eChannelTrick_Max, "Invalid")
ENUM_TO_MSTRING_END()

CChannel::CChannel(
        const char *     strName,
        eBoardResource   type,
        CConfiguration * pCfg
        ) :
    CMvcModel(strName),
    _type(type),
    _major(0),
    _minor(0),
    _strDescription("Channel"),
    _strDescriptionLong(""),
    _strDescriptionShort(""),
    _programNum(0),
#ifdef SNMP_SUPPORT
    _ccError(0),
    _teiError(0),
    _cci(0),
#endif
    _pInputBand(NULL),
    _tuned(false),
    _pParserBand(NULL),
    _pPlayback(NULL),
    _pPlaypump(NULL),
#if NEXUS_HAS_VIDEO_ENCODER
    _pEncode(NULL),
#endif
    _transportType(NEXUS_TransportType_eUnknown),
#if NEXUS_HAS_FRONTEND
    _pTuner(NULL),
#endif
    _pRecord(NULL),
    _pCfg(pCfg),
    _pStc(NULL),
    _trickModeRate(1),
    _trickModeState(eChannelTrick_Play),
    _bTunerRequired(false),
    _width(0),
    _height(0),
    _durationInMsecs(0),
    _bStopAllowed(false),
    _bPipSwapSupported(true),
    _numSubChannels(0),
    _pParent(NULL),
    _pModel(NULL),
    _pWidgetEngine(NULL)
{
    _pidMgr.initialize(_pCfg);
}

CChannel::CChannel(const CChannel & ch) :
    CMvcModel(ch._strName),
    _type(ch._type),
    _major(ch._major),
    _minor(ch._minor),
    _strDescription(ch._strDescription),
    _strDescriptionLong(ch._strDescriptionLong),
    _strDescriptionShort(ch._strDescriptionShort),
    _programNum(ch._programNum),
#ifdef SNMP_SUPPORT
    _ccError(0),
    _teiError(0),
    _cci(0),
#endif
    _pInputBand(NULL),
    _tuned(false),
    _pParserBand(NULL),
    _pPlayback(NULL),
    _pPlaypump(NULL),
#if NEXUS_HAS_VIDEO_ENCODER
    _pEncode(NULL),
#endif
    _transportType(ch._transportType),
    _pidMgr(ch._pidMgr),
#if NEXUS_HAS_FRONTEND
    _pTuner(NULL),
#endif
    _pRecord(NULL),
    _pCfg(ch._pCfg),
    _pStc(NULL),
    _trickModeRate(1),
    _trickModeState(eChannelTrick_Play),
    _bTunerRequired(ch._bTunerRequired),
    _width(ch._width),
    _height(ch._height),
    _durationInMsecs(ch._durationInMsecs),
    _bStopAllowed(ch._bStopAllowed),
    _bPipSwapSupported(ch._bPipSwapSupported),
    _numSubChannels(0),
    _pParent(NULL),
    _pModel(ch._pModel),
    _pWidgetEngine(ch._pWidgetEngine)
{
}

CChannel::~CChannel()
{
}

#ifdef SNMP_SUPPORT
static void found_ccError(
        void * ctx,
        int    param
        )
{
    CChannel * pChannel = (CChannel *) ctx;

    BSTD_UNUSED(param);
    pChannel->countCcError();
    return;
}

static void found_teiError(
        void * ctx,
        int    param
        )
{
    CChannel * pChannel = (CChannel *) ctx;

    BSTD_UNUSED(param);
    pChannel->countTeiError();
    return;
}

#endif /* ifdef SNMP_SUPPORT */

eRet CChannel::dupParserBand(CParserBand * pParserBand)
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

eRet CChannel::mapInputBand(
        CInputBand *  pInputBand,
        CParserBand * pParserBand
        )
{
    eRet                     ret    = eRet_Ok;
    NEXUS_Error              nerror = NEXUS_SUCCESS;
    NEXUS_ParserBandSettings settings;
    NEXUS_ParserBand         band;

    BDBG_ASSERT(NULL != _pParserBand);
    BDBG_ASSERT(NULL != pInputBand);

    if (pParserBand != NULL)
    {
        BDBG_MSG(("parserBand is passed in"));
        band = pParserBand->getBand();
    }
    else
    {
        band = _pParserBand->getBand();
    }
    NEXUS_ParserBand_GetSettings(band, &settings);

#if NEXUS_HAS_FRONTEND
    {
        NEXUS_FrontendUserParameters userParams;

        nerror = NEXUS_Frontend_GetUserParameters(_pTuner->getFrontend(), &userParams);
        CHECK_NEXUS_ERROR_GOTO("unable to get frontend user parameters", ret, nerror, error);

        if (true == userParams.isMtsif)
        {
            settings.sourceType               = NEXUS_ParserBandSourceType_eMtsif;
            settings.sourceTypeSettings.mtsif = _pTuner->getFrontendConnector();
        }
        else
        {
            settings.sourceType                   = NEXUS_ParserBandSourceType_eInputBand;
            settings.sourceTypeSettings.inputBand = pInputBand->getBand();
        }
    }
#else /* if NEXUS_HAS_FRONTEND */
    settings.sourceType                   = NEXUS_ParserBandSourceType_eInputBand;
    settings.sourceTypeSettings.inputBand = pInputBand->getBand();
#endif /* NEXUS_HAS_FRONTEND */

    BDBG_ASSERT((NEXUS_TransportType_eTs == _transportType) ||
            (NEXUS_TransportType_eDssEs == _transportType) ||
            (NEXUS_TransportType_eDssPes == _transportType));
    settings.transportType = _transportType;

#ifdef SNMP_SUPPORT
    settings.ccError.callback  = found_ccError;
    settings.ccError.context   = this;
    settings.ccError.param     = 0;
    settings.teiError.callback = found_teiError;
    settings.teiError.context  = this;
    settings.teiError.param    = 0;
#endif /* ifdef SNMP_SUPPORT */

    nerror = NEXUS_ParserBand_SetSettings(band, &settings);
    CHECK_NEXUS_ERROR_GOTO("error setting parser band settings", ret, nerror, error);

error:
    return(ret);
} /* mapInputBand */

void CChannel::dump(bool bForce)
{
    CPid *     pPid = NULL;
    BDBG_Level level;

    if (true == bForce)
    {
        BDBG_GetModuleLevel("atlas_channel", &level);
        BDBG_SetModuleLevel("atlas_channel", BDBG_eMsg);
    }

    BDBG_MSG(("channel - type:%s, number:%d.%d, tuned:%d", boardResourceToString(_type).s(), _major, _minor, _tuned));
    for (int i = 0; NULL != (pPid = _pidMgr.getPid(i, ePidType_Video)); i++)
    {
        BDBG_MSG(("     video pid:0x%2x", pPid->getPid()));
    }
    for (int i = 0; NULL != (pPid = _pidMgr.getPid(i, ePidType_Audio)); i++)
    {
        BDBG_MSG(("     audio pid:0x%2x", pPid->getPid()));
    }
    for (int i = 0; NULL != (pPid = _pidMgr.getPid(i, ePidType_Ancillary)); i++)
    {
        BDBG_MSG(("     ancillary pid:0x%2x", pPid->getPid()));
    }
    pPid = _pidMgr.getPid(0, ePidType_Pcr);
    if (NULL != pPid)
    {
        BDBG_MSG(("     pcr   pid:0x%2x", pPid->getPid()));
    }

    if (true == bForce)
    {
        BDBG_SetModuleLevel("atlas_channel", level);
    }
} /* dump */

eRet CChannel::readXML(MXmlElement * xmlElemChannel)
{
    eRet    ret = eRet_Ok;
    MString strMajor;
    MString strMinor;

    BDBG_ASSERT(NULL != xmlElemChannel);

    strMajor = xmlElemChannel->attrValue(XML_ATT_MAJOR);
    if (false == strMajor.isEmpty())
    {
        setMajor(strMajor.toInt());
    }

    strMinor = xmlElemChannel->attrValue(XML_ATT_MINOR);
    if (false == strMinor.isEmpty())
    {
        setMinor(strMinor.toInt());
    }

    return(ret);
} /* readXML */

void CChannel::writeXML(MXmlElement * xmlElemChannel)
{
    BDBG_ASSERT(NULL != xmlElemChannel);

    xmlElemChannel->addAttr(XML_ATT_MAJOR, MString(getMajor()));
    xmlElemChannel->addAttr(XML_ATT_MINOR, MString(getMinor()));
}

bool CChannel::isRecording(void)
{
    if (_pRecord == NULL)
    {
        return(false);
    }
    else
    {
        return(true);
    }
}

bool CChannel::isEncoding(void)
{
#if NEXUS_HAS_VIDEO_ENCODER
    if (_pEncode == NULL)
    {
        return(false);
    }
    else
    {
        return(true);
    }
#else /* if NEXUS_HAS_VIDEO_ENCODER */
    return(false);

#endif /* if NEXUS_HAS_VIDEO_ENCODER */
}

bool CChannel::operator ==(CChannel &other)
{
    /*
     * check base class equivalency first
     * if (false == CModelSource::operator==(other))
     */
    if (other._pidMgr == _pidMgr)
    {
        if (other.getType() == getType())
        {
            return(true);
        }
    }

    return(false);
}

/* get PSI channel info for current tuned channel */
eRet CChannel::getChannelInfo(
        CHANNEL_INFO_T * pChanInfo,
        bool             bScanning
        )
{
    BERR_Code     err            = BERR_SUCCESS;
    CParserBand * pBand          = getParserBand();
    int           patTimeout     = 50; /* in tsPsi_setTimeout() this is 500msecs */
    int           patTimeoutOrig = 0;
    int           pmtTimeout     = 50; /* in tsPsi_setTimeout() this is 500msecs */
    int           pmtTimeoutOrig = 0;

    if (NULL == pBand)
    {
        return(eRet_NotAvailable);
    }

    if (true == bScanning)
    {
#ifdef MPOD_SUPPORT
        patTimeout = 800;
        pmtTimeout = 800;
#endif
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
    return((BERR_SUCCESS == err) ? eRet_Ok : eRet_ExternalError);
} /* getChannelInfo */

/* debug function to print out results of channel scan */
static void printinfo(CHANNEL_INFO_T * info)
{
    int i, j;

    BDBG_MSG((
                "\n"
                "CHANNEL_INFO_T\n"
                "==============\n"
                "version %d\n"
                "transport_stream_id %d\n"
                "sectionBitmap %d\n"
                "num_programs %d\n\n",
                info->version,
                info->transport_stream_id,
                info->sectionBitmap,
                info->num_programs));
    for (i = 0; i < info->num_programs; i++)
    {
        char   str[255];
        char * pStr = NULL;

        BDBG_MSG(("program_number %d", info->program_info[i].program_number));
        BDBG_MSG(("  version %d", info->program_info[i].version));
        BDBG_MSG(("  pcr pid   : 0x%x", info->program_info[i].pcr_pid));

        str[0] = 0;
        pStr   = str;
        for (j = 0; j < info->program_info[i].num_video_pids; j++)
        {
            snprintf(pStr,
                    sizeof(str) - 1,
                    " 0x%x/%d",
                    info->program_info[i].video_pids[j].pid,
                    info->program_info[i].video_pids[j].streamType);
            pStr = str + strlen(str);
        }
        BDBG_MSG(("  video_pids:%s", str));

        str[0] = 0;
        pStr   = str;
        for (j = 0; j < info->program_info[i].num_audio_pids; j++)
        {
            snprintf(pStr,
                    sizeof(str) - 1,
                    " 0x%x/%d",
                    info->program_info[i].audio_pids[j].pid,
                    info->program_info[i].audio_pids[j].streamType);
            pStr = str + strlen(str);
        }
        BDBG_MSG(("  audio_pids:%s", str));

        str[0] = 0;
        pStr   = str;
        for (j = 0; j < info->program_info[i].num_other_pids; j++)
        {
            snprintf(pStr,
                    sizeof(str) - 1,
                    " 0x%x/%d",
                    info->program_info[i].other_pids[j].pid,
                    info->program_info[i].other_pids[j].streamType);
            pStr = str + strlen(str);
        }
        BDBG_MSG(("  other_pids:%s", str));
    }
} /* printinfo */

/* take given tuned pChannel and search psi data for programs.  call callback with any found channels.
 * returns the number of found programs.  this method assumes the major channel number has already been
 * set (it will set minor channel numbers).  the given channel will also be updated with the first found
 * valid (as determined by callback) psi program info.*/
int CChannel::addPsiPrograms(
        CTunerScanCallback addChannelCallback,
        void *             context
        )
{
    eRet ret   = eRet_Ok;
    int  minor = 1;
    /*
     * coverity[stack_use_local_overflow]
     * coverity[stack_use_overflow]
     */
    CHANNEL_INFO_T chanInfo;

    BDBG_ASSERT(NULL != addChannelCallback);
    memset(&chanInfo, 0, sizeof(chanInfo));

    /* get PSI channel info for current tuned channel */
    ret = getChannelInfo(&chanInfo, true);
    CHECK_ERROR_GOTO("PSI channel info retrieval failed, skipping to next channel to be scanned", ret, error);

    /* channel info received */
    printinfo(&chanInfo); /* debug */

    /* PSI info retrieved - populate new channel objects and give to callback */
    for (int i = 0; i < chanInfo.num_programs; i++)
    {
        /* since all these channels share a common frequency, we'll assign minor
         * channel numbers here (minor numbers start at 1) */
        initialize(&chanInfo.program_info[i]);
        updateDescription();
        setMinor(minor);

        /* give new channel to callback */
        if (true == addChannelCallback(this, context))
        {
            minor++;
        }
    }

error:
    return(minor - 1);
} /* addPsiPrograms */

void CChannel::updateDescription(void)
{
    _strDescription      = boardResourceToString(getType());
    _strDescriptionShort = getChannelNum();

    addMetadata("Description", _strDescription);
    addMetadata("Number", getChannelNum());
    addMetadata("Type", boardResourceToString(getType()));
}

eRet CChannel::openPids(
        CSimpleAudioDecode * pAudioDecode,
        CSimpleVideoDecode * pVideoDecode
        )
{
    CParserBand * pParserBand = _pParserBand;
    CPid *        pVideoPid   = NULL;
    CPid *        pAudioPid   = NULL;
    CPid *        pPcrPid     = NULL;
    eRet          ret         = eRet_Ok;

    BSTD_UNUSED(pAudioDecode);
    BSTD_UNUSED(pVideoDecode);

    if (NULL == _pStc)
    {
        ret = eRet_InvalidState;
        goto error;
    }

    pVideoPid = getPid(0, ePidType_Video);
    pAudioPid = getPid(0, ePidType_Audio);
    pPcrPid   = getPid(0, ePidType_Pcr);

    /* open pids */
    if (pVideoPid)
    {
        CHECK_PTR_ERROR_GOTO("missing parser band - tune failed", pParserBand, ret, eRet_InvalidState, error);
        ret = pVideoPid->open(pParserBand);
        CHECK_ERROR_GOTO("open video pid channel failed", ret, error);
    }

    if (pAudioPid)
    {
        CHECK_PTR_ERROR_GOTO("missing parser band - tune failed", pParserBand, ret, eRet_InvalidState, error);
        ret = pAudioPid->open(pParserBand);
        CHECK_ERROR_GOTO("open audio pid channel failed", ret, error);
    }

    if (pPcrPid)
    {
        /* only open pcr pid channel if it is different from audio/video pids */
        if (false == pPcrPid->isOpen())
        {
            CHECK_PTR_ERROR_GOTO("missing parser band - tune failed", pParserBand, ret, eRet_InvalidState, error);
            ret = pPcrPid->open(pParserBand);
            CHECK_ERROR_GOTO("open Pcr pid channel failed", ret, error);
        }
    }

    _pStc->setStcType(((NULL != pVideoPid) && (NULL != pAudioPid)) ? eStcType_ParserBand : eStcType_ParserBandSyncOff);
    if (pPcrPid == NULL)
    {
        BDBG_ERR((" ERROR COME out PCR IS NULL"));
        return(eRet_ExternalError);
    }
    ret = _pStc->configure(pPcrPid);
    CHECK_ERROR_GOTO("error configuring stc channel", ret, error);

error:
    return(ret);
} /* openPids */

eRet CChannel::closePids()
{
    eRet ret = eRet_Ok;

    _pidMgr.closePidChannels();

error:
    return(ret);
} /* closePids */

const char * CChannel::getMetadataTag(int index)
{
    const char * pStr = _metadata.getName(index);

    return(pStr);
}

const char * CChannel::getMetadataValue(int index)
{
    MString * pStr = NULL;

    pStr = _metadata.getData(index);
    if (NULL != pStr)
    {
        return(pStr->s());
    }
    return(NULL);
}

eRet CChannel::start(
        CSimpleAudioDecode * pAudioDecode,
        CSimpleVideoDecode * pVideoDecode
        )
{
    BSTD_UNUSED(pAudioDecode);
    BSTD_UNUSED(pVideoDecode);

    return(eRet_Ok);
}
