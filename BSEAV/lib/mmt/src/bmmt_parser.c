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
#include "bmmt_parser.h"
#include "bkni.h"
#include <stdio.h>
#include <stdlib.h>

BDBG_MODULE(bmmt_parser);

#define BDBG_MSG_TRACE(x)  BDBG_MSG(x)
#define BDBG_TRACE_ERROR(x) (BDBG_LOG(("Error: %s (%s,%u)",#x,__FILE__,__LINE__)),x)

int bmmt_parse_mmt_header(batom_cursor *cursor,bmmt_packet_header *header)
{
    unsigned data;
    unsigned version;
    uint8_t i=0;

    /* ISO/IEC 23008-1
     * 8.2.2 Structure of an MMTP Packet
     * Figure 7.
     */
    data = batom_cursor_uint32_be(cursor);
    if(BATOM_IS_EOF(cursor)) {
        return BDBG_TRACE_ERROR(BTLV_RESULT_END_OF_DATA);
    }
    version = B_GET_BITS(data,31,30);
    if(version != 0) {
        BDBG_ERR(("Not supported version:%u", version));
        return BDBG_TRACE_ERROR(BTLV_RESULT_FORMAT_ERROR);
    }
    header->rap_flag = B_GET_BIT(data,24);
    header->type = B_GET_BITS(data,21,16);
    header->packet_id = B_GET_BITS(data,15,0);
    header->timestamp = batom_cursor_uint32_be(cursor);
    header->packet_seq_num = batom_cursor_uint32_be(cursor);
    header->packet_counter_valid = B_GET_BIT(data,29);
    header->packet_counter = 0;
    if(header->packet_counter_valid) {
        header->packet_counter = batom_cursor_uint32_be(cursor);
    }
    if(B_GET_BIT(data,25)) {
        unsigned ext_length=0;
        uint16_t hdr_ext_length=0;
        batom_cursor_uint16_be(cursor); /* header_extension type */
        ext_length = batom_cursor_uint16_be(cursor); /* header_extension length */
        if(BATOM_IS_EOF(cursor)) {
            return BDBG_TRACE_ERROR(BTLV_RESULT_END_OF_DATA);
        }
        header->hdr_ext_type = batom_cursor_uint16_be(cursor);
        header->hdr_ext_end_flag = (header->hdr_ext_type & 0x8000)? true:false;
        header->hdr_ext_type = ( header->hdr_ext_type & 0x7fff);
        hdr_ext_length = batom_cursor_uint16_be(cursor);
        if(BATOM_IS_EOF(cursor)) {
            return BDBG_TRACE_ERROR(BTLV_RESULT_END_OF_DATA);
        }
        header->scrambling_control = batom_cursor_byte(cursor);
        header->scrambling_control = header->scrambling_control & 0x1f;
        header->scrambling_method_present = (0x04 &  header->scrambling_control)? true:false;
        header->scrambling_auth_present =  (0x02 &  header->scrambling_control)? true:false;
        header->scrambling_iv_present =  (0x01 &  header->scrambling_control)? true:false;
        header->scrambling_control = (0x18 &  header->scrambling_control) >> 3;
        /*batom_cursor_skip(cursor, length);*/
        BDBG_MSG((" ext_len %d hdr_ext_len %d",ext_length,hdr_ext_length));
        BDBG_MSG((" header->scrambling_control %x",header->scrambling_control));
        BDBG_MSG((" header->scrambling_method_present %s",header->scrambling_method_present?"true":"false"));
        BDBG_MSG((" header->scrambling_auth_present %s",header->scrambling_auth_present?"true":"false"));
        BDBG_MSG((" header->scrambling_iv_present %s",header->scrambling_iv_present?"true":"false"));
        if (header->scrambling_iv_present) {

           if (batom_cursor_size(cursor) < BMMT_MAX_IV_SIZE) {
              return BDBG_TRACE_ERROR(BTLV_RESULT_END_OF_DATA);
           }
           else {
              batom_cursor_copy(cursor,header->iv,BMMT_MAX_IV_SIZE);
           }
        }
        else {
           header->iv[1] = header->packet_id & 0x00ff;
           header->iv[0] = (header->packet_id & 0xff00) >> 8;
           header->iv[5] = (header->packet_seq_num & 0x000000ff);
           header->iv[4] = (header->packet_seq_num & 0x0000ff00) >> 8;
           header->iv[3] = (header->packet_seq_num & 0x00ff0000) >> 16;
           header->iv[2] = (header->packet_seq_num & 0xff000000) >> 24;
           for (i=6;i<sizeof(header->iv);i++) {
               header->iv[i] = 0x0;
           }
        }

    }
    else
    {
       header->hdr_ext_type = 0;
       header->scrambling_control = 0;
    }
    if(BATOM_IS_EOF(cursor)) {
        return BDBG_TRACE_ERROR(BTLV_RESULT_END_OF_DATA);
    }
    BDBG_MSG(("mmt: type:%u packet_id:%u timestamp:%#x packet_sequence_number:%u packet_counter:%u", (unsigned)header->type, (unsigned)header->packet_id, (unsigned)header->timestamp, (unsigned)header->packet_seq_num, (unsigned)header->packet_counter));
    return 0;
}

int bmmt_parse_mpu_payload_header(batom_cursor *cursor, bmmt_mpu_header *header)
{
    unsigned data;
    /* ISO/IEC 23008-1
     * 8.3.2 The MPU Mode
     */
    data = batom_cursor_uint32_be(cursor);
    header->length = B_GET_BITS(data,32,16);
    header->type = B_GET_BITS(data,15,12);
    header->timed = B_GET_BIT(data,11);
    header->f_i = B_GET_BITS(data,10, 9);
    header->aggregation = B_GET_BIT(data, 8);
    header->frag_counter = B_GET_BITS(data,7,0);
    header->sequence_number = batom_cursor_uint32_be(cursor);
    if(BATOM_IS_EOF(cursor)) {
        return BDBG_TRACE_ERROR(BTLV_RESULT_END_OF_DATA);
    }
    BDBG_MSG(("mpu: length:%u type:%u timed:%u f_i:%u aggregation:%u fragment_counter:%u sequence_number:%u", (unsigned)header->length, (unsigned)header->type, (unsigned)header->timed, (unsigned)header->f_i, (unsigned)header->aggregation, (unsigned)header->frag_counter, (unsigned)header->sequence_number));
    return 0;
}

int bmmt_parse_mpu_payload_mfu(batom_cursor *cursor, const bmmt_mpu_header *header, bmmt_timed_mfu_data *mfu, unsigned max_mfu, unsigned *parsed_mfu)
{
    /*
     * MMT-BASED MEDIA TRANSPORT SCHEME
     * ARIB STD-B60 1.5
     * Table 6-1 MMTP configuration of payload
     */
    if(max_mfu<=0) {
        return BDBG_TRACE_ERROR(BTLV_RESULT_UNKNOWN);
    }
    if(!header->aggregation) {
        mfu[0].movie_fragment_sequence_number = batom_cursor_uint32_be(cursor);
        mfu[0].sample_number = batom_cursor_uint32_be(cursor);
        mfu[0].offset = batom_cursor_uint32_be(cursor);
        mfu[0].priority = batom_cursor_byte(cursor);
        mfu[0].dependency_counter = batom_cursor_byte(cursor);
        if(BATOM_IS_EOF(cursor)) {
            return BDBG_TRACE_ERROR(BTLV_RESULT_END_OF_DATA);
        }
        mfu[0].length = batom_cursor_size(cursor);
        BATOM_CLONE(&mfu[0].payload,cursor);
        *parsed_mfu = 1;
        BDBG_MSG(("mfu: %u bytes movie_fragment_sequence_number:%u sample_number:%u offset:%u priority:%u dependency_counter:%u", (unsigned)mfu[0].length, (unsigned)mfu[0].movie_fragment_sequence_number, (unsigned)mfu[0].sample_number, (unsigned)mfu[0].offset, (unsigned)mfu[0].priority, (unsigned)mfu[0].dependency_counter));
    } else {
        unsigned i;
        for(i=0;;) {
            uint16_t data_unit_length;
            const unsigned mfu_header_length =
               BMEDIA_FIELD_LEN("movie_fragment_sequence_number",uint32_t) +
               BMEDIA_FIELD_LEN("sample_number",uint32_t) +
               BMEDIA_FIELD_LEN("offset",uint32_t) +
               BMEDIA_FIELD_LEN("priority",uint8_t) +
               BMEDIA_FIELD_LEN("dependency_counter",uint8_t);
            if(i>=max_mfu) {
                break;
            }
            data_unit_length = batom_cursor_uint16_be(cursor);
            if(BATOM_IS_EOF(cursor)) {
                break;
            }
            mfu[i].movie_fragment_sequence_number = batom_cursor_uint32_be(cursor);
            mfu[i].sample_number = batom_cursor_uint32_be(cursor);
            mfu[i].offset = batom_cursor_uint32_be(cursor);
            mfu[i].priority = batom_cursor_byte(cursor);
            mfu[i].dependency_counter = batom_cursor_byte(cursor);
            if(BATOM_IS_EOF(cursor)) {
                return BDBG_TRACE_ERROR(BTLV_RESULT_END_OF_DATA);
            }
            if(data_unit_length<mfu_header_length) {
                return BDBG_TRACE_ERROR(BTLV_RESULT_UNKNOWN);
            }
            data_unit_length -= mfu_header_length;
            mfu[i].length = data_unit_length;
            BATOM_CLONE(&mfu[i].payload,cursor);
            if(batom_cursor_skip(cursor,data_unit_length) != data_unit_length) {
                return BDBG_TRACE_ERROR(BTLV_RESULT_END_OF_DATA);
            }
            BDBG_MSG(("mfu[%u]: %u bytes movie_fragment_sequence_number:%u sample_number:%u offset:%u priority:%u dependency_counter:%u", i, (unsigned)mfu[i].length, (unsigned)mfu[i].movie_fragment_sequence_number, (unsigned)mfu[i].sample_number, (unsigned)mfu[i].offset, (unsigned)mfu[i].priority, (unsigned)mfu[i].dependency_counter));
            i++;
        }
        *parsed_mfu = i;
    }
    return 0;
}

