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
* FILENAME: $Workfile: trunk/stack/common/Mailbox/src/bbMailService.c $
*
* DESCRIPTION:
*       implementation of mailbox service module
*
* $Revision: 3612 $
* $Date: 2014-09-17 09:29:25Z $
*
****************************************************************************************/
/************************* INCLUDES ****************************************************/
#include "bbMailAPI.h"
#include "bbMailService.h"
#include "private/bbMailPrivateService.h"
#include "private/bbMailPrivateAdapter.h"
#include "private/bbMailPrivateClient.h"
#include "private/bbMailPrivateServer.h"

/************************* IMPLEMENTATION **********************************************/
BITMAP_DECLARE(mailboxIndRoutingBitmap, FINAL_PUBLIC_FID);

#define ROUTING_SET_TO_STACK(fId)    (BITMAP_SET(mailboxIndRoutingBitmap, fId))
#define ROUTING_SET_TO_MAILBOX(fId)  (BITMAP_CLR(mailboxIndRoutingBitmap, fId))
#define ROUTING_IS_TO_STACK(fId)     (BITMAP_ISSET(mailboxIndRoutingBitmap, fId))
#define ROUTING_IS_TO_MAILBOX(fId)   (!BITMAP_ISSET(mailboxIndRoutingBitmap, fId))

#define ROUTING_RESET()  (BITMAP_RESET(mailboxIndRoutingBitmap))


static const MailServiceFunctionInfo_t mailboxMemoryFunctionTable[] =
{
#define FILL_REQ_PART__HAS_CB__NO_PARAMS(desc_name, parameters_name, param_payload)  \
            .reqCallbackOffset      = offsetof(desc_name, callback),                 \
            .reqParametersLength    = 0,                                             \
            .reqParametersOffset    = 0,                                             \
            .reqDataPointerOffset   = MAIL_INVALID_OFFSET,
#define FILL_REQ_PART__HAS_CB__NO_DATA(desc_name, parameters_name, param_payload)    \
            .reqCallbackOffset      = offsetof(desc_name, callback),                 \
            .reqParametersLength    = sizeof(parameters_name),                       \
            .reqParametersOffset    = offsetof(desc_name, params),                   \
            .reqDataPointerOffset   = MAIL_INVALID_OFFSET,
#define FILL_REQ_PART__HAS_CB__HAS_DATA(desc_name, parameters_name, param_payload)   \
            .reqCallbackOffset      = offsetof(desc_name, callback),                 \
            .reqParametersLength    = sizeof(parameters_name),                       \
            .reqParametersOffset    = offsetof(desc_name, params),                   \
            .reqDataPointerOffset   = offsetof(parameters_name, param_payload),
#define FILL_REQ_PART__NO_CB__NO_PARAMS(desc_name, parameters_name, param_payload)   \
            .reqCallbackOffset      = MAIL_INVALID_OFFSET,                           \
            .reqParametersLength    = 0,                                             \
            .reqParametersOffset    = 0,                                             \
            .reqDataPointerOffset   = MAIL_INVALID_OFFSET,
#define FILL_REQ_PART__NO_CB__NO_DATA(desc_name, parameters_name, param_payload)     \
            .reqCallbackOffset      = MAIL_INVALID_OFFSET,                           \
            .reqParametersLength    = sizeof(parameters_name),                       \
            .reqParametersOffset    = 0,                                             \
            .reqDataPointerOffset   = MAIL_INVALID_OFFSET,
#define FILL_REQ_PART__NO_CB__HAS_DATA(desc_name, parameters_name, param_payload)    \
            .reqCallbackOffset      = MAIL_INVALID_OFFSET,                           \
            .reqParametersLength    = sizeof(parameters_name),                       \
            .reqParametersOffset    = 0,                                             \
            .reqDataPointerOffset   = offsetof(parameters_name, param_payload),

#define FILL_CONF_PART__NO_CONFIRM(confirm_name, confirm_payload)               \
            .confParametersLength   = 0,                                        \
            .confDataPointerOffset  = MAIL_INVALID_OFFSET,
#define FILL_CONF_PART__NO_DATA(confirm_name, confirm_payload)                  \
            .confParametersLength   = sizeof(confirm_name),                     \
            .confDataPointerOffset  = MAIL_INVALID_OFFSET,
#define FILL_CONF_PART__HAS_DATA(confirm_name, confirm_payload)                 \
            .confParametersLength   = sizeof(confirm_name),                     \
            .confDataPointerOffset  = offsetof(confirm_name, confirm_payload),


#define TABLE_ENTRY(function_name, enum_name, desc_mod, desc_name, params_mod, parameters_name, param_payload, confirm_mod, confirm_name, confirm_payload) \
        {                                                                                           \
            .function           = (MailPublicFunction_t)function_name,                              \
            .id                 = enum_name,                                                        \
            FILL_REQ_PART__##desc_mod##__##params_mod(desc_name, parameters_name, param_payload)    \
            FILL_CONF_PART__##confirm_mod(confirm_name, confirm_payload)                            \
        },

#define ANY_DIRECTION(...) TABLE_ENTRY(__VA_ARGS__)
#define HOST_TO_STACK(...) TABLE_ENTRY(__VA_ARGS__)
#define STACK_TO_HOST(...) TABLE_ENTRY(__VA_ARGS__)

#include "bbMailFunctionList.h"

#undef TABLE_ENTRY
#undef FILL_REQ_PART__HAS_CB__NO_PARAMS
#undef FILL_REQ_PART__HAS_CB__NO_DATA
#undef FILL_REQ_PART__HAS_CB__HAS_DATA
#undef FILL_REQ_PART__NO_CB__NO_PARAMS
#undef FILL_REQ_PART__NO_CB__NO_DATA
#undef FILL_REQ_PART__NO_CB__HAS_DATA
#undef FILL_CONF_PART__NO_CONFIRM
#undef FILL_CONF_PART__NO_DATA
#undef FILL_CONF_PART__HAS_DATA
};

