/***************************************************************************
*     (c)2004-2013 Broadcom Corporation
*
*  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
*
* $brcm_Workfile: $
* $brcm_Revision: $
* $brcm_Date: $
*
* API Description:
*
* Revision History:
*
* $brcm_Log: $
*
*
*
***************************************************************************/

#include "nexus_playback.h"
#include "nexus_playback_impl.h"
#include "nexus_playback_piff.h"
#include "nexus_dma.h"
#include "nexus_platform_client.h"
#include "nexus_core_utils.h"

#include "bmp4_player.h"
#include "bmp4_util.h"
#include "bpiff.h"

#include "drm_prdy.h"

BDBG_MODULE(nexus_playback_piff);

/**************************************************************************
 * Defines
 **************************************************************************/

#define BDBG_MSG_TRACE(x)  /*BDBG_MSG(x)*/

#define BUFFER_LENGTH       (16*4096)
#define READ_LENGTH         (BUFFER_LENGTH)
/*
 * Much of the code are borrowed from prdy_mp4.c
 */

#define BMP4_PROPRITARY_BMP4        BMP4_TYPE('b','m','p','4')
#define BMP4_SAMPLE_AUX_INFO_SIZE   BMP4_TYPE('s','a','i','z')
#define BMP4_SAMPLE_AUX_INFO_OFFSET BMP4_TYPE('s','a','i','o')
#define BMP4_SAMPLE_TO_GROUP        BMP4_TYPE('s','b','g','p')
#define BMP4_SAMPLE_GROUP_DESC      BMP4_TYPE('s','g','p','d')

#define CONVERT_ALG_ID_TO_DRM_ENCR_STATE(_alg_id, _state)       \
    do {                                                        \
        if (_alg_id == 0) _state = piff_encryption_none;        \
        else if(_alg_id == 1) _state = piff_encryption_aes_ctr; \
        else _state = piff_encryption_aes_cbc;                  \
    } while (0)

typedef enum piff_err {
    piff_err_ok = 0,
    piff_err_fail,
    piff_err_buffer_size                   /* buffer too small */
} piff_err;

typedef enum piff_encryption {
    piff_encryption_none     = 0,          /* no encryption */
    piff_encryption_aes_ctr  = 1,          /* AES CTR encrypted stream */
    piff_encryption_aes_cbc  = 2,          /* AES CBC encrypted stream */
    piff_encryption_max
} piff_encryption;

#define KEY_ID_LENGTH (16)
typedef struct piff_scheme {
    piff_encryption alg_id;
    uint8_t iv_size;
    uint8_t kid[KEY_ID_LENGTH];
} piff_scheme;

typedef struct piff_subsample {
    uint16_t bytesOfClearData;
    uint32_t bytesOfEncData;
} piff_subsample;

/* Pool size for sub-sample entries in a single sample */
#define MAX_SUBSAMPLE_ENTRIES (10)
typedef struct piff_se_sample {
    uint8_t iv[16];    /* If IV size is 8, iv[0-7] contains the IV. */
    uint16_t num_subsample_entries;
    piff_subsample subsample_entries[MAX_SUBSAMPLE_ENTRIES];
} piff_se_sample;

/* Pool size for piff_se_sample */
#define SAMPLES_POOL_SIZE (500)
typedef struct piff_se {
    bmp4_box_extended extended;     /* TODO: need to keep this? */
    bmp4_fullbox fullbox;           /* Need to keep flags */
    piff_scheme se_scheme;
    uint32_t sample_count;
    piff_se_sample samples[SAMPLES_POOL_SIZE];

    uint32_t track_id;  /* track that this se box belongs to */
    bool used;          /* this object is being used */
} piff_se;

/* Pool size for sample encryption box.
 * Should be >= total num tracks in the stream */
#define MAX_SAMPLE_ENC_BOX       (5)

typedef struct piff_sample_info {
    piff_se se[MAX_SAMPLE_ENC_BOX];  /* Encryption parameters from the SamspleEncryptionBox */
    piff_scheme scheme;              /* Ultimate encryption parameters used to decrypt samples */
    uint32_t sample_count_saiz;      /* sample_count read from saiz box */
    uint8_t default_sample_info_size;
    uint8_t sample_info_size[SAMPLES_POOL_SIZE];
} piff_sample_info;

typedef struct piff_drm {
    bpiff_mp4_headers header;
    piff_sample_info sample;
    DRM_Prdy_DecryptContext_t *p_decrypt_context;
} piff_drm;
typedef struct piff_drm *piff_drm_t;

typedef struct piff_file {
    NEXUS_FilePlayHandle fileplay;
    batom_factory_t factory;
    void *buffer_data;
    bfile_buffer_t file_buffer;
} piff_file;

#define NUM_DMA_BLOCK   (MAX_SUBSAMPLE_ENTRIES)
typedef struct piff_dma {
    NEXUS_DmaJobBlockSettings dma_blocks[NUM_DMA_BLOCK];
    NEXUS_HeapHandle heap;
} piff_dma;

typedef struct piff_instance {
    piff_file file;
    piff_dma dma;
    piff_drm drm;
    unsigned int cookie;
} piff_instance;
typedef struct piff_instance *piff_t;


/**************************************************************************
 * Globals
 **************************************************************************/
#define NUM_PIFF_CONTEXT 2
static piff_instance piff_contexts[NUM_PIFF_CONTEXT];  /* top level context */
static bool initialized = false;


/**************************************************************************
 * Functions
 **************************************************************************/

static piff_t piff_get_context(unsigned int cookie)
{
    int i;

    for (i=0; i<NUM_PIFF_CONTEXT; i++) {
        if (piff_contexts[i].cookie == cookie) {
            return &piff_contexts[i];
        }
    }

    BDBG_WRN(("No piff_context found for the cookie %#lx", cookie));

    return NULL;
}

