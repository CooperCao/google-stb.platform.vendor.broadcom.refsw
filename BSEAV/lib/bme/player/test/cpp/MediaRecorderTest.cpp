/***************************************************************************
*  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*  See ‘License-BroadcomSTB-CM-Software.txt’ for terms and conditions.
***************************************************************************/
#include <gtest/gtest.h>
#include <sys/statvfs.h>
#include <string>
#include <fstream>
#include <json/value.h>
#include <json/writer.h>
#include <json/reader.h>
#include "nexus_platform.h"
#include "nexus_stc_channel.h"
#include "nexus_timebase.h"
#include "nexus_simple_video_decoder.h"
#include "nexus_simple_audio_decoder.h"
#include "nexus_types.h"
#include "nexus_core_utils.h"
#include "nexus_recpump.h"
#include "tv/include/TVManager.h"
#include "tv/include/TVTuner.h"
#include "tv/include/TVSource.h"
#include "tv/include/TVSourceListener.h"
#include "tv/include/TVChannel.h"
#include "tv/psikit/PsikitBridge.h"
#include "Media.h"
#include "MediaRecorder.h"
#include "MediaRecorderListener.h"
#include "RecorderUtil.h"
#include "player/include/TimeshiftSource.h"
#include "player/include/TsbConverter.h"
#include "player/include/SimpleDecoder.h"
#include "server/include/Transcoder.h"

using namespace std;
using namespace Broadcom::TV;

static Broadcom::TV::TVSourceType _sourceType = TVSOURCETYPE_ATSC;

