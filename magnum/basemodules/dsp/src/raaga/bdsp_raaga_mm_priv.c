/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
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
 *****************************************************************************/

#include "bdsp_raaga.h"
#include "bdsp_raaga_mm_priv.h"
#include "bdsp_raaga_priv.h"
#include "bdsp_raaga_fwdownload_priv.h"
#include "bdsp_raaga_cmdresp_priv.h"
#include "bdsp_raaga_fw.h"
#include "bdsp_raaga_fw_settings.h"

BDBG_MODULE(bdsp_raaga_mm);

/***************************************************************************************
Summary:
#Use BDSP_MEM_P_ConvertOffsetToCacheAddr
#to convert from cache address to offset
#In BDSP: Write and Read requires cached address
#In Raaga: Write or Read into DRAM requries virtual offset.

*****************************************************************************************/

BERR_Code BDSP_MEM_P_ConvertOffsetToCacheAddress(BMEM_Handle    hHeap,
                                                                 uint32_t       ui32Offset,
                                                                 void **ppCachedAddr
                                                                 )
{
       void * pUnCached;
        BERR_Code err;


        err = BMEM_ConvertOffsetToAddress(hHeap,ui32Offset,(void *)&pUnCached );
        if(err){
               BDBG_ERR((" Error in input value ui32Offset to the function BDSP_MEM_P_ConvertOffsetToCacheAddr"));
               return BERR_TRACE(err);
        }
       err = BMEM_ConvertAddressToCached(hHeap, (void *)pUnCached , ppCachedAddr );
       if(err != BERR_SUCCESS){
               *ppCachedAddr = NULL;
               BDBG_ERR((" %s: Issue with ptr=%p being tried to convert to cache", __FUNCTION__, pUnCached));
               return BERR_TRACE(err);
       }
       return BERR_SUCCESS;
}
/*****************************************************************************
Function Name: BDSP_MM_P_GetFwMemRequired

            If the algorithm is supported, size for that algo is calculated
                and the required cumulative memory requirement is found out.

*****************************************************************************/


BERR_Code BDSP_MM_P_GetFwMemRequired(
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

    pDwnldMemInfo->ui32AllocwithGuardBand =  pDwnldMemInfo->ui32AllocatedBinSize;
    if(pSettings->preloadImages == true)
        pDwnldMemInfo->ui32AllocwithGuardBand += BDSP_CODE_DWNLD_GUARD_BAND_SIZE;

    return ret;
}


BERR_Code BDSP_MM_P_GetFwMemRequirement(BDSP_Raaga *pDevice)
{
    BDSP_Raaga_P_DwnldMemInfo *pDwnldMemInfo;

    BDBG_OBJECT_ASSERT(pDevice, BDSP_Raaga);

    pDwnldMemInfo = &pDevice->memInfo.sDwnldMemInfo;

    BDSP_MM_P_GetFwMemRequired(&(pDevice->settings),pDwnldMemInfo,(void *)&(pDevice->imgCache[0]),true,NULL);

    pDwnldMemInfo->pImgBuf = BDSP_MEM_P_AllocateAlignedMemory(pDevice->memHandle, pDwnldMemInfo->ui32AllocwithGuardBand,2,0); /* 32bit aligned */
    if(pDwnldMemInfo->pImgBuf == NULL ){
        return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
    }
    BDBG_MSG(("**************Allocated size %d, Bin Size = %d, ptr = %p Preloaded = %d", pDwnldMemInfo->ui32AllocwithGuardBand,pDwnldMemInfo->ui32AllocatedBinSize,pDwnldMemInfo->pImgBuf,pDwnldMemInfo->IsImagePreLoaded));
    pDevice->fwHeapSize = pDwnldMemInfo->ui32AllocwithGuardBand;
    pDevice->pFwHeapMemory = pDwnldMemInfo->pImgBuf;
    return BERR_SUCCESS;
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
    if(pDevice->memInfo.sDwnldMemInfo.pImgBuf == NULL )
        return;

#ifdef FWDWNLD_DBG
    BDSP_Raaga_P_FwDwnldBuf_Dump(pDeviceHandle);
#endif

    BDSP_MEM_P_FreeMemory(pDevice->memHandle,pDevice->memInfo.sDwnldMemInfo.pImgBuf);
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

    for(i=0;i<pDevice->numDsp;i++)
    {
        BDSP_MEM_P_FreeMemory(pDevice->memHandle,pDevice->memInfo.cmdQueueParams[i].pBaseAddr);
        BDSP_MEM_P_FreeMemory(pDevice->memHandle,pDevice->memInfo.genRspQueueParams[i].pBaseAddr);
        BDSP_MEM_P_FreeMemory(pDevice->memHandle,pDevice->memInfo.pDSPFifoAddrStruct[i]);

        for (j = 0; j < BDSP_Raaga_DebugType_eLast; j++)
        {
            BDSP_MEM_P_FreeMemory(pDevice->memHandle,pDevice->memInfo.pFwDebugBuf[i][j].pAddr);
            if(j == BDSP_Raaga_DebugType_eTargetPrintf)
            {
                BDSP_MEM_P_FreeMemory(pDevice->memHandle,pDevice->memInfo.pTargetPrintBuffer[i]);
            }
        }
        BDSP_MEM_P_FreeMemory(pDevice->memHandle,pDevice->memInfo.sRaagaSwapMemoryBuf[i].pAddr);
    }

    return err;
}



BERR_Code BDSP_Raaga_P_AssignMem_DwnldBuf(void * pDeviceHandle, void *ptr)
{
    BDSP_Raaga *pDevice = (BDSP_Raaga *)pDeviceHandle;
    uint32_t i=0, j=0;
    BDSP_Raaga_P_AlgoTypeImgBuf *pAlgoBufInfo;
    BERR_Code retVal = BERR_SUCCESS;
    BDSP_Raaga_P_DwnldBufUsageInfo *pDwnldBufInfo;

    BDBG_ENTER(BDSP_Raaga_P_AssignMem_DwnldBuf);
    BDBG_OBJECT_ASSERT(pDevice, BDSP_Raaga);

    if(ptr == NULL ){
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
                pDwnldBufInfo->pAddr = NULL;
                continue;
            }

            pDwnldBufInfo->pAddr = ptr;
            ptr = (void *)((uint8_t *)ptr + pAlgoBufInfo[i].ui32Size);

            if(pDwnldBufInfo->pAddr == NULL)
            {
                BDBG_ERR((" NULL pointer allocated for algotype %u of size %u buffer num %u ", i ,pAlgoBufInfo[i].ui32Size,j));
                retVal = BERR_INVALID_PARAMETER;
                goto error;
            }
            pDwnldBufInfo->numUser = 0;

            BDBG_MSG((" Assigned Addr %p  size %u AlgoType %d", pDwnldBufInfo->pAddr ,pAlgoBufInfo[i].ui32Size,i ));
        }
    }

    if(ptr > (void *)((uint8_t *)pDevice->memInfo.sDwnldMemInfo.pImgBuf + pDevice->memInfo.sDwnldMemInfo.ui32AllocatedBinSize))
    {
        BDBG_ERR(("Used memory more than allocated memory.MemInfo size parameter might be corrupted.\
                    Used till %p Allocated till %p -- exclusive", ptr, \
                    (void *)((uint8_t *)pDevice->memInfo.sDwnldMemInfo.pImgBuf + pDevice->memInfo.sDwnldMemInfo.ui32AllocatedBinSize)));
        retVal = BERR_INVALID_PARAMETER;
        goto error;
    }

error:
    BDBG_LEAVE(BDSP_Raaga_P_AssignMem_DwnldBuf);
    return retVal;

}


