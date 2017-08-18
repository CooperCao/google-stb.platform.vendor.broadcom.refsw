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

#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

#include "libdspcontrol/CHIP.h"
#include "libdspcontrol/DSPLOG.h"

#include "fp_sdk_config.h"

#include "DBG_core_debug_server_gdb_mp.h"
#include "DBG_core_debug_server_dbp_mp.h"
#include "DBG_core_mp_dbp_msg.h"
#include "DBG_core_pid_tid.h"
#include "DBG_interfaces.h"

#if !(FEATURE_IS(DBG_CORE, DEBUG_SERVER_MP))
#  error "This source is for the debug_server variant of the DBG core"
#endif

#if FEATURE_IS(DBG_TARGET_IF, FPOS_DEBUG)
#include "DBG_target_fpos_debug.h"
#endif

#if !FEATURE_IS(LIBC, HOSTED)
#  error "This compilation unit requires support for dynamic memory allocation"
#endif
/****************************************************************************
 *                          Static function declarations
 * *************************************************************************/

static void
DBG_create_dbp_header(dbp_command_t *p_dbp_command, uint16_t u16_msg_len, DBP_CMD_TYPE_ID command);

static void
DBG_core_print_dbp_response(dbp_response_t *p_dbp_resp);

/*DBP response function declarations*/
static void
DBG_handle_dbp_config_resp(dbp_response_t *p_dbp_response, DBG_core_debug_server *p_dbg_server, DBG_TARGET *p_dbg_target);

static void
DBG_handle_get_pid_list_resp(dbp_response_t *p_dbp_response, DBG *dbg);

static void
DBG_handle_process_get_name_resp(dbp_response_t *p_dbp_response, DBG *dbg);

static void
DBG_handle_process_get_tid_list_resp(dbp_response_t *p_dbp_response, DBG *dbg);

static void
DBG_handle_process_get_state_resp(dbp_response_t *p_dbp_response, DBG *dbg);

static void
DBG_handle_process_freeze_resp(dbp_response_t *p_dbp_response, DBG *dbg);

static void
DBG_handle_process_unfreeze_resp(dbp_response_t *p_dbp_response, DBG_core_debug_server *p_dbg_server);

static void
DBG_handle_process_attach_resp(dbp_response_t *p_dbp_resp, DBG *dbg);

static void
DBG_handle_process_detach_resp(dbp_response_t *p_dbp_resp, DBG *dbg);

static void
DBG_handle_process_read_memory_resp(dbp_response_t *p_dbp_response, DBG *dbg);

static void
DBG_handle_process_write_memory_resp(dbp_response_t *p_dbp_response, DBG *dbg);

static void
DBG_handle_process_create_resp(dbp_response_t *p_dbp_respons, DBG *dbg);

static void
DBG_handle_process_terminate_resp(dbp_response_t *p_dbp_response, DBG *dbg);

static void
DBG_handle_thread_freeze_resp(dbp_response_t *p_dbp_response, DBG *dbg);

static void
DBG_handle_thread_unfreeze_resp(dbp_response_t *p_dbp_response, DBG *dbg);

static void
DBG_handle_thread_get_state_resp(dbp_response_t *p_dbp_response, DBG *dbg);

static void
DBG_handle_thread_read_register_resp(dbp_response_t *p_dbp_response, DBG *dbg);

static void
DBG_handle_thread_write_register_resp(dbp_response_t *p_dbp_response, DBG *dbg);

static void
DBG_handle_thread_read_all_registers_resp(dbp_response_t *p_dbp_response, DBG *dbg);

static void
DBG_handle_process_get_dso_count_resp(dbp_response_t *p_dbp_response, DBG *dbg);

static void
DBG_handle_process_get_dso_sonames_resp(dbp_response_t *p_dbp_response, DBG *dbg);

static void
DBG_handle_process_get_dso_segments_resp(dbp_response_t *p_dbp_response, DBG *dbg);

static void
DBG_handle_process_get_dso_sba_resp(dbp_response_t *p_dbp_response, DBG *dbg);

static void
DBG_handle_process_get_dso_info_resp(dbp_response_t *p_dbp_response, DBG *dbg);

static void
DBG_handle_process_get_exe_segments_resp(dbp_response_t *p_dbp_response, DBG *dbg);

static void
DBG_handle_process_get_exe_sba_resp(dbp_response_t *p_dbp_response, DBG *dbg);

static void
DBG_handle_set_breakpoint_resp(dbp_response_t *p_dbp_response, DBG *dbg);

static void
DBG_handle_remove_breakpoint_resp(dbp_response_t *p_dbp_response, DBG *dbg);

static void
DBG_handle_set_watchpoint_resp(dbp_response_t *p_dbp_response, DBG *dbg);

static void
DBG_handle_remove_watchpoint_resp(dbp_response_t *p_dbp_response, DBG *dbg);

static void
DBG_handle_thread_set_single_step_resp(dbp_response_t *p_dbp_response, DBG *dbg);

static void
DBG_handle_thread_clear_single_step_resp(dbp_response_t *p_dbp_response, DBG *dbg);

static void
DBG_handle_breakpoint_hit(dbp_response_t *p_dbp_response, DBG *dbg);

static void
DBG_handle_single_step_complete(dbp_response_t *p_dbp_response, DBG *dbg);

static void
DBG_handle_watchpoint_hit(dbp_response_t *p_dbp_response, DBG *dbg);

static void
DBG_handle_resp_process_exited(dbp_response_t *p_dbp_response, DBG *dbg);

static void
DBG_handle_resp_thread_created(dbp_response_t *p_dbp_response, DBG *dbg);

static void
DBG_handle_resp_thread_exited(dbp_response_t *p_dbp_response, DBG *dbg);

static void
DBG_handle_resp_library_loaded(dbp_response_t *p_dbp_response, DBG *dbg);

static void
DBG_handle_resp_library_unloaded(dbp_response_t *p_dbp_response, DBG *dbg);

static void
DBG_handle_resp_fatal(dbp_response_t *p_dbp_response, DBG *dbg);

static void
DBG_core_print_dbp_response(dbp_response_t *p_dbp_resp)
{
    DSPLOG_DEBUG("msglen : 0x%x", p_dbp_resp->msglen);
    DSPLOG_DEBUG("response: 0x%x", p_dbp_resp->response);
    DSPLOG_DEBUG("status : 0x%x", p_dbp_resp->status);

    uint16_t u16_msg_len = p_dbp_resp->msglen - sizeof(struct dbp_header);
    uint8_t *p_u8_buf = (uint8_t *)p_dbp_resp + sizeof(struct dbp_header);

    while(u16_msg_len > 0)
    {
        DSPLOG_DEBUG("0x%x", *p_u8_buf++);
        u16_msg_len--;
    }
}

/*
 * @brief   Handle DBP responses from the target
 *
 * The DBP comms mechanism enqueues DBP responses from the target and this
 * function dequeues the responses and dispatches them to the respective response
 * handler functions. This function also frees the memory occupied by the
 * response message.
 *
 * @param   p_dbg_server    Pointer to the debug server
 * @param   p_dbg_target    Pointer to the debug target component
 * */

DBG_DBP_ERR_CODE
DBG_core_dbp_handle_resp(DBG *dbg)
{
    DBG_core_debug_server *p_dbg_server = dbg->dbg_core.p_dbg_server;
    DBG_TARGET *p_dbg_target = &dbg->dbg_target_if;

    dbg_dbp_msg_queue_t *p_dbp_msg_resp_queue = &p_dbg_server->dbp_resp_queue;

    dbg_dbp_msg_t *p_dbp_resp_msg;

    /*Check if there are messages in the channel*/
    while((p_dbp_resp_msg = DBG_core_dequeue_dbp_msg(p_dbp_msg_resp_queue)) != NULL)
    {

        /*Decode the response message and handle individual responses*/
        dbp_response_t *p_dbp_resp = (dbp_response_t *)p_dbp_resp_msg->p_u8_buf;

        DBG_core_print_dbp_response(p_dbp_resp);

        /*Sanity check*/
        if(p_dbp_resp->msglen != p_dbp_resp_msg->u16_buf_size)
        {
            DSPLOG_ERROR("Length of the message in the response header (0x%x) doesn't match with the buffer length (0x%x)",
                    p_dbp_resp->msglen, p_dbp_resp_msg->u16_buf_size);
            return DBG_DBP_ERR_CODE_RESP_NOT_VALID;
        }

        /*Handle the message response*/
        switch(p_dbp_resp->response & 0x3F)
        {
            case DBP_CMD_GET_DBP_CONFIG:
                DSPLOG_INFO("Handle dbp_config response");
                DBG_handle_dbp_config_resp(p_dbp_resp, p_dbg_server, p_dbg_target);
                break;

            case DBP_CMD_GET_PID_LIST:
                DSPLOG_INFO("Handle DBP_CMD_GET_PID_LIST response - 0x%x", p_dbp_resp->response);
                DBG_handle_get_pid_list_resp(p_dbp_resp, dbg);
                break;

            case DBP_CMD_PROCESS_GET_NAME:
                DSPLOG_INFO("Handle DBP_CMD_PROCESS_GET_NAME response - 0x%x", p_dbp_resp->response);
                DBG_handle_process_get_name_resp(p_dbp_resp, dbg);
                break;

            case DBP_CMD_PROCESS_GET_TID_LIST:
                DSPLOG_INFO("Handle DBP_CMD_PROCESS_GET_TID_LIST response");
                DBG_handle_process_get_tid_list_resp(p_dbp_resp, dbg);
                break;

            case DBP_CMD_PROCESS_GET_STATE:
                DSPLOG_INFO("Handle DBP_CMD_PROCESS_GET_STATE response");
                DBG_handle_process_get_state_resp(p_dbp_resp, dbg);
                break;

            case DBP_CMD_PROCESS_FREEZE:
                DSPLOG_INFO("Handle DBP_CMD_PROCESS_FREEZE response");
                DBG_handle_process_freeze_resp(p_dbp_resp, dbg);
                break;

            case DBP_CMD_PROCESS_UNFREEZE:
                DSPLOG_INFO("Handle DBP_CMD_PROCESS_UNFREEZE response");
                DBG_handle_process_unfreeze_resp(p_dbp_resp, p_dbg_server);
                break;

            case DBP_CMD_PROCESS_ATTACH:
                DSPLOG_INFO("Handle DBP_CMD_PROCESS_ATTACH response");
                DBG_handle_process_attach_resp(p_dbp_resp, dbg);
                break;

            case DBP_CMD_PROCESS_DETACH:
                DSPLOG_INFO("Handle DBP_CMD_PROCESS_DETACH response");
                DBG_handle_process_detach_resp(p_dbp_resp, dbg);
                break;

            case DBP_CMD_PROCESS_READ_MEMORY:
                DSPLOG_INFO("Handle DBP_CMD_PROCESS_READ_MEMORY response");
                DBG_handle_process_read_memory_resp(p_dbp_resp, dbg);
                break;

            case DBP_CMD_PROCESS_WRITE_MEMORY:
                DSPLOG_INFO("Handle DBP_CMD_PROCESS_WRITE_MEMORY response");
                DBG_handle_process_write_memory_resp(p_dbp_resp, dbg);
                break;

            case DBP_CMD_PROCESS_CREATE:
                DSPLOG_INFO("Handle DBP_CMD_PROCESS_CREATE response");
                DBG_handle_process_create_resp(p_dbp_resp, dbg);
                break;

            case DBP_CMD_PROCESS_TERMINATE:
                DSPLOG_INFO("Handle DBP_CMD_PROCESS_TERMINATE response");
                DBG_handle_process_terminate_resp(p_dbp_resp, dbg);
                break;

            case DBP_CMD_THREAD_FREEZE:
                DSPLOG_INFO("Handle DBP_CMD_THREAD_FREEZE response");
                DBG_handle_thread_freeze_resp(p_dbp_resp, dbg);
                break;

            case DBP_CMD_THREAD_UNFREEZE:
                DSPLOG_INFO("Handle DBP_CMD_THREAD_UNFREEZE response");
                DBG_handle_thread_unfreeze_resp(p_dbp_resp, dbg);
                break;

            case DBP_CMD_THREAD_GET_STATE:
                DSPLOG_INFO("Handle DBP_CMD_THREAD_GET_STATE response");
                DBG_handle_thread_get_state_resp(p_dbp_resp, dbg);
                break;

            case DBP_CMD_THREAD_READ_REGISTER:
                DSPLOG_INFO("Handle DBP_CMD_THREAD_READ_REGISTER response");
                DBG_handle_thread_read_register_resp(p_dbp_resp, dbg);
                break;

            case DBP_CMD_THREAD_WRITE_REGISTER:
                DSPLOG_INFO("Handle DBP_CMD_THREAD_WRITE_REGISTER response");
                DBG_handle_thread_write_register_resp(p_dbp_resp, dbg);
                break;

            case DBP_CMD_THREAD_READ_ALL_REGISTERS:
                DSPLOG_INFO("Handle DBP_CMD_THREAD_READ_ALL_REGISTERS response");
                DBG_handle_thread_read_all_registers_resp(p_dbp_resp, dbg);
                break;

            case DBP_CMD_PROCESS_GET_DSO_COUNT:
                DSPLOG_INFO("Handle DBP_CMD_PROCESS_GET_DSO_COUNT response");
                DBG_handle_process_get_dso_count_resp(p_dbp_resp, dbg);
                break;

            case DBP_CMD_PROCESS_GET_DSO_SONAMES:
                DSPLOG_INFO("Handle DBP_CMD_PROCESS_GET_DSO_SONAMES response");
                DBG_handle_process_get_dso_sonames_resp(p_dbp_resp, dbg);
                break;

            case DBP_CMD_PROCESS_GET_DSO_SEGMENTS:
                DSPLOG_INFO("Handle DBP_CMD_PROCESS_GET_DSO_SEGMENTS response");
                DBG_handle_process_get_dso_segments_resp(p_dbp_resp, dbg);
                break;

            case DBP_CMD_PROCESS_GET_DSO_SBA:
                DSPLOG_INFO("Handle DBP_CMD_PROCESS_GET_DSO_SBA response");
                DBG_handle_process_get_dso_sba_resp(p_dbp_resp, dbg);
                break;

            case DBP_CMD_PROCESS_GET_DSO_INFO:
                DSPLOG_INFO("Handle DBP_CMD_PROCESS_GET_DSO_INFO response");
                DBG_handle_process_get_dso_info_resp(p_dbp_resp, dbg);
                break;

            case DBP_CMD_PROCESS_GET_EXE_SEGMENTS:
                DSPLOG_INFO("Handle DBP_CMD_PROCESS_GET_EXE_SEGMENTS response");
                DBG_handle_process_get_exe_segments_resp(p_dbp_resp, dbg);
                break;

            case DBP_CMD_PROCESS_GET_EXE_SBA:
                DSPLOG_INFO("Handle DBP_CMD_PROCESS_GET_EXE_SBA response");
                DBG_handle_process_get_exe_sba_resp(p_dbp_resp, dbg);
                break;

            case DBP_CMD_SET_BREAKPOINT:
                DSPLOG_INFO("Handle DBP_CMD_SET_BREAKPOINT response");
                DBG_handle_set_breakpoint_resp(p_dbp_resp, dbg);
                break;

            case DBP_CMD_REMOVE_BREAKPOINT:
                DSPLOG_INFO("Handle DBP_CMD_REMOVE_BREAKPOINT response");
                DBG_handle_remove_breakpoint_resp(p_dbp_resp, dbg);
                break;

            case DBP_CMD_SET_WATCHPOINT:
                DSPLOG_INFO("Handle DBP_CMD_SET_WATCHPOINT response");
                DBG_handle_set_watchpoint_resp(p_dbp_resp, dbg);
                break;

            case DBP_CMD_REMOVE_WATCHPOINT:
                DSPLOG_INFO("Handle DBP_CMD_SET_WATCHPOINT response");
                DBG_handle_remove_watchpoint_resp(p_dbp_resp, dbg);
                break;

            case DBP_CMD_THREAD_SET_SINGLE_STEP:
                DSPLOG_INFO("Handle DBP_CMD_THREAD_SET_SINGLE_STEP response");
                DBG_handle_thread_set_single_step_resp(p_dbp_resp, dbg);
                break;

            case DBP_CMD_THREAD_CLEAR_SINGLE_STEP:
                DSPLOG_INFO("Handle DBP_CMD_THREAD_CLEAR_SINGLE_STEP response");
                DBG_handle_thread_clear_single_step_resp(p_dbp_resp, dbg);
                break;

            case DBP_RESP_BREAKPOINT_HIT:
                DSPLOG_INFO("Handle DBP_RESP_BREAKPOINT_HIT response");
                DBG_handle_breakpoint_hit(p_dbp_resp, dbg);
                break;

            case DBP_RESP_WATCHPOINT_HIT:
                DSPLOG_INFO("Handle DBP_RESP_WATCHPOINT_HIT response");
                DBG_handle_watchpoint_hit(p_dbp_resp, dbg);
                break;

            case DBP_RESP_SINGLE_STEP_COMPLETE:
                DSPLOG_INFO("Handle DBP_RESP_SINGLE_STEP_COMPLETE response");
                DBG_handle_single_step_complete(p_dbp_resp, dbg);
                break;

            case DBP_RESP_PROCESS_EXITED:
                DSPLOG_INFO("Handle DBP_RESP_PROCESS_EXITED response");
                DBG_handle_resp_process_exited(p_dbp_resp, dbg);
                break;

            case DBP_RESP_THREAD_CREATED:
                DSPLOG_INFO("Handle DBP_RESP_THREAD_CREATED response");
                DBG_handle_resp_thread_created(p_dbp_resp, dbg);
                break;

            case DBP_RESP_THREAD_EXITED:
                DSPLOG_INFO("Handle DBP_RESP_THREAD_EXITED response");
                DBG_handle_resp_thread_exited(p_dbp_resp, dbg);
                break;

            case DBP_RESP_LIBRARY_LOADED:
                DSPLOG_INFO("Handle DBP_RESP_LIBRARY_LOADED response");
                DBG_handle_resp_library_loaded(p_dbp_resp, dbg);
                break;

            case DBP_RESP_LIBRARY_UNLOADED:
                DSPLOG_INFO("Handle DBP_RESP_LIBRARY_UNLOADED response");
                DBG_handle_resp_library_unloaded(p_dbp_resp, dbg);
                break;

            case DBP_RESP_FATAL:
                DSPLOG_INFO("Handle DBP_RESP_FATAL response");
                DBG_handle_resp_fatal(p_dbp_resp, dbg);
                break;

            default:
                DSPLOG_ERROR("core_dbp_handle_resp: Unrecogonized response");
                break;
        }

        /*free the response message*/
        DBG_core_dbp_msg_dealloc(p_dbp_resp_msg);
    }
    return DBG_DBP_ERR_CODE_SUCCESS;
}

