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

/* Implementation of the 'debug_server_mp' variant of the DBG core */
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libdspcontrol/CHIP.h"
#include "libdspcontrol/DBG.h"
#include "libdspcontrol/COMMON.h"
#include "libdspcontrol/DSPLOG.h"

#include "libosbase/irq_context_save.h"

#include "fp_sdk_config.h"

#include "DBG_interfaces.h"
#include "DBG_core_debug_server_gdb_mp.h"
#include "DBG_core_debug_server_dbp_mp.h"
#include "DBG_core_debug_server_gdb_utils.h"

#if !(FEATURE_IS(DBG_CORE, DEBUG_SERVER_MP))
#  error "This source is for the debug_server variant of the DBG core"
#endif
#if !FEATURE_IS(LIBC, HOSTED)
#  error "This compilation unit requires support for dynamic memory allocation"
#endif

gdb_ptid_t gdb_null_ptid = {0, 0};
gdb_ptid_t gdb_minus_one_ptid = {-1, 0};
gdb_ptid_t gdb_invalid_ptid = {-1, -1};

#define IS_NULL_PTID(ptid)      (ptid.pid == gdb_null_ptid.pid && ptid.tid == gdb_null_ptid.tid)
#define IS_MINUS_ONE_PTID(ptid) (ptid.pid == gdb_minus_one_ptid.pid && ptid.tid == gdb_minus_one_ptid.tid)
#define IS_INVALID_PTID(ptid)   (ptid.pid == gdb_invalid_ptid.pid && ptid.tid == gdb_invalid_ptid.tid)

#define DEFAULT_FILE_NAME   "bin/helloworld.fpexe"
#define DEFAULT_FILENAME_SIZE   strlen(DEFAULT_FILE_NAME)

/* Index in the segment information packet which points to the data segment
 * This is just an indicator - we however derive the accurate segment index
 * depending on the SBA value*/
#define DATA_SEG_INDEX  3

static void
copy_ptid(gdb_ptid_t *dest, gdb_ptid_t *src);

static void
DBG_core_gdb_init(DBG *dbg);

static void
DBG_core_gdb_handle_qthread_info(DBG *dbg);

static void
DBG_core_gdb_handle_qAttached(DBG *dbg);

static bool
DBG_core_must_be_running(DBG *dbg, char *p_ch_pack_name, char *p_ch_gdb_err_code);

static void
DBG_core_write_ptid(dbg_pid_t pid, dbg_tid_t tid, char *p_ch_buf);

static void
DBG_core_gdb_handle_qOffsets(DBG *dbg);

static void
DBG_core_gdb_handle_qXfer_libaries(DBG *dbg);

static void
DBG_core_gdb_handle_qXfer_osdata(DBG *dbg);

static void
DBG_core_gdb_handle_QThread_Events(DBG *dbg);

static void
DBG_core_gdb_qxfer_libraries_resp(DBG *dbg, bool b_err_status);

static void
DBG_core_gdb_handle_insert_break_watch_mp(DBG *dbg, bool b_err_status);

static void
DBG_core_gdb_handle_remove_break_watch_mp(DBG *dbg, bool b_err_status);

static void
DBG_core_gdb_handle_vCont(DBG *dbg);

static void
DBG_core_gdb_handle_vAttach(DBG *dbg);

static void
DBG_core_gdb_handle_vRun(DBG *dbg);

static void
DBG_core_gdb_handle_vKill(DBG *dbg);

static void
DBG_core_gdb_handle_h_int(DBG *dbg, bool b_err_status);

static void
DBG_core_gdb_handle_h_response(DBG *dbg, bool b_err_status);

static void
DBG_core_gdb_handle_stop_response(DBG *dbg, bool b_err_status);

/*Return the pid_info for the current process*/
static dbg_core_pid_info_t *
DBG_core_get_current_process(DBG_core_debug_server *p_dbg_server);

static bool
DBG_core_must_be_running(DBG *dbg, char *p_ch_pack_name, char *p_ch_gdb_err_code)
{
    DBG_core_debug_server *p_dbg_server = dbg->dbg_core.p_dbg_server;

    if(p_dbg_server->core_state < DBG_CORE_STATE_DP_STARTED)
    {
        DSPLOG_ERROR("Cannot handle %s packet without core starting Debug Process", p_ch_pack_name);
        DBG_core_gdb_send_reply(dbg, p_ch_gdb_err_code, true);
        return false;
    }

    return true;
}

static dbg_core_pid_info_t *
DBG_core_get_current_process(DBG_core_debug_server *p_dbg_server)
{
    return DBG_core_find_pid_info(p_dbg_server->cur_ptid_info.pid, &p_dbg_server->pid_info_queue);
}

/*Handle DBP responses to send status update to GDB*/
void
DBG_core_handle_status_response(DBG *dbg, bool b_err_status)
{
    DBG_core_debug_server *p_dbg_server = dbg->dbg_core.p_dbg_server;
    p_dbg_server->gdb_process_freeze_cb = NULL;

    if(!b_err_status)
    {
        DSPLOG_ERROR("Process 0x%x cannot be frozen", p_dbg_server->cur_ptid_info.pid);

        /*Make hg_ptid point to an invalid ptid*/
        copy_ptid(&p_dbg_server->hg_ptid, &gdb_invalid_ptid);

        /*Reply process exited*/
        DBG_core_gdb_send_reply(dbg, "W09", true);
    }
    else
    {
        dbg_core_pid_info_t *p_pid_info = DBG_core_find_pid_info(p_dbg_server->cur_ptid_info.pid,
                &p_dbg_server->pid_info_queue);

        /*We can only respond stop status to one inferior. Pick the
         * first thread*/
        dbg_tid_t tid = DBG_core_peek_tid_info(&p_pid_info->thread_info_queue)->tid;

        char ch_status_response[16];
        sprintf(ch_status_response, "T05thread:p%x.%x;", p_pid_info->pid, tid);

        DBG_core_gdb_send_reply(dbg, ch_status_response, true);
    }
}

/* Handle status update
 *
 * Indicate the reason the target halted. Reply should conform to the
 * Stop-Reply specification
 * */

void
DBG_core_gdb_handle_send_status_update(DBG *dbg)
{
    DSPLOG_INFO("Handle status update");

    DBG_core_debug_server *p_dbg_server = dbg->dbg_core.p_dbg_server;

    /*Core must be running to process this packet*/
    if (!DBG_core_must_be_running(dbg, "status upate", "E01"))
        return;

    /*Check if the ptid info is valid */
    if( IS_INVALID_PTID(p_dbg_server->cur_ptid_info) ||
        IS_MINUS_ONE_PTID(p_dbg_server->cur_ptid_info)
    )
    {
        DSPLOG_ERROR("gdb_send_status_update(): Cannot status update for p%x.%x",
                p_dbg_server->cur_ptid_info.pid, p_dbg_server->cur_ptid_info.tid);
        DBG_core_gdb_send_reply(dbg, "E01", true);
        return;
    }

    if(IS_NULL_PTID(p_dbg_server->cur_ptid_info))
    {
        DSPLOG_INFO("gdb_send_status_update(): NULL ptid - no user process available");
        DBG_core_gdb_send_reply(dbg, "W00", true);
        return;
    }

    /*Make sure the process for the thread is frozen*/
    dbg_process_state_t process_state = DBG_core_get_process_state(p_dbg_server->cur_ptid_info.pid,
                                        &p_dbg_server->pid_info_queue);

    if(process_state == DBG_PROCESS_INVALID)
    {
        DSPLOG_ERROR("gdb_send_status_update(): Process not available p%x", p_dbg_server->cur_ptid_info.pid);
        DBG_core_gdb_send_reply(dbg, "E01", true);
        return;
    }

    if( (process_state != DBG_PROCESS_STATE_FROZEN) &&
        (process_state != DBG_PROCESS_STATE_FROZEN_BP))
    {
       p_dbg_server->gdb_process_freeze_cb = DBG_core_handle_status_response;

       DBG_dbp_process_freeze(p_dbg_server, p_dbg_server->cur_ptid_info.pid );

       return;
    }

    DSPLOG_INFO("send_status_update(): Calls status_response()");
    DBG_core_handle_status_response(dbg, true);
    return;
}

void
DBG_core_gdb_handle_qSymbol(DBG *dbg __unused){}

/*
 * @brief   Handle read all registers for the current thread pointed by cur_ptid_info
 *
 * Ensures the process containing the thread is frozen - If not frozen enqueues a
 * DBP_CMD_PROCESS_FREEZE command and registers itself as a callback.
 *
 * After freezing the process - enqueues a read_all_registers packet
 * */
static void
DBG_core_gdb_handle_read_all_registers_mp(DBG *dbg, bool b_err_status)
{
    static bool b_process_freeze_cmd_enqueued = false;

    DBG_core_debug_server *p_dbg_server = dbg->dbg_core.p_dbg_server;

    /*Core must be running to process this packet*/
    if (!DBG_core_must_be_running(dbg, "read all registers", "E01"))
        return;

    /*Check if the ptid info is valid */
    if( IS_INVALID_PTID(p_dbg_server->cur_ptid_info) ||
        IS_NULL_PTID(p_dbg_server->cur_ptid_info) ||
        IS_MINUS_ONE_PTID(p_dbg_server->cur_ptid_info)
    )
    {
        DSPLOG_ERROR("gdb_read_all_registers_mp(): Cannot read all registers for p%x.%x",
                p_dbg_server->cur_ptid_info.pid, p_dbg_server->cur_ptid_info.tid);
        DBG_core_gdb_send_reply(dbg, "E01", true);
        return;
    }

    /*Make sure the process for the thread is frozen*/
    dbg_process_state_t process_state = DBG_core_get_process_state(p_dbg_server->cur_ptid_info.pid,
                                        &p_dbg_server->pid_info_queue);

    if(process_state == DBG_PROCESS_INVALID)
    {
        DSPLOG_ERROR("gdb_read_all_registers_mp(): Process not available p%x", p_dbg_server->cur_ptid_info.pid);
        DBG_core_gdb_send_reply(dbg, "E01", true);
        return;
    }

    if( (process_state != DBG_PROCESS_STATE_FROZEN) &&
        (process_state != DBG_PROCESS_STATE_FROZEN_BP))
    {
       p_dbg_server->gdb_process_freeze_cb = DBG_core_gdb_handle_read_all_registers_mp;

       DBG_dbp_process_freeze(p_dbg_server, p_dbg_server->cur_ptid_info.pid );

       b_process_freeze_cmd_enqueued = true;

       return;
    }
    /*Process dbp_process_freeze_cb here*/
    if(b_process_freeze_cmd_enqueued)
    {
        b_process_freeze_cmd_enqueued = false;
        p_dbg_server->gdb_process_freeze_cb = NULL;
    }

    if(!b_err_status)
    {
        DSPLOG_ERROR("Process 0x%x cannot be frozen", p_dbg_server->cur_ptid_info.pid);

        /*Make hg_ptid point to an invalid ptid*/
        copy_ptid(&p_dbg_server->hg_ptid, &gdb_invalid_ptid);

        /*Reply error*/
        DBG_core_gdb_send_reply(dbg, "E02", true);
    }
    else
    {
        /*Enqueue a thread_read_all_registers packet*/
        DBG_dbp_thread_read_all_registers(p_dbg_server,
                p_dbg_server->cur_ptid_info.pid, p_dbg_server->cur_ptid_info.tid);
    }
}

void
DBG_core_gdb_handle_read_all_registers(DBG *dbg)
{
    DBG_core_gdb_handle_read_all_registers_mp(dbg, true);
}


/*
 * @brief   Response handler for read all registers RSP packet
 *
 * Converts the register context array from the target and repackes into format that GDB understands
 * Replies error if the packet is invalid
 * */
void
DBG_core_gdb_thread_read_all_registers_resp(DBG *dbg,
                                            uint8_t *p_reg_context,
                                            bool b_err_status)
{
    DBG_core_debug_server *p_dbg_server = dbg->dbg_core.p_dbg_server;

    if(!b_err_status)
    {
        DSPLOG_ERROR("read all registers for p%d.%d returned failure",
                p_dbg_server->cur_ptid_info.pid, p_dbg_server->cur_ptid_info.tid);

        DBG_core_gdb_send_reply(dbg, "E01", true);
        return;
    }

    DEBUG_SERVER_GDB_BUFFER_T *p_gdb_resp_buffer = &p_dbg_server->buffer_gdb_send_pack;
    char *p_ch_gdb_resp_buf = &p_gdb_resp_buffer->p_ch_buf[1];
    uint32_t index;
    DBG_core_cs_reg_info_t reg_info;

    for(index = FP_REG_R0; index <= FP_REGS_LIMIT; index++)
    {
        /*Perform a read operation*/
        DBG_core_get_dba_reg_info(index, &reg_info);

        if(reg_info.b_valid)
        {
            memToHex(&p_reg_context[reg_info.u32_cs_reg_offset], p_ch_gdb_resp_buf, reg_info.u32_cs_reg_size);
            p_ch_gdb_resp_buf += (reg_info.u32_cs_reg_size * 2);
        }
        else
        {
            uint32_t i;
            for(i=0; i<reg_info.u32_cs_reg_size*2; i++)
                *p_ch_gdb_resp_buf++ = 'x';
            *p_ch_gdb_resp_buf = '\0';
        }

        DSPLOG_DEBUG("read_all_registers: (%d %d %d %d) - %s",
                index, reg_info.b_valid, reg_info.u32_cs_reg_offset, reg_info.u32_cs_reg_size, &p_gdb_resp_buffer->p_ch_buf[1]);
    }
    DBG_core_gdb_send_reply(dbg, NULL, false);
}

/*
 * Handle 'm' packet from GDB
 * mAA..AA,LLLL  Read LLLL bytes at address AA..AA
 *
 * Read length bytes of memory starting at address addr. Note that addr may
 * not be aligned to any particular boundary. The stub need not use any
 * particular size or alignment when gathering data from memory for the
 * response; even if addr is word-aligned and length is a multiple of the word size,
 * the stub is free to use byte accesses, or not. For this reason, this packet
 * may not be suitable for accessing memory-mapped I/O devices.
 *
 * Reply:
 *
 *  XX... Memory contents; each byte is transmitted as a two-digit hexadecimal number.
 *  The reply may contain fewer bytes than requested if the server was able to read
 *  only part of the region of memory.
 *
 *  E NN NN is errno
 * */

static void
DBG_core_gdb_handle_read_memory_mp(DBG *dbg, bool b_err_status)
{
    DBG_core_debug_server *p_dbg_server = dbg->dbg_core.p_dbg_server;
    DBG_TARGET *p_dbg_target = &dbg->dbg_target_if;

    DEBUG_SERVER_GDB_BUFFER_T *p_gdb_resp_buffer = &p_dbg_server->buffer_gdb_send_pack;
    char *p_ch_gdb_pack = p_dbg_server->gdb_packet.p_ch_gdb_pack;
    uint32_t u32_addr = 0;
    uint32_t u32_len = 0;
    static bool b_process_freeze_cmd_enqueued = false;

    DSPLOG_DEBUG("DBG(%u): Handle read memory packet", dbg->dbg_core.id);

    /* mAA..AA,LLLL  Read LLLL bytes at address AA..AA */
    if(hexToInt(&p_ch_gdb_pack, &u32_addr))
    {
        if(*(p_ch_gdb_pack++) == ',')
        {
            /*Get the number of bytes to process*/
            if(hexToInt(&p_ch_gdb_pack, &u32_len))
            {
                p_ch_gdb_pack = 0;
            }
        }
    }

    DSPLOG_DETAIL("gdb_handle_read_memory(): u32_addr : 0x%x u32_len : 0x%x", u32_addr, u32_len);
    if(p_ch_gdb_pack || u32_addr == 0 || u32_len == 0)
        goto read_mem_packet_err;

    /*Ceil the length to the max buffer size of the gdb response buffer*/
    u32_len = (u32_len > ((p_gdb_resp_buffer->s_max_buf_size >> 2) - 4) ?
            ((p_gdb_resp_buffer->s_max_buf_size >> 2) - 4) : u32_len);

    /*Core must be running to process this packet*/
    if (!DBG_core_must_be_running(dbg, "read memory", "E01"))
        return;

    /*Check if the ptid info is valid */
    if( IS_INVALID_PTID(p_dbg_server->cur_ptid_info) ||
        IS_NULL_PTID(p_dbg_server->cur_ptid_info) ||
        IS_MINUS_ONE_PTID(p_dbg_server->cur_ptid_info)
    )
    {
        DSPLOG_ERROR("gdb_read_memory_mp(): Cannot read memory for p%x.%x",
                p_dbg_server->cur_ptid_info.pid, p_dbg_server->cur_ptid_info.tid);
        DBG_core_gdb_send_reply(dbg, "E01", true);
        return;
    }

    /*Make sure the process for the thread is frozen*/
    dbg_process_state_t process_state = DBG_core_get_process_state(p_dbg_server->cur_ptid_info.pid,
                                        &p_dbg_server->pid_info_queue);

    if(process_state == DBG_PROCESS_INVALID)
    {
        DSPLOG_ERROR("gdb_read_memory_mp(): Process not available p%x", p_dbg_server->cur_ptid_info.pid);
        DBG_core_gdb_send_reply(dbg, "E01", true);
        return;
    }

    if( (process_state != DBG_PROCESS_STATE_FROZEN) &&
        (process_state != DBG_PROCESS_STATE_FROZEN_BP))
    {
       p_dbg_server->gdb_process_freeze_cb = DBG_core_gdb_handle_read_memory_mp;

       DBG_dbp_process_freeze(p_dbg_server, p_dbg_server->cur_ptid_info.pid );

       b_process_freeze_cmd_enqueued = true;

       return;
    }

    if(b_process_freeze_cmd_enqueued)
    {
        b_process_freeze_cmd_enqueued = false;
        p_dbg_server->gdb_process_freeze_cb = NULL;
    }

    if(!b_err_status)
    {
        DSPLOG_ERROR("Process 0x%x cannot be frozen", p_dbg_server->cur_ptid_info.pid);

        /*Make hg_ptid point to an invalid ptid*/
        copy_ptid(&p_dbg_server->hg_ptid, &gdb_invalid_ptid);

        /*Reply error*/
        DBG_core_gdb_send_reply(dbg, "E02", true);
    }
    else
    {
        /* Enqueue read memory command
         * We read to a maximum of channel size, even if GDB
         * requests more.
         * This helps to maintain atomic command and response
         * with the target.
         *  */
        uint16_t u16_block_read_size = (u32_len > p_dbg_target->u16_channel_size) ?
                                            p_dbg_target->u16_channel_size : u32_len;

        DBG_dbp_process_read_memory(p_dbg_server, p_dbg_server->cur_ptid_info.pid,
                u32_addr,
                u16_block_read_size);

    }

    return;

read_mem_packet_err:
    DSPLOG_ERROR("Invalid register read packet - return error");
    DBG_core_gdb_send_reply(dbg, "E01", true);
    return;
}

