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
#include "drm_data.h"

#include "drmbytemanip.h"
#include "drmmanager.h"
#include "drmbase64.h"
#include "drmmanagertypes.h"
#include "drmsoapxmlutility.h"
#include "oemcommon.h"
#include "drmconstants.h"
#include "drmsecureclockstatus.h"

#include "prdy_http.h"

#include "nxclient.h"
#include "nexus_surface_client.h"

#include <sage_srai.h>

#define REPACK_VIDEO_PES_ID 0xE0
#define REPACK_AUDIO_PES_ID 0xC0

#define BOX_HEADER_SIZE (8)
#define BUF_SIZE (1024 * 1024 * 2) /* 2MB */

#define CALCULATE_PTS(t)        (((uint64_t)(t) / 10000LL) * 45LL)

// ~100 KB to start * 64 (2^6) ~= 6.4 MB, don't allocate more than ~6.4 MB
#define DRM_MAXIMUM_APPCONTEXT_OPAQUE_BUFFER_SIZE ( 64 * MINIMUM_APPCONTEXT_OPAQUE_BUFFER_SIZE )

#define SECURE_CLOCK 1

BDBG_MODULE(prdy30_svp);


typedef struct app_ctx {
    FILE *fp_piff;
    uint32_t piff_filesize;

    uint8_t *pPayload;
    uint8_t *pOutBuf;

    size_t outBufSize;

    uint64_t last_video_fragment_time;
    uint64_t last_audio_fragment_time;

    DRM_DECRYPT_CONTEXT decryptor;
} app_ctx;

typedef struct PRDY_APP_CONTEXT
{
    DRM_APP_CONTEXT     *pDrmAppCtx;          /* drm application context */
    DRM_VOID            *pOEMContext;         /* Oem Context */
    DRM_BYTE            *pbOpaqueBuffer;      /* Opaque buffer */
    DRM_DWORD            cbOpaqueBuffer;
} PRDY_APP_CONTEXT;

typedef struct pthread_info {
    NEXUS_SimpleVideoDecoderHandle videoDecoder;
    int result;
} pthread_info;

/* stream type */
int vc1_stream = 0;
typedef app_ctx * app_ctx_t;
static int video_decode_hdr;

static int gui_init( NEXUS_SurfaceClientHandle surfaceClient );

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
        NEXUS_SimpleVideoDecoder_GetStatus(info->videoDecoder, &status);
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

static DRM_RESULT secure_process_fragment(CommonCryptoHandle commonCryptoHandle, app_ctx *app,
        piff_parse_frag_info *frag_info, size_t payload_size,
        void *decoder_data, size_t decoder_len,
        NEXUS_PlaypumpHandle playpump, BKNI_EventHandle event)
{
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
    uint8_t *out2;
    DRM_AES_COUNTER_MODE_CONTEXT aesCtrInfo;
    size_t sampleSize = 0;
    DRM_RESULT dr = DRM_SUCCESS;

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
            out2 = out;

            if(frag_info->samples_enc->flags & 0x000002) {
                uint64_t     qwOffset = 0;

#if 0
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

                    BDBG_MSG(("%s:%d: Drm_Reader_DecryptLegacy(..., ..., %p, %u)",
                              __FUNCTION__, __LINE__, out, num_enc));
                    ChkDR(DRM_Reader_Decrypt_Legacy(
                                &app->decryptor,
                                &aesCtrInfo,
                                (uint8_t *)out,
                                num_enc ));

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

#endif
                uint32_t *pEncryptedRegionMappings = BKNI_Malloc( sizeof(uint32_t) * pSample->nbOfEntries * 2);
                NEXUS_DmaJobBlockSettings *blkSettings = BKNI_Malloc( sizeof(NEXUS_DmaJobBlockSettings) * pSample->nbOfEntries * 2);
                CommonCryptoJobSettings cryptoJobSettings;
                size_t entryNb = 0;
                int blk_idx = 0;
                int k = 0;

                for(j = 0; j < pSample->nbOfEntries; j++) {

                    uint32_t num_clear = pSample->entries[j].bytesOfClearData;
                    uint32_t num_enc = pSample->entries[j].bytesOfEncData;

                    pEncryptedRegionMappings[entryNb++] = num_clear;
                    pEncryptedRegionMappings[entryNb++] = num_enc;

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


                    outSize += (num_enc + num_clear - decrypt_offset);

                    /* Skip over remaining clear units */
                    out += (num_clear - decrypt_offset);
                    batom_cursor_skip((batom_cursor *)&frag_info->cursor, (num_clear - decrypt_offset));

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

                for (k = 0; k < blk_idx; k++ ) {
                    NEXUS_FlushCache(blkSettings[k].pSrcAddr, blkSettings[k].blockSize);
                    blkSettings[k].cached = false; /* Prevent the DMA from flushing the buffers later on */
                }

                CommonCrypto_GetDefaultJobSettings(&cryptoJobSettings);
                CommonCrypto_DmaXfer(commonCryptoHandle,  &cryptoJobSettings, blkSettings, blk_idx);

                for (k = 0; k < blk_idx; k++ ) {
                    NEXUS_FlushCache(blkSettings[k].pSrcAddr, blkSettings[k].blockSize);
                }

                ChkDR(Drm_Reader_DecryptOpaque(
                                &app->decryptor,
                                pSample->nbOfEntries *2,
                                pEncryptedRegionMappings,
                                aesCtrInfo.qwInitializationVector,
                                sampleSize,
                                out2,
                                &sampleSize,
                                &out2 ));

                BKNI_Free(blkSettings);
                BKNI_Free(pEncryptedRegionMappings);


            } else {
                uint32_t encryptedRegionMappings[2];

                piff_playback_dma_buffer(commonCryptoHandle, out, (uint8_t *)frag_info->cursor.cursor,
                        sampleSize, true);
                outSize += sampleSize;

                encryptedRegionMappings[0] = 0; /* 0 bytes of clear */
                encryptedRegionMappings[1] = sampleSize; /* 0 bytes of clear */

                BDBG_MSG(("%s:%d: Drm_Reader_DecryptLegacy(..., ..., %p, %u)",
                          __FUNCTION__, __LINE__, out, sampleSize));

                ChkDR(Drm_Reader_DecryptOpaque(
                                &app->decryptor,
                                2,
                                encryptedRegionMappings,
                                aesCtrInfo.qwInitializationVector,
                                sampleSize,
                                out,
                                &sampleSize,
                                &out ));

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
        dr = DRM_E_CRYPTO_FAILED;
    }

ErrorExit:
    return dr;
}




static void play_callback(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
}

static void
wait_for_drain(NEXUS_PlaypumpHandle videoPlaypump, NEXUS_SimpleVideoDecoderHandle videoDecoder,
               NEXUS_PlaypumpHandle audioPlaypump, NEXUS_SimpleAudioDecoderHandle audioDecoder)
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
            rc=NEXUS_SimpleVideoDecoder_GetStatus(videoDecoder, &decoderStatus);
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
            rc=NEXUS_SimpleAudioDecoder_GetStatus(audioDecoder, &decoderStatus);
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
        NEXUS_SimpleAudioDecoderHandle audioDecoder,
        NEXUS_SimpleVideoDecoderHandle videoDecoder,
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
        NEXUS_SimpleVideoDecoder_Stop(videoDecoder);
        NEXUS_Playpump_ClosePidChannel(videoPlaypump, videoPidChannel);
        NEXUS_Playpump_Stop(videoPlaypump);
        NEXUS_StopCallbacks(videoPlaypump);
    }
    if (audioDecoder) {
        NEXUS_SimpleAudioDecoder_Stop(audioDecoder);
        NEXUS_Playpump_ClosePidChannel(audioPlaypump, audioPidChannel);
        NEXUS_Playpump_Stop(audioPlaypump);
        NEXUS_StopCallbacks(audioPlaypump);
    }

    NEXUS_Playpump_Close(videoPlaypump);
    NEXUS_Playpump_Close(audioPlaypump);

    return 0;
}

