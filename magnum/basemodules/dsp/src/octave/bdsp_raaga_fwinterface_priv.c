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

#include "bdsp_raaga_priv_include.h"

BDBG_MODULE(bdsp_raaga_fwinterface);

BERR_Code BDSP_Raaga_P_CreateMsgQueue(
    BDSP_Raaga_P_MsgQueueParams    *psMsgQueueParams,    /* [in]*/
    BREG_Handle                     hRegister,           /* [in] */
    uint32_t                        ui32DspOffset,       /* [in] */
    BDSP_Raaga_P_MsgQueueHandle     *hMsgQueue
    )
{
    BERR_Code errCode = BERR_SUCCESS;
    BDSP_Raaga_P_MsgQueueHandle  hHandle = NULL;

    BDBG_ENTER(BDSP_Raaga_P_CreateMsgQueue);
    BDBG_ASSERT(hRegister);
    BDBG_ASSERT(psMsgQueueParams);
    BDBG_ASSERT(psMsgQueueParams->ui32FifoId != BDSP_RAAGA_FIFO_INVALID);

    BDBG_MSG(("CREATING MSGQUEUE - Base Address %p, Size %u, FifoId %d",
        psMsgQueueParams->Memory.pAddr,
        psMsgQueueParams->ui32Size,
        psMsgQueueParams->ui32FifoId));

    hHandle = (BDSP_Raaga_P_MsgQueueHandle)BKNI_Malloc(sizeof(BDSP_Raaga_P_MsgQueue));
    if(hHandle == NULL)
    {
        errCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        BDBG_ERR(("BDSP_Raaga_P_CreateMsgQueue: Error is allocating Kernal Memory for Message Queue"));
        goto end;
    }
    BKNI_Memset (hHandle, 0, sizeof(struct BDSP_Raaga_P_MsgQueue));
    hHandle->Memory        = psMsgQueueParams->Memory;
    hHandle->ui32FifoId    = psMsgQueueParams->ui32FifoId;
    hHandle->ui32Size      = psMsgQueueParams->ui32Size;
    hHandle->ui32DspOffset = ui32DspOffset;
    hHandle->hRegister     = hRegister;

    *hMsgQueue = hHandle;
    /* Address will be initilaised and FIFO registers will be initialised in the BDSP_Raaga_P_InitMsgQueue */
end:
    BDBG_LEAVE(BDSP_Raaga_P_CreateMsgQueue);
    return errCode;
}

BERR_Code BDSP_Raaga_P_InitMsgQueue(
    BDSP_Raaga_P_MsgQueueHandle hMsgQueue)
{
    BERR_Code errCode = BERR_SUCCESS;
    dramaddr_t  BaseOffset=0, EndOffset=0;
    uint32_t    ui32RegOffset;

    BDBG_ENTER(BDSP_Raaga_P_InitMsgQueue);
    BDBG_ASSERT(hMsgQueue);
    BDBG_ASSERT(hMsgQueue->hRegister);
    BDBG_ASSERT(hMsgQueue->ui32Size);
    BDBG_ASSERT(hMsgQueue->ui32FifoId != BDSP_RAAGA_FIFO_INVALID);

    /* Writing Zeros to the memory for which queue has be to be created */
    BKNI_Memset(hMsgQueue->Memory.pAddr, 0, hMsgQueue->ui32Size);
    BDSP_MMA_P_FlushCache(hMsgQueue->Memory, hMsgQueue->ui32Size);

    /* Initliasing the Physical Address Structure in the Queue Handle */
    hMsgQueue->Address.BaseOffset = hMsgQueue->Memory.offset;
    hMsgQueue->Address.ReadOffset = hMsgQueue->Memory.offset;
    hMsgQueue->Address.WriteOffset= hMsgQueue->Memory.offset;
    hMsgQueue->Address.EndOffset  = hMsgQueue->Memory.offset+ hMsgQueue->ui32Size;

    /* Initilaising the FIFO Registers */
    ui32RegOffset = BCHP_RAAGA_DSP_FW_CFG_FIFO_1_BASE_ADDR - \
                    BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR;

    BaseOffset = hMsgQueue->Memory.offset;

    BDSP_WriteReg64( hMsgQueue->hRegister,
        BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR + hMsgQueue->ui32DspOffset +
            (ui32RegOffset * hMsgQueue->ui32FifoId) +
            BDSP_RAAGA_P_FIFO_BASE_OFFSET,
        BaseOffset); /* base */

    BDSP_WriteReg64(hMsgQueue->hRegister,
        BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR + hMsgQueue->ui32DspOffset +
            (ui32RegOffset * hMsgQueue->ui32FifoId) +
            BDSP_RAAGA_P_FIFO_READ_OFFSET,
        BaseOffset); /* read */

    BDSP_WriteReg64(hMsgQueue->hRegister,
        BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR + hMsgQueue->ui32DspOffset +
            (ui32RegOffset * hMsgQueue->ui32FifoId) +
            BDSP_RAAGA_P_FIFO_WRITE_OFFSET,
        BaseOffset); /* write */

    EndOffset = BaseOffset + (hMsgQueue->ui32Size);

    BDSP_WriteReg64(hMsgQueue->hRegister,
        BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR + hMsgQueue->ui32DspOffset +
            (ui32RegOffset * hMsgQueue->ui32FifoId) +
            BDSP_RAAGA_P_FIFO_END_OFFSET,
        EndOffset); /* end */

    BDBG_LEAVE(BDSP_Raaga_P_InitMsgQueue);
    return errCode;
}