int bmmt_parse_signalling_header(batom_cursor *cursor, bmmt_signalling_header *header)
{
    unsigned data;

    /* ISO/IEC 23008-1
     * 8.3.4. Signalling Message Mode
     */
    data = batom_cursor_uint16_be(cursor);
    header->f_i = B_GET_BITS(data, 15,14);
    header->h = B_GET_BIT(data,9);
    header->aggregation_flag = B_GET_BIT(data,8);
    header->frag_count = B_GET_BITS(data, 7,0);
    if(BATOM_IS_EOF(cursor)) {
        return BDBG_TRACE_ERROR(BTLV_RESULT_END_OF_DATA);
    }
    BDBG_MSG(("signalling: f_i:%u h:%u aggregation:%u fragment_counter:%u", (unsigned)header->f_i, (unsigned)header->h, (unsigned)header->aggregation_flag, (unsigned)header->frag_count));
    return 0;
}


int bmmt_parse_signalling_message(batom_cursor *cursor, const bmmt_signalling_header *header, bmmt_signalling_message_data *data, unsigned max_messages, unsigned *parsed_messages)
{
    unsigned i;
    if(max_messages<=0) {
        return BDBG_TRACE_ERROR(BTLV_RESULT_FORMAT_ERROR);
    }
    /* ISO/IEC 23008-1
     * 8.3.4.2 MMTP payload header for Signalling message mode
     */
    if(!header->aggregation_flag) {
        data[0].length = batom_cursor_size(cursor);
        BATOM_CLONE(&data[0].message, cursor);
        *parsed_messages = 1;
        return 0;
    }
    for(i=0;;) {
        unsigned message_length;
        if(header->h) {
            message_length = batom_cursor_uint32_be(cursor);
        } else {
            message_length = batom_cursor_uint16_be(cursor);
        }
        if(BATOM_IS_EOF(cursor)) {
            break;
        }
        if(i>=max_messages) {
            return BDBG_TRACE_ERROR(BTLV_RESULT_TOO_MUCH_DATA);
        }
        data[i].length = message_length;
        BDBG_MSG(("signalling message[%u]: length:%u", i, data[i].length));
        BATOM_CLONE(&data[i].message,cursor);
        if(batom_cursor_skip(cursor,message_length) != message_length) {
            return BDBG_TRACE_ERROR(BTLV_RESULT_END_OF_DATA);
        }
    }
    *parsed_messages = i;
    return 0;
}

int bmmt_parse_signalling_message_header(batom_cursor *cursor, bmmt_signalling_message_header *header)
{
    header->message_id = batom_cursor_uint16_be(cursor);
    header->version = batom_cursor_byte(cursor);
    if(BATOM_IS_EOF(cursor)) {
        return BDBG_TRACE_ERROR(BTLV_RESULT_END_OF_DATA);
    }
    if( BMMT_IS_PA_MESSAGE(header->message_id) || BMMT_IS_MPI_MESSAGE(header->message_id)) {
        header->length = batom_cursor_uint32_be(cursor);
    } else {
        header->length = batom_cursor_uint16_be(cursor);
    }
    if(BATOM_IS_EOF(cursor)) {
        return BDBG_TRACE_ERROR(BTLV_RESULT_END_OF_DATA);
    }
    BDBG_MSG(("message header: message_id:%u version:%u length:%u", (unsigned)header->message_id, (unsigned)header->version, (unsigned)header->length));
    return 0;
}

int bmmt_parse_package_access_message(batom_cursor *cursor, bmmt_package_access_message *message, unsigned max_messages, unsigned *parsed_messages)
{
    unsigned number_of_tables;
    unsigned i;
    /* ISO/IEC 23008-1
     * 9.3.2 The PA Message
     */

    number_of_tables = batom_cursor_byte(cursor);
    if(BATOM_IS_EOF(cursor)) {
        return BDBG_TRACE_ERROR(BTLV_RESULT_END_OF_DATA);
    }
    BDBG_MSG(("package_access_message: number_of_tables:%u", number_of_tables));
    if(number_of_tables>=max_messages) {
        return BDBG_TRACE_ERROR(BTLV_RESULT_TOO_MUCH_DATA);
    }

    if(number_of_tables==0)
    {
        for(i=0;;i++)
        {
            uint8_t table_id;
            uint8_t table_version;
            uint16_t table_length;
            table_id = batom_cursor_byte(cursor);
            message[i].table_id = table_id;
            if(BATOM_IS_EOF(cursor))
            {
                break;
            }
            table_version = batom_cursor_byte(cursor);
            message[i].table_version = table_version;
            table_length = batom_cursor_uint16_be(cursor);
            message[i].table_length = table_length;
            if(BATOM_IS_EOF(cursor))
            {
                return BDBG_TRACE_ERROR(BTLV_RESULT_END_OF_DATA);
            }
            BDBG_MSG(("package_access_message[%u'']: table_id:%#x table_version:%u table_length:%u", i, (unsigned)table_id, (unsigned)table_version, (unsigned)table_length));
            BATOM_CLONE(&message[i].payload, cursor);
            if(batom_cursor_skip(cursor, table_length)!=table_length)
            {
                return BDBG_TRACE_ERROR(BTLV_RESULT_END_OF_DATA);
            }
        }
        *parsed_messages = i;
        BDBG_MSG(("parsed_messages %u",i));
        return 0;
    }

    for(i=0;i<number_of_tables;i++) {
        message[i].table_id = batom_cursor_byte(cursor);
        message[i].table_version = batom_cursor_byte(cursor);
        message[i].table_length = batom_cursor_uint16_be(cursor);
        if(BATOM_IS_EOF(cursor)) {
            return BDBG_TRACE_ERROR(BTLV_RESULT_END_OF_DATA);
        }
        BDBG_MSG(("package_access_message[%u']: table_id:%#x table_version:%u table_length:%u", i, (unsigned)message[i].table_id, (unsigned)message[i].table_version, (unsigned)message[i].table_length));
    }
    for(i=0;i<number_of_tables;i++) {
        uint8_t table_id;
        uint8_t table_version;
        uint16_t table_length;

        table_id = batom_cursor_byte(cursor);
        table_version = batom_cursor_byte(cursor);
        table_length = batom_cursor_uint16_be(cursor);
        if(BATOM_IS_EOF(cursor)) {
            return BDBG_TRACE_ERROR(BTLV_RESULT_END_OF_DATA);
        }
        BDBG_MSG(("package_access_message[%u'']: table_id:%#x table_version:%u table_length:%u", i, (unsigned)table_id, (unsigned)table_version, (unsigned)table_length));
        if(table_id != message[i].table_id || table_version != message[i].table_version || table_version != message[i].table_version) {
            return BDBG_TRACE_ERROR(BTLV_RESULT_FORMAT_ERROR);
        }
        BATOM_CLONE(&message[i].payload, cursor);
        if(batom_cursor_skip(cursor, table_length)!=table_length) {
            return BDBG_TRACE_ERROR(BTLV_RESULT_END_OF_DATA);
        }
    }
    *parsed_messages = number_of_tables;
    return 0;
}

