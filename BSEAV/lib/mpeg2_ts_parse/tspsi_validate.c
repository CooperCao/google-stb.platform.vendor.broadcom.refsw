/******************************************************************************
 * (c) 2004-2014 Broadcom Corporation
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

#include "tspsi_validate.h"

/* taken from BSEAV/lib/mpeg2_ts_parse/ts_priv.h */
#define TS_PSI_TABLE_ID_OFFSET              0
#define TS_PSI_SECTION_LENGTH_OFFSET        1
#define TS_PSI_TABLE_ID_EXT_OFFSET          3
#define TS_PSI_CNI_OFFSET                   5
#define TS_PSI_SECTION_NUMBER_OFFSET        6
#define TS_PSI_LAST_SECTION_NUMBER_OFFSET   7

#define TS_READ_16( buf ) ((uint16_t)((buf)[0]<<8|(buf)[1]))
#define TS_PSI_GET_SECTION_LENGTH( buf )    (uint16_t)(TS_READ_16( &(buf)[TS_PSI_SECTION_LENGTH_OFFSET] ) & 0x0FFF)
#define TS_PSI_MAX_BYTE_OFFSET( buf )       (TS_PSI_GET_SECTION_LENGTH(buf) - 1)

bool TS_Filter_Compare(const uint8_t *msg, const uint8_t *inclMask, const uint8_t *exclMask,
    const uint8_t *coef, size_t filterSize )
{
    bool inclResult = true;
    bool exclResult = true;
    bool exclEnabled = false;
    size_t i;

    for( i = 0; i < filterSize; i++ )
    {
        if( (inclMask[i] | coef[i]) != (inclMask[i] | msg[i]) )
        {
            inclResult = false;
            break;
        }

        if( exclEnabled == false && exclMask[i] != 0xFF )
        {
            exclEnabled = true;
            exclResult = false;
        }

        if( (~exclMask[i] & coef[i]) != (~exclMask[i] & msg[i]) )
        {
            exclResult = true;
        }
    }

#if 0
/* for debug to make sure the logic is right */
    if( !(inclResult && exclResult) )
    {
        printf("Incl: ");
        for( i = 0; i < filterSize; i++ )
        {
            printf("%02x ", inclMask[i]);
        }
        printf("\nExcl: ");
        for( i = 0; i < filterSize; i++ )
        {
            printf("%02x ", exclMask[i]);
        }
        printf("\nCoef: ");
        for( i = 0; i < filterSize; i++ )
        {
            printf("%02x ", coef[i]);
        }
        printf("\n Msg: ");
        for( i = 0; i < filterSize; i++ )
        {
            printf("%02x ", msg[i]);
        }
        printf("\n");
    }
#endif

    return inclResult && exclResult;
}

int TS_Get_Message_Size(const uint8_t *msg)
{
    size_t message_size = TS_PSI_GET_SECTION_LENGTH(msg) + 3;
    /*
    The transport hardware automatically pads messages to be 32 bit aligned.
    Therefore, the message size must be increased to take this into account.
    */
    if( message_size%4 ) {
        message_size += 4-(message_size%4);
    }
    return message_size;
}

bool TS_Validate_Size( const uint8_t *msg, size_t size)
{
    size_t current_size = 0;
    while (current_size + TS_PSI_SECTION_LENGTH_OFFSET + 1 < size) {
        current_size += TS_Get_Message_Size(&msg[current_size]);
    }
    /* In the end, it must be exactly equal */
    return current_size == size;
}
