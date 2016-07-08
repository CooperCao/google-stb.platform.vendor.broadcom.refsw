/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
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
 ******************************************************************************
/*****************************************************************************
*
* FILENAME: $Workfile: trunk/stack/ZbPro/NWK/include/friend/bbZbProNwkNeighborTableFriend.h $
*
* DESCRIPTION:
*   Contains definitions for friend interface for ZigBee PRO NWK Neighbor Table.
*
* $Revision: 2999 $
* $Date: 2014-07-21 13:30:43Z $
*
****************************************************************************************/
#ifndef _ZBPRO_NWK_NEIGHBOR_TABLE_FRIEND_H
#define _ZBPRO_NWK_NEIGHBOR_TABLE_FRIEND_H

/************************* INCLUDES ****************************************************/
#include "bbZbProNwkSapTypesIb.h"
#include "private/bbZbProNwkNeighborTable.h"

/************************* PROTOTYPES **************************************************/
/******************************************************************************
  \brief Count number of associated devices.
  \param[in] startIndex - number of items to be omitted.

  \return Number of associated devices.
 ******************************************************************************/
uint8_t zbProNwkAssocDevNumber(uint8_t startIndex);

/******************************************************************************
  \brief Prepare list of associated devices.
  \param[in] ptr - pointer to array to be filled.
  \param[in] startIndex - number of items to be omitted.
  \param[in] count - size of array to be filled.

  \return Filled list of associated devices.
 ******************************************************************************/
void zbProNwkAssocDevList(ZBPRO_NWK_NwkAddr_t *ptr, uint8_t startIndex, uint8_t count);

/******************************************************************************
  \brief Returns true, if the specified NWK address belongs to one of the children
 ******************************************************************************/
bool zbProNwkIsChild(ZBPRO_NWK_NwkAddr_t addr);

/************************************************************************************//**
  \brief Returns total number of transmit failures over the all neighbors.
  \return Value of total number of transmit failures.
****************************************************************************************/
ZBPRO_NWK_NIB_TxTotal_t zbProNwkNTGetTransmissionFailures(void);

#endif /* _ZBPRO_NWK_NEIGHBOR_TABLE_FRIEND_H */