void
DBG_core_gdb_handle_read_memory(DBG *dbg)
{

    DBG_core_gdb_handle_read_memory_mp(dbg, true);
}

void
DBG_core_gdb_process_read_memory_resp(DBG *dbg, uint32_t u32_block_address __unused,
                                      uint16_t u16_block_size,
                                      uint8_t *p_u8_data,
                                      bool b_err_status)
{
    DBG_core_debug_server *p_dbg_server = dbg->dbg_core.p_dbg_server;
    char *p_ch_gdb_pack = p_dbg_server->gdb_packet.p_ch_gdb_pack;
    uint32_t u32_addr = 0;
    uint32_t u32_len = 0;

    DEBUG_SERVER_GDB_BUFFER_T *p_gdb_resp_buffer = &p_dbg_server->buffer_gdb_send_pack;
    char *p_ch_gdb_resp_buf = NULL;

    if(!b_err_status)
    {
        DSPLOG_ERROR("Received invalid response from the target - return ERROR");
        DBG_core_gdb_send_reply(dbg, "E02", true);
        return;
    }
    if(u16_block_size == 0)
    {
        DSPLOG_ERROR("process_read_memory_resp(): Target returned 0 bytes instead of reading %d bytes", u32_len);
        DBG_core_gdb_send_reply(dbg, "E03", true);
        return;
    }

    /* Read the address and length from the GDB command packet
     * once again to process the response*/
    /* mAA..AA,LLLL  Read LLLL bytes at address AA..AA */
    if(hexToInt(&p_ch_gdb_pack, &u32_addr))
    {
        if(*(p_ch_gdb_pack++) == ',')
        {
            /*Get the number of bytes to process*/
            if(hexToInt(&p_ch_gdb_pack, &u32_len))
            {
                p_ch_gdb_pack = 0;
            }
        }
    }

    /*Ceil the length to the max buffer size of the gdb response buffer*/
    u32_len = (u32_len > ((p_gdb_resp_buffer->s_max_buf_size >> 2) - 4) ?
            ((p_gdb_resp_buffer->s_max_buf_size >> 2) - 4) : u32_len);

    p_ch_gdb_resp_buf = &p_gdb_resp_buffer->p_ch_buf[1];

    /*Handle the actual response*/
    memToHex(p_u8_data, p_ch_gdb_resp_buf, u16_block_size);
    DBG_core_gdb_send_reply(dbg, NULL, false);
    return;
}

/*
 * Handle 'M' packet for GDB
 * 'M addr,length:XX...'
 * Write length bytes of memory starting at address addr. The data is given by
 * XX. . . ; each byte is transmitted as a two-digit hexadecimal number.
 *
 * Reply:
 *
 * 'OK' for success
 * 'E NN' for an error (this includes the case where only part of the data was
 * written).
 */

static void
DBG_core_gdb_handle_write_memory_mp(DBG *dbg, bool b_err_status)
{
    DBG_core_debug_server *p_dbg_server = dbg->dbg_core.p_dbg_server;
    DBG_TARGET *p_dbg_target = &dbg->dbg_target_if;

    char *p_ch_gdb_pack = p_dbg_server->gdb_packet.p_ch_gdb_pack;
    uint32_t u32_addr = 0;
    uint32_t u32_len = 0;
    static bool b_process_freeze_cmd_enqueued = false;
    uint8_t *p_u8_vals = NULL;

    DSPLOG_DEBUG("DBG(%u): Handle write memory packet", dbg->dbg_core.id);

    /* MAA..AA,LLLL:XX...XX  Write XX..XX, LLLL bytes at address AA..AA */
    if(hexToInt(&p_ch_gdb_pack, &u32_addr))
    {
        if(*(p_ch_gdb_pack++) == ',')
        {
            /*Get the number of bytes to process*/
            if(hexToInt(&p_ch_gdb_pack, &u32_len))
            {
                if(*(p_ch_gdb_pack++) == ':')
                {
                    /*Store bytes in memory*/
                    p_u8_vals = (uint8_t *)malloc(u32_len);
                    if(p_u8_vals == NULL)
                        goto write_mem_packet_err;

                    hexToMem (p_ch_gdb_pack, p_u8_vals, u32_len);
                    p_ch_gdb_pack = 0;
                }
            }
        }
    }
    if(p_ch_gdb_pack)
        goto write_mem_packet_err;

    /*Core must be running to process this packet*/
    if (!DBG_core_must_be_running(dbg, "write memory", "E01"))
        goto write_mem_return;

    /*Check if the ptid info is valid */
    if( IS_INVALID_PTID(p_dbg_server->cur_ptid_info) ||
        IS_NULL_PTID(p_dbg_server->cur_ptid_info) ||
        IS_MINUS_ONE_PTID(p_dbg_server->cur_ptid_info)
    )
    {
        DSPLOG_ERROR("gdb_write_memory_mp(): Cannot write memory for p%x.%x",
                p_dbg_server->cur_ptid_info.pid, p_dbg_server->cur_ptid_info.tid);
        DBG_core_gdb_send_reply(dbg, "E01", true);

        goto write_mem_return;
    }

    /*Make sure the process for the thread is frozen*/
    dbg_process_state_t process_state = DBG_core_get_process_state(p_dbg_server->cur_ptid_info.pid,
                                        &p_dbg_server->pid_info_queue);

    if(process_state == DBG_PROCESS_INVALID)
    {
        DSPLOG_ERROR("gdb_write_memory_mp(): Process not available p%x", p_dbg_server->cur_ptid_info.pid);
        DBG_core_gdb_send_reply(dbg, "E01", true);
        return;
    }

    if( (process_state != DBG_PROCESS_STATE_FROZEN) &&
        (process_state != DBG_PROCESS_STATE_FROZEN_BP))
    {
       p_dbg_server->gdb_process_freeze_cb = DBG_core_gdb_handle_write_memory_mp;

       DBG_dbp_process_freeze(p_dbg_server, p_dbg_server->cur_ptid_info.pid );

       b_process_freeze_cmd_enqueued = true;

       goto write_mem_return;

    }

    if(b_process_freeze_cmd_enqueued)
    {
        b_process_freeze_cmd_enqueued = false;
        p_dbg_server->gdb_process_freeze_cb = NULL;
    }

    if(!b_err_status)
    {
        DSPLOG_ERROR("Process 0x%x cannot be frozen", p_dbg_server->cur_ptid_info.pid);

        /*Make hg_ptid point to an invalid ptid*/
        copy_ptid(&p_dbg_server->hg_ptid, &gdb_invalid_ptid);

        /*Reply error*/
        DBG_core_gdb_send_reply(dbg, "E02", true);
    }
    else
    {

        /*Enqueue write memory commands*/
        uint16_t u16_block_write_size = p_dbg_target->u16_channel_size;
        uint16_t u32_address_index = 0;

        while (u32_len)
        {
            if((uint32_t) u16_block_write_size > u32_len)
                u16_block_write_size = (uint16_t)u32_len;

            DBG_dbp_process_write_memory(p_dbg_server, p_dbg_server->cur_ptid_info.pid,
                    u32_addr+u32_address_index,
                    u16_block_write_size,
                    &p_u8_vals[u32_address_index]);

            p_dbg_server->num_rw_mem_count++;

            u32_len -= u16_block_write_size;
            u32_address_index +=u16_block_write_size;
        }
    }

write_mem_return:
    if(p_u8_vals)
        free(p_u8_vals);

    return;

write_mem_packet_err:
    DSPLOG_ERROR("Invalid write memory packet - return error");
    DBG_core_gdb_send_reply(dbg, "E02", true);
    if(p_u8_vals)
        free(p_u8_vals);
    return;
}

void
DBG_core_gdb_handle_write_memory(DBG *dbg)
{

    DBG_core_gdb_handle_write_memory_mp(dbg, true);
}

void
DBG_core_gdb_process_write_memory_resp(DBG *dbg,
        uint16_t u16_size __unused, bool b_err_status)
{
    DBG_core_debug_server *p_dbg_server = dbg->dbg_core.p_dbg_server;

    if(!b_err_status)
    {
        DSPLOG_ERROR("Received invalid response from the target - return ERROR");
        p_dbg_server->num_rw_mem_count = 0;
        DBG_core_gdb_send_reply(dbg, "E02", true);
        return;
    }

    /*Handle the actual response*/
    p_dbg_server->num_rw_mem_count--;

    /*We are expecting more valid responses*/
    if(p_dbg_server->num_rw_mem_count > 0)
        return;

    if(p_dbg_server->num_rw_mem_count == 0)
        DBG_core_gdb_reply_ok(dbg);

    return;

}


/*
 * Handle 'p' packet for GDB
 * p n
 * Read the value of register n; n is in hex.
 *
 * Reply:
 * XX... - the register's value
 * E NN - for an error
 *  */
static void
DBG_core_gdb_handle_read_register_mp(DBG *dbg, bool b_err_status)
{
    DBG_core_debug_server *p_dbg_server = dbg->dbg_core.p_dbg_server;

    char *p_ch_gdb_pack = p_dbg_server->gdb_packet.p_ch_gdb_pack;
    static bool b_process_freeze_cmd_enqueued = false;
    uint32_t u32_regnum;

    DSPLOG_DEBUG("DBG(%u): Handle read register packet", dbg->dbg_core.id);

    if(hexToInt(&p_ch_gdb_pack, &u32_regnum) == 0)
    {
        DSPLOG_ERROR("Invalid register read packet - return error");
        DBG_core_gdb_send_reply(dbg, "E01", true);
        return;
    }

    /*Core must be running to process this packet*/
    if (!DBG_core_must_be_running(dbg, "read register", "E01"))
        return;

    /*Check if the ptid info is valid */
    if( IS_INVALID_PTID(p_dbg_server->cur_ptid_info) ||
        IS_NULL_PTID(p_dbg_server->cur_ptid_info) ||
        IS_MINUS_ONE_PTID(p_dbg_server->cur_ptid_info)
    )
    {
        DSPLOG_ERROR("gdb_read_register_mp(): Cannot write memory for p%x.%x",
                p_dbg_server->cur_ptid_info.pid, p_dbg_server->cur_ptid_info.tid);
        DBG_core_gdb_send_reply(dbg, "E01", true);

        return;
    }

    /*Make sure the process for the thread is frozen*/
    dbg_process_state_t process_state = DBG_core_get_process_state(p_dbg_server->cur_ptid_info.pid,
                                        &p_dbg_server->pid_info_queue);

    if(process_state == DBG_PROCESS_INVALID)
    {
        DSPLOG_ERROR("gdb_read_register_mp(): Process not available p%x", p_dbg_server->cur_ptid_info.pid);
        DBG_core_gdb_send_reply(dbg, "E01", true);
        return;
    }

    if( (process_state != DBG_PROCESS_STATE_FROZEN) &&
        (process_state != DBG_PROCESS_STATE_FROZEN_BP))
    {
       p_dbg_server->gdb_process_freeze_cb = DBG_core_gdb_handle_read_register_mp;

       DBG_dbp_process_freeze(p_dbg_server, p_dbg_server->cur_ptid_info.pid );

       b_process_freeze_cmd_enqueued = true;

       return;
    }

    if(b_process_freeze_cmd_enqueued)
    {
        b_process_freeze_cmd_enqueued = false;
        p_dbg_server->gdb_process_freeze_cb = NULL;
    }

    if(!b_err_status)
    {
        DSPLOG_ERROR("Process 0x%x cannot be frozen", p_dbg_server->cur_ptid_info.pid);

        /*Make hg_ptid point to an invalid ptid*/
        copy_ptid(&p_dbg_server->hg_ptid, &gdb_invalid_ptid);

        /*Reply error*/
        DBG_core_gdb_send_reply(dbg, "E02", true);
    }
    else
    {
        /*Read register*/
        DBG_core_cs_reg_info_t reg_info;
        DBG_core_get_dba_reg_info(u32_regnum, &reg_info);

        if(reg_info.b_valid)
        {
            /*Enqueue read-register command*/
            DBG_dbp_thread_read_register(p_dbg_server,
                                         p_dbg_server->cur_ptid_info.pid,
                                         p_dbg_server->cur_ptid_info.tid,
                                         reg_info.u32_cs_reg_offset,
                                         reg_info.u32_cs_reg_size);

        }
        else
        {
            /*Return xxxx*/
            DEBUG_SERVER_GDB_BUFFER_T *p_gdb_resp_buffer = &p_dbg_server->buffer_gdb_send_pack;
            char *p_ch_gdb_resp_buf = &p_gdb_resp_buffer->p_ch_buf[1];

            /*fill the buffer with XXs*/
            unsigned int i;
            for(i=0; i<reg_info.u32_cs_reg_size*2;i++)
                *p_ch_gdb_resp_buf++ = 'x';

            *p_ch_gdb_resp_buf = '\0';

            DBG_core_gdb_send_reply(dbg, NULL, false);
        }
    }

    return;
}

void
DBG_core_gdb_handle_read_register(DBG *dbg)
{
    DBG_core_gdb_handle_read_register_mp(dbg, true);
}

void
DBG_core_gdb_thread_read_register_resp(DBG *dbg, uint8_t *p_u8_value,
                                    uint32_t u32_reg_size,
                                    bool b_err_status)
{
    DBG_core_debug_server *p_dbg_server = dbg->dbg_core.p_dbg_server;
    DEBUG_SERVER_GDB_BUFFER_T *p_gdb_resp_buffer = &p_dbg_server->buffer_gdb_send_pack;
    char *p_ch_gdb_resp_buf = &p_gdb_resp_buffer->p_ch_buf[1];
    if(!b_err_status)
    {
        DSPLOG_ERROR("Received invalid response from the target - return ERROR");
        DBG_core_gdb_send_reply(dbg, "E02", true);
        return;
    }

    memToHex(p_u8_value, p_ch_gdb_resp_buf, u32_reg_size);

    DBG_core_gdb_send_reply(dbg, NULL, false);

    return;
}

/*
 * Handle 'P' write register packet
 * 'P n...=r...' Write register n. . . with value r. . . .
 * The register number n is in hexadecimal,
 * and r. . . contains two hex digits for each byte
 * in the register (target byte order).
 *
 * Reply:
 *
 * 'OK' for success
 * 'ENN' for an error
 * */
