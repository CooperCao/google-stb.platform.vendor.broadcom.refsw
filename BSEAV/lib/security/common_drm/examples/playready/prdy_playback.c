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

#ifndef NEXUS_HAS_SAGE

/* #include "drm_piff.h" */
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
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"
#include "bmp4_util.h"
#include "bbase64.h"

BDBG_MODULE(encode_piff_example);

#define BMP4_VISUAL_ENTRY_SIZE   (78)  /* Size of a Visual Sample Entry in bytes */
#define BMP4_AUDIO_ENTRY_SIZE    (28)  /* Size of an Audio Sample Entry in bytes */

#define BMP4_PROTECTIONSCHEMEINFO BMP4_TYPE('s','i','n','f')
#define BMP4_ORIGINALFMT          BMP4_TYPE('f','r','m','a')
#define BMP4_IPMPINFO             BMP4_TYPE('i','m','i','f')
#define BMP4_SCHEMETYPE           BMP4_TYPE('s','c','h','m')
#define BMP4_SCHEMEINFO           BMP4_TYPE('s','c','h','i')
#define BMP4_PIFF_SCHEMETYPE      BMP4_TYPE('p','i','f','f')
#define BMP4_CENC_SCHEMETYPE      BMP4_TYPE('c','e','n','c')
#define BMP4_CENC_TE_BOX          BMP4_TYPE('t','e','n','c')

#define BMP4_DASH_PSSH            BMP4_TYPE('p','s','s','h')
#define BMP4_PROPRITARY_BMP4      BMP4_TYPE('b','m','p','4')

#define BMP4_CENC_AVCC              BMP4_TYPE('a','v','c','C')
#define BMP4_CENC_ESDS              BMP4_TYPE('e','s','d','s')
#define BMP4_CENC_WFEX              BMP4_TYPE('w','f','e','x')
#define BMP4_CENC_DVC1              BMP4_TYPE('d','v','c','1')

/* Input file pointer */
FILE *fp;
/* stream type */
int vc1_stream = 0;

static bmp4_box_extended pssh_extended_type =
{{0xd0, 0x8a, 0x4f, 0x18, 0x10, 0xf3, 0x4a, 0x82, 0xb6, 0xc8, 0x32, 0xd8, 0xab, 0xa1, 0x83, 0xd3}};

static const bmedia_guid bmedia_protection_system_identifier_guid =
{{0x9A, 0x04, 0xF0, 0x79, 0x98, 0x40, 0x42, 0x86, 0xAB, 0x92, 0xE6, 0x5B, 0xE0, 0x88, 0x5F, 0x95}};

static bmp4_box_extended track_enc_extended_type =
{{0x89, 0x74, 0xdb, 0xce, 0x7b, 0xe7, 0x4c, 0x51, 0x84, 0xf9, 0x71, 0x48, 0xf9, 0x88, 0x25, 0x54}};

#define UPDATE_FRAGMENT_DURATION(app,fragment_duration) { \
    uint32_t i=0;                                         \
    for(; i<app.run_header.sample_count; ++i) {           \
        fragment_duration += app.samples[i].duration;     \
    }                                                     \
}                                                         \

#define BOX_HEADER_SIZE (8)
#define MAX_SAMPLES (1024)
#define MAX_PPS_SPS 256
#define BUF_SIZE (1024 * 1024 * 2) /* 2MB */

/* **FixMe** put this here for now */
/*#define NETFLIX_EXT 1*/  /*disable it for now, pls enable it after LDL feature is fully functional*/

#ifdef NETFLIX_EXT
static bool		bIsSecureStopEnabled = false;
static uint8_t	sSessionIdBuf[DRM_PRDY_SESSION_ID_LEN];
#endif

typedef enum bdrm_encr_state {
    bdrm_encr_none     = (0),                /* no encryption */
    bdrm_encr_wmdrm    = (1),                /* wmdrm encrypted asf */
    bdrm_encr_aes_ctr  = (2),                /* AES CTR encrypted stream */
    bdrm_encr_aes_cbc  = (3),                /* AES CBC encrypted stream */
    bdrm_encr_max      = (4)
} bdrm_encr_state;

#define CONVERT_ALG_ID_TO_BPIFF_ENCR_STATE(_alg_id, _state) do { \
    if(_alg_id == 0) _state = bdrm_encr_none;            \
    else if(_alg_id == 1) _state = bdrm_encr_aes_ctr;     \
    else _state = bdrm_encr_aes_cbc;  }while(0)


typedef struct bmp4_cipher_info {
    bdrm_encr_state alg_id;
    uint8_t  iv_size;
    uint8_t  kid[16];
}bmp4_scheme_info;

typedef struct bdrm_mp4_te_box{
    bmp4_fullbox     fullbox;
    bmp4_scheme_info info;
}bdrm_mp4_te_box;

typedef struct bdrm_mp4_scheme_type_box{
    bmp4_fullbox fullbox;
    uint32_t scheme_type;
    uint32_t scheme_version;
}bdrm_mp4_scheme_type_box;

typedef struct bdrm_mp4_original_fmt_box{
    uint32_t codingname;
}bdrm_mp4_original_fmt_box;


/* There is one protection scheme per track */
typedef struct bdrm_mp4_protect_scheme{
    bdrm_mp4_scheme_type_box  schm;   /* One per protection scheme */
    bdrm_mp4_original_fmt_box frma;   /* One per protection scheme */
    bdrm_mp4_te_box           te;     /* One per protection scheme*/
    uint32_t trackId;
}bdrm_mp4_protect_scheme;


/* PIFF Protection System Specific Header Box */
typedef struct bdrm_mp4_pssh {
    bmp4_box_extended extended;
    bmp4_fullbox fullbox;
    uint8_t  systemId[16];
    uint32_t size;
    uint8_t *data;
} bdrm_mp4_pssh;


typedef struct bdrm_mp4_box_se_entry {
    uint16_t bytesOfClearData;
    uint32_t bytesOfEncData;
}bdrm_mp4_box_se_entry;


#define MAX_ENTRIES_PER_SAMPLE        (10)  /*Max number of entries per sample */

typedef struct bdrm_mp4_box_se_sample {
    uint8_t  iv[16];  /* If the IV size is 8, then bytes 0 to 7 of teh 16 byte array contains the IV. */

    /* The following values are only meaningfull if flag & 0x000002 is true */
    uint16_t nbOfEntries;
    bdrm_mp4_box_se_entry entries[MAX_ENTRIES_PER_SAMPLE];

}bdrm_mp4_box_se_sample;

#define BPIFF_KEY_ID_LENGTH (16)    /* Key Id length in bytes */

typedef struct bmp4_trackInfo {
    bdrm_mp4_protect_scheme scheme; /* Will be set to NULL of no protection scheme box is found for the track */
    bool scheme_box_valid;
} bmp4_trackInfo;


#define SAMPLES_POOL_SIZE        (500)  /* Nb of bdrm_mp4_box_se_sample in the pool */

