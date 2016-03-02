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
 *****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
//#include "zigbee_types.h"
#include "bbMailService.h"
#include "bbMailAPI.h"
#include "bbMailClient.h"
#include "zigbee_rf4ce_registration.h"
#include "ha_registration.h"
#include "bbMailPowerFilterKey.h"
#include "bbSysDbg.h"
#ifndef BYPASS_RPC
#include "zigbee_rpc.h"
#endif
#include "zigbee_api.h"
#include "zigbee_common.h"
#ifndef BYPASS_RPC
#include "zigbee_rpc.h"
#include "zigbee_rpc_frame.h"
#include "zigbee_mbox_frame.h"
#endif
#ifdef SERVER
#include "zigbee_ioctl.h"
#endif
//#include "bbHalSharedFifo.h"
#include "bbMailTypes.h"
#include "bbMailClientTable.h"
#include "bbMailServerTable.h"
#ifdef SERVER
#ifdef BYPASS_RPC
    #include "zigbee.h"
#else
    #include "zigbee_rpc_server_priv.h"
    #include "zigbee_socket_server.h"
#endif
#else
    #include "zigbee_socket_client.h"
#endif

#ifndef BYPASS_RPC
#ifdef SERVER
    zigbee_api_cb_t zigbee_api_cb;
    zigbee_api_rf4ce_cb_t zigbee_api_rf4ce_cb[MAX_SOCKETS];
    zigbee_api_cb_Req_t   zigbee_api_req_cb[MAX_SOCKETS];
#endif
#endif

/******************************************************************************
    Zigbee Request functions runs from the caller thread
******************************************************************************/

#if 0
/* 4.2.6 */
void RF4CE_CounterExpiredInd( RF4CE_PairingReferenceIndParams_t *indication) {}
void RF4CE_GDP_PullAttributesInd( RF4CE_GDP_HostAttributesReqDescr_t *request) {}
void RF4CE_GDP_PushAttributesInd( RF4CE_GDP_HostAttributesReqDescr_t *request) {}
void RF4CE_GDP_GetAttributesInd( RF4CE_GDP_HostAttributesReqDescr_t *request) {}
void RF4CE_GDP_SetAttributesInd( RF4CE_GDP_HostAttributesReqDescr_t *request) {}
void RF4CE_ZRC_UnpairInd( RF4CE_PairingReferenceIndParams_t *indication) {}
void RF4CE_ZRC_GetAttributesReq(RF4CE_GDP_AttributeDescr_t *request) {}
void RF4CE_ZRC_SetAttributesReq(RF4CE_GDP_AttributeDescr_t *request) {}
void RF4CE_ZRC_PushAttributesReq(RF4CE_GDP_AttributeDescr_t *request) {}
void RF4CE_ZRC_PullAttributesReq(RF4CE_GDP_AttributeDescr_t *request) {}
void RF4CE_UnpairReq(RF4CE_UnpairReqDescr_t *request) {}
void RF4CE_ZRC_KeyExchangeReq( RF4CE_GDP_KeyExchangeReqDescr_t *request) {}
#endif

#define PAYLOAD_OFFSET_C2S_RF4CE_StartReq_REQ 0
#define PAYLOAD_OFFSET_C2S_RF4CE_StartReq_REQ_data 1

#define PAYLOAD_OFFSET_S2C_RF4CE_StartReq_callback_REQ 0
#define PAYLOAD_OFFSET_S2C_RF4CE_StartReq_callback_REQ_data 1
#define PAYLOAD_OFFSET_S2C_RF4CE_StartReq_callback_CONF 3
#define PAYLOAD_OFFSET_S2C_RF4CE_StartReq_callback_CONF_data 4

#define WORDALIGN(size)  (((size) + 3 ) / 4)


typedef struct _list_head {
    struct _list_head *next, *prev;
}list_head;

static inline void INIT_LIST_HEAD(list_head *list)
{
    list->next = list;
    list->prev = list;
}

static inline int list_empty(const list_head *head)
{
    return head->next == head;
}

static inline void list_add(list_head *new, list_head *head)
{
    head->prev->next = new;
    new->next = head;
    new->prev = head->prev;
    head->prev = new;
}

static inline void list_del(list_head *entry)
{
    entry->prev->next = entry->next;
    entry->next->prev = entry->prev;
}

#define INIT_CLIENT_LIST_INFO(info, cInd, sInd, origCallback)                                                   \
    INIT_LIST_HEAD(&(info)->list);                                                                              \
    info->clientInd = cInd;                                                                                     \
    info->serverInd = sInd;                                                                                     \
    info->origIndCallback = origCallback;

#define INIT_SERVER_LIST_INFO(info, cReq, sReq, msgId, sck)                                                     \
    INIT_LIST_HEAD(&(info)->list);                                                                              \
    info->clientRequest = cReq;                                                                                 \
    info->serverRequest = sReq;                                                                                 \
    info->fId = msgId;                                                                                          \
    info->socket = sck;


#define list_for_each(pos, head) \
    for (pos = (head)->next; pos != (head); pos = pos->next)

typedef struct _List_ServerReq_Info_t{
    list_head       list;
    unsigned int    clientRequest;
    unsigned int    serverRequest;
    unsigned int    socket;
    void            *callback;
    uint16_t        fId;
    char            clientRequestData[0];
}List_ServerReq_Info_t;

typedef struct _List_ClientIndication_Info_t{
    list_head   list;
    int         clientInd;
    int         serverInd;
    int         origIndCallback;
}List_ClientIndication_Info_t;

/* All the code templates to create function for rpc server and client side */

/*********************************************************************************/

/* note: because the function name##_Call implemented by mailbox adapter layer is
 * a asynchronous call, for the first parameter request which is pointer, it's
 * necessary to malloc it instead of using the stack allocated area.
 */
typedef uint8_t NoAppropriateType_t;

#ifdef BYPASS_RPC
#define CREATE_CLIENT_REQUEST_API_FUNCTION(fId, name, reqType, confType, rpcMsgCode)
#else
#define CREATE_CLIENT_REQUEST_API_FUNCTION(fId, name, reqType, confType, rpcMsgCode)    \
    CREATE_CLIENT_REQUEST_CALLBACK_FUNCTION(fId, name, reqType, confType)               \
    CREATE_CLIENT_REQUEST_FUNCTION(fId, name, reqType, rpcMsgCode)
#endif


/************************************************************************************//**
    \brief Find first empty entry in postponed call table.
    \param[in] client - client module descriptor.

    \return Pointer to the found entry.
****************************************************************************************/
#ifdef SERVER

#define CREATE_SERVER_REQUEST_API_FUNCTION(fId, name, reqType, confType, rpcMsgCode)        \
        CREATE_SERVER_REQUEST_CALLBACK_FUNCTION(fId, name, reqType, confType, rpcMsgCode)   \
        CREATE_SERVER_REQUEST_FUNCTION(fId, name, reqType, confType)

#define CREATE_SERVER_LOCAL_REQUEST_API_FUNCTION(fId, name, reqType, confType)  \
        CREATE_SERVER_LOCAL_REQUEST_CALLBACK_FUNCTION(fId, name, reqType, confType)   \
        CREATE_SERVER_LOCAL_REQUEST_FUNCTION(fId, name, reqType, confType)



void rf4ce_ZRC2_Set_Default_Check_Validation_Period(uint8_t pairingRef)
{
    uint8_t status = 0;
    RF4CE_ZRC2_SetAttributeId_t *setAttribute;

    RF4CE_ZRC2_SetAttributesReqDescr_t req = {0};
    void rf4ce_ZRC2_Set_Default_Check_Validation_Period_Callback(RF4CE_ZRC2_SetAttributesReqDescr_t *request, RF4CE_ZRC2_SetAttributesConfParams_t *conf)
    {
        printf("rf4ce_ZRC2_Set_Default_Check_Validation_Period_Callback\n");
        SYS_FreePayload(&request->params.payload);
        if(0 == conf->status)
            status = 1;
        //free(conf);
    }
    /* CHECK_VALIDATION_PERIOD */
    int size = sizeof(RF4CE_ZRC2_SetAttributeId_t) - 1 + RF4CE_ZRC2_AUTO_CHECK_VALIDATION_PERIOD_SIZE;
    SYS_MemAlloc(&req.params.payload, size);

    uint8_t * const buffSetAttribute = malloc(size);
    setAttribute = (RF4CE_ZRC2_SetAttributeId_t*)buffSetAttribute;
    setAttribute->id.attributeId = 0x87;
    setAttribute->id.index.index = 0;
    setAttribute->size = RF4CE_ZRC2_AUTO_CHECK_VALIDATION_PERIOD_SIZE;
    *(uint16_t*)setAttribute->value = 500;

    SYS_CopyToPayload(&req.params.payload, 0, buffSetAttribute, size);
    free(buffSetAttribute);
    req.params.pairingRef = pairingRef;
    req.callback = rf4ce_ZRC2_Set_Default_Check_Validation_Period_Callback;

    server_RF4CE_ZRC2_SetAttributesReq_local_call(&req);
    while(!status);
}


//extern MailDescriptor_t *mailDescriptorPtr;
extern void rpcRequestRepostHandler(SYS_SchedulerTaskDescriptor_t *const taskDescr);

static int availablePendingTableEntry()
{
    MailDescriptor_t *mail = mailDescriptorPtr;

    MailClientDescriptor_t *client = &mail->client;
    int available = 0;
    for (uint8_t i = 0; i < MAIL_CLIENT_MAX_AMOUNT_PENDING_CALLS; ++i)
        if (INCORRECT_REQ_ID == client->pendingTable[i].fId)
            available++;
    return available;
}

static list_head serverRequestListHeader = {
                    .next = &serverRequestListHeader,
                    .prev = &serverRequestListHeader,
};

static list_head serverPendingRequestListHeader = {
                    .next = &serverPendingRequestListHeader,
                    .prev = &serverPendingRequestListHeader,
};

static const SYS_SchedulerTaskHandler_t rpcRequestDispatchHandlers[2] =
{
    [0] = rpcRequestRepostHandler,
    NULL
};

static SYS_SchedulerTaskDescriptor_t requestDispatcherTask = {
                    .priority = SYS_SCHEDULER_ZBPRO_ZDO_PRIORITY,
                    .handlers = rpcRequestDispatchHandlers,
                    .handlersMask = 0
                    };

void rpcRequestRepostHandler(SYS_SchedulerTaskDescriptor_t *const taskDescr)
{
    if(!list_empty(&serverPendingRequestListHeader) && availablePendingTableEntry()){
        list_head *pos = serverPendingRequestListHeader.next;
        list_del(pos);
        List_ServerReq_Info_t *info = GET_PARENT_BY_FIELD(List_ServerReq_Info_t, list, pos);
        list_add(pos, &serverRequestListHeader);
        Mail_Serialize(mailDescriptorPtr, info->fId, (void*)info->serverRequest);
    }
}

#define CREATE_SERVER_LOCAL_REQUEST_FUNCTION(fId, name, reqType, confType)                                              \
    void server_##name##_local_call(reqType *req)                                                                       \
    {                                                                                                                   \
        reqType *request = (reqType *)malloc(sizeof(reqType));                                                          \
        memcpy((char*)request, req, sizeof(reqType));                                                                   \
        List_ServerReq_Info_t *info = malloc(sizeof(List_ServerReq_Info_t) + sizeof(reqType));                          \
        INIT_SERVER_LIST_INFO(info, (unsigned int)req, (unsigned int)request, fId, 0)                                   \
        memcpy((char*)info->clientRequestData, (char*)request, sizeof(reqType));                                        \
        info->callback = (void*)request->callback;                                                                      \
        request->callback = server_##name##_local_callback;                                                             \
        if(availablePendingTableEntry()){                                                                               \
            list_add(&info->list, &serverRequestListHeader);                                                            \
            name##_Call(request);                                                                                       \
        }else                                                                                                           \
            list_add(&info->list, &serverPendingRequestListHeader);                                                     \
    }


#define CREATE_SERVER_LOCAL_REQUEST_CALLBACK_FUNCTION(fId, name, reqType, confType)                                 \
    void server_##name##_local_callback(reqType  *request, confType *conf)                                          \
    {                                                                                                               \
        list_head *pos = 0;                                                                                         \
        int found = 0;                                                                                              \
        List_ServerReq_Info_t *info = 0;                                                                            \
        list_for_each(pos, &serverRequestListHeader){                                                               \
            if((info = GET_PARENT_BY_FIELD(List_ServerReq_Info_t, list, pos))->serverRequest == (unsigned int)request)       \
            {                                                                                                       \
                found = 1;                                                                                          \
                list_del(pos);                                                                                      \
                break;                                                                                              \
            }                                                                                                       \
        }                                                                                                           \
        if(!found){                                                                                                 \
            printf("can't found the origin pointer\n");                                                             \
            return;                                                                                                 \
        }                                                                                                           \
                                                                                                                    \
        ((void(*)(reqType*, confType*))info->callback)((reqType*)info->clientRequest, conf);                        \
        free(info);                                                                                                 \
        free(request);                                                                                              \
    }

#ifdef BYPASS_RPC
#define CREATE_SERVER_REQUEST_FUNCTION(fId, name, reqType, confType)                                                   \
    void name(reqType *request)                                                                                         \
    {                                                                                                                   \
        const MailClientParametersTableEntry_t *const reqInfo = mailClientTableGetAppropriateEntry(fId);                \
        if(MAIL_INVALID_PAYLOAD_OFFSET != reqInfo->dataPointerOffset)                                                   \
        {                                                                                                               \
            printf("hanging\n"); while(1);                                                                              \
        }                                                                                                               \
        List_ServerReq_Info_t *info = malloc(sizeof(List_ServerReq_Info_t) + sizeof(reqType));                          \
        INIT_SERVER_LIST_INFO(info, 0, (unsigned int)request, fId, 7)                                                   \
        memcpy((char*)info->clientRequestData, (char*)request, sizeof(reqType));                                        \
        if(reqInfo->callbackOffset != sizeof(reqType))                                                                  \
            *(int*)((uint8_t *)request + reqInfo->callbackOffset) = (int)(void(*)(reqType*, confType*))server_##name##_callback;\
        if(availablePendingTableEntry()){                                                                               \
            list_add(&info->list, &serverRequestListHeader);                                                            \
            name##_Call(request);                                                                                       \
        }else                                                                                                           \
            list_add(&info->list, &serverPendingRequestListHeader);                                                     \
    }
#else
#define CREATE_SERVER_REQUEST_FUNCTION(fId, name, reqType, confType)                                                    \
    void server_##name(unsigned int *buf, int socket)                                                                   \
    {                                                                                                                   \
        reqType *request = (reqType *)malloc(sizeof(reqType));                                                          \
        memcpy((char*)request, &buf[1], sizeof(reqType));                                                               \
        const MailClientParametersTableEntry_t *const reqInfo = mailClientTableGetAppropriateEntry(fId);                \
        if(MAIL_INVALID_PAYLOAD_OFFSET != reqInfo->dataPointerOffset)                                                   \
        {                                                                                                               \
            SYS_DataPointer_t *const dataPointer = (SYS_DataPointer_t *)(                                               \
                (uint8_t *)request + reqInfo->paramOffset + reqInfo->dataPointerOffset);                                \
            int size_payload = SYS_GetPayloadSize(dataPointer);                                                         \
            SYS_SetEmptyPayload(dataPointer);                                                                           \
            SYS_MemAlloc(dataPointer, size_payload);                                                                    \
            SYS_CopyToPayload(dataPointer, 0, (const void*)&buf[1 + WORDALIGN(sizeof(reqType))], size_payload);         \
        }                                                                                                               \
        List_ServerReq_Info_t *info = malloc(sizeof(List_ServerReq_Info_t) + sizeof(reqType));                          \
        INIT_SERVER_LIST_INFO(info, buf[0], (unsigned int)request, fId, socket)                                         \
        memcpy((char*)info->clientRequestData, (char*)request, sizeof(reqType));                                        \
        if(reqInfo->callbackOffset != sizeof(reqType))                                                                  \
            *(int*)((uint8_t *)request + reqInfo->callbackOffset) = (int)(void(*)(reqType*, confType*))server_##name##_callback;\
        if(availablePendingTableEntry()){                                                                               \
            list_add(&info->list, &serverRequestListHeader);                                                            \
            name##_Call(request);                                                                                       \
        }else                                                                                                           \
            list_add(&info->list, &serverPendingRequestListHeader);                                                     \
    }
#endif

/*
 *  In the callback funciton, we need to free the payload allocated in the request function
 */
