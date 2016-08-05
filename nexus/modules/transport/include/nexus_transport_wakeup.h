/***************************************************************************
 *  Copyright (C) 2007-2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *
 **************************************************************************/
#ifndef NEXUS_TRANSPORT_WAKEUP_H__
#define NEXUS_TRANSPORT_WAKEUP_H__

#include "nexus_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define NEXUS_WAKEUP_PACKET_SIZE 200   /* Each filter is 200 bytes. */
#define NEXUS_MAX_WAKEUP_PACKETS 4 /* Up to 4 packet types are supported (0 through 3) */

/** 
Summary: 
    Mask type defines how a series of bytes are matched.
    0 = Ignore byte. No byte comparison will be done.
    1 = All bytes with mask-type of 1 must match, whether they are contiguous or not.                                                                      .
        Byte comparison will be done against 8-bit mask.
    2 = All bytes with mask-type of 2 must match, but only within a contiguous series of 2's,                                                             .
        in order for a partial match for that series to be generated.
    3 = All bytes with mask-type of 3 must match, but only within a contiguous series of 3's,                                                             .
        in order for a partial match for that series to be generated. 
**/ 
typedef enum NEXUS_WakeupPacketType {
    NEXUS_WakeupPacketType_eIgnoreByte,
    NEXUS_WakeupPacketType_eCompareByte,
    NEXUS_WakeupPacketType_eCompareSeries2,
    NEXUS_WakeupPacketType_eCompareSeries3,
    NEXUS_WakeupPacketType_eMax        
} NEXUS_WakeupPacketType;

/**
Summary: 
This struct defines the packet filtering config for a single byte of a 
transport wakeup packet
**/
typedef struct NEXUS_TransportWakeupFilter
{
    /* Bit pattern to compare the incoming packet against.  */
    uint8_t comparePattern;

    /*
    Defines which bits in the CompareByte are used in the wakeup packet detection. 
    0 = Corresponding bit will be ignored and treated as always matching.
    1 = Corresponding bit will be matched against compare bit value.  
    */
    uint8_t compareMask;
    
    NEXUS_WakeupPacketType maskType;   
} NEXUS_TransportWakeupFilter;

/**
Summary:
Settings to configure a Transport Wakeup
**/
typedef struct NEXUS_TransportWakeupSettings {
    NEXUS_InputBand inputBand;    /* Defaults to NEXUS_InputBand_e0. */
    bool errorIgnore;             /* Wakeup ignores the error input signal and TEI bit. Defaults to false. */
    unsigned packetLength;          /* The length of the transport packet. Defaults to 188. Max is NEXUS_WAKEUP_PACKET_SIZE*/ 
    struct {    
        NEXUS_TransportWakeupFilter packet[NEXUS_WAKEUP_PACKET_SIZE];    
    } filter[NEXUS_MAX_WAKEUP_PACKETS];
    bool enabled;
} NEXUS_TransportWakeupSettings;

/**
Summary:
Get default settings for the structure.
**/
void NEXUS_TransportWakeup_GetSettings(    
    NEXUS_TransportWakeupSettings *pSettings /* [out] */
    );

/**
Set settings for transport packet wakeup
**/
NEXUS_Error NEXUS_TransportWakeup_SetSettings(
    const NEXUS_TransportWakeupSettings *pSettings 
    );

#ifdef __cplusplus
}
#endif

#endif
