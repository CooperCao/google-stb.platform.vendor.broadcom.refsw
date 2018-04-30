/***************************************************************************
*  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*  See ‘License-BroadcomSTB-CM-Software.txt’ for terms and conditions.
***************************************************************************/

#include "Debug.h"
#include "bfile_stdio.h"
#include "MediaProbe.h"

#include "bmedia_probe.h"
#include "bmkv_probe.h"
#include "bmp4_probe.h"
#include "bmp3_probe.h"
#if B_HAS_ASF
#include "basf_probe.h"
#endif
#if B_HAS_AVI
#include "bavi_probe.h"
#endif

enum {
    bmp3_id3_none,
    bmp3_id3v1,
    bmp3_id3v2_2,
    bmp3_id3v2_3,
    bmp3_id3v2_4
};

namespace Broadcom
{
namespace Media
{

TRLS_DBG_MODULE(MediaProbe);

typedef struct {
    bvideo_codec settop;
    IMedia::VideoCodec imedia;
} VideoCodecEntry;

VideoCodecEntry videoCodecMap[] = {
    {bvideo_codec_none, IMedia::UnknownVideoCodec},
    {bvideo_codec_unknown, IMedia::UnknownVideoCodec},
    {bvideo_codec_mpeg1, IMedia::Mpeg1VideoCodec},
    {bvideo_codec_mpeg2, IMedia::Mpeg2VideoCodec},
    {bvideo_codec_mpeg4_part2, IMedia::Mpeg4Part2VideoCodec},
    {bvideo_codec_h263, IMedia::H263VideoCodec},
    {bvideo_codec_h264, IMedia::H264VideoCodec},
    {bvideo_codec_h264_svc, IMedia::H264SvcVideoCodec},
    {bvideo_codec_h264_mvc, IMedia::H264MvcVideoCodec},
    {bvideo_codec_vc1, IMedia::Vc1VideoCodec},
    {bvideo_codec_vc1_sm, IMedia::Vc1SimpleMainVideoCodec},
    {bvideo_codec_divx_311, IMedia::Divx311VideoCodec},
    {bvideo_codec_avs, IMedia::AvsVideoCodec},
    {bvideo_codec_rv40, IMedia::Rv40VideoCodec},
    {bvideo_codec_vp6, IMedia::Vp6VideoCodec},
    {bvideo_codec_vp8, IMedia::Vp8VideoCodec},
    {bvideo_codec_spark, IMedia::SparkVideoCodec},
    {bvideo_codec_mjpeg, IMedia::MotionJpeg},
    {bvideo_codec_h265, IMedia::H265VideoCodec}
};

typedef struct {
    baudio_format settop;
    IMedia::AudioCodec imedia;
} AudioCodecEntry;

AudioCodecEntry audioCodecMap[] = {
    {baudio_format_unknown, IMedia::UnknownAudioCodec},
    {baudio_format_mpeg, IMedia::MpegAudioCodec},
    {baudio_format_mp3, IMedia::Mp3AudioCodec},
    {baudio_format_aac, IMedia::AacAdtsAudioCodec},
    {baudio_format_aac, IMedia::AacLoasAudioCodec},
    {baudio_format_aac_plus_loas, IMedia::AacPlusLoasAudioCodec},
    {baudio_format_aac_plus_adts, IMedia::AacPlusAdtsAudioCodec},
    {baudio_format_ac3, IMedia::Ac3AudioCodec},
    {baudio_format_ac3_plus, IMedia::Ac3PlusAudioCodec},
    {baudio_format_dts, IMedia::DtsAudioCodec},
    {baudio_format_lpcm_dvd, IMedia::LpcmHdDvdAudioCodec},
    {baudio_format_lpcm_bluray, IMedia::LpcmBluRayAudioCodec},
    {baudio_format_dts_hd, IMedia::DtsHdAudioCodec},
    {baudio_format_wma_std, IMedia::WmaStdAudioCodec},
    {baudio_format_wma_pro, IMedia::WmaProAudioCodec},
    {baudio_format_lpcm_dvd, IMedia::LpcmDvdAudioCodec},
    {baudio_format_avs, IMedia::AvsAudioCodec},
    {baudio_format_amr, IMedia::AmrAudioCodec},
    {baudio_format_dra, IMedia::DraAudioCodec},
    {baudio_format_cook, IMedia::CookAudioCodec},
    {baudio_format_pcm,  IMedia::PcmAudioCodec},
    {baudio_format_flac, IMedia::FlacAudioCodec},
    {baudio_format_adpcm, IMedia::AdpcmAudioCodec},
    {baudio_format_dvi_adpcm, IMedia::DviAdpcmAudioCodec},
    {baudio_format_vorbis, IMedia::VorbisAudioCodec},
    {baudio_format_dts_lbr, IMedia::DtsExpressAudioCodec},
    {baudio_format_g711, IMedia::G711AudioCodec},
    {baudio_format_g726, IMedia::G726AudioCodec},
    {baudio_format_mlp, IMedia::MlpAudioCodec},
    {baudio_format_ape, IMedia::ApeAudioCodec},
    {baudio_format_lpcm_1394, IMedia::Lpcm1394AudioCodec},
};

typedef struct {
    bstream_mpeg_type settop;
    IMedia::StreamType imedia;
} StreamTypeEntry;

StreamTypeEntry streamTypeMap[] = {
    {bstream_mpeg_type_unknown, IMedia::UnknownStreamType},
    {bstream_mpeg_type_es, IMedia::EsStreamType},
    {bstream_mpeg_type_pes, IMedia::Mpeg2PesStreamType},
    {bstream_mpeg_type_ts, IMedia::TsStreamType},
    {bstream_mpeg_type_dss_es, IMedia::DssEsStreamType},
    {bstream_mpeg_type_dss_pes, IMedia::DssPesStreamType},
    {bstream_mpeg_type_vob, IMedia::VobStreamType},
    {bstream_mpeg_type_asf, IMedia::AsfStreamType},
    {bstream_mpeg_type_avi, IMedia::AviStreamType},
    {bstream_mpeg_type_mpeg1, IMedia::Mpeg1PsStreamType},
    {bstream_mpeg_type_mp4, IMedia::Mp4StreamType},
    {bstream_mpeg_type_mkv, IMedia::MkvStreamType},
    {bstream_mpeg_type_wav, IMedia::WavStreamType},
    {bstream_mpeg_type_mp4_fragment, IMedia::Mp4FragmentStreamType},
    {bstream_mpeg_type_rmff, IMedia::RmffStreamType},
    {bstream_mpeg_type_flv, IMedia::FlvStreamType},
    {bstream_mpeg_type_ogg, IMedia::OggStreamType}
};

typedef struct {
    bmedia_track_type settop;
    IMedia::TrackType imedia;
} TrackTypeEntry;

TrackTypeEntry trackTypeMap[] = {
    {bmedia_track_type_video, IMedia::VideoTrackType},
    {bmedia_track_type_audio, IMedia::AudioTrackType},
    {bmedia_track_type_pcr, IMedia::PcrTrackType},
    {bmedia_track_type_other, IMedia::OtherTrackType}
};

#define CONVERT(g_struct) \
    unsigned i; \
    for (i = 0; i < sizeof(g_struct)/sizeof(g_struct[0]); i++) { \
        if (g_struct[i].settop == media_value) { \
            return g_struct[i].imedia; \
        } \
    } \
    printf("unable to find value %d in %s\n", media_value, #g_struct); \
    return g_struct[0].imedia

IMedia::VideoCodec convertVideoCodec(const bvideo_codec& media_value)
{
    CONVERT(videoCodecMap);
}

IMedia::AudioCodec convertAudioCodec(const baudio_format& media_value)
{
    CONVERT(audioCodecMap);
}

IMedia::StreamType convertStreamType(const bstream_mpeg_type& media_value)
{
    CONVERT(streamTypeMap);
}

IMedia::TrackType convertTrackType(const bmedia_track_type& media_value)
{
    CONVERT(trackTypeMap);
}


char *trimString(char *str, int length)
{
    while (str != NULL && length > 0) {
        if (*str != '\0') break;
        str++;
        length--;
    }
    return str;
}

MediaTrack::MediaTrack(uint32_t id, uint32_t type)
{
    setId(id);
    setType((bmedia_track_type)type);
}

MediaTrack::~MediaTrack()
{
}

void MediaTrack::setId(uint32_t id)
{
    _id = id;
}

uint32_t MediaTrack::getId()
{
    return _id;
}

void MediaTrack::setType(uint32_t type)
{
    _type = convertTrackType((bmedia_track_type)type);
}

IMedia::TrackType MediaTrack::getType()
{
    return _type;
}

void MediaTrack::setCodec(uint32_t type)
{
    if (_type == IMedia::VideoTrackType) {
        _codec.video = convertVideoCodec((bvideo_codec)type);
    } else if (_type == IMedia::AudioTrackType) {
        _codec.audio = convertAudioCodec((baudio_format)type);
    }
}

IMedia::VideoCodec MediaTrack::getVideoCodec()
{
    if (_type == IMedia::VideoTrackType)
        return _codec.video;
    return IMedia::UnknownVideoCodec;
}

IMedia::AudioCodec MediaTrack::getAudioCodec()
{
    if (_type == IMedia::AudioTrackType)
        return _codec.audio;
    return IMedia::UnknownAudioCodec;
}

void MediaTrack::setWidth(uint32_t width)
{
    _width = width;
}

uint32_t MediaTrack::getWidth()
{
    return _width;
}

void MediaTrack::setHeight(uint32_t height)
{
    _height = height;
}

uint32_t MediaTrack::getHeight()
{
    return _height;
}

void MediaTrack::setBitrate(uint32_t bitrate)
{
    _bitrate = bitrate;
}

uint32_t MediaTrack::getBitrate()
{
    return _bitrate;
}

void MediaTrack::setLanguage(const std::string &language)
{
    _language = language;
}

const std::string MediaTrack::getLanguage()
{
    return _language;
}

void MediaTrack::setCodecId(const std::string &codecId)
{
    _codecId= codecId;
}

const std::string MediaTrack::getCodecId()
{
    return _codecId;
}

MediaProgram::MediaProgram(uint32_t id)
{
    _id = id;
}

MediaProgram::~MediaProgram()
{
}

void MediaProgram::setId(uint32_t id)
{
    _id = id;
}

uint32_t MediaProgram::getId()
{
    return _id;
}

const MediaTrackList MediaProgram::getTrackList()
{
    return _trackList;
}

void MediaProgram::addTrack(const MediaTrack track)
{
    _trackList.push_back(track);
}

uint32_t MediaProgram::getTotalTracks()
{
    return _trackList.size();
}

MediaProbe::MediaProbe()
{
    _probe = NULL;
}

MediaProbe::~MediaProbe()
{
    uninit();
}

void MediaProbe::initialize(void *fin)
{
    BME_DEBUG_ENTER();

    //B_Os_Init();
    if (!fin) {
        BME_DEBUG_ERROR(("Invalid file handle\n"));
        return;
    }

    _fd = bfile_stdio_read_attach((FILE *)fin);
    if (!_fd) {
        BME_DEBUG_ERROR(("can't attach bfile io\n"));
        return;
    }

    _probe = bmedia_probe_create();
    if (!_probe) {
        BME_DEBUG_ERROR(("can't create bmedia probe\n"));
        return;
    }

    //BME_DEBUG_EXIT();
}

void MediaProbe::uninit()
{
    BME_DEBUG_ENTER();

    if (_probe) {
        bmedia_probe_destroy((bmedia_probe *)_probe);
    }
    _probe = NULL;

    if (_fd) {
        bfile_stdio_read_detach((bfile_io_read *)_fd);
    }
    _fd = NULL;
    BME_DEBUG_EXIT();
}

bool MediaProbe::probe()
{
    bmedia_probe_config probe_config;
    const bmedia_probe_stream *stream = NULL;
    const bmedia_probe_track *track = NULL;
    bool bNewProgram;

    BME_DEBUG_ENTER();
    bmedia_probe_default_cfg(&probe_config);
    probe_config.type = bstream_mpeg_type_unknown;
    stream = bmedia_probe_parse((bmedia_probe *)_probe, (bfile_io_read *)_fd, &probe_config);

    if (!stream) {
        BME_DEBUG_ERROR(("can't probe stream ..."));
        return false;
    }

    _nPrograms = stream->nprograms;
    _nTracks = stream->ntracks;
    _duration = stream->duration;
    _streamType = convertStreamType(stream->type);

    if ((stream->type == bstream_mpeg_type_es) && (stream->probe_id == BMP3_PROBE_ID)) {
        const bmp3_probe_stream *mp3 = (bmp3_probe_stream *)stream;

        _streamType = IMedia::Mp3StreamType;
        if (mp3->title.len)
            _title = trimString((char *)mp3->title.str, mp3->title.len);

        if (mp3->artist.len)
            _artist = trimString((char *)mp3->artist.str, mp3->artist.len);

        if (mp3->album.len)
            _album = trimString((char *)mp3->album.str, mp3->album.len);

        if (mp3->year.len)
            _year = trimString((char *)mp3->year.str, mp3->year.len);

        if (mp3->genre.len)
            _genre = trimString((char *)mp3->genre.str, mp3->genre.len);

        if (mp3->start_time.len)
            _start_time = trimString((char *)mp3->start_time.str, mp3->start_time.len);

        if (mp3->end_time.len)
            _end_time = trimString((char *)mp3->end_time.str, mp3->end_time.len);

        if (mp3->comment.len)
            _comment = trimString((char *)mp3->comment.str, mp3->comment.len);
    }

    for (track = BLST_SQ_FIRST(&stream->tracks); track; track = BLST_SQ_NEXT(track, link)) {
        char *language = NULL;
        char *codec_id = NULL;
        MediaTrack mTrack(track->number, track->type);

        switch (track->type) {
            case bmedia_track_type_audio:
                mTrack.setCodec(track->info.audio.codec);
                mTrack.setBitrate(track->info.audio.bitrate);
                break;
            case bmedia_track_type_video:
                mTrack.setCodec(track->info.video.codec);
                mTrack.setBitrate(track->info.video.bitrate);
                mTrack.setWidth(track->info.video.width);
                mTrack.setHeight(track->info.video.height);
                break;
            case bmedia_track_type_pcr:
                break;
            case bmedia_track_type_other:
                break;
        }

        if (stream->type == bstream_mpeg_type_mp4) {
            language = ((bmp4_probe_track*)track)->language;
        } else if (stream->type == bstream_mpeg_type_mkv) {
            language = ((bmkv_probe_track*)track)->language;
            codec_id = ((bmkv_probe_track*)track)->codec_id;
        }
#if B_HAS_AVI
        else if (stream->type == bstream_mpeg_type_avi) {
            language = ((bavi_probe_track*)track)->language;
        }
#endif

        if (language && *language != '\0') {
            mTrack.setLanguage(language);
        }
        if (codec_id && *codec_id != '\0') {
            mTrack.setCodecId(codec_id);
        }

        bNewProgram = true;
        for (unsigned i = 0; i < _programList.size(); i ++) {
            MediaProgram &program = _programList.at(i);
            if (program.getId() == track->program) {
                program.addTrack(mTrack);
                bNewProgram = false;
                break;
            }
        }

        if (bNewProgram) {
            MediaProgram newProg(track->program);
            newProg.addTrack(mTrack);
            _programList.push_back(newProg);
        }
    }
    BME_DEBUG_EXIT();

    return true;
}

IMedia::StreamType MediaProbe::getStreamType()
{
    return _streamType;
}

uint32_t MediaProbe::getTotalPrograms()
{
    return _nPrograms;
}

const MediaProgramList MediaProbe::getProgramList()
{
    return _programList;
}

/* For Mp3 Meta Data */
const std::string MediaProbe::getTitle()
{
    return _title;
}

const std::string MediaProbe::getArtist()
{
    return _artist;
}

const std::string MediaProbe::getAlbum()
{
    return _album;
}

const std::string MediaProbe::getYear()
{
    return _year;
}

const std::string MediaProbe::getGenre()
{
    return _genre;
}

const std::string MediaProbe::getStartTime()
{
    return _start_time;
}

const std::string MediaProbe::getEndTime()
{
    return _end_time;
}

const std::string MediaProbe::getComment()
{
    return _comment;
}
}  // namespace Browser
}  // namespace Broadcom
