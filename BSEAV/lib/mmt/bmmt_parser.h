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

 **************************************************************************/
#ifndef __BMMT_PARSER_H__
#define __BMMT_PARSER_H__ 1

#include "btlv_parser.h"
#include "bmmt_utils.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct btlv_mmt_packet_header {
    uint8_t type;
    uint16_t packet_id;
    uint32_t timestamp;
    uint32_t packet_seq_num;
    uint32_t packet_counter;
    bool packet_counter_valid;
} btlv_mmt_packet_header;

#define BTLV_MMT_TYPE_MPU                   0
#define BTLV_MMT_TYPE_GENERIC_OBJECT        1
#define BTLV_MMT_TYPE_SIGNALLING_MESSAGE    2
#define BTLV_MMT_TYPE_REPAIR_SYMBOL         3

typedef struct btlv_mpu_header {
    uint16_t length;
    uint8_t type;
    bool timed;
    bool aggregation;
    uint8_t f_i;
    uint8_t frag_counter;
    uint32_t sequence_number;
} btlv_mpu_header;

#define BTLV_MPU_TYPE_MPU_METADATA          0
#define BTLV_MPU_TYPE_MOVIE_FRAGMENT_DATA   1
#define BTLV_MPU_TYPE_MFU                   2

typedef struct btlv_timed_mfu_data {
    uint32_t movie_fragment_sequence_number;
    uint32_t sample_number;
    uint32_t offset;
    uint8_t priority;
    uint8_t dependency_counter;
    unsigned length;
    batom_cursor payload;
} btlv_timed_mfu_data;

int btlv_parse_mmt_header(batom_cursor *cursor,btlv_mmt_packet_header *header);
int btlv_parse_mpu_payload_header(batom_cursor *cursor, btlv_mpu_header *header);
int btlv_parse_mpu_payload_mfu(batom_cursor *cursor, const btlv_mpu_header *header, btlv_timed_mfu_data *mfu, unsigned max_mfu, unsigned *parsed_mfu);

typedef struct btlv_signalling_header {
    uint8_t f_i;
    bool h;
    bool aggregation_flag;
    uint8_t frag_count;
} btlv_signalling_header;

int btlv_parse_signalling_header(batom_cursor *cursor, btlv_signalling_header *header);

typedef struct btlv_signalling_message_data {
    unsigned length;
    batom_cursor message;
} btlv_signalling_message_data;

int btlv_parse_signalling_message(batom_cursor *cursor, const btlv_signalling_header *header, btlv_signalling_message_data *data, unsigned max_messages, unsigned *parsed_messages);

typedef struct btlv_signalling_message_header {
    uint16_t message_id;
    uint8_t version;
    uint32_t length;
} btlv_signalling_message_header;

#define BTLV_IS_PA_MESSAGE(id) ((id)==0x0000)
#define BTLV_IS_MPI_MESSAGE(id) ((id)>=0x0001 && (id)<=0x000F)

int btlv_parse_signalling_message_header(batom_cursor *cursor, btlv_signalling_message_header *header);

#define BTLV_COMPLETE_MP_TABLE  0x20

typedef struct btlv_package_access_message {
    uint8_t table_id;
    uint8_t table_version;
    uint16_t table_length;
    batom_cursor payload;
} btlv_package_access_message;

int btlv_parse_package_access_message(batom_cursor *cursor, btlv_package_access_message *message, unsigned max_messages, unsigned *parsed_messages);

typedef struct btlv_mp_table_header {
    unsigned MP_table_mode;
    unsigned MMT_package_id_length;
    unsigned MP_table_descriptors_length;
    unsigned number_of_assets;
    uint8_t MMT_package_id[256+1];
    uint8_t MP_table_descriptors[256+1];
} btlv_mp_table_header;

int btlv_parse_mp_table_header(batom_cursor *cursor, btlv_mp_table_header *table);

typedef struct btlv_variable_data {
    unsigned length;
    void *data;
} btlv_variable_data;

int btlv_parse_variable_data(batom_cursor *cursor, btlv_variable_data *data);
void btlv_variable_data_init(btlv_variable_data *data);
void btlv_variable_data_shutdown(btlv_variable_data *data);

typedef struct btlv_mmt_asset_id {
    uint32_t asset_id_scheme;
    uint8_t asset_id_length;
    uint8_t asset_id_byte[256+1];
} btlv_mmt_asset_id;

int btlv_parse_mmt_asset_id(batom_cursor *cursor, btlv_mmt_asset_id *data);

