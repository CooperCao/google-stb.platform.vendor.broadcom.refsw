/***************************************************************************
 *  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 **************************************************************************/
#include "bstd.h"
#include "bflv_decoder.h"
#include "bkni.h"
#include "bkni_multi.h"
#include "libavcodec/avcodec.h"
#include "bflv_parser.h"
#include "bmedia_probe.h"
#include "bfile_stdio.h"
#include "bmedia_probe.h"
#include "bmedia_util.h"
#include "bsoft_decoder.h"
#include "butils_libavcodec.h"

BDBG_MODULE(bflv_decoder);

extern AVCodec vp6f_decoder;
extern AVCodec flv_decoder;

static int
ffmpeg_init(butils_libavcodec *state,bvideo_codec video_codec)
{
    AVCodec *codec;

    avcodec_init();

    switch(video_codec) {
    case bvideo_codec_vp6:
        BDBG_WRN(("VP6"));
        codec = &vp6f_decoder;
        break;
    case bvideo_codec_sorenson_h263:
        BDBG_WRN(("Sorenson H.263"));
        codec = &flv_decoder;
        break;
    default:
        BDBG_WRN(("not supported video codec %u", (unsigned)video_codec));
        return -1;
    }
    butils_libavcodec_init(state, codec, NULL);

    return 0;
}

static void
ffmpeg_shutdown(butils_libavcodec *state)
{
    butils_libavcodec_shutdown(state);
    return;
}


#define B_VIDEO_STREAM  0
#define B_AUDIO_STREAM  1


typedef struct b_flv_reader *b_flv_reader_t;

BDBG_OBJECT_ID(b_flv_reader);
struct b_flv_stream {
    bflv_parser_handler handler;
    b_flv_reader_t flv;
    unsigned stream_id;
    batom_t frame;
};

struct b_flv_reader {
    BDBG_OBJECT(b_flv_reader)
    bfile_io_read_t file;
    batom_factory_t factory;
    bflv_parser_t parser;
    batom_pipe_t pipe;
    bvideo_codec video_codec;
    struct b_flv_stream streams[2];
    uint8_t buf[64*1024];
};

static bflv_parser_action
b_flv_reader_object_handler(bflv_parser_handler *handler, batom_t object, uint8_t meta)
{
    struct b_flv_stream *stream =  (struct b_flv_stream *)handler;
    BDBG_OBJECT_ASSERT(stream->flv, b_flv_reader);
    BSTD_UNUSED(meta);
    BDBG_MSG(("b_flv_reader_object_handler: %#lx frame:%#lx(%u) id:%u", (unsigned long)handler, (unsigned long)object, batom_len(object), (unsigned)handler->tag_type));
    BDBG_ASSERT(stream->frame==NULL);
    stream->frame = object;
    /* batom_dump(object, "frame"); */
    return bflv_parser_action_return;
}

static b_flv_reader_t
b_flv_reader_create(batom_factory_t factory, bfile_io_read_t file, unsigned video, unsigned audio)
{
    b_flv_reader_t flv;
    bflv_parser_cfg cfg;
    unsigned i;

    bmedia_probe_t probe;
    bmedia_probe_config probe_config;
    const bmedia_probe_stream *stream;
    const bmedia_probe_track *track;
    bvideo_codec video_codec=bvideo_codec_unknown;


    probe = bmedia_probe_create();
    if(!probe) {goto err_probe;}
    bmedia_probe_default_cfg(&probe_config);
    probe_config.type = bstream_mpeg_type_flv;
    stream = bmedia_probe_parse(probe, file, &probe_config);
    if(!stream) { goto err_stream; }
    for(track=BLST_SQ_FIRST(&stream->tracks);track;track=BLST_SQ_NEXT(track, link)) {
        if(track->type==bmedia_track_type_video) {
            video_codec = track->info.video.codec;
            break;
        }
    }
    bmedia_probe_stream_free(probe, stream);
err_stream:
    bmedia_probe_destroy(probe);
err_probe:
    if(video_codec==bvideo_codec_unknown) {
        return NULL;
    }
    file->seek(file, 0, SEEK_SET);

    flv = BKNI_Malloc(sizeof(*flv));
    if(!flv) { goto err_alloc;}
    BDBG_OBJECT_INIT(flv, b_flv_reader);
    flv->video_codec = video_codec;
    flv->file = file;
    flv->factory = factory;
    bflv_parser_default_cfg(&cfg);
    flv->parser = bflv_parser_create(factory, &cfg);
    if(!flv->parser) { goto err_parser; }
    flv->pipe = batom_pipe_create(factory);
    if(!flv->pipe) {goto err_pipe;}
    flv->streams[B_VIDEO_STREAM].stream_id = video;
    flv->streams[1].stream_id = audio;
    for(i=0;i<sizeof(flv->streams)/sizeof(*flv->streams);i++) {
        flv->streams[i].flv = flv;
        flv->streams[i].frame = NULL;
        if(flv->streams[i].stream_id) {
            bflv_parser_install_handler(flv->parser, &flv->streams[i].handler, flv->streams[i].stream_id, b_flv_reader_object_handler);
        }
    }

    return flv;

err_pipe:
    bflv_parser_destroy(flv->parser);
err_parser:
    BKNI_Free(flv);
err_alloc:
    return NULL;
}

