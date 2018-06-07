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


BDBG_MODULE(bdsp_arm_fwdownload_priv);

extern const uint32_t BDSP_ARM_SystemID_MemoryReqd[BDSP_ARM_SystemImgId_eMax];


/**********************************************************************************
Please keep the following debug related information in-sync with the BDSP_AlgorithmType definition
***********************************************************************************/

static const char * const BDSP_P_AlgoTypeName[]=
{
    "Audio Decode",
    "Audio Passthrough",
    "Audio Encode",
    "Audio Mixer",
    "Audio EchoCanceller",
    "Audio Processing",
    "Video Decode",
    "Video Encode",
    "Security",
    "Custom"
};


/******************************************************************************
Summary:
    This Function Copies the FW exec from Image interface to buffer in the DRAM.
*******************************************************************************/
BERR_Code BDSP_Arm_P_CopyFWImageToMem(
        const BIMG_Interface *iface,
        void *pImgContext,
        BDSP_MMA_Memory *pMemory,
        unsigned firmware_id
        )
{
    void *image = NULL;
    const void *data = NULL;
    void *context = pImgContext;
    uint8_t *pMemAddr;

    uint32_t ui32Size = 0, ui32numOfChunks = 0,ui32ChunkLen = 0;
    uint32_t ui32Count = 0;
    uint32_t uiSizeCopied=0;

    BERR_Code rc = BERR_SUCCESS;
    BDBG_ASSERT(iface);
    BDBG_ASSERT(pImgContext);

    rc = iface->open(context, &image, firmware_id);
    if (rc != BERR_SUCCESS)
    {
      BDBG_ERR(("Error in Opening the Image Interface"));
      return BERR_TRACE(rc);
    }

    rc = iface->next(image, 0, &data, 8);
    if (rc != BERR_SUCCESS)
    {
      BDBG_ERR(("Error in fetching next chunk in Image Interface"));
      iface->close(image);
      return BERR_TRACE(rc);
    }

    ui32Size =((uint32_t *) data)[0];
    ui32numOfChunks = ((uint32_t *) data)[1];

    pMemAddr = (uint8_t *)pMemory->pAddr;

    /*BDBG_MSG(("Total Size = %d",ui32Size));*/
    for (ui32Count = 1;ui32Count <= ui32numOfChunks; ui32Count++)
    {
      /* The implementation of image interface has to be such that there is
        one header array and then there are firmware binary arrays each having
        bytes of size BDSP_ARM_IMG_CHUNK_SIZE. But the last array most probably will
        not have a size equal to exactly BDSP_ARM_IMG_CHUNK_SIZE. So we are testing
        for the last chunk here and getting the size based on the total firmware
        binary size and number of chunks*/

      ui32ChunkLen = (ui32Count == ui32numOfChunks) ?  \
          (ui32Size - ((ui32Count - 1)*BDSP_ARM_IMG_CHUNK_SIZE)): BDSP_ARM_IMG_CHUNK_SIZE;

      BDBG_ASSERT(ui32ChunkLen <= BDSP_ARM_IMG_CHUNK_SIZE);
        /*BDBG_MSG(("ui32Count = %d, ui32numOfChunks = %d , ui32ChunkLen =%d , "
                "pAddress = %#x",ui32Count,ui32numOfChunks,ui32ChunkLen,ui32MemAddr)); */

      rc = iface->next(image, ui32Count, &data, ui32ChunkLen);
      if (rc != BERR_SUCCESS)
      {
          BDBG_ERR(("Error in fetching next chunk in Image Interface"));;
          iface->close(image);
          return BERR_TRACE(rc);
      }


      BKNI_Memcpy((void *)pMemAddr,data,ui32ChunkLen);
      pMemAddr +=ui32ChunkLen;
      uiSizeCopied +=  ui32ChunkLen;
    }

    if(uiSizeCopied != ui32Size)
    {
      BDBG_ERR(("FW Image (Id =%#x) not downloaded properly",firmware_id));
    }

    iface->close(image);

    return BERR_SUCCESS;
}


BERR_Code BDSP_Arm_P_RequestImg(
    const BIMG_Interface *pImageInterface,
    void **pImageContext,
    BDSP_ArmImgCacheEntry *imgCache,
    unsigned imageId,
    bool bDownload,
    BDSP_MMA_Memory *pMemory
    )
{
    BERR_Code errCode=BERR_SUCCESS;


    BDBG_ASSERT(imageId < BDSP_ARM_IMG_ID_MAX);

    if ( imgCache[imageId].size > 0 )
    {
        BDBG_ASSERT( pMemory->pAddr != NULL );
        imgCache[imageId].pMemory = pMemory->pAddr;
        imgCache[imageId].offset  = pMemory->offset;

        if( bDownload == true )
        {
            errCode = BDSP_Arm_P_CopyFWImageToMem(pImageInterface,
                                                    pImageContext,
                                                    pMemory,
                                                    imageId);
            if ( errCode )
            {
                imgCache[imageId].pMemory = NULL;
                imgCache[imageId].offset = 0;
                return BERR_TRACE(errCode);
            }
        }
    }
    return BERR_SUCCESS;
}