#ifdef BYPASS_RPC
#define CREATE_SERVER_REQUEST_CALLBACK_FUNCTION(fId, name, reqType, confType, rpcMsgCode)                               \
    void server_##name##_callback(reqType  *request, confType *conf)                                                    \
    {                                                                                                                   \
            int size = 0;                                                                                               \
            int message_tx[MAX_MSG_SIZE_IN_WORDS];                                                                      \
            reqType request2;                                                                                           \
                                                                                                                        \
            printf("ZIGBEE_RPC_API:  invoking %s Callback\n", #name);                                                   \
            list_head *pos = 0;                                                                                         \
            int found = 0;                                                                                              \
            List_ServerReq_Info_t *info = 0;                                                                            \
            list_for_each(pos, &serverRequestListHeader){                                                               \
                if((info = GET_PARENT_BY_FIELD(List_ServerReq_Info_t, list, pos))->serverRequest == (int)request)       \
                {                                                                                                       \
                    found = 1;                                                                                          \
                    list_del(pos);                                                                                      \
                    break;                                                                                              \
                }                                                                                                       \
            }                                                                                                           \
            if(!found){                                                                                                 \
                printf("can't found the origin pointer\n");                                                             \
                return;                                                                                                 \
            }                                                                                                           \
                                                                                                                        \
            message_tx[0] = info->clientRequest;                                                                        \
            free(info);                                                                                                 \
            memcpy((char *)&request2, (char *)info->clientRequestData, sizeof(reqType));                                \
            const MailServerParametersTableEntry_t *const confirmInfo = mailServerTableGetAppropriateEntry(fId);        \
            if(MAIL_INVALID_PAYLOAD_OFFSET != confirmInfo->confDataPointerOffset){                                      \
                printf("hanging\n"); while(1);                                                                          \
            }                                                                                                           \
                                                                                                                        \
            const MailClientParametersTableEntry_t *const reqInfo = mailClientTableGetAppropriateEntry(fId);            \
            if(MAIL_INVALID_PAYLOAD_OFFSET != reqInfo->dataPointerOffset){                                              \
                printf("hanging\n"); while(1);                                                                          \
            }                                                                                                           \
            request2.callback(request, conf);                                                                           \
            SYS_SchedulerPostTask(&requestDispatcherTask, 0);                                                           \
    }
#else
#define CREATE_SERVER_REQUEST_CALLBACK_FUNCTION(fId, name, reqType, confType, rpcMsgCode)                               \
    void server_##name##_callback(reqType  *request, confType *conf)                                                    \
    {                                                                                                                   \
            int size = 0;                                                                                               \
            int message_tx[MAX_MSG_SIZE_IN_WORDS];                                                                      \
                                                                                                                        \
            printf("ZIGBEE_RPC_API:  invoking %s Callback\n", #name);                                                   \
            list_head *pos = 0;                                                                                         \
            int found = 0;                                                                                              \
            List_ServerReq_Info_t *info = 0;                                                                            \
            list_for_each(pos, &serverRequestListHeader){                                                               \
                if((info = GET_PARENT_BY_FIELD(List_ServerReq_Info_t, list, pos))->serverRequest == (int)request)       \
                {                                                                                                       \
                    found = 1;                                                                                          \
                    list_del(pos);                                                                                      \
                    break;                                                                                              \
                }                                                                                                       \
            }                                                                                                           \
            if(!found){                                                                                                 \
                printf("can't found the origin pointer\n");                                                             \
                return;                                                                                                 \
            }                                                                                                           \
                                                                                                                        \
            message_tx[0] = info->clientRequest;                                                                        \
            free(info);                                                                                                 \
            size += 1;                                                                                                  \
            memcpy((char *)&message_tx[size], (char*)info->clientRequestData, sizeof(reqType));                         \
            size += WORDALIGN(sizeof(reqType));                                                                         \
            message_tx[size] = (unsigned int)conf;                                                                      \
            size += 1;                                                                                                  \
            memcpy((char *)&message_tx[size], (char *)conf, sizeof(confType));                                          \
            size += WORDALIGN(sizeof(confType));                                                                        \
            const MailServerParametersTableEntry_t *const confirmInfo = mailServerTableGetAppropriateEntry(fId);        \
            if(MAIL_INVALID_PAYLOAD_OFFSET != confirmInfo->confDataPointerOffset){                                      \
                SYS_DataPointer_t *const dataPointer = (SYS_DataPointer_t *)                                            \
                                                    ((uint8_t *)conf + confirmInfo->confDataPointerOffset);             \
                int size_payload = SYS_GetPayloadSize(dataPointer);                                                     \
                SYS_CopyFromPayload((void*)&message_tx[size], dataPointer, 0, size_payload);                            \
                size += WORDALIGN(size_payload);                                                                        \
                if(SYS_CheckPayload(dataPointer))                                                                       \
                   SYS_FreePayload(dataPointer);                                                                        \
            }                                                                                                           \
                                                                                                                        \
            const MailClientParametersTableEntry_t *const reqInfo = mailClientTableGetAppropriateEntry(fId);            \
            if(MAIL_INVALID_PAYLOAD_OFFSET != reqInfo->dataPointerOffset){                                              \
                SYS_DataPointer_t *const reqDataPointer = (SYS_DataPointer_t *)(                                        \
                    (uint8_t *)request + reqInfo->paramOffset + reqInfo->dataPointerOffset);                            \
                if(SYS_CheckPayload(reqDataPointer))                                                                    \
                   SYS_FreePayload(reqDataPointer);                                                                     \
            }                                                                                                           \
            free(request);                                                                                              \
            Zigbee_Rpc_Send(message_tx, size, rpcMsgCode | RPC_RESPONSE, Zigbee_Socket_ServerSend, info->socket);       \
            SYS_SchedulerPostTask(&requestDispatcherTask, 0);                                                           \
    }
#endif
#endif

/************************************************************************************************************************
 * \brief The API template for client side.
 ************************************************************************************************************************/
#ifndef BYPASS_RPC
#define CREATE_CLIENT_REQUEST_FUNCTION(fId, name, reqType, rpcMsgCode)                                                  \
    void name(reqType *request)                                                                                         \
    {                                                                                                                   \
        unsigned int buf[RPC_MAX_PAYLOAD_SIZE_IN_WORDS];                                                                \
        int size_in_words = 0;                                                                                          \
        memset(&buf[0], 0, sizeof(buf));                                                                                \
        buf[0] = (unsigned int)request;                                                                                 \
        size_in_words += 1;                                                                                             \
        memcpy((char *)&buf[1], request, sizeof(reqType));                                                              \
        size_in_words += WORDALIGN(sizeof(reqType));                                                                    \
        const MailClientParametersTableEntry_t *const reqInfo = mailClientTableGetAppropriateEntry(fId);                \
        if(MAIL_INVALID_PAYLOAD_OFFSET != reqInfo->dataPointerOffset)                                                   \
        {                                                                                                               \
            SYS_DataPointer_t *const dataPointer = (SYS_DataPointer_t *)(                                               \
                (uint8_t *)request + reqInfo->paramOffset + reqInfo->dataPointerOffset);                                \
            int size_payload = SYS_GetPayloadSize(dataPointer);                                                         \
            SYS_CopyFromPayload((char*)&buf[size_in_words], dataPointer, 0, size_payload);                              \
            size_in_words += WORDALIGN(size_payload);                                                                   \
        }                                                                                                               \
        Zigbee_Rpc_Send(buf, size_in_words, rpcMsgCode, Zigbee_Socket_ClientSend, 0);                                   \
    }
#endif

/* To support the request caller and callback function are running in different context,                  */
/* The application request callback function should free confirmation and its payload, plus req if needed */

#ifndef BYPASS_RPC
#define CREATE_CLIENT_REQUEST_CALLBACK_FUNCTION(fId, name, reqType, confType)                                           \
    void client_##name##_callback(unsigned int *message_rx)                                                             \
    {                                                                                                                   \
        reqType req;                                                                                                    \
        confType *conf = malloc(sizeof(confType));                                                                      \
        memcpy((char *)&req, (char *)&message_rx[1], sizeof(req));                                                      \
        memcpy((char *)conf, (char *)&message_rx[2 + WORDALIGN(sizeof(reqType))], sizeof(confType));                    \
        const MailServerParametersTableEntry_t *const confirmInfo = mailServerTableGetAppropriateEntry(fId);            \
        SYS_DataPointer_t *const dataPointer = (SYS_DataPointer_t *)                                                    \
                                        ((uint8_t *)conf + confirmInfo->confDataPointerOffset);                         \
        if(MAIL_INVALID_PAYLOAD_OFFSET != confirmInfo->confDataPointerOffset)                                           \
        {                                                                                                               \
            int size_payload = SYS_GetPayloadSize(dataPointer);                                                         \
            SYS_SetEmptyPayload(dataPointer);                                                                           \
            SYS_MemAlloc(dataPointer, size_payload);                                                                    \
            int offset = 2 + WORDALIGN(sizeof(reqType)) + WORDALIGN(sizeof(confType));                                  \
            SYS_CopyToPayload(dataPointer, 0, (const void*)&message_rx[offset], size_payload);                          \
        }                                                                                                               \
        if (req.callback) {                                                                                             \
            req.callback((reqType *)message_rx[0], conf);                                                               \
        }                                                                                                               \
        if(MAIL_INVALID_PAYLOAD_OFFSET != confirmInfo->confDataPointerOffset)                                           \
            SYS_FreePayload(dataPointer);                                                                               \
        free(conf);                                                                                                     \
    }
#endif


/*********************************************************************************************************/
/*
* We assume the indication function implemented by client Application is synchronous in which callback function
* is called in the same context. To eliminate saving some intermediate variables, we use the nested function to
* implement this, which requires the C11 compiler feature.
*/
#define CREATE_SERVER_INDICATION_API_FUNCTION(fId, name, indType, respType, rpcMsgCode)     \
    CREATE_SERVER_INDICATION_CALLBACK_FUNCTION(fId, name, indType, respType)                \
    CREATE_SERVER_INDICATION_FUNCTION(fId, name, indType, rpcMsgCode)


#ifdef BYPASS_RPC
#define CREATE_CLIENT_INDICATION_API_FUNCTION(fId, name, indType, respType, rpcMsgCode)
#else
#define CREATE_CLIENT_INDICATION_API_FUNCTION(fId, name, indType, respType, rpcMsgCode)     \
    CREATE_CLIENT_INDICATION_CALLBACK_FUNCTION(fId, name, indType, respType, rpcMsgCode)    \
    CREATE_CLIENT_INDICATION_FUNCTION(fId, name, indType, respType, rpcMsgCode)
#endif

#ifdef BYPASS_RPC
#define CREATE_SERVER_INDICATION_FUNCTION(fId, name, indType, rpcMsgCode)                                               \
    void server_##name(indType *indication, int socket)                                                                 \
    {                                                                                                                   \
        int size = 0;                                                                                                   \
        int message_tx[MAX_MSG_SIZE_IN_WORDS];                                                                          \
        message_tx[0] = (int)indication;                                                                                \
        const MailServerParametersTableEntry_t *const respInfo = mailServerTableGetAppropriateEntry(fId);               \
        if(respInfo->callbackOffset != respInfo->reqLength){                                                            \
            size = 1;                                                                                                   \
            message_tx[size] = *(int*)((uint8_t*)indication + respInfo->callbackOffset);                                \
            *(int*)((uint8_t*)indication + respInfo->callbackOffset) = (int)server_##name##_callback;                   \
        }                                                                                                               \
        else{                                                                                                           \
            size = 1;                                                                                                   \
            message_tx[size] = (int)NULL;                                                                               \
        }                                                                                                               \
        size +=1;                                                                                                       \
        memcpy((char *)&message_tx[size], (char *)indication, sizeof(indType));                                         \
        size += WORDALIGN(sizeof(indType));                                                                             \
        const MailClientParametersTableEntry_t *const clientInfo = mailClientTableGetAppropriateEntry(fId);             \
        if(MAIL_INVALID_PAYLOAD_OFFSET != clientInfo->dataPointerOffset)                                                \
        {                                                                                                               \
                SYS_DataPointer_t *const dataPointer = (SYS_DataPointer_t *)(                                           \
            (uint8_t *)indication + clientInfo->paramOffset + clientInfo->dataPointerOffset);                           \
                                                                                                                        \
            int size_payload = SYS_GetPayloadSize(dataPointer);                                                         \
            SYS_CopyFromPayload((void*)&message_tx[size], dataPointer, 0, size_payload);                                \
            size += WORDALIGN(size_payload);                                                                            \
            if(Zigbee_GetCallback()->name)                                                                              \
                Zigbee_GetCallback()->name(indication);                                                                 \
            if(respInfo->callbackOffset == respInfo->reqLength)                                                         \
                if(SYS_CheckPayload(dataPointer))     SYS_FreePayload(dataPointer);                                     \
                                                                                                                        \
        }                                                                                                               \
    }
#else
#define CREATE_SERVER_INDICATION_FUNCTION(fId, name, indType, rpcMsgCode)                                               \
    void server_##name(indType *indication, int socket)                                                                 \
    {                                                                                                                   \
        int size = 0;                                                                                                   \
        int message_tx[MAX_MSG_SIZE_IN_WORDS];                                                                          \
        message_tx[0] = (int)indication;                                                                                \
        const MailServerParametersTableEntry_t *const respInfo = mailServerTableGetAppropriateEntry(fId);               \
        if(respInfo->callbackOffset != respInfo->reqLength){                                                            \
            size = 1;                                                                                                   \
            message_tx[size] = *(int*)((uint8_t*)indication + respInfo->callbackOffset);                                \
            *(int*)((uint8_t*)indication + respInfo->callbackOffset) = (int)server_##name##_callback;                   \
        }                                                                                                               \
        else{                                                                                                           \
            size = 1;                                                                                                   \
            message_tx[size] = (int)NULL;                                                                               \
        }                                                                                                               \
        size +=1;                                                                                                       \
        memcpy((char *)&message_tx[size], (char *)indication, sizeof(indType));                                         \
        size += WORDALIGN(sizeof(indType));                                                                             \
        const MailClientParametersTableEntry_t *const clientInfo = mailClientTableGetAppropriateEntry(fId);             \
        if(MAIL_INVALID_PAYLOAD_OFFSET != clientInfo->dataPointerOffset)                                                \
        {                                                                                                               \
                SYS_DataPointer_t *const dataPointer = (SYS_DataPointer_t *)(                                           \
            (uint8_t *)indication + clientInfo->paramOffset + clientInfo->dataPointerOffset);                           \
                                                                                                                        \
            int size_payload = SYS_GetPayloadSize(dataPointer);                                                         \
            SYS_CopyFromPayload((void*)&message_tx[size], dataPointer, 0, size_payload);                                \
            size += WORDALIGN(size_payload);                                                                            \
            if(respInfo->callbackOffset == respInfo->reqLength)                                                         \
                if(SYS_CheckPayload(dataPointer))     SYS_FreePayload(dataPointer);                                     \
                                                                                                                        \
        }                                                                                                               \
                                                                                                                        \
        socket_cb[socket].message_id = rpcMsgCode;                                                                      \
        Zigbee_Rpc_Send((unsigned int *)message_tx, size, rpcMsgCode, Zigbee_Socket_ServerSend, socket);                                \
    }
#endif


#define CREATE_SERVER_INDICATION_CALLBACK_FUNCTION(fId, name, indType, respType)                                        \
    void server_##name##_callback(unsigned int *buf, int socket)                                                        \
    {                                                                                                                   \
        int size = 0;                                                                                                   \
        indType *ind;                                                                                                   \
        respType resp;                                                                                                  \
        void (*callback)(indType *req, respType *conf);                                                                 \
        ind = (indType *)buf[0];                                                                                        \
        size = 1;                                                                                                       \
        callback = (void (*)(indType *req, respType *conf))buf[1];                                                      \
        size += 1;                                                                                                      \
        memcpy((char*)&resp, (char*)&buf[size], sizeof(respType));                                                      \
        size += WORDALIGN(sizeof(respType));                                                                            \
        const MailServerParametersTableEntry_t *const respInfo = mailServerTableGetAppropriateEntry(fId);               \
        if(MAIL_INVALID_PAYLOAD_OFFSET != respInfo->confDataPointerOffset)                                              \
        {                                                                                                               \
            SYS_DataPointer_t *const dataPointer = (SYS_DataPointer_t *)(                                               \
            (uint8_t *)&resp + respInfo->confDataPointerOffset);                                                        \
                                                                                                                        \
            int size_payload = SYS_GetPayloadSize(dataPointer);                                                         \
            SYS_SetEmptyPayload(dataPointer);                                                                           \
            SYS_MemAlloc(dataPointer, size_payload);                                                                    \
            SYS_CopyToPayload(dataPointer, 0,  (void*)&buf[size],  size_payload);                                       \
            size += WORDALIGN(size_payload);                                                                            \
        }                                                                                                               \
        callback(ind, &resp);                                                                                           \
    }


#ifndef BYPASS_RPC
#define CREATE_CLIENT_INDICATION_CALLBACK_FUNCTION(fId, name, indType, respType, rpcMsgCode)                    \
void client_##name##_callback(indType *ind, respType *resp)                                                     \
{                                                                                                               \
    int size = 0;                                                                                               \
    int message_tx[MAX_MSG_SIZE_IN_WORDS];                                                                      \
                                                                                                                \
    list_head *pos = 0;                                                                                         \
    int found = 0;                                                                                              \
    List_ClientIndication_Info_t *info = 0;                                                                     \
    list_for_each(pos, &clientIndicationListHeader){                                                            \
        if((info = GET_PARENT_BY_FIELD(List_ClientIndication_Info_t, list, pos))->clientInd == (int)ind){       \
            found = 1;                                                                                          \
            list_del(pos);                                                                                      \
            break;                                                                                              \
        }                                                                                                       \
    }                                                                                                           \
    if(!found){                                                                                                 \
        printf("can't found the origin pointer\n");                                                             \
        return;                                                                                                 \
    }                                                                                                           \
    message_tx[0] = info->serverInd;                                                                            \
    size += 1;                                                                                                  \
    message_tx[size] = info->origIndCallback;                                                                   \
    size += 1;                                                                                                  \
    const MailClientParametersTableEntry_t *const clientInfo = mailClientTableGetAppropriateEntry(fId);         \
    if(MAIL_INVALID_PAYLOAD_OFFSET != clientInfo->dataPointerOffset)                                            \
    {                                                                                                           \
        SYS_DataPointer_t *const data = (SYS_DataPointer_t *)(                                                  \
                (uint8_t *)ind + clientInfo->paramOffset + clientInfo->dataPointerOffset);                      \
                                                                                                                \
        SYS_FreePayload(data);                                                                                  \
    }                                                                                                           \
    free(ind);                                                                                                  \
    free(info);                                                                                                 \
    memcpy((char *)&message_tx[size], (char *)resp, sizeof(respType));                                          \
    size += WORDALIGN(sizeof(respType));                                                                        \
    const MailServerParametersTableEntry_t *const respInfo = mailServerTableGetAppropriateEntry(fId);           \
    if(MAIL_INVALID_PAYLOAD_OFFSET != respInfo->confDataPointerOffset)                                          \
    {                                                                                                           \
        SYS_DataPointer_t *const dataPointer = (SYS_DataPointer_t *)(                                           \
        (uint8_t *)resp + respInfo->confDataPointerOffset);                                                     \
        int size_payload = SYS_GetPayloadSize(dataPointer);                                                     \
        SYS_CopyFromPayload((void*)&message_tx[size], dataPointer, 0, size_payload);                            \
        size += WORDALIGN(size_payload);                                                                        \
        if(SYS_CheckPayload(dataPointer))     SYS_FreePayload(dataPointer);                                     \
    }                                                                                                           \
    Zigbee_Rpc_Send((unsigned int *)message_tx, size, rpcMsgCode | RPC_RESPONSE, Zigbee_Socket_ClientSend, 0);  \
}
#endif

#ifndef BYPASS_RPC
#define CREATE_CLIENT_INDICATION_FUNCTION(fId, name, indType, respType, rpcMsgCode)                                     \
    void client_##name(unsigned int *message_rx)                                                                        \
    {                                                                                                                   \
        indType *serverind, *clientInd = malloc(sizeof(indType));                                                       \
        serverind = (indType *)message_rx[0];                                                                           \
        void *origIndCallback = (void*)message_rx[1];                                                                   \
        memcpy((char*)clientInd, (char*)&message_rx[2], sizeof(indType));                                               \
        const MailServerParametersTableEntry_t *const respInfo = mailServerTableGetAppropriateEntry(fId);               \
                                                                                                                        \
        const MailClientParametersTableEntry_t *const clientInfo = mailClientTableGetAppropriateEntry(fId);             \
        if(MAIL_INVALID_PAYLOAD_OFFSET != clientInfo->dataPointerOffset)                                                \
        {                                                                                                               \
                    SYS_DataPointer_t *const dataPointer = (SYS_DataPointer_t *)(                                       \
                (uint8_t *)clientInd + clientInfo->paramOffset + clientInfo->dataPointerOffset);                        \
                                                                                                                        \
            int size_payload = SYS_GetPayloadSize(dataPointer);                                                         \
            SYS_SetEmptyPayload(dataPointer);                                                                           \
            SYS_MemAlloc(dataPointer, size_payload);                                                                    \
            int offset = 2 + WORDALIGN(sizeof(indType));                                                                \
            SYS_CopyToPayload(dataPointer, 0, (const void*)&message_rx[offset], size_payload);                          \
        }                                                                                                               \
        if(respInfo->callbackOffset != respInfo->reqLength){                                                            \
            *(int*)((uint8_t*)clientInd + respInfo->callbackOffset) = (int)client_##name##_callback;                    \
            List_ClientIndication_Info_t *info = malloc(sizeof(List_ClientIndication_Info_t));                          \
            INIT_CLIENT_LIST_INFO(info, (int)clientInd, (int)serverind, (int)origIndCallback)                           \
            list_add(&info->list, &clientIndicationListHeader);                                                         \
        }                                                                                                               \
        if(Zigbee_GetCallback()->name)                                                                                  \
            Zigbee_GetCallback()->name(clientInd);                                                                      \
        if(respInfo->callbackOffset == respInfo->reqLength){                                                            \
            if(MAIL_INVALID_PAYLOAD_OFFSET != clientInfo->dataPointerOffset){                                           \
                    SYS_DataPointer_t *const dataPointer = (SYS_DataPointer_t *)(                                       \
                (uint8_t *)clientInd + clientInfo->paramOffset + clientInfo->dataPointerOffset);                        \
                                                                                                                        \
                SYS_FreePayload(dataPointer);                                                                           \
            }                                                                                                           \
            free(clientInd);                                                                                            \
        }                                                                                                               \
    }
#endif


#ifdef SERVER

CREATE_SERVER_REQUEST_API_FUNCTION(RF4CE_MAC_REQ_GET_FID, RF4CE_MAC_GetReq, MAC_GetReqDescr_t, MAC_GetConfParams_t, RPC_C2S_RF4CE_MAC_GetReq)

CREATE_SERVER_REQUEST_API_FUNCTION(RF4CE_MAC_REQ_SET_FID, RF4CE_MAC_SetReq, MAC_SetReqDescr_t, MAC_SetConfParams_t, RPC_C2S_RF4CE_MAC_SetReq)

CREATE_SERVER_REQUEST_API_FUNCTION(RF4CE_PROFILE_REQ_RESET_FID, RF4CE_ResetReq, RF4CE_ResetReqDescr_t, RF4CE_StartResetConfParams_t, RPC_C2S_RF4CE_ResetReq)

CREATE_SERVER_REQUEST_API_FUNCTION(RF4CE_PROFILE_REQ_START_FID, RF4CE_StartReq, RF4CE_StartReqDescr_t, RF4CE_StartResetConfParams_t, RPC_C2S_RF4CE_StartReq)

CREATE_SERVER_REQUEST_API_FUNCTION(RF4CE_NWK_REQ_DATA_FID, RF4CE_NWK_DataReq, RF4CE_NWK_DataReqDescr_t, RF4CE_NWK_DataConfParams_t, RPC_C2S_RF4CE_DataReq)

CREATE_SERVER_REQUEST_API_FUNCTION(RF4CE_NWK_REQ_GET_FID, RF4CE_NWK_GetReq, RF4CE_NWK_GetReqDescr_t, RF4CE_NWK_GetConfParams_t, RPC_C2S_RF4CE_GetReq)

CREATE_SERVER_REQUEST_API_FUNCTION(RF4CE_NWK_REQ_SET_FID, RF4CE_NWK_SetReq, RF4CE_NWK_SetReqDescr_t, RF4CE_NWK_SetConfParams_t, RPC_C2S_RF4CE_SetReq)

CREATE_SERVER_REQUEST_API_FUNCTION(RF4CE_ZRC1_REQ_GET_FID, RF4CE_ZRC1_GetAttributesReq, RF4CE_ZRC1_GetAttributeDescr_t, RF4CE_ZRC1_GetAttributeConfParams_t, RPC_C2S_RF4CE_ZRC1_GetAttributesReq)

CREATE_SERVER_REQUEST_API_FUNCTION(RF4CE_ZRC1_REQ_SET_FID, RF4CE_ZRC1_SetAttributesReq, RF4CE_ZRC1_SetAttributeDescr_t, RF4CE_ZRC1_SetAttributeConfParams_t, RPC_C2S_RF4CE_ZRC1_SetAttributesReq)

#ifndef BYPASS_RPC
CREATE_SERVER_REQUEST_API_FUNCTION(TE_ECHO_FID, Mail_TestEngineEcho, TE_EchoCommandReqDescr_t, TE_EchoCommandConfParams_t, RPC_C2S_Mail_TestEngineEcho)

CREATE_SERVER_REQUEST_API_FUNCTION(SYS_EVENT_SUBSCRIBE_FID, SYS_EventSubscribe, SYS_EventHandlerParams_t, NoAppropriateType_t, RPC_C2S_SYS_EventSubscribe)
#endif

CREATE_SERVER_INDICATION_API_FUNCTION(RF4CE_ZRC2_CHECK_VALIDATION_IND_FID, RF4CE_ZRC2_CheckValidationInd, RF4CE_ZRC2_CheckValidationIndParams_t, NoAppropriateType_t, RPC_S2C_RF4CE_ZRC2_CheckValidationInd)

CREATE_SERVER_REQUEST_API_FUNCTION(RF4CE_ZRC2_REQ_SET_FID, RF4CE_ZRC2_SetAttributesReq, RF4CE_ZRC2_SetAttributesReqDescr_t, RF4CE_ZRC2_SetAttributesConfParams_t, RPC_C2S_RF4CE_ZRC2_SetAttributesReq)

CREATE_SERVER_LOCAL_REQUEST_API_FUNCTION(RF4CE_ZRC2_REQ_SET_FID, RF4CE_ZRC2_SetAttributesReq, RF4CE_ZRC2_SetAttributesReqDescr_t, RF4CE_ZRC2_SetAttributesConfParams_t)

CREATE_SERVER_INDICATION_API_FUNCTION(RF4CE_ZRC2_CONTROL_COMMAND_IND_FID, RF4CE_ZRC2_ControlCommandInd, RF4CE_ZRC2_ControlCommandIndParams_t, NoAppropriateType_t, RPC_S2C_RF4CE_ZRC2_ControlCommandInd)

CREATE_SERVER_INDICATION_API_FUNCTION(RF4CE_ZRC1_IND_CONTROLCOMMAND_FID, RF4CE_ZRC1_ControlCommandInd, RF4CE_ZRC1_ControlCommandIndParams_t, NoAppropriateType_t, RPC_S2C_RF4CE_ZRC1_ControlCommandInd)

CREATE_SERVER_REQUEST_API_FUNCTION(TE_ECHO_DELAY_FID, Mail_SetEchoDelay, TE_SetEchoDelayCommandReqDescr_t, TE_SetEchoDelayCommandConfParams_t, RPC_C2S_Mail_SetEchoDelay)

CREATE_SERVER_INDICATION_API_FUNCTION(RF4CE_PROFILE_IND_PAIR_FID, RF4CE_PairInd, RF4CE_PairingIndParams_t, NoAppropriateType_t, RPC_S2C_RF4CE_ZRC_PairInd)

CREATE_SERVER_REQUEST_API_FUNCTION(RF4CE_ZRC_SET_WAKEUP_ACTION_CODE_FID, RF4CE_ZRC_SetWakeUpActionCodeReq, RF4CE_ZRC_SetWakeUpActionCodeReqDescr_t, RF4CE_ZRC_SetWakeUpActionCodeConfParams_t, RPC_C2S_RF4CE_ZRC_SetWakeUpActionCodeKey)

CREATE_SERVER_REQUEST_API_FUNCTION(RF4CE_ZRC_GET_WAKEUP_ACTION_CODE_FID, RF4CE_ZRC_GetWakeUpActionCodeReq, RF4CE_ZRC_GetWakeUpActionCodeReqDescr_t, RF4CE_ZRC_GetWakeUpActionCodeConfParams_t, RPC_C2S_RF4CE_ZRC_GetWakeUpActionCodeKey)


//CREATE_SERVER_REQUEST_API_FUNCTION(RF4CE_PROFILE_REQ_UNPAIR_FID, RF4CE_UnpairReq, RF4CE_UnpairReqDescr_t, RF4CE_UnpairConfParams_t, RPC_C2S_RF4CE_UnpairReq)

CREATE_SERVER_REQUEST_API_FUNCTION(RF4CE_ZRC1_VENDORSPECIFIC_REQ_FID, RF4CE_ZRC1_VendorSpecificReq, RF4CE_ZRC1_VendorSpecificReqDescr_t, RF4CE_ZRC1_VendorSpecificConfParams_t, RPC_C2S_RF4CE_ZRC1_VendorSpecificReq)

CREATE_SERVER_INDICATION_API_FUNCTION(RF4CE_ZRC1_VENDORSPECIFIC_IND_FID, RF4CE_ZRC1_VendorSpecificInd, RF4CE_ZRC1_VendorSpecificIndParams_t, NoAppropriateType_t, RPC_S2C_RF4CE_ZRC1_VendorSpecificInd)

/* Virtual UART staff */
CREATE_SERVER_REQUEST_API_FUNCTION(TE_HOST_TO_UART1_FID, Mail_Host2Uart1, TE_Host2Uart1ReqDescr_t,   NoAppropriateType_t, RPC_C2S_TE_Host2Uart1Req)
/* End of virtual UART staff */

/* Phy test staff */
CREATE_SERVER_REQUEST_API_FUNCTION(RF4CE_CTRL_TEST_GET_CAPS_FID, Phy_Test_Get_Caps_Req,                             Phy_Test_Get_Caps_ReqDescr_t,   Phy_Test_Get_Caps_ConfParams_t, RPC_C2S_Phy_Test_Get_Caps_Req)
CREATE_SERVER_REQUEST_API_FUNCTION(RF4CE_CTRL_TEST_SET_CHANNEL_FID, Phy_Test_Set_Channel_Req,                       Phy_Test_Set_Channel_ReqDescr_t,   Phy_Test_Set_Channel_ConfParams_t, RPC_C2S_Phy_Test_Set_Channel_Req)
CREATE_SERVER_REQUEST_API_FUNCTION(RF4CE_CTRL_TEST_CONTINUOUS_WAVE_START_FID, Phy_Test_Continuous_Wave_Start_Req,   Phy_Test_Continuous_Wave_Start_ReqDescr_t,   Phy_Test_Continuous_Wave_StartStop_ConfParams_t, RPC_C2S_Phy_Test_Continuous_Wave_Start_Req)
CREATE_SERVER_REQUEST_API_FUNCTION(RF4CE_CTRL_TEST_CONTINUOUS_WAVE_STOP_FID, Phy_Test_Continuous_Wave_Stop_Req,     Phy_Test_Continuous_Wave_Stop_ReqDescr_t,   Phy_Test_Continuous_Wave_StartStop_ConfParams_t, RPC_C2S_Phy_Test_Continuous_Wave_Stop_Req)
CREATE_SERVER_REQUEST_API_FUNCTION(RF4CE_CTRL_TEST_TRANSMIT_START_FID, Phy_Test_Transmit_Start_Req,                 Phy_Test_Transmit_Start_ReqDescr_t,   Phy_Test_Transmit_StartStop_ConfParams_t, RPC_C2S_Phy_Test_Transmit_Start_Req)
CREATE_SERVER_REQUEST_API_FUNCTION(RF4CE_CTRL_TEST_TRANSMIT_STOP_FID, Phy_Test_Transmit_Stop_Req,                   Phy_Test_Transmit_Stop_ReqDescr_t,   Phy_Test_Transmit_StartStop_ConfParams_t, RPC_C2S_Phy_Test_Transmit_Stop_Req)
CREATE_SERVER_REQUEST_API_FUNCTION(RF4CE_CTRL_TEST_RECEIVE_START_FID, Phy_Test_Receive_Start_Req,                   Phy_Test_Receive_Start_ReqDescr_t,   Phy_Test_Receive_StartStop_ConfParams_t, RPC_C2S_Phy_Test_Receive_Start_Req)
CREATE_SERVER_REQUEST_API_FUNCTION(RF4CE_CTRL_TEST_RECEIVE_STOP_FID, Phy_Test_Receive_Stop_Req,                     Phy_Test_Receive_Stop_ReqDescr_t,   Phy_Test_Receive_StartStop_ConfParams_t, RPC_C2S_Phy_Test_Receive_Stop_Req)
CREATE_SERVER_REQUEST_API_FUNCTION(RF4CE_CTRL_TEST_ECHO_START_FID, Phy_Test_Echo_Start_Req,                         Phy_Test_Echo_Start_ReqDescr_t,   Phy_Test_Echo_StartStop_ConfParams_t, RPC_C2S_Phy_Test_Echo_Start_Req)
CREATE_SERVER_REQUEST_API_FUNCTION(RF4CE_CTRL_TEST_ECHO_STOP_FID, Phy_Test_Echo_Stop_Req,                           Phy_Test_Echo_Stop_ReqDescr_t,   Phy_Test_Echo_StartStop_ConfParams_t, RPC_C2S_Phy_Test_Echo_Stop_Req)
CREATE_SERVER_REQUEST_API_FUNCTION(RF4CE_CTRL_TEST_ENERGY_DETECT_SCAN_FID, Phy_Test_Energy_Detect_Scan_Req,         Phy_Test_Energy_Detect_Scan_ReqDescr_t,   Phy_Test_Energy_Detect_Scan_ConfParams_t, RPC_C2S_Phy_Test_Energy_Detect_Scan_Req)
CREATE_SERVER_REQUEST_API_FUNCTION(RF4CE_CTRL_TEST_GET_STATS_FID, Phy_Test_Get_Stats_Req,                           Phy_Test_Get_Stats_ReqDescr_t,   Phy_Test_Get_Stats_ConfParams_t, RPC_C2S_Phy_Test_Get_Stats_Req)
CREATE_SERVER_REQUEST_API_FUNCTION(RF4CE_CTRL_TEST_RESET_STATS_FID, Phy_Test_Reset_Stats_Req,                       Phy_Test_Reset_Stats_ReqDescr_t,   Phy_Test_Reset_Stats_ConfParams_t, RPC_C2S_Phy_Test_Reset_Stats_Req)
CREATE_SERVER_REQUEST_API_FUNCTION(RF4CE_CTRL_TEST_SET_TX_POWER_FID, Phy_Test_Set_TX_Power_Req,                     Phy_Test_Set_TX_Power_ReqDescr_t,   Phy_Test_Set_TX_Power_ConfParams_t, RPC_C2S_Phy_Test_Set_TX_Power_Req)
CREATE_SERVER_REQUEST_API_FUNCTION(RF4CE_CTRL_TEST_SELECT_ANTENNA_FID, Phy_Test_SelectAntenna_Req,                  Phy_Test_Select_Antenna_ReqDescr_t,   Phy_Test_Select_Antenna_ConfParams_t, RPC_C2S_Phy_Test_SelectAntenna_Req)
CREATE_SERVER_REQUEST_API_FUNCTION(RF4CE_CTRL_GET_DIAGNOSTICS_CAPS_FID, RF4CE_Get_Diag_Caps_Req,                    RF4CE_Diag_Caps_ReqDescr_t,   RF4CE_Diag_Caps_ConfParams_t, RPC_C2S_RF4CE_Get_Diag_Caps_Req)
CREATE_SERVER_REQUEST_API_FUNCTION(RF4CE_CTRL_GET_DIAGNOSTIC_FID, RF4CE_Get_Diag_Req,                               RF4CE_Diag_ReqDescr_t,   RF4CE_Diag_ConfParams_t, RPC_C2S_RF4CE_Get_Diag_Req)

/* End of Phy test staff */

#ifndef BYPASS_RPC
CREATE_SERVER_REQUEST_API_FUNCTION(TE_RESET_FID, Mail_TestEngineReset, TE_ResetCommandReqDescr_t, NoAppropriateType_t, RPC_S2C_Mail_TestEngineReset)
#endif

CREATE_SERVER_INDICATION_API_FUNCTION(SYS_EVENT_NOTIFY_FID, SYS_EventNtfy, SYS_EventNotifyParams_t, NoAppropriateType_t, RPC_S2C_SYS_EVENTNTFY)
/* NWK staff */
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_NWK_REQ_PERMIT_JOINING_FID, ZBPRO_NWK_PermitJoiningReq, ZBPRO_NWK_PermitJoiningReqDescr_t, ZBPRO_NWK_PermitJoiningConfParams_t, RPC_C2S_ZBPRO_NWK_PermitJoiningReq)
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_NWK_REQ_LEAVE_FID, ZBPRO_NWK_LeaveReq, ZBPRO_NWK_LeaveReqDescr_t, ZBPRO_NWK_LeaveConfParams_t, RPC_C2S_ZBPRO_NWK_LeaveReq)
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_NWK_REQ_GET_KEY_FID, ZBPRO_NWK_GetKeyReq, ZBPRO_NWK_GetKeyReqDescr_t, ZBPRO_NWK_GetKeyConfParams_t, RPC_C2S_ZBPRO_NWK_GetKeyReq)
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_NWK_REQ_SET_KEY_FID, ZBPRO_NWK_SetKeyReq, ZBPRO_NWK_SetKeyReqDescr_t, ZBPRO_NWK_SetKeyConfParams_t, RPC_C2S_ZBPRO_NWK_SetKeyReq)
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_NWK_REQ_ROUTE_DISCOVERY_FID, ZBPRO_NWK_RouteDiscoveryReq, ZBPRO_NWK_RouteDiscoveryReqDescr_t, ZBPRO_NWK_RouteDiscoveryConfParams_t, RPC_C2S_ZBPRO_NWK_RouteDiscoveryReq)
/* End of NWK staff */

/* APS staff */
CREATE_SERVER_REQUEST_CALLBACK_FUNCTION(ZBPRO_APS_REQ_ENDPOINTREGISTER_FID, ZBPRO_APS_EndpointRegisterReq, ZBPRO_APS_EndpointRegisterReqDescr_t, ZBPRO_APS_EndpointRegisterConfParams_t, RPC_C2S_ZBPRO_APS_EndpointRegisterReq)
CREATE_SERVER_REQUEST_CALLBACK_FUNCTION(ZBPRO_APS_REQ_ENDPOINTUNREGISTER_FID, ZBPRO_APS_EndpointUnregisterReq, ZBPRO_APS_EndpointUnregisterReqDescr_t, ZBPRO_APS_EndpointRegisterConfParams_t, RPC_C2S_ZBPRO_APS_EndpointUnregisterReq)
//CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_APS_REQ_ENDPOINTREGISTER_FID, ZBPRO_APS_EndpointRegisterReq, ZBPRO_APS_EndpointRegisterReqDescr_t, ZBPRO_APS_EndpointRegisterConfParams_t, RPC_C2S_ZBPRO_APS_EndpointRegisterReq)
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_APS_REQ_DATA_FID, ZBPRO_APS_DataReq, ZBPRO_APS_DataReqDescr_t, ZBPRO_APS_DataConfParams_t, RPC_C2S_ZBPRO_APS_DataReq)
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_APS_REQ_BIND_FID, ZBPRO_APS_BindReq, ZBPRO_APS_BindUnbindReqDescr_t, ZBPRO_APS_BindUnbindConfParams_t, RPC_C2S_ZBPRO_APS_BindReq)
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_APS_REQ_UNBIND_FID, ZBPRO_APS_UnbindReq, ZBPRO_APS_BindUnbindReqDescr_t, ZBPRO_APS_BindUnbindConfParams_t, RPC_C2S_ZBPRO_APS_UnbindReq)
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_APS_REQ_GET_FID, ZBPRO_APS_GetReq, ZBPRO_APS_GetReqDescr_t, ZBPRO_APS_GetConfParams_t, RPC_C2S_ZBPRO_APS_GetReq)
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_APS_REQ_SET_FID, ZBPRO_APS_SetReq, ZBPRO_APS_SetReqDescr_t, ZBPRO_APS_SetConfParams_t, RPC_C2S_ZBPRO_APS_SetReq)
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_APS_REQ_GET_KEY_FID, ZBPRO_APS_GetKeyReq, ZBPRO_APS_GetKeyReqDescr_t, ZBPRO_APS_GetKeyConfParams_t, RPC_C2S_ZBPRO_APS_GetKeyReq)
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_APS_REQ_SET_KEY_FID, ZBPRO_APS_SetKeyReq, ZBPRO_APS_SetKeyReqDescr_t, ZBPRO_APS_SetKeyConfParams_t, RPC_C2S_ZBPRO_APS_SetKeyReq)
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_APS_REQ_ADDGROUP_FID, ZBPRO_APS_AddGroupReq, ZBPRO_APS_AddGroupReqDescr_t, ZBPRO_APS_AddGroupConfParams_t, RPC_C2S_ZBPRO_APS_AddGroupReq)
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_APS_REQ_REMOVEGROUP_FID, ZBPRO_APS_RemoveGroupReq, ZBPRO_APS_RemoveGroupReqDescr_t, ZBPRO_APS_RemoveGroupConfParams_t, RPC_C2S_ZBPRO_APS_RemoveGroupReq)
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_APS_REQ_REMOVEALLGROUPS_FID, ZBPRO_APS_RemoveAllGroupsReq, ZBPRO_APS_RemoveAllGroupsReqDescr_t, ZBPRO_APS_RemoveAllGroupsConfParams_t, RPC_C2S_ZBPRO_APS_RemoveAllGroupsReq)
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_APS_REQ_TRANSPORTKEY_FID, ZBPRO_APS_TransportKeyReq, ZBPRO_APS_TransportKeyReqDescr_t, ZBPRO_APS_SecurityServicesConfParams_t, RPC_C2S_ZBPRO_APS_TransportKeyReq)
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_APS_REQ_UPDATEDEVICE_FID, ZBPRO_APS_UpdateDeviceReq, ZBPRO_APS_UpdateDeviceReqDescr_t, ZBPRO_APS_SecurityServicesConfParams_t, RPC_C2S_ZBPRO_APS_UpdateDeviceReq)
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_APS_REQ_REMOTEDEVICE_FID, ZBPRO_APS_RemoveDeviceReq, ZBPRO_APS_RemoveDeviceReqDescr_t, ZBPRO_APS_SecurityServicesConfParams_t, RPC_C2S_ZBPRO_APS_RemoveDeviceReq)
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_APS_REQ_REQUESTKEY_FID, ZBPRO_APS_RequestKeyReq, ZBPRO_APS_RequestKeyReqDescr_t, ZBPRO_APS_SecurityServicesConfParams_t, RPC_C2S_ZBPRO_APS_RequestKeyReq)
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_APS_REQ_SWITCHKEY_FID, ZBPRO_APS_SwitchKeyReq, ZBPRO_APS_SwitchKeyReqDescr_t, ZBPRO_APS_SecurityServicesConfParams_t, RPC_C2S_ZBPRO_APS_SwitchKeyReq)
/* End of APS staff */