DRM_API DRM_RESULT DRM_CALL DRMTOOLS_PrintOPLOutputIDs( __in const DRM_OPL_OUTPUT_IDS *f_pOPLs )
{
    DRM_RESULT dr = DRM_SUCCESS;
    DRM_DWORD i;
    DRM_WCHAR rgwszGUID[DRM_GUID_STRING_LEN+1] = {0};
    DRM_CHAR  rgszGUID[DRM_NO_OF(rgwszGUID)] = {0};

    printf("    (%d GUIDs)\r\n", f_pOPLs->cIds );
    for( i = 0; i < f_pOPLs->cIds; i++ )
    {
        ChkDR( DRM_UTL_GuidToString( &f_pOPLs->rgIds[i], rgwszGUID ) );
        /* Safe to use, input parameter is in ASCII */
        DRM_UTL_DemoteUNICODEtoASCII( rgwszGUID, rgszGUID, DRM_NO_OF(rgwszGUID)-1 );

        printf("    GUID = %s\r\n", rgszGUID);
    }
    printf("\r\n");
ErrorExit:
    return dr;
}

DRM_API DRM_RESULT DRM_CALL DRMTOOLS_PrintVideoOutputProtectionIDs( __in const DRM_VIDEO_OUTPUT_PROTECTION_IDS_EX *f_pOPLs )
{
    DRM_RESULT dr = DRM_SUCCESS;
    DRM_DWORD i;
    DRM_WCHAR rgwszGUID[DRM_GUID_STRING_LEN+1] = {0};
    DRM_CHAR  rgszGUID[DRM_NO_OF(rgwszGUID)] = {0};

    printf("    (%d entries)\r\n", f_pOPLs->cEntries );
    for( i = 0; i < f_pOPLs->cEntries; i++ )
    {
        ChkDR( DRM_UTL_GuidToString( &f_pOPLs->rgVop[i].guidId,
                            rgwszGUID ) );
        /* Safe to use, input parameter is in ASCII */
        DRM_UTL_DemoteUNICODEtoASCII( rgwszGUID, rgszGUID, DRM_NO_OF(rgwszGUID)-1 );

        printf("    GUID = %s\r\n", rgszGUID);
    }
    printf("\r\n");
ErrorExit:
    return dr;
}

DRM_RESULT policy_callback(
    const DRM_VOID                 *f_pvPolicyCallbackData,
          DRM_POLICY_CALLBACK_TYPE  f_dwCallbackType,
    const DRM_VOID  *f_pv )
{
    DRM_RESULT dr = DRM_SUCCESS;
    const DRM_PLAY_OPL_EX *oplPlay = NULL;

    BSTD_UNUSED(f_pv);

    switch( f_dwCallbackType )
    {
        case DRM_PLAY_OPL_CALLBACK:
            printf("  Got DRM_PLAY_OPL_CALLBACK from Bind:\r\n");
            ChkArg( f_pvPolicyCallbackData != NULL );
            oplPlay = (const DRM_PLAY_OPL_EX*)f_pvPolicyCallbackData;

            printf("    minOPL:\r\n");
            printf("    wCompressedDigitalVideo   = %d\r\n", oplPlay->minOPL.wCompressedDigitalVideo);
            printf("    wUncompressedDigitalVideo = %d\r\n", oplPlay->minOPL.wUncompressedDigitalVideo);
            printf("    wAnalogVideo              = %d\r\n", oplPlay->minOPL.wAnalogVideo);
            printf("    wCompressedDigitalAudio   = %d\r\n", oplPlay->minOPL.wCompressedDigitalAudio);
            printf("    wUncompressedDigitalAudio = %d\r\n", oplPlay->minOPL.wUncompressedDigitalAudio);
            printf("\r\n");

            printf("    oplIdReserved:\r\n");
            ChkDR( DRMTOOLS_PrintOPLOutputIDs( &oplPlay->oplIdReserved ) );

            printf("    vopi:\r\n");
            ChkDR( DRMTOOLS_PrintVideoOutputProtectionIDs( &oplPlay->vopi ) );

            break;

        case DRM_EXTENDED_RESTRICTION_QUERY_CALLBACK:
            {
                const DRM_EXTENDED_RESTRICTION_CALLBACK_STRUCT *pExtCallback = (const DRM_EXTENDED_RESTRICTION_CALLBACK_STRUCT*)f_pvPolicyCallbackData;
                DRM_DWORD i = 0;

                printf("  Got DRM_EXTENDED_RESTRICTION_QUERY_CALLBACK from Bind:\r\n");

                printf("    wRightID = %d\r\n", pExtCallback->wRightID);
                printf("    wType    = %d\r\n", pExtCallback->pRestriction->wType);
                printf("    wFlags   = %x\r\n", pExtCallback->pRestriction->wFlags);

                printf("    Data     = ");

                for( i = pExtCallback->pRestriction->ibData; (i - pExtCallback->pRestriction->ibData) < pExtCallback->pRestriction->cbData; i++ )
                {
                    printf("0x%.2X ", pExtCallback->pRestriction->pbBuffer[ i ] );
                }
                printf("\r\n\r\n");

                /* Report that restriction was not understood */
                dr = DRM_E_EXTENDED_RESTRICTION_NOT_UNDERSTOOD;
            }
            break;
        case DRM_EXTENDED_RESTRICTION_CONDITION_CALLBACK:
            {
                const DRM_EXTENDED_RESTRICTION_CALLBACK_STRUCT *pExtCallback = (const DRM_EXTENDED_RESTRICTION_CALLBACK_STRUCT*)f_pvPolicyCallbackData;
                DRM_DWORD i = 0;

                printf("  Got DRM_EXTENDED_RESTRICTION_CONDITION_CALLBACK from Bind:\r\n");

                printf("    wRightID = %d\r\n", pExtCallback->wRightID);
                printf("    wType    = %d\r\n", pExtCallback->pRestriction->wType);
                printf("    wFlags   = %x\r\n", pExtCallback->pRestriction->wFlags);

                printf("    Data     = ");
                for( i = pExtCallback->pRestriction->ibData; (i - pExtCallback->pRestriction->ibData) < pExtCallback->pRestriction->cbData; i++ )
                {
                    printf("0x%.2X ", pExtCallback->pRestriction->pbBuffer[ i ] );
                }
                printf("\r\n\r\n");
            }
            break;
        case DRM_EXTENDED_RESTRICTION_ACTION_CALLBACK:
            {
                const DRM_EXTENDED_RESTRICTION_CALLBACK_STRUCT *pExtCallback = (const DRM_EXTENDED_RESTRICTION_CALLBACK_STRUCT*)f_pvPolicyCallbackData;
                DRM_DWORD i = 0;

                printf("  Got DRM_EXTENDED_RESTRICTION_ACTION_CALLBACK from Bind:\r\n");

                printf("    wRightID = %d\r\n", pExtCallback->wRightID);
                printf("    wType    = %d\r\n", pExtCallback->pRestriction->wType);
                printf("    wFlags   = %x\r\n", pExtCallback->pRestriction->wFlags);

                printf("    Data     = ");
                for( i = pExtCallback->pRestriction->ibData; (i - pExtCallback->pRestriction->ibData) < pExtCallback->pRestriction->cbData; i++ )
                {
                    printf("0x%.2X ", pExtCallback->pRestriction->pbBuffer[ i ] );
                }
                printf("\r\n\r\n");
            }
            break;
    default:
        printf("  Callback from Bind with unknown callback type of %d.\r\n", f_dwCallbackType);

        /* Report that this callback type is not implemented */
        ChkDR( DRM_E_NOTIMPL );
    }

ErrorExit:
    return dr;

}

