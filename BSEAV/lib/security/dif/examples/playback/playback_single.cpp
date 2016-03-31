/******************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
 ******************************************************************************/

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
    NEXUS_DisplayHandle display;
    NEXUS_VideoWindowHandle window;
    NEXUS_VideoDecoderHandle videoDecoder;
    NEXUS_AudioDecoderHandle audioDecoder;
    NEXUS_PlaypumpHandle videoPlaypump;
    NEXUS_PlaypumpHandle audioPlaypump;
    NEXUS_PidChannelHandle videoPidChannel;
    NEXUS_PidChannelHandle audioPidChannel;
    BKNI_EventHandle event;

    uint64_t last_video_fragment_time;
    uint64_t last_audio_fragment_time;
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
    display = NULL;
    window = NULL;
    videoDecoder = NULL;
    audioDecoder = NULL;
    videoPidChannel = NULL;
    audioPidChannel = NULL;
    event = NULL;

    last_video_fragment_time = 0;
    last_audio_fragment_time = 0;
}

AppContext::~AppContext()
{
    LOGW(("%s: start cleaning up", __FUNCTION__));
    if (videoDecoder) {
        NEXUS_VideoDecoder_Stop(videoDecoder);
    }
    if (audioDecoder) {
        NEXUS_AudioDecoder_Stop(audioDecoder);
    }

    if (pAvccHdr) NEXUS_Memory_Free(pAvccHdr);
    if (pPayload) NEXUS_Memory_Free(pPayload);
    if (pAudioHeaderBuf) NEXUS_Memory_Free(pAudioHeaderBuf);
    if (pVideoHeaderBuf) NEXUS_Memory_Free(pVideoHeaderBuf);

    if(parser) {
        LOGW(("Destroying parser %p", parser));
        delete parser;
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
        LOGW(("Closing videoDecoder %p", videoDecoder));
        NEXUS_VideoDecoder_Close(videoDecoder);
    }

    if (audioDecoder != NULL) {
        LOGW(("Closing audioDecoder %p", audioDecoder));
        NEXUS_AudioDecoder_Close(audioDecoder);
    }

    if (window != NULL) {
        LOGW(("Closing window %p", window));
        NEXUS_VideoWindow_Close(window);
    }

    if (display != NULL) {
        LOGW(("Closing display %p", display));
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

int vc1_stream = 0;                        /* stream type */
DrmType requestedDrmType = drm_type_eUnknown;
static int video_decode_hdr=0;
static bool secure_video=false;

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
    void *decoder_data, size_t decoder_len)
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
            rc = NEXUS_VideoDecoder_GetStatus(s_app.videoDecoder, &decoderStatus);
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
            rc = NEXUS_AudioDecoder_GetStatus(s_app.audioDecoder, &decoderStatus);
            if (rc != NEXUS_SUCCESS)
                break;

            if (decoderStatus.queuedFrames < 4)
                break;

            BKNI_Sleep(100);
        }
    }
    return;
}

void playback_mp4(NEXUS_VideoDecoderHandle videoDecoder,
    NEXUS_AudioDecoderHandle audioDecoder, char *mp4_file)
{
    NEXUS_MemoryAllocationSettings memSettings;
    NEXUS_StcChannelHandle stcChannel;
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_AudioDecoderStartSettings audioProgram;
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_PlaypumpSettings playpumpSettings;

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

    NEXUS_Memory_GetDefaultAllocationSettings(&memSettings);
    memSettings.heap = NEXUS_MEMC0_MAIN_HEAP;

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

    NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;
    stcSettings.mode = NEXUS_StcChannelMode_eAuto;
    stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);

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

    NEXUS_AudioDecoder_GetDefaultStartSettings(&audioProgram);
    NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);

    if ( vc1_stream ) {
       LOGW(("@@@ set video audio program for vc1"));
       videoProgram.codec = NEXUS_VideoCodec_eVc1;
       audioProgram.codec = NEXUS_AudioCodec_eWmaPro;
    } else {
       LOGW(("@@@ set video audio program for h264"));
       videoProgram.codec = NEXUS_VideoCodec_eH264;
       audioProgram.codec = NEXUS_AudioCodec_eAacAdts;
    }

    videoProgram.pidChannel = s_app.videoPidChannel;
    videoProgram.stcChannel = stcChannel;
    NEXUS_VideoDecoder_Start(videoDecoder, &videoProgram);

    audioProgram.pidChannel = s_app.audioPidChannel;
    audioProgram.stcChannel = stcChannel;
    NEXUS_AudioDecoder_Start(audioDecoder, &audioProgram);

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

    NEXUS_Error rc;
    NEXUS_VideoDecoderOpenSettings videoDecoderOpenSettings;
    NEXUS_AudioDecoderOpenSettings audioDecoderOpenSettings;
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_MemoryConfigurationSettings memConfigSettings;
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputStatus hdmiStatus;
    NEXUS_DisplaySettings displaySettings;