/* Get the se box bound to a given track id. Return null if not found. */
static piff_se *get_se_ptr(piff_sample_info *p_sample_info, uint32_t track_id)
{
    int ii;

    for (ii=0; ii<MAX_SAMPLE_ENC_BOX; ii++) {
        if (p_sample_info->se[ii].track_id == track_id)
            return &p_sample_info->se[ii];
    }

    return NULL;
}

/* Get an se box from the pool and bind it to a track id */
static piff_se *get_availble_se_ptr(piff_sample_info *p_sample_info, uint32_t track_id)
{
    int ii;

    for (ii=0; ii<MAX_SAMPLE_ENC_BOX; ii++) {
        if (p_sample_info->se[ii].used == false) {
            p_sample_info->se[ii].used = true;
            p_sample_info->se[ii].track_id = track_id;
            return &p_sample_info->se[ii];
        }
    }

    BDBG_ERR(("Too many encrypted tracks, increase MAX_SAMPLE_ENC_BOX."));

    return NULL;
}

/* Parse and read sample_count */
static piff_err bmp4_parse_saiz_box(piff_drm_t drm, batom_cursor *cursor)
{
    bmp4_fullbox fullbox;
    uint32_t aux_info_type;
    uint32_t aux_info_type_parameter;
    uint8_t default_sample_info_size;
    uint32_t sample_count = 0;

    if (bmp4_parse_fullbox(cursor, &fullbox)) {
        if (fullbox.flags & 0x1) {
            aux_info_type = batom_cursor_uint32_be(cursor);
            aux_info_type_parameter = batom_cursor_uint32_be(cursor);

            BDBG_MSG(("saiz: aux_info_type: " B_MP4_TYPE_FORMAT ", aux_info_type_parameter: 0x%x", B_MP4_TYPE_ARG(aux_info_type), aux_info_type_parameter));

        }
        default_sample_info_size = batom_cursor_byte(cursor);
        sample_count = batom_cursor_uint32_be(cursor);
        if (sample_count > SAMPLES_POOL_SIZE) {
            BDBG_ERR(("saiz: sample_count (%d) exceeds buffer size!", sample_count));
            BDBG_ASSERT(false);
        }
        if (default_sample_info_size == 0) {
            uint32_t i;
            BDBG_WRN(("Each sample has different info size"));
            for (i=0; i<sample_count; i++) {
                drm->sample.sample_info_size[i] = batom_cursor_byte(cursor);
            }
        }
        else {
            BKNI_Memset(&drm->sample.sample_info_size, 0, sizeof(drm->sample.sample_info_size) /* SAMPLES_POOL_SIZE */);
        }

        drm->sample.sample_count_saiz = sample_count;
        drm->sample.default_sample_info_size = default_sample_info_size;

        BDBG_MSG(("saiz: default_sample_info_size %d, sample_count %d", default_sample_info_size, sample_count));
    }
    else {
        return piff_err_fail;
    }

    return piff_err_ok;
}

/* Parse and read se offset */
static piff_err bmp4_parse_saio_box(piff_drm_t drm, batom_cursor *cursor, uint64_t *se_offset)
{
    bmp4_fullbox fullbox;
    uint32_t aux_info_type;
    uint32_t aux_info_type_parameter;
    uint32_t entry_count = 0;
    BSTD_UNUSED(drm);

    if (bmp4_parse_fullbox(cursor, &fullbox)) {
        if (fullbox.flags & 0x1) {
            aux_info_type = batom_cursor_uint32_be(cursor);
            aux_info_type_parameter = batom_cursor_uint32_be(cursor);
            BDBG_MSG(("saio: aux_info_type: " B_MP4_TYPE_FORMAT ", aux_info_type_parameter: 0x%x", entry_count, *se_offset));
        }
        entry_count = batom_cursor_uint32_be(cursor);  /* Spec says this SHALL be 1 */
        if (fullbox.version == 0) {
            *se_offset = batom_cursor_uint32_be(cursor);
        }
        else {
            *se_offset = batom_cursor_uint64_be(cursor);
        }

        BDBG_MSG(("saio: entry_count %d, offset 0x%llx", entry_count, *se_offset));
    }
    else {
        return piff_err_fail;
    }

    return piff_err_ok;
}

/* Read and save SE information */
static piff_err bmp4_read_sample_enc_info(piff_drm_t drm, batom_cursor *cursor, uint32_t track_id)
{
    piff_err rc = piff_err_ok;
    piff_se *p_se;
    piff_se_sample *p_sample;
    piff_subsample *p_subsample_entry;
    bpiff_mp4_headers *header;
    uint32_t ii, jj;

    BDBG_MSG_TRACE(("%s: enter", __FUNCTION__));

    p_se = get_se_ptr(&drm->sample, track_id);
    if (p_se == NULL) {
        p_se = get_availble_se_ptr(&drm->sample, track_id);
    }
    BDBG_ASSERT(p_se != NULL);
    header = &drm->header;

    /* Use TE scheme info which is the track default */
    /* TODO: SampleToGroup/SampleGroupDescription Boxes */
    for (ii=0; ii<header->nbOfSchemes; ii++) {
        if (header->scheme[ii].trackId == track_id)
            break;
    }
    if (ii < header->nbOfSchemes) {
        drm->sample.scheme.alg_id = header->scheme[ii].trackEncryption.info.algorithm;
        drm->sample.scheme.iv_size = header->scheme[ii].trackEncryption.info.ivSize;
        BKNI_Memcpy(&drm->sample.scheme.kid, header->scheme[ii].trackEncryption.info.keyId, KEY_ID_LENGTH);
    }
    else {
        BDBG_ERR(("Unable to find the default scheme info from the TE"));
        BDBG_ASSERT(false);
    }

    /* Read IV and subsample info */
    p_se->sample_count = drm->sample.sample_count_saiz;
    if (p_se->sample_count != 0) {
        BKNI_Memset(p_se->samples, 0, sizeof(p_se->samples));
        p_sample = p_se->samples;
        for (ii=0; ii<p_se->sample_count; ii++) {
            batom_cursor_copy(cursor, &p_sample->iv[8], 8);  /* Save to iv[8-15] */
            if (drm->sample.scheme.iv_size == 16)
                batom_cursor_copy(cursor, p_sample->iv, 8);  /* End up with swapping the 16-byte IV */
            if (
                (drm->sample.default_sample_info_size == 0
                 && drm->sample.sample_info_size[ii] > drm->sample.scheme.iv_size)
                ||
                (drm->sample.default_sample_info_size != 0
                 && drm->sample.default_sample_info_size > drm->sample.scheme.iv_size)
               ) {
                /* Sub-sample info exists */
                p_sample->num_subsample_entries = batom_cursor_uint16_be(cursor);
                if (p_sample->num_subsample_entries != 0) {
                    if (p_sample->num_subsample_entries  > MAX_SUBSAMPLE_ENTRIES) {
                        BDBG_ERR(("Default nb of entry too small, increase MAX_SUBSAMPLE_ENTRIES"));
                        BDBG_ASSERT(false);
                    }
                    p_subsample_entry = p_sample->subsample_entries;
                    for (jj=0; jj< p_sample->num_subsample_entries; jj++) {
                        p_subsample_entry->bytesOfClearData = batom_cursor_uint16_be(cursor);
                        p_subsample_entry->bytesOfEncData = batom_cursor_uint32_be(cursor);
                        p_subsample_entry++;
                    }
                }
            }
            p_sample++;
        }
    }
    else {
        BDBG_ERR(("sample_count is zero!"));
    }

    BDBG_MSG_TRACE(("%s: exit, rc %d", __FUNCTION__, rc));

    return rc;
}

