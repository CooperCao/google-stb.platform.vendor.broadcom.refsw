/***************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 *
 * Module Description: This is the HW independent part of the SW message filter. It
 * accepts data, parsers the packet, and compares it with the filter. It is based on
 * 7003 SW filter file BCM3543\tests\filter\msg_filter.c
 *
 *
 ***************************************************************************/
#include "nexus_transport_module.h"
#include "blst_slist.h"
#include "bdemux_ts.h"
#include "bdemux_pes.h"
#include "biobits.h"

BDBG_MODULE(nexus_message_swfilter_parser);

#define TS_PACKET_SIZE 188

#define B_STR_FLAG(v,name) ((v)?#name " ":"")
#define B_NEXT_CONT_COUNTER(c) (((c)+1)&0x0F)

struct NEXUS_SwFilter_FilterState {
    BLST_S_ENTRY(NEXUS_SwFilter_FilterState) next;
    void *buffer;
    unsigned buffer_size;
    NEXUS_SwFilter_Msg_P_callback_t callback;
    void * context;

    /* The following are used for For PSI filters*/
    NEXUS_MessageFilter filt;
    bool enabled;
    struct ts_pid * pid_state;  /* parent */
    bool disable_crc_check;

    /* Used for PES filter */
    bdemux_pes pes;
    size_t data_size;   /*PES data size*/

    /* no filters for TS */
};

typedef struct ts_packet_tag{
    uint16_t PID;
    bool payload_unit_start_indicator;
    uint8_t continuity_counter;
    uint8_t *p_data_byte;
    uint8_t data_size;
    uint8_t *p_packet;
    uint8_t discontinuity;
} ts_packet_t;

#define TS_READ_16( buf ) ((uint16_t)((buf)[0]<<8|(buf)[1]))


BLST_S_HEAD(filter_list_t, NEXUS_SwFilter_FilterState);

BLST_S_HEAD(pes_filter_list_t, NEXUS_SwFilter_FilterState);


struct ts_pid {
    BLST_S_ENTRY(ts_pid) link;
    uint16_t pid;
    uint8_t continuity_counter;
    uint8_t discontinuity;
    size_t want;
    uint8_t * msg_end;
    uint8_t tmp_filter;
    uint8_t tmp_buffer[NEXUS_MESSAGE_FILTER_SIZE];
    struct NEXUS_SwFilter_FilterState * matched_filter;
    struct filter_list_t filters;
    struct NEXUS_SwFilter_FilterState * capture_filter;
    size_t bytes_complete;
};


struct parser_state_t {
    uint32_t packet_count;
    BLST_S_HEAD(ts_pid_list, ts_pid) pids;
    struct filter_list_t free_filter;
};




struct pes_state_t {
    uint32_t packet_count;
    struct pes_filter_list_t free_filter;
    struct pes_filter_list_t used_filter;
};

static struct pes_state_t pst;

static struct parser_state_t msg_pst;

typedef enum {bdemux_pes_result_match, bdemux_pes_result_nomatch, bdemux_pes_result_more} bdemux_pes_result;

static uint32_t bcrc32(uint8_t * data, unsigned len);
static struct ts_pid * NEXUS_SwFilter_Msg_P_FindPid(uint16_t pid);
static void NEXUS_SwFilter_Msg_P_ProcessPacket(ts_packet_t * packet, struct ts_pid * pid_state);
static void NEXUS_SwFilter_Msg_P_MatchFilters(const uint8_t * msg, struct ts_pid * pid_state);
static void NEXUS_SwFilter_Msg_P_ParsePacket(const uint8_t *buf, ts_packet_t * packet);
static void NEXUS_SwFilter_Msg_P_CapturePacket(ts_packet_t * packet, struct ts_pid * pid_state);

void NEXUS_SwFilter_Msg_P_Init(void)
{
    BKNI_Memset(&msg_pst, 0, sizeof(struct parser_state_t));
    BLST_S_INIT(&msg_pst.free_filter);
}

/* Break buffer on packets and process each packet */
size_t NEXUS_SwFilter_Msg_P_Feed(uint8_t * buffer, size_t size)
{
    ts_packet_t packet;
    size_t consumed;
    uint8_t * walker;
    struct ts_pid * pid_state;

    BDBG_MSG(("NEXUS_SwFilter_Msg_P_Feed: Entering: buffer=%p size=%u", (void *)buffer, (uint32_t)size));

    BKNI_Memset(&packet, 0, sizeof(struct ts_packet_tag));
    consumed = 0;
    if(0x47 != *buffer){
        BDBG_ERR(("%s %d sync error", __FILE__, __LINE__));
        consumed = size;
        goto ExitFunc;
    }
    walker = buffer;
    while(size - consumed >= TS_PACKET_SIZE){
        NEXUS_SwFilter_Msg_P_ParsePacket(walker, &packet);
        /* TODO: we only process one filter per pid. HW processes multiple filters per pid. this would improve performance in some apps greatly. */
        pid_state = NEXUS_SwFilter_Msg_P_FindPid(packet.PID);
        if(NULL != pid_state){
            NEXUS_SwFilter_Msg_P_ProcessPacket(&packet, pid_state);
        }
        walker += TS_PACKET_SIZE;
        consumed += TS_PACKET_SIZE;
        msg_pst.packet_count++;
    }
ExitFunc:
    return consumed;
}

struct ts_pid * NEXUS_SwFilter_Msg_P_FindPid(uint16_t pid)
{
    struct ts_pid * pid_state;
    for (pid_state = BLST_S_FIRST(&msg_pst.pids); pid_state; pid_state = BLST_S_NEXT(pid_state, link)) {
        if (pid == pid_state->pid) break;
    }
    return pid_state;
}

static struct ts_pid * NEXUS_SwFilter_Msg_P_AllocPid(void)
{
    struct ts_pid *pid_state;
    pid_state = BKNI_Malloc(sizeof(*pid_state));
    if (!pid_state) {
        BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }
    pid_state->discontinuity = 1;
    BLST_S_INIT(&pid_state->filters);
    BLST_S_INSERT_HEAD(&msg_pst.pids, pid_state, link);
    return pid_state;
}

