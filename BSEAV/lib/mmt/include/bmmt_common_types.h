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
#ifndef __BMMT_COMMON_TYPES_H__
#define __BMMT_COMMON_TYPES_H__ 1

#include "bstd.h"

#ifdef __cplusplus
extern "C"
{
#endif
typedef enum bmmt_stream_type {
    bmmt_stream_type_unknown,
    bmmt_stream_type_h265,
    bmmt_stream_type_aac,
    bmmt_stream_type_subtitle
} bmmt_stream_type;


typedef struct btlv_ipv4_address {
    uint8_t addr[4];
    uint16_t port;
} btlv_ipv4_address;

typedef struct btlv_ipv6_address {
    uint8_t addr[16];
    uint16_t port;
} btlv_ipv6_address;


/* Recommendation ITU-R BT.1869 TABLE 2 Packet type assignment values */
#define BTLV_PACKET_TYPE_IPV4               0x01
#define BTLV_PACKET_TYPE_IPV6               0x02
#define BTLV_PACKET_TYPE_COMPRESSED_IP      0x03
#define BTLV_PACKET_TYPE_SIGNALING          0xFE
#define BTLV_PACKET_TYPE_NULL               0xFF

#define BMMT_MAX_DMA_BLOCKS 100
#define BMMT_MAX_AES_CTR_KEY_SIZE 16
#define BMMT_MAX_IV_SIZE 16

typedef struct btlv_ip_address {
    enum {
        btlv_ip_address_ipv4 = BTLV_PACKET_TYPE_IPV4,
        btlv_ip_address_ipv6 = BTLV_PACKET_TYPE_IPV6,
        btlv_ip_address_invalid
    } type;
    union {
        btlv_ipv4_address ipv4;
        btlv_ipv6_address ipv6;
    } address;
} btlv_ip_address;

typedef struct bmmt_variable_data {
    unsigned length;
    void *data;
} bmmt_variable_data;

typedef struct bmmt_general_location_info {
    enum {
        bmmt_general_location_type_id = 0x00,
        bmmt_general_location_type_ipv4 = 0x01,
        bmmt_general_location_type_ipv6 = 0x02,
        bmmt_general_location_type_mpeg2ts = 0x03,
        bmmt_general_location_type_mpeg2ts_ipv6 = 0x04,
        bmmt_general_location_type_url = 0x05,
        bmmt_general_location_type_private = 0x06
    } location_type;
    union {
        uint16_t packet_id;
        struct {
            uint8_t ipv4_src_addr[4];
            uint8_t ipv4_dst_addr[4];
            uint16_t dst_port;
            uint16_t packet_id;
        } mmt_ipv4;
        struct {
            uint8_t ipv6_src_addr[16];
            uint8_t ipv6_dst_addr[16];
            uint16_t dst_port;
            uint16_t packet_id;
        } mmt_ipv6;
        struct {
            uint16_t network_id;
            uint16_t MPEG_2_transport_stream_id;
            uint16_t MPEG_2_PID;
        } mpeg2ts;
        struct {
            uint8_t ipv6_src_addr[16];
            uint8_t ipv6_dst_addr[16];
            uint16_t dst_port;
            uint16_t MPEG_2_PID;
        } mpeg2ts_ipv6;
        struct {
            unsigned URL_length;
            uint8_t URL_byte[256+1];
        } url;
        bmmt_variable_data _private;
    } data;
} bmmt_general_location_info;


typedef struct bmmt_pl_table_packages {
    uint8_t package_id_length;
    uint8_t package_id[256+1];
    bmmt_general_location_info location_info;
}bmmt_pl_table_packages;

typedef struct bmmt_pl_table_ip_flow {
    uint32_t transport_file_id;
    uint8_t location_type;
    union {
        struct {
            uint8_t src_addr[4];
            uint8_t dst_addr[4];
            uint16_t dst_port;
            } ipv4;
        struct {
            uint8_t src_addr[16];
            uint8_t dst_addr[16];
            uint16_t dst_port;
            } ipv6;
        struct {
            unsigned URL_length;
            uint8_t URL_byte[256+1];
        } url;
    } location_info;
    uint16_t descriptor_loop_length;
    uint8_t descriptor[256+1];
} bmmt_pl_table_ip_flow;

typedef struct bmmt_pl_table {
    uint8_t num_of_packages;
    bmmt_pl_table_packages packages[256];
    uint8_t num_of_ip_delivery;
    bmmt_pl_table_ip_flow ip_flow[256];
}bmmt_pl_table;

typedef struct bmmt_mp_table_asset {
    uint8_t identifier_type;
    uint32_t id_scheme;
    uint8_t id_length;
    uint8_t id[256+1];
    uint8_t type[4+1];
    bool  clock_relation_flag;
    uint8_t location_count;
    bmmt_general_location_info location_info[5];
    uint16_t descriptor_length;
    uint8_t descriptor[1024+1];
}bmmt_mp_table_asset;

typedef struct bmmt_mp_table {
    uint8_t mpt_mode;
    uint8_t package_id_length;
    uint8_t package_id[256+1];
    uint16_t descriptor_length;
    uint8_t descriptor[1024+1];
    uint8_t num_of_assets;
    bmmt_mp_table_asset assets[8];
}bmmt_mp_table;

typedef struct btlv_am_table_services {
   uint16_t service_id;
   bool is_ipv6;
   uint16_t service_loop_length;
   union {
      struct {
            uint8_t src_addr[4];
            uint8_t dst_addr[4];
            uint8_t src_mask;
            uint8_t dst_mask;
        } ipv4;
        struct {
            uint8_t src_addr[16];
            uint8_t dst_addr[16];
            uint8_t src_mask;
            uint8_t dst_mask;
        } ipv6;
    } addr;
} btlv_am_table_services;

typedef struct btlv_am_table{
   uint8_t table_id;
   bool section_syntax_indicator;
   uint16_t section_length;
   uint16_t table_id_extension;
   uint8_t version_number;
   bool current_next_indicator;
   uint8_t section_number;
   uint8_t last_section_number;
   uint16_t num_of_service_id;
   btlv_am_table_services services[16];
   uint16_t crc_32;
} btlv_am_table;

#ifdef __cplusplus
}
#endif


#endif /* __BMMT_COMMON_TYPES_H__ */
