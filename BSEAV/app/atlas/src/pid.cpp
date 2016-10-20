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

#include "pid.h"
#include "band.h"
#include "mxmlparser.h"
#include "mstringhash.h"
#include "convert.h"
#include "playback.h"
#include "record.h"
#include "video_decode.h"
#include "audio_decode.h"
#include "nexus_core_utils.h"

BDBG_MODULE(atlas_pid);

eRet CPid::parseAudio(MXmlElement * xmlElemPid)
{
    eRet ret = eRet_NotAvailable;

    MString          strAudio;
    NEXUS_AudioCodec audioType = NEXUS_AudioCodec_eUnknown;

    uint16_t audioPidNum = 0;

    strAudio = xmlElemPid->attrValue(XML_ATT_AUDIOTYPE);
    if (!strAudio || !strAudio.strncmp("", 1))
    {
        BDBG_MSG(("No Audio Pid present"));
        return(ret);
    }
    else
    {
        audioType = stringToAudioCodec(strAudio);
        BDBG_MSG((" Audio Codec %s", strAudio.s()));
    }

    strAudio = xmlElemPid->attrValue(XML_ATT_AUDIOPID);
    if (!strAudio || !strAudio.strncmp("", 1))
    {
        BDBG_WRN(("Audio PID is _not_ set!"));
        return(ret);
    }
    else
    {
        audioPidNum = strAudio.toInt();
    }

    {
        MString strSampleRate = xmlElemPid->attrValue(XML_ATT_SAMPLE_RATE);
        if (false == strSampleRate.isEmpty())
        {
            setSampleRate(strSampleRate.toInt());
        }
    }
    {
        MString strSampleSize = xmlElemPid->attrValue(XML_ATT_SAMPLE_SIZE);
        if (false == strSampleSize.isEmpty())
        {
            setSampleSize(strSampleSize.toInt());
        }
    }
    {
        MString strAudioChannels = xmlElemPid->attrValue(XML_ATT_NUM_CHANNELS);
        if (false == strAudioChannels.isEmpty())
        {
            setAudioChannels(strAudioChannels.toInt());
        }
    }

    BDBG_MSG(("Audio PID is set to %d", audioPidNum));

    /* Set pid info here. Need to move */
    _pid            = audioPidNum;
    _pidChannel     = NULL;
    _videoCodec     = NEXUS_VideoCodec_eUnknown;
    _videoFrameRate = NEXUS_VideoFrameRate_eUnknown;
    _audioCodec     = audioType;
    _isPcr          = false;

    return(eRet_Ok);
} /* parseAudio */

eRet CPid::parseVideo(MXmlElement * xmlElemPid)
{
    eRet ret = eRet_NotAvailable;

    MString              strVideo;
    NEXUS_VideoCodec     videoType      = NEXUS_VideoCodec_eUnknown;
    NEXUS_VideoFrameRate videoFrameRate = NEXUS_VideoFrameRate_eUnknown;
    uint16_t             videoPidNum    = 0;

    strVideo = xmlElemPid->attrValue(XML_ATT_VIDEOTYPE);
    if (!strVideo || !strVideo.strncmp("", 1))
    {
        BDBG_MSG(("Video Type is _not_ set!"));
        /* TRY PCR */
        strVideo = xmlElemPid->attrValue(XML_ATT_PCRPID);
        if (!strVideo || !strVideo.strncmp("", 1))
        {
            BDBG_MSG(("PCR Type is _not_ set!"));
            return(ret);
        }
        else
        {   /* PCR */
            videoPidNum = strVideo.toInt();
            BDBG_MSG(("PCR PID is set to %d", videoPidNum));
            _isPcr      = true;
            _pidType    = ePidType_Pcr;
            _pidChannel = NULL;
            _pid        = videoPidNum;
            return(eRet_Ok);
        }
    }
    else
    {
        videoType = stringToVideoCodec(strVideo);
        BDBG_MSG((" Video Codec %s", strVideo.s()));
    }

    {
        MString strWidth = xmlElemPid->attrValue(XML_ATT_WIDTH);
        if (false == strWidth.isEmpty())
        {
            setWidth(strWidth.toInt());
        }
    }
    {
        MString strHeight = xmlElemPid->attrValue(XML_ATT_HEIGHT);
        if (false == strHeight.isEmpty())
        {
            setHeight(strHeight.toInt());
        }
    }
    {
        strVideo = xmlElemPid->attrValue(XML_ATT_FRAMERATE);
        if (!strVideo || !strVideo.strncmp("", 1))
        {
            BDBG_MSG(("Video Frame Rate is not set"));
        }
        else
        {
            NEXUS_LookupFrameRate(strVideo.toInt(), &videoFrameRate);
            BDBG_MSG(("Frame Rate %0.00f NEXUS_VideoFrameRate=%d", strVideo.toInt()/1000.0, videoFrameRate));
        }
    }

    {
        strVideo = xmlElemPid->attrValue(XML_ATT_VIDEOPID);
        if (!strVideo || !strVideo.strncmp("", 1))
        {
            BDBG_WRN(("Video PID is _not_ set"));
            return(ret);
        }
        else
        {
            videoPidNum = strVideo.toInt();
        }
    }

    BDBG_MSG(("Video PID is set to %d", videoPidNum));

    /* Set pid info here. Need to move */
    _pid            = videoPidNum;
    _pidChannel     = NULL;
    _videoCodec     = videoType;
    _videoFrameRate = videoFrameRate;
    _audioCodec     = NEXUS_AudioCodec_eUnknown;
    /*_isPcr = true;  default to PCR pid */
    _pidType = ePidType_Video;

    return(eRet_Ok);
} /* parseVideo */

