/***************************************************************************
 * Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

/***************************************************************************
Overview: 
 
The wakeup module within XPT allows the chip to be brought out of a 
powerdown/standby state when a special "wakeup" transport packet is received 
at the XPT front-end. Two independent interrupts can be triggered when such 
a packet is captured: one interrupt to Power Mangement Unit and another to 
the host MIPS. 
 
Note that the interrupt to the PMU is a level type, and will stay asserted 
until the MIPS clears it. The interrupt is 
 
Almost any packet can serve as a "wakeup". The hardware implements a bit-wise 
filter, against which all incoming packets are compared. The filter allows 
for both individual bit-wise matches and for sequences of bits that must 
match. Up to 4 indepedent filters can used simulatenously. A status API 
allows the MIPS to determine which of the filters successfully matched. 
 
The filter matching is done completely in the XPT_WAKEUP hardware block, 
which allows the rest of XPT to be powered down. This also means that 
it's not necessary to allocate or configure a PID channel or parser. 
 
The type of wake-up packet to be detected is fully configurable by 
through the PI, according to the following settings:
-   Packet length - defines the number of bytes to be compared.
-   Compare byte - 8-bit value to compare against.
-   Compare mask - 8-bit mask to define which bits to compare.
-   Compare mask type - 2-bit value to define how to compare a
    series of bytes.

The compare mask is defined as:
-   0: bit will be ignored and treated as always matching.
-   1: bit will be matched against corresponding compare bit value.

The compare mask_type is defined as:
-   00b ("0"): ignore byte. No byte comparison will be done.
-   01b ("1"): byte must be matched. Byte comparison will be done
    against 8-bit mask.
-   10b ("2") and 11b ("3"):
-   All bytes within a contiguous series of 2s or 3s must match
    ("logical AND") in order for a partial match, for that series, 
    to be generated; 
-   Partial results from all independent "2" and "3" series are ORed
    together. The result is then ANDed with the result from matching 
    all the "1" bytes.

"2" and "3" mean the same thing. Two levels are used to identify 
consecutive independent series of 2s or 3s. Of course, a "0", "1" or 
the end of the packet can also indicate the end of a series of 2s 
or 3s.

Example of byte matching:
                    0 0 0 0 0 0 0 0 0 0 1 1 1 1 1 1 1 1 1 1 2 2 2
Array index:        0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2

Array content:      1 1 1 1 2 2 2 2 2 3 3 3 3 3 2 2 2 2 2 0 0 1 1

-   Bytes 0, 1, 2, 3, 21, 22 must all match (mask_type == 1 for each
    byte)
-   There are 3 series of 2s and 3s. 3 partial results are defined:
-   Bytes 4, 5, 6, 7, 8 are tested for a match (all must match 0 AND)
    partial result 1
-   Bytes 9, 10, 11, 12, 13 are tested for a match partial result 2
-   Bytes 14, 15, 16, 17, 18 are tested for a match partial result 3
    IF (partial_result1 == match OR partial_result2 == match OR 
    partial_result3 == match) THEN result (all 2s and 3s) = match
-   Bytes 19, 20 are ignored.
-   Final result: result(all 1s) and result(all 2s and 3s)
 
Sample Code: 
 
#include "bstd.h"
#include "bxpt_priv.h"
#include "bxpt_wakeup.h"
#include "bint.h"
#include "wakeup_tests.h"
#include "bchp_xpt_wakeup.h"
#include "bchp_int_id_xpt_wakeup.h"
#include "bkni.h"
#include "bkni_multi.h"

#include <stdio.h>

static void PacketFoundHostIsr( 
    void *Parm1,
    int Parm2
    )
{
    BSTD_UNUSED( Parm2 );
    BKNI_SetEvent( (BKNI_EventHandle) Parm1 );
}

static unsigned char CapturedPacket[ BXPT_WAKEUP_PACKET_SIZE ];

/ * 
This is the pattern in wakeup_packet.ts (streamer version of ib0.dat) 
Offset Bytes 
------ ----------------------------------------------- 
0002f0 47 12 34 15 12 34 03 46 66 4f 31 00 88 77 66 55
000300 44 33 ff ff ff ff ff ff ff ff ff ff ff ff ff ff
* /
static BXPT_Wakeup_PacketFilter Filter[ BXPT_WAKEUP_PACKET_SIZE ] = 
{
    { 0x47, 0xFF, 1 },      / * The packet header, including PID. * /
    { 0x12, 0xFF, 1 },
    { 0x34, 0xFF, 1 },
    { 0x15, 0xFF, 1 },      

    { 0x12, 0xFF, 2 },
    { 0x34, 0xFF, 2 },
    { 0x03, 0xFF, 2 },
    { 0x46, 0xFF, 2 },
    { 0x66, 0xFF, 2 },

    { 0x4F, 0xFF, 3 },
    { 0x31, 0xFF, 3 },
    { 0x00, 0xFF, 3 },

    { 0x88, 0xFF, 2 },
    { 0x77, 0xFF, 2 },
    { 0x66, 0xFF, 2 },
    { 0x55, 0xFF, 2 },
    { 0x44, 0xFF, 2 },
    { 0x33, 0xFF, 2 }
};

void WakeupTest( 
    BXPT_Handle hXpt 
    )
{
    BXPT_Wakeup_Settings Settings; 
    BINT_CallbackHandle hPacketFoundHostIsrCb;
    BKNI_EventHandle hPacketFoundEvent;
    BXPT_Wakeup_Status Status;

    BKNI_CreateEvent( &hPacketFoundEvent );

    / * Host will need a callback to clear the interrupt to the PMU. * /
    BINT_CreateCallback( 
        &hPacketFoundHostIsrCb, 
        hXpt->hInt, 
        BCHP_INT_ID_PKT_DETECT,
        PacketFoundHostIsr, 
        ( void * ) hPacketFoundEvent, 
        0 );
    BINT_EnableCallback( hPacketFoundHostIsrCb );
     
    / * Set the filter criterion. Should be done before calling
    BXPT_Wakeup_SetSettings() * /
    BXPT_Wakeup_SetPacketFilterBytes( hXpt, 0, Filter );

    / *
    The interrupt to the PMU is a level type, and will stay asserted until the 
    source in XPT is cleared. This done by a call to BXPT_Wakeup_ClearInterruptToPMU(). 
    For that reason, the host interrupt will normally be enabled. 
    * /
    BXPT_Wakeup_GetDefaults( &Settings ); 
    Settings.InputBand = 5;                 / * 97425 SV board uses IB5 * /
    BXPT_Wakeup_SetSettings( hXpt, &Settings ); 
    BXPT_Wakeup_Armed( hXpt, true );

    BKNI_WaitForEvent( hPacketFoundEvent, BKNI_INFINITE );

    BXPT_Wakeup_Armed( hXpt, false );
    BXPT_Wakeup_GetStatus( hXpt, &Status );
    printf( "Status: Found %s, Type %u, Count %u\n", Status.Found ? "TRUE" : "FALSE", Status.Type, Status.Count );

    BXPT_Wakeup_ClearInterruptToPMU( hXpt );
    BXPT_Wakeup_GetCapturedPacket( hXpt, CapturedPacket );
    printf( "Captured packet: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\n", 
            CapturedPacket[ 0 ], CapturedPacket[ 1 ], CapturedPacket[ 2 ], CapturedPacket[ 3 ],
            CapturedPacket[ 4 ], CapturedPacket[ 5 ], CapturedPacket[ 6 ], CapturedPacket[ 7 ],
            CapturedPacket[ 8 ], CapturedPacket[ 9 ], CapturedPacket[ 10 ], CapturedPacket[ 11 ]
            );

    BINT_DisableCallback( hPacketFoundHostIsrCb );
    BINT_DestroyCallback( hPacketFoundHostIsrCb );
    BKNI_DestroyEvent( hPacketFoundEvent );
}
 
***************************************************************************/

