/******************************************************************************
 * (c) 2006-2016 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 *****************************************************************************/

#include "bdsp_raaga_fwinterface_priv.h"
#include "bdsp_raaga_priv.h"

#include "bdsp_common_mm_priv.h"


BDBG_MODULE(bdsp_raaga_fwinterface);

BERR_Code BDSP_Raaga_P_CreateMsgQueue(
    BDSP_Raaga_P_MsgQueueParams    *psMsgQueueParams,    /* [in]*/
    BMEM_Handle                     hHeap,               /* [in] */
    BREG_Handle                     hRegister,           /* [in] */
    uint32_t                        ui32DspOffset,       /* [in] */
    BDSP_Raaga_P_MsgQueueHandle    *hMsgQueue            /* [out]*/
    )
{

    BERR_Code err=BERR_SUCCESS;

    raaga_dramaddr  ui32BaseAddr=0, ui32EndAddr=0;
    BDSP_Raaga_P_MsgQueueHandle  hHandle = NULL;
    uint32_t    ui32RegOffset = 0;


    BDBG_ENTER(BDSP_Raaga_P_CreateMsgQueue);

    BDBG_ASSERT(psMsgQueueParams);
    BDBG_ASSERT(hHeap);
    BDBG_ASSERT(hRegister);
    BDBG_ASSERT(hMsgQueue);
    BDBG_ASSERT((unsigned)psMsgQueueParams->i32FifoId != BDSP_RAAGA_FIFO_INVALID);

    BDBG_MSG(("MSGQUEUE - pBaseAddr %p, Dsp index: %d uiMsgQueueSize %u, i32FifoId %d",
        psMsgQueueParams->pBaseAddr,!!ui32DspOffset,/* dsp offset being non-zero says it is DSP 1 else it is DSP0*/
        psMsgQueueParams->uiMsgQueueSize,
        psMsgQueueParams->i32FifoId));

    /* Allocate memory for the Message Queue */
    hHandle =(BDSP_Raaga_P_MsgQueueHandle)
              BKNI_Malloc(sizeof(struct BDSP_Raaga_P_MsgQueue));

    if(hHandle == NULL)
    {
        err = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto exit;
    }
    BKNI_Memset (hHandle, 0, sizeof(struct BDSP_Raaga_P_MsgQueue));
    hHandle->pBaseAddr = psMsgQueueParams->pBaseAddr;

    BKNI_Memset(psMsgQueueParams->pBaseAddr, 0, psMsgQueueParams->uiMsgQueueSize);
    BDSP_MEM_P_FlushCache(hHeap, psMsgQueueParams->pBaseAddr, psMsgQueueParams->uiMsgQueueSize);

    /* Conversion of address from virtual to physical*/
    BDSP_MEM_P_ConvertAddressToOffset(
            hHeap,psMsgQueueParams->pBaseAddr,&ui32BaseAddr);


    /*Initializing attributes of message queue in DRAM (device memory)*/
    ui32RegOffset = BCHP_RAAGA_DSP_FW_CFG_FIFO_1_BASE_ADDR - \
                    BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR;
    BDSP_Write32(
        hRegister,
        BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR +
            (ui32RegOffset * psMsgQueueParams->i32FifoId) +
            BDSP_RAAGA_P_FIFO_BASE_OFFSET + ui32DspOffset,
        ui32BaseAddr); /* base */

    ui32EndAddr = ui32BaseAddr + (psMsgQueueParams->uiMsgQueueSize);

    BDSP_Write32(
        hRegister,
        BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR +
            (ui32RegOffset * psMsgQueueParams->i32FifoId) +
            BDSP_RAAGA_P_FIFO_END_OFFSET + ui32DspOffset,
        ui32EndAddr); /* end */

    BDSP_Write32(
        hRegister,
        BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR +
            (ui32RegOffset * psMsgQueueParams->i32FifoId) +
            BDSP_RAAGA_P_FIFO_READ_OFFSET + ui32DspOffset,
        ui32BaseAddr); /* read */

    BDSP_Write32(
        hRegister,
        BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR +
            (ui32RegOffset * psMsgQueueParams->i32FifoId) +
            BDSP_RAAGA_P_FIFO_WRITE_OFFSET + ui32DspOffset,
        ui32BaseAddr); /* write */

    /* Initializes attributes in the local copy(handle) in system memory*/

    hHandle->hHeap          = hHeap;
    hHandle->hRegister      = hRegister;
    hHandle->ui32BaseAddr     = ui32BaseAddr ;
    hHandle->ui32EndAddr      = ui32EndAddr;
    hHandle->ui32ReadAddr      = ui32BaseAddr;
    hHandle->ui32WriteAddr     = ui32BaseAddr;
    hHandle->i32FifoId      = psMsgQueueParams->i32FifoId;
    hHandle->ui32DspOffset  = ui32DspOffset;

    *hMsgQueue = hHandle;

exit:

   BDBG_LEAVE(BDSP_Raaga_P_CreateMsgQueue);
   return err;

}

BERR_Code BDSP_Raaga_P_InitMsgQueue(
    BDSP_Raaga_P_MsgQueueParams    *psMsgQueueParams ,  /* [in]*/
    BMEM_Handle                     hHeap,              /* [in] */
    BREG_Handle                     hRegister,          /* [in] */
    uint32_t                        ui32DspOffset,      /* [in] */
    BDSP_Raaga_P_MsgQueueHandle    *hMsgQueue           /* [out]*/
    )
{

    BERR_Code err=BERR_SUCCESS;

    raaga_dramaddr          ui32BaseAddr=0, ui32EndAddr=0;
    BDSP_Raaga_P_MsgQueueHandle  hHandle = NULL;
    uint32_t    ui32RegOffset = 0;


    BDBG_ENTER(BDSP_Raaga_P_InitMsgQueue);

    BDBG_ASSERT(psMsgQueueParams);
    BDBG_ASSERT(hHeap);
    BDBG_ASSERT(hRegister);
    BDBG_ASSERT(hMsgQueue);
    BDBG_ASSERT((unsigned)psMsgQueueParams->i32FifoId != BDSP_RAAGA_FIFO_INVALID);

    BDBG_MSG(("MSGQUEUE - ui32BaseAddr %p, uiMsgQueueSize %u, i32FifoId %d",
        psMsgQueueParams->pBaseAddr,
        psMsgQueueParams->uiMsgQueueSize,
        psMsgQueueParams->i32FifoId));

    hHandle = *hMsgQueue;

    /* Conversion of address from virtual to physical*/
    BDSP_MEM_P_ConvertAddressToOffset(
            hHeap,psMsgQueueParams->pBaseAddr,&ui32BaseAddr);

    BKNI_Memset(psMsgQueueParams->pBaseAddr, 0, psMsgQueueParams->uiMsgQueueSize);
    BDSP_MEM_P_FlushCache(hHeap, psMsgQueueParams->pBaseAddr, psMsgQueueParams->uiMsgQueueSize);

    /*Initializing attributes of message queue in DRAM (device memory)*/

    ui32RegOffset = BCHP_RAAGA_DSP_FW_CFG_FIFO_1_BASE_ADDR - \
                    BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR;
    BDSP_Write32(
        hRegister,
        BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR +
            (ui32RegOffset * psMsgQueueParams->i32FifoId) +
            BDSP_RAAGA_P_FIFO_BASE_OFFSET + ui32DspOffset,
        ui32BaseAddr); /* base */

    ui32EndAddr = ui32BaseAddr + (psMsgQueueParams->uiMsgQueueSize);

    BDSP_Write32(
        hRegister,
        BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR +
            (ui32RegOffset * psMsgQueueParams->i32FifoId) +
            BDSP_RAAGA_P_FIFO_END_OFFSET + ui32DspOffset,
        ui32EndAddr); /* end */

    BDSP_Write32(
        hRegister,
        BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR +
            (ui32RegOffset * psMsgQueueParams->i32FifoId) +
            BDSP_RAAGA_P_FIFO_READ_OFFSET + ui32DspOffset,
        ui32BaseAddr); /* read */

    BDSP_Write32(
        hRegister,
        BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR +
            (ui32RegOffset * psMsgQueueParams->i32FifoId) +
            BDSP_RAAGA_P_FIFO_WRITE_OFFSET + ui32DspOffset,
        ui32BaseAddr); /* write */

    /* Initializes attributes in the local copy(handle) in system memory*/

    hHandle->hHeap          = hHeap;
    hHandle->hRegister      = hRegister;
    hHandle->ui32BaseAddr     = ui32BaseAddr ;
    hHandle->ui32EndAddr      = ui32EndAddr;
    hHandle->ui32ReadAddr      = ui32BaseAddr;
    hHandle->ui32WriteAddr     = ui32BaseAddr;
    hHandle->i32FifoId      = psMsgQueueParams->i32FifoId;
    hHandle->ui32DspOffset  = ui32DspOffset;

    BDBG_LEAVE(BDSP_Raaga_P_InitMsgQueue);
    return err;

}