void CPid::readXML(MXmlElement * xmlElemPid)
{
    if (xmlElemPid->tag() != XML_TAG_PID)
    {
        return;
    }

    {
        MString strBitrate = xmlElemPid->attrValue(XML_ATT_BITRATE);
        if (false == strBitrate.isEmpty())
        {
            setBitrate(strBitrate.toInt());
        }
    }

    if (parseAudio(xmlElemPid) != eRet_Ok)
    {
        /* PCR case handled by parseVideo() */
        if (parseVideo(xmlElemPid) == eRet_Ok)
        {
            BDBG_MSG(("Added Video Pid"));
        }
        else
        {
            BDBG_WRN(("Not a Valid Pid Entry"));
            _pidType = ePidType_Unknown;
        }
    }
    else
    {
        BDBG_MSG(("Added Audio Pid"));
        _pidType = ePidType_Audio;
    }

    if (_pidType != ePidType_Unknown)
    {
        if ((xmlElemPid->attrValue(XML_ATT_TYPE)) == MString("playback"))
        {
            _isPlayback = true;
        }
    }

#if NEXUS_HAS_SECURITY
    {
        MXmlElement * xmlElemSecurity = NULL;

        xmlElemSecurity = xmlElemPid->findChild(XML_TAG_SECURITY);
        if (xmlElemSecurity != NULL)
        {
            CCryptoCP * pCrypto = new CCryptoCP("DVR Crypto");
            pCrypto->readXML(xmlElemSecurity);
            _pCrypto = (CCrypto *) pCrypto;
        }
    }
#else /* if NEXUS_HAS_SECURITY */

#endif /* if NEXUS_HAS_SECURITY */
} /* readXML */

CPid::CPid(
        uint16_t pid,
        ePidType type
        ) :
    _pid(pid),
    _caPid(0),
    _pidType(type),
    _isPcr(false),
    _isPlayback(false),
    _isDVREncryption(false),
    _pidChannel(NULL),
    _videoCodec(NEXUS_VideoCodec_eUnknown),
    _videoFrameRate(NEXUS_VideoFrameRate_eUnknown),
    _audioCodec(NEXUS_AudioCodec_eUnknown),
    _pCrypto(NULL),
    _width(0),
    _height(0),
    _bitrate(0),
    _audioSampleRate(0),
    _audioSampleSize(0),
    _audioChannels(0)
{
    memset(&_settings, 0, sizeof(_settings));
    memset(&_playbackSettings, 0, sizeof(_playbackSettings));
    memset(&_playpumpSettings, 0, sizeof(_playpumpSettings));
    memset(&_recordSettings, 0, sizeof(_recordSettings));

    NEXUS_PidChannel_GetDefaultSettings(&_settings);
    NEXUS_Playback_GetDefaultPidChannelSettings(&_playbackSettings);
    NEXUS_Playpump_GetDefaultOpenPidChannelSettings(&_playpumpSettings);
    NEXUS_Record_GetDefaultPidChannelSettings(&_recordSettings);
}

