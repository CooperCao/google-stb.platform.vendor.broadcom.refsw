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

#include "libdspcontrol/CHIP.h"
#include "libdspcontrol/DBG.h"
#include "libdspcontrol/DSPLOG.h"
#include "libdspcontrol/COMMON.h"

#include "libdspcontrol/src/DSP_octave_maestro.h"
#include "DBG_interfaces.h"
#include "DBG_core_debug_server_common.h"

#if !(FEATURE_IS(DBG_CORE, DEBUG_SERVER) || FEATURE_IS(DBG_CORE, DEBUG_SERVER_MP))
#  error "This source is for the debug_server variant of the DBG core"
#endif
#if !FEATURE_IS(LIBC, HOSTED)
#  error "This compilation unit requires support for dynamic memory allocation"
#endif

#if (FEATURE_IS(DBG_CORE, DEBUG_SERVER))
#include "DBG_core_debug_server.h"
#include "DBG_core_debug_server_gdb.h"
#include "DBG_core_debug_server_gdb_dbp.h"
#else
#include "DBG_core_debug_server_mp.h"
#include "DBG_core_debug_server_gdb_mp.h"
#endif
#include "DBG_core_debug_server_gdb_utils.h"
#include "DBG_core_debug_server_gdb_syscall.h"

/****************************************************************************
 *              GLOBAL VARIABLES/ARRAYS/CONSTANTS
 ****************************************************************************/
bool b_noack_mode = false;
bool b_killed = false;
const char pos_reply = '+';
const char neg_reply = '-';

void
DBG_core_gdb_pack_init(DEBUG_SERVER_GDB_PACKET *p_gdb_packet)
{
    p_gdb_packet->b_gdb_pack_valid  = false;
    p_gdb_packet->packet_type       = DBG_GDB_PACKET_TYPE_NONE;
    p_gdb_packet->p_ch_gdb_pack = "\0";
}

bool
DBG_core_support_mp(void)
{
#if FEATURE_IS(DEBUG_CORE, DEBUG_SERVER)
    return false;
#elif FEATURE_IS(DEBUG_CORE, DEBUG_SERVER_MP)
    return true;
#endif
}

void
DBG_service_gdb_packet(DBG *dbg)
{
    DBG_CORE *dbg_core = &dbg->dbg_core;
    DBG_core_debug_server *p_dbg_server = dbg_core->p_dbg_server;

    /*Gather GDB packet from host*/
    DBG_core_gdb_get_packet(&dbg->dbg_host_if, p_dbg_server);

    /*If a valid packet is received - handle this packet*/
    if(p_dbg_server->gdb_packet.b_gdb_pack_valid)
    {
        if(p_dbg_server->gdb_packet.packet_type == DBG_GDB_PACKET_TYPE_PACKET)
            DBG_core_gdb_handle_packet(dbg);
        else if (p_dbg_server->gdb_packet.packet_type == DBG_GDB_PACKET_TYPE_INTERRUPT)
            DBG_core_gdb_handle_interrupt(dbg);
    }
}

bool
DBG_core_is_core_ready(DBG *dbg)
{
    DBG_TARGET *p_dbg_target = &dbg->dbg_target_if;

    DSP_CORES_BITMAP cores;
    uint32_t u32_sys_flg;
    DSP_enabledStatus(p_dbg_target->dsp, &cores);

    if(cores & (1 << p_dbg_target->dsp_core))
    {
        /* Check if the core is loaded -
         * check for the misc sys flag bit0 */
        u32_sys_flg = DSP_readSharedRegister(p_dbg_target->dsp, MISC_BLOCK(p_dbg_target->dsp, p_dbg_target->dsp_core, CORECTRL_SYS_FLG0_STATUS));
        return u32_sys_flg & 0x01;
    }
    return false;
}

void
DBG_core_gdb_handle_interrupt(DBG *dbg)
{
#if (FEATURE_IS(DBG_CORE, DEBUG_SERVER))
    DBG_core_debug_server *p_dbg_server = dbg->dbg_core.p_dbg_server;
    DSPLOG_DEBUG("DBG(%u): Issue stop", dbg->dbg_core.id);

    if(DBG_core_gdb_check_is_core_alive(dbg))
    {
        DBG_core_stop(dbg);
        p_dbg_server->core_state = DBG_CORE_STATE_STOPPED;

        DSPLOG_DEBUG("gdb_handle_interrupt(): core_state STOPPED");
        DBG_core_gdb_send_signal(dbg, DBG_GDB_SIGNAL_INT);
    }
    else
    {
        DSPLOG_ERROR("gdb_handle_interrupt(): core not enabled");
        DBG_core_gdb_send_signal(dbg, DBG_GDB_SIGNAL_PWR);
    }
#else
    DSPLOG_DEBUG("DBG(%u): Issue stop", dbg->dbg_core.id);

    DBG_core_gdb_handle_stop(dbg);
#endif
}