static void
b_flv_reader_destroy(b_flv_reader_t flv)
{
    unsigned i;
    BDBG_OBJECT_ASSERT(flv, b_flv_reader);

    for(i=0;i<sizeof(flv->streams)/sizeof(*flv->streams);i++) {
        if(flv->streams[i].stream_id) {
            bflv_parser_remove_handler(flv->parser, &flv->streams[i].handler);
        }
    }
    bflv_parser_destroy(flv->parser);
    batom_pipe_destroy(flv->pipe);
    BDBG_OBJECT_DESTROY(flv, b_flv_reader);
    BKNI_Free(flv);
    return;
}

static void
b_atom_free_media(batom_t atom, void *user)
{
    void *ptr = *(void **)user;
    BDBG_MSG(("free_media %#lx:%#lx", (unsigned long)atom, ptr));
    BSTD_UNUSED(atom);
    BKNI_Free(ptr);
    return;
}

static const batom_user b_atom_media = {
    b_atom_free_media,
    sizeof(void **)
};

static int
b_flv_read_data(b_flv_reader_t flv, uint8_t *frame, size_t frame_max_size, struct bsoft_decoder_frame *picture, NEXUS_PlaypumpHandle playpump)
{
    void *buf;
    ssize_t rc;
    batom_t atom;
    batom_cursor cursor;
    size_t data_len;
    const bmedia_packet_header *header;

    for(;;) {

        bflv_parser_feed(flv->parser, flv->pipe);

        if(flv->streams[B_VIDEO_STREAM].frame) {

            header = batom_userdata(flv->streams[B_VIDEO_STREAM].frame);
            BDBG_ASSERT(header);
            picture->timestamp = header->pts;
            batom_cursor_from_atom(&cursor, flv->streams[B_VIDEO_STREAM].frame);
            if(flv->video_codec == bvideo_codec_vp6) {
                batom_cursor_next(&cursor);
            }
            data_len = batom_cursor_copy(&cursor, frame, frame_max_size);
            batom_release(flv->streams[B_VIDEO_STREAM].frame);
            flv->streams[B_VIDEO_STREAM].frame=NULL;
            break;
        }
        if(flv->streams[B_AUDIO_STREAM].frame) {
            batom_cursor_from_atom(&cursor, flv->streams[B_AUDIO_STREAM].frame);
            bmedia_pes_info pes_info;
            uint8_t pes_header[32];
            size_t pes_len;

            header = batom_userdata(flv->streams[B_AUDIO_STREAM].frame);
            BMEDIA_PES_INFO_INIT(&pes_info, 0xC0);
            BMEDIA_PES_SET_PTS(&pes_info, header->pts);
            pes_len = bmedia_pes_header_init(pes_header, batom_len(flv->streams[B_AUDIO_STREAM].frame), &pes_info);
            data_len = 0;

            for(;;) {
                void *buffer;
                size_t buffer_size;
                NEXUS_Error rc;

                rc=NEXUS_Playpump_GetBuffer(playpump, &buffer, &buffer_size);
                if(rc!=NEXUS_SUCCESS) {
                    rc = BERR_TRACE(rc);
                    return -1;
                }
                if(buffer_size==0) {
                    BDBG_WRN(("audio feed sleep"));
                    BKNI_Sleep(30);
                    continue;
                }
                if(pes_len>0) {
                    if(buffer_size>pes_len) {
                        buffer_size = pes_len;
                    }
                    BKNI_Memcpy(buffer, pes_header+data_len, buffer_size);
                    pes_len -= buffer_size;
                    data_len += buffer_size;
                    NEXUS_Playpump_ReadComplete(playpump, 0, buffer_size);
                } else {
                    data_len = batom_cursor_copy(&cursor, buffer, buffer_size);
                    BDBG_MSG(("%u %u %u", data_len, buffer_size, batom_len(flv->streams[B_AUDIO_STREAM].frame)));
                    if(data_len>0) {
                        NEXUS_Playpump_ReadComplete(playpump, 0, data_len);
                    }
                    if(data_len!=buffer_size) {
                        break;
                    }
                }
            }
            batom_release(flv->streams[B_AUDIO_STREAM].frame);
            flv->streams[B_AUDIO_STREAM].frame=NULL;
            continue;
        }
        buf = BKNI_Malloc(48*1024);
        if(!buf) { BDBG_ERR(("err_alloc"));goto err_alloc;}
        rc = flv->file->read(flv->file, buf, 48*1024);
        BDBG_MSG(("read %d %#lx", rc, (unsigned long)buf));
        if(rc<0) { goto err_read;}
        if(rc==0) {
            BKNI_Free(buf);
            data_len = 0;
            break;
        }
        atom = batom_from_range(flv->factory, buf, rc, &b_atom_media, &buf);
        if(!atom) { BDBG_ERR(("err_atom"));goto err_atom; }
        batom_pipe_push(flv->pipe, atom);
    }
    return data_len;

err_atom:
err_read:
    BKNI_Free(buf);
err_alloc:
    BDBG_ERR(("read error"));
    return -1;
}