/* ZDO staff */
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZDO_REQ_ADDR_RESOLVING_FID, ZBPRO_ZDO_AddrResolvingReq, ZBPRO_ZDO_AddrResolvingReqDescr_t, ZBPRO_ZDO_AddrResolvingConfParams_t, RPC_C2S_ZBPRO_ZDO_AddrResolvingReq)
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZDO_REQ_NODE_DESC_FID, ZBPRO_ZDO_NodeDescReq, ZBPRO_ZDO_NodeDescReqDescr_t, ZBPRO_ZDO_NodeDescConfParams_t, RPC_C2S_ZBPRO_ZDO_NodeDescReq)
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZDO_REQ_POWER_DESC_FID, ZBPRO_ZDO_PowerDescReq, ZBPRO_ZDO_PowerDescReqDescr_t, ZBPRO_ZDO_PowerDescConfParams_t, RPC_C2S_ZBPRO_ZDO_PowerDescReq)
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZDO_REQ_SIMPLE_DESC_FID, ZBPRO_ZDO_SimpleDescReq, ZBPRO_ZDO_SimpleDescReqDescr_t, ZBPRO_ZDO_SimpleDescConfParams_t, RPC_C2S_ZBPRO_ZDO_SimpleDescReq)
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZDO_REQ_ACTIVE_EP_FID, ZBPRO_ZDO_ActiveEpReq, ZBPRO_ZDO_ActiveEpReqDescr_t, ZBPRO_ZDO_ActiveEpConfParams_t, RPC_C2S_ZBPRO_ZDO_ActiveEpReq)
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZDO_REQ_MATCH_DESC_FID, ZBPRO_ZDO_MatchDescReq, ZBPRO_ZDO_MatchDescReqDescr_t, ZBPRO_ZDO_MatchDescConfParams_t, RPC_C2S_ZBPRO_ZDO_MatchDescReq)
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZDO_REQ_DEVICE_ANNCE_FID, ZBPRO_ZDO_DeviceAnnceReq, ZBPRO_ZDO_DeviceAnnceReqDescr_t, ZBPRO_ZDO_DeviceAnnceConfParams_t, RPC_C2S_ZBPRO_ZDO_DeviceAnnceReq)
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZDO_REQ_ED_BIND_FID, ZBPRO_ZDO_EndDeviceBindReq, ZBPRO_ZDO_EndDeviceBindReqDescr_t, ZBPRO_ZDO_BindConfParams_t, RPC_C2S_ZBPRO_ZDO_EndDeviceBindReq)
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZDO_REQ_BIND_FID, ZBPRO_ZDO_BindReq, ZBPRO_ZDO_BindUnbindReqDescr_t, ZBPRO_ZDO_BindConfParams_t, RPC_C2S_ZBPRO_ZDO_BindReq)
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZDO_REQ_UNBIND_FID, ZBPRO_ZDO_UnbindReq, ZBPRO_ZDO_BindUnbindReqDescr_t, ZBPRO_ZDO_BindConfParams_t, RPC_C2S_ZBPRO_ZDO_UnbindReq)
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZDO_REQ_START_NETWORK_FID, ZBPRO_ZDO_StartNetworkReq, ZBPRO_ZDO_StartNetworkReqDescr_t, ZBPRO_ZDO_StartNetworkConfParams_t, RPC_C2S_ZBPRO_ZDO_StartNetworkReq)
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZDO_REQ_MGMT_LEAVE_FID, ZBPRO_ZDO_MgmtLeaveReq, ZBPRO_ZDO_MgmtLeaveReqDescr_t, ZBPRO_ZDO_MgmtLeaveConfParams_t, RPC_C2S_ZBPRO_ZDO_MgmtLeaveReq)
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZDO_REQ_MGMT_PERMIT_JOINING_FID, ZBPRO_ZDO_MgmtPermitJoiningReq, ZBPRO_ZDO_MgmtPermitJoiningReqDescr_t, ZBPRO_ZDO_MgmtPermitJoiningConfParams_t, RPC_C2S_ZBPRO_ZDO_MgmtPermitJoiningReq)
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZDO_REQ_MGMT_NWK_UPDATE_FID, ZBPRO_ZDO_MgmtNwkUpdateReq, ZBPRO_ZDO_MgmtNwkUpdateReqDescr_t, ZBPRO_ZDO_MgmtNwkUpdateConfParams_t, RPC_C2S_ZBPRO_ZDO_MgmtNwkUpdateReq)
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZDO_RESP_MGMT_NWK_UPDATE_UNSOL_FID, ZBPRO_ZDO_MgmtNwkUpdateUnsolResp, ZBPRO_ZDO_MgmtNwkUpdateUnsolRespDescr_t, ZBPRO_ZDO_MgmtNwkUpdateUnsolConfParams_t, RPC_C2S_ZBPRO_ZDO_MgmtNwkUpdateUnsolResp)
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZDO_REQ_MGMT_LQI_FID, ZBPRO_ZDO_MgmtLqiReq, ZBPRO_ZDO_MgmtLqiReqDescr_t, ZBPRO_ZDO_MgmtLqiConfParams_t, RPC_C2S_ZBPRO_ZDO_MgmtLqiReq)
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZDO_REQ_MGMT_BIND_FID, ZBPRO_ZDO_MgmtBindReq, ZBPRO_ZDO_MgmtBindReqDescr_t, ZBPRO_ZDO_MgmtBindConfParams_t, RPC_C2S_ZBPRO_ZDO_MgmtBindReq)
CREATE_SERVER_INDICATION_API_FUNCTION(ZBPRO_ZDO_IND_MGMT_NWK_UPDATE_UNSOL_FID, ZBPRO_ZDO_MgmtNwkUpdateUnsolInd, ZBPRO_ZDO_MgmtNwkUpdateUnsolIndParams_t, NoAppropriateType_t, RPC_C2S_ZBPRO_ZDO_MgmtNwkUpdateUnsolInd)
/* End of ZDO staff */

/* ZCL staff */
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_TC_REQ_NWK_KEY_UPDATE_FID, ZBPRO_TC_NwkKeyUpdateReq, ZBPRO_TC_NwkKeyUpdateReqDescr_t, ZBPRO_TC_NwkKeyUpdateConfParams_t, RPC_C2S_ZBPRO_TC_NwkKeyUpdateReq)
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_REQ_SET_POWER_SOURCE_FID, ZBPRO_ZCL_SetPowerSourceReq, ZBPRO_ZCL_SetPowerSourceReqDescr_t, ZBPRO_ZCL_SetPowerSourceConfParams_t, RPC_C2S_ZBPRO_ZCL_SetPowerSourceReq)
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_PROFILE_WIDE_CMD_DISCOVER_ATTRIBUTE_REQ_FID, ZBPRO_ZCL_ProfileWideCmdDiscoverAttrReq, ZBPRO_ZCL_ProfileWideCmdDiscoverAttrDescr_t, ZBPRO_ZCL_ProfileWideCmdDiscoverAttrConfParams_t, RPC_C2S_ZBPRO_ZCL_ProfileWideCmdDiscoverAttrReq)
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_PROFILE_WIDE_CMD_READ_ATTRIBUTE_REQ_FID, ZBPRO_ZCL_ProfileWideCmdReadAttributesReq, ZBPRO_ZCL_ProfileWideCmdReadAttrReqDescr_t, ZBPRO_ZCL_ProfileWideCmdReadAttrConfParams_t, RPC_C2S_ZBPRO_ZCL_ProfileWideCmdReadAttributesReq)
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_PROFILE_WIDE_CMD_WRITE_ATTRIBUTE_REQ_FID, ZBPRO_ZCL_ProfileWideCmdWriteAttributesReq, ZBPRO_ZCL_ProfileWideCmdWriteAttrReqDescr_t, ZBPRO_ZCL_ProfileWideCmdWriteAttrConfParams_t, RPC_C2S_ZBPRO_ZCL_ProfileWideCmdWriteAttributesReq)
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_PROFILE_WIDE_CMD_CONFIGURE_REPORTING_REQ_FID, ZBPRO_ZCL_ProfileWideCmdConfigureReportingReq, ZBPRO_ZCL_ProfileWideCmdConfigureReportingReqDescr_t, ZBPRO_ZCL_ProfileWideCmdConfigureReportingConfParams_t, RPC_C2S_ZBPRO_ZCL_ProfileWideCmdConfigureReportingReq)
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_PROFILE_WIDE_CMD_READ_REPORTING_CONFIGURATION_REQ_FID, ZBPRO_ZCL_ProfileWideCmdReadReportingConfigurationReq, ZBPRO_ZCL_ProfileWideCmdReadReportingConfigurationReqDescr_t, ZBPRO_ZCL_ProfileWideCmdReadReportingConfigurationConfParams_t, RPC_C2S_ZBPRO_ZCL_ProfileWideCmdReadReportingConfigurationReq)
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_IDENTIFY_CMD_IDENTIFY_REQ_FID, ZBPRO_ZCL_IdentifyCmdIdentifyReq, ZBPRO_ZCL_IdentifyCmdIdentifyReqDescr_t, ZBPRO_ZCL_IdentifyCmdConfParams_t, RPC_C2S_ZBPRO_ZCL_IdentifyCmdIdentifyReq)
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_IDENTIFY_CMD_IDENTIFY_QUERY_REQ_FID, ZBPRO_ZCL_IdentifyCmdIdentifyQueryReq, ZBPRO_ZCL_IdentifyCmdIdentifyQueryReqDescr_t, ZBPRO_ZCL_IdentifyCmdConfParams_t, RPC_C2S_ZBPRO_ZCL_IdentifyCmdIdentifyQueryReq)
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_GROUPS_CMD_ADD_GROUP_REQ_FID, ZBPRO_ZCL_GroupsCmdAddGroupReq, ZBPRO_ZCL_GroupsCmdAddGroupReqDescr_t, ZBPRO_ZCL_GroupsCmdAddGroupConfParams_t, RPC_C2S_ZBPRO_ZCL_GroupsCmdAddGroupReq)
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_GROUPS_CMD_VIEW_GROUP_REQ_FID, ZBPRO_ZCL_GroupsCmdViewGroupReq, ZBPRO_ZCL_GroupsCmdViewGroupReqDescr_t, ZBPRO_ZCL_GroupsCmdViewGroupConfParams_t, RPC_C2S_ZBPRO_ZCL_GroupsCmdViewGroupReq)
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_GROUPS_CMD_GET_GROUP_MEMBERSHIP_REQ_FID, ZBPRO_ZCL_GroupsCmdGetGroupMembershipReq, ZBPRO_ZCL_GroupsCmdGetGroupMembershipReqDescr_t, ZBPRO_ZCL_GroupsCmdGetGroupMembershipConfParams_t, RPC_C2S_ZBPRO_ZCL_GroupsCmdGetGroupMembershipReq)
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_GROUPS_CMD_REMOVE_GROUP_REQ_FID, ZBPRO_ZCL_GroupsCmdRemoveGroupReq, ZBPRO_ZCL_GroupsCmdRemoveGroupReqDescr_t, ZBPRO_ZCL_GroupsCmdRemoveGroupConfParams_t, RPC_C2S_ZBPRO_ZCL_GroupsCmdRemoveGroupReq)
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_GROUPS_CMD_REMOVE_ALL_GROUPS_REQ_FID, ZBPRO_ZCL_GroupsCmdRemoveAllGroupsReq, ZBPRO_ZCL_GroupsCmdRemoveAllGroupsReqDescr_t, ZBPRO_ZCL_GroupsCmdRemoveAllGroupsConfParams_t, RPC_C2S_ZBPRO_ZCL_GroupsCmdRemoveAllGroupsReq)
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_GROUPS_CMD_ADD_GROUP_IF_IDENTIFY_REQ_FID, ZBPRO_ZCL_GroupsCmdAddGroupIfIdentifyReq, ZBPRO_ZCL_GroupsCmdAddGroupIfIdentifyReqDescr_t, ZBPRO_ZCL_GroupsCmdAddGroupIfIdentifyConfParams_t, RPC_C2S_ZBPRO_ZCL_GroupsCmdAddGroupIfIdentifyReq)
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_SCENES_CMD_ADD_SCENE_REQ_FID, ZBPRO_ZCL_ScenesCmdAddSceneReq, ZBPRO_ZCL_ScenesCmdAddSceneReqDescr_t, ZBPRO_ZCL_ScenesCmdAddSceneConfParams_t, RPC_C2S_ZBPRO_ZCL_ScenesCmdAddSceneReq)
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_SCENES_CMD_VIEW_SCENE_REQ_FID, ZBPRO_ZCL_ScenesCmdViewSceneReq, ZBPRO_ZCL_ScenesCmdViewSceneReqDescr_t, ZBPRO_ZCL_ScenesCmdViewSceneConfParams_t, RPC_C2S_ZBPRO_ZCL_ScenesCmdViewSceneReq)
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_SCENES_CMD_STORE_SCENE_REQ_FID, ZBPRO_ZCL_ScenesCmdStoreSceneReq, ZBPRO_ZCL_ScenesCmdStoreSceneReqDescr_t, ZBPRO_ZCL_ScenesCmdStoreSceneConfParams_t, RPC_C2S_ZBPRO_ZCL_ScenesCmdStoreSceneReq)
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_SCENES_CMD_RECALL_SCENE_REQ_FID, ZBPRO_ZCL_ScenesCmdRecallSceneReq, ZBPRO_ZCL_ScenesCmdRecallSceneReqDescr_t, ZBPRO_ZCL_ScenesCmdConfParams_t, RPC_C2S_ZBPRO_ZCL_ScenesCmdRecallSceneReq)
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_SCENES_CMD_REMOVE_SCENE_REQ_FID, ZBPRO_ZCL_ScenesCmdRemoveSceneReq, ZBPRO_ZCL_ScenesCmdRemoveSceneReqDescr_t, ZBPRO_ZCL_ScenesCmdRemoveSceneConfParams_t, RPC_C2S_ZBPRO_ZCL_ScenesCmdRemoveSceneReq)
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_SCENES_CMD_REMOVE_ALL_SCENES_REQ_FID, ZBPRO_ZCL_ScenesCmdRemoveAllScenesReq, ZBPRO_ZCL_ScenesCmdRemoveAllScenesReqDescr_t, ZBPRO_ZCL_ScenesCmdRemoveAllScenesConfParams_t, RPC_C2S_ZBPRO_ZCL_ScenesCmdRemoveAllScenesReq)
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_SCENES_CMD_GET_SCENE_MEMBERSHIP_REQ_FID, ZBPRO_ZCL_ScenesCmdGetSceneMembershipReq, ZBPRO_ZCL_ScenesCmdGetSceneMembershipReqDescr_t, ZBPRO_ZCL_ScenesCmdConfParams_t, RPC_C2S_ZBPRO_ZCL_ScenesCmdGetSceneMembershipReq)
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_ONOFF_CMD_OFF_REQ_FID, ZBPRO_ZCL_OnOffCmdOffReq, ZBPRO_ZCL_OnOffCmdReqDescr_t, ZBPRO_ZCL_OnOffCmdConfParams_t, RPC_C2S_ZBPRO_ZCL_OnOffCmdOffReq)
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_ONOFF_CMD_ON_REQ_FID, ZBPRO_ZCL_OnOffCmdOnReq, ZBPRO_ZCL_OnOffCmdReqDescr_t, ZBPRO_ZCL_OnOffCmdConfParams_t, RPC_C2S_ZBPRO_ZCL_OnOffCmdOnReq)
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_ONOFF_CMD_TOGGLE_REQ_FID, ZBPRO_ZCL_OnOffCmdToggleReq, ZBPRO_ZCL_OnOffCmdReqDescr_t, ZBPRO_ZCL_OnOffCmdConfParams_t, RPC_C2S_ZBPRO_ZCL_OnOffCmdToggleReq)
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_LEVEL_CONTROL_CMD_MOVE_TO_LEVEL_REQ_FID, ZBPRO_ZCL_LevelControlCmdMoveToLevelReq, ZBPRO_ZCL_LevelControlCmdMoveToLevelReqDescr_t, ZBPRO_ZCL_LevelControlCmdConfParams_t, RPC_C2S_ZBPRO_ZCL_LevelControlCmdMoveToLevelReq)
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_LEVEL_CONTROL_CMD_MOVE_REQ_FID, ZBPRO_ZCL_LevelControlCmdMoveReq, ZBPRO_ZCL_LevelControlCmdMoveReqDescr_t, ZBPRO_ZCL_LevelControlCmdConfParams_t, RPC_C2S_ZBPRO_ZCL_LevelControlCmdMoveReq)
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_LEVEL_CONTROL_CMD_STEP_REQ_FID, ZBPRO_ZCL_LevelControlCmdStepReq, ZBPRO_ZCL_LevelControlCmdStepReqDescr_t, ZBPRO_ZCL_LevelControlCmdConfParams_t, RPC_C2S_ZBPRO_ZCL_LevelControlCmdStepReq)
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_LEVEL_CONTROL_CMD_STOP_REQ_FID, ZBPRO_ZCL_LevelControlCmdStopReq, ZBPRO_ZCL_LevelControlCmdStopReqDescr_t, ZBPRO_ZCL_LevelControlCmdConfParams_t, RPC_C2S_ZBPRO_ZCL_LevelControlCmdStopReq)
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_DOOR_LOCK_CMD_LOCK_REQ_FID, ZBPRO_ZCL_DoorLockCmdLockReq, ZBPRO_ZCL_DoorLockCmdLockUnlockReqDescr_t, ZBPRO_ZCL_DoorLockCmdLockUnlockConfParams_t, RPC_C2S_ZBPRO_ZCL_DoorLockCmdLockReq)
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_DOOR_LOCK_CMD_UNLOCK_REQ_FID, ZBPRO_ZCL_DoorLockCmdUnlockReq, ZBPRO_ZCL_DoorLockCmdLockUnlockReqDescr_t, ZBPRO_ZCL_DoorLockCmdLockUnlockConfParams_t, RPC_C2S_ZBPRO_ZCL_DoorLockCmdUnlockReq)
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_WINDOW_COVERING_CMD_UP_OPEN_REQ_FID, ZBPRO_ZCL_WindowCoveringCmdUpOpenReq, ZBPRO_ZCL_WindowCoveringCmdReqDescr_t, ZBPRO_ZCL_WindowCoveringCmdConfParams_t, RPC_C2S_ZBPRO_ZCL_WindowCoveringCmdUpOpenReq)
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_WINDOW_COVERING_CMD_DOWN_CLOSE_REQ_FID, ZBPRO_ZCL_WindowCoveringCmdDownCloseReq, ZBPRO_ZCL_WindowCoveringCmdReqDescr_t, ZBPRO_ZCL_WindowCoveringCmdConfParams_t, RPC_C2S_ZBPRO_ZCL_WindowCoveringCmdDownCloseReq)
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_WINDOW_COVERING_CMD_STOP_REQ_FID, ZBPRO_ZCL_WindowCoveringCmdStopReq, ZBPRO_ZCL_WindowCoveringCmdReqDescr_t, ZBPRO_ZCL_WindowCoveringCmdConfParams_t, RPC_C2S_ZBPRO_ZCL_WindowCoveringCmdStopReq)
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_WINDOW_COVERING_CMD_GO_TO_LIFT_PERCENTAGE_REQ_FID, ZBPRO_ZCL_WindowCoveringCmdGotoLiftPecentageReq, ZBPRO_ZCL_WindowCoveringLiftTiltPercentCmdReqDescr_t, ZBPRO_ZCL_WindowCoveringCmdConfParams_t, RPC_C2S_ZBPRO_ZCL_WindowCoveringCmdGotoLiftPecentageReq)
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_WINDOW_COVERING_CMD_GO_TO_TILT_PERCENTAGE_REQ_FID, ZBPRO_ZCL_WindowCoveringCmdGotoTiltPecentageReq, ZBPRO_ZCL_WindowCoveringLiftTiltPercentCmdReqDescr_t, ZBPRO_ZCL_WindowCoveringCmdConfParams_t, RPC_C2S_ZBPRO_ZCL_WindowCoveringCmdGotoTiltPecentageReq)
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_IAS_ZONE_CMD_ZONE_ENROLL_RESPONSE_REQ_FID, ZBPRO_ZCL_IASZoneCmdZoneEnrollResponseReq, ZBPRO_ZCL_IASZoneCmdZoneEnrollResponseReqDescr_t, ZBPRO_ZCL_IASZoneCmdZoneEnrollResponseConfParams_t, RPC_C2S_ZBPRO_ZCL_IASZoneCmdZoneEnrollResponseReq)
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_IAS_ACE_ARM_RESP_REQ_FID, ZBPRO_ZCL_SapIasAceArmRespReq, ZBPRO_ZCL_SapIasAceArmRespReqDescr_t, ZBPRO_ZCL_SapIasAceRespReqConfParams_t, RPC_C2S_ZBPRO_ZCL_SapIasAceArmRespReq)
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_IAS_ACE_BYPASS_RESP_REQ_FID, ZBPRO_ZCL_SapIasAceBypassRespReq, ZBPRO_ZCL_SapIasAceBypassRespReqDescr_t, ZBPRO_ZCL_SapIasAceRespReqConfParams_t, RPC_C2S_ZBPRO_ZCL_SapIasAceBypassRespReq)
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_IAS_ACE_GET_ZONE_ID_MAP_RESP_REQ_FID, ZBPRO_ZCL_SapIasAceGetZoneIdMapRespReq, ZBPRO_ZCL_SapIasAceGetZoneIdMapRespReqDescr_t, ZBPRO_ZCL_SapIasAceRespReqConfParams_t, RPC_C2S_ZBPRO_ZCL_SapIasAceGetZoneIdMapRespReq)
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_IAS_ACE_GET_ZONE_INFO_RESP_REQ_FID, ZBPRO_ZCL_SapIasAceGetZoneInfoRespReq, ZBPRO_ZCL_SapIasAceGetZoneInfoRespReqDescr_t, ZBPRO_ZCL_SapIasAceRespReqConfParams_t, RPC_C2S_ZBPRO_ZCL_SapIasAceGetZoneInfoRespReq)
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_IAS_ACE_GET_PANEL_STATUS_RESP_REQ_FID, ZBPRO_ZCL_SapIasAceGetPanelStatusRespReq, ZBPRO_ZCL_SapIasAceGetPanelStatusRespReqDescr_t, ZBPRO_ZCL_SapIasAceRespReqConfParams_t, RPC_C2S_ZBPRO_ZCL_SapIasAceGetPanelStatusRespReq)
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_IAS_ACE_SET_BYPASSED_ZONE_LIST_RESP_REQ_FID, ZBPRO_ZCL_SapIasAceSetBypassedZoneListRespReq, ZBPRO_ZCL_SapIasAceSetBypassedZoneListRespReqDescr_t, ZBPRO_ZCL_SapIasAceRespReqConfParams_t, RPC_C2S_ZBPRO_ZCL_SapIasAceSetBypassedZoneListRespReq)
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_IAS_ACE_GET_ZONE_STATUS_RESP_REQ_FID, ZBPRO_ZCL_SapIasAceGetZoneStatusRespReq, ZBPRO_ZCL_SapIasAceGetZoneStatusRespReqDescr_t, ZBPRO_ZCL_SapIasAceRespReqConfParams_t, RPC_C2S_ZBPRO_ZCL_SapIasAceGetZoneStatusRespReq)
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_IAS_ACE_ZONE_STATUS_CHANGED_REQ_FID, ZBPRO_ZCL_SapIasAceZoneStatusChangedReq, ZBPRO_ZCL_SapIasAceZoneStatusChangedReqDescr_t, ZBPRO_ZCL_SapIasAceRespReqConfParams_t, RPC_C2S_ZBPRO_ZCL_SapIasAceZoneStatusChangedReq)
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_IAS_ACE_PANEL_STATUS_CHANGED_REQ_FID, ZBPRO_ZCL_SapIasAcePanelStatusChangedReq, ZBPRO_ZCL_SapIasAcePanelStatusChangedReqDescr_t, ZBPRO_ZCL_SapIasAceRespReqConfParams_t, RPC_C2S_ZBPRO_ZCL_SapIasAcePanelStatusChangedReq)
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_IAS_WD_CMD_START_WARNING_REQ_FID, ZBPRO_ZCL_IASWDCmdStartWarningReq, ZBPRO_ZCL_IASWDCmdStartWarningReqDescr_t, ZBPRO_ZCL_IASWDCmdStartWarningConfParams_t, RPC_C2S_ZBPRO_ZCL_IASWDCmdStartWarningReq)
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_IAS_WD_CMD_SQUAWK_REQ_FID, ZBPRO_ZCL_IASWDCmdSquawkgReq, ZBPRO_ZCL_IASWDCmdSquawkReqDescr_t, ZBPRO_ZCL_IASWDCmdSquawkConfParams_t, RPC_C2S_ZBPRO_ZCL_IASWDCmdSquawkgReq)
CREATE_SERVER_INDICATION_API_FUNCTION(ZBPRO_ZCL_IND_IDENTIFY_FID, ZBPRO_ZCL_IdentifyInd, ZBPRO_ZCL_IdentifyIndParams_t, NoAppropriateType_t, RPC_C2S_ZBPRO_ZCL_IdentifyInd)
CREATE_SERVER_INDICATION_API_FUNCTION(ZBPRO_ZCL_PROFILE_WIDE_CMD_REPORT_ATTRIBUTES_IND_FID, ZBPRO_ZCL_ProfileWideCmdReportAttributesInd, ZBPRO_ZCL_ProfileWideCmdReportAttributesIndParams_t, NoAppropriateType_t, RPC_C2S_ZBPRO_ZCL_ProfileWideCmdReportAttributesInd)
CREATE_SERVER_INDICATION_API_FUNCTION(ZBPRO_ZCL_GROUPS_CMD_GET_GROUP_MEMBERSHIP_RESPONSE_IND_FID, ZBPRO_ZCL_GroupsCmdGetGroupMembershipResponseInd, ZBPRO_ZCL_GroupsCmdGetGroupMembershipIndParams_t, NoAppropriateType_t, RPC_C2S_ZBPRO_ZCL_GroupsCmdGetGroupMembershipResponseInd)
CREATE_SERVER_INDICATION_API_FUNCTION(ZBPRO_ZCL_SCENES_CMD_GET_SCENE_MEMBERSHIP_RESPONSE_IND_FID, ZBPRO_ZCL_ScenesCmdGetSceneMembershipResponseInd, ZBPRO_ZCL_ScenesCmdGetSceneMembershipResponseIndParams_t, NoAppropriateType_t, RPC_C2S_ZBPRO_ZCL_ScenesCmdGetSceneMembershipResponseInd)
CREATE_SERVER_INDICATION_API_FUNCTION(ZBPRO_ZCL_IAS_ZONE_CMD_ZONE_ENROLL_IND_FID, ZBPRO_ZCL_IASZoneCmdZoneEnrollRequestInd, ZBPRO_ZCL_IASZoneCmdZoneEnrollRequestIndParams_t, NoAppropriateType_t, RPC_C2S_ZBPRO_ZCL_IASZoneCmdZoneEnrollRequestInd)
CREATE_SERVER_INDICATION_API_FUNCTION(ZBPRO_ZCL_IAS_ZONE_CMD_ZONE_STATUS_CHANGED_NOTIFICATION_IND_FID, ZBPRO_ZCL_IASZoneCmdZoneStatusChangeNotificationInd, ZBPRO_ZCL_IASZoneCmdZoneStatusChangeNotificationIndParams_t, NoAppropriateType_t, RPC_C2S_ZBPRO_ZCL_IASZoneCmdZoneStatusChangeNotificationInd)
CREATE_SERVER_INDICATION_API_FUNCTION(ZBPRO_ZCL_IAS_ACE_ARM_IND_FID, ZBPRO_ZCL_SapIasAceArmInd, ZBPRO_ZCL_SapIasAceArmIndParams_t, NoAppropriateType_t, RPC_C2S_ZBPRO_ZCL_SapIasAceArmInd)
CREATE_SERVER_INDICATION_API_FUNCTION(ZBPRO_ZCL_IAS_ACE_BYPASS_IND_FID, ZBPRO_ZCL_SapIasAceBypassInd, ZBPRO_ZCL_SapIasAceBypassIndParams_t, NoAppropriateType_t, RPC_C2S_ZBPRO_ZCL_SapIasAceBypassInd)
CREATE_SERVER_INDICATION_API_FUNCTION(ZBPRO_ZCL_IAS_ACE_EMERGENCY_IND_FID, ZBPRO_ZCL_SapIasAceEmergencyInd, ZBPRO_ZCL_SapIasAceAlarmIndParams_t, NoAppropriateType_t, RPC_C2S_ZBPRO_ZCL_SapIasAceEmergencyInd)
CREATE_SERVER_INDICATION_API_FUNCTION(ZBPRO_ZCL_IAS_ACE_FIRE_IND_FID, ZBPRO_ZCL_SapIasAceFireInd, ZBPRO_ZCL_SapIasAceAlarmIndParams_t, NoAppropriateType_t, RPC_C2S_ZBPRO_ZCL_SapIasAceFireInd)
CREATE_SERVER_INDICATION_API_FUNCTION(ZBPRO_ZCL_IAS_ACE_PANIC_IND_FID, ZBPRO_ZCL_SapIasAcePanicInd, ZBPRO_ZCL_SapIasAceAlarmIndParams_t, NoAppropriateType_t, RPC_C2S_ZBPRO_ZCL_SapIasAcePanicInd)
CREATE_SERVER_INDICATION_API_FUNCTION(ZBPRO_ZCL_IAS_ACE_GET_ZONE_ID_MAP_IND_FID, ZBPRO_ZCL_SapIasAceGetZoneIdMapInd, ZBPRO_ZCL_SapIasAceGetZoneIdMapIndParams_t, NoAppropriateType_t, RPC_C2S_ZBPRO_ZCL_SapIasAceGetZoneIdMapInd)
CREATE_SERVER_INDICATION_API_FUNCTION(ZBPRO_ZCL_IAS_ACE_GET_ZONE_INFO_IND_FID, ZBPRO_ZCL_SapIasAceGetZoneInfoInd, ZBPRO_ZCL_SapIasAceGetZoneInfoIndParams_t, NoAppropriateType_t, RPC_C2S_ZBPRO_ZCL_SapIasAceGetZoneInfoInd)
CREATE_SERVER_INDICATION_API_FUNCTION(ZBPRO_ZCL_IAS_ACE_GET_PANEL_STATUS_IND_FID, ZBPRO_ZCL_SapIasAceGetPanelStatusInd, ZBPRO_ZCL_SapIasAceGetPanelStatusIndParams_t, NoAppropriateType_t, RPC_C2S_ZBPRO_ZCL_SapIasAceGetPanelStatusInd)
CREATE_SERVER_INDICATION_API_FUNCTION(ZBPRO_ZCL_IAS_ACE_GET_BYPASSED_ZONE_LIST_IND_FID, ZBPRO_ZCL_SapIasAceGetBypassedZoneListInd, ZBPRO_ZCL_SapIasAceGetBypassedZoneListIndParams_t, NoAppropriateType_t, RPC_C2S_ZBPRO_ZCL_SapIasAceGetBypassedZoneListInd)
CREATE_SERVER_INDICATION_API_FUNCTION(ZBPRO_ZCL_IAS_ACE_GET_ZONE_STATUS_IND_FID, ZBPRO_ZCL_SapIasAceGetZoneStatusInd, ZBPRO_ZCL_SapIasAceGetZoneStatusIndParams_t, NoAppropriateType_t, RPC_C2S_ZBPRO_ZCL_SapIasAceGetZoneStatusInd)
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_COLOR_CONTROL_CMD_MOVE_TO_COLOR_REQ_FID, ZBPRO_ZCL_ColorControlCmdMoveToColorReq, ZBPRO_ZCL_ColorControlCmdMoveToColorReqDescr_t, ZBPRO_ZCL_ColorControlCmdMoveToColorConfParams_t, RPC_C2S_ZBPRO_ZCL_ColorControlCmdMoveToColorReq)
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_COLOR_CONTROL_CMD_MOVE_COLOR_REQ_FID, ZBPRO_ZCL_ColorControlCmdMoveColorReq, ZBPRO_ZCL_ColorControlCmdMoveColorReqDescr_t, ZBPRO_ZCL_ColorControlCmdMoveColorConfParams_t, RPC_C2S_ZBPRO_ZCL_ColorControlCmdMoveColorReq)
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_COLOR_CONTROL_CMD_STEP_COLOR_REQ_FID, ZBPRO_ZCL_ColorControlCmdStepColorReq, ZBPRO_ZCL_ColorControlCmdStepColorReqDescr_t, ZBPRO_ZCL_ColorControlCmdStepColorConfParams_t, RPC_C2S_ZBPRO_ZCL_ColorControlCmdStepColorReq)
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_COLOR_CONTROL_CMD_ENHANCED_MOVE_TO_HUE_REQ_FID, ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueReq, ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueReqDescr_t, ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueConfParams_t, RPC_C2S_ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueReq)
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_COLOR_CONTROL_CMD_ENHANCED_MOVE_HUE_REQ_FID, ZBPRO_ZCL_ColorControlCmdEnhancedMoveHueReq, ZBPRO_ZCL_ColorControlCmdEnhancedMoveHueReqDescr_t, ZBPRO_ZCL_ColorControlCmdEnhancedMoveHueConfParams_t, RPC_C2S_ZBPRO_ZCL_ColorControlCmdEnhancedMoveHueReq)
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_COLOR_CONTROL_CMD_ENHANCED_STEP_HUE_REQ_FID, ZBPRO_ZCL_ColorControlCmdEnhancedStepHueReq, ZBPRO_ZCL_ColorControlCmdEnhancedStepHueReqDescr_t, ZBPRO_ZCL_ColorControlCmdEnhancedStepHueConfParams_t, RPC_C2S_ZBPRO_ZCL_ColorControlCmdEnhancedStepHueReq)
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_COLOR_CONTROL_CMD_ENHANCED_MOVE_TO_HUE_AND_SATURATION_REQ_FID, ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueAndSaturationReq, ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueAndSaturationReqDescr_t, ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueAndSaturationConfParams_t, RPC_C2S_ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueAndSaturationReq)
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_COLOR_CONTROL_CMD_COLOR_LOOP_SET_REQ_FID, ZBPRO_ZCL_ColorControlCmdColorLoopSetReq, ZBPRO_ZCL_ColorControlCmdColorLoopSetReqDescr_t, ZBPRO_ZCL_ColorControlCmdColorLoopSetConfParams_t, RPC_C2S_ZBPRO_ZCL_ColorControlCmdColorLoopSetReq)
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_COLOR_CONTROL_CMD_STOP_MOVE_STEP_REQ_FID, ZBPRO_ZCL_ColorControlCmdStopMoveStepReq, ZBPRO_ZCL_ColorControlCmdStopMoveStepReqDescr_t, ZBPRO_ZCL_ColorControlCmdStopMoveStepConfParams_t, RPC_C2S_ZBPRO_ZCL_ColorControlCmdStopMoveStepReq)
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_COLOR_CONTROL_CMD_MOVE_COLOR_TEMPERATURE_REQ_FID, ZBPRO_ZCL_ColorControlCmdMoveColorTemperatureReq, ZBPRO_ZCL_ColorControlCmdMoveColorTemperatureReqDescr_t, ZBPRO_ZCL_ColorControlCmdMoveColorTemperatureConfParams_t, RPC_C2S_ZBPRO_ZCL_ColorControlCmdMoveColorTemperatureReq)
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZCL_COLOR_CONTROL_CMD_STEP_COLOR_TEMPERATURE_REQ_FID, ZBPRO_ZCL_ColorControlCmdStepColorTemperatureReq, ZBPRO_ZCL_ColorControlCmdStepColorTemperatureReqDescr_t, ZBPRO_ZCL_ColorControlCmdStepColorTemperatureConfParams_t, RPC_C2S_ZBPRO_ZCL_ColorControlCmdStepColorTemperatureReq)
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZHA_EZ_MODE_FID, ZBPRO_ZHA_EzModeReq, ZBPRO_ZHA_EzModeReqDescr_t, ZBPRO_ZHA_EzModeConfParams_t, RPC_C2S_ZBPRO_ZHA_EzModeReq)
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZHA_CIE_ENROLL_FID, ZBPRO_ZHA_CieDeviceEnrollReq, ZBPRO_ZHA_CieEnrollReqDescr_t, ZBPRO_ZHA_CieEnrollConfParams_t, RPC_C2S_ZBPRO_ZHA_CieDeviceEnrollReq)
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZHA_CIE_SET_PANEL_STATUS_REQ_FID, ZBPRO_ZHA_CieDeviceSetPanelStatusReq, ZBPRO_ZHA_CieSetPanelStatusReqDescr_t, ZBPRO_ZHA_CieSetPanelStatusConfParams_t, RPC_C2S_ZBPRO_ZHA_CieDeviceSetPanelStatusReq)
CREATE_SERVER_REQUEST_API_FUNCTION(ZBPRO_ZHA_CIE_ZONE_SET_BYPASS_STATE_REQ_FID, ZBPRO_ZHA_CieZoneSetBypassStateReq, ZBPRO_ZHA_CieZoneSetBypassStateReqDescr_t, ZBPRO_ZHA_CieZoneSetBypassStateConfParams_t, RPC_C2S_ZBPRO_ZHA_CieZoneSetBypassStateReq)
CREATE_SERVER_INDICATION_API_FUNCTION(ZBPRO_ZHA_CIE_SET_PANEL_STATUS_IND_FID, ZBPRO_ZHA_CieDeviceSetPanelStatusInd, ZBPRO_ZHA_CieSetPanelStatusIndParams_t, NoAppropriateType_t, RPC_C2S_ZBPRO_ZHA_CieDeviceSetPanelStatusInd)
/* end of ZCL staff */

