/***************************************************************************
*  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*  See ‘License-BroadcomSTB-CM-Software.txt’ for terms and conditions.
***************************************************************************/
#include "BaseSource.h"
#include "stdio.h"
#include "Debug.h"

namespace Broadcom
{
namespace Media {

typedef struct {
    NEXUS_VideoCodec nexus;
    IMedia::VideoCodec settop;
} VideoCodecEntry;

VideoCodecEntry videoCodecs[] = {
    {NEXUS_VideoCodec_eUnknown, IMedia::UnknownVideoCodec},
    {NEXUS_VideoCodec_eUnknown, IMedia::UnknownVideoCodec},
    {NEXUS_VideoCodec_eMpeg1, IMedia::Mpeg1VideoCodec},
    {NEXUS_VideoCodec_eMpeg2, IMedia::Mpeg2VideoCodec},
    {NEXUS_VideoCodec_eMpeg4Part2, IMedia::Mpeg4Part2VideoCodec},
    {NEXUS_VideoCodec_eH263, IMedia::H263VideoCodec},
    {NEXUS_VideoCodec_eH264, IMedia::H264VideoCodec},
    {NEXUS_VideoCodec_eH264_Svc, IMedia::H264SvcVideoCodec},
    {NEXUS_VideoCodec_eH264_Mvc, IMedia::H264MvcVideoCodec},
    {NEXUS_VideoCodec_eVc1, IMedia::Vc1VideoCodec},
    {NEXUS_VideoCodec_eVc1SimpleMain, IMedia::Vc1SimpleMainVideoCodec},
    {NEXUS_VideoCodec_eDivx311, IMedia::Divx311VideoCodec},
    {NEXUS_VideoCodec_eAvs, IMedia::AvsVideoCodec},
    {NEXUS_VideoCodec_eRv40, IMedia::Rv40VideoCodec},
    {NEXUS_VideoCodec_eVp6, IMedia::Vp6VideoCodec},
    {NEXUS_VideoCodec_eVp7, IMedia::Vp7VideoCodec},
    {NEXUS_VideoCodec_eVp8, IMedia::Vp8VideoCodec},
    {NEXUS_VideoCodec_eVp9, IMedia::Vp9VideoCodec},
    {NEXUS_VideoCodec_eSpark, IMedia::SparkVideoCodec},
    {NEXUS_VideoCodec_eH265, IMedia::H265VideoCodec}
};

typedef struct {
    NEXUS_AudioCodec nexus;
    IMedia::AudioCodec settop;
} AudioCodecEntry;

AudioCodecEntry audioCodecs[] = {
    {NEXUS_AudioCodec_eUnknown, IMedia::UnknownAudioCodec},
    {NEXUS_AudioCodec_eMpeg, IMedia::MpegAudioCodec},
    {NEXUS_AudioCodec_eMp3, IMedia::Mp3AudioCodec},
    {NEXUS_AudioCodec_eAac, IMedia::AacAdtsAudioCodec},
    {NEXUS_AudioCodec_eAacLoas, IMedia::AacLoasAudioCodec},
    {NEXUS_AudioCodec_eAacPlusLoas, IMedia::AacPlusLoasAudioCodec},
    {NEXUS_AudioCodec_eAacPlusAdts, IMedia::AacPlusAdtsAudioCodec},
    {NEXUS_AudioCodec_eAc3, IMedia::Ac3AudioCodec},
    {NEXUS_AudioCodec_eAc3Plus, IMedia::Ac3PlusAudioCodec},
    {NEXUS_AudioCodec_eDts, IMedia::DtsAudioCodec},
    {NEXUS_AudioCodec_eLpcmHdDvd, IMedia::LpcmHdDvdAudioCodec},
    {NEXUS_AudioCodec_eLpcmBluRay, IMedia::LpcmBluRayAudioCodec},
    {NEXUS_AudioCodec_eDtsHd, IMedia::DtsHdAudioCodec},
    {NEXUS_AudioCodec_eWmaStd, IMedia::WmaStdAudioCodec},
    {NEXUS_AudioCodec_eWmaPro, IMedia::WmaProAudioCodec},
    {NEXUS_AudioCodec_eLpcmDvd, IMedia::LpcmDvdAudioCodec},
    {NEXUS_AudioCodec_eAvs, IMedia::AvsAudioCodec},
    {NEXUS_AudioCodec_eAmr, IMedia::AmrAudioCodec},
    {NEXUS_AudioCodec_eDra, IMedia::DraAudioCodec},
    {NEXUS_AudioCodec_eCook, IMedia::CookAudioCodec},
    {NEXUS_AudioCodec_ePcm,  IMedia::PcmAudioCodec},
    {NEXUS_AudioCodec_ePcmWav, IMedia::PcmWavAudioCodec},
    {NEXUS_AudioCodec_eFlac, IMedia::FlacAudioCodec},
    {NEXUS_AudioCodec_eAdpcm, IMedia::AdpcmAudioCodec},
    {NEXUS_AudioCodec_eAdpcm, IMedia::DviAdpcmAudioCodec},
    {NEXUS_AudioCodec_eVorbis, IMedia::VorbisAudioCodec},
    {NEXUS_AudioCodec_eOpus, IMedia::OpusAudioCodec}
};

typedef struct {
    NEXUS_TransportType nexus;
    IMedia::StreamType settop;
} MpegTypeEntry;

MpegTypeEntry mpegTypes[] = {
    {NEXUS_TransportType_eTs, IMedia::UnknownStreamType},
    {NEXUS_TransportType_eEs, IMedia::EsStreamType},
    {NEXUS_TransportType_eMpeg2Pes, IMedia::Mpeg2PesStreamType},
    {NEXUS_TransportType_eTs, IMedia::TsStreamType},
    {NEXUS_TransportType_eDssEs, IMedia::DssEsStreamType},
    {NEXUS_TransportType_eDssPes, IMedia::DssPesStreamType},
    {NEXUS_TransportType_eVob, IMedia::VobStreamType},
    {NEXUS_TransportType_eAsf, IMedia::AsfStreamType},
    {NEXUS_TransportType_eAvi, IMedia::AviStreamType},
    {NEXUS_TransportType_eMpeg1Ps, IMedia::Mpeg1PsStreamType},
    {NEXUS_TransportType_eMp4, IMedia::Mp4StreamType},
    {NEXUS_TransportType_eMkv, IMedia::MkvStreamType},
    {NEXUS_TransportType_eWav, IMedia::WavStreamType},
    {NEXUS_TransportType_eMp4Fragment, IMedia::Mp4FragmentStreamType},
    {NEXUS_TransportType_eRmff, IMedia::RmffStreamType},
    {NEXUS_TransportType_eFlv, IMedia::FlvStreamType},
    {NEXUS_TransportType_eOgg, IMedia::OggStreamType}
};

#define CONVERT(g_struct) \
    unsigned i; \
    for (i = 0; i < sizeof(g_struct)/sizeof(g_struct[0]); i++) { \
        if (g_struct[i].settop == media_value) { \
            return g_struct[i].nexus; \
        } \
    } \
    printf("unable to find value %d in %s\n", media_value, #g_struct); \
    return g_struct[0].nexus

BaseSource::BaseSource()
{
    _decoderType = IMedia::DecoderType::Main;
    _preferredAudioPid = 0;
}

#include <string.h>
bool BaseSource::support4Kp60()
{
#if((((BCHP_CHIP == 7445) || (BCHP_CHIP == 7252)) && (BCHP_VER == BCHP_VER_D0)) || \
    (BCHP_CHIP == 7366 && BCHP_VER == BCHP_VER_B0) \
    || (BCHP_CHIP == 7439) || (BCHP_CHIP == 7271))
    char const* boxMode = getenv("B_REFSW_BOXMODE");
    if (boxMode == NULL) {
        if (BCHP_CHIP == 7366)
            return false;
        else
            return true;
    } else {
        if (strncmp(boxMode, "3", 1) == 0) {
            printf("4K supported !!! Box Mode 3");
            return true;
        } else {
            return false;
        }
    }
    return true;
#else
    return false;
#endif
}
BaseSource::~BaseSource()
{
}

NEXUS_VideoCodec BaseSource::convertVideoCodec(const IMedia::VideoCodec& media_value)
{
    CONVERT(videoCodecs);
}

NEXUS_AudioCodec BaseSource::convertAudioCodec(const IMedia::AudioCodec& media_value)
{
    CONVERT(audioCodecs);
}

NEXUS_TransportType BaseSource::convertStreamType(const IMedia::StreamType& media_value)
{
    CONVERT(mpegTypes);
}

void BaseSource::setDecoderType(const  IMedia::DecoderType decoderType)
{
    _decoderType = decoderType;
}

}  // namespace Media
}  // namespace Broadcom
