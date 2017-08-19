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

/******************************************************************************
 *
 * DESCRIPTION:
 *      This is the private header file for the RF4CE ZRC profile Control Command Handler handler.
 *
*******************************************************************************/

#ifndef _RF4CE_ZRC_PRIVATE_CONTROL_COMMAND_H
#define _RF4CE_ZRC_PRIVATE_CONTROL_COMMAND_H

/************************* INCLUDES ****************************************************/
#include "bbRF4CEPM.h"

/************************* DEFINITIONS *************************************************/

/**//**
 * \brief RF4CE ZRC 2.0 Action Command ID.
 */
#define RF4CE_ZRC2_ACTION_COMMAND_ID 0x06

/************************* TYPES *******************************************************/
/**//**
 * \brief RF4CE ZRC 2.0 Action command Request Processing state
 */
typedef enum  _rf4ceZrc2ControlCommandState_t
{
    RF4CE_ZRC2_CONTROLCOMMAND_IDLE,
    RF4CE_ZRC2_CONTROLCOMMAND_PRESS,
    RF4CE_ZRC2_CONTROLCOMMAND_RELEASE,
    RF4CE_ZRC2_CONTROLCOMMAND_REPEAT
} rf4ceZrc2ControlCommandState_t;

/************************* FUNCTIONS PROTOTYPES ****************************************/

/************************************************************************************//**
 \brief Processes the input ZRC 1.1 Control Command request packet.

 \param[in] indication - pointer to the indication structure.
 ****************************************************************************************/
void RF4CE_ZRC1_ControlCommandDataIndication(RF4CE_NWK_DataIndParams_t *indication);

/************************************************************************************//**
 \brief Control Command ZRC 2.0 data indication.

 \param[in] indication - pointer to the indication data structure.
 \param[in] length - the length in bytes of the incoming payload.
 \param[in] leaveReceiverOn - true if necessary to leave receiver on.
 \return Nothing.
 ****************************************************************************************/
void rf4ceZrc2ControlCommandInd(RF4CE_NWK_DataIndParams_t *indication, uint32_t length, bool leaveReceiverOn);

/************************************************************************************//**
 \brief ZRC2 Control Command Pressed/Released task handler.

 \param[in] queueElement - pointer to the queue element structure.
****************************************************************************************/
void rf4ceZrc2ControlCommandHandler(SYS_QueueElement_t *queueElement);

/************************************************************************************//**
 \brief ZRC2 Control Command Mapping task handler

 \param[in] queueElement - pointer to the queue element structure
****************************************************************************************/
void rf4ceZrc2ControlCommandMapHandler(SYS_QueueElement_t *queueElement);

/************************************************************************************//**
 \brief ZRC2 Control Command Send Action Frame task handler.

 \param[in] queueElement - pointer to the queue element structure.
****************************************************************************************/
void rf4ceZrc2ControlCommandTxHandler(SYS_QueueElement_t *queueElement);

#endif /* _RF4CE_ZRC_PRIVATE_CONTROL_COMMAND_H */

/* eof bbRF4CEZRCPrivateControlCommand.h */