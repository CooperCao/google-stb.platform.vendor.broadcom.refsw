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
* FILENAME: $Workfile: trunk/stack/ZbPro/NWK/include/private/bbZbProNwkRouteRply.h $
*
* DESCRIPTION:
*   Contains declaration for route reply command handler.
*
* $Revision: 3165 $
* $Date: 2014-08-06 10:33:59Z $
*
****************************************************************************************/
#ifndef _ZBPRO_NWK_ROUTE_REPLY_H
#define _ZBPRO_NWK_ROUTE_REPLY_H

/************************* INCLUDES ****************************************************/
#include "private/bbZbProNwkServices.h"
#include "private/bbZbProNwkRouteDiscoveryTable.h"

/************************* DEFINITIONS *************************************************/

/**//**
 * \brief Initializer for Route Reply service descriptor.
 */
#define NWK_ROUTE_REPLY_SERVICE_DESCRIPTOR { \
    .payloadSize    = zbProNwkRouteReplyMemSize, \
    .fill       = zbProNwkRouteReplyFill, \
    .conf       = NULL, \
    .ind        = zbProNwkRouteReplyInd, \
    .reset      = NULL, \
}

/**//**
 * \brief Length of base part of route reply command payload. ZigBee Spec r20, 3.4.2.
 */
#define NWK_ROUTE_REPLY_BASE_PAYLOAD_LENGTH \
    ( /* Command Id */          1 + \
      /* Command options */     1 + \
      /* Route request id */    1 + \
      /* Originator address */  2 + \
      /* Responder address */   2 + \
      /* Path cost */           1)

/**//**
 * \brief Calculates length of route reply command payload. ZigBee Spec r20, 3.4.2.
 */
#define NWK_ROUTE_REPLY_PAYLOAD_LENGTH(options) \
    (NWK_ROUTE_REPLY_BASE_PAYLOAD_LENGTH + \
     (NWK_GET_ROUTE_RPLY_OPT_ORG_EXT_ADDR_FLAG(options) ? /* Originator IEEE address */ 8 : 0) + \
     (NWK_GET_ROUTE_RPLY_OPT_RESP_EXT_ADDR_FLAG(options) ? /* Responder IEEE address */ 8 : 0))

/**//**
 * \brief Calculates maximum length of route reply command payload. ZigBee Spec r20, 3.4.2.
 */
#define NWK_ROUTE_REPLY_MAX_PAYLOAD_LENGTH \
    (NWK_ROUTE_REPLY_BASE_PAYLOAD_LENGTH + /* Originator IEEE address */ 8 + /* Responder IEEE address */ 8)

/**//**
 * \brief Default value of route reply options field.
 */
#define NWK_DEFAULT_ROUTE_RPLY_OPTIONS                  0U

/**//**
 * \brief Macros to work with route reply options.
 */
#define NWK_GET_ROUTE_RPLY_OPT_ORG_EXT_ADDR_FLAG(options)           GET_BITFIELD_VALUE(options, 4,  1)
#define NWK_GET_ROUTE_RPLY_OPT_RESP_EXT_ADDR_FLAG(options)          GET_BITFIELD_VALUE(options, 5,  1)
#define NWK_GET_ROUTE_RPLY_OPT_MULTICAST_FLAG(options)              GET_BITFIELD_VALUE(options, 6,  1)

#define NWK_SET_ROUTE_RPLY_OPT_ORG_EXT_ADDR_FLAG(options, value)    SET_BITFIELD_VALUE(options, 4,  1, value)
#define NWK_SET_ROUTE_RPLY_OPT_RESP_EXT_ADDR_FLAG(options, value)   SET_BITFIELD_VALUE(options, 5,  1, value)
#define NWK_SET_ROUTE_RPLY_OPT_MULTICAST_FLAG(options, value)       SET_BITFIELD_VALUE(options, 6,  1, value)

