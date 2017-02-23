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

#include "bdsp_arm_priv_include.h"

BDBG_MODULE(bdsp_arm_mm);

BDBG_OBJECT_ID(BDSP_Arm);
BDBG_OBJECT_ID(BDSP_ArmContext);
BDBG_OBJECT_ID(BDSP_ArmTask);
BDBG_OBJECT_ID(BDSP_ArmStage);


BERR_Code  BDSP_Arm_P_InsertEntry_MapTable(
    BDSP_Arm_MapTableEntry *pMapTable,
	BDSP_MMA_Memory *pMemory,
    uint32_t size,
    BDSP_ARM_AF_P_MemoryMapType eMapType,
    uint32_t ui32MaxEntry
    )
{
    BERR_Code err = BERR_SUCCESS;
    uint32_t ui32PhyAddr = 0;
    BDSP_Arm_MapTableEntry *pTable = pMapTable;
    int32_t i = 0;
	void *pAddress =  pMemory->pAddr;;

    BDBG_ENTER(BDSP_Arm_P_InsertEntry_MapTable);

    if((size == 0)||(pAddress == NULL))
    {
        BDBG_ERR(("BDSP_Arm_P_InsertEntry_MapTable: Trying to MAP a invalid Entry size = %d and Address = %p",size, pAddress));
        err = BERR_TRACE(BERR_INVALID_PARAMETER);
        return err;
    }
    for(i = 0; i < (int32_t)ui32MaxEntry; i++)
    {
        if((false == pTable->entry_valid)&& (false == pTable->map_valid))
            break;
        pTable++;
    }

    if(i >= (int32_t)ui32MaxEntry)
    {
        BDBG_ERR(("BDSP_Arm_P_InsertEntry_MapTable: Cannot Insert the allocation(%p) into the MAP TABLE",pAddress));
        err = BERR_TRACE(BERR_INVALID_PARAMETER);
        return err;
    }

    if((BCHP_PHYSICAL_OFFSET & (uint32_t)pAddress)==BCHP_PHYSICAL_OFFSET)
    {
        ui32PhyAddr = (uint32_t)pAddress;
    }
    else
    {
		ui32PhyAddr = pMemory->offset;
    }
    pTable->entry_valid = true;
    pTable->offset      = ui32PhyAddr;
    pTable->size        = size;
    pTable->map_valid   = false;
    pTable->eMemMapType = eMapType;
    BDBG_LEAVE(BDSP_Arm_P_InsertEntry_MapTable);
    return err;
}

BERR_Code  BDSP_Arm_P_DeleteEntry_MapTable(
	BDSP_Arm_MapTableEntry *pMapTable,
	BDSP_MMA_Memory *pMemory,
    uint32_t ui32MaxEntry
    )
{
    BERR_Code err = BERR_SUCCESS;
    uint32_t ui32PhyAddr = 0;
    BDSP_Arm_MapTableEntry *pTable = pMapTable;
    int32_t i = 0;
	void *pAddress =  pMemory->pAddr;;

    BDBG_ENTER(BDSP_Arm_P_DeleteEntry_MapTable);

	if((BCHP_PHYSICAL_OFFSET & (uint32_t)pAddress)== BCHP_PHYSICAL_OFFSET)
	{
		ui32PhyAddr = (uint32_t)pAddress;
	}
	else
    {
		ui32PhyAddr = pMemory->offset;

    }
    while(i < (int32_t)ui32MaxEntry)
    {
        if(ui32PhyAddr == pTable->offset)
        {
           pTable->entry_valid = false;
            if(false == pTable->map_valid)
            {
                pTable->offset = 0;
                pTable->size   = 0;
                pTable->eMemMapType = BDSP_ARM_AF_P_Map_eInvalid;
            }
            break;
        }
        i++;
        pTable++;
    }

    if(i >= (int32_t)ui32MaxEntry)
    {
        BDBG_ERR(("BDSP_Arm_P_DeleteEntry_MapTable: Cannot find the Entry(%p) in the MAP Table!!!!!",pAddress));
        err = BERR_TRACE(BERR_INVALID_PARAMETER);
        return err;
    }

    BDBG_LEAVE(BDSP_Arm_P_DeleteEntry_MapTable);
    return err;
}

BERR_Code BDSP_Arm_P_RetrieveEntriesToMap(
    BDSP_Arm_MapTableEntry *pMapTable,
    BDSP_MAP_Table_Entry   *pMapUnMapArray,
    uint32_t *pui32NumEntries,
    uint32_t ui32MaxEntry
    )
{
    BERR_Code err = BERR_SUCCESS;
    BDSP_MAP_Table_Entry   *pLocalArray = pMapUnMapArray;
    int32_t i = 0;
    uint32_t count = 0;
    BDBG_ENTER(BDSP_Arm_P_RetrieveEntriesToMap);
    for(i=0; i< (int32_t)ui32MaxEntry; i++)
    {
        if((true == pMapTable[i].entry_valid)&&(false == pMapTable[i].map_valid))
        {
            pLocalArray->ui32PageStart = pMapTable[i].offset;
            pLocalArray->ui32Size      = pMapTable[i].size;
            pLocalArray->eMemMapType   = pMapTable[i].eMemMapType;

            pMapTable[i].map_valid = true;
            count++;
            pLocalArray++;
        }
    }
    *pui32NumEntries = count;

    BDBG_MSG(("Number Entries to be Mapped = %d",count));
    BDBG_LEAVE(BDSP_Arm_P_RetrieveEntriesToMap);
    return err;
}

BERR_Code BDSP_Arm_P_RetrieveEntriesToUnMap(
    BDSP_Arm_MapTableEntry *pMapTable,
    BDSP_MAP_Table_Entry   *pMapUnMapArray,
    uint32_t *pui32NumEntries,
    uint32_t ui32MaxEntry
    )
{
    BERR_Code err = BERR_SUCCESS;
    BDSP_MAP_Table_Entry   *pLocalArray = pMapUnMapArray;
    int32_t i = 0;
    uint32_t count = 0;
    BDBG_ENTER(BDSP_Arm_P_RetrieveEntriesToUnMap);
    for(i=0; i < (int32_t)ui32MaxEntry; i++)
    {
        if((true == pMapTable[i].entry_valid)&&(true == pMapTable[i].map_valid))
        {
            pLocalArray->ui32PageStart = pMapTable[i].offset;
            pLocalArray->ui32Size      = pMapTable[i].size;
            pLocalArray->eMemMapType   = pMapTable[i].eMemMapType;

            pMapTable[i].map_valid = false;
            count++;
            pLocalArray++;
        }
    }
    *pui32NumEntries = count;

    BDBG_MSG(("Number Entries to be UnMapped = %d",count));
    BDBG_LEAVE(BDSP_Arm_P_RetrieveEntriesToUnMap);
    return err;
}

void BDSP_Arm_P_InitMem( void *pDeviceHandle)
{

    BDSP_Arm *pDevice = (BDSP_Arm *)pDeviceHandle;
    unsigned i=0,j=0;
    BDSP_Arm_P_AlgoTypeImgBuf *pAlgoTypeBuf;

    BDBG_ENTER(BDSP_Arm_P_InitMem);

    BDBG_OBJECT_ASSERT(pDevice, BDSP_Arm);

    BKNI_Memset(&pDevice->memInfo, 0, sizeof(pDevice->memInfo));

    for(i=0;i<BDSP_AlgorithmType_eMax ; i++ )
    {
        pAlgoTypeBuf = &pDevice->memInfo.sDwnldMemInfo.AlgoTypeBufs[i];
        for(j=0; j< BDSP_ARM_MAX_DWNLD_BUFS ; j++ )
        {
            BDSP_ARM_INIT_DWNLDBUF( &pAlgoTypeBuf->DwnldBufUsageInfo[j]);
        }
    }

    BDBG_LEAVE(BDSP_Arm_P_InitMem);

}

/*****************************************************************************
Function Name: BDSP_MM_P_GetFwMemRequired

            If the algorithm is supported, size for that algo is calculated
                and the required cumulative memory requirement is found out.

*****************************************************************************/


