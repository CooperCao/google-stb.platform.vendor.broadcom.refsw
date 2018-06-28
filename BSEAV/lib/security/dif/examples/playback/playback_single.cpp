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
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_video_input.h"
#include "nexus_mosaic_video_decoder.h"
#include "nexus_mosaic_display.h"
#include "nexus_spdif_output.h"
#include "nexus_component_output.h"
#include "nexus_video_adj.h"
#include "nexus_base_mmap.h"
#include "nexus_core_utils.h"
#include "namevalue.h"

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>

#if NEXUS_NUM_HDMI_OUTPUTS
#include <sys/types.h>
#include <sys/stat.h>
#endif

#include <pthread.h>

BDBG_MODULE(playback_dif_single);
#define LOG_TAG "playback_dif_single"
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

    NEXUS_DisplayHandle display;
    NEXUS_VideoWindowHandle window;
    NEXUS_VideoFormat format;

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
    NEXUS_VideoDecoderHandle videoDecoder[MAX_MOSAICS];
    NEXUS_AudioDecoderHandle audioDecoder[MAX_MOSAICS];
    NEXUS_StcChannelHandle stcChannel[MAX_MOSAICS];
    NEXUS_PlaypumpHandle videoPlaypump[MAX_MOSAICS];
    NEXUS_PlaypumpHandle audioPlaypump[MAX_MOSAICS];
    NEXUS_PidChannelHandle videoPidChannel[MAX_MOSAICS];
    NEXUS_PidChannelHandle audioPidChannel[MAX_MOSAICS];
    BKNI_EventHandle event;
#if NEXUS_NUM_HDMI_OUTPUTS
    bool hdcpAuthenticated;
    std::string hdcp2xKey;
#endif
    uint32_t exportHeapSize;

    // Session Type - only for Wv3x
    SessionType sessionType[MAX_MOSAICS];
    std::string license[MAX_MOSAICS];

    uint64_t last_video_fragment_time;
    uint64_t last_audio_fragment_time;
    bool bypassAudio;

    struct {
        NEXUS_Rect rect;
        unsigned zorder;
        bool done;
        NEXUS_VideoWindowHandle window;
    } mosaic[MAX_MOSAICS];
    unsigned num_mosaics;
};

AppContext::AppContext()
{
    display = NULL;
    window = NULL;
    format = NEXUS_VideoFormat_eUnknown;

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
        mosaic[i].zorder = i;
        mosaic[i].done = false;
        mosaic[i].window = NULL;
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
#if NEXUS_NUM_HDMI_OUTPUTS
    hdcpAuthenticated = false;
    hdcp2xKey.clear();
#endif
    exportHeapSize = 0;

    last_video_fragment_time = 0;
    last_audio_fragment_time = 0;
    bypassAudio = false;
}

