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

#include "bdsp_raaga.h"
#include "bdsp_raaga_mm_priv.h"
#include "bdsp_raaga_priv.h"
#include "bdsp_raaga_fwdownload_priv.h"
#include "bdsp_raaga_cmdresp_priv.h"
#include "bdsp_raaga_fw.h"
#include "bdsp_raaga_fw_settings.h"

BDBG_MODULE(bdsp_raaga_mm);

/*****************************************************************************
Function Name: BDSP_MM_P_GetFwMemRequired

            If the algorithm is supported, size for that algo is calculated
                and the required cumulative memory requirement is found out.

*****************************************************************************/
BERR_Code BDSP_Raaga_P_GetFwMemRequired(
        const BDSP_RaagaSettings  *pSettings,
        BDSP_Raaga_P_DwnldMemInfo *pDwnldMemInfo,      /*[out]*/
        void                      *pImg,
        bool                       UseBDSPMacro,
        const BDSP_RaagaUsageOptions *pUsage
)
{
    BERR_Code ret= BERR_SUCCESS;
    unsigned i=0, j=0, size=0;
    const BDSP_Raaga_P_AlgorithmInfo *pInfo;
    BDSP_Raaga_P_AlgoTypeImgBuf * pAlgoTypeBuf;
    BDSP_RaagaImgCacheEntry     * pImgCache;
    unsigned systemImgSize = 0;
    BDSP_Algorithm algorithm;
    size_t totalSize = 0;
    uint32_t ui32AlgorithmSize = 0;
    const BDSP_Raaga_P_AlgorithmSupportInfo *pSupportInfo;

    BDSP_AlgorithmType        Algotype;
    BDSP_AF_P_sALGO_EXEC_INFO algoExecInfo;

    pImgCache    = (BDSP_RaagaImgCacheEntry *)pImg;
    pAlgoTypeBuf = &pDwnldMemInfo->AlgoTypeBufs[0];
    BKNI_Memset( pImgCache, 0, (sizeof(BDSP_RaagaImgCacheEntry)*BDSP_IMG_ID_MAX));

    totalSize = BDSP_Raaga_P_AssignAlgoSizes(
                    pSettings->pImageInterface,
                    pSettings->pImageContext,
                    (BDSP_RaagaImgCacheEntry * )pImg,
                    pUsage,
                    UseBDSPMacro);

    systemImgSize =0;
    for ( i = 0; i < BDSP_SystemImgId_eMax; i++ )
    {
        systemImgSize += pImgCache[i].size;
    }

    for ( algorithm = 0; algorithm < BDSP_Algorithm_eMax; algorithm++ )
    {
        if(UseBDSPMacro)
        {
            pInfo     = BDSP_Raaga_P_LookupAlgorithmInfo(algorithm);
            Algotype  = pInfo->type;
            algoExecInfo = pInfo->algoExecInfo;
        }
        else
        {
            pSupportInfo = BDSP_Raaga_P_LookupAlgorithmSupported(algorithm,pUsage->DolbyCodecVersion);
            Algotype  = pSupportInfo->type;
            algoExecInfo = pSupportInfo->algoExecInfo;
        }

        ui32AlgorithmSize = 0;
        for ( i = 0; i < algoExecInfo.NumNodes; i++ )
        {
            BDSP_AF_P_AlgoId algoId = algoExecInfo.eAlgoIds[i];
            if ( algoId < BDSP_AF_P_AlgoId_eMax )
            {
                ui32AlgorithmSize += pImgCache[BDSP_IMG_ID_CODE(algoId)].size;
                ui32AlgorithmSize += pImgCache[BDSP_IMG_ID_IFRAME(algoId)].size;
                ui32AlgorithmSize += pImgCache[BDSP_IMG_ID_TABLE(algoId)].size;

            }
        }


        /* Fills up the meminfo structure with the sizes. This structure will be used for allocating memory
        only in case of non-secure mode. For AlgoType AudioProcessing we are going to download all the
        algorithms even in non-secure mode. So add up all allocations */
        if(Algotype == BDSP_AlgorithmType_eAudioProcessing){
            pAlgoTypeBuf[Algotype].ui32Size += ui32AlgorithmSize;
        }
        else
        {
            if(pAlgoTypeBuf[Algotype].ui32Size < ui32AlgorithmSize){
                pAlgoTypeBuf[Algotype].ui32Size  = ui32AlgorithmSize;
            }
        }
    }

    pDwnldMemInfo->ui32TotalSupportedBinSize = totalSize;
    pDwnldMemInfo->IsImagePreLoaded = false;
    pDwnldMemInfo->ui32AllocatedBinSize = 0;
    pAlgoTypeBuf = &pDwnldMemInfo->AlgoTypeBufs[0];
    pDwnldMemInfo->ui32AllocatedBinSize = systemImgSize; /* Adding the static system image size */

    for(i=0;i<BDSP_AlgorithmType_eMax ; i++ )
    {
        size = pAlgoTypeBuf[i].ui32Size ;
        for(j=0; j< pSettings->maxAlgorithms[i]; j++ ) /* In case pDevice->settings.maxAlgorithms[i]  is corrupted better use a define */
        {
            pDwnldMemInfo->ui32AllocatedBinSize += size;
        }
    }

    BDBG_MSG((" Local worst case allocation size =  %d, Total size = %d", pDwnldMemInfo->ui32AllocatedBinSize, pDwnldMemInfo->ui32TotalSupportedBinSize));

    if((pDwnldMemInfo->ui32AllocatedBinSize >=  pDwnldMemInfo->ui32TotalSupportedBinSize )||
        (pSettings->preloadImages == true))
    {
        pDwnldMemInfo->IsImagePreLoaded = true;
        pDwnldMemInfo->ui32AllocatedBinSize = pDwnldMemInfo->ui32TotalSupportedBinSize;
    }
    BDBG_MSG((" Is image preloaded = %d", pDwnldMemInfo->IsImagePreLoaded));

#if (BCHP_CHIP == 7278)
    pDwnldMemInfo->ui32SystemImgSize = systemImgSize;
    /* deduct the system images size to get memory size allocated for algorithms */
    pDwnldMemInfo->ui32AllocatedAlgoSizes = pDwnldMemInfo->ui32AllocatedBinSize - pDwnldMemInfo->ui32SystemImgSize;
#endif

    pDwnldMemInfo->ui32AllocwithGuardBand =  pDwnldMemInfo->ui32AllocatedBinSize;
    if(pSettings->preloadImages == true)
        pDwnldMemInfo->ui32AllocwithGuardBand += BDSP_CODE_DWNLD_GUARD_BAND_SIZE;

    return ret;
}

static BERR_Code BDSP_Raaga_P_GetFwMemRequirement(BDSP_Raaga *pDevice)
{
    BDSP_Raaga_P_DwnldMemInfo *pDwnldMemInfo;
	BERR_Code errCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(pDevice, BDSP_Raaga);

    pDwnldMemInfo = &pDevice->memInfo.sDwnldMemInfo;

    errCode = BDSP_Raaga_P_GetFwMemRequired(&(pDevice->settings),pDwnldMemInfo,(void *)&(pDevice->imgCache[0]),true,NULL);
	if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Raaga_P_GetFwMemRequirement: Unable to gather the FW Memory requirement"));
		goto end;
	}

#if(BCHP_CHIP != 7278)
	errCode = BDSP_MMA_P_AllocateAlignedMemory(pDevice->memHandle, pDwnldMemInfo->ui32AllocwithGuardBand, &(pDwnldMemInfo->ImgBuf),BDSP_MMA_Alignment_32bit);
#else
	/* physical address shall be 512 byte aligned */
    errCode = BDSP_MMA_P_AllocateAlignedMemory(pDevice->memHandle, pDwnldMemInfo->ui32AllocwithGuardBand, &(pDwnldMemInfo->ImgBuf),BDSP_MMA_Alignment_4096bit);
#ifdef FIREPATH_BM
    BDBG_MSG(("pDevice->memHandle : %x", pDevice->memHandle));
    BDBG_MSG(("pDevice->regHandle : %x", pDevice->regHandle));
    BDBG_MSG(("hBlock : %x", pDwnldMemInfo->ImgBuf.hBlock));
    BDBG_MSG(("pAddr : %x", pDwnldMemInfo->ImgBuf.pAddr));
    BDBG_MSG(("offset : %x", pDwnldMemInfo->ImgBuf.offset));
#endif
#endif

    if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("Cannot allocate the requested IMG Memory"));
		return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
	}
	BDBG_MSG(("**************Allocated size %d, Bin Size = %d, ptr = %p Preloaded = %d", pDwnldMemInfo->ui32AllocwithGuardBand,pDwnldMemInfo->ui32AllocatedBinSize,pDwnldMemInfo->ImgBuf.pAddr,pDwnldMemInfo->IsImagePreLoaded));
	pDevice->fwHeapSize = pDwnldMemInfo->ui32AllocwithGuardBand;
	pDevice->pFwHeapMemory = pDwnldMemInfo->ImgBuf.pAddr;
end:
    return errCode;
}

/*#define DWNLD_BUF_DBG*/
void BDSP_Raaga_P_FreeFwExec(   void *pDeviceHandle)
{
    BDSP_Raaga *pDevice = (BDSP_Raaga *)pDeviceHandle;
    unsigned imageId=0;
    BDSP_Raaga_P_AlgoTypeImgBuf *pAlgoTypeBuf;
    unsigned i=0,j=0;

    BDBG_ENTER(BDSP_Raaga_P_FreeFwExec);

    BDBG_OBJECT_ASSERT(pDevice, BDSP_Raaga);

    if(pDevice->memInfo.sDwnldMemInfo.ImgBuf.pAddr== NULL )
        return;

#ifdef FWDWNLD_DBG
    BDSP_Raaga_P_FwDwnldBuf_Dump(pDeviceHandle);
#endif

    BDSP_MMA_P_FreeMemory(&pDevice->memInfo.sDwnldMemInfo.ImgBuf);
    /* size element of both these structures are reused during watchdog so we dont zero it out here */
    for (imageId=0; imageId<BDSP_IMG_ID_MAX; imageId++)
    {
        pDevice->imgCache[imageId].pMemory = NULL;
        pDevice->imgCache[imageId].offset= 0;
    }
    for(i=0;i<BDSP_AlgorithmType_eMax ; i++ )
    {
        pAlgoTypeBuf = &pDevice->memInfo.sDwnldMemInfo.AlgoTypeBufs[i];
        for(j=0; j< BDSP_RAAGA_MAX_DWNLD_BUFS ; j++ )
        {
            BDSP_INIT_DWNLDBUF( &pAlgoTypeBuf->DwnldBufUsageInfo[j]);
        }
    }

    BDBG_LEAVE(BDSP_Raaga_P_FreeFwExec);

}


BERR_Code BDSP_Raaga_P_FreeInitMemory(
    void *pDeviceHandle
    )
{
    BDSP_Raaga *pDevice = (BDSP_Raaga *)pDeviceHandle;
    unsigned i=0,j=0;
    BERR_Code   err=BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(pDevice, BDSP_Raaga);

    /* Nothing complex needs to be done to release the FW images, just destroy the FW heap and free the memory */
    BDSP_Raaga_P_FreeFwExec(pDeviceHandle);

#if (BCHP_CHIP != 7278)
    for(i=0;i<pDevice->numDsp;i++)
    {
		BDSP_MMA_P_FreeMemory(&pDevice->memInfo.cmdQueueParams[i].Queue);
		BDSP_MMA_P_FreeMemory(&pDevice->memInfo.genRspQueueParams[i].Queue);
		BDSP_MMA_P_FreeMemory(&pDevice->memInfo.DSPFifoAddrStruct[i]);

		for (j = 0; j < BDSP_Raaga_DebugType_eLast; j++)
		{
			BDSP_MMA_P_FreeMemory(&pDevice->memInfo.FwDebugBuf[i][j].Buffer);
			if(j == BDSP_Raaga_DebugType_eTargetPrintf)
			{
				BDSP_MMA_P_FreeMemory(&pDevice->memInfo.TargetPrintBuffer[i]);
			}
		}
		BDSP_MMA_P_FreeMemory(&pDevice->memInfo.sRaagaSwapMemoryBuf[i].Buffer);
    }
#else
    /* For OctaveM2, we shall free the large rw memory chunk from which other buffers are allocated
     * Reset the allocated and used memory sizes as well */
    BDSP_MMA_P_FreeMemory(&pDevice->memInfo.sRaagaRWMemoryBuf.Buffer);
    pDevice->memInfo.sRaagaRWMemoryBuf.ui32Size = 0;
    pDevice->memInfo.ui32UsedRWMemsize = 0;
    BSTD_UNUSED(i);
    BSTD_UNUSED(j);
#endif /* (BCHP_CHIP != 7278) */
    return err;
}

BERR_Code BDSP_Raaga_P_AssignMem_DwnldBuf(void *pDeviceHandle, BDSP_MMA_Memory *pMemory)
{
    BDSP_Raaga *pDevice = (BDSP_Raaga *)pDeviceHandle;
    uint32_t i=0, j=0;
    BDSP_Raaga_P_AlgoTypeImgBuf *pAlgoBufInfo;
    BERR_Code retVal = BERR_SUCCESS;
    BDSP_Raaga_P_DwnldBufUsageInfo *pDwnldBufInfo;

    BDBG_ENTER(BDSP_Raaga_P_AssignMem_DwnldBuf);
    BDBG_OBJECT_ASSERT(pDevice, BDSP_Raaga);

    if(pMemory->pAddr == NULL ){
        retVal = BERR_INVALID_PARAMETER;
        goto error;
    }

    pAlgoBufInfo = &pDevice->memInfo.sDwnldMemInfo.AlgoTypeBufs[0];

    for(i=0;i<BDSP_AlgorithmType_eMax ; i++ )
    {
        for(j=0; j< pDevice->settings.maxAlgorithms[i] ; j++ )
        {
            pDwnldBufInfo = &pAlgoBufInfo[i].DwnldBufUsageInfo[j];
            if( pAlgoBufInfo[i].ui32Size == 0 )
            {
                pDwnldBufInfo->Memory.pAddr = NULL;
                continue;
            }

            pDwnldBufInfo->Memory = *pMemory;
            pMemory->pAddr = (void *)((uint8_t *)pMemory->pAddr + pAlgoBufInfo[i].ui32Size);
			pMemory->offset= pMemory->offset + pAlgoBufInfo[i].ui32Size;

            if(pDwnldBufInfo->Memory.pAddr == NULL)
            {
                BDBG_ERR((" NULL pointer allocated for algotype %u of size %u buffer num %u ", i ,pAlgoBufInfo[i].ui32Size,j));
                retVal = BERR_INVALID_PARAMETER;
                goto error;
            }
            pDwnldBufInfo->numUser = 0;

            BDBG_MSG((" Assigned Addr %p  size %u AlgoType %d", pDwnldBufInfo->Memory.pAddr, pAlgoBufInfo[i].ui32Size,i ));
        }
    }

    if(pMemory->pAddr > (void *)((uint8_t *)pDevice->memInfo.sDwnldMemInfo.ImgBuf.pAddr+ pDevice->memInfo.sDwnldMemInfo.ui32AllocatedBinSize))
    {
        BDBG_ERR(("Used memory more than allocated memory.MemInfo size parameter might be corrupted.\
                    Used till %p Allocated till %p -- exclusive", pMemory->pAddr, \
                    (void *)((uint8_t *)pDevice->memInfo.sDwnldMemInfo.ImgBuf.pAddr+ pDevice->memInfo.sDwnldMemInfo.ui32AllocatedBinSize)));
        retVal = BERR_INVALID_PARAMETER;
        goto error;
    }

error:
    BDBG_LEAVE(BDSP_Raaga_P_AssignMem_DwnldBuf);
    return retVal;
}

static void BDSP_Raaga_P_InitMem( void *pDeviceHandle)
{

    BDSP_Raaga *pDevice = (BDSP_Raaga *)pDeviceHandle;
    unsigned i=0,j=0;
    BDSP_Raaga_P_AlgoTypeImgBuf *pAlgoTypeBuf;

    BDBG_ENTER(BDSP_Raaga_P_InitMem);

    BDBG_OBJECT_ASSERT(pDevice, BDSP_Raaga);

    BKNI_Memset(&pDevice->memInfo, 0, sizeof(pDevice->memInfo));

    for(i=0;i<BDSP_AlgorithmType_eMax ; i++ )
    {
        pAlgoTypeBuf = &pDevice->memInfo.sDwnldMemInfo.AlgoTypeBufs[i];
        for(j=0; j< BDSP_RAAGA_MAX_DWNLD_BUFS ; j++ )
        {
            BDSP_INIT_DWNLDBUF( &pAlgoTypeBuf->DwnldBufUsageInfo[j]);
        }
    }

    BDBG_LEAVE(BDSP_Raaga_P_InitMem);
}

/***********************************************************************
Name        :   BDSP_Raaga_P_CalculateInitMemory

Type        :   BDSP Internal

Input       :   pMemoryReq -pointer provided the higher BDSP function to return the memory required

Return      :   Error Code to return SUCCESS or FAILURE

Functionality   :   Following are the operations performed.
        1)  Return the memory required for CMD QUEUE, RESPONSE QUEUE, DSP FIFOs and SWAP Memory per DSP
***********************************************************************/

BERR_Code BDSP_Raaga_P_CalculateInitMemory (
    unsigned *pMemoryReq,
    unsigned NumDsp
)
{
    BERR_Code err = BERR_SUCCESS;
    int32_t i32DspIndex=0;

    /* Calculate the memory required for CMD QUEUE, RESPONSE QUEUE, DSP FIFOs and SWAP Memory per DSP */
    for(i32DspIndex=0;i32DspIndex < (int32_t)NumDsp;i32DspIndex++)
    {
        /* Max tasks that can supported 12 in case of 6x Passthru */
        *pMemoryReq += (BDSP_RAAGA_MAX_MSGS_PER_QUEUE * sizeof(BDSP_Raaga_P_Command)*BDSP_RAAGA_MAX_FW_TASK_PER_DSP);
        /* Single Response Queue is allocated for whole of DSP */
        *pMemoryReq += (BDSP_RAAGA_MAX_MSGS_PER_QUEUE * sizeof(BDSP_Raaga_P_Response));
        /* Memory Requirement of the FIFO's per DSP */
        /* AJ : Check with Vijay */
        *pMemoryReq += (BDSP_RAAGA_NUM_FIFOS * BDSP_RAAGA_NUM_PTR_PER_FIFO * sizeof(dramaddr_t));
        /*Swap Memory Requirement for the DSP */
        *pMemoryReq += BDSP_P_FW_SYSTEM_SWAP_MEMORY_SIZE;
    }
    return err;
}