/*
 * @brief   Helper function which creats a DBP header
 *
 * Simply populates the DBP header part of the DBP command message
 *
 * @param   p_dbp_command   Pointer to the DBP command which needs to be populated
 * @param   u16_msg_len     Total length of the DBP message
 * @param   command         Command type
 * */
static void
DBG_create_dbp_header(dbp_command_t *p_dbp_command, uint16_t u16_msg_len, DBP_CMD_TYPE_ID command)
{
    p_dbp_command->dbp_header.msglen = sizeof(struct dbp_header) + u16_msg_len;
    p_dbp_command->dbp_header.command = command;
    p_dbp_command->dbp_header.reserved = 0;
}

/*
 * @brief   Create and enqueues a DBP_CMD_GET_DBP_CONFIG message
 *
 * Initial hand-shake packet. Always uses the MISC block MAILBOXES for the entire
 * command and response. The host isn’t aware of the shared buffer size or the location
 * at this point. The target returns the following:
 *  Major version
 *  Minor version
 *  Size of the shared buffer
 *  Location of the shared buffer
 *
 * @param   p_dbg_server    Pointer to the debug server
 * */
DBG_DBP_ERR_CODE
DBG_dbp_cmd_dbp_config(DBG_core_debug_server *p_dbg_server)
{
    dbp_command_t dbp_config_cmd;

    DBG_create_dbp_header(&dbp_config_cmd,
                          sizeof(dbp_get_dbp_config_cmd_t),
                          DBP_CMD_GET_DBP_CONFIG);

    /*Create and enqueue dbp_msg*/
    return DBG_core_create_and_enqueue_dbp_msg(&p_dbg_server->dbp_cmd_queue, &dbp_config_cmd);
}

/*
 *@brief    Create and enqueue a DBP_CMD_GET_PID_LIST command
 *
 * Request the target to return a list of all pids for the core 0
 *
 * @param   p_dbg_server    Pointer to the debug server*/
DBG_DBP_ERR_CODE
DBG_dbp_get_pid_list(DBG_core_debug_server *p_dbg_server)
{
    dbp_command_t dbp_get_pid_list;

    DBG_create_dbp_header(&dbp_get_pid_list,
                          sizeof(dbp_get_pid_list_cmd_t),
                          DBP_CMD_GET_PID_LIST);

    dbp_get_pid_list.get_pid_list.core = 0;  /*For now just core0*/

    /*Create and enqueue dbp_msg*/
    return DBG_core_create_and_enqueue_dbp_msg(&p_dbg_server->dbp_cmd_queue, &dbp_get_pid_list);
}

/*
 *@brief    Create and enqueue a DBP_CMD_PROCESS_GET_NAME command
 *
 * Request the target to return a list of all pids for the core 0
 *
 * @param   p_dbg_server    Pointer to the debug server
 * @param   pid             process-id
 */
DBG_DBP_ERR_CODE
DBG_dbp_process_get_name(DBG_core_debug_server *p_dbg_server, dbg_pid_t pid)
{
    dbp_command_t dbp_process_get_name;

    DBG_create_dbp_header(&dbp_process_get_name,
                          sizeof(dbp_process_get_name_cmd_t),
                          DBP_CMD_PROCESS_GET_NAME);

    dbp_process_get_name.process_get_name.pid = pid;

    /*Create and enqueue dbp_msg*/
    return DBG_core_create_and_enqueue_dbp_msg(&p_dbg_server->dbp_cmd_queue, &dbp_process_get_name);
}

/*
 * @brief   Create and enqueue a DBP_CMD_PROCESS_GET_PID_LIST command
 *
 * Request the target to return the list of all tids for a process denoted
 * by pid
 *
 * @param   p_dbg_server    Pointer to the debug server
 * @param   pid             process-id
 * */
DBG_DBP_ERR_CODE
DBG_dbp_process_get_tid_list(DBG_core_debug_server *p_dbg_server, dbg_pid_t pid )
{
    dbp_command_t dbp_process_get_tid_list;

    DBG_create_dbp_header(&dbp_process_get_tid_list,
                          sizeof(dbp_process_get_tid_list_cmd_t),
                          DBP_CMD_PROCESS_GET_TID_LIST);

    dbp_process_get_tid_list.process_get_tid_list.pid = pid;

    /*Create and enqueue dbp_msg*/
    return DBG_core_create_and_enqueue_dbp_msg(&p_dbg_server->dbp_cmd_queue, &dbp_process_get_tid_list);
}

/*
 * @brief   Create and enqueue a DBP_CMD_PROCESS_GET_STATE command
 *
 * Request the target to return the state of a process denoted
 * by pid
 *
 * @param   p_dbg_server    Pointer to the debug server
 * @param   pid             process-id
 * */
DBG_DBP_ERR_CODE
DBG_dbp_process_get_state(DBG_core_debug_server *p_dbg_server, dbg_pid_t pid )
{
    dbp_command_t dbp_process_get_state;

    DBG_create_dbp_header(&dbp_process_get_state,
                          sizeof(dbp_process_get_state_cmd_t),
                          DBP_CMD_PROCESS_GET_STATE);

    dbp_process_get_state.process_get_state.pid = pid;

    DSPLOG_INFO("dbp_process_get_state(): pid - 0x%x", pid);

    /*Create and enqueue dbp_msg*/
    return DBG_core_create_and_enqueue_dbp_msg(&p_dbg_server->dbp_cmd_queue, &dbp_process_get_state);
}

/*
 * @brief   Create and enqueue a DBP_CMD_PROCESS_FREEZE command
 *
 * Request the target to freeze a process denoted by pid
 *
 * @param   p_dbg_server    Pointer to the debug server
 * @param   pid             process-id
 * */
DBG_DBP_ERR_CODE
DBG_dbp_process_freeze(DBG_core_debug_server *p_dbg_server, dbg_pid_t pid )
{
    dbp_command_t dbp_process_freeze;

    DBG_create_dbp_header(&dbp_process_freeze,
                          sizeof(dbp_process_freeze_cmd_t),
                          DBP_CMD_PROCESS_FREEZE);

    dbp_process_freeze.process_freeze.pid = pid;

    /*Create and enqueue dbp_msg*/
    return DBG_core_create_and_enqueue_dbp_msg(&p_dbg_server->dbp_cmd_queue, &dbp_process_freeze);
}

/*
 * @brief   Create and enqueue a DBP_CMD_PROCESS_ATTACH command
 *
 * Request the target to mark the process (pid) as attached
 *
 * @param   p_dbg_server    Pointer to the debug server
 * @param   pid             process-id
 * */
DBG_DBP_ERR_CODE
DBG_dbp_process_attach(DBG_core_debug_server *p_dbg_server, dbg_pid_t pid )
{
    dbp_command_t dbp_process_attach;

    DBG_create_dbp_header(&dbp_process_attach,
                          sizeof(dbp_process_attach_cmd_t),
                          DBP_CMD_PROCESS_ATTACH);

    dbp_process_attach.process_attach.pid = pid;

    /*Create and enqueue dbp_msg*/
    return DBG_core_create_and_enqueue_dbp_msg(&p_dbg_server->dbp_cmd_queue, &dbp_process_attach);
}

/*
 * @brief   Create and enqueue a DBP_CMD_PROCESS_DETACH command
 *
 * Request the target to mark the process (pid) as detached
 *
 * @param   p_dbg_server    Pointer to the debug server
 * @param   pid             process-id
 * */
DBG_DBP_ERR_CODE
DBG_dbp_process_detach(DBG_core_debug_server *p_dbg_server, dbg_pid_t pid )
{
    dbp_command_t dbp_process_detach;

    DBG_create_dbp_header(&dbp_process_detach,
                          sizeof(dbp_process_detach_cmd_t),
                          DBP_CMD_PROCESS_DETACH);

    dbp_process_detach.process_detach.pid = pid;

    /*Create and enqueue dbp_msg*/
    return DBG_core_create_and_enqueue_dbp_msg(&p_dbg_server->dbp_cmd_queue, &dbp_process_detach);
}

/*
 * @brief   Create and enqueue a DBP_CMD_PROCESS_UNFREEZE command
 *
 * Request the target to unfreeze a process denoted by pid
 *
 * @param   p_dbg_server    Pointer to the debug server
 * @param   pid             process-id
 * */
DBG_DBP_ERR_CODE
DBG_dbp_process_unfreeze(DBG_core_debug_server *p_dbg_server, dbg_pid_t pid )
{
    dbp_command_t dbp_process_unfreeze;

    DBG_create_dbp_header(&dbp_process_unfreeze,
                          sizeof(dbp_process_unfreeze_cmd_t),
                          DBP_CMD_PROCESS_UNFREEZE);

    dbp_process_unfreeze.process_unfreeze.pid = pid;

    /*Create and enqueue dbp_msg*/
    return DBG_core_create_and_enqueue_dbp_msg(&p_dbg_server->dbp_cmd_queue, &dbp_process_unfreeze);
}

/*
 * @brief   Create and enqueue a DBP_CMD_PROCESS_READ_MEMORY command
 *
 * Read a block of memory from the target for the process denoted by pid.
 * We assume the size of the memory block requested will be within the
 * size of the channel - so that the response can be fitted in a single
 * packet
 *
 * @param   p_dbg_server    Pointer to the debug server
 * @param   pid             process-id
 * @param   u32_address     address to read
 * @param   u16_size        size of the block to read
 * */
DBG_DBP_ERR_CODE
DBG_dbp_process_read_memory(DBG_core_debug_server *p_dbg_server, dbg_pid_t pid,
                            uint32_t u32_address, uint16_t u16_size)
{
    dbp_command_t dbp_process_read_memory;

    DBG_create_dbp_header(&dbp_process_read_memory,
                          sizeof(dbp_process_read_memory_cmd_t),
                          DBP_CMD_PROCESS_READ_MEMORY);

    dbp_process_read_memory.process_read_memory.pid = pid;
    dbp_process_read_memory.process_read_memory.address = u32_address;
    dbp_process_read_memory.process_read_memory.size = u16_size;

    /*Create and enqueue dbp_msg*/
    return DBG_core_create_and_enqueue_dbp_msg(&p_dbg_server->dbp_cmd_queue, &dbp_process_read_memory);
}
/*
 * @brief   Create and enqueue a DBP_CMD_PROCESS_WRITE_MEMORY command
 *
 * Write a block of memory to the target for the process denoted by pid.
 * We assume the size of the memory block we write will be within the
 * size of the channel - so that the command can be fitted in a single
 * packet
 *
 * @param   p_dbg_server    Pointer to the debug server
 * @param   pid             process-id
 * @param   u32_address     address to read
 * @param   u16_size        size of the block to read
 * @param   p_u8_buf        Pointer to the data to be written
 * */
DBG_DBP_ERR_CODE
DBG_dbp_process_write_memory(DBG_core_debug_server *p_dbg_server,
                             dbg_pid_t pid,
                            uint32_t u32_address,
                            uint16_t u16_size,
                            uint8_t *p_u8_buf)
{
    dbp_command_t dbp_process_write_memory;

    DBG_create_dbp_header(&dbp_process_write_memory,
                          sizeof(dbp_process_write_memory_cmd_t) + (u16_size * sizeof(uint8_t)),
                          DBP_CMD_PROCESS_WRITE_MEMORY);

    dbp_process_write_memory.process_write_memory.pid = pid;
    dbp_process_write_memory.process_write_memory.address = u32_address;
    dbp_process_write_memory.process_write_memory.size = u16_size;

    /*Create and enqueue dbp_msg*/
    return DBG_core_create_and_enqueue_dbp_msg_with_data(&p_dbg_server->dbp_cmd_queue,
                                                         &dbp_process_write_memory, p_u8_buf, u16_size);
}

/*
 * @brief   Create and enqueue a DBP_CMD_SET_BREAKPOINT command
 *
 * Set a breakpoint in one thread or all threads in a process at a given PC address
 *
 * @param   p_dbg_server    Pointer to the debug server
 * @param   pid             process-id
 * @param   tid             thread-id
 * @param   u32_pc          PC address to insert a breakpoint
 * */