BERR_Code BDSP_Raaga_P_DestroyMsgQueue(
    BDSP_Raaga_P_MsgQueueHandle    hMsgQueue
    )
{
    BERR_Code   err = BERR_SUCCESS;
    uint32_t    ui32RegOffset = 0;

    BDBG_ENTER(BDSP_Raaga_P_DestroyMsgQueue);

    BDBG_ASSERT(hMsgQueue);
    BDBG_MSG(("Destroying MSGQUEUE - FifoId %d",hMsgQueue->ui32FifoId));

    /*Reseting the FIFO buffers to invalid dram address*/
    ui32RegOffset = BCHP_RAAGA_DSP_FW_CFG_FIFO_1_BASE_ADDR - \
                    BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR;

    BDSP_WriteReg64(hMsgQueue->hRegister,
        BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR + hMsgQueue->ui32DspOffset +
            (ui32RegOffset * hMsgQueue->ui32FifoId) +
            BDSP_RAAGA_P_FIFO_BASE_OFFSET,
        BDSP_RAAGA_INVALID_DRAM_ADDRESS); /* base */

    BDSP_WriteReg64(hMsgQueue->hRegister,
        BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR + hMsgQueue->ui32DspOffset +
            (ui32RegOffset * hMsgQueue->ui32FifoId) +
            BDSP_RAAGA_P_FIFO_END_OFFSET,
        BDSP_RAAGA_INVALID_DRAM_ADDRESS); /* end */

    BDSP_WriteReg64(hMsgQueue->hRegister,
        BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR + hMsgQueue->ui32DspOffset +
            (ui32RegOffset * hMsgQueue->ui32FifoId) +
            BDSP_RAAGA_P_FIFO_READ_OFFSET,
        BDSP_RAAGA_INVALID_DRAM_ADDRESS); /* read */

    BDSP_WriteReg64(hMsgQueue->hRegister,
        BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR + hMsgQueue->ui32DspOffset +
            (ui32RegOffset * hMsgQueue->ui32FifoId) +
            BDSP_RAAGA_P_FIFO_WRITE_OFFSET,
        BDSP_RAAGA_INVALID_DRAM_ADDRESS); /* write */

    BKNI_Free(hMsgQueue);
    BDBG_LEAVE(BDSP_Raaga_P_DestroyMsgQueue);
    return err;
}