struct b_softdecoder {
    bsoft_decoder_t decoder;
    NEXUS_VideoImageInputSettings imageInputSettings;
    BKNI_EventHandle decoder_event;
    BKNI_MutexHandle decoder_lock;
    butils_libavcodec *state;
    b_flv_reader_t flv;
    NEXUS_VideoImageInputHandle imageInput;
    NEXUS_StcChannelHandle stcChannel;
    NEXUS_PlaypumpHandle playpump;
    NEXUS_AudioDecoderHandle pcmDecoder;
    bool done;
};

static BERR_Code
b_softdecoder_init(struct b_softdecoder *sd)
{
    BERR_Code rc;
    bsoft_decoder_config sd_config;

    bsoft_decoder_default_config(&sd_config);
    sd->decoder = bsoft_decoder_create(&sd_config);
    BDBG_ASSERT(sd->decoder);

    rc = BKNI_CreateEvent(&sd->decoder_event);
    if(rc!=BERR_SUCCESS) {return BERR_TRACE(rc);}

    rc = BKNI_CreateMutex(&sd->decoder_lock);
    if(rc!=BERR_SUCCESS) {return BERR_TRACE(rc);}

    sd->state = NULL;
    sd->flv = NULL;
    sd->imageInput = NULL;
    sd->stcChannel = NULL;
    sd->playpump = NULL;
    sd->pcmDecoder = NULL;
    sd->done = false;
    return BERR_SUCCESS;
}


static void
b_softdecoder_shutdown(struct b_softdecoder *sd)
{
    bsoft_decoder_destroy(sd->decoder);
    BKNI_DestroyEvent(sd->decoder_event);
    BKNI_DestroyMutex(sd->decoder_lock);
    return ;
}

static void
b_softdecoder_task(void *context)
{
    struct b_softdecoder *sd = context;

    BKNI_AcquireMutex(sd->decoder_lock);
    for(;;) {
        struct bsoft_decoder_frame *frame;
        int rc;
        int frame_len;

        frame = bsoft_decoder_pop_free_frame(sd->decoder);
        BKNI_ReleaseMutex(sd->decoder_lock);
        if(frame==NULL) {
            BKNI_WaitForEvent(sd->decoder_event, BKNI_INFINITE);
            BKNI_AcquireMutex(sd->decoder_lock);
            continue;
        }

        frame_len = b_flv_read_data(sd->flv, sd->flv->buf, sizeof(sd->flv->buf), frame, sd->playpump);
        if(frame_len<=0) {
            BDBG_WRN(("end of data"));
            break;
        }
        /* BDBG_MSG(("decode %#lx:%u:%u [%u]", (unsigned long)frame, sd->decoder_frames, sd->display_frames, frame_len)); */

        rc = butils_libavcodec_decode(sd->state, sd->flv->buf, frame_len, frame);
        BKNI_AcquireMutex(sd->decoder_lock);
        if(rc!=0) {
            break;
        }
        bsoft_decoder_push_decoded_frame(sd->decoder, frame);
    }
    BKNI_ReleaseMutex(sd->decoder_lock);
    sd->done = true;
    return;
}

static void *
b_softdecoder_thread(void *context)
{
    b_softdecoder_task(context);
    return NULL;
}