DBG_DBP_ERR_CODE
DBG_dbp_set_breakpoint(DBG_core_debug_server *p_dbg_server, dbg_pid_t pid, dbg_tid_t tid, uint32_t u32_pc)
{
    dbp_command_t dbp_set_breakpoint;

    DBG_create_dbp_header(&dbp_set_breakpoint,
                          sizeof(dbp_set_breakpoint_cmd_t),
                          DBP_CMD_SET_BREAKPOINT);

    dbp_set_breakpoint.set_breakpoint.pid = pid;
    dbp_set_breakpoint.set_breakpoint.tid = tid;
    dbp_set_breakpoint.set_breakpoint.pc = u32_pc;

    /*Create and enqueue dbp_msg*/
    return DBG_core_create_and_enqueue_dbp_msg(&p_dbg_server->dbp_cmd_queue, &dbp_set_breakpoint);
}

/*
 * @brief   Create and enqueue a DBP_CMD_REMOVE_BREAKPOINT command
 *
 * Remove a breakpoint denoted by breakpoint number set for one thread
 * or all threads in a process
 *
 * @param   p_dbg_server    Pointer to the debug server
 * @param   pid             process-id
 * @param   tid             thread-id
 * @param   bpnum           breakpoint number
 * * */
DBG_DBP_ERR_CODE
DBG_dbp_remove_breakpoint(DBG_core_debug_server *p_dbg_server,
                          dbg_pid_t pid, dbg_tid_t tid,
                          uint8_t u8_bp_num)
{
    dbp_command_t dbp_remove_breakpoint;

    DBG_create_dbp_header(&dbp_remove_breakpoint,
                          sizeof(dbp_remove_breakpoint_cmd_t),
                          DBP_CMD_REMOVE_BREAKPOINT);

    dbp_remove_breakpoint.remove_breakpoint.pid = pid;
    dbp_remove_breakpoint.remove_breakpoint.tid = tid;
    dbp_remove_breakpoint.remove_breakpoint.bp = u8_bp_num;

    /*Create and enqueue dbp_msg*/
    return DBG_core_create_and_enqueue_dbp_msg(&p_dbg_server->dbp_cmd_queue, &dbp_remove_breakpoint);
}

/*
 * @brief   Create and enqueue a DBP_CMD_SET_WATCHPOINT command
 *
 * Set a watchpoint in one thread or all threads in a process at a given PC address
 *
 * @param   p_dbg_server    Pointer to the debug server
 * @param   pid             process-id
 * @param   tid             thread-id
 * @param   u32_address     address to watch by the watchpoint
 * @param   u32_length      Coverage of watchpoint in bytes
 * @param   type            Watchpoint type (one of READ, WRITE, ACCESS)
 * */
DBG_DBP_ERR_CODE
DBG_dbp_set_watchpoint(DBG_core_debug_server *p_dbg_server,
                              dbg_pid_t pid, dbg_tid_t tid,
                              uint32_t address,
                              uint32_t length,
                              DBP_WPT_TYPE_ID type)
{
    dbp_command_t dbp_set_watchpoint;

    DBG_create_dbp_header(&dbp_set_watchpoint,
                          sizeof(dbp_set_watchpoint_cmd_t),
                          DBP_CMD_SET_WATCHPOINT);

    dbp_set_watchpoint.set_watchpoint.pid = pid;
    dbp_set_watchpoint.set_watchpoint.tid = tid;
    dbp_set_watchpoint.set_watchpoint.address = address;
    dbp_set_watchpoint.set_watchpoint.length = length;
    dbp_set_watchpoint.set_watchpoint.type = type;

    /*Create and enqueue dbp_msg*/
    return DBG_core_create_and_enqueue_dbp_msg(&p_dbg_server->dbp_cmd_queue, &dbp_set_watchpoint);
}

/*
 * @brief   Create and enqueue a DBP_CMD_REMOVE_WATCHPOINT command
 *
 * Remove a watchpoint denoted by watchpoint number set for one thread
 * or all threads in a process
 *
 * @param   p_dbg_server    Pointer to the debug server
 * @param   pid             process-id
 * @param   tid             thread-id
 * @param   bpnum           watchpoint number
 * * */
DBG_DBP_ERR_CODE
DBG_dbp_remove_watchpoint(DBG_core_debug_server *p_dbg_server,
                          dbg_pid_t pid, dbg_tid_t tid,
                          uint8_t u8_wp_num)
{
    dbp_command_t dbp_remove_watchpoint;

    DBG_create_dbp_header(&dbp_remove_watchpoint,
                          sizeof(dbp_remove_watchpoint_cmd_t),
                          DBP_CMD_REMOVE_WATCHPOINT);

    dbp_remove_watchpoint.remove_watchpoint.pid = pid;
    dbp_remove_watchpoint.remove_watchpoint.tid = tid;
    dbp_remove_watchpoint.remove_watchpoint.wp = u8_wp_num;

    /*Create and enqueue dbp_msg*/
    return DBG_core_create_and_enqueue_dbp_msg(&p_dbg_server->dbp_cmd_queue, &dbp_remove_watchpoint);
}

/*
 * @brief   Create and enqueue a DBP_CMD_THREAD_SET_SINGLE_STEP command
 *
 * Set the singlestep bit for the thread tid under process pid.
 * The thread remains in the FROZEN state - Single step will be executed
 * upon UNFREEZE of the thread
 *
 * @param   p_dbg_server    Pointer to the debug server
 * @param   pid             process-id
 * @param   tid             thread-id
 * */
DBG_DBP_ERR_CODE
DBG_dbp_thread_set_single_step(DBG_core_debug_server *p_dbg_server, dbg_pid_t pid, dbg_tid_t tid)
{
    dbp_command_t dbp_thread_set_single_step;

    DBG_create_dbp_header(&dbp_thread_set_single_step,
                           sizeof(dbp_thread_set_single_step_cmd_t),
                           DBP_CMD_THREAD_SET_SINGLE_STEP);

    dbp_thread_set_single_step.thread_set_single_step.pid = pid;
    dbp_thread_set_single_step.thread_set_single_step.tid = tid;

    /*Create and enqueue dbp message*/
    return DBG_core_create_and_enqueue_dbp_msg(&p_dbg_server->dbp_cmd_queue, &dbp_thread_set_single_step);
}

/*
 * @brief   Create and enqueue a DBP_CMD_THREAD_CLEAR_SINGLE_STEP command
 *
 * Unset the singlestep bit for the thread tid under process pid
 *
 * @param   p_dbg_server    Pointer to the debug server
 * @param   pid             process-id
 * @param   tid             thread-id
 * */
DBG_DBP_ERR_CODE
DBG_dbp_thread_clear_single_step(DBG_core_debug_server *p_dbg_server, dbg_pid_t pid, dbg_tid_t tid)
{
    DSPLOG_DEBUG("create and enqueue clear single_step for 0x%x.0x%x", pid, tid);
    dbp_command_t dbp_thread_clear_single_step;

    DBG_create_dbp_header(&dbp_thread_clear_single_step,
                           sizeof(dbp_thread_clear_single_step_cmd_t),
                           DBP_CMD_THREAD_CLEAR_SINGLE_STEP);

    dbp_thread_clear_single_step.thread_clear_single_step.pid = pid;
    dbp_thread_clear_single_step.thread_clear_single_step.tid = tid;

    /*Create and enqueue dbp message*/
    return DBG_core_create_and_enqueue_dbp_msg(&p_dbg_server->dbp_cmd_queue, &dbp_thread_clear_single_step);
}

/*
 * @brief   Create and enqueue a DBP_CMD_THREAD_GET_STATE command
 *
 * Request the target to return the state of a thread denoted
 * by tid in a process pid
 *
 * @param   p_dbg_server    Pointer to the debug server
 * @param   pid             process-id
 * @param   tid             thread-id
 * */
DBG_DBP_ERR_CODE
DBG_dbp_thread_get_state(DBG_core_debug_server *p_dbg_server, dbg_pid_t pid, dbg_tid_t tid )
{
    dbp_command_t dbp_thread_get_state;

    DBG_create_dbp_header(&dbp_thread_get_state,
                          sizeof(dbp_thread_get_state_cmd_t),
                          DBP_CMD_THREAD_GET_STATE);

    dbp_thread_get_state.thread_get_state.pid = pid;
    dbp_thread_get_state.thread_get_state.tid = tid;

    /*Create and enqueue dbp_msg*/
    return DBG_core_create_and_enqueue_dbp_msg(&p_dbg_server->dbp_cmd_queue, &dbp_thread_get_state);
}

/*
 * @brief   Create and enqueue a DBP_CMD_THREAD_FREEZE command
 *
 * Request the target to freeze a thread denoted by tid in a
 * process pid
 *
 * @param   p_dbg_server    Pointer to the debug server
 * @param   pid             process-id
 * @param   tid             thread-id
 * */
DBG_DBP_ERR_CODE
DBG_dbp_thread_freeze(DBG_core_debug_server *p_dbg_server, dbg_pid_t pid, dbg_tid_t tid)
{
    dbp_command_t dbp_thread_freeze;

    DBG_create_dbp_header(&dbp_thread_freeze,
                          sizeof(dbp_thread_freeze_cmd_t),
                          DBP_CMD_THREAD_FREEZE);

    dbp_thread_freeze.thread_freeze.pid = pid;
    dbp_thread_freeze.thread_freeze.tid = tid;

    /*Create and enqueue dbp_msg*/
    return DBG_core_create_and_enqueue_dbp_msg(&p_dbg_server->dbp_cmd_queue, &dbp_thread_freeze);
}

/*
 * @brief   Create and enqueue a DBP_CMD_THREAD_UNFREEZE command
 *
 * Request the target to unfreeze a thread denoted by tid in a
 * process pid
 *
 * @param   p_dbg_server    Pointer to the debug server
 * @param   pid             process-id
 * @param   tid             thread-id
 * */
DBG_DBP_ERR_CODE
DBG_dbp_thread_unfreeze(DBG_core_debug_server *p_dbg_server, dbg_pid_t pid, dbg_tid_t tid)
{
    dbp_command_t dbp_thread_unfreeze;

    DBG_create_dbp_header(&dbp_thread_unfreeze,
                          sizeof(dbp_thread_unfreeze_cmd_t),
                          DBP_CMD_THREAD_UNFREEZE);

    dbp_thread_unfreeze.thread_unfreeze.pid = pid;
    dbp_thread_unfreeze.thread_unfreeze.tid = tid;

    /*Create and enqueue dbp_msg*/
    return DBG_core_create_and_enqueue_dbp_msg(&p_dbg_server->dbp_cmd_queue, &dbp_thread_unfreeze);
}

/*
 * @brief   Create and enqueue a DBP_CMD_THREAD_READ_REGISTER command
 *
 * Read a register from an offset in the TX_fullContext structure
 * from a thread tid in a process pid
 *
 * @param   p_dbg_server    Pointer to the debug server
 * @param   pid             process-id
 * @param   tid             thread-id
 * @param   reg_offset      offset in the TX_fullContext structure
 * @param   reg_size        size of the register to be read
 * */
DBG_DBP_ERR_CODE
DBG_dbp_thread_read_register(DBG_core_debug_server *p_dbg_server, dbg_pid_t pid, dbg_tid_t tid, uint32_t reg_offset, uint32_t reg_size)
{
    DSPLOG_INFO("dbp_thread_read_register(): read register @offset - 0x%x size - 0x%x for p0x%x.0x%x", reg_offset, reg_size, pid, tid);
    dbp_command_t dbp_thread_read_register;

    DBG_create_dbp_header(&dbp_thread_read_register,
                          sizeof(dbp_thread_read_register_cmd_t),
                          DBP_CMD_THREAD_READ_REGISTER);

    dbp_thread_read_register.thread_read_register.pid = pid;
    dbp_thread_read_register.thread_read_register.tid = tid;
    dbp_thread_read_register.thread_read_register.reg_offset = reg_offset;
    dbp_thread_read_register.thread_read_register.reg_size = reg_size;

    /*Create and enqueue dbp msg*/
    return DBG_core_create_and_enqueue_dbp_msg(&p_dbg_server->dbp_cmd_queue, &dbp_thread_read_register);
}

/*
 * @brief   Create and enqueue a DBP_CMD_THREAD_WRITE_REGISTER command
 *
 * Write a register denoted by u16_regnum with a value u64_value for a thread
 * tid in a process pid
 *
 * @param   p_dbg_server    Pointer to the debug server
 * @param   pid             process-id
 * @param   tid             thread-id
 * @param   u16_regnum      Register number
 * @param   u64_value       Value of the register
 * */
DBG_DBP_ERR_CODE
DBG_dbp_thread_write_register(DBG_core_debug_server *p_dbg_server,
                              dbg_pid_t pid, dbg_tid_t tid, uint16_t u16_regnum,
                              uint64_t u64_value)
{
    dbp_command_t dbp_thread_write_register;

    DBG_create_dbp_header(&dbp_thread_write_register,
                          sizeof(dbp_thread_write_register_cmd_t),
                          DBP_CMD_THREAD_WRITE_REGISTER);

    dbp_thread_write_register.thread_write_register.pid = pid;
    dbp_thread_write_register.thread_write_register.tid = tid;
    dbp_thread_write_register.thread_write_register.regnum = u16_regnum;
    dbp_thread_write_register.thread_write_register.value = u64_value;

    /*Create and enqueue dbp msg*/
    return DBG_core_create_and_enqueue_dbp_msg(&p_dbg_server->dbp_cmd_queue, &dbp_thread_write_register);
}

/*
 * @brief   Create and enqueue a DBP_CMD_THREAD_READ_ALL_REGISTERS command
 *
 * Request the target to read all the registers from the thread context for the thread
 * denoted by pid.tid
 *
 * @param   p_dbg_server    Pointer to the debug server
 * @param   pid             process-id
 * @param   tid             thread-id
 * */

DBG_DBP_ERR_CODE
DBG_dbp_thread_read_all_registers(DBG_core_debug_server *p_dbg_server, dbg_pid_t pid, dbg_tid_t tid)
{
    dbp_command_t dbp_thread_read_all_registers;

    DBG_create_dbp_header(&dbp_thread_read_all_registers,
                          sizeof(dbp_thread_read_all_registers_cmd_t),
                          DBP_CMD_THREAD_READ_ALL_REGISTERS);

    dbp_thread_read_all_registers.thread_read_all_registers.pid = pid;
    dbp_thread_read_all_registers.thread_read_all_registers.tid = tid;

    /*Create and enqueue dbp msg*/
    return DBG_core_create_and_enqueue_dbp_msg(&p_dbg_server->dbp_cmd_queue, &dbp_thread_read_all_registers);
}

/*
 * @brief   Create and enqueue a DBP_CMD_PROCESS_GET_DSO_COUNT
 *
 * Request the target the number of dynamic shared objects linked for the user process pid
 *
 * @param   p_dbg_server    Pointer to the debug server
 * @param   pid             process-id*/
DBG_DBP_ERR_CODE
DBG_dbp_process_get_dso_count(DBG_core_debug_server *p_dbg_server, dbg_pid_t pid)
{
    dbp_command_t   dbp_process_get_dso_count;

    DBG_create_dbp_header(&dbp_process_get_dso_count,
                          sizeof(dbp_process_get_dso_count_cmd_t),
                          DBP_CMD_PROCESS_GET_DSO_COUNT);

    dbp_process_get_dso_count.process_get_dso_count.pid = pid;

    /*Create and enqueu dbp msg*/
    return DBG_core_create_and_enqueue_dbp_msg(&p_dbg_server->dbp_cmd_queue, &dbp_process_get_dso_count);
}

/*
 * @brief   Create and enqueue a DBP_CMD_PROCESS_GET_DSO_SONAMES
 *
 * Request the target the names of the dynamic shared objects linked
 * for the user process pid, starting from the index start
 *
 * @param   p_dbg_server    Pointer to the debug server
 * @param   pid             process-id
 * @param   start           Index of the first soname to return
 * */
