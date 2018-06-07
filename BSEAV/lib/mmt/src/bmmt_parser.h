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
#ifndef __BMMT_PARSER_H__
#define __BMMT_PARSER_H__ 1

#include "btlv_parser.h"
#include "bmmt_utils.h"
#include "bmmt_common_types.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define BMMT_TABLE_SUBSET_MP00     0x11
#define BMMT_TABLE_SUBSET_MP01     0x12
#define MMT_TABLE_SUBSET_MP02      0x13
#define BMMT_TABLE_SUBSET_MP03     0x14
#define BMMT_TABLE_SUBSET_MP04     0x15
#define BMMT_TABLE_SUBSET_MP05     0x16
#define BMMT_TABLE_SUBSET_MP06     0x17
#define BMMT_TABLE_SUBSET_MP07     0x18
#define BMMT_TABLE_SUBSET_MP08     0x19
#define BMMT_TABLE_SUBSET_MP09     0x1A
#define BMMT_TABLE_SUBSET_MP10     0x1B
#define BMMT_TABLE_SUBSET_MP11     0x1C
#define BMMT_TABLE_SUBSET_MP12     0x1D
#define BMMT_TABLE_SUBSET_MP13     0x1E
#define BMMT_TABLE_SUBSET_MP14     0x1F
#define BMMT_TABLE_COMPLETE_MP     0x20
#define BMMT_TABLE_PLT             0x80
#define BMMT_TABLE_LCT             0x81
#define BMMT_TABLE_ECM_0           0x82
#define BMMT_TABLE_ECM_1           0x83
#define BMMT_TABLE_EMM_0           0x84
#define BMMT_TABLE_EMM_1           0x85
#define BMMT_TABLE_CAT             0x86
#define BMMT_TABLE_DCM_0           0x87
#define BMMT_TABLE_DCM_1           0x88
#define BMMT_TABLE_DMM_0           0x89
#define BMMT_TABLE_DMM_1           0x8A
#define BMMT_TABLE_MH_EIT_CURRENT  0x8B
#define BMMT_TABLE_MH_EIT_00       0x8C
#define BMMT_TABLE_MH_EIT_01       0x8D
#define BMMT_TABLE_MH_EIT_02       0x8E
#define BMMT_TABLE_MH_EIT_03       0x8F
#define BMMT_TABLE_MH_EIT_04       0x90
#define BMMT_TABLE_MH_EIT_05       0x91
#define BMMT_TABLE_MH_EIT_06       0x92
#define BMMT_TABLE_MH_EIT_07       0x93
#define BMMT_TABLE_MH_EIT_08       0x94
#define BMMT_TABLE_MH_EIT_09       0x95
#define BMMT_TABLE_MH_EIT_10       0x96
#define BMMT_TABLE_MH_EIT_11       0x97
#define BMMT_TABLE_MH_EIT_12       0x98
#define BMMT_TABLE_MH_EIT_13       0x99
#define BMMT_TABLE_MH_EIT_14       0x9A
#define BMMT_TABLE_MH_EIT_15       0x9B
#define BMMT_TABLE_MH_AIT          0x9C
#define BMMT_TABLE_BIT             0x9D
#define BMMT_TABLE_SDTT            0x9E
#define BMMT_TABLE_SDT_OWN         0x9F
#define BMMT_TABLE_SDT_OTHER       0xA0
#define BMMT_TABLE_TOT             0xA1
#define BMMT_TABLE_CDT             0xA2
#define BMMT_TABLE_DDM             0xA3
#define BMMT_TABLE_DAM             0xA4
#define BMMT_TABLE_DCC             0xA5
#define BMMT_TABLE_EMT             0xA6

#define BMMT_MESSAGE_ID_PA          0x0000
#define BMMT_MESSAGE_ID_MPEG2       0x8000
#define BMMT_MESSAGE_ID_CA          0x8001
#define BMMT_MESSAGE_ID_MPEG2_SHORT 0x8002
#define BMMT_MESSAGE_ID_DATA        0x8003

#define BMMT_IS_PA_MESSAGE(id) ((id)==BMMT_MESSAGE_ID_PA)
#define BMMT_IS_MPI_MESSAGE(id) ((id)>=0x0001 && (id)<=0x000F)
#define BMMT_IS_MPT_MESSAGE(id) ((id)>=0x0010 && (id)<=0x001F)