typedef struct bdrm_mp4_se {
    bmp4_box_extended extended;
    bmp4_fullbox fullbox;
    bmp4_scheme_info info;
    uint32_t sample_count;
    bdrm_mp4_box_se_sample samples[SAMPLES_POOL_SIZE];

    uint32_t track_Id; /* Id of the track this se box belongs to*/
    bool inUse;        /* Flag used to managed the pool of track id. Use to detect it the se has been assign a track id.*/
}bdrm_mp4_se;

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

    struct decoder_configuration vdec_config;
    struct decoder_configuration adec_config;

    uint8_t track_type[5];

    /* MOOF info */
    bmp4_track_fragment_run_state  state;
    bmp4_track_fragment_header     fragment_header;
    bmp4_track_fragment_run_header run_header;
    /* bmp4_trackextendsbox     track_extends; */
    bmp4_track_fragment_run_sample samples[MAX_SAMPLES];

    /* samples encryption box */
    bdrm_mp4_se samples_enc;

    /* PSSH box */
    bdrm_mp4_pssh pssh;

    uint8_t contentKey[16];

    uint64_t last_video_fragment_duration;
    uint64_t last_audio_fragment_duration;

    DRM_Prdy_DecryptContext_t decryptor;

}app_ctx;

typedef app_ctx * app_ctx_t;

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


int decrypt_sample( app_ctx_t app,
                    uint32_t sampleSize,
                    batom_cursor * cursor,
                    bdrm_mp4_box_se_sample *pSample,
                    uint32_t *bytes_processed )
{
    uint32_t     rc=0;
    uint8_t     *encBuf=NULL;
    uint8_t      ii=0;

    DRM_Prdy_AES_CTR_Info_t  aesCtrInfo;

    *bytes_processed = 0;

    BKNI_Memcpy( &aesCtrInfo.qwInitializationVector,&pSample->iv[8],8);
    aesCtrInfo.qwBlockOffset = 0;
    aesCtrInfo.bByteOffset = 0;

    if(app->samples_enc.fullbox.flags & 0x000002) {
        uint64_t     qwOffset = 0;
        for(ii = 0; ii <  pSample->nbOfEntries; ii++){
            uint32_t num_clear = pSample->entries[ii].bytesOfClearData;
            uint32_t num_enc = pSample->entries[ii].bytesOfEncData;
            *bytes_processed  += num_clear + num_enc;
            batom_cursor_skip((batom_cursor *)cursor,num_clear);

            aesCtrInfo.qwBlockOffset = qwOffset  / 16 ;
            aesCtrInfo.bByteOffset = qwOffset  % 16 ;

            if(DRM_Prdy_Reader_Decrypt(
                        &app->decryptor,
                        &aesCtrInfo,
                        (uint8_t *) cursor->cursor,
                        num_enc ) != DRM_Prdy_ok)
            {
                printf("Reader_Decrypt failed\n");
                rc = -1;
                goto ErrorExit;
            }

            batom_cursor_skip((batom_cursor *)cursor,num_enc);
            qwOffset = num_enc;

            if( *bytes_processed > sampleSize)
            {
                printf("Wrong buffer size is detected while decrypting the ciphertext, bytes processed %d, sample size to decrypt %d\n",*bytes_processed,sampleSize);
                rc = -1;
                goto ErrorExit;
            }
        }
    }
    else {
        if(DRM_Prdy_Reader_Decrypt(
                        &app->decryptor,
                        &aesCtrInfo,
                        (uint8_t *) cursor->cursor,
                        sampleSize ) != DRM_Prdy_ok)
        {
            printf("Reader_Decrypt failed\n");
            rc = -1;
            goto ErrorExit;
        }
        cursor->cursor += sampleSize;
        *bytes_processed = sampleSize;
    }

ErrorExit:
    if( encBuf != NULL) NEXUS_Memory_Free(encBuf);
    return rc;
}

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

            if(decrypt_sample(app, sampleSize, cursor, pSample, &numOfByteDecrypted)!=0) {
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
        const struct decoder_configuration *vdec_config,
        const char *fourcc)
{
    NEXUS_PlaypumpStatus playpumpStatus;
    size_t offset;
    uint64_t timescale = 90000;
    mem_file fout;
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
        if((uint8_t *)playpumpBuffer >= (uint8_t *)playpumpStatus.bufferBase +  (playpumpStatus.fifoSize - fragment_size)) {
            NEXUS_Playpump_WriteComplete(playpump, bufferSize, 0); /* skip buffer what wouldn't be big enough */
        }
    }
    fout.buf = playpumpBuffer;
    fout.buf_len = bufferSize;
    mem_file_seek(&fout, 0);
    fout.length = 0;

    offset = start_mp4_box(&fout, "bmp4");
    write_bhed_box(&fout, timescale, start_time, fourcc);
    write_bdat_box(&fout, vdec_config->data, vdec_config->size);
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
    }
    if(audioDecoder) {
        NEXUS_AudioDecoder_Stop(audioDecoder);
        NEXUS_Playpump_ClosePidChannel(audioPlaypump, audioPidChannel);
        NEXUS_Playpump_Stop(audioPlaypump);
        NEXUS_StopCallbacks(audioPlaypump);
    }
    NEXUS_Playpump_Close(videoPlaypump);
    NEXUS_Playpump_Close(audioPlaypump);
    BKNI_DestroyEvent(event);
    NEXUS_VideoWindow_Close(window);
    NEXUS_Display_Close(display);
    return 0;
}

bool find_box_in_file(batom_factory_t  factory,
                      FILE            *fp,
                      uint8_t         *pBuf,
                      uint32_t         box_type,
                      batom_t         *box,
                      uint32_t        *remaining_bytes,
                      uint32_t        *box_size)
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

        if(curr_box_type != box_type){
            fseek(fp, curr_box_size - BOX_HEADER_SIZE, SEEK_CUR);
        }
        else {
            size_to_read = curr_box_size - BOX_HEADER_SIZE;
            size = fread(pBuf + BOX_HEADER_SIZE, 1, size_to_read, fp);

            if(size != size_to_read) {
                fprintf(stderr,"fread failed\n");
                break;
            }

            (*box) = batom_from_range(factory, pBuf, curr_box_size, NULL, NULL);
            found = true;

        }

        (*remaining_bytes) -= curr_box_size;
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
        curr_box_type = batom_cursor_uint32_be(cursor);

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

    for(ii = 0; ii < parent_box.size; ii += (box_hdr_size + target_box.size)) {
        batom_checkpoint target_start;
        batom_cursor_save(cursor, &target_start);
        box_hdr_size = bmp4_parse_box(cursor, &target_box);

        if(box_hdr_size==0) {
            printf("%s empty box - %d\n", __FUNCTION__, __LINE__);
            break;
        }
        if( target_box.type == target_box_type) {
            rc = true;
            batom_cursor_rollback(cursor, &target_start);
            break; /* get out from the for loop */
        }
        else {
            /* Not the box we are looking for */
            batom_cursor_skip(cursor, target_box.size - box_hdr_size);
        }
    } /* for */

    return rc;
}