DBG_DBP_ERR_CODE
DBG_dbp_process_get_dso_sonames(DBG_core_debug_server *p_dbg_server, dbg_pid_t pid,
                                uint32_t start)
{
    dbp_command_t   dbp_process_get_dso_sonames;

    DBG_create_dbp_header(&dbp_process_get_dso_sonames,
                          sizeof(dbp_process_get_dso_sonames_cmd_t),
                          DBP_CMD_PROCESS_GET_DSO_SONAMES);

    dbp_process_get_dso_sonames.process_get_dso_sonames.pid = pid;
    dbp_process_get_dso_sonames.process_get_dso_sonames.start = start;

    /*Create and enqueu dbp msg*/
    return DBG_core_create_and_enqueue_dbp_msg(&p_dbg_server->dbp_cmd_queue, &dbp_process_get_dso_sonames);
}

/*
 * @brief   Create and enqueue a DBP_CMD_PROCESS_GET_DSO_SEGMENTS
 *
 * Request the target the segments the dynamic shared objects linked
 * for the user process pid, starting from the index start
 *
 * @param   p_dbg_server    Pointer to the debug server
 * @param   pid             process-id
 * @param   start           Index of dso to be queried
 * */
DBG_DBP_ERR_CODE
DBG_dbp_process_get_dso_segments(DBG_core_debug_server *p_dbg_server, dbg_pid_t pid,
                                 uint32_t index)
{
    dbp_command_t   dbp_process_get_dso_segments;

    DBG_create_dbp_header(&dbp_process_get_dso_segments,
                          sizeof(dbp_process_get_dso_segments_cmd_t),
                          DBP_CMD_PROCESS_GET_DSO_SEGMENTS);

    dbp_process_get_dso_segments.process_get_dso_segments.pid = pid;
    dbp_process_get_dso_segments.process_get_dso_segments.index = index;

    /*Create and enqueu dbp msg*/
    return DBG_core_create_and_enqueue_dbp_msg(&p_dbg_server->dbp_cmd_queue, &dbp_process_get_dso_segments);
}

/*
 * @brief   Create and enqueue a DBP_CMD_PROCESS_GET_DSO_SBA
 *
 * Request the target the SBA of a dynamic shared object with the index
 * linked for the user process pid
 *
 *
 * @param   p_dbg_server    Pointer to the debug server
 * @param   pid             process-id
 * @param   dso_index       Index of the dso to be queried*/
DBG_DBP_ERR_CODE
DBG_dbp_process_get_dso_sba(DBG_core_debug_server *p_dbg_server, dbg_pid_t pid, uint32_t index)
{
    dbp_command_t   dbp_process_get_dso_sba;

    DBG_create_dbp_header(&dbp_process_get_dso_sba,
                          sizeof(dbp_process_get_dso_sba_cmd_t),
                          DBP_CMD_PROCESS_GET_DSO_SBA);

    dbp_process_get_dso_sba.process_get_dso_sba.pid = pid;
    dbp_process_get_dso_sba.process_get_dso_sba.index = index;

    /*Create and enqueu dbp msg*/
    return DBG_core_create_and_enqueue_dbp_msg(&p_dbg_server->dbp_cmd_queue, &dbp_process_get_dso_sba);
}

/*
 * @brief   Create and enqueue a DBP_CMD_PROCESS_GET_DSO_INFO
 *
 * Request the target to provide all DSO related information for the process pid
 *
 * @param   p_dbg_server    Pointer to the debug server
 * @param   pid             process-id
 * */
DBG_DBP_ERR_CODE
DBG_dbp_process_get_dso_info(DBG_core_debug_server *p_dbg_server, dbg_pid_t pid)
{
    dbp_command_t   dbp_process_get_dso_info;

    DBG_create_dbp_header(&dbp_process_get_dso_info,
                          sizeof(dbp_process_get_dso_info_cmd_t),
                          DBP_CMD_PROCESS_GET_DSO_INFO);

    dbp_process_get_dso_info.process_get_dso_info.pid = pid;

    /*Create and enqueue dbp msg*/
    return DBG_core_create_and_enqueue_dbp_msg(&p_dbg_server->dbp_cmd_queue, &dbp_process_get_dso_info);
}

/*
 * @brief   Create and enqueue a DBP_CMD_PROCESS_GET_EXE_SEGMENTS command
 *
 * Request the target to get the exe segments for the process pid
 *
 * @param   p_dbg_server    Pointer to the debug server
 * @param   pid             process-id
 * */
DBG_DBP_ERR_CODE
DBG_dbp_process_get_exe_segments(DBG_core_debug_server *p_dbg_server, dbg_pid_t pid )
{
    dbp_command_t dbp_process_get_exe_segments;

    DBG_create_dbp_header(&dbp_process_get_exe_segments,
                          sizeof(dbp_process_get_exe_segments_cmd_t),
                          DBP_CMD_PROCESS_GET_EXE_SEGMENTS);

    dbp_process_get_exe_segments.process_get_exe_segments.pid = pid;

    /*Create and enqueue dbp_msg*/
    return DBG_core_create_and_enqueue_dbp_msg(&p_dbg_server->dbp_cmd_queue, &dbp_process_get_exe_segments);
}

/*
 * @brief   Create and enqueue a DBP_CMD_PROCESS_GET_EXE_SBA command
 *
 * Request the target to get the exe SBA for the process pid
 *
 * @param   p_dbg_server    Pointer to the debug server
 * @param   pid             process-id
 * */
DBG_DBP_ERR_CODE
DBG_dbp_process_get_exe_sba(DBG_core_debug_server *p_dbg_server, dbg_pid_t pid)
{
    dbp_command_t dbp_process_get_exe_sba;

    DBG_create_dbp_header(&dbp_process_get_exe_sba,
                          sizeof(dbp_process_get_exe_sba_cmd_t),
                          DBP_CMD_PROCESS_GET_EXE_SBA);

    dbp_process_get_exe_sba.process_get_exe_sba.pid = pid;

    /*Create and enqueue dbp_msg*/
    return DBG_core_create_and_enqueue_dbp_msg(&p_dbg_server->dbp_cmd_queue, &dbp_process_get_exe_sba);
}

/*
 * @brief Create and enqueue DBP_CMD_PROCESS_CREATE command
 *
 * Request the target to create a new process
 *
 * @param   p_dbg_server    Pointet to the debug server
 * @param   argc            Number of arguments
 *                          (First argument is the path to the program)
 * @param   bufferlen       length of the argv buffer
 * @param   argv            Buffer containing sequence of null-terminated argument strings
 * */
DBG_DBP_ERR_CODE
DBG_dbp_process_create(DBG_core_debug_server *p_dbg_server,
                        int argc,
                        uint16_t buffer_len,
                        char *argv)
{
    dbp_command_t dbp_process_create;

    DBG_create_dbp_header(&dbp_process_create,
                          sizeof(dbp_process_create_cmd_t) + (buffer_len * sizeof(char)),
                          DBP_CMD_PROCESS_CREATE);

    dbp_process_create.process_create.argc = argc;

    return DBG_core_create_and_enqueue_dbp_msg_with_data(&p_dbg_server->dbp_cmd_queue,
                                                        &dbp_process_create, (uint8_t *)argv, buffer_len);
}

/*
 * @brief   Create and enqueue a DBP_CMD_PROCESS_TERMINATE command
 *
 * Request the target to terminate process pid
 *
 * @param   p_dbg_server    Pointer to the debug server
 * @param   pid             process-id
 * */
DBG_DBP_ERR_CODE
DBG_dbp_process_terminate(DBG_core_debug_server *p_dbg_server, dbg_pid_t pid)
{
    dbp_command_t dbp_process_terminate;

    DBG_create_dbp_header(&dbp_process_terminate,
                          sizeof(dbp_process_terminate_cmd_t),
                          DBP_CMD_PROCESS_TERMINATE);

    dbp_process_terminate.process_terminate.pid = pid;
    dbp_process_terminate.process_terminate.exit_code = 0;

    /*Create and enqueue dbp_msg*/
    return DBG_core_create_and_enqueue_dbp_msg(&p_dbg_server->dbp_cmd_queue, &dbp_process_terminate);
}

static void
DBG_handle_dbp_config_resp(dbp_response_t *p_dbp_response, DBG_core_debug_server *p_dbg_server, DBG_TARGET *p_dbg_target)
{
    DSPLOG_DEBUG("Handle DBP_CMD_GET_DBP_CONFIG response");

    if(p_dbp_response->response & DBP_PACK)
    {
        DSPLOG_DEBUG("DBP_CMD_GET_DBP_CONFIG PACK");
        if (p_dbp_response->get_dbp_response.major != DBP_MAJOR || p_dbp_response->get_dbp_response.minor != DBP_MINOR)
        {
            DSPLOG_ERROR("DBP version mismatch");
            return;
        }
        p_dbg_target->u16_channel_size = p_dbp_response->get_dbp_response.size;
        p_dbg_target->p_u8_channel_address = (uint8_t *)(uintptr_t)p_dbp_response->get_dbp_response.address;
        p_dbg_target->b_channel_configured = true;

        p_dbg_server->core_state = DBG_CORE_STATE_DP_STARTED;

        DSPLOG_INFO("Channel configured - channel address 0x%" PRIxPTR " channel size 0x%x",
                    (uintptr_t) p_dbg_target->p_u8_channel_address, p_dbg_target->u16_channel_size);
    }
    else
        DSPLOG_ERROR("DBP_CMD_GET_DBP_CONFIG NACK - error status %x", p_dbp_response->status);
}

static void
DBG_handle_get_pid_list_resp(dbp_response_t *p_dbp_response, DBG* dbg)
{
    DBG_core_debug_server *p_dbg_server = dbg->dbg_core.p_dbg_server;

    DSPLOG_DEBUG("Handle DBP_CMD_GET_PID_LIST response 0x%d", p_dbp_response->response);
    bool b_status = true;
    int pid_num;

    if(p_dbp_response->response & DBP_PACK)
    {
        uint16_t u16_num_pids = (p_dbp_response->msglen - sizeof(struct dbp_header))/sizeof(dbg_pid_t);
        DSPLOG_INFO("DBP_CMD_GET_PID_LIST PACK num_pids - %d", u16_num_pids);

        /*For each pid in the response that doesn't contain a pid_info -
         * create a pid_info and enqueue in the pid_info_queue*/
        for(pid_num = 0; pid_num < u16_num_pids; pid_num++)
        {
            dbg_core_pid_info_t *p_pid_info = DBG_core_find_pid_info(p_dbp_response->get_pid_list.pid[pid_num], &p_dbg_server->pid_info_queue);

            if(p_pid_info == NULL)
            {
                DSPLOG_INFO("get_pid_list_resp(): Allocate pid info for pid - 0x%x", p_dbp_response->get_pid_list.pid[pid_num]);
                p_pid_info = DBG_core_pid_info_alloc(p_dbp_response->get_pid_list.pid[pid_num],
                                                      0,       /*core*/
                                                      0,       /*num_threads*/
                                                      NULL,    /*thread_list*/
                                                      DBG_PROCESS_STATE_CREATED);

                /*Enqueue the process info structure*/
                DBG_core_enqueue_pid_info(p_pid_info, &p_dbg_server->pid_info_queue);
            }
        }

        /*Prune the pid_info queue by deleting all processes that doesn't exist in the p_dbp_response*/
        dbg_core_pid_info_t *p_pid_info = DBG_core_peek_pid_info(&p_dbg_server->pid_info_queue);
        while(p_pid_info != NULL)
        {
            for(pid_num = 0; pid_num < u16_num_pids;)
            {
                if(p_pid_info->pid == p_dbp_response->get_pid_list.pid[pid_num])
                {
                    break;
                }
                else
                    pid_num++;
            }

            dbg_core_pid_info_t *p_prev_pid_info = p_pid_info;
            GET_NEXT_PID(p_pid_info);

            if(pid_num == u16_num_pids)
            {
                DSPLOG_INFO("gdb_get_pid_list_resp(): Remove pid_info for pid - 0x%x", p_prev_pid_info->pid);
                DBG_core_remove_pid_info(p_prev_pid_info->pid, &p_dbg_server->pid_info_queue);
            }
        }
    }
    else
    {
        DSPLOG_ERROR("DBP_CMD_GET_PID_LIST NACK - error status %x", p_dbp_response->status);
        b_status = false;
    }

    /*Invoke the GDB command that is expecting a response*/
    if(p_dbg_server->gdb_get_pid_list_cb != NULL)
        p_dbg_server->gdb_get_pid_list_cb(dbg, b_status);
}

static void
DBG_handle_process_get_name_resp(dbp_response_t *p_dbp_response, DBG *dbg)
{
    bool b_status = true;
    DBG_core_debug_server *p_dbg_server = dbg->dbg_core.p_dbg_server;
    DSPLOG_DEBUG("Handle DBP_CMD_PROCESS_GET_NAME response 0x%x", p_dbp_response->response);

    if(p_dbp_response->response & DBP_PACK)
    {
        DSPLOG_INFO("DBP_CMD_PROCESS_GET_NAME PACK");

        /*Update process name to the process info*/
        DBG_core_process_set_name(p_dbp_response->process_get_name.pid, &p_dbg_server->pid_info_queue,
                                     &p_dbp_response->process_get_name.process_name[0]);
    }
    else
    {
        DSPLOG_ERROR("DBP_CMD_PROCESS_GET_NAME NACK - error status 0x%x", p_dbp_response->status);
        b_status = false;
        DBG_core_remove_pid_info(p_dbp_response->process_get_name.pid, &p_dbg_server->pid_info_queue);
    }

    if(p_dbg_server->gdb_process_get_name_cb != NULL)
        p_dbg_server->gdb_process_get_name_cb(dbg, b_status);
}

static void
DBG_handle_process_get_tid_list_resp(dbp_response_t *p_dbp_response, DBG *dbg)
{
    bool b_status = true;
    DBG_core_debug_server *p_dbg_server = dbg->dbg_core.p_dbg_server;
    DSPLOG_DEBUG("Handle DBP_CMD_PROCESS_GET_TID_LIST response 0x%x", p_dbp_response->response);

    if(p_dbp_response->response & DBP_PACK)
    {
        uint16_t u16_num_threads = (p_dbp_response->msglen - sizeof(struct dbp_header) - sizeof(p_dbp_response->process_get_tid_list.pid))/sizeof(dbg_tid_t);
        DSPLOG_INFO("DBP_CMD_PROCESS_GET_TID_LIST PACK num_tids - 0x%x", u16_num_threads);

        /*Update the thread list to the process-info structure*/
        DBG_core_process_update_thread_list(p_dbp_response->process_get_tid_list.pid,
                &p_dbg_server->pid_info_queue,
                &p_dbp_response->process_get_tid_list.tid[0],
                u16_num_threads);

    }
    else
    {
        DSPLOG_ERROR("DBP_CMD_PROCESS_GET_TID_LIST NACK - error status 0x%x", p_dbp_response->status);
        b_status = false;
        DBG_core_remove_pid_info(p_dbp_response->process_get_tid_list.pid, &p_dbg_server->pid_info_queue);
    }

    if(p_dbg_server->gdb_process_get_tid_list_cb != NULL)
        p_dbg_server->gdb_process_get_tid_list_cb(dbg, b_status);
}