/* Parse and save SE box information */
static piff_err bmp4_parse_sample_enc_box(piff_drm_t drm, batom_cursor *cursor, uint32_t track_id)
{
    piff_err rc = piff_err_ok;
    uint32_t ii, jj;
    piff_se *p_se;
    piff_se_sample *p_sample;
    piff_subsample *p_subsample_entry;
    uint32_t alg_id;
    bpiff_mp4_headers *header;

    BDBG_MSG_TRACE(("%s: enter", __FUNCTION__));

    BDBG_ASSERT(drm != NULL);
    BDBG_ASSERT(cursor != NULL);

    p_se = get_se_ptr(&drm->sample, track_id);
    if (p_se == NULL) {
        p_se = get_availble_se_ptr(&drm->sample, track_id);
    }
    BDBG_ASSERT(p_se != NULL);

    header = &drm->header;
    if (bmp4_parse_box_extended(cursor, &p_se->extended)) {
        if (bmp4_parse_fullbox(cursor, &p_se->fullbox)) {
            if (p_se->fullbox.flags & 0x000001) {  /* Use SE scheme info which is fragment specific */
                alg_id = batom_cursor_uint24_be(cursor);
                CONVERT_ALG_ID_TO_DRM_ENCR_STATE(alg_id, p_se->se_scheme.alg_id);
                p_se->se_scheme.iv_size = batom_cursor_byte(cursor);
                batom_cursor_copy(cursor, p_se->se_scheme.kid, 16);
                BKNI_Memcpy(&drm->sample.scheme, &p_se->se_scheme, sizeof(piff_scheme));
            }
            else {  /* Use TE scheme info which is the track default */
                for (ii=0; ii<header->nbOfSchemes; ii++) {
                    if (header->scheme[ii].trackId == track_id)
                        break;
                }
                if (ii < header->nbOfSchemes) {
                    drm->sample.scheme.alg_id = header->scheme[ii].trackEncryption.info.algorithm;
                    drm->sample.scheme.iv_size = header->scheme[ii].trackEncryption.info.ivSize;
                    BKNI_Memcpy(&drm->sample.scheme.kid, header->scheme[ii].trackEncryption.info.keyId, KEY_ID_LENGTH);
                } else {
                    BDBG_ERR(("Unable to find the default scheme info from the TE"));
                    BDBG_ASSERT(false);
                }
            }

            /* Read IV and subsample info */
            p_se->sample_count = batom_cursor_uint32_be(cursor);
            if (p_se->sample_count != 0) {
                if (p_se->sample_count > SAMPLES_POOL_SIZE){
                    BDBG_ERR(("Sample pools too small, increase SAMPLES_POOL_SIZE."));
                    BDBG_ASSERT(false);
                }

                p_sample = p_se->samples;
                for (ii=0; ii<p_se->sample_count; ii++) {
                    batom_cursor_copy(cursor, &p_sample->iv[8], 8);  /* Save to iv[8-15] */
                    if (drm->sample.scheme.iv_size == 16)
                        batom_cursor_copy(cursor, p_sample->iv, 8);  /* End up with swapping the 16-byte IV */
                    if (p_se->fullbox.flags & 0x000002) {  /* Sub-sample info exists */
                        p_sample->num_subsample_entries = batom_cursor_uint16_be(cursor);
                        if (p_sample->num_subsample_entries != 0) {
                            if (p_sample->num_subsample_entries  > MAX_SUBSAMPLE_ENTRIES) {
                                BDBG_ERR(("Default nb of entry too small, increase MAX_SUBSAMPLE_ENTRIES"));
                                BDBG_ASSERT(false);
                            }
                            p_subsample_entry = p_sample->subsample_entries;
                            for (jj=0; jj< p_sample->num_subsample_entries; jj++) {
                                p_subsample_entry->bytesOfClearData = batom_cursor_uint16_be(cursor);
                                p_subsample_entry->bytesOfEncData = batom_cursor_uint32_be(cursor);
                                p_subsample_entry++;
                            }
                        }
                    }
                    p_sample++;
                }
            }
            else {
                BDBG_ERR(("sample_count in SE box is zero!"));
            }
        }
        else {
            BDBG_ERR(("Might be an invalid SE box!"));
            rc = piff_err_fail;
        }
    }
    else {
        BDBG_ERR(("Might be an invalid box!"));
        rc = piff_err_fail;
    }

    BDBG_MSG_TRACE(("%s: exit, rc %d", __FUNCTION__, rc));
    return rc;
}

