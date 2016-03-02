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

/* Nexus example app: Play Ready decrypt, PIFF parser, and PES conversion decode */

#define LOG_NDEBUG 0

#include "nexus_platform.h"
#include "nexus_video_decoder.h"
#include "nexus_stc_channel.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_video_input.h"
#include "nexus_spdif_output.h"
#include "nexus_component_output.h"
#include "nexus_video_adj.h"
#include "nexus_playback.h"
#include "nexus_core_utils.h"

#include "common_crypto.h"
#include "drm_prdy_http.h"
#include "drm_prdy.h"
#include "drm_prdy_types.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"
#include "bmp4_util.h"
#include "bbase64.h"
#include "piff_parser.h"
#include "bfile_stdio.h"
#include "bpiff.h"


#define DEBUG_OUTPUT_CAPTURE    0

#if USE_SVP
#include <sage_srai.h>
#endif

#define REPACK_VIDEO_PES_ID 0xE0
#define REPACK_AUDIO_PES_ID 0xC0

#define BOX_HEADER_SIZE (8)
#define BUF_SIZE (1024 * 1024 * 2) /* 2MB */

#define CALCULATE_PTS(t)        (((uint64_t)(t) / 10000LL) * 45LL)

BDBG_MODULE(playready_svp_single);

#include "bpiff.c"
#include "piff_parser.c"

typedef struct app_ctx {
    FILE *fp_piff;
    uint32_t piff_filesize;

    uint8_t *pPayload;
    uint8_t *pOutBuf;

    size_t outBufSize;

    uint64_t last_video_fragment_time;
    uint64_t last_audio_fragment_time;

    DRM_Prdy_DecryptContext_t decryptor;
} app_ctx;

typedef struct pthread_info {
    NEXUS_VideoDecoderHandle videoDecoder;
    int result;
} pthread_info;

#if DEBUG_OUTPUT_CAPTURE
/* Input file pointer */
FILE *fp_vid;
FILE *fp_aud;
#endif

/* **FixMe** put this here for now */
/* Enable NETFLIX_EXT only if USE_SVP=1*/
/*
#if USE_SVP
#define NETFLIX_EXT 1
#endif
*/

#ifdef NETFLIX_EXT
/* **FixMe** test code until IsSecureStopAPI is implemented */
static bool     bIsSecureStopEnabled = false;
static uint8_t  sSessionIdBuf[DRM_PRDY_SESSION_ID_LEN];
#endif

/* stream type */
int vc1_stream = 0;

/* Use secure buffers for playback if true
* Secure buffers only apply when SAGE enabled, always false otherwise.
* Should be FALSE for SAGE_SECURE_MODE=1, and TRUE for SAGE_SECURE_MODE 5/6/9 */
bool secure_pic_buffers = false;

typedef app_ctx * app_ctx_t;
static int video_decode_hdr;


static int piff_playback_dma_buffer(CommonCryptoHandle commonCryptoHandle, void *dst,
        void *src, size_t size, bool flush)
{
    NEXUS_DmaJobBlockSettings blkSettings;
    CommonCryptoJobSettings cryptoJobSettings;

    BDBG_MSG(("%s: from=%p, to=%p, size=%u", __FUNCTION__, src, dst, size));

    NEXUS_DmaJob_GetDefaultBlockSettings(&blkSettings);
    blkSettings.pSrcAddr = src;
    blkSettings.pDestAddr = dst;
    blkSettings.blockSize = size;
    blkSettings.resetCrypto = true;
    blkSettings.scatterGatherCryptoStart = true;
    blkSettings.scatterGatherCryptoEnd = true;

    if (flush)
    {
        /* Need to flush manually the source buffer (non secure heap). We need to flush manually as soon as we copy data into
           the secure heap. Setting blkSettings[ii].cached = true would also try to flush the destination address in the secure heap
           which is not accessible. This would cause the whole memory to be flushed at once. */
        NEXUS_FlushCache(blkSettings.pSrcAddr, blkSettings.blockSize);
        blkSettings.cached = false; /* Prevent the DMA from flushing the buffers later on */
    }

    CommonCrypto_GetDefaultJobSettings(&cryptoJobSettings);
    CommonCrypto_DmaXfer(commonCryptoHandle,  &cryptoJobSettings, &blkSettings, 1);

    if (flush)
    {
        /* Need to flush manually the source buffer (non secure heap). We need to flush manually as soon as we copy data into
           the secure heap. Setting blkSettings[ii].cached = true would also try to flush the destination address in the secure heap
           which is not accessible. This would cause the whole memory to be flushed at once. */
        NEXUS_FlushCache(blkSettings.pSrcAddr, blkSettings.blockSize);
    }

    return 0;
}

static int parse_esds_config(bmedia_adts_hdr *hdr, bmedia_info_aac *info_aac, size_t payload_size)
{
    bmedia_adts_header adts_header;

    bmedia_adts_header_init_aac(&adts_header, info_aac);
    bmedia_adts_hdr_init(hdr, &adts_header, payload_size);

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
        BKNI_Memcpy(dst, bpiff_nal, sizeof(bpiff_nal)); dst += sizeof(bpiff_nal);
        /* Add SPS */
        BKNI_Memcpy(dst, data, sps_len);
        dst += sps_len;
        data += sps_len;
        *hdr_len += (sizeof(bpiff_nal) + sps_len);
    }

    data = (uint8_t *)meta.pps.data;
    for (i = 0; i < meta.pps.no; i++)
    {
        pps_len = (((uint16_t)data[0]) <<8) | data[1];
        data += 2;
        /* Add NAL */
        BKNI_Memcpy(dst, bpiff_nal, sizeof(bpiff_nal));
        dst += sizeof(bpiff_nal);
        /* Add PPS */
        BKNI_Memcpy(dst, data, pps_len);
        dst += pps_len;
        data += pps_len;
        *hdr_len += (sizeof(bpiff_nal) + pps_len);
    }
    return 0;
}

#if USE_SVP
static void *check_buffer(void *threadInfo )
{
    int i;
    pthread_info * info = (pthread_info *) threadInfo;

    if(info == NULL) {
        return NULL;
    }

    for (i = 0; i < 20; i++)
    {
        NEXUS_VideoDecoderStatus status;
        NEXUS_VideoDecoder_GetStatus(info->videoDecoder, &status);
        BDBG_MSG(("Main - numDecoded = '%u',   numDecodeErrors = '%u',   ptsErrorCount = '%u'", status.numDecoded, status.numDecodeErrors, status.ptsErrorCount));
        if (status.numDecodeErrors)
        {
            BDBG_ERR(("failing with numDecodeErrors = '%u'", status.numDecodeErrors));
            info->result = -1;
            break;
        }
        BKNI_Sleep(1000);
    }
    info->result = 0;

    return (void*)info;
}
#endif

