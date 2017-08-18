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
 *      MAC-FE Transactions Dispatcher interface.
 *
*******************************************************************************/

#ifndef _BB_MAC_FE_TRANS_DISP_H
#define _BB_MAC_FE_TRANS_DISP_H


/************************* INCLUDES *****************************************************/
#include "private/bbMacMpdu.h"      /* MAC MPDU definitions. */
#include "bbMacSapTypesPurge.h"     /* MCPS-PURGE service data types. */


/************************* VALIDATIONS **************************************************/
#if !defined(_MAC_CONTEXT_ZBPRO_)
# error This header shall be compiled only if the ZigBee PRO context is included into the build.
#endif


/************************* DEFINITIONS **************************************************/
/**//**
 * \brief   Searches for the given 8-bit hash value in the MAC-FE Pending Destination
 *  Addresses Hash Set.
 * \param[in]   hash    Hash value (8-bit) to be searched for in the Set.
 * \return  TRUE if the \p hash is found in the Set; FALSE otherwise.
 * \note    This macro-function is called only in the thread of an interrupt handler. No
 *  atomic section is used by this function.
 */
#define MAC_FE_TRANS_DISP_IS_HASH_IN_SET(hash)\
        BITMAP_ISSET(MAC_MEMORY_PENDING_DEST_ADDR_HASH_SET(), (hash))


/************************* INLINES ******************************************************/
/*************************************************************************************//**
 * \brief   Evaluates the 8-bit hash value of the given Address and PAN Id pair.
 * \param[in]   address     Extended (64-bit) address or short (16-bit) address extended
 *  to 64-bits with zeroes of a device.
 * \param[in]   panId       PAN Id (16-bit) of a device.
 * \return  Hash value (8-bit) of the given \p address and \p panId pair.
*****************************************************************************************/
INLINE MacAddrHash_t macFeTransDispEvaluateAddressHash(const MAC_Addr64bit_t addrLLU, const MAC_PanId_t panId)
{
    uint32_t  hash;         /* Hash value of the Address and PAN Id pair. */

    hash = ((uint32_t)(addrLLU >> 32)) ^ ((uint32_t)addrLLU) ^ ((uint32_t)panId);
    hash = (hash >> 16) ^ (uint16_t)hash;
    hash = (hash >> 8)  ^ (uint8_t)hash;

    return hash;
}


/************************* PROTOTYPES ***************************************************/
/*************************************************************************************//**
 * \brief   Creates a new transaction for the Currently Active MAC-FE Request/Response in
 *  the MAC-FE Pending Requests Queue.
 * \details
 *  This function shall be called once for new indirect request/response when it is put
 *  into the transactions queue for the first time.
 * \details
 *  This function performs the following:
 *  - appends the Currently Active MAC-FE Request/Response to the tail of the MAC-FE
 *      Pending Requests Queue,
 *  - marks the corresponding request descriptor as Pending Request to distinguish it
 *      later from new indirect requests in MCPS-DATA.request and MLME-ASSOCIATE.response
 *      processors,
 *  - evaluates and assigns the indirect transaction lifetime according to the current
 *      value of the macTransactionPersistenceTime attribute,
 *  - evaluates and assigns the destination address hash for quick searching of pending
 *      transaction in the Queue on request, and includes this hash value into the MAC-FE
 *      Pending Destination Addresses Hash Set.
 *
 * \note
 *  This function does not finish the Currently Active MAC-FE Request/Response. The MAC-FE
 *  Requests Dispatcher shall be called separately then to perform this action.
*****************************************************************************************/
MAC_PRIVATE void macFeTransDispCreateTransaction(void);


/*************************************************************************************//**
 * \brief   Processes the Data Request MAC Command received.
 * \param[in]   mpduSurr    Pointer to the MPDU Surrogate structured object of the Data
 *  Request MAC Command frame received from an associated device in the PAN.
 * \details
 *  This function searches the first pending transaction in the MAC-FE Pending Requests
 *  Queue (starting from the queue head) addressed to the requesting device. If a
 *  transaction is found, it is extracted from the MAC-FE Pending Requests Queue and
 *  assigned to the MAC-FE Requests Dispatcher to be processed with the highest priority
 *  among other requests (if it will not be processed successfully it will be then
 *  returned back to the MAC-FE Pending Requests Queue head preserving the original order
 *  of transactions addressed to the same destination device). If no transaction is found
 *  in the queue, then either nothing is done or the internal MCPS-DATA.request is
 *  assigned to the MAC-FE Requests Dispatcher with empty payload - depending on whether
 *  the requesting device (i.e., the Source Address of the Data Request MAC Command frame)
 *  is presented in the MAC-FE Pending Addresses Hash Set (i.e., if the ACK frame on the
 *  Data Request MAC Command was issued with the FramePending subfield set to one).
*****************************************************************************************/
MAC_PRIVATE void macFeTransDispDataRequest(const MacMpduSurr_t *const mpduSurr);


