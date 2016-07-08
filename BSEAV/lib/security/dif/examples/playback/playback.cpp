/******************************************************************************
 *    (c)2015-2016 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Example to playback DRM encrypted content using DRM Integration Framework
 *
 *****************************************************************************/

#include "nexus_platform.h"
#include "nexus_video_decoder.h"
#include "nexus_stc_channel.h"
#include "bgui.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_video_input.h"
#include "nexus_spdif_output.h"
#include "nexus_component_output.h"
#include "nexus_video_adj.h"

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

#include "nxclient.h"
#include "nexus_surface_client.h"

BDBG_MODULE(playback_dif);
#define LOG_TAG "playback_dif"
#undef LOGE
#undef LOGW
#undef LOGD
#undef LOGV
#define LOGE BDBG_ERR
#define LOGW BDBG_WRN
#define LOGD BDBG_MSG
#define LOGV BDBG_MSG

// DRM Integration Framework
//#include "secure_playback.h"
#include "decryptor.h"
#include "cenc_parser.h"
#include "piff_parser.h"

#include "dump_hex.h"

#define ZORDER_TOP 10

#define MAX_MOSAICS 5 // FIXME: Fails to get key slots when >5

#define REPACK_VIDEO_PES_ID 0xE0
#define REPACK_AUDIO_PES_ID 0xC0

#define BUF_SIZE (1024 * 1024 * 2) /* 2MB */

#define CALCULATE_PTS(t) (((uint64_t)(t) / 10000LL) * 45LL)

using namespace dif_streamer;
using namespace media_parser;

static bool shouldExit = false;
static bool secure_video = false;

static void sigHandler(int sig) {
    LOGW(("Caught Siginal %d", sig));

    // To avoid being killed by multiple signals
    signal(SIGINT, SIG_IGN);
    signal(SIGTERM, SIG_IGN);

    shouldExit = true;
}

class AppContext
{
public:
    AppContext();
    ~AppContext();

    MediaParser* parser[MAX_MOSAICS];
    IDecryptor* decryptor[MAX_MOSAICS];
    bool use_default_url[MAX_MOSAICS];
    IStreamer* videoStreamer[MAX_MOSAICS];
    IStreamer* audioStreamer[MAX_MOSAICS];

    uint8_t *pAvccHdr;
    uint8_t *pPayload;
    uint8_t *pAudioHeaderBuf;
    uint8_t *pVideoHeaderBuf;

    FILE *fp_mp4[MAX_MOSAICS];
    char* file[MAX_MOSAICS];
    DrmType drmType[MAX_MOSAICS];
    NEXUS_SimpleVideoDecoderHandle videoDecoder[MAX_MOSAICS];
    NEXUS_SimpleAudioDecoderHandle audioDecoder[MAX_MOSAICS];
    NEXUS_PlaypumpHandle videoPlaypump[MAX_MOSAICS];
    NEXUS_PlaypumpHandle audioPlaypump[MAX_MOSAICS];
    NEXUS_PidChannelHandle videoPidChannel[MAX_MOSAICS];
    NEXUS_PidChannelHandle audioPidChannel[MAX_MOSAICS];
    BKNI_EventHandle event;

    uint64_t last_video_fragment_time;
    uint64_t last_audio_fragment_time;

    unsigned connectId[MAX_MOSAICS];
    NxClient_AllocResults s_allocResults;

    bgui_t gui;
    struct {
        NEXUS_SurfaceClientHandle video_sc;
        NEXUS_Rect rect;
        unsigned zorder;
        bool done;
    } mosaic[MAX_MOSAICS];
    unsigned num_mosaics;
};

AppContext::AppContext()
{
    gui = NULL;

    pAvccHdr = NULL;
    pPayload = NULL;
    pAudioHeaderBuf = NULL;
    pVideoHeaderBuf = NULL;


    for (int i = 0; i < MAX_MOSAICS; i++) {
        file[i] = NULL;
        drmType[i] = drm_type_eUnknown;
        parser[i] = NULL;
        decryptor[i] = NULL;
        use_default_url[i] = false;
        fp_mp4[i] = NULL;
        videoStreamer[i] = NULL;
        audioStreamer[i] = NULL;
        videoDecoder[i] = NULL;
        audioDecoder[i] = NULL;
        videoPidChannel[i] = NULL;
        audioPidChannel[i] = NULL;
        connectId[i] = 0xFFFF;
        mosaic[i].zorder = ZORDER_TOP;
        mosaic[i].done = false;
    }
    event = NULL;

    last_video_fragment_time = 0;
    last_audio_fragment_time = 0;
}

AppContext::~AppContext()
{
    LOGW(("%s: start cleaning up", __FUNCTION__));
    for (int i = 0; i < MAX_MOSAICS; i++) {
        if (videoDecoder[i]) {
            NEXUS_SimpleVideoDecoder_Stop(videoDecoder[i]);
        }
        if (audioDecoder[i]) {
            NEXUS_SimpleAudioDecoder_Stop(audioDecoder[i]);
        }
    }

    if (pAvccHdr) NEXUS_Memory_Free(pAvccHdr);
    if (pPayload) NEXUS_Memory_Free(pPayload);
    if (pAudioHeaderBuf) NEXUS_Memory_Free(pAudioHeaderBuf);
    if (pVideoHeaderBuf) NEXUS_Memory_Free(pVideoHeaderBuf);

    for (int i = 0; i < MAX_MOSAICS; i++) {
        if(parser[i]) {
            delete parser[i];
            LOGW(("Destroying parser %p", parser[i]));
        }

        if (fp_mp4[i]) fclose(fp_mp4[i]);

        if (decryptor[i] != NULL) {
            LOGW(("Destroying decryptor %p", decryptor[i]));
            DecryptorFactory::DestroyDecryptor(decryptor[i]);
        }

        if (videoStreamer[i] != NULL) {
            LOGW(("Destroying videoStreamer %p", videoStreamer[i]));
            StreamerFactory::DestroyStreamer(videoStreamer[i]);
        }

        if (audioStreamer[i] != NULL) {
            LOGW(("Destroying audioStreamer %p", audioStreamer[i]));
            StreamerFactory::DestroyStreamer(audioStreamer[i]);
        }

        if (videoDecoder[i] != NULL) {
            NEXUS_SimpleVideoDecoder_Release(videoDecoder[i]);
        }

        if (audioDecoder[i] != NULL) {
            NEXUS_SimpleAudioDecoder_Release(audioDecoder[i]);
        }

        if (connectId[i] != 0xFFFF) {
            NxClient_Disconnect(connectId[i]);
        }
    }

    if (event != NULL) {
        BKNI_DestroyEvent((BKNI_EventHandle)event);
    }

    if (gui != NULL) {
        bgui_destroy(gui);
    }
    NxClient_Free(&s_allocResults);
    NxClient_Uninit();
}