static void
DBG_core_gdb_handle_write_register_mp(DBG *dbg, bool b_err_status)
{
    DBG_core_debug_server *p_dbg_server = dbg->dbg_core.p_dbg_server;

    char *p_ch_gdb_pack = p_dbg_server->gdb_packet.p_ch_gdb_pack;
    uint32_t u32_regnum;
    static bool b_process_freeze_cmd_enqueued = false;
    DBG_core_cs_reg_info_t reg_info;

    DSPLOG_DEBUG("DBG(%u): Handle write register packet", dbg->dbg_core.id);

    /*Core must be running to process this packet*/
    if (!DBG_core_must_be_running(dbg, "read register", "E02"))
        return;

    /*Check if the ptid info is valid */
    if( IS_INVALID_PTID(p_dbg_server->cur_ptid_info) ||
        IS_NULL_PTID(p_dbg_server->cur_ptid_info) ||
        IS_MINUS_ONE_PTID(p_dbg_server->cur_ptid_info)
    )
    {
        DSPLOG_ERROR("gdb_write_register_mp(): Cannot write memory for p%x.%x",
                p_dbg_server->cur_ptid_info.pid, p_dbg_server->cur_ptid_info.tid);
        DBG_core_gdb_send_reply(dbg, "E01", true);

        return;
    }

    /*Parse write_register packet*/
    if(hexToInt(&p_ch_gdb_pack, &u32_regnum) == 0)
        goto write_reg_packet_err;

    if(*p_ch_gdb_pack++ != '=')
        goto write_reg_packet_err;

    /* Do register number translation from GDB to DBP offsets and populate
     * reg_info structure
     * */
    DBG_core_get_dba_reg_info(u32_regnum, &reg_info);
    TX_REGISTER_ID tx_regnum = DBG_core_gdb_to_threadxp_regnum(u32_regnum);
    uint64_t u64_reg_value = 0;

    /*Currently the target supports writing registers that are upto 64 bits wide*/
    if(reg_info.b_valid && tx_regnum != TX_NUM_REGISTER_IDS &&
            reg_info.u32_cs_reg_size <= sizeof(uint64_t))
    {
        hexToMem(p_ch_gdb_pack, (uint8_t *)&u64_reg_value, reg_info.u32_cs_reg_size);

        /*Make sure the process for the thread is frozen*/
        dbg_process_state_t process_state = DBG_core_get_process_state(p_dbg_server->cur_ptid_info.pid,
                &p_dbg_server->pid_info_queue);

        if(process_state == DBG_PROCESS_INVALID)
        {
            DSPLOG_ERROR("gdb_write_register_mp(): Process not available p%x", p_dbg_server->cur_ptid_info.pid);
            DBG_core_gdb_send_reply(dbg, "E01", true);
            return;
        }

        if( (process_state != DBG_PROCESS_STATE_FROZEN) &&
                (process_state != DBG_PROCESS_STATE_FROZEN_BP))
        {
            p_dbg_server->gdb_process_freeze_cb = DBG_core_gdb_handle_write_register_mp;

            DBG_dbp_process_freeze(p_dbg_server, p_dbg_server->cur_ptid_info.pid );

            b_process_freeze_cmd_enqueued = true;

            return;
        }

        if(b_process_freeze_cmd_enqueued)
        {
            b_process_freeze_cmd_enqueued = false;
            p_dbg_server->gdb_process_freeze_cb = NULL;
        }

        if(!b_err_status)
        {
            DSPLOG_ERROR("Process 0x%x cannot be frozen", p_dbg_server->cur_ptid_info.pid);

            /*Make hg_ptid point to an invalid ptid*/
            copy_ptid(&p_dbg_server->hg_ptid, &gdb_invalid_ptid);

            /*Reply error*/
            DBG_core_gdb_send_reply(dbg, "E02", true);
        }
        else
        {
            /*Perform register write - can we write PC ??*/
            DBG_dbp_thread_write_register(p_dbg_server,
                    p_dbg_server->cur_ptid_info.pid,
                    p_dbg_server->cur_ptid_info.tid,
                    tx_regnum, u64_reg_value);
        }

        return;
    }

write_reg_packet_err:
    /*Invalid packet or register*/
    DBG_core_gdb_send_reply(dbg, "E01", true);

}

void
DBG_core_gdb_handle_write_register(DBG *dbg)
{
    DBG_core_gdb_handle_write_register_mp(dbg, true);
}


void
DBG_core_gdb_thread_write_register_resp(DBG *dbg, bool b_err_status)
{
    if(b_err_status)
        DBG_core_gdb_reply_ok(dbg);
    else
    {
        DSPLOG_ERROR("gdb_write_register_resp() return E03");
        DBG_core_gdb_send_reply(dbg, "E03", true);
    }
}

static void
DBG_core_gdb_handle_h_int(DBG *dbg, bool b_err_status)
{
    DBG_core_debug_server *p_dbg_server = dbg->dbg_core.p_dbg_server;

    dbg_core_pid_info_t *p_pid_info = NULL;

    if(!b_err_status)
    {
        DSPLOG_ERROR("Update pid_list returned failure - return error");
        DBG_core_gdb_send_reply(dbg, "E02", true);
        return;
    }

    if(p_dbg_server->hg_ptid.pid > 0)
    {
        p_pid_info = DBG_core_find_pid_info(p_dbg_server->hg_ptid.pid, &p_dbg_server->pid_info_queue);

        if(p_pid_info == NULL)
        {
            DSPLOG_ERROR("handle_h_int(): Unable to find process %d", p_dbg_server->hg_ptid.pid);
            DBG_core_gdb_send_reply(dbg, "E04", true);
            return;
        }
    }
    else if(p_dbg_server->hg_ptid.pid == 0)
    {
        DSPLOG_INFO("handle_h_int(): 0 pid is any pid - cannot freeze any process - reply OK");
        DBG_core_gdb_reply_ok(dbg);
        return;
    }
    else
    {
        DSPLOG_ERROR("handle_h_int(): -1 pid is invalid");
        DBG_core_gdb_send_reply(dbg, "E03", true);
        return;
    }

    /*Update the ptid_info in the p_dbg_server with the chosen pid*/
    p_dbg_server->hg_ptid.pid = p_pid_info->pid;

    /*Attach the process if not attached*/
    if(!DBG_core_is_process_attached(p_pid_info))
    {
        p_dbg_server->gdb_process_attach_cb = NULL;
        DBG_dbp_process_attach(p_dbg_server, p_pid_info->pid);
    }

    /*Freeze the process*/
    p_dbg_server->gdb_process_freeze_cb = DBG_core_gdb_handle_h_response;
    dbg_process_state_t process_state = p_pid_info->process_state;

    if( (process_state != DBG_PROCESS_STATE_FROZEN) &&
        (process_state != DBG_PROCESS_STATE_FROZEN_BP))
    {
        DBG_dbp_process_freeze(p_dbg_server, p_dbg_server->hg_ptid.pid );
    }
    else
        p_dbg_server->gdb_process_freeze_cb(dbg, true);

    return;
}

/*Send appropriate response to the 'H' RSP packet*/
static void
DBG_core_gdb_handle_h_response(DBG *dbg, bool b_err_status)
{
    DBG_core_debug_server *p_dbg_server = dbg->dbg_core.p_dbg_server;
    /*de-register the callback function*/
    p_dbg_server->gdb_process_freeze_cb = NULL;

    if(!b_err_status)
    {
        DSPLOG_ERROR("handle_h_response(): Unable to freeze process");
        DBG_core_gdb_send_reply(dbg, "E04", true);
        return;
    }

    /*We are lazy to check if p_pid_info is NULL again*/
    dbg_core_pid_info_t *p_pid_info = DBG_core_find_pid_info(p_dbg_server->hg_ptid.pid,
                                        &p_dbg_server->pid_info_queue);

    dbg_core_thread_info_t *p_thread_info = NULL;

    dbg_tid_t tid;

    if(p_dbg_server->hg_ptid.tid > 0)
    {
        p_thread_info = DBG_core_find_thread_info(p_dbg_server->hg_ptid.tid, &p_pid_info->thread_info_queue);

        if(p_thread_info == NULL)
        {
            DSPLOG_ERROR("handle_h_response(): Cannot find specified thread");
            DBG_core_gdb_send_reply(dbg, "E04", true);
            return;
        }
        tid = p_thread_info->tid;
    }
    else if(p_dbg_server->hg_ptid.tid == 0)
    {
        p_thread_info = DBG_core_peek_tid_info(&p_pid_info->thread_info_queue);
        if(p_thread_info == NULL)
        {
            DSPLOG_INFO("handle_h_response(): No threads for this process");
            tid = 0;
        }
        else
            tid = p_thread_info->tid;
    }
    else    /*hg_ptid.tid == -1*/
    {
        tid = 0xFFFF;
    }

    p_dbg_server->cur_ptid_info.pid = p_pid_info->pid;
    p_dbg_server->cur_ptid_info.tid = tid;

    DSPLOG_INFO("handle_h_response(): Selected ptid p%x.%x", p_pid_info->pid, tid);

    DBG_core_gdb_reply_ok(dbg);

    return;
}

static void
copy_ptid(gdb_ptid_t *dest, gdb_ptid_t *src)
{
    dest->pid = src->pid;
    dest->tid = src->tid;
}

/*@brief - Read the incoming gdb character array and parse the ptid information
 *
 * p_ch_buf - start of the ptid information in the incoming buffer
 * *ptid    - pointer to hold the parsed ptid value
 *
 * Returns the pointer after the ptid information
 * */
static char*
DBG_core_read_ptid(char *p_ch_buf, gdb_ptid_t *ptid)
{
    int64_t pid = 0, tid = 0;
    bool b_neg_tid = false;

    /* Multi-process ptid.*/
    if(*p_ch_buf++ == 'p')
    {
        DSPLOG_INFO("dbg_core_read_ptid() - multiprocess - pid");
        if(hexToInt64(&p_ch_buf, &pid))
        {
            DSPLOG_INFO("dbg_core_read_ptid() - pid - %" PRId64, pid);
            if (*p_ch_buf++ != '.')
            {
                goto invalid_pid;
            }

            DSPLOG_INFO("dbg_core_read_ptid %s", p_ch_buf);

            if(*p_ch_buf == '-')
            {
                p_ch_buf++;
                b_neg_tid = true;
            }
            hexToInt64(&p_ch_buf, &tid);
            if(b_neg_tid)
                tid = -tid;

            DSPLOG_INFO("dbg_core_read_ptid() - tid - %" PRId64, tid);
        }
        else
            goto invalid_pid;
    }
    else
    {
        /* No multi-process.  Just a pid.  */
        if (strncmp(p_ch_buf, "-1", sizeof("-1")) == 0)
        {
            p_ch_buf+=2;
            pid = -1;
        }
        else
            hexToInt64(&p_ch_buf, &pid);

        tid = 0;
        DSPLOG_INFO("dbg_core_read_ptid() - no multiprocess pid - 0x%" PRIx64, pid);
    }

    ptid->pid = (int16_t)pid;
    ptid->tid = (int16_t)tid;

    return p_ch_buf;

invalid_pid:
    DSPLOG_ERROR("Invalid ptid");
    copy_ptid(ptid, &gdb_invalid_ptid);
    return p_ch_buf;
}

static void
DBG_core_write_ptid(dbg_pid_t pid, dbg_tid_t tid, char *p_ch_buf)
{
    p_ch_buf += sprintf(p_ch_buf, "p%x.", pid);
    p_ch_buf += sprintf(p_ch_buf, "%x", tid);
}


/*
 * @brief   -   Handle H packet from GDB
 *
 * H op thread-id
 *
 * Set thread for subsequent operations (m, M, g, G, et.al.). Depending on the
 * operation to be performed, op should be c for step and continue operations
 * (note that this is deprecated, supporting the vCont command is a better option),
 * and g for other operations. The thread designator thread-id has the
 * format and interpretation described in [thread-id syntax].
 *
 * Reply:
 * OK for success
 * E NN for an error
*/
void
DBG_core_gdb_handle_h_packet(DBG *dbg)
{
    DBG_core_debug_server *p_dbg_server = dbg->dbg_core.p_dbg_server;
    char *p_ch_gdb_pack = p_dbg_server->gdb_packet.p_ch_gdb_pack;

    gdb_ptid_t gdb_ptid;

    DSPLOG_DEBUG("handle_h_packet()");

    if (!DBG_core_must_be_running(dbg, "H ", "E01"))
        return;

    char ch_H_sub_cmd = *p_ch_gdb_pack++;

    if( ch_H_sub_cmd  == 'g' || ch_H_sub_cmd == 'c' || ch_H_sub_cmd == 's')
    {
        /*Read the process and thread-id from the gdb packet*/
        DBG_core_read_ptid(p_ch_gdb_pack, &gdb_ptid);
        DSPLOG_INFO("pid - %d tid - %d", gdb_ptid.pid, gdb_ptid.tid);

        /*Store them to the appropriate book-keeping members in the debugserver structure*/
        if(ch_H_sub_cmd == 'g')
            copy_ptid(&p_dbg_server->hg_ptid, &gdb_ptid);
        else
            copy_ptid(&p_dbg_server->hc_ptid, &gdb_ptid);

        if(ch_H_sub_cmd == 'g')
        {
            DSPLOG_INFO("handle_h_packet(): enqueue get_pid_list");
            p_dbg_server->gdb_get_pid_list_cb = DBG_core_gdb_handle_h_int;
            DBG_dbp_get_pid_list(p_dbg_server);
        }
        else
            DBG_core_gdb_reply_ok(dbg);
    }
    else
    {
        DSPLOG_DEBUG("Unsupported H packet format - reply no support");
        DBG_core_gdb_reply_no_support(dbg);
    }
    return;
}


/*
 * 'T thread-id'
 * Find out if the thread thread-id is alive.
 *
 * Reply:
 * 'OK' thread is still alive
 * 'E NN' thread is dead
 *
 * */

static dbg_core_ptid_info_t dbg_t_ptid = {0xFFFF, 0xFFFF};

static void
DBG_core_gdb_handle_T_packet_int(DBG *dbg, bool b_err_status);

static void
DBG_core_gdb_handle_T_response(DBG *dbg, bool b_err_status);

void
DBG_core_gdb_handle_T_packet(DBG *dbg)
{
    DBG_core_debug_server *p_dbg_server = dbg->dbg_core.p_dbg_server;
    char *p_ch_gdb_pack = p_dbg_server->gdb_packet.p_ch_gdb_pack;

    gdb_ptid_t ptid;

    /*Core must be running to process this packet*/
    if (!DBG_core_must_be_running(dbg, "T packet", "E01"))
        return;

    DBG_core_read_ptid(p_ch_gdb_pack, &ptid);

    dbg_t_ptid.pid = (ptid.pid < 0) ? 0xFFFF : ptid.pid;
    dbg_t_ptid.tid = (ptid.tid < 0) ? 0xFFFF : ptid.tid;

    if( IS_INVALID_PTID(ptid) ||
        IS_NULL_PTID(ptid) ||
        IS_MINUS_ONE_PTID(ptid) )
    {
        DSPLOG_ERROR("gdb_handle_T_packet(): invalid ptid p%d.%d", ptid.pid, ptid.tid);
        DBG_core_gdb_send_reply(dbg, "E02", true);
        return;
    }

    dbg_core_pid_info_t *p_pid_info = DBG_core_find_pid_info(dbg_t_ptid.pid, &p_dbg_server->pid_info_queue);

    if(p_pid_info == NULL)
    {
        DSPLOG_INFO("gdb_handle_T_packet: Unknown pid %d - get_pid_list", dbg_t_ptid.pid);
        p_dbg_server->gdb_get_pid_list_cb = DBG_core_gdb_handle_T_packet_int;
        DBG_dbp_get_pid_list(p_dbg_server);

        return;
    }

    DBG_core_gdb_handle_T_packet_int(dbg, true);
}

static void
DBG_core_gdb_handle_T_packet_int(DBG *dbg, bool b_err_status)
{
    DBG_core_debug_server *p_dbg_server = dbg->dbg_core.p_dbg_server;

    if(!b_err_status)
    {
        DSPLOG_ERROR("gdb_handle_T_packet(): get_pid_list retruned failure");
        DBG_core_gdb_send_reply(dbg, "E01", true);
        return;
    }

    p_dbg_server->gdb_get_pid_list_cb = NULL;

    dbg_core_pid_info_t *p_pid_info = DBG_core_find_pid_info(dbg_t_ptid.pid, &p_dbg_server->pid_info_queue);

    if(p_pid_info->process_state != DBG_PROCESS_STATE_FROZEN_BP &&
       p_pid_info->process_state != DBG_PROCESS_STATE_FROZEN)
    {
        p_dbg_server->gdb_process_freeze_cb = DBG_core_gdb_handle_T_response;
        DBG_dbp_process_freeze(p_dbg_server, dbg_t_ptid.pid);
        return;
    }

    DBG_core_gdb_handle_T_response(dbg, true);
}

