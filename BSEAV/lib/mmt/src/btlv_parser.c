/***************************************************************************
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
 **************************************************************************/

#include "bstd.h"
#include "bmedia_util.h"
#include "biobits.h"
#include "btlv_parser.h"
#include "bkni.h"


BDBG_MODULE(btlv_parser);

#define BDBG_MSG_TRACE(x)  BDBG_MSG(x)
#define BDBG_TRACE_ERROR(x) (BDBG_LOG(("Error: %s (%s,%u)",#x,__FILE__,__LINE__)),x)

#define B_MPEG2TS_PKT_LEN       BMPEG2TS_PKT_LEN
#define B_MPEG2TS_TLV_PKT_HDR   3
#define B_TLV_PKT_HDR           4


int btlv_parser_split_mpeg2ts(uint16_t pid, const void *payload, btlv_segments *segments) {
    const uint8_t *data = payload;
    unsigned word;
    unsigned tlv_pid;

    /* JCTEA STD-007-6.0
     * Figure 0.5-2 the structure of divided TLV packets. */

    /* Recommendation ITU-R BT.1869  TABLE 1 TLV container */

    if(data[0]!=0x47) { /* sync_byte */
        return BDBG_TRACE_ERROR(BTLV_RESULT_TS_SYNC_ERROR);
    }
    word = B_MEDIA_LOAD_UINT16_BE(data, 1);
    if(B_GET_BIT(word, 15)) { /*  transport_error_indicator */
        return BTLV_RESULT_TRANSPORT_ERROR;
    }
    tlv_pid = B_GET_BITS(word,12,0);
    if(tlv_pid!=pid) {
        return BTLV_RESULT_UNKNOWN_PID;
    }
    if(B_GET_BIT(word,13)) { /* '0' */
        return BDBG_TRACE_ERROR(BTLV_RESULT_FORMAT_ERROR);
    }
    if(!B_GET_BIT(word,14)) { /* !tlv_pkt_start_indicator */
        segments->tail.size = 0;
        segments->tail.offset = 0;
        segments->head.offset = B_MPEG2TS_TLV_PKT_HDR;
        segments->head.size = B_MPEG2TS_PKT_LEN - B_MPEG2TS_TLV_PKT_HDR;
        segments->count = 0;
        BDBG_MSG_TRACE(("head %u:%u", (unsigned)segments->head.offset, (unsigned)segments->head.size));
    } else {
        unsigned count;
        unsigned tlv_pkt_start_pos;
        unsigned offset;
        tlv_pkt_start_pos = data[B_MPEG2TS_TLV_PKT_HDR];
        BDBG_MSG_TRACE(("tlv_pkt_start_pos:%u", tlv_pkt_start_pos));
        offset = B_MPEG2TS_TLV_PKT_HDR + 1 + tlv_pkt_start_pos;
        if(offset>B_MPEG2TS_PKT_LEN) {
            return BDBG_TRACE_ERROR(BTLV_RESULT_FORMAT_ERROR);
        }
        segments->head.size = tlv_pkt_start_pos;
        segments->head.offset = 4;
        BDBG_MSG_TRACE(("head %u:%u", (unsigned)segments->head.offset, (unsigned)segments->head.size));
        count = 0;
        while(offset < (B_MPEG2TS_PKT_LEN-B_TLV_PKT_HDR)) {
            /* TLV packet output sequence */
            word = data[offset];
            if(B_GET_BITS(word,7,6)!=0x1) { /*  sync_bits */
                return BDBG_TRACE_ERROR(BTLV_RESULT_FORMAT_ERROR);
            }
            if(B_GET_BITS(word,5,0)!=0x3F) { /* '111111' */
                return BDBG_TRACE_ERROR(BTLV_RESULT_FORMAT_ERROR);
            }
            word = B_MEDIA_LOAD_UINT16_BE(data, offset+2);
            word += B_TLV_PKT_HDR;
            if(offset + word>B_MPEG2TS_PKT_LEN) { /* TLV packet spans to next TS packet */
                break;
            }
            if(count>=sizeof(segments->packets)/sizeof(segments->packets[0])) {
                return BDBG_TRACE_ERROR(BTLV_RESULT_FORMAT_ERROR);
            }
            segments->packets[count].offset = offset;
            segments->packets[count].size = word;
            BDBG_MSG_TRACE(("packet[%u] %u:%u", count, (unsigned)segments->packets[count].offset, (unsigned)segments->packets[count].size));
            count++;
            offset+=word;
        }
        segments->tail.offset = offset;
        segments->tail.size = B_MPEG2TS_PKT_LEN - offset;
        segments->count = count;
        BDBG_MSG_TRACE(("tail %u:%u", (unsigned)segments->tail.offset, (unsigned)segments->tail.size));
    }
    return 0;
}


void btlv_parser_reset(btlv_parser *parser)
{
    parser->fragments = 0;
    parser->packet_size = 0;
    parser->expected_packet_size = 0;
    parser->in_sync = false;
    parser->eps_higher_byte = 0;
    parser->tail_size = 0;
    return;
}

void btlv_parser_init(btlv_parser *parser, uint16_t pid)
{
    BKNI_Memset(parser, 0, sizeof(*parser));
    parser->pid = pid;
    btlv_parser_reset(parser);
    return;
}

void btlv_parser_shutdown(btlv_parser *parser)
{
    BSTD_UNUSED(parser);
    return;
}