static void NEXUS_SwFilter_Msg_P_FreePid(struct ts_pid *pid_state)
{
    BLST_S_REMOVE(&msg_pst.pids, pid_state, ts_pid, link);
    BKNI_Free(pid_state);
}

/* Parse packet and assemble sections */
void NEXUS_SwFilter_Msg_P_ProcessPacket(ts_packet_t * packet, struct ts_pid * pid_state)
{
    uint8_t * new_pos;
    uint8_t * current_pos;
    size_t bytes_remains;       /* number of bytes to process */
    uint8_t * dst;
    size_t msg_length;
    bool msg_skip;              /* skip current message */

/*    BDBG_MSG(("NEXUS_SwFilter_Msg_P_ProcessPacket: Entering: packet=0x%x, pid_state=0x%x", packet, pid_state));*/


    /* check continuity */
    if((pid_state->continuity_counter == packet->continuity_counter) && !pid_state->discontinuity && !packet->discontinuity){
        /* duplicate packet */
        BDBG_MSG(("NEXUS_SwFilter_Msg_P_ProcessPacket: duplicate pkt. discarding."));
        goto ExitFunc;
    }
    pid_state->continuity_counter = (pid_state->continuity_counter + 1) & 0xF;

    if((pid_state->continuity_counter != packet->continuity_counter) || (packet->discontinuity)){
        /* discontinuity */
        pid_state->continuity_counter = packet->continuity_counter;
        pid_state->discontinuity = 1;
        pid_state->matched_filter = NULL;
        BDBG_MSG(("NEXUS_SwFilter_Msg_P_ProcessPacket: discontinuity = 1"));
    }else{
        pid_state->discontinuity = 0;
    }

    if(packet->data_size == 0){
        goto ExitFunc;
    }

    if(NULL != pid_state->capture_filter){
        NEXUS_SwFilter_Msg_P_CapturePacket(packet, pid_state);
        goto ExitFunc;
    }

    current_pos = (uint8_t *)packet->p_data_byte;
    new_pos = NULL;
    if(1 == packet->payload_unit_start_indicator){
        new_pos = current_pos + 1 + current_pos[0]; /* new section */
        current_pos += 1;       /* skip pointer_field */
    }

    /* If there is no section in progress check if new section starts
       in this packet. If no new section starts discard the packet. */
    if((NULL == pid_state->matched_filter) && (0 == pid_state->tmp_filter)){
        if(NULL != new_pos){    /* new section starts in this packet */
/*            BDBG_MSG(("NEXUS_SwFilter_Msg_P_ProcessPacket: new section starting"));*/
            pid_state->want = NEXUS_MESSAGE_FILTER_SIZE; /* we want at least filter size */ /* rj: see if we shud implement this as atomsize for recpump */
            current_pos = new_pos;
        }else{
/*            BDBG_MSG(("NEXUS_SwFilter_Msg_P_ProcessPacket: waiting for new section. discarding packet."));*/
            goto ExitFunc;
        }
    }

    bytes_remains = packet->data_size + packet->p_data_byte - current_pos;
    msg_skip = 0;

    while(bytes_remains > 0){

        BDBG_MSG(("NEXUS_SwFilter_Msg_P_ProcessPacket: bytes_remains=%u", (uint32_t)bytes_remains));
        if(bytes_remains >= pid_state->want){

            if((NULL == pid_state->matched_filter) && (0 == msg_skip)){
                if(pid_state->tmp_filter){
                    /* part of the filter is in the temp buffer */
                    /* copy the rest */

                    dst = pid_state->tmp_buffer + (NEXUS_MESSAGE_FILTER_SIZE - pid_state->want);
                    BKNI_Memcpy(dst, current_pos, pid_state->want);
                    /* reset dst pointer */
                    dst = pid_state->tmp_buffer;
                    NEXUS_SwFilter_Msg_P_MatchFilters(dst, pid_state);

                    if(NULL != pid_state->matched_filter){
                        BKNI_Memcpy(pid_state->matched_filter->buffer, dst, NEXUS_MESSAGE_FILTER_SIZE);
                        pid_state->msg_end = (uint8_t * )(pid_state->matched_filter->buffer) + NEXUS_MESSAGE_FILTER_SIZE;

                    }else{
                        msg_skip = 1;
                    }
                    pid_state->tmp_filter = 0;
                    msg_length = (TS_READ_16(&dst[1]) & 0xFFF) + 3;
                }else{
                    NEXUS_SwFilter_Msg_P_MatchFilters(current_pos, pid_state);
                    if(NULL != pid_state->matched_filter){
                        BKNI_Memcpy(pid_state->matched_filter->buffer, current_pos, NEXUS_MESSAGE_FILTER_SIZE);
                        pid_state->msg_end = (uint8_t * )(pid_state->matched_filter->buffer) + NEXUS_MESSAGE_FILTER_SIZE;

                    }else{
                        msg_skip = 1;
                    }
                    msg_length = (TS_READ_16(&current_pos[1]) & 0xFFF) + 3;
                }

                if(msg_length < NEXUS_MESSAGE_FILTER_SIZE){
                    current_pos += msg_length;
                    bytes_remains -= msg_length;
                    pid_state->want = 0;
                }else{
                    current_pos += pid_state->want;
                    bytes_remains -= pid_state->want;
                    pid_state->want = msg_length - NEXUS_MESSAGE_FILTER_SIZE;
                }
            }else{
                if(NULL != pid_state->matched_filter){
                    bool msg_notify;
                    uint8_t syntax_indicator;
                    void * new_buffer;
                    uint8_t * pBuffer;
                    pBuffer = (uint8_t * )(pid_state->matched_filter->buffer);
                    /* complete message */
                    BKNI_Memcpy(pid_state->msg_end, current_pos, pid_state->want);

                    pid_state->msg_end += pid_state->want;
                    /* notify here*/
/*
                    msg_length = 3 + (TS_READ_16(&pid_state->matched_filter->buffer[1]) & 0xFFF);
                    syntax_indicator = (uint8_t)(pid_state->matched_filter->buffer[1]) & 0x80;*/

                    msg_length = 3 + (TS_READ_16(&pBuffer[1]) & 0xFFF);
                    syntax_indicator = (uint8_t)(pBuffer[1]) & 0x80;

                    msg_notify = 1;
                    if((0 == pid_state->matched_filter->disable_crc_check) && (0 != syntax_indicator)){
                        msg_notify = (0 == bcrc32(pid_state->matched_filter->buffer, msg_length));
                        BDBG_MSG(("msg_process_packet: crc %s",((msg_notify) ? "passed" : "failed")));
                    }
                    BDBG_MSG(("msg_process_packet: pid %x msg %x len %u not %d sid %x", pid_state->pid, pBuffer[0], (uint32_t)msg_length, msg_notify, syntax_indicator));

                    if((0 != msg_notify) && (NULL != pid_state->matched_filter->callback)){
                        new_buffer = (pid_state->matched_filter->callback)( pid_state->matched_filter->context, msg_length);
                        if(NULL == new_buffer){
                            pid_state->matched_filter->enabled = false;
                        }else{
                            pid_state->matched_filter->buffer = new_buffer;
                        }
                    }
                    /* terminate message */
                    pid_state->matched_filter = NULL;
                    current_pos += pid_state->want;
                    if(0xFF == *current_pos){
                        /* end of messages in this packet */
                        bytes_remains = 0;
                    }else{
                        bytes_remains -= pid_state->want;
                        pid_state->want = NEXUS_MESSAGE_FILTER_SIZE;
                    }
                }else{
                    /* skipping message */
                    msg_skip = 0;
                    current_pos += pid_state->want;
                    if(0xFF == *current_pos){
                        /* end of messages in this packet */
                        bytes_remains = 0;
                    }else{
                        bytes_remains -= pid_state->want;
                        pid_state->want = NEXUS_MESSAGE_FILTER_SIZE;
                    }
                }
            }
        }else{
            if(NULL == pid_state->matched_filter){
                if(0 == msg_skip){ /* capture header */
                    dst = pid_state->tmp_buffer;
                    dst += NEXUS_MESSAGE_FILTER_SIZE - pid_state->want;
                    BKNI_Memcpy(dst, current_pos, bytes_remains);
                    pid_state->want -= bytes_remains;
                    pid_state->tmp_filter = 1;
                }
                bytes_remains = 0;
            }else{
                /* we are processing filter, copy data in to filter buffer */
                BKNI_Memcpy(pid_state->msg_end, current_pos, bytes_remains);
                pid_state->msg_end += bytes_remains;
                pid_state->want -= bytes_remains;
                bytes_remains = 0;
            }
        }
    }
ExitFunc:
    return;
}