static piff_err parse_mp4_traf_box(piff_drm_t drm, batom_cursor *cursor, bmp4_box *p_box)
{
    piff_err rc = piff_err_fail;
    bmp4_box box;
    size_t box_hdr_size;
    uint32_t ii;
    struct batom_checkpoint se_start;
    struct batom_checkpoint tfhd_start;
    uint64_t se_offset = 0;
    bool se_offset_found = false;
    bool se_box_found = false;
    bool tfhd_box_found = false;
    bmp4_track_fragment_header traf_hdr;

    BDBG_MSG_TRACE(("%s: enter", __FUNCTION__));
    BDBG_ASSERT((p_box->type == BMP4_TRACK_FRAGMENT) || (p_box->type == BMP4_TRACK_FRAGMENT_HEADER));
    batom_cursor_save(cursor, &tfhd_start);

    for (ii=0; ii<p_box->size-8; ii+= box.size) {
        box_hdr_size = bmp4_parse_box(cursor, &box);
        if (box_hdr_size == 0) {
            break;
        }

        if (box.type == BMP4_TRACK_FRAGMENT_HEADER) {  /* required to parse track id */
            if (!bmp4_parse_track_fragment_header(cursor, &traf_hdr)) {
                BDBG_ERR(("bmp4_parse_track_fragment_header failed"));
                goto ErrorExit;
            }
            tfhd_box_found = true;
            if (se_box_found) break;
            else continue;
        }
        else {
            /* PIFF 1.1 - Look for uuid box (SampleEncryptionBox) */
            if (!drm->header.piff_1_3) {
                if (box.type == BMP4_EXTENDED) {
                    /* Save position of the SE box */
                    batom_cursor_save(cursor, &se_start);
                    se_box_found = true;
                    if (tfhd_box_found) break;
                }
                else {
                    batom_cursor_skip(cursor, box.size - box_hdr_size);
                }
            }
            /* PIFF 1.3 - Solely rely on 'saio' (5.4.3, 1) b.) */
            else {
                if (box.type == BMP4_SAMPLE_AUX_INFO_OFFSET) {
                    BDBG_MSG(("found saio box"));
                    if (bmp4_parse_saio_box(drm, cursor, &se_offset) != piff_err_ok) {
                        BDBG_ERR(("failed to parse saio box"));
                        goto ErrorExit;
                    }
                    se_offset_found = true;
                }
                else if (box.type == BMP4_SAMPLE_AUX_INFO_SIZE) {
                    /* Must parse to know the sample_count and enc info size */
                    BDBG_MSG(("found saiz box"));
                    if (bmp4_parse_saiz_box(drm, cursor) != piff_err_ok) {
                        BDBG_ERR(("failed to parse saiz box"));
                        goto ErrorExit;
                    }
                }
#if 0
                /* TODO */
                else if (box.type == BMP4_SAMPLE_TO_GROUP) {
                    BDBG_MSG(("found sbgp box"));
                }
                else if (box.type == BMP4_SAMPLE_GROUP_DESC) {
                    BDBG_MSG(("found sgpd box"));
                }
#endif
                else {
                    /* Not the box we are looking for. Skip over it.*/
                    batom_cursor_skip(cursor, box.size - box_hdr_size);
                }
            }
        }
    }

    if (se_box_found) {
        /* Rollback to the position of the SE box. */
        batom_cursor_rollback(cursor, &se_start);
        if (bmp4_parse_sample_enc_box(drm, cursor, traf_hdr.track_ID) != piff_err_ok) {
            BDBG_ERR(("bmp4_parse_sample_enc_box failed"));
            goto ErrorExit;
        }
    }
    else if (se_offset_found) {
        /* Position the cursor to the specified offset */
        batom_cursor_rollback(cursor, &tfhd_start);
        batom_cursor_skip(cursor, se_offset - 32);  /* TODO: Assuming fixed size for moof, mfhd, traf */
        if (bmp4_read_sample_enc_info(drm, cursor, traf_hdr.track_ID) != piff_err_ok) {
            BDBG_ERR(("bmp4_read_sample_enc_info faild"));
            goto ErrorExit;
        }
    }
    else {
        /* No Sample encryption box found. The content is most likely unencrypted */
    }

    rc = piff_err_ok;

ErrorExit:
    BDBG_MSG_TRACE(("%s: exit", __FUNCTION__));
    return rc;
}

static piff_err mp4_parse_moof(piff_drm_t drm, batom_cursor *cursor, bmp4_box *p_box)
{
    piff_err rc = piff_err_ok;
    bmp4_box box;
    size_t   box_hdr_size;
    uint32_t ii;

    BDBG_MSG_TRACE(("%s: enter", __FUNCTION__));

    BDBG_ASSERT(cursor != NULL);
    BDBG_ASSERT(drm != NULL);
    BDBG_ASSERT(p_box->type == BMP4_MOVIE_FRAGMENT);

    for (ii=0; ii<p_box->size-8; ii+=box.size) {
        box_hdr_size = bmp4_parse_box(cursor, &box);

        if (box_hdr_size == 0) {
            break;
        }

        if (box.type == BMP4_TRACK_FRAGMENT) {
            rc = parse_mp4_traf_box(drm, cursor, &box);
            break; /* Done with traf parsing. Exit */
        }
        else {
            /* Not the box we are looking for. Skip over it.*/
            batom_cursor_skip(cursor, box.size - box_hdr_size);
        }
    }

    BDBG_MSG_TRACE(("%s: exit, rc %d", __FUNCTION__, rc));
    return rc;
}

