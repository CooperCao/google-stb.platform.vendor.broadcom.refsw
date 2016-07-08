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
 * FILENAME: $Workfile: trunk/stack/ZbPro/APS/include/private/bbZbProApsRx.h $
 *
 * DESCRIPTION:
 *   Declaration of the ZigBee PRO APS Rx component.
 *
 * $Revision: 10263 $
 * $Date: 2016-02-29 18:03:06Z $
 *
 ****************************************************************************************/

#ifndef _ZBPRO_APS_RX_H
#define _ZBPRO_APS_RX_H

/************************* INCLUDES ****************************************************/
#include "bbSysFsm.h"

#include "bbZbProSsp.h"

#include "bbZbProNwkSapTypesData.h"

#include "bbZbProApsCommon.h"
#include "bbZbProApsKeywords.h"
#include "friend/bbZbProApsKeyPairFriend.h"

/************************* TYPES *******************************************************/
/**//**
 * \brief Descriptor of Hub variables
 */
typedef struct _ZbProApsRxDesc_t
{
    SYS_QueueDescriptor_t fsmQueue;
    SYS_QueueDescriptor_t rxAckQueue;
} ZbProApsRxDesc_t;

/**//**
 * \brief Rx buffer type definition
 */
typedef struct _ZbProApsRxBuffer_t
{
    SYS_QueueElement_t          qElem;

    ZbProSspFrameBuffer_t       frameBuffer;

    ZbProSspKey_t               actualKey;      /* effective key of a Transport Key command */

    union
    {
        ZbProSspDecryptReq_t        decReq;
        ZBPRO_NWK_DataReqDescr_t    nwkReq;
    };

    ZbProApsKeyPairReference_t  *apsKeyRef;

    ZBPRO_APS_Timestamp_t       rxTime;
    ZBPRO_APS_ShortAddr_t       nwkSrcAddr;

    union
    {
        ZBPRO_APS_ShortAddr_t   nwkDstAddr;
        ZBPRO_APS_GroupId_t     groupId;
    };

    ZBPRO_APS_ClusterId_t       clusterId;
    ZBPRO_APS_ProfileId_t       profileId;

    ZBPRO_APS_EndpointId_t      dstEndpoint;
    ZBPRO_APS_EndpointId_t      srcEndpoint;

    ZbProApsCounter_t           apsCounter;

    PHY_LQI_t                   lqi;
    ZBPRO_APS_Status_t          status;

    SYS_FSM_StateId_t           rxFsmState;
    ZbProSspKeyId_t             keyId;

    struct
    {
        BitField8_t             nwkSecurity     : 1;

        BitField8_t             security        : 1;
        BitField8_t             ackRequest      : 1;

        BitField8_t             isCommand       : 1;
        BitField8_t             isGroupcast     : 1;
        BitField8_t             isExtHeader     : 1;
        BitField8_t             isExtNonce      : 1;
    };

} ZbProApsRxBuffer_t;

/************************* FUNCTION PROTOTYPES ******************************************/

/**//**
 * \brief Handles an incoming ACK
 */
APS_PRIVATE void zbProApsRxAckHandler(SYS_SchedulerTaskDescriptor_t *const taskDescriptor);

/**//**
 * \brief Rx FSM task handler
 */
APS_PRIVATE void zbProApsRxFsmHandler(SYS_SchedulerTaskDescriptor_t *const taskDescriptor);

/**//**
 * \brief Frees the specified Rx buffer
 */
APS_PRIVATE void zbProApsRxFreeBuffer(ZbProApsRxBuffer_t *buf);

/**//**
 * \brief Resets the module
 */
APS_PRIVATE void zbProApsRxReset(void);

#endif /* _ZBPRO_APS_RX_H */
