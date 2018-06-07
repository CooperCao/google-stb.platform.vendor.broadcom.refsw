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

#include "bdsp_arm_priv_include.h"


BDBG_MODULE(bdsp_arm_fwinterface);

/***********************************************************************
Name        :   BDSP_Arm_P_ReleaseFIFO

Type        :   BDSP Internal

Input       :   pDevice     -   Handle of the Device.
                dspIndex        -   Index of the DSP.
                i32Fifo     -   Start index of the FIFO's Allocated.
                numfifos        -   Number of FIFO's allocated.

Return      :   Error Code to return SUCCESS or FAILURE

Functionality   :   Following are the operations performed.
        1)  Acquire the FIFOID mutex of the DSP.
        2)  Free the FIFOs by setting the array to FALSE.
        3)  Release the Mutex before returning.
***********************************************************************/
BERR_Code BDSP_Arm_P_ReleaseInterfaceQueueHandle(BDSP_Arm   *pDevice, int32_t i32QueueHandleIndex)
{
    BKNI_AcquireMutex(pDevice->armInterfaceQHndlMutex);
    if(true != pDevice->armIntrfcQHndlFlag[i32QueueHandleIndex])
    {
        BDBG_ERR(("Unused Arm Queue Handle %d being freed", i32QueueHandleIndex));
        BKNI_ReleaseMutex(pDevice->armInterfaceQHndlMutex);
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    BDBG_MSG(("Freeing Arm Queue Handle %d", i32QueueHandleIndex));
    pDevice->armIntrfcQHndlFlag[i32QueueHandleIndex]= false;

    BKNI_ReleaseMutex(pDevice->armInterfaceQHndlMutex);
    return BERR_SUCCESS;
}


/***********************************************************************
Name        :   BDSP_Arm_P_AssignFreeFIFO

Type        :   BDSP Internal

Input       :   pDevice     -   Handle of the Device.
                dspIndex        -   Index of the DSP.
                i32Fifo     -   Start index of the FIFO's Allocated to be returned back.
                numfifosreqd    -   Number of FIFO's requested.

Return      :   Error Code to return SUCCESS or FAILURE

Functionality   :   Following are the operations performed.
        1)  Acquire the FIFOID mutex of the DSP.
        2)  Poll for the contiguous free FIFOs requested for in the DSP list.
        3)  Acquire the FIFOs if available by setting the value to TRUE.
        4)  Return the start index of the FIFOs from where the contiguous list is free.
        4)  Release the Mutex before returning.
***********************************************************************/
BERR_Code BDSP_Arm_P_AssignFreeInterfaceQueueHandle(BDSP_Arm *pDevice, int32_t* i32QueueHandleIndex)
{
    BERR_Code   err=BERR_SUCCESS;

    int32_t i =0;
    BKNI_AcquireMutex(pDevice->armInterfaceQHndlMutex);
    /* Find free Arm Queue Handle  Index*/
    for (i=0; i < (int32_t)BDSP_ARM_NUM_INTERFACE_QUEUE_HANDLE; i++)
    {
        if (false == pDevice->armIntrfcQHndlFlag[i])
        {
            /* Found a free Arm Queue Handle Index*/
            break;
        }
    }
    if (i >= (int32_t)BDSP_ARM_NUM_INTERFACE_QUEUE_HANDLE)
    {
        BKNI_ReleaseMutex(pDevice->armInterfaceQHndlMutex);
        err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
        return err;
    }

    BDBG_MSG(("Allocating Arm Queue Handle Index %d", i));
    pDevice->armIntrfcQHndlFlag[i] = true;
    *i32QueueHandleIndex = i;
    BKNI_ReleaseMutex(pDevice->armInterfaceQHndlMutex);
    return BERR_SUCCESS;
}

BERR_Code BDSP_Arm_P_InitMsgQueue(
    BDSP_Arm *pDevice,
    BDSP_Arm_P_MsgQueueHandle    hMsgQueue
    )
{
    BERR_Code err=BERR_SUCCESS;
    uint32_t    ui32BaseAddrOffset=0;
    uint32_t    ui32EndAddrOffset=0;
    BDSP_MMA_Memory Memory;
    BDBG_ENTER(BDSP_Arm_P_InitMsgQueue);

	BDBG_MSG(("MSGQUEUE - uiBaseAddr %p, uiMsgQueueSize %u, i32MsgQID %d",
        hMsgQueue->Memory.pAddr,
        hMsgQueue->ui32Size,
        hMsgQueue->MsgQueueHandleIndex));

    ui32BaseAddrOffset = hMsgQueue->Memory.offset;
    ui32EndAddrOffset = ui32BaseAddrOffset + hMsgQueue->ui32Size;

    pDevice->sArmInterfaceQ.parmInterfaceQHndl[hMsgQueue->MsgQueueHandleIndex].ui32BaseAddr = ui32BaseAddrOffset;
    pDevice->sArmInterfaceQ.parmInterfaceQHndl[hMsgQueue->MsgQueueHandleIndex].ui32EndAddr  = ui32EndAddrOffset;
    pDevice->sArmInterfaceQ.parmInterfaceQHndl[hMsgQueue->MsgQueueHandleIndex].ui32WrapAddr = ui32EndAddrOffset;
    pDevice->sArmInterfaceQ.parmInterfaceQHndl[hMsgQueue->MsgQueueHandleIndex].ui32WriteAddr= ui32BaseAddrOffset;
    pDevice->sArmInterfaceQ.parmInterfaceQHndl[hMsgQueue->MsgQueueHandleIndex].ui32ReadAddr = ui32BaseAddrOffset;

    Memory = pDevice->sArmInterfaceQ.Memory;
    Memory.pAddr = &pDevice->sArmInterfaceQ.parmInterfaceQHndl[hMsgQueue->MsgQueueHandleIndex];
    BDSP_MMA_P_FlushCache(Memory, sizeof(BDSP_AF_P_sDRAM_CIRCULAR_BUFFER));

    hMsgQueue->psQueuePointer = &pDevice->sArmInterfaceQ.parmInterfaceQHndl[hMsgQueue->MsgQueueHandleIndex];

    BDBG_LEAVE(BDSP_Arm_P_InitMsgQueue);
    return err;
}

BERR_Code BDSP_Arm_P_InvalidateMsgQueue(
    BDSP_Arm *pDevice,
    BDSP_Arm_P_MsgQueueHandle    hMsgQueue   /* [in]*/
    )
{
    BERR_Code   err = BERR_SUCCESS;

    BDSP_MMA_Memory Memory;

    BDBG_ENTER(BDSP_Arm_P_InvalidateMsgQueue);

    Memory = pDevice->sArmInterfaceQ.Memory;
    pDevice->sArmInterfaceQ.parmInterfaceQHndl[hMsgQueue->MsgQueueHandleIndex].ui32BaseAddr = BDSP_ARM_INVALID_DRAM_ADDRESS;
    pDevice->sArmInterfaceQ.parmInterfaceQHndl[hMsgQueue->MsgQueueHandleIndex].ui32EndAddr  = BDSP_ARM_INVALID_DRAM_ADDRESS;
    pDevice->sArmInterfaceQ.parmInterfaceQHndl[hMsgQueue->MsgQueueHandleIndex].ui32WrapAddr = BDSP_ARM_INVALID_DRAM_ADDRESS;
    pDevice->sArmInterfaceQ.parmInterfaceQHndl[hMsgQueue->MsgQueueHandleIndex].ui32WriteAddr= BDSP_ARM_INVALID_DRAM_ADDRESS;
    pDevice->sArmInterfaceQ.parmInterfaceQHndl[hMsgQueue->MsgQueueHandleIndex].ui32ReadAddr = BDSP_ARM_INVALID_DRAM_ADDRESS;

    Memory.pAddr = &pDevice->sArmInterfaceQ.parmInterfaceQHndl[hMsgQueue->MsgQueueHandleIndex];
    BDSP_MMA_P_FlushCache(Memory, sizeof(BDSP_AF_P_sDRAM_CIRCULAR_BUFFER));

    hMsgQueue->psQueuePointer = NULL;
    BDBG_LEAVE  (BDSP_Arm_P_InvalidateMsgQueue);
    return err;
}

BERR_Code BDSP_Arm_P_CreateMsgQueue(
    BDSP_Arm *pDevice,
    BDSP_Arm_P_MsgQueueParams    *psMsgQueueParams   /* [in]*/,
    BDSP_Arm_P_MsgQueueHandle    *hMsgQueue
    )
{
    BERR_Code   err = BERR_SUCCESS;
    BDSP_Arm_P_MsgQueueHandle  hHandle = NULL;

    BDBG_ENTER(BDSP_Arm_P_CreateMsgQueue);

    /* Allocate memory for the Message Queue */
    hHandle =(BDSP_Arm_P_MsgQueueHandle)BKNI_Malloc(sizeof(struct BDSP_Arm_P_MsgQueue));

    if(hHandle == NULL)
    {
        err = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto end;
    }

    /* Create the Message Queue Handle  - get index, store virtual memory and size*/
    err = BDSP_Arm_P_AssignFreeInterfaceQueueHandle(pDevice, &(hHandle->MsgQueueHandleIndex));
    if (BERR_SUCCESS != err)
    {
        BDBG_ERR(("BDSP_Arm_P_CreateMsgQueue: Queue Creation failed in getting a QueuHandleIndex"));
        err = BERR_TRACE(err);
        goto err_assign_freeinterfacequeuehandle;
    }
    hHandle->Memory = psMsgQueueParams->Queue;
    hHandle->ui32Size  = psMsgQueueParams->uiMsgQueueSize;

    err = BDSP_Arm_P_InitMsgQueue(pDevice, hHandle);
    if (BERR_SUCCESS != err)
    {
        BDBG_ERR(("BDSP_Arm_P_CreateMsgQueue: Queue Creation failed in Init Stage!!!"));
        err = BERR_TRACE(err);
        goto err_init_queue;
    }

    *hMsgQueue = hHandle;
    goto end;
err_init_queue:
    BDSP_Arm_P_ReleaseInterfaceQueueHandle(pDevice,hHandle->MsgQueueHandleIndex);
err_assign_freeinterfacequeuehandle:
    BKNI_Free(hHandle);
end:
    BDBG_LEAVE(BDSP_Arm_P_CreateMsgQueue);
    return err;
}

BERR_Code BDSP_Arm_P_DestroyMsgQueue(
    BDSP_Arm *pDevice,
    BDSP_Arm_P_MsgQueueHandle    hMsgQueue
    )
{
    BERR_Code   err = BERR_SUCCESS;

    BDBG_ENTER(BDSP_Arm_P_DestroyMsgQueue);

    err = BDSP_Arm_P_InvalidateMsgQueue(pDevice,hMsgQueue);
    if (BERR_SUCCESS != err)
    {
        BDBG_ERR(("BDSP_Arm_P_DestroyMsgQueue: Queue Destroy failed in Invalidation Stage!!!"));
        err = BERR_TRACE(err);
    }

    err = BDSP_Arm_P_ReleaseInterfaceQueueHandle(pDevice, hMsgQueue->MsgQueueHandleIndex);
    if (BERR_SUCCESS != err)
    {
        BDBG_ERR(("BDSP_Arm_P_DestroyMsgQueue: Queue Destroy failed in Releasing of Handle Index!!!"));
        err = BERR_TRACE(err);
    }
	hMsgQueue->Memory.pAddr = NULL;
	hMsgQueue->Memory.offset = 0;
    hMsgQueue->ui32Size  = 0;
    hMsgQueue->MsgQueueHandleIndex = 0xFFFF;

    BKNI_Free(hMsgQueue);

    BDBG_LEAVE  (BDSP_Arm_P_DestroyMsgQueue);
    return err;
}

BERR_Code BDSP_Arm_P_WriteMsg_isr(
    BDSP_Arm_P_MsgQueueHandle    hMsgQueue   /*[in]*/,
    void                        *pMsgBuf,   /*[in]*/
    unsigned int                uiBufSize   /*[in]*/
    )
{
    BERR_Code err = BERR_SUCCESS;
    unsigned int i,uiFreeSpace=0;
    uint32_t ui32chunk1=0,ui32chunk2=0;
    uint32_t ui32dramReadPtr=0;
    uint32_t ui32dramWritePtr=0;
    uint32_t ui32maskReadPtr=0;
    uint32_t ui32maskWritePtr=0;
	BDSP_MMA_Memory MsgQueueWriteAddr;

    BDBG_ENTER(BDSP_Arm_P_WriteMsg_isr);

    BDBG_ASSERT(hMsgQueue);
    BDBG_ASSERT(pMsgBuf);

    ui32dramReadPtr  = hMsgQueue->psQueuePointer->ui32ReadAddr;
    ui32dramWritePtr = hMsgQueue->psQueuePointer->ui32WriteAddr;
    ui32maskReadPtr  = ui32dramReadPtr;
    ui32maskWritePtr = ui32dramWritePtr;

    /*Sanity check*/
    /* Checking boundness of read pointer -
    if((readptr>endaddr) OR (readptr<baseaddr)) read ptr not within bound */

    if ( (ui32maskReadPtr > hMsgQueue->psQueuePointer->ui32EndAddr)||
         (ui32maskReadPtr < hMsgQueue->psQueuePointer->ui32BaseAddr))
    {
        BDBG_ERR(("Read pointer not within bounds in Message Queue, Queue Handle ID =%d , ui32dramReadPtr = %d, EndAddr = %d, BaseAddr =%d",
            hMsgQueue->MsgQueueHandleIndex, ui32maskReadPtr, hMsgQueue->psQueuePointer->ui32EndAddr, hMsgQueue->psQueuePointer->ui32BaseAddr));
        BDBG_ASSERT(0);
        return BERR_TRACE(BDSP_ERR_BAD_DEVICE_STATE);
    }

    /*Checking boundness of write pointer -
    if((writeptr>endaddr) OR (writeptr<baseaddr))  write ptr not within bound */

    if ( (ui32maskWritePtr > hMsgQueue->psQueuePointer->ui32EndAddr)||
         (ui32maskWritePtr < hMsgQueue->psQueuePointer->ui32BaseAddr))
    {
        BDBG_ERR(("Write pointer not within bounds in Message Queue, Queue Handle ID =%d , ui32dramReadPtr = %d, EndAddr = %d, BaseAddr =%d",
            hMsgQueue->MsgQueueHandleIndex, ui32maskWritePtr, hMsgQueue->psQueuePointer->ui32EndAddr, hMsgQueue->psQueuePointer->ui32BaseAddr));
        BDBG_ASSERT(0);
        return BERR_TRACE(BDSP_ERR_BAD_DEVICE_STATE);
    }

    /* End of Sanity Check */

    /* --------------------------------------------------------------------

    Different cases:

    If maskwriteptr > maskreadptr
        ReadPtrMSB  WritePtrMSB     Freespace
        0           0               ((endaddr-writeptr)+(readptr-baseaddr));
        0           1               Impossible Condition
        1           0               Impossible Condition
        1           1               ((endaddr-writeptr)+(readptr-baseaddr));


    If maskreadptr > maskwriteptr
        ReadptrMSB  WritePtrMSB     Freespace
        0           0               Impossible Condition
        0           1               read-write
        1           0               read-write
        1           1               Impossible Condition

    If maskreadptr == maskwriteptr
        If the toggle bits are the same,then the buffer is empty
        If the toggle bits are different,the buffer is full

    --------------------------------------------------------------------- */

    /* Calculate the free space in the message queue */

    /* Case1: if(maskwriteptr>maskreadptr) */
    if(ui32maskWritePtr > ui32maskReadPtr)
    {
        uiFreeSpace = ((hMsgQueue->psQueuePointer->ui32EndAddr - ui32maskWritePtr)+
                      (ui32maskReadPtr - hMsgQueue->psQueuePointer->ui32BaseAddr))-uiBufSize;
    }

    /* Case2: if(maskreadptr>maskwriteptr) */
    if(ui32maskReadPtr > ui32maskWritePtr)
    {
        uiFreeSpace = (ui32maskReadPtr - ui32maskWritePtr)-uiBufSize;
    }

    /* Case3: if(maskreadptr==maskwriteptr) */
    if(ui32maskReadPtr == ui32maskWritePtr)
    {
        /* The buffer is empty */
        uiFreeSpace = (hMsgQueue->psQueuePointer->ui32EndAddr - hMsgQueue->psQueuePointer->ui32BaseAddr)-uiBufSize;
    }

    /* Generate BUFFER_FULL error when there is no space for the message to be
        written into the message queue*/

    if(uiFreeSpace <= 0)
    {
        BDBG_ERR(("No Free space in the buffer.No new messages can be written"));
        return BERR_TRACE(BDSP_ERR_BUFFER_FULL);
    }

    /* Writing Messages into the Message Queue */

    /*Assume: The Message buffer always has only one message */

    BDBG_MSG(("Buffer size should be a multiple of 4"));
    BDBG_ASSERT(!(uiBufSize%4));
    BDBG_MSG(("uiBufSize > %d", uiBufSize));

    /* hMsgQueue->pBaseAddr has the base address in cache format */
	MsgQueueWriteAddr = hMsgQueue->Memory;
	MsgQueueWriteAddr.pAddr = (void *)((uint8_t *)MsgQueueWriteAddr.pAddr + (ui32maskWritePtr - hMsgQueue->psQueuePointer->ui32BaseAddr ));

    /* Writing data in two chunks taking wrap-around into consideration */
    if ( (ui32maskWritePtr > ui32maskReadPtr)||
         (ui32maskWritePtr == ui32maskReadPtr))
    {
        if(uiBufSize > (hMsgQueue->psQueuePointer->ui32EndAddr - ui32maskWritePtr))
        {
            ui32chunk1 = hMsgQueue->psQueuePointer->ui32EndAddr - ui32maskWritePtr;
            ui32chunk2 = uiBufSize - ui32chunk1;
        }
        else
        {
            ui32chunk1 = uiBufSize;
            ui32chunk2 = 0;
        }
    }
    else
    {
        ui32chunk1 = uiBufSize;
        ui32chunk2 = 0;
    }

    /* Writing into chunk1 */
    for (i=0; i<(ui32chunk1/4); i++)
    {
        BDBG_MSG(("*((uint32_t *)pMsgBuf+i) > %x", *((uint32_t *)pMsgBuf+i)));
		err = BDSP_MMA_P_MemWrite32_isr(&MsgQueueWriteAddr, *((uint32_t *)pMsgBuf+i));
		if(err != BERR_SUCCESS)
		{
			BDBG_ERR(("BDSP_Arm_P_WriteMsg_isr: Error in updating the Data in the MSG Queue CHUNK 1"));
			goto end;
		}
		MsgQueueWriteAddr.pAddr = (void *)((uint8_t *)MsgQueueWriteAddr.pAddr+4);

        ui32dramWritePtr = ui32dramWritePtr + 4;
    }

    /* Toggling the write pointer to wrap around */
    if((ui32maskWritePtr + ui32chunk1) == hMsgQueue->psQueuePointer->ui32EndAddr )
    {
        ui32dramWritePtr = hMsgQueue->psQueuePointer->ui32BaseAddr;
        ui32maskWritePtr = ui32dramWritePtr;
    }

    /* Writing into chunk 2 */
    if ( ui32chunk2 > 0 )
    {
		MsgQueueWriteAddr = hMsgQueue->Memory;
		MsgQueueWriteAddr.pAddr = (void *)((uint8_t *)MsgQueueWriteAddr.pAddr + (ui32maskWritePtr - hMsgQueue->psQueuePointer->ui32BaseAddr ));
        for (i=0; i<(ui32chunk2/4); i++)
        {
            BDBG_MSG(("-->*((uint32_t *)pMsgBuf+i) > %x",
                       *((uint32_t *)pMsgBuf+(ui32chunk1/4)+i)));

			err = BDSP_MMA_P_MemWrite32_isr(&MsgQueueWriteAddr, *((uint32_t *)pMsgBuf+(ui32chunk1/4)+i));
			if(err != BERR_SUCCESS)
			{
				BDBG_ERR(("BDSP_Arm_P_WriteMsg_isr: Error in updating the Data in the MSG Queue CHUNK 2"));
				goto end;
			}
			MsgQueueWriteAddr.pAddr = (void *)((uint8_t *)MsgQueueWriteAddr.pAddr+4);

            ui32dramWritePtr = ui32dramWritePtr+4;
        }
    }

    BDBG_MSG(("ui32dramReadPtr > %x",  ui32dramReadPtr));
    BDBG_MSG(("ui32dramWritePtr > %x", ui32dramWritePtr));

    /* Updating write ptr in the handle */
    hMsgQueue->psQueuePointer->ui32WriteAddr = ui32dramWritePtr;
	BDSP_MMA_P_FlushCache_isr(hMsgQueue->Memory, sizeof(BDSP_Arm_P_MsgQueue));

end:
	BDBG_LEAVE(BDSP_Arm_P_WriteMsg_isr);
    return err;

}

BERR_Code BDSP_Arm_P_SendCommand_isr(
    BDSP_Arm_P_MsgQueueHandle    hMsgQueue   /*[in]*/,
    const BDSP_Arm_P_Command         *psCommand  /*[in]*/,
    void    *pTaskHandle       /*[in] Task handle */)
{
    BERR_Code err = BERR_SUCCESS;
    unsigned int uiCommandSize;
    BDSP_ArmTask *pArmTask = (BDSP_ArmTask *)pTaskHandle;

    BDBG_ENTER( BDSP_Arm_P_SendCommand_isr );

    BDBG_ASSERT( hMsgQueue );
    BDBG_ASSERT( psCommand );

    if ( (psCommand->sCommandHeader.ui32CommandID != BDSP_PING_COMMAND_ID)
        && (psCommand->sCommandHeader.ui32CommandID != BDSP_ARM_MAP_COMMAND_ID)
        && (psCommand->sCommandHeader.ui32CommandID != BDSP_ARM_UNMAP_COMMAND_ID))
    {
        /* When isStopped is true at that instance STOP/START commands can come
            and should be processed */

        if( (pArmTask->isStopped == true) &&
            (psCommand->sCommandHeader.ui32CommandID != \
                BDSP_START_TASK_COMMAND_ID))
        {
            BDBG_MSG(("Task is in stop state, Can't accept Command %#x",
                        psCommand->sCommandHeader.ui32CommandID ));
            return BERR_SUCCESS;
        }
    }
    else
    {
        BSTD_UNUSED(pArmTask);
    }

    uiCommandSize = sizeof(BDSP_Arm_P_Command);

    BDBG_MSG(("psCommand->sCommandHeader.ui32CommandSizeInBytes > %d",
                psCommand->sCommandHeader.ui32CommandSizeInBytes));

    err = BDSP_Arm_P_WriteMsg_isr( hMsgQueue,(void *) psCommand,uiCommandSize);
    if(BERR_SUCCESS != err)
    {
        return BERR_TRACE(err);
    }

    if(psCommand->sCommandHeader.ui32CommandID == \
            BDSP_STOP_TASK_COMMAND_ID)
    {
        pArmTask->isStopped = true;
    }

    BDBG_LEAVE( BDSP_Arm_P_SendCommand_isr );

    return err;

}

/*----------------------------------------------------------------------------*/
BERR_Code BDSP_Arm_P_SendCommand(
    BDSP_Arm_P_MsgQueueHandle    hMsgQueue   /*[in]*/,
    const BDSP_Arm_P_Command         *psCommand  /*[in]*/ ,
    void *pTaskHandle      /*[in] Task handle */
    )
{
    BERR_Code   rc = BERR_SUCCESS;
    BDBG_ENTER(BDSP_Arm_P_SendCommand);

    BKNI_EnterCriticalSection();
    rc = BDSP_Arm_P_SendCommand_isr(hMsgQueue, psCommand,pTaskHandle);
    BKNI_LeaveCriticalSection();
#if 0   /*SR_TBD*/
    BDBG_ERR(("Responce not implemented for command, Hence loop till read ptr == wrt ptr"));
    while(hMsgQueue->psQueuePointer->ui32ReadAddr != hMsgQueue->psQueuePointer->ui32WriteAddr)
    {
        BDSP_MEM_P_FlushCache_isr(hMsgQueue->hHeap,(void *)(&hMsgQueue->psQueuePointer->ui32WriteAddr),sizeof(hMsgQueue->psQueuePointer->ui32WriteAddr));
        BDSP_MEM_P_FlushCache_isr(hMsgQueue->hHeap,(void *)(&hMsgQueue->psQueuePointer->ui32ReadAddr),sizeof(hMsgQueue->psQueuePointer->ui32ReadAddr));
    }
#endif
    BDBG_LEAVE(BDSP_Arm_P_SendCommand);
    return rc;
}

BERR_Code BDSP_Arm_P_GetAsyncMsg_isr(
    BDSP_Arm_P_MsgQueueHandle    hMsgQueue,  /*[in]*/
    void                        *pMsgBuf,   /*[in]*/
    unsigned int                *puiNumMsgs /*[out]*/
    )
{
    BERR_Code err=BERR_SUCCESS;

    uint32_t ui32dramReadPtr=0;
    uint32_t ui32dramWritePtr=0;
    uint32_t ui32maskReadPtr=0;
    uint32_t ui32maskWritePtr=0;
    uint32_t ui32chunk1=0,ui32chunk2=0,i = 0;
    int32_t  i32BytesToBeRead=0;
    uint32_t ui32ResponseSize = 0;
	BDSP_MMA_Memory MsgQueueReadAddr;
    unsigned int uiMsgIndex = 0, uiContMsgs = 0, uiMoreMsgs = 0;

    BDBG_ENTER(BDSP_Arm_P_GetAsyncMsg_isr);

    BDBG_ASSERT(hMsgQueue);
    BDBG_ASSERT(pMsgBuf);
    BDBG_ASSERT(puiNumMsgs);
    BSTD_UNUSED(ui32chunk1);
    BSTD_UNUSED(ui32chunk2);

    *puiNumMsgs = 0;

    ui32dramReadPtr  = hMsgQueue->psQueuePointer->ui32ReadAddr;
    ui32dramWritePtr = hMsgQueue->psQueuePointer->ui32WriteAddr;

    BDBG_MSG(("ui32dramReadPtr > %x",  ui32dramReadPtr));
    BDBG_MSG(("ui32dramWritePtr > %x", ui32dramWritePtr));

    ui32maskReadPtr  = ui32dramReadPtr;
    ui32maskWritePtr = ui32dramWritePtr;

     /*Sanity check*/
    /* checking write ptr boundness -
        if((writeptr>endaddr)|(writeptr<baseaddr)) write ptr not within bound */

    if ( (ui32maskWritePtr > hMsgQueue->psQueuePointer->ui32EndAddr)||
         (ui32maskWritePtr < hMsgQueue->psQueuePointer->ui32BaseAddr))
    {
        BDBG_ERR(("Write pointer not within bounds in Message Queue"));
        BDBG_ASSERT(0);
        return BERR_TRACE(BDSP_ERR_BAD_DEVICE_STATE);
    }


    /* checking read ptr boundness -
        if((readptr>endaddr)|(readptr<baseaddr)) read ptr is not within bound */
    if ( (ui32maskReadPtr > hMsgQueue->psQueuePointer->ui32EndAddr)||
         (ui32maskReadPtr < hMsgQueue->psQueuePointer->ui32BaseAddr))
    {
        BDBG_ERR(("Read pointer not within bounds in Message Queue"));
        BDBG_ASSERT(0);
        return BERR_TRACE(BDSP_ERR_BAD_DEVICE_STATE);
    }

    /* End of Sanity Check */


    /* --------------------------------------------------------------------

    Different cases:

    If maskwriteptr > maskreadptr
        ReadPtrMSB  WritePtrMSB     Freespace
        0           0               write-read
        0           1               Impossible Condition
        1           0               Impossible Condition
        1           1               write-read


    If maskreadptr > maskwriteptr
        ReadptrMSB  WritePtrMSB     Freespace
        0           0               Impossible Condition
        0           1               (end-read)+(write-base)
        1           0               (end-read)+(write-base)
        1           1               Impossible Condition

    If maskreadptr == maskwriteptr
        If the toggle bits are the same,then there is no message to be read
        If the toggle bits are different, all the messages have to be read

    --------------------------------------------------------------------- */

    /* Condition for reading messages from the message queue into the
        message buffer If no msg is to be read, generate a
        BDSP_ERR_BUFFER_EMPTY error */
    ui32ResponseSize = BDSP_ARM_ASYNC_RESPONSE_SIZE_IN_BYTES;

    /* Checking if a message is present */

    /* Case1: if(readptr>writeptr) */
    if(ui32maskReadPtr > ui32maskWritePtr)
    {

        i32BytesToBeRead=(hMsgQueue->psQueuePointer->ui32EndAddr - ui32maskReadPtr)+
                (ui32maskWritePtr - hMsgQueue->psQueuePointer->ui32BaseAddr);

        uiContMsgs = (hMsgQueue->psQueuePointer->ui32EndAddr - ui32maskReadPtr)/
                        BDSP_ARM_ASYNC_RESPONSE_SIZE_IN_BYTES;

        uiMoreMsgs = (ui32maskWritePtr - hMsgQueue->psQueuePointer->ui32BaseAddr)/
                        BDSP_ARM_ASYNC_RESPONSE_SIZE_IN_BYTES;
    }

     /* Case2: if(writeptr>readptr) */
    if(ui32maskWritePtr > ui32maskReadPtr)
    {
        i32BytesToBeRead = ui32maskWritePtr - ui32maskReadPtr;
        uiContMsgs = i32BytesToBeRead / BDSP_ARM_ASYNC_RESPONSE_SIZE_IN_BYTES;
        uiMoreMsgs = 0;
    }

     if(i32BytesToBeRead <= 0)
     {
        BDBG_MSG(("The Message Queue is empty.No message is present."));
        /*BDBG_ASSERT(0); */
        /* Removing the assert to take care of int timings from fw */
        /*return BERR_TRACE(BDSP_ERR_BUFFER_EMPTY);*/
        return BERR_TRACE(err);
     }

    /* Revisit this if we make buffers a non-integral multiple of message size */
    *puiNumMsgs = i32BytesToBeRead/BDSP_ARM_ASYNC_RESPONSE_SIZE_IN_BYTES;

	MsgQueueReadAddr = hMsgQueue->Memory;
	MsgQueueReadAddr.pAddr = (void *)((uint8_t *)MsgQueueReadAddr.pAddr+(ui32maskReadPtr - hMsgQueue->psQueuePointer->ui32BaseAddr ));

    for(uiMsgIndex = 0; uiMsgIndex < uiContMsgs; uiMsgIndex++)
    {
        for(i=0; i<(ui32ResponseSize/4); i++)
        {
			*((uint32_t *)pMsgBuf+(uiMsgIndex * BDSP_ARM_ASYNC_RESPONSE_SIZE_IN_BYTES/4)+i) = BDSP_MMA_P_MemRead32_isr(&MsgQueueReadAddr);
			MsgQueueReadAddr.pAddr = (void *)((uint8_t *)MsgQueueReadAddr.pAddr + 4);

        }

        ui32dramReadPtr +=  BDSP_ARM_ASYNC_RESPONSE_SIZE_IN_BYTES;

        /*updating read ptr in the Queue Attribute Structure*/
        hMsgQueue->psQueuePointer->ui32ReadAddr = ui32dramReadPtr;
    }

	MsgQueueReadAddr = hMsgQueue->Memory;

    for(uiMsgIndex = 0; uiMsgIndex < uiMoreMsgs; uiMsgIndex++)
    {
        for(i=0;i<(ui32ResponseSize/4);i++)
        {
			*((uint32_t *)pMsgBuf+((uiMsgIndex+uiContMsgs) * BDSP_ARM_ASYNC_RESPONSE_SIZE_IN_BYTES/4)+i) = BDSP_MMA_P_MemRead32_isr(&MsgQueueReadAddr);
			MsgQueueReadAddr.pAddr = (void *)((uint8_t *)MsgQueueReadAddr.pAddr + 4);

        }
        ui32dramReadPtr = hMsgQueue->psQueuePointer->ui32BaseAddr +
                            (uiMsgIndex+1)*
                            BDSP_ARM_ASYNC_RESPONSE_SIZE_IN_BYTES;

        /*updating read ptr in the Queue Attribute Structure*/
        hMsgQueue->psQueuePointer->ui32ReadAddr = ui32dramReadPtr;
    }
    /*updating read ptr in the handle*/
    hMsgQueue->psQueuePointer->ui32ReadAddr = ui32dramReadPtr;

    BDBG_LEAVE(BDSP_Arm_P_GetAsyncMsg_isr);
    return err;

}

BERR_Code BDSP_Arm_P_GetMsg_isr(
    BDSP_Arm_P_MsgQueueHandle    hMsgQueue,  /*[in]*/
    void                         *pMsgBuf,/*[in]*/
    BDSP_P_MsgType           eMgsType
    )
{
    BERR_Code err=BERR_SUCCESS;

    uint32_t ui32dramReadPtr=0;
    uint32_t ui32dramWritePtr=0;
    uint32_t ui32maskReadPtr=0;
    uint32_t ui32maskWritePtr=0;
    uint32_t ui32chunk1=0,ui32chunk2=0,i;
    int32_t  i32BytesToBeRead=0;
    uint32_t ui32ResponseSize = 0;
	BDSP_MMA_Memory MsgQueueReadAddr;

    BDBG_ENTER(BDSP_Arm_P_GetMsg_isr);

    BDBG_ASSERT(hMsgQueue);
    BDBG_ASSERT(pMsgBuf);

    ui32dramReadPtr  = hMsgQueue->psQueuePointer->ui32ReadAddr;
    ui32dramWritePtr = hMsgQueue->psQueuePointer->ui32WriteAddr;

    BDBG_MSG(("ui32dramReadPtr > %x",  ui32dramReadPtr));
    BDBG_MSG(("ui32dramWritePtr > %x", ui32dramWritePtr));

    ui32maskReadPtr  = ui32dramReadPtr;
    ui32maskWritePtr = ui32dramWritePtr;

     /*Sanity check*/

    /* checking write ptr boundness- if((writeptr>endaddr)|(writeptr<baseaddr))
        write ptr is not within bound*/
    if ( (ui32maskWritePtr > hMsgQueue->psQueuePointer->ui32EndAddr)||
         (ui32maskWritePtr < hMsgQueue->psQueuePointer->ui32BaseAddr))
    {
        BDBG_ERR(("Write pointer not within bounds in Message Queue, Queue Handle ID =%d , ui32dramReadPtr = %d, EndAddr = %d, BaseAddr =%d",
            hMsgQueue->MsgQueueHandleIndex, ui32maskWritePtr, hMsgQueue->psQueuePointer->ui32EndAddr, hMsgQueue->psQueuePointer->ui32BaseAddr));
        BDBG_ASSERT(0);
        return BERR_TRACE(BDSP_ERR_BAD_DEVICE_STATE);
    }

    /* checking read ptr boundness- if((readptr>endaddr)|(readptr<baseaddr))
        read ptr is not within bound*/
    if ( (ui32maskReadPtr > hMsgQueue->psQueuePointer->ui32EndAddr)||
         (ui32maskReadPtr < hMsgQueue->psQueuePointer->ui32BaseAddr))
    {
        BDBG_ERR(("Read pointer not within bounds in Message Queue, Queue Handle ID =%d , ui32dramReadPtr = %d, EndAddr = %d, BaseAddr =%d",
            hMsgQueue->MsgQueueHandleIndex, ui32maskReadPtr, hMsgQueue->psQueuePointer->ui32EndAddr, hMsgQueue->psQueuePointer->ui32BaseAddr));
        BDBG_ASSERT(0);
        return BERR_TRACE(BDSP_ERR_BAD_DEVICE_STATE);
    }

    /* End of Sanity Check */

    /* --------------------------------------------------------------------

    Different cases:

    If maskwriteptr > maskreadptr
        ReadPtrMSB  WritePtrMSB     Freespace
        0           0               write-read
        0           1               Impossible Condition
        1           0               Impossible Condition
        1           1               write-read


    If maskreadptr > maskwriteptr
        ReadptrMSB  WritePtrMSB     Freespace
        0           0               Impossible Condition
        0           1               (end-read)+(write-base)
        1           0               (end-read)+(write-base)
        1           1               Impossible Condition

    If maskreadptr == maskwriteptr
        If the toggle bits are the same,then there is no message to be read
        If the toggle bits are different, all the messages have to be read

    --------------------------------------------------------------------- */

    /* Condition for reading messages from the message queue into the message
        buffer */

    /* If no msg is to be read, generate a BDSP_ERR_BUFFER_EMPTY */
    if(BDSP_P_MsgType_eSyn == eMgsType)
    {
        ui32ResponseSize = BDSP_ARM_RESPONSE_SIZE_IN_BYTES;
    }
    else if(BDSP_P_MsgType_eAsyn == eMgsType)
    {
        ui32ResponseSize = BDSP_ARM_ASYNC_RESPONSE_SIZE_IN_BYTES;
    }

    /* Checking if a msg is present */

    /* Case 1: if(readptr > writeptr)*/
    if (ui32maskReadPtr > ui32maskWritePtr)
    {
        i32BytesToBeRead = (hMsgQueue->psQueuePointer->ui32EndAddr - ui32maskReadPtr)+
                (ui32maskWritePtr - hMsgQueue->psQueuePointer->ui32BaseAddr);
    }

     /* Case 2:if(writeptr>readptr) */
    if (ui32maskWritePtr > ui32maskReadPtr)
    {
        i32BytesToBeRead = ui32maskWritePtr - ui32maskReadPtr;
    }

    /* Case 3: if readptr == writeptr */
    if (ui32maskWritePtr == ui32maskReadPtr)
    {
        /*All messages have to be read*/
        i32BytesToBeRead = hMsgQueue->psQueuePointer->ui32EndAddr - hMsgQueue->psQueuePointer->ui32BaseAddr;
    }
     if (i32BytesToBeRead < 0)
     {
        BDBG_ERR(("The Message Queue is empty.No message is present."));
        BDBG_ASSERT(0);
        return BERR_TRACE(BDSP_ERR_BUFFER_EMPTY);
     }

        /* hMsgQueue->pBaseAddr has the base address in cache format */
	MsgQueueReadAddr = hMsgQueue->Memory;
	MsgQueueReadAddr.pAddr = (void *)((uint8_t *)MsgQueueReadAddr.pAddr + (ui32maskReadPtr - hMsgQueue->psQueuePointer->ui32BaseAddr ));

    /* Reading data in two chunks taking wrap-around into consideration  */
    if ( (ui32maskReadPtr > ui32maskWritePtr)||
         (ui32maskReadPtr == ui32maskWritePtr))
    {
        if(ui32ResponseSize > (hMsgQueue->psQueuePointer->ui32EndAddr - ui32maskReadPtr))
        {
            ui32chunk1 = hMsgQueue->psQueuePointer->ui32EndAddr - ui32maskReadPtr;
            ui32chunk2 = i32BytesToBeRead - ui32chunk1;
        }
        else
        {
            ui32chunk1 = ui32ResponseSize;
            ui32chunk2 = 0;
        }
    }
    else
    {
        ui32chunk1 = ui32ResponseSize;
        ui32chunk2 = 0;
    }


    /* Reading from chunk1 */
    for(i=0;i<(ui32chunk1/4);i++)
    {
		*((uint32_t *)pMsgBuf+i) = BDSP_MMA_P_MemRead32_isr(&MsgQueueReadAddr);
		MsgQueueReadAddr.pAddr = (void *)((uint8_t * )MsgQueueReadAddr.pAddr + 4);
        ui32dramReadPtr = ui32dramReadPtr+4;
    }

    /* Toggling the read pointer */
    if(ui32maskReadPtr + ui32chunk1 == hMsgQueue->psQueuePointer->ui32EndAddr)
    {
        ui32dramReadPtr = hMsgQueue->psQueuePointer->ui32BaseAddr;
        ui32maskReadPtr = ui32dramReadPtr;
    }

    /* Reading from chunk2 */
    if(ui32chunk2>0)
    {
		MsgQueueReadAddr = hMsgQueue->Memory;
		MsgQueueReadAddr.pAddr = (void *)((uint8_t *)MsgQueueReadAddr.pAddr+ (ui32maskReadPtr - hMsgQueue->psQueuePointer->ui32BaseAddr ));
		for(i=0;i<(ui32chunk2/4);i++)
		{
			*((uint32_t *)pMsgBuf+i) = BDSP_MMA_P_MemRead32_isr(&MsgQueueReadAddr);
			MsgQueueReadAddr.pAddr = (void *)((uint8_t *)MsgQueueReadAddr.pAddr + 4);
			ui32dramReadPtr = ui32dramReadPtr+4;
		}
    }

    BDBG_MSG(("ui32dramReadPtr > %x",  ui32dramReadPtr));
    BDBG_MSG(("ui32dramWritePtr > %x", ui32dramWritePtr));

    /* Updating read ptr in the handle */
    hMsgQueue->psQueuePointer->ui32ReadAddr= ui32dramReadPtr;

    BDBG_LEAVE(BDSP_Arm_P_GetMsg_isr);
    return err;

}

/*----------------------------------------------------------------------------*/
BERR_Code BDSP_Arm_P_GetMsg(
    BDSP_Arm_P_MsgQueueHandle    hMsgQueue,  /*[in]*/
    void                        *pMsgBuf,   /*[in]*/
    BDSP_P_MsgType           eMgsType
    )
{
    BERR_Code   rc = BERR_SUCCESS;
    BDBG_ENTER(BDSP_Arm_P_GetMsg);

    BKNI_EnterCriticalSection();
    rc = BDSP_Arm_P_GetMsg_isr(hMsgQueue, pMsgBuf,eMgsType);
    BKNI_LeaveCriticalSection();

    BDBG_LEAVE(BDSP_Arm_P_GetMsg);
    return rc;
}

BERR_Code BDSP_Arm_P_GetAlgorithmSettings(
    BDSP_Algorithm          eAlgorithm,
    BDSP_MMA_Memory        *pMemory,
    uint32_t                ui32ConfigBufSize,    /* [in] Config Buf Size */
    void                   *pSettingsBuffer,
    size_t                  settingsBufferSize
    )
{
    const BDSP_Arm_P_AlgorithmInfo *pInfo;

    BDBG_ENTER( BDSP_Arm_P_GetAlgorithmSettings );

    if((pSettingsBuffer == NULL) || (settingsBufferSize ==0))
    {
        return BERR_SUCCESS;
    }

    pInfo = BDSP_Arm_P_LookupAlgorithmInfo(eAlgorithm);

    if ( settingsBufferSize != pInfo->userConfigSize )
    {
        BDBG_ERR(("Settings buffer size provided (%u) does not match expected size (%u) for algorithm %u (%s)",
                  (unsigned int)settingsBufferSize, (unsigned int)pInfo->userConfigSize, eAlgorithm, pInfo->pName));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    BDBG_ASSERT(settingsBufferSize <= ui32ConfigBufSize);

    BDSP_MMA_P_CopyDataFromDram(pSettingsBuffer, pMemory, settingsBufferSize);

    BDBG_LEAVE( BDSP_Arm_P_GetAlgorithmSettings );

    return BERR_SUCCESS;
}

BERR_Code BDSP_Arm_P_SetAlgorithmSettings(
    BDSP_Algorithm          eAlgorithm,
    BDSP_MMA_Memory        *pMemory,
    uint32_t                ui32ConfigBufSize,    /* [in] Config Buf Size */
    const void             *pSettingsBuffer,
    size_t                  settingsBufferSize
    )
{
	BERR_Code err = BERR_SUCCESS;
    const BDSP_Arm_P_AlgorithmInfo *pInfo;

    BDBG_ENTER( BDSP_Arm_P_SetAlgorithmSettings );

    if((pSettingsBuffer == NULL) || (settingsBufferSize ==0))
    {
        return BERR_SUCCESS;
    }

    pInfo = BDSP_Arm_P_LookupAlgorithmInfo(eAlgorithm);

    if ( settingsBufferSize != pInfo->userConfigSize )
    {
        BDBG_ERR(("Settings buffer size provided (%u) does not match expected size (%u) for algorithm %u (%s)",
                  (unsigned int)settingsBufferSize, (unsigned int)pInfo->userConfigSize, eAlgorithm, pInfo->pName));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    BDBG_ASSERT(settingsBufferSize <= ui32ConfigBufSize);

    err = BDSP_MMA_P_CopyDataToDram(pMemory, (void *)pSettingsBuffer, settingsBufferSize);
	if(err != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Arm_P_SetAlgorithmSettings: Error in Copying the Settings buffer"));
	}

    BDBG_LEAVE( BDSP_Arm_P_SetAlgorithmSettings );

	return err;
}

BERR_Code BDSP_Arm_P_GetFrameSyncTsmStageConfigParams_isr(
    BDSP_Algorithm  eAlgorithm,
	BDSP_MMA_Memory *pConfigBuf,
    uint32_t        ui32ConfigBufSize,     /* [in] Config Buf Size */
    void           *pSettingsBuffer,
    size_t          settingsBufferSize
    )
{
    const BDSP_Arm_P_AlgorithmInfo *pInfo;

    BDBG_ENTER( BDSP_Arm_P_GetFrameSyncTsmStageConfigParams_isr );

    if((pSettingsBuffer == NULL) || (settingsBufferSize ==0))
    {
        return BERR_SUCCESS;
    }

    pInfo = BDSP_Arm_P_LookupAlgorithmInfo_isrsafe(eAlgorithm);

    if ( settingsBufferSize != pInfo->idsConfigSize )
    {
        BDBG_ERR(("Datasync settings buffer size provided (%u) does not match expected size (%u) for algorithm %u (%s)",
                  (unsigned int)settingsBufferSize, (unsigned int)pInfo->userConfigSize, eAlgorithm, pInfo->pName));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    BDBG_ASSERT(settingsBufferSize <= ui32ConfigBufSize);

	BDSP_MMA_P_CopyDataFromDram_isr(pSettingsBuffer, pConfigBuf, settingsBufferSize);

    BDBG_LEAVE( BDSP_Arm_P_GetFrameSyncTsmStageConfigParams_isr );

    return BERR_SUCCESS;
}

BERR_Code BDSP_Arm_P_SetFrameSyncTsmStageConfigParams_isr(
    BDSP_Algorithm      eAlgorithm,
	BDSP_MMA_Memory    *pConfigBuf,
    uint32_t            uiConfigBufSize,     /* [in] Config Buf Size */
    const void         *pSettingsBuffer,
    size_t              settingsBufferSize
    )
{
    const BDSP_Arm_P_AlgorithmInfo *pInfo;

    BDBG_ENTER( BDSP_Arm_P_SetFrameSyncTsmStageConfigParams_isr );

    if((pSettingsBuffer == NULL) || (settingsBufferSize ==0))
    {
        return BERR_SUCCESS;
    }

    pInfo = BDSP_Arm_P_LookupAlgorithmInfo_isrsafe(eAlgorithm);

    if ( settingsBufferSize != pInfo->idsConfigSize )
    {
        BDBG_ERR(("Datasync settings buffer size provided (%u) does not match expected size (%u) for algorithm %u (%s)",
                  (unsigned int)settingsBufferSize, (unsigned int)pInfo->userConfigSize, eAlgorithm, pInfo->pName));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    BDBG_ASSERT(settingsBufferSize <= uiConfigBufSize);

	BDSP_MMA_P_CopyDataToDram_isr(pConfigBuf, (void *)pSettingsBuffer, settingsBufferSize);

    BDBG_LEAVE( BDSP_Arm_P_SetFrameSyncTsmStageConfigParams_isr );

    return BERR_SUCCESS;
}
