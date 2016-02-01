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
* FILENAME: $Workfile: trunk/stack/ZbPro/NWK/include/private/bbZbProNwkLinkStatus.h $
*
* DESCRIPTION:
*   Contains declaration of link status service.
*
* $Revision: 3362 $
* $Date: 2014-08-21 10:52:52Z $
*
****************************************************************************************/
#ifndef _ZBPRO_NWK_LINK_STATUS_H
#define _ZBPRO_NWK_LINK_STATUS_H

/************************* INCLUDES ****************************************************/
#include "bbSysTimeoutTask.h"
#include "private/bbZbProNwkServices.h"

/************************* DEFINITIONS *************************************************/

/**//**
 * \brief Initializer for Link Status service descriptor.
 */
#define NWK_LINK_STATUS_SERVICE_DESCRIPTOR { \
    .payloadSize = zbProNwkLinkStatusPayloadSize, \
    .fill       = zbProNwkLinkStatusFill, \
    .conf       = zbProNwkLinkStatusConf, \
    .ind        = zbProNwkLinkStatusInd, \
    .reset      = zbProNwkLinkStatusReset, \
}

/**//**
 * \brief Length of Network Header of the Link Status command. ZigBee Spec r20, 3.4.8.2.
 */
#define NWK_LINK_STATUS_NETWORK_HEADER_SIZE \
    (NWK_SIZE_OF_BASE_HEADER + sizeof(ZBPRO_NWK_ExtAddr_t))

/**//**
 * \brief Length of Link Status Entry. ZigBee Spec r20, Figure 3.23.
 */
#define NWK_LINK_STATUS_ENTRY_SIZE \
    (sizeof(ZBPRO_NWK_NwkAddr_t) + sizeof(ZBPRO_NWK_LinkCost_t))


/**//**
 * \brief Maximum number of Link Status List Fields which fit into one network packet.
 */
#define NWK_MAX_LINK_ENTRIES_AMOUNT_PER_PACKET \
    ((MAC_MAX_SAFE_PAYLOAD_SIZE \
      - NWK_LINK_STATUS_NETWORK_HEADER_SIZE \
      - NWK_LINK_STATUS_BASE_PAYLOAD_LENGTH) / NWK_LINK_STATUS_ENTRY_SIZE)

/**//**
 * \brief Length of the base part of link status command payload. ZigBee Spec r20, 3.4.8.
 */
#define NWK_LINK_STATUS_BASE_PAYLOAD_LENGTH  sizeof(ZbProNwkLinkStatusPayload_t)

/**//**
 * \brief Calculates length of link status command payload. ZigBee Spec r20, 3.4.8.
 */
#define NWK_LINK_STATUS_PAYLOAD_LENGTH(options) \
    (NWK_LINK_STATUS_BASE_PAYLOAD_LENGTH + \
     (NWK_GET_LINK_STATUS_OPTIONS_AMOUNT(options) * /* Link Status List Field */ NWK_LINK_STATUS_ENTRY_SIZE))

/**//**
 * \brief Calculates maximum length of link status command payload. ZigBee Spec r20, 3.4.8.
 */
#define NWK_LINK_STATUS_MAX_PAYLOAD_LENGTH \
    (NWK_LINK_STATUS_BASE_PAYLOAD_LENGTH + NWK_MAX_LINK_ENTRIES_AMOUNT_PER_PACKET * NWK_LINK_STATUS_ENTRY_SIZE)

/************************* TYPES *******************************************************/
/**//**
 * \brief Macros to work with link status command options.
 */
#define NWK_GET_LINK_STATUS_OPTIONS_AMOUNT(cmdOptions)          GET_BITFIELD_VALUE(cmdOptions, 0, 5)
#define NWK_GET_LINK_STATUS_OPTIONS_IS_FIRST(cmdOptions)        GET_BITFIELD_VALUE(cmdOptions, 5, 1)
#define NWK_GET_LINK_STATUS_OPTIONS_IS_LAST(cmdOptions)         GET_BITFIELD_VALUE(cmdOptions, 6, 1)
#define NWK_GET_LINK_STATUS_OPTIONS_RESERVED(cmdOptions)        GET_BITFIELD_VALUE(cmdOptions, 7, 1)