#if !USE_SVP
static int decrypt_sample(CommonCryptoHandle commonCryptoHandle,
        uint32_t sampleSize, batom_cursor * cursor,
        bpiff_drm_mp4_box_se_sample *pSample, uint32_t *bytes_processed,
        DRM_Prdy_DecryptContext_t *decryptor, uint32_t enc_flags,
        uint8_t *out, size_t decrypt_offset)
{
    uint32_t rc=0;
    uint8_t i=0;
    uint8_t *src;
    uint8_t *dst;
    DRM_Prdy_AES_CTR_Info_t     aesCtrInfo;

    *bytes_processed = 0;

    BKNI_Memcpy( &aesCtrInfo.qwInitializationVector,&pSample->iv[8],8);
    aesCtrInfo.qwBlockOffset = 0;
    aesCtrInfo.bByteOffset = 0;

    if(enc_flags & 0x000002) {
        uint64_t     qwOffset = 0;

        for(i = 0; i <  pSample->nbOfEntries; i++) {
            uint32_t num_clear = pSample->entries[i].bytesOfClearData;
            uint32_t num_enc = pSample->entries[i].bytesOfEncData;

            /* Skip over clear units by offset amount */
            BDBG_ASSERT(num_clear >= decrypt_offset);
            batom_cursor_skip((batom_cursor *)cursor, decrypt_offset);

            /* Add NAL header per entry */
            if (!vc1_stream) {
                BKNI_Memcpy(out, bpiff_nal, sizeof(bpiff_nal));
                out += sizeof(bpiff_nal);
            }

            src = (uint8_t *)cursor->cursor;
            dst = out;

            piff_playback_dma_buffer(commonCryptoHandle, dst, src,
                    (num_enc + num_clear - decrypt_offset), false);

            /* Skip over remaining clear units */
            out += (num_clear - decrypt_offset);
            batom_cursor_skip((batom_cursor *)cursor, (num_clear - decrypt_offset));

            aesCtrInfo.qwBlockOffset = qwOffset / 16 ;
            aesCtrInfo.bByteOffset = qwOffset % 16 ;

            BDBG_MSG(("%s:%d: DRM_Prdy_Reader_Decrypt(..., ..., %p, %u)",
                      __FUNCTION__, __LINE__, out, num_enc));

            if(DRM_Prdy_Reader_Decrypt(
                        decryptor,
                        &aesCtrInfo,
                        (uint8_t *)out,
                        num_enc ) != DRM_Prdy_ok) {
                BDBG_ERR(("%s Reader_Decrypt failed - %d", __FUNCTION__, __LINE__));
                rc = -1;
                goto ErrorExit;
            }

            out += num_enc;
            batom_cursor_skip((batom_cursor *)cursor,num_enc);
            qwOffset = num_enc;
            if (!vc1_stream)
                *bytes_processed  += (num_clear - decrypt_offset +
                        num_enc + sizeof(bpiff_nal));
            else
                *bytes_processed  += (num_clear - decrypt_offset + num_enc);

            if( *bytes_processed > sampleSize) {
                BDBG_WRN(("Wrong buffer size is detected while decrypting the ciphertext, bytes processed %d, sample size to decrypt %d",*bytes_processed,sampleSize));
                rc = -1;
                goto ErrorExit;
            }
        }
    } else {
            src = (uint8_t *)cursor->cursor;
            dst = out;

            piff_playback_dma_buffer(commonCryptoHandle, dst, src, sampleSize, false);

            BDBG_MSG(("%s:%d: DRM_Prdy_Reader_Decrypt(..., ..., %p, %u)",
                      __FUNCTION__, __LINE__, out, sampleSize));

            if(DRM_Prdy_Reader_Decrypt(
                        decryptor,
                        &aesCtrInfo,
                        (uint8_t *)out,
                        sampleSize ) != DRM_Prdy_ok) {
            BDBG_ERR(("%s Reader_Decrypt failed - %d", __FUNCTION__, __LINE__));
            rc = -1;
            goto ErrorExit;
        }

        out += (sampleSize);

        batom_cursor_skip((batom_cursor *)cursor, sampleSize);
        *bytes_processed = sampleSize;
    }

ErrorExit:
    return rc;
}
#endif