#endif

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

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;

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
    NEXUS_Platform_GetConfiguration(&platformConfig);

    /* Bring up video display and outputs */
    s_app.display = NEXUS_Display_Open(0, NULL);
    s_app.window = NEXUS_VideoWindow_Open(s_app.display, 0);

#if NEXUS_NUM_COMPONENT_OUTPUTS
    NEXUS_Display_AddOutput(s_app.display, NEXUS_ComponentOutput_GetConnector(platformConfig.outputs.component[0]));
#endif
#if NEXUS_NUM_COMPOSITE_OUTPUTS
    NEXUS_Display_AddOutput(s_app.display, NEXUS_CompositeOutput_GetConnector(platformConfig.outputs.composite[0]));
#endif
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_Display_AddOutput(s_app.display, NEXUS_HdmiOutput_GetVideoConnector(platformConfig.outputs.hdmi[0]));
    rc = NEXUS_HdmiOutput_GetStatus(platformConfig.outputs.hdmi[0], &hdmiStatus);
    if ( !rc && hdmiStatus.connected )
    {
        /* If current display format is not supported by monitor, switch to monitor's preferred format.
           If other connected outputs do not support the preferred format, a harmless error will occur. */
        NEXUS_Display_GetSettings(s_app.display, &displaySettings);
        if ( !hdmiStatus.videoFormatSupported[displaySettings.format] ) {
            displaySettings.format = hdmiStatus.preferredVideoFormat;
            NEXUS_Display_SetSettings(s_app.display, &displaySettings);
        }
    }
#endif

#if NEXUS_NUM_COMPONENT_OUTPUTS
    NEXUS_Display_AddOutput(s_app.display, NEXUS_ComponentOutput_GetConnector(platformConfig.outputs.component[0]));
#endif
#if NEXUS_NUM_COMPOSITE_OUTPUTS
    NEXUS_Display_AddOutput(s_app.display, NEXUS_CompositeOutput_GetConnector(platformConfig.outputs.composite[0]));
#endif
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_Display_AddOutput(s_app.display, NEXUS_HdmiOutput_GetVideoConnector(platformConfig.outputs.hdmi[0]));
    rc = NEXUS_HdmiOutput_GetStatus(platformConfig.outputs.hdmi[0], &hdmiStatus);
    if ( !rc && hdmiStatus.connected )
    {
        /* If current display format is not supported by monitor, switch to monitor's preferred format.
           If other connected outputs do not support the preferred format, a harmless error will occur. */
        NEXUS_Display_GetSettings(s_app.display, &displaySettings);
        if ( !hdmiStatus.videoFormatSupported[displaySettings.format] ) {
            displaySettings.format = hdmiStatus.preferredVideoFormat;
            NEXUS_Display_SetSettings(s_app.display, &displaySettings);
        }
    }
#endif

    /* bring up decoder and connect to display */
    NEXUS_VideoDecoder_GetDefaultOpenSettings(&videoDecoderOpenSettings);
    videoDecoderOpenSettings.cdbHeap = platformConfig.heap[NEXUS_VIDEO_SECURE_HEAP];
    s_app.videoDecoder = NEXUS_VideoDecoder_Open(0, &videoDecoderOpenSettings);
    if (s_app.videoDecoder == NULL) {
        exit(EXIT_FAILURE);
    }
    NEXUS_VideoWindow_AddInput(s_app.window, NEXUS_VideoDecoder_GetConnector(s_app.videoDecoder));

    /* Bring up audio decoders and outputs */
    NEXUS_AudioDecoder_GetDefaultOpenSettings(&audioDecoderOpenSettings);
    audioDecoderOpenSettings.cdbHeap = platformConfig.heap[NEXUS_VIDEO_SECURE_HEAP];
    s_app.audioDecoder = NEXUS_AudioDecoder_Open(0, &audioDecoderOpenSettings);
    if (s_app.audioDecoder == NULL) {
        exit(EXIT_FAILURE);
    }
#if NEXUS_NUM_AUDIO_DACS
    NEXUS_AudioOutput_AddInput(
        NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]),
        NEXUS_AudioDecoder_GetConnector(s_app.audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
#endif
#if NEXUS_NUM_SPDIF_OUTPUTS
    NEXUS_AudioOutput_AddInput(
        NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]),
        NEXUS_AudioDecoder_GetConnector(s_app.audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
#endif
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_AudioOutput_AddInput(
        NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]),
        NEXUS_AudioDecoder_GetConnector(s_app.audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
#endif

    playback_mp4(s_app.videoDecoder, s_app.audioDecoder, argv[1]);

    if (shouldExit) {
        exit(2);
    } else {
        wait_for_drain();
        exit(EXIT_SUCCESS);
    }
}
