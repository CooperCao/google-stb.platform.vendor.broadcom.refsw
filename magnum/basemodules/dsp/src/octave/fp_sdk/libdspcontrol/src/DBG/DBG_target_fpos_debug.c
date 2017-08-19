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
#include <stdint.h>

#include "libdspcontrol/CHIP.h"
#include "libdspcontrol/DSPLOG.h"
#include "libdspcontrol/OS.h"

#include "DBG_interfaces.h"
#include "DBG_target_fpos_debug.h"

#if !FEATURE_IS(DBG_TARGET_IF, FPOS_DEBUG)
#  error "This source is for the fpos_debug variant of the DBG target interface"
#endif

#define SLOW_RETRY_ATTEMPTS 4
#define SLEEP_IN_MS_BETWEEN_ATTEMPTS 10

/*The channel is a combination of MISC block mailboxes and shared buffer allocated by the target*/

DBG_RET
DBG_targetInit(DBG_TARGET *p_dbg_target, DBG_PARAMETERS *parameters)
{
    p_dbg_target->id = parameters->id;
    p_dbg_target->dsp = parameters->dsp;
    p_dbg_target->dsp_core = parameters->dsp_core;

    /*Initially the shared buffer is not configured -
     * the host doesn't know about the location and size*/
    p_dbg_target->mutex_acquire_retries = parameters->mutex_acquire_retries;
    p_dbg_target->b_channel_configured = false;
    p_dbg_target->p_u8_channel_address = NULL;

    p_dbg_target->u16_channel_size = 20;
    p_dbg_target->target_err_code = DBG_TARGET_SUCCESS;

    return DBG_SUCCESS;
}

void
DBG_targetFinish(DBG_TARGET *p_dbg_target __unused)
{
}

void
DBG_target_raise_external_debug_interrupt(DBG_TARGET *p_dbg_target)
{
    DSP_writeSharedRegister(p_dbg_target->dsp,
            MISC_BLOCK(p_dbg_target->dsp, p_dbg_target->dsp_core, INTERRUPT_DRQ_CLEAR), 0x1);

    DSP_writeSharedRegister(p_dbg_target->dsp,
            MISC_BLOCK(p_dbg_target->dsp, p_dbg_target->dsp_core, INTERRUPT_DRQ_SET), 0x1);
}

void
DBG_target_release_mutex(DBG_TARGET *p_dbg_target)
{
    DSP_writeSharedRegister(p_dbg_target->dsp,
            MISC_COMM_DBG_MUTEX(p_dbg_target->dsp, p_dbg_target->dsp_core), 0x1);
}

DBG_TARGET_ERR_CODE
DBG_target_acquire_mutex(DBG_TARGET *p_dbg_target)
{
    unsigned acquire_attempts = 0;
    int i;

    for(i= 0; i<SLOW_RETRY_ATTEMPTS;i++)
    {
        while(acquire_attempts++ <= p_dbg_target->mutex_acquire_retries)
        {
            if(DSP_readSharedRegister(p_dbg_target->dsp,
                        MISC_COMM_DBG_MUTEX(p_dbg_target->dsp, p_dbg_target->dsp_core)) == 1)
            {
                DSPLOG_DEBUG("DBG_acquireMutex(): mutex acquired");
                return DBG_TARGET_SUCCESS;
            }
        }
        OS_sleep(SLEEP_IN_MS_BETWEEN_ATTEMPTS);
    }
    return DBG_TARGET_CANNOT_ACQUIRE_MUTEX;
}


void
DBG_target_read_thdr(DBG_TARGET *p_dbg_target, dbp_thdr_t *p_thdr)
{
    p_thdr->u32_thdr = DSP_readSharedRegister(p_dbg_target->dsp,
                    MISC_COMM_DBG_MAILBOX_BASE_ADDR(p_dbg_target->dsp, p_dbg_target->dsp_core));
}

void
DBG_target_write_thdr(DBG_TARGET *p_dbg_target, dbp_thdr_t *p_thdr)
{
    DSP_writeSharedRegister(p_dbg_target->dsp, MISC_COMM_DBG_MAILBOX_BASE_ADDR(p_dbg_target->dsp,
                p_dbg_target->dsp_core), p_thdr->u32_thdr);
}

DBG_TARGET_STATE
DBG_target_query_state_from_channel(DBG_TARGET *p_dbg_target)
{
    dbp_thdr_t thdr;

    DBG_target_read_thdr(p_dbg_target, &thdr);

    return DBG_targetQueryState(&thdr);
}

DBG_TARGET_STATE
DBG_targetQueryState(dbp_thdr_t *p_thdr)
{
    if(!p_thdr->dbp_thdr_bitfields.b_hdr_valid)
        /*Nothing to read - can write*/
        return W_READY;
    else if(p_thdr->dbp_thdr_bitfields.b_hdr_valid && p_thdr->dbp_thdr_bitfields.b_hdr_dest == DEST_HOST)
        /*Valid packet waiting to be consumed by the HOST - read and write if necessary*/
        return R_READY;
    else
        /*Valid packet waiting to be consumed by the target - cannot read or write*/
        return RW_NOT_READY;
}


