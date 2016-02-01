/***************************************************************************
 *     Copyright (c) 2006-2013, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
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
 * Example to demostrate the basic video or audio playback of an encrypted PIFF file. 
 * This example are using PlayReady DRM to decrypt the content through the PR license acquisition process.
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/

#include "nexus_platform.h"
#include "nexus_wmdrmpd.h"
#include "nexus_wmdrmpd_types.h"
#include "nexus_security.h"
#include "nexus_video_decoder.h"
#include "nexus_stc_channel.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_video_input.h"
#include "nexus_spdif_output.h"
#include "nexus_component_output.h"
#include "nexus_video_adj.h"
#if NEXUS_HAS_PLAYBACK 
#include "nexus_playback.h"
#endif
#include "common_crypto.h"
#include "nexus_core_utils.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <assert.h>
#include <pthread.h>
#include <fcntl.h>
#include <errno.h>

#include <stdio.h>
#include <string.h>
#include "bstd.h"
#include "bkni.h"
#include "bdbg.h"

#include "bmp4_util.h"
#include "prdy_core.h"
#include "prdy_mp4.h"
#include "prdy_http.h"
#include "nexus_security_datatypes.h"
#include "nexus_dma_types.h"
#include "bpiff.h"
#include "bpiff_encoder_types.h"

BDBG_MODULE(playback_piff);
 
extern size_t bdrm_map_cursor_to_dmablks(batom_cursor *cursor, size_t count, NEXUS_DmaJobBlockSettings *blks, uint32_t nb_blks_avail, uint32_t *nb_blks_used);

#define DUMP_DATA_HEX(string,data,size) {        \
   char tmp[256]= "\0";                          \
   uint32_t i=0, l=strlen(string);               \
   sprintf(tmp,"%s",string);                     \
   while( i<size && l < 256) {                   \
    sprintf(tmp+l," %02x", data[i]); ++i; l+=3;} \
   printf(tmp);                                  \
   fflush(stdout);                               \
} 

#define UPDATE_FRAGMENT_DURATION(app,fragment_duration) { \
    uint32_t i=0;                                         \
    for(; i<app.run_header.sample_count; ++i) {           \
        fragment_duration += app.samples[i].duration;     \
    }                                                     \
}                                                         \

#define DMA_BLK_POOL_SIZE    100
#define DMA_BLKS_SIZE        100
#define KEY_IV_BUFFER_SIZE   100  
#define AES_EKL_SIZE         32

/* Box size + box type */
#define BOX_HEADER_SIZE (8)
#define MAX_SAMPLES (1024)
#define MAX_PPS_SPS 256

#define PLAYREADY_DRM 1

struct decoder_configuration {
    size_t size;
    uint8_t data[5+MAX_PPS_SPS];
};

typedef enum trackType {
        eAUDIO = 1,
        eVIDEO = 2
} trackType;

typedef struct app_ctx{
    FILE *fp_piff;
    uint32_t piff_filesize;

    batom_factory_t factory;

    uint8_t *pPayload;
    /* bdrm_t drm; */
    NEXUS_WmDrmPdHandle  drm;

    struct decoder_configuration avc_config;
    struct decoder_configuration aac_config;

    uint8_t track_type[5];

    /* MOOF info */
    bmp4_track_fragment_run_state  state;
    bmp4_track_fragment_header     fragment_header;
    bmp4_track_fragment_run_header run_header;
    /* bmp4_trackextendsbox     track_extends; */
    bmp4_track_fragment_run_sample samples[MAX_SAMPLES];

    /* samples encryption box */
    bdrm_mp4_se samples_enc; 

    uint64_t last_video_fragment_duration;
    uint64_t last_audio_fragment_duration;

    bool senc_parsed;
    bool video_encrypted;
    bool audio_encrypted;

}app_ctx;

typedef app_ctx * app_ctx_t;

const char usage_str[] =
    "\n"
    "DESCRIPTION:\n"
    "  Playback a well-formed PIFF file.\n"
    "\n"
    "USAGE: "
    " playback_piff -f piff_file [-audio]\n"
    "\n"
    "  -f, input PIFF fil.e\n"
    "  -audio, playback audio only. Playback video only if omitted.\n"
    "\n";

static int usage(void)
{
    fprintf(stderr, "%s\n", usage_str);
    return 0; 
}
    
bdrm_err dummy_license_callback(bdrm_acq_state* lic_acq_state,  bdrm_license_t license, const void* ctx)
{
    BSTD_UNUSED(lic_acq_state);
    BSTD_UNUSED(license);
    BSTD_UNUSED(ctx);
    return bdrm_err_ok;
}

bdrm_err dummy_policy_callback(bdrm_policy_t *policy, const void* ctx)
{
    BSTD_UNUSED(policy);
    BSTD_UNUSED(ctx);
    return bdrm_err_ok;
}

#define BUF_SIZE (1024 * 1024 * 2) /* 2MB */

typedef struct mem_file {
    void *buf;
    size_t buf_len;
    size_t length;
    size_t offset;
} mem_file;

static void mem_file_write(mem_file *file,const void *ptr, size_t size)
{
    BDBG_ASSERT(file->offset + size<= file->buf_len);
    BKNI_Memcpy((uint8_t *)file->buf + file->offset, ptr, size);
    file->offset += size;
    if(file->length<file->offset) {
        file->length = file->offset;
    }
    return ;
}

static size_t mem_file_tell(mem_file *file)
{
    return file->offset;
}

static void mem_file_seek(mem_file *file, size_t offset)
{
    file->offset = offset;
    return;
}

#if 0

static uint8_t g_contentKey[16] = {0x4c, 0xda, 0xd6, 0x21, 0xc6, 0x8f, 0xbb, 0xd2, 0x85, 0xc3, 0x63, 0x73, 0x7f, 0x96, 0x6f, 0x1e }; 

typedef struct local_decryptor 
{
    CommonCryptoHandle             cryptoHandle;
    NEXUS_KeySlotHandle            keySlot;
    NEXUS_DmaJobBlockSettings      blks[DMA_BLKS_SIZE];
} local_decryptor; 

static
void uninit_decryptor( local_decryptor * pDecryptor)
{
    if( pDecryptor)
    {
        if(pDecryptor->keySlot) NEXUS_Security_FreeKeySlot(pDecryptor->keySlot);
        if(pDecryptor->cryptoHandle) CommonCrypto_Close(pDecryptor->cryptoHandle);
    }
}