#define NWK_SET_LINK_STATUS_OPTIONS_AMOUNT(cmdOptions, value)   SET_BITFIELD_VALUE(cmdOptions, 0, 5, value)
#define NWK_SET_LINK_STATUS_OPTIONS_IS_FIRST(cmdOptions, value) SET_BITFIELD_VALUE(cmdOptions, 5, 1, value)
#define NWK_SET_LINK_STATUS_OPTIONS_IS_LAST(cmdOptions, value)  SET_BITFIELD_VALUE(cmdOptions, 6, 1, value)
#define NWK_SET_LINK_STATUS_OPTIONS_RESERVED(cmdOptions, value) SET_BITFIELD_VALUE(cmdOptions, 7, 1, value)

/**//**
 * \brief Link status type. (ZigBee spec r20 3.4.8.3.2 p.335)
 */
typedef struct _ZbProNwkLinkStatus_t
{
    ZBPRO_NWK_NwkAddr_t     addr;
    ZBPRO_NWK_LinkCost_t    linkCost;
} ZbProNwkLinkStatus_t;

/**//**
 * \brief Link status payload type. (ZigBee spec r20 3.4.8.3 p.334)
 */
typedef struct _ZbProNwkLinkStatusPayload_t
{
    uint8_t                 cmdId;
    uint8_t                 cmdOptions;
    ZbProNwkLinkStatus_t    link[];
} ZbProNwkLinkStatusPayload_t;

/**//**
 * \brief Link status service descriptor.
 */
typedef struct _ZbProNwkLinkStatusServiceDescr_t
{
    /*  */
    ZBPRO_NWK_NwkAddr_t lastAddr;                       /*!< Device address which have been linked last time. */
    SYS_TimeoutSignal_t timeoutSignal;                  /*!< Internal timer. Is used to perform a delay
                                                             before/after transmission attempt. */
} ZbProNwkLinkStatusServiceDescr_t;

/************************* FUNCTIONS PROTOTYPES ****************************************/
/************************************************************************************//**
  \brief Link status service handler.
 ****************************************************************************************/
NWK_PRIVATE void zbProNwkLinkStatusHandler(SYS_SchedulerTaskDescriptor_t *const notUsed);

/************************************************************************************//**
    \brief Initialize service.
****************************************************************************************/
NWK_PRIVATE ZbProNwkResetServiceHandler_t       zbProNwkLinkStatusReset;

/************************************************************************************//**
    \brief Returns memory size required for route request header and payload.
    \param[in] outputBuffer - buffer pointer.
 ****************************************************************************************/
NWK_PRIVATE ZbProNwkGetPayloadSizeServiceHandler_t  zbProNwkLinkStatusPayloadSize;

/************************************************************************************//**
    \brief Puts data from the pending request to the output buffer. This function invoked
        by the network dispatcher when buffer has been allocated.
    \param[in] outputBuffer - buffer pointer.
****************************************************************************************/
NWK_PRIVATE ZbProNwkFillPacketServiceHandler_t  zbProNwkLinkStatusFill;

/************************************************************************************//**
    \brief This function invoked when frame dropped into air.
    \param[in] outputBuffer - buffer pointer.
    \param[in] status       - transmission status
****************************************************************************************/
NWK_PRIVATE ZbProNwkConfServiceHandler_t        zbProNwkLinkStatusConf;

/************************************************************************************//**
    \brief This function invoked when network status frame has been received.
    \param[in] inputBuffer - buffer pointer.
****************************************************************************************/
NWK_PRIVATE ZbProNwkIndServiceHandler_t         zbProNwkLinkStatusInd;
#endif /* _ZBPRO_NWK_LINK_STATUS_H */