static BERR_Code BDSP_Raaga_P_WriteMsg_isr(
    BDSP_Raaga_P_MsgQueueHandle    hMsgQueue  /*[in]*/,
    void                          *pMsg,      /*[in]*/
    unsigned int                   uiMsgSize     /*[in]*/
    )
{
    BERR_Code   errCode = BERR_SUCCESS;
    dramaddr_t  ReadOffset=0, WriteOffset=0;
    void *pSrc = NULL;
    unsigned int uiFreeSpace = 0, uiChunk1 = 0, uiChunk2 = 0;
    BDSP_MMA_Memory Memory;
    uint32_t    ui32RegOffset;

    BDBG_ENTER(BDSP_Raaga_P_WriteMsg_isr);
    BDBG_ASSERT(hMsgQueue);
    BDBG_ASSERT(pMsg);

    ui32RegOffset = BCHP_RAAGA_DSP_FW_CFG_FIFO_1_BASE_ADDR - \
                    BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR;

    ReadOffset = BDSP_ReadReg64(hMsgQueue->hRegister,
                   BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR + hMsgQueue->ui32DspOffset+
                   (ui32RegOffset * hMsgQueue->ui32FifoId)+
                   BDSP_RAAGA_P_FIFO_READ_OFFSET);

    WriteOffset = BDSP_ReadReg64(hMsgQueue->hRegister,
                   BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR + hMsgQueue->ui32DspOffset+
                   (ui32RegOffset * hMsgQueue->ui32FifoId)+
                   BDSP_RAAGA_P_FIFO_WRITE_OFFSET);

    BDBG_MSG(("BDSP_Raaga_P_WriteMsg_isr, Before Write: Write Offset="BDSP_MSG_FMT",Read Offset="BDSP_MSG_FMT,
        BDSP_MSG_ARG(WriteOffset),BDSP_MSG_ARG(ReadOffset)));

    /*Check for Sanity of Read Pointer  and update the Queue Read Pointer*/
    if ((ReadOffset > hMsgQueue->Address.EndOffset)||
        (ReadOffset < hMsgQueue->Address.BaseOffset))
    {
        BDBG_ERR(("Read pointer not within bounds in Message Queue, Fifo ID =%d , DSPOffset = %d, Read Offset = " BDSP_MSG_FMT
                    ",End Offset = " BDSP_MSG_FMT ",Base Offset = " BDSP_MSG_FMT, hMsgQueue->ui32FifoId,
                    hMsgQueue->ui32DspOffset, BDSP_MSG_ARG(ReadOffset), BDSP_MSG_ARG(hMsgQueue->Address.EndOffset),
                    BDSP_MSG_ARG(hMsgQueue->Address.BaseOffset)));
        BDBG_ASSERT(0);
    }
    hMsgQueue->Address.ReadOffset = ReadOffset;

    /*Check for Sanity of Write Pointer */
    if ((WriteOffset > hMsgQueue->Address.EndOffset)||
        (WriteOffset < hMsgQueue->Address.BaseOffset))
    {
        BDBG_ERR(("Write pointer not within bounds in Message Queue, Fifo ID =%d , DSPOffset = %d, Read Offset = " BDSP_MSG_FMT
                    ",End Offset = " BDSP_MSG_FMT ",Base Offset = " BDSP_MSG_FMT, hMsgQueue->ui32FifoId,
                    hMsgQueue->ui32DspOffset, BDSP_MSG_ARG(WriteOffset), BDSP_MSG_ARG(hMsgQueue->Address.EndOffset),
                    BDSP_MSG_ARG(hMsgQueue->Address.BaseOffset)));
        BDBG_ASSERT(0);
    }
    if (WriteOffset != hMsgQueue->Address.WriteOffset)
    {
        BDBG_ERR(("Write Offset corruption detected, From Register="BDSP_MSG_FMT", From QueueHandle "BDSP_MSG_FMT,
            BDSP_MSG_ARG(WriteOffset),BDSP_MSG_ARG(hMsgQueue->Address.WriteOffset) ));
        BDBG_ASSERT(0);
    }

    /* Scenarios of the Queue
            1. Write = Read (Full Empty condition). This condition is never hit again in future as freespace
                is always accounted with 4 bytes less.
                                                            |----------------|
                                                            ^                               ^
                                                        Ba, Rd, Wr                     End

            2. Write >Read(Not Wrapped)
                                                            |----//////------|
                                                            ^       ^        ^            ^
                                                            Ba     Rd       Wr         End

            3. Write  <Read (Wrapped )
                                                            |/////----////////|
                                                            ^       ^        ^             ^
                                                            Ba     Wr       Rd          End
       */

    if(WriteOffset == ReadOffset)
    {
        uiFreeSpace = hMsgQueue->Address.EndOffset - hMsgQueue->Address.BaseOffset - 4;
    }
    else if(WriteOffset > ReadOffset)
    {
        uiFreeSpace = (hMsgQueue->Address.EndOffset - WriteOffset)+
                       (ReadOffset - hMsgQueue->Address.BaseOffset) - 4;
    }
    else
    {
        uiFreeSpace = ReadOffset-WriteOffset - 4;
    }

    if(uiFreeSpace < uiMsgSize)
    {
        BDBG_ERR(("BDSP_Raaga_P_WriteMsg_isr: No space available to write the Message, Freespace (%d) MsgSize (%d)",uiFreeSpace,uiMsgSize));
        BDBG_ASSERT(0);
    }

    if(WriteOffset > ReadOffset)
    {
        /*Scenario 2 taken care here */
        if(uiMsgSize >(hMsgQueue->Address.EndOffset - WriteOffset))
        {
           uiChunk1 = (hMsgQueue->Address.EndOffset - WriteOffset);
           uiChunk2 = uiMsgSize - uiChunk1;
        }
        else
        {
            uiChunk1 = uiMsgSize;
            uiChunk2 = 0;
        }
    }
    else
    {
        /*Scenario 1, 3 taken care here */
        uiChunk1 = uiMsgSize;
        uiChunk2 = 0;
    }

    Memory = hMsgQueue->Memory;
    Memory.pAddr = (void *)((uint8_t *)Memory.pAddr + (WriteOffset - hMsgQueue->Address.BaseOffset));
    pSrc = pMsg;

    errCode = BDSP_MMA_P_CopyDataToDram_isr(&Memory, pSrc, (uint32_t)uiChunk1);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Raaga_P_WriteMsg_isr: Error in Writing Chunk 1"));
        BDBG_ASSERT(0);
    }
    WriteOffset = WriteOffset+uiChunk1;
    pSrc = (void *)((uint8_t *)pSrc + uiChunk1);
    if(WriteOffset == hMsgQueue->Address.EndOffset)
    {
        WriteOffset = hMsgQueue->Address.BaseOffset;
    }

    if(uiChunk2)
    {
        Memory = hMsgQueue->Memory;
        Memory.pAddr = (void *)((uint8_t *)Memory.pAddr + (WriteOffset - hMsgQueue->Address.BaseOffset));

        errCode = BDSP_MMA_P_CopyDataToDram_isr(&Memory, pSrc, (uint32_t)uiChunk2);
        if(errCode != BERR_SUCCESS)
        {
            BDBG_ERR(("BDSP_Raaga_P_WriteMsg_isr: Error in Writing Chunk 2"));
            BDBG_ASSERT(0);
        }
        WriteOffset = WriteOffset+uiChunk2;
    }

    BDSP_WriteReg64(hMsgQueue->hRegister,
        BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR + hMsgQueue->ui32DspOffset+
        (ui32RegOffset * hMsgQueue->ui32FifoId)+
        BDSP_RAAGA_P_FIFO_WRITE_OFFSET,
        WriteOffset);

    hMsgQueue->Address.WriteOffset = WriteOffset;

    BDBG_MSG(("BDSP_Raaga_P_WriteMsg_isr, After Write: Write Offset="BDSP_MSG_FMT",Read Offset="BDSP_MSG_FMT,
        BDSP_MSG_ARG(WriteOffset),BDSP_MSG_ARG(ReadOffset)));

    BDBG_LEAVE(BDSP_Raaga_P_WriteMsg_isr);
    return errCode;
}