static bool NEXUS_SwFilter_Msg_P_FilterCompare(const uint8_t *msg, const uint8_t *inclMask, const uint8_t *exclMask, const uint8_t *coef, size_t filterSize )
{
    bool inclResult = true;
    bool exclResult = true;
    bool exclEnabled = false;
    size_t i;

/* Skip byte two (since the hardware doesn't look at that byte). we want to make the SW filter same as HW  */


    for( i = 0; i < 1; i++ )
    {
        if( (inclMask[i] | coef[i]) != (inclMask[i] | msg[i]) )
        {
            inclResult = false;
            break;
        }

        if( exclEnabled == false && exclMask[i] != 0xFF )
        {
            exclEnabled = true;
            exclResult = false;
        }

        if( (~exclMask[i] & coef[i]) != (~exclMask[i] & msg[i]) )
        {
            exclResult = true;
        }
    }

    for( i = 2; i < filterSize; i++ )
    {
        if( (inclMask[i] | coef[i]) != (inclMask[i] | msg[i+1]) )
        {
            inclResult = false;
            break;
        }

        if( exclEnabled == false && exclMask[i] != 0xFF )
        {
            exclEnabled = true;
            exclResult = false;
        }

        if( (~exclMask[i] & coef[i]) != (~exclMask[i] & msg[i+1]) )
        {
            exclResult = true;
        }
    }

    return inclResult && exclResult;
}

/* Enumerate filters for pid and compare them against message */
static void NEXUS_SwFilter_Msg_P_MatchFilters(const uint8_t * msg, struct ts_pid * pid_state)
{
    struct NEXUS_SwFilter_FilterState * f;
/*    BDBG_MSG(("NEXUS_SwFilter_Msg_P_MatchFilters: msg=0x%x, pid_state=0x%x", msg, pid_state));*/

    f = BLST_S_FIRST(&pid_state->filters);
    for(; f != NULL ; f = BLST_S_NEXT(f, next)){
        if(0 != f->enabled){
            if(NEXUS_SwFilter_Msg_P_FilterCompare(msg, f->filt.mask, f->filt.exclusion, f->filt.coefficient, NEXUS_MESSAGE_FILTER_SIZE)){
                pid_state->matched_filter = f;
                BDBG_MSG(("NEXUS_SwFilter_Msg_P_MatchFilters: matched filter handle=%p, msg=%p, pid=0x%x", (void *)f, (void *)msg, pid_state->pid));
                break;
            }
        }
    }

} /*rj: this will only get the first filter that matches. */

/* Parse transport packet. Idea is from Marcus's library but only needed fields
 are extracted to reduce execution time.*/
