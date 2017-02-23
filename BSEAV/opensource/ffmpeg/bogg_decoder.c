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
#include "bogg_decoder.h"
#include "bkni.h"
#include "bkni_multi.h"
#include "bsoft_decoder.h"
#include "butils_libavcodec.h"
#include "libavformat/avformat.h"
#include <pthread.h>

BDBG_MODULE(bogg_decoder);
BDBG_OBJECT_ID(bogg_decoder);

struct bogg_decoder {
    BDBG_OBJECT(bogg_decoder)
    butils_libavcodec ffmpeg;
    ByteIOContext *data_source;
    AVFormatContext *demux;
    int videoStream;
    AVRational timeBase;
    bool startedDisplay;
    pthread_t thread;
    BKNI_EventHandle decoder_event;
    BKNI_MutexHandle decoder_lock;
    bsoft_decoder_t decoder;
    bogg_decoder_start_settings start_settings;
    NEXUS_VideoImageInputSettings imageInputSettings;
};


void
bogg_decoder_get_default_create_settings(bogg_decoder_create_settings *settings)
{
    BDBG_ASSERT(settings);
    settings->decoder_queue = 16;
    return;
}

bogg_decoder_t
bogg_decoder_create(const bogg_decoder_create_settings *settings)
{
    NEXUS_Error rc;
    bogg_decoder_t  decoder;
    bsoft_decoder_config sd_config;

    bogg_decoder_create_settings def_settings;
    if(!settings) {
        bogg_decoder_get_default_create_settings(&def_settings);
        settings = &def_settings;
    }
    if(settings->decoder_queue<2) { rc=BERR_TRACE(NEXUS_INVALID_PARAMETER);goto err_settings;}
    decoder = BKNI_Malloc(sizeof(*decoder));
    if(!decoder) { rc=BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);goto err_alloc; }
    BDBG_OBJECT_INIT(decoder, bogg_decoder);

    rc = BKNI_CreateEvent(&decoder->decoder_event);
    if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc); goto err_event;}

    rc = BKNI_CreateMutex(&decoder->decoder_lock);
    if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc); goto err_mutex;}


    bsoft_decoder_default_config(&sd_config);
    decoder->decoder = bsoft_decoder_create(&sd_config);
    if(decoder->decoder==NULL) {rc=BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);goto err_decoder; }

    return decoder;

err_decoder:
err_mutex:
err_event:
err_alloc:
err_settings:
    return NULL;
}

void
bogg_decoder_get_default_start_settings(bogg_decoder_start_settings *settings)
{
    BDBG_ASSERT(settings);
    BKNI_Memset(settings, 0, sizeof(*settings));
    return;
}

extern AVInputFormat ogg_demuxer;
extern URLProtocol file_protocol;
extern AVCodec theora_decoder;

static void *
b_oggdecoder_thread(void *context)
{
    bogg_decoder_t ogg = context;
    BDBG_OBJECT_ASSERT(ogg, bogg_decoder);

    for(;;) {
        AVPacket packet;
        int result;
        NEXUS_Error rc;
        uint32_t pts;

        result = av_read_frame(ogg->demux, &packet);

        if(result<0) { rc = BERR_TRACE(NEXUS_NOT_SUPPORTED);goto err_data; }
        pts = (packet.pts * ogg->timeBase.num * (uint64_t)45000) / ogg->timeBase.den;

        BDBG_MSG((" stream %u -> %u %#x PTS:%u(%u)", packet.stream_index, packet.size, packet.data, (unsigned)packet.pts, pts));
        if(packet.stream_index == ogg->videoStream) {
            struct bsoft_decoder_frame *frame;

            for(;;) {
                BKNI_AcquireMutex(ogg->decoder_lock);
                frame = bsoft_decoder_pop_free_frame(ogg->decoder);
                if(frame) {
                    BKNI_ReleaseMutex(ogg->decoder_lock);
                    frame->timestamp = pts;
                    break;
                }
                if(!ogg->startedDisplay) {
                    const struct bsoft_decoder_frame *display_frame;
                    NEXUS_StcChannelSettings stcSettings;
                    display_frame = bsoft_decoder_get_display_frame(ogg->decoder, &ogg->imageInputSettings);
                    BDBG_ASSERT(display_frame);
                    ogg->startedDisplay = true;
                    NEXUS_Surface_Flush(display_frame->surface);
                    NEXUS_VideoImageInput_SetSurface(ogg->start_settings.imageInput, display_frame->surface);

                    NEXUS_StcChannel_GetSettings(ogg->start_settings.stcChannel, &stcSettings);
                    stcSettings.mode = NEXUS_StcChannelMode_eAuto;
                    stcSettings.modeSettings.Auto.transportType = NEXUS_TransportType_eMpeg2Pes;
                    NEXUS_StcChannel_SetSettings(ogg->start_settings.stcChannel, &stcSettings);

                    NEXUS_StcChannel_SetStc(ogg->start_settings.stcChannel, 0);
                }
                BKNI_ReleaseMutex(ogg->decoder_lock);
                BKNI_WaitForEvent(ogg->decoder_event, BKNI_INFINITE);
            }

            rc = butils_libavcodec_decode(&ogg->ffmpeg, packet.data, packet.size, frame);
            if(rc!=NEXUS_SUCCESS) { rc = BERR_TRACE(rc);break;}
            BKNI_AcquireMutex(ogg->decoder_lock);
            bsoft_decoder_push_decoded_frame(ogg->decoder, frame);
            BKNI_ReleaseMutex(ogg->decoder_lock);
        }
        if(packet.data!=NULL) {
            av_free_packet(&packet);
        }
    }
err_data:
    return NULL;
}

