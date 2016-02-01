/******************************************************************************
 *    (c)2008-2014 Broadcom Corporation
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
 * Example to encode mpeg2 TS to PIFF using PIFF Creation Module's APIs.
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 *****************************************************************************/
/* Nexus example app: playback and decode */

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
#if NEXUS_HAS_PLAYBACK && NEXUS_HAS_VIDEO_ENCODER
#include "nexus_video_encoder_output.h"
#include "nexus_playback.h"
#include "nexus_file.h"
#include "nexus_video_encoder.h"
#endif
#if NEXUS_DTV_PLATFORM
#include "nexus_platform_boardcfg.h"
#endif
#include "nexus_core_utils.h"

#include "common_crypto.h"

/*#include <pthread.h>*/
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"
#include "bmp4_util.h"
#include "bbase64.h"
#include "bpiff_encoder.h"

BDBG_MODULE(encode_piff_example);

#if 0
static char lic_data_SupperSpeedway[] = "<WRMHEADER xmlns=\"http://schemas.microsoft.com/DRM/2007/03/PlayReadyHeader\" version=\"4.0.0.0\"><DATA><PROTECTINFO><KEYLEN>16</KEYLEN><ALGID>AESCTR</ALGID></PROTECTINFO><KID>AmfjCTOPbEOl3WD/5mcecA==</KID><CHECKSUM>BGw1aYZ1YXM=</CHECKSUM><CUSTOMATTRIBUTES><IIS_DRM_VERSION>7.1.1064.0</IIS_DRM_VERSION></CUSTOMATTRIBUTES><LA_URL>http://playready.directtaps.net/pr/svc/rightsmanager.asmx</LA_URL><DS_ID>AH+03juKbUGbHl1V/QIwRA==</DS_ID></DATA></WRMHEADER>";
/*
static char lic_data_BearVideoOPLs[] = "<WRMHEADER xmlns=\"http://schemas.microsoft.com/DRM/2007/03/PlayReadyHeader\" version=\"4.0.0.0\"><DATA><PROTECTINFO><KEYLEN>16</KEYLEN><ALGID>AESCTR</ALGID></PROTECTINFO><KID>tusYN3uoeU+zLAXCJuHQ0w==</KID><LA_URL>http://playready.directtaps.net/pr/svc/rightsmanager.asmx?</LA_URL><LUI_URL>http://playready.directtaps.net/pr/svc/rightsmanager.asmx?</LUI_URL><DS_ID>AH+03juKbUGbHl1V/QIwRA==</DS_ID><CHECKSUM>3hNyF98QQko=</CHECKSUM></DATA></WRMHEADER>";
*/
static char lic_data_BearVideoOPLs[] = "<WRMHEADER xmlns=\"http://schemas.microsoft.com/DRM/2007/03/PlayReadyHeader\" version=\"4.0.0.0\"><DATA><PROTECTINFO><KEYLEN>16</KEYLEN><ALGID>AESCTR</ALGID></PROTECTINFO><KID>tusYN3uoeU+zLAXCJuHQ0w==</KID><LA_URL>http://playready.directtaps.net/pr/svc/rightsmanager.asmx?</LA_URL><LUI_URL>http://playready.directtaps.net/pr/svc/rightsmanager.asmx?</LUI_URL><CHECKSUM>3hNyF98QQk1=</CHECKSUM></DATA></WRMHEADER>";

static char DS_ID[] = "AH+03juKbUGbHl1V/QIwRA==";
#endif

PIFF_encoder_handle  g_piff_handle;

const char usage_str[] =
"\n"
"USAGE: "
"encode_piff [-i input_video.ts] [-o piff_file.mp4]\n"
"\n"
"DESCRIPTION:\n"
"  Encrypt input TS stream and outpu a well-formed PIFF file.\n"
"\n";

#define DUMP_DATA_HEX(string,data,size) {        \
   char tmp[512]= "\0";                          \
   uint32_t i=0, l=strlen(string);               \
   sprintf(tmp,"%s",string);                     \
   while( i<size && l < 512) {                   \
    sprintf(tmp+l," %02x", data[i]); ++i; l+=3;} \
   printf(tmp); printf("\n");                    \
   BDBG_MSG((tmp));                              \
}

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

