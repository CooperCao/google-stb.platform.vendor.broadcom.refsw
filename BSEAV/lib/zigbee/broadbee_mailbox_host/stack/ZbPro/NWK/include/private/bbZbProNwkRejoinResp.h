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
* FILENAME: $Workfile: trunk/stack/ZbPro/NWK/include/private/bbZbProNwkRejoinResp.h $
*
* DESCRIPTION:
*   Contains declaration of rejoin response service.
*
* $Revision: 2784 $
* $Date: 2014-07-02 16:12:58Z $
*
****************************************************************************************/
#ifndef _ZBPRO_NWK_REJOIN_RESPONSE_H
#define _ZBPRO_NWK_REJOIN_RESPONSE_H

/************************* INCLUDES ****************************************************/
#include "private/bbZbProNwkServices.h"

/************************* DEFINITIONS *************************************************/
/**//**
 * \brief Initializer for Rejoin Response service descriptor.
 */
#define NWK_REJOIN_RESP_SERVICE_DESCRIPTOR \
    { \
        .payloadSize    = zbProNwkRejoinRespSize, \
        .fill           = zbProNwkRejoinRespFill, \
        .conf           = zbProNwkRejoinRespConf, \
        .ind            = zbProNwkRejoinRespInd, \
        .reset          = zbProNwkRejoinRespReset, \
    }

/**//**
 * \brief Rejoin response command payload length. ZigBee Spec r20, 3.4.7.
 */
#define ZBPRO_NWK_REJOIN_RESP_PAYLOAD_LENGTH    \
    ( /* Command Id */      1 +                 \
      /* Network address */ 2 +                 \
      /* Rejoin status */   1)

/************************* TYPES *******************************************************/
/**//**
 * \brief Rejoin response payload type.(ZigBee spec r20 3.4.7 p.331)
 */
typedef struct _ZbProNwkRejoinResponsePayload_t
{
    ZBPRO_NWK_NwkAddr_t assignedAddr;
    uint8_t             status;
} ZbProNwkRejoinResponsePayload_t;

/**//**
 * \brief Rejoin response parameters.
 */
typedef struct _ZbProNwkRejoinRespParams_t
{
    ZBPRO_NWK_ExtAddr_t joiningDeviceExtAddr;
    ZBPRO_NWK_NwkAddr_t joiningDeviceAddr;
    ZBPRO_NWK_NwkAddr_t assignedAddr;
    uint8_t             status;
    bool                isSecured;
    bool                isDirect;
} ZbProNwkRejoinRespParams_t;

/**//**
 * \brief Rejoin response descriptor prototype.
 */
typedef struct _ZbProNwkRejoinResponseDescriptor_t ZbProNwkRejoinResponseDescriptor_t;

/**//**
 * \brief Rejoin callback type.
 */
typedef void (*ZbProNwkRejoinResponseCallback_t)(ZbProNwkRejoinResponseDescriptor_t *const respDescr,
        ZbProNwkServiceDefaultConfParams_t *const confParams);

/**//**
 * \brief Rejoin response descriptor type.
 */
struct _ZbProNwkRejoinResponseDescriptor_t
{
    SYS_QueueElement_t                  elem;
    ZbProNwkRejoinResponseCallback_t    callback;
    ZbProNwkRejoinRespParams_t          params;
};

/**//**
 * \brief Rejoin response indication parameters.
 */
typedef struct _ZbProNwkRejoinRespIndParams_t
{
    ZBPRO_NWK_ExtAddr_t parentExtAddr;
    ZBPRO_NWK_NwkAddr_t assignedAddr;
    uint8_t             status;
} ZbProNwkRejoinRespIndParams_t;

/**//**
 * \brief Rejoin response service descriptor.
 */
typedef struct _ZbProNwkRejoinResponseServiceDescr_t
{
    SYS_QueueDescriptor_t respQueue;
} ZbProNwkRejoinResponseServiceDescr_t;
/************************* FUNCTIONS PROTOTYPES ****************************************/
/**************************************************************************//**
  \brief Request to send the rejoin response command.
  \param[in] resp - pointer to the rejoin response descriptor.
******************************************************************************/
NWK_PRIVATE void zbProNwkRejoinResp(ZbProNwkRejoinResponseDescriptor_t *resp);

/************************************************************************************//**
    \brief Initialize service.
****************************************************************************************/
NWK_PRIVATE ZbProNwkResetServiceHandler_t       zbProNwkRejoinRespReset;

/************************************************************************************//**
  \brief Returns memory size required for rejoin response header and payload.
 ****************************************************************************************/
NWK_PRIVATE ZbProNwkGetPayloadSizeServiceHandler_t  zbProNwkRejoinRespSize;

/************************************************************************************//**
    \brief Puts data from the pending request to the output buffer. This function invoked
        by the network dispatcher when buffer has been allocated.
    \param[in] outputBuffer - buffer pointer.
****************************************************************************************/
NWK_PRIVATE ZbProNwkFillPacketServiceHandler_t  zbProNwkRejoinRespFill;

/************************************************************************************//**
    \brief This function invoked when frame dropped into air.
    \param[in] outputBuffer - buffer pointer.
    \param[in] status       - transmission status
****************************************************************************************/
NWK_PRIVATE ZbProNwkConfServiceHandler_t        zbProNwkRejoinRespConf;

/************************************************************************************//**
    \brief This function invoked when network status frame has been received.
    \param[in] inputBuffer - buffer pointer.
****************************************************************************************/
NWK_PRIVATE ZbProNwkIndServiceHandler_t         zbProNwkRejoinRespInd;

#endif /* _ZBPRO_NWK_REJOIN_RESPONSE_H */