// Content Protection license server data
const std::string kCpLicenseServer =
    "http://widevine-proxy.appspot.com/proxy";
const std::string kCpClientAuth = "";

// Google Play license server data
const std::string kGpLicenseServer =
"http://dash-mse-test.appspot.com/api/drm/";
//    "http://license.uat.widevine.com/getlicense/widevine_test";
//    "https://jmt17.google.com/video/license/GetCencLicense";

// Test client authorization string.
// NOTE: Append a userdata attribute to place a unique marker that the
// server team can use to track down specific requests during debugging
// e.g., "<existing-client-auth-string>&userdata=<your-ldap>.<your-tag>"
//       "<existing-client-auth-string>&userdata=jbmr2.dev"
const std::string kGpWidevineAuth =
"widevine?drm_system=widevine&source=YOUTUBE&video_id=03681262dc412c06&ip=0.0.0.0&ipbits=0&expire=19000000000&sparams=ip,ipbits,expire,source,video_id,drm_system&signature=289105AFC9747471DB0D2A998544CC1DAF75B8F9.18DE89BB7C1CE9B68533315D0F84DF86387C6BB3&key=test_key1";
//    "?source=YOUTUBE&video_id=EGHC6OHNbOo&oauth=ya.gtsqawidevine";

const std::string kGpPlayreadyAuth =
"playready?drm_system=playready&source=YOUTUBE&video_id=03681262dc412c06&ip=0.0.0.0&ipbits=0&expire=19000000000&sparams=ip,ipbits,expire,drm_system,source,video_id&signature=3BB038322E72D0B027F7233A733CD67D518AF675.2B7C39053DA46498D23F3BCB87596EF8FD8B1669&key=test_key1";

const std::string kGpClientOfflineQueryParameters =
"";
//    "&offline=true";
const std::string kGpClientOfflineRenewalQueryParameters =
    "&offline=true&renewal=true";
const std::string kGpClientOfflineReleaseQueryParameters =
    "&offline=true&release=true";

static int video_decode_hdr=0;

static AppContext s_app;

static int parse_esds_config(uint8_t* pBuf, bmedia_info_aac *info_aac, size_t payload_size)
{
    bmedia_adts_header adts_header;

    bmedia_adts_header_init_aac(&adts_header, info_aac);
    adts_header.adts[2] = 0x50;
    bmedia_adts_header_fill(pBuf, &adts_header, payload_size);
    return 0;
}

static int parse_avcc_config(uint8_t *avcc_hdr, size_t *hdr_len, size_t *nalu_len,
        uint8_t *cfg_data, size_t cfg_data_size)
{
    bmedia_h264_meta meta;
    unsigned int i, sps_len, pps_len;
    uint8_t *data;
    uint8_t *dst;

    bmedia_read_h264_meta(&meta, cfg_data, cfg_data_size);

    *nalu_len = meta.nalu_len;

    data = (uint8_t *)meta.sps.data;
    dst = avcc_hdr;
    *hdr_len = 0;

    for(i = 0; i < meta.sps.no; i++)
    {
        sps_len = (((uint16_t)data[0]) <<8) | data[1];
        data += 2;
        /* Add NAL */
        BKNI_Memcpy(dst, bmp4_nal, sizeof(bmp4_nal)); dst += sizeof(bmp4_nal);
        /* Add SPS */
        BKNI_Memcpy(dst, data, sps_len);
        dst += sps_len;
        data += sps_len;
        *hdr_len += (sizeof(bmp4_nal) + sps_len);
    }

    data = (uint8_t *)meta.pps.data;
    for (i = 0; i < meta.pps.no; i++)
    {
        pps_len = (((uint16_t)data[0]) <<8) | data[1];
        data += 2;
        /* Add NAL */
        BKNI_Memcpy(dst, bmp4_nal, sizeof(bmp4_nal));
        dst += sizeof(bmp4_nal);
        /* Add PPS */
        BKNI_Memcpy(dst, data, pps_len);
        dst += pps_len;
        data += pps_len;
        *hdr_len += (sizeof(bmp4_nal) + pps_len);
    }
    return 0;
}