static int secure_process_fragment(CommonCryptoHandle commonCryptoHandle, app_ctx *app,
        piff_parse_frag_info *frag_info, size_t payload_size,
        void *decoder_data, size_t decoder_len,
        NEXUS_PlaypumpHandle playpump, BKNI_EventHandle event)
{
    int rc = 0;
    uint32_t i, j, bytes_processed;
    bpiff_drm_mp4_box_se_sample *pSample;
    uint8_t pes_header[BMEDIA_PES_HEADER_MAX_SIZE];
    size_t pes_header_len;
    bmedia_pes_info pes_info;
    uint64_t frag_duration;
    uint8_t *pOutBuf = app->pOutBuf;
    size_t decrypt_offset = 0;
    NEXUS_PlaypumpStatus playpumpStatus;
    size_t fragment_size = payload_size;
    void *playpumpBuffer;
    size_t bufferSize;
    size_t outSize = 0;
    uint8_t *out;
    DRM_Prdy_AES_CTR_Info_t aesCtrInfo;

    /* Obtain secure playpump buffer */
    NEXUS_Playpump_GetStatus(playpump, &playpumpStatus);
    fragment_size += 512;   /* Provide headroom for PES and SPS/PPS headers */
    BDBG_ASSERT(fragment_size <= playpumpStatus.fifoSize);
    for(;;) {
        NEXUS_Playpump_GetBuffer(playpump, &playpumpBuffer, &bufferSize);
        if(bufferSize >= fragment_size) {
            break;
        }
        if(bufferSize==0) {
            BKNI_WaitForEvent(event, 100);
            continue;
        }
        if((uint8_t *)playpumpBuffer >= (uint8_t *)playpumpStatus.bufferBase +
                (playpumpStatus.fifoSize - fragment_size)) {
            NEXUS_Playpump_WriteComplete(playpump, bufferSize, 0); /* skip buffer what wouldn't be big enough */
        }
    }

    BDBG_MSG(("%s: NEXUS_Playpump_GetBuffer return buffer %p, size %u",
              __FUNCTION__, playpumpBuffer, bufferSize));

    app->outBufSize = 0;
    bytes_processed = 0;
    if (frag_info->samples_enc->sample_count != 0) {
        /* Iterate through the samples within the fragment */
        for (i = 0; i < frag_info->samples_enc->sample_count; i++) {
            size_t numOfByteDecrypted = 0;
            size_t sampleSize = 0;

            pSample = &frag_info->samples_enc->samples[i];
            sampleSize = frag_info->sample_info[i].size;

            if (frag_info->trackType == BMP4_SAMPLE_ENCRYPTED_VIDEO) {
                if (!vc1_stream) {
                    /* H.264 Decoder configuration parsing */
                    uint8_t avcc_hdr[BPIFF_MAX_PPS_SPS];
                    size_t avcc_hdr_size;
                    size_t nalu_len = 0;

                    pOutBuf = app->pOutBuf;
                    app->outBufSize = 0;

                    parse_avcc_config(avcc_hdr, &avcc_hdr_size, &nalu_len, decoder_data, decoder_len);

                    bmedia_pes_info_init(&pes_info, REPACK_VIDEO_PES_ID);
                    frag_duration = app->last_video_fragment_time +
                        (int32_t)frag_info->sample_info[i].composition_time_offset;
                    app->last_video_fragment_time += frag_info->sample_info[i].duration;

                    pes_info.pts_valid = true;
                    pes_info.pts = CALCULATE_PTS(frag_duration);
                    if (video_decode_hdr == 0) {
                        pes_header_len = bmedia_pes_header_init(pes_header,
                                (sampleSize + avcc_hdr_size - nalu_len + sizeof(bpiff_nal)), &pes_info);
                    } else {
                        pes_header_len = bmedia_pes_header_init(pes_header,
                                (sampleSize - nalu_len + sizeof(bpiff_nal)), &pes_info);
                    }

                    /* Copy PES header and SPS/PPS to intermediate buffer */
                    BKNI_Memcpy(pOutBuf, &pes_header, pes_header_len);
                    pOutBuf += pes_header_len;
                    app->outBufSize += pes_header_len;

                    /* Only add header once */
                    if (video_decode_hdr == 0) {
                        BKNI_Memcpy(pOutBuf, avcc_hdr, avcc_hdr_size);
                        pOutBuf += avcc_hdr_size;
                        app->outBufSize += avcc_hdr_size;
                        video_decode_hdr = 1;
                    }

                    decrypt_offset = nalu_len;

                    piff_playback_dma_buffer(commonCryptoHandle, (uint8_t *)playpumpBuffer + outSize,
                            app->pOutBuf, app->outBufSize, true);
                    outSize += app->outBufSize;
                }
            } else if (frag_info->trackType == BMP4_SAMPLE_ENCRYPTED_AUDIO) {
                if (!vc1_stream) {
                    /* AAC information parsing */
                    bmedia_adts_hdr hdr;
                    bmedia_info_aac *info_aac = (bmedia_info_aac *)decoder_data;

                    pOutBuf = app->pOutBuf;
                    app->outBufSize = 0;

                    parse_esds_config(&hdr, info_aac, sampleSize);

                    bmedia_pes_info_init(&pes_info, REPACK_AUDIO_PES_ID);
                    frag_duration = app->last_audio_fragment_time +
                        (int32_t)frag_info->sample_info[i].composition_time_offset;
                    app->last_audio_fragment_time += frag_info->sample_info[i].duration;

                    pes_info.pts_valid = true;
                    pes_info.pts = CALCULATE_PTS(frag_duration);

                    pes_header_len = bmedia_pes_header_init(pes_header,
                            (sampleSize + BMEDIA_ADTS_HEADER_SIZE), &pes_info);
                    BKNI_Memcpy(pOutBuf, &pes_header, pes_header_len);
                    BKNI_Memcpy(pOutBuf + pes_header_len, &hdr.adts, BMEDIA_ADTS_HEADER_SIZE);

                    pOutBuf += pes_header_len + BMEDIA_ADTS_HEADER_SIZE;
                    app->outBufSize += pes_header_len + BMEDIA_ADTS_HEADER_SIZE;

                    decrypt_offset = 0;

                    piff_playback_dma_buffer(commonCryptoHandle, (uint8_t *)playpumpBuffer + outSize,
                            app->pOutBuf, app->outBufSize, true);
                    outSize += app->outBufSize;
                }
            } else {
                BDBG_WRN(("%s Unsupported track type %d detected", __FUNCTION__, frag_info->trackType));
                return -1;
            }

            /* Process and decrypt samples */

            BKNI_Memcpy( &aesCtrInfo.qwInitializationVector,&pSample->iv[8],8);
            aesCtrInfo.qwBlockOffset = 0;
            aesCtrInfo.bByteOffset = 0;
            out = (uint8_t *)playpumpBuffer + outSize;

            if(frag_info->samples_enc->flags & 0x000002) {
                uint64_t     qwOffset = 0;

                for(j = 0; j < pSample->nbOfEntries; j++) {
                    uint32_t num_clear = pSample->entries[j].bytesOfClearData;
                    uint32_t num_enc = pSample->entries[j].bytesOfEncData;
                    int blk_idx = 0;
                    int k;
                    NEXUS_DmaJobBlockSettings blkSettings[2];
                    CommonCryptoJobSettings cryptoJobSettings;

                    /* Skip over clear units by offset amount */
                    BDBG_ASSERT(num_clear >= decrypt_offset);
                    batom_cursor_skip((batom_cursor *)&frag_info->cursor, decrypt_offset);

                    if (!vc1_stream) {
                        /* Append NAL Header */
                        BKNI_Memcpy(app->pOutBuf, bpiff_nal, sizeof(bpiff_nal));
                        NEXUS_DmaJob_GetDefaultBlockSettings(&blkSettings[blk_idx]);
                        blkSettings[blk_idx].pSrcAddr = app->pOutBuf;
                        blkSettings[blk_idx].pDestAddr = out;
                        blkSettings[blk_idx].blockSize = sizeof(bpiff_nal);
                        blkSettings[blk_idx].resetCrypto = true;
                        blkSettings[blk_idx].scatterGatherCryptoStart = true;
                        blkSettings[blk_idx].scatterGatherCryptoEnd = false;
                        blk_idx++;

                        out += sizeof(bpiff_nal);
                        outSize += sizeof(bpiff_nal);
                    }

                    NEXUS_DmaJob_GetDefaultBlockSettings(&blkSettings[blk_idx]);
                    blkSettings[blk_idx].pSrcAddr = (uint8_t *)frag_info->cursor.cursor;
                    blkSettings[blk_idx].pDestAddr = out;
                    blkSettings[blk_idx].blockSize = (num_enc + num_clear - decrypt_offset);
                    blkSettings[blk_idx].resetCrypto = blk_idx ? false : true;
                    blkSettings[blk_idx].scatterGatherCryptoStart = blk_idx ? false : true;
                    blkSettings[blk_idx].scatterGatherCryptoEnd = true;
                    blk_idx++;

                    for (k = 0; k < blk_idx; k++ ) {
                        NEXUS_FlushCache(blkSettings[k].pSrcAddr, blkSettings[k].blockSize);
                        blkSettings[k].cached = false; /* Prevent the DMA from flushing the buffers later on */
                    }

                    CommonCrypto_GetDefaultJobSettings(&cryptoJobSettings);
                    CommonCrypto_DmaXfer(commonCryptoHandle,  &cryptoJobSettings, blkSettings, blk_idx);

                    for (k = 0; k < blk_idx; k++ ) {
                        NEXUS_FlushCache(blkSettings[k].pSrcAddr, blkSettings[k].blockSize);
                    }

                    outSize += (num_enc + num_clear - decrypt_offset);

                    /* Skip over remaining clear units */
                    out += (num_clear - decrypt_offset);
                    batom_cursor_skip((batom_cursor *)&frag_info->cursor, (num_clear - decrypt_offset));

                    aesCtrInfo.qwBlockOffset = qwOffset / 16 ;
                    aesCtrInfo.bByteOffset = qwOffset % 16 ;

                    BDBG_MSG(("%s:%d: DRM_Prdy_Reader_Decrypt(..., ..., %p, %u)",
                              __FUNCTION__, __LINE__, out, num_enc));
                    if(DRM_Prdy_Reader_Decrypt(
                                &app->decryptor,
                                &aesCtrInfo,
                                (uint8_t *)out,
                                num_enc ) != DRM_Prdy_ok) {
                        BDBG_ERR(("%s Reader_Decrypt failed - %d", __FUNCTION__, __LINE__));
                        return -1;
                    }

                    out += num_enc;
                    batom_cursor_skip((batom_cursor *)&frag_info->cursor,num_enc);
                    qwOffset = num_enc;
                    numOfByteDecrypted  += (num_clear - decrypt_offset + num_enc);

                    if(numOfByteDecrypted > sampleSize) {
                        BDBG_WRN(("Wrong buffer size is detected while decrypting the ciphertext, bytes processed %d, sample size to decrypt %d",
                                    numOfByteDecrypted, sampleSize));
                        return -1;
                    }
                }
            } else {
                piff_playback_dma_buffer(commonCryptoHandle, out, (uint8_t *)frag_info->cursor.cursor,
                        sampleSize, true);
                outSize += sampleSize;

                BDBG_MSG(("%s:%d: DRM_Prdy_Reader_Decrypt(..., ..., %p, %u)",
                          __FUNCTION__, __LINE__, out, sampleSize));
                if(DRM_Prdy_Reader_Decrypt(
                            &app->decryptor,
                            &aesCtrInfo,
                            (uint8_t *)out,
                            sampleSize ) != DRM_Prdy_ok) {
                    BDBG_ERR(("%s Reader_Decrypt failed - %d", __FUNCTION__, __LINE__));
                    return -1;
                }

                batom_cursor_skip((batom_cursor *)&frag_info->cursor, sampleSize);
                numOfByteDecrypted = sampleSize;
            }

            bytes_processed += numOfByteDecrypted + decrypt_offset;
        }
        BDBG_MSG(("%s: NEXUS_Playpump_WriteComplete buffer %p, size %u",
                  __FUNCTION__, playpumpBuffer, bufferSize));
        NEXUS_Playpump_WriteComplete(playpump, 0, outSize);
    }

    if(bytes_processed != payload_size) {
        BDBG_WRN(("%s the number of bytes %d decrypted doesn't match the actual size %d of the payload, return failure...%d",__FUNCTION__,
                    bytes_processed, payload_size, __LINE__));
        rc = -1;
    }

    return rc;
}

