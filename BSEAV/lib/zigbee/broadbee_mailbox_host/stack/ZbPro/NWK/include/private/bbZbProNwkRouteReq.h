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
*       Contains declaration for route request command handler.
*
*******************************************************************************/

#ifndef _ZBPRO_NWK_ROUTE_REQUEST_H
#define _ZBPRO_NWK_ROUTE_REQUEST_H

/************************* INCLUDES ****************************************************/
#include "private/bbZbProNwkServices.h"
#include "private/bbZbProNwkRouteDiscoveryTable.h"

/************************* DEFINITIONS *************************************************/
/**//**
 * \brief Initializer for Route Request service descriptor.
 */
#define NWK_ROUTE_REQUEST_SERVICE_DESCRIPTOR { \
    .payloadSize    = zbProNwkRouteRequestMemSize, \
    .fill       = zbProNwkRouteRequestFill, \
    .conf       = NULL, \
    .ind        = zbProNwkRouteRequestInd, \
    .reset      = NULL, \
}

/**//**
 * \brief Length of base part of route request command payload. ZigBee Spec r20, 3.4.1.
 */
#define NWK_ROUTE_REQUEST_BASE_PAYLOAD_LENGTH \
    ( /* Command Id */          1 + \
      /* Command options */     1 + \
      /* Route request id */    1 + \
      /* Destination address */ 2 + \
      /* Path cost */           1)

/**//**
 * \brief Calculates length of route request command payload. ZigBee Spec r20, 3.4.1.
 */
#define NWK_ROUTE_REQUEST_PAYLOAD_LENGTH(options) \
    (NWK_ROUTE_REQUEST_BASE_PAYLOAD_LENGTH + \
     (NWK_GET_ROUTE_REQ_OPT_DST_EXT_ADDR_FLAG(options) ? /* Destination IEEE address */ 8 : 0))

/**//**
 * \brief Calculates maximum length of route request command payload. ZigBee Spec r20, 3.4.1.
 */
#define NWK_ROUTE_REQUEST_MAX_PAYLOAD_LENGTH \
    (NWK_ROUTE_REQUEST_BASE_PAYLOAD_LENGTH + /* Destination IEEE address */ 8)

/**//**
 * \brief Default value of route request options field.
 */
#define NWK_DEFAULT_ROUTE_REQ_OPTIONS                   0U

/**//**
 * \brief Macros to work with route request options.
 */
#define NWK_GET_ROUTE_REQ_OPT_MANY_TO_ONE(options)                GET_BITFIELD_VALUE(options, 3,  2)
#define NWK_GET_ROUTE_REQ_OPT_DST_EXT_ADDR_FLAG(options)          GET_BITFIELD_VALUE(options, 5,  1)
#define NWK_GET_ROUTE_REQ_OPT_MULTICAST_FLAG(options)             GET_BITFIELD_VALUE(options, 6,  1)

#define NWK_SET_ROUTE_REQ_OPT_MANY_TO_ONE(options, value)         SET_BITFIELD_VALUE(options, 3,  2, value)
#define NWK_SET_ROUTE_REQ_OPT_DST_EXT_ADDR_FLAG(options, value)   SET_BITFIELD_VALUE(options, 5,  1, value)
#define NWK_SET_ROUTE_REQ_OPT_MULTICAST_FLAG(options, value)      SET_BITFIELD_VALUE(options, 6,  1, value)

/************************* TYPES *******************************************************/

/** Many-to-One field values. See ZigBee spec r18, 3.4.1.3.1.1, page 320. */
typedef enum _ZbProNwkRReqManyToOneFlag_t
{
  /** The route request is not a many-to-one route request. */
  NWK_RREQ_IS_NOT_MANY_TO_ONE = 0,
  /** The route request is a many-to-one route request and the sender supports
   * a route record table.*/
  NWK_RREQ_IS_MANY_TO_ONE = 1,
  /** The route request is a many-to-one route request and the sender does not
   * support a route record table. */
  NWK_RREQ_IS_MANY_TO_ONE_NO_RREC_TABLE = 2,
  NWK_RREQ_RESERVED_MANY_TO_ONE_FLAG = 3
} ZbProNwkRReqManyToOneFlag_t;