static void
bsoft_decoder_frame_callback(void *context, int _arg1)
{
    bogg_decoder_t sd = context;
    uint32_t stc;
    const struct bsoft_decoder_frame *frame;

    BSTD_UNUSED(_arg1);
    BDBG_OBJECT_ASSERT(sd, bogg_decoder);

    BKNI_AcquireMutex(sd->decoder_lock);
    NEXUS_StcChannel_GetStc(sd->start_settings.stcChannel, &stc);
    frame = bsoft_decoder_get_mature_frame(sd->decoder, stc, &sd->imageInputSettings);
    BKNI_ReleaseMutex(sd->decoder_lock);
    BKNI_SetEvent(sd->decoder_event);
    NEXUS_Surface_Flush(frame->surface);
    NEXUS_VideoImageInput_SetSettings(sd->start_settings.imageInput, &sd->imageInputSettings);
    NEXUS_VideoImageInput_SetSurface(sd->start_settings.imageInput, frame->surface);
    return;
}

NEXUS_Error
bogg_decoder_start(bogg_decoder_t decoder, const bogg_decoder_start_settings *settings, bogg_decoder_start_information *information)
{
    NEXUS_Error rc;
    const char *filename=NULL;
    unsigned i;
    AVCodecContext *context;
    AVCodec *codec;

    BDBG_OBJECT_ASSERT(decoder, bogg_decoder);
    BDBG_ASSERT(settings);
    BDBG_ASSERT(information);

    BDBG_MSG(("bogg_decoder_start"));

    if(settings->imageInput==NULL || settings->stcChannel==NULL || settings->fname==NULL) {
        rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);goto err_settings;
    }
    decoder->start_settings = *settings;
    filename = settings->fname;
    BDBG_MSG(("av_register_all()"));

    avcodec_init();
    register_protocol(&file_protocol);
    register_avcodec(&theora_decoder);
    if (url_fopen(&decoder->data_source, filename, URL_RDONLY) < 0) {
        BDBG_ERR(("can't open file"));
        rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);goto err_settings;
     }

    if(av_open_input_stream(&decoder->demux, decoder->data_source, filename, &ogg_demuxer, NULL)!=0) {
        rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);goto err_settings;
    }
    if(av_find_stream_info(decoder->demux)<0) {
        rc = BERR_TRACE(NEXUS_NOT_SUPPORTED);goto err_settings;
    }
    dump_format(decoder->demux, 0, filename, false);

    decoder->videoStream=-1;
    for(i=0; i<decoder->demux->nb_streams; i++) {
        if(decoder->demux->streams[i]->codec->codec_type==CODEC_TYPE_VIDEO) {
            decoder->timeBase=decoder->demux->streams[i]->time_base;
            decoder->videoStream=i;
            break;
        }
    }
    if(decoder->videoStream==-1) {
        rc = BERR_TRACE(NEXUS_NOT_SUPPORTED);goto err_settings;
    }
    context=decoder->demux->streams[decoder->videoStream]->codec;
    BDBG_WRN(("stream %#x -> %d(%d/%d)", (unsigned)context, decoder->videoStream, decoder->timeBase.num, decoder->timeBase.den));
    /* av_seek_frame(pFormatCtx, videoStream, 0.0, 0); */
    codec=avcodec_find_decoder(context->codec_id);
    if(codec==NULL) {
        rc = BERR_TRACE(NEXUS_NOT_SUPPORTED);goto err_settings;
    }
    rc = butils_libavcodec_init(&decoder->ffmpeg, codec, context);
    if(rc!=NEXUS_SUCCESS) {
        rc = BERR_TRACE(NEXUS_NOT_SUPPORTED);goto err_settings;
    }
    decoder->startedDisplay = false;

    NEXUS_VideoImageInput_GetDefaultSettings(&decoder->imageInputSettings);
    decoder->imageInputSettings.imageCallback.callback = bsoft_decoder_frame_callback;
    decoder->imageInputSettings.imageCallback.context = decoder;
    rc = NEXUS_VideoImageInput_SetSettings(decoder->start_settings.imageInput, &decoder->imageInputSettings);
    if(rc!=NEXUS_SUCCESS) {rc=BERR_TRACE(rc);goto err_settings;}

    pthread_create(&decoder->thread, NULL, b_oggdecoder_thread, decoder);

    return NEXUS_SUCCESS;

err_settings:
    return rc;
}