int playback_piff( NEXUS_SimpleVideoDecoderHandle videoDecoder,
                   NEXUS_SimpleAudioDecoderHandle audioDecoder,
                   PRDY_APP_CONTEXT         *pPrdyContext,
                   char *piff_file)
{

    uint8_t *pSecureVideoHeapBuffer = NULL;
    uint8_t *pSecureAudioHeapBuffer = NULL;
    pthread_t pthread;
    pthread_info *info;
    int finalResult = -1;

    NEXUS_ClientConfiguration clientConfig;
    NEXUS_MemoryAllocationSettings memSettings;
    NEXUS_SimpleStcChannelHandle stcChannel;
    NEXUS_SimpleVideoDecoderStartSettings videoProgram;
    NEXUS_SimpleAudioDecoderStartSettings audioProgram;
    NEXUS_PlaypumpOpenSettings videoplaypumpOpenSettings;
    NEXUS_PlaypumpOpenSettings audioplaypumpOpenSettings;
    NEXUS_SimpleStcChannelSettings stcSettings;
    NEXUS_PlaypumpSettings playpumpSettings;
    NEXUS_Error rc;
    CommonCryptoHandle commonCryptoHandle = NULL;
    CommonCryptoSettings  cmnCryptoSettings;

    NEXUS_DisplayHandle display = NULL;
    NEXUS_PidChannelHandle videoPidChannel = NULL;
    BKNI_EventHandle event = NULL;
    NEXUS_PlaypumpHandle videoPlaypump = NULL;
    NEXUS_PlaypumpHandle audioPlaypump = NULL;
    DRM_RESULT dr;

    DRM_CHAR *pszCustomDataUsed = NULL;
    DRM_DWORD cchCustomDataUsed = 0;
    const DRM_CONST_STRING *rgstrRights[ 1 ] = { &g_dstrWMDRM_RIGHT_PLAYBACK };
    uint8_t                *pbNewOpaqueBuffer = NULL;
    uint32_t cbNewOpaqueBuffer = pPrdyContext->cbOpaqueBuffer * 2;
    DRM_LICENSE_RESPONSE oResponse;

    NEXUS_PidChannelHandle audioPidChannel = NULL;

    app_ctx app;
    bool moovBoxParsed = false;

    uint8_t resp_buffer[64*1024];
    char *pCh_url = NULL;
    uint8_t *pCh_data = NULL;
    uint8_t *pResponse = resp_buffer;
    size_t respLen;
    size_t respOffset;
    size_t urlLen;
    size_t chLen;
    piff_parser_handle_t piff_handle;
    bfile_io_read_t fd;
    uint8_t *pssh_data;
    uint32_t pssh_len;
    NEXUS_PlaypumpOpenPidChannelSettings video_pid_settings;
    DRM_DWORD dwEncryptionMode  = OEM_TEE_AES128CTR_DECRYPTION_MODE_NOT_SECURE;

    if(piff_file == NULL ) {
        goto ErrorExit;
    }

    info = BKNI_Malloc(sizeof(pthread_info));

    BDBG_MSG(("PIFF file: %s\n",piff_file));
    fflush(stdout);

    BKNI_Memset(&app, 0, sizeof( app_ctx));
    app.last_video_fragment_time = 0;
    app.last_audio_fragment_time = 0;

    app.fp_piff = fopen(piff_file, "rb");
    if(app.fp_piff == NULL){
        fprintf(stderr,"failed to open %s\n", piff_file);
        goto ErrorExit;
    }

    fd = bfile_stdio_read_attach(app.fp_piff);

    piff_handle = piff_parser_create(fd);
    if (!piff_handle) {
        BDBG_ERR(("Unable to create PIFF parser context"));
        goto ErrorExit;
    }

    fseek(app.fp_piff, 0, SEEK_END);
    app.piff_filesize = ftell(app.fp_piff);
    fseek(app.fp_piff, 0, SEEK_SET);

    if( pPrdyContext == NULL)
    {
       BDBG_ERR(("pPrdyContext is NULL, quitting...."));
       goto ErrorExit ;
    }

    CommonCrypto_GetDefaultSettings(&cmnCryptoSettings);
    commonCryptoHandle = CommonCrypto_Open(&cmnCryptoSettings);

    NEXUS_Platform_GetClientConfiguration(&clientConfig);
    NEXUS_Memory_GetDefaultAllocationSettings(&memSettings);
    memSettings.heap = clientConfig.heap[1]; /* heap 1 is the eFull heap for the nxclient. */

    /* Show heaps info */
    {
        int g;
        BDBG_MSG(("NxClient Heaps Info -----------------"));
        for (g = NXCLIENT_DEFAULT_HEAP; g <= NXCLIENT_SECONDARY_GRAPHICS_HEAP; g++)
        {
            NEXUS_MemoryStatus status;
            NEXUS_Heap_GetStatus(clientConfig.heap[g], &status);

            BDBG_MSG(("Heap[%d]: memoryType=%u, heapType=%u, offset=%u, addr=%p, size=%u",
                      g, status.memoryType, status.heapType, (uint32_t)status.offset, status.addr, status.size));
        }
        BDBG_MSG(("-------------------------------------"));
    }

    app.pPayload = NULL;
    if( NEXUS_Memory_Allocate(BUF_SIZE, &memSettings, (void *)&app.pPayload) !=  NEXUS_SUCCESS) {
        fprintf(stderr,"NEXUS_Memory_Allocate failed");
        goto ErrorExit;
    }

    app.pOutBuf = NULL;
    if( NEXUS_Memory_Allocate(BUF_SIZE, &memSettings, (void *)&app.pOutBuf) !=  NEXUS_SUCCESS) {
        fprintf(stderr,"NEXUS_Memory_Allocate failed");
        goto ErrorExit;
    }

    /* Perform parsing of the movie information */
    moovBoxParsed = piff_parser_scan_movie_info(piff_handle);

    if(!moovBoxParsed) {
        BDBG_ERR(("Failed to parse moov box, can't continue..."));
        goto ErrorExit;
    }

    BDBG_MSG(("Successfully parsed the moov box, continue...\n\n"));

    /* EXTRACT AND PLAYBACK THE MDAT */

    NEXUS_Playpump_GetDefaultOpenSettings(&videoplaypumpOpenSettings);
    videoplaypumpOpenSettings.fifoSize *= 7;
    videoplaypumpOpenSettings.numDescriptors *= 7;

    info->videoDecoder = videoDecoder;
    videoplaypumpOpenSettings.dataNotCpuAccessible = true;
    pSecureVideoHeapBuffer = SRAI_Memory_Allocate(videoplaypumpOpenSettings.fifoSize,
            SRAI_MemoryType_SagePrivate);
    if ( pSecureVideoHeapBuffer == NULL ) {
        BDBG_ERR((" Failed to allocate from Secure Video heap"));
        BDBG_ASSERT( false );
    }
    videoplaypumpOpenSettings.memory = NEXUS_MemoryBlock_FromAddress(pSecureVideoHeapBuffer);

    videoPlaypump = NEXUS_Playpump_Open(NEXUS_ANY_ID, &videoplaypumpOpenSettings);
    if (!videoPlaypump) {
        BDBG_ERR(("@@@ Video Playpump Open FAILED----"));
        goto ErrorExit;
    }
    BDBG_ASSERT(videoPlaypump != NULL);


    NEXUS_Playpump_GetDefaultOpenSettings(&audioplaypumpOpenSettings);
    audioplaypumpOpenSettings.dataNotCpuAccessible = true;
    pSecureAudioHeapBuffer = SRAI_Memory_Allocate(audioplaypumpOpenSettings.fifoSize,
            SRAI_MemoryType_SagePrivate);
    if ( pSecureAudioHeapBuffer == NULL ) {
        BDBG_ERR((" Failed to allocate from Secure Audio heap"));
        goto ErrorExit;
    }
    BDBG_ASSERT( pSecureAudioHeapBuffer != NULL );
    audioplaypumpOpenSettings.memory = NEXUS_MemoryBlock_FromAddress(pSecureAudioHeapBuffer);

    audioPlaypump = NEXUS_Playpump_Open(NEXUS_ANY_ID, &audioplaypumpOpenSettings);
    if (!audioPlaypump) {
        BDBG_ERR(("@@@ Video Playpump Open FAILED----"));
        goto ErrorExit;
    }
    BDBG_ASSERT(audioPlaypump != NULL);

    stcChannel = NEXUS_SimpleStcChannel_Create(NULL);
    NEXUS_SimpleStcChannel_GetSettings(stcChannel, &stcSettings);
    stcSettings.mode = NEXUS_StcChannelMode_eAuto;
    rc = NEXUS_SimpleStcChannel_SetSettings(stcChannel, &stcSettings);
    if (rc) {
       BDBG_WRN(("@@@ Stc Set FAILED ---------------"));
    }

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

    /* already connected in main */
    NEXUS_SimpleAudioDecoder_GetDefaultStartSettings(&audioProgram);
    NEXUS_SimpleVideoDecoder_GetDefaultStartSettings(&videoProgram);

    NEXUS_Playpump_Start(videoPlaypump);
    NEXUS_Playpump_Start(audioPlaypump);

    NEXUS_Playpump_GetDefaultOpenPidChannelSettings(&video_pid_settings);
    video_pid_settings.pidType = NEXUS_PidType_eVideo;

    videoPidChannel = NEXUS_Playpump_OpenPidChannel(videoPlaypump, REPACK_VIDEO_PES_ID, &video_pid_settings);
    NEXUS_SetPidChannelBypassKeyslot(videoPidChannel, NEXUS_BypassKeySlot_eGR2R);

    if ( !videoPidChannel )
      BDBG_WRN(("@@@ videoPidChannel NULL"));
    else
      BDBG_WRN(("@@@ videoPidChannel OK"));

    audioPidChannel = NEXUS_Playpump_OpenPidChannel(audioPlaypump, REPACK_AUDIO_PES_ID, NULL);
    NEXUS_SetPidChannelBypassKeyslot(audioPidChannel, NEXUS_BypassKeySlot_eGR2R);

    if ( !audioPidChannel )
      BDBG_WRN(("@@@ audioPidChannel NULL"));
    else
      BDBG_WRN(("@@@ audioPidChannel OK"));

    NEXUS_SimpleAudioDecoder_GetDefaultStartSettings(&audioProgram);
    NEXUS_SimpleVideoDecoder_GetDefaultStartSettings(&videoProgram);

    if ( vc1_stream ) {
       BDBG_MSG(("@@@ set video audio program for vc1"));
       videoProgram.settings.codec = NEXUS_VideoCodec_eVc1;
       audioProgram.primary.codec = NEXUS_AudioCodec_eWmaPro;
    } else {
       BDBG_MSG(("@@@ set video audio program for h264"));
       videoProgram.settings.codec = NEXUS_VideoCodec_eH264;
       audioProgram.primary.codec = NEXUS_AudioCodec_eAacAdts;
    }

    videoProgram.settings.pidChannel = videoPidChannel;
    NEXUS_SimpleVideoDecoder_Start(videoDecoder, &videoProgram);

    audioProgram.primary.pidChannel = audioPidChannel;
    NEXUS_SimpleAudioDecoder_Start(audioDecoder, &audioProgram);

    if (videoProgram.settings.pidChannel) {
        BDBG_WRN(("@@@ set stc channel video"));
        NEXUS_SimpleVideoDecoder_SetStcChannel(videoDecoder, stcChannel);
    }

    if (audioProgram.primary.pidChannel) {
        BDBG_WRN(("@@@ set stc channel audio"));
        NEXUS_SimpleAudioDecoder_SetStcChannel(audioDecoder, stcChannel);
    }

    /***********************
     * now ready to decrypt
     ***********************/
    pssh_data = piff_parser_get_pssh(piff_handle, &pssh_len);
    if (!pssh_data) {
        BDBG_ERR(("Failed to obtain pssh data"));
        goto ErrorExit;
    }

    printf("%s - calling Drm_Content_SetProperty\n", __FUNCTION__);

    ChkDR (Drm_Content_SetProperty(pPrdyContext->pDrmAppCtx,
                                   DRM_CSP_AUTODETECT_HEADER,
                                   pssh_data,
                                   pssh_len ));


    /* set encryption/decryption mode */
    dwEncryptionMode = OEM_TEE_AES128CTR_DECRYPTION_MODE_HANDLE;
    dr = Drm_Content_SetProperty(
        pPrdyContext->pDrmAppCtx,
        DRM_CSP_DECRYPTION_OUTPUT_MODE,
        (const DRM_BYTE*)&dwEncryptionMode,
        sizeof( DRM_DWORD ) ) ;
    if ( dr != DRM_SUCCESS ) {
        BDBG_ERR(("Drm_Content_SetProperty() failed, exiting"));
        goto ErrorExit;
    }


    printf("%s - calling Drm_LicenseAcq_GenerateChallenge\n", __FUNCTION__);
    dr = Drm_LicenseAcq_GenerateChallenge(
            pPrdyContext->pDrmAppCtx,
            rgstrRights,
            sizeof(rgstrRights)/sizeof(DRM_CONST_STRING*), /*1,*/
            NULL,
            pszCustomDataUsed,
            cchCustomDataUsed,
            NULL,
            &urlLen,
            NULL,
            NULL,
            NULL,
            &chLen);

    if ( dr != DRM_E_BUFFERTOOSMALL ) {
        BDBG_ERR(("Drm_LicenseAcq_GenerateChallenge() failed, exiting"));
        goto ErrorExit;
    }

    pCh_url = BKNI_Malloc(urlLen);
    if(pCh_url == NULL) {
        BDBG_ERR(("BKNI_Malloc(urlent) failed, exiting..."));
        goto ErrorExit;
    }

    pCh_data = BKNI_Malloc(chLen);
    if(pCh_data == NULL) {
        BDBG_ERR(("BKNI_Malloc(chLen) failed, exiting..."));
        goto ErrorExit;
    }

    printf("%s - calling Drm_LicenseAcq_GenerateChallenge 2\n", __FUNCTION__);
    ChkDR( Drm_LicenseAcq_GenerateChallenge(
                pPrdyContext->pDrmAppCtx,
                rgstrRights,
                sizeof(rgstrRights)/sizeof(DRM_CONST_STRING*), /*1,*/
                NULL,
                pszCustomDataUsed,
                cchCustomDataUsed,
                pCh_url,
                 &urlLen, /*(pUrl_len>0)?&cchURL:NULL, */
                NULL,
                NULL,
                pCh_data,
                &chLen));

    pCh_data[ chLen ] = 0;

    if(PRDY_HTTP_Client_LicensePostSoap("http://playready.directtaps.net/pr/svc/rightsmanager.asmx?PlayRight=1&SecurityLevel=3000", pCh_data, 1,
    /*if(PRDY_HTTP_Client_LicensePostSoap(pCh_url, pCh_data, 1,*/
        150, (unsigned char **)&pResponse, &respOffset, &respLen) != 0) {
        BDBG_ERR(("PRDY_HTTP_Client_LicensePostSoap() failed, exiting"));
        goto ErrorExit;
    }

    printf("%s - calling Drm_LicenseAcq_ProcessResponse\n", __FUNCTION__);
    BKNI_Memset( &oResponse, 0, sizeof( DRM_LICENSE_RESPONSE ) );

    dr =  Drm_LicenseAcq_ProcessResponse(
            pPrdyContext->pDrmAppCtx,
            DRM_PROCESS_LIC_RESPONSE_NO_FLAGS,
            (const uint8_t * )&pResponse[respOffset],
            respLen,
            &oResponse );
    printf("%s - calling Drm_LicenseAcq_ProcessResponse done, dr = %x\n", __FUNCTION__, (unsigned int)dr);
    ChkDR(dr);


    printf("%s - calling Drm_Reader_Bind\n", __FUNCTION__);
    while( (dr = Drm_Reader_Bind(
                    pPrdyContext->pDrmAppCtx,
                    rgstrRights,
                    1,
                    (DRMPFNPOLICYCALLBACK)policy_callback,
                    (void *) pPrdyContext,
                    (DRM_DECRYPT_CONTEXT *)  &app.decryptor)) == DRM_E_BUFFERTOOSMALL)
    {
        BDBG_ASSERT( cbNewOpaqueBuffer > pPrdyContext->cbOpaqueBuffer ); /* overflow check */

        if( cbNewOpaqueBuffer > DRM_MAXIMUM_APPCONTEXT_OPAQUE_BUFFER_SIZE )
        {
            ChkDR( DRM_E_OUTOFMEMORY );
        }

        ChkMem( pbNewOpaqueBuffer = ( uint8_t* )Oem_MemAlloc( cbNewOpaqueBuffer ) );

        ChkDR( Drm_ResizeOpaqueBuffer(
                    pPrdyContext->pDrmAppCtx,
                    pbNewOpaqueBuffer,
                    cbNewOpaqueBuffer ) );

        /*
         Free the old buffer and then transfer the new buffer ownership
         Free must happen after Drm_ResizeOpaqueBuffer because that
         function assumes the existing buffer is still valid
        */
        SAFE_OEM_FREE( pPrdyContext->pbOpaqueBuffer );
        pPrdyContext->cbOpaqueBuffer = cbNewOpaqueBuffer;
        pPrdyContext->pbOpaqueBuffer = pbNewOpaqueBuffer;
        pbNewOpaqueBuffer = NULL;
    }

    printf("%s - calling Drm_Reader_Bind dr %x\n", __FUNCTION__, (unsigned int)dr);

    if (DRM_FAILED( dr )) {
        if (dr == DRM_E_LICENSE_NOT_FOUND) {
            /* could not find a license for the KID */
            BDBG_ERR(("%s: no licenses found in the license store. Please request one from the license server.\n", __FUNCTION__));
        }
        else if(dr == DRM_E_LICENSE_EXPIRED) {
            /* License is expired */
            BDBG_ERR(("%s: License expired. Please request one from the license server.\n", __FUNCTION__));
        }
        else if(  dr == DRM_E_RIV_TOO_SMALL ||
                  dr == DRM_E_LICEVAL_REQUIRED_REVOCATION_LIST_NOT_AVAILABLE )
        {
            /* Revocation Package must be update */
            BDBG_ERR(("%s: Revocation Package must be update. 0x%x\n", __FUNCTION__,(unsigned int)dr));
        }
        else {
            BDBG_ERR(("%s: unexpected failure during bind. 0x%x\n", __FUNCTION__,(unsigned int)dr));
        }
    }

    printf("%s - calling Drm_Reader_Commit dr %x\n", __FUNCTION__, (unsigned int)dr);
    ChkDR( Drm_Reader_Commit( pPrdyContext->pDrmAppCtx, NULL, NULL ) );
    printf("%s - calling Drm_Reader_Commit dr %x\n", __FUNCTION__, (unsigned int)dr);

    /* now go back to the begining and get all the moof boxes */
    fseek(app.fp_piff, 0, SEEK_END);
    app.piff_filesize = ftell(app.fp_piff);
    fseek(app.fp_piff, 0, SEEK_SET);

    video_decode_hdr = 0;
    rc = pthread_create(&pthread, NULL, check_buffer, (void *)info);
    if (rc)
    {
        BDBG_ERR(("return code from pthread_create() is %d\n", rc));
        goto ErrorExit;
    }

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
                goto ErrorExit;
            }
        }
        decoder_data = piff_parser_get_dec_data(piff_handle, &decoder_len, frag_info.trackId);

        if (frag_info.trackType == BMP4_SAMPLE_ENCRYPTED_VIDEO) {
            secure_process_fragment(commonCryptoHandle, &app, &frag_info, (frag_info.mdat_size - BOX_HEADER_SIZE),
                    decoder_data, decoder_len, videoPlaypump, event);

        } else if (frag_info.trackType == BMP4_SAMPLE_ENCRYPTED_AUDIO) {
            secure_process_fragment(commonCryptoHandle, &app, &frag_info, (frag_info.mdat_size - BOX_HEADER_SIZE),
                    decoder_data, decoder_len, audioPlaypump, event);
        }
    } /* while */

