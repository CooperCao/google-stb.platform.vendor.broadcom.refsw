/***************************************************************************
*  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*  See ‘License-BroadcomSTB-CM-Software.txt’ for terms and conditions.
***************************************************************************/
#include <gtest/gtest.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <curl.h>
#include "MediaPlayer.h"
#include "MediaPlayerListener.h"
#include "PushYTMediaSource.h"

namespace Broadcom
{
namespace Media
{
/* Please go \\brcm-irv.broadcom.com\dfs\projects\stbdevstream_scratch\streams\playback\
* To get test stream
*/
static const std::string videoFile_h264 = "/usr/local/input.v.h264.mp4";
static const std::string audioFile_h264 = "/usr/local/input.a.h264.mp4";
static const std::string videoFile_vp9 = "/usr/local/input.v.vp9.mp4";
static const std::string audioFile_vp9 = "/usr/local/input.a.vp9.mp4";
static const std::string videoFile_drm = "/usr/local/oops_cenc-20121114-145.mp4";
static const std::string audioFile_drm = "/usr/local/oops_cenc-20121114-150.mp4";
static const char videoId[] = "1";
static const char audioId[] = "2";

const uint8_t kWidevineUUID[] = {0xed, 0xef, 0x8b, 0xa9, 0x79, 0xd6, 0x4a, 0xce,
                                 0xa3, 0xc8, 0x27, 0xdc, 0xd5, 0x1d, 0x21, 0xed};
const uint8_t kPlayreadyUUID[] = {0x9A, 0x04, 0xF0, 0x79, 0x98, 0x40, 0x42, 0x86,
                                  0xAB, 0x92, 0xE6, 0x5B, 0xE0, 0x88, 0x5F, 0x95};

enum CODECTYPE {
    CODECTYPE_H264,
    CODECTYPE_VP9
};

enum ENCTYPE {
    ENCTYPE_CLEAR,
    ENCTYPE_WV,
    ENCTYPE_PR
};

struct MemoryStruct {
    char *memory;
    size_t size;
};

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    //std::cout << "WriteMemoryCallback" << std::endl;
    mem->memory = (char*) realloc(mem->memory, mem->size + realsize + 1);
    if (mem->memory == NULL) {
        /* out of memory! */
        printf("not enough memory (realloc returned NULL)\n");
        return 0;
    }

    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

static void http_post(const char *url, const char *input, size_t inputsize, char **output, size_t *outsize)
{
    CURL *curl;
    CURLcode res;

    struct MemoryStruct chunk;
    //std::cout << "http_post ---- url : " << url << std::endl;
    //std::cout << "http_post ---- inputlen : " << inputsize << std::endl;
    //std::cout << "http_post ---- input : " << input << std::endl;

    chunk.memory = (char*) malloc(1);  /* will be grown as needed by the realloc above */
    chunk.size = 0;    /* no data at this point */

    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);

        /* send all data to this function  */
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

        /* we pass our 'chunk' struct to the callback function */
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

        /* some servers don't like requests that are made without a user-agent
           field, so we provide one */
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");

        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, input);

        /* if we don't provide POSTFIELDSIZE, libcurl will strlen() by
           itself */
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)inputsize);

        /* Perform the request, res will get the return code */
        res = curl_easy_perform(curl);
        /* Check for errors */
        if (res != CURLE_OK) {
            printf("curl_easy_perform() failed: %s %d\n",
                    curl_easy_strerror(res), res);
            *output = NULL;
            *outsize = 0;
        } else {
            /*
             * Now, our chunk.memory points to a memory block that is chunk.size
             * bytes big and contains the remote file.
             *
             * Do something nice with it!
             */
            //printf("response %s\n",chunk.memory);
            *output = chunk.memory;
            *outsize = chunk.size;
        }

        /* always cleanup */
        curl_easy_cleanup(curl);

        /* we're done with libcurl, so clean it up */
        curl_global_cleanup();
    }
}