CPid::CPid(
        uint16_t         pid,
        NEXUS_VideoCodec videoCodec
        ) :
    _pid(pid),
    _caPid(0),
    _pidType(ePidType_Video),
    _isPcr(false),
    _isPlayback(false),
    _isDVREncryption(false),
    _pidChannel(NULL),
    _videoCodec(videoCodec),
    _videoFrameRate(NEXUS_VideoFrameRate_eUnknown),
    _audioCodec(NEXUS_AudioCodec_eUnknown),
    _pCrypto(NULL),
    _width(0),
    _height(0),
    _bitrate(0),
    _audioSampleRate(0),
    _audioSampleSize(0),
    _audioChannels(0)
{
    memset(&_settings, 0, sizeof(_settings));
    memset(&_playbackSettings, 0, sizeof(_playbackSettings));
    memset(&_playpumpSettings, 0, sizeof(_playpumpSettings));
    memset(&_recordSettings, 0, sizeof(_recordSettings));

    NEXUS_PidChannel_GetDefaultSettings(&_settings);
    NEXUS_Playback_GetDefaultPidChannelSettings(&_playbackSettings);
    NEXUS_Playpump_GetDefaultOpenPidChannelSettings(&_playpumpSettings);
    NEXUS_Record_GetDefaultPidChannelSettings(&_recordSettings);
}

CPid::CPid(
        uint16_t         pid,
        NEXUS_AudioCodec audioCodec
        ) :
    _pid(pid),
    _caPid(0),
    _pidType(ePidType_Audio),
    _isPcr(false),
    _isPlayback(false),
    _isDVREncryption(false),
    _pidChannel(NULL),
    _videoCodec(NEXUS_VideoCodec_eUnknown),
    _videoFrameRate(NEXUS_VideoFrameRate_eUnknown),
    _audioCodec(audioCodec),
    _pCrypto(NULL),
    _width(0),
    _height(0),
    _bitrate(0),
    _audioSampleRate(0),
    _audioSampleSize(0),
    _audioChannels(0)
{
    memset(&_settings, 0, sizeof(_settings));
    memset(&_playbackSettings, 0, sizeof(_playbackSettings));
    memset(&_playpumpSettings, 0, sizeof(_playpumpSettings));
    memset(&_recordSettings, 0, sizeof(_recordSettings));

    NEXUS_PidChannel_GetDefaultSettings(&_settings);
    NEXUS_Playback_GetDefaultPidChannelSettings(&_playbackSettings);
    NEXUS_Playpump_GetDefaultOpenPidChannelSettings(&_playpumpSettings);
    NEXUS_Record_GetDefaultPidChannelSettings(&_recordSettings);
}

CPid::CPid(MXmlElement * xmlElemPid) :
    _pid(0),
    _caPid(0),
    _pidType(ePidType_Unknown),
    _isPcr(false),
    _isPlayback(false),
    _isDVREncryption(false),
    _pidChannel(NULL),
    _videoCodec(NEXUS_VideoCodec_eUnknown),
    _videoFrameRate(NEXUS_VideoFrameRate_eUnknown),
    _audioCodec(NEXUS_AudioCodec_eUnknown),
    _pCrypto(NULL),
    _width(0),
    _height(0),
    _bitrate(0),
    _audioSampleRate(0),
    _audioSampleSize(0),
    _audioChannels(0)
{
    memset(&_settings, 0, sizeof(_settings));
    memset(&_playbackSettings, 0, sizeof(_playbackSettings));
    memset(&_playpumpSettings, 0, sizeof(_playpumpSettings));
    memset(&_recordSettings, 0, sizeof(_recordSettings));

    NEXUS_PidChannel_GetDefaultSettings(&_settings);
    NEXUS_Playback_GetDefaultPidChannelSettings(&_playbackSettings);
    NEXUS_Playpump_GetDefaultOpenPidChannelSettings(&_playpumpSettings);
    NEXUS_Record_GetDefaultPidChannelSettings(&_recordSettings);

    readXML(xmlElemPid);
}

