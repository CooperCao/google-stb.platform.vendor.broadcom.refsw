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
*
* FILENAME: $Workfile: trunk/stack/ZbPro/NWK/include/private/bbZbProNwkBufferManager.h $
*
* DESCRIPTION:
*   Contains declaration of the private network's packet buffers and interface for them.
*
* $Revision: 10263 $
* $Date: 2016-02-29 18:03:06Z $
*
****************************************************************************************/
#ifndef _ZBPRO_NWK_PACKET_MANAGER_H
#define _ZBPRO_NWK_PACKET_MANAGER_H

/************************* INCLUDES ****************************************************/
#include "bbSysFsm.h"
#include "bbSysQueue.h"
#include "bbSysPayload.h"
#include "bbMacSapForZBPRO.h"

#include "bbZbProSsp.h"
#include "bbZbProNwkRouteDiscovery.h"
#include "bbZbProNwkConfig.h"
#include "private/bbZbProNwkCommonPrivate.h"
#include "private/bbZbProNwkFrame.h"
#include "private/bbZbProNwkRoutingTable.h"
#include "private/bbZbProNwkTxStatus.h"
#include "private/bbZbProNwkPassiveAck.h"
#include "private/bbZbProNwkStatus.h"
/************************* TYPES *******************************************************/
/**//**
 * \brief Network packet type enumeration.
 */
typedef enum _ZbProNwkPacketType_t
{
    ZBPRO_NWK_UNKNOWN_PACKET        = 0x0,  /* Indicates empty buffer */
    ZBPRO_NWK_OUTPUT_COMMAND_PACKET = 0x1,  /* Command packet from NWK component. */
    ZBPRO_NWK_OUTPUT_DATA_PACKET    = 0x2,  /* Data packet from an external layer. */
    ZBPRO_NWK_INPUT_COMMAND_PACKET  = 0x3,  /* Command packet from MAC-layer. */
    ZBPRO_NWK_INPUT_DATA_PACKET     = 0x4,  /* Data packet from MAC-layer. */
    ZBPRO_NWK_TRANSIT_PACKET        = 0x5,  /* Transit packet from MAC-layer. */
    ZBPRO_NWK_BUFFER_TYPE_LAST
} ZbProNwkPacketType_t;

/**//**
 * \brief Output buffer descriptor data type.
 */
typedef struct _ZbProNwkOutputServiceData_t
{
    MAC_DataReqDescr_t              macDataReq;         /* Mac data request structure. */
    ZbProNwkRouteInfo_t             routeInfo;          /* Routing information. */
    ZbProNwkTxStatus_t              txStatus;           /* Transmission status structure. */
    SYS_FSM_StateId_t               txFsmState;         /* TX FSM state. */
    union
    {
        ZBPRO_NWK_RouteDiscoveryReqDescr_t  routeDiscovery;
        ZbProNwkStatusReqDescr_t    nwkStatusReq;
    };

    struct
    {
        uint8_t                     rDiscEnabled : 1;       /* If route discovery is enabled. */
        uint8_t                     rDiscForce   : 1;       /* If route discovery shall be started immediately */
        uint8_t                     secured      : 1;       /* If output packet was encrypted. */
        uint8_t                     isLoopback   : 1;       /* If shall be delivered to himself is set to True. */
    };

} ZbProNwkOutputServiceData_t;

/**//**
 * \brief Input buffer descriptor data type.
 */
typedef struct _ZbProNwkInputServiceData_t
{
    PHY_LQI_t                   linkQuality;
    ZBPRO_NWK_Timestamp_t       timestamp;
} ZbProNwkInputServiceData_t;

/**//**
 * \brief The common network buffer structure.
 */
typedef struct _ZbProNwkBuffer_t
{
    /* Common parameters of the Network buffer. */
    SYS_QueueElement_t          qElem;
    void                        *originalReq;

    ZbProNwkServiceId_t         sId;
    ZbProNwkParsedHeader_t      parsedHeader;

    ZBPRO_NWK_NwkAddr_t         prevHopAddr;

    ZbProSspFrameBuffer_t       frameBuffer;

    union
    {
        ZbProSspEncryptReq_t        encryptReq;
        ZbProSspDecryptReq_t        decryptReq;
    };

    union
    {
        ZbProNwkInputServiceData_t      inData;
        ZbProNwkOutputServiceData_t     outData;
    };

    /* used only by buffer manager */
    struct
    {
        ZbProNwkPacketType_t    type;
    } service;
} ZbProNwkBuffer_t;