#ifndef BXPT_WAKEUP_H__
#define BXPT_WAKEUP_H__

#include "bxpt.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
Summary: 
Miscellaneous constants used by this module.  
****************************************************************************/
#define BXPT_WAKEUP_PACKET_SIZE         (200)   /* Each filter is 200 bytes. */
#define BXPT_WAKEUP_MAX_PACKET_TYPE     (3)     /* Up to 4 packet types are supported (0 through 3) */

/***************************************************************************
Summary: 
These settings affect all 4 of the packet filters. The default values given 
are returned by BXPT_Wakeup_GetDefaults()
****************************************************************************/
typedef struct 
{
    /* Which input band to scan for the wakeup packet. */
    unsigned InputBand;         /* Defaults to 0. */

    /* The length of the transport packet. */
    unsigned PacketLength;      /* Defaults to 188. Max is BXPT_WAKEUP_PACKET_SIZE.*/

    /* Wakeip ignores the error input signal and TEI bit */
    bool ErrorInputIgnore;      /* Defaults to false. */
}
BXPT_Wakeup_Settings;

/***************************************************************************
Summary:
Returns the default values, as described in the BXPT_Wakeup_Settings comment 
block. 
 
Returns:
    void
****************************************************************************/
void BXPT_Wakeup_GetDefaults(
    BXPT_Wakeup_Settings *Settings  /* [out] The defaults */
    );

