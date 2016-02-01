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

#ifndef TS_PACKET_H__
#define TS_PACKET_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    bool        transport_error_indicator;
    bool        payload_unit_start_indicator;
    bool        transport_priority;
    uint16_t    PID;
    uint8_t     transport_scambling_control;
    uint8_t     adaptation_field_control;
    uint8_t     continuity_counter;
    struct
    {
        bool        discontinuity_indicator;
        bool        random_access_indicator;
        bool        elementary_stream_priority_indicator;
        bool        PCR_flag;
            uint64_t    program_clock_reference_base;
            uint16_t    program_clock_reference_extension;
        bool        OPCR_flag;
            uint64_t    original_program_clock_reference_base;
            uint16_t    original_program_clock_reference_extension;
        bool        splicing_point_flag;
            uint8_t     splice_countdown;
        bool        transport_private_data_flag;
            uint8_t     transport_private_data_length;
            const uint8_t *p_private_data_byte;
        bool        adaptation_field_extension_flag;
            bool        ltw_flag;
                bool        ltw_valid_flag;
                uint16_t    ltw_offset;
            bool        piecewise_rate_flag;
                uint32_t    piecewise_rate;
            bool        seamless_splice_flag;
                uint8_t     splice_type;
                uint64_t    DTS_next_AU;
    } adaptation_field;
    const uint8_t *p_data_byte;
    uint8_t     data_size;
} TS_packet;

void TS_parseTsPacket( const uint8_t *buf, TS_packet *p_packet );

#ifdef __cplusplus
}
#endif
#endif
/* End of File */
