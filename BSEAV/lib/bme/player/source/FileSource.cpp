/***************************************************************************
*  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*  See ‘License-BroadcomSTB-CM-Software.txt’ for terms and conditions.
***************************************************************************/
#include "FileSource.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <glob.h>
#include <fstream>
#include <mutex>
#include "nexus_types.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_pid_channel.h"
#include "nexus_playback.h"
#include "nexus_core_utils.h"
#include "bmedia_pcm.h"
#include "bmedia_probe.h"
#include "bmpeg2ts_probe.h"
#include "bmedia_cdxa.h"
#if NEXUS_HAS_PICTURE_DECODER
#include "nexus_picture_decoder.h"
#endif
#include "nexus_surface.h"
#include "nexus_surface_compositor_types.h"
#include "nxclient.h"
#include "nexus_surface_client.h"
#if B_HAS_ASF
#include "basf_probe.h"
#endif
#if B_HAS_AVI
#include "bavi_probe.h"
#endif
#include "bfile_stdio.h"
//#include "b_os_lib.h"
#include "bhevc_video_probe.h"
#include "nexus_file_chunk.h"

namespace Broadcom
{
namespace Media {

TRLS_DBG_MODULE(FileSource);

const int16_t FileSource::_trickModes[] = { -64, -32, -16, -8, -4, -2, 1, 2, 4, 8, 16, 32, 64};
const uint32_t FileSource::_trickModeCount = 13;

#define PICTUER_DECODER_BUFFER_SIZE 128*1024*10

typedef struct {
    NEXUS_VideoCodec nexus;
    bvideo_codec settop;
} FileEngineVideoCodec;

FileEngineVideoCodec g_videoCodec[] = {
    {NEXUS_VideoCodec_eUnknown, bvideo_codec_none},
    {NEXUS_VideoCodec_eUnknown, bvideo_codec_unknown},
    {NEXUS_VideoCodec_eMpeg1, bvideo_codec_mpeg1},
    {NEXUS_VideoCodec_eMpeg2, bvideo_codec_mpeg2},
    {NEXUS_VideoCodec_eMpeg4Part2, bvideo_codec_mpeg4_part2},
    {NEXUS_VideoCodec_eH263, bvideo_codec_h263},
    {NEXUS_VideoCodec_eH264, bvideo_codec_h264},
    {NEXUS_VideoCodec_eH264_Svc, bvideo_codec_h264_svc},
    {NEXUS_VideoCodec_eH264_Mvc, bvideo_codec_h264_mvc},
    {NEXUS_VideoCodec_eVc1, bvideo_codec_vc1},
    {NEXUS_VideoCodec_eVc1SimpleMain, bvideo_codec_vc1_sm},
    {NEXUS_VideoCodec_eDivx311, bvideo_codec_divx_311},
    {NEXUS_VideoCodec_eAvs, bvideo_codec_avs},
    {NEXUS_VideoCodec_eRv40, bvideo_codec_rv40},
    {NEXUS_VideoCodec_eVp6, bvideo_codec_vp6},
    {NEXUS_VideoCodec_eVp8, bvideo_codec_vp8},
    {NEXUS_VideoCodec_eVp9, bvideo_codec_vp9},
    {NEXUS_VideoCodec_eSpark, bvideo_codec_spark},
    {NEXUS_VideoCodec_eH265, bvideo_codec_h265},
};

typedef struct {
    NEXUS_AudioCodec nexus;
    baudio_format settop;
} FileEngineAudioCodec;

FileEngineAudioCodec g_audioCodec[] = {
    {NEXUS_AudioCodec_eUnknown, baudio_format_unknown},
    {NEXUS_AudioCodec_eMpeg, baudio_format_mpeg},
    {NEXUS_AudioCodec_eMp3, baudio_format_mp3},
    {NEXUS_AudioCodec_eAac, baudio_format_aac},
    {NEXUS_AudioCodec_eAacPlus, baudio_format_aac_plus},
    {NEXUS_AudioCodec_eAacPlusAdts, baudio_format_aac_plus_adts},
    {NEXUS_AudioCodec_eAacPlusLoas, baudio_format_aac_plus_loas},
    {NEXUS_AudioCodec_eAc3, baudio_format_ac3},
    {NEXUS_AudioCodec_eAc3Plus, baudio_format_ac3_plus},
    {NEXUS_AudioCodec_eDts, baudio_format_dts},
    {NEXUS_AudioCodec_eLpcmHdDvd, baudio_format_lpcm_hddvd},
    {NEXUS_AudioCodec_eLpcmBluRay, baudio_format_lpcm_bluray},
    {NEXUS_AudioCodec_eDtsHd, baudio_format_dts_hd},
    {NEXUS_AudioCodec_eWmaStd, baudio_format_wma_std},
    {NEXUS_AudioCodec_eWmaPro, baudio_format_wma_pro},
    {NEXUS_AudioCodec_eLpcmDvd, baudio_format_lpcm_dvd},
    {NEXUS_AudioCodec_eAvs, baudio_format_avs},
    {NEXUS_AudioCodec_eAmr, baudio_format_amr},
    {NEXUS_AudioCodec_eDra, baudio_format_dra},
    {NEXUS_AudioCodec_eCook, baudio_format_cook},
    {NEXUS_AudioCodec_ePcmWav, baudio_format_pcm},
    {NEXUS_AudioCodec_eAdpcm, baudio_format_adpcm},
    {NEXUS_AudioCodec_eAdpcm, baudio_format_dvi_adpcm},
    {NEXUS_AudioCodec_eVorbis, baudio_format_vorbis},
    {NEXUS_AudioCodec_eOpus, baudio_format_opus}
};

typedef struct {
    NEXUS_TransportType nexus;
    unsigned settop;
} FileEngineMpegType;

FileEngineMpegType g_mpegType[] = {
    {NEXUS_TransportType_eTs, bstream_mpeg_type_unknown},
    {NEXUS_TransportType_eEs, bstream_mpeg_type_es},
    {NEXUS_TransportType_eTs, bstream_mpeg_type_bes},
    {NEXUS_TransportType_eMpeg2Pes, bstream_mpeg_type_pes},
    {NEXUS_TransportType_eTs, bstream_mpeg_type_ts},
    {NEXUS_TransportType_eDssEs, bstream_mpeg_type_dss_es},
    {NEXUS_TransportType_eDssPes, bstream_mpeg_type_dss_pes},
    {NEXUS_TransportType_eVob, bstream_mpeg_type_vob},
    {NEXUS_TransportType_eAsf, bstream_mpeg_type_asf},
    {NEXUS_TransportType_eAvi, bstream_mpeg_type_avi},
    {NEXUS_TransportType_eMpeg1Ps, bstream_mpeg_type_mpeg1},
    {NEXUS_TransportType_eMp4, bstream_mpeg_type_mp4},
    {NEXUS_TransportType_eMkv, bstream_mpeg_type_mkv},
    {NEXUS_TransportType_eWav, bstream_mpeg_type_wav},
    {NEXUS_TransportType_eMp4Fragment, bstream_mpeg_type_mp4_fragment},
    {NEXUS_TransportType_eRmff, bstream_mpeg_type_rmff},
    {NEXUS_TransportType_eFlv, bstream_mpeg_type_flv},
    {NEXUS_TransportType_eOgg, bstream_mpeg_type_ogg}
};

#define CONVERT(g_struct) \
    unsigned i; \
    for (i = 0; i < sizeof(g_struct)/sizeof(g_struct[0]); i++) { \
        if (g_struct[i].settop == settop_value) { \
            return g_struct[i].nexus; \
        } \
    } \
    printf("unable to find value %d in %s\n", settop_value, #g_struct); \
    return g_struct[0].nexus

NEXUS_VideoCodec b_videocodec2nexus(bvideo_codec settop_value)
{
    CONVERT(g_videoCodec);
}

NEXUS_AudioCodec b_audiocodec2nexus(baudio_format settop_value)
{
    CONVERT(g_audioCodec);
}

NEXUS_TransportType b_mpegtype2nexus(bstream_mpeg_type settop_value)
{
    CONVERT(g_mpegType);
}

static void complete(void *data, int unused)
{
    BSTD_UNUSED(unused);
    BKNI_SetEvent((BKNI_EventHandle)data);
}

struct  picDecoder_t {
#if NEXUS_HAS_PICTURE_DECODER
    NEXUS_PictureDecoderHandle decoder;
    NEXUS_PictureDecoderOpenSettings decoderSettings;
    unsigned multiScanBufferSize;
    NEXUS_PictureDecoderStatus pictureStatus;
#endif
    BKNI_EventHandle displayedEvent;
    NxClient_AllocResults allocResults;
    NEXUS_SurfaceClientHandle blit_client;
};

typedef struct picDecoder_t *picDecoderHandle;

typedef struct tagFileSourceContext {
    NEXUS_TransportType transportType;
    unsigned videoPid, pcrPid, audioPid, extraVideoPid;
    NEXUS_VideoCodec videoCodec;
    NEXUS_VideoCodec extraVideoCodec;
    NEXUS_AudioCodec audioCodec;
    NEXUS_VideoFormat displayFormat;
    NEXUS_TransportTimestampType tsTimestampType;
    NEXUS_VideoDecoderTimestampMode decoderTimestampMode;
    NEXUS_VideoFrameRate videoFrameRate;
    NEXUS_AspectRatio aspectRatio;
    NEXUS_VideoOrientation  displayOrientation;
    struct {
        unsigned x, y;
    } sampleAspectRatio;
    uint16_t videoWidth;
    uint16_t videoHeight;

    bool mad;
    bool compressedAudio;
    bool multichannelAudio;
    bool decodedAudio;
    bool probe;
    bool cdxaFile;
    bool detectAvcExtension;
    bool playpumpTimestampReordering;
    bool pcm;
    // bpcm_file_config pcm_config;
    /* asf wma drc */
#if B_HAS_ASF
    bool dynamicRangeControlValid;
    struct {
        unsigned peakReference;
        unsigned peakTarget;
        unsigned averageReference;
        unsigned averageTarget;
    } dynamicRangeControl;
#endif

    bool stcTrick;
    bool sync;
    bool streamProcessing;
    bool autoBitrate;
    bool closedCaptionEnabled;
    bool avc51;
    unsigned fixedBitrate; /* non-zero */
    NEXUS_StcChannelAutoModeBehavior stcChannelMaster;
    NEXUS_PlaybackLoopMode endOfStreamAction;
    NEXUS_VideoDecoderErrorHandling videoErrorHandling;
    bool customFileIo;
    bool playbackMonitor;
    unsigned maxDecoderRate;

    NEXUS_FilePlayHandle file;
    NEXUS_SimpleVideoDecoderHandle videoDecoderHandle;
    NEXUS_SimpleAudioDecoderHandle audioDecoderHandle;
    NEXUS_SimpleVideoDecoderStartSettings videoProgram;
    NEXUS_SimpleAudioDecoderStartSettings audioProgram;
    NEXUS_PlaypumpHandle playpump;
    NEXUS_PlaybackHandle playback;
    NEXUS_PidChannelHandle videoPidChannel;
    NEXUS_PidChannelHandle audioPidChannel;
    NEXUS_PidChannelHandle pcrPidChannel;
    NEXUS_PidChannelHandle videoExtPidChannel;

    BKNI_EventHandle stateChanged;
    unsigned colorDepth;
    int playbackRate;

    picDecoderHandle picDecoder;
} FileSourceContext;

FileSource::FileSource():
    _context(NULL),
    _state(IMedia::IdleState),
    _initialSeekTimeMSec(0),
    _looping(false),
    _duration(0),
    _resourceAcquired(false),
    _fileType(RecordMode::Linear)
{
}

FileSource::~FileSource()
{
    if (_context) {
        release();
    }
    if (_prepareAsyncThread.joinable()) {
        _prepareAsyncThread.join();
    }
    if (_stopInternalThread.joinable()) {
        _stopInternalThread.join();
    }
}

void FileSource::init()
{
    if (!_context) {
        _context = new FileSourceContext();
        memset(_context, 0, sizeof(*_context));
        _context->playbackRate = 1;
    }
    BKNI_CreateEvent(&_context->stateChanged);
}

void FileSource::uninit()
{
    if (_context) {\
        delete _context;
        _context = NULL;
    }
    _state = IMedia::IdleState;
}

void FileSource::endOfStreamCallback(void *context, int param)
{
    BME_DEBUG_ENTER();
    FileSource* fileSource = static_cast<FileSource*>(context);
    fileSource->onCompletion(*fileSource);
    BME_DEBUG_EXIT();
}

void FileSource::beginningOfStreamCallback(void *context, int param)
{
    BME_DEBUG_ENTER();
    FileSource* fileSource = static_cast<FileSource*>(context);
    fileSource->onBeginning(*fileSource);
    BME_DEBUG_EXIT();
}

SourceConnector* FileSource::getConnector()
{
    return _connector;
}

uint32_t FileSource::setConnector(SourceConnector* connector)
{
    _connector = connector;
    return 0;
}

void FileSource::acquireResources()
{
    NEXUS_PlaypumpOpenSettings playpumpOpenSettings;
    NEXUS_Playpump_GetDefaultOpenSettings(&playpumpOpenSettings);
    _context->playpump = NEXUS_Playpump_Open(NEXUS_ANY_ID, &playpumpOpenSettings);

    if (!_context->playpump)  {
        BME_DEBUG_ERROR(("Unable to create NEXUS Playpump %d"));
        BME_DEBUG_THROW_EXCEPTION(ReturnFailure);
    }

    _context->playback = NEXUS_Playback_Create();

    if (!_context->playback)  {
        BME_DEBUG_ERROR(("Unable to create NEXUS Playback handle"));
        BME_DEBUG_THROW_EXCEPTION(ReturnFailure);
    }

    _resourceAcquired = true;
}

bool FileSource::isImage(const std::string& fileName)
{
    if (fileName.find(".JPG") != std::string::npos ||
            fileName.find(".JPEG") != std::string::npos ||
            fileName.find(".jpg") != std::string::npos ||
            fileName.find(".jpeg") != std::string::npos) {
        return true;
    }
    return false;
}

void FileSource::releasePicDecoder()
{
    BME_DEBUG_ENTER();
#if NEXUS_HAS_PICTURE_DECODER
    if (_context->picDecoder == NULL) {
        return;
    }
    picDecoderHandle picDecoder = _context->picDecoder;

    if (picDecoder->decoder) {
        NEXUS_PictureDecoder_Close(picDecoder->decoder);
    }
    picDecoder->decoder = NULL;

    if (picDecoder->blit_client) {
        NEXUS_SurfaceClient_Clear(picDecoder->blit_client);
        NEXUS_SurfaceClient_Release(picDecoder->blit_client);
    }
    picDecoder->blit_client = NULL;

    if (picDecoder->displayedEvent) {
        BKNI_DestroyEvent(picDecoder->displayedEvent);
    }
    picDecoder->displayedEvent = NULL;
    NxClient_Free(&picDecoder->allocResults);
#endif
    BME_DEBUG_EXIT();
}

IMedia::ErrorType FileSource::acquirePicDecoder()
{
#if NEXUS_HAS_PICTURE_DECODER
    NEXUS_Error rc;
    NxClient_AllocSettings allocSettings;
    NEXUS_SurfaceClientSettings client_settings;
    picDecoderHandle picDecoder;

    BME_DEBUG_ENTER();
    if (_context->picDecoder == NULL) {
        _context->picDecoder = (picDecoderHandle)malloc(sizeof(*_context->picDecoder));
        if (!_context->picDecoder) {
            BME_DEBUG_ERROR(("Out of memory"));
            return IMedia::MEDIA_ERROR_UNKNOWN;
        }
    }
    picDecoder = _context->picDecoder;

    memset(picDecoder, 0, sizeof(*picDecoder));

    BKNI_CreateEvent(&picDecoder->displayedEvent);
    if (picDecoder->displayedEvent == NULL) {
        BME_DEBUG_ERROR(("Failed to create event"));
        goto error;
    }

    NxClient_GetDefaultAllocSettings(&allocSettings);
    allocSettings.surfaceClient = 1;
    rc = NxClient_Alloc(&allocSettings, &picDecoder->allocResults);
    if (rc) {
        BME_DEBUG_ERROR(("NxClient_Alloc failed"));
        goto error;
    }

    /* No NxClient_Connect needed for SurfaceClient */
    picDecoder->blit_client = NEXUS_SurfaceClient_Acquire(picDecoder->allocResults.surfaceClient[0].id);
    if (!picDecoder->blit_client) {
        BME_DEBUG_ERROR(("NEXUS_SurfaceClient_Acquire failed"));
        goto error;
    }

    NEXUS_SurfaceClient_GetSettings(picDecoder->blit_client, &client_settings);
    client_settings.displayed.callback = complete;
    client_settings.displayed.context = picDecoder->displayedEvent;
    rc = NEXUS_SurfaceClient_SetSettings(picDecoder->blit_client, &client_settings);
    BDBG_ASSERT(!rc);

    if (picDecoder->decoder) {
        NEXUS_PictureDecoder_Close(picDecoder->decoder);
        picDecoder ->decoder = NULL;
        BME_DEBUG_WARNING(("reopening SID with %d multiscan buffer", picDecoder->multiScanBufferSize));
    }
    NEXUS_PictureDecoder_GetDefaultOpenSettings(&picDecoder->decoderSettings);
    picDecoder->decoderSettings.bufferSize = PICTUER_DECODER_BUFFER_SIZE;
    picDecoder->decoderSettings.multiScanBufferSize = picDecoder->multiScanBufferSize;
    picDecoder->decoder = NEXUS_PictureDecoder_Open(0, &picDecoder->decoderSettings);
    if (!picDecoder->decoder) {
        if (picDecoder->decoderSettings.multiScanBufferSize) {
            picDecoder->decoderSettings.multiScanBufferSize = 0;
            BME_DEBUG_WARNING(("unable to open SID with %d multiscan buffer. going back to 0.",
                picDecoder->multiScanBufferSize));
            picDecoder->decoder = NEXUS_PictureDecoder_Open(0, &picDecoder->decoderSettings);
        }
        if (!picDecoder->decoder) {
            BME_DEBUG_ERROR(("picture decoder open failed"));
            goto error;
        }
    }
    _resourceAcquired = true;
    BME_DEBUG_EXIT();
    return IMedia::MEDIA_SUCCESS;
#endif

error:
    releasePicDecoder();
    BME_DEBUG_EXIT();
    return IMedia::MEDIA_ERROR_UNKNOWN;
}

void FileSource::startPicDecoder()
{
    BME_DEBUG_ENTER();
#if NEXUS_HAS_PICTURE_DECODER
    int rc = -1;
    uint32_t fileSize;
    void *buffer;
    size_t size;
    uint32_t bytesRemain = 0;
    FILE *fin = NULL;
    bool bStarted = false;
    NEXUS_SurfaceHandle picture = NULL;
    NEXUS_SurfaceCreateSettings createSettings;
    NEXUS_PictureDecoderStartSettings pictureSettings;
    NEXUS_PictureDecoderStatus pictureStatus;
    NEXUS_PictureDecoderHandle pictureDecoder = _context->picDecoder->decoder;

    if (pictureDecoder == NULL) {
        BME_DEBUG_ERROR(("Invalid picture decoder"));
        return;
    }
    fin = fopen(_dataSource.c_str(), "rb");
    if (!fin) {
        BME_DEBUG_ERROR(("failed to open file"));
        goto done;
    }
    /* Determine file size of image to display */
    fseek(fin, 0, SEEK_END);
    fileSize = ftell(fin);
    fseek(fin, 0, SEEK_SET);

    NEXUS_PictureDecoder_GetDefaultStartSettings(&pictureSettings);
    pictureSettings.format = NEXUS_PictureFormat_eJpeg;
    pictureSettings.imageSize = fileSize;
    bytesRemain = fileSize;
    rc = NEXUS_PictureDecoder_GetBuffer(pictureDecoder, &buffer, &size);
    if (size > bytesRemain)
        size = bytesRemain;

    rc = fread(buffer, 1, size, fin);
    if (rc < 0) {
        BME_DEBUG_ERROR(("failed to read file data"));
        goto done;
    }

    rc = NEXUS_PictureDecoder_ReadComplete(pictureDecoder, 0, rc);
    if (rc) {
        BME_DEBUG_ERROR(("picture decoder read completed failed"));
        goto done;
    }

    bytesRemain = bytesRemain - size;
    rc = NEXUS_PictureDecoder_Start(pictureDecoder, &pictureSettings);
    if (rc) {
        BME_DEBUG_ERROR(("picture decoder start failed"));
         goto done;
    }
    bStarted = true;

    do {
        rc = NEXUS_PictureDecoder_GetStatus(pictureDecoder, &pictureStatus);
        BME_DEBUG_TRACE(("pictureDecoder state:%d\n", pictureStatus.state));

        if (pictureStatus.state == NEXUS_PictureDecoderState_eError) {
            BME_DEBUG_ERROR(("picture decode error"));
            goto done;
        } else if (pictureStatus.state == NEXUS_PictureDecoderState_eMoreData) {
            rc = NEXUS_PictureDecoder_GetBuffer(pictureDecoder, &buffer, &size);
            if (size > bytesRemain) size = bytesRemain;

            rc = fread(buffer, 1, size, fin);
            if (rc) {
                NEXUS_PictureDecoder_ReadComplete(pictureDecoder, 0, rc);
                bytesRemain -= size;
            }
        }
        usleep(1000);
    } while (!pictureStatus.headerValid);

    NEXUS_Surface_GetDefaultCreateSettings(&createSettings);
    createSettings.pixelFormat = pictureStatus.header.format;
    createSettings.width = pictureStatus.header.surface.width;
    createSettings.height = pictureStatus.header.surface.height;
    createSettings.alignment = 2;

    NEXUS_PixelFormatInfo pixelFormatInfo;
    NEXUS_PixelFormat_GetInfo(createSettings.pixelFormat, &pixelFormatInfo);
    createSettings.pitch = (createSettings.width*pixelFormatInfo.bpp + 7) / 8;
    createSettings.pitch = ((createSettings.pitch + 3) & ~3);
    BDBG_ASSERT(createSettings.pitch % 4 == 0);

    picture = NEXUS_Surface_Create(&createSettings);
    if (!picture) {
        BME_DEBUG_ERROR(("Surface Create Failed"));
        goto done;
    }

    if (NEXUS_PIXEL_FORMAT_IS_PALETTE(createSettings.pixelFormat)) {
        NEXUS_PictureDecoderPalette sidPalette;
        NEXUS_SurfaceMemory mem;

        rc = NEXUS_PictureDecoder_GetPalette(pictureDecoder, &sidPalette);
        NEXUS_Surface_GetMemory(picture, &mem),
        BKNI_Memcpy(mem.palette, sidPalette.palette, mem.numPaletteEntries*sizeof(NEXUS_PixelFormat));
        NEXUS_Surface_Flush(picture);
    }

    if (fileSize > PICTUER_DECODER_BUFFER_SIZE) {
        BME_DEBUG_TRACE(("File requires multi part read, SID fw requires read from start"));
        fseek(fin, 0, SEEK_SET);
        NEXUS_PictureDecoder_GetBuffer(pictureDecoder, &buffer, &size);
        rc = fread(buffer, 1, size, fin);
        if (rc) {
            NEXUS_PictureDecoder_ReadComplete(pictureDecoder, 0, rc);
        } else {
            BME_DEBUG_ERROR(("Read Complete failed"));
            goto done;
        }
    }

    /* start decoding */
    rc = NEXUS_PictureDecoder_DecodeSegment(pictureDecoder, picture, NULL);
    if (rc) {
        BME_DEBUG_ERROR(("Decode Segment failed"));
        goto done;
    }
    do {
        rc = NEXUS_PictureDecoder_GetStatus(pictureDecoder, &pictureStatus);
        BME_DEBUG_TRACE(("pictureDecoder state:%d\n", pictureStatus.state));

        if (rc) {
            BME_DEBUG_ERROR(("Get Decode Status failed"));
            goto done;
        }

        if (pictureStatus.state == NEXUS_PictureDecoderState_eError) {
            BME_DEBUG_ERROR(("picture decoder decode error"));
            goto done;
        } else if (pictureStatus.state == NEXUS_PictureDecoderState_eMoreData) {
            rc = NEXUS_PictureDecoder_GetBuffer(pictureDecoder, &buffer, &size);
            rc = fread(buffer, 1, size, fin);
            if (size > bytesRemain) size = bytesRemain;

            if (rc) {
                rc = NEXUS_PictureDecoder_ReadComplete(pictureDecoder, 0, rc);
                bytesRemain -= size;
            }  else {
                BME_DEBUG_ERROR(("read data failed"));
                goto done;
            }
        }
        usleep(1000);
    } while (pictureStatus.state != NEXUS_PictureDecoderState_eSegmentDone);
    rc = NEXUS_SurfaceClient_SetSurface(_context->picDecoder->blit_client, picture);
    BDBG_ASSERT(!rc);
    rc = BKNI_WaitForEvent(_context->picDecoder->displayedEvent, 5000);

done:
    if (picture) {
        NEXUS_Surface_Destroy(picture);
        picture = NULL;
    }

    if (fin) {
        fclose(fin);
        fin = NULL;
    }

    if (bStarted) {
        NEXUS_PictureDecoder_Stop(pictureDecoder);
        bStarted = false;
    }
#endif
    BME_DEBUG_EXIT();
}

void FileSource::stopPicDecoder()
{
    BME_DEBUG_ENTER();
#if NEXUS_HAS_PICTURE_DECODER
    NEXUS_SurfaceClient_Clear(_context->picDecoder->blit_client);
#endif
    BME_DEBUG_EXIT();
}

void FileSource::start()
{
    BME_DEBUG_ENTER();
    NEXUS_Error rc;
    NEXUS_PlaybackSettings playbackSettings;
    NEXUS_PlaybackPidChannelSettings playbackPidSettings;

    if (_context->picDecoder) {
        startPicDecoder();
        _state = IMedia::StartedState;
        BME_DEBUG_EXIT();
        return;
    }

    if (_state == IMedia::PausedState) {
        rc = NEXUS_Playback_Play(_context->playback);
        BDBG_ASSERT(!rc);
        _state = IMedia::StartedState;
        return;
    }

    if (_context->playbackRate != 1) {
        setPlaybackRate(std::string("1"));
        return;
    }

    if (!_resourceAcquired) {
        acquireResources();
    }

    NEXUS_Playback_GetSettings(_context->playback, &playbackSettings);
    playbackSettings.playpump = _context->playpump;
    playbackSettings.playpumpSettings.transportType = _context->transportType;
    playbackSettings.playpumpSettings.timestamp.pacing = false;
    playbackSettings.playpumpSettings.timestamp.type = _context->tsTimestampType;
    playbackSettings.startPaused = (_initialSeekTimeMSec > 0) ? true : false;
    playbackSettings.simpleStcChannel = getConnector()->stcChannel;
    playbackSettings.stcTrick = true;

    if (_looping)
        playbackSettings.endOfStreamAction = NEXUS_PlaybackLoopMode_eLoop;
    else
        playbackSettings.endOfStreamAction = NEXUS_PlaybackLoopMode_ePause;

    playbackSettings.endOfStreamCallback.callback = FileSource::endOfStreamCallback;
    playbackSettings.endOfStreamCallback.context = static_cast<void*>(this);
    playbackSettings.beginningOfStreamCallback.callback = FileSource::beginningOfStreamCallback;
    playbackSettings.beginningOfStreamCallback.context = static_cast<void*>(this);

    playbackSettings.enableStreamProcessing = _context->streamProcessing;

    if (_context->audioCodec == NEXUS_AudioCodec_eMp3 &&
            _context->videoPid == 0) {
        /* set to true if application needs to seek in MP3 format,
           it pushes decoder to receive MPEG-2 PES data */
        playbackSettings.enableStreamProcessing = true;
    }

    BME_CHECK(NEXUS_Playback_SetSettings(_context->playback, &playbackSettings));

    /* Open the audio and video pid channels */
    if (_context->videoCodec != NEXUS_VideoCodec_eNone && _context->videoPid != 0) {
        NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
        playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
        playbackPidSettings.pidSettings.allowTimestampReordering =
            _context->playpumpTimestampReordering;
        playbackPidSettings.pidTypeSettings.video.simpleDecoder = getConnector()->videoDecoder;
        playbackPidSettings.pidTypeSettings.video.index = true;
        playbackPidSettings.pidTypeSettings.video.codec = _context->videoCodec;
        _context->videoPidChannel = NEXUS_Playback_OpenPidChannel(
                                        _context->playback,
                                        _context->videoPid,
                                        &playbackPidSettings);
    }

    if (_context->extraVideoCodec != NEXUS_VideoCodec_eNone && _context->extraVideoPid != 0) {
        NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
        playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
        playbackPidSettings.pidTypeSettings.video.index = true;
        playbackPidSettings.pidSettings.allowTimestampReordering =
            _context->playpumpTimestampReordering;
        playbackPidSettings.pidTypeSettings.video.simpleDecoder = getConnector()->videoDecoder;
        playbackPidSettings.pidTypeSettings.video.codec = _context->extraVideoCodec;
        _context->videoExtPidChannel = NEXUS_Playback_OpenPidChannel(
                                           _context->playback,
                                           _context->extraVideoPid,
                                           &playbackPidSettings);
    }

    if (_context->audioCodec != NEXUS_AudioCodec_eUnknown && _context->audioPid != 0) {
        NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
        playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eAudio;
        playbackPidSettings.pidTypeSettings.audio.simpleDecoder = getConnector()->audioDecoder;
        playbackPidSettings.pidSettings.pidTypeSettings.audio.codec = _context->audioCodec;
        _context->audioPidChannel = NEXUS_Playback_OpenPidChannel(
                                        _context->playback,
                                        _context->audioPid,
                                        &playbackPidSettings);
    }

    if (_context->pcrPid && _context->pcrPid != _context->videoPid &&
            _context->pcrPid != _context->audioPid) {
        NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
        playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eOther;
        _context->pcrPidChannel = NEXUS_Playback_OpenPidChannel(
                                      _context->playback,
                                      _context->pcrPid,
                                      &playbackPidSettings);
    }

    if (getConnector()->stcChannel) {
        NEXUS_SimpleStcChannelSettings stcSettings;
        NEXUS_SimpleStcChannel_GetSettings(getConnector()->stcChannel, &stcSettings);

        stcSettings.modeSettings.Auto.transportType = _context->transportType;
        stcSettings.mode = NEXUS_StcChannelMode_eAuto;
        BME_CHECK(NEXUS_SimpleStcChannel_SetSettings(getConnector()->stcChannel,
                   &stcSettings));
    }

    // start playback  */
    BME_CHECK(NEXUS_Playback_Start(_context->playback, _context->file, NULL));

    if (_initialSeekTimeMSec) {
        if (_fileType == RecordMode::ChunkedFile) {
            NEXUS_FilePosition first, last;
            rc = NEXUS_FilePlay_GetBounds(_context->file, &first, &last);
            BDBG_ASSERT(!rc);
            _initialSeekTimeMSec = first.timestamp + _initialSeekTimeMSec;
        }
        if (_initialSeekTimeMSec > _duration) {
            BME_DEBUG_ERROR(("Seeking beyond duration, stopping stream"));
            _stopInternalThread = std::thread(&FileSource::stopInternalThread,
                    static_cast<void*>(this));
            return;
        }
        BME_CHECK(NEXUS_Playback_Seek(_context->playback, _initialSeekTimeMSec));
        BME_CHECK(NEXUS_Playback_Play(_context->playback));
    }

    getConnector()->videoPidChannel = _context->videoPidChannel;
    getConnector()->audioPidChannel = _context->audioPidChannel;
    getConnector()->pcrPidChannel = _context->pcrPidChannel;
    _state = IMedia::StartedState;
    BME_DEBUG_EXIT();
}

void FileSource::stop(bool holdLastFrame)
{
    BME_DEBUG_ENTER();
    if (_context->picDecoder) {
        stopPicDecoder();
        _state = IMedia::StoppedState;
        BME_DEBUG_EXIT();
        return;
    }
    _state = IMedia::StoppedState;
    NEXUS_Playback_Stop(_context->playback);

    if (_context->videoPidChannel) {
        NEXUS_Playback_ClosePidChannel(_context->playback, _context->videoPidChannel);
        _context->videoPidChannel = NULL;
    }

    if (_context->videoExtPidChannel) {
        NEXUS_Playback_ClosePidChannel(_context->playback, _context->videoExtPidChannel);
        _context->videoExtPidChannel = NULL;
    }

    if (_context->audioPidChannel) {
        NEXUS_Playback_ClosePidChannel(_context->playback, _context->audioPidChannel);
        _context->audioPidChannel = NULL;
    }

    if (_context->pcrPidChannel) {
        NEXUS_Playback_ClosePidChannel(_context->playback, _context->pcrPidChannel);
        _context->pcrPidChannel = NULL;
    }

    BME_DEBUG_EXIT();
}

void FileSource::pause()
{
    BME_DEBUG_ENTER();
    if (!_context->picDecoder) {
        int rc;
        rc = NEXUS_Playback_Pause(_context->playback);
        BDBG_ASSERT(!rc);
    }
    _state = IMedia::PausedState;
    BME_DEBUG_EXIT();
}

static int globError(const char *epath, int eerrno)
{
    BME_DEBUG_PRINT(("glob_errfunc %s %d", epath, eerrno));
    return -1;
}


static int detectChunk(const char *fileName, off_t *chunkSize, unsigned *firstChunkNumber)
{
    std::string path(fileName);
    path.append("_*");
    int rc;
    glob_t glb;

    rc = glob(path.c_str(), 0, globError, &glb);
    if (rc) {
        BME_DEBUG_ERROR(("no chunk files found with name %s", fileName));
        return -1;
    }
    if (glb.gl_pathc) {
        unsigned fileNameLen = strlen(fileName);
        unsigned len = strlen(glb.gl_pathv[0]);
        struct stat st;
        unsigned n;

        if (len <= fileNameLen) {
            goto done;
        }

        n = atoi(&glb.gl_pathv[0][fileNameLen+1]);
        *firstChunkNumber = ((n/1000)*64) + n%1000;

        rc = stat(glb.gl_pathv[0], &st);
        if (rc) {goto done;}
        *chunkSize = st.st_size;
        BME_DEBUG_PRINT(("found file %s and results %d %d\n", glb.gl_pathv[0], (unsigned)*chunkSize, *firstChunkNumber));
        rc = 0;
    }
done:
    globfree(&glb);
    return rc;
}

IMedia::ErrorType FileSource::prepare()
{
    BME_DEBUG_ENTER();
    int audioParamIndex = 0;
    BME_DEBUG_TRACE(("File: %s", _dataSource.c_str()));

    if (isImage(_dataSource)) {
        return acquirePicDecoder();
    }

    bool fileCanIndex;
    if (_context->videoPid == 0) {
        if (_fileType == RecordMode::ChunkedFile) {
            std::string file(_dataSource+"_*");
            glob_t glb;
            int rc = 0;
            rc = glob(file.c_str(), 0, globError, &glb);
            if (rc) {
                BME_DEBUG_ERROR(("no chunk files found"));
                return IMedia::MEDIA_ERROR_UNKNOWN;
            }
            if (glb.gl_pathc) {
                probe(glb.gl_pathv[0], "", &fileCanIndex);
            }
        } else {
            probe(_dataSource, "", &fileCanIndex);
        }
    }
    if (fileCanIndex) {
        //Opening chunkfile in probe
        if (_fileType != RecordMode::ChunkedFile) {
            _context->file = NEXUS_FilePlay_OpenPosix(_dataSource.c_str(), _dataSource.c_str());
        }
    } else {
        _context->file = NEXUS_FilePlay_OpenPosix(_dataSource.c_str(), NULL);
    }

    if (!_context->file) {
        BME_DEBUG_ERROR(("can't open files:%s\n", _dataSource.c_str()));
        return IMedia::MEDIA_ERROR_IO;
    }
    if (_preferredAudioPid) {
        audioParamIndex = _metadata.getAudioParamIndex(_preferredAudioPid);
        if (audioParamIndex == -1) {
            audioParamIndex = 0; // pick first
            BME_DEBUG_ERROR(("Audio pid: %d, not found\n", _preferredAudioPid));
        }
        IMedia::AudioParameters audioParam;
        audioParam = _metadata.audioParamList[audioParamIndex];
        _context->audioPid = audioParam.streamId;
        BME_DEBUG_TRACE(("audiopid selected: %d\n", audioParam.streamId));
        _context->audioCodec = (NEXUS_AudioCodec)audioParam.audioCodec;
    }
    _state = IMedia::StoppedState;
    BME_DEBUG_EXIT();

    return IMedia::MEDIA_SUCCESS;
}

/* updateChunkFileDuration() method is responsible to open the chunk file and update duration for chunk files
   currently bmedia_probe_parse() method takes first chunk details and returns duration of first chunk */
void FileSource::updateChunkFileDuration()
{
    NEXUS_ChunkedFilePlayOpenSettings chunkedFilePlayOpenSettings;
    const char *fileName = _dataSource.c_str();

    std::string indName = _dataSource;
    indName.replace(_dataSource.length() - 3, 3, "nav");
    const char *indexName = indName.c_str();

    NEXUS_ChunkedFilePlay_GetDefaultOpenSettings(&chunkedFilePlayOpenSettings);

    detectChunk(fileName, &chunkedFilePlayOpenSettings.chunkSize, &chunkedFilePlayOpenSettings.firstChunkNumber);
    strcpy(chunkedFilePlayOpenSettings.chunkTemplate, "%s_%04u");
     _context->file = NEXUS_ChunkedFilePlay_Open(fileName, indexName, &chunkedFilePlayOpenSettings);
    if (!_context->file) {
        BME_DEBUG_PRINT(("can't open file:%s", fileName));
    }
    NEXUS_FilePosition first, last;
    NEXUS_FilePlay_GetBounds(_context->file, &first, &last);
    _duration =last.timestamp - first.timestamp;
}

void FileSource::probe(const std::string& filename,
                       const std::string& indexname, bool* fileCanIndex)
{
    /* use media probe to set values */
    TRLS_UNUSED(indexname);
    bmedia_probe_t probe = NULL;
    bmedia_probe_config probe_config;
    const bmedia_probe_stream *stream = NULL;
    const bmedia_probe_track *track = NULL;
    bfile_io_read_t fd = NULL;
    bool foundAudio = false, foundVideo = false;
    FILE *fin;
    char stream_info[512];
    probe = bmedia_probe_create();
    _context->videoCodec = NEXUS_VideoCodec_eUnknown;
    _context->audioCodec = NEXUS_AudioCodec_eUnknown;
    fin = fopen64(filename.c_str(), "rb");

    if (!fin) {
        BME_DEBUG_ERROR(("can't open media file '%s' for probing\n", filename.c_str()));
        goto done;
    }

    fd = bfile_stdio_read_attach(fin);
    // TODO(DLiu): Support LPCM files
    //    if(_context->pcm) {
    //        pcm_file = bpcm_file_create(fd, &_context->pcm_config);
    //        BDBG_ASSERT(pcm_file);
    //    }
    bmedia_probe_default_cfg(&probe_config);
    probe_config.file_name = filename.c_str();
    probe_config.type = bstream_mpeg_type_unknown;
    stream = bmedia_probe_parse(probe, fd, &probe_config);

    if (stream && stream->type == bstream_mpeg_type_cdxa) {
        bcdxa_file_t cdxa_file;
        bmedia_stream_to_string(stream, stream_info, sizeof(stream_info));
        BME_DEBUG_PRINT(("Media Probe:\n" "%s\n\n", stream_info));
        cdxa_file = bcdxa_file_create(fd);

        if (cdxa_file) {
            const bmedia_probe_stream *cdxa_stream;
            cdxa_stream = bmedia_probe_parse(probe,
                                             bcdxa_file_get_file_interface(cdxa_file), &probe_config);
            bcdxa_file_destroy(cdxa_file);

            if (cdxa_stream) {
                bmedia_probe_stream_free(probe, stream);
                stream = cdxa_stream;
                _context->cdxaFile = true;
            }
        }
    }

    /* now stream is either NULL, or stream descriptor with linked list of audio/video tracks */
    bfile_stdio_read_detach(fd);
    fclose(fin);

    if (!stream) {
        BME_DEBUG_ERROR(("media probe can't parse stream '%s'\n", filename.c_str()));
        goto done;
    }

    /* if the user has specified the index, don't override */
    /*
    if (indexname && !*indexname) {
        if (stream->index == bmedia_probe_index_available || stream->index == bmedia_probe_index_required) {
        //    *indexname = filename;
        }
    }
    */
    bmedia_stream_to_string(stream, stream_info, sizeof(stream_info));
    BME_DEBUG_TRACE(("-- Media Probe: %s\n", stream_info));
    *fileCanIndex = false;
    if (stream->index == bmedia_probe_index_available ||
            stream->index == bmedia_probe_index_self ||
            stream->index == bmedia_probe_index_required) {
        *fileCanIndex = true;
    }

    if ((*fileCanIndex = true) && (_fileType == RecordMode::ChunkedFile)) {
        updateChunkFileDuration();
    } else {
        _duration = stream->duration;
    }
    _context->transportType = b_mpegtype2nexus(stream->type);

    if (stream->type == bstream_mpeg_type_ts) {
        if ((((bmpeg2ts_probe_stream*)stream)->pkt_len) == 192) {
            if (_context->tsTimestampType == NEXUS_TransportTimestampType_eNone) {
                _context->tsTimestampType = NEXUS_TransportTimestampType_eMod300;
            }
        }
    }

    _metadata.reset();
    _metadata.streamType = (IMedia::StreamType)_context->transportType;
    _metadata.duration = _duration;
    for (track = BLST_SQ_FIRST(&stream->tracks); track; track = BLST_SQ_NEXT(track, link)) {
        // Right now we support one video stream
        if ((track->type == bmedia_track_type_video) && (foundVideo))
            break;

        switch (track->type) {
        case bmedia_track_type_audio: {
            if (track->info.audio.codec != baudio_format_unknown && !foundAudio) {
                _context->audioPid = track->number;
                _context->audioCodec = b_audiocodec2nexus(track->info.audio.codec);
                foundAudio = true;
            }

            IMedia::AudioParameters audioParam = {(uint16_t)track->number, 0,
                    (IMedia::AudioCodec)b_audiocodec2nexus(track->info.audio.codec), 0, 0, 0};
            _metadata.audioParamList.push_back(audioParam);
            break;
        }

        case bmedia_track_type_video:
            if (track->info.video.codec == bvideo_codec_h264_svc ||
                    track->info.video.codec == bvideo_codec_h264_mvc) {
                if (_context->detectAvcExtension) {
                    _context->extraVideoPid = track->number;
                    _context->extraVideoCodec = b_videocodec2nexus(track->info.video.codec);
                }

                break;
            } else if (track->info.video.codec != bvideo_codec_unknown && !foundVideo) {
                _context->videoPid = track->number;
                _context->videoCodec = b_videocodec2nexus(track->info.video.codec);
                _context->videoHeight = track->info.video.height;
                _context->videoWidth = track->info.video.width;

                _metadata.videoParam.streamId = _context->videoPid;
                _metadata.videoParam.videoCodec = (IMedia::VideoCodec)(_context->videoCodec);
                _metadata.videoParam.maxHeight = _context->videoHeight;
                _metadata.videoParam.maxWidth = _context->videoWidth;
                foundVideo = true;

                /* timestamp reordering can be done at the host or decoder.
                   to do it at the decoder, disable it at the host and use media_probe to
                   determine the correct decoder timestamp mode */
                if (_context->playpumpTimestampReordering == false) {
                    _context->decoderTimestampMode =
                        (NEXUS_VideoDecoderTimestampMode)track->info.video.timestamp_order;
                }

#if B_HAS_ASF

                if (stream->type == bstream_mpeg_type_asf) {
                    basf_probe_track *asf_track = (basf_probe_track *)track;

                    if (asf_track->aspectRatioValid) {
                        _context->aspectRatio = NEXUS_AspectRatio_eSar;
                        _context->sampleAspectRatio.x = asf_track->aspectRatio.x;
                        _context->sampleAspectRatio.y = asf_track->aspectRatio.y;
                    }

                    if (asf_track->dynamicRangeControlValid) {
                        _context->dynamicRangeControlValid = true;
                        _context->dynamicRangeControl.peakReference =
                            asf_track->dynamicRangeControl.peakReference;
                        _context->dynamicRangeControl.peakTarget =
                            asf_track->dynamicRangeControl.peakTarget;
                        _context->dynamicRangeControl.averageReference =
                            asf_track->dynamicRangeControl.averageReference;
                        _context->dynamicRangeControl.averageTarget =
                            asf_track->dynamicRangeControl.averageTarget;
                    }
                }

#endif
            }

            if (track->info.video.codec == bvideo_codec_h265) {
                _context->colorDepth =
                    ((bmedia_probe_h265_video*)&track->info.video.codec_specific)->sps.bit_depth_luma;
            } else {
                _context->colorDepth = 8;
            }
            _metadata.videoParam.colorDepth = _context->colorDepth;
            BME_DEBUG_TRACE(("color depth is: %d\n", _context->colorDepth));
            break;

        case bmedia_track_type_pcr:
            _context->pcrPid = track->number;
            _metadata.videoParam.substreamId = _context->pcrPid;
            break;

        default:
            break;
        }
    }

#if B_HAS_AVI

    if (stream->type == bstream_mpeg_type_avi && ((bavi_probe_stream *)stream)->video_framerate &&
            _context->videoFrameRate == 0) {
        NEXUS_LookupFrameRate(((bavi_probe_stream *)stream)->video_framerate,
                              &_context->videoFrameRate);
    }

#endif

done:
    if (probe) {
        if (stream) {
            bmedia_probe_stream_free(probe, stream);
        }
        bmedia_probe_destroy(probe);
    }
}

void FileSource::prepareAsyncThread(void* data)
{
    BME_DEBUG_ENTER();
    FileSource* fileSource = static_cast<FileSource*>(data);
    FileSourceContext* context = fileSource->getContext();
    bool fileCanIndex;
    fileSource->probe(fileSource->getDataSource(), "", &fileCanIndex);
    if (fileCanIndex)
        context->file = NEXUS_FilePlay_OpenPosix(fileSource->getDataSource().c_str(),
                        fileSource->getDataSource().c_str());
    else
        context->file = NEXUS_FilePlay_OpenPosix(fileSource->getDataSource().c_str(), NULL);

    if (!context->file) {
        fileSource->onError(*fileSource, IMedia::MEDIA_ERROR_IO);
        BME_DEBUG_EXIT();
        return;
    }

    // make sure state didn't change while we are putting in fixed delay
    NEXUS_Error rc = BKNI_WaitForEvent(context->stateChanged, 100);

    if (BERR_TIMEOUT == rc) {
        fileSource->onPrepared(*fileSource);
    }

    BME_DEBUG_EXIT();
}

void FileSource::prepareAsync()
{
    BME_DEBUG_ENTER();
    BKNI_ResetEvent(_context->stateChanged);
    _state = IMedia::PreparingState;
    _prepareAsyncThread = std::thread(&FileSource::prepareAsyncThread,
            static_cast<void*>(this));
    BME_DEBUG_EXIT();
}

void FileSource::setDataSource(MediaStream *mediaStream)
{
    BME_DEBUG_ENTER();
    std::string fileHeader(IMedia::FILE_URI_PREFIX);
    std::string sourceUri = mediaStream->getUri();
    _dataSource = sourceUri.substr(fileHeader.length());
    // Check if this is a recorded file by checking if
    // an associating metadata file exists (ie filename.json)
#if 0
    std::string metadataFilename = _dataSource.substr(0,
        _dataSource.find_last_of(".")) + ".json";
    std::ifstream inputStream(metadataFilename);
    if (inputStream.good()) {
        std::stringstream strStream;
        Json::Value root;
        Json::Reader reader;

        strStream << inputStream.rdbuf();
        bool result = reader.parse(strStream.str(), root);
        if (!result) {
           BME_DEBUG_ERROR(("Invalid JSON file %s\n",
                reader.getFormattedErrorMessages().c_str()));
        }
        _fileType = static_cast<RecordMode>(root.get("mode", 0).asInt());
    } else {
        // Metadata does not exist, assume linear non-chunked file
        _fileType = RecordMode::Linear;
    }
#else
    _fileType = RecordMode::Linear;
#endif

    init();
    _preferredAudioPid = mediaStream->getPreferredAudioPid();
    setStreamMetadata(mediaStream->metadata);
    BME_DEBUG_EXIT();
}

void FileSource::reset()
{
    releaseResources();
    uninit();
}

void FileSource::release()
{
    releaseResources();
    uninit();
}

void FileSource::releaseResources()
{
    BME_DEBUG_ENTER();

    if (false == _resourceAcquired)
        return;

#if NEXUS_HAS_PICTURE_DECODER
    releasePicDecoder();
#endif

    if (_context->file) {
        NEXUS_FilePlay_Close(_context->file);
        _context->file = NULL;
    }

    if (_context->playback) {
        NEXUS_Playback_Destroy(_context->playback);
        _context->playback = NULL;
    }

    if (_context->playpump) {
        NEXUS_Playpump_Close(_context->playpump);
        _context->playpump = NULL;
    }

    _resourceAcquired = false;
    BME_DEBUG_EXIT();
}

bool FileSource::checkUrlSupport(const std::string& url)
{
    // FIXME
    return false;
}

std::string FileSource::getType()
{
    return SOURCE_FILE;
}

void FileSource::setStreamMetadata(const IMedia::StreamMetadata& metaData)
{
    if (metaData.streamType != IMedia::AutoStreamType) {
        _metadata = metaData;

        IMedia::VideoParameters videoParam;
        videoParam = _metadata.videoParam;
        IMedia::AudioParameters audioParam;
        audioParam = _metadata.audioParamList[0];

        _context->transportType = (NEXUS_TransportType)_metadata.streamType;
        _context->videoPid = videoParam.streamId;
        _context->videoCodec = (NEXUS_VideoCodec)videoParam.videoCodec;
        _context->extraVideoPid = 0;
        _context->extraVideoCodec = NEXUS_VideoCodec_eUnknown;
        _context->pcrPid = videoParam.substreamId;
        _context->videoHeight = videoParam.maxHeight;
        _context->videoWidth = videoParam.maxWidth;
        if (videoParam.colorDepth) {
            _context->colorDepth = videoParam.colorDepth;
        } else {
            _context->colorDepth = 8;
        }
        BME_DEBUG_TRACE(("color depth is: %d\n", _context->colorDepth));
        BME_DEBUG_TRACE(("width: %d, height: %d\n", _context->videoWidth, _context->videoHeight));

        _context->audioPid = audioParam.streamId;
        _context->audioCodec = (NEXUS_AudioCodec)audioParam.audioCodec;
    }
}

IMedia::StreamMetadata FileSource::getStreamMetadata()
{
    return _metadata;
}

void FileSource::stopInternalThread(void* data)
{
    FileSource* source = static_cast<FileSource*>(data);
    source->onCompletion(*source);
}

void FileSource::flush()
{
    if (_context->playpump) {
        NEXUS_Playpump_Flush(_context->playpump);
    }
    if (getConnector()->stcChannel) {
        NEXUS_SimpleStcChannel_Invalidate(getConnector()->stcChannel);
    }
    if (getConnector()->videoDecoder) {
        NEXUS_SimpleVideoDecoder_Flush(getConnector()->videoDecoder);
    }
    if (getConnector()->audioDecoder) {
        NEXUS_SimpleAudioDecoder_Flush(getConnector()->audioDecoder);
    }
}

IMedia::ErrorType FileSource::seekTo(const uint32_t& milliseconds,
                        IMedia::PlaybackOperation playOperation,
                        IMedia::PlaybackSeekTime  playSeekTime)

{
    BME_DEBUG_ENTER();
    NEXUS_Error rc;
    unsigned long position = milliseconds;

    if (_state == IMedia::PreparingState ||
        _state == IMedia::StoppedState   ||
        _state == IMedia::IdleState) {
        _initialSeekTimeMSec = position;
        return IMedia::MEDIA_ERROR_INVALID_STATE;
    }

    if (_fileType == RecordMode::ChunkedFile) {
        NEXUS_FilePosition first, last;
        rc = NEXUS_FilePlay_GetBounds(_context->file, &first, &last);
        BDBG_ASSERT(!rc);
        position = first.timestamp + milliseconds;
    }

    if (position > _duration) {
        BME_DEBUG_ERROR(("Seeking beyond duration, stopping stream"));
        _stopInternalThread = std::thread(&FileSource::stopInternalThread,
            static_cast<void*>(this));
        return IMedia::MEDIA_ERROR_ARG_OUT_OF_RANGE;
    }

    rc = NEXUS_Playback_Seek(_context->playback, position);
    BDBG_ASSERT(!rc);
    return IMedia::MEDIA_SUCCESS;
    BME_DEBUG_EXIT();
}

void FileSource::setPlaybackRate(const std::string& rate)
{
    BME_DEBUG_ENTER();

    // No trickmode without video
    if (!_context->videoPid)
        return;

    int playbackRate = atoi(rate.c_str());

    if (_context->playbackRate == playbackRate) {
        return;
    }

    NEXUS_PlaybackTrickModeSettings trickSettings;
    NEXUS_Playback_GetDefaultTrickModeSettings(&trickSettings);
    trickSettings.rate = NEXUS_NORMAL_DECODE_RATE * playbackRate;

    NEXUS_Error rc;
    rc = NEXUS_Playback_TrickMode(_context->playback, &trickSettings);
    if (rc) {
        BME_DEBUG_ERROR(("Unable to start trick mode rc = %d\n", rc));
        BME_DEBUG_THROW_EXCEPTION(ReturnFailure);
    }
    _context->playbackRate = playbackRate;
    BME_DEBUG_EXIT();
}

int FileSource::getPlaybackRate()
{
    return 0;
}

static std::string inttostring(const int u)
{
    std::stringstream ss;
    ss << u;
    return ss.str();
}

std::string FileSource::getAvailablePlaybackRate()
{
    if (!_playbackRates.empty())
        return _playbackRates;

    for (unsigned i = 0; i < _trickModeCount; i++) {
        _playbackRates += inttostring((int16_t)_trickModes[i]);
        _playbackRates += ",";
    }

    // remove last comma to avoid parsing error
    if (_playbackRates.find_last_of(',') != std::string::npos) {
        _playbackRates.erase(_playbackRates.find_last_of(','));
    }
    return _playbackRates;
}

IMedia::PlaybackOperation FileSource::getPlaybackOperation()
{
    return IMedia::OperationForward;
}

uint64_t FileSource::getDuration()
{
    return _duration;
}

uint32_t FileSource::getCurrentPosition()
{
    NEXUS_PlaybackStatus status;
    if (_context->playback) {
        BME_CHECK(NEXUS_Playback_GetStatus(_context->playback, &status));
    } else {
        status.position = 0;
    }
    return status.position;
}

uint64_t FileSource::getBytesDecoded()
{
    NEXUS_PlaybackStatus status;
    if (_context->playback) {
        BME_CHECK(NEXUS_Playback_GetStatus(_context->playback, &status));
    } else {
        status.bytesPlayed = 0;
    }
    return status.bytesPlayed;
}

const IMedia::TimeInfo FileSource::getTimeInfo()
{
    IMedia::TimeInfo timeinfo;
    return timeinfo;
}

Connector FileSource::connect(const ConnectSettings& settings)
{
    NEXUS_PidChannelHandle pidChannel;
    pidChannel = NEXUS_Playback_OpenPidChannel(_context->playback,
                                            settings.streamId, NULL);
    return reinterpret_cast<Connector>(pidChannel);
}

void FileSource::disconnect(const Connector& connector)
{
    NEXUS_PidChannelHandle pidChannel;
    pidChannel = reinterpret_cast<NEXUS_PidChannelHandle>(connector);
    NEXUS_Playback_ClosePidChannel(_context->playback, pidChannel);
}

void FileSource::setLooping(bool looping)
{
    BME_DEBUG_ENTER();
    _looping = looping;

    if (_context->playback) {
        NEXUS_PlaybackSettings playbackSettings;
        NEXUS_Playback_GetSettings(_context->playback, &playbackSettings);

        if (_looping)
            playbackSettings.endOfStreamAction = NEXUS_PlaybackLoopMode_eLoop;
        else
            playbackSettings.endOfStreamAction = NEXUS_PlaybackLoopMode_ePause;

        NEXUS_Playback_SetSettings(_context->playback, &playbackSettings);
    }
    BME_DEBUG_EXIT();
}

void FileSource::onError(FileSource&, const IMedia::ErrorType& errorType)
{
    notify(SourceEvents::Error, errorType);
}

void FileSource::onPrepared(FileSource&)
{
    _state = IMedia::StoppedState;
    notify(SourceEvents::Prepared);
}

void FileSource::onCompletion(FileSource&)
{
    notify(SourceEvents::Completed);
}

void FileSource::onBeginning(FileSource&)
{
    BME_DEBUG_ENTER();
    notify(SourceEvents::BeginningOfStream);
    BME_DEBUG_EXIT();
}

std::string FileSource::getDataSource()
{
    return _dataSource;
}

}  // namespace Media
}  // namespace Broadcom