static
int init_decryptor( local_decryptor * pDecryptor)
{
    CommonCryptoSettings           cryptoSettings;
    NEXUS_SecurityKeySlotSettings  keySlotSettings;
    CommonCryptoKeyConfigSettings  algSettings;
    int                            rc = 0;

    if( pDecryptor == NULL) {
        rc = -1;
        goto ErrorExit;
    }

    CommonCrypto_GetDefaultSettings(&cryptoSettings);
    pDecryptor->cryptoHandle = CommonCrypto_Open(&cryptoSettings);
    if( pDecryptor->cryptoHandle == NULL) {
        rc = -1;
        goto ErrorExit;
    }

    /* Allocate key slot for AES Counter mode */
    NEXUS_Security_GetDefaultKeySlotSettings(&keySlotSettings);
    keySlotSettings.keySlotEngine = NEXUS_SecurityEngine_eM2m;

    pDecryptor->keySlot = NEXUS_Security_AllocateKeySlot(&keySlotSettings);
    if(pDecryptor->keySlot == NULL) {
        printf("Failure to allocate key slot for decryptor\n");
        rc = -1;
        goto ErrorExit;
    }
     
    CommonCrypto_GetDefaultKeyConfigSettings(&algSettings);
    algSettings.keySlot = pDecryptor->keySlot;  
    algSettings.settings.opType = NEXUS_SecurityOperation_eDecrypt;
    algSettings.settings.algType = NEXUS_SecurityAlgorithm_eAes128;
    algSettings.settings.algVariant = NEXUS_SecurityAlgorithmVariant_eCounter;
    algSettings.settings.termMode = NEXUS_SecurityTerminationMode_eClear;
    algSettings.settings.enableExtKey = true;
    algSettings.settings.enableExtIv = true;	
    /* always assume IV size is 8 */
    algSettings.settings.aesCounterSize = NEXUS_SecurityAesCounterSize_e64Bits;

    /* Configure key slot for AES Counter mode */
    if(CommonCrypto_LoadKeyConfig( pDecryptor->cryptoHandle, &algSettings) != NEXUS_SUCCESS) {
        printf("CommonCrypto_ConfigAlg failed aes ctr\n");
        rc = -1;
    }
ErrorExit:
    if( rc != 0) {
        uninit_decryptor(pDecryptor);
    }
    BDBG_MSG(("%s - exiting, rc %d", __FUNCTION__, rc));
    return rc;
}

int decrypt_sample( app_ctx_t app, uint32_t sampleSize, batom_cursor * cursor, bdrm_mp4_box_se_sample *pSample, uint32_t *bytes_processed )
{
    uint32_t                 rc=0;
    uint32_t                 nb_Blks = 1; /* The first block is for external key and iv */
    uint8_t                 *pBuf = NULL;
    uint8_t                  ii;
    local_decryptor          decryptor;  
    CommonCryptoJobSettings  jobSettings;
    uint8_t                 *pIv = pSample->iv;
    size_t bytes_ref;
    uint32_t nb_blks_used;

    *bytes_processed = 0;  

    rc = init_decryptor(&decryptor);
    if( rc != 0) {
        printf("failed to initialize the decryptor, can't continue.");
        goto ErrorExit; 
    }
    /*
    if(pSample->nbOfEntries == 0){ 
        printf("number of entries is zero, nothing to decrypt");
        goto ErrorExit;
    }
    */

    /* allocate the memory for the IV */
    rc = NEXUS_Memory_Allocate(KEY_IV_BUFFER_SIZE, NULL, (void *)&pBuf);
    if(rc != NEXUS_SUCCESS){
        printf("NEXUS_Memory_Allocate failed, rc = %d\n",rc);
        goto ErrorExit;
    }

    /* decrypt using external key and IV.  Scatter and gather must be used for 
     * external key and IV to work 
     */
    BKNI_Memcpy(pBuf, g_contentKey+8, 8);	/* Copy key.  H and L need to be swapped */
    BKNI_Memcpy(pBuf+8, g_contentKey, 8);	
    BKNI_Memcpy(&pBuf[16], pIv, 16);
    BKNI_Memset(&pBuf[16], 0, 8);
    
    /* Set Key and IV */
    NEXUS_DmaJob_GetDefaultBlockSettings(&decryptor.blks[0]);
    decryptor.blks[0].pSrcAddr  = pBuf;
    decryptor.blks[0].pDestAddr = pBuf;
    decryptor.blks[0].blockSize = AES_EKL_SIZE;
    decryptor.blks[0].resetCrypto = true;
    decryptor.blks[0].scatterGatherCryptoStart = true;

    NEXUS_FlushCache(decryptor.blks[0].pSrcAddr, AES_EKL_SIZE);

    switch( app->track_type[app->fragment_header.track_ID])
    {
        case eAUDIO:
        {
            *bytes_processed += sampleSize;
            bytes_ref = bdrm_map_cursor_to_dmablks(
                                    (batom_cursor *)cursor, 
                                    sampleSize,
                                    &decryptor.blks[nb_Blks], 
                                    DMA_BLK_POOL_SIZE - 1, 
                                    &nb_blks_used);

            if(bytes_ref != sampleSize){
                BDBG_ERR(("%s - out of DMA blks, decryption failed.", __FUNCTION__));
                rc = -1;
                goto ErrorExit;
            }
            nb_Blks += nb_blks_used;
            break;
        }
        case eVIDEO:
        {
            for(ii = 0; ii <  pSample->nbOfEntries; ii++){
                uint32_t num_clear = pSample->entries[ii].bytesOfClearData;
                uint32_t num_enc = pSample->entries[ii].bytesOfEncData;
                *bytes_processed  += num_clear + num_enc;
                batom_cursor_skip((batom_cursor *)cursor, num_clear);
                bytes_ref = bdrm_map_cursor_to_dmablks(
                                    (batom_cursor *)cursor, 
                                    num_enc,
                                    &decryptor.blks[nb_Blks], 
                                    DMA_BLK_POOL_SIZE - 1, 
                                    &nb_blks_used);

                if(bytes_ref != num_enc){
                    BDBG_ERR(("%s - out of DMA blks, decryption failed.", __FUNCTION__));
                    rc = -1;
                    goto ErrorExit;
                }
                nb_Blks += nb_blks_used;
            } /*for*/
            break;
        }
        default:
        {
            fprintf(stderr,"Detected unknown type of fragment, can't continues\n");
            goto ErrorExit;
        }
    }/* switch */

    decryptor.blks[nb_Blks-1].scatterGatherCryptoEnd = true;

    CommonCrypto_GetDefaultJobSettings(&jobSettings);
    jobSettings.keySlot = decryptor.keySlot;

    rc = CommonCrypto_DmaXfer(decryptor.cryptoHandle, &jobSettings, decryptor.blks, nb_Blks);
    if(rc != NEXUS_SUCCESS) {
        BDBG_ERR(("%s - CommonCrypto_DmaXfer failed rc %x\n", __FUNCTION__, rc));
    }

ErrorExit:
    if( pBuf)
        NEXUS_Memory_Free(pBuf);
    if( decryptor.keySlot)
        NEXUS_Security_FreeKeySlot(decryptor.keySlot);
    if( decryptor.cryptoHandle)
        CommonCrypto_Close(decryptor.cryptoHandle);
    return rc;
}

/* Function to decrypt payload manually, it is used only for the initial testing.
 * This function is no longer  used as it has been replaced by using 
 * PlayReady DRM through the playpump.
 */
static
int decrypt_fragment(app_ctx_t app, batom_cursor * cursor, uint32_t payload_size, bdrm_mp4_se *pSamples_enc)
{
    int      rc = 0;
    uint32_t ii,bytes_processed;
    bdrm_mp4_box_se_sample *pSample;

    bytes_processed = 0;
    if(pSamples_enc->sample_count != 0){
        for(ii = 0; ii < pSamples_enc->sample_count; ii++){
            uint32_t numOfByteDecrypted = 0;
            uint32_t sampleSize = 0;
            pSample = &pSamples_enc->samples[ii]; 
            sampleSize = app->samples[ii].size;
            if(decrypt_sample(app, sampleSize,cursor,pSample,&numOfByteDecrypted)!=0) {
                printf("Failed to decrypt sample, can't continue...\n");
                return -1;
                break;
            } 
            bytes_processed += numOfByteDecrypted;
        }
    }
    if( bytes_processed != payload_size) {
        printf("the number of bytes %d decrypted doesn't match the actual size %d of the payload, return failure...\n",bytes_processed,payload_size);
        rc = -1;
    }

    return rc;
}
#endif