static void NEXUS_SwFilter_Msg_P_ParsePacket(const uint8_t *buf, ts_packet_t * packet)
{

/*    BDBG_MSG(("NEXUS_SwFilter_Msg_P_ParsePacket: Entering : buffer=%p, packet=%p", (void *)buf, (void *)packet));*/

    if(0x47 != buf[0]){
        BDBG_ERR(("%s %d sync error", __FILE__, __LINE__));
    }
    packet->PID = TS_READ_16(buf+1) & 0x1FFF;
    packet->payload_unit_start_indicator = (buf[1] & 0x40) != 0;
    packet->continuity_counter = buf[3] & 0xF;
    switch(buf[3] & 0x30){
    case 0x20:
    case 0x00:
        packet->p_data_byte = NULL;
        packet->data_size = 0;
        packet->p_packet = NULL;
        break;
    case 0x10:
        packet->p_data_byte = (uint8_t *)(buf + 4);
        packet->data_size = TS_PACKET_SIZE - 4;
        packet->p_packet = (uint8_t*)buf;
        break;
    case 0x30:
        packet->p_data_byte = (uint8_t *)(buf + (buf[4] + 5));
        packet->data_size = buf + TS_PACKET_SIZE - packet->p_data_byte;
        packet->p_packet = (uint8_t*)buf;
        break;
    }

    if(buf[3] & 0x20){
        packet->discontinuity = (buf[5] & 0x80) != 0;
    }else{
        packet->discontinuity = false;
    }

    BDBG_MSG(("NEXUS_SwFilter_Msg_P_ParsePacket: p_data_byte=%p, data_size=%d, p_packet=%p", (void *)packet->p_data_byte, packet->data_size, (void *)packet->p_packet));
}

struct NEXUS_SwFilter_FilterState * NEXUS_SwFilter_Msg_P_SetFilter(NEXUS_SwFilter_MsgParams_t * params)
{
    struct NEXUS_SwFilter_FilterState * f;
    struct ts_pid * pid_state;
    int i;

    f = BLST_S_FIRST(&msg_pst.free_filter);
    if (!f) {
        f = BKNI_Malloc(sizeof(*f));
        if (!f) {
            BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
            return NULL;
        }
        BKNI_Memset(f, 0, sizeof(*f));
    }
    else {
        BLST_S_REMOVE_HEAD(&msg_pst.free_filter, next);
    }
    
    BKNI_Memcpy(f->filt.coefficient, params->filt.coefficient, NEXUS_MESSAGE_FILTER_SIZE);
    BKNI_Memcpy(f->filt.mask, params->filt.mask, NEXUS_MESSAGE_FILTER_SIZE);
    BKNI_Memcpy(f->filt.exclusion, params->filt.exclusion, NEXUS_MESSAGE_FILTER_SIZE);


    f->buffer = params->buffer;
    f->buffer_size = params->buffer_size;
    f->disable_crc_check = params->disable_crc_check;
    f->callback = params->callback;
    f->context = params->context;
    pid_state = NEXUS_SwFilter_Msg_P_FindPid(params->pid);
    if(NULL == pid_state){
        pid_state = NEXUS_SwFilter_Msg_P_AllocPid();
        if (NULL == pid_state){
            BLST_S_INSERT_HEAD(&msg_pst.free_filter, f, next);
            return NULL;
        }
        pid_state->pid = params->pid;
        pid_state->discontinuity = 1;
        pid_state->tmp_filter = 0;
        pid_state->bytes_complete = 0;
        pid_state->capture_filter = (0 == params->capture_ts) ? NULL : f;
        pid_state->matched_filter = NULL;
    }
    f->pid_state = pid_state;
    f->enabled = true;
    BLST_S_INSERT_HEAD(&pid_state->filters, f, next);

    for (i=0; i<NEXUS_MESSAGE_FILTER_SIZE; i++){
        BDBG_MSG(("NEXUS_SwFilter_Msg_P_SetFilter: coeff[%d]=%d, mask[%d]=%d, excl[%d]=%d",
            i, f->filt.coefficient[i], i, f->filt.mask[i], i, f->filt.exclusion[i]));
    }

    return f;
}

void NEXUS_SwFilter_Msg_P_RemoveFilter(struct NEXUS_SwFilter_FilterState *filter)
{
    struct ts_pid * pid_state;
    pid_state = filter->pid_state;

    BDBG_MSG(("NEXUS_SwFilter_Msg_P_RemoveFilter: filter handle %p", (void *)filter));

    filter->enabled = false;
    filter->pid_state = NULL;
    if(pid_state->matched_filter == filter){
        pid_state->matched_filter = NULL;
    }
    BLST_S_REMOVE(&pid_state->filters, filter, NEXUS_SwFilter_FilterState, next);
    BLST_S_INSERT_HEAD(&msg_pst.free_filter, filter, next);
    if(BLST_S_EMPTY(&pid_state->filters)){
        NEXUS_SwFilter_Msg_P_FreePid(pid_state);
    }
}

void NEXUS_SwFilter_Msg_P_ResetPids(void)
{
    struct ts_pid * pid_state;
    struct NEXUS_SwFilter_FilterState * f;
    while ((pid_state = BLST_S_FIRST(&msg_pst.pids))) {
        NEXUS_SwFilter_Msg_P_FreePid(pid_state);
    }
    while ((f = BLST_S_FIRST(&msg_pst.free_filter))) {
        BLST_S_REMOVE_HEAD(&msg_pst.free_filter, next);
        BKNI_Free(f);
    }
    while ((f = BLST_S_FIRST(&pst.free_filter))) {
        BLST_S_REMOVE_HEAD(&pst.free_filter, next);
        BKNI_Free(f);
    }
}

void NEXUS_SwFilter_Msg_P_CapturePacket(ts_packet_t * packet, struct ts_pid * pid_state)
{
    size_t bytes_remains;
    uint8_t * dst;
    void * new_buffer;

    BDBG_MSG(("NEXUS_SwFilter_Msg_P_CapturePacket: packet=%p, pid_state=%p", (void *)packet, (void *)pid_state));

    if(!pid_state->capture_filter->enabled){
        goto ExitFunc;
    }
    dst = (uint8_t *)(pid_state->capture_filter->buffer) + pid_state->bytes_complete;
    pid_state->bytes_complete += TS_PACKET_SIZE;
    BKNI_Memcpy(dst, packet->p_packet, TS_PACKET_SIZE);

    bytes_remains = pid_state->capture_filter->buffer_size - pid_state->bytes_complete;
    if(TS_PACKET_SIZE > bytes_remains){
        new_buffer = (pid_state->capture_filter->callback)(pid_state->capture_filter->context, pid_state->bytes_complete);
        if(NULL == new_buffer){
            pid_state->capture_filter->enabled = false;
        }else{
            pid_state->capture_filter->buffer = new_buffer;
            pid_state->bytes_complete = 0;
        }
    }

ExitFunc:
    return;
}