int btlv_parser_process_tlv(btlv_parser *parser,const void *payload, unsigned payload_length, btlv_parser_packets *packets)
{
    int rc=0;
    const uint8_t *data = payload;
    unsigned offset=0;
    uint16_t remaining_packet_size = 0;
    uint8_t remaining_tlv_hdr_size=0;
    packets->packet_valid = false;
    packets->count = 0;
    packets->keep_previous_packets = false;
    packets->keep_current_packet = false;

    if (!parser->in_sync)
    {
        for(offset=0; (data[offset] != 0x7F) && (offset < payload_length); offset++);

        if(offset == payload_length)
            return 0;
        else
        {
            BDBG_ASSERT((data[offset]== 0x7F));
            parser->in_sync = true;
            BDBG_ERR(("offset %x",offset));
        }
    }

    if (parser->tail_size)
    {

        BDBG_ASSERT((parser->fragments == 1));
        parser->packet[0] = parser->tail_pkt;
        if (parser->expected_packet_size == 0)
        {
            BDBG_ASSERT(((parser->tail_size > 0) && (parser->tail_size < 4) ));
            switch (parser->tail_size)
            {
            case 1:
                parser->expected_packet_size = data[offset+1] << 8 | data[offset+2];
                break;
            case 2:
                parser->expected_packet_size = data[offset] << 8 | data[offset+1];
                break;
            case 3:
                parser->expected_packet_size = parser->eps_higher_byte << 8 | data[offset];
                break;
            default:
                BDBG_ASSERT((0));
            }
            remaining_tlv_hdr_size = B_TLV_PKT_HDR - parser->tail_size;
            remaining_packet_size = parser->expected_packet_size + remaining_tlv_hdr_size;
            parser->eps_higher_byte = 0;
        }
        else
        {
            remaining_packet_size = parser->expected_packet_size - parser->packet_size;
            remaining_tlv_hdr_size=0;
        }

    }

    if (parser->expected_packet_size)
    {
        uint16_t current_packet_size=0;
        if (parser->tail_size)
        {
            parser->tail_size = 0;
        }
        else
        {
            remaining_packet_size = parser->expected_packet_size - parser->packet_size;
            remaining_tlv_hdr_size=0;
        }
        current_packet_size = (offset + remaining_packet_size < payload_length)?remaining_packet_size:payload_length;
        BATOM_VEC_INIT(&parser->packet[parser->fragments],
                       data+offset,
                       current_packet_size);
        offset += current_packet_size;
        parser->fragments++;
        parser->packet_size+=current_packet_size;
        parser->packet_size-=remaining_tlv_hdr_size;
        if (parser->expected_packet_size == parser->packet_size)
        {
            batom_cursor_from_vec(&packets->packet, parser->packet, parser->fragments);
            parser->fragments = 0;
            parser->expected_packet_size = 0;
            parser->packet_size = 0;
            packets->packet_valid = true;
        }
        else
        {
            packets->keep_previous_packets = true;
        }

    }
    BDBG_ASSERT((packets->count==0));

    while((offset + B_TLV_PKT_HDR < payload_length))
    {
        uint16_t packet_size=0;
        if (data[offset]!=0x7f)
        {
            parser->in_sync = false;
            goto done;
        }
        packet_size = data[offset + 2] << 8  | data[offset+3];
        if ((offset + B_TLV_PKT_HDR + packet_size) <= payload_length)
        {
            BATOM_VEC_INIT(&packets->packets[packets->count], data+offset,B_TLV_PKT_HDR + packet_size);
            offset += B_TLV_PKT_HDR + packet_size;
            packets->count++;
        }
        else
        {
            break;
        }
    }

    #if 0
    if (parser->fragments) {

        BDBG_ERR(("parser->expected_packet_size %d parser->packet_size %d",parser->expected_packet_size,parser->packet_size));
        BDBG_ERR(("offset %d payload_length %d",offset,payload_length));
        BDBG_ERR(("parser->fragments %d packets->count %d",parser->fragments,packets->count));
    }
    #endif
    BDBG_ASSERT((parser->fragments==0));
    if (offset < payload_length)
    {
        if (data[offset]!=0x7f)
        {
            parser->in_sync = false;
            goto done;
        }
        parser->tail_size = payload_length - offset;
        BATOM_VEC_INIT(&parser->tail_pkt, data+offset,parser->tail_size);
        if((offset + B_TLV_PKT_HDR) > payload_length)
        {
            parser->expected_packet_size = 0;
            parser->packet_size = 0;
            if (parser->tail_size == 3)
            {
                parser->eps_higher_byte = data[offset + 2];
            }
        }
        else
        {
            parser->expected_packet_size = data[offset + 2] << 8 | data[offset + 3];
            parser->packet_size = parser->tail_size - B_TLV_PKT_HDR;
        }
        packets->keep_current_packet = true;
        offset += parser->tail_size;
        parser->fragments = 1;
    }

done:
    if (parser->in_sync)
    {
        BDBG_ASSERT((offset == payload_length));
    }
    else
    {
        parser->fragments = 0;
        parser->expected_packet_size = 0;
        parser->packet_size = 0;
        parser->tail_size = 0;
        parser->eps_higher_byte =0;
    }
    return rc;
}