#if !USE_SVP
static int process_fragment(CommonCryptoHandle commonCryptoHandle, app_ctx *app, piff_parse_frag_info *frag_info,
        size_t payload_size, void *decoder_data, size_t decoder_len)
{
    int rc = 0;
    uint32_t i, bytes_processed;
    bpiff_drm_mp4_box_se_sample *pSample;
    uint8_t pes_header[BMEDIA_PES_HEADER_MAX_SIZE];
    size_t pes_header_len;
    bmedia_pes_info pes_info;
    uint64_t frag_duration;
    uint8_t *pOutBuf = app->pOutBuf;
    size_t decrypt_offset = 0;

    app->outBufSize = 0;
    bytes_processed = 0;
    if (frag_info->samples_enc->sample_count != 0) {
        for (i = 0; i < frag_info->samples_enc->sample_count; i++) {
            size_t numOfByteDecrypted = 0;
            size_t sampleSize = 0;

            pSample = &frag_info->samples_enc->samples[i];
            sampleSize = frag_info->sample_info[i].size;

            if (frag_info->trackType == BMP4_SAMPLE_ENCRYPTED_VIDEO) {
                if (!vc1_stream) {
                    /* H.264 Decoder configuration parsing */
                    uint8_t avcc_hdr[BPIFF_MAX_PPS_SPS];
                    size_t avcc_hdr_size;
                    size_t nalu_len = 0;

                    parse_avcc_config(avcc_hdr, &avcc_hdr_size, &nalu_len, decoder_data, decoder_len);

                    bmedia_pes_info_init(&pes_info, REPACK_VIDEO_PES_ID);
                    frag_duration = app->last_video_fragment_time +
                        (int32_t)frag_info->sample_info[i].composition_time_offset;
                    app->last_video_fragment_time += frag_info->sample_info[i].duration;

                    pes_info.pts_valid = true;
                    pes_info.pts = CALCULATE_PTS(frag_duration);
                    if (video_decode_hdr == 0) {
                        pes_header_len = bmedia_pes_header_init(pes_header,
                                (sampleSize + avcc_hdr_size - nalu_len + sizeof(bpiff_nal)), &pes_info);
                    } else {
                        pes_header_len = bmedia_pes_header_init(pes_header,
                                (sampleSize - nalu_len + sizeof(bpiff_nal)), &pes_info);
                    }

                    BKNI_Memcpy(pOutBuf, &pes_header, pes_header_len);
                    pOutBuf += pes_header_len;
                    app->outBufSize += pes_header_len;

                    if (video_decode_hdr == 0) {
                        BKNI_Memcpy(pOutBuf, avcc_hdr, avcc_hdr_size);
                        pOutBuf += avcc_hdr_size;
                        app->outBufSize += avcc_hdr_size;
                        video_decode_hdr = 1;
                    }

                    decrypt_offset = nalu_len;
                }
            } else if (frag_info->trackType == BMP4_SAMPLE_ENCRYPTED_AUDIO) {
                if (!vc1_stream) {
                    /* AAC information parsing */
                    bmedia_adts_hdr hdr;
                    bmedia_info_aac *info_aac = (bmedia_info_aac *)decoder_data;

                    parse_esds_config(&hdr, info_aac, sampleSize);

                    bmedia_pes_info_init(&pes_info, REPACK_AUDIO_PES_ID);
                    frag_duration = app->last_audio_fragment_time +
                        (int32_t)frag_info->sample_info[i].composition_time_offset;
                    app->last_audio_fragment_time += frag_info->sample_info[i].duration;

                    pes_info.pts_valid = true;
                    pes_info.pts = CALCULATE_PTS(frag_duration);

                    pes_header_len = bmedia_pes_header_init(pes_header,
                            (sampleSize + BMEDIA_ADTS_HEADER_SIZE), &pes_info);
                    BKNI_Memcpy(pOutBuf, &pes_header, pes_header_len);
                    BKNI_Memcpy(pOutBuf + pes_header_len, &hdr.adts, BMEDIA_ADTS_HEADER_SIZE);

                    pOutBuf += pes_header_len + BMEDIA_ADTS_HEADER_SIZE;
                    app->outBufSize += pes_header_len + BMEDIA_ADTS_HEADER_SIZE;

                    decrypt_offset = 0;
                }
            } else {
                BDBG_WRN(("%s Unsupported track type %d detected", __FUNCTION__, frag_info->trackType));
                return -1;
            }

            if(decrypt_sample(commonCryptoHandle, sampleSize, &frag_info->cursor, pSample, &numOfByteDecrypted,
                        &app->decryptor, frag_info->samples_enc->flags, pOutBuf, decrypt_offset) !=0) {
                BDBG_ERR(("%s Failed to decrypt sample, can't continue - %d", __FUNCTION__, __LINE__));
                return -1;
                break;
            }
            pOutBuf += numOfByteDecrypted;
            app->outBufSize += numOfByteDecrypted;
            bytes_processed += numOfByteDecrypted;
        }
    }

    if( bytes_processed != payload_size) {
        BDBG_WRN(("%s the number of bytes %d decrypted doesn't match the actual size %d of the payload, return failure...%d",__FUNCTION__,bytes_processed,payload_size, __LINE__));
        rc = -1;
    }

    return rc;
}
#endif

#if !USE_SVP
static int send_fragment_data(
        CommonCryptoHandle commonCryptoHandle,
        uint8_t *pData,
        uint32_t dataSize,
        NEXUS_PlaypumpHandle playpump,
        BKNI_EventHandle event)
{
    NEXUS_PlaypumpStatus playpumpStatus;
    size_t fragment_size = dataSize;
    void *playpumpBuffer;
    size_t bufferSize;
    NEXUS_Playpump_GetStatus(playpump, &playpumpStatus);
    fragment_size += 512;
    BDBG_ASSERT(fragment_size <= playpumpStatus.fifoSize);
    for(;;) {
        NEXUS_Playpump_GetBuffer(playpump, &playpumpBuffer, &bufferSize);
        if(bufferSize >= fragment_size) {
            break;
        }
        if(bufferSize==0) {
            BKNI_WaitForEvent(event, 100);
            continue;
        }
        if((uint8_t *)playpumpBuffer >= (uint8_t *)playpumpStatus.bufferBase +
                (playpumpStatus.fifoSize - fragment_size)) {
            NEXUS_Playpump_WriteComplete(playpump, bufferSize, 0); /* skip buffer what wouldn't be big enough */
        }
    }
    piff_playback_dma_buffer(commonCryptoHandle, playpumpBuffer, pData, dataSize, true);
    NEXUS_Playpump_WriteComplete(playpump, 0, dataSize);

    return 0;
}
#endif

static void play_callback(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
}

static void
wait_for_drain(NEXUS_PlaypumpHandle videoPlaypump, NEXUS_VideoDecoderHandle videoDecoder,
               NEXUS_PlaypumpHandle audioPlaypump, NEXUS_AudioDecoderHandle audioDecoder)
{
    NEXUS_Error rc;
    NEXUS_PlaypumpStatus playpumpStatus;

    for(;;) {
        rc=NEXUS_Playpump_GetStatus(videoPlaypump, &playpumpStatus);
        if(rc!=NEXUS_SUCCESS)
            break;

        if(playpumpStatus.fifoDepth==0) {
            break;
        }
        BKNI_Sleep(100);
    }
    for(;;) {
        rc=NEXUS_Playpump_GetStatus(audioPlaypump, &playpumpStatus);
        if(rc!=NEXUS_SUCCESS)
            break;

        if(playpumpStatus.fifoDepth==0)
            break;

        BKNI_Sleep(100);
    }

    if(videoDecoder) {
        for(;;) {
            NEXUS_VideoDecoderStatus decoderStatus;
            rc=NEXUS_VideoDecoder_GetStatus(videoDecoder, &decoderStatus);
            if(rc!=NEXUS_SUCCESS)
                break;

            if(decoderStatus.queueDepth==0)
                break;

            BKNI_Sleep(100);
        }
    }
    if(audioDecoder) {
        for(;;) {
            NEXUS_AudioDecoderStatus decoderStatus;
            rc=NEXUS_AudioDecoder_GetStatus(audioDecoder, &decoderStatus);
            if(rc!=NEXUS_SUCCESS)
                break;

            if(decoderStatus.queuedFrames < 4)
                break;

            BKNI_Sleep(100);
        }
    }
    return;
}

static int complete_play_fragments(
        NEXUS_AudioDecoderHandle audioDecoder,
        NEXUS_VideoDecoderHandle videoDecoder,
        NEXUS_PlaypumpHandle videoPlaypump,
        NEXUS_PlaypumpHandle audioPlaypump,
        NEXUS_DisplayHandle display,
        NEXUS_PidChannelHandle audioPidChannel,
        NEXUS_PidChannelHandle videoPidChannel,
        NEXUS_VideoWindowHandle window,
        BKNI_EventHandle event)
{
    BSTD_UNUSED(window);
    BSTD_UNUSED(display);
    BSTD_UNUSED(event);
    wait_for_drain(videoPlaypump, videoDecoder, audioPlaypump, audioDecoder);

    if (event != NULL) BKNI_DestroyEvent(event);

    if (videoDecoder) {
        NEXUS_VideoDecoder_Stop(videoDecoder);
        NEXUS_Playpump_ClosePidChannel(videoPlaypump, videoPidChannel);
        NEXUS_Playpump_Stop(videoPlaypump);
        NEXUS_StopCallbacks(videoPlaypump);
    }
    if (audioDecoder) {
        NEXUS_AudioDecoder_Stop(audioDecoder);
        NEXUS_Playpump_ClosePidChannel(audioPlaypump, audioPidChannel);
        NEXUS_Playpump_Stop(audioPlaypump);
        NEXUS_StopCallbacks(audioPlaypump);
    }

    NEXUS_Playpump_Close(videoPlaypump);
    NEXUS_Playpump_Close(audioPlaypump);

    return 0;
}