AppContext::~AppContext()
{
    LOGW(("%s: start cleaning up", BSTD_FUNCTION));
    for (int i = 0; i < MAX_MOSAICS; i++) {
        if (videoDecoder[i]) {
            NEXUS_VideoDecoder_Stop(videoDecoder[i]);
        }
        if (audioDecoder[i]) {
            NEXUS_AudioDecoder_Stop(audioDecoder[i]);
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
            NEXUS_VideoDecoder_Close(videoDecoder[i]);
        }

        if (audioDecoder[i] != NULL) {
            NEXUS_AudioDecoder_Close(audioDecoder[i]);
        }

        if (stcChannel[i] != NULL) {
            NEXUS_StcChannel_Close(stcChannel[i]);
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

#if NEXUS_NUM_HDMI_OUTPUTS
    if (!hdcp2xKey.empty()) {
        hdcp2xKey.clear();
    }
#endif

    if (window != NULL) {
        LOGW(("Closing window %p", (void*)window));
        NEXUS_VideoWindow_Close(window);
    }

    if (display != NULL) {
        LOGW(("Closing display %p", (void*)display));
        NEXUS_Display_Close(display);
    }

    if (event != NULL) {
        BKNI_DestroyEvent((BKNI_EventHandle)event);
    }

    NEXUS_Platform_Uninit();
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
                pes_info.pts = (uint32_t)(frag_duration * 1000LL / ((uint64_t)(frag_info->trackTimeScale)) * 45LL);

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
                pes_info.pts = (uint32_t)(frag_duration * 1000LL / ((uint64_t)(frag_info->trackTimeScale)) * 45LL);

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
                    if (s_app.audioDecOut[index][s_app.audio_idx[index]] != NULL) {
                        BufferFactory::DestroyBuffer(s_app.audioDecOut[index][s_app.audio_idx[index]]);
                    }
                    if (s_app.decryptor[index]) {
                        IBuffer* decOutput = BufferFactory::CreateBuffer(sampleSize, NULL, streamer->IsSecure());
                        s_app.audioDecOut[index][s_app.audio_idx[index]] = decOutput;
                        numOfByteDecrypted = s_app.decryptor[index]->DecryptSample(pSample,
                            input, decOutput, sampleSize, trackProtectionInfo);
                        streamer->SubmitSample(pSample, input, decOutput);
                    } else {
                        streamer->SubmitScatterGather(input, true);
                        numOfByteDecrypted = sampleSize;
                    }
                    s_app.audio_idx[index]++;
                    s_app.audio_idx[index] %= NUM_SAMPLES_HELD;
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
                rc = NEXUS_VideoDecoder_GetStatus(s_app.videoDecoder[i], &decoderStatus);
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
                rc = NEXUS_AudioDecoder_GetStatus(s_app.audioDecoder[i], &decoderStatus);
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

#if NEXUS_NUM_HDMI_OUTPUTS
static void hdmiOutputHdcpStateChanged(void *pContext, int param)
{
    NEXUS_Error rc;
    bool success = false;
    NEXUS_HdmiOutputHandle handle = (NEXUS_HdmiOutputHandle)pContext;
    NEXUS_HdmiOutputHdcpStatus hdcpStatus;

    BSTD_UNUSED(param);

    NEXUS_HdmiOutput_GetHdcpStatus(handle, &hdcpStatus);

    LOGW(("%s: state=%d error=%d", BSTD_FUNCTION, hdcpStatus.hdcpState, hdcpStatus.hdcpError));

    if (hdcpStatus.hdcpState == NEXUS_HdmiOutputHdcpState_eUnpowered) {
        LOGW(("Attached Device is unpowered"));
    }
    else {
        LOGD(("HDCP State: %d", hdcpStatus.hdcpState));

       /* report any error */
        switch (hdcpStatus.hdcpError)
        {
        case NEXUS_HdmiOutputHdcpError_eSuccess:
            if (hdcpStatus.linkReadyForEncryption || hdcpStatus.transmittingEncrypted) {
                LOGW(("HDCP Authentication Successful\n"));
                success = true;
                s_app.hdcpAuthenticated = true;
            }
            break;

        case NEXUS_HdmiOutputHdcpError_eRxBksvError:
            LOGE(("HDCP Rx BKsv Error"));
            break;

        case NEXUS_HdmiOutputHdcpError_eRxBksvRevoked:
            LOGE(("HDCP Rx BKsv/Keyset Revoked"));
            break;

        case NEXUS_HdmiOutputHdcpError_eRxBksvI2cReadError:
        case NEXUS_HdmiOutputHdcpError_eTxAksvI2cWriteError:
            LOGE(("HDCP I2C Read Error"));
            break;

        case NEXUS_HdmiOutputHdcpError_eTxAksvError:
            LOGE(("HDCP Tx Aksv Error"));
            break;

        case NEXUS_HdmiOutputHdcpError_eReceiverAuthenticationError:
            LOGE(("HDCP Receiver Authentication Failure"));
            break;

        case NEXUS_HdmiOutputHdcpError_eRepeaterAuthenticationError:
        case NEXUS_HdmiOutputHdcpError_eRepeaterLinkFailure: /* Repeater Error; unused */
            LOGE(("HDCP Repeater Authentication Failure"));
            break;

        case NEXUS_HdmiOutputHdcpError_eRxDevicesExceeded:
            LOGE(("HDCP Repeater MAX Downstram Devices Exceeded"));
            break;

        case NEXUS_HdmiOutputHdcpError_eRepeaterDepthExceeded:
            LOGE(("HDCP Repeater MAX Downstram Levels Exceeded"));
            break;

        case NEXUS_HdmiOutputHdcpError_eRepeaterFifoNotReady:
            LOGE(("Timeout waiting for Repeater"));
            break;

        case NEXUS_HdmiOutputHdcpError_eRepeaterDeviceCount0: /* unused */
            break;

        case NEXUS_HdmiOutputHdcpError_eLinkRiFailure:
            LOGE(("HDCP Ri Integrity Check Failure"));
            break;

        case NEXUS_HdmiOutputHdcpError_eLinkPjFailure:
            LOGE(("HDCP Pj Integrity Check Failure"));
            break;

        case NEXUS_HdmiOutputHdcpError_eFifoUnderflow:
        case NEXUS_HdmiOutputHdcpError_eFifoOverflow:
            LOGE(("Video configuration issue"));
            break;

        case NEXUS_HdmiOutputHdcpError_eMultipleAnRequest: /* Should not reach here; but flag if occurs */
            LOGW(("Multiple Authentication Request..."));
            break;

        default:
            LOGW(("Unknown HDCP Authentication Error %d", hdcpStatus.hdcpError));
        }
    }

    if (!success) {
        rc = NEXUS_HdmiOutput_DisableHdcpAuthentication(handle);
        if (rc != NEXUS_SUCCESS) {
            LOGE(("%s: Error in disabling HDCP authentication", BSTD_FUNCTION));
        }
    }
}

static NEXUS_Error loadHdcp2xKeys(
    NEXUS_PlatformConfiguration &platformConfig)
{
    NEXUS_HdmiOutputHdcpSettings hdmiOutputHdcpSettings;

    FILE *fileFd = NULL;
    uint8_t *buffer = NULL;
    NEXUS_Error rc = NEXUS_SUCCESS;
    uint32_t fileSize;
    struct stat buf;

    LOGD(("%s: HDCP2x Key:%s", BSTD_FUNCTION, s_app.hdcp2xKey.c_str()));
    if (stat(s_app.hdcp2xKey.c_str(), &buf) != 0) {
        LOGW(("%s: stat failed(%d): %s", BSTD_FUNCTION, errno, s_app.hdcp2xKey.c_str()));
        return false;
    }
    fileSize = buf.st_size;

    fileFd = fopen(s_app.hdcp2xKey.c_str(), "rb");
    if (fileFd == NULL) {
        LOGE(("%s: fopen failed(%d): %s", BSTD_FUNCTION, errno, s_app.hdcp2xKey.c_str()));
        return false;
    }

    buffer = (uint8_t*)BKNI_Malloc(fileSize);
    if (fread(buffer, sizeof(char), fileSize, fileFd) != fileSize)
    {
        LOGE(("%s: fread failed on %s", BSTD_FUNCTION, s_app.hdcp2xKey.c_str()));
        rc = NEXUS_OS_ERROR;
        goto end;
    }

    LOGW(("%s: buff=%p, size=%u", BSTD_FUNCTION, buffer, fileSize));

    NEXUS_HdmiOutput_GetHdcpSettings(platformConfig.outputs.hdmi[0], &hdmiOutputHdcpSettings);
    LOGW(("%s: HDCP Version=%d", BSTD_FUNCTION, hdmiOutputHdcpSettings.hdcp_version));
#if 1
    hdmiOutputHdcpSettings.hdcp_version = NEXUS_HdmiOutputHdcpVersion_e2_2;
    hdmiOutputHdcpSettings.hdcp2xContentStreamControl = NEXUS_Hdcp2xContentStream_eType1;
#else
    hdmiOutputHdcpSettings.hdcp_version = NEXUS_HdmiOutputHdcpVersion_eAuto;
    hdmiOutputHdcpSettings.hdcp2xContentStreamControl = NEXUS_Hdcp2xContentStream_eType0;
#endif

    BKNI_Memset(hdmiOutputHdcpSettings.aksv.data, 0, sizeof(hdmiOutputHdcpSettings.aksv.data));
    BKNI_Memset(&hdmiOutputHdcpSettings.encryptedKeySet, 0, sizeof(hdmiOutputHdcpSettings.encryptedKeySet));

    /* install HDCP success callback */
    hdmiOutputHdcpSettings.successCallback.callback = hdmiOutputHdcpStateChanged;
    hdmiOutputHdcpSettings.successCallback.context = platformConfig.outputs.hdmi[0];

    /* install HDCP failure callback */
    hdmiOutputHdcpSettings.failureCallback.callback = hdmiOutputHdcpStateChanged;
    hdmiOutputHdcpSettings.failureCallback.context = platformConfig.outputs.hdmi[0];

    /* install HDCP stateChanged callback */
    hdmiOutputHdcpSettings.stateChangedCallback.callback = hdmiOutputHdcpStateChanged;
    hdmiOutputHdcpSettings.stateChangedCallback.context = platformConfig.outputs.hdmi[0];

    NEXUS_HdmiOutput_SetHdcpSettings(platformConfig.outputs.hdmi[0], &hdmiOutputHdcpSettings);

    rc = NEXUS_HdmiOutput_SetHdcp2xBinKeys(platformConfig.outputs.hdmi[0], buffer, (uint32_t)fileSize);
    if (rc != NEXUS_SUCCESS) {
        LOGE(("%s: Error setting Hdcp2x encrypted keys. HDCP2.x will not work.", BSTD_FUNCTION));
        goto end;
    }

end:
    if (fileFd != NULL) {
        fclose(fileFd);
    }

    if (buffer != NULL) {
        BKNI_Free(buffer);
    }

    return rc;
}
#endif

static uint32_t parse_size(const char *parse)
{
    if (strchr(parse, 'M') || strchr(parse, 'm')) {
        return atof(parse) * 1024 * 1024;
    }
    else if (strchr(parse, 'K') || strchr(parse, 'k')) {
        return atof(parse) * 1024;
    }
    else {
        return strtoul(parse, NULL, 0);
    }
}


static void setup_gui()
{
    NEXUS_Error rc;
    NEXUS_AudioDecoderOpenSettings audioDecoderOpenSettings;
    NEXUS_DisplayCapabilities displayCap;
    NEXUS_VideoDecoderCapabilities videoDecoderCap;
    NEXUS_VideoWindowSettings windowSettings;

    NEXUS_PlatformConfiguration platformConfig;
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputStatus hdmiStatus;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_VideoFormatInfo formatInfo;
#endif
    unsigned num_columns, num_rows;
    NEXUS_Rect virtualDisplay = {0, 0, 1920, 1080};

    NEXUS_Platform_GetConfiguration(&platformConfig);

    NEXUS_GetDisplayCapabilities(&displayCap);
    NEXUS_GetVideoDecoderCapabilities(&videoDecoderCap);
    if (videoDecoderCap.memory[0].mosaic.maxNumber == 0) {
        LOGE(("unable to perform mosaic"));
    }
    if (s_app.num_mosaics > videoDecoderCap.memory[0].mosaic.maxNumber) {
        s_app.num_mosaics = videoDecoderCap.memory[0].mosaic.maxNumber;
        LOGW(("num_mosaics reduced to %d (videoDecoderCap)", s_app.num_mosaics));
    }
    LOGW(("displayCap: numVideoWindows=%d",
        displayCap.display[0].numVideoWindows));
    if (s_app.num_mosaics > displayCap.display[0].numVideoWindows) {
        s_app.num_mosaics = displayCap.display[0].numVideoWindows;
        LOGW(("num_mosaics reduced to %d (displayCap)", s_app.num_mosaics));
    }
    if (s_app.num_mosaics == 0) s_app.num_mosaics = 1;

    /* Bring up video display and outputs */
    s_app.display = NEXUS_Display_Open(0, NULL);

    if (s_app.format != NEXUS_VideoFormat_eUnknown) {
        NEXUS_Display_GetSettings(s_app.display, &displaySettings);
        displaySettings.format = s_app.format;
        LOGW(("display format is set to %s", lookup_name(g_videoFormatStrs, displaySettings.format)));
        NEXUS_Display_SetSettings(s_app.display, &displaySettings);
    }

#if NEXUS_NUM_COMPONENT_OUTPUTS
    NEXUS_Display_AddOutput(s_app.display, NEXUS_ComponentOutput_GetConnector(platformConfig.outputs.component[0]));
#endif
#if NEXUS_NUM_COMPOSITE_OUTPUTS
    NEXUS_Display_AddOutput(s_app.display, NEXUS_CompositeOutput_GetConnector(platformConfig.outputs.composite[0]));
#endif
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_Display_AddOutput(s_app.display, NEXUS_HdmiOutput_GetVideoConnector(platformConfig.outputs.hdmi[0]));
    s_app.hdcpAuthenticated = false;
    rc = NEXUS_HdmiOutput_GetStatus(platformConfig.outputs.hdmi[0], &hdmiStatus);
    if (rc == NEXUS_SUCCESS && hdmiStatus.connected) {
        if (!s_app.hdcp2xKey.empty()) {
            rc = loadHdcp2xKeys(platformConfig);
            if (rc == NEXUS_SUCCESS) {
                rc = NEXUS_HdmiOutput_StartHdcpAuthentication(platformConfig.outputs.hdmi[0]);
                if (rc != NEXUS_SUCCESS) {
                    LOGW(("%s: Error in HDCP authentication", BSTD_FUNCTION));
                }
            }
        }
        /* If current display format is not supported by monitor, switch to monitor's preferred format.
           If other connected outputs do not support the preferred format, a harmless error will occur. */
        NEXUS_Display_GetSettings(s_app.display, &displaySettings);
        if ( !hdmiStatus.videoFormatSupported[displaySettings.format] ) {
            displaySettings.format = hdmiStatus.preferredVideoFormat;
            NEXUS_Display_SetSettings(s_app.display, &displaySettings);
        }
    }
#endif

    // Retrieve display format
    NEXUS_Display_GetSettings(s_app.display, &displaySettings);
    NEXUS_VideoFormat_GetInfo(displaySettings.format, &formatInfo);
    virtualDisplay.width = formatInfo.width;
    virtualDisplay.height = formatInfo.height;

    LOGW(("Display: %dx%d", virtualDisplay.width, virtualDisplay.height));

    s_app.window = NEXUS_VideoWindow_Open(s_app.display, 0);
    NEXUS_VideoWindow_GetSettings(s_app.window, &windowSettings);
    windowSettings.position.width = virtualDisplay.width;
    windowSettings.position.height = virtualDisplay.height;
    rc = NEXUS_VideoWindow_SetSettings(s_app.window, &windowSettings);
    if (rc != NEXUS_SUCCESS) {
        LOGE(("VideoWindow_SetSettings failed rc=%d", rc));
        exit(EXIT_FAILURE);
    }

    // Calculating rect sizes/positions
    num_columns = (s_app.num_mosaics + 1) / 2;
    num_rows = (s_app.num_mosaics == 1) ? 1 : 2;
    for (int i = 0; i < s_app.num_mosaics; i++) {
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
    }

    // Opening video decoders
    if (s_app.num_mosaics == 1) {
        NEXUS_VideoDecoderOpenSettings openSettings;
        NEXUS_VideoDecoder_GetDefaultOpenSettings(&openSettings);
        if (secure_video)
        {
            openSettings.cdbHeap = platformConfig.heap[NEXUS_VIDEO_SECURE_HEAP];
            openSettings.secureVideo = NEXUS_VideoDecoderSecureType_eSecure;
        }
        s_app.videoDecoder[0] = NEXUS_VideoDecoder_Open(0, &openSettings);
        if (s_app.videoDecoder[0] == NULL) {
            LOGE(("video decoder not available"));
            exit(EXIT_FAILURE);
        }

        /* Check if decoder needs larger max */
        uint32_t width = 0;
        uint32_t height = 0;
        if (!s_app.parser[0]->GetVideoResolution(&width, &height)) {
            LOGW(("failed to get video resolution"));
        }
        LOGW(("Video resolution: %dx%d", width, height));

        NEXUS_VideoDecoderSettings decSettings;
        NEXUS_VideoDecoder_GetSettings(s_app.videoDecoder[0], &decSettings);
        if (width > decSettings.maxWidth || height > decSettings.maxHeight) {
            decSettings.maxWidth = width;
            decSettings.maxHeight = height;
            NEXUS_VideoDecoder_SetSettings(s_app.videoDecoder[0], &decSettings);
        }
    } else
    for (int i = 0; i < s_app.num_mosaics; i++)
    {
        NEXUS_VideoDecoderOpenMosaicSettings openMosaicSettings;

        NEXUS_VideoDecoder_GetDefaultOpenMosaicSettings(&openMosaicSettings);
        openMosaicSettings.maxWidth = s_app.mosaic[i].rect.width;
        openMosaicSettings.maxHeight = s_app.mosaic[i].rect.height;
        if (secure_video)
        {
            openMosaicSettings.openSettings.cdbHeap = platformConfig.heap[NEXUS_VIDEO_SECURE_HEAP];
            openMosaicSettings.openSettings.secureVideo = NEXUS_VideoDecoderSecureType_eSecure;
        }
        s_app.videoDecoder[i] = NEXUS_VideoDecoder_OpenMosaic(0, i, &openMosaicSettings);
        if (s_app.videoDecoder[i] == NULL) {
            LOGE(("video decoder not available %d", i));
            exit(EXIT_FAILURE);
        }
    }

    // Opening audio decoder
    if (!s_app.bypassAudio)
    {
        /* Bring up audio decoder and outputs */
        NEXUS_AudioDecoder_GetDefaultOpenSettings(&audioDecoderOpenSettings);
        if (secure_video)
            audioDecoderOpenSettings.cdbHeap = platformConfig.heap[NEXUS_VIDEO_SECURE_HEAP];

        s_app.audioDecoder[0] = NEXUS_AudioDecoder_Open(0, &audioDecoderOpenSettings);
        if (s_app.audioDecoder[0] == NULL) {
            exit(EXIT_FAILURE);
        }
#if NEXUS_NUM_AUDIO_DACS
        if (platformConfig.outputs.audioDacs[0])
        {
            NEXUS_AudioOutput_AddInput(
                NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]),
                NEXUS_AudioDecoder_GetConnector(s_app.audioDecoder[0], NEXUS_AudioDecoderConnectorType_eStereo));
        }
#endif
#if NEXUS_NUM_SPDIF_OUTPUTS
        if (platformConfig.outputs.spdif[0])
        {
            NEXUS_AudioOutput_AddInput(
                NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]),
                NEXUS_AudioDecoder_GetConnector(s_app.audioDecoder[0], NEXUS_AudioDecoderConnectorType_eStereo));
        }
#endif
#if NEXUS_NUM_HDMI_OUTPUTS
        NEXUS_AudioOutput_AddInput(
            NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]),
            NEXUS_AudioDecoder_GetConnector(s_app.audioDecoder[0], NEXUS_AudioDecoderConnectorType_eStereo));
