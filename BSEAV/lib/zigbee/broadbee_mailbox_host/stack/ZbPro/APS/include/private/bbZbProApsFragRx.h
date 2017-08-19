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

/****************************************************************************************
 *
 * DESCRIPTION:
 *      Declaration of the ZigBee PRO APS Frag Rx component
 *
 ****************************************************************************************/

#ifndef _ZBPRO_APS_FRAG_RX_H
#define _ZBPRO_APS_FRAG_RX_H

/************************* INCLUDES ***********************************************************************************/
#include "bbZbProAps.h"
#include "bbZbProApsRx.h"
#include "bbSysTimeoutTask.h"

/************************* DEFINITIONS ********************************************************************************/
/**//**
 * \brief   Data type for structure containing the Fragmentation RX unit extended memory.
 * \details This data structure contains all the information related to the fragmented transaction being currently
 *  received.
 * \details The embedded RX Buffer object is used for holding all the necessary parameters of the received transaction,
 *  holding the reassembled ASDU payload, and issuing an APSDE-DATA.indication to the next higher-level layer.
 * \details In order not to blow up the data structure, some parameters related to the whole transaction being received
 *  are stored in corresponding fields of the embedded RX Buffer object during the whole session. They are:
 *  - rxBuf.isFragmented - TRUE if the structure is currently busy with a transaction being received. Initialized with
 *      FALSE, and set back to FALSE when the whole transaction is successfully received (or aborted) - which means that
 *      a new fragmented reception may be started.
 *  - rxBuf.frameBuffer.payload - keep the reassembled ASDU payload. All the successfully received fragments are linked
 *      sequentially according to their Block Id values. When a new fragment is received it is inserted into the payload
 *      according to its personal Block Id between the two previously received fragments (or appended to the head or the
 *      tail of the payload).
 *  - rxBuf.fragsNum - keeps the total number of fragments in the current transaction. This field takes values in the
 *      range 1..255; it is initialized with 0 and stays equal to 0 until the very first fragment (which specifies the
 *      total number of fragments in the transaction) is received. Value 0 means that the total number of fragments is
 *      currently unknown (the first fragment was not received yet).
 *  - rxBuf.fragIdx - keeps the index of the first fragment in the current reception window. Initially assigned with 0,
 *      which means that the first window is being received, and then incremented in steps by the number of fragments in
 *      a single window (the window size).
 *
 * \details The following data is stored in dedicated fields:
 *  - fragSizes - array of individual fragment sizes within the current reception window, in bytes, up to 8 elements.
 *      Initially and each time the window propagates, this array is initialized with all 0. When a new fragment is
 *      received for the first time, its actual size is saved in this array. This array is used for calculating the
 *      local offset of the insertion point for each newly received fragment within the current window.
 *  - windowOffset - offset of the beginning of the current reception window within the original ASDU payload, in bytes.
 *      It equals to the sum of individual sizes of all previously received fragments in all previous reception window
 *      positions. Notice that fragments received in the current window are not included into this sum.
 *  - windowSize - size of the reception window, in fragments, in the range from 1 to 8. This parameter is initialized
 *      at the beginning of reception of a new transaction according to the Dst. Endpoint specified in the APS frame
 *      header.
 *  - fragsMask - the 8-bit bitmap of fragments already received in the current window. Only the lover N bits - bits
 *      #0..(N-1) - are valid, where N is the number of fragments in a single window (the window size) from 1 to 8. If N
 *      is lower than 8, then all the higher bits - bits #N..7 - are forced to 1.
 */
typedef struct _ZbProApsFragRxDesc_t {
    ZbProApsRxBuffer_t  rxBuf;                                      /*!< The embedded RX Buffer. */
    uint8_t             fragSizes[ZBPRO_APS_MAX_MAX_WINDOW_SIZE];   /*!< Fragment sizes within the window, in bytes. */
    uint16_t            windowOffset;                               /*!< Offset of the whole window, in bytes. */
    uint8_t             windowSize;                                 /*!< Size of the reception window, in fragments. */
    BitField8_t         fragsMask;                                  /*!< Bitmap of received fragments in the window. */
    SYS_TimeoutTask_t   timer;                                      /*!< Fragmentation timeout timer. */
} ZbProApsFragRxDesc_t;

/************************* PROTOTYPES *********************************************************************************/
/**//**
 * \brief   Stops and initializes APS Fragmentation RX unit.
 * \details This function is to be called at arbitrary moment when it's necessary to stop the APS Fragmentation RX unit
 *  (to stop the whole APS, for example) and initialize it.
 * \note    When this function is called at the application start up the APS Fragmentation RX unit descriptor shall be
 *  already assigned with all zeroes in order this function worked properly.
 */
APS_PRIVATE void zbProApsFragRxReset(void);

/*--------------------------------------------------------------------------------------------------------------------*/
/**//**
 * \brief   Processes the received APS frame.
 * \param[in/out]   pRxBuf      Pointer to the RX Buffer linked to the received frame.
 * \details This function is called by the APS RX FSM each time a new valid or semi-valid APS Data or APS Command frame
 *  addressed to this node is received, both fragmented or nonfragmented. A frame is considered 'valid' here if its
 *  header was parsed successfully and, for secured frames, if the frame was successfully unsecured. A secured frame is
 *  considered 'semi-valid' if its header was parsed successfully, but the frame was not unsecured for some reason. This
 *  function is not called for APS ACK frames and other APS broken frames. No duplication filter shall be imposed prior
 *  to call this function.
 * \details This function performs further processing of the frame. For nonfragmented frames, the processing is trivial:
 *  such Data frames are simply indicated to the next higher-level layer with APSDE-DATA.indication, and Command frames
 *  are delivered to their corresponding internal APS services; then ACK frame generation may be started when necessary.
 *  For fragmented Data frames, this function performs the full set of algorithms of reassembling the original APSU
 *  payload, ACKing the fragment windows, etc.; finally the received APSU payload is indicated to the next higher-level
 *  layer with APSDE-DATA.indication. Also this function arranges the whole set of operations for duplication filtering
 *  feature (however it does not perform them itself).
 */
APS_PRIVATE void zbProApsFragRxProcess(ZbProApsRxBuffer_t *pRxBuf);

/*--------------------------------------------------------------------------------------------------------------------*/
/**//**
 * \brief   Handles the window-propagation timeout event.
 * \param[in]   taskDescriptor      Pointer to the task descriptor linked to the event.
 */
APS_PRIVATE void zbProApsFragRxTimeoutHandler(SYS_SchedulerTaskDescriptor_t *const taskDescriptor);

/*--------------------------------------------------------------------------------------------------------------------*/
/**//**
 * \brief   Handles the last-window-ack-retry timeout event.
 * \param[in]   taskDescriptor      Pointer to the task descriptor linked to the event.
 */
APS_PRIVATE void zbProApsFragRxPersistHandler(SYS_SchedulerTaskDescriptor_t *const taskDescriptor);

#endif /* _ZBPRO_APS_FRAG_RX_H */