static void parse_mp4_fragment_context(piff_drm_t drm, const batom_cursor *cursor, size_t payload_len)
{
    bmp4_box   fragment;
    size_t     box_hdr_size;
    uint32_t   ii;
    struct batom_checkpoint start;

    BDBG_MSG_TRACE(("%s: enter", __FUNCTION__));

    /* save cursor position on entry */
    batom_cursor_save(cursor, &start);

    if (bmp4_parse_box((batom_cursor *)cursor, &fragment)) {
        if (fragment.type == BMP4_TRACK_FRAGMENT) {
            /* We received a traf box, parse it to find the sample encryption box*/
            if (parse_mp4_traf_box(drm, (batom_cursor *)cursor, &fragment) != piff_err_ok) {
                BDBG_ERR(("parse_mp4_traf_box failed"));
                goto ErrorExit;
            }
        }
        else if (fragment.type == BMP4_TRACK_FRAGMENT_HEADER) {
            /* Received the "payload" of the traf box, that is tfhd. Rollback and parse.
             * TODO: Awkward but needed to accomodate the caller's quirk. */
            batom_cursor_rollback((batom_cursor*)cursor, &start);
            fragment.size = payload_len;
            if (parse_mp4_traf_box(drm, (batom_cursor *)cursor, &fragment) != piff_err_ok) {
                BDBG_ERR(("parse_mp4_traf_box from the header failed"));
                goto ErrorExit;
            }
        }
        else {
            /* Not a traf box, check if we are dealing with a bmp4 box propritary */
            /* TODO: Not need for standard PIFF, but keep the code just in case. */
            if (fragment.type == BMP4_PROPRITARY_BMP4) {
                bmp4_box box;
                for (ii=0; ii<fragment.size-8; ii+=box.size) {
                    box_hdr_size = bmp4_parse_box((batom_cursor *)cursor, &box);

                    if (box_hdr_size==0) {
                        break;
                    }

                    switch (box.type) {
                        case BMP4_MOVIE_FRAGMENT:
                            if (mp4_parse_moof(drm, (batom_cursor *)cursor, &box)!= piff_err_ok) {
                                BDBG_ERR(("mp4_parse_moof failed"));
                                goto ErrorExit;
                            }
                            break;
                        default :
                            /* Not the box we are looking for. Skip over it.*/
                            batom_cursor_skip((batom_cursor *)cursor, box.size - box_hdr_size);
                            break;
                    }
                }
            }
            else {
                /* Invalid box. */
                BDBG_ERR(("Invalid track fragment"));
            }
        }
    }
    else {
        /* Not a box */
        BDBG_ERR(("Invalid MP4 box"));
    }

ErrorExit:
    batom_cursor_rollback((batom_cursor*)cursor, &start);
    BDBG_MSG_TRACE(("%s: exit", __FUNCTION__));
    return;
}

static int parse_piff_track_fragment(piff_drm_t drm, batom_cursor *cursor)
{
    BDBG_MSG_TRACE(("%s: enter", __FUNCTION__));

    parse_mp4_fragment_context(drm, cursor, batom_cursor_size(cursor));

    BDBG_MSG_TRACE(("%s: exit", __FUNCTION__));

    return 0;
}

static size_t cursor_refill(batom_cursor *cursor)
{
    BDBG_ASSERT(cursor->left<=0);
    BDBG_MSG_TRACE(("%s: enter", __FUNCTION__));

    if (cursor->left == 0) {
        unsigned pos;
        const batom_vec *vec;
        BDBG_ASSERT(cursor->vec);
        BDBG_ASSERT(cursor->pos <= cursor->count);
        for (pos=cursor->pos,vec=&cursor->vec[pos]; pos<cursor->count; vec++) {
            pos++;
            cursor->pos = pos;
            if (vec->len > 0) {
                cursor->left = vec->len;
                cursor->cursor = vec->base;
                return (size_t)cursor->left;
            }
        }
        /* reached EOF */
        cursor->left = BATOM_EOF;
    }

    BDBG_MSG_TRACE(("%s: exit", __FUNCTION__));
    return 0;
}

static size_t copy_heap_to_cursor(batom_cursor *cursor, const uint8_t *src, size_t count)
{
    size_t bytes_copied = 0;
    size_t copy_left = count;

    for (; copy_left>0 && cursor->left>0;) {
        size_t bytes_to_copy;
        bytes_to_copy = ((size_t)cursor->left > copy_left)? (copy_left):(size_t)(cursor->left);
        BKNI_Memcpy((uint8_t *)cursor->cursor, src, bytes_to_copy);
        copy_left -= bytes_to_copy;
        src += bytes_to_copy;
        if (copy_left <= 0) { break; }
        batom_cursor_skip(cursor, bytes_to_copy);
        bytes_copied += bytes_to_copy;
        if (cursor_refill(cursor) == 0) {
            BDBG_WRN(("Insufficient cursor while %d bytes remaining", copy_left));
        }
    }

    return bytes_copied;
}