static void
DBG_core_gdb_handle_T_response(DBG *dbg, bool b_err_status)
{
    DBG_core_debug_server *p_dbg_server = dbg->dbg_core.p_dbg_server;

    if(!b_err_status)
    {
        DSPLOG_ERROR("gdb_handle_T_packet(): process_freeze retruned failure");
        DBG_core_gdb_send_reply(dbg, "E01", true);
        return;
    }

    p_dbg_server->gdb_process_freeze_cb = NULL;

    dbg_core_pid_info_t *p_pid_info = DBG_core_find_pid_info(dbg_t_ptid.pid, &p_dbg_server->pid_info_queue);
    dbg_core_thread_info_t *p_thread_info = DBG_core_find_thread_info(dbg_t_ptid.tid, &p_pid_info->thread_info_queue);

    if(p_thread_info == NULL)
    {
        DSPLOG_INFO("gdb_handle_T_packet(): p%d.%d not found", dbg_t_ptid.pid, dbg_t_ptid.tid);
        DBG_core_gdb_send_reply(dbg, "E02", true);
    }
    else
    {
        DSPLOG_INFO("gdb_handle_T_packet(): p%d.%d found", dbg_t_ptid.pid, dbg_t_ptid.tid);
        DBG_core_gdb_reply_ok(dbg);
    }

    dbg_t_ptid.pid = 0xFFFF;
    dbg_t_ptid.tid = 0xFFFF;
}


/*
 * v Packets starting with 'v' are identified by a multi-letter name,
 * up to the first ';' or '?' (or the end of the packet).
*/
void
DBG_core_gdb_handle_v_packet(DBG *dbg)
{
    DBG_core_debug_server *p_dbg_server = dbg->dbg_core.p_dbg_server;
    DEBUG_SERVER_GDB_PACKET *p_gdb_packet = &p_dbg_server->gdb_packet;

    DSPLOG_DEBUG("handle_v_packet():");

    if(strstr(p_gdb_packet->p_ch_gdb_pack, "MustReplyEmpty"))
        DBG_core_gdb_reply_ok(dbg);
    else if(strstr(p_gdb_packet->p_ch_gdb_pack, "Cont?"))
        DBG_core_gdb_send_reply(dbg, "vCont;c;C;s;", true);
    else if(strstr(p_gdb_packet->p_ch_gdb_pack, "Cont;"))
        DBG_core_gdb_handle_vCont(dbg);
    else if(strstr(p_gdb_packet->p_ch_gdb_pack, "Attach;"))
        DBG_core_gdb_handle_vAttach(dbg);
    else if(strstr(p_gdb_packet->p_ch_gdb_pack, "Kill;"))
        DBG_core_gdb_handle_vKill(dbg);
    else if(strstr(p_gdb_packet->p_ch_gdb_pack, "Run;"))
        DBG_core_gdb_handle_vRun(dbg);
    else
        DBG_core_gdb_reply_no_support(dbg);
}

/*
 * vAttach;pid
 *
 * Attach to a new process with the specified process ID pid. The process ID is a
 * hexadecimal integer identifying the process. In all-stop mode, all threads in the
 * attached process are stopped; in non-stop mode, it may be attached without
 * being stopped if that is supported by the target.
 *
 * This packet is only available in extended mode.
 *
 * Reply:
 *
 * 'E nn' for an error
 * 'Any stop packet' for success in all-stop mode
 * 'OK' for success in non-stop mode
 * */

static dbg_pid_t vattach_pid = 0xFFFF;

static void
DBG_core_gdb_handle_vAttach_int(DBG *dbg, bool b_err_status);

static void
DBG_core_gdb_handle_vAttach_response(DBG *dbg, bool b_err_status);

static void
DBG_core_gdb_handle_vAttach(DBG *dbg)
{
    DBG_core_debug_server *p_dbg_server = dbg->dbg_core.p_dbg_server;

    /*Core must be running to process this packet*/
    if (!DBG_core_must_be_running(dbg, "vAttach packet", "E01"))
        return;

    /*Get the pid*/
    char *p_ch_gdb_pack = p_dbg_server->gdb_packet.p_ch_gdb_pack;
    char *p_ch = &p_ch_gdb_pack[7];

    uint32_t u32_pid_value = 0;

    hexToInt(&p_ch, &u32_pid_value);

    if(u32_pid_value == 0)
    {
        DSPLOG_ERROR("Invalid pid information in vAttach");
        DBG_core_gdb_send_reply(dbg, "E01", true);
        return;
    }

    vattach_pid = (dbg_pid_t) u32_pid_value;

    dbg_core_pid_info_t *p_pid_info = DBG_core_find_pid_info(vattach_pid, &p_dbg_server->pid_info_queue);

    if(p_pid_info == NULL)
    {
        DSPLOG_INFO("handle_vAttach(): Unable to identify p%d get_pid_list", vattach_pid);
        p_dbg_server->gdb_get_pid_list_cb = DBG_core_gdb_handle_vAttach_int;
        DBG_dbp_get_pid_list(p_dbg_server);
        return;
    }

    DBG_core_gdb_handle_vAttach_int(dbg, true);
}

static void
DBG_core_gdb_handle_vAttach_int(DBG *dbg, bool b_err_status)
{
    DBG_core_debug_server *p_dbg_server = dbg->dbg_core.p_dbg_server;

    p_dbg_server->gdb_get_pid_list_cb = NULL;

    if(!b_err_status)
    {
        DSPLOG_ERROR("handle_vAttach(): Unable to get PID list - reply error");
        DBG_core_gdb_send_reply(dbg, "E02", true);
        return;
    }

    dbg_core_pid_info_t *p_pid_info = DBG_core_find_pid_info(vattach_pid, &p_dbg_server->pid_info_queue);

    if(p_pid_info == NULL)
    {
        DSPLOG_ERROR("handle_vAttach(): Unable to find p%d", vattach_pid);
        DBG_core_gdb_send_reply(dbg, "E02", true);
        return;
    }

    /*Attach and freeze the process*/
    if(!DBG_core_is_process_attached(p_pid_info))
    {
        p_dbg_server->gdb_process_attach_cb = DBG_core_gdb_handle_vAttach_response;
        DBG_dbp_process_attach(p_dbg_server, vattach_pid);
        return;
    }

    DBG_core_gdb_handle_vAttach_response(dbg, true);
}

static void
DBG_core_gdb_handle_vAttach_response(DBG *dbg, bool b_err_status)
{
    DBG_core_debug_server *p_dbg_server = dbg->dbg_core.p_dbg_server;
    static bool b_process_freeze_enqueued = false;

    dbg_core_pid_info_t *p_pid_info = DBG_core_find_pid_info(vattach_pid, &p_dbg_server->pid_info_queue);

    if(!b_process_freeze_enqueued)
    {
        p_dbg_server->gdb_process_attach_cb = NULL;

        if(!b_err_status)
        {
            DSPLOG_ERROR("handle_v_attach(): Cannot attach to process - process may have exited");
            DBG_core_gdb_send_reply(dbg, "E03",true);
            return;
        }

        if( (p_pid_info->process_state != DBG_PROCESS_STATE_FROZEN) &&
            (p_pid_info->process_state != DBG_PROCESS_STATE_FROZEN_BP) )
        {
            p_dbg_server->gdb_process_freeze_cb = DBG_core_gdb_handle_vAttach_response;
            b_process_freeze_enqueued = true;
            DBG_dbp_process_freeze(p_dbg_server, vattach_pid);
            return;
        }
    }
    else
    {
        /*Process freeze callback*/
        b_process_freeze_enqueued = false;

        p_dbg_server->gdb_process_freeze_cb = NULL;

        if(!b_err_status)
        {
            DSPLOG_ERROR("handle_v_attach(): Cannot freeze process - process may have exited");
            DBG_core_gdb_send_reply(dbg, "E03",true);
            return;
        }
    }

    /*Process is attached and frozen*/
    dbg_tid_t tid = DBG_core_peek_tid_info(&p_pid_info->thread_info_queue)->tid;

    /*Mark the newly created process as the current process*/
    p_dbg_server->cur_ptid_info.pid = vattach_pid;
    p_dbg_server->cur_ptid_info.tid = tid;

    char ch_status_response[16];
    sprintf(ch_status_response, "T05thread:p%x.%x;", p_pid_info->pid, tid);

    DBG_core_gdb_send_reply(dbg, ch_status_response, true);
}

/*
 * vCont[;action[:thread-id]]...
 *
 * Resume the inferior, specifying different actions for each thread. If an action
 * is specified with no thread-id, then it is applied to any threads that don't have
 * a specific action specified; if no default action is specified then other threads
 * should remain stopped in all-stop mode and in their current state in non-stop
 * mode. Specifying multiple default actions is an error; specifying no actions is
 * also an error. Thread IDs are specified using the standard RSP syntax.
 *
 * Currently supported actions are:
 * c Continue.
 * s Step.
 * The optional argument addr normally associated with the c, C, s, and S
 * packets is not supported in vCont.
 *
 * A stop reply should be generated for any affected thread not already stopped.
 * The stub must support vCont if it reports support for multiprocess extensions.
 * Note that in this case vCont actions can be specified to apply to all threads
 * in a process by using the ppid.-1 form of the thread-id.
 *
 * Reply: Stop Reply Packets
 * */

static dbg_core_resume_info_t *
DBG_core_parse_vCont_packet(char *p_ch_gdb_pack, int *num_actions);

static dbg_thread_resume_action_t
DBG_core_get_resume_action(dbg_resume_type_t resume_type);

static void
DBG_core_dbp_thread_freeze(DBG_core_debug_server *p_dbg_server, dbg_pid_t pid, dbg_tid_t tid);

static void
DBG_core_dbp_thread_unfreeze_if_frozen(DBG_core_debug_server *p_dbg_server, dbg_pid_t pid, dbg_tid_t tid);

static void
DBG_core_dbp_thread_step(DBG_core_debug_server *p_dbg_server, dbg_pid_t pid, dbg_tid_t tid);

static void
DBG_core_gdb_handle_pending_stop_reason(DBG *dbg, dbg_core_stop_reason_t *p_stop_reason);

static void
DBG_core_gdb_handle_vCont(DBG *dbg)
{
    DBG_core_debug_server *p_dbg_server = dbg->dbg_core.p_dbg_server;
    char *p_ch_gdb_pack = p_dbg_server->gdb_packet.p_ch_gdb_pack;
    int num_actions = 0;

    /*Core must be running to process this packet*/
    if (!DBG_core_must_be_running(dbg, "vCont packet", "E01"))
        return;

    dbg_pid_t pid = p_dbg_server->cur_ptid_info.pid;

    dbg_core_pid_info_t *p_pid_info = DBG_core_find_pid_info(pid, &p_dbg_server->pid_info_queue);

    if(p_pid_info == NULL)
    {
        DSPLOG_ERROR("gdb_handle_vCont(): Cannot find the process");
        DBG_core_gdb_send_reply(dbg, "E02", true);
        return;
    }

    /*Check if there is a pending stop_response before handling the vCont packet*/
    DSPLOG_DEBUG("gdb_handle_vcont(): dequeue stop reason");
    dbg_core_stop_reason_t *p_stop_reason = DBG_core_dequeue_stop_reason(&p_pid_info->stop_reason_queue);
    if(p_stop_reason != NULL)
    {
        DBG_core_gdb_handle_pending_stop_reason(dbg, p_stop_reason);
        return;
    }

    /* parse the vCont packet
     * Returns an array of resume actions and the number of actions
     * There could also be a default action which could be specified
     * */
    dbg_core_resume_info_t *p_resume_info = DBG_core_parse_vCont_packet(p_ch_gdb_pack, &num_actions);

    if(p_resume_info == NULL)
    {
        DSPLOG_ERROR("gdb_handle_vCont - parsing failed");
        DBG_core_gdb_send_reply(dbg, "E01", true);
        return;
    }

    /*assign resume action for each thread
     * The actions can be
     * 1. freeze
     * 2. unfreeze
     * 3. singlestep
     * */

    /* If there is only one action and the resume info marks the
     * pid value as 0xFFFF we need to derive the pid value from
     * the cur_ptid info
     *
     * If there are more than one action in the vCont packet, then
     * there is certainly one valid pid value.
     * Note: GDB would try to specify different actions for each
     * thread in a process, but wouldn't attempt to present
     * actions across processes in a single vCont packet
     * */
    bool b_default_action_present = false;
    int default_action_index = 0;

    if(num_actions == 1)
    {
        if (p_resume_info[0].tid == 0xFFFF)
        {
            DSPLOG_INFO("gdb_handle_vCont(): spotted default action - %d", p_resume_info[0].resume_type);
            pid = p_dbg_server->cur_ptid_info.pid;
            b_default_action_present = true;
            default_action_index = 0;
        }
    }
    else
    {
        /*Identify the process to resume*/
        int act_num;
        for(act_num = 0; act_num < num_actions; act_num++)
        {
            if(p_resume_info[act_num].pid != 0xFFFF)
            {
                pid = p_resume_info[act_num].pid;
                break;
            }
        }
        /* Check if there is any default action or an action
         * that applies to all threads*/
        for(act_num = 0; act_num < num_actions; act_num++)
        {
            if(p_resume_info[act_num].tid == 0xFFFF)
            {
                DSPLOG_INFO("gdb_handle_vCont(): spotted default action - %d", p_resume_info[act_num].resume_type);
                b_default_action_present = true;
                default_action_index = act_num;
            }
        }
    }

    dbg_core_thread_info_t *p_thread_info = DBG_core_peek_tid_info(&p_pid_info->thread_info_queue);

    /* Populate the resume action of all threads to the default action first -
     * we will overwrite with the thread specific action in the next pass.*/

    /* We are in all-stop mode, so default action is all threads stopped
     * if no default action is specified*/

    while(p_thread_info != NULL)
    {
        if(b_default_action_present)
            p_thread_info->resume_action = DBG_core_get_resume_action(p_resume_info[default_action_index].resume_type);
        else
            p_thread_info->resume_action = DBG_core_get_resume_action(DBG_RESUME_TYPE_STOPPED);

        GET_NEXT_TID(p_thread_info);
    }

    /*Overwrite thread specific actions here*/
    int actions;
    for(actions = 0; actions < num_actions; actions++)
    {
        if(b_default_action_present && actions == default_action_index)
            continue;
        else
        {
            p_thread_info = DBG_core_find_thread_info(p_resume_info[actions].tid, &p_pid_info->thread_info_queue);

            if(p_thread_info == NULL)
            {
                DSPLOG_ERROR("gdb_handle_vCont(): Unknown thread");
                free(p_resume_info);
                DBG_core_gdb_send_reply(dbg, "E03", true);
                return;
            }

            p_thread_info->resume_action = DBG_core_get_resume_action(p_resume_info[actions].resume_type);
        }
    }

    /*iterate over all threads and invoke the
     * action*/
    p_thread_info = DBG_core_peek_tid_info(&p_pid_info->thread_info_queue);

    while(p_thread_info != NULL)
    {
        if(p_thread_info->resume_action)
            p_thread_info->resume_action(p_dbg_server, p_pid_info->pid, p_thread_info->tid);

        GET_NEXT_TID(p_thread_info);
    }

    if(p_resume_info != NULL)
        free(p_resume_info);

    /*Finally resume the process*/
    DBG_dbp_process_unfreeze(p_dbg_server, p_pid_info->pid);
}

static void
DBG_core_gdb_handle_pending_stop_reason(DBG *dbg, dbg_core_stop_reason_t *p_stop_reason)
{
    DSPLOG_DEBUG("Handle pending stop_reason");

    switch(p_stop_reason->stop_reason)
    {
        case DBG_STOP_REASON_BPT_HIT:
            DSPLOG_DEBUG("pending stop reason - breakpoint hit - 0x%x.0x%x", p_stop_reason->pid, p_stop_reason->tid);
            DBG_core_gdb_stop_reply(dbg, p_stop_reason->pid, p_stop_reason->tid);
            break;

        case DBG_STOP_REASON_SSTEP:
            DSPLOG_DEBUG("pending stop reason - sstep - 0x%x.0x%x", p_stop_reason->pid, p_stop_reason->tid);
            /*Clear the single-step bit*/
            DBG_dbp_thread_clear_single_step(dbg->dbg_core.p_dbg_server, p_stop_reason->pid, p_stop_reason->tid);
            /*Send GDB response*/
            DBG_core_gdb_stop_reply(dbg, p_stop_reason->pid, p_stop_reason->tid);
            break;

        case DBG_STOP_REASON_WPT_HIT:
            DSPLOG_DEBUG("pending stop_reason - watchpoint hit - 0x%x.0x%x", p_stop_reason->pid, p_stop_reason->tid);
            /*Send GDB response*/
            DBG_core_gdb_watchpoint_hit(dbg, p_stop_reason->pid, p_stop_reason->tid,
                    p_stop_reason->u32_addr,
                    p_stop_reason->bpt_type);

            break;

        case DBG_STOP_REASON_THREAD_CREATED:
            DBG_core_gdb_thread_created(dbg, p_stop_reason->pid, p_stop_reason->tid);
            break;

        case DBG_STOP_REASON_THREAD_EXITED:
            DBG_core_gdb_thread_exited(dbg, p_stop_reason->pid, p_stop_reason->tid);
            break;

        case DBG_STOP_REASON_LIBRARY_LOADED:
        case DBG_STOP_REASON_LIBRARY_UNLOADED:
            DBG_core_gdb_notify_library_change(dbg);
            break;

        case DBG_STOP_REASON_PROCESS_EXITED:
            DBG_core_gdb_notify_process_terminate(dbg, p_stop_reason->pid, DBP_FAULT_UNKNOWN);
            break;

        default:
            DSPLOG_ERROR("Unsupported stop reason");
            break;
    }
}