class PushYTSourceTest
    : public ::testing::Test,
    public MediaPlayerListener,
    public PushYTMediaSourceListener
{
 public:
    PushYTSourceTest() :
        _mediaPlayer(NULL),
        _gotEndofStream(false),
        _gotPrepared(false),
        _gotError(false) {
        }

    virtual void SetUp() {
    }

    virtual void TearDown() {
    }

    void pushAudioTask(std::string inputFile)
    {
        // open file and send to media player
        uint32_t cnt = 0;
        FILE *fd = NULL;
        size_t bufLen = 32*1024;
        char buf[32*1024];
        IMedia::DataBuffer buffer;

        keyreqs.at_id = std::this_thread::get_id();
        //printf("pushAudioTask %x\n", keyreqs.at_id);
        if (NULL  == (fd =  fopen(inputFile.c_str(), "rb"))) {
            printf("failed to open file --- streamFile:%s\n", inputFile.c_str());
            return;
        }

        while (1) {
            if (bufLen != (cnt = fread(buf, 1, bufLen, fd))) {
                if (cnt == 0) {
                    std::cout << "Finished to read audio file to the end." << std::endl;
                    break;
                }
            }
            //std::cout << "audio read" << std::endl;
            buffer.setData((size_t)buf);
            buffer.setSize(cnt);
            _source->pushContainerChunk(buffer, audioId);
            usleep(1000000);
        }
        fclose(fd);
    }

    void pushVideoTask(std::string inputFile)
    {
        // open file and send to media player
        uint32_t cnt = 0;
        FILE *fd = NULL;
        size_t bufLen = 128*1024;
        char buf[128*1024];
        IMedia::DataBuffer buffer;

        keyreqs.vt_id = std::this_thread::get_id();
        //printf("pushVideoTask %x\n", keyreqs.vt_id);
        if (NULL  == (fd = fopen(inputFile.c_str(), "rb"))) {
            printf("failed to open file --- streamFile:%s\n", inputFile.c_str());
            return;
        }

        while (1) {
            if (bufLen != (cnt = fread(buf, 1, bufLen, fd))) {
                if (cnt == 0) {
                    std::cout << "Finished to read video file to the end." << std::endl;
                    break;
                }
            }
            //std::cout << "video read" << std::endl;
            buffer.setData((size_t)buf);
            buffer.setSize(cnt);
            _source->pushContainerChunk(buffer, videoId);
            usleep(300000);
        }
        fclose(fd);
    }

    struct KeyReq {
        std::string id;
        char * initData;
        size_t initDatasize;
        char *resp;
        size_t respsize;
    };

    class KeyReqs {
    public:
        KeyReqs() {
            reset(0);
            reset(1);
        }
        void reset(int i) {
            rq[i].initDatasize = rq[i].respsize = 0;
            rq[i].id = "";
        }
        KeyReq *get_keyreq() {
            if (vt_id == std::this_thread::get_id()) return &rq[0];
            if (at_id == std::this_thread::get_id()) return &rq[1];
            return NULL;
        }
        std::thread *get_thread() {
            if (vt_id == std::this_thread::get_id()) return &kt[0];
            if (at_id == std::this_thread::get_id()) return &kt[1];
            return NULL;
        }
        void join() {
            if (kt[0].joinable()) {
                //std::cout << "PlayVideo kt0 joinable" << std::endl;
                kt[0].join();
            }
            if (kt[1].joinable()) {
                //std::cout << "PlayVideo kt1 joinable" << std::endl;
                kt[1].join();
            }
        }
        std::thread kt[2];
        KeyReq rq[2];
        std::thread::id vt_id;
        std::thread::id at_id;
    };

    void procAddKey(KeyReq &krq)
    {
        IMedia::DataBuffer keyBuffer;
        size_t i = 0;
        //std::cout << "krq.respsize : " << krq.respsize << std::endl;

        for (i = 0; i < krq.respsize; i++) {
            if (enctype == ENCTYPE_PR) {
                if (krq.resp[i] == '<') break;
            } else if (enctype == ENCTYPE_WV) {
                if (krq.resp[i] == 0x8) break;
            }
        }
        //std::cout << "found the beginning point : " << i << std::endl;
        keyBuffer.setData((size_t)&krq.resp[i]);
        keyBuffer.setSize(krq.respsize-i);

        IMedia::DataBuffer initDataBuffer;
        initDataBuffer.setData((size_t)krq.initData);
        initDataBuffer.setSize(krq.initDatasize);

        if (enctype == ENCTYPE_PR) {
            _source->addKey(IMedia::YouTubePlayReadyEncryptionType, keyBuffer,
                            initDataBuffer, krq.id);
        } else if (enctype == ENCTYPE_WV) {
            _source->addKey(IMedia::YouTubeWidevineEncryptionType, keyBuffer,
                            initDataBuffer, krq.id);
        }
        //std::cout << "return addKey : " << krq.id << std::endl;
        free(krq.resp);
        free(krq.initData);
    }

    void procKeyTask(KeyReq *krq)
    {
        krq->respsize = 0;
        //std::cout << "procKeyTask start: " << krq->id << std::endl;

        while (1) {
            if (krq->respsize != 0) {
                //std::cout << "procKeyTask : " << krq->id << " " << krq->initDatasize << " " << krq->respsize << std::endl;
                KeyReq temp = *krq;
                procAddKey(temp);
                break;
            }
            usleep(100000);
        }
        //std::cout << "procKeyTask end" << std::endl;
    }

    void PlayVideo(const std::string& vFile, const std::string& aFile, CODECTYPE ct, ENCTYPE et)
    {
        Broadcom::Media::MediaStream mediaStream;
        IMedia::StreamMetadata metadata;
        metadata.streamType = IMedia::AutoStreamType;
        mediaStream.setUri(IMedia::PUSH_YTMS_URI_PREFIX);
        mediaStream.metadata = metadata;
        _mediaPlayer = new MediaPlayer();
        this->observe(_mediaPlayer);
        _mediaPlayer->setDataSource(&mediaStream);

        _mediaPlayer->prepare();
        ASSERT_EQ(_mediaPlayer->getState(), IMedia::StoppedState);
        _source = reinterpret_cast<PushYTMediaSource*>(_mediaPlayer->getSource());
        observeMediaSource(_source);

        Media::StreamData streamData;
        Media::StreamData streamDataAudio;
        Media::VideoCodecList videoCodecList;
        Media::AudioCodecList audioCodecList;

        streamData.setStreamType(IMedia::Mp4FragmentStreamType);
        streamDataAudio.setStreamType(IMedia::Mp4FragmentStreamType);
        if (ct == CODECTYPE_H264)
            videoCodecList.push_back(MediaPlayer::H264VideoCodec);
        else // ct == CODECTYPE_VP9
            videoCodecList.push_back(MediaPlayer::Vp9VideoCodec);
        streamData.setVideoCodecList(videoCodecList);

        enctype = et;

        audioCodecList.push_back(MediaPlayer::AacAdtsAudioCodec);
        streamDataAudio.setAudioCodecList(audioCodecList);
        streamDataAudio.setIsVideo(false);

        _source->addStream(videoId, streamData);
        _source->addStream(audioId, streamDataAudio);

        // Will creat threads to run decoder pipeline
        std::thread vt(&PushYTSourceTest::pushVideoTask, this, vFile);
        std::thread at(&PushYTSourceTest::pushAudioTask, this, aFile);

        sleep(5);
        _mediaPlayer->start();
        vt.join();
        at.join();
        keyreqs.join();

        _source->pushEndOfStream(0);
        while (_gotEndofStream == false) {}

        _mediaPlayer->stop();
        ASSERT_EQ(_mediaPlayer->getState(), IMedia::StoppedState);
        _mediaPlayer->release();
        delete _mediaPlayer;
    }

    uint32_t getPosition() {
        return _mediaPlayer->getCurrentPosition();
    }

    IMedia::State getState() {
        return _mediaPlayer->getState();
    }

    // listeners
    virtual void onCompletion() {
        printf("onCompletion\n");
        _gotEndofStream = true;
    }

    virtual void onError(const IMedia::ErrorType& errorType) {
        TRLS_UNUSED(errorType);
        _gotError = true;
    }

    virtual void onPrepared() {
        _gotPrepared = true;
    }

    virtual void onInfo(const IMedia::InfoType& infoType, int32_t extra) {
        TRLS_UNUSED(infoType);
        TRLS_UNUSED(extra);
    }
    virtual void onSeekComplete() { }
    virtual void onVideoSizeChanged(uint16_t width, uint16_t height) {
        TRLS_UNUSED(width);
        TRLS_UNUSED(height);
    }

    virtual void onDrmKeyNeeded(const IMedia::EncryptionType encSystem,
                                const std::string& sessionId,
                                const std::string& type,
                                const std::string& initData) {
        // mutex guaranteed between audiotask and videotask
        //std::cout << "---- onDrmKeyNeeded called : " << encSystem << std::endl;
        //std::cout << "---- onDrmKeyNeeded : " << sessionId << std::endl;
        //std::cout << "---- initDatasize : " << initData.size() << std::endl;
        TRLS_UNUSED(encSystem);
        TRLS_UNUSED(sessionId);
        TRLS_UNUSED(type);

        // Parsing initdata to match encSystem
        size_t psshdata_ptr = 0;
        size_t psshdata_size = 0;
        while (psshdata_ptr < initData.size()) {
            const unsigned char *t = (const unsigned char *) &initData.data()[psshdata_ptr];
            psshdata_size = (t[0] << 24) | (t[1] << 16) | (t[2] << 8) | t[3];
            if (enctype == ENCTYPE_PR) {
                if (memcmp(&t[12], kPlayreadyUUID, sizeof(kPlayreadyUUID)) == 0) break;
            } else if (enctype == ENCTYPE_WV) {
                if (memcmp(&t[12], kWidevineUUID, sizeof(kWidevineUUID)) == 0) break;
            }
            psshdata_ptr += psshdata_size;
        }

        if (psshdata_ptr >= initData.size()) {
            std::cout << "---- onDrmKeyNeeded : pssh data not found for the encSystem" << std::endl;
            return;
        }

        KeyReq *keyreq = keyreqs.get_keyreq();
        if (keyreq == NULL) {
            std::cout << "---- onDrmKeyNeeded critical error" << std::endl;
            return;
        }

        keyreq->initDatasize = psshdata_size;
        keyreq->initData = (char*) malloc(psshdata_size);
        for (size_t i = 0; i < psshdata_size; i++) {
            keyreq->initData[i] = initData.data()[psshdata_ptr+i];
        }

        std::thread *kthread = keyreqs.get_thread();
        if (kthread == NULL) {
            std::cout << "---- onDrmKeyNeeded critical error2" << std::endl;
            return;
        }
        *kthread = std::thread(&PushYTSourceTest::procKeyTask, this, keyreq);

        IMedia::DataBuffer initDataBuffer;
        initDataBuffer.setData((size_t)&initData.data()[psshdata_ptr]);
        initDataBuffer.setSize(psshdata_size);

        //std::cout << "---- generateKeyRequest called" << std::endl;
        ReturnCode rc;
        if (enctype == ENCTYPE_PR) {
            rc = _source->generateKeyRequest(IMedia::YouTubePlayReadyEncryptionType, initDataBuffer);
        } else if (enctype == ENCTYPE_WV) {
            rc = _source->generateKeyRequest(IMedia::YouTubeWidevineEncryptionType, initDataBuffer);
        }
        if (rc) {
            std::cout << "generateKeyRequest return error %d" << rc << std::endl;
        }
        //std::cout << "---- generateKeyRequest returns : " << rc << std::endl;
    }

    virtual void onDrmKeyMessage(const IMedia::EncryptionType encSystem,
                                 const std::string& sessionId,
                                 const std::string& message,
                                 const std::string& defaultUrl) {
        // mutex guaranteed between audiotask and videotask
        //std::cout << "---- onDrmKeyMessage is called : " << encSystem << std::endl;
        //std::cout << "---- sessionId : " << sessionId << std::endl;
        //std::cout << "---- messagesize : " << message.size() << std::endl;
        TRLS_UNUSED(defaultUrl);
        TRLS_UNUSED(encSystem);

        const std::string kGpLicenseServer = "http://dash-mse-test.appspot.com/api/drm/";
        const std::string kGpPlayreadyAuth =
        "playready?drm_system=playready&source=YOUTUBE&video_id=03681262dc412c06&ip=0.0.0.0&"
        "ipbits=0&expire=19000000000&sparams=ip,ipbits,expire,drm_system,source,video_id&"
        "signature=3BB038322E72D0B027F7233A733CD67D518AF675.2B7C39053DA46498D23F3BCB"
        "87596EF8FD8B1669&key=test_key1";
        const std::string kGpWidevineAuth =
        "widevine?drm_system=widevine&source=YOUTUBE&video_id=03681262dc412c06&ip=0.0.0.0&"
        "ipbits=0&expire=19000000000&sparams=ip,ipbits,expire,source,video_id,drm_system&"
        "signature=289105AFC9747471DB0D2A998544CC1DAF75B8F9.18DE89BB7C1CE9B68533315D"
        "0F84DF86387C6BB3&key=test_key1";

        std::string license_server;
        if (enctype == ENCTYPE_PR) {
            license_server.assign(kGpLicenseServer + kGpPlayreadyAuth);
        } else if (enctype == ENCTYPE_WV) {
            license_server.assign(kGpLicenseServer + kGpWidevineAuth);
        }
        char *output;
        size_t output_size;
        http_post(license_server.c_str(), message.data(), message.size(), &output, &output_size);
        //http_post(defaultUrl.c_str(), message.c_str(), &output, &output_size);

        KeyReq *keyreq = keyreqs.get_keyreq();
        if (keyreq == NULL) {
            std::cout << "---- onDrmKeyMessage critical error" << std::endl;
            return;
        }
        keyreq->id = sessionId;
        keyreq->resp = output;
        keyreq->respsize = output_size;
    }

    MediaPlayer* _mediaPlayer;
    PushYTMediaSource* _source;
    bool _gotEndofStream;
    bool _gotPrepared;
    bool _gotError;
    KeyReqs keyreqs;
    ENCTYPE enctype;
};

TEST_F(PushYTSourceTest, H264codec_Test)
{
    PlayVideo(videoFile_h264, audioFile_h264, CODECTYPE_H264, ENCTYPE_CLEAR);
}

TEST_F(PushYTSourceTest, VP9codec_Test)
{
    PlayVideo(videoFile_vp9, audioFile_vp9, CODECTYPE_VP9, ENCTYPE_CLEAR);
}

TEST_F(PushYTSourceTest, Playready_Test)
{
    PlayVideo(videoFile_drm, audioFile_drm, CODECTYPE_H264, ENCTYPE_PR);
}

TEST_F(PushYTSourceTest, Widevine_Test)
{
    PlayVideo(videoFile_drm, audioFile_drm, CODECTYPE_H264, ENCTYPE_WV);
}

}  // namespace Media
}  // namespace Broadcom
