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
* FILENAME: $Workfile: $
*
* DESCRIPTION:
*   Non volatile memory API.
*
* $Revision: $
* $Date: $
*
*****************************************************************************************/

#ifndef BBNVMAPI_H_
#define BBNVMAPI_H_
/************************* INCLUDES ****************************************************/
#include "bbSysBasics.h"
#include "bbSysPayload.h"
#ifdef _RF4CE_
# include "bbRF4CEConfig.h" // NOTE: workaround. We should include the RF4CE config file to accept appropriate flags/constants.
#include "bbRF4CENWKConstants.h"
#endif
#include "bbZbProApsConfig.h"
/************************* TYPES ********************************************************/
/**//**
 * \brief Nvm callback type.
 */
typedef void (* NvmTransacCb_t)(void *);

/**//**
 * \brief Nvm transaction types.
 */
typedef enum _NvmTransacType_t
{
    NVM_READ_TRANSAC,
    NVM_WRITE_TRANSAC
} NvmTransacType_t;

/**//**
 * \brief List of parameter identificators which stored into nvm.
 */
#define PARAMETER_DECLARATION(id, pointer, size)    id##_ID,
#define UNION_DECLARATION PARAMETER_DECLARATION
#define ARRAY_DECLARATION PARAMETER_DECLARATION
#define SCENARIO_DECLARATION(id, size, params)      id##_ID,
typedef enum PACKED _NvmInstanceIds_t
{
#   include "bbNvmParameters.h"
    NVM_INVALID_PARAM_ID,
    NVM_LAST_PARAM_ID = NVM_INVALID_PARAM_ID,

#   include "bbNvmScenarios.h"
    NVM_LAST_SCENARIO_ID
} NvmInstanceIds_t;
#undef PARAMETER_DECLARATION
#undef UNION_DECLARATION
#undef ARRAY_DECLARATION
#undef SCENARIO_DECLARATION

/**//**
 * \brief Request to NVM component to perform some transaction.
 */
typedef struct _NvmTransacReq_t
{
    NvmTransacType_t type;        /*!< Transaction type. Read and Write transactions are available for public. */
    NvmInstanceIds_t id;          /*!< Parameter identificator (both parameters and scenarios are available). */
    uint16_t         offset;      /*!< Offset relatively of the parameter start address (available for parameters). */
    uint16_t         length;      /*!< Stored length of the parameter in bytes (available for parameters). */
    NvmTransacCb_t   cb;          /*!< Callback about transaction is finished. */
    void            *cbParam;     /*!< Pointer to the callback argument. */
} NVM_TransacReq_t;


/**//**
 * \brief Enumerator for the BufferedTransaction status.
*/
typedef enum _Nvm_BufferedTransactionStatus
{
    NVM_BUFFERED_SUCCESS = 0,       /*!< Transaction was finished successfully. */

    NVM_BUFFERED_BUFFER_IS_IN_USE,  /*!< Transaction could not be finished, because the buffer is busy.
                                         When get this status, application can just repost a transaction or
                                         start it later. */

    NVM_BUFFERED_NO_MEMMORY         /*!< This status means that memmory allocation failed during a read procedure. */

} Nvm_BufferedTransactionStatus;

/**//**
 * \brief Struct for BufferedTransaction callback params. It contains a custom-defined callback parameters
 * (from the request) and the status of the transaction.
*/
typedef struct _NvmBufferedTransactCallbackParams_t
{
    Nvm_BufferedTransactionStatus status;
    SYS_DataPointer_t *dataPtr;
    void *customParams;
} NvmBufferedTransactCallbackParams_t;

/**//**
 * \brief Nvm buffered callback type. This function will be called after the buffered transaction will be finished.
 * To this function will be sent a custom-defined parameters (from request) and the status of the
 * transaction.
 */
typedef void (* NvmBufferedTransacCb_t)(NvmBufferedTransactCallbackParams_t *);

/**//**
 * \brief Request to NVM component to perform some buffered transaction.
 * \note Buffered transaction not needs a custom-defined static buffer to data saving - it uses
 * a fixed buffer in the it's implementation. So, it is not need to copy saving data anywhere to static buffer.
 */
typedef struct _NvmBufferedTransacReq_t
{
    NvmTransacType_t        type;      /*!< Transaction type. Read or Write. */
    NvmInstanceIds_t        id;        /*!< Parameter identificator. */
    SYS_DataPointer_t       *data;     /*!< Data pointer.
                                            For Write transaction it must contains a data to save
                                            For Read transaction it must be SYS_EMPTY_PAYLOAD. Memmory will be
                                            allocated by this service. */
    SYS_DataLength_t        offset;    /*!< Offset relatively of the parameter start address. */
    SYS_DataLength_t        length;    /*!< Stored length of the parameter in bytes. */
    NvmBufferedTransacCb_t  cb;        /*!< Callback about transaction request is finished. */
    void                   *cbParam;   /*!< Pointer to the custom callback argument. */
} NvmBufferedTransacReq_t;

/************************* PROTOTYPES **************************************************/
/************************************************************************************//**
 \brief NVM initialization routine.
 ****************************************************************************************/
void NVM_Initialization(void);

/************************************************************************************//**
 \brief Returns appropriate parameter ID or NVM_INVALID_PARAM_ID if no matches found.
 ****************************************************************************************/
NvmInstanceIds_t NVM_GetIdByPtr(void *ptr);

/************************************************************************************//**
  \brief Request to non volatile memory component to perform a transaction.

  \param[in] request - pointer to transaction.
****************************************************************************************/
void NVM_TransactionRequest(NVM_TransacReq_t *request);


/************************************************************************************//**
  \brief Request to perform the buffered volatile memmory transaction.

  \param[in] request - pointer to transaction.
****************************************************************************************/
void NVM_BufferedTransactionRequest(NvmBufferedTransacReq_t *request);

#endif /* BBNVMAPI_H_ */