static dbg_thread_resume_action_t
DBG_core_get_resume_action(dbg_resume_type_t resume_type)
{
    switch(resume_type)
    {
        case DBG_RESUME_TYPE_STOPPED:
            return DBG_core_dbp_thread_freeze;

        case DBG_RESUME_TYPE_STEP:
            return DBG_core_dbp_thread_step;

        case DBG_RESUME_TYPE_CONTINUE:
            return DBG_core_dbp_thread_unfreeze_if_frozen;

        default:
            return NULL;
    }
}

static void
DBG_core_dbp_thread_freeze(DBG_core_debug_server *p_dbg_server, dbg_pid_t pid, dbg_tid_t tid)
{
    DSPLOG_INFO("freeze_thread - p%x.%x", pid, tid);
    DBG_dbp_thread_freeze(p_dbg_server, pid, tid);
}

static void
DBG_core_dbp_thread_unfreeze_if_frozen(DBG_core_debug_server *p_dbg_server, dbg_pid_t pid, dbg_tid_t tid)
{
    dbg_core_pid_info_t *p_pid_info = DBG_core_find_pid_info(pid, &p_dbg_server->pid_info_queue);
    dbg_core_thread_info_t *p_thread_info = DBG_core_find_thread_info(tid, &p_pid_info->thread_info_queue);

    if(p_thread_info->thread_state == DBG_THREAD_STATE_FROZEN ||
       p_thread_info->thread_state == DBG_THREAD_STATE_FROZEN_BP)
    {
        DSPLOG_INFO("unfreeze thread - p%x.%x", pid, tid);
        DBG_dbp_thread_unfreeze(p_dbg_server, pid, tid);
    }
}

static void
DBG_core_dbp_thread_step(DBG_core_debug_server *p_dbg_server, dbg_pid_t pid, dbg_tid_t tid)
{
    DSPLOG_INFO("thread singlestep - p%x.%x", pid, tid);
    if(DBG_dbp_thread_set_single_step(p_dbg_server, pid, tid) == DBG_DBP_ERR_CODE_SUCCESS)
        DBG_core_dbp_thread_unfreeze_if_frozen(p_dbg_server, pid, tid);
}

/*
 * @brief   -   Parse vCont packet
 *
 * Parameters
 * 1. char *p_ch_gdb_pack - Pointer to the char buffer holding the vCont packet
 *                          (starting as Cont; - because the $ and v characters
 *                          are parsed alread)
 *
 * 2. int *num_actions    - Records the number of actions
 *
 * Return value
 * Pointer to the memory which holds resume information*/

static dbg_core_resume_info_t *
DBG_core_parse_vCont_packet(char *p_ch_gdb_pack, int *num_actions)
{
    int num = 0;

    /*Get the number of actions specified in the packet*/
    DSPLOG_INFO("parse_vcont(): ch_gdb_pack %s", p_ch_gdb_pack);
    char *p_ch = &p_ch_gdb_pack[4];
    while (p_ch)
    {
        num++;
        p_ch++;
        p_ch = strchr (p_ch, ';');
    }

    *num_actions = num;

    DSPLOG_INFO("num_actions in the vCont packet %d", *num_actions);

    dbg_core_resume_info_t *p_resume_info =
        (dbg_core_resume_info_t *) malloc(num * sizeof(dbg_core_resume_info_t));

    if(p_resume_info == NULL)
    {
        DSPLOG_ERROR("cannot allocate resume_info array");
        return NULL;
    }

    p_ch = &p_ch_gdb_pack[4];

    int resume_index = 0;

    while(*p_ch != '#')
    {
        /*Move to the action letter*/
        p_ch++;

        if(*p_ch == 'c')
            p_resume_info[resume_index].resume_type = DBG_RESUME_TYPE_CONTINUE;
        else if(*p_ch == 's')
            p_resume_info[resume_index].resume_type = DBG_RESUME_TYPE_STEP;
        else
        {
            DSPLOG_ERROR("parse_vcont_packet(): Unsupported action");
            free(p_resume_info);
            return NULL;
        }

        p_ch++;

        /* if no pid, tid value is present - this is the default action
         * for all inferiors whose actions are not specified
         * */
        if(*p_ch == '#')
        {
            p_resume_info[resume_index].pid = 0xFFFF;   /*Special marker to denote default action*/
            p_resume_info[resume_index].tid = 0xFFFF;
        }
        else if (*p_ch == ':')
        {
            gdb_ptid_t ptid;

            p_ch = DBG_core_read_ptid(&p_ch[1], &ptid);
            DSPLOG_INFO("parse_vcont(): p%d.%d", ptid.pid, ptid.tid);
            if(IS_INVALID_PTID(ptid))
            {
                DSPLOG_ERROR("parse_vcont_packet(): Invalid ptid in the packet");
                free(p_resume_info);
                return NULL;
            }
            p_resume_info[resume_index].pid = ptid.pid;

            if(ptid.tid == -1)
                p_resume_info[resume_index].tid = 0xFFFF;
            else
                p_resume_info[resume_index].tid = ptid.tid;

            if(*p_ch != ';' && *p_ch != '#')
            {
                DSPLOG_ERROR("parse_vcont_packet(): Invalid packet");
                free(p_resume_info);
                return NULL;
            }
        }
        resume_index++;
    }

    return p_resume_info;
}

/*@brief - Handle vRun packet
 *
 * vRun;filename[;argument]...
 *
 * Run the program filename, passing it each argument on its command line. The
 * file and arguments are hex-encoded strings. If filename is an empty string, the
 * stub may use a default program (e.g. the last program run). The program is
 * created in the stopped state.
 *
 * This packet is only available in extended mode.
 *
 * Reply:
 * 'E nn' for an error
 * 'Any stop packet' for success
 * */

void
DBG_core_gdb_process_create_resp_1(DBG *dbg, bool b_err_status)
{
    DBG_core_debug_server *p_dbg_server = dbg->dbg_core.p_dbg_server;

    /*Respond GDB to pick any thread as we will get the thread list response later*/
    char ch_status_response[24];
    dbg_pid_t pid = p_dbg_server->cur_ptid_info.pid;

    if(!b_err_status)
    {
        DSPLOG_ERROR("get_tid_list for new process %d returned false", pid);
        sprintf(ch_status_response, "T05thread:p%x.-1;", pid);
    }
    else
    {
        DSPLOG_INFO("get_tid_list for new process %d success - pick a thread", pid);
        dbg_core_pid_info_t *p_pid_info = DBG_core_get_current_process(p_dbg_server);
        dbg_tid_t tid = DBG_core_peek_tid_info(&p_pid_info->thread_info_queue)->tid;
        sprintf(ch_status_response, "T05thread:p%x.%x;", pid, tid);
    }
    DBG_core_gdb_send_reply(dbg, ch_status_response, true);
}

void
DBG_core_gdb_process_create_resp(DBG *dbg, dbg_pid_t pid, bool b_err_status)
{
    DBG_core_debug_server *p_dbg_server = dbg->dbg_core.p_dbg_server;

    if(!b_err_status)
    {
        DBG_core_gdb_send_reply(dbg, "E02", true);
        return;
    }

    /*Process create - frozen by default - attach and gather thread list*/
    p_dbg_server->gdb_process_attach_cb = NULL;
    DBG_dbp_process_attach(p_dbg_server, pid);

    p_dbg_server->gdb_process_get_tid_list_cb = DBG_core_gdb_process_create_resp_1;
    DBG_dbp_process_get_tid_list(p_dbg_server, pid);

    /*Mark the newly created process as the current process*/
    p_dbg_server->cur_ptid_info.pid = pid;
}

static void
DBG_core_gdb_handle_vRun(DBG *dbg)
{
    DBG_core_debug_server *p_dbg_server = dbg->dbg_core.p_dbg_server;

    char *p_ch_gdb_pack = p_dbg_server->gdb_packet.p_ch_gdb_pack;
    char *p_ch_vRun = strchr(p_ch_gdb_pack, '#');
    char *p_ch_source_packet = (char *)malloc(1 + (p_ch_vRun - p_ch_gdb_pack));

    /*Copy the gdb packet without the checksum string*/
    strncpy(p_ch_source_packet, p_ch_gdb_pack, (p_ch_vRun - p_ch_gdb_pack));
    p_ch_source_packet[(p_ch_vRun - p_ch_gdb_pack)] = '\0';

    DSPLOG_INFO("handle_vRun(): source %s vRun %s gdb_pack %s", p_ch_source_packet, p_ch_vRun, p_ch_gdb_pack);

    /* Allocate an argv string long enough to hold all the arguments sent with the vRun packet
     */
    char *argv = (char *)malloc(strlen(p_ch_source_packet) - strlen("Run") + DEFAULT_FILENAME_SIZE);
    int argc = 0;   /*Number of arguments including the executable name*/
    char *p_ch_argv = NULL;

    /*Calculate the number of arguments passed in the vRun packet*/
    for(p_ch_argv = p_ch_source_packet + strlen("Run"); p_ch_argv ; p_ch_argv = strchr(p_ch_argv, ';'))
    {
        p_ch_argv++;
        argc++;
    }

    /*parse the arguments*/
    char *p_ch_next_argv = NULL;
    char *argv_dest = argv;
    uint16_t buffer_len = 0;

    p_ch_argv = p_ch_source_packet + strlen("Run");
    int i;
    for(i = 0; i < argc ; i++)
    {
        p_ch_argv++;
        p_ch_next_argv = strchr(p_ch_argv, ';');

        if(p_ch_next_argv == NULL)
            p_ch_next_argv = p_ch_argv + strlen(p_ch_argv);

        DSPLOG_INFO("p_ch_argv %s p_ch_next_argv %s", p_ch_argv, p_ch_next_argv);

        int argv_len = 0;

        /*no filename - choose default file*/
        if(i == 0 && p_ch_argv == p_ch_next_argv)
        {
            strncpy(argv_dest, DEFAULT_FILE_NAME, DEFAULT_FILENAME_SIZE);
            argv_len = DEFAULT_FILENAME_SIZE;
        }
        else
        {
            /*Convert 2 byte hex array to string*/
            argv_len = hexToAscii(argv_dest, p_ch_argv, (p_ch_next_argv - p_ch_argv)/2);
        }
        argv_dest[argv_len] = '\0';

        DSPLOG_INFO("handle_vRun(): argv - %s", argv_dest);

        argv_dest += (argv_len + 1);
        buffer_len += (argv_len + 1);
    }

    /*Create the process*/
    DSPLOG_INFO("handle_vRun(): argc - %d buffer_len - %d", argc, buffer_len);
    DBG_dbp_process_create(p_dbg_server, argc, buffer_len, argv);
}

/*
 * @brief - Handle vKill packet
 *
 * vKill;pid
 *
 * Kill the process with the specified process ID pid, which is a hexadecimal integer
 * identifying the process. This packet is used in preference to k when
 * multiprocess protocol extensions are supported.
 *
 * Reply:
 * 'E nn' for an error
 * 'OK' for success
 * */
static void
DBG_core_gdb_handle_vKill(DBG *dbg)
{
    DBG_core_debug_server *p_dbg_server = dbg->dbg_core.p_dbg_server;

    char *p_ch_gdb_pack = p_dbg_server->gdb_packet.p_ch_gdb_pack;
    char *p_ch = p_ch_gdb_pack + strlen("Kill;");
    uint32_t u32_pid;

    /*Core must be running to process this packet*/
    if (!DBG_core_must_be_running(dbg, "vKill packet", "E01"))
        return;

    if(hexToInt(&p_ch, &u32_pid) == 0)
    {
        DSPLOG_ERROR("gdb_handle_vKill(): Invalid pid in the gdb packet");
        DBG_core_gdb_send_reply(dbg, "E02", true);
        return;
    }

    dbg_pid_t pid = (dbg_pid_t) u32_pid;

    DBG_dbp_process_terminate(p_dbg_server, pid);
}

void
DBG_core_gdb_vKill_resp(DBG *dbg)
{
    DBG_core_gdb_reply_ok(dbg);
}

/*
 * @brief   -   Handle insert/remove breakpoint/watchpoint
 *
 * z type,addr,kind
 * Z type,addr,kind
 *
 * Insert (Z) or remove (z) a type breakpoint or watchpoint starting at address
 * address of kind kind.
 *
 * Each breakpoint and watchpoint packet type is documented separately.
 *
 * Implementation notes: A remote target shall return an empty string for an unrecognized
 * breakpoint or watchpoint packet type. A remote target shall support
 * either both or neither of a given Ztype... and ztype... packet pair. To
 * avoid potential problems with duplicate packets, the operations should be implemented
 * in an idempotent way.
 * */
void
DBG_core_gdb_handle_z_packet(DBG *dbg, char b_cmd_type)
{
    DBG_core_debug_server *p_dbg_server = dbg->dbg_core.p_dbg_server;
    char *p_ch_gdb_pack = p_dbg_server->gdb_packet.p_ch_gdb_pack;
    uint32_t u32_bpt_type = 0;
    uint32_t u32_addr = 0;
    uint32_t u32_wp_num_bytes = 0;

    DSPLOG_DEBUG("DBG(%u): Handle %s breakpoint packet", dbg->dbg_core.id,( (b_cmd_type == DBG_BPT_WPT_REMOVE) ? "remove" : "insert" ));

    /*Core must be running to process this packet*/
    if (!DBG_core_must_be_running(dbg, "read register", "E02"))
        return;

    /*Check if the ptid info is valid */
    if( IS_INVALID_PTID(p_dbg_server->cur_ptid_info) ||
        IS_NULL_PTID(p_dbg_server->cur_ptid_info) ||
        IS_MINUS_ONE_PTID(p_dbg_server->cur_ptid_info)
    )
    {
        DSPLOG_ERROR("gdb_handle_z_packet(): Cannot set breakpoint/watchpoint for p%x.%x",
                p_dbg_server->cur_ptid_info.pid, p_dbg_server->cur_ptid_info.tid);
        DBG_core_gdb_send_reply(dbg, "E01", true);

        return;
    }

    /* Zn,addr,kind */
    if(hexToInt(&p_ch_gdb_pack, &u32_bpt_type))
    {
        if(u32_bpt_type <= DBG_BREAKPOINT_TYPE_HW && *(p_ch_gdb_pack++) == ',')
        {
            u32_bpt_type = DBG_BREAKPOINT_TYPE_HW;
            /*Get the address to plant the breakpoint*/
            if(hexToInt(&p_ch_gdb_pack, &u32_addr))
            {
                p_ch_gdb_pack = 0;
            }
        }
        else if(u32_bpt_type <= DBG_WATCHPOINT_TYPE_ACCESS && *(p_ch_gdb_pack++) == ',')
        {
            /*Get the address to watch*/
            if(hexToInt(&p_ch_gdb_pack, &u32_addr))
            {
                if(*(p_ch_gdb_pack++)==',')
                {
                    if(hexToInt(&p_ch_gdb_pack, &u32_wp_num_bytes))
                    {
                        p_ch_gdb_pack = 0;
                    }
                }

            }
        }
    }
    if(p_ch_gdb_pack)
    {
        DSPLOG_ERROR("DBG(%u) gdb_handle_Z_packet(): Invalid Z packet", dbg->dbg_core.id);
        DBG_core_gdb_send_reply(dbg, "E01", true);
        return;
    }

    /*Allocate breakpoint info and mark as a pending breakpoint/watchpoint event
     * We first need to ensure our current process is frozen (Most probably it is)
     * */
    dbg_core_pid_info_t *p_pid_info = DBG_core_get_current_process(p_dbg_server);

    /*insert breakpoint/watchpoint*/
    if(b_cmd_type)
    {
        /*Create a breakpoint info variable*/
        p_pid_info->p_dbg_pending_bp = DBG_core_bp_info_alloc(p_dbg_server->cur_ptid_info.pid,
                                                              0xFFFF,
                                                              DBG_BP_NUM_PENDING,
                                                              u32_bpt_type,
                                                              u32_addr,
                                                              u32_wp_num_bytes);

        if(p_pid_info->p_dbg_pending_bp == NULL)
        {
            DSPLOG_ERROR("Cannot allocate breakpoint info");
            DBG_core_gdb_send_reply(dbg, "E01", true);
            return;
        }
    }
    else
    {
        /*find an existing breakpoint info to remove*/
        p_pid_info->p_dbg_pending_bp = DBG_core_find_bp_info_from_addr_1(u32_addr,
                                       u32_bpt_type,
                                       p_dbg_server->cur_ptid_info.tid,
                                       p_pid_info);

        if(p_pid_info->p_dbg_pending_bp == NULL)
        {
            DSPLOG_INFO("Breakpoint/watchpoint not in the target - reply ok");
            DBG_core_gdb_reply_ok(dbg);
            return;
        }
    }

    /*Make sure the process for the thread is frozen*/
    dbg_process_state_t process_state = p_pid_info->process_state;

    if(process_state == DBG_PROCESS_INVALID)
    {
        DSPLOG_ERROR("gdb_breakpoint_mp(): Process not available p%x", p_dbg_server->cur_ptid_info.pid);
        DBG_core_gdb_send_reply(dbg, "E01", true);
        return;
    }

    if(b_cmd_type)
        p_dbg_server->gdb_process_freeze_cb = DBG_core_gdb_handle_insert_break_watch_mp;
    else
        p_dbg_server->gdb_process_freeze_cb = DBG_core_gdb_handle_remove_break_watch_mp;

    if( (process_state != DBG_PROCESS_STATE_FROZEN) &&
        (process_state != DBG_PROCESS_STATE_FROZEN_BP))
        DBG_dbp_process_freeze(p_dbg_server, p_dbg_server->cur_ptid_info.pid );
    else
        p_dbg_server->gdb_process_freeze_cb(dbg, true);

    return;
}