/************ for PES filter ********************/
void NEXUS_SwFilter_Msg_P_InitPes(void)
{
    BKNI_Memset(&pst, 0, sizeof(struct pes_state_t));
    BLST_S_INIT(&pst.free_filter);
    BLST_S_INIT(&pst.used_filter);
}


void
bdemux_pes_init(bdemux_pes *pes, uint8_t id)
{
/*  BDBG_OBJECT_INIT(pes, bdemux_pes);*/
    pes->state = bdemux_pes_state_sync;
    pes->pkt_len = 0;
    pes->pkt_offset = 0;
    pes->id = id;
    pes->ts_flags = 0;
    pes->hold_enable = false;
    pes->pes_packets = 0;
    return;
}

void
bdemux_ts_init(bdemux_ts *ts, uint16_t pid)
{
    BDBG_ASSERT(ts);
/*  BDBG_OBJECT_INIT(ts, bdemux_ts);*/
    ts->pid = pid;
    ts->continuity_counter = 0;
    ts->flags = 0;

    /* verify B_GET_BITS */
    BDBG_CASSERT(B_GET_BITS(0xDE,7,4)==0xD);
    BDBG_CASSERT(B_GET_BITS(0xDE,3,0)==0xE);
    BDBG_CASSERT(B_GET_BITS(0xDE,3,3)==0x1);
    BDBG_CASSERT(B_GET_BITS(0xDE,1,1)==0x1);
    BDBG_CASSERT(B_GET_BITS(0xDE,0,0)==0x0);
    BDBG_CASSERT(B_GET_BITS(0xDE,6,0)==0x5E);
    BDBG_CASSERT(!B_GET_BIT(0xDE,0));
    BDBG_CASSERT(B_GET_BIT(0xDE,1));
    BDBG_CASSERT(B_GET_BIT(0xDE,7));
    BDBG_CASSERT(!B_GET_BIT(0xDE,5));

    BDBG_CASSERT(B_SET_BIT( _ , 1, 0) == 1);
    BDBG_CASSERT(B_SET_BIT( _ , 2, 0) == 1);
    BDBG_CASSERT(B_SET_BIT( _ , 1, 1) == 2);
    BDBG_CASSERT(B_SET_BIT( _ , 1, 31) == (1u<<31));
    BDBG_CASSERT(B_SET_BITS( _ , 1, 0, 0) == 1);
    BDBG_CASSERT(B_SET_BITS( _ , 1, 1, 0) == 1);
    BDBG_CASSERT(B_SET_BITS( _ , 1, 1, 1) == 2);
    BDBG_CASSERT(B_SET_BITS( _ , 0x55, 7, 1) == 0xAA);
    return;
}

struct NEXUS_SwFilter_FilterState * NEXUS_SwFilter_Msg_P_SetPesFilter(NEXUS_SwFilter_MsgParams_t * params)
{
    struct NEXUS_SwFilter_FilterState * f;

    f = BLST_S_FIRST(&pst.free_filter);
    if (!f) {
        f = BKNI_Malloc(sizeof(*f));
        if (!f) {
            BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
            return NULL;
        }
        BKNI_Memset(f, 0, sizeof(*f));
    }
    else {
        BLST_S_REMOVE(&pst.free_filter, f, NEXUS_SwFilter_FilterState, next);
    }

    BDBG_MSG(("%s: params->pid=%d params->buffer_size=%u params->buffer=%p",
        __FUNCTION__,params->pid,(uint32_t)params->buffer_size,params->buffer));
    bdemux_pes_init(&f->pes, 0); /*ignore PES stream id*/
    bdemux_ts_init(&f->pes.ts,params->pid);
    f->data_size = 0;
    f->buffer = params->buffer;
    f->buffer_size = params->buffer_size;
    f->callback = params->callback;
    f->context = params->context;

    BLST_S_INSERT_HEAD(&pst.used_filter, f, next);

    return f;
}

void NEXUS_SwFilter_Msg_P_RemovePesFilter(struct NEXUS_SwFilter_FilterState * filter)
{
    BLST_S_REMOVE(&pst.used_filter, filter, NEXUS_SwFilter_FilterState, next);
    BLST_S_INSERT_HEAD(&pst.free_filter, filter, next);
}

struct NEXUS_SwFilter_FilterState * NEXUS_SwFilter_Msg_P_FindPesPid(uint16_t pid)
{
    struct NEXUS_SwFilter_FilterState * f;

    f = BLST_S_FIRST(&pst.used_filter);
    for(; f != NULL ; f = BLST_S_NEXT(f, next)){
        if(f->pes.ts.pid == pid){
            return f;
        }
    }

    return f;
}

void NEXUS_SwFilter_Msg_P_CopyPesData(struct NEXUS_SwFilter_FilterState *filter, const uint8_t *buffer, size_t size)
{
    /*printf("%s: filter->buffer=%p, filter->data_size=%d,size=%d\n",__FUNCTION__,filter->buffer,filter->data_size,size);*/


    BKNI_Memcpy((uint8_t *)filter->buffer + filter->data_size, buffer, size);
    filter->data_size += size;

#if 0 /* print all the filtered data */
        if (0) {
            unsigned i;
            BDBG_MSG(("Filtered data: size=%d", size));
            for (i=0;i<size;i++) {
                printf("%02x", buffer[i]);
            }
            printf("\n");
        }
        BDBG_MSG(("Data captured so far=%d, pkt_len=%d", filter->data_size, filter->pes.pkt_len));
#endif


    if(filter->data_size >= filter->pes.pkt_len)
    {
        /*capture one PES packet*/
        if(filter->callback)
        {
            (filter->callback)(filter->context, filter->pes.pkt_len);
        }
    }

}


/* from the TS/PES library */