static void
DBG_handle_process_get_state_resp(dbp_response_t *p_dbp_response, DBG *dbg)
{
    bool b_err_status = true;
    DBG_core_debug_server *p_dbg_server = dbg->dbg_core.p_dbg_server;

    DSPLOG_DEBUG("Handle DBP_CMD_PROCESS_GET_STATE response 0x%x", p_dbp_response->response);

    if(p_dbp_response->response & DBP_PACK)
    {
        DSPLOG_INFO("DBP_CMD_PROCESS_GET_STATE PACK pid - 0x%x", p_dbp_response->process_get_state.pid);

        if(p_dbp_response->process_get_state.is_frozen)
            DBG_core_process_update_state(p_dbp_response->process_get_state.pid,
                                          &p_dbg_server->pid_info_queue,
                                          DBG_PROCESS_STATE_FROZEN );
        else
            DBG_core_process_update_state(p_dbp_response->process_get_state.pid,
                                          &p_dbg_server->pid_info_queue,
                                          DBG_PROCESS_STATE_UNFROZEN);
    }
    else
    {
        DSPLOG_ERROR("DBP_CMD_PROCESS_GET_STATE NACK - error status 0x%x pid - 0x%x", p_dbp_response->status, p_dbp_response->process_get_state.pid);
        b_err_status = false;
        DBG_core_remove_pid_info(p_dbp_response->process_get_state.pid, &p_dbg_server->pid_info_queue);
    }
    if(p_dbg_server->gdb_get_pid_state_cb != NULL)
        p_dbg_server->gdb_get_pid_state_cb(dbg, b_err_status);
}

static void
DBG_handle_process_freeze_resp(dbp_response_t *p_dbp_response, DBG* dbg)
{
    bool b_err_status = true;
    DBG_core_debug_server *p_dbg_server = dbg->dbg_core.p_dbg_server;
    DSPLOG_DEBUG("Handle DBP_CMD_PROCESS_FREEZE response 0x%x", p_dbp_response->response);

    if(p_dbp_response->response & DBP_PACK)
    {
        DSPLOG_INFO("DBP_CMD_PROCESS_FREEZE PACK - pid - 0x%x", p_dbp_response->process_freeze.pid);

        DBG_core_process_update_state(p_dbp_response->process_freeze.pid, &p_dbg_server->pid_info_queue, DBG_PROCESS_STATE_FROZEN );

        uint16_t u16_num_threads = (p_dbp_response->msglen - sizeof(struct dbp_header) - sizeof(p_dbp_response->process_freeze.pid))/sizeof(dbg_tid_t);
        DSPLOG_INFO("DBP_CMD_PROCESS_GET_TID_LIST PACK num_tids - 0x%x", u16_num_threads);

        /*Update the thread list to the process-info structure*/
        DBG_core_process_update_thread_list(p_dbp_response->process_freeze.pid,
                &p_dbg_server->pid_info_queue,
                p_dbp_response->process_freeze.tids,
                u16_num_threads);
    }
    else
    {
        DSPLOG_ERROR("DBP_CMD_PROCESS_FREEZE NACK - error status 0x%x", p_dbp_response->status);
        DBG_core_remove_pid_info(p_dbp_response->process_freeze.pid, &p_dbg_server->pid_info_queue);
        b_err_status = false;
    }
    if(p_dbg_server->gdb_process_freeze_cb)
        p_dbg_server->gdb_process_freeze_cb(dbg, b_err_status);

}

static void
DBG_handle_process_unfreeze_resp(dbp_response_t *p_dbp_response, DBG_core_debug_server *p_dbg_server)
{
    DSPLOG_DEBUG("Handle DBP_CMD_PROCESS_UNFREEZE response 0x%x", p_dbp_response->response);

    if(p_dbp_response->response & DBP_PACK)
    {
        DSPLOG_INFO("DBP_CMD_PROCESS_UNFREEZE PACK");

        DBG_core_process_update_state(p_dbp_response->process_unfreeze.pid,
                                      &p_dbg_server->pid_info_queue,
                                      DBG_PROCESS_STATE_UNFROZEN );
    }
    else
    {
        DSPLOG_ERROR("DBP_CMD_PROCESS_UNFREEZE NACK - error status 0x%x", p_dbp_response->status);
        DBG_core_remove_pid_info(p_dbp_response->process_unfreeze.pid, &p_dbg_server->pid_info_queue);
    }
}

static void
DBG_handle_process_attach_resp(dbp_response_t *p_dbp_response, DBG *dbg)
{
    DSPLOG_DEBUG("Handle DBP_CMD_PROCESS_ATTACH response 0x%x", p_dbp_response->response);
    DBG_core_debug_server *p_dbg_server = dbg->dbg_core.p_dbg_server;
    bool b_err_status = true;

    if(p_dbp_response->response & DBP_PACK)
    {
        DSPLOG_INFO("DBP_CMD_PROCESS_ATTACH PACK");
        DBG_core_attach_process(p_dbp_response->process_attach.pid,
                                &p_dbg_server->pid_info_queue);
    }
    else
    {
        DSPLOG_ERROR("DBP_CMD_PROCESS_ATTACH NACK");
        DBG_core_remove_pid_info(p_dbp_response->process_attach.pid, &p_dbg_server->pid_info_queue);
        b_err_status = false;
    }

    if(p_dbg_server->gdb_process_attach_cb)
        p_dbg_server->gdb_process_attach_cb(dbg, b_err_status);
}

static void
DBG_handle_process_detach_resp(dbp_response_t *p_dbp_response, DBG *dbg)
{
    DSPLOG_DEBUG("Handle DBP_CMD_PROCESS_DETACH response 0x%x", p_dbp_response->response);
    DBG_core_debug_server *p_dbg_server = dbg->dbg_core.p_dbg_server;

    if(p_dbp_response->response & DBP_PACK)
    {
        DSPLOG_INFO("DBP_CMD_PROCESS_DETACH PACK");
        DBG_core_detach_process(p_dbp_response->process_detach.pid,
                                &p_dbg_server->pid_info_queue);
    }
    else
    {
        DSPLOG_ERROR("DBP_CMD_PROCESS_DETACH NACK");
        DBG_core_remove_pid_info(p_dbp_response->process_detach.pid, &p_dbg_server->pid_info_queue);
    }
}

static void
DBG_handle_thread_freeze_resp(dbp_response_t *p_dbp_response, DBG *dbg)
{
    bool b_err_status = true;
    DBG_core_debug_server *p_dbg_server = dbg->dbg_core.p_dbg_server;
    DSPLOG_DEBUG("Handle DBP_CMD_THREAD_FREEZE response 0x%x", p_dbp_response->response);

    /*FIXME: will there be a missing pid_info - I hope not*/
    dbg_core_pid_info_t *p_pid_info = DBG_core_find_pid_info(p_dbp_response->thread_freeze.pid,
                                                             &p_dbg_server->pid_info_queue);

    if(p_dbp_response->response & DBP_PACK)
    {
        DSPLOG_INFO("DBP_CMD_THREAD_FREEZE PACK - pid - 0x%x tid - 0x%x", p_dbp_response->thread_freeze.pid, p_dbp_response->thread_freeze.tid);

        DBG_core_thread_update_state(p_dbp_response->thread_freeze.tid,
                                     &p_pid_info->thread_info_queue,
                                     DBG_THREAD_STATE_FROZEN, true);
    }
    else
    {
        DSPLOG_ERROR("DBP_CMD_THREAD_FREEZE NACK - error status 0x%x", p_dbp_response->status);

        DBG_core_remove_tid_info(p_dbp_response->thread_freeze.tid, &p_pid_info->thread_info_queue);
        b_err_status = false;
    }

    if(p_dbg_server->gdb_thread_freeze_cb)
        p_dbg_server->gdb_thread_freeze_cb(dbg, b_err_status);
}

static void
DBG_handle_thread_unfreeze_resp(dbp_response_t *p_dbp_response, DBG *dbg)
{
    DBG_core_debug_server *p_dbg_server = dbg->dbg_core.p_dbg_server;

    DSPLOG_DEBUG("Handle DBP_CMD_THREAD_UNFREEZE response 0x%x", p_dbp_response->response);

    dbg_core_pid_info_t *p_pid_info = DBG_core_find_pid_info(p_dbp_response->thread_unfreeze.pid,
                                                            &p_dbg_server->pid_info_queue);

    if(p_dbp_response->response & DBP_PACK)
    {
        DSPLOG_INFO("DBP_CMD_THREAD_UNFREEZE PACK");

        DBG_core_thread_update_state(p_dbp_response->thread_unfreeze.tid,
                                      &p_pid_info->thread_info_queue,
                                      DBG_THREAD_STATE_UNFROZEN , true);
    }
    else
    {
        DSPLOG_ERROR("DBP_CMD_THREAD_UNFREEZE NACK - error status 0x%x", p_dbp_response->status);
        DBG_core_remove_tid_info(p_dbp_response->thread_unfreeze.tid, &p_pid_info->thread_info_queue);
    }
}

static void
DBG_handle_thread_get_state_resp(dbp_response_t *p_dbp_response, DBG *dbg)
{
    DBG_core_debug_server *p_dbg_server = dbg->dbg_core.p_dbg_server;

    DSPLOG_DEBUG("Handle DBP_CMD_THREAD_GET_STATE response 0x%x", p_dbp_response->response);

    dbg_core_pid_info_t *p_pid_info = DBG_core_find_pid_info(p_dbp_response->thread_get_state.pid,
                                                            &p_dbg_server->pid_info_queue);

    if(p_dbp_response->response & DBP_PACK)
    {
        DSPLOG_INFO("DBP_CMD_THREAD_GET_STATE PACK p%x.%x", p_dbp_response->thread_get_state.pid,
                                                            p_dbp_response->thread_get_state.tid);

        if(p_dbp_response->thread_get_state.is_frozen)
            DBG_core_thread_update_state(p_dbp_response->thread_get_state.tid,
                                          &p_pid_info->thread_info_queue,
                                          DBG_THREAD_STATE_FROZEN, true );
        else
            DBG_core_thread_update_state(p_dbp_response->thread_get_state.tid,
                                          &p_pid_info->thread_info_queue,
                                          DBG_THREAD_STATE_UNFROZEN, true);
    }
    else
    {
        DSPLOG_ERROR("DBP_CMD_THREAD_GET_STATE NACK - error status 0x%x p%x.%x", p_dbp_response->status,
                                    p_dbp_response->thread_get_state.pid,
                                    p_dbp_response->thread_get_state.tid);

        DBG_core_remove_tid_info(p_dbp_response->thread_get_state.tid, &p_pid_info->thread_info_queue);
    }
}

static void
DBG_handle_process_read_memory_resp(dbp_response_t *p_dbp_response, DBG *dbg)
{
    DSPLOG_DEBUG("Handle DBP_CMD_PROCESS_READ_MEMORY response 0x%x", p_dbp_response->response);

    if(p_dbp_response->response & DBP_PACK)
    {
        DSPLOG_INFO("DBP_CMD_PROCESS_READ_MEMORY PACK");

        DBG_core_gdb_process_read_memory_resp(dbg, p_dbp_response->process_read_memory.address,
                p_dbp_response->process_read_memory.size, p_dbp_response->process_read_memory.data, true);

    }
    else
    {
        DSPLOG_ERROR("DBP_CMD_PROCESS_READ_MEMORY NACK - error status 0x%x", p_dbp_response->status);

        DBG_core_gdb_process_read_memory_resp(dbg,
                p_dbp_response->process_read_memory.address,
                p_dbp_response->process_read_memory.size, NULL, false);
    }
}

static void
DBG_handle_process_write_memory_resp(dbp_response_t *p_dbp_response, DBG *dbg)
{
    DBG_core_debug_server *p_dbg_server = dbg->dbg_core.p_dbg_server;
    DSPLOG_DEBUG("Handle DBP_CMD_PROCESS_WRITE_MEMORY response 0x%x", p_dbp_response->response);
    bool b_err_status = true;

    if(p_dbp_response->response & DBP_PACK)
        DSPLOG_INFO("DBP_CMD_PROCESS_WRITE_MEMORY PACK");
    else
    {
        DSPLOG_ERROR("DBP_CMD_PROCESS_READ_MEMORY NACK - error status 0x%x", p_dbp_response->status);
        b_err_status = false;
    }

    if(p_dbg_server->num_rw_mem_count)
        DBG_core_gdb_process_write_memory_resp(dbg,
                                            p_dbp_response->process_write_memory.written,
                                            b_err_status);
}

static void
DBG_handle_process_create_resp(dbp_response_t *p_dbp_response, DBG *dbg)
{
    DBG_core_debug_server *p_dbg_server = dbg->dbg_core.p_dbg_server;

    DSPLOG_DEBUG("Handle DBP_CMD_PROCESS_CREATE response 0x%x", p_dbp_response->response);
    bool b_err_status = true;

    /**/
    if(p_dbp_response->response & DBP_PACK)
    {
        DSPLOG_INFO("DBP_CMD_PROCESS_CREATE PACK");

        /*Create and enqueue pid_info for the process-id*/
        dbg_core_pid_info_t *p_pid_info = DBG_core_pid_info_alloc(p_dbp_response->process_create.pid,
                0,       /*core*/
                0,       /*num_threads*/
                NULL,    /*thread_list*/
                DBG_PROCESS_STATE_FROZEN);  /*The process is created in a frozen state*/

        /*Enqueue the process info structure*/
        DBG_core_enqueue_pid_info(p_pid_info, &p_dbg_server->pid_info_queue);

        /*Mark the process as created by the debugger*/
        DBG_core_process_created(p_pid_info);
    }
    else
    {
        DSPLOG_ERROR("Error creating process DBP_CMD_PROCESS_CREATE NACK");
        b_err_status = false;
    }

    DBG_core_gdb_process_create_resp(dbg, p_dbp_response->process_create.pid, b_err_status);
}

static void
DBG_handle_process_terminate_resp(dbp_response_t *p_dbp_response, DBG *dbg)
{
    DBG_core_debug_server *p_dbg_server = dbg->dbg_core.p_dbg_server;
    DSPLOG_DEBUG("Handle DBP_CMD_PROCESS_TERMINATE response 0x%x", p_dbp_response->response);

    if(p_dbp_response->response & DBP_PACK)
    {
        DSPLOG_INFO("DBP_CMD_PROCESS_TERMINATE PACK");

        /*Mark the process as pending termination
         * We use this flag to reliably remove the process from our pid_info_queue*/
        DBG_core_process_kill_pending(p_dbp_response->process_terminate.pid,
                                      &p_dbg_server->pid_info_queue);
    }
    else
    {
        DSPLOG_ERROR("DBP_CMD_PROCESS_TERMINATE NACK");
        /*Remove the pid_info from the pid_info_queue*/
        DBG_core_remove_pid_info(p_dbp_response->process_terminate.pid,
                &p_dbg_server->pid_info_queue);

    }
    /*If the terminated process is the current process under debug, then mark the
     * cur_ptid_info to contain invalid ptid
     * */
    if(p_dbg_server->cur_ptid_info.pid == p_dbp_response->process_terminate.pid)
    {
        p_dbg_server->cur_ptid_info.pid = gdb_invalid_ptid.pid;
        p_dbg_server->cur_ptid_info.tid = gdb_invalid_ptid.tid;
    }
    /* respond to GDB - we are not waiting for the process_exited response
     * from the target as it may lead to uncertain response time
     * */
    DBG_core_gdb_vKill_resp(dbg);
}

static void
DBG_handle_thread_read_register_resp(dbp_response_t *p_dbp_response, DBG *dbg)
{
    DSPLOG_DEBUG("Handle DBP_CMD_THREAD_READ_REGISTER response 0x%x", p_dbp_response->response);
    bool b_err_status = true;

    if(p_dbp_response->response & DBP_PACK)
        DSPLOG_INFO("DBP_CMD_THREAD_READ_REGISTER PACK");

    else
    {
        DSPLOG_ERROR("DBP_CMD_THREAD_READ_REGISTER NACK - error status 0x%x", p_dbp_response->status);
        b_err_status = false;
    }
    DBG_core_gdb_thread_read_register_resp(dbg, p_dbp_response->thread_read_register.value, p_dbp_response->thread_read_register.reg_size, b_err_status);
}