BERR_Code BDSP_Raaga_P_AllocateInitMemory (
    void *pDeviceHandle
    )
{
    BERR_Code err;
    unsigned j=0;
    int32_t i32DspIndex=0;

    BDSP_Raaga *pDevice = (BDSP_Raaga *)pDeviceHandle;
    int32_t i32MaxDsp = pDevice->numDsp; /* Added to beat coverity job of IRVINE */

    BDBG_ENTER(BDSP_Raaga_P_AllocateInitMemory);

    BDBG_OBJECT_ASSERT(pDevice, BDSP_Raaga);

    BDSP_Raaga_P_InitMem(pDeviceHandle);

    /* Get memory download requirement here */
    err = BDSP_Raaga_P_GetFwMemRequirement(pDeviceHandle);
    if(BERR_SUCCESS != err)
    {
        BDBG_ERR(("BDSP_MEM_P_CalcMemPoolReq: Error getting firmware memory requirements!"));
        err = BERR_TRACE(err);
        goto err_calcMemPoolReq;
    }

#if (BCHP_CHIP != 7278)
    /* TODO For 7278, AJ Moving this to a seperate function to allocate from the contiguous RW memory */
    /* In case the authentication is enabled, all the binaries are downloaded so there is no need to optimize on code download memory.
    In other cases we limit of download of algorithms based on the number of worst case instances of decoder/framesync/encoder/PP etc
    allowed for download and we rotate the pointers internally. Allocation here is for the later case where we manage fewer downloads internally
    in case of non-secure mode*/

    /* Allocate Task Cmd/Resp Queues */
    for(i32DspIndex=0;i32DspIndex<i32MaxDsp;i32DspIndex++)
    {
		err = BDSP_MMA_P_AllocateAlignedMemory(pDevice->memHandle,
						(BDSP_RAAGA_MAX_MSGS_PER_QUEUE * sizeof(BDSP_Raaga_P_Command)*BDSP_RAAGA_MAX_FW_TASK_PER_DSP),
						&(pDevice->memInfo.cmdQueueParams[i32DspIndex].Queue),
						BDSP_MMA_Alignment_32bit);
		if(err)
		{
			BDBG_ERR(("BDSP_DSP_AllocMem: Unable to Allocate memory for cmd queue!"));
			err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
			goto err_alloc_CmdQueue;
		}
		pDevice->memInfo.cmdQueueParams[i32DspIndex].uiMsgQueueSize = BDSP_RAAGA_MAX_MSGS_PER_QUEUE * sizeof(BDSP_Raaga_P_Command)*BDSP_RAAGA_MAX_FW_TASK_PER_DSP;
        err = BDSP_Raaga_P_AssignFreeFIFO(pDevice,(unsigned)i32DspIndex,&(pDevice->memInfo.cmdQueueParams[i32DspIndex].i32FifoId), 1);
        if(err)
        {
            BDBG_ERR(("Unable to find free fifo for CMD QUEUE!!!!"));
            goto err_alloc_CmdQueue;
        }

        /* Allocate Generic Response queue */
		err = BDSP_MMA_P_AllocateAlignedMemory(pDevice->memHandle,
						(BDSP_RAAGA_MAX_MSGS_PER_QUEUE * sizeof(BDSP_Raaga_P_Response)),
						&(pDevice->memInfo.genRspQueueParams[i32DspIndex].Queue),
						BDSP_MMA_Alignment_32bit);
		if(err)
		{
			BDBG_ERR(("BDSP_DSP_AllocMem: Unable to Allocate memory for generic response queue!"));
			err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
			goto err_alloc_genRespQueue;
		}
		pDevice->memInfo.genRspQueueParams[i32DspIndex].uiMsgQueueSize = BDSP_RAAGA_MAX_MSGS_PER_QUEUE * sizeof(BDSP_Raaga_P_Response);
        err = BDSP_Raaga_P_AssignFreeFIFO(pDevice,(unsigned)i32DspIndex,&(pDevice->memInfo.genRspQueueParams[i32DspIndex].i32FifoId), 1);
        if(err)
        {
            BDBG_ERR(("Unable to find free fifo for GENRIC RESP QUEUE!!!!"));
            goto err_alloc_CmdQueue;
        }

    /*  Creating structure for firmware which will contain physical addresses of all
        18 fifo registers in the order base,end,read,write,wrap. Wrap will also be
        programmed with fifo end address. Base address of this will be programmed in
        BCHP_AUD_DSP_CFG0_CONTROL_REGISTER0_CXT0 before bootup */
		err = BDSP_MMA_P_AllocateAlignedMemory(pDevice->memHandle,
								(BDSP_RAAGA_NUM_FIFOS * BDSP_RAAGA_NUM_PTR_PER_FIFO * sizeof(dramaddr_t)),
								&(pDevice->memInfo.DSPFifoAddrStruct[i32DspIndex]),
								BDSP_MMA_Alignment_32bit);
        if(err)
        {
            BDBG_ERR(("BDSP_Raaga_P_Open: Unable to Allocate memory for FIFO Register addresses!"));
            err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
            goto err_alloc_DspFifoAddrStruct;
        }
        for (j = 0; j < BDSP_Raaga_DebugType_eLast; j++)
        {
			if(pDevice->settings.debugSettings[j].enabled == true)
			{
				pDevice->memInfo.FwDebugBuf[i32DspIndex][j].ui32Size = pDevice->settings.debugSettings[j].bufferSize;
			}
			else
			{
				pDevice->memInfo.FwDebugBuf[i32DspIndex][j].ui32Size = 4;
			}
			err = BDSP_MMA_P_AllocateAlignedMemory(pDevice->memHandle,
									(pDevice->memInfo.FwDebugBuf[i32DspIndex][j].ui32Size),
									&(pDevice->memInfo.FwDebugBuf[i32DspIndex][j].Buffer),
									BDSP_MMA_Alignment_32bit);
			if(err)
			{
				BDBG_ERR(("BDSP_RAAGA_Open: Unable to Allocate memory for debug buffer!"));
				err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
				goto err_alloc_FwDebugBuf;
			}
			BKNI_Memset(pDevice->memInfo.FwDebugBuf[i32DspIndex][j].Buffer.pAddr, 0, pDevice->memInfo.FwDebugBuf[i32DspIndex][j].ui32Size);
			BDSP_MMA_P_FlushCache(pDevice->memInfo.FwDebugBuf[i32DspIndex][j].Buffer, pDevice->memInfo.FwDebugBuf[i32DspIndex][j].ui32Size);

            /* Incase of TargetPrint allocate buffer for local TargetPrint buffer */
            if(j == BDSP_Raaga_DebugType_eTargetPrintf)
            {
                /* Allocate buffer for local TargetPrint buffer */
				err = BDSP_MMA_P_AllocateAlignedMemory(pDevice->memHandle,
													(pDevice->memInfo.FwDebugBuf[i32DspIndex][j].ui32Size),
													&(pDevice->memInfo.TargetPrintBuffer[i32DspIndex]),
													BDSP_MMA_Alignment_32bit);
                if(err)
                {
                    BDBG_ERR(("BDSP_RAAGA_Open: Unable to Allocate memory for local TargetPrint debug buffer!"));
                    err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
                    goto err_alloc_FwDebugBuf;
                }
                BKNI_Memset((void *)pDevice->memInfo.TargetPrintBuffer[i32DspIndex].pAddr, 0, pDevice->memInfo.FwDebugBuf[i32DspIndex][j].ui32Size);
				BDSP_MMA_P_FlushCache(pDevice->memInfo.TargetPrintBuffer[i32DspIndex], pDevice->memInfo.FwDebugBuf[i32DspIndex][j].ui32Size);
            }
        }

		/* Need to allocate raaga system swap memory. Raaga system need a memory in DRAM to swap its data memory contents */
		pDevice->memInfo.sRaagaSwapMemoryBuf[i32DspIndex].ui32Size = BDSP_P_FW_SYSTEM_SWAP_MEMORY_SIZE;
		err = BDSP_MMA_P_AllocateAlignedMemory(pDevice->memHandle,
											(pDevice->memInfo.sRaagaSwapMemoryBuf[i32DspIndex].ui32Size),
											&(pDevice->memInfo.sRaagaSwapMemoryBuf[i32DspIndex].Buffer),
											BDSP_MMA_Alignment_32bit);
		if(err)
		{
			BDBG_ERR(("BDSP_RAAGA_Open: Unable to Allocate memory for raaga system swapping!"));
			err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
			goto err_alloc_RaagaSwapMemBuf;
		}
    }
    goto end;
err_alloc_RaagaSwapMemBuf:
    for (j = 0; j < BDSP_Raaga_DebugType_eLast; j++)
    {
		if(pDevice->memInfo.FwDebugBuf[i32DspIndex][j].Buffer.pAddr)
		{
			BDSP_MMA_P_FreeMemory(&pDevice->memInfo.FwDebugBuf[i32DspIndex][j].Buffer);
		}
		if(j == BDSP_Raaga_DebugType_eTargetPrintf)
		{
			if(pDevice->memInfo.TargetPrintBuffer[i32DspIndex].pAddr)
			{
				BDSP_MMA_P_FreeMemory(&pDevice->memInfo.TargetPrintBuffer[i32DspIndex]);
			}
		}
    }
err_alloc_FwDebugBuf:
	if(pDevice->memInfo.DSPFifoAddrStruct[i32DspIndex].pAddr)
	{
		BDSP_MMA_P_FreeMemory(&pDevice->memInfo.DSPFifoAddrStruct[i32DspIndex]);
	}
err_alloc_DspFifoAddrStruct:
	if(pDevice->memInfo.genRspQueueParams[i32DspIndex].Queue.pAddr)
	{
		BDSP_MMA_P_FreeMemory(&pDevice->memInfo.genRspQueueParams[i32DspIndex].Queue);
	}
err_alloc_genRespQueue:
	if(pDevice->memInfo.cmdQueueParams[i32DspIndex].Queue.pAddr)
	{
		BDSP_MMA_P_FreeMemory(&pDevice->memInfo.cmdQueueParams[i32DspIndex].Queue);
	}
err_alloc_CmdQueue:
    while(i32DspIndex!=0)
    {
        i32DspIndex--;
		if(pDevice->memInfo.sRaagaSwapMemoryBuf[i32DspIndex].Buffer.pAddr)
		{
			BDSP_MMA_P_FreeMemory(&pDevice->memInfo.sRaagaSwapMemoryBuf[i32DspIndex].Buffer);
		}
		for (j = 0; j < BDSP_Raaga_DebugType_eLast; j++)
		{
			if(pDevice->memInfo.FwDebugBuf[i32DspIndex][j].Buffer.pAddr)
			{
				BDSP_MMA_P_FreeMemory(&pDevice->memInfo.FwDebugBuf[i32DspIndex][j].Buffer);
			}
		}
		if(pDevice->memInfo.DSPFifoAddrStruct[i32DspIndex].pAddr)
		{
			BDSP_MMA_P_FreeMemory(&pDevice->memInfo.DSPFifoAddrStruct[i32DspIndex]);
		}
		if(pDevice->memInfo.genRspQueueParams[i32DspIndex].Queue.pAddr)
		{
			BDSP_MMA_P_FreeMemory(&pDevice->memInfo.genRspQueueParams[i32DspIndex].Queue);
		}
		if(pDevice->memInfo.cmdQueueParams[i32DspIndex].Queue.pAddr)
		{
			BDSP_MMA_P_FreeMemory(&pDevice->memInfo.cmdQueueParams[i32DspIndex].Queue);
		}
    }
err_calcMemPoolReq:
    BDSP_Raaga_P_FreeFwExec(pDeviceHandle);
end:
#else
    BSTD_UNUSED(i32MaxDsp);
    BSTD_UNUSED(i32DspIndex);
    BSTD_UNUSED(j);
    goto end;
err_calcMemPoolReq:
    BDSP_Raaga_P_FreeFwExec(pDeviceHandle);
end:
#endif /* 7278 */
    BDBG_LEAVE(BDSP_Raaga_P_AllocateInitMemory);
    return err;
}

BERR_Code BDSP_Raaga_P_CalcStageMemPoolReq(void *pStageHandle)
{
    BERR_Code err = BERR_SUCCESS;
    BDSP_AF_P_AlgoId algoId;

    uint32_t ui32AlgoIf = 0, ui32AlgoCfgBuf = 0, ui32AlgoStatusBuf = 0;
    uint32_t ui32FsIf = 0, ui32FsCfgBuf = 0, ui32FsStatusBuf = 0;
    uint32_t ui32TempIf = 0, ui32TempCfgBuf = 0, ui32TempStatusBuf =0;

    unsigned int i = 0, j = 0;

    BDSP_RaagaStage *pRaagaStage = (BDSP_RaagaStage   *)pStageHandle;
    BDSP_RaagaContext   *pRaagaContext = pRaagaStage->pContext;

    BDBG_ASSERT(NULL != pRaagaStage);
    BDBG_ENTER(BDSP_Raaga_P_CalcStageMemPoolReq);

    /* For Decoders */
    for ( i=0; i < BDSP_Algorithm_eMax; i++ )
    {
        const BDSP_Raaga_P_AlgorithmInfo *pInfo;

        pInfo = BDSP_Raaga_P_LookupAlgorithmInfo(i);
        if ( !pInfo->supported )
        {
            continue;
        }

        if(!pRaagaStage->settings.algorithmSupported[i])
        {
            continue;
        }

        ui32TempIf = 0;
        ui32TempCfgBuf = 0;
        ui32TempStatusBuf =0;

        /* Framesync buffer requirement is computed separately */
        algoId = pInfo->algoExecInfo.eAlgoIds[0];
        if ( algoId < BDSP_AF_P_AlgoId_eMax )
        {
            if (ui32FsIf < BDSP_sNodeInfo[algoId].ui32InterFrmBuffSize)
            {
                ui32FsIf = BDSP_sNodeInfo[algoId].ui32InterFrmBuffSize;
            }
            if (ui32FsCfgBuf < BDSP_sNodeInfo[algoId].ui32UserCfgBuffSize)
            {
                ui32FsCfgBuf = BDSP_sNodeInfo[algoId].ui32UserCfgBuffSize;
            }
            if (ui32FsStatusBuf < BDSP_sNodeInfo[algoId].ui32StatusBuffSize)
            {
                ui32FsStatusBuf = BDSP_sNodeInfo[algoId].ui32StatusBuffSize;
            }
        }

        /* Video Encoder related Interframe is huge. Keep them away from Audio Calculations*/
        if(pRaagaContext->settings.contextType != BDSP_ContextType_eVideoEncode)
        {
            /* Buffer requirement for the rest of the nodes */
            for( j=1; j < pInfo->algoExecInfo.NumNodes; j++ )
            {
                algoId = pInfo->algoExecInfo.eAlgoIds[j];
                if ( algoId < BDSP_AF_P_AlgoId_eMax )
                {
                    if ( algoId < BDSP_VF_P_AlgoId_eVideoEncodeAlgoStartIdx || algoId > BDSP_VF_P_AlgoId_eEndOfVideoEncodeAlgos)
                    {
                        ui32TempIf += BDSP_sNodeInfo[algoId].ui32InterFrmBuffSize;
                        ui32TempCfgBuf += BDSP_sNodeInfo[algoId].ui32UserCfgBuffSize;
                        ui32TempStatusBuf += BDSP_sNodeInfo[algoId].ui32StatusBuffSize;
                    }
                }
            }
        } else
        {
            /* Buffer requirement for the rest of the nodes */
            for( j=1; j < pInfo->algoExecInfo.NumNodes; j++ )
            {
                algoId = pInfo->algoExecInfo.eAlgoIds[j];
                if ( algoId < BDSP_AF_P_AlgoId_eMax )
                {
                    ui32TempIf += BDSP_sNodeInfo[algoId].ui32InterFrmBuffSize;
                    ui32TempCfgBuf += BDSP_sNodeInfo[algoId].ui32UserCfgBuffSize;
                    ui32TempStatusBuf += BDSP_sNodeInfo[algoId].ui32StatusBuffSize;
                }
            }
        }

        if ( ui32TempIf > ui32AlgoIf )
        {
            ui32AlgoIf = ui32TempIf;
        }
        if ( ui32TempCfgBuf > ui32AlgoCfgBuf )
        {
            ui32AlgoCfgBuf = ui32TempCfgBuf;
        }
        if ( ui32TempStatusBuf > ui32AlgoStatusBuf )
        {
            ui32AlgoStatusBuf = ui32TempStatusBuf;
        }
    }

    pRaagaStage->stageMemInfo.ui32InterframeBufReqd = ui32AlgoIf + ui32FsIf;
    pRaagaStage->stageMemInfo.ui32UserConfigReqd = ui32AlgoCfgBuf + ui32FsCfgBuf;
    pRaagaStage->stageMemInfo.ui32StatusBufReqd = ui32AlgoStatusBuf + ui32FsStatusBuf;

    pRaagaStage->sFrameSyncOffset.ui32IfOffset = ui32AlgoIf;
    pRaagaStage->sFrameSyncOffset.ui32UserCfgOffset = ui32AlgoCfgBuf;
    pRaagaStage->sFrameSyncOffset.ui32StatusOffset = ui32AlgoStatusBuf;

    BDBG_MSG(("ui32AlgoIf = %d ui32AlgoCfgBuf =%d ui32AlgoStatusBuf =%d",ui32AlgoIf,ui32AlgoCfgBuf, ui32AlgoStatusBuf));
    BDBG_MSG(("ui32FsIf = %d ui32FsCfgBuf =%d ui32FsStatusBuf =%d",ui32FsIf,ui32FsCfgBuf, ui32FsStatusBuf));

    BDBG_LEAVE(BDSP_Raaga_P_CalcStageMemPoolReq);
    return err;
}

/***********************************************************************
Name        :   BDSP_Raaga_P_CalcScratchAndISbufferReq_MemToolAPI

Type        :   BDSP Internal

Input       :   pui32ScratchMem         -Pointer provided to the caller function to return the max Scratch memory Required.
                pui32InterstageIOMem        -Pointer provided to the caller function to return the max Interstage IO memory Required.
                pui32InterstageIOGenMem     -Pointer provided to the caller function to return the max Interstage IO Generic memory Required.
                pui32Numch              -Pointer provided to the caller function to return the Max number of channels supported.
                pUsage                  -Usage Case As provided by the Caller.

Return      :   Error Code to return SUCCESS or FAILURE

Functionality   :   Following are the operations performed.
        1)  Return the highest Scratch, Interstage IO, Interstage IO Generic and Max channels required/supported by the system.
***********************************************************************/
BERR_Code BDSP_Raaga_P_CalcScratchAndISbufferReq_MemToolAPI(
        uint32_t *pui32ScratchMem,
        uint32_t *pui32InterstageIOMem,
        uint32_t *pui32InterstageIOGenMem,
        uint32_t *pui32Numch,
        BDSP_AF_P_eSchedulingGroup eSchedulingGroup,
        const BDSP_RaagaUsageOptions *pUsage
)
{
    BERR_Code err = BERR_SUCCESS;
    unsigned int Algoindex = 0, Nodeindex = 0;
    uint32_t ui32TempScratch = 0, ui32TempIs = 0, ui32TempIsIf = 0;
    uint32_t ui32Scratch = 0, ui32Is = 0, ui32IsIf = 0;
    uint32_t ui32NumCh=0;

    BDBG_ENTER(BDSP_Raaga_P_CalcScratchAndISbufferReq_MemToolAPI);

    /* For Decoders */
    for ( Algoindex=0; Algoindex < BDSP_Algorithm_eMax; Algoindex++ )
    {
        const BDSP_Raaga_P_AlgorithmSupportInfo *pInfo;
        if(pUsage->Codeclist[Algoindex]!= true)
        {
            continue;
        }
        pInfo = BDSP_Raaga_P_LookupAlgorithmSupported(Algoindex,pUsage->DolbyCodecVersion);
		/* ignore the algorithm if scheduling group is not supported */
		if (pInfo->sAlgoSchedulingGroupInfo.IsSchedulingGroupSupported[eSchedulingGroup] == false)
		{
			continue;
		}

        ui32TempScratch = 0;
        ui32TempIs = 0;
        ui32TempIsIf = 0;

        for( Nodeindex=0; Nodeindex < pInfo->algoExecInfo.NumNodes; Nodeindex++ )
        {
            BDSP_AF_P_AlgoId algoId = pInfo->algoExecInfo.eAlgoIds[Nodeindex];
            if ( algoId < BDSP_AF_P_AlgoId_eMax )
            {
                ui32TempScratch += BDSP_sNodeInfo[algoId].ui32ScratchBuffSize;

                if(ui32TempIs < BDSP_sNodeInfo[algoId].ui32MaxSizePerChan)
                    ui32TempIs = BDSP_sNodeInfo[algoId].ui32MaxSizePerChan;
                if(ui32NumCh < BDSP_sNodeInfo[algoId].ui32MaxNumChansSupported)
                    ui32NumCh = BDSP_sNodeInfo[algoId].ui32MaxNumChansSupported;

                ui32TempIsIf += BDSP_sNodeInfo[algoId].ui32InterStgGenericBuffSize;
            }
        }

        if(ui32TempScratch >= ui32Scratch)
            ui32Scratch = ui32TempScratch;
        if(ui32TempIs >= ui32Is)
            ui32Is = ui32TempIs;
        if(ui32TempIsIf >= ui32IsIf)
            ui32IsIf = ui32TempIsIf;
    }

    *pui32ScratchMem        = ui32Scratch;
    *pui32InterstageIOMem   = ui32Is;
    *pui32InterstageIOGenMem= ui32IsIf;
    *pui32Numch             = ui32NumCh;

    BDBG_LEAVE(BDSP_Raaga_P_CalcScratchAndISbufferReq_MemToolAPI);
    return err;
}