int bmmt_parse_mp_table_header(batom_cursor *cursor, bmmt_mp_table_header *table)
{
    unsigned data;

    /* ISO/IEC 23008-1
     * 9.3.9 The MP table
     */
    data = batom_cursor_byte(cursor);
    if(BATOM_IS_EOF(cursor)) {
        return BDBG_TRACE_ERROR(BTLV_RESULT_END_OF_DATA);
    }
    table->MP_table_mode = B_GET_BITS(data,1,0);
    table->MMT_package_id_length = batom_cursor_byte(cursor);
    if(BATOM_IS_EOF(cursor)) {
        return BDBG_TRACE_ERROR(BTLV_RESULT_END_OF_DATA);
    }
    if(table->MMT_package_id_length >= sizeof(table->MMT_package_id)) {
        return BDBG_TRACE_ERROR(BTLV_RESULT_TOO_MUCH_DATA);
    }
    if(batom_cursor_copy(cursor, table->MMT_package_id, table->MMT_package_id_length)!=table->MMT_package_id_length) {
        return BDBG_TRACE_ERROR(BTLV_RESULT_END_OF_DATA);
    }
    table->MMT_package_id[table->MMT_package_id_length+1]='\0';

    table->MP_table_descriptors_length = batom_cursor_uint16_be(cursor);
    if(BATOM_IS_EOF(cursor)) {
        return BDBG_TRACE_ERROR(BTLV_RESULT_END_OF_DATA);
    }
    if(table->MP_table_descriptors_length >= sizeof(table->MP_table_descriptors)) {
        return BDBG_TRACE_ERROR(BTLV_RESULT_TOO_MUCH_DATA);
    }
    if(batom_cursor_copy(cursor, table->MP_table_descriptors, table->MP_table_descriptors_length)!=table->MP_table_descriptors_length) {
        return BDBG_TRACE_ERROR(BTLV_RESULT_END_OF_DATA);
    }
    table->number_of_assets = batom_cursor_byte(cursor);
    if(BATOM_IS_EOF(cursor)) {
        return BDBG_TRACE_ERROR(BTLV_RESULT_END_OF_DATA);
    }
    BDBG_MSG(("mp_table_header: MP_table_mode:%u MMT_package_id_length:%u MP_table_descriptors_length:%u number_of_assets:%u", table->MP_table_mode, table->MMT_package_id_length, table->MP_table_descriptors_length, table->number_of_assets));
    return 0;
}

int bmmt_parse_variable_data(batom_cursor *cursor, bmmt_variable_data *data)
{
    data->length = batom_cursor_uint16_be(cursor);
    if(BATOM_IS_EOF(cursor)) {
        return BDBG_TRACE_ERROR(BTLV_RESULT_END_OF_DATA);
    }
    data->data = BKNI_Malloc(data->length+1);
    if(data->data==NULL) {
        return BDBG_TRACE_ERROR(BTLV_RESULT_OUT_OF_MEMORY);
    }
    if(batom_cursor_copy(cursor, data->data, data->length)!=data->length) {
        BKNI_Free(data->data);
        return BDBG_TRACE_ERROR(BTLV_RESULT_END_OF_DATA);
    }
    ((char *)data->data)[data->length] = '\0';
    return 0;
}

void bmmt_variable_data_init(bmmt_variable_data *data)
{
    data->data = NULL;
    data->length = 0;
    return;
}

void bmmt_variable_data_shutdown(bmmt_variable_data *data)
{
    if(data->data) {
        BKNI_Free(data->data);
        data->data = NULL;
    }
    return ;
}

int bmmt_parse_asset_id(batom_cursor *cursor, bmmt_asset_id *data)
{
    unsigned asset_id_length;
    /* ISO/IEC 23008-1
     * 9.6.2 asset_id
     */
    data->asset_id_scheme = batom_cursor_uint32_be(cursor);
    asset_id_length = batom_cursor_byte(cursor);
    data->asset_id_length = asset_id_length;
    if(BATOM_IS_EOF(cursor)) {
        return BDBG_TRACE_ERROR(BTLV_RESULT_END_OF_DATA);
    }
    if(asset_id_length >= sizeof(data->asset_id_byte)) {
        return BDBG_TRACE_ERROR(BTLV_RESULT_UNKNOWN);
    }
    if(batom_cursor_copy(cursor, data->asset_id_byte, data->asset_id_length)!=data->asset_id_length) {
        return BDBG_TRACE_ERROR(BTLV_RESULT_END_OF_DATA);
    }
    data->asset_id_byte[data->asset_id_length] = '\0';
    return 0;
}

void bmmt_identifier_mapping_init(bmmt_identifier_mapping *data)
{
    data->identifier_type = bmmt_identifier_type_asset_id;
    return;
}

void bmmt_identifier_mapping_shutdown(bmmt_identifier_mapping *data)
{
    unsigned i;
    switch(data->identifier_type) {
    case bmmt_identifier_type_asset_id:
        break;
    case bmmt_identifier_type_url:
        for(i=0;i<data->data.url.URL_count;i++) {
            bmmt_variable_data_shutdown(&data->data.url.urls[i]);
        }
        break;
    case bmmt_identifier_type_regex:
        bmmt_variable_data_shutdown(&data->data.regex);
        break;
    case bmmt_identifier_type_representation_id:
        bmmt_variable_data_shutdown(&data->data.representation_id);
        break;
    default:
        bmmt_variable_data_shutdown(&data->data._private);
        break;
    }
    data->identifier_type = bmmt_identifier_type_asset_id;
    return;
}

int bmmt_parse_identifier_mapping(batom_cursor *cursor, bmmt_identifier_mapping *data)
{
    int rc;
    unsigned i;
    unsigned identifier_type;
    /* ISO/IEC 23008-1
     * 9.6.3 Identifier mapping
     */
    identifier_type = batom_cursor_byte(cursor);
    if(BATOM_IS_EOF(cursor)) {
        return BDBG_TRACE_ERROR(BTLV_RESULT_END_OF_DATA);
    }
    switch(identifier_type) {
    case bmmt_identifier_type_asset_id:
        rc = bmmt_parse_asset_id(cursor, &data->data.asset_id);
        if(rc!=0) {
            return BDBG_TRACE_ERROR(rc);
        }
        break;
    case bmmt_identifier_type_url:
        data->data.url.URL_count = batom_cursor_uint16_be(cursor);
        if(BATOM_IS_EOF(cursor)) {
            return BDBG_TRACE_ERROR(BTLV_RESULT_TOO_MUCH_DATA);
        }
        if(data->data.url.URL_count>=sizeof(data->data.url.urls)/sizeof(data->data.url.urls[0])) {
            return BDBG_TRACE_ERROR(BTLV_RESULT_TOO_MUCH_DATA);
        }
        for(i=0;i<data->data.url.URL_count;i++) {
            rc = bmmt_parse_variable_data(cursor, &data->data.url.urls[i]);
            if(rc!=0) {
                unsigned j;
                for(j=0;j<i;j++) {
                    bmmt_variable_data_shutdown(&data->data.url.urls[i]);
                }
                return BDBG_TRACE_ERROR(rc);
            }
        }
        break;
    case bmmt_identifier_type_regex:
        rc = bmmt_parse_variable_data(cursor, &data->data.regex);
        if(rc!=0) {
            return BDBG_TRACE_ERROR(rc);
        }
        break;
    case bmmt_identifier_type_representation_id:
        rc = bmmt_parse_variable_data(cursor, &data->data.representation_id);
        if(rc!=0) {
            return BDBG_TRACE_ERROR(rc);
        }
        break;
    default:
        rc = bmmt_parse_variable_data(cursor, &data->data._private);
        if(rc!=0) {
            return BDBG_TRACE_ERROR(rc);
        }
        break;
    }
    data->identifier_type = identifier_type;
    return 0;
}

void bmmt_general_location_info_init(bmmt_general_location_info *data)
{
    data->location_type = bmmt_general_location_type_id;
    return;
}

void bmmt_general_location_info_shutdown(bmmt_general_location_info *data)
{
    switch(data->location_type) {
    case bmmt_general_location_type_private:
        bmmt_variable_data_shutdown(&data->data._private);
        break;
    default:
        break;
    }
    data->location_type = bmmt_general_location_type_id;
    return;
}

