/***************************************************************************
*  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*  See ‘License-BroadcomSTB-CM-Software.txt’ for terms and conditions.
***************************************************************************/
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <string>
#include <string.h>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "AudioPlayback.h"
#include "AudioPlaybackListener.h"

using namespace Broadcom;
using namespace Broadcom::Media;

class PcmPlayerListener
    : public AudioPlaybackListener
{
    public:
        PcmPlayerListener(AudioPlayback* audioPlayback)
            : _callback(NULL)
        {
            observe(audioPlayback);
        }

        void onBufferReady()
        {
            printf("PcmPlayerListener: onBufferReady\n");
            if (_callback) {
                _callback();
            }
        }

        void registerCallback(std::function<void()> callback)
        {
            _callback = callback;
        }

        void unregisterCallback()
        {
            _callback = NULL;
        }

    private:
        std::function<void()> _callback;
};

class PcmPlayer
{
 public:
    PcmPlayer(int argc, char** argv)
        : _argc(argc),
        _argv(argv)
    {
        _bufferReady = false;
        _uri = _argv[1];
    }

    void onBufferReady()
    {
        printf("PcmPlayer: onBufferReady\n");
        std::unique_lock<std::mutex> myLock(_bufferReadyMutex);
        _bufferReady = true;
        _bufferReadyCV.notify_one();
    }

    void run()
    {
        uint8_t *data;
        size_t dataSize = 0;
        int32_t sizeRead = 0;
        int32_t sizePushed = 0;
        int32_t sizeToBePushed = 0;
        int error_code = 0;
        FILE *file;

        // Open AudioPlayback
        AudioPlaybackOpenSettings openSettings;
        _audioPlayback = AudioPlayback::open(openSettings);
        if (_audioPlayback == 0) {
            printf("Error on open...exiting\n");
            return;
        }
        _pcmPlayerListener = new PcmPlayerListener(_audioPlayback);
        _pcmPlayerListener->registerCallback(std::bind(&PcmPlayer::onBufferReady, this));

        // Start audioPlayback
        AudioPlaybackParameters playbackParams;
        playbackParams.sampleRate = 44100;
        playbackParams.bitsPerSample = 16;
        playbackParams.numOfChannels = 2;
        error_code = _audioPlayback->start(playbackParams);
        if (error_code) {
            printf("Error on start...exiting\n");
            return;
        }

        // Set volume to 10
        printf("Setting volume to 1.0...\n");
        _audioPlayback->setVolume(1.0f);
        printf("getVolume() = %f\n", _audioPlayback->getVolume());

        file = fopen(_uri.c_str(), "rb");
        if (!file) {
            printf("unable to open %s\n", _uri.c_str());
            return;
        }

        // Determine size of file
        fseek(file, 0, SEEK_END);
        dataSize = ftell(file);
        if (dataSize <= 0) {
            printf("unable to seek in file\n");
            return;
        }
        // Rewind file ptr back to beginning
        fseek(file, 0, SEEK_SET);
        data = (uint8_t*)malloc(dataSize);
        if (!data) {
            printf("unable to allocate memory\n");
            return;
        }

        sizeRead = fread(data, 1, dataSize, file);

        if (sizeRead < (int32_t)dataSize) {
            printf("Cannot read in all bytes from file\n");
            return;
        }

        sizeToBePushed = sizeRead;
        while (sizePushed < sizeRead) {
            int n = _audioPlayback->pushAudioChunk(data + sizePushed, sizeToBePushed);
            if (n < 0) {
                printf("Error on pushAudioChunk\n");
            }
            sizePushed += n;
            if (sizePushed < sizeRead) {
                // Not enough room available in playback buffer,
                // so we need to wait until some space frees up
                printf("Ran out of buffer space totalSize [%d] sizePushed [%d]\n", sizeRead, sizePushed);
                std::unique_lock<std::mutex> myLock(_bufferReadyMutex);
                _bufferReady = false;
                bool success = _bufferReadyCV.wait_for(myLock, std::chrono::milliseconds(2000),
                               [this]() { return _bufferReady; });
                if (!success) {
                    printf("Did not get bufferReady event\n");
                }
            }
            sizeToBePushed = sizeRead - sizePushed;
        }

        // Wait for player to finish playing all bytes
        // and then stop
        while (1) {
            AudioPlaybackStatus status = _audioPlayback->getStatus();
            if (status.queuedBytes == 0) {
                break;
            }
            usleep(100000);
        }

        printf("Stopping audio playback...\n");
        _audioPlayback->stop();

        // Clean up
        // Unregister callback from listener
        _pcmPlayerListener->unregisterCallback();
        // Close AudioPlayback
        _audioPlayback->close();
        // Delete _audioPlayback before listener
        // to ensure we get no more callbacks to listener
        delete _audioPlayback;
        // Delete listener
        delete _pcmPlayerListener;

        // Clean up other resources
        fclose(file);
        free(data);
    }

 private:
    int _argc;
    char** _argv;
    AudioPlayback* _audioPlayback;
    PcmPlayerListener* _pcmPlayerListener;
    std::string _uri;
    std::mutex _bufferReadyMutex;
    std::condition_variable _bufferReadyCV;
    bool _bufferReady;
};

void printUsage()
{
    printf("Usage: pcmPlayer <uri>\n");
    printf("(ie ./pcmPlayer /data/media/test.pcm)\n");
}

int main(int argc, char **argv)
{
    if ((argc <= 1) || (std::string(argv[1]) == "--help")) {
        printUsage();
        return -1;
    }

    std::unique_ptr<PcmPlayer> app(new PcmPlayer(argc, argv));
    app->run();

    return 0;
}
