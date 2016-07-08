/***************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2007-2016 Broadcom. All rights reserved.
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
#ifndef NEXUS_SECURITY_TYPES_H__
#define NEXUS_SECURITY_TYPES_H__



#ifdef __cplusplus
extern "C" {
#endif


/**
Summary:
Security keyslot handle for conditional access and cryptography

Description:
See NEXUS_Security_AllocateKeySlot
**/
typedef struct NEXUS_KeySlot *NEXUS_KeySlotHandle;

/**
Summary:
This enum defines the crypto engines.

Description:
The security engine chosen when allocating a keyslot determines what the expected use of the
key will be.  Nexus tracks this information to validate key usage and for proper allocation and
de-allocation.
**/
typedef enum NEXUS_SecurityEngine
{
    NEXUS_SecurityEngine_eCa,
    NEXUS_SecurityEngine_eM2m,
    NEXUS_SecurityEngine_eCp,
    NEXUS_SecurityEngine_eCaCp,
    NEXUS_SecurityEngine_eRmx,
    NEXUS_SecurityEngine_eGeneric, /* Specifies that the key slot will be used only
                                      to pass a drm context to the playpump. */

    NEXUS_SecurityEngine_eMax
} NEXUS_SecurityEngine;

/**
Summary:
This enum defines the supported key slot types.

Description:
This selects the specific keyslot type when allocating a keyslot.

The default setting is NEXUS_SecurityKeySlotType_eAuto, which does not need to be changed.
When set to NEXUS_SecurityKeySlotType_eAuto, the Nexus security module will manage the actual
keyslot type when the keyslot is allocated via NEXUS_Security_AllocateKeySlot.
**/
typedef enum NEXUS_SecurityKeySlotType
{
    NEXUS_SecurityKeySlotType_eAuto,
    NEXUS_SecurityKeySlotType_eType0,
    NEXUS_SecurityKeySlotType_eType1,
    NEXUS_SecurityKeySlotType_eType2,
    NEXUS_SecurityKeySlotType_eType3,
    NEXUS_SecurityKeySlotType_eType4,
    NEXUS_SecurityKeySlotType_eType5,
    NEXUS_SecurityKeySlotType_eType6,
    NEXUS_SecurityKeySlotType_eType7,

    /* Add new key entry type definition before this line */
    NEXUS_SecurityKeySlotType_eMax
}   NEXUS_SecurityKeySlotType;

/* backward compat */
typedef unsigned NEXUS_SecurityKeySlotNumber;

/**
Summary:
This enum defines security client types for key slot and VKL allocations.
**/
typedef enum NEXUS_SecurityClientType {
    NEXUS_SecurityClientType_eHost,
    NEXUS_SecurityClientType_eSage,
    NEXUS_SecurityClientType_eMax
} NEXUS_SecurityClientType;