void
DBG_core_gdb_get_packet(DBG_HOST *p_dbg_host, DBG_core_debug_server *p_dbg_server)
{
    DEBUG_SERVER_GDB_BUFFER_T *p_gdb_recv_buffer = &p_dbg_server->buffer_gdb_recv_pack;

    /*Space available in buffer*/
    size_t inout_len = p_gdb_recv_buffer->s_max_buf_size;
    char *p_ch_gdb_cmd_buffer;
    uint8_t u8_comp_chksum = 0;
    char ch;
    uint8_t u8_stored_chksum;
    size_t sh = sizeof(char);

    /*Get bytes from the host socket*/
    DBG_hostReceiveData(p_dbg_host, (uint8_t *)p_gdb_recv_buffer->p_ch_buf, &inout_len);

    p_gdb_recv_buffer->p_ch_buf[inout_len] = '\0';

    /*Valid bytes received*/
    if(inout_len > 0)
    {
        DSPLOG_INFO("DBG: Received GDB packet:%s size:%d", p_gdb_recv_buffer->p_ch_buf, (uint8_t)inout_len);
        p_ch_gdb_cmd_buffer = p_gdb_recv_buffer->p_ch_buf;

        if(*p_ch_gdb_cmd_buffer == GDB_INTERRUPT)
        {
            DSPLOG_INFO("gdb_get_packet(): handle interrupt");
            p_dbg_server->gdb_packet.p_ch_gdb_pack    = p_ch_gdb_cmd_buffer;
            p_dbg_server->gdb_packet.b_gdb_pack_valid = true;
            p_dbg_server->gdb_packet.packet_type      = DBG_GDB_PACKET_TYPE_INTERRUPT;
            return;
        }
        /* Look for the start of the packet symbol */
        while(*p_ch_gdb_cmd_buffer++ != '$');

        p_dbg_server->gdb_packet.p_ch_gdb_pack = p_ch_gdb_cmd_buffer;

        while(*p_ch_gdb_cmd_buffer != '#')
        {
            u8_comp_chksum += *(uint8_t *)p_ch_gdb_cmd_buffer++;
        }
        ch = *++p_ch_gdb_cmd_buffer;
        u8_stored_chksum = handle_hex_char(ch)<<4;
        ch = *++p_ch_gdb_cmd_buffer;
        u8_stored_chksum += handle_hex_char(ch);

        if(strncmp(p_dbg_server->gdb_packet.p_ch_gdb_pack, "qSupported", strlen("qSupported")) == 0)
        {
            DSPLOG_INFO("received qSupported - reset noack_mode");
            b_noack_mode = false;
        }

        if(u8_comp_chksum == u8_stored_chksum)
        {
            p_dbg_server->gdb_packet.b_gdb_pack_valid = true;
            p_dbg_server->gdb_packet.packet_type = DBG_GDB_PACKET_TYPE_PACKET;
            if(!b_noack_mode)
                DBG_hostSendData(p_dbg_host, (uint8_t *)&pos_reply, &sh);
        }
        else
        {
            DSPLOG_ERROR("ERROR: Invalid GDB Packet u8_comp_chksum:%x u8_stored_chksum:%x",
                    u8_comp_chksum, u8_stored_chksum);
            p_dbg_server->gdb_packet.b_gdb_pack_valid = false;
            if(!b_noack_mode)
                DBG_hostSendData(p_dbg_host, (uint8_t *)&neg_reply, &sh);
        }
    }
}

/*Sends GDB response
 * DBG*/
void
DBG_core_gdb_send_reply(DBG *dbg, char *p_in_pkt_data, bool b_copy)
{
    DBG_core_debug_server *p_dbg_server             = dbg->dbg_core.p_dbg_server;
    DEBUG_SERVER_GDB_BUFFER_T *p_gdb_resp_buffer    = &p_dbg_server->buffer_gdb_send_pack;
    char *p_pkt_buf = p_gdb_resp_buffer->p_ch_buf;  /* Pointer to the response buffer */
    size_t data_to_send;

    p_pkt_buf[0] = '$';

    if(b_copy)
        strcpy(&p_pkt_buf[1], p_in_pkt_data);

    DBG_core_add_chksum(&p_pkt_buf[1], strlen(&p_pkt_buf[1]));

    data_to_send = strlen(p_pkt_buf);
    /*Send the packet back*/
    DSPLOG_DETAIL("gdb_send_reply(): response:%s", p_pkt_buf);
    DBG_hostSendData(&dbg->dbg_host_if, (uint8_t *)p_pkt_buf, &data_to_send);
}