int playback_piff( NEXUS_VideoDecoderHandle videoDecoder,
                    NEXUS_AudioDecoderHandle audioDecoder,
                    DRM_Prdy_Handle_t drm_context,
                    char *piff_file)
{
#if USE_SVP
    uint8_t *pSecureVideoHeapBuffer = NULL;
    uint8_t *pSecureAudioHeapBuffer = NULL;
    pthread_t pthread;
    pthread_info *info;
    int finalResult = -1;
#endif
    NEXUS_MemoryAllocationSettings memSettings;
    NEXUS_StcChannelHandle stcChannel;
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_AudioDecoderStartSettings audioProgram;
    NEXUS_PlaypumpOpenSettings videoplaypumpOpenSettings;
    NEXUS_PlaypumpOpenSettings audioplaypumpOpenSettings;
    NEXUS_PlaypumpSettings playpumpSettings;
    NEXUS_PlaypumpOpenPidChannelSettings video_pid_settings;
    NEXUS_Error rc;
    CommonCryptoHandle commonCryptoHandle = NULL;
    CommonCryptoSettings  cmnCryptoSettings;

    NEXUS_DisplayHandle display = NULL;
    NEXUS_PidChannelHandle videoPidChannel = NULL;
    NEXUS_PidChannelHandle audioPidChannel = NULL;
    BKNI_EventHandle event = NULL;
    NEXUS_PlaypumpHandle videoPlaypump = NULL;
    NEXUS_PlaypumpHandle audioPlaypump = NULL;


    app_ctx app;
    bool moovBoxParsed = false;

    uint8_t resp_buffer[64*1024];
    char *pCh_url = NULL;
    char *pCh_data = NULL;
    uint8_t *pResponse = resp_buffer;
    size_t respLen;
    size_t respOffset;
    size_t urlLen;
    size_t chLen;
    piff_parser_handle_t piff_handle;
    bfile_io_read_t fd;
    uint8_t *pssh_data;
    uint32_t pssh_len;

    BDBG_MSG(("%s - %d\n", __FUNCTION__, __LINE__));
    if(piff_file == NULL ) {
        goto clean_exit;
    }

#if USE_SVP
    info = BKNI_Malloc(sizeof(pthread_info));
#endif

    BDBG_MSG(("PIFF file: %s\n",piff_file));
    fflush(stdout);

    BKNI_Memset(&app, 0, sizeof( app_ctx));
    app.last_video_fragment_time = 0;
    app.last_audio_fragment_time = 0;

    app.fp_piff = fopen(piff_file, "rb");
    if(app.fp_piff == NULL){
        fprintf(stderr,"failed to open %s\n", piff_file);
        goto clean_exit;
    }

    fd = bfile_stdio_read_attach(app.fp_piff);

    piff_handle = piff_parser_create(fd);
    if (!piff_handle) {
        BDBG_ERR(("Unable to create PIFF parser context"));
        goto clean_exit;
    }

    fseek(app.fp_piff, 0, SEEK_END);
    app.piff_filesize = ftell(app.fp_piff);
    fseek(app.fp_piff, 0, SEEK_SET);

    if( drm_context == NULL)
    {
       BDBG_ERR(("drm_context is NULL, quitting...."));
       goto clean_exit ;
    }

    CommonCrypto_GetDefaultSettings(&cmnCryptoSettings);
    commonCryptoHandle = CommonCrypto_Open(&cmnCryptoSettings);

    NEXUS_Memory_GetDefaultAllocationSettings(&memSettings);
    memSettings.heap = NEXUS_MEMC0_MAIN_HEAP;

    app.pPayload = NULL;
    if( NEXUS_Memory_Allocate(BUF_SIZE, &memSettings, (void *)&app.pPayload) !=  NEXUS_SUCCESS) {
        fprintf(stderr,"NEXUS_Memory_Allocate failed");
        goto clean_up;
    }

    app.pOutBuf = NULL;
    if( NEXUS_Memory_Allocate(BUF_SIZE, &memSettings, (void *)&app.pOutBuf) !=  NEXUS_SUCCESS) {
        fprintf(stderr,"NEXUS_Memory_Allocate failed");
        goto clean_up;
    }

    /* Perform parsing of the movie information */
    moovBoxParsed = piff_parser_scan_movie_info(piff_handle);

    if(!moovBoxParsed) {
        BDBG_ERR(("Failed to parse moov box, can't continue..."));
        goto clean_up;
    }

    BDBG_MSG(("Successfully parsed the moov box, continue...\n\n"));

    /* EXTRACT AND PLAYBACK THE MDAT */

    NEXUS_Playpump_GetDefaultOpenSettings(&videoplaypumpOpenSettings);
    videoplaypumpOpenSettings.fifoSize *= 7;
    videoplaypumpOpenSettings.numDescriptors *= 7;
#if USE_SVP
    info->videoDecoder = videoDecoder;
    videoplaypumpOpenSettings.dataNotCpuAccessible = true;
    pSecureVideoHeapBuffer = SRAI_Memory_Allocate(videoplaypumpOpenSettings.fifoSize,
            SRAI_MemoryType_SagePrivate);
    if ( pSecureVideoHeapBuffer == NULL ) {
        BDBG_ERR((" Failed to allocate from Secure Video heap"));
        BDBG_ASSERT( false );
    }
    videoplaypumpOpenSettings.memory = NEXUS_MemoryBlock_FromAddress(pSecureVideoHeapBuffer);
#else
    videoplaypumpOpenSettings.heap = NEXUS_MEMC0_MAIN_HEAP;
    videoplaypumpOpenSettings.boundsHeap = NEXUS_MEMC0_MAIN_HEAP;
#endif

    videoPlaypump = NEXUS_Playpump_Open(NEXUS_ANY_ID, &videoplaypumpOpenSettings);
    if (!videoPlaypump) {
        BDBG_ERR(("@@@ Video Playpump Open FAILED----"));
        goto clean_up;
    }
    BDBG_ASSERT(videoPlaypump != NULL);

    NEXUS_Playpump_GetDefaultOpenSettings(&audioplaypumpOpenSettings);
#if USE_SVP
    audioplaypumpOpenSettings.dataNotCpuAccessible = true;
    pSecureAudioHeapBuffer = SRAI_Memory_Allocate(audioplaypumpOpenSettings.fifoSize,
            SRAI_MemoryType_SagePrivate);
    if ( pSecureAudioHeapBuffer == NULL ) {
        BDBG_ERR((" Failed to allocate from Secure Audio heap"));
        goto clean_up;
    }
    BDBG_ASSERT( pSecureAudioHeapBuffer != NULL );
    audioplaypumpOpenSettings.memory = NEXUS_MemoryBlock_FromAddress(pSecureAudioHeapBuffer);
#else
    audioplaypumpOpenSettings.heap = NEXUS_MEMC0_MAIN_HEAP;
    audioplaypumpOpenSettings.boundsHeap = NEXUS_MEMC0_MAIN_HEAP;
#endif
    audioPlaypump = NEXUS_Playpump_Open(NEXUS_ANY_ID, &audioplaypumpOpenSettings);
    if (!audioPlaypump) {
        BDBG_ERR(("@@@ Video Playpump Open FAILED----"));
        goto clean_up;
    }
    BDBG_ASSERT(audioPlaypump != NULL);

    NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;
    stcSettings.mode = NEXUS_StcChannelMode_eAuto;
    stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);

    BKNI_CreateEvent(&event);

    NEXUS_Playpump_GetSettings(videoPlaypump, &playpumpSettings);
    playpumpSettings.dataCallback.callback = play_callback;
    playpumpSettings.dataCallback.context = event;
    playpumpSettings.transportType = NEXUS_TransportType_eMpeg2Pes;
    NEXUS_Playpump_SetSettings(videoPlaypump, &playpumpSettings);

    NEXUS_Playpump_GetSettings(audioPlaypump, &playpumpSettings);
    playpumpSettings.dataCallback.callback = play_callback;
    playpumpSettings.dataCallback.context = event;
    playpumpSettings.transportType = NEXUS_TransportType_eMpeg2Pes;
    NEXUS_Playpump_SetSettings(audioPlaypump, &playpumpSettings);

    NEXUS_Playpump_Start(videoPlaypump);
    NEXUS_Playpump_Start(audioPlaypump);

    NEXUS_Playpump_GetDefaultOpenPidChannelSettings(&video_pid_settings);
    video_pid_settings.pidType = NEXUS_PidType_eVideo;

    videoPidChannel = NEXUS_Playpump_OpenPidChannel(videoPlaypump, REPACK_VIDEO_PES_ID, &video_pid_settings);
#if USE_SVP
    NEXUS_SetPidChannelBypassKeyslot(videoPidChannel, NEXUS_BypassKeySlot_eGR2R);
#endif
    if ( !videoPidChannel )
      BDBG_WRN(("@@@ videoPidChannel NULL"));
    else
      BDBG_WRN(("@@@ videoPidChannel OK"));

    audioPidChannel = NEXUS_Playpump_OpenPidChannel(audioPlaypump, REPACK_AUDIO_PES_ID, NULL);
