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
 * FILENAME: $Workfile: trunk/stack/ZbPro/NWK/include/private/bbZbProNwkFrame.h $
 *
 * DESCRIPTION:
 *   Contains types declarations for the ZigBee PRO NWK frame structures.
 *
 * $Revision: 3816 $
 * $Date: 2014-10-02 07:46:11Z $
 *
 ****************************************************************************************/
#ifndef _ZBPRO_NWK_FRAME_H
#define _ZBPRO_NWK_FRAME_H

/************************* INCLUDES *****************************************************/
#include "bbSysPayload.h"
#include "private/bbZbProNwkCommonPrivate.h"

#include "bbZbProSspFrameBuffer.h"

/************************* DEFINITIONS **************************************************/
/**//**
 * \brief Offset of the frame control field within the network header.
 */
#define NWK_FRAME_CONTROL_OFFSET                0U

/**//**
 * \brief Offset of the sequence number field within the network header.
 */
// TODO: not used
#define NWK_FRAME_SEQUENCE_NUMBER_OFFSET        7U

/**//**
 * \brief Size of base NWK header. ZigBee spec r20, Figure 3.5
 */
#define NWK_SIZE_OF_BASE_HEADER                 8U

/**//**
 * \brief Size of source routing subframe base fields (relay index and relay count).
 *        ZigBee spec r20, Figure 3.8.
 */
#define NWK_SIZE_OF_BASE_SRC_ROUTE_SUBFRAME     2U

/**//**
 * \brief Maximum possible size of NWK header (without source routing subframe).
 *        NWK_SIZE_OF_BASE_HEADER +
 *        sizeof(ZBPRO_NWK_ExtAddr_t) +
 *        sizeof(ZBPRO_NWK_ExtAddr_t) +
 *        sizeof(ZbProNwkMulticastControl_t)
 */
#define NWK_SIZE_OF_MAX_HEADER                  25U

/**//**
 * \brief The size of NWK Command Identifier field.
 */
#define NWK_SIZE_OF_COMMAND_ID                  1U

/**//**
 * \brief The maximum value of Non-member radius field.
 */
#define NWK_MAX_NON_MEMBER_MULTICAST_RADIUS     7U

/**//**
 * \brief The forbidden value of relay index field. Is used to determine the situation
 *        when a source routed frame was received with relay index field equal to zero.
 */
#define ZBPRO_NWK_FORBIDDEN_RELAY_INDEX_VALUE   255U

/**//**
 * \brief Macro to be used in logical operations. True if network header belongs to the Data packet.
 */
#define NWK_IS_DATA_FRAME(parsedHeader) \
    (NWK_FRAME_TYPE_DATA == NWK_GET_FRAME_TYPE((parsedHeader)->frameControl))

/**//**
 * \brief Macro to be used in logical operations. True if network header belongs to the Command packet.
 */
#define NWK_IS_COMMAND_FRAME(parsedHeader) \
    (NWK_FRAME_TYPE_CMD == NWK_GET_FRAME_TYPE((parsedHeader)->frameControl))

/**//**
 * \brief Macro to be used in logical operations. True if network header belongs to the Command packet.
 */
#define NWK_IS_VALID_CMD_ID(id) \
    (ZBPRO_NWK_CMD_MIN_ID <= parsedHeader->cmdId && ZBPRO_NWK_CMD_MAX_ID >= parsedHeader->cmdId)

/**//**
 * \brief Macro to be used in logical operations. True if network header belongs to the multicast.
 */
#define NWK_IS_MULTICAST_FRAME(parsedHeader) \
    (1U == NWK_GET_MULTICAST_FLAG((parsedHeader)->frameControl))

/**//**
 * \brief Macros to work with frame control.
 */
#define NWK_GET_FRAME_TYPE(frameControl)                GET_BITFIELD_VALUE(frameControl, 0,  2)
#define NWK_GET_PROTOCOL_VERSION(frameControl)          GET_BITFIELD_VALUE(frameControl, 2,  4)
#define NWK_GET_DISCOVERY_ROUTE(frameControl)           GET_BITFIELD_VALUE(frameControl, 6,  2)
#define NWK_GET_MULTICAST_FLAG(frameControl)            GET_BITFIELD_VALUE(frameControl, 8,  1)
#define NWK_GET_SECURITY_FLAG(frameControl)             GET_BITFIELD_VALUE(frameControl, 9,  1)
#define NWK_GET_SRC_ROUTE_FLAG(frameControl)            GET_BITFIELD_VALUE(frameControl, 10, 1)
#define NWK_GET_DST_EXT_ADDR_FLAG(frameControl)         GET_BITFIELD_VALUE(frameControl, 11, 1)
#define NWK_GET_SRC_EXT_ADDR_FLAG(frameControl)         GET_BITFIELD_VALUE(frameControl, 12, 1)
#define NWK_GET_RESERVED_FIELD(frameControl)            GET_BITFIELD_VALUE(frameControl, 13, 3)