ErrorExit:
    complete_play_fragments(audioDecoder, videoDecoder, videoPlaypump,
            audioPlaypump, display, audioPidChannel, videoPidChannel, NULL, event);
    if(stcChannel) NEXUS_SimpleStcChannel_Destroy(stcChannel);
    if(pthread){
        if(pthread_join(pthread, (void**) &info) != 0)
        {
            BDBG_ERR(("ERROR IN PTHREAD_JOIN"));
            goto ErrorExit;
        } else {
            finalResult = info->result;
        }
        if (info->result == 0) {
            BDBG_LOG(("Success!"));
        } else {
            BDBG_ERR(("ERROR: thread failed"));
        }
    }

    if(pbNewOpaqueBuffer) Oem_MemFree(pbNewOpaqueBuffer);

    if( &app.decryptor) Drm_Reader_Close( &app.decryptor);

    if(pCh_data != NULL) BKNI_Free(pCh_data);
    if(pCh_url != NULL) BKNI_Free(pCh_url);

    if(pSecureAudioHeapBuffer) SRAI_Memory_Free(pSecureAudioHeapBuffer);
    if(pSecureVideoHeapBuffer) SRAI_Memory_Free(pSecureVideoHeapBuffer);

    if(app.pOutBuf) NEXUS_Memory_Free(app.pOutBuf);
    if(app.pPayload) NEXUS_Memory_Free(app.pPayload);

    if(commonCryptoHandle) CommonCrypto_Close(commonCryptoHandle);

    if(piff_handle != NULL) piff_parser_destroy(piff_handle);

    bfile_stdio_read_detach(fd);
    if(app.fp_piff) fclose(app.fp_piff);

    if(info != NULL) BKNI_Free(info);
    return finalResult;
}