bool app_parse_sample_enc_extended_box(app_ctx_t app, batom_cursor *cursor, bmp4_box_extended senc_box)
{
    uint32_t     ii,jj;
    bdrm_mp4_box_se_sample *pSample;
    bdrm_mp4_box_se_entry *pEntry;

    BSTD_UNUSED(senc_box);
    BDBG_ASSERT(cursor != NULL);

    if(bmp4_parse_fullbox(cursor, &app->samples_enc.fullbox)){
        app->samples_enc.sample_count = batom_cursor_uint32_be(cursor);
        if(app->samples_enc.sample_count != 0){
            if(app->samples_enc.sample_count > SAMPLES_POOL_SIZE){
                printf("Sample pools too small, increase SAMPLES_POOL_SIZE, number of samples: %d\n",app->samples_enc.sample_count);
                return false;
            }

            pSample = app->samples_enc.samples;
            BKNI_Memset(pSample, 0, sizeof(bdrm_mp4_box_se_sample));

            for(ii = 0; ii < app->samples_enc.sample_count; ii++){
                uint8_t *pIv;

                batom_cursor_copy(cursor, &pSample->iv[15], 1);
                batom_cursor_copy(cursor, &pSample->iv[14], 1);
                batom_cursor_copy(cursor, &pSample->iv[13], 1);
                batom_cursor_copy(cursor, &pSample->iv[12], 1);
                batom_cursor_copy(cursor, &pSample->iv[11], 1);
                batom_cursor_copy(cursor, &pSample->iv[10], 1);
                batom_cursor_copy(cursor, &pSample->iv[9], 1);
                batom_cursor_copy(cursor, &pSample->iv[8], 1);

                BKNI_Memset(&pSample->iv[0], 0, 8);

                pIv = &app->samples_enc.samples[ii].iv[0];

                if(app->samples_enc.fullbox.flags & 0x000002){
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
    bool          senc_parsed = false;

    bool          rc = false;

    if(traf.type == BMP4_TRACK_FRAGMENT)
    {
        bool skipThisFrg = false;

        for(ii = 0; ii < traf.size; ii += (box_hdr_size + box.size))
        {
            box_hdr_size = bmp4_parse_box(cursor, &box);
            if(box_hdr_size==0) {
                break;
            }

            switch(box.type){
                case BMP4_TRACK_FRAGMENT_HEADER:
                {
                    uint8_t trackId = 0;
                    /* printf("calling bmp4_parse_track_fragment_header\n"); */
                    bmp4_parse_track_fragment_header(cursor, &app->fragment_header);
                    trackId = (uint8_t)app->fragment_header.track_ID;
                    if( app->track_type[trackId] != eAUDIO && app->track_type[trackId] != eVIDEO) {
                        printf("Detected an unknown type of fragment, skip this fragment...\n");
                        skipThisFrg = true;
                    }
                    break;
                }
                case BMP4_TRACK_FRAGMENT_RUN:
                    bmp4_parse_track_fragment_run_header(cursor, &app->run_header);

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

                 case BMP4_EXTENDED:
                 {
                     bmp4_box_extended senc_box;
                     bmp4_parse_box_extended(cursor, &senc_box);
                     rc = app_parse_sample_enc_extended_box(app, cursor, senc_box);
                     if( rc == false){
                        printf("Failed to parse sample enc box (extended), can't continue...\n");
                        goto ErrorExit;
                     }
                     senc_parsed = true;
                     break;
                 }
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

        if( rc && !senc_parsed)
        {
            printf("no sample enc box is found, the media may be unprotected...\n");
        }
    }
    else {
        /* Note a TRAF box */
        rc = false;
    }

ErrorExit:
    return rc;
}

static
bool app_parse_sinf(app_ctx_t app, batom_cursor *cursor, bmp4_box *pBox, bmp4_trackInfo *pTrack)
{
    bool         rc = false;
    bmp4_box     box;
    bmp4_box     te_box;
    size_t       box_hdr_size;
    uint32_t     ii;
    bdrm_mp4_protect_scheme  *pScheme = NULL;

    bmp4_box_extended extended;
    bmp4_fullbox     fullbox;

    bool found_piff= false;
    bool found_fmt= false;
    bool found_extended= false;
    bool found_cenc = false;
    bool found_tenc = false;
    uint32_t alg_id;

    BSTD_UNUSED(app);
    BDBG_ASSERT(cursor != NULL);
    BDBG_ASSERT(pBox->type == BMP4_PROTECTIONSCHEMEINFO);

    pScheme = &pTrack->scheme;

    for(ii = 0; ii < pBox->size; ii += (box_hdr_size + box.size))
    {
        box_hdr_size = bmp4_parse_box(cursor, &box);

        if(box_hdr_size == 0) {
            break;
        }

        switch(box.type){
            case BMP4_ORIGINALFMT:
                pScheme->frma.codingname =  batom_cursor_uint32_be(cursor);
                found_fmt= true;
                break;
            case BMP4_SCHEMETYPE:

                if(!bmp4_parse_fullbox(cursor, &fullbox)){
                    rc = false; goto ErrorExit;
                }

                pScheme->schm.fullbox.version = fullbox.version;
                pScheme->schm.fullbox.flags = fullbox.flags;

                pScheme->schm.scheme_type = batom_cursor_uint32_be(cursor);
                if (pScheme->schm.scheme_type == BMP4_CENC_SCHEMETYPE) {
                    found_cenc = true;
                    BDBG_MSG(("found 'cenc'"));
                }
                else if (pScheme->schm.scheme_type == BMP4_PIFF_SCHEMETYPE) {
                    found_piff = true;
                }
                else {
                    batom_cursor_skip(cursor, box.size - box_hdr_size - box_hdr_size);
                    continue;
                }
                pScheme->schm.scheme_version = batom_cursor_uint32_be(cursor);

                /* Skip over scheme uri, if present */
                if(pScheme->schm.fullbox.flags & 0x000001)
                    batom_cursor_skip(cursor, 1);

                break;
            case BMP4_SCHEMEINFO:
                box_hdr_size = bmp4_parse_box(cursor, &te_box);

                if(box_hdr_size == 0) {
                    break;
                }

                if (te_box.type == BMP4_CENC_TE_BOX) {
                    found_tenc = true;
                    BDBG_MSG(("found 'tenc'"));
                    if (found_piff) {
                        BDBG_WRN(("expected 'uuid' box but 'tenc' is found"));
                    }
                }
                else if (te_box.type == BMP4_EXTENDED) {
                    found_extended = true;
                    if (found_cenc) {
                        BDBG_WRN(("expected 'tenc' box but 'uuid' is found"));
                    }
                    else {
                        BDBG_MSG(("found 'uuid'"));
                    }
                    if (!bmp4_parse_box_extended(cursor, &extended)) {
                        rc = false; goto ErrorExit;
                    }
                    /* Make sure we found a TE box. */
                    if (BKNI_Memcmp(extended.usertype, track_enc_extended_type.usertype, 16) != 0) {
                        goto ErrorExit;
                    }
                }
                else {
                    /* Streams can have more than one schi box which contains other things than uuid or tenc. Just skip over it */
                    batom_cursor_skip(cursor, te_box.size - box_hdr_size);
                    continue;
                }

                if(!bmp4_parse_fullbox(cursor, &fullbox)){
                    rc = false; goto ErrorExit;
                }
                pScheme->te.fullbox.version = fullbox.version;
                pScheme->te.fullbox.flags = fullbox.flags;

                alg_id = batom_cursor_uint24_be(cursor);
                CONVERT_ALG_ID_TO_BPIFF_ENCR_STATE(alg_id, pScheme->te.info.alg_id);

                pScheme->te.info.iv_size = batom_cursor_byte(cursor);
                batom_cursor_copy(cursor,pScheme->te.info.kid, 16);
                break;
            case BMP4_IPMPINFO:
                /* This box doesn't contain any meaningfull Playready info, Ignore it*/
            default :
                /* Not the box we are looking for. Skip over it.*/
                batom_cursor_skip(cursor, box.size - box_hdr_size);
                break;
        }
    }

ErrorExit:
    if (found_fmt == true && found_piff == true && found_extended == true) {
        pTrack->scheme_box_valid = true;
        rc = true;
    }

    if (found_fmt == true && found_cenc == true && found_tenc == true) {
        pTrack->scheme_box_valid = true;
        rc = true;
    }
    return rc;
}

static
bool app_parse_stsd(app_ctx_t app, batom_cursor *cursor, bmp4_box *pBox, bmp4_trackInfo *pTrack)
{
    bool         rc = false;
    bmp4_box     box;
    size_t       box_hdr_size;
    size_t       entry_hdr_size;
    uint32_t     ii, jj;
    bmp4_box entry_box;
    bmp4_fullbox fullbox;

    uint32_t entry_count = 0;

    BDBG_ASSERT(cursor != NULL);
    BDBG_ASSERT(pBox->type == BMP4_SAMPLEDESCRIPTION);

    if(!bmp4_parse_fullbox(cursor, &fullbox)) {
        return rc;
    }

    entry_count = batom_cursor_uint32_be(cursor);

    for(ii = 0; ii < entry_count; ii++){
        entry_hdr_size = bmp4_parse_box(cursor, &entry_box);

        if(entry_hdr_size == 0) {
            break;
        }

        if((entry_box.type==BMP4_SAMPLE_ENCRYPTED_VIDEO) ||
            (entry_box.type==BMP4_SAMPLE_ENCRYPTED_AUDIO) )
        {
            bmp4_box box;
            uint32_t skip_bytes = entry_box.type==BMP4_SAMPLE_ENCRYPTED_VIDEO?BMP4_VISUAL_ENTRY_SIZE:BMP4_AUDIO_ENTRY_SIZE;

            batom_cursor_skip(cursor, skip_bytes);

            for(jj = skip_bytes + entry_hdr_size; jj < entry_box.size; jj += box.size)
            {
                box_hdr_size = bmp4_parse_box(cursor, &box);

                if(box_hdr_size == 0) {
                    break;
                }

                if(box.type == BMP4_PROTECTIONSCHEMEINFO) {
                    rc = app_parse_sinf(app, cursor, &box, pTrack);
                    if(rc == false) {
                        goto ErrorExit;
                    }
                    else if (pTrack->scheme_box_valid == true) {
                        /* No need to dig further once a valid TE box has been parsed */
                        goto ErrorExit;
                    }
                }
                BDBG_MSG(("box type = %x", box.type));
                if (vc1_stream) {
                    if (box.type == BMP4_CENC_DVC1) {
                        printf("%s - Got the DVC1 box, read the vc1_config data\n", __FUNCTION__);
                        batom_cursor_copy(cursor, app->vdec_config.data, box.size-box_hdr_size);
                        app->vdec_config.size = box.size-box_hdr_size;
                        rc = true;
                    }
                } else {
                    if(box.type == BMP4_CENC_AVCC) {
                        printf("%s - Got the AvcC box, read the avc_config data\n", __FUNCTION__);
                        batom_cursor_copy(cursor, app->vdec_config.data, box.size-box_hdr_size);
                        app->vdec_config.size = box.size-box_hdr_size;
                        rc = true;
                    }
                }

                if (vc1_stream) {
                    if(box.type == BMP4_CENC_WFEX) {
                        printf("%s - Got the wmap box, read the wmapro_config data\n", __FUNCTION__);
                        batom_cursor_copy(cursor, app->adec_config.data, box.size-box_hdr_size);
                        app->adec_config.size = box.size-box_hdr_size;
                        rc = true;
                    } else {
                        batom_cursor_skip(cursor, box.size - box_hdr_size);
                    }
                } else {
                    if(box.type == BMP4_CENC_ESDS) {
                        printf("%s - Got the esds box, read the aac_config data\n", __FUNCTION__);
                        batom_cursor_copy(cursor, app->adec_config.data, box.size-box_hdr_size);
                        app->adec_config.size = box.size-box_hdr_size;
                        rc = true;
                    } else {
                        batom_cursor_skip(cursor, box.size - box_hdr_size);
                    }
                }
            }
        }
        else {
            batom_cursor_skip(cursor, box.size - box_hdr_size);
        }
    }

ErrorExit:
    return rc;
}

static
bool app_parse_stbl(app_ctx_t app, batom_cursor *cursor, bmp4_box *pBox, bmp4_trackInfo *pTrack)
{
    bool         rc = false;
    bmp4_box     box;
    size_t       box_hdr_size;
    uint32_t     ii;

    BDBG_ASSERT(cursor != NULL);
    BDBG_ASSERT(pBox->type == BMP4_SAMPLETABLE);

    for(ii = 0; ii < pBox->size; ii += (box_hdr_size + box.size))
    {
        box_hdr_size = bmp4_parse_box(cursor, &box);

        if(box_hdr_size == 0) {
            break;
        }

        switch(box.type){
            case BMP4_SAMPLEDESCRIPTION:
                rc = app_parse_stsd(app, cursor, &box, pTrack);
                if(rc == false) goto ErrorExit;
                break;
            default :
                /* Not the box we are looking for. Skip over it.*/
                batom_cursor_skip(cursor, box.size - box_hdr_size);
                break;
        }
    }

ErrorExit:
    return rc;
}


static
bool app_parse_minf(app_ctx_t app, batom_cursor *cursor, bmp4_box *pBox, bmp4_trackInfo *pTrack)
{

    bool         rc = false;
    bmp4_box     box;
    size_t       box_hdr_size;
    uint32_t     ii;

    BDBG_ASSERT(cursor != NULL);
    BDBG_ASSERT(pBox->type == BMP4_MEDIAINFORMATION);

    for(ii = 0; ii < pBox->size; ii += (box_hdr_size + box.size))
    {
        box_hdr_size = bmp4_parse_box(cursor, &box);

        if(box_hdr_size == 0) {
            break;
        }

        switch(box.type){
            case BMP4_SAMPLETABLE:
                rc = app_parse_stbl(app, cursor, &box, pTrack);
                if(rc == false) goto ErrorExit;
                break;
            default :
                /* Not the box we are looking for. Skip over it.*/
                batom_cursor_skip(cursor, box.size - box_hdr_size);
                break;
        }
    }

ErrorExit:
    return rc;
}

static
bool app_parse_mdia(app_ctx_t app, batom_cursor *cursor, bmp4_box *pBox, bmp4_trackInfo *pTrack)
{
    bool         rc = false;
    bmp4_box     box;
    size_t       box_hdr_size;
    uint32_t     ii;

    uint32_t trackType;
    bmp4_fullbox fullbox;
    size_t fullbox_hdr_size;
    batom_checkpoint box_start;

    BDBG_ASSERT(cursor != NULL);
    BDBG_ASSERT(pBox->type == BMP4_MEDIA);

    for(ii = 0; ii < pBox->size; ii += (box_hdr_size + box.size))
    {
        box_hdr_size = bmp4_parse_box(cursor, &box);
        batom_cursor_save(cursor, &box_start);
        if(box_hdr_size == 0) {
            break;
        }

        switch(box.type){
            case BMP4_MEDIAINFORMATION:
                rc = app_parse_minf(app, cursor, &box, pTrack);
                if(rc == false) goto ErrorExit;
                break;
            case BMP4_HANDLER:
                fullbox_hdr_size = bmp4_parse_fullbox(cursor, &fullbox);
                batom_cursor_skip(cursor, 4);/* skip 4 bytes, over pre_defined field*/
                /* get the handler type */
                trackType = batom_cursor_uint32_be(cursor);

                if( trackType == BMP4_TYPE('s','o','u','n')) {
                    printf("Found track %d : audio\n", pTrack->scheme.trackId);
                    app->track_type[pTrack->scheme.trackId] = eAUDIO;
                    rc = true;
                }
                else if( trackType == BMP4_TYPE('v','i','d','e')) {
                    printf("Found track %d : video\n", pTrack->scheme.trackId);
                    app->track_type[pTrack->scheme.trackId] = eVIDEO;
                    rc = true;
                }

                batom_cursor_rollback(cursor, &box_start);
                /* Skip over the hdlr box */
                batom_cursor_skip(cursor, 0x26 - box_hdr_size);
                break;
            default :
                /* Not the box we are looking for. Skip over it.*/
                batom_cursor_skip(cursor, box.size - box_hdr_size);
                break;
        }
    }

ErrorExit:
    return rc;
}

static
bool app_parse_trak(app_ctx_t app, batom_t atom, batom_cursor *cursor, bmp4_box *pBox)
{
    bool         rc = false;
    bmp4_box     box;
    size_t       box_hdr_size;
    uint32_t     ii;
    bmp4_trackInfo  track;
    bmp4_trackheaderbox track_header;
    batom_t tkhd = NULL;
    batom_cursor start;

    BDBG_ASSERT(atom != NULL);
    BDBG_ASSERT(pBox->type == BMP4_TRACK);

    BKNI_Memset(&track, 0, sizeof(bmp4_trackInfo));

    for(ii = 0; ii < pBox->size; ii += (box_hdr_size + box.size))
    {
        box_hdr_size = bmp4_parse_box(cursor, &box);
        if(box_hdr_size == 0) {
            break;
        }

        switch(box.type){
            case BMP4_TRACKHEADER:
                /* Get an atom on the track header. */
                BATOM_CLONE(&start, cursor);
                batom_cursor_skip(cursor, box.size - box_hdr_size);
                tkhd = batom_extract(atom, &start, cursor, NULL, NULL);

                if(!bmp4_parse_trackheader(tkhd, &track_header)){
                    printf("%s - bmp4_parse_trackheader() failded rc %d\n", __FUNCTION__, rc);
                    rc = false; goto ErrorExit;
                }
                track.scheme.trackId = track_header.track_ID;
                break;
            case BMP4_MEDIA:
                rc = app_parse_mdia(app, cursor, &box, &track);
                if(rc == false) {
                    printf("%s - app_parse_mdia() failded rc %d\n", __FUNCTION__, rc);
                    goto ErrorExit;
                }
                break;
            default :
                /* Not the box we are looking for. Skip over it.*/
                batom_cursor_skip(cursor, box.size - box_hdr_size);
                break;
        }
    }


ErrorExit:
    if (tkhd) {
        batom_release(tkhd);
    }

    return rc;
}

static
int app_parse_pssh(app_ctx_t app, batom_cursor *cursor, bmp4_box *pExtended_box)
{
    int     rc = false;
    NEXUS_Error errCode;
    NEXUS_MemoryAllocationSettings allocSettings;

    BDBG_ASSERT(cursor != NULL);
    BDBG_ASSERT(pExtended_box != NULL);
    BDBG_ASSERT(pExtended_box->type == BMP4_EXTENDED);

    if(bmp4_parse_box_extended(cursor, &app->pssh.extended)){
        if(bmp4_parse_fullbox(cursor, &app->pssh.fullbox)){
            if(BKNI_Memcmp(app->pssh.extended.usertype, pssh_extended_type.usertype, 16) == 0){
                batom_cursor_copy(cursor, app->pssh.systemId, 16);
                /* Verify that the PSSH object found is really one describing the Playready content protection system */
                if(BKNI_Memcmp(app->pssh.systemId, bmedia_protection_system_identifier_guid.guid, 16) == 0){
                    app->pssh.size = batom_cursor_uint32_be(cursor);
                    NEXUS_Memory_GetDefaultAllocationSettings(&allocSettings);

                    errCode = NEXUS_Memory_Allocate(app->pssh.size, &allocSettings, (void *)(&app->pssh.data));
                    if ( errCode )
                        return rc;

                    batom_cursor_copy(cursor, app->pssh.data, app->pssh.size);
                    rc = true;
                }
            }
            else {
                /* Found another PSSH box. Might be a test box*/
                batom_cursor_skip(cursor, pExtended_box->size - 8 - 4 - 16);
            }
        }
    }

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

    if(atom == NULL)
        return false;

    batom_cursor_from_atom(&cursor, atom);

    if(bmp4_parse_box(&cursor, &moov)){

        if(moov.type == BMP4_MOVIE) {
            for(ii = 0; ii < moov.size; ii += (box_hdr_size + box.size)){
                box_hdr_size = bmp4_parse_box(&cursor, &box);
                batom_cursor_save(&cursor, &trak_payload);

                if(box_hdr_size==0) {
                    break;
                }
                if(box.type == BMP4_TRACK) {
                    rc = app_parse_trak(app, atom, &cursor, &box);
                    if(rc == false) {
                        fprintf(stderr, "Failed to parse track box, exiting...\n");
                        break;
                    }
                }
                else if(box.type == BMP4_EXTENDED) {
                    rc = app_parse_pssh(app, &cursor, &box);
                    if(rc == false) {
                        fprintf(stderr, "Failed to parse pssh box, exiting...\n");
                        break;
                    }
                }
                else {
                    /* Not the box we are looking for. Skip over it.*/
                    batom_cursor_skip(&cursor, box.size - box_hdr_size);
                }
            } /* for */

            batom_cursor_rollback(&cursor, &trak_payload);

        }
    }

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

    if(atom == NULL)
        return false;

    batom_cursor_from_atom(&cursor, atom);

    if(bmp4_parse_box(&cursor, &moof)){
        if(moof.type == BMP4_MOVIE_FRAGMENT)
        {
            for(ii = 0; ii < moof.size; ii += (box_hdr_size + box.size)){
                batom_cursor_save(&cursor, &traf_start);
                box_hdr_size = bmp4_parse_box(&cursor, &box);

                batom_cursor_save(&cursor, &traf_payload);
                if(box_hdr_size==0) {
                    break;
                }

                if(box.type == BMP4_TRACK_FRAGMENT) {
                    uint32_t traf_len;
                    /* DRM expect the whole traf box, not just the traf box payload*/
                    batom_cursor_rollback(&cursor, &traf_start);
                    traf_len = batom_cursor_size(&cursor);

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

    return rc;
}

void policy_callback(void *context, int param)
{
    BSTD_UNUSED(param);
    printf("policy_callback - event %p\n", context);
}

void playback_piff( NEXUS_VideoDecoderHandle videoDecoder,
                    NEXUS_AudioDecoderHandle audioDecoder,
                    DRM_Prdy_Handle_t    drm_context,
                    char                *piff_file)
{
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_StcChannelHandle stcChannel;
    NEXUS_VideoWindowHandle window;
    NEXUS_DisplayHandle display;
    NEXUS_PidChannelHandle videoPidChannel;
    BKNI_EventHandle event;
    NEXUS_PlaypumpHandle videoPlaypump;
    NEXUS_PlaypumpHandle audioPlaypump;

    NEXUS_PidChannelHandle audioPidChannel;

#if NEXUS_HAS_PLAYBACK
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_PlaypumpSettings playpumpSettings;
    NEXUS_Error rc;
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputStatus hdmiStatus;
#endif
#endif
    app_ctx app;
    bool moovBoxParsed = false;
    batom_t piff_container;
    uint32_t box_size = 0;

    uint8_t resp_buffer[64*1024];
    char *pCh_url = NULL;
    char *pCh_data = NULL;
    uint8_t *pResponse = resp_buffer;
    size_t respLen;
    size_t respOffset;
    size_t urlLen;
    size_t chLen;


    BSTD_UNUSED(hex2bin);
    printf("%s - %d\n", __FUNCTION__, __LINE__);
    if(piff_file == NULL ){
        goto clean_exit;
    }

    printf("PIFF file: %s\n",piff_file);
    fflush(stdout);

    BKNI_Memset(&app, 0, sizeof( app_ctx));
    app.last_video_fragment_duration = 2000000;
    app.last_audio_fragment_duration = 2000000;

    app.fp_piff = fopen(piff_file, "rb");
    if(app.fp_piff == NULL){
        fprintf(stderr,"failed to open %s\n", piff_file);
        goto clean_exit;
    }

    fseek(app.fp_piff, 0, SEEK_END);
    app.piff_filesize = ftell(app.fp_piff);
    fseek(app.fp_piff, 0, SEEK_SET);

    if( drm_context == NULL)
    {
       printf("drm_context is NULL, quitting....");
       goto clean_exit ;
    }

    NEXUS_Platform_GetConfiguration(&platformConfig);

    if( NEXUS_Memory_Allocate(BUF_SIZE, NULL, (void *)&app.pPayload) !=  NEXUS_SUCCESS) {
        fprintf(stderr,"NEXUS_Memory_Allocate failed");
        goto clean_up;
    }

    app.factory = batom_factory_create(bkni_alloc, 256);

    if(!find_box_in_file(app.factory, app.fp_piff, app.pPayload, BMP4_MOVIE, &piff_container, &app.piff_filesize,&box_size)) {
        printf("could not find moov\n");
    }
    else {
        moovBoxParsed = app_parse_moov(&app, piff_container);
    }

    if(!moovBoxParsed) {
        printf("Failed to parse moov box, can't continue...\n");
        goto clean_up;
    }

    printf("Successfully parseed the moov box, continue...\n\n");

    /* EXTRACT AND PLAYBACK THE MDAT */

    videoPlaypump = NEXUS_Playpump_Open(0, NULL);
    audioPlaypump = NEXUS_Playpump_Open(1, NULL);
    assert(videoPlaypump);
    assert(audioPlaypump);

    NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;
    stcSettings.mode = NEXUS_StcChannelMode_eAuto;
    stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);

    /* bring up decoder and connect to local display */

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
    NEXUS_Playpump_SetSettings(videoPlaypump, &playpumpSettings);

    NEXUS_Playpump_GetSettings(audioPlaypump, &playpumpSettings);
    playpumpSettings.dataCallback.callback = play_callback;
    playpumpSettings.dataCallback.context = event;
    playpumpSettings.transportType = NEXUS_TransportType_eMp4Fragment;
    NEXUS_Playpump_SetSettings(audioPlaypump, &playpumpSettings);

    if(videoDecoder ) {
        NEXUS_PlaypumpOpenPidChannelSettings videoPidSettings;
        NEXUS_VideoDecoderStartSettings videoProgram;
        NEXUS_VideoWindow_AddInput(window, NEXUS_VideoDecoder_GetConnector(videoDecoder));
        NEXUS_Playpump_GetDefaultOpenPidChannelSettings(&videoPidSettings);
        videoPidSettings.pidType = NEXUS_PidType_eVideo;


        videoPidChannel = NEXUS_Playpump_OpenPidChannel(videoPlaypump, 2, &videoPidSettings);
        NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
        if (vc1_stream)
            videoProgram.codec = NEXUS_VideoCodec_eVc1;
        else
            videoProgram.codec = NEXUS_VideoCodec_eH264;
        videoProgram.pidChannel = videoPidChannel;
        videoProgram.stcChannel = stcChannel;
        NEXUS_VideoDecoder_Start(videoDecoder, &videoProgram);
    }

    if(audioDecoder ) {
        NEXUS_AudioDecoderStartSettings audioProgram;
        NEXUS_PlaypumpOpenPidChannelSettings audioPidSettings;
        NEXUS_AudioOutput_AddInput(
                NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]),
                NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
#if NEXUS_NUM_AUDIO_DACS != 0
        NEXUS_AudioOutput_AddInput(
                NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]),
                NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
#endif
        NEXUS_Playpump_GetDefaultOpenPidChannelSettings(&audioPidSettings);
        audioPidSettings.pidType = NEXUS_PidType_eAudio;
        audioPidChannel = NEXUS_Playpump_OpenPidChannel(audioPlaypump, 1, &audioPidSettings);

        NEXUS_AudioDecoder_GetDefaultStartSettings(&audioProgram);
        if (vc1_stream)
            audioProgram.codec = NEXUS_AudioCodec_eWmaPro;
        else
            audioProgram.codec = NEXUS_AudioCodec_eAacPlusAdts; /* NEXUS_AudioCodec_eAacAdts; */
        audioProgram.pidChannel = audioPidChannel;
        audioProgram.stcChannel = stcChannel;

        NEXUS_AudioDecoder_Start(audioDecoder, &audioProgram);
    }

    NEXUS_Playpump_Start(videoPlaypump);
    NEXUS_Playpump_Start(audioPlaypump);

    /***********************
     * now ready to decrypt
     ***********************/

    if(  DRM_Prdy_Content_SetProperty( drm_context,
                                       DRM_Prdy_contentSetProperty_eAutoDetectHeader,
                                       (uint8_t *) app.pssh.data,
                                       app.pssh.size) != DRM_Prdy_ok )
    {
        printf("Failed to SetProperty for the KID, exiting...\n");
        goto clean_exit;
    }

#ifdef NETFLIX_EXT
	if (bIsSecureStopEnabled == true) {
        if(  DRM_Prdy_Get_Buffer_Size(
            drm_context,
            DRM_Prdy_getBuffer_licenseAcq_challenge_Netflix,
            NULL,
            0,
            &urlLen,
            &chLen) != DRM_Prdy_ok )
        {
            printf("DRM_Prdy_Get_Buffer_Size() failed, exiting\n");
            goto clean_exit;
        }
    } else {
        if(  DRM_Prdy_Get_Buffer_Size(
            drm_context,
            DRM_Prdy_getBuffer_licenseAcq_challenge,
            NULL,
            0,
            &urlLen,
            &chLen) != DRM_Prdy_ok )
        {
            printf("DRM_Prdy_Get_Buffer_Size() failed, exiting\n");
            goto clean_exit;
        }
    }
#else
    if(  DRM_Prdy_Get_Buffer_Size(
        drm_context,
        DRM_Prdy_getBuffer_licenseAcq_challenge,
        NULL,
        0,
        &urlLen,
        &chLen) != DRM_Prdy_ok )
    {
        printf("DRM_Prdy_Get_Buffer_Size() failed, exiting\n");
        goto clean_exit;
    }
#endif

    pCh_url = BKNI_Malloc(urlLen);
    if(pCh_url == NULL)
    {
        printf("BKNI_Malloc(urlent) failed, exiting...\n");
        goto clean_exit;
    }

    pCh_data = BKNI_Malloc(chLen);
    if(pCh_data == NULL)
    {
        printf("BKNI_Malloc(chLen) failed, exiting...\n");
        goto clean_exit;
    }

#ifdef NETFLIX_EXT
	if (bIsSecureStopEnabled == true) {
        uint8_t    tNounce[DRM_PRDY_SESSION_ID_LEN];
        if( DRM_Prdy_LicenseAcq_GenerateChallenge_Netflix(
            drm_context,
            NULL,
            0,
            pCh_url,
            &urlLen,
            pCh_data,
            &chLen,
            tNounce,
            false) != DRM_Prdy_ok )
        {
            printf("DRM_Prdy_LicenseAcq_GenerateChallenge_Netflix() failed, exiting\n");
            goto clean_exit;
        }
	} else {
        if( DRM_Prdy_LicenseAcq_GenerateChallenge(
            drm_context,
            NULL,
            0,
            pCh_url,
            &urlLen,
            pCh_data,
            &chLen) != DRM_Prdy_ok )
        {
            printf("DRM_Prdy_License_GenerateChallenge() failed, exiting\n");
            goto clean_exit;
        }
	}
#else
    if( DRM_Prdy_LicenseAcq_GenerateChallenge(
        drm_context,
        NULL,
        0,
        pCh_url,
        &urlLen,
        pCh_data,
        &chLen) != DRM_Prdy_ok )
    {
        printf("DRM_Prdy_License_GenerateChallenge() failed, exiting\n");
        goto clean_exit;
    }
#endif
    pCh_data[ chLen ] = 0;

    if(DRM_Prdy_http_client_license_post_soap (
        pCh_url,
        pCh_data,
        1,
        150,
        (unsigned char **)&pResponse,
        64*1024,
        &respOffset,
        &respLen
        )!= 0)
    {
        printf("DRM_Prdy_http_client_license_post_soap() failed, exiting\n");
        goto clean_exit;
    }

#ifdef NETFLIX_EXT
	if (bIsSecureStopEnabled == true) {
		if( DRM_Prdy_LicenseAcq_ProcessResponse_SecStop( drm_context,
												 (char *)&pResponse[respOffset],
												 respLen, sSessionIdBuf, NULL) != DRM_Prdy_ok )
		{
			printf("[FAILED] - %d Failed to process license response with SessionID buffer.\n",__LINE__);
			goto clean_exit;
		}
	}
	else {
		if( DRM_Prdy_LicenseAcq_ProcessResponse( drm_context,
												 (char *)&pResponse[respOffset],
												 respLen, NULL) != DRM_Prdy_ok )
		{
			printf("[FAILED] - %d Failed to process license response.\n",__LINE__);
			goto clean_exit;
		}
	}
#else
    if( DRM_Prdy_LicenseAcq_ProcessResponse(
        drm_context,
        (char *)&pResponse[respOffset],
        respLen,
        NULL) != DRM_Prdy_ok )
    {
        printf("DRM_Prdy_LicenseAcq_ProcessResponse() failed, exiting\n");
        goto clean_exit;
    }
#endif

    if( DRM_Prdy_Reader_Bind( drm_context,
                              &app.decryptor)!= DRM_Prdy_ok )
    {
        printf("Failed to Bind the license, the license may not exist. Exiting...\n");
        goto clean_exit;
    }


    if( DRM_Prdy_Reader_Commit( drm_context) != DRM_Prdy_ok )
    {
        printf("Failed to Commit the license after Reader_Bind, exiting...\n");
        goto clean_exit;
    }


    /* now go back to the begining and get all the moof boxes */
    fseek(app.fp_piff, 0, SEEK_END);
    app.piff_filesize = ftell(app.fp_piff);
    fseek(app.fp_piff, 0, SEEK_SET);

    /* Start parsing the the file to look for MOOFs and MDATs */
    while(app.piff_filesize != 0)
    {
        uint32_t moof_size = 0;
        bmp4_init_track_fragment_run_state(&app.state);
        if(find_box_in_file(app.factory,
                            app.fp_piff,
                            app.pPayload,
                            BMP4_MOVIE_FRAGMENT,
                            &piff_container,
                            &app.piff_filesize,&moof_size))
        {
            if(app_parse_moof(&app, piff_container))
            {
                uint32_t mdat_size = 0;
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
                    if( mdat.type == BMP4_MOVIE_DATA)
                    {
                        if(decrypt_fragment(&app, &cursor, mdat.size-8, &app.samples_enc) == 0)
                        {
                            if( app.track_type[app.fragment_header.track_ID] == eVIDEO)
                            {
                                start_time = app.last_video_fragment_duration;
                                UPDATE_FRAGMENT_DURATION(app,app.last_video_fragment_duration);
                                fwrite(app.pPayload, 1, mdat_size+moof_size, fp);
                                send_fragment_data(app.pPayload,
                                                   mdat_size+moof_size,
                                                   app.fragment_header.track_ID,
                                                   start_time,
                                                   videoPlaypump,
                                                   event,
                                                   &app.vdec_config,
                                                   vc1_stream == 0 ? "avc1" : "WVC1");
                            }
                            else if(app.track_type[app.fragment_header.track_ID] == eAUDIO)
                            {
                                start_time = app.last_audio_fragment_duration;
                                UPDATE_FRAGMENT_DURATION(app,app.last_audio_fragment_duration);
                                send_fragment_data(app.pPayload,
                                                   mdat_size+moof_size,
                                                   app.fragment_header.track_ID,
                                                   start_time,
                                                   audioPlaypump,
                                                   event,
                                                   &app.adec_config,
                                                   vc1_stream == 0 ? "mp4a" : "WMAP");
                            }
                        }
                        else
                        {
                            printf("decrypt_fragment returned failure. \n");
                        }
                    }
                }
                else
                {
                    printf("could not find mdat in reference file\n");
                    break;
                }
            }
            else
            {
                printf("Not the fragment we're looking for, check the next one... \n");
            }
        }
    } /* while */

    complete_play_fragments(audioDecoder,videoDecoder,videoPlaypump,audioPlaypump,display,audioPidChannel,videoPidChannel,window,event);

clean_up:
    if(app.decryptor.pDecrypt != NULL) DRM_Prdy_Reader_Close( &app.decryptor);
    if(app.pPayload) NEXUS_Memory_Free(app.pPayload);
    batom_factory_destroy(app.factory);

    if(app.fp_piff) fclose(app.fp_piff);

    if(pCh_url != NULL) BKNI_Free(pCh_url);
    if(pCh_data != NULL) BKNI_Free(pCh_data);
    if(app.pssh.data != NULL) NEXUS_Memory_Free(app.pssh.data);

clean_exit:
    return;
}



int main(int argc, char* argv[])
{
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_StcChannelHandle stcChannel;
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_VideoDecoderHandle videoDecoder;
    NEXUS_AudioDecoderHandle audioDecoder;
    BKNI_EventHandle event;

    /* DRM_Prdy specific */
    DRM_Prdy_Init_t     prdyParamSettings;
    DRM_Prdy_Handle_t   drm_context;

    int rc = 0;

    if (argc < 2)
    {
        BDBG_ERR(("Usage : %s <input_file> [-vc1]", argv[0]));
        rc = -1;
        goto clean_exit;
    }

    if ((argc == 3) && (strcmp(argv[2], "-vc1") == 0))
        vc1_stream = 1;

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);

    BKNI_CreateEvent(&event);

    NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;
    stcSettings.mode = NEXUS_StcChannelMode_eAuto;
    stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);

    DRM_Prdy_GetDefaultParamSettings(&prdyParamSettings);
    drm_context =  DRM_Prdy_Initialize( &prdyParamSettings);
    if(drm_context == NULL)
    {
       printf("Failed to create drm_context, quitting....");
       rc = -1;
       goto clean_exit ;
    }

#ifdef NETFLIX_EXT
	/* enable secure stop */
	if ( DRM_Prdy_TurnSecureStop(drm_context, 1) ) {
	    printf("[FAILED] - %d Failed to enable Secure Stop \n",__LINE__);
	}
	else{
		bIsSecureStopEnabled = true;
	}
#endif

    fp = fopen ("video.out", "wb");
    videoDecoder = NEXUS_VideoDecoder_Open(0, NULL); /* take default capabilities */
    audioDecoder = NEXUS_AudioDecoder_Open(0, NULL);

    playback_piff(videoDecoder,
                  audioDecoder,
                  drm_context,
                  argv[1]);

    fclose (fp);
    NEXUS_VideoDecoder_Close(videoDecoder);
    NEXUS_StcChannel_Close(stcChannel);
    NEXUS_AudioDecoder_Close(audioDecoder);

#ifdef NETFLIX_EXT
	if (bIsSecureStopEnabled == true)
	{
		uint8_t             *pSecureStop = NULL;
		uint16_t 			dataSize = 0;
		uint8_t 			dummy;
		uint8_t 			sessionIds[DRM_PRDY_MAX_NUM_SECURE_STOPS][DRM_PRDY_SESSION_ID_LEN];
		uint32_t 			count = 0;
		uint32_t			i;
		DRM_Prdy_Error_e	err;

		/* Get the list of Secure Stop Session IDs that are ready for release */
		err = DRM_Prdy_GetSecureStopIds(drm_context, sessionIds, &count);
		if (err == DRM_Prdy_ok)
		{
			int j;
			printf("DRM_Prdy_GetSecureStopIds retrieved the following %ld session ID(s): \n", count);
			for (i = 0; i < count; i++)
			{
				for (j = 0; j < DRM_PRDY_SESSION_ID_LEN; j++)
				{
						printf("%02x ", sessionIds[i][j]);
				}
				printf("\n");
			}
		}
		else
		{
			printf("DRM_Prdy_GetSecureStopIds failed with error: %d\n", err);
		}

		printf("Actual session ID is: \n");
		for (i = 0; i < DRM_PRDY_SESSION_ID_LEN; i++)
		{
			printf("%02x ", sSessionIdBuf[i]);
		}
		printf("\n");

		/* call once with zero size to determine actual size of secure stop */
		err = DRM_Prdy_GetSecureStop(drm_context, sSessionIdBuf, &dummy, &dataSize);
		if (err != DRM_Prdy_buffer_size)
		{
			printf("DRM_Prdy_GetSecureStop failed input data size = 0 with error: %d\n", err);
		}
		else
		{
			if ( NEXUS_SUCCESS == NEXUS_Memory_Allocate(dataSize, NULL, (void **)(&pSecureStop)) )
			{
				/* now get the secure stop */
				err = DRM_Prdy_GetSecureStop(drm_context, sSessionIdBuf, pSecureStop, &dataSize);
				if (err != DRM_Prdy_ok)
				{
					printf("DRM_Prdy_GetSecureStop failed with error %d\n", err);
				}
				NEXUS_Memory_Free(pSecureStop);
			}
		}
	}
#endif

    DRM_Prdy_Uninitialize(drm_context);
    NEXUS_Platform_Uninit();

clean_exit:
    return rc;
}
#else /* ifndef NEXUS_HAS_SAGE */
#include <stdio.h>
#include "bstd.h"
int main(int argc, char* argv[])
{
    BSTD_UNUSED(argc);
    BSTD_UNUSED(argv);
    printf("This application is not supported when SAGE has been enabled.\n");
    printf("Please unset SAGE_ENABLED and rebuild nexus and this application.\n");
    return -1;
}
#endif
