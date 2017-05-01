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
#ifndef _DBG_CORE_DEBUG_SERVER_GDB_MP_H_
#define _DBG_CORE_DEBUG_SERVER_GDB_MP_H_

#include <stdint.h>

#include "libdspcontrol/CHIP.h"
#include "libdspcontrol/DSP.h"
#include "libdspcontrol/DBG.h"

#include "DBG_core_debug_server_mp.h"
#include "DBG_core_debug_server_common.h"
#include "DBG_core_debug_server_gdb_regaccess.h"

/*Public function declarations*/
void
DBG_core_gdb_handle_continue(DBG *dbg);

void
DBG_core_gdb_handle_stop(DBG  *dbg);

void
DBG_core_gdb_handle_detach(DBG *dbg);

void
DBG_core_gdb_handle_singlestep(DBG *dbg);

void
DBG_core_gdb_handle_send_status_update(DBG *dbg);

void
DBG_core_handle_status_response(DBG *dbg, bool b_err_status __unused);

void
DBG_core_gdb_handle_query_packet(DBG *dbg);

void
DBG_core_gdb_handle_qSymbol(DBG *dbg);

void
DBG_core_gdb_handle_qOffsets_resp(DBG *dbg,
                                  uint32_t num_segments,
                                  uint32_t *seg_info, bool b_err_status);

void
DBG_core_gdb_handle_read_all_registers(DBG *dbg);

void
DBG_core_gdb_thread_read_all_registers_resp(DBG *dbg,
                uint8_t *p_reg_context,
                uint8_t *p_u8_pred,
                bool b_err_status);

void
DBG_core_gdb_handle_read_memory(DBG *dbg);

void
DBG_core_gdb_process_read_memory_resp(DBG *dbg,
                                      uint32_t u32_address,
                                      uint16_t u16_size,
                                      uint8_t *p_u8_data,
                                      bool b_err_status);

void
DBG_core_gdb_handle_write_memory(DBG *dbg);

void
DBG_core_gdb_process_write_memory_resp(DBG *dbg,
                                       uint16_t u16_size,
                                       bool b_err_status);

#if 0
void
DBG_core_gdb_handle_write_memory_binary(DBG *dbg);
#endif

void
DBG_core_gdb_handle_read_register(DBG *dbg);

void
DBG_core_gdb_thread_read_register_resp(DBG *dbg, uint64_t u64_value, bool b_err_status);

void
DBG_core_gdb_handle_write_register(DBG *dbg);

void
DBG_core_gdb_thread_write_register_resp(DBG *dbg, bool b_err_status);

void
DBG_core_gdb_handle_h_packet(DBG *dbg);

void
DBG_core_gdb_handle_T_packet(DBG *dbg);

void
DBG_core_gdb_handle_v_packet(DBG *dbg);

void
DBG_core_gdb_process_create_resp(DBG *dbg, dbg_pid_t pid, bool b_err_status);

void
DBG_core_gdb_process_create_resp_1(DBG *dbg, bool b_err_status);

void
DBG_core_gdb_vKill_resp(DBG *dbg);

void
DBG_core_gdb_handle_z_packet(DBG *dbg, char b_cmd_type);

void
DBG_core_gdb_reply_z_packet(DBG *dbg, bool b_err_status);

void
DBG_core_gdb_breakpoint_hit(DBG *dbg, dbg_pid_t pid, dbg_tid_t tid, uint32_t u32_address);

void
DBG_core_gdb_watchpoint_hit(DBG *dbg,
                            dbg_pid_t pid,
                            dbg_tid_t tid,
                            uint32_t u32_address,
                            DBG_BPT_WPT_TYPE wp_type);

void
DBG_core_gdb_single_step_complete(DBG *dbg, dbg_pid_t pid, dbg_tid_t tid);

void
DBG_core_gdb_process_exited(DBG *dbg, dbg_pid_t pid);

void
DBG_core_gdb_thread_created(DBG *dbg, dbg_pid_t pid __unused, dbg_tid_t tid __unused);

void
DBG_core_gdb_notify_library_change(DBG *dbg);

void
DBG_core_gdb_notify_process_terminate(DBG *dbg, dbg_pid_t pid, DBP_FAULT_ID fault);

void
DBG_core_gdb_thread_exited(DBG *dbg, dbg_pid_t pid __unused, dbg_tid_t tid);

#endif