#ifdef SECURE_CLOCK
#define MAX_TIME_CHALLENGE_RESPONSE_LENGTH (1024*64)
#define MAX_URL_LENGTH (512)
int initSecureClock( DRM_APP_CONTEXT *pDrmAppCtx)
{
    int                   rc = 0;
    uint8_t              *pTimeChallengeResponse = NULL;
    char                 *pTimeChallengeURL = NULL;
    DRM_WCHAR            *pTimeURL=NULL;
    uint8_t              *pTimeChallengeData=NULL;
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
    DRM_RESULT            drResponse = DRM_SUCCESS;
    DRM_RESULT            dr = DRM_SUCCESS;

    dr = Drm_SecureClock_GenerateChallenge(
                    pDrmAppCtx,
                    (DRM_WCHAR *) NULL,
                    &timeURL_len,
                    (DRM_BYTE *) NULL,
                    &timeCh_len );
    if( dr == DRM_E_BUFFERTOOSMALL )
    {
        rc = NEXUS_Memory_Allocate(timeURL_len*sizeof(wchar_t), NULL, (void **)(&pTimeURL));
        if(rc != NEXUS_SUCCESS)
        {
            BDBG_ERR(("%s - %d NEXUS_Memory_Allocate failed for the Time server URL buffer, rc = %d\n", __FUNCTION__, __LINE__, rc));
            goto ErrorExit;
        }

        rc = NEXUS_Memory_Allocate(timeCh_len+1, NULL, (void **)(&pTimeChallengeData));
        if(rc != NEXUS_SUCCESS)
        {
            BDBG_ERR(("%s - %d NEXUS_Memory_Allocate failed for the time challenge buffer, rc = %d\n", __FUNCTION__, __LINE__, rc));
            goto ErrorExit;
        }

        BKNI_Memset(pTimeURL, 0, timeURL_len);
        BKNI_Memset(pTimeChallengeData, 0, timeCh_len+1);

        ChkDR(Drm_SecureClock_GenerateChallenge(
                    pDrmAppCtx,
                    pTimeURL,
                    &timeURL_len,
                    pTimeChallengeData,
                    &timeCh_len ));
    }
    else {
        ChkDR(dr);
    }

    timeURL_cstr[timeURL_len] = 0; /* null terminator */
    DRM_UTL_DemoteUNICODEtoASCII( pTimeURL, timeURL_cstr, timeURL_len);

    pTimeChallengeData[timeCh_len] = 0;

    NEXUS_Memory_GetDefaultAllocationSettings(&allocSettings);
    rc = NEXUS_Memory_Allocate(MAX_URL_LENGTH, &allocSettings, (void **)(&pTimeChallengeURL ));
    if(rc != NEXUS_SUCCESS)
    {
        BDBG_ERR(("%s - %d  NEXUS_Memory_Allocate failed for time challenge response buffer, rc = %d\n", __FUNCTION__, __LINE__, rc));
        goto ErrorExit;
    }

    do
    {
           redirect = false;

           /* send the petition request to Microsoft with HTTP GET */
           petRC = PRDY_HTTP_Client_GetPetition( timeURL_cstr,
                                                 &petRespCode,
                                                 (char**)&pTimeChallengeURL);
           if( petRC != 0)
           {
               BDBG_ERR(("%d Secure Clock Petition request failed, rc = %d\n",__LINE__, petRC));
               rc = petRC;
               goto ErrorExit;
           }

           /* we need to check if the Pettion responded with redirection */

           if( petRespCode == 200)
           {
               redirect = false;
           }
           else if( petRespCode == 302 || petRespCode == 301)
           {
               char * isHttps = NULL;

               redirect = true;

               memset(timeURL_cstr,0,timeURL_len);

               /* check if the URL is "https" */
               isHttps = strstr(pTimeChallengeURL,"https");
               if( isHttps )
               {
                   strcpy(timeURL_cstr,"http");
                   strcpy(timeURL_cstr+4,isHttps+5);
               }
               else
               {
                   strcpy(timeURL_cstr,pTimeChallengeURL);
               }

               memset(pTimeChallengeURL,0,MAX_URL_LENGTH);
           }
           else
           {
               BDBG_ERR(("%d Secure Clock Petition responded with unsupported result, rc = %d, can't get the tiem challenge URL\n",__LINE__, petRespCode));
               rc = -1;
               goto ErrorExit;
           }
    } while (redirect);


    NEXUS_Memory_GetDefaultAllocationSettings(&allocSettings);
    rc = NEXUS_Memory_Allocate(MAX_TIME_CHALLENGE_RESPONSE_LENGTH, &allocSettings, (void **)(&pTimeChallengeResponse ));
    if(rc != NEXUS_SUCCESS)
    {
        BDBG_ERR(("%d NEXUS_Memory_Allocate failed for time challenge response buffer, rc = %d\n",__LINE__, rc));
        goto ErrorExit;
    }

    BKNI_Memset(pTimeChallengeResponse, 0, MAX_TIME_CHALLENGE_RESPONSE_LENGTH);
    post_ret = PRDY_HTTP_Client_TimeChallengePost(pTimeChallengeURL,
                                                 (char *)pTimeChallengeData,
                                                 1,
                                                 150,
                                                 (unsigned char**)&(pTimeChallengeResponse),
                                                 &startOffset,
                                                 &length);
    if( post_ret != 0)
    {
        BDBG_ERR(("%d Secure Clock Challenge request failed, rc = %d\n",__LINE__, post_ret));
        rc = post_ret;
        goto ErrorExit;
    }

    ChkDR(Drm_SecureClock_ProcessResponse(
                pDrmAppCtx,
                (uint8_t *) pTimeChallengeResponse,
                length,
                &drResponse ));
    if ( drResponse != DRM_SUCCESS )
    {
       dr = drResponse;
       ChkDR( drResponse);

    }

    BDBG_LOG(("%d Initialized Playready Secure Clock success.",__LINE__));

    /* NOW testing the system time */

ErrorExit:

    if( pTimeChallengeResponse   != NULL)
        NEXUS_Memory_Free(pTimeChallengeResponse );

    if( pTimeChallengeURL    != NULL)
        NEXUS_Memory_Free(pTimeChallengeURL  );

    if( pTimeURL  != NULL)
        NEXUS_Memory_Free(pTimeURL );

    if( pTimeChallengeData  != NULL)
        NEXUS_Memory_Free(pTimeChallengeData );

    return rc;
}
#endif