#else
#include "zigbee.h"

static list_head clientIndicationListHeader = {
                    .next = &clientIndicationListHeader,
                    .prev = &clientIndicationListHeader,
};

CREATE_CLIENT_REQUEST_API_FUNCTION(RF4CE_MAC_REQ_GET_FID, RF4CE_MAC_GetReq, MAC_GetReqDescr_t, MAC_GetConfParams_t, RPC_C2S_RF4CE_MAC_GetReq)

/* Since this API is never used, we borrow it as a RPC API to get the IEEE address */
CREATE_CLIENT_REQUEST_API_FUNCTION(RF4CE_MAC_REQ_GET_FID, ZBPRO_MAC_GetReq, MAC_GetReqDescr_t, MAC_GetConfParams_t, RPC_C2S_RF4CE_MAC_GetReq)

CREATE_CLIENT_REQUEST_API_FUNCTION(RF4CE_MAC_REQ_SET_FID, RF4CE_MAC_SetReq, MAC_SetReqDescr_t, MAC_SetConfParams_t, RPC_C2S_RF4CE_MAC_SetReq)

CREATE_CLIENT_REQUEST_API_FUNCTION(RF4CE_PROFILE_REQ_RESET_FID, RF4CE_ResetReq, RF4CE_ResetReqDescr_t, RF4CE_StartResetConfParams_t, RPC_C2S_RF4CE_ResetReq)

CREATE_CLIENT_REQUEST_API_FUNCTION(RF4CE_PROFILE_REQ_START_FID, RF4CE_StartReq, RF4CE_StartReqDescr_t, RF4CE_StartResetConfParams_t, RPC_C2S_RF4CE_StartReq)

CREATE_CLIENT_REQUEST_API_FUNCTION(RF4CE_NWK_REQ_DATA_FID, RF4CE_NWK_DataReq, RF4CE_NWK_DataReqDescr_t, RF4CE_NWK_DataConfParams_t, RPC_C2S_RF4CE_DataReq)

CREATE_CLIENT_REQUEST_API_FUNCTION(RF4CE_NWK_REQ_GET_FID, RF4CE_NWK_GetReq, RF4CE_NWK_GetReqDescr_t, RF4CE_NWK_GetConfParams_t, RPC_C2S_RF4CE_GetReq)

CREATE_CLIENT_REQUEST_API_FUNCTION(RF4CE_NWK_REQ_SET_FID, RF4CE_NWK_SetReq, RF4CE_NWK_SetReqDescr_t, RF4CE_NWK_SetConfParams_t, RPC_C2S_RF4CE_SetReq)

CREATE_CLIENT_REQUEST_API_FUNCTION(RF4CE_ZRC1_REQ_GET_FID, RF4CE_ZRC1_GetAttributesReq, RF4CE_ZRC1_GetAttributeDescr_t, RF4CE_ZRC1_GetAttributeConfParams_t, RPC_C2S_RF4CE_ZRC1_GetAttributesReq)

CREATE_CLIENT_REQUEST_API_FUNCTION(RF4CE_ZRC1_REQ_SET_FID, RF4CE_ZRC1_SetAttributesReq, RF4CE_ZRC1_SetAttributeDescr_t, RF4CE_ZRC1_SetAttributeConfParams_t, RPC_C2S_RF4CE_ZRC1_SetAttributesReq)

CREATE_CLIENT_REQUEST_API_FUNCTION(TE_ECHO_FID, Mail_TestEngineEcho, TE_EchoCommandReqDescr_t, TE_EchoCommandConfParams_t, RPC_C2S_Mail_TestEngineEcho)

CREATE_CLIENT_REQUEST_FUNCTION(SYS_EVENT_SUBSCRIBE_FID, SYS_EventSubscribe, SYS_EventHandlerParams_t, RPC_C2S_SYS_EventSubscribe)

CREATE_CLIENT_INDICATION_API_FUNCTION(RF4CE_ZRC2_CHECK_VALIDATION_IND_FID, RF4CE_ZRC2_CheckValidationInd, RF4CE_ZRC2_CheckValidationIndParams_t, NoAppropriateType_t, RPC_S2C_RF4CE_ZRC2_CheckValidationInd)

CREATE_CLIENT_REQUEST_API_FUNCTION(RF4CE_ZRC2_REQ_SET_FID, RF4CE_ZRC2_SetAttributesReq, RF4CE_ZRC2_SetAttributesReqDescr_t, RF4CE_ZRC2_SetAttributesConfParams_t, RPC_C2S_RF4CE_ZRC2_SetAttributesReq)

CREATE_CLIENT_REQUEST_FUNCTION(RF4CE_ZRC2_CHECK_VALIDATION_RESP_FID, RF4CE_ZRC2_CheckValidationResp, RF4CE_ZRC2_CheckValidationRespDescr_t, RPC_C2S_RF4CE_ZRC2_CheckValidationResp)

CREATE_CLIENT_INDICATION_API_FUNCTION(RF4CE_ZRC2_CONTROL_COMMAND_IND_FID, RF4CE_ZRC2_ControlCommandInd, RF4CE_ZRC2_ControlCommandIndParams_t, NoAppropriateType_t, RPC_S2C_RF4CE_ZRC2_ControlCommandInd)

CREATE_CLIENT_INDICATION_API_FUNCTION(RF4CE_ZRC1_IND_CONTROLCOMMAND_FID, RF4CE_ZRC1_ControlCommandInd, RF4CE_ZRC1_ControlCommandIndParams_t, NoAppropriateType_t, RPC_S2C_RF4CE_ZRC1_ControlCommandInd)

CREATE_CLIENT_REQUEST_API_FUNCTION(RF4CE_ZRC1_REQ_TARGET_BIND_FID, RF4CE_ZRC1_TargetBindReq, RF4CE_ZRC1_BindReqDescr_t, RF4CE_ZRC1_BindConfParams_t, RPC_C2S_RF4CE_ZRC1_TargetBindReq)

CREATE_CLIENT_REQUEST_API_FUNCTION(TE_ECHO_DELAY_FID, Mail_SetEchoDelay, TE_SetEchoDelayCommandReqDescr_t, TE_SetEchoDelayCommandConfParams_t, RPC_C2S_Mail_SetEchoDelay)

CREATE_CLIENT_INDICATION_API_FUNCTION(RF4CE_PROFILE_IND_PAIR_FID, RF4CE_PairInd, RF4CE_PairingIndParams_t, NoAppropriateType_t, RPC_S2C_RF4CE_ZRC_PairInd)

CREATE_CLIENT_REQUEST_API_FUNCTION(RF4CE_ZRC_SET_WAKEUP_ACTION_CODE_FID, RF4CE_ZRC_SetWakeUpActionCodeReq, RF4CE_ZRC_SetWakeUpActionCodeReqDescr_t, RF4CE_ZRC_SetWakeUpActionCodeConfParams_t, RPC_C2S_RF4CE_ZRC_SetWakeUpActionCodeKey)

CREATE_CLIENT_REQUEST_API_FUNCTION(RF4CE_ZRC_GET_WAKEUP_ACTION_CODE_FID, RF4CE_ZRC_GetWakeUpActionCodeReq, RF4CE_ZRC_GetWakeUpActionCodeReqDescr_t, RF4CE_ZRC_GetWakeUpActionCodeConfParams_t, RPC_C2S_RF4CE_ZRC_GetWakeUpActionCodeKey)

CREATE_CLIENT_REQUEST_API_FUNCTION(RF4CE_ZRC1_VENDORSPECIFIC_REQ_FID, RF4CE_ZRC1_VendorSpecificReq, RF4CE_ZRC1_VendorSpecificReqDescr_t, RF4CE_ZRC1_VendorSpecificConfParams_t, RPC_C2S_RF4CE_ZRC1_VendorSpecificReq)

CREATE_CLIENT_INDICATION_API_FUNCTION(RF4CE_ZRC1_VENDORSPECIFIC_IND_FID, RF4CE_ZRC1_VendorSpecificInd, RF4CE_ZRC1_VendorSpecificIndParams_t, NoAppropriateType_t, RPC_S2C_RF4CE_ZRC1_VendorSpecificInd)

CREATE_CLIENT_REQUEST_API_FUNCTION(RF4CE_ZRC2_SET_PUSH_BUTTON_STIMULUS_FID, RF4CE_ZRC2_SetPushButtonStimulusReq, RF4CE_ZRC2_ButtonBindingReqDescr_t,RF4CE_ZRC2_BindingConfParams_t, RPC_C2S_RF4CE_ZRC2_SetPushButtonStimulusReq)

CREATE_CLIENT_REQUEST_API_FUNCTION(RF4CE_ZRC2_ENABLE_BINDING_FID, RF4CE_ZRC2_EnableBindingReq, RF4CE_ZRC2_BindingReqDescr_t, RF4CE_ZRC2_BindingConfParams_t, RPC_C2S_RF4CE_ZRC2_EnableBindingReq)

CREATE_CLIENT_REQUEST_API_FUNCTION(RF4CE_PROFILE_REQ_UNPAIR_FID, RF4CE_UnpairReq, RF4CE_UnpairReqDescr_t, RF4CE_UnpairConfParams_t, RPC_C2S_RF4CE_UnpairReq)

/* Virtual UART staff */
CREATE_CLIENT_REQUEST_API_FUNCTION(TE_HOST_TO_UART1_FID, Mail_Host2Uart1, TE_Host2Uart1ReqDescr_t,   NoAppropriateType_t, RPC_C2S_TE_Host2Uart1Req)
/* End virtual UART staff */

/* PHY test stuff */
CREATE_CLIENT_REQUEST_API_FUNCTION(RF4CE_CTRL_TEST_GET_CAPS_FID, Phy_Test_Get_Caps_Req,                             Phy_Test_Get_Caps_ReqDescr_t,   Phy_Test_Get_Caps_ConfParams_t, RPC_C2S_Phy_Test_Get_Caps_Req)
CREATE_CLIENT_REQUEST_API_FUNCTION(RF4CE_CTRL_TEST_SET_CHANNEL_FID, Phy_Test_Set_Channel_Req,                       Phy_Test_Set_Channel_ReqDescr_t,   Phy_Test_Set_Channel_ConfParams_t, RPC_C2S_Phy_Test_Set_Channel_Req)
CREATE_CLIENT_REQUEST_API_FUNCTION(RF4CE_CTRL_TEST_CONTINUOUS_WAVE_START_FID, Phy_Test_Continuous_Wave_Start_Req,   Phy_Test_Continuous_Wave_Start_ReqDescr_t,   Phy_Test_Continuous_Wave_StartStop_ConfParams_t, RPC_C2S_Phy_Test_Continuous_Wave_Start_Req)
CREATE_CLIENT_REQUEST_API_FUNCTION(RF4CE_CTRL_TEST_CONTINUOUS_WAVE_STOP_FID, Phy_Test_Continuous_Wave_Stop_Req,     Phy_Test_Continuous_Wave_Stop_ReqDescr_t,   Phy_Test_Continuous_Wave_StartStop_ConfParams_t, RPC_C2S_Phy_Test_Continuous_Wave_Stop_Req)
CREATE_CLIENT_REQUEST_API_FUNCTION(RF4CE_CTRL_TEST_TRANSMIT_START_FID, Phy_Test_Transmit_Start_Req,                 Phy_Test_Transmit_Start_ReqDescr_t,   Phy_Test_Transmit_StartStop_ConfParams_t, RPC_C2S_Phy_Test_Transmit_Start_Req)
CREATE_CLIENT_REQUEST_API_FUNCTION(RF4CE_CTRL_TEST_TRANSMIT_STOP_FID, Phy_Test_Transmit_Stop_Req,                   Phy_Test_Transmit_Stop_ReqDescr_t,   Phy_Test_Transmit_StartStop_ConfParams_t, RPC_C2S_Phy_Test_Transmit_Stop_Req)
CREATE_CLIENT_REQUEST_API_FUNCTION(RF4CE_CTRL_TEST_RECEIVE_START_FID, Phy_Test_Receive_Start_Req,                   Phy_Test_Receive_Start_ReqDescr_t,   Phy_Test_Receive_StartStop_ConfParams_t, RPC_C2S_Phy_Test_Receive_Start_Req)
CREATE_CLIENT_REQUEST_API_FUNCTION(RF4CE_CTRL_TEST_RECEIVE_STOP_FID, Phy_Test_Receive_Stop_Req,                     Phy_Test_Receive_Stop_ReqDescr_t,   Phy_Test_Receive_StartStop_ConfParams_t, RPC_C2S_Phy_Test_Receive_Stop_Req)
CREATE_CLIENT_REQUEST_API_FUNCTION(RF4CE_CTRL_TEST_ECHO_START_FID, Phy_Test_Echo_Start_Req,                         Phy_Test_Echo_Start_ReqDescr_t,   Phy_Test_Echo_StartStop_ConfParams_t, RPC_C2S_Phy_Test_Echo_Start_Req)
CREATE_CLIENT_REQUEST_API_FUNCTION(RF4CE_CTRL_TEST_ECHO_STOP_FID, Phy_Test_Echo_Stop_Req,                           Phy_Test_Echo_Stop_ReqDescr_t,   Phy_Test_Echo_StartStop_ConfParams_t, RPC_C2S_Phy_Test_Echo_Stop_Req)
CREATE_CLIENT_REQUEST_API_FUNCTION(RF4CE_CTRL_TEST_ENERGY_DETECT_SCAN_FID, Phy_Test_Energy_Detect_Scan_Req,         Phy_Test_Energy_Detect_Scan_ReqDescr_t,   Phy_Test_Energy_Detect_Scan_ConfParams_t, RPC_C2S_Phy_Test_Energy_Detect_Scan_Req)
CREATE_CLIENT_REQUEST_API_FUNCTION(RF4CE_CTRL_TEST_GET_STATS_FID, Phy_Test_Get_Stats_Req,                           Phy_Test_Get_Stats_ReqDescr_t,   Phy_Test_Get_Stats_ConfParams_t, RPC_C2S_Phy_Test_Get_Stats_Req)
CREATE_CLIENT_REQUEST_API_FUNCTION(RF4CE_CTRL_TEST_RESET_STATS_FID, Phy_Test_Reset_Stats_Req,                       Phy_Test_Reset_Stats_ReqDescr_t,   Phy_Test_Reset_Stats_ConfParams_t, RPC_C2S_Phy_Test_Reset_Stats_Req)
CREATE_CLIENT_REQUEST_API_FUNCTION(RF4CE_CTRL_TEST_SET_TX_POWER_FID, Phy_Test_Set_TX_Power_Req,                     Phy_Test_Set_TX_Power_ReqDescr_t,   Phy_Test_Set_TX_Power_ConfParams_t, RPC_C2S_Phy_Test_Set_TX_Power_Req)
CREATE_CLIENT_REQUEST_API_FUNCTION(RF4CE_CTRL_TEST_SELECT_ANTENNA_FID, Phy_Test_SelectAntenna_Req,                  Phy_Test_Select_Antenna_ReqDescr_t,   Phy_Test_Select_Antenna_ConfParams_t, RPC_C2S_Phy_Test_SelectAntenna_Req)
CREATE_CLIENT_REQUEST_API_FUNCTION(RF4CE_CTRL_GET_DIAGNOSTICS_CAPS_FID, RF4CE_Get_Diag_Caps_Req,                    RF4CE_Diag_Caps_ReqDescr_t,   RF4CE_Diag_Caps_ConfParams_t, RPC_C2S_RF4CE_Get_Diag_Caps_Req)
CREATE_CLIENT_REQUEST_API_FUNCTION(RF4CE_CTRL_GET_DIAGNOSTIC_FID, RF4CE_Get_Diag_Req,                               RF4CE_Diag_ReqDescr_t,   RF4CE_Diag_ConfParams_t, RPC_C2S_RF4CE_Get_Diag_Req)

/* End of DirecTV staff */

CREATE_CLIENT_REQUEST_FUNCTION(TE_RESET_FID, Mail_TestEngineReset, TE_ResetCommandReqDescr_t, RPC_S2C_Mail_TestEngineReset)
CREATE_CLIENT_INDICATION_API_FUNCTION(SYS_EVENT_NOTIFY_FID, SYS_EventNtfy, SYS_EventNotifyParams_t, NoAppropriateType_t, RPC_S2C_SYS_EVENTNTFY)
/* NWK staff */
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_NWK_REQ_PERMIT_JOINING_FID, ZBPRO_NWK_PermitJoiningReq, ZBPRO_NWK_PermitJoiningReqDescr_t, ZBPRO_NWK_PermitJoiningConfParams_t, RPC_C2S_ZBPRO_NWK_PermitJoiningReq)
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_NWK_REQ_LEAVE_FID, ZBPRO_NWK_LeaveReq, ZBPRO_NWK_LeaveReqDescr_t, ZBPRO_NWK_LeaveConfParams_t, RPC_C2S_ZBPRO_NWK_LeaveReq)
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_NWK_REQ_GET_KEY_FID, ZBPRO_NWK_GetKeyReq, ZBPRO_NWK_GetKeyReqDescr_t, ZBPRO_NWK_GetKeyConfParams_t, RPC_C2S_ZBPRO_NWK_GetKeyReq)
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_NWK_REQ_SET_KEY_FID, ZBPRO_NWK_SetKeyReq, ZBPRO_NWK_SetKeyReqDescr_t, ZBPRO_NWK_SetKeyConfParams_t, RPC_C2S_ZBPRO_NWK_SetKeyReq)
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_NWK_REQ_ROUTE_DISCOVERY_FID, ZBPRO_NWK_RouteDiscoveryReq, ZBPRO_NWK_RouteDiscoveryReqDescr_t, ZBPRO_NWK_RouteDiscoveryConfParams_t, RPC_C2S_ZBPRO_NWK_RouteDiscoveryReq)
/* End of NWK staff */

