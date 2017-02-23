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
#include "bpiff_encoder.h"
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include "bmp4_util.h"
#include "nexus_memory.h"
#include "bkni.h"
#include "bkni_multi.h"
#include "common_crypto.h"
#include "nexus_base_os.h"
#include "nexus_random_number.h"
#include "nexus_video_input.h"
#include "nexus_video_encoder_output.h"
#include "nexus_types.h"
#include "drm_common_priv.h"
#include "drm_types.h"
#include "drm_common_swcrypto.h"
#include "bbase64.h"
#include "drm_prdy.h"

BDBG_MODULE(bpiff_encoder);

#define BACKWARDS_V1_1

/************
 *  DEFINES
 ************/
#define DUMP_DATA_HEX(string,data,size) {        \
   char tmp[512]= "\0";                          \
   uint32_t i=0, l=strlen(string);               \
   sprintf(tmp,"%s",string);                     \
   while( i<size && l < 512) {                   \
    sprintf(tmp+l," %02x", data[i]); ++i; l+=3;} \
   printf(tmp); printf("\n");                    \
   BDBG_MSG((tmp));                              \
} 

#define AES_EKL_SIZE                32
#define DMA_BLK_POOL_SIZE           100

#define BMP4_ISO_FTYP_MINOR_VER     1
#define BMP4_ISO_FTYP               BMP4_TYPE('f','t','y','p')
#define BMP4_ISO_BRAND_MP42         BMP4_TYPE('m','p','4','2')
#define BMP4_ISO_BRAND_MP41         BMP4_TYPE('m','p','4','1')
#define BMP4_ISO_CENC               BMP4_TYPE('c','e','n','c')
#define BMP4_PIFF_SCHEMETYPE        BMP4_TYPE('p','i','f','f')
#define BMP4_AVC1_BRAND             BMP4_TYPE('a','v','c','1')
#define BMP4_MP4A_BRAND             BMP4_TYPE('m','p','4','a')
#define BMP4_SAMPLE_MP4V            BMP4_TYPE('m','p','4','v')

#define BMP4_ISO_AVCC_NUM_PIC_PARAM_SETS 1
#define BMP4_ISO_AVCC_NALU_LENGTH_SIZE_MINUS_1 0x3
#define BMP4_ISO_AVCC_NUM_SEQ_PARAM_SETS 1
#define BMP4_ISO_AVCC_RESERVED_3BITS 0xE0
#define BMP4_ISO_AVCC_CONFIG_VER     0x01
#define BMP4_ISO_AUDIO_CHANNEL_COUNT 2
#define BMP4_ISO_AUDIO_SAMPLE_SIZE   16
#define BMP4_ISO_VIDEO_DEPTH         0x0018 
#define BMP4_ISO_VIDEO_PREDEFINED    -1
#define BMP4_ISO_VIDEO_RESOLUTION    0x00480000
#define BMP4_ISO_DEFAULT_VIDEO_WIDTH   1280
#define BMP4_ISO_DEFAULT_VIDEO_HEIGHT  720
#define BMP4_ISO_VIDEO_FRAME_COUNT   1
#define BMP4_ISO_VMHD_FLAG           0x000001
#define BMP4_ISO_NULL_CHAR          '\0'
#define BMP4_ISO_LANGUAGE_CODE_UND   0x55C4
#define BMP4_ISO_RATE_1_0            0x00010000
#define BMP4_ISO_VOLUME_1_0          0x0100
#define BMP4_ISO_RESERVED            0         /* MUST have the value zero */
#define BMP4_ISO_TIMESCALE_90KHZ     90000 

#define BMP4_SCHM_PIFF_VERSION       0x00010001  
#define BMP4_SCHM_PIFF_1_3_VERSION   0x00010000  
#define BOX_HEADER_SIZE              8 
#define BUF_SIZE                     1024 * 1024 * 2  /* 2MB */
#define MAX_NAL_BUF_SIZE             1024 * 512 /* 512KB */ 
#define MAX_NUM_SAMPLES_TRUN         128 

#define PIFF_MAX_SAMPLES             1024 
#define PIFF_MAX_ENTRIES_PER_SAMPLE  10   /*Max number of entries per sample */

                                     /* version + flags + sample_count */
#define TRUN_BOX_DATA_OFFSET_SIZE   (sizeof(uint32_t)+sizeof(uint32_t))
#define PAYLOAD_BUFFER_SIZE          2048 * 1024  /* Media obj can be a bit big in aes ctr mode */
#define KEY_IV_BUFFER_SIZE           100  

/* encoding status */
#define VIDEO_ENCODING_START         0x01 
#define VIDEO_ENCODING_COMPLETE      0x02 
#define VIDEO_ENCODING_ERROR         0x04 
#define AUDIO_ENCODING_START         0x01 
#define AUDIO_ENCODING_COMPLETE      0x02 
#define AUDIO_ENCODING_ERROR         0x04 

/* context state */
#define STATE_CONTEXT_NOT_INITIALIZED      0x00000000 
#define STATE_CONTEXT_INITIALIZED          0x00000001 
#define STATE_CONTEXT_PROCESSED_FTYP       0x00000002 
#define STATE_CONTEXT_PROCESSED_MOOV       0x00000004 
#define STATE_CONTEXT_TERMINATE_ENCODE     0x80000000

#define DMA_BLKS_SIZE                100

static const char KEYID_TAG[] = "<KID>"; 
static const char KEYID_END_TAG[] = "</KID>"; 
static const char LAURL_TAG[] = "<LA_URL>"; 
static const char LAURL[] = "http://playready.directtaps.net/pr/svc/rightsmanager.asmx?"; 
static const char LAURL_END_TAG[] = "</LA_URL>"; 
static const char LUIURL_TAG[] = "<LUI_URL>"; 
static const char LUIURL[] = "http://playready.directtaps.net/pr/svc/rightsmanager.asmx?"; 
static const char LUIURL_END_TAG[] = "</LUI_URL>"; 
static const char DSID_TAG[] = "<DS_ID>"; 
static const char DSID_END_TAG[] = "</DS_ID>"; 
static const char CUSTATTR_TAG[] = "<CUSTOMATTRIBUTES>"; 
static const char CUSTATTR_END_TAG[] = "</CUSTOMATTRIBUTES>"; 
static const char CHECKSUM_TAG[] = "<CHECKSUM>"; 
static const char CHECKSUM_END_TAG[] = "</CHECKSUM>"; 
static const char WRMHEADER[] = "<WRMHEADER xmlns=\"http://schemas.microsoft.com/DRM/2007/03/PlayReadyHeader\" version=\"4.0.0.0\"><DATA><PROTECTINFO><KEYLEN>16</KEYLEN><ALGID>AESCTR</ALGID></PROTECTINFO>";
static const char WRMHEADER_END[] = "</DATA></WRMHEADER>"; 

static const uint32_t IdentityMatrix[9] =
/* Stored order: a, b, u, c, d, v, x, y, w */
{  0x00010000, 0, 0, 0, 0x00010000, 0, 0, 0, 0x40000000 };

#define SAVE_UINT16_BE(b, d) do { (b)[0] = (uint8_t)((d)>>8); (b)[1]=(uint8_t)(d);}while(0)
#define LOAD_UINT16_BE(p,off) \
			(((uint16_t)(((uint8_t *)(p))[(off)+0])<<8) | \
			((uint16_t)(((uint8_t *)(p))[(off)+1])))

#define FRAGMENT_HEADER_SIZE 4096
#define MAX_PPS_SPS          256

extern size_t bdrm_map_cursor_to_dmablks(batom_cursor *cursor, size_t count, NEXUS_DmaJobBlockSettings *blks, uint32_t nb_blks_avail, uint32_t *nb_blks_used);

typedef enum
{
   BMP4_ISO_TrackType_eUnknown,
   /* NOTE: H.264 parameter tracks are also of type "video" */
   BMP4_ISO_TrackType_eVideo,
   BMP4_ISO_TrackType_eAudio,
   BMP4_ISO_TrackType_eHint,
   BMP4_ISO_TrackType_eODSM,
   BMP4_ISO_TrackType_eSDSM
} BMP4_ISO_TrackType;

typedef struct trackInfo
{
   uint32_t           trackId;
   uint32_t           width;    /* video-only */
   uint32_t           height;   /* video-only */
   uint32_t           duration;
   BMP4_ISO_TrackType type;

} track_info;

typedef struct memory_buffer {
    unsigned offset;
    unsigned entries;
    unsigned nalu_len_offset;
    uint8_t buf[MAX_PPS_SPS];
}memory_buffer;

typedef struct nal_writer {
    size_t nal_size;
    int nal_size_offset;
}nal_writer;

typedef struct frame_state {
    uint64_t dts; /* 33-bit DTS value (in 90 Khz) */
    uint64_t pts; /* 33-bit PTS value (in 90 KHz) */
    bool dts_valid;
    bool pts_valid;
} frame_state;

struct fragment_entry {
    uint64_t start_time;
};

typedef struct audio_payload {
    uint32_t alloc_size;
    uint32_t actual_size;
    uint8_t *data; /*dynamically allocated buffer, make sure to release the memory after */
}audio_payload;

typedef struct nal_payload {
    uint32_t alloc_size;
    uint32_t actual_size;
    uint8_t  type;
    uint8_t *data; /*dynamically allocated buffer, make sure to release the memory after */
}nal_payload;

typedef struct trun_sample {
    unsigned duration;
    unsigned size;
    unsigned composition_time_offset;

    uint8_t  iv[16];  /* If the IV size is 8, then [8] to [15] of the 16 byte array contains the IV. */
    unsigned numOfNals;
    nal_payload nals[PIFF_MAX_ENTRIES_PER_SAMPLE]; /*max of 10 nals per sample*/
}trun_sample;

typedef struct trun_accumulator {
    unsigned     nsamples;
    bool         need_sample_composition_time_offset;
    uint64_t     first_sample_time;
    uint64_t     prev_sample_time;
    trun_sample  samples[MAX_NUM_SAMPLES_TRUN];
} trun_accumulator;

struct decoder_configuration {
    size_t size;
    uint8_t data[5+MAX_PPS_SPS];
};

/***************************************************************************** 
 * Media context sturcture holds the current processing media informaiton for 
 * a given track that could be either an audio OR video BUT NOT 
 * for both. 
 *****************************************************************************/ 
typedef struct media_context 
{
    NEXUS_VideoEncoderStatus videoEncoderStatus;
    unsigned int             frame_no;
    unsigned int             fragment_no;
    bool                     fragment_in_progress;
    struct {
        NEXUS_VideoEncoderStatus  encoderStatus;
        const uint8_t            *bufferBase;
    } video;
    struct {
        NEXUS_AudioMuxOutputStatus outputStatus;
        uint64_t next_pts; /* 33-bit PTS value (in 90 KHz) */
        bool next_pts_valid;
        unsigned fragment_length; /* in miliseconds */
    } audio;
    struct {
        struct {
            bool avc_config_valid;
            struct decoder_configuration avc_config;
            struct memory_buffer pps,sps;
        } h264;
        struct {
            struct decoder_configuration aac_config;
            bool aac_config_valid;
        } aac;
    } codec;

    /* house keeping info for the current fragment */ 
    struct {
        uint32_t           moof_checkpoint;
        uint32_t           trun_data_offset_checkpoint;
        uint32_t           senc_data_offset_from_moof;
        trun_accumulator   trun;
    } fragment;

    track_info         track; 
    uint32_t           encoding_status;
} piff_media_context;

typedef struct bpiff_encoder_context 
{
    DRM_Prdy_Handle_t             prdyDrmCtx; 
    DRM_Prdy_license_handle       phLicense;                
    PIFF_Pssh_System_ID           systemId;
    PIFF_Encryption_Type          algorithm; 
    unsigned                      ivSize;
    char                         *destPiffFileName; /* destinated stream - encrypted PlayReady well-form PIFF file */   
    PIFF_Completion_CB            callback;
    void *                        callbackContext;

    /* following XML string and length fields are optional as PR 2.5 ND doesn't need
     * the license server's URL for license acquisition  */
    uint8_t                      *pLicAcqXMLStr;    
    uint32_t                      licAcqXMLStrLength;

    uint8_t                       contentKey[16]; /* for current version only. The content key will never be explosed 
                                                     in this module once PR 2.5 ND is ready */
    uint8_t                       key_checksum[8];

    uint8_t                       next_iv[16];    /* next iv */

    FILE                         *destPiffFile;      /* destinated stream - encrypted PlayReady well-form PIFF file */   
    uint8_t                       keyId[PIFF_KEY_ID_LENGTH];   /* Default key identifier for this track. */

    /* media specific */
    NEXUS_AudioMuxOutputHandle    audio_mux_output_handle;
    piff_media_context            audio_context; 
    NEXUS_VideoEncoderHandle      video_encoder_handle;
    piff_media_context            video_context; 

    /* state and mutex */
    BKNI_MutexHandle              lock;              
    unsigned                      next_fragment_no;
    uint32_t                      state;
     
}  bpiff_encoder_context; 


/***************
 * static const
 ***************/
#ifdef BACKWARDS_V1_1    
static uint8_t pssh_extended_type[16] = 
    {0xd0, 0x8a, 0x4f, 0x18, 0x10, 0xf3, 0x4a, 0x82, 0xb6, 0xc8, 0x32, 0xd8, 0xab, 0xa1, 0x83, 0xd3};
static uint8_t track_enc_extended_type[16] = 
    {0x89, 0x74, 0xdb, 0xce, 0x7b, 0xe7, 0x4c, 0x51, 0x84, 0xf9, 0x71, 0x48, 0xf9, 0x88, 0x25, 0x54};
static uint8_t sample_enc_extended_type[16] = 
    {0xa2, 0x39, 0x4f, 0x52, 0x5a, 0x9b, 0x4f, 0x14, 0xa2, 0x44, 0x6c, 0x42, 0x7c, 0x64, 0x8d, 0xf4};
#endif

static const uint8_t playready_sys_id[16] = 
    {0x9A, 0x04, 0xF0, 0x79, 0x98, 0x40, 0x42, 0x86, 0xAB, 0x92, 0xE6, 0x5B, 0xE0, 0x88, 0x5F, 0x95};

/************************
 * local data structures
 ************************/
typedef struct bpiff_encryptor 
{
    CommonCryptoHandle             cryptoHandle;
    NEXUS_KeySlotHandle            keySlot;
    NEXUS_DmaJobBlockSettings      blks[DMA_BLKS_SIZE];
} bpiff_encryptor; 

typedef struct bpiff_se_entry_box 
{
    uint16_t bytesOfClearData;
    uint32_t bytesOfEncData;
} bpiff_se_entry_box; 

typedef struct bpiff_sample_enc_box 
{
    uint32_t size;
    uint8_t  iv[16];  /* If the IV size is 8, then bytes 8 to 15 of the 16 byte array contains the IV. */
    uint16_t nbOfEntries;
    bpiff_se_entry_box entries[PIFF_MAX_ENTRIES_PER_SAMPLE];

} bpiff_sample_enc_box;

/*************************** 
 * static helper functions 
 ***************************/
static 
void write_uint8(FILE *fout, uint8_t d)
{
    fwrite(&d, sizeof(d), 1, fout);
}

static 
void write_uint16_be(FILE *fout, uint16_t data)
{
    uint8_t d[2];
    d[0] = data >> 8;
    d[1] = data;
    fwrite(d, sizeof(d), 1, fout);
}

static 
void write_uint24_be(FILE *fout, uint32_t data)
{
    uint8_t d[3];
    d[0] = data >> 16;
    d[1] = data >> 8;
    d[2] = data;
    fwrite(d, sizeof(d), 1, fout);
}

static 
void write_uint32_be(FILE *fout, uint32_t data)
{
    uint8_t d[4];
    d[0] = data >> 24;
    d[1] = data >> 16;
    d[2] = data >> 8;
    d[3] = data;
    fwrite(d, sizeof(d), 1, fout);
}

static 
void write_data(FILE *fout, const uint8_t *data, size_t len)
{
    fwrite(data, len, 1, fout);
}

static 
void write_fourcc(FILE *fout, const char *fourcc)
{
    write_data(fout, (const uint8_t *)fourcc, 4);
}

static
int gen_random_num( uint32_t	numberOfBytes, uint8_t *pIV)
{
    int rc = 0;
    NEXUS_RandomNumberGenerateSettings settings;
    NEXUS_RandomNumberOutput rngOutput;
    NEXUS_Error nxs_rc = NEXUS_SUCCESS;

    BDBG_MSG(("%s - Entered function", __FUNCTION__));

    NEXUS_RandomNumber_GetDefaultGenerateSettings(&settings);
    settings.randomNumberSize = numberOfBytes;

    nxs_rc = NEXUS_RandomNumber_Generate(&settings, &rngOutput);
    if( (nxs_rc != NEXUS_SUCCESS) || (rngOutput.size != numberOfBytes) )
    {
        BDBG_ERR(("%s - Error generating '%u' random bytes (only '%u' bytes returned) ", __FUNCTION__, numberOfBytes, rngOutput.size));
        rc = -1;
        goto ErrorExit;
    }

    BKNI_Memcpy(pIV, rngOutput.buffer, numberOfBytes);

ErrorExit:
    BDBG_MSG(("%s - Exiting function", __FUNCTION__));
    return rc;
}

#if 0
/************************************************************** 
 * Function returns a pointer of the context->next_iv[16] and 
 * increments the next IV after. 
 * It is thread-safe. 
 **************************************************************/