/***********************************************************************
Name        :   BDSP_Raaga_P_CalcScratchAndISbufferReq

Type        :   BDSP Internal

Input       :   pui32ScratchMem         -Pointer provided the higher BDSP function to return the max Scratch memory Required.
                pui32InterstageIOMem        -Pointer provided the higher BDSP function to return the max Interstage IO memory Required.
                pui32InterstageIOGenMem     -Pointer provided the higher BDSP function to return the max Interstage IO Generic memory Required.
                pui32Numch              -Pointer provided the higher BDSP function to return the Max number of channels supported.

Return      :   Error Code to return SUCCESS or FAILURE

Functionality   :   Following are the operations performed.
        1)  Return the highest Scratch, Interstage IO, Interstage IO Generic and Max channels required/supported by the system.
***********************************************************************/
BERR_Code BDSP_Raaga_P_CalcScratchAndISbufferReq(
        uint32_t *pui32ScratchMem,
        uint32_t *pui32InterstageIOMem,
        uint32_t *pui32InterstageIOGenMem,
        uint32_t *pui32Numch,
        BDSP_AF_P_eSchedulingGroup eSchedulingGroup
)
{
    BERR_Code err = BERR_SUCCESS;
    unsigned int Algoindex = 0, Nodeindex = 0;
    uint32_t ui32TempScratch = 0, ui32TempIs = 0, ui32TempIsIf = 0;
    uint32_t ui32Scratch = 0, ui32Is = 0, ui32IsIf = 0;
    uint32_t ui32NumCh=0;

    BDBG_ENTER(BDSP_Raaga_P_CalcScratchAndISbufferReq);

    /* For Decoders */
    for ( Algoindex=0; Algoindex < BDSP_Algorithm_eMax; Algoindex++ )
    {
        const BDSP_Raaga_P_AlgorithmInfo *pInfo;

        pInfo = BDSP_Raaga_P_LookupAlgorithmInfo(Algoindex);
        if ( !pInfo->supported || (pInfo->sAlgoSchedulingGroupInfo.IsSchedulingGroupSupported[eSchedulingGroup] == false))
        {
            continue;
        }

        ui32TempScratch = 0;
        ui32TempIs = 0;
        ui32TempIsIf = 0;

        for( Nodeindex=0; Nodeindex < pInfo->algoExecInfo.NumNodes; Nodeindex++ )
        {
            BDSP_AF_P_AlgoId algoId = pInfo->algoExecInfo.eAlgoIds[Nodeindex];
            if ( algoId < BDSP_AF_P_AlgoId_eMax )
            {
                ui32TempScratch += BDSP_sNodeInfo[algoId].ui32ScratchBuffSize;

                if(ui32TempIs < BDSP_sNodeInfo[algoId].ui32MaxSizePerChan)
                    ui32TempIs = BDSP_sNodeInfo[algoId].ui32MaxSizePerChan;
                if(ui32NumCh < BDSP_sNodeInfo[algoId].ui32MaxNumChansSupported)
                    ui32NumCh = BDSP_sNodeInfo[algoId].ui32MaxNumChansSupported;

                ui32TempIsIf += BDSP_sNodeInfo[algoId].ui32InterStgGenericBuffSize;

            }
        }

        if(ui32TempScratch >= ui32Scratch)
            ui32Scratch = ui32TempScratch;
        if(ui32TempIs >= ui32Is)
            ui32Is = ui32TempIs;
        if(ui32TempIsIf >= ui32IsIf)
            ui32IsIf = ui32TempIsIf;

    }

    *pui32ScratchMem        = ui32Scratch;
    *pui32InterstageIOMem   = ui32Is;
    *pui32InterstageIOGenMem= ui32IsIf;
    *pui32Numch             = ui32NumCh;

    BDBG_LEAVE(BDSP_Raaga_P_CalcScratchAndISbufferReq);
    return err;
}

BERR_Code BDSP_Raaga_P_CalcandAllocScratchISbufferReq(void *pDeviceHandle)
{

    BERR_Code err = BERR_SUCCESS;
	uint32_t ui32TempScratch[BDSP_AF_P_eSchedulingGroup_Max] = {0}, ui32TempIs[BDSP_AF_P_eSchedulingGroup_Max] = {0}, ui32TempIsIf[BDSP_AF_P_eSchedulingGroup_Max] = {0};
	uint32_t ui32NumCh[BDSP_AF_P_eSchedulingGroup_Max] = {0};
    dramaddr_t  raagaOffsetAddr = 0;
    unsigned int k=0;
    int32_t i32DspIndex =0, i32TaskIndex = 0, i32SchedulingGroupIndex=0;
    int32_t i32MaxDsp = 0;

    BDSP_Raaga *pDevice = (BDSP_Raaga *)pDeviceHandle;
    BDBG_OBJECT_ASSERT(pDevice, BDSP_Raaga);
    i32MaxDsp = pDevice->numDsp; /* Added to beat coverity job of IRVINE */

    BDBG_ENTER(BDSP_Raaga_P_CalcandAllocScratchISbufferReq);
	for(i32SchedulingGroupIndex=0; i32SchedulingGroupIndex<(int32_t)BDSP_AF_P_eSchedulingGroup_Max; i32SchedulingGroupIndex++)
	{
	    BDSP_Raaga_P_CalcScratchAndISbufferReq(&ui32TempScratch[i32SchedulingGroupIndex],
			&ui32TempIs[i32SchedulingGroupIndex],
			&ui32TempIsIf[i32SchedulingGroupIndex],
			&ui32NumCh[i32SchedulingGroupIndex],
			i32SchedulingGroupIndex);

	    BDBG_MSG(("ui32TempScratch[%d] = %d , ui32TempIs[%d] = %d ui32TempIsIf]%d] =%d",
			i32SchedulingGroupIndex, ui32TempScratch[i32SchedulingGroupIndex],
			i32SchedulingGroupIndex, ui32TempIs[i32SchedulingGroupIndex],
			i32SchedulingGroupIndex, ui32TempIsIf[i32SchedulingGroupIndex]));
	}
#if (BCHP_CHIP == 7278)
    BSTD_UNUSED(i32MaxDsp);
#endif
    /*ui32TempIs is the worst case IS buffer per channel for a decoder/passthrough
            ui32PaIs is the worst case IS buffer per channel for a PP
            ui32EncIs is the worst case IS buffer per channel for a Encoder
            ui32NumCh is the worst case number of channels

    ui32TempIsIf is the worst case IS Generic buffer for a decoder/passthrough
            ui32PaIsIf is the worst case IS Generic buffer for a PP
            ui32EncIsIf is the worst case IS Generic buffer for a Encoder
    */

    /* this is done in Raaga_P_Open time itself for each DSP and not for every Context*/
#if (BCHP_CHIP != 7278)
    for(i32DspIndex=0;i32DspIndex<i32MaxDsp;i32DspIndex++)
#endif /* (BCHP_CHIP != 7278) */
    {
		for(i32SchedulingGroupIndex=0; i32SchedulingGroupIndex<(int32_t)BDSP_AF_P_eSchedulingGroup_Max; i32SchedulingGroupIndex++)
		{
        /* Allocate Scratch Memory */
			if (ui32TempScratch[i32SchedulingGroupIndex])
			{
#if (BCHP_CHIP != 7278)
	            err = BDSP_MMA_P_AllocateAlignedMemory(pDevice->memHandle,
	                            ui32TempScratch[i32SchedulingGroupIndex],
	                            &(pDevice->memInfo.sScratchandISBuff[i32DspIndex].DspScratchMemGrant[i32SchedulingGroupIndex].Buffer),
	                            BDSP_MMA_Alignment_32bit);
#else
	            err = BDSP_Raaga_P_AssignFromRWMem(pDevice,
	                            ui32TempScratch[i32SchedulingGroupIndex],
	                            &(pDevice->memInfo.sScratchandISBuff[i32DspIndex].DspScratchMemGrant[i32SchedulingGroupIndex].Buffer));
#endif
				if(err != BERR_SUCCESS)
				{
					BDBG_ERR(("BDSP_Raaga_P_CalcandAllocScratchISbufferReq: Unable to Allocate memory for Scratch Buffer!"));
					err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
					goto error_alloc_scratch_mem;
				}
			}

			pDevice->memInfo.sScratchandISBuff[i32DspIndex].DspScratchMemGrant[i32SchedulingGroupIndex].ui32Size=ui32TempScratch[i32SchedulingGroupIndex];
	        for(i32TaskIndex=0; i32TaskIndex<BDSP_RAAGA_MAX_BRANCH_PER_TASK; i32TaskIndex++)
	        {
#if (BCHP_CHIP != 7278)
	            err = BDSP_MMA_P_AllocateAlignedMemory(pDevice->memHandle,
								ui32TempIs[i32SchedulingGroupIndex]*ui32NumCh[i32SchedulingGroupIndex],
								&(pDevice->memInfo.sScratchandISBuff[i32DspIndex].InterStageIOBuff[i32SchedulingGroupIndex][i32TaskIndex].Buffer),
								BDSP_MMA_Alignment_32bit);
#else
	            err = BDSP_Raaga_P_AssignFromRWMem(pDevice,
	                             ui32TempIs[i32SchedulingGroupIndex]*ui32NumCh[i32SchedulingGroupIndex],
	                             &(pDevice->memInfo.sScratchandISBuff[i32DspIndex].InterStageIOBuff[i32SchedulingGroupIndex][i32TaskIndex].Buffer));
#endif
				if(err != BERR_SUCCESS)
				{
					BDBG_ERR(("BDSP_Raaga_P_CalcandAllocScratchISbufferReq: Unable to Allocate memory for IO Buffer!"));
					err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
					/*free scratch*/
					goto error_alloc_IO_mem;
				}

				/*Mark all the I/O and I/O Gen buffers as not in use to start with and set them during stage connects. */
				pDevice->memInfo.sScratchandISBuff[i32DspIndex].InUse[i32SchedulingGroupIndex][i32TaskIndex]=0;
				for(k=0; k<ui32NumCh[i32SchedulingGroupIndex]; k++)
				{
					raagaOffsetAddr = pDevice->memInfo.sScratchandISBuff[i32DspIndex].InterStageIOBuff[i32SchedulingGroupIndex][i32TaskIndex].Buffer.offset + (k*ui32TempIs[i32SchedulingGroupIndex]);
					pDevice->memInfo.sScratchandISBuff[i32DspIndex].InterStageIOBuff[i32SchedulingGroupIndex][i32TaskIndex].IOBuff.sCircBuffer[k].ui32BaseAddr = raagaOffsetAddr;
					pDevice->memInfo.sScratchandISBuff[i32DspIndex].InterStageIOBuff[i32SchedulingGroupIndex][i32TaskIndex].IOBuff.sCircBuffer[k].ui32ReadAddr = raagaOffsetAddr;
					pDevice->memInfo.sScratchandISBuff[i32DspIndex].InterStageIOBuff[i32SchedulingGroupIndex][i32TaskIndex].IOBuff.sCircBuffer[k].ui32WriteAddr= raagaOffsetAddr;
					pDevice->memInfo.sScratchandISBuff[i32DspIndex].InterStageIOBuff[i32SchedulingGroupIndex][i32TaskIndex].IOBuff.sCircBuffer[k].ui32EndAddr  = raagaOffsetAddr + ui32TempIs[i32SchedulingGroupIndex];
					pDevice->memInfo.sScratchandISBuff[i32DspIndex].InterStageIOBuff[i32SchedulingGroupIndex][i32TaskIndex].IOBuff.sCircBuffer[k].ui32WrapAddr = raagaOffsetAddr + ui32TempIs[i32SchedulingGroupIndex];
				}
				pDevice->memInfo.sScratchandISBuff[i32DspIndex].InterStageIOBuff[i32SchedulingGroupIndex][i32TaskIndex].IOBuff.ui32NumBuffers  = ui32NumCh[i32SchedulingGroupIndex];/*worst case, maxchannels*/
				pDevice->memInfo.sScratchandISBuff[i32DspIndex].InterStageIOBuff[i32SchedulingGroupIndex][i32TaskIndex].IOBuff.eBufferType	  = BDSP_AF_P_BufferType_eDRAM_IS;


				/* Allocate IO Generic Memory and also the configuration structure*/
#if (BCHP_CHIP != 7278)
				err = BDSP_MMA_P_AllocateAlignedMemory(pDevice->memHandle,
								 ui32TempIsIf[i32SchedulingGroupIndex],
								&(pDevice->memInfo.sScratchandISBuff[i32DspIndex].InterStageIOGenericBuff[i32SchedulingGroupIndex][i32TaskIndex].Buffer),
								BDSP_MMA_Alignment_32bit);
#else
				err = BDSP_Raaga_P_AssignFromRWMem(pDevice,
				                ui32TempIsIf[i32SchedulingGroupIndex],
	                            &(pDevice->memInfo.sScratchandISBuff[i32DspIndex].InterStageIOGenericBuff[i32SchedulingGroupIndex][i32TaskIndex].Buffer));
#endif
				if(err != BERR_SUCCESS)
				{
					BDBG_ERR(("BDSP_Raaga_P_CalcandAllocScratchISbufferReq: Unable to Allocate memory for IO Generic!"));
					err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
					/*Free scratch and IO buffer here*/
					goto error_alloc_IOGen_mem;
				}
				pDevice->memInfo.sScratchandISBuff[i32DspIndex].InterStageIOGenericBuff[i32SchedulingGroupIndex][i32TaskIndex].IOGenericBuff.ui32NumBuffers= 1;
				pDevice->memInfo.sScratchandISBuff[i32DspIndex].InterStageIOGenericBuff[i32SchedulingGroupIndex][i32TaskIndex].IOGenericBuff.eBufferType	= BDSP_AF_P_BufferType_eDRAM_IS;

				raagaOffsetAddr = pDevice->memInfo.sScratchandISBuff[i32DspIndex].InterStageIOGenericBuff[i32SchedulingGroupIndex][i32TaskIndex].Buffer.offset;
				pDevice->memInfo.sScratchandISBuff[i32DspIndex].InterStageIOGenericBuff[i32SchedulingGroupIndex][i32TaskIndex].IOGenericBuff.sCircBuffer.ui32BaseAddr = raagaOffsetAddr;
				pDevice->memInfo.sScratchandISBuff[i32DspIndex].InterStageIOGenericBuff[i32SchedulingGroupIndex][i32TaskIndex].IOGenericBuff.sCircBuffer.ui32ReadAddr = raagaOffsetAddr;
				pDevice->memInfo.sScratchandISBuff[i32DspIndex].InterStageIOGenericBuff[i32SchedulingGroupIndex][i32TaskIndex].IOGenericBuff.sCircBuffer.ui32WriteAddr= raagaOffsetAddr;
				pDevice->memInfo.sScratchandISBuff[i32DspIndex].InterStageIOGenericBuff[i32SchedulingGroupIndex][i32TaskIndex].IOGenericBuff.sCircBuffer.ui32EndAddr	 = raagaOffsetAddr+ui32TempIsIf[i32SchedulingGroupIndex];
				pDevice->memInfo.sScratchandISBuff[i32DspIndex].InterStageIOGenericBuff[i32SchedulingGroupIndex][i32TaskIndex].IOGenericBuff.sCircBuffer.ui32WrapAddr = raagaOffsetAddr+ui32TempIsIf[i32SchedulingGroupIndex];
	        }
		}
    }
    goto end;
/* AJ : For 7278, only no memory is allocated but only assignment happens.
 * Clean up is taken care as part of error handling  */
#if  (BCHP_CHIP != 7278)
error_alloc_IOGen_mem:
	if(pDevice->memInfo.sScratchandISBuff[i32DspIndex].InterStageIOBuff[i32SchedulingGroupIndex][i32TaskIndex].Buffer.pAddr)
	{
		BDSP_MMA_P_FreeMemory(&pDevice->memInfo.sScratchandISBuff[i32DspIndex].InterStageIOBuff[i32SchedulingGroupIndex][i32TaskIndex].Buffer);
	}
error_alloc_IO_mem:
	while(i32TaskIndex!=0)
	{
		i32TaskIndex--;
		if(pDevice->memInfo.sScratchandISBuff[i32DspIndex].InterStageIOGenericBuff[i32SchedulingGroupIndex][i32TaskIndex].Buffer.pAddr)
		{
			BDSP_MMA_P_FreeMemory(&pDevice->memInfo.sScratchandISBuff[i32DspIndex].InterStageIOGenericBuff[i32SchedulingGroupIndex][i32TaskIndex].Buffer);
		}
		if(pDevice->memInfo.sScratchandISBuff[i32DspIndex].InterStageIOBuff[i32SchedulingGroupIndex][i32TaskIndex].Buffer.pAddr)
		{
			BDSP_MMA_P_FreeMemory(&pDevice->memInfo.sScratchandISBuff[i32DspIndex].InterStageIOBuff[i32SchedulingGroupIndex][i32TaskIndex].Buffer);
		}
	}
	if (pDevice->memInfo.sScratchandISBuff[i32DspIndex].DspScratchMemGrant[i32SchedulingGroupIndex].Buffer.pAddr)
	{
			BDSP_MMA_P_FreeMemory(&pDevice->memInfo.sScratchandISBuff[i32DspIndex].DspScratchMemGrant[i32SchedulingGroupIndex].Buffer);
	}
error_alloc_scratch_mem:
    /* free IS and Scratch memory allocated for previous scheduling groups and current dspIndex */
	while(i32SchedulingGroupIndex!=0)
	{
	    i32SchedulingGroupIndex--;
	    for((i32TaskIndex=(BDSP_RAAGA_MAX_BRANCH_PER_TASK-1)); i32TaskIndex >= 0; i32TaskIndex--)
	    {
			if(pDevice->memInfo.sScratchandISBuff[i32DspIndex].InterStageIOGenericBuff[i32SchedulingGroupIndex][i32TaskIndex].Buffer.pAddr)
			{
				BDSP_MMA_P_FreeMemory(&pDevice->memInfo.sScratchandISBuff[i32DspIndex].InterStageIOGenericBuff[i32SchedulingGroupIndex][i32TaskIndex].Buffer);
			}
			if(pDevice->memInfo.sScratchandISBuff[i32DspIndex].InterStageIOBuff[i32SchedulingGroupIndex][i32TaskIndex].Buffer.pAddr)
			{
				BDSP_MMA_P_FreeMemory(&pDevice->memInfo.sScratchandISBuff[i32DspIndex].InterStageIOBuff[i32SchedulingGroupIndex][i32TaskIndex].Buffer);
			}
	    }
		if (pDevice->memInfo.sScratchandISBuff[i32DspIndex].DspScratchMemGrant[i32SchedulingGroupIndex].ui32Size)
		{
			if(pDevice->memInfo.sScratchandISBuff[i32DspIndex].DspScratchMemGrant[i32SchedulingGroupIndex].Buffer.pAddr)
			{
				BDSP_MMA_P_FreeMemory(&pDevice->memInfo.sScratchandISBuff[i32DspIndex].DspScratchMemGrant[i32SchedulingGroupIndex].Buffer);
			}
		}
	}

/* free IS and Scratch memory allocated for previous dspIndexes for all scheduling groupus and branches */

    while(i32DspIndex!=0)
    {
        i32DspIndex--;

		for((i32SchedulingGroupIndex=(BDSP_AF_P_eSchedulingGroup_Max-1)); i32SchedulingGroupIndex >= 0; i32SchedulingGroupIndex--)
		{
			for((i32TaskIndex=(BDSP_RAAGA_MAX_BRANCH_PER_TASK-1)); i32TaskIndex >= 0; i32TaskIndex--)
			{
				if(pDevice->memInfo.sScratchandISBuff[i32DspIndex].InterStageIOGenericBuff[i32SchedulingGroupIndex][i32TaskIndex].Buffer.pAddr)
				{
					BDSP_MMA_P_FreeMemory(&pDevice->memInfo.sScratchandISBuff[i32DspIndex].InterStageIOGenericBuff[i32SchedulingGroupIndex][i32TaskIndex].Buffer);
				}
				if(pDevice->memInfo.sScratchandISBuff[i32DspIndex].InterStageIOBuff[i32SchedulingGroupIndex][i32TaskIndex].Buffer.pAddr)
				{
					BDSP_MMA_P_FreeMemory(&pDevice->memInfo.sScratchandISBuff[i32DspIndex].InterStageIOBuff[i32SchedulingGroupIndex][i32TaskIndex].Buffer);
				}
			}
			if(pDevice->memInfo.sScratchandISBuff[i32DspIndex].DspScratchMemGrant[i32SchedulingGroupIndex].Buffer.pAddr)
			{
				BDSP_MMA_P_FreeMemory(&pDevice->memInfo.sScratchandISBuff[i32DspIndex].DspScratchMemGrant[i32SchedulingGroupIndex].Buffer);
			}
		}
    }
#else
error_alloc_IOGen_mem :
error_alloc_IO_mem :
error_alloc_scratch_mem :

#endif /* (BCHP_CHIP != 7278) */
end:
    BDBG_LEAVE(BDSP_Raaga_P_CalcandAllocScratchISbufferReq);
    return err;
}

