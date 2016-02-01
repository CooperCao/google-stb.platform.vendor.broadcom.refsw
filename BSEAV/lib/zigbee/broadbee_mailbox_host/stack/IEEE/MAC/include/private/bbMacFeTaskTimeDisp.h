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
* FILENAME: $Workfile: trunk/stack/IEEE/MAC/include/private/bbMacFeTaskTimeDisp.h $
*
* DESCRIPTION:
*   MAC-FE Task-Time Dispatcher interface.
*
* $Revision: 3159 $
* $Date: 2014-08-05 19:11:02Z $
*
*****************************************************************************************/


#ifndef _BB_MAC_FE_TASK_TIME_DISP_H
#define _BB_MAC_FE_TASK_TIME_DISP_H


/************************* INCLUDES *****************************************************/
#include "private/bbMacMpdu.h"      /* MAC MPDU definitions. */


/************************* PROTOTYPES ***************************************************/
/*************************************************************************************//**
 * \brief   Schedules the START task for activating next MAC-FE Request Processor.
 * \details
 *  When called schedules the START task that will instigate the MAC-FE Task-Time
 *  Dispatcher to assign the next MAC-FE Request/Response from the MAC-FE Main Requests
 *  Queue to become the Currently Active Request/Response and then issue the START event
 *  to this newly activated Request/Response Processor.
 * \details
 *  This task is used by the MAC-FE Requests Dispatcher to split the context of either (a)
 *  the new request/response originator (NWK layer) and corresponding Request/Response
 *  Processor (MAC layer), or (b) the previous request/response has just finished and the
 *  next request/response from the MAC-FE Main Requests Queue being started. This method
 *  is used by the MAC-FE Requests Dispatcher only.
 * \note
 *  After the MAC-FE Main Requests Queue is emptied during RESET, the START event task is
 *  not to be dismissed even if the queue is finally empty - the corresponding task
 *  handler will ignore the task if the queue is empty at the moment. The reason is that
 *  the Queue is emptied only for the specified MAC Context from two (for the case of
 *  dual-context MAC).
*****************************************************************************************/
MAC_PRIVATE void macFeTaskTimeDispScheduleStart(void);


/*************************************************************************************//**
 * \brief   Requests the MAC-LE to perform an MPDU transmission.
 * \param[in]   mpduConstructor     Entry point to MPDU Surrogate Constructor function.
 * \details
 *  Sets the specified MPDU Surrogate Constructor function entry point and issues the
 *  MAC-LE-START-TX.request for the new data frame transmission. The function specified
 *  with the \p mpduConstructor will be called later; it shall construct the MPDU
 *  Surrogate structured object in the memory pointed by its argument \c mpduSurr.
*****************************************************************************************/
MAC_PRIVATE void macFeTaskTimeDispStartTxReq(MacMpduConstructor_t *mpduConstructor);


#if !defined(_MAC_CONTEXT_RF4CE_CONTROLLER_)
/*************************************************************************************//**
 * \brief   Requests the MAC-LE to perform an ED Scan.
 * \param[in]   duration    Duration of the ED Scan to be performed, in symbol quotients.
 * \details
 *  Issues the MAC-LE-START-ED.request to start the ED Scan. The \p duration argument
 *  specifies the overhaul duration of scanning.
*****************************************************************************************/
MAC_PRIVATE void macFeTaskTimeDispStartEdReq(const HAL_SymbolTimeshift_t duration);
#endif


/*************************************************************************************//**
 * \brief   Requests the MAC-LE to perform an ED Scan.
 * \param[in]   channelOnPage   The 16-bit plain value of the Channel-on-Page object
 *  containing the Channel Page IEEE-index in its MSB and the Logical Channel index in its
 *  LSB to be switched to.
 * \details
 *  Issues the MAC-LE-SET-CHANNEL.request to switch the current channel and channel page
 *  as specified by the \p channelOnPage argument.
*****************************************************************************************/
MAC_PRIVATE void macFeTaskTimeDispSetChannelReq(const PHY_ChannelOnPagePlain_t channelOnPage);


#if !defined(_MAC_CONTEXT_RF4CE_CONTROLLER_)
/*************************************************************************************//**
 * \brief   Appoints the TIMEOUT task for the Currently Active MAC-FE Request Processor.
 * \param[in]   period      Duration of period to schedule the task, in whole symbols.
 * \details
 *  When called appoints the timed TIMEOUT task that will issue TIMEOUT event to the FSM
 *  of the Currently Active MAC-FE Request Processor. This task is used by the Currently
 *  Active MAC-FE Request Processor to limit period of waiting for some external condition
 *  or to perform specified unconditional delay.
 * \note
 *  Before finishing the Currently Active MAC-FE Request/Response (or its particular
 *  states) under some other condition but not the timeout, the previously appointed
 *  TIMEOUT task must be recalled.
*****************************************************************************************/
MAC_PRIVATE void macFeTaskTimeDispAppointTimeout(const HAL_SymbolPeriod_t period);


/*************************************************************************************//**
 * \brief   Recalls appointment of the TIMEOUT task for the Currently Active MAC-FE
 *  Request Processor.
 * \details
 *  When called recalls the previously appointed TIMEOUT task. If the due-time of the
 *  appointed task has already passed and the task was scheduled for execution but still
 *  not selected for execution from the tasks queue, then such a scheduled task is
 *  dismissed also.
*****************************************************************************************/
MAC_PRIVATE void macFeTaskTimeDispRecallTimeout(void);
#endif /* ! _MAC_CONTEXT_RF4CE_CONTROLLER_ */


#if defined(_MAC_CONTEXT_ZBPRO_)
/*************************************************************************************//**
 * \brief   Appoints or reappoints the EXPIRED task for the MAC-FE Transactions
 *  Dispatcher.
 * \param[in]   timeshift   Timeshift of period to schedule the task, in symbol quotients.
 * \details
 *  When called appoints the timed EXPIRED task that will instigate the MAC-FE
 *  Transactions Dispatcher to parse the MAC-FE Pending Transactions Queue and confirm
 *  expired transactions.
 * \details
 *  This method is to be called by the MAC-FE Transactions Dispatcher to appoint or
 *  reappoint delivery of the EXPIRED signal to itself on the closest expiration moment
 *  from all the pending transactions.
 * \details
 *  If the EXPIRED task was already appointed by the previous call (for the previously
 *  assigned pending transaction) and it has not triggered yet, the appointment will be
 *  reappointed to the new timestamp, according to the \p timeshift, if the new expiration
 *  moment is closer to the current moment than the previously appointed.
*****************************************************************************************/
MAC_PRIVATE void macFeTaskTimeDispReappointExpired(const HAL_SymbolTimeshift_t timeshift);


/*************************************************************************************//**
 * \brief   Recalls appointment of the EXPIRED task for the MAC-FE Transactions
 *  Dispatcher.
 * \details
 *  When called recalls the previously appointed EXPIRED task. If the due-time of the
 *  appointed task has already passed and the task was scheduled for execution but still
 *  not selected for execution from the tasks queue, then such a scheduled task is
 *  dismissed also.
*****************************************************************************************/
MAC_PRIVATE void macFeTaskTimeDispRecallExpired(void);
#endif /* _MAC_CONTEXT_ZBPRO_ */


#endif /* _BB_MAC_FE_TASK_TIME_DISP_H */