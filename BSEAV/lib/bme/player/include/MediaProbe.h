/***************************************************************************
*  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*  See ‘License-BroadcomSTB-CM-Software.txt’ for terms and conditions.
***************************************************************************/
#ifndef __IMEDIAPROBEIMPL_H__
#define __IMEDIAPROBEIMPL_H__

#include "Media.h"
#include "Debug.h"

#include <string>


namespace Broadcom
{
namespace Media
{

class BME_SO_EXPORT MediaTrack
{
private:
    uint32_t _id;
    IMedia::TrackType _type;
    union {
        IMedia::VideoCodec video;
        IMedia::AudioCodec audio;
    } _codec;

    uint32_t _bitrate;
    uint32_t _width;
    uint32_t _height;

    std::string _language;
    std::string _codecId;

public:
    MediaTrack(uint32_t id, uint32_t type);
    virtual ~MediaTrack();

    virtual void setType(uint32_t type);
    virtual IMedia::TrackType getType();

    virtual void setCodec(uint32_t type);
    virtual IMedia::VideoCodec getVideoCodec();
    virtual IMedia::AudioCodec getAudioCodec();

    virtual void setId(uint32_t id);
    virtual uint32_t getId();

    virtual void setBitrate(uint32_t bitrate);
    virtual uint32_t getBitrate();

    virtual void setWidth(uint32_t width);
    virtual uint32_t getWidth();

    virtual void setHeight(uint32_t height);
    virtual uint32_t getHeight();

    virtual void setLanguage(const std::string &language);
    virtual const std::string getLanguage();

    virtual void setCodecId(const std::string &codecId);
    virtual const std::string getCodecId();
};

typedef std::vector<MediaTrack> MediaTrackList;

class BME_SO_EXPORT MediaProgram
{
public:
    MediaProgram(uint32_t id);
    virtual ~MediaProgram();

private:
    uint32_t _id;
    MediaTrackList _trackList;
    uint32_t _tt;

public:
    virtual void setId(uint32_t id);
    virtual uint32_t getId();
    virtual void addTrack(const MediaTrack track);
    virtual const MediaTrackList getTrackList();
    virtual uint32_t getTotalTracks();
};

typedef std::vector<MediaProgram> MediaProgramList;

class BME_SO_EXPORT MediaProbe
{
private:
    void * _probe;
    void* _fd;

    IMedia::StreamType _streamType;
    uint32_t _duration;
    uint32_t _nPrograms;
    uint32_t _nTracks;
    uint32_t _maxBitrate;
    MediaProgramList _programList;

    /* Mp3 meta data */
    std::string _title;
    std::string _artist;
    std::string _album;
    std::string _year;
    std::string _genre;
    std::string _start_time;
    std::string _end_time;
    std::string _comment;

public:
    MediaProbe();
    virtual ~MediaProbe();

    virtual void initialize(void *fin);
    virtual bool probe();
    virtual void uninit();
    virtual IMedia::StreamType getStreamType();
    virtual uint32_t getTotalPrograms();
    virtual const MediaProgramList getProgramList();

    /* Mp3 Meta data */
    virtual const std::string getTitle();
    virtual const std::string getArtist();
    virtual const std::string getAlbum();
    virtual const std::string getYear();
    virtual const std::string getGenre();
    virtual const std::string getStartTime();
    virtual const std::string getEndTime();
    virtual const std::string getComment();
};

}  // namespace Browser
}  // namespace Broadcom
#endif  // __IMEDIAPROBEIMPL_H__