BERR_Code BDSP_Raaga_P_FreeStageMemory(
    void *pStageHandle
    )
{
    BERR_Code   err = BERR_SUCCESS;
    BDSP_RaagaStage *pRaagaStage = (BDSP_RaagaStage *)pStageHandle;

    BDBG_ENTER(BDSP_Raaga_P_FreeStageMemory);

	/* Free the interframe buffer allocation */
	if(pRaagaStage->sDramInterFrameBuffer.Buffer.pAddr)
	{
		BDSP_MMA_P_FreeMemory(&pRaagaStage->sDramInterFrameBuffer.Buffer);
	}

	/* Free the user settings buffer allocation */
	if(pRaagaStage->sDramUserConfigBuffer.Buffer.pAddr)
	{
		BDSP_MMA_P_FreeMemory(&pRaagaStage->sDramUserConfigBuffer.Buffer);
	}

	/* Free the BDSP working buffer of user settings */
	if(pRaagaStage->sDramUserConfigSpareBuffer.Buffer.pAddr )
	{
		BDSP_MMA_P_FreeMemory(&pRaagaStage->sDramUserConfigSpareBuffer.Buffer);
	}

	/* Free the status buffer allocation */
	if(pRaagaStage->sDramStatusBuffer.Buffer.pAddr)
	{
		BDSP_MMA_P_FreeMemory(&pRaagaStage->sDramStatusBuffer.Buffer);
	}

	/* Free the descriptor buffer allocation */
	BDSP_MMA_P_FreeMemory(&pRaagaStage->sStageInput[0].IoBuffDesc);
    BDBG_LEAVE(BDSP_Raaga_P_FreeStageMemory);

    return err;
}



BERR_Code BDSP_Raaga_P_FreeTaskMemory(
    void *pTaskHandle
    )
{
    BERR_Code   err=BERR_SUCCESS;
    BDSP_RaagaTask   *pRaagaTask = (BDSP_RaagaTask   *)pTaskHandle;
    BDSP_RaagaContext   *pRaagaContext = (BDSP_RaagaContext   *)pRaagaTask->pContext;
    BDBG_ENTER(BDSP_Raaga_P_FreeTaskMemory);

    BDBG_OBJECT_ASSERT(pRaagaTask, BDSP_RaagaTask);

    err = BDSP_Raaga_P_FreeTaskQueues(pTaskHandle);
    if (BERR_SUCCESS != err)
    {
        BDBG_ERR(("BDSP_Raaga_P_FreeTaskMemory: Queue destroy failed!"));
        err = BERR_TRACE(err);
    }
	if (pRaagaTask->settings.masterTask == BDSP_TaskSchedulingMode_eMaster)
	{
		if ((pRaagaTask->FeedbackBuffer.pAddr !=NULL))
		{
			BDBG_MSG(("BDSP_Raaga_P_FreeTaskMemory: Freeing pRaagaTask->FeedbackBuffer.pAddr = 0x%p",pRaagaTask->FeedbackBuffer.pAddr));
			BDSP_MMA_P_FreeMemory(&pRaagaTask->FeedbackBuffer);
		}
	}

	BDSP_MMA_P_FreeMemory(&pRaagaTask->taskMemGrants.sTaskQueue.sTaskSyncQueue.Queue);
	BDSP_MMA_P_FreeMemory(&pRaagaTask->taskMemGrants.sTaskQueue.sTaskAsyncQueue.Queue);

	if ( pRaagaTask->taskMemGrants.sTaskQueue.sAsyncMsgBufmem.pAddr )
	{
		BKNI_Free(pRaagaTask->taskMemGrants.sTaskQueue.sAsyncMsgBufmem.pAddr);
		pRaagaTask->taskMemGrants.sTaskQueue.sAsyncMsgBufmem.pAddr = 0;
	}

	BDSP_MMA_P_FreeMemory(&pRaagaTask->taskMemGrants.sTaskGateOpenBufInfo.Buffer);
	BDSP_MMA_P_FreeMemory(&pRaagaTask->taskMemGrants.sStackSwapBuff.Buffer);
	BDSP_MMA_P_FreeMemory(&pRaagaTask->taskMemGrants.sTaskCfgBufInfo.Buffer);
	BDSP_MMA_P_FreeMemory(&pRaagaTask->taskMemGrants.sTaskInfo.Buffer);
	BDSP_MMA_P_FreeMemory(&pRaagaTask->taskMemGrants.sCitStruct.Buffer);

	if(pRaagaTask->taskMemGrants.sSpareCitStruct.Buffer.pAddr != NULL )
	{
		BDSP_MMA_P_FreeMemory(&pRaagaTask->taskMemGrants.sSpareCitStruct.Buffer);
	}

	if( BDSP_ContextType_eVideo == pRaagaContext->settings.contextType)
	{
		BDSP_MMA_P_FreeMemory(&pRaagaTask->taskMemGrants.sTaskQueue.sPDQueue.Queue);
		BDSP_MMA_P_FreeMemory(&pRaagaTask->taskMemGrants.sTaskQueue.sPRQueue.Queue);
		BDSP_MMA_P_FreeMemory(&pRaagaTask->taskMemGrants.sTaskQueue.sDSQueue.Queue);
	}
	else if(BDSP_ContextType_eVideoEncode == pRaagaContext->settings.contextType )
	{
		BDSP_MMA_P_FreeMemory(&pRaagaTask->taskMemGrants.sTaskQueue.sRRQueue.Queue);
		BDSP_MMA_P_FreeMemory(&pRaagaTask->taskMemGrants.sTaskQueue.sCCDQueue.Queue);
	}
    BDBG_LEAVE(BDSP_Raaga_P_FreeTaskMemory);

    return err;
}

BERR_Code BDSP_Raaga_P_FreeContextMemory(
    void *pContextHandle
    )
{
    BDSP_RaagaContext   *pRaagaContext = (BDSP_RaagaContext   *)pContextHandle;

    BERR_Code   err=BERR_SUCCESS;

    BDBG_ENTER(BDSP_Raaga_P_FreeContextMemory);

    BDBG_OBJECT_ASSERT(pRaagaContext, BDSP_RaagaContext);

	BDSP_MMA_P_FreeMemory(&pRaagaContext->contextMemInfo.sVomTableInfo.Buffer);

    BDBG_LEAVE(BDSP_Raaga_P_FreeContextMemory);

    return err;
}

BERR_Code BDSP_Raaga_P_CalculateStageMemory(
    BDSP_AlgorithmType AlgorithmType,
    unsigned *pMemoryReq,
    const BDSP_RaagaUsageOptions *pUsage
    )
{
    BERR_Code err = BERR_SUCCESS;
    BDSP_AF_P_AlgoId algoId;

    uint32_t ui32AlgoIf = 0, ui32AlgoCfgBuf = 0, ui32AlgoStatusBuf = 0;
    uint32_t ui32FsIf = 0, ui32FsCfgBuf = 0, ui32FsStatusBuf = 0;
    uint32_t ui32TempIf = 0, ui32TempCfgBuf = 0, ui32TempStatusBuf =0;

    unsigned int i = 0, j = 0;

    BDBG_ENTER(BDSP_Raaga_P_CalculateStageMemory);

    for ( i=0; i < BDSP_Algorithm_eMax; i++ )
    {
        const BDSP_Raaga_P_AlgorithmSupportInfo *pInfo;
        pInfo = BDSP_Raaga_P_LookupAlgorithmSupported(i,pUsage->DolbyCodecVersion);
        if (( pUsage->Codeclist[i]!= true)||( pInfo->type != AlgorithmType))
        {
            continue;
        }

        ui32TempIf = 0;
        ui32TempCfgBuf = 0;
        ui32TempStatusBuf =0;

        /* Framesync buffer requirement is computed separately */
        algoId = pInfo->algoExecInfo.eAlgoIds[0];
        if ( algoId < BDSP_AF_P_AlgoId_eMax )
        {
            if (ui32FsIf < BDSP_sNodeInfo[algoId].ui32InterFrmBuffSize)
            {
                ui32FsIf = BDSP_sNodeInfo[algoId].ui32InterFrmBuffSize;
            }
            if (ui32FsCfgBuf < BDSP_sNodeInfo[algoId].ui32UserCfgBuffSize)
            {
                ui32FsCfgBuf = BDSP_sNodeInfo[algoId].ui32UserCfgBuffSize;
            }
            if (ui32FsStatusBuf < BDSP_sNodeInfo[algoId].ui32StatusBuffSize)
            {
                ui32FsStatusBuf = BDSP_sNodeInfo[algoId].ui32StatusBuffSize;
            }
        }

        /* Buffer requirement for the rest of the nodes */
        for( j=1; j < pInfo->algoExecInfo.NumNodes; j++ )
        {
            algoId = pInfo->algoExecInfo.eAlgoIds[j];
            if(algoId < BDSP_AF_P_AlgoId_eMax)
            {
                ui32TempIf += BDSP_sNodeInfo[algoId].ui32InterFrmBuffSize;
                ui32TempCfgBuf += BDSP_sNodeInfo[algoId].ui32UserCfgBuffSize;
                ui32TempStatusBuf += BDSP_sNodeInfo[algoId].ui32StatusBuffSize;
            }
        }

        if ( ui32TempIf > ui32AlgoIf )
        {
            ui32AlgoIf = ui32TempIf;
        }
        if ( ui32TempCfgBuf > ui32AlgoCfgBuf )
        {
            ui32AlgoCfgBuf = ui32TempCfgBuf;
        }
        if ( ui32TempStatusBuf > ui32AlgoStatusBuf )
        {
            ui32AlgoStatusBuf = ui32TempStatusBuf;
        }
    }

    /* Accounting Interframe */
    *pMemoryReq += (ui32AlgoIf + ui32FsIf);

    /* Accounting DSP User Config */
    *pMemoryReq += (ui32AlgoCfgBuf + ui32FsCfgBuf);

    /* Accounting HOST User Config */
    *pMemoryReq += (ui32AlgoCfgBuf + ui32FsCfgBuf);

    /* Accounting Status Buffer */
    *pMemoryReq += (ui32AlgoStatusBuf + ui32FsStatusBuf);

    /*Configuration structure allocations for a stage*/
    *pMemoryReq += (SIZEOF(BDSP_AF_P_sIO_BUFFER)+ SIZEOF(BDSP_AF_P_sIO_GENERIC_BUFFER))*(BDSP_AF_P_MAX_IP_FORKS+BDSP_AF_P_MAX_OP_FORKS);

    /* For IDS output buffer descriptors */
    *pMemoryReq += (SIZEOF(BDSP_AF_P_sIO_BUFFER)+ SIZEOF(BDSP_AF_P_sIO_GENERIC_BUFFER));

    BDBG_LEAVE(BDSP_Raaga_P_CalculateStageMemory);
    return err;
}

BERR_Code BDSP_Raaga_P_AllocateStageMemory(
    void *pStageHandle
    )
{
    BERR_Code err = BERR_SUCCESS;
	BDSP_MMA_Memory Buffer;
	uint32_t i;
	uint8_t *pAddr;
	dramaddr_t offset;

    BDSP_RaagaStage *pRaagaStage = (BDSP_RaagaStage *)pStageHandle;

    BDSP_RaagaContext   *pRaagaContext = (BDSP_RaagaContext   *)pRaagaStage->pContext;

    BDSP_Raaga *pDevice = (BDSP_Raaga *)pRaagaContext->pDevice;
    unsigned bytesreqd=0;

    err = BDSP_Raaga_P_CalcStageMemPoolReq ((void *)pRaagaStage);
    if(err != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Raaga_P_AllocateStageMemory: "
                    "Unable to get memory requirements for DSP!"));
        err = BERR_TRACE(err);
        goto end;
    }

	/*Interframe allocation for a stage*/
	pRaagaStage->sDramInterFrameBuffer.ui32Size = 0;;
	if( pRaagaStage->stageMemInfo.ui32InterframeBufReqd )
	{
		err = BDSP_MMA_P_AllocateAlignedMemory(pDevice->memHandle,
						pRaagaStage->stageMemInfo.ui32InterframeBufReqd,
						&(pRaagaStage->sDramInterFrameBuffer.Buffer),
						BDSP_MMA_Alignment_32bit);
		if(BERR_SUCCESS != err)
		{
			BDBG_ERR(("BDSP_Raaga_P_AllocateStageMemory: Unable to Allocate memory for IFrameCfgBuf!"));
			err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
			goto end;
		}
		pRaagaStage->sDramInterFrameBuffer.ui32Size = pRaagaStage->stageMemInfo.ui32InterframeBufReqd;
	}

	pRaagaStage->sDramUserConfigBuffer.ui32Size = 0;
	if( pRaagaStage->stageMemInfo.ui32UserConfigReqd )
	{
		err = BDSP_MMA_P_AllocateAlignedMemory(pDevice->memHandle,
						pRaagaStage->stageMemInfo.ui32UserConfigReqd,
						&(pRaagaStage->sDramUserConfigBuffer.Buffer),
						BDSP_MMA_Alignment_32bit);
		if(BERR_SUCCESS != err)
		{
			BDBG_ERR(("BDSP_Raaga_P_AllocateStageMemory: Unable to Allocate memory for Userconfig!"));
			err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
			goto end;
		}
		pRaagaStage->sDramUserConfigBuffer.ui32Size = pRaagaStage->stageMemInfo.ui32UserConfigReqd;
	}

	/* Allocate memory for the host buffer to hold the modified settings when the task is running */
	pRaagaStage->sDramUserConfigSpareBuffer.ui32Size = 0;
	if(pRaagaStage->stageMemInfo.ui32UserConfigReqd)
	{
		err = BDSP_MMA_P_AllocateAlignedMemory(pDevice->memHandle,
						pRaagaStage->stageMemInfo.ui32UserConfigReqd,
						&(pRaagaStage->sDramUserConfigSpareBuffer.Buffer),
						BDSP_MMA_Alignment_32bit);
		if(BERR_SUCCESS != err)
		{
			BDBG_ERR(("BDSP_Raaga_P_AllocateStageMemory: Unable to Allocate memory for Userconfig!"));
			err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
			goto end;
		}
		pRaagaStage->sDramUserConfigSpareBuffer.ui32Size = pRaagaStage->stageMemInfo.ui32UserConfigReqd;
	}

	/*Status Buffer allocation for a stage*/
	pRaagaStage->sDramStatusBuffer.ui32Size = 0;
	if(pRaagaStage->stageMemInfo.ui32StatusBufReqd)
	{
		err = BDSP_MMA_P_AllocateAlignedMemory(pDevice->memHandle,
						pRaagaStage->stageMemInfo.ui32StatusBufReqd,
						&(pRaagaStage->sDramStatusBuffer.Buffer),
						BDSP_MMA_Alignment_32bit);
		if(BERR_SUCCESS != err)
		{
			BDBG_ERR(("BDSP_Raaga_P_AllocateStageMemory: Unable to Allocate memory for Status Buffer!"));
			err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
			goto end;
		}
		pRaagaStage->sDramStatusBuffer.ui32Size = pRaagaStage->stageMemInfo.ui32StatusBufReqd;
	}

    /*Configuration structure allocations for a stage*/
    bytesreqd=(SIZEOF(BDSP_AF_P_sIO_BUFFER)+ SIZEOF(BDSP_AF_P_sIO_GENERIC_BUFFER))*(BDSP_AF_P_MAX_IP_FORKS+BDSP_AF_P_MAX_OP_FORKS);

    /* For IDS output buffer descriptors */
    bytesreqd+=(SIZEOF(BDSP_AF_P_sIO_BUFFER)+ SIZEOF(BDSP_AF_P_sIO_GENERIC_BUFFER));

	err = BDSP_MMA_P_AllocateAlignedMemory(pDevice->memHandle,
								bytesreqd,
								&Buffer,
								BDSP_MMA_Alignment_32bit);
	if(BERR_SUCCESS != err)
	{
		BDBG_ERR(("BDSP_Raaga_P_AllocateStageMemory: Unable to Allocate memory for IFrameCfgBuf!"));
		err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
		goto end;
	}

	pAddr = (uint8_t *)Buffer.pAddr;
	offset = (dramaddr_t)Buffer.offset;

	/*Split the memory for all the input and output descriptors in vitual memory itself as their contents needs to be populated later.*/
	for(i=0; i<BDSP_AF_P_MAX_IP_FORKS; i++)
	{
		pRaagaStage->sStageInput[i].IoBuffDesc.hBlock = Buffer.hBlock;
		pRaagaStage->sStageInput[i].IoBuffDesc.pAddr  = (void *)pAddr;
		pRaagaStage->sStageInput[i].IoBuffDesc.offset = offset;
		pAddr = pAddr + SIZEOF(BDSP_AF_P_sIO_BUFFER);
		offset = offset + SIZEOF(BDSP_AF_P_sIO_BUFFER);

		pRaagaStage->sStageInput[i].IoGenBuffDesc.hBlock = Buffer.hBlock;
		pRaagaStage->sStageInput[i].IoGenBuffDesc.pAddr  = (void *)pAddr;
		pRaagaStage->sStageInput[i].IoGenBuffDesc.offset = offset;
		pAddr = pAddr + SIZEOF(BDSP_AF_P_sIO_GENERIC_BUFFER);
		offset = offset + SIZEOF(BDSP_AF_P_sIO_GENERIC_BUFFER);
	}
	for(i=0; i<BDSP_AF_P_MAX_OP_FORKS; i++)
	{
		pRaagaStage->sStageOutput[i].IoBuffDesc.hBlock = Buffer.hBlock;
		pRaagaStage->sStageOutput[i].IoBuffDesc.pAddr  = (void *)pAddr;
		pRaagaStage->sStageOutput[i].IoBuffDesc.offset = offset;
		pAddr = pAddr + SIZEOF(BDSP_AF_P_sIO_BUFFER);
		offset = offset + SIZEOF(BDSP_AF_P_sIO_BUFFER);

		pRaagaStage->sStageOutput[i].IoGenBuffDesc.hBlock = Buffer.hBlock;
		pRaagaStage->sStageOutput[i].IoGenBuffDesc.pAddr  = (void *)pAddr;
		pRaagaStage->sStageOutput[i].IoGenBuffDesc.offset = offset;
		pAddr = pAddr + SIZEOF(BDSP_AF_P_sIO_GENERIC_BUFFER);
		offset = offset + SIZEOF(BDSP_AF_P_sIO_GENERIC_BUFFER);

		/*mark all of them invalid till stages are connected to make them valid*/
		pRaagaStage->eStageOpBuffDataType[i] = BDSP_AF_P_DistinctOpType_eInvalid;
	}

	pRaagaStage->sIdsStageOutput.IoBuffDesc.hBlock = Buffer.hBlock;
	pRaagaStage->sIdsStageOutput.IoBuffDesc.pAddr = (void *)pAddr;
	pRaagaStage->sIdsStageOutput.IoBuffDesc.offset = offset;

	pAddr = pAddr + SIZEOF(BDSP_AF_P_sIO_BUFFER);
	offset = offset + SIZEOF(BDSP_AF_P_sIO_BUFFER);

	pRaagaStage->sIdsStageOutput.IoGenBuffDesc.hBlock = Buffer.hBlock;
	pRaagaStage->sIdsStageOutput.IoGenBuffDesc.pAddr = (void *)pAddr;
	pRaagaStage->sIdsStageOutput.IoGenBuffDesc.offset = offset;

	pRaagaStage->sIdsStageOutput.StageIOBuffDescAddr = pRaagaStage->sIdsStageOutput.IoBuffDesc.offset;
	pRaagaStage->sIdsStageOutput.StageIOGenericBuffDescAddr = pRaagaStage->sIdsStageOutput.IoGenBuffDesc.offset;

    /*if decode stage, during task start, the FS as well as the decode node info should be populated with the same buffer*/

