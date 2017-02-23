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

#include <inttypes.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "libdspcontrol/CHIP.h"
#include "libdspcontrol/DSPLOG.h"

#include "fp_sdk_config.h"

#include "DBG_core_debug_server_dbp_mp.h"
#include "DBG_core_mp_dbp_msg.h"
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

static dbg_dbp_msg_t*
DBG_core_dbp_msg_alloc(uint16_t u16_msg_len);

static void
DBG_core_enqueue_dbp_msg(dbg_dbp_msg_queue_t *p_dbp_msg_queue, dbg_dbp_msg_t *p_dbp_msg);

static bool
DBG_core_is_queue_empty(dbg_dbp_msg_queue_t *p_dbp_msg_queue);

static dbg_dbp_msg_t*
DBG_core_dbp_msg_alloc(uint16_t u16_msg_len)
{
    dbg_dbp_msg_t *p_dbp_msg = NULL;

    DSPLOG_DEBUG("dbp_msg_alloc(): Allocate a dbp_msg");

    if( (p_dbp_msg = (dbg_dbp_msg_t*) malloc(sizeof(dbg_dbp_msg_t))) == NULL )
    {
        DSPLOG_ERROR("Allocation of dbp_msg FAILED");
        return NULL;
    }

    /*Allocate dbp message buffer big enough to hold the dbp_msg len*/
    if( (p_dbp_msg->p_u8_buf = (uint8_t *)malloc(u16_msg_len)) == NULL )
    {
        DSPLOG_ERROR("Allocation of dbp_msg buffer FAILED");
        free(p_dbp_msg);
        return NULL;
    }

    /*Initialize other parameters*/
    p_dbp_msg->u16_buf_size = u16_msg_len;
    p_dbp_msg->p_dbp_msg_next = NULL;

    return p_dbp_msg;
}

void
DBG_core_dbp_msg_dealloc(dbg_dbp_msg_t *p_dbg_dbp_msg)
{
    free(p_dbg_dbp_msg->p_u8_buf);
    free(p_dbg_dbp_msg);
}

static void
DBG_core_enqueue_dbp_msg(dbg_dbp_msg_queue_t *p_dbp_msg_queue, dbg_dbp_msg_t *p_dbp_msg)
{
    DSPLOG_DEBUG("Enqueue dbp_msg 0x%" PRIxPTR " in the queue 0x%" PRIxPTR,
                 (uintptr_t) p_dbp_msg, (uintptr_t) p_dbp_msg_queue);
    //If queue is empty
    if(p_dbp_msg_queue->p_dbp_msg_head == NULL)
    {
        p_dbp_msg->p_dbp_msg_next = NULL;
        p_dbp_msg_queue->p_dbp_msg_head = p_dbp_msg_queue->p_dbp_msg_tail = p_dbp_msg;
        return;
    }
    p_dbp_msg_queue->p_dbp_msg_tail->p_dbp_msg_next = p_dbp_msg;
    p_dbp_msg_queue->p_dbp_msg_tail = p_dbp_msg;
    p_dbp_msg_queue->p_dbp_msg_tail->p_dbp_msg_next = NULL;
}

dbg_dbp_msg_t*
DBG_core_dequeue_dbp_msg(dbg_dbp_msg_queue_t *p_dbp_msg_queue)
{
    DSPLOG_DEBUG("Dequeue dbp_msg from the queue 0x%" PRIxPTR, (uintptr_t) p_dbp_msg_queue);
    if(p_dbp_msg_queue->p_dbp_msg_head == NULL)
        return NULL;

    dbg_dbp_msg_t *p_dbp_msg;

    p_dbp_msg = p_dbp_msg_queue->p_dbp_msg_head;
    p_dbp_msg_queue->p_dbp_msg_head = p_dbp_msg_queue->p_dbp_msg_head->p_dbp_msg_next;
    return p_dbp_msg;
}

static bool
DBG_core_is_queue_empty(dbg_dbp_msg_queue_t *p_dbp_msg_queue)
{
    if(p_dbp_msg_queue->p_dbp_msg_head == NULL)
        return true;

    return false;
}

