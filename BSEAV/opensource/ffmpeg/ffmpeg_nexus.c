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
#include "libavcodec/avcodec.h"
#include "libavutil/imgutils.h"
#include "libavformat/avformat.h"
#include <libavfilter/avfilter.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>

#include "blst_list.h"
#include "bkni.h"
#include "nexus_types.h"
#include "ffmpeg_buffers.h"
#include "nexus_platform.h"
#include "nexus_display.h"
#include "nexus_component_output.h"
#include "nexus_composite_output.h"
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#endif
#include "nexus_graphics2d.h"
#include "nexus_video_output.h"
#include "nexus_video_window.h"
#include "nexus_video_image_input.h"
#include "nexus_video_input.h"
#include "nexus_playpump.h"
#include "nexus_audio_dac.h"
#include "nexus_audio_decoder.h"
#include "nexus_audio_input.h"
#include "nexus_audio_output.h"
#include "nexus_spdif_output.h"
#include "bmedia_util.h"

BDBG_MODULE(ffmpeg_nexus);

struct decode_stream_nexus_video_frame {
    NEXUS_SurfaceHandle surface;
    BM2MC_PACKET_Plane plane;
    bool submitted;
};

struct decode_stream_nexus_video {
    NEXUS_Graphics2DHandle gfx;
    BKNI_EventHandle gfxEvent;

    NEXUS_VideoImageInputHandle imageInput;
    BKNI_EventHandle imageEvent;
    NEXUS_VideoInput videoInput;
    NEXUS_HeapHandle gfxInputHeap;
    struct decode_stream_nexus_video_frame surfaces[4];
    struct b_avcodec_buffer_queue queue;
};

struct decode_stream_nexus_audio {
    NEXUS_PlaypumpHandle playpump;
    BKNI_EventHandle playpumpEvent;
    NEXUS_AudioDecoderHandle decoder;
    NEXUS_PidChannelHandle pidChannel;
    bmedia_waveformatex_header waveformat;
    const uint8_t *playpumpBufferEnd;
    uint8_t waveheader[BMEDIA_PES_HEADER_MAX_SIZE+BMEDIA_WAVEFORMATEX_BASE_SIZE];
};

struct decode_stream_convert_audio {
    AVFilter *src;
    AVFilter *format;
    AVFilter *dst;
    AVFilterInOut *in;
    AVFilterInOut *out;
    AVFilterGraph *filter;
    AVFilterContext *dst_ctx;
    AVFilterContext *src_ctx;
    AVFilterContext *format_ctx;
    AVFrame *frame;
};

struct decode_stream {
    int id;
    AVStream *stream;
    AVCodecContext *decoder;
    AVFrame *frame;
    union {
        struct {
            int width;
            int height;
            enum AVPixelFormat format;
            struct decode_stream_nexus_video nexus;
        } video;
        struct {
            struct decode_stream_convert_audio convert;
            struct decode_stream_nexus_audio nexus;
        } audio;
    } data;
};

struct decode_file {
    AVFormatContext *fmt;
    struct decode_stream video;
    struct decode_stream audio;
};

static NEXUS_Error decode_alloc(void *self, size_t size, b_video_softdecode_memory_token *token)
{
    NEXUS_Error rc;
    NEXUS_MemoryBlockHandle block;
    BSTD_UNUSED(self);
    block = NEXUS_MemoryBlock_Allocate(NULL, size, 0, NULL);
    BDBG_MSG(("Alloc %u->%p", size, (void *)block));
    if(block==NULL) { return BERR_TRACE(NEXUS_OUT_OF_DEVICE_MEMORY); }
    rc = NEXUS_MemoryBlock_LockOffset(block, &token->addr);
    if(rc!=NEXUS_SUCCESS) {
        NEXUS_MemoryBlock_Free(block);
        return BERR_TRACE(rc);
    }
    rc = NEXUS_MemoryBlock_Lock(block, &token->ptr);
    if(rc!=NEXUS_SUCCESS) {
        NEXUS_MemoryBlock_Free(block);
        return BERR_TRACE(rc);
    }
    token->opaque = block;
    return NEXUS_SUCCESS;
}

static void decode_free(void *self, const b_video_softdecode_memory_token *token)
{
    NEXUS_MemoryBlockHandle block = token->opaque;
    BSTD_UNUSED(self);
    BDBG_MSG(("Free %p", (void *)block));
    NEXUS_MemoryBlock_Unlock(block);
    NEXUS_MemoryBlock_Free(block);
    return;
}

static const b_video_sofdecode_memory_methods decode_memory = {
    decode_alloc, decode_free
};