CPid::CPid(const CPid & pid) :
    _pid(pid._pid),
    _caPid(pid._caPid),
    _pidType(pid._pidType),
    _isPcr(pid._isPcr),
    _isPlayback(pid._isPlayback),
    _isDVREncryption(pid._isDVREncryption),
    _pidChannel(NULL),
    _videoCodec(pid._videoCodec),
    _videoFrameRate(pid._videoFrameRate),
    _audioCodec(pid._audioCodec),
    _pCrypto(NULL),
    _width(pid._width),
    _height(pid._height),
    _bitrate(pid._bitrate),
    _audioSampleRate(pid._audioSampleRate),
    _audioSampleSize(pid._audioSampleSize),
    _audioChannels(pid._audioChannels)
{
    memset(&_settings, 0, sizeof(_settings));
    memset(&_playbackSettings, 0, sizeof(_playbackSettings));
    memset(&_playpumpSettings, 0, sizeof(_playpumpSettings));
    memset(&_recordSettings, 0, sizeof(_recordSettings));

    if (pid._pCrypto)
    {
        CCryptoCP * crypto = new CCryptoCP((const CCryptoCP &)*pid._pCrypto);
        _pCrypto = (CCrypto *) crypto;
    }
    else
    {
        _pCrypto = NULL;
    }

    NEXUS_PidChannel_GetDefaultSettings(&_settings);
    NEXUS_Playback_GetDefaultPidChannelSettings(&_playbackSettings);
    NEXUS_Playpump_GetDefaultOpenPidChannelSettings(&_playpumpSettings);
    NEXUS_Record_GetDefaultPidChannelSettings(&_recordSettings);
}

void CPid::setVideoDecoder(CSimpleVideoDecode * pDecoder)
{
    if (NULL == pDecoder)
    {
        _playbackSettings.pidTypeSettings.video.simpleDecoder = NULL;
    }
    else
    {
        _playbackSettings.pidTypeSettings.video.simpleDecoder = pDecoder->getSimpleDecoder();
    }
}

void CPid::setAudioDecoder(CSimpleAudioDecode * pDecoder)
{
    if (NULL == pDecoder)
    {
        _playbackSettings.pidTypeSettings.audio.simpleDecoder = NULL;
        _playbackSettings.pidTypeSettings.audio.primary       = NULL;
    }
    else
    {
        _playbackSettings.pidTypeSettings.audio.simpleDecoder = pDecoder->getSimpleDecoder();
    }
}

void CPid::getSettings(NEXUS_PlaypumpOpenPidChannelSettings * pSettings)
{
    BKNI_Memcpy(pSettings, &_playpumpSettings, sizeof(_playpumpSettings));
}

void CPid::setSettings(NEXUS_PlaypumpOpenPidChannelSettings * pSettings)
{
    BKNI_Memcpy(&_playpumpSettings, pSettings, sizeof(_playpumpSettings));
}

eRet CPid::setSettings(CPlayback * pPlayback)
{
    NEXUS_Error nerror = NEXUS_SUCCESS;
    eRet        ret    = eRet_Ok;

    nerror = NEXUS_Playback_SetPidChannelSettings(pPlayback->getPlayback(), _pidChannel, &_playbackSettings);
    CHECK_NEXUS_ERROR_GOTO("cannot NEXUS_Playback_SetPidChannelSettings", nerror, ret, error);

    BDBG_MSG(("Success setting the _playbacksettings simpleDecoder=%s ", (_playbackSettings.pidTypeSettings.audio.primary) ? "VALID" : "NULL"));

error:
    return(ret);
}

void CPid::encrypt(
        NEXUS_SecurityAlgorithm algo,
        uint8_t *               key,
        bool                    encrypt
        )
{
    if (_pCrypto)
    {
        /* Info should already be done by read XML or it will go to default */
        _pCrypto->encrypt(encrypt);
        _pCrypto->keySlotConfig();
        _pCrypto->setAlgorithmSettings();
    }
    else
    {
        BDBG_MSG(("Setting up new Crypto"));
        CCryptoCP * pCrypto = new CCryptoCP("Crypto Pid");
        pCrypto->encrypt(encrypt);
        pCrypto->setAlgorithm(algo);
        pCrypto->setKey(key);
        pCrypto->keySlotConfig();
        pCrypto->setAlgorithmSettings();
        _pCrypto = (CCrypto *) pCrypto;
    }

    BDBG_MSG(("%s pid ", encrypt ? "encrypt" : "decrypt"));
} /* encrypt */

