/******************************************************************************
 *    (c)2015 Broadcom Corporation
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
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_video_input.h"
#include "nexus_spdif_output.h"
#include "nexus_component_output.h"
#include "nexus_video_adj.h"

//#define TEST_SECURE_COPY

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

#include "nxclient.h"
#include "nexus_surface_client.h"

#ifdef ANDROID
#define LOG_NDEBUG 0
#define LOG_TAG "playback_dif"
#include <utils/Log.h>
#define LOGE(x) ALOGE x
#define LOGW(x) ALOGW x
#define LOGD(x) ALOGD x
#define LOGV(x) ALOGV x
#else
BDBG_MODULE(playback_dif);
#undef LOGE
#undef LOGW
#undef LOGD
#undef LOGV
#define LOGE BDBG_ERR
#define LOGW BDBG_WRN
#define LOGD BDBG_MSG
#define LOGV BDBG_MSG
#endif

// DRM Integration Framework
//#include "secure_playback.h"
#include "decryptor.h"
#include "media_parser.h"
#include "cenc_parser.h"
#include "piff_parser.h"

#include "dump_hex.h"

//#define NEED_TO_BE_TRUSTED_APP
#ifdef NEED_TO_BE_TRUSTED_APP
#define NEXUS_TRUSTED_DATA_PATH "/data/misc/nexus"
#endif

#define ZORDER_TOP 10

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

    MediaParser* parser;
    IDecryptor* decryptor;
    IStreamer* videoStreamer;
    IStreamer* audioStreamer;

    uint8_t *pAvccHdr;
    uint8_t *pPayload;
    uint8_t *pAudioHeaderBuf;
    uint8_t *pVideoHeaderBuf;

    FILE *fp_mp4;
    NEXUS_SimpleVideoDecoderHandle videoDecoder;
    NEXUS_SimpleAudioDecoderHandle audioDecoder;
    NEXUS_PlaypumpHandle videoPlaypump;
    NEXUS_PlaypumpHandle audioPlaypump;
    NEXUS_PidChannelHandle videoPidChannel;
    NEXUS_PidChannelHandle audioPidChannel;
    BKNI_EventHandle event;
    NEXUS_SurfaceClientHandle surfaceClient;

    uint64_t last_video_fragment_time;
    uint64_t last_audio_fragment_time;

    unsigned s_connectId;
    NxClient_AllocResults s_allocResults;
};

AppContext::AppContext()
{
    parser = NULL;
    decryptor = NULL;
    videoStreamer = NULL;
    audioStreamer = NULL;

    pAvccHdr = NULL;
    pPayload = NULL;
    pAudioHeaderBuf = NULL;
    pVideoHeaderBuf = NULL;

    fp_mp4 = NULL;
    videoDecoder = NULL;
    audioDecoder = NULL;
    videoPidChannel = NULL;
    audioPidChannel = NULL;
    event = NULL;
    surfaceClient = NULL;

    last_video_fragment_time = 0;
    last_audio_fragment_time = 0;

    s_connectId = 0xFFFF;
}

AppContext::~AppContext()
{
    LOGW(("%s: start cleaning up", __FUNCTION__));
    if (videoDecoder) {
        NEXUS_SimpleVideoDecoder_Stop(videoDecoder);
    }
    if (audioDecoder) {
        NEXUS_SimpleAudioDecoder_Stop(audioDecoder);
    }

    if (pAvccHdr) NEXUS_Memory_Free(pAvccHdr);
    if (pPayload) NEXUS_Memory_Free(pPayload);
    if (pAudioHeaderBuf) NEXUS_Memory_Free(pAudioHeaderBuf);
    if (pVideoHeaderBuf) NEXUS_Memory_Free(pVideoHeaderBuf);

    if(parser) {
        delete parser;
        LOGW(("Destroying parser %p", parser));
    }

    if (fp_mp4) fclose(fp_mp4);

    if (decryptor != NULL) {
        LOGW(("Destroying decryptor %p", decryptor));
        DecryptorFactory::DestroyDecryptor(decryptor);
    }

    if (videoStreamer != NULL) {
        LOGW(("Destroying videoStreamer %p", videoStreamer));
        StreamerFactory::DestroyStreamer(videoStreamer);
    }

    if (audioStreamer != NULL) {
        LOGW(("Destroying audioStreamer %p", audioStreamer));
        StreamerFactory::DestroyStreamer(audioStreamer);
    }

    if (videoDecoder != NULL) {
        NEXUS_SimpleVideoDecoder_Release(videoDecoder);
    }

    if (audioDecoder != NULL) {
        NEXUS_SimpleAudioDecoder_Release(audioDecoder);
    }

    if (surfaceClient != NULL) {
        NEXUS_SurfaceClient_Release(surfaceClient);
    }

    if (event != NULL) {
        BKNI_DestroyEvent((BKNI_EventHandle)event);
    }

    if (s_connectId != 0xFFFF) {
        NxClient_Disconnect(s_connectId);
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

int vc1_stream = 0;                        /* stream type */
DrmType requestedDrmType = drm_type_eUnknown;
static int video_decode_hdr=0;

