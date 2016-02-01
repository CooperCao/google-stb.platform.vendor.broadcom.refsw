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
* FILENAME: $Workfile$
*
* DESCRIPTION:
*   ZDP Transaction common definitions.
*
* $Revision$
* $Date$
*
*****************************************************************************************/


#ifndef _BB_ZBPRO_ZDP_TRANSACTION_H
#define _BB_ZBPRO_ZDP_TRANSACTION_H


/************************* INCLUDES *****************************************************/
#include "bbRpc.h"
#include "bbZbProZdoSapTypesDiscoveryManager.h"
#include "bbZbProZdoSapTypesActiveEp.h"
#include "bbZbProZdoSapTypesBindingManager.h"
#include "bbZbProZdoSapTypesNodeManager.h"
#include "bbZbProZdoSapTypesMgmtLqiHandler.h"
#include "bbZbProZdoSapTypesMgmtNwkUpdate.h"
#include "private/bbZbProZdpDefaultHandler.h"
#include "bbZbProApsData.h"
#include "bbZbProApsSapBindUnbind.h"
#include "bbZbProApsSapGetSet.h"
#include "bbZbProNwkSapTypesPermitJoining.h"
#include "bbZbProNwkSapTypesDiscovery.h"
#include "bbZbProNwkSapTypesJoin.h"
#include "bbZbProNwkSapTypesLeave.h"
#include "bbZbProNwkSapTypesGetSet.h"
#include "bbZbProZdoSapTypesMgmtBind.h"


/************************* DEFINITIONS **************************************************/
/**//**
 * \brief   Structure for ZDP Transaction descriptor.
 */
