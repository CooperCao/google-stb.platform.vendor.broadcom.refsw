/***************************************************************************
 *     Copyright (c) 2003-2015, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 **************************************************************************/

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

void BMXT_Wakeup_ClearInterruptToPMU_isr(
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
