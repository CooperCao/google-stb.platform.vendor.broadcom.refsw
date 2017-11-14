/******************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *****************************************************************************/
#include "media_probe.h"
#include "bstd.h"
#include "namevalue.h"
#if NEXUS_HAS_FILE
#include "nexus_file.h"
#endif
#include <string.h>

/* media probe */
#include "bmedia_probe.h"
#include "bmpeg2ts_probe.h"
#include "bfile_stdio.h"
#if B_HAS_ASF
#include "basf_probe.h"
#endif
#if B_HAS_AVI
#include "bavi_probe.h"
#endif
#include "bhevc_video_probe.h"
#include "bmkv_probe.h"
#include "bmkv_util.h"
#include "nexus_core_utils.h"
#include "bfile_crypto.h"

BDBG_MODULE(media_probe);

#if NEXUS_HAS_PLAYBACK
static void b_print_media_string(const bmedia_probe_stream *stream)
{
    char stream_info[512];
    bmedia_stream_to_string(stream, stream_info, sizeof(stream_info));
    printf( "Media Probe:\n" "%s\n\n", stream_info);
}
#endif

int probe_media(const char *streamname, struct probe_results *results)
{
#if NEXUS_HAS_PLAYBACK
    struct probe_request request;
    probe_media_get_default_request(&request);
    request.streamname = streamname;
    return probe_media_request(&request, results);
#else
    BSTD_UNUSED(streamname);
    BSTD_UNUSED(results);
    return -1;
#endif
}

void probe_media_get_default_request(struct probe_request *request)
{
    memset(request, 0, sizeof(*request));
    request->decrypt.algo = NEXUS_SecurityAlgorithm_eMax;
}

#if NEXUS_HAS_PLAYBACK
static void probe_p_parse_mkv_block(void *data, unsigned len, struct probe_results *results)
{
    bmkv_TrackEntryVideoColour colour;
    batom_vec vec;
    batom_cursor cursor;
    bmkv_TrackEntryVideoColourMasteringMetadata *pmeta;

    BATOM_VEC_INIT(&vec, data, len);
    batom_cursor_from_vec(&cursor, &vec, 1);
    bmkv_TrackEntryVideoColour_desc.init(&colour);

    if(bmkv_element_parse(&cursor, len, bmkv_TrackEntryVideoColour_desc.entries, bmkv_TrackEntryVideoColour_desc.bmkv_parser_desc_name, &colour)) {
        bmkv_element_print(bmkv_TrackEntryVideoColour_desc.entries, BDBG_eLog, 0, bmkv_TrackEntryVideoColour_desc.bmkv_parser_desc_name, &colour);

        if(colour.validate.MaxCLL) {
            results->videoColourMasteringMetadata.contentLightLevel.max = (unsigned)colour.MaxCLL;
        }
        if(colour.validate.MaxFALL) {
            results->videoColourMasteringMetadata.contentLightLevel.maxFrameAverage = (unsigned)colour.MaxFALL;
        }

        if(colour.validate.TransferCharacteristics) {
            results->videoColourMasteringMetadata.eotf =
                (colour.TransferCharacteristics == 16) ? NEXUS_VideoEotf_eHdr10 : ((colour.TransferCharacteristics == 18) ? NEXUS_VideoEotf_eHlg : NEXUS_VideoEotf_eInvalid);
        }

        if(colour.validate.MasteringMetadata) {
            results->videoColourMasteringMetadata.valid = true;

            pmeta = &BMKV_TABLE_ELEM(colour.MasteringMetadata, bmkv_TrackEntryVideoColourMasteringMetadata, 0);

            if(pmeta->validate.PrimaryRChromaticityX) {
                results->videoColourMasteringMetadata.masteringDisplayColorVolume.redPrimary.x = (int)
                    ((pmeta->PrimaryRChromaticityX/2.0)*10000.0);
            }
            if(pmeta->validate.PrimaryRChromaticityY) {
                results->videoColourMasteringMetadata.masteringDisplayColorVolume.redPrimary.y = (int)
                    ((pmeta->PrimaryRChromaticityY/2.0)*10000.0);
            }

            if(pmeta->validate.PrimaryGChromaticityX) {
                results->videoColourMasteringMetadata.masteringDisplayColorVolume.greenPrimary.x = (int)
                    ((pmeta->PrimaryGChromaticityX/2.0)*10000.0);
            }
            if(pmeta->validate.PrimaryGChromaticityY) {
                results->videoColourMasteringMetadata.masteringDisplayColorVolume.greenPrimary.y = (int)
                    ((pmeta->PrimaryGChromaticityY/2.0)*10000.0);
            }

            if(pmeta->validate.PrimaryBChromaticityX) {
                results->videoColourMasteringMetadata.masteringDisplayColorVolume.bluePrimary.x = (int)
                    ((pmeta->PrimaryBChromaticityX/2.0)*10000.0);
            }
            if(pmeta->validate.PrimaryBChromaticityY) {
                results->videoColourMasteringMetadata.masteringDisplayColorVolume.bluePrimary.y = (int)
                    ((pmeta->PrimaryBChromaticityY/2.0)*10000.0);
            }

            if(pmeta->validate.WhitePointChromaticityX) {
                results->videoColourMasteringMetadata.masteringDisplayColorVolume.whitePoint.x = (int)
                    ((pmeta->WhitePointChromaticityX/2.0)*10000.0);
            }
            if(pmeta->validate.WhitePointChromaticityY) {
                results->videoColourMasteringMetadata.masteringDisplayColorVolume.whitePoint.y = (int)
                    ((pmeta->WhitePointChromaticityY/2.0)*10000.0);
            }

            if(pmeta->validate.LuminanceMax) {
                results->videoColourMasteringMetadata.masteringDisplayColorVolume.luminance.max = (unsigned)
                    pmeta->LuminanceMax;
            }
            if(pmeta->validate.LuminanceMin) {
                results->videoColourMasteringMetadata.masteringDisplayColorVolume.luminance.min = (unsigned)
                    (pmeta->LuminanceMin*10000.0);
            }
        }
        bmkv_element_shutdown(bmkv_TrackEntryVideoColour_desc.entries, &colour);
    }
}
#endif