int bmmt_parse_general_location_info(batom_cursor *cursor, bmmt_general_location_info *data)
{
    int rc;
    /* ISO/IEC 23008-1
     * 9.6.1 MMT_general_location_info
     */

    data->location_type = batom_cursor_byte(cursor);
    if(BATOM_IS_EOF(cursor)) {
        return BDBG_TRACE_ERROR(BTLV_RESULT_END_OF_DATA);
    }
    switch(data->location_type) {
    case bmmt_general_location_type_id:
        data->data.packet_id = batom_cursor_uint16_be(cursor);
        if(BATOM_IS_EOF(cursor)) {
            return BDBG_TRACE_ERROR(BTLV_RESULT_END_OF_DATA);
        }
        break;
    case bmmt_general_location_type_ipv4:
        batom_cursor_copy(cursor,data->data.mmt_ipv4.ipv4_src_addr,sizeof(data->data.mmt_ipv4.ipv4_src_addr));
        batom_cursor_copy(cursor,data->data.mmt_ipv4.ipv4_dst_addr,sizeof(data->data.mmt_ipv4.ipv4_dst_addr));
        data->data.mmt_ipv4.dst_port = batom_cursor_uint16_be(cursor);
        data->data.mmt_ipv4.packet_id = batom_cursor_uint16_be(cursor);
        if(BATOM_IS_EOF(cursor)) {
            return BDBG_TRACE_ERROR(BTLV_RESULT_END_OF_DATA);
        }
        break;
    case bmmt_general_location_type_ipv6:
        batom_cursor_copy(cursor,data->data.mmt_ipv6.ipv6_src_addr,sizeof(data->data.mmt_ipv6.ipv6_src_addr));
        batom_cursor_copy(cursor,data->data.mmt_ipv6.ipv6_dst_addr,sizeof(data->data.mmt_ipv6.ipv6_dst_addr));
        data->data.mmt_ipv6.dst_port = batom_cursor_uint16_be(cursor);
        data->data.mmt_ipv6.packet_id = batom_cursor_uint16_be(cursor);
        if(BATOM_IS_EOF(cursor)) {
            return BDBG_TRACE_ERROR(BTLV_RESULT_END_OF_DATA);
        }
        break;
    case bmmt_general_location_type_mpeg2ts:
        data->data.mpeg2ts.network_id = batom_cursor_uint16_be(cursor);
        data->data.mpeg2ts.MPEG_2_transport_stream_id = batom_cursor_uint16_be(cursor);
        data->data.mpeg2ts.MPEG_2_PID = batom_cursor_uint16_be(cursor);
        data->data.mpeg2ts.MPEG_2_PID = B_GET_BITS(data->data.mpeg2ts.MPEG_2_PID, 13, 0);
        if(BATOM_IS_EOF(cursor)) {
            return BDBG_TRACE_ERROR(BTLV_RESULT_END_OF_DATA);
        }
        break;
    case bmmt_general_location_type_mpeg2ts_ipv6:
        batom_cursor_copy(cursor,data->data.mpeg2ts_ipv6.ipv6_src_addr,sizeof(data->data.mpeg2ts_ipv6.ipv6_src_addr));
        batom_cursor_copy(cursor,data->data.mpeg2ts_ipv6.ipv6_dst_addr,sizeof(data->data.mpeg2ts_ipv6.ipv6_dst_addr));
        data->data.mpeg2ts_ipv6.dst_port = batom_cursor_uint16_be(cursor);
        data->data.mpeg2ts_ipv6.MPEG_2_PID = B_GET_BITS(data->data.mpeg2ts_ipv6.MPEG_2_PID, 13, 0);
        if(BATOM_IS_EOF(cursor)) {
            return BDBG_TRACE_ERROR(BTLV_RESULT_END_OF_DATA);
        }
        break;
    case bmmt_general_location_type_url:
        data->data.url.URL_length = batom_cursor_byte(cursor);
        if(BATOM_IS_EOF(cursor)) {
            return BDBG_TRACE_ERROR(BTLV_RESULT_END_OF_DATA);
        }
        if(data->data.url.URL_length >= sizeof(data->data.url.URL_byte)) {
            return BDBG_TRACE_ERROR(BTLV_RESULT_TOO_MUCH_DATA);
        }
        if(batom_cursor_copy(cursor, data->data.url.URL_byte, data->data.url.URL_length) != data->data.url.URL_length) {
            return BDBG_TRACE_ERROR(BTLV_RESULT_END_OF_DATA);
        }
        data->data.url.URL_byte[data->data.url.URL_length] = '\0';
        break;
    case bmmt_general_location_type_private:
        rc = bmmt_parse_variable_data(cursor, &data->data._private);
        if(rc!=0) {
            return BDBG_TRACE_ERROR(rc);
        }
        break;
    }
    return 0;
}

void bmmt_package_table_asset_init(bmmt_package_table_asset *data)
{
    BKNI_Memset(data, 0, sizeof(*data));
    bmmt_identifier_mapping_init(&data->identifier_mapping);
    data->asset_descriptors.asset_descriptors_length = 0;
    data->asset_location.location_count = 0;
    return ;
}

void bmmt_package_table_asset_shutdown(bmmt_package_table_asset *data)
{
    unsigned i;
    bmmt_identifier_mapping_shutdown(&data->identifier_mapping);
    for(i=0;i<data->asset_location.location_count;i++) {
        bmmt_general_location_info_shutdown(&data->asset_location.locations[i]);
    }
    data->asset_location.location_count = 0;
    data->asset_descriptors.asset_descriptors_length = 0;
    return ;
}

int bmmt_parse_package_table_asset(batom_cursor *cursor, bmmt_package_table_asset *asset)
{
    int rc;
    unsigned data;

    /* ISO/IEC 23008-1
     * 9.3.9 The MP table
     */

    rc = bmmt_parse_identifier_mapping(cursor, &asset->identifier_mapping);
    if(rc!=0) {
        return BDBG_TRACE_ERROR(rc);
    }
    asset->asset_type = batom_cursor_uint32_le(cursor);
    data = batom_cursor_byte(cursor);
    if(BATOM_IS_EOF(cursor)) {
        rc = BDBG_TRACE_ERROR(BTLV_RESULT_END_OF_DATA);
        goto err_asset_clock_relation_flag;
    }
    asset->asset_clock_relation_flag = B_GET_BIT(data,0);
    if(asset->asset_clock_relation_flag) {
        asset->asset_clock_relation.asset_clock_relation_id = batom_cursor_byte(cursor);
        data = batom_cursor_byte(cursor);
        if(BATOM_IS_EOF(cursor)) {
            rc = BDBG_TRACE_ERROR(BTLV_RESULT_END_OF_DATA);
            goto err_asset_timescale_flag;
        }
        asset->asset_clock_relation.asset_timescale_flag = B_GET_BIT(data,0);
        if(asset->asset_clock_relation.asset_timescale_flag) {
            asset->asset_clock_relation.asset_timescale = batom_cursor_uint32_be(cursor);
        }
    }
    asset->asset_location.location_count = batom_cursor_byte(cursor);
    if(BATOM_IS_EOF(cursor)) {
        rc = BDBG_TRACE_ERROR(BTLV_RESULT_END_OF_DATA);
        goto err_location_count;
    }
    if(asset->asset_location.location_count!=1) {
        rc = BDBG_TRACE_ERROR(BTLV_RESULT_UNKNOWN);
        goto err_location_count;
    }
    asset->asset_location.locations = &asset->asset_location.data.first_location;
    rc = bmmt_parse_general_location_info(cursor, &asset->asset_location.locations[0]);
    if(rc!=0) {
        rc = BDBG_TRACE_ERROR(rc);
        goto err_locations;
    }
    asset->asset_descriptors.asset_descriptors_length = batom_cursor_uint16_be(cursor);
    if(BATOM_IS_EOF(cursor)) {
        rc = BDBG_TRACE_ERROR(BTLV_RESULT_END_OF_DATA);
        goto err_asset_descriptors;
    }
    BATOM_CLONE(&asset->asset_descriptors.asset_descriptors_bytes,cursor);
    if(batom_cursor_skip(cursor, asset->asset_descriptors.asset_descriptors_length) != asset->asset_descriptors.asset_descriptors_length) {
        rc = BDBG_TRACE_ERROR(BTLV_RESULT_END_OF_DATA);
        goto err_asset_descriptors;
    }
    BDBG_MSG(("asset: identifier_type:%u asset_type:" BMEDIA_FOURCC_FORMAT " asset_clock_relation_id:%d location_count:%u asset_descriptors_length:%u", (unsigned)(asset->identifier_mapping.identifier_type), BMEDIA_FOURCC_ARG(asset->asset_type), asset->asset_clock_relation_flag?(int)(asset->asset_clock_relation.asset_clock_relation_id):-1, asset->asset_location.location_count, asset->asset_descriptors.asset_descriptors_length));

    return 0;

err_asset_descriptors:
    bmmt_general_location_info_shutdown(&asset->asset_location.locations[0]);
err_locations:
err_location_count:
    asset->asset_location.location_count = 0;
err_asset_timescale_flag:
    bmmt_identifier_mapping_shutdown(&asset->identifier_mapping);
err_asset_clock_relation_flag:
    return rc;
}


int bmmt_parse_mpu_timestamp_descriptor_header(batom_cursor *cursor, bmmt_mpu_timestamp_descriptor_header *data)
{
    uint16_t descriptor_tag;
    uint8_t descriptor_length;
    const unsigned entry_length = BMEDIA_FIELD_LEN("mpu_sequence_number",uint32_t) + BMEDIA_FIELD_LEN("mpu_presentation_time",uint64_t);
    batom_checkpoint ck;
    /* ISO/IEC 23008-1
     * 9.5.2 MPU timestamp descriptor
     */
    BATOM_SAVE(cursor,&ck);
    descriptor_tag = batom_cursor_uint16_be(cursor);
    descriptor_length = batom_cursor_byte(cursor);
    if(BATOM_IS_EOF(cursor)) {
        BATOM_ROLLBACK(cursor, &ck);
        return BTLV_RESULT_END_OF_DATA;
    }
    /* Table 36 - Descriptor Tag Values */
    if(descriptor_tag != 0x0001) {
        BATOM_ROLLBACK(cursor, &ck);
        return BTLV_RESULT_INVALID_TAG;
    }
    data->N = descriptor_length/entry_length;
    if(descriptor_length%entry_length != 0) {
        return BDBG_TRACE_ERROR(BTLV_RESULT_UNKNOWN);
    }
    BDBG_MSG(("timestamp_descriptor: N:%u", data->N));
    return 0;
}