static int process_fragment(mp4_parse_frag_info *frag_info,
    void *decoder_data, size_t decoder_len, int index)
{
    IStreamer* streamer;
    SampleInfo *pSample;
    size_t pes_header_len;
    bmedia_pes_info pes_info;
    uint64_t frag_duration;
    uint32_t bytes_processed = 0;
    uint32_t last_bytes_processed = 0;
    if (frag_info->samples_info->sample_count == 0) {
        LOGE(("%s: No samples", __FUNCTION__));
        return -1;
    }

    LOGD(("%s: #samples=%d",__FUNCTION__, frag_info->samples_info->sample_count));
    for (unsigned i = 0; i < frag_info->samples_info->sample_count; i++) {
        uint32_t numOfByteDecrypted = 0;
        size_t sampleSize = 0;

        pSample = &frag_info->samples_info->samples[i];
        numOfByteDecrypted = sampleSize = frag_info->sample_info[i].size;

        if (frag_info->trackType == BMP4_SAMPLE_ENCRYPTED_VIDEO ||
            frag_info->trackType == BMP4_SAMPLE_AVC) {
            streamer = s_app.videoStreamer[index];
            /* H.264 Decoder configuration parsing */
            size_t avcc_hdr_size;
            size_t nalu_len = 0;
            parse_avcc_config(s_app.pAvccHdr, &avcc_hdr_size, &nalu_len, (uint8_t*)decoder_data, decoder_len);

            bmedia_pes_info_init(&pes_info, REPACK_VIDEO_PES_ID);
            frag_duration = s_app.last_video_fragment_time +
                (int32_t)frag_info->sample_info[i].composition_time_offset;
            s_app.last_video_fragment_time += frag_info->sample_info[i].duration;

            pes_info.pts_valid = true;
            pes_info.pts = (uint32_t)CALCULATE_PTS(frag_duration);

            if (video_decode_hdr == 0) {
                pes_header_len = bmedia_pes_header_init(s_app.pVideoHeaderBuf,
                    (sampleSize + avcc_hdr_size - nalu_len + sizeof(bmp4_nal)), &pes_info);
            } else {
                pes_header_len = bmedia_pes_header_init(s_app.pVideoHeaderBuf,
                    sampleSize - nalu_len + sizeof(bmp4_nal), &pes_info);
            }

            if (pes_header_len > 0) {
                IBuffer* output =
                    streamer->GetBuffer(pes_header_len);
                output->Copy(0, s_app.pVideoHeaderBuf, pes_header_len);
                BufferFactory::DestroyBuffer(output);
                bytes_processed += pes_header_len;
            }

            if (video_decode_hdr == 0) {
                IBuffer* output =
                    streamer->GetBuffer(avcc_hdr_size);
                output->Copy(0, s_app.pAvccHdr, avcc_hdr_size);
                BufferFactory::DestroyBuffer(output);
                bytes_processed += avcc_hdr_size;
                video_decode_hdr = 1;
            }

            IBuffer* input = BufferFactory::CreateBuffer(sampleSize, (uint8_t*)frag_info->cursor.cursor);

            /* Add NAL header per entry */
            uint32_t dst_offset = 0;
            do {
                uint32_t entryDataSize;
                entryDataSize = batom_cursor_uint32_be(&frag_info->cursor);
                input->Copy(dst_offset, (uint8_t*)bmp4_nal, sizeof(bmp4_nal));
                dst_offset += sizeof(entryDataSize);
                dst_offset += entryDataSize;
                batom_cursor_skip(&frag_info->cursor, entryDataSize);
            } while (dst_offset < sampleSize);

            IBuffer* decOutput = streamer->GetBuffer(sampleSize);
            if (s_app.decryptor[index]) {
                numOfByteDecrypted = s_app.decryptor[index]->DecryptSample(pSample,
                    input, decOutput, sampleSize);
            } else {
                decOutput->Copy(0, input, sampleSize);
                numOfByteDecrypted = sampleSize;
            }
            if (numOfByteDecrypted != sampleSize)
            {
                LOGE(("%s Failed to decrypt sample, can't continue - %d", __FUNCTION__, __LINE__));
                BufferFactory::DestroyBuffer(input);
                BufferFactory::DestroyBuffer(decOutput);
                return -1;
            }
            BufferFactory::DestroyBuffer(input);
            BufferFactory::DestroyBuffer(decOutput);
            bytes_processed += numOfByteDecrypted;
        } else if (frag_info->trackType == BMP4_SAMPLE_ENCRYPTED_AUDIO ||
            frag_info->trackType == BMP4_SAMPLE_MP4A) {
            streamer = s_app.audioStreamer[index];
            bmedia_pes_info_init(&pes_info, REPACK_AUDIO_PES_ID);
            frag_duration = s_app.last_audio_fragment_time +
                (int32_t)frag_info->sample_info[i].composition_time_offset;
            s_app.last_audio_fragment_time += frag_info->sample_info[i].duration;

            pes_info.pts_valid = true;
            pes_info.pts = (uint32_t)CALCULATE_PTS(frag_duration);

            pes_header_len = bmedia_pes_header_init(s_app.pAudioHeaderBuf,
                (sampleSize + BMEDIA_ADTS_HEADER_SIZE), &pes_info);

            /* AAC information parsing */
            bmedia_info_aac *info_aac = (bmedia_info_aac *)decoder_data;
            parse_esds_config(s_app.pAudioHeaderBuf + pes_header_len, info_aac, sampleSize);

            if (streamer) {
                IBuffer* output =
                    streamer->GetBuffer(pes_header_len + BMEDIA_ADTS_HEADER_SIZE);
                output->Copy(0, s_app.pAudioHeaderBuf, pes_header_len + BMEDIA_ADTS_HEADER_SIZE);
                bytes_processed += pes_header_len + BMEDIA_ADTS_HEADER_SIZE;
                BufferFactory::DestroyBuffer(output);
            }

            IBuffer* input = BufferFactory::CreateBuffer(sampleSize, (uint8_t*)frag_info->cursor.cursor);
            batom_cursor_skip(&frag_info->cursor, sampleSize);

            if (streamer) {
                IBuffer* decOutput = streamer->GetBuffer(sampleSize);
                if (s_app.decryptor[index]) {
                    numOfByteDecrypted = s_app.decryptor[index]->DecryptSample(pSample,
                        input, decOutput, sampleSize);
                } else {
                    decOutput->Copy(0, input, sampleSize);
                    numOfByteDecrypted = sampleSize;
                }
                if (numOfByteDecrypted != sampleSize)
                {
                    LOGE(("%s Failed to decrypt sample, can't continue - %d", __FUNCTION__, __LINE__));
                    BufferFactory::DestroyBuffer(input);
                    BufferFactory::DestroyBuffer(decOutput);
                    return -1;
                }
                BufferFactory::DestroyBuffer(input);
                BufferFactory::DestroyBuffer(decOutput);
                bytes_processed += numOfByteDecrypted;
            }
        } else {
            LOGW(("%s Unsupported track type %d detected", __FUNCTION__, frag_info->trackType));
            return -1;
        }

        if (streamer)
            streamer->Push(bytes_processed - last_bytes_processed);
        last_bytes_processed = bytes_processed;
    }

    return 0;
}