/* APS staff */
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_APS_REQ_ENDPOINTREGISTER_FID, ZBPRO_APS_EndpointRegisterReq, ZBPRO_APS_EndpointRegisterReqDescr_t, ZBPRO_APS_EndpointRegisterConfParams_t, RPC_C2S_ZBPRO_APS_EndpointRegisterReq)
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_APS_REQ_ENDPOINTUNREGISTER_FID, ZBPRO_APS_EndpointUnregisterReq, ZBPRO_APS_EndpointUnregisterReqDescr_t, ZBPRO_APS_EndpointRegisterConfParams_t, RPC_C2S_ZBPRO_APS_EndpointUnregisterReq)
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_APS_REQ_DATA_FID, ZBPRO_APS_DataReq, ZBPRO_APS_DataReqDescr_t, ZBPRO_APS_DataConfParams_t, RPC_C2S_ZBPRO_APS_DataReq)
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_APS_REQ_BIND_FID, ZBPRO_APS_BindReq, ZBPRO_APS_BindUnbindReqDescr_t, ZBPRO_APS_BindUnbindConfParams_t, RPC_C2S_ZBPRO_APS_BindReq)
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_APS_REQ_UNBIND_FID, ZBPRO_APS_UnbindReq, ZBPRO_APS_BindUnbindReqDescr_t, ZBPRO_APS_BindUnbindConfParams_t, RPC_C2S_ZBPRO_APS_UnbindReq)
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_APS_REQ_GET_FID, ZBPRO_APS_GetReq, ZBPRO_APS_GetReqDescr_t, ZBPRO_APS_GetConfParams_t, RPC_C2S_ZBPRO_APS_GetReq)
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_APS_REQ_SET_FID, ZBPRO_APS_SetReq, ZBPRO_APS_SetReqDescr_t, ZBPRO_APS_SetConfParams_t, RPC_C2S_ZBPRO_APS_SetReq)
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_APS_REQ_GET_KEY_FID, ZBPRO_APS_GetKeyReq, ZBPRO_APS_GetKeyReqDescr_t, ZBPRO_APS_GetKeyConfParams_t, RPC_C2S_ZBPRO_APS_GetKeyReq)
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_APS_REQ_SET_KEY_FID, ZBPRO_APS_SetKeyReq, ZBPRO_APS_SetKeyReqDescr_t, ZBPRO_APS_SetKeyConfParams_t, RPC_C2S_ZBPRO_APS_SetKeyReq)
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_APS_REQ_ADDGROUP_FID, ZBPRO_APS_AddGroupReq, ZBPRO_APS_AddGroupReqDescr_t, ZBPRO_APS_AddGroupConfParams_t, RPC_C2S_ZBPRO_APS_AddGroupReq)
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_APS_REQ_REMOVEGROUP_FID, ZBPRO_APS_RemoveGroupReq, ZBPRO_APS_RemoveGroupReqDescr_t, ZBPRO_APS_RemoveGroupConfParams_t, RPC_C2S_ZBPRO_APS_RemoveGroupReq)
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_APS_REQ_REMOVEALLGROUPS_FID, ZBPRO_APS_RemoveAllGroupsReq, ZBPRO_APS_RemoveAllGroupsReqDescr_t, ZBPRO_APS_RemoveAllGroupsConfParams_t, RPC_C2S_ZBPRO_APS_RemoveAllGroupsReq)
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_APS_REQ_TRANSPORTKEY_FID, ZBPRO_APS_TransportKeyReq, ZBPRO_APS_TransportKeyReqDescr_t, ZBPRO_APS_SecurityServicesConfParams_t, RPC_C2S_ZBPRO_APS_TransportKeyReq)
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_APS_REQ_UPDATEDEVICE_FID, ZBPRO_APS_UpdateDeviceReq, ZBPRO_APS_UpdateDeviceReqDescr_t, ZBPRO_APS_SecurityServicesConfParams_t, RPC_C2S_ZBPRO_APS_UpdateDeviceReq)
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_APS_REQ_REMOTEDEVICE_FID, ZBPRO_APS_RemoveDeviceReq, ZBPRO_APS_RemoveDeviceReqDescr_t, ZBPRO_APS_SecurityServicesConfParams_t, RPC_C2S_ZBPRO_APS_RemoveDeviceReq)
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_APS_REQ_REQUESTKEY_FID, ZBPRO_APS_RequestKeyReq, ZBPRO_APS_RequestKeyReqDescr_t, ZBPRO_APS_SecurityServicesConfParams_t, RPC_C2S_ZBPRO_APS_RequestKeyReq)
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_APS_REQ_SWITCHKEY_FID, ZBPRO_APS_SwitchKeyReq, ZBPRO_APS_SwitchKeyReqDescr_t, ZBPRO_APS_SecurityServicesConfParams_t, RPC_C2S_ZBPRO_APS_SwitchKeyReq)
/* End of APS staff */

/* ZDO staff */
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZDO_REQ_ADDR_RESOLVING_FID, ZBPRO_ZDO_AddrResolvingReq, ZBPRO_ZDO_AddrResolvingReqDescr_t, ZBPRO_ZDO_AddrResolvingConfParams_t, RPC_C2S_ZBPRO_ZDO_AddrResolvingReq)
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZDO_REQ_NODE_DESC_FID, ZBPRO_ZDO_NodeDescReq, ZBPRO_ZDO_NodeDescReqDescr_t, ZBPRO_ZDO_NodeDescConfParams_t, RPC_C2S_ZBPRO_ZDO_NodeDescReq)
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZDO_REQ_POWER_DESC_FID, ZBPRO_ZDO_PowerDescReq, ZBPRO_ZDO_PowerDescReqDescr_t, ZBPRO_ZDO_PowerDescConfParams_t, RPC_C2S_ZBPRO_ZDO_PowerDescReq)
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZDO_REQ_SIMPLE_DESC_FID, ZBPRO_ZDO_SimpleDescReq, ZBPRO_ZDO_SimpleDescReqDescr_t, ZBPRO_ZDO_SimpleDescConfParams_t, RPC_C2S_ZBPRO_ZDO_SimpleDescReq)
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZDO_REQ_ACTIVE_EP_FID, ZBPRO_ZDO_ActiveEpReq, ZBPRO_ZDO_ActiveEpReqDescr_t, ZBPRO_ZDO_ActiveEpConfParams_t, RPC_C2S_ZBPRO_ZDO_ActiveEpReq)
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZDO_REQ_MATCH_DESC_FID, ZBPRO_ZDO_MatchDescReq, ZBPRO_ZDO_MatchDescReqDescr_t, ZBPRO_ZDO_MatchDescConfParams_t, RPC_C2S_ZBPRO_ZDO_MatchDescReq)
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZDO_REQ_DEVICE_ANNCE_FID, ZBPRO_ZDO_DeviceAnnceReq, ZBPRO_ZDO_DeviceAnnceReqDescr_t, ZBPRO_ZDO_DeviceAnnceConfParams_t, RPC_C2S_ZBPRO_ZDO_DeviceAnnceReq)
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZDO_REQ_ED_BIND_FID, ZBPRO_ZDO_EndDeviceBindReq, ZBPRO_ZDO_EndDeviceBindReqDescr_t, ZBPRO_ZDO_BindConfParams_t, RPC_C2S_ZBPRO_ZDO_EndDeviceBindReq)
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZDO_REQ_BIND_FID, ZBPRO_ZDO_BindReq, ZBPRO_ZDO_BindUnbindReqDescr_t, ZBPRO_ZDO_BindConfParams_t, RPC_C2S_ZBPRO_ZDO_BindReq)
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZDO_REQ_UNBIND_FID, ZBPRO_ZDO_UnbindReq, ZBPRO_ZDO_BindUnbindReqDescr_t, ZBPRO_ZDO_BindConfParams_t, RPC_C2S_ZBPRO_ZDO_UnbindReq)
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZDO_REQ_START_NETWORK_FID, ZBPRO_ZDO_StartNetworkReq, ZBPRO_ZDO_StartNetworkReqDescr_t, ZBPRO_ZDO_StartNetworkConfParams_t, RPC_C2S_ZBPRO_ZDO_StartNetworkReq)
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZDO_REQ_MGMT_LEAVE_FID, ZBPRO_ZDO_MgmtLeaveReq, ZBPRO_ZDO_MgmtLeaveReqDescr_t, ZBPRO_ZDO_MgmtLeaveConfParams_t, RPC_C2S_ZBPRO_ZDO_MgmtLeaveReq)
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZDO_REQ_MGMT_PERMIT_JOINING_FID, ZBPRO_ZDO_MgmtPermitJoiningReq, ZBPRO_ZDO_MgmtPermitJoiningReqDescr_t, ZBPRO_ZDO_MgmtPermitJoiningConfParams_t, RPC_C2S_ZBPRO_ZDO_MgmtPermitJoiningReq)
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZDO_REQ_MGMT_NWK_UPDATE_FID, ZBPRO_ZDO_MgmtNwkUpdateReq, ZBPRO_ZDO_MgmtNwkUpdateReqDescr_t, ZBPRO_ZDO_MgmtNwkUpdateConfParams_t, RPC_C2S_ZBPRO_ZDO_MgmtNwkUpdateReq)
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZDO_RESP_MGMT_NWK_UPDATE_UNSOL_FID, ZBPRO_ZDO_MgmtNwkUpdateUnsolResp, ZBPRO_ZDO_MgmtNwkUpdateUnsolRespDescr_t, ZBPRO_ZDO_MgmtNwkUpdateUnsolConfParams_t, RPC_C2S_ZBPRO_ZDO_MgmtNwkUpdateUnsolResp)
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZDO_REQ_MGMT_LQI_FID, ZBPRO_ZDO_MgmtLqiReq, ZBPRO_ZDO_MgmtLqiReqDescr_t, ZBPRO_ZDO_MgmtLqiConfParams_t, RPC_C2S_ZBPRO_ZDO_MgmtLqiReq)
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZDO_REQ_MGMT_BIND_FID, ZBPRO_ZDO_MgmtBindReq, ZBPRO_ZDO_MgmtBindReqDescr_t, ZBPRO_ZDO_MgmtBindConfParams_t, RPC_C2S_ZBPRO_ZDO_MgmtBindReq)
CREATE_CLIENT_INDICATION_API_FUNCTION(ZBPRO_ZDO_IND_MGMT_NWK_UPDATE_UNSOL_FID, ZBPRO_ZDO_MgmtNwkUpdateUnsolInd, ZBPRO_ZDO_MgmtNwkUpdateUnsolIndParams_t, NoAppropriateType_t, RPC_C2S_ZBPRO_ZDO_MgmtNwkUpdateUnsolInd)
/* End of ZDO staff */

/* ZCL staff */
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_TC_REQ_NWK_KEY_UPDATE_FID, ZBPRO_TC_NwkKeyUpdateReq, ZBPRO_TC_NwkKeyUpdateReqDescr_t, ZBPRO_TC_NwkKeyUpdateConfParams_t, RPC_C2S_ZBPRO_TC_NwkKeyUpdateReq)
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_REQ_SET_POWER_SOURCE_FID, ZBPRO_ZCL_SetPowerSourceReq, ZBPRO_ZCL_SetPowerSourceReqDescr_t, ZBPRO_ZCL_SetPowerSourceConfParams_t, RPC_C2S_ZBPRO_ZCL_SetPowerSourceReq)
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_PROFILE_WIDE_CMD_DISCOVER_ATTRIBUTE_REQ_FID, ZBPRO_ZCL_ProfileWideCmdDiscoverAttrReq, ZBPRO_ZCL_ProfileWideCmdDiscoverAttrDescr_t, ZBPRO_ZCL_ProfileWideCmdDiscoverAttrConfParams_t, RPC_C2S_ZBPRO_ZCL_ProfileWideCmdDiscoverAttrReq)
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_PROFILE_WIDE_CMD_READ_ATTRIBUTE_REQ_FID, ZBPRO_ZCL_ProfileWideCmdReadAttributesReq, ZBPRO_ZCL_ProfileWideCmdReadAttrReqDescr_t, ZBPRO_ZCL_ProfileWideCmdReadAttrConfParams_t, RPC_C2S_ZBPRO_ZCL_ProfileWideCmdReadAttributesReq)
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_PROFILE_WIDE_CMD_WRITE_ATTRIBUTE_REQ_FID, ZBPRO_ZCL_ProfileWideCmdWriteAttributesReq, ZBPRO_ZCL_ProfileWideCmdWriteAttrReqDescr_t, ZBPRO_ZCL_ProfileWideCmdWriteAttrConfParams_t, RPC_C2S_ZBPRO_ZCL_ProfileWideCmdWriteAttributesReq)
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_PROFILE_WIDE_CMD_CONFIGURE_REPORTING_REQ_FID, ZBPRO_ZCL_ProfileWideCmdConfigureReportingReq, ZBPRO_ZCL_ProfileWideCmdConfigureReportingReqDescr_t, ZBPRO_ZCL_ProfileWideCmdConfigureReportingConfParams_t, RPC_C2S_ZBPRO_ZCL_ProfileWideCmdConfigureReportingReq)
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_PROFILE_WIDE_CMD_READ_REPORTING_CONFIGURATION_REQ_FID, ZBPRO_ZCL_ProfileWideCmdReadReportingConfigurationReq, ZBPRO_ZCL_ProfileWideCmdReadReportingConfigurationReqDescr_t, ZBPRO_ZCL_ProfileWideCmdReadReportingConfigurationConfParams_t, RPC_C2S_ZBPRO_ZCL_ProfileWideCmdReadReportingConfigurationReq)
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_IDENTIFY_CMD_IDENTIFY_REQ_FID, ZBPRO_ZCL_IdentifyCmdIdentifyReq, ZBPRO_ZCL_IdentifyCmdIdentifyReqDescr_t, ZBPRO_ZCL_IdentifyCmdConfParams_t, RPC_C2S_ZBPRO_ZCL_IdentifyCmdIdentifyReq)
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_IDENTIFY_CMD_IDENTIFY_QUERY_REQ_FID, ZBPRO_ZCL_IdentifyCmdIdentifyQueryReq, ZBPRO_ZCL_IdentifyCmdIdentifyQueryReqDescr_t, ZBPRO_ZCL_IdentifyCmdConfParams_t, RPC_C2S_ZBPRO_ZCL_IdentifyCmdIdentifyQueryReq)
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_GROUPS_CMD_ADD_GROUP_REQ_FID, ZBPRO_ZCL_GroupsCmdAddGroupReq, ZBPRO_ZCL_GroupsCmdAddGroupReqDescr_t, ZBPRO_ZCL_GroupsCmdAddGroupConfParams_t, RPC_C2S_ZBPRO_ZCL_GroupsCmdAddGroupReq)
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_GROUPS_CMD_VIEW_GROUP_REQ_FID, ZBPRO_ZCL_GroupsCmdViewGroupReq, ZBPRO_ZCL_GroupsCmdViewGroupReqDescr_t, ZBPRO_ZCL_GroupsCmdViewGroupConfParams_t, RPC_C2S_ZBPRO_ZCL_GroupsCmdViewGroupReq)
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_GROUPS_CMD_GET_GROUP_MEMBERSHIP_REQ_FID, ZBPRO_ZCL_GroupsCmdGetGroupMembershipReq, ZBPRO_ZCL_GroupsCmdGetGroupMembershipReqDescr_t, ZBPRO_ZCL_GroupsCmdGetGroupMembershipConfParams_t, RPC_C2S_ZBPRO_ZCL_GroupsCmdGetGroupMembershipReq)
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_GROUPS_CMD_REMOVE_GROUP_REQ_FID, ZBPRO_ZCL_GroupsCmdRemoveGroupReq, ZBPRO_ZCL_GroupsCmdRemoveGroupReqDescr_t, ZBPRO_ZCL_GroupsCmdRemoveGroupConfParams_t, RPC_C2S_ZBPRO_ZCL_GroupsCmdRemoveGroupReq)
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_GROUPS_CMD_REMOVE_ALL_GROUPS_REQ_FID, ZBPRO_ZCL_GroupsCmdRemoveAllGroupsReq, ZBPRO_ZCL_GroupsCmdRemoveAllGroupsReqDescr_t, ZBPRO_ZCL_GroupsCmdRemoveAllGroupsConfParams_t, RPC_C2S_ZBPRO_ZCL_GroupsCmdRemoveAllGroupsReq)
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_GROUPS_CMD_ADD_GROUP_IF_IDENTIFY_REQ_FID, ZBPRO_ZCL_GroupsCmdAddGroupIfIdentifyReq, ZBPRO_ZCL_GroupsCmdAddGroupIfIdentifyReqDescr_t, ZBPRO_ZCL_GroupsCmdAddGroupIfIdentifyConfParams_t, RPC_C2S_ZBPRO_ZCL_GroupsCmdAddGroupIfIdentifyReq)
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_SCENES_CMD_ADD_SCENE_REQ_FID, ZBPRO_ZCL_ScenesCmdAddSceneReq, ZBPRO_ZCL_ScenesCmdAddSceneReqDescr_t, ZBPRO_ZCL_ScenesCmdAddSceneConfParams_t, RPC_C2S_ZBPRO_ZCL_ScenesCmdAddSceneReq)
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_SCENES_CMD_VIEW_SCENE_REQ_FID, ZBPRO_ZCL_ScenesCmdViewSceneReq, ZBPRO_ZCL_ScenesCmdViewSceneReqDescr_t, ZBPRO_ZCL_ScenesCmdViewSceneConfParams_t, RPC_C2S_ZBPRO_ZCL_ScenesCmdViewSceneReq)
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_SCENES_CMD_STORE_SCENE_REQ_FID, ZBPRO_ZCL_ScenesCmdStoreSceneReq, ZBPRO_ZCL_ScenesCmdStoreSceneReqDescr_t, ZBPRO_ZCL_ScenesCmdStoreSceneConfParams_t, RPC_C2S_ZBPRO_ZCL_ScenesCmdStoreSceneReq)
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_SCENES_CMD_RECALL_SCENE_REQ_FID, ZBPRO_ZCL_ScenesCmdRecallSceneReq, ZBPRO_ZCL_ScenesCmdRecallSceneReqDescr_t, ZBPRO_ZCL_ScenesCmdConfParams_t, RPC_C2S_ZBPRO_ZCL_ScenesCmdRecallSceneReq)
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_SCENES_CMD_REMOVE_SCENE_REQ_FID, ZBPRO_ZCL_ScenesCmdRemoveSceneReq, ZBPRO_ZCL_ScenesCmdRemoveSceneReqDescr_t, ZBPRO_ZCL_ScenesCmdRemoveSceneConfParams_t, RPC_C2S_ZBPRO_ZCL_ScenesCmdRemoveSceneReq)
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_SCENES_CMD_REMOVE_ALL_SCENES_REQ_FID, ZBPRO_ZCL_ScenesCmdRemoveAllScenesReq, ZBPRO_ZCL_ScenesCmdRemoveAllScenesReqDescr_t, ZBPRO_ZCL_ScenesCmdRemoveAllScenesConfParams_t, RPC_C2S_ZBPRO_ZCL_ScenesCmdRemoveAllScenesReq)
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_SCENES_CMD_GET_SCENE_MEMBERSHIP_REQ_FID, ZBPRO_ZCL_ScenesCmdGetSceneMembershipReq, ZBPRO_ZCL_ScenesCmdGetSceneMembershipReqDescr_t, ZBPRO_ZCL_ScenesCmdConfParams_t, RPC_C2S_ZBPRO_ZCL_ScenesCmdGetSceneMembershipReq)
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_ONOFF_CMD_OFF_REQ_FID, ZBPRO_ZCL_OnOffCmdOffReq, ZBPRO_ZCL_OnOffCmdReqDescr_t, ZBPRO_ZCL_OnOffCmdConfParams_t, RPC_C2S_ZBPRO_ZCL_OnOffCmdOffReq)
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_ONOFF_CMD_ON_REQ_FID, ZBPRO_ZCL_OnOffCmdOnReq, ZBPRO_ZCL_OnOffCmdReqDescr_t, ZBPRO_ZCL_OnOffCmdConfParams_t, RPC_C2S_ZBPRO_ZCL_OnOffCmdOnReq)
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_ONOFF_CMD_TOGGLE_REQ_FID, ZBPRO_ZCL_OnOffCmdToggleReq, ZBPRO_ZCL_OnOffCmdReqDescr_t, ZBPRO_ZCL_OnOffCmdConfParams_t, RPC_C2S_ZBPRO_ZCL_OnOffCmdToggleReq)
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_LEVEL_CONTROL_CMD_MOVE_TO_LEVEL_REQ_FID, ZBPRO_ZCL_LevelControlCmdMoveToLevelReq, ZBPRO_ZCL_LevelControlCmdMoveToLevelReqDescr_t, ZBPRO_ZCL_LevelControlCmdConfParams_t, RPC_C2S_ZBPRO_ZCL_LevelControlCmdMoveToLevelReq)
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_LEVEL_CONTROL_CMD_MOVE_REQ_FID, ZBPRO_ZCL_LevelControlCmdMoveReq, ZBPRO_ZCL_LevelControlCmdMoveReqDescr_t, ZBPRO_ZCL_LevelControlCmdConfParams_t, RPC_C2S_ZBPRO_ZCL_LevelControlCmdMoveReq)
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_LEVEL_CONTROL_CMD_STEP_REQ_FID, ZBPRO_ZCL_LevelControlCmdStepReq, ZBPRO_ZCL_LevelControlCmdStepReqDescr_t, ZBPRO_ZCL_LevelControlCmdConfParams_t, RPC_C2S_ZBPRO_ZCL_LevelControlCmdStepReq)
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_LEVEL_CONTROL_CMD_STOP_REQ_FID, ZBPRO_ZCL_LevelControlCmdStopReq, ZBPRO_ZCL_LevelControlCmdStopReqDescr_t, ZBPRO_ZCL_LevelControlCmdConfParams_t, RPC_C2S_ZBPRO_ZCL_LevelControlCmdStopReq)
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_DOOR_LOCK_CMD_LOCK_REQ_FID, ZBPRO_ZCL_DoorLockCmdLockReq, ZBPRO_ZCL_DoorLockCmdLockUnlockReqDescr_t, ZBPRO_ZCL_DoorLockCmdLockUnlockConfParams_t, RPC_C2S_ZBPRO_ZCL_DoorLockCmdLockReq)
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_DOOR_LOCK_CMD_UNLOCK_REQ_FID, ZBPRO_ZCL_DoorLockCmdUnlockReq, ZBPRO_ZCL_DoorLockCmdLockUnlockReqDescr_t, ZBPRO_ZCL_DoorLockCmdLockUnlockConfParams_t, RPC_C2S_ZBPRO_ZCL_DoorLockCmdUnlockReq)
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_WINDOW_COVERING_CMD_UP_OPEN_REQ_FID, ZBPRO_ZCL_WindowCoveringCmdUpOpenReq, ZBPRO_ZCL_WindowCoveringCmdReqDescr_t, ZBPRO_ZCL_WindowCoveringCmdConfParams_t, RPC_C2S_ZBPRO_ZCL_WindowCoveringCmdUpOpenReq)
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_WINDOW_COVERING_CMD_DOWN_CLOSE_REQ_FID, ZBPRO_ZCL_WindowCoveringCmdDownCloseReq, ZBPRO_ZCL_WindowCoveringCmdReqDescr_t, ZBPRO_ZCL_WindowCoveringCmdConfParams_t, RPC_C2S_ZBPRO_ZCL_WindowCoveringCmdDownCloseReq)
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_WINDOW_COVERING_CMD_STOP_REQ_FID, ZBPRO_ZCL_WindowCoveringCmdStopReq, ZBPRO_ZCL_WindowCoveringCmdReqDescr_t, ZBPRO_ZCL_WindowCoveringCmdConfParams_t, RPC_C2S_ZBPRO_ZCL_WindowCoveringCmdStopReq)
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_WINDOW_COVERING_CMD_GO_TO_LIFT_PERCENTAGE_REQ_FID, ZBPRO_ZCL_WindowCoveringCmdGotoLiftPecentageReq, ZBPRO_ZCL_WindowCoveringLiftTiltPercentCmdReqDescr_t, ZBPRO_ZCL_WindowCoveringCmdConfParams_t, RPC_C2S_ZBPRO_ZCL_WindowCoveringCmdGotoLiftPecentageReq)
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_WINDOW_COVERING_CMD_GO_TO_TILT_PERCENTAGE_REQ_FID, ZBPRO_ZCL_WindowCoveringCmdGotoTiltPecentageReq, ZBPRO_ZCL_WindowCoveringLiftTiltPercentCmdReqDescr_t, ZBPRO_ZCL_WindowCoveringCmdConfParams_t, RPC_C2S_ZBPRO_ZCL_WindowCoveringCmdGotoTiltPecentageReq)
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_IAS_ZONE_CMD_ZONE_ENROLL_RESPONSE_REQ_FID, ZBPRO_ZCL_IASZoneCmdZoneEnrollResponseReq, ZBPRO_ZCL_IASZoneCmdZoneEnrollResponseReqDescr_t, ZBPRO_ZCL_IASZoneCmdZoneEnrollResponseConfParams_t, RPC_C2S_ZBPRO_ZCL_IASZoneCmdZoneEnrollResponseReq)
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_IAS_ACE_ARM_RESP_REQ_FID, ZBPRO_ZCL_SapIasAceArmRespReq, ZBPRO_ZCL_SapIasAceArmRespReqDescr_t, ZBPRO_ZCL_SapIasAceRespReqConfParams_t, RPC_C2S_ZBPRO_ZCL_SapIasAceArmRespReq)
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_IAS_ACE_BYPASS_RESP_REQ_FID, ZBPRO_ZCL_SapIasAceBypassRespReq, ZBPRO_ZCL_SapIasAceBypassRespReqDescr_t, ZBPRO_ZCL_SapIasAceRespReqConfParams_t, RPC_C2S_ZBPRO_ZCL_SapIasAceBypassRespReq)
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_IAS_ACE_GET_ZONE_ID_MAP_RESP_REQ_FID, ZBPRO_ZCL_SapIasAceGetZoneIdMapRespReq, ZBPRO_ZCL_SapIasAceGetZoneIdMapRespReqDescr_t, ZBPRO_ZCL_SapIasAceRespReqConfParams_t, RPC_C2S_ZBPRO_ZCL_SapIasAceGetZoneIdMapRespReq)
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_IAS_ACE_GET_ZONE_INFO_RESP_REQ_FID, ZBPRO_ZCL_SapIasAceGetZoneInfoRespReq, ZBPRO_ZCL_SapIasAceGetZoneInfoRespReqDescr_t, ZBPRO_ZCL_SapIasAceRespReqConfParams_t, RPC_C2S_ZBPRO_ZCL_SapIasAceGetZoneInfoRespReq)
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_IAS_ACE_GET_PANEL_STATUS_RESP_REQ_FID, ZBPRO_ZCL_SapIasAceGetPanelStatusRespReq, ZBPRO_ZCL_SapIasAceGetPanelStatusRespReqDescr_t, ZBPRO_ZCL_SapIasAceRespReqConfParams_t, RPC_C2S_ZBPRO_ZCL_SapIasAceGetPanelStatusRespReq)
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_IAS_ACE_SET_BYPASSED_ZONE_LIST_RESP_REQ_FID, ZBPRO_ZCL_SapIasAceSetBypassedZoneListRespReq, ZBPRO_ZCL_SapIasAceSetBypassedZoneListRespReqDescr_t, ZBPRO_ZCL_SapIasAceRespReqConfParams_t, RPC_C2S_ZBPRO_ZCL_SapIasAceSetBypassedZoneListRespReq)
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_IAS_ACE_GET_ZONE_STATUS_RESP_REQ_FID, ZBPRO_ZCL_SapIasAceGetZoneStatusRespReq, ZBPRO_ZCL_SapIasAceGetZoneStatusRespReqDescr_t, ZBPRO_ZCL_SapIasAceRespReqConfParams_t, RPC_C2S_ZBPRO_ZCL_SapIasAceGetZoneStatusRespReq)
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_IAS_ACE_ZONE_STATUS_CHANGED_REQ_FID, ZBPRO_ZCL_SapIasAceZoneStatusChangedReq, ZBPRO_ZCL_SapIasAceZoneStatusChangedReqDescr_t, ZBPRO_ZCL_SapIasAceRespReqConfParams_t, RPC_C2S_ZBPRO_ZCL_SapIasAceZoneStatusChangedReq)
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_IAS_ACE_PANEL_STATUS_CHANGED_REQ_FID, ZBPRO_ZCL_SapIasAcePanelStatusChangedReq, ZBPRO_ZCL_SapIasAcePanelStatusChangedReqDescr_t, ZBPRO_ZCL_SapIasAceRespReqConfParams_t, RPC_C2S_ZBPRO_ZCL_SapIasAcePanelStatusChangedReq)
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_IAS_WD_CMD_START_WARNING_REQ_FID, ZBPRO_ZCL_IASWDCmdStartWarningReq, ZBPRO_ZCL_IASWDCmdStartWarningReqDescr_t, ZBPRO_ZCL_IASWDCmdStartWarningConfParams_t, RPC_C2S_ZBPRO_ZCL_IASWDCmdStartWarningReq)
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_IAS_WD_CMD_SQUAWK_REQ_FID, ZBPRO_ZCL_IASWDCmdSquawkgReq, ZBPRO_ZCL_IASWDCmdSquawkReqDescr_t, ZBPRO_ZCL_IASWDCmdSquawkConfParams_t, RPC_C2S_ZBPRO_ZCL_IASWDCmdSquawkgReq)
CREATE_CLIENT_INDICATION_API_FUNCTION(ZBPRO_ZCL_IND_IDENTIFY_FID, ZBPRO_ZCL_IdentifyInd, ZBPRO_ZCL_IdentifyIndParams_t, NoAppropriateType_t, RPC_C2S_ZBPRO_ZCL_IdentifyInd)
CREATE_CLIENT_INDICATION_API_FUNCTION(ZBPRO_ZCL_PROFILE_WIDE_CMD_REPORT_ATTRIBUTES_IND_FID, ZBPRO_ZCL_ProfileWideCmdReportAttributesInd, ZBPRO_ZCL_ProfileWideCmdReportAttributesIndParams_t, NoAppropriateType_t, RPC_C2S_ZBPRO_ZCL_ProfileWideCmdReportAttributesInd)
CREATE_CLIENT_INDICATION_API_FUNCTION(ZBPRO_ZCL_GROUPS_CMD_GET_GROUP_MEMBERSHIP_RESPONSE_IND_FID, ZBPRO_ZCL_GroupsCmdGetGroupMembershipResponseInd, ZBPRO_ZCL_GroupsCmdGetGroupMembershipIndParams_t, NoAppropriateType_t, RPC_C2S_ZBPRO_ZCL_GroupsCmdGetGroupMembershipResponseInd)
CREATE_CLIENT_INDICATION_API_FUNCTION(ZBPRO_ZCL_SCENES_CMD_GET_SCENE_MEMBERSHIP_RESPONSE_IND_FID, ZBPRO_ZCL_ScenesCmdGetSceneMembershipResponseInd, ZBPRO_ZCL_ScenesCmdGetSceneMembershipResponseIndParams_t, NoAppropriateType_t, RPC_C2S_ZBPRO_ZCL_ScenesCmdGetSceneMembershipResponseInd)
CREATE_CLIENT_INDICATION_API_FUNCTION(ZBPRO_ZCL_IAS_ZONE_CMD_ZONE_ENROLL_IND_FID, ZBPRO_ZCL_IASZoneCmdZoneEnrollRequestInd, ZBPRO_ZCL_IASZoneCmdZoneEnrollRequestIndParams_t, NoAppropriateType_t, RPC_C2S_ZBPRO_ZCL_IASZoneCmdZoneEnrollRequestInd)
CREATE_CLIENT_INDICATION_API_FUNCTION(ZBPRO_ZCL_IAS_ZONE_CMD_ZONE_STATUS_CHANGED_NOTIFICATION_IND_FID, ZBPRO_ZCL_IASZoneCmdZoneStatusChangeNotificationInd, ZBPRO_ZCL_IASZoneCmdZoneStatusChangeNotificationIndParams_t, NoAppropriateType_t, RPC_C2S_ZBPRO_ZCL_IASZoneCmdZoneStatusChangeNotificationInd)
CREATE_CLIENT_INDICATION_API_FUNCTION(ZBPRO_ZCL_IAS_ACE_ARM_IND_FID, ZBPRO_ZCL_SapIasAceArmInd, ZBPRO_ZCL_SapIasAceArmIndParams_t, NoAppropriateType_t, RPC_C2S_ZBPRO_ZCL_SapIasAceArmInd)
CREATE_CLIENT_INDICATION_API_FUNCTION(ZBPRO_ZCL_IAS_ACE_BYPASS_IND_FID, ZBPRO_ZCL_SapIasAceBypassInd, ZBPRO_ZCL_SapIasAceBypassIndParams_t, NoAppropriateType_t, RPC_C2S_ZBPRO_ZCL_SapIasAceBypassInd)
CREATE_CLIENT_INDICATION_API_FUNCTION(ZBPRO_ZCL_IAS_ACE_EMERGENCY_IND_FID, ZBPRO_ZCL_SapIasAceEmergencyInd, ZBPRO_ZCL_SapIasAceAlarmIndParams_t, NoAppropriateType_t, RPC_C2S_ZBPRO_ZCL_SapIasAceEmergencyInd)
CREATE_CLIENT_INDICATION_API_FUNCTION(ZBPRO_ZCL_IAS_ACE_FIRE_IND_FID, ZBPRO_ZCL_SapIasAceFireInd, ZBPRO_ZCL_SapIasAceAlarmIndParams_t, NoAppropriateType_t, RPC_C2S_ZBPRO_ZCL_SapIasAceFireInd)
CREATE_CLIENT_INDICATION_API_FUNCTION(ZBPRO_ZCL_IAS_ACE_PANIC_IND_FID, ZBPRO_ZCL_SapIasAcePanicInd, ZBPRO_ZCL_SapIasAceAlarmIndParams_t, NoAppropriateType_t, RPC_C2S_ZBPRO_ZCL_SapIasAcePanicInd)
CREATE_CLIENT_INDICATION_API_FUNCTION(ZBPRO_ZCL_IAS_ACE_GET_ZONE_ID_MAP_IND_FID, ZBPRO_ZCL_SapIasAceGetZoneIdMapInd, ZBPRO_ZCL_SapIasAceGetZoneIdMapIndParams_t, NoAppropriateType_t, RPC_C2S_ZBPRO_ZCL_SapIasAceGetZoneIdMapInd)
CREATE_CLIENT_INDICATION_API_FUNCTION(ZBPRO_ZCL_IAS_ACE_GET_ZONE_INFO_IND_FID, ZBPRO_ZCL_SapIasAceGetZoneInfoInd, ZBPRO_ZCL_SapIasAceGetZoneInfoIndParams_t, NoAppropriateType_t, RPC_C2S_ZBPRO_ZCL_SapIasAceGetZoneInfoInd)
CREATE_CLIENT_INDICATION_API_FUNCTION(ZBPRO_ZCL_IAS_ACE_GET_PANEL_STATUS_IND_FID, ZBPRO_ZCL_SapIasAceGetPanelStatusInd, ZBPRO_ZCL_SapIasAceGetPanelStatusIndParams_t, NoAppropriateType_t, RPC_C2S_ZBPRO_ZCL_SapIasAceGetPanelStatusInd)
CREATE_CLIENT_INDICATION_API_FUNCTION(ZBPRO_ZCL_IAS_ACE_GET_BYPASSED_ZONE_LIST_IND_FID, ZBPRO_ZCL_SapIasAceGetBypassedZoneListInd, ZBPRO_ZCL_SapIasAceGetBypassedZoneListIndParams_t, NoAppropriateType_t, RPC_C2S_ZBPRO_ZCL_SapIasAceGetBypassedZoneListInd)
CREATE_CLIENT_INDICATION_API_FUNCTION(ZBPRO_ZCL_IAS_ACE_GET_ZONE_STATUS_IND_FID, ZBPRO_ZCL_SapIasAceGetZoneStatusInd, ZBPRO_ZCL_SapIasAceGetZoneStatusIndParams_t, NoAppropriateType_t, RPC_C2S_ZBPRO_ZCL_SapIasAceGetZoneStatusInd)
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_COLOR_CONTROL_CMD_MOVE_TO_COLOR_REQ_FID, ZBPRO_ZCL_ColorControlCmdMoveToColorReq, ZBPRO_ZCL_ColorControlCmdMoveToColorReqDescr_t, ZBPRO_ZCL_ColorControlCmdMoveToColorConfParams_t, RPC_C2S_ZBPRO_ZCL_ColorControlCmdMoveToColorReq)
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_COLOR_CONTROL_CMD_MOVE_COLOR_REQ_FID, ZBPRO_ZCL_ColorControlCmdMoveColorReq, ZBPRO_ZCL_ColorControlCmdMoveColorReqDescr_t, ZBPRO_ZCL_ColorControlCmdMoveColorConfParams_t, RPC_C2S_ZBPRO_ZCL_ColorControlCmdMoveColorReq)
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_COLOR_CONTROL_CMD_STEP_COLOR_REQ_FID, ZBPRO_ZCL_ColorControlCmdStepColorReq, ZBPRO_ZCL_ColorControlCmdStepColorReqDescr_t, ZBPRO_ZCL_ColorControlCmdStepColorConfParams_t, RPC_C2S_ZBPRO_ZCL_ColorControlCmdStepColorReq)
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_COLOR_CONTROL_CMD_ENHANCED_MOVE_TO_HUE_REQ_FID, ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueReq, ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueReqDescr_t, ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueConfParams_t, RPC_C2S_ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueReq)
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_COLOR_CONTROL_CMD_ENHANCED_MOVE_HUE_REQ_FID, ZBPRO_ZCL_ColorControlCmdEnhancedMoveHueReq, ZBPRO_ZCL_ColorControlCmdEnhancedMoveHueReqDescr_t, ZBPRO_ZCL_ColorControlCmdEnhancedMoveHueConfParams_t, RPC_C2S_ZBPRO_ZCL_ColorControlCmdEnhancedMoveHueReq)
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_COLOR_CONTROL_CMD_ENHANCED_STEP_HUE_REQ_FID, ZBPRO_ZCL_ColorControlCmdEnhancedStepHueReq, ZBPRO_ZCL_ColorControlCmdEnhancedStepHueReqDescr_t, ZBPRO_ZCL_ColorControlCmdEnhancedStepHueConfParams_t, RPC_C2S_ZBPRO_ZCL_ColorControlCmdEnhancedStepHueReq)
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_COLOR_CONTROL_CMD_ENHANCED_MOVE_TO_HUE_AND_SATURATION_REQ_FID, ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueAndSaturationReq, ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueAndSaturationReqDescr_t, ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueAndSaturationConfParams_t, RPC_C2S_ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueAndSaturationReq)
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_COLOR_CONTROL_CMD_COLOR_LOOP_SET_REQ_FID, ZBPRO_ZCL_ColorControlCmdColorLoopSetReq, ZBPRO_ZCL_ColorControlCmdColorLoopSetReqDescr_t, ZBPRO_ZCL_ColorControlCmdColorLoopSetConfParams_t, RPC_C2S_ZBPRO_ZCL_ColorControlCmdColorLoopSetReq)
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_COLOR_CONTROL_CMD_STOP_MOVE_STEP_REQ_FID, ZBPRO_ZCL_ColorControlCmdStopMoveStepReq, ZBPRO_ZCL_ColorControlCmdStopMoveStepReqDescr_t, ZBPRO_ZCL_ColorControlCmdStopMoveStepConfParams_t, RPC_C2S_ZBPRO_ZCL_ColorControlCmdStopMoveStepReq)
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_COLOR_CONTROL_CMD_MOVE_COLOR_TEMPERATURE_REQ_FID, ZBPRO_ZCL_ColorControlCmdMoveColorTemperatureReq, ZBPRO_ZCL_ColorControlCmdMoveColorTemperatureReqDescr_t, ZBPRO_ZCL_ColorControlCmdMoveColorTemperatureConfParams_t, RPC_C2S_ZBPRO_ZCL_ColorControlCmdMoveColorTemperatureReq)
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZCL_COLOR_CONTROL_CMD_STEP_COLOR_TEMPERATURE_REQ_FID, ZBPRO_ZCL_ColorControlCmdStepColorTemperatureReq, ZBPRO_ZCL_ColorControlCmdStepColorTemperatureReqDescr_t, ZBPRO_ZCL_ColorControlCmdStepColorTemperatureConfParams_t, RPC_C2S_ZBPRO_ZCL_ColorControlCmdStepColorTemperatureReq)
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZHA_EZ_MODE_FID, ZBPRO_ZHA_EzModeReq, ZBPRO_ZHA_EzModeReqDescr_t, ZBPRO_ZHA_EzModeConfParams_t, RPC_C2S_ZBPRO_ZHA_EzModeReq)
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZHA_CIE_ENROLL_FID, ZBPRO_ZHA_CieDeviceEnrollReq, ZBPRO_ZHA_CieEnrollReqDescr_t, ZBPRO_ZHA_CieEnrollConfParams_t, RPC_C2S_ZBPRO_ZHA_CieDeviceEnrollReq)
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZHA_CIE_SET_PANEL_STATUS_REQ_FID, ZBPRO_ZHA_CieDeviceSetPanelStatusReq, ZBPRO_ZHA_CieSetPanelStatusReqDescr_t, ZBPRO_ZHA_CieSetPanelStatusConfParams_t, RPC_C2S_ZBPRO_ZHA_CieDeviceSetPanelStatusReq)
CREATE_CLIENT_REQUEST_API_FUNCTION(ZBPRO_ZHA_CIE_ZONE_SET_BYPASS_STATE_REQ_FID, ZBPRO_ZHA_CieZoneSetBypassStateReq, ZBPRO_ZHA_CieZoneSetBypassStateReqDescr_t, ZBPRO_ZHA_CieZoneSetBypassStateConfParams_t, RPC_C2S_ZBPRO_ZHA_CieZoneSetBypassStateReq)
CREATE_CLIENT_INDICATION_API_FUNCTION(ZBPRO_ZHA_CIE_SET_PANEL_STATUS_IND_FID, ZBPRO_ZHA_CieDeviceSetPanelStatusInd, ZBPRO_ZHA_CieSetPanelStatusIndParams_t, NoAppropriateType_t, RPC_C2S_ZBPRO_ZHA_CieDeviceSetPanelStatusInd)
/* End of ZCL staff */

