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
* FILENAME: $Workfile: branches/akhromykh/BEE554_NVM_Implementation/common/Security/include/bbNvmMm.h $
*
* DESCRIPTION:
*   Memory menegment for NVM.
*
* $Revision: 1832 $
* $Date: 2014-03-19 07:10:11Z $
*
*****************************************************************************************/

#ifndef BBNVMMM_H_
#define BBNVMMM_H_

/************************* INCLUDES *****************************************************/
#include "bbSysTaskScheduler.h"
#include "bbSysNvmManager.h"
#include "private\bbNvmDescription.h"

/************************* TYPES *******************************************************/
/**//**
 * \brief Type describes the common NVM memory.
 */
typedef struct _NvmMem_t
{
    SYS_SchedulerTaskDescriptor_t  tasks;
    SYS_QueueDescriptor_t          reqQueue;
    union
    {
        NVM_OpenFileIndDescr_t openBoxReq;
        NVM_CloseFileIndDescr_t closeBoxReq;
        NVM_ReadFileIndDescr_t  readBoxReq;
        NVM_WriteFileIndDescr_t writeBoxReq;
    };
    Bool8_t requestExists;
    struct
    {
        NvmInstanceIds_t id;
        NvmTransacType_t type;
        uint16_t offset;
        uint16_t length;
        uint16_t transferred;
    } requestParams;
} NvmMem_t;

/**//**
 * \brief It is a BufferedTransaction buffer declaration.
 * \note It is extern because of the using in the NvmParameters.h
*/
extern Nvm_BufferedTransactBufferDescr_t nvmBufferedTransactionBufferDescr;

#define GetBufferedTransactionBufferDescr() (&nvmBufferedTransactionBufferDescr)
#define GetBufferedTransactionQueue() (&GetBufferedTransactionBufferDescr()->queue)
#define GetBufferedTransactionBuffer() (GetBufferedTransactionBufferDescr()->rawData)

/************************* FUNCTIONS PROTOTYPES ****************************************/
/************************************************************************************//**
 \brief Returns a pointer to the common NVM memory
 ****************************************************************************************/
NvmMem_t *nvmMemGet(void);

/************************************************************************************//**
 \brief Returns a pointer to non volatile data descriptors
 ****************************************************************************************/
const NvmDescr_t *nvmDescrGet(void);

/************************************************************************************//**
 \brief Returns a scenario size by Id
 ****************************************************************************************/
const uint8_t nvmGetScenarioSizeById(uint8_t id);

#endif /* BBNVMMM_H_ */