static void play_callback(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
}

static void wait_for_drain()
{
    LOGW(("%s: start", __FUNCTION__));
    NEXUS_Error rc;
    NEXUS_PlaypumpStatus playpumpStatus;
    int drained = 0;
    bool done[MAX_MOSAICS];

    for (int i = 0; i < s_app.num_mosaics; i++) {
        done[i] = false;
    }

    for (;;) {
        for (int i = 0; i < s_app.num_mosaics; i++) {
            if (!done[i]) {
                if (s_app.videoPlaypump[i] == NULL) {
                    done[i] = true;
                    drained++;
                    continue;
                }
                rc = NEXUS_Playpump_GetStatus(s_app.videoPlaypump[i], &playpumpStatus);
                if (rc != NEXUS_SUCCESS) {
                    done[i] = true;
                    drained++;
                }

                if (playpumpStatus.fifoDepth == 0 && !done[i]) {
                    done[i] = true;
                    drained++;
                }
            }
        }
        if (drained == s_app.num_mosaics)
            break;
        BKNI_Sleep(100);
    }
    LOGW(("%s: video playpump was drained", __FUNCTION__));

    drained = 0;
    for (int i = 0; i < s_app.num_mosaics; i++) {
        done[i] = false;
    }

    for (;;) {
        for (int i = 0; i < s_app.num_mosaics; i++) {
            if (!done[i]) {
                if (s_app.audioPlaypump[i] == NULL) {
                    done[i] = true;
                    drained++;
                    continue;
                }
                rc = NEXUS_Playpump_GetStatus(s_app.audioPlaypump[i], &playpumpStatus);
                if (rc != NEXUS_SUCCESS) {
                    done[i] = true;
                    drained++;
                }

                if (playpumpStatus.fifoDepth == 0 && !done[i]) {
                    done[i] = true;
                    drained++;
                }
            }
        }
        if (drained == s_app.num_mosaics)
            break;
        BKNI_Sleep(100);
    }
    LOGW(("%s: audio playpump was drained", __FUNCTION__));

    drained = 0;
    for (int i = 0; i < s_app.num_mosaics; i++) {
        done[i] = false;
    }

    for (;;) {
        NEXUS_VideoDecoderStatus decoderStatus;
        for (int i = 0; i < s_app.num_mosaics; i++) {
            if (!done[i]) {
                if (s_app.videoDecoder[i] == NULL) {
                    done[i] = true;
                    drained++;
                    continue;
                }
                rc = NEXUS_SimpleVideoDecoder_GetStatus(s_app.videoDecoder[i], &decoderStatus);
                if (rc != NEXUS_SUCCESS) {
                    done[i] = true;
                    drained++;
                }

                if (decoderStatus.queueDepth == 0 && !done[i]) {
                    done[i] = true;
                    drained++;
                }
            }
        }
        if (drained == s_app.num_mosaics)
            break;
        BKNI_Sleep(100);
    }
    LOGW(("%s: video decoder was drained", __FUNCTION__));

    drained = 0;
    for (int i = 0; i < s_app.num_mosaics; i++) {
        done[i] = false;
    }

    for (;;) {
        NEXUS_AudioDecoderStatus decoderStatus;
        for (int i = 0; i < s_app.num_mosaics; i++) {
            if (!done[i]) {
                if (s_app.audioDecoder[i] == NULL) {
                    done[i] = true;
                    drained++;
                    continue;
                }
                rc = NEXUS_SimpleAudioDecoder_GetStatus(s_app.audioDecoder[i], &decoderStatus);
                if (rc != NEXUS_SUCCESS) {
                    done[i] = true;
                    drained++;
                }

                if (decoderStatus.queuedFrames < 4 && !done[i]) {
                    done[i] = true;
                    drained++;
                }
            }
        }
        if (drained == s_app.num_mosaics)
            break;

        BKNI_Sleep(100);
    }
    LOGW(("%s: audio decoder was drained", __FUNCTION__));

    // wait for 1 sec to stabilize
    BKNI_Sleep(1000);
    return;
}