int bmmt_parse_mpu_timestamp_descriptor_entry(batom_cursor *cursor, bmmt_mpu_timestamp_descriptor_entry *data)
{
    data->mpu_sequence_number = batom_cursor_uint32_be(cursor);
    data->mpu_presentation_time = batom_cursor_uint64_be(cursor);
    if(BATOM_IS_EOF(cursor)) {
        return BDBG_TRACE_ERROR(BTLV_RESULT_END_OF_DATA);
    }
    BDBG_MSG(("timestamp_descriptor: mpu_sequence_number:%u mpu_presentation_time:" BDBG_UINT64_FMT "", (unsigned)data->mpu_sequence_number, BDBG_UINT64_ARG(data->mpu_presentation_time)));
    return 0;
}

int bmmt_parse_mpu_extended_timestamp_descriptor(batom_cursor *cursor, bmmt_mpu_extended_timestamp_descriptor_header *header, unsigned max_entries, bmmt_mpu_extended_timestamp_descriptor_entry *entry, unsigned *parsed_entries)
{
    int rc = 0;
    uint16_t descriptor_tag;
    uint8_t descriptor_length;
    unsigned data;
    size_t start_of_descriptor;
    unsigned i;

    batom_checkpoint ck;
    *parsed_entries = 0;
    /*
     * MMT-BASED MEDIA TRANSPORT SCHEME
     * ARIB STD-B60 1.5
     * 7.4.3.35 MPU extended time stamp descriptor
     * Table 7-82
     */
    BATOM_SAVE(cursor,&ck);
    descriptor_tag = batom_cursor_uint16_be(cursor);
    descriptor_length = batom_cursor_byte(cursor);
    start_of_descriptor = batom_cursor_pos(cursor);
    data = batom_cursor_byte(cursor);
    if(BATOM_IS_EOF(cursor)) {
        rc = BTLV_RESULT_END_OF_DATA;
        goto err_read_tag;
    }
    if(descriptor_tag != 0x8026) {
        rc = BTLV_RESULT_INVALID_TAG;
        goto err_test_tag;
    }
    header->pts_offset_type = B_GET_BITS(data,2,1);
    header->timescale_flag = B_GET_BIT(data,0);
    if(header->timescale_flag) {
        header->timescale = batom_cursor_uint32_be(cursor);
    }
    if(header->pts_offset_type == 1) {
        header->default_pts_offset = batom_cursor_uint16_be(cursor);
    }
    BDBG_MSG(("mpu_extended_timestamp_descriptor: pts_offset_type:%u timescale:%u", header->pts_offset_type, header->timescale_flag?(unsigned)(header->timescale):0));
    for(i=0;;i++) {
        size_t current_pos;
        unsigned j;
        if(BATOM_IS_EOF(cursor)) {
            rc = BDBG_TRACE_ERROR(BTLV_RESULT_END_OF_DATA);
            goto err_descriptor;
        }
        current_pos = batom_cursor_pos(cursor);
        if(current_pos > start_of_descriptor + descriptor_length) {
            rc = BDBG_TRACE_ERROR(BTLV_RESULT_END_OF_DATA);
            goto err_descriptor;
        }
        if(current_pos == start_of_descriptor + descriptor_length) {
            *parsed_entries = i;
            return 0;
        }
        if(i>=max_entries) {
            rc = BDBG_TRACE_ERROR(BTLV_RESULT_TOO_MUCH_DATA);
            goto err_descriptor;
        }
        entry[i].mpu_sequence_number = batom_cursor_uint32_be(cursor);
        data = batom_cursor_byte(cursor);
        entry[i].mpu_presentation_time_leap_indicator = B_GET_BITS(data,7,6);
        entry[i].mpu_decoding_time_offset = batom_cursor_uint16_be(cursor);
        entry[i].num_of_au = batom_cursor_byte(cursor);
        if(entry[i].num_of_au >= sizeof(entry[i].au)/sizeof(entry[i].au[0])) {
            rc = BDBG_TRACE_ERROR(BTLV_RESULT_UNKNOWN);
            goto err_descriptor;
        }
        BDBG_MSG(("mpu_extended_timestamp_descriptor_entry[%u]: mpu_sequence_number:%u mpu_presentation_time_leap_indicator:%u mpu_decoding_time_offset:%u num_of_au:%u", i, entry[i].mpu_sequence_number, entry[i].mpu_presentation_time_leap_indicator, entry[i].mpu_decoding_time_offset, entry[i].num_of_au));
        for(j=0;j<entry[i].num_of_au;j++) {
            entry[i].au[j].dts_pts_offset = batom_cursor_uint16_be(cursor);
            if(header->pts_offset_type==2) {
                entry[i].au[j].pts_offset = batom_cursor_uint16_be(cursor);
            }
        }
    }
    return 0;

err_descriptor:
err_test_tag:
err_read_tag:
    BATOM_ROLLBACK(cursor, &ck);
    return rc;
}

int  bmmt_parse_pl_table(batom_cursor *cursor, bmmt_pl_table *table)
{
    int i,j;
    table->num_of_packages = batom_cursor_byte(cursor);
    for (i=0;i<table->num_of_packages;i++ )
    {
        table->packages[i].package_id_length = batom_cursor_byte(cursor);
        for(j=0;j<table->packages[i].package_id_length;j++)
        {
            table->packages[i].package_id[j] = batom_cursor_byte(cursor);
        }
        bmmt_parse_general_location_info(cursor,&table->packages[i].location_info );
    }
    table->num_of_ip_delivery = batom_cursor_byte(cursor);

    for (i=0;i<table->num_of_ip_delivery;i++ )
    {
        table->ip_flow[i].transport_file_id = batom_cursor_uint32_le(cursor);
        table->ip_flow[i].location_type = batom_cursor_byte(cursor);
        switch (table->ip_flow[i].location_type)
        {
        case 0x01: {
            batom_cursor_copy(cursor,table->ip_flow[i].location_info.ipv4.src_addr,sizeof(table->ip_flow[i].location_info.ipv4.src_addr));
            batom_cursor_copy(cursor,table->ip_flow[i].location_info.ipv4.dst_addr,sizeof(table->ip_flow[i].location_info.ipv4.dst_addr));
            table->ip_flow[i].location_info.ipv4.dst_port = batom_cursor_uint16_be(cursor);
        }
        break;
        case 0x02: {
            batom_cursor_copy(cursor,table->ip_flow[i].location_info.ipv6.src_addr,sizeof(table->ip_flow[i].location_info.ipv6.src_addr));
            batom_cursor_copy(cursor,table->ip_flow[i].location_info.ipv6.dst_addr,sizeof(table->ip_flow[i].location_info.ipv6.dst_addr));
            table->ip_flow[i].location_info.ipv6.dst_port = batom_cursor_uint16_be(cursor);
        }
        break;
        case 0x05:{
             table->ip_flow[i].location_info.url.URL_length = batom_cursor_byte(cursor);
             BDBG_ASSERT( sizeof(table->ip_flow[i].location_info.url.URL_byte) >= table->ip_flow[i].location_info.url.URL_length);
             batom_cursor_copy(cursor, table->ip_flow[i].location_info.url.URL_byte, table->ip_flow[i].location_info.url.URL_length);
             table->ip_flow[i].location_info.url.URL_byte[table->ip_flow[i].location_info.url.URL_length] = '\0';
        }
        break;
        default:
            BDBG_WRN(("%s unknown location type", __extension__ __FUNCTION__));
        }
        table->ip_flow[i].descriptor_loop_length = batom_cursor_uint16_be(cursor);
        BDBG_ASSERT( sizeof(table->ip_flow[i].descriptor) >= table->ip_flow[i].descriptor_loop_length);
        batom_cursor_copy(cursor,table->ip_flow[i].descriptor, table->ip_flow[i].descriptor_loop_length);
        table->ip_flow[i].descriptor[table->ip_flow[i].descriptor_loop_length] = '\0';
    }

    return 0;
}