#endif
    }
    else
    {
        s_app.audioDecoder[0] = NULL;
    }

    // Opening video windows
    if (s_app.num_mosaics == 1) {
        NEXUS_VideoWindow_AddInput(s_app.window, NEXUS_VideoDecoder_GetConnector(s_app.videoDecoder[0]));
    } else
    for (int i = 0; i < s_app.num_mosaics; i++)
    {
        s_app.mosaic[i].window = NEXUS_VideoWindow_OpenMosaic(s_app.window, i);

        NEXUS_VideoWindow_GetSettings(s_app.mosaic[i].window, &windowSettings);
        windowSettings.position.x = s_app.mosaic[i].rect.x;
        windowSettings.position.y = s_app.mosaic[i].rect.y;
        windowSettings.position.width = s_app.mosaic[i].rect.width;
        windowSettings.position.height = s_app.mosaic[i].rect.height;
        windowSettings.zorder = s_app.mosaic[i].zorder;
        windowSettings.visible = true;
        rc = NEXUS_VideoWindow_SetSettings(s_app.mosaic[i].window, &windowSettings);
        if (rc != NEXUS_SUCCESS) {
            LOGE(("VideoWindow_SetSettings failed for %d rc=%d", i, rc));
            exit(EXIT_FAILURE);
        }

        rc = NEXUS_VideoWindow_AddInput(s_app.mosaic[i].window, NEXUS_VideoDecoder_GetConnector(s_app.videoDecoder[i]));
        if (rc != NEXUS_SUCCESS) {
            LOGE(("VideoWindow_AddInput failed for %d rc=%d", i, rc));
            exit(EXIT_FAILURE);
        }
    }
}