static 
int get_next_iv(bpiff_encoder_context * piff_cx, uint8_t *pIV )
{
    int      rc = 0;
    int8_t  idx = 15;
    bool   done = false;
    BDBG_ASSERT(piff_cx != NULL);

    BDBG_MSG(("%s - Entering function", __FUNCTION__));

    if( piff_cx == NULL) {
       BDBG_ERR(("%s - PIFF encoder handle is NULL ", __FUNCTION__));
       return -1;
    }
    if(!piff_cx->lock ) {
       BDBG_ERR(("%s - failed to get next available fragment number, the lock is not available\n", __FUNCTION__));
       return -1;
    }
    if( pIV == NULL) {
       BDBG_ERR(("%s - Invalid IV[]!\n", __FUNCTION__));
       return -1;
    }

    /* protected the next_iv[] */
    BKNI_AcquireMutex(piff_cx->lock);
    /* DUMP_DATA_HEX( "previous IV for this sample:", piff_cx->next_iv,16); */
    while(idx > 7 && !done) {
        if( ++piff_cx->next_iv[idx]==0) { --idx; }
        else { done=true; }
    }
    /* DUMP_DATA_HEX( "increment IV for this sample:", piff_cx->next_iv,16); */

    if(!done) { rc = gen_random_num(8, &piff_cx->next_iv[8]); } 

    if( rc == 0) { 
        BKNI_Memcpy(pIV, &piff_cx->next_iv[0], 16);
    }
    else
       BDBG_ERR(("%s - gen_random_num returned failure!\n", __FUNCTION__));
       
    BKNI_ReleaseMutex(piff_cx->lock);

    BDBG_MSG(("%s - Exiting function", __FUNCTION__));
    return rc;
}

/*******************************************************************************
 * Description: This function calculates the checksum using KeyId and content key. 
 * Although the checksum is said to be optional (not required) in the PlayReady Header
 * object, the PlayReady SDK 1.2 License Challenge will not work without it.   
 *
 * For ALGID value set to AESCTR, 16-byte KeyId is encrytped with 16-byte 
 * AES content key using ECB mode. The first 8 bytes of the buffer is extracted 
 * and base64 encoded.
 *******************************************************************************/
static
int gen_checksum_for_key(bpiff_encoder_context * piff_cx)
{
    NEXUS_DmaJobBlockSettings      blks[3];
    CommonCryptoClearKeySettings   keySettings;
    NEXUS_SecurityKeySlotSettings  keySlotSettings;
    CommonCryptoKeyConfigSettings  algSettings;
    CommonCryptoJobSettings        jobSettings;
    CommonCryptoHandle             handle;
    CommonCryptoSettings           cryptoSettings;
    NEXUS_KeySlotHandle            keySlot = 0;
    uint8_t                       *pBuf = NULL;
    uint32_t                       nb_blks = 0;
    int                            ii;
    int                            rc = 0;

    CommonCrypto_GetDefaultSettings(&cryptoSettings);
    handle = CommonCrypto_Open(&cryptoSettings);
    if( handle == NULL) {
        rc = -1;
        goto ErrorExit;
    }

    /* Allocate key slot for AES Counter mode */
    NEXUS_Security_GetDefaultKeySlotSettings(&keySlotSettings);
    keySlotSettings.keySlotEngine = NEXUS_SecurityEngine_eM2m;

    keySlot = NEXUS_Security_AllocateKeySlot(&keySlotSettings);
    if(keySlot == NULL) {
        BDBG_ERR(("%s - Failure to allocate key slot.\n", __FUNCTION__));
        rc = -1;
        goto ErrorExit;
    }
     
    CommonCrypto_GetDefaultKeyConfigSettings(&algSettings);
    algSettings.keySlot = keySlot;  
    algSettings.settings.opType = NEXUS_SecurityOperation_eEncrypt;
    algSettings.settings.algType = NEXUS_SecurityAlgorithm_eAes128;
    algSettings.settings.algVariant = NEXUS_SecurityAlgorithmVariant_eEcb;
    algSettings.settings.termMode = NEXUS_SecurityTerminationMode_eClear;
    algSettings.settings.keySlotType = NEXUS_SecurityKeyType_eOdd;
    algSettings.settings.enableExtKey = false;
    algSettings.settings.enableExtIv = false;	
    algSettings.settings.aesCounterSize = NEXUS_SecurityAesCounterSize_e128Bits;

    /* Configure key slot for AES Counter mode */
    if(CommonCrypto_LoadKeyConfig( handle, &algSettings) != NEXUS_SUCCESS) {
        BDBG_ERR(("%s - CommonCrypto_ConfigAlg failed aes ctr\n", __FUNCTION__));
        rc = -1;
    }

    CommonCrypto_GetDefaultClearKeySettings(&keySettings);
    keySettings.keySlot = keySlot;
    keySettings.keySlotType = NEXUS_SecurityKeyType_eOdd;
    BKNI_Memcpy(keySettings.settings.key, piff_cx->contentKey, 16);
    keySettings.settings.keySize = 16; 
    BKNI_Memset(keySettings.settings.iv, 0, 16);
    keySettings.settings.ivSize = 16; 
    rc =  CommonCrypto_LoadClearKeyIv(handle, &keySettings);
    if(rc != NEXUS_SUCCESS){
        BDBG_ERR(("%s - CommonCrypto_LoadClearKeyIv failed, rc = %d\n", __FUNCTION__, rc));
        goto ErrorExit;
    }

    rc = NEXUS_Memory_Allocate(16, NULL, (void *)&pBuf);
    if(rc != NEXUS_SUCCESS){
        BDBG_ERR(("%s - NEXUS_Memory_Allocate failed, rc = %d\n", __FUNCTION__, rc));
        goto ErrorExit;
    }

    BKNI_Memcpy(pBuf, piff_cx->keyId, 16);

    for(ii = 0; ii < 3; ii++) {
        NEXUS_DmaJob_GetDefaultBlockSettings(&blks[ii]);
    }

    blks[0].scatterGatherCryptoStart = true;
    blks[0].resetCrypto = true;

    blks[nb_blks].pSrcAddr =  pBuf;
    blks[nb_blks].pDestAddr = pBuf;
    blks[nb_blks].blockSize = 16;
    blks[nb_blks].scatterGatherCryptoEnd = true;
    nb_blks++;

    CommonCrypto_GetDefaultJobSettings(&jobSettings);
    jobSettings.keySlot = keySlot;

    rc = CommonCrypto_DmaXfer(handle, &jobSettings, blks, nb_blks);
    if(rc != NEXUS_SUCCESS){
        BDBG_ERR(("%s - CommonCrypto_DmaXfer failed, rc = %d\n", __FUNCTION__, rc));
        goto ErrorExit;
    }

    BKNI_Memcpy(piff_cx->key_checksum, pBuf, 8);
     

ErrorExit:
    if(pBuf) NEXUS_Memory_Free(pBuf);
    if(keySlot) NEXUS_Security_FreeKeySlot(keySlot);
    if(handle) CommonCrypto_Close(handle);

    BDBG_MSG(("%s - exiting, rc %d", __FUNCTION__, rc));
    return rc;
}

/* This function is only for testing only that it shall not be used for production codes. */ 
int gen_key_from_seed( uint8_t * kid, uint8_t * key )
{
    DrmCommonInit_t commonDrmInit; 
    uint8_t seed[30] = { 0x5D, 0x50, 0x68, 0xBE, 0xC9, 0xB3, 0x84, 0xFF, 0x60, 0x44, 
                         0x86, 0x71, 0x59, 0xF1, 0x6D, 0x6B, 0x75, 0x55, 0x44, 0xFC, 
                         0xD5, 0x11, 0x69, 0x89, 0xB1, 0xAC, 0xC4, 0x27, 0x8E, 0x88 };
    uint8_t seed_kid[30+16] = {0};
    uint8_t seed_kid_seed[30+16+30] = {0};
    uint8_t seed_kid_seed_kid[30+16+30+16] = {0};
    uint8_t sha_A[32] = {0};
    uint8_t sha_B[32] = {0};
    uint8_t sha_C[32] = {0};
    int i = 0;

    DRM_Common_BasicInitialize(commonDrmInit);

    memcpy(seed_kid,seed,30);
    memcpy(&seed_kid[30],kid,16);

    memcpy(seed_kid_seed,seed,30);
    memcpy(&seed_kid_seed[30],kid,16);
    memcpy(&seed_kid_seed[30+16],seed,30);

    memcpy(seed_kid_seed_kid,seed,30);
    memcpy(&seed_kid_seed_kid[30],kid,16);
    memcpy(&seed_kid_seed_kid[30+16],seed,30);
    memcpy(&seed_kid_seed_kid[30+16+30],kid,16);
    
	DRM_Common_SwSha256(seed_kid, sha_A, 30+16);
	DRM_Common_SwSha256(seed_kid_seed, sha_B, 30+16+30);
	DRM_Common_SwSha256(seed_kid_seed_kid, sha_C, 30+16+30+16);

    for( i = 0; i < 16; i++)
    { 
       key[i] = sha_A[i] ^ sha_A[i+16] ^ sha_B[i] ^ sha_B[i+16] ^ sha_C[i] ^ sha_C[i+16]; 
    }
    return 0;
}
#endif

static
int init_license( bpiff_encoder_context * piff_cx, PIFF_Encoder_Settings * pSettings)
{
    Drm_Prdy_KID_t KIDCheck;  
    if( DRM_Prdy_LocalLicense_CreateLicense( piff_cx->prdyDrmCtx,
                                             &pSettings->licPolicyDescriptor,
                                             pSettings->licType,
                                             &pSettings->keyId,
                                             0,
                                             NULL,
                                             NULL,
                                             &piff_cx->phLicense) != DRM_Prdy_ok)
    {
        BDBG_ERR(("failed to create license.", __FUNCTION__));
        return -1;
    } 

    /* sanity check */
    if( DRM_Prdy_LocalLicense_GetKID( piff_cx->phLicense,&KIDCheck) !=  DRM_Prdy_ok)
    {
        BDBG_ERR(("%s - Sanity check failed, can't get the KID from the created license.", __FUNCTION__));
        DRM_Prdy_LocalLicense_Release(&piff_cx->phLicense);
        return -1;
    }  

    if( memcmp( &pSettings->keyId.data[0],&KIDCheck.data[0],16) != 0)
    {
        BDBG_ERR(("%s - Sanity check failed, the KIDs did not match.", __FUNCTION__));
        DRM_Prdy_LocalLicense_Release(&piff_cx->phLicense);
        return -1;
    }

    /* store the license in XMR*/
    if( DRM_Prdy_LocalLicense_StoreLicense( piff_cx->phLicense,
                                            eDRM_Prdy_elocal_license_xmr_store) != DRM_Prdy_ok)
    {
        BDBG_ERR(("%s - failed to Store license.", __FUNCTION__));
        DRM_Prdy_LocalLicense_Release(&piff_cx->phLicense);
        return -1;
    } 

#if 0
    /* for the current implementation only. Use the PR testing Bear Video's key and Key ID */ 
    uint8_t key[16] = { 0x4c, 0xda, 0xd6, 0x21, 0xc6, 0x8f, 0xbb, 0xd2, 0x85, 0xc3, 0x63, 0x73, 0x7f, 0x96, 0x6f, 0x1e }; 
    uint8_t kid[16] = { 0xB6, 0xEB, 0x18, 0x37, 0x7B, 0xA8, 0x79, 0x4F, 0xB3, 0x2C, 0x05, 0xC2, 0x26, 0xE1, 0xD0, 0xD3 };
#endif

    return 0;
} 

/****************************************************************
 * Function to get the next fragment #. It is thread-safe. 
 ***************************************************************/
static 
int get_next_fragment_no( bpiff_encoder_context * piff_cx, unsigned int *next_fragmt_no )
{
   if( piff_cx == NULL) {
       BDBG_ERR(("%s - PIFF encoder handle is NULL ", __FUNCTION__));
       return -1;
   }
   if(!piff_cx->lock ) {
       BDBG_ERR(("%s - failed to get next available fragment number, the lock is not available\n", __FUNCTION__));
       return -1;
   }
   else {
       BKNI_AcquireMutex(piff_cx->lock);
       *next_fragmt_no = piff_cx->next_fragment_no;  
       piff_cx->next_fragment_no++;   
       BKNI_ReleaseMutex(piff_cx->lock);
   } 

   return 0;
}

int build_lic_acq_xml_string(bpiff_encoder_context * piff_cx, PIFF_Encoder_Settings *pSetting)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    uint32_t idx = 0;
    char tmp[1024] = "\0";
    char KID_Base64[25] = "\0";

    if( piff_cx == NULL || pSetting == NULL)
        return -1;

    /* <WRMHEADER> */
    BKNI_Memcpy(&tmp[0],WRMHEADER,strlen(WRMHEADER)); 
    idx += strlen(WRMHEADER);

    /* <KID> */
    BKNI_Memcpy(tmp+idx,KEYID_TAG,strlen(KEYID_TAG));
    idx += strlen(KEYID_TAG);

    if( pSetting->licAcqKeyId != NULL) {
        bbase64_encode(pSetting->licAcqKeyId, 16, KID_Base64,24);
    }
    else { 
        bbase64_encode(piff_cx->keyId, 16, KID_Base64,24);
    }
    /*
    printf("Key ID = %s\n",KID_Base64);
    fflush(stdout);
    */
    BKNI_Memcpy(tmp+idx,KID_Base64,24);
    idx += 24;
    BKNI_Memcpy(tmp+idx,KEYID_END_TAG,strlen(KEYID_END_TAG));
    idx += strlen(KEYID_END_TAG);

    /* <LA_URL> */
    BKNI_Memcpy(tmp+idx,LAURL_TAG,strlen(LAURL_TAG));
    idx += strlen(LAURL_TAG);
    if( pSetting->licAcqLAURL != NULL){ 
        BKNI_Memcpy(tmp+idx,pSetting->licAcqLAURL,pSetting->licAcqLAURLLen);
        idx += pSetting->licAcqLAURLLen;
    }
    else {
        BKNI_Memcpy(tmp+idx,LAURL,strlen(LAURL));
        idx += strlen(LAURL);
    } 
    BKNI_Memcpy(tmp+idx,LAURL_END_TAG,strlen(LAURL_END_TAG));
    idx += strlen(LAURL_END_TAG);

    /* <LUI_URL> */
    BKNI_Memcpy(tmp+idx,LUIURL_TAG,strlen(LUIURL_TAG));
    idx += strlen(LUIURL_TAG);
    if( pSetting->licAcqLUIURL != NULL){ 
        BKNI_Memcpy(tmp+idx,pSetting->licAcqLUIURL,pSetting->licAcqLUIURLLen);
        idx += pSetting->licAcqLUIURLLen;
    }
    else {
        BKNI_Memcpy(tmp+idx,LUIURL,strlen(LUIURL));
        idx += strlen(LUIURL);
    } 
    BKNI_Memcpy(tmp+idx,LUIURL_END_TAG,strlen(LUIURL_END_TAG));
    idx += strlen(LUIURL_END_TAG);

    /* <DS_ID> */
    if( pSetting->licAcqDSId != NULL){ 
        BKNI_Memcpy(tmp+idx,DSID_TAG,strlen(DSID_TAG));
        idx += strlen(DSID_TAG);
        BKNI_Memcpy(tmp+idx,pSetting->licAcqDSId,pSetting->licAcqDSIdLen);
        idx += pSetting->licAcqDSIdLen;
        BKNI_Memcpy(tmp+idx,DSID_END_TAG,strlen(DSID_END_TAG));
        idx += strlen(DSID_END_TAG);
    }

    /* <CUSTOMATTRIBUTES> */
    if( pSetting->licCustomAttr != NULL){ 
        BKNI_Memcpy(tmp+idx,CUSTATTR_TAG,strlen(CUSTATTR_TAG));
        idx += strlen(CUSTATTR_TAG);
        BKNI_Memcpy(tmp+idx,pSetting->licCustomAttr,pSetting->licCustomAttrLen);
        idx += pSetting->licCustomAttrLen;
        BKNI_Memcpy(tmp+idx,CUSTATTR_END_TAG,strlen(CUSTATTR_END_TAG));
        idx += strlen(DSID_END_TAG);
    }

    /* <CHECKSUM> */
    BKNI_Memcpy(tmp+idx,CHECKSUM_TAG,strlen(CHECKSUM_TAG));
    idx += strlen(CHECKSUM_TAG);
    /* encode the key checksum into Base64 */
    {
        char checksum[13] = "\0";
        bbase64_encode(piff_cx->key_checksum, 8, checksum, 12);
        BKNI_Memcpy(tmp+idx,checksum,12);
        idx += 12; 
        BDBG_MSG(("Key Checksum : %s",checksum));
    }
    BKNI_Memcpy(tmp+idx,CHECKSUM_END_TAG,strlen(CHECKSUM_END_TAG));
    idx += strlen(CHECKSUM_END_TAG);

    /* close the </WRMHEARDER> TAG */
    BKNI_Memcpy(tmp+idx,WRMHEADER_END,strlen(WRMHEADER_END));
    idx += strlen(WRMHEADER_END);

    /* now copy the whole string to the context */ 
    rc = NEXUS_Memory_Allocate(idx, NULL, (void *)&piff_cx->pLicAcqXMLStr);
    if(rc != NEXUS_SUCCESS) {
        BDBG_ERR(("%s - NEXUS_Memory_Allocate failed for the license acquisition string, rc = %d\n", __FUNCTION__, rc));
        return -1;
    }

    BKNI_Memcpy(piff_cx->pLicAcqXMLStr, tmp, idx);
    piff_cx->licAcqXMLStrLength = idx;

    return 0;
}

/************************************
 * static mp4 boxes helper functions
 ************************************/ 
static 
int start_mp4_box(FILE *fout, const char *type)
{
    int offset = ftell(fout);
    write_uint32_be(fout, 0); /* size */
    write_fourcc(fout, type); /* type */
    return offset;
}

static 
int start_mp4_fullbox(FILE *fout, const char *type, unsigned version, uint32_t flags)
{
    int offset = start_mp4_box(fout, type);
    write_uint8(fout, version);
    write_uint24_be(fout, flags);
    return offset; 
}