static void
DBG_handle_thread_write_register_resp(dbp_response_t *p_dbp_response, DBG *dbg)
{
    DSPLOG_DEBUG("Handle DBP_CMD_THREAD_WRITE_REGISTER response 0x%x", p_dbp_response->response);
    bool b_err_status = true;

    if(p_dbp_response->response & DBP_PACK)
        DSPLOG_INFO("DBP_CMD_THREAD_WRITE_REGISTER PACK");
    else
    {
        DSPLOG_ERROR("DBP_CMD_THREAD_WRITE_REGISTER NACK - error status 0x%x", p_dbp_response->status);
        b_err_status = false;
    }
    DBG_core_gdb_thread_write_register_resp(dbg, b_err_status);
}

static void
DBG_handle_thread_read_all_registers_resp(dbp_response_t *p_dbp_response, DBG *dbg)
{
    DSPLOG_DEBUG("Handle DBP_CMD_THREAD_READ_ALL_REGISTERS response 0x%x", p_dbp_response->response);

    if(p_dbp_response->response & DBP_PACK)
    {
        DSPLOG_INFO("DBP_CMD_THREAD_READ_ALL_REGISTERS PACK");

        DBG_core_gdb_thread_read_all_registers_resp(dbg,
                (uint8_t *)&p_dbp_response->thread_read_all_registers.context,
                true);

    }
    else
    {
        DSPLOG_ERROR("DBP_CMD_THREAD_READ_ALL_REGISTERS NACK - error status 0x%x", p_dbp_response->status);
        DBG_core_gdb_thread_read_all_registers_resp(dbg, NULL, false);
    }

    DSPLOG_INFO("dbp_thread_read_all_registers_resp(): replied");
}

static void
DBG_handle_process_get_exe_segments_resp(dbp_response_t *p_dbp_response, DBG *dbg)
{
    bool b_err_status = true;

    DSPLOG_DEBUG("Handle DBP_CMD_PROCESS_GET_EXE_SEGMENTS response 0x%x", p_dbp_response->response);

    if(p_dbp_response->response & DBP_PACK)
        DSPLOG_INFO("DBP_CMD_PROCESS_GET_EXE_SEGMENTS pack");
    else
    {
        DSPLOG_ERROR("DBP_CMD_PROCESS_GET_EXE_SEGMENTS nack - error status 0x%x", p_dbp_response->status);
        b_err_status = false;
    }

    DBG_core_gdb_handle_qOffsets_resp(dbg, p_dbp_response->process_get_exe_segments.count,
                                    (uint32_t*)p_dbp_response->process_get_exe_segments.segments,
                                    b_err_status);
}

static void
DBG_handle_process_get_exe_sba_resp(dbp_response_t *p_dbp_response, DBG *dbg)
{
    bool b_err_status = true;
    DBG_core_debug_server *p_dbg_server = dbg->dbg_core.p_dbg_server;

    DSPLOG_DEBUG("Handle DBP_CMD_PROCESS_GET_EXE_SBA response 0x%x", p_dbp_response->response);

    if(p_dbp_response->response & DBP_PACK)
    {
        DSPLOG_INFO("DBP_CMD_PROCESS_GET_EXE_SBA pack");

        if(DBG_core_process_set_sba(p_dbp_response->process_get_exe_sba.pid,
                &p_dbg_server->pid_info_queue,
                p_dbp_response->process_get_exe_sba.sba) == false)
            DSPLOG_ERROR("Setting SBA for pid %x failed", p_dbp_response->process_get_exe_sba.pid);
    }
    else
    {
        DSPLOG_ERROR("DBP_CMD_PROCESS_GET_EXE_SBA nack - error stsatus 0x%x", p_dbp_response->status);
        b_err_status = false;
    }

    DBG_core_gdb_handle_qOffsets_int(dbg, b_err_status);
}

static void
DBG_handle_process_get_dso_count_resp(dbp_response_t *p_dbp_response, DBG *dbg)
{
    bool b_err_status = true;
    DBG_core_debug_server *p_dbg_server = dbg->dbg_core.p_dbg_server;

    DSPLOG_DEBUG("Handle DBP_CMD_PROCESS_GET_DSO_COUNT response 0x%x", p_dbp_response->response);

    if(p_dbp_response->response & DBP_PACK)
    {
        DSPLOG_INFO("DBP_CMD_PROCESS_GET_DSO_COUNT pack");

        if(DBG_core_process_set_dso_count(p_dbp_response->process_get_dso_count.pid,
                                        &p_dbg_server->pid_info_queue,
                                        p_dbp_response->process_get_dso_count.count) == false)
            b_err_status = false;
    }
    else
    {
        DSPLOG_ERROR("DBP_CMD_PROCESS_GET_DSO_COUNT nack - error stsatus 0x%x", p_dbp_response->status);
        b_err_status = false;
    }

    if(p_dbg_server->gdb_process_get_dso_count_cb != NULL)
        p_dbg_server->gdb_process_get_dso_count_cb(dbg, b_err_status);
}

static void
DBG_handle_process_get_dso_sonames_resp(dbp_response_t *p_dbp_response, DBG *dbg)
{
    bool b_err_status = true;
    DBG_core_debug_server *p_dbg_server = dbg->dbg_core.p_dbg_server;

    DSPLOG_DEBUG("Handle DBP_CMD_PROCESS_GET_DSO_SONAMES response 0x%x", p_dbp_response->response);

    if(p_dbp_response->response & DBP_PACK)
    {
        DSPLOG_INFO("DBP_CMD_PROCESS_GET_DSO_SONAMES pack");
        uint32_t name;

        for(name = p_dbp_response->process_get_dso_sonames.start;
                name < p_dbp_response->process_get_dso_sonames.count; name++)
        {
            int str_index = 0;

            if(DBG_core_process_set_dso_soname(p_dbp_response->process_get_dso_sonames.pid,
                    &p_dbg_server->pid_info_queue,
                    &p_dbp_response->process_get_dso_sonames.sonames[str_index],
                    name) == false)
            {
                b_err_status = false;
                break;
            }

            str_index += strlen(&p_dbp_response->process_get_dso_sonames.sonames[str_index]) + 1;
        }
    }
    else
    {
        DSPLOG_ERROR("DBP_CMD_PROCESS_GET_DSO_SONAMES nack - error stsatus 0x%x", p_dbp_response->status);
        b_err_status = false;
    }

    if(p_dbg_server->gdb_process_get_dso_sonames_cb != NULL)
        p_dbg_server->gdb_process_get_dso_sonames_cb(dbg,
                p_dbp_response->process_get_dso_sonames.start,
                p_dbp_response->process_get_dso_sonames.count,
                b_err_status);
}

static void
DBG_handle_process_get_dso_segments_resp(dbp_response_t *p_dbp_response, DBG *dbg)
{
    bool b_err_status = true;
    DBG_core_debug_server *p_dbg_server = dbg->dbg_core.p_dbg_server;

    DSPLOG_DEBUG("Handle DBP_CMD_PROCESS_GET_DSO_SEGMENTS response 0x%x", p_dbp_response->response);

    if(p_dbp_response->response & DBP_PACK)
    {
        DSPLOG_INFO("DBP_CMD_PROCESS_GET_DSO_SEGMENTS pack");

        if(DBG_core_process_set_dso_segments(p_dbp_response->process_get_dso_segments.pid,
                &p_dbg_server->pid_info_queue,
                p_dbp_response->process_get_dso_segments.index,
                p_dbp_response->process_get_dso_segments.count,
                (uint32_t*)p_dbp_response->process_get_dso_segments.segments) == false)

            b_err_status = false;
    }
    else
    {
        DSPLOG_ERROR("DBP_CMD_PROCESS_GET_DSO_SEGMENTS nack - error stsatus 0x%x", p_dbp_response->status);
        b_err_status = false;
    }

    if(p_dbg_server->gdb_process_get_dso_segments_cb != NULL)
        p_dbg_server->gdb_process_get_dso_segments_cb(dbg, b_err_status);
}

static void
DBG_handle_process_get_dso_sba_resp(dbp_response_t *p_dbp_response, DBG *dbg)
{
    DBG_core_debug_server *p_dbg_server = dbg->dbg_core.p_dbg_server;

    DSPLOG_DEBUG("Handle DBP_CMD_PROCESS_GET_DSO_SBA response 0x%x", p_dbp_response->response);

    if(p_dbp_response->response & DBP_PACK)
    {
        DSPLOG_INFO("DBP_CMD_PROCESS_GET_DSO_SBA pack");

        if(DBG_core_process_set_dso_sba(p_dbp_response->process_get_dso_sba.pid,
                &p_dbg_server->pid_info_queue,
                p_dbp_response->process_get_dso_sba.index,
                p_dbp_response->process_get_dso_sba.sba) == false)
            DSPLOG_ERROR("Setting SBA for pid %x failed", p_dbp_response->process_get_dso_sba.pid);
    }
    else
        DSPLOG_ERROR("DBP_CMD_PROCESS_GET_DSO_SBA nack - error stsatus 0x%x", p_dbp_response->status);
}

static void
DBG_handle_process_get_dso_info_resp(dbp_response_t *p_dbp_response, DBG *dbg)
{
    bool b_err_status = true;
    DBG_core_debug_server *p_dbg_server = dbg->dbg_core.p_dbg_server;

    DSPLOG_DEBUG("Handle DBP_CMD_PROCESS_GET_DSO_INFO response 0x%x", p_dbp_response->response);

    if(p_dbp_response->response & DBP_PACK)
    {
        dbp_process_get_dso_info_rsp_t *dso_info = &p_dbp_response->process_get_dso_info;

        DSPLOG_INFO("DBP_CMD_PROCESS_GET_DSO_INFO pack count %d", dso_info->count);

        if(DBG_core_process_set_dso_count(dso_info->pid,
                                          &p_dbg_server->pid_info_queue,
                                          dso_info->count) == false)
        {
            DSPLOG_ERROR("handle_process_get_dso_info_resp(): Cannot set dso count");
            b_err_status = false;

            goto dso_info_gdb_callback;
        }

        uint8_t *p_u8_info = dso_info->info;
        uint8_t u8_dso_num;

        for(u8_dso_num = 0; u8_dso_num < dso_info->count; u8_dso_num++)
        {
            /*set dso_soname*/
            DSPLOG_INFO("handle_process_get_dso_info_resp(): dso_soname %s strlen %zu", (char *)p_u8_info, strlen((char*)p_u8_info));
            if(DBG_core_process_set_dso_soname(dso_info->pid,
                                               &p_dbg_server->pid_info_queue,
                                               (char *)p_u8_info,
                                               u8_dso_num) == false)
            {
                DSPLOG_ERROR("handle_process_get_dso_info_resp(): Cannot set dso soname for dso 0x%x", u8_dso_num);
                b_err_status = false;
                goto dso_info_gdb_callback;
            }
            p_u8_info += (strlen((char *)p_u8_info) + 1);

            /*Read num of setgments*/
            uint8_t u8_num_segments = *p_u8_info++;

            DSPLOG_INFO("handle_process_get_dso_info_resp(): Num segments 0x%x", u8_num_segments);
            /*Copy the segments*/
            if(DBG_core_process_set_dso_segments(dso_info->pid,
                                  &p_dbg_server->pid_info_queue,
                                  u8_dso_num,
                                  u8_num_segments,
                                  (uint32_t *)p_u8_info) == false)
            {
                DSPLOG_ERROR("handle_process_get_dso_info_resp(): Cannot set dso segments for dso 0x%x", u8_dso_num);
                b_err_status = false;
                goto dso_info_gdb_callback;
            }

            p_u8_info += (u8_num_segments * 2 * sizeof(uint32_t));
        }
    }
    else
    {
        DSPLOG_ERROR("DBP_CMD_PROCESS_GET_DSO_INFO nack error status - 0x%x", p_dbp_response->status);
        b_err_status = false;
    }

dso_info_gdb_callback:
    if(p_dbg_server->gdb_process_get_dso_info_cb != NULL)
    {
        DSPLOG_INFO("handle_process_get_dso_info_resp(): Invoke gdb_dso_info_cb()");
        p_dbg_server->gdb_process_get_dso_info_cb(dbg, b_err_status);
    }
}

static void
DBG_handle_set_breakpoint_resp(dbp_response_t *p_dbp_response, DBG *dbg)
{
    DBG_core_debug_server *p_dbg_server = dbg->dbg_core.p_dbg_server;
    DSPLOG_DEBUG("Handle DBP_CMD_SET_BREAKPOINT response - 0x%x", p_dbp_response->response);

    bool b_err_status = true;

    /*Get the pid_info associated with the set breakpoint response pid*/
    dbg_core_pid_info_t *p_pid_info = DBG_core_find_pid_info(p_dbp_response->set_breakpoint.pid,
            &p_dbg_server->pid_info_queue);

    if(p_pid_info == NULL)
    {
        DSPLOG_ERROR("dbp_set_breakpoint_resp(): pid - not found - should update pid_list");
        b_err_status = false;
        goto dbp_set_breakpoint_gdb_callback;
    }

    if(p_dbp_response->response & DBP_PACK)
    {
        DSPLOG_DEBUG("DBP_CMD_SET_BREAKPOINT pack");

        /*Check if the pending breakpoint we hold is the actual breakpoint being set*/
        if(p_pid_info->p_dbg_pending_bp == NULL)
        {
            DSPLOG_INFO("dbp_set_breakpoint_resp(): Create a bp_info for p%x.%x at addr 0x%x",
                    p_dbp_response->set_breakpoint.pid,
                    p_dbp_response->set_breakpoint.tid,
                    p_dbp_response->set_breakpoint.pc);

            p_pid_info->p_dbg_pending_bp = DBG_core_bp_info_alloc(p_pid_info->pid,
                    p_dbp_response->set_breakpoint.tid,
                    p_dbp_response->set_breakpoint.bp,
                    DBG_BREAKPOINT_TYPE_HW,
                    p_dbp_response->set_breakpoint.pc,
                    0);

            if(p_pid_info->p_dbg_pending_bp == NULL)
            {
                DSPLOG_ERROR("dbp_set_breakpoint_resp(): Unable to allocate breakpoint info for bp 0x%x",
                        p_dbp_response->set_breakpoint.bp);
                b_err_status = false;
                goto dbp_set_breakpoint_gdb_callback;
            }
        }
        dbg_core_bp_info_t *p_pending_bp = p_pid_info->p_dbg_pending_bp;

        if(p_pending_bp->u32_addr != p_dbp_response->set_breakpoint.pc ||
           p_pending_bp->bpt_type != DBG_BREAKPOINT_TYPE_HW)
        {
            DSPLOG_ERROR("dbp_set_breakpoint(): Breakpoint address doesn't match in response");
            b_err_status = false;
            goto dbp_set_breakpoint_gdb_callback;
        }

        DSPLOG_INFO("dbp_set_breakpoint_resp(): setting bp 0x%x @addr 0x%x for p%x.%x",
                p_dbp_response->set_breakpoint.bp,
                p_dbp_response->set_breakpoint.pc,
                p_dbp_response->set_breakpoint.pid,
                p_dbp_response->set_breakpoint.tid);

        /*Finally, Set the breakpoint number to pending_bp*/
        DBG_core_set_bp_num(p_dbp_response->set_breakpoint.bp, p_pending_bp);

        /*Enqueue the breakpoint info to the breakpoint queue*/
        DBG_core_enqueue_bp_info(p_pending_bp, &p_pid_info->bp_info_queue);

        p_pid_info->p_dbg_pending_bp = NULL;
    }
    else
    {
        DSPLOG_ERROR("dbp_set_breakpoint_resp(): Target cannot set breakpoint at addr 0x%x for p%x.%x",
                p_dbp_response->set_breakpoint.pc, p_dbp_response->set_breakpoint.pid,
                p_dbp_response->set_breakpoint.tid);

        b_err_status = false;

        p_pid_info->p_dbg_pending_bp = NULL;
    }

dbp_set_breakpoint_gdb_callback:
    DBG_core_gdb_reply_z_packet(dbg, b_err_status);
}