int bmmt_print_pl_table(bmmt_pl_table *table)
{

    int i,j;
    printf("\n****** PL TABLE ************\n");
    printf("\n num_of_packages %u",table->num_of_packages);
    for (i=0;i<table->num_of_packages;i++ )
    {
        printf("\n\t package_id_length %u",table->packages[i].package_id_length);
        printf("\n\t package_id:");
        for(j=0;j<table->packages[i].package_id_length;j++)
        {
            printf("%u ", table->packages[i].package_id[j]);
        }

        printf("\n\t location_type %u",table->packages[i].location_info.location_type);
        switch (table->packages[i].location_info.location_type) {
        case bmmt_general_location_type_id:
            printf("\n\t\t packet ID %04x",table->packages[i].location_info.data.packet_id);

            break;
        case bmmt_general_location_type_ipv4:
            printf("\n\t\t packet ID %04x",table->packages[i].location_info.data.mmt_ipv4.packet_id);
            printf("\n\t\t dst port %04x",table->packages[i].location_info.data.mmt_ipv4.dst_port);
            printf("\n\t\t src ipv4 %02x.%02x.%02x.%02x",
                   table->packages[i].location_info.data.mmt_ipv4.ipv4_src_addr[3],
                   table->packages[i].location_info.data.mmt_ipv4.ipv4_src_addr[2],
                   table->packages[i].location_info.data.mmt_ipv4.ipv4_src_addr[1],
                   table->packages[i].location_info.data.mmt_ipv4.ipv4_src_addr[0]);
            printf("\n\t\t dst ipv4 %02x.%02x.%02x.%02x",
                   table->packages[i].location_info.data.mmt_ipv4.ipv4_dst_addr[3],
                   table->packages[i].location_info.data.mmt_ipv4.ipv4_dst_addr[2],
                   table->packages[i].location_info.data.mmt_ipv4.ipv4_dst_addr[1],
                   table->packages[i].location_info.data.mmt_ipv4.ipv4_dst_addr[0]);
            break;
        case bmmt_general_location_type_ipv6:
            printf("\n\t\t packet ID %04x",table->packages[i].location_info.data.mmt_ipv6.packet_id);
            printf("\n\t\t dst port %04x",table->packages[i].location_info.data.mmt_ipv6.dst_port);
            printf("\n\t\t src ipv6 %08x.%08x.%08x.%08x",
                   table->packages[i].location_info.data.mmt_ipv6.ipv6_src_addr[3] << 24 |
                   table->packages[i].location_info.data.mmt_ipv6.ipv6_src_addr[2] << 16 |
                   table->packages[i].location_info.data.mmt_ipv6.ipv6_src_addr[1] << 8 |
                   table->packages[i].location_info.data.mmt_ipv6.ipv6_src_addr[0],
                   table->packages[i].location_info.data.mmt_ipv6.ipv6_src_addr[7] << 24 |
                   table->packages[i].location_info.data.mmt_ipv6.ipv6_src_addr[6] << 16 |
                   table->packages[i].location_info.data.mmt_ipv6.ipv6_src_addr[5] << 8 |
                   table->packages[i].location_info.data.mmt_ipv6.ipv6_src_addr[4],
                   table->packages[i].location_info.data.mmt_ipv6.ipv6_src_addr[11] << 24 |
                   table->packages[i].location_info.data.mmt_ipv6.ipv6_src_addr[10] << 16 |
                   table->packages[i].location_info.data.mmt_ipv6.ipv6_src_addr[9] << 8 |
                   table->packages[i].location_info.data.mmt_ipv6.ipv6_src_addr[8],
                   table->packages[i].location_info.data.mmt_ipv6.ipv6_src_addr[15] << 24 |
                   table->packages[i].location_info.data.mmt_ipv6.ipv6_src_addr[14] << 16 |
                   table->packages[i].location_info.data.mmt_ipv6.ipv6_src_addr[13] << 8 |
                   table->packages[i].location_info.data.mmt_ipv6.ipv6_src_addr[12]);
            printf("\n\t\t dst ipv6 %08x.%08x.%08x.%08x",
                   table->packages[i].location_info.data.mmt_ipv6.ipv6_dst_addr[3] << 24 |
                   table->packages[i].location_info.data.mmt_ipv6.ipv6_dst_addr[2] << 16 |
                   table->packages[i].location_info.data.mmt_ipv6.ipv6_dst_addr[1] << 8 |
                   table->packages[i].location_info.data.mmt_ipv6.ipv6_dst_addr[0],
                   table->packages[i].location_info.data.mmt_ipv6.ipv6_dst_addr[7] << 24 |
                   table->packages[i].location_info.data.mmt_ipv6.ipv6_dst_addr[6] << 16 |
                   table->packages[i].location_info.data.mmt_ipv6.ipv6_dst_addr[5] << 8 |
                   table->packages[i].location_info.data.mmt_ipv6.ipv6_dst_addr[4],
                   table->packages[i].location_info.data.mmt_ipv6.ipv6_dst_addr[11] << 24 |
                   table->packages[i].location_info.data.mmt_ipv6.ipv6_dst_addr[10] << 16 |
                   table->packages[i].location_info.data.mmt_ipv6.ipv6_dst_addr[9] << 8 |
                   table->packages[i].location_info.data.mmt_ipv6.ipv6_dst_addr[8],
                   table->packages[i].location_info.data.mmt_ipv6.ipv6_dst_addr[15] << 24 |
                   table->packages[i].location_info.data.mmt_ipv6.ipv6_dst_addr[14] << 16 |
                   table->packages[i].location_info.data.mmt_ipv6.ipv6_dst_addr[13] << 8 |
                   table->packages[i].location_info.data.mmt_ipv6.ipv6_dst_addr[12]);


            break;
        case bmmt_general_location_type_mpeg2ts:
            printf("\n\t\t networkd ID %04x",table->packages[i].location_info.data.mpeg2ts.network_id);
            printf("\n\t\t ts ID %04x",table->packages[i].location_info.data.mpeg2ts.MPEG_2_transport_stream_id);
            printf("\n\t\t MPEG2 PID %04x",table->packages[i].location_info.data.mpeg2ts.MPEG_2_PID);
            break;
        case bmmt_general_location_type_mpeg2ts_ipv6:
            printf("\n\t\t MPEG2 PID %04x",table->packages[i].location_info.data.mpeg2ts_ipv6.MPEG_2_PID);
            printf("\n\t\t dst port %04x",table->packages[i].location_info.data.mmt_ipv6.dst_port);
            printf("\n\t\t src ipv4 %08x.%08x.%08x.%08x",
                   table->packages[i].location_info.data.mpeg2ts_ipv6.ipv6_src_addr[3] << 24 |
                   table->packages[i].location_info.data.mpeg2ts_ipv6.ipv6_src_addr[2] << 16 |
                   table->packages[i].location_info.data.mpeg2ts_ipv6.ipv6_src_addr[1] << 8 |
                   table->packages[i].location_info.data.mpeg2ts_ipv6.ipv6_src_addr[0],
                   table->packages[i].location_info.data.mpeg2ts_ipv6.ipv6_src_addr[7] << 24 |
                   table->packages[i].location_info.data.mpeg2ts_ipv6.ipv6_src_addr[6] << 16 |
                   table->packages[i].location_info.data.mpeg2ts_ipv6.ipv6_src_addr[5] << 8 |
                   table->packages[i].location_info.data.mpeg2ts_ipv6.ipv6_src_addr[4],
                   table->packages[i].location_info.data.mpeg2ts_ipv6.ipv6_src_addr[11] << 24 |
                   table->packages[i].location_info.data.mpeg2ts_ipv6.ipv6_src_addr[10] << 16 |
                   table->packages[i].location_info.data.mpeg2ts_ipv6.ipv6_src_addr[9] << 8 |
                   table->packages[i].location_info.data.mpeg2ts_ipv6.ipv6_src_addr[8],
                   table->packages[i].location_info.data.mpeg2ts_ipv6.ipv6_src_addr[15] << 24 |
                   table->packages[i].location_info.data.mpeg2ts_ipv6.ipv6_src_addr[14] << 16 |
                   table->packages[i].location_info.data.mpeg2ts_ipv6.ipv6_src_addr[13] << 8 |
                   table->packages[i].location_info.data.mpeg2ts_ipv6.ipv6_src_addr[12]);
            printf("\n\t\t dst ipv4 %08x.%08x.%08x.%08x",
                   table->packages[i].location_info.data.mpeg2ts_ipv6.ipv6_dst_addr[3] << 24 |
                   table->packages[i].location_info.data.mpeg2ts_ipv6.ipv6_dst_addr[2] << 16 |
                   table->packages[i].location_info.data.mpeg2ts_ipv6.ipv6_dst_addr[1] << 8 |
                   table->packages[i].location_info.data.mpeg2ts_ipv6.ipv6_dst_addr[0],
                   table->packages[i].location_info.data.mpeg2ts_ipv6.ipv6_dst_addr[7] << 24 |
                   table->packages[i].location_info.data.mpeg2ts_ipv6.ipv6_dst_addr[6] << 16 |
                   table->packages[i].location_info.data.mpeg2ts_ipv6.ipv6_dst_addr[5] << 8 |
                   table->packages[i].location_info.data.mpeg2ts_ipv6.ipv6_dst_addr[4],
                   table->packages[i].location_info.data.mpeg2ts_ipv6.ipv6_dst_addr[11] << 24 |
                   table->packages[i].location_info.data.mpeg2ts_ipv6.ipv6_dst_addr[10] << 16 |
                   table->packages[i].location_info.data.mpeg2ts_ipv6.ipv6_dst_addr[9] << 8 |
                   table->packages[i].location_info.data.mpeg2ts_ipv6.ipv6_dst_addr[8],
                   table->packages[i].location_info.data.mpeg2ts_ipv6.ipv6_dst_addr[15] << 24 |
                   table->packages[i].location_info.data.mpeg2ts_ipv6.ipv6_dst_addr[14] << 16 |
                   table->packages[i].location_info.data.mpeg2ts_ipv6.ipv6_dst_addr[13] << 8 |
                   table->packages[i].location_info.data.mpeg2ts_ipv6.ipv6_dst_addr[12]);

            break;
        case bmmt_general_location_type_url:
            printf("\n\t\t URL %s",table->packages[i].location_info.data.url.URL_byte);
            break;
        case bmmt_general_location_type_private:
            printf("\n \t \t bmmt_general_location_type_private");
        }

    }

    printf("\n num_of_ip_delivery %u",table->num_of_ip_delivery);
    for (i=0;i<table->num_of_ip_delivery;i++ )
    {
        printf("\n\t transport_file_id %u",table->ip_flow[i].transport_file_id);
        printf("\n\t location_type %u",table->ip_flow[i].location_type);
        switch (table->ip_flow[i].location_type)
        {
        case 0x01: {
            printf("\n\t\t dst port %04x",table->ip_flow[i].location_info.ipv4.dst_port);
            printf("\n\t\t src ipv4 %02x.%02x.%02x.%02x",
                   table->ip_flow[i].location_info.ipv4.src_addr[3],
                   table->ip_flow[i].location_info.ipv4.src_addr[2],
                   table->ip_flow[i].location_info.ipv4.src_addr[1],
                   table->ip_flow[i].location_info.ipv4.src_addr[0]);
            printf("\n\t\t dst ipv4 %02x.%02x.%02x.%02x",
                   table->ip_flow[i].location_info.ipv4.dst_addr[3],
                   table->ip_flow[i].location_info.ipv4.dst_addr[2],
                   table->ip_flow[i].location_info.ipv4.dst_addr[1],
                   table->ip_flow[i].location_info.ipv4.dst_addr[0]);
        }
        break;
        case 0x02: {
            printf("\n\t\t dst port %04x",table->ip_flow[i].location_info.ipv6.dst_port);
            printf("\n\t\t src ipv6 %08x.%08x.%08x.%08x",
                   table->ip_flow[i].location_info.ipv6.src_addr[3] << 24 |
                   table->ip_flow[i].location_info.ipv6.src_addr[2] << 16 |
                   table->ip_flow[i].location_info.ipv6.src_addr[1] << 8 |
                   table->ip_flow[i].location_info.ipv6.src_addr[0],
                   table->ip_flow[i].location_info.ipv6.src_addr[7] << 24 |
                   table->ip_flow[i].location_info.ipv6.src_addr[6] << 16 |
                   table->ip_flow[i].location_info.ipv6.src_addr[5] << 8 |
                   table->ip_flow[i].location_info.ipv6.src_addr[4],
                   table->ip_flow[i].location_info.ipv6.src_addr[11] << 24 |
                   table->ip_flow[i].location_info.ipv6.src_addr[10] << 16 |
                   table->ip_flow[i].location_info.ipv6.src_addr[9] << 8 |
                   table->ip_flow[i].location_info.ipv6.src_addr[8],
                   table->ip_flow[i].location_info.ipv6.src_addr[15] << 24 |
                   table->ip_flow[i].location_info.ipv6.src_addr[14] << 16 |
                   table->ip_flow[i].location_info.ipv6.src_addr[13] << 8 |
                   table->ip_flow[i].location_info.ipv6.src_addr[12]);
            printf("\n\t\t dst ipv6 %08x.%08x.%08x.%08x",
                   table->ip_flow[i].location_info.ipv6.dst_addr[3] << 24 |
                   table->ip_flow[i].location_info.ipv6.dst_addr[2] << 16 |
                   table->ip_flow[i].location_info.ipv6.dst_addr[1] << 8 |
                   table->ip_flow[i].location_info.ipv6.dst_addr[0],
                   table->ip_flow[i].location_info.ipv6.dst_addr[7] << 24 |
                   table->ip_flow[i].location_info.ipv6.dst_addr[6] << 16 |
                   table->ip_flow[i].location_info.ipv6.dst_addr[5] << 8 |
                   table->ip_flow[i].location_info.ipv6.dst_addr[4],
                   table->ip_flow[i].location_info.ipv6.dst_addr[11] << 24 |
                   table->ip_flow[i].location_info.ipv6.dst_addr[10] << 16 |
                   table->ip_flow[i].location_info.ipv6.dst_addr[9] << 8 |
                   table->ip_flow[i].location_info.ipv6.dst_addr[8],
                   table->ip_flow[i].location_info.ipv6.dst_addr[15] << 24 |
                   table->ip_flow[i].location_info.ipv6.dst_addr[14] << 16 |
                   table->ip_flow[i].location_info.ipv6.dst_addr[13] << 8 |
                   table->ip_flow[i].location_info.ipv6.dst_addr[12]);
        }
        break;
        case 0x05:{
            printf("\n\t\t URL %s",table->ip_flow[i].location_info.url.URL_byte);
        }
        break;
        default:
            BDBG_WRN(("%s unknown location type", __extension__ __FUNCTION__));
        }
        printf("\n\t descriptor %s",table->ip_flow[i].descriptor);
    }
    printf("\n****** PL TABLE ************\n");
    return 0;
}


