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
*       Contains interface definitions for ZigBee PRO NWK Route Discovery Table.
*
*******************************************************************************/

#ifndef _ZBPRO_NWK_ROUTE_DISCOVERY_TABLE_H
#define _ZBPRO_NWK_ROUTE_DISCOVERY_TABLE_H

/************************* INCLUDES ****************************************************/
#include "bbZbProNwkConfig.h"
#include "bbZbProNwkRouteDiscovery.h"
#include "private/bbZbProNwkServices.h"

/************************* DEFINITIONS *************************************************/

/**//**
 * \brief Search criterion for functions which perform search in the discovery table.
 */
typedef enum _ZbProNwkRDiscTableSearchCriteria_t
{
    NWK_RDISC_ROUTE_REQUEST_REQUIRED,
    NWK_RDISC_ROUTE_REPLY_REQUIRED
} ZbProNwkRDiscTableSearchCriteria_t;


typedef uint8_t ZbProNwkDiscTableTtl_t;

/**//**
 * \brief Macros to work with discovery table entry options.
 */
#define NWK_GET_RDISC_OPT_RREQ_REQUIRED(options)              GET_BITFIELD_VALUE(options, 0,  1)
#define NWK_GET_RDISC_OPT_RREQ_INITIAL(options)               GET_BITFIELD_VALUE(options, 1,  1)
#define NWK_GET_RDISC_OPT_RRPLY_REQUIRED(options)             GET_BITFIELD_VALUE(options, 2,  1)
#define NWK_GET_RDISC_OPT_RRPLY_RECEIVED(options)             GET_BITFIELD_VALUE(options, 3,  1)
#define NWK_GET_RDISC_OPT_NOROUTECACHE(options)               GET_BITFIELD_VALUE(options, 4,  1)
#define NWK_GET_RDISC_OPT_MANYTOONE(options)                  GET_BITFIELD_VALUE(options, 5,  1)

#define NWK_SET_RDISC_OPT_RREQ_REQUIRED(options, value)       SET_BITFIELD_VALUE(options, 0,  1, value)
#define NWK_SET_RDISC_OPT_RREQ_INITIAL(options, value)        SET_BITFIELD_VALUE(options, 1,  1, value)
#define NWK_SET_RDISC_OPT_RRPLY_REQUIRED(options, value)      SET_BITFIELD_VALUE(options, 2,  1, value)
#define NWK_SET_RDISC_OPT_RRPLY_RECEIVED(options, value)      SET_BITFIELD_VALUE(options, 3,  1, value)
#define NWK_SET_RDISC_OPT_NOROUTECACHE(options, value)        SET_BITFIELD_VALUE(options, 4,  1, value)
#define NWK_SET_RDISC_OPT_MANYTOONE(options, value)           SET_BITFIELD_VALUE(options, 5,  1, value)

/**//**
 * \brief Options field for route discovery table entry.
 *  \note
 *  The structure of route discovery table entry options is described below in C
 *  language style:
 *  \code
 *  typedef struct {
 *      For this entry route request transmission is required or not.
 *      bool routeRequestRequired : 1,
 *      A flag indicating that route request is required on originator.
 *      bool initialRouteRequest  : 1,
 *      For this entry route reply transmission is required or not.
 *      bool routeReplyRequired   : 1,
 *      For this entry route reply
 *      bool routeReplyReceived   : 1,
 *      A flag indicating that the concentrator does not store source routes.
 *      bool noRouteCache         : 1,
 *      bool manyToOne            : 1,
 *      bool reserved             : 2
 *  };
 *  \note
 *  To access discovery entry option fields the special macros shall be used.
 */
typedef uint8_t ZbProNwkRouteDiscoveryEntryOptions_t;

/**//**
 * \brief Route discovery table entry. See ZigBee spec r20, Table 3.53.
 */
typedef struct _ZbProNwkRouteDiscoveryEntry_t
{
    ZbProNwkDiscTableTtl_t                ttl;              /*!< Time-to-live of entry. If this field is equal to 0 then
                                                                 entry is inactive. */
    ZbProNwkRouteDiscoveryEntryOptions_t  options;          /*!< Options field for route discovery table entry. */
    ZBPRO_NWK_PathCost_t                    forwardCost;      /*!< The accumulated path cost from the source of the route
                                                                 request to the current device. */
    ZBPRO_NWK_PathCost_t                    residualCost;     /*!< The accumulated path cost from the current device to
                                                                 the destination device. */
    ZBPRO_NWK_NwkAddr_t                   srcAddr;          /*!< The 16-bit network address of the route request's
                                                                 initiator. */
    ZBPRO_NWK_NwkAddr_t                   senderAddr;       /*!< The 16-bit network address of the device that has sent
                                                                 the most recent lowest cost route request command frame
                                                                 corresponding to this entry's route request identifier
                                                                 and source address. This field is used to determine the
                                                                 path that an eventual route reply command frame should
                                                                 follow. */
    ZBPRO_NWK_NwkAddr_t                   dstAddr;          /*!< Destination short address or group id of route
                                                                 discovery request. */
    ZbProNwkRouteRequestId_t              routeRequestId;   /*!< A sequence number for a route request command frame that
                                                                 is incremented each time a device initiates a route
                                                                 request. */
    ZbProNwkRadius_t                      rreqRadius;       /*!< Value of the radius field for the new rebroadcasted
                                                                 route request. */
    ZbProNwkRadius_t                      rrepRadius;       /*!< Value of the radius field of the route replay command
                                                                 with the best cost. */
    uint8_t                               sequenceNumber;   /*!< Sequence number of initial route request command. */
    ZBPRO_NWK_RouteDiscoveryReqDescr_t    *req;             /*!< Initial route request parameters. */
    ZbProNwkOutputBuffer_t                *outPkt;          /*!< Pointer to output network packet. */
} ZbProNwkRouteDiscoveryEntry_t;

