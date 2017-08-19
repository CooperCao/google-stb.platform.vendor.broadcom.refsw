/******************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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


#include "bdsp.h"
#include "bdsp_priv.h"

BDBG_MODULE(bdsp_priv);

BDBG_OBJECT_ID(BDSP_Device);
BDBG_OBJECT_ID(BDSP_Context);
BDBG_OBJECT_ID(BDSP_Task);
BDBG_OBJECT_ID(BDSP_Stage);
BDBG_OBJECT_ID(BDSP_InterTaskBuffer);
BDBG_OBJECT_ID(BDSP_ExternalInterrupt);
BDBG_OBJECT_ID(BDSP_Queue);

void BDSP_P_InitDevice(
    BDSP_Device *pDevice,
    void *pDeviceHandle
    )
{
    BDBG_ASSERT(NULL != pDevice);
    BDBG_ASSERT(NULL != pDeviceHandle);
    BKNI_Memset(pDevice, 0, sizeof(BDSP_Device));
    BDBG_OBJECT_SET(pDevice, BDSP_Device);
    pDevice->pDeviceHandle = pDeviceHandle;
}

void BDSP_P_InitStage(
    BDSP_Stage *pStage,
    void *pStageHandle
    )
{
    BDBG_ASSERT(NULL != pStage);
    BDBG_ASSERT(NULL != pStageHandle);
    BKNI_Memset(pStage, 0, sizeof(BDSP_Stage));
    BDBG_OBJECT_SET(pStage, BDSP_Stage);
    pStage->pStageHandle = pStageHandle;
}

void BDSP_P_InitInterTaskBuffer(
    BDSP_InterTaskBuffer *pInterTaskBuffer,
    void *pInterTaskBufferHandle
    )
{
    BDBG_ASSERT(NULL != pInterTaskBuffer);
    BDBG_ASSERT(NULL != pInterTaskBufferHandle);
    BKNI_Memset(pInterTaskBuffer, 0, sizeof(BDSP_InterTaskBuffer));
    BDBG_OBJECT_SET(pInterTaskBuffer, BDSP_InterTaskBuffer);
    pInterTaskBuffer->pInterTaskBufferHandle = pInterTaskBufferHandle;
}

void BDSP_P_InitQueue(
    BDSP_Queue *pQueue,
    void *pQueueHandle
    )
{
    BDBG_ASSERT(NULL != pQueue);
    BDBG_ASSERT(NULL != pQueueHandle);
    BKNI_Memset(pQueue, 0, sizeof(BDSP_Queue));
    BDBG_OBJECT_SET(pQueue, BDSP_Queue);
    pQueue->pQueueHandle = pQueueHandle;
}

void BDSP_P_InitContext(
    BDSP_Context *pContext,
    void *pContextHandle
    )
{
    BDBG_ASSERT(NULL != pContext);
    BDBG_ASSERT(NULL != pContextHandle);
    BKNI_Memset(pContext, 0, sizeof(BDSP_Context));
    BDBG_OBJECT_SET(pContext, BDSP_Context);
    pContext->pContextHandle = pContextHandle;
}

void BDSP_P_InitTask(
    BDSP_Task *pTask,
    void *pTaskHandle
    )
{
    BDBG_ASSERT(NULL != pTask);
    BDBG_ASSERT(NULL != pTaskHandle);
    BKNI_Memset(pTask, 0, sizeof(BDSP_Task));
    BDBG_OBJECT_SET(pTask, BDSP_Task);
    pTask->pTaskHandle = pTaskHandle;
}
