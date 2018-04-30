/***************************************************************************
*  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*  See ‘License-BroadcomSTB-CM-Software.txt’ for terms and conditions.
***************************************************************************/

#include <mutex>
#include <iostream>
#include <condition_variable>
#include <thread>
#include <string.h>
#include <chrono>
#include "Debug.h"
#include "nexus_recpump.h"
#include "nexus_playpump.h"
#include "nexus_still_decoder.h"
#include "nexus_graphics2d.h"
#include "jpeglib.h"
#include "ThumbnailExtractor.h"
#include "nexus_types.h"
#include "berr.h"
#include "bkni.h"
#include "ThumbnailImgGen.h"
#include "bcmindexer.h"
#include "bcmindexer_nav.h"
#include "FileSource.h"
#include "nxclient.h"
#include "bfile_stdio.h"

#define ICON_QUALITY 100
#define NAV_DATA_TIMEOUT 1000    // num of ms. to wait for nav data
#define NUM_OF_RETRIES 5

namespace Broadcom
{
namespace Media {
TRLS_DBG_MODULE(ThumbnailImgGen);


class ThumbnailHandles {
public:
    NEXUS_PidChannelHandle parserbandVideoPidChannel;
    NEXUS_PidChannelHandle playpumpPidChannel;
    NEXUS_ParserBand parserBand;
    NEXUS_PlaypumpHandle playpump;
    NEXUS_RecpumpHandle recpumpHandle;
    bthumbnail_extractor_t thumbnailExtractor;
    BKNI_EventHandle dataReadyEvent;
    BKNI_EventHandle indexReadyEvent;
    BKNI_EventHandle checkpointEvent;
    BKNI_EventHandle stillDecoderEvent;
    NEXUS_Graphics2DHandle gfxHandle;
    NEXUS_SurfaceHandle srcSurface;
    NEXUS_SurfaceHandle dstSurface;
    NEXUS_SurfaceMemory dstSurfaceMem;
    NEXUS_StillDecoderHandle stillDecoder;
    IMedia::StreamMetadata metadata;
    BaseSource* source;
    FILE* stdio_datafile;
    FILE* stdio_indexfile;
    bfile_io_read_t datafile;
    bfile_io_read_t indexfile;
    unsigned char* dataBuff; /* iFrame */
    unsigned dataBuffReadPos; /* readPos for dataBuf */
    size_t dataBuffLen; /* iframe Len */
    unsigned dataSize; /* iframe len */
    unsigned offset; /* iframe offset in recpump data */
    unsigned char* navBuff; /* navBuff */
    unsigned navBuffLen;  /* len of navBuff */
    unsigned curPos; /* read index for recpump */
    unsigned imageNum; /* counts number of images generated */
    std::thread genImageThread;
    BNAV_Indexer_Handle bcmindexer;
    bool overflowDetected;
    class ImageData { /* decoded RGB image */
    public:
        unsigned short int width;
        unsigned short int height;
        int depth;
        int format;
        char *data;
    };
    ImageData *imgPtr = NULL;