static void
DBG_handle_remove_breakpoint_resp(dbp_response_t *p_dbp_response, DBG *dbg)
{
    DSPLOG_DEBUG("Handle DBP_CMD_REMOVE_BREAKPOINT response - 0x%x", p_dbp_response->response);
    DBG_core_debug_server *p_dbg_server = dbg->dbg_core.p_dbg_server;
    bool b_err_status = true;

    dbg_core_pid_info_t *p_pid_info = DBG_core_find_pid_info(p_dbp_response->remove_breakpoint.pid,
            &p_dbg_server->pid_info_queue);

    if(p_pid_info == NULL)
    {
        DSPLOG_ERROR("dbp_remove_breakpoint_resp(): Unknown process in the response packet");
        b_err_status = false;
        goto dbp_remove_breakpoint_gdb_callback;
    }

    if(p_pid_info->p_dbg_pending_bp == NULL)
    {
        DSPLOG_ERROR("dbp_remove_breakpoint_resp(): Not expecting a remove breakpoint resp for the pid 0x%x",
                p_pid_info->pid);

        b_err_status = false;
        goto dbp_remove_breakpoint_gdb_callback;
    }

    /*Check if the pending bp info and the response matches*/
    dbg_core_bp_info_t *p_pending_bp = p_pid_info->p_dbg_pending_bp;

    if(p_dbp_response->response & DBP_PACK)
    {
        DSPLOG_INFO("DBP_CMD_REMOVE_BREAKPOINT PACK");

        if(p_pending_bp->bpt_type != DBG_BREAKPOINT_TYPE_HW)
        {
            DSPLOG_ERROR("dbp_remove_breakpoint_resp(): Breakpoint address doesn't match response");
            b_err_status = false;
            goto dbp_remove_breakpoint_gdb_callback;
        }

        /*Finally remove the breakpoint info from the queue*/
        DBG_core_remove_bp_info(p_pending_bp->u8_bp_wp_num,
                                DBG_BREAKPOINT_TYPE_HW,
                                &p_pid_info->bp_info_queue);

        p_pid_info->p_dbg_pending_bp = NULL;
    }
    else
    {

        DSPLOG_ERROR("dbp_remove_breakpoint_resp(): Target cannot remove breakpoint @ addr 0x%x for p%x.%x",
                      p_pending_bp->u32_addr,
                      p_dbp_response->remove_breakpoint.pid,
                      p_dbp_response->remove_breakpoint.tid);

        b_err_status = false;

        p_pid_info->p_dbg_pending_bp = NULL;
    }

dbp_remove_breakpoint_gdb_callback:
    DBG_core_gdb_reply_z_packet(dbg, b_err_status);
}

static void
DBG_handle_breakpoint_hit(dbp_response_t *p_dbp_response, DBG *dbg)
{
    DBG_core_debug_server *p_dbg_server = dbg->dbg_core.p_dbg_server;

    DSPLOG_INFO("Handle DBP_RESP_BREAPOINT_HIT address - 0x%x", p_dbp_response->breakpoint_hit.address);

    dbg_core_pid_info_t *p_pid_info = DBG_core_find_pid_info(p_dbp_response->breakpoint_hit.pid,
                                        &p_dbg_server->pid_info_queue);

    if(p_pid_info == NULL)
    {
        DSPLOG_ERROR("dbp_breakpoint_hit(): Unknown pid 0x%x", p_dbp_response->breakpoint_hit.pid);
        return;
    }

    /*Get the current known state of the process - before we mark it as frozen because of the breakpoint event*/
    dbg_process_state_t process_state = DBG_core_get_process_state(p_dbp_response->breakpoint_hit.pid,
                                                                        &p_dbg_server->pid_info_queue);
    /*Mark this process as FROZEN - increamenting the freeze count*/
    DBG_core_process_update_state(p_pid_info->pid, &p_dbg_server->pid_info_queue, DBG_PROCESS_STATE_FROZEN_BP);

    /*Check if the breakpoint is hit on the process which is under debug and
     * the process was in an unfrozen state (we may have 2 DEBUG events appearing in the same cycle, so we may
     * have frozen it already)*/
    if(process_state == DBG_PROCESS_STATE_UNFROZEN && p_dbg_server->cur_ptid_info.pid == p_dbp_response->breakpoint_hit.pid)
    {
        /*Switch this thread as a current thread*/
        p_dbg_server->cur_ptid_info.tid = p_dbp_response->breakpoint_hit.tid;

        /*Make cur_ptid point to the thread where the breakpoint is hit*/
        DBG_core_gdb_stop_reply(dbg, p_dbp_response->breakpoint_hit.pid,
                p_dbp_response->breakpoint_hit.tid);
    }
    else
    {
        DSPLOG_INFO("dbp_breakpoint_hit(): Process in a frozen state - enqueue stop-reply for the pid 0x%x",
                        p_dbp_response->breakpoint_hit.pid);

        dbg_core_stop_reason_t *p_stop_reason = DBG_core_stop_reason_alloc(p_dbp_response->breakpoint_hit.pid,
                                                                           p_dbp_response->breakpoint_hit.tid,
                                                                           DBG_STOP_REASON_BPT_HIT,
                                                                           DBG_BREAKPOINT_TYPE_HW,
                                                                           p_dbp_response->breakpoint_hit.address);

        if(p_stop_reason == NULL)
        {
            DSPLOG_ERROR("Unable to allocate stop_reason for pid 0x%x", p_dbp_response->breakpoint_hit.pid);
            return;
        }

        DBG_core_enqueue_stop_reason(p_stop_reason, &p_pid_info->stop_reason_queue);
    }

}

static void
DBG_handle_single_step_complete(dbp_response_t *p_dbp_response, DBG *dbg)
{
    DBG_core_debug_server *p_dbg_server = dbg->dbg_core.p_dbg_server;

    DSPLOG_INFO("Handle DBP_RESP_SINGLE_STEP_COMPLETE address - 0x%x", p_dbp_response->single_step_complete.pc);

    dbg_core_pid_info_t *p_pid_info = DBG_core_find_pid_info(p_dbp_response->single_step_complete.pid,
                                        &p_dbg_server->pid_info_queue);
    if(p_pid_info == NULL)
    {
        DSPLOG_ERROR("dbp_single_step_complete(): Unknown pid 0x%x", p_dbp_response->single_step_complete.pid);
        return;
    }

    /*Get the current known state of the process - before we mark it as frozen because of the breakpoint event*/
    dbg_process_state_t process_state = DBG_core_get_process_state(p_dbp_response->single_step_complete.pid,
                                                                        &p_dbg_server->pid_info_queue);
    /*Mark this process as FROZEN - increamenting the freeze count*/
    DBG_core_process_update_state(p_pid_info->pid, &p_dbg_server->pid_info_queue, DBG_PROCESS_STATE_FROZEN_BP);

    /*Check if the single step complete is on the process which is under debug and
     * the process was in an unfrozen state (we may have 2 DEBUG events appearing in the same cycle, so we may
     * have frozen it already)*/
    if(process_state == DBG_PROCESS_STATE_UNFROZEN && p_dbg_server->cur_ptid_info.pid == p_dbp_response->single_step_complete.pid)
    {
        /*Switch this thread as a current thread*/
        p_dbg_server->cur_ptid_info.tid = p_dbp_response->single_step_complete.tid;

        /*Clear the single step bit*/
        DBG_dbp_thread_clear_single_step(p_dbg_server, p_dbp_response->single_step_complete.pid, p_dbp_response->single_step_complete.tid);


        /*Make cur_ptid point to the thread where the breakpoint is hit*/
        DBG_core_gdb_stop_reply(dbg, p_dbp_response->single_step_complete.pid,
                p_dbp_response->single_step_complete.tid);
    }
    else
    {
        DSPLOG_INFO("dbp_single_step_complete(): Enqueue stop-reply for pid 0x%x", p_dbp_response->single_step_complete.pid);

        dbg_core_stop_reason_t *p_stop_reason = DBG_core_stop_reason_alloc(p_dbp_response->single_step_complete.pid,
                                                                           p_dbp_response->single_step_complete.tid,
                                                                           DBG_STOP_REASON_SSTEP,
                                                                           DBG_BPT_WPT_TYPE_INVALID, 0);

        if(p_stop_reason == NULL)
        {
            DSPLOG_ERROR("Unable to allocate stop_reason for pid 0x%x", p_dbp_response->single_step_complete.pid);
            return;
        }

        DBG_core_enqueue_stop_reason(p_stop_reason, &p_pid_info->stop_reason_queue);
    }
}

static void
DBG_handle_watchpoint_hit(dbp_response_t *p_dbp_response, DBG *dbg)
{
    DBG_core_debug_server *p_dbg_server = dbg->dbg_core.p_dbg_server;

    DSPLOG_INFO("Handle DBP_RESP_WATCHPOINT_HIT address - 0x%x pc - 0x%x", p_dbp_response->watchpoint_hit.address,
                                                                           p_dbp_response->watchpoint_hit.pc);

    dbg_core_pid_info_t *p_pid_info = DBG_core_find_pid_info(p_dbp_response->watchpoint_hit.pid,
                                        &p_dbg_server->pid_info_queue);
    if(p_pid_info == NULL)
    {
        DSPLOG_ERROR("dbp_watchpoint_hit(): Unknown pid 0x%x", p_dbp_response->watchpoint_hit.pid);
        return;
    }

    /*Get the current known state of the process - before we mark it as frozen because of the breakpoint event*/
    dbg_process_state_t process_state = DBG_core_get_process_state(p_dbp_response->watchpoint_hit.pid,
                                                                   &p_dbg_server->pid_info_queue);

    /*Mark this process as FROZEN - increamenting the freeze count*/
    DBG_core_process_update_state(p_pid_info->pid, &p_dbg_server->pid_info_queue, DBG_PROCESS_STATE_FROZEN_BP);

    dbg_core_bp_info_t *p_bp_info = DBG_core_find_bp_info_from_addr_1(p_dbp_response->watchpoint_hit.address,
                                                                      DBG_WATCHPOINT_TYPE_ACCESS,
                                                                      p_dbp_response->watchpoint_hit.tid,
                                                                      p_pid_info);

    /*Check if the single step complete is on the process which is under debug and
     * the process was in an unfrozen state (we may have 2 DEBUG events appearing in the same cycle, so we may
     * have frozen it already)*/
    if(process_state == DBG_PROCESS_STATE_UNFROZEN && p_dbg_server->cur_ptid_info.pid == p_dbp_response->watchpoint_hit.pid)
    {
        /*Switch this thread as a current thread*/
        p_dbg_server->cur_ptid_info.tid = p_dbp_response->watchpoint_hit.tid;

        /*Make cur_ptid point to the thread where the breakpoint is hit*/
        DBG_core_gdb_watchpoint_hit(dbg, p_dbp_response->watchpoint_hit.pid,
            p_dbp_response->watchpoint_hit.tid,
            p_dbp_response->watchpoint_hit.address,
            (p_bp_info != NULL)? p_bp_info->bpt_type: DBG_WATCHPOINT_TYPE_ACCESS);
    }
    else
    {
        DSPLOG_INFO("dbp_watchpoint_hit(): Enqueue stop-reply for pid 0x%x", p_dbp_response->watchpoint_hit.pid);

        dbg_core_stop_reason_t *p_stop_reason = DBG_core_stop_reason_alloc(p_dbp_response->watchpoint_hit.pid,
                                                                           p_dbp_response->watchpoint_hit.tid,
                                                                           DBG_STOP_REASON_WPT_HIT,
                                                                           (p_bp_info != NULL)? p_bp_info->bpt_type: DBG_WATCHPOINT_TYPE_ACCESS,
                                                                           p_dbp_response->watchpoint_hit.address);

        if(p_stop_reason == NULL)
        {
            DSPLOG_ERROR("Unable to allocate stop_reason for pid 0x%x", p_dbp_response->watchpoint_hit.pid);
            return;
        }

        DBG_core_enqueue_stop_reason(p_stop_reason, &p_pid_info->stop_reason_queue);
    }
}

static void
DBG_handle_set_watchpoint_resp(dbp_response_t *p_dbp_response, DBG *dbg)
{
    DBG_core_debug_server *p_dbg_server = dbg->dbg_core.p_dbg_server;
    DSPLOG_DEBUG("Handle DBP_CMD_SET_WATCHPOINT response - 0x%x", p_dbp_response->response);

    bool b_err_status = true;

    /*Get the pid_info associated with the set watchpoint response pid*/
    dbg_core_pid_info_t *p_pid_info = DBG_core_find_pid_info(p_dbp_response->set_watchpoint.pid,
            &p_dbg_server->pid_info_queue);

    if(p_pid_info == NULL)
    {
        DSPLOG_ERROR("dbp_set_watchpoint_resp(): pid - not found - should update pid_list");
        b_err_status = false;
        goto dbp_set_watchpoint_gdb_callback;
    }

    if(p_dbp_response->response & DBP_PACK)
    {
        DSPLOG_DEBUG("DBP_CMD_SET_WATCHPOINT pack");

        /*Check if the pending watchpoint we hold is the actual watchpoint being set*/
        if(p_pid_info->p_dbg_pending_bp == NULL)
        {
            DSPLOG_INFO("dbp_set_watchpoint_resp(): Create a bp_info for p%x.%x at addr 0x%x",
                    p_dbp_response->set_watchpoint.pid,
                    p_dbp_response->set_watchpoint.tid,
                    p_dbp_response->set_watchpoint.address);

            p_pid_info->p_dbg_pending_bp = DBG_core_bp_info_alloc(p_pid_info->pid,
                    p_dbp_response->set_watchpoint.tid,
                    p_dbp_response->set_watchpoint.wp,
                    ((p_dbp_response->set_watchpoint.type == DWP_WPT_LOAD) ? DBG_WATCHPOINT_TYPE_READ :
                     ((p_dbp_response->set_watchpoint.type == DBP_WPT_STORE) ? DBG_WATCHPOINT_TYPE_WRITE: DBG_WATCHPOINT_TYPE_ACCESS)),
                    p_dbp_response->set_watchpoint.address,
                    0);

            if(p_pid_info->p_dbg_pending_bp == NULL)
            {
                DSPLOG_ERROR("dbp_set_watchpoint_resp(): Unable to allocate watchpoint info for bp 0x%x",
                        p_dbp_response->set_watchpoint.wp);
                b_err_status = false;
                goto dbp_set_watchpoint_gdb_callback;
            }
        }
        dbg_core_bp_info_t *p_pending_bp = p_pid_info->p_dbg_pending_bp;

        if(p_pending_bp->u32_addr != p_dbp_response->set_watchpoint.address)
        {
            DSPLOG_ERROR("dbp_set_watchpoint(): watchpoint address doesn't match in response");
            b_err_status = false;
            goto dbp_set_watchpoint_gdb_callback;
        }

        DSPLOG_INFO("dbp_set_watchpoint_resp(): setting bp 0x%x @addr 0x%x for p%x.%x",
                p_dbp_response->set_watchpoint.wp,
                p_dbp_response->set_watchpoint.address,
                p_dbp_response->set_watchpoint.pid,
                p_dbp_response->set_watchpoint.tid);

        /*Finally, Set the watchpoint number to pending_bp*/
        DBG_core_set_bp_num(p_dbp_response->set_watchpoint.wp, p_pending_bp);

        /*Enqueue the watchpoint info to the watchpoint queue*/
        DBG_core_enqueue_bp_info(p_pending_bp, &p_pid_info->bp_info_queue);

        p_pid_info->p_dbg_pending_bp = NULL;
    }
    else
    {
        DSPLOG_ERROR("dbp_set_watchpoint_resp(): Target cannot set watchpoint at addr 0x%x for p%x.%x",
                p_dbp_response->set_watchpoint.address, p_dbp_response->set_watchpoint.pid,
                p_dbp_response->set_watchpoint.tid);

        b_err_status = false;

        p_pid_info->p_dbg_pending_bp = NULL;
    }

dbp_set_watchpoint_gdb_callback:
    DBG_core_gdb_reply_z_packet(dbg, b_err_status);
}