BERR_Code BDSP_Raaga_P_ReadMsg_isr(
    BDSP_Raaga_P_MsgQueueHandle  hMsgQueue,  /*[in]*/
    void                        *pMsg,       /*[out]*/
    unsigned int                 uiMsgSize   /*[in]*/
)
{
    BERR_Code   errCode = BERR_SUCCESS;
    dramaddr_t  ReadOffset=0, WriteOffset=0;
    void *pDest = NULL;
    unsigned int uiBytesToRead = 0, uiChunk1 = 0, uiChunk2 = 0;
    BDSP_MMA_Memory Memory;
    uint32_t    ui32RegOffset;

    BDBG_ENTER(BDSP_Raaga_P_ReadMsg_isr);
    BDBG_ASSERT(hMsgQueue);
    BDBG_ASSERT(pMsg);

    ui32RegOffset = BCHP_RAAGA_DSP_FW_CFG_FIFO_1_BASE_ADDR - \
                    BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR;

    ReadOffset = BDSP_ReadReg64(hMsgQueue->hRegister,
                   BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR + hMsgQueue->ui32DspOffset+
                   (ui32RegOffset * hMsgQueue->ui32FifoId)+
                   BDSP_RAAGA_P_FIFO_READ_OFFSET);

    WriteOffset = BDSP_ReadReg64(hMsgQueue->hRegister,
                   BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR + hMsgQueue->ui32DspOffset+
                   (ui32RegOffset * hMsgQueue->ui32FifoId)+
                   BDSP_RAAGA_P_FIFO_WRITE_OFFSET);

    BDBG_MSG(("BDSP_Raaga_P_ReadMsg_isr: Before Read, Write Offset="BDSP_MSG_FMT",Read Offset="BDSP_MSG_FMT,
        BDSP_MSG_ARG(WriteOffset),BDSP_MSG_ARG(ReadOffset)));

    /*Check for Sanity of Read Pointer  and update the Queue Read Pointer*/
    if ((ReadOffset > hMsgQueue->Address.EndOffset)||
        (ReadOffset < hMsgQueue->Address.BaseOffset))
    {
        BDBG_ERR(("Read pointer not within bounds in Message Queue, Fifo ID =%d , DSPOffset = %d, Read Offset = " BDSP_MSG_FMT
                    ",End Offset = " BDSP_MSG_FMT ",Base Offset = " BDSP_MSG_FMT, hMsgQueue->ui32FifoId,
                    hMsgQueue->ui32DspOffset, BDSP_MSG_ARG(ReadOffset), BDSP_MSG_ARG(hMsgQueue->Address.EndOffset),
                    BDSP_MSG_ARG(hMsgQueue->Address.BaseOffset)));
        BDBG_ASSERT(0);
    }

    /*Check for Sanity of Write Pointer */
    if ((WriteOffset > hMsgQueue->Address.EndOffset)||
        (WriteOffset < hMsgQueue->Address.BaseOffset))
    {
        BDBG_ERR(("Write pointer not within bounds in Message Queue, Fifo ID =%d , DSPOffset = %d, Read Offset = " BDSP_MSG_FMT
                    ",End Offset = " BDSP_MSG_FMT ",Base Offset = " BDSP_MSG_FMT, hMsgQueue->ui32FifoId,
                    hMsgQueue->ui32DspOffset, BDSP_MSG_ARG(WriteOffset), BDSP_MSG_ARG(hMsgQueue->Address.EndOffset),
                    BDSP_MSG_ARG(hMsgQueue->Address.BaseOffset)));
        BDBG_ASSERT(0);
    }
    hMsgQueue->Address.WriteOffset = WriteOffset;

    if (ReadOffset != hMsgQueue->Address.ReadOffset)
    {
        BDBG_ERR(("Read Offset corruption detected, From Register="BDSP_MSG_FMT", From QueueHandle ="BDSP_MSG_FMT,
            BDSP_MSG_ARG(ReadOffset),BDSP_MSG_ARG(hMsgQueue->Address.ReadOffset)));
        BDBG_ASSERT(0);
    }

    /* Scenarios of the Queue
            1. Write = Read (Full condition).
                                                            |////////////////|
                                                            ^              ^               ^
                                                           Ba          Rd, Wr          End

            2. Write >Read(Not Wrapped)
                                                            |----//////------|
                                                            ^       ^        ^            ^
                                                            Ba     Rd       Wr         End

            3. Write  <Read (Wrapped )
                                                            |/////----////////|
                                                            ^       ^        ^             ^
                                                            Ba     Wr       Rd          End
       */
    if(WriteOffset == ReadOffset)
    {
        /*(Write -Base) + (End -Read) is the actual logic */
        uiBytesToRead = hMsgQueue->Address.EndOffset - hMsgQueue->Address.BaseOffset;
    }
    else if(WriteOffset > ReadOffset)
    {
        uiBytesToRead = WriteOffset - ReadOffset;
    }
    else
    {
        uiBytesToRead = (hMsgQueue->Address.EndOffset - ReadOffset)+
                            (WriteOffset - hMsgQueue->Address.BaseOffset);
    }

    if(uiBytesToRead < uiMsgSize)
    {
        BDBG_ERR(("BDSP_Raaga_P_ReadMsg_isr: No messages available, BytestoRead (%d) MsgSize (%d)",uiBytesToRead,uiMsgSize));
        BDBG_ASSERT(0);
    }

    if(ReadOffset > WriteOffset)
    {
        /*Scenario 3 taken care here */
        if(uiMsgSize >(hMsgQueue->Address.EndOffset - ReadOffset))
        {
           uiChunk1 = (hMsgQueue->Address.EndOffset - ReadOffset);
           uiChunk2 = uiMsgSize - uiChunk1;
        }
        else
        {
            uiChunk1 = uiMsgSize;
            uiChunk2 = 0;
        }
    }
    else
    {
        /*Scenario 1, 2 taken care here */
        uiChunk1 = uiMsgSize;
        uiChunk2 = 0;
    }

    Memory = hMsgQueue->Memory;
    Memory.pAddr = (void *)((uint8_t *)Memory.pAddr + (ReadOffset - hMsgQueue->Address.BaseOffset));
    pDest = pMsg;

    errCode = BDSP_MMA_P_CopyDataFromDram_isr(pDest, &Memory, (uint32_t)uiChunk1);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Raaga_P_WriteMsg_isr: Error in Writing Chunk 1"));
        BDBG_ASSERT(0);
    }
    ReadOffset = ReadOffset+uiChunk1;
    pDest = (void *)((uint8_t *)pDest + uiChunk1);
    if(ReadOffset == hMsgQueue->Address.EndOffset)
    {
        ReadOffset = hMsgQueue->Address.BaseOffset;
    }

    if(uiChunk2)
    {
        Memory = hMsgQueue->Memory;
        Memory.pAddr = (void *)((uint8_t *)Memory.pAddr + (ReadOffset - hMsgQueue->Address.BaseOffset));

        errCode = BDSP_MMA_P_CopyDataFromDram_isr(pDest, &Memory, (uint32_t)uiChunk2);
        if(errCode != BERR_SUCCESS)
        {
            BDBG_ERR(("BDSP_Raaga_P_WriteMsg_isr: Error in Writing Chunk 2"));
            BDBG_ASSERT(0);
        }
        ReadOffset = ReadOffset+uiChunk2;
    }

    BDSP_WriteReg64(hMsgQueue->hRegister,
        BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR + hMsgQueue->ui32DspOffset+
        (ui32RegOffset * hMsgQueue->ui32FifoId)+
        BDSP_RAAGA_P_FIFO_READ_OFFSET,
        ReadOffset);

    hMsgQueue->Address.ReadOffset = ReadOffset;

    BDBG_MSG(("BDSP_Raaga_P_ReadMsg_isr: After Read, Write Offset="BDSP_MSG_FMT",Read Offset="BDSP_MSG_FMT,
        BDSP_MSG_ARG(WriteOffset),BDSP_MSG_ARG(ReadOffset)));

    BDBG_LEAVE(BDSP_Raaga_P_ReadMsg_isr);
    return errCode;
}

