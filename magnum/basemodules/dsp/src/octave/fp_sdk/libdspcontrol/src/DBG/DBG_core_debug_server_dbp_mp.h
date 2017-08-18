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

#ifndef _DBG_CORE_DEBUG_SERVER_DBP_MP_H_
#define _DBG_CORE_DEBUG_SERVER_DBP_MP_H_

#include <stdint.h>

#include "libdspcontrol/CHIP.h"
#include "libdspcontrol/DSP.h"
#include "libdspcontrol/DBG.h"

#include "libthreadxp/user/dp/dbp.h"

#include "DBG_core_debug_server_mp.h"

/*Function declarations*/
DBG_DBP_ERR_CODE
DBG_core_dbp_handle_resp(DBG *dbg);

/*DBP_CMD_GET_DBP_CONFIG*/
DBG_DBP_ERR_CODE
DBG_dbp_cmd_dbp_config(DBG_core_debug_server *p_dbg_server);

/*DBP_CMD_GET_PID_LIST*/
DBG_DBP_ERR_CODE
DBG_dbp_get_pid_list(DBG_core_debug_server *p_dbg_server);

/*DBP_CMD_GET_PROCESS_NAME*/
DBG_DBP_ERR_CODE
DBG_dbp_process_get_name(DBG_core_debug_server *p_dbg_server, dbg_pid_t pid);

/*DBP_CMD_PROCESS_GET_TID_LIST*/
DBG_DBP_ERR_CODE
DBG_dbp_process_get_tid_list(DBG_core_debug_server *p_dbg_server, dbg_pid_t pid );

/*DBP_CMD_PROCESS_GET_STATE*/
DBG_DBP_ERR_CODE
DBG_dbp_process_get_state(DBG_core_debug_server *p_dbg_server, dbg_pid_t pid );

/*DBP_CMD_PROCESS_FREEZE*/
DBG_DBP_ERR_CODE
DBG_dbp_process_freeze(DBG_core_debug_server *p_dbg_server, dbg_pid_t pid );

/*DBP_CMD_PROCESS_UNFREEZE*/
DBG_DBP_ERR_CODE
DBG_dbp_process_unfreeze(DBG_core_debug_server *p_dbg_server, dbg_pid_t pid );

/*DBP_CMD_PROCESS_ATTACH*/
DBG_DBP_ERR_CODE
DBG_dbp_process_attach(DBG_core_debug_server *p_dbg_server, dbg_pid_t pid );

/*DBP_CMD_PROCESS_DETACH*/
DBG_DBP_ERR_CODE
DBG_dbp_process_detach(DBG_core_debug_server *p_dbg_server, dbg_pid_t pid );

/*DBP_CMD_THREAD_FREEZE*/
DBG_DBP_ERR_CODE
DBG_dbp_thread_freeze(DBG_core_debug_server *p_dbg_server, dbg_pid_t pid, dbg_tid_t tid );

/*DBP_CMD_THREAD_UNFREEZE*/
DBG_DBP_ERR_CODE
DBG_dbp_thread_unfreeze(DBG_core_debug_server *p_dbg_server, dbg_pid_t pid, dbg_tid_t tid );

/*DBP_CMD_THREAD_GET_STATE*/
DBG_DBP_ERR_CODE
DBG_dbp_thread_get_state(DBG_core_debug_server *p_dbg_server, dbg_pid_t pid, dbg_tid_t tid );

/*DBP_CMD_PROCESS_READ_MEMORY*/
DBG_DBP_ERR_CODE
DBG_dbp_process_read_memory(DBG_core_debug_server *p_dbg_server, dbg_pid_t pid,
                            uint32_t u32_address, uint16_t u16_size);

/*DBP_CMD_PROCESS_WRITE_MEMORY*/
DBG_DBP_ERR_CODE
DBG_dbp_process_write_memory(DBG_core_debug_server *p_dbg_server,
                             dbg_pid_t pid,
                            uint32_t u32_address,
                            uint16_t u16_size,
                            uint8_t *p_u8_buf);

DBG_DBP_ERR_CODE
DBG_dbp_process_create(DBG_core_debug_server *p_dbg_server,
                       int argc,
                       uint16_t buffer_len,
                       char *argv);

/*DBP_CMD_PROCESS_TERMINATE*/
DBG_DBP_ERR_CODE
DBG_dbp_process_terminate(DBG_core_debug_server *p_dbg_server, dbg_pid_t pid);