#endif

#ifdef SERVER

//CREATE_SERVER_REQUEST_CALLBACK_FUNCTION(RF4CE_ZRC1_REQ_TARGET_BIND_FID, RF4CE_ZRC1_TargetBindReq, RF4CE_ZRC1_BindReqDescr_t, RF4CE_ZRC1_BindConfParams_t, RPC_C2S_RF4CE_ZRC1_TargetBindReq)

CREATE_SERVER_REQUEST_CALLBACK_FUNCTION(RF4CE_ZRC2_SET_PUSH_BUTTON_STIMULUS_FID, RF4CE_ZRC2_SetPushButtonStimulusReq, RF4CE_ZRC2_ButtonBindingReqDescr_t, RF4CE_ZRC2_BindingConfParams_t, RPC_C2S_RF4CE_ZRC2_SetPushButtonStimulusReq)

//CREATE_SERVER_REQUEST_API_FUNCTION(RF4CE_ZRC2_ENABLE_BINDING_FID, RF4CE_ZRC2_EnableBindingReq, RF4CE_ZRC2_BindingReqDescr_t, RF4CE_ZRC2_BindingConfParams_t, RPC_C2S_RF4CE_ZRC2_EnableBindingReq)

CREATE_SERVER_REQUEST_CALLBACK_FUNCTION(RF4CE_ZRC2_ENABLE_BINDING_FID, RF4CE_ZRC2_EnableBindingReq, RF4CE_ZRC2_BindingReqDescr_t, RF4CE_ZRC2_BindingConfParams_t, RPC_C2S_RF4CE_ZRC2_EnableBindingReq)

CREATE_SERVER_REQUEST_FUNCTION(RF4CE_PROFILE_REQ_UNPAIR_FID, RF4CE_UnpairReq, RF4CE_UnpairReqDescr_t, RF4CE_UnpairConfParams_t)

#ifdef BYPASS_RPC
void server_RF4CE_ZRC1_TargetBindReq_callback(RF4CE_ZRC1_BindReqDescr_t  *request, RF4CE_ZRC1_BindConfParams_t *conf)
{
    int size = 0;
    int message_tx[MAX_MSG_SIZE_IN_WORDS];
    RF4CE_ZRC1_BindReqDescr_t request2;                                                                                           \

    printf("ZIGBEE_RPC_API:  invoking RF4CE_ZRC1_TargetBindReq Callback\n");
    list_head *pos = 0;
    int found = 0;
    List_ServerReq_Info_t *info = 0;
    if(RF4CE_ZRC1_BOUND != conf->status)
        registerRemoveRegistryInfoInWaitForPairQueue();

    list_for_each(pos, &serverRequestListHeader){
        if((info = GET_PARENT_BY_FIELD(List_ServerReq_Info_t, list, pos))->serverRequest == (int)request)
        {
            found = 1;
            list_del(pos);
            break;
        }
    }
    if(!found){
        printf("can't found the origin pointer\n");
        return;
    }

    message_tx[0] = info->clientRequest;
    free(info);

    memcpy((char *)&request2, (char *)info->clientRequestData, sizeof(RF4CE_ZRC1_BindReqDescr_t));

    const MailServerParametersTableEntry_t *const confirmInfo = mailServerTableGetAppropriateEntry(RF4CE_ZRC1_REQ_TARGET_BIND_FID);
    if(MAIL_INVALID_PAYLOAD_OFFSET != confirmInfo->confDataPointerOffset){
        printf("hanging\n"); while(1);
    }

    const MailClientParametersTableEntry_t *const reqInfo = mailClientTableGetAppropriateEntry(RF4CE_ZRC1_REQ_TARGET_BIND_FID);
    if(MAIL_INVALID_PAYLOAD_OFFSET != reqInfo->dataPointerOffset){
        printf("hanging\n"); while(1);
    }
    request2.callback(request, conf);
    SYS_SchedulerPostTask(&requestDispatcherTask, 0);
}
#else
void server_RF4CE_ZRC1_TargetBindReq_callback(RF4CE_ZRC1_BindReqDescr_t  *request, RF4CE_ZRC1_BindConfParams_t *conf)
{
    int size = 0;
    int message_tx[MAX_MSG_SIZE_IN_WORDS];

    printf("ZIGBEE_RPC_API:  invoking RF4CE_ZRC1_TargetBindReq Callback\n");
    list_head *pos = 0;
    int found = 0;
    List_ServerReq_Info_t *info = 0;
    if(RF4CE_ZRC1_BOUND != conf->status)
        registerRemoveRegistryInfoInWaitForPairQueue();

    list_for_each(pos, &serverRequestListHeader){
        if((info = GET_PARENT_BY_FIELD(List_ServerReq_Info_t, list, pos))->serverRequest == (int)request)
        {
            found = 1;
            list_del(pos);
            break;
        }
    }
    if(!found){
        printf("can't found the origin pointer\n");
        return;
    }

    message_tx[0] = info->clientRequest;
    free(info);
    size += 1;
    memcpy((char *)&message_tx[size], (char*)info->clientRequestData, sizeof(RF4CE_ZRC1_BindReqDescr_t));
    size += WORDALIGN(sizeof(RF4CE_ZRC1_BindReqDescr_t));
    message_tx[size] = (unsigned int)conf;
    size += 1;
    memcpy((char *)&message_tx[size], (char *)conf, sizeof(RF4CE_ZRC1_BindConfParams_t));
    size += WORDALIGN(sizeof(RF4CE_ZRC1_BindConfParams_t));
    const MailServerParametersTableEntry_t *const confirmInfo = mailServerTableGetAppropriateEntry(RF4CE_ZRC1_REQ_TARGET_BIND_FID);
    if(MAIL_INVALID_PAYLOAD_OFFSET != confirmInfo->confDataPointerOffset){
        SYS_DataPointer_t *const dataPointer = (SYS_DataPointer_t *)
                                            ((uint8_t *)conf + confirmInfo->confDataPointerOffset);
        int size_payload = SYS_GetPayloadSize(dataPointer);
        SYS_CopyFromPayload((void*)&message_tx[size], dataPointer, 0, size_payload);
        size += WORDALIGN(size_payload);
        if(SYS_CheckPayload(dataPointer))
           SYS_FreePayload(dataPointer);
    }

    const MailClientParametersTableEntry_t *const reqInfo = mailClientTableGetAppropriateEntry(RF4CE_ZRC1_REQ_TARGET_BIND_FID);
    if(MAIL_INVALID_PAYLOAD_OFFSET != reqInfo->dataPointerOffset){
        SYS_DataPointer_t *const reqDataPointer = (SYS_DataPointer_t *)(
            (uint8_t *)request + reqInfo->paramOffset + reqInfo->dataPointerOffset);
        if(SYS_CheckPayload(reqDataPointer))
           SYS_FreePayload(reqDataPointer);
    }
    free(request);
    Zigbee_Rpc_Send((unsigned int *)message_tx, size, RPC_C2S_RF4CE_ZRC1_TargetBindReq | RPC_RESPONSE, Zigbee_Socket_ServerSend, info->socket);
    SYS_SchedulerPostTask(&requestDispatcherTask, 0);
}
#endif

void server_RF4CE_UnpairReq_callback(RF4CE_UnpairReqDescr_t  *request, RF4CE_UnpairConfParams_t *conf)
{
    int size = 0;
    int message_tx[MAX_MSG_SIZE_IN_WORDS];

    printf("ZIGBEE_RPC_API:  invoking RF4CE_UnpairReq Callback\n");
    list_head *pos = 0;
    int found = 0;
    List_ServerReq_Info_t *info = 0;
    if(RF4CE_SUCCESS == conf->status)
        registerRemoveRegistryInfoForPairRef(request->params.pairingRef);

    list_for_each(pos, &serverRequestListHeader){
        if((info = GET_PARENT_BY_FIELD(List_ServerReq_Info_t, list, pos))->serverRequest == (int)request)
        {
            found = 1;
            list_del(pos);
            break;
        }
    }
    if(!found){
        printf("can't found the origin pointer\n");
        return;
    }

    message_tx[0] = info->clientRequest;
    free(info);
    size += 1;
    memcpy((char *)&message_tx[size], (char*)info->clientRequestData, sizeof(RF4CE_UnpairReqDescr_t));
    size += WORDALIGN(sizeof(RF4CE_UnpairReqDescr_t));
    message_tx[size] = (unsigned int)conf;
    size += 1;
    memcpy((char *)&message_tx[size], (char *)conf, sizeof(RF4CE_UnpairConfParams_t));
    size += WORDALIGN(sizeof(RF4CE_UnpairConfParams_t));
    const MailServerParametersTableEntry_t *const confirmInfo = mailServerTableGetAppropriateEntry(RF4CE_PROFILE_REQ_UNPAIR_FID);
    if(MAIL_INVALID_PAYLOAD_OFFSET != confirmInfo->confDataPointerOffset){
        SYS_DataPointer_t *const dataPointer = (SYS_DataPointer_t *)
                                            ((uint8_t *)conf + confirmInfo->confDataPointerOffset);
        int size_payload = SYS_GetPayloadSize(dataPointer);
        SYS_CopyFromPayload((void*)&message_tx[size], dataPointer, 0, size_payload);
        size += WORDALIGN(size_payload);
        if(SYS_CheckPayload(dataPointer))
           SYS_FreePayload(dataPointer);
    }

    const MailClientParametersTableEntry_t *const reqInfo = mailClientTableGetAppropriateEntry(RF4CE_PROFILE_REQ_UNPAIR_FID);
    if(MAIL_INVALID_PAYLOAD_OFFSET != reqInfo->dataPointerOffset){
        SYS_DataPointer_t *const reqDataPointer = (SYS_DataPointer_t *)(
            (uint8_t *)request + reqInfo->paramOffset + reqInfo->dataPointerOffset);
        if(SYS_CheckPayload(reqDataPointer))
           SYS_FreePayload(reqDataPointer);
    }
    free(request);
#ifndef BYPASS_RPC
    Zigbee_Rpc_Send((unsigned int *)message_tx, size, RPC_C2S_RF4CE_UnpairReq | RPC_RESPONSE, Zigbee_Socket_ServerSend, info->socket);
#endif
    SYS_SchedulerPostTask(&requestDispatcherTask, 0);

}

#ifdef BYPASS_RPC
void RF4CE_ZRC1_TargetBindReq(RF4CE_ZRC1_BindReqDescr_t *request)
{
    /*
    RF4CE_ZRC1_BindReqDescr_t *request = (RF4CE_ZRC1_BindReqDescr_t *)malloc(sizeof(RF4CE_ZRC1_BindReqDescr_t));
    memcpy((char*)request, &buf[1], sizeof(RF4CE_ZRC1_BindReqDescr_t));
    */
    const MailClientParametersTableEntry_t *const reqInfo = mailClientTableGetAppropriateEntry(RF4CE_ZRC1_REQ_TARGET_BIND_FID);
    if(MAIL_INVALID_PAYLOAD_OFFSET != reqInfo->dataPointerOffset)
    {
        printf("hanging\n"); while(1);
    }
    List_ServerReq_Info_t *info = malloc(sizeof(List_ServerReq_Info_t) + sizeof(RF4CE_ZRC1_BindReqDescr_t));
    INIT_SERVER_LIST_INFO(info, 0, (unsigned int)request, RF4CE_ZRC1_REQ_TARGET_BIND_FID, 7)
    memcpy((char*)info->clientRequestData, (char*)request, sizeof(RF4CE_ZRC1_BindReqDescr_t));
    request->callback = (void(*)(RF4CE_ZRC1_BindReqDescr_t*, RF4CE_ZRC1_BindConfParams_t*))server_RF4CE_ZRC1_TargetBindReq_callback;
    if(availablePendingTableEntry()){
        list_add(&info->list, &serverRequestListHeader);
        /* register this client for binding */
        if(!InsertClientInfoToWaitingPairQueue(7))
        {
            RF4CE_ZRC1_BindConfParams_t conf = {0};
            conf.status = 1;
            request->callback(request, &conf);
            return;
        }
        RF4CE_ZRC1_TargetBindReq_Call(request);
    }else
        list_add(&info->list, &serverPendingRequestListHeader);
}
#else
void server_RF4CE_ZRC1_TargetBindReq(unsigned int *buf, int socket)
{
    RF4CE_ZRC1_BindReqDescr_t *request = (RF4CE_ZRC1_BindReqDescr_t *)malloc(sizeof(RF4CE_ZRC1_BindReqDescr_t));
    memcpy((char*)request, &buf[1], sizeof(RF4CE_ZRC1_BindReqDescr_t));
    const MailClientParametersTableEntry_t *const reqInfo = mailClientTableGetAppropriateEntry(RF4CE_ZRC1_REQ_TARGET_BIND_FID);
    if(MAIL_INVALID_PAYLOAD_OFFSET != reqInfo->dataPointerOffset)
    {
        SYS_DataPointer_t *const dataPointer = (SYS_DataPointer_t *)(
            (uint8_t *)request + reqInfo->paramOffset + reqInfo->dataPointerOffset);
        int size_payload = SYS_GetPayloadSize(dataPointer);
        SYS_SetEmptyPayload(dataPointer);
        SYS_MemAlloc(dataPointer, size_payload);
        SYS_CopyToPayload(dataPointer, 0, (const void*)&buf[1 + WORDALIGN(sizeof(RF4CE_ZRC1_BindReqDescr_t))], size_payload);
    }
    List_ServerReq_Info_t *info = malloc(sizeof(List_ServerReq_Info_t) + sizeof(RF4CE_ZRC1_BindReqDescr_t));
    INIT_SERVER_LIST_INFO(info, buf[0], (unsigned int)request, RF4CE_ZRC1_REQ_TARGET_BIND_FID, socket)
    memcpy((char*)info->clientRequestData, (char*)request, sizeof(RF4CE_ZRC1_BindReqDescr_t));
    request->callback = (void(*)(RF4CE_ZRC1_BindReqDescr_t*, RF4CE_ZRC1_BindConfParams_t*))server_RF4CE_ZRC1_TargetBindReq_callback;
    if(availablePendingTableEntry()){
        list_add(&info->list, &serverRequestListHeader);
        /* register this client for binding */
        if(!InsertClientInfoToWaitingPairQueue(socket))
        {
            RF4CE_ZRC1_BindConfParams_t conf = {0};
            conf.status = 1;
            request->callback(request, &conf);
            return;
        }
        RF4CE_ZRC1_TargetBindReq_Call(request);
    }else
        list_add(&info->list, &serverPendingRequestListHeader);
}
#endif

void server_RF4CE_ZRC2_SetPushButtonStimulusReq(unsigned int *buf, int socket)
{
    RF4CE_ZRC2_ButtonBindingReqDescr_t *request = (RF4CE_ZRC2_ButtonBindingReqDescr_t *)malloc(sizeof(RF4CE_ZRC2_ButtonBindingReqDescr_t));
    memcpy((char*)request, &buf[1], sizeof(RF4CE_ZRC2_ButtonBindingReqDescr_t));
    const MailClientParametersTableEntry_t *const reqInfo = mailClientTableGetAppropriateEntry(RF4CE_ZRC2_SET_PUSH_BUTTON_STIMULUS_FID);
    if(MAIL_INVALID_PAYLOAD_OFFSET != reqInfo->dataPointerOffset)
    {
        SYS_DataPointer_t *const dataPointer = (SYS_DataPointer_t *)(
            (uint8_t *)request + reqInfo->paramOffset + reqInfo->dataPointerOffset);
        int size_payload = SYS_GetPayloadSize(dataPointer);
        SYS_SetEmptyPayload(dataPointer);
        SYS_MemAlloc(dataPointer, size_payload);
        SYS_CopyToPayload(dataPointer, 0, (const void*)&buf[1 + WORDALIGN(sizeof(RF4CE_ZRC2_ButtonBindingReqDescr_t))], size_payload);
    }
    List_ServerReq_Info_t *info = malloc(sizeof(List_ServerReq_Info_t) + sizeof(RF4CE_ZRC2_ButtonBindingReqDescr_t));
    INIT_SERVER_LIST_INFO(info, buf[0], (unsigned int)request, RF4CE_ZRC2_SET_PUSH_BUTTON_STIMULUS_FID, socket)
    memcpy((char*)info->clientRequestData, (char*)request, sizeof(RF4CE_ZRC2_ButtonBindingReqDescr_t));
    request->callback = (void(*)(RF4CE_ZRC2_ButtonBindingReqDescr_t*, RF4CE_ZRC2_BindingConfParams_t*))server_RF4CE_ZRC2_SetPushButtonStimulusReq_callback;
    if(availablePendingTableEntry()){
        list_add(&info->list, &serverRequestListHeader);
        /* register this client for binding */
        if(!InsertClientInfoToWaitingPairQueue(socket))
        {
            RF4CE_ZRC2_BindingConfParams_t conf = {0};
            conf.status = 1;
            request->callback(request, &conf);
            return;
        }
        RF4CE_ZRC2_SetPushButtonStimulusReq_Call(request);
    }else
        list_add(&info->list, &serverPendingRequestListHeader);
}

#if 1
void server_RF4CE_ZRC2_EnableBindingReq(unsigned int *buf, int socket)
{
    RF4CE_ZRC2_BindingReqDescr_t *request = (RF4CE_ZRC2_BindingReqDescr_t *)malloc(sizeof(RF4CE_ZRC2_BindingReqDescr_t));
    memcpy((char*)request, &buf[1], sizeof(RF4CE_ZRC2_BindingReqDescr_t));
    const MailClientParametersTableEntry_t *const reqInfo = mailClientTableGetAppropriateEntry(RF4CE_ZRC2_ENABLE_BINDING_FID);
    if(MAIL_INVALID_PAYLOAD_OFFSET != reqInfo->dataPointerOffset)
    {
        SYS_DataPointer_t *const dataPointer = (SYS_DataPointer_t *)(
            (uint8_t *)request + reqInfo->paramOffset + reqInfo->dataPointerOffset);
        int size_payload = SYS_GetPayloadSize(dataPointer);
        SYS_SetEmptyPayload(dataPointer);
        SYS_MemAlloc(dataPointer, size_payload);
        SYS_CopyToPayload(dataPointer, 0, (const void*)&buf[1 + WORDALIGN(sizeof(RF4CE_ZRC2_BindingReqDescr_t))], size_payload);
    }
    List_ServerReq_Info_t *info = malloc(sizeof(List_ServerReq_Info_t) + sizeof(RF4CE_ZRC2_BindingReqDescr_t));
    INIT_SERVER_LIST_INFO(info, buf[0], (unsigned int)request, RF4CE_ZRC2_ENABLE_BINDING_FID, socket)
    memcpy((char*)info->clientRequestData, (char*)request, sizeof(RF4CE_ZRC2_BindingReqDescr_t));
    request->callback = (void(*)(RF4CE_ZRC2_BindingReqDescr_t*, RF4CE_ZRC2_BindingConfParams_t*))server_RF4CE_ZRC2_EnableBindingReq_callback;
    if(availablePendingTableEntry()){
        list_add(&info->list, &serverRequestListHeader);
        /* register this client for binding */
        if(!InsertClientInfoToWaitingPairQueue(socket))
        {
            RF4CE_ZRC2_BindingConfParams_t conf = {0};
            conf.status = 0;
            request->callback(request, &conf);
            return;
        }
        RF4CE_ZRC2_EnableBindingReq_Call(request);
    }else
        list_add(&info->list, &serverPendingRequestListHeader);
}
#endif