#define BMMT_TYPE_MPU                   0
#define BMMT_TYPE_GENERIC_OBJECT        1
#define BMMT_TYPE_SIGNALLING_MESSAGE    2
#define BMMT_TYPE_REPAIR_SYMBOL         3

#define BMMT_MPU_TYPE_MPU_METADATA          0
#define BMMT_MPU_TYPE_MOVIE_FRAGMENT_DATA   1
#define BMMT_MPU_TYPE_MFU                   2

typedef struct bmmt_packet_header {
    uint8_t type;
    uint16_t packet_id;
    uint32_t timestamp;
    uint32_t packet_seq_num;
    uint32_t packet_counter;
    bool packet_counter_valid;
    bool rap_flag;
    uint16_t hdr_ext_type;
    bool hdr_ext_end_flag;
    uint8_t scrambling_control;
    bool scrambling_method_present;
    bool scrambling_auth_present;
    bool scrambling_iv_present;
    uint8_t iv[BMMT_MAX_IV_SIZE];
} bmmt_packet_header;

typedef struct bmmt_mpu_header {
    uint16_t length;
    uint8_t type;
    bool timed;
    bool aggregation;
    uint8_t f_i;
    uint8_t frag_counter;
    uint32_t sequence_number;
} bmmt_mpu_header;

typedef struct bmmt_timed_mfu_data {
    uint32_t movie_fragment_sequence_number;
    uint32_t sample_number;
    uint32_t offset;
    uint8_t priority;
    uint8_t dependency_counter;
    unsigned length;
    batom_cursor payload;
} bmmt_timed_mfu_data;

int bmmt_parse_mmt_header(batom_cursor *cursor,bmmt_packet_header *header);
int bmmt_parse_mpu_payload_header(batom_cursor *cursor, bmmt_mpu_header *header);
int bmmt_parse_mpu_payload_mfu(batom_cursor *cursor, const bmmt_mpu_header *header, bmmt_timed_mfu_data *mfu, unsigned max_mfu, unsigned *parsed_mfu);

typedef struct bmmt_signalling_header {
    uint8_t f_i;
    bool h;
    bool aggregation_flag;
    uint8_t frag_count;
} bmmt_signalling_header;

int bmmt_parse_signalling_header(batom_cursor *cursor, bmmt_signalling_header *header);

typedef struct bmmt_signalling_message_data {
    unsigned length;
    batom_cursor message;
} bmmt_signalling_message_data;

int bmmt_parse_signalling_message(batom_cursor *cursor, const bmmt_signalling_header *header, bmmt_signalling_message_data *data, unsigned max_messages, unsigned *parsed_messages);

typedef struct bmmt_signalling_message_header {
    uint16_t message_id;
    uint8_t version;
    uint32_t length;
} bmmt_signalling_message_header;

int bmmt_parse_signalling_message_header(batom_cursor *cursor, bmmt_signalling_message_header *header);

typedef struct bmmt_package_access_message {
    uint8_t table_id;
    uint8_t table_version;
    uint16_t table_length;
    batom_cursor payload;
} bmmt_package_access_message;

int bmmt_parse_package_access_message(batom_cursor *cursor, bmmt_package_access_message *message, unsigned max_messages, unsigned *parsed_messages);

typedef struct bmmt_mp_table_header {
    unsigned MP_table_mode;
    unsigned MMT_package_id_length;
    unsigned MP_table_descriptors_length;
    unsigned number_of_assets;
    uint8_t MMT_package_id[256+1];
    uint8_t MP_table_descriptors[256+1];
} bmmt_mp_table_header;

int bmmt_parse_mp_table_header(batom_cursor *cursor, bmmt_mp_table_header *table);



int bmmt_parse_variable_data(batom_cursor *cursor, bmmt_variable_data *data);
void bmmt_variable_data_init(bmmt_variable_data *data);
void bmmt_variable_data_shutdown(bmmt_variable_data *data);

typedef struct bmmt_asset_id {
    uint32_t asset_id_scheme;
    uint8_t asset_id_length;
    uint8_t asset_id_byte[256+1];
} bmmt_asset_id;