int main(int argc, char* argv[])
{
    NxClient_JoinSettings joinSettings;
    NxClient_AllocSettings allocSettings;
    NxClient_AllocResults allocResults;
    NEXUS_Error rc;
    int ret = -1;

    NEXUS_ClientConfiguration clientConfig;
    NxClient_ConnectSettings connectSettings;
    unsigned connectId;
    NEXUS_SurfaceClientHandle surfaceClient = NULL;
    NEXUS_SurfaceClientHandle videoSurfaceClient = NULL;
    NEXUS_SimpleVideoDecoderHandle videoDecoder = NULL;
    NEXUS_SimpleAudioDecoderHandle audioDecoder = NULL;

    /* DRM_Prdy specific */
    DRM_RESULT           dr = DRM_SUCCESS;
    PRDY_APP_CONTEXT     prdyCtx;
    OEM_Settings         oemSettings;
    DRM_BYTE            *pbRevocationBuffer;

    DRM_WCHAR           *hdsDir = bdrm_get_hds_dir();
    DRM_WCHAR           *hdsFname = bdrm_get_pr3x_hds_fname();

    DRM_CONST_STRING   sDstrHDSPath = DRM_EMPTY_DRM_STRING;
    DRM_WCHAR          sRgwchHDSPath[ DRM_MAX_PATH ];

#ifdef SECURE_CLOCK
    uint32_t            secClkStatus; /* secure clock status */
    DRM_DWORD           cchSecTime         = 0;
    DRM_WCHAR          *pwszSecTime        = NULL;
    DRM_BYTE           *pbTimeStatus       = NULL;
    DRM_DWORD           cbTimeStatus       = 0;
#endif

    if (argc < 2) {
        BDBG_ERR(("Usage : %s <input_file> [-vc1]", argv[0]));
        return -1;
    }

    while(argc>2)
    {
        argc--;
        if(strcmp(argv[argc], "-vc1") == 0)
        {
            printf("%s - vc1_stream found\n", __FUNCTION__);
            vc1_stream = 1;
        }
        else
        {
            BDBG_ERR(("Unrecognized option: %s", argv[argc]));
        }
    }


    BDBG_MSG(("@@@ MSG Check Point Start vc1_stream %d--", vc1_stream));

    NxClient_GetDefaultJoinSettings(&joinSettings);
    snprintf(joinSettings.name, NXCLIENT_MAX_NAME, "pr_piff_playback");
    rc = NxClient_Join(&joinSettings);
    if (rc)
    {
        BDBG_ERR(("Error in NxClient_Join"));
        return -1;
    }
    /* print heaps on server side */
    NEXUS_Memory_PrintHeaps();


    NxClient_GetDefaultAllocSettings(&allocSettings);
    allocSettings.simpleVideoDecoder = 1;
    allocSettings.simpleAudioDecoder = 1;
    allocSettings.surfaceClient = 1;
    rc = NxClient_Alloc(&allocSettings, &allocResults);
    if (rc)
    {
        BDBG_ERR(("Error in NxClient_Alloc"));
        return BERR_TRACE(rc);
    }

    oemSettings.binFileName = NULL;
    oemSettings.keyHistoryFileName = NULL;

    /* initialize the DRM_APP_CONTEXT */
    BKNI_Memset(&prdyCtx, 0, sizeof(prdyCtx));

    prdyCtx.pDrmAppCtx = Oem_MemAlloc(sizeof(DRM_APP_CONTEXT));
    ChkMem(prdyCtx.pDrmAppCtx);
    BKNI_Memset(( uint8_t * )prdyCtx.pDrmAppCtx, 0, sizeof( DRM_APP_CONTEXT));

    NEXUS_Platform_GetClientConfiguration(&clientConfig);
    oemSettings.heap = clientConfig.heap[1]; /* heap 1 is the eFull heap for the nxclient. */

    dr = Drm_Platform_Initialize((void *)&oemSettings);
    ChkDR(dr);

    prdyCtx.pOEMContext = oemSettings.f_pOEMContext;
    ChkMem(prdyCtx.pOEMContext);
    printf("%s - prdyCtx.pOEMContext %p\n", __FUNCTION__, prdyCtx.pOEMContext);

    /* Initialize OpaqueBuffer and RevocationBuffer */
    prdyCtx.cbOpaqueBuffer = MINIMUM_APPCONTEXT_OPAQUE_BUFFER_SIZE;
    ChkMem( prdyCtx.pbOpaqueBuffer = ( uint8_t * )Oem_MemAlloc(MINIMUM_APPCONTEXT_OPAQUE_BUFFER_SIZE));
    ChkMem( pbRevocationBuffer = ( uint8_t * )Oem_MemAlloc( REVOCATION_BUFFER_SIZE));

    /* Drm_Initialize */
    sDstrHDSPath.pwszString = sRgwchHDSPath;
    sDstrHDSPath.cchString = DRM_MAX_PATH;
    printf("%s - %d\n", __FUNCTION__, __LINE__);

    /* Convert the HDS path to DRM_STRING. */

    printf("%s - %d\n", __FUNCTION__, __LINE__);
    if (bdrm_get_hds_dir_lgth() > 0){
        BKNI_Memcpy((DRM_WCHAR*)sDstrHDSPath.pwszString, hdsDir, bdrm_get_hds_dir_lgth() * sizeof(DRM_WCHAR));
    }
    BKNI_Memcpy((DRM_WCHAR*)sDstrHDSPath.pwszString + bdrm_get_hds_dir_lgth(), hdsFname, (bdrm_get_pr3x_hds_fname_lgth() + 1) * sizeof(DRM_WCHAR));


    if (hdsFname != NULL && bdrm_get_pr3x_hds_fname_lgth() > 0) {
        if (bdrm_get_hds_dir_lgth() > 0)
        {
            BKNI_Memcpy((DRM_WCHAR*)sDstrHDSPath.pwszString, hdsDir, bdrm_get_hds_dir_lgth() * sizeof(DRM_WCHAR));
            BKNI_Memcpy((DRM_WCHAR*)sDstrHDSPath.pwszString + bdrm_get_hds_dir_lgth(),
                        hdsFname, (bdrm_get_pr3x_hds_fname_lgth() + 1) * sizeof(DRM_WCHAR));
        }
    }

    printf("%s - %d\n", __FUNCTION__, __LINE__);
   ChkDR( Drm_Initialize( prdyCtx.pDrmAppCtx,
                          prdyCtx.pOEMContext,
                          prdyCtx.pbOpaqueBuffer,
                          prdyCtx.cbOpaqueBuffer,
                          &sDstrHDSPath) );
   printf("%s - %d\n", __FUNCTION__, __LINE__);

#ifdef SECURE_CLOCK
    dr = Drm_SecureClock_GetValue( prdyCtx.pDrmAppCtx, pwszSecTime, &cchSecTime, &secClkStatus, pbTimeStatus, &cbTimeStatus );
    if ( dr != DRM_E_BUFFERTOOSMALL )
    {
        goto ErrorExit;
    }

    ChkMem( pwszSecTime = (DRM_WCHAR*) Oem_MemAlloc( cchSecTime * sizeof( DRM_WCHAR ) ) );
    ChkMem( pbTimeStatus = (DRM_BYTE*) Oem_MemAlloc( cbTimeStatus ) );
    BKNI_Memset( pwszSecTime, 'a', cchSecTime * sizeof( DRM_WCHAR ) );
    BKNI_Memset( pbTimeStatus, 'b', cbTimeStatus );

    ChkDR( Drm_SecureClock_GetValue( prdyCtx.pDrmAppCtx, pwszSecTime, &cchSecTime, &secClkStatus, pbTimeStatus, &cbTimeStatus ) );

    if( secClkStatus != DRM_CLK_SET)
    {
       /* setup the Playready secure clock */
       if(initSecureClock(prdyCtx.pDrmAppCtx) != 0)
       {
           BDBG_ERR(("%d Failed to initiize Secure Clock, quitting....\n",__LINE__));
           goto ErrorExit;
       }
    }
#endif

    ChkDR( Drm_Revocation_SetBuffer( prdyCtx.pDrmAppCtx,
                                     pbRevocationBuffer,
                                     REVOCATION_BUFFER_SIZE ) );

    printf("%s - %d\n", __FUNCTION__, __LINE__);

    BDBG_MSG(("@@@ Check Point #01"));

    if (allocResults.simpleVideoDecoder[0].id) {
        BDBG_MSG(("@@@ to acquire video decoder"));
        videoDecoder = NEXUS_SimpleVideoDecoder_Acquire(allocResults.simpleVideoDecoder[0].id);
    }
    BDBG_ASSERT(videoDecoder);
    if (allocResults.simpleAudioDecoder.id) {
        BDBG_MSG(("@@@ to acquire audio decoder"));
        audioDecoder = NEXUS_SimpleAudioDecoder_Acquire(allocResults.simpleAudioDecoder.id);
    }
    BDBG_ASSERT(audioDecoder);
    if (allocResults.surfaceClient[0].id) {
        BDBG_MSG(("@@@ to acquire surfaceclient"));
        /* surfaceClient is the top-level graphics window in which video will fit.
        videoSurfaceClient must be "acquired" to associate the video window with surface compositor.
        Graphics do not have to be submitted to surfaceClient for video to work, but at least an
        "alpha hole" surface must be submitted to punch video through other client's graphics.
        Also, the top-level surfaceClient ID must be submitted to NxClient_ConnectSettings below. */
        surfaceClient = NEXUS_SurfaceClient_Acquire(allocResults.surfaceClient[0].id);
        videoSurfaceClient = NEXUS_SurfaceClient_AcquireVideoWindow(surfaceClient, 0);
    }
    NxClient_GetDefaultConnectSettings(&connectSettings);
    connectSettings.simpleVideoDecoder[0].id = allocResults.simpleVideoDecoder[0].id;
    connectSettings.simpleVideoDecoder[0].surfaceClientId = allocResults.surfaceClient[0].id;
    connectSettings.simpleAudioDecoder.id = allocResults.simpleAudioDecoder.id;
    connectSettings.simpleVideoDecoder[0].decoderCapabilities.secureVideo = true;
    rc = NxClient_Connect(&connectSettings, &connectId);
    if (rc)
    {
        BDBG_ERR(("Error in NxClient_Connect"));
        return BERR_TRACE(rc);
    }
    gui_init( surfaceClient );
    ret = playback_piff(videoDecoder,
                  audioDecoder,
                  &prdyCtx,
                  argv[1]);

ErrorExit:


    if (videoDecoder != NULL) {
        NEXUS_SimpleVideoDecoder_Release( videoDecoder );
    }

    if ( audioDecoder != NULL) {
        NEXUS_SimpleAudioDecoder_Release( audioDecoder );
    }

    if ( surfaceClient != NULL ) {
        NEXUS_SurfaceClient_Release( surfaceClient );
    }

#ifdef SECURE_CLOCK
    if( pwszSecTime != NULL ) {
        Oem_MemFree( pwszSecTime );
    }

    if( pbTimeStatus != NULL ) {
        Oem_MemFree( pbTimeStatus );
    }
#endif


    if( prdyCtx.pDrmAppCtx ) {
        Drm_Uninitialize( prdyCtx.pDrmAppCtx );
        Oem_MemFree(prdyCtx.pDrmAppCtx );
    }
    Oem_MemFree(prdyCtx.pbOpaqueBuffer);
    Oem_MemFree(pbRevocationBuffer);

    Drm_Platform_Uninitialize(prdyCtx.pOEMContext);

    NxClient_Disconnect(connectId);
    NxClient_Free(&allocResults);
    NxClient_Uninit();

    return ret;
}