BERR_Code BDSP_Arm_P_PreLoadFwImages(const BIMG_Interface *pImageInterface,
                                            void **pImageContext,
                                            BDSP_ArmImgCacheEntry *imgCache,
                                            BDSP_MMA_Memory *pMemory,
                                            unsigned ui32AllocatedSize
)
{
    unsigned imageId;
    BDSP_MMA_Memory MemoryInit = *pMemory;
    bool bDownload = true;
    size_t size_check=0;
    BERR_Code errCode=BERR_SUCCESS;


    for (imageId=0; imageId<BDSP_ARM_IMG_ID_MAX; imageId++)
    {
		int32_t mask=0xFFFFFF80;
		int64_t mask_ext=(intptr_t) mask;

        if(imgCache[imageId].size == 0 )
		{
            continue;
        }

        /* Making the address 16byte aligned */
		pMemory->pAddr = (void *)(uintptr_t)((((uintptr_t)pMemory->pAddr+127)& mask_ext));
		pMemory->offset= (((pMemory->offset+127)& mask_ext));
        errCode = BDSP_Arm_P_RequestImg(pImageInterface,pImageContext, imgCache, imageId, bDownload, pMemory);
        pMemory->pAddr = (void *)((uint8_t *)pMemory->pAddr + imgCache[imageId].size);
        pMemory->offset = pMemory->offset + imgCache[imageId].size;

        if ( errCode ){
            errCode = BERR_TRACE(errCode);
            goto error;
        }
        size_check+=imgCache[imageId].size;

    }

    /* Error Check */
    if(pMemory->pAddr > (void *)((uint8_t *)MemoryInit.pAddr + ui32AllocatedSize))
    {
        BDBG_ERR(("Used memory more than allocated memory.MemInfo size parameter might be corrupted.\
        Used till %p Allocated till %p -- exclusive", pMemory->pAddr, \
        (void *)((uint8_t *)MemoryInit.pAddr + ui32AllocatedSize)));
        errCode = BERR_TRACE(BERR_INVALID_PARAMETER);
        goto error;
    }

error:

    return errCode;
}

BERR_Code BDSP_Arm_P_AssignMem_DwnldBuf(void *pDeviceHandle, BDSP_MMA_Memory *pMemory)
{
    BDSP_Arm *pDevice = (BDSP_Arm *)pDeviceHandle;
    uint32_t i=0, j=0;
    BDSP_Arm_P_AlgoTypeImgBuf *pAlgoBufInfo;
    BERR_Code retVal = BERR_SUCCESS;
    BDSP_Arm_P_DwnldBufUsageInfo *pDwnldBufInfo;

    BDBG_ENTER(BDSP_Arm_P_AssignMem_DwnldBuf);
    BDBG_OBJECT_ASSERT(pDevice, BDSP_Arm);

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
    BDBG_LEAVE(BDSP_Arm_P_AssignMem_DwnldBuf);
    return retVal;
}