#undef BMP4_FRAGMENT_PRIMITIVES_ONLY
#define BMP4_FRAGMENT_MAKE_EMBEDDED 1
#define FILE mem_file
#define fwrite(p,s,n,f) mem_file_write(f,p,(s*n))
#define fseek(f, o, w) mem_file_seek(f, o)
#define ftell(f) mem_file_tell(f)
#include "bmp4_fragment_make_segment.c"
#undef FILE
#undef fwrite
#undef fseek
#undef ftell

static int send_fragment_data(
        uint8_t *pData,
        uint32_t dataSize,
        uint8_t trackId,
        uint64_t start_time,
        NEXUS_PlaypumpHandle playpump, 
        BKNI_EventHandle event, 
        const struct decoder_configuration *avc_config, 
        const char *fourcc)
{
    NEXUS_PlaypumpStatus playpumpStatus;
    size_t offset;
    uint64_t timescale = 90000;
    mem_file fout;
    size_t fragment_size = dataSize;
    void *playpumpBuffer;
    size_t bufferSize;

    /*printf("Playing fragment size %d \n",dataSize); */
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
        if((uint8_t *)playpumpBuffer >= (uint8_t *)playpumpStatus.bufferBase +  (playpumpStatus.fifoSize - fragment_size)) {
            NEXUS_Playpump_WriteComplete(playpump, bufferSize, 0); /* skip buffer what wouldn't be big enough */
        }
    }
    fout.buf = playpumpBuffer;
    fout.buf_len = bufferSize;
    mem_file_seek(&fout, 0);
    fout.length = 0;

    offset = start_mp4_box(&fout, "bmp4");
    /*write_bhed_box(&fout, timescale, 4551460, fourcc); */
    write_bhed_box(&fout, timescale, start_time, fourcc);
    write_bdat_box(&fout, avc_config->data, avc_config->size);
    write_trex_box(&fout, trackId, 0, 0, 0, 0);
    write_data(&fout, pData, dataSize);
    finish_mp4_box(&fout, offset);
    NEXUS_Playpump_WriteComplete(playpump, 0, fout.length); /* skip buffer what wouldn't be big enough */
    return 0;
}

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
        if(rc!=NEXUS_SUCCESS) {
            break;
        }
        if(playpumpStatus.fifoDepth==0) {
            break;
        }
        BKNI_Sleep(100);
    }
    for(;;) {
        rc=NEXUS_Playpump_GetStatus(audioPlaypump, &playpumpStatus);
        if(rc!=NEXUS_SUCCESS) {
            break;
        }
        if(playpumpStatus.fifoDepth==0) {
            break;
        }
        BKNI_Sleep(100);
    }
    if(videoDecoder) {
        for(;;) {
            NEXUS_VideoDecoderStatus decoderStatus;
            rc=NEXUS_VideoDecoder_GetStatus(videoDecoder, &decoderStatus);
            if(rc!=NEXUS_SUCCESS) {
                break;
            }
            if(decoderStatus.queueDepth==0) {
                break;
            }
            BKNI_Sleep(100);
        }
    }
    if(audioDecoder) {
        for(;;) {
            NEXUS_AudioDecoderStatus decoderStatus;
            rc=NEXUS_AudioDecoder_GetStatus(audioDecoder, &decoderStatus);
            if(rc!=NEXUS_SUCCESS) {
                break;
            }
            if(decoderStatus.queuedFrames<4) {
                break;
            }
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
    wait_for_drain(videoPlaypump, videoDecoder, audioPlaypump, audioDecoder);
    if(videoDecoder) {
        NEXUS_VideoDecoder_Stop(videoDecoder);
        NEXUS_Playpump_ClosePidChannel(videoPlaypump, videoPidChannel);
        NEXUS_Playpump_Stop(videoPlaypump);
        NEXUS_StopCallbacks(videoPlaypump);
        NEXUS_VideoDecoder_Close(videoDecoder);
    }
    if(audioDecoder) {
        NEXUS_AudioDecoder_Stop(audioDecoder);
        NEXUS_Playpump_ClosePidChannel(audioPlaypump, audioPidChannel);
        NEXUS_Playpump_Stop(audioPlaypump);
        NEXUS_StopCallbacks(audioPlaypump);
        NEXUS_AudioDecoder_Close(audioDecoder);
    }
    NEXUS_Playpump_Close(videoPlaypump); 
    NEXUS_Playpump_Close(audioPlaypump); 
    BKNI_DestroyEvent(event);
    NEXUS_VideoWindow_Close(window);
    NEXUS_Display_Close(display);
    return 0;
}

bool find_box_in_file(batom_factory_t factory, FILE *fp, uint8_t *pBuf, uint32_t box_type, batom_t *box, uint32_t *remaining_bytes, uint32_t *box_size)
{
    uint32_t curr_box_size;
    uint32_t curr_box_type = 0;
    uint32_t size_to_read;
    bool found = false;
    batom_cursor cursor;

    uint32_t size = 0;

    while(curr_box_type != box_type){

        size_to_read = BOX_HEADER_SIZE; /* Read box header */
        size = fread(pBuf, 1, size_to_read, fp);
        if(size != size_to_read) {
            fprintf(stderr,"fread failed\n");
            break;
        }

        (*box) = batom_from_range(factory, pBuf, size_to_read, NULL, NULL);
        batom_cursor_from_atom(&cursor, (*box));

        curr_box_size = batom_cursor_uint32_be(&cursor);
        curr_box_type = batom_cursor_uint32_be(&cursor);
        *box_size     = curr_box_size;
        batom_cursor_skip(&cursor, curr_box_size - BOX_HEADER_SIZE);

        /* printf("%s - box curr_box_size %d\n", __FUNCTION__, curr_box_size); */
        /* printf("%s - curr_box_type" B_MP4_TYPE_FORMAT"\n", __FUNCTION__, B_MP4_TYPE_ARG(curr_box_type)); */
        /* printf("current box type " B_MP4_TYPE_FORMAT"\n",B_MP4_TYPE_ARG(curr_box_type)); */
        if(curr_box_type != box_type){
            /* Go to next box 
            printf("not the box, skip to next box...\n");*/
            fseek(fp, curr_box_size - BOX_HEADER_SIZE, SEEK_CUR);
        }
        else {
            size_to_read = curr_box_size - BOX_HEADER_SIZE;
            size = fread(pBuf + BOX_HEADER_SIZE, 1, size_to_read, fp);
            /* printf("Read the rest of the box ==> need to read %d, buffer size %d, size read %d\n",size_to_read,BUF_SIZE,size); */
            if(size != size_to_read) {
                fprintf(stderr,"fread failed\n");
                break;
            }

            (*box) = batom_from_range(factory, pBuf, curr_box_size, NULL, NULL);
            found = true;
           
        }

        (*remaining_bytes) -= curr_box_size;
        /* printf("remaining_bytes %d\n", (*remaining_bytes)); */
        if((*remaining_bytes)  == 0){
            /* EOF has been reached */
            break;
      }
    }
    return found;
}

/* Find a box in the cursor's data */
bool find_box_cursor(app_ctx_t ctx, uint32_t box_type, batom_cursor *cursor)
{
    uint32_t curr_box_size;
    uint32_t curr_box_type = 0;
    bool found = false;
    batom_checkpoint checkpoint;
    batom_cursor_skip(cursor, BOX_HEADER_SIZE);

    BSTD_UNUSED(ctx);
    while(curr_box_type != box_type){  
        batom_cursor_save(cursor, &checkpoint);

        curr_box_size= batom_cursor_uint32_be(cursor);
        printf("t %x\n", curr_box_size);
        curr_box_type = batom_cursor_uint32_be(cursor);
        printf("curr_box_type" B_MP4_TYPE_FORMAT"\n", B_MP4_TYPE_ARG(curr_box_type));
        if(curr_box_type != box_type){
            /* Go to next box */
            batom_cursor_skip(cursor, curr_box_size - BOX_HEADER_SIZE);
        }else{
            batom_cursor_rollback(cursor, &checkpoint);
            found = true;
            break;
        }
    }

    return found;
}

bool get_box_for_type( batom_cursor *cursor, bmp4_box parent_box, uint32_t target_box_type)
{
    bmp4_box      target_box;
    size_t        box_hdr_size = 8;
    uint32_t      ii;
    bool          rc = false;

    /*
    printf("%s - entering\n", __FUNCTION__);
    printf("Inside " B_MP4_TYPE_FORMAT, B_MP4_TYPE_ARG(parent_box.type)); 
    printf("and look for box " B_MP4_TYPE_FORMAT"\n",B_MP4_TYPE_ARG(target_box_type));
    */

    for(ii = 0; ii < parent_box.size; ii += (box_hdr_size + target_box.size)) {    
        batom_checkpoint target_start;
        batom_cursor_save(cursor, &target_start);
        box_hdr_size = bmp4_parse_box(cursor, &target_box);
        /*
        printf("box.size %u ", (uint32_t)target_box.size);
        printf(" type " B_MP4_TYPE_FORMAT"\n",B_MP4_TYPE_ARG(target_box.type));
        */
        if(box_hdr_size==0) {
            printf("%s empty box - %d\n", __FUNCTION__, __LINE__);
            break;
        }
        if( target_box.type == target_box_type) {
            /* printf("found the box " B_MP4_TYPE_FORMAT"\n",B_MP4_TYPE_ARG(target_box.type)); */
            rc = true;
            batom_cursor_rollback(cursor, &target_start); 
            break; /* get out from the for loop */
        }
        else {
            /* Not the box we are looking for */ 
            batom_cursor_skip(cursor, target_box.size - box_hdr_size);
        }
    } /* for */

    /* printf("%s - exiting, rc %d\n", __FUNCTION__, rc); */
    return rc;
}

bool app_parse_sample_enc(app_ctx_t app, batom_cursor *cursor, bmp4_box senc_box)
{
    uint32_t     ii,jj; 
    bmp4_fullbox fullbox;
    bdrm_mp4_box_se_sample *pSample;
    bdrm_mp4_box_se_entry *pEntry;

    BSTD_UNUSED(senc_box); 
    BDBG_ASSERT(cursor != NULL);

    if(bmp4_parse_fullbox(cursor, &fullbox)){
        app->samples_enc.sample_count = batom_cursor_uint32_be(cursor);
        if(app->samples_enc.sample_count != 0){
            if(app->samples_enc.sample_count > SAMPLES_POOL_SIZE){
                printf("Sample pools too small, increase SAMPLES_POOL_SIZE, number of samples: %d\n",app->samples_enc.sample_count);
                return false; 
            }
            pSample = app->samples_enc.samples; 
            for(ii = 0; ii < app->samples_enc.sample_count; ii++){
                uint8_t *pIv;
                batom_cursor_copy(cursor, &pSample->iv[8], 8);
                BKNI_Memset(&pSample->iv[0], 0, 8);
                 
                /* printf("Sample[%d] ",ii); */
                pIv = &app->samples_enc.samples[ii].iv[0];
                /*
                 DUMP_DATA_HEX("iv ", pIv,8); 
                printf("\n"); fflush(stdout); 
                */
                
                if(fullbox.flags & 0x000002){ 
                    pSample->nbOfEntries = batom_cursor_uint16_be(cursor);
                    if(pSample->nbOfEntries != 0){ 
                        if(pSample->nbOfEntries  > MAX_ENTRIES_PER_SAMPLE){
                            BDBG_ERR(("%s - Default nb of entry too small, increase MAX_ENTRIES_PER_SAMPLE %x\n", __FUNCTION__));
                            return false;
                        }
                        pEntry = pSample->entries;
                        for(jj = 0; jj <  pSample->nbOfEntries; jj++){
                            pEntry->bytesOfClearData = batom_cursor_uint16_be(cursor);
                            pEntry->bytesOfEncData = batom_cursor_uint32_be(cursor);
                            pEntry++;
                            /*
                            printf("Entries[%d] numOfClearData %d ", jj, app->samples_enc.samples[ii].entries[jj].bytesOfClearData);
                            printf("numOfEncData %d\n", app->samples_enc.samples[ii].entries[jj].bytesOfEncData);
                            */
                        } /*for*/
                    }
                }
                pSample++;
            } /* for */
        }
    }

    return true;
}

bool app_parse_traf(app_ctx_t app, batom_cursor *cursor, bmp4_box traf)
{
    bmp4_trackextendsbox     track_extends;
    bmp4_box      box;
    size_t        box_hdr_size = 8;
    uint32_t      ii, jj;
    bool rc = true;

    /*
    printf("%s - entering\n", __FUNCTION__);
    */

    if(traf.type == BMP4_TRACK_FRAGMENT)
    {
        bool skipThisFrg = false;
        /* printf("%s - BMP4_TRACK_FRAGMENT\n", __FUNCTION__); */
        for(ii = 0; ii < traf.size; ii += (box_hdr_size + box.size))
        {    
            box_hdr_size = bmp4_parse_box(cursor, &box);
            if(box_hdr_size==0) {
                printf("%s - %d\n", __FUNCTION__, __LINE__);
                break;
            }

            switch(box.type){
                case BMP4_TRACK_FRAGMENT_HEADER:
                {
                    uint8_t trackId = 0;
                    /* printf("calling bmp4_parse_track_fragment_header\n"); */
                    bmp4_parse_track_fragment_header(cursor, &app->fragment_header);
                    trackId = (uint8_t)app->fragment_header.track_ID; 
                    if( (app->track_type[trackId] != eAUDIO && app->track_type[trackId] != eVIDEO)) {
                        printf("Detected an unknown type of fragment, skip this fragment...\n");
                        skipThisFrg = true;
                    }
                    break;
                }
                case BMP4_TRACK_FRAGMENT_RUN:
                    /* printf("calling bmp4_parse_track_fragment_run_header\n"); */
                    bmp4_parse_track_fragment_run_header(cursor, &app->run_header);

                    /* printf("app->run_header.sample_count %d\n", app->run_header.sample_count); */
                    if(app->run_header.sample_count > MAX_SAMPLES){
                        printf("we don't have enough samples desc in the pool to store all the samples in the fragment\n");
                        goto ErrorExit;
                    }

                    for(jj = 0; jj < app->run_header.sample_count; jj++){
                        bmp4_parse_track_fragment_run_sample(cursor,
                                                            &app->fragment_header,
                                                            &app->run_header, 
                                                            &track_extends,
                                                            &app->state, 
                                                            &app->samples[jj]);
                    }
                    break;
                case BMP4_CENC_SAMPLEENCRYPTION:
                    if( !app_parse_sample_enc(app, cursor, box)) {
                        printf("Failed to parse sample enc box, can't continue...\n");
                        rc = false;
                        goto ErrorExit;
                    }
                    app->senc_parsed = true;
                    break;
                default :
                    /* Not the box we are looking for. Skip over it.*/
                    batom_cursor_skip(cursor, box.size - box_hdr_size);  
                    break;
            }

            if( skipThisFrg) {
               printf("not a video track, skip over to next moof box...\n");
               rc = false;
               break; 
            }
        } /* for */

        if( rc && !app->senc_parsed)
        {
            printf("no sample enc box is found, the media may be unprotected...\n");
        } 
    }
    else {
        /* Note a TRAF box */
        rc = false;
    }

ErrorExit:
    /* printf("%s - exiting, rc %d\n", __FUNCTION__, rc); */
    return rc;
}

bool app_parse_trak(app_ctx_t app, batom_cursor *cursor, bmp4_box trak_box)
{
    batom_checkpoint trak_start;
    uint32_t         trackId = 0;
    size_t           box_hdr_size = 8;
    bool             rc = false;

    batom_cursor_save(cursor, &trak_start);
    /* printf("%s - entering\n", __FUNCTION__); */

    if(trak_box.type == BMP4_TRACK) {
        if( get_box_for_type(cursor,trak_box,BMP4_TRACKHEADER )) {
           bmp4_box tkhd_box;
           bmp4_fullbox fullbox;
           box_hdr_size = bmp4_parse_box(cursor, &tkhd_box);
           box_hdr_size = bmp4_parse_fullbox(cursor, &fullbox);
           if( fullbox.version == 1)
               batom_cursor_skip(cursor, 16);/* skip 16 bytes */
           else
               batom_cursor_skip(cursor, 8);/* skip 8 bytes */
           trackId = batom_cursor_uint32_be(cursor);
           /* printf("%s - This track's ID is  = %d\n", __FUNCTION__,trackId); */
           batom_cursor_rollback(cursor, &trak_start);
        }

        /* printf("find the mida box \n"); */
        if( get_box_for_type(cursor,trak_box,BMP4_MEDIA)) {
           batom_checkpoint mdia_start;
           bmp4_box         mdia_box;

           box_hdr_size = bmp4_parse_box(cursor, &mdia_box);
           /* printf("%s - Got the mdia box, find the minf box\n", __FUNCTION__); */
           batom_cursor_save(cursor, &mdia_start);

           /* get the hdlr box to find out if this is a video or audio track */
           if( get_box_for_type(cursor,mdia_box,BMP4_HANDLER)) {
               uint32_t trackType;
               bmp4_box hdlr_box;
               bmp4_fullbox fullbox;
               box_hdr_size = bmp4_parse_box(cursor, &hdlr_box);
               box_hdr_size = bmp4_parse_fullbox(cursor, &fullbox);
               batom_cursor_skip(cursor, 4);/* skip 4 bytes */
               /* get the track type */
               trackType = batom_cursor_uint32_be(cursor); 
               /*printf("track type : " B_MP4_TYPE_FORMAT"\n", B_MP4_TYPE_ARG(trackType));*/ 
               if( trackType == BMP4_TYPE('s','o','u','n')) {
                   /*printf("Found track %d : audio\n",trackId); */
                   app->track_type[trackId] = eAUDIO;
               }
               else if( trackType == BMP4_TYPE('v','i','d','e')) {
                   /*printf("Found track %d : video\n",trackId); */
                   app->track_type[trackId] = eVIDEO;
               }
               else {
                   printf("Detected an unknown track type, skip this track...\n"); 
               }
               batom_cursor_rollback(cursor, &mdia_start);
           }

           if( get_box_for_type(cursor,mdia_box,BMP4_MEDIAINFORMATION)) {
               bmp4_box minf_box;
               box_hdr_size = bmp4_parse_box(cursor, &minf_box);
               /* printf("%s - Got the minf box, find the stbl box\n", __FUNCTION__); */

               if( get_box_for_type(cursor,minf_box,BMP4_SAMPLETABLE)) {
                   bmp4_box stbl_box;
                   box_hdr_size = bmp4_parse_box(cursor, &stbl_box);
                   /* printf("%s - Got the stbl box, find the stsd box\n", __FUNCTION__); */
                  
                   if( get_box_for_type(cursor,stbl_box,BMP4_SAMPLEDESCRIPTION)) {
                       bmp4_box stsd_box;
                       bmp4_fullbox fullbox;
                       bmp4_box encv_box;
                       box_hdr_size = bmp4_parse_box(cursor, &stsd_box);
                       box_hdr_size = bmp4_parse_fullbox(cursor, &fullbox);
                       /* printf("%s - Got the stsd full box, find the encv box\n", __FUNCTION__); */
                       batom_cursor_skip(cursor, 4);/* skip 4 bytes */
                       box_hdr_size = bmp4_parse_box(cursor, &encv_box);

                       /* printf("Checking for box with type " B_MP4_TYPE_FORMAT"\n",B_MP4_TYPE_ARG(encv_box.type)); */
                       if( encv_box.type == BMP4_SAMPLE_ENCRYPTED_VIDEO) {
                           bmp4_box avcC_box;
                           /* printf("%s - Got the encv box, find the avcC box\n", __FUNCTION__); */
                           batom_cursor_skip(cursor, 78); /* skips 78 bytes to get the "avcC' box */
                           box_hdr_size = bmp4_parse_box(cursor, &avcC_box);

                           /* printf("Checking for box with type " B_MP4_TYPE_FORMAT"\n",B_MP4_TYPE_ARG(avcC_box.type)); */
                           if( avcC_box.type == BMP4_CENC_AVCC) {
                               /* printf("%s - Got the AvcC box, read the avc_config data\n", __FUNCTION__); */
                               batom_cursor_copy(cursor, app->avc_config.data, avcC_box.size-box_hdr_size);
                               app->avc_config.size = avcC_box.size-box_hdr_size;
                               rc = true;
                               /* may need to get the KID */
                           }
                       }
                       else if( encv_box.type == BMP4_SAMPLE_AVC) {
                           /* non-encrypted version */
                           bmp4_box avcC_box;
                           /*printf("%s - Got the avc1 box, find the avcC box\n", __FUNCTION__); */ 
                           batom_cursor_skip(cursor, 78); /* skips 78 bytes to get the "avcC' box */
                           box_hdr_size = bmp4_parse_box(cursor, &avcC_box);

                           /* printf("Checking for box with type " B_MP4_TYPE_FORMAT"\n",B_MP4_TYPE_ARG(avcC_box.type)); */
                           if( avcC_box.type == BMP4_CENC_AVCC) {

                               /* printf("%s - Got the AvcC box, read the avc_config data\n", __FUNCTION__); */
                               batom_cursor_copy(cursor, app->avc_config.data, avcC_box.size-box_hdr_size);
                               app->avc_config.size = avcC_box.size-box_hdr_size;
                               app->video_encrypted = false;
                               rc = true;
                               /* may need to get the KID */
                           }
                       }
                       else if( encv_box.type == BMP4_SAMPLE_ENCRYPTED_AUDIO ) {
                           bmp4_box esds_box;
                           /*printf("%s - Got the enca box, find the avcC box\n", __FUNCTION__); */ 
                           batom_cursor_skip(cursor, 28); /* skips 28 bytes to get the "esds' box */
                           box_hdr_size = bmp4_parse_box(cursor, &esds_box);

                           /*printf("Checking for box with type " B_MP4_TYPE_FORMAT"\n",B_MP4_TYPE_ARG(esds_box.type)); */ 
                           if( esds_box.type == BMP4_CENC_ESDS ) {
                               /*printf("%s - Got the esds box, read the aac_config data\n", __FUNCTION__); */
                               batom_cursor_copy(cursor, app->aac_config.data, esds_box.size-box_hdr_size);
                               app->aac_config.size = esds_box.size-box_hdr_size;
                               rc = true;
                               /* may need to get the KID */
                           }
                       }
                       else if( encv_box.type == BMP4_SAMPLE_MP4A ) {
                           /* non-encrypted version */
                           bmp4_box esds_box;
                           /*printf("%s - Got the mp4a box, find the avcC box\n", __FUNCTION__);*/ 
                           batom_cursor_skip(cursor, 28); /* skips 28 bytes to get the "esds' box */
                           box_hdr_size = bmp4_parse_box(cursor, &esds_box);

                           /*printf("Checking for box with type " B_MP4_TYPE_FORMAT"\n",B_MP4_TYPE_ARG(esds_box.type)); */
                           if( esds_box.type == BMP4_CENC_ESDS ) {
                               /*printf("%s - Got the esds box, read the aac_config data\n", __FUNCTION__); */
                               batom_cursor_copy(cursor, app->aac_config.data, esds_box.size-box_hdr_size);
                               app->aac_config.size = esds_box.size-box_hdr_size;
                               app->audio_encrypted = false;
                               rc = true;
                               /* may need to get the KID */
                           }
                       }
                       /*
                       else {
                           printf("Not " B_MP4_TYPE_FORMAT", skip...\n",B_MP4_TYPE_ARG(BMP4_TYPE('e','n','c','v')));

                       } */ /* encv */
                   } /* stsd */
               } /* stbl */
           } /* minf */ 
        } /* mdia */
    } /* trak */
    
    /* printf("%s - exiting, rc %d\n", __FUNCTION__, rc); */
    return rc;
}

bool app_parse_moov(app_ctx_t app, batom_t atom)
{
    batom_cursor  cursor;
    bmp4_box      box;
    bmp4_box      moov;
    size_t        box_hdr_size;
    uint32_t      ii;
    batom_checkpoint trak_payload;
    bool           rc = false;
    /* 
    printf("%s - entering\n", __FUNCTION__);
    */
    if(atom == NULL)
        return false;
    
    batom_cursor_from_atom(&cursor, atom);

    if(bmp4_parse_box(&cursor, &moov)){
        /* 
        printf("Box size %u",(uint32_t)moov.size);
        printf("with type " B_MP4_TYPE_FORMAT"\n",B_MP4_TYPE_ARG(moov.type));
        */
        if(moov.type == BMP4_MOVIE) {
            for(ii = 0; ii < moov.size; ii += (box_hdr_size + box.size)){
                /* printf("ii %d\n", ii); */
                box_hdr_size = bmp4_parse_box(&cursor, &box);
                batom_cursor_save(&cursor, &trak_payload);
                /* printf("app_parse_moov - checking box "B_MP4_TYPE_FORMAT"\n", B_MP4_TYPE_ARG(box.type)); */
                if(box_hdr_size==0) {
                    break;
                }
                if(box.type == BMP4_TRACK) {
                    /* printf("Found the Trak box "B_MP4_TYPE_FORMAT"\n",B_MP4_TYPE_ARG(box.type) ); */
                    rc = app_parse_trak(app, &cursor, box);
                    if(!rc) {
                        fprintf(stderr, "Failed to parse track box, exiting...\n");
                        break;
                    }
                }
                else {
                    /* Not the box we are looking for. Skip over it.*/
                    batom_cursor_skip(&cursor, box.size - box_hdr_size);
                }
            } /* for */

            if( rc == true) {
                /* printf("%s - We are done\n", __FUNCTION__); */
            }
            batom_cursor_rollback(&cursor, &trak_payload); 
             
        }
        else {
            /* printf("%s - Not a moov box????\n", __FUNCTION__); */
        }
    }

    /* printf("%s - exiting, rc %d\n", __FUNCTION__, rc); */
    return rc;
}

bool app_parse_moof(app_ctx_t app, batom_t atom)
{
    batom_checkpoint traf_start;
    batom_checkpoint traf_payload;
    batom_cursor     cursor;
    bmp4_box         box;
    bmp4_box         moof;
    size_t           box_hdr_size;
    uint32_t         ii;
    bool             rc = true;
    
    /*
    printf("%s - entering\n", __FUNCTION__);
    */

    if(atom == NULL)
        return false;
    
    batom_cursor_from_atom(&cursor, atom);

    if(bmp4_parse_box(&cursor, &moof)){
        if(moof.type == BMP4_MOVIE_FRAGMENT)
        {
            /*
            printf("moof.size %u\n",(uint32_t)moof.size);
            */
            for(ii = 0; ii < moof.size; ii += (box_hdr_size + box.size)){
                batom_cursor_save(&cursor, &traf_start);
                box_hdr_size = bmp4_parse_box(&cursor, &box);
                
                batom_cursor_save(&cursor, &traf_payload);
                /*
                printf("Box size %u",(uint32_t)box.size);
                printf("with type " B_MP4_TYPE_FORMAT"\n",B_MP4_TYPE_ARG(box.type));
                */
                if(box_hdr_size==0) {
                    break;
                }

                if(box.type == BMP4_TRACK_FRAGMENT) {
                    uint32_t traf_len;
                    /* DRM expect the whole traf box, not just the traf box payload*/
                    batom_cursor_rollback(&cursor, &traf_start);                                
                    traf_len = batom_cursor_size(&cursor);
                    /* printf("app_parse_traf traf_len %x\n", traf_len); */
                    /* bdrm_parse_mp4_fragment_context(app->drm, &cursor, box.size); */

                    batom_cursor_rollback(&cursor, &traf_payload);
                    rc = app_parse_traf(app, &cursor, box);
                }
                else {
                    /* Not the box we are looking for. Skip over it.*/
                    batom_cursor_skip(&cursor, box.size - box_hdr_size);
                }
            }
        }
        else {
            /* Note a MOOF box */
            rc = false;
        }
    }
    else {
        /* Not a box */
        rc = false;
    }

    /*
    printf("%s - exiting, rc %d\n", __FUNCTION__, rc);
    */
    return rc;
}
     
void policy_callback(void *context, int param)
{
    BSTD_UNUSED(param);
    printf("policy_callback - event %p\n", context);
}

int 
main(int argc, char* argv[])
{
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_WmDrmPdSettings drmSettings; 
    NEXUS_DmaHandle hDma;
    NEXUS_KeySlotHandle drmKeyHandle_;
    NEXUS_SecurityKeySlotSettings drmKeySettings;

    NEXUS_VideoDecoderHandle videoDecoder; 
    NEXUS_PlaypumpHandle videoPlaypump; 
    NEXUS_PlaypumpHandle audioPlaypump; 
    NEXUS_StcChannelHandle stcChannel;
    NEXUS_VideoWindowHandle window;
    NEXUS_DisplayHandle display; 
    NEXUS_PidChannelHandle videoPidChannel;
    BKNI_EventHandle event;

    NEXUS_AudioDecoderHandle audioDecoder;
    NEXUS_PidChannelHandle audioPidChannel;

#if NEXUS_HAS_PLAYBACK 
#ifdef NEXUS_CLIENT_MODE
    NEXUS_ClientConfiguration clientConfig;
#endif
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_PlaypumpSettings playpumpSettings;
    
    NEXUS_PlaypumpOpenSettings videoplaypumpOpenSettings;
    NEXUS_PlaypumpOpenSettings audioplaypumpOpenSettings;
   
    NEXUS_Error rc;
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputStatus hdmiStatus;
#endif
#endif
    char *piff_file = NULL;
    int ii;
    app_ctx app;
    bool moovBoxParsed = false;
    bdrm_cfg cfg;
    batom_t piff_container;
    uint32_t box_size = 0;

    BSTD_UNUSED(hex2bin);

    for (ii = 1; ii < argc; ii++) {
        if ( !strcmp(argv[ii], "-f") ){
            piff_file = argv[++ii];
        }
    }

    if(piff_file == NULL ){
        usage(); 
        /*fprintf(stderr, "ERROR - missing argument"); */
        goto clean_exit; 
    }

    printf("PIFF file: %s\n",piff_file);
    fflush(stdout);

    BKNI_Memset(&app, 0, sizeof( app_ctx));
    app.video_encrypted = true;
    app.audio_encrypted = true;
    app.senc_parsed = false;
    app.last_video_fragment_duration = 0;
    app.last_audio_fragment_duration = 0;

    app.fp_piff = fopen(piff_file, "rb");
    if(app.fp_piff == NULL){
        fprintf(stderr,"failed to open %s\n", piff_file);
        goto clean_exit;
    }

    fseek(app.fp_piff, 0, SEEK_END);
    app.piff_filesize = ftell(app.fp_piff);
    fseek(app.fp_piff, 0, SEEK_SET);

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);

    /* Open up a DMA channel, it will be used by the security code. */
    hDma = NEXUS_Dma_Open(0, NULL);

    /* Save the DMA handle. */
    bdrm_set_dma_handle( hDma );

    NEXUS_WmDrmPd_GetDefaultSettings(&drmSettings);
    drmSettings.transportType = NEXUS_TransportType_eMp4Fragment; 
    drmSettings.dma = hDma; 

    drmSettings.policyCallback.callback = policy_callback;
    drmSettings.policyCallback.context = NULL;
    drmSettings.policyCallback.param = 0;

    app.drm = NEXUS_WmDrmPd_Create(&drmSettings);
    if( app.drm == NULL)
        printf("NEXUS_WmDrmPd_Create failed!\n");
    else
        printf("NEXUS_WmDrmPd_Create success!\n");
    fflush(stdout);

    NEXUS_Security_GetDefaultKeySlotSettings(&drmKeySettings);
    drmKeySettings.keySlotEngine = NEXUS_SecurityEngine_eGeneric;
    drmKeyHandle_ = NEXUS_Security_AllocateKeySlot(&drmKeySettings);
    NEXUS_WmDrmPd_ConfigureKeySlot(app.drm, drmKeyHandle_);

    if( NEXUS_Memory_Allocate(BUF_SIZE, NULL, (void *)&app.pPayload) !=  NEXUS_SUCCESS) {
        fprintf(stderr,"NEXUS_Memory_Allocate failed");
        goto clean_up;
    }

    app.factory = batom_factory_create(bkni_alloc, 256);

    BKNI_Memset(&cfg, 0, sizeof(cfg));

    cfg.lic_callback = dummy_license_callback;   /* external status callback */
    cfg.lic_callback_ctx = NULL;             /* external status callback context */
    cfg.lic_extern = NULL;       /* external grabber func */   
    cfg.transport = bdrm_default_transport;
    cfg.lic_extern_ctx = NULL;               /* external grabber func context */

    cfg.user_data_store_file_path[0] = 0x00;    /* user default*/
    cfg.ext_asf_scanning = false;

    cfg.cnt_type = bdrm_cnt_mp4;

    if(!find_box_in_file(app.factory, app.fp_piff, app.pPayload, BMP4_MOVIE, &piff_container, &app.piff_filesize,&box_size)) {
        printf("could not find moov\n");
    }
    else {
        /* Set the PSSH box for license acquisition */ 
        uint32_t i;
        uint32_t post_ret;
        uint8_t non_quiet= 1; /*CH_DEFAULT_NON_QUIET;   */ 
        uint32_t app_security= 150; /*CH_DEFAULT_APP_SEC; */
        uint32_t startOffset, length;
        bpiff_mp4_headers drmMp4Handle_;
        NEXUS_WmDrmPdLicenseChallenge challenge;
        bpiff_GetDefaultMp4Header(&drmMp4Handle_);
        bpiff_parse_moov(&drmMp4Handle_, piff_container);
        NEXUS_WmDrmPd_SetPsshBox(app.drm, &drmMp4Handle_.psshSystemId, drmMp4Handle_.pPsshData, drmMp4Handle_.psshDataLength);
        printf("NbOfSchemes = %d\n",drmMp4Handle_.nbOfSchemes);
        for(i = 0; i < drmMp4Handle_.nbOfSchemes; i++){
            NEXUS_WmDrmPd_SetProtectionSchemeBox(app.drm, &drmMp4Handle_.scheme[i]);
        }

        bpiff_FreeMp4Header(&drmMp4Handle_);

        if ( NEXUS_WmDrmPd_LoadLicense(app.drm) != NEXUS_SUCCESS){
            NEXUS_WmDrmPd_GetLicenseChallenge(app.drm, &challenge);
#if PLAYREADY_DRM
            post_ret = bhttpclient_license_post_soap((char *)challenge.pUrl, 
                                                     (char *)challenge.pData, 
                                                     non_quiet, app_security, 
                                                     (unsigned char**)&(challenge.pResponseBuffer), 
                                                     &startOffset, 
                                                     &length);
#else
            post_ret = bhttpclient_license_post_default((char *)challenge.pUrl, 
                                                        (char *)challenge.pData, 
                                                        non_quiet, 
                                                        app_security, 
                                                        (unsigned char**)&(challenge.pResponseBuffer), 
                                                        &startOffset, 
                                                        &length);
#endif
            if (post_ret == 0) {
                challenge.pResponseBuffer[length] = '\0';
                /*printf("Response <<%s>>\n", challenge.pResponseBuffer);*/
                NEXUS_WmDrmPd_LicenseChallengeComplete( app.drm, length, startOffset);
            }
            else {
                printf("Error doing post \n");
                goto clean_up;
            }
        }
         
        /* we parse the moov box again (own parser here) to get the video and audio media configurations */
        fseek(app.fp_piff, 0, SEEK_END);
        app.piff_filesize = ftell(app.fp_piff);
        fseek(app.fp_piff, 0, SEEK_SET);
        find_box_in_file(app.factory, app.fp_piff, app.pPayload, BMP4_MOVIE, &piff_container, &app.piff_filesize,&box_size);
        moovBoxParsed = app_parse_moov(&app, piff_container);
    }

    if(!moovBoxParsed) {
        printf("Failed to parse moov box, can't continue...\n");
        goto clean_up;
    }

    printf("Successfully parsed the moov box, continue...\n\n");

    /* EXTRACT AND PLAYBACK THE MDAT */
    NEXUS_Playpump_GetDefaultOpenSettings(&videoplaypumpOpenSettings);
    videoplaypumpOpenSettings.fifoSize *= 7; 
    videoplaypumpOpenSettings.numDescriptors *= 7; 
    NEXUS_Playpump_GetDefaultOpenSettings(&audioplaypumpOpenSettings);
#ifdef NEXUS_CLIENT_MODE
    NEXUS_Platform_GetClientConfiguration(&clientConfig);
    videoplaypumpOpenSettings.heap = clientConfig.heap[1];
    videoplaypumpOpenSettings.boundsHeap = clientConfig.heap[1];
    audioplaypumpOpenSettings.heap = clientConfig.heap[1];
    audioplaypumpOpenSettings.boundsHeap = clientConfig.heap[1];
#endif

    videoPlaypump = NEXUS_Playpump_Open(0, &videoplaypumpOpenSettings);
    audioPlaypump = NEXUS_Playpump_Open(1, &audioplaypumpOpenSettings); 
    assert(videoPlaypump);
    assert(audioPlaypump);

    NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;
    stcSettings.mode = NEXUS_StcChannelMode_eAuto;
    stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);

    /* bring up decoder and connect to local display */
    videoDecoder = NEXUS_VideoDecoder_Open(0, NULL); /* take default capabilities */

    audioDecoder = NEXUS_AudioDecoder_Open(0, NULL);

    NEXUS_Platform_GetConfiguration(&platformConfig);

    BKNI_CreateEvent(&event);

    display = NEXUS_Display_Open(0, NULL);
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
        NEXUS_DisplaySettings displaySettings;
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

    NEXUS_Playpump_GetSettings(videoPlaypump, &playpumpSettings);
    playpumpSettings.dataCallback.callback = play_callback;
    playpumpSettings.dataCallback.context = event;
    playpumpSettings.transportType = NEXUS_TransportType_eMp4Fragment;
    playpumpSettings.securityContext =    drmKeyHandle_;
    NEXUS_Playpump_SetSettings(videoPlaypump, &playpumpSettings);

    NEXUS_Playpump_GetSettings(audioPlaypump, &playpumpSettings);
    playpumpSettings.dataCallback.callback = play_callback;
    playpumpSettings.dataCallback.context = event;
    playpumpSettings.transportType = NEXUS_TransportType_eMp4Fragment;
    playpumpSettings.securityContext =    drmKeyHandle_;
    NEXUS_Playpump_SetSettings(audioPlaypump, &playpumpSettings);

    if(videoDecoder) { 
        NEXUS_PlaypumpOpenPidChannelSettings videoPidSettings;
        NEXUS_VideoDecoderStartSettings      videoProgram;
        NEXUS_VideoWindow_AddInput(window, NEXUS_VideoDecoder_GetConnector(videoDecoder));
        NEXUS_Playpump_GetDefaultOpenPidChannelSettings(&videoPidSettings);
        videoPidSettings.pidType = NEXUS_PidType_eVideo;
        videoPidChannel = NEXUS_Playpump_OpenPidChannel(videoPlaypump, 2, &videoPidSettings);
        NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
        videoProgram.codec = NEXUS_VideoCodec_eH264;
        videoProgram.pidChannel = videoPidChannel;
        videoProgram.stcChannel = stcChannel;
        videoProgram.progressiveOverrideMode = NEXUS_VideoDecoderProgressiveOverrideMode_eDisable;
        videoProgram.timestampMode = NEXUS_VideoDecoderTimestampMode_eDisplay;
        NEXUS_VideoDecoder_Start(videoDecoder, &videoProgram);
    }

    if(audioDecoder) {
        NEXUS_PlaypumpOpenPidChannelSettings audioPidSettings;
        NEXUS_AudioDecoderStartSettings      audioProgram;
        NEXUS_AudioOutput_AddInput(
                NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]),
                NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
        NEXUS_Playpump_GetDefaultOpenPidChannelSettings(&audioPidSettings);
        audioPidSettings.pidType = NEXUS_PidType_eAudio;
        audioPidChannel = NEXUS_Playpump_OpenPidChannel(audioPlaypump, 1,&audioPidSettings);
        NEXUS_AudioDecoder_GetDefaultStartSettings(&audioProgram);
        audioProgram.codec = NEXUS_AudioCodec_eAacPlusAdts; /* NEXUS_AudioCodec_eAacAdts; */
        audioProgram.pidChannel = audioPidChannel;
        audioProgram.stcChannel = stcChannel;
        NEXUS_AudioDecoder_Start(audioDecoder, &audioProgram);
    }

    NEXUS_Playpump_Start(videoPlaypump);
    NEXUS_Playpump_Start(audioPlaypump);

    /* now go back to the begining of the file and start processing the moof boxes */
    fseek(app.fp_piff, 0, SEEK_END);
    app.piff_filesize = ftell(app.fp_piff);
    fseek(app.fp_piff, 0, SEEK_SET);

    /* Start parsing the the file to look for MOOFs and MDATs */
    while(app.piff_filesize != 0){
        uint32_t moof_size = 0;
        bmp4_init_track_fragment_run_state(&app.state);
        /*
        printf("Look for a moof\n");
        */
        if(find_box_in_file(app.factory, 
                            app.fp_piff, 
                            app.pPayload, 
                            BMP4_MOVIE_FRAGMENT, 
                            &piff_container, 
                            &app.piff_filesize,&moof_size))
        {
            if(app_parse_moof(&app, piff_container)) {
                uint32_t mdat_size = 0;
                /* printf("Look for a mdat\n"); */
                /* look for the mdat and get the box's size */ 
                if(find_box_in_file(app.factory, 
                                    app.fp_piff, 
                                    app.pPayload+moof_size, 
                                    BMP4_MOVIE_DATA, 
                                    &piff_container, 
                                    &app.piff_filesize,
                                    &mdat_size))
                {
                    uint64_t         start_time;
                    batom_cursor     cursor;
                    bmp4_box         mdat;
                    batom_cursor_from_atom(&cursor, piff_container);
                    bmp4_parse_box(&cursor, &mdat);
                    if( mdat.type == BMP4_MOVIE_DATA) {
                        /* send fragment for playback. PlayReady DRM will be triggered to decrypt the fragment */ 
                        if( app.track_type[app.fragment_header.track_ID] == eVIDEO) { 
                            start_time = app.last_video_fragment_duration;
                            UPDATE_FRAGMENT_DURATION(app,app.last_video_fragment_duration);
                            send_fragment_data(app.pPayload,
                                               mdat_size+moof_size,
                                               app.fragment_header.track_ID, 
                                               start_time,
                                               videoPlaypump,
                                               event,
                                               &app.avc_config,
                                               "avc1");
                        }
                        else if( app.track_type[app.fragment_header.track_ID] == eAUDIO) { 
                            start_time = app.last_audio_fragment_duration;
                            UPDATE_FRAGMENT_DURATION(app,app.last_audio_fragment_duration);
                            send_fragment_data(app.pPayload,
                                               mdat_size+moof_size,
                                               app.fragment_header.track_ID, 
                                               start_time,
                                               audioPlaypump,
                                               event,
                                               &app.aac_config,
                                               "mp4a"); 
                        }
                    }
                    else {
                        printf("No mdat is found. \n");
                    }
                }
                else {
                    printf("could not find mdat in reference file\n");
                    break;
                }
            }
            else{
                printf("Not the fragment we're looking for, check the next one... \n");
            }
        } /* if find the moov box */
    } /* while */

    complete_play_fragments(audioDecoder,videoDecoder,videoPlaypump,audioPlaypump,display,audioPidChannel,videoPidChannel,window,event);

clean_up:

    if(app.pPayload) NEXUS_Memory_Free(app.pPayload); 
    batom_factory_destroy(app.factory); 

    if(app.fp_piff) fclose(app.fp_piff);
    if(app.drm != NULL) 
    {
        NEXUS_WmDrmPd_Destroy(app.drm);
    }

    NEXUS_Platform_Uninit();

clean_exit:

    return 0;
}