int probe_media_request(const struct probe_request *request, struct probe_results *results)
{
#if NEXUS_HAS_PLAYBACK
    bmedia_probe_t probe;
    bfile_io_read_t fd = NULL;
    bfile_io_read_t fd2 = NULL;
    bfile_crypto_t cfd = NULL;
    FILE *fin;
    bmedia_probe_config probe_config;
    const bmedia_probe_track *track;
    const bmedia_probe_stream *stream;
    NEXUS_FilePlayHandle file;
    int rc = 0;
    const char *indexname = NULL;
    unsigned cur_program_offset = 0, prev_program = 0;
    bool prev_program_set = false;

    memset(results, 0, sizeof(*results));

    probe = bmedia_probe_create();
    if (!probe) {
        return BERR_TRACE(-1);
    }

    fin = fopen64(request->streamname, "rb");
    if (!fin) {
        BDBG_ERR(("can't open '%s'", request->streamname));
        bmedia_probe_destroy(probe);
        return -1;
    }

    fd = bfile_stdio_read_attach(fin);

    if (request->decrypt.algo != NEXUS_SecurityAlgorithm_eMax) {
        cfd = bfile_crypto_create(request->decrypt.algo, request->decrypt.key.data, request->decrypt.key.size);
        if (!cfd) {
            BDBG_ERR(("no crypto supported"));
            return -1;
        }
        bfile_crypto_attach(cfd, fd);
        fd2 = (bfile_io_read_t)cfd;
    }
    else {
        fd2 = fd;
    }

    bmedia_probe_default_cfg(&probe_config);
    probe_config.file_name = request->streamname;
    probe_config.type = bstream_mpeg_type_unknown;
    stream = bmedia_probe_parse(probe, fd2, &probe_config);

    if (cfd) {
        bfile_crypto_detach(cfd);
        bfile_crypto_destroy(cfd);
    }

    /* now stream is either NULL, or stream descriptor with linked list of audio/video tracks */
    bfile_stdio_read_detach(fd);

    fclose(fin);
    if(!stream) {
        BDBG_ERR(("media probe can't parse '%s'", request->streamname));
        bmedia_probe_destroy(probe);
        return -1;
    }

    if (!request->quiet) b_print_media_string(stream);

    if (stream->index == bmedia_probe_index_available || stream->index == bmedia_probe_index_required) {
        /* if user didn't specify an index, use the file as index if probe indicates */
        indexname = request->streamname;
        results->useStreamAsIndex = true;
    }
    file = NEXUS_FilePlay_OpenPosix(request->streamname, indexname);
    if (!file) {
        BDBG_ERR(("can't open '%s'", request->streamname));
        rc = -1;
        goto error;
    }

    results->transportType = b_mpegtype2nexus(stream->type);
    if (stream->type == bstream_mpeg_type_ts) {
        if ((((bmpeg2ts_probe_stream*)stream)->pkt_len) == 192) {
            results->timestampType = NEXUS_TransportTimestampType_eBinary;
        }
    }

    for(track=BLST_SQ_FIRST(&stream->tracks);track;track=BLST_SQ_NEXT(track, link)) {
        if ((track->type == bmedia_track_type_audio || track->type == bmedia_track_type_video) && track->program != (unsigned)-1) {
            if (prev_program_set) {
                if (track->program != prev_program) {
                    cur_program_offset++;
                }
            }
            prev_program = track->program;
            prev_program_set = true;
        }
        if (cur_program_offset != request->program) continue;

        switch(track->type) {
            case bmedia_track_type_audio:
                if(track->info.audio.codec != baudio_format_unknown && results->num_audio < sizeof(results->audio)/sizeof(results->audio[0])) {
                    results->audio[results->num_audio].pid = track->number;
                    results->audio[results->num_audio].codec = b_audiocodec2nexus(track->info.audio.codec);
                    results->num_audio++;
                }
                break;
            case bmedia_track_type_video:
                if (track->info.video.codec == bvideo_codec_h264_svc || track->info.video.codec == bvideo_codec_h264_mvc) {
                    unsigned index = results->num_video ? results->num_video-1:0;
                    results->video[index].enhancement.pid = track->number;
                    results->video[index].enhancement.codec = b_videocodec2nexus(track->info.video.codec);
                }
                else if (track->info.video.codec != bvideo_codec_unknown && results->num_video < sizeof(results->video)/sizeof(results->video[0])) {
                    results->video[results->num_video].pid = track->number;
                    results->video[results->num_video].codec = b_videocodec2nexus(track->info.video.codec);
                    results->video[results->num_video].width = track->info.video.width;
                    results->video[results->num_video].height = track->info.video.height;
                    results->video[results->num_video].colorDepth  = bmedia_probe_get_video_color_depth(track);
#if B_HAS_ASF
                    if (stream->type == bstream_mpeg_type_asf) {
                        basf_probe_track *asf_track = (basf_probe_track *)track;
                        if (asf_track->aspectRatioValid) {
                            results->video[results->num_video].aspectRatio = NEXUS_AspectRatio_eSar;
                            results->video[results->num_video].sampleAspectRatio.x = asf_track->aspectRatio.x;
                            results->video[results->num_video].sampleAspectRatio.y = asf_track->aspectRatio.y;
                        }
                        if (asf_track->averageTimePerFrame) {
                            uint64_t framerate = 10*(1000000000/asf_track->averageTimePerFrame);
                            NEXUS_LookupFrameRate((unsigned)framerate, &results->video[results->num_video].frameRate);
                        }
#if 0
/* TODO: see nexus/utils/playback impl of the same */
                        if(asf_track->dynamicRangeControlValid) {
                            opts->dynamicRangeControlValid = true;
                            opts->dynamicRangeControl.peakReference = asf_track->dynamicRangeControl.peakReference;
                            opts->dynamicRangeControl.peakTarget = asf_track->dynamicRangeControl.peakTarget;
                            opts->dynamicRangeControl.averageReference = asf_track->dynamicRangeControl.averageReference;
                            opts->dynamicRangeControl.averageTarget = asf_track->dynamicRangeControl.averageTarget;
                        }
#endif
                    }
#endif
                    if (stream->type == bstream_mpeg_type_mkv) {
                        bmkv_probe_track *mkv_track = (bmkv_probe_track *)track;
                        probe_p_parse_mkv_block(mkv_track->data.video.Colour.data, mkv_track->data.video.Colour.len, results);
                    }
                    results->num_video++;
                }
                break;
            case bmedia_track_type_other:
                results->other[results->num_other].pid = track->number;
                results->num_other++;
                break;
            default:
                break;
        }
    }

#if B_HAS_AVI
    if (stream->type == bstream_mpeg_type_avi && ((bavi_probe_stream *)stream)->video_framerate) {
        NEXUS_LookupFrameRate(((bavi_probe_stream *)stream)->video_framerate, &results->video[0].frameRate);
    }
#endif


    /* enable stream processing for MP3 ES audio streams */
    if (results->transportType == NEXUS_TransportType_eEs &&
        results->num_audio == 1 &&
        results->audio[0].codec == NEXUS_AudioCodec_eMp3)
    {
        results->enableStreamProcessing = true;
    }

error:
    if (file) {
        NEXUS_FilePlay_Close(file);
    }
    if (probe) {
        if (stream) {
            bmedia_probe_stream_free(probe, stream);
        }
        bmedia_probe_destroy(probe);
    }

    return rc;
#else /* NEXUS_HAS_PLAYBACK */
    BSTD_UNUSED(request);
    BSTD_UNUSED(results);
    return -1;
#endif
}