void CPid::writeXML(MXmlElement * xmlElem)
{
    MXmlElement * xmlElemPid = new MXmlElement(xmlElem, XML_TAG_PID);

    if (!xmlElemPid)
    {
        BDBG_ERR((" Cannot Create PID element "));
        return;
    }

    if (_isPlayback)
    {
        xmlElemPid->addAttr(XML_ATT_TYPE, MString("playback"));
    }

    if (0 < getBitrate())
    {
        xmlElemPid->addAttr(XML_ATT_BITRATE, MString(getBitrate()));
    }

    switch (_pidType)
    {
    case ePidType_Audio:
        BDBG_MSG((" Value of Audio string is 0x%x", _pid));
        xmlElemPid->addAttr(XML_ATT_AUDIOPID, MString(_pid));
        xmlElemPid->addAttr(XML_ATT_AUDIOTYPE, audioCodecToString(_audioCodec));

        if (0 < getSampleRate())
        {
            xmlElemPid->addAttr(XML_ATT_SAMPLE_RATE, MString(getSampleRate()));
        }
        if (0 < getSampleSize())
        {
            xmlElemPid->addAttr(XML_ATT_SAMPLE_SIZE, MString(getSampleSize()));
        }
        if (0 < getAudioChannels())
        {
            xmlElemPid->addAttr(XML_ATT_NUM_CHANNELS, MString(getAudioChannels()));
        }
        break;

    case ePidType_Video:
        BDBG_MSG((" Value of Video string is 0x%x", _pid));
        xmlElemPid->addAttr(XML_ATT_VIDEOPID, MString(_pid));
        xmlElemPid->addAttr(XML_ATT_VIDEOTYPE, videoCodecToString(_videoCodec));
        if (0 < getWidth())
        {
            xmlElemPid->addAttr(XML_ATT_WIDTH, MString(getWidth()));
        }
        if (0 < getHeight())
        {
            xmlElemPid->addAttr(XML_ATT_HEIGHT, MString(getHeight()));
        }
        if (_videoFrameRate != NEXUS_VideoFrameRate_eUnknown)
        {
            xmlElemPid->addAttr(XML_ATT_FRAMERATE, videoFrameRateThousandthsToString(_videoFrameRate));
        }
        break;

    case ePidType_Ancillary:
        xmlElemPid->addAttr(XML_ATT_ANCILLARYPID, NULL);
        break;

    case ePidType_Pcr:
        BDBG_MSG((" Value of PCR string is 0x%x", _pid));
        xmlElemPid->addAttr(XML_ATT_PCRPID, MString(_pid));
        break;

    default:
        BDBG_WRN((" unknown PID type:%d", _pidType));
        delete xmlElemPid;
        return;

        break;
    } /* switch */

    if (_pCrypto)
    {
        _pCrypto->writeXML(xmlElemPid);
    }
} /* writeXML */

CPid::~CPid()
{
    DEL(_pCrypto);
}

eRet CPid::open(CParserBand * pParserBand)
{
    eRet ret = eRet_Ok;

    BDBG_ASSERT(pParserBand);

    if (true == isOpen())
    {
        BDBG_WRN(("trying to open an already opened pid channel (pid:0x%x)", _pid));
        goto error;
    }

    _pidChannel = NEXUS_PidChannel_Open(pParserBand->getBand(), _pid, &_settings);
    CHECK_PTR_ERROR_GOTO("nexus pid channel open failed", _pidChannel, ret, eRet_NotAvailable, error);

error:
    return(ret);
} /* open */