static AppContext s_app;

static int gui_init();

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
    void *decoder_data, size_t decoder_len)
{
    IStreamer* streamer;
    SampleInfo *pSample;
    size_t pes_header_len;
    bmedia_pes_info pes_info;
    uint64_t frag_duration;
    uint32_t bytes_processed = 0;
    uint32_t last_bytes_processed = 0;
    if (frag_info->samples_enc->sample_count == 0) {
        LOGE(("%s: No samples", __FUNCTION__));
        return -1;
    }

    LOGD(("%s: #samples=%d",__FUNCTION__, frag_info->samples_enc->sample_count));
    for (unsigned i = 0; i < frag_info->samples_enc->sample_count; i++) {
        uint32_t numOfByteDecrypted = 0;
        size_t sampleSize = 0;

        pSample = &frag_info->samples_enc->samples[i];
        numOfByteDecrypted = sampleSize = frag_info->sample_info[i].size;

        if (frag_info->trackType == BMP4_SAMPLE_ENCRYPTED_VIDEO ||
            frag_info->trackType == BMP4_SAMPLE_AVC) {
            streamer = s_app.videoStreamer;
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

#ifdef TEST_SECURE_COPY
            IBuffer* decOutput = BufferFactory::CreateBuffer(sampleSize, NULL, true);
            numOfByteDecrypted = s_app.decryptor->DecryptSample(pSample,
                input, decOutput, sampleSize);
            if (numOfByteDecrypted != sampleSize)
            {
                LOGE(("%s Failed to decrypt sample, can't continue - %d", __FUNCTION__, __LINE__));
                return -1;
            }
            SecureBuffer* output = (SecureBuffer*)streamer->GetBuffer(sampleSize);
            // This is R2R copy
            output->Copy(0, decOutput, sampleSize);
#else
            IBuffer* decOutput = streamer->GetBuffer(sampleSize);
            numOfByteDecrypted = s_app.decryptor->DecryptSample(pSample,
                input, decOutput, sampleSize);
            if (numOfByteDecrypted != sampleSize)
            {
                LOGE(("%s Failed to decrypt sample, can't continue - %d", __FUNCTION__, __LINE__));
                BufferFactory::DestroyBuffer(input);
                BufferFactory::DestroyBuffer(decOutput);
                return -1;
            }
#endif
            BufferFactory::DestroyBuffer(input);
            BufferFactory::DestroyBuffer(decOutput);
            bytes_processed += numOfByteDecrypted;
        } else if (frag_info->trackType == BMP4_SAMPLE_ENCRYPTED_AUDIO ||
            frag_info->trackType == BMP4_SAMPLE_MP4A) {
            streamer = s_app.audioStreamer;
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

            IBuffer* output =
                streamer->GetBuffer(pes_header_len + BMEDIA_ADTS_HEADER_SIZE);
            output->Copy(0, s_app.pAudioHeaderBuf, pes_header_len + BMEDIA_ADTS_HEADER_SIZE);
            bytes_processed += pes_header_len + BMEDIA_ADTS_HEADER_SIZE;

            IBuffer* input = BufferFactory::CreateBuffer(sampleSize, (uint8_t*)frag_info->cursor.cursor);
            batom_cursor_skip(&frag_info->cursor, sampleSize);

            IBuffer* decOutput = streamer->GetBuffer(sampleSize);
            numOfByteDecrypted = s_app.decryptor->DecryptSample(pSample,
                input, decOutput, sampleSize);
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
        } else {
            LOGW(("%s Unsupported track type %d detected", __FUNCTION__, frag_info->trackType));
            return -1;
        }

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

    for (;;) {
        rc = NEXUS_Playpump_GetStatus(s_app.videoPlaypump, &playpumpStatus);
        if (rc != NEXUS_SUCCESS)
            break;

        if (playpumpStatus.fifoDepth == 0) {
            break;
        }
        BKNI_Sleep(100);
    }

    for (;;) {
        rc = NEXUS_Playpump_GetStatus(s_app.audioPlaypump, &playpumpStatus);
        if (rc != NEXUS_SUCCESS)
            break;

        if (playpumpStatus.fifoDepth == 0)
            break;

        BKNI_Sleep(100);
    }

    if (s_app.videoDecoder) {
        for (;;) {
            NEXUS_VideoDecoderStatus decoderStatus;
            rc = NEXUS_SimpleVideoDecoder_GetStatus(s_app.videoDecoder, &decoderStatus);
            if (rc != NEXUS_SUCCESS)
                break;

            if (decoderStatus.queueDepth == 0)
                break;

            BKNI_Sleep(100);
        }
    }

    if (s_app.audioDecoder) {
        for (;;) {
            NEXUS_AudioDecoderStatus decoderStatus;
            rc = NEXUS_SimpleAudioDecoder_GetStatus(s_app.audioDecoder, &decoderStatus);
            if (rc != NEXUS_SUCCESS)
                break;

            if (decoderStatus.queuedFrames < 4)
                break;

            BKNI_Sleep(100);
        }
    }
    return;
}

void playback_mp4(NEXUS_SimpleVideoDecoderHandle videoDecoder,
    NEXUS_SimpleAudioDecoderHandle audioDecoder, char *mp4_file)
{
    NEXUS_ClientConfiguration clientConfig;
    NEXUS_MemoryAllocationSettings memSettings;
    NEXUS_SimpleStcChannelHandle stcChannel;
    NEXUS_SimpleVideoDecoderStartSettings videoProgram;
    NEXUS_SimpleAudioDecoderStartSettings audioProgram;
    NEXUS_SimpleStcChannelSettings stcSettings;
    NEXUS_PlaypumpSettings playpumpSettings;
    NEXUS_Error rc;

    // New API stuffs
    s_app.videoStreamer = StreamerFactory::CreateStreamer();
    s_app.audioStreamer = StreamerFactory::CreateStreamer();

    std::string license_server;
    std::string key_response;

    std::string cpsSpecificData;    /*content protection system specific data*/
    std::string psshDataStr;
    DrmType drmType;

    mp4_parse_frag_info frag_info;
    void *decoder_data;
    size_t decoder_len;

    bool use_default_url = false;

    if (s_app.videoStreamer == NULL || s_app.audioStreamer == NULL ) {
        exit(EXIT_FAILURE);
    }
    s_app.videoPlaypump = s_app.videoStreamer->OpenPlaypump(true);
    s_app.audioPlaypump = s_app.audioStreamer->OpenPlaypump(false);

    LOGD(("%s - %d\n", __FUNCTION__, __LINE__));
    if (mp4_file == NULL ) {
        exit(EXIT_FAILURE);
    }

    LOGD(("MP4 file: %s\n",mp4_file));
    fflush(stdout);

    s_app.fp_mp4 = fopen(mp4_file, "rb");
    if (s_app.fp_mp4 == NULL) {
        LOGE(("failed to open %s", mp4_file));
        exit(EXIT_FAILURE);
    }

    s_app.parser = new PiffParser(s_app.fp_mp4);
    if (s_app.parser ==  NULL) {
        LOGE(("failed to new a PiffParser instance"));
        exit(EXIT_FAILURE);
    }

    if (s_app.parser->Initialize()) {
        LOGW(("PiffParser was initialized for %s", mp4_file));
        use_default_url = true;
    } else {
        LOGW(("failed to initialize the PiffParser for %s, try CencParser...", mp4_file));
        delete s_app.parser;

        s_app.parser = new CencParser(s_app.fp_mp4);
        if (s_app.parser ==  NULL) {
            LOGE(("failed to new a CencParser instance"));
            exit(EXIT_FAILURE);
        }

        if (!s_app.parser->Initialize()) {
            LOGE(("failed to initialize the CencParser for %s", mp4_file));
            exit(EXIT_FAILURE);
        }
    }

    if(requestedDrmType != drm_type_eUnknown){
        bool drmMatch = false;
        DrmType drmTypes[BMP4_MAX_DRM_SCHEMES];
        uint8_t numOfDrmSchemes = s_app.parser->GetNumOfDrmSchemes(drmTypes, BMP4_MAX_DRM_SCHEMES);
        for(int i = 0; i<numOfDrmSchemes; i++){
            if(drmTypes[i] == requestedDrmType){
                drmMatch = true;
                s_app.parser-> SetDrmSchemes(i);
            }
        }
        if(!drmMatch){
            LOGE(("DRM Type: %d was not found in the stream.", requestedDrmType ));
            LOGE(("Do you want to play it with its default DRM type: %d? [y/n]", drmTypes[0]));
            char resp;
            scanf("%c", &resp);
            if(resp != 'y')
                exit(EXIT_FAILURE);
        }
    }

    NEXUS_Platform_GetClientConfiguration(&clientConfig);
    NEXUS_Memory_GetDefaultAllocationSettings(&memSettings);
    memSettings.heap = clientConfig.heap[1]; /* heap 1 is the eFull heap for the nxclient. */

    s_app.pAvccHdr = NULL;
    if (NEXUS_Memory_Allocate(BMP4_MAX_PPS_SPS, &memSettings, (void **)&s_app.pAvccHdr) !=  NEXUS_SUCCESS) {
        fprintf(stderr,"NEXUS_Memory_Allocate failed");
        exit(EXIT_FAILURE);
    }

    s_app.pPayload = NULL;
    if (NEXUS_Memory_Allocate(BUF_SIZE, &memSettings, (void **)&s_app.pPayload) !=  NEXUS_SUCCESS) {
        fprintf(stderr,"NEXUS_Memory_Allocate failed");
        exit(EXIT_FAILURE);
    }

    s_app.pAudioHeaderBuf = NULL;
    if (NEXUS_Memory_Allocate(BMEDIA_PES_HEADER_MAX_SIZE + BMP4_MAX_PPS_SPS,
        &memSettings, (void **)&s_app.pAudioHeaderBuf) !=  NEXUS_SUCCESS) {
        fprintf(stderr,"NEXUS_Memory_Allocate failed");
        exit(EXIT_FAILURE);
    }

    s_app.pVideoHeaderBuf = NULL;
    if (NEXUS_Memory_Allocate(BMEDIA_PES_HEADER_MAX_SIZE + BMP4_MAX_PPS_SPS,
        &memSettings, (void **)&s_app.pVideoHeaderBuf) !=  NEXUS_SUCCESS) {
        fprintf(stderr,"NEXUS_Memory_Allocate failed");
        exit(EXIT_FAILURE);
    }

    stcChannel = NEXUS_SimpleStcChannel_Create(NULL);
    NEXUS_SimpleStcChannel_GetSettings(stcChannel, &stcSettings);
    stcSettings.mode = NEXUS_StcChannelMode_eAuto;
    rc = NEXUS_SimpleStcChannel_SetSettings(stcChannel, &stcSettings);
    if (rc) {
       LOGW(("@@@ Stc Set FAILED ---------------"));
    }

    BKNI_CreateEvent(&s_app.event);

    NEXUS_Playpump_GetSettings(s_app.videoPlaypump, &playpumpSettings);
    playpumpSettings.dataCallback.callback = play_callback;
    playpumpSettings.dataCallback.context = s_app.event;
    playpumpSettings.transportType = NEXUS_TransportType_eMpeg2Pes;
    NEXUS_Playpump_SetSettings(s_app.videoPlaypump, &playpumpSettings);

    NEXUS_Playpump_GetSettings(s_app.audioPlaypump, &playpumpSettings);
    playpumpSettings.dataCallback.callback = play_callback;
    playpumpSettings.dataCallback.context = s_app.event;
    playpumpSettings.transportType = NEXUS_TransportType_eMpeg2Pes;
    NEXUS_Playpump_SetSettings(s_app.audioPlaypump, &playpumpSettings);

    NEXUS_Playpump_Start(s_app.videoPlaypump);
    NEXUS_Playpump_Start(s_app.audioPlaypump);

    NEXUS_PlaypumpOpenPidChannelSettings video_pid_settings;
    NEXUS_Playpump_GetDefaultOpenPidChannelSettings(&video_pid_settings);
    video_pid_settings.pidType = NEXUS_PidType_eVideo;

    s_app.videoPidChannel = s_app.videoStreamer->OpenPidChannel(REPACK_VIDEO_PES_ID, &video_pid_settings);
    if ( !s_app.videoPidChannel )
      LOGW(("@@@ videoPidChannel NULL"));
    else
      LOGW(("@@@ videoPidChannel OK"));

    s_app.audioPidChannel = s_app.audioStreamer->OpenPidChannel(REPACK_AUDIO_PES_ID, NULL);

    if ( !s_app.audioPidChannel )
      LOGW(("@@@ audioPidChannel NULL"));
    else
      LOGW(("@@@ audioPidChannel OK"));

    NEXUS_SimpleAudioDecoder_GetDefaultStartSettings(&audioProgram);
    NEXUS_SimpleVideoDecoder_GetDefaultStartSettings(&videoProgram);

    if ( vc1_stream ) {
       LOGW(("@@@ set video audio program for vc1"));
       videoProgram.settings.codec = NEXUS_VideoCodec_eVc1;
       audioProgram.primary.codec = NEXUS_AudioCodec_eWmaPro;
    } else {
       LOGW(("@@@ set video audio program for h264"));
       videoProgram.settings.codec = NEXUS_VideoCodec_eH264;
       audioProgram.primary.codec = NEXUS_AudioCodec_eAacAdts;
    }

    videoProgram.settings.pidChannel = s_app.videoPidChannel;
    NEXUS_SimpleVideoDecoder_Start(videoDecoder, &videoProgram);

    audioProgram.primary.pidChannel = s_app.audioPidChannel;
    NEXUS_SimpleAudioDecoder_Start(audioDecoder, &audioProgram);

    if (videoProgram.settings.pidChannel) {
        LOGW(("@@@ set stc channel video"));
        NEXUS_SimpleVideoDecoder_SetStcChannel(videoDecoder, stcChannel);
    }

    if (audioProgram.primary.pidChannel) {
        LOGW(("@@@ set stc channel audio"));
        NEXUS_SimpleAudioDecoder_SetStcChannel(audioDecoder, stcChannel);
    }

    // New API - creating Decryptor
    drmType = s_app.parser->GetDrmType();
    s_app.decryptor = DecryptorFactory::CreateDecryptor(drmType);
    if (s_app.decryptor == NULL) {
        LOGE(("Failed to create Decryptor"));
        exit(EXIT_FAILURE);
    }

    // New API - initializing Decryptor
    psshDataStr = s_app.parser->GetPssh();
    dump_hex("pssh", psshDataStr.data(), psshDataStr.size());
    if (!s_app.decryptor->Initialize(psshDataStr)) {
           LOGE(("Failed to initialize Decryptor"));
        exit(EXIT_FAILURE);
    }

    // New API - GenerateKeyRequest
    cpsSpecificData = s_app.parser->GetCpsSpecificData();
    dump_hex("cpsSpecificData", cpsSpecificData.data(), cpsSpecificData.size());
    if (!s_app.decryptor->GenerateKeyRequest(cpsSpecificData)) {
        LOGE(("Failed to generate key request"));
        exit(EXIT_FAILURE);
    }

    dump_hex("message", s_app.decryptor->GetKeyMessage().data(), s_app.decryptor->GetKeyMessage().size());

    if (use_default_url) {
        license_server.clear();
    } else {
#if 1
        // Use Google Play server
        if (drmType == drm_type_eWidevine) {
            license_server.assign(kGpLicenseServer + kGpWidevineAuth);
        } else if (drmType == drm_type_ePlayready) {
            license_server.assign(kGpLicenseServer + kGpPlayreadyAuth);
        }
#else
        // Use Content Protection server
        license_server.assign(kCpLicenseServer + kCpClientAuth);
#endif
        license_server.append(kGpClientOfflineQueryParameters);
    }

    // New API - GetKeyRequestResponse
    key_response = s_app.decryptor->GetKeyRequestResponse(license_server);

    // New API - AddKey
    if (!s_app.decryptor->AddKey(key_response)) {
        LOGE(("Failed to add key"));
        exit(EXIT_FAILURE);
    }
    dump_hex("key_response", key_response.data(), key_response.size());

    for (;;) {
        decoder_data = s_app.parser->GetFragmentData(frag_info, s_app.pPayload, decoder_len);
        if (decoder_data == NULL || shouldExit)
            break;

        if (process_fragment(&frag_info, decoder_data, decoder_len) != 0) {
            exit(EXIT_FAILURE);
        }
    }

    return;
}

int main(int argc, char* argv[])
{
    signal(SIGINT, sigHandler);
    signal(SIGTERM, sigHandler);

    NxClient_JoinSettings joinSettings;
    NxClient_AllocSettings allocSettings;
    NEXUS_Error rc;

    NxClient_ConnectSettings connectSettings;
    NEXUS_SurfaceComposition comp;
    NEXUS_SurfaceClientHandle videoSurfaceClient = NULL;

/*
#ifdef NEED_TO_BE_TRUSTED_APP
    char nx_key[PROPERTY_VALUE_MAX];
    FILE *key = NULL;

        sprintf(nx_key, "%s/nx_key", NEXUS_TRUSTED_DATA_PATH);
        key = fopen(nx_key, "r");

        if (key == NULL) {
           fprintf(stderr, "%s: failed to open key file \'%s\', err=%d (%s)\n", __FUNCTION__, nx_key, errno, strerror(errno));
        } else {
           memset(nx_key, 0, sizeof(nx_key));
           fread(nx_key, PROPERTY_VALUE_MAX, 1, key);
           fclose(key);
        }
#endif
*/
    NxClient_GetDefaultJoinSettings(&joinSettings);
    snprintf(joinSettings.name, NXCLIENT_MAX_NAME, "playback_dif");
    joinSettings.ignoreStandbyRequest = true;

/*
#ifdef NEED_TO_BE_TRUSTED_APP
        joinSettings.mode = NEXUS_ClientMode_eUntrusted;
        if (strlen(nx_key)) {
           if (strstr(nx_key, "trusted:") == nx_key) {
              const char *password = &nx_key[8];
              joinSettings.mode = NEXUS_ClientMode_eProtected;
              joinSettings.certificate.length = strlen(password);
              memcpy(joinSettings.certificate.data, password, joinSettings.certificate.length);
           }
        }
#endif
*/
    rc = NxClient_Join(&joinSettings);
    if (rc)
        exit(EXIT_FAILURE);

    NxClient_GetDefaultAllocSettings(&allocSettings);
    allocSettings.simpleVideoDecoder = NEXUS_ANY_ID;
    allocSettings.simpleAudioDecoder = NEXUS_ANY_ID;
    allocSettings.surfaceClient = NEXUS_ANY_ID;
    rc = NxClient_Alloc(&allocSettings, &s_app.s_allocResults);
    if (rc)
        exit(EXIT_FAILURE);

    if (argc < 2) {
        LOGE(("Usage : %s <input_file> [OPTIONS]", argv[0]));
        LOGE(("        -vc1         vc1 stream type"));
        LOGE(("        -pr          set DRM type as playready"));
        LOGE(("        -wv          set DRM type as widevine"));
        LOGE(("        -noloop      no decrypt loop for testing"));
        LOGE(("        -secure      Use secure video path"));
        exit(EXIT_FAILURE);
    }

    for (int i=0; i<argc; i++){
        if(strcmp(argv[i], "-vc1") == 0)
            vc1_stream = 1;
        if(strcmp(argv[i], "-pr") == 0)
            requestedDrmType = drm_type_ePlayready;
        if(strcmp(argv[i], "-wv") == 0)
            requestedDrmType = drm_type_eWidevine;
        if(strcmp(argv[i], "-noloop") == 0)
            shouldExit = true;
		if(strcmp(argv[i], "-secure") == 0)
			secure_video = true;
    }

    LOGD(("@@@ Check Point #01"));

    if (s_app.s_allocResults.simpleVideoDecoder[0].id) {
        LOGD(("@@@ to acquire video decoder %d", s_app.s_allocResults.simpleVideoDecoder[0].id));
        s_app.videoDecoder = NEXUS_SimpleVideoDecoder_Acquire(s_app.s_allocResults.simpleVideoDecoder[0].id);
    }
    if (s_app.videoDecoder == NULL) {
        exit(EXIT_FAILURE);
    }
    if (s_app.s_allocResults.simpleAudioDecoder.id) {
        LOGD(("@@@ to acquire audio decoder %d", s_app.s_allocResults.simpleAudioDecoder.id));
        s_app.audioDecoder = NEXUS_SimpleAudioDecoder_Acquire(s_app.s_allocResults.simpleAudioDecoder.id);
    }
    if (s_app.audioDecoder == NULL) {
        exit(EXIT_FAILURE);
    }
    if (s_app.s_allocResults.surfaceClient[0].id) {
        LOGD(("@@@ to acquire surfaceclient"));
        /* surfaceClient is the top-level graphics window in which video will fit.
        videoSurfaceClient must be "acquired" to associate the video window with surface compositor.
        Graphics do not have to be submitted to surfaceClient for video to work, but at least an
        "alpha hole" surface must be submitted to punch video through other client's graphics.
        Also, the top-level surfaceClient ID must be submitted to NxClient_ConnectSettings below. */
        s_app.surfaceClient = NEXUS_SurfaceClient_Acquire(s_app.s_allocResults.surfaceClient[0].id);
        videoSurfaceClient = NEXUS_SurfaceClient_AcquireVideoWindow(s_app.surfaceClient, 0);

        NxClient_GetSurfaceClientComposition(s_app.s_allocResults.surfaceClient[0].id, &comp);
        comp.zorder = ZORDER_TOP;   /* try to stay on top most */
        NxClient_SetSurfaceClientComposition(s_app.s_allocResults.surfaceClient[0].id, &comp);
    }
    NxClient_GetDefaultConnectSettings(&connectSettings);
    connectSettings.simpleVideoDecoder[0].id = s_app.s_allocResults.simpleVideoDecoder[0].id;
    connectSettings.simpleVideoDecoder[0].surfaceClientId = s_app.s_allocResults.surfaceClient[0].id;
    connectSettings.simpleAudioDecoder.id = s_app.s_allocResults.simpleAudioDecoder.id;

	if (secure_video)
	{
		connectSettings.simpleVideoDecoder[0].decoderCapabilities.secureVideo=true;
	}

    rc = NxClient_Connect(&connectSettings, &s_app.s_connectId);
    if (rc)
        exit(EXIT_FAILURE);

    NxClient_AudioSettings audioSettings;
    NxClient_GetAudioSettings(&audioSettings);

    audioSettings.muted = false;
    audioSettings.volumeType = NEXUS_AudioVolumeType_eLinear;
    audioSettings.rightVolume = audioSettings.leftVolume = NEXUS_AUDIO_VOLUME_LINEAR_NORMAL;
    rc = NxClient_SetAudioSettings(&audioSettings);

    gui_init();

    playback_mp4(s_app.videoDecoder, s_app.audioDecoder, argv[1]);

    if (shouldExit) {
        exit(2);
    } else {
        wait_for_drain();
        exit(EXIT_SUCCESS);
    }
}

static int gui_init()
{
    NEXUS_Graphics2DHandle gfx;
    NEXUS_SurfaceHandle surface;

    NEXUS_Graphics2DSettings gfxSettings;
    NEXUS_SurfaceCreateSettings createSettings;
    NEXUS_Graphics2DFillSettings fillSettings;
    int rc;

    if (!s_app.surfaceClient)
        exit(EXIT_FAILURE);

    LOGD(("@@@ gui_init surfaceclient %d", (int)s_app.surfaceClient));
    gfx = NEXUS_Graphics2D_Open(NEXUS_ANY_ID, NULL);
    NEXUS_Graphics2D_GetSettings(gfx, &gfxSettings);
    rc = NEXUS_Graphics2D_SetSettings(gfx, &gfxSettings);
    if (rc != 0)
        exit(EXIT_FAILURE);

    NEXUS_Surface_GetDefaultCreateSettings(&createSettings);
    createSettings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
    createSettings.width = 720;
    createSettings.height = 480;
    surface = NEXUS_Surface_Create(&createSettings);

    NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
    fillSettings.surface = surface;
    fillSettings.color = 0;
    rc = NEXUS_Graphics2D_Fill(gfx, &fillSettings);
    if (rc != 0)
        exit(EXIT_FAILURE);

    rc = NEXUS_Graphics2D_Checkpoint(gfx, NULL); /* require to execute queue */

    rc = NEXUS_SurfaceClient_SetSurface(s_app.surfaceClient, surface);
    if (rc != 0)
        exit(EXIT_FAILURE);

    return 0;
}