DBG_TARGET_ERR_CODE
DBG_target_write(DBG_TARGET *p_dbg_target, dbp_thdr_t *p_thdr,
                 uint8_t *p_u8_send_buffer, size_t *send_length)
{
    dbp_thdr_t send_thdr;
    DBG_TARGET_STATE t_state = DBG_targetQueryState(p_thdr);
    /*Check if the channel has any bytes to read and if it is not the last packet for the message*/
    if(t_state == R_READY && !p_thdr->dbp_thdr_bitfields.b_hdr_last_packet)
    {
        /*We cannot write into the channel when the incoming message isn't complete*/
        DSPLOG_INFO("Partial incoming message - cannot write now");
        *send_length = 0;
    }

    if(*send_length > p_dbg_target->u16_channel_size)
    {
        DSPLOG_ERROR("DBG(%u): Message too big for the channel - channel size 0x%x, packet size 0x%zx",
                     p_dbg_target->id, p_dbg_target->u16_channel_size, *send_length);
        *send_length = 0;
        return DBG_TARGET_CANNOT_SEND_DATA;
    }

    send_thdr.dbp_thdr_bitfields.b_hdr_valid = (*send_length != 0) ? true : false;
    send_thdr.dbp_thdr_bitfields.b_hdr_dest = DEST_DSP;
    /*FIXME: Do we always ensure that the entire message fits in the packet*/
    send_thdr.dbp_thdr_bitfields.b_hdr_last_packet = true;
    send_thdr.dbp_thdr_bitfields.u_reserved = 0;
    send_thdr.dbp_thdr_bitfields.u16_hdr_pack_length = *send_length;

    /*Write the payload if available*/
    if (*send_length > 0)
    {
        if(p_dbg_target->b_channel_configured)
        {
            DSPLOG_DEBUG("write to the channel 0x%" PRIxPTR " num_bytes 0x%zx",
                         (uintptr_t) p_dbg_target->p_u8_channel_address, ((*send_length+3)>>2)<<2);
            if(DSP_writeSystemData(p_dbg_target->dsp, (uint32_t)(uintptr_t)p_dbg_target->p_u8_channel_address, p_u8_send_buffer, ((*send_length+3)>>2)<<2) != DSP_SUCCESS)
            {
                DSPLOG_ERROR("target_write failed - no bytes written");
                *send_length = 0;
                return DBG_TARGET_CANNOT_SEND_DATA;
            }
        }
        else
        {
            uint16_t u16_pack_length = (uint16_t)*send_length;
            while (u16_pack_length >0)
            {
                int mailbox_base_index = 1;

                DSP_writeSharedRegister(p_dbg_target->dsp,
                                        MISC_COMM_DBG_MAILBOX_BASE_ADDR(p_dbg_target->dsp, p_dbg_target->dsp_core) + (mailbox_base_index * 4),
                                        *(uint32_t *)p_u8_send_buffer++);
                mailbox_base_index++;
                u16_pack_length -= sizeof(uint32_t);

                u16_pack_length = ( (u16_pack_length > sizeof(uint32_t)) ? u16_pack_length - sizeof(uint32_t) : 0);
            }

        }

        /*Write the transport header*/
        DBG_target_write_thdr(p_dbg_target, &send_thdr);
    }

    return DBG_TARGET_SUCCESS;
}

/*The caller must ensure that there is enough space in the receive_buffer*/
DBG_TARGET_ERR_CODE
DBG_target_read(DBG_TARGET *p_dbg_target, dbp_thdr_t *p_thdr,
                 uint8_t *p_u8_receive_buffer, size_t *receive_length)
{
    uint16_t u16_pack_len = p_thdr->dbp_thdr_bitfields.u16_hdr_pack_length;
    int mailbox_base_index = 1;
    uint32_t *p_u32_buf = (uint32_t *)p_u8_receive_buffer;

    DSPLOG_DEBUG("target_read(): read %d bytes", u16_pack_len);

    if (u16_pack_len > *receive_length || p_u8_receive_buffer == NULL)
    {
        DSPLOG_ERROR("Not enough space to read bytes");
        *receive_length = 0;
        return DBG_TARGET_NO_SPACE_RECV_BUFFER;
    }

    if(p_dbg_target->b_channel_configured)
    {
        DSPLOG_DEBUG("DBG_target_read(): Channel configured - reading %d bytes from the shared buffer at 0x%x",
                (((u16_pack_len + 3)>>2)<<2), (uint32_t)(uintptr_t)p_dbg_target->p_u8_channel_address);

        if(DSP_readSystemData(p_dbg_target->dsp,
                              (void *)p_u8_receive_buffer,
                              (uint32_t)(uintptr_t)p_dbg_target->p_u8_channel_address,
                              ((u16_pack_len+3)>>2)<<2) != DSP_SUCCESS)
        {
            DSPLOG_ERROR("target_read failed - no bytes read");
            *receive_length = 0;
            return DBG_TARGET_CANNOT_READ_DATA;
        }
    }
    else
    {
        /*Channel is not yet configured - So the contents are in the mailboxes*/
        uint16_t u16_bytes_read = u16_pack_len;
        while(u16_bytes_read)
        {
            DSPLOG_DEBUG("DBG_target_read(): Channel not configured - read %d bytes from the mailbox addr 0x%x", u16_bytes_read,
                    MISC_COMM_DBG_MAILBOX_BASE_ADDR(p_dbg_target->dsp, p_dbg_target->dsp_core) + mailbox_base_index * 4);

            p_u32_buf[mailbox_base_index-1] = DSP_readSharedRegister(p_dbg_target->dsp,
                                                             (MISC_COMM_DBG_MAILBOX_BASE_ADDR(p_dbg_target->dsp, p_dbg_target->dsp_core) + mailbox_base_index * 4) );
            mailbox_base_index++;

            u16_bytes_read =( (u16_bytes_read > sizeof(uint32_t)) ? u16_bytes_read - sizeof(uint32_t) : 0 );
        }
    }

    *receive_length = u16_pack_len;
    /*target read successful, clear the valid bit*/
    p_thdr->dbp_thdr_bitfields.b_hdr_valid = false;
    DBG_target_write_thdr(p_dbg_target, p_thdr);

    return DBG_TARGET_SUCCESS;
}