int btlv_parser_process_mpeg2ts(btlv_parser *parser, const void *payload, btlv_parser_packets *packets)
{
    int rc;
    const uint8_t *data = payload;
    rc = btlv_parser_split_mpeg2ts(parser->pid, data, &parser->segments);
    if(rc!=0) {
        return rc;
    }
    packets->packet_valid = false;
    packets->count = 0;
    packets->keep_previous_packets = false;
    packets->keep_current_packet = false;
    if(parser->segments.head.size) {
        if(parser->fragments!=0) {
            if(parser->fragments>=sizeof(parser->packet)/sizeof(parser->packet[0])) {
                return BDBG_TRACE_ERROR(BTLV_RESULT_FORMAT_ERROR);
            }
            packets->keep_previous_packets = true;
            BATOM_VEC_INIT(&parser->packet[parser->fragments], data+parser->segments.head.offset,parser->segments.head.size);
            parser->fragments++;
            parser->packet_size += parser->segments.head.size;
            if(parser->packet_size >= B_TLV_PKT_HDR) {
                if(parser->expected_packet_size==0) {
                    batom_cursor cursor;
                    int word;
                    parser->packet[0] = parser->tail_pkt;
                    batom_cursor_from_vec(&cursor, parser->packet, parser->fragments);
                    if(0) batom_cursor_dump(&cursor, "tlv");
                    BATOM_NEXT(word,&cursor);
                    if(word==BATOM_EOF) {
                        return BDBG_TRACE_ERROR(BTLV_RESULT_UNKNOWN);
                    }
                    /* Recommendation ITU-R BT.1869  TABLE 1 TLV container */
                    if(B_GET_BITS(word,7,6)!=0x1) { /*  sync_bits */
                        return BDBG_TRACE_ERROR(BTLV_RESULT_FORMAT_ERROR);
                    }
                    if(B_GET_BITS(word,5,0)!=0x3F) { /* '111111' */
                        return BDBG_TRACE_ERROR(BTLV_RESULT_FORMAT_ERROR);
                    }
                    BATOM_NEXT(word,&cursor);
                    parser->expected_packet_size = batom_cursor_uint16_be(&cursor);
                    BDBG_MSG_TRACE(("expected_packet_size %u", parser->expected_packet_size));
                    if(BATOM_IS_EOF(&cursor)) {
                        return BDBG_TRACE_ERROR(BTLV_RESULT_UNKNOWN);
                    }
                    parser->expected_packet_size += B_TLV_PKT_HDR;
                }
                BDBG_MSG_TRACE(("packet_size:%u expected_packet_size:%u", parser->packet_size, parser->expected_packet_size));
                if(parser->packet_size == parser->expected_packet_size) {
                    packets->packet_valid = true;
                    packets->keep_previous_packets = false;
                    batom_cursor_from_vec(&packets->packet, parser->packet, parser->fragments);
                } else if(parser->packet_size > parser->expected_packet_size) {
                    return BDBG_TRACE_ERROR(BTLV_RESULT_PACKET_ERROR);
                }
            }
        } else { /* if(parser->fragments!=0) { */
            BDBG_MSG(("dropping partial payload %u", parser->segments.head.size));
            if(parser->in_sync) {
                return BDBG_TRACE_ERROR(BTLV_RESULT_PACKET_ERROR);
            }
        }
    }
    if(parser->segments.count) {
        unsigned i;
        parser->in_sync = true;
        if(parser->fragments) {
            if(parser->packet_size != parser->expected_packet_size) {
                return BDBG_TRACE_ERROR(BTLV_RESULT_UNKNOWN);
            }
            parser->fragments = 0;
        }
        for(i=0;i<parser->segments.count;i++) {
            if(i>=sizeof(packets->packets)/sizeof(packets->packets[0])) {
                return BDBG_TRACE_ERROR(BTLV_RESULT_UNKNOWN);
            }
            BATOM_VEC_INIT(&packets->packets[i], data+parser->segments.packets[i].offset,parser->segments.packets[i].size);
        }
        packets->count = i;
    }
    if(parser->segments.tail.size) {
        parser->in_sync = true;
        if(parser->fragments) {
            if(parser->packet_size != parser->expected_packet_size) {
                return BDBG_TRACE_ERROR(BTLV_RESULT_UNKNOWN);
            }
        }
        BATOM_VEC_INIT(&parser->tail_pkt, data+parser->segments.tail.offset,parser->segments.tail.size);
        parser->packet_size = parser->segments.tail.size;
        parser->expected_packet_size = 0;
        parser->fragments = 1;
        packets->keep_current_packet = true;
    }
    return 0;
}

int btlv_parser_parse_compressed_ip_packet(batom_cursor *cursor, btlv_compressed_ip_packet *packet)
{
    unsigned word;
    int cid_header_type;

    /* Recommendation ITU-R BT.1869 TABLE 3 Header compressed IP packet */
    word = batom_cursor_uint16_be(cursor);
    if(BATOM_IS_EOF(cursor)) {
        return BDBG_TRACE_ERROR(BTLV_RESULT_TLV_ERROR);
    }
    packet->cid = B_GET_BITS(word, 15, 4);
    packet->sn = B_GET_BITS(word, 3, 0);
    BATOM_NEXT(cid_header_type, cursor);
    if(cid_header_type==BATOM_EOF) {
        return BDBG_TRACE_ERROR(BTLV_RESULT_TLV_ERROR);
    }
    BDBG_MSG_TRACE(("TLV compressed IP:%#x cid:%#x sn:%#x", (unsigned)cid_header_type, (unsigned)packet->cid, (unsigned)packet->sn));
    packet->cid_header_type = cid_header_type;
    switch(cid_header_type) {
    case BTLV_COMPRESSED_IPV4:
        if(batom_cursor_copy(cursor, packet->header.ipv4.ipv4, sizeof(packet->header.ipv4.ipv4)) != sizeof(packet->header.ipv4.ipv4)) {
            return BDBG_TRACE_ERROR(BTLV_RESULT_TLV_ERROR);
        }
        if(batom_cursor_copy(cursor, packet->header.ipv4.udp, sizeof(packet->header.ipv4.udp)) != sizeof(packet->header.ipv4.udp)) {
            return BDBG_TRACE_ERROR(BTLV_RESULT_TLV_ERROR);
        }
        break;
    case BTLV_COMPRESSED_IPV6:
        if(0) {batom_cursor_dump(cursor, "IPV6 HEADER");}
        if(batom_cursor_copy(cursor, packet->header.ipv6.ipv6, sizeof(packet->header.ipv6.ipv6)) != sizeof(packet->header.ipv6.ipv6)) {
            return BDBG_TRACE_ERROR(BTLV_RESULT_TLV_ERROR);
        }
        if(0) {batom_cursor_dump(cursor, "IPV6 UDP");}
        if(batom_cursor_copy(cursor, packet->header.ipv6.udp, sizeof(packet->header.ipv6.udp)) != sizeof(packet->header.ipv6.udp)) {
            return BDBG_TRACE_ERROR(BTLV_RESULT_TLV_ERROR);
        }
        if(0) {batom_cursor_dump(cursor, "IPV6 payload");}
        break;
    case BTLV_COMPRESSED_IPV4_RAW:
        break;
    case BTLV_COMPRESSED_IPV6_RAW:
        break;
    default:
        return BDBG_TRACE_ERROR(BTLV_RESULT_UNKNOWN_TYPE);
    }
    return 0;
}