static void setup_gui()
{
    NEXUS_Error rc;
    unsigned num_columns, num_rows;
    NxClient_AllocSettings allocSettings;
    NEXUS_SurfaceRegion virtualDisplay = {1280, 720};

    s_app.gui = bgui_create(virtualDisplay.width, virtualDisplay.height);

    num_columns = (s_app.num_mosaics + 1) / 2;
    num_rows = (s_app.num_mosaics == 1) ? 1 : 2;

    for (int i = 0; i < s_app.num_mosaics; i++) {
        s_app.mosaic[i].video_sc = NEXUS_SurfaceClient_AcquireVideoWindow(bgui_surface_client(s_app.gui), i);
        if (!s_app.mosaic[i].video_sc) {
            LOGE(("unable to create window %d", i));
            rc = -1;
        }

        s_app.mosaic[i].rect.width = virtualDisplay.width / num_columns;
        s_app.mosaic[i].rect.height = virtualDisplay.height / num_rows;
        s_app.mosaic[i].rect.x = s_app.mosaic[i].rect.width * (i % num_columns);
        s_app.mosaic[i].rect.y = s_app.mosaic[i].rect.height * (i / num_columns);
        LOGW(("rect[%d] %dx%d %d,%d", i,
            s_app.mosaic[i].rect.width, s_app.mosaic[i].rect.height,
            s_app.mosaic[i].rect.x, s_app.mosaic[i].rect.y));

        unsigned max_height = s_app.mosaic[i].rect.width*3/4;
        if (s_app.mosaic[i].rect.height > max_height) {
            s_app.mosaic[i].rect.y += (s_app.mosaic[i].rect.height - max_height) / 2;
            s_app.mosaic[i].rect.height = max_height;
        }

        NEXUS_SurfaceClientSettings settings;
        NEXUS_SurfaceClient_GetSettings(s_app.mosaic[i].video_sc, &settings);
        settings.composition.position = s_app.mosaic[i].rect;
        settings.composition.zorder = s_app.mosaic[i].zorder;
        settings.composition.virtualDisplay = virtualDisplay;
        NEXUS_SurfaceClient_SetSettings(s_app.mosaic[i].video_sc, &settings);
    }

    NxClient_GetDefaultAllocSettings(&allocSettings);
    allocSettings.simpleVideoDecoder = s_app.num_mosaics;
    rc = NxClient_Alloc(&allocSettings, &s_app.s_allocResults);
    if (rc)
        exit(EXIT_FAILURE);

    for (int i = 0; i < s_app.num_mosaics; i++) {
        if (s_app.s_allocResults.simpleVideoDecoder[i].id) {
            LOGD(("@@@ to acquire video decoder %d for %d", s_app.s_allocResults.simpleVideoDecoder[i].id, i));
            s_app.videoDecoder[i] = NEXUS_SimpleVideoDecoder_Acquire(s_app.s_allocResults.simpleVideoDecoder[i].id);
        }
        if (s_app.videoDecoder[i] == NULL) {
            LOGE(("video decoder not available %d", i));
            exit(EXIT_FAILURE);
        }

        NxClient_AllocResults audioAllocResults;
        if (i == 0) {
            NxClient_AllocSettings audioAllocSettings;
            NxClient_GetDefaultAllocSettings(&audioAllocSettings);
            audioAllocSettings.simpleAudioDecoder = 1;
            rc = NxClient_Alloc(&audioAllocSettings, &audioAllocResults);
            if (audioAllocResults.simpleAudioDecoder.id) {
                LOGD(("@ to acquire audio decoder %d", audioAllocResults.simpleAudioDecoder.id));
                s_app.audioDecoder[i] = NEXUS_SimpleAudioDecoder_Acquire(audioAllocResults.simpleAudioDecoder.id);
            } else if (i == 0) {
                if (s_app.s_allocResults.simpleAudioDecoder.id) {
                    LOGD(("@@ to acquire audio decoder %d", s_app.s_allocResults.simpleAudioDecoder.id));
                    s_app.audioDecoder[i] = NEXUS_SimpleAudioDecoder_Acquire(s_app.s_allocResults.simpleAudioDecoder.id);
                }
            }
            if (s_app.audioDecoder[i] == NULL) {
                LOGE(("audio decoder not available %d", i));
                exit(EXIT_FAILURE);
            }
        }

        NxClient_ConnectSettings connectSettings;
        NxClient_GetDefaultConnectSettings(&connectSettings);
        for (int j = 0; j < s_app.num_mosaics; j++) {
            connectSettings.simpleVideoDecoder[j].id = s_app.s_allocResults.simpleVideoDecoder[j].id;
            connectSettings.simpleVideoDecoder[j].surfaceClientId = bgui_surface_client_id(s_app.gui);
            connectSettings.simpleVideoDecoder[j].windowId = j;
            connectSettings.simpleVideoDecoder[j].decoderCapabilities.maxWidth = s_app.mosaic[j].rect.width;
            connectSettings.simpleVideoDecoder[j].decoderCapabilities.maxHeight = s_app.mosaic[j].rect.height;
            if (secure_video)
            {
                connectSettings.simpleVideoDecoder[j].decoderCapabilities.secureVideo = true;
            }
        }
        if (i == 0)
            connectSettings.simpleAudioDecoder.id = audioAllocResults.simpleAudioDecoder.id;
        rc = NxClient_Connect(&connectSettings, &s_app.connectId[i]);
        if (rc)
            exit(EXIT_FAILURE);
    }

    NEXUS_Graphics2DHandle gfx = bgui_blitter(s_app.gui);
    NEXUS_Graphics2DFillSettings fillSettings;

    NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
    fillSettings.surface = bgui_surface(s_app.gui);
    fillSettings.color = 0;
    rc = NEXUS_Graphics2D_Fill(gfx, &fillSettings);
    BDBG_ASSERT(!rc);

    bgui_checkpoint(s_app.gui);
    bgui_submit(s_app.gui);
}

