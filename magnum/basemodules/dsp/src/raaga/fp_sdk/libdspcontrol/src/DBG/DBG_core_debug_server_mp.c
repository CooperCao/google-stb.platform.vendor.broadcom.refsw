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
/* Implementation of the 'debug_server_mp' variant of the DBG core */
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

#include "libdspcontrol/CHIP.h"
#include "libdspcontrol/COMMON.h"
#include "libdspcontrol/DSPLOG.h"
#include "libdspcontrol/DBG.h"

#include "fp_sdk_config.h"
#include "DBG_interfaces.h"

#include "DBG_core_debug_server_mp.h"
#include "DBG_core_debug_server_dbp_mp.h"

#if !(FEATURE_IS(DBG_CORE, DEBUG_SERVER_MP))
#  error "This source is for the debug_server MP variant of the DBG core"
#endif

/* ******************************************************************************************
 *                                      READ ME FIRST
 * ******************************************************************************************
 * THIS debug server MP variant is built to enable debugging a multi-process FPOS based
 * application. Unlike the normal debug server, which can support loading the applicaion
 * and enabling the core via GDB, this debug server variant operates with the core
 * previously enabled, kernel booted and a debug-process spawned by the kernel.
 */

static DBG_RET DBG_core_debug_server_init(DBG *dbg);

static DBG_RET
DBG_core_debug_server_init(DBG *dbg)
{
    DEBUG_SERVER_GDB_BUFFER_T *p_gdb_buffer;
    DBG_core_debug_server *p_dbg_server = dbg->dbg_core.p_dbg_server;
    DBG_TARGET *p_dbg_target = &dbg->dbg_target_if;
    DSP_CORES_BITMAP cores;

    /*Initialize pid list and mark the current pid*/
    p_dbg_server->pid_info_queue.p_pid_info_head = NULL;
    p_dbg_server->pid_info_queue.p_pid_info_tail = NULL;

    /*Initialize ptid*/
    p_dbg_server->hg_ptid = gdb_invalid_ptid;
    p_dbg_server->hc_ptid = gdb_invalid_ptid;

    p_dbg_server->cur_ptid_info.pid = 0;
    p_dbg_server->cur_ptid_info.tid = 0;

    /*Create GDB buffers*/
    DSPLOG_DEBUG("DBG: debug_server_init - allocating GDB buffers");

    p_gdb_buffer = &p_dbg_server->buffer_gdb_recv_pack;
    MALLOC_OR_FAIL(p_gdb_buffer->p_ch_buf, char *, DBG_GDB_BUFFER_SIZE, "DBG:");
    p_gdb_buffer->s_max_buf_size = DBG_GDB_BUFFER_SIZE;

    p_gdb_buffer = &p_dbg_server->buffer_gdb_send_pack;
    MALLOC_OR_FAIL(p_gdb_buffer->p_ch_buf, char *, DBG_GDB_BUFFER_SIZE, "DBG:");
    p_gdb_buffer->s_max_buf_size = DBG_GDB_BUFFER_SIZE;

    /*Initialize an empty GDB packet*/
    DBG_core_gdb_pack_init(&p_dbg_server->gdb_packet);

    /*Initialize DBP msg queues*/
    p_dbg_server->dbp_cmd_queue.p_dbp_msg_head = NULL;
    p_dbg_server->dbp_cmd_queue.p_dbp_msg_tail = NULL;
    p_dbg_server->dbp_resp_queue.p_dbp_msg_head = NULL;
    p_dbg_server->dbp_resp_queue.p_dbp_msg_tail = NULL;

    p_dbg_server->num_rw_mem_count = 0;

    /*Check core state*/
    DSP_enabledStatus(p_dbg_target->dsp, &cores);

    /*Check if core0 is enabled
     * FIXME: Should be if any core is enabled*/
    if( (cores & DSP_CORES_0) != 0)
        p_dbg_server->core_state = DBG_CORE_STATE_ENABLED;
    else
        p_dbg_server->core_state = DBG_CORE_STATE_NOT_ENABLED;

#ifdef DSPCONTROL_DBG_TEST
    p_dbg_server->u32_resp_flag = 0;
    p_dbg_server->t_start = time(NULL);
#endif

    p_dbg_server->b_notify_thread_events = false;
    p_dbg_server->b_gdb_extended_mode = false;
    p_dbg_server->gdb_get_pid_list_cb = NULL;
    p_dbg_server->gdb_get_pid_state_cb = NULL;
    p_dbg_server->gdb_process_freeze_cb = NULL;
    p_dbg_server->gdb_process_attach_cb = NULL;

    p_dbg_server->gdb_thread_freeze_cb = NULL;

    p_dbg_server->gdb_process_get_dso_count_cb = NULL;
    p_dbg_server->gdb_process_get_dso_sonames_cb = NULL;
    p_dbg_server->gdb_process_get_dso_segments_cb = NULL;
    p_dbg_server->gdb_process_get_dso_info_cb = NULL;

    if(DBG_core_is_core_ready(dbg))
    {
        /*configure channel by reading from the mailbox*/
        DBG_core_dbp_comms_check(dbg);
        if(p_dbg_target->b_channel_configured)
        {
            p_dbg_server->core_state = DBG_CORE_STATE_DP_STARTED;
            DSPLOG_INFO("dbg_server_init() - channel configured successfully");
        }
    }

    DSPLOG_INFO("dbg_server_init complete");

    return DBG_SUCCESS;
}

