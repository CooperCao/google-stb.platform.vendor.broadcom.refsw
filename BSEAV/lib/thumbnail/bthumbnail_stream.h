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

#ifndef BTHUMBNAIL_STREAM_H__
#define BTHUMBNAIL_STREAM_H__

#include "bstd.h"
#include "bthumbnail.h"
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct bthumbnail_stream *bthumbnail_stream_t;

typedef struct bthumbnail_stream_create_settings {
    void *user_context;
    const char *name; /* used for debug. memory is not copied. */
} bthumbnail_stream_create_settings;

void bthumbnail_stream_get_default_create_settings(
    bthumbnail_stream_create_settings *create_settings /* [out] */
    );

bthumbnail_stream_t bthumbnail_stream_create(
    bthumbnail_manager_t manager,
    const bthumbnail_stream_create_settings *create_settings
    );

void bthumbnail_stream_get_create_settings(
    bthumbnail_stream_t stream,
    bthumbnail_stream_create_settings *create_settings /* [out] */
    );

void bthumbnail_stream_destroy(
    bthumbnail_stream_t stream
    );

/**
A bthumbnail_stream supports two views:
1) view_window - a subset of the stream. may have narrower spacing.
2) full_view - spans the entire stream. should have very wide spacing.
**/
typedef struct bthumbnail_stream_settings {
    unsigned first_time; /* bounds for the entire stream, in msec */
    unsigned last_time; /* bounds for the entire stream, in msec */

    struct {
        bool visible; /* is the view window being viewed on the UI right now? */
        unsigned spacing; /* in seconds */
        unsigned first_time, last_time; /* current window in stream which is being viewed */
    } view_window;

    struct {
        bool visible; /* is the full view being viewed on the UI right now? */
        unsigned number; /* number of stills to span the stream */
        unsigned min_spacing; /* in seconds */
    } full_view;
} bthumbnail_stream_settings;

void bthumbnail_stream_get_settings(
    bthumbnail_stream_t stream,
    bthumbnail_stream_settings *settings /* [out] */
    );

int bthumbnail_stream_set_settings(
    bthumbnail_stream_t stream,
    const bthumbnail_stream_settings *settings
    );

typedef struct bthumbnail_stream_data {
    unsigned total_thumbnails; /* number of thumbnails created */
} bthumbnail_stream_data;

void bthumbnail_stream_get_data(
    bthumbnail_stream_t stream,
    bthumbnail_stream_data *data /* [out] */
    );

/* get all thumbnails already available within a window */
int bthumbnail_stream_get_thumbnails(
    bthumbnail_stream_t stream,
    unsigned start_time,           /* in seconds */
    unsigned end_time,           /* in seconds */
    bthumbnail_t *thumbnail_array, /* size_is(amount) array of thumbnails */
    unsigned total,                /* maximum of thumbnails that can be retrieved */
    unsigned *actual_read          /* [out] actual number of bthumbnail_t's populated in thumbnail_array */
    );

/* get all thumbnails already available based on the "full_view" settings */
int bthumbnail_stream_get_full_view_thumbnails(
    bthumbnail_stream_t stream,
    bthumbnail_t *thumbnail_array, /* size_is(amount) array of thumbnails */
    unsigned total,                /* maximum of thumbnails that can be retrieved */
    unsigned *actual_read          /* [out] actual number of bthumbnail_t's populated in thumbnail_array */
    );

#ifdef __cplusplus
}
#endif

#endif /* BTHUMBNAIL_STREAM_H__ */
