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

/* Internal header file relating to DEBUG SERVER (Multi process) core component */
#ifndef _DBG_CORE_DEBUG_SERVER_MP_H_
#define _DBG_CORE_DEBUG_SERVER_MP_H_

#include <stdint.h>
#include <stdbool.h>

#include "libdspcontrol/CHIP.h"
#include "libdspcontrol/DSP.h"
#include "libdspcontrol/DBG.h"

#include "DBG_core_pid_tid.h"
#include "DBG_core_mp_dbp_msg.h"
#include "DBG_core_debug_server_common.h"

//#define DSPCONTROL_DBG_TEST 1

#ifdef DSPCONTROL_DBG_TEST
#include <time.h>
#endif

typedef struct
{
    int16_t pid;
    int16_t tid;
}gdb_ptid_t;

typedef struct
{
    dbg_pid_t           pid;                /*Process-id*/
    dbg_tid_t           tid;                /*Thread-id*/
}dbg_core_ptid_info_t;

typedef enum
{
    DBG_RESUME_TYPE_STOPPED,
    DBG_RESUME_TYPE_STEP,
    DBG_RESUME_TYPE_CONTINUE
}dbg_resume_type_t;

typedef struct
{
    dbg_pid_t           pid;
    dbg_tid_t           tid;
    dbg_resume_type_t   resume_type;
}dbg_core_resume_info_t;

extern gdb_ptid_t gdb_null_ptid;
extern gdb_ptid_t gdb_minus_one_ptid;
extern gdb_ptid_t gdb_invalid_ptid;

/*gdb callback functions*/
typedef void (*DBG_core_gdb_get_pid_list_cb_t)(DBG *dbg, bool b_err_status);
typedef void (*DBG_core_gdb_get_pid_state_cb_t)(DBG *dbg, bool b_err_status);
typedef void (*DBG_core_gdb_process_freeze_cb_t)(DBG *dbg, bool b_err_status);
typedef void (*DBG_core_gdb_process_attach_cb_t)(DBG *dbg, bool b_err_status);
typedef void (*DBG_core_gdb_process_get_tid_list_cb_t)(DBG *dbg, bool b_err_status);

typedef void (*DBG_core_gdb_thread_freeze_cb_t)(DBG *dbg, bool b_err_status);

typedef void (*DBG_core_gdb_process_get_dso_count_cb_t)(DBG *dbg, bool b_err_status);
typedef void (*DBG_core_gdb_process_get_dso_sonames_cb_t)(DBG *dbg, int start, int count, bool b_err_status);
typedef void (*DBG_core_gdb_process_get_dso_segments_cb_t)(DBG *dbg, bool b_err_status);
typedef void (*DBG_core_gdb_process_get_dso_info_cb_t)(DBG *dbg, bool b_err_status);

struct dbg_core_debug_server_mp
{
    DBG_CORE_STATE              core_state;

    gdb_ptid_t                  hg_ptid;        /*Thread set by the Hg packet*/
    gdb_ptid_t                  hc_ptid;        /*Thread set by the Hc /vCont packet*/

    dbg_core_ptid_info_t        cur_ptid_info;  /*Current process-info/thread-info under debug*/
    dbg_core_pid_info_queue_t   pid_info_queue; /*Queue of process_info under debug*/

    /*DBP cmd and response queues*/
    dbg_dbp_msg_queue_t         dbp_cmd_queue;
    dbg_dbp_msg_queue_t         dbp_resp_queue;

    /*GDB buffers*/
    DEBUG_SERVER_GDB_BUFFER_T   buffer_gdb_recv_pack;
    DEBUG_SERVER_GDB_BUFFER_T   buffer_gdb_send_pack;
    DEBUG_SERVER_GDB_PACKET     gdb_packet;

    /*
     * Counter to keep track of number of read/write memory
     * commands issued to the target per m/M packet.
     * This is necessary to ensure that we have received
     * responses for all the commands (we cannot rely on
     * the size parameter as the target may return less
     * than the actual size)
     * */
    int                         num_rw_mem_count;   /*mem access event count*/
    bool                        b_notify_thread_events;
    bool                        b_gdb_extended_mode;

    /*GDB callback functions*/
    DBG_core_gdb_get_pid_list_cb_t              gdb_get_pid_list_cb;
    DBG_core_gdb_get_pid_state_cb_t             gdb_get_pid_state_cb;
    DBG_core_gdb_process_freeze_cb_t            gdb_process_freeze_cb;
    DBG_core_gdb_process_attach_cb_t            gdb_process_attach_cb;
    DBG_core_gdb_process_get_tid_list_cb_t      gdb_process_get_tid_list_cb;
    DBG_core_gdb_thread_freeze_cb_t             gdb_thread_freeze_cb;
    DBG_core_gdb_process_get_dso_count_cb_t     gdb_process_get_dso_count_cb;
    DBG_core_gdb_process_get_dso_sonames_cb_t   gdb_process_get_dso_sonames_cb;
    DBG_core_gdb_process_get_dso_segments_cb_t  gdb_process_get_dso_segments_cb;
    DBG_core_gdb_process_get_dso_info_cb_t      gdb_process_get_dso_info_cb;

#ifdef DSPCONTROL_DBG_TEST
    time_t                      t_start;
    uint32_t                    u32_resp_flag;
#endif
};
#endif
