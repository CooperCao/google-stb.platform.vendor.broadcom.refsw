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
*
*
*******************************************************************************/

#ifndef _ZBPRO_NWK_SAP_NETWORK_STATUS_H
#define _ZBPRO_NWK_SAP_NETWORK_STATUS_H

/************************* INCLUDES ****************************************************/
#include "bbZbProNwkCommon.h"

/************************* TYPES *******************************************************/
/**//**
 * \brief Network status error code enumeration (ZigBee spec r20 3.4.3.3.1 p.324)
 * \ingroup ZBPRO_NWK_StatusInd
 */
typedef enum _ZBPRO_NWK_NetworkStatusCode_t
{
    ZBPRO_NWK_NO_ROUTE_AVAILABLE            = 0x00U, /*!< No route available: Route discovery and/or repair has
                                                          been attempted and no route to the intended destination
                                                          address has been discovered. */
    ZBPRO_NWK_TREE_LINK_FAILURE             = 0x01U, /*!< Tree link failure: The routing failure occurred as a
                                                          result of the failure of an attempt to route the frame
                                                          along the tree. */
    ZBPRO_NWK_NON_TREE_LINK_FAILUER         = 0x02U, /*!< Non-tree link failure: The routing failure did not occur
                                                          as a result of an attempt to route along the tree. */
    ZBPRO_NWK_LOW_BATTARY_LEVEL             = 0x03U, /*!< Low battery level: The frame was not relayed because the
                                                          relaying device was running low on battery power. */
    ZBPRO_NWK_NO_ROUTING_CAPACITY           = 0x04U, /*!< No routing capacity: The failure occurred because the
                                                          relaying device has no routing capacity. */
    ZBPRO_NWK_NO_INDERECT_CAPACITY          = 0x05U, /*!< No indirect capacity: The failure occurred as the result
                                                          of an attempt to buffer a frame for a sleeping end device
                                                          child and the relaying device had no buffer capacity to use. */
    ZBPRO_NWK_INDERECT_TRANSACTION_EXPIRY   = 0x06U, /*!< Indirect transaction expiry: A frame that was buffered on
                                                          behalf of a sleeping end device child has been dropped as a
                                                          result of a time-out. */
    ZBPRO_NWK_TARGET_DEVICE_UNAVAILABLE     = 0x07U, /*!< Target device unavailable: An end device child of the
                                                          relaying device is for some reason unavailable. */
    ZBPRO_NWK_TARGET_ADDRESS_UNALLOCATED    = 0x08U, /*!< Target address unallocated: The frame was addressed to a
                                                          non-existent end device child of the relaying device. */
    ZBPRO_NWK_PARENT_LINK_FAILURE           = 0x09U, /*!< Parent link failure: The failure occurred as a result of a
                                                          failure in the RF link to the device's parent. This status
                                                          is only used locally on a device to indicate loss of
                                                          communication with the parent. */
    ZBPRO_NWK_VALIDATE_ROUTE                = 0x0AU, /*!< Validate route: The multicast route identified in the
                                                          destination address field should be validated as described
                                                          in sub-clause 3.6.3.6. */
    ZBPRO_NWK_SOURCE_ROUTE_FAILURE          = 0x0BU, /*!< Source route failure: Source routing has failed, probably
                                                          indicating a link failure in one of the source route's links. */
    ZBPRO_NWK_MANY_TO_ONE_ROUTE_FAILURE     = 0x0CU, /*!< Many-to-one route failure: A route established as a result
                                                          of a many-to-one route request has failed. */
    ZBPRO_NWK_ADDRESS_CONFLICT              = 0x0DU, /*!< Address conflict: The address in the destination address
                                                          field has been determined to be in use by two or more
                                                          devices. */
    ZBPRO_NWK_VERIFY_ADDRESSES              = 0x0EU, /*!< Verify addresses: The source device has the IEEE address in
                                                          the Source IEEE address field and, if the Destination IEEE
                                                          address field is present, the value it contains is the
                                                          expected IEEE address of the destination. */
    ZBPRO_NWK_PAN_IDENTIFIER_UPDATE         = 0x0FU, /*!< PAN identifier update: The operational network PAN
                                                          identifier of the device has been updated. */
    ZBPRO_NWK_NETWORK_ADDRESS_UPDATE        = 0x10U, /*!< Network address update: The network address of the device
                                                          has been updated. */
    ZBPRO_NWK_BAD_FRAME_COUNTER             = 0x11U, /*!< Bad frame counter: A frame counter reported in a received
                                                          frame had a value less than or equal to that stored in
                                                          nwkSecurityMaterialSet. */
    ZBPRO_NWK_BAD_KEY_SEQUENCE_NUMBER       = 0x12U, /*!< Bad key sequence number: The key sequence number reported
                                                          in a received frame did not match that of
                                                          nwkActiveKeySeqNumber. */
} ZBPRO_NWK_NetworkStatusCode_t;

/**//**
 * \brief NLME-NWK-STATUS indication primitive's parameters structure.
 * Zigbee Specification r20, 3.2.2.30 NLME-NWK-STATUS.indication, page 305.
 * \ingroup ZBPRO_NWK_StatusInd
 */
typedef struct _NWK_NwkStatusInd_t
{
    ZBPRO_NWK_NwkAddr_t             networkAddr;     /*!< The 16-bit network address of the device associated with
                                                          the status information. */
    ZBPRO_NWK_NetworkStatusCode_t   status;          /*!< The error code associated with the failure. */
} ZBPRO_NWK_NwkStatusIndParams_t;

/******************************************************************************
                              Prototypes section
 ******************************************************************************/
/**************************************************************************//**
  \brief NLME-NWK-STATUS indication primitive's prototype.
  \ingroup ZBPRO_NWK_Functions

  \param[in] ind - pointer to the indication parameters.
  \return Nothing.
  \note : ZigBee spec r20 3.2.2.30.2 p.304 NLME-NWK-STATUS.indication
This primitive is generated by the NWK layer on a device and passed to the next
higher layer when one of the following occurs:
  \li The device has failed to discover or repair a route to the destination given by
the NetworkAddr parameter.
  \li The NWK layer on that device has failed to deliver a frame to its end device
child with the 16 - bit network address given by the NetworkAddr parameter,
due to one of the reasons given in Table 3.42.
  \li The NWK layer has received a network status command frame destined for the
device. In this case, the values of the NetworkAddr and Status parameters will
reflect the values of the destination address and error code fields of the
command frame.
 ******************************************************************************/
void ZBPRO_NWK_NwkStatusInd(ZBPRO_NWK_NwkStatusIndParams_t *ind);

#endif /* _ZBPRO_NWK_SAP_NETWORK_STATUS_H */

/* eof bbZbProNwkSapTypesNetworkStatus.h */