uint16_t
DBG_target_query_packet_length(DBG_TARGET *p_dbg_target)
{
    dbp_thdr_t thdr;

    DBG_target_read_thdr(p_dbg_target, &thdr);

    if(thdr.dbp_thdr_bitfields.b_hdr_valid && thdr.dbp_thdr_bitfields.b_hdr_dest == DEST_HOST)
        return thdr.dbp_thdr_bitfields.u16_hdr_pack_length;

    return 0;
}

bool
DBG_targetExchangeData(DBG_TARGET *p_dbg_target,
                        uint8_t *send_buffer, size_t *send_length,
                        uint8_t *receive_buffer, size_t *receive_length)
{
    DBG_TARGET_ERR_CODE err_code = DBG_TARGET_SUCCESS;
    DBG_TARGET_STATE t_state;
    dbp_thdr_t thdr;

    /*Read the transport header from the misc block - safe and no need to acquire mutex*/
    DBG_target_read_thdr(p_dbg_target, &thdr);
    t_state = DBG_targetQueryState(&thdr);

    /*Check if the channel is available to the HOST*/
    if(t_state == RW_NOT_READY)
    {
        *send_length = 0;
        *receive_length = 0;
        DSPLOG_INFO("DBG(%u): Channel not available", p_dbg_target->id);
        return false;
    }
    if(DBG_target_acquire_mutex(p_dbg_target) == DBG_TARGET_SUCCESS)
    {
        /*Read the transport header again*/
        DBG_target_read_thdr(p_dbg_target, &thdr);
        t_state = DBG_targetQueryState(&thdr);

        DSPLOG_DEBUG("targetExchangeData(): Initial thdr - 0x%x",thdr.u32_thdr);

        /*Valid bytes present in the channel - consume the packet*/
        if(t_state == R_READY)
        {
            DSPLOG_INFO("targetExchangeData(): target read - 0x%zx bytes", *receive_length);
            err_code = DBG_target_read(p_dbg_target, &thdr, receive_buffer, receive_length);
        }

        if(err_code != DBG_TARGET_SUCCESS)
        {
            DSPLOG_ERROR("DBG(%u): Channel read failed - err_code 0x%x", p_dbg_target->id, err_code);
            *send_length = 0;
            *receive_length = 0;
            p_dbg_target->target_err_code = err_code;
            goto release_mutex_and_return;
        }

        /*Write to the channel*/
        DSPLOG_INFO("targetExchangeData(): target write - 0x%zx bytes", *send_length);
        err_code = DBG_target_write(p_dbg_target, &thdr, send_buffer, send_length);

        if(err_code != DBG_TARGET_SUCCESS)
        {
            DSPLOG_ERROR("DBG(%u): Channel write failed - err_code 0x%x", p_dbg_target->id, err_code);
            *send_length = 0;
            p_dbg_target->target_err_code = err_code;
            goto release_mutex_and_return;
        }
        DSPLOG_DEBUG("targetExchangeData(): Release mutex");
        DBG_target_release_mutex(p_dbg_target);

        /*Raise external debug interrupt*/
        DSPLOG_DEBUG("targetExchangeData(): Raise debug interrupt");
        DBG_target_raise_external_debug_interrupt(p_dbg_target);

        return true;
    }
    else
    {
        DSPLOG_ERROR("DBG(%u): Host cannot acquire mutex", p_dbg_target->id);
        *send_length = 0;
        *receive_length = 0;
        p_dbg_target->target_err_code = DBG_TARGET_CANNOT_ACQUIRE_MUTEX;
        return false;
    }

release_mutex_and_return:
    DBG_target_release_mutex(p_dbg_target);
    return false;
}