#ifdef BACKWARDS_V1_1    
static 
int start_mp4_extended_fullbox(FILE *fout, const uint8_t *extended_type, unsigned version, uint32_t flags)
{
    int offset = start_mp4_box(fout,"uuid");

	BDBG_ASSERT(extended_type);

    write_data(fout, (const uint8_t *) extended_type, 16);
    write_uint8(fout, version);
    write_uint24_be(fout, flags);
    return offset; 
}
#endif

static 
void finish_mp4_box(FILE *fout, int offset)
{
    int current = ftell(fout);

    fseek(fout, offset, SEEK_SET);
    write_uint32_be(fout, current-offset); /* size */
    fseek(fout, current, SEEK_SET);
    return;
}

/*******************************
 * Encryption functions
 *******************************/
#if 0
static
void uninit_piff_encryptor( bpiff_encryptor * pEncryptor)
{
    if( pEncryptor)
    {
        if(pEncryptor->keySlot) NEXUS_Security_FreeKeySlot(pEncryptor->keySlot);
        if(pEncryptor->cryptoHandle) CommonCrypto_Close(pEncryptor->cryptoHandle);
    }
}

static
int init_piff_encryptor( bpiff_encryptor * pEncryptor)
{
    CommonCryptoSettings           cryptoSettings;
    NEXUS_SecurityKeySlotSettings  keySlotSettings;
    CommonCryptoKeyConfigSettings  algSettings;
    int                            rc = 0;

    BDBG_MSG(("%s - Entering function", __FUNCTION__));

    if( pEncryptor == NULL) {
        rc = -1;
        goto ErrorExit;
    }

    CommonCrypto_GetDefaultSettings(&cryptoSettings);
    pEncryptor->cryptoHandle = CommonCrypto_Open(&cryptoSettings);
    if( pEncryptor->cryptoHandle == NULL) {
        rc = -1;
        goto ErrorExit;
    }

    /* Allocate key slot for AES Counter mode */
    NEXUS_Security_GetDefaultKeySlotSettings(&keySlotSettings);
    keySlotSettings.keySlotEngine = NEXUS_SecurityEngine_eM2m;

    pEncryptor->keySlot = NEXUS_Security_AllocateKeySlot(&keySlotSettings);
    if(pEncryptor->keySlot == NULL) {
        BDBG_ERR(("%s - Failure to allocate key slot for piff encryptor\n", __FUNCTION__));
        rc = -1;
        goto ErrorExit;
    }
     
    CommonCrypto_GetDefaultKeyConfigSettings(&algSettings);
    algSettings.keySlot = pEncryptor->keySlot;  
    algSettings.settings.opType = NEXUS_SecurityOperation_eEncrypt;
    algSettings.settings.algType = NEXUS_SecurityAlgorithm_eAes128;
    algSettings.settings.algVariant = NEXUS_SecurityAlgorithmVariant_eCounter;
    algSettings.settings.termMode = NEXUS_SecurityTerminationMode_eClear;
    algSettings.settings.enableExtKey = true;
    algSettings.settings.enableExtIv = true;	
    /* always assume IV size is 8 */
    algSettings.settings.aesCounterSize = NEXUS_SecurityAesCounterSize_e64Bits;

    /* Configure key slot for AES Counter mode */
    if(CommonCrypto_LoadKeyConfig( pEncryptor->cryptoHandle, &algSettings) != NEXUS_SUCCESS) {
        BDBG_ERR(("%s - CommonCrypto_ConfigAlg failed aes ctr\n", __FUNCTION__));
        rc = -1;
    }

ErrorExit:
    if( rc != 0) {
        uninit_piff_encryptor(pEncryptor);
    }
    BDBG_MSG(("%s - exiting, rc %d", __FUNCTION__, rc));
    return rc;
}
#endif

/************************************************************************************* 
 * This function simply encrypts a sample on the fly, which is either an audio or 
 * vidoe payload. The input parameter "trun_sample" will also be the output but encrypted.  
 * On exiting, if no error all the resources will be properly released.
 *************************************************************************************/ 
static 
int encrypt_sample(piff_media_context *media_cx, bpiff_encoder_context *piff_cx, trun_sample *sample )
{
    uint32_t     currentSize = 0;
    uint32_t     rc=0;
    uint8_t      jj=0;
    uint8_t     *encBuf=NULL;

    BKNI_AcquireMutex(piff_cx->lock);

    BDBG_MSG(("%s - Entering function", __FUNCTION__));

    BKNI_Memset(sample->iv, 0, 16);
    /* check if this is for video */
    BDBG_MSG(("    number of nals %d", sample->numOfNals));
    if( media_cx->track.type == BMP4_ISO_TrackType_eAudio) 
    { 
        /* printf("Encrypt Audio sample...\n");
         */
        nal_payload *nal = &sample->nals[0];
        DRM_Prdy_sample_t   payload;
        DRM_Prdy_subSample_t s_sample;
        s_sample.size = nal->actual_size; 
        s_sample.sample = nal->data; 

        payload.numOfSubsamples=1; 
        payload.subsamples=&s_sample; 

        if(DRM_Prdy_LocalLicense_EncryptSample( piff_cx->phLicense,
                                                nal->data, 
                                                nal->data, 
                                                nal->actual_size, 
                                                &sample->iv[8]) != DRM_Prdy_ok)
        {
            BDBG_ERR(("%s - DRM_Prdy_LocalLicense_EncryptSample failed. No encryption has been done.", __FUNCTION__));
            rc = -1;
            goto ErrorExit;
        };
    }
    else 
    {
        DRM_Prdy_sample_t     payload;
        DRM_Prdy_subSample_t  s_sample;
        DRM_Prdy_subSample_t *pSubSamples;
        uint32_t              numOfNals = sample->numOfNals;

        currentSize = 0;

        /* consturct the DRM_Prdy_sample_t structure */
        pSubSamples = BKNI_Malloc( sizeof(DRM_Prdy_subSample_t) * numOfNals);  
        for(jj = 0; jj < sample->numOfNals; ++jj) {
            nal_payload *nal = &sample->nals[jj]; 
            if( (currentSize + nal->actual_size) > sample->size) {
                BDBG_ERR(("%s - Incorrect NAL size detected: accumulated NAL size %d > sample size %d , can't encrypt the layload." ,
                            __FUNCTION__,
                            currentSize+nal->actual_size,
                            sample->size));

                if( encBuf != NULL) {
                    NEXUS_Memory_Free(encBuf);
                }

                rc = -1;
                goto ErrorExit;
            }
            pSubSamples[jj].sample = nal->data;
            pSubSamples[jj].size = nal->actual_size;
            currentSize += nal->actual_size;
        }

        /* encrypt the sample using SDK */
        s_sample.size = currentSize; 
        s_sample.sample = encBuf; 
        payload.numOfSubsamples=sample->numOfNals; 
        payload.subsamples=pSubSamples; 
        if(DRM_Prdy_LocalLicense_EncryptSubsamples( piff_cx->phLicense,
                                                   &payload,
                                                   &payload,
                                                   &sample->iv[8]) != DRM_Prdy_ok)
        {
            BDBG_ERR(("%s - DRM_Prday_LocalLicense_EncryptSubsamples failed. No encryption has been done.", __FUNCTION__));
            if( encBuf != NULL) { 
                NEXUS_Memory_Free(encBuf);
            }
            if( pSubSamples != NULL) { 
                BKNI_Free(pSubSamples); 
            }

            rc = -1;
            goto ErrorExit;
        };

        if( pSubSamples != NULL) {
            BKNI_Free(pSubSamples); 
        }

        /* free the temporary encrypted buffer */
        if( encBuf != NULL) NEXUS_Memory_Free(encBuf);

    } /* else */
          
ErrorExit:

    BKNI_ReleaseMutex(piff_cx->lock);
    BDBG_MSG(("%s - exiting, rc %d", __FUNCTION__, rc));

    return rc;
}

/************************************
 * static PIFF encoding functions
 ************************************/

/****************************************************************************************
 *  PSSH - PIFF V1.1 Spec. Sec 5.3.1.1
 *  PSSH - PIFF V1.3 ISO/IEC 23001-7:2011 Spec. Sec. 8.1 
 *
 * For v1.1
 *  FullBox('uuid',extended_type=0xd08a4f1810f34a82b6c832d8aba183d3,version=0,flags=0)
 * For v1.3
 *  FullBox('pssh',version=0,flags=0)
 *  {
 *    uint8_t    SystemID[16] 
 *    uint32_t   DataSize
 *    uint8_t    Data[DataSize]
 *  }
 *
 *  PlayReady's SystemID=9A04F07998404286ab92e65be0885f95
 *  Data[] = PlayReadyHeaderObj + PlayReadyRecord
 *  
 *  struct PlayReadyRecord
 *  {
 *    uint16_t   RecordType
 *    uint16_t   RecordLength
 *    BYTE       RecordValue[RecordLength] (license XML string)
 *  }
 *
 *  struct PlayReadyHeaderObj
 *  {
 *    uint32_t         Length (size of the entire PlayReady header object in bytes)
 *    uint16_t         RecordCount
 *    PlayReadyRecord  Record
 *  }
 *
 * Note - PlayReady Header and Record are stored in little-endian format and the XML
 *        string are UTF-16 (2-byte). 
 **************************************************************************************/
static 
void write_pssh_box(const bpiff_encoder_context *piff_cx, FILE *fout )
{
    int current;
    int offset = ftell(fout);
    uint32_t idx;
    uint16_t w_str;
    uint32_t dataSize=0;
    uint16_t recordCount=1;
    uint16_t rec_len_start_offset;
    uint16_t recordType=1;
    uint16_t recordLen=0;
    uint32_t dataSize_offset;
    uint32_t raw_data_start_offset;
    uint32_t pdyheadObj_offset;

    BDBG_MSG(("%s - Entered function", __FUNCTION__));

    /*----------------------------- 
     * build the extended PSSH box
     *-----------------------------*/
    offset = start_mp4_fullbox(fout,"pssh",0,0); 
    /* write_data(fout,(const uint8_t *)piff_cx->psshSystemId.systemId.data,16);  SystemID */
    write_data(fout,(const uint8_t *)piff_cx->systemId.data,PIFF_GUID_LENGTH); /* SystemID */

    dataSize_offset = ftell(fout); /* DataSize_offset */
    write_uint32_be(fout, 0);      /* DataSize place holder */

    /* only build when it is supplied */
    if( piff_cx->licAcqXMLStrLength > 0)
    {
        /*------------------------------------ 
         * build the PlayReady Header Object 
         *------------------------------------*/
        pdyheadObj_offset = ftell(fout);  /* Data start */
        idx = 0;
        w_str = 0;
        dataSize = 0;
        fwrite(&dataSize,4,1,fout);    /* 4-byte of the length field (little endian) */
        fwrite(&recordCount,2,1,fout); /* 2-byte of the record count field (little endian) */
        fwrite(&recordType,2,1,fout);  /* 2-byte of the record type field (little endian) */
        rec_len_start_offset = ftell(fout);
        fwrite(&recordLen,2,1,fout);   /* 2-byte of the record length field (little endian) */

        raw_data_start_offset = ftell(fout);    /* position for raw data start */
        while( idx < piff_cx->licAcqXMLStrLength ) {
            w_str = piff_cx->pLicAcqXMLStr[idx];
            fwrite(&w_str,2,1,fout); /* 2-byte (utf-16) for each character (little endian) */
            idx++;
        }

        /* it is done, get the current position */
        current = ftell(fout);
        BDBG_MSG(("write the lic data complete, current position %d, data size: %d", current, current-raw_data_start_offset));

        /* write the raw date size to the record length field */ 
        fseek(fout,rec_len_start_offset,SEEK_SET);
        recordLen = (uint16_t) (current-raw_data_start_offset);
        BDBG_MSG(("write the record length %d", recordLen));

        /* write 2-byte of the record length field (little-endian) */
        fwrite(&recordLen,2,1,fout);  

        /* write the total data size + PlayReady header object into the length field of the PlayReady header object */
        fseek(fout,pdyheadObj_offset,SEEK_SET);
        dataSize = current-pdyheadObj_offset;
        BDBG_MSG(("write the length of the PlayReady Header object %d", dataSize));

        /* write 4-byte of the record length field (little-endian) */ 
        fwrite(&dataSize,4,1,fout);   

        /* also write the total data size + PlayReady header object into the DataSize field of the PSSH box */
        fseek(fout,dataSize_offset,SEEK_SET);
        BDBG_MSG(("write the DataSize to the PSSH object %d", dataSize));
        write_uint32_be(fout, dataSize);

        BDBG_MSG(("pdyheader - number of bytes : %d*** from the offset: %d",dataSize,pdyheadObj_offset));
        fseek(fout,current,SEEK_SET);
    }

    finish_mp4_box(fout,offset);

#ifdef BACKWARDS_V1_1    
    offset = start_mp4_extended_fullbox(fout,pssh_extended_type,0,0); 
    /* write_data(fout,(const uint8_t *)piff_cx->psshSystemId.systemId.data,16);  SystemID */
    write_data(fout,(const uint8_t *)piff_cx->systemId.data,PIFF_GUID_LENGTH); /* SystemID */

    dataSize_offset = ftell(fout); /* DataSize_offset */
    write_uint32_be(fout, 0);      /* DataSize place holder */

    /* only build when it is supplied */
    if( piff_cx->licAcqXMLStrLength > 0)
    {
        /*------------------------------------
         * build the PlayReady Header Object 
         * ------------------------------------*/
        pdyheadObj_offset = ftell(fout);  /* Data start */
        idx = 0;
        w_str = 0;
        dataSize = 0;
        fwrite(&dataSize,4,1,fout);    /* 4-byte length field (little endian) */
        fwrite(&recordCount,2,1,fout); /* 2-byte record count field (little endian) */
        fwrite(&recordType,2,1,fout);  /* 2-byte record type field (little endian) */
        rec_len_start_offset = ftell(fout);
        fwrite(&recordLen,2,1,fout);   /* 2-byte record length field (little endian) */

        raw_data_start_offset = ftell(fout);    /* position for raw data start */
        while( idx < piff_cx->licAcqXMLStrLength ) {
            w_str = piff_cx->pLicAcqXMLStr[idx];
            fwrite(&w_str,2,1,fout); /* 2-byte (utf-16) for each character (little endian) */
            idx++;
        }

        /* it is done, get the current position */
        current = ftell(fout);
        BDBG_MSG(("write the lic data complete, current position %d, data size: %d", current, current-raw_data_start_offset));

        /* write the raw date size to the record length field */ 
        fseek(fout,rec_len_start_offset,SEEK_SET);
        recordLen = (uint16_t) (current-raw_data_start_offset);
        BDBG_MSG(("write the record length %d", recordLen));

        /* write 2-byte of the record length field (little-endian) */
        fwrite(&recordLen,2,1,fout);  

        /* write the total data size + PlayReady header object into the length field of the PlayReady header object */
        fseek(fout,pdyheadObj_offset,SEEK_SET);
        dataSize = current-pdyheadObj_offset;
        BDBG_MSG(("write the length of the PlayReady Header object %d", dataSize));

        /* write 4-byte of the record length field (little-endian) */ 
        fwrite(&dataSize,4,1,fout);   

        /* also write the total data size + PlayReady header object into the DataSize field of the PSSH box */
        fseek(fout,dataSize_offset,SEEK_SET);
        BDBG_MSG(("write the DataSize to the PSSH object %d", dataSize));
        write_uint32_be(fout, dataSize);

        BDBG_MSG(("pdyheader - number of bytes : %d*** from the offset: %d",dataSize,pdyheadObj_offset));

        fseek(fout,current,SEEK_SET);
    }

    finish_mp4_box(fout,offset);

#endif /* BACKWARDS_V1_1 */

    fflush(fout);
      
    BDBG_MSG(("%s - Exiting function", __FUNCTION__));
}

static
int write_mdat_box(piff_media_context *media_cx, bpiff_encoder_context *piff_cx)
{
    int         rc=0;
    FILE       *fout;
    uint32_t    start;
    uint32_t    current;
    uint32_t    mdat_offset; 

    uint32_t      ii;
    uint32_t      jj;
    uint32_t      numOfSamples;

    BDBG_MSG(("%s - Entering function", __FUNCTION__));

    fout = piff_cx->destPiffFile;

    mdat_offset = start_mp4_box(fout,"mdat");

    start = ftell(fout); /* beginning of the payload */

    numOfSamples = media_cx->fragment.trun.nsamples;
    BDBG_MSG(("%s - Number of samples %d", __FUNCTION__, numOfSamples));
    for(ii=0;ii<numOfSamples;++ii) { 
        trun_sample *sample = &media_cx->fragment.trun.samples[ii]; 
        if( media_cx->track.type == BMP4_ISO_TrackType_eAudio) { 
            nal_payload *nal = &sample->nals[0]; 
            write_data(fout,(uint8_t *)nal->data,nal->actual_size);
            NEXUS_Memory_Free(nal->data);
        }
        else {
            /* write the encrypted sample to the temp file 
             * The layout of NALs within a sample is following:
             * |--------|--------|-------------------||--------|--------|------------------||---------- 
             * | 4-byte | 1-byte | NAL#1 payload     || 4-byte | 1-byte | NAL#2 payload    || Next NAL 
             * | length | type   | (length-1) bytes  || length | type   | (length-1) bytes || ...
             * |--------|--------|-------------------||--------|--------|------------------||----------
             * the length value includes the 1-byte type.
             * So, the actual size of the NAL payload will be length - 1.
             * The total size of a sample will be (4 + length ) * number of NALs
             * Only the NAL payload would be encrypted; the length and type fields will be left as 
             * clear.
             */
            for(jj=0 ; jj<sample->numOfNals; ++jj) {
                nal_payload *nal = &sample->nals[jj]; 
                BDBG_MSG(("  write the length field = %d",nal->actual_size+1));
                write_uint32_be(fout,nal->actual_size+1); /* include the 1 byte type field */ 
                BDBG_MSG(("  write the type field = %d",nal->type));
                write_uint8(fout,nal->type);
                BDBG_MSG(("  write the data field with address 0x%08x with size = %d",nal->data,nal->actual_size));
                write_data(fout,(uint8_t *)nal->data,nal->actual_size);
                /* we can now safely release the nal data buffer */
                BDBG_MSG(("the NAL is encrypted, releasing the memory size=%d", nal->alloc_size));
                NEXUS_Memory_Free(nal->data);
            } /* for */

        } /* else */

    } /* end for ii<numOfSamples */
     
    current = ftell(fout);
    finish_mp4_box(fout,mdat_offset);
 
    /* update the data_offset of the trun box */
    fseek(fout, media_cx->fragment.trun_data_offset_checkpoint, SEEK_SET);
    write_uint32_be(fout,start-media_cx->fragment.moof_checkpoint);

    fseek(fout, current, SEEK_SET);

    fflush(fout);

    BDBG_MSG(("%s - Exiting function", __FUNCTION__));

    return rc;
}

