/******************************************************************************
 * (c) 2003-2014 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 *****************************************************************************/

#include "bstd.h"
#include "ts_packet.h"
#include "psip_priv.h"
BDBG_MODULE(ts_packet);

void TS_parseTsPacket( const uint8_t *buf, TS_packet *p_packet )
{
    CHECK( buf[0] == 0x47 );
    p_packet->transport_error_indicator = (buf[1]>>7)&1;
    p_packet->payload_unit_start_indicator = (buf[1]>>6)&1;
    p_packet->transport_priority = (buf[1]>>5)&1;
    p_packet->PID = (uint16_t)(TS_READ_16(&buf[1]) & 0x1FFF);
    p_packet->transport_scambling_control = (uint8_t)((buf[3]>>6)&3);
    p_packet->adaptation_field_control = (uint8_t)((buf[3]>>4)&3);
    p_packet->continuity_counter = (uint8_t)(buf[3]&0xF);

    if( p_packet->adaptation_field_control & 0x2 )
    {
        int byteOffset = 6;

        p_packet->adaptation_field.discontinuity_indicator              = (buf[5]>>7)&1;
        p_packet->adaptation_field.random_access_indicator              = (buf[5]>>6)&1;
        p_packet->adaptation_field.elementary_stream_priority_indicator = (buf[5]>>5)&1;
        p_packet->adaptation_field.PCR_flag                             = (buf[5]>>4)&1;
        p_packet->adaptation_field.OPCR_flag                                = (buf[5]>>3)&1;
        p_packet->adaptation_field.splicing_point_flag                  = (buf[5]>>2)&1;
        p_packet->adaptation_field.transport_private_data_flag          = (buf[5]>>1)&1;
        p_packet->adaptation_field.adaptation_field_extension_flag      = buf[5]&1;

        if( p_packet->adaptation_field.PCR_flag )
        {
            p_packet->adaptation_field.program_clock_reference_base = TS_READ_64( &buf[byteOffset] )>>31;
            p_packet->adaptation_field.program_clock_reference_extension = (uint16_t)(TS_READ_16( &buf[byteOffset+4] ) & 0x1FF);
            byteOffset += 6;
        }
        if( p_packet->adaptation_field.OPCR_flag )
        {
            p_packet->adaptation_field.original_program_clock_reference_base = TS_READ_64( &buf[byteOffset] )>>31;
            p_packet->adaptation_field.original_program_clock_reference_extension = (uint16_t)(TS_READ_16( &buf[byteOffset+4] ) & 0x1FF);
            byteOffset += 6;
        }
        if( p_packet->adaptation_field.splicing_point_flag )
        {
            p_packet->adaptation_field.splice_countdown = buf[byteOffset];
            byteOffset += 1;
        }
        if( p_packet->adaptation_field.transport_private_data_flag )
        {
            p_packet->adaptation_field.transport_private_data_length = buf[byteOffset];
            p_packet->adaptation_field.p_private_data_byte = &buf[byteOffset+1];
            byteOffset += 1 + p_packet->adaptation_field.transport_private_data_length;
        }
        if( p_packet->adaptation_field.adaptation_field_extension_flag )
        {
            byteOffset += 1;
            p_packet->adaptation_field.ltw_flag             = (buf[byteOffset]>>7)&1;
            p_packet->adaptation_field.piecewise_rate_flag  = (buf[byteOffset]>>6)&1;
            p_packet->adaptation_field.seamless_splice_flag = (buf[byteOffset]>>5)&1;

            byteOffset += 1;
            if( p_packet->adaptation_field.ltw_flag )
            {
                p_packet->adaptation_field.ltw_valid_flag = (buf[byteOffset]>>7)&1;
                p_packet->adaptation_field.ltw_offset = (uint16_t)(TS_READ_16( &buf[byteOffset+1] ) & 0x7FFF);
                byteOffset += 2;
            }
            if( p_packet->adaptation_field.piecewise_rate_flag )
            {
                p_packet->adaptation_field.piecewise_rate = (TS_READ_32( &buf[byteOffset] )>>8) & 0x3FFFFF;
                byteOffset += 3;
            }
            if( p_packet->adaptation_field.seamless_splice_flag )
            {
                p_packet->adaptation_field.splice_type = (uint8_t)((buf[byteOffset]>>4)&0xF);
//              p_packet->adaptation_field.DTS_next_AU = ;
                byteOffset += 5;
            }
        }
    }

    if(p_packet->adaptation_field_control & 0x3)
    {
        if( p_packet->adaptation_field_control & 0x2 )
        {
            p_packet->p_data_byte = &buf[5+buf[4]];
            p_packet->data_size = (uint8_t)(188-(5+buf[4]));
        }
        else
        {
            p_packet->p_data_byte = &buf[4];
            p_packet->data_size = 188-4;
        }
    }
    else
    {
        p_packet->p_data_byte = NULL;
        p_packet->data_size = 0;
    }

}