static int decrypt_piff_sample(
    piff_t piff_cxt,
    unsigned track_no,
    unsigned sample_no,
    batom_cursor *cursor,
    size_t payload_len
)
{
    piff_se *p_se;
    piff_se_sample *p_sample;
    uint8_t *heap_start = NULL;
    uint8_t *heap_end = NULL;
    struct batom_checkpoint start;
    size_t bytes_copied;
    int ii;
    NEXUS_Error rc;

    DRM_Prdy_AES_CTR_Info_t aes_ctr_info;
    DRM_Prdy_Error_e prdy_dr;

    NEXUS_HeapHandle heap;
    piff_drm_t p_drm;

    BDBG_MSG_TRACE(("%s: enter", __FUNCTION__));

    if (piff_cxt == NULL) {
        BDBG_ERR(("piff_cxt is NULL"));
        return -1;
    }
    BKNI_Memset(&aes_ctr_info, 0, sizeof(DRM_Prdy_AES_CTR_Info_t));

    heap = piff_cxt->dma.heap;
    p_drm = &piff_cxt->drm;


    /*
     * 1) Get IV, subsample info from SE.
     */

    p_se = get_se_ptr(&p_drm->sample, track_no);
    if (p_se == NULL) {
        BDBG_ERR(("Unable to get SE box"));
        return -1;
    }

    if (p_drm->sample.scheme.alg_id == 0x3) {
        BDBG_ERR(("AES-CBC is not supported"));
        return -1;
    }

    if (p_drm->sample.scheme.alg_id == 0x0) {
        BDBG_WRN(("Data in the clear, nothing to do"));
        return 0;
    }

    /* Read and cook IV */
    p_sample = &p_se->samples[sample_no];
    {
        /* TODO: This is not good, but is required by the driver */

        unsigned char *ptr = (unsigned char *)&(aes_ctr_info.qwInitializationVector);
        int i;
        for (i=0; i<8; i++) {
            *ptr++ = p_sample->iv[15-i];
        }
    }

    batom_cursor_save(cursor, &start);

    /*
     * 2) Allocate heap for DMA operation
     */
    BDBG_MSG_TRACE(("%s: Allocating %d bytes from the nexus heap", __FUNCTION__, payload_len));

    if (heap) {
        NEXUS_MemoryAllocationSettings mem_alloc_settings;
        NEXUS_Memory_GetDefaultAllocationSettings(&mem_alloc_settings);
        mem_alloc_settings.heap = heap;
        rc = NEXUS_Memory_Allocate(payload_len, &mem_alloc_settings, (void **)&heap_start);
    }
    else {
        BDBG_WRN(("%s: dma heap is NULL. Will try the default heap.", __FUNCTION__));
        rc = NEXUS_Memory_Allocate(payload_len, NULL, (void **)&heap_start);
    }
    if (rc || heap_start == NULL) {
        BDBG_ERR(("Unable to allocate dma heap"));
        return -1;
    }
    BDBG_MSG_TRACE(("Allocated dma heap at %#lx", heap_start));
    heap_end = heap_start;

    /*
     * 3) Copy data to the heap and prepare for decryption.
     */
    BDBG_MSG_TRACE(("p_se %lx, flags %x, alg %x", p_se, p_se->fullbox.flags, p_drm->sample.scheme.alg_id));
    if (p_sample->num_subsample_entries) {  /* sub-sample exists */
        BDBG_MSG_TRACE(("%s: sub-sample blocks %d (track %d)", __FUNCTION__, p_sample->num_subsample_entries, track_no));
        for (ii=0; ii<p_sample->num_subsample_entries; ii++) {
            /*
             *   <cursor>            <heap>
             *  +--------+         +--------+
             *  |  (C0)  |         |//(E0)//|
             *  |--------| ======> +--------+
             *  |//(E0)//|  copy   |//(E1)//|
             *  +--------+         +--------+
             *  |  (C1)  |         |  ...   |
             *  |--------|
             *  |//(E1)//|
             *  +--------+
             *  |  ...   |
             *
             */
            BDBG_MSG_TRACE(("%s: entry %d of %d: #clean %d, #enc %d", __FUNCTION__, ii+1, p_sample->num_subsample_entries, p_sample->subsample_entries[ii].bytesOfClearData, p_sample->subsample_entries[ii].bytesOfEncData));
            batom_cursor_skip(cursor, p_sample->subsample_entries[ii].bytesOfClearData);
            bytes_copied = batom_cursor_copy(cursor, heap_end, p_sample->subsample_entries[ii].bytesOfEncData);
            if (bytes_copied != p_sample->subsample_entries[ii].bytesOfEncData) {
                 BDBG_ERR(("Only %d of %d bytes copied at %dth sub-sample.. Continuing for the best..",
                             bytes_copied, p_sample->subsample_entries[ii].bytesOfEncData, ii));
            }
            heap_end += p_sample->subsample_entries[ii].bytesOfEncData;
        }
    }
    else {  /* no sub-samples. just a single encrypted chunk */
        bytes_copied = batom_cursor_copy(cursor, heap_end, payload_len);
        if (bytes_copied != payload_len) {
             BDBG_ERR(("Only %d of %d bytes copied.. Continuing anyway..", bytes_copied, payload_len));
        }
        heap_end += payload_len;
    }

    BDBG_MSG_TRACE(("start %lx end %lx size %lx", heap_start, heap_end, (heap_end-heap_start)));
    /*
     * 4) Request decryption.
     */
    BDBG_MSG_TRACE(("decrypt"));

    prdy_dr = DRM_Prdy_Reader_Decrypt(
                p_drm->p_decrypt_context,
                &aes_ctr_info,
                (uint8_t *)heap_start,
                (uint32_t)(heap_end - heap_start));
    if (prdy_dr != DRM_Prdy_ok) {
         BDBG_ERR(("DRM_Prdy_Reader_Decrypt failed (%d)", prdy_dr));
         goto ErrorExit;
    }

    /*
     * 5) copy the decrypted data back to the cursor
     */
    BDBG_MSG_TRACE(("copy data back to cursor"));
    batom_cursor_rollback(cursor, &start);
    heap_end = heap_start;
    if (p_sample->num_subsample_entries) {  /* sub-samples */
        for (ii=0; ii<p_sample->num_subsample_entries; ii++) {
            batom_cursor_skip(cursor, p_sample->subsample_entries[ii].bytesOfClearData);
            bytes_copied = copy_heap_to_cursor(cursor, heap_end, p_sample->subsample_entries[ii].bytesOfEncData);
            batom_cursor_skip(cursor, p_sample->subsample_entries[ii].bytesOfEncData - bytes_copied);
            heap_end += p_sample->subsample_entries[ii].bytesOfEncData;
        }
    }
    else {
        copy_heap_to_cursor(cursor, heap_end, payload_len);
    }

ErrorExit:
    if (heap_start) {
        BDBG_MSG_TRACE(("freeing %#lx", heap_start));
        NEXUS_Memory_Free(heap_start);
    }
    BDBG_MSG_TRACE(("%s: exit", __FUNCTION__));

    return 0;
}