/*DBP_CMD_THREAD_READ_REGISTER*/
DBG_DBP_ERR_CODE
DBG_dbp_thread_read_register(DBG_core_debug_server *p_dbg_server, dbg_pid_t pid, dbg_tid_t tid, uint32_t reg_offset, uint32_t reg_size);

/*DBP_CMD_THREAD_WRITE_REGISTER*/
DBG_DBP_ERR_CODE
DBG_dbp_thread_write_register(DBG_core_debug_server *p_dbg_server,
                              dbg_pid_t pid, dbg_tid_t tid, uint16_t u16_regnum,
                              uint64_t u64_value);

/*DBP_CMD_THREAD_READ_ALL_REGISTERS*/
DBG_DBP_ERR_CODE
DBG_dbp_thread_read_all_registers(DBG_core_debug_server *p_dbg_server, dbg_pid_t pid, dbg_tid_t tid);

/*DBP_CMD_THREAD_SET_BREAKPOINT*/
DBG_DBP_ERR_CODE
DBG_dbp_set_breakpoint(DBG_core_debug_server *p_dbg_server, dbg_pid_t pid, dbg_tid_t tid, uint32_t u32_pc);

/*DBP_CMD_THREAD_REMOVE_BREAKPOINT*/
DBG_DBP_ERR_CODE
DBG_dbp_remove_breakpoint(DBG_core_debug_server *p_dbg_server, dbg_pid_t pid, dbg_tid_t tid, uint8_t u8_bp_num);

/*DBP_CMD_THREAD_SET_WATCHPOINT*/
DBG_DBP_ERR_CODE
DBG_dbp_set_watchpoint(DBG_core_debug_server *p_dbg_server,
                              dbg_pid_t pid, dbg_tid_t tid,
                              uint32_t address,
                              uint32_t length,
                              DBP_WPT_TYPE_ID type);

/*DBP_CMD_THREAD_REMOVE_WATCHPOINT*/
DBG_DBP_ERR_CODE
DBG_dbp_remove_watchpoint(DBG_core_debug_server *p_dbg_server,
                                 dbg_pid_t pid, dbg_tid_t tid, uint8_t wp);

/*DBP_CMD_THREAD_SET_SINGLE_STEP*/
DBG_DBP_ERR_CODE
DBG_dbp_thread_set_single_step(DBG_core_debug_server *p_dbg_server,
                               dbg_pid_t pid, dbg_tid_t tid);

/*DBP_CMD_THREAD_CLEAR_SINGLE_STEP*/
DBG_DBP_ERR_CODE
DBG_dbp_thread_clear_single_step(DBG_core_debug_server *p_dbg_server,
                                 dbg_pid_t pid, dbg_tid_t tid);

/*DBP_CMD_PROCESS_GET_DSO_COUNT*/
DBG_DBP_ERR_CODE
DBG_dbp_process_get_dso_count(DBG_core_debug_server *p_dbg_server, dbg_pid_t pid);

/*DBP_CMD_PROCESS_GET_DSO_SONAMES*/
DBG_DBP_ERR_CODE
DBG_dbp_process_get_dso_sonames(DBG_core_debug_server *p_dbg_server, dbg_pid_t pid,
                                uint32_t start);

/*DBP_CMD_PROCESS_GET_DSO_SEGMENTS*/
DBG_DBP_ERR_CODE
DBG_dbp_process_get_dso_segments(DBG_core_debug_server *p_dbg_server, dbg_pid_t pid,
                                 uint32_t index);

/*DBP_CMD_PROCESS_GET_DSO_SBA*/
DBG_DBP_ERR_CODE
DBG_dbp_process_get_dso_sba(DBG_core_debug_server *p_dbg_server, dbg_pid_t pid,
                                 uint32_t index);

/*DBP_CMD_PROCESS_GET_DSO_INFO*/
DBG_DBP_ERR_CODE
DBG_dbp_process_get_dso_info(DBG_core_debug_server *p_dbg_server, dbg_pid_t pid);

/*DBG_CMD_PROCESS_GET_EXE_SEGMENTS*/
DBG_DBP_ERR_CODE
DBG_dbp_process_get_exe_segments(DBG_core_debug_server *p_dbg_server, dbg_pid_t pid);

/*DBG_CMD_PROCESS_GET_EXE_SBA*/
DBG_DBP_ERR_CODE
DBG_dbp_process_get_exe_sba(DBG_core_debug_server *p_dbg_server, dbg_pid_t pid);

#endif