/************************************************************************************************* 
 * Description: This function constructs a PIFF compatible Sample encryption box. 
 * For v1.3:
 *     The InitializationVector, subsample_count, BytesOfClearData, and BytesOfEncryptedData fields 
 *     are represented using Sample Auxiliary Information using the CencSampleAuxiliaryDataFormat 
 *     structure defined in [CENC] Section 7. Since the [CENC] specification does not specify where 
 *     this data is stored within the file, it is RECOMMENDED (by PIFF Spec.) that the senc 
 *     version of this box be used to store the initialization vector and subsample encryption data 
 *     and that the SampleAuxiliaryInformationOffsetsBox (saio) contain a single OFFSET pointing 
 *     to the data contained in this box. 
 *
 * For v1.1 backwards compatibility:
 *     The uuid version of the box MAY be used either in addition to the senc version or 
 *     instead of the senc version for PIFF 1.1 compatibility per section 5.4 of this specification.
 *
 * input : bpiff_media_context and bpiff_encoder_context 
 *     
 *************************************************************************************************/
static
int write_sample_enc_box( piff_media_context *media_cx, bpiff_encoder_context *piff_cx)
{
    FILE             *fout;
    int               rc=0;
    int               sample_enc_box_offset;
    uint32_t          ii;
    trun_accumulator *run = &media_cx->fragment.trun;        

    BDBG_MSG(("%s - Entering function", __FUNCTION__));

    fout = piff_cx->destPiffFile;

    BDBG_ASSERT(piff_cx != NULL);
    BDBG_ASSERT(media_cx != NULL);

    /* start writing the sample encryption box 'senc' */
    if( media_cx->track.type == BMP4_ISO_TrackType_eAudio) { 
        sample_enc_box_offset = start_mp4_fullbox(fout,"senc",0,0);
    }
    else {
        sample_enc_box_offset = start_mp4_fullbox(fout,"senc",0,0x2);
    }


    write_uint32_be(fout,run->nsamples ); /* sample_count */

    /* mark the offset of the first IV for the 'saio' where there is having
     * a single offset pointing to the data here.
     * It shall be calculated as the difference between the first byte of
     * the containing 'moof' atom and the first byte of the first IV here 
     */ 
    media_cx->fragment.senc_data_offset_from_moof = ftell(fout)-media_cx->fragment.moof_checkpoint;

    /* support only IV size 8. 
     * In fact an IV size 16 doesn't give you more IVs since the entire 16 bytes will be used as
     * the counter value. In such case, IVs must have large enough numeric differences to prevent from
     * duplicating counter values for any encrypted block using the same KID. */ 

    if( media_cx->track.type == BMP4_ISO_TrackType_eAudio) { 
        for( ii = 0; ii < run->nsamples; ++ii) {
            /* write the IV */
            write_data(fout,&run->samples[ii].iv[8],8); 
        }
    }
    else {
        for( ii = 0; ii < run->nsamples; ++ii)
        {
            /* for each sample, extract the NALs information */
            uint32_t no_of_entries=run->samples[ii].numOfNals; 
            uint32_t entry_no=0;

            /* write the IV */
            write_data(fout,&run->samples[ii].iv[8],8); 

            /* write number of entries */
            write_uint16_be(fout,no_of_entries); 

            while( entry_no < no_of_entries && entry_no < PIFF_MAX_ENTRIES_PER_SAMPLE)
            {
                nal_payload *nal = &run->samples[ii].nals[entry_no];
                write_uint16_be(fout,5);                /* BytesOfClearData (length+type) */ 
                write_uint32_be(fout,nal->actual_size); /* BytesOfEncryptedData */ 
                /*
                 BDBG_MSG(("sample[%d] nal[%d] size=%d, type=%d\n",ii,entry_no,nal->actual_size,nal->type));
                 */
                entry_no++;
            } /* while */ 
        } /* end for each sample */
    } /* else eAudio */

    finish_mp4_box(fout,sample_enc_box_offset);

#ifdef BACKWARDS_V1_1    
    if( media_cx->track.type == BMP4_ISO_TrackType_eAudio) { 
        sample_enc_box_offset = start_mp4_extended_fullbox(fout,sample_enc_extended_type,0,0);
    }
    else {
        sample_enc_box_offset = start_mp4_extended_fullbox(fout,sample_enc_extended_type,0,0x2);
    }
    write_uint32_be(fout,run->nsamples ); /* sample_count */

    if( media_cx->track.type == BMP4_ISO_TrackType_eAudio) { 
        for( ii = 0; ii < run->nsamples; ++ii) {
            /* write the IV */
            write_data(fout,&run->samples[ii].iv[8],8); 
        }
    }
    else
    {
        /* for each sample, extract the NALs information */
        for( ii = 0; ii < run->nsamples; ++ii) {
            uint32_t no_of_entries=run->samples[ii].numOfNals; 
            uint32_t entry_no=0;

            /* write the IV to the file */
            write_data(fout,&run->samples[ii].iv[8],8); 
             
            /* write number of entries */
            write_uint16_be(fout,no_of_entries); 

            while( entry_no < no_of_entries && entry_no < PIFF_MAX_ENTRIES_PER_SAMPLE) {
                nal_payload *nal = &run->samples[ii].nals[entry_no];
                write_uint16_be(fout,5);                /* BytesOfClearData (length+type) */ 
                write_uint32_be(fout,nal->actual_size); /* BytesOfEncryptedData */ 
                /*
                BDBG_MSG(("sample[%d] nal[%d] size=%d, type=%d\n",ii,entry_no,nal->actual_size,nal->type));
                */
                entry_no++;
            } /* while */ 
        } /* end for each sample */
    }

    finish_mp4_box(fout,sample_enc_box_offset);
#endif /* BACKWARDS_V1_1  */ 

    fflush(fout);

    BDBG_MSG(("%s - exiting, rc %d", __FUNCTION__, rc));
    return rc;
}

static
int write_trun_box(piff_media_context * media_cx, bpiff_encoder_context *piff_cx)
{
    FILE             *fout;
    unsigned int      i; 
    uint32_t          tr_flags;  
    uint32_t          trun_offset;
    trun_accumulator *run = &media_cx->fragment.trun;        

    BDBG_MSG(("%s - Entering function", __FUNCTION__));

    fout = piff_cx->destPiffFile;

    /* 8.36. Track Fragment Run Box, data-offset, per-sample sample duration and sample size */
    tr_flags = 0x000001 | 0x0000100 | 0x000200; 
    if(run->need_sample_composition_time_offset) 
       tr_flags |= 0x000800;

    /* create the 'trun' box */
    trun_offset = start_mp4_fullbox(fout, "trun", 0, tr_flags);
    write_uint32_be(fout, run->nsamples); 

    /* mark the checkpoint of the trun data-offset field */
    media_cx->fragment.trun_data_offset_checkpoint = ftell(fout);

    write_uint32_be(fout, 0); /*place holder for the dat-offset */

    for(i=0; i<run->nsamples; i++) 
    {
       trun_sample *sample = run->samples + i;
       write_uint32_be(fout, sample->duration);
       write_uint32_be(fout, sample->size);
       if(run->need_sample_composition_time_offset) 
       {
           write_uint32_be(fout, sample->composition_time_offset);
       }
      
       /*
       BDBG_MSG(("sample:%u %u,%u,%u", i, sample->duration, sample->size, sample->composition_time_offset));
       */
    }

    finish_mp4_box(fout, trun_offset);

    fflush(fout);

    BDBG_MSG(("%s - Exiting function", __FUNCTION__));
    return 0;
}

/*************************************************************************************
 * For PIFF v1.3 only
 *
 * aligned(8) class SampleAuxiliaryInformationSizesBox
 * extends FullBox(saiz, version = 0, flags)
 * {
 * if (flags & 1) {
 * unsigned int(32) aux_info_type;
 * unsigned int(32) aux_info_type_parameter;
 * }
 * unsigned int(8) default_sample_info_size;
 * unsigned int(32) sample_count;
 * if (default_sample_info_size == 0) {
 * unsigned int(8) sample_info_size[ sample_count ];
 * }
 * }
 *  
 * aligned(8) class SampleAuxiliaryInformationOffsetsBox
 * extends FullBox(saio, version, flags)
 * {
 * if (flags & 1) {
 * unsigned int(32) aux_info_type;
 * unsigned int(32) aux_info_type_parameter;
 * }
 * unsigned int(32) entry_count;
 * if ( version == 0 ) {
 * unsigned int(32) offset[ entry_count ];
 * }
 * else {
 * unsigned int(64) offset[ entry_count ];
 * }
 * }
 **************************************************************************************/
static
int write_saiz_saio_boxes( piff_media_context  *media_cx, bpiff_encoder_context *piff_cx)
{
    uint32_t        offset;
    FILE           *fout;
    BDBG_MSG(("%s - Entering function", __FUNCTION__));

    fout = piff_cx->destPiffFile;

    /* -----------------------------------
     * 'saiz' box 
     * PIFF v1.3 5.4.3 - the sample_count field MUST match the sample_count in the 
     * SampleEncryptionBox. The default_sample_info_size MUST be zero if the size 
     * of the per sample information is not the same for all of the samples in the 
     * SampleEncryptionBox.
     */
    offset = start_mp4_fullbox(fout, "saiz", 0, 0x01);
    write_uint32_be(fout,BMP4_ISO_CENC);   /* aux_info_type = 'cenc'  */
    write_uint32_be(fout,0);               /* aux_info_type_parameter = 0  */
    write_uint8(fout,0);                   /* default sample_info size  */
    write_uint32_be(fout,media_cx->fragment.trun.nsamples); /* number of samples */
    write_uint8(fout,0);                   
    finish_mp4_box(fout,offset);

    /* -----------------------------------
     * 'saio' box 
     * PIFF v1.3 5.4.3 - the entry_count field SHALL be 1 (the data in the 
     * SampleEncryptionBox is contiguous for all of the samples in the movie fragment). 
     * Further, the offset field of SHALL be calculated as the difference between 
     * the first byte of the containing Movie Fragment (moof) atom and the first 
     * byte of the first InitializationVector in the SampleEncryptionBox (since movie 
     * fragment relative addressing where no base data offset is provided in the track 
     * fragment header is used in PIFF).
     */
    offset = start_mp4_fullbox(fout, "saio", 0, 0x01);
    write_uint32_be(fout,BMP4_ISO_CENC);   /* aux_info_type = 'cenc'  */
    write_uint32_be(fout,0);               /* aux_info_type_parameter = 0  */
    write_uint32_be(fout,1);               /* entry count  */
    write_uint32_be(fout,media_cx->fragment.senc_data_offset_from_moof);
    finish_mp4_box(fout,offset);

    fflush(fout);

    return 0;
}

static
int write_moof_box(piff_media_context * media_cx, bpiff_encoder_context *piff_cx)
{
    FILE           *fout;
    uint32_t        moof_offset; 
    uint32_t        mfhd_offset; 
    uint32_t        traf_offset; 
    uint32_t        tfhd_offset; 

    BDBG_MSG(("%s - Entering function", __FUNCTION__));

    fout = piff_cx->destPiffFile;

    /* mark the beginning of the moof, to be used by trun data-offset 
     * to calculate the payload starting address */ 
    media_cx->fragment.moof_checkpoint = ftell(fout); 

    /* create the 'moof' box 
     * +--moof
     *      |--mfhd
     *      +--traf
     *           |--tfhd
     *           |--trun
     *           |--uuid (PIFF v1.1 - sample encryption box) 
     *           |--saiz (PIFF v1.3 - sample auxiliary information size box) 
     *           |--saio (PIFF v1.3 - sample auxiliary information offsets box) 
     */
    moof_offset = start_mp4_box(fout, "moof");
   
    /* create the 'mfhd' box */
    mfhd_offset = start_mp4_fullbox(fout, "mfhd", 0, 0);
    write_uint32_be(fout, media_cx->fragment_no); /* seqeunece number */
    finish_mp4_box(fout, mfhd_offset);

    /* create the 'traf' box */
    traf_offset = start_mp4_box(fout, "traf"); 

    /* create the 'tfhd' box */
    tfhd_offset = start_mp4_fullbox(fout, "tfhd", 0, 0);
    write_uint32_be(fout, media_cx->track.trackId); /* track_ID */
    finish_mp4_box(fout, tfhd_offset);

    /* create the 'trun' box */
    write_trun_box(media_cx,piff_cx); 

    /* create the sample encryption box */
    write_sample_enc_box(media_cx,piff_cx); 

    /* create 'saiz' and 'saio' */
    write_saiz_saio_boxes(media_cx,piff_cx); 
    
    finish_mp4_box(fout, traf_offset);

    finish_mp4_box(fout, moof_offset);
 
    fflush(fout);

    BDBG_MSG(("%s - Exiting function", __FUNCTION__));

    return 0;
}

/****************************************************************
 * 14496-12 - 8.5.2.2
 * aligned(8) abstract class SampleEntry (unsigned int(32) format)
 * extends Box(format){
 * const unsigned int(8)[6] reserved = 0;
 * unsigned int(16) data_reference_index;
 * }
****************************************************************/
static
int write_sample_entry_box(FILE * fout)
{
    uint8_t i = 0;
    for( ; i<6; ++i) write_uint8(fout,0);
    write_uint16_be(fout,1);
    return 0;
}

static
int write_esds_box(piff_media_context * media_cx, bpiff_encoder_context *piff_cx)
{
    uint32_t  esds_offset;
    FILE     *fout;

    fout = piff_cx->destPiffFile;
    esds_offset = start_mp4_box(fout,"esds");  

    write_data(fout,media_cx->codec.aac.aac_config.data, media_cx->codec.aac.aac_config.size);

    finish_mp4_box(fout,esds_offset);

    fflush(fout);

    return 0;
}

static
int write_avcC_box(piff_media_context * media_cx, bpiff_encoder_context *piff_cx)
{
    uint32_t  avcC_offset;
    FILE     *fout;

    fout = piff_cx->destPiffFile;
    avcC_offset = start_mp4_box(fout,"avcC");  
    write_data(fout,media_cx->codec.h264.avc_config.data,media_cx->codec.h264.avc_config.size);
#if 0
    write_uint8(fout,BMP4_ISO_AVCC_CONFIG_VER);    
    write_uint8(fout,0);  /* TODO Profile */ 
    write_uint8(fout,0);  /* TODO Profile compatibility */  
    write_uint8(fout,0);  /* TODO level */  
    write_uint8(fout, BMP4_ISO_AVCC_RESERVED_3BITS | BMP4_ISO_AVCC_NALU_LENGTH_SIZE_MINUS_1);
    write_uint8(fout, BMP4_ISO_AVCC_RESERVED_3BITS | BMP4_ISO_AVCC_NUM_SEQ_PARAM_SETS);  /* num seq param sets = 1 */
#endif


    /* write_uint16_be(fout, 0);  AVC SPS Length */

    /* The following lines need to be uncommented once we know what and how to do them!
     * Note: look at \magnum\syslib\muxlib\src\file\mp4\bmuxlib_file_mp4_boxes.c -> 
     * CreateBoxAvcC() for references.... 
     
    for (i = 0; i < AVCSPSLength; i++)
      write_uint8(fout, AVCSPSData[i]);

    write_uint8(fout, BMP4_ISO_AVCC_NUM_PIC_PARAM_SETS );  
    write_uint16_be(fout, AVCPPSLength);
  
    for (i = 0; i < hMP4Mux->AVCPPSLength; i++)
      write_uint8(fout, AVCPPSData[i]);
    */

    finish_mp4_box(fout,avcC_offset);

    fflush(fout);

    return 0;
}