#if (!B_REFSW_MINIMAL)
/***************************************************************************
Summary:
Returns the actual settings currently used by the hardware. 
 
Returns:
    void
****************************************************************************/
void BXPT_Wakeup_GetSettings(
    BXPT_Handle hXpt,                   /* [in] Handle for this transport */
    BXPT_Wakeup_Settings *Settings      /* [out] Current hardware values */
    );
#endif

/***************************************************************************
Summary:
Write the given settings out to the hardware. 
 
Returns:
    BERR_SUCCESS                - New settings are being used.
    BERR_INVALID_PARAMETER      - One of the Settings values is invalid.
****************************************************************************/
BERR_Code BXPT_Wakeup_SetSettings(
    BXPT_Handle hXpt,               /* [in] Handle for this transport */
    const BXPT_Wakeup_Settings *Settings
    );

/***************************************************************************
Summary: 
Status of the wakeup block. Note that the Type field should be ignored if 
Found is false. Found will be reset to false when the wakeup block is 
re-armed. 
 
Note that calling BXPT_Wakeup_ClearInterruptToPMU() will also clear Found.  
****************************************************************************/
typedef struct 
{
    bool Found;         /* true if a wakeup packet was received. */
    unsigned Type;      /* Which packet type filter matched */
    unsigned Count;     /* How many packets that have been scanned. */
}
BXPT_Wakeup_Status;

#if (!B_REFSW_MINIMAL)
/***************************************************************************
Summary:
Return the current status values from hardware.  
 
Returns:
    void
****************************************************************************/
void BXPT_Wakeup_GetStatus(
    BXPT_Handle hXpt,               /* [in] Handle for this transport */
    BXPT_Wakeup_Status *Status
    );
#endif

/***************************************************************************
Summary:
Clear the wakeup interrupt to the Power Management Unit. This interrupt is 
set when an incoming packet matches one of the filters. The interrupt remains 
asserted until this API is called.  
 
Note that calling BXPT_Wakeup_ClearInterruptToPMU() will also clear the 
Found bool in the status register and BXPT_Wakeup_GetStatus() structure.
 
Returns:
    void
****************************************************************************/
void BXPT_Wakeup_ClearInterruptToPMU(
    BXPT_Handle hXpt                /* [in] Handle for this transport */
    );