BERR_Code BDSP_Raaga_P_DestroyMsgQueue(
    BDSP_Raaga_P_MsgQueueHandle    hMsgQueue,
    BREG_Handle                 hRegister,          /* [in] */
    uint32_t                    ui32DspOffset      /* [in] */
    )
{
    BERR_Code   err = BERR_SUCCESS;
    uint32_t    ui32RegOffset = 0;

    BDBG_ENTER(BDSP_Raaga_P_DestroyMsgQueue);

    BDBG_ASSERT(hMsgQueue);
    BDBG_MSG(("Destroying MSGQUEUE - Dsp index: %d , i32FifoId %d",
                !!ui32DspOffset,hMsgQueue->i32FifoId));/* dsp offset being non-zero says it is DSP 1 else it is DSP0*/

    /*Reseting the FIFO buffers to invalid dram address*/
    ui32RegOffset = BCHP_RAAGA_DSP_FW_CFG_FIFO_1_BASE_ADDR - \
                    BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR;

    BDSP_Write32(
        hRegister,
        BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR +
            (ui32RegOffset * hMsgQueue->i32FifoId) +
            BDSP_RAAGA_P_FIFO_BASE_OFFSET + ui32DspOffset,
        BDSP_RAAGA_INVALID_DRAM_ADDRESS); /* base */

    BDSP_Write32(
        hRegister,
        BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR +
            (ui32RegOffset * hMsgQueue->i32FifoId) +
            BDSP_RAAGA_P_FIFO_END_OFFSET + ui32DspOffset,
        BDSP_RAAGA_INVALID_DRAM_ADDRESS); /* end */

    BDSP_Write32(
        hRegister,
        BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR +
            (ui32RegOffset * hMsgQueue->i32FifoId) +
            BDSP_RAAGA_P_FIFO_READ_OFFSET + ui32DspOffset,
        BDSP_RAAGA_INVALID_DRAM_ADDRESS); /* read */

    BDSP_Write32(
        hRegister,
        BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR +
            (ui32RegOffset * hMsgQueue->i32FifoId) +
            BDSP_RAAGA_P_FIFO_WRITE_OFFSET + ui32DspOffset,
        BDSP_RAAGA_INVALID_DRAM_ADDRESS); /* write */

    BKNI_Free(hMsgQueue);
    BDBG_LEAVE  (BDSP_Raaga_P_DestroyMsgQueue);
    return err;

}


BERR_Code BDSP_Raaga_P_CreateRdbQueue(
    BDSP_Raaga_P_RdbQueueParams    *psMsgQueueParams ,  /* [in]*/
    BMEM_Handle                     hHeap,              /* [in] */
    BREG_Handle                     hRegister,          /* [in] */
    uint32_t                        ui32DspOffset,      /* [in] */
    BDSP_Raaga_P_MsgQueueHandle    *hMsgQueue           /* [out]*/
    )
{

    BERR_Code err=BERR_SUCCESS;

    raaga_dramaddr ui32BaseAddr=0, ui32EndAddr=0;
    BDSP_Raaga_P_MsgQueueHandle  hHandle = NULL;
    uint32_t    ui32RegOffset = 0;


    BDBG_ENTER(BDSP_Raaga_P_CreateRdbQueue);

    BDBG_ASSERT(psMsgQueueParams);
    BDBG_ASSERT(hHeap);
    BDBG_ASSERT(hRegister);
    BDBG_ASSERT(hMsgQueue);
    BDBG_ASSERT((unsigned)psMsgQueueParams->i32FifoId != BDSP_RAAGA_FIFO_INVALID);

    BDBG_MSG(("psMsgQueueParams->i32FifoId > %d",
                psMsgQueueParams->i32FifoId));

    /* Allocate memory for the Message Queue */
    hHandle =(BDSP_Raaga_P_MsgQueueHandle)
              BKNI_Malloc(sizeof(struct BDSP_Raaga_P_MsgQueue));

    if(hHandle == NULL)
    {
        err = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto exit;
    }
    BKNI_Memset (hHandle, 0, sizeof(struct BDSP_Raaga_P_MsgQueue));


    /* Base address is the actual Fifo Index from where RDBs are free. */
    ui32BaseAddr = BDSP_RAAGA_REGSET_ADDR_FOR_DSP( BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR + \
        psMsgQueueParams->startIndexOfFreeFifo*(BCHP_RAAGA_DSP_FW_CFG_FIFO_1_BASE_ADDR-BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR)+\
        ui32DspOffset );

    /*Initializing attributes of message queue in DRAM (device memory)*/

    ui32RegOffset = BCHP_RAAGA_DSP_FW_CFG_FIFO_1_BASE_ADDR - \
                    BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR;
    BDSP_Write32(
        hRegister,
        BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR +
        (ui32RegOffset * psMsgQueueParams->i32FifoId) +
        BDSP_RAAGA_P_FIFO_BASE_OFFSET + ui32DspOffset,
        ui32BaseAddr); /* base */

    ui32EndAddr = ui32BaseAddr + (psMsgQueueParams->uiMsgQueueSize);

    BDSP_Write32(
        hRegister,
        BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR +
            (ui32RegOffset * psMsgQueueParams->i32FifoId) +
            BDSP_RAAGA_P_FIFO_END_OFFSET + ui32DspOffset,
        ui32EndAddr); /* end */

    BDSP_Write32(
        hRegister,
        BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR +
            (ui32RegOffset * psMsgQueueParams->i32FifoId) +
            BDSP_RAAGA_P_FIFO_READ_OFFSET + ui32DspOffset,
        ui32BaseAddr); /* read */

    BDSP_Write32(
        hRegister,
        BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR +
            (ui32RegOffset * psMsgQueueParams->i32FifoId) +
            BDSP_RAAGA_P_FIFO_WRITE_OFFSET + ui32DspOffset,
        ui32BaseAddr); /* write */

    /* Initializes attributes in the local copy(handle) in system memory*/

    hHandle->hHeap          = hHeap;
    hHandle->hRegister      = hRegister;
    hHandle->ui32BaseAddr     = ui32BaseAddr ;
    hHandle->ui32EndAddr      = ui32EndAddr;
    hHandle->ui32ReadAddr      = ui32BaseAddr;
    hHandle->ui32WriteAddr     = ui32BaseAddr;
    hHandle->i32FifoId      = psMsgQueueParams->i32FifoId;
    hHandle->ui32DspOffset  = ui32DspOffset;

    *hMsgQueue = hHandle;

exit:

   BDBG_LEAVE(BDSP_Raaga_P_CreateRdbQueue);
   return err;

}

BERR_Code BDSP_Raaga_P_InitRdbQueue(
    BDSP_Raaga_P_RdbQueueParams    *psMsgQueueParams ,  /* [in]*/
    BMEM_Handle                     hHeap,              /* [in] */
    BREG_Handle                     hRegister,          /* [in] */
    uint32_t                        ui32DspOffset,      /* [in] */
    BDSP_Raaga_P_MsgQueueHandle    *hMsgQueue           /* [out]*/
    )
{

    BERR_Code err=BERR_SUCCESS;

    raaga_dramaddr ui32BaseAddr=0, ui32EndAddr=0;
    BDSP_Raaga_P_MsgQueueHandle  hHandle = NULL;
    uint32_t    ui32RegOffset = 0;


    BDBG_ENTER(BDSP_Raaga_P_InitRdbQueue);

    BDBG_ASSERT(psMsgQueueParams);
    BDBG_ASSERT(hHeap);
    BDBG_ASSERT(hRegister);
    BDBG_ASSERT(hMsgQueue);
    BDBG_ASSERT((unsigned)psMsgQueueParams->i32FifoId != BDSP_RAAGA_FIFO_INVALID);

    BDBG_MSG(("psMsgQueueParams->i32FifoId > %d",
                psMsgQueueParams->i32FifoId));

    hHandle = *hMsgQueue;


    /* Base address is the actual Fifo Index from where RDBs are free. */
    ui32BaseAddr = BDSP_RAAGA_REGSET_ADDR_FOR_DSP( BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR + \
        psMsgQueueParams->startIndexOfFreeFifo*(BCHP_RAAGA_DSP_FW_CFG_FIFO_1_BASE_ADDR-BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR)+\
        ui32DspOffset );

    /*Initializing attributes of message queue in DRAM (device memory)*/

    ui32RegOffset = BCHP_RAAGA_DSP_FW_CFG_FIFO_1_BASE_ADDR - \
                    BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR;
    BDSP_Write32(
        hRegister,
        BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR +
        (ui32RegOffset * psMsgQueueParams->i32FifoId) +
        BDSP_RAAGA_P_FIFO_BASE_OFFSET + ui32DspOffset,
        ui32BaseAddr); /* base */

    ui32EndAddr = ui32BaseAddr + (psMsgQueueParams->uiMsgQueueSize);

    BDSP_Write32(
        hRegister,
        BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR +
            (ui32RegOffset * psMsgQueueParams->i32FifoId) +
            BDSP_RAAGA_P_FIFO_END_OFFSET + ui32DspOffset,
        ui32EndAddr); /* end */

    BDSP_Write32(
        hRegister,
        BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR +
            (ui32RegOffset * psMsgQueueParams->i32FifoId) +
            BDSP_RAAGA_P_FIFO_READ_OFFSET + ui32DspOffset,
        ui32BaseAddr); /* read */

    BDSP_Write32(
        hRegister,
        BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR +
            (ui32RegOffset * psMsgQueueParams->i32FifoId) +
            BDSP_RAAGA_P_FIFO_WRITE_OFFSET + ui32DspOffset,
        ui32BaseAddr); /* write */

    /* Initializes attributes in the local copy(handle) in system memory*/

    hHandle->hHeap          = hHeap;
    hHandle->hRegister      = hRegister;
    hHandle->ui32BaseAddr     = ui32BaseAddr ;
    hHandle->ui32EndAddr      = ui32EndAddr;
    hHandle->ui32ReadAddr      = ui32BaseAddr;
    hHandle->ui32WriteAddr     = ui32BaseAddr;
    hHandle->i32FifoId      = psMsgQueueParams->i32FifoId;
    hHandle->ui32DspOffset  = ui32DspOffset;

    BDBG_LEAVE(BDSP_Raaga_P_InitRdbQueue);
    return err;

}