bdemux_pes_result
NEXUS_SwFilter_Msg_P_bdemux_parse_pes_hdr(struct NEXUS_SwFilter_FilterState *filter, const uint8_t *pes_header, size_t len)
{
    uint32_t pts=0;
    unsigned flags = 0;
    bdemux_pes * pes = &(filter->pes);

    if( (pes_header[0]|pes_header[1])!=0x00 || pes_header[2]!=0x01 ) { /*  packet_start_code_prefix */
        BDBG_ERR(("NEXUS_SwFilter_Msg_P_bdemux_ts_data: %#lx invalid packet_start_code_prefix 0x%02x%02x%02x != 0x000001", (unsigned long)pes, pes_header[0], pes_header[1], pes_header[2]));
        return bdemux_pes_result_nomatch;
    }
    if(pes->id && pes->id!=pes_header[3]) {
        BDBG_WRN(("NEXUS_SwFilter_Msg_P_bdemux_ts_data: %#lx unknown pes id %#x(%#x)", (unsigned long)pes, pes_header[3], pes->id));
        return bdemux_pes_result_nomatch;
    }
    pes->pkt_len = B_TS_LOAD16(pes_header,4) +6;




    if(B_GET_BIT(pes_header[6], 2)) {
        flags = BDEMUX_PES_DATA_ALIGMENT;
    }
    if (B_GET_BITS(pes_header[7], 7, 6) & 2) {
        if (len<14) {
            return bdemux_pes_result_more;
        }
        pts =
         /* pts_32_30 */((uint32_t)B_GET_BITS(pes_header[9], 3, 1)<<29) |
         /* pts_29_22 */((uint32_t)pes_header[10]<<21) |
         /* pts_21_15 */((uint32_t)B_GET_BITS(pes_header[11], 7, 1)<<14) |
         /* pts_14_7 */((uint32_t)pes_header[12]<<6) |
         /* pts_6_0 */B_GET_BITS(pes_header[13], 7, 2);
        flags |= BDEMUX_PES_PTS;
    }
    pes->pes_packets++;
    pes->pkt_data_offset = 9+pes_header[8]; /* PES_header_data_length */

    filter->data_size = 0;/*reset PES data size*/
    return bdemux_pes_result_match;
}


void
NEXUS_SwFilter_Msg_P_NEXUS_SwFilter_Msg_P_bdemux_ts_data(struct NEXUS_SwFilter_FilterState *filter, unsigned flags, const uint8_t *data, size_t len)
{
    bdemux_pes *pes = &(filter->pes);

    BDBG_ASSERT(data);
    if (len == 0)
        return;
    /*BDBG_MSG(("NEXUS_SwFilter_Msg_P_bdemux_ts_data: %#lx %#lx:%u:%#x %s%s%s%s",(unsigned long)ts, (unsigned long)data, len, flags, B_STR_FLAG(flags&BDEMUX_TS_ERROR, ERR), B_STR_FLAG(flags&BDEMUX_TS_DISCONTINUITY, DISC),B_STR_FLAG(flags&BDEMUX_TS_MARKED_DISCONTINUITY, MDISC), B_STR_FLAG(flags&BDEMUX_TS_PAYLOAD_UNIT_START,START)));*/
    /*BDBG_ERR("NEXUS_SwFilter_Msg_P_bdemux_ts_data: [%02X, %02X,%02X,%02X,%02X,%02X]:%u:%#x %s%s%s%s", data[0], data[1],data[2],data[3],data[4],data[5], len, flags, B_STR_FLAG(flags&BDEMUX_TS_ERROR, ERR), B_STR_FLAG(flags&BDEMUX_TS_DISCONTINUITY, DISC),B_STR_FLAG(flags&BDEMUX_TS_MARKED_DISCONTINUITY, MDISC), B_STR_FLAG(flags&BDEMUX_TS_PAYLOAD_UNIT_START,START));*/

    /* ISO/IEC 13818-1 */
    /* Table 2-18 -- PES packet */
    if(flags&BDEMUX_TS_PAYLOAD_UNIT_START) {
        switch(NEXUS_SwFilter_Msg_P_bdemux_parse_pes_hdr(filter, data, len)) {
        case bdemux_pes_result_nomatch:
            BDBG_ERR(("NEXUS_SwFilter_Msg_P_bdemux_ts_data: pes hdr nomatch"));
            pes->state = bdemux_pes_state_sync;
            return ;
        case bdemux_pes_result_more:
            BDBG_WRN(("NEXUS_SwFilter_Msg_P_bdemux_ts_data:%#lx partial pes header %u", (unsigned long)pes, (uint32_t)len));
            break ;
        case bdemux_pes_result_match:
            break;
        }
        pes->state = bdemux_pes_state_data;
    }
    if(pes->state == bdemux_pes_state_data) {
        NEXUS_SwFilter_Msg_P_CopyPesData(filter, data, len);
    }
}




/* from the TS/PES library */

