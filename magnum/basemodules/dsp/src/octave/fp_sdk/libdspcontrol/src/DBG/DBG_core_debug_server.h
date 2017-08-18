/****************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

/* Internal header file relating to DEBUG SERVER core component */
#ifndef _DBG_CORE_DEBUG_SERVER_H_
#define _DBG_CORE_DEBUG_SERVER_H_

#include <stdint.h>
#include <stdbool.h>

#include "libdspcontrol/CHIP.h"
#include "libdspcontrol/DSP.h"
#include "libdspcontrol/DBG.h"

#include "DBG_core_debug_server_common.h"

#define DBG_BP_NUM_INVALID      -1
#define DBG_WP_NUM_INVALID      -1
/**
 * Enumeration */

typedef struct
{
    uint8_t *p_u8_buf;          /* Pointer to begining of the buffer */
    uint8_t u8_valid_bytes;     /* Number of valid bytes */
    uint8_t u8_buf_size;        /* Total buffer size */
} DEBUG_SERVER_BUFFER_T;

typedef struct DBG_BPT_EVENTS_T
{
    char                    b_cmd_type; /* Insert or remove bpt/wpt */
    DBG_BPT_WPT_TYPE        bpt_type;     /* BPT_SW/BPT_HW/WP_W/WP_R/WP_A */
    uint32_t                u32_addr;
    uint32_t                u32_wp_num_bytes;
    struct DBG_BPT_EVENTS_T *p_event_next;

} DBG_BPT_EVENTS_T;

typedef struct
{
    DBG_BPT_EVENTS_T *p_head;
    DBG_BPT_EVENTS_T *p_tail;
}DBG_BPT_EVENT_QUEUE_T;

struct DBG_core_debug_server_sp
{
    DBG_CORE_STATE              core_state;
    uint16_t                    u16_bpt_mask;       /* Breakpoint mask */
    uint32_t                    *p_u32_bpt_address;     /* Array to store PCs for breakpoints */
    uint8_t                     u8_wp_mask; /* 2bits for each watchpoint (replicating DIR_WP_CONTROL bitmap) */
    uint32_t                    *p_u32_wp_address_top;
    uint32_t                    *p_u32_wp_address_bottom;
    bool                        b_gdb_extended_mode;

    DEBUG_SERVER_GDB_BUFFER_T   buffer_gdb_recv_pack;   /* buffer to receive gdb packet from host */
    DEBUG_SERVER_GDB_BUFFER_T   buffer_gdb_send_pack;
    DEBUG_SERVER_BUFFER_T       buffer_dbp_cmd;         /* buffer to send dbp packet to the target */
    DEBUG_SERVER_BUFFER_T       buffer_dbp_resp;        /* buffer to receive dbp response from the target */
    DEBUG_SERVER_GDB_PACKET     gdb_packet;
    DBG_BPT_EVENT_QUEUE_T   bp_wp_queue;
};

#endif