static void decode_stream_init(struct decode_stream *s)
{
    BKNI_Memset(s, 0, sizeof(*s));
    s->id = -1;
    s->stream  = NULL;
    s->decoder = NULL;
    s->frame = NULL;
    return;
}

static uint32_t decode_rescale_pts(int64_t pts, const AVRational time_base)
{
    AVRational _45KHz = av_make_q(1, 45000);
    if(pts!=AV_NOPTS_VALUE) {
        int64_t new_pts = av_rescale_q(pts, time_base, _45KHz);
        return (uint32_t)new_pts;
    }
    return 0;
}

static int decode_stream_open(struct decode_stream *s, const struct decode_file *f, enum AVMediaType type)
{
    int rc;
    AVCodec *decoder;
    AVDictionary *opts = NULL;

    rc = av_find_best_stream(f->fmt, type, -1, -1, NULL, 0);
    if(rc<0) { return BERR_TRACE(rc);}
    s->id = rc;
    s->stream = f->fmt->streams[s->id];
    s->decoder = s->stream->codec;
    decoder =  avcodec_find_decoder(s->decoder->codec_id);
    if(rc<0) { return BERR_TRACE(rc);}
    av_dict_set(&opts, "refcounted_frames", "1", 0);
    rc = avcodec_open2(s->decoder, decoder, &opts);
    if(rc<0) { return BERR_TRACE(rc);}
    s->frame = av_frame_alloc();
    if(s->frame==NULL) {return BERR_TRACE(-1);}
    return 0;
}

static void decode_stream_close(struct decode_stream *s)
{
    avcodec_close(s->decoder);
    av_frame_free(&s->frame);
    return;
}

static void decode_file_init(struct decode_file *f)
{
    f->fmt = NULL;
    decode_stream_init(&f->video);
    decode_stream_init(&f->audio);
    return;
}


static int decode_file_open(struct decode_file *f, const char *filename)
{
    int rc=0;

    rc = avformat_open_input(&f->fmt, filename, NULL, NULL);
    if(rc< 0) {
        rc = BERR_TRACE(rc);
        goto err_open;
    }
    rc = avformat_find_stream_info(f->fmt, NULL);
    if(rc<0) {
        rc = BERR_TRACE(rc);
        goto err_stream;
    }
    rc = decode_stream_open(&f->video, f, AVMEDIA_TYPE_VIDEO);
    if (rc>=0) {
        struct decode_stream * video = &f->video;
        video->data.video.width = video->decoder->width;
        video->data.video.height = video->decoder->height;
        video->data.video.format = video->decoder->pix_fmt;
        b_avcodec_buffer_queue_init(&video->data.video.nexus.queue, &decode_memory, video);
        b_avcodec_buffer_queue_connect(&video->data.video.nexus.queue, video->decoder);
    }
    rc = decode_stream_open(&f->audio, f, AVMEDIA_TYPE_AUDIO);

    av_dump_format(f->fmt, 0, filename, 0);
    if(f->video.id<0 && f->audio.id<0) {
        rc = BERR_TRACE(-1);goto err_no_data;
    }

    return 0;

err_no_data:
err_stream:
err_open:
    return rc;
}

static void decode_file_close(struct decode_file *f)
{
    if(f->video.decoder) {
        struct decode_stream * video = &f->video;
        decode_stream_close(video);
        b_avcodec_buffer_queue_disconnect(&video->data.video.nexus.queue);
    }
    if(f->video.decoder) {
        decode_stream_close(&f->audio);
    }
    avformat_close_input(&f->fmt);

    return;
}

static NEXUS_Error decode_stream_convert_plane(struct b_avcodec_buffer_queue *queue, BM2MC_PACKET_Plane *pPlane, const AVFrame *frame, unsigned component, NEXUS_PixelFormat pixelFormat)
{
    uint8_t *buf = frame->data[component];
    const b_video_softdecode_memory_token *block = b_avcodec_buffer_queue_find(queue, buf);

    if(block==NULL) {
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }
    pPlane->address = block->addr + (buf - (uint8_t *)block->ptr);
    pPlane->pitch = frame->linesize[component];
    pPlane->format = pixelFormat;
    pPlane->width = frame->width;
    pPlane->height = frame->height;
    if(pixelFormat!=NEXUS_PixelFormat_eY8) {
        pPlane->width /=2;
        pPlane->height /=2;
    }
    NEXUS_FlushCache(buf, pPlane->height * pPlane->pitch);
    return NEXUS_SUCCESS;
}