/**
Summary:
This enum defines key sources.
Enums are defined in pairs.  First one is for ASKM mode;  Second one is for regular mode.
**/
typedef enum NEXUS_SecurityKeySource
{
    NEXUS_SecurityKeySource_eReserved0,
    NEXUS_SecurityKeySource_eFirstRam = NEXUS_SecurityKeySource_eReserved0,
    NEXUS_SecurityKeySource_eReserved1,
    NEXUS_SecurityKeySource_eSecondRam = NEXUS_SecurityKeySource_eReserved1,
    NEXUS_SecurityKeySource_eReserved2,
    NEXUS_SecurityKeySource_eThirdRam = NEXUS_SecurityKeySource_eReserved2,
    NEXUS_SecurityKeySource_eKey3,
    NEXUS_SecurityKeySource_eKey3KeyLadder1 = NEXUS_SecurityKeySource_eKey3,
    NEXUS_SecurityKeySource_eKey4,
    NEXUS_SecurityKeySource_eKey4KeyLadder1 = NEXUS_SecurityKeySource_eKey4,
    NEXUS_SecurityKeySource_eKey5,
    NEXUS_SecurityKeySource_eKey5KeyLadder1 = NEXUS_SecurityKeySource_eKey5,
    NEXUS_SecurityKeySource_eReserved6,
    NEXUS_SecurityKeySource_eAvCPCW = NEXUS_SecurityKeySource_eReserved6,
    NEXUS_SecurityKeySource_eKey6 = NEXUS_SecurityKeySource_eReserved6,
    NEXUS_SecurityKeySource_eKey3KeyLadder2 = NEXUS_SecurityKeySource_eAvCPCW,
    NEXUS_SecurityKeySource_eReserved7,
    NEXUS_SecurityKeySource_eAvCW = NEXUS_SecurityKeySource_eReserved7,
    NEXUS_SecurityKeySource_eKey7 = NEXUS_SecurityKeySource_eReserved7,
    NEXUS_SecurityKeySource_eKey4KeyLadder2 = NEXUS_SecurityKeySource_eAvCW,
    NEXUS_SecurityKeySource_eFirstRamAskm,
    NEXUS_SecurityKeySource_eKey5KeyLadder2 = NEXUS_SecurityKeySource_eFirstRamAskm,
    NEXUS_SecurityKeySource_ePkl = NEXUS_SecurityKeySource_eKey5KeyLadder2,
    NEXUS_SecurityKeySource_eSecondRamAskm,
    NEXUS_SecurityKeySource_eKey3KeyLadder3 = NEXUS_SecurityKeySource_eSecondRamAskm,
    NEXUS_SecurityKeySource_eFirstRamAskm40nm = NEXUS_SecurityKeySource_eKey3KeyLadder3,
    NEXUS_SecurityKeySource_eThirdRamAskm,
    NEXUS_SecurityKeySource_eKey4KeyLadder3 = NEXUS_SecurityKeySource_eThirdRamAskm,
    NEXUS_SecurityKeySource_eFek = NEXUS_SecurityKeySource_eKey4KeyLadder3,
    NEXUS_SecurityKeySource_eFirstRam2048,
    NEXUS_SecurityKeySource_eKey5KeyLadder3 = NEXUS_SecurityKeySource_eFirstRam2048,
    NEXUS_SecurityKeySource_eSecondRam2048,
    NEXUS_SecurityKeySource_eThirdRam2048,
    NEXUS_SecurityKeySource_eFirstRNRam1024,
    NEXUS_SecurityKeySource_eSecondRNRam1024,
    NEXUS_SecurityKeySource_eMax
} NEXUS_SecurityKeySource;

/**
Summary:
**/
typedef struct NEXUS_SecurityKeySlotSettings {
    NEXUS_SecurityEngine keySlotEngine;
    NEXUS_SecurityKeySource keySlotSource;
    NEXUS_SecurityKeySlotType keySlotType;
    NEXUS_SecurityClientType client;
} NEXUS_SecurityKeySlotSettings;

/**
Summary:
Information from a keyslot returned by NEXUS_KeySlot_GetInfo
**/
typedef struct NEXUS_SecurityKeySlotInfo
{
    NEXUS_SecurityEngine        keySlotEngine;  /* Identifies the Crypto Engine (i.e., CA/CPS/CPD) that the keyslot is being used with. */
    unsigned                    keySlotNumber;  /* keySlotNumber and keySlotType uniquely identify a physical keyslot. */
    NEXUS_SecurityKeySlotType   keySlotType;
    struct {
        unsigned pidChannelIndex;               /* A pid channel is reserved by Nexus Security if keyslot is for a DMA transfer.
                                                   Valid only if keySlotEngine == NEXUS_SecurityEngine_eM2m.  */
    }dma;
} NEXUS_SecurityKeySlotInfo;

typedef NEXUS_SecurityKeySlotInfo NEXUS_KeySlotInfo;

/**
Summary:
Enumerator to identify a particular Bypass Keyslot.

Description:
All pid channels for clear data transfers will be associated with a bypass keyslot. By default the system
automatically configures pid channels that have not been explicitly assocaited with a keyslot to
the NEXUS_BypassKeySlot_eG2GR keyslot. The client can modify this to NEXUS_BypassKeySlot_eGR2R with the
function NEXUS_SetPidChannelBypassKeyslot.
Note: GLR (Global Region) can be accessed from the HOST processor. CRR (Compressed Restricted Region)
is a memory region that contains compressed media data and can't be accessed from the HOST processor.
**/
typedef enum NEXUS_BypassKeySlot{
    NEXUS_BypassKeySlot_eG2GR,              /* allows transfer of clear data from GLR to GLR or CRR */
    NEXUS_BypassKeySlot_eGR2R,              /* allows transfer of clear data from GLR or CRR to CRR */
    NEXUS_BypassKeySlot_eMax
} NEXUS_BypassKeySlot;

#ifdef __cplusplus
}
#endif

#endif
