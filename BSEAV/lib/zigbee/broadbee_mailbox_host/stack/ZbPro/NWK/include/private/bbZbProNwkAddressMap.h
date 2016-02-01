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
* FILENAME: $Workfile: trunk/stack/ZbPro/NWK/include/private/bbZbProNwkAddressMap.h $
*
* DESCRIPTION:
*   Address map declarations.
*
* $Revision: 3062 $
* $Date: 2014-07-25 11:04:46Z $
*
****************************************************************************************/
#ifndef _ZBPRO_NWK_ADDRESS_MAP_H
#define _ZBPRO_NWK_ADDRESS_MAP_H

/************************* INCLUDES ****************************************************/
#include "bbSysBasics.h"
#include "bbZbProNwkCommon.h"
#include "bbZbProNwkConfig.h"

/************************* TYPES *******************************************************/
/**//**
 * \brief Address map entry type.
 */
typedef struct _ZbProNwkAddressMapEntry_t
{
    ZBPRO_NWK_ExtAddr_t extAddr;
    ZBPRO_NWK_NwkAddr_t networkAddr;
    Bool8_t isBusy;
    Bool8_t isConflicted;
} ZbProNwkAddressMapEntry_t;

/**//**
 * \brief Address map service descriptor.
 */
typedef struct _ZbProNwkAddressMapDescriptor_t
{
    ZbProNwkAddressMapEntry_t *oldestEntry;
    ZbProNwkAddressMapEntry_t map[ZBPRO_NWK_ADDRESS_MAP_ENTRY_AMOUNT];
} ZbProNwkAddressMapDescriptor_t;

/************************* FUNCTIONS PROTOTYPES ****************************************/
/************************************************************************************//**
  \brief Resets address map.
****************************************************************************************/
NWK_PRIVATE void zbProNwkAddressMapReset(void);

/************************************************************************************//**
  \brief Updates AddressMap using the specified address pair.
  \return true, if Update succeeds; false, if an address conflict has been detected.
****************************************************************************************/
NWK_PRIVATE bool zbProNwkAddressMapUpdate(const ZBPRO_NWK_NwkAddr_t nwkAddr, const ZBPRO_NWK_ExtAddr_t *const extAddr);

/************************************************************************************//**
  \brief Finds conflicting entry in table.
  \return entry pointer if exist otherwise NULL.
****************************************************************************************/
NWK_PRIVATE ZBPRO_NWK_NwkAddr_t zbProNwkAddressMapFindConflictedAddr(void);

/************************************************************************************//**
  \brief Handles notification about resolving address conflict.
  \return entry pointer or NULL if .
****************************************************************************************/
NWK_PRIVATE void zbProNwkAddressConflictResolved(const ZBPRO_NWK_NwkAddr_t shortAddr);

/************************************************************************************//**
  \brief Gets extended address associated with the short address.
  \param[in] shortAddr - short address.
  \return ZBPRO_NWK_NOT_VALID_EXTENDED_ADDR if associated address not exist and extended address otherwise.
****************************************************************************************/
NWK_PRIVATE ZBPRO_NWK_ExtAddr_t zbProNwkAddressMapGetExtbyShort(const ZBPRO_NWK_NwkAddr_t shortAddr);

/************************************************************************************//**
  \brief Gets short address associated with the extended address.
  \param[in] extAddr - extended address
  \return ZBPRO_NWK_NOT_VALID_UNICAST_ADDR if associated address not exist and extended address otherwise.
****************************************************************************************/
ZBPRO_NWK_NwkAddr_t zbProNwkAddressMapGetShortbyExt(const ZBPRO_NWK_ExtAddr_t *const extAddr);

/************************************************************************************//**
  \brief Removes entry from address map by specified extended address.
  \param[in] extAddr - extended address.
****************************************************************************************/
NWK_PRIVATE void zbProNwkAddressMapRemoveByExt(const ZBPRO_NWK_ExtAddr_t *const extAddr);

#endif /* _ZBPRO_NWK_ADDRESS_MAP_H */