/***************************************************************************
Summary:
ISR version of BXPT_Wakeup_ClearInterruptToPMU
****************************************************************************/
void BXPT_Wakeup_ClearInterruptToPMU_isr(
    BXPT_Handle hXpt                /* [in] Handle for this transport */
    );

/***************************************************************************
Summary: 
This struct defines the packet filtering config for a single byte of a 
transport packet. The full filter is an array of (up to) BXPT_WAKEUP_PACKET_SIZE 
of these structs. See the discussion and example in the module Overview 
(above) for more information on how to use create a filter config.  
****************************************************************************/
typedef struct 
{
    /* Bit pattern to compare the incoming packet against.  */
    unsigned CompareByte;

    /*
    Defines which bits in the CompareByte are used in the wakeup packet detection. 
    0 = Corresponding bit will be ignored and treated as always matching.
    1 = Corresponding bit will be matched against compare bit value.  
    */
    unsigned Mask;

    /* 
    Mask type defines how a series of bytes are matched.
    0 = Ignore byte. No byte comparison will be done.
    1 = All bytes with mask-type of 1 must match, whether they are contiguous or not.                                                                      .
        Byte comparison will be done against 8-bit mask.
    2 = All bytes with mask-type of 2 must match, but only within a contiguous series of 2's,                                                             .
        in order for a partial match for that series to be generated.
    3 = All bytes with mask-type of 3 must match, but only within a contiguous series of 3's,                                                             .
        in order for a partial match for that series to be generated. 
    */ 
    unsigned MaskType;   
}
BXPT_Wakeup_PacketFilter;

/***************************************************************************
Summary: 
Copy the current filter config to user memory. The caller must allocate the 
memory into which the config will be copied. For example, 
 
BXPT_Wakeup_PacketFilter Filter[ BXPT_WAKEUP_PACKET_SIZE ]; 
 
Returns:
    BERR_SUCCESS                - Filter config copied to user memory.
    BERR_INVALID_PARAMETER      - WhichPacketType is out-of-range.
****************************************************************************/
BERR_Code BXPT_Wakeup_GetPacketFilterBytes(
    BXPT_Handle hXpt,               /* [in] Handle for this transport */
    unsigned WhichPacketType,
    BXPT_Wakeup_PacketFilter *Filter
    );

/***************************************************************************
Summary:
Load the given filter config into hardware. The change will take effect 
immediately. 
 
Returns:
    BERR_SUCCESS                - New filter config is being used.
    BERR_INVALID_PARAMETER      - WhichPacketType is out-of-range.
****************************************************************************/
BERR_Code BXPT_Wakeup_SetPacketFilterBytes(
    BXPT_Handle hXpt,               /* [in] Handle for this transport */
    unsigned WhichPacketType,
    const BXPT_Wakeup_PacketFilter *Filter
    );

/***************************************************************************
Summary:
Copy out the packet that matched the wakeup filter. The caller must allocate 
memory to receive the packet. For example, 
  
unsigned char CapturedPacket[ BXPT_WAKEUP_PACKET_SIZE ]; 
 
Returns:
    void
****************************************************************************/
void BXPT_Wakeup_GetCapturedPacket(
    BXPT_Handle hXpt,               /* [in] Handle for this transport */
    unsigned char *PacketBuffer 
    );

/***************************************************************************
Summary:
Arm/disarm the wakeup block. When armed, the block will begin scanning the 
input band data. When disarmed, no interrupts to the PMU or MIPS will be 
generated if wakeup packets are seen in the input. 
 
Returns:
    void
****************************************************************************/
void BXPT_Wakeup_Armed(
    BXPT_Handle hXpt,       /* [in] Handle for this transport */
    bool Armed              /* [in] true if wakeup should be enabled, false otherwise. */
    );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BXPT_WAKEUP_H__ */

/* end of file */