void BDSP_Raaga_P_InitMem( void *pDeviceHandle)
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
        *pMemoryReq += (BDSP_RAAGA_NUM_FIFOS * BDSP_RAAGA_NUM_PTR_PER_FIFO * 4);
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
    err = BDSP_MM_P_GetFwMemRequirement(pDeviceHandle);
    if(BERR_SUCCESS != err)
    {
        BDBG_ERR(("BDSP_MEM_P_CalcMemPoolReq: Error getting firmware memory requirements!"));
        err = BERR_TRACE(err);
        goto err_calcMemPoolReq;
    }


    /* In case the authentication is enabled, all the binaries are downloaded so there is no need to optimize on code download memory.
    In other cases we limit of download of algorithms based on the number of worst case instances of decoder/framesync/encoder/PP etc
    allowed for download and we rotate the pointers internally. Allocation here is for the later case where we manage fewer downloads internally
    in case of non-secure mode*/

    /* Allocate Task Cmd/Resp Queues */
    for(i32DspIndex=0;i32DspIndex<i32MaxDsp;i32DspIndex++)
    {
        void  *pBaseAddr;

        /* Allocate Command queue */
        pBaseAddr = BDSP_MEM_P_AllocateAlignedMemory(pDevice->memHandle, (BDSP_RAAGA_MAX_MSGS_PER_QUEUE * sizeof(BDSP_Raaga_P_Command)*BDSP_RAAGA_MAX_FW_TASK_PER_DSP),2, 0);
        if(NULL == pBaseAddr)
        {
            BDBG_ERR(("BDSP_DSP_AllocMem: Unable to Allocate memory for cmd queue!"));
            err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
            goto err_alloc_CmdQueue;
        }
        pDevice->memInfo.cmdQueueParams[i32DspIndex].pBaseAddr = pBaseAddr;
        pDevice->memInfo.cmdQueueParams[i32DspIndex].uiMsgQueueSize = BDSP_RAAGA_MAX_MSGS_PER_QUEUE * sizeof(BDSP_Raaga_P_Command)*BDSP_RAAGA_MAX_FW_TASK_PER_DSP;

        err = BDSP_Raaga_P_AssignFreeFIFO(pDevice,(unsigned)i32DspIndex,&(pDevice->memInfo.cmdQueueParams[i32DspIndex].i32FifoId), 1);
        if(err)
        {
            BDBG_ERR(("Unable to find free fifo for CMD QUEUE!!!!"));
            goto err_alloc_CmdQueue;
        }

        /* Allocate Generic Response queue */
        pBaseAddr = BDSP_MEM_P_AllocateAlignedMemory(pDevice->memHandle, (BDSP_RAAGA_MAX_MSGS_PER_QUEUE * sizeof(BDSP_Raaga_P_Response)),2, 0);
        if(NULL == pBaseAddr)
        {
            BDBG_ERR(("BDSP_DSP_AllocMem: Unable to Allocate memory for generic response queue!"));
            err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
            goto err_alloc_genRespQueue;
        }
        pDevice->memInfo.genRspQueueParams[i32DspIndex].pBaseAddr = pBaseAddr;
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

        pDevice->memInfo.pDSPFifoAddrStruct[i32DspIndex] = BDSP_MEM_P_AllocateAlignedMemory(pDevice->memHandle,
                    BDSP_RAAGA_NUM_FIFOS * BDSP_RAAGA_NUM_PTR_PER_FIFO * 4,2, 0);   /* 32 bit aligned*/
        if( NULL == pDevice->memInfo.pDSPFifoAddrStruct[i32DspIndex])
        {
            BDBG_ERR(("BDSP_Raaga_P_Open: Unable to Allocate memory for FIFO Register addresses!"));
            err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
            goto err_alloc_DspFifoAddrStruct;
        }

        for (j = 0; j < BDSP_Raaga_DebugType_eLast; j++)
        {
            if(pDevice->settings.debugSettings[j].enabled == true)
            {
                pDevice->memInfo.pFwDebugBuf[i32DspIndex][j].ui32Size = pDevice->settings.debugSettings[j].bufferSize;
            }
            else
            {
                pDevice->memInfo.pFwDebugBuf[i32DspIndex][j].ui32Size = 4;
            }
            pDevice->memInfo.pFwDebugBuf[i32DspIndex][j].pAddr = BDSP_MEM_P_AllocateAlignedMemory(pDevice->memHandle, pDevice->memInfo.pFwDebugBuf[i32DspIndex][j].ui32Size,2, 0);   /* 32 bit aligned*/
            if( NULL == pDevice->memInfo.pFwDebugBuf[i32DspIndex][j].pAddr)
            {
                BDBG_ERR(("BDSP_RAAGA_Open: Unable to Allocate memory for debug buffer!"));
                err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
                goto err_alloc_FwDebugBuf;
            }
            BKNI_Memset(pDevice->memInfo.pFwDebugBuf[i32DspIndex][j].pAddr,0,pDevice->memInfo.pFwDebugBuf[i32DspIndex][j].ui32Size);
            BDSP_MEM_P_FlushCache(pDevice->memHandle, pDevice->memInfo.pFwDebugBuf[i32DspIndex][j].pAddr, pDevice->memInfo.pFwDebugBuf[i32DspIndex][j].ui32Size);
            /* Incase of TargetPrint allocate buffer for local TargetPrint buffer */
            if(j == BDSP_Raaga_DebugType_eTargetPrintf)
            {
                /* Allocate buffer for local TargetPrint buffer */
                pDevice->memInfo.pTargetPrintBuffer[i32DspIndex] = (uint8_t *)BDSP_MEM_P_AllocateAlignedMemory(pDevice->memHandle, pDevice->memInfo.pFwDebugBuf[i32DspIndex][j].ui32Size,2, 0);   /* 32 bit aligned*/
                if( NULL == (void *)pDevice->memInfo.pTargetPrintBuffer[i32DspIndex])
                {
                    BDBG_ERR(("BDSP_RAAGA_Open: Unable to Allocate memory for local TargetPrint debug buffer!"));
                    err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
                    goto err_alloc_FwDebugBuf;
                }
                BKNI_Memset((void *)pDevice->memInfo.pTargetPrintBuffer[i32DspIndex],0,pDevice->memInfo.pFwDebugBuf[i32DspIndex][j].ui32Size);
                BDSP_MEM_P_FlushCache(pDevice->memHandle, (void *)pDevice->memInfo.pTargetPrintBuffer[i32DspIndex], pDevice->memInfo.pFwDebugBuf[i32DspIndex][j].ui32Size);
            }
        }

        /* Need to allocate raaga system swap memory. Raaga system need a memory in DRAM to swap its data memory contents */
        pDevice->memInfo.sRaagaSwapMemoryBuf[i32DspIndex].ui32Size = BDSP_P_FW_SYSTEM_SWAP_MEMORY_SIZE;
        pDevice->memInfo.sRaagaSwapMemoryBuf[i32DspIndex].pAddr =
                    BDSP_MEM_P_AllocateAlignedMemory(pDevice->memHandle, pDevice->memInfo.sRaagaSwapMemoryBuf[i32DspIndex].ui32Size, 2, 0);   /* 32 bit aligned*/

        if( NULL == pDevice->memInfo.sRaagaSwapMemoryBuf[i32DspIndex].pAddr)
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
        if(pDevice->memInfo.pFwDebugBuf[i32DspIndex][j].pAddr)
        {
            BDSP_MEM_P_FreeMemory(pDevice->memHandle,pDevice->memInfo.pFwDebugBuf[i32DspIndex][j].pAddr);
        }
        if(j == BDSP_Raaga_DebugType_eTargetPrintf)
        {
            if(pDevice->memInfo.pTargetPrintBuffer[i32DspIndex])
            {
                BDSP_MEM_P_FreeMemory(pDevice->memHandle,pDevice->memInfo.pTargetPrintBuffer[i32DspIndex]);
            }
        }

    }
err_alloc_FwDebugBuf:
    if(pDevice->memInfo.pDSPFifoAddrStruct[i32DspIndex])
    {
        BDSP_MEM_P_FreeMemory(pDevice->memHandle,pDevice->memInfo.pDSPFifoAddrStruct[i32DspIndex]);
    }
err_alloc_DspFifoAddrStruct:
    if(pDevice->memInfo.genRspQueueParams[i32DspIndex].pBaseAddr)
    {
        BDSP_MEM_P_FreeMemory(pDevice->memHandle,pDevice->memInfo.genRspQueueParams[i32DspIndex].pBaseAddr);
    }
err_alloc_genRespQueue:
    if(pDevice->memInfo.cmdQueueParams[i32DspIndex].pBaseAddr)
    {
        BDSP_MEM_P_FreeMemory(pDevice->memHandle,pDevice->memInfo.cmdQueueParams[i32DspIndex].pBaseAddr);
    }
err_alloc_CmdQueue:

    while(i32DspIndex!=0)
    {
        i32DspIndex--;
        if(pDevice->memInfo.sRaagaSwapMemoryBuf[i32DspIndex].pAddr)
        {
            BDSP_MEM_P_FreeMemory(pDevice->memHandle,pDevice->memInfo.sRaagaSwapMemoryBuf[i32DspIndex].pAddr);
        }
        for (j = 0; j < BDSP_Raaga_DebugType_eLast; j++)
        {
            if(pDevice->memInfo.pFwDebugBuf[i32DspIndex][j].pAddr)
            {
                BDSP_MEM_P_FreeMemory(pDevice->memHandle,pDevice->memInfo.pFwDebugBuf[i32DspIndex][j].pAddr);
            }
        }
        if(pDevice->memInfo.pDSPFifoAddrStruct[i32DspIndex])
        {
            BDSP_MEM_P_FreeMemory(pDevice->memHandle,pDevice->memInfo.pDSPFifoAddrStruct[i32DspIndex]);
        }
        if(pDevice->memInfo.genRspQueueParams[i32DspIndex].pBaseAddr)
        {
            BDSP_MEM_P_FreeMemory(pDevice->memHandle,pDevice->memInfo.genRspQueueParams[i32DspIndex].pBaseAddr);
        }
        if(pDevice->memInfo.cmdQueueParams[i32DspIndex].pBaseAddr)
        {
            BDSP_MEM_P_FreeMemory(pDevice->memHandle,pDevice->memInfo.cmdQueueParams[i32DspIndex].pBaseAddr);
        }
    }

