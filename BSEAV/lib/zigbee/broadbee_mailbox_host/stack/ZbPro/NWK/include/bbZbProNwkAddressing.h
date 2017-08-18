/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *****************************************************************************/

/******************************************************************************
*
* DESCRIPTION:
*       Network Address Map definitions and declaration.
*
*******************************************************************************/

#ifndef _BB_ZB_PRO_NWK_ADDRESSING_H
#define _BB_ZB_PRO_NWK_ADDRESSING_H

/************************* INCLUDES *****************************************************/
#include "bbZbProNwkCommon.h"
#include "bbMacSapAddress.h"

/************************* FUNCTION PROTOTYPES ******************************************/

/**************************************************************************//**
  \brief Adds a new Extended/Short address pair to both the Neighbor and Address Map table.

  \param[in] nwkAddr - network address.
  \param[in] extAddr - extended address.
  \return true if the pair was successfully added, false otherwise due to an Address Conflict.
 ******************************************************************************/
NWK_PUBLIC bool ZBPRO_NWK_LinkShortAndExtAddress(
        const ZBPRO_NWK_NwkAddr_t nwkAddr, const ZBPRO_NWK_ExtAddr_t *const extAddr);

/**************************************************************************//**
  \brief Updates/Sets up the specified Extended<->Short address pair in
         both the Neighbor and Address Map table on a Device_Annce.

  \param[in] nwkAddr - network address.
  \param[in] extAddr - extended address.
  \return true if the pair was successfully updated,
             false otherwise due to an Address Conflict.
 ******************************************************************************/
NWK_PUBLIC bool ZBPRO_NWK_UpdateAddressPairOnAnnce(
        const ZBPRO_NWK_NwkAddr_t nwkAddr, const ZBPRO_NWK_ExtAddr_t *const extAddr);

/**************************************************************************//**
  \brief Finds appropriate extended address by given network address.

  \param[in] nwkAddr - network address.
  \return MAC_UNASSIGNED_EXTENDED_ADDRESS if pair is absent, otherwise valid extended address.
 ******************************************************************************/
NWK_PUBLIC ZBPRO_NWK_ExtAddr_t ZBPRO_NWK_GetExtByNetworkAddress(const ZBPRO_NWK_NwkAddr_t nwkAddr);

/**************************************************************************//**
  \brief Finds appropriate network address by given extended address.
  \param[in] extAddr - extended address.
  \return 0xFFFF if pair is absent, otherwise a valid network address.
 ******************************************************************************/
NWK_PUBLIC ZBPRO_NWK_NwkAddr_t ZBPRO_NWK_GetNetworkByExtAddress(const ZBPRO_NWK_ExtAddr_t *const extAddr);

/**************************************************************************//**
  \brief Finds appropriate (random) network address.
  \return 0xFFFF if pair is absent, otherwise a valid network address.
 ******************************************************************************/
NWK_PUBLIC ZBPRO_NWK_NwkAddr_t ZBPRO_NWK_GetRandomNetworkAddress(void);

#endif /* _BB_ZB_PRO_NWK_ADDRESSING_H */

/* eof bbZbProNwkAddressing.h */