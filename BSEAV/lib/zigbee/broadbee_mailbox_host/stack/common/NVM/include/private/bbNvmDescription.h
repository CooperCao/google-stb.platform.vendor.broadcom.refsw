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
 *      Non volatile memory structure and API.
 *
*******************************************************************************/

#ifndef BBNVMDESC_H_
#define BBNVMDESC_H_
/************************* INCLUDES ****************************************************/
#include "bbSysQueue.h"
#include "bbSysMemMan.h"
#include "bbNvmApi.h"
#include "private/bbZbProApsEndpointPrivateTypes.h"

/************************* TYPES ********************************************************/
/**//**
 * \brief Nvm scenario states.
 */
typedef enum _NvmScenarioStep_t
{
    NVM_SCENARIO_IDLE,
    NVM_SCENARIO_OPEN,
    NVM_SCENARIO_CLOSE,
} NvmScenarioStep_t;

/**//**
 * \brief Service structure to place request into queue.
 */
typedef struct _NvmMemReq_t
{
    SYS_QueueElement_t   next;
    MM_ChunkId_t         blockId;
    NvmTransacCb_t       cb;
    void                *cbParam;
    uint16_t             offset;
    uint16_t             length;
    NvmTransacType_t     type;
    NvmInstanceIds_t     id;
    NvmScenarioStep_t    step;
    uint8_t              cbNum;     /* Number of times the callback has to be called */
} NvmMemReq_t;

/**//**
 * \brief Service structure to place request into queue.
 */
typedef struct _NvmMemBufferedReq_t
{
    SYS_QueueElement_t              next;
    MM_ChunkId_t                    blockId;
    NvmBufferedTransacReq_t         request;
    Nvm_BufferedTransactionStatus   status;
} NvmMemBufferedReq_t;

/**//**
 * \bief Union for all data-types, which could be saved by BufferedTransaction.
 * This union is used to get a maximum size of the BufferedTransaction static buffer.
 * All types, which will be saved by BufferedTransaction must be appended to this union.
*/
typedef union _NVM_BufferedTransactBufferUnion_t
{
    ZbProApsEndpointClusterList_t   endpointClusterList;
} NVM_BufferedTransactBufferUnion_t;

typedef  uint8_t  NvmBufferUsedLength_t;

#define NVM_BUFFERED_TRANSACT_DATA_SIZE    sizeof(NVM_BufferedTransactBufferUnion_t)
#define NVM_BUFFERED_TRANSACT_BUFFER_SIZE  NVM_BUFFERED_TRANSACT_DATA_SIZE + sizeof(NvmBufferUsedLength_t)
SYS_DbgAssertStatic(UINT8_MAX > NVM_BUFFERED_TRANSACT_BUFFER_SIZE);


/**//**
 * \brief Struct for BufferedTransaction buffer descriptor.
*/
typedef struct _Nvm_BufferedTransactBufferDescr_t
{
    SYS_QueueDescriptor_t          queue;                   /*!< Queue for BufferedTransaction requests. */
    bool     isInUse;                                       /*!< Is buffer in use now. */
    struct
    {
        NvmBufferUsedLength_t usedLength;                   /*!< Length of the data, which are reading or writing. */
        uint8_t  data[NVM_BUFFERED_TRANSACT_DATA_SIZE];     /*!< Buffer rawData. */
    } rawData;
} Nvm_BufferedTransactBufferDescr_t;

/**//**
 * \brief Strcuture with possible scenarios.
 */
#define SCENARIO_DECLARATION(id, size, params)   NvmInstanceIds_t id##arr[size];
typedef struct PACKED _NvmScenarios_t
{
#   include "bbNvmScenarios.h"
} NvmScenarios_t;
#undef SCENARIO_DECLARATION

/**//**
 * \brief Parameter types
 */
typedef enum _NvmParameterTypes_t
{
    NVM_PARAMETER_ORDINARY,
    NVM_PARAMETER_UNION,        /* subsequent NVM parameters of common size equal to the size of
                                   this parameter can be accesses at once using this parameter ID */
    NVM_PARAMETER_ARRAY         /* use to access a big size parameter which is an array of fixed size elements
                                   which size has to be set as destination RAM location */
} NvmParameterTypes_t;

/**//**
 * \brief Parameter descriptor.
 */
typedef struct PACKED _NvmParameters_t
{
    uint8_t             *data;
    uint16_t            length;
    NvmParameterTypes_t parameterType;
} NvmParameters_t;

/**//**
 * \brief NVM descriptors for data stored to non volatile memory.
 */
typedef struct _NvmDescr_t
{
    NvmParameters_t parameters[NVM_LAST_PARAM_ID];
    NvmScenarios_t scenarios;
} NvmDescr_t;

#endif /* BBNVMDESC_H_ */

/* eof bbNvmDescription.h */