end:
    if(err != BERR_SUCCESS)
    {
        BDSP_Raaga_P_FreeStageMemory(pStageHandle);
    }

    return err;
}

BERR_Code BDSP_Raaga_P_CalculateInterTaskBufferMemory(
    unsigned *pMemoryReq,
    unsigned Numchannels
    )
{
    BERR_Code err = BERR_SUCCESS;

    *pMemoryReq += (Numchannels*BDSP_AF_P_INTERTASK_IOBUFFER_SIZE);

    /* IO Generic buffer size required */
    *pMemoryReq += BDSP_AF_P_INTERTASK_IOGENBUFFER_SIZE;

    /* Allocation for Descirptors */
    *pMemoryReq += sizeof(BDSP_AF_P_sIO_BUFFER) + sizeof(BDSP_AF_P_sIO_GENERIC_BUFFER);

    return err;
}


BERR_Code BDSP_Raaga_P_CalculateTaskMemory(
    unsigned *pMemoryReq
    )
{
    BERR_Code err = BERR_SUCCESS;
    /* Task Port Config Memory Allocation */
    *pMemoryReq += (BDSP_CIT_P_TASK_PORT_CONFIG_MEM_SIZE + /*BDSP_CIT_P_TASK_SPDIF_USER_CFG_MEM_SIZE*/
                    BDSP_CIT_P_TASK_FMM_GATE_OPEN_CONFIG +
                    BDSP_CIT_P_TASK_HW_FW_CONFIG +
                    BDSP_CIT_P_TASK_FS_MAPPING_LUT_SIZE +
                    BDSP_CIT_P_TASK_STC_TRIG_CONFIG_SIZE);

    /*Memory for Sync Response Queue */
    *pMemoryReq += (BDSP_RAAGA_MAX_MSGS_PER_QUEUE * sizeof(BDSP_Raaga_P_Response));

    /*Memory for Async Response Queue */
    *pMemoryReq += (BDSP_RAAGA_MAX_ASYNC_MSGS_PER_QUEUE * sizeof(BDSP_Raaga_P_AsynEventMsg));

    /*Memory for Task Swapping */
    *pMemoryReq += BDSP_CIT_P_TASK_SWAP_BUFFER_SIZE_INBYTES;

    /*Memory for Task  Gate Open Buffer Configuration Swapping */
    *pMemoryReq += (BDSP_AF_P_MAX_OP_FORKS * SIZEOF(BDSP_AF_P_sIO_BUFFER));

    /*Memory for Task Parameters */
    *pMemoryReq += sizeof(BDSP_P_TaskParamInfo);

    /* Memory for CIT structure */
    *pMemoryReq += sizeof(BDSP_AF_P_sTASK_CONFIG);

    /* Memory for SPARE CIT structure */
    *pMemoryReq += sizeof(BDSP_AF_P_sTASK_CONFIG);

    /*Memory for Feedback buffer for Master Task */
    *pMemoryReq += (BDSP_AF_P_INTERTASK_FEEDBACK_BUFFER_SIZE + 4);
    return err;
}

BERR_Code BDSP_Raaga_P_AllocateTaskMemory(
    void *pTaskHandle,
    const BDSP_TaskCreateSettings *pSettings
    )
{
    BERR_Code err = BERR_SUCCESS;
    uint32_t ui32UsedSize = 0;
    BDSP_RaagaTask   *pRaagaTask = (BDSP_RaagaTask   *)pTaskHandle;
    BDSP_RaagaContext   *pRaagaContext = (BDSP_RaagaContext   *)pRaagaTask->pContext;
    BDSP_Raaga *pDevice = (BDSP_Raaga *)pRaagaContext->pDevice;

    BDBG_ENTER(BDSP_Raaga_P_AllocateTaskMemory);

    BDBG_OBJECT_ASSERT(pRaagaTask, BDSP_RaagaTask);

    /*ui32TaskPortConfig + */
    ui32UsedSize =  BDSP_CIT_P_TASK_PORT_CONFIG_MEM_SIZE + /*BDSP_CIT_P_TASK_SPDIF_USER_CFG_MEM_SIZE*/
                    BDSP_CIT_P_TASK_FMM_GATE_OPEN_CONFIG +
                    BDSP_CIT_P_TASK_HW_FW_CONFIG +
                    BDSP_CIT_P_TASK_FS_MAPPING_LUT_SIZE +
                    BDSP_CIT_P_TASK_STC_TRIG_CONFIG_SIZE;

	err = BDSP_MMA_P_AllocateAlignedMemory(pDevice->memHandle,
						ui32UsedSize,
						&(pRaagaTask->taskMemGrants.sTaskCfgBufInfo.Buffer),
						BDSP_MMA_Alignment_32bit);
    if(BERR_SUCCESS != err)
    {
        BDBG_ERR(("BDSP_Raaga_P_AllocateTaskMemory: Unable to Allocate memory for TaskCfgBuf!"));
        err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
        goto end;
    }
    pRaagaTask->taskMemGrants.sTaskCfgBufInfo.ui32Size = ui32UsedSize;

    /* Sync Queue */
	err = BDSP_MMA_P_AllocateAlignedMemory(pDevice->memHandle,
						(BDSP_RAAGA_MAX_MSGS_PER_QUEUE * sizeof(BDSP_Raaga_P_Response)),
						&(pRaagaTask->taskMemGrants.sTaskQueue.sTaskSyncQueue.Queue),
						BDSP_MMA_Alignment_32bit);
    if(BERR_SUCCESS != err)
    {
        BDBG_ERR(("BDSP_Raaga_P_AllocateTaskMemory: Unable to Allocate memory for sync task queue!"));
        err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
        goto end;
    }
    pRaagaTask->taskMemGrants.sTaskQueue.sTaskSyncQueue.uiMsgQueueSize = BDSP_RAAGA_MAX_MSGS_PER_QUEUE * sizeof(BDSP_Raaga_P_Response);

    err = BDSP_Raaga_P_AssignFreeFIFO(pDevice,pSettings->dspIndex,&(pRaagaTask->taskMemGrants.sTaskQueue.sTaskSyncQueue.i32FifoId), 1);
    if(err)
    {
        BDBG_ERR(("Unable to find free fifo for SYNC QUEUE!!!!"));
        goto end;
    }

    /* Async Queue */
	err = BDSP_MMA_P_AllocateAlignedMemory(pDevice->memHandle,
						(BDSP_RAAGA_MAX_ASYNC_MSGS_PER_QUEUE * sizeof(BDSP_Raaga_P_AsynEventMsg)),
						&(pRaagaTask->taskMemGrants.sTaskQueue.sTaskAsyncQueue.Queue),
						BDSP_MMA_Alignment_32bit);
    if(BERR_SUCCESS != err)
    {
        BDBG_ERR(("BDSP_Raaga_P_AllocateTaskMemory: Unable to Allocate memory for async task queue!"));
        err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
        goto end;
    }
    pRaagaTask->taskMemGrants.sTaskQueue.sTaskAsyncQueue.uiMsgQueueSize = BDSP_RAAGA_MAX_ASYNC_MSGS_PER_QUEUE * sizeof(BDSP_Raaga_P_AsynEventMsg);

    err = BDSP_Raaga_P_AssignFreeFIFO(pDevice,pSettings->dspIndex,&(pRaagaTask->taskMemGrants.sTaskQueue.sTaskAsyncQueue.i32FifoId), 1);
    if(err)
    {
        BDBG_ERR(("Unable to find free fifo for ASYNC QUEUE!!!!"));
        goto end;
    }

    /* Async MSG Buf Memory */
    pRaagaTask->taskMemGrants.sTaskQueue.sAsyncMsgBufmem.pAddr = BKNI_Malloc((BDSP_RAAGA_MAX_ASYNC_MSGS_PER_QUEUE * sizeof(BDSP_Raaga_P_AsynEventMsg)));
    if(NULL == pRaagaTask->taskMemGrants.sTaskQueue.sAsyncMsgBufmem.pAddr)
    {
        BDBG_ERR(("BDSP_Raaga_P_AllocateTaskMemory: Unable to Allocate memory for async msgs!"));
        err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
        goto end;
    }
    pRaagaTask->taskMemGrants.sTaskQueue.sAsyncMsgBufmem.ui32Size = BDSP_RAAGA_MAX_ASYNC_MSGS_PER_QUEUE * sizeof(BDSP_Raaga_P_AsynEventMsg);

    /* Task stack swap memory */
	err = BDSP_MMA_P_AllocateAlignedMemory(pDevice->memHandle,
						BDSP_CIT_P_TASK_SWAP_BUFFER_SIZE_INBYTES,
						&(pRaagaTask->taskMemGrants.sStackSwapBuff.Buffer),
						BDSP_MMA_Alignment_32bit);
    if(BERR_SUCCESS != err)
    {
        BDBG_ERR(("BDSP_Raaga_P_AllocateTaskMemory: Unable to Allocate memory for task stack swap!"));
        err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
        goto end;
    }
    pRaagaTask->taskMemGrants.sStackSwapBuff.ui32Size = BDSP_CIT_P_TASK_SWAP_BUFFER_SIZE_INBYTES;

    /*Gate Open Buffer configuration memory */
	err = BDSP_MMA_P_AllocateAlignedMemory(pDevice->memHandle,
						(BDSP_AF_P_MAX_OP_FORKS * SIZEOF(BDSP_AF_P_sIO_BUFFER)),
						&(pRaagaTask->taskMemGrants.sTaskGateOpenBufInfo.Buffer),
						BDSP_MMA_Alignment_32bit);
    if(BERR_SUCCESS != err)
    {
        BDBG_ERR(("BDSP_Raaga_P_AllocateTaskMemory: Unable to Allocate memory for task Gate Open Configuration!"));
        err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
        goto end;
    }
    pRaagaTask->taskMemGrants.sTaskGateOpenBufInfo.ui32Size = (BDSP_AF_P_MAX_OP_FORKS * SIZEOF(BDSP_AF_P_sIO_BUFFER));

    if(pRaagaContext->settings.contextType == BDSP_ContextType_eVideo)
    {

        /* Cit memory. This is now context dependent as the sizes changes for different context */
		err = BDSP_MMA_P_AllocateAlignedMemory(pDevice->memHandle,
							sizeof(BDSP_VF_P_sDEC_TASK_CONFIG),
							&(pRaagaTask->taskMemGrants.sCitStruct.Buffer),
							BDSP_MMA_Alignment_32bit);
		if(BERR_SUCCESS != err)
        {
            BDBG_ERR(("BDSP_Raaga_P_AllocateTaskMemory: Unable to Allocate memory for cit!"));
            err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
            goto end;
        }
        pRaagaTask->taskMemGrants.sCitStruct.ui32Size = sizeof(BDSP_VF_P_sDEC_TASK_CONFIG);

        /* Video does not require a spare cit buffer as it does not support seamless input/output port switching */
        pRaagaTask->taskMemGrants.sSpareCitStruct.Buffer.pAddr = 0;
        pRaagaTask->taskMemGrants.sSpareCitStruct.ui32Size = 0;

        /* Allocate Picture delivery queue */
		err = BDSP_MMA_P_AllocateAlignedMemory(pDevice->memHandle,
							BDSP_RAAGA_REALVIDEO_MAX_MSGS_PER_QUEUE*4,
							&(pRaagaTask->taskMemGrants.sTaskQueue.sPDQueue.Queue),
							BDSP_MMA_Alignment_32bit);
		if(BERR_SUCCESS != err)
        {
            BDBG_ERR(("BDSP_DSP_AllocMem: Unable to Allocate memory for Picture delivery queue!"));
            err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
            goto end;
        }
        pRaagaTask->taskMemGrants.sTaskQueue.sPDQueue.uiMsgQueueSize = BDSP_RAAGA_REALVIDEO_MAX_MSGS_PER_QUEUE * 4;

        err = BDSP_Raaga_P_AssignFreeFIFO(pDevice,pSettings->dspIndex,&(pRaagaTask->taskMemGrants.sTaskQueue.sPDQueue.i32FifoId), 1 );
        if(err)
        {
            BDBG_ERR(("Unable to find free fifo for PDQ QUEUE!!!!"));
            goto end;
        }

        /* Allocate Picture release queue */
		err = BDSP_MMA_P_AllocateAlignedMemory(pDevice->memHandle,
							 BDSP_RAAGA_REALVIDEO_MAX_MSGS_PER_QUEUE*4,
							&(pRaagaTask->taskMemGrants.sTaskQueue.sPRQueue.Queue),
							BDSP_MMA_Alignment_32bit);
		if(BERR_SUCCESS != err)
        {
            BDBG_ERR(("BDSP_DSP_AllocMem: Unable to Allocate memory for Picture Release queue!"));
            err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
            goto end;
        }
        pRaagaTask->taskMemGrants.sTaskQueue.sPRQueue.uiMsgQueueSize = BDSP_RAAGA_REALVIDEO_MAX_MSGS_PER_QUEUE * 4;

        err = BDSP_Raaga_P_AssignFreeFIFO(pDevice,pSettings->dspIndex,&(pRaagaTask->taskMemGrants.sTaskQueue.sPRQueue.i32FifoId), 1);
        if(err)
        {
            BDBG_ERR(("Unable to find free fifo for PR QUEUE!!!!"));
            goto end;
        }

        /* Allocate Display  queue */
		err = BDSP_MMA_P_AllocateAlignedMemory(pDevice->memHandle,
							 BDSP_RAAGA_REALVIDEO_MAX_MSGS_PER_QUEUE*4,
							&(pRaagaTask->taskMemGrants.sTaskQueue.sDSQueue.Queue),
							BDSP_MMA_Alignment_32bit);
		if(BERR_SUCCESS != err)
        {
            BDBG_ERR(("BDSP_DSP_AllocMem: Unable to Allocate memory for DS queue!"));
            err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
            goto end;
        }
        pRaagaTask->taskMemGrants.sTaskQueue.sDSQueue.uiMsgQueueSize = BDSP_RAAGA_REALVIDEO_MAX_MSGS_PER_QUEUE * 4;

        err = BDSP_Raaga_P_AssignFreeFIFO(pDevice,pSettings->dspIndex,&(pRaagaTask->taskMemGrants.sTaskQueue.sDSQueue.i32FifoId), 1);
        if(err)
        {
            BDBG_ERR(("Unable to find free fifo for DS QUEUE!!!!"));
            goto end;
        }
    }
    else if(BDSP_ContextType_eVideoEncode == pRaagaContext->settings.contextType )
    {
        /* Cit memory. This is now context dependent as the sizes changes for different context */
		err = BDSP_MMA_P_AllocateAlignedMemory(pDevice->memHandle,
							 sizeof(BDSP_VF_P_sENC_TASK_CONFIG),
							&(pRaagaTask->taskMemGrants.sCitStruct.Buffer),
							BDSP_MMA_Alignment_32bit);
        if(BERR_SUCCESS != err)
		{
            BDBG_ERR(("BDSP_Raaga_P_AllocateTaskMemory: Unable to Allocate memory for cit!"));
            err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
            goto end;
        }
        pRaagaTask->taskMemGrants.sCitStruct.ui32Size = sizeof(BDSP_VF_P_sENC_TASK_CONFIG);

        /* Video does not require a spare cit buffer as it does not support seamless input/output port switching */
        pRaagaTask->taskMemGrants.sSpareCitStruct.Buffer.pAddr= 0;
        pRaagaTask->taskMemGrants.sSpareCitStruct.ui32Size = 0;

        pRaagaTask->taskMemGrants.sTaskQueue.sRDQueue.uiMsgQueueSize = BDSP_RAAGA_REALVIDEO_MAX_MSGS_PER_QUEUE * 4;
        err = BDSP_Raaga_P_AssignFreeFIFO(pDevice,pSettings->dspIndex,&(pRaagaTask->taskMemGrants.sTaskQueue.sRDQueue.i32FifoId), 1);
        if(err)
        {
            BDBG_ERR(("Unable to find free fifo for RD QUEUE!!!!"));
            goto end;
        }

        /* Allocate Raw Picture release queue */
		err = BDSP_MMA_P_AllocateAlignedMemory(pDevice->memHandle,
							 BDSP_RAAGA_REALVIDEO_MAX_MSGS_PER_QUEUE*4,
							&(pRaagaTask->taskMemGrants.sTaskQueue.sRRQueue.Queue),
							BDSP_MMA_Alignment_32bit);
        if(BERR_SUCCESS != err)
        {
            BDBG_ERR(("BDSP_DSP_AllocMem: Unable to Allocate memory for Raw Picture Release queue!"));
            err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
            goto end;
        }
        pRaagaTask->taskMemGrants.sTaskQueue.sRRQueue.uiMsgQueueSize = BDSP_RAAGA_REALVIDEO_MAX_MSGS_PER_QUEUE * 4;
        err = BDSP_Raaga_P_AssignFreeFIFO(pDevice,pSettings->dspIndex,&(pRaagaTask->taskMemGrants.sTaskQueue.sRRQueue.i32FifoId), 1);
        if(err)
        {
            BDBG_ERR(("Unable to find free fifo for RR QUEUE!!!!"));
            goto end;
        }

        /* Allocate CC data input queue */
		err = BDSP_MMA_P_AllocateAlignedMemory(pDevice->memHandle,
							BDSP_RAAGA_CC_DATA_MSGS_PER_QUEUE*sizeof(BDSP_Raaga_Video_DCCparse_ccdata),
							&(pRaagaTask->taskMemGrants.sTaskQueue.sCCDQueue.Queue),
							BDSP_MMA_Alignment_32bit);
        if(BERR_SUCCESS != err)
        {
            BDBG_ERR(("BDSP_DSP_AllocMem: Unable to Allocate memory for CC data queue!"));
            err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
            goto end;
        }
        pRaagaTask->taskMemGrants.sTaskQueue.sCCDQueue.uiMsgQueueSize = BDSP_RAAGA_CC_DATA_MSGS_PER_QUEUE*sizeof(BDSP_Raaga_Video_DCCparse_ccdata);
        err = BDSP_Raaga_P_AssignFreeFIFO(pDevice,pSettings->dspIndex,&(pRaagaTask->taskMemGrants.sTaskQueue.sCCDQueue.i32FifoId), 1);
        if(err)
        {
            BDBG_ERR(("Unable to find free fifo for CCD QUEUE!!!!"));
            goto end;
        }
    }
    else if(BDSP_ContextType_eScm == pRaagaContext->settings.contextType )
    {
        /* Cit memory. This is now context dependent as the sizes changes for different context */
		err = BDSP_MMA_P_AllocateAlignedMemory(pDevice->memHandle,
							sizeof(BDSP_SCM_P_sTASK_CONFIG),
							&(pRaagaTask->taskMemGrants.sCitStruct.Buffer),
							BDSP_MMA_Alignment_32bit);
        if(BERR_SUCCESS != err)
        {
            BDBG_ERR(("BDSP_Raaga_P_AllocateTaskMemory: Unable to Allocate memory for cit!"));
            err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
            goto end;
        }
        pRaagaTask->taskMemGrants.sCitStruct.ui32Size = sizeof(BDSP_SCM_P_sTASK_CONFIG);

        /* Security does not require a spare cit buffer as it does not support seamless input/output port switching */
        pRaagaTask->taskMemGrants.sSpareCitStruct.Buffer.pAddr = 0;
        pRaagaTask->taskMemGrants.sSpareCitStruct.ui32Size = 0;
    }

    /* Start Task memory */
	err = BDSP_MMA_P_AllocateAlignedMemory(pDevice->memHandle,
						sizeof(BDSP_P_TaskParamInfo),
						&(pRaagaTask->taskMemGrants.sTaskInfo.Buffer),
						BDSP_MMA_Alignment_32bit);
	if(BERR_SUCCESS != err)
    {
        BDBG_ERR(("BDSP_Raaga_P_AllocateTaskMemory: Unable to Allocate memory for start task params!"));
        err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
        goto end;
    }
    pRaagaTask->taskMemGrants.sTaskInfo.ui32Size = sizeof(BDSP_P_TaskParamInfo);

    if(pRaagaContext->settings.contextType == BDSP_ContextType_eAudio)
    {
        /* Cit memory. This is now context dependent as the sizes changes for different context */
		err = BDSP_MMA_P_AllocateAlignedMemory(pDevice->memHandle,
							sizeof(BDSP_AF_P_sTASK_CONFIG),
							&(pRaagaTask->taskMemGrants.sCitStruct.Buffer),
							BDSP_MMA_Alignment_32bit);
		if(BERR_SUCCESS != err)
        {
            BDBG_ERR(("BDSP_Raaga_P_AllocateTaskMemory: Unable to Allocate memory for cit!"));
            err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
            goto end;
        }
        pRaagaTask->taskMemGrants.sCitStruct.ui32Size = sizeof(BDSP_AF_P_sTASK_CONFIG);

        /* Allocate memory for the spare cit structure used as working buffer by basemodules */
		err = BDSP_MMA_P_AllocateAlignedMemory(pDevice->memHandle,
							sizeof(BDSP_AF_P_sTASK_CONFIG),
							&(pRaagaTask->taskMemGrants.sSpareCitStruct.Buffer),
							BDSP_MMA_Alignment_32bit);
        if(BERR_SUCCESS != err)
		{
            BDBG_ERR(("BDSP_Raaga_P_AllocateTaskMemory: Unable to Allocate memory for cit!"));
            err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
            goto end;
        }
        pRaagaTask->taskMemGrants.sSpareCitStruct.ui32Size = sizeof(BDSP_AF_P_sTASK_CONFIG);

        if(pRaagaTask->settings.numSrc>BDSP_MAX_INPUTS_PER_TASK || pRaagaTask->settings.numDst>BDSP_MAX_OUTPUTS_PER_TASK)
        {
            BDBG_ERR(("BDSP_Raaga_P_AllocateTaskMemory: Number of Input or Outputs or both out of order"));
            goto end;
        }
    }
    /* Better to Initialise the variables as they may not be used at all depending on the task type and configuration*/
	pRaagaTask->FeedbackBuffer.pAddr = NULL;
	pRaagaTask->FeedbackBuffer.offset = 0;
    pRaagaTask->hPDQueue = NULL;
    pRaagaTask->hPRQueue = NULL;
    pRaagaTask->hDSQueue = NULL;
    pRaagaTask->hRDQueue = NULL;
    pRaagaTask->hRRQueue = NULL;
    pRaagaTask->hCCDQueue = NULL;

    /*Create the Feedback Buffer required for the master task*/
    if(pSettings->masterTask == true)
    {
		err = BDSP_MMA_P_AllocateAlignedMemory(pDevice->memHandle,
							(BDSP_AF_P_INTERTASK_FEEDBACK_BUFFER_SIZE + 4),
							&(pRaagaTask->FeedbackBuffer),
							BDSP_MMA_Alignment_32bit);
        if(BERR_SUCCESS != err)
		{
			BDBG_ERR(("Unable to Allocate memory for Feedback  Buffer !"));
			err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
			goto end;
		}
		BKNI_Memset((void *)pRaagaTask->FeedbackBuffer.pAddr, 0x0, (BDSP_AF_P_INTERTASK_FEEDBACK_BUFFER_SIZE + 4));
		BDSP_MMA_P_FlushCache(pRaagaTask->FeedbackBuffer,(BDSP_AF_P_INTERTASK_FEEDBACK_BUFFER_SIZE + 4));
    }
    /* Create the Queues required for the task */
    err = BDSP_Raaga_P_CreateTaskQueues( pTaskHandle, pSettings->dspIndex);
    if (BERR_SUCCESS != err)
    {
        BDBG_ERR(("BDSP_Raaga_P_InitParams_CreateTask: Unable to create queues"));
        err = BERR_TRACE(err);
    }

end:
    if(err != BERR_SUCCESS)
    {
        BDSP_Raaga_P_FreeTaskMemory(pTaskHandle);
    }
    BDBG_LEAVE(BDSP_Raaga_P_AllocateTaskMemory);
return err;
}