static void
DBG_handle_remove_watchpoint_resp(dbp_response_t *p_dbp_response __unused, DBG *dbg __unused)
{
    DSPLOG_DEBUG("Handle DBP_CMD_REMOVE_WATCHPOINT response - 0x%x", p_dbp_response->response);
    DBG_core_debug_server *p_dbg_server = dbg->dbg_core.p_dbg_server;
    bool b_err_status = true;

    dbg_core_pid_info_t *p_pid_info = DBG_core_find_pid_info(p_dbp_response->remove_watchpoint.pid,
            &p_dbg_server->pid_info_queue);

    if(p_pid_info == NULL)
    {
        DSPLOG_ERROR("dbp_remove_watchpoint_resp(): Unknown process in the response packet");
        b_err_status = false;
        goto dbp_remove_watchpoint_gdb_callback;
    }

    if(p_pid_info->p_dbg_pending_bp == NULL)
    {
        DSPLOG_ERROR("dbp_remove_watchpoint_resp(): Not expecting a remove watchpoint resp for the pid 0x%x",
                p_pid_info->pid);

        b_err_status = false;
        goto dbp_remove_watchpoint_gdb_callback;
    }

    /*Check if the pending bp info and the response matches*/
    dbg_core_bp_info_t *p_pending_bp = p_pid_info->p_dbg_pending_bp;

    if(p_dbp_response->response & DBP_PACK)
    {
        DSPLOG_INFO("DBP_CMD_REMOVE_WATCHPOINT PACK");

        /*Finally remove the watchpoint info from the queue*/
        DBG_core_remove_bp_info(p_pending_bp->u8_bp_wp_num,
                                p_pending_bp->bpt_type,
                                &p_pid_info->bp_info_queue);

        p_pid_info->p_dbg_pending_bp = NULL;
    }
    else
    {

        DSPLOG_ERROR("dbp_remove_watchpoint_resp(): Target cannot remove watchpoint @ addr 0x%x for p%x.%x",
                      p_pending_bp->u32_addr,
                      p_dbp_response->remove_watchpoint.pid,
                      p_dbp_response->remove_watchpoint.tid);

        b_err_status = false;

        p_pid_info->p_dbg_pending_bp = NULL;
    }

dbp_remove_watchpoint_gdb_callback:
    DBG_core_gdb_reply_z_packet(dbg, b_err_status);

}

static void
DBG_handle_resp_process_exited(dbp_response_t *p_dbp_response, DBG *dbg)
{
    DSPLOG_DEBUG("Handle DBP_RESP_PROCESS_EXITED response 0x%x", p_dbp_response->response);

    DBG_core_debug_server *p_dbg_server = dbg->dbg_core.p_dbg_server;

    DSPLOG_INFO("resp_process_exited: Process %d exited", p_dbp_response->process_exited.pid);

    dbg_core_pid_info_t *p_pid_info = DBG_core_find_pid_info(p_dbp_response->process_exited.pid,
                                                             &p_dbg_server->pid_info_queue);

    if(p_pid_info == NULL)
    {
        DSPLOG_ERROR("resp_process_exited: Unknown pid %d - nothing to do", p_dbp_response->process_exited.pid);
    }
    else
    {
        /* If the exited process isn't terminated by GDB, then inform GDB about the exiting
         * process.*/
        if(p_pid_info->u8_kill_pending == 0)
        {
            DBG_core_gdb_process_exited(dbg, p_dbp_response->process_exited.pid);

            /*If the current process under debug has exited, then mark the cur_ptid_info
             * as invalid ptid*/
            if(p_dbg_server->cur_ptid_info.pid == p_dbp_response->process_exited.pid)
            {
                p_dbg_server->cur_ptid_info.pid = gdb_invalid_ptid.pid;
                p_dbg_server->cur_ptid_info.tid = gdb_invalid_ptid.tid;
            }
        }

        /*Finally, Remove pid from the pid_info_queue*/
        DBG_core_remove_pid_info(p_dbp_response->process_exited.pid, &p_dbg_server->pid_info_queue);

    }
}

static void
DBG_handle_resp_thread_created(dbp_response_t *p_dbp_response, DBG *dbg)
{
    DSPLOG_DEBUG("Handle DBP_RESP_THREAD_CREATED response");

    DBG_core_debug_server *p_dbg_server = dbg->dbg_core.p_dbg_server;

    dbg_core_pid_info_t *p_pid_info = DBG_core_find_pid_info(p_dbp_response->thread_created.pid,
                                        &p_dbg_server->pid_info_queue);

    if(p_pid_info == NULL)
    {
        DSPLOG_ERROR("Unknown pid - we haven't registered p%x to receive thread notifications",
                p_dbp_response->thread_created.pid);
        return;
    }

    dbg_core_thread_info_t *p_thread_info = DBG_core_tid_info_alloc(p_dbp_response->thread_created.tid,
                                                DBG_THREAD_STATE_CREATED);

    if(p_thread_info == NULL)
    {
        DSPLOG_ERROR("Cannot allocate thread info for p%x.%x", p_dbp_response->thread_created.pid,
                        p_dbp_response->thread_created.tid);
        return;
    }

    DBG_core_enqueue_tid_info(p_thread_info, &p_pid_info->thread_info_queue);

    DSPLOG_INFO("Thread %x created for p%x", p_dbp_response->thread_created.tid,
            p_dbp_response->thread_created.pid);

    if(p_dbg_server->b_notify_thread_events)
    {
        DSPLOG_INFO("Notify thread event to gdb");

        DBG_core_process_update_state(p_pid_info->pid, &p_dbg_server->pid_info_queue,
                                        DBG_PROCESS_STATE_FROZEN_BP);

        if(p_dbg_server->cur_ptid_info.pid == p_dbp_response->thread_created.pid)
            DBG_core_gdb_thread_created(dbg, p_dbp_response->thread_created.pid, p_dbp_response->thread_created.tid);
        else
        {

            dbg_core_stop_reason_t *p_stop_reason = DBG_core_stop_reason_alloc(p_dbp_response->thread_created.pid,
                                                                               p_dbp_response->thread_created.tid,
                                                                               DBG_STOP_REASON_THREAD_CREATED,
                                                                               DBG_BPT_WPT_TYPE_INVALID,
                                                                               0);

            if(p_stop_reason == NULL)
            {
                DSPLOG_ERROR("Unable to allocate stop_reason for pid 0x%x", p_dbp_response->thread_created.pid);
                return;
            }

            DBG_core_enqueue_stop_reason(p_stop_reason, &p_pid_info->stop_reason_queue);
        }
    }
    else
    {
        /*thread notification not requested by gdb - unfreeze the process*/
        DBG_dbp_process_unfreeze(p_dbg_server, p_dbp_response->thread_created.pid);
    }
}

static void
DBG_handle_resp_thread_exited(dbp_response_t *p_dbp_response, DBG *dbg)
{
    DSPLOG_DEBUG("Handle DBP_RESP_THREAD_EXITED response");

    DBG_core_debug_server *p_dbg_server = dbg->dbg_core.p_dbg_server;

    dbg_core_pid_info_t *p_pid_info = DBG_core_find_pid_info(p_dbp_response->thread_exited.pid,
                                            &p_dbg_server->pid_info_queue);

    if(p_pid_info == NULL)
    {
        DSPLOG_ERROR("Unknown pid - we haven't registered p%x to receive thread notifications",
                        p_dbp_response->thread_exited.pid);
        return;
    }

    DBG_core_remove_tid_info(p_dbp_response->thread_exited.tid, &p_pid_info->thread_info_queue);

    DSPLOG_INFO("Thread %x exited for p%x", p_dbp_response->thread_exited.tid,
                        p_dbp_response->thread_exited.tid);

    if(p_dbg_server->b_notify_thread_events)
    {
        DSPLOG_INFO("Notify thread event to gdb");

        DBG_core_process_update_state(p_pid_info->pid, &p_dbg_server->pid_info_queue,
                                      DBG_PROCESS_STATE_FROZEN_BP);

        /*Notify the event to gdb if
         * 1. the process is the process under debug
         * 2. thread exited is not part of vKill request by gdb
         * */
        if(p_dbg_server->cur_ptid_info.pid == p_dbp_response->thread_exited.pid && p_pid_info->u8_kill_pending == 0)
            DBG_core_gdb_thread_exited(dbg, p_dbp_response->thread_exited.pid, p_dbp_response->thread_exited.tid);
        else
        {
            dbg_core_stop_reason_t *p_stop_reason = DBG_core_stop_reason_alloc(p_dbp_response->thread_exited.pid,
                                                                               p_dbp_response->thread_exited.tid,
                                                                               DBG_STOP_REASON_THREAD_EXITED,
                                                                               DBG_BPT_WPT_TYPE_INVALID, 0);

            if(p_stop_reason == NULL)
            {
                DSPLOG_ERROR("Unable to allocate stop_reason for pid 0x%x", p_dbp_response->thread_exited.pid);
                return;
            }

            DBG_core_enqueue_stop_reason(p_stop_reason, &p_pid_info->stop_reason_queue);
        }
    }
    else
    {
        if(p_pid_info->u8_kill_pending == 0)
            /*thread notification not requested by gdb - unfreeze the process*/
            DBG_dbp_process_unfreeze(p_dbg_server, p_dbp_response->thread_created.pid);
    }
}

static void
DBG_handle_resp_library_loaded(dbp_response_t *p_dbp_response, DBG *dbg)
{
    DSPLOG_DEBUG("Handle DBP_RESP_LIBRARY_LOADED response");

    DBG_core_debug_server *p_dbg_server = dbg->dbg_core.p_dbg_server;

    dbg_core_pid_info_t *p_pid_info = DBG_core_find_pid_info(p_dbp_response->library_loaded.pid,
                                            &p_dbg_server->pid_info_queue);

    if(p_pid_info == NULL)
    {
        DSPLOG_ERROR("Unknown pid - we haven't registered p%x to receive library notifications",
                            p_dbp_response->library_loaded.pid);
        return;
    }

    /*Mark the process as frozen*/
    DBG_core_process_update_state(p_dbp_response->library_loaded.pid, &p_dbg_server->pid_info_queue,
                                    DBG_PROCESS_STATE_FROZEN_BP);

    /*Inform GDB that there are changes to the */
    if(p_dbp_response->library_loaded.pid == p_dbg_server->cur_ptid_info.pid)
    {
        DSPLOG_INFO("notify gdb about library event for p%x", p_dbp_response->library_loaded.pid);
        DBG_core_gdb_notify_library_change(dbg);
    }
    else
    {
        dbg_core_stop_reason_t *p_stop_reason = DBG_core_stop_reason_alloc(p_dbp_response->library_loaded.pid,
                                                                           p_dbp_response->library_loaded.tid,
                                                                           DBG_STOP_REASON_LIBRARY_LOADED,
                                                                           DBG_BPT_WPT_TYPE_INVALID, 0);

        if(p_stop_reason == NULL)
        {
            DSPLOG_ERROR("Unable to allocate stop_reason for pid 0x%x", p_dbp_response->library_loaded.pid);
            return;
        }

        DBG_core_enqueue_stop_reason(p_stop_reason, &p_pid_info->stop_reason_queue);
    }
}

static void
DBG_handle_resp_library_unloaded(dbp_response_t *p_dbp_response, DBG *dbg)
{
    DSPLOG_DEBUG("Handle DBP_RESP_LIBRARY_UNLOADED response");

    DBG_core_debug_server *p_dbg_server = dbg->dbg_core.p_dbg_server;

    dbg_core_pid_info_t *p_pid_info = DBG_core_find_pid_info(p_dbp_response->library_unloaded.pid,
                                            &p_dbg_server->pid_info_queue);

    if(p_pid_info == NULL)
    {
        DSPLOG_ERROR("Unknown pid - we haven't registered p%x to receive library notifications",
                            p_dbp_response->library_unloaded.pid);
        return;
    }

    /*Mark the process as frozen*/
    DBG_core_process_update_state(p_dbp_response->library_unloaded.pid, &p_dbg_server->pid_info_queue,
                                    DBG_PROCESS_STATE_FROZEN_BP);

    /*Inform GDB that there are changes to the */
    if(p_dbp_response->library_unloaded.pid == p_dbg_server->cur_ptid_info.pid)
    {
        DSPLOG_INFO("notify gdb about library event for p%x", p_dbp_response->library_unloaded.pid);
        DBG_core_gdb_notify_library_change(dbg);
    }
    else
    {
        dbg_core_stop_reason_t *p_stop_reason = DBG_core_stop_reason_alloc(p_dbp_response->library_unloaded.pid,
                                                                           p_dbp_response->library_unloaded.tid,
                                                                           DBG_STOP_REASON_LIBRARY_UNLOADED,
                                                                           DBG_BPT_WPT_TYPE_INVALID, 0);

        if(p_stop_reason == NULL)
        {
            DSPLOG_ERROR("Unable to allocate stop_reason for pid 0x%x", p_dbp_response->library_unloaded.pid);
            return;
        }

        DBG_core_enqueue_stop_reason(p_stop_reason, &p_pid_info->stop_reason_queue);
    }
}

static void
DBG_handle_resp_fatal(dbp_response_t *p_dbp_response, DBG *dbg)
{
    DSPLOG_DEBUG("Handle DBP_RESP_FATAL response");
    DBG_core_debug_server *p_dbg_server = dbg->dbg_core.p_dbg_server;

    dbg_core_pid_info_t *p_pid_info = DBG_core_find_pid_info(p_dbp_response->fatal.pid, &p_dbg_server->pid_info_queue);

    if(p_pid_info == NULL)
    {
        DSPLOG_ERROR("Unknown pid 0x%x - FATAL response", p_dbp_response->fatal.pid);
        return;
    }

    DSPLOG_INFO("Process terminate 0x%x - reason %d @ PC 0x%x address 0x%x",
            p_dbp_response->fatal.pid, p_dbp_response->fatal.type, p_dbp_response->fatal.pc, p_dbp_response->fatal.address);

    DBG_core_gdb_notify_process_terminate(dbg, p_dbp_response->fatal.pid, p_dbp_response->fatal.type);

    /*Remove pid from the pid_info_queue*/
    DBG_core_remove_pid_info(p_dbp_response->fatal.pid, &p_dbg_server->pid_info_queue);
}

static void
DBG_handle_thread_set_single_step_resp(dbp_response_t *p_dbp_response __unused, DBG *dbg __unused){}

static void
DBG_handle_thread_clear_single_step_resp(dbp_response_t *p_dbp_response __unused, DBG *dbg __unused){}
