/******************************************************************************
 *  Copyright (C) 2018 Broadcom.
 *  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to
 *  the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied),
 *  right to use, or waiver of any kind with respect to the Software, and
 *  Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 *  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 *  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization,
 *  constitutes the valuable trade secrets of Broadcom, and you shall use all
 *  reasonable efforts to protect the confidentiality thereof, and to use this
 *  information only in connection with your use of Broadcom integrated circuit
 *  products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 *  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 *  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 *  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 *  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 *  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 *  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 *  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/

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
#include <errno.h>

#include <pthread.h>

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

#define MAX_MOSAICS 8

#ifdef DIF_SCATTER_GATHER
/* We hold multiple samples - greater than half of descFifoSize */
/* This is to avoid overwriting by processing the current sample */
#define NUM_SAMPLES_HELD 60

/* We hold a couple of buffers to prevent the previous fragment data */
/* from being overwritten by file io for the subsequent fragment */
#define NUM_FRAGMENTS_HELD 3
#endif

#define REPACK_VIDEO_PES_ID 0xE0
#define REPACK_AUDIO_PES_ID 0xC0

#define BUF_SIZE (1024 * 1024 * 4) /* 4MB */
#define VIDEO_PLAYPUMP_BUF_SIZE (1024 * 1024 * 2) /* 2MB */

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
    bool useGooglePlayLicServer[MAX_MOSAICS];
    IStreamer* videoStreamer[MAX_MOSAICS];
    IStreamer* audioStreamer[MAX_MOSAICS];
    int video_decode_hdr[MAX_MOSAICS];
    size_t nalu_len[MAX_MOSAICS];

    uint8_t *pAvccHdr[MAX_MOSAICS];
    uint8_t *pAudioHeaderBuf[MAX_MOSAICS];
    uint8_t *pVideoHeaderBuf[MAX_MOSAICS];
    pthread_t playback_thread[MAX_MOSAICS];
#ifdef DIF_SCATTER_GATHER
    uint32_t audio_idx[MAX_MOSAICS];
    IBuffer* audioPesBuf[MAX_MOSAICS][NUM_SAMPLES_HELD];
    uint32_t video_idx[MAX_MOSAICS];
    IBuffer* videoPesBuf[MAX_MOSAICS][NUM_SAMPLES_HELD];
    IBuffer* audioDecOut[MAX_MOSAICS][NUM_SAMPLES_HELD];
    IBuffer* videoDecOut[MAX_MOSAICS][NUM_SAMPLES_HELD];
    /* We use 2 buffers by turn to prevent the previous fragment data */
    /* from being overwritten by file io for the subsequent fragment */
    uint8_t *pPayload[MAX_MOSAICS][NUM_FRAGMENTS_HELD];
#else
    uint8_t *pPayload[MAX_MOSAICS];
#endif

    FILE *fp_mp4[MAX_MOSAICS];
    char* file[MAX_MOSAICS];
    DrmType parserDrmType[MAX_MOSAICS];
    DrmType decryptorDrmType[MAX_MOSAICS];
    NEXUS_SimpleVideoDecoderHandle videoDecoder[MAX_MOSAICS];
    NEXUS_SimpleAudioDecoderHandle audioDecoder[MAX_MOSAICS];
    NEXUS_SimpleStcChannelHandle stcChannel[MAX_MOSAICS];
    NEXUS_PlaypumpHandle videoPlaypump[MAX_MOSAICS];
    NEXUS_PlaypumpHandle audioPlaypump[MAX_MOSAICS];
    NEXUS_PidChannelHandle videoPidChannel[MAX_MOSAICS];
    NEXUS_PidChannelHandle audioPidChannel[MAX_MOSAICS];
    BKNI_EventHandle event;

    // Session Type - only for Wv3x
    SessionType sessionType[MAX_MOSAICS];
    std::string license[MAX_MOSAICS];

    uint64_t last_video_fragment_time;
    uint64_t last_audio_fragment_time;
    bool bypassAudio;

    unsigned connectId;
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

    for (int i = 0; i < MAX_MOSAICS; i++) {
        file[i] = NULL;
        parserDrmType[i] = drm_type_eUnknown;
        decryptorDrmType[i] = drm_type_eUnknown;
        parser[i] = NULL;
        decryptor[i] = NULL;
        useGooglePlayLicServer[i] = false;
        fp_mp4[i] = NULL;
        videoStreamer[i] = NULL;
        audioStreamer[i] = NULL;
        video_decode_hdr[i] = 0;
        nalu_len[i] = 0;
        videoDecoder[i] = NULL;
        audioDecoder[i] = NULL;
        stcChannel[i] = NULL;
        videoPidChannel[i] = NULL;
        audioPidChannel[i] = NULL;
        connectId = 0xFFFF;
        mosaic[i].zorder = ZORDER_TOP;
        mosaic[i].done = false;
        sessionType[i] = session_type_eTemporary;
        license[i].clear();
        pAvccHdr[i] = NULL;
        pAudioHeaderBuf[i] = NULL;
        pVideoHeaderBuf[i] = NULL;
#ifdef DIF_SCATTER_GATHER
        audio_idx[i] = 0;
        video_idx[i] = 0;
        for (int j = 0; j < NUM_SAMPLES_HELD; j++) {
            audioPesBuf[i][j] = NULL;
            videoPesBuf[i][j] = NULL;
            audioDecOut[i][j] = NULL;
            videoDecOut[i][j] = NULL;
        }
        for (int j = 0; j < NUM_FRAGMENTS_HELD; j++) {
            pPayload[i][j] = NULL;
        }
#else
        pPayload[i] = NULL;
#endif
    }

    event = NULL;

    last_video_fragment_time = 0;
    last_audio_fragment_time = 0;
    bypassAudio = false;
}