typedef struct btlv_mmt_identifier_mapping {
    enum {
        btlv_mmt_identifier_type_asset_id = 0x00,
        btlv_mmt_identifier_type_url = 0x01,
        btlv_mmt_identifier_type_regex = 0x02,
        btlv_mmt_identifier_type_representation_id = 0x03
    } identifier_type;
    union {
        btlv_mmt_asset_id asset_id;
        struct {
            unsigned URL_count;
            btlv_variable_data urls[16];
        } url;
        btlv_variable_data regex;
        btlv_variable_data representation_id;
        btlv_variable_data _private;
    } data;
} btlv_mmt_identifier_mapping;

void btlv_mmt_identifier_mapping_init(btlv_mmt_identifier_mapping *data);
void btlv_mmt_identifier_mapping_shutdown(btlv_mmt_identifier_mapping *data);
int btlv_parse_mmt_identifier_mapping(batom_cursor *cursor, btlv_mmt_identifier_mapping *data);

typedef struct btlv_mmt_general_location_info {
    enum {
        btlv_mmt_general_location_type_id = 0x00,
        btlv_mmt_general_location_type_mmt_ipv4 = 0x01,
        btlv_mmt_general_location_type_mmt_ipv6 = 0x02,
        btlv_mmt_general_location_type_mpeg2ts = 0x03,
        btlv_mmt_general_location_type_mpeg2ts_ipv6 = 0x04,
        btlv_mmt_general_location_type_url = 0x05,
        btlv_mmt_general_location_type_private = 0x06
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
        btlv_variable_data _private;
    } data;
} btlv_mmt_general_location_info;

void btlv_mmt_general_location_info_init(btlv_mmt_general_location_info *data);
void btlv_mmt_general_location_info_shutdown(btlv_mmt_general_location_info *data);
int btlv_parse_mmt_general_location_info(batom_cursor *cursor, btlv_mmt_general_location_info *data);

typedef struct btlv_mmt_package_table_asset {
    btlv_mmt_identifier_mapping identifier_mapping;
    uint32_t asset_type;
    bool asset_clock_relation_flag;
    struct {
        uint8_t asset_clock_relation_id;
        bool asset_timescale_flag;
        uint32_t asset_timescale;
    } asset_clock_relation;
    struct {
        uint8_t location_count;
        btlv_mmt_general_location_info *locations;
        struct {
            btlv_mmt_general_location_info first_location;
        } data;
    } asset_location;
    struct {
        unsigned asset_descriptors_length;
        batom_cursor asset_descriptors_bytes;
    } asset_descriptors;
} btlv_mmt_package_table_asset;

void btlv_mmt_package_table_asset_init(btlv_mmt_package_table_asset *data);
void btlv_mmt_package_table_asset_shutdown(btlv_mmt_package_table_asset *data);
int btlv_parse_mmt_package_table_asset(batom_cursor *cursor, btlv_mmt_package_table_asset *asset);

typedef struct btlv_mpu_timestamp_descriptor_header {
    unsigned N;
} btlv_mpu_timestamp_descriptor_header;
int btlv_parse_mpu_timestamp_descriptor_header(batom_cursor *cursor, btlv_mpu_timestamp_descriptor_header *data);


typedef struct btlv_mpu_timestamp_descriptor_entry {
    uint32_t mpu_sequence_number;
    btlv_ntp_time mpu_presentation_time;
} btlv_mpu_timestamp_descriptor_entry;

int btlv_parse_mpu_timestamp_descriptor_entry(batom_cursor *cursor, btlv_mpu_timestamp_descriptor_entry *data);

typedef struct btlv_mpu_extended_timestamp_descriptor_header {
    unsigned pts_offset_type;
    bool timescale_flag;
    uint32_t timescale;
    uint16_t default_pts_offset;
} btlv_mpu_extended_timestamp_descriptor_header;

typedef struct btlv_mpu_extended_timestamp_descriptor_entry {
    uint32_t mpu_sequence_number;
    uint8_t mpu_presentation_time_leap_indicator;
    uint16_t mpu_decoding_time_offset;
    uint8_t num_of_au;
    struct {
        uint16_t dts_pts_offset;
        uint16_t pts_offset;
    } au[128];
} btlv_mpu_extended_timestamp_descriptor_entry;

int btlv_parse_mpu_extended_timestamp_descriptor(batom_cursor *cursor, btlv_mpu_extended_timestamp_descriptor_header *header, unsigned max_entries, btlv_mpu_extended_timestamp_descriptor_entry *entry, unsigned *parsed_entries);


#ifdef __cplusplus
}
#endif


#endif /* __BMMT_PARSER_H__ */