BERR_Code BDSP_Arm_P_Dwnld_AudioProc_Algos(void* pDeviceHandle, BDSP_MMA_Memory *pMemory)
{
    const BDSP_Arm_P_AlgorithmInfo *pInfo;
    unsigned i=0;
    BDSP_Algorithm algorithm;
    bool bDownload = true;
    BERR_Code errCode = BERR_SUCCESS;
    BDSP_Arm *pDevice = pDeviceHandle;
    BDSP_MMA_Memory MemoryInit = *pMemory;

    BDBG_OBJECT_ASSERT(pDevice, BDSP_Arm);

    for(algorithm= BDSP_Algorithm_eAudioProcessing_StrtIdx ; algorithm <= BDSP_Algorithm_eAudioProcessing_EndIdx ;algorithm++)
    {
        pInfo = BDSP_Arm_P_LookupAlgorithmInfo(algorithm);
        BDBG_MSG(("Requesting algorithm %s (%u) num nodes %d at %p", pInfo->pName, algorithm,pInfo->algoExecInfo.NumNodes, pMemory->pAddr));
        if ( pInfo->supported )
        {
            for ( i = 0; i < pInfo->algoExecInfo.NumNodes; i++ )
            {
                BDSP_ARM_AF_P_AlgoId algoId = pInfo->algoExecInfo.eAlgoIds[i];
                bDownload = true;

                if ( algoId < BDSP_ARM_AF_P_AlgoId_eMax )
                {
/*                    BDBG_MSG((" AlgoId (%d) %s", algoId, algoidname[algoId]));*/
#if 0  /* TBD : CODE_DOWNLOAD Enable when individual algo code download is enabled */
                    errCode = BDSP_Arm_P_RequestImg(pDevice->settings.pImageInterface,pDevice->settings.pImageContext,\
                                                        pDevice->imgCache, BDSP_ARM_IMG_ID_CODE(algoId), bDownload, pMemory);
                    IF_ERR_GOTO_error;
                    BDSP_MMA_P_FlushCache((*pMemory),pDevice->imgCache[BDSP_ARM_IMG_ID_CODE(algoId)].size);
                    pMemory->pAddr = (void *)((uint8_t *)pMemory->pAddr + pDevice->imgCache[BDSP_ARM_IMG_ID_CODE(algoId)].size);
                    pMemory->offset= pMemory->offset + pDevice->imgCache[BDSP_ARM_IMG_ID_CODE(algoId)].size;
#endif
                    errCode = BDSP_Arm_P_RequestImg(pDevice->settings.pImageInterface,pDevice->settings.pImageContext,\
                                                        pDevice->imgCache, BDSP_ARM_IMG_ID_IFRAME(algoId), bDownload, pMemory);
                    IF_ERR_GOTO_error;
                    BDSP_MMA_P_FlushCache((*pMemory),pDevice->imgCache[BDSP_ARM_IMG_ID_IFRAME(algoId)].size);
                    pMemory->pAddr = (void *)((uint8_t *)pMemory->pAddr + pDevice->imgCache[BDSP_ARM_IMG_ID_IFRAME(algoId)].size);
                    pMemory->offset= pMemory->offset + pDevice->imgCache[BDSP_ARM_IMG_ID_IFRAME(algoId)].size;

                    if ( BDSP_Arm_P_AlgoHasTables(algoId) )
                    {
                        /* Making the address 16byte aligned */
						int32_t mask=0xFFFFFF80;
						int64_t mask_ext=(intptr_t) mask;
						pMemory->pAddr = (void *)(uintptr_t)((((uintptr_t)pMemory->pAddr+127)& mask_ext));
						pMemory->offset= (uintptr_t)(((pMemory->offset+127)& mask_ext));
                        errCode = BDSP_Arm_P_RequestImg(pDevice->settings.pImageInterface,pDevice->settings.pImageContext,\
                                                            pDevice->imgCache, BDSP_ARM_IMG_ID_TABLE(algoId), bDownload, pMemory);
                        IF_ERR_GOTO_error;
                        BDSP_MMA_P_FlushCache((*pMemory),pDevice->imgCache[BDSP_ARM_IMG_ID_TABLE(algoId)].size);
                        pMemory->pAddr = (void *)((uint8_t *)pMemory->pAddr + pDevice->imgCache[BDSP_ARM_IMG_ID_TABLE(algoId)].size);
                        pMemory->offset= pMemory->offset + pDevice->imgCache[BDSP_ARM_IMG_ID_TABLE(algoId)].size;
                    }

                }

#ifdef FWDWNLD_DBG
                BDBG_MSG((" code ptr = %p (off = %p)",pDevice->imgCache[BDSP_ARM_IMG_ID_CODE(algoId)].pMemory, pDevice->imgCache[BDSP_ARM_IMG_ID_CODE(algoId)].offset));
                BDBG_MSG((" iframe ptr = %p (off = %p)",pDevice->imgCache[BDSP_ARM_IMG_ID_IFRAME(algoId)].pMemory, pDevice->imgCache[BDSP_ARM_IMG_ID_IFRAME(algoId)].offset));
                BDBG_MSG((" table ptr = %p (off = %p)",pDevice->imgCache[BDSP_ARM_IMG_ID_TABLE(algoId)].pMemory, pDevice->imgCache[BDSP_ARM_IMG_ID_TABLE(algoId)].offset));
#endif
            }
        }
    }
    if(pMemory->pAddr > (void *)((uint8_t *)MemoryInit.pAddr + pDevice->memInfo.sDwnldMemInfo.AlgoTypeBufs[BDSP_AlgorithmType_eAudioProcessing].ui32Size))
    {
        BDBG_ERR(("Used memory more than allocated memory.MemInfo size parameter might be \
            corrupted. Used till %p allocated till %p -- exclusive", pMemory->pAddr, \
            (void *)((uint8_t *)MemoryInit.pAddr + pDevice->memInfo.sDwnldMemInfo.AlgoTypeBufs[BDSP_AlgorithmType_eAudioProcessing].ui32Size) ));
        errCode = BERR_INVALID_PARAMETER;
        goto error;
    }

error:
    /* All the memory will be freed in the parent function "BDSP_Raaga_P_Alloc_DwnldFwExec" */
    return errCode;
}

void BDSP_Arm_P_FreeFwExec(   void *pDeviceHandle)
{
    BDSP_Arm *pDevice = (BDSP_Arm *)pDeviceHandle;
    unsigned imageId=0;
    BDSP_Arm_P_AlgoTypeImgBuf *pAlgoTypeBuf;
    unsigned i=0,j=0;

    BDBG_ENTER(BDSP_Arm_P_FreeFwExec);

    BDBG_OBJECT_ASSERT(pDevice, BDSP_Arm);
    if(pDevice->memInfo.sDwnldMemInfo.ImgBuf.pAddr == NULL )
        return;
    BDSP_MMA_P_FreeMemory(&(pDevice->memInfo.sDwnldMemInfo.ImgBuf));
    /* size element of both these structures are reused during watchdog so we dont zero it out here */
    for (imageId=0; imageId<BDSP_ARM_IMG_ID_MAX; imageId++)
    {
        pDevice->imgCache[imageId].pMemory = NULL;
        pDevice->imgCache[imageId].offset= 0;
    }
    for(i=0;i<BDSP_AlgorithmType_eMax ; i++ )
    {
        pAlgoTypeBuf = &pDevice->memInfo.sDwnldMemInfo.AlgoTypeBufs[i];
        for(j=0; j< BDSP_ARM_MAX_DWNLD_BUFS ; j++ )
        {
            BDSP_ARM_INIT_DWNLDBUF( &pAlgoTypeBuf->DwnldBufUsageInfo[j]);
        }
    }

    BDBG_LEAVE(BDSP_Arm_P_FreeFwExec);

}