static void setup_streamers()
{
    NEXUS_MemoryAllocationSettings memSettings;
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_AudioDecoderStartSettings audioProgram;
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_PlaypumpOpenSettings playpumpOpenSettings;
    NEXUS_PlaypumpSettings playpumpSettings;

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
    memSettings.heap = NEXUS_MEMC0_MAIN_HEAP;

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
            LOGE(("NEXUS_Memory_Allocate failed"));
            exit(EXIT_FAILURE);
        }
    }

    BKNI_CreateEvent(&s_app.event);

    // Set up Playpumps and Decoders
    for (int i = 0; i < s_app.num_mosaics; i++) {
        NEXUS_StcChannel_GetDefaultSettings(i, &stcSettings);
        stcSettings.stcIndex = 0; /* must have shared STC for all mosaics on a single video decoder */
        stcSettings.timebase = NEXUS_Timebase_e0;
        stcSettings.mode = NEXUS_StcChannelMode_eAuto;
        s_app.stcChannel[i] = NEXUS_StcChannel_Open(i, &stcSettings);

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

            NEXUS_AudioDecoder_GetDefaultStartSettings(&audioProgram);
        }
        NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);

        LOGW(("@@@ set video audio program for h264"));
        videoProgram.codec = NEXUS_VideoCodec_eH264;
        audioProgram.codec = NEXUS_AudioCodec_eAacAdts;

        videoProgram.pidChannel = s_app.videoPidChannel[i];
        videoProgram.stcChannel = s_app.stcChannel[i];
        NEXUS_VideoDecoder_Start(s_app.videoDecoder[i], &videoProgram);

        if (s_app.audioDecoder[i])
        {
            audioProgram.pidChannel = s_app.audioPidChannel[i];
            audioProgram.stcChannel = s_app.stcChannel[i];
            NEXUS_AudioDecoder_Start(s_app.audioDecoder[i], &audioProgram);
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
           LOGE(("Failed to get scheme protection information"));
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
    LOGE(("        -hdcp2x_keys <file> Specify HDCP2x key file"));
    LOGE(("        -export <bytes> Specify export heap size in bytes. Use M or K suffix for units."));
    LOGE(("        -noAudio   Play only video"));
    LOGE(("        -display_format <format> Specify display format"));
}