const MailServiceFunctionInfo_t *Mail_ServiceGetFunctionInfo(MailFID_t fId)
{
    uint16_t lo = 0;
    uint16_t mid = 0;
    uint16_t hi = ARRAY_SIZE(mailboxMemoryFunctionTable);
    while (lo <= hi)
    {
        mid = lo + (hi - lo) / 2;
        if      (fId < mailboxMemoryFunctionTable[mid].id)
            hi = mid - 1;
        else if (fId > mailboxMemoryFunctionTable[mid].id)
            lo = mid + 1;
        else
            break;
    }
    if (fId != mailboxMemoryFunctionTable[mid].id)
        SYS_DbgHalt(MAILBOX_SERVICE__FUNCTION_WITH_GIVED_IN_DOES_NOT_DEFINED_INTO_MAILBOX);
    return &mailboxMemoryFunctionTable[mid];
}

/************************************************************************************//**
    \brief Initialize mail service.
    \param[in] mail - mailbox descriptor.
****************************************************************************************/
void Mail_ServiceInit()
{
#if defined(_DEBUG_COMPLEX_)
    /* NOTE: This code checks mandatory conditions for the fast search algorithm. */
    {
        uint16_t lastId = INCORRECT_REQ_ID;
        const uint16_t tableSize = ARRAY_SIZE(mailboxMemoryFunctionTable);
        for (uint16_t i = 0; i < tableSize; ++i)
        {
            SYS_DbgAssertComplex(lastId != mailboxMemoryFunctionTable[i].id, MAILBOX_SERVICE__TABLE_CAN_NOT_KEEP_SEVERAL_DEFINITION_FOR_ONE_FID__PLEASE_CHECK_FUNCTION_LIST);
            SYS_DbgAssertComplex(lastId < mailboxMemoryFunctionTable[i].id, MAILBOX_SERVICE__FUNCTION_MUST_BE_DEFINED_IN_ASCENDING_ORDER__PLEASE_CHECK_FUNCTION_LIST);
            lastId = mailboxMemoryFunctionTable[i].id;
        }
    }
#endif

    ROUTING_RESET();

    mailClientInit();
    mailServerInit();
    mailAdapterInit();

#ifdef MAILBOX_STACK_SIDE
    Mail_TestEngineSendHello();
#endif
}

/************************************************************************************//**
    \brief Helper function for initialize mail service. Used at testing.
    \param[in] handler - handler
****************************************************************************************/
WEAK void Mail_RequestHandler(MailFID_t fId, MailPublicFunction_t handler, void *req)
{
    (void)fId;
    handler(req);
}

/************************************************************************************//**
    \brief This function envoke application callback.
    \param[in] offset - callback offset.
    \param[in] req - request poiter.
    \param[in] confirm - the confirmation structure pointer.
****************************************************************************************/
WEAK void Mail_CallbackHandler(MailFID_t fId, const ConfirmCall_t callback, void *req, void *confirm)
{
    (void)fId;
    callback(req, confirm);
}