#define NWK_SET_FRAME_TYPE(frameControl, value)         SET_BITFIELD_VALUE(frameControl, 0,  2, value)
#define NWK_SET_PROTOCOL_VERSION(frameControl, value)   SET_BITFIELD_VALUE(frameControl, 2,  4, value)
#define NWK_SET_DISCOVERY_ROUTE(frameControl, value)    SET_BITFIELD_VALUE(frameControl, 6,  2, value)
#define NWK_SET_MULTICAST_FLAG(frameControl, value)     SET_BITFIELD_VALUE(frameControl, 8,  1, value)
#define NWK_SET_SECURITY_FLAG(frameControl, value)      SET_BITFIELD_VALUE(frameControl, 9,  1, value)
#define NWK_SET_SRC_ROUTE_FLAG(frameControl, value)     SET_BITFIELD_VALUE(frameControl, 10, 1, value)
#define NWK_SET_DST_EXT_ADDR_FLAG(frameControl, value)  SET_BITFIELD_VALUE(frameControl, 11, 1, value)
#define NWK_SET_SRC_EXT_ADDR_FLAG(frameControl, value)  SET_BITFIELD_VALUE(frameControl, 12, 1, value)
#define NWK_SET_RESERVED_FIELD(frameControl, value)     SET_BITFIELD_VALUE(frameControl, 13, 3, value)

/**//**
 * \brief Macros to work with frame control.
 */
#define NWK_GET_MULTICAST_MODE(multicastControl)                GET_BITFIELD_VALUE(multicastControl, 0, 2)
#define NWK_GET_NON_MEMBER_RADIUS(multicastControl)             GET_BITFIELD_VALUE(multicastControl, 2, 3)
#define NWK_GET_MAX_NON_MEMBER_RADIUS(multicastControl)         GET_BITFIELD_VALUE(multicastControl, 5, 3)

#define NWK_SET_MULTICAST_MODE(multicastControl, value)         SET_BITFIELD_VALUE(multicastControl, 0, 2, value)
#define NWK_SET_NON_MEMBER_RADIUS(multicastControl, value)      SET_BITFIELD_VALUE(multicastControl, 2, 3, value)
#define NWK_SET_MAX_NON_MEMBER_RADIUS(multicastControl, value)  SET_BITFIELD_VALUE(multicastControl, 5, 3, value)

/************************* TYPES ********************************************************/
/**//**
 * \brief NPDU Frame Control sub-field values, see ZigBee Specification r20, 3.3.1.1.1.
 */
typedef enum _ZbProNwkFrameType_t
{
    NWK_FRAME_TYPE_DATA       = 0,
    NWK_FRAME_TYPE_CMD        = 1,
    NWK_FRAME_TYPE_RSRV       = 2,
    NWK_FRAME_TYPE_INTER_PAN  = 3
} ZbProNwkFrameType_t;

/**//**
 * \brief NPDU Discover Route sub-field values, see ZigBee Specification r20, 3.3.1.1.3.
 */
typedef enum _ZbProNwkFrameDiscoverRoute_t
{
    NWK_FRAME_DISCOVER_ROUTE_SUPPRESS     = 0,
    NWK_FRAME_DISCOVER_ROUTE_ENABLE       = 1,
    NWK_FRAME_DISCOVER_ROUTE_RSRV1        = 2,
    NWK_FRAME_DISCOVER_ROUTE_RSRV2        = 3
} ZbProNwkFrameDiscoverRoute_t;

/**//**
 * \brief NPDU Frame Control Field, see ZigBee Specification r20, 3.3.1.1.
 */
typedef uint16_t ZbProNwkFrameControl_t;

/**//**
 * \brief NPDU Multicast Control Field, see ZigBee Specification r20, 3.3.1.8.1.
 */