int bmmt_parse_asset_id(batom_cursor *cursor, bmmt_asset_id *data);

typedef struct bmmt_identifier_mapping {
    enum {
        bmmt_identifier_type_asset_id = 0x00,
        bmmt_identifier_type_url = 0x01,
        bmmt_identifier_type_regex = 0x02,
        bmmt_identifier_type_representation_id = 0x03
    } identifier_type;
    union {
        bmmt_asset_id asset_id;
        struct {
            unsigned URL_count;
            bmmt_variable_data urls[16];
        } url;
        bmmt_variable_data regex;
        bmmt_variable_data representation_id;
        bmmt_variable_data _private;
    } data;
} bmmt_identifier_mapping;

void bmmt_identifier_mapping_init(bmmt_identifier_mapping *data);
void bmmt_identifier_mapping_shutdown(bmmt_identifier_mapping *data);
int bmmt_parse_identifier_mapping(batom_cursor *cursor, bmmt_identifier_mapping *data);



void bmmt_general_location_info_init(bmmt_general_location_info *data);
void bmmt_general_location_info_shutdown(bmmt_general_location_info *data);
int bmmt_parse_general_location_info(batom_cursor *cursor, bmmt_general_location_info *data);

typedef struct bmmt_package_table_asset {
    bmmt_identifier_mapping identifier_mapping;
    uint32_t asset_type;
    bool asset_clock_relation_flag;
    struct {
        uint8_t asset_clock_relation_id;
        bool asset_timescale_flag;
        uint32_t asset_timescale;
    } asset_clock_relation;
    struct {
        uint8_t location_count;
        bmmt_general_location_info *locations;
        struct {
            bmmt_general_location_info first_location;
        } data;
    } asset_location;
    struct {
        unsigned asset_descriptors_length;
        batom_cursor asset_descriptors_bytes;
    } asset_descriptors;
} bmmt_package_table_asset;

void bmmt_package_table_asset_init(bmmt_package_table_asset *data);
void bmmt_package_table_asset_shutdown(bmmt_package_table_asset *data);
int bmmt_parse_package_table_asset(batom_cursor *cursor, bmmt_package_table_asset *asset);

typedef struct bmmt_mpu_timestamp_descriptor_header {
    unsigned N;
} bmmt_mpu_timestamp_descriptor_header;
int bmmt_parse_mpu_timestamp_descriptor_header(batom_cursor *cursor, bmmt_mpu_timestamp_descriptor_header *data);


typedef struct bmmt_mpu_timestamp_descriptor_entry {
    uint32_t mpu_sequence_number;
    btlv_ntp_time mpu_presentation_time;
} bmmt_mpu_timestamp_descriptor_entry;

int bmmt_parse_mpu_timestamp_descriptor_entry(batom_cursor *cursor, bmmt_mpu_timestamp_descriptor_entry *data);

typedef struct bmmt_mpu_extended_timestamp_descriptor_header {
    unsigned pts_offset_type;
    bool timescale_flag;
    uint32_t timescale;
    uint16_t default_pts_offset;
} bmmt_mpu_extended_timestamp_descriptor_header;

typedef struct bmmt_mpu_extended_timestamp_descriptor_entry {
    uint32_t mpu_sequence_number;
    uint8_t mpu_presentation_time_leap_indicator;
    uint16_t mpu_decoding_time_offset;
    uint8_t num_of_au;
    struct {
        uint16_t dts_pts_offset;
        uint16_t pts_offset;
    } au[128];
} bmmt_mpu_extended_timestamp_descriptor_entry;

int bmmt_parse_mpu_extended_timestamp_descriptor(batom_cursor *cursor, bmmt_mpu_extended_timestamp_descriptor_header *header, unsigned max_entries, bmmt_mpu_extended_timestamp_descriptor_entry *entry, unsigned *parsed_entries);


int bmmt_parse_pl_table(batom_cursor *cursor, bmmt_pl_table *table);
int bmmt_print_pl_table(bmmt_pl_table *table);

int bmmt_parse_mp_table(batom_cursor *cursor, bmmt_mp_table *table);
int bmmt_print_mp_table(bmmt_mp_table *table);


#ifdef __cplusplus
}
#endif


#endif /* __BMMT_PARSER_H__ */