AppContext::~AppContext()
{
    LOGW(("%s: start cleaning up", BSTD_FUNCTION));
    for (int i = 0; i < MAX_MOSAICS; i++) {
        if (videoDecoder[i]) {
            NEXUS_SimpleVideoDecoder_Stop(videoDecoder[i]);
        }
        if (audioDecoder[i]) {
            NEXUS_SimpleAudioDecoder_Stop(audioDecoder[i]);
        }
    }

    for (int i = 0; i < MAX_MOSAICS; i++) {
        if(parser[i]) {
            delete parser[i];
            LOGW(("Destroying parser %p", (void*)parser[i]));
        }

        if (fp_mp4[i]) fclose(fp_mp4[i]);

        if (decryptor[i] != NULL) {
            LOGW(("Destroying decryptor %p", (void*)decryptor[i]));
            DecryptorFactory::DestroyDecryptor(decryptor[i]);
        }

        if (videoStreamer[i] != NULL) {
            LOGW(("Destroying videoStreamer %p", (void*)videoStreamer[i]));
            StreamerFactory::DestroyStreamer(videoStreamer[i]);
        }

        if (audioStreamer[i] != NULL) {
            LOGW(("Destroying audioStreamer %p", (void*)audioStreamer[i]));
            StreamerFactory::DestroyStreamer(audioStreamer[i]);
        }

        if (videoDecoder[i] != NULL) {
            NEXUS_SimpleVideoDecoder_Release(videoDecoder[i]);
        }

        if (audioDecoder[i] != NULL) {
            NEXUS_SimpleAudioDecoder_Release(audioDecoder[i]);
        }

        if (stcChannel[i] != NULL) {
            NEXUS_SimpleStcChannel_Destroy(stcChannel[i]);
        }

        license[i].clear();
        if (pAvccHdr[i]) NEXUS_Memory_Free(pAvccHdr[i]);
        if (pAudioHeaderBuf[i]) NEXUS_Memory_Free(pAudioHeaderBuf[i]);
        if (pVideoHeaderBuf[i]) NEXUS_Memory_Free(pVideoHeaderBuf[i]);
#ifdef DIF_SCATTER_GATHER
        for (int j = 0; j < NUM_SAMPLES_HELD; j++) {
            if (audioPesBuf[i][j] != NULL) {
                BufferFactory::DestroyBuffer(audioPesBuf[i][j]);
            }
            if (videoPesBuf[i][j] != NULL) {
                BufferFactory::DestroyBuffer(videoPesBuf[i][j]);
            }
            if (audioDecOut[i][j] != NULL) {
                BufferFactory::DestroyBuffer(audioDecOut[i][j]);
            }
            if (videoDecOut[i][j] != NULL) {
                BufferFactory::DestroyBuffer(videoDecOut[i][j]);
            }
        }
        if (pPayload[i][0]) NEXUS_Memory_Free(pPayload[i][0]);
#else
        if (pPayload[i]) NEXUS_Memory_Free(pPayload[i]);
#endif
    }
    if (connectId != 0xFFFF) {
        NxClient_Disconnect(connectId);
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
    batom_cursor cursor;

    batom_factory_t factory = batom_factory_create(bkni_alloc, 64);
    batom_accum_t dst = batom_accum_create(factory);

    bmedia_read_h264_meta(&meta, cfg_data, cfg_data_size);

    bmedia_copy_h264_meta_with_nal_vec(dst, &meta, &bmp4_nal_vec);
    batom_cursor_from_accum(&cursor, dst);
    *hdr_len = batom_cursor_copy(&cursor, avcc_hdr, BMP4_MAX_PPS_SPS);
    *nalu_len = meta.nalu_len;

    batom_accum_clear(dst);
    batom_accum_destroy(dst);
    batom_factory_destroy(factory);

    return 0;
}

static int process_fragment(mp4_parse_frag_info *frag_info,
    void *decoder_data, size_t decoder_len, int index)
{
    IStreamer* streamer;
    SampleInfo *pSample;
    bmp4_protectionSchemeInfo trackProtectionInfo;
    size_t pes_header_len;
    bmedia_pes_info pes_info;
    uint64_t frag_duration;
    uint32_t bytes_processed = 0;
    uint32_t last_bytes_processed = 0;
    if (frag_info->samples_info->sample_count == 0) {
        LOGE(("%s: No samples", BSTD_FUNCTION));
        return -1;
    }

    s_app.parser[index]->GetProtectionInfoForTrack(trackProtectionInfo, frag_info->trackId);

    LOGD(("%s: #samples=%d",BSTD_FUNCTION, frag_info->samples_info->sample_count));
    for (unsigned i = 0; i < frag_info->samples_info->sample_count; i++) {
        uint32_t numOfByteDecrypted = 0;
        size_t sampleSize = 0;

        pSample = &frag_info->samples_info->samples[i];
        numOfByteDecrypted = sampleSize = frag_info->sample_info[i].size;

        switch(frag_info->trackType)
        {
            case BMP4_SAMPLE_ENCRYPTED_VIDEO:
            case BMP4_SAMPLE_AVC:
            {
                streamer = s_app.videoStreamer[index];
                /* H.264 Decoder configuration parsing */
                size_t avcc_hdr_size = 0;

                bmedia_pes_info_init(&pes_info, REPACK_VIDEO_PES_ID);
                frag_duration = s_app.last_video_fragment_time +
                    (int32_t)frag_info->sample_info[i].composition_time_offset;
                s_app.last_video_fragment_time += frag_info->sample_info[i].duration;

                pes_info.pts_valid = true;
                pes_info.pts = (uint32_t)CALCULATE_PTS(frag_duration);

                if (s_app.video_decode_hdr[index] == 0) {
                    parse_avcc_config(s_app.pAvccHdr[index], &avcc_hdr_size, &s_app.nalu_len[index], (uint8_t*)decoder_data, decoder_len);
                    pes_header_len = bmedia_pes_header_init(s_app.pVideoHeaderBuf[index],
                        (sampleSize + avcc_hdr_size - s_app.nalu_len[index] + sizeof(bmp4_nal)), &pes_info);
                } else {
                    pes_header_len = bmedia_pes_header_init(s_app.pVideoHeaderBuf[index],
                        sampleSize - s_app.nalu_len[index] + sizeof(bmp4_nal), &pes_info);
                }

                if (pes_header_len > 0) {
#ifdef DIF_SCATTER_GATHER
                    if (s_app.videoPesBuf[index][s_app.video_idx[index]] == NULL) {
                        s_app.videoPesBuf[index][s_app.video_idx[index]] = BufferFactory::CreateBuffer(BMEDIA_PES_HEADER_MAX_SIZE + BMP4_MAX_PPS_SPS);
                    }
                    s_app.videoPesBuf[index][s_app.video_idx[index]]->Copy(0, s_app.pVideoHeaderBuf[index], pes_header_len);
                    streamer->SubmitScatterGather(
                        s_app.videoPesBuf[index][s_app.video_idx[index]]->GetPtr(),
                        pes_header_len);
#else
                    IBuffer* output =
                        streamer->GetBuffer(pes_header_len);
                    output->Copy(0, s_app.pVideoHeaderBuf[index], pes_header_len);
                    BufferFactory::DestroyBuffer(output);
#endif
                    bytes_processed += pes_header_len;
                }

                if (s_app.video_decode_hdr[index] == 0) {
#ifdef DIF_SCATTER_GATHER
                    streamer->SubmitScatterGather(s_app.pAvccHdr[index],
                        avcc_hdr_size);
#else
                    IBuffer* output =
                        streamer->GetBuffer(avcc_hdr_size);
                    output->Copy(0, s_app.pAvccHdr[index], avcc_hdr_size);
                    BufferFactory::DestroyBuffer(output);
#endif
                    bytes_processed += avcc_hdr_size;
                    s_app.video_decode_hdr[index] = 1;
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

#ifdef DIF_SCATTER_GATHER
                // Create secure buffer for decrypt output
                // Needs flow control
                if (s_app.videoDecOut[index][s_app.video_idx[index]] != NULL) {
                    BufferFactory::DestroyBuffer(s_app.videoDecOut[index][s_app.video_idx[index]]);
                }
                IBuffer* decOutput = BufferFactory::CreateBuffer(sampleSize, NULL, streamer->IsSecure());
                s_app.videoDecOut[index][s_app.video_idx[index]] = decOutput;
                s_app.video_idx[index]++;
                s_app.video_idx[index] %= NUM_SAMPLES_HELD;
                if (s_app.decryptor[index]) {
                    numOfByteDecrypted = s_app.decryptor[index]->DecryptSample(pSample,
                        input, decOutput, sampleSize, trackProtectionInfo);
                    streamer->SubmitSample(pSample, input, decOutput);
                } else {
                    streamer->SubmitScatterGather(input, true);
                    numOfByteDecrypted = sampleSize;
                }
                if (numOfByteDecrypted != sampleSize)
                {
                    LOGE(("%s Failed to decrypt sample, can't continue - %d", BSTD_FUNCTION, __LINE__));
                    BufferFactory::DestroyBuffer(input);
                    return -1;
                }
                BufferFactory::DestroyBuffer(input);
#else // DIF_SCATTER_GATHER
                IBuffer* decOutput = streamer->GetBuffer(sampleSize);
                // Transfer clear data to destination
                decOutput->Copy(0, input, sampleSize);
                if (s_app.decryptor[index]) {
                    numOfByteDecrypted = s_app.decryptor[index]->DecryptSample(pSample,
                        input, decOutput, sampleSize, trackProtectionInfo);
                } else {
                    numOfByteDecrypted = sampleSize;
                }
                if (numOfByteDecrypted != sampleSize)
                {
                    LOGE(("%s Failed to decrypt sample, can't continue - %d", BSTD_FUNCTION, __LINE__));
                    BufferFactory::DestroyBuffer(input);
                    BufferFactory::DestroyBuffer(decOutput);
                    return -1;
                }
                BufferFactory::DestroyBuffer(input);
                BufferFactory::DestroyBuffer(decOutput);
#endif // DIF_SCATTER_GATHER
                bytes_processed += numOfByteDecrypted;

                break;
            }

            case BMP4_SAMPLE_ENCRYPTED_AUDIO:
            case BMP4_SAMPLE_MP4A:
            {
                if (s_app.bypassAudio) return 0;
                streamer = s_app.audioStreamer[index];
                bmedia_pes_info_init(&pes_info, REPACK_AUDIO_PES_ID);
                frag_duration = s_app.last_audio_fragment_time +
                    (int32_t)frag_info->sample_info[i].composition_time_offset;
                s_app.last_audio_fragment_time += frag_info->sample_info[i].duration;

                pes_info.pts_valid = true;
                pes_info.pts = (uint32_t)CALCULATE_PTS(frag_duration);

                pes_header_len = bmedia_pes_header_init(s_app.pAudioHeaderBuf[index],
                    (sampleSize + BMEDIA_ADTS_HEADER_SIZE), &pes_info);

                /* AAC information parsing */
                bmedia_info_aac *info_aac = (bmedia_info_aac *)decoder_data;
                parse_esds_config(s_app.pAudioHeaderBuf[index] + pes_header_len, info_aac, sampleSize);

                if (streamer) {
#ifdef DIF_SCATTER_GATHER
                    if (s_app.audioPesBuf[index][s_app.audio_idx[index]] == NULL) {
                        s_app.audioPesBuf[index][s_app.audio_idx[index]] = BufferFactory::CreateBuffer(BMEDIA_PES_HEADER_MAX_SIZE + BMP4_MAX_PPS_SPS + BMEDIA_ADTS_HEADER_SIZE);
                    }
                    s_app.audioPesBuf[index][s_app.audio_idx[index]]->Copy(0, s_app.pAudioHeaderBuf[index], pes_header_len + BMEDIA_ADTS_HEADER_SIZE);
                    streamer->SubmitScatterGather(
                        s_app.audioPesBuf[index][s_app.audio_idx[index]]->GetPtr(),
                        pes_header_len + BMEDIA_ADTS_HEADER_SIZE);
#else // DIF_SCATTER_GATHER
                    IBuffer* output =
                        streamer->GetBuffer(pes_header_len + BMEDIA_ADTS_HEADER_SIZE);
                    output->Copy(0, s_app.pAudioHeaderBuf[index], pes_header_len + BMEDIA_ADTS_HEADER_SIZE);
                    BufferFactory::DestroyBuffer(output);
#endif // DIF_SCATTER_GATHER
                    bytes_processed += pes_header_len + BMEDIA_ADTS_HEADER_SIZE;
                }

                IBuffer* input = BufferFactory::CreateBuffer(sampleSize, (uint8_t*)frag_info->cursor.cursor);
                batom_cursor_skip(&frag_info->cursor, sampleSize);

                if (streamer) {
#ifdef DIF_SCATTER_GATHER
                    // Create secure buffer for decrypt output
                    // Needs flow control
                    if (s_app.audioDecOut[index][s_app.audio_idx[index]] != NULL) {
                        BufferFactory::DestroyBuffer(s_app.audioDecOut[index][s_app.audio_idx[index]]);
                    }
                    IBuffer* decOutput = BufferFactory::CreateBuffer(sampleSize, NULL, streamer->IsSecure());
                    s_app.audioDecOut[index][s_app.audio_idx[index]] = decOutput;
                    s_app.audio_idx[index]++;
                    s_app.audio_idx[index] %= NUM_SAMPLES_HELD;
                    if (s_app.decryptor[index]) {
                        numOfByteDecrypted = s_app.decryptor[index]->DecryptSample(pSample,
                            input, decOutput, sampleSize, trackProtectionInfo);
                        streamer->SubmitSample(pSample, input, decOutput);
                    } else {
                        streamer->SubmitScatterGather(input, true);
                        numOfByteDecrypted = sampleSize;
                    }
                    if (numOfByteDecrypted != sampleSize)
                    {
                        LOGE(("%s Failed to decrypt sample, can't continue - %d", BSTD_FUNCTION, __LINE__));
                        BufferFactory::DestroyBuffer(input);
                        return -1;
                    }
                    BufferFactory::DestroyBuffer(input);
#else // DIF_SCATTER_GATHER
                    IBuffer* decOutput = streamer->GetBuffer(sampleSize);
                    // Transfer clear data to destination
                    decOutput->Copy(0, input, sampleSize);
                    if (s_app.decryptor[index]) {
                        numOfByteDecrypted = s_app.decryptor[index]->DecryptSample(pSample,
                            input, decOutput, sampleSize, trackProtectionInfo);
                    } else {
                        numOfByteDecrypted = sampleSize;
                    }
                    if (numOfByteDecrypted != sampleSize)
                    {
                        LOGE(("%s Failed to decrypt sample, can't continue - %d", BSTD_FUNCTION, __LINE__));
                        BufferFactory::DestroyBuffer(input);
                        BufferFactory::DestroyBuffer(decOutput);
                        return -1;
                    }
                    BufferFactory::DestroyBuffer(input);
                    BufferFactory::DestroyBuffer(decOutput);
#endif // DIF_SCATTER_GATHER
                    bytes_processed += numOfByteDecrypted;
                }
                break;
            }

            default:
            LOGW(("%s Unsupported track type %d detected", BSTD_FUNCTION, frag_info->trackType));
            return -1;
        } /* End switch */

        if (streamer)
            streamer->Push(bytes_processed - last_bytes_processed);
        last_bytes_processed = bytes_processed;
    } /* End for loop */

    return 0;
}

static void play_callback(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
}

static void wait_for_drain()
{
    LOGW(("%s: start", BSTD_FUNCTION));
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
    LOGW(("%s: video playpump was drained", BSTD_FUNCTION));

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
    LOGW(("%s: audio playpump was drained", BSTD_FUNCTION));

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
    LOGW(("%s: video decoder was drained", BSTD_FUNCTION));

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
    LOGW(("%s: audio decoder was drained", BSTD_FUNCTION));

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
    struct bgui_settings gui_settings;

    bgui_get_default_settings(&gui_settings);
    gui_settings.width = virtualDisplay.width;
    gui_settings.height = virtualDisplay.height;
    s_app.gui = bgui_create(&gui_settings);

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
    allocSettings.simpleAudioDecoder = 1;
    rc = NxClient_Alloc(&allocSettings, &s_app.s_allocResults);
    if (rc)
        exit(EXIT_FAILURE);

    for (int i = 0; i < s_app.num_mosaics; i++)
    {
        if (s_app.s_allocResults.simpleVideoDecoder[i].id) {
            LOGD(("@@@ to acquire video decoder %d for %d", s_app.s_allocResults.simpleVideoDecoder[i].id, i));
            s_app.videoDecoder[i] = NEXUS_SimpleVideoDecoder_Acquire(s_app.s_allocResults.simpleVideoDecoder[i].id);
        }
        if (s_app.videoDecoder[i] == NULL) {
            LOGE(("video decoder not available %d", i));
            exit(EXIT_FAILURE);
        }

        {
            /* TEMP disable CC for mosaics */
            NEXUS_SimpleVideoDecoderClientSettings settings;
            NEXUS_SimpleVideoDecoder_GetClientSettings(s_app.videoDecoder[i], &settings);
            settings.closedCaptionRouting = false;
            NEXUS_SimpleVideoDecoder_SetClientSettings(s_app.videoDecoder[i], &settings);
        }

        if (!s_app.bypassAudio && i == 0) {
            if (s_app.s_allocResults.simpleAudioDecoder.id) {
                LOGD(("@ to acquire audio decoder %d", s_app.s_allocResults.simpleAudioDecoder.id));
                s_app.audioDecoder[i] = NEXUS_SimpleAudioDecoder_Acquire(s_app.s_allocResults.simpleAudioDecoder.id);
            }
            if (s_app.audioDecoder[i] == NULL) {
                LOGE(("audio decoder not available %d", i));
                exit(EXIT_FAILURE);
            }
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
    connectSettings.simpleAudioDecoder.id = s_app.s_allocResults.simpleAudioDecoder.id;
    rc = NxClient_Connect(&connectSettings, &s_app.connectId);
    if (rc)
        exit(EXIT_FAILURE);

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
    NEXUS_SimpleVideoDecoderStartSettings videoProgram;
    NEXUS_SimpleAudioDecoderStartSettings audioProgram;
    NEXUS_SimpleStcChannelSettings stcSettings;
    NEXUS_PlaypumpOpenSettings playpumpOpenSettings;
    NEXUS_PlaypumpSettings playpumpSettings;
    NEXUS_Error rc;

    NEXUS_Platform_GetClientConfiguration(&clientConfig);

    /* Show heaps info */
    int g;
    BDBG_LOG(("NxClient Heaps Info -----------------"));
    for (g = NXCLIENT_DEFAULT_HEAP; g <= NXCLIENT_ARR_HEAP; g++)
    {
        NEXUS_MemoryStatus status;
        NEXUS_Heap_GetStatus(clientConfig.heap[g], &status);

        BDBG_LOG(("Heap[%d]: memoryType=%u, heapType=%u, offset=%u, addr=%p, size=%u",
            g, status.memoryType, status.heapType, (uint32_t)status.offset, status.addr, status.size));
    }
    BDBG_LOG(("-------------------------------------"));

    // Create Streamers
    for (int i = 0; i < s_app.num_mosaics; i++) {
        s_app.videoStreamer[i] = StreamerFactory::CreateStreamer(!secure_video);
        if (s_app.videoStreamer[i] == NULL) {
            exit(EXIT_FAILURE);
        }
        if (!s_app.bypassAudio && i == 0) {
            // FIXME: set up audio only for the first one
            s_app.audioStreamer[i] = StreamerFactory::CreateStreamer(!secure_video);
            if (s_app.audioStreamer[i] == NULL) {
                exit(EXIT_FAILURE);
            }
        }

        // Increase playpump buffer size for video
        s_app.videoStreamer[i]->GetDefaultPlaypumpOpenSettings(&playpumpOpenSettings);
        playpumpOpenSettings.fifoSize = VIDEO_PLAYPUMP_BUF_SIZE;
        s_app.videoPlaypump[i] = s_app.videoStreamer[i]->OpenPlaypump(&playpumpOpenSettings);

        if (s_app.videoPlaypump[i] == NULL ) {
            LOGE(("Failed to open video playpump[%d]", i));
            exit(EXIT_FAILURE);
        }

        if (s_app.audioStreamer[i]) {
            // Use default playpump open settings
            s_app.audioPlaypump[i] = s_app.audioStreamer[i]->OpenPlaypump(NULL);

            if (s_app.audioPlaypump[i] == NULL ) {
                LOGE(("Failed to open audio playpump[%d]", i));
                exit(EXIT_FAILURE);
            }
        }
    }

    LOGD(("%s - %d\n", BSTD_FUNCTION, __LINE__));

    NEXUS_Memory_GetDefaultAllocationSettings(&memSettings);
    memSettings.heap = clientConfig.heap[1]; /* heap 1 is the eFull heap for the nxclient. */

    for (int i = 0; i < s_app.num_mosaics; i++) {
        s_app.pAvccHdr[i] = NULL;
        if (NEXUS_Memory_Allocate(BMP4_MAX_PPS_SPS, &memSettings, (void **)&s_app.pAvccHdr[i]) != NEXUS_SUCCESS) {
            fprintf(stderr,"NEXUS_Memory_Allocate failed");
            exit(EXIT_FAILURE);
        }

#ifdef DIF_SCATTER_GATHER
        uint8_t * pPayload = NULL;
        if (NEXUS_Memory_Allocate(BUF_SIZE * NUM_FRAGMENTS_HELD, &memSettings, (void **)&pPayload) != NEXUS_SUCCESS) {
            fprintf(stderr,"NEXUS_Memory_Allocate failed");
            exit(EXIT_FAILURE);
        }
        for (int j = 0; j < NUM_FRAGMENTS_HELD; j++) {
            s_app.pPayload[i][j] = pPayload + BUF_SIZE * j;
        }
#else
        s_app.pPayload[i] = NULL;
        if (NEXUS_Memory_Allocate(BUF_SIZE, &memSettings, (void **)&s_app.pPayload[i]) != NEXUS_SUCCESS) {
            fprintf(stderr,"NEXUS_Memory_Allocate failed");
            exit(EXIT_FAILURE);
        }
#endif

        s_app.pAudioHeaderBuf[i] = NULL;
        if (NEXUS_Memory_Allocate(BMEDIA_PES_HEADER_MAX_SIZE + BMP4_MAX_PPS_SPS,
            &memSettings, (void **)&s_app.pAudioHeaderBuf[i]) != NEXUS_SUCCESS) {
            fprintf(stderr,"NEXUS_Memory_Allocate failed");
            exit(EXIT_FAILURE);
        }

        s_app.pVideoHeaderBuf[i] = NULL;
        if (NEXUS_Memory_Allocate(BMEDIA_PES_HEADER_MAX_SIZE + BMP4_MAX_PPS_SPS,
            &memSettings, (void **)&s_app.pVideoHeaderBuf[i]) != NEXUS_SUCCESS) {
            fprintf(stderr,"NEXUS_Memory_Allocate failed");
            exit(EXIT_FAILURE);
        }
    }

    BKNI_CreateEvent(&s_app.event);

    // Set up Playpumps and Decoders
    for (int i = 0; i < s_app.num_mosaics; i++) {
        s_app.stcChannel[i] = NEXUS_SimpleStcChannel_Create(NULL);
        NEXUS_SimpleStcChannel_GetSettings(s_app.stcChannel[i], &stcSettings);
        stcSettings.mode = NEXUS_StcChannelMode_eAuto;
        rc = NEXUS_SimpleStcChannel_SetSettings(s_app.stcChannel[i], &stcSettings);
        if (rc) {
           LOGW(("@@@ Stc Set FAILED ---------------"));
        }

        s_app.videoStreamer[i]->GetSettings(&playpumpSettings);
        playpumpSettings.dataCallback.callback = play_callback;
        playpumpSettings.dataCallback.context = s_app.event;
        playpumpSettings.transportType = NEXUS_TransportType_eMpeg2Pes;
        s_app.videoStreamer[i]->SetSettings(&playpumpSettings);

        if (s_app.audioStreamer[i]) {
            s_app.audioStreamer[i]->GetSettings(&playpumpSettings);
            playpumpSettings.dataCallback.callback = play_callback;
            playpumpSettings.dataCallback.context = s_app.event;
            playpumpSettings.transportType = NEXUS_TransportType_eMpeg2Pes;
            s_app.audioStreamer[i]->SetSettings(&playpumpSettings);
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
            NEXUS_SimpleVideoDecoder_SetStcChannel(s_app.videoDecoder[i], s_app.stcChannel[i]);
        }

        if (s_app.audioStreamer[i]) {
            if (audioProgram.primary.pidChannel) {
                LOGW(("@@@ set stc channel audio"));
                NEXUS_SimpleAudioDecoder_SetStcChannel(s_app.audioDecoder[i], s_app.stcChannel[i]);
            }
        }
    }

}

static void setup_files()
{
    for (int index = 0; index < s_app.num_mosaics; index++) {
        LOGW(("MP4 file: %s\n", s_app.file[index]));
        fflush(stdout);
        s_app.fp_mp4[index] = fopen(s_app.file[index], "rb");
        if (s_app.fp_mp4[index] == NULL) {
            LOGE(("failed to open %s", s_app.file[index]));
            exit(EXIT_FAILURE);
        }
    }
}

static void setup_parsers()
{
    // Set up Parsers
    for (int index = 0; index < s_app.num_mosaics; index++) {
        fflush(stdout);
        s_app.parser[index] = new PiffParser(s_app.fp_mp4[index]);
        if (s_app.parser[index] ==  NULL) {
            LOGE(("failed to new a PiffParser instance"));
            exit(EXIT_FAILURE);
        }

        if (s_app.parser[index]->Initialize()) {
            LOGW(("PiffParser was initialized for %s", s_app.file[index]));
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

            if (s_app.parserDrmType[index] != drm_type_eUnknown) {
                bool drmMatch = false;
                DrmType drmTypes[BMP4_MAX_DRM_SCHEMES];
                uint8_t numOfDrmSchemes = s_app.parser[index]->GetNumOfDrmSchemes(drmTypes, BMP4_MAX_DRM_SCHEMES);
                for (int j = 0; j < numOfDrmSchemes; j++) {
                    if (drmTypes[j] == s_app.parserDrmType[index]) {
                        drmMatch = true;
                        s_app.parser[index]->SetDrmSchemes(j);
                        break;
                    } else if (s_app.parserDrmType[index] == drm_type_eWidevine && drmTypes[j] == drm_type_eWidevine3x) {
                        drmMatch = true;
                        s_app.parserDrmType[index] = drmTypes[j];
                        s_app.decryptorDrmType[index] = drmTypes[j];
                        break;
                    } else if (s_app.parserDrmType[index] == drm_type_eWidevine3x && drmTypes[j] == drm_type_eWidevine) {
                        drmMatch = true;
                        s_app.parserDrmType[index] = drmTypes[j];
                        s_app.decryptorDrmType[index] = drmTypes[j];
                        break;
                    }
                }
                if (!drmMatch) {
                    LOGE(("DRM Type: %d was not found in the stream.", s_app.parserDrmType[index]));
                    LOGE(("Do you want to play it with its default DRM type: %d? [y/n]", drmTypes[0]));
                    char resp;
                    scanf("%c", &resp);
                    if (resp != 'y')
                        exit(EXIT_FAILURE);
                    s_app.parserDrmType[index] = s_app.decryptorDrmType[index] = drmTypes[0];
                }
            }
        }
    }
}

static void setup_decryptors()
{
    std::string cpsSpecificData;    /*content protection system specific data*/
    std::string psshDataStr;
    DrmType decryptorDrmType, parserDrmType;

    std::string license_server;
    std::string key_response;

    // Set up Decryptors
    for (int i = 0; i < s_app.num_mosaics; i++) {
        decryptorDrmType = s_app.decryptorDrmType[i];
        parserDrmType = s_app.parser[i]->GetDrmType();
        if (decryptorDrmType == drm_type_eUnknown) {
            decryptorDrmType = parserDrmType;
            if (decryptorDrmType == drm_type_eWidevine ||
                decryptorDrmType == drm_type_eWidevine3x) {
                BDBG_WRN(("%s: defaulting to WV with GooglePlay License server", BSTD_FUNCTION));
                s_app.useGooglePlayLicServer[i] = true;
            }
        }
        if (decryptorDrmType != parserDrmType &&
            parserDrmType == drm_type_eUnknown)
            decryptorDrmType = drm_type_eUnknown;
        if (decryptorDrmType == drm_type_ePlayready30) {
            LOGW(("Playready 3.x for %s", s_app.file[i]));
        } else if (decryptorDrmType == drm_type_eWidevine3x) {
            LOGW(("Widevine 3.x for %s", s_app.file[i]));
        } else if (decryptorDrmType == drm_type_eWidevine) {
            LOGW(("Widevine for %s", s_app.file[i]));
        } else if (decryptorDrmType == drm_type_ePlayready) {
            LOGW(("Playready 2.5 for %s", s_app.file[i]));
        } else {
            LOGW(("Unknown DRM type for %s", s_app.file[i]));
            continue;
        }

        // If decryptor type is not WV3x, set session type to temporary
        if (decryptorDrmType != drm_type_eWidevine3x)
            s_app.sessionType[i] = session_type_eTemporary;

        // New API - creating Decryptor
        s_app.decryptor[i] = DecryptorFactory::CreateDecryptor(decryptorDrmType);
        if (s_app.decryptor[i] == NULL) {
            LOGE(("Failed to create Decryptor"));
            exit(EXIT_FAILURE);
        }

        // New API - initializing Decryptor
        psshDataStr = s_app.parser[i]->GetPssh();
        dump_hex("pssh", psshDataStr.data(), psshDataStr.size(), true);
        if (!s_app.decryptor[i]->Initialize(psshDataStr)) {
            LOGE(("Failed to initialize Decryptor"));
            exit(EXIT_FAILURE);
        }

        // New API - Load
        if (s_app.sessionType[i] == session_type_ePersistent &&
            !s_app.license[i].empty()) {
            if (s_app.decryptor[i]->Load(s_app.license[i])) {
                LOGW(("Successfully loaded persistent license: %s", s_app.license[i].c_str()));
                continue;
            } else {
                LOGW(("Failed to load persistent license: %s", s_app.license[i].c_str()));
                LOGW(("Continuing to acquire a license..."));
            }
        }

        // New API - GenerateKeyRequest
        cpsSpecificData = s_app.parser[i]->GetCpsSpecificData();
        dump_hex("cpsSpecificData", cpsSpecificData.data(), cpsSpecificData.size(), true);
        if (!s_app.decryptor[i]->GenerateKeyRequest(cpsSpecificData, s_app.sessionType[i])) {
            LOGE(("Failed to generate key request"));
            exit(EXIT_FAILURE);
        }

        dump_hex("message", s_app.decryptor[i]->GetKeyMessage().data(), s_app.decryptor[i]->GetKeyMessage().size());

        if (s_app.useGooglePlayLicServer[i]) {
            // Use Google Play server
            if (decryptorDrmType == drm_type_eWidevine ||
                decryptorDrmType == drm_type_eWidevine3x) {
                license_server.assign(kGpLicenseServer + kGpWidevineAuth);
            } else if (decryptorDrmType == drm_type_ePlayready ||
                decryptorDrmType == drm_type_ePlayready30) {
                license_server.assign(kGpLicenseServer + kGpPlayreadyAuth);
            }
            license_server.append(kGpClientOfflineQueryParameters);
        } else {
            license_server.clear();
        }

        LOGW(("License Server for %s: %s", s_app.file[i], license_server.c_str()));

        // New API - GetKeyRequestResponse
        key_response = s_app.decryptor[i]->GetKeyRequestResponse(license_server);

        // New API - AddKey
        bmp4_protection_info    protectionInfo;
        if ( !s_app.parser[i]->GetProtectionInfo(protectionInfo) ) {
            LOGE(("Failed to get protection information for tracks"));
            exit(EXIT_FAILURE);
        }
        if (!s_app.decryptor[i]->AddKey(key_response, protectionInfo)) {
            LOGE(("Failed to add key"));
            dump_hex("key_response", key_response.data(), key_response.size(), true);
            LOGE(("key_response string: %s", key_response.c_str()));
            exit(EXIT_FAILURE);
        }
        dump_hex("key_response", key_response.data(), key_response.size());
    }

}

void* playback(void* arg)
{
    mp4_parse_frag_info frag_info;
    void *decoder_data;
    uint32_t decoder_len;
    int done_count = 0;
    int i = (intptr_t)arg;
    LOGW(("playback thread starting stream %d", i));
#ifdef DIF_SCATTER_GATHER
    for (unsigned k = 0;; k++) {
#else
    for (;;) {
#endif
        if (!s_app.mosaic[i].done) {
#ifdef DIF_SCATTER_GATHER
            decoder_data = s_app.parser[i]->GetFragmentData(frag_info, s_app.pPayload[i][k%NUM_FRAGMENTS_HELD], decoder_len);
#else
            decoder_data = s_app.parser[i]->GetFragmentData(frag_info, s_app.pPayload[i], decoder_len);
#endif
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
            goto end;
        if (done_count >= 1) {
            wait_for_drain();
            goto end;
        }
    }
end:
    return NULL;
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
    LOGE(("        -pr:g <file> Set DRM type as playready and use GooglePlay license server"));
    LOGE(("        -pr3x <file> Set DRM type as playready 3.x"));
    LOGE(("        -pr3x:g <file> Set DRM type as playready 3.x and use GooglePlay license server"));
    LOGE(("        -wv <file> Set DRM type as widevine"));
    LOGE(("        -wv3x <file> Set DRM type as widevine 3x"));
    LOGE(("        -wv3x:p <file> Set DRM type as widevine 3x and session type as persistent"));
    LOGE(("        -wv3x:p:<license> <file> Load persistent license for widevine 3x session"));
    LOGE(("        -wv3x:pur <file> Set DRM type as widevine 3x and session type as persistent usage record"));
    LOGE(("        -loop N    Set # of playback loops"));
    LOGE(("        -secure    Use secure video picture buffers (URR)"));
    LOGE(("        -n N       Set # of mosaics, max=%d", MAX_MOSAICS));
    LOGE(("        -noAudio   Play only video"));
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
        if (strncmp(argv[i], "-pr3x", 5) == 0) {
            if (i >= argc - 1) {
                print_usage(argv[0]);
                exit(EXIT_FAILURE);
            }
            if (strcmp(argv[i], "-pr3x:g") == 0) {
                LOGW(("Use GooglePlay license server"));
                s_app.useGooglePlayLicServer[num_files] = true;
            } else {
                LOGW(("Use Microsoft license server"));
                s_app.useGooglePlayLicServer[num_files] = false;
            }
            s_app.parserDrmType[num_files] = drm_type_ePlayready;
            s_app.decryptorDrmType[num_files] = drm_type_ePlayready30;
            s_app.file[num_files] = argv[++i];
            LOGW(("File[%d] PR3.x: %s", num_files, s_app.file[num_files]));
            num_files++;
        }
        else if (strncmp(argv[i], "-pr", 3) == 0) {
            if (i >= argc - 1) {
                print_usage(argv[0]);
                exit(EXIT_FAILURE);
            }
            if (strcmp(argv[i], "-pr:g") == 0) {
                LOGW(("Use GooglePlay license server"));
                s_app.useGooglePlayLicServer[num_files] = true;
            } else {
                LOGW(("Use Microsoft license server"));
                s_app.useGooglePlayLicServer[num_files] = false;
            }
            s_app.parserDrmType[num_files] = drm_type_ePlayready;
            s_app.decryptorDrmType[num_files] = drm_type_ePlayready;
            s_app.file[num_files] = argv[++i];
            LOGW(("File[%d] PR2.5: %s", num_files, s_app.file[num_files]));
            num_files++;
        }
        else if (strcmp(argv[i], "-wv") == 0) {
            if (i >= argc - 1) {
                print_usage(argv[0]);
                exit(EXIT_FAILURE);
            }
            s_app.useGooglePlayLicServer[num_files] = true;
            s_app.parserDrmType[num_files] = drm_type_eWidevine;
            s_app.decryptorDrmType[num_files] = drm_type_eWidevine;
            s_app.file[num_files] = argv[++i];
            LOGW(("File[%d] WV: %s", num_files, s_app.file[num_files]));
            num_files++;
        }
        else if (strncmp(argv[i], "-wv3x", 5) == 0) {
            if (i >= argc - 1) {
                print_usage(argv[0]);
                exit(EXIT_FAILURE);
            }
            if (strncmp(argv[i], "-wv3x:pur", 9) == 0) {
                s_app.sessionType[num_files] = session_type_ePersistentUsageRecord;
            } else if (strncmp(argv[i], "-wv3x:p", 7) == 0) {
                s_app.sessionType[num_files] = session_type_ePersistent;
                if (strncmp(argv[i], "-wv3x:p:", 8) == 0 &&
                    strlen(argv[i]) > 8) {
                    s_app.license[num_files].assign(&argv[i][8]);
                    LOGW(("Persistent License: %s", s_app.license[num_files].c_str()));
                }
            }
            s_app.useGooglePlayLicServer[num_files] = true;
            s_app.parserDrmType[num_files] = drm_type_eWidevine;
            s_app.decryptorDrmType[num_files] = drm_type_eWidevine3x;
            s_app.file[num_files] = argv[++i];
            LOGW(("File[%d] WV3x: %s", num_files, s_app.file[num_files]));
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
        else if (strcmp(argv[i], "-noAudio") == 0) {
            s_app.bypassAudio = true;
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
                s_app.parserDrmType[num_files * i + j] = s_app.parserDrmType[j];
                s_app.decryptorDrmType[num_files * i + j] = s_app.decryptorDrmType[j];
                s_app.sessionType[num_files * i + j] = s_app.sessionType[j];
                s_app.useGooglePlayLicServer[num_files * i + j] = s_app.useGooglePlayLicServer[j];
                remaining--;
            }
        }
    } else {
        LOGW(("num_mosaics set to %d", num_files));
        s_app.num_mosaics = num_files;
    }

    for (int i = 0; i < s_app.num_mosaics; i++) {
        LOGW(("File[%d] parserDrm=%d decryptorDrm=%d: %s", i,
            s_app.parserDrmType[i], s_app.decryptorDrmType[i], s_app.file[i]));
    }

    LOGD(("@@@ Check Point #01"));

    setup_gui();

    setup_streamers();

    setup_files();

    setup_parsers();

    setup_decryptors();

    for (int i = 0; i < num_loops; i++) {
        LOGW(("Playback Loop: %d starting", i));
        LOGW(("Spawning %d threads", s_app.num_mosaics));
        for (int j = 0; j < s_app.num_mosaics; j++) {
            if(pthread_create(&s_app.playback_thread[j], NULL, playback, (void*)(intptr_t)j)) {
                LOGE(("Error creating thread %d errno=%d", j, errno));
                exit(EXIT_FAILURE);
            }
        }

        for (int j = 0; j < s_app.num_mosaics; j++) {
            if (pthread_join(s_app.playback_thread[j], NULL)) {
                LOGE(("Error joining thread %d errno=%d", j, errno));
                exit(EXIT_FAILURE);
            }
        }
        if (shouldExit)
            break;
        LOGW(("Playback Loop: %d done", i));
        if (i < num_loops - 1)
            rewind_parsers();
    }

    exit(EXIT_SUCCESS);
}
