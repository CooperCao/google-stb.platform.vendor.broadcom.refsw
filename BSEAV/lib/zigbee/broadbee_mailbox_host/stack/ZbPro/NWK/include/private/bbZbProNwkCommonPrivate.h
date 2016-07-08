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
* FILENAME: $Workfile: trunk/stack/ZbPro/NWK/include/private/bbZbProNwkCommonPrivate.h $
*
* DESCRIPTION:
*   Network Layer Private declarations of common types.
*
* $Revision: 3816 $
* $Date: 2014-10-02 07:46:11Z $
*
****************************************************************************************/

#ifndef _ZBPRO_NWK_COMMON_PRIVATE_H
#define _ZBPRO_NWK_COMMON_PRIVATE_H

/************************* INCLUDES ****************************************************/
#include "bbSysPayload.h"
#include "bbZbProNwkCommon.h"
#include "bbZbProNwkConfig.h"

#ifndef _ZBPRO_NVM_SUPPORT_DISABLED_
#include "bbNvmApi.h"
#endif

/************************ DEFINITIONS **************************************************/

/**//**
 * \brief Determines if input packet is transit and should be retransmitted.
 */
#define ZBPRO_NWK_IS_TRANSIT(buffer) (ZBPRO_NWK_ROUTE_RECORD_TRANSIT_SID <= buffer->sId && \
                                      ZBPRO_NWK_UNICAST_TRANSIT_SID >= buffer->sId)

/**//**
 * \brief Determines if packet is a data frame requested to be transmitted by upper layer.
 */
#define ZBPRO_NWK_IS_DATA(buffer) (ZBPRO_NWK_UNICAST_DATA_SID == (buffer)->sId \
                                   || ZBPRO_NWK_BROADCAST_DATA_SID == (buffer)->sId)

/************************* TYPES *******************************************************/

/**//**
 * \brief NWK layer service identifier.
 * \note All these values are not from the Standard. They are just for debugging purposes.
 *       Do not ever change the order of NWK services (by changing their SID values) because
 *       it may interfere NWK behavior.
 *       Make sure that ZBPRO_NWK_MAX_SID and ZBPRO_NWK_UNKNOWN_SID are the very last rows in
 *       this enumeration.
 */
typedef enum _ZbProNwkServiceId_t
{
    ZBPRO_NWK_REPORT_SID                = 0x00, //********************************
    ZBPRO_NWK_UPDATE_SID                = 0x01, // Any conflicts resolving
    ZBPRO_NWK_BROADCAST_STATUS_SID      = 0x02, //
    ZBPRO_NWK_UNICAST_STATUS_SID        = 0x03, //********************************

    ZBPRO_NWK_LEAVE_SID                 = 0x04, //********************************
    ZBPRO_NWK_REJOIN_RESP_SID           = 0x05, // The main network functionality
    ZBPRO_NWK_REJOIN_REQ_SID            = 0x06, //********************************

    ZBPRO_NWK_LINK_STATUS_SID           = 0x07, //********************************
    ZBPRO_NWK_ROUTE_RECORD_SID          = 0x08, //
    ZBPRO_NWK_ROUTE_REPLY_SID           = 0x09, // Functionality of
    ZBPRO_NWK_MANY_TO_ONE_RREQ_SID      = 0x0A, //      the routing procedure
    ZBPRO_NWK_INITIAL_ROUTE_REQ_SID     = 0x0B, //
    ZBPRO_NWK_ROUTE_REQ_SID             = 0x0C, //
    ZBPRO_NWK_MAX_COMMAND_SID           = 0x0D, //********************************

    /* Transit */
    ZBPRO_NWK_ROUTE_RECORD_TRANSIT_SID  = 0x0E, //********************************
    ZBPRO_NWK_MULTICAST_TRANSIT_SID     = 0x0F, // The frame transit
    ZBPRO_NWK_BROADCAST_TRANSIT_SID     = 0x10, //     functionality
    ZBPRO_NWK_UNICAST_TRANSIT_SID       = 0x13, //********************************

    /* Data Service */
    ZBPRO_NWK_UNICAST_DATA_SID          = 0x14, //********************************
    ZBPRO_NWK_BROADCAST_DATA_SID        = 0x15, // Data communication
    // ZBPRO_NWK_SOURCE_ROUTE_SID       = 0x16, //********************************

    ZBPRO_NWK_MAX_SID,
    ZBPRO_NWK_UNKNOWN_SID
} ZbProNwkServiceId_t;

/**//**
 * \brief Type of initial route request identifier. ZigBee spec r20, 3.4.1.3.2.
 */
typedef uint8_t ZbProNwkRouteRequestId_t;

/**//**
 * \brief Type of children counters.
 */
typedef uint8_t ZbProNwkChildCount_t;

/**//**
 * \brief Type of lifetime counter in milliseconds.
 */
typedef uint32_t ZbProNwkLifeTimeMs_t;

/************************* PROTOTYPES **************************************************/

/************************************************************************************//**
 \brief Resets all modules of the ZB PRO NWK layer.
 \param[in] warmStart - false if the request is expected reset all stack values to
                        their initial default values.
 ****************************************************************************************/
void bbZbProNwkReset(bool setToDefault);

#endif /* _ZBPRO_NWK_COMMON_PRIVATE_H */