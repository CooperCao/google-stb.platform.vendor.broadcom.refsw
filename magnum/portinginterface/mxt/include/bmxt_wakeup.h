/******************************************************************************
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
 ******************************************************************************/

#ifndef BMXT_WAKEUP_H__
#define BMXT_WAKEUP_H__

#include "bmxt.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
Summary:
Common settings for all 4 packet filters
****************************************************************************/
typedef struct
{
    unsigned InputBand;    /* input band to scan for wakeup packet */
    unsigned PacketLength; /* length of transport packet */
    bool ErrorInputIgnore; /* ignore error input signal and TEI bit */
}
BMXT_Wakeup_Settings;

void BMXT_Wakeup_GetSettings(
    BMXT_Handle handle,
    BMXT_Wakeup_Settings *pSettings /* [out] */
    );

BERR_Code BMXT_Wakeup_SetSettings(
    BMXT_Handle handle,
    const BMXT_Wakeup_Settings *pSettings
    );

/***************************************************************************
Summary:
Clear the wakeup interrupt to the Power Management Unit. This interrupt is
set when an incoming packet matches one of the filters. The interrupt remains
asserted until this API is called.
****************************************************************************/
void BMXT_Wakeup_ClearInterruptToPMU(
    BMXT_Handle handle
    );

/***************************************************************************
Summary:
Packet filtering config for a single byte of a transport packet
****************************************************************************/
typedef struct
{
    /* Bit pattern to compare the incoming packet against */
    unsigned CompareByte;

    /*
    Defines which bits in the CompareByte are used in the wakeup packet detection
    0 = Corresponding bit will be ignored and treated as always matching
    1 = Corresponding bit will be matched against compare bit value
    */
    unsigned Mask;

    /*
    Mask type defines how a series of bytes are matched.
    0 = Ignore byte. No byte comparison will be done
    1 = All bytes with mask-type of 1 must match, whether they are contiguous or not.
        Byte comparison will be done against 8-bit mask
    2 = All bytes with mask-type of 2 must match, but only within a contiguous series of 2's,
        in order for a partial match for that series to be generated
    3 = All bytes with mask-type of 3 must match, but only within a contiguous series of 3's,
        in order for a partial match for that series to be generated.
    */
    unsigned MaskType;
}
BMXT_Wakeup_PacketFilter;

/***************************************************************************
Summary:
Load the given filter config into hardware
****************************************************************************/
BERR_Code BMXT_Wakeup_SetPacketFilterBytes(
    BMXT_Handle handle,
    unsigned packetType,
    const BMXT_Wakeup_PacketFilter *pFilter
    );

/***************************************************************************
Summary:
Arm/disarm the wakeup block. When armed, the block will begin scanning the
input band data. When disarmed, no interrupts will be generated
****************************************************************************/
void BMXT_Wakeup_Enable(
    BMXT_Handle handle,
    bool enable
    );

#ifdef __cplusplus
}
#endif

#endif
