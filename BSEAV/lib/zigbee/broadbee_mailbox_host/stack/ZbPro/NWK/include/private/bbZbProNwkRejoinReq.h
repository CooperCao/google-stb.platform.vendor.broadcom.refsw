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
* FILENAME: $Workfile: trunk/stack/ZbPro/NWK/include/private/bbZbProNwkRejoinReq.h $
*
* DESCRIPTION:
*   Contains declaration of rejoin request service.
*
* $Revision: 10289 $
* $Date: 2016-03-02 13:51:14Z $
*
****************************************************************************************/
#ifndef _ZBPRO_NWK_REJOIN_REQUEST_H
#define _ZBPRO_NWK_REJOIN_REQUEST_H

/************************* INCLUDES ****************************************************/
#include "private/bbZbProNwkServices.h"

/************************* DEFINITIONS *************************************************/
/**//**
 * \brief Initializer for Rejoin Request service descriptor.
 */
#define NWK_REJOIN_REQ_SERVICE_DESCRIPTOR \
{ \
    .payloadSize    = zbProNwkRejoinReqSize, \
    .fill           = zbProNwkRejoinReqFill, \
    .conf           = zbProNwkRejoinReqConf, \
    .ind            = zbProNwkRejoinReqInd, \
    .reset          = zbProNwkRejoinReqReset, \
}

/**//**
 * \brief Rejoin request command payload length. ZigBee Spec r20, 3.4.6.
 */
#define ZBPRO_NWK_REJOIN_REQ_PAYLOAD_LENGTH \
    ( /* Command Id */ 1 +                  \
      /* Capability information */ 1)

/************************* TYPES *******************************************************/

/**//**
 * \brief Rejoin request payload type. See ZigBee spec r20, 3.4.6.
 */
typedef struct _ZbProNwkRejoinRequestPayload_t
{
    ZBPRO_NWK_Capability_t capabilityInfo;
} ZbProNwkRejoinRequestPayload_t;

/**//**
 * \brief Rejoin request parameters.
 */
typedef struct _ZbProNwkRejoinReqParams_t
{
    ZBPRO_NWK_ExtAddr_t     candidateExtAddr;
    ZBPRO_NWK_NwkAddr_t     candidateNwkAddr;
    ZBPRO_NWK_Capability_t  capabilityInfo;
    Bool8_t                 isKnownCandidateExtAddr;
    Bool8_t                 secured;
} ZbProNwkRejoinReqParams_t;

/**//**
 * \brief Rejoin request descriptor prototype.
 */
typedef struct _ZbProNwkRejoinReqDescriptor_t ZbProNwkRejoinReqDescriptor_t;

/**//**
 * \brief Rejoin request callback type.
 */
typedef void (*ZbProNwkRejoinReqCallback_t) (ZbProNwkRejoinReqDescriptor_t *const reqDescr,
                                             ZbProNwkServiceDefaultConfParams_t *const confParams);
/**//**
 * \brief Rejoin request descriptor type.
 */
struct _ZbProNwkRejoinReqDescriptor_t
{
    SYS_QueueElement_t          elem;
    ZbProNwkRejoinReqParams_t   params;
    ZbProNwkRejoinReqCallback_t callback;
};

/**//**
 * \brief Rejoin requests service descriptor.
 */
typedef struct _ZbProNwkRejoinRequestServiceDescr_t
{
    SYS_QueueDescriptor_t rejoinQueue;
} ZbProNwkRejoinRequestServiceDescr_t;

/************************* FUNCTIONS PROTOTYPES ****************************************/
/**************************************************************************//**
  \brief Request to send the rejoin request command.
  \param[in] req - pointer to the network status request descriptor.
******************************************************************************/
NWK_PRIVATE void zbProNwkRejoinReq(ZbProNwkRejoinReqDescriptor_t *req);

/************************************************************************************//**
    \brief Initialize service.
****************************************************************************************/
NWK_PRIVATE ZbProNwkResetServiceHandler_t       zbProNwkRejoinReqReset;

/************************************************************************************//**
  \brief Returns memory size required for rejoin request header and payload.
 ****************************************************************************************/
NWK_PRIVATE ZbProNwkGetPayloadSizeServiceHandler_t  zbProNwkRejoinReqSize;

/************************************************************************************//**
    \brief Puts data from the pending request to the output buffer. This function invoked
        by the network dispatcher when buffer has been allocated.
    \param[in] outputBuffer - buffer pointer.
****************************************************************************************/
NWK_PRIVATE ZbProNwkFillPacketServiceHandler_t  zbProNwkRejoinReqFill;

/************************************************************************************//**
    \brief This function invoked when frame dropped into air.
    \param[in] outputBuffer - buffer pointer.
    \param[in] status       - transmission status
****************************************************************************************/
NWK_PRIVATE ZbProNwkConfServiceHandler_t        zbProNwkRejoinReqConf;

/************************************************************************************//**
    \brief This function invoked when network status frame has been received.
    \param[in] inputBuffer - buffer pointer.
****************************************************************************************/
NWK_PRIVATE ZbProNwkIndServiceHandler_t         zbProNwkRejoinReqInd;

#endif /* _ZBPRO_NWK_REJOIN_REQUEST_H */