BERR_Code BDSP_Raaga_P_SendCommand_isr(
    BDSP_Raaga_P_MsgQueueHandle    hMsgQueue, /*[in]*/
    const BDSP_Raaga_P_Command    *psCommand  /*[in]*/
)
{
    BERR_Code   errCode = BERR_SUCCESS;
    unsigned int uiCommandSize;
    BDBG_ENTER(BDSP_Raaga_P_SendCommand_isr);

    BDBG_ASSERT(hMsgQueue);
    BDBG_ASSERT(psCommand);
    uiCommandSize = sizeof(BDSP_Raaga_P_Command);
    BDBG_MSG(("psCommand->sCommandHeader.ui32CommandSizeInBytes > %d",
                psCommand->sCommandHeader.ui32CommandSizeInBytes));

    errCode = BDSP_Raaga_P_WriteMsg_isr(hMsgQueue, (void *)psCommand, uiCommandSize);
    if(BERR_SUCCESS != errCode)
    {
        BDBG_ERR(("BDSP_Raaga_P_WriteMsg_isr: Error in writing the Message into Queue"));
        goto end;
    }

end:
    BDBG_LEAVE(BDSP_Raaga_P_SendCommand_isr);
    return errCode;
}

BERR_Code BDSP_Raaga_P_SendCommand(
    BDSP_Raaga_P_MsgQueueHandle    hMsgQueue, /*[in]*/
    const BDSP_Raaga_P_Command    *psCommand  /*[in]*/
    )
{
    BERR_Code   errCode = BERR_SUCCESS;
    BDBG_ENTER(BDSP_Raaga_P_SendCommand);

    BKNI_EnterCriticalSection();
    errCode= BDSP_Raaga_P_SendCommand_isr(hMsgQueue, psCommand);
    BKNI_LeaveCriticalSection();

    BDBG_LEAVE(BDSP_Raaga_P_SendCommand);
    return errCode;
}
BERR_Code BDSP_Raaga_P_GetResponse(
    BDSP_Raaga_P_MsgQueueHandle  hMsgQueue,  /*[in]*/
    void                        *pMsgBuf,    /*[out]*/
    unsigned int                 uiMsgSize   /*[in]*/
    )
{
    BERR_Code   errCode = BERR_SUCCESS;
    BDBG_ENTER(BDSP_Raaga_P_GetResponse);

    BKNI_EnterCriticalSection();
    errCode = BDSP_Raaga_P_ReadMsg_isr(hMsgQueue, pMsgBuf, uiMsgSize);
    BKNI_LeaveCriticalSection();

    BDBG_LEAVE(BDSP_Raaga_P_GetResponse);
    return errCode;
}