DBG_DBP_ERR_CODE
DBG_core_create_and_enqueue_dbp_msg(dbg_dbp_msg_queue_t *p_dbp_msg_queue, dbp_command_t *p_dbp_command)
{
    /*Allocate a DBP message*/
    dbg_dbp_msg_t *p_dbp_msg = DBG_core_dbp_msg_alloc(p_dbp_command->dbp_header.msglen);

    if(p_dbp_msg == NULL)
    {
        DSPLOG_ERROR("Cannot allocate command message buffer");
        return DBG_DBP_ERR_CODE_CANNOT_ALLOCATE_BUFFER;
    }

    /*Copy the command to the message buffer*/
    memcpy(p_dbp_msg->p_u8_buf, p_dbp_command, p_dbp_command->dbp_header.msglen);

    /*Enqueue the DBP message to the command queue*/
    DBG_core_enqueue_dbp_msg(p_dbp_msg_queue, p_dbp_msg);

    DSPLOG_INFO("cmd 0x%x message enqueued", p_dbp_command->dbp_header.command);

    return DBG_DBP_ERR_CODE_SUCCESS;
}

DBG_DBP_ERR_CODE
DBG_core_create_and_enqueue_dbp_msg_with_data(dbg_dbp_msg_queue_t *p_dbp_msg_queue,
                                              dbp_command_t *p_dbp_command,
                                              uint8_t *p_u8_data, uint16_t u16_size)
{
    /*If the dbp_header msglen field is less than the additional payload size
     * minus the size of the dbp_header - then the command message is incorrect*/
    if(p_dbp_command->dbp_header.msglen < (u16_size - sizeof(struct dbp_header)))
    {
        DSPLOG_ERROR("Invalid command -> p_dbp_command->dbp_header.msglen < u16_size - sizeof(struct dbp_header)");
        return DBG_DBP_ERR_CODE_CANNOT_CREATE_CMD;
    }

    /*Allocate a DBP message*/
    dbg_dbp_msg_t *p_dbp_msg = DBG_core_dbp_msg_alloc(p_dbp_command->dbp_header.msglen);

    if(p_dbp_msg == NULL)
    {
        DSPLOG_ERROR("Cannot allocate command message buffer");
        return DBG_DBP_ERR_CODE_CANNOT_ALLOCATE_BUFFER;
    }

    uint16_t u16_cmd_len = p_dbp_command->dbp_header.msglen - u16_size;

    /*Copy the command to the message buffer*/
    memcpy(p_dbp_msg->p_u8_buf, p_dbp_command, u16_cmd_len);

    /*Copy the additional data to the message buffer*/
    memcpy(&p_dbp_msg->p_u8_buf[u16_cmd_len], p_u8_data, u16_size);

    /*Enqueue the DBP message to the command queue*/
    DBG_core_enqueue_dbp_msg(p_dbp_msg_queue, p_dbp_msg);

    DSPLOG_INFO("cmd 0x%x message enqueued", p_dbp_command->dbp_header.command);

    return DBG_DBP_ERR_CODE_SUCCESS;
}

/*
 * while(cmd_queue is empty)
 * {
 *      read_channel_transport_header
 *      if(valid && dest == DSP)
 *      {
 *          read_packet_length
 *          allocate dbp_msg buffer to hold the entire response
 *      }
 *
 *      simple_dequeue(cmd_queue)
 *      targetexchange()
 *      if(send_successful)
 *      {
 *          dequeue(cmd_queue)
 *          free(dbp_cmd_msg_buf)
 *      }
 *      else
 *      {
 *         Probably because we didn't allocate response buffer
 *         reallocate and try again
 *      }
 *      if(receive valid)
 *      {
 *          enqueue(resp_queue)
 *      }
 *}
 * FIXME: THE FUNCTION DOESN'T YET DEAL WITH FRAGMENTED MESSAGES !!!!!
 */