void btlv_ip_parser_reset(btlv_ip_parser *parser)
{
    unsigned i;
    for(i=0;i<sizeof(parser->cid)/sizeof(parser->cid[0]);i++) {
        if(parser->cid[i].header) {
            BKNI_Free(parser->cid[i].header);
            parser->cid[i].header=NULL;
        }
    }
    return;
}

void btlv_ip_parser_init(btlv_ip_parser *parser)
{
    BKNI_Memset(parser, 0, sizeof(*parser));
    return;
}

static uint16_t b_tlv_ip_checksum(batom_cursor *cursor)
{
    unsigned sum = 0;

#if 1
    /* RFC 1071 4.1 "C" */
    for(;;) {
        int data;
        batom_checkpoint chk;
        BATOM_SAVE(cursor, &chk);

        data = batom_cursor_uint16_be(cursor);
        if(BATOM_IS_EOF(cursor)) {
            BATOM_ROLLBACK(cursor, &chk);
            BATOM_NEXT(data,cursor);
            if(data!=BATOM_EOF) {
                sum += (data<<8);
            }
            break;
        }
        sum +=  data;
    }

    while(sum>>16) {
        sum = (sum & 0xFFFF) + (sum>>16);
    }
#endif
    return B_GET_BITS(~sum,15,0);
}


static int btlv_ip_parser_reconstruct_ipv4(btlv_ip_parser *parser, const btlv_compressed_ip_packet *header, batom_cursor *payload, btlv_ip_parser_result *result)
{
    unsigned payload_length = batom_cursor_size(payload);
    size_t payload_vecs;
    uint16_t checksum;
    batom_cursor cursor;

    if(0) {batom_cursor_dump(payload, "btlv_ip_parser_reconstruct_ipv6");}
    if(0) {batom_range_dump(header->header.ipv4.ipv4, sizeof(header->header.ipv4.ipv4), "header.ipv4.ipv4");}

    payload_length += BTLV_UDP_HEADER_SIZE;
    B_MEDIA_SAVE_UINT16_BE(parser->temp.payload_length,payload_length);
    B_MEDIA_SAVE_UINT16_BE(parser->temp.total_length,payload_length+BTLV_IPV4_HEADER_SIZE);
    B_MEDIA_SAVE_UINT16_BE(parser->temp.ip_checksum,0);

    /* UDP "pseudo-header" */
    BATOM_VEC_INIT(&parser->temp.ip_packet[0], header->header.ipv4.ipv4 + BTLV_IPV4_ADDRESS_OFFSET-(BTLV_IPV4_TOTAL_LENGTH_SIZE + BTLV_IPV4_HEADER_CHECKSUM_SIZE), BTLV_IPV4_ADDRESS_SIZE);
    BATOM_VEC_INIT(&parser->temp.ip_packet[1], parser->temp.zero_24, 1);
    BATOM_VEC_INIT(&parser->temp.ip_packet[2], header->header.ipv4.ipv4 + BTLV_IPV4_PROTOCOL_OFFSET - (BTLV_IPV4_TOTAL_LENGTH_SIZE),  BTLV_IPV4_PROTOCOL_SIZE);
    BATOM_VEC_INIT(&parser->temp.ip_packet[3], parser->temp.payload_length, sizeof(parser->temp.payload_length));
#if 0
    batom_cursor_from_vec(&cursor, parser->temp.ip_packet, 4);
    if(1) { batom_cursor_dump(&cursor, "HEADER"); }
#endif

    /* UDP header */
    BATOM_VEC_INIT(&parser->temp.ip_packet[4], header->header.ipv4.udp, BTLV_UDP_LENGTH_OFFSET);
    BATOM_VEC_INIT(&parser->temp.ip_packet[5], parser->temp.payload_length, sizeof(parser->temp.payload_length));

    batom_cursor_extract(payload, &parser->temp.ip_packet[6], sizeof(parser->temp.ip_packet)/sizeof(parser->temp.ip_packet[0]) - 6, &payload_vecs);
    batom_cursor_from_vec(&cursor, parser->temp.ip_packet, payload_vecs + 6);
    checksum = b_tlv_ip_checksum(&cursor);
    if(0) BDBG_LOG(("UDP checksum %#x", checksum));
    B_MEDIA_SAVE_UINT16_BE(parser->temp.udp_checksum,checksum);


    /* 2. Create IPV4 packet */
    /* IPV4 header */
    BATOM_VEC_INIT(&parser->temp.ip_packet[0], header->header.ipv4.ipv4, BTLV_IPV4_TOTAL_LENGTH_OFFSET);
    BATOM_VEC_INIT(&parser->temp.ip_packet[1], parser->temp.total_length, sizeof(parser->temp.total_length));
    BATOM_VEC_INIT(&parser->temp.ip_packet[2], header->header.ipv4.ipv4 + BTLV_IPV4_TOTAL_LENGTH_OFFSET, (BTLV_IPV4_HEADER_CHECKSUM_OFFSET - BTLV_IPV4_TOTAL_LENGTH_OFFSET)-BTLV_IPV4_TOTAL_LENGTH_SIZE);
    BATOM_VEC_INIT(&parser->temp.ip_packet[3], parser->temp.ip_checksum, sizeof(parser->temp.ip_checksum));
    BATOM_VEC_INIT(&parser->temp.ip_packet[4], header->header.ipv4.ipv4 + BTLV_IPV4_ADDRESS_OFFSET-(BTLV_IPV4_TOTAL_LENGTH_SIZE + BTLV_IPV4_HEADER_CHECKSUM_SIZE), BTLV_IPV4_HEADER_SIZE-BTLV_IPV4_ADDRESS_OFFSET);
    batom_cursor_from_vec(&cursor, parser->temp.ip_packet, 5);
    checksum = b_tlv_ip_checksum(&cursor);
    if(0) BDBG_LOG(("IP checksum %#x", checksum));
    B_MEDIA_SAVE_UINT16_BE(parser->temp.ip_checksum,checksum);
    /* UDP header */
    BATOM_VEC_INIT(&parser->temp.ip_packet[5], header->header.ipv4.udp, BTLV_UDP_LENGTH_OFFSET);
    BATOM_VEC_INIT(&parser->temp.ip_packet[6], parser->temp.payload_length, sizeof(parser->temp.payload_length));
    BATOM_VEC_INIT(&parser->temp.ip_packet[7], &parser->temp.udp_checksum, sizeof(parser->temp.udp_checksum));
    batom_cursor_extract(payload, &parser->temp.ip_packet[8], sizeof(parser->temp.ip_packet)/sizeof(parser->temp.ip_packet[0]) - 8, &payload_vecs);
    result->type = btlv_ip_parser_result_ipv4;
    batom_cursor_from_vec(&result->data, parser->temp.ip_packet, payload_vecs + 8);
    if(0) {batom_cursor_dump(&result->data, "IPV4 UDP");}

    return 0;
}