static void
DBG_core_gdb_handle_insert_break_watch_mp(DBG *dbg, bool b_err_status)
{
    DBG_core_debug_server *p_dbg_server = dbg->dbg_core.p_dbg_server;

    dbg_core_bp_info_t *p_pending_bp = DBG_core_get_current_process(p_dbg_server)->p_dbg_pending_bp;

    if(!b_err_status)
    {
        DSPLOG_ERROR("Unable to freeze p%x - cannot set/remove breakpoint/watchpoint",
                p_pending_bp->pid);
        DBG_core_gdb_send_reply(dbg, "E02", true);

        return;
    }
    p_dbg_server->gdb_process_freeze_cb = NULL;

    switch(p_pending_bp->bpt_type)
    {
        case DBG_BREAKPOINT_TYPE_HW:
        case DBG_BREAKPOINT_TYPE_SW:
            DSPLOG_INFO("insert hw breakpoint @ addr 0x%x for p%x.%x", p_pending_bp->u32_addr,
                                                                        p_pending_bp->pid,
                                                                        p_pending_bp->tid);
            DBG_dbp_set_breakpoint(p_dbg_server,
                    p_pending_bp->pid,
                    p_pending_bp->tid,
                    p_pending_bp->u32_addr);
            break;

        case DBG_WATCHPOINT_TYPE_WRITE:
            DSPLOG_DEBUG("insert hw write watchpoint @ addr 0x%x for 0x%x bytes for p%x.%x", p_pending_bp->u32_addr,
                    p_pending_bp->u32_wp_num_bytes,
                    p_pending_bp->pid, p_pending_bp->tid);

            DBG_dbp_set_watchpoint(p_dbg_server,
                    p_pending_bp->pid,
                    p_pending_bp->tid,
                    p_pending_bp->u32_addr,
                    p_pending_bp->u32_wp_num_bytes,
                    DBP_WPT_STORE);
            break;

        case DBG_WATCHPOINT_TYPE_READ:
            DSPLOG_DEBUG("insert hw read watchpoint @ addr 0x%x for 0x%x bytes", p_pending_bp->u32_addr,
                    p_pending_bp->u32_wp_num_bytes);
            DBG_dbp_set_watchpoint(p_dbg_server,
                    p_pending_bp->pid,
                    p_pending_bp->tid,
                    p_pending_bp->u32_addr,
                    p_pending_bp->u32_wp_num_bytes,
                    DWP_WPT_LOAD);
            break;
        case DBG_WATCHPOINT_TYPE_ACCESS:
            DSPLOG_DEBUG("insert hw watchpoint @ addr 0x%x for 0x%x bytes", p_pending_bp->u32_addr,
                    p_pending_bp->u32_wp_num_bytes);
            DBG_dbp_set_watchpoint(p_dbg_server,
                    p_pending_bp->pid,
                    p_pending_bp->tid,
                    p_pending_bp->u32_addr,
                    p_pending_bp->u32_wp_num_bytes,
                    DBP_WPT_ALL);
            break;

        default:
            DSPLOG_ERROR("Unknown breakpoint/watchpoint type - error");
            DBG_core_gdb_send_reply(dbg, "E02", true);
            return;
    }

}

static void
DBG_core_gdb_handle_remove_break_watch_mp(DBG *dbg, bool b_err_status)
{
    DBG_core_debug_server *p_dbg_server = dbg->dbg_core.p_dbg_server;

    dbg_core_bp_info_t *p_pending_bp = DBG_core_get_current_process(p_dbg_server)->p_dbg_pending_bp;

    if(!b_err_status)
    {
        DSPLOG_ERROR("Unable to freeze p%x - cannot set/remove breakpoint/watchpoint",
                p_pending_bp->pid);
        DBG_core_gdb_send_reply(dbg, "E02", true);

        return;
    }
    p_dbg_server->gdb_process_freeze_cb = NULL;

    switch(p_pending_bp->bpt_type)
    {
        case DBG_BREAKPOINT_TYPE_HW:
            DSPLOG_INFO("remove hw breakpoint @ addr 0x%x", p_pending_bp->u32_addr);
            DBG_dbp_remove_breakpoint(p_dbg_server,
                    p_pending_bp->pid,
                    p_pending_bp->tid,
                    p_pending_bp->u8_bp_wp_num);
            break;

        case DBG_WATCHPOINT_TYPE_WRITE:
            DSPLOG_DEBUG("remove hw write watchpoint @ addr 0x%x for 0x%x bytes", p_pending_bp->u32_addr,
                    p_pending_bp->u32_wp_num_bytes);
            DBG_dbp_remove_watchpoint(p_dbg_server,
                    p_pending_bp->pid,
                    p_pending_bp->tid,
                    p_pending_bp->u8_bp_wp_num);
            break;

        case DBG_WATCHPOINT_TYPE_READ:
            DSPLOG_DEBUG("remove hw read watchpoint @ addr 0x%x for 0x%x bytes", p_pending_bp->u32_addr,
                    p_pending_bp->u32_wp_num_bytes);
            DBG_dbp_remove_watchpoint(p_dbg_server,
                    p_pending_bp->pid,
                    p_pending_bp->tid,
                    p_pending_bp->u8_bp_wp_num);
            break;
        case DBG_WATCHPOINT_TYPE_ACCESS:
            DSPLOG_DEBUG("remove hw watchpoint @ addr 0x%x for 0x%x bytes", p_pending_bp->u32_addr,
                    p_pending_bp->u32_wp_num_bytes);
            DBG_dbp_remove_watchpoint(p_dbg_server,
                    p_pending_bp->pid,
                    p_pending_bp->tid,
                    p_pending_bp->u8_bp_wp_num);
            break;

        default:
            DSPLOG_ERROR("Unknown breakpoint/watchpoint type - error");
            DBG_core_gdb_send_reply(dbg, "E02", true);
            return;
    }
}

void
DBG_core_gdb_reply_z_packet(DBG *dbg, bool b_err_status)
{
    if(b_err_status)
        DBG_core_gdb_reply_ok(dbg);
    else
        DBG_core_gdb_send_reply(dbg, "E03", true);
}

void
DBG_core_gdb_stop_reply(DBG *dbg, dbg_pid_t pid, dbg_tid_t tid)
{
    DSPLOG_INFO("gdb_stop_reply(): p%x.%x", pid, tid);

    char ch_status_response[16];
    sprintf(ch_status_response, "T05thread:p%x.%x;", pid, tid);

    DBG_core_gdb_send_reply(dbg, ch_status_response, true);
}

void
DBG_core_gdb_watchpoint_hit(DBG *dbg,
                            dbg_pid_t pid,
                            dbg_tid_t tid,
                            uint32_t u32_address,
                            DBG_BPT_WPT_TYPE wp_type)
{
    DSPLOG_INFO("gdb_watchpoint_hit(): p%x.%x @addr 0x%x", pid, tid, u32_address);

    char ch_wp_type[7];

    if(wp_type == DBG_WATCHPOINT_TYPE_WRITE)
        strcpy(ch_wp_type, "watch");
    else if(wp_type == DBG_WATCHPOINT_TYPE_READ)
        strcpy(ch_wp_type, "rwatch");
    else
        strcpy(ch_wp_type, "awatch");

    char ch_status_response[40];
    sprintf(ch_status_response, "T05thread:p%x.%x;%s:%x;", pid, tid, ch_wp_type, u32_address);

    DBG_core_gdb_send_reply(dbg, ch_status_response, true);
}

/*
 * @brief - Handle query packets from GDB
 *
 * We support the following query packets
 *  qSupported          - Usually the first GDB packet. Essential to exchange the list
 *                        of features supported between the stub and the debugger.
 *                        We reply differently to this packet between the standard
 *                        debug server and the multiprocess variant. If we are able to
 *                        enquire the target and get the channel size, we can pass the
 *                        same capacity as the packet size between the GDB and the
 *                        debug server. We attempt to do this here.
 *
 *  QStartNoAckMode     - Respond to GDB that we are in the NO-ACK mode of operation,
 *                        where we don't reply with +/- after receiving each GDB RSP
 *                        packet.
 *
 *
 * Paramerters
 * dbg      Pointer to the debug component*/

void
DBG_core_gdb_handle_query_packet(DBG *dbg)
{
    DBG_core_debug_server *p_dbg_server = dbg->dbg_core.p_dbg_server;
    DEBUG_SERVER_GDB_PACKET *p_gdb_packet = &p_dbg_server->gdb_packet;

    DSPLOG_DEBUG("handle_query_packet(): %s", p_gdb_packet->p_ch_gdb_pack);

    if(strstr(p_gdb_packet->p_ch_gdb_pack, "Supported:"))
    {
        DSPLOG_INFO("handle a qSupported: packet %s", p_gdb_packet->p_ch_gdb_pack);

        /* Try to check if the kernel is booted on the target and the debug process is spawned
         * If yes - issue the initial DBP_CONFIG command to configure the target comms channel
         * */
        DBG_core_gdb_init(dbg);
        DBG_core_gdb_send_reply(dbg, "QStartNoAckMode+;multiprocess+;vContSupported+;no-resumed+;qXfer:libraries:read+;QThreadEvents+;qXfer:osdata:read+", true);
    }
    else if(strstr(p_gdb_packet->p_ch_gdb_pack, "StartNoAckMode"))
    {
        DSPLOG_DEBUG("Processing QStartNoAckMode - reply ok");
        b_noack_mode = true;
        DBG_core_gdb_reply_ok(dbg);
    }
    else if(strstr(p_gdb_packet->p_ch_gdb_pack, "fThreadInfo"))
    {
        DSPLOG_DEBUG("Processing qfThreadInfo");
        DBG_core_gdb_handle_qthread_info(dbg);
    }
    else if(strstr(p_gdb_packet->p_ch_gdb_pack, "sThreadInfo"))
    {
        DSPLOG_DEBUG("Processing qsThreadInfo");
        DBG_core_gdb_handle_qthread_info(dbg);
    }
    else if(strstr(p_gdb_packet->p_ch_gdb_pack, "Attached:"))
    {
        DSPLOG_DEBUG("Processing qAttached");
        DBG_core_gdb_handle_qAttached(dbg);
    }
    else if(strstr(p_gdb_packet->p_ch_gdb_pack, "Offsets"))
    {
        DSPLOG_DEBUG("Processing qOffsets");
        DBG_core_gdb_handle_qOffsets(dbg);
    }
    else if(strstr(p_gdb_packet->p_ch_gdb_pack, "Symbol"))
    {
        DSPLOG_DEBUG("Processing qSymbol - No symbol lookup required - reply ok");
        DBG_core_gdb_reply_ok(dbg);
    }
    else if(strstr(p_gdb_packet->p_ch_gdb_pack, "Xfer:libraries:read"))
    {
        DSPLOG_DEBUG("Processing qXfer:libraries:read");
        DBG_core_gdb_handle_qXfer_libaries(dbg);
    }
    else if(strstr(p_gdb_packet->p_ch_gdb_pack, "Xfer:osdata:read"))
    {
        DSPLOG_DEBUG("Processing qXfer:osdata:read");
        DBG_core_gdb_handle_qXfer_osdata(dbg);
    }
    else if(strstr(p_gdb_packet->p_ch_gdb_pack, "ThreadEvents"))
    {
        DSPLOG_DEBUG("Processing QThreadEvents");
        DBG_core_gdb_handle_QThread_Events(dbg);
    }
    else
    {
        DSPLOG_ERROR("Query packet unsupported - reply empty packet: %s", p_gdb_packet->p_ch_gdb_pack);
        DBG_core_gdb_reply_no_support(dbg);
    }
}

/*
 * @brief - Handle qXfer:osdata:read packet
 *
 * qXfer:osdata:read::offset,length
 *
 * Access the target's operating system information.
 *
 * Users of gdb often wish to obtain information about the state of the operating system
 * running on the target-for example the list of processes, or the list of open files.
 *
 * Operating system information is retrived from the target via the remote protocol, using
 * 'qXfer' requests. The object name in the request should be 'osdata', and the annex
 * identifies the data to be fetched.
 *
 * When requesting the process list, the annex field in the 'qXfer' request should be
 * 'processes'. The returned data is an XML document. The formal syntax of this document
 * is defined in 'gdb/features/osdata.dtd'.
 *
 * An example document is:
 *
 * <?xml version="1.0"?>
 * <!DOCTYPE target SYSTEM "osdata.dtd">
 * <osdata type="processes">
 *  <item>
 *      <column name="pid">1</column>
 *      <column name="user">root</column>
 *      <column name="command">/sbin/init</column>
 *      <column name="cores">1,2,3</column>
 *  </item>
 * </osdata>
 *
 * Each item should include a column whose name is 'pid'. The value of that column should
 * identify the process on the target. The 'user' and 'command' columns are optional, and will
 * be displayed by gdb. The 'cores' column, if present, should contain a comma-separated
 * list of cores that this process is running on. Target may provide additional columns, which
 * gdb currently ignores.
 * */
static int qxfer_osdata_num_process = 0;

static void
DBG_core_gdb_handle_qXfer_osdata_resp(DBG *dbg, bool b_err_status)
{
    DBG_core_debug_server *p_dbg_server = dbg->dbg_core.p_dbg_server;

    if(!b_err_status)
    {
        /*DBP_CMD_PROCESS_GET_NAME returned false - return error to gdb*/
        DSPLOG_ERROR("get process name returned failure - return error");
        p_dbg_server->gdb_process_get_name_cb = NULL;
        qxfer_osdata_num_process = 0;
        DBG_core_gdb_send_reply(dbg, "E03", true);
        return;
    }

    /*Valid response - check if we have received responses for all the processes in the system*/
    if(--qxfer_osdata_num_process)
        return;

    /*We have received all the responses successfully - Starting constructing the xml message*/

    /*Get the total size required for the xml message*/
    int total_size = 100;
    dbg_core_pid_info_t *p_pid_info = DBG_core_peek_pid_info(&p_dbg_server->pid_info_queue);

    while(p_pid_info != NULL)
    {
        char *p_process_name = DBG_core_process_get_name(p_pid_info);

        if(p_process_name != NULL)
            total_size += 150 + 2*strlen(p_process_name);

        GET_NEXT_PID(p_pid_info);
    }

    char *p_xml_osdata = (char *)malloc(total_size);

    if(p_xml_osdata == NULL)
    {
        DSPLOG_ERROR("qxfer:osdata:read() - Cannot allocate string for xml doc");
        DBG_core_gdb_send_reply(dbg, "E04", true);
        return;
    }

    char *p_doc = p_xml_osdata;

    /*Write l as the first character in the response buffer to indicate no more information will follow*/
    *p_doc++ = 'l';

    /*Insert standard header information*/
    strcpy(p_doc, "<?xml version=\"1.0\"?>\n");
    p_doc = p_xml_osdata + strlen(p_xml_osdata);

    strcpy(p_doc, "<!DOCTYPE target SYSTEM \"osdata.dtd\">\n");
    p_doc = p_xml_osdata + strlen(p_xml_osdata);

    strcpy(p_doc, "<osdata type=\"processes\">\n");
    p_doc = p_xml_osdata + strlen(p_xml_osdata);

    /*Insert per process information*/
    p_pid_info = DBG_core_peek_pid_info(&p_dbg_server->pid_info_queue);

    while(p_pid_info != NULL)
    {
        char *p_process_name = DBG_core_process_get_name(p_pid_info);

        if(p_process_name == NULL)
            continue;

        DSPLOG_INFO("p_process_name %s", p_process_name);

        strcpy(p_doc, " <item>\n");
        p_doc = p_xml_osdata + strlen(p_xml_osdata);

        sprintf(p_doc, "    <column name=\"pid\">%d</column>\n", p_pid_info->pid);
        p_doc = p_xml_osdata + strlen(p_xml_osdata);

        strcpy(p_doc, "    <column name=\"user\">user</column>\n");
        p_doc = p_xml_osdata + strlen(p_xml_osdata);

        char *p_name = DBG_core_xml_escape_text(p_process_name);
        if(p_name == NULL)
        {
            DSPLOG_ERROR("qxfer:osdata:read(): Unable to escape process name %s",
                            p_process_name);
            if(p_xml_osdata)
                free(p_xml_osdata);

            return;
        }

        sprintf(p_doc, "    <column name=\"command\">%s</column>\n", p_name);
        p_doc = p_xml_osdata + strlen(p_xml_osdata);

        strcpy(p_doc, " </item>\n");
        p_doc = p_xml_osdata + strlen(p_xml_osdata);

        GET_NEXT_PID(p_pid_info);
    }

    strcpy(p_doc, "</osdata>");

    DBG_core_gdb_send_reply(dbg, p_xml_osdata, true);
    free(p_xml_osdata);
    return;
}

