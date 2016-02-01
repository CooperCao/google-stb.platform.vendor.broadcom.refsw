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

#ifndef BTHUMBNAIL_PRIV_H__
#define BTHUMBNAIL_PRIV_H__

#include "bstd.h"
#include "bkni.h"
#include "blst_list.h"

BDBG_OBJECT_ID_DECLARE(bthumbnail_manager);
BDBG_OBJECT_ID_DECLARE(bthumbnail_stream);
BDBG_OBJECT_ID_DECLARE(bthumbnail);

struct bthumbnail_manager {
    BDBG_OBJECT(bthumbnail_manager)
    BLST_D_HEAD(bthumbnail_stream_list, bthumbnail_stream) list;
    BLST_D_HEAD(bthumbnail_dangling_list, bthumbnail) dangling;
    bthumbnail_manager_settings settings;
    void *compressed_buffer;
    unsigned total_size;
    unsigned total_thumbnails;
    bool wait_for_change; /* avoid an infinite busy loop of creating and destroying the same thumbnail over and over again */
    unsigned amount_short; /* how many still were we unable to request. when reducing memory, clean enough for those. */
};

struct bthumbnail_stream {
    BDBG_OBJECT(bthumbnail_stream)
    bthumbnail_manager_t manager;
    BLST_D_ENTRY(bthumbnail_stream) link; /* list in bthumbnail_manager */
    BLST_D_HEAD(bthumbnail_list, bthumbnail) list;
    bthumbnail_stream_create_settings create_settings;
    bthumbnail_stream_settings settings;
    uint8_t endcode_packet[188]; /* TODO: support non-188 DSS and timestamp streams? */
    unsigned actual_full_view_spacing;
};

struct bthumbnail {
    BDBG_OBJECT(bthumbnail)
    BLST_D_ENTRY(bthumbnail) link; /* list in bthumbnail_stream */
    bthumbnail_data data;
    bthumbnail_manager_t manager; /* set if this was deleted while is_decoding is true and now the manager owns it in the dangling list */
    unsigned size;
};

bthumbnail_t bthumbnail_stream_p_create_next_thumbnail(bthumbnail_stream_t stream);
bthumbnail_t bthumbnail_p_create_thumbnail( bthumbnail_stream_t stream, unsigned time );
void bthumbnail_p_destroy( bthumbnail_t thumbnail );
int bthumbnail_p_request_decode(bthumbnail_t thumbnail, uint8_t *buffer, unsigned buffer_size, unsigned *amount_read);
void bthumbnail_manager_p_delete_surface(bthumbnail_manager_t manager, bthumbnail_t thumbnail);
bool bthumbnail_stream_p_delete_thumbnails(bthumbnail_stream_t stream, bool aggressive);

#endif /* BTHUMBNAIL_PRIV_H__ */