static void
b_softdecoder_next_frame(struct b_softdecoder *sd)
{
    const struct bsoft_decoder_frame *frame;
    uint32_t stc;
    BKNI_AcquireMutex(sd->decoder_lock);
    NEXUS_StcChannel_GetStc(sd->stcChannel, &stc);
    frame = bsoft_decoder_get_mature_frame(sd->decoder, stc, &sd->imageInputSettings);
    BKNI_ReleaseMutex(sd->decoder_lock);
    BKNI_SetEvent(sd->decoder_event);
    NEXUS_Surface_Flush(frame->surface);
    NEXUS_VideoImageInput_SetSettings(sd->imageInput, &sd->imageInputSettings);
    NEXUS_VideoImageInput_SetSurface(sd->imageInput, frame->surface);
    return;
}

static void
bsoft_decoder_frame_callback(void *context, int _arg1)
{
    struct b_softdecoder *sd = context;
    BSTD_UNUSED(_arg1);
    b_softdecoder_next_frame(sd);
    return;
}

#include <pthread.h>

static NEXUS_Error
b_softdecoder_decode(struct b_softdecoder *sd, butils_libavcodec *state, b_flv_reader_t flv, NEXUS_VideoImageInputHandle imageInput, NEXUS_StcChannelHandle stcChannel, NEXUS_PlaypumpHandle playpump,  NEXUS_AudioDecoderHandle pcmDecoder)
{
    NEXUS_Error rc;
    const struct bsoft_decoder_frame *frame;
    pthread_t thread;

    NEXUS_VideoImageInput_GetDefaultSettings(&sd->imageInputSettings);
    sd->imageInputSettings.imageCallback.callback = bsoft_decoder_frame_callback;
    sd->imageInputSettings.imageCallback.context = sd;
    rc = NEXUS_VideoImageInput_SetSettings(imageInput, &sd->imageInputSettings);
    if(rc!=NEXUS_SUCCESS) {rc=BERR_TRACE(rc);goto err_image_input;}

    sd->state = state;
    sd->flv = flv;
    sd->imageInput = imageInput;
    sd->stcChannel = stcChannel;
    sd->playpump = playpump;
    sd->pcmDecoder = pcmDecoder;
    pthread_create(&thread, NULL, b_softdecoder_thread, sd);
    BKNI_Sleep(200);
    if(stcChannel) {
        NEXUS_StcChannelSettings stcSettings;
        NEXUS_StcChannel_GetSettings(stcChannel, &stcSettings);
        stcSettings.mode = NEXUS_StcChannelMode_eAuto;
        stcSettings.modeSettings.Auto.transportType = NEXUS_TransportType_eMpeg2Pes;
        NEXUS_StcChannel_SetSettings(stcChannel, &stcSettings);

        NEXUS_StcChannel_SetStc(stcChannel, 0);
    }
    BKNI_AcquireMutex(sd->decoder_lock);
    frame = bsoft_decoder_get_display_frame(sd->decoder, &sd->imageInputSettings);
    BKNI_ReleaseMutex(sd->decoder_lock);
    if(frame==NULL) {
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }
    NEXUS_Surface_Flush(frame->surface);
    NEXUS_VideoImageInput_SetSurface(imageInput, frame->surface);
#if 0
    while(!sd->done) {
        BKNI_Sleep(100);
    }
#endif
    return NEXUS_SUCCESS;

err_image_input:
    return rc;
}

BDBG_OBJECT_ID(bflv_decoder);
struct bflv_decoder {
    BDBG_OBJECT(bflv_decoder)
    struct b_softdecoder sd;
    butils_libavcodec ffmpeg;
    b_flv_reader_t reader;
    batom_factory_t factory;
};

void
bflv_decoder_get_default_create_settings(bflv_decoder_create_settings *settings)
{
    BDBG_ASSERT(settings);
    settings->decoder_queue = 16;
    return;
}


bflv_decoder_t
bflv_decoder_create(const bflv_decoder_create_settings *settings)
{
    NEXUS_Error rc;
    bflv_decoder_t  decoder;

    bflv_decoder_create_settings def_settings;
    if(!settings) {
        bflv_decoder_get_default_create_settings(&def_settings);
        settings = &def_settings;
    }
    if(settings->decoder_queue<2) { rc=BERR_TRACE(NEXUS_INVALID_PARAMETER);goto err_settings;}
    decoder = BKNI_Malloc(sizeof(*decoder));
    if(!decoder) { rc=BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);goto err_alloc; }
    BDBG_OBJECT_INIT(decoder, bflv_decoder);
    decoder->reader=NULL;
    decoder->factory = batom_factory_create(bkni_alloc, 64);
    if(!decoder->factory) {rc=BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);goto err_factory;}
    rc = b_softdecoder_init(&decoder->sd);
    if(rc!=NEXUS_SUCCESS) {rc=BERR_TRACE(rc);goto err_softdecoder;}

    return decoder;