static void
DBG_core_gdb_handle_qXfer_osdata_int(DBG *dbg, bool b_err_status)
{
    DBG_core_debug_server *p_dbg_server = dbg->dbg_core.p_dbg_server;

    /*DBP_CMD_GET_PID_LIST returned false - cannot proceed - return error*/
    if(!b_err_status)
    {
        DSPLOG_ERROR("Update pid_list returned failure - return error");
        DBG_core_gdb_send_reply(dbg, "E02", true);
        return;
    }

    /* Iterate through all the processes and enqueue DBP_CMD_GET_PROCESS_NAME requests
     * for each process*/
    dbg_core_pid_info_t *p_pid_info = DBG_core_peek_pid_info(&p_dbg_server->pid_info_queue);

    while(p_pid_info != NULL)
    {
        /*Register a callback for the DBP_CMD_GET_PROCESS_NAME command*/
        p_dbg_server->gdb_process_get_name_cb = DBG_core_gdb_handle_qXfer_osdata_resp;

        /*Enqueue DBP_CMD_GET_PROCESS_NAME command*/
        DBG_dbp_process_get_name(p_dbg_server, p_pid_info->pid);

        /*increament the valid process count*/
        qxfer_osdata_num_process++;

        /*Move to the next pid_info*/
        GET_NEXT_PID(p_pid_info);
    }

    return;
}

static bool b_qxfer_osdata_enqueue_get_pid_list = false;

static void
DBG_core_gdb_handle_qXfer_osdata(DBG *dbg)
{
    DBG_core_debug_server *p_dbg_server = dbg->dbg_core.p_dbg_server;

    if(!b_qxfer_osdata_enqueue_get_pid_list)
    {
        DSPLOG_DEBUG("handle_qXfer_osdata_read() packet");

        if (!DBG_core_must_be_running(dbg, "qXfer:osdata:read ", "E01"))
            return;

        DSPLOG_INFO("handle_qXfer_osdata_read(): enqueue get_pid_list");

        p_dbg_server->gdb_get_pid_list_cb = DBG_core_gdb_handle_qXfer_osdata_int;
        DBG_dbp_get_pid_list(p_dbg_server);
    }

    return;
}

/*
 * @brief - Handle qXfer:libraries:read packet
 *
 * qXfer:libraries:read:annex:offset,length
 *
 * Access the target's list of loaded libraries - in a Library
 * List Format. The annex part of the generic qXfer packet must
 * be empty. Targets which maintain a list of libraries in the
 * program's memory do not need to implement this packet;
 * it is designed for platforms where the operating system manages
 * the list of loaded libraries. This packet is not probed by
 * default; the remote stub must request it, by supplying an
 * appropriate qSupported response.
 *
 * Example Library list:
 * A simple memory map, with one loaded library relocated by a single offset,
 * looks like this:
 *
 *  <library-list>
 *      <library name="/lib/libc.so.6">
 *          <segment address="0x10000000"/>
 *       </library>
 *  </library-list>
 *
 *  Another simple memory map, with one loaded library with three allocated sections (.text,
 *  .data, .bss), looks like this:
 *
 *  <library-list>
 *      <library name="sharedlib.o">
 *          <section address="0x10000000"/>
 *          <section address="0x20000000"/>
 *          <section address="0x30000000"/>
 *      </library>
 *  </library-list>
 *
 * */
static void
DBG_core_gdb_qxfer_libraries_int(DBG *dbg, bool b_frozen_err_status __unused);

static void
DBG_core_gdb_qxfer_libraries_resp(DBG *dbg, bool b_err_status);

static void
DBG_core_gdb_handle_qXfer_libaries(DBG *dbg)
{
    DBG_core_debug_server *p_dbg_server = dbg->dbg_core.p_dbg_server;

    /*Core must be running to process this packet*/
    if (!DBG_core_must_be_running(dbg, "qXfer:libraries:read ", "E01"))
        return;

    /*Check if the ptid info is valid */
    if( IS_INVALID_PTID(p_dbg_server->cur_ptid_info) ||
        IS_NULL_PTID(p_dbg_server->cur_ptid_info) ||
        IS_MINUS_ONE_PTID(p_dbg_server->cur_ptid_info)
    )
    {
        DSPLOG_ERROR("gdb_handle_qxfer_libraries_read(): Cannot read libraries for p%x.%x",
                p_dbg_server->cur_ptid_info.pid, p_dbg_server->cur_ptid_info.tid);
        DBG_core_gdb_send_reply(dbg, "E01", true);

        return;
    }

    dbg_core_pid_info_t *p_pid_info = DBG_core_get_current_process(p_dbg_server);

    if(p_pid_info == NULL)
    {
        DSPLOG_ERROR("gdb_handle_qxfer_libraries_read(): Cannot find current process");
        DBG_core_gdb_send_reply(dbg, "E02", true);

        return;
    }

    /*Make sure the process for the thread is frozen*/
    dbg_process_state_t process_state = DBG_core_get_process_state(p_pid_info->pid,
                                        &p_dbg_server->pid_info_queue);

    if(process_state == DBG_PROCESS_INVALID)
    {
        DSPLOG_ERROR("gdb_qxfer_libraries_read(): Process not available p%x", p_pid_info->pid);
        DBG_core_gdb_send_reply(dbg, "E01", true);
        return;
    }

    if( (process_state != DBG_PROCESS_STATE_FROZEN) &&
        (process_state != DBG_PROCESS_STATE_FROZEN_BP))
    {
       p_dbg_server->gdb_process_freeze_cb = DBG_core_gdb_qxfer_libraries_int;

       DBG_dbp_process_freeze(p_dbg_server, p_dbg_server->cur_ptid_info.pid );

       return;
    }

    DBG_core_gdb_qxfer_libraries_int(dbg, true);
}

/*Call back registered with the process_freeeze DBP command
 * */
static void
DBG_core_gdb_qxfer_libraries_int(DBG *dbg, bool b_frozen_err_status)
{
    DBG_core_debug_server *p_dbg_server = dbg->dbg_core.p_dbg_server;

    dbg_core_pid_info_t *p_pid_info = DBG_core_get_current_process(p_dbg_server);

    p_dbg_server->gdb_process_freeze_cb = NULL;

    if(!b_frozen_err_status)
    {
        DSPLOG_ERROR("qxfer_libraries_int(): Cannot freeze process p%x", p_pid_info->pid);
        DBG_core_gdb_send_reply(dbg, "E03", true);
        return;
    }

    p_dbg_server->gdb_process_get_dso_info_cb = DBG_core_gdb_qxfer_libraries_resp;

    /*reset dso_count for the pid*/
    DBG_core_process_set_dso_count(p_pid_info->pid,
            &p_dbg_server->pid_info_queue,
            PID_DSO_NOT_INITIALIZED);

    /*Enqueue DBP_PROCESS_GET_DSO_INFO commands*/
    DBG_dbp_process_get_dso_info(p_dbg_server, p_pid_info->pid);
}

/*Callback registerd with dso_info DBP command*/
static void
DBG_core_gdb_qxfer_libraries_resp(DBG *dbg, bool b_err_status)
{
    DBG_core_debug_server *p_dbg_server = dbg->dbg_core.p_dbg_server;

    /*Gather dso info for all processes*/
    char *p_xml_lib = NULL;

    DSPLOG_INFO("gdb_qxfer_libraries_resp(): callback");

    /*Callback starts here*/
    if(!b_err_status)
    {
        DSPLOG_ERROR("dso_info returned failure - returning error to gdb");
        DBG_core_gdb_send_reply(dbg, "E04", true);
        return;
    }

    dbg_core_pid_info_t *p_pid_info_resp = DBG_core_get_current_process(p_dbg_server);

    int total_size = 0;
    /*for each process (calculate the length required)
     * allocate a string buffer to hold this data
     *
     * strcpy(str, "<library-list version=\"1.0\">\n")
     * for each process(write_dso_information)
     *
     * write_dso_information:
     *      strcpy(str. "    <library name=\"");
     * strcpy (str, "</library-list>\n");
     * */
    int dso_count = DBG_core_process_get_dso_count(p_pid_info_resp->pid, &p_dbg_server->pid_info_queue);

    int dso;
    for(dso = 0; dso < dso_count; dso++)
    {
        char *p_dso_soname = DBG_core_process_get_dso_soname(p_pid_info_resp->pid,
                &p_dbg_server->pid_info_queue,
                dso);

        if(p_dso_soname != NULL)
            total_size += 128 + 6*strlen(p_dso_soname);
    }
    DSPLOG_INFO("qxfer:libraries:read() - size allocated for xml doc 0x%x", total_size);

    p_xml_lib = (char *)malloc(total_size);

    if(p_xml_lib == NULL)
    {
        DSPLOG_ERROR("qxfer:libraries:read() - Cannot allocate string for xml doc");
        DBG_core_gdb_send_reply(dbg, "E04", true);
        return;
    }

    /*emit dso information for the process*/
    char *p_doc = p_xml_lib;

    /*Write l as the first character in the response buffer to indicate no more information will follow*/
    *p_doc++ = 'l';

    strcpy(p_doc, "<library-list version=\"1.0\">\n");
    p_doc = p_xml_lib + strlen(p_xml_lib);

    for(dso = 0; dso < dso_count; dso++)
    {
        dso_info_t *p_dso_info = &p_pid_info_resp->process_dso_info.p_dso_info[dso];

        strcpy(p_doc, "  <library name=\"");
        p_doc = p_xml_lib + strlen(p_xml_lib);

        char *p_name = DBG_core_xml_escape_text(p_dso_info->dso_name);

        if(p_name == NULL)
        {
            DBG_core_gdb_send_reply(dbg, "E04", true);
            if(p_xml_lib)
                free(p_xml_lib);
            return;
        }

        strcpy(p_doc, p_name);
        free(p_name);

        p_doc = p_xml_lib + strlen(p_xml_lib);

        strcpy(p_doc, "\">\n    <segment address=\"");
        p_doc = p_xml_lib + strlen(p_xml_lib);

        sprintf(p_doc, "0x%lx",  (long)p_dso_info->segments[0]);
        p_doc = p_xml_lib + strlen(p_xml_lib);

        strcpy (p_doc, "\"/>\n  </library>\n");
        p_doc = p_xml_lib + strlen (p_xml_lib);
    }

    strcpy(p_doc, "</library-list>\n");

    DBG_core_gdb_send_reply(dbg, p_xml_lib, true);
    free(p_xml_lib);
    return;
}

/*
 * @brief - Handle qOffsets RSP packet
 *
 * qOffsets
 *
 * Get section offsets that the target used when relocating the downloaded image.
 *
 * Reply:
 * Text=xxx;Data=yyy[;Bss=zzz]
 * Relocate the Text section by xxx from its original address. Relocate
 * the Data section by yyy from its original address. If the object file
 * format provides segment information (e.g. elf PT_LOAD program
 * headers), gdb will relocate entire segments by the supplied offsets.
 *
 * Note: while a Bss offset may be included in the response, gdb
 * ignores this and instead applies the Data offset to the Bss section.
 *
 * TextSeg=xxx[;DataSeg=yyy]
 * Relocate the first segment of the object file, which conventionally
 * contains program code, to a starting address of xxx. If DataSeg
 * is specified, relocate the second segment, which conventionally contains
 * modifiable data, to a starting address of yyy. gdb will report
 * an error if the object file does not contain segment information, or
 * does not contain at least as many segments as mentioned in the
 * reply. Extra segments are kept at fixed offsets relative to the last
 * relocated segment.
 *
 * */

static void
DBG_core_gdb_handle_qOffsets(DBG *dbg)
{
    DBG_core_debug_server *p_dbg_server = dbg->dbg_core.p_dbg_server;

    /*Core must be running to process this packet*/
    if (!DBG_core_must_be_running(dbg, "qOffsets", "E01"))
        return;

    /*Get the SBA for the process executables*/
    DBG_dbp_process_get_exe_sba(p_dbg_server, p_dbg_server->cur_ptid_info.pid);

    return;
}

void
DBG_core_gdb_handle_qOffsets_int(DBG *dbg, bool b_err_status)
{
    DBG_core_debug_server *p_dbg_server = dbg->dbg_core.p_dbg_server;

    if(!b_err_status)
    {
        DSPLOG_ERROR("qOffsets for p%d.%d returned failure",
                p_dbg_server->cur_ptid_info.pid, p_dbg_server->cur_ptid_info.tid);

        DBG_core_gdb_send_reply(dbg, "Text=;Data=;Bss=",true);
        return;
    }

    /*Get the offsets for the executable for the current pid*/
    DBG_dbp_process_get_exe_segments(p_dbg_server, p_dbg_server->cur_ptid_info.pid);
}

void
DBG_core_gdb_handle_qOffsets_resp(DBG *dbg,
                                  uint32_t num_segments,
                                  uint32_t *seg_info, bool b_err_status)
{
    DBG_core_debug_server *p_dbg_server = dbg->dbg_core.p_dbg_server;
    char ch_gdb_resp_buf[30];
    if(!b_err_status)
    {
        DSPLOG_ERROR("qOffsets for p%d.%d returned failure",
                p_dbg_server->cur_ptid_info.pid, p_dbg_server->cur_ptid_info.tid);

        DBG_core_gdb_send_reply(dbg, "Text=;Data=;Bss=",true);
        return;
    }

    /* For each segment we have been passed a pair of values, the base address
     * and the size of the segment in bytes.
     * Identify the data segment which contains SBA
     */
    uint32_t i, sba, data_seg = seg_info[2*DATA_SEG_INDEX];
    sba = DBG_core_process_get_sba(p_dbg_server->cur_ptid_info.pid, &p_dbg_server->pid_info_queue);
    for(i = 0; i<num_segments; i++)
    {
        DSPLOG_INFO("qOffsets_resp(): segment[%d] - 0x%x", i, seg_info[2*i]);
        if(sba != 0 && (sba >= seg_info[2*i]) && (sba < (seg_info[2*i] + seg_info[2*i+1])))
            data_seg = seg_info[2*i];
    }

    sprintf(ch_gdb_resp_buf, "TextSeg=%x;DataSeg=%x", seg_info[0], data_seg);
    DSPLOG_INFO("qOffsets reply %s", ch_gdb_resp_buf);

    DBG_core_gdb_send_reply(dbg, ch_gdb_resp_buf, true);
}

/* @brief - Handle qAttached RSP packet
 *
 * qAttached:pid
 *
 * Return an indication of whether the remote server attached to an existing process
 * or created a new process. When the multiprocess protocol extensions are
 * supported (see [multiprocess extensions], page 644), pid is an integer in hexadecimal
 * format identifying the target process. Otherwise, gdb will omit the
 * pid field and the query packet will be simplified as qAttached.
 * This query is used, for example, to know whether the remote process should be
 * detached or killed when a gdb session is ended with the quit command.
 *
 * Reply:
 * 1 The remote server attached to an existing process.
 * 0 The remote server created a new process.
 * E NN A badly formed request or an error was encountered.
 * */

static void
DBG_core_gdb_handle_qAttached(DBG *dbg)
{
    DBG_core_debug_server *p_dbg_server = dbg->dbg_core.p_dbg_server;
    char *p_ch_gdb_pack = p_dbg_server->gdb_packet.p_ch_gdb_pack;

    /*Core must be running to process this packet*/
    if (!DBG_core_must_be_running(dbg, "qAttached ", "E01"))
        return;

    /*get the value of the pid that GDB is trying to attach*/
    p_ch_gdb_pack += strlen("Attached:");
    DSPLOG_INFO("handle qAttached:%s", p_ch_gdb_pack);

    dbg_pid_t pid;
    hexToInt(&p_ch_gdb_pack, (uint32_t *)&pid);

    /*Check if we already know about the pid in question*/
    dbg_core_pid_info_t *p_pid_info = DBG_core_find_pid_info(pid, &p_dbg_server->pid_info_queue);

    if(p_pid_info != NULL)
    {
        if(DBG_core_is_process_created(p_pid_info))
        {
            DSPLOG_INFO("Debug service created a new process");
            DBG_core_gdb_send_reply(dbg, "0", true);
        }
        else
        {
            DSPLOG_INFO("Attach to an existing process");
            DBG_core_gdb_send_reply(dbg, "1", true);
        }
    }
    else
    {
        /* We are not refreshing the pid-list and checking again,
         * as there is no guarentee that the kernel will not
         * throw away the process before we reply - so keep it simple -
         * let GDB know the process doesn't exist and we need to create it*/
        DSPLOG_ERROR("qAttached: The process %d doesn't exist", pid);
        DBG_core_gdb_send_reply(dbg, "E02", true);
    }

    return;
}

