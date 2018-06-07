/******************************************************************************
 * Copyright (C) 2018 Broadcom.
 * The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to
 * the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied),
 * right to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 * THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 * IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 * THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 * OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
*****************************************************************************/

#include "bdsp_common_priv_include.h"

BDBG_MODULE(bdsp_common_fwinterface);

BERR_Code BDSP_P_CreateMsgQueue(
    BDSP_P_MsgQueueParams          *psMsgQueueParams,    /* [in]*/
    BREG_Handle                     hRegister,           /* [in] */
    uint32_t                        ui32DspOffset,       /* [in] */
    BDSP_P_MsgQueueHandle          *hMsgQueue
    )
{
    BERR_Code errCode = BERR_SUCCESS;
    BDSP_P_MsgQueueHandle  hHandle = NULL;

    BDBG_ENTER(BDSP_P_CreateMsgQueue);
    BDBG_ASSERT(hRegister);
    BDBG_ASSERT(psMsgQueueParams);
    BDBG_ASSERT(psMsgQueueParams->ui32FifoId != BDSP_FIFO_INVALID);

    BDBG_MSG(("CREATING MSGQUEUE - Base Address %p, Size %u, FifoId %d",
        psMsgQueueParams->Memory.pAddr,
        psMsgQueueParams->ui32Size,
        psMsgQueueParams->ui32FifoId));

    hHandle = (BDSP_P_MsgQueueHandle)BKNI_Malloc(sizeof(BDSP_P_MsgQueue));
    if(hHandle == NULL)
    {
        errCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        BDBG_ERR(("BDSP_P_CreateMsgQueue: Error is allocating Kernal Memory for Message Queue"));
        goto end;
    }
    BKNI_Memset (hHandle, 0, sizeof(struct BDSP_P_MsgQueue));
    hHandle->Memory        = psMsgQueueParams->Memory;
    hHandle->ui32FifoId    = psMsgQueueParams->ui32FifoId;
    hHandle->ui32Size      = psMsgQueueParams->ui32Size;
    hHandle->ui32DspOffset = ui32DspOffset;
    hHandle->hRegister     = hRegister;

    *hMsgQueue = hHandle;
    /* Address will be initilaised and FIFO registers will be initialised in the BDSP_Raaga_P_InitMsgQueue */
end:
    BDBG_LEAVE(BDSP_P_CreateMsgQueue);
    return errCode;
}

BERR_Code BDSP_P_DestroyMsgQueue(
    BDSP_P_MsgQueueHandle    hMsgQueue
)
{
    BERR_Code   errCode = BERR_SUCCESS;

    BDBG_ENTER(BDSP_P_DestroyMsgQueue);
    BDBG_ASSERT(hMsgQueue);
    BDBG_MSG(("Destroying MSGQUEUE - FifoId %d",hMsgQueue->ui32FifoId));

    BKNI_Free(hMsgQueue);
    BDBG_LEAVE(BDSP_P_DestroyMsgQueue);
    return errCode;
}