int main(int argc, char* argv[])
{
    signal(SIGINT, sigHandler);
    signal(SIGTERM, sigHandler);

    unsigned num_files = 0;
    unsigned num_loops = 1;
    NEXUS_Error rc;
    NEXUS_PlatformSettings platformSettings;
    NEXUS_MemoryConfigurationSettings memConfigSettings;

    s_app.num_mosaics = 1;

    if (argc < 2) {
        print_usage(argv[0]);
        exit(EXIT_FAILURE);
    }

    for (int i = 1; i < argc; i++) {
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
        else if (strcmp(argv[i], "-hdcp2x_keys") == 0) {
            if (i >= argc - 1) {
                print_usage(argv[0]);
                exit(EXIT_FAILURE);
            }
            s_app.hdcp2xKey.assign(argv[++i]);
            LOGW(("HDCP2x Key: %s", s_app.hdcp2xKey.c_str()));
        }
        else if (strcmp(argv[i], "-export") == 0) {
            if (i >= argc - 1) {
                print_usage(argv[0]);
                exit(EXIT_FAILURE);
            }
            s_app.exportHeapSize = parse_size(argv[++i]);
#ifndef NEXUS_EXPORT_HEAP
            LOGW(("Export heap is not available"));
#endif
        }
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
        else if (!strcmp(argv[i], "-display_format")) {
            if (i >= argc - 1) {
                print_usage(argv[0]);
                exit(EXIT_FAILURE);
            }
            s_app.format = (NEXUS_VideoFormat)lookup(g_videoFormatStrs, argv[++i]);
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

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;

#ifdef NEXUS_EXPORT_HEAP
    /* Configure export heap since it's not allocated by nexus by default */
    if (s_app.exportHeapSize != 0) {
        LOGW(("Configure XRR to %u bytes", s_app.exportHeapSize));
        platformSettings.heap[NEXUS_EXPORT_HEAP].size = s_app.exportHeapSize;
    }
#endif

    NEXUS_GetDefaultMemoryConfigurationSettings(&memConfigSettings);
    if (secure_video)
    {
		int i,j;

        /* Request secure picture buffers, i.e. URR
        * Should only do this if SAGE is in use, and when SAGE_SECURE_MODE is NOT 1 */
        /* For now default to SVP2.0 type configuration (i.e. ALL buffers are
        * secure ONLY */
        for (i=0;i<NEXUS_NUM_VIDEO_DECODERS;i++)
        {
            memConfigSettings.videoDecoder[i].secure = NEXUS_SecureVideo_eSecure;
        }
        for (i=0;i<NEXUS_NUM_DISPLAYS;i++)
        {
            for (j=0;j<NEXUS_NUM_VIDEO_WINDOWS;j++)
            {
                memConfigSettings.display[i].window[j].secure = NEXUS_SecureVideo_eSecure;
            }
        }
    }

    rc = NEXUS_Platform_MemConfigInit(&platformSettings, &memConfigSettings);
    if (rc) {
        LOGE(("NEXUS_Platform_Init failed"));
        exit(EXIT_FAILURE);
    }

    LOGD(("@@@ Check Point #01"));

    setup_files();

    setup_parsers();

    setup_gui();

    setup_streamers();

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