bool Mail_RoutingChange(MailFID_t fId, MailRouteDirection_t routeDirection)
{
    if (fId >= FINAL_PUBLIC_FID)
        return false;

    switch(routeDirection)
    {
        case MAIL_ROUTE_TO_MAILBOX:
            ROUTING_SET_TO_MAILBOX(fId);
            break;
        case MAIL_ROUTE_TO_STACK:
            ROUTING_SET_TO_STACK(fId);
            break;
        default:
            SYS_DbgAssertComplex(false, HALT_Mail_RoutingChange_IncorrectRouteDirection);
            return false;
    }

    return true;
}

bool Mail_RoutingChangeList(MailFID_t const * const fIdList, uint16_t fIdAmount, MailRouteDirection_t routeDirection)
{
    for(uint16_t i=0; i<fIdAmount; ++i)
    {
        if (!Mail_RoutingChange(fIdList[i], routeDirection))
            return false;
    }
    return true;
}

bool Mail_IsReadyToSerialize()
{
    return (NULL != mailClientFindEmptyPendingTableEntry());
}

void Mail_Serialize(MailFID_t fId, void *req)
{
    mailClientSerialize(fId, (uint8_t *)req);
}


#ifdef MAILBOX_UNIT_TEST
# define CREATE_FUNCTION(wrapper_name, arg_type, enum_name)  \
    WEAK void wrapper_name(arg_type *_arg) {Mail_Serialize(enum_name, _arg);}
#else /* MAILBOX_UNIT_TEST */

# ifdef MAILBOX_STACK_SIDE
#  define CREATE_FUNCTION(wrapper_name, arg_type, enum_name)  \
    extern void wrapper_name##_internal(arg_type * _arg);   \
    WEAK void wrapper_name(arg_type *_arg)                  \
    {                                                       \
        if (ROUTING_IS_TO_MAILBOX(enum_name))               \
            Mail_Serialize(enum_name, _arg);                \
        else                                                \
            wrapper_name##_internal(_arg);                  \
    }
# endif /* MAILBOX_STACK_SIDE */

# ifdef MAILBOX_HOST_SIDE
#  define CREATE_FUNCTION(wrapper_name, arg_type, enum_name)  \
    WEAK void wrapper_name(arg_type *_arg) {Mail_Serialize(enum_name, _arg);}
# endif /* MAILBOX_HOST_SIDE */

#endif /* MAILBOX_UNIT_TEST */

#define MAILBOX_CALL__NO_CB(function_name, enum_name, desc_name, parameters_name)   CREATE_FUNCTION(function_name, parameters_name, enum_name)
#define MAILBOX_CALL__HAS_CB(function_name, enum_name, desc_name, parameters_name)  CREATE_FUNCTION(function_name, desc_name, enum_name)

#ifdef MAILBOX_UNIT_TEST
#define MAKE_WRAPPER_FUNCTION(function_name, enum_name, desc_mod, desc_name, params_mod, parameters_name, param_payload, confirm_mod, confirm_name, confirm_payload) \
    MAILBOX_CALL__##desc_mod(function_name##_Call, enum_name, desc_name, parameters_name)
#else
#define MAKE_WRAPPER_FUNCTION(function_name, enum_name, desc_mod, desc_name, params_mod, parameters_name, param_payload, confirm_mod, confirm_name, confirm_payload) \
    MAILBOX_CALL__##desc_mod(function_name, enum_name, desc_name, parameters_name)
#endif

#define ANY_DIRECTION(...) /*NOTE: This type functions can be called only through Mail_Serialize(...) */

#ifdef MAILBOX_HOST_SIDE
# define HOST_TO_STACK(...) MAKE_WRAPPER_FUNCTION(__VA_ARGS__)
#endif /* MAILBOX_HOST_SIDE */

#ifdef MAILBOX_STACK_SIDE
# define STACK_TO_HOST(...) MAKE_WRAPPER_FUNCTION(__VA_ARGS__)
#endif /* MAILBOX_STACK_SIDE */

#include "bbMailFunctionList.h"

#undef MAKE_WRAPPER_FUNCTION
#undef MAILBOX_CALL__HAS_CB
#undef MAILBOX_CALL__NO_CB
#undef CREATE_FUNCTION