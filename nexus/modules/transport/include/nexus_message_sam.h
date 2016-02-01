/***************************************************************************
 *     (c)2007-2011 Broadcom Corporation
 *
 *  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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

#ifndef NEXUS_MESSAGE_SAM_H__
#define NEXUS_MESSAGE_SAM_H__

#include "nexus_types.h"
#include "nexus_message.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
Summary:
Size of SAM addresses in bytes
*/
#define NEXUS_MESSAGE_SAM_M40_ADDR_SIZE 5
#define NEXUS_MESSAGE_SAM_M24_ADDR_SIZE 3
#define NEXUS_MESSAGE_SAM_M16_ADDR_SIZE 2

/* 
Summary:
Multicast 40 address filters and wildcard configuration 
*/
typedef struct NEXUS_MessageSamM40Address
{
    bool wildcardByteMode;
    bool wildcardEnabled;
    uint8_t addressWildcard;
    uint8_t address[NEXUS_MESSAGE_SAM_M40_ADDR_SIZE];
} NEXUS_MessageSamM40Address;

/* 
Summary: 
SAM address configuration

Description:
This configuration is global across the platform
*/
typedef struct NEXUS_MessageSamAddresses
{
    uint8_t physicalAddress[NEXUS_MESSAGE_SAM_M40_ADDR_SIZE]; /* Singlecast or physical address */
    NEXUS_MessageSamM40Address type0NetworkAddress;           /* Type 0 network address */
    NEXUS_MessageSamM40Address type1NetworkAddress;           /* Type 1 network address */
    NEXUS_MessageSamM40Address type0M40Address;               /* Type 0 multicast 40 address */
    NEXUS_MessageSamM40Address type1M40Address;               /* Type 1 multicast 40 address */
    uint8_t m24Address[NEXUS_MESSAGE_SAM_M24_ADDR_SIZE];      /* Multicast 24 address */

    /* Upper and lower limits when filtering by Table ID */
    uint8_t idRejectHighLimit;
    uint8_t idRejectLowLimit;
} NEXUS_MessageSamAddresses;

/* 
Summary:
Get current SAM address configuration

Description:
This configuration is global across the platform
*/
void NEXUS_Message_GetSamAddresses(
    NEXUS_MessageSamAddresses *pAddresses /* [out] */
    );

/* 
Summary:
Set new SAM address configuration

Description:
This configuration is global across the platform
*/
NEXUS_Error NEXUS_Message_SetSamAddresses(
    const NEXUS_MessageSamAddresses *pAddresses
    );

/* 
Summary:
Multicast 16 address filters
*/
typedef struct NEXUS_MessageSamM16Address
{
    uint8_t address0[NEXUS_MESSAGE_SAM_M16_ADDR_SIZE];
    uint8_t address1[NEXUS_MESSAGE_SAM_M16_ADDR_SIZE];
    uint8_t address2[NEXUS_MESSAGE_SAM_M16_ADDR_SIZE];
    uint8_t address3[NEXUS_MESSAGE_SAM_M16_ADDR_SIZE];
} NEXUS_MessageSamM16Address;

/* 
Summary:
Get current M16 settings

Description:
M16 settings are specific to a message stream.
*/
void NEXUS_Message_GetSamM16Address(
    NEXUS_MessageHandle handle,
    NEXUS_MessageSamM16Address *pAddress /* [out] */
    );

/* 
Summary:
Set new M16 settings

Description:
M16 settings are specific to a message stream.
*/
NEXUS_Error NEXUS_Message_SetSamM16Address(
    NEXUS_MessageHandle handle,
    const NEXUS_MessageSamM16Address *pAddress
    );

/* 
Summary:
SAM settings
*/
typedef struct NEXUS_MessageSamSettings
{
    bool physicalEnabled;
    bool networkEnabled;
    bool m40Enabled;
    bool m24Enabled;
    bool m16Enabled;
    bool idRejectEnabled;
    bool specialNotEnabled;
} NEXUS_MessageSamSettings;

/* 
Summary:
Get default SAM settings
*/
void NEXUS_Message_GetDefaultSamSettings(
    NEXUS_MessageSamSettings *pSettings /* out */
    );

/* 
Summary:
Set new SAM settings

Description:
Enable or disable SAM filter settings. Note that this should be done before calling NEXUS_Message_Start() .
*/
NEXUS_Error NEXUS_Message_SetSamSettings(
    NEXUS_MessageHandle handle,
    const NEXUS_MessageSamSettings *pSettings
    );

#ifdef __cplusplus
}
#endif

#endif