namespace Broadcom
{
namespace Media
{

TRLS_DBG_MODULE(MediaRecorderTest);

class MyMediaRecorderListener :
    public MediaRecorderListener
{
    public:
        MyMediaRecorderListener() :
            _diskFull(false),
            _maxFileSizeReached(false)
        {
        }

        void init(MediaRecorder* mediaRecorder)
        {
            observe(mediaRecorder);
        }

        void onStarted()
        {
            printf("MyMediaRecorderListener::onStarted\n");
        }

        void onStopped()
        {
            printf("MyMediaRecorderListener::onStopped\n");
        }

        void onError(IMedia::ErrorType errorType)
        {
            printf("MyMediaRecorderListener::onError\n");
            if (errorType == IMedia::MEDIA_ERROR_RECORDER_DISK_FULL) {
                _diskFull = true;
            } else if (errorType == IMedia::MEDIA_ERROR_RECORDER_MAX_FILE_SIZE) {
                _maxFileSizeReached = true;
            }
        }

        bool isDiskFull()
        {
            return _diskFull;
        }

        bool isMaxFileReached()
        {
            return _maxFileSizeReached;
        }

    private:
        bool _diskFull;
        bool _maxFileSizeReached;
};

class MyTVSourceListener :
    public TVSourceListener
{
    public:
        MyTVSourceListener() :
            _channelChanged(false)
        {
        }

        virtual ~MyTVSourceListener() {}

        virtual void init(TVSource* source) {
            observe(source);
        }

        virtual void onCurrentChannelChanged(TVSource::TVCurrentChannelChangedEvent currentChannelChangedEvent)
        {
            _channelChanged = true;
        }

        virtual void onScanningStateChanged(TVSource::TVScanningStateChangedEvent scanningStateChangedEvent)
        {
            _scanningState = scanningStateChangedEvent.state;
        }

        bool channelChanged()
        {
            return _channelChanged;
        }

        TVScanningState getScanningState()
        {
            return _scanningState;
        }

        void reset()
        {
            _channelChanged = false;
            _scanningState = TVSCANNINGSTATE_CLEARED;
        }

    private:
        bool _channelChanged;
        TVScanningState _scanningState;
};

class MediaRecorderTest
    : public ::testing::Test
{
    public:
        MediaRecorderTest() {
        }

        virtual void SetUp() {
            _psi = new PsikitBridge();
            _tvManager = new TVManager(_psi);
            _tunerList = _tvManager->getTuners();
        }

        virtual void TearDown() {
            delete _tvManager;
            delete _psi;
        }

        PsikitBridge* _psi;
        TVManager* _tvManager;
        vector<TVTuner*> _tunerList;
};

class TimeshiftTest
    : public ::testing::Test
{
    public:
        TimeshiftTest() {
        }


        virtual void SetUp() {
            // _timeshiftSource = new TimeshiftSource(&_recorder, true);
            _psi = new PsikitBridge();
            _tvManager = new TVManager(_psi);
            _tunerList = _tvManager->getTuners();
        }

        virtual void TearDown() {
            // delete _recorder;
            // delete _timeshiftSource;
            delete _tvManager;
            delete _psi;
        }

        static void dataReadyCallback(void *context, int param)
        {
            TRLS_UNUSED(context);
            TRLS_UNUSED(param);
        }

        PsikitBridge* _psi;
        TVManager* _tvManager;
        vector<TVTuner*> _tunerList;
};

TEST(RecorderUtilTest, EmptyFilename)
{
    EXPECT_EQ(RecorderUtil::getRecordingSize(""), 0);
    EXPECT_EQ(RecorderUtil::getRecordingFiles("").size(), 0);
}

TEST(RecorderUtilTest, WhitespaceFilename)
{
    EXPECT_EQ(RecorderUtil::getRecordingSize(" "), 0);
    // Create file with empty filename
    FILE* f = fopen(("/data/record/.mpg"), "w");
    fclose(f);
    // Make sure ".mpg" file is not inadvertently removed
    RecorderUtil::removeRecording(" ");
    ifstream file("/data/record/.mpg");
    EXPECT_TRUE(file.good());
    EXPECT_EQ(RecorderUtil::getRecordingFiles(" ").size(), 0);
    // Cleanup
    remove("/data/record/.mpg");
}

TEST(RecorderUtilTest, VerifyFunctions)
{
    std::string filename = "/data/record/test_recording_chunked";
    std::string filename2 = "/data/record/test_recording_nonchunked";
    int size1 = 1024 * 1024;
    int size2 = 1024 * 1024;
    int size3 = 256 * 1024;
    int totalSize = size1 + size2 + size3;
    int totalSize2 = 1024 * 1024;

    // Create 3 chunks of known size
    FILE* f1 = fopen((filename + ".mpg_0000").c_str(), "w");
    ftruncate(fileno(f1), size1);
    fclose(f1);

    FILE* f2 = fopen((filename + ".mpg_0001").c_str(), "w");
    ftruncate(fileno(f2), size2);
    fclose(f2);

    FILE* f3 = fopen((filename + ".mpg_0002").c_str(), "w");
    ftruncate(fileno(f3), size3);
    fclose(f3);

    // Create dummy json file
    std::ofstream chunkedOutStream(filename + ".json");
    Json::Value chunkedRoot;
    Json::StyledWriter chunkedWriter;
    chunkedRoot["filename"] = filename + ".mpg";
    chunkedRoot["indexFilename"] = filename + ".nav";
    chunkedRoot["mode"] = static_cast<int>(RecordMode::ChunkedFile);
    chunkedRoot["chunkSize"] = size1;
    chunkedRoot["state"] = static_cast<int>(RecordState::Stopped);
    std::string chunkedOutput = chunkedWriter.write(chunkedRoot);
    chunkedOutStream << chunkedOutput.c_str();
    chunkedOutStream.close();

    // Create dummy nav file
    FILE* f5 = fopen((filename + ".nav").c_str(), "w");
    ftruncate(fileno(f5), 256);
    fclose(f5);

    // Create dummy non-chunked file
    FILE* f6 = fopen((filename2 + ".mpg").c_str(), "w");
    ftruncate(fileno(f6), totalSize2);
    fclose(f6);

    // Create dummy json file
    std::ofstream nonChunkedOutStream(filename2 + ".json");
    Json::Value nonChunkedRoot;
    Json::StyledWriter nonChunkedWriter;
    nonChunkedRoot["filename"] = filename2 + ".mpg";
    nonChunkedRoot["indexFilename"] = filename2 + ".nav";
    nonChunkedRoot["mode"] = static_cast<int>(RecordMode::Linear);
    nonChunkedRoot["state"] = static_cast<int>(RecordState::Stopped);
    std::string nonChunkedOutput = nonChunkedWriter.write(nonChunkedRoot);
    nonChunkedOutStream << nonChunkedOutput.c_str();
    nonChunkedOutStream.close();

    // Create dummy nav file
    FILE* f8 = fopen((filename2 + ".nav").c_str(), "w");
    ftruncate(fileno(f8), 256);
    fclose(f8);

    // Verify getRecordingSize
    ASSERT_EQ(RecorderUtil::getRecordingSize(filename + ".mpg"), totalSize);
    ASSERT_EQ(RecorderUtil::getRecordingSize(filename2 + ".mpg"), totalSize2);

    // Verify removeRecording
    RecorderUtil::removeRecording(filename + ".mpg");
    RecorderUtil::removeRecording(filename2 + ".mpg");
    ASSERT_EQ(RecorderUtil::getRecordingFiles(filename + ".mpg").size(), 0);
    ASSERT_EQ(RecorderUtil::getRecordingFiles(filename2 + ".mpg").size(), 0);
}

TEST_F(MediaRecorderTest, RecordSingleChannel)
{
    MyTVSourceListener listener;
    std::string filename = "/data/record/test_recording.mpg";
    TVTuner* tuner = _tunerList[0];
    tuner->setCurrentSource(_sourceType);
    TVSource* source = tuner->getCurrentSource();
    if (!source) {
        printf("Error: Current source not set or tuner does not support sourceType [%d].\n", _sourceType);
        return;
    }
    vector<TVChannel*> channelList = source->getChannels();
    if (channelList.size() < 2) {
        printf("Error: Insufficient number of channels found [%u]\n", channelList.size());
        return;
    }
    TVChannel* ch = channelList[0];   // select channel to be recorded
    listener.init(source);
    source->setCurrentChannel(ch->getNumber());

    MediaRecorderOptions recorderOptions;
    recorderOptions.mode = RecordMode::ChunkedFile;
    recorderOptions.chunkedFileRecordSettings.chunkSize = 1 * 1024 * 1024;
    recorderOptions.chunkedFileRecordSettings.chunkTemplate = "%s_%04u";

    MediaRecorder recorder;
    MediaStream* mediaStream = tuner->getMediaStream();
    recorderOptions.destFile = std::string("file://") + filename;
    recorderOptions.sourceMediaStream = mediaStream;
    recorder.setOptions(recorderOptions);

    // Wait for channel to be changed before starting record
    // so the parserBand is set up.
    while (!listener.channelChanged()) {
        sleep(1);
    }

    IMedia::ErrorType errorType = recorder.start();
    ASSERT_EQ(errorType, IMedia::MEDIA_SUCCESS);

    // Record for 10 seconds and then verify recording was created with size > 0
    sleep(10);
    recorder.stop();

    EXPECT_GT(RecorderUtil::getRecordingSize(filename), 0);

    // Remove recording
    RecorderUtil::removeRecording(filename);
    // Verify no associated recording files are found
    ASSERT_EQ(RecorderUtil::getRecordingFiles(filename).size(), 0);
}

TEST_F(MediaRecorderTest, DualRecordsSameChannelSimultaneously)
{
    IMedia::ErrorType errorType;
    MyTVSourceListener listener;
    std::string location1 = "/data/record/test_recording1.mpg";
    std::string location2 = "/data/record/test_recording2.mpg";
    TVTuner* tuner = _tunerList[0];
    tuner->setCurrentSource(_sourceType);
    TVSource* source = tuner->getCurrentSource();
    if (!source) {
        printf("Error: Current source not set or tuner does not support sourceType [%d].\n", _sourceType);
        return;
    }
    vector<TVChannel*> channelList = source->getChannels();
    if (channelList.size() < 2) {
        printf("Error: Insufficient number of channels found [%u]\n", channelList.size());
        return;
    }
    TVChannel* ch = channelList[0];   // select channel to be recorded
    listener.init(source);
    source->setCurrentChannel(ch->getNumber());

    MediaRecorderOptions recorderOptions;
    recorderOptions.mode = RecordMode::ChunkedFile;
    recorderOptions.chunkedFileRecordSettings.chunkSize = 1 * 1024 * 1024;
    recorderOptions.chunkedFileRecordSettings.chunkTemplate = "%s_%04u";

    MediaStream* mediaStream = tuner->getMediaStream();
    recorderOptions.sourceMediaStream = mediaStream;

    MediaRecorder recorder1;
    MediaRecorder recorder2;
    recorderOptions.destFile = std::string("file://") + location1;
    recorder1.setOptions(recorderOptions);
    recorderOptions.destFile = std::string("file://") + location2;
    recorder2.setOptions(recorderOptions);

    // Wait for channel to be changed before starting record
    // so the parserBand is set up.
    while (!listener.channelChanged()) {
        sleep(1);
    }

    errorType = recorder1.start();
    ASSERT_EQ(errorType, IMedia::MEDIA_SUCCESS);
    errorType = recorder2.start();
    ASSERT_EQ(errorType, IMedia::MEDIA_SUCCESS);

    // Record for 5 seconds
    sleep(5);
    recorder1.stop();
    recorder2.stop();

    // Verify recording was created with size > 0
    EXPECT_GT(RecorderUtil::getRecordingSize(location1), 0);
    EXPECT_GT(RecorderUtil::getRecordingSize(location2), 0);

    // Remove recording
    RecorderUtil::removeRecording(location1);
    RecorderUtil::removeRecording(location2);

    // Verify no associated recording files are found
    ASSERT_EQ(RecorderUtil::getRecordingFiles(location1).size(), 0);
    ASSERT_EQ(RecorderUtil::getRecordingFiles(location2).size(), 0);
}

TEST_F(MediaRecorderTest, DualRecordsDifferentChannelSimultaneously)
{
    if (_tunerList.size() > 1) {
        IMedia::ErrorType errorType;
        MyTVSourceListener listener1;
        MyTVSourceListener listener2;
        std::string location1 = "/data/record/test_recording1.mpg";
        std::string location2 = "/data/record/test_recording2.mpg";
        TVTuner* tuner1 = _tunerList[0];
        TVTuner* tuner2 = _tunerList[1];
        tuner1->setCurrentSource(_sourceType);
        tuner2->setCurrentSource(_sourceType);
        TVSource* source1 = tuner1->getCurrentSource();
        if (!source1) {
            printf("Error: Current source not set or tuner does not support sourceType [%d].\n", _sourceType);
            return;
        }
        TVSource* source2 = tuner2->getCurrentSource();
        if (!source2) {
            printf("Error: Current source not set or tuner does not support sourceType [%d].\n", _sourceType);
            return;
        }
        vector<TVChannel*> channelList = source1->getChannels();
        if (channelList.size() < 2) {
            printf("Error: Insufficient number of channels found [%u]\n", channelList.size());
            return;
        }
        TVChannel* ch1 = channelList[0];   // select channel to be recorded
        TVChannel* ch2 = channelList[1];   // select channel to be recorded
        listener1.init(source1);
        listener2.init(source2);
        source1->setCurrentChannel(ch1->getNumber());
        source2->setCurrentChannel(ch2->getNumber());

        MediaRecorderOptions recorderOptions;
        recorderOptions.mode = RecordMode::ChunkedFile;
        recorderOptions.chunkedFileRecordSettings.chunkSize = 1 * 1024 * 1024;
        recorderOptions.chunkedFileRecordSettings.chunkTemplate = "%s_%04u";

        MediaRecorder recorder1;
        MediaRecorder recorder2;
        MediaStream* mediaStream1 = tuner1->getMediaStream();
        MediaStream* mediaStream2 = tuner2->getMediaStream();
        recorderOptions.sourceMediaStream = mediaStream1;
        recorderOptions.destFile = std::string("file://") + location1;
        recorder1.setOptions(recorderOptions);
        recorderOptions.sourceMediaStream = mediaStream2;
        recorderOptions.destFile = std::string("file://") + location2;
        recorder2.setOptions(recorderOptions);

        // Wait for channel to be changed before starting record
        // so the parserBand is set up.
        while (!listener1.channelChanged()) {
            sleep(1);
        }
        while (!listener2.channelChanged()) {
            sleep(1);
        }

        errorType = recorder1.start();
        ASSERT_EQ(errorType, IMedia::MEDIA_SUCCESS);
        errorType = recorder2.start();
        ASSERT_EQ(errorType, IMedia::MEDIA_SUCCESS);

        // Record for 5 seconds
        sleep(5);
        recorder1.stop();
        recorder2.stop();

        // Verify recording was created with size > 0
        EXPECT_GT(RecorderUtil::getRecordingSize(location1), 0);
        EXPECT_GT(RecorderUtil::getRecordingSize(location2), 0);

        // Remove recording
        RecorderUtil::removeRecording(location1);
        RecorderUtil::removeRecording(location2);

        // Verify no associated recording files are found
        ASSERT_EQ(RecorderUtil::getRecordingFiles(location1).size(), 0);
        ASSERT_EQ(RecorderUtil::getRecordingFiles(location2).size(), 0);
    }
}

TEST_F(MediaRecorderTest, RecordToDifferentLocations)
{
    IMedia::ErrorType errorType;
    MyTVSourceListener listener;
    std::string location1 = "/data/record/test_recording1.mpg";
    std::string location2 = "/data/test_recording2.mpg";
    TVTuner* tuner = _tunerList[0];
    tuner->setCurrentSource(_sourceType);
    TVSource* source = tuner->getCurrentSource();
    if (!source) {
        printf("Error: Current source not set or tuner does not support sourceType [%d].\n", _sourceType);
        return;
    }
    vector<TVChannel*> channelList = source->getChannels();
    if (channelList.size() < 2) {
        printf("Error: Insufficient number of channels found [%u]\n", channelList.size());
        return;
    }
    TVChannel* ch = channelList[0];   // select channel to be recorded
    listener.init(source);
    source->setCurrentChannel(ch->getNumber());

    MediaRecorderOptions recorderOptions;
    recorderOptions.mode = RecordMode::ChunkedFile;
    recorderOptions.chunkedFileRecordSettings.chunkSize = 1 * 1024 * 1024;
    recorderOptions.chunkedFileRecordSettings.chunkTemplate = "%s_%04u";

    MediaRecorder recorder;
    MediaStream* mediaStream = tuner->getMediaStream();
    recorderOptions.sourceMediaStream = mediaStream;
    recorderOptions.destFile = std::string("file://") + location1;
    recorder.setOptions(recorderOptions);

    // Wait for channel to be changed before starting record
    // so the parserBand is set up.
    while (!listener.channelChanged()) {
        sleep(1);
    }

    errorType = recorder.start();
    ASSERT_EQ(errorType, IMedia::MEDIA_SUCCESS);

    // Record for 5 seconds
    sleep(5);
    recorder.stop();

    // Set the second location to record
    recorderOptions.destFile = std::string("file://") + location2;
    recorder.setOptions(recorderOptions);

    errorType = recorder.start();
    ASSERT_EQ(errorType, IMedia::MEDIA_SUCCESS);

    // Record for 5 seconds
    sleep(5);
    recorder.stop();

    // Verify recording was created with size > 0
    EXPECT_GT(RecorderUtil::getRecordingSize(location1), 0);
    EXPECT_GT(RecorderUtil::getRecordingSize(location2), 0);

    // Remove recording
    RecorderUtil::removeRecording(location1);
    RecorderUtil::removeRecording(location2);

    // Verify no associated recording files are found
    ASSERT_EQ(RecorderUtil::getRecordingFiles(location1).size(), 0);
    ASSERT_EQ(RecorderUtil::getRecordingFiles(location2).size(), 0);
}

// This test relies on creating a ramdisk of small
// size to simulate disk full.
// i.e.
// $ mkdir /mnt/ramdisk
// $ mount -t tmpfs -o size=1m tmpfs /mnt/ramdisk
TEST_F(MediaRecorderTest, DISABLED_DiskFull)
{
    struct statvfs stats;
    MyTVSourceListener listener;
    std::string recordDir = "/mnt/ramdisk/";
    std::string filename = recordDir + "test_recording.mpg";
    TVTuner* tuner = _tunerList[0];
    tuner->setCurrentSource(_sourceType);
    TVSource* source = tuner->getCurrentSource();
    if (!source) {
        printf("Error: Current source not set or tuner does not support sourceType [%d].\n", _sourceType);
        return;
    }
    vector<TVChannel*> channelList = source->getChannels();
    if (channelList.size() < 2) {
        printf("Error: Insufficient number of channels found [%u]\n", channelList.size());
        return;
    }
    TVChannel* ch = channelList[0];   // select channel to be recorded
    listener.init(source);
    source->setCurrentChannel(ch->getNumber());

    MediaRecorderOptions recorderOptions;
    recorderOptions.mode = RecordMode::ChunkedFile;
    recorderOptions.chunkedFileRecordSettings.chunkSize = 1 * 1024 * 1024;
    recorderOptions.chunkedFileRecordSettings.chunkTemplate = "%s_%04u";

    MediaStream* mediaStream = tuner->getMediaStream();
    recorderOptions.destFile = std::string("file://") + filename;
    recorderOptions.sourceMediaStream = mediaStream;

    MediaRecorder recorder;
    MyMediaRecorderListener recorderListener;
    recorderListener.init(&recorder);
    recorder.setOptions(recorderOptions);

    // Wait for channel to be changed before starting record
    // so the parserBand is set up.
    while (!listener.channelChanged()) {
        sleep(1);
    }

    // Verify there's some free space to record
    statvfs(recordDir.c_str(), &stats);
    ASSERT_GT(stats.f_bfree, 0);

    // Record to location with little free space left
    IMedia::ErrorType errorType = recorder.start();
    ASSERT_EQ(errorType, IMedia::MEDIA_SUCCESS);

    // Record until disk is full and verify we get disk full error
    // within 10 sec
    int timeout = 0;
    while (!recorderListener.isDiskFull() && timeout < 10) {
        sleep(1);
        timeout++;
    }
    recorder.stop();
    EXPECT_LT(timeout, 10);

    // Verify available disk space is 0
    statvfs(recordDir.c_str(), &stats);
    EXPECT_EQ(stats.f_bfree, 0);

    // Remove recording
    RecorderUtil::removeRecording(filename);
    // Verify no associated recording files are found
    ASSERT_EQ(RecorderUtil::getRecordingFiles(filename).size(), 0);
}

TEST_F(MediaRecorderTest, UnexpectedDisconnection)
{
    MyTVSourceListener listener;
    std::string filenameNoExt = "/data/record/test_recording";
    std::string filename = filenameNoExt + ".mpg";
    std::string filename2 = filenameNoExt + "2.mpg";
    TVTuner* tuner = _tunerList[0];
    tuner->setCurrentSource(_sourceType);
    TVSource* source = tuner->getCurrentSource();
    if (!source) {
        printf("Error: Current source not set or tuner does not support sourceType [%d].\n", _sourceType);
        return;
    }
    vector<TVChannel*> channelList = source->getChannels();
    if (channelList.size() < 2) {
        printf("Error: Insufficient number of channels found [%u]\n", channelList.size());
        return;
    }
    TVChannel* ch = channelList[0];   // select channel to be recorded
    listener.init(source);
    source->setCurrentChannel(ch->getNumber());

    MediaRecorderOptions recorderOptions;
    recorderOptions.mode = RecordMode::ChunkedFile;
    recorderOptions.chunkedFileRecordSettings.chunkSize = 1 * 1024 * 1024;
    recorderOptions.chunkedFileRecordSettings.chunkTemplate = "%s_%04u";

    MediaStream* mediaStream = tuner->getMediaStream();
    recorderOptions.sourceMediaStream = mediaStream;
    recorderOptions.destFile = std::string("file://") + filename;

    MediaRecorder* recorder = new MediaRecorder();
    recorder->setOptions(recorderOptions);

    // Wait for channel to be changed before starting record
    // so the parserBand is set up.
    while (!listener.channelChanged()) {
        sleep(1);
    }

    IMedia::ErrorType errorType = recorder->start();
    ASSERT_EQ(errorType, IMedia::MEDIA_SUCCESS);

    // Record for 5 seconds
    sleep(5);

    // Delete recorder to simulate "pulling the plug"
    delete recorder;

    // Verify recording exists
    EXPECT_GT(RecorderUtil::getRecordingSize(filename), 0);

    // Check metadata exists and reports that the recording "Started"
    std::ifstream inputStream(filenameNoExt + ".json");
    ASSERT_TRUE(inputStream.good());
    std::stringstream strStream;
    Json::Value root;
    Json::Reader reader;

    strStream << inputStream.rdbuf();
    bool result = reader.parse(strStream.str(), root);
    ASSERT_TRUE(result);
    RecordState recordState = static_cast<RecordState>(root["state"].asInt());
    ASSERT_EQ(recordState, RecordState::Started);

    // Recover from unexpected disconnection and continue recording
    recorderOptions.destFile = std::string("file://") + filename2;
    recorderOptions.sourceMediaStream = mediaStream;
    MediaRecorder recorder2;
    recorder2.setOptions(recorderOptions);

    errorType = recorder2.start();
    ASSERT_EQ(errorType, IMedia::MEDIA_SUCCESS);

    // Record for 5 seconds
    sleep(5);
    recorder2.stop();

    // Verify recording exists
    EXPECT_GT(RecorderUtil::getRecordingSize(filename2), 0);

    // Verify recording completed
    std::ifstream inputStream2(filenameNoExt + "2.json");
    ASSERT_TRUE(inputStream2.good());
    std::stringstream strStream2;
    Json::Value root2;
    Json::Reader reader2;

    strStream2 << inputStream2.rdbuf();
    bool result2 = reader2.parse(strStream2.str(), root2);
    ASSERT_TRUE(result2);
    RecordState recordState2 = static_cast<RecordState>(root2["state"].asInt());
    ASSERT_EQ(recordState2, RecordState::Stopped);

    // Remove recording
    RecorderUtil::removeRecording(filename);
    RecorderUtil::removeRecording(filename2);
    // Verify no associated recording files are found
    ASSERT_EQ(RecorderUtil::getRecordingFiles(filename).size(), 0);
    ASSERT_EQ(RecorderUtil::getRecordingFiles(filename2).size(), 0);
}

TEST_F(MediaRecorderTest, DISABLED_ThumbnailGeneration)
{
    MyTVSourceListener listener;
    std::string filename = "/data/record/test_recording.mpg";
    std::string thumbnailFilename = "test_thumbnail.jpg";
    TVTuner* tuner = _tunerList[0];
    tuner->setCurrentSource(_sourceType);
    TVSource* source = tuner->getCurrentSource();
    if (!source) {
        printf("Error: Current source not set or tuner does not support sourceType [%d].\n", _sourceType);
        return;
    }
    vector<TVChannel*> channelList = source->getChannels();
    if (channelList.size() < 2) {
        printf("Error: Insufficient number of channels found [%u]\n", channelList.size());
        return;
    }
    TVChannel* ch = channelList[0];   // select channel to be recorded
    listener.init(source);
    source->setCurrentChannel(ch->getNumber());

    MediaStream* mediaStream = tuner->getMediaStream();

    MediaRecorderOptions recorderOptions;
    recorderOptions.mode = RecordMode::ChunkedFile;
    recorderOptions.chunkedFileRecordSettings.chunkSize = 1 * 1024 * 1024;
    recorderOptions.chunkedFileRecordSettings.chunkTemplate = "%s_%04u";
    recorderOptions.thumbnailOptions.enabled = true;
    recorderOptions.thumbnailOptions.outputFilePath = "/data/record/";
    recorderOptions.thumbnailOptions.outputFileName = thumbnailFilename;
    recorderOptions.thumbnailOptions.videoPid = mediaStream->metadata.videoParam.streamId;
    recorderOptions.thumbnailOptions.repeatEvery = 1000;

    MediaRecorder recorder;
    recorderOptions.sourceMediaStream = mediaStream;
    recorderOptions.destFile = std::string("file://") + filename;
    recorder.setOptions(recorderOptions);

    // Wait for channel to be changed before starting record
    // so the parserBand is set up.
    while (!listener.channelChanged()) {
        sleep(1);
    }

    IMedia::ErrorType errorType = recorder.start();
    ASSERT_EQ(errorType, IMedia::MEDIA_SUCCESS);

    // Record for 10 seconds and then verify thumbnails exist
    sleep(10);
    recorder.stop();

    EXPECT_GT(RecorderUtil::getRecordingSize(filename), 0);

    // Clean up
    // Remove recording
    RecorderUtil::removeRecording(filename);
    // Remove thumbnails
    RecorderUtil::removeThumbnails(filename);
    ASSERT_EQ(RecorderUtil::getThumbnailFiles(filename).size(), 0);
}

TEST_F(MediaRecorderTest, ThumbnailGenerationAllTuners)
{
    int numOfTuners = _tunerList.size();
    std::vector<MyTVSourceListener*> listeners;
    std::string filenameNoExt = "/data/record/test_recording";
    std::string thumbnailFilenameNoExt = "test_thumbnail";
    std::vector<TVTuner*> tuners;
    std::vector<TVSource*> sources;
    std::vector<MediaRecorder*> recorders;
    int i = 0;

    MediaRecorderOptions recorderOptions;
    recorderOptions.mode = RecordMode::ChunkedFile;
    recorderOptions.chunkedFileRecordSettings.chunkSize = 1 * 1024 * 1024;
    recorderOptions.chunkedFileRecordSettings.chunkTemplate = "%s_%04u";
    recorderOptions.thumbnailOptions.enabled = true;
    recorderOptions.thumbnailOptions.outputFilePath = "/data/record/";
    recorderOptions.thumbnailOptions.repeatEvery = 2000;

    // Start a record session on each tuner
    for (i = 0; i < numOfTuners; i++) {
        listeners.push_back(new MyTVSourceListener());
        tuners.push_back(_tunerList[i]);
        tuners[i]->setCurrentSource(_sourceType);
        sources.push_back(tuners[i]->getCurrentSource());
        if (!sources[i]) {
            printf("Error: Current source not set or tuner does not support sourceType [%d].\n", _sourceType);
            return;
        }
        vector<TVChannel*> channelList = sources[i]->getChannels();
        if (channelList.size() < 2) {
            printf("Error: Insufficient number of channels found [%u]\n", channelList.size());
            return;
        }
        TVChannel* ch = channelList[i % channelList.size()];   // select channel to be recorded
        listeners[i]->init(sources[i]);
        sources[i]->setCurrentChannel(ch->getNumber());

        MediaStream* mediaStream = tuners[i]->getMediaStream();

        recorderOptions.thumbnailOptions.mediaStream = mediaStream;
        recorderOptions.thumbnailOptions.outputFileName = thumbnailFilenameNoExt +
                                                          std::to_string(i) + ".jpg";
        recorderOptions.thumbnailOptions.videoPid = mediaStream->metadata.videoParam.streamId;

        recorders.push_back(new MediaRecorder());

        recorderOptions.destFile = std::string("file://") + filenameNoExt +
                                   std::to_string(i) + ".mpg";
        recorderOptions.sourceMediaStream = mediaStream;
        recorders[i]->setOptions(recorderOptions);

        // Wait for channel to be changed before starting record
        // so the parserBand is set up.
        while (!listeners[i]->channelChanged()) {
            sleep(1);
        }
    }

    for (i = 0; i < 3; i++) {
        printf("\n******* ITERATION %d **********\n", i);
        for (int j = 0; j < numOfTuners; j++) {
            IMedia::ErrorType errorType = recorders[j]->start();
            ASSERT_EQ(errorType, IMedia::MEDIA_SUCCESS);
        }
            // Record for 5 seconds and then verify thumbnails exist
            sleep(5);
        for (int j = 0; j < numOfTuners; j++) {
            printf("Stopping %d\n", j);
            recorders[j]->stop();

            EXPECT_GT(RecorderUtil::getRecordingSize(filenameNoExt + std::to_string(j) + ".mpg"), 0);

            // Clean up
            // Remove recording
            RecorderUtil::removeRecording(filenameNoExt + std::to_string(j) + ".mpg");
            // Remove thumbnails
            // RecorderUtil::removeThumbnails(filenameNoExt + std::to_string(i) + ".mpg");
            // ASSERT_EQ(RecorderUtil::getThumbnailFiles(filenameNoExt + std::to_string(i) + ".mpg").size(), 0);
        }
    }
}

TEST_F(MediaRecorderTest, BasicConversionToLinear)
{
    MyTVSourceListener listener;
    std::string filename_circular = "/data/record/test_recording_circular.mpg";
    std::string filename_linear = "/data/record/test_recording_linear.mpg";
    TVTuner* tuner = _tunerList[0];
    tuner->setCurrentSource(_sourceType);
    TVSource* source = tuner->getCurrentSource();
    if (!source) {
        printf("Error: Current source not set or tuner does not support sourceType [%d].\n", _sourceType);
        return;
    }
    vector<TVChannel*> channelList = source->getChannels();
    if (channelList.size() < 2) {
        printf("Error: Insufficient number of channels found [%u]\n", channelList.size());
        return;
    }
    TVChannel* ch = channelList[0];   // select channel to be recorded
    listener.init(source);
    source->setCurrentChannel(ch->getNumber());

    MediaRecorderOptions recorderOptions;
    recorderOptions.mode = RecordMode::ChunkedFifo;
    recorderOptions.chunkedFifoRecordSettings.interval = 30 * 60;
    recorderOptions.chunkedFifoRecordSettings.snapshotInterval = 2;
    recorderOptions.chunkedFifoRecordSettings.chunkSize = 1 * 1024 * 1024;

    MediaStream* mediaStream = tuner->getMediaStream();
    MediaRecorder recorder;
    recorderOptions.destFile = std::string("file://") + filename_circular;
    recorderOptions.sourceMediaStream = mediaStream;
    recorder.setOptions(recorderOptions);

    // Wait for channel to be changed before starting record
    // so the parserBand is set up.
    while (!listener.channelChanged()) {
        sleep(1);
    }

    IMedia::ErrorType errorType;
    errorType = recorder.start();
    ASSERT_EQ(errorType, IMedia::MEDIA_SUCCESS);

    // Record for 5 seconds and then convert
    sleep(5);

    // Convert to linear file
    TsbConverter* converter = new TsbConverter(&recorder);
    // Convert entire contents
    errorType = converter->start(filename_linear, 0, 0);
    EXPECT_EQ(errorType, IMedia::MEDIA_SUCCESS);

    // Continue to convert in progress recording
    sleep(5);
    errorType = converter->stop(filename_linear);
    EXPECT_EQ(errorType, IMedia::MEDIA_SUCCESS);
    recorder.stop();

    // Make sure TsbConverter doesn't crash in destructor
    delete converter;

    EXPECT_GT(RecorderUtil::getRecordingSize(filename_circular), 0);
    EXPECT_GT(RecorderUtil::getRecordingSize(filename_linear), 0);

    // Remove recording
    RecorderUtil::removeRecording(filename_circular);
    RecorderUtil::removeRecording(filename_linear);
    // Verify no associated recording files are found
    ASSERT_EQ(RecorderUtil::getRecordingFiles(filename_circular).size(), 0);
    ASSERT_EQ(RecorderUtil::getRecordingFiles(filename_linear).size(), 0);
}

TEST_F(TimeshiftTest, MultiSession)
{
    NEXUS_Error rc;
    // TODO: Determine numOfTranscoders at runtime
    int numOfTranscoders = 1;
    NEXUS_RecpumpOpenSettings recpumpOpenSettings;
    NEXUS_RecpumpSettings recpumpSettings;
    IMedia::ErrorType errorType;
    std::vector<std::string> filenames;
    std::vector<NEXUS_RecpumpHandle> recpumpHandles;
    std::vector<Transcoder*> transcoders;
    std::vector<MediaRecorder*> recorders;
    std::vector<TimeshiftSource*> timeshiftSources;
    std::vector<MyTVSourceListener*> listeners;
    std::vector<TVTuner*> tuners;
    std::vector<TVSource*> sources;

    NEXUS_Recpump_GetDefaultOpenSettings(&recpumpOpenSettings);
    MediaRecorderOptions recorderOptions;
    recorderOptions.mode = RecordMode::ChunkedFifo;
    recorderOptions.chunkedFifoRecordSettings.interval = 20;
    recorderOptions.chunkedFifoRecordSettings.snapshotInterval = 2;
    recorderOptions.chunkedFifoRecordSettings.chunkSize = 1 * 1024 * 1024;

    for (int i = 0; i < numOfTranscoders; ++i) {
        filenames.push_back("/data/record/live_buffer" + std::to_string(i) + ".mpg");
        // setup the recpump for buffering the live channel
        recpumpHandles.push_back(NEXUS_Recpump_Open(NEXUS_ANY_ID, &recpumpOpenSettings));
        ASSERT_NE(recpumpHandles[i], (void*)NULL);
        NEXUS_Recpump_GetSettings(recpumpHandles[i], &recpumpSettings);
        recpumpSettings.data.dataReady.callback = dataReadyCallback;
        recpumpSettings.data.dataReady.context = this;
        recpumpSettings.bandHold = NEXUS_RecpumpFlowControl_eEnable;
        rc = NEXUS_Recpump_SetSettings(recpumpHandles[i], &recpumpSettings);
        ASSERT_EQ(rc, NEXUS_SUCCESS);

        transcoders.push_back(new Transcoder());
        recorders.push_back(new MediaRecorder());
        timeshiftSources.push_back(new TimeshiftSource(recorders[i], true));
        tuners.push_back(_tunerList[i]);
        tuners[i]->setCurrentSource(_sourceType);
        sources.push_back(tuners[i]->getCurrentSource());
        ASSERT_NE(sources[i], (void*)NULL);

        listeners.push_back(new MyTVSourceListener());
        listeners[i]->init(sources[i]);

        vector<TVChannel*> channelList = sources[i]->getChannels();

        // ASSERT_GT(channelList.size(), 2);

        TVChannel* ch = channelList[0];

        MediaStream* mediaStream = tuners[i]->getMediaStream();
        recorderOptions.destFile = std::string("timeshift://") + filenames[i];
        recorderOptions.sourceMediaStream = mediaStream;
        recorders[i]->setOptions(recorderOptions);

        listeners[i]->reset();
        Broadcom::TV::TVErrorType tvErrorType = sources[i]->setCurrentChannel(ch->getNumber());
        EXPECT_EQ(tvErrorType, Broadcom::TV::TVErrorType::Success);
        while (!listeners[i]->channelChanged()) {
            sleep(1);
        }

        // Set data source after we are tuned since timeshiftSource
        // needs to know the pids
        timeshiftSources[i]->setDataSource(mediaStream);
        errorType = recorders[i]->start();
        EXPECT_EQ(errorType, IMedia::MEDIA_SUCCESS);

        TranscodeOptions transcodeOptions;
        transcodeOptions.enabled = true;
        transcodeOptions.realTime = false;

        TranscoderConfig* transcoderConfig = new TranscoderConfig();
        transcoderConfig->source = reinterpret_cast<void*>(timeshiftSources[i]);
        transcoderConfig->recpumpHandle = reinterpret_cast<void*>(recpumpHandles[i]);
        transcoderConfig->transcodeOptions = &transcodeOptions;
        transcoders[i]->setTranscodeConfig(transcoderConfig);
        errorType = transcoders[i]->prepare();
        ASSERT_EQ(tvErrorType, Broadcom::TV::TVErrorType::Success);

        NEXUS_Recpump_Start(recpumpHandles[i]);

        errorType = transcoders[i]->start();
        ASSERT_EQ(tvErrorType, Broadcom::TV::TVErrorType::Success);
    }
    sleep(10);

    const void* dataBuffer;
    size_t dataBufferSize;
    int idx = 0;
    while (idx < 10) {
        for (int i = 0; i < numOfTranscoders; ++i) {
            // TODO: Maybe check if content being transcoded is valid as well
            rc = NEXUS_Recpump_GetDataBuffer(recpumpHandles[i], &dataBuffer, &dataBufferSize);
            ASSERT_EQ(rc, NEXUS_SUCCESS);
            EXPECT_GT(dataBufferSize, 0);
            rc = NEXUS_Recpump_DataReadComplete(recpumpHandles[i], dataBufferSize);
            ASSERT_EQ(rc, NEXUS_SUCCESS);
        }
        idx++;
        BKNI_Sleep(500);
    }

    for (int i = 0; i < numOfTranscoders; ++i) {
        NEXUS_Recpump_Stop(recpumpHandles[i]);
        transcoders[i]->stop();
        // TsbConverter converter(recorders[i]);
        // converter.start("/data/record/" + ch->getName() + ".mpg", 0, 0);
        // // Poll until conversion is complete
        // while (converter.getStatus("/data/record/" + ch->getName()) != ConversionStatus::Done) {
        //     sleep(1);
        // }
        recorders[i]->stop();
        delete transcoders[i]->getTranscodeConfig();
        // Verify TSB files created
        EXPECT_GT(RecorderUtil::getRecordingSize(filenames[i]), 0);
        RecorderUtil::removeRecording(filenames[i]);
    }
}

TEST_F(TimeshiftTest, GetStartTime)
{
    NEXUS_Error rc;
    int tsbLength = 10; // in sec.
    uint32_t numOfTsb = 1;
    IMedia::ErrorType errorType;
    std::vector<std::string> filenames;
    std::vector<MediaRecorder*> recorders;
    std::vector<TimeshiftSource*> timeshiftSources;
    std::vector<MyTVSourceListener*> listeners;
    std::vector<TVTuner*> tuners;
    std::vector<TVSource*> sources;

    MediaRecorderOptions recorderOptions;
    recorderOptions.mode = RecordMode::ChunkedFifo;
    recorderOptions.chunkedFifoRecordSettings.interval = tsbLength;
    recorderOptions.chunkedFifoRecordSettings.snapshotInterval = 2;
    recorderOptions.chunkedFifoRecordSettings.chunkSize = 1 * 1024 * 1024;

    for (uint32_t i = 0; i < numOfTsb; ++i) {
        filenames.push_back("/data/record/live_buffer" + std::to_string(i) + ".mpg");

        recorders.push_back(new MediaRecorder());
        timeshiftSources.push_back(new TimeshiftSource(recorders[i], true));
        tuners.push_back(_tunerList[i]);
        tuners[i]->setCurrentSource(_sourceType);
        sources.push_back(tuners[i]->getCurrentSource());
        ASSERT_NE(sources[i], (void*)NULL);

        listeners.push_back(new MyTVSourceListener());
        listeners[i]->init(sources[i]);

        vector<TVChannel*> channelList = sources[i]->getChannels();

        // ASSERT_GT(channelList.size(), 2);

        TVChannel* ch = channelList[0];

        MediaStream* mediaStream = tuners[i]->getMediaStream();
        recorderOptions.destFile = std::string("timeshift://") + filenames[i];
        recorderOptions.sourceMediaStream = mediaStream;
        recorders[i]->setOptions(recorderOptions);

        listeners[i]->reset();
        Broadcom::TV::TVErrorType tvErrorType = sources[i]->setCurrentChannel(ch->getNumber());
        EXPECT_EQ(tvErrorType, Broadcom::TV::TVErrorType::Success);
        while (!listeners[i]->channelChanged()) {
            sleep(1);
        }

        // Set data source after we are tuned since timeshiftSource
        // needs to know the pids
        timeshiftSources[i]->setDataSource(mediaStream);
        errorType = recorders[i]->start();
        EXPECT_EQ(errorType, IMedia::MEDIA_SUCCESS);
    }

    sleep(5);
    for (uint32_t i = 0; i < numOfTsb; ++i) {
        printf("Start Time 1: %llu\n", timeshiftSources[i]->getTimeInfo().startTime);
        EXPECT_EQ(timeshiftSources[i]->getTimeInfo().startTime, 0);
    }

    sleep(tsbLength);
    for (uint32_t i = 0; i < numOfTsb; ++i) {
        printf("Start Time 2: %llu\n", timeshiftSources[i]->getTimeInfo().startTime);
        EXPECT_GT(timeshiftSources[i]->getTimeInfo().startTime, 0);
    }

    for (uint32_t i = 0; i < recorders.size(); ++i) {
        recorders[i]->stop();
        // Verify TSB files created
        EXPECT_GT(RecorderUtil::getRecordingSize(filenames[i]), 0);
        RecorderUtil::removeRecording(filenames[i]);
    }
}

TEST_F(TimeshiftTest, Seek)
{
    NEXUS_Error rc;
    // TODO: Determine numOfTranscoders at runtime
    uint32_t numOfTranscoders = 1;
    uint32_t initialWait = 5;   // in sec.
    uint32_t tsbLength = 10;    // in sec.
    NEXUS_RecpumpOpenSettings recpumpOpenSettings;
    NEXUS_RecpumpSettings recpumpSettings;
    IMedia::ErrorType errorType;
    std::vector<std::string> filenames;
    std::vector<NEXUS_RecpumpHandle> recpumpHandles;
    std::vector<Transcoder*> transcoders;
    std::vector<MediaRecorder*> recorders;
    std::vector<TimeshiftSource*> timeshiftSources;
    std::vector<MyTVSourceListener*> listeners;
    std::vector<TVTuner*> tuners;
    std::vector<TVSource*> sources;

    NEXUS_Recpump_GetDefaultOpenSettings(&recpumpOpenSettings);

    MediaRecorderOptions recorderOptions;
    recorderOptions.mode = RecordMode::ChunkedFifo;
    recorderOptions.chunkedFifoRecordSettings.interval = tsbLength;
    recorderOptions.chunkedFifoRecordSettings.snapshotInterval = 2;
    recorderOptions.chunkedFifoRecordSettings.chunkSize = 1 * 1024 * 1024;

    for (uint32_t i = 0; i < numOfTranscoders; ++i) {
        filenames.push_back("/data/record/live_buffer" + std::to_string(i) + ".mpg");
        // setup the recpump for buffering the live channel
        recpumpHandles.push_back(NEXUS_Recpump_Open(NEXUS_ANY_ID, &recpumpOpenSettings));
        ASSERT_NE(recpumpHandles[i], (void*)NULL);
        NEXUS_Recpump_GetSettings(recpumpHandles[i], &recpumpSettings);
        recpumpSettings.data.dataReady.callback = dataReadyCallback;
        recpumpSettings.data.dataReady.context = this;
        recpumpSettings.bandHold = NEXUS_RecpumpFlowControl_eEnable;
        rc = NEXUS_Recpump_SetSettings(recpumpHandles[i], &recpumpSettings);
        ASSERT_EQ(rc, NEXUS_SUCCESS);

        transcoders.push_back(new Transcoder());
        recorders.push_back(new MediaRecorder());
        timeshiftSources.push_back(new TimeshiftSource(recorders[i], true));
        tuners.push_back(_tunerList[i]);
        tuners[i]->setCurrentSource(_sourceType);
        sources.push_back(tuners[i]->getCurrentSource());
        ASSERT_NE(sources[i], (void*)NULL);

        listeners.push_back(new MyTVSourceListener());
        listeners[i]->init(sources[i]);

        vector<TVChannel*> channelList = sources[i]->getChannels();

        // ASSERT_GT(channelList.size(), 2);

        TVChannel* ch = channelList[0];

        MediaStream* mediaStream = tuners[i]->getMediaStream();
        recorderOptions.destFile = std::string("timeshift://") + filenames[i];
        recorderOptions.sourceMediaStream = mediaStream;
        recorders[i]->setOptions(recorderOptions);

        listeners[i]->reset();
        Broadcom::TV::TVErrorType tvErrorType = sources[i]->setCurrentChannel(ch->getNumber());
        EXPECT_EQ(Broadcom::TV::TVErrorType::Success, tvErrorType);
        while (!listeners[i]->channelChanged()) {
            sleep(1);
        }

        // Set data source after we are tuned since timeshiftSource
        // needs to know the pids
        timeshiftSources[i]->setDataSource(mediaStream);
        errorType = recorders[i]->start();
        EXPECT_EQ(IMedia::MEDIA_SUCCESS, errorType);

        TranscodeOptions transcodeOptions;
        transcodeOptions.enabled = true;
        transcodeOptions.realTime = false;

        TranscoderConfig* transcoderConfig = new TranscoderConfig();
        transcoderConfig->source = reinterpret_cast<void*>(timeshiftSources[i]);
        transcoderConfig->recpumpHandle = reinterpret_cast<void*>(recpumpHandles[i]);
        transcoderConfig->transcodeOptions = &transcodeOptions;
        transcoders[i]->setTranscodeConfig(transcoderConfig);
        errorType = transcoders[i]->prepare();
        ASSERT_EQ(Broadcom::TV::TVErrorType::Success, tvErrorType);

        NEXUS_Recpump_Start(recpumpHandles[i]);

        errorType = transcoders[i]->start();
        ASSERT_EQ(Broadcom::TV::TVErrorType::Success, tvErrorType);
    }

    sleep(initialWait);
    // Seek to valid range
    for (uint32_t i = 0; i < numOfTranscoders; ++i) {
        uint32_t seekPosition = (initialWait * 1000) / 2;
        uint32_t currentPosition = 0;
        printf("Seeking to position %u\n", seekPosition);
        errorType = timeshiftSources[i]->seekTo(seekPosition);
        EXPECT_EQ(IMedia::MEDIA_SUCCESS, errorType);
        currentPosition = timeshiftSources[i]->getCurrentPosition();
        printf("Current position %u\n", currentPosition);
        EXPECT_GE(currentPosition, seekPosition);
        IMedia::TimeInfo timeInfo = timeshiftSources[i]->getTimeInfo();
        printf("Current TSB range: [%llu, %llu]\n", timeInfo.startTime, timeInfo.endTime);
        EXPECT_EQ(0, timeInfo.startTime);
        EXPECT_GE(timeInfo.endTime, initialWait * 1000);
    }

    sleep(tsbLength);
    // Seek to invalid range
    for (uint32_t i = 0; i < numOfTranscoders; ++i) {
        uint32_t seekPosition = 1000;
        uint32_t currentPosition = 0;
        printf("Seeking to position %u\n", seekPosition);
        errorType = timeshiftSources[i]->seekTo(seekPosition);
        EXPECT_EQ(IMedia::MEDIA_ERROR_ARG_OUT_OF_RANGE, errorType);
        currentPosition = timeshiftSources[i]->getCurrentPosition();
        printf("Current position %u\n", currentPosition);
        EXPECT_GT(currentPosition, seekPosition);
        IMedia::TimeInfo timeInfo = timeshiftSources[i]->getTimeInfo();
        printf("Current TSB range: [%llu, %llu]\n", timeInfo.startTime, timeInfo.endTime);
        EXPECT_GT(timeInfo.startTime, 0);
        EXPECT_GE(timeInfo.endTime, (tsbLength + initialWait) * 1000);
    }

    const void* dataBuffer;
    size_t dataBufferSize;
    int idx = 0;
    while (idx < 10) {
        for (uint32_t i = 0; i < numOfTranscoders; ++i) {
            // TODO: Maybe check if content being transcoded is valid as well
            rc = NEXUS_Recpump_GetDataBuffer(recpumpHandles[i], &dataBuffer, &dataBufferSize);
            ASSERT_EQ(NEXUS_SUCCESS, rc);
            EXPECT_GT(dataBufferSize, 0);
            rc = NEXUS_Recpump_DataReadComplete(recpumpHandles[i], dataBufferSize);
            ASSERT_EQ(NEXUS_SUCCESS, rc);
        }
        idx++;
        BKNI_Sleep(500);
    }

    for (uint32_t i = 0; i < numOfTranscoders; ++i) {
        NEXUS_Recpump_Stop(recpumpHandles[i]);
        transcoders[i]->stop();
        recorders[i]->stop();
        delete transcoders[i]->getTranscodeConfig();
        // Verify TSB files created
        EXPECT_GT(RecorderUtil::getRecordingSize(filenames[i]), 0);
        RecorderUtil::removeRecording(filenames[i]);
    }
}

}  // namespace Media
}  // namespace Broadcom

int main(int argc, char** argv)
{
    std::string modulation = "atsc";
    testing::InitGoogleTest(&argc, argv);
    for (int i = 0; i < argc; ++i) {
        if (std::string(argv[i]).compare(0, 6, "--mod=") == 0) {
            modulation = std::string(argv[i]).substr(6);
        }
    }
    if (modulation == "atsc")
        _sourceType = TVSOURCETYPE_ATSC;
    else if (modulation == "dvbs")
        _sourceType = TVSOURCETYPE_DVBS;
    else if (modulation == "dvbs2")
        _sourceType = TVSOURCETYPE_DVBS2;
    else if (modulation == "dvbs2x")
        _sourceType = TVSOURCETYPE_DVBS2X;
    else if (modulation == "dvbt")
        _sourceType = TVSOURCETYPE_DVBT;
    else if (modulation == "dvbt2")
        _sourceType = TVSOURCETYPE_DVBT2;
    else if (modulation == "dvbc")
        _sourceType = TVSOURCETYPE_DVBC;
    else if (modulation == "dvbc2")
        _sourceType = TVSOURCETYPE_DVBC2;

    return RUN_ALL_TESTS();
}