int
NEXUS_SwFilter_Msg_P_bdemux_ts_feed(struct NEXUS_SwFilter_FilterState *filter, const uint8_t *data, size_t len)
{
    bdemux_ts *ts = &(filter->pes.ts);
    size_t left;
    unsigned ts_continuity_counter;

    BDBG_ASSERT(len>0);
    BDBG_ASSERT((len%B_TS_PKT_LEN)==0);

    /* ISO/IEC 13818-1 */

    ts_continuity_counter = ts->continuity_counter;
    for(left=len;left>=B_TS_PKT_LEN;left-=B_TS_PKT_LEN,data+=B_TS_PKT_LEN) {
        uint32_t word;
        unsigned pid;
        unsigned adaptation_field_control;
        unsigned continuity_counter;
        unsigned off; /* offset into the transport packet */
        unsigned flags = ts->flags;

        if (data[0]!=B_TS_SYNC) {
            goto err_out_of_sync;
        }
        BDBG_ASSERT(data[0]==B_TS_SYNC);

        /* Table 2-3 -- ITU-T Rec. H.222.0 | ISO/IEC 13818 transport packet */
        word = B_TS_LOAD16(data, 1);
        pid = B_GET_BITS(word, 12, 0);
        if (pid!=ts->pid) {
            BDBG_ERR(("bdemux_ts_feed: %#lx wrong pid %#x(%#x)", (unsigned long)ts, (unsigned)pid, (unsigned)ts->pid));
            goto wrong_pid;
        }
        if (B_GET_BIT(word, 15) /* transport_error_indicator */) {
            goto ts_error;
        }
        if (B_GET_BIT(word, 14) /* payload_unit_start_indicator */) {
            flags |= BDEMUX_TS_PAYLOAD_UNIT_START;
        }
        word = B_TS_LOAD8(data, 3);
        continuity_counter = B_GET_BITS(word, 3, 0);
        adaptation_field_control = B_GET_BITS(word, 5, 4);

        /*printf("%s: continuity_counter=%d ts_continuity_counter=%d,adaptation_field_control=%d",__FUNCTION__, continuity_counter,ts_continuity_counter,adaptation_field_control);*/
        if (continuity_counter != ts_continuity_counter) {
            goto cc_error;
        }
after_cc:
        off = 4;
        if (adaptation_field_control==0x02 || adaptation_field_control==0x03) {
            goto adaptation_payload;
        }
after_adaptation_payload:
        if (adaptation_field_control==0x01) {
            NEXUS_SwFilter_Msg_P_NEXUS_SwFilter_Msg_P_bdemux_ts_data(filter, flags, data+off, B_TS_PKT_LEN-off);
            flags = 0; /* clear flags */
        }
next_cc:
        ts_continuity_counter = B_NEXT_CONT_COUNTER(continuity_counter);
done:
        if(flags & BDEMUX_TS_PAYLOAD_UNIT_START) {
            BDBG_MSG(("bdemux_ts_feed: %#lx clearing PUSI bit", (unsigned long)ts));
            flags &= ~BDEMUX_TS_PAYLOAD_UNIT_START; /* clearing PUSI bit, this code should only execute if there was a TS packet with PUSI bit, but without payload */
        }
        ts->flags = flags;
wrong_pid:
        ;
        continue;
ts_error:
        BDBG_ERR(("bdemux_ts_feed: %#lx transport_error_indicator", (unsigned long)ts));
        flags |= BDEMUX_TS_ERROR;
        goto done;
cc_error:
        if (!(adaptation_field_control==0x00 || adaptation_field_control==0x02)) {
            if(B_NEXT_CONT_COUNTER(continuity_counter)==ts_continuity_counter) {
                BDBG_ERR(("bdemux_ts_feed: %#lx continuity counter duplicate %#x(%#x)", (unsigned long)ts, (unsigned)continuity_counter, (unsigned)ts->continuity_counter));
                goto done;
            }
            BDBG_MSG(("bdemux_ts_feed: %#lx continuity counter error %#x(%#x)", (unsigned long)ts, (unsigned)continuity_counter, (unsigned)ts->continuity_counter));
            flags |= BDEMUX_TS_DISCONTINUITY;
        }
        goto after_cc;
adaptation_payload:
        {
            unsigned adaptation_field_length = B_TS_LOAD8(data, 4);
            if (adaptation_field_length>0) {
                word = B_TS_LOAD8(data, 5);
                if (B_GET_BIT(word, 7) /* discontinuity_indicator  */) {
                    flags |= BDEMUX_TS_MARKED_DISCONTINUITY;
                }
            }
            off = 5 + adaptation_field_length;
            if (off>B_TS_PKT_LEN) {
                BDBG_ERR(("bdemux_ts_feed: %#lx invalid adaptation_field_length=%u", (unsigned long)ts, adaptation_field_length));
                goto next_cc;
            }
            adaptation_field_control &= ~2; /* clear bit 1, e.g. 3 would become 1 */
        }
        goto after_adaptation_payload;
    }
    ts->continuity_counter = ts_continuity_counter;
    BDBG_ASSERT(len>=left);
    return len-left;
err_out_of_sync:
    BDBG_ERR(("out of_sync %02x:%02x:%02x:%02x(%#lx) at %u:%u", data[0], data[1], data[2], data[3], (unsigned long)data, (uint32_t)(len-left), (uint32_t)len));
    ts->continuity_counter = ts_continuity_counter;
    if(len==left) {
        return -B_TS_PKT_LEN;
    }
    return -(len-left);
}


size_t NEXUS_SwFilter_Msg_P_FeedPes(uint8_t * buffer, size_t size)
{
    size_t consumed;
    uint8_t * walker;
    struct NEXUS_SwFilter_FilterState* filter;

    /*printf("%s: size=%d buffer[0]=0x%x",__FUNCTION__,size,buffer[0]);*/

    consumed = 0;
    walker = buffer;

    while(size - consumed >= B_TS_PKT_LEN){
        uint32_t word;
        unsigned pid;

        if (walker[0]!=B_TS_SYNC) {
            BDBG_ERR(("%s %d sync error", __FILE__, __LINE__));
            consumed = size;
            goto ExitFunc;
        }

        word = B_TS_LOAD16(walker, 1);
        pid = B_GET_BITS(word, 12, 0);

        filter = NEXUS_SwFilter_Msg_P_FindPesPid(pid);
        if(NULL == filter){
            BDBG_ERR(("%s %d sync error", __FILE__, __LINE__));
            consumed = size;
            goto ExitFunc;
        }

        NEXUS_SwFilter_Msg_P_bdemux_ts_feed(filter, walker, B_TS_PKT_LEN);
        walker += B_TS_PKT_LEN;
        consumed += B_TS_PKT_LEN;
        pst.packet_count++;
    }
ExitFunc:
    return consumed;
}













/************************ CRC ****************/



