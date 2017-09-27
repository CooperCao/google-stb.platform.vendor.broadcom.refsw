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

/*******************************************************************************
 *
 * DESCRIPTION:
 *
 *
*******************************************************************************/

#ifndef _MAIL_SERVICE_H
#define _MAIL_SERVICE_H

/************************* INCLUDES ****************************************************/
#include "bbSysBasics.h"
#include "bbMailFunctionList.h"

/************************* TYPES *******************************************************/
typedef enum _MailRouteDirection_t
{
    MAIL_ROUTE_TO_MAILBOX = 0,
    MAIL_ROUTE_TO_STACK = 1
} MailRouteDirection_t;


/**//**
 * \brief The public function pointer prototype.
 */
typedef void (* MailPublicFunction_t)(void *req);

/**//**
 * \brief The callback pointer prototype.
 */
typedef void(* ConfirmCall_t)(void *req, void *confirm);

/**//**
* \brief Information about public API function.
*/
typedef struct _MailServiceFunctionInfo_t
{
    MailPublicFunction_t    function;
    MailFID_t               id;

    uint8_t                 reqCallbackOffset;
    uint8_t                 reqParametersLength;
    uint8_t                 reqParametersOffset;
    uint8_t                 reqDataPointerOffset;

    uint8_t                 confParametersLength;
    uint8_t                 confDataPointerOffset;
} MailServiceFunctionInfo_t;

/************************* PROTOTYPES **************************************************/
/************************************************************************************//**
    \brief Gets information related to a public function.
    \param[in] fId - function number.
 ****************************************************************************************/
const MailServiceFunctionInfo_t *Mail_ServiceGetFunctionInfo(MailFID_t fId);

/************************************************************************************//**
    \brief Initialize mail service.
****************************************************************************************/
void Mail_ServiceInit(void);


/************************************************************************************//**
    \brief Deinitialize mail service.
****************************************************************************************/
void Mail_ServiceDeinit(void);
/************************************************************************************//**
    \brief Returns true if mailbox ready to serialize a one more transaction and false otherwise.
 ****************************************************************************************/
bool Mail_IsReadyToSerialize(void);

/************************************************************************************//**
    \brief Starts a serialization process.
    \param[in] fId - function number.
    \param[in] req - request pointer.
****************************************************************************************/
void Mail_Serialize(MailFID_t fId, void *req);

/************************************************************************************//**
    \brief Calls the application handler. Implementation can redefined in suitable way for application.
****************************************************************************************/
void Mail_RequestHandler(MailFID_t fId, MailPublicFunction_t handler, void *req);

/************************************************************************************//**
    \brief Calls the application callback handler. Implementation can redefined in suitable way for application.
****************************************************************************************/
void Mail_CallbackHandler(MailFID_t fId, const ConfirmCall_t callback, void *req, void *confirm);

/************************************************************************************//**
    \brief Public primitive for changing the routing direction for Indication.
    \param[in]  fId  - indication identifier,
    \param[in]  routeToStack - if true, new routing will be TO STACK, else - TO MAILBOX.
    \return     true, if change routing procedure was finished successfully.
****************************************************************************************/
bool Mail_RoutingChange(MailFID_t fId, MailRouteDirection_t routeDirection);

/************************************************************************************//**
    \brief Public primitive for changing the routing direction for list of Indications.
    \param[in]  fIdList  - pointer to list of indications identifiers,
    \param[in]  fIdCount - amount of the indications in the /p fIdList list.
    \param[in]  routeToStack - if true, new routing will be TO STACK, else - TO MAILBOX.
    \return     true, if change routing procedure was finished successfully.
****************************************************************************************/
bool Mail_RoutingChangeList(MailFID_t const * const fIdList, uint16_t fIdAmount, MailRouteDirection_t routeDirection);

#endif /* _MAIL_SERVICE_H */

/* eof bbMailService.h */