static int gui_init( NEXUS_SurfaceClientHandle surfaceClient )
{
    NEXUS_Graphics2DHandle gfx;
    NEXUS_SurfaceHandle surface;

    NEXUS_Graphics2DSettings gfxSettings;
    NEXUS_SurfaceCreateSettings createSettings;
    NEXUS_Graphics2DFillSettings fillSettings;
    int rc;

    if (!surfaceClient) return -1;

    BDBG_MSG(("@@@ gui_init surfaceclient %d", (int)surfaceClient));
    gfx = NEXUS_Graphics2D_Open(NEXUS_ANY_ID, NULL);
    NEXUS_Graphics2D_GetSettings(gfx, &gfxSettings);
    rc = NEXUS_Graphics2D_SetSettings(gfx, &gfxSettings);
    BDBG_ASSERT(!rc);

    NEXUS_Surface_GetDefaultCreateSettings(&createSettings);
    createSettings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
    createSettings.width = 720;
    createSettings.height = 480;
    surface = NEXUS_Surface_Create(&createSettings);

    NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
    fillSettings.surface = surface;
    fillSettings.color = 0;
    rc = NEXUS_Graphics2D_Fill(gfx, &fillSettings);
    BDBG_ASSERT(!rc);

    rc = NEXUS_Graphics2D_Checkpoint(gfx, NULL); /* require to execute queue */

    rc = NEXUS_SurfaceClient_SetSurface(surfaceClient, surface);
    BDBG_ASSERT(!rc);

    return 0;
}
