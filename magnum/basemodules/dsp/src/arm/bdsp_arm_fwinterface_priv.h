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

#ifndef BDSP_ARM_FWINTERFACE_PRIV_H__
#define BDSP_ARM_FWINTERFACE_PRIV_H__

#include "bdsp_arm_priv.h"

BERR_Code BDSP_Arm_P_AssignFreeInterfaceQueueHandle(
    BDSP_Arm *pDevice,
    int32_t* i32QueueHandleIndex);

BERR_Code BDSP_Arm_P_ReleaseInterfaceQueueHandle(
    BDSP_Arm    *pDevice,
    int32_t i32QueueHandleIndex);

BERR_Code BDSP_Arm_P_InitMsgQueue(
    BDSP_Arm *pDevice,
    BDSP_Arm_P_MsgQueueHandle    hMsgQueue
    );

BERR_Code BDSP_Arm_P_InvalidateMsgQueue(
    BDSP_Arm *pDevice,
    BDSP_Arm_P_MsgQueueHandle    hMsgQueue   /* [in]*/
    );

BERR_Code BDSP_Arm_P_CreateMsgQueue(
    BDSP_Arm *pDevice,
    BDSP_Arm_P_MsgQueueParams    *psMsgQueueParams   /* [in]*/,
    BDSP_Arm_P_MsgQueueHandle    *hMsgQueue
    );

BERR_Code BDSP_Arm_P_DestroyMsgQueue(
    BDSP_Arm *pDevice,
    BDSP_Arm_P_MsgQueueHandle    hMsgQueue
    );

BERR_Code BDSP_Arm_P_SendCommand(
    BDSP_Arm_P_MsgQueueHandle    hMsgQueue   /*[in]*/,
    const BDSP_Arm_P_Command     *psCommand  /*[in]*/ ,
    void *pTaskHandle      /*[in] Task handle */
    );

BERR_Code BDSP_Arm_P_SendCommand_isr(
    BDSP_Arm_P_MsgQueueHandle    hMsgQueue   /*[in]*/,
    const BDSP_Arm_P_Command         *psCommand  /*[in]*/,
    void    *pTaskHandle       /*[in] Task handle */);

BERR_Code BDSP_Arm_P_GetAsyncMsg_isr(
    BDSP_Arm_P_MsgQueueHandle    hMsgQueue,  /*[in]*/
    void                        *pMsgBuf,   /*[in]*/
    unsigned int                *puiNumMsgs /*[out]*/
    );

BERR_Code BDSP_Arm_P_GetMsg_isr(
    BDSP_Arm_P_MsgQueueHandle    hMsgQueue,  /*[in]*/
    void                         *pMsgBuf,/*[in]*/
    BDSP_P_MsgType           eMgsType
    );

/***************************************************************************
Summary:
    Gets a message from the message queue and writes in into the message buffer

Description:
    Sanity check is done to check if the read and write pointers haven't been corrupted in the shared copy.
    Checks if a message is present. If no message is there in the message queue BUFFER_EMPTY error is returned
    MESSAGE_SIZE/4 number of words are read from the msg buffer into the message queue
    Read Pointers are updated both in the shared and the local copy

Returns:
    BERR_SUCCESS else error

**************************************************************************/
BERR_Code BDSP_Arm_P_GetMsg(
    BDSP_Arm_P_MsgQueueHandle  hMsgQueue,
    void *pMsgBuf,
    BDSP_P_MsgType eMgsType
    );

BERR_Code BDSP_Arm_P_GetAlgorithmSettings(
    BDSP_Algorithm          eAlgorithm,
    BDSP_MMA_Memory        *pMemory,
    uint32_t                ui32ConfigBufSize,    /* [in] Config Buf Size */
    void                   *pSettingsBuffer,
    size_t                  settingsBufferSize
    );

BERR_Code BDSP_Arm_P_SetAlgorithmSettings(
    BDSP_Algorithm          eAlgorithm,
    BDSP_MMA_Memory        *pMemory,
    uint32_t                ui32ConfigBufSize,    /* [in] Config Buf Size */
    const void             *pSettingsBuffer,
    size_t                  settingsBufferSize
    );

BERR_Code BDSP_Arm_P_GetFrameSyncTsmStageConfigParams_isr(
	BDSP_Algorithm eAlgorithm,
	BDSP_MMA_Memory *pConfigBuf,
	uint32_t        ui32ConfigBufSize,     /* [in] Config Buf Size */
    void           *pSettingsBuffer,
    size_t          settingsBufferSize
    );

BERR_Code BDSP_Arm_P_SetFrameSyncTsmStageConfigParams_isr(
	BDSP_Algorithm		eAlgorithm,
	BDSP_MMA_Memory *pConfigBuf,
	uint32_t            uiConfigBufSize,     /* [in] Config Buf Size */
    const void         *pSettingsBuffer,
    size_t              settingsBufferSize
    );

#endif /* BDSP_ARM_FWINTERFACE_PRIV_H__*/