/* This function will be hooked as decrypt_callback() which is called from bmp4_player */
static void piff_decrypt_callback(void *cntx, batom_cursor *cursor, size_t *length, void *drm_info, unsigned track_no)
{
    bmp4_player_drm_info *mp4_drm_info = (bmp4_player_drm_info *)drm_info;
    piff_t piff = NULL;
    BSTD_UNUSED(length);

    BDBG_MSG_TRACE(("cntx %lx, cursor %lx, length %lx, drm_info %lx, track %lu", cntx, cursor, length, drm_info, track_no));

    piff = piff_get_context((unsigned int)cntx);
    if (piff == NULL) {
        BDBG_ERR(("No piff_context found for cntx %#lx", cntx));
        return;
    }
    if (mp4_drm_info->type == bmp4_player_drm_info_frame) {
        BDBG_MSG_TRACE(("frame: track %u, sample %u, size %ld", track_no, mp4_drm_info->data.frame.sample_no, batom_cursor_size((batom_cursor *)cursor)));
        decrypt_piff_sample(piff, track_no, mp4_drm_info->data.frame.sample_no-1, cursor, batom_cursor_size(cursor));
    }
    else if (mp4_drm_info->type == bmp4_player_drm_info_track_fragment) {
        batom_t atom = NULL;
        batom_cursor buf_cursor;
        bfile_buffer_result result;

        BDBG_MSG(("fragment: offset %#jx, length %#zx, buffer %#lx",
                                 mp4_drm_info->data.track_fragment.offset,
                                 mp4_drm_info->data.track_fragment.length,
                                 mp4_drm_info->data.track_fragment.buffer));

        atom = bfile_buffer_read(
                piff->file.file_buffer,
                mp4_drm_info->data.track_fragment.offset,
                mp4_drm_info->data.track_fragment.length,
                &result);

        if (atom == NULL) {
            BDBG_ERR(("bfile_buffer_read failed with result %d", result));
        }
        else {
            batom_cursor_from_atom(&buf_cursor, atom);
            parse_piff_track_fragment(&piff->drm, &buf_cursor);
        }
        if (atom) {
            batom_release(atom);
        }
    }
    else {
        BDBG_ERR(("Unknown drm info type %d", mp4_drm_info->type));
    }
}

static int file_buffer_create(piff_file *file)
{
    bfile_buffer_cfg buffer_cfg;

    if (file->factory == NULL) {
        BDBG_ERR(("factory is NULL"));
        return -1;
    }

    file->buffer_data = BKNI_Malloc(BUFFER_LENGTH);   /* TODO: don't forget to free! */
    if (file->buffer_data == NULL) {
        BDBG_ERR(("Unable to BKNI_Malloc %d bytes", BUFFER_LENGTH));
        return -1;
    }
    bfile_buffer_default_cfg(&buffer_cfg);
    buffer_cfg.buf_len = BUFFER_LENGTH;
    buffer_cfg.buf = file->buffer_data;
    buffer_cfg.fd = file->fileplay->file.index;
    buffer_cfg.nsegs = BUFFER_LENGTH/4096;
    file->file_buffer = bfile_buffer_create(file->factory, &buffer_cfg);

    if (!file->file_buffer) {
        BDBG_ERR(("bfile_buffer_create failed"));
        BKNI_Free(file->buffer_data);
        return -1;
    }

    return 0;
}

static int file_buffer_destroy(piff_file *file)
{
    if (file->buffer_data) { BKNI_Free(file->buffer_data); }
    if (file->file_buffer) { bfile_buffer_destroy(file->file_buffer); }
/*    if (file->factory) { batom_factory_destroy(file->factory); }*/

    return 0;
}

static int read_drm_info(piff_t piff_cxt)
{
    batom_t atom = NULL;
    bfile_buffer_result result;
    size_t size;
    size_t box_hdr_size;
    batom_cursor cursor;
    int rc = -1;
    bmp4_box box;
    bpiff_mp4_headers piff_header;
    off_t start, end;
    unsigned int size_to_read;

    BDBG_MSG_TRACE(("%s start", __FUNCTION__));

    /*
     * 0) get file size and set initial read offset to zero
     * 1) Read READ_LENGTH or remaining size
     *    - error if atom is NULL
     * 2) Parse only the first box
     *    - error if box size is zero
     *    - if moov is found, mark and get out of the loop.
     * 3) release atom
     * 4) Skip the file offset by box size
     * 5) continue from 1)
     */

    if (bfile_buffer_get_bounds(piff_cxt->file.file_buffer, &start, &end)) {
        BDBG_WRN(("unable to get the bounds"));
        size_to_read = BUFFER_LENGTH;  /* Just try initial part */
    }
    else {
        BDBG_MSG(("start %#lx, end %#lx", (unsigned long)start, (unsigned long)end));
        size_to_read = (unsigned int)end;
    }

    start = 0;
    while (size_to_read) {
        if (size_to_read > READ_LENGTH) {
            size = READ_LENGTH;
        }
        else {
            size = size_to_read;
        }
        atom = bfile_buffer_read(piff_cxt->file.file_buffer, start, size, &result);
        if (atom == NULL) {
            BDBG_ERR(("Unable to read file, result %d", result));
            goto read_drm_info_exit;
        }
        batom_cursor_from_atom(&cursor, atom);
        /* find moov */
        box_hdr_size = bmp4_parse_box(&cursor, &box);
        if (box_hdr_size == 0) {
            BDBG_ERR(("%s: Moov not found", __FUNCTION__));
            goto read_drm_info_exit;
        }
        BDBG_MSG_TRACE(("box size %#lx", (unsigned long)box.size));
        if (box.type == BMP4_MOVIE) {
            BDBG_MSG(("Found moov"));
            break;
        }
        size_to_read -= size;
        start += box.size;
        batom_release(atom);
        atom = NULL;
    }

    /* Here, atom should be pointing to moov. Pass it to moov parser. */
    BKNI_Memset(&piff_header, 0, sizeof(piff_header));
    rc = bpiff_parse_moov(&piff_header, atom);
    if (rc) {
        BDBG_ERR(("bpiff_parse_moov() failed (%d)", rc));
        goto read_drm_info_exit;
    }
    if (0) {  /* Enable for debug dump */
        uint32_t i;
        BKNI_Printf("* PIFF version: %s\n", (piff_header.piff_1_3)? "1.3":"1.1");
        BKNI_Printf("* Num of protection schemes: %d\n", piff_header.nbOfSchemes);
        for (i=0; i<piff_header.nbOfSchemes; i++) {
            int j;
            BKNI_Printf("* Scheme %d\n", i);
            BKNI_Printf("  Box ver      0x%x\n", piff_header.scheme[i].schemeType.version);
            BKNI_Printf("  Box flags    0x%x\n", piff_header.scheme[i].schemeType.flags);
            BKNI_Printf("  Sch type     " B_MP4_TYPE_FORMAT"\n", B_MP4_TYPE_ARG(piff_header.scheme[i].schemeType.schemeType));
            BKNI_Printf("  Sch ver      0x%x\n", piff_header.scheme[i].schemeType.schemeVersion);
            BKNI_Printf("  Org fmt      " B_MP4_TYPE_FORMAT"\n", B_MP4_TYPE_ARG(piff_header.scheme[i].originalFormat.codingName));
            BKNI_Printf("  trackID      0x%x\n", piff_header.scheme[i].trackId);
            BKNI_Printf("  TE box ver   0x%x\n", piff_header.scheme[i].trackEncryption.version);
            BKNI_Printf("  TE box flags 0x%x\n", piff_header.scheme[i].trackEncryption.flags);
            BKNI_Printf("  TE algo      0x%x\n", piff_header.scheme[i].trackEncryption.info.algorithm);
            BKNI_Printf("  TE IV size   0x%x\n", piff_header.scheme[i].trackEncryption.info.ivSize);
            BKNI_Printf("  TE keyID     ");
                for (j=0; j<16; j++) {
                    BKNI_Printf("0x%x ", piff_header.scheme[i].trackEncryption.info.keyId[j]);
                }
                BKNI_Printf("\n");
        }
        BKNI_Printf("------------------------\n");
    }
    BKNI_Memcpy(&piff_cxt->drm.header, &piff_header, sizeof(bpiff_mp4_headers));
    bpiff_FreeMp4Header(&piff_header);
    piff_header.pPsshData = NULL;

    rc = 0;

read_drm_info_exit:
    if (atom) { batom_release(atom); }
    BDBG_MSG_TRACE(("%s end", __FUNCTION__));

    return rc;
}

