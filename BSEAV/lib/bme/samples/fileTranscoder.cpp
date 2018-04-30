/***************************************************************************
*  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*  See ‘License-BroadcomSTB-CM-Software.txt’ for terms and conditions.
***************************************************************************/
#include <unistd.h>
#include <sys/time.h>
#include <fstream>
#include "Transcoder.h"
#include "TranscoderListener.h"
#include "MediaStream.h"

using namespace Broadcom;
using namespace Broadcom::Media;

void printUsage();
void PrintMenu(void);
unsigned input_number(void);
char ibuffer[256];
int exitFlag = 0;
std::ofstream outFile;

class Listener: public TranscoderListener
{
public:
    void onOutputData(uint8_t* dataBuffer, uint32_t size)
    {
        if (outFile.is_open()) {
            outFile.write((char*)dataBuffer, size);
        }
    }

    void onCompletion()
    {
        exitFlag = 1;
        printf("Transcode completed!!\n");
    }
};

int main(int argc, char **argv)
{
    if ((argc <= 1) || (std::string(argv[1]) == "--help")) {
        printUsage();
        return -1;
    }

    if (std::string(argv[1]).find("file:///") == std::string::npos) {
        printf("Input file name should start with file://<sourceFilePath>\n");
        printUsage();
        return -1;
    }

    std::string sourceFile;
    std::string destFile("transcodeOutput.mpg");
    int command;

    if (argc > 1) {
        sourceFile.assign(argv[1]);
    }
    if (argc > 2) {
        destFile.assign(argv[2]);
    }

    outFile.open(destFile, std::ios::out | std::ios::binary | std::ios::app);
    if (!outFile.is_open()) {
        printf("Cannot open file %s\n", destFile.c_str());
        return -1;
    }
    MediaStream* mediaStream;
    Transcoder* transcoder;
    Listener* listener;
    TranscoderConfig transcodeConfig; // transcoder input configurations
    TranscodeOptions options;  // transcode output options

    mediaStream = new MediaStream();
    transcoder = new Transcoder();
    listener = new Listener();

    mediaStream->setUri(sourceFile);

    transcodeConfig.notifyOutputData = true;
    transcodeConfig.mediaStream = mediaStream;
    transcodeConfig.transcodeOptions = &options; // default options;

    listener->observe(transcoder);
    transcoder->setTranscodeConfig(&transcodeConfig);
    transcoder->prepare();
    transcoder->start();

    PrintMenu();
    while ((command = input_number()) != EOF) {
        if (command == 0) {
            break;
        } else {
            printf("Bytes Transcoded: %d\n\n", transcoder->getBytesTranscoded());
        }
        PrintMenu();
    }

    transcoder->release();

    delete listener;
    delete mediaStream;
    delete transcoder;
    return 0;
}

void PrintMenu(void)
{
    printf("***********************************************\n");
    printf("*** App Console                             ***\n");
    printf("***********************************************\n");
    printf("State: Transcoding\n");
    printf("0) Exit\n");
    printf("1) Bytes Transcoded\n");
}

unsigned input_number(void)
{
    fd_set rfds;
    struct timeval tv;
    while (1) {
        FD_ZERO(&rfds);
        FD_SET(fileno(stdin), &rfds);
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        if (select(fileno(stdin) + 1, &rfds, NULL, NULL, &tv) > 0) {
            fgets(ibuffer, 100, stdin);
            break;
        } else {
           if (exitFlag) {
               outFile.close();
               return 0;
           }
        }
    }
    return atoi(ibuffer);
}
void printUsage(void)
{
    printf("Usage: ./fileTranscoder file://<sourceFilePath> [<destFilePath>]\n");