static void setup_streamers()
{
    NEXUS_ClientConfiguration clientConfig;
    NEXUS_MemoryAllocationSettings memSettings;
    NEXUS_SimpleStcChannelHandle stcChannel;
    NEXUS_SimpleVideoDecoderStartSettings videoProgram;
    NEXUS_SimpleAudioDecoderStartSettings audioProgram;
    NEXUS_SimpleStcChannelSettings stcSettings;
    NEXUS_PlaypumpSettings playpumpSettings;
    NEXUS_Error rc;

    // Create Streamers
    for (int i = 0; i < s_app.num_mosaics; i++) {
        s_app.videoStreamer[i] = StreamerFactory::CreateStreamer();
        if (i == 0) // FIXME: set up audio only for the first one
            s_app.audioStreamer[i] = StreamerFactory::CreateStreamer();
        if (s_app.videoStreamer[i] == NULL) {
            exit(EXIT_FAILURE);
        }

        if (s_app.mosaic[i].rect.height > 576)
            s_app.videoPlaypump[i] = s_app.videoStreamer[i]->OpenPlaypump(true);
        else
            s_app.videoPlaypump[i] = s_app.videoStreamer[i]->OpenPlaypump(false);

        if (s_app.audioStreamer[i])
            s_app.audioPlaypump[i] = s_app.audioStreamer[i]->OpenPlaypump(false);
    }

    LOGD(("%s - %d\n", __FUNCTION__, __LINE__));

    NEXUS_Platform_GetClientConfiguration(&clientConfig);
    NEXUS_Memory_GetDefaultAllocationSettings(&memSettings);
    memSettings.heap = clientConfig.heap[1]; /* heap 1 is the eFull heap for the nxclient. */

    s_app.pAvccHdr = NULL;
    if (NEXUS_Memory_Allocate(BMP4_MAX_PPS_SPS, &memSettings, (void **)&s_app.pAvccHdr) != NEXUS_SUCCESS) {
        fprintf(stderr,"NEXUS_Memory_Allocate failed");
        exit(EXIT_FAILURE);
    }

    s_app.pPayload = NULL;
    if (NEXUS_Memory_Allocate(BUF_SIZE, &memSettings, (void **)&s_app.pPayload) != NEXUS_SUCCESS) {
        fprintf(stderr,"NEXUS_Memory_Allocate failed");
        exit(EXIT_FAILURE);
    }

    s_app.pAudioHeaderBuf = NULL;
    if (NEXUS_Memory_Allocate(BMEDIA_PES_HEADER_MAX_SIZE + BMP4_MAX_PPS_SPS,
        &memSettings, (void **)&s_app.pAudioHeaderBuf) != NEXUS_SUCCESS) {
        fprintf(stderr,"NEXUS_Memory_Allocate failed");
        exit(EXIT_FAILURE);
    }

    s_app.pVideoHeaderBuf = NULL;
    if (NEXUS_Memory_Allocate(BMEDIA_PES_HEADER_MAX_SIZE + BMP4_MAX_PPS_SPS,
        &memSettings, (void **)&s_app.pVideoHeaderBuf) != NEXUS_SUCCESS) {
        fprintf(stderr,"NEXUS_Memory_Allocate failed");
        exit(EXIT_FAILURE);
    }

    BKNI_CreateEvent(&s_app.event);

    // Set up Playpumps and Decoders
    for (int i = 0; i < s_app.num_mosaics; i++) {
        stcChannel = NEXUS_SimpleStcChannel_Create(NULL);
        NEXUS_SimpleStcChannel_GetSettings(stcChannel, &stcSettings);
        stcSettings.mode = NEXUS_StcChannelMode_eAuto;
        rc = NEXUS_SimpleStcChannel_SetSettings(stcChannel, &stcSettings);
        if (rc) {
           LOGW(("@@@ Stc Set FAILED ---------------"));
        }

        NEXUS_Playpump_GetSettings(s_app.videoPlaypump[i], &playpumpSettings);
        playpumpSettings.dataCallback.callback = play_callback;
        playpumpSettings.dataCallback.context = s_app.event;
        playpumpSettings.transportType = NEXUS_TransportType_eMpeg2Pes;
        NEXUS_Playpump_SetSettings(s_app.videoPlaypump[i], &playpumpSettings);

        if (s_app.audioStreamer[i]) {
            NEXUS_Playpump_GetSettings(s_app.audioPlaypump[i], &playpumpSettings);
            playpumpSettings.dataCallback.callback = play_callback;
            playpumpSettings.dataCallback.context = s_app.event;
            playpumpSettings.transportType = NEXUS_TransportType_eMpeg2Pes;
            NEXUS_Playpump_SetSettings(s_app.audioPlaypump[i], &playpumpSettings);
        }

        NEXUS_Playpump_Start(s_app.videoPlaypump[i]);
        if (s_app.audioStreamer[i]) {
            NEXUS_Playpump_Start(s_app.audioPlaypump[i]);
        }

        NEXUS_PlaypumpOpenPidChannelSettings video_pid_settings;
        NEXUS_Playpump_GetDefaultOpenPidChannelSettings(&video_pid_settings);
        video_pid_settings.pidType = NEXUS_PidType_eVideo;

        s_app.videoPidChannel[i] = s_app.videoStreamer[i]->OpenPidChannel(REPACK_VIDEO_PES_ID, &video_pid_settings);

        if (!s_app.videoPidChannel)
            LOGW(("@@@ videoPidChannel NULL"));
        else
            LOGW(("@@@ videoPidChannel OK"));

        if (s_app.audioStreamer[i]) {
            s_app.audioPidChannel[i] = s_app.audioStreamer[i]->OpenPidChannel(REPACK_AUDIO_PES_ID, NULL);

            if (!s_app.audioPidChannel)
                LOGW(("@@@ audioPidChannel NULL"));
            else
                LOGW(("@@@ audioPidChannel OK"));

            NEXUS_SimpleAudioDecoder_GetDefaultStartSettings(&audioProgram);
        }
        NEXUS_SimpleVideoDecoder_GetDefaultStartSettings(&videoProgram);

        LOGW(("@@@ set video audio program for h264"));
        videoProgram.settings.codec = NEXUS_VideoCodec_eH264;
        audioProgram.primary.codec = NEXUS_AudioCodec_eAacAdts;

        videoProgram.maxWidth = s_app.mosaic[i].rect.width;
        videoProgram.maxHeight = s_app.mosaic[i].rect.height;

        videoProgram.settings.pidChannel = s_app.videoPidChannel[i];
        NEXUS_SimpleVideoDecoder_Start(s_app.videoDecoder[i], &videoProgram);

        if (s_app.audioStreamer[i]) {
            audioProgram.primary.pidChannel = s_app.audioPidChannel[i];
            NEXUS_SimpleAudioDecoder_Start(s_app.audioDecoder[i], &audioProgram);
        }

        if (videoProgram.settings.pidChannel) {
            LOGW(("@@@ set stc channel video"));
            NEXUS_SimpleVideoDecoder_SetStcChannel(s_app.videoDecoder[i], stcChannel);
        }

        if (s_app.audioStreamer[i]) {
            if (audioProgram.primary.pidChannel) {
                LOGW(("@@@ set stc channel audio"));
                NEXUS_SimpleAudioDecoder_SetStcChannel(s_app.audioDecoder[i], stcChannel);
            }
        }
    }

}