#if USE_SVP
    NEXUS_SetPidChannelBypassKeyslot(audioPidChannel, NEXUS_BypassKeySlot_eGR2R);
#endif

    if ( !audioPidChannel )
      BDBG_WRN(("@@@ audioPidChannel NULL"));
    else
      BDBG_WRN(("@@@ audioPidChannel OK"));

    /* already connected in main */
    NEXUS_AudioDecoder_GetDefaultStartSettings(&audioProgram);
    NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);

    if ( vc1_stream ) {
       BDBG_MSG(("@@@ set video audio program for vc1"));
       videoProgram.codec = NEXUS_VideoCodec_eVc1;
       audioProgram.codec = NEXUS_AudioCodec_eWmaPro;
    } else {
       BDBG_MSG(("@@@ set video audio program for h264"));
       videoProgram.codec = NEXUS_VideoCodec_eH264;
       audioProgram.codec = NEXUS_AudioCodec_eAacAdts;
    }

    videoProgram.pidChannel = videoPidChannel;
    videoProgram.stcChannel = stcChannel;
    NEXUS_VideoDecoder_Start(videoDecoder, &videoProgram);

    audioProgram.pidChannel = audioPidChannel;
    audioProgram.stcChannel = stcChannel;
    NEXUS_AudioDecoder_Start(audioDecoder, &audioProgram);


    /***********************
     * now ready to decrypt
     ***********************/
    pssh_data = piff_parser_get_pssh(piff_handle, &pssh_len);
    if (!pssh_data) {
        BDBG_ERR(("Failed to obtain pssh data"));
        goto clean_exit;
    }

    if( DRM_Prdy_Content_SetProperty(drm_context,
                DRM_Prdy_contentSetProperty_eAutoDetectHeader,
                pssh_data, pssh_len) != DRM_Prdy_ok) {
        BDBG_ERR(("Failed to SetProperty for the KID, exiting..."));
        goto clean_exit;
    }

#ifdef NETFLIX_EXT
	if (bIsSecureStopEnabled == true) {
        if( DRM_Prdy_Get_Buffer_Size( drm_context, DRM_Prdy_getBuffer_licenseAcq_challenge_Netflix,
                    NULL, 0, &urlLen, &chLen) != DRM_Prdy_ok ) {
            BDBG_ERR(("DRM_Prdy_Get_Buffer_Size() failed, exiting"));
            goto clean_exit;
        }
    } else {
        if( DRM_Prdy_Get_Buffer_Size( drm_context, DRM_Prdy_getBuffer_licenseAcq_challenge,
                    NULL, 0, &urlLen, &chLen) != DRM_Prdy_ok ) {
            BDBG_ERR(("DRM_Prdy_Get_Buffer_Size() failed, exiting"));
            goto clean_exit;
        }
    }
#else
    if( DRM_Prdy_Get_Buffer_Size( drm_context, DRM_Prdy_getBuffer_licenseAcq_challenge,
                NULL, 0, &urlLen, &chLen) != DRM_Prdy_ok ) {
        BDBG_ERR(("DRM_Prdy_Get_Buffer_Size() failed, exiting"));
        goto clean_exit;
    }
#endif

    pCh_url = BKNI_Malloc(urlLen);

    if(pCh_url == NULL) {
        BDBG_ERR(("BKNI_Malloc(urlent) failed, exiting..."));
        goto clean_exit;
    }

    pCh_data = BKNI_Malloc(chLen);
    if(pCh_data == NULL) {
        BDBG_ERR(("BKNI_Malloc(chLen) failed, exiting..."));
        goto clean_exit;
    }

#ifdef NETFLIX_EXT
	if (bIsSecureStopEnabled == true) {
        uint8_t    tNounce[DRM_PRDY_SESSION_ID_LEN];
        if(DRM_Prdy_LicenseAcq_GenerateChallenge_Netflix(drm_context, NULL,
                    0, pCh_url, &urlLen, pCh_data, &chLen, tNounce, false) != DRM_Prdy_ok ) {
            BDBG_ERR(("DRM_Prdy_LicenseAcq_GenerateChallenge_Netflix() failed, exiting"));
            goto clean_exit;
        }
	} else {
        if(DRM_Prdy_LicenseAcq_GenerateChallenge(drm_context, NULL,
                    0, pCh_url, &urlLen, pCh_data, &chLen) != DRM_Prdy_ok ) {
            BDBG_ERR(("DRM_Prdy_License_GenerateChallenge() failed, exiting"));
            goto clean_exit;
        }
	}
#else
    if(DRM_Prdy_LicenseAcq_GenerateChallenge(drm_context, NULL,
                0, pCh_url, &urlLen, pCh_data, &chLen) != DRM_Prdy_ok ) {
        BDBG_ERR(("DRM_Prdy_License_GenerateChallenge() failed, exiting"));
        goto clean_exit;
    }
#endif

    pCh_data[ chLen ] = 0;

    if(DRM_Prdy_http_client_license_post_soap(pCh_url, pCh_data, 1,
        150, (unsigned char **)&pResponse,64*1024, &respOffset, &respLen) != 0) {
        BDBG_ERR(("DRM_Prdy_http_client_license_post_soap() failed, exiting"));
        goto clean_exit;
    }

#ifdef NETFLIX_EXT
    if (bIsSecureStopEnabled) {
        if( DRM_Prdy_LicenseAcq_ProcessResponse_SecStop( drm_context,
                                                 (char *)&pResponse[respOffset],
                                                 respLen, sSessionIdBuf, NULL) != DRM_Prdy_ok )
        {
            BDBG_ERR(("DRM_Prdy_LicenseAcq_ProcessResponse_SecStop() failed with SessionID buffer, exiting"));
            goto clean_exit;
        }
    }
    else {
        if( DRM_Prdy_LicenseAcq_ProcessResponse(drm_context, (char *)&pResponse[respOffset],
                    respLen, NULL) != DRM_Prdy_ok ) {
            BDBG_ERR(("DRM_Prdy_LicenseAcq_ProcessResponse() failed, exiting"));
            goto clean_exit;
        }
    }
#else
    if( DRM_Prdy_LicenseAcq_ProcessResponse(drm_context, (char *)&pResponse[respOffset],
                respLen, NULL) != DRM_Prdy_ok ) {
        BDBG_ERR(("DRM_Prdy_LicenseAcq_ProcessResponse() failed, exiting"));
        goto clean_exit;
    }
#endif

#ifdef NETFLIX_EXT
    if( DRM_Prdy_Reader_Bind_Netflix( drm_context, sSessionIdBuf,
                &app.decryptor)!= DRM_Prdy_ok ) {
        BDBG_ERR(("Failed to Bind the license, the license may not exist. Exiting..."));
        goto clean_exit;
    }
#else
    if( DRM_Prdy_Reader_Bind( drm_context,
                &app.decryptor)!= DRM_Prdy_ok ) {
        BDBG_ERR(("Failed to Bind the license, the license may not exist. Exiting..."));
        goto clean_exit;
    }
#endif
    if( DRM_Prdy_Reader_Commit(drm_context) != DRM_Prdy_ok ) {
        BDBG_ERR(("Failed to Commit the license after Reader_Bind, exiting..."));
        goto clean_exit;
    }

    /* now go back to the begining and get all the moof boxes */
    fseek(app.fp_piff, 0, SEEK_END);
    app.piff_filesize = ftell(app.fp_piff);
    fseek(app.fp_piff, 0, SEEK_SET);

    video_decode_hdr = 0;
#if USE_SVP
    rc = pthread_create(&pthread, NULL, check_buffer, (void *)info);
    if (rc)
    {
        BDBG_ERR(("return code from pthread_create() is %d\n", rc));
        goto clean_up;
    }
