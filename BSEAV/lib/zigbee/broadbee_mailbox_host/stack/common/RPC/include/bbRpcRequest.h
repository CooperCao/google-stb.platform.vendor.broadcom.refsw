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
*   RPC Request common definitions.
*
* $Revision$
* $Date$
*
*****************************************************************************************/


#ifndef _BB_RPC_REQUEST_H
#define _BB_RPC_REQUEST_H


/************************* INCLUDES *****************************************************/
#include "bbRpcService.h"
#include "bbSysQueue.h"
#include "bbSysPayload.h"


/************************* DEFINITIONS **************************************************/
/**//**
 * \brief   Data type for Transaction Sequence Number.
 */
typedef uint32_t  RpcTransSeqNum_t;


/**//**
 * \brief   Structure for the service field of Local Client Request object.
 * \details
 *  This structure shall be embedded as a service field into each application specific
 *  Local Client Request structure.
 */
struct _RpcLocalRequest_t
{
    /* 32-bit data. */

    SYS_QueueElement_t  element;            /*!< Queue element. */

    RpcServiceId_t      serviceId;          /*!< RPC Service identifier (profile and cluster, in particular). */

    RpcTransSeqNum_t    forcedTsn;          /*!< Forced Transaction Sequence Number. */

    /* 8-bit data. */

    Bool8_t             isUnsolResp;        /*!< TRUE for server unsolicited response, FALSE for client request. */

    Bool8_t             forceTsn;           /*!< TRUE if shall force TSN; FALSE if shall generate new TSN. */
};


/**//**
 * \brief   Union for the Remote Client Request handler object.
 * \details
 *  This union shall be assigned by the application-specific RPC Dispatcher Wrapper when
 *  calling the common RPC Dispatcher received-data-indication handling method. Either
 *  pointer to the whole data-indication parameters structure, or just the data-indication
 *  payload descriptor shall be assigned. Later it may be reassigned with the application-
 *  specific payload if needed.
 * \details
 *  This union is embedded into the Service Transaction service field as-is and provides
 *  the activated application-specific Service with the indicated data frame parameters or
 *  only its payload (according to need of particular application and service), or the
 *  application-specific data saved between Service Handler calls. The called Service is
 *  considered to be well informed within its own application what type of data indeed is
 *  transferred to it with this union via the common RPC Dispatcher from its parent
 *  application-specific RPC Dispatcher Wrapper - either pointer to parameters structure
 *  or the dynamic payload descriptor of particular content.
 * \note
 *  The \c params points to an object allocated in the program stack. It may be used only
 *  in the same execution flow with the first call of the Service for the received data
 *  indication. If the called Service needs to keep these parameters for its deferred
 *  activity or some derived data, it shall save them into the dynamic memory and reassign
 *  this union with the descriptor of such a payload. If the called Service needs to keep
 *  only the data payload passed within the data-indication parameters, it shall drop the
 *  parameters object and reassign this union with the descriptor of such a data-
 *  indication payload.
 * \note
 *  The called Service is responsible for freeing the dynamic memory allocated by itself
 *  or by the lower-level layer for the received data-indication payload either specified
 *  with the \p payload descriptor of this union, or contained in the parameters object
 *  pointed with the \p params field. Nevertheless, if RPC Dispatcher has not succeeded in
 *  binding received Remote Server Response to a pending Local Client Request Transaction,
 *  or allocating new Server Transaction for the received Remote Client Request (or
 *  Unsolicited Server Response), the RPC Dispatcher will return 'Not processed' (FALSE)
 *  result to the calling application-specific RPC Dispatcher Wrapper, and the last one
 *  shall free all the dynamic memory linked to Remote Request parameters itself.
 */
union _RpcRemoteRequest_t
{
    /* Structured / 32-bit data. */

    void              *params;              /*!< Pointer to indicated remote Client Request Parameters. */

    SYS_DataPointer_t  payload;             /*!< Descriptor of indicated remote Client Request Payload or application-
                                                specific Payload saved between Service Handler calls. */

    uint32_t           plain;               /*!< Plain data. */
};

#ifndef _UNIT_TEST_
/*
 * Validate that the size of the whole union is equal to the size of its plain field.
 */
//SYS_DbgAssertStatic(sizeof(union _RpcRemoteRequest_t) == FIELD_SIZE(union _RpcRemoteRequest_t, plain));
#endif


#endif /* _BB_RPC_REQUEST_H */