static
int write_stsd_box(piff_media_context * media_cx, bpiff_encoder_context *piff_cx)
{
    int              rc = 0;
    int              i;
    int              stsd_offset;
    int              sinf_offset;
    int              frma_offset;
    int              schm_offset;
    int              schi_offset;
    int              tenc_offset;  /* track encryption box offset */
    int              enc_offset;
    uint32_t         orig_codingname=0;
    FILE            *fout;

    /************************************************** 
     * create the 'stsd' box 
     * +--stsd
     *     +--encv (for video else 'enca' for audio) 
     *          |--avcC 
     ***************************************************/
    fout = piff_cx->destPiffFile;

    stsd_offset = start_mp4_fullbox(fout,"stsd", 0, 0 );  

    /* only one entry */
    write_uint32_be( fout, 1); 

    if( media_cx->track.type == BMP4_ISO_TrackType_eVideo) 
    {
        orig_codingname = BMP4_SAMPLE_MP4V;	 
        /* ---------------------
         * create a 'encv' box 
         * ---------------------*/

        enc_offset = start_mp4_box(fout,"encv");
        write_sample_entry_box(fout); 
        write_uint16_be( fout, 0); 
        write_uint16_be( fout, BMP4_ISO_RESERVED); 
        write_uint32_be( fout, 0); 
        write_uint32_be( fout, 0); 
        write_uint32_be( fout, 0); 
        write_uint16_be( fout, (uint16_t)(BMP4_ISO_DEFAULT_VIDEO_WIDTH )); 
        write_uint16_be( fout, (uint16_t)(BMP4_ISO_DEFAULT_VIDEO_HEIGHT ));
        write_uint32_be( fout, BMP4_ISO_VIDEO_RESOLUTION); 
        write_uint32_be( fout, BMP4_ISO_VIDEO_RESOLUTION); 
        write_uint32_be( fout, BMP4_ISO_RESERVED); 
        write_uint16_be( fout, BMP4_ISO_VIDEO_FRAME_COUNT); 
        for( i=0; i<32; ++i) write_uint8(fout,0); 
        write_uint16_be( fout, BMP4_ISO_VIDEO_DEPTH); 
        write_uint16_be( fout, BMP4_ISO_VIDEO_PREDEFINED); 

        /* create the ISO AvcC box */
        write_avcC_box(media_cx,piff_cx); 
    }
    else if( media_cx->track.type == BMP4_ISO_TrackType_eAudio) 
    {
        orig_codingname = BMP4_SAMPLE_MP4A;	 
        /* -------------------------
         * create 'enca' box
         * ------------------------*/
        enc_offset = start_mp4_box(fout,"enca");
        write_sample_entry_box(fout); 
        write_uint32_be( fout, BMP4_ISO_RESERVED); 
        write_uint32_be( fout, BMP4_ISO_RESERVED); 
        write_uint16_be( fout, BMP4_ISO_AUDIO_CHANNEL_COUNT); 
        write_uint16_be( fout, BMP4_ISO_AUDIO_SAMPLE_SIZE); 
        write_uint16_be( fout, 0); 
        write_uint16_be( fout, BMP4_ISO_RESERVED); 
        write_uint32_be( fout, 0); /* TODO sample rate */ 

        /* create the ISO esds box */
        write_esds_box( media_cx,piff_cx); 
    }
    else {
        /* not something we are expecting here */
        return -1;
    }

    /*******************************************
     * Now add +--sinf
     *             |--frma
     *             |--schm
     *             +--schi 
     *                 |--tenc
     ********************************************/
    sinf_offset = start_mp4_box(fout,"sinf");

    /* -------------------- 
     * frma box 
     * --------------------*/
    frma_offset = start_mp4_box(fout,"frma");
    write_uint32_be(fout, orig_codingname); 
    finish_mp4_box(fout,frma_offset);

    /* --------------------
     * schm box 
     * --------------------*/
    schm_offset = start_mp4_fullbox(fout,"schm", 0, 0 );  
    write_uint32_be(fout,BMP4_ISO_CENC);   /* scheme type 'cenc' */
    write_uint32_be(fout,BMP4_SCHM_PIFF_1_3_VERSION); /* scheme version 0x00010000 */
    finish_mp4_box(fout,schm_offset);

    /* --------------------
     * schi box 
     * --------------------*/
    schi_offset = start_mp4_box(fout,"schi");

    /* ------------------------------------------
     * insert the extended track encryption box 
     * - default isEncrypted (3-byte)
     * - default IV size (1-byte)
     * - default KID (16-byte)  
     * ------------------------------------------*/
    tenc_offset = start_mp4_fullbox(fout,"tenc",0,0);
    /*if( piff_cx->defaultSchemaInfo.algorithm != NEXUS_WmDrmPdEncryptionType_eAesCtr) */
    if( piff_cx->algorithm != PIFF_Encryption_Type_eAesCtr) { 
        BDBG_ERR(("%s - PIFF supports only AES-CTR algorithm! ", __FUNCTION__));
    }
    write_uint24_be(fout, 1);
    /*write_uint8(fout, (uint8_t) piff_cx->defaultSchemaInfo.ivSize ); */
    write_uint8(fout, (uint8_t) piff_cx->ivSize );
    /*write_data(fout,(const uint8_t *) piff_cx->defaultSchemaInfo.keyId,16); */
    write_data(fout,(const uint8_t *) piff_cx->keyId,PIFF_KEY_ID_LENGTH);
    finish_mp4_box(fout,tenc_offset);

    /* write the schi box size back to the offset */
    finish_mp4_box(fout,schi_offset);

    /* write the sinf box size back to the offset */
    finish_mp4_box(fout,sinf_offset);

#ifdef BACKWARDS_V1_1    
    /*******************************************
     * Now add +--sinf
     *             |--frma
     *             |--schm
     *             +--schi 
     *                 |--uuid
     ********************************************/
    sinf_offset = start_mp4_box(fout,"sinf");

    /* -------------------- 
     * frma box 
     * --------------------*/
    frma_offset = start_mp4_box(fout,"frma");
    write_uint32_be(fout, orig_codingname); 
    finish_mp4_box(fout,frma_offset);

    /* --------------------
     * schm box 
     * --------------------*/
    schm_offset = start_mp4_fullbox(fout,"schm", 0, 0 );  
    write_uint32_be(fout,BMP4_PIFF_SCHEMETYPE);   /* scheme type 'piff' */
    write_uint32_be(fout, BMP4_SCHM_PIFF_VERSION); /* scheme version 0x00010001 */
    finish_mp4_box(fout,schm_offset);

    /* --------------------
     * schi box 
     * --------------------*/
    schi_offset = start_mp4_box(fout,"schi");
    /* ------------------------------------------
     * insert the extended track encryption box 
     * - default isEncrypted (3-byte)
     * - default IV size (1-byte)
     * - default KID (16-byte)  
     * ------------------------------------------*/
    tenc_offset = start_mp4_extended_fullbox(fout,track_enc_extended_type,0,0);
    if( piff_cx->algorithm != PIFF_Encryption_Type_eAesCtr) { 
        BDBG_ERR(("%s - PIFF supports only AES-CTR algorithm! ", __FUNCTION__));
    }
    write_uint24_be(fout, 1); /* 1 = default_IsEncrypted */
    write_uint8(fout, (uint8_t) piff_cx->ivSize );
    write_data(fout,(const uint8_t *) piff_cx->keyId,PIFF_KEY_ID_LENGTH);
    finish_mp4_box(fout,tenc_offset);

    /* write the schi box size back to the offset */
    finish_mp4_box(fout,schi_offset);

    /* write the sinf box size back to the offset */
    finish_mp4_box(fout,sinf_offset);

#endif /* BACKWARDS_V1_1 */ 

    /* write the enc box size back to the enc_offset */
    finish_mp4_box(fout,enc_offset);

    /* write the stsd box size back to the offset */
    finish_mp4_box(fout,stsd_offset);

    fflush(fout);

    return rc;
} 

static
int write_stbl_box(piff_media_context * media_cx, bpiff_encoder_context *piff_cx)
{
    uint32_t stbl_offset=0; 
    FILE     *fout;

    fout = piff_cx->destPiffFile;

    /************************
     * create the "stbl" box
     ************************/
    stbl_offset = start_mp4_box(fout,"stbl");
    write_stsd_box(media_cx,piff_cx);
    finish_mp4_box(fout,stbl_offset);

    fflush(fout);

    return 0;
}

static
int write_dinf_box(piff_media_context * media_cx, bpiff_encoder_context *piff_cx)
{
    uint32_t dinf_offset=0; 
    uint32_t dref_offset=0; 
    uint32_t url_offset=0; 
    FILE     *fout;

    BSTD_UNUSED(media_cx); 

    fout = piff_cx->destPiffFile;
    /* create 'dinf' */
    dinf_offset = start_mp4_box(fout,"dinf");

    /* create 'dref' */
    dref_offset = start_mp4_fullbox(fout,"dref", 0, 0 );  
    write_uint32_be(fout, 1);   /* entry-count - one fixed entry */
    /* create 'url ' */
    url_offset = start_mp4_fullbox(fout,"url ", 0, 0x000001 );  
    finish_mp4_box(fout,url_offset);

    finish_mp4_box(fout,dref_offset);

    finish_mp4_box(fout,dinf_offset);

    fflush(fout);

    return 0;
}

static
int write_minf_box(piff_media_context * media_cx, bpiff_encoder_context *piff_cx)
{
    uint32_t minf_offset=0;
    FILE     *fout;

    fout = piff_cx->destPiffFile;

    /************************
     * create the "minf" box
     ************************/
    minf_offset = start_mp4_box(fout,"minf");
    if( media_cx->track.type == BMP4_ISO_TrackType_eVideo)
    {
        /************************
         * create the "vmhd" box
         ************************/
       uint32_t vmhd_offset=0; 
       vmhd_offset = start_mp4_fullbox(fout,"vmhd", 0, BMP4_ISO_VMHD_FLAG);  
       write_uint16_be(fout,0); /* graphics mode (copy) */
       write_uint16_be(fout,0); /* opcolor[0] - red */
       write_uint16_be(fout,0); /* opcolor[1] - green */
       write_uint16_be(fout,0); /* opcolor[2] - blue */
       finish_mp4_box(fout,vmhd_offset);
    } 
    else
    {
        /**********************************
         * create the "smhd" box for audio
         **********************************/
       uint32_t smhd_offset=0; 
       smhd_offset = start_mp4_fullbox(fout,"smhd", 0, 0);  
       write_uint16_be(fout,0); /* balance */
       write_uint16_be(fout,BMP4_ISO_RESERVED); 
       finish_mp4_box(fout,smhd_offset);
    }

    /************************
     * create the "dinf" box
     ************************/
    write_dinf_box( media_cx,piff_cx); 

    /************************
     * create the "stbl" box
     ************************/
    write_stbl_box( media_cx,piff_cx); 

    finish_mp4_box(fout,minf_offset);

    fflush(fout);

    return 0;
}

static
int write_mdia_box(piff_media_context * media_cx, bpiff_encoder_context *piff_cx)
{
    uint32_t mdia_offset=0;
    uint32_t mdhd_offset=0;
    uint32_t hdlr_offset=0;
    FILE     *fout;

    fout = piff_cx->destPiffFile;

    /************************
     * create the "mdia" box
     ************************/
    mdia_offset = start_mp4_box(fout,"mdia");

    /************************
     * create the "mdhd" box
     ************************/
    mdhd_offset = start_mp4_fullbox(fout,"mdhd", 0, 0 );  
    write_uint32_be(fout,0); /* TODO creation time */ 
    write_uint32_be(fout,0); /* TODO modification time */ 
    write_uint32_be(fout,BMP4_ISO_TIMESCALE_90KHZ); /* TODO timescale*/ 
    write_uint32_be(fout,media_cx->track.duration); /* TODO duration (in 90kHz timescale) */ 

    write_uint16_be(fout,BMP4_ISO_LANGUAGE_CODE_UND); /* packed ISO 639 language */
    write_uint16_be(fout,0); /* pre-defined */
    finish_mp4_box(fout,mdhd_offset);

    /************************
     * create the "hdlr" box
     ************************/
    hdlr_offset = start_mp4_fullbox(fout,"hdlr", 0, 0 );  
    write_uint32_be(fout,0); /* pre-defined */ 
    if( media_cx->track.type == BMP4_ISO_TrackType_eVideo) 
       write_fourcc(fout, "vide"); /* type */
    else
       write_fourcc(fout, "soun"); /* type */
    write_uint32_be(fout,BMP4_ISO_RESERVED); 
    write_uint32_be(fout,BMP4_ISO_RESERVED); 
    write_uint32_be(fout,BMP4_ISO_RESERVED); 
    write_uint8(fout,BMP4_ISO_NULL_CHAR); /* name - empty string */
    finish_mp4_box(fout,hdlr_offset);

    /* create the 'minf' box */
    write_minf_box(media_cx,piff_cx);

    finish_mp4_box(fout,mdia_offset);

    fflush(fout);

    return 0;
}

static
int write_trak_box(piff_media_context * media_cx, bpiff_encoder_context *piff_cx)
{
    uint32_t trak_offset=0;
    uint32_t tkhd_offset=0;
    uint32_t i;
    FILE     *fout;

    /* create the 'trak' box 
     * +--trak
     *      |--tkhd
     *      +--mdia
     *          |--mdhd
     *          |--hdlr
     *          +--minf
     *              |--vmhd (video)
     *              +--dinf
     *              |   +--dref
     *              |        --url
     *              +--stbl 
     *
     * */
    BDBG_MSG(("%s - Entering function", __FUNCTION__));

    fout = piff_cx->destPiffFile;

    /************************
     * create the "trak" box
     ************************/
    trak_offset = start_mp4_box(fout,"trak");

    /************************
     * create the "tkhd" box
     ************************/
    tkhd_offset = start_mp4_fullbox(fout,"tkhd", 0, 0 );  
    write_uint32_be(fout,0); /* TODO creation time */ 
    write_uint32_be(fout,0); /* TODO modification time */ 
    write_uint32_be(fout,media_cx->track.trackId); /* track ID */ 
    write_uint32_be(fout,BMP4_ISO_RESERVED); 
    write_uint32_be(fout,media_cx->track.duration); /* TODO duration (in 90kHz timescale) */ 

    write_uint32_be(fout,BMP4_ISO_RESERVED); 
    write_uint32_be(fout,BMP4_ISO_RESERVED); 
    write_uint16_be(fout,0); /* layer */
    write_uint16_be(fout,0); /* alternate group */
    write_uint16_be(fout,0); /* volumn */

    write_uint16_be(fout,BMP4_ISO_RESERVED);
    
    /* transformation matrix ... */
    for( i=0; i < 9; i++)
        write_uint32_be(fout,IdentityMatrix[i]);

    if( media_cx->track.type ==  BMP4_ISO_TrackType_eVideo)
    {
        write_uint32_be(fout,(uint32_t)( BMP4_ISO_DEFAULT_VIDEO_WIDTH << 16)); /* TODO track width (16.16 format) */
        write_uint32_be(fout,(uint32_t) (BMP4_ISO_DEFAULT_VIDEO_HEIGHT << 16)); /* TODO track height (16.16 format) */
    } 
    else {
        write_uint32_be(fout, 0); 
        write_uint32_be(fout, 0); 
    }

    finish_mp4_box(fout,tkhd_offset);

    /* create media box */
    write_mdia_box(media_cx,piff_cx); 

    finish_mp4_box(fout,trak_offset);

    fflush(fout);

    BDBG_MSG(("%s - Exiting function", __FUNCTION__));

    return 0;
}

static
int write_moov_box(bpiff_encoder_context *piff_cx)
{
    int             rc = 0;
    int             i;
    uint32_t        moov_offset; 
    uint32_t        mvhd_offset; 
    FILE           *fout;

    /* create the 'moov' box 
     * +--moov
     *      |--mvhd
     *      |--uuid (PSSH)
     *      +--trak (audio)
     *      +--trak (video)
     * */
    BDBG_MSG(("%s - Entering function", __FUNCTION__));

    fout = piff_cx->destPiffFile;

    /************************
     * create the "moov" box 
     ************************/
    moov_offset = start_mp4_box(fout,"moov"); 

    /************************
     * create the "mvhd" box
     ************************/
    mvhd_offset = start_mp4_fullbox(fout,"mvhd", 0, 0 );  
    write_uint32_be(fout,0); /* TODO creation time */ 
    write_uint32_be(fout,0); /* TODO modification time */ 
    /* timescale is always 90kHz since this is the inherent timing of the presentation ... */
    write_uint32_be(fout,BMP4_ISO_TIMESCALE_90KHZ); 
    write_uint32_be(fout,piff_cx->video_context.track.duration); 

    write_uint32_be(fout,BMP4_ISO_RATE_1_0);   /* = 0x00010000 in 16.16 fixed = 1.0 (normal playback rate) */
    write_uint16_be(fout,BMP4_ISO_VOLUME_1_0); /* = 0x0100 in 8.8 fixed = 1.0 (full volume) */
    write_uint16_be(fout,BMP4_ISO_RESERVED); 
    write_uint32_be(fout,BMP4_ISO_RESERVED); 
    write_uint32_be(fout,BMP4_ISO_RESERVED); 

    /* transformation matrix ... */
    for( i=0; i < 9; i++)
        write_uint32_be(fout,IdentityMatrix[i]); 

    /* predefined ... (older spec compatibility) */
    for( i=0; i < 6; i++)
        write_uint32_be(fout,0); 
    
    /* set next_track_id */
    write_uint32_be(fout,1); 
    finish_mp4_box(fout,mvhd_offset);
     
    /********************
     * add the pssh box 
     ********************/
    write_pssh_box( piff_cx, fout); 

    /********************************************
     * create the trak boxes for audio and video
     *******************************************/
    write_trak_box(&piff_cx->audio_context, piff_cx);  /*audio track*/ 
    write_trak_box(&piff_cx->video_context, piff_cx);  /*video track*/ 

    finish_mp4_box(fout,moov_offset);

    fflush(fout);

    BDBG_MSG(("%s - Exiting function", __FUNCTION__));

    return rc;
}

static
int write_ftyp_box( bpiff_encoder_context * piff_cx)
{
    int ftyp_offset = 0;  
    FILE * fout;

    BDBG_MSG(("%s - Entering function", __FUNCTION__));

    fout = piff_cx->destPiffFile;
    fseek(fout, 0, SEEK_SET);
    ftyp_offset = start_mp4_box(fout,"ftyp"); 
    write_fourcc(fout,"mp42");
    write_uint32_be(fout,BMP4_ISO_FTYP_MINOR_VER);     

   /* compatibility brands */
    write_fourcc(fout,"mp42");
    write_fourcc(fout,"mp41");
    write_fourcc(fout,"isom");
    write_fourcc(fout,"piff");
    finish_mp4_box(fout,ftyp_offset); 

    fflush(fout);

    BDBG_MSG(("%s - Exiting function", __FUNCTION__));

    return 0;
}

