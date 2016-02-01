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

#ifndef BTHUMBNAIL_MANAGER_H__
#define BTHUMBNAIL_MANAGER_H__

#include "bstd.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
The bthumbnail library has no internal synchronization. You must not call it re-entrantly.
You must not call bthumbnail from a bthumbnail callback.

The bthumbnail library depends on:
    bstd.h (from Magnum)
    bmedia_player
    stdio

All thumbnail decoding, destriping and scaling is handled by means of callbacks.
**/

typedef struct bthumbnail_manager *bthumbnail_manager_t;

typedef struct bthumbnail_manager_settings {
    unsigned maximum_memory;    /* bthumbnail_manager will free up the least-necessary thumbnails when this is exceeded */

    unsigned compressed_buffer_size; /* size of maximum uncompressed still picture */

    void (*request_ready)(void *context); /* notify the app when it should call bthumbnail_manager_get_next_request */
    void (*delete_surface)(void *context, void *surface); /* request application to delete a void *surface which was passed into bthumbnail_manager_set_surface */
    void *callback_context;
} bthumbnail_manager_settings;

void bthumbnail_manager_get_default_settings(
    bthumbnail_manager_settings *settings /* [out] */
    );

bthumbnail_manager_t bthumbnail_manager_create(
    const bthumbnail_manager_settings *settings
    );

void bthumbnail_manager_destroy(
    bthumbnail_manager_t manager
    );

/**
The application must ask the manager for the next bthumbnail_t that requires a still decode.
The application should use bthumbnail_extractor decode the still, then report the results by calling bthumbnail_manager_set_surface.

Call bthumbnail_get_data to learn stream and codec information.

If it returns non-zero, then there is no bthumbnail_t that needs to be decoded. App can wait for request_ready callback
before calling again.
**/
int bthumbnail_manager_get_next_request(
    bthumbnail_manager_t manager,
    bthumbnail_t *thumbnail /* [out] */
    );

/**
Give the bsurface_t which resulted from the last call to bthumbnail_manager_get_next_request.
bthumbnail_manager will now own the bsurface_t and will deallocate it when it wants to.
The application should not remember the bthumbnail_t as it could be deallocated.
**/
int bthumbnail_manager_set_surface(
    bthumbnail_manager_t manager,
    bthumbnail_t thumbnail,
    void *surface, /* application-defined surface handle. see bthumbnail_manager_settings.delete_surface. */
    unsigned size  /* size of the surface */
    );

/**
Summary:
Make all streams invisible.

Description:
A UI can use this to make all streams invisible, then make visible the specific streams it wants.
**/
void bthumbnail_manager_clear_visible(
    bthumbnail_manager_t manager
    );

#ifdef __cplusplus
}
#endif

#endif /* BTHUMBNAIL_MANAGER_H__ */