static int btlv_ip_parser_reconstruct_ipv6(btlv_ip_parser *parser, const btlv_compressed_ip_packet *header, batom_cursor *payload, btlv_ip_parser_result *result)
{
    unsigned payload_length = batom_cursor_size(payload);
    batom_cursor cursor;
    size_t payload_vecs;
    uint16_t checksum;

    if(0) {batom_cursor_dump(payload, "btlv_ip_parser_reconstruct_ipv6");}
    if(0) {batom_range_dump(header->header.ipv6.ipv6, sizeof(header->header.ipv6.ipv6), "header.ipv6.ipv6");}
    /* 1. Create UDP pseudo header
     * RFC 2460 8.1 Upper-Layer Checksums */
    payload_length += BTLV_UDP_HEADER_SIZE;
    B_MEDIA_SAVE_UINT32_BE(parser->temp.payload_length_32,payload_length);
    B_MEDIA_SAVE_UINT16_BE(parser->temp.payload_length,payload_length);

    /* UDP "pseudo-header" */
    BATOM_VEC_INIT(&parser->temp.ip_packet[0], header->header.ipv6.ipv6 + BTLV_IPV6_ADDRESS_OFFSET, BTLV_IPV6_ADDRESS_SIZE);
    BATOM_VEC_INIT(&parser->temp.ip_packet[1], parser->temp.payload_length_32, sizeof(parser->temp.payload_length_32));
    BATOM_VEC_INIT(&parser->temp.ip_packet[2], parser->temp.zero_24, sizeof(parser->temp.zero_24));
    BATOM_VEC_INIT(&parser->temp.ip_packet[3], header->header.ipv6.ipv6 + (BTLV_IPV6_NEXT_HEADER_OFFSET-BTLV_IPV6_PAYLOAD_LENGTH_SIZE), BTLV_IPV6_NEXT_HEADER_SIZE);
    /* UDP header */
    BATOM_VEC_INIT(&parser->temp.ip_packet[4], header->header.ipv6.udp, BTLV_UDP_LENGTH_OFFSET);
    BATOM_VEC_INIT(&parser->temp.ip_packet[5], parser->temp.payload_length, sizeof(parser->temp.payload_length));

    batom_cursor_extract(payload, &parser->temp.ip_packet[6], sizeof(parser->temp.ip_packet)/sizeof(parser->temp.ip_packet[0]) - 6, &payload_vecs);
#if 0
    batom_cursor_from_vec(&cursor, parser->temp.ip_packet, 4);
    if(1) { batom_cursor_dump(&cursor, "HEADER"); }
    checksum = b_tlv_ip_checksum(&cursor);
    if(1) BDBG_LOG(("HEADER %#x", checksum));

    batom_cursor_from_vec(&cursor, parser->temp.ip_packet+4, payload_vecs + 3);
    if(1) { batom_cursor_dump(&cursor, "DATA"); }
    checksum = b_tlv_ip_checksum(&cursor);
    if(1) BDBG_LOG(("DATA %#x", checksum));
#endif

    batom_cursor_from_vec(&cursor, parser->temp.ip_packet, payload_vecs + 6);
    if(0) { batom_cursor_dump(&cursor, "UDP pseudo-packet"); }
    checksum = b_tlv_ip_checksum(&cursor);
    if(0) BDBG_LOG(("checksum %#x", checksum));
    B_MEDIA_SAVE_UINT16_BE(parser->temp.udp_checksum,checksum);

    /* 2. Create IPV6 packet */
    /* IPV6 header */
    BATOM_VEC_INIT(&parser->temp.ip_packet[0], header->header.ipv6.ipv6, BTLV_IPV6_PAYLOAD_LENGTH_OFFSET);
    BATOM_VEC_INIT(&parser->temp.ip_packet[1], parser->temp.payload_length, sizeof(parser->temp.payload_length));
    BATOM_VEC_INIT(&parser->temp.ip_packet[2], header->header.ipv6.ipv6 + BTLV_IPV6_PAYLOAD_LENGTH_OFFSET , sizeof(header->header.ipv6.ipv6) - (BTLV_IPV6_PAYLOAD_LENGTH_OFFSET));
    /* UDP header */
    BATOM_VEC_INIT(&parser->temp.ip_packet[3], header->header.ipv6.udp, BTLV_UDP_LENGTH_OFFSET);
    BATOM_VEC_INIT(&parser->temp.ip_packet[4], parser->temp.payload_length, sizeof(parser->temp.payload_length));
    BATOM_VEC_INIT(&parser->temp.ip_packet[5], &parser->temp.udp_checksum, sizeof(parser->temp.udp_checksum));
    batom_cursor_extract(payload, &parser->temp.ip_packet[6], sizeof(parser->temp.ip_packet)/sizeof(parser->temp.ip_packet[0]) - 6, &payload_vecs);
    result->type = btlv_ip_parser_result_ipv6;
    batom_cursor_from_vec(&result->data, parser->temp.ip_packet, payload_vecs + 6);
    if(0) {batom_cursor_dump(&result->data, "IPV6 UDP");}

    return 0;
}