BERR_Code BDSP_Raaga_P_DestroyRdbQueue(
    BDSP_Raaga_P_MsgQueueHandle    hMsgQueue,
    BREG_Handle                    hRegister,          /* [in] */
    uint32_t                       ui32DspOffset      /* [in] */
    )
{
    BERR_Code   err = BERR_SUCCESS;
    uint32_t    ui32RegOffset = 0;

    BDBG_ENTER(BDSP_Raaga_P_DestroyRdbQueue);

    BDBG_ASSERT(hMsgQueue);

    /*Reseting the FIFO buffers to invalid dram address*/
    ui32RegOffset = BCHP_RAAGA_DSP_FW_CFG_FIFO_1_BASE_ADDR - \
                    BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR;

    BDSP_Write32(
        hRegister,
        BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR +
            (ui32RegOffset * hMsgQueue->i32FifoId) +
            BDSP_RAAGA_P_FIFO_BASE_OFFSET + ui32DspOffset,
        BDSP_RAAGA_INVALID_DRAM_ADDRESS); /* base */

    BDSP_Write32(
        hRegister,
        BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR +
            (ui32RegOffset * hMsgQueue->i32FifoId) +
            BDSP_RAAGA_P_FIFO_END_OFFSET + ui32DspOffset,
        BDSP_RAAGA_INVALID_DRAM_ADDRESS); /* end */

    BDSP_Write32(
        hRegister,
        BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR +
            (ui32RegOffset * hMsgQueue->i32FifoId) +
            BDSP_RAAGA_P_FIFO_READ_OFFSET + ui32DspOffset,
        BDSP_RAAGA_INVALID_DRAM_ADDRESS); /* read */

    BDSP_Write32(
        hRegister,
        BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR +
            (ui32RegOffset * hMsgQueue->i32FifoId) +
            BDSP_RAAGA_P_FIFO_WRITE_OFFSET + ui32DspOffset,
        BDSP_RAAGA_INVALID_DRAM_ADDRESS); /* write */

    BKNI_Free(hMsgQueue);
    BDBG_LEAVE  (BDSP_Raaga_P_DestroyRdbQueue);
    return err;

}