BERR_Code BDSP_ARM_MM_P_GetFwMemRequired(
        const BDSP_ArmSettings  *pSettings,
        BDSP_Arm_P_DwnldMemInfo *pDwnldMemInfo,      /*[out]*/
        void                      *pImg,
        bool                       UseBDSPMacro,
        const BDSP_ArmUsageOptions *pUsage
)
{
    BERR_Code ret= BERR_SUCCESS;
    unsigned i=0, j=0, size=0;
    const BDSP_Arm_P_AlgorithmInfo *pInfo;
    BDSP_Arm_P_AlgoTypeImgBuf * pAlgoTypeBuf;
    BDSP_ArmImgCacheEntry     * pImgCache;
    unsigned systemImgSize = 0;
    BDSP_Algorithm algorithm;
    size_t totalSize = 0;
    uint32_t ui32AlgorithmSize = 0;
    const BDSP_Arm_P_AlgorithmSupportInfo *pSupportInfo;

    BDSP_AlgorithmType        Algotype;
    BDSP_ARM_AF_P_sALGO_EXEC_INFO algoExecInfo;

    pImgCache    = (BDSP_ArmImgCacheEntry *)pImg;
    pAlgoTypeBuf = &pDwnldMemInfo->AlgoTypeBufs[0];
    BKNI_Memset( pImgCache, 0, (sizeof(BDSP_ArmImgCacheEntry)*BDSP_ARM_IMG_ID_MAX));

    totalSize = BDSP_Arm_P_AssignAlgoSizes(
                    pSettings->pImageInterface,
                    pSettings->pImageContext,
                    (BDSP_ArmImgCacheEntry * )pImg,
                    pUsage,
                    UseBDSPMacro);

    systemImgSize =0;

    for ( i = 0; i < BDSP_ARM_SystemImgId_eMax; i++ )
    {
        systemImgSize += pImgCache[i].size+127;
    }

    for ( algorithm = 0; algorithm < BDSP_Algorithm_eMax; algorithm++ )
    {
        if(UseBDSPMacro)
        {
            pInfo     = BDSP_Arm_P_LookupAlgorithmInfo(algorithm);
            Algotype  = pInfo->type;
            algoExecInfo = pInfo->algoExecInfo;
        }
        else
        {
            pSupportInfo = BDSP_Arm_P_LookupAlgorithmSupported(algorithm,pUsage->DolbyCodecVersion);
            Algotype  = pSupportInfo->type;
            algoExecInfo = pSupportInfo->algoExecInfo;
        }

        ui32AlgorithmSize = 0;
        for ( i = 0; i < algoExecInfo.NumNodes; i++ )
        {
            BDSP_AF_P_AlgoId algoId = algoExecInfo.eAlgoIds[i];
            if ( algoId < BDSP_AF_P_AlgoId_eMax )
            {
#if 0  /* TBD : CODE_DOWNLOAD Enable when individual algo code download is enabled */
                ui32AlgorithmSize += pImgCache[BDSP_ARM_IMG_ID_CODE(algoId)].size;
#endif
                ui32AlgorithmSize += pImgCache[BDSP_ARM_IMG_ID_IFRAME(algoId)].size;
                ui32AlgorithmSize += pImgCache[BDSP_ARM_IMG_ID_TABLE(algoId)].size;

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
            pDwnldMemInfo->ui32AllocatedBinSize += size+127;
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
        pDwnldMemInfo->ui32AllocwithGuardBand += BDSP_ARM_CODE_DWNLD_GUARD_BAND_SIZE;

    return ret;
}


BERR_Code BDSP_ARM_MM_P_GetFwMemRequirement(BDSP_Arm *pDevice)
{
	BERR_Code errCode;
    BDSP_Arm_P_DwnldMemInfo *pDwnldMemInfo;

    BDBG_OBJECT_ASSERT(pDevice, BDSP_Arm);

    pDwnldMemInfo = &pDevice->memInfo.sDwnldMemInfo;

    BDSP_ARM_MM_P_GetFwMemRequired(&(pDevice->settings),pDwnldMemInfo,(void *)&(pDevice->imgCache[0]),true,NULL);

	errCode = BDSP_MMA_P_AllocateAlignedMemory(pDevice->memHandle,pDwnldMemInfo->ui32AllocwithGuardBand, &(pDwnldMemInfo->ImgBuf),BDSP_MMA_Alignment_32bit);
	if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("Cannot allocate the requested IMG Memory"));
		return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
	}
    BDBG_MSG(("**************Allocated size %d, Bin Size = %d, ptr = %p Preloaded = %d", pDwnldMemInfo->ui32AllocwithGuardBand,pDwnldMemInfo->ui32AllocatedBinSize,pDwnldMemInfo->ImgBuf.pAddr,pDwnldMemInfo->IsImagePreLoaded));
    pDevice->fwHeapSize = pDwnldMemInfo->ui32AllocwithGuardBand;
    pDevice->pFwHeapMemory = pDwnldMemInfo->ImgBuf.pAddr;
	pDevice->FwHeapOffset = pDwnldMemInfo->ImgBuf.offset;
    return errCode;
}



BERR_Code BDSP_Arm_P_AllocateInitMemory (
    void *pDeviceHandle
    )
{
    BERR_Code err = BERR_SUCCESS;
    BDSP_Arm *pDevice = (BDSP_Arm *)pDeviceHandle;

    BDBG_ENTER(BDSP_Arm_P_AllocateInitMemory);

    BDSP_Arm_P_InitMem(pDeviceHandle);

    /* Get memory download requirement here */
    err = BDSP_ARM_MM_P_GetFwMemRequirement(pDeviceHandle);
    if(BERR_SUCCESS != err)
    {
        BDBG_ERR(("BDSP_MEM_P_CalcMemPoolReq: Error getting firmware memory requirements!"));
        err = BERR_TRACE(err);
        goto err_calcMemPoolReq;
    }

    /* Allocate memory for storing cmd q handles */
	err = BDSP_MMA_P_AllocateAlignedMemory(pDevice->memHandle,
						(BDSP_ARM_NUM_INTERFACE_QUEUE_HANDLE * sizeof(BDSP_AF_P_sDRAM_CIRCULAR_BUFFER)),
						&(pDevice->sArmInterfaceQ.Memory),
						BDSP_MMA_Alignment_32bit);
	if (BERR_SUCCESS != err)
	{
		BDBG_ERR(("BDSP_Arm_P_AllocateInitMemory: Unable to Allocate memory for storing cmd q handles!"));
		err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
		goto err_alloc_QueueHndl;
	}
	pDevice->sArmInterfaceQ.parmInterfaceQHndl =  pDevice->sArmInterfaceQ.Memory.pAddr;
	err = BDSP_Arm_P_InsertEntry_MapTable(&(pDevice->sDeviceMapTable[0]), &pDevice->sArmInterfaceQ.Memory, (BDSP_ARM_NUM_INTERFACE_QUEUE_HANDLE * sizeof(BDSP_AF_P_sDRAM_CIRCULAR_BUFFER)),BDSP_ARM_AF_P_Map_eDram, BDSP_ARM_MAX_ALLOC_DEVICE);
	if (BERR_SUCCESS != err)
	{
		BDBG_ERR(("BDSP_Arm_P_AllocateInitMemory: Error in updating the MAP Table for cmd q handle memory"));
		err = BERR_TRACE(err);
		goto err_updating_maptable_cmd_q_hndl;
	}
    /* Allocate Command queue */
	err = BDSP_MMA_P_AllocateAlignedMemory(pDevice->memHandle,
						(BDSP_ARM_MAX_MSGS_PER_QUEUE * sizeof(BDSP_Arm_P_Command)*BDSP_ARM_MAX_FW_TASK_PER_DSP),
						&(pDevice->memInfo.cmdQueueParams.Queue),
						BDSP_MMA_Alignment_32bit);
	if (BERR_SUCCESS != err)
	{
		BDBG_ERR(("BDSP_Arm_P_AllocateInitMemory: Unable to Allocate memory for cmd queue!"));
		err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
		goto err_alloc_CmdQueue;
	}
	pDevice->memInfo.cmdQueueParams.uiMsgQueueSize = BDSP_ARM_MAX_MSGS_PER_QUEUE * sizeof(BDSP_Arm_P_Command)*BDSP_ARM_MAX_FW_TASK_PER_DSP;

    /* Mapping comand send through TZIOC cmd err = BDSP_Arm_P_InsertEntry_MapTable(pDevice->memHandle, &(pDevice->sDeviceMapTable[0]), pAddr, pDevice->memInfo.cmdQueueParams.uiMsgQueueSize,BDSP_ARM_AF_P_Map_eDram,BDSP_ARM_MAX_ALLOC_DEVICE);*/
    if (BERR_SUCCESS != err)
    {
        BDBG_ERR(("BDSP_Arm_P_AllocateInitMemory: Error in updating the MAP Table for Command Queue"));
        err = BERR_TRACE(err);
        goto err_updating_cmd_queue_map_table;
    }

    /* Allocate Generic Response queue */
	err = BDSP_MMA_P_AllocateAlignedMemory(pDevice->memHandle,
						(BDSP_ARM_MAX_MSGS_PER_QUEUE * sizeof(BDSP_Arm_P_Response)),
						&(pDevice->memInfo.genRspQueueParams.Queue),
						BDSP_MMA_Alignment_32bit);
	if (BERR_SUCCESS != err)
	{
		BDBG_ERR(("BDSP_Arm_P_AllocateInitMemory: Unable to Allocate memory for gen resp queue!"));
		err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
		goto err_alloc_GenRespQueue;
	}
    pDevice->memInfo.genRspQueueParams.uiMsgQueueSize = BDSP_ARM_MAX_MSGS_PER_QUEUE * sizeof(BDSP_Arm_P_Response);

    /* Mapping comand send through TZIOC cmd err = BDSP_Arm_P_InsertEntry_MapTable(pDevice->memHandle, &(pDevice->sDeviceMapTable[0]), pAddr, pDevice->memInfo.genRspQueueParams.uiMsgQueueSize,BDSP_ARM_AF_P_Map_eDram,BDSP_ARM_MAX_ALLOC_DEVICE);*/
    if (BERR_SUCCESS != err)
    {
        BDBG_ERR(("BDSP_Arm_P_AllocateInitMemory: Error in updating the MAP Table for Generic Response Queue"));
        err = BERR_TRACE(err);
        goto err_updating_resp_queue_map_table;
    }

    /* Allocate memory for the MAP Table */
	err = BDSP_MMA_P_AllocateAlignedMemory(pDevice->memHandle,
						sizeof(BDSP_MAP_Table),
						&(pDevice->memInfo.sMapTable.Buffer),
						BDSP_MMA_Alignment_32bit);
	if(BERR_SUCCESS != err)
	{
		BDBG_ERR(("BDSP_Arm_P_AllocateInitMemory: Unable to Allocate memory for MAP Table!!"));
		err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
		goto err_alloc_MapTable;
	}
	pDevice->memInfo.sMapTable.ui32Size  = sizeof(BDSP_MAP_Table);
    err = BDSP_Arm_P_InsertEntry_MapTable(&(pDevice->sDeviceMapTable[0]), &pDevice->memInfo.sMapTable.Buffer, sizeof(BDSP_MAP_Table),BDSP_ARM_AF_P_Map_eDram, BDSP_ARM_MAX_ALLOC_DEVICE);
    if (BERR_SUCCESS != err)
    {
        BDBG_ERR(("BDSP_Arm_P_AllocateInitMemory: Error in updating the MAP Table for MAP Table Store Address"));
        err = BERR_TRACE(err);
        goto err_updating_maptableaddr_map_table;
    }

    /* Allocate memory for the Hbc Info */
	err = BDSP_MMA_P_AllocateAlignedMemory(pDevice->memHandle,
						sizeof(BDSP_Arm_P_HbcInfo),
						&(pDevice->HbcInfo.Memory),
						BDSP_MMA_Alignment_32bit);
	if(BERR_SUCCESS != err)
	{
		BDBG_ERR(("BDSP_Arm_P_AllocateInitMemory: Unable to Allocate memory for BDSP_Arm_P_HbcInfo!!"));
		err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
		goto err_alloc_hbcinfo;
	}
	pDevice->HbcInfo.psHbcInfo = (BDSP_Arm_P_HbcInfo *)pDevice->HbcInfo.Memory.pAddr;
	pDevice->HbcInfo.psHbcInfo->hbcValid = 0;
	pDevice->HbcInfo.psHbcInfo->hbc = 0;
	BDSP_MMA_P_FlushCache(pDevice->HbcInfo.Memory, sizeof(BDSP_Arm_P_HbcInfo));
    goto alloc_init_mem_sucess;

err_alloc_hbcinfo:
err_updating_maptableaddr_map_table:
	BDSP_MMA_P_FreeMemory(&pDevice->memInfo.sMapTable.Buffer);
err_alloc_MapTable:
	BDSP_Arm_P_DeleteEntry_MapTable(&(pDevice->sDeviceMapTable[0]), &pDevice->memInfo.genRspQueueParams.Queue, BDSP_ARM_MAX_ALLOC_DEVICE);
err_updating_resp_queue_map_table:
	BDSP_MMA_P_FreeMemory(&pDevice->memInfo.genRspQueueParams.Queue);
err_alloc_GenRespQueue:
	BDSP_Arm_P_DeleteEntry_MapTable(&(pDevice->sDeviceMapTable[0]), &pDevice->memInfo.cmdQueueParams.Queue, BDSP_ARM_MAX_ALLOC_DEVICE);
err_updating_cmd_queue_map_table:
	BDSP_MMA_P_FreeMemory(&pDevice->memInfo.cmdQueueParams.Queue);
err_alloc_CmdQueue:
err_updating_maptable_cmd_q_hndl:
	BDSP_MMA_P_FreeMemory(&pDevice->sArmInterfaceQ.Memory);
err_alloc_QueueHndl:
err_calcMemPoolReq:
    BDSP_Arm_P_FreeFwExec(pDeviceHandle);

alloc_init_mem_sucess:

    BDBG_LEAVE(BDSP_Arm_P_AllocateInitMemory);
    return err;
}

BERR_Code BDSP_Arm_P_FreeInitMemory(
    void *pDeviceHandle
    )
{
    BERR_Code err = BERR_SUCCESS;
    BDSP_Arm *pDevice = (BDSP_Arm *)pDeviceHandle;

    BDBG_ENTER(BDSP_Arm_P_FreeInitMemory);

    /* Nothing complex needs to be done to release the FW images, just destroy the FW heap and free the memory */
    BDSP_Arm_P_FreeFwExec(pDeviceHandle);

	BDSP_MMA_P_FreeMemory(&pDevice->sArmInterfaceQ.Memory);

	/*BDSP_Arm_P_DeleteEntry_MapTable(pDevice->memHandle, &(pDevice->sDeviceMapTable[0]), pDevice->memInfo.cmdQueueParams.pBaseAddr, BDSP_ARM_MAX_ALLOC_DEVICE);*/
	BDSP_MMA_P_FreeMemory(&pDevice->memInfo.cmdQueueParams.Queue);

	BDSP_MMA_P_FreeMemory(&pDevice->memInfo.genRspQueueParams.Queue);

    err = BDSP_Arm_P_DeleteEntry_MapTable(&(pDevice->sDeviceMapTable[0]), &pDevice->memInfo.sMapTable.Buffer, BDSP_ARM_MAX_ALLOC_DEVICE);
    if(err != BERR_SUCCESS)
    {
        BDBG_ERR(("Unable to Delete the Entry of Map Table from the Device Memory Map List...Anyways going ahead and deleting it"));
    }

	/*BDSP_Arm_P_DeleteEntry_MapTable(pDevice->memHandle, &(pDevice->sDeviceMapTable[0]), pDevice->memInfo.genRspQueueParams.pBaseAddr, BDSP_ARM_MAX_ALLOC_DEVICE);*/
	BDSP_MMA_P_FreeMemory(&pDevice->memInfo.sMapTable.Buffer);

	BDSP_MMA_P_FreeMemory(&pDevice->HbcInfo.Memory);

    BDBG_LEAVE(BDSP_Arm_P_FreeInitMemory);
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

BERR_Code BDSP_Arm_MM_P_CalcScratchAndISbufferReq(
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

    BDBG_ENTER(BDSP_Arm_MM_P_CalcScratchAndISbufferReq);

    /* For Decoders */
    for ( Algoindex=0; Algoindex < BDSP_Algorithm_eMax; Algoindex++ )
    {
        const BDSP_Arm_P_AlgorithmInfo *pInfo;

        pInfo = BDSP_Arm_P_LookupAlgorithmInfo(Algoindex);
        if ( !pInfo->supported )
        {
            continue;
        }

        ui32TempScratch = 0;
        ui32TempIs = 0;
        ui32TempIsIf = 0;

        for( Nodeindex=0; Nodeindex < pInfo->algoExecInfo.NumNodes; Nodeindex++ )
        {
            BDSP_ARM_AF_P_AlgoId algoId = pInfo->algoExecInfo.eAlgoIds[Nodeindex];
            if ( algoId < BDSP_ARM_AF_P_AlgoId_eMax )
            {
                ui32TempScratch += BDSP_ARM_sNodeInfo[algoId].ui32ScratchBuffSize;

                if(ui32TempIs < BDSP_ARM_sNodeInfo[algoId].ui32MaxSizePerChan)
                    ui32TempIs = BDSP_ARM_sNodeInfo[algoId].ui32MaxSizePerChan;
                if(ui32NumCh < BDSP_ARM_sNodeInfo[algoId].ui32MaxNumChansSupported)
                    ui32NumCh = BDSP_ARM_sNodeInfo[algoId].ui32MaxNumChansSupported;

                ui32TempIsIf += BDSP_ARM_sNodeInfo[algoId].ui32InterStgGenericBuffSize;

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

    BDBG_LEAVE(BDSP_Arm_MM_P_CalcScratchAndISbufferReq);

    return err;
}

BERR_Code BDSP_Arm_MM_P_CalcandAllocScratchISbufferReq(void *pDeviceHandle)
{
    BERR_Code err = BERR_SUCCESS;
    uint32_t ui32TempScratch = 0, ui32TempIs = 0, ui32TempIsIf = 0;
    uint32_t ui32BaseAddr = 0;
    uint32_t ui32NumCh=0, k = 0;
    int32_t i32Index = 0;
    BDSP_Arm *pDevice = (BDSP_Arm *)pDeviceHandle;

    BDBG_ENTER(BDSP_Arm_MM_P_CalcandAllocScratchISbufferReq);

    BDBG_OBJECT_ASSERT(pDevice, BDSP_Arm);

    BDSP_Arm_MM_P_CalcScratchAndISbufferReq(&ui32TempScratch, &ui32TempIs, &ui32TempIsIf, &ui32NumCh);

    /* Allocate Scratch Memory */
	if (ui32TempScratch)
	{
		err = BDSP_MMA_P_AllocateAlignedMemory(pDevice->memHandle,
			ui32TempScratch,
			&(pDevice->memInfo.sScratchandISBuff.DspScratchMemGrant.Buffer),
			BDSP_MMA_Alignment_32bit);
		if(BERR_SUCCESS != err)
		{
			BDBG_ERR(("BDSP_Arm_MM_P_CalcandAllocScratchISbufferReq: Unable to Allocate memory for Scratch Buffer!"));
			err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
			goto error_alloc;
		}
		err = BDSP_Arm_P_InsertEntry_MapTable(&(pDevice->sDeviceMapTable[0]),&pDevice->memInfo.sScratchandISBuff.DspScratchMemGrant.Buffer, ui32TempScratch, BDSP_ARM_AF_P_Map_eDram, BDSP_ARM_MAX_ALLOC_DEVICE);
		if (BERR_SUCCESS != err)
		{
			BDBG_ERR(("BDSP_Arm_MM_P_CalcandAllocScratchISbufferReq: Error in updating the MAP Table for Scratch Buffer"));
			err = BERR_TRACE(err);
			goto error_alloc;
		}
	}
	pDevice->memInfo.sScratchandISBuff.DspScratchMemGrant.ui32Size = ui32TempScratch;

    /* Allocate InterStage Memory */
    for(i32Index=0; i32Index < BDSP_ARM_MAX_BRANCH; i32Index++)
    {
        /* Allocate IO Memory */
		err = BDSP_MMA_P_AllocateAlignedMemory(pDevice->memHandle,
					ui32TempIs*ui32NumCh,
					&(pDevice->memInfo.sScratchandISBuff.InterStageIOBuff[i32Index].Buffer),
					BDSP_MMA_Alignment_32bit);
		if(BERR_SUCCESS != err)
		{
			BDBG_ERR(("BDSP_Arm_MM_P_CalcandAllocScratchISbufferReq: Unable to Allocate memory for IO Buffer of InterStage!"));
			err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
			goto error_alloc;
		}

		err = BDSP_Arm_P_InsertEntry_MapTable(&(pDevice->sDeviceMapTable[0]), &pDevice->memInfo.sScratchandISBuff.InterStageIOBuff[i32Index].Buffer, (ui32TempIs*ui32NumCh), BDSP_ARM_AF_P_Map_eDram, BDSP_ARM_MAX_ALLOC_DEVICE);
		if (BERR_SUCCESS != err)
		{
			BDBG_ERR(("BDSP_Arm_MM_P_CalcandAllocScratchISbufferReq: Error in updating the MAP Table for InterStage IO buffer"));
			err = BERR_TRACE(err);
			goto error_alloc;
		}

        for(k=0; k<ui32NumCh; k++)
        {
            ui32BaseAddr = pDevice->memInfo.sScratchandISBuff.InterStageIOBuff[i32Index].Buffer.offset + (k*ui32TempIs);
            pDevice->memInfo.sScratchandISBuff.InterStageIOBuff[i32Index].IOBuff.sCircBuffer[k].ui32BaseAddr = ui32BaseAddr;
            pDevice->memInfo.sScratchandISBuff.InterStageIOBuff[i32Index].IOBuff.sCircBuffer[k].ui32ReadAddr = ui32BaseAddr;
            pDevice->memInfo.sScratchandISBuff.InterStageIOBuff[i32Index].IOBuff.sCircBuffer[k].ui32WriteAddr= ui32BaseAddr;
            pDevice->memInfo.sScratchandISBuff.InterStageIOBuff[i32Index].IOBuff.sCircBuffer[k].ui32EndAddr  = ui32BaseAddr + ui32TempIs;
            pDevice->memInfo.sScratchandISBuff.InterStageIOBuff[i32Index].IOBuff.sCircBuffer[k].ui32WrapAddr = ui32BaseAddr + ui32TempIs;
        }
        pDevice->memInfo.sScratchandISBuff.InterStageIOBuff[i32Index].IOBuff.ui32NumBuffers = ui32NumCh;/*worst case, maxchannels*/
        pDevice->memInfo.sScratchandISBuff.InterStageIOBuff[i32Index].IOBuff.eBufferType    = BDSP_AF_P_BufferType_eDRAM_IS;

        /* Allocate IO Generic Memory and also the configuration structure*/
		err = BDSP_MMA_P_AllocateAlignedMemory(pDevice->memHandle,
							ui32TempIsIf,
							&(pDevice->memInfo.sScratchandISBuff.InterStageIOGenericBuff[i32Index].Buffer),
							BDSP_MMA_Alignment_32bit);
        if(BERR_SUCCESS != err)
        {
            BDBG_ERR(("BDSP_Arm_MM_P_CalcandAllocScratchISbufferReq: Unable to Allocate memory for IO Generic!"));
            err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
            /*Free scratch and IO buffer here*/
            goto error_alloc;
        }
        err = BDSP_Arm_P_InsertEntry_MapTable(&(pDevice->sDeviceMapTable[0]), &pDevice->memInfo.sScratchandISBuff.InterStageIOGenericBuff[i32Index].Buffer, ui32TempIsIf, BDSP_ARM_AF_P_Map_eDram, BDSP_ARM_MAX_ALLOC_DEVICE);
        if (BERR_SUCCESS != err)
        {
            BDBG_ERR(("BDSP_Arm_MM_P_CalcandAllocScratchISbufferReq: Error in updating the MAP Table for InterStage IO Gen buffer"));
            err = BERR_TRACE(err);
            goto error_alloc;
        }

        ui32BaseAddr = pDevice->memInfo.sScratchandISBuff.InterStageIOGenericBuff[i32Index].Buffer.offset;
        pDevice->memInfo.sScratchandISBuff.InterStageIOGenericBuff[i32Index].IOGenericBuff.sCircBuffer.ui32BaseAddr = ui32BaseAddr;
        pDevice->memInfo.sScratchandISBuff.InterStageIOGenericBuff[i32Index].IOGenericBuff.sCircBuffer.ui32ReadAddr = ui32BaseAddr;
        pDevice->memInfo.sScratchandISBuff.InterStageIOGenericBuff[i32Index].IOGenericBuff.sCircBuffer.ui32WriteAddr= ui32BaseAddr;
        pDevice->memInfo.sScratchandISBuff.InterStageIOGenericBuff[i32Index].IOGenericBuff.sCircBuffer.ui32EndAddr  = ui32BaseAddr + ui32TempIsIf;
        pDevice->memInfo.sScratchandISBuff.InterStageIOGenericBuff[i32Index].IOGenericBuff.sCircBuffer.ui32WrapAddr = ui32BaseAddr + ui32TempIsIf;

        pDevice->memInfo.sScratchandISBuff.InterStageIOGenericBuff[i32Index].IOGenericBuff.ui32NumBuffers = 1;
        pDevice->memInfo.sScratchandISBuff.InterStageIOGenericBuff[i32Index].IOGenericBuff.eBufferType    = BDSP_AF_P_BufferType_eDRAM_IS;

    }
    goto end;
error_alloc:
    BDSP_Arm_P_FreeScratchISbuffer(pDeviceHandle);
end:
    BDBG_LEAVE(BDSP_Arm_MM_P_CalcandAllocScratchISbufferReq);
    return err;
}

void BDSP_Arm_P_FreeScratchISbuffer(
            void *pDeviceHandle
            )
{
    BDSP_Arm *pDevice = (BDSP_Arm *)pDeviceHandle;
    int32_t j = 0;

    BDBG_OBJECT_ASSERT(pDevice, BDSP_Arm);

    BDBG_ENTER(BDSP_Arm_P_FreeScratchISbuffer);
	if (pDevice->memInfo.sScratchandISBuff.DspScratchMemGrant.ui32Size)
	{
		if(pDevice->memInfo.sScratchandISBuff.DspScratchMemGrant.Buffer.pAddr)
		{
			BDSP_Arm_P_DeleteEntry_MapTable(&(pDevice->sDeviceMapTable[0]),&pDevice->memInfo.sScratchandISBuff.DspScratchMemGrant.Buffer,BDSP_ARM_MAX_ALLOC_DEVICE);
			BDSP_MMA_P_FreeMemory(&pDevice->memInfo.sScratchandISBuff.DspScratchMemGrant.Buffer);
		}
	}

    for(j = 0; j<BDSP_ARM_MAX_BRANCH; j++)
    {
        if(pDevice->memInfo.sScratchandISBuff.InterStageIOBuff[j].Buffer.pAddr)
        {
            BDSP_Arm_P_DeleteEntry_MapTable(&(pDevice->sDeviceMapTable[0]), &pDevice->memInfo.sScratchandISBuff.InterStageIOBuff[j].Buffer,BDSP_ARM_MAX_ALLOC_DEVICE);
            BDSP_MMA_P_FreeMemory(&pDevice->memInfo.sScratchandISBuff.InterStageIOBuff[j].Buffer);
        }

        if(pDevice->memInfo.sScratchandISBuff.InterStageIOGenericBuff[j].Buffer.pAddr)
        {
            BDSP_Arm_P_DeleteEntry_MapTable(&(pDevice->sDeviceMapTable[0]), &pDevice->memInfo.sScratchandISBuff.InterStageIOGenericBuff[j].Buffer,BDSP_ARM_MAX_ALLOC_DEVICE);
            BDSP_MMA_P_FreeMemory(&pDevice->memInfo.sScratchandISBuff.InterStageIOGenericBuff[j].Buffer);
        }
    }
    BDBG_LEAVE(BDSP_Arm_P_FreeScratchISbuffer);
}

BERR_Code BDSP_Arm_MM_P_CalcStageMemPoolReq(void *pStageHandle)
{
    BERR_Code err = BERR_SUCCESS;
    BDSP_ARM_AF_P_AlgoId algoId;

    uint32_t ui32AlgoIf = 0, ui32AlgoCfgBuf = 0, ui32AlgoStatusBuf = 0;
    uint32_t ui32FsIf = 0, ui32FsCfgBuf = 0, ui32FsStatusBuf = 0;
    uint32_t ui32TempIf = 0, ui32TempCfgBuf = 0, ui32TempStatusBuf =0;

    unsigned int i = 0, j = 0;

    BDSP_ArmStage   *pArmStage = (BDSP_ArmStage *)pStageHandle;
    BDSP_ArmContext *pArmContext = pArmStage->pContext;
    BDBG_OBJECT_ASSERT(pArmStage, BDSP_ArmStage);
    BDBG_OBJECT_ASSERT(pArmContext, BDSP_ArmContext);

    BDBG_ENTER(BDSP_Arm_MM_P_CalcStageMemPoolReq);

    BDBG_ASSERT(NULL != pArmStage);

    /* For Decoders */
    for ( i=0; i < BDSP_Algorithm_eMax; i++ )
    {
        const BDSP_Arm_P_AlgorithmInfo *pInfo;

        pInfo = BDSP_Arm_P_LookupAlgorithmInfo(i);
        if ( !pInfo->supported )
        {
            continue;
        }

        if(!pArmStage->settings.algorithmSupported[i])
        {
            continue;
        }

        ui32TempIf = 0;
        ui32TempCfgBuf = 0;
        ui32TempStatusBuf =0;

        /* Framesync buffer requirement is computed separately */
        algoId = pInfo->algoExecInfo.eAlgoIds[0];
        if ( algoId < BDSP_ARM_AF_P_AlgoId_eMax )
        {
            if (ui32FsIf < BDSP_ARM_sNodeInfo[algoId].ui32InterFrmBuffSize)
            {
                ui32FsIf = BDSP_ARM_sNodeInfo[algoId].ui32InterFrmBuffSize;
            }
            if (ui32FsCfgBuf < BDSP_ARM_sNodeInfo[algoId].ui32UserCfgBuffSize)
            {
                ui32FsCfgBuf = BDSP_ARM_sNodeInfo[algoId].ui32UserCfgBuffSize;
            }
            if (ui32FsStatusBuf < BDSP_ARM_sNodeInfo[algoId].ui32StatusBuffSize)
            {
                ui32FsStatusBuf = BDSP_ARM_sNodeInfo[algoId].ui32StatusBuffSize;
            }
        }

        if(pArmContext->settings.contextType == BDSP_ContextType_eAudio)
        {
            /* Buffer requirement for the rest of the nodes */
            for( j=1; j < pInfo->algoExecInfo.NumNodes; j++ )
            {
                algoId = pInfo->algoExecInfo.eAlgoIds[j];
                if ( algoId < BDSP_ARM_AF_P_AlgoId_eMax )
                {
                    ui32TempIf += BDSP_ARM_sNodeInfo[algoId].ui32InterFrmBuffSize;
                    ui32TempCfgBuf += BDSP_ARM_sNodeInfo[algoId].ui32UserCfgBuffSize;
                    ui32TempStatusBuf += BDSP_ARM_sNodeInfo[algoId].ui32StatusBuffSize;
                }
            }
        } else
        {
            BDBG_ERR(("TRYING TO ALLOCATE MEMORY FOR NON_AUDIO ALGORITHM"));
            return BERR_TRACE(BERR_INVALID_PARAMETER);
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

    pArmStage->stageMemInfo.ui32InterframeBufReqd = ui32AlgoIf + ui32FsIf;
    pArmStage->stageMemInfo.ui32UserConfigReqd = ui32AlgoCfgBuf + ui32FsCfgBuf;
    pArmStage->stageMemInfo.ui32StatusBufReqd = ui32AlgoStatusBuf + ui32FsStatusBuf;

    pArmStage->sFrameSyncOffset.ui32IfOffset = ui32AlgoIf;
    pArmStage->sFrameSyncOffset.ui32UserCfgOffset = ui32AlgoCfgBuf;
    pArmStage->sFrameSyncOffset.ui32StatusOffset = ui32AlgoStatusBuf;

    BDBG_MSG(("ui32AlgoIf = %d ui32AlgoCfgBuf =%d ui32AlgoStatusBuf =%d",ui32AlgoIf,ui32AlgoCfgBuf, ui32AlgoStatusBuf));
    BDBG_MSG(("ui32FsIf = %d ui32FsCfgBuf =%d ui32FsStatusBuf =%d",ui32FsIf,ui32FsCfgBuf, ui32FsStatusBuf));

    BDBG_LEAVE(BDSP_Arm_MM_P_CalcStageMemPoolReq);
    return err;
}

BERR_Code BDSP_Arm_P_AllocateContextMemory(
    void *pContextHandle
    )
{
    BERR_Code err = BERR_SUCCESS;
    BDBG_ENTER(BDSP_Arm_P_AllocateContextMemory);

    BSTD_UNUSED(pContextHandle);
    BDBG_MSG(("BDSP_Arm_P_AllocateContextMemory: Right now no memory needs to be allocated for ARM under a CONTEXT"));

    BDBG_LEAVE(BDSP_Arm_P_AllocateContextMemory);
    return err;
}

BERR_Code BDSP_Arm_P_FreeContextMemory(
    void *pContextHandle
    )
{
    BERR_Code err = BERR_SUCCESS;
    BDBG_ENTER(BDSP_Arm_P_FreeContextMemory);

    BSTD_UNUSED(pContextHandle);
    BDBG_MSG(("BDSP_Arm_P_FreeContextMemory: Right now no memory needs to be freed for ARM under a CONTEXT"));

    BDBG_LEAVE(BDSP_Arm_P_FreeContextMemory);
    return err;
}

BERR_Code BDSP_Arm_P_AllocateTaskMemory(
    void *pTaskHandle,
    const BDSP_TaskCreateSettings *pSettings
    )
{
    BERR_Code err = BERR_SUCCESS;
    uint32_t ui32UsedSize = 0;
    void *pBaseAddr = 0;
    BDSP_ArmTask   *pArmTask = (BDSP_ArmTask   *)pTaskHandle;
    BDSP_ArmContext   *pArmContext = (BDSP_ArmContext   *)pArmTask->pContext;
    BDSP_Arm *pDevice = (BDSP_Arm *)pArmContext->pDevice;

    BDBG_ENTER(BDSP_Arm_P_AllocateTaskMemory);

    BDBG_OBJECT_ASSERT(pArmTask, BDSP_ArmTask);

    ui32UsedSize =  BDSP_CIT_P_TASK_PORT_CONFIG_MEM_SIZE + /*BDSP_CIT_P_TASK_SPDIF_USER_CFG_MEM_SIZE*/
                    BDSP_CIT_P_TASK_FMM_GATE_OPEN_CONFIG +
                    BDSP_CIT_P_TASK_HW_FW_CONFIG +
                    BDSP_CIT_P_TASK_FS_MAPPING_LUT_SIZE +
                    BDSP_CIT_P_TASK_STC_TRIG_CONFIG_SIZE;

	err = BDSP_MMA_P_AllocateAlignedMemory(pDevice->memHandle,
					ui32UsedSize,
					&(pArmTask->taskMemGrants.sTaskCfgBufInfo.Buffer),
					BDSP_MMA_Alignment_32bit);
	if(BERR_SUCCESS != err)
	{
		BDBG_ERR(("BDSP_Arm_P_AllocateTaskMemory: Unable to Allocate memory for TaskCfgBuf!"));
		err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
		goto end;
	}
	pArmTask->taskMemGrants.sTaskCfgBufInfo.ui32Size = ui32UsedSize;

	err = BDSP_Arm_P_InsertEntry_MapTable(&(pArmTask->sTaskMapTable[0]), &pArmTask->taskMemGrants.sTaskCfgBufInfo.Buffer, ui32UsedSize, BDSP_ARM_AF_P_Map_eDram, BDSP_ARM_MAX_ALLOC_TASK);
	if (BERR_SUCCESS != err)
	{
		BDBG_ERR(("BDSP_Arm_P_AllocateTaskMemory: Error in updating the MAP Table for Task config Buffer"));
		err = BERR_TRACE(err);
		goto end;
	}

	/* Sync Queue */
	err = BDSP_MMA_P_AllocateAlignedMemory(pDevice->memHandle,
					(BDSP_ARM_MAX_MSGS_PER_QUEUE * sizeof(BDSP_Arm_P_Response)),
					&(pArmTask->taskMemGrants.sTaskQueue.sTaskSyncQueue.Queue),
					BDSP_MMA_Alignment_32bit);
	if(BERR_SUCCESS != err)
	{
		BDBG_ERR(("BDSP_Arm_P_AllocateTaskMemory: Unable to Allocate memory for sync task queue!"));
		err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
		goto end;
	}
	pArmTask->taskMemGrants.sTaskQueue.sTaskSyncQueue.uiMsgQueueSize = BDSP_ARM_MAX_MSGS_PER_QUEUE * sizeof(BDSP_Arm_P_Response);
	err = BDSP_Arm_P_InsertEntry_MapTable(&(pArmTask->sTaskMapTable[0]), &pArmTask->taskMemGrants.sTaskQueue.sTaskSyncQueue.Queue, (BDSP_ARM_MAX_MSGS_PER_QUEUE * sizeof(BDSP_Arm_P_Response)), BDSP_ARM_AF_P_Map_eDram, BDSP_ARM_MAX_ALLOC_TASK);
	if (BERR_SUCCESS != err)
	{
		BDBG_ERR(("BDSP_Arm_P_AllocateTaskMemory: Error in updating the MAP Table for Sync Queue"));
		err = BERR_TRACE(err);
		goto end;
	}

    /* Async Queue */
	err = BDSP_MMA_P_AllocateAlignedMemory(pDevice->memHandle,
					(BDSP_ARM_MAX_ASYNC_MSGS_PER_QUEUE * sizeof(BDSP_Arm_P_AsynEventMsg)),
					&(pArmTask->taskMemGrants.sTaskQueue.sTaskAsyncQueue.Queue),
					BDSP_MMA_Alignment_32bit);
    if(BERR_SUCCESS != err)
    {
        BDBG_ERR(("BDSP_Arm_P_AllocateTaskMemory: Unable to Allocate memory for async task queue!"));
        err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
        goto end;
    }
    pArmTask->taskMemGrants.sTaskQueue.sTaskAsyncQueue.uiMsgQueueSize = BDSP_ARM_MAX_ASYNC_MSGS_PER_QUEUE * sizeof(BDSP_Arm_P_AsynEventMsg);
    err = BDSP_Arm_P_InsertEntry_MapTable(&(pArmTask->sTaskMapTable[0]), &pArmTask->taskMemGrants.sTaskQueue.sTaskAsyncQueue.Queue,(BDSP_ARM_MAX_ASYNC_MSGS_PER_QUEUE * sizeof(BDSP_Arm_P_AsynEventMsg)), BDSP_ARM_AF_P_Map_eDram, BDSP_ARM_MAX_ALLOC_TASK);
    if (BERR_SUCCESS != err)
    {
        BDBG_ERR(("BDSP_Arm_P_AllocateTaskMemory: Error in updating the MAP Table for Async Queue"));
        err = BERR_TRACE(err);
        goto end;
    }

    /* Async MSG Buf Memory */
    pBaseAddr = BKNI_Malloc((BDSP_ARM_MAX_ASYNC_MSGS_PER_QUEUE * sizeof(BDSP_Arm_P_AsynEventMsg)));
    if(NULL == pBaseAddr)
    {
        BDBG_ERR(("BDSP_Arm_P_AllocateTaskMemory: Unable to Allocate memory for async msgs!"));
        err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
        goto end;
    }
    pArmTask->taskMemGrants.sTaskQueue.sAsyncMsgBufmem.pAddr = pBaseAddr;
    pArmTask->taskMemGrants.sTaskQueue.sAsyncMsgBufmem.ui32Size = BDSP_ARM_MAX_ASYNC_MSGS_PER_QUEUE * sizeof(BDSP_Arm_P_AsynEventMsg);

    /* Task stack swap memory */
	err = BDSP_MMA_P_AllocateAlignedMemory(pDevice->memHandle,
					BDSP_CIT_P_TASK_SWAP_BUFFER_SIZE_INBYTES,
					&(pArmTask->taskMemGrants.sStackSwapBuff.Buffer),
					BDSP_MMA_Alignment_32bit);
    if(BERR_SUCCESS != err)
    {
        BDBG_ERR(("BDSP_Arm_P_AllocateTaskMemory: Unable to Allocate memory for task stack swap!"));
        err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
        goto end;
    }
    pArmTask->taskMemGrants.sStackSwapBuff.ui32Size = BDSP_CIT_P_TASK_SWAP_BUFFER_SIZE_INBYTES;
    err = BDSP_Arm_P_InsertEntry_MapTable(&(pArmTask->sTaskMapTable[0]), &pArmTask->taskMemGrants.sStackSwapBuff.Buffer, BDSP_CIT_P_TASK_SWAP_BUFFER_SIZE_INBYTES, BDSP_ARM_AF_P_Map_eDram, BDSP_ARM_MAX_ALLOC_TASK);
    if (BERR_SUCCESS != err)
    {
        BDBG_ERR(("BDSP_Arm_P_AllocateTaskMemory: Error in updating the MAP Table for Stack Swap Buffer"));
        err = BERR_TRACE(err);
        goto end;
    }

    /*Gate Open Buffer configuration memory */
	err = BDSP_MMA_P_AllocateAlignedMemory(pDevice->memHandle,
					(BDSP_AF_P_MAX_OP_FORKS * SIZEOF(BDSP_AF_P_sIO_BUFFER)),
					&(pArmTask->taskMemGrants.sTaskGateOpenBufInfo.Buffer),
					BDSP_MMA_Alignment_32bit);
    if(BERR_SUCCESS != err)
    {
        BDBG_ERR(("BDSP_Arm_P_AllocateTaskMemory: Unable to Allocate memory for task Gate Open Configuration!"));
        err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
        goto end;
    }
    pArmTask->taskMemGrants.sTaskGateOpenBufInfo.ui32Size = (BDSP_AF_P_MAX_OP_FORKS * SIZEOF(BDSP_AF_P_sIO_BUFFER));
    err = BDSP_Arm_P_InsertEntry_MapTable(&(pArmTask->sTaskMapTable[0]), &pArmTask->taskMemGrants.sTaskGateOpenBufInfo.Buffer, (BDSP_AF_P_MAX_OP_FORKS * SIZEOF(BDSP_AF_P_sIO_BUFFER)), BDSP_ARM_AF_P_Map_eDram, BDSP_ARM_MAX_ALLOC_TASK);
    if (BERR_SUCCESS != err)
    {
        BDBG_ERR(("BDSP_Arm_P_AllocateTaskMemory: Error in updating the MAP Table for task Gate Open Configuration!"));
        err = BERR_TRACE(err);
        goto end;
    }

    if((BDSP_ContextType_eVideo == pArmContext->settings.contextType)||
       (BDSP_ContextType_eVideoEncode == pArmContext->settings.contextType)||
       (BDSP_ContextType_eScm== pArmContext->settings.contextType))
    {

        BDBG_ERR(("BDSP_Arm_P_AllocateTaskMemory: Rite now memory is allocated for Audio Tasks on ARM"));
        err = BERR_TRACE(BERR_INVALID_PARAMETER);
        goto end;
    }
    else
    {

		/* Cit memory. This is now context dependent as the sizes changes for different context */
		err = BDSP_MMA_P_AllocateAlignedMemory(pDevice->memHandle,
						sizeof(BDSP_ARM_AF_P_sTASK_CONFIG),
						&(pArmTask->taskMemGrants.sCitStruct.Buffer),
						BDSP_MMA_Alignment_32bit);
		if(BERR_SUCCESS != err)
		{
			BDBG_ERR(("BDSP_Arm_P_AllocateTaskMemory: Unable to Allocate memory for cit!"));
			err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
			goto end;
		}
		pArmTask->taskMemGrants.sCitStruct.ui32Size = sizeof(BDSP_AF_P_sTASK_CONFIG);
		err = BDSP_Arm_P_InsertEntry_MapTable(&(pArmTask->sTaskMapTable[0]), &pArmTask->taskMemGrants.sCitStruct.Buffer, sizeof(BDSP_AF_P_sTASK_CONFIG), BDSP_ARM_AF_P_Map_eDram, BDSP_ARM_MAX_ALLOC_TASK);
		if (BERR_SUCCESS != err)
		{
			BDBG_ERR(("BDSP_Arm_P_AllocateTaskMemory: Error in updating the MAP Table for CIT Buffer"));
			err = BERR_TRACE(err);
			goto end;
		}

		/* Allocate memory for the spare cit structure used as working buffer by basemodules */
		err = BDSP_MMA_P_AllocateAlignedMemory(pDevice->memHandle,
						sizeof(BDSP_ARM_AF_P_sTASK_CONFIG),
						&(pArmTask->taskMemGrants.sSpareCitStruct.Buffer),
						BDSP_MMA_Alignment_32bit);
		if(BERR_SUCCESS != err)
		{
			BDBG_ERR(("BDSP_Arm_P_AllocateTaskMemory: Unable to Allocate memory for cit!"));
			err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
			goto end;
		}
		pArmTask->taskMemGrants.sSpareCitStruct.ui32Size = sizeof(BDSP_AF_P_sTASK_CONFIG);
		err = BDSP_Arm_P_InsertEntry_MapTable(&(pArmTask->sTaskMapTable[0]), &pArmTask->taskMemGrants.sSpareCitStruct.Buffer, sizeof(BDSP_AF_P_sTASK_CONFIG), BDSP_ARM_AF_P_Map_eDram, BDSP_ARM_MAX_ALLOC_TASK);
		if (BERR_SUCCESS != err)
		{
			BDBG_ERR(("BDSP_Arm_P_AllocateTaskMemory: Error in updating the MAP Table for SPARE CIT Buffer"));
			err = BERR_TRACE(err);
			goto end;
		}
    }

	/* Start Task memory */
	err = BDSP_MMA_P_AllocateAlignedMemory(pDevice->memHandle,
						sizeof(BDSP_P_TaskParamInfo),
						&(pArmTask->taskMemGrants.sTaskInfo.Buffer),
						BDSP_MMA_Alignment_32bit);
	if(BERR_SUCCESS != err)
	{
		BDBG_ERR(("BDSP_Arm_P_AllocateTaskMemory: Unable to Allocate memory for start task params!"));
		err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
		goto end;
	}
	pArmTask->taskMemGrants.sTaskInfo.ui32Size = sizeof(BDSP_P_TaskParamInfo);
	err = BDSP_Arm_P_InsertEntry_MapTable(&(pArmTask->sTaskMapTable[0]), &pArmTask->taskMemGrants.sTaskInfo.Buffer, sizeof(BDSP_P_TaskParamInfo), BDSP_ARM_AF_P_Map_eDram, BDSP_ARM_MAX_ALLOC_TASK);
	if (BERR_SUCCESS != err)
	{
		BDBG_ERR(("BDSP_Arm_P_AllocateTaskMemory: Error in updating the MAP Table for Task Info"));
		err = BERR_TRACE(err);
		goto end;
	}

	/*Create the Feedback Buffer required for the master task*/
	if(true == pSettings->masterTask)
	{
		err = BDSP_MMA_P_AllocateAlignedMemory(pDevice->memHandle,
							(BDSP_ARM_AF_P_INTERTASK_FEEDBACK_BUFFER_SIZE + 4),
							&(pArmTask->FeedbackBuffer),
							BDSP_MMA_Alignment_32bit);

		if (BERR_SUCCESS != err)
		{
			BDBG_ERR(("BDSP_Arm_P_AllocateTaskMemory: Unable to Allocate memory for Feedback  Buffer !"));
			err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
			goto end;
		}
		BKNI_Memset((void *)pArmTask->FeedbackBuffer.pAddr, 0x0, (BDSP_ARM_AF_P_INTERTASK_FEEDBACK_BUFFER_SIZE + 4));
		BDSP_MMA_P_FlushCache(pArmTask->FeedbackBuffer, (BDSP_ARM_AF_P_INTERTASK_FEEDBACK_BUFFER_SIZE + 4));
		err = BDSP_Arm_P_InsertEntry_MapTable(&(pArmTask->sTaskMapTable[0]), &pArmTask->FeedbackBuffer, (BDSP_ARM_AF_P_INTERTASK_FEEDBACK_BUFFER_SIZE + 4), BDSP_ARM_AF_P_Map_eDram, BDSP_ARM_MAX_ALLOC_TASK);
		if (BERR_SUCCESS != err)
		{
			BDBG_ERR(("BDSP_Arm_P_AllocateTaskMemory: Error in updating the MAP Table for Master Task Feedback buffer"));
			err = BERR_TRACE(err);
			goto end;
		}
	}

    /* Create the Queues required for the task */
    err = BDSP_Arm_P_CreateTaskQueues( pTaskHandle);
    if (BERR_SUCCESS != err)
    {
        BDBG_ERR(("BDSP_Arm_P_AllocateTaskMemory: Unable to create queues for the tasks"));
        err = BERR_TRACE(err);
    }

end:
    if(err != BERR_SUCCESS)
    {
        BDSP_Arm_P_FreeTaskMemory(pTaskHandle);
    }
    BDBG_LEAVE(BDSP_Arm_P_AllocateTaskMemory);
    return err;
}


BERR_Code BDSP_Arm_P_FreeTaskMemory(
    void *pTaskHandle
    )
{
    BERR_Code err = BERR_SUCCESS;
    BDSP_ArmTask   *pArmTask = (BDSP_ArmTask   *)pTaskHandle;

    BDBG_ENTER(BDSP_Arm_P_FreeTaskMemory);

    BDBG_OBJECT_ASSERT(pArmTask, BDSP_ArmTask);

    err = BDSP_Arm_P_DestroyTaskQueues(pTaskHandle);
    if (BERR_SUCCESS != err)
    {
        BDBG_ERR(("BDSP_Arm_P_FreeTaskMemory: Queue destroy failed for the Task"));
        err = BERR_TRACE(err);
    }

	if (pArmTask->settings.masterTask == BDSP_TaskSchedulingMode_eMaster)
	{
		if ((pArmTask->FeedbackBuffer.pAddr !=NULL))
		{
			BDSP_Arm_P_DeleteEntry_MapTable(&(pArmTask->sTaskMapTable[0]), &pArmTask->FeedbackBuffer, BDSP_ARM_MAX_ALLOC_TASK);
			BDSP_MMA_P_FreeMemory(&pArmTask->FeedbackBuffer);
		}
	}

    BDSP_Arm_P_DeleteEntry_MapTable(&(pArmTask->sTaskMapTable[0]), &pArmTask->taskMemGrants.sTaskQueue.sTaskSyncQueue.Queue, BDSP_ARM_MAX_ALLOC_TASK);
    BDSP_MMA_P_FreeMemory(&pArmTask->taskMemGrants.sTaskQueue.sTaskSyncQueue.Queue);

    BDSP_Arm_P_DeleteEntry_MapTable(&(pArmTask->sTaskMapTable[0]), &pArmTask->taskMemGrants.sTaskQueue.sTaskAsyncQueue.Queue, BDSP_ARM_MAX_ALLOC_TASK);
    BDSP_MMA_P_FreeMemory(&pArmTask->taskMemGrants.sTaskQueue.sTaskAsyncQueue.Queue);

    if ( pArmTask->taskMemGrants.sTaskQueue.sAsyncMsgBufmem.pAddr )
    {
        BKNI_Free(pArmTask->taskMemGrants.sTaskQueue.sAsyncMsgBufmem.pAddr);
        pArmTask->taskMemGrants.sTaskQueue.sAsyncMsgBufmem.pAddr= 0;
    }

    BDSP_Arm_P_DeleteEntry_MapTable(&(pArmTask->sTaskMapTable[0]), &pArmTask->taskMemGrants.sTaskGateOpenBufInfo.Buffer, BDSP_ARM_MAX_ALLOC_TASK);
    BDSP_MMA_P_FreeMemory(&pArmTask->taskMemGrants.sTaskGateOpenBufInfo.Buffer);

    BDSP_Arm_P_DeleteEntry_MapTable(&(pArmTask->sTaskMapTable[0]), &pArmTask->taskMemGrants.sStackSwapBuff.Buffer, BDSP_ARM_MAX_ALLOC_TASK);
    BDSP_MMA_P_FreeMemory(&pArmTask->taskMemGrants.sStackSwapBuff.Buffer);

    BDSP_Arm_P_DeleteEntry_MapTable(&(pArmTask->sTaskMapTable[0]),&pArmTask->taskMemGrants.sTaskCfgBufInfo.Buffer, BDSP_ARM_MAX_ALLOC_TASK);
    BDSP_MMA_P_FreeMemory(&pArmTask->taskMemGrants.sTaskCfgBufInfo.Buffer);

    BDSP_Arm_P_DeleteEntry_MapTable(&(pArmTask->sTaskMapTable[0]), &pArmTask->taskMemGrants.sTaskInfo.Buffer, BDSP_ARM_MAX_ALLOC_TASK);
    BDSP_MMA_P_FreeMemory(&pArmTask->taskMemGrants.sTaskInfo.Buffer);

    BDSP_Arm_P_DeleteEntry_MapTable(&(pArmTask->sTaskMapTable[0]), &pArmTask->taskMemGrants.sCitStruct.Buffer, BDSP_ARM_MAX_ALLOC_TASK);
    BDSP_MMA_P_FreeMemory(&pArmTask->taskMemGrants.sCitStruct.Buffer);
    if(pArmTask->taskMemGrants.sSpareCitStruct.Buffer.pAddr!= NULL )
    {
        BDSP_Arm_P_DeleteEntry_MapTable(&(pArmTask->sTaskMapTable[0]), &pArmTask->taskMemGrants.sSpareCitStruct.Buffer, BDSP_ARM_MAX_ALLOC_TASK);
        BDSP_MMA_P_FreeMemory(&pArmTask->taskMemGrants.sSpareCitStruct.Buffer);
    }

    BDBG_LEAVE(BDSP_Arm_P_FreeTaskMemory);
    return err;
}

BERR_Code BDSP_Arm_P_AllocateStageMemory(
    void *pStageHandle
    )
{
    BERR_Code err = BERR_SUCCESS;
	BDSP_MMA_Memory Buffer;
	uint8_t *pAddr;
	uint32_t Offset, i;

    BDSP_ArmStage *pArmStage = (BDSP_ArmStage *)pStageHandle;
    BDSP_ArmContext *pArmContext = (BDSP_ArmContext *)pArmStage->pContext;

    BDSP_Arm *pDevice = (BDSP_Arm *)pArmContext->pDevice;
    unsigned bytesreqd=0;

    BDBG_ENTER(BDSP_Arm_P_AllocateStageMemory);

    err = BDSP_Arm_MM_P_CalcStageMemPoolReq ((void *)pArmStage);
    if(err != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Arm_P_AllocateStageMemory: Unable to get memory requirements for ARM DSP!"));
        err = BERR_TRACE(err);
        goto end;
    }

    /*Interframe allocation for a stage*/
	pArmStage->sDramInterFrameBuffer.ui32Size = 0;;
	if( pArmStage->stageMemInfo.ui32InterframeBufReqd )
	{
		err = BDSP_MMA_P_AllocateAlignedMemory(pDevice->memHandle,
							pArmStage->stageMemInfo.ui32InterframeBufReqd,
							&(pArmStage->sDramInterFrameBuffer.Buffer),
							BDSP_MMA_Alignment_32bit);
		if(BERR_SUCCESS != err)
		{
			BDBG_ERR(("BDSP_Arm_P_AllocateStageMemory: Unable to Allocate memory for IFrameCfgBuf!"));
			err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
			goto end;
		}
		err = BDSP_Arm_P_InsertEntry_MapTable(&(pArmStage->sStageMapTable[0]), &pArmStage->sDramInterFrameBuffer.Buffer, pArmStage->stageMemInfo.ui32InterframeBufReqd, BDSP_ARM_AF_P_Map_eDram, BDSP_ARM_MAX_ALLOC_STAGE);
		if (BERR_SUCCESS != err)
		{
			BDBG_ERR(("BDSP_Arm_P_AllocateStageMemory: Error in updating the MAP Table for InterFrame Buffer"));
			err = BERR_TRACE(err);
			goto end;
		}
		pArmStage->sDramInterFrameBuffer.ui32Size = pArmStage->stageMemInfo.ui32InterframeBufReqd;
	}

	pArmStage->sDramUserConfigBuffer.ui32Size = 0;
	if( pArmStage->stageMemInfo.ui32UserConfigReqd )
	{
		err = BDSP_MMA_P_AllocateAlignedMemory(pDevice->memHandle,
							pArmStage->stageMemInfo.ui32UserConfigReqd,
							&(pArmStage->sDramUserConfigBuffer.Buffer),
							BDSP_MMA_Alignment_32bit);
		if(BERR_SUCCESS != err)
		{
			BDBG_ERR(("BDSP_Arm_P_AllocateStageMemory: Unable to Allocate memory for Userconfig!"));
			err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
			goto end;
		}
		err = BDSP_Arm_P_InsertEntry_MapTable(&(pArmStage->sStageMapTable[0]), &pArmStage->sDramUserConfigBuffer.Buffer, pArmStage->stageMemInfo.ui32UserConfigReqd, BDSP_ARM_AF_P_Map_eDram, BDSP_ARM_MAX_ALLOC_STAGE);
		if (BERR_SUCCESS != err)
		{
			BDBG_ERR(("BDSP_Arm_P_AllocateStageMemory: Error in updating the MAP Table for User Config Buffer"));
			err = BERR_TRACE(err);
			goto end;
		}
		pArmStage->sDramUserConfigBuffer.ui32Size = pArmStage->stageMemInfo.ui32UserConfigReqd;
	}

	/* Allocate memory for the host buffer to hold the modified settings when the task is running */
	pArmStage->sDramUserConfigSpareBuffer.ui32Size = 0;
	if(pArmStage->stageMemInfo.ui32UserConfigReqd)
	{
		err = BDSP_MMA_P_AllocateAlignedMemory(pDevice->memHandle,
							pArmStage->stageMemInfo.ui32UserConfigReqd,
							&(pArmStage->sDramUserConfigSpareBuffer.Buffer),
							BDSP_MMA_Alignment_32bit);
		if(BERR_SUCCESS != err)
		{
			BDBG_ERR(("BDSP_Arm_P_AllocateStageMemory: Unable to Allocate memory for Userconfig!"));
			err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
			goto end;
		}
		err = BDSP_Arm_P_InsertEntry_MapTable(&(pArmStage->sStageMapTable[0]),&pArmStage->sDramUserConfigSpareBuffer.Buffer, pArmStage->stageMemInfo.ui32UserConfigReqd, BDSP_ARM_AF_P_Map_eDram, BDSP_ARM_MAX_ALLOC_STAGE);
		if (BERR_SUCCESS != err)
		{
			BDBG_ERR(("BDSP_Arm_P_AllocateStageMemory: Error in updating the MAP Table for Spare User Config Buffer"));
			err = BERR_TRACE(err);
			goto end;
		}
		pArmStage->sDramUserConfigSpareBuffer.ui32Size = pArmStage->stageMemInfo.ui32UserConfigReqd;
	}

	/*Status Buffer allocation for a stage*/
	pArmStage->sDramStatusBuffer.ui32Size = 0;
	if(pArmStage->stageMemInfo.ui32StatusBufReqd)
	{
		err = BDSP_MMA_P_AllocateAlignedMemory(pDevice->memHandle,
							pArmStage->stageMemInfo.ui32StatusBufReqd,
							&(pArmStage->sDramStatusBuffer.Buffer),
							BDSP_MMA_Alignment_32bit);
		if(BERR_SUCCESS != err)
		{
			BDBG_ERR(("BDSP_Arm_P_AllocateStageMemory: Unable to Allocate memory for Status Buffer!"));
			err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
			goto end;
		}
		err = BDSP_Arm_P_InsertEntry_MapTable(&(pArmStage->sStageMapTable[0]), &pArmStage->sDramStatusBuffer.Buffer, pArmStage->stageMemInfo.ui32StatusBufReqd, BDSP_ARM_AF_P_Map_eDram, BDSP_ARM_MAX_ALLOC_STAGE);
		if (BERR_SUCCESS != err)
		{
			BDBG_ERR(("BDSP_Arm_P_AllocateStageMemory: Error in updating the MAP Table for Status Buffer"));
			err = BERR_TRACE(err);
			goto end;
		}
		pArmStage->sDramStatusBuffer.ui32Size = pArmStage->stageMemInfo.ui32StatusBufReqd;
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
		BDBG_ERR(("BDSP_Arm_P_AllocateStageMemory: Unable to Allocate memory for IFrameCfgBuf!"));
		err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
		goto end;
	}

	err = BDSP_Arm_P_InsertEntry_MapTable(&(pArmStage->sStageMapTable[0]), &Buffer, bytesreqd, BDSP_ARM_AF_P_Map_eDram, BDSP_ARM_MAX_ALLOC_STAGE);
	if (BERR_SUCCESS != err)
	{
		BDBG_ERR(("BDSP_Arm_P_AllocateStageMemory: Error in updating the MAP Table for Stage and IDS IO Buffer and IO config buffer for input and output"));
		err = BERR_TRACE(err);
		goto end;
	}

	pAddr = (uint8_t *)Buffer.pAddr;
	Offset = (uint32_t)Buffer.offset;

	/*Split the memory for all the input and output descriptors in vitual memory itself as their contents needs to be populated later.*/
	for(i=0; i<BDSP_AF_P_MAX_IP_FORKS; i++)
	{
		pArmStage->sStageInput[i].IoBuffDesc.hBlock = Buffer.hBlock;
		pArmStage->sStageInput[i].IoBuffDesc.pAddr  = (void *)pAddr;
		pArmStage->sStageInput[i].IoBuffDesc.offset = Offset;
		pAddr = pAddr + SIZEOF(BDSP_AF_P_sIO_BUFFER);
		Offset = Offset + SIZEOF(BDSP_AF_P_sIO_BUFFER);

		pArmStage->sStageInput[i].IoGenBuffDesc.hBlock = Buffer.hBlock;
		pArmStage->sStageInput[i].IoGenBuffDesc.pAddr  = (void *)pAddr;
		pArmStage->sStageInput[i].IoGenBuffDesc.offset = Offset;
		pAddr = pAddr + SIZEOF(BDSP_AF_P_sIO_GENERIC_BUFFER);
		Offset = Offset + SIZEOF(BDSP_AF_P_sIO_GENERIC_BUFFER);
	}
	for(i=0; i<BDSP_AF_P_MAX_OP_FORKS; i++)
	{
		pArmStage->sStageOutput[i].IoBuffDesc.hBlock = Buffer.hBlock;
		pArmStage->sStageOutput[i].IoBuffDesc.pAddr  = (void *)pAddr;
		pArmStage->sStageOutput[i].IoBuffDesc.offset = Offset;
		pAddr = pAddr + SIZEOF(BDSP_AF_P_sIO_BUFFER);
		Offset = Offset + SIZEOF(BDSP_AF_P_sIO_BUFFER);

		pArmStage->sStageOutput[i].IoGenBuffDesc.hBlock = Buffer.hBlock;
		pArmStage->sStageOutput[i].IoGenBuffDesc.pAddr  = (void *)pAddr;
		pArmStage->sStageOutput[i].IoGenBuffDesc.offset = Offset;
		pAddr = pAddr + SIZEOF(BDSP_AF_P_sIO_GENERIC_BUFFER);
		Offset = Offset + SIZEOF(BDSP_AF_P_sIO_GENERIC_BUFFER);

		/*mark all of them invalid till stages are connected to make them valid*/
		pArmStage->eStageOpBuffDataType[i] = BDSP_AF_P_DistinctOpType_eInvalid;
	}

	pArmStage->sIdsStageOutput.IoBuffDesc.hBlock = Buffer.hBlock;
	pArmStage->sIdsStageOutput.IoBuffDesc.pAddr = (void *)pAddr;
	pArmStage->sIdsStageOutput.IoBuffDesc.offset = Offset;
	pAddr = pAddr + SIZEOF(BDSP_AF_P_sIO_BUFFER);
	Offset = Offset + SIZEOF(BDSP_AF_P_sIO_BUFFER);

	pArmStage->sIdsStageOutput.IoGenBuffDesc.hBlock = Buffer.hBlock;
	pArmStage->sIdsStageOutput.IoGenBuffDesc.pAddr = (void *)pAddr;
	pArmStage->sIdsStageOutput.IoGenBuffDesc.offset = Offset;

	pArmStage->sIdsStageOutput.StageIOBuffDescAddr = pArmStage->sIdsStageOutput.IoBuffDesc.offset;
	pArmStage->sIdsStageOutput.StageIOGenericBuffDescAddr = pArmStage->sIdsStageOutput.IoGenBuffDesc.offset;

end:
    if(err != BERR_SUCCESS)
    {
        BDSP_Arm_P_FreeStageMemory(pStageHandle);
    }
    BDBG_LEAVE(BDSP_Arm_P_AllocateStageMemory);
    return err;
}


BERR_Code BDSP_Arm_P_FreeStageMemory(
    void *pStageHandle
    )
{
    BERR_Code err = BERR_SUCCESS;
    BDSP_ArmStage *pArmStage = (BDSP_ArmStage *)pStageHandle;

    BDBG_ENTER(BDSP_Arm_P_FreeStageMemory);

	/* Free the interframe buffer allocation */
	if(pArmStage->sDramInterFrameBuffer.Buffer.pAddr)
	{
		BDSP_Arm_P_DeleteEntry_MapTable(&(pArmStage->sStageMapTable[0]), &pArmStage->sDramInterFrameBuffer.Buffer, BDSP_ARM_MAX_ALLOC_STAGE);
		BDSP_MMA_P_FreeMemory(&pArmStage->sDramInterFrameBuffer.Buffer);
	}

	/* Free the user settings buffer allocation */
	if(pArmStage->sDramUserConfigBuffer.Buffer.pAddr)
	{
		BDSP_Arm_P_DeleteEntry_MapTable(&(pArmStage->sStageMapTable[0]), &pArmStage->sDramUserConfigBuffer.Buffer, BDSP_ARM_MAX_ALLOC_STAGE);
		BDSP_MMA_P_FreeMemory(&pArmStage->sDramUserConfigBuffer.Buffer);
	}

	/* Free the BDSP working buffer of user settings */
	if(pArmStage->sDramUserConfigSpareBuffer.Buffer.pAddr)
	{
		BDSP_Arm_P_DeleteEntry_MapTable(&(pArmStage->sStageMapTable[0]), &pArmStage->sDramUserConfigSpareBuffer.Buffer, BDSP_ARM_MAX_ALLOC_STAGE);
		BDSP_MMA_P_FreeMemory(&pArmStage->sDramUserConfigSpareBuffer.Buffer);
	}

	/* Free the status buffer allocation */
	if(pArmStage->sDramStatusBuffer.Buffer.pAddr)
	{
		BDSP_Arm_P_DeleteEntry_MapTable(&(pArmStage->sStageMapTable[0]), &pArmStage->sDramStatusBuffer.Buffer, BDSP_ARM_MAX_ALLOC_STAGE);
		BDSP_MMA_P_FreeMemory(&pArmStage->sDramStatusBuffer.Buffer);
	}

	/* Free the descriptor buffer allocation */
	BDSP_Arm_P_DeleteEntry_MapTable(&(pArmStage->sStageMapTable[0]), &pArmStage->sStageInput[0].IoBuffDesc, BDSP_ARM_MAX_ALLOC_STAGE);
	BDSP_MMA_P_FreeMemory(&pArmStage->sStageInput[0].IoBuffDesc);
    BDBG_LEAVE(BDSP_Arm_P_FreeStageMemory);
    return err;
}
