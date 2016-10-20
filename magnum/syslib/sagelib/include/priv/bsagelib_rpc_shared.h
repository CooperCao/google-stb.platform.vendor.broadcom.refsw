/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
 ******************************************************************************/

#ifndef BSAGELIB_RPC_SHARED_H_
#define BSAGELIB_RPC_SHARED_H_

#include "bsagelib_types.h"


#ifdef __cplusplus
extern "C" {
#endif


/* message protocol version */
#define BSAGELIB_MESSAGE_VERSION_MINOR 0x0000

#if SAGE_VERSION < SAGE_VERSION_CALC(3,0)
#define BSAGELIB_MESSAGE_VERSION_MAJOR 0x0002
#else
#define BSAGELIB_MESSAGE_VERSION_MAJOR 0x0003
#endif

#define BSAGELIB_MESSAGE_VERSION ((BSAGELIB_MESSAGE_VERSION_MAJOR << 16) | BSAGELIB_MESSAGE_VERSION_MINOR)



#if SAGE_VERSION < SAGE_VERSION_CALC(3,0)
/************************************************************************
  There are two types of 'channels'
   - Request channel, from Host to SAGE, used to
       send a request from Host to SAGE (RpcRequest message)
   - Response channel, from SAGE to Host, used to
       send a response from SAGE to Host (RpcResponse message)
         (there is exactly one response per request)
       send an indication from SAGE to Host (RpcIndication message)
**************************************************************************/

typedef struct
{
    uint64_t messageOffset; /* physical address of a BSAGElib_RpcMessage */
} BSAGElib_RpcRequest;

typedef struct
{
    uint32_t rc;     /* Request return Code */
} BSAGElib_RpcAck;

typedef enum
{
    BSAGElib_Response_eResponse = 0x1,
    BSAGElib_Response_eIndication
} BSAGElib_Response_eType;

typedef struct
{
    uint32_t responseType;    /* BSAGElib_Response_eResponse */
    uint32_t rc;              /* Request processing return Code for response */
    uint64_t messageOffset;   /* Physical address of a BSAGElib_RpcMessage : if 0, the response is an indication and is not bound to any request. */
} BSAGElib_RpcResponse;

typedef struct
{
    uint32_t responseType;    /* BSAGElib_Response_eIndication */
    uint32_t instanceId;      /* instance Id to identify to which platform/module this indication belongs */
    uint32_t id;              /* Indication IDentifier */
    uint32_t value;           /* Indication value */
} BSAGElib_RpcIndication;

/* type used to get the sized of the bigger of all that can be backhauled by response channel */
typedef union {
    uint32_t responseType;
    BSAGElib_RpcResponse response;
    BSAGElib_RpcIndication indication;
} BSAGElib_Response;

/* could define a BSAGElib_Request similar to BSAGElib_Response but request only backhault one type of message */
#else
/************************************************************************
  There are three 'channels'
   - Request channel, from Host to SAGE, used to
       send a request from Host to SAGE (RpcRequest message)
       send a callback response from Host to SAGE (RpcCallbackResponse message)
         (there is exactly one response from Host per request from SAGE)
   - ACK Channel
       send an ACK from SAGE to Host to acknowledge good reception of a request
   - Response channel, from SAGE to Host, used to
       send a response from SAGE to Host (RpcResponse message)
         (there is exactly one response from SAGE per request from Host)
       send a callback request from SAGE to Host (RpcCallbackRequest message)
       ! there is no ACK to a callback request
       send an indication from SAGE to Host (RpcIndication message)
**************************************************************************/

typedef enum
{
    /* Request Channel messages (Host to SAGE) */
    BSAGElib_RpcMessage_eRequest = 0x001,
    BSAGElib_RpcMessage_eCallbackResponse,

    /* Response Channel messages (SAGE to Host) */
    BSAGElib_RpcMessage_eResponse = 0x010,
    BSAGElib_RpcMessage_eIndication,
    BSAGElib_RpcMessage_eCallbackRequest,
    BSAGElib_RpcMessage_eTATerminate,

    /* ACK Channel messages (SAGE to Host) */
    BSAGElib_RpcMessage_eRequestAck = 0x100

} BSAGElib_RpcMessage_eType;

/* note: the type used for BSAGElib_RpcMessage_eType is uint32_t in the Rpc* structures. */

typedef struct
{
    uint32_t type;            /* BSAGElib_RpcMessage_eRequest or BSAGElib_RpcMessage_eCallbackRequest */
    uint64_t messageOffset;   /* physical address of a BSAGElib_RpcMessage */
} BSAGElib_RpcRequest;

typedef struct
{
    uint32_t type;            /* BSAGElib_RpcMessage_eRequestAck or BSAGElib_RpcMessage_eCallbackRequestAck */
    uint32_t rc;              /* Request return Code */
} BSAGElib_RpcAck;

typedef struct
{
    uint32_t type;            /* BSAGElib_RpcMessage_eResponse or BSAGElib_RpcMessage_eCallbackResponse */
    uint32_t rc;              /* Request processing return Code for response */
    uint64_t messageOffset;   /* Physical address of a BSAGElib_RpcMessage */
} BSAGElib_RpcResponse;

typedef struct
{
    uint32_t type;            /* BSAGElib_Response_eIndication */
    uint32_t instanceId;      /* instance Id to identify to which platform/module this indication belongs */
    uint32_t id;              /* Indication IDentifier */
    uint32_t value;           /* Indication value */
} BSAGElib_RpcIndication;

typedef struct
{
    uint32_t type;            /* BSAGElib_Response_eTATerminate */
    uint32_t instanceId;      /* instance Id to identify to which platform (TA) had been terminated */
    uint32_t reason;          /* reason why the TA got terminated */
    uint32_t source;          /* source of the command that was processing while terminated (0: host) */
} BSAGElib_RpcTATerminate;

/* type used to get the sized of the bigger of all that can be backhauled by ACK channel */
typedef union
{
    uint32_t type;
    BSAGElib_RpcAck ack;
} BSAGElib_AckChannel;

/* type used to get the sized of the bigger of all that can be backhauled by response channel */
typedef union {
    uint32_t type;
    BSAGElib_RpcResponse response;
    BSAGElib_RpcRequest callbackRequest;
    BSAGElib_RpcIndication indication;
    BSAGElib_RpcTATerminate taTerminate;
} BSAGElib_ResponseChannel;

/* type used to get the sized of the bigger of all that can be backhauled by request channel */
typedef union {
    uint32_t type;
    BSAGElib_RpcRequest request;
    BSAGElib_RpcResponse callbackResponse;
} BSAGElib_RequestChannel;
#endif

/* RpcMessage type is used to carry request from one another */
typedef struct
{
    uint32_t version;         /* Sage message protocol version : BSAGELIB_MESSAGE_VERSION */
    uint32_t timestamp;       /* Time Stamp in micro seconds */
    uint32_t sequence;        /* Message Sequence number, unique system-wide */
    uint32_t instanceId;      /* Instance ID */
    uint32_t systemCommandId; /* System Command ID, processed by SAGE RPC */
    uint32_t platformId;      /* Platform ID (unique in the system) */
    uint32_t moduleId;        /* Module ID (unique per Platform) */
    uint32_t moduleCommandId; /* Module command ID to dispatch */
    /* next has to be 64 bits aligned */
    uint64_t payloadOffset;   /* Container physical address */
} BSAGElib_RpcMessage;

#ifdef BSAGELIB_MESSAGE_DEBUG
#include "bdbg.h"
#define BSAGELIB_DUMP_MESSAGE(STR, MESSAGE, RAW)                    \
    if (RAW) {                                                      \
        unsigned int i, j;                                          \
        uint8_t *b = (uint8_t *)(MESSAGE);                          \
        for (i = 0, j = 0; i < sizeof(*(MESSAGE)); i+=4, j++) {     \
            uint32_t *w = (uint32_t *)b;                            \
            BDBG_MSG(("DUMP #%02d\t0x%08x\t%02x %02x %02x %02x ",   \
                      j, *w, *b, *(b+1), *(b+2), *(b+3)));          \
            b+=4;                                                   \
        }                                                           \
    }                                                               \
    BDBG_LOG(("DUMP message %s @ 0x%08x (%d bytes):\n"              \
              "\t version=%08x\n"                                   \
              "\t timestamp=%08x\n"                                 \
              "\t sequence=%08x\n"                                  \
              "\t instanceId=%08x\n"                                \
              "\t systemCommandId=%08x\n"                           \
              "\t platformId=%08x\n"                                \
              "\t moduleId=%08x\n"                                  \
              "\t moduleCommandId=%08x\n"                           \
              "\t payloadOffset=0x%08x\n",                          \
              (STR), (void *)(MESSAGE), sizeof(*(MESSAGE)),         \
              (MESSAGE)->version, (MESSAGE)->timestamp,             \
              (MESSAGE)->sequence, (MESSAGE)->instanceId,           \
              (MESSAGE)->systemCommandId, (MESSAGE)->platformId,    \
              (MESSAGE)->moduleId, (MESSAGE)->moduleCommandId,      \
              (uint32_t)(MESSAGE)->payloadOffset))
#else
#define BSAGELIB_DUMP_MESSAGE(STR, MESSAGE, RAW)
#endif


#ifdef __cplusplus
}
#endif


#endif /* BSAGELIB_RPC_SHARED_H_ */