/* Playpump Version */
eRet CPid::open(CPlaypump * pPlaypump)
{
    eRet ret = eRet_Ok;

    switch (_pidType)
    {
    case ePidType_Video:
        _playpumpSettings.pidType = NEXUS_PidType_eVideo;
        BDBG_MSG((" Opening VIDEO PID %d", _pid));
        break;
    case ePidType_Audio:
        _playpumpSettings.pidType                     = NEXUS_PidType_eAudio;
        _playpumpSettings.pidTypeSettings.audio.codec = _audioCodec;
        BDBG_MSG((" Opending AUDIO PID %d", _pid));
        break;
    case ePidType_Pcr:
        _playpumpSettings.pidType = NEXUS_PidType_eOther;
        break;
    default:
        BDBG_ERR((" Incorrect PID Type %d", _pidType));
        goto error;
    } /* switch */

    _pidChannel = NEXUS_Playpump_OpenPidChannel(pPlaypump->getPlaypump(), _pid, &_playpumpSettings);
    CHECK_PTR_ERROR_GOTO("nexus playpump pid channel open failed", _pidChannel, ret, eRet_NotAvailable, error);

    if (_pCrypto)
    {
        BDBG_MSG(("%s About to loadKey, H ", __FUNCTION__));
        _pCrypto->loadKey(NULL, _pidChannel);
    }
    BDBG_MSG(("pid channel %p successful", (void *)_pidChannel));

error:
    return(ret);
} /* open */

/* Playback Version */
eRet CPid::open(CPlayback * pPlayback)
{
    eRet ret = eRet_Ok;

    if (true == isOpen())
    {
        BDBG_WRN(("trying to open an already opened pid channel (pid:0x%0x)", _pid));
        goto error;
    }

    /* Only set what Pid knows about */
    switch (_pidType)
    {
    case ePidType_Video:
        _playbackSettings.pidSettings.pidType         = NEXUS_PidType_eVideo;
        _playbackSettings.pidTypeSettings.video.codec = _videoCodec; /* must be told codec for correct handling */
        if (pPlayback->hasIndex())
        {
            _playbackSettings.pidTypeSettings.video.index = true;
        }
        break;
    case ePidType_Audio:
        _playbackSettings.pidSettings.pidType = NEXUS_PidType_eAudio;
        break;
    case ePidType_Pcr:
        _playbackSettings.pidSettings.pidType = NEXUS_PidType_eOther;
        break;
    default:
        BDBG_ERR(("Cannot open PidType %d", _pidType));
        goto error;
    } /* switch */

    _pidChannel = NEXUS_Playback_OpenPidChannel(pPlayback->getPlayback(), _pid, &_playbackSettings);
    CHECK_PTR_ERROR_GOTO("nexus playback pid channel open failed", _pidChannel, ret, eRet_NotAvailable, error);

    if (_pCrypto)
    {
        BDBG_MSG(("%s About to loadKey T", __FUNCTION__));
        _pCrypto->loadKey(NULL, _pidChannel);
    }

error:
    return(ret);
} /* open */