static int decode_stream_convert_video(NEXUS_Graphics2DHandle graphics, BKNI_EventHandle checkpointEvent, const AVFrame *frame, struct b_avcodec_buffer_queue *queue, struct decode_stream_nexus_video_frame *nexus_frame)
{
    static const BM2MC_PACKET_Blend combColor = {BM2MC_PACKET_BlendFactor_eSourceColor, BM2MC_PACKET_BlendFactor_eOne, false,
        BM2MC_PACKET_BlendFactor_eDestinationColor, BM2MC_PACKET_BlendFactor_eOne, false, BM2MC_PACKET_BlendFactor_eZero};
    static const BM2MC_PACKET_Blend copyAlpha = {BM2MC_PACKET_BlendFactor_eZero, BM2MC_PACKET_BlendFactor_eOne, false,
        BM2MC_PACKET_BlendFactor_eZero, BM2MC_PACKET_BlendFactor_eZero, false, BM2MC_PACKET_BlendFactor_eZero};
    BM2MC_PACKET_Plane planeY, planeCb, planeCr, planeYCbCr;
    void *buffer, *next;
    size_t size;
    NEXUS_Error rc;

    decode_stream_convert_plane(queue, &planeY, frame, 0, NEXUS_PixelFormat_eY8);
    decode_stream_convert_plane(queue, &planeCb, frame, 1, NEXUS_PixelFormat_eCb8);
    decode_stream_convert_plane(queue, &planeCr, frame, 2, NEXUS_PixelFormat_eCr8);

    planeYCbCr = nexus_frame->plane;

    /* contributed by Shi-Long (Steven) Yang <syang@broadcom.com> */
    rc = NEXUS_Graphics2D_GetPacketBuffer(graphics, &buffer, &size, 1024);
    BDBG_ASSERT((!rc) && (size));

    next = buffer;
    {
        BM2MC_PACKET_PacketFilterEnable *pPacket = next;
        BM2MC_PACKET_INIT(pPacket, FilterEnable, false );
        pPacket->enable = 0;
        next = ++pPacket;
    }
    {
        BM2MC_PACKET_PacketSourceFeeders *pPacket = next;
        BM2MC_PACKET_INIT(pPacket, SourceFeeders, false );
        pPacket->plane0 = planeCb;
        pPacket->plane1 = planeCr;
        pPacket->color = 0;
        next = ++pPacket;
    }
    {
        BM2MC_PACKET_PacketDestinationFeeder *pPacket = next;
        BM2MC_PACKET_INIT(pPacket, DestinationFeeder, false );
        pPacket->plane = planeY;
        pPacket->color = 0;
        next = ++pPacket;
    }
    {
        BM2MC_PACKET_PacketOutputFeeder *pPacket = next;
        BM2MC_PACKET_INIT(pPacket, OutputFeeder, false);
        pPacket->plane = planeYCbCr;
        next = ++pPacket;
    }
    {
        BM2MC_PACKET_PacketBlend *pPacket = next;
        BM2MC_PACKET_INIT( pPacket, Blend, false );
        pPacket->color_blend = combColor;
        pPacket->alpha_blend = copyAlpha;
        pPacket->color = 0;
        next = ++pPacket;
    }
    {
        BM2MC_PACKET_PacketScaleBlendBlit *pPacket = next;
        BM2MC_PACKET_INIT(pPacket, ScaleBlendBlit, true);
        pPacket->src_rect.x = 0;
        pPacket->src_rect.y = 0;
        pPacket->src_rect.width = planeCb.width;
        pPacket->src_rect.height = planeCb.height;
        pPacket->out_rect.x = 0;
        pPacket->out_rect.y = 0;
        pPacket->out_rect.width = planeYCbCr.width;
        pPacket->out_rect.height = planeYCbCr.height;
        pPacket->dst_point.x = 0;
        pPacket->dst_point.y = 0;
        next = ++pPacket;
    }

    rc = NEXUS_Graphics2D_PacketWriteComplete(graphics, (uint8_t*)next - (uint8_t*)buffer);
    BDBG_ASSERT(!rc);
    rc = NEXUS_Graphics2D_Checkpoint(graphics, NULL);
    if (rc == NEXUS_GRAPHICS2D_QUEUED) {
        rc = BKNI_WaitForEvent(checkpointEvent, 5000);
        BDBG_ASSERT(!rc);
    }
    else {
        BDBG_ASSERT(!rc);
    }
    return 0;
}

