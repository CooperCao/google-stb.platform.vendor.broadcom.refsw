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

#ifndef _DBG_CORE_MP_DBP_MSG_H_
#define _DBG_CORE_MP_DBP_MSG_H_

#include "libdspcontrol/DBG.h"
#include "libthreadxp/user/dp/dbp.h"

#include "DBG_core_debug_server_mp.h"
#include "DBG_core_debug_server_common.h"

typedef struct dbg_dbp_msg_t
{
    uint8_t *p_u8_buf;                      /*Pointer to the circular buffer*/
    uint16_t u16_buf_size;                  /*Total buffer size*/
    struct dbg_dbp_msg_t *p_dbp_msg_next;
} dbg_dbp_msg_t;

typedef struct
{
    dbg_dbp_msg_t   *p_dbp_msg_head;
    dbg_dbp_msg_t   *p_dbp_msg_tail;
} dbg_dbp_msg_queue_t;

void
DBG_core_dbp_msg_dealloc(dbg_dbp_msg_t *p_dbg_dbp_msg);

dbg_dbp_msg_t*
DBG_core_dequeue_dbp_msg(dbg_dbp_msg_queue_t *p_dbp_msg_queue);

DBG_DBP_ERR_CODE
DBG_core_dbp_comms_check(DBG *dbg);

DBG_DBP_ERR_CODE
DBG_core_create_and_enqueue_dbp_msg(dbg_dbp_msg_queue_t *p_dbp_msg_queue, dbp_command_t *p_dbp_command);

DBG_DBP_ERR_CODE
DBG_core_create_and_enqueue_dbp_msg_with_data(dbg_dbp_msg_queue_t *p_dbp_msg_queue,
                                              dbp_command_t *p_dbp_command,
                                              uint8_t *p_u8_data, uint16_t u16_size);

#endif