/*****************************************************************************************
 * static media fragmenting functions
 * Note: based on \rockford\unittests\nexus\playback\mp4_fragment\encode_mp4fragment.c
 *****************************************************************************************/
static 
void media_context_init(piff_media_context *context)
{
    BDBG_MSG(("%s - Entering function", __FUNCTION__));

    context->fragment_in_progress = false;
    context->codec.h264.avc_config_valid = false;
    context->codec.aac.aac_config_valid = false;
    context->audio.next_pts_valid = false;
    context->audio.fragment_length = 1000;
    context->frame_no = 0;
    context->fragment_no = 0;
    context->encoding_status = 0;

    BDBG_MSG(("%s - Exiting function", __FUNCTION__));
    return;
}


static 
void memory_buffer_reset(memory_buffer *m) 
{
    m->offset = 0;
    m->entries = 0;
    m->nalu_len_offset = 0;
    return;
}

static 
int memory_buffer_start_nal(memory_buffer *m, uint8_t nalu)
{
    BDBG_MSG(("%p, %u,nal:%#x", m, m->offset, nalu));
    if(m->offset + 2 + 1>= sizeof(m->buf)) {
        return -1;
    }
    m->nalu_len_offset = m->offset;
    m->offset += 2 + 1;
    m->entries ++;
    SAVE_UINT16_BE(m->buf+m->nalu_len_offset, 1);
    m->buf[m->nalu_len_offset + 2] = nalu;
    return 0;
}

static 
int memory_buffer_add_data(memory_buffer *m, const void *data, size_t len) 
{
    size_t nalu_len;

    BDBG_MSG(("%p, %d data:%#x,%#x", m, len, ((uint8_t *)data)[0], ((uint8_t *)data)[1]));
    if(m->offset + len >= sizeof(m->buf)) {
        return -1;
    }
    nalu_len = LOAD_UINT16_BE(m->buf, m->nalu_len_offset);
    if(((size_t)nalu_len + len) >= (1<<16)) {
        return -1;
    }
    nalu_len += len;
    BKNI_Memcpy(m->buf+m->offset, data, len);
    SAVE_UINT16_BE(m->buf+m->nalu_len_offset, nalu_len);
    m->offset += len;
    return 0;
}

static 
int accum_nal_to_memory( nal_payload *nal, const void *data, size_t len)
{
    uint32_t need_size = len;
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_MemoryAllocationSettings allocSettings;
    
    BDBG_MSG(("%s - Entering function", __FUNCTION__));

    if(nal != NULL) {
        need_size += nal->actual_size;
        if( nal->alloc_size >= need_size) 
        {
            /* append this partial nal to the existing nal's buffer */
            BDBG_MSG(("    adding nal size=%d", len));
            BKNI_Memcpy(nal->data+nal->actual_size, data, len);	
            nal->actual_size += len;
        }
        else 
        {
            NEXUS_Memory_GetDefaultAllocationSettings(&allocSettings);
            /* need to increase the buffer size but try not to over allocate */
            if( (nal->actual_size == 0) && (need_size < MAX_NAL_BUF_SIZE)) 
            {
                BDBG_MSG(("    allocating memory for nal size=%d, alloc size=%d", len,MAX_NAL_BUF_SIZE));
                /* first time allocation 
                 * allocate MAX_NAL_BUF_SIZE, it should be big enough 
                 * for one complete NAL within a sample */  
                rc = NEXUS_Memory_Allocate(MAX_NAL_BUF_SIZE, &allocSettings, (void *)&nal->data);
                if(rc != NEXUS_SUCCESS) {
                    BDBG_ERR(("%s - NEXUS_Memory_Allocate failed for the NAL, rc = %d\n", __FUNCTION__, rc));
                    rc = -1;
                    goto ErrorExit;
                }
                BKNI_Memset(nal->data, 0, MAX_NAL_BUF_SIZE);
                nal->alloc_size = MAX_NAL_BUF_SIZE; 

                /* cpy the nal */
                BKNI_Memcpy(nal->data, data, len);	
                nal->actual_size = len;
            }
            else 
            {
                /* This is a very rare case, but still need to handle it if happens 
                 * So, resize the original buffer and move over everthing into it  */ 
                uint8_t *tmp;
                if(nal->actual_size > 0) 
                {
                    BDBG_MSG(("    allocating temp memory for nal, size=%d", nal->actual_size));
                    rc = NEXUS_Memory_Allocate(nal->actual_size, &allocSettings, (void *)&tmp);
                    if(rc != NEXUS_SUCCESS) {
                        BDBG_ERR(("%s - NEXUS_Memory_Allocate failed for tmp NAL, rc = %d\n", __FUNCTION__, rc));
                        rc = -1;
                        goto ErrorExit;
                    }
                    BKNI_Memcpy(tmp, nal->data, nal->actual_size);	
                    NEXUS_Memory_Free(nal->data);

                    BDBG_MSG(("    allocating new memory for nal, original size=%d, new size=%d", nal->actual_size,need_size));

                    rc = NEXUS_Memory_Allocate(need_size, &allocSettings, (void *)&nal->data);
                    if(rc != NEXUS_SUCCESS) {
                        BDBG_ERR(("%s - NEXUS_Memory_Allocate failed for the new NAL size, rc = %d\n", __FUNCTION__, rc));
                        rc = -1;
                        goto ErrorExit;
                    }
                    /* allocate the new size */
                    BKNI_Memcpy(nal->data, tmp, nal->actual_size);	
                    BKNI_Memcpy(nal->data+nal->actual_size, data, len);	

                    /* release the tmp buffer */
                    BDBG_MSG(("    releasing temp memory for nal, size=%d", nal->actual_size));
                    NEXUS_Memory_Free(tmp);
                }
                else 
                {
                    BDBG_MSG(("    allocating new memory for nal, original size=%d, new size=%d", nal->actual_size,need_size));
                    rc = NEXUS_Memory_Allocate(need_size, &allocSettings, (void *)&nal->data);
                    if(rc != NEXUS_SUCCESS) {
                        BDBG_ERR(("%s - NEXUS_Memory_Allocate failed for the new NAL size, rc = %d\n", __FUNCTION__, rc));
                        rc = -1;
                        goto ErrorExit;
                    }
                    BKNI_Memcpy(nal->data, data, len);	
                }
                nal->alloc_size = need_size; 
                nal->actual_size = need_size;
            }             
        }
    }

ErrorExit:
    BDBG_MSG(("%s - Exiting function", __FUNCTION__));
    return rc;
}

static 
void nal_writer_start(struct nal_writer *w, uint8_t nal_type, nal_payload *nal)
{
    w->nal_size = 1;  /* 1 means that the total size of the nal will include the 1-byte type field */

    if( nal != NULL)
    {
        nal->alloc_size = 0;
        nal->actual_size = 0;
        nal->type = nal_type;
        nal->data = NULL;
    }
}

static 
void audio_data_init(nal_payload *audio_data)
{
    if( audio_data != NULL)
    {
        audio_data->alloc_size = 0;
        audio_data->actual_size = 0;
        audio_data->data = NULL;
    }
}
static
void audio_add_data( const void *data, size_t len, nal_payload *audio_payload)
{
    BDBG_MSG(("%s - Entering function", __FUNCTION__));

    /* We want to encrypt before writing to the file.  */
    if( audio_payload != NULL) {
        accum_nal_to_memory(audio_payload,data,len);
    }
    BDBG_MSG(("%s - Exiting function", __FUNCTION__));
}

static 
void nal_writer_add_data(struct nal_writer *w, void *data, size_t len, nal_payload *nal)
{
    uint8_t tmpData[8];
    BKNI_Memcpy(tmpData,data,8);
    BDBG_MSG(("%s - Entering function", __FUNCTION__));
    /* DUMP_DATA_HEX("ADD Nal first 8-byte: ",tmpData,8); */

    /* We want to encrypt before writing to the file.  */
    w->nal_size += len;
    
    if( nal != NULL) {
        accum_nal_to_memory(nal,data,len);
    }
    BDBG_MSG(("%s - Exiting function", __FUNCTION__));
}

static 
int nal_writer_finish(struct nal_writer *w )
{
    /* return the total size of the NAL including : length+nal*/
    return w->nal_size+4;
}

static 
void frame_state_init(frame_state *f)
{
    f->dts = f->pts = 0;
    f->dts_valid = f->pts_valid = false;
}

static 
void frame_state_set_pts(frame_state *f, uint64_t pts)
{
    f->pts = pts;
    f->pts_valid = true;
}

static 
void frame_state_set_dts(frame_state *f, uint64_t dts)
{
    f->dts = dts;
    f->dts_valid = false;
}

static 
void trun_accumulator_start(trun_accumulator *r)
{
    r->nsamples = 0;
    r->need_sample_composition_time_offset = false;
    BKNI_Memset(r->samples, 0, sizeof(trun_sample)*MAX_NUM_SAMPLES_TRUN);
}

/*************************************************************************** 
 * This function writes the processed sample's metadata to trun structure. 
 * payload_size = the size of a sample
 * For video, one sample contains multiple nal(s)
 * Encrypt the sample on the memory at the end
 ***************************************************************************/
static 
int trun_accumulator_add_sample( 
        trun_accumulator      *r, 
        frame_state           *f, 
        size_t                 payload_size, 
        piff_media_context    *media_cx, 
        bpiff_encoder_context *piff_cx)
{
    int            rc=0;
    trun_sample   *sample;
    uint64_t       sample_time;

    BDBG_MSG(("%s - Entering function", __FUNCTION__));
    
    BDBG_ASSERT(f->pts_valid);
    BDBG_MSG(("sizeof nsample [%d] checking [%d] \n",r->nsamples,sizeof(r->samples)/sizeof(*r->samples)));
    if(r->nsamples >= sizeof(r->samples)/sizeof(*r->samples)) { 
        rc = -1;
        goto ErrorExit;
    }
    sample = r->samples + r->nsamples;
    sample_time = f->dts_valid ?  f->dts : f->pts;
    if(r->nsamples==0) { 
        r->first_sample_time = sample_time;
    }
    else {
        sample[-1].duration = sample_time - r->prev_sample_time;
        BDBG_MSG(("sample[%d] duration: %d\n",r->nsamples - 1,sample[-1].duration));
    }
     
    r->nsamples ++;
    r->prev_sample_time = sample_time;
    sample->duration = 0;
    sample->size = payload_size;
    if(f->dts_valid) {
        sample->composition_time_offset = f->pts - f->dts;
        r->need_sample_composition_time_offset = true;;
    } 
    else {
        sample->composition_time_offset = 0;
    }

    /* encrypt it */
    if( encrypt_sample(media_cx, piff_cx,sample) != 0) {
        BDBG_ERR(("%s - failed to encrypt the sample, can't continue.", __FUNCTION__));
        if( media_cx->track.type == BMP4_ISO_TrackType_eAudio) 
            media_cx->encoding_status = AUDIO_ENCODING_ERROR;         
        else
            media_cx->encoding_status = VIDEO_ENCODING_ERROR;         
        rc = -1;
        goto ErrorExit;
    } 

    BDBG_MSG(("Sample encrypted!"));

ErrorExit:
    BDBG_MSG(("%s - Exiting function", __FUNCTION__));
    return rc;
}

static 
int trun_accumulator_stop(trun_accumulator *r)
{
    switch(r->nsamples) 
    {
    case 0: break;
    case 1: r->samples[0].duration = 1; break;
    default: 
    {
        trun_sample *sample = r->samples + (r->nsamples - 1);
        sample->duration = sample[-1].duration;
    }
    break;
    } 
    return 0;
}

static 
int avc_decoder_configuration_make(struct decoder_configuration *d, const memory_buffer *sps, const memory_buffer *pps)
{
    unsigned pps_offset;

    /* ISO-IEC 14496-15 5.2.4.1. AVC Decoder Configuration Record */
    d->data[0] = 0x01;        /* version */
    d->data[1] = sps->buf[4]; /*AVCProfileIndication */
    d->data[2] = sps->buf[5]; /*profile_compatibility */
    d->data[3] = sps->buf[6]; /*AVCLevelIndication */
    d->data[4] = 0xFF;        /* lengthSizeMinusOne == 2 */
    d->data[5] = 0xE0 | sps->entries;
    BDBG_MSG(("sps:%p(%u) pps:%p(%u)", sps, sps->offset, pps, pps->offset));
    BKNI_Memcpy(d->data+6, sps->buf, sps->offset);
    pps_offset = 6 + sps->offset;
    d->data[pps_offset] = pps->entries;
    BKNI_Memcpy(d->data+pps_offset+1, pps->buf, pps->offset);
    d->size = pps_offset+1+pps->offset;
    return 0;
}

static 
int start_fragment(piff_media_context *media_cx)
{
    int rc = 0;

    BDBG_MSG(("%s - Entering function", __FUNCTION__));

    media_cx->fragment_in_progress = true;
    trun_accumulator_start(&media_cx->fragment.trun);

    BDBG_MSG(("%s - Exiting function", __FUNCTION__));
    return rc;
}

/****************************************************************** 
 * This function encodes the fragment into PIFF format. 
 * Fragment will be encrypted and stored in a temporary file. 
 * The temporary file will be removed once a mdat box is created.
 ******************************************************************/
static 
int finish_fragment(piff_media_context *media_cx, bpiff_encoder_context *piff_cx)
{
    int      rc = 0;
    uint32_t ii = 0;

    BDBG_MSG(("%s - Entered function", __FUNCTION__));
    if( piff_cx == NULL) {
       BDBG_ERR(("%s - PIFF encoder handle is NULL ", __FUNCTION__));
       return -1;
    }
    if(!piff_cx->lock ) {
       BDBG_ERR(("%s - failed to get next available fragment number, the lock is not available\n", __FUNCTION__));
       return -1;
    }

    rc = get_next_fragment_no(piff_cx, &media_cx->fragment_no);
    if( rc != 0) {
        BDBG_ERR(("%s - failed to get the next fragment number, can't continue.", __FUNCTION__));
        return rc; 
    }

    trun_accumulator_stop(&media_cx->fragment.trun);
    BDBG_MSG(("num of samples: %d",media_cx->fragment.trun.nsamples));
     
    /* update the track duration */
    for( ; ii < media_cx->fragment.trun.nsamples; ++ii) {
        media_cx->track.duration += media_cx->fragment.trun.samples[ii].duration;
        BDBG_MSG(("sample [%d] duration: %d accu : %d\n",ii,media_cx->fragment.trun.samples[ii].duration,media_cx->track.duration));
    }

    /* we need to protect the output file for writting */
    BKNI_AcquireMutex(piff_cx->lock);

    if( (piff_cx->state & STATE_CONTEXT_PROCESSED_FTYP) != STATE_CONTEXT_PROCESSED_FTYP ) {
        write_ftyp_box(piff_cx); 
        piff_cx->state |=  STATE_CONTEXT_PROCESSED_FTYP; 
    }

    /* create the 'moof' box for the fragment */
    write_moof_box(media_cx,piff_cx);

    /* now create the 'mdat' box */
    write_mdat_box(media_cx, piff_cx);

    BKNI_ReleaseMutex(piff_cx->lock);

    BDBG_MSG(("%s - Exiting function", __FUNCTION__));

    return rc;
}

/****************************************************************** 
 * Function to finalize the final PIFF output file. 
 * This function is not thread-safe that it should only be called 
 * from the main process thread.
 ******************************************************************/
static
int finalize(bpiff_encoder_context *piff_cx)
{
   /*************************************************** 
     * According to 14496-12, 'moov' box can be placed
     * either close to the beginning or end of the file
     * though this is not required. It also means that 
     * it can be located anywhere in the file.
     * We are simply append it to the end of the file.
     ***************************************************/
     write_moov_box(piff_cx);

     fclose(piff_cx->destPiffFile );
     return 0;
}

/***************************************************
 * Function checks if needs to stop PIFF encoding. 
 * This function is thread-safe.
 ***************************************************/
static
bool terminate(bpiff_encoder_context *piff_cx)
{
   bool rc = false;
   BDBG_MSG(("%s - Entering function", __FUNCTION__));
   if(piff_cx->lock != NULL) {
        BKNI_AcquireMutex(piff_cx->lock);
        if( (piff_cx->state & STATE_CONTEXT_TERMINATE_ENCODE) == STATE_CONTEXT_TERMINATE_ENCODE){
            rc = true;
        }
        BKNI_ReleaseMutex(piff_cx->lock);
   } 
   BDBG_MSG(("%s - Exiting function", __FUNCTION__));
   return rc;
}

/*****************************************************************************************************
 * Function to construct a fragment for the audio frames (ES) directly from AudioMuxOutputFrame 
 * When it is done, the payload is encrypted and written in a temporary file.  
 *
 * Note: code are copied from \rockford\unittests\nexus\playback\mp4_fragment\encode_mp4fragment.c
 *****************************************************************************************************/
