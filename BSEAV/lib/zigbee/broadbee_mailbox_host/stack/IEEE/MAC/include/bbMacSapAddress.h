/******************************************************************************
* (c) 2014 Broadcom Corporation
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
******************************************************************************/
/*****************************************************************************
*
* FILENAME: $Workfile: trunk/stack/IEEE/MAC/include/bbMacSapAddress.h $
*
* DESCRIPTION:
*   MAC-SAP addressing definitions.
*
* $Revision: 2952 $
* $Date: 2014-07-16 17:08:40Z $
*
*****************************************************************************************/


#ifndef _BB_MAC_SAP_ADDRESS_H
#define _BB_MAC_SAP_ADDRESS_H


/************************* INCLUDES *****************************************************/
#include "bbMacBasics.h"            /* Basic MAC set. */


/************************* DEFINITIONS **************************************************/
/*
 * Reserved 16-bit short addresses:
 * - 0xFFFF - unassigned short address / broadcast short address,
 * - 0xFFFE - direction to use 64-bit IEEE address in all communications,
 * - if (address < 0xFFFE) then short address is valid.
 */
#define MAC_RESERVED_SHORT_ADDRESS_FFFF     0xFFFF
#define MAC_RESERVED_SHORT_ADDRESS_FFFE     0xFFFE
#define MAC_DONT_USE_SHORT_ADDRESS          MAC_RESERVED_SHORT_ADDRESS_FFFE
#define MAC_RESERVED_SHORT_ADDRESSES        MAC_RESERVED_SHORT_ADDRESS_FFFE


/*
 * Reserved 64-bit IEEE address:
 * - 0x0000'0000'0000'0000 - invalid 64-bit address.
 */
#define MAC_RESERVED_EXTENDED_ADDRESS       0x0000000000000000ull


/*
 * Broadcast MAC addresses:
 * - 0xFFFF - broadcast PAN ID,
 * - 0xFFFF - broadcast short address.
 */
#define MAC_BROADCAST_PAN_ID                MAC_RESERVED_SHORT_ADDRESS_FFFF
#define MAC_BROADCAST_SHORT_ADDRESS         MAC_RESERVED_SHORT_ADDRESS_FFFF


/*
 * Unassigned (default) addresses:
 */
#define MAC_UNASSIGNED_PAN_ID               MAC_RESERVED_SHORT_ADDRESS_FFFF
#define MAC_UNASSIGNED_SHORT_ADDRESS        MAC_RESERVED_SHORT_ADDRESS_FFFF
#define MAC_UNASSIGNED_EXTENDED_ADDRESS     MAC_RESERVED_EXTENDED_ADDRESS


/**//**
 * \brief MAC addressing modes enumeration.
 */
typedef enum _MAC_AddrMode_t
{
    MAC_NO_ADDRESS         = 0x00,      /*!< No address (addressing fields omitted). */
    MAC_RESERVED_ADDR_MODE = 0x01,      /*!< Reserved addressing mode. */
    MAC_16BIT_ADDRESS      = 0x02,      /*!< 16-bit short address. */
    MAC_64BIT_ADDRESS      = 0x03,      /*!< 64-bit extended address. */
} MAC_AddrMode_t;


/**//**
 * \brief MAC 16-bit short address data type.
 */
typedef uint16_t  MAC_Addr16bit_t;


/**//**
 * \brief MAC 64-bit extended address data type.
 */
typedef uint64_t  MAC_Addr64bit_t;


/**//**
 * \brief MAC device address data type.
 */
typedef union _MAC_Address_t
{
    uint64_t         plain;         /*!< Plain 64-bit value. */
    MAC_Addr64bit_t  addr64bit;     /*!< 64-bit Extended Address. */
    MAC_Addr16bit_t  addr16bit;     /*!< 16-bit Short Address. */
} MAC_Address_t;


/**//**
 * \brief MAC PAN Identifier data type.
 */
typedef uint16_t  MAC_PanId_t;


/************************* PROTOTYPES ***************************************************/
/*************************************************************************************//**
  \brief Returns the context-specific 64-bit (IEEE) address assigned to this device.
  \param __givenContextId Identifier of the specified context (for dual-context build).
  \return The 64-bit (IEEE) address assigned to this device within the specified context.
*****************************************************************************************/
MAC_PUBLIC MAC_Addr64bit_t MAC_GetAddr64bit(MAC_WITHIN_GIVEN_CONTEXT);


#endif /* _BB_MAC_SAP_ADDRESS_H */