BERR_Code BDSP_Arm_P_Alloc_DwnldFwExec(
    void *pDeviceHandle
)
{
    BERR_Code errCode = BERR_SUCCESS;
    BDSP_Arm *pDevice = pDeviceHandle;
    BDSP_Arm_P_DwnldMemInfo *pDwnldMemInfo;
    unsigned imageId;
    unsigned i=0,j=0;
    BDSP_MMA_Memory Memory;
    bool bDownload = true;
    BDSP_Arm_P_AlgoTypeImgBuf *pAlgoTypeBuf;

    BDBG_ENTER(BDSP_Arm_P_Alloc_DwnldFwExec);

    BDBG_OBJECT_ASSERT(pDevice, BDSP_Arm);

    pDwnldMemInfo = &pDevice->memInfo.sDwnldMemInfo;


    if(pDevice->deviceWatchdogFlag == true)
    {
        /* This implementation is specific to watchdog recovery.
             Traverse through all the tasks which were registered on ArmDsp
             and release all the algorithms one by one. Unregister themselves from the Device*/
        BDSP_ArmTask *pArmTask = NULL;
        for(i=0;i<BDSP_ARM_MAX_FW_TASK_PER_DSP;i++)
        {
            if(pDevice->taskDetails.pArmTask[i]!=NULL)
            {
                pArmTask = (BDSP_ArmTask *)pDevice->taskDetails.pArmTask[i];
                /* Traverse through all stages in the task and reset the running flag and task handle */
                BDSP_ARM_STAGE_TRAVERSE_LOOP_BEGIN(pArmTask->startSettings.primaryStage->pStageHandle, pStageIterator)
                BSTD_UNUSED(macroStId);
                BSTD_UNUSED(macroBrId);
                {
                    BDSP_Arm_P_ReleaseAlgorithm(pDevice, pStageIterator->algorithm);
                }
                BDSP_ARM_STAGE_TRAVERSE_LOOP_END(pStageIterator)

                pDevice->taskDetails.pArmTask[BDSP_ARM_GET_TASK_INDEX(pArmTask->taskId)]  = NULL;
                /* coverity[double_lock: FALSE] */
                BKNI_AcquireMutex(pDevice->taskDetails.taskIdMutex);
                pDevice->taskDetails.taskId[BDSP_ARM_GET_TASK_INDEX(pArmTask->taskId)]   = true;
                BKNI_ReleaseMutex(pDevice->taskDetails.taskIdMutex);
                if( pArmTask->isStopped == true)
                {
                    /* This memclear is called to ensure that start Settings for all the task which were stopped before Arm Open in
                         watchdog recovery are cleared */
                    BKNI_Memset((void *)&pArmTask->startSettings, 0, sizeof(pArmTask->startSettings));
                }
            }
        }
    }

    BDBG_MSG(("**************Allocated size %d, ptr = %p Preloaded = %d", pDwnldMemInfo->ui32AllocatedBinSize, pDwnldMemInfo->ImgBuf.pAddr, pDwnldMemInfo->IsImagePreLoaded));

    /* Download Fw */
    if(pDwnldMemInfo->ImgBuf.pAddr == NULL ){
            return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    /*Init*/
    BKNI_Memset(pDwnldMemInfo->ImgBuf.pAddr, 0, pDwnldMemInfo->ui32AllocwithGuardBand);
    for (imageId=0; imageId<BDSP_ARM_IMG_ID_MAX; imageId++)
    {
        pDevice->imgCache[imageId].pMemory = NULL;
        pDevice->imgCache[imageId].offset= 0;
    }
    for(i=0;i<BDSP_AlgorithmType_eMax ; i++ )
    {
        pAlgoTypeBuf = &pDevice->memInfo.sDwnldMemInfo.AlgoTypeBufs[i];
        for(j=0; j< BDSP_ARM_MAX_DWNLD_BUFS ; j++ )
        {
            BDSP_ARM_INIT_DWNLDBUF( &pAlgoTypeBuf->DwnldBufUsageInfo[j]);
        }
    }
    Memory = pDwnldMemInfo->ImgBuf;

    /*Rest of the Image Code download/allocation */
    if( pDevice->memInfo.sDwnldMemInfo.IsImagePreLoaded == true )
    {
        errCode = BDSP_Arm_P_PreLoadFwImages(
                        pDevice->settings.pImageInterface,
                        pDevice->settings.pImageContext,
                        pDevice->imgCache,
                        &Memory,
                        pDwnldMemInfo->ui32AllocatedBinSize
                        );
        IF_ERR_GOTO_error;
        BDSP_MMA_P_FlushCache(pDwnldMemInfo->ImgBuf, pDwnldMemInfo->ui32AllocatedBinSize);
    }
    else
    {
        /* System Code download*/
        for ( imageId = 0; imageId < BDSP_ARM_SystemImgId_eMax; imageId++ )
        {
            if(pDevice->imgCache[imageId].size == 0 ){
                    continue;
            }
            errCode = BDSP_Arm_P_RequestImg(pDevice->settings.pImageInterface,pDevice->settings.pImageContext, pDevice->imgCache, imageId, bDownload, &Memory);
            IF_ERR_GOTO_error;
            BDSP_MMA_P_FlushCache(Memory, pDevice->imgCache[imageId].size);
            Memory.pAddr = (void *)((uint8_t *)Memory.pAddr + pDevice->imgCache[imageId].size);
            Memory.offset= Memory.offset + pDevice->imgCache[imageId].size;
        }
        /* Code download happens during start task when images are not
             preloaded. Only pointers are allocated here.*/
        errCode = BDSP_Arm_P_AssignMem_DwnldBuf(pDeviceHandle, &Memory);
        IF_ERR_GOTO_error;
#if 0 /* Not Audio Post process algo to be preloaded as of now */
        errCode = BDSP_Arm_P_Dwnld_AudioProc_Algos(pDeviceHandle, &(pDevice->memInfo.sDwnldMemInfo.AlgoTypeBufs[BDSP_AlgorithmType_eAudioProcessing].DwnldBufUsageInfo[0].Memory));
        IF_ERR_GOTO_error;
#endif
    }

return errCode;

error:
    BDSP_Arm_P_FreeFwExec(pDeviceHandle);

    BDBG_LEAVE(BDSP_Arm_P_Alloc_DwnldFwExec);
    return errCode;

}

BERR_Code BDSP_Arm_P_DownloadStartTimeFWExec(
    void *pContextHandle,
    void *pPrimaryStageHandle
    )
{
    BERR_Code errCode;
    BDSP_ArmContext *pArmContext = (BDSP_ArmContext *)pContextHandle;
    BDSP_Arm  *pDevice= pArmContext->pDevice;

    BDSP_ArmStage *pArmErrorStage;
    BDSP_ArmStage *pArmPrimaryStage = (BDSP_ArmStage *)pPrimaryStageHandle;

    BDBG_ASSERT(NULL != pArmPrimaryStage);

    BDSP_ARM_STAGE_TRAVERSE_LOOP_BEGIN(pArmPrimaryStage, pStageIterator)
    BSTD_UNUSED(macroStId);
    BSTD_UNUSED(macroBrId);
    {
        errCode = BDSP_Arm_P_RequestAlgorithm(pDevice, pStageIterator->algorithm);
        if ( errCode )
        {
            errCode = BERR_TRACE(errCode);
            pArmErrorStage = pStageIterator;
            goto error;
        }
    }
    BDSP_ARM_STAGE_TRAVERSE_LOOP_END(pStageIterator)

    return BERR_SUCCESS;
error:
    BDSP_ARM_STAGE_TRAVERSE_LOOP_BEGIN(pArmPrimaryStage, pStageIterator)
    BSTD_UNUSED(macroBrId);
    BSTD_UNUSED(macroStId);
    {
        BDSP_Arm_P_ReleaseAlgorithm(pDevice, pStageIterator->algorithm);
        if (pArmErrorStage == pStageIterator)
        {
            break;
        }
    }
    BDSP_ARM_STAGE_TRAVERSE_LOOP_END(pStageIterator)

    return errCode;
}

BDSP_Arm_P_DwnldBufUsageInfo * BDSP_Arm_P_LookUpFree_DwnldBuf(void * pDeviceHandle,BDSP_Algorithm algorithm, const BDSP_Arm_P_AlgorithmInfo *pInfo)
{
    BDSP_Arm *pDevice = pDeviceHandle;
    unsigned i=0;
    BDSP_Arm_P_DwnldBufUsageInfo *pDwnldBuf = NULL;
    BDSP_Arm_P_DwnldBufUsageInfo *pDwnldBuf_AlgoPresent = NULL;
    BDSP_Arm_P_DwnldBufUsageInfo *pDwnldBuf_FreeMem = NULL;
    BDSP_Arm_P_DwnldBufUsageInfo *pDwnldBuf_ReUseMem = NULL;
    BDSP_Arm_P_AlgoTypeImgBuf *pAlgoTypeBuf = NULL;
    BDSP_AlgorithmType AlgoType = pInfo->type;

    BDBG_ENTER( BDSP_Arm_P_LookUpFree_DwnldBuf);
    BDBG_OBJECT_ASSERT(pDevice, BDSP_Arm);

    /* Make sure the sync between the names and the structure definition is maintained */
    BDBG_ASSERT((sizeof(BDSP_P_AlgoTypeName)/sizeof(char *) ) == BDSP_AlgorithmType_eMax);

    pAlgoTypeBuf = &pDevice->memInfo.sDwnldMemInfo.AlgoTypeBufs[AlgoType];

    /* Understand the usage of memory */
    for(i=0;i< pDevice->settings.maxAlgorithms[AlgoType]; i++)
    {
        pDwnldBuf = &pAlgoTypeBuf->DwnldBufUsageInfo[i];
        if( pDwnldBuf->algorithm== algorithm )
        {
            pDwnldBuf_AlgoPresent = pDwnldBuf;
            break;
        }
        else if (pDwnldBuf->algorithm == BDSP_Algorithm_eMax )
        {
            if( pDwnldBuf_FreeMem == NULL )
                pDwnldBuf_FreeMem = pDwnldBuf ;
        }
        else if (pDwnldBuf->numUser == 0)
        {
            if( pDwnldBuf_ReUseMem  == NULL )
                pDwnldBuf_ReUseMem = pDwnldBuf;

        }
    }

    /****************************************************************
    Decide which location you wish to use
    Priority:
    * Matching algoID ( algo already downloaded )
    * FreeMem ( unused )
    * NumUser == 0 , indicates that that memory is not being used presently

    Input: pDwnldBuf_AlgoPresent , pDwnldBuf_FreeMem, pDwnldBuf_ReUseMem
    Ouput: pDwnldBuf --> buffer to be used
    ****************************************************************/

    pDwnldBuf = NULL;

    if (pDwnldBuf_AlgoPresent != NULL ){
        pDwnldBuf = pDwnldBuf_AlgoPresent;
        pDwnldBuf->bIsExistingDwnldValid = true;
    }
    else if(pDwnldBuf_FreeMem != NULL ){
        pDwnldBuf = pDwnldBuf_FreeMem;
        BDSP_ARM_INIT_DWNLDBUF(pDwnldBuf);
    }
    else if(pDwnldBuf_ReUseMem != NULL )
    {
        pDwnldBuf = pDwnldBuf_ReUseMem;
        BDSP_ARM_INIT_DWNLDBUF(pDwnldBuf);
    }

    if( pDwnldBuf == NULL ){
        BDBG_ERR(("Insufficient memory for %u %s algorithms.  Max %s algorithms is set to %u",
            pDevice->settings.maxAlgorithms[pInfo->type]+1, BDSP_P_AlgoTypeName[pInfo->type],
            BDSP_P_AlgoTypeName[pInfo->type], pDevice->settings.maxAlgorithms[pInfo->type] ));

        goto    error;
    }

    pDwnldBuf->numUser++;
    pDwnldBuf->algorithm = algorithm;
    BDBG_MSG((" Request Mem = %p for algoId %s (%d)  numuser = %d existing download valid = %d" ,
        pDwnldBuf->Memory.pAddr, BDSP_P_AlgoTypeName[pInfo->type],algorithm , pDwnldBuf->numUser, pDwnldBuf->bIsExistingDwnldValid));

error:
    BDBG_LEAVE( BDSP_Arm_P_LookUpFree_DwnldBuf);
    return pDwnldBuf;

}


BERR_Code BDSP_Arm_P_Release_DwnldBufUsage(void * pDeviceHandle,BDSP_Algorithm algorithm, const BDSP_Arm_P_AlgorithmInfo *pInfo)
{
    BDSP_Arm *pDevice = pDeviceHandle;
    unsigned i=0;
    BDSP_Arm_P_DwnldBufUsageInfo *pDwnldBuf = NULL;
    BDSP_Arm_P_AlgoTypeImgBuf *pAlgoTypeBuf = NULL;
    BDSP_AlgorithmType AlgoType = pInfo->type;
    BERR_Code errCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(pDevice, BDSP_Arm);

    pAlgoTypeBuf = &pDevice->memInfo.sDwnldMemInfo.AlgoTypeBufs[AlgoType];

    for(i=0;i< pDevice->settings.maxAlgorithms[AlgoType]; i++)
    {
        pDwnldBuf = &pAlgoTypeBuf->DwnldBufUsageInfo[i];
        if( pDwnldBuf->algorithm == algorithm)
        {
            BDBG_ASSERT(pDwnldBuf->numUser > 0 );

            pDwnldBuf->numUser--;

            break;
        }
        else
        {
            pDwnldBuf = NULL;
        }
    }

    if( pDwnldBuf == NULL ){
            BDBG_ERR((" Expected match not found: For Algotype %u Algorithm: %s (%u) ",pInfo->type, pInfo->pName,pInfo->algorithm));
            errCode = BERR_TRACE(BERR_INVALID_PARAMETER);
            goto error;
    }

    BDBG_MSG((" Releasing Download buf %d type %d, algorithm (%d) numuser = %d", i, AlgoType,algorithm,pDwnldBuf->numUser ));

    error:
    return errCode;

}

/******************************************
Summary:
    Request an image in memory.

*******************************************/
BERR_Code BDSP_Arm_P_RequestAlgorithm(
    void *pDeviceHandle,
    BDSP_Algorithm algorithm        /*input*/
    )
{
    BERR_Code errCode = BERR_SUCCESS;
    BDSP_Arm *pDevice = pDeviceHandle;
    const BDSP_Arm_P_AlgorithmInfo *pInfo;
    unsigned i=0;
    BDSP_Arm_P_DwnldBufUsageInfo *pDwnldBufInfo;
    BDSP_MMA_Memory Memory;
    BDSP_MMA_Memory MemoryInit;
    bool bDownload=true;

    BDBG_OBJECT_ASSERT(pDevice, BDSP_Arm);

    pInfo = BDSP_Arm_P_LookupAlgorithmInfo(algorithm);
    if ( pInfo->supported )
    {
        if( pInfo->type == BDSP_AlgorithmType_eAudioProcessing){
            BDBG_MSG(("For Algorithm Type = AudioProcessing, Memory is already allocated statically"));
            /* All the BDSP_AlgorithmType_eAudioProcessing type of algorithms are downloaded beforehand in the function "BDSP_Arm_P_Alloc_DwnldFwExec" */
            return BERR_SUCCESS;
        }

        pDwnldBufInfo = BDSP_Arm_P_LookUpFree_DwnldBuf(pDeviceHandle, algorithm, pInfo);

        if(pDwnldBufInfo == NULL )
        {
            errCode = BERR_INVALID_PARAMETER;
            errCode = BERR_TRACE(errCode);
            goto error;
        }

        if(pDwnldBufInfo->Memory.pAddr == NULL )
        {
            errCode = BERR_INVALID_PARAMETER;
            errCode = BERR_TRACE(errCode);
            goto error;
        }

        Memory = pDwnldBufInfo->Memory;
        MemoryInit = pDwnldBufInfo->Memory;

        if( pDwnldBufInfo->bIsExistingDwnldValid == true){
             bDownload = false;
        }

        BDBG_MSG(("Requesting algorithm %s (%u) ptr = %p num nodes %d", pInfo->pName, algorithm, pDwnldBufInfo->Memory.pAddr, pInfo->algoExecInfo.NumNodes));
        for ( i = 0; i < pInfo->algoExecInfo.NumNodes; i++ )
        {
            BDSP_ARM_AF_P_AlgoId algoId = pInfo->algoExecInfo.eAlgoIds[i];
            /*BDBG_MSG((" AlgoId (%d) %s", algoId, algoidname[algoId]));*/

            if ( algoId < BDSP_ARM_AF_P_AlgoId_eMax )
            {
				if((pInfo->algoExecInfo.NumNodes == 2)&& (i==1)) /* TBD : IDS is currently part of System code, hence only download algo code */
				{
	                errCode = BDSP_Arm_P_RequestImg(pDevice->settings.pImageInterface,pDevice->settings.pImageContext, pDevice->imgCache, BDSP_ARM_IMG_ID_CODE(algoId), bDownload, &Memory);
	                IF_ERR_GOTO_error;
	                BDSP_MMA_P_FlushCache(Memory, pDevice->imgCache[BDSP_ARM_IMG_ID_CODE(algoId)].size);
	                Memory.pAddr = (void *)((uint8_t *)Memory.pAddr + pDevice->imgCache[BDSP_ARM_IMG_ID_CODE(algoId)].size);
	                Memory.offset= Memory.offset + pDevice->imgCache[BDSP_ARM_IMG_ID_CODE(algoId)].size;

					errCode = BDSP_Arm_P_DownloadFwToAstra(pDevice->armDspApp.hClient,pDevice,	algoId);
					IF_ERR_GOTO_error;
				}
                errCode = BDSP_Arm_P_RequestImg(pDevice->settings.pImageInterface,pDevice->settings.pImageContext, pDevice->imgCache, BDSP_ARM_IMG_ID_IFRAME(algoId), bDownload, &Memory);
                IF_ERR_GOTO_error;
                BDSP_MMA_P_FlushCache(Memory, pDevice->imgCache[BDSP_ARM_IMG_ID_IFRAME(algoId)].size);
                Memory.pAddr = (void *)((uint8_t *)Memory.pAddr + pDevice->imgCache[BDSP_ARM_IMG_ID_IFRAME(algoId)].size);
                Memory.offset= Memory.offset + pDevice->imgCache[BDSP_ARM_IMG_ID_IFRAME(algoId)].size;
                if ( BDSP_Arm_P_AlgoHasTables(algoId) )
                {
                   int32_t mask=0xFFFFFF80;
				   int64_t mask_ext=(intptr_t) mask;
                    /* Making the address 16byte aligned */
					Memory.pAddr = (void *)(uintptr_t)((((uintptr_t)Memory.pAddr+127)& mask_ext));
					Memory.offset= (((Memory.offset+127)& mask_ext));
                    errCode = BDSP_Arm_P_RequestImg(pDevice->settings.pImageInterface,pDevice->settings.pImageContext, pDevice->imgCache, BDSP_ARM_IMG_ID_TABLE(algoId), bDownload, &Memory);
                    IF_ERR_GOTO_error;
                    BDSP_MMA_P_FlushCache(Memory, pDevice->imgCache[BDSP_ARM_IMG_ID_TABLE(algoId)].size);
                    Memory.pAddr = (void *)((uint8_t *)Memory.pAddr + pDevice->imgCache[BDSP_ARM_IMG_ID_TABLE(algoId)].size);
                    Memory.offset= Memory.offset + pDevice->imgCache[BDSP_ARM_IMG_ID_TABLE(algoId)].size;
                }
            }
#ifdef ARM_FWDWNLD_DBG
#if 0  /* TBD : CODE_DOWNLOAD Enable when individual algo code download is enabled */
            BDBG_MSG(("Code ptr = %p (off = %p)",pDevice->imgCache[BDSP_ARM_IMG_ID_CODE(algoId)].pMemory, pDevice->imgCache[BDSP_ARM_IMG_ID_CODE(algoId)].offset));
#endif
            BDBG_MSG(("iframe ptr = %p (off = %p)",pDevice->imgCache[BDSP_ARM_IMG_ID_IFRAME(algoId)].pMemory,pDevice->imgCache[BDSP_ARM_IMG_ID_IFRAME(algoId)].offset));
            BDBG_MSG(("table ptr = %p (off = %p)",pDevice->imgCache[BDSP_ARM_IMG_ID_TABLE(algoId)].pMemory, pDevice->imgCache[BDSP_ARM_IMG_ID_TABLE(algoId)].offset));
#endif
            }
        if(Memory.pAddr > (void*)((uint8_t *)MemoryInit.pAddr+pDevice->memInfo.sDwnldMemInfo.AlgoTypeBufs[pInfo->type].ui32Size)){
            BDBG_ERR((" Binary downloaded is more than the size allocated for the type %s (%d) ", BDSP_P_AlgoTypeName[pInfo->type], pInfo->type));
            errCode= BERR_INVALID_PARAMETER;
            goto error;
        }
    }
    else
    {
        BDBG_ERR(("Algorithm %u (%s) is not supported", algorithm, pInfo->pName));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    return BERR_SUCCESS;

error:

    return errCode;
}

void BDSP_Arm_P_ReleaseAlgorithm(
     void *pDeviceHandle,
     BDSP_Algorithm algorithm
     )
{

    BDSP_Arm *pDevice = pDeviceHandle;
    const BDSP_Arm_P_AlgorithmInfo *pInfo;
    BERR_Code errCode = BERR_SUCCESS;

    BDBG_ENTER(BDSP_Arm_P_ReleaseAlgorithm);
    BDBG_OBJECT_ASSERT(pDevice, BDSP_Arm);

    pInfo = BDSP_Arm_P_LookupAlgorithmInfo(algorithm);

    if( pDevice->memInfo.sDwnldMemInfo.IsImagePreLoaded == false )
    {
        if ( pInfo->supported )
        {
            if(pInfo->type == BDSP_AlgorithmType_eAudioProcessing)
            {
                /* All the post processing are allocated at the open time and will be just freed at the end.
                Usage count is of no use. So no need to release */
                return;
            }
            BDBG_MSG(("Releasing algorithm %s (%u)", pInfo->pName, algorithm));
            errCode = BDSP_Arm_P_Release_DwnldBufUsage(pDeviceHandle, algorithm, pInfo);
            if(errCode)
            {
                BDBG_ERR(("Algorithm %s (%u) could not be released properly",pInfo->pName, algorithm));
                errCode = BERR_TRACE(errCode);
            }
        }
    }

    /* We dont release the images in case of secure mode all of them are resident always until Arm_Close*/

    BDBG_LEAVE(BDSP_Arm_P_ReleaseAlgorithm);
}



unsigned BDSP_Arm_P_AssignAlgoSizes(
                const BIMG_Interface *pImageInterface,
                void **pImageContext,
                BDSP_ArmImgCacheEntry * pImgCache,
                const BDSP_UsageOptions *pUsage,
                bool UseBDSPMacro)
{
    unsigned i=0;
    const BDSP_Arm_P_AlgorithmInfo *pInfo;
    const BDSP_Arm_P_AlgorithmSupportInfo *pSupportInfo;
    BDSP_Algorithm algorithm;
    uint32_t uiCurrentSize = 0;
    size_t totalSize = 0;

    BKNI_Memset( pImgCache, 0, (sizeof(BDSP_ArmImgCacheEntry)*BDSP_ARM_IMG_ID_MAX));
    /* Compute system image requirements */
    for ( i = 0; i < BDSP_ARM_SystemImgId_eMax; i++ )
    {

        if(UseBDSPMacro)
        {
            /* Normal Path */
            BDSP_Arm_P_GetFWSize(pImageInterface, pImageContext, i, &uiCurrentSize);
        }
        else
        {
            /*Memory Estimate API Path*/
             uiCurrentSize = BDSP_ARM_SystemID_MemoryReqd[i];
        }
        pImgCache[i].size = uiCurrentSize;
        totalSize += uiCurrentSize+127;
    }

    /* Compute supported algorithm requirements */
    for ( algorithm = 0; algorithm < BDSP_Algorithm_eMax; algorithm++ )
    {
        bool supported = false;
        BDSP_ARM_AF_P_sALGO_EXEC_INFO algoExecInfo;
        const char               *pAlgoName;
        if(UseBDSPMacro)
        {
            pInfo     = BDSP_Arm_P_LookupAlgorithmInfo(algorithm);
            supported = pInfo->supported;
            pAlgoName = pInfo->pName;
            algoExecInfo = pInfo->algoExecInfo;
        }
        else
        {
            pSupportInfo = BDSP_Arm_P_LookupAlgorithmSupported(algorithm,pUsage->DolbyCodecVersion);
            supported = pUsage->Codeclist[algorithm];
            pAlgoName = pSupportInfo->pName;
            algoExecInfo = pSupportInfo->algoExecInfo;
        }
        if (supported)
        {
            for ( i = 0; i < algoExecInfo.NumNodes; i++ )
            {
                BDSP_ARM_AF_P_AlgoId algoId = algoExecInfo.eAlgoIds[i];
                if ( algoId < BDSP_ARM_AF_P_AlgoId_eMax )
                {
                    BDBG_MSG(("Sizing %s (algoid %u)", pAlgoName, algoId));
                    if ( pImgCache[BDSP_ARM_IMG_ID_CODE(algoId)].size == 0 )
                    {
                        if(UseBDSPMacro)
                        {
                            /* Normal Path */
                            BDSP_Arm_P_GetFWSize(pImageInterface, pImageContext, BDSP_ARM_IMG_ID_CODE(algoId), &uiCurrentSize);
                        }
                        else
                        {
                            /*Memory Estimate API Path*/
                            uiCurrentSize = BDSP_ARM_sNodeInfo[algoId].ui32CodeSize;
                        }
                        pImgCache[BDSP_ARM_IMG_ID_CODE(algoId)].size = uiCurrentSize;
                        BDBG_MSG(("img %u size %u", BDSP_ARM_IMG_ID_CODE(algoId), uiCurrentSize));
                        totalSize += uiCurrentSize;
                    }

                    if ((pImgCache[BDSP_ARM_IMG_ID_IFRAME(algoId)].size == 0 )&&(BDSP_ARM_sNodeInfo[algoId].ui32InterFrmBuffSize != 0))
                    {
                        if(UseBDSPMacro)
                        {
                            /* Normal Path */
                            BDSP_Arm_P_GetFWSize(pImageInterface, pImageContext, BDSP_ARM_IMG_ID_IFRAME(algoId), &uiCurrentSize);
                        }
                        else
                        {
                            /*Memory Estimate API Path*/
                            uiCurrentSize = BDSP_ARM_sNodeInfo[algoId].ui32InterFrmBuffSize;
                        }
                        pImgCache[BDSP_ARM_IMG_ID_IFRAME(algoId)].size = uiCurrentSize;
                        BDBG_MSG(("IFrame img %u size %u", BDSP_ARM_IMG_ID_IFRAME(algoId), uiCurrentSize));
                        totalSize += uiCurrentSize+127;
                    }

                    if ( BDSP_Arm_P_AlgoHasTables(algoId) )
                    {
                        if ( pImgCache[BDSP_ARM_IMG_ID_TABLE(algoId)].size == 0 )
                        {
                            if(UseBDSPMacro)
                            {
                                /* Normal Path */
                                BDSP_Arm_P_GetFWSize(pImageInterface, pImageContext, BDSP_ARM_IMG_ID_TABLE(algoId), &uiCurrentSize);
                            }
                            else
                            {
                                /*Memory Estimate API Path*/
                                uiCurrentSize = BDSP_ARM_sNodeInfo[algoId].ui32RomTableSize;
                            }
                            pImgCache[BDSP_ARM_IMG_ID_TABLE(algoId)].size = uiCurrentSize;
                            BDBG_MSG(("Table img %u size %u", BDSP_ARM_IMG_ID_TABLE(algoId), uiCurrentSize));
                            totalSize += uiCurrentSize+127;
                        }
                    }
                }
            }
        }
    }
    return totalSize;
}