BERR_Code BDSP_Raaga_P_WriteMsg_isr(
    BDSP_Raaga_P_MsgQueueHandle    hMsgQueue   /*[in]*/,
    void                        *pMsgBuf,   /*[in]*/
    unsigned int                uiBufSize   /*[in]*/
    )
{
    BERR_Code err = BERR_SUCCESS;
    unsigned int i,uiFreeSpace=0;
    uint32_t ui32chunk1=0,ui32chunk2=0;
    raaga_dramaddr  ui32dramReadAddr=0;
    raaga_dramaddr  ui32dramWriteAddr=0;
    raaga_dramaddr  ui32maskReadAddr=0;
    raaga_dramaddr  ui32maskWriteAddr=0;
    void *pvMsgQueueWriteAddr=NULL;


    BDBG_ENTER(BDSP_Raaga_P_WriteMsg_isr);

    BDBG_ASSERT(hMsgQueue);
    BDBG_ASSERT(pMsgBuf);

    ui32dramReadAddr = BDSP_Read32_isr(
                        hMsgQueue->hRegister,
                        BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR +
                            (4 * 4 * hMsgQueue->i32FifoId) +
                            BDSP_RAAGA_P_FIFO_READ_OFFSET +
                            hMsgQueue->ui32DspOffset);

    ui32dramWriteAddr = BDSP_Read32_isr(
                        hMsgQueue->hRegister,
                        BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR +
                        (4 * 4 * hMsgQueue->i32FifoId) +
                        BDSP_RAAGA_P_FIFO_WRITE_OFFSET +
                        hMsgQueue->ui32DspOffset);

    ui32maskReadAddr  = ui32dramReadAddr;
    ui32maskWriteAddr = ui32dramWriteAddr;

    /*Sanity check*/
    /* Checking boundness of read pointer -
    if((readptr>endaddr) OR (readptr<baseaddr)) read ptr not within bound */

    if ( (ui32maskReadAddr > hMsgQueue->ui32EndAddr)||
         (ui32maskReadAddr<hMsgQueue->ui32BaseAddr))
    {
        BDBG_ERR(("Read pointer not within bounds in Message Queue, Fifo ID =%d , DSPOffset = %d, ui32dramReadAddr = %d, hMsgQueue->ui32EndAddr = %d, hMsgQueue->ui32BaseAddr =%d"
            ,hMsgQueue->i32FifoId,hMsgQueue->ui32DspOffset,ui32dramReadAddr,hMsgQueue->ui32EndAddr,hMsgQueue->ui32BaseAddr));
        BDBG_ASSERT(0);
        return BERR_TRACE(BDSP_ERR_BAD_DEVICE_STATE);
    }

    /*Checking boundness of write pointer -
    if((writeptr>endaddr) OR (writeptr<baseaddr))  write ptr not within bound */

    if ( (ui32maskWriteAddr>hMsgQueue->ui32EndAddr)||
         (ui32maskWriteAddr<hMsgQueue->ui32BaseAddr))
    {
        BDBG_ERR(("Write pointer not within bounds in Message Queue"));
        BDBG_ASSERT(0);
        return BERR_TRACE(BDSP_ERR_BAD_DEVICE_STATE);
    }

    /*update read ptr */
    hMsgQueue->ui32ReadAddr= ui32maskReadAddr;

    /* checking write ptrs */
    if((hMsgQueue->ui32WriteAddr)!=ui32maskWriteAddr)
    {
        BDBG_ERR(("Write pointer corrupted in the Message Queue, hMsgQueue->ui32WriteAddr=%x , ui32dramWriteAddr = %x"
            ,hMsgQueue->ui32WriteAddr,ui32dramWriteAddr));
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
    if(ui32maskWriteAddr > ui32maskReadAddr)
    {
        uiFreeSpace=((hMsgQueue->ui32EndAddr-ui32maskWriteAddr)+
                     (ui32maskReadAddr-hMsgQueue->ui32BaseAddr))-uiBufSize;
    }

    /* Case2: if(maskreadptr>maskwriteptr) */
    if(ui32maskReadAddr>ui32maskWriteAddr)
    {
        uiFreeSpace=(ui32maskReadAddr-ui32maskWriteAddr)-uiBufSize;
    }

    /* Case3: if(maskreadptr==maskwriteptr) */
    if(ui32maskReadAddr==ui32maskWriteAddr)
    {
        /* The buffer is empty */
        uiFreeSpace=(hMsgQueue->ui32EndAddr-hMsgQueue->ui32BaseAddr)-uiBufSize;
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
    pvMsgQueueWriteAddr = (void *)((uint8_t *)hMsgQueue->pBaseAddr + (ui32maskWriteAddr - hMsgQueue->ui32BaseAddr ));


    /* Writing data in two chunks taking wrap-around into consideration */
    if ( (ui32maskWriteAddr > ui32maskReadAddr)||
         (ui32maskWriteAddr == ui32maskReadAddr))
    {
        if(uiBufSize > (hMsgQueue->ui32EndAddr-ui32maskWriteAddr))
        {
            ui32chunk1 = hMsgQueue->ui32EndAddr-ui32maskWriteAddr;
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
        BDSP_P_MemWrite32_isr( hMsgQueue->hHeap,
            (uint8_t *)pvMsgQueueWriteAddr+(i*4), *((uint32_t *)pMsgBuf+i));

        ui32dramWriteAddr=ui32dramWriteAddr+4;
    }

    /* Toggling the write pointer to wrap around */
    if((ui32maskWriteAddr + ui32chunk1) == hMsgQueue->ui32EndAddr )
    {
        ui32dramWriteAddr = hMsgQueue->ui32BaseAddr;
        ui32maskWriteAddr = ui32dramWriteAddr;
    }

    /* Writing into chunk 2 */
    if ( ui32chunk2 > 0 )
    {

        pvMsgQueueWriteAddr = (void *)((uint8_t *)hMsgQueue->pBaseAddr + (ui32maskWriteAddr - hMsgQueue->ui32BaseAddr ));

        for (i=0; i<(ui32chunk2/4); i++)
        {
            BDBG_MSG(("-->*((uint32_t *)pMsgBuf+i) > %x",
                       *((uint32_t *)pMsgBuf+(ui32chunk1/4)+i)));
            BDSP_P_MemWrite32_isr(hMsgQueue->hHeap,
                (uint8_t *)pvMsgQueueWriteAddr+(i*4),
                *((uint32_t *)pMsgBuf+(ui32chunk1/4)+i)
                );
            ui32dramWriteAddr=ui32dramWriteAddr+4;
        }
    }

    /* Updating write ptr in the Queue Attribute Structure */
    BDSP_Write32_isr(
        hMsgQueue->hRegister,
        BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR +
            (4 * 4 * hMsgQueue->i32FifoId) +
            BDSP_RAAGA_P_FIFO_WRITE_OFFSET + hMsgQueue->ui32DspOffset,
        ui32dramWriteAddr); /* write */

    BDBG_MSG(("ui32dramReadAddr > %x",  ui32dramReadAddr));
    BDBG_MSG(("ui32dramWriteAddr > %x", ui32dramWriteAddr));

    /* Updating write ptr in the handle */
    hMsgQueue->ui32WriteAddr = ui32dramWriteAddr;

    BDBG_LEAVE(BDSP_Raaga_P_WriteMsg_isr);

    return err;

}


/*----------------------------------------------------------------------------*/
BERR_Code BDSP_Raaga_P_SendCommand(
    BDSP_Raaga_P_MsgQueueHandle    hMsgQueue   /*[in]*/,
    const BDSP_Raaga_P_Command         *psCommand  /*[in]*/ ,
    void *pTaskHandle      /*[in] Task handle */
    )
{
    BERR_Code   rc = BERR_SUCCESS;
    BDBG_ENTER(BDSP_Raaga_P_SendCommand);

    BKNI_EnterCriticalSection();
    rc = BDSP_Raaga_P_SendCommand_isr(hMsgQueue, psCommand,pTaskHandle);
    BKNI_LeaveCriticalSection();

    BDBG_LEAVE(BDSP_Raaga_P_SendCommand);
    return rc;
}


/*----------------------------------------------------------------------------*/
BERR_Code BDSP_Raaga_P_SendCommand_isr(
    BDSP_Raaga_P_MsgQueueHandle    hMsgQueue   /*[in]*/,
    const BDSP_Raaga_P_Command         *psCommand  /*[in]*/,
    void    *pTaskHandle       /*[in] Task handle */)
{
    BERR_Code err = BERR_SUCCESS;
    unsigned int uiCommandSize;
    BDSP_RaagaTask *pRaagaTask = (BDSP_RaagaTask *)pTaskHandle;

    BDBG_ENTER( BDSP_Raaga_P_SendCommand_isr );

    BDBG_ASSERT( hMsgQueue );
    BDBG_ASSERT( psCommand );


    if ( (psCommand->sCommandHeader.ui32CommandID != BDSP_RAAGA_PING_COMMAND_ID)
        && (psCommand->sCommandHeader.ui32CommandID != BDSP_RAAGA_GET_SYSTEM_SWAP_MEMORY_COMMAND_ID))
    {
        /* When isStopped is true at that instance STOP/START commands can come
            and should be processed */

        if( (pRaagaTask->isStopped == true) &&
            (psCommand->sCommandHeader.ui32CommandID != \
                BDSP_RAAGA_START_TASK_COMMAND_ID)
            &&(psCommand->sCommandHeader.ui32CommandID != \
                BDSP_RAAGA_GET_VOM_TABLE_COMMAND_ID)
          )
        {
            BDBG_MSG(("Task is in stop state, Can't accept Command %#x",
                        psCommand->sCommandHeader.ui32CommandID ));
            return BERR_SUCCESS;
        }
    }
    else
    {
        BSTD_UNUSED(pRaagaTask);
    }

    uiCommandSize = sizeof(BDSP_Raaga_P_Command);

    BDBG_MSG(("psCommand->sCommandHeader.ui32CommandSizeInBytes > %d",
                psCommand->sCommandHeader.ui32CommandSizeInBytes));

    err = BDSP_Raaga_P_WriteMsg_isr( hMsgQueue,(void *) psCommand,uiCommandSize);
    if(BERR_SUCCESS != err)
    {
        return BERR_TRACE(err);
    }

    if(psCommand->sCommandHeader.ui32CommandID == \
            BDSP_RAAGA_STOP_TASK_COMMAND_ID)
    {
        pRaagaTask->isStopped = true;
    }

    BDBG_LEAVE( BDSP_Raaga_P_SendCommand_isr );

    return err;

}

/*----------------------------------------------------------------------------*/
BERR_Code BDSP_Raaga_P_GetMsg(
    BDSP_Raaga_P_MsgQueueHandle    hMsgQueue,  /*[in]*/
    void                        *pMsgBuf,   /*[in]*/
    BDSP_Raaga_P_MsgType              eMgsType
    )
{
    BERR_Code   rc = BERR_SUCCESS;
    BDBG_ENTER(BDSP_Raaga_P_GetMsg);

    BKNI_EnterCriticalSection();
    rc = BDSP_Raaga_P_GetMsg_isr(hMsgQueue, pMsgBuf,eMgsType);
    BKNI_LeaveCriticalSection();

    BDBG_LEAVE(BDSP_Raaga_P_GetMsg);
    return rc;
}


/*----------------------------------------------------------------------------*/
BERR_Code BDSP_Raaga_P_GetMsg_isr(
    BDSP_Raaga_P_MsgQueueHandle    hMsgQueue,  /*[in]*/
    void                        *pMsgBuf,/*[in]*/
    BDSP_Raaga_P_MsgType              eMgsType
    )
{
    BERR_Code err=BERR_SUCCESS;

    raaga_dramaddr  ui32dramReadAddr=0;
    raaga_dramaddr  ui32dramWriteAddr=0;
    raaga_dramaddr  ui32maskReadAddr=0;
    raaga_dramaddr  ui32maskWriteAddr=0;
    uint32_t ui32chunk1=0,ui32chunk2=0,i;
    int32_t  i32BytesToBeRead=0;
    uint32_t ui32ResponseSize = 0;
    void *   pvMsgQueueReadAddr=NULL;

    BDBG_ENTER(BDSP_Raaga_P_GetMsg_isr);

    BDBG_ASSERT(hMsgQueue);
    BDBG_ASSERT(pMsgBuf);

    ui32dramReadAddr = BDSP_Read32_isr(
                        hMsgQueue->hRegister,
                        BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR +
                            (4 * 4 * hMsgQueue->i32FifoId) +
                            BDSP_RAAGA_P_FIFO_READ_OFFSET +
                            hMsgQueue->ui32DspOffset);

    ui32dramWriteAddr = BDSP_Read32_isr(
                        hMsgQueue->hRegister,
                        BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR +
                            (4 * 4 * hMsgQueue->i32FifoId) +
                            BDSP_RAAGA_P_FIFO_WRITE_OFFSET +
                            hMsgQueue->ui32DspOffset);

    BDBG_MSG(("ui32dramReadAddr > %x",  ui32dramReadAddr));
    BDBG_MSG(("ui32dramWriteAddr > %x", ui32dramWriteAddr));

    ui32maskReadAddr  = ui32dramReadAddr;
    ui32maskWriteAddr = ui32dramWriteAddr;

     /*Sanity check*/

    /* checking write ptr boundness- if((writeptr>endaddr)|(writeptr<baseaddr))
        write ptr is not within bound*/
    if( (ui32maskWriteAddr>hMsgQueue->ui32EndAddr)||
        (ui32maskWriteAddr<hMsgQueue->ui32BaseAddr))
    {
        BDBG_ERR(("Write pointer not within bounds in Message Queue"));
        BDBG_ASSERT(0);
        return BERR_TRACE(BDSP_ERR_BAD_DEVICE_STATE);
    }


    /* checking read ptr boundness- if((readptr>endaddr)|(readptr<baseaddr))
        read ptr is not within bound*/
    if( (ui32maskReadAddr>hMsgQueue->ui32EndAddr)||
        (ui32maskReadAddr<hMsgQueue->ui32BaseAddr))
    {
        BDBG_ERR(("Read pointer not within bounds in Message Queue"));
        BDBG_ASSERT(0);
        return BERR_TRACE(BDSP_ERR_BAD_DEVICE_STATE);
    }

    /*Updating write ptr */
    hMsgQueue->ui32WriteAddr= ui32maskWriteAddr;

    /* checking read ptrs to see if they are the same */
    if((hMsgQueue->ui32ReadAddr)!=ui32maskReadAddr)
    {
        BDBG_ERR(("Read pointer corrupted in the Message Queue"));
        BDBG_ASSERT(0);
        return BERR_TRACE(BDSP_ERR_BAD_DEVICE_STATE );
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
    if(BDSP_Raaga_P_MsgType_eSyn == eMgsType)
    {
        ui32ResponseSize = BDSP_RAAGA_RESPONSE_SIZE_IN_BYTES;
    }
    else if(BDSP_Raaga_P_MsgType_eAsyn == eMgsType)
    {
        ui32ResponseSize = BDSP_RAAGA_ASYNC_RESPONSE_SIZE_IN_BYTES;
    }

    /* Checking if a msg is present */

    /* Case 1: if(readptr > writeptr)*/
    if (ui32maskReadAddr > ui32maskWriteAddr)
    {
        i32BytesToBeRead=(hMsgQueue->ui32EndAddr-ui32maskReadAddr)+
                (ui32maskWriteAddr-hMsgQueue->ui32BaseAddr);
    }

     /* Case 2:if(writeptr>readptr) */
    if (ui32maskWriteAddr>ui32maskReadAddr)
    {
        i32BytesToBeRead=ui32maskWriteAddr-ui32maskReadAddr;
    }

    /* Case 3: if readptr == writeptr */
    if (ui32maskWriteAddr == ui32maskReadAddr)
    {
        /*All messages have to be read*/
        i32BytesToBeRead = hMsgQueue->ui32EndAddr-hMsgQueue->ui32BaseAddr;
    }
     if (i32BytesToBeRead < 0)
     {
        BDBG_ERR(("The Message Queue is empty.No message is present."));
        BDBG_ASSERT(0);
        return BERR_TRACE(BDSP_ERR_BUFFER_EMPTY);
     }

        /* hMsgQueue->pBaseAddr has the base address in cache format */
    pvMsgQueueReadAddr = (void *)((uint8_t *)hMsgQueue->pBaseAddr + (ui32maskReadAddr - hMsgQueue->ui32BaseAddr ));



    /* Reading data in two chunks taking wrap-around into consideration  */
    if ( (ui32maskReadAddr > ui32maskWriteAddr)||
         (ui32maskReadAddr == ui32maskWriteAddr))
    {
        if(ui32ResponseSize> (hMsgQueue->ui32EndAddr-ui32maskReadAddr))
        {
            ui32chunk1=hMsgQueue->ui32EndAddr-ui32maskReadAddr;
            ui32chunk2=i32BytesToBeRead-ui32chunk1;
        }
        else
        {
            ui32chunk1=ui32ResponseSize;
            ui32chunk2=0;
        }
    }
    else
    {
        ui32chunk1=ui32ResponseSize;
        ui32chunk2=0;
    }


    /* Reading from chunk1 */
    for(i=0;i<(ui32chunk1/4);i++)
    {
        *((uint32_t *)pMsgBuf+i) = BDSP_P_MemRead32_isr(hMsgQueue->hHeap,
                                    (uint8_t * )pvMsgQueueReadAddr+(i*4));
        ui32dramReadAddr=ui32dramReadAddr+4;

    }

    /* Toggling the read pointer */
    if(ui32maskReadAddr + ui32chunk1 == hMsgQueue->ui32EndAddr)
    {
        ui32dramReadAddr=hMsgQueue->ui32BaseAddr;
        ui32maskReadAddr=ui32dramReadAddr;
    }


    /* Reading from chunk2 */
    if(ui32chunk2>0)
    {

        pvMsgQueueReadAddr = (void *)((uint8_t *)hMsgQueue->pBaseAddr + (ui32maskReadAddr - hMsgQueue->ui32BaseAddr ));

        for(i=0;i<(ui32chunk2/4);i++)
        {
            *((uint32_t *)pMsgBuf+i) = BDSP_P_MemRead32_isr(hMsgQueue->hHeap,
                (uint8_t *) pvMsgQueueReadAddr+(i*4));
            ui32dramReadAddr=ui32dramReadAddr+4;
        }
    }

    /* Updating read ptr in the Queue Attribute Structure */
    BDSP_Write32_isr(
        hMsgQueue->hRegister,
        BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR +
            (4 * 4 * hMsgQueue->i32FifoId) +
            BDSP_RAAGA_P_FIFO_READ_OFFSET + hMsgQueue->ui32DspOffset,
        ui32dramReadAddr);

    BDBG_MSG(("ui32dramReadAddr > %x",  ui32dramReadAddr));
    BDBG_MSG(("ui32dramWriteAddr > %x", ui32dramWriteAddr));

    /* Updating read ptr in the handle */
    hMsgQueue->ui32ReadAddr = ui32dramReadAddr;

    BDBG_LEAVE(BDSP_Raaga_P_GetMsg_isr);
    return err;

}

BERR_Code BDSP_Raaga_P_GetAlgorithmStatus(
    BMEM_Handle         hHeap,
    BDSP_Algorithm      eAlgorithm,
    void                *pStatusBuf,    /* [in] Config Buf Address */
    uint32_t            uiStatusBufSize,    /* [in] Config Buf Size */
    void               *pStatusBuffer,      /*[out]*/
    size_t              statusBufferSize
    )
{
    const BDSP_Raaga_P_AlgorithmInfo *pInfo;
    uint32_t *pStatusValid;

    BDBG_ENTER( BDSP_Raaga_P_GetAlgorithmStatus );

    if((pStatusBuffer == NULL) || (statusBufferSize ==0))
    {
        return BERR_SUCCESS;
    }

    pInfo = BDSP_Raaga_P_LookupAlgorithmInfo(eAlgorithm);

    if ( statusBufferSize != pInfo->statusBufferSize )
    {
        BDBG_ERR(("Status buffer size provided (%lu) does not match expected size (%lu) for algorithm %u (%s)",
                  (unsigned long)statusBufferSize, (unsigned long)pInfo->statusBufferSize, eAlgorithm, pInfo->pName));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    BDBG_ASSERT(statusBufferSize <= uiStatusBufSize);

    BDSP_P_CopyDataFromDram(hHeap, pStatusBuffer, pStatusBuf, statusBufferSize);

    if ( pInfo->statusValidOffset != 0xffffffff )
    {
        pStatusValid = (void *)((uint8_t *)pStatusBuffer + pInfo->statusValidOffset);
        if ( 0 != *pStatusValid )
        {
            BDBG_MSG(("Status buffer for algorithm %u (%s) marked invalid", eAlgorithm, pInfo->pName));
            return BDSP_ERR_BAD_DEVICE_STATE;   /* BERR_TRACE intentionally omitted */
        }
    }

    BDBG_LEAVE( BDSP_Raaga_P_GetAlgorithmStatus );

    return BERR_SUCCESS;
}

BERR_Code BDSP_Raaga_P_SetAlgorithmSettings(
    BMEM_Handle             hHeap,
    BDSP_Algorithm          eAlgorithm,
    void                    *pConfigBuf,    /* [in] Config Buf Address */
    uint32_t                uiConfigBufSize,    /* [in] Config Buf Size */
    const void             *pSettingsBuffer,
    size_t                  settingsBufferSize
    )
{
    const BDSP_Raaga_P_AlgorithmInfo *pInfo;

    BDBG_ENTER( BDSP_Raaga_P_SetAlgorithmSettings );

    if((pSettingsBuffer == NULL) || (settingsBufferSize ==0))
    {
        return BERR_SUCCESS;
    }

    pInfo = BDSP_Raaga_P_LookupAlgorithmInfo(eAlgorithm);

    if ( settingsBufferSize != pInfo->userConfigSize )
    {
        BDBG_ERR(("Settings buffer size provided (%lu) does not match expected size (%lu) for algorithm %u (%s)",
                  (unsigned long)settingsBufferSize, (unsigned long)pInfo->userConfigSize, eAlgorithm, pInfo->pName));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    BDBG_ASSERT(settingsBufferSize <= uiConfigBufSize);

    BDSP_P_CopyDataToDram(hHeap, (void *)pSettingsBuffer, pConfigBuf, settingsBufferSize);

    BDBG_LEAVE( BDSP_Raaga_P_SetAlgorithmSettings );

    return BERR_SUCCESS;
}


BERR_Code BDSP_Raaga_P_GetAlgorithmSettings(
    BMEM_Handle             hHeap,
    BDSP_Algorithm          eAlgorithm,
    void                    *pConfigBuf,        /* [in] Config Buf Address */
    uint32_t                uiConfigBufSize,    /* [in] Config Buf Size */
    void                   *pSettingsBuffer,
    size_t                  settingsBufferSize
    )
{
    const BDSP_Raaga_P_AlgorithmInfo *pInfo;

    BDBG_ENTER( BDSP_Raaga_P_GetAlgorithmSettings );

    if((pSettingsBuffer == NULL) || (settingsBufferSize ==0))
    {
        return BERR_SUCCESS;
    }

    pInfo = BDSP_Raaga_P_LookupAlgorithmInfo(eAlgorithm);

    if ( settingsBufferSize != pInfo->userConfigSize )
    {
        BDBG_ERR(("Settings buffer size provided (%lu) does not match expected size (%lu) for algorithm %u (%s)",
                  (unsigned long)settingsBufferSize, (unsigned long)pInfo->userConfigSize, eAlgorithm, pInfo->pName));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    BDBG_ASSERT(settingsBufferSize <= uiConfigBufSize);

    BDSP_P_CopyDataFromDram(hHeap,pSettingsBuffer,pConfigBuf,settingsBufferSize);

    BDBG_LEAVE( BDSP_Raaga_P_GetAlgorithmSettings );

    return BERR_SUCCESS;
}

BERR_Code BDSP_Raaga_P_GetFrameSyncTsmStageConfigParams(
    BMEM_Handle     hHeap,
    BDSP_Algorithm eAlgorithm,
    void            *pConfigBuf,    /* [in] Config Buf Address */
    uint32_t        uiConfigBufSize,     /* [in] Config Buf Size */
    void           *pSettingsBuffer,
    size_t          settingsBufferSize
    )
{
    BERR_Code err=BERR_SUCCESS;
    BDBG_ASSERT(pSettingsBuffer);
    BKNI_EnterCriticalSection();
    err = BDSP_Raaga_P_GetFrameSyncTsmStageConfigParams_isr(hHeap, eAlgorithm, pConfigBuf, uiConfigBufSize, pSettingsBuffer, settingsBufferSize);
    BKNI_LeaveCriticalSection();
    return err;
}

BERR_Code BDSP_Raaga_P_SetFrameSyncTsmStageConfigParams(
    BMEM_Handle         hHeap,
    BDSP_Algorithm      eAlgorithm,
    void                *pConfigBuf,    /* [in] Config Buf Address */
    uint32_t            uiConfigBufSize,     /* [in] Config Buf Size */
    const void         *pSettingsBuffer,
    size_t              settingsBufferSize
    )
{
    BERR_Code err=BERR_SUCCESS;
    BDBG_ASSERT(pSettingsBuffer);
    BKNI_EnterCriticalSection();
    err = BDSP_Raaga_P_SetFrameSyncTsmStageConfigParams_isr(hHeap, eAlgorithm, pConfigBuf, uiConfigBufSize, pSettingsBuffer, settingsBufferSize);
    BKNI_LeaveCriticalSection();
    return err;
}


BERR_Code BDSP_Raaga_P_GetFrameSyncTsmStageConfigParams_isr(
    BMEM_Handle     hHeap,
    BDSP_Algorithm eAlgorithm,
    void            *pConfigBuf,    /* [in] Config Buf Address */
    uint32_t        uiConfigBufSize,     /* [in] Config Buf Size */
    void           *pSettingsBuffer,
    size_t          settingsBufferSize
    )
{
    const BDSP_Raaga_P_AlgorithmInfo *pInfo;

    BDBG_ENTER( BDSP_Raaga_P_SetAlgorithmSettings );

    if((pSettingsBuffer == NULL) || (settingsBufferSize ==0))
    {
        return BERR_SUCCESS;
    }

    pInfo = BDSP_Raaga_P_LookupAlgorithmInfo_isrsafe(eAlgorithm);

    if ( settingsBufferSize != pInfo->idsConfigSize )
    {
        BDBG_ERR(("Datasync settings buffer size provided (%lu) does not match expected size (%lu) for algorithm %u (%s)",
                  (unsigned long)settingsBufferSize, (unsigned long)pInfo->userConfigSize, eAlgorithm, pInfo->pName));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    BDBG_ASSERT(settingsBufferSize <= uiConfigBufSize);

    BDSP_P_CopyDataFromDram_isr(hHeap, (void *)pSettingsBuffer, pConfigBuf, settingsBufferSize);

    BDBG_LEAVE( BDSP_Raaga_P_SetAlgorithmSettings );

    return BERR_SUCCESS;
}

BERR_Code BDSP_Raaga_P_SetFrameSyncTsmStageConfigParams_isr(
    BMEM_Handle         hHeap,
    BDSP_Algorithm      eAlgorithm,
    void                *pConfigBuf,    /* [in] Config Buf Address */
    uint32_t            uiConfigBufSize,     /* [in] Config Buf Size */
    const void         *pSettingsBuffer,
    size_t              settingsBufferSize
    )
{
    const BDSP_Raaga_P_AlgorithmInfo *pInfo;

    BDBG_ENTER( BDSP_Raaga_P_SetAlgorithmSettings );

    if((pSettingsBuffer == NULL) || (settingsBufferSize ==0))
    {
        return BERR_SUCCESS;
    }

    pInfo = BDSP_Raaga_P_LookupAlgorithmInfo_isrsafe(eAlgorithm);

    if ( settingsBufferSize != pInfo->idsConfigSize )
    {
        BDBG_ERR(("Datasync settings buffer size provided (%lu) does not match expected size (%lu) for algorithm %u (%s)",
                  (unsigned long)settingsBufferSize, (unsigned long)pInfo->userConfigSize, eAlgorithm, pInfo->pName));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    BDBG_ASSERT(settingsBufferSize <= uiConfigBufSize);

    BDSP_P_CopyDataToDram_isr(hHeap, (void *)pSettingsBuffer, pConfigBuf, settingsBufferSize);

    BDBG_LEAVE( BDSP_Raaga_P_SetAlgorithmSettings );

    return BERR_SUCCESS;
}

BERR_Code BDSP_Raaga_P_GetAsyncMsg_isr(
    BDSP_Raaga_P_MsgQueueHandle    hMsgQueue,  /*[in]*/
    void                        *pMsgBuf,   /*[in]*/
    unsigned int                *puiNumMsgs /*[out]*/
    )
{
    BERR_Code err=BERR_SUCCESS;

    raaga_dramaddr  ui32dramReadAddr=0;
    raaga_dramaddr  ui32dramWriteAddr=0;
    raaga_dramaddr  ui32maskReadAddr=0;
    raaga_dramaddr  ui32maskWriteAddr=0;
    uint32_t ui32chunk1=0,ui32chunk2=0,i = 0;
    int32_t  i32BytesToBeRead=0;
    uint32_t ui32ResponseSize = 0;
    void     *pvMsgQueueReadAddr=NULL;
    unsigned int uiMsgIndex = 0, uiContMsgs = 0, uiMoreMsgs = 0;

    BDBG_ENTER(BDSP_Raaga_P_GetMsg);

    BDBG_ASSERT(hMsgQueue);
    BDBG_ASSERT(pMsgBuf);
    BDBG_ASSERT(puiNumMsgs);
    BSTD_UNUSED(ui32chunk1);
    BSTD_UNUSED(ui32chunk2);

    *puiNumMsgs = 0;

    ui32dramReadAddr = BDSP_Read32_isr(
                        hMsgQueue->hRegister,
                        BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR +
                            (4 * 4 * hMsgQueue->i32FifoId) +
                            BDSP_RAAGA_P_FIFO_READ_OFFSET +
                            hMsgQueue->ui32DspOffset);

    ui32dramWriteAddr = BDSP_Read32_isr(
                        hMsgQueue->hRegister,
                        BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR +
                            (4 * 4 * hMsgQueue->i32FifoId) +
                            BDSP_RAAGA_P_FIFO_WRITE_OFFSET +
                            hMsgQueue->ui32DspOffset);


    ui32maskReadAddr  = ui32dramReadAddr;
    ui32maskWriteAddr = ui32dramWriteAddr;

     /*Sanity check*/
    /* checking write ptr boundness -
        if((writeptr>endaddr)|(writeptr<baseaddr)) write ptr not within bound */

    if ( (ui32maskWriteAddr>hMsgQueue->ui32EndAddr)||
         (ui32maskWriteAddr<hMsgQueue->ui32BaseAddr))
    {
        BDBG_ERR(("Write pointer not within bounds in Message Queue"));
        BDBG_ASSERT(0);
        return BERR_TRACE(BDSP_ERR_BAD_DEVICE_STATE);
    }


    /* checking read ptr boundness -
        if((readptr>endaddr)|(readptr<baseaddr)) read ptr is not within bound */
    if ( (ui32maskReadAddr>hMsgQueue->ui32EndAddr)||
         (ui32maskReadAddr<hMsgQueue->ui32BaseAddr))
    {
        BDBG_ERR(("Read pointer not within bounds in Message Queue"));
        BDBG_ASSERT(0);
        return BERR_TRACE(BDSP_ERR_BAD_DEVICE_STATE);
    }

    /*Updating write ptr */
    hMsgQueue->ui32WriteAddr = ui32maskWriteAddr;

    /* checking read ptrs to see if they are the same */
    if ( (hMsgQueue->ui32ReadAddr) != ui32maskReadAddr)
    {
        BDBG_ERR(("Read pointer corrupted in the Message Queue"));
        BDBG_ASSERT(0);
        return BERR_TRACE(BDSP_ERR_BAD_DEVICE_STATE );
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
    ui32ResponseSize = BDSP_RAAGA_ASYNC_RESPONSE_SIZE_IN_BYTES;

    /* Checking if a message is present */

    /* Case1: if(readptr>writeptr) */
    if(ui32maskReadAddr > ui32maskWriteAddr)
    {

        i32BytesToBeRead=(hMsgQueue->ui32EndAddr-ui32maskReadAddr)+
                (ui32maskWriteAddr-hMsgQueue->ui32BaseAddr);

        uiContMsgs = (hMsgQueue->ui32EndAddr-ui32maskReadAddr)/
                        BDSP_RAAGA_ASYNC_RESPONSE_SIZE_IN_BYTES;

        uiMoreMsgs = (ui32maskWriteAddr-hMsgQueue->ui32BaseAddr)/
                        BDSP_RAAGA_ASYNC_RESPONSE_SIZE_IN_BYTES;
    }

     /* Case2: if(writeptr>readptr) */
    if(ui32maskWriteAddr>ui32maskReadAddr)
    {
        i32BytesToBeRead = ui32maskWriteAddr-ui32maskReadAddr;
        uiContMsgs = i32BytesToBeRead / BDSP_RAAGA_ASYNC_RESPONSE_SIZE_IN_BYTES;
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
    *puiNumMsgs = i32BytesToBeRead/BDSP_RAAGA_ASYNC_RESPONSE_SIZE_IN_BYTES;

    pvMsgQueueReadAddr = (void *)((uint8_t *)hMsgQueue->pBaseAddr + (ui32maskReadAddr - hMsgQueue->ui32BaseAddr ));

    for(uiMsgIndex = 0; uiMsgIndex < uiContMsgs; uiMsgIndex++)
    {
        for(i=0; i<(ui32ResponseSize/4); i++)
        {
            *((uint32_t *)pMsgBuf+(uiMsgIndex * BDSP_RAAGA_ASYNC_RESPONSE_SIZE_IN_BYTES/4)+i)
                = BDSP_P_MemRead32_isr(hMsgQueue->hHeap,
                    (uint8_t * )pvMsgQueueReadAddr+(i*4) +
                        (uiMsgIndex * BDSP_RAAGA_ASYNC_RESPONSE_SIZE_IN_BYTES));
        }

       ui32dramReadAddr +=  BDSP_RAAGA_ASYNC_RESPONSE_SIZE_IN_BYTES;

    /*updating read ptr in the Queue Attribute Structure*/
    BDSP_Write32_isr(
        hMsgQueue->hRegister,
        BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR +
            (4 * 4 * hMsgQueue->i32FifoId) +
            BDSP_RAAGA_P_FIFO_READ_OFFSET + hMsgQueue->ui32DspOffset,
        ui32dramReadAddr
        );
    }

    pvMsgQueueReadAddr = hMsgQueue->pBaseAddr;


    for(uiMsgIndex = 0; uiMsgIndex < uiMoreMsgs; uiMsgIndex++)
    {
        for(i=0;i<(ui32ResponseSize/4);i++)
        {
            *((uint32_t *)pMsgBuf+((uiMsgIndex+uiContMsgs) * BDSP_RAAGA_ASYNC_RESPONSE_SIZE_IN_BYTES/4)+i)
                =BDSP_P_MemRead32_isr(hMsgQueue->hHeap,(uint8_t *)pvMsgQueueReadAddr+(i*4)+(uiMsgIndex * BDSP_RAAGA_ASYNC_RESPONSE_SIZE_IN_BYTES));
        }
        ui32dramReadAddr = hMsgQueue->ui32BaseAddr +
                            (uiMsgIndex+1)*
                            BDSP_RAAGA_ASYNC_RESPONSE_SIZE_IN_BYTES;

    /* Updating read ptr in the Queue Attribute Structure */
    BDSP_Write32(
        hMsgQueue->hRegister,
        BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR +
            (4 * 4 * hMsgQueue->i32FifoId) +
            BDSP_RAAGA_P_FIFO_READ_OFFSET + hMsgQueue->ui32DspOffset,
        ui32dramReadAddr
        );
    }
    /*updating read ptr in the handle*/
    hMsgQueue->ui32ReadAddr = ui32dramReadAddr;

    BDBG_LEAVE(BDSP_Raaga_P_GetMsg);
    return err;

}


BERR_Code BDSP_Raaga_P_GetVideoMsg_isr(BDSP_Raaga_P_MsgQueueHandle  hMsgQueue,/*[in]*/
                                     uint32_t *pMsgBuf,bool bReadUpdate)

{
    BERR_Code err=BERR_SUCCESS;

    raaga_dramaddr  ui32dramReadAddr=0;
    raaga_dramaddr  ui32dramWriteAddr=0;
    raaga_dramaddr  ui32maskReadAddr=0;
    raaga_dramaddr  ui32maskWriteAddr=0;
    int32_t i32BytesToBeRead=0;
    void *pvMsgQueueReadAddr=NULL;

    BDBG_ASSERT(hMsgQueue);
    BDBG_ASSERT(pMsgBuf);


    ui32dramReadAddr=BDSP_Read32_isr(hMsgQueue->hRegister,BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR + 4 * 4 * hMsgQueue->i32FifoId +
                    BDSP_RAAGA_P_FIFO_READ_OFFSET + hMsgQueue->ui32DspOffset);

    ui32dramWriteAddr=BDSP_Read32_isr(hMsgQueue->hRegister,BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR + 4 * 4 * hMsgQueue->i32FifoId +
                    BDSP_RAAGA_P_FIFO_WRITE_OFFSET + hMsgQueue->ui32DspOffset);

    BDBG_MSG(("ui32dramReadAddr > %x", ui32dramReadAddr));
    BDBG_MSG(("ui32dramWriteAddr > %x", ui32dramWriteAddr));
    ui32maskReadAddr=ui32dramReadAddr;
    ui32maskWriteAddr=ui32dramWriteAddr;

     /*Sanity check*/
    /* checking write ptr boundness- if((writeptr>endaddr)|(writeptr<baseaddr)) write ptr is not within bound*/
    if((ui32maskWriteAddr>hMsgQueue->ui32EndAddr)||(ui32maskWriteAddr<hMsgQueue->ui32BaseAddr))
    {
            BDBG_ERR(("Write pointer not within bounds in Message Queue"));
            err = BERR_TRACE(BDSP_ERR_BAD_DEVICE_STATE);
            goto end;
    }


    /* checking read ptr boundness- if((readptr>endaddr)|(readptr<baseaddr)) read ptr is not within bound*/
    if((ui32maskReadAddr>hMsgQueue->ui32EndAddr)||(ui32maskReadAddr<hMsgQueue->ui32BaseAddr))
    {
            BDBG_ERR(("Read pointer not within bounds in Message Queue"));
            err = BERR_TRACE(BDSP_ERR_BAD_DEVICE_STATE);
            goto end;
    }

    /*Updating write ptr */
    hMsgQueue->ui32WriteAddr= ui32maskWriteAddr;

    /* checking read ptrs to see if they are the same */
    if((hMsgQueue->ui32ReadAddr)!=ui32maskReadAddr)
    {
            BDBG_ERR(("Read pointer corrupted in the Message Queue"));
            err = BERR_TRACE(BDSP_ERR_BAD_DEVICE_STATE);
            goto end;
    }

    /* End of Sanity Check */

    /*******************************************************************
   Different cases:

    If maskreadptr>maskwriteptr
     ReadPtrMSB   WritePtrMSB
        0                   0                   This condn. is not possible
        0                   1                   (end-read)+(write-base)
        1                   0                     (end-read)+(write-base)
        1                   1                      this condn. is not possible

  If maskwriteptr>maskreadptr
   ReadptrMSB   WritePtrMSB
    0                   0               write-read
    0                   1                  this condn. not possible
    1                   0                  this condn. not possible
    1                   1                  write-read

  If maskreadptr==maskwriteptr
  If the toggle bits are the same,no message to read
  If the toggle bits are different all the messages have to be read


 ***********************************************************************/
    /*Condn. for reading messages from the message queue into the message buffer*/
    /* If no msg is to be read, generate a BRAP_ERR_BUFFER_EMPTY error(new error defined in brap.h)*/

    /* Checking if a msg is present */

    /* Case1:if(readptr>writeptr)*/
    if(ui32maskReadAddr > ui32maskWriteAddr)
    {
            i32BytesToBeRead=(hMsgQueue->ui32EndAddr-ui32maskReadAddr)+
                                            (ui32maskWriteAddr-hMsgQueue->ui32BaseAddr);
    }

    /* Case2:if(writeptr>readptr) */
    if(ui32maskWriteAddr>ui32maskReadAddr)
    {
        i32BytesToBeRead=ui32maskWriteAddr-ui32maskReadAddr;
    }

    /*Case 3:if readptr == writeptr */
    if(ui32maskWriteAddr ==ui32maskReadAddr)
    {
        /*All messages have to be read*/
        i32BytesToBeRead=0;
    }
    if(i32BytesToBeRead == 0)
    {
        BDBG_ERR(("The Message Queue is empty.No message is present."));
        goto end;
    }

    pvMsgQueueReadAddr = (void *)((uint8_t *)hMsgQueue->pBaseAddr + (ui32maskReadAddr - hMsgQueue->ui32BaseAddr ));

    /*Reading Message from the message queue into the message buffer*/
    *pMsgBuf=BDSP_P_MemRead32_isr(hMsgQueue->hHeap,pvMsgQueueReadAddr);

    BDBG_MSG(("In BRAP_P_GetMsg_isr *pMsgBuf = 0x%x\n",*pMsgBuf));

    if ((bReadUpdate == true)&&(i32BytesToBeRead!=0))
    {
        ui32dramReadAddr=ui32dramReadAddr+4;
        if(ui32dramReadAddr==hMsgQueue->ui32EndAddr)
        ui32dramReadAddr=hMsgQueue->ui32BaseAddr;

        /*updating read ptr in the Queue Attribute Structure*/
        BDSP_Write32_isr(hMsgQueue->hRegister,BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR + 4 * 4 * hMsgQueue->i32FifoId +
        BDSP_RAAGA_P_FIFO_READ_OFFSET + hMsgQueue->ui32DspOffset, ui32dramReadAddr);
        BDBG_MSG(("ui32dramReadAddr > %x", ui32dramReadAddr));
        BDBG_MSG(("ui32dramWriteAddr > %x", ui32dramWriteAddr));

        /*updating read ptr in the handle*/
        hMsgQueue->ui32ReadAddr = ui32dramReadAddr;
    }

end:
    return err;

}


BERR_Code BDSP_Raaga_P_WriteVideoMsg_isr(BDSP_Raaga_P_MsgQueueHandle   hMsgQueue/*[in]*/,
                                         void *pMsgBuf, /*[in]*/
                                         unsigned int uiBufSize/*[in]*/)
{
    BERR_Code err=BERR_SUCCESS;
    unsigned int uiFreeSpace=0;
    raaga_dramaddr  ui32dramReadAddr=0;
    raaga_dramaddr  ui32dramWriteAddr=0;
    raaga_dramaddr  ui32maskReadAddr=0;
    raaga_dramaddr  ui32maskWriteAddr=0;
    void *pvMsgQueueWriteAddr=NULL;

    BDBG_ASSERT(hMsgQueue);
    BDBG_ASSERT(pMsgBuf);

    ui32dramReadAddr=BDSP_Read32_isr(hMsgQueue->hRegister,BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR + 4 * 4 * hMsgQueue->i32FifoId +
                    BDSP_RAAGA_P_FIFO_READ_OFFSET + hMsgQueue->ui32DspOffset);

    ui32dramWriteAddr=BDSP_Read32_isr(hMsgQueue->hRegister,BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR + 4 * 4 * hMsgQueue->i32FifoId +
                    BDSP_RAAGA_P_FIFO_WRITE_OFFSET + hMsgQueue->ui32DspOffset);

        ui32maskReadAddr=ui32dramReadAddr;
        ui32maskWriteAddr=ui32dramWriteAddr;

        /*Sanity check*/
    /*Checking boundness of read pointer- if((readptr>endaddr) OR (readptr<baseaddr)) read ptr not within bound*/
    if((ui32maskReadAddr>hMsgQueue->ui32EndAddr)||(ui32maskReadAddr<hMsgQueue->ui32BaseAddr))
    {
            BDBG_ERR(("Read pointer not within bounds in Message Queue"));
            err = BERR_TRACE(BDSP_ERR_BAD_DEVICE_STATE);
            goto end;
    }

    /*Checking boundness of write pointer - if((writeptr>endaddr) OR (writeptr<baseaddr))  write ptr not within bound*/
    if((ui32maskWriteAddr>hMsgQueue->ui32EndAddr)||(ui32maskWriteAddr<hMsgQueue->ui32BaseAddr))
    {
            BDBG_ERR(("Write pointer not within bounds in Message Queue"));
            err = BERR_TRACE(BDSP_ERR_BAD_DEVICE_STATE);
            goto end;
    }

    /*update read ptr */
    hMsgQueue->ui32ReadAddr= ui32maskReadAddr;

    /* checking write ptrs */
    if((hMsgQueue->ui32WriteAddr)!=ui32maskWriteAddr)
    {
            BDBG_ERR(("Write pointer corrupted in the Message Queue"));
            err = BERR_TRACE(BDSP_ERR_BAD_DEVICE_STATE);
            goto end;
    }


/* End of Sanity Check */
/*******************************************************************
   Different cases:

    If maskwriteptr> maskreadptr
     ReadPtrMSB   WritePtrMSB   freespace
        0                   0          freespace=((endaddr-writeptr)+(readptr-baseaddr));
        0                   1            this condn not possible
        1                   0            this condn. not possible
        1                   1            freespace=((endaddr-writeptr)+(readptr-baseaddr));

  If maskreadptr>maskwriteptr
   ReadptrMSB   WritePtrMSB  freespace
    0                   0             this condn. not possible
    0                   1             read-write
    1                   0             read-write
    1                   1             this condn. not possible

  If maskreadptr==maskwriteptr
  If the toggle bits are the same,then the buffer is empty
  If the toggle bits are different,the buffer is full
 ***********************************************************************/


    /*Calc free space in the message queue*/

    /* Case1:if(maskwriteptr>maskreadptr)*/
    if(ui32maskWriteAddr > ui32maskReadAddr)
    {
        uiFreeSpace=((hMsgQueue->ui32EndAddr-ui32maskWriteAddr)+
                            (ui32maskReadAddr-hMsgQueue->ui32BaseAddr))-uiBufSize;
    }


    /* Case2:if(maskreadptr>maskwriteptr) */
    if(ui32maskReadAddr>ui32maskWriteAddr)
    {
        uiFreeSpace=(ui32maskReadAddr-ui32maskWriteAddr)-uiBufSize;
    }


    /* Case3:if(maskreadptr==maskwriteptr) */
    if(ui32maskReadAddr==ui32maskWriteAddr)
    {
        /* The buffer is empty */
        uiFreeSpace=(hMsgQueue->ui32EndAddr-hMsgQueue->ui32BaseAddr)-uiBufSize;
    }

    /*Generate BUFFER_FULL error  when there is no space for the message to be written into the message queue*/
    /* BRAP_ERR_BUFFER_FULL is a new error that has been defined in brap.h */

    if(uiFreeSpace <= 0)/*Considering the buffer has only one message (i.e) uiBufSize=MESSAGE_SIZE */
    {
        BDBG_ERR(("No Free space in the buffer.No new messages can be written"));
        err =  BERR_TRACE(BDSP_ERR_BUFFER_FULL);
    }

    /*writing msgs into the message queue*/
    /*Assume:the Message buffer always has only one message*/

    BDBG_MSG(("Buffer size should be a multiple of 4"));
    BDBG_ASSERT(!(uiBufSize%4));
    BDBG_MSG(("uiBufSize > %d", uiBufSize));


    pvMsgQueueWriteAddr = (void *)((uint8_t *)hMsgQueue->pBaseAddr + (ui32maskWriteAddr - hMsgQueue->ui32BaseAddr ));


    /*Writing into Message queue*/
    BDBG_MSG(("In BRAP_P_WriteMsg_isr *(uint32_t *)pMsgBuf > 0x%x\n", *((uint32_t *)pMsgBuf)));
    BDSP_P_MemWrite32_isr(hMsgQueue->hHeap,pvMsgQueueWriteAddr, *((uint32_t *)pMsgBuf));
    ui32dramWriteAddr=ui32dramWriteAddr+4;

    /* Taking wrap-around into consideration*/
    if(ui32dramWriteAddr==hMsgQueue->ui32EndAddr)
    ui32dramWriteAddr=hMsgQueue->ui32BaseAddr;

    /*updating write ptr in the Queue Attribute Structure*/
    BDSP_Write32_isr(hMsgQueue->hRegister,BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR + 4 * 4 * hMsgQueue->i32FifoId +
    BDSP_RAAGA_P_FIFO_WRITE_OFFSET + hMsgQueue->ui32DspOffset, ui32dramWriteAddr);
    BDBG_MSG(("ui32dramReadAddr > %x", ui32dramReadAddr));
    BDBG_MSG(("ui32dramWriteAddr > %x", ui32dramWriteAddr));

    /*updating write ptr in the handle*/
    hMsgQueue->ui32WriteAddr=ui32dramWriteAddr;
    BDBG_LEAVE(BRAP_P_WriteMsg_isr);

    end:
    return err;
  }
