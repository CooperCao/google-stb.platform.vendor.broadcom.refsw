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

#ifndef PSIP_DCCT_H__
#define PSIP_DCCT_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    uint8_t     dcc_subtype;
    uint8_t     dcc_id;
    uint8_t     dcc_test_count;
} PSIP_DCCT_header;

typedef enum
{
    PSIP_DCCT_temporary_retune,
    PSIP_DCCT_channel_redirect
} PSIP_DCCT_context;

typedef struct
{
    PSIP_DCCT_context   dcc_context;
    uint16_t            dcc_from_major_channel_number;
    uint16_t            dcc_from_minor_channel_number;
    uint16_t            dcc_to_major_channel_number;
    uint16_t            dcc_to_minor_channel_number;
    uint32_t            dcc_start_time;
    uint32_t            dcc_end_time;
    uint32_t            dcc_term_count;
} PSIP_DCCT_test;

typedef enum
{
    PSIP_DCCT_all,
    PSIP_DCCT_north_west,
    PSIP_DCCT_north_central,
    PSIP_DCCT_north_east,
    PSIP_DCCT_west_central,
    PSIP_DCCT_central,
    PSIP_DCCT_east_central,
    PSIP_DCCT_south_west,
    PSIP_DCCT_south_central,
    PSIP_DCCT_south_east
} PSIP_DCCT_county_subdivision;

typedef struct
{
    uint8_t     dcc_selection_type;
    uint64_t    dcc_selection_id;
} PSIP_DCCT_term;

void PSIP_DCCT_getHeader( const uint8_t *buf, PSIP_DCCT_header *p_header );
TS_PSI_descriptor PSIP_DCCT_getAdditionalDescriptor( const uint8_t *buf, int descriptorNum );

BERR_Code PSIP_DCCT_getTest( const uint8_t *buf, int testNum, PSIP_DCCT_test *p_test );
TS_PSI_descriptor PSIP_DCCT_getTestDescriptor( const uint8_t *buf, int testNum, int descriptorNum );

BERR_Code PSIP_DCCT_getTerm( const uint8_t *buf, int testNum, int termNum, PSIP_DCCT_term *p_term );
TS_PSI_descriptor PSIP_DCCT_getTermDescriptor( const uint8_t *buf, int testNum, int termNum, int descriptorNum );

#ifdef __cplusplus
}
#endif
#endif
/* End of File */