DBG_DBP_ERR_CODE
DBG_core_dbp_comms_check(DBG *dbg)
{
    DBG_core_debug_server *p_dbg_server = dbg->dbg_core.p_dbg_server;
    DBG_TARGET *p_dbg_target = &dbg->dbg_target_if;

    dbg_dbp_msg_queue_t *p_dbp_msg_cmd_queue = &p_dbg_server->dbp_cmd_queue;
    dbg_dbp_msg_queue_t *p_dbp_msg_resp_queue = &p_dbg_server->dbp_resp_queue;

    /*Don't dequeue dbp_msg from the queue just yet*/
    dbg_dbp_msg_t *p_dbp_cmd = p_dbp_msg_cmd_queue->p_dbp_msg_head;

    dbg_dbp_msg_t *p_dbp_resp = NULL;
    uint16_t u16_resp_pack_len = 0;
    DBG_TARGET_STATE t_state = DBG_target_query_state_from_channel(p_dbg_target);

    if(t_state == RW_NOT_READY)
    {
        DSPLOG_JUNK("channel not ready");
        return DBG_DBP_ERR_CODE_CHANNEL_NOT_READY;
    }
    /* Loop until we have processed all the commands in the dbp command queue
     * or we have valid bytes to receive
     * */

    while( (p_dbp_cmd != NULL && t_state != RW_NOT_READY) || t_state == R_READY)
    {
        /*If the channel has a valid response*/
        if(t_state == R_READY)
        {
            /* Get the packet length from the channel */
            u16_resp_pack_len = DBG_target_query_packet_length(p_dbg_target);
            DSPLOG_INFO("dbp_comms_check(): Channel has a valid response - packet length 0x%x", u16_resp_pack_len);

            /*Allocate a dbp_msg_buffer which can hold the entire packet*/
            p_dbp_resp = DBG_core_dbp_msg_alloc(u16_resp_pack_len);
            if(p_dbp_resp == NULL)
            {
                DSPLOG_ERROR("Allocation of DBP response message FAILED - Cannot read");
                return DBG_DBP_ERR_CODE_CANNOT_ALLOCATE_BUFFER;
            }
        }

        /*trigger transport exchange*/
        size_t send_length = (p_dbp_cmd == NULL) ? 0 : (size_t)p_dbp_cmd->u16_buf_size;
        size_t receive_length = (size_t)u16_resp_pack_len;
        size_t prev_send_length = send_length;

        DSPLOG_INFO("dbp_comms_check(): Trigger transfer exchange send_length 0x%x receive_length 0x%x",
                    (unsigned) send_length, (unsigned) receive_length);

        bool b_exchange_status =
            DBG_targetExchangeData(p_dbg_target, (p_dbp_cmd != NULL) ? p_dbp_cmd->p_u8_buf : NULL, &send_length,
                    (p_dbp_resp != NULL) ? p_dbp_resp->p_u8_buf : NULL , &receive_length);

        /*Deal with successful transfer*/
        if(b_exchange_status)
        {
            /*If message sent fully*/
            if(p_dbp_cmd != NULL && send_length == prev_send_length)
            {
                DSPLOG_INFO("dbp_comms_check(): Message sent successfully");
                /*dequeue the dbp message*/
                p_dbp_cmd = DBG_core_dequeue_dbp_msg(p_dbp_msg_cmd_queue);

                /*free dbp message and its buffer*/
                DBG_core_dbp_msg_dealloc(p_dbp_cmd);
            }

            /*If a message was received*/
            if(receive_length != 0)
            {
                DSPLOG_INFO("dbp_comms_check(): Message received len %x enqueue dbp response",
                            (unsigned) receive_length);
                DBG_core_enqueue_dbp_msg(p_dbp_msg_resp_queue, p_dbp_resp);
            }
            else if(p_dbp_resp != NULL)
            {
                DBG_core_dbp_msg_dealloc(p_dbp_resp);
            }
        }
        else
        {
            DSPLOG_ERROR("DBG_TargetExchange not successful");
            return DBG_DBP_ERR_CODE_TARGET_EXCHANGE_FAILED;
        }

        /*Next command*/
        p_dbp_cmd = p_dbp_msg_cmd_queue->p_dbp_msg_head;
        t_state = DBG_target_query_state_from_channel(p_dbg_target);
    }

    if(!DBG_core_is_queue_empty(p_dbp_msg_resp_queue))
        DBG_core_dbp_handle_resp(dbg);

    return DBG_DBP_ERR_CODE_SUCCESS;
}