static int btlv_ip_parser_process_compressed_ip(btlv_ip_parser *parser, const btlv_compressed_ip_packet *packet, batom_cursor *cursor, btlv_ip_parser_result *result)
{
    btlv_compressed_ip_packet *header;
    int rc;
    BSTD_UNUSED(result);

    BDBG_ASSERT(packet->cid<sizeof(parser->cid)/sizeof(parser->cid[0]));
    header = parser->cid[packet->cid].header;
    if(packet->cid_header_type==BTLV_COMPRESSED_IPV4 || packet->cid_header_type==BTLV_COMPRESSED_IPV6) {
        if(header==NULL) {
            header = BKNI_Malloc(sizeof(*header));
            if(header==NULL) {
                return BDBG_TRACE_ERROR(BTLV_RESULT_OUT_OF_MEMORY);
            }
            parser->cid[packet->cid].header=header;
        }
        *header = *packet;
    } else {
        if(header==NULL) {
            BDBG_MSG(("process_compressed_ip:%p unknown cid:%u", (void*)parser, (unsigned)packet->cid));
            return BTLV_RESULT_UNKNOWN_CID;
        }
        if(packet->cid_header_type==BTLV_COMPRESSED_IPV4_RAW) {
            if(header->cid_header_type != BTLV_COMPRESSED_IPV4) {
                return BDBG_TRACE_ERROR(BTLV_RESULT_TLV_ERROR);
            }
        } else if(packet->cid_header_type==BTLV_COMPRESSED_IPV6_RAW) {
            if(header->cid_header_type != BTLV_COMPRESSED_IPV6) {
                return BDBG_TRACE_ERROR(BTLV_RESULT_TLV_ERROR);
            }
        }
    }

    if(header->cid_header_type==BTLV_COMPRESSED_IPV4) {
        rc = btlv_ip_parser_reconstruct_ipv4(parser, header, cursor, result);
        if(rc!=0) {
            return BDBG_TRACE_ERROR(rc);
        }
    } else if(header->cid_header_type==BTLV_COMPRESSED_IPV6) {
        rc = btlv_ip_parser_reconstruct_ipv6(parser, header, cursor, result);
        if(rc!=0) {
            return BDBG_TRACE_ERROR(rc);
        }
    }
    return 0;
}

int btlv_ip_parser_process(btlv_ip_parser *parser, batom_cursor *cursor, btlv_ip_parser_result *result)
{
    int data;
    int rc;
    btlv_compressed_ip_packet packet;

    /* Recommendation ITU-R BT.1869 TABLE 1 TLV container */
    BATOM_NEXT(data,cursor);
    BATOM_NEXT(data,cursor); /* packet_type */
    batom_cursor_skip(cursor, 2); /* length */
    if(BATOM_IS_EOF(cursor)) {
        return BDBG_TRACE_ERROR(BTLV_RESULT_PACKET_ERROR);
    }
    BATOM_CLONE(&result->data,cursor);
    BDBG_MSG_TRACE(("TLV packet: %u (%u)", (unsigned)data, (unsigned)batom_cursor_size(cursor)));
    switch(data) {
    case BTLV_PACKET_TYPE_IPV4:
        result->type = btlv_ip_parser_result_ipv4;
        if(0) {batom_cursor_dump(&result->data, "IPV4");}
        break;
    case BTLV_PACKET_TYPE_IPV6:
        result->type = btlv_ip_parser_result_ipv6;
        break;
    case BTLV_PACKET_TYPE_SIGNALING:
        result->type = btlv_ip_parser_result_signaling;
        break;
    case BTLV_PACKET_TYPE_NULL:
        result->type = btlv_ip_parser_result_null;;
        break;
    case BTLV_PACKET_TYPE_COMPRESSED_IP:
        rc = btlv_parser_parse_compressed_ip_packet(cursor, &packet);
        if(rc!=0) {
            return BDBG_TRACE_ERROR(rc);
        }
        rc = btlv_ip_parser_process_compressed_ip(parser, &packet, cursor, result);
        if(rc!=0) {
            if(rc==BTLV_RESULT_UNKNOWN_CID) {
                return 0;
            }
            return BDBG_TRACE_ERROR(rc);
        }
        break;
    default:
        result->type = btlv_ip_parser_result_invalid;;
        break;
    }
    return 0;
}

