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

#ifndef BDSP_RAAGA_FWINTERFACE_PRIV_H_
#define BDSP_RAAGA_FWINTERFACE_PRIV_H_

#include "bdsp_raaga_priv_include.h"

#define BDSP_RAAGA_P_FIFO_BASE_OFFSET    0
#define BDSP_RAAGA_P_FIFO_END_OFFSET     (BCHP_RAAGA_DSP_FW_CFG_FIFO_0_END_ADDR   -BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR)
#define BDSP_RAAGA_P_FIFO_READ_OFFSET    (BCHP_RAAGA_DSP_FW_CFG_FIFO_0_READ_ADDR  -BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR)
#define BDSP_RAAGA_P_FIFO_WRITE_OFFSET   (BCHP_RAAGA_DSP_FW_CFG_FIFO_0_WRITE_ADDR -BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR)

typedef struct BDSP_Raaga_P_MsgQueueParams
{
    BDSP_MMA_Memory Memory;
    uint32_t ui32Size;
    uint32_t ui32FifoId;  /* Fifo Id for this message queue */
} BDSP_Raaga_P_MsgQueueParams;

typedef struct BDSP_Raaga_P_MsgQueue
{
    BREG_Handle hRegister;  /* Register Handle */
    uint32_t ui32FifoId;    /* Fifo Id for this message queue */
    uint32_t ui32DspOffset; /* DSP Register Offset */
    uint32_t ui32Size;      /* Size of the Buffer */
    BDSP_MMA_Memory Memory; /*MMA descriptor for the Buffer*/
    BDSP_P_BufferPointer Address; /*Structure describing the pointers of the Actual Buffer*/
} BDSP_Raaga_P_MsgQueue;

typedef struct BDSP_Raaga_P_MsgQueue *BDSP_Raaga_P_MsgQueueHandle;

BERR_Code BDSP_Raaga_P_CreateMsgQueue(
    BDSP_Raaga_P_MsgQueueParams    *psMsgQueueParams,    /* [in]*/
    BREG_Handle                     hRegister,           /* [in] */
    uint32_t                        ui32DspOffset,       /* [in] */
    BDSP_Raaga_P_MsgQueueHandle     *hMsgQueue
);
BERR_Code BDSP_Raaga_P_InitMsgQueue(
    BDSP_Raaga_P_MsgQueueHandle hMsgQueue
);
BERR_Code BDSP_Raaga_P_DestroyMsgQueue(
    BDSP_Raaga_P_MsgQueueHandle    hMsgQueue
);
BERR_Code BDSP_Raaga_P_SendCommand_isr(
    BDSP_Raaga_P_MsgQueueHandle    hMsgQueue, /*[in]*/
    const BDSP_Raaga_P_Command    *psCommand  /*[in]*/
);
BERR_Code BDSP_Raaga_P_SendCommand(
    BDSP_Raaga_P_MsgQueueHandle    hMsgQueue, /*[in]*/
    const BDSP_Raaga_P_Command    *psCommand  /*[in]*/
);
BERR_Code BDSP_Raaga_P_ReadMsg_isr(
    BDSP_Raaga_P_MsgQueueHandle  hMsgQueue,  /*[in]*/
    void                        *pMsg,       /*[out]*/
    unsigned int                 uiMsgSize   /*[in]*/
);
BERR_Code BDSP_Raaga_P_GetResponse(
    BDSP_Raaga_P_MsgQueueHandle  hMsgQueue,  /*[in]*/
    void                        *pMsgBuf,    /*[out]*/
    unsigned int                 uiMsgSize   /*[in]*/
);
BERR_Code BDSP_Raaga_P_GetAsyncMsg_isr(
    BDSP_Raaga_P_MsgQueueHandle    hMsgQueue,  /*[in]*/
    void                        *pMsgBuf,   /*[in]*/
    unsigned int                *puiNumMsgs /*[out]*/
);
#endif /*BDSP_RAAGA_FWINTERFACE_PRIV_H_*/