int bmmt_parse_mp_table(batom_cursor *cursor,bmmt_mp_table *table)
{
    unsigned i,j;

    table->mpt_mode = batom_cursor_byte(cursor) & 0x3;
    table->package_id_length = batom_cursor_byte(cursor);
    batom_cursor_copy(cursor, table->package_id, table->package_id_length);
    table->package_id[table->package_id_length+1]='\0';

    table->descriptor_length = batom_cursor_uint16_be(cursor);
    BDBG_ASSERT(sizeof(table->descriptor) >= table->descriptor_length);
    batom_cursor_copy(cursor, table->descriptor, table->descriptor_length);
    table->descriptor[table->descriptor_length+1]='\0';

    table->num_of_assets = batom_cursor_byte(cursor);
    for (i=0;i<table->num_of_assets;i++)
    {
        table->assets[i].identifier_type = batom_cursor_byte(cursor);
        table->assets[i].id_scheme = batom_cursor_uint32_be(cursor);
        table->assets[i].id_length = batom_cursor_byte(cursor);
        batom_cursor_copy(cursor, table->assets[i].id, table->assets[i].id_length);
        table->assets[i].id[table->assets[i].id_length+1]='\0';

        batom_cursor_copy(cursor, table->assets[i].type,4);
        table->assets[i].type[4] = '\0';
        table->assets[i].clock_relation_flag = batom_cursor_byte(cursor) & 0x1;
        table->assets[i].location_count = batom_cursor_byte(cursor);

        for (j=0;j<table->assets[i].location_count;j++)
        {
            bmmt_parse_general_location_info(cursor,&table->assets[i].location_info[j]);
        }

        table->assets[i].descriptor_length = batom_cursor_uint16_be(cursor);
        batom_cursor_copy(cursor, table->assets[i].descriptor, table->assets[i].descriptor_length);
        table->assets[i].descriptor[table->assets[i].descriptor_length+1]='\0';
    }
    return 0;
}