BERR_Code BDSP_Raaga_P_CalculateContextMemory(
    unsigned *pMemoryReq
    )
{
    BERR_Code err = BERR_SUCCESS;
    *pMemoryReq = sizeof(BDSP_VOM_Table);
    return err;
}

#if (BCHP_CHIP == 7278)
/* Calculates total memory required for the following
 *        -  RW memory required for kernel
 *        - Shared RW memory between cores (kernel)
 *        - RW memory used by each of the processes
 *        - RW memory required Kernel process */
BERR_Code BDSP_Raaga_P_CalculateProcRWMemory(
    unsigned *procMemoryReq,
    int32_t i32NumDsp
    )
{
    BERR_Code err = BERR_SUCCESS;
    unsigned reqMem = 0;

    reqMem = (BDSP_RAAGA_OTHER_PROC_MEM_SIZE * BDSP_AF_P_Process_eMaxProcess) + /* Spawning process */
            BDSP_RAAGA_MM_PROC_HEAP_SIZE + BDSP_RAAGA_INIT_PROC_MEM_SIZE +
            BDSP_RAAGA_APP_PROC_MEM_SIZE;

    reqMem = reqMem * i32NumDsp;

    /* This is for dual core and shared memory together */
    reqMem += BDSP_RAAGA_KERNEL_RW_MEM_SIZE;

    *procMemoryReq = reqMem;
    return err;
}

BERR_Code BDSP_Raaga_P_AssignFromRWMem(
    void *pDeviceHandle,
    uint32_t ui32ReqSize,
    BDSP_MMA_Memory *pBuffer
    )
{
    BERR_Code err = BERR_SUCCESS;
    BDSP_Raaga *pDevice = NULL;
    uint32_t ui32AvailableMem = 0;
    BDSP_P_FwBuffer *pRwMem = NULL;

    BDBG_ENTER(BDSP_Raaga_P_AssignFromRWMem);

    pDevice = (BDSP_Raaga *) pDeviceHandle;
    BDBG_OBJECT_ASSERT(pDevice, BDSP_Raaga);

    pRwMem = &pDevice->memInfo.sRaagaRWMemoryBuf;
    ui32AvailableMem = pRwMem->ui32Size - pDevice->memInfo.ui32UsedRWMemsize;

    if(ui32AvailableMem < ui32ReqSize)
    {
        BDBG_ERR(("BDSP_Raaga_P_AssignFromRWMem: Insuffficient RW Memory!"));
        err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
    }
    else
    {
        /* To get 4-Byte (32-bit) aligned address */
        if(pDevice->memInfo.ui32UsedRWMemsize & 0x3)
        {
            pDevice->memInfo.ui32UsedRWMemsize += 4 - (pDevice->memInfo.ui32UsedRWMemsize & 0x3);
        }
        pBuffer->pAddr = (void * )((uint8_t *)(pRwMem->Buffer.pAddr) + pDevice->memInfo.ui32UsedRWMemsize);
        pBuffer->hBlock = pRwMem->Buffer.hBlock;
        pBuffer->offset = pRwMem->Buffer.offset + pDevice->memInfo.ui32UsedRWMemsize;
        /*BDBG_MSG(("hBlock:%lx, pAddr :%p, offset : " BDSP_MSG_FMT, (uintptr_t) pBuffer->hBlock, pBuffer->pAddr, BDSP_MSG_ARG(pBuffer->offset)));*/
        pDevice->memInfo.ui32UsedRWMemsize += ui32ReqSize;
    }

    BDBG_LEAVE(BDSP_Raaga_P_AssignFromRWMem);
    return err;
}

/* Allocates memory from the RW memory buffer
 * Allocates, Core0 and Core1 RW memory and shared RW memory */
BERR_Code BDSP_Raaga_P_AllocCoresRWMemory(
    void *pDeviceHandle
    )
{
    BERR_Code err = BERR_SUCCESS;
    BDSP_MMA_Memory sBuffer;
    uint32_t ui32reqSize = 0;

    BDSP_Raaga *pDevice = (BDSP_Raaga *)pDeviceHandle;
    BDBG_OBJECT_ASSERT(pDevice, BDSP_Raaga);

    BDBG_ENTER(BDSP_Raaga_P_AllocCoresRWMemory);
    ui32reqSize = BDSP_RAAGA_KERNEL_RW_MEM_SIZE;

    /* Allocated memory is a place holder for the Kernel rw memory and shared memory */
    /* Firmware memory allocation will start after this, as per the M2 Design */
    err = BDSP_Raaga_P_AssignFromRWMem(pDeviceHandle, ui32reqSize, &sBuffer);
    if(BERR_SUCCESS != err)
    {
        BDBG_ERR(("BDSP_Raaga_P_AllocCoresRWMemory: Insufficient RW memory"));
        BERR_TRACE(err);
        goto end;
    }

end:
    BDBG_LEAVE(BDSP_Raaga_P_AllocCoresRWMemory);
    return err;
    }

/* Allocates memory from the RW memory buffer
 * Allocates, Memory for each of the Raaga processes
 * Init, MM, Fileserver, ProcesManager, and application processes */
BERR_Code BDSP_Raaga_P_AllocProcessRWMemory(
    void *pDeviceHandle
    )
{
    BERR_Code err = BERR_SUCCESS;
    BDSP_MMA_Memory sBuffer;
    uint32_t ui32reqSize = 0;

    BDSP_Raaga *pDevice = (BDSP_Raaga *)pDeviceHandle;
    BDBG_OBJECT_ASSERT(pDevice, BDSP_Raaga);

    BDBG_ENTER(BDSP_Raaga_P_AllocProcessRWMemory);

    /* Allocated memory is a place holder for the memory for all the processes */
    /* We do not assign to any of the memories */
    /* calculate the memory size required by all the processes */
    ui32reqSize = (BDSP_RAAGA_OTHER_PROC_MEM_SIZE * BDSP_AF_P_Process_eMaxProcess) +
                  BDSP_RAAGA_MM_PROC_HEAP_SIZE + BDSP_RAAGA_INIT_PROC_MEM_SIZE +
                  BDSP_RAAGA_APP_PROC_MEM_SIZE;

    err = BDSP_Raaga_P_AssignFromRWMem(pDeviceHandle, ui32reqSize, &sBuffer);
    if(BERR_SUCCESS != err)
    {
        BDBG_ERR(("BDSP_Raaga_P_AllocProcessRWMemory: Insufficient RW memory"));
        BERR_TRACE(err);
        goto end;
    }
end:
    BDBG_LEAVE(BDSP_Raaga_P_AllocProcessRWMemory);
    return err;

}
#endif /* (BCHP_CHIP == 7278) */

BERR_Code BDSP_Raaga_P_AllocateContextMemory(
    void *pContextHandle
    )
{
    BERR_Code err = BERR_SUCCESS;
    BDSP_RaagaContext   *pRaagaContext = (BDSP_RaagaContext   *)pContextHandle;
    BDSP_Raaga *pDevice = (BDSP_Raaga *)pRaagaContext->pDevice;

    BDBG_ENTER(BDSP_Raaga_P_AllocateContextMemory);

    BDBG_OBJECT_ASSERT(pRaagaContext, BDSP_RaagaContext);
    BDBG_OBJECT_ASSERT(pDevice, BDSP_Raaga);

    /* VOM Table DRAM */
	err = BDSP_MMA_P_AllocateAlignedMemory (pDevice->memHandle,
						sizeof(BDSP_VOM_Table),
						&(pRaagaContext->contextMemInfo.sVomTableInfo.Buffer),
						BDSP_MMA_Alignment_32bit);
	if(err != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Raaga_P_AllocateContextMemory: Unable to Allocate memory for VOM Table in DRAM!"));
		err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
		goto end;
	}

	pRaagaContext->contextMemInfo.sVomTableInfo.ui32Size = sizeof(BDSP_VOM_Table);
end:
    if(err != BERR_SUCCESS)
    {
#if (BCHP_CHIP != 7278)
        BDSP_Raaga_P_FreeContextMemory(pContextHandle);
#endif
    }
    BDBG_LEAVE(BDSP_Raaga_P_AllocateContextMemory);
    return err;
}