static void setup_parsers()
{
    // Set up Parsers
    for (int index = 0; index < s_app.num_mosaics; index++) {
        LOGW(("MP4 file: %s\n", s_app.file[index]));
        fflush(stdout);

        s_app.fp_mp4[index] = fopen(s_app.file[index], "rb");
        if (s_app.fp_mp4[index] == NULL) {
            LOGE(("failed to open %s", s_app.file[index]));
            exit(EXIT_FAILURE);
        }

        s_app.parser[index] = new PiffParser(s_app.fp_mp4[index]);
        if (s_app.parser[index] ==  NULL) {
            LOGE(("failed to new a PiffParser instance"));
            exit(EXIT_FAILURE);
        }

        if (s_app.parser[index]->Initialize()) {
            LOGW(("PiffParser was initialized for %s", s_app.file[index]));
            s_app.use_default_url[index] = true;
        } else {
            LOGW(("failed to initialize the PiffParser for %s, try CencParser...", s_app.file[index]));
            delete s_app.parser[index];

            s_app.parser[index] = new CencParser(s_app.fp_mp4[index]);
            if (s_app.parser[index] ==  NULL) {
                LOGE(("failed to new a CencParser instance"));
                exit(EXIT_FAILURE);
            }

            if (!s_app.parser[index]->Initialize()) {
                LOGE(("failed to initialize the CencParser for %s", s_app.file[index]));
                exit(EXIT_FAILURE);
            }

            if(s_app.drmType[index] != drm_type_eUnknown){
                bool drmMatch = false;
                DrmType drmTypes[BMP4_MAX_DRM_SCHEMES];
                uint8_t numOfDrmSchemes = s_app.parser[index]->GetNumOfDrmSchemes(drmTypes, BMP4_MAX_DRM_SCHEMES);
                for(int j = 0; j < numOfDrmSchemes; j++){
                    if(drmTypes[j] == s_app.drmType[index]){
                        drmMatch = true;
                        s_app.parser[index]-> SetDrmSchemes(j);
                    }
                }
                if(!drmMatch){
                    LOGE(("DRM Type: %d was not found in the stream.", s_app.drmType[index] ));
                    LOGE(("Do you want to play it with its default DRM type: %d? [y/n]", drmTypes[0]));
                    char resp;
                    scanf("%c", &resp);
                    if(resp != 'y')
                        exit(EXIT_FAILURE);
                }
            }
        }
    }
}

static void setup_decryptors()
{
    std::string cpsSpecificData;    /*content protection system specific data*/
    std::string psshDataStr;
    DrmType drmType;

    std::string license_server;
    std::string key_response;

    // Set up Decryptors
    for (int i = 0; i < s_app.num_mosaics; i++) {
        // New API - creating Decryptor
        drmType = s_app.parser[i]->GetDrmType();
        if (drmType == drm_type_eWidevine) {
            LOGW(("Widevine for %s", s_app.file[i]));
        } else if (drmType == drm_type_ePlayready) {
            LOGW(("Playready for %s", s_app.file[i]));
        } else {
            LOGW(("Unknown DRM type for %s", s_app.file[i]));
            continue;
        }

        s_app.decryptor[i] = DecryptorFactory::CreateDecryptor(drmType);
        if (s_app.decryptor[i] == NULL) {
            LOGE(("Failed to create Decryptor"));
            exit(EXIT_FAILURE);
        }

        // New API - initializing Decryptor
        psshDataStr = s_app.parser[i]->GetPssh();
        dump_hex("pssh", psshDataStr.data(), psshDataStr.size());
        if (!s_app.decryptor[i]->Initialize(psshDataStr)) {
            LOGE(("Failed to initialize Decryptor"));
            exit(EXIT_FAILURE);
        }

        // New API - GenerateKeyRequest
        cpsSpecificData = s_app.parser[i]->GetCpsSpecificData();
        dump_hex("cpsSpecificData", cpsSpecificData.data(), cpsSpecificData.size());
        if (!s_app.decryptor[i]->GenerateKeyRequest(cpsSpecificData)) {
            LOGE(("Failed to generate key request"));
            exit(EXIT_FAILURE);
        }

        dump_hex("message", s_app.decryptor[i]->GetKeyMessage().data(), s_app.decryptor[i]->GetKeyMessage().size());

        if (s_app.use_default_url[i]) {
            license_server.clear();
        } else {
            // Use Google Play server
            if (drmType == drm_type_eWidevine) {
                license_server.assign(kGpLicenseServer + kGpWidevineAuth);
            } else if (drmType == drm_type_ePlayready) {
                license_server.assign(kGpLicenseServer + kGpPlayreadyAuth);
            }
            license_server.append(kGpClientOfflineQueryParameters);
        }

        LOGW(("License Server for %s: %s", s_app.file[i], license_server.c_str()));

        // New API - GetKeyRequestResponse
        key_response = s_app.decryptor[i]->GetKeyRequestResponse(license_server);

        // New API - AddKey
        if (!s_app.decryptor[i]->AddKey(key_response)) {
            LOGE(("Failed to add key"));
            exit(EXIT_FAILURE);
        }
        dump_hex("key_response", key_response.data(), key_response.size());
    }

}