static void
DBG_core_gdb_init(DBG *dbg)
{
    DBG_TARGET *p_dbg_target = &dbg->dbg_target_if;

    if(!p_dbg_target->b_channel_configured && DBG_core_is_core_ready(dbg))
    {
        /*If we haven't had a chance to configure the channel - configure now*/
        DSPLOG_INFO("dbg_core_gdb_init(): Configure channel");
        DBG_core_dbp_comms_check(dbg);
    }
}

static void
DBG_core_gdb_send_qthread_info_resp(DBG *dbg)
{
    DBG_core_debug_server *p_dbg_server = dbg->dbg_core.p_dbg_server;

    static dbg_core_thread_info_t *p_thread_info = NULL;
    static bool b_thread_info_initialized = false;

    char ch_thread_resp[12];

    dbg_core_pid_info_t *p_pid_info = DBG_core_get_current_process(p_dbg_server);

    if(!b_thread_info_initialized)
    {
        p_thread_info = DBG_core_peek_tid_info(&p_pid_info->thread_info_queue);
        b_thread_info_initialized = true;
    }

    /*No more threads to send - Mark this packet as the last of the transaction*/
    if(p_thread_info == NULL)
    {
        DBG_core_gdb_send_reply(dbg, "l", true);
        b_thread_info_initialized = false;
        return;
    }
    ch_thread_resp[0] = 'm';

    DBG_core_write_ptid(p_pid_info->pid, p_thread_info->tid, &ch_thread_resp[1]);
    DBG_core_gdb_send_reply(dbg, ch_thread_resp, true);

    GET_NEXT_TID(p_thread_info);
    return;
}

/*Assemble and respond to qfThreadInfo/qsThreadInfo RSP packets*/
static void
DBG_core_gdb_qthread_info_resp(DBG *dbg, bool b_err_status)
{
    DBG_core_debug_server *p_dbg_server = dbg->dbg_core.p_dbg_server;

    if(!b_err_status)
    {
        DSPLOG_ERROR("Process 0x%x cannot be frozen", p_dbg_server->cur_ptid_info.pid);

        /*Make hg_ptid point to an invalid ptid*/
        copy_ptid(&p_dbg_server->hg_ptid, &gdb_invalid_ptid);

        /*Reply error*/
        DBG_core_gdb_send_reply(dbg, "E02", true);

        return;
    }
    p_dbg_server->gdb_process_freeze_cb = NULL;

    /* Concoct a GDB response only when invoked via DBG_core_gdb_handle_qthread_info
     * */
    DBG_core_gdb_send_qthread_info_resp(dbg);
}

/*@brief Handle a qfThreadInfo/qsThreadInfo RSP packet
 *
 * qfThreadInfo/qsThreadInfo
 * Obtain a list of all active thread IDs from the target (OS). Since there may be
 * too many active threads to fit into one reply packet, this query works iteratively:
 * it may require more than one query/reply sequence to obtain the entire list of
 * threads. The first query of the sequence will be the qfThreadInfo query;
 * subsequent queries in the sequence will be the qsThreadInfo query.
 * NOTE: This packet replaces the qL query (see below).
 *
 * Reply:
 * m thread-id
 * A single thread ID
 *
 * m thread-id,thread-id...
 * a comma-separated list of thread IDs
 *
 * l (lower case letter L) denotes end of list.
 *
 * In response to each query, the target will reply with a list of one or more thread
 * IDs, separated by commas. gdb will respond to each reply with a request for
 * more thread ids (using the qs form of the query), until the target responds
 * with l (lower-case ell, for last).
 *
 * Note: gdb will send the qfThreadInfo query during the initial connection with
 * the remote target, and the very first thread ID mentioned in the reply will be
 * stopped by gdb in a subsequent message. Therefore, the stub should ensure that
 * the first thread ID in the qfThreadInfo reply is suitable for being stopped by
 * gdb.
*/
static void
DBG_core_gdb_handle_qthread_info(DBG *dbg)
{
    DBG_core_debug_server *p_dbg_server = dbg->dbg_core.p_dbg_server;

    /*Core must be running to process this packet*/
    if (!DBG_core_must_be_running(dbg, "qThreadInfo ", "E01"))
        return;

    /*Check if the ptid info is valid */
    if( IS_INVALID_PTID(p_dbg_server->cur_ptid_info) ||
        IS_NULL_PTID(p_dbg_server->cur_ptid_info) ||
        IS_MINUS_ONE_PTID(p_dbg_server->cur_ptid_info)
    )
    {
        DSPLOG_ERROR("gdb_qthread_info_mp(): Cannot write memory for p%x.%x",
                p_dbg_server->cur_ptid_info.pid, p_dbg_server->cur_ptid_info.tid);
        DBG_core_gdb_send_reply(dbg, "E01", true);

        return;
    }

    /*Make sure the process for the thread is frozen*/
    dbg_process_state_t process_state = DBG_core_get_process_state(p_dbg_server->cur_ptid_info.pid,
                                        &p_dbg_server->pid_info_queue);

    if(process_state == DBG_PROCESS_INVALID)
    {
        DSPLOG_ERROR("gdb_read_all_registers_mp(): Process not available p%x", p_dbg_server->cur_ptid_info.pid);
        DBG_core_gdb_send_reply(dbg, "E01", true);
        return;
    }

    if( (process_state != DBG_PROCESS_STATE_FROZEN) &&
        (process_state != DBG_PROCESS_STATE_FROZEN_BP))
    {
       p_dbg_server->gdb_process_freeze_cb = DBG_core_gdb_qthread_info_resp;

       DBG_dbp_process_freeze(p_dbg_server, p_dbg_server->cur_ptid_info.pid );

       return;
    }

    /*If no commands are enqueued, invoke the callback here*/
    DBG_core_gdb_qthread_info_resp(dbg, true);
}

/*
 * @brief - Support QThreadEvents packet
 *
 * QThreadEvents:1
 * QThreadEvents:0
 *
 * Enable (QThreadEvents:1) or disable (QThreadEvents:0) reporting of
 * thread create and exit events. See [thread create event], page 629, for the reply
 * specifications. For example, this is used in non-stop mode when gdb stops a
 * set of threads and synchronously waits for the their corresponding stop replies.
 * Without exit events, if one of the threads exits, gdb would hang forever not
 * knowing that it should no longer expect a stop for that same thread. gdb does
 * not enable this feature unless the stub reports that it supports it by including
 * QThreadEvents+ in its qSupported reply.
 *
 * Reply:
 * OK The request succeeded.
 * E nn An error occurred. The error number nn is given as hex digits.
 * '' An empty reply indicates that QThreadEvents is not supported
 * by the stub.
 *
 * Use of this packet is controlled by the set remote thread-events command
 * (see Section 20.4 [Remote Configuration], page 270).*/

static void
DBG_core_gdb_handle_QThread_Events(DBG *dbg)
{
    DBG_core_debug_server *p_dbg_server = dbg->dbg_core.p_dbg_server;
    DEBUG_SERVER_GDB_PACKET *p_gdb_packet = &p_dbg_server->gdb_packet;

    /*Core must be running to process this packet*/
    if (!DBG_core_must_be_running(dbg, "qThreadInfo ", "E01"))
        return;

    /*Read the set or disable variable*/
    char *thread_event = p_gdb_packet->p_ch_gdb_pack + strlen("QThreadEvents:");

    if (strcmp(thread_event, "1") == 0)
        p_dbg_server->b_notify_thread_events = true;
    else if(strcmp(thread_event, "0") == 0)
        p_dbg_server->b_notify_thread_events = false;
    else
    {
        DSPLOG_ERROR("Unknown thread notfiy event %s", thread_event);
        DBG_core_gdb_send_reply(dbg, "E02", true);
        return;
    }

    DBG_core_gdb_reply_ok(dbg);
}


void
DBG_core_gdb_handle_continue(DBG *dbg)
{
    DSPLOG_INFO("gdb_handle_continue: We don't support c packet - gdb will use vCont;c");
    DBG_core_gdb_reply_no_support(dbg);
}

static int int_stop_issued = 0;

static void
DBG_core_gdb_handle_stop_response(DBG *dbg, bool b_err_status)
{
    DBG_core_debug_server *p_dbg_server = dbg->dbg_core.p_dbg_server;

    /* Process cb is a NULL pointer - so must have been invoked by the
     * stop_handler. No process attached to the debugger is actually
     * frozen - Reply ok
     * */
    if(p_dbg_server->gdb_process_freeze_cb == NULL)
    {
        DSPLOG_DEBUG("stop_response: No process_freeze command sent");
        goto gdb_handle_stop_response_return;
    }

    /*Got an error when attempting to freeze a process.
     * This stop response handler doesn't know the number of processes
     * which has a FREEZE request. So this handler can't reliably know
     * if there will be anymore successful FROZEN callbacks.
     *
     * So the safest option is to reply OK to GDB.
     * */
    if(!b_err_status)
    {
        DSPLOG_ERROR("gdb_handle_stop(): Error while freezing a process - reply ok");
        goto gdb_handle_stop_response_return;
    }

    /* A process is frozen. We need not wait for all the processes to be frozen,
     * as the response packet can hold only one response
     * */
    dbg_core_pid_info_t *p_pid_info = DBG_core_peek_pid_info(&p_dbg_server->pid_info_queue);

    while(p_pid_info != NULL)
    {
        if(DBG_core_is_process_attached(p_pid_info) &&
           (IS_PROCESS_FROZEN(p_pid_info)))
        {
            break;
        }
        GET_NEXT_PID(p_pid_info);
    }

gdb_handle_stop_response_return:
    p_dbg_server->gdb_process_freeze_cb = NULL;
    int_stop_issued = 0;
    DBG_core_gdb_send_signal(dbg, DBG_GDB_SIGNAL_INT);

    return;
}

void
DBG_core_gdb_handle_stop(DBG *dbg)
{
    DBG_core_debug_server *p_dbg_server = dbg->dbg_core.p_dbg_server;

    if(!int_stop_issued)
    {
        DSPLOG_INFO("gdb_handle_stop: Process interrupt - Freeze all processes");

        dbg_core_pid_info_t *p_pid_info = DBG_core_peek_pid_info(&p_dbg_server->pid_info_queue);

        if(!DBG_core_must_be_running(dbg, "stop", "E01"))
            return;

        /*Freeze all processes attached to the debugger*/
        p_dbg_server->gdb_process_freeze_cb = NULL;
        while(p_pid_info != NULL)
        {
            if(DBG_core_is_process_attached(p_pid_info))
            {
                p_dbg_server->gdb_process_freeze_cb = DBG_core_gdb_handle_stop_response;

                DBG_dbp_process_freeze(p_dbg_server, p_pid_info->pid);
            }

            GET_NEXT_PID(p_pid_info);
        }

        int_stop_issued = 1;

        if(p_dbg_server->gdb_process_freeze_cb == NULL)
            DBG_core_gdb_handle_stop_response(dbg, true);
    }

    return;
}

/*
 * 'D'
 * 'D;pid' The first form of the packet is used to detach gdb
 * from the remote system. It is sent to the remote target before
 * gdb disconnects via the detach command.
 *
 * The second form, including a process ID, is used when multiprocess protocol
 * extensions are enabled, to detach only a specific process. The pid is
 * specified as a big-endian hex string.
 *
 * Reply:
 * 'OK' for success
 * 'E NN' for an error
 * */

void
DBG_core_gdb_handle_detach(DBG *dbg)
{
    DBG_core_debug_server *p_dbg_server = dbg->dbg_core.p_dbg_server;

    /*Core must be running to process this packet*/
    if (!DBG_core_must_be_running(dbg, "qThreadInfo ", "E01"))
        return;

    /*Get the pid*/
    char *p_ch_gdb_pack = p_dbg_server->gdb_packet.p_ch_gdb_pack;
    char *p_ch = &p_ch_gdb_pack[1];

    uint32_t u32_pid_value = 0;

    hexToInt(&p_ch, &u32_pid_value);

    if(u32_pid_value == 0)
    {
        DSPLOG_ERROR("Invalid pid information in vAttach");
        DBG_core_gdb_send_reply(dbg, "E01", true);
        return;
    }

    dbg_pid_t pid = (dbg_pid_t)u32_pid_value;

    dbg_core_pid_info_t *p_pid_info = DBG_core_find_pid_info(pid, &p_dbg_server->pid_info_queue);

    if(p_pid_info != NULL)
    {
        /*Enqueue detach command*/
        if(DBG_core_is_process_attached(p_pid_info))
            DBG_dbp_process_detach(p_dbg_server, pid);

        /* Remove all breakpoints
         * FIXME: Will there be any breakpoint in flight ??*/
        dbg_core_bp_info_t *p_bp_info = DBG_core_peek_bp_info(&p_pid_info->bp_info_queue);

        while(p_bp_info != NULL)
        {
            if(p_bp_info->bpt_type <= DBG_BREAKPOINT_TYPE_HW)
                DBG_dbp_remove_breakpoint(p_dbg_server, p_bp_info->pid, p_bp_info->tid, p_bp_info->u8_bp_wp_num);
            else
                DBG_dbp_remove_watchpoint(p_dbg_server, p_bp_info->pid, p_bp_info->tid, p_bp_info->u8_bp_wp_num);

            GET_NEXT_BP(p_bp_info);
        }

        if(p_pid_info->process_state == DBG_PROCESS_STATE_FROZEN ||
           p_pid_info->process_state == DBG_PROCESS_STATE_FROZEN_BP)
            DBG_dbp_process_unfreeze(p_dbg_server, pid);
    }

    DBG_core_gdb_reply_ok(dbg);
}

void
DBG_core_gdb_handle_singlestep(DBG *dbg __unused)
{
    DSPLOG_INFO("gdb_handle_sstep: We don't support s packet - gdb will use vCont;s");
    DBG_core_gdb_reply_no_support(dbg);
}

void
DBG_core_gdb_syscall_handle_reply(DBG *dbg)
{
    DBG_core_gdb_reply_no_support(dbg);
}

void
DBG_core_gdb_process_exited(DBG *dbg, dbg_pid_t pid)
{
    char ch_process_exit_resp[20];

    sprintf(ch_process_exit_resp, "W09;process:%x", pid);
    DBG_core_gdb_send_reply(dbg, ch_process_exit_resp, true);
}

void
DBG_core_gdb_thread_created(DBG *dbg, dbg_pid_t pid __unused, dbg_tid_t tid __unused)
{
    char ch_thread_created_response[7];

    sprintf(ch_thread_created_response, "create");
    DBG_core_gdb_send_reply(dbg, ch_thread_created_response, true);
}

void
DBG_core_gdb_thread_exited(DBG *dbg, dbg_pid_t pid __unused, dbg_tid_t tid)
{
    char ch_thread_exited_response[10];

    sprintf(ch_thread_exited_response, "w05;%02x", tid);
    DBG_core_gdb_send_reply(dbg, ch_thread_exited_response, true);

}

void
DBG_core_gdb_notify_library_change(DBG *dbg)
{
    DBG_core_gdb_send_reply(dbg, "T05library:;", true);
}

void
DBG_core_gdb_notify_process_terminate(DBG *dbg, dbg_pid_t pid, DBP_FAULT_ID fault)
{
    char ch_process_terminate[20];
    DBG_GDB_SIGNALS signal=DBG_GDB_SIGNAL_NONE;
    switch(fault)
    {
        case DBP_FAULT_CODE_UNMAPPED:
        case DBP_FAULT_MISALIGNED_PC:
        case DBP_FAULT_INVALID_INSN:
            signal = DBG_GDB_SIGNAL_ILL;
            break;

        case DBP_FAULT_INVALID_SVC:
        case DBP_FAULT_DATA_UNMAPPED:
        case DBP_FAULT_BUS:
            signal = DBG_GDB_SIGNAL_BUS;
            break;

        case DBP_FAULT_UNKNOWN:
        default:
            signal = DBG_GDB_SIGNAL_SEGV;
            break;
    }
    sprintf(ch_process_terminate, "X%x;process:%x",signal, pid);
    DBG_core_gdb_send_reply(dbg, ch_process_terminate, true);
}