static 
int process_audio_frame(piff_media_context              *media_cx, 
                        const NEXUS_AudioMuxOutputFrame *desc, 
                        size_t                           n_desc, 
                        const NEXUS_AudioMuxOutputFrame *ext_desc, 
                        size_t                           n_ext_desc,
                        bpiff_encoder_context           *piff_context)
{ 
    bool frame_started = false;
    unsigned frame_size = 0;
    struct frame_state frame;
    void *pDataBuffer;

    BDBG_MSG(("%s - Entered function", __FUNCTION__));

    frame_state_init(&frame);
    for(;;) {
        unsigned i;
        for(i=0;i<n_desc;i++) {
            const NEXUS_AudioMuxOutputFrame *d = desc+i;
            if(d->flags & NEXUS_AUDIOMUXOUTPUTFRAME_FLAG_FRAME_START) {
                frame_started = true;
                frame_size = 0;
            }
            if(d->flags & NEXUS_AUDIOMUXOUTPUTFRAME_FLAG_PTS_VALID) {
                if(!media_cx->audio.next_pts_valid || d->pts >= media_cx->audio.next_pts) {
                    if(media_cx->fragment_in_progress) {
                        media_cx->fragment_in_progress = false;

                        /* encrypts the fragment and appends it to the output file */
                        finish_fragment(media_cx,piff_context);
                        /* check if needs to terminate */ 
                        if( terminate(piff_context)) {
                            BDBG_MSG(("Terminating..."));
                            return -1;
                        }
                    }
                    media_cx->audio.next_pts = d->pts + (media_cx->audio.fragment_length * 90);
                    media_cx->audio.next_pts_valid = true;
                    start_fragment(media_cx);
                    audio_data_init(&media_cx->fragment.trun.samples[media_cx->fragment.trun.nsamples].nals[0]); 
                }
                frame_state_set_pts(&frame, d->pts);
                BDBG_MSG(("P:%p:frame:%u pts:%u", d, media_cx->frame_no, (unsigned)(d->pts/2)));
            }
            if(frame_started) {
                if(d->rawDataLength) {
                    trun_sample *run = &media_cx->fragment.trun.samples[media_cx->fragment.trun.nsamples]; /* always the first entry */ 
                    BDBG_ASSERT(media_cx->fragment_in_progress);
                    frame_size += d->rawDataLength;
                    /* assuming each audio sample is small enough to fit in memory */
                    NEXUS_MemoryBlock_Lock(media_cx->audio.outputStatus.bufferBlock, &pDataBuffer);
                    audio_add_data(((const uint8_t *)pDataBuffer)+d->rawDataOffset, d->rawDataLength,&run->nals[0]);
                }
            }
        } /* end for */

        if(n_ext_desc==0) {
            break;
        }
        desc = ext_desc;
        n_desc = n_ext_desc;
        n_ext_desc = 0;
        /* run one more time */

    } /* for */
    if(frame_size) {
        if( trun_accumulator_add_sample(&media_cx->fragment.trun, &frame, frame_size, media_cx, piff_context) != 0)
            return -1;
    }
    BDBG_MSG(("frame:%u size:%u", media_cx->frame_no, frame_size));
    media_cx->frame_no++;

    BDBG_MSG(("%s - Exiting function", __FUNCTION__));

    return 0;
}

/*****************************************************************************************************
 * Function to construct a fragment for the video frames (ES) directly from video encoder. 
 * When it is done, the payload is encrypted and written to the output file 
 *
 * Note: code are copied from \rockford\unittests\nexus\playback\mp4_fragment\encode_mp4fragment.c
 *****************************************************************************************************/
static 
int process_video_frame(piff_media_context *media_cx, 
                        const NEXUS_VideoEncoderDescriptor *desc, 
                        size_t n_desc, 
                        const NEXUS_VideoEncoderDescriptor *ext_desc, 
                        size_t n_ext_desc,
                        bpiff_encoder_context *piff_context)
{
    frame_state frame;
    bool nal_valid=false;
    bool nal_writer_started=false;
    bool frame_started = false;
    unsigned frame_size = 0;
    unsigned nal_unit_type=0;
    unsigned nal_offset=0;
    enum {capture_pps, capture_sps, save_payload} state = save_payload;
    nal_writer nal_writer;

    BDBG_MSG(("%s - Entered function", __FUNCTION__));

    nal_writer.nal_size = 0;
    nal_writer.nal_size_offset = 0;
    
    frame_state_init(&frame);
    memory_buffer_reset(&media_cx->codec.h264.pps);
    memory_buffer_reset(&media_cx->codec.h264.sps);
    
    for(;;) 
    {
        unsigned i;
        for(i=0;i<n_desc;i++) 
        {
            const NEXUS_VideoEncoderDescriptor *d = desc+i;
            size_t d_length = d->length;
            if(d->videoFlags & NEXUS_VIDEOENCODERDESCRIPTOR_VIDEOFLAG_DATA_UNIT_START) 
            {
                const char *type;
                nal_unit_type = d->dataUnitType & 0x1F;
                switch(nal_unit_type) 
                { /* H.264 Table 7-1 NAL Unit type codes */
                case 0: type = "Unspecified"; break;
                case 1: type = "Slice NON IDR"; break;
                case 2: case 3: case 4: type = "Slice"; break;
                case 5: type = "Slice IDR"; break;
                case 6: type = "SEI"; break;
                case 7: type = "SPS"; break;
                case 8: type = "PPS"; break;
                case 9: type = "AUD"; break;
                case 10: type = "EOSeq"; break;
                case 11: type = "EOS"; break;
                case 12: type = "Filler"; break;
                case 13: type = "SPS Ext"; break;
                case 14: type = "Prefix"; break;
                case 15: type = "SSPS"; break;
                case 19: type = "Slice AUX"; break;
                case 20: type = "Slice Ext"; break;
                default: type = "Unknown"; break;
                }

                BDBG_MSG(("P:%p:frame:%u nal:%#x(%u:%s)", d, media_cx->frame_no, d->dataUnitType, nal_unit_type, type));
                if(nal_writer_started) {
                    nal_writer_started = false;
                    frame_size += nal_writer_finish(&nal_writer);
                    /* frame_size is accumulated to be used for the sample size */
                }

                nal_offset = 0;
                nal_valid = true;
                switch(nal_unit_type) 
                {
                case 7: state = capture_sps; break;
                case 8: state = capture_pps; break;
                case 5: 
                        BDBG_ASSERT(!frame_started);
                        if(media_cx->fragment_in_progress) {
                            media_cx->fragment_in_progress = false;

                            /* encrypts the fragment and appends it to the output file */
                            finish_fragment(media_cx,piff_context);

                            /* check if needs to terminate */ 
                            if( terminate(piff_context)) {
                                BDBG_MSG(("Terminating..."));
                                return -1;
                            }
                        }
                        start_fragment(media_cx);
                        /* continue */
                case 1: case 2: case 3: case 4: case 19: case 20: 
                        if(!frame_started) 
                        {
                           frame_started = true;
                        }
                        state = save_payload; 
                        break;
                }
            }

            if((d->flags & NEXUS_VIDEOENCODERDESCRIPTOR_FLAG_PTS_VALID) || 
               (d->videoFlags & NEXUS_VIDEOENCODERDESCRIPTOR_VIDEOFLAG_DTS_VALID)) {
                if(d->flags & NEXUS_VIDEOENCODERDESCRIPTOR_FLAG_PTS_VALID) {
                    frame_state_set_pts(&frame, d->pts);
                }
                if(d->flags & NEXUS_VIDEOENCODERDESCRIPTOR_VIDEOFLAG_DTS_VALID) {
                    frame_state_set_dts(&frame, d->dts);
                }
                BDBG_MSG(("P:%p:frame:%u pts:%u dts:%u", d, media_cx->frame_no, (unsigned)(d->pts/2), (unsigned)(d->dts/2)));
            }

            if(nal_valid) 
            {
                size_t payload;
                size_t payload_offset = 0;
                if(nal_offset < 2) { /* skip over 00 00 */
                    payload = 2 - nal_offset;
                    if(payload > d_length) {
                        payload = d_length;
                    }
                    nal_offset += payload;
                    payload_offset += payload;
                    d_length -= payload;
                }
                /* find 0x01 */
                while(nal_offset==2 && d_length) 
                {
                    /*uint8_t byte = ((uint8_t *)media_cx->video.encoderStatus.bufferBase)[d->offset + payload_offset];*/
                    uint8_t byte;
                    byte = media_cx->video.bufferBase[d->offset + payload_offset];
                    payload_offset ++;
                    d_length--;
                    switch(byte) 
                    {
                    case 0x01: nal_offset=3; break;
                    case 0x00: break;
                    default: BDBG_ASSERT(0);
                    } 
                }
                
                /* get nal type */
                if(nal_offset==3 && d_length) 
                {
                    /*uint8_t nal = ((uint8_t *)media_cx->video.encoderStatus.bufferBase)[d->offset + payload_offset];*/
                    uint8_t nal = media_cx->video.bufferBase[d->offset + payload_offset];
                    switch(state) {
                    case capture_pps: memory_buffer_start_nal(&media_cx->codec.h264.pps, nal); break;
                    case capture_sps: memory_buffer_start_nal(&media_cx->codec.h264.sps, nal); break;
                    case save_payload: 
                         {
                             /* a new NAL detected, start a new buffer */
                             uint8_t curr = media_cx->fragment.trun.nsamples; 
                             trun_sample *run = &media_cx->fragment.trun.samples[curr]; 
                             BDBG_MSG(("NAL start"));
                             nal_writer_start(&nal_writer, nal, &run->nals[run->numOfNals]);
                             nal_writer_started = true; 
                             run->numOfNals++; /* TODO check if numOfNals > PIFF_MAX_ENTRIES_PER_SAMPLE */
                             break;
                         }
                    } /* switch */
                    payload_offset++;
                    nal_offset++;
                    d_length--;
                }

                /* save payload */
                if(nal_offset>3 && d_length) 
                {
                    nal_offset += d_length;
                    switch(state) 
                    {
                    case capture_pps: memory_buffer_add_data(&media_cx->codec.h264.pps, (uint8_t *)media_cx->video.bufferBase + d->offset + payload_offset, d_length); break;
                    case capture_sps: memory_buffer_add_data(&media_cx->codec.h264.sps, (uint8_t *)media_cx->video.bufferBase + d->offset + payload_offset, d_length); break;
                    case save_payload: 
                         {
                             /* an incomplete NAL detected, we append the data to the last nal */
                             uint8_t curr = media_cx->fragment.trun.nsamples; 
                             trun_sample *run = &media_cx->fragment.trun.samples[curr]; 
                             uint8_t curr_nal = run->numOfNals-1;
                             BDBG_MSG(("NAL Added..."));
                             nal_writer_add_data(&nal_writer, 
                                                 (uint8_t *)media_cx->video.bufferBase + d->offset + payload_offset,
                                                 d_length,
                                                 &run->nals[curr_nal] ); 
                             /* ++run->numOfNals */;
                             break;
                         }
                    }
                }
            }
        } /* for(i=0;i<n_desc;i++) */
        BDBG_MSG(("FOR loop done"));
         
        if(n_ext_desc==0) break;
        desc = ext_desc;
        n_desc = n_ext_desc;
        n_ext_desc = 0;
        /* run one more time */
    } /* for(;;) */ 

    if(nal_writer_started) {
        /* add the contigous nal to the sample */
        frame_size += nal_writer_finish(&nal_writer);
    }
    if(frame_started) {
        /* finish one sample, frame_size is this sample size */
        if( trun_accumulator_add_sample(&media_cx->fragment.trun, &frame, frame_size, media_cx, piff_context) != 0)
            return -1;
    }
    BDBG_MSG(("frame:%u size:%u", media_cx->frame_no, frame_size));
    media_cx->frame_no++;
    if(!media_cx->codec.h264.avc_config_valid) {
        if(media_cx->codec.h264.pps.offset && media_cx->codec.h264.sps.offset) {
            avc_decoder_configuration_make(&media_cx->codec.h264.avc_config, &media_cx->codec.h264.sps, &media_cx->codec.h264.pps);
            media_cx->codec.h264.avc_config_valid = true;
        }
    }

    BDBG_MSG(("%s - Exiting function", __FUNCTION__));

    return 0;
}

static 
void * encode_video( void * context)
{
    NEXUS_VideoEncoderHandle  videoEncoder;  
    bpiff_encoder_context    *piff_ctx;
    piff_media_context       *media_ctx; 
    size_t                    bytes;
    unsigned                  descs;
     
    BDBG_MSG(("%s - Entering function", __FUNCTION__));

    piff_ctx = (bpiff_encoder_context *) context;
    if( piff_ctx == NULL) {
        BDBG_ERR(("%s - PIFF encoder handle is NULL ", __FUNCTION__));
        goto ErrorExit;
    }

    videoEncoder = piff_ctx->video_encoder_handle;
    if( videoEncoder == NULL) {
        BDBG_ERR(("%s - videoEncoder is NULL ", __FUNCTION__));
        goto ErrorExit;
    }

    /* initialize the the video media context structure which 
     * will be used through out the creation of every vidoe fragment */
    media_ctx = &piff_ctx->video_context; 
    media_context_init(media_ctx);

    NEXUS_VideoEncoder_GetStatus(videoEncoder, &media_ctx->video.encoderStatus); 
    {
        void *base;
        NEXUS_Error rc;
        rc = NEXUS_MemoryBlock_Lock(media_ctx->video.encoderStatus.bufferBlock, &base);
        media_ctx->video.bufferBase = base;

    }
    media_ctx->track.trackId = 2; /* assuming video on track #2 */ 
    media_ctx->track.type = BMP4_ISO_TrackType_eVideo;
    media_ctx->encoding_status |= VIDEO_ENCODING_START;          

    /*----------------------------- 
     * main loop to fragment the stream from the encoder 
     * Note: code are copied from \rockford\unittests\nexus\playback\mp4_fragment\encode_mp4fragment.c
     */
    for(bytes=0;;) {
        size_t size[2];
        const NEXUS_VideoEncoderDescriptor *desc[2];
        unsigned i,j;
        unsigned frame_start_i, frame_start_j;
        unsigned frame_no=0;
        
        NEXUS_VideoEncoder_GetBuffer(videoEncoder, &desc[0], &size[0], &desc[1], &size[1]);
        if(size[0]==0 && size[1]==0) {
            BKNI_Sleep(30);
            continue;
        }

        BDBG_MSG(("descs:%p[%u],%p[%u]", desc[0],size[0],desc[1],size[1]));
        for(frame_start_i=0, frame_start_j=0,descs=0,i=0,j=0;;) 
        {
            const NEXUS_VideoEncoderDescriptor *d;
            if(i>=size[j]) {
                i=0;
                j++;
                if(j>=2) break;
                continue;
            }

            d = &desc[j][i];
            i++;
            if(d->flags & NEXUS_VIDEOENCODERDESCRIPTOR_FLAG_FRAME_START) 
            {
                BDBG_MSG(("%p:frame %u", d, frame_no));
                if(frame_no!=0) {
                    if(j==frame_start_j) {
                        descs += i-frame_start_i;
                        if(process_video_frame(media_ctx, desc[frame_start_j]+frame_start_i, 
                                               i-frame_start_i, NULL, 0, piff_ctx)!=0) {
                            goto done_encoding;
                        }
                    } 
                    else {
                        descs += (size[frame_start_j]-frame_start_i) + i;
                        if(process_video_frame(media_ctx, desc[frame_start_j]+frame_start_i, 
                                               size[frame_start_j]-frame_start_i, desc[j], i, piff_ctx)!=0) {
                            goto done_encoding;
                        }
                    }
                }
                frame_start_i=i;
                frame_start_j=j;
                frame_no++;
            }
            if(d->videoFlags & NEXUS_VIDEOENCODERDESCRIPTOR_VIDEOFLAG_DATA_UNIT_START) {
               /*BDBG_MSG(("%p:frame:%u nal:%#x", d, frame_no, d->dataUnitType));
                */
            }
            if((d->flags & NEXUS_VIDEOENCODERDESCRIPTOR_FLAG_PTS_VALID) || 
               (d->videoFlags & NEXUS_VIDEOENCODERDESCRIPTOR_VIDEOFLAG_DTS_VALID)) {
               /*BDBG_MSG(("%p:frame:%u pts:%u dts:%u", d, frame_no, (unsigned)(d->pts/2), (unsigned)(d->dts/2)));
                */
            }
        }
        if(descs) {
            /*BDBG_MSG(("descs:%u(%u)", descs, size[0]+size[1]));
             */
            NEXUS_VideoEncoder_ReadComplete(videoEncoder, descs);
        } 
        else {
           BKNI_Sleep(30);
        }
    } /* for */

done_encoding:
    if(media_ctx->fragment_in_progress) {
        media_ctx->fragment_in_progress = false;
        finish_fragment(media_ctx,piff_ctx); 
    }
     
    NEXUS_MemoryBlock_Unlock(media_ctx->video.encoderStatus.bufferBlock);
    media_ctx->encoding_status |= VIDEO_ENCODING_COMPLETE;          

    BDBG_MSG(("%s - Exiting function", __FUNCTION__));
    return NULL;

ErrorExit:

    BDBG_MSG(("%s - Exiting function", __FUNCTION__));
    return NULL;
}