void server_RF4CE_ZRC2_CheckValidationResp(unsigned int *buf, int socket)
{
    RF4CE_ZRC2_CheckValidationRespDescr_t request = {0};
    memcpy((char*)&request, &buf[1], sizeof(RF4CE_ZRC2_CheckValidationRespDescr_t));
    RF4CE_ZRC2_CheckValidationResp_Call(&request);
}


/*
 * Since the registration function doesn't go through the HW mailbox, we don't plan to use the template above to create this
 * specific API function.
 */
 /* this call is a synchronous function call and will directly call the callback function */
 /* Even we use the callback function to return function result                           */
void server_RF4CE_RegisterVirtualDevice(unsigned int *buf, int socket)
{
    RF4CE_RegisterVirtualDeviceReqDescr_t request;
    RF4CE_RegisterVirtualDeviceReqDescr_t *clientRequest = (RF4CE_RegisterVirtualDeviceReqDescr_t *)buf[0];
    RF4CE_RegisterVirtualDeviceReqDescr_t clientRequestData;
    memcpy((char*)&clientRequestData, &buf[1], sizeof(RF4CE_RegisterVirtualDeviceReqDescr_t));
    memcpy((char*)&request, (char*)&clientRequestData, sizeof(RF4CE_RegisterVirtualDeviceReqDescr_t));
    void server_RF4CE_RegisterVirtualDevice_callback(RF4CE_RegisterVirtualDeviceReqDescr_t *req, RF4CE_RegisterVirtualDeviceConfParams_t *conf)
    {
            int size = 0;
            int message_tx[MAX_MSG_SIZE_IN_WORDS];

            printf("ZIGBEE_RPC_API:  invoking RF4CE_RegisterVirtualDevice Callback\n");

            message_tx[0] = (int)clientRequest;
            size += 1;
            memcpy((char *)&message_tx[size], (char *)&clientRequestData, sizeof(RF4CE_RegisterVirtualDeviceReqDescr_t));
            size += WORDALIGN(sizeof(RF4CE_RegisterVirtualDeviceReqDescr_t));
            message_tx[size] = (unsigned int)conf;
            size += 1;
            memcpy((char *)&message_tx[size], (char *)conf, sizeof(RF4CE_RegisterVirtualDeviceConfParams_t));
            size += WORDALIGN(sizeof(RF4CE_RegisterVirtualDeviceConfParams_t));

#ifndef BYPASS_RPC
            if(NULL != clientRequestData.callback)
                Zigbee_Rpc_Send((unsigned int *)message_tx, size, RPC_C2S_RF4CE_RegisterVirtualDevice | RPC_RESPONSE, Zigbee_Socket_ServerSend, socket);
#endif
    }
    request.callback = (RF4CE_RegisterVirtualDeviceConfCallback_t*)server_RF4CE_RegisterVirtualDevice_callback;
    RF4CE_RegisterVirtualDevice(&request, socket);
}

void server_ZBPRO_MAC_GetReq(unsigned int *buf, int socket)
{
    MAC_GetReqDescr_t request;
    MAC_GetReqDescr_t *clientRequest = (MAC_GetReqDescr_t *)buf[0];
    MAC_GetReqDescr_t clientRequestData;
    memcpy((char*)&clientRequestData, &buf[1], sizeof(MAC_GetReqDescr_t));
    memcpy((char*)&request, (char*)&clientRequestData, sizeof(MAC_GetReqDescr_t));
    void server_ZBPRO_MAC_Get_callback(MAC_GetReqDescr_t *req, MAC_GetConfParams_t *conf)
    {
            int size = 0;
            int message_tx[MAX_MSG_SIZE_IN_WORDS];

            message_tx[0] = (int)clientRequest;
            size += 1;
            memcpy((char *)&message_tx[size], (char *)&clientRequestData, sizeof(MAC_GetReqDescr_t));
            size += WORDALIGN(sizeof(MAC_GetReqDescr_t));
            message_tx[size] = (unsigned int)conf;
            size += 1;
            memcpy((char *)&message_tx[size], (char *)conf, sizeof(MAC_GetConfParams_t));
            size += WORDALIGN(sizeof(MAC_GetConfParams_t));

#ifndef BYPASS_RPC
            if(NULL != clientRequestData.callback)
                Zigbee_Rpc_Send((unsigned int *)message_tx, size, RPC_C2S_RF4CE_MAC_GetReq | RPC_RESPONSE, Zigbee_Socket_ServerSend, socket);
#endif
    }
    request.callback = server_ZBPRO_MAC_Get_callback;
    MAC_GetConfParams_t  retConf;
    memset(&retConf, 0, sizeof(MAC_GetConfParams_t));
    MAC_ExtendedAddress_t ieeeAddress;
    extern int g_zigbeeFd;
    Zigbee_Ioctl_GetZbproMacAddr(g_zigbeeFd, (char*)&ieeeAddress);
    retConf.attributeValue.macExtendedAddress = ieeeAddress;
    request.callback(clientRequest, &retConf);
}

void server_ZBPRO_APS_EndpointRegisterReq(unsigned int *buf, int socket)
{
    ZBPRO_APS_EndpointRegisterReqDescr_t *request = (ZBPRO_APS_EndpointRegisterReqDescr_t *)malloc(sizeof(ZBPRO_APS_EndpointRegisterReqDescr_t));
    memcpy((char*)request, &buf[1], sizeof(ZBPRO_APS_EndpointRegisterReqDescr_t));
    const MailClientParametersTableEntry_t *const reqInfo = mailClientTableGetAppropriateEntry(ZBPRO_APS_REQ_ENDPOINTREGISTER_FID);
    if(MAIL_INVALID_PAYLOAD_OFFSET != reqInfo->dataPointerOffset)
    {
        SYS_DataPointer_t *const dataPointer = (SYS_DataPointer_t *)(
            (uint8_t *)request + reqInfo->paramOffset + reqInfo->dataPointerOffset);
        int size_payload = SYS_GetPayloadSize(dataPointer);
        SYS_SetEmptyPayload(dataPointer);
        SYS_MemAlloc(dataPointer, size_payload);
        SYS_CopyToPayload(dataPointer, 0, (const void*)&buf[1 + WORDALIGN(sizeof(ZBPRO_APS_EndpointRegisterReqDescr_t))], size_payload);
    }
    List_ServerReq_Info_t *info = malloc(sizeof(List_ServerReq_Info_t) + sizeof(ZBPRO_APS_EndpointRegisterReqDescr_t));
    INIT_SERVER_LIST_INFO(info, buf[0], (unsigned int)request, ZBPRO_APS_REQ_ENDPOINTREGISTER_FID, socket)
    memcpy((char*)info->clientRequestData, (char*)request, sizeof(ZBPRO_APS_EndpointRegisterReqDescr_t));
    request->callback = (void(*)(ZBPRO_APS_EndpointRegisterReqDescr_t*, ZBPRO_APS_EndpointRegisterConfParams_t*))server_ZBPRO_APS_EndpointRegisterReq_callback;
    // Add this socket id to registration infomation
    HA_Register_Itself(socket, request->params.simpleDescriptor.endpoint);
    if(availablePendingTableEntry()){
        list_add(&info->list, &serverRequestListHeader);
        ZBPRO_APS_EndpointRegisterReq_Call(request);
    }else
        list_add(&info->list, &serverPendingRequestListHeader);
}

void server_ZBPRO_APS_EndpointUnregisterReq(unsigned int *buf, int socket)
{
    ZBPRO_APS_EndpointUnregisterReqDescr_t *request = (ZBPRO_APS_EndpointUnregisterReqDescr_t *)malloc(sizeof(ZBPRO_APS_EndpointUnregisterReqDescr_t));
    memcpy((char*)request, &buf[1], sizeof(ZBPRO_APS_EndpointUnregisterReqDescr_t));
    const MailClientParametersTableEntry_t *const reqInfo = mailClientTableGetAppropriateEntry(ZBPRO_APS_REQ_ENDPOINTREGISTER_FID);
    if(MAIL_INVALID_PAYLOAD_OFFSET != reqInfo->dataPointerOffset)
    {
        SYS_DataPointer_t *const dataPointer = (SYS_DataPointer_t *)(
            (uint8_t *)request + reqInfo->paramOffset + reqInfo->dataPointerOffset);
        int size_payload = SYS_GetPayloadSize(dataPointer);
        SYS_SetEmptyPayload(dataPointer);
        SYS_MemAlloc(dataPointer, size_payload);
        SYS_CopyToPayload(dataPointer, 0, (const void*)&buf[1 + WORDALIGN(sizeof(ZBPRO_APS_EndpointUnregisterReqDescr_t))], size_payload);
    }
    List_ServerReq_Info_t *info = malloc(sizeof(List_ServerReq_Info_t) + sizeof(ZBPRO_APS_EndpointUnregisterReqDescr_t));
    INIT_SERVER_LIST_INFO(info, buf[0], (unsigned int)request, ZBPRO_APS_REQ_ENDPOINTREGISTER_FID, socket)
    memcpy((char*)info->clientRequestData, (char*)request, sizeof(ZBPRO_APS_EndpointUnregisterReqDescr_t));
    request->callback = (void(*)(ZBPRO_APS_EndpointUnregisterReqDescr_t*, ZBPRO_APS_EndpointRegisterConfParams_t*))server_ZBPRO_APS_EndpointUnregisterReq_callback;
    // Add this socket id to registration infomation
    HA_Unregister_Itself(socket, request->params.endpoint);
    if(availablePendingTableEntry()){
        list_add(&info->list, &serverRequestListHeader);
        ZBPRO_APS_EndpointUnregisterReq_Call(request);
    }else
        list_add(&info->list, &serverPendingRequestListHeader);
}

#else

void client_RF4CE_RegisterVirtualDevice_callback(unsigned int *message_rx)
{
    RF4CE_RegisterVirtualDeviceReqDescr_t req;
    RF4CE_RegisterVirtualDeviceConfParams_t *conf = malloc(sizeof(RF4CE_RegisterVirtualDeviceConfParams_t));
    memcpy((char *)&req, (char *)&message_rx[1], sizeof(req));
    memcpy((char *)conf, (char *)&message_rx[2 + WORDALIGN(sizeof(RF4CE_RegisterVirtualDeviceReqDescr_t))], sizeof(conf));
    if (req.callback) {
        req.callback((RF4CE_RegisterVirtualDeviceReqDescr_t *)message_rx[0], conf);
    }
}

#ifndef BYPASS_RPC
void RF4CE_RegisterVirtualDevice(RF4CE_RegisterVirtualDeviceReqDescr_t *request)
{
    unsigned int buf[RPC_MAX_PAYLOAD_SIZE_IN_WORDS];
    int size_in_words = 0;
    memset(&buf[0], 0, sizeof(buf));
    buf[0] = (unsigned int)request;
    size_in_words += 1;
    memcpy((char *)&buf[1], request, sizeof(RF4CE_RegisterVirtualDeviceReqDescr_t));
    size_in_words += WORDALIGN(sizeof(RF4CE_RegisterVirtualDeviceReqDescr_t));
    Zigbee_Rpc_Send(buf, size_in_words, RPC_C2S_RF4CE_RegisterVirtualDevice, Zigbee_Socket_ClientSend, 0);
}
#endif

#endif

#if 0
#ifdef SERVER
    void server_RF4CE_StartReq_callback(RF4CE_StartReqDescr_t *request, RF4CE_StartResetConfParams_t *conf, int socket)
    {
        int size;
        int message_tx[MAX_MSG_SIZE_IN_WORDS];

        printf("ZIGBEE_RPC_API:  invoking server_RF4CE_StartReq_callback\n");

        message_tx[PAYLOAD_OFFSET_S2C_RF4CE_StartReq_callback_REQ] = zigbee_api_rf4ce_cb[socket].RF4CE_StartReq.client_request;
        memcpy((char *)&message_tx[PAYLOAD_OFFSET_S2C_RF4CE_StartReq_callback_REQ_data], (char *)&zigbee_api_rf4ce_cb[socket].RF4CE_StartReq.client_request_data, sizeof(RF4CE_StartReqDescr_t));
        message_tx[PAYLOAD_OFFSET_S2C_RF4CE_StartReq_callback_CONF] = (unsigned int)conf;
        memcpy((char *)&message_tx[PAYLOAD_OFFSET_S2C_RF4CE_StartReq_callback_CONF_data], (char *)conf, sizeof(conf));
        size = PAYLOAD_OFFSET_S2C_RF4CE_StartReq_callback_CONF_data + (sizeof(conf)+3)/4;
        Zigbee_Rpc_Send((unsigned int *)message_tx, size, RPC_C2S_RF4CE_StartReq | RPC_RESPONSE, Zigbee_Socket_ServerSend, socket);
        #ifdef SIM
            socket_cb[socket].state = SOCKET_NEED_TO_SEND_RPC_S2C_RF4CE_ZRC_PairInd;
        #endif
    }
#else
    void client_RF4CE_StartReq_callback(unsigned int *message_rx)
    {
        RF4CE_StartReqDescr_t req;
        RF4CE_StartResetConfParams_t conf;
        memcpy((char *)&req, (char *)&message_rx[PAYLOAD_OFFSET_S2C_RF4CE_StartReq_callback_REQ_data], sizeof(req));
        memcpy((char *)&conf, (char *)&message_rx[PAYLOAD_OFFSET_S2C_RF4CE_StartReq_callback_CONF_data], sizeof(conf));
        if (req.callback) {
            req.callback((RF4CE_StartReqDescr_t *)message_rx[PAYLOAD_OFFSET_S2C_RF4CE_StartReq_callback_REQ], &conf);
        };
    }
#endif

#ifdef SERVER
    void server_RF4CE_StartReq(unsigned int *buf, int socket)
    {
        RF4CE_StartReqDescr_t request;
        zigbee_api_rf4ce_cb[socket].RF4CE_StartReq.server_request = (unsigned int)&request;
        zigbee_api_rf4ce_cb[socket].RF4CE_StartReq.client_request = buf[PAYLOAD_OFFSET_C2S_RF4CE_StartReq_REQ];
        memcpy((char *)&zigbee_api_rf4ce_cb[socket].RF4CE_StartReq.client_request_data, &buf[PAYLOAD_OFFSET_C2S_RF4CE_StartReq_REQ_data], sizeof(request));
        request.callback = server_RF4CE_StartReq_callback;
        RF4CE_StartReq_Call(&request, socket);
    }
#else
    void RF4CE_StartReq(RF4CE_StartReqDescr_t *request)
    {
        unsigned int buf[RPC_FRAME_LENGTH];
        int size_in_words;

        /* Compose the Request message */
        memset(&buf[0], 0, sizeof(buf));
        buf[PAYLOAD_OFFSET_C2S_RF4CE_StartReq_REQ]=(unsigned int)request; /* 1st parameter */
        memcpy((char *)&buf[PAYLOAD_OFFSET_C2S_RF4CE_StartReq_REQ_data], request, sizeof(RF4CE_StartReqDescr_t));
        size_in_words = PAYLOAD_OFFSET_C2S_RF4CE_StartReq_REQ_data + (sizeof(RF4CE_StartReqDescr_t)+3)/4;
        Zigbee_Rpc_Send(buf, size_in_words, RPC_C2S_RF4CE_StartReq, Zigbee_Socket_ClientSend, 0);   /* Upon return of this function, the data is copied to the driver so the local buffer can be freed */
    }
#endif

#endif

#if 0
void RF4CE_ResetReq(RF4CE_ResetReqDescr_t *request) {}
#endif

#if 0 /* delete RF4CE_ZRC_PairInd by if 0 */
/* 4.2.7 */
#define PAYLOAD_OFFSET_S2C_RF4CE_ZRC_PairInd_INDICATION_data                0

#ifdef SERVER
    void RF4CE_ZRC_PairInd(RF4CE_PairingReferenceIndParams_t *indication, int socket)
    {
        int size;
        int message_tx[MAX_MSG_SIZE_IN_WORDS];
        socket_cb[socket].message_id = RPC_S2C_RF4CE_ZRC_PairInd;
        memcpy((char *)&message_tx[PAYLOAD_OFFSET_S2C_RF4CE_ZRC_PairInd_INDICATION_data], (char *)indication, sizeof(*indication));
        size = (sizeof(*indication) + 3) / 4;
        Zigbee_Rpc_Send((unsigned int *)message_tx, size, RPC_S2C_RF4CE_ZRC_PairInd, Zigbee_Socket_ServerSend, socket);
        #ifdef SIM
            socket_cb[socket].state = SOCKET_NEED_TO_SEND_RPC_S2C_RF4CE_ZRC_CheckValidationInd;
        #endif
    }
#else
    void client_RF4CE_ZRC_PairInd(zigbeeCallback *pZigbeeCallback, unsigned int *message_rx)
    {
        RF4CE_PairingReferenceIndParams_t indication;
        memcpy((char *)&indication, (char *)&message_rx[PAYLOAD_OFFSET_S2C_RF4CE_ZRC_PairInd_INDICATION_data], sizeof(indication));
        if (pZigbeeCallback->RF4CE_ZRC_PairInd != NULL) {
            pZigbeeCallback->RF4CE_ZRC_PairInd(&indication);
        }
    }
#endif

#endif  /* delete RF4CE_ZRC_PairInd by if 0 */

#if 0
void RF4CE_ZRC_HeartbeatInd( RF4CE_PairingReferenceIndParams_t *indication) {}
#endif

#if  0 /* deleted by Huajun.Xiong */
#ifdef SERVER
void server_RF4CE_ZRC_CheckValidationInd_callback(unsigned int *buf, int socket)
{
    RF4CE_GDP_CheckValidationIndDescr_t *req;
    RF4CE_GDP_CheckValidationRespConfParams_t conf;
    void (*callback)(RF4CE_GDP_CheckValidationIndDescr_t *req, RF4CE_GDP_CheckValidationRespConfParams_t *conf);

    memcpy((char *)&req, (char *)&buf[3], sizeof(req));
    memcpy((char *)&conf, (char *)&buf[4], sizeof(RF4CE_GDP_CheckValidationRespConfParams_t));

    /* invoke callback */
    printf("ZIGBEE_RPC_API:  prior to invoking RF4CE_ZRC_CheckValidationInd callback\n");
    callback = (void (*)(RF4CE_GDP_CheckValidationIndDescr_t *, RF4CE_GDP_CheckValidationRespConfParams_t *))buf[2];
    callback(req, &conf);
#ifdef SIM
    socket_cb[socket].state = SOCKET_NEED_TO_SEND_RPC_S2C_RF4CE_ZRC_ControlCommandInd;
#endif
}
#else
unsigned int RF4CE_ZRC_CheckValidationInd_remote_callback;
void client_RF4CE_ZRC_CheckValidationInd_callback(RF4CE_GDP_CheckValidationIndDescr_t *req, RF4CE_GDP_CheckValidationRespConfParams_t *conf)
{
    unsigned int buffer[RPC_MAX_PAYLOAD_SIZE_IN_WORDS];
    memset(&buffer[0], 0, sizeof(buffer));
    buffer[2] = RF4CE_ZRC_CheckValidationInd_remote_callback;
    buffer[3] = (unsigned int)req;
    memcpy((char *)&buffer[4], (char *)conf, sizeof(RF4CE_GDP_CheckValidationRespConfParams_t));
    Zigbee_Rpc_Send(buffer, 120/4, (RPC_S2C_RF4CE_ZRC_CheckValidationInd | RPC_RESPONSE), Zigbee_Socket_ClientSend, 0);   /* Upon return of this function, the data is copied to the driver so the local buffer can be freed */
}
#endif

#define PAYLOAD_OFFSET_S2C_RF4CE_ZRC_CheckValidationInd_INDICATION_data     0

#ifdef SERVER
    void server_RF4CE_ZRC_CheckValidationInd(RF4CE_GDP_CheckValidationIndDescr_t *indication, int socket)
    {
        int size;
        int message_tx[MAX_MSG_SIZE_IN_WORDS];
        memcpy((char *)&message_tx[PAYLOAD_OFFSET_S2C_RF4CE_ZRC_CheckValidationInd_INDICATION_data], (char *)indication, sizeof(RF4CE_GDP_CheckValidationIndDescr_t));
        size = (sizeof(RF4CE_GDP_CheckValidationIndDescr_t) + 3) / 4;
        socket_cb[socket].message_id = RPC_S2C_RF4CE_ZRC_CheckValidationInd;
        Zigbee_Rpc_Send((unsigned int *)message_tx, size, RPC_S2C_RF4CE_ZRC_CheckValidationInd, Zigbee_Socket_ServerSend, socket);
    }
#else
    void client_RF4CE_ZRC_CheckValidationInd(zigbeeCallback *pZigbeeCallback, unsigned int *message_rx)
    {
        RF4CE_GDP_CheckValidationIndDescr_t indication;
        memcpy((char *)&indication, (char *)&message_rx[PAYLOAD_OFFSET_S2C_RF4CE_ZRC_CheckValidationInd_INDICATION_data], sizeof(indication));

        /* save remote callback */
        RF4CE_ZRC_CheckValidationInd_remote_callback = (unsigned int)indication.callback;

        /* overwrite with local callback */
        indication.callback = client_RF4CE_ZRC_CheckValidationInd_callback;

        if (pZigbeeCallback->RF4CE_ZRC_CheckValidationInd != NULL) {
            pZigbeeCallback->RF4CE_ZRC_CheckValidationInd(&indication);
        }
    }
#endif

#endif  /* delete by Huajun.Xiong */

#if 0
void RF4CE_ZRC_EnableBindingReq(RF4CE_GDP_BindingReqDescr_t *request) {}
void RF4CE_ZRC_DisableBindingReq(RF4CE_GDP_BindingReqDescr_t *request) {}
void RF4CE_ZRC_ButtonBindingReq( RF4CE_GDP_ButtonBindingReqDescr_t *request) {}
void RF4CE_ZRC_ClientNotificationReq( RF4CE_GDP_ClientNotificationReqDescr_t *request) {}
#endif

/* 4.2.8 */
#define PAYLOAD_OFFSET_S2C_RF4CE_ZRC_ControlCommandInd_INDICATION 0
#define PAYLOAD_OFFSET_S2C_RF4CE_ZRC_ControlCommandInd_INDICATION_data 1

#if 0
#ifdef SERVER
    void server_RF4CE_ZRC_ControlCommandInd(RF4CE_ZRC_ControlCommandIndParams_t *indication, int socket)
    {
        int size;
        int message_tx[MAX_MSG_SIZE_IN_WORDS];
        message_tx[PAYLOAD_OFFSET_S2C_RF4CE_ZRC_ControlCommandInd_INDICATION] = (unsigned int)indication;
        memcpy((char *)&message_tx[PAYLOAD_OFFSET_S2C_RF4CE_ZRC_ControlCommandInd_INDICATION_data], (char *)indication, sizeof(RF4CE_ZRC_ControlCommandIndParams_t));
        size = PAYLOAD_OFFSET_S2C_RF4CE_ZRC_ControlCommandInd_INDICATION_data;
        size += (sizeof(RF4CE_ZRC_ControlCommandIndParams_t) + 3) / 4;
        socket_cb[socket].message_id = RPC_S2C_RF4CE_ZRC_ControlCommandInd;
        Zigbee_Rpc_Send(message_tx, size, RPC_S2C_RF4CE_ZRC_ControlCommandInd, Zigbee_Socket_ServerSend, socket);
    }
#else
    void client_RF4CE_ZRC_ControlCommandInd(zigbeeCallback *pZigbeeCallback, unsigned int *message_rx)
    {
        RF4CE_ZRC_ControlCommandIndParams_t indication;
        memcpy((char *)&indication, (char *)&message_rx[PAYLOAD_OFFSET_S2C_RF4CE_ZRC_ControlCommandInd_INDICATION_data], sizeof(indication));
        if (pZigbeeCallback->RF4CE_ZRC_ControlCommandInd != NULL) {
            pZigbeeCallback->RF4CE_ZRC_ControlCommandInd(&indication);
        }
    }
#endif
#endif

#if 0
/* 5.2.1 */
void RF4CE_MSO_GetProfileAttributeReq( RF4CE_MSO_GetProfileAttributeReqDescr_t *request) {}
void RF4CE_MSO_SetProfileAttributeReq( RF4CE_MSO_SetProfileAttributeReqDescr_t *request) {}

/* 5.2.2 */
void RF4CE_MSO_GetRIBInd(RF4CE_MSO_GetRIBAttributeReqDescr_t *request) {}
void RF4CE_MSO_SetRIBInd(RF4CE_MSO_SetRIBAttributeReqDescr_t *request) {}
void RF4CE_MSO_GetRIBAttributeReq( RF4CE_MSO_GetRIBAttributeReqDescr_t *request) {}
void RF4CE_MSO_SetRIBAttributeReq( RF4CE_MSO_SetRIBAttributeReqDescr_t *request) {}

/* 5.2.3 */
void RF4CE_MSO_CheckValidationInd( RF4CE_MSO_CheckValidationRespDescr_t *indication) {}

/* 5.2.4 */
void RF4CE_MSO_UserControlInd( RF4CE_MSO_UserControlIndParams_t *indication) {}

/* HA */

/* 6.4 */
void HA_RegisterEndpointReq(HA_RegisterEndpointReq_t *req) {}
void HA_UnregisterEndpointReq(HA_UnregisterEndpointReq_t *req) {}
void HA_ChangeSubscriptionForAttributeEventsReq( HA_ChangeSubscriptionForAttributeEventsReq_t *req) {}

/* 6.5 */
void HA_ReadAttributeReq(HA_ReadAttributeReq_t *req) {}
void HA_ReadAttributesStructuredReq(HA_ReadAttributesStructuredReq_t *req) {}
void HA_ReadAttributeRespInd(HA_ReadAttributeResp_t *resp) {}
void HA_ReadAttributeNtfyInd(HA_AttributeAccessNtfy_t *ntfy) {}
void HA_WriteAttributeReq(HA_WriteAttributeReq_t *req) {}
void HA_WriteAttributeUndividedReq(HA_WriteAttributeReq_t *req) {}
void HA_WriteAttributeNoResponseReq(HA_WriteAttributeReq_t *req) {}
void HA_WriteAttributesStructuredReq(HA_WriteAttributesStructuredReq_t *req) {}
void HA_WriteAttributeNtfyInd(HA_AttributeAccessNtfy_t *ind) {}
void HA_WriteAttributeRespInd(HA_WriteAttributeResp_t *resp) {}
void HA_WriteAttributesStructuredRespInd(HA_WriteAttributesStructuredResp_t *resp) {}
void HA_ConfigureReportingReq(HA_ConfigureReportingReq_t *req) {}
void HA_ConfigureReportingRespInd(HA_ConfigureReportingResp_t *resp) {}
void HA_ReadReportingConfigurationReq(HA_ReadReportingConfigurationReq_t *ind) {}
void HA_ReadReportingConfigurationRespInd(HA_ReadReportingConfigurationResp_t *resp) {}
void HA_ReportAttributeInd(HA_ReportAttributeInd_t *ind) {}
void HA_DefaultRespInd(HA_DefaultResp_t *resp) {}
void HA_DiscoverAttributesReq(HA_DiscoverAttributesReq_t *req) {}
void HA_DiscoverAttributesRespInd(HA_DiscoverAttributesResp_t *resp) {}

/* 6.7 */
void HA_OnOffClusterOnReq(HA_OnReq_t *req);
void HA_OnOffClusterOffReq(HA_OffReq_t *req);
void HA_OnOffClusterToggleReq(HA_ToggleReq_t *req);

/* 6.8 */
void HA_ScenesClusterAddSceneReq(HA_AddSceneReq_t *req);
void HA_ScenesClusterViewSceneReq(HA_ViewSceneReq_t *req);
void HA_ScenesClusterRemoveSceneReq(HA_RemoveSceneReq_t *req);
void HA_ScenesClusterRemoveAllScenesReq(HA_RemoveAllScenesReq_t *req);
void HA_ScenesClusterStoreSceneReq(HA_StoreSceneReq_t *req);
void HA_ScenesClusterRecallSceneReq(HA_RecallSceneReq_t *req);
void HA_ScenesClusterGetSceneMembershipReq(HA_GetSceneMembershipReq_t *req);
void HA_ScenesClusterAddSceneResponseInd(HA_AddSceneResponseInd_t *ind);
void HA_ScenesClusterViewSceneResponseInd( HA_ViewSceneResponseInd_t *ind);
void HA_ScenesClusterRemoveSceneResponseInd( HA_RemoveSceneResponseInd_t *ind);
void HA_ScenesClusterRemoveAllScenesResponseInd( HA_RemoveAllScenesResponseInd_t *ind);
void HA_ScenesClusterStoreSceneResponseInd( HA_StoreSceneResponseInd_t *ind);
void HA_ScenesClusterGetSceneMembershipResponseInd( HA_GetSceneMembershipResponseInd_t *ind);

