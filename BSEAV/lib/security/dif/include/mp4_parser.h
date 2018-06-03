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

#ifndef MP4_PARSER_H
#define MP4_PARSER_H

#include "bfile_io.h"
#include "bmp4.h"

typedef enum MediaType {
    media_type_eUnknown, /* unknown/not supported media container type */
    media_type_eCenc,    /* CENC format */
    media_type_ePiff,    /* PIFF format */
    media_type_eMax
} MediaType;


typedef void *  mp4_parser_handle_t;

typedef struct mp4_parser_context {
    bfile_io_read_t fd;
    batom_factory_t factory;
    bmp4_mp4_headers mp4_mp4;
    bmp4_mp4_frag_headers mp4_mp4_frag;
    uint8_t *mp4_payload;
    bmp4_filetypebox filetype;
} mp4_parser_context;

typedef struct mp4_parse_frag_info {
    batom_cursor cursor;
    uint32_t moof_size;
    uint32_t mdat_size;
    uint32_t trackType;
    uint32_t trackId;
    uint32_t trackTimeScale;
    bmp4_track_fragment_run_sample *sample_info;
    uint32_t run_sample_count;
    bmp4_drm_mp4_se *samples_info;
    uint32_t aux_info_size;
} mp4_parse_frag_info;

mp4_parser_handle_t mp4_parser_create(bfile_io_read_t fd);
void mp4_parser_destroy(mp4_parser_handle_t handle);
bool mp4_parser_scan_file_type(mp4_parser_handle_t handle);
bool mp4_parser_scan_movie_info(mp4_parser_handle_t handle);
bool mp4_parser_scan_movie_fragment(mp4_parser_handle_t handle,
        mp4_parse_frag_info *frag_info, uint8_t *pBuf,
        uint32_t buffer_size, MediaType mediaType);

uint8_t* mp4_parser_get_pssh(mp4_parser_handle_t handle, uint32_t *len, uint8_t drmSchemeIndex);
uint32_t mp4_parser_get_track_type(mp4_parser_handle_t handle, uint32_t trackId);
void* mp4_parser_get_dec_data(mp4_parser_handle_t handle, uint32_t *len, uint32_t trackId);

#endif /* MP4_PARSER_H */