#endif

    /* Start parsing the the file to look for MOOFs and MDATs */
    while(!feof(app.fp_piff))
    {
        piff_parse_frag_info frag_info;
        void *decoder_data;
        size_t decoder_len;

        if (!piff_parser_scan_movie_fragment(piff_handle, &frag_info, app.pPayload, BUF_SIZE)) {
            if (feof(app.fp_piff)) {
                BDBG_WRN(("Reached EOF"));
                break;
            } else {
                BDBG_ERR(("Unable to parse movie fragment"));
                goto clean_up;
            }
        }
        decoder_data = piff_parser_get_dec_data(piff_handle, &decoder_len, frag_info.trackId);

#if USE_SVP
        if (frag_info.trackType == BMP4_SAMPLE_ENCRYPTED_VIDEO) {
            secure_process_fragment(commonCryptoHandle, &app, &frag_info, (frag_info.mdat_size - BOX_HEADER_SIZE),
                    decoder_data, decoder_len, videoPlaypump, event);

        } else if (frag_info.trackType == BMP4_SAMPLE_ENCRYPTED_AUDIO) {
            secure_process_fragment(commonCryptoHandle, &app, &frag_info, (frag_info.mdat_size - BOX_HEADER_SIZE),
                    decoder_data, decoder_len, audioPlaypump, event);
        }
#else
        if(process_fragment(commonCryptoHandle, &app, &frag_info, (frag_info.mdat_size - BOX_HEADER_SIZE),
                    decoder_data, decoder_len) == 0) {
            if (frag_info.trackType == BMP4_SAMPLE_ENCRYPTED_VIDEO) {
#if DEBUG_OUTPUT_CAPTURE
                fwrite(app.pOutBuf, 1, app.outBufSize, fp_vid);
#endif
                send_fragment_data(commonCryptoHandle, app.pOutBuf, app.outBufSize,
                        videoPlaypump, event);

            } else if (frag_info.trackType == BMP4_SAMPLE_ENCRYPTED_AUDIO) {
#if DEBUG_OUTPUT_CAPTURE
                fwrite(app.pOutBuf, 1, app.outBufSize, fp_aud);
#endif
                send_fragment_data(commonCryptoHandle, app.pOutBuf, app.outBufSize,
                        audioPlaypump, event);
            }
        }
#endif /* USE_SVP */
    } /* while */

    complete_play_fragments(audioDecoder, videoDecoder, videoPlaypump,
            audioPlaypump, display, audioPidChannel, videoPidChannel, NULL, event);

#if USE_SVP
    if(pthread_join(pthread, (void**) &info) != 0)
    {
        BDBG_ERR(("ERROR IN PTHREAD_JOIN"));
        goto clean_up;
    } else {
        finalResult = info->result;
    }
    if (info->result == 0) {
        BDBG_LOG(("Success!"));
    } else {
        BDBG_ERR(("ERROR: thread failed"));
    }
#endif

clean_up:
    if(commonCryptoHandle) CommonCrypto_Close(commonCryptoHandle);
#if USE_SVP
    if(pSecureVideoHeapBuffer) SRAI_Memory_Free(pSecureVideoHeapBuffer);
    if(pSecureAudioHeapBuffer) SRAI_Memory_Free(pSecureAudioHeapBuffer);
    if(info != NULL) BKNI_Free(info);
#endif
    if(app.decryptor.pDecrypt != NULL) DRM_Prdy_Reader_Close( &app.decryptor);
    if(app.pPayload) NEXUS_Memory_Free(app.pPayload);
    if(app.pOutBuf) NEXUS_Memory_Free(app.pOutBuf);

    if(piff_handle != NULL) piff_parser_destroy(piff_handle);

    bfile_stdio_read_detach(fd);
    if(app.fp_piff) fclose(app.fp_piff);

    if(pCh_url != NULL) BKNI_Free(pCh_url);
    if(pCh_data != NULL) BKNI_Free(pCh_data);

clean_exit:
#if USE_SVP
    return finalResult;
#else
    return 0;
#endif
}

#ifdef NETFLIX_EXT
#define MAX_TIME_CHALLENGE_RESPONSE_LENGTH (1024*5)
#define MAX_URL_LENGTH (512)
int initSecureClock( DRM_Prdy_Handle_t drm)
{
    int                   rc = 0;
    uint8_t              *timeChResp = NULL;
    char                 *timeChURL = NULL;
    wchar_t              *pTimeURL=NULL;
    uint8_t              *timeCh_data=NULL;
    char                  timeURL_cstr[256] = {0};
    uint32_t              timeURL_len=0;
    uint32_t              timeCh_len=0;
    bool                  redirect = true;
    int32_t               petRC=0;
    uint32_t              petRespCode = 0;
    uint32_t              startOffset;
    uint32_t              length;
    uint32_t              post_ret;
    NEXUS_MemoryAllocationSettings allocSettings;

    if( DRM_Prdy_SecureClock_GenerateChallenge(drm,NULL,&timeURL_len,NULL,&timeCh_len) != DRM_Prdy_buffer_size)
    {
       BDBG_ERR(("DRM_Prdy_SecureClock_GenerateChallenge Failed, quitting....\n",__LINE__));
       return 0;
    }

    /*printf("%d DRM_Prdy_SecureClock_GenerateChallenge succeeded.\n",__LINE__); */
    /*printf("\tTime Server URL length %d, challenge len %d\n",timeURL_len,timeCh_len);*/

    rc = NEXUS_Memory_Allocate(timeURL_len*sizeof(wchar_t), NULL, (void **)(&pTimeURL));
    if(rc != NEXUS_SUCCESS)
    {
        BDBG_ERR(("%d NEXUS_Memory_Allocate failed for the Time server URL buffer, rc = %d\n",__LINE__, rc));
        goto clean_exit;
    }

    rc = NEXUS_Memory_Allocate(timeCh_len+1, NULL, (void **)(&timeCh_data));
    if(rc != NEXUS_SUCCESS)
    {
        BDBG_ERR(("NEXUS_Memory_Allocate failed for the time challenge buffer, rc = %d\n",__LINE__, rc));
        goto clean_exit;
    }

    BKNI_Memset(pTimeURL, 0, timeURL_len);
    BKNI_Memset(timeCh_data, 0, timeCh_len+1);

    if( DRM_Prdy_SecureClock_GenerateChallenge(drm,pTimeURL,&timeURL_len,timeCh_data,&timeCh_len) != DRM_Prdy_ok)
    {
        BDBG_ERR(("%d DRM_Prdy_SecureClock_GenerateChallenge failed.\n",__LINE__));
        goto clean_exit;
    }

    timeURL_cstr[timeURL_len] = 0; /* null terminator */
    if( !DRM_Prdy_convertWStringToCString( pTimeURL, timeURL_cstr, timeURL_len))
    {
        BDBG_ERR(("%d DRM_Prdy_convertWStringToCString failed to convert URL from wchar to char *, can't procceed time challenge.\n",__LINE__));
        goto clean_exit;
    }

    /*   printf("\tTime challenge petition server URL: %s\n",timeURL_cstr);*/

    timeCh_data[timeCh_len] = 0;

    NEXUS_Memory_GetDefaultAllocationSettings(&allocSettings);
    rc = NEXUS_Memory_Allocate(MAX_URL_LENGTH, &allocSettings, (void **)(&timeChURL ));
    if(rc != NEXUS_SUCCESS)
    {
        BDBG_ERR(("NEXUS_Memory_Allocate failed for time challenge response buffer, rc = %d\n",__LINE__, rc));
        goto clean_exit;
    }

    do
    {
           redirect = false;

           /* send the petition request to Microsoft with HTTP GET */
           petRC = DRM_Prdy_http_client_get_petition( timeURL_cstr,
                                                      &petRespCode,
                                                      (char**)&timeChURL);
           if( petRC != 0)
           {
               BDBG_ERR(("%d Secure Clock Petition request failed, rc = %d\n",__LINE__, petRC));
               rc = petRC;
               goto clean_exit;
           }

           /*printf("[PASSED] - %d Time petition request sent successfully, rc = %d, with RespCode = %d\n",__LINE__,petRC,petRespCode);*/

           /* we need to check if the Pettion responded with redirection */

           if( petRespCode == 200)
           {
               redirect = false;
           }
           else if( petRespCode == 302 || petRespCode == 301)
           {
               char * isHttps = NULL;

               /*printf("\tPetition responded with redirection. The new redirect petition URL: \n\t%s\n",timeChURL);*/

               redirect = true;

               memset(timeURL_cstr,0,timeURL_len);

               /* check if the URL is "https" */
               isHttps = strstr(timeChURL,"https");
               if( isHttps )
               {
                   strcpy(timeURL_cstr,"http");
                   strcpy(timeURL_cstr+4,isHttps+5);
               }
               else
               {
                   strcpy(timeURL_cstr,timeChURL);
               }

               memset(timeChURL,0,MAX_URL_LENGTH);
               /*printf("\tRe-sending petition to URL: %s\n",timeURL_cstr);*/
           }
           else
           {
               BDBG_ERR(("%d Secure Clock Petition responded with unsupported result, rc = %d, can't get the tiem challenge URL\n",__LINE__, petRespCode));
               rc = -1;
               goto clean_exit;
           }
    } while (redirect);

    /*printf("[PASSED] - %d Petition requests succeeded. The responded time challage server URL:\n\t%s\n",__LINE__,timeChURL);*/

    NEXUS_Memory_GetDefaultAllocationSettings(&allocSettings);
    rc = NEXUS_Memory_Allocate(MAX_TIME_CHALLENGE_RESPONSE_LENGTH, &allocSettings, (void **)(&timeChResp ));
    if(rc != NEXUS_SUCCESS)
    {
        BDBG_ERR(("%d NEXUS_Memory_Allocate failed for time challenge response buffer, rc = %d\n",__LINE__, rc));
        goto clean_exit;
    }

    BKNI_Memset(timeChResp, 0, MAX_TIME_CHALLENGE_RESPONSE_LENGTH);
    post_ret = DRM_Prdy_http_client_time_challenge_post(timeChURL,
                                                        (char *)timeCh_data,
                                                        1,
                                                        150,
                                                        (unsigned char**)&(timeChResp),
							1024*5,
                                                        &startOffset,
                                                        &length);
    if( post_ret != 0)
    {
        BDBG_ERR(("%d Secure Clock Challenge request failed, rc = %d\n",__LINE__, post_ret));
        rc = post_ret;
        goto clean_exit;
    }

    /*printf("[PASSED] - %d DRM_Prdy_http_client_time_challenge_post succeeded for Secure Clock Challenge with response size = %d.\n",__LINE__,length); */

    if( DRM_Prdy_SecureClock_ProcessResponse( drm, (uint8_t *) timeChResp, length) != DRM_Prdy_ok)
    {
        BDBG_ERR(("%d Secure Clock Process Challenge response failed\n",__LINE__));
        rc = -1;
        goto clean_exit;
    }

    BDBG_LOG(("%d Initialized Playready Secure Clock success.",__LINE__));

    /* NOW testing the system time */

clean_exit:

    if( timeChResp   != NULL)
        NEXUS_Memory_Free(timeChResp );

    if( timeChURL    != NULL)
        NEXUS_Memory_Free(timeChURL  );

    if( pTimeURL  != NULL)
        NEXUS_Memory_Free(pTimeURL );

    if( timeCh_data  != NULL)
        NEXUS_Memory_Free(timeCh_data );

    return rc;
}
#endif