static int decode_file_decode_video(struct decode_stream *s, int *got_frame, const AVPacket *pkt)
{
    int rc;
    unsigned i;

    rc = avcodec_decode_video2(s->decoder, s->frame, got_frame, pkt);
    if(rc<0) { return BERR_TRACE(rc); }
    if(*got_frame) {
        struct decode_stream_nexus_video *video = &s->data.video.nexus;
        uint32_t pts = decode_rescale_pts(s->frame->pkt_pts, s->stream->time_base);
        BDBG_MSG(("video: %u %ux%u %p", pts, s->frame->width, s->frame->height, s->frame->data[0]));
        if (s->frame->width != s->data.video.width || s->frame->height != s->data.video.height || s->frame->format != s->data.video.format) {
            return BERR_TRACE(-1);
        }
        for(;;) {
            struct decode_stream_nexus_video_frame *frame=NULL;
            for( i=0; i < sizeof(video->surfaces)/sizeof(video->surfaces[0]); i++ ) {
                if(!video->surfaces[i].submitted) {
                    BDBG_MSG(("using %u", i));
                    frame = &video->surfaces[i];
                    break;
                }
            }
            if(frame) {
                NEXUS_VideoImageInputSurfaceSettings surfaceSettings;
                NEXUS_VideoImageInput_GetDefaultSurfaceSettings( &surfaceSettings );
                surfaceSettings.pts = pts;
                surfaceSettings.ptsValid = s->frame->pkt_pts != AV_NOPTS_VALUE;
                frame->submitted = true;
                decode_stream_convert_video(video->gfx, video->gfxEvent, s->frame, &video->queue, frame);
                NEXUS_VideoImageInput_PushSurface(video->imageInput, frame->surface, &surfaceSettings );
                break;
            } else {
                NEXUS_Error rc;
                NEXUS_SurfaceHandle recycled[1];
                size_t n_recyled=0;
                rc = NEXUS_VideoImageInput_RecycleSurface( video->imageInput, recycled, sizeof(recycled)/sizeof(recycled[0]), &n_recyled);
                BDBG_ASSERT(rc==NEXUS_SUCCESS);
                BDBG_MSG(("recycled %u", n_recyled));
                if(n_recyled==0) {
                    BKNI_WaitForEvent(video->imageEvent, BKNI_INFINITE);
                } else {
                    unsigned j;
                    for(j=0;j<n_recyled;j++) {
                        for( i=0; i < sizeof(video->surfaces)/sizeof(video->surfaces[0]); i++ ) {
                            if(video->surfaces[i].surface == recycled[j]) {
                                BDBG_MSG(("recycle %u", i));
                                video->surfaces[i].submitted = false;
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
    return pkt->size;
}

static int decode_file_decode_audio(struct decode_stream *s, int *got_frame, const AVPacket *pkt)
{
    int rc;
    rc = avcodec_decode_audio4(s->decoder, s->frame, got_frame, pkt);
    if (rc < 0) { return BERR_TRACE(rc);}
    if(rc > pkt->size) {
        rc = pkt->size;
    }
    if (*got_frame) {
        struct decode_stream_nexus_audio *audio = &s->data.audio.nexus;
        struct decode_stream_convert_audio *convert = &s->data.audio.convert;
        int rc;
        BDBG_MSG(("audio decoded: %u %u@%u %p %u %s", (unsigned)s->frame->pkt_pts, s->frame->nb_samples, s->frame->sample_rate, s->frame->data[0], (unsigned)(s->frame->nb_samples * av_get_bytes_per_sample(s->frame->format)), av_get_sample_fmt_name(s->decoder->sample_fmt)));
        rc = av_buffersrc_write_frame(convert->src_ctx, s->frame);
        if(rc<0) {return BERR_TRACE(rc);}
        for(;;) {
            void *buffer;
            size_t size;
            size_t used;
            bmedia_pes_info pes;
            size_t payload;
            size_t waveformat_size;


            rc = av_buffersink_get_frame(convert->dst_ctx, convert->frame);
            if (rc == AVERROR_EOF || rc == AVERROR(EAGAIN)) {
                break;
            }
            if(rc<0) {return BERR_TRACE(rc);}
            payload = convert->frame->nb_samples * av_get_bytes_per_sample(convert->frame->format) * convert->frame->channels;
            for(;;) {
                size=0;
                NEXUS_Playpump_GetBuffer(audio->playpump, &buffer, &size);
                if(size>=payload + BMEDIA_PES_HEADER_MAX_SIZE + BMEDIA_WAVEFORMATEX_BASE_SIZE + bmedia_frame_bcma.len + 4) {
                    break;
                }
                if((uint8_t *)buffer + size >= audio->playpumpBufferEnd) {
                    NEXUS_Playpump_WriteComplete(audio->playpump, size, 0);
                } else {
                    BKNI_WaitForEvent(audio->playpumpEvent, BKNI_INFINITE);
                }
            }
            BMEDIA_PES_INFO_INIT(&pes, 0xC0);
            if( convert->frame->pkt_pts != AV_NOPTS_VALUE) {
                uint32_t pts = decode_rescale_pts(s->frame->pkt_pts, s->stream->time_base);
                BMEDIA_PES_SET_PTS(&pes, pts);
            }
            BDBG_MSG(("audio converted: %u(%u) %u@%u %p %u %s", (unsigned)convert->frame->pkt_pts, (unsigned)pes.pts, convert->frame->nb_samples, convert->frame->sample_rate, convert->frame->data[0], (unsigned)payload, av_get_sample_fmt_name(convert->frame->format)));
            used = bmedia_pes_header_init(buffer, payload + BMEDIA_WAVEFORMATEX_BASE_SIZE + bmedia_frame_bcma.len + 4, &pes);
            BKNI_Memcpy((uint8_t *)buffer + used, bmedia_frame_bcma.base, bmedia_frame_bcma.len);
            used += bmedia_frame_bcma.len;
            B_MEDIA_SAVE_UINT32_BE((uint8_t *)buffer+used, payload);
            used += sizeof(uint32_t);
            waveformat_size = bmedia_write_waveformatex((uint8_t *)buffer+used, &audio->waveformat);
            BDBG_ASSERT(waveformat_size==BMEDIA_WAVEFORMATEX_BASE_SIZE);
            used += waveformat_size;
            /* batom_range_dump(buffer, used, "pes"); */
            BKNI_Memcpy((uint8_t *)buffer+used,convert->frame->data[0],payload);
            used += payload;
            BDBG_ASSERT(used<=size);
            NEXUS_Playpump_WriteComplete(audio->playpump, 0, used);
        }
    }
    return rc;
}

static int decode_stream_convert_audio_init(struct decode_stream *s)
{
    struct decode_stream_convert_audio *convert = &s->data.audio.convert;
    int rc;
    char buf[128];

    convert->src = avfilter_get_by_name("abuffer");
    convert->format = avfilter_get_by_name("aformat");
    convert->dst = avfilter_get_by_name("abuffersink");
    convert->in = avfilter_inout_alloc();
    convert->out = avfilter_inout_alloc();
    convert->filter = avfilter_graph_alloc();
    BKNI_Snprintf(buf, sizeof(buf),"time_base=%d/%d:sample_rate=%d:sample_fmt=%s:channel_layout=0x%x",
                  s->stream->time_base.num, s->stream->time_base.den, s->decoder->sample_rate, av_get_sample_fmt_name(s->decoder->sample_fmt), (unsigned)(s->decoder->channel_layout));

    rc = avfilter_graph_create_filter(&convert->src_ctx, convert->src, "src", buf, NULL, convert->filter);
    if(rc<0) {return BERR_TRACE(rc);}

    rc = avfilter_graph_create_filter(&convert->dst_ctx, convert->dst, "dst", NULL, NULL, convert->filter);
    if(rc<0) {return BERR_TRACE(rc);}

    BKNI_Snprintf(buf, sizeof(buf),"sample_fmts=%s:channel_layouts=0x%x",
                  av_get_sample_fmt_name(AV_SAMPLE_FMT_S16), AV_CH_LAYOUT_STEREO);

    rc = avfilter_graph_create_filter(&convert->format_ctx, convert->format, "format", buf, NULL, convert->filter);
    if(rc<0) {return BERR_TRACE(rc);}

    rc = avfilter_link(convert->src_ctx, 0, convert->format_ctx, 0);
    if(rc<0) {return BERR_TRACE(rc);}
    rc = avfilter_link(convert->format_ctx, 0, convert->dst_ctx, 0);
    if(rc<0) {return BERR_TRACE(rc);}
    rc = avfilter_graph_config(convert->filter, NULL);
    if(rc<0) {return BERR_TRACE(rc);}

    convert->frame = av_frame_alloc();

    return 0;
}

static void decode_stream_convert_audio_uninit(struct decode_stream *s)
{
    struct decode_stream_convert_audio *convert = &s->data.audio.convert;
    avfilter_graph_free(&convert->filter);
    av_frame_free(&convert->frame);
    return;
}

static int decode_file_decode_packet(struct decode_file *f, int *pgot_frame, const AVPacket *pkt)
{
    int rc = pkt->size;
    int got_frame = 0;
    struct decode_stream *s = NULL;

    if(pkt->stream_index == f->video.id) {
        s = &f->video;
        rc = decode_file_decode_video(s, &got_frame, pkt);
        if(rc<0) {return BERR_TRACE(rc);}
    } else if(pkt->stream_index == f->audio.id) {
        s = &f->audio;
        rc = decode_file_decode_audio(s, &got_frame, pkt);
        if(rc<0) {return BERR_TRACE(rc);}
    }
    if(s && got_frame) {
        av_frame_unref(s->frame);
    }
    if(pgot_frame) {
        *pgot_frame = got_frame;
    }
    return rc;
}

static int decode_file_decode(struct decode_file *f)
{
    AVPacket pkt;
    int rc;

    av_init_packet(&pkt);
    pkt.data = NULL;
    pkt.size = 0;

    for(;;) {
        AVPacket temp;
        rc = av_read_frame(f->fmt, &pkt);
        if(rc<0) {
            break;
        }
        temp = pkt;
        BDBG_MSG(("packet %u", pkt.size));
        for(;;) {
            rc = decode_file_decode_packet(f, NULL, &temp);

            if(rc<0) {
                break;
            }
            temp.data += rc;
            temp.size -= rc;
            if(temp.size==0) {
                break;
            }
        }
        av_free_packet(&pkt);
    }
    BDBG_LOG(("End of File"));
    pkt.data = NULL;
    pkt.size = 0;
    for(;;) {
        int got_frame = NULL;
        got_frame = 0;
        decode_file_decode_packet(f, &got_frame, &pkt);
        if(!got_frame) {
            break;
        }
    }
    return 0;
}

static void complete(void *data, int unused)
{
    BSTD_UNUSED(unused);
    BKNI_SetEvent((BKNI_EventHandle)data);
}

static void imageBufferCallback( void *context, int param )
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
}


static int decode_stream_video_nexus_open(struct decode_stream *s, NEXUS_VideoWindowHandle window, const NEXUS_PlatformConfiguration *platformConfig, NEXUS_StcChannelHandle stcChannel)
{
    unsigned i;
    NEXUS_Graphics2DSettings gfxSettings;
    NEXUS_VideoImageInputSettings imageInputSettings;
    NEXUS_VideoImageInputStatus imageInputStatus;
    struct decode_stream_nexus_video *video = &s->data.video.nexus;
    NEXUS_Error rc;
    NEXUS_SurfaceCreateSettings surfaceCfg;

    BKNI_CreateEvent(&video->gfxEvent);
    video->gfx = NEXUS_Graphics2D_Open(0, NULL);
    NEXUS_Graphics2D_GetSettings(video->gfx, &gfxSettings);
    gfxSettings.checkpointCallback.callback = complete;
    gfxSettings.checkpointCallback.context = video->gfxEvent;
    NEXUS_Graphics2D_SetSettings(video->gfx, &gfxSettings);

    NEXUS_VideoImageInput_GetDefaultSettings(&imageInputSettings);
    imageInputSettings.type = NEXUS_VideoImageInput_eMfd;
    imageInputSettings.stcChannel = stcChannel;
    imageInputSettings.lowDelayMode = false;
    imageInputSettings.tsmEnabled = true;
    imageInputSettings.fifoSize = sizeof(video->surfaces)/sizeof(video->surfaces[0]);
    video->imageInput = NEXUS_VideoImageInput_Open(0, &imageInputSettings);
    BDBG_ASSERT(video->imageInput);

    NEXUS_VideoImageInput_GetStatus(video->imageInput, &imageInputStatus);

    video->gfxInputHeap = NULL;
    for (i=0;i<NEXUS_MAX_HEAPS;i++) {
        NEXUS_MemoryStatus s;
        if (platformConfig->heap[i]==NULL || NEXUS_Heap_GetStatus(platformConfig->heap[i], &s)!=NEXUS_SUCCESS) continue;
        if (s.memcIndex == imageInputStatus.memcIndex && (s.memoryType & NEXUS_MemoryType_eApplication) && s.largestFreeBlock >= 960*1080*2) {
            video->gfxInputHeap = platformConfig->heap[i];
            break;
        }
    }
    BDBG_ASSERT(video->gfxInputHeap);

    video->videoInput = NEXUS_VideoImageInput_GetConnector(video->imageInput);
    BDBG_ASSERT(video->videoInput);

    rc = NEXUS_VideoWindow_AddInput(window, video->videoInput);
    BDBG_ASSERT(!rc);

    BKNI_CreateEvent(&video->imageEvent);
    NEXUS_VideoImageInput_GetSettings(video->imageInput, &imageInputSettings);
    imageInputSettings.imageCallback.callback = imageBufferCallback;
    imageInputSettings.imageCallback.context  = video->imageEvent;
    NEXUS_VideoImageInput_SetSettings( video->imageInput, &imageInputSettings);

    NEXUS_Surface_GetDefaultCreateSettings(&surfaceCfg);
    surfaceCfg.width  = s->data.video.width;
    surfaceCfg.height = s->data.video.height;
    surfaceCfg.pixelFormat = NEXUS_PixelFormat_eCr8_Y18_Cb8_Y08;
    surfaceCfg.heap = video->gfxInputHeap;
    for( i=0; i < sizeof(video->surfaces)/sizeof(video->surfaces[0]); i++ ) {
        video->surfaces[i].surface = NEXUS_Surface_Create(&surfaceCfg);
        BDBG_ASSERT(video->surfaces[i].surface);
        video->surfaces[i].submitted = false;
        NEXUS_Surface_LockPlaneAndPalette(video->surfaces[i].surface, &video->surfaces[i].plane, NULL);
    }

    return 0;
}

static void decode_stream_video_nexus_close(struct decode_stream *s, NEXUS_VideoWindowHandle window)
{
    unsigned i;
    struct decode_stream_nexus_video *video = &s->data.video.nexus;

    NEXUS_VideoWindow_RemoveInput(window, video->videoInput);
    NEXUS_VideoInput_Shutdown(video->videoInput);
    NEXUS_VideoImageInput_Close(video->imageInput);
    BKNI_DestroyEvent(video->imageEvent);

    BKNI_DestroyEvent(video->gfxEvent);
    NEXUS_Graphics2D_Close(video->gfx);


    for( i=0; i < sizeof(video->surfaces)/sizeof(video->surfaces[0]); i++ ) {
        NEXUS_Surface_UnlockPlaneAndPalette(video->surfaces[i].surface);
        NEXUS_Surface_Destroy(video->surfaces[i].surface);
    }

    return;
}

static void play_callback(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
}

static int decode_stream_audio_nexus_open(struct decode_stream *s, const NEXUS_PlatformConfiguration *platformConfig, NEXUS_StcChannelHandle stcChannel)
{
    struct decode_stream_nexus_audio *audio = &s->data.audio.nexus;
    NEXUS_PlaypumpSettings playpumpSettings;
    NEXUS_AudioDecoderStartSettings audioProgram;
    NEXUS_PlaypumpStatus playpumpStatus;

    audio->decoder = NEXUS_AudioDecoder_Open(0, NULL);
#if NEXUS_NUM_AUDIO_DACS
    NEXUS_AudioOutput_AddInput(
        NEXUS_AudioDac_GetConnector(platformConfig->outputs.audioDacs[0]),
        NEXUS_AudioDecoder_GetConnector(audio->decoder, NEXUS_AudioDecoderConnectorType_eStereo));
#endif
#if NEXUS_NUM_SPDIF_OUTPUTS
    NEXUS_AudioOutput_AddInput(
        NEXUS_SpdifOutput_GetConnector(platformConfig->outputs.spdif[0]),
        NEXUS_AudioDecoder_GetConnector(audio->decoder, NEXUS_AudioDecoderConnectorType_eStereo));
#endif
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_AudioOutput_AddInput(
        NEXUS_HdmiOutput_GetAudioConnector(platformConfig->outputs.hdmi[0]),
        NEXUS_AudioDecoder_GetConnector(audio->decoder, NEXUS_AudioDecoderConnectorType_eStereo));
#endif
    audio->playpump = NEXUS_Playpump_Open(NEXUS_ANY_ID, NULL);

    BKNI_CreateEvent(&audio->playpumpEvent);
    NEXUS_Playpump_GetSettings(audio->playpump, &playpumpSettings);
    playpumpSettings.dataCallback.callback = play_callback;
    playpumpSettings.dataCallback.context = audio->playpumpEvent;
    playpumpSettings.transportType = NEXUS_TransportType_eMpeg2Pes;
    NEXUS_Playpump_SetSettings(audio->playpump, &playpumpSettings);
    NEXUS_Playpump_GetStatus(audio->playpump, &playpumpStatus);
    audio->playpumpBufferEnd = (uint8_t *)playpumpStatus.bufferBase + playpumpStatus.fifoDepth;


    audio->pidChannel = NEXUS_Playpump_OpenPidChannel(audio->playpump, 0xC0, NULL);
    NEXUS_AudioDecoder_GetDefaultStartSettings(&audioProgram);
    audioProgram.codec = NEXUS_AudioCodec_ePcmWav;
    audioProgram.pidChannel = audio->pidChannel;
    audioProgram.stcChannel = stcChannel;
    NEXUS_AudioDecoder_Start(audio->decoder, &audioProgram);

    bmedia_init_waveformatex(&audio->waveformat);
    audio->waveformat.wFormatTag = 0x0001;
    audio->waveformat.nChannels = s->decoder->channels;
    audio->waveformat.nSamplesPerSec = s->decoder->sample_rate;
    audio->waveformat.wBitsPerSample = 16;
    audio->waveformat.nBlockAlign = (audio->waveformat.nChannels * audio->waveformat.wBitsPerSample)/8;
    audio->waveformat.cbSize = 0;
    audio->waveformat.nAvgBytesPerSec = (audio->waveformat.wBitsPerSample * audio->waveformat.nSamplesPerSec * audio->waveformat.nChannels)/8;

    NEXUS_Playpump_Start(audio->playpump);

    return 0;
}

static void decode_stream_audio_nexus_close(struct decode_stream *s)
{
    struct decode_stream_nexus_audio *audio = &s->data.audio.nexus;

    NEXUS_Playpump_Stop(audio->playpump);
    NEXUS_AudioDecoder_Stop(audio->decoder);
    NEXUS_Playpump_ClosePidChannel(audio->playpump, audio->pidChannel);
    NEXUS_Playpump_Close(audio->playpump);
    BKNI_DestroyEvent(audio->playpumpEvent);
    NEXUS_AudioDecoder_Close(audio->decoder);
    return;
}


int main(int argc, const char *argv[])
{
    NEXUS_Error rc;
    NEXUS_PlatformSettings platformSettings;
    struct decode_file f;
    const char *filename=NULL;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_DisplayHandle display;
    NEXUS_VideoWindowHandle window;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_StcChannelHandle stcChannel;
    NEXUS_StcChannelSettings stcSettings;
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputStatus hdmiStatus;
#endif

    av_register_all();
    avfilter_register_all();

    if(argc>1) {
        filename = argv[1];
    }
    if(filename==NULL) {
        fprintf(stderr,"Missing playback file; See usage.\n");
        return 1;
    }
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    rc = NEXUS_Platform_Init(&platformSettings);
    BDBG_ASSERT(!rc);
    NEXUS_Platform_GetConfiguration(&platformConfig);

    NEXUS_Display_GetDefaultSettings(&displaySettings);
    displaySettings.displayType = NEXUS_DisplayType_eAuto;
    displaySettings.format      = NEXUS_VideoFormat_eNtsc; /* change to xx_e720p to observe faster draw rate */
    display = NEXUS_Display_Open(0, &displaySettings);

    NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;
    stcSettings.mode = NEXUS_StcChannelMode_eAuto;
    stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);

#if NEXUS_NUM_COMPONENT_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_ComponentOutput_GetConnector(platformConfig.outputs.component[0]));
#endif
#if NEXUS_NUM_COMPOSITE_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_CompositeOutput_GetConnector(platformConfig.outputs.composite[0]));
#endif
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_HdmiOutput_GetVideoConnector(platformConfig.outputs.hdmi[0]));
    rc = NEXUS_HdmiOutput_GetStatus(platformConfig.outputs.hdmi[0], &hdmiStatus);
    if ( !rc && hdmiStatus.connected )
    {
        /* If current display format is not supported by monitor, switch to monitor's preferred format.
           If other connected outputs do not support the preferred format, a harmless error will occur. */
        NEXUS_Display_GetSettings(display, &displaySettings);
        if ( !hdmiStatus.videoFormatSupported[displaySettings.format] ) {
            displaySettings.format = hdmiStatus.preferredVideoFormat;
            NEXUS_Display_SetSettings(display, &displaySettings);
        }
    }
#endif

    window = NEXUS_VideoWindow_Open(display, 0);
    BDBG_ASSERT(window);


    decode_file_init(&f);
    decode_file_open(&f, filename);
    if(f.video.decoder) {
        decode_stream_video_nexus_open(&f.video, window, &platformConfig, stcChannel);
    }
    if(f.audio.decoder) {
        decode_stream_audio_nexus_open(&f.audio, &platformConfig, stcChannel);
        decode_stream_convert_audio_init(&f.audio);
    }
    decode_file_decode(&f);
    if(f.audio.decoder) {
        decode_stream_convert_audio_uninit(&f.audio);
        decode_stream_audio_nexus_close(&f.audio);
    }
    if(f.video.decoder) {
        decode_stream_video_nexus_close(&f.video, window);
    }
    decode_file_close(&f);

    NEXUS_VideoWindow_Close(window);
    NEXUS_Display_Close(display);
    /* Bring up all modules for a platform in a default configuration for this platform */
    NEXUS_Platform_Uninit();

    return 0;
}