/**//**
 * \brief The memory descriptor for Packet Manager module.
 */
typedef struct _ZbProNwkPacketManagerDescr_t
{
    /* Start point of the packet table */
    ZbProNwkBuffer_t           table[ZBPRO_NWK_MAX_PACKET_AMOUNT];
} ZbProNwkPacketManagerDescr_t;

/**//**
 * \brief Output buffer type declaration.
 */
typedef ZbProNwkBuffer_t ZbProNwkOutputBuffer_t;

/**//**
 * \brief Input buffer type declaration.
 */
typedef ZbProNwkBuffer_t ZbProNwkInputBuffer_t;

/************************* FUNCTIONS PROTOTYPES ****************************************/
/************************************************************************************//**
    \brief Sets internal variables of the network packet manager to the default state.
****************************************************************************************/
NWK_PRIVATE void zbProNwkBufferManagerReset(void);

/************************************************************************************//**
    \brief Finds a free packet buffer and marks it as input buffer.
    \param[in] type - requested type of the packet buffer

    \return Pointer to the allocated buffer or NULL if free buffer is absent
****************************************************************************************/
NWK_PRIVATE ZbProNwkInputBuffer_t *zbProNwkBufferManagerAllocInputPacket(ZbProNwkPacketType_t type);

/************************************************************************************//**
    \brief Tries to duplicate the packet buffer.
    \return Pointer to the duplicated buffer or NULL if there is no free buffers or memory
****************************************************************************************/
NWK_PRIVATE ZbProNwkBuffer_t *zbProNwkBufferManagerDuplicatePacket(const ZbProNwkBuffer_t *const buffer);

/************************************************************************************//**
    \brief Gets a free the packet buffer and marks it as output buffer.
    \param[in] type - requested type of the packet buffer
    \param[in] headerLength - header length in bytes.
    \param[in] payloadLength - payload length in bytes.
    \return a pointer to the allocated buffer or NULL if free buffer is absent
****************************************************************************************/
NWK_PRIVATE ZbProNwkOutputBuffer_t *zbProNwkBufferManagerAllocOutputPacket(ZbProNwkPacketType_t type,
                                                                           uint8_t headerLength,
                                                                           uint8_t payloadLength);

/************************************************************************************//**
    \brief Frees the packet buffer marked as input buffer.
    \param[in] buffer - pointer to the buffer to be freed.
    \param[in] freePayload - should be true if the payload is needed to be cleared.
****************************************************************************************/
NWK_PRIVATE void zbProNwkBufferManagerFreeInputPacket(ZbProNwkInputBuffer_t *buffer, bool freePayload);

/************************************************************************************//**
    \brief Frees the packet buffer marked as output buffer.
    \param[in] buffer - pointer to the buffer to be freed.
    \param[in] freePayload - should be true if the payload is needed to be cleared.
****************************************************************************************/
NWK_PRIVATE void zbProNwkBufferManagerFreeOutputPacket(ZbProNwkOutputBuffer_t *buffer, bool freePayload);

/************************************************************************************//**
    \brief Receives indication what packet manager has a free buffer.
****************************************************************************************/
extern void zbProNwkBufferManagerFreePacketInd(void);

/************************************************************************************//**
    \brief Changes buffer type
    \param[in] buffer - a pointer to the packet buffer.
****************************************************************************************/
NWK_PRIVATE void zbProNwkBufferManagerConvertPacketType(ZbProNwkInputBuffer_t *buffer, ZbProNwkPacketType_t typeToSet);

/************************************************************************************//**
    \brief Transform output buffer to input one. Used in loopback case.
    \param[in] buffer - a pointer to the output buffer.
    \return Pointer to the input buffer.
****************************************************************************************/
NWK_PRIVATE ZbProNwkInputBuffer_t *zbProNwkBufferManagerConvertOutToIn(ZbProNwkOutputBuffer_t *outputBuffer);

#endif /* _ZBPRO_NWK_PACKET_MANAGER_H */
