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
#ifndef __BTLV_PARSER_H__
#define __BTLV_PARSER_H__ 1

#include "bioatom.h"
#include "bmmt_utils.h"
#include "bmmt_common_types.h"
#include <stdio.h>
#include <stdlib.h>
#ifdef __cplusplus
extern "C"
{
#endif
#define BMPEG2TS_PKT_LEN    (188)
#define BTLV_DEFAULT_PID    (0x2D)

typedef struct btlv_segment {
    uint8_t offset;
    uint8_t size; /* 0 if invalid */
} btlv_segment;

typedef struct btlv_segments {
    struct btlv_segment head; /* part of the packet at head of MPEG-2 TS */
    struct btlv_segment tail; /* part of the packet at end of MPEG-2 TS */
    uint8_t count; /* number of full packets */
    btlv_segment packets[40]; /* full packets */
} btlv_segments;

#define BTLV_RESULT_TRANSPORT_ERROR (-1)
#define BTLV_RESULT_UNKNOWN_PID     (-2)
#define BTLV_RESULT_TS_SYNC_ERROR   (-3)
#define BTLV_RESULT_FORMAT_ERROR    (-4)
#define BTLV_RESULT_UNKNOWN         (-5)
#define BTLV_RESULT_PACKET_ERROR    (-6)
#define BTLV_RESULT_TLV_ERROR       (-7)
#define BTLV_RESULT_UNKNOWN_TYPE    (-8)
#define BTLV_RESULT_OUT_OF_MEMORY   (-9)
#define BTLV_RESULT_UNKNOWN_CID     (-10)
#define BTLV_RESULT_END_OF_DATA     (-11)
#define BTLV_RESULT_TOO_MUCH_DATA   (-12)
#define BTLV_RESULT_INVALID_TAG     (-13)
#define BTLV_RESULT_NOT_FOUND       (-14)

typedef struct btlv_parser {
    uint16_t pid;
    bool in_sync;
    unsigned fragments;
    unsigned packet_size;
    unsigned expected_packet_size;
    batom_vec tail_pkt;
    uint16_t tail_size;
    uint8_t eps_higher_byte;
    btlv_segments segments;
    batom_vec packet[360]; /* largest number of fragments 65536 / 184 */
} btlv_parser;


typedef struct btlv_parser_packets {
    bool keep_previous_packets;
    bool keep_current_packet;
    bool packet_valid;
    batom_cursor packet;
    unsigned count;
    batom_vec packets[40];
} btlv_parser_packets;

int btlv_parser_split_mpeg2ts(uint16_t pid, const void *payload, btlv_segments *segments);

void btlv_parser_reset(btlv_parser *parser);
void btlv_parser_init(btlv_parser *parser, uint16_t pid);
void btlv_parser_shutdown(btlv_parser *parser);
int btlv_parser_process_mpeg2ts(btlv_parser *parser, const void *payload, btlv_parser_packets *packets);
int btlv_parser_process_tlv(btlv_parser *parser, const void *payload, unsigned payload_length, btlv_parser_packets *packets);

#define  BTLV_IPPROTO_UDP             17              /* user datagram protocol */

/* RFC: 791 3.1. Internet Header Format */
#define BTLV_IPV4_HEADER_SIZE               20
#define BTLV_IPV4_ADDRESS_OFFSET            12
#define BTLV_IPV4_ADDRESS_SIZE             (4 + 4)
#define BTLV_IPV4_TOTAL_LENGTH_OFFSET       2
#define BTLV_IPV4_TOTAL_LENGTH_SIZE         2
#define BTLV_IPV4_PROTOCOL_OFFSET           9
#define BTLV_IPV4_PROTOCOL_SIZE             1
#define BTLV_IPV4_HEADER_CHECKSUM_OFFSET    10
#define BTLV_IPV4_HEADER_CHECKSUM_SIZE      2
#define BTLV_IPV4_PROTOCOL_OFFSET           9

/* RFC 768  User Datagram Header Format */
#define BTLV_UDP_HEADER_SIZE        8
#define BTLV_UDP_LENGTH_OFFSET      4
#define BTLV_UDP_LENGTH_SIZE        2
#define BTLV_UDP_CHECKSUM_OFFSET    6
#define BTLV_UDP_CHECKSUM_SIZE      2
#define BTLV_UDP_DESTINATION_PORT_OFFSET    2

/* RFC 2460 3. IPv6 Header Format */
#define BTLV_IPV6_HEADER_SIZE               40
#define BTLV_IPV6_PAYLOAD_LENGTH_OFFSET     4
#define BTLV_IPV6_PAYLOAD_LENGTH_SIZE       2
#define BTLV_IPV6_ADDRESS_OFFSET           (BTLV_IPV6_PAYLOAD_LENGTH_OFFSET + 1 /* Next Header */ +  1 /* Hop Limit */)
#define BTLV_IPV6_ADDRESS_SIZE             (16 + 16)
#define BTLV_IPV6_NEXT_HEADER_OFFSET        6
#define BTLV_IPV6_NEXT_HEADER_SIZE          1



/* Recommendation ITU-R BT.1869 TABLE 3 Header compressed IP packet */
#define BTLV_COMPRESSED_IPV4                0x20
#define BTLV_COMPRESSED_IPV4_RAW            0x21
#define BTLV_COMPRESSED_IPV6                0x60
#define BTLV_COMPRESSED_IPV6_RAW            0x61

typedef struct btlv_ip_parser_result {
    enum {
        btlv_ip_parser_result_ipv4 = BTLV_PACKET_TYPE_IPV4,
        btlv_ip_parser_result_ipv6 = BTLV_PACKET_TYPE_IPV6,
        btlv_ip_parser_result_signaling = BTLV_PACKET_TYPE_SIGNALING,
        btlv_ip_parser_result_null = BTLV_PACKET_TYPE_NULL,
        btlv_ip_parser_result_invalid
    } type;
    batom_cursor data;
} btlv_ip_parser_result;

typedef struct btlv_compressed_ip_packet {
    uint16_t cid;
    uint8_t sn;
    uint8_t cid_header_type;
    union {
        struct {
            uint8_t ipv4[BTLV_IPV4_HEADER_SIZE-BTLV_IPV4_TOTAL_LENGTH_SIZE-BTLV_IPV4_HEADER_CHECKSUM_SIZE];
            uint8_t udp[BTLV_UDP_HEADER_SIZE-BTLV_UDP_LENGTH_SIZE-BTLV_UDP_CHECKSUM_SIZE];
        } ipv4;
        struct {
            uint8_t ipv6[BTLV_IPV6_HEADER_SIZE-BTLV_IPV6_PAYLOAD_LENGTH_SIZE];
            uint8_t udp[BTLV_UDP_HEADER_SIZE-BTLV_UDP_LENGTH_SIZE-BTLV_UDP_CHECKSUM_SIZE];
        } ipv6;
    } header;
} btlv_compressed_ip_packet;

typedef struct btlv_ip_parser {
    struct {
        uint8_t total_length[2];
        uint8_t payload_length[2];
        uint8_t payload_length_32[4];
        uint8_t zero_24[3];
        uint8_t udp_checksum[2];
        uint8_t ip_checksum[2];
        batom_vec ip_packet[360+8]; /* largest number of fragments 65536 / 184, plus IP  header fragments */
    } temp;
    struct {
        btlv_compressed_ip_packet *header;
    } cid[4096];
} btlv_ip_parser;

void btlv_ip_parser_reset(btlv_ip_parser *parser);
void btlv_ip_parser_init(btlv_ip_parser *parser);
void btlv_ip_parser_shutdown(btlv_ip_parser *parser);
int btlv_ip_parser_process(btlv_ip_parser *parser, batom_cursor *cursor, btlv_ip_parser_result *result);


bool btlv_ip_demux(const btlv_ip_parser_result *ip_result, const btlv_ip_address *addr, batom_cursor *payload);



int btlv_parse_am_table(batom_cursor *cursor, btlv_am_table *table);
int btlv_print_am_table(btlv_am_table *table);
#ifdef __cplusplus
}
#endif


#endif /* __BTLV_PARSER_H__ */