void btlv_ip_parser_shutdown(btlv_ip_parser *parser)
{
    btlv_ip_parser_reset(parser);
    return;
}

static bool btlv_ip_demux_ipv4(const btlv_ip_address *addr, batom_cursor *cursor)
{
    int word;

    batom_cursor_skip(cursor, BTLV_IPV4_PROTOCOL_OFFSET);

    BATOM_NEXT(word,cursor);
    if(word==BATOM_EOF) {
        return false;
    }
    if(word!=BTLV_IPPROTO_UDP) {
        return false;
    }
    batom_cursor_skip(cursor,BTLV_IPV4_HEADER_CHECKSUM_SIZE + sizeof(addr->address.ipv4.addr));
    if(batom_cursor_compare(cursor, addr->address.ipv4.addr,sizeof(addr->address.ipv4.addr))!=0) {
        return false;
    }
    batom_cursor_skip(cursor, BTLV_UDP_DESTINATION_PORT_OFFSET + sizeof(addr->address.ipv4.addr));
    word = batom_cursor_uint16_be(cursor);
    if(BATOM_IS_EOF(cursor)) {
        return false;
    }
    #if 0
    if(word!=addr->address.ipv4.port) {
        return false;
    }
    #endif
    batom_cursor_skip(cursor, BTLV_UDP_LENGTH_SIZE + BTLV_UDP_CHECKSUM_SIZE);
    if(BATOM_IS_EOF(cursor)) {
        return false;
    }
    return true;
}

static bool btlv_ip_demux_ipv6(const btlv_ip_address *addr, batom_cursor *cursor)
{
    int word;

    batom_cursor_skip(cursor, BTLV_IPV6_NEXT_HEADER_OFFSET);

    BATOM_NEXT(word,cursor);
    if(word==BATOM_EOF) {
        return false;
    }
    if(word!=BTLV_IPPROTO_UDP) {
        return false;
    }
    BATOM_NEXT(word,cursor); /* skip Hop Limit */
    batom_cursor_skip(cursor,sizeof(addr->address.ipv6.addr));
    if(batom_cursor_compare(cursor, addr->address.ipv6.addr,sizeof(addr->address.ipv6.addr))!=0) {
        return false;
    }
    batom_cursor_skip(cursor, sizeof(addr->address.ipv6.addr) + BTLV_UDP_DESTINATION_PORT_OFFSET);
    word = batom_cursor_uint16_be(cursor);
    if(BATOM_IS_EOF(cursor)) {
        return false;
    }
    #if 0
    if(word!=addr->address.ipv6.port) {
        return false;
    }
    #endif
    batom_cursor_skip(cursor, BTLV_UDP_LENGTH_SIZE + BTLV_UDP_CHECKSUM_SIZE);
    if(BATOM_IS_EOF(cursor)) {
        return false;
    }
    return true;
}

bool btlv_ip_demux(const btlv_ip_parser_result *ip_result, const btlv_ip_address *addr, batom_cursor *payload)
{
    BATOM_CLONE(payload, &ip_result->data);
    if(ip_result->type == btlv_ip_parser_result_ipv4 && addr->type == btlv_ip_address_ipv4) {
        return btlv_ip_demux_ipv4(addr, payload);
    } else if(ip_result->type == btlv_ip_parser_result_ipv6 && addr->type == btlv_ip_address_ipv6) {
        return btlv_ip_demux_ipv6(addr, payload);
    }
    return false;
}

int btlv_parse_am_table(batom_cursor *cursor, btlv_am_table *table)
{
   int i=0;
   table->table_id = batom_cursor_byte(cursor);
   if (table->table_id != 0xfe)
   {
      return -1;
   }

   table->section_length = batom_cursor_uint16_be(cursor);
   if (table->section_length && 0x8000)
   {
      table->section_syntax_indicator = true;
   }
   else
   {
      table->section_syntax_indicator = false;
   }
   table->section_length &= 0x0fff;
   table->table_id_extension = batom_cursor_uint16_be(cursor);
   table->version_number = batom_cursor_byte(cursor) & 0x3f;
   if (table->version_number & 0x1)
   {
      table->current_next_indicator = true;
   }
   else
   {
      table->current_next_indicator = false;
   }
   table->version_number = table->version_number >> 1;
   table->section_number = batom_cursor_byte(cursor);
   table->last_section_number = batom_cursor_byte(cursor);
   table->num_of_service_id = batom_cursor_uint16_be(cursor);
   table->num_of_service_id = table->num_of_service_id & 0xffc0;
   table->num_of_service_id = table->num_of_service_id >> 6;
   printf("\n table->num_of_service_id %x",table->num_of_service_id);

   for (i=0;i<table->num_of_service_id;i++)
   {
       uint16_t word;
       table->services[i].service_id = batom_cursor_uint16_be(cursor);
       table->services[i].service_loop_length = word = batom_cursor_uint16_be(cursor);
       table->services[i].service_loop_length = table->services[i].service_loop_length & 0x03ff;
       if (word & 0x8000)
       {
           table->services[i].is_ipv6 = true;
           batom_cursor_copy(cursor, table->services[i].addr.ipv6.src_addr,sizeof(table->services[i].addr.ipv6.src_addr));
           table->services[i].service_loop_length  -= sizeof(table->services[i].addr.ipv6.src_addr);
           table->services[i].addr.ipv6.src_mask = batom_cursor_byte(cursor);
           table->services[i].service_loop_length -= 1;
           batom_cursor_copy(cursor, table->services[i].addr.ipv6.dst_addr,sizeof(table->services[i].addr.ipv6.dst_addr));
           table->services[i].service_loop_length -= sizeof(table->services[i].addr.ipv6.dst_addr);
           table->services[i].addr.ipv6.dst_mask = batom_cursor_byte(cursor);
           table->services[i].service_loop_length -= 1;
       }
       else
       {
           table->services[i].is_ipv6 = false;
           batom_cursor_copy(cursor, table->services[i].addr.ipv4.src_addr,sizeof(table->services[i].addr.ipv4.src_addr));
           table->services[i].service_loop_length  -= sizeof(table->services[i].addr.ipv4.src_addr);
           table->services[i].addr.ipv4.src_mask = batom_cursor_byte(cursor);
           table->services[i].service_loop_length -= 1;
           batom_cursor_copy(cursor, table->services[i].addr.ipv4.dst_addr,sizeof(table->services[i].addr.ipv4.dst_addr));
           table->services[i].service_loop_length -= sizeof(table->services[i].addr.ipv6.dst_addr);
           table->services[i].addr.ipv4.dst_mask = batom_cursor_byte(cursor);
           table->services[i].service_loop_length -= 1;
       }
       batom_cursor_skip(cursor,table->services[i].service_loop_length);
   }
   table->crc_32 = batom_cursor_uint32_be(cursor);
   return 0;
}


