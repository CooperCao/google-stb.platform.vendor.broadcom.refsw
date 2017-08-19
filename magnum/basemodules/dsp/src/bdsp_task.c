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

BDBG_MODULE(bdsp_task);

void BDSP_Task_GetDefaultCreateSettings(
    BDSP_ContextHandle context,
    BDSP_TaskCreateSettings *pSettings     /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(context, BDSP_Context);
    BDBG_ASSERT(NULL != pSettings);

    if ( context->getDefaultTaskSettings )
    {
        context->getDefaultTaskSettings(context->pContextHandle, pSettings);
    }
    else
    {
        BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    }
}

BERR_Code BDSP_Task_Create(
    BDSP_ContextHandle context,
    const BDSP_TaskCreateSettings *pSettings,
    BDSP_TaskHandle *pTask    /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(context, BDSP_Context);
    BDBG_ASSERT(NULL != pSettings);
    BDBG_ASSERT(NULL != pTask);

    if ( context->createTask )
    {
        return context->createTask(context->pContextHandle, pSettings, pTask);
    }
    else
    {
        *pTask = NULL;
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

void BDSP_Task_Destroy(
    BDSP_TaskHandle task
    )
{
    BDBG_OBJECT_ASSERT(task, BDSP_Task);
    BDBG_ASSERT(NULL != task->destroy);
    task->destroy(task->pTaskHandle);
}

BERR_Code BDSP_Task_Start(
    BDSP_TaskHandle task,
    BDSP_TaskStartSettings *pSettings    /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(task, BDSP_Task);
    if ( task->start )
    {
        return task->start(task->pTaskHandle,pSettings);
    }
    else
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

BERR_Code BDSP_Task_Stop(
    BDSP_TaskHandle task
    )
{
    BDBG_OBJECT_ASSERT(task, BDSP_Task);
    if ( task->stop )
    {
        return task->stop(task->pTaskHandle);
    }
    else
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

void BDSP_Task_GetDefaultStartSettings(
    BDSP_TaskHandle task,
    BDSP_TaskStartSettings *pSettings    /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(task, BDSP_Task);
    BDBG_ASSERT(NULL != pSettings);

    if ( task->getDefaultTaskStartSettings )
    {
        task->getDefaultTaskStartSettings(task->pTaskHandle, pSettings);
    }
    else
    {
        BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    }
}

BERR_Code BDSP_Task_RetrieveGateOpenSettings(
    BDSP_TaskHandle task,
    BDSP_TaskGateOpenSettings *pSettings   /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(task, BDSP_Task);
    if ( task->retreiveGateOpenSettings )
    {
        return task->retreiveGateOpenSettings(task->pTaskHandle, pSettings);
    }
    else
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}