void
DBG_core_gdb_send_signal(DBG *dbg, DBG_GDB_SIGNALS sigval)
{
    char ch[3];
    sprintf(ch, "S%02x", sigval);
    DSPLOG_INFO("gdb_send_signal(): sending signal %s", ch);
    DBG_core_gdb_send_reply(dbg, ch, true);
}

void
DBG_core_gdb_reply_ok(DBG *dbg)
{
    DBG_core_gdb_send_reply(dbg, "OK", true);
}

void
DBG_core_gdb_reply_no_support(DBG *dbg)
{
    DBG_core_gdb_send_reply(dbg, "", true);
}

void
DBG_core_gdb_enable_extended_mode(DBG *dbg)
{
    DBG_core_debug_server *p_dbg_server = dbg->dbg_core.p_dbg_server;

    p_dbg_server->b_gdb_extended_mode = true;
    DBG_core_gdb_reply_ok(dbg);
}

/******************************************************************************
 * FUNCTION           :  DBG_core_gdb_handle_packet
 * ARGUMENTS          :
 * dbg                :  Pointer to the DBG structure
 *
 * Return             :  None
 * Main GDB packet handling function - Parses first gdb packet character following
 * the $ symbol and dispatches
 ******************************************************************************/
void
DBG_core_gdb_handle_packet(DBG *dbg)
{
    DBG_core_debug_server   *p_dbg_server = dbg->dbg_core.p_dbg_server;
    DEBUG_SERVER_GDB_PACKET *p_gdb_packet = &p_dbg_server->gdb_packet;

    char *p_ch_gdb_cmd = NULL;

    /*Pretty printing of incoming GDB packet*/
    char *p_ch_gdb_cmd_in = p_gdb_packet->p_ch_gdb_pack;
    char *p_ch_gdb_cmd_print;
    char end_ch;

    MALLOC_OR_FAIL(p_ch_gdb_cmd, char *, DBG_GDB_BUFFER_SIZE, "DBG:");
    p_ch_gdb_cmd_print = p_ch_gdb_cmd;

#if 0
    if(*p_ch_gdb_cmd_in == 'M' || *p_ch_gdb_cmd_in == 'X')
        /*Don't print values in the mem write packet*/
        end_ch = ':';
    else
#endif
    end_ch = '#';

    while(*p_ch_gdb_cmd_in != end_ch)
        *p_ch_gdb_cmd++ = *p_ch_gdb_cmd_in++;

    *p_ch_gdb_cmd = '\0';

    DSPLOG_INFO("DBG: GDB handle packet %s",p_ch_gdb_cmd_print);
    free(p_ch_gdb_cmd_print);

    /*Handle GDB packet*/
    switch((*(p_gdb_packet->p_ch_gdb_pack++)))
    {
        case '?':
            DBG_core_gdb_handle_send_status_update(dbg);
            break;

        case '!':
            DBG_core_gdb_enable_extended_mode(dbg);
            break;

        case 'c':
            DBG_core_gdb_handle_continue(dbg);
            break;

        case 'D':
            DBG_core_gdb_handle_detach(dbg);
            break;

        case 'F':
            DBG_core_gdb_syscall_handle_reply(dbg);
            break;

        case 'q':
        case 'Q':
            DBG_core_gdb_handle_query_packet(dbg);
            break;

        case 'g':
            DBG_core_gdb_handle_read_all_registers(dbg);
            break;

        case 'H':
            DBG_core_gdb_handle_h_packet(dbg);
            break;

        case 'm':
            DBG_core_gdb_handle_read_memory(dbg);
            break;

        case 'M':
            DBG_core_gdb_handle_write_memory(dbg);
            break;

        case 'p':
            DBG_core_gdb_handle_read_register(dbg);
            break;

        case 'P':
            DBG_core_gdb_handle_write_register(dbg);
            break;

        case 's':
            DBG_core_gdb_handle_singlestep(dbg);
            break;

        case 'T':
            DBG_core_gdb_handle_T_packet(dbg);
            break;

        case 'v':
            DBG_core_gdb_handle_v_packet(dbg);
            break;

        case 'Z':
            DBG_core_gdb_handle_z_packet(dbg, DBG_BPT_WPT_INSERT);
            break;

        case 'z':
            DBG_core_gdb_handle_z_packet(dbg, DBG_BPT_WPT_REMOVE);
            break;

        default:
            DSPLOG_ERROR("Packet %s unsupported - reply empty packet", (p_gdb_packet->p_ch_gdb_pack - sizeof(char)));
            DBG_core_gdb_reply_no_support(dbg);
            break;
    }

    /*reset gdb_packet*/
    DBG_core_gdb_pack_init(p_gdb_packet);
}
