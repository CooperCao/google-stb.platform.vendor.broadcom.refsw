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
* FILENAME: $Workfile: trunk/stack/ZbPro/NWK/include/bbZbProNwkConstants.h $
*
* DESCRIPTION:
*   Contains ZigBee PRO NWK layer constants.
*
* $Revision: 2595 $
* $Date: 2014-06-03 15:11:16Z $
*
****************************************************************************************/
#ifndef _ZBPRO_NWK_CONSTANTS_H
#define _ZBPRO_NWK_CONSTANTS_H

/************************* DEFINITIONS *************************************************/

/**************** Network Layer constants. See ZigBee Spec r20, Table 3.43. ************/

/**//**
 * \brief A Boolean flag indicating whether the device is capable of becoming the ZigBee
 *        coordinator. Value is configuration dependent.
 */
#define ZBPRO_NWKC_COORD_CAPABLE            true

/**//**
 * \brief The default security level to be used.
 */
#define ZBPRO_NWKC_DEFAULT_SECURITY_LEVEL   0x05

/**//**
 * \brief The minimum number of octets added by the NWK layer to an NSDU.
 */
#define ZBPRO_NWKC_MIN_HEADER_OVERHEAD      0x08

/**//**
 * \brief The version of the ZigBee NWK protocol in the device.
 */
#define ZBPRO_NWKC_PROTOCOL_VERSION         0x02

/**//**
 * \brief The number of OctetDurations, on the originator of a multicast route request,
          between receiving a route reply and sending a message to validate the route.
 */
#define ZBPRO_NWKC_WAIT_BEFORE_VALIDATION       0x9c40
/* TODO: Use some function from MAC/PHY to calculate time in ms using
         current octet duration value. */
#define ZBPRO_NWKC_WAIT_BEFORE_VALIDATION_MS    0x0500

/**//**
 * \brief The number of OctetDurations until a route discovery expires.
 */
#define ZBPRO_NWKC_ROUTE_DISCOVERY_TIME         0x4c4b4
/* TODO: Use some function from MAC/PHY to calculate time in ms using
         current octet duration value. */
#define ZBPRO_NWKC_ROUTE_DISCOVERY_TIME_MS      0x2710

/**//**
 * \brief The maximum broadcast jitter time measured in OctetDurations.
 */
#define ZBPRO_NWKC_MIN_BROADCAST_JITTER         0x0034
#define ZBPRO_NWKC_MAX_BROADCAST_JITTER         0x07d0
/* TODO: Use some function from MAC/PHY to calculate time in ms using
         current octet duration value. */
#define ZBPRO_NWKC_MAX_BROADCAST_JITTER_MS      0x40

/**//**
 * \brief Delay between unicast retransmission (see formula from R20 3.5.2.1).
 */
#define ZBPRO_NWKC_HOP_DELIVERY_DELAY_MS        5
/**//**
 * \brief The number of times the first broadcast transmission of a route request command
 *        frame is retried.
 */
#define ZBPRO_NWKC_INITIAL_RREQ_RETRIES         0x03

/**//**
 * \brief The number of times the broadcast transmission of a route request command
 *        frame is retried on relay by an intermediate ZigBee router or ZigBee coordinator.
 */
#define ZBPRO_NWKC_RREQ_RETRIES                 0x02

/**//**
 * \brief The number of OctetDurations between retries of a broadcast route request command
 *        frame.
 */
#define ZBPRO_NWKC_RREQ_RETRY_INTERVAL          0x1f02
/* TODO: Use some function from MAC/PHY to calculate time in ms using
         current octet duration value. */
#define ZBPRO_NWKC_RREQ_RETRY_INTERVAL_MS       0xfe

/**//**
 * \brief The minimum jitter, in OctetDurations, for broadcast retransmission of a route request
 *        command frame.
 */
#define ZBPRO_NWKC_MIN_RREQ_JITTER              0x3f
/* TODO: Use some function from MAC/PHY to calculate time in ms using
         current octet duration value. */
#define ZBPRO_NWKC_MIN_RREQ_JITTER_MS           0x02

/**//**
 * \brief The maximum jitter, in OctetDurations, for broadcast retransmission of a route request
 *        command frame.
 */
#define ZBPRO_NWKC_MAX_RREQ_JITTER              0x0fa0
/* TODO: Use some function from MAC/PHY to calculate time in ms using
         current octet duration value. */
#define ZBPRO_NWKC_MAX_RREQ_JITTER_MS           0x128

/**//**
 * \brief The size of the MAC header used by the ZigBee NWK layer.
 */
#define ZBPRO_NWKC_MAC_FRAME_OVERHEAD           0x0b



/********* Additional Network Layer constants. Not specified in ZigBee Spec r20 ********/

/**//**
 * \brief The maximum value of the LQI parameter.
 */
#define ZBPRO_NWKC_MAX_LQI                        0xFF

/**//**
 * \brief The outgoing cost field contains the cost of the link as measured by the neighbor.
 *   The value is obtained from the most recent link status command frame received
 *   from the neighbor. A value of 0 indicates that no link status command listing this
 *   device has been received. See ZigBee Spec r20, Table 3.48..
 */
#define ZBPRO_NWK_NO_OUTGOING_COST                  0U

/**//**
 * \brief Maximum value of a link cost metric.
 */
#define ZBPRO_NWK_MAX_LINK_COST                     7U

/**//**
 * \brief Maximum cost of route path.
 */
#define ZBPRO_NWK_MAX_PATH_COST                     ((uint8_t)UINT8_MAX - 1U)

/**//**
 * \brief The number of link failures after which the neighbor is treated as lost.
 */
#define ZBPRO_NWKC_MAX_LINK_FAILURES_ALLOWED        3

/**//**
 * \brief The maximum jitter, in OctetDurations, for broadcast transmission of a link status
 *        command frame.
 */
#define ZBPRO_NWKC_MAX_LINK_STATUS_JITTER           0x0FFF

/**//**
 * \brief The minimum jitter, in OctetDurations, for broadcast transmission of a link status
 *        command frame.
 */
#define ZBPRO_NWKC_MIN_LINK_STATUS_JITTER           0x0034

/**//**
 * \brief The maximum depth of the network.
 */
#define ZBPRO_NWKC_MAX_DEPTH                        20

/**//**
 * \brief Threshold value of a link cost to neighbor.
 */
#define ZBPRO_NWKC_ROUTE_TO_NEIGHBOR_LINK_COST_THRESHOLD   3U

/**//**
 * \brief This field identifies the network layer protocols in use and, for
 *        purposes of this specification, shall always be set to 0,
 *        indicating the ZigBee protocols. The value 0xff shall also be
 *        reserved for future use by the ZigBee Alliance. ZigBee Spec r20 Table 3.56.
 */
#define ZBPRO_NWKC_PROTOCOL_ID                      0x00

/**//**
 * \brief MAC Beacon payload length in bytes. ZigBee Spec r20 3.6.7.
 */
#define ZBPRO_NWKC_BEACON_PAYLOAD_LENGTH            15

/**//**
 * \brief Stack profile identifier (Zigbee PRO profile is equal to 2).
 *        See ZigBee Document 074855r05.
 */
#define ZBPRO_NWKC_STACK_PROFILE_ID                 0x02

/**//**
 * \brief Network security level (Zigbee PRO network security level shall be set to 5).
 *        See ZigBee Document 074855r05.
 */
#define ZBPRO_NWKC_SECURITY_LEVEL                   5

#endif /* _ZBPRO_NWK_CONSTANTS_H */