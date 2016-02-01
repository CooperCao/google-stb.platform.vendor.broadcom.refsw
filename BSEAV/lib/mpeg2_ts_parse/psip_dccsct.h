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

#ifndef PSIP_DCCSCT_H__
#define PSIP_DCCSCT_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    uint16_t    dccsct_type;
    uint8_t     updates_defined;
} PSIP_DCCSCT_header;

typedef enum
{
    PSIP_DCCSCT_reserved,
    PSIP_DCCSCT_new_genre_category,
    PSIP_DCCSCT_new_state,
    PSIP_DCCSCT_new_county
} PSIP_DCCSCT_update_type;

typedef struct
{
    uint8_t         genre_category_code;
    const uint8_t   *p_genre_category_name_text;
} PSIP_DCCSCT_genre;

typedef struct
{
    uint8_t         dcc_state_location_code;
    const uint8_t   *p_dcc_state_location_code_text;
} PSIP_DCCSCT_state;

typedef struct
{
    uint8_t         state_code;
    uint16_t        dcc_county_location_code;
    const uint8_t   *p_dcc_county_location_code_text;
} PSIP_DCCSCT_county;

typedef struct
{
    PSIP_DCCSCT_update_type update_type;
    union
    {
        PSIP_DCCSCT_genre   genre;
        PSIP_DCCSCT_state   state;
        PSIP_DCCSCT_county  county;
    } update;
} PSIP_DCCSCT_update;

void PSIP_DCCSCT_getHeader( const uint8_t *buf, PSIP_DCCSCT_header *p_header );
TS_PSI_descriptor PSIP_DCCSCT_getAdditionalDescriptor( const uint8_t *buf, int descriptorNum );

BERR_Code PSIP_DCCT_getUpdate( const uint8_t *buf, int updateNum, PSIP_DCCSCT_update *p_update );
TS_PSI_descriptor PSIP_DCCT_getUpdateDescriptor( const uint8_t *buf, int updateNum, int descriptorNum );

#ifdef __cplusplus
}
#endif
#endif
/* End of File */
