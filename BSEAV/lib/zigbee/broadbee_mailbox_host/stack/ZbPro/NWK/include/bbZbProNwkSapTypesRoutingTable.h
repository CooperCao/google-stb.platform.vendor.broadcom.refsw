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
* FILENAME: $Workfile: trunk/stack/ZbPro/NWK/include/bbZbProNwkSapTypesRoutingTable.h $
*
* DESCRIPTION:
*   Contains definitions for public interface for ZigBee PRO NWK Routing Table.
*
* $Revision: 2440 $
* $Date: 2014-05-19 14:56:23Z $
*
****************************************************************************************/
#ifndef _ZBPRO_NWK_SAP_TYPES_ROUTING_TABLE_H
#define _ZBPRO_NWK_SAP_TYPES_ROUTING_TABLE_H

/************************* INCLUDES ****************************************************/
#include "bbZbProNwkCommon.h"

/************************* TYPES *******************************************************/
/**//**
 * \brief Route Status Values. ZigBee spec r20, Table 3.52.
 */
typedef enum _ZBPRO_NWK_RouteStatus_t
{
    ZBPRO_NWK_ROUTE_STATUS_ACTIVE                   = 0,
    ZBPRO_NWK_ROUTE_STATUS_DISCOVERY_UNDERWAY       = 1,
    ZBPRO_NWK_ROUTE_STATUS_DISCOVERY_FAILED         = 2,
    ZBPRO_NWK_ROUTE_STATUS_INACTIVE                 = 3,
    ZBPRO_NWK_ROUTE_STATUS_VALIDATION_UNDERWAY      = 4
} ZBPRO_NWK_RouteStatus_t;

/**//**
 * \brief Type of routing rate.
 */
typedef uint8_t ZBPRO_NWK_RoutingRate_t;

/**//**
 * \brief Bit fields for Routing table entry. ZigBee spec r20, Table 3.51.
 */
typedef struct _ZBPRO_NWK_RoutingTableEntryFlags_t
{
    /* A flag indicating that the destination indicated by this address does not store source routes. */
    BitField8_t noRouteCache :1;
    /* A flag indicating that the destination is a concentrator that issued a many-to-one route request. */
    BitField8_t manyToOne :1;
    /* A flag indicating that a route record command frame should be sent to
     * the destination prior to the next data packet. */
    BitField8_t routeRecordRequired :1;
    /* A flag indicating that the destination address is a Group ID. */
    BitField8_t groupId :1;
    /* Indicate to upper layer about new concentrator. */
    BitField8_t newConcentrator :1;
    /* Counter of the many-to-one discovery periods without any source route packets. */
    BitField8_t noSourceRoutePeriods :2;
    /* Unused part of byte. */
    BitField8_t reserved :1;
} ZBPRO_NWK_RoutingTableEntryFlags_t;

/**//**
 * \brief Routing table entry. ZigBee spec r20, Table 3.51.
 */
typedef struct _ZBPRO_NWK_RoutingTableEntry_t
{
    /* The 16-bit network address or Group ID of this route. If the destination
     * device is a ZigBee router, ZigBee coordinator, or an end device, and
     * nwkAddrAlloc has a value of 0x02, this field shall contain the actual
     * 16-bit address of that device. If the destination device is an end device
     * and nwkAddrAlloc has a value of 0x00, this field shall contain the 16-bit
     * network address of the device's parent. */
    ZBPRO_NWK_NwkAddr_t dstAddr;
    /* The 16-bit network address of the next hop on the way to the destination. */
    ZBPRO_NWK_NwkAddr_t nextHopAddress;
    ZBPRO_NWK_RoutingRate_t rate;
    /* Cost of route path to destination node. */
    ZBPRO_NWK_PathCost_t cost;
    /* A flag indicating status of entry. */
    ZBPRO_NWK_RouteStatus_t status;
    /* Bit fields. */
    ZBPRO_NWK_RoutingTableEntryFlags_t flags;
} ZBPRO_NWK_RoutingTableEntry_t;

#endif /* _ZBPRO_NWK_SAP_TYPES_ROUTING_TABLE_H */