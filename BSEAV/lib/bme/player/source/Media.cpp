/***************************************************************************
*  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*  See ‘License-BroadcomSTB-CM-Software.txt’ for terms and conditions.
***************************************************************************/
#include <stdint.h>
#include "Media.h"

namespace Broadcom
{
namespace Media
{

const std::string IMedia::FILE_URI_PREFIX = "file://";
const std::string IMedia::NETWORK_HTTP_URI_PREFIX = "http://";
const std::string IMedia::NETWORK_HTTPS_URI_PREFIX = "https://";
const std::string IMedia::NETWORK_UDP_URI_PREFIX = "udp://";
const std::string IMedia::NETWORK_RTP_URI_PREFIX = "rtp://";
const std::string IMedia::NETWORK_RTSP_URI_PREFIX = "rtsp://";
const std::string IMedia::TV_URI_PREFIX = "tv://";
const std::string IMedia::TIMESHIFT_URI_PREFIX = "timeshift://";
const std::string IMedia::PUSH_ES_URI_PREFIX = "push_es://";
const std::string IMedia::PUSH_YTMS_URI_PREFIX = "push_ytms://";
const std::string IMedia::PUSH_TS_URI_PREFIX = "push_ts://";
const std::string IMedia::HDMI_URI_PREFIX = "hdmi://";

IMedia::StreamMetadata::StreamMetadata()
{
    reset();
}

IMedia::StreamMetadata::~StreamMetadata()
{
    reset();
}

void IMedia::StreamMetadata::reset()
{
    streamType = AutoStreamType;
    encryptionType = NoEncryptionType;
    timeStampEnabled = false;
    parserBand = 0;
    duration = 0;
    videoParam.streamId = 0;
    videoParam.substreamId = 0;
    videoParam.videoCodec = IMedia::UnknownVideoCodec;
    videoParam.extraVideoCodec = IMedia::UnknownVideoCodec;
    videoParam.maxHeight = 0;
    videoParam.maxWidth = 0;
    videoParam.colorDepth = 8;

    audioParamList.clear();
    pmtParamList.clear();
}


void IMedia::StreamMetadata::operator=(const IMedia::StreamMetadata& md)
{
    streamType = md.streamType;
    encryptionType = md.encryptionType;
    videoParam = md.videoParam;
    parserBand = md.parserBand;
    duration = md.duration;
    timeStampEnabled = md.timeStampEnabled;

    audioParamList.clear();
    for (unsigned i = 0; i < md.audioParamList.size(); i ++) {
        audioParamList.push_back(md.audioParamList[i]);
    }

    pmtParamList.clear();
    for (unsigned i = 0; i < md.pmtParamList.size(); i++) {
        pmtParamList.push_back(md.pmtParamList[i]);
    }
}


int IMedia::StreamMetadata::getAudioParamIndex(uint16_t pid) const
{
    int pidIndex = -1;
    for (unsigned idx = 0; idx < audioParamList.size(); idx++) {
        if (audioParamList[idx].streamId == pid) {
            pidIndex = idx;
            break;
        }
    }
    return pidIndex;
}

}  // namespace Media
}  // namespace Broadcom