typedef enum _ZbProNwkFrameMulticastMode_t
{
    NWK_FRAME_MULTICAST_MODE_NONMEMBER    = 0,
    NWK_FRAME_MULTICAST_MODE_MEMBER       = 1,
    NWK_FRAME_MULTICAST_MODE_RSRV1        = 2,
    NWK_FRAME_MULTICAST_MODE_RSRV2        = 3
} ZbProNwkFrameMulticastMode_t;

/**//**
 * \brief NPDU Multicast Control Field, see ZigBee Specification r20, 3.3.1.8.
 */
typedef uint8_t ZbProNwkMulticastControl_t;

/**//**
 * \brief Type of source route relay count field.
 */
typedef uint8_t ZbProNwkRelayCount_t;

/**//**
 * \brief Type of source route relay index field.
 */
typedef uint8_t ZbProNwkRelayIndex_t;

/**//**
 * \brief Type of source route relay list field.
 */
typedef SYS_DataPointer_t ZbProNwkRelayList_t;

/**//**
 * \brief Network Header Source Route Subframe Field, see ZigBee Specification r20, 3.3.1.9
 *        Be aware that position of fields doesn't correspond to the position described in spec.
 */
typedef struct _ZbProNwkSourceRouteSubframe_t
{
    SYS_DataPointer_t     relayList;
    ZbProNwkRelayCount_t  relayCnt;
    ZbProNwkRelayIndex_t  relayIdx;
} ZbProNwkSourceRouteSubframe_t;

/**//**
 * \brief Type of radius counter.
 */
typedef uint8_t ZbProNwkRadius_t;

/**//**
 * \brief Type of network sequence number.
 */
typedef uint8_t ZbProNwkSequenceNumber_t;

/**//**
 * \brief NWK command frame identifiers. ZigBee spec r20, Table 3.40. TODO: move to the right place
 */
typedef enum _ZbProNwkCommandId_t
{
    ZBPRO_NWK_CMD_ROUTE_REQUEST     = 0x01U, /* Zigbee spec r20, 3.4.1 (Route request) */
    ZBPRO_NWK_CMD_MIN_ID            = ZBPRO_NWK_CMD_ROUTE_REQUEST,
    ZBPRO_NWK_CMD_ROUTE_REPLY       = 0x02U, /* Zigbee spec r20, 3.4.2 (Route reply) */
    ZBPRO_NWK_CMD_NETWORK_STATUS    = 0x03U, /* Zigbee spec r20, 3.4.3 (Network Status) */
    ZBPRO_NWK_CMD_LEAVE             = 0x04U, /* Zigbee spec r20, 3.4.4 (Leave) */
    ZBPRO_NWK_CMD_ROUTE_RECORD      = 0x05U, /* Zigbee spec r20, 3.4.5 (Route Record) */
    ZBPRO_NWK_CMD_REJOIN_REQUEST    = 0x06U, /* Zigbee spec r20, 3.4.6 (Rejoin request) */
    ZBPRO_NWK_CMD_REJOIN_RESPONSE   = 0x07U, /* Zigbee spec r20, 3.4.7 (Rejoin response) */
    ZBPRO_NWK_CMD_LINK_STATUS       = 0x08U, /* Zigbee spec r20, 3.4.8 (Link Status) */
    ZBPRO_NWK_CMD_NETWORK_REPORT    = 0x09U, /* Zigbee spec r20, 3.4.9 (Network Report) */
    ZBPRO_NWK_CMD_NETWORK_UPDATE    = 0x0AU, /* Zigbee spec r20, 3.4.10 (Network Update) */
    ZBPRO_NWK_CMD_MAX_ID            = ZBPRO_NWK_CMD_NETWORK_UPDATE,
} ZbProNwkCommandId_t;

/**//**
 * \brief Parsed network layer header, see ZigBee Specification r20, 3.3.1.
 *        Be aware that position of fields doesn't correspond to the position described in spec.
 */
typedef struct _ZbProNwkParsedHeader_t
{
    ZBPRO_NWK_ExtAddr_t             dstExtAddr;
    ZBPRO_NWK_ExtAddr_t             srcExtAddr;
    ZbProNwkSourceRouteSubframe_t   sourceRoute;
    ZbProNwkFrameControl_t          frameControl;
    ZBPRO_NWK_NwkAddr_t             dstAddr;
    ZBPRO_NWK_NwkAddr_t             srcAddr;
    ZbProNwkRadius_t                radius;
    ZbProNwkSequenceNumber_t        seqNum;
    ZbProNwkMulticastControl_t      multicastControl;
} ZbProNwkParsedHeader_t;

