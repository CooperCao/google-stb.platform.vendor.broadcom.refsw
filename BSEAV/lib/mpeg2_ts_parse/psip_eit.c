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
#include "psip_priv.h"
#include "psip_eit.h"
BDBG_MODULE(psip_eit);

#define TITLE_LENGTH_BYTE_OFFSET        (byteOffset+9)
#define DESCRIPTOR_LENGTH_BYTE_OFFSET   (TITLE_LENGTH_BYTE_OFFSET+1+buf[TITLE_LENGTH_BYTE_OFFSET])

static int PSIP_EIT_P_getEventByteOffset( const uint8_t *buf, int eventNum )
{
    uint8_t i;
    int byteOffset = PSIP_TABLE_DATA_OFFSET + 1;

    if( eventNum == -1 )
    {
        eventNum = buf[PSIP_TABLE_DATA_OFFSET];
    }

    /* Jump to correct table (or first byte after last table) */
    for( i = 0; i < eventNum; i++ )
    {
        byteOffset += 12 + buf[TITLE_LENGTH_BYTE_OFFSET] + (TS_READ_16(&buf[DESCRIPTOR_LENGTH_BYTE_OFFSET]) & 0x0FFF);

        CHECK( byteOffset <= TS_PSI_MAX_BYTE_OFFSET(buf) );
    }

    return byteOffset;
}


uint8_t PSIP_EIT_getNumEvents( const uint8_t *buf )
{
    return buf[PSIP_TABLE_DATA_OFFSET];
}

BERR_Code PSIP_EIT_getEvent( const uint8_t *buf, int eventNum, PSIP_EIT_event *p_event )
{
    int byteOffset;

    if( eventNum >= buf[PSIP_TABLE_DATA_OFFSET] )
    {
        return BERR_INVALID_PARAMETER;
    }

    byteOffset = PSIP_EIT_P_getEventByteOffset( buf, eventNum );

    p_event->event_id = (uint16_t)(TS_READ_16( &buf[byteOffset] ) & 0x3FFF);
    p_event->start_time = TS_READ_32( &buf[byteOffset+2] );
    p_event->ETM_location = (buf[byteOffset+6]>>4)&3;
    p_event->length_in_seconds = (TS_READ_32(&buf[byteOffset+6])>>8)&0x000FFFFF;
    p_event->p_title_text = &buf[TITLE_LENGTH_BYTE_OFFSET+1];

    return BERR_SUCCESS;
}

TS_PSI_descriptor PSIP_EIT_getEventDescriptor( const uint8_t *buf, int eventNum, int descriptorNum )
{
    int byteOffset;

    if( eventNum >= buf[PSIP_TABLE_DATA_OFFSET] )
    {
        return NULL;
    }

    byteOffset = PSIP_EIT_P_getEventByteOffset( buf, eventNum );

    return TS_P_getDescriptor( &buf[DESCRIPTOR_LENGTH_BYTE_OFFSET+2], (TS_READ_16(&buf[DESCRIPTOR_LENGTH_BYTE_OFFSET]) & 0xFFF), descriptorNum );
}