typedef enum bdrm_encr_state {
    bdrm_encr_none     = (0),                /* no encryption */
    bdrm_encr_wmdrm    = (1),                /* wmdrm encrypted asf */
    bdrm_encr_aes_ctr  = (2),                /* AES CTR encrypted stream */
    bdrm_encr_aes_cbc  = (3),                /* AES CBC encrypted stream */
    bdrm_encr_max      = (4)
} bdrm_encr_state;


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

    /* The following three values are only meaningfull if flag & 0x000002 is true */
    uint16_t nbOfEntries;
    bdrm_mp4_box_se_entry entries[MAX_ENTRIES_PER_SAMPLE];

}bdrm_mp4_box_se_sample;


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

    uint8_t contentKey[16];

    uint64_t last_video_fragment_duration;
    uint64_t last_audio_fragment_duration;

    DRM_Prdy_DecryptContext_t pDecryptor;

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
    uint8_t      ii;

    DRM_Prdy_AES_CTR_Info_t  aesCtrInfo;

    *bytes_processed = 0;

    BKNI_Memcpy( &aesCtrInfo.qwInitializationVector,&pSample->iv[8],8);
    aesCtrInfo.qwBlockOffset = 0;
    aesCtrInfo.bByteOffset = 0;

    switch( app->track_type[app->fragment_header.track_ID])
    {
        case eAUDIO:
        {
            if(DRM_Prdy_Reader_Decrypt(
                         &app->pDecryptor,
                         &aesCtrInfo,
                         (uint8_t *) cursor->cursor,
                         /*(uint8_t *) cursor->cursor,*/
                         sampleSize ) != DRM_Prdy_ok)
            {
                printf("Reader_Decrypt failed\n");
                rc = -1;
            }
            cursor->cursor += sampleSize;
            *bytes_processed = sampleSize;
            break;
        }
        case eVIDEO:
        {
            uint64_t     qwOffset = 0;

            for(ii = 0; ii <  pSample->nbOfEntries; ii++){
                uint32_t num_clear = pSample->entries[ii].bytesOfClearData;
                uint32_t num_enc = pSample->entries[ii].bytesOfEncData;
                *bytes_processed  += num_clear + num_enc;
                batom_cursor_skip((batom_cursor *)cursor,num_clear);
 
                aesCtrInfo.qwBlockOffset = qwOffset  / 16 ;
                aesCtrInfo.bByteOffset = qwOffset  % 16 ;

                if(DRM_Prdy_Reader_Decrypt(
                            &app->pDecryptor,
                            &aesCtrInfo,
                            (uint8_t *) cursor->cursor,
                            /*(uint8_t *) cursor->cursor,*/
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
            break;
        }
        default:
        {
            rc = -1;
            fprintf(stderr,"Detected unknown type %d of fragment, can't continues\n", app->track_type[app->fragment_header.track_ID]);
            goto ErrorExit;
        }
    }/* switch */


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
    bool          senc_parsed = false;

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
                    if( app->track_type[trackId] != eAUDIO && app->track_type[trackId] != eVIDEO) {
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
                    senc_parsed = true;
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
               printf("track type : " B_MP4_TYPE_FORMAT"\n", B_MP4_TYPE_ARG(trackType));
               if( trackType == BMP4_TYPE('s','o','u','n')) {
                   printf("Found track %d : audio\n",trackId);
                   app->track_type[trackId] = eAUDIO;
               }
               else if( trackType == BMP4_TYPE('v','i','d','e')) {
                   printf("Found track %d : video\n",trackId);
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
                               /* Now get the KID */
                               {
                                   bmp4_box sinf_box;
                                   box_hdr_size = bmp4_parse_box(cursor, &sinf_box);
                                   if( sinf_box.type == BMP4_TYPE('s','i','n','f')) {
                                       /* printf("found the sinf box\n"); */
                                       if( get_box_for_type(cursor,sinf_box,BMP4_TYPE('s','c','h','i'))) {
                                           bmp4_box schi_box;
                                           box_hdr_size = bmp4_parse_box(cursor, &schi_box);
                                           if( schi_box.type == BMP4_TYPE('s','c','h','i')) {
                                               bmp4_box tenc_box;
                                               bmp4_fullbox tenc_fullbox;
                                               box_hdr_size = bmp4_parse_box(cursor, &tenc_box);
                                               box_hdr_size = bmp4_parse_fullbox(cursor, &tenc_fullbox);
                                               /* printf("found the schi box\n"); */
                                               if(tenc_box.type == BMP4_TYPE('t','e','n','c')) {
                                                   uint8_t kid[16] = {0};
                                                   char    kidBase64[25] = "\0";
                                                   /* printf("found the tenc box\n"); */
                                                   batom_cursor_skip(cursor, 4); /* skips 4 bytes to get the key id */
                                                   batom_cursor_copy(cursor, kid, 16);
                                                   bbase64_encode(kid, 16, kidBase64,24);
                                                   printf("Key ID = %s\n ",kidBase64);
                                                   /*
                                                   DUMP_DATA_HEX("Key ID = ", kid,16);
                                                   if( gen_key_from_seed(kid,app->contentKey) != 0) {
                                                       fprintf(stderr, "Failed to create the content key...\n");
                                                   }
                                                   else {
                                                       printf("Successfully generated the content key \n");
                                                   }
                                                   */
                                               }
                                           }
                                       }
                                   }
                               }
                           }

                       }
                       else if( encv_box.type == BMP4_SAMPLE_ENCRYPTED_AUDIO ) {
                           bmp4_box esds_box;
                           /* printf("%s - Got the enca box, find the esds_box\n", __FUNCTION__);  */
                           batom_cursor_skip(cursor, 28); /* skips 28 bytes to get the "esds' box */
                           box_hdr_size = bmp4_parse_box(cursor, &esds_box);
                           /* printf("Checking for box with type " B_MP4_TYPE_FORMAT"\n",B_MP4_TYPE_ARG(esds_box.type)); */
                           if( esds_box.type == BMP4_CENC_ESDS ) {
                               /*printf("%s - Got the esds box, read the aac_config data\n", __FUNCTION__); */
                               batom_cursor_copy(cursor, app->aac_config.data, esds_box.size-box_hdr_size);
                               app->aac_config.size = esds_box.size-box_hdr_size;
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

static int usage(void)
{
    fprintf(stderr, "%s\n", usage_str);
    return 0;
}

void playback_piff( NEXUS_VideoDecoderHandle videoDecoder,
                    NEXUS_AudioDecoderHandle audioDecoder,
                    PIFF_encoder_handle  piff_handle,
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
    uint16_t kidBase64W[25] = {0};
    uint32_t kidSize=0;

    BSTD_UNUSED(hex2bin);

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
        videoProgram.codec = NEXUS_VideoCodec_eH264;
        videoProgram.pidChannel = videoPidChannel;
        videoProgram.stcChannel = stcChannel;
        videoProgram.progressiveOverrideMode = NEXUS_VideoDecoderProgressiveOverrideMode_eDisable;
        videoProgram.timestampMode = NEXUS_VideoDecoderTimestampMode_eDisplay;
        NEXUS_VideoDecoder_Start(videoDecoder, &videoProgram);
    }

    if(audioDecoder ) {
        NEXUS_AudioDecoderStartSettings audioProgram;
        NEXUS_PlaypumpOpenPidChannelSettings audioPidSettings;
        NEXUS_AudioOutput_AddInput(
                NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]),
                NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
        NEXUS_Playpump_GetDefaultOpenPidChannelSettings(&audioPidSettings);
        audioPidSettings.pidType = NEXUS_PidType_eAudio;
        audioPidChannel = NEXUS_Playpump_OpenPidChannel(audioPlaypump, 1, &audioPidSettings);

        NEXUS_AudioDecoder_GetDefaultStartSettings(&audioProgram);
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

    /* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
     * in a real production code, application should extract the KID either
     * from the PIFF file or through other means.
     * We use the piff_handle function here to get the Key ID just for a quick
     * testing only.
     * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

    /* Get the Key ID in base64 */
    if(piff_get_key_id_based64W( piff_handle,
                                 kidBase64W,
                                &kidSize) != PIFF_ENCODE_SUCCESS )
    {
        printf("Failed to get the key id, exiting...\n");
        goto clean_exit;
    }

    /* Since the license was created earlier by the piff_encoder, that We can
     * simply bind the existing license for decryption using the KID
     */
    if(  DRM_Prdy_Content_SetProperty( drm_context,
                                       DRM_Prdy_contentSetProperty_eKID,
                                       (uint8_t *) kidBase64W,
                                       kidSize*sizeof(uint16_t)) != DRM_Prdy_ok )
    {
        printf("Failed to SetProperty for the KID, exiting...\n");
        goto clean_exit;
    }

    /* Reader_Bind() */
    if( DRM_Prdy_Reader_Bind( drm_context,
                              &app.pDecryptor)!= DRM_Prdy_ok )
    {
        printf("Failed to Bind the license, the license may not exist. Exiting...\n");
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
                                send_fragment_data(app.pPayload,
                                                   mdat_size+moof_size,
                                                   app.fragment_header.track_ID,
                                                   start_time,
                                                   videoPlaypump,
                                                   event,
                                                   &app.avc_config,
                                                   "avc1");
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
                                                   &app.aac_config,
                                                   "mp4a");
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

    if( DRM_Prdy_Reader_Commit( drm_context) != DRM_Prdy_ok )
    {
        printf("Failed to Commit the license after Reader_Bind, exiting...\n");
        goto clean_exit;
    }

    complete_play_fragments(audioDecoder,videoDecoder,videoPlaypump,audioPlaypump,display,audioPidChannel,videoPidChannel,window,event);

clean_up:
    if(app.pDecryptor.pDecrypt != NULL) DRM_Prdy_Reader_Close( &app.pDecryptor);
    if(app.pPayload) NEXUS_Memory_Free(app.pPayload);
    batom_factory_destroy(app.factory);

    if(app.fp_piff) fclose(app.fp_piff);

clean_exit:
    return;
}

static void *getchar_thread(void *c)
{
    bool *key_pressed = c;
    getchar();
    *key_pressed = true;
    if( g_piff_handle != NULL)
    {
        printf("Key pressed, stopping the piff encoding here\n");
        piff_encode_stop(g_piff_handle);
    }
    return NULL;
}

static void play_endOfStreamCallback(void *context, int param)
{
	BSTD_UNUSED(context);
	BSTD_UNUSED(param);

	printf("End of stream detected\n");
    if( g_piff_handle != NULL)
    {
        printf("Stop piff encoding.\n");
        piff_encode_stop(g_piff_handle);
    }
	return;
}

static void piffEncodeComplete( void * context)
{
	BKNI_SetEvent((BKNI_EventHandle)context);
}


int main(int argc, char* argv[])
{
#if NEXUS_HAS_PLAYBACK && NEXUS_HAS_VIDEO_ENCODER
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_StcChannelHandle stcChannel, stcChannelEncoder;
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_PidChannelHandle videoPidChannel;
    NEXUS_VideoDecoderHandle videoDecoder;
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_FilePlayHandle file;
    NEXUS_PlaypumpHandle playpump;
    NEXUS_PlaybackHandle playback;
    NEXUS_PlaybackSettings playbackSettings;
    NEXUS_PlaybackPidChannelSettings playbackPidSettings;

    NEXUS_DisplayHandle displayTranscode;
    NEXUS_VideoWindowHandle windowTranscode;
    NEXUS_VideoEncoderHandle videoEncoder;
    NEXUS_VideoEncoderSettings videoEncoderConfig;
    NEXUS_VideoEncoderDelayRange videoDelay;
    NEXUS_VideoEncoderStartSettings videoEncoderStartConfig;
    NEXUS_VideoEncoderStatus videoEncoderStatus;

    NEXUS_AudioDecoderHandle audioDecoder;
    NEXUS_PidChannelHandle audioPidChannel;
    NEXUS_AudioDecoderStartSettings audioProgram;
 
    NEXUS_AudioMuxOutputHandle audioMuxOutput;
    NEXUS_AudioEncoderHandle audioEncoder;

	BKNI_EventHandle event;

    bool key_pressed  = false;
    pthread_t getchar_thread_id;
    NEXUS_DisplaySettings displaySettings;
    char fname[] = "videos/avatar_AVC_15M.ts";
    char fout[] = "piff.mp4";

    /* DRM_Prdy specific */
    DRM_Prdy_Init_t     prdyParamSettings;
    DRM_Prdy_Handle_t   drm_context;

    /* PIFF specific */
    PIFF_encoder_handle piff_handle=NULL;
    PIFF_Encoder_Settings  piffSettings;

#ifdef NEXUS_NUM_DSP_VIDEO_ENCODERS
    NEXUS_VideoWindowScalerSettings sclSettings;
    NEXUS_VideoWindowMadSettings madSettings;
    NEXUS_VideoWindowSettings windowSettings;
#endif

    BSTD_UNUSED(argc);
    BSTD_UNUSED(argv);

/*
    int ii;
    for (ii = 1; ii < argc; ii++) {
        if( !strcmp(argv[ii], "-i") ){
            fname = argv[++ii];
        }
        else if( !strcmp(argv[ii], "-o") ){
            fout = argv[++ii];
        }
    }

    if(fname == NULL || fout == NULL ){
        usage();
        goto clean_exit;
    }
*/

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);

	BKNI_CreateEvent(&event);

    printf("Creating the Event...\n");

    printf("Finish waiting...\n");
    playpump = NEXUS_Playpump_Open(0, NULL);
    assert(playpump);
    playback = NEXUS_Playback_Create();
    assert(playback);

    file = NEXUS_FilePlay_OpenPosix(fname, NULL);
    if (!file) {
        fprintf(stderr, "can't open file:%s\n", fname);
        goto clean_exit;
    }

    NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;
    stcSettings.mode = NEXUS_StcChannelMode_eAuto;
    stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);

    /* encoder requires different STC broadcast mode from decoder */
    NEXUS_StcChannel_GetDefaultSettings(1, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;
    stcSettings.mode = NEXUS_StcChannelMode_eAuto;
    stcSettings.pcrBits = NEXUS_StcChannel_PcrBits_eFull42;/* ViCE2 requires 42-bit STC broadcast */
    stcChannelEncoder = NEXUS_StcChannel_Open(1, &stcSettings);

    NEXUS_Playback_GetSettings(playback, &playbackSettings);
    playbackSettings.playpump = playpump;
    /* set a stream format, it could be any audio video transport type or file format, */
    /* i.e NEXUS_TransportType_eMp4, NEXUS_TransportType_eAvi ...                      */
    playbackSettings.playpumpSettings.transportType = NEXUS_TransportType_eTs;
    playbackSettings.stcChannel = stcChannel;

    playbackSettings.endOfStreamCallback.callback = play_endOfStreamCallback;
    playbackSettings.endOfStreamCallback.context  = NULL;

    NEXUS_Playback_SetSettings(playback, &playbackSettings);

    printf("Create a DRM_Prdy_context");
    DRM_Prdy_GetDefaultParamSettings(&prdyParamSettings);
    drm_context =  DRM_Prdy_Initialize( &prdyParamSettings);
    if( drm_context == NULL)
    {
       printf("Failed to create drm_context, quitting....");
       goto clean_exit ;
    }

    printf("Create a piff handle\n");
    piff_GetDefaultSettings(&piffSettings);
    piffSettings.destPiffFileName = fout;
    piffSettings.completionCallBack.callback = piffEncodeComplete;
    piffSettings.completionCallBack.context = event;
    piffSettings.licPolicyDescriptor.wSecurityLevel = 2000;
    /*
    piffSettings.licAcqDSId = DS_ID;
    piffSettings.licAcqDSIdLen = strlen(DS_ID);
    */
    piff_handle = piff_create_encoder_handle(&piffSettings, drm_context);
    if( piff_handle == NULL) {
        printf("FAILED to create piff handle, I'm quitting...\n");
        return 0;
    }
    printf("created a piff handle\n");
    g_piff_handle=piff_handle;

    /* bring up decoder and connect to local display */
    videoDecoder = NEXUS_VideoDecoder_Open(0, NULL); /* take default capabilities */

    audioDecoder = NEXUS_AudioDecoder_Open(0, NULL);

    /* NOTE: must open video encoder before display; otherwise open will init ViCE2 core
     * which might cause encoder display GISB error since encoder display would
     * trigger RDC to program mailbox registers in ViCE2;
     */
    videoEncoder = NEXUS_VideoEncoder_Open(0, NULL);
    assert(videoEncoder);

    /* Bring up video encoder display */
    NEXUS_Display_GetDefaultSettings(&displaySettings);
#ifdef NEXUS_NUM_DSP_VIDEO_ENCODERS
    displaySettings.format = NEXUS_VideoFormat_e720p;
    displayTranscode = NEXUS_Display_Open(0, &displaySettings);
#else
    displaySettings.displayType = NEXUS_DisplayType_eAuto;
    displaySettings.timingGenerator = NEXUS_DisplayTimingGenerator_eEncoder;
    displaySettings.format = NEXUS_VideoFormat_e720p24hz;/* bring up 480p first */
    displayTranscode = NEXUS_Display_Open(NEXUS_ENCODER_DISPLAY_IDX, &displaySettings);
#endif
    assert(displayTranscode);

    windowTranscode = NEXUS_VideoWindow_Open(displayTranscode, 0);
    assert(windowTranscode);

#ifdef NEXUS_NUM_DSP_VIDEO_ENCODERS
    NEXUS_VideoWindow_GetSettings(windowTranscode, &windowSettings);
    windowSettings.position.width = 416;
    windowSettings.position.height = 224;
    windowSettings.pixelFormat = NEXUS_PixelFormat_eCr8_Y18_Cb8_Y08;
    windowSettings.visible = false;
    NEXUS_VideoWindow_SetSettings(windowTranscode, &windowSettings);
    NEXUS_VideoWindow_GetScalerSettings(windowTranscode, &sclSettings);
    sclSettings.bandwidthEquationParams.bias = NEXUS_ScalerCaptureBias_eScalerBeforeCapture;
    sclSettings.bandwidthEquationParams.delta = 1000000;
    NEXUS_VideoWindow_SetScalerSettings(windowTranscode, &sclSettings);
    NEXUS_VideoWindow_GetMadSettings(windowTranscode, &madSettings);
    madSettings.deinterlace = true;
    madSettings.enable22Pulldown = true;
    madSettings.enable32Pulldown = true;
    NEXUS_VideoWindow_SetMadSettings(windowTranscode, &madSettings);
#endif

    /* connect same decoder to encoder display
     * This simul mode is for video encoder bringup only; audio path may have limitation
     * for simul display+transcode mode;
     */
    NEXUS_VideoWindow_AddInput(windowTranscode, NEXUS_VideoDecoder_GetConnector(videoDecoder));

    /* Open the video pid channel */
    NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
    playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
    playbackPidSettings.pidTypeSettings.video.codec = NEXUS_VideoCodec_eH264; /* must be told codec for correct handling */
    playbackPidSettings.pidTypeSettings.video.index = true;
    playbackPidSettings.pidTypeSettings.video.decoder = videoDecoder;
    videoPidChannel = NEXUS_Playback_OpenPidChannel(playback, 0x101, &playbackPidSettings);

    /* Set up decoder Start structures now. We need to know the audio codec to properly set up
    the audio outputs. */
    NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
    videoProgram.codec = NEXUS_VideoCodec_eH264;
    videoProgram.pidChannel = videoPidChannel;
    videoProgram.stcChannel = stcChannel;
 
    NEXUS_VideoEncoder_GetSettings(videoEncoder, &videoEncoderConfig);
#ifdef NEXUS_NUM_DSP_VIDEO_ENCODERS
    videoEncoderConfig.frameRate = NEXUS_VideoFrameRate_e29_97;
    videoEncoderConfig.bitrateMax = 400*1000;
#else
    videoEncoderConfig.variableFrameRate = true;
    videoEncoderConfig.frameRate = NEXUS_VideoFrameRate_e24;
    videoEncoderConfig.bitrateMax = 6*1000*1000;
    videoEncoderConfig.streamStructure.framesP = 29; /* IPP GOP size = 30 */
    videoEncoderConfig.streamStructure.framesB = 0;
#endif
    NEXUS_VideoDecoder_Start(videoDecoder, &videoProgram);

    /* Open the audio pid channel */
    NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
    playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eAudio;
    playbackPidSettings.pidTypeSettings.audio.primary = audioDecoder; /* must be told codec for correct handling */
    audioPidChannel = NEXUS_Playback_OpenPidChannel(playback, 0x104, &playbackPidSettings);

    /* Set up decoder Start structures now. We need to know the audio codec to properly set up the audio outputs. */
    NEXUS_AudioDecoder_GetDefaultStartSettings(&audioProgram);

    audioProgram.codec = NEXUS_AudioCodec_eAc3;
    audioProgram.pidChannel = audioPidChannel;
    audioProgram.stcChannel = stcChannel;

    /* Connect audio decoders to outputs */
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_AudioOutput_AddInput(
        NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]),
        NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
#else
    NEXUS_AudioOutput_AddInput(
        NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]),
        NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
#endif

    {
        NEXUS_AudioMuxOutputStartSettings audioMuxStartSettings;
        NEXUS_AudioEncoderSettings encoderSettings;

        audioMuxOutput = NEXUS_AudioMuxOutput_Create(NULL);

        NEXUS_AudioEncoder_GetDefaultSettings(&encoderSettings);
        encoderSettings.codec = NEXUS_AudioCodec_eAacAdts;
        audioEncoder = NEXUS_AudioEncoder_Open(&encoderSettings);

        /* Connect encoder to decoder */
        NEXUS_AudioEncoder_AddInput(audioEncoder,
            NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
        /* Connect mux to encoder */
        NEXUS_AudioOutput_AddInput(
            NEXUS_AudioMuxOutput_GetConnector(audioMuxOutput), NEXUS_AudioEncoder_GetConnector(audioEncoder));

        /* Start audio mux output */
        NEXUS_AudioMuxOutput_GetDefaultStartSettings(&audioMuxStartSettings);
        audioMuxStartSettings.stcChannel = stcChannelEncoder;
        NEXUS_AudioMuxOutput_Start(audioMuxOutput, &audioMuxStartSettings);
        NEXUS_AudioDecoder_Start(audioDecoder, &audioProgram);
    }


    /* Start playback */
    NEXUS_Playback_Start(playback, file, NULL);

    BKNI_Sleep(1000);

    NEXUS_VideoEncoder_GetDefaultStartSettings(&videoEncoderStartConfig);
#ifdef NEXUS_NUM_DSP_VIDEO_ENCODERS
    videoEncoderStartConfig.input = displayTranscode;
    videoEncoderStartConfig.codec = NEXUS_VideoCodec_eH264;
    videoEncoderStartConfig.profile = NEXUS_VideoCodecProfile_eBaseline;
    videoEncoderStartConfig.level = NEXUS_VideoCodecLevel_e30;
    videoEncoderStartConfig.bounds.inputDimension.max.width = 416;
    videoEncoderStartConfig.bounds.inputDimension.max.height = 224;
    videoEncoderStartConfig.stcChannel = stcChannelEncoder;
#else
    videoEncoderStartConfig.codec = NEXUS_VideoCodec_eH264;
    videoEncoderStartConfig.profile = NEXUS_VideoCodecProfile_eBaseline;
    videoEncoderStartConfig.level = NEXUS_VideoCodecLevel_e31;
    videoEncoderStartConfig.input = displayTranscode;
    videoEncoderStartConfig.stcChannel = stcChannelEncoder;

	/******************************************
	 * add configurable delay to video path
	 */
	/* NOTE: ITFP is encoder feature to detect and lock on 3:2/2:2 cadence in the video content to help
	 * efficient coding for interlaced formats; disabling ITFP will impact the bit efficiency but reduce the encode delay. */
	videoEncoderConfig.enableFieldPairing = true;

	/* 0 to use default 750ms rate buffer delay; TODO: allow user to adjust it to lower encode delay at cost of quality reduction! */
	videoEncoderStartConfig.rateBufferDelay = 0;

	/* to allow 23.976p passthru; TODO: allow user to configure minimum framerate to achieve lower delay!
	 * Note: lower minimum framerate means longer encode delay */
	videoEncoderStartConfig.bounds.inputFrameRate.min = NEXUS_VideoFrameRate_e23_976;

	/* to allow 24 ~ 60p dynamic frame rate coding TODO: allow user to config higher minimum frame rate for lower delay! */
	videoEncoderStartConfig.bounds.outputFrameRate.min = NEXUS_VideoFrameRate_e23_976;
	videoEncoderStartConfig.bounds.outputFrameRate.max = NEXUS_VideoFrameRate_e60;

	/* max encode size allows 1080p encode; TODO: allow user to choose lower max resolution for lower encode delay */
	videoEncoderStartConfig.bounds.inputDimension.max.width = 1920;
	videoEncoderStartConfig.bounds.inputDimension.max.height = 1088;
#endif

    /* NOTE: video encoder delay is in 27MHz ticks */
    NEXUS_VideoEncoder_GetDelayRange(videoEncoder, &videoEncoderConfig, &videoEncoderStartConfig, &videoDelay);
    BDBG_WRN(("\n\tVideo encoder end-to-end delay = [%u ~ %u] ms", videoDelay.min/27000, videoDelay.max/27000));
    videoEncoderConfig.encoderDelay = videoDelay.min;

    printf("PIFF encoding starts. Press Enter to stop or wait for the encoding to finish.\n");
    fflush(stdout);

    /* note the Dee is set by SetSettings */
    NEXUS_VideoEncoder_SetSettings(videoEncoder, &videoEncoderConfig);
    NEXUS_VideoEncoder_Start(videoEncoder, &videoEncoderStartConfig);
    NEXUS_VideoEncoder_GetStatus(videoEncoder, &videoEncoderStatus);

    printf("Create thread for the key pressed.\n");
    pthread_create(&getchar_thread_id, NULL, getchar_thread, &key_pressed);

    printf("Start piff encoding.\n");
    piff_encode_start( audioMuxOutput,videoEncoder,piff_handle);
    printf("piff encoding started...\n");

    BKNI_WaitForEvent(event, BKNI_INFINITE);

    printf("PIFF Encoding completed.\n");

    NEXUS_VideoEncoder_Stop(videoEncoder, NULL);

    NEXUS_VideoDecoder_Stop(videoDecoder);
    NEXUS_Playback_Stop(playback);
    NEXUS_FilePlay_Close(file);
    NEXUS_Playback_Destroy(playback);

    /* Bring down system */
    NEXUS_AudioDecoder_Stop(audioDecoder);
    NEXUS_AudioMuxOutput_Stop(audioMuxOutput);
    NEXUS_AudioEncoder_RemoveAllInputs(audioEncoder);
    NEXUS_AudioOutput_RemoveAllInputs(NEXUS_AudioMuxOutput_GetConnector(audioMuxOutput));
    NEXUS_AudioMuxOutput_Destroy(audioMuxOutput);
    NEXUS_AudioEncoder_Close(audioEncoder);

    NEXUS_VideoInput_Shutdown(NEXUS_VideoDecoder_GetConnector(videoDecoder));
    NEXUS_VideoWindow_Close(windowTranscode);
    NEXUS_Display_Close(displayTranscode);

    NEXUS_Playpump_Close(playpump);

    playback_piff(videoDecoder,
                  audioDecoder,
                  piff_handle,
                  drm_context,
                  "piff.mp4");

    NEXUS_VideoDecoder_Close(videoDecoder);
    NEXUS_StcChannel_Close(stcChannel);

    NEXUS_AudioDecoder_Close(audioDecoder);

    NEXUS_VideoEncoder_Close(videoEncoder);
    NEXUS_StcChannel_Close(stcChannelEncoder);

    DRM_Prdy_Uninitialize(drm_context);

    piff_destroy_encoder_handle(piff_handle);

    NEXUS_Platform_Uninit();

#endif
    BSTD_UNUSED(usage);

clean_exit:

    return 0;
}