static 
void * encode_audio( void * context)
{
    NEXUS_AudioMuxOutputHandle  audioMuxOutput;
    bpiff_encoder_context      *piff_ctx;
    piff_media_context         *media_ctx; 
    size_t                      bytes;
    unsigned                    descs;
    void *pMetadataBuffer;
     
    BDBG_MSG(("%s - Entering function", __FUNCTION__));

    piff_ctx = (bpiff_encoder_context *) context;
    if( piff_ctx == NULL) {
        BDBG_ERR(("%s - PIFF encoder handle is NULL ", __FUNCTION__));
        goto ErrorExit;
    }

    audioMuxOutput = piff_ctx->audio_mux_output_handle;
    if( audioMuxOutput == NULL) {
        BDBG_ERR(("%s - audioMuxOutput is NULL ", __FUNCTION__));
        goto ErrorExit;
    }

    /* initialize the the audio media context structure which 
     * will be used through out the creation of every audio fragment */
    media_ctx = &piff_ctx->audio_context; 
    media_context_init(media_ctx);
    NEXUS_AudioMuxOutput_GetStatus(audioMuxOutput, &media_ctx->audio.outputStatus);
    
    media_ctx->track.trackId = 1; /* always assuming track# 1 is audio */
    media_ctx->track.type = BMP4_ISO_TrackType_eAudio;
    media_ctx->encoding_status |= AUDIO_ENCODING_START;          

    /*-------------------------------- 
     * main loop to fragment the stream from the encoder 
     * Note: code are copied from \rockford\unittests\nexus\playback\mp4_fragment\encode_mp4fragment.c
     */
    for(bytes=0;;) {
        size_t size[2];
        const NEXUS_AudioMuxOutputFrame *desc[2];
        unsigned i,j;
        unsigned frame_start_i, frame_start_j;
        unsigned frame_no=0;

        NEXUS_AudioMuxOutput_GetBuffer(audioMuxOutput, &desc[0], &size[0], &desc[1], &size[1]);
        if(size[0]==0 && size[1]==0) {
            BKNI_Sleep(30);
            continue;
        }
        BDBG_MSG(("descs:%p[%u],%p[%u]", desc[0],size[0],desc[1],size[1]));
        for(frame_start_i=0, frame_start_j=0,descs=0,i=0,j=0;;) {
            const NEXUS_AudioMuxOutputFrame *d;
            if(i>=size[j]) {
                i=0;
                j++;
                if(j>=2) {
                    break;
                }
                continue;
            }
            d = &desc[j][i];
            i++;
            if(!media_ctx->codec.aac.aac_config_valid  && d->flags & NEXUS_AUDIOMUXOUTPUTFRAME_FLAG_METADATA) {
                NEXUS_MemoryBlock_Lock(media_ctx->audio.outputStatus.metadataBufferBlock, &pMetadataBuffer);
                const NEXUS_AudioMetadataDescriptor *meta = (void *)(((const uint8_t *)pMetadataBuffer)+d->offset);
                BDBG_ASSERT(meta->protocolData.aac.ascLengthBytes < sizeof(media_ctx->codec.aac.aac_config.data));
                BKNI_Memcpy(media_ctx->codec.aac.aac_config.data, meta->protocolData.aac.asc, meta->protocolData.aac.ascLengthBytes);
                media_ctx->codec.aac.aac_config.size = meta->protocolData.aac.ascLengthBytes;
                media_ctx->codec.aac.aac_config_valid = true;
            }
            if(d->flags & NEXUS_AUDIOMUXOUTPUTFRAME_FLAG_FRAME_START) {
                BDBG_MSG(("%p:frame %u", d, frame_no));
                if(frame_no!=0) {
                    if(j==frame_start_j) {
                        descs += i-frame_start_i;
                        if(process_audio_frame(media_ctx, desc[frame_start_j]+frame_start_i, i-frame_start_i, NULL, 0, piff_ctx )!=0) {
                            goto done_encoding;
                        }
                    } else {
                        descs += (size[frame_start_j]-frame_start_i) + i;
                        if(process_audio_frame(media_ctx, desc[frame_start_j]+frame_start_i, size[frame_start_j]-frame_start_i, desc[j], i, piff_ctx )!=0) {
                            goto done_encoding;
                        }
                    }
                }
                frame_start_i=i;
                frame_start_j=j;
                frame_no++;
            }
            if((d->flags & NEXUS_AUDIOMUXOUTPUTFRAME_FLAG_PTS_VALID)) {
               BDBG_MSG(("%p:frame:%u pts:%u" , d, frame_no, (unsigned)(d->pts/2)));
            }
        }
        if(descs) {
            BDBG_MSG(("descs:%u(%u)", descs, size[0]+size[1]));
            NEXUS_AudioMuxOutput_ReadComplete(audioMuxOutput, descs);
        } else {
           BKNI_Sleep(30);
        }
    }

done_encoding:
    if(media_ctx->fragment_in_progress) {
        media_ctx->fragment_in_progress = false;
        finish_fragment(media_ctx,piff_ctx); 
    }
    media_ctx->encoding_status |= AUDIO_ENCODING_COMPLETE;          
     

ErrorExit:

    BDBG_MSG(("%s - Exiting function", __FUNCTION__));
    return NULL;
}

static 
void * main_process( void * context)
{
    bpiff_encoder_context *piff_ctx;
    pthread_t              encode_video_thread_id;
    pthread_t              encode_audio_thread_id;

    if( context == NULL) {
        BDBG_ERR(("%s - Context is null.", __FUNCTION__));
        return NULL;
    }
    piff_ctx = (bpiff_encoder_context *) context;

    /* create audio and video threads to encode the media separately */
    pthread_create(&encode_audio_thread_id, NULL, encode_audio, piff_ctx);
    pthread_create(&encode_video_thread_id, NULL, encode_video, piff_ctx);

    /* wait for them to complete */

    pthread_join(encode_audio_thread_id, NULL);
    pthread_join(encode_video_thread_id, NULL);

    /* if no error was encountered, finalize the PIFF file */
    if( (piff_ctx->audio_context.encoding_status & AUDIO_ENCODING_ERROR ) || 
        (piff_ctx->video_context.encoding_status & VIDEO_ENCODING_ERROR )) {
       BDBG_ERR(("%s - PIFF encoding failed.", __FUNCTION__));
    }
    else {
       finalize(piff_ctx);
       BDBG_MSG(("%s - PIFF encoding complete.", __FUNCTION__));
    }

    if( piff_ctx->callback != NULL) {
       (*piff_ctx->callback)(piff_ctx->callbackContext); 
    }
    else {
        BDBG_ERR(("No callback provided in the configuration"));
    }

    return NULL;
}

/****************************************************************************
 ****************************************************************************
 *
 *                  public interface implementations
 *
 ****************************************************************************
 ****************************************************************************/
void piff_GetDefaultSettings( PIFF_Encoder_Settings *pSettings)    /* [out] default settings */
{
    BDBG_MSG(("%s - Entering function", __FUNCTION__));
    BDBG_ASSERT(NULL != pSettings);

    BKNI_Memset(pSettings, 0, sizeof(PIFF_Encoder_Settings));
    BKNI_Memcpy(pSettings->systemId.data, playready_sys_id, 16);
    pSettings->algorithm = PIFF_Encryption_Type_eAesCtr;
    pSettings->ivSize = 8; 
    pSettings->licAcqLAURL      = NULL;
    pSettings->licAcqLAURLLen   = 0;
    pSettings->licAcqLUIURL     = NULL;
    pSettings->licAcqLUIURLLen  = 0;
    pSettings->licAcqKeyId      = NULL;
    pSettings->licAcqKeyLen     = 0;
    pSettings->licAcqDSId       = NULL;
    pSettings->licAcqDSIdLen    = 0;
    pSettings->licCustomAttr    = NULL;
    pSettings->licCustomAttrLen = 0;

    pSettings->completionCallBack.callback = NULL;
    pSettings->completionCallBack.context = NULL;

    if( DRM_Prdy_LocalLicense_InitializePolicyDescriptor( &pSettings->licPolicyDescriptor) != DRM_Prdy_ok)
    {
        BDBG_WRN(("%s - Failed to initialize License Policy Descriptor. ", __FUNCTION__));
    }

    pSettings->licPolicyDescriptor.wSecurityLevel = 2000;
    pSettings->licPolicyDescriptor.fCannotPersist = 0;
    pSettings->licPolicyDescriptor.cPlayEnablers = 2;
    BKNI_Memcpy(&pSettings->licPolicyDescriptor.rgoPlayEnablers[0], 
                &DRM_PRDY_ND_PLAYENABLER_UNKNOWN_OUTPUT, sizeof(DRM_Prdy_guid_t));   
    BKNI_Memcpy(&pSettings->licPolicyDescriptor.rgoPlayEnablers[1], 
                &DRM_PRDY_ND_PLAYENABLER_CONSTRAINED_RESOLUTION_UNKNOWN_OUTPUT, sizeof(DRM_Prdy_guid_t));   
    /* set the license type to local_license_bound_simple */
    pSettings->licType = eDRM_Prdy_eLocal_license_bound_simple;

    BDBG_MSG(("%s - Exiting function", __FUNCTION__));
}

void piff_destroy_encoder_handle(PIFF_encoder_handle handle)
{
    BDBG_MSG(("%s - Entering function", __FUNCTION__));
    if( handle != NULL) { 
        DRM_Prdy_LocalLicense_Release(&handle->phLicense);
        if( handle->licAcqXMLStrLength > 0) {
            if( handle->pLicAcqXMLStr != NULL) NEXUS_Memory_Free(handle->pLicAcqXMLStr);
        }
        if( handle->lock != NULL) BKNI_DestroyMutex(handle->lock);
        NEXUS_Memory_Free(handle);
    }
    BDBG_MSG(("%s - Exiting function", __FUNCTION__));
}  

PIFF_encoder_handle piff_create_encoder_handle(PIFF_Encoder_Settings *pSettings, DRM_Prdy_Handle_t  pPrdyDrmCtx)
{
    NEXUS_MemoryAllocationSettings allocSettings;
    NEXUS_Error rc = NEXUS_SUCCESS;
    PIFF_encoder_handle handle=NULL;

    BDBG_MSG(("%s - Entering function", __FUNCTION__));

    BDBG_ASSERT(pSettings != NULL);
    BDBG_ASSERT(pSettings->destPiffFileName != NULL);
    BDBG_ASSERT(pPrdyDrmCtx != NULL);

    NEXUS_Memory_GetDefaultAllocationSettings(&allocSettings); 
    rc = NEXUS_Memory_Allocate(sizeof(bpiff_encoder_context), &allocSettings, (void *)&handle);
    if(rc != NEXUS_SUCCESS) {
        BDBG_ERR(("%s - NEXUS_Memory_Allocate failed for PIFF encoder handle, rc = %d\n", __FUNCTION__, rc));
        (void)BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto ErrorExit; 
    }
    
    if ( NULL == handle ) {
        (void)BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto ErrorExit; 
    }

    BKNI_Memset(handle, 0, sizeof(bpiff_encoder_context));

    BKNI_Memcpy(handle->systemId.data, pSettings->systemId.data, 16);
    handle->prdyDrmCtx = pPrdyDrmCtx; 
    handle->algorithm = pSettings->algorithm;  
    handle->ivSize = pSettings->ivSize;  

    if( pSettings->completionCallBack.callback == NULL)
    {
        BDBG_ERR(("No callback provided in the configuration"));
        goto ErrorExit; 
    }
    else {
        handle->callback = pSettings->completionCallBack.callback;
        handle->callbackContext = pSettings->completionCallBack.context;
    }
     
    /*
    BKNI_Memcpy(handle->settings.destPiffFileName, pSettings->destPiffFileName, strlen(pSettings->destPiffFileName));
    */
    handle->destPiffFile = fopen(pSettings->destPiffFileName, "wb");
    if(handle->destPiffFile == NULL) {
        BDBG_ERR(("%s - stderr,failed to open destination PIFF file to write %s\n.", __FUNCTION__, pSettings->destPiffFileName));
        goto ErrorExit;
    }

    
    /*
    handle->licAcqXMLStrLength = 0; 
    if(pSettings->licAcqXMLStrLength > 0)
    {
        rc = NEXUS_Memory_Allocate(pSettings->licAcqXMLStrLength, NULL, (void *)&handle->pLicAcqXMLStr);
        if(rc != NEXUS_SUCCESS) {
            BDBG_ERR(("%s - NEXUS_Memory_Allocate failed for the license acquisition string, rc = %d\n", __FUNCTION__, rc));
            goto ErrorExit; 
        }
        BKNI_Memcpy(handle->pLicAcqXMLStr, pSettings->pLicAcqXMLStr, pSettings->licAcqXMLStrLength);
        handle->licAcqXMLStrLength = pSettings->licAcqXMLStrLength;
    }
    */

    /* randomly generate the first IV */
    if( gen_random_num(16, handle->next_iv) != 0) {
        BDBG_ERR(("%s - Failed to generate next iv.", __FUNCTION__));
        goto ErrorExit;
    }
    
    /* keyId is not provided, generate it here */
    if( gen_random_num(16, &pSettings->keyId.data[0]) != 0) {
        BDBG_ERR(("%s - Failed to generate key ID.", __FUNCTION__));
        goto ErrorExit; 
    }

    if( init_license(handle,pSettings) != 0) {
        BDBG_ERR(("%s - init_license FAILED", __FUNCTION__));
        goto ErrorExit;
    }
     
    BKNI_Memcpy(&handle->keyId, &pSettings->keyId.data[0], PIFF_KEY_ID_LENGTH);

    /* still build the default PlayReady Header object 
     * it will contains the KID in base64 format
     */
    build_lic_acq_xml_string(handle,pSettings);
    /*printf("license XML string: %s\n",handle->pLicAcqXMLStr);*/
    BDBG_MSG(("license XML string: %s\n",handle->pLicAcqXMLStr));

    if( BKNI_CreateMutex(&handle->lock) != BERR_SUCCESS) {
        BDBG_ERR(("failed to create mutex.", __FUNCTION__));
        goto ErrorExit;
    }

    handle->next_fragment_no = 1; /* start with 1 for the sequence number of mfhd box */ 

    handle->state = STATE_CONTEXT_INITIALIZED;
    BDBG_MSG(("%s - Exiting function", __FUNCTION__));
    return handle;

ErrorExit:
    piff_destroy_encoder_handle(handle); 
    BDBG_MSG(("%s - Exiting function", __FUNCTION__));
    return NULL;
}

BPIFF_Error piff_encode_start(
        NEXUS_AudioMuxOutputHandle audioMuxOutput, 
        NEXUS_VideoEncoderHandle   videoEncoder, 
        PIFF_encoder_handle        piffHandle)
{
    pthread_t   thread_id;
    unsigned    rc = PIFF_ENCODE_SUCCESS;

    BDBG_MSG(("%s - Entering function", __FUNCTION__));

    if( audioMuxOutput == NULL) {
        BDBG_ERR(("%s - Audio Mux Output Handler is NULL ", __FUNCTION__));
        rc = PIFF_ENCODE_INVALID_AUDIO_HANDLE;
        goto ErrorExit;
    }
    if( videoEncoder == NULL) {
        BDBG_ERR(("%s - videoEncoder is NULL ", __FUNCTION__));
        rc = PIFF_ENCODE_INVALID_VIDEO_HANDLE;
        goto ErrorExit;
    }
    if( piffHandle == NULL) {
        BDBG_ERR(("%s - PIFF encoder handle is NULL ", __FUNCTION__));
        rc = PIFF_ENCODE_INVALID_HANDLE;
        goto ErrorExit;
    }

    piffHandle->audio_mux_output_handle = audioMuxOutput; 
    piffHandle->video_encoder_handle = videoEncoder; 

    pthread_create(&thread_id, NULL, main_process, piffHandle);

ErrorExit:
    BDBG_MSG(("%s - Exiting function", __FUNCTION__));
    return rc;
} 

BPIFF_Error piff_encode_stop( PIFF_encoder_handle piffHandle)
{
   BDBG_MSG(("%s - Entering function", __FUNCTION__));
   unsigned rc = PIFF_ENCODE_SUCCESS;

   if( piffHandle == NULL) {
       BDBG_ERR(("%s - PIFF encoder handle is NULL ", __FUNCTION__));
       rc = PIFF_ENCODE_INVALID_HANDLE;
       goto ErrorExit;
   }
   if(!piffHandle->lock ) {
       BDBG_ERR(("%s - failed to stop encoding, the lock is not available\n", __FUNCTION__));
       rc = PIFF_ENCODE_INVALID_LOCK;
       goto ErrorExit;
   }
   else {
       BKNI_AcquireMutex(piffHandle->lock);
       piffHandle->state |= STATE_CONTEXT_TERMINATE_ENCODE;
       BKNI_ReleaseMutex(piffHandle->lock);
   } 

ErrorExit:
   BDBG_MSG(("%s - Exiting function", __FUNCTION__));
   return rc;
}

BPIFF_Error piff_get_key_id( PIFF_encoder_handle piffHandle, uint8_t *pKeyId )
{
   BDBG_MSG(("%s - Entering function", __FUNCTION__));
   unsigned rc = PIFF_ENCODE_SUCCESS;
   
   if( piffHandle == NULL) {
       BDBG_ERR(("%s - PIFF encoder handle is NULL ", __FUNCTION__));
       rc = PIFF_ENCODE_INVALID_HANDLE;
       goto ErrorExit;
   }
   if( pKeyId == NULL)
   {
       BDBG_ERR(("%s - the given KeyId buffer is NULL ", __FUNCTION__));
       rc = PIFF_ENCODE_INVALID_ARG;
   }  

   BKNI_Memcpy(pKeyId, piffHandle->keyId, PIFF_KEY_ID_LENGTH);

ErrorExit:
   BDBG_MSG(("%s - Exiting function", __FUNCTION__));
   return rc;
}

BPIFF_Error  piff_get_key_id_based64W( PIFF_encoder_handle piffHandle, uint16_t *pKeyIdB64W, uint32_t *pkeyIdSize )
{
   BDBG_MSG(("%s - Entering function", __FUNCTION__));
   unsigned rc = PIFF_ENCODE_SUCCESS;
   
   if( piffHandle == NULL) {
       BDBG_ERR(("%s - PIFF encoder handle is NULL ", __FUNCTION__));
       rc = PIFF_ENCODE_INVALID_HANDLE;
       goto ErrorExit;
   }
   if( pKeyIdB64W == NULL)
   {
       BDBG_ERR(("%s - the given KeyId buffer is NULL ", __FUNCTION__));
       rc = PIFF_ENCODE_INVALID_ARG;
   }  

   if( piffHandle->phLicense == NULL)
   {
       BDBG_ERR(("%s - PIFF encoder handle for the licnese is NULL ", __FUNCTION__));
       rc = PIFF_ENCODE_INVALID_HANDLE;
       goto ErrorExit;
   } 

   if( DRM_Prdy_LocalLicense_GetKID_base64W(piffHandle->phLicense,
                                             pKeyIdB64W,
                                             pkeyIdSize) != DRM_Prdy_ok)
   {
       BDBG_ERR(("%s - GetKID failed", __FUNCTION__));
       rc = PIFF_ENCODE_KEY_ID_NOT_AVAILABLE;     
       goto ErrorExit;
   }

ErrorExit:
   BDBG_MSG(("%s - Exiting function", __FUNCTION__));
   return rc;
}

