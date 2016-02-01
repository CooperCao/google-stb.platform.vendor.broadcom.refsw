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
* FILENAME: $Workfile: trunk/stack/ZbPro/NWK/include/private/bbZbProNwkRoutingTable.h $
*
* DESCRIPTION:
*   Contains definitions for interface for ZigBee PRO NWK Routing Table.
*
* $Revision: 3816 $
* $Date: 2014-10-02 07:46:11Z $
*
****************************************************************************************/
#ifndef _ZBPRO_NWK_ROUTING_TABLE_H
#define _ZBPRO_NWK_ROUTING_TABLE_H

/************************* INCLUDES ****************************************************/
#include "bbZbProNwkConfig.h"
#include "bbZbProNwkSapTypesRoutingTable.h"
#include "private/bbZbProNwkCommonPrivate.h"
#include "bbZbProNwkNeighbor.h"

/************************* TYPES *******************************************************/
/**//**
 * \brief The routing table.
 */
typedef struct _ZbProNwkRoutingTable_t
{
    /* Pointer to first entry of the routing table. */
    ZBPRO_NWK_RoutingTableEntry_t table[ZBPRO_NWK_ROUTING_TABLE_SIZE];
    /* Number of failure transmission after that a routing entry will be deleted. */
    uint8_t failOrder;
} ZbProNwkRoutingTable_t;

/* Types intended to convey information about specified route. */
/**//**
 * \brief Routing types, used for data packet transmission..
 */
typedef enum _ZbProNwkRouteType_t
{
    NWK_ROUTE_TYPE_UNKNOWN,
    NWK_ROUTE_TYPE_FAIL,
    NWK_ROUTE_TYPE_INDIRECT,
    NWK_ROUTE_TYPE_TO_NEIGHBOR,
    NWK_ROUTE_TYPE_MESH,
    NWK_ROUTE_TYPE_BROADCAST,
    NWK_ROUTE_TYPE_DIRECT,
    NWK_ROUTE_TYPE_SOURCE_ROUTED
} ZbProNwkRouteType_t;

/**//**
 * \brief Describes routing information structure.
 */
typedef struct _ZbProNwkRouteInfo_t
{
    ZbProNwkRouteType_t   type;
    ZBPRO_NWK_NwkAddr_t   nextHopAddress;
    ZBPRO_NWK_PathCost_t  cost;

    /* fields requared for failure reports */
    ZBPRO_NWK_Neighbor_t *neighborForLinkFailure;
    Bool8_t               isManyToOneRoute;
} ZbProNwkRouteInfo_t;

/************************* PROTOTYPES **************************************************/
/************************************************************************************//**
  \brief Allocate a new entry in the routing table.

  \return pointer to an entry or NULL.
****************************************************************************************/
NWK_PRIVATE ZBPRO_NWK_RoutingTableEntry_t* zbProNwkAllocRoutingEntry(void);

/************************************************************************************//**
  \brief Free an entry of the routing table.

  \param[in] dstAddr - the short address of destination node.
  \param[in] isGroupId - 'true' if dstAddr is group id otherwise 'false'.
  \return None.
****************************************************************************************/
NWK_PRIVATE void zbProNwkFreeRoutingEntry(const ZBPRO_NWK_NwkAddr_t dstAddr);

/************************************************************************************//**
  \brief Free all entries with the given next hop.

  \param[in] nextHop - the short address of next hop node.
  \return None.
****************************************************************************************/
NWK_PRIVATE void zbProNwkDeleteNextHop(const ZBPRO_NWK_NwkAddr_t nextHop);

/************************************************************************************//**
  \brief Find a routing table entry by destination address.
  \param[in] dstAddr - a short address of destination node.
  \return pointer to a routing table entry with given dstAddr.
****************************************************************************************/
NWK_PRIVATE ZBPRO_NWK_RoutingTableEntry_t* zbProNwkFindRoutingEntry(const ZBPRO_NWK_NwkAddr_t dstAddr);

/************************************************************************************//**
 \brief Update information of the routing table entry after a packet transmission.

 \param[in] entry - pointer to an entry in the routing table.
 \param status - NWK status of packet transmission.
****************************************************************************************/
NWK_PRIVATE void zbProNwkUpdateRoutingEntry(ZBPRO_NWK_RoutingTableEntry_t *const entry,
    const ZBPRO_NWK_Status_t status);

/************************************************************************************//**
  \brief Reset the routing table.
****************************************************************************************/
NWK_PRIVATE void zbProNwkRoutingTableReset(void);

/***********************************************************************************//**
  \brief Setting the next hop address of the routing table entry.

  \param[in] dstAddr - a short address of destination node.
  \param[in] nextHopAddr - short address of next hop node.
  \param[in] cost - cost of path to destination node.
 **************************************************************************************/
NWK_PRIVATE void zbProNwkUpdateNextHop(const ZBPRO_NWK_NwkAddr_t dstAddr,
    const ZBPRO_NWK_NwkAddr_t nextHopAddr, const ZBPRO_NWK_PathCost_t cost);

/************************* INLINES *****************************************************/
/************************************************************************************//**
  \brief Delete the device from the routing table.
    Free all entries with next hop address or destination address are equal to the given short address.

  \param[in] shortAddr - the short address of removed node.
  \return None.
****************************************************************************************/
INLINE void zbProNwkDeleteFromRoutingTable(const ZBPRO_NWK_NwkAddr_t shortAddr)
{
    zbProNwkFreeRoutingEntry(shortAddr);
    zbProNwkDeleteNextHop(shortAddr);
}

#endif /* _ZBPRO_NWK_ROUTING_TABLE_H */