void print_usage(char *program)
{
    BDBG_ERR(("%s usage:", program));
    BDBG_ERR(("\t %s <input file> [option(s)]", program));
    BDBG_ERR(("\t -vc1     stream is vc1"));
#if USE_SVP
    BDBG_ERR(("\t -secure  Use secure picture buffers (SAGE_SECURE_MODE != 1)"));
#endif
}

int main(int argc, char* argv[])
{
    NEXUS_Error rc;
    int ret = -1;
    NEXUS_DisplayHandle display;
    NEXUS_VideoWindowHandle window;
    NEXUS_VideoDecoderHandle videoDecoder;
    NEXUS_AudioDecoderHandle audioDecoder;
    NEXUS_VideoDecoderOpenSettings videoDecoderOpenSettings;
    NEXUS_AudioDecoderOpenSettings audioDecoderOpenSettings;
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_MemoryConfigurationSettings memConfigSettings;
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputStatus hdmiStatus;
    NEXUS_DisplaySettings displaySettings;
#endif

    /* DRM_Prdy specific */
    DRM_Prdy_Init_t     prdyParamSettings;
    DRM_Prdy_Handle_t   drm_context;

    uint32_t            secClkStatus; /* secure clock status */

    if (argc < 2) {
        print_usage(argv[0]);
        return -1;
    }

    while(argc>2)
    {
        argc--;
        if(strcmp(argv[argc], "-vc1") == 0)
        {
            vc1_stream = 1;
        }
#if USE_SVP
        /* Secure picture buffers only apply w/ SAGE enabled... only allow
        * setting this w/ SAGE */
        else if(strcmp(argv[argc], "-secure") == 0)
        {
            secure_pic_buffers = true;
        }
#endif
        else
        {
            BDBG_ERR(("Unrecognized option: %s", argv[argc]));
        }
    }

    BDBG_MSG(("@@@ MSG Check Point Start: vc1_stream=%d --  URR=%d",
              vc1_stream, secure_pic_buffers));

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    NEXUS_GetDefaultMemoryConfigurationSettings(&memConfigSettings);
#if USE_SVP
    if (secure_pic_buffers)
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
#endif
    platformSettings.openFrontend = false;
    rc = NEXUS_Platform_MemConfigInit(&platformSettings, &memConfigSettings);
    if (rc) return -1;
    NEXUS_Platform_GetConfiguration(&platformConfig);


    DRM_Prdy_GetDefaultParamSettings(&prdyParamSettings);

    drm_context =  DRM_Prdy_Initialize( &prdyParamSettings);
    if( drm_context == NULL) {
       BDBG_ERR(("Failed to create drm_context, quitting...."));
       goto clean_exit;
    }

#ifdef NETFLIX_EXT
    /* Getting the current state of the secure clock*/
    if( DRM_Prdy_SecureClock_GetStatus(drm_context,&secClkStatus) !=  DRM_Prdy_ok)
    {
       BDBG_ERR(("%d Failed to get the Playready Secure Clock status, quitting....\n",__LINE__));
       goto clean_exit;
    }

    if( secClkStatus != DRM_PRDY_CLK_SET)
    {
       /* setup the Playready secure clock */
       if(initSecureClock(drm_context) != 0)
       {
           BDBG_ERR(("%d Failed to initiize Secure Clock, quitting....\n",__LINE__));
           goto clean_exit;
       }
    }

    /* enable secure stop */
    if ( DRM_Prdy_TurnSecureStop(drm_context, 1) ) {
        printf("[FAILED] - %d Failed to enable Secure Stop \n",__LINE__);
    }
    else{
        bIsSecureStopEnabled = true;
    }
#endif

#if DEBUG_OUTPUT_CAPTURE
    fp_vid = fopen ("./video.out", "wb");
    fp_aud = fopen ("./audio.out", "wb");
#endif
    BDBG_MSG(("@@@ Check Point #01"));


    /* Bring up video display and outputs */
    display = NEXUS_Display_Open(0, NULL);
    window = NEXUS_VideoWindow_Open(display, 0);

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

    /* bring up decoder and connect to display */
    NEXUS_VideoDecoder_GetDefaultOpenSettings(&videoDecoderOpenSettings);
#if USE_SVP
    videoDecoderOpenSettings.cdbHeap = platformConfig.heap[NEXUS_VIDEO_SECURE_HEAP];
    if (secure_pic_buffers)
    {
        /* Request a secure decoder, i.e. secure picture buffers, i.e. URR
        * Should only do this if SAGE is in use, and when SAGE_SECURE_MODE is NOT 1 */
        videoDecoderOpenSettings.secureVideo = NEXUS_SecureVideo_eSecure;
    }
#endif
    videoDecoder = NEXUS_VideoDecoder_Open(0, &videoDecoderOpenSettings);
    NEXUS_VideoWindow_AddInput(window, NEXUS_VideoDecoder_GetConnector(videoDecoder));

    /* Bring up audio decoders and outputs */
    NEXUS_AudioDecoder_GetDefaultOpenSettings(&audioDecoderOpenSettings);
#if USE_SVP
    audioDecoderOpenSettings.cdbHeap = platformConfig.heap[NEXUS_VIDEO_SECURE_HEAP];
#endif
    audioDecoder = NEXUS_AudioDecoder_Open(0, &audioDecoderOpenSettings);
#if NEXUS_NUM_AUDIO_DACS
    NEXUS_AudioOutput_AddInput(
        NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]),
        NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
#endif
#if NEXUS_NUM_SPDIF_OUTPUTS
    NEXUS_AudioOutput_AddInput(
        NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]),
        NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
#endif
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_AudioOutput_AddInput(
        NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]),
        NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
#endif


    ret = playback_piff(videoDecoder,
                  audioDecoder,
                  drm_context,
                  argv[1]);

#if DEBUG_OUTPUT_CAPTURE
    fclose (fp_vid);
    fclose (fp_aud);
#endif

    DRM_Prdy_Uninitialize(drm_context);

    NEXUS_VideoDecoder_Stop(videoDecoder);
    NEXUS_AudioDecoder_Stop(audioDecoder);
    NEXUS_VideoDecoder_Close(videoDecoder);
    NEXUS_AudioDecoder_Close(audioDecoder);
    NEXUS_VideoWindow_Close(window);
    NEXUS_Display_Close(display);
    NEXUS_Platform_Uninit();


clean_exit:
    return ret;
}
