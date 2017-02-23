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

#ifndef NEXUS_PARSERBAND_CHANNELBONDING_H__
#define NEXUS_PARSERBAND_CHANNELBONDING_H__

#include "nexus_types.h"
#include "nexus_transport_capabilities.h"
#include "nexus_playpump.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct NEXUS_ParserBandStartBondingGroupSettings
{
    unsigned bondingPid; /* MPEG PID number for packets that will contain bonding metadata */

    struct {
        unsigned playpumpIndex; /* defaults to NEXUS_ANY_ID */
        NEXUS_Timebase timebase;
        unsigned pcrPid;
    } soft;

    NEXUS_ParserBand slave[NEXUS_MAX_PARSER_BANDS];
} NEXUS_ParserBandStartBondingGroupSettings;

/*
Summary:
Get default StartBondingGroupSettings
*/
void NEXUS_ParserBand_GetDefaultStartBondingGroupSettings(
    NEXUS_ParserBandStartBondingGroupSettings *pSettings
    );

/*
Summary:
Start a bonding group. Use status of master for the whole group
*/
NEXUS_Error NEXUS_ParserBand_StartBondingGroup(
    NEXUS_ParserBand master,
    const NEXUS_ParserBandStartBondingGroupSettings *pSettings
    );

/*
Summary:
Stop a bonding group
*/
void NEXUS_ParserBand_StopBondingGroup(
    NEXUS_ParserBand master
    );

/*
Summary:
Bonding group status
*/
typedef struct NEXUS_ParserBandBondingGroupStatus
{
    bool locked; /* bonding lock state */
} NEXUS_ParserBandBondingGroupStatus;

/*
Summary:
Get current bonding group status
*/
NEXUS_Error NEXUS_ParserBand_GetBondingGroupStatus(
    NEXUS_ParserBand master,
    NEXUS_ParserBandBondingGroupStatus *pStatus
    );

/**
Summary:
Get the internal playpump handle for soft-GCB

Description:
This API is subject to change.

This returns the bonding group's playpump handle that receives the re-sequentialized data.
The handle is only valid after start and becomes invalid after stop.
Only valid when used with soft-Generic Channel Bonding
**/
NEXUS_PlaypumpHandle NEXUS_ParserBand_GetBondingGroupPlaypump(
    NEXUS_ParserBand master
    );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef NEXUS_PARSERBAND_CHANNELBONDING_H__ */