static void piff_context_cleanup(piff_t piff)
{
    file_buffer_destroy(&piff->file);
    BKNI_Memset(piff, 0, sizeof(piff_instance));
}

void NEXUS_Playback_P_PiffStart(
    NEXUS_PlaybackHandle playback,
    decrypt_callback *cb
    )
{
    NEXUS_ClientConfiguration clientConfig;
    NEXUS_MemoryStatus status;
    piff_t piff = NULL;
    int i;

    BDBG_MSG(("%s", __FUNCTION__));;

    if (!initialized) {
        for (i=0; i<NUM_PIFF_CONTEXT; i++) {
            BKNI_Memset(&piff_contexts[i], 0, sizeof(piff_instance));
        }
        initialized = true;
    }

    *cb = NULL;
    BDBG_OBJECT_ASSERT(playback, NEXUS_Playback);

    piff = piff_get_context((unsigned int)playback->params.playpumpSettings.securityContext);
    if (piff != NULL) {
        BDBG_ERR(("piff_context already exists at %#lx", piff));
        return;  /* TODO: Should be BDBG_ASSERT(0); for debugging */
    }
    else {
        for (i=0; i<NUM_PIFF_CONTEXT; i++) {
            if (!piff_contexts[i].cookie) {
                /* Use this context */
                piff = &piff_contexts[i];
                break;
            }
        }
    }

    if (piff == NULL) {
        BDBG_ERR(("No piff_context is available"));
        return;
    }

    BKNI_Memset(piff, 0, sizeof(piff_instance));

    piff->cookie = (unsigned int)playback->params.playpumpSettings.securityContext;
    piff->file.fileplay = playback->file;
    piff->file.factory = playback->state.media.factory;
#if 0
    piff.file.buffer_data = playback->state.media.buffer;
#endif

    NEXUS_KeySlot_GetTag(playback->params.playpumpSettings.securityContext, (NEXUS_KeySlotTag*)&piff->drm.p_decrypt_context);
    if (piff->drm.p_decrypt_context == NULL) {
        BDBG_ERR(("decryptor context is NULL"));
        return;
    }

    if (file_buffer_create(&piff->file)) {
        BDBG_ERR(("file_buffer_create() failed"));
        piff_context_cleanup(piff);
        return;
    }

    /* Read default algorithm, etc. from the TE box */
    if (read_drm_info(piff)) {
        BDBG_ERR(("Unable to read DRM info from the file."));
        if (0) piff_context_cleanup(piff);
        return;
    }

    /* Determine the DMA heap */
    NEXUS_Platform_GetClientConfiguration(&clientConfig);
    for (i=0; i<NEXUS_MAX_HEAPS; i++) {
        if (clientConfig.heap[i] != NULL) {
            NEXUS_Heap_GetStatus(clientConfig.heap[i], &status);
            if (status.memoryType == NEXUS_MemoryType_eFull) {
                piff->dma.heap = clientConfig.heap[i];
                BDBG_WRN(("Found eFull heap at index %d", i));
                break;
            }
        }
    }

    *cb = piff_decrypt_callback;
}

void NEXUS_Playback_P_PiffStop(
    NEXUS_PlaybackHandle playback
    )
{
    piff_t piff = NULL;

    BDBG_MSG(("%s", __FUNCTION__));
    BDBG_OBJECT_ASSERT(playback, NEXUS_Playback);

    piff = piff_get_context((unsigned int)playback->params.playpumpSettings.securityContext);

    if (piff == NULL) {
        BDBG_WRN(("Trying to stop for unknown cntx %#lx",
                    playback->params.playpumpSettings.securityContext));
        return;
    }
    piff_context_cleanup(piff);
}