/************************* TYPES *******************************************************/

/**//**
    \brief
    The options field of route reply command. ZigBee spec. r20, Figure 3.14.
    \note
    The structure of route reply options is described below in C
    language style:
    \code
    typedef struct {
        uint8_t         reserved1 : 4,
        // If this field has value of 1 then the IEEE address of originator is
        // included in the payload of the route reply command.
        uint8_t         originatorExtAddr : 1,
        // If this field has value of 1 then the IEEE address of responder is
        // included in the payload of the route reply command.
        uint8_t         responderExtAddr : 1,
        // This field shall have a value of 1 if and only if the command frame is
        // a reply to a request for a route to a multicast group.
        uint8_t         multicast : 1,
        uint8_t         reserved2 : 1
    };
    \note
    To access route reply fields the special macros shall be used.
 */
typedef uint8_t ZbProNwkRouteReplyOptions_t;

/**//**
 * \brief Route reply command frame format. ZigBee spec r20, Figure 3.13.
 * \note It is not recommended to use this type directly since using
 *       of unaligned access on ARC dramatically increases consumption of
 *       code memory.
 */
typedef struct _ZbProNwkRouteReplyPayload_t
{
    ZbProNwkRouteReplyOptions_t     options;        /*!< Route reply command options field. */
    ZbProNwkRouteRequestId_t        identifier;     /*!< The route request identifier is the number of the route
                                                         request to which this frame is a reply.
                                                         ZigBee spec r20, 3.4.2.3.2. */
    ZBPRO_NWK_NwkAddr_t             orgNwkAddr;     /*!< The network address of the originator of the route request
                                                         command frame to which this frame is a reply.
                                                         ZigBee spec r20, 3.4.2.3.3. */
    ZBPRO_NWK_NwkAddr_t             respNwkAddr;    /*!< The responder address field shall always be the same as
                                                         the value in the destination address field of the
                                                         corresponding route request command frame.
                                                         ZigBee spec r20, 3.4.2.3.4. */
    ZBPRO_NWK_PathCost_t              pathCost;       /*!< The path cost field is used to sum link cost as the route
                                                         reply command frame transits the network.
                                                         ZigBee spec r20, 3.4.2.3.5, sub-clause 3.6.3.5.2. */
    ZBPRO_NWK_ExtAddr_t             orgExtAddr;     /*!< 64-bit address of the originator of the route request
                                                         command frame to which this frame is a reply.
                                                         ZigBee spec r20, 3.4.2.3.6. */
    ZBPRO_NWK_ExtAddr_t             respExtAddr;    /*!< 64-bit address of the destination of the route request
                                                         command frame to which this frame is a reply.
                                                         ZigBee spec r20, 3.4.2.3.7. */
} ZbProNwkRouteReplyPayload_t;


/************************* FUNCTIONS PROTOTYPES ****************************************/
/**************************************************************************//**
  \brief Initiates route reply command transmission.
  \param[in] discoveryEntry - pointer to the related route discovery entry.
 ******************************************************************************/
NWK_PRIVATE void zbProNwkSendRouteReply(ZbProNwkRouteDiscoveryEntry_t * discoveryEntry);

/************************************************************************************//**
  \brief Returns memory size required for route reply header and payload.
 ****************************************************************************************/
NWK_PRIVATE ZbProNwkGetPayloadSizeServiceHandler_t zbProNwkRouteReplyMemSize;

/************************************************************************************//**
  \brief Fills route reply header and payload.
 ****************************************************************************************/
NWK_PRIVATE ZbProNwkFillPacketServiceHandler_t  zbProNwkRouteReplyFill;

/************************************************************************************//**
  \brief Handles a received route reply command.
 ****************************************************************************************/
NWK_PRIVATE ZbProNwkIndServiceHandler_t         zbProNwkRouteReplyInd;

#endif /* _ZBPRO_NWK_ROUTE_REPLY_H */