BERR_Code BDSP_Raaga_P_GetAsyncMsg_isr(
    BDSP_Raaga_P_MsgQueueHandle    hMsgQueue,  /*[in]*/
    void                        *pMsgBuf,   /*[in]*/
    unsigned int                *puiNumMsgs /*[out]*/
)
{
    BERR_Code   errCode = BERR_SUCCESS;
    dramaddr_t  ReadOffset=0, WriteOffset=0;
    void *pDest = NULL;
    unsigned uiBytesToRead = 0, uiChunk1 = 0, uiChunk2 = 0;
    BDSP_MMA_Memory Memory;
    uint32_t    ui32RegOffset;

    BDBG_ENTER(BDSP_Raaga_P_GetAsyncMsg_isr);
    BDBG_ASSERT(hMsgQueue);
    BDBG_ASSERT(pMsgBuf);

    ui32RegOffset = BCHP_RAAGA_DSP_FW_CFG_FIFO_1_BASE_ADDR - \
                    BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR;

    ReadOffset = BDSP_ReadReg64(hMsgQueue->hRegister,
                   BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR + hMsgQueue->ui32DspOffset+
                   (ui32RegOffset * hMsgQueue->ui32FifoId)+
                   BDSP_RAAGA_P_FIFO_READ_OFFSET);

    WriteOffset = BDSP_ReadReg64(hMsgQueue->hRegister,
                   BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR + hMsgQueue->ui32DspOffset+
                   (ui32RegOffset * hMsgQueue->ui32FifoId)+
                   BDSP_RAAGA_P_FIFO_WRITE_OFFSET);

    BDBG_MSG(("BDSP_Raaga_P_GetAsyncMsg_isr: Before Read, Write Offset="BDSP_MSG_FMT",Read Offset="BDSP_MSG_FMT,
        BDSP_MSG_ARG(WriteOffset),BDSP_MSG_ARG(ReadOffset)));

    /*Check for Sanity of Read Pointer  and update the Queue Read Pointer*/
    if ((ReadOffset > hMsgQueue->Address.EndOffset)||
        (ReadOffset < hMsgQueue->Address.BaseOffset))
    {
        BDBG_ERR(("BDSP_Raaga_P_GetAsyncMsg_isr: Read pointer not within bounds in Message Queue, Fifo ID =%d , DSPOffset = %d, Read Offset = " BDSP_MSG_FMT
                    ",End Offset = " BDSP_MSG_FMT ",Base Offset = " BDSP_MSG_FMT, hMsgQueue->ui32FifoId,
                    hMsgQueue->ui32DspOffset, BDSP_MSG_ARG(ReadOffset), BDSP_MSG_ARG(hMsgQueue->Address.EndOffset),
                    BDSP_MSG_ARG(hMsgQueue->Address.BaseOffset)));
        BDBG_ASSERT(0);
    }

    /*Check for Sanity of Write Pointer */
    if ((WriteOffset > hMsgQueue->Address.EndOffset)||
        (WriteOffset < hMsgQueue->Address.BaseOffset))
    {
        BDBG_ERR(("BDSP_Raaga_P_GetAsyncMsg_isr: Write pointer not within bounds in Message Queue, Fifo ID =%d , DSPOffset = %d, Read Offset = " BDSP_MSG_FMT
                    ",End Offset = " BDSP_MSG_FMT ",Base Offset = " BDSP_MSG_FMT, hMsgQueue->ui32FifoId,
                    hMsgQueue->ui32DspOffset, BDSP_MSG_ARG(WriteOffset), BDSP_MSG_ARG(hMsgQueue->Address.EndOffset),
                    BDSP_MSG_ARG(hMsgQueue->Address.BaseOffset)));
        BDBG_ASSERT(0);
    }
    hMsgQueue->Address.WriteOffset = WriteOffset;

    if (ReadOffset != hMsgQueue->Address.ReadOffset)
    {
        BDBG_ERR(("Read Offset corruption detected, From Register="BDSP_MSG_FMT", From QueueHandle ="BDSP_MSG_FMT,
            BDSP_MSG_ARG(ReadOffset),BDSP_MSG_ARG(hMsgQueue->Address.ReadOffset)));
        BDBG_ASSERT(0);
    }

    /* Scenarios of the Queue

            1. Write >Read(Not Wrapped)
                                                            |----//////------|
                                                            ^       ^        ^            ^
                                                            Ba     Rd       Wr         End

            2. Write  <Read (Wrapped )
                                                            |/////----////////|
                                                            ^       ^        ^             ^
                                                            Ba     Wr       Rd          End
       */
    if(WriteOffset == ReadOffset)
    {
		/* This Scenario was triggered because interrupt bit got set in firmware whose msg was read already*/
        BDBG_MSG(("BDSP_Raaga_P_GetAsyncMsg_isr: The Message Queue is empty. No message is present."));
		return(BERR_SUCCESS);
    }
    else if(WriteOffset > ReadOffset)
    {
        uiBytesToRead = WriteOffset - ReadOffset;
    }
    else
    {
        uiBytesToRead = (hMsgQueue->Address.EndOffset - ReadOffset)+
                            (WriteOffset - hMsgQueue->Address.BaseOffset);
    }

	if(uiBytesToRead < sizeof(BDSP_Raaga_P_AsynMsg))
	{
		BDBG_ERR(("BDSP_Raaga_P_GetAsyncMsg_isr: Bytes to read(%d) is less than size of the size of single Async message",uiBytesToRead));
		BDBG_ASSERT(0);
	}
	BDBG_MSG(("BDSP_Raaga_P_GetAsyncMsg_isr: Payload size = %lu and bytes to read = %d",(unsigned long)(sizeof(BDSP_Raaga_P_AsynMsg)),uiBytesToRead));
    /* Revisit this if we make buffers a non-integral multiple of message size */
    *puiNumMsgs = uiBytesToRead/sizeof(BDSP_Raaga_P_AsynMsg);

    if(ReadOffset > WriteOffset)
    {
        /*Scenario 3 taken care here */
        if(uiBytesToRead >(hMsgQueue->Address.EndOffset - ReadOffset))
        {
           uiChunk1 = (hMsgQueue->Address.EndOffset - ReadOffset);
           uiChunk2 = uiBytesToRead - uiChunk1;
        }
        else
        {
            uiChunk1 = uiBytesToRead;
            uiChunk2 = 0;
        }
    }
    else
    {
        /*Scenario 1, 2 taken care here */
        uiChunk1 = uiBytesToRead;
        uiChunk2 = 0;
    }

    Memory = hMsgQueue->Memory;
    Memory.pAddr = (void *)((uint8_t *)Memory.pAddr + (ReadOffset - hMsgQueue->Address.BaseOffset));
    pDest = pMsgBuf;

    errCode = BDSP_MMA_P_CopyDataFromDram_isr(pDest, &Memory, (uint32_t)uiChunk1);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Raaga_P_GetAsyncMsg_isr: Error in Writing Chunk 1"));
        BDBG_ASSERT(0);
    }
    ReadOffset = ReadOffset+uiChunk1;

    pDest = (void *)((uint8_t *)pDest + uiChunk1);
    if(ReadOffset == hMsgQueue->Address.EndOffset)
    {
        ReadOffset = hMsgQueue->Address.BaseOffset;
    }

    if(uiChunk2)
    {
        Memory = hMsgQueue->Memory;
        Memory.pAddr = (void *)((uint8_t *)Memory.pAddr + (ReadOffset - hMsgQueue->Address.BaseOffset));

        errCode = BDSP_MMA_P_CopyDataFromDram_isr(pDest, &Memory, (uint32_t)uiChunk2);
        if(errCode != BERR_SUCCESS)
        {
            BDBG_ERR(("BDSP_Raaga_P_GetAsyncMsg_isr: Error in Writing Chunk 2"));
            BDBG_ASSERT(0);
        }
        ReadOffset = ReadOffset+uiChunk2;
    }

    BDSP_WriteReg64(hMsgQueue->hRegister,
        BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR + hMsgQueue->ui32DspOffset+
        (ui32RegOffset * hMsgQueue->ui32FifoId)+
        BDSP_RAAGA_P_FIFO_READ_OFFSET,
        ReadOffset);

    hMsgQueue->Address.ReadOffset = ReadOffset;

    BDBG_MSG(("BDSP_Raaga_P_GetAsyncMsg_isr: After Read, Write Offset="BDSP_MSG_FMT",Read Offset="BDSP_MSG_FMT,
        BDSP_MSG_ARG(WriteOffset),BDSP_MSG_ARG(ReadOffset)));

    BDBG_LEAVE(BDSP_Raaga_P_GetAsyncMsg_isr);
    return errCode;

}
