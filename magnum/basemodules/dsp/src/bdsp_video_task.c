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

#include "bdsp.h"
#include "bdsp_priv.h"

 BERR_Code BDSP_Video_GetPictureCount_isr(
     BDSP_TaskHandle task,
     unsigned *pPictureCount
     )
{
    BDBG_OBJECT_ASSERT(task, BDSP_Task);
    if ( task->getPictureCount_isr)
    {
        return task->getPictureCount_isr(task->pTaskHandle,pPictureCount);
    }
    else
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

BERR_Code BDSP_Video_PeekAtPicture_isr(
    BDSP_TaskHandle task,
    unsigned index,
    dramaddr_t **pUnifiedPicture
    )
{
    BDBG_OBJECT_ASSERT(task, BDSP_Task);
    if ( task->peekAtPicture_isr)
    {
        return task->peekAtPicture_isr(task->pTaskHandle,index,pUnifiedPicture);
    }
    else
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

BERR_Code BDSP_Video_GetNextPicture_isr(
    BDSP_TaskHandle task,
    uint32_t **pUnifiedPicture
    )
{
    BDBG_OBJECT_ASSERT(task, BDSP_Task);
    if ( task->getNextPicture_isr)
    {
        return task->getNextPicture_isr(task->pTaskHandle,pUnifiedPicture);
    }
    else
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

BERR_Code BDSP_Video_ReleasePicture_isr(
    BDSP_TaskHandle task,
    uint32_t *pUnifiedPicture
    )
{
    BDBG_OBJECT_ASSERT(task, BDSP_Task);
    if ( task->releasePicture_isr)
    {
        return task->releasePicture_isr(task->pTaskHandle,pUnifiedPicture);
    }
    else
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

 BERR_Code BDSP_Video_GetPictureDropPendingCount_isr(
     BDSP_TaskHandle task,
     unsigned *pPictureDropPendingCount
     )
{
    BDBG_OBJECT_ASSERT(task, BDSP_Task);
    if ( task->getPictureDropPendingCount_isr)
    {
        return task->getPictureDropPendingCount_isr(task->pTaskHandle,pPictureDropPendingCount);
    }
    else
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

 BERR_Code BDSP_Video_RequestPictureDrop_isr(
     BDSP_TaskHandle task,
     unsigned *pPictureDropRequestCount
     )
{
    BDBG_OBJECT_ASSERT(task, BDSP_Task);
    if ( task->requestPictureDrop_isr)
    {
        return task->requestPictureDrop_isr(task->pTaskHandle,pPictureDropRequestCount);
    }
    else
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

 BERR_Code BDSP_Video_DisplayInterruptEvent_isr(
     BDSP_TaskHandle task
     )
{
    BDBG_OBJECT_ASSERT(task, BDSP_Task);
    if ( task->displayInterruptEvent_isr)
    {
        return task->displayInterruptEvent_isr(task->pTaskHandle);
    }
    else
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}
