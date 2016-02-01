/******************************************************************************
 * (c) 2002-2014 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 *****************************************************************************/

#ifndef BTHUMBNAIL_EXTRACTOR_H__
#define BTHUMBNAIL_EXTRACTOR_H__

#include "nexus_types.h"
#include "nexus_playpump.h"
#include "bfile_io.h"

#ifdef __cplusplus
extern "C" {
#endif

/*=**************
bthumbnail_extractor is a library to extract thumbnails from streams.
It uses Nexus for access to playback and for basic data types.
It uses media framework, specifically bmedia_player, to parse streams.
It supports any stream or codec supported by Nexus.

See BSEAV/app/thumbnail for a simple example of thumbnail extraction.

Brutus uses bthumbnail_extractor along with bthumbnail_stream and bthumbnail_manager.
bthumbnail_extractor does not depend on bthumbnail_stream and bthumbnail_manager.
****************/

/**
Summary:
**/
typedef struct bthumbnail_extractor *bthumbnail_extractor_t;

/**
Summary:
**/
typedef struct bthumbnail_extractor_create_settings
{
    unsigned buffer_size; /* set for maximum sized thumbnail */
} bthumbnail_extractor_create_settings;

/**
Summary:
**/
typedef struct bthumbnail_extractor_settings
{
    NEXUS_VideoCodec videoCodec;
    NEXUS_TransportType transportType;
    NEXUS_TransportTimestampType timestampType;
    unsigned short videoPid;
    NEXUS_PlaypumpHandle playpump;
    bfile_io_read_t datafile;
    bfile_io_read_t indexfile;
    unsigned videoCdbSize;
} bthumbnail_extractor_settings;

/**
Summary:
**/
void bthumbnail_extractor_get_default_create_settings(
    bthumbnail_extractor_create_settings *p_settings /* [out] */
    );

/**
Summary:
**/
bthumbnail_extractor_t bthumbnail_extractor_create(
    const bthumbnail_extractor_create_settings *p_settings
    );

/**
Summary:
**/
void bthumbnail_extractor_destroy(
    bthumbnail_extractor_t handle
    );

/**
Summary:
**/
void bthumbnail_extractor_get_settings(
    bthumbnail_extractor_t handle,
    bthumbnail_extractor_settings *p_settings /* [out] */
    );

/**
Summary:
**/
int bthumbnail_extractor_set_settings(
    bthumbnail_extractor_t handle,
    const bthumbnail_extractor_settings *p_settings
    );

/**
Summary:
**/
int bthumbnail_extractor_start_playpump(
    bthumbnail_extractor_t handle
    );

/**
Summary:
Extract a thumbnail and send it to transport using playpump.
**/
int bthumbnail_extractor_feed_picture(
    bthumbnail_extractor_t handle,
    unsigned timestamp /* in milliseconds */
    );

/**
Summary:
**/
void bthumbnail_extractor_stop_playpump(
    bthumbnail_extractor_t handle
    );

/**
Summary:
**/
typedef struct bthumbnail_extractor_status
{
    unsigned timestamp; /* Timestamp of the extracted picture in milliseconds */
} bthumbnail_extractor_status;

/**
Summary:
Get status of thumbnail extractor.
The status is cleared and populated again on each call to bthumbnail_extractor_feed_picture.
**/
void bthumbnail_extractor_get_status(
    bthumbnail_extractor_t handle,
    bthumbnail_extractor_status *status
    );


#ifdef __cplusplus
}
#endif

#endif /* BTHUMBNAIL_EXTRACTOR_H__ */
