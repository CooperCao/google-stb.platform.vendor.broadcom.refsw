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

#ifndef PID_H__
#define PID_H__

#include "atlas.h"
#include "nexus_video_types.h"
#include "nexus_audio_types.h"
#include "nexus_pid_channel.h"
#include "nexus_playback.h"
#include "nexus_record.h"
#include "mxmlelement.h"
#include "xmltags.h"
#include "crypto.h"

#ifdef __cplusplus
extern "C" {
#endif

class CParserBand;
class CPlaypump;
class CPlayback;
class CRecord;
class CSimpleVideoDecode;
class CSimpleAudioDecode;

typedef enum
{
    ePidType_Unknown,
    ePidType_Video,
    ePidType_Audio,
    ePidType_Ancillary,
    ePidType_Pcr,
    ePidType_Max
} ePidType;

/* 0 is a valid pid, but only with respect to PSI table acquisition */
#define VALID_PID(pid)  ((pid) != 0 && (pid) < 0x7FFFFFFF)

class CPid
{
public:
    CPid(
            unsigned pid = 0xFFFF,
            ePidType type = ePidType_Unknown
            );
    CPid(
            unsigned         pid,
            NEXUS_VideoCodec videoCodec
            );
    CPid(
            unsigned         pid,
            NEXUS_AudioCodec audioCodec
            );
    CPid(MXmlElement * xmlElemPid);
    CPid(const CPid & pid); /* copy constructor */

    virtual ~CPid(void);
    virtual bool operator ==(CPid &other);

    eRet                   open(CParserBand * pParserBand);
    eRet                   open(CPlayback * pPlayback);
    eRet                   open(CPlaypump * pPlaypump);
    eRet                   open(CRecord * pRecord);
    eRet                   close(void);
    eRet                   close(CPlaypump * pPlaypump);
    eRet                   close(CPlayback * pPlayback);
    eRet                   close(CRecord * pRecord);
    eRet                   parseAudio(MXmlElement * xmlElemPid);
    eRet                   parseVideo(MXmlElement * xmlElemPid);
    void                   readXML(MXmlElement * xmlElemPid);
    void                   writeXML(MXmlElement * xmlElem);
    unsigned               getPid(void)                                     { return(_pid); }
    void                   setPid(unsigned pid)                             { _pid = pid; }
    NEXUS_PidChannelHandle getPidChannel(void)                              { return(_pidChannel); }
    void                   setPidChannel(NEXUS_PidChannelHandle pidChannel) { _pidChannel = pidChannel; }
    void                   setPidType(ePidType pidType)                     { _pidType = pidType; }
    void                   setPcrType(bool isPcr)                           { _isPcr = isPcr; }
    void                   setPlayback(bool isPlayback)                     { _isPlayback = isPlayback; }
    void                   setVideoDecoder(CSimpleVideoDecode * pDecoder = NULL);
    void                   setAudioDecoder(CSimpleAudioDecode * pDecoder = NULL);
    void                   encrypt(NEXUS_SecurityAlgorithm algo = NEXUS_SecurityAlgorithm_eMax, uint8_t * key = NULL, bool encrypt = false);
    ePidType               getPidType(void)                                  { return(_pidType); }
    void                   getSettings(NEXUS_PidChannelSettings * pSettings) { *pSettings = _settings; }
    void                   setSettings(NEXUS_PidChannelSettings * pSettings) { _settings = *pSettings; }
    eRet                   setSettings(CPlayback * pPlayback);
    void                   getSettings(NEXUS_PlaybackPidChannelSettings * pSettings) { *pSettings = _playbackSettings; }
    void                   setSettings(NEXUS_PlaybackPidChannelSettings * pSettings) { _playbackSettings = *pSettings; }
    void                   getSettings(NEXUS_PlaypumpOpenPidChannelSettings * pSettings);
    void                   setSettings(NEXUS_PlaypumpOpenPidChannelSettings * pSettings);
    unsigned               getCaPid(void)                                         { return(_caPid); }
    void                   setCaPid(unsigned caPid)                               { _caPid = caPid; }
    NEXUS_VideoCodec       getVideoCodec()                                        { return(_videoCodec); }
    NEXUS_VideoFrameRate   getVideoFrameRate()                                    { return(_videoFrameRate); }
    NEXUS_AudioCodec       getAudioCodec()                                        { return(_audioCodec); }
    void                   setVideoCodec(NEXUS_VideoCodec videoCodec)             { _videoCodec = videoCodec; }
    void                   setVideoFrameRate(NEXUS_VideoFrameRate videoFrameRate) { _videoFrameRate = videoFrameRate; }
    void                   setAudioCodec(NEXUS_AudioCodec audioCodec)             { _audioCodec = audioCodec; }
    bool                   isPcrPid(void)                                         { return((true == _isPcr) || (ePidType_Pcr == _pidType)); }
    bool                   isUniquePcrPid(void)                                   { return(ePidType_Pcr == _pidType); }
    bool                   isVideoPid(void)                                       { return(ePidType_Video == _pidType); }
    bool                   isAudioPid(void)                                       { return(ePidType_Audio == _pidType); }
    bool                   isAncillaryPid(void)                                   { return(ePidType_Ancillary == _pidType); }
    bool                   isEncrypted(void)                                      { return(VALID_PID(_caPid)); }
    bool                   isPlayback(void)                                       { return(_isPlayback); }
    bool                   isOpen()                                               { return(NULL != _pidChannel); }
    bool                   isDVREncryption(void)                                  { return((_pCrypto != NULL) ? true : false);  }
    unsigned               getWidth(void)                                         { return(_width); }
    void                   setWidth(unsigned width)                               { _width = width; }
    unsigned               getHeight(void)                                        { return(_height); }
    void                   setHeight(unsigned height)                             { _height = height; }
    unsigned               getBitrate(void)                                       { return(_bitrate); }
    void                   setBitrate(unsigned bitrate)                           { _bitrate = bitrate; }
    unsigned               getSampleRate(void)                                    { return(_audioSampleRate); }
    void                   setSampleRate(unsigned sampleRate)                     { _audioSampleRate = sampleRate; }
    unsigned               getSampleSize(void)                                    { return(_audioSampleSize); }
    void                   setSampleSize(unsigned sampleSize)                     { _audioSampleSize = sampleSize; }
    unsigned               getAudioChannels(void)                                 { return(_audioChannels); }
    void                   setAudioChannels(unsigned audioChannels)               { _audioChannels = audioChannels; }
    void                   print(bool bForce = false);
    void                   dump(bool bForce = false) { print(bForce); }

protected:
    unsigned                             _pid;
    unsigned                             _caPid;
    ePidType                             _pidType;
    bool                                 _isPcr;
    bool                                 _isPlayback;
    bool                                 _isDVREncryption;
    NEXUS_PidChannelHandle               _pidChannel;
    NEXUS_PidChannelSettings             _settings;
    NEXUS_PlaybackPidChannelSettings     _playbackSettings;
    NEXUS_PlaypumpOpenPidChannelSettings _playpumpSettings;
    NEXUS_RecordPidChannelSettings       _recordSettings;
    NEXUS_VideoCodec                     _videoCodec;
    NEXUS_VideoFrameRate                 _videoFrameRate;
    NEXUS_AudioCodec                     _audioCodec;
    CCrypto *                            _pCrypto;
    unsigned                             _width;
    unsigned                             _height;
    unsigned                             _bitrate;
    unsigned                             _audioSampleRate;
    unsigned                             _audioSampleSize;
    unsigned                             _audioChannels;
};

class CPidVideo : public CPid
{
public:
    CPidVideo(
            unsigned         pid,
            NEXUS_VideoCodec videoCodec
            ) :
        CPid(pid, ePidType_Video),
        _codec(videoCodec) {}

    NEXUS_VideoCodec getCodec() { return(_codec); }

    eRet readXML(MXmlElement * xmlElemPid);
    void writeXML(MXmlElement * xmlElemChannel);

protected:
    NEXUS_VideoCodec _codec;
};

class CPidAudio : public CPid
{
public:
    CPidAudio(
            unsigned         pid,
            NEXUS_AudioCodec audioCodec
            ) :
        CPid(pid, ePidType_Audio),
        _codec(audioCodec) {}

    NEXUS_AudioCodec getCodec() { return(_codec); }

    eRet readXML(MXmlElement * xmlElemPid);
    void writeXML(MXmlElement * xmlElemChannel);

protected:
    NEXUS_AudioCodec _codec;
};

class CPidPcr : public CPid
{
public:
    CPidPcr(unsigned pid) :
        CPid(pid, ePidType_Pcr) {}

    eRet readXML(MXmlElement * xmlElemPid);
    void writeXML(MXmlElement * xmlElemChannel);
};

class CPidAncillary : public CPid
{
public:
    CPidAncillary(unsigned pid) :
        CPid(pid, ePidType_Ancillary) {}

    eRet readXML(MXmlElement * xmlElemPid);
    void writeXML(MXmlElement * xmlElemChannel);
};

#ifdef __cplusplus
}
#endif

#endif /* PID_H__ */