err_calcMemPoolReq:
    BDSP_Raaga_P_FreeFwExec(pDeviceHandle);
end:
    BDBG_LEAVE(BDSP_Raaga_P_AllocateInitMemory);
    return err;
}

BERR_Code BDSP_MM_P_CalcStageMemPoolReq(void *pStageHandle)
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
    BDBG_ENTER(BDSP_MM_P_CalcStageMemPoolReq);

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

    BDBG_LEAVE(BDSP_MM_P_CalcStageMemPoolReq);
    return err;
}

/***********************************************************************
Name        :   BDSP_MM_P_CalcScratchAndISbufferReq_MemToolAPI

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

BERR_Code BDSP_MM_P_CalcScratchAndISbufferReq_MemToolAPI(
        uint32_t *pui32ScratchMem,
        uint32_t *pui32InterstageIOMem,
        uint32_t *pui32InterstageIOGenMem,
        uint32_t *pui32Numch,
        const BDSP_RaagaUsageOptions *pUsage
)
{
    BERR_Code err = BERR_SUCCESS;
    unsigned int Algoindex = 0, Nodeindex = 0;
    uint32_t ui32TempScratch = 0, ui32TempIs = 0, ui32TempIsIf = 0;
    uint32_t ui32Scratch = 0, ui32Is = 0, ui32IsIf = 0;
    uint32_t ui32NumCh=0;

    BDBG_ENTER(BDSP_MM_P_CalcScratchAndISbufferReq);

    /* For Decoders */
    for ( Algoindex=0; Algoindex < BDSP_Algorithm_eMax; Algoindex++ )
    {
        const BDSP_Raaga_P_AlgorithmSupportInfo *pInfo;
        if(pUsage->Codeclist[Algoindex]!= true)
        {
            continue;
        }
        pInfo = BDSP_Raaga_P_LookupAlgorithmSupported(Algoindex,pUsage->DolbyCodecVersion);

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

    BDBG_LEAVE(BDSP_MM_P_CalcScratchAndISbufferReq);
    return err;
}

/***********************************************************************
Name        :   BDSP_MM_P_CalcScratchAndISbufferReq

Type        :   BDSP Internal

Input       :   pui32ScratchMem         -Pointer provided the higher BDSP function to return the max Scratch memory Required.
                pui32InterstageIOMem        -Pointer provided the higher BDSP function to return the max Interstage IO memory Required.
                pui32InterstageIOGenMem     -Pointer provided the higher BDSP function to return the max Interstage IO Generic memory Required.
                pui32Numch              -Pointer provided the higher BDSP function to return the Max number of channels supported.

Return      :   Error Code to return SUCCESS or FAILURE

Functionality   :   Following are the operations performed.
        1)  Return the highest Scratch, Interstage IO, Interstage IO Generic and Max channels required/supported by the system.
***********************************************************************/

