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
 *      This is the private header file for the RF4CE ZRC profile commands handler.
 *
*******************************************************************************/

#ifndef _RF4CE_ZRC_COMMANDS_H
#define _RF4CE_ZRC_COMMANDS_H

/************************* INCLUDES ****************************************************/
#include "bbRF4CEPM.h"
#include "bbSysMemMan.h"

/************************* DEFINITIONS *************************************************/
/**//**
 * \brief ZRC 1.1 command codes.
 */
typedef enum _RF4CE_ZRC1_Commands_t
{
    RF4CE_ZRC1_USER_CONTROL_PRESSED = 0x01,
    RF4CE_ZRC1_USER_CONTROL_REPEATED = 0x02,
    RF4CE_ZRC1_USER_CONTROL_RELEASED = 0x03,
    RF4CE_ZRC1_COMMAND_DISCOVERY_REQUEST = 0x04,
    RF4CE_ZRC1_COMMAND_DISCOVERY_RESPONSE = 0x05
} RF4CE_ZRC1_Commands_t;

/**//**
 * \brief GDP 2.0 command codes.
 */
typedef enum _RF4CE_GDP_Commands_t
{
    RF4CE_GDP_GENERIC_RESPONSE = 0x00,
    RF4CE_GDP_CONFIGURATION_COMPLETE = 0x01,
    RF4CE_GDP_HEARTBEAT = 0x02,
    RF4CE_GDP_GET_ATTRIBUTES = 0x03,
    RF4CE_GDP_GET_ATTRIBUTES_RESPONSE = 0x04,
    RF4CE_GDP_PUSH_ATTRIBUTES = 0x05,
    RF4CE_GDP_SET_ATTRIBUTES = 0x06,
    RF4CE_GDP_PULL_ATTRIBUTES = 0x07,
    RF4CE_GDP_PULL_ATTRIBUTES_RESPONSE = 0x08,
    RF4CE_GDP_CHECK_VALIDATION = 0x09,
    RF4CE_GDP_CLIENT_NOTIFICATION = 0x0A,
    RF4CE_GDP_KEY_EXCHANGE = 0x0B
} RF4CE_GDP_Commands_t;

/************************* TYPES *******************************************************/
/************************************************************************************//**
 \brief ZRC2.0 struct for the incomming indication.
 ****************************************************************************************/
typedef struct _RF4CE_ZRC_DataIncommingInd_t
{
    SYS_QueueElement_t          queueElement;   /* Service field. */
    RF4CE_NWK_DataIndParams_t   indication;     /* Copy of the incomming indication. */
} RF4CE_ZRC_DataIncommingInd_t;

/**//**
 * \brief The Frame Header
 */
typedef uint8_t RF4CE_GDP_FrameHeader_t;

/************************* FUNCTIONS PROTOTYPES ****************************************/
/************************************************************************************//**
 \brief Processes the input packet containing command ZRC v1.1, parses it and calls corresponding function.

 \param[in] indication - pointer to the incoming indication structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_ZRC1_Data(RF4CE_NWK_DataIndParams_t *indication);

/************************************************************************************//**
 \brief Processes the input packet containing command ZRC v2.0, parses it and calls corresponding function.

 \param[in] indication - pointer to the incoming indication structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_ZRC2_Data(RF4CE_NWK_DataIndParams_t *indication);

/************************************************************************************//**
  \brief ZRC task handler for incomming indications.
  \param[in] taskDescriptor - pointer to the task descriptor.
****************************************************************************************/
void rf4cezrc2DataIndicationHandler(SYS_SchedulerTaskDescriptor_t *const taskDescriptor);

/************************************************************************************//**
  \brief ZRC function for the indications which are not processed and need to be reposted.
****************************************************************************************/
void rf4cezrc2DataIndicationRepost(void);
/************************************************************************************//**
  \brief ZRC function for the indications which are processed. This function removes
  current indication from the queue and start processing of the next.
****************************************************************************************/
void rf4cezrc2DataIndicationFinishProcessing(void);

/************************************************************************************//**
 \brief Common ZRC data confirmation callback.

 \param[in] req - pointer to the original request structure.
 \param[in] conf - pointer to the confirmation structure.
 \return Nothing.
 ****************************************************************************************/
void rf4cezrc2DataCommonDataRequestConfirm(RF4CE_NWK_DataReqDescr_t *req, RF4CE_NWK_DataConfParams_t *conf);

#endif /* _RF4CE_ZRC_COMMANDS_H */

/* eof bbRF4CEZRCCommands.h */