/* Record Version */
eRet CPid::open(CRecord * pRecord)
{
    eRet ret = eRet_Ok;

    CParserBand * pParserBand = pRecord->getBand();

    /* IP VERSION */
    if (pParserBand == NULL)
    {
        if (_pidChannel == NULL)
        {
            BDBG_ERR(("You must call open(pPlayback) for PID Channel before you call open(pRecord)"));
            goto error;
        }
        BDBG_MSG((" Non-Tuner Entry!"));
        if (_pidType == ePidType_Video)
        {
            /* configure the video pid for indexing */
            BKNI_Memset(&_recordSettings, 0, sizeof(_recordSettings));
            NEXUS_Record_GetDefaultPidChannelSettings(&_recordSettings);
            _recordSettings.recpumpSettings.pidType = NEXUS_PidType_eVideo;
            if (pRecord->hasIndex())
            {
                _recordSettings.recpumpSettings.pidTypeSettings.video.index = true;
            }
            else
            {
                BDBG_MSG(("NO INDEX for REcording"));
            }
            _recordSettings.recpumpSettings.pidTypeSettings.video.codec = _videoCodec; /* must be told codec for correct handling */
            /* Need to do special handling for H264 case */
            if (NEXUS_Record_AddPidChannel(pRecord->getRecord(), _pidChannel, &_recordSettings) != NEXUS_SUCCESS)
            {
                BDBG_ERR((" Cannot add VIDEO pidChannel to Record_AddPidChannel"));
                ret = eRet_NotAvailable;
                goto error;
            }

            /* H.264 DQT requires indexing of random access indicator using TPIT */
            if (_recordSettings.recpumpSettings.pidTypeSettings.video.codec == NEXUS_VideoCodec_eH264)
            {
                (pRecord->getRecpump())->setTpitFilter(_pidChannel);
            }
        }
        else
        if (_pidType == ePidType_Audio)
        {
            if (NEXUS_Record_AddPidChannel(pRecord->getRecord(), _pidChannel, NULL) != NEXUS_SUCCESS)
            {
                BDBG_ERR((" Cannot add AUDIO pidChannel to Record_AddPidChannel"));
                ret = eRet_NotAvailable;
                goto error;
            }
        }
        else
        if (_pidType == ePidType_Pcr)
        {
            if (NEXUS_Record_AddPidChannel(pRecord->getRecord(), _pidChannel, NULL) != NEXUS_SUCCESS)
            {
                BDBG_ERR((" Cannot add PCR pidChannel to Record_AddPidChannel"));
                ret = eRet_NotAvailable;
                goto error;
            }
        }

        goto done;
    }

    if (true != isOpen())
    {
        BDBG_WRN(("trying to record a not open pid channel (pid:0x%0x)", _pid));
        goto error;
    }

    /* Only set what Pid knows about */
    if (_pidType == ePidType_Video)
    {
        /* configure the video pid for indexing */
        BKNI_Memset(&_recordSettings, 0, sizeof(_recordSettings));
        NEXUS_Record_GetDefaultPidChannelSettings(&_recordSettings);
        _recordSettings.recpumpSettings.pidType = NEXUS_PidType_eVideo;
        if (pRecord->hasIndex())
        {
            _recordSettings.recpumpSettings.pidTypeSettings.video.index = true;
        }
        else
        {
            BDBG_MSG(("NO INDEX for REcording"));
        }
        _recordSettings.recpumpSettings.pidTypeSettings.video.codec = _videoCodec; /* must be told codec for correct handling */
        /* Need to do special handling for H264 case */
        print();
        if (NEXUS_Record_AddPidChannel(pRecord->getRecord(), _pidChannel, &_recordSettings) != NEXUS_SUCCESS)
        {
            BDBG_ERR((" Cannot add VIDEO pidChannel to Record_AddPidChannel"));
            ret = eRet_NotAvailable;
            goto error;
        }

        /* H.264 DQT requires indexing of random access indicator using TPIT */
        if (_recordSettings.recpumpSettings.pidTypeSettings.video.codec == NEXUS_VideoCodec_eH264)
        {
            (pRecord->getRecpump())->setTpitFilter(_pidChannel);
        }
    }
    else
    if (_pidType == ePidType_Audio)
    {
        if (NEXUS_Record_AddPidChannel(pRecord->getRecord(), _pidChannel, NULL) != NEXUS_SUCCESS)
        {
            BDBG_ERR((" Cannot add AUDIO pidChannel to Record_AddPidChannel"));
            ret = eRet_NotAvailable;
            goto error;
        }
    }
    else
    if (_pidType == ePidType_Pcr)
    {
        if (NEXUS_Record_AddPidChannel(pRecord->getRecord(), _pidChannel, NULL) != NEXUS_SUCCESS)
        {
            BDBG_ERR((" Cannot add PCR pidChannel to Record_AddPidChannel"));
            ret = eRet_NotAvailable;
            goto error;
        }
    }

done:
    if (_pCrypto)
    {
        BDBG_MSG(("%s About to loadKey Z ", __FUNCTION__));
        _pCrypto->loadKey(NULL, _pidChannel);
    }
error:
    return(ret);
} /* open */

eRet CPid::close(void)
{
    eRet ret = eRet_Ok;

    CHECK_PTR_MSG_GOTO("Nexus PidChannel Not Opened", _pidChannel, ret, eRet_NotAvailable, error);
    if (_pCrypto)
    {
        _pCrypto->removeKey(_pidChannel);
    }

    NEXUS_PidChannel_Close(_pidChannel);

error:
    BDBG_MSG(("PidChannel close (pid:0x%0x)", _pid));
    _pidChannel = NULL;
    return(ret);
} /* close */