/* 6.9 */
void HA_IdentifyClusterIdentifyReq(HA_IdentifyReq_t *req);
void HA_IdentifyClusterIdentifyQueryReq(HA_IdentifyQueryReq_t *req);
void HA_IdentifyClusterIdentifyQueryResponseInd(HA_IdentifyQueryResponseInd_t *ind);

/* 6.10 */
void HA_IdentifyClusterIdentifyQueryResponseReq(HA_IdentifyQueryResponseReq_t *req);
void HA_IdentifyClusterIdentifyInd(HA_IdentifyInd_t *ind);
void HA_IdentifyClusterIdentifyQueryInd(HA_IndicationAddressingInfo_t *addrInfo);

/* 6.11 */
void HA_GroupsClusterAddGroupReq(HA_AddGroupReq_t *req);
void HA_GroupsClusterViewGroupReq(HA_ViewGroupReq_t *req);
void HA_GroupsClusterGetGroupMembershipReq(HA_GetGroupMembershipReq_t *req);
void HA_GroupsClusterRemoveGroupReq(HA_RemoveGroupReq_t *req);
void HA_GroupsClusterRemoveAllGroupsReq( HA_RequestAddressingInfo_t *addrInfo);
void HA_GroupsClusterStoreGroupReq(HA_AddGroupReq_t *req);
void HA_GroupsClusterAddGroupResponseInd(HA_AddGroupResponseInd_t *ind);
void HA_GroupsClusterViewGroupResponseInd(HA_ViewGroupResponseInd_t *ind);
void HA_GroupsClusterGetGroupMembershipResponseInd(HA_GetGroupMembershipResponseInd_t *ind);
void HA_GroupsClusterRemoveGroupResponseInd(HA_RemoveGroupResponseInd_t *ind);

/* 6.12 */
void HA_DoorLockClusterLockDoorReq(HA_LockDoorReq_t *req);
void HA_DoorLockClusterUnlockDoorReq(HA_UnlockDoorReq_t *req);
void HA_DoorLockClusterLockDoorResponseInd(HA_LockDoorResponseInd_t *ind);
void HA_DoorLockClusterUnlockDoorResponseInd( HA_UnlockDoorResponseInd_t *ind);

/* 6.13 */
void HA_LevelControlClusterMoveToLevelReq(HA_MoveToLevelReq_t *req);
void HA_LevelControlClusterMoveReq(HA_MoveLevelReq_t *req);
void HA_LevelControlClusterStepReq(HA_Step_t *cmd);
void HA_LevelControlClusterStopReq(HA_StopReq_t *req);

/* 6.14 */
void HA_WindowCoveringClusterUpOpenReq(HA_UpOpenReq_t *req);
void HA_WindowCoveringClusterDownCloseReq(HA_DownCloseReq_t *req);
void HA_WindowCoveringClusterStopReq(HA_StopReq_t *req);

/* 6.15 */
void HA_ColorControlClusterMoveToColorReq(HA_MoveToColorReq_t *req);
void HA_ColorControlClusterMoveToColorReq(HA_MoveColorReq_t *req);
void HA_ColorControlClusterStepReq(HA_StepColorReq_t *req);

/* 6.17 */
void HA_IASZoneClusterZoneEnrollResponseReq(HA_ZoneEnrollResponseReq_t *req);
void HA_IASZoneClusterZoneStatusChangeNotificationInd(HA_ZoneStatusChangeNotificationInd_t *ind);
void HA_IASZoneClusterZoneEnrollRequestInd(HA_ZoneEnrollRequestInd_t *ind);

/* 6.18 */
void HA_IASWDClusterStartWarningReq(HA_StartWarningReq_t *req);
void HA_IASWDClusterSquawkReq(HA_SquawkReq_t *req);

/* 6.19 */
void HA_IASACEClusterArmResponseReq(HA_ArmResponseReq_t *req);
void HA_IASACEClusterGetZoneIDMapResponseReq(HA_GetZoneIDMapResponseReq_t *req);
void HA_IASACEClusterGetZoneInformationResponseReq(HA_GetZoneInformationResponseReq_t *req);
void HA_IASACEClusterZoneStatusChangedReq(HA_ZoneStatusChangedReq_t *req);
void HA_IASACEClusterPanelStatusChangedReq(HA_PanelStatusChangedReq_t *req);
void HA_IASACEClusterArmInd(HA_ArmInd_t *ind);
void HA_IASACEClusterBypassInd(HA_BypassInd_t *ind);
void HA_IASACEClusterEmergencyInd(HA_IndicationAddressingInfo_t * addrInfo);
void HA_IASACEClusterFireInd(HA_IndicationAddressingInfo_t *addrInfo);
void HA_IASACEClusterPanicInd(HA_IndicationAddressingInfo_t *addrInfo);
void HA_IASACEClusterGetZoneIdMapInd( HA_IndicationAddressingInfo_t *addrInfo);
void HA_IASACEClusterGetZoneInformationInd( HA_GetZoneInformationInd_t *ind);
#endif

/* 6.20 */
#define PAYLOAD_OFFSET_C2S_HA_EnterNetworkReq_REQ 0
#define PAYLOAD_OFFSET_C2S_HA_EnterNetworkReq_REQ_data 1
#define PAYLOAD_OFFSET_S2C_HA_EnterNetworkReq_callback_REQ 0
#define PAYLOAD_OFFSET_S2C_HA_EnterNetworkReq_callback_REQ_data 1
#define PAYLOAD_OFFSET_S2C_HA_EnterNetworkReq_callback_CONF 2
#define PAYLOAD_OFFSET_S2C_HA_EnterNetworkReq_callback_CONF_data 3

#if 0   /* TODO */

#ifdef SERVER

void HA_EnterNetworkReq(HA_EnterNetworkReq_t *req) {}

void server_HA_EnterNetworkReq(unsigned int *buf, int socket)
{
    HA_EnterNetworkReq_t request;

    zigbee_api_cb.HA_EnterNetworkReq.socket = socket;
    zigbee_api_cb.HA_EnterNetworkReq.server_request = (unsigned int)&request;
    zigbee_api_cb.HA_EnterNetworkReq.client_request = (HA_EnterNetworkReq_t *)buf[PAYLOAD_OFFSET_C2S_HA_EnterNetworkReq_REQ];
    memcpy((char *)&zigbee_api_cb.HA_EnterNetworkReq.client_request_data, (char *)&buf[PAYLOAD_OFFSET_C2S_HA_EnterNetworkReq_REQ_data], sizeof(zigbee_api_cb.HA_EnterNetworkReq.client_request_data));

    memcpy((char*)&request, (char*)&buf[PAYLOAD_OFFSET_C2S_HA_EnterNetworkReq_REQ], sizeof(HA_EnterNetworkReq_t));

    request.HA_EnterNetworkConf = server_HA_EnterNetworkReq_callback;
    HA_EnterNetworkReq(&request);
}

#else
void HA_EnterNetworkReq(HA_EnterNetworkReq_t *req)
{
    unsigned int buffer[RPC_FRAME_LENGTH_IN_WORDS];
    int size;

    /* Compose the Request message */
    memset(&buffer[0], 0, sizeof(buffer));
    buffer[PAYLOAD_OFFSET_C2S_HA_EnterNetworkReq_REQ]=(unsigned int)req; /* 1st parameter */
    memcpy((char *)&buffer[PAYLOAD_OFFSET_C2S_HA_EnterNetworkReq_REQ_data], req, sizeof(RF4CE_StartReqDescr_t));
    Zigbee_Rpc_Send(buffer, 120/4, RPC_C2S_HA_EnterNetworkReq, Zigbee_Socket_ClientSend, 0);   /* Upon return of this function, the data is copied to the driver so the local buffer can be freed */
}
#endif


#ifdef SERVER
void server_HA_EnterNetworkReq_callback(HA_EnterNetworkReq_t *origReq, HA_EnterNetworkConf_t *conf)
{
    int socket = zigbee_api_cb.HA_EnterNetworkReq.socket;
    int size;
    int message_tx[MAX_MSG_SIZE_IN_WORDS];
    printf("ZIGBEE_RPC_API:  invoking server_HA_EnterNetworkReq_callback\n");

    size = 0;
    message_tx[PAYLOAD_OFFSET_S2C_HA_EnterNetworkReq_callback_REQ] = (unsigned int)zigbee_api_cb.HA_EnterNetworkReq.client_request;
    size += (sizeof(zigbee_api_cb.HA_EnterNetworkReq.client_request) + 3) / 4;

    memcpy((char *)&message_tx[PAYLOAD_OFFSET_S2C_HA_EnterNetworkReq_callback_REQ_data], (char *)&zigbee_api_cb.HA_EnterNetworkReq.client_request_data, sizeof(zigbee_api_cb.HA_EnterNetworkReq.client_request_data)); // client
    size += (sizeof(zigbee_api_cb.HA_EnterNetworkReq.client_request_data) + 3) / 4;

    message_tx[PAYLOAD_OFFSET_S2C_HA_EnterNetworkReq_callback_CONF] = (unsigned int)conf;
    size += (sizeof(conf) + 3) / 4;

    memcpy((char *)&message_tx[PAYLOAD_OFFSET_S2C_HA_EnterNetworkReq_callback_CONF_data], (char *)&conf, sizeof(HA_EnterNetworkConf_t));
    size += (sizeof(HA_EnterNetworkConf_t) + 3) / 4;

    Zigbee_Rpc_Send(message_tx, size, RPC_C2S_HA_EnterNetworkReq | RPC_RESPONSE, Zigbee_Socket_ServerSend, socket);
    printf("ZIGBEE_RPC_API:  socket %d HA Network formed\n", socket);
    socket_cb[socket].state = SOCKET_IDLE;
}
#else
client_HA_EnterNetworkReq_callback(unsigned int *message_rx)
{
    HA_EnterNetworkReq_t origReq;
    HA_EnterNetworkConf_t conf;
    memcpy((char *)&origReq, (char *)&message_rx[PAYLOAD_OFFSET_S2C_HA_EnterNetworkReq_callback_REQ_data], sizeof(origReq));
    memcpy((char *)&conf, (char *)&message_rx[PAYLOAD_OFFSET_S2C_HA_EnterNetworkReq_callback_CONF_data], sizeof(conf));
    if (origReq.HA_EnterNetworkConf) {
        origReq.HA_EnterNetworkConf((HA_EnterNetworkReq_t *)&message_rx[PAYLOAD_OFFSET_S2C_HA_EnterNetworkReq_callback_REQ], &conf);
    };
}
#endif

#endif  /* TODO */

#if 0
void HA_ResetToFactoryFreshReq(HA_ResetToFactoryFreshReq_t *req);
void HA_EZModeNetworkSteeringReq(HA_EZModeNetworkSteeringReq_t *req);
void HA_EZModeFindingAndBindingReq( HA_EZModeFindingAndBindingReq_t *req);
void HA_PermitJoiningReq(HA_PermitJoiningReq_t *req);
void HA_SetReq(HA_SetReq_t *req);
void HA_GetReq(HA_GetReq_t *req);
#endif

#ifdef TEST
    #ifndef SERVER
        void ServerLoopbackStart(void)
        {
            Zigbee_Rpc_Send(NULL, 0, RPC_C2S_ServerLoopbackStart, Zigbee_Socket_ClientSend, 0);
        }
    #endif

    #define PAYLOAD_OFFSET_C2S_SERVER_LOOPBACK_CALLBACK             0
    #define PAYLOAD_OFFSET_C2S_SERVER_LOOPBACK_NUM_OF_WORDS         1
    #define PAYLOAD_OFFSET_C2S_SERVER_LOOPBACK_SERVER_TX_BUFFER     2
    #define PAYLOAD_OFFSET_C2S_SERVER_LOOPBACK_SERVER_RX_BUFFER     3
    #define PAYLOAD_OFFSET_C2S_SERVER_LOOPBACK_RX_DATA              4

    #ifdef SERVER
        void server_ServerLoopbackInd_callback(unsigned int *buf, int socket)
        {
            int i;
            unsigned int num_of_words;
            unsigned int *tx_buffer, *rx_buffer;
            void (*callback)(unsigned int *tx_buffer, unsigned int *rx_buffer, unsigned int num_of_words);

            /* invoke callback */
            printf("ZIGBEE_RPC_API:  in server_ServerLoopbackInd_callback\n");
            callback = (void (*)(unsigned int *, unsigned int *, unsigned int))buf[PAYLOAD_OFFSET_C2S_SERVER_LOOPBACK_CALLBACK];
            num_of_words = buf[PAYLOAD_OFFSET_C2S_SERVER_LOOPBACK_NUM_OF_WORDS];
            tx_buffer = (unsigned int *)buf[PAYLOAD_OFFSET_C2S_SERVER_LOOPBACK_SERVER_TX_BUFFER];
            rx_buffer = (unsigned int *)buf[PAYLOAD_OFFSET_C2S_SERVER_LOOPBACK_SERVER_RX_BUFFER];
            num_of_words = buf[PAYLOAD_OFFSET_C2S_SERVER_LOOPBACK_NUM_OF_WORDS];

            assert(num_of_words < MAX_MSG_SIZE_IN_WORDS);

            /* copy data */
            for (i=0; i<num_of_words; i++) {
                rx_buffer[i] = buf[PAYLOAD_OFFSET_C2S_SERVER_LOOPBACK_RX_DATA+i];
            }
            callback(tx_buffer, rx_buffer, num_of_words);
        }
    #else
        unsigned int ServerLoopbackInd_remote_callback;
        unsigned int ServerLoopbackInd_remote_server_tx_buffer;
        unsigned int ServerLoopbackInd_remote_server_rx_buffer;
        void client_ServerLoopbackInd_callback(unsigned int *tx_buffer, unsigned int *rx_buffer, unsigned int num_of_words)
        {
            unsigned int buffer[RPC_FRAME_LENGTH_IN_WORDS];
            int i;
            memset(&buffer[0], 0, sizeof(buffer));
            buffer[PAYLOAD_OFFSET_C2S_SERVER_LOOPBACK_CALLBACK] = ServerLoopbackInd_remote_callback;
            buffer[PAYLOAD_OFFSET_C2S_SERVER_LOOPBACK_NUM_OF_WORDS] = num_of_words;
            buffer[PAYLOAD_OFFSET_C2S_SERVER_LOOPBACK_SERVER_TX_BUFFER] = ServerLoopbackInd_remote_server_tx_buffer;
            buffer[PAYLOAD_OFFSET_C2S_SERVER_LOOPBACK_SERVER_RX_BUFFER] = ServerLoopbackInd_remote_server_rx_buffer;
            for (i=0; i<num_of_words; i++) {
                buffer[PAYLOAD_OFFSET_C2S_SERVER_LOOPBACK_RX_DATA+i] = tx_buffer[i];
            }
            Zigbee_Rpc_Send(buffer, PAYLOAD_OFFSET_C2S_SERVER_LOOPBACK_RX_DATA+num_of_words+1, (RPC_S2C_ServerLoopbackInd | RPC_RESPONSE), Zigbee_Socket_ClientSend, 0);   /* Upon return of this function, the data is copied to the driver so the local buffer can be freed */
        }
    #endif

    #define PAYLOAD_OFFSET_S2C_SERVER_LOOPBACK_CALLBACK             0
    #define PAYLOAD_OFFSET_S2C_SERVER_LOOPBACK_NUM_OF_WORDS         1
    #define PAYLOAD_OFFSET_S2C_SERVER_LOOPBACK_SERVER_TX_BUFFER     2
    #define PAYLOAD_OFFSET_S2C_SERVER_LOOPBACK_SERVER_RX_BUFFER     3
    #define PAYLOAD_OFFSET_S2C_SERVER_LOOPBACK_TX_DATA              4
    #ifdef SERVER
        void server_ServerLoopbackStart(int socket)
        {
            socket_cb[socket].state=SOCKET_NEED_TO_SEND_RPC_S2C_ServerLoopbackInd;
        }

        void ServerLoopbackInd(int socket, unsigned int *server_tx_buffer, unsigned int *server_rx_buffer, unsigned int num_of_words_to_test, void (*server_ServerLoopbackInd_callback)(unsigned int *tx_buffer, unsigned int *rx_buffer, unsigned int num_of_words))
        {
            int i;
            int size;
            int message_tx[MAX_MSG_SIZE_IN_WORDS];

            printf("ZIGBEE_RPC_API:  in ServerLoopbackInd:  server_tx_buffer=%p, server_rx_buffer=%p, num_of_words_to_test=%d\n", server_tx_buffer, server_rx_buffer, num_of_words_to_test);
            message_tx[PAYLOAD_OFFSET_S2C_SERVER_LOOPBACK_NUM_OF_WORDS] = num_of_words_to_test;
            message_tx[PAYLOAD_OFFSET_S2C_SERVER_LOOPBACK_CALLBACK] = (unsigned int)server_ServerLoopbackInd_callback;
            message_tx[PAYLOAD_OFFSET_S2C_SERVER_LOOPBACK_SERVER_TX_BUFFER] = (unsigned int)server_tx_buffer;
            message_tx[PAYLOAD_OFFSET_S2C_SERVER_LOOPBACK_SERVER_RX_BUFFER] = (unsigned int)server_rx_buffer;

            for (i=0; i<num_of_words_to_test; i++) {
                message_tx[PAYLOAD_OFFSET_S2C_SERVER_LOOPBACK_TX_DATA+i] = server_tx_buffer[i];
            }

            size = PAYLOAD_OFFSET_S2C_SERVER_LOOPBACK_TX_DATA + num_of_words_to_test + 1;
            socket_cb[socket].message_id = RPC_S2C_ServerLoopbackInd;
            Zigbee_Rpc_Send(message_tx, size, RPC_S2C_ServerLoopbackInd, Zigbee_Socket_ServerSend, socket);
            socket_cb[socket].state = SOCKET_NEED_TO_WAIT_FOR_RPC_S2C_ServerLoopbackInd_callback;
        }
    #else
        void client_ServerLoopbackInd(zigbeeCallback *pZigbeeCallback, unsigned int *message_rx)
        {
            /* save remote parameters */
            int num_of_words;
            num_of_words = message_rx[PAYLOAD_OFFSET_S2C_SERVER_LOOPBACK_NUM_OF_WORDS];
            assert(num_of_words < MAX_MSG_SIZE_IN_WORDS);
            ServerLoopbackInd_remote_callback = message_rx[PAYLOAD_OFFSET_S2C_SERVER_LOOPBACK_CALLBACK];
            ServerLoopbackInd_remote_server_tx_buffer = message_rx[PAYLOAD_OFFSET_S2C_SERVER_LOOPBACK_SERVER_TX_BUFFER];
            ServerLoopbackInd_remote_server_rx_buffer = message_rx[PAYLOAD_OFFSET_S2C_SERVER_LOOPBACK_SERVER_RX_BUFFER];

            /* overwrite with local callback */
            /*indication.callback = ServerLoopbackInd_callback;*/
            #ifdef DEBUG
            {
                int i;
                printf("from client:\n");
                for (i=0; i<num_of_words; i++) {
                    printf("%d:  0x%08x\n", i, message_rx[PAYLOAD_OFFSET_S2C_SERVER_LOOPBACK_TX_DATA+i]);
                }
            }
            #endif

            if (pZigbeeCallback->ServerLoopbackInd != NULL) {
                unsigned int *tx_buffer, *rx_buffer;
                tx_buffer = (unsigned int *)malloc(num_of_words*4);
                rx_buffer = (unsigned int *)malloc(num_of_words*4);
                memcpy((char *)tx_buffer, (char *)&message_rx[PAYLOAD_OFFSET_S2C_SERVER_LOOPBACK_TX_DATA], num_of_words*4);
                pZigbeeCallback->ServerLoopbackInd(tx_buffer, rx_buffer, num_of_words, client_ServerLoopbackInd_callback);
                free(rx_buffer);
                free(tx_buffer);
            } else {
                printf("ZIGBEE_RPC_API:  no callback provided for ServerLoopbackInd...\n");
            }
        }
    #endif

    #ifdef SERVER
        void server_ClientRawBufferLoopback(int socket)
        {
            int i;
            int message_tx[MAX_MSG_SIZE_IN_WORDS];
            for (i=0; i<socket_cb[socket].message_rx_payload_size_in_words; i++) {
                message_tx[i] = socket_cb[socket].message_rx[i];
            }
            Zigbee_Rpc_Send(message_tx, socket_cb[socket].message_rx_payload_size_in_words, RPC_C2S_ClientRawBufferLoopback | RPC_RESPONSE, Zigbee_Socket_ServerSend, socket);
            socket_cb[socket].state = SOCKET_IDLE;
        }
    #else
        void client_ClientRawBufferLoopback_callback(zigbeeCallback *pZigbeeCallback, unsigned int *message_rx, unsigned int message_payload_size_in_words)
        {
            int i;
            bool test_passed=true;
            pZigbeeCallback->BufferCompare(message_rx, message_payload_size_in_words);
        }
    #endif

    #ifdef SERVER
        void server_ClientRawBufferCoreLoopback(int socket)
        {
            int payload_words_to_send_this_time;
            int write_ptr = 0;
            unsigned int buffer[MBOX_FRAME_SIZE_IN_WORDS];
            int payload_words_remaining_to_send = socket_cb[socket].message_rx_payload_size_in_words;
            zigbee_api_cb.ClientRawBufferCoreLoopbackReq.socket = socket;
            printf("saved socket number=%d\n", socket);

            while (payload_words_remaining_to_send > 0) {
                memset((char *)buffer, 0, MBOX_FRAME_SIZE_IN_WORDS);
                MBOX_FRAME_SET_MSG_ID(&buffer[MBOX_FRAME_HEADER_OFFSET], MBOX_ClientRawBufferCoreLoopback);
                if (payload_words_remaining_to_send > MBOX_FRAME_PAYLOAD_SIZE_IN_WORDS) {
                    payload_words_to_send_this_time = MBOX_FRAME_PAYLOAD_SIZE_IN_WORDS;
                    MBOX_FRAME_SET_MSG_FRAGMENT(&buffer[MBOX_FRAME_HEADER_OFFSET], 1);
                } else {
                    payload_words_to_send_this_time = payload_words_remaining_to_send;
                    MBOX_FRAME_SET_MSG_FRAGMENT(&buffer[MBOX_FRAME_HEADER_OFFSET], 0);
                }

                MBOX_FRAME_SET_LENGTH(&buffer[MBOX_FRAME_HEADER_OFFSET], payload_words_to_send_this_time+MBOX_FRAME_HEADER_SIZE_IN_WORDS-1);
                memcpy((char *)&buffer[MBOX_FRAME_PAYLOAD_OFFSET], (char *)&socket_cb[socket].message_rx[write_ptr], payload_words_to_send_this_time*4);
                assert(Zigbee_Ioctl_WriteToMbox(g_zigbeeFd, (char *)&buffer[0]) != ZIGBEE_WRITE_TO_MBOX_ERROR);
                payload_words_remaining_to_send -= payload_words_to_send_this_time;
                if (payload_words_remaining_to_send) {
                    MBOX_FRAME_SET_MSG_FRAGMENT(&buffer[MBOX_FRAME_HEADER_OFFSET], 1);
                }
                write_ptr += payload_words_to_send_this_time;
            }
        }
    #endif

    #ifdef SERVER
        void server_ClientRawBufferCoreLoopbackConf(unsigned int *message_buffer, int payload_size, int socket)
        {
            Zigbee_Rpc_Send(message_buffer, payload_size, RPC_C2S_ClientRawBufferCoreLoopback | RPC_RESPONSE, Zigbee_Socket_ServerSend, socket);
        }
    #else
        void client_ClientRawBufferCoreLoopback_callback(zigbeeCallback *pZigbeeCallback, unsigned int *message_rx, unsigned int message_payload_size_in_words)
        {
            int i;
            bool test_passed=true;
            printf("ZIGBEE_RPC_API:  raw buffers received!  message_payload_size_in_words=%d\n", message_payload_size_in_words);
            pZigbeeCallback->BufferCompare(message_rx, message_payload_size_in_words);
        }
    #endif

    #define PAYLOAD_OFFSET_C2S_CLIENT_LOOPBACK_CALLBACK         0
    #define PAYLOAD_OFFSET_C2S_CLIENT_LOOPBACK_NUM_OF_WORDS     1
    #define PAYLOAD_OFFSET_C2S_CLIENT_LOOPBACK_RX_BUFFER        2
    #define PAYLOAD_OFFSET_C2S_CLIENT_LOOPBACK_DATA             3

    #define PAYLOAD_OFFSET_S2C_CLIENT_LOOPBACK_CALLBACK_CALLBACK        0
    #define PAYLOAD_OFFSET_S2C_CLIENT_LOOPBACK_CALLBACK_NUM_OF_WORDS    1
    #define PAYLOAD_OFFSET_S2C_CLIENT_LOOPBACK_CALLBACK_RXBUFFER        2
    #define PAYLOAD_OFFSET_S2C_CLIENT_LOOPBACK_CALLBACK_DATA            3

    #ifdef SERVER
        void server_ClientLoopbackReq(int socket)
        {
            unsigned int num_of_words = socket_cb[socket].message_rx[PAYLOAD_OFFSET_C2S_CLIENT_LOOPBACK_NUM_OF_WORDS];
            int i;
            int size;
            int message_tx[MAX_MSG_SIZE_IN_WORDS];

            /* process tx buffer */
            message_tx[PAYLOAD_OFFSET_S2C_CLIENT_LOOPBACK_CALLBACK_CALLBACK] = socket_cb[socket].message_rx[PAYLOAD_OFFSET_C2S_CLIENT_LOOPBACK_CALLBACK];
            message_tx[PAYLOAD_OFFSET_S2C_CLIENT_LOOPBACK_CALLBACK_RXBUFFER] = socket_cb[socket].message_rx[PAYLOAD_OFFSET_C2S_CLIENT_LOOPBACK_RX_BUFFER];
            message_tx[PAYLOAD_OFFSET_S2C_CLIENT_LOOPBACK_CALLBACK_NUM_OF_WORDS] = socket_cb[socket].message_rx[PAYLOAD_OFFSET_C2S_CLIENT_LOOPBACK_NUM_OF_WORDS];

            for (i=0; i<num_of_words; i++) {
                message_tx[PAYLOAD_OFFSET_S2C_CLIENT_LOOPBACK_CALLBACK_DATA+i] = socket_cb[socket].message_rx[PAYLOAD_OFFSET_C2S_CLIENT_LOOPBACK_DATA+i];
            }

            size = PAYLOAD_OFFSET_S2C_CLIENT_LOOPBACK_CALLBACK_DATA + num_of_words;
            Zigbee_Rpc_Send(message_tx, size, RPC_C2S_ClientLoopback | RPC_RESPONSE, Zigbee_Socket_ServerSend, socket);
            socket_cb[socket].state = SOCKET_IDLE;
        }
    #else
        void client_ClientLoopback_callback(unsigned int *message_rx)
        {
            int i;
            void (*func)(unsigned int*, unsigned int) = (void (*)(unsigned int*, unsigned int))message_rx[PAYLOAD_OFFSET_S2C_CLIENT_LOOPBACK_CALLBACK_CALLBACK];
            unsigned int *rx_buffer = (unsigned int *)message_rx[PAYLOAD_OFFSET_S2C_CLIENT_LOOPBACK_CALLBACK_RXBUFFER];
            unsigned int num_of_words = message_rx[PAYLOAD_OFFSET_S2C_CLIENT_LOOPBACK_CALLBACK_NUM_OF_WORDS];
            for (i=0; i<num_of_words; i++) {
                rx_buffer[i] = message_rx[PAYLOAD_OFFSET_S2C_CLIENT_LOOPBACK_CALLBACK_DATA+i];
            }
            func(rx_buffer, num_of_words);
        }

        int ClientLoopbackReq(unsigned int *tx_buffer, unsigned int num_of_words, void (*callback)(unsigned int *rx_buffer, unsigned int num_of_words), unsigned int *rx_buffer)
        {
            unsigned int buffer[MAX_MSG_SIZE_IN_WORDS];
            if (num_of_words > (MAX_MSG_SIZE_IN_WORDS-PAYLOAD_OFFSET_C2S_CLIENT_LOOPBACK_DATA)) {
                return -1;
            }
            buffer[PAYLOAD_OFFSET_C2S_CLIENT_LOOPBACK_NUM_OF_WORDS]=(unsigned int)num_of_words;
            buffer[PAYLOAD_OFFSET_C2S_CLIENT_LOOPBACK_CALLBACK]=(unsigned int)callback;
            buffer[PAYLOAD_OFFSET_C2S_CLIENT_LOOPBACK_RX_BUFFER]=(unsigned int)rx_buffer;
            memcpy((char *)&buffer[PAYLOAD_OFFSET_C2S_CLIENT_LOOPBACK_DATA], (char *)tx_buffer, num_of_words*4);
            Zigbee_Rpc_Send(buffer, num_of_words+PAYLOAD_OFFSET_C2S_CLIENT_LOOPBACK_RX_BUFFER+1, RPC_C2S_ClientLoopback, Zigbee_Socket_ClientSend, 0);   /* Upon return of this function, the data is copied to the driver so the local buffer can be freed */
            return 0;
        }
    #endif
#endif