/**//**
 *  \brief
 *  Route request command options field. ZigBee spec r20, 3.4.1.3.1.
 *  \note
 *  The structure of route request options is described below in C
 *  language style:
 *  \code
 *  typedef struct {
 *      uint8_t         reserved1 : 3,
 *      // The many-to-one field shall have one of the non-reserved values shown
 *      // in ZigBee spec r20, Table 3.41. Since the many-to-one route request is
 *      // not supported the value of this field shall be set to 0.
 *      uint8_t         manyToOne : 2,
 *      // The destination IEEE address field is a single-bit field. It shall have
 *      // a value of 1 if, and only if, the command frame contains the destination
 *      // IEEE address. ZigBee spec r20, 3.4.1.3.1.2.
 *      uint8_t         dstExt    : 1,
 *      // The multicast sub-field is a single-bit field. It shall have a value
 *      // of 1 if, and only if, the command frame is a request for a route
 *      // to a multicast group... ZigBee spec r20, 3.4.1.3.1.3.
 *      uint8_t         multicast : 1,
 *      uint8_t         reserved2 : 1
 *  };
 *  \note
 *  To access route request fields the special macros shall be used.
 */
typedef uint8_t ZbProNwkRouteRequestOptions_t;

/**//**
 * \brief Route request command frame format. ZigBee spec r20, Figure 3.11.
 * \note Please be aware that this structure is not packed and can not be
 *       used for frame parsing.
 */
typedef struct _ZbProNwkRouteRequestPayload_t
{
    ZbProNwkRouteRequestOptions_t   options;    /*!< Route request command options field. */
    ZbProNwkRouteRequestId_t        identifier; /*!< The route request identifier is an 8-bit sequence number
                                                     for route requests and is incremented by 1 every time the
                                                     NWK layer on a particular device issues a route request.
                                                     ZigBee spec r20, 3.4.1.3.2. */
    ZBPRO_NWK_NwkAddr_t             dstAddr;    /*!< The destination address shall be 2 octets in length and
                                                     represents the intended destination of the route request
                                                     command frame. ZigBee spec r20, 3.4.1.3.3. */
    ZBPRO_NWK_PathCost_t              pathCost;   /*!< The path cost field is eight bits in length and is used
                                                     to accumulate routing cost information as a route request
                                                     command frame moves through the network.
                                                     ZigBee spec r20, 3.4.1.3.4, sub-clause 3.6.3.5.2. */
    ZBPRO_NWK_ExtAddr_t             dstExtAddr; /*!< The destination IEEE address shall be 8 octets in length
                                                     and represents the IEEE address of the destination of the
                                                     route request command frame.
                                                     ZigBee spec r20, 3.4.1.3.5. */
} ZbProNwkRouteRequestPayload_t;


/************************* FUNCTIONS PROTOTYPES ****************************************/
/************************************************************************************//**
  \brief Initiates route request command transmission.
  \param[in] discoveryEntry - pointer to the related route discovery entry.
  \param[in] initialReq - flag indicating that this request was initiated by this device.
 ***************************************************************************************/
NWK_PRIVATE void zbProNwkSendRouteRequest(ZbProNwkRouteDiscoveryEntry_t * discoveryEntry, bool initialReq);

/************************************************************************************//**
  \brief Returns memory size required for route request header and payload.
 ****************************************************************************************/
NWK_PRIVATE ZbProNwkGetPayloadSizeServiceHandler_t zbProNwkRouteRequestMemSize;

/************************************************************************************//**
  \brief Fills route request header and payload.
 ****************************************************************************************/
NWK_PRIVATE ZbProNwkFillPacketServiceHandler_t  zbProNwkRouteRequestFill;

/************************************************************************************//**
  \brief Handles a received route request command.
 ****************************************************************************************/
NWK_PRIVATE ZbProNwkIndServiceHandler_t         zbProNwkRouteRequestInd;

#endif /* _ZBPRO_NWK_ROUTE_REQUEST_H */

/* eof bbZbProNwkRouteReq.h */