eRet CPid::close(CPlaypump * pPlaypump)
{
    NEXUS_Error nerror = NEXUS_SUCCESS;
    eRet        ret    = eRet_Ok;

    CHECK_PTR_MSG_GOTO("Nexus PidChannel Not Opened", _pidChannel, ret, eRet_NotAvailable, error);
    CHECK_PTR_ERROR_GOTO("Invalid Nexus Playpump Handle", pPlaypump, ret, eRet_InvalidParameter, error);

    if (_pCrypto)
    {
        _pCrypto->removeKey(_pidChannel);
    }

    nerror = NEXUS_Playpump_ClosePidChannel(pPlaypump->getPlaypump(), _pidChannel);
    CHECK_NEXUS_ERROR_GOTO("Cannot Close Pid Channel", ret, nerror, error);

error:
    BDBG_MSG(("PidChannel close (pid:0x%0x)", _pid));
    _pidChannel = NULL;
    return(ret);
} /* close */

eRet CPid::close(CPlayback * pPlayback)
{
    eRet ret = eRet_Ok;

    CHECK_PTR_MSG_GOTO("Nexus PidChannel Not Opened", _pidChannel, ret, eRet_NotAvailable, error);
    CHECK_PTR_ERROR_GOTO("Invalid Nexus Playback Handle", pPlayback, ret, eRet_InvalidParameter, error);

    if (_pCrypto)
    {
        _pCrypto->removeKey(_pidChannel);
    }
    NEXUS_Playback_ClosePidChannel(pPlayback->getPlayback(), _pidChannel);

error:
    BDBG_MSG(("PidChannel (pid:0x%0x) closed", _pid));
    _pidChannel = NULL;
    return(ret);
} /* close */

/* This pid close is only to remove the Record Pid Channel */
eRet CPid::close(CRecord * pRecord)
{
    NEXUS_Error nerror = NEXUS_SUCCESS;
    eRet        ret    = eRet_Ok;

    CHECK_PTR_MSG_GOTO("Nexus PidChannel Not Opened", _pidChannel, ret, eRet_NotAvailable, error);
    CHECK_PTR_ERROR_GOTO("Invalid Nexus Record Handle", pRecord, ret, eRet_InvalidParameter, error);

    nerror = NEXUS_Record_RemovePidChannel(pRecord->getRecord(), _pidChannel);
    CHECK_NEXUS_ERROR_GOTO("Cannot Close Pid Channel", ret, nerror, error);

error:
    BDBG_MSG(("PidChannel (pid:0x%0x) removed record ", _pid));
    return(ret);
} /* close */

void CPid::print(bool bForce)
{
    BDBG_Level level;

    if (true == bForce)
    {
        BDBG_GetModuleLevel("atlas_pid", &level);
        BDBG_SetModuleLevel("atlas_pid", BDBG_eMsg);
    }

    if (_pidType == ePidType_Video)
    {
        BDBG_MSG(("VIDEO PID number is 0x%0x, %s PID, Video Codec %s width:%d height:%d open:%d",
                  _pid, (_isPlayback) ? "Playback" : "LIVE", videoCodecToString(_videoCodec).s(), getWidth(), getHeight(), isOpen()));
    }
    else
    if (_pidType == ePidType_Audio)
    {
        BDBG_MSG(("AUDIO PID number is 0x%0x, %s PID, Audio Codec %s open:%d",
                  _pid, (_isPlayback) ? "Playback" : "LIVE", audioCodecToString(_audioCodec).s(), isOpen()));
    }

    if (_isPlayback)
    {
        BDBG_MSG(("PlaybackPIDSettings.pidSettings.pidChannelIndex = %d open:%d", _playbackSettings.pidSettings.pidSettings.pidChannelIndex, isOpen()));
    }

    if (true == bForce)
    {
        BDBG_SetModuleLevel("atlas_pid", level);
    }
} /* print */

bool CPid::operator ==(CPid &other)
{
    if (getPid() == other.getPid())
    {
        if (getPidType() == other.getPidType())
        {
            return(true);
        }
    }
    return(false);
}