#if (BCHP_CHIP == 7278)
/* This function Allocate and assigns the memory from the RW memory chunk */
BERR_Code BDSP_Raaga_P_AllocateFWSharedMem (
    void *pDeviceHandle,
    int32_t i32DspIndex
    )
{
    BERR_Code err = BERR_SUCCESS;
    unsigned j=0;
    uint32_t ui32VrtlAddrOffset = 0;
    BDSP_MMA_Memory *pBuffer = NULL;
    BDSP_MMA_Memory *pDstBuffer = NULL;

    BDSP_Raaga *pDevice = (BDSP_Raaga *)pDeviceHandle;

    BDBG_ENTER(BDSP_Raaga_P_AllocateFWSharedMem);

    BDBG_OBJECT_ASSERT(pDevice, BDSP_Raaga);

    /* Allocate Task Cmd/Resp Queues only for single core
     * for each core loop is run in the calling function */
    {
        /* Allocate Command queue */
        err = BDSP_Raaga_P_AssignFromRWMem(pDeviceHandle, (BDSP_RAAGA_MAX_MSGS_PER_QUEUE * sizeof(BDSP_Raaga_P_Command)*BDSP_RAAGA_MAX_FW_TASK_PER_DSP),
                &pDevice->memInfo.cmdQueueParams[i32DspIndex].Queue);
        if(err != BERR_SUCCESS)
        {
            BDBG_ERR(("BDSP_DSP_AllocMem: Unable to Allocate memory for cmd queue!"));
            BERR_TRACE(err);
            goto err_alloc_CmdQueue;
        }
        pDevice->memInfo.cmdQueueParams[i32DspIndex].uiMsgQueueSize = BDSP_RAAGA_MAX_MSGS_PER_QUEUE * sizeof(BDSP_Raaga_P_Command)*BDSP_RAAGA_MAX_FW_TASK_PER_DSP;

        err = BDSP_Raaga_P_AssignFreeFIFO(pDevice,(unsigned)i32DspIndex,&(pDevice->memInfo.cmdQueueParams[i32DspIndex].i32FifoId), 1);
        if(err)
        {
            BDBG_ERR(("Unable to find free fifo for CMD QUEUE!!!!"));
            goto err_alloc_CmdQueue;
        }

        /* Allocate Generic Response queue */
        err = BDSP_Raaga_P_AssignFromRWMem(pDeviceHandle, (BDSP_RAAGA_MAX_MSGS_PER_QUEUE * sizeof(BDSP_Raaga_P_Response)),
                &pDevice->memInfo.genRspQueueParams[i32DspIndex].Queue);
        if(err != BERR_SUCCESS)
        {
            BDBG_ERR(("BDSP_DSP_AllocMem: Unable to Allocate memory for generic response queue!"));
            BERR_TRACE(err);
            goto err_alloc_genRespQueue;
        }
        pDevice->memInfo.genRspQueueParams[i32DspIndex].uiMsgQueueSize = BDSP_RAAGA_MAX_MSGS_PER_QUEUE * sizeof(BDSP_Raaga_P_Response);

        err = BDSP_Raaga_P_AssignFreeFIFO(pDevice,(unsigned)i32DspIndex,&(pDevice->memInfo.genRspQueueParams[i32DspIndex].i32FifoId), 1);
        if(err)
        {
            BDBG_ERR(("Unable to find free fifo for GENRIC RESP QUEUE!!!!"));
            BERR_TRACE(err);
            goto err_alloc_CmdQueue;
        }

    /*  Creating structure for firmware which will contain physical addresses of all
        18 fifo registers in the order base,end,read,write,wrap. Wrap will also be
        programmed with fifo end address. Base address of this will be programmed in
        BCHP_AUD_DSP_CFG0_CONTROL_REGISTER0_CXT0 before bootup */

        err = BDSP_Raaga_P_AssignFromRWMem(pDeviceHandle, (BDSP_RAAGA_NUM_FIFOS * BDSP_RAAGA_NUM_PTR_PER_FIFO * sizeof(dramaddr_t)),
                &pDevice->memInfo.DSPFifoAddrStruct[i32DspIndex]);
        if(err != BERR_SUCCESS)
        {
            BDBG_ERR(("BDSP_Raaga_P_Open: Unable to Allocate memory for FIFO Register addresses!"));
            BERR_TRACE(err);
            goto err_alloc_DspFifoAddrStruct;
        }

        for (j = 0; j < BDSP_Raaga_DebugType_eLast; j++)
        {
            if(pDevice->settings.debugSettings[j].enabled == true)
            {
                pDevice->memInfo.FwDebugBuf[i32DspIndex][j].ui32Size = pDevice->settings.debugSettings[j].bufferSize;
            }
            else
            {
                pDevice->memInfo.FwDebugBuf[i32DspIndex][j].ui32Size = 4;
            }

            if(j != BDSP_Raaga_DebugType_eTargetPrintf)
            {
                err = BDSP_Raaga_P_AssignFromRWMem(pDeviceHandle, pDevice->memInfo.FwDebugBuf[i32DspIndex][j].ui32Size,
                        &pDevice->memInfo.FwDebugBuf[i32DspIndex][j].Buffer);   /* 32 bit aligned*/
                if(err != BERR_SUCCESS)
                {
                    BDBG_ERR(("BDSP_RAAGA_Open: Unable to Allocate memory for debug buffer!"));
                    BERR_TRACE(err);
                    goto err_alloc_FwDebugBuf;
                }

                BKNI_Memset(pDevice->memInfo.FwDebugBuf[i32DspIndex][j].Buffer.pAddr, 0,
                        pDevice->memInfo.FwDebugBuf[i32DspIndex][j].ui32Size);
                BDSP_MMA_P_FlushCache(pDevice->memInfo.FwDebugBuf[i32DspIndex][j].Buffer, pDevice->memInfo.FwDebugBuf[i32DspIndex][j].ui32Size);
            }
            else
            {
                /* Incase of TargetPrint allocate buffer for local TargetPrint buffer */
                /* Allocate buffer for local TargetPrint buffer */
                err = BDSP_Raaga_P_AssignFromRWMem(pDeviceHandle, pDevice->memInfo.FwDebugBuf[i32DspIndex][j].ui32Size,
                        &pDevice->memInfo.TargetPrintBuffer[i32DspIndex]);   /* 32 bit aligned*/
                if(err != BERR_SUCCESS)
                {
                    BDBG_ERR(("BDSP_RAAGA_Open: Unable to Allocate memory for local TargetPrint debug buffer!"));
                    BERR_TRACE(err);
                    goto err_alloc_FwDebugBuf;
                }
                BKNI_Memset((void *)pDevice->memInfo.TargetPrintBuffer[i32DspIndex].pAddr,0,pDevice->memInfo.FwDebugBuf[i32DspIndex][j].ui32Size);
                BDSP_MMA_P_FlushCache(pDevice->memInfo.TargetPrintBuffer[i32DspIndex], pDevice->memInfo.FwDebugBuf[i32DspIndex][j].ui32Size);

                /* For 7278, FWDebugBuffer is allocated in firmware. This address is derived based on virtual address of this buffer */
                /* Derive the physical address and offset from the offset of TB Buffer from ATU_VIRTUAL_RW_MEM_START_ADDR */
                ui32VrtlAddrOffset = BDSP_TB_BUF_START_ADDR - ATU_VIRTUAL_RW_MEM_START_ADDR;
                pBuffer = &pDevice->memInfo.sRaagaRWMemoryBuf.Buffer;
                pDstBuffer = &pDevice->memInfo.FwDebugBuf[i32DspIndex][j].Buffer;
                pDevice->memInfo.FwDebugBuf[i32DspIndex][j].ui32Size = BDSP_TB_BUF_MEM_SIZE; /* set this size to Target Buffer size from tb_buff */
                pDstBuffer->hBlock = pBuffer->hBlock;
                pDstBuffer->offset = pBuffer->offset + ui32VrtlAddrOffset;
                pDstBuffer->pAddr = (void *)((uint8_t *)pBuffer->pAddr + ui32VrtlAddrOffset);
            }
        }

        /* Need to allocate raaga system swap memory. Raaga system need a memory in DRAM to swap its data memory contents */
        pDevice->memInfo.sRaagaSwapMemoryBuf[i32DspIndex].ui32Size = BDSP_P_FW_SYSTEM_SWAP_MEMORY_SIZE;

        err = BDSP_Raaga_P_AssignFromRWMem(pDeviceHandle, pDevice->memInfo.sRaagaSwapMemoryBuf[i32DspIndex].ui32Size,
                &pDevice->memInfo.sRaagaSwapMemoryBuf[i32DspIndex].Buffer);   /* 32 bit aligned*/
        if(err != BERR_SUCCESS)
        {
            BDBG_ERR(("BDSP_RAAGA_Open: Unable to Allocate memory for raaga system swapping!"));
            BERR_TRACE(err);
            goto err_alloc_RaagaSwapMemBuf;
        }
    }
    goto end;

/* AJ : Here we do not allocate and only assign the memory. During error handling clean up is taken care */
err_alloc_RaagaSwapMemBuf:
err_alloc_FwDebugBuf:
err_alloc_DspFifoAddrStruct:
err_alloc_genRespQueue:
err_alloc_CmdQueue:
end:
    BDBG_LEAVE(BDSP_Raaga_P_AllocateFWSharedMem);
    return err;
}

/***********************************************************************
Name        :   BDSP_Raaga_P_CalcDebugMemoryReq

Type        :   BDSP Internal

Input       :   pDeviceHandle         - pointer to BDSP_Raaga instance

Return      :   Error Code to return SUCCESS or FAILURE

Functionality   :  Calculates total memory for Firmware debug buffers
***********************************************************************/

BERR_Code BDSP_Raaga_P_CalculateDebugMemoryReq(
    void *pDeviceHandle,
    uint32_t *ui32FWDebugMemReq
    )
{
    BERR_Code err = BERR_SUCCESS;
    unsigned j=0;
    uint32_t ui32reqMem = 0;
    uint32_t ui32DebugMemory = 0;

    BDSP_Raaga *pDevice = (BDSP_Raaga *)pDeviceHandle;
    int32_t i32NumDsp = BDSP_RAAGA_MAX_DSP; /*pDevice->numDsp;*/

    BDBG_ENTER(BDSP_Raaga_P_CalculateDebugMemoryReq);

    BDBG_OBJECT_ASSERT(pDevice, BDSP_Raaga);

    for (j = 0; j < BDSP_Raaga_DebugType_eLast; j++)
    {
        if(pDevice->settings.debugSettings[j].enabled == true)
        {
            ui32reqMem = pDevice->settings.debugSettings[j].bufferSize;
        }
        else
        {
            ui32reqMem = 4;
        }

        /* Incase of TargetPrint allocate buffer for local TargetPrint buffer */
        if(j == BDSP_Raaga_DebugType_eTargetPrintf)
        {
            /* This is for local target buffer */
            ui32reqMem +=  ui32reqMem;
        }

        ui32DebugMemory += ui32reqMem;
    }

    /* Per core meory shall be multiplied with no.of DSPs */
    *ui32FWDebugMemReq = ui32DebugMemory * i32NumDsp;
    BDBG_LEAVE(BDSP_Raaga_P_CalculateDebugMemoryReq);
    return err;
}

/***********************************************************************
Name        :   BDSP_Raaga_P_CalcAndAllocRWMemoryReq

Type        :   BDSP Internal

Input       :   pDeviceHandle         - pointer to BDSP_Raaga instance

Return      :   Error Code to return SUCCESS or FAILURE

Functionality   :  Calculate total memory requirement for each of the
following buffers :
    - Command and Response Message Queues
    - Memory to hold FIFO pointers
    - IS and scratch buffers
    - Debug buffers
    - Context Memory
Allocates the memory for the complete size
***********************************************************************/
BERR_Code BDSP_Raaga_P_CalcAndAllocRWMemoryReq(
    void *pDeviceHandle
    )
{
    BERR_Code err = BERR_SUCCESS;
    int32_t i32DspIndex=0;
    uint32_t ui32TotalMemReq = 0;
    uint32_t ui32reqMem = 0;
	uint32_t ui32TempScratch[BDSP_AF_P_eSchedulingGroup_Max] = {0}, ui32TempIs[BDSP_AF_P_eSchedulingGroup_Max] = {0}, ui32TempIsIf[BDSP_AF_P_eSchedulingGroup_Max] = {0};
	uint32_t ui32NumCh[BDSP_AF_P_eSchedulingGroup_Max] = {0};
    uint32_t ui32ContextMemory = 0;
    uint32_t ui32ProcMemory = 0;
    uint32_t ui32FWDebugMemory = 0;
    int32_t  i32BranchIndex = 0;
    uint32_t ui32AlignExtraSize = 0;
    uint32_t ui32AlignReqSize = 0;
	int32_t  i32SchedulingGroupIndex=0;

    BDSP_Raaga *pDevice = (BDSP_Raaga *)pDeviceHandle;
    int32_t i32NumDsp = BDSP_RAAGA_MAX_DSP; /*pDevice->numDsp; *//* Added to beat coverity job of IRVINE */

    BDBG_ENTER(BDSP_Raaga_P_CalcAndAllocRWMemoryReq);

    BDBG_OBJECT_ASSERT(pDevice, BDSP_Raaga);

    /*Calculate the Memory required for Message Queues, FIFO pointers and Swap Memory */
    err = BDSP_Raaga_P_CalculateInitMemory(&ui32reqMem, i32NumDsp);
    if (err != BERR_SUCCESS)
    {
        return err;
    }
    ui32TotalMemReq += ui32reqMem;

    /* Adding the Firmware debug buffer allocation */
    err = BDSP_Raaga_P_CalculateDebugMemoryReq(pDeviceHandle, &ui32FWDebugMemory);
    if (err != BERR_SUCCESS)
    {
        return err;
    }
    ui32TotalMemReq += ui32FWDebugMemory;

    /*Calculate the maximum Scratch, Interstage and Interstage Generic buffer*/

	for(i32SchedulingGroupIndex=0; i32SchedulingGroupIndex<(int32_t)BDSP_AF_P_eSchedulingGroup_Max; i32SchedulingGroupIndex++)
	{
	    err = BDSP_Raaga_P_CalcScratchAndISbufferReq(&ui32TempScratch[i32SchedulingGroupIndex],
				&ui32TempIs[i32SchedulingGroupIndex],
				&ui32TempIsIf[i32SchedulingGroupIndex],
				&ui32NumCh[i32SchedulingGroupIndex],
				i32SchedulingGroupIndex);
	    if (err != BERR_SUCCESS)
	    {
	        return err;
	    }
	}
    /* Allocation is done per DSP basis. Calculate the total memory requirement
        Scratch buffer - 1
        InterstageIO buffer -1 per task and max task supported is 3
        InterstageIOGen buffer -1 per task and max task supported is 3*/
    for(i32DspIndex=0;i32DspIndex < i32NumDsp;i32DspIndex++)
    {
		for(i32SchedulingGroupIndex=0; i32SchedulingGroupIndex<(int32_t)BDSP_AF_P_eSchedulingGroup_Max; i32SchedulingGroupIndex++)
		{
	        ui32TotalMemReq += ui32TempScratch[i32SchedulingGroupIndex];
	        for(i32BranchIndex=0; i32BranchIndex<BDSP_RAAGA_MAX_BRANCH_PER_TASK; i32BranchIndex++)
	        {
	            ui32TotalMemReq += (ui32TempIs[i32SchedulingGroupIndex]*ui32NumCh[i32SchedulingGroupIndex]);
	            ui32TotalMemReq += ui32TempIsIf[i32SchedulingGroupIndex];
	        }
		}
    }
#if 0
    /* Memory Allocated for Context - VOM table */
    err = BDSP_Raaga_P_CalculateContextMemory(&ui32ContextMemory);
    if (err != BERR_SUCCESS)
    {
        return err;
    }
    ui32TotalMemReq += ui32ContextMemory;
#else
    BSTD_UNUSED(ui32ContextMemory);
#endif
    /* Memory Allocated for DSP Process Memory */
    err = BDSP_Raaga_P_CalculateProcRWMemory(&ui32ProcMemory, i32NumDsp);
    if (err != BERR_SUCCESS)
    {
        return err;
    }
    ui32TotalMemReq += ui32ProcMemory;

    /* Add the Guard band for alignments */
    ui32TotalMemReq += BDSP_RAAGA_GUARD_RW_MEM_SIZE;

    ui32AlignExtraSize = ui32TotalMemReq & (BDSP_MAX_ALIGNED_ADDRESS -1);

    if(ui32AlignExtraSize)
    {
        ui32AlignReqSize = BDSP_MAX_ALIGNED_ADDRESS - ui32AlignExtraSize;
        ui32TotalMemReq += ui32AlignReqSize;
    }

    err = BDSP_MMA_P_AllocateAlignedMemory(pDevice->memHandle, ui32TotalMemReq, &(pDevice->memInfo.sRaagaRWMemoryBuf.Buffer), BDSP_MMA_Alignment_4096bit);
    if(err != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Raaga_P_CalcAndAllocRWMemoryReq: Unable to Allocate memory for RW Memory"));
        err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
        return err;
    }

    /* Ensure any stale dirty data is flushed out of this region before giving it to the DSP. */
    BDSP_MMA_P_FlushCache(pDevice->memInfo.sRaagaRWMemoryBuf.Buffer, ui32TotalMemReq);

    pDevice->memInfo.sRaagaRWMemoryBuf.ui32Size = ui32TotalMemReq;
    pDevice->memInfo.ui32AvailableRWMemSize = 0;

    BDBG_LEAVE(BDSP_Raaga_P_CalcAndAllocRWMemoryReq);
    return err;
}

/***********************************************************************
Name        :   BDSP_Raaga_P_FreeRWMemRegion

Type        :   BDSP Internal

Input       :   pDeviceHandle         - pointer to BDSP_Raaga instance

Return      :   Error Code to return SUCCESS or FAILURE

Functionality   :  Frees up the entire RW memory region allocated in
BDSP_Raaga_P_CalcAndAllocRWMemoryReq
***********************************************************************/
BERR_Code BDSP_Raaga_P_FreeRWMemRegion(
    void *pDeviceHandle
    )
{
    BERR_Code err = BERR_SUCCESS;

    BDSP_Raaga *pDevice = (BDSP_Raaga *)pDeviceHandle;

    BDBG_ENTER(BDSP_Raaga_P_FreeRWMemRegion);

    BDBG_OBJECT_ASSERT(pDevice, BDSP_Raaga);

    BDSP_MMA_P_FreeMemory(&pDevice->memInfo.sRaagaRWMemoryBuf.Buffer);
    pDevice->memInfo.sRaagaRWMemoryBuf.ui32Size = 0;
    pDevice->memInfo.ui32AvailableRWMemSize = 0;
    BKNI_Memset(&pDevice->memInfo.sRaagaRWMemoryBuf.Buffer, 0, sizeof(BDSP_MMA_Memory));

    BDBG_LEAVE(BDSP_Raaga_P_FreeRWMemRegion);
    return err;
}

/***********************************************************************
Name        :   BDSP_Raaga_P_FreeRWMemRegion

Type        :   BDSP Internal

Input       :   pDeviceHandle         - pointer to BDSP_Raaga instance

Return      :   Error Code to return SUCCESS or FAILURE

Functionality   :  Fresets assigned pointers for scratch, IS and debug
buffers
***********************************************************************/
BERR_Code BDSP_Raaga_P_FreeScratchISmem(
    void *pDeviceHandle
    )
{
    BERR_Code err = BERR_SUCCESS;

    uint32_t ui32DspIndex =0;
    int32_t i32TaskIndex = 0, i32SchedulingGroupIndex=0;
    BDSP_Raaga *pDevice = (BDSP_Raaga *)pDeviceHandle;

    BDBG_ENTER(BDSP_Raaga_P_FreeScratchISmem);

    BDBG_OBJECT_ASSERT(pDevice, BDSP_Raaga);

    for(ui32DspIndex = 0; ui32DspIndex < pDevice->numDsp; ui32DspIndex++)
    {
        for(i32SchedulingGroupIndex = 0; i32SchedulingGroupIndex<(int32_t)BDSP_AF_P_eSchedulingGroup_Max; i32SchedulingGroupIndex++)
        {
            pDevice->memInfo.sScratchandISBuff[ui32DspIndex].DspScratchMemGrant[i32SchedulingGroupIndex].ui32Size = 0;
            BKNI_Memset(&pDevice->memInfo.sScratchandISBuff[ui32DspIndex].DspScratchMemGrant[i32SchedulingGroupIndex].Buffer, 0 , sizeof(BDSP_MMA_Memory));

            for(i32TaskIndex = 0; i32TaskIndex<BDSP_RAAGA_MAX_BRANCH_PER_TASK; i32TaskIndex++)
            {
                BKNI_Memset(&pDevice->memInfo.sScratchandISBuff[ui32DspIndex].InterStageIOBuff[i32SchedulingGroupIndex][i32TaskIndex].Buffer, 0 , sizeof(BDSP_MMA_Memory));

                /*Mark all the I/O and I/O Gen buffers as not in use to start with and set them during stage connects. */
                pDevice->memInfo.sScratchandISBuff[ui32DspIndex].InUse[i32SchedulingGroupIndex][i32TaskIndex]=0;
                BKNI_Memset(&pDevice->memInfo.sScratchandISBuff[ui32DspIndex].InterStageIOBuff[i32SchedulingGroupIndex][i32TaskIndex].IOBuff, 0, sizeof(BDSP_AF_P_sIO_BUFFER));

               BKNI_Memset(&pDevice->memInfo.sScratchandISBuff[ui32DspIndex].InterStageIOGenericBuff[i32SchedulingGroupIndex][i32TaskIndex].Buffer, 0 , sizeof(BDSP_MMA_Memory));
               BKNI_Memset(&pDevice->memInfo.sScratchandISBuff[ui32DspIndex].InterStageIOGenericBuff[i32SchedulingGroupIndex][i32TaskIndex].IOGenericBuff, 0, sizeof(BDSP_AF_P_sIO_GENERIC_BUFFER));
            }
        }
    }

    BDBG_LEAVE(BDSP_Raaga_P_FreeScratchISmem);
    return err;
}

/***********************************************************************
Name        :   BDSP_Raaga_P_FreeFWSharedMem

Type        :   BDSP Internal

Input       :   pDeviceHandle         - pointer to BDSP_Raaga instance

Return      :   Error Code to return SUCCESS or FAILURE

Functionality   :  resets assigned pointers for fw shared memory
***********************************************************************/
BERR_Code BDSP_Raaga_P_FreeFWSharedMem(
    void *pDeviceHandle
    )
{
    BERR_Code err = BERR_SUCCESS;

    BDSP_Raaga *pDevice = (BDSP_Raaga *)pDeviceHandle;
    int32_t i32NumDsp = pDevice->numDsp; /* only for Octave-M2 */
    int32_t i32DspIndex = 0 /*pDevice->numDsp*/; /* only for Octave-M2 */
    int32_t j = 0;

    BDBG_ENTER(BDSP_Raaga_P_FreeFWSharedMem);

    BDBG_OBJECT_ASSERT(pDevice, BDSP_Raaga);

    for(i32DspIndex = 0; i32DspIndex < i32NumDsp; i32DspIndex++)
    {
        /* Cleanup CmdQueue */
        pDevice->memInfo.cmdQueueParams[i32DspIndex].uiMsgQueueSize = 0;
        BKNI_Memset(&pDevice->memInfo.cmdQueueParams[i32DspIndex].Queue, 0, sizeof(BDSP_MMA_Memory));

        BDSP_Raaga_P_ReleaseFIFO(pDevice,(unsigned)i32DspIndex,&(pDevice->memInfo.cmdQueueParams[i32DspIndex].i32FifoId), 1);

        /* Cleanup genRspQueue */
        pDevice->memInfo.genRspQueueParams[i32DspIndex].uiMsgQueueSize = 0;
        BKNI_Memset(&pDevice->memInfo.genRspQueueParams[i32DspIndex].Queue, 0, sizeof(BDSP_MMA_Memory));

        BDSP_Raaga_P_ReleaseFIFO(pDevice,(unsigned)i32DspIndex,&(pDevice->memInfo.genRspQueueParams[i32DspIndex].i32FifoId), 1);
    }

    for(i32DspIndex = 0; i32DspIndex < i32NumDsp; i32DspIndex++)
    {
       /* Cleanup the structure holding 18 fifo registers */
        BKNI_Memset(&pDevice->memInfo.DSPFifoAddrStruct[i32DspIndex], 0, sizeof(BDSP_MMA_Memory));

        for (j = 0; j < BDSP_Raaga_DebugType_eLast; j++)
        {
            /* Cleanup Debug buffers */
            pDevice->memInfo.FwDebugBuf[i32DspIndex][j].ui32Size = 0;
            BKNI_Memset(&pDevice->memInfo.FwDebugBuf[i32DspIndex][j].Buffer, 0, sizeof(BDSP_MMA_Memory));

            /* Cleanup TargetPrint buffer */
            if(j == BDSP_Raaga_DebugType_eTargetPrintf)
            {
                BKNI_Memset(&pDevice->memInfo.TargetPrintBuffer[i32DspIndex], 0, sizeof(BDSP_MMA_Memory));
            }
        }

        /* Cleanup RaagaSwapMemoryBuf buffer */
        pDevice->memInfo.sRaagaSwapMemoryBuf[i32DspIndex].ui32Size = 0;
        BKNI_Memset(&pDevice->memInfo.sRaagaSwapMemoryBuf[i32DspIndex].Buffer, 0, sizeof(BDSP_MMA_Memory));
    }

    BDBG_LEAVE(BDSP_Raaga_P_FreeFWSharedMem);
    return err;

}

