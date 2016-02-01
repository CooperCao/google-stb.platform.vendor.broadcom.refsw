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

#ifndef PSIP_VCT_H__
#define PSIP_VCT_H__

#include "psip_common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    PSIP_VCT_analog,
    PSIP_VCT_SCTE_mode_1,
    PSIP_VCT_SCTE_mode_2,
    PSIP_VCT_ATSC_8,
    PSIP_VCT_ATSC_16
} PSIP_VCT_modulation_mode;

typedef enum
{
    PSIP_VCT_analog_television = 1,
    PSIP_VCT_ATSC_digital_television,
    PSIP_VCT_ATSC_audio,
    PSIP_VCT_ATSC_data_only_service
} PSIP_VCT_service_type;

typedef struct
{
    uint16_t                    short_name[7];
    uint16_t                    major_channel_number;
    uint16_t                    minor_channel_number;
    PSIP_VCT_modulation_mode    modulation_mode;
    uint32_t                    carrier_frequency;
    uint16_t                    channel_TSID;
    uint16_t                    program_number;
    PSIP_ETM_location           ETM_location;
    uint16_t                    access_controlled;
    bool                        hidden;
    bool                        path_select;    /* valid for cable VCT only */
    bool                        out_of_band;    /* valid for cable VCT only */
    bool                        hide_guide;
    PSIP_VCT_service_type       service_type;
    uint16_t                    source_id;
} PSIP_VCT_channel;

uint8_t PSIP_VCT_getNumChannels( const uint8_t *buf );
TS_PSI_descriptor PSIP_VCT_getAdditionalDescriptor( const uint8_t *buf, int descriptorNum );

BERR_Code PSIP_VCT_getChannel( const uint8_t *buf, int channelNum, PSIP_VCT_channel *p_channel );
TS_PSI_descriptor PSIP_VCT_getChannelDescriptor( const uint8_t *buf, int channelNum, int descriptorNum );

#ifdef __cplusplus
}
#endif
#endif
/* End of File */