int bmmt_print_mp_table(bmmt_mp_table *table)
{
    int i,j;
    printf("\n*********************MP TABLE ***************************\n");
    printf("\n mpt_mode %u",table->mpt_mode);
    printf("\n package_id_length %u",table->package_id_length);
    printf("\n package_id:");
    for(i=0;i<table->package_id_length;i++)
    {
        printf("%02x ", table->package_id[i]);
    }
    printf("\n table->num_of_assets %u \n",table->num_of_assets);

    for (i=0;i<table->num_of_assets;i++ )
    {
        printf("\n\t identifier type %u",table->assets[i].identifier_type);
        printf("\n\t identifer scheme %u",table->assets[i].id_scheme);

        printf("\n\t id_length %u",table->assets[i].id_length);
         printf("\n\t asset_id:");
        for(j=0;j<table->assets[j].id_length;j++)
        {
            printf("%02x ", table->assets[i].id[j]);
        }
        printf("\n\t asset type %s",table->assets[i].type);
        printf("\n\t location count %u",table->assets[i].location_count);
        for (j=0;j<table->assets[i].location_count;j++) {
            switch (table->assets[i].location_info[j].location_type) {
            case bmmt_general_location_type_id:
                printf("\n\t\t packet ID %04x",table->assets[j].location_info[j].data.packet_id);

                break;
            case bmmt_general_location_type_ipv4:
                printf("\n\t\t packet ID %04x",table->assets[i].location_info[j].data.mmt_ipv4.packet_id);
                printf("\n\t\t dst port %04x",table->assets[i].location_info[j].data.mmt_ipv4.dst_port);
                printf("\n\t\t src ipv4 %02x.%02x.%02x.%02x",
                       table->assets[i].location_info[j].data.mmt_ipv4.ipv4_src_addr[3],
                       table->assets[i].location_info[j].data.mmt_ipv4.ipv4_src_addr[2],
                       table->assets[i].location_info[j].data.mmt_ipv4.ipv4_src_addr[1],
                       table->assets[j].location_info[j].data.mmt_ipv4.ipv4_src_addr[0]);
                printf("\n\t\t dst ipv4 %02x.%02x.%02x.%02x",
                       table->assets[i].location_info[j].data.mmt_ipv4.ipv4_dst_addr[3],
                       table->assets[i].location_info[j].data.mmt_ipv4.ipv4_dst_addr[2],
                       table->assets[i].location_info[j].data.mmt_ipv4.ipv4_dst_addr[1],
                       table->assets[i].location_info[j].data.mmt_ipv4.ipv4_dst_addr[0]);
                break;
            case bmmt_general_location_type_ipv6:
                printf("\n\t\t packet ID %04x",table->assets[i].location_info[j].data.mmt_ipv6.packet_id);
                printf("\n\t\t dst port %04x",table->assets[i].location_info[j].data.mmt_ipv6.dst_port);
                printf("\n\t\t src ipv6 %08x.%08x.%08x.%08x",
                       table->assets[i].location_info[j].data.mmt_ipv6.ipv6_src_addr[3] << 24 |
                       table->assets[i].location_info[j].data.mmt_ipv6.ipv6_src_addr[2] << 16 |
                       table->assets[i].location_info[j].data.mmt_ipv6.ipv6_src_addr[1] << 8 |
                       table->assets[i].location_info[j].data.mmt_ipv6.ipv6_src_addr[0],
                       table->assets[i].location_info[j].data.mmt_ipv6.ipv6_src_addr[7] << 24 |
                       table->assets[i].location_info[j].data.mmt_ipv6.ipv6_src_addr[6] << 16 |
                       table->assets[i].location_info[j].data.mmt_ipv6.ipv6_src_addr[5] << 8 |
                       table->assets[i].location_info[j].data.mmt_ipv6.ipv6_src_addr[4],
                       table->assets[i].location_info[j].data.mmt_ipv6.ipv6_src_addr[11] << 24 |
                       table->assets[i].location_info[j].data.mmt_ipv6.ipv6_src_addr[10] << 16 |
                       table->assets[i].location_info[j].data.mmt_ipv6.ipv6_src_addr[9] << 8 |
                       table->assets[i].location_info[j].data.mmt_ipv6.ipv6_src_addr[8],
                       table->assets[i].location_info[j].data.mmt_ipv6.ipv6_src_addr[15] << 24 |
                       table->assets[i].location_info[j].data.mmt_ipv6.ipv6_src_addr[14] << 16 |
                       table->assets[i].location_info[j].data.mmt_ipv6.ipv6_src_addr[13] << 8 |
                       table->assets[i].location_info[j].data.mmt_ipv6.ipv6_src_addr[12]);
                printf("\n\t\t dst ipv6 %08x.%08x.%08x.%08x",
                       table->assets[i].location_info[j].data.mmt_ipv6.ipv6_dst_addr[3] << 24 |
                       table->assets[i].location_info[j].data.mmt_ipv6.ipv6_dst_addr[2] << 16 |
                       table->assets[i].location_info[j].data.mmt_ipv6.ipv6_dst_addr[1] << 8 |
                       table->assets[i].location_info[j].data.mmt_ipv6.ipv6_dst_addr[0],
                       table->assets[i].location_info[j].data.mmt_ipv6.ipv6_dst_addr[7] << 24 |
                       table->assets[i].location_info[j].data.mmt_ipv6.ipv6_dst_addr[6] << 16 |
                       table->assets[i].location_info[j].data.mmt_ipv6.ipv6_dst_addr[5] << 8 |
                       table->assets[i].location_info[j].data.mmt_ipv6.ipv6_dst_addr[4],
                       table->assets[i].location_info[j].data.mmt_ipv6.ipv6_dst_addr[11] << 24 |
                       table->assets[i].location_info[j].data.mmt_ipv6.ipv6_dst_addr[10] << 16 |
                       table->assets[i].location_info[j].data.mmt_ipv6.ipv6_dst_addr[9] << 8 |
                       table->assets[i].location_info[j].data.mmt_ipv6.ipv6_dst_addr[8],
                       table->assets[i].location_info[j].data.mmt_ipv6.ipv6_dst_addr[15] << 24 |
                       table->assets[i].location_info[j].data.mmt_ipv6.ipv6_dst_addr[14] << 16 |
                       table->assets[i].location_info[j].data.mmt_ipv6.ipv6_dst_addr[13] << 8 |
                       table->assets[i].location_info[j].data.mmt_ipv6.ipv6_dst_addr[12]);


                break;
            case bmmt_general_location_type_mpeg2ts:
                printf("\n\t\t networkd ID %04x",table->assets[i].location_info[j].data.mpeg2ts.network_id);
                printf("\n\t\t ts ID %04x",table->assets[i].location_info[j].data.mpeg2ts.MPEG_2_transport_stream_id);
                printf("\n\t\t MPEG2 PID %04x",table->assets[i].location_info[j].data.mpeg2ts.MPEG_2_PID);
                break;
            case bmmt_general_location_type_mpeg2ts_ipv6:
                printf("\n\t\t MPEG2 PID %04x",table->assets[i].location_info[j].data.mpeg2ts_ipv6.MPEG_2_PID);
                printf("\n\t\t dst port %04x",table->assets[i].location_info[j].data.mmt_ipv6.dst_port);
                printf("\n\t\t src ipv4 %08x.%08x.%08x.%08x",
                       table->assets[i].location_info[j].data.mpeg2ts_ipv6.ipv6_src_addr[3] << 24 |
                       table->assets[i].location_info[j].data.mpeg2ts_ipv6.ipv6_src_addr[2] << 16 |
                       table->assets[i].location_info[j].data.mpeg2ts_ipv6.ipv6_src_addr[1] << 8 |
                       table->assets[i].location_info[j].data.mpeg2ts_ipv6.ipv6_src_addr[0],
                       table->assets[i].location_info[j].data.mpeg2ts_ipv6.ipv6_src_addr[7] << 24 |
                       table->assets[i].location_info[j].data.mpeg2ts_ipv6.ipv6_src_addr[6] << 16 |
                       table->assets[i].location_info[j].data.mpeg2ts_ipv6.ipv6_src_addr[5] << 8 |
                       table->assets[i].location_info[j].data.mpeg2ts_ipv6.ipv6_src_addr[4],
                       table->assets[i].location_info[j].data.mpeg2ts_ipv6.ipv6_src_addr[11] << 24 |
                       table->assets[i].location_info[j].data.mpeg2ts_ipv6.ipv6_src_addr[10] << 16 |
                       table->assets[i].location_info[j].data.mpeg2ts_ipv6.ipv6_src_addr[9] << 8 |
                       table->assets[i].location_info[j].data.mpeg2ts_ipv6.ipv6_src_addr[8],
                       table->assets[i].location_info[j].data.mpeg2ts_ipv6.ipv6_src_addr[15] << 24 |
                       table->assets[i].location_info[j].data.mpeg2ts_ipv6.ipv6_src_addr[14] << 16 |
                       table->assets[i].location_info[j].data.mpeg2ts_ipv6.ipv6_src_addr[13] << 8 |
                       table->assets[i].location_info[j].data.mpeg2ts_ipv6.ipv6_src_addr[12]);
                printf("\n\t\t dst ipv4 %08x.%08x.%08x.%08x",
                       table->assets[i].location_info[j].data.mpeg2ts_ipv6.ipv6_dst_addr[3] << 24 |
                       table->assets[i].location_info[j].data.mpeg2ts_ipv6.ipv6_dst_addr[2] << 16 |
                       table->assets[i].location_info[j].data.mpeg2ts_ipv6.ipv6_dst_addr[1] << 8 |
                       table->assets[i].location_info[j].data.mpeg2ts_ipv6.ipv6_dst_addr[0],
                       table->assets[i].location_info[j].data.mpeg2ts_ipv6.ipv6_dst_addr[7] << 24 |
                       table->assets[i].location_info[j].data.mpeg2ts_ipv6.ipv6_dst_addr[6] << 16 |
                       table->assets[i].location_info[j].data.mpeg2ts_ipv6.ipv6_dst_addr[5] << 8 |
                       table->assets[i].location_info[j].data.mpeg2ts_ipv6.ipv6_dst_addr[4],
                       table->assets[i].location_info[j].data.mpeg2ts_ipv6.ipv6_dst_addr[11] << 24 |
                       table->assets[i].location_info[j].data.mpeg2ts_ipv6.ipv6_dst_addr[10] << 16 |
                       table->assets[i].location_info[j].data.mpeg2ts_ipv6.ipv6_dst_addr[9] << 8 |
                       table->assets[i].location_info[j].data.mpeg2ts_ipv6.ipv6_dst_addr[8],
                       table->assets[i].location_info[j].data.mpeg2ts_ipv6.ipv6_dst_addr[15] << 24 |
                       table->assets[i].location_info[j].data.mpeg2ts_ipv6.ipv6_dst_addr[14] << 16 |
                       table->assets[i].location_info[j].data.mpeg2ts_ipv6.ipv6_dst_addr[13] << 8 |
                       table->assets[i].location_info[j].data.mpeg2ts_ipv6.ipv6_dst_addr[12]);

                break;
            case bmmt_general_location_type_url:
                printf("\n\t\t URL %s",table->assets[i].location_info[j].data.url.URL_byte);
                break;
            case bmmt_general_location_type_private:
                printf("\n \t\t  bmmt_general_location_type_private");
            }
        }
    }
    printf("\n*********************MP TABLE ***************************\n");
    return 0;
}