/***********************************************************************
Name        :   BDSP_Raaga_P_ResetAtuEntries

Type        :   BDSP Internal

Input       :   pDeviceHandle         - pointer to BDSP_Raaga instance

Return      :   Error Code to return SUCCESS or FAILURE

Functionality   :  resets ATU table
***********************************************************************/
BERR_Code BDSP_Raaga_P_ResetAtuEntries(
    void *pDeviceHandle
    )
{
    BERR_Code err = BERR_SUCCESS;
    unsigned j=0;
    bool *pIsAtuEntryUsed = NULL;
    BDSP_Raaga *pDevice = (BDSP_Raaga *)pDeviceHandle;

    BDBG_OBJECT_ASSERT(pDevice, BDSP_Raaga);
    BDBG_ENTER(BDSP_Raaga_P_ResetAtuEntries);

    pIsAtuEntryUsed = pDevice->memInfo.bIsATUEntryUsed;

    for(j = 0; j < BDSP_RAAGA_MAX_ATU_ENTRIES; j++)
    {
        pIsAtuEntryUsed[j] = false;
    }

    BDBG_LEAVE(BDSP_Raaga_P_ResetAtuEntries);
    /* AJ: TODO check if ATU Table should be refset */
    return err;
}

/***********************************************************************
Name        :   BDSP_Raaga_P_GetFreeAtuIndex

Type        :   BDSP Internal

Input       :   pDeviceHandle         - pointer to BDSP_Raaga instance

Return      :   Error Code to return SUCCESS or FAILURE

Functionality   :  returns free ATU index
***********************************************************************/
int32_t BDSP_Raaga_P_GetFreeAtuIndex(
        void *pDeviceHandle
        )
{
    unsigned j=0;
    bool *pIsAtuEntryUsed = NULL;
    BDSP_Raaga *pDevice = (BDSP_Raaga *)pDeviceHandle;
    BDBG_OBJECT_ASSERT(pDevice, BDSP_Raaga);

    BDBG_ENTER(BDSP_Raaga_P_GetFreeAtuIndex);

    pIsAtuEntryUsed = pDevice->memInfo.bIsATUEntryUsed;

    for(j = 0; j < BDSP_RAAGA_MAX_ATU_ENTRIES; j++)
    {
        if(false == pIsAtuEntryUsed[j])
        {
            pIsAtuEntryUsed[j] = true;
            return j;
        }
    }

    BDBG_LEAVE(BDSP_Raaga_P_GetFreeAtuIndex);
    /* Reached Max ATU entries */
    return -1;
}

void BDSP_Raaga_P_WriteAtuEntries(
        BREG_Handle pHandle,
        int32_t i32AtuIndex,
        uint32_t ui32StartAddr,
        uint32_t ui32EndAddr,
        dramaddr_t physicalEntry
        )
{
    uint64_t ui64AtuVrtlEntry = (uint64_t)0;
    uint32_t ui32VrtlEntryOffset = 0;
    uint32_t ui32AtuPhysEntry = 0;
    uint32_t ui32PhysEntryOffset = 0;
    uint64_t ui64Temp = 0;

    BDBG_MSG(("---------------------------------------------------------------------------------------------------------------------------"));
    BDBG_MSG(("StartAddr : %x \t EndAddr  : %x \t PhysAddr : " BDBG_UINT64_FMT, ui32StartAddr, ui32EndAddr, BDBG_UINT64_ARG(physicalEntry)));
    /* Add Virtual start and end addresses to ATU table */
    ui32VrtlEntryOffset = BCHP_RAAGA_DSP_L2C_ADDR_TRANSLATION_TABLEi_ARRAY_BASE + (i32AtuIndex * 8);

    ui64Temp = (uint64_t)((ui32EndAddr >> 12) & 0xfffff);
    ui64AtuVrtlEntry = (uint64_t)((ui64Temp << 44) |
            (ui32StartAddr & 0xfffff000) | 0x001);
    /*ui64AtuVrtlEntry = BDSP_Read64(pHandle, ui32VrtlEntryOffset);

    BCHP_SET_FIELD_DATA(ui64AtuVrtlEntry, RAAGA_DSP_L2C_ADDR_TRANSLATION_TABLEi, VIRTUAL_END_ADDR, (ui32EndAddr >> 12));
    BCHP_SET_FIELD_DATA(ui64AtuVrtlEntry, RAAGA_DSP_L2C_ADDR_TRANSLATION_TABLEi, VIRTUAL_START_ADDR, (ui32StartAddr >> 12));
    BCHP_SET_FIELD_DATA(ui64AtuVrtlEntry, RAAGA_DSP_L2C_ADDR_TRANSLATION_TABLEi, PAGEING_EN, 0);
    BCHP_SET_FIELD_DATA(ui64AtuVrtlEntry, RAAGA_DSP_L2C_ADDR_TRANSLATION_TABLEi, TRANSLATION_VALID, 1);*/

    BDSP_Write64 (pHandle, ui32VrtlEntryOffset, ui64AtuVrtlEntry);

    /* Add Physical start and end addresses to ATU table */
    ui32PhysEntryOffset = BCHP_RAAGA_DSP_L2C_PHYSICAL_40B_BASE_ADDRi_ARRAY_BASE + (i32AtuIndex * 4);

    ui32AtuPhysEntry = BDSP_Read32(pHandle, ui32PhysEntryOffset);
    BCHP_SET_FIELD_DATA(ui32AtuPhysEntry, RAAGA_DSP_L2C_PHYSICAL_40B_BASE_ADDRi, BASE_ADDR, ((uint32_t)(physicalEntry >> 9)));
    BDSP_Write32 (pHandle, ui32PhysEntryOffset, ui32AtuPhysEntry);

    BDBG_MSG(("---------------------------------------------------------------------------------------------------------------------------"));
    BDBG_MSG(("ui32PhysEntryOffset : %x \t ui32PhysEntryOffset : %x \t ui32VrtlEntryOffset : %x \t ui64AtuVrtlEntry : " BDBG_UINT64_FMT,
            ui32PhysEntryOffset, ui32AtuPhysEntry, ui32VrtlEntryOffset, BDBG_UINT64_ARG(ui64AtuVrtlEntry)));
}

/* This is most critical part of M2 for Address mapping
 * Please refer to Design at : https://docs.google.com/a/broadcom.com/document/d/13xkXX_ZMx6MZjeJcyAYmCybxWuUUA6EfsDuSJggjuCg */
BERR_Code BDSP_Raaga_P_AddAtuEntries(
        void *pDeviceHandle
        )
{
    BERR_Code err = BERR_SUCCESS;
    int32_t i32AtuIndex = -1;
    uint32_t ui32StartAddr = 0;
    uint32_t ui32EndAddr = 0;
    dramaddr_t physicalEntry = 0;

    uint32_t ui32RoMemorySize = 0;
    uint32_t ui32RwMemorySize = 0;
    dramaddr_t pRoPhyStartAddr = 0;
    dramaddr_t pRoPhysEndAddr = 0;
    dramaddr_t pRwPhyStartAddr = 0;

    /* TODO AJ : Until API is exposed */
    /* Heap start is assumed to start at 1 GB and ends at (4GB - 128MB) */
    dramaddr_t HeapStart = (dramaddr_t)(0x40000000);
#ifdef FIREPATH_BM
    /* shall reduce as the BM model uses only 3GB */
    dramaddr_t HeapEnd = (dramaddr_t) (0xb8000000);
#else
    dramaddr_t HeapEnd = (dramaddr_t) (0xc0000000);
#endif

    BDSP_Raaga_P_DwnldMemInfo *pDwnldMemInfo;

    BDSP_Raaga *pDevice = (BDSP_Raaga *)pDeviceHandle;
    BDBG_OBJECT_ASSERT(pDevice, BDSP_Raaga);

    BDBG_ENTER(BDSP_Raaga_P_AddAtuEntries);

    pDwnldMemInfo = &pDevice->memInfo.sDwnldMemInfo;

    /* Add ATU entry RO Memory */
    i32AtuIndex = BDSP_Raaga_P_GetFreeAtuIndex(pDevice);
    if(i32AtuIndex < 0)
    {
        BDBG_ERR(("Invalid ATU Index"));
        return err;
    }

    ui32StartAddr = ATU_VIRTUAL_RO_MEM_START_ADDR; /* RO memory start address is always 0 */
    ui32RoMemorySize = pDwnldMemInfo->ui32AllocwithGuardBand;
    ui32EndAddr = (ui32StartAddr + ui32RoMemorySize) - 1;
    physicalEntry = pDwnldMemInfo->ImgBuf.offset;

    BDSP_Raaga_P_WriteAtuEntries(pDevice->regHandle, i32AtuIndex, ui32StartAddr, ui32EndAddr, physicalEntry);

    /* Make ATU Entry for RW memory  */
    i32AtuIndex = BDSP_Raaga_P_GetFreeAtuIndex(pDevice);
    if(i32AtuIndex < 0)
    {
        BDBG_ERR(("Invalid ATU Index"));
        return err;
    }

    ui32StartAddr = ATU_VIRTUAL_RW_MEM_START_ADDR ; /* RW memory start address is always at 256MB boundary */
    ui32RwMemorySize = pDevice->memInfo.sRaagaRWMemoryBuf.ui32Size;
    ui32EndAddr = ui32StartAddr + ui32RwMemorySize -1;
    physicalEntry = pDevice->memInfo.sRaagaRWMemoryBuf.Buffer.offset;

    BDSP_Raaga_P_WriteAtuEntries(pDevice->regHandle, i32AtuIndex, ui32StartAddr, ui32EndAddr, physicalEntry);

    /* Add ATU Entry for the rest of the physical memory below RW memory */
    i32AtuIndex = BDSP_Raaga_P_GetFreeAtuIndex(pDevice);
    if(i32AtuIndex < 0)
    {
        BDBG_ERR(("Invalid ATU Index"));
        return err;
    }

    physicalEntry = (pDevice->memInfo.sRaagaRWMemoryBuf.Buffer.offset + ui32RwMemorySize);
    ui32StartAddr = ui32EndAddr + 1; /* Begins after previous virtual memory entry */
    ui32EndAddr = ui32StartAddr + (uint32_t)(HeapEnd - physicalEntry) - 1;

    BDSP_Raaga_P_WriteAtuEntries(pDevice->regHandle, i32AtuIndex, ui32StartAddr, ui32EndAddr, physicalEntry);

    /* Add ATU Entry for the physical memory above the ro memory allocation */
    i32AtuIndex = BDSP_Raaga_P_GetFreeAtuIndex(pDevice);
    if(i32AtuIndex < 0)
    {
        BDBG_ERR(("Invalid ATU Index"));
        return err;
    }

    pRoPhyStartAddr = pDwnldMemInfo->ImgBuf.offset; /* This is used to identify size of region above RO region */
    ui32StartAddr = ui32EndAddr + 1;
    ui32EndAddr = ui32StartAddr + (uint32_t)(pRoPhyStartAddr - HeapStart) - 1;
    physicalEntry = HeapStart;

    BDSP_Raaga_P_WriteAtuEntries(pDevice->regHandle, i32AtuIndex, ui32StartAddr, ui32EndAddr, physicalEntry);

    /* Add ATU Entry for the physical memory between RO memory and RW memory */
    i32AtuIndex = BDSP_Raaga_P_GetFreeAtuIndex(pDevice);
    if(i32AtuIndex < 0)
    {
        BDBG_ERR(("Invalid ATU Index"));
        return err;
    }

    pRwPhyStartAddr = pDevice->memInfo.sRaagaRWMemoryBuf.Buffer.offset;
    pRoPhysEndAddr = (pDwnldMemInfo->ImgBuf.offset + pDwnldMemInfo->ui32AllocwithGuardBand);

    /*if both are equal, means it is contiguous, no need to map */
    if(pRwPhyStartAddr > pRoPhysEndAddr)
    {
        ui32StartAddr = ui32EndAddr + 1; /* Begins after previous virtual memory entry */
        ui32EndAddr = ui32StartAddr + (uint32_t)(pRwPhyStartAddr - pRoPhysEndAddr) - 1;

        physicalEntry = pRoPhysEndAddr;

        BDSP_Raaga_P_WriteAtuEntries(pDevice->regHandle, i32AtuIndex, ui32StartAddr, ui32EndAddr, physicalEntry);
    }

    return err;
}

BERR_Code BDSP_Raaga_P_GetATUEntries(
        void *pDeviceHandle,
        int32_t i32AtuIndex,
        uint64_t *pui64PhysicalAddr,
        uint32_t *pui32VrtlStartAddr,
        uint32_t *pui32VrtlEndAddr
)
{
    BERR_Code err = BERR_SUCCESS;
    uint64_t ui64AtuVrtlEntry = 0;
    uint32_t ui32PhysEntry = 0;
    uint32_t ui32TempAddr = 0;
    BDSP_Raaga *pDevice = (BDSP_Raaga *)pDeviceHandle;
    BDBG_OBJECT_ASSERT(pDevice, BDSP_Raaga);

    BDBG_ENTER(BDSP_Raaga_P_GetATUEntries);
    if(i32AtuIndex > BDSP_RAAGA_MAX_ATU_ENTRIES)
    {
        BDBG_ERR(("Invalid ATU index"));
        return err;
    }

    ui64AtuVrtlEntry = BDSP_Read64 (pDevice->regHandle, (BCHP_RAAGA_DSP_L2C_ADDR_TRANSLATION_TABLEi_ARRAY_BASE + (i32AtuIndex * 8)));
    ui32TempAddr = BCHP_GET_FIELD_DATA(ui64AtuVrtlEntry, RAAGA_DSP_L2C_ADDR_TRANSLATION_TABLEi, VIRTUAL_END_ADDR);
    *pui32VrtlEndAddr = (ui32TempAddr << 12);
    ui32TempAddr = BCHP_GET_FIELD_DATA(ui64AtuVrtlEntry, RAAGA_DSP_L2C_ADDR_TRANSLATION_TABLEi, VIRTUAL_START_ADDR);
    *pui32VrtlStartAddr = ui32TempAddr;

    ui32PhysEntry = BDSP_Read32 (pDevice->regHandle, (BCHP_RAAGA_DSP_L2C_PHYSICAL_40B_BASE_ADDRi_ARRAY_BASE + (i32AtuIndex * 4)));
    ui32TempAddr = BCHP_GET_FIELD_DATA(ui32PhysEntry, RAAGA_DSP_L2C_PHYSICAL_40B_BASE_ADDRi, BASE_ADDR);
    *pui64PhysicalAddr = (ui32TempAddr << 9);

    BDBG_ENTER(BDSP_Raaga_P_GetATUEntries);
    return BERR_SUCCESS;
}

BERR_Code BDSP_Raaga_P_GetVirtualAddress(
        void *pDeviceHandle,
        uint64_t ui64PhysicalAddr,
        uint32_t *pui32VirtualAddr
        )
{
    BERR_Code err = BERR_SUCCESS;
    unsigned j=0;
    bool *pIsAtuEntryUsed = NULL;
    uint64_t ui64PhysAddr = 0;
    uint32_t ui32VrtlStartAddr = 0, ui32VrtlEndAddr = 0;
    uint32_t ui32BlockSize = 0;

    BDSP_Raaga *pDevice = (BDSP_Raaga *)pDeviceHandle;
    BDBG_OBJECT_ASSERT(pDevice, BDSP_Raaga);

    BDBG_ENTER(BDSP_Raaga_P_GetVirtualAddress);

    pIsAtuEntryUsed = pDevice->memInfo.bIsATUEntryUsed;

    for(j = 0; j < BDSP_RAAGA_MAX_ATU_ENTRIES; j++)
    {
        if(false == pIsAtuEntryUsed[j])
        {
             BDBG_ERR(("ATU entries are not populated, Invalid"));
             return err;
        }

        BDSP_Raaga_P_GetATUEntries(pDevice, j, &ui64PhysAddr, &ui32VrtlStartAddr, &ui32VrtlEndAddr);
        ui32BlockSize =  (ui32VrtlEndAddr - ui32VrtlStartAddr);

        /* Check if Entry is available in this block */
        if(ui64PhysicalAddr > (ui64PhysAddr + ui32BlockSize))
        {
            continue;
        }
        else
        {
            *pui32VirtualAddr = ui32VrtlStartAddr + (uint32_t)(ui64PhysicalAddr - ui64PhysAddr);
        }
    }

    BDBG_LEAVE(BDSP_Raaga_P_GetVirtualAddress);
    return err;
}

BERR_Code BDSP_Raaga_P_GetPhysicalAddress(
        void *pDeviceHandle,
        uint32_t ui32VirtualAddr,
        uint64_t *pui64PhysicalAddr
        )
{
    BERR_Code err = BERR_SUCCESS;
    unsigned j=0;
    bool *pIsAtuEntryUsed = NULL;
    uint64_t ui64PhysAddr = 0;
    uint32_t ui32VrtlStartAddr = 0, ui32VrtlEndAddr = 0;
    BDSP_Raaga *pDevice = (BDSP_Raaga *)pDeviceHandle;
    BDBG_OBJECT_ASSERT(pDevice, BDSP_Raaga);

    BDBG_ENTER(BDSP_Raaga_P_GetPhysicalAddress);

    pIsAtuEntryUsed = pDevice->memInfo.bIsATUEntryUsed;

    for(j = 0; j < BDSP_RAAGA_MAX_ATU_ENTRIES; j++)
    {
        if(false == pIsAtuEntryUsed[j])
        {
             BDBG_ERR(("ATU entries are not populated, Invalid"));
             return err;
        }

        BDSP_Raaga_P_GetATUEntries(pDevice, j, &ui64PhysAddr, &ui32VrtlStartAddr, &ui32VrtlEndAddr);

        if(ui32VirtualAddr > ui32VrtlEndAddr)
        {
            continue;
        }
        else
        {
            *pui64PhysicalAddr = ui64PhysAddr + (ui32VirtualAddr - ui32VrtlStartAddr);
        }
    }

    BDBG_LEAVE(BDSP_Raaga_P_GetPhysicalAddress);
    return err;
}
#endif
