/***************************************************************************
*  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*  See ‘License-BroadcomSTB-CM-Software.txt’ for terms and conditions.
***************************************************************************/
#include <gtest/gtest.h>
#include "MediaProbe.h"
#include <iostream>

const std::string mp3File = "/data/media/SomeoneLikeYou.mp3";
const std::string mp4File = "/data/media/Bourne_1080p.mp4";
const std::string TsFile = "/data/media/698_6900_EIT.ts";

namespace Broadcom
{
namespace Media
{
TRLS_DBG_MODULE(MediaProbeTest);

class MediaProbeTest
    : public ::testing::Test
{
public:
    MediaProbeTest():
        _mediaProbe(NULL) {
    }

    virtual void SetUp() {
        _mediaProbe = new MediaProbe();
        ASSERT_TRUE(_mediaProbe);
    }

    virtual void TearDown() {
        if (_mediaProbe)
            delete _mediaProbe;
        _mediaProbe = NULL;
    }

    void showTracks(MediaProgram *pProgram) {
        uint32_t i;
        ASSERT_TRUE(pProgram);

        MediaTrackList mTL = pProgram->getTrackList();

        for (i = 0; i < mTL.size(); i ++) {
            MediaTrack track = mTL.at(i);

            if (track.getType() == IMedia::VideoTrackType) {
                BME_DEBUG_TRACE(("    Video Track: [ID:0x%x] [Codec:%d] [Language:%s] [CodecID:%s] [W, H]=[%d, %d] \n",
                        track.getId(), track.getVideoCodec(),
                        track.getLanguage().c_str(), track.getCodecId().c_str(),
                        track.getWidth(), track.getHeight()));
            } else if (track.getType() == IMedia::AudioTrackType) {
                BME_DEBUG_TRACE(("    Audio Track: [ID:0x%x] [Codec:%d] [Language:%s] [CodecID:%s]\n",
                        track.getId(), track.getAudioCodec(),
                        track.getLanguage().c_str(), track.getCodecId().c_str()));
            } else if (track.getType() == IMedia::PcrTrackType) {
                BME_DEBUG_TRACE(("    PCR Track: [ID:0x%x]\n", track.getId()));
            }
        }
    }

    void showPrograms() {
        uint32_t i;

        ASSERT_TRUE(_mediaProbe);

        MediaProgramList mPL = _mediaProbe->getProgramList();

        for (i = 0; i < mPL.size(); i ++) {
            MediaProgram program = mPL.at(i);
            BME_DEBUG_TRACE(("  Program ID:0x%x\n", program.getId()));
            showTracks(&program);
        }
    }

    void streamProbe(std::string url) {
        fIn = fopen64(url.c_str(), "rb");
        ASSERT_TRUE(fIn);

        _mediaProbe->initialize((void *)fIn);
        ASSERT_TRUE(_mediaProbe->probe());

        BME_DEBUG_TRACE(("Stream Type:%d #Programs:%d\n", _mediaProbe->getStreamType(), _mediaProbe->getTotalPrograms()));
        if (_mediaProbe->getStreamType() == IMedia::Mp3StreamType) {
            BME_DEBUG_TRACE(("MP3 Stream: [Artist:%s] [Title:%s] [Album:%s]"
                        "[Year:%s] [Genre:%s] [Start:%s] [End:%s] [Comment:%s]\n",
                _mediaProbe->getArtist().c_str(),
                _mediaProbe->getTitle().c_str(),
                _mediaProbe->getAlbum().c_str(),
                _mediaProbe->getYear().c_str(),
                _mediaProbe->getGenre().c_str(),
                _mediaProbe->getStartTime().c_str(),
                _mediaProbe->getEndTime().c_str(),
                _mediaProbe->getComment().c_str()));
        }

        showPrograms();
        fclose(fIn);
    }

private:
    MediaProbe* _mediaProbe;
    FILE *fIn;
};

TEST_F(MediaProbeTest, Mp3ProbeTest)
{
    streamProbe(mp3File);
}

TEST_F(MediaProbeTest, TsProbeTest)
{
    streamProbe(TsFile);
}

TEST_F(MediaProbeTest, Mp4ProbeTest)
{
    streamProbe(mp4File);
}
}  // namespace Media
}  // namespace Broadcom