typedef struct _ZbProZdpTransaction_t
{
    /* Structured data with 32-bit alignment. */

    RpcTransaction_t                            rpcTransactionDescr;                /*!< Embedded descriptor of the RPC
                                                                                        Transaction. */
    union
    {
        ZbProZdoDefaultRespParams_t             zdoDefaultRespParams;               /*!< Parameters of Default ZDO
                                                                                        Response to be sent back. */

        ZBPRO_ZDO_AddrResolvingReqParams_t      zdoAddrResolvingReqParams;          /*!< Parameters of received ZDO
                                                                                        Address Resolving (NWK_Addr or
                                                                                        IEEE_Addr) Request. */

        ZBPRO_ZDO_ActiveEpReqParams_t           activeEpReqParam;                   /*!< Parameters of received ZDO
                                                                                        Active_EP Request. */

        SYS_DataPointer_t                       matchDescRespStorage;               /*!< Client side uses it to store
                                                                                        responses. */

        ZBPRO_ZDO_EndDeviceBindReqParams_t      zdoEndDeviceBindReqParams;          /*!< Parameters of received ZDO
                                                                                        End_Device_Bind Request. */

        ZBPRO_ZDO_BindUnbindReqParams_t         zdoBindUnbindReqParams;             /*!< Parameters of received ZDO Bind
                                                                                        or Unbind Request. */

        ZBPRO_ZDO_MgmtLqiReqParams_t            zdoMgmtLqiReqParams;                /*!< Parametes of recieved ZDO
                                                                                                 Mgmt_Lqi_Req. */

        ZBPRO_ZDO_MgmtLeaveReqParams_t          zdoMgmtLeaveReqParams;              /*!< Parameters of received ZDO
                                                                                        Mgmt_Leave Request. */

        ZBPRO_ZDO_MgmtPermitJoiningReqParams_t  zdoMgmtPermitJoiningReqParams;      /*!< Parameters of received ZDO
                                                                                        Mgmt_Permit_Joining Request. */

        ZBPRO_ZDO_MgmtNwkUpdateReqParams_t      zdoMgmtNwkUpdateReqParams;          /*!< Parameters of received ZDO
                                                                                        Mgmt_NWK_Update Request. */

        ZBPRO_ZDO_MgmtBindReqParams_t           zdoMgmtBindReqParams;               /*!< Parameters of recieved ZDO
                                                                                         Mgmt_Bind request       */

    }                                           remoteZdoReqParams;                 /*!< Embedded structure for parsed
                                                                                        parameters of received Remote
                                                                                        ZDO Request. */

    ZBPRO_APS_DataReqDescr_t                    apsDataReqDescr;                    /*!< Embedded descriptor of the
                                                                                        APSDE-DATA.request. ZDP
                                                                                        Dispatcher performs final
                                                                                        freeing of payload on
                                                                                        transaction killing. If inside
                                                                                        single Server Transaction
                                                                                        multiple data frames (Server
                                                                                        Responses) are sent, the
                                                                                        corresponding Service Handler
                                                                                        shall decide if to free the
                                                                                        previous payload and allocate
                                                                                        the next one for each data frame
                                                                                        sent, or to reuse the same
                                                                                        payload for all of them. */
    union
    {
        ZBPRO_ZDO_Status_t                      respondWithStatus;                  /*!< Failure status to be reported
                                                                                        in remote server response. */

        ZBPRO_APS_BindUnbindReqDescr_t          apsBindUnbindReqDescr;              /*!< Embedded descriptor of the
                                                                                        APSME-(UN)BIND.request. */

        ZBPRO_APS_SetReqDescr_t                 apsSetReqDescr;                     /*!< Embedded descriptor of the
                                                                                        APSME-SET.request. */

        ZBPRO_NWK_PermitJoiningReqDescr_t       nwkPermitJoiningReqDescr;           /*!< Embedded descriptor of the
                                                                                        NLME-PERMIT-JOINING.request. */

        ZBPRO_NWK_EDScanReqDescr_t              nwkEdScanReqDescr;                  /*!< Embedded descriptor of the
                                                                                        NLME-ED-SCAN.request. */

        ZBPRO_NWK_JoinReqDescr_t                nwkJoinReqDescr;                    /*!< Embedded descriptor of the
                                                                                        NLME-JOIN.request. */

        ZBPRO_NWK_LeaveReqDescr_t               nwkLeaveReqDescr;                   /*!< Embedded descriptor of the
                                                                                        NLME-LEAVE.request. */

        ZBPRO_NWK_SetReqDescr_t                 nwkSetReqDescr;                     /*!< Embedded descriptor of the
                                                                                        NLME-SET.request. */
        struct
        {
            SYS_TimeoutTask_t                   timerDescr;                         /*!< Embedded descriptor of multi-
                                                                                        purpose transaction timer. May
                                                                                        be used for arranging delays or
                                                                                        custom timeouts within
                                                                                        transaction execution flow. */

            SYS_SchedulerTaskDescriptor_t       taskDescr;                          /*!< Embedded descriptor of multi-
                                                                                        purpose transaction task. May be
                                                                                        used in conjunction with
                                                                                        transaction timer or for
                                                                                        arranging transaction FSM. */
        };
    };

    /* 32-bit data. */

    const ZBPRO_APS_DataConfParams_t           *apsDataConfParams;                  /*!< Pointer to the
                                                                                        APSDE-DATA.confirm parameters.
                                                                                        Service doesn't have to reset
                                                                                        this field to NULL after
                                                                                        processing of each confirmation
                                                                                        in the case of multiple Server
                                                                                        Responses sent by it within
                                                                                        single Server Transaction. */
    /* 8-bit data. */

    Bool8_t                                     isZdoConfIssued;                    /*!< TRUE if Local ZDO Confirmation
                                                                                        was already issued; FALSE if it
                                                                                        wasn't yet. This field shall be
                                                                                        set to TRUE by the linked
                                                                                        Service Handler as soon as Local
                                                                                        ZDO Confirmation is issued by
                                                                                        it. Next time on attempt to
                                                                                        issue Local ZDO Confirmation it
                                                                                        will be canceled because this
                                                                                        flag is equal to TRUE.
                                                                                        Validation and assignment of
                                                                                        this flag shall be implemented
                                                                                        directly inside the function
                                                                                        responsible for issuing the
                                                                                        Local ZDO Confirmation. */
} ZbProZdpTransaction_t;


#endif /* _BB_ZBPRO_ZDP_TRANSACTION_H */