/*************************************************************************************//**
 * \brief   Returns existent transaction that is Currently Active from unsuccessful
 *  processing back to the MAC-FE Pending Requests Queue to be processed later.
 * \details
 *  This function shall be called in the case of unsuccessful attempt to transmit data
 *  requested by other device.
 * \details
 *  This function appends the Currently Active MAC-FE Request/Response to the head of the
 *  MAC-FE Pending Requests Queue. The previously evaluated indirect transaction lifetime
 *  is preserved by this function. This function also marks the corresponding request
 *  descriptor as Second Attempt to distinguish it later (at second and consecutive
 *  transmission attempts) from transactions transmitted for the first time - it affects
 *  evaluation of the MPDU.MHR.DSN field.
 * \details
 *  The existent transaction is returned to the transactions queue head but not tail to
 *  preserve the original order of indirect transactions addressed to the same destination
 *  device. Such a strategy obviously may brake the original order of all transactions in
 *  the queue, but it has no value because the relative order of transactions must be
 *  preserved only in relation to any given destination device but not between different
 *  destination devices.
 * \note
 *  This function does not finish the Currently Active MAC-FE Request/Response. The MAC-FE
 *  Requests Dispatcher shall be called separately then to perform this action.
*****************************************************************************************/
MAC_PRIVATE void macFeTransDispReturnTransaction(void);


/*************************************************************************************//**
 * \brief   Designates all the expired transactions for confirmation with status
 *  TRANSACTION_EXPIRED.
 * \details
 *  This function is called by the MAC-FE Task-Time Dispatcher on the EXPIRED task
 *  scheduled by the Symbol Timer that was previously appointed to the most close
 *  expiration timestamp according to lifetimes of all transactions in the MAC-FE Pending
 *  Requests Queue. All transactions in the queue with expired lifetime are designated to
 *  be confirmed with the TRANSACTION_EXPIRED status by the MAC-FE Requests Dispatcher -
 *  either the MCPS-DATA.confirm or the MLME-COMM-STATUS-ASSOCIATE.indication are issued
 *  for that according to the type of each confirmed (expired) MAC-FE Request/Response. If
 *  after that there are transactions in the MAC-FE Pending Requests Queue, a new EXPIRED
 *  task is appointed.
*****************************************************************************************/
MAC_PRIVATE void macFeTransDispAcceptExpired(void);


/*************************************************************************************//**
 * \brief   Purges all pending MCPS-DATA.request transactions matching the specified MSDU
 *  handle.
 * \param[in]   msduHandle      The handle of the MSDU to be purged from the transaction
 *  queue.
 * \return  SUCCESS if at least one transaction was purged; INVALID_HANDLE if no matches
 *  were found.
 * \details
 *  This function is called by the MCPS-PURGE.request Processor.
 * \details
 *  This function runs over the MAC-FE Pending Requests Queue and searches for
 *  transactions of the type MCPS-DATA.request with the MSDU handle value equal to the
 *  specified \p msduHandle. All matched transactions are passed to the MCPS-DATA.request
 *  Processor to be confirmed immediately with the PURGED status.
*****************************************************************************************/
MAC_PRIVATE MAC_Status_t macFeTransDispPurge(const MAC_MsduHandle_t msduHandle);


/*************************************************************************************//**
 * \brief   Terminates all pending transactions ahead of schedule.
 * \return  Pointer to the head element of the temporary queue of transactions to be
 *  reset by the caller.
 * \details
 *  This function is called by the MLME-RESET.request Processor when the ZigBee PRO
 *  context of the MAC is to be reset.
 * \details
 *  This function terminates all transactions in the MAC-FE Pending Requests Queue and
 *  returns the pointer to their queue head. Then the caller shall confirm all
 *  transactions in the queue with the RESET status.
*****************************************************************************************/
MAC_PRIVATE SYS_QueueElement_t* macFeTransDispReset(void);


#endif /* _BB_MAC_FE_TRANS_DISP_H */

/* eof bbMacFeTransDisp.h */