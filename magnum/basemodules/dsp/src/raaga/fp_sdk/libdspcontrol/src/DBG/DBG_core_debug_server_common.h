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

/* Internal header file relating to DEBUG SERVER core component */
#ifndef _DBG_CORE_DEBUG_SERVER_COMMON_H_
#define _DBG_CORE_DEBUG_SERVER_COMMON_H_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "libdspcontrol/CHIP.h"
#include "libdspcontrol/DBG.h"

#if FEATURE_IS(DBG_CORE, DEBUG_SERVER)

#define DBG_DBP_BUFFER_SIZE     128
/*The GDB buffer should be minimum twice the size of the DBP buffer */
/* We allocate the buffer size to hold response to a 'g' packet     */
/* For Octave PIKE we need 1544 * 2 bytes + overhead for GDB chksum */
/* Allocate additional bytes for future expansion                   */
#define DBG_GDB_BUFFER_SIZE     (DBG_DBP_BUFFER_SIZE * 2 * 13) + 32

#elif FEATURE_IS(DBG_CORE, DEBUG_SERVER_MP)
#define DBG_GDB_BUFFER_SIZE 0x1000
#endif

#ifdef __FP4014__
#define DBG_MAX_NUM_BP          16
#define DBG_MAX_NUM_WP          4
#else
#define DBG_MAX_NUM_BP          8
#define DBG_MAX_NUM_WP          2
#endif

#define DBG_BP_NUM_INVALID      -1
#define DBG_WP_NUM_INVALID      -1

/*
 * Breakpoint and watchpoint type enumerations
 * These are enumerated as suffixes to the respective 'Z'
 * packets
 * */

#define DBG_BPT_WPT_REMOVE (0)
#define DBG_BPT_WPT_INSERT (1)
#define GDB_INTERRUPT   '\003'
/**
 * Enumeration of GDB packet types
 * */
typedef enum
{
    DBG_GDB_PACKET_TYPE_NONE,
    DBG_GDB_PACKET_TYPE_PACKET,
    DBG_GDB_PACKET_TYPE_INTERRUPT
} DBG_GDB_PACKET_TYPE;

/*Enumeration of DBP error types*/
typedef enum
{
    DBG_DBP_ERR_CODE_SUCCESS = 0,
    DBG_DBP_ERR_CODE_CHANNEL_NOT_READY,
    DBG_DBP_ERR_CODE_CANNOT_CREATE_CMD,
    DBG_DBP_ERR_CODE_CANNOT_FIT_CMD,
    DBG_DBP_ERR_CODE_CANNOT_ALLOCATE_BUFFER,
    DBG_DBP_ERR_CODE_TARGET_TIMEOUT,
    DBG_DBP_ERR_CODE_TARGET_EXCHANGE_FAILED,
    DBG_DBP_ERR_CODE_TARGET_READ_FAILED,
    DBG_DBP_ERR_CODE_TARGET_WRITE_FAILED,
    DBG_DBP_ERR_CODE_RESP_NOT_VALID
} DBG_DBP_ERR_CODE;

/**
 * Enumeration of core status
 * */
typedef enum
{
    DBG_CORE_STATE_NOT_ENABLED,             /* State indicating the core is not enabled */
    DBG_CORE_STATE_NOT_LOADED,              /* State indicating the core doesn't have a DBA to respond */
    DBG_CORE_STATE_NOT_ALIVE = DBG_CORE_STATE_NOT_LOADED,   /* Important to point NOT_ALIVE state to the last "NOT" state */
    DBG_CORE_STATE_ENABLED,                 /*FPOS - specific - core enabled - kernel and debug process state not known*/
    DBG_CORE_STATE_STOPPED = DBG_CORE_STATE_ENABLED,  /* DBA available - core spinning inside the DBA */
    DBG_CORE_STATE_STOPPED_SYSCALL,         /* DBA available - core spinning inside DBA after hitting magic syscall breakpoint */
    DBG_CORE_STATE_DP_STARTED,              /* FPOS specific - Core enabled->loaded->kernel booted->DP spawned->can process DP events */
    DBG_CORE_STATE_RUNNING,                 /* Core executing */
    DBG_CORE_STATE_RUNNING_SSTEP,           /* Core executed singlestep - internal intermidiate state */
    /*May be some more states for coredump*/
} DBG_CORE_STATE;

typedef enum
{
    DBG_BREAKPOINT_TYPE_SW = 0,
    DBG_BREAKPOINT_TYPE_HW,
    DBG_WATCHPOINT_TYPE_WRITE,
    DBG_WATCHPOINT_TYPE_READ,
    DBG_WATCHPOINT_TYPE_ACCESS
} DBG_BPT_WPT_TYPE;

typedef enum
{
    DBG_GDB_SIGNAL_NONE = 0,
    DBG_GDB_SIGNAL_INT  = 2,
    DBG_GDB_SIGNAL_TRAP = 5,
    DBG_GDB_SIGNAL_KILL = 9,
    DBG_GDB_SIGNAL_PWR  = 32,
} DBG_GDB_SIGNALS;

typedef struct
{
    bool                b_gdb_pack_valid;       /* Indicate if the packet is valid */
    DBG_GDB_PACKET_TYPE packet_type;            /* Type of the GDB packet */
    char                *p_ch_gdb_pack;         /* Pointer inside circular buffer */
                                                /* pointing to packet data */
} DEBUG_SERVER_GDB_PACKET;

typedef struct
{
    char *p_ch_buf;
    size_t s_max_buf_size;
} DEBUG_SERVER_GDB_BUFFER_T;

extern bool b_noack_mode;
extern bool b_killed;
extern const char pos_reply;
extern const char neg_reply;

/*Public function declarations*/
void
DBG_core_gdb_send_reply(DBG *dbg, char *p_pkt_data, bool b_copy);

void
DBG_core_gdb_send_signal(DBG *dbg, DBG_GDB_SIGNALS sigval);

void
DBG_core_gdb_handle_interrupt(DBG *dbg);

void
DBG_core_gdb_pack_init(DEBUG_SERVER_GDB_PACKET *p_gdb_packet);

bool
DBG_core_is_core_ready(DBG *dbg);

void
DBG_core_gdb_get_packet(DBG_HOST *p_dbg_host, DBG_core_debug_server *p_dbg_server);

void
DBG_core_gdb_reply_no_support(DBG *dbg);

void
DBG_core_gdb_reply_ok(DBG *dbg);

void
DBG_core_gdb_handle_packet(DBG *dbg);

bool
DBG_core_support_mp(void);

void
DBG_service_gdb_packet(DBG *dbg);

void
DBG_core_gdb_enable_extended_mode(DBG *dbg);

#endif