DBG_RET
DBG_init(DBG *dbg, DBG_PARAMETERS *parameters)
{
    DBG_RET dbg_ret_status;

    /*Initialize DBG target*/
    DSPLOG_DEBUG("DBG: Initialize DBG target");
    dbg_ret_status = DBG_targetInit(&dbg->dbg_target_if, parameters);

    if(DBG_FAILED(dbg_ret_status))
    {
        DSPLOG_ERROR("DBG target init failed");
        return DBG_FAILURE;
    }

    /*Initialize DBG host*/
    DSPLOG_DEBUG("DBG: Initialize DBG host");
    dbg_ret_status = DBG_hostInit(&dbg->dbg_host_if, parameters);

    if(DBG_FAILED(dbg_ret_status))
    {
        DSPLOG_ERROR("DBG host init failed");
        DBG_targetFinish(&dbg->dbg_target_if);
        return DBG_FAILURE;
    }

    /*Allocate a multi-process debug server*/
    MALLOC_OR_FAIL(dbg->dbg_core.p_dbg_server, DBG_core_debug_server *, sizeof(DBG_core_debug_server), "DBG:");
    DSPLOG_DEBUG("DBG: allocated debug_server: 0x%" PRIxPTR, (uintptr_t) dbg->dbg_core.p_dbg_server);
    /*Initialize DBG debug server mp component*/
    DSPLOG_DEBUG("DBG: Initialize DBG server");
    dbg_ret_status = DBG_core_debug_server_init(dbg);

    if(DBG_FAILED(dbg_ret_status))
    {
        DSPLOG_ERROR("DBG core debug server MP init failed");
        DBG_targetFinish(&dbg->dbg_target_if);
        DBG_hostFinish(&dbg->dbg_host_if);
        return DBG_FAILURE;
    }

    return DBG_SUCCESS;
}

void
DBG_finish(DBG *dbg)
{
    DBG_hostFinish(&dbg->dbg_host_if);
    DBG_targetFinish(&dbg->dbg_target_if);
}

#ifdef DSPCONTROL_DBG_TEST
static void
dbg_dspcontrol_test(DBG *dbg)
{
    DBG_core_debug_server *p_dbg_server = dbg->dbg_core.p_dbg_server;
    DBG_TARGET *p_dbg_target = &dbg->dbg_target_if;

    static uint32_t u32_test_cmd_flags = DBP_CMD_GET_DBP_CONFIG;

        /*Tick the comms layer and handle any responses*/
    if(p_dbg_target->b_channel_configured)
    {
        /*Wait for the target to spawn the user process*/
        static bool b_user_process_start = false;
        if(!b_user_process_start && ((time(NULL) - p_dbg_server->t_start) < 80) )
            return;

        b_user_process_start = true;

        switch(u32_test_cmd_flags)
        {
            case DBP_CMD_GET_DBP_CONFIG:
                DSPLOG_INFO("submitting DBP_CMD_GET_PID_LIST");
                u32_test_cmd_flags = DBP_CMD_INVALID;
                if(DBG_dbp_get_pid_list(p_dbg_server) == DBG_DBP_ERR_CODE_SUCCESS)
                    u32_test_cmd_flags = DBP_CMD_GET_PID_LIST;
                break;

            case DBP_CMD_GET_PID_LIST:
                if(p_dbg_server->u32_resp_flag == DBP_CMD_GET_PID_LIST)
                {
                    p_dbg_server->u32_resp_flag = 0;
                    u32_test_cmd_flags = DBP_CMD_INVALID;
                    /*Get the state of the first process*/
                    dbg_core_pid_info_t *p_pid_info = DBG_core_peek_pid_info(&p_dbg_server->pid_info_queue);
                    if(p_pid_info == NULL)
                    {
                        DSPLOG_ERROR("dbg_dspcontrol_test(): gets 0 pid");
                        return;
                    }
                    if(DBG_dbp_process_get_state(p_dbg_server, p_pid_info->pid) == DBG_DBP_ERR_CODE_SUCCESS)
                        u32_test_cmd_flags = DBP_CMD_PROCESS_GET_STATE;
                }
                break;

            default:
                break;
        }

    }
}
#endif


DBG_RET
DBG_service(DBG *dbg)
{
#ifdef DSPCONTROL_DBG_TEST
    dbg_dspcontrol_test(dbg);
#else
    DBG_service_gdb_packet(dbg);
#endif

    /*Tick the comms layer and handle any responses*/
    DBG_core_dbp_comms_check(dbg);
    return DBG_SUCCESS;
}