/************************* FUNCTION PROTOTYPES *****************************************/

/************************************************************************************//**
  \brief Function performs calculation of the Network Layer header size for given parameters.
  \param[in] parsedHeader - pointer to network header.
  \return Network Layer header size.
****************************************************************************************/
NWK_PRIVATE uint8_t zbProNwkCalculateHeaderSize(ZbProNwkParsedHeader_t *const parsedHeader);

/************************************************************************************//**
  \brief Function performs calculation of the total memory size required for
         security auxiliary header and mic.
  \param[in] parsedHeader - pointer to network header.
  \return Additional size required for security needs.
****************************************************************************************/
NWK_PRIVATE uint8_t zbProNwkCalculateTotalSecurityOverhead(ZbProNwkParsedHeader_t *const parsedHeader);

/************************************************************************************//**
  \brief Function performs reading of the Network Layer Command Identifier.
  \param[in]    payload - pointer to the payload.
****************************************************************************************/
NWK_PRIVATE ZbProNwkCommandId_t zbProNwkReadCommandId(SYS_DataPointer_t *payload);

/************************************************************************************//**
  \brief Initialize the common part of network header.
  \param[in] parsedHeader - pointer to a parsed network header structure.
 ***************************************************************************************/
NWK_PRIVATE void zbProNwkInitHeader(ZbProNwkParsedHeader_t *const parsedHeader);

/************************************************************************************//**
  \brief Function performs composing of the Network Layer header.
  \param[in, out]   payload - pointer to the destination payload.
  \param[in]        parsedHeader - pointer to network header.
****************************************************************************************/
NWK_PRIVATE void zbProNwkComposeHeader(SYS_DataPointer_t *payload,
                                       ZbProNwkParsedHeader_t *const parsedHeader);

/************************************************************************************//**
  \brief Function performs parsing of the Network header length.
  \param[in]    payload - pointer to the payload.
  \param[out]   parsedHeader - pointer to network header.
  \return True if the procedure is completed successfully, false otherwise.
****************************************************************************************/
NWK_PRIVATE bool zbProNwkParseHeaderCommonPart(SYS_DataPointer_t *payload,
                                               ZbProNwkParsedHeader_t *const parsedHeader);

/************************************************************************************//**
  \brief Function finalizes parsing of the Network header.
  \param[in]    frameBuffer - pointer to the frame buffer structure.
  \param[in,out]   parsedHeader - pointer to network header.
  \return True if the procedure is completed successfully, false otherwise.
****************************************************************************************/
NWK_PRIVATE bool zbProNwkParseHeaderFinalize(ZbProSspFrameBuffer_t *const frameBuffer,
                                             ZbProNwkParsedHeader_t *const parsedHeader);

/************************************************************************************//**
  \brief Gets maximum default radius. It's (2 * maxDepth) or 255.
  \return 2 * maxDepth if maxDepth less 128 otherwise 255.
 ***************************************************************************************/
NWK_PRIVATE ZbProNwkRadius_t zbProNwkGetDefaultRadius(void);

/**************************************************************************//**
    \brief Splits incoming payload to frame buffer structure.
    \param[in] payload - pointer to the MCPS-DATA indication payload.
    \param[in] parsedHeader - pointer to structure with parsed fields.
    \param[in] frameBuffer - pointer to the frame buffer structure.
    \return true if operation was successful, false otherwise.
 ******************************************************************************/
NWK_PRIVATE bool zbProNwkParseFrame(SYS_DataPointer_t *const payload,
                                    ZbProNwkParsedHeader_t *const parsedHeader,
                                    ZbProSspFrameBuffer_t *const frameBuffer);

/**************************************************************************//**
    \brief Returns the relay list entry for specified index.
    \param[in] parsedHeader - pointer to structure with parsed fields.
    \param[in] relayIdx - index of the relay to be returned.
    \return Relay list entry.
 ******************************************************************************/
NWK_PRIVATE ZBPRO_NWK_NwkAddr_t zbProNwkGetRelayListEntry(ZbProNwkParsedHeader_t *const parsedHeader,
                                                          const uint8_t index);

#endif /* _ZBPRO_NWK_FRAME_H */