    void release();
    struct PrepareOptions
    {
        MediaStream* mediaStream;
        std::string indexFileName;
        unsigned width;
        unsigned height;
    };
    struct CaptureImageOptions
    {
        MediaStream* mediaStream;
        unsigned width;
        unsigned height;
        std::string outputFilePath;
        std::string outputFileName;
        unsigned time;
    };
    IMedia::ErrorType prepare(const PrepareOptions& options);
    IMedia::ErrorType captureImage(const CaptureImageOptions& options);
    IMedia::ErrorType generateNav();
    void calculateOffsetnSize();
    void readDataFromRecpump();
    void addToDataBuff(const void* buf, size_t size);
    void imageEncode(std::string outFilePath, std::string outFileName);
};

void ThumbnailHandles::release()
{
    BME_DEBUG_ENTER();
    NEXUS_StillDecoder_Close(stillDecoder);
    NEXUS_Surface_Destroy(dstSurface);
    NEXUS_Surface_Destroy(srcSurface);
    NEXUS_Graphics2D_Close(gfxHandle);
    bthumbnail_extractor_destroy(thumbnailExtractor);

    NEXUS_Playpump_ClosePidChannel(playpump, playpumpPidChannel);
    NEXUS_Playpump_Close(playpump);

    if (parserbandVideoPidChannel) {
        NEXUS_PidChannel_Close(parserbandVideoPidChannel);
        parserbandVideoPidChannel = NULL;
    }
    if (recpumpHandle) {
        NEXUS_Recpump_RemoveAllPidChannels(recpumpHandle);
        NEXUS_Recpump_Close(recpumpHandle);
        recpumpHandle = NULL;
    }
    if (dataBuff) {
        free(dataBuff);
        dataBuff = NULL;
    }
    if (navBuff) {
        free(navBuff);
        navBuff = NULL;
    }
    if (source) {
        source->release();
        delete source;
        source = NULL;
    }
    if (dataReadyEvent) {
        BKNI_DestroyEvent(dataReadyEvent);
        dataReadyEvent = NULL;
    }
    if (indexReadyEvent) {
        BKNI_DestroyEvent(indexReadyEvent);
        indexReadyEvent = NULL;
    }
    if (checkpointEvent) {
        BKNI_DestroyEvent(checkpointEvent);
        checkpointEvent = NULL;
    }
    if (stillDecoderEvent) {
        BKNI_DestroyEvent(stillDecoderEvent);
    }
    dataBuffReadPos = 0;
    dataBuffLen = 0;
    dataSize = 0;
    offset = 0;
    navBuffLen = 0;
    curPos = 0;
    overflowDetected = false;
    BME_DEBUG_EXIT();
}

static long unsigned int otfIndexerRecvNav(const void *ptr, long unsigned size,
                                                    long unsigned nmemb, void *fp)
{
    BME_DEBUG_ENTER();
    ThumbnailHandles* ctx = (ThumbnailHandles*)fp;
    BME_DEBUG_TRACE(("received nav data\n"));
    if (ctx->navBuff) {
        /* ignore nav data when we already have some nav data being processed */
        return (size * nmemb);
    } else {
        if (BNAV_get_frameType((BNAV_AVC_Entry*)ptr) == eSCTypeIFrame) {
            BME_DEBUG_TRACE(("found I frame\n"));
            ctx->navBuff = (unsigned char*)malloc(size * nmemb);
            ctx->navBuffLen = 0;
            memcpy(ctx->navBuff + ctx->navBuffLen, ptr, size * nmemb);
            ctx->navBuffLen += (size * nmemb);
        }
    }
    BME_DEBUG_EXIT();
    return (size * nmemb);
}

static ssize_t readBuffer(void* buf, size_t bufLength, void* data)
{
    BME_DEBUG_ENTER();
    ThumbnailHandles* ctx = reinterpret_cast<ThumbnailHandles*>(data);
    size_t readBufLength = bufLength;
    if (readBufLength > (ctx->dataSize - ctx->dataBuffReadPos)) {
        readBufLength = ctx->dataSize - ctx->dataBuffReadPos;
    }
    if (readBufLength) {
        memcpy((unsigned char*)buf, (unsigned char*)(ctx->dataBuff + ctx->dataBuffReadPos),
                                                                                readBufLength);
        BME_DEBUG_TRACE(("read buff func length: %d, bufLen: %d\n",
                                                readBufLength, ctx->dataBuffLen));
        ctx->dataBuffReadPos += readBufLength;
    }
    BME_DEBUG_EXIT();
    return readBufLength;
}

static void datareadyCallback(void *data, int param)
{
    BME_DEBUG_ENTER();
    if (data) {
        BKNI_SetEvent((BKNI_EventHandle)(data));
    }
    TRLS_UNUSED(param);
    BME_DEBUG_EXIT();
}

// If we get overflow, we must restart the recpump
static void overflowCallback(void *data, int param)
{
    BME_DEBUG_ENTER();
    ThumbnailHandles* handles = static_cast<ThumbnailHandles*>(data);
    handles->overflowDetected = true;
    BME_DEBUG_ERROR(("Overflow!!!"));
    TRLS_UNUSED(param);
    BME_DEBUG_EXIT();
}

void complete(void *context, int unused)
{
    BME_DEBUG_ENTER();
    TRLS_UNUSED(unused);
    BKNI_SetEvent((BKNI_EventHandle)context);
    BME_DEBUG_EXIT();
}

IMedia::ErrorType ThumbnailHandles::prepare(const PrepareOptions& options)
{
    BME_DEBUG_ENTER();
    bthumbnail_extractor_settings extracterSettings;
    bthumbnail_extractor_create_settings thumbExtCreateSettings;
    NEXUS_RecpumpOpenSettings recpumpOpenSettings;
    NEXUS_Graphics2DOpenSettings graphicsOpenSettings;
    NEXUS_SurfaceCreateSettings surfaceCreateSettings;
    NEXUS_PlaypumpOpenPidChannelSettings openPidChannelSettings;
    NEXUS_RecpumpSettings recpumpSettings;
    NEXUS_RecpumpAddPidChannelSettings pidSettings;
    NEXUS_Graphics2DSettings gfxSettings;

    BKNI_CreateEvent(&checkpointEvent);
    BKNI_CreateEvent(&stillDecoderEvent);

    if (options.mediaStream->metadata.parserBand) {
#ifdef BME_ENABLE_TV
        metadata = options.mediaStream->metadata;
        parserbandVideoPidChannel = NEXUS_PidChannel_Open(metadata.parserBand,
                                                metadata.videoParam.streamId, NULL);
        datafile = NULL;
        indexfile = NULL;
        overflowDetected = false;
        BKNI_CreateEvent(&dataReadyEvent);
        BKNI_CreateEvent(&indexReadyEvent);
        NEXUS_Recpump_GetDefaultOpenSettings(&recpumpOpenSettings);
        // Increase default buffer sizes to reduce probablity of overflow
        recpumpOpenSettings.data.bufferSize = 4 * recpumpOpenSettings.data.bufferSize;
        recpumpOpenSettings.index.bufferSize = 4 * recpumpOpenSettings.index.bufferSize;
        recpumpHandle = NEXUS_Recpump_Open(NEXUS_ANY_ID, &recpumpOpenSettings);
        NEXUS_Recpump_GetSettings(recpumpHandle, &recpumpSettings);
        BME_DEBUG_TRACE(("datasize:%d\n", recpumpOpenSettings.data.bufferSize));
        BME_DEBUG_TRACE(("indexsize:%d\n", recpumpOpenSettings.index.bufferSize));
        recpumpSettings.data.dataReady.callback = datareadyCallback;
        recpumpSettings.data.dataReady.context = (void*)dataReadyEvent;
        recpumpSettings.index.dataReady.callback = datareadyCallback; /* same callback */
        recpumpSettings.index.dataReady.context = (void*)indexReadyEvent; /* same event */
        recpumpSettings.data.overflow.callback = overflowCallback;
        recpumpSettings.data.overflow.context = (void*)this;
        recpumpSettings.index.overflow.callback = overflowCallback;
        recpumpSettings.index.overflow.context = (void*)this;
        NEXUS_Recpump_SetSettings(recpumpHandle, &recpumpSettings);

        NEXUS_Recpump_GetDefaultAddPidChannelSettings(&pidSettings);
        pidSettings.pidType = NEXUS_PidType_eVideo;
        pidSettings.pidTypeSettings.video.index = true;
        pidSettings.pidTypeSettings.video.codec =
                            (NEXUS_VideoCodec)metadata.videoParam.videoCodec;
        pidSettings.pidTypeSettings.video.pid = metadata.videoParam.streamId;
        NEXUS_Recpump_AddPidChannel(recpumpHandle, parserbandVideoPidChannel, &pidSettings);
#endif
    } else {
        if (options.mediaStream->getUri().find(IMedia::FILE_URI_PREFIX) != std::string::npos) {
            source = new FileSource();
            source->setDataSource(options.mediaStream);
            source->prepare();
            metadata = source->getStreamMetadata();
            stdio_datafile = fopen(options.mediaStream->getUri().substr(7).c_str(), "rb");
            if (options.indexFileName.length()) {
                stdio_indexfile =  fopen(options.indexFileName.c_str(), "rb");
                indexfile = bfile_stdio_read_attach(stdio_indexfile);
            }
            datafile = bfile_stdio_read_attach(stdio_datafile);
        } else {
            BME_DEBUG_ERROR(("unsupported media stream used to generate thumbnail\n"));
            return IMedia::MEDIA_ERROR_UNSUPPORTED_STREAM;
        }
    }

    playpump = NEXUS_Playpump_Open(NEXUS_ANY_ID, NULL);
    NEXUS_Playpump_GetDefaultOpenPidChannelSettings(&openPidChannelSettings);
    openPidChannelSettings.pidType = NEXUS_PidType_eVideo;
    playpumpPidChannel = NEXUS_Playpump_OpenPidChannel(playpump,
                                        metadata.videoParam.streamId,
                                        &openPidChannelSettings);

    memset(&thumbExtCreateSettings, 0, sizeof(bthumbnail_extractor_create_settings));
    bthumbnail_extractor_get_default_create_settings(&thumbExtCreateSettings);
    thumbnailExtractor = bthumbnail_extractor_create(&thumbExtCreateSettings);
    bthumbnail_extractor_get_settings(thumbnailExtractor, &extracterSettings);
    extracterSettings.videoCodec =
            (NEXUS_VideoCodec)metadata.videoParam.videoCodec;
    extracterSettings.transportType = NEXUS_TransportType_eTs;
    extracterSettings.timestampType = NEXUS_TransportTimestampType_eNone;
    extracterSettings.videoPid = metadata.videoParam.streamId;
    extracterSettings.playpump = playpump;
    extracterSettings.datafile = datafile;
#ifdef BME_ENABLE_TV
    if (options.mediaStream->metadata.parserBand) {
        extracterSettings.readBuffer = readBuffer;
    }
#endif
    extracterSettings.indexfile = indexfile;
    extracterSettings.data = (void*)this;
    bthumbnail_extractor_set_settings(thumbnailExtractor, &extracterSettings);

    NEXUS_Graphics2D_GetDefaultOpenSettings(&graphicsOpenSettings);
    graphicsOpenSettings.packetFifoSize = 1024;
    gfxHandle = NEXUS_Graphics2D_Open(NEXUS_ANY_ID, &graphicsOpenSettings);

    NEXUS_Graphics2D_GetSettings(gfxHandle, &gfxSettings);
    gfxSettings.checkpointCallback.callback = complete;
    gfxSettings.checkpointCallback.context = checkpointEvent;
    NEXUS_Graphics2D_SetSettings(gfxHandle, &gfxSettings);

    NEXUS_Surface_GetDefaultCreateSettings(&surfaceCreateSettings);
    surfaceCreateSettings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
    surfaceCreateSettings.width = 1920;
    surfaceCreateSettings.height = 1080;
    srcSurface = NEXUS_Surface_Create(&surfaceCreateSettings);

    NEXUS_Surface_GetDefaultCreateSettings(&surfaceCreateSettings);
    surfaceCreateSettings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
    surfaceCreateSettings.width = options.width;
    surfaceCreateSettings.height = options.height;
    dstSurface = NEXUS_Surface_Create(&surfaceCreateSettings);

    NEXUS_Surface_GetMemory(dstSurface, &dstSurfaceMem);
    BKNI_Memset(dstSurfaceMem.buffer, 0,
                        dstSurfaceMem.pitch * surfaceCreateSettings.height);
    NEXUS_Surface_Flush(dstSurface);

    stillDecoder = NEXUS_StillDecoder_Open(NULL, 0, NULL);

    BME_DEBUG_EXIT();
    return IMedia::MEDIA_SUCCESS;
}

void ThumbnailHandles::imageEncode(std::string outputFilePath, std::string outputFileName)
{
    BME_DEBUG_ENTER();
    FILE* outfile;               /* target file */
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    JSAMPLE *image_buffer;
    JSAMPROW row_pointer[1];      /* pointer to JSAMPLE row[s] */
    int row_stride;               /* physical row width in image buffer */
    ImageData* img_ptr = imgPtr;
    if (!outputFilePath.empty()) {
        // Add forward slash in case user omits it
        outputFilePath += "/";
    }
    std::string fileName = outputFilePath + std::to_string(imageNum) + outputFileName;

    if ((outfile = fopen(fileName.c_str(), "wb")) == NULL) {
        BME_DEBUG_TRACE(("image_encode(): can't open %s\n", fileName.c_str()));
        free(img_ptr->data); // brcm-bca: memleak
        return;
    } else {
        BME_DEBUG_TRACE(("image_encode(%p): To encode %s\n", img_ptr, fileName.c_str()));
        image_buffer = (JSAMPLE*)img_ptr->data;
        /* Step 1: allocate and initialize JPEG compression object */
        cinfo.err = jpeg_std_error(&jerr);
        jpeg_create_compress(&cinfo);
        /* Step 2: specify data destination (eg, a file) */
        jpeg_stdio_dest(&cinfo, outfile);
        /* Step 3: set parameters for compression */
        /* image width and height, in pixels */
        cinfo.image_width = (int)img_ptr->width;
        cinfo.image_height = (int)img_ptr->height;
        cinfo.input_components = img_ptr->depth;           /* # of color components per pixel */
        cinfo.in_color_space = JCS_RGB;       /* colorspace of input image */
        jpeg_set_defaults(&cinfo);
        jpeg_set_quality(&cinfo, ICON_QUALITY, TRUE /* limit to baseline-JPEG values  */);
        /* Step 4: Start compressor */
        jpeg_start_compress(&cinfo, TRUE);
        /* Step 5: while (scan lines remain to be written) */
        row_stride = img_ptr->width * 3; /* JSAMPLEs per row in image_buffer */
        while (cinfo.next_scanline < cinfo.image_height) {
            row_pointer[0] = & image_buffer[cinfo.next_scanline * row_stride];
            (void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
        }
        /* Step 6: Finish compression */
        jpeg_finish_compress(&cinfo);
        fclose(outfile);
        /* Step 7: release JPEG compression object */
        jpeg_destroy_compress(&cinfo);
        BME_DEBUG_TRACE(("image_encode(): end of image compressed\n"));
        free(img_ptr->data);
        imageNum++;
    }
    BME_DEBUG_EXIT();
}/* image_encode() */

void ThumbnailHandles::addToDataBuff(const void* buf, size_t size)
{
    BME_DEBUG_ENTER();
    if (dataBuff == NULL) {
        dataBuff = (unsigned char*)malloc(size);
    } else {
        dataBuff = (unsigned char*)realloc(dataBuff, dataBuffLen + size);
    }
    BME_DEBUG_TRACE(("dataBuff: %p, dataBuffLen: %d, size:%d\n", dataBuff, dataBuffLen, size));
    memcpy(dataBuff + dataBuffLen, buf, size);
    dataBuffLen += size;
    BME_DEBUG_EXIT();
}

void ThumbnailHandles::readDataFromRecpump()
{
    BME_DEBUG_ENTER();
    bool readMore = true;
    const void* dataBuffer;
    size_t dataBufferSize;
    unsigned startPos;
    unsigned dataToRead = 0;
    unsigned remainingDataInBuffer = 0;
    BME_DEBUG_TRACE(("total Data to read: %d\n", dataSize));
    while (readMore) {
        NEXUS_Recpump_GetDataBuffer(recpumpHandle, &dataBuffer, &dataBufferSize);
        if (dataBufferSize == 0) {
            /* wait for more data in recpump */
            BKNI_Sleep(50);
            continue;
        }
        dataToRead = dataSize - dataBuffLen;
        if (dataToRead > dataBufferSize) {
            dataToRead = dataBufferSize;
        }
        BME_DEBUG_TRACE(("datainRecpumpAvail: %d\n", dataBufferSize));
        BME_DEBUG_TRACE(("dataToReadFromRecpump: %d\n", dataToRead));
        BME_DEBUG_TRACE(("curPos: %d\n", curPos));
        BME_DEBUG_TRACE(("offset: %d\n", offset));
        BME_DEBUG_TRACE(("read so far: dataBuffLen : %d\n", dataBuffLen));
        if (curPos >= offset) {
            /* copy as much required from available*/
            BME_DEBUG_TRACE(("copy as much required\n"));
            addToDataBuff(dataBuffer, dataToRead);
            curPos += dataBufferSize;
        } else if (curPos + dataBufferSize <= offset) {
            /* nothing to copy */
            BME_DEBUG_TRACE(("nothing to copy\n"));
            curPos += dataBufferSize;
        } else {
            /* skip upto offset and copy required from remaining available*/
            BME_DEBUG_TRACE(("partial dataBuffer size: %d\n", dataBufferSize));
            startPos = offset - curPos;
            remainingDataInBuffer = dataBufferSize - startPos;
            if (dataToRead > remainingDataInBuffer) {
                dataToRead = remainingDataInBuffer;
            }
            BME_DEBUG_TRACE(("startPos:%d, dataToRead:%d\n", startPos, dataToRead));
            addToDataBuff((const unsigned char*)dataBuffer + startPos, dataToRead);
            curPos += dataBufferSize;
        }
        NEXUS_Recpump_DataReadComplete(recpumpHandle, dataBufferSize);
        BME_DEBUG_TRACE(("\n"));
        if (dataBuffLen > dataSize) {
            BME_DEBUG_ERROR(("Buffer length greater than data to be read. Error !!!\n"));
            exit(0);
        }
        if (dataSize == dataBuffLen) {
            BME_DEBUG_TRACE(("success read sufficient data\n"));
            NEXUS_Recpump_Stop(recpumpHandle);
            readMore = false;
        }
    }
    BME_DEBUG_EXIT();
}

void ThumbnailHandles::calculateOffsetnSize()
{
    BNAV_AVC_Entry* nav;
    unsigned loffset;
    unsigned size;
    unsigned backup;
    unsigned packetSize = 188;

    nav = (BNAV_AVC_Entry*)navBuff;
    BME_DEBUG_TRACE(("frame type: %d\n", BNAV_get_frameType(nav)));
    loffset = BNAV_get_frameOffsetLo(nav);
    size = BNAV_get_frameSize(nav);
    backup = BNAV_get_seqHdrStartOffset(nav);
    loffset -= backup;
    size += backup;
    /* packet align */
    if (loffset % packetSize) {
        size += loffset % packetSize;
        loffset -= loffset % packetSize;
    }
    if (size % packetSize) {
        size += packetSize - (size % packetSize);
    }
    offset = loffset;
    dataSize = size;
    BME_DEBUG_TRACE(("found frame: %d %d with backup %d", offset, size, backup));
    BME_DEBUG_EXIT();
}

IMedia::ErrorType ThumbnailHandles::generateNav()
{
    IMedia::ErrorType errorType = IMedia::MEDIA_SUCCESS;
    size_t indexBufferSize = 0;
    const void* indexBuffer;
    NEXUS_Error rc = NEXUS_SUCCESS;
    BNAV_Indexer_Settings bcmindexerSettings;
    long numEntries;
    long feedStatus = 0;
    int indexReadyCount = 0;
    int noNewNavDataCount = 0;
    int navDataTimeout = NAV_DATA_TIMEOUT / 50;
    while (navBuffLen == 0) {
        rc = BKNI_WaitForEvent(indexReadyEvent, 50);
        if (rc == 0) {
            indexBufferSize = 0;
            NEXUS_Recpump_GetIndexBuffer(recpumpHandle, &indexBuffer, &indexBufferSize);
            if (indexBufferSize) {
                indexBufferSize -= (indexBufferSize % sizeof(BSCT_SixWord_Entry));
                numEntries = indexBufferSize / sizeof(BSCT_SixWord_Entry);
                if (numEntries % 2) {
                    indexBufferSize -= sizeof(BSCT_SixWord_Entry);
                    numEntries = indexBufferSize / sizeof(BSCT_SixWord_Entry);
                }
                BME_DEBUG_TRACE(("numEntries; %ld\n", numEntries));
                if (numEntries) {
                    BNAV_Indexer_GetDefaultSettings(&bcmindexerSettings);
                    bcmindexerSettings.navVersion = BNAV_Version2;
                    bcmindexerSettings.videoFormat = BNAV_Indexer_VideoFormat_MPEG2;
                    bcmindexerSettings.writeCallback = otfIndexerRecvNav;
                    bcmindexerSettings.filePointer = (void*)this;
                    bcmindexerSettings.transportTimestampEnabled = 0;
                    bcmindexerSettings.sctVersion = BSCT_Version6wordEntry;
                    BNAV_Indexer_Open(&bcmindexer, &bcmindexerSettings);
                    feedStatus = BNAV_Indexer_Feed(bcmindexer,
                                                    (void*)indexBuffer, numEntries);
                    BME_DEBUG_TRACE(("feedStatus: %ld, numEntries: %ld\n",
                                                        feedStatus, numEntries));
                    if (feedStatus != -1 && navBuffLen) {
                        calculateOffsetnSize();
                        readDataFromRecpump();
                        NEXUS_Recpump_IndexReadComplete(recpumpHandle, indexBufferSize);
                        noNewNavDataCount = 0;
                    } else {
                        // We want to read in more data to the buffer without discarding
                        // any of the data currently in the buffer.  To accomplish this,
                        // we need to call NEXUS_Recpump_IndexReadComplete with 0 bytes read.
                        NEXUS_Recpump_IndexReadComplete(recpumpHandle, 0);
                        noNewNavDataCount++;
                        usleep(50000);  // Wait 50ms for new nav data
                    }
                    BNAV_Indexer_Close(bcmindexer);

                    if (navBuffLen == 0 && overflowDetected) {
                        errorType = IMedia::MEDIA_ERROR_RECORDER_OVERFLOW;
                        break;
                    }
                }
            }
            indexReadyCount = 0;
        } else {
            BME_DEBUG_TRACE(("indexReadyCount = %d", indexReadyCount));
            indexReadyCount++;
        }

        if (indexReadyCount >= navDataTimeout || noNewNavDataCount >= navDataTimeout) {
            BME_DEBUG_ERROR(("%p: IndexReady event timed out!\n", this));
            return IMedia::MEDIA_ERROR_RECORDER_NO_DATA_TIMEOUT;
        }
    }
    BME_DEBUG_EXIT();
    return errorType;
}

IMedia::ErrorType ThumbnailHandles::captureImage(const CaptureImageOptions& options)
{
    BME_DEBUG_ENTER();
    NEXUS_StillDecoderStartSettings stillDecoderStartSettings;
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_StripedSurfaceHandle stripedSurface;
    NEXUS_Graphics2DBlitSettings blitSettings;
    NEXUS_StillDecoderStatus status;
    char* ptr = NULL;
#ifdef BME_ENABLE_TV
    if (options.mediaStream->metadata.parserBand) {
        IMedia::ErrorType navError;
        NEXUS_Recpump_Start(recpumpHandle);
        navError = generateNav();
        if (navError == IMedia::MEDIA_ERROR_RECORDER_OVERFLOW ||
            navError == IMedia::MEDIA_ERROR_RECORDER_NO_DATA_TIMEOUT) {
            NEXUS_Recpump_Stop(recpumpHandle);
            return navError;
        }
    }
#endif
    bthumbnail_extractor_start_playpump(thumbnailExtractor);
    NEXUS_StillDecoder_GetDefaultStartSettings(&stillDecoderStartSettings);
    stillDecoderStartSettings.pidChannel = playpumpPidChannel;
    stillDecoderStartSettings.stillPictureReady.callback = complete;
    stillDecoderStartSettings.stillPictureReady.context = stillDecoderEvent;
    stillDecoderStartSettings.codec = (NEXUS_VideoCodec)metadata.videoParam.videoCodec;
    NEXUS_StillDecoder_Start(stillDecoder, &stillDecoderStartSettings);
    bthumbnail_extractor_feed_picture(thumbnailExtractor, options.time);

    rc = BKNI_WaitForEvent(stillDecoderEvent, 2000);

    if (rc) {
        rc = NEXUS_StillDecoder_GetStatus(stillDecoder, &status);
        if (!rc && !status.endCodeFound) {
            BME_DEBUG_ERROR(("Still decode timed out because no end code was found in the ITB"));
        } else if (!rc && !status.stillPictureReady) {
            BME_DEBUG_ERROR(("Still decode timed out because the decoder did not respond"));
        } else {
            BME_DEBUG_ERROR(("Still decode timed out for unknown reasons"));
        }
        BME_DEBUG_EXIT();
        return IMedia::MEDIA_ERROR_IO;
    } else {
        NEXUS_StillDecoder_GetStripedSurface(stillDecoder, &stripedSurface);
        NEXUS_Graphics2D_DestripeToSurface(gfxHandle, stripedSurface,
                                           srcSurface, NULL);
        rc = NEXUS_Graphics2D_Checkpoint(gfxHandle, NULL);
        if (rc == NEXUS_GRAPHICS2D_QUEUED) {
            rc = BKNI_WaitForEvent(checkpointEvent, 2000);
            if (rc) {
                BME_DEBUG_ERROR(("error in %d\n", __LINE__));
            }
        }
    }
    NEXUS_StillDecoder_Stop(stillDecoder);
    NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);
    blitSettings.source.surface = srcSurface;
    blitSettings.output.surface = dstSurface;
    NEXUS_Graphics2D_Blit(gfxHandle, &blitSettings);
    rc = NEXUS_Graphics2D_Checkpoint(gfxHandle, NULL);
    if (rc == NEXUS_GRAPHICS2D_QUEUED) {
        rc = BKNI_WaitForEvent(checkpointEvent, 2000);
        if (rc) {
            BME_DEBUG_ERROR(("Checkpoint event timeout"));
            return IMedia::MEDIA_ERROR_IO;
        }
    }
    NEXUS_Surface_Flush(dstSurface);
    bthumbnail_extractor_stop_playpump(thumbnailExtractor);
    imgPtr = (ImageData*)malloc(sizeof(ImageData));
    imgPtr->width = options.width;
    imgPtr->height = options.height;
    imgPtr->depth = 3;
    ptr = (char*)malloc(imgPtr->width * imgPtr->height * 3); /*24 bits*/
    memset(ptr, 0, imgPtr->width * imgPtr->height * 3);

    imgPtr->data = (char*)dstSurfaceMem.buffer;

    /*need to strip hightest byte "FF" and reverse for jpeg encoder*/
    for (int i = 0; i < (imgPtr->width) * imgPtr->height; i++)
    {
        ptr[i * 3] = imgPtr->data[i * 4 + 2];
        ptr[i * 3 + 1] = imgPtr->data[i * 4 + 1];
        ptr[i * 3 + 2] = imgPtr->data[i * 4];
    }
    imgPtr->data = ptr;

    imageEncode(options.outputFilePath, options.outputFileName);
    free(imgPtr);
    imgPtr = NULL;
    BME_DEBUG_TRACE(("created image: %s%s\n", options.outputFilePath.c_str(), options.outputFileName.c_str()));
    BME_DEBUG_EXIT();
    return IMedia::MEDIA_SUCCESS;
}

ThumbnailImgGen::ThumbnailImgGen()
    : _ctx(NULL)
    , _state(ThumbnailState::IdleState)
{
    BME_DEBUG_ENTER();
    _ctx = new ThumbnailHandles;
    memset(_ctx, 0, sizeof(ThumbnailHandles));
}

ThumbnailImgGen::~ThumbnailImgGen()
{
    BME_DEBUG_ENTER();
    if (_ctx) {
        delete _ctx;
        _ctx = NULL;
    }
    BME_DEBUG_EXIT();
}

ThumbnailOptions ThumbnailImgGen::getThumbnailOptions()
{
    BME_DEBUG_ENTER();
    BME_DEBUG_EXIT();
    return _options;
}

void ThumbnailImgGen::setThumbnailOptions(const ThumbnailOptions& options)
{
    BME_DEBUG_ENTER();
    _options = options;
    _state = ThumbnailState::InitializedState;
    BME_DEBUG_EXIT();
}

void ThumbnailImgGen::waitIntervalTime(unsigned waitTime)
{
    unsigned sleepTime = 0;
    unsigned incrementInterval = 200 * 1000;
    BME_DEBUG_ENTER();
    while (sleepTime/1000 < waitTime) {
        usleep(incrementInterval);
        if (_state != ThumbnailState::GenImageState) {
            break;
        }
        sleepTime += incrementInterval;
    }
    BME_DEBUG_EXIT();
}

void ThumbnailImgGen::genImage()
{
    BME_DEBUG_ENTER();
    IMedia::ErrorType err;
    ThumbnailHandles::PrepareOptions prepareOptions;
    prepareOptions.mediaStream = _options.mediaStream;
    prepareOptions.indexFileName = _options.inIndexFileName;
    prepareOptions.width = _options.width;
    prepareOptions.height = _options.height;

    ThumbnailHandles::CaptureImageOptions captureOptions;
    captureOptions.mediaStream = _options.mediaStream;
    captureOptions.width = _options.width;
    captureOptions.height = _options.height;
    captureOptions.outputFilePath = _options.outputFilePath;
    captureOptions.outputFileName = _options.outputFileName;
    captureOptions.time = _options.time;
    int retryCounter = 0;
    uint32_t startTime = 0;
    uint32_t duration = 0;
    do {
        _ctx->prepare(prepareOptions);
        if (retryCounter == 0) {
            // Only reset start time if this is not a retry
            startTime = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count();
        }
        err = _ctx->captureImage(captureOptions);
        BME_DEBUG_TRACE(("retryCounter = %d\n", retryCounter));
        if (err == IMedia::MEDIA_ERROR_RECORDER_OVERFLOW) {
            retryCounter++;
        } else {
            retryCounter = 0;
        }
        _ctx->release();
        if (retryCounter > NUM_OF_RETRIES) {
            _state = ThumbnailState::ErrorState;
            BME_DEBUG_ERROR(("Thumbnail cannot be generated due to multiple overflows"));
            break;
        }
#ifdef BME_ENABLE_TV
    if (_options.mediaStream->metadata.parserBand) {
        if (err != IMedia::MEDIA_ERROR_RECORDER_OVERFLOW) {
            duration = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count() -
                startTime;
            if (_options.repeatEvery > duration) {
                waitIntervalTime(_options.repeatEvery - duration);
            }
        }
    } else {
#else
    {
#endif
        if (_options.mediaStream->getUri().find(IMedia::FILE_URI_PREFIX) != std::string::npos) {
            captureOptions.time += _options.repeatEvery;
            if  (captureOptions.time > _ctx->metadata.duration) {
                _state = ThumbnailState::StopGenImageState;
            }
        }
    }
    } while (_options.repeatEvery > 0 && _state == ThumbnailState::GenImageState);
    BME_DEBUG_EXIT();
}

void ThumbnailImgGen::startGenImage()
{
    BME_DEBUG_ENTER();
    if (_state != ThumbnailState::InitializedState) {
        BME_DEBUG_ERROR(("Request to start image Gen in invalid State: %d\n", _state));
        return;
    }
    if (_state == ThumbnailState::GenImageState) {
        BME_DEBUG_ERROR(("Image generation already in progress\n"));
        return;
    }
    _state = ThumbnailState::GenImageState;
    _ctx->genImageThread = std::thread(&ThumbnailImgGen::genImage, this);
    BME_DEBUG_EXIT();
}

void ThumbnailImgGen::stopGenImage()
{
    BME_DEBUG_ENTER();
    _state = ThumbnailState::StopGenImageState;
    _ctx->genImageThread.join();
    BME_DEBUG_EXIT();
}

}  // namespace Media
}  // namespace Broadcom