BERR_Code BDSP_MM_P_CalcScratchAndISbufferReq(
        uint32_t *pui32ScratchMem,
        uint32_t *pui32InterstageIOMem,
        uint32_t *pui32InterstageIOGenMem,
        uint32_t *pui32Numch
)
{
    BERR_Code err = BERR_SUCCESS;
    unsigned int Algoindex = 0, Nodeindex = 0;
    uint32_t ui32TempScratch = 0, ui32TempIs = 0, ui32TempIsIf = 0;
    uint32_t ui32Scratch = 0, ui32Is = 0, ui32IsIf = 0;
    uint32_t ui32NumCh=0;

    BDBG_ENTER(BDSP_MM_P_CalcScratchAndISbufferReq);

    /* For Decoders */
    for ( Algoindex=0; Algoindex < BDSP_Algorithm_eMax; Algoindex++ )
    {
        const BDSP_Raaga_P_AlgorithmInfo *pInfo;

        pInfo = BDSP_Raaga_P_LookupAlgorithmInfo(Algoindex);
        if ( !pInfo->supported )
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

    BDBG_LEAVE(BDSP_MM_P_CalcScratchAndISbufferReq);

    return err;
}

BERR_Code BDSP_MM_P_CalcandAllocScratchISbufferReq(void *pDeviceHandle)
{

    BERR_Code err = BERR_SUCCESS;
    uint32_t ui32TempScratch = 0, ui32TempIs = 0, ui32TempIsIf = 0;
    uint32_t ui32NumCh=0;
    void  *pBaseAddr = 0;
    dramaddr_t  raagaOffsetAddr = 0;
    unsigned int k=0;
    int32_t i32DspIndex =0, i32TaskIndex = 0;
    void *pAddress = NULL;

    BDSP_Raaga *pDevice = (BDSP_Raaga *)pDeviceHandle;
    int32_t i32MaxDsp = pDevice->numDsp; /* Added to beat coverity job of IRVINE */

    /*BDBG_OBJECT_ASSERT(pDeviceHandle, BDSP_Raaga);*/

    BDBG_ENTER(BDSP_MM_P_CalcandAllocScratchISbufferReq);

    BDSP_MM_P_CalcScratchAndISbufferReq(&ui32TempScratch, &ui32TempIs, &ui32TempIsIf, &ui32NumCh);

    BDBG_MSG(("ui32TempScratch = %d , ui32TempIs = %d ui32TempIsIf =%d",ui32TempScratch,ui32TempIs,ui32TempIsIf));

    /*ui32TempIs is the worst case IS buffer per channel for a decoder/passthrough
            ui32PaIs is the worst case IS buffer per channel for a PP
            ui32EncIs is the worst case IS buffer per channel for a Encoder
            ui32NumCh is the worst case number of channels

    ui32TempIsIf is the worst case IS Generic buffer for a decoder/passthrough
            ui32PaIsIf is the worst case IS Generic buffer for a PP
            ui32EncIsIf is the worst case IS Generic buffer for a Encoder
    */

    /* this is done in Raaga_P_Open time itself for each DSP and not for every Context*/
    for(i32DspIndex=0;i32DspIndex<i32MaxDsp;i32DspIndex++)
    {

        /* Allocate Scratch Memory */
        if (ui32TempScratch)
        {
            pBaseAddr = BDSP_MEM_P_AllocateAlignedMemory(pDevice->memHandle, ui32TempScratch,2, 0); /* 32 bit aligned*/

            if(NULL == pBaseAddr)
            {
                BDBG_ERR(("BDSP_MM_P_CalcandAllocScratchISbufferReq: Unable to Allocate memory for Scratch Buffer!"));
                err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
                goto error_alloc_scratch_mem;
            }

            BDSP_MEM_P_ConvertAddressToOffset(  pDevice->memHandle,
                    pBaseAddr,
                    &raagaOffsetAddr
                    );
            /* pBaseAddr = raagaOffsetAddr;*/
        }

        pDevice->memInfo.sScratchandISBuff[i32DspIndex].ui32DspScratchMemGrant.ui32DramBufferAddress=raagaOffsetAddr;
        pDevice->memInfo.sScratchandISBuff[i32DspIndex].ui32DspScratchMemGrant.ui32BufferSizeInBytes=ui32TempScratch;

        for(i32TaskIndex=0; i32TaskIndex<BDSP_RAAGA_MAX_BRANCH_PER_TASK; i32TaskIndex++)
        {

            /* Allocate IO Memory */

            pBaseAddr = BDSP_MEM_P_AllocateAlignedMemory(pDevice->memHandle, ui32TempIs*ui32NumCh ,2, 0);   /* 32 bit aligned*/

            if(NULL == pBaseAddr)
            {
                BDBG_ERR(("BDSP_MM_P_CalcandAllocScratchISbufferReq: Unable to Allocate memory for IO Buffer!"));
                err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
                /*free scratch*/
                goto error_alloc_IO_mem;
            }

            BDSP_MEM_P_ConvertAddressToOffset(  pDevice->memHandle,
                                            pBaseAddr,
                                            &raagaOffsetAddr
                                         );
            /* ui32BaseAddr = raagaOffsetAddr; */


            /*Mark all the I/O and I/O Gen buffers as not in use to start with and set them during stage connects. */
            pDevice->memInfo.sScratchandISBuff[i32DspIndex].InUse[i32TaskIndex]=0;

            for(k=0; k<ui32NumCh; k++)
            {
                pDevice->memInfo.sScratchandISBuff[i32DspIndex].InterStageIOBuff[i32TaskIndex].sCircBuffer[k].ui32BaseAddr  =
                                                    raagaOffsetAddr  + (k*ui32TempIs);
                pDevice->memInfo.sScratchandISBuff[i32DspIndex].InterStageIOBuff[i32TaskIndex].sCircBuffer[k].ui32ReadAddr  =
                                                    pDevice->memInfo.sScratchandISBuff[i32DspIndex].InterStageIOBuff[i32TaskIndex].sCircBuffer[k].ui32BaseAddr;
                pDevice->memInfo.sScratchandISBuff[i32DspIndex].InterStageIOBuff[i32TaskIndex].sCircBuffer[k].ui32WriteAddr =
                                                    pDevice->memInfo.sScratchandISBuff[i32DspIndex].InterStageIOBuff[i32TaskIndex].sCircBuffer[k].ui32BaseAddr;
                pDevice->memInfo.sScratchandISBuff[i32DspIndex].InterStageIOBuff[i32TaskIndex].sCircBuffer[k].ui32EndAddr   =
                                                    pDevice->memInfo.sScratchandISBuff[i32DspIndex].InterStageIOBuff[i32TaskIndex].sCircBuffer[k].ui32BaseAddr + ui32TempIs;
                pDevice->memInfo.sScratchandISBuff[i32DspIndex].InterStageIOBuff[i32TaskIndex].sCircBuffer[k].ui32WrapAddr  =
                                                    pDevice->memInfo.sScratchandISBuff[i32DspIndex].InterStageIOBuff[i32TaskIndex].sCircBuffer[k].ui32EndAddr;
            }
            pDevice->memInfo.sScratchandISBuff[i32DspIndex].InterStageIOBuff[i32TaskIndex].ui32NumBuffers = ui32NumCh;/*worst case, maxchannels*/
            pDevice->memInfo.sScratchandISBuff[i32DspIndex].InterStageIOBuff[i32TaskIndex].eBufferType    = BDSP_AF_P_BufferType_eDRAM_IS;


            /* Allocate IO Generic Memory and also the configuration structure*/

            pBaseAddr = BDSP_MEM_P_AllocateAlignedMemory(pDevice->memHandle, ui32TempIsIf ,2, 0);   /* 32 bit aligned*/

            if(NULL == pBaseAddr)
            {
                BDBG_ERR(("BDSP_MM_P_CalcandAllocScratchISbufferReq: Unable to Allocate memory for IO Generic!"));
                err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
                /*Free scratch and IO buffer here*/
                goto error_alloc_IOGen_mem;
            }

            BDSP_MEM_P_ConvertAddressToOffset(  pDevice->memHandle,
                                            pBaseAddr,
                                            &raagaOffsetAddr
                                         );

            pDevice->memInfo.sScratchandISBuff[i32DspIndex].InterStageIOGenericBuff[i32TaskIndex].ui32NumBuffers= 1;
            pDevice->memInfo.sScratchandISBuff[i32DspIndex].InterStageIOGenericBuff[i32TaskIndex].eBufferType   = BDSP_AF_P_BufferType_eDRAM_IS;
            pDevice->memInfo.sScratchandISBuff[i32DspIndex].InterStageIOGenericBuff[i32TaskIndex].sCircBuffer.ui32BaseAddr  =
                                                    raagaOffsetAddr;
            pDevice->memInfo.sScratchandISBuff[i32DspIndex].InterStageIOGenericBuff[i32TaskIndex].sCircBuffer.ui32ReadAddr  =
                                                    pDevice->memInfo.sScratchandISBuff[i32DspIndex].InterStageIOGenericBuff[i32TaskIndex].sCircBuffer.ui32BaseAddr;
            pDevice->memInfo.sScratchandISBuff[i32DspIndex].InterStageIOGenericBuff[i32TaskIndex].sCircBuffer.ui32WriteAddr =
                                                    pDevice->memInfo.sScratchandISBuff[i32DspIndex].InterStageIOGenericBuff[i32TaskIndex].sCircBuffer.ui32BaseAddr;
            pDevice->memInfo.sScratchandISBuff[i32DspIndex].InterStageIOGenericBuff[i32TaskIndex].sCircBuffer.ui32EndAddr   =
                                                    pDevice->memInfo.sScratchandISBuff[i32DspIndex].InterStageIOGenericBuff[i32TaskIndex].sCircBuffer.ui32BaseAddr + ui32TempIsIf;
            pDevice->memInfo.sScratchandISBuff[i32DspIndex].InterStageIOGenericBuff[i32TaskIndex].sCircBuffer.ui32WrapAddr  =
                                                    pDevice->memInfo.sScratchandISBuff[i32DspIndex].InterStageIOGenericBuff[i32TaskIndex].sCircBuffer.ui32EndAddr;

            /*Frame sync and Dec/Enc/PassTh will reuse the same IO and IOGeneric buffer*/
        }
    }
    goto end;
error_alloc_IOGen_mem:
    pAddress = NULL;
    BDSP_MEM_P_ConvertOffsetToCacheAddr(
        pDevice->memHandle,
        pDevice->memInfo.sScratchandISBuff[i32DspIndex].InterStageIOBuff[i32TaskIndex].sCircBuffer[0].ui32BaseAddr,
        &pAddress);
    if(pAddress)
    {
        BDSP_MEM_P_FreeMemory((void *)pDevice->memHandle, pAddress);
    }
error_alloc_IO_mem:
    while(i32TaskIndex!=0)
    {
        i32TaskIndex--;
        pAddress = NULL;
        BDSP_MEM_P_ConvertOffsetToCacheAddr(
            pDevice->memHandle,
            pDevice->memInfo.sScratchandISBuff[i32DspIndex].InterStageIOGenericBuff[i32TaskIndex].sCircBuffer.ui32BaseAddr,
            &pAddress);
        if(pAddress)
        {
            BDSP_MEM_P_FreeMemory((void *)pDevice->memHandle, pAddress);
        }
        pAddress = NULL;
        BDSP_MEM_P_ConvertOffsetToCacheAddr(
            pDevice->memHandle,
            pDevice->memInfo.sScratchandISBuff[i32DspIndex].InterStageIOBuff[i32TaskIndex].sCircBuffer[0].ui32BaseAddr,
            &pAddress);
        if(pAddress)
        {
            BDSP_MEM_P_FreeMemory((void *)pDevice->memHandle, pAddress);
        }
    }
        if (pDevice->memInfo.sScratchandISBuff[i32DspIndex].ui32DspScratchMemGrant.ui32BufferSizeInBytes)
        {
            pAddress = NULL;
            BDSP_MEM_P_ConvertOffsetToCacheAddr(
                    pDevice->memHandle,
                    pDevice->memInfo.sScratchandISBuff[i32DspIndex].ui32DspScratchMemGrant.ui32DramBufferAddress,
                    &pAddress);
            if(pAddress)
            {
                BDSP_MEM_P_FreeMemory((void *)pDevice->memHandle, pAddress);
            }
        }
error_alloc_scratch_mem:

    while(i32DspIndex!=0)
    {
        i32DspIndex--;
        for((i32TaskIndex=(BDSP_RAAGA_MAX_BRANCH_PER_TASK-1)); i32TaskIndex >= 0; i32TaskIndex--)
        {
            pAddress = NULL;
            BDSP_MEM_P_ConvertOffsetToCacheAddr(
                pDevice->memHandle,
                pDevice->memInfo.sScratchandISBuff[i32DspIndex].InterStageIOGenericBuff[i32TaskIndex].sCircBuffer.ui32BaseAddr,
                &pAddress);
            if(pAddress)
            {
                BDSP_MEM_P_FreeMemory((void *)pDevice->memHandle, pAddress);
            }
            pAddress = NULL;
            BDSP_MEM_P_ConvertOffsetToCacheAddr(
                pDevice->memHandle,
                pDevice->memInfo.sScratchandISBuff[i32DspIndex].InterStageIOBuff[i32TaskIndex].sCircBuffer[0].ui32BaseAddr,
                &pAddress);
            if(pAddress)
            {
                BDSP_MEM_P_FreeMemory((void *)pDevice->memHandle, pAddress);
            }
        }
        pAddress = NULL;
        BDSP_MEM_P_ConvertOffsetToCacheAddr(
            pDevice->memHandle,
            pDevice->memInfo.sScratchandISBuff[i32DspIndex].ui32DspScratchMemGrant.ui32DramBufferAddress,
            &pAddress);
        if(pAddress)
        {
            BDSP_MEM_P_FreeMemory((void *)pDevice->memHandle, pAddress);
        }
    }
end:
    BDBG_LEAVE(BDSP_MM_P_CalcandAllocScratchISbufferReq);
    return err;
}

BERR_Code BDSP_Raaga_P_FreeStageMemory(
    void *pStageHandle
    )
{
    BERR_Code   err = BERR_SUCCESS;
    void *pAddress=NULL;
    BDSP_RaagaStage *pRaagaStage = (BDSP_RaagaStage *)pStageHandle;

    BDBG_ENTER(BDSP_Raaga_P_FreeStageMemory);

    /* Free the interframe buffer allocation */
    if(pRaagaStage->sDramInterFrameBuffer.ui32DramBufferAddress)
    {
        BDSP_MEM_P_ConvertOffsetToCacheAddr(pRaagaStage->pContext->pDevice->memHandle,
                                    pRaagaStage->sDramInterFrameBuffer.ui32DramBufferAddress,
                                    &pAddress);

        BDSP_MEM_P_FreeMemory(pRaagaStage->pContext->pDevice->memHandle, pAddress);
    }

    /* Free the user settings buffer allocation */

    if(pRaagaStage->sDramUserConfigBuffer.ui32DramBufferAddress)
    {
        BDSP_MEM_P_ConvertOffsetToCacheAddr(pRaagaStage->pContext->pDevice->memHandle,
                                    pRaagaStage->sDramUserConfigBuffer.ui32DramBufferAddress,
                                    &pAddress);

        BDSP_MEM_P_FreeMemory(pRaagaStage->pContext->pDevice->memHandle, pAddress);
    }

    /* Free the BDSP working buffer of user settings */
    if(pRaagaStage->sDramUserConfigSpareBuffer.ui32DramBufferAddress )
    {
        BDSP_MEM_P_ConvertOffsetToCacheAddr(pRaagaStage->pContext->pDevice->memHandle,
                                pRaagaStage->sDramUserConfigSpareBuffer.ui32DramBufferAddress,
                                &pAddress);

        BDSP_MEM_P_FreeMemory(pRaagaStage->pContext->pDevice->memHandle, pAddress);
    }

    /* Free the status buffer allocation */
    if(pRaagaStage->sDramStatusBuffer.ui32DramBufferAddress)
    {
        BDSP_MEM_P_ConvertOffsetToCacheAddr(pRaagaStage->pContext->pDevice->memHandle,
                                    pRaagaStage->sDramStatusBuffer.ui32DramBufferAddress,
                                    &pAddress);

        BDSP_MEM_P_FreeMemory(pRaagaStage->pContext->pDevice->memHandle, pAddress);
    }

    /* Free the descriptor buffer allocation */
    BDSP_MEM_P_FreeMemory(pRaagaStage->pContext->pDevice->memHandle, pRaagaStage->sStageInput[0].pIoBuffDesc);

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
    BDSP_Raaga *pDevice = (BDSP_Raaga *)pRaagaContext->pDevice;

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
        if ((pRaagaTask->pFeedbackBuffer!=NULL))
        {
            BDBG_MSG(("BDSP_Raaga_P_FreeTaskMemory: Freeing pRaagaTask->pFeedbackBuffer = 0x%p",pRaagaTask->pFeedbackBuffer));
            BDSP_MEM_P_FreeMemory(pDevice->memHandle, pRaagaTask->pFeedbackBuffer);
            pRaagaTask->pFeedbackBuffer = NULL;
        }
    }

    BDSP_MEM_P_FreeMemory(pDevice->memHandle,pRaagaTask->taskMemGrants.sTaskQueue.sTaskSyncQueue.pBaseAddr);
    BDSP_MEM_P_FreeMemory(pDevice->memHandle,pRaagaTask->taskMemGrants.sTaskQueue.sTaskAsyncQueue.pBaseAddr);

    if ( pRaagaTask->taskMemGrants.sTaskQueue.sAsyncMsgBufmem.pBaseAddr )
    {
        BKNI_Free((void *)pRaagaTask->taskMemGrants.sTaskQueue.sAsyncMsgBufmem.pBaseAddr);
        pRaagaTask->taskMemGrants.sTaskQueue.sAsyncMsgBufmem.pBaseAddr = 0;
    }

    BDSP_MEM_P_FreeMemory(pDevice->memHandle,pRaagaTask->taskMemGrants.sTaskGateOpenBufInfo.pBaseAddr);
    BDSP_MEM_P_FreeMemory(pDevice->memHandle,pRaagaTask->taskMemGrants.sStackSwapBuff.pBaseAddr);
    BDSP_MEM_P_FreeMemory(pDevice->memHandle,pRaagaTask->taskMemGrants.sTaskCfgBufInfo.pBaseAddr);
    BDSP_MEM_P_FreeMemory(pDevice->memHandle,pRaagaTask->taskMemGrants.sTaskInfo.pBaseAddr);
    BDSP_MEM_P_FreeMemory(pDevice->memHandle,pRaagaTask->taskMemGrants.sCitStruct.pBaseAddr);

    if(pRaagaTask->taskMemGrants.sSpareCitStruct.pBaseAddr != NULL ){
    BDSP_MEM_P_FreeMemory(pDevice->memHandle,pRaagaTask->taskMemGrants.sSpareCitStruct.pBaseAddr);
    }

    if( BDSP_ContextType_eVideo == pRaagaContext->settings.contextType)
    {
        BDSP_MEM_P_FreeMemory(pDevice->memHandle,pRaagaTask->taskMemGrants.sTaskQueue.sPDQueue.pBaseAddr);
        BDSP_MEM_P_FreeMemory(pDevice->memHandle,pRaagaTask->taskMemGrants.sTaskQueue.sPRQueue.pBaseAddr);
        BDSP_MEM_P_FreeMemory(pDevice->memHandle,pRaagaTask->taskMemGrants.sTaskQueue.sDSQueue.pBaseAddr);
    }
    else if(BDSP_ContextType_eVideoEncode == pRaagaContext->settings.contextType )
    {
        BDSP_MEM_P_FreeMemory(pDevice->memHandle,pRaagaTask->taskMemGrants.sTaskQueue.sRRQueue.pBaseAddr);
        BDSP_MEM_P_FreeMemory(pDevice->memHandle,pRaagaTask->taskMemGrants.sTaskQueue.sCCDQueue.pBaseAddr);
    }

    BDBG_LEAVE(BDSP_Raaga_P_FreeTaskMemory);

    return err;
}