int btlv_print_am_table(btlv_am_table *table)
{
   int i=0;

   if (table->table_id != 0xfe)
   {
      return -1;
   }
   printf("\n****** AM TABLE ************\n");
   printf("\n num_of_service_id %u",table->num_of_service_id);
   for (i=0;i<table->num_of_service_id;i++)
   {
       printf("\n\t service_id %u", table->services[i].service_id);
       printf("\n\t service_loop_length %u",table->services[i].service_loop_length);
       printf("\n\t is_ipv6 %s",table->services[i].is_ipv6?"true":"false");

       if (table->services[i].is_ipv6)
       {
           printf("\n\t\t src ipv6 %08x.%08x.%08x.%08x",
                       table->services[i].addr.ipv6.src_addr[0] << 24 |
                       table->services[i].addr.ipv6.src_addr[1] << 16 |
                       table->services[i].addr.ipv6.src_addr[2] << 8 |
                       table->services[i].addr.ipv6.src_addr[3],
                       table->services[i].addr.ipv6.src_addr[4] << 24 |
                       table->services[i].addr.ipv6.src_addr[5] << 16 |
                       table->services[i].addr.ipv6.src_addr[6] << 8 |
                       table->services[i].addr.ipv6.src_addr[7],
                       table->services[i].addr.ipv6.src_addr[8] << 24 |
                       table->services[i].addr.ipv6.src_addr[9] << 16 |
                       table->services[i].addr.ipv6.src_addr[10] << 8 |
                       table->services[i].addr.ipv6.src_addr[11],
                       table->services[i].addr.ipv6.src_addr[12] << 24 |
                       table->services[i].addr.ipv6.src_addr[13] << 16 |
                       table->services[i].addr.ipv6.src_addr[14] << 8 |
                       table->services[i].addr.ipv6.src_addr[15]);
           printf("\n\t src ipv6 mask  %02x",table->services[i].addr.ipv6.src_mask);
           printf("\n\t\t dst ipv6 %08x.%08x.%08x.%08x",
                       table->services[i].addr.ipv6.dst_addr[0] << 24 |
                       table->services[i].addr.ipv6.dst_addr[1] << 16 |
                       table->services[i].addr.ipv6.dst_addr[2] << 8 |
                       table->services[i].addr.ipv6.dst_addr[3],
                       table->services[i].addr.ipv6.dst_addr[4] << 24 |
                       table->services[i].addr.ipv6.dst_addr[5] << 16 |
                       table->services[i].addr.ipv6.dst_addr[6] << 8 |
                       table->services[i].addr.ipv6.dst_addr[7],
                       table->services[i].addr.ipv6.dst_addr[8] << 24 |
                       table->services[i].addr.ipv6.dst_addr[9] << 16 |
                       table->services[i].addr.ipv6.dst_addr[10] << 8 |
                       table->services[i].addr.ipv6.dst_addr[11],
                       table->services[i].addr.ipv6.dst_addr[12] << 24 |
                       table->services[i].addr.ipv6.dst_addr[13] << 16 |
                       table->services[i].addr.ipv6.dst_addr[14] << 8 |
                       table->services[i].addr.ipv6.dst_addr[15]);
           printf("\n\t src ipv6 mask  %02x",table->services[i].addr.ipv6.dst_mask);

       }
       else
       {
           printf("\n\t src ipv4 %02x.%02x.%02x.%02x",
                           table->services[i].addr.ipv4.src_addr[3],
                           table->services[i].addr.ipv4.src_addr[2],
                           table->services[i].addr.ipv4.src_addr[1],
                           table->services[i].addr.ipv4.src_addr[0]);

           printf("\n\t src ipv4 mask  %02x",table->services[i].addr.ipv4.src_mask);
           printf("\n\t dst ipv4 %02x.%02x.%02x.%02x",
                           table->services[i].addr.ipv4.dst_addr[3],
                           table->services[i].addr.ipv4.dst_addr[2],
                           table->services[i].addr.ipv4.dst_addr[1],
                           table->services[i].addr.ipv4.dst_addr[0]);
           printf("\n\t src ipv4 mask  %02x",table->services[i].addr.ipv4.dst_mask);

       }

   }
   printf("\n****** AM TABLE ************\n");
   return 0;
}