/**//**
 * \brief Route discovery table type definition.
 */
typedef struct _ZbProNwkRouteDiscoveryTable_t
{
    ZbProNwkRouteDiscoveryEntry_t table[ZBPRO_NWK_ROUTE_DISCOVERY_TABLE_SIZE];
    SYS_TimeoutTask_t timeoutTask;
} ZbProNwkRouteDiscoveryTable_t;

/************************* PROTOTYPES **************************************************/
/************************************************************************************//**
  \brief Resets the route discovery table.
 ***************************************************************************************/
NWK_PRIVATE void zbProNwkRouteDiscoveryTableReset(void);

/************************************************************************************//**
  \brief Route discovery table task handler.
 ***************************************************************************************/
NWK_PRIVATE void zbProNwkRouteDiscoveryTableHandler(SYS_SchedulerTaskDescriptor_t *const taskDescriptor);

/************************************************************************************//**
  \brief Allocate place for new discovery request in the route discovery table.
  \return Pointer to an allocated route discovery entry or NULL.
 ***************************************************************************************/
NWK_PRIVATE ZbProNwkRouteDiscoveryEntry_t* zbProNwkAllocRouteDiscoveryEntry(void);

/************************************************************************************//**
  \brief Notification about an expired discovery entry.
****************************************************************************************/
NWK_PRIVATE void zbProNwkRouteDiscoveryFreeEntryNtf(ZbProNwkRouteDiscoveryEntry_t *const entry);

/************************************************************************************//**
  \brief Find an active route discovery entry by route request id and source address.
  \param[in] srcAddr - network address of route discovery originator.
  \param[in] requestId - identifier of initial route request.
  \return Pointer to a route discovery entry or NULL if entry is not found.
 ***************************************************************************************/
NWK_PRIVATE ZbProNwkRouteDiscoveryEntry_t* zbProNwkFindRouteDiscoveryEntry(
                                                const ZBPRO_NWK_NwkAddr_t srcAddr,
                                                const ZbProNwkRouteRequestId_t requestId);

/************************************************************************************//**
  \brief Find an active route discovery entry by destination and source addresses.
  \param[in] dstAddr - network address of route discovery destination.
  \param[in] srcAddr - network address of route discovery originator.
  \return Pointer to the active route discovery entry and NULL if an active procedure doesn't exist.
 ***************************************************************************************/
NWK_PRIVATE ZbProNwkRouteDiscoveryEntry_t* zbProNwkIsRouteDiscoveryEntryExist(
                                                const ZBPRO_NWK_NwkAddr_t dstAddr,
                                                const ZBPRO_NWK_NwkAddr_t srcAddr);

/************************************************************************************//**
  \brief Returns the first entry which is waiting for an output buffer allocation.
  \return Pointer to discovery table entry or NULL if there is no pending entries.
 ***************************************************************************************/
NWK_PRIVATE ZbProNwkRouteDiscoveryEntry_t *zbProNwkGetPendingRDiscEntry(
                                            ZbProNwkRDiscTableSearchCriteria_t criteria);

/************************* INLINE FUNCTION PROTOTYPES **********************************/
/************************************************************************************//**
  \brief Updates a path cost with a given link cost.
  \param[in] pathCost - cost of path.
  \param[in] linkCost - cost of link.
  \return Summary path cost.
 ***************************************************************************************/
INLINE ZBPRO_NWK_PathCost_t zbProNwkUpdatePathCost(const ZBPRO_NWK_PathCost_t pathCost,
                                                 const ZBPRO_NWK_Cost_t linkCost)
{
    return MIN((uint16_t)pathCost + linkCost, ZBPRO_NWK_MAX_PATH_COST);
}

#endif /* _ZBPRO_NWK_ROUTE_DISCOVERY_TABLE_H */

/* eof bbZbProNwkRouteDiscoveryTable.h */