void playback_loop()
{
    mp4_parse_frag_info frag_info;
    void *decoder_data;
    size_t decoder_len;

    int done_count = 0;
    for (;;) {
        for (int i = 0; i < s_app.num_mosaics; i++) {
            if (!s_app.mosaic[i].done) {
                decoder_data = s_app.parser[i]->GetFragmentData(frag_info, s_app.pPayload, decoder_len);
                if (decoder_data == NULL) {
                    s_app.mosaic[i].done = true;
                    done_count++;
                    LOGW(("stream %d done", i));
                    continue;
                }

                if (process_fragment(&frag_info, decoder_data, decoder_len, i) != 0)
                    exit(EXIT_FAILURE);
            }
            if (shouldExit)
                return;
            if (done_count >= s_app.num_mosaics) {
                wait_for_drain();
                return;
            }
        }
    }
}

static void rewind_parsers()
{
    for (int i = 0; i < s_app.num_mosaics; i++) {
        // FIXME: rewinding file pointer produces memory leaks in Parser
        //rewind(s_app.fp_mp4[i]);
        delete s_app.parser[i];
        s_app.parser[i] = NULL;
        s_app.mosaic[i].done = false;
    }

    setup_parsers();
}

static void print_usage(char* command)
{
    LOGE(("Usage : %s <files> [OPTIONS]", command));
    LOGE(("        -pr <file> Set DRM type as playready"));
    LOGE(("        -wv <file> Set DRM type as widevine"));
    LOGE(("        -loop N    Set # of playback loops"));
    LOGE(("        -secure    Use secure video picture buffers (URR)"));
    LOGE(("        -n N       Set # of mosaics, max=%d", MAX_MOSAICS));
}

int main(int argc, char* argv[])
{
    signal(SIGINT, sigHandler);
    signal(SIGTERM, sigHandler);

    NxClient_JoinSettings joinSettings;
    NEXUS_Error rc;
    unsigned num_files = 0;
    unsigned num_loops = 1;

    NxClient_GetDefaultJoinSettings(&joinSettings);
    snprintf(joinSettings.name, NXCLIENT_MAX_NAME, "playback_dif");
    joinSettings.ignoreStandbyRequest = true;

    rc = NxClient_Join(&joinSettings);
    if (rc)
        exit(EXIT_FAILURE);

    s_app.num_mosaics = 1;

    if (argc < 2) {
        print_usage(argv[0]);
        exit(EXIT_FAILURE);
    }

    for (int i = 1; i < argc; i++){
        if (strcmp(argv[i], "-pr") == 0) {
            if (i >= argc - 1) {
                print_usage(argv[0]);
                exit(EXIT_FAILURE);
            }
            s_app.drmType[num_files] = drm_type_ePlayready;
            s_app.file[num_files] = argv[++i];
            LOGW(("File[%d] PR: %s", num_files, s_app.file[num_files]));
            num_files++;
        }
        else if (strcmp(argv[i], "-wv") == 0) {
            if (i >= argc - 1) {
                print_usage(argv[0]);
                exit(EXIT_FAILURE);
            }
            s_app.drmType[num_files] = drm_type_eWidevine;
            s_app.file[num_files] = argv[++i];
            LOGW(("File[%d] WV: %s", num_files, s_app.file[num_files]));
            num_files++;
        }
        else if (strcmp(argv[i], "-loop") == 0) {
            if (i >= argc - 1) {
                print_usage(argv[0]);
                exit(EXIT_FAILURE);
            }
            num_loops = atoi(argv[++i]);
            LOGW(("# of playback loops: %u", num_loops));
        }
        else if (strcmp(argv[i], "-secure") == 0)
            secure_video = true;
        else if (!strcmp(argv[i], "-n")) {
            if (i >= argc - 1) {
                print_usage(argv[0]);
                exit(EXIT_FAILURE);
            }
            s_app.num_mosaics = atoi(argv[++i]);
            if (s_app.num_mosaics > MAX_MOSAICS ||
                s_app.num_mosaics == 0) {
                LOGE(("Invalid # of mosaics: please specify 1-%d", MAX_MOSAICS));
                print_usage(argv[0]);
                exit(EXIT_FAILURE);
            }
        }
        else {
            s_app.file[num_files] = argv[i];
            LOGW(("File[%d]: %s", num_files, s_app.file[num_files]));
            num_files++;
        }
    }

    if (num_files == 0) {
        print_usage(argv[0]);
        exit(EXIT_FAILURE);
    }

    if (num_files < s_app.num_mosaics) {
        LOGW(("num_files=%d num_mosaics=%d", num_files, s_app.num_mosaics));
        int remaining = s_app.num_mosaics - num_files;
        for (int i = 1; remaining > 0; i++) {
            for (int j = 0; j < num_files && remaining > 0; j++) {
                s_app.file[num_files * i + j] = s_app.file[j];
                s_app.drmType[num_files * i + j] = s_app.drmType[j];
                remaining--;
            }
        }
    } else {
        LOGW(("num_mosaics set to %d", num_files));
        s_app.num_mosaics = num_files;
    }

    for (int i = 0; i < s_app.num_mosaics; i++) {
        LOGW(("File[%d] drmType(%d): %s", i, s_app.drmType[i], s_app.file[i]));
    }

    LOGD(("@@@ Check Point #01"));

    setup_gui();

    setup_streamers();

    setup_parsers();

    setup_decryptors();

    for (int i = 0; i < num_loops; i++) {
        LOGW(("Playback Loop: %d starting", i));
        playback_loop();
        if (shouldExit)
            break;
        LOGW(("Playback Loop: %d done", i));
        if (i < num_loops - 1)
            rewind_parsers();
    }

    exit(EXIT_SUCCESS);
}