BERR_Code BDSP_Raaga_P_FreeContextMemory(
    void *pContextHandle
    )
{
    BDSP_RaagaContext   *pRaagaContext = (BDSP_RaagaContext   *)pContextHandle;
    BDSP_Raaga *pDevice = (BDSP_Raaga *)pRaagaContext->pDevice;

    BERR_Code   err=BERR_SUCCESS;

    BDBG_ENTER(BDSP_Raaga_P_FreeContextMemory);

    BDBG_OBJECT_ASSERT(pRaagaContext, BDSP_RaagaContext);

    BDSP_MEM_P_FreeMemory(pDevice->memHandle,pRaagaContext->contextMemInfo.sVomTableInfo.pBaseAddr);

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
    dramaddr_t raagaOffsetAddr = 0, i;
    void *pBaseAddr = 0;

    BDSP_RaagaStage *pRaagaStage = (BDSP_RaagaStage *)pStageHandle;

    BDSP_RaagaContext   *pRaagaContext = (BDSP_RaagaContext   *)pRaagaStage->pContext;

    BDSP_Raaga *pDevice = (BDSP_Raaga *)pRaagaContext->pDevice;
    unsigned bytesreqd=0;

    err = BDSP_MM_P_CalcStageMemPoolReq ((void *)pRaagaStage);
    if(err != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Raaga_P_AllocateStageMemory: "
                    "Unable to get memory requirements for DSP!"));
        err = BERR_TRACE(err);
        goto end;
    }


    /*Interframe allocation for a stage*/
    pRaagaStage->sDramInterFrameBuffer.ui32DramBufferAddress= 0;
    pRaagaStage->sDramInterFrameBuffer.ui32BufferSizeInBytes= 0;;

    if( pRaagaStage->stageMemInfo.ui32InterframeBufReqd )
    {
        pBaseAddr = BDSP_MEM_P_AllocateAlignedMemory(pDevice->memHandle, pRaagaStage->stageMemInfo.ui32InterframeBufReqd,2, 0);  /* 32 bit aligned*/

        if(NULL == pBaseAddr)
        {
            BDBG_ERR(("BDSP_Raaga_P_AllocateStageMemory: Unable to Allocate memory for IFrameCfgBuf!"));
            err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
            goto end;
        }
        else
        {
            /*convert to physical and */
            BDSP_MEM_P_ConvertAddressToOffset(  pDevice->memHandle,
                                            pBaseAddr,
                                            &raagaOffsetAddr
                                         );
            /* ui32BaseAddr = raagaOffsetAddr; */

            pRaagaStage->sDramInterFrameBuffer.ui32DramBufferAddress= raagaOffsetAddr;
            pRaagaStage->sDramInterFrameBuffer.ui32BufferSizeInBytes= pRaagaStage->stageMemInfo.ui32InterframeBufReqd;
        }
    }

    pRaagaStage->sDramUserConfigBuffer.ui32DramBufferAddress = 0;
    pRaagaStage->sDramUserConfigBuffer.ui32BufferSizeInBytes = 0;

    if( pRaagaStage->stageMemInfo.ui32UserConfigReqd )
    {
        pBaseAddr = BDSP_MEM_P_AllocateAlignedMemory(pDevice->memHandle, pRaagaStage->stageMemInfo.ui32UserConfigReqd, 2, 0);  /* 32 bit aligned*/
        pRaagaStage->sDramUserConfigBuffer.pDramBufferAddress = pBaseAddr;

        if(NULL == pBaseAddr)
        {
            BDBG_ERR(("BDSP_Raaga_P_AllocateStageMemory: Unable to Allocate memory for Userconfig!"));
            err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
            goto end;
        }
        else
        {
            /*convert to physical and */
            BDSP_MEM_P_ConvertAddressToOffset(  pDevice->memHandle,
                                            pBaseAddr,
                                            &raagaOffsetAddr
                                         );


            pRaagaStage->sDramUserConfigBuffer.ui32DramBufferAddress = raagaOffsetAddr;
            pRaagaStage->sDramUserConfigBuffer.ui32BufferSizeInBytes = pRaagaStage->stageMemInfo.ui32UserConfigReqd;
        }
    }

    /* Allocate memory for the host buffer to hold the modified settings when the task is running */
    pRaagaStage->sDramUserConfigSpareBuffer.ui32DramBufferAddress = 0;
    pRaagaStage->sDramUserConfigSpareBuffer.ui32BufferSizeInBytes = 0;

    if(pRaagaStage->stageMemInfo.ui32UserConfigReqd)
    {
        pBaseAddr = BDSP_MEM_P_AllocateAlignedMemory(pDevice->memHandle, pRaagaStage->stageMemInfo.ui32UserConfigReqd, 2, 0);  /* 32 bit aligned*/
        pRaagaStage->sDramUserConfigSpareBuffer.pDramBufferAddress = pBaseAddr;

        if(NULL == pBaseAddr)
        {
            BDBG_ERR(("BDSP_Raaga_P_AllocateStageMemory: Unable to Allocate memory for Userconfig!"));
            err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
            goto end;
        }
        else
        {

         /* BDSP_Raaga_P_CopyDataToDram(hHeap, (uint32_t *)pSettingsBuffer, uiConfigBufAddr, settingsBufferSize);
            but initialize the buffers(user config, interframe after the Gen_cit in the task start as was done earlier.
            pSettingsBuffer is the pointer to the constant value

         */
            /*convert to physical and */
            BDSP_MEM_P_ConvertAddressToOffset(  pDevice->memHandle,
                                            pBaseAddr,
                                            &raagaOffsetAddr
                                         );

            pRaagaStage->sDramUserConfigSpareBuffer.ui32DramBufferAddress = raagaOffsetAddr;
            pRaagaStage->sDramUserConfigSpareBuffer.ui32BufferSizeInBytes = pRaagaStage->stageMemInfo.ui32UserConfigReqd;
        }
    }

    /*Status Buffer allocation for a stage*/
    pRaagaStage->sDramStatusBuffer.ui32DramBufferAddress= 0;
    pRaagaStage->sDramStatusBuffer.ui32BufferSizeInBytes= 0;

    if(pRaagaStage->stageMemInfo.ui32StatusBufReqd)
    {
        pBaseAddr = BDSP_MEM_P_AllocateAlignedMemory(pDevice->memHandle, pRaagaStage->stageMemInfo.ui32StatusBufReqd,2, 0);  /* 32 bit aligned*/
        pRaagaStage->sDramStatusBuffer.pDramBufferAddress = pBaseAddr;

        if(NULL == pBaseAddr)
        {
            BDBG_ERR(("BDSP_Raaga_P_AllocateStageMemory: Unable to Allocate memory for Status Buffer!"));
            err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
            goto end;
        }
        else
        {
            /*convert to physical and */
            BDSP_MEM_P_ConvertAddressToOffset(  pDevice->memHandle,
                                            pBaseAddr,
                                            &raagaOffsetAddr
                                         );

            pRaagaStage->sDramStatusBuffer.ui32DramBufferAddress= raagaOffsetAddr;
            pRaagaStage->sDramStatusBuffer.ui32BufferSizeInBytes= pRaagaStage->stageMemInfo.ui32StatusBufReqd;
        }
    }
    /*Configuration structure allocations for a stage*/
    bytesreqd=(SIZEOF(BDSP_AF_P_sIO_BUFFER)+ SIZEOF(BDSP_AF_P_sIO_GENERIC_BUFFER))*(BDSP_AF_P_MAX_IP_FORKS+BDSP_AF_P_MAX_OP_FORKS);

    /* For IDS output buffer descriptors */
    bytesreqd+=(SIZEOF(BDSP_AF_P_sIO_BUFFER)+ SIZEOF(BDSP_AF_P_sIO_GENERIC_BUFFER));

    pBaseAddr = BDSP_MEM_P_AllocateAlignedMemory(pDevice->memHandle, bytesreqd,2, 0);  /* 32 bit aligned*/
    if(NULL == pBaseAddr)
    {
        BDBG_ERR(("BDSP_Raaga_P_AllocateStageMemory: Unable to Allocate memory for IFrameCfgBuf!"));
        err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
        goto end;
    }
    else
    {
        /*Split the memory for all the input and output descriptors in vitual memory itself as their contents needs to be populated later.*/
        for(i=0; i<BDSP_AF_P_MAX_IP_FORKS; i++)
        {
            pRaagaStage->sStageInput[i].pIoBuffDesc = pBaseAddr;
            pRaagaStage->sStageInput[i].pIoGenBuffDesc = (void *)((uint8_t *)pBaseAddr+SIZEOF(BDSP_AF_P_sIO_BUFFER));

            pBaseAddr=(void *)((uint8_t *)pBaseAddr + SIZEOF(BDSP_AF_P_sIO_BUFFER)+ SIZEOF(BDSP_AF_P_sIO_GENERIC_BUFFER));
        }
        for(i=0; i<BDSP_AF_P_MAX_OP_FORKS; i++)
        {
            pRaagaStage->sStageOutput[i].pIoBuffDesc = pBaseAddr;
            pRaagaStage->sStageOutput[i].pIoGenBuffDesc = (void *)((uint8_t *)pBaseAddr+SIZEOF(BDSP_AF_P_sIO_BUFFER));

            pBaseAddr=(void *)((uint8_t *)pBaseAddr+ SIZEOF(BDSP_AF_P_sIO_BUFFER)+ SIZEOF(BDSP_AF_P_sIO_GENERIC_BUFFER));

            /*mark all of them invalid till stages are connected to make them valid*/
            pRaagaStage->eStageOpBuffDataType[i] = BDSP_AF_P_DistinctOpType_eInvalid;
        }

        /* IDS output buffer descriptor memory split */
        /*convert to physical only for ids stage output */
        BDSP_MEM_P_ConvertAddressToOffset(  pDevice->memHandle,
                                        pBaseAddr,
                                        &raagaOffsetAddr
                                     );

        pRaagaStage->sIdsStageOutput.ui32StageIOBuffCfgAddr=raagaOffsetAddr;
        pRaagaStage->sIdsStageOutput.ui32StageIOGenericDataBuffCfgAddr=raagaOffsetAddr+SIZEOF(BDSP_AF_P_sIO_BUFFER);
        pBaseAddr=(void *)((uint8_t *)pBaseAddr + SIZEOF(BDSP_AF_P_sIO_BUFFER)+ SIZEOF(BDSP_AF_P_sIO_GENERIC_BUFFER));
    }


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
    void *pBaseAddr = 0;
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


    pBaseAddr = BDSP_MEM_P_AllocateAlignedMemory(pDevice->memHandle, ui32UsedSize,2, 0);  /* 32 bit aligned*/
    if(NULL == pBaseAddr)
    {
        BDBG_ERR(("BDSP_Raaga_P_AllocateTaskMemory: Unable to Allocate memory for TaskCfgBuf!"));
        err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
        goto end;
    }
    else
    {
        pRaagaTask->taskMemGrants.sTaskCfgBufInfo.pBaseAddr = pBaseAddr;
        pRaagaTask->taskMemGrants.sTaskCfgBufInfo.ui32Size = ui32UsedSize;
    }


    /*No buffer allocations in the new CIT restructure during Task create time*/
    /*Interframe, usercfg, status buff will be created during stage create time*/

    /* Sync Queue */
    pBaseAddr = BDSP_MEM_P_AllocateAlignedMemory(pDevice->memHandle, (BDSP_RAAGA_MAX_MSGS_PER_QUEUE * sizeof(BDSP_Raaga_P_Response)),2, 0);
    if(NULL == pBaseAddr)
    {
        BDBG_ERR(("BDSP_Raaga_P_AllocateTaskMemory: Unable to Allocate memory for sync task queue!"));
        err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
        goto end;
    }
    pRaagaTask->taskMemGrants.sTaskQueue.sTaskSyncQueue.pBaseAddr = pBaseAddr;
    pRaagaTask->taskMemGrants.sTaskQueue.sTaskSyncQueue.uiMsgQueueSize = BDSP_RAAGA_MAX_MSGS_PER_QUEUE * sizeof(BDSP_Raaga_P_Response);

    err = BDSP_Raaga_P_AssignFreeFIFO(pDevice,pSettings->dspIndex,&(pRaagaTask->taskMemGrants.sTaskQueue.sTaskSyncQueue.i32FifoId), 1);
    if(err)
    {
        BDBG_ERR(("Unable to find free fifo for SYNC QUEUE!!!!"));
        goto end;
    }

    /* Async Queue */
    pBaseAddr =  BDSP_MEM_P_AllocateAlignedMemory(pDevice->memHandle, (BDSP_RAAGA_MAX_ASYNC_MSGS_PER_QUEUE * sizeof(BDSP_Raaga_P_AsynEventMsg)),2, 0);
    if(NULL == pBaseAddr)
    {
        BDBG_ERR(("BDSP_Raaga_P_AllocateTaskMemory: Unable to Allocate memory for async task queue!"));
        err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
        goto end;
    }
    pRaagaTask->taskMemGrants.sTaskQueue.sTaskAsyncQueue.pBaseAddr = pBaseAddr;
    pRaagaTask->taskMemGrants.sTaskQueue.sTaskAsyncQueue.uiMsgQueueSize = BDSP_RAAGA_MAX_ASYNC_MSGS_PER_QUEUE * sizeof(BDSP_Raaga_P_AsynEventMsg);

    err = BDSP_Raaga_P_AssignFreeFIFO(pDevice,pSettings->dspIndex,&(pRaagaTask->taskMemGrants.sTaskQueue.sTaskAsyncQueue.i32FifoId), 1);
    if(err)
    {
        BDBG_ERR(("Unable to find free fifo for ASYNC QUEUE!!!!"));
        goto end;
    }

    /* Async MSG Buf Memory */
    pBaseAddr = BKNI_Malloc((BDSP_RAAGA_MAX_ASYNC_MSGS_PER_QUEUE * sizeof(BDSP_Raaga_P_AsynEventMsg)));
    if(NULL == pBaseAddr)
    {
        BDBG_ERR(("BDSP_Raaga_P_AllocateTaskMemory: Unable to Allocate memory for async msgs!"));
        err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
        goto end;
    }
    pRaagaTask->taskMemGrants.sTaskQueue.sAsyncMsgBufmem.pBaseAddr = pBaseAddr;
    pRaagaTask->taskMemGrants.sTaskQueue.sAsyncMsgBufmem.ui32Size = BDSP_RAAGA_MAX_ASYNC_MSGS_PER_QUEUE * sizeof(BDSP_Raaga_P_AsynEventMsg);

    /* Task stack swap memory */
    pBaseAddr = BDSP_MEM_P_AllocateAlignedMemory(pDevice->memHandle, BDSP_CIT_P_TASK_SWAP_BUFFER_SIZE_INBYTES, 2, 0);
    if(NULL == pBaseAddr)
    {
        BDBG_ERR(("BDSP_Raaga_P_AllocateTaskMemory: Unable to Allocate memory for task stack swap!"));
        err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
        goto end;
    }
    pRaagaTask->taskMemGrants.sStackSwapBuff.pBaseAddr = pBaseAddr;
    pRaagaTask->taskMemGrants.sStackSwapBuff.ui32Size = BDSP_CIT_P_TASK_SWAP_BUFFER_SIZE_INBYTES;

    /*Gate Open Buffer configuration memory */
    pBaseAddr = BDSP_MEM_P_AllocateAlignedMemory(pDevice->memHandle,(BDSP_AF_P_MAX_OP_FORKS * SIZEOF(BDSP_AF_P_sIO_BUFFER)), 2, 0);
    if(NULL == pBaseAddr)
    {
        BDBG_ERR(("BDSP_Raaga_P_AllocateTaskMemory: Unable to Allocate memory for task Gate Open Configuration!"));
        err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
        goto end;
    }
    pRaagaTask->taskMemGrants.sTaskGateOpenBufInfo.pBaseAddr = pBaseAddr;
    pRaagaTask->taskMemGrants.sTaskGateOpenBufInfo.ui32Size = (BDSP_AF_P_MAX_OP_FORKS * SIZEOF(BDSP_AF_P_sIO_BUFFER));

    if(pRaagaContext->settings.contextType == BDSP_ContextType_eVideo)
    {

        /* Cit memory. This is now context dependent as the sizes changes for different context */
        pBaseAddr = BDSP_MEM_P_AllocateAlignedMemory(pDevice->memHandle, sizeof(BDSP_VF_P_sDEC_TASK_CONFIG),2, 0);
        if(NULL == pBaseAddr)
        {
            BDBG_ERR(("BDSP_Raaga_P_AllocateTaskMemory: Unable to Allocate memory for cit!"));
            err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
            goto end;
        }
        pRaagaTask->taskMemGrants.sCitStruct.pBaseAddr = pBaseAddr;
        pRaagaTask->taskMemGrants.sCitStruct.ui32Size = sizeof(BDSP_VF_P_sDEC_TASK_CONFIG);

        /* Allocate Picture delivery queue */
        pBaseAddr = BDSP_MEM_P_AllocateAlignedMemory(pDevice->memHandle, BDSP_RAAGA_REALVIDEO_MAX_MSGS_PER_QUEUE*4,2, 0);
        if(NULL == pBaseAddr)
        {
            BDBG_ERR(("BDSP_DSP_AllocMem: Unable to Allocate memory for Picture delivery queue!"));
            err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
            goto end;
        }

        /* Video does not require a spare cit buffer as it does not support seamless input/output port switching */
        pRaagaTask->taskMemGrants.sSpareCitStruct.pBaseAddr = 0;
        pRaagaTask->taskMemGrants.sSpareCitStruct.ui32Size = 0;

        pRaagaTask->taskMemGrants.sTaskQueue.sPDQueue.pBaseAddr = pBaseAddr;
        pRaagaTask->taskMemGrants.sTaskQueue.sPDQueue.uiMsgQueueSize = BDSP_RAAGA_REALVIDEO_MAX_MSGS_PER_QUEUE * 4;

        err = BDSP_Raaga_P_AssignFreeFIFO(pDevice,pSettings->dspIndex,&(pRaagaTask->taskMemGrants.sTaskQueue.sPDQueue.i32FifoId), 1 );
        if(err)
        {
            BDBG_ERR(("Unable to find free fifo for PDQ QUEUE!!!!"));
            goto end;
        }

        /* Allocate Picture release queue */
        pBaseAddr = BDSP_MEM_P_AllocateAlignedMemory(pDevice->memHandle, BDSP_RAAGA_REALVIDEO_MAX_MSGS_PER_QUEUE*4,2, 0);
        if(NULL == pBaseAddr)
        {
            BDBG_ERR(("BDSP_DSP_AllocMem: Unable to Allocate memory for Picture Release queue!"));
            err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
            goto end;
        }
        pRaagaTask->taskMemGrants.sTaskQueue.sPRQueue.pBaseAddr = pBaseAddr;
        pRaagaTask->taskMemGrants.sTaskQueue.sPRQueue.uiMsgQueueSize = BDSP_RAAGA_REALVIDEO_MAX_MSGS_PER_QUEUE * 4;

        err = BDSP_Raaga_P_AssignFreeFIFO(pDevice,pSettings->dspIndex,&(pRaagaTask->taskMemGrants.sTaskQueue.sPRQueue.i32FifoId), 1);
        if(err)
        {
            BDBG_ERR(("Unable to find free fifo for PR QUEUE!!!!"));
            goto end;
        }

        /* Allocate Display  queue */
        pBaseAddr = BDSP_MEM_P_AllocateAlignedMemory(pDevice->memHandle, BDSP_RAAGA_REALVIDEO_MAX_MSGS_PER_QUEUE*4,2, 0);
        if(NULL == pBaseAddr)
        {
            BDBG_ERR(("BDSP_DSP_AllocMem: Unable to Allocate memory for DS queue!"));
            err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
            goto end;
        }
        pRaagaTask->taskMemGrants.sTaskQueue.sDSQueue.pBaseAddr = pBaseAddr;
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
        pBaseAddr = BDSP_MEM_P_AllocateAlignedMemory(pDevice->memHandle, sizeof(BDSP_VF_P_sENC_TASK_CONFIG),2, 0);
        if(NULL == pBaseAddr)
        {
            BDBG_ERR(("BDSP_Raaga_P_AllocateTaskMemory: Unable to Allocate memory for cit!"));
            err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
            goto end;
        }
        pRaagaTask->taskMemGrants.sCitStruct.pBaseAddr = pBaseAddr;
        pRaagaTask->taskMemGrants.sCitStruct.ui32Size = sizeof(BDSP_VF_P_sENC_TASK_CONFIG);

        /* Video does not require a spare cit buffer as it does not support seamless input/output port switching */
        pRaagaTask->taskMemGrants.sSpareCitStruct.pBaseAddr = 0;
        pRaagaTask->taskMemGrants.sSpareCitStruct.ui32Size = 0;

        pRaagaTask->taskMemGrants.sTaskQueue.sRDQueue.uiMsgQueueSize = BDSP_RAAGA_REALVIDEO_MAX_MSGS_PER_QUEUE * 4;

        err = BDSP_Raaga_P_AssignFreeFIFO(pDevice,pSettings->dspIndex,&(pRaagaTask->taskMemGrants.sTaskQueue.sRDQueue.i32FifoId), 1);
        if(err)
        {
            BDBG_ERR(("Unable to find free fifo for RD QUEUE!!!!"));
            goto end;
        }

        /* Allocate Raw Picture release queue */
        pBaseAddr = BDSP_MEM_P_AllocateAlignedMemory(pDevice->memHandle, BDSP_RAAGA_REALVIDEO_MAX_MSGS_PER_QUEUE*4,2, 0);
        if(NULL == pBaseAddr)
        {
            BDBG_ERR(("BDSP_DSP_AllocMem: Unable to Allocate memory for Raw Picture Release queue!"));
            err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
            goto end;
        }
        pRaagaTask->taskMemGrants.sTaskQueue.sRRQueue.pBaseAddr = pBaseAddr;
        pRaagaTask->taskMemGrants.sTaskQueue.sRRQueue.uiMsgQueueSize = BDSP_RAAGA_REALVIDEO_MAX_MSGS_PER_QUEUE * 4;

        err = BDSP_Raaga_P_AssignFreeFIFO(pDevice,pSettings->dspIndex,&(pRaagaTask->taskMemGrants.sTaskQueue.sRRQueue.i32FifoId), 1);
        if(err)
        {
            BDBG_ERR(("Unable to find free fifo for RR QUEUE!!!!"));
            goto end;
        }

        /* Allocate CC data input queue */
        pBaseAddr = BDSP_MEM_P_AllocateAlignedMemory(pDevice->memHandle, BDSP_RAAGA_CC_DATA_MSGS_PER_QUEUE*sizeof(BDSP_Raaga_Video_DCCparse_ccdata),2, 0);
        if(NULL == pBaseAddr)
        {
            BDBG_ERR(("BDSP_DSP_AllocMem: Unable to Allocate memory for CC data queue!"));
            err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
            goto end;
        }
        pRaagaTask->taskMemGrants.sTaskQueue.sCCDQueue.pBaseAddr = pBaseAddr;
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
        pBaseAddr = BDSP_MEM_P_AllocateAlignedMemory(pDevice->memHandle, sizeof(BDSP_SCM_P_sTASK_CONFIG),2, 0);
        if(NULL == pBaseAddr)
        {
            BDBG_ERR(("BDSP_Raaga_P_AllocateTaskMemory: Unable to Allocate memory for cit!"));
            err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
            goto end;
        }
        pRaagaTask->taskMemGrants.sCitStruct.pBaseAddr = pBaseAddr;
        pRaagaTask->taskMemGrants.sCitStruct.ui32Size = sizeof(BDSP_SCM_P_sTASK_CONFIG);

        /* Security does not require a spare cit buffer as it does not support seamless input/output port switching */
        pRaagaTask->taskMemGrants.sSpareCitStruct.pBaseAddr = 0;
        pRaagaTask->taskMemGrants.sSpareCitStruct.ui32Size = 0;
    }

    /* Start Task memory */
   pBaseAddr = BDSP_MEM_P_AllocateAlignedMemory(pDevice->memHandle, sizeof(BDSP_P_TaskParamInfo),2, 0);
    if(NULL == pBaseAddr)
    {
        BDBG_ERR(("BDSP_Raaga_P_AllocateTaskMemory: Unable to Allocate memory for start task params!"));
        err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
        goto end;
    }
    pRaagaTask->taskMemGrants.sTaskInfo.pBaseAddr = pBaseAddr;
    pRaagaTask->taskMemGrants.sTaskInfo.ui32Size = sizeof(BDSP_P_TaskParamInfo);

    if(pRaagaContext->settings.contextType == BDSP_ContextType_eAudio)
    {
        /* Cit memory. This is now context dependent as the sizes changes for different context */
        pBaseAddr = BDSP_MEM_P_AllocateAlignedMemory(pDevice->memHandle, sizeof(BDSP_AF_P_sTASK_CONFIG),2, 0);
        if(NULL == pBaseAddr)
        {
            BDBG_ERR(("BDSP_Raaga_P_AllocateTaskMemory: Unable to Allocate memory for cit!"));
            err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
            goto end;
        }
        pRaagaTask->taskMemGrants.sCitStruct.pBaseAddr = pBaseAddr;
        pRaagaTask->taskMemGrants.sCitStruct.ui32Size = sizeof(BDSP_AF_P_sTASK_CONFIG);

        /* Allocate memory for the spare cit structure used as working buffer by basemodules */
        pBaseAddr = BDSP_MEM_P_AllocateAlignedMemory(pDevice->memHandle, sizeof(BDSP_AF_P_sTASK_CONFIG),2, 0);
        if(NULL == pBaseAddr)
        {
            BDBG_ERR(("BDSP_Raaga_P_AllocateTaskMemory: Unable to Allocate memory for cit!"));
            err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
            goto end;
        }
        pRaagaTask->taskMemGrants.sSpareCitStruct.pBaseAddr = pBaseAddr;
        pRaagaTask->taskMemGrants.sSpareCitStruct.ui32Size = sizeof(BDSP_AF_P_sTASK_CONFIG);

        /* Allocations related to new CIT_REFAC will come here*/
        /* As per discussion:
                Per-Task (Task Create Time)
                    Allocate (TaskCreateSettings.numInputs+numOutputs) of:
                    IOBuf structures (to be filled with FMM or RAVE addresses as required during start time)
                    IOGenBuf structures (always zeroed)
                    TASK_CONFIG structure
                and

                    Allocate Inter-stage IOBuf structures (populated with DRAM buffer addresses)
                    Allocate Inter-stage IOGenBuf structures (populated with Generic buffer address)
                    (the actual buffer allocations have happened during Raaga_Open time)
        */

        if(pRaagaTask->settings.numSrc>BDSP_MAX_INPUTS_PER_TASK || pRaagaTask->settings.numDst>BDSP_MAX_OUTPUTS_PER_TASK)
        {
            BDBG_ERR(("BDSP_Raaga_P_AllocateTaskMemory: Number of Input or Outputs or both out of order"));
            goto end;
        }
    }
    /* Better to Initialise the variables as they may not be used at all depending on the task type and configuration*/
    pRaagaTask->pFeedbackBuffer = NULL;
    pRaagaTask->hPDQueue = NULL;
    pRaagaTask->hPRQueue = NULL;
    pRaagaTask->hDSQueue = NULL;
    pRaagaTask->hRDQueue = NULL;
    pRaagaTask->hRRQueue = NULL;
    pRaagaTask->hCCDQueue = NULL;

    /*Create the Feedback Buffer required for the master task*/
    if(pSettings->masterTask == true)
    {
        pRaagaTask->pFeedbackBuffer = BDSP_MEM_P_AllocateAlignedMemory(pDevice->memHandle, (BDSP_AF_P_INTERTASK_FEEDBACK_BUFFER_SIZE + 4) ,5, 0);    /* 32 bit aligned*/
        if ( NULL == pRaagaTask->pFeedbackBuffer )
        {
            BDBG_ERR(("Unable to Allocate memory for Feedback  Buffer !"));
            err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
            goto end;
        }
        BKNI_Memset((void *)pRaagaTask->pFeedbackBuffer, 0x0, (BDSP_AF_P_INTERTASK_FEEDBACK_BUFFER_SIZE + 4));
        BDSP_MEM_P_FlushCache(pDevice->memHandle, (void *)pRaagaTask->pFeedbackBuffer, (BDSP_AF_P_INTERTASK_FEEDBACK_BUFFER_SIZE + 4));

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

BERR_Code BDSP_Raaga_P_AllocateContextMemory(
    void *pContextHandle
    )
{
    BERR_Code err = BERR_SUCCESS;
    void *pBaseAddr = 0;
    BDSP_RaagaContext   *pRaagaContext = (BDSP_RaagaContext   *)pContextHandle;
    BDSP_Raaga *pDevice = (BDSP_Raaga *)pRaagaContext->pDevice;

    BDBG_ENTER(BDSP_Raaga_P_AllocateContextMemory);

    BDBG_OBJECT_ASSERT(pRaagaContext, BDSP_RaagaContext);
    BDBG_OBJECT_ASSERT(pDevice, BDSP_Raaga);

    /* VOM Table DRAM */
    pBaseAddr = BDSP_MEM_P_AllocateAlignedMemory(pDevice->memHandle, sizeof(BDSP_VOM_Table), 2, 0);
    if(NULL == pBaseAddr)
    {
        BDBG_ERR(("BDSP_Raaga_P_AllocateContextMemory: Unable to Allocate memory for VOM Table in DRAM!"));
        err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
        goto end;
    }
    pRaagaContext->contextMemInfo.sVomTableInfo.pBaseAddr = pBaseAddr;
    pRaagaContext->contextMemInfo.sVomTableInfo.ui32Size = sizeof(BDSP_VOM_Table);

end:
    if(err != BERR_SUCCESS)
    {
        BDSP_Raaga_P_FreeContextMemory(pContextHandle);
    }
    BDBG_LEAVE(BDSP_Raaga_P_AllocateContextMemory);
    return err;
}