static const uint32_t bcrc32_table[256] = {
  0x00000000, 0x04c11db7, 0x09823b6e, 0x0d4326d9,
  0x130476dc, 0x17c56b6b, 0x1a864db2, 0x1e475005,
  0x2608edb8, 0x22c9f00f, 0x2f8ad6d6, 0x2b4bcb61,
  0x350c9b64, 0x31cd86d3, 0x3c8ea00a, 0x384fbdbd,
  0x4c11db70, 0x48d0c6c7, 0x4593e01e, 0x4152fda9,
  0x5f15adac, 0x5bd4b01b, 0x569796c2, 0x52568b75,
  0x6a1936c8, 0x6ed82b7f, 0x639b0da6, 0x675a1011,
  0x791d4014, 0x7ddc5da3, 0x709f7b7a, 0x745e66cd,
  0x9823b6e0, 0x9ce2ab57, 0x91a18d8e, 0x95609039,
  0x8b27c03c, 0x8fe6dd8b, 0x82a5fb52, 0x8664e6e5,
  0xbe2b5b58, 0xbaea46ef, 0xb7a96036, 0xb3687d81,
  0xad2f2d84, 0xa9ee3033, 0xa4ad16ea, 0xa06c0b5d,
  0xd4326d90, 0xd0f37027, 0xddb056fe, 0xd9714b49,
  0xc7361b4c, 0xc3f706fb, 0xceb42022, 0xca753d95,
  0xf23a8028, 0xf6fb9d9f, 0xfbb8bb46, 0xff79a6f1,
  0xe13ef6f4, 0xe5ffeb43, 0xe8bccd9a, 0xec7dd02d,
  0x34867077, 0x30476dc0, 0x3d044b19, 0x39c556ae,
  0x278206ab, 0x23431b1c, 0x2e003dc5, 0x2ac12072,
  0x128e9dcf, 0x164f8078, 0x1b0ca6a1, 0x1fcdbb16,
  0x018aeb13, 0x054bf6a4, 0x0808d07d, 0x0cc9cdca,
  0x7897ab07, 0x7c56b6b0, 0x71159069, 0x75d48dde,
  0x6b93dddb, 0x6f52c06c, 0x6211e6b5, 0x66d0fb02,
  0x5e9f46bf, 0x5a5e5b08, 0x571d7dd1, 0x53dc6066,
  0x4d9b3063, 0x495a2dd4, 0x44190b0d, 0x40d816ba,
  0xaca5c697, 0xa864db20, 0xa527fdf9, 0xa1e6e04e,
  0xbfa1b04b, 0xbb60adfc, 0xb6238b25, 0xb2e29692,
  0x8aad2b2f, 0x8e6c3698, 0x832f1041, 0x87ee0df6,
  0x99a95df3, 0x9d684044, 0x902b669d, 0x94ea7b2a,
  0xe0b41de7, 0xe4750050, 0xe9362689, 0xedf73b3e,
  0xf3b06b3b, 0xf771768c, 0xfa325055, 0xfef34de2,
  0xc6bcf05f, 0xc27dede8, 0xcf3ecb31, 0xcbffd686,
  0xd5b88683, 0xd1799b34, 0xdc3abded, 0xd8fba05a,
  0x690ce0ee, 0x6dcdfd59, 0x608edb80, 0x644fc637,
  0x7a089632, 0x7ec98b85, 0x738aad5c, 0x774bb0eb,
  0x4f040d56, 0x4bc510e1, 0x46863638, 0x42472b8f,
  0x5c007b8a, 0x58c1663d, 0x558240e4, 0x51435d53,
  0x251d3b9e, 0x21dc2629, 0x2c9f00f0, 0x285e1d47,
  0x36194d42, 0x32d850f5, 0x3f9b762c, 0x3b5a6b9b,
  0x0315d626, 0x07d4cb91, 0x0a97ed48, 0x0e56f0ff,
  0x1011a0fa, 0x14d0bd4d, 0x19939b94, 0x1d528623,
  0xf12f560e, 0xf5ee4bb9, 0xf8ad6d60, 0xfc6c70d7,
  0xe22b20d2, 0xe6ea3d65, 0xeba91bbc, 0xef68060b,
  0xd727bbb6, 0xd3e6a601, 0xdea580d8, 0xda649d6f,
  0xc423cd6a, 0xc0e2d0dd, 0xcda1f604, 0xc960ebb3,
  0xbd3e8d7e, 0xb9ff90c9, 0xb4bcb610, 0xb07daba7,
  0xae3afba2, 0xaafbe615, 0xa7b8c0cc, 0xa379dd7b,
  0x9b3660c6, 0x9ff77d71, 0x92b45ba8, 0x9675461f,
  0x8832161a, 0x8cf30bad, 0x81b02d74, 0x857130c3,
  0x5d8a9099, 0x594b8d2e, 0x5408abf7, 0x50c9b640,
  0x4e8ee645, 0x4a4ffbf2, 0x470cdd2b, 0x43cdc09c,
  0x7b827d21, 0x7f436096, 0x7200464f, 0x76c15bf8,
  0x68860bfd, 0x6c47164a, 0x61043093, 0x65c52d24,
  0x119b4be9, 0x155a565e, 0x18197087, 0x1cd86d30,
  0x029f3d35, 0x065e2082, 0x0b1d065b, 0x0fdc1bec,
  0x3793a651, 0x3352bbe6, 0x3e119d3f, 0x3ad08088,
  0x2497d08d, 0x2056cd3a, 0x2d15ebe3, 0x29d4f654,
  0xc5a92679, 0xc1683bce, 0xcc2b1d17, 0xc8ea00a0,
  0xd6ad50a5, 0xd26c4d12, 0xdf2f6bcb, 0xdbee767c,
  0xe3a1cbc1, 0xe760d676, 0xea23f0af, 0xeee2ed18,
  0xf0a5bd1d, 0xf464a0aa, 0xf9278673, 0xfde69bc4,
  0x89b8fd09, 0x8d79e0be, 0x803ac667, 0x84fbdbd0,
  0x9abc8bd5, 0x9e7d9662, 0x933eb0bb, 0x97ffad0c,
  0xafb010b1, 0xab710d06, 0xa6322bdf, 0xa2f33668,
  0xbcb4666d, 0xb8757bda, 0xb5365d03, 0xb1f740b4
};

uint32_t bcrc32(uint8_t * data, unsigned len)
{
  uint32_t crc = ~0;

  while (len--) {
    crc = bcrc32_table[(crc >> 24) ^ (uint32_t)(*data++)] ^ (crc << 8);
  }
  return crc;
}