err_softdecoder:
    batom_factory_destroy(decoder->factory);
err_factory:
    BKNI_Free(decoder);
err_alloc:
err_settings:
    return NULL;
}

void
bflv_decoder_destroy(bflv_decoder_t decoder)
{
    BDBG_OBJECT_ASSERT(decoder, bflv_decoder);
    b_softdecoder_shutdown(&decoder->sd);
    BDBG_OBJECT_DESTROY(decoder, bflv_decoder);
    BKNI_Free(decoder);
    return;
}

void
bflv_decoder_get_status(bflv_decoder_t decoder, bflv_decoder_status *status)
{
    BDBG_OBJECT_ASSERT(decoder, bflv_decoder);
    status->video.pts_type = NEXUS_PtsType_eInterpolatedFromInvalidPTS;
    status->video.pts = 0;
    status->video.decoder_queue = 0;
    return;
}

void
bflv_decoder_get_default_start_settings(bflv_decoder_start_settings *settings)
{
    BDBG_ASSERT(settings);
    BKNI_Memset(settings, 0, sizeof(*settings));
    return;
}

NEXUS_Error
bflv_decoder_start(bflv_decoder_t decoder, const bflv_decoder_start_settings *settings, bflv_decoder_start_information *information)
{
    NEXUS_Error rc;
    NEXUS_PlaypumpSettings playpumpSettings;

    BDBG_OBJECT_ASSERT(decoder, bflv_decoder);
    BDBG_ASSERT(settings);
    BDBG_ASSERT(information);

    if(settings->fd==NULL || settings->imageInput==NULL || (settings->audioDecoder!=NULL && settings->playpump==NULL)) {
        rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);goto err_settings;
    }
    decoder->reader = b_flv_reader_create(decoder->factory, settings->fd, 9, settings->audioDecoder?8:0);
    if(!decoder->reader) { rc = BERR_TRACE(NEXUS_NOT_SUPPORTED); goto err_reader;}

    rc = ffmpeg_init(&decoder->ffmpeg, decoder->reader->video_codec);
    if(rc!=0) { rc = BERR_TRACE(NEXUS_NOT_SUPPORTED); goto err_ffmpeg;}

    if(settings->playpump) {
        NEXUS_Playpump_GetDefaultSettings(&playpumpSettings);
        NEXUS_Playpump_GetSettings(settings->playpump, &playpumpSettings);
        playpumpSettings.transportType = NEXUS_TransportType_eMpeg2Pes;
        rc = NEXUS_Playpump_SetSettings(settings->playpump, &playpumpSettings);
        if(rc!=NEXUS_SUCCESS) {rc=BERR_TRACE(rc);goto err_playpump_set;}
        rc = NEXUS_Playpump_Start(settings->playpump);
        if(rc!=NEXUS_SUCCESS) {rc=BERR_TRACE(rc);goto err_playpump_start;}
        information->audioPid = NEXUS_Playpump_OpenPidChannel(settings->playpump, 0xC0, NULL);
    }
    rc = b_softdecoder_decode(&decoder->sd, &decoder->ffmpeg, decoder->reader, settings->imageInput, settings->stcChannel, settings->playpump, settings->audioDecoder);
    if(rc!=NEXUS_SUCCESS) { rc = BERR_TRACE(NEXUS_NOT_SUPPORTED); goto err_decode;}

    return NEXUS_SUCCESS;

err_decode:
    if(settings->playpump) {
        NEXUS_Playpump_Stop(settings->playpump);
    }
err_playpump_start:
    if(settings->playpump) {
        NEXUS_Playpump_GetDefaultSettings(&playpumpSettings);
        rc = NEXUS_Playpump_SetSettings(settings->playpump, &playpumpSettings);
        if(rc!=NEXUS_SUCCESS) {rc=BERR_TRACE(rc);}
    }

err_playpump_set:
    ffmpeg_shutdown(&decoder->ffmpeg);
err_ffmpeg:
    b_flv_reader_destroy(decoder->reader);
    decoder->reader = NULL;
err_reader:
err_settings:
    return rc;
}
