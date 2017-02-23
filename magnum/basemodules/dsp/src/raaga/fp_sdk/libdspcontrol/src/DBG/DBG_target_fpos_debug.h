/****************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
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
 ****************************************************************************/

#ifndef DBG_TARGET_FPOS_DEBUG_H
#define DBG_TARGET_FPOS_DEBUG_H

#include <stdint.h>
#include <stdlib.h>

#include "libdspcontrol/DSP.h"
#include "libdspcontrol/DBG.h"
#include "libdspcontrol/src/DSP_octave_maestro.h"

#define DEST_HOST   0
#define DEST_DSP    1

typedef union
{
    struct __attribute__((packed))
    {
        bool b_hdr_valid : 1;
        bool b_hdr_dest : 1;
        bool b_hdr_last_packet : 1;
        unsigned u_reserved : 13;
        unsigned u16_hdr_pack_length : 16;
    } dbp_thdr_bitfields;

    uint32_t u32_thdr;
} dbp_thdr_t;

#ifndef __FIREPATH__

void
DBG_target_read_thdr(DBG_TARGET *p_dbg_target, dbp_thdr_t *p_thdr);

void
DBG_target_write_thdr(DBG_TARGET *p_dbg_target, dbp_thdr_t *p_thdr);

DBG_TARGET_ERR_CODE
DBG_target_read(DBG_TARGET *p_dbg_target, dbp_thdr_t *p_thdr,
                uint8_t *p_u8_receive_buffer, size_t *p_bytes_read);

DBG_TARGET_ERR_CODE
DBG_target_write(DBG_TARGET *p_dbg_target, dbp_thdr_t *p_thdr,
                 uint8_t *p_u8_send_buffer, size_t *p_bytes_sent);

DBG_TARGET_ERR_CODE
DBG_target_acquire_mutex(DBG_TARGET *p_dbg_target);

DBG_TARGET_STATE
DBG_target_query_state_from_channel(DBG_TARGET *p_dbg_target);

uint16_t
DBG_target_query_packet_length(DBG_TARGET *p_dbg_target);

DBG_TARGET_STATE
DBG_targetQueryState(dbp_thdr_t *p_thdr);

void
DBG_target_release_mutex(DBG_TARGET *p_dbg_target);

#endif

#endif
