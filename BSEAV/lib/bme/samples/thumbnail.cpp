/***************************************************************************
*  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*  See ‘License-BroadcomSTB-CM-Software.txt’ for terms and conditions.
***************************************************************************/
#include "nxclient.h"
#include "ThumbnailImgGen.h"
#include <string.h>

using namespace Broadcom;
using namespace Broadcom::Media;

void printUsage()
{
    printf("Usage: ./thumbnail <stream_file> <index_file> [OPTIONS]\n");
    printf("e.g: ./thumbnail cnnticker.mpg cnnticker.nav --time 1000 --interval 1000 --resolution 320,180 --output ./out\n");
    printf("\nOPTIONS:\n");
    printf(
        "   --time MILLISECONDS             time into the stream of where to capture I-Frame.  Defaults to 0.\n"
        "   --interval MILLISECONDS         amount of time between each I-Frame to capture.  A value of 0 means\n"
        "                                   do NOT repeat and capture.  Defaults to 0.\n"
        "   --output FOLDER_NAME            specifies the folder where to output all thumbnail captures.  Defaults to\n"
        "                                   current working directory.\n"
        "   --resolution WIDTH,HEIGHT       output resolution of thumbnail generated.  Defaults to capture size.\n"
    );
}

int main(int argc, char** argv)
{
    if (argc < 3) {
        printUsage();
        return -1;
    }

    if (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help")) {
        printUsage();
        return 0;
    }

    std::string stream(argv[1]);
    std::string indexFilename(argv[2]);
    int currentArg = 3;
    int interval = -1;
    int time = 0;
    uint32_t width = 320;
    uint32_t height = 180;
    std::string folderName = "./";
    NEXUS_Error rc;
    NxClient_JoinSettings joinSettings;
    NxClient_GetDefaultJoinSettings(&joinSettings);
    snprintf(joinSettings.name, NXCLIENT_MAX_NAME, "%s", argv[0]);
    rc = NxClient_Join(&joinSettings);
    if (rc) return -1;

    while (currentArg < argc) {
        if (!strcmp(argv[currentArg], "--time") && (currentArg + 1 < argc)) {
            time = atoi(argv[++currentArg]);
        } else if (!strcmp(argv[currentArg], "--interval") && (currentArg + 1 < argc)) {
            interval = atoi(argv[++currentArg]);
        } else if (!strcmp(argv[currentArg], "--output") && (currentArg + 1 < argc)) {
            folderName = argv[++currentArg];
        } else if (!strcmp(argv[currentArg], "--resolution") && (currentArg + 1 < argc)) {
            if (sscanf(argv[++currentArg], "%d,%d", &width, &height) != 2) {
                printf("Invalid width,height format.\n");
                printUsage();
                return 1;
            }
        } else {
            printUsage();
            return 1;
        }
        currentArg++;
    }

    MediaStream mediaStream;
    mediaStream.setUri(IMedia::FILE_URI_PREFIX + stream);

    ThumbnailOptions options;
    options.outputFilePath = folderName;
    options.outputFileName = "capturefile.jpg";
    options.time = time;
    options.width = width;
    options.height = height;
    options.mediaStream = &mediaStream;
    if  (interval > 0) {
        options.repeatEvery = interval;
    }
    options.inIndexFileName = indexFilename;

    ThumbnailImgGen thumbnailGen;
    thumbnailGen.setThumbnailOptions(options);

    printf("Generating thumbnails...\n");
    thumbnailGen.startGenImage();
    if (interval > 0) {
        sleep(15);
    }
    thumbnailGen.stopGenImage();
    printf("Thumbnails generated.\n");

    return 0;
}
