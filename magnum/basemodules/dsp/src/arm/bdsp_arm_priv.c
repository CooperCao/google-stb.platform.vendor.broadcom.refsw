/******************************************************************************
 *  Copyright (C) 2018 Broadcom.
 *  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to
 *  the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied),
 *  right to use, or waiver of any kind with respect to the Software, and
 *  Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 *  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 *  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization,
 *  constitutes the valuable trade secrets of Broadcom, and you shall use all
 *  reasonable efforts to protect the confidentiality thereof, and to use this
 *  information only in connection with your use of Broadcom integrated circuit
 *  products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 *  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 *  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 *  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 *  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 *  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 *  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 *  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/

#include <fcntl.h> /*SR_TBD: Remove this include before release */

#include "bdsp_arm_priv_include.h"
#include "bchp_sun_top_ctrl.h"
#include "bchp_aud_fmm_bf_ctrl.h"

#ifdef BDSP_QUEUE_DEBUG
#include "../raaga/bdsp_raaga_priv.h"
#endif /* BDSP_QUEUE_DEBUG */


BDBG_MODULE(bdsp_arm_priv);

BDBG_OBJECT_ID(BDSP_ArmCapture);


BERR_Code   BDSP_Arm_P_CreateTaskQueues(void *pTaskHandle)
{
    BERR_Code   err = BERR_SUCCESS;
    BDSP_ArmTask *pArmTask = (BDSP_ArmTask *)pTaskHandle;
    BDSP_ArmContext *pArmContext = (BDSP_ArmContext *)pArmTask->pContext;
    BDSP_Arm *pDevice = (BDSP_Arm *)pArmContext->pDevice;

    BDBG_ENTER(BDSP_Arm_P_CreateTaskQueues);

    /* Creating SYNC Queue */
    err = BDSP_Arm_P_CreateMsgQueue(pDevice,&(pArmTask->taskMemGrants.sTaskQueue.sTaskSyncQueue),&(pArmTask->hSyncMsgQueue));
    if (BERR_SUCCESS != err)
    {
        BDBG_ERR(("BDSP_Arm_P_CreateTaskQueues: Error in creating Sync Queue"));
        err = BERR_TRACE(err);
        goto err_sync_queue;
    }

    /* Creating ASYNC Queue */
    err = BDSP_Arm_P_CreateMsgQueue(pDevice,&(pArmTask->taskMemGrants.sTaskQueue.sTaskAsyncQueue),&(pArmTask->hAsyncMsgQueue));
    if (BERR_SUCCESS != err)
    {
        BDBG_ERR(("BDSP_Arm_P_CreateTaskQueues: Error in creating ASync Queue"));
        err = BERR_TRACE(err);
        goto err_async_queue;
    }
    goto end;

err_async_queue:
    BDSP_Arm_P_DestroyMsgQueue(pDevice,pArmTask->hSyncMsgQueue);
err_sync_queue:
end:
    BDBG_LEAVE(BDSP_Arm_P_CreateTaskQueues);
    return err;
}

BERR_Code   BDSP_Arm_P_DestroyTaskQueues(void *pTaskHandle)
{
    BERR_Code   err = BERR_SUCCESS;
    BDSP_ArmTask *pArmTask = (BDSP_ArmTask *)pTaskHandle;
    BDSP_ArmContext *pArmContext = (BDSP_ArmContext *)pArmTask->pContext;
    BDSP_Arm *pDevice = (BDSP_Arm *)pArmContext->pDevice;

    BDBG_ENTER(BDSP_Arm_P_DestroyTaskQueues);

    /* Destroy SYNC Queue */
    err = BDSP_Arm_P_DestroyMsgQueue(pDevice,pArmTask->hSyncMsgQueue);
    if (BERR_SUCCESS != err)
    {
        BDBG_ERR(("BDSP_Arm_P_DestroyTaskQueues: Error in destroying Sync Queue"));
        err = BERR_TRACE(err);
    }

    /* Destroy ASYNC Queue */
    err = BDSP_Arm_P_DestroyMsgQueue(pDevice,pArmTask->hAsyncMsgQueue);
    if (BERR_SUCCESS != err)
    {
        BDBG_ERR(("BDSP_Arm_P_DestroyTaskQueues: Error in destroying ASync Queue"));
        err = BERR_TRACE(err);
    }

    BDBG_LEAVE(BDSP_Arm_P_DestroyTaskQueues);
    return err;
}

void BDSP_Arm_P_Validate_Open_settings(void *pDeviceHandle)
{
    BDSP_Arm *pDevice = (BDSP_Arm *)pDeviceHandle;

    BDBG_ENTER(BDSP_Arm_P_Validate_Open_settings);
    BSTD_UNUSED(pDevice);
#if 0

    unsigned i=0;

    BDBG_OBJECT_ASSERT(pDevice, BDSP_Arm);

    for(i=0;i<BDSP_AlgorithmType_eMax ; i++ )
    {
        if (pDevice->settings.maxAlgorithms[i] > BDSP_ARM_MAX_DWNLD_BUFS )
        {
            BDBG_ERR((" Please make sure the max Algorithms per algo, in Arm open settings, is less than %d",BDSP_ARM_MAX_DWNLD_BUFS ));
            BDBG_ERR((" Try increasing the define -BDSP_ARM_MAX_DWNLD_BUFS,in bdsp_arm_mm_priv.h, to continue testing"));
            BDBG_ASSERT(0);
        }
    }
    /*All Post process are always downloaded so hardcode it to 1 */
    pDevice->settings.maxAlgorithms[BDSP_AlgorithmType_eAudioProcessing] = 1;
#endif
    BDBG_LEAVE(BDSP_Arm_P_Validate_Open_settings);

}

uint32_t BDSP_Arm_P_CheckSum(void *ImgStart,uint32_t ui32ChunkLen)
{
    uint32_t i,sum=0;
    uint8_t *data = ImgStart;
    for(i=0;i<ui32ChunkLen;i++)
    {
        sum += data[i];
    }
    return sum;
}

BERR_Code BDSP_Arm_P_PreloadFwToAstra(BTEE_ClientHandle hClient,	BDSP_Arm *pDevice,	BDSP_ARM_AF_P_AlgoId AlgoId)
{
	BERR_Code ret = BERR_SUCCESS;

	BTEE_FileHandle pFile;
	uint32_t BytesWritten;

	BDSP_ArmImgCacheEntry *pimgCache = &pDevice->imgCache[BDSP_ARM_IMG_ID_CODE(AlgoId)];

	/* Open file in Astra for writing system elf file */
	ret = BTEE_File_Open(hClient,BDSP_ARM_AlgoFileName[AlgoId],(O_WRONLY|O_CREAT),&pFile);
    if (ret)
	{
        BDBG_ERR(("failed to open file in astra for algo %d",AlgoId));
        return BERR_TRACE(ret);
    }

	/* Write the system elf file */
	ret = BTEE_File_Write(pFile,pimgCache->offset,pimgCache->size,&BytesWritten);
    if (ret)
	{
        BDBG_ERR(("failed writing to file in astra for algo %d",AlgoId));
        return BERR_TRACE(ret);
    }

	/*Check if the write was correct */
	if(BytesWritten != pimgCache->size)
    {
      BDBG_ERR(("FW algo (Id =%#x) not downloaded properly. Bytes written is %d, expected was %d",AlgoId,(unsigned int)BytesWritten, (unsigned int)pimgCache->size));
    }

	BTEE_File_Close(pFile);

    return ret;
}


BERR_Code BDSP_Arm_P_DownloadFwToAstra(BTEE_ClientHandle hClient,	BDSP_Arm *pDevice,	BDSP_ARM_AF_P_AlgoId AlgoId)
{
	BERR_Code ret = BERR_SUCCESS;

	BTEE_FileHandle pFile;
	uint32_t BytesWritten;

	BDSP_ArmImgCacheEntry *pimgCache = &pDevice->imgCache[BDSP_ARM_IMG_ID_CODE(AlgoId)];

	/* Open file in Astra for writing system elf file */
	ret = BTEE_File_Open(hClient,BDSP_ARM_AlgoFileName[AlgoId],(O_WRONLY|O_CREAT),&pFile);
    if (ret)
	{
        BDBG_ERR(("failed to open file in astra for algo %d",AlgoId));
        return BERR_TRACE(ret);
    }

	/* Write the system elf file */
	ret = BTEE_File_Write(pFile,pimgCache->offset,pimgCache->size,&BytesWritten);
    if (ret)
	{
        BDBG_ERR(("failed writing to file in astra for algo %d",AlgoId));
        return BERR_TRACE(ret);
    }

	/*Check if the write was correct */
	if(BytesWritten != pimgCache->size)
    {
      BDBG_ERR(("FW algo (Id =%#x) not downloaded properly. Bytes written is %d, expected was %d",AlgoId, (unsigned int) BytesWritten,(unsigned int)pimgCache->size));
    }

	BTEE_File_Close(pFile);

    return ret;
}


BERR_Code BDSP_Arm_P_DownloadLibDmaCodeToAstra(BTEE_ClientHandle hClient,BDSP_Arm *pDevice, BDSP_Arm_SystemImgId ImgId)
{
    BERR_Code ret = BERR_SUCCESS;

	BTEE_FileHandle pFile;
	uint32_t BytesWritten;
	BDSP_ArmImgCacheEntry *pimgCache = &pDevice->imgCache[ImgId];

	/* Open file in Astra for writing system elf file */
	ret = BTEE_File_Open(hClient,"/lib/libdma.so",(O_WRONLY|O_CREAT),&pFile);
    if (ret)
	{
        BDBG_ERR(("failed to open file in astra for image %d",ImgId));
        return BERR_TRACE(ret);
    }

	/* Write the system elf file */
	ret = BTEE_File_Write(pFile,pimgCache->offset,pimgCache->size,&BytesWritten);
    if (ret)
	{
        BDBG_ERR(("failed writing to file in astra for image %d",ImgId));
        return BERR_TRACE(ret);
    }

	/*Check if the write was correct */
	if(BytesWritten != pimgCache->size)
    {
      BDBG_ERR(("FW Image (Id =%#x) not downloaded properly. Bytes written is %d, expected was %d",ImgId, (unsigned int)BytesWritten,(unsigned int)pimgCache->size));
    }

	BTEE_File_Close(pFile);

    return ret;

}



BERR_Code BDSP_Arm_P_DownloadSystemCodeToAstra(BTEE_ClientHandle hClient,BDSP_Arm *pDevice, BDSP_Arm_SystemImgId ImgId)
{
    BERR_Code ret = BERR_SUCCESS;

    BTEE_FileHandle pFile;
    uint32_t BytesWritten;
    BDSP_ArmImgCacheEntry *pimgCache = &pDevice->imgCache[ImgId];

    /* Open file in Astra for writing system elf file */
    ret = BTEE_File_Open(hClient,"/armdsp_system.elf",(O_WRONLY|O_CREAT),&pFile);
    if (ret)
    {
        BDBG_ERR(("failed to open file in astra for image %d",ImgId));
        return BERR_TRACE(ret);
    }

    /* Write the system elf file */
    ret = BTEE_File_Write(pFile,pimgCache->offset,pimgCache->size,&BytesWritten);
    if (ret)
    {
        BDBG_ERR(("failed writing to file in astra for image %d",ImgId));
        return BERR_TRACE(ret);
    }

    /*Check if the write was correct */
    if(BytesWritten != pimgCache->size)
    {
      BDBG_ERR(("FW Image (Id =%#x) not downloaded properly. Bytes written is %d, expected was %d",ImgId,(unsigned int)BytesWritten, (unsigned int)pimgCache->size));
    }

    BTEE_File_Close(pFile);

    return ret;

}


BERR_Code BDSP_Arm_P_DownloadHbcMonitorToAstra(BTEE_ClientHandle hClient,BDSP_Arm *pDevice, BDSP_Arm_SystemImgId ImgId)
{
    BERR_Code ret = BERR_SUCCESS;

    BTEE_FileHandle pFile;
    uint32_t BytesWritten;
    BDSP_ArmImgCacheEntry *pimgCache = &pDevice->imgCache[ImgId];

    /* Open file in Astra for writing system elf file */
    ret = BTEE_File_Open(hClient,"/hbc_monitor.elf",(O_WRONLY|O_CREAT),&pFile);
    if (ret)
    {
        BDBG_ERR(("failed to open file in astra for image %d",ImgId));
        return BERR_TRACE(ret);
    }

    /* Write the system elf file */
    ret = BTEE_File_Write(pFile,pimgCache->offset,pimgCache->size,&BytesWritten);
    if (ret)
    {
        BDBG_ERR(("failed writing to file in astra for image %d",ImgId));
        return BERR_TRACE(ret);
    }

    /*Check if the write was correct */
    if(BytesWritten != pimgCache->size)
    {
      BDBG_ERR(("FW Image (Id =%#x) not downloaded properly. Bytes written is %d, expected was %d",ImgId, (unsigned int) BytesWritten, (unsigned int)pimgCache->size));
    }

    BTEE_File_Close(pFile);

    return ret;

}

BERR_Code BDSP_Arm_P_StartHbcMonitor(BDSP_Arm *pDevice)
{
    BERR_Code ret = BERR_SUCCESS;
    BTEE_ConnectionSettings sConnectionSettings;

    BDBG_ENTER(BDSP_Arm_P_StartHbcMonitor);

    if (pDevice->deviceWatchdogFlag == false)
    {
        /* Download HBC monitor code to astra */
        ret = BDSP_Arm_P_DownloadHbcMonitorToAstra(pDevice->armDspApp.hClient,pDevice,BDSP_ARM_SystemImgId_eHbcMonitorCode);
        if (BERR_SUCCESS != ret) {
            BDBG_ERR(("failed to download hbc_monitor"));
            goto err_hbc_dwnld;
        }
    }
    /* Start HBC monitor */
    ret = BTEE_Application_Open(
                pDevice->armDspApp.hClient,
                "hbc_monitor",
                "/hbc_monitor.elf",
                &pDevice->armDspApp.hHbcApplication);

    if (BERR_SUCCESS != ret) {
        BDBG_ERR(("failed to start hbc_monitor"));
        goto err_hbc_peer_start;
    }

    /* Create a connection */
    BTEE_Connection_GetDefaultSettings(
                pDevice->armDspApp.hClient,
                &sConnectionSettings    /* [out] */
                );
    ret = BTEE_Connection_Open(
                pDevice->armDspApp.hHbcApplication,
                "hbc_monitor",
                &sConnectionSettings,   /* */
                &pDevice->armDspApp.hHbcConnection /* [out] Connection Handle */
                );
    if (BERR_SUCCESS != ret) {
        BDBG_ERR(("failed to open HBC connection"));
        goto err_open_hbc_connection;
    }

    /* Send HBC params msg */
    {
        /* Create msg */
        BDSP_ArmDspSystemCmd sCmd;
        sCmd.eArmSysMsg = BDSP_ARM_DSP_MSG_HBC_INFO;

        /* Physical address of command and generic responce queue */
        sCmd.uCommand.sHbcParams.hbcValidDramAddr = pDevice->HbcInfo.Memory.offset;
		sCmd.uCommand.sHbcParams.hbcDramAddr = (dramaddr_t)(pDevice->HbcInfo.Memory.offset + (uintptr_t)(&((BDSP_Arm_P_HbcInfo *)NULL)->hbc));

        /*Send msg */
        ret = BTEE_Connection_SendMessage(
            pDevice->armDspApp.hHbcConnection,
            &sCmd,
            sizeof(BDSP_ArmDspSystemCmd)
            );
        if (BERR_SUCCESS != ret) {
            BDBG_ERR(("failed to sending msg"));
            goto err_sending_hbc_info_msg;
        }

    }

    goto start_hbc_success;

err_hbc_dwnld:

err_hbc_peer_start:

err_open_hbc_connection:

err_sending_hbc_info_msg:


start_hbc_success:
    BDBG_LEAVE(BDSP_Arm_P_StartHbcMonitor);

return ret;
}

/***********************************************************************
 Name        :   BDSP_Arm_P_Open

 Type        :   BDSP Internal

 Input       :   pDeviceHandle -Device Handle which needs to be opened/created

 Return      :   Error Code to return SUCCESS or FAILURE

 Functionality   :

***********************************************************************/
BERR_Code BDSP_Arm_P_Open(
     void *pDeviceHandle
     )
{
    BERR_Code ret = BERR_SUCCESS;
    unsigned index = 0;
    BDSP_Arm_P_MsgQueueHandle hMsgQueue;
    BDSP_Arm *pDevice = (BDSP_Arm *)pDeviceHandle;
    BDSP_MAP_Table_Entry MapTable[BDSP_ARM_MAX_ALLOC_DEVICE];
    uint32_t ui32NumEntries = 0;
	uint32_t algoId = 0;

    BTEE_ClientCreateSettings sClientSettings;
    BTEE_ConnectionSettings sConnectionSettings;

    BDBG_ENTER(BDSP_Arm_P_Open);
    BDBG_OBJECT_ASSERT(pDevice, BDSP_Arm);

    BDSP_Arm_P_Validate_Open_settings(pDeviceHandle);
    if (pDevice->deviceWatchdogFlag == false)
    {
        if (pDevice->settings.authenticationEnabled == true)
            pDevice->settings.preloadImages = true;

    ret = BKNI_CreateMutex(&(pDevice->taskDetails.taskIdMutex));
    BDBG_ASSERT(ret == BERR_SUCCESS);

    ret = BKNI_CreateMutex(&(pDevice->armInterfaceQHndlMutex));
    BDBG_ASSERT(ret == BERR_SUCCESS);

    ret = BKNI_CreateMutex(&(pDevice->captureMutex));
    BDBG_ASSERT(ret == BERR_SUCCESS);

    ret = BKNI_CreateMutex(&(pDevice->watchdogMutex));
    BDBG_ASSERT(ret == BERR_SUCCESS);

    for (index=0 ; index< BDSP_ARM_MAX_FW_TASK_PER_DSP; index++)
    {
        pDevice->taskDetails.taskId[index]   = true;
        pDevice->taskDetails.pArmTask[index] = NULL;
    }

    for (index=0;index<BDSP_ARM_NUM_INTERFACE_QUEUE_HANDLE;index++)
    {
        pDevice->armIntrfcQHndlFlag[index] = false;
    }
    /*THIS SHOULD BE DONE ONLY IF RAAGA RDB IS USED BY ARM Add register to mapp table*/
    {
		BDSP_MMA_Memory Addr;
		uint32_t u32Size = (59*(BCHP_RAAGA_DSP_FW_CFG_FIFO_1_BASE_ADDR-BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR));/*59 is the number of fifo available in Raaga*/
		Addr.pAddr = (void *) (BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR+BCHP_PHYSICAL_OFFSET);
		Addr.offset =  (BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR+BCHP_PHYSICAL_OFFSET);
		ret = BDSP_Arm_P_InsertEntry_MapTable(&(pDevice->sDeviceMapTable[0]), &Addr, u32Size, (BDSP_ARM_AF_P_Map_eDevice), BDSP_ARM_MAX_ALLOC_DEVICE);
        if (BERR_SUCCESS != ret)
        {
            BDBG_ERR(("BDSP_ARM_P_Open: Error in updating the MAP Table with Raaga RDB"));
            ret = BERR_TRACE(ret);
            goto err_updating_rdb_map_table;
        }
    }

        ret = BDSP_Arm_P_AllocateInitMemory (pDeviceHandle);
        if (ret != BERR_SUCCESS)
        {
            ret = BERR_TRACE(ret);
            goto err_allocate_initmem;
        }
        ret = BDSP_Arm_P_CalcandAllocScratchISbufferReq(pDevice);/*allocation of =DSP scratch+InterstageIO+IO Generic*/
        if (ret != BERR_SUCCESS)
        {
            ret = BERR_TRACE(ret);
            goto err_allocate_scratchISmem;
        }
    }
    if (!((pDevice->settings.authenticationEnabled == true)
        && (pDevice->deviceWatchdogFlag == true)))
    {
        /* Code Download */
        ret = BDSP_Arm_P_Alloc_DwnldFwExec(pDeviceHandle);
        if (ret != BERR_SUCCESS)
        {
            ret = BERR_TRACE(BDSP_ERR_DOWNLOAD_FAILED);
            goto err_downloadfw;
        }
    }
    if (pDevice->deviceWatchdogFlag == true)
    {
        hMsgQueue =  pDevice->hCmdQueue;
    }

    /*Command Queue*/
    if(pDevice->deviceWatchdogFlag)
    {
        ret = BDSP_Arm_P_InitMsgQueue(pDevice,hMsgQueue);
    }
    else
    {
        /*Command Queue*/
        ret = BDSP_Arm_P_CreateMsgQueue(pDevice, &(pDevice->memInfo.cmdQueueParams),&hMsgQueue);
    }
    if (BERR_SUCCESS != ret)
    {
        BDBG_ERR(("BDSP_ARM_P_Open: Command queue creation failed!"));
        goto err_create_cmdqueue;
    }

    if (pDevice->deviceWatchdogFlag == false)
    {
        pDevice->hCmdQueue = hMsgQueue;
    }

    if (pDevice->deviceWatchdogFlag == true)
    {
        hMsgQueue = pDevice->hGenRspQueue;
    }
    /*Generic response Queue*/
    if(pDevice->deviceWatchdogFlag)
    {
        ret = BDSP_Arm_P_InitMsgQueue(pDevice,hMsgQueue);
    }
    else
    {
        /*Generic response Queue*/
        ret = BDSP_Arm_P_CreateMsgQueue(pDevice, &(pDevice->memInfo.genRspQueueParams), &hMsgQueue);
    }
    if (BERR_SUCCESS != ret)
    {
        BDBG_ERR(("BDSP_ARM_P_Open: Generic Response queue creation failed!"));
        goto err_create_genqueue;
    }
    if (pDevice->deviceWatchdogFlag == false)
    {
        pDevice->hGenRspQueue = hMsgQueue;
    }



    if (pDevice->deviceWatchdogFlag == true)
    {
        pDevice->HbcInfo.psHbcInfo->hbcValid = 2;
        BDSP_MMA_P_FlushCache(pDevice->HbcInfo.Memory ,sizeof(uint32_t));
    }

    if (pDevice->deviceWatchdogFlag == false)
    {
        /* Event created for Device level acknowledgment to be recieved from ARM/Asrtra*/
        ret = BKNI_CreateEvent(&(pDevice->hDeviceEvent));
        if (BERR_SUCCESS != ret)
        {
            BDBG_ERR(("BDSP_Arm_P_Open: Unable to create event"));
            ret = BERR_TRACE(ret);
            goto err_event_create;
        }
    }
    BKNI_ResetEvent(pDevice->hDeviceEvent);


    {
        pDevice->armDspApp.hBteeInstance = pDevice->settings.hBteeInstance;
        BTEE_Client_GetDefaultCreateSettings(pDevice->armDspApp.hBteeInstance,&sClientSettings);
        sClientSettings.pEventCallback_isr = (BTEE_ClientEventCallback)BDSP_Arm_P_AckEventCallback_isr;
        sClientSettings.pCallbackData = pDeviceHandle;

        ret = BTEE_Client_Create(
                pDevice->armDspApp.hBteeInstance,   /* Instance Handle */
                "arm_bdsp",                         /* Client Name */
                &sClientSettings,                          /* Client Settings */
                &pDevice->armDspApp.hClient         /* [out] */
                );
        if(BERR_SUCCESS != ret)
        {
            BDBG_ERR(("failed to open arm_bdsp client"));
            goto err_armdsp_client_open;
        }

        if(pDevice->settings.authenticationEnabled == false)
        {
            ret = BDSP_Arm_P_StartHbcMonitor(pDevice);
            if (BERR_SUCCESS != ret) {
                    BDBG_ERR(("failed to start hbc_monitor"));
                    goto err_hbc_start;
            }

            if (pDevice->deviceWatchdogFlag == false)
            {
				/* Write the downloaded application code into Astra secure memory */
		        ret = BDSP_Arm_P_DownloadLibDmaCodeToAstra(pDevice->armDspApp.hClient,pDevice,BDSP_ARM_SystemImgId_eLibDmaCode);
		        if (BERR_SUCCESS != ret) {
		            BDBG_ERR(("failed to download libdma"));
		            goto err_armdsp_dwnld;
		        }

		        /* Write the downloaded application code into Astra secure memory */
		        ret = BDSP_Arm_P_DownloadSystemCodeToAstra(pDevice->armDspApp.hClient,pDevice,BDSP_ARM_SystemImgId_eSystemCode);
		        if (BERR_SUCCESS != ret) {
		            BDBG_ERR(("failed to start armdsp_system"));
		            goto err_armdsp_dwnld;
		        }

			    for (algoId=0; algoId<BDSP_ARM_AF_P_AlgoId_eMax; algoId++)
			    {
			        if(pDevice->imgCache[BDSP_ARM_IMG_ID_CODE(algoId)].size == 0 ){
						BDBG_ERR(("imageId=%d SIZE=0",algoId));
			            continue;
			        }
					/* BDBG_ERR(("imageId=%d Size=%d File name=%s",algoId,pDevice->imgCache[BDSP_ARM_IMG_ID_CODE(algoId)].size,BDSP_ARM_AlgoFileName[algoId])); */
			        ret = BDSP_Arm_P_PreloadFwToAstra(pDevice->armDspApp.hClient, pDevice,	algoId);
			        if ( ret ){
			            ret = BERR_TRACE(ret);
			            goto err_armdsp_fw_dwnld;
			        }
                }
            }

            ret = BTEE_Application_Open(
                pDevice->armDspApp.hClient,
                "armdsp_system",
                "/armdsp_system.elf",
                &pDevice->armDspApp.hApplication);

            if (BERR_SUCCESS != ret) {
                BDBG_ERR(("failed to start armdsp_system"));
                goto err_peer_start;
            }
            /*SR_TBD: This msg queue connection will not be required once Async and Sync msg que is done through connection */
            /* Create a connection */
            BTEE_Connection_GetDefaultSettings(
                pDevice->armDspApp.hClient,
                &sConnectionSettings    /* [out] */
                );
            ret = BTEE_Connection_Open(
                pDevice->armDspApp.hApplication,
                "armdsp_system",
                &sConnectionSettings,   /* */
                &pDevice->armDspApp.hConnection /* [out] Connection Handle */
                );
            if (BERR_SUCCESS != ret) {
                BDBG_ERR(("failed to open connection"));
                goto err_open_connection;
            }

            /* Send msg with Sync and Async command queue*/
            {
                /* Create msg */
                BDSP_ArmDspSystemCmd sCmd;

                /* Send HBC params msg */
                sCmd.eArmSysMsg = BDSP_ARM_DSP_MSG_HBC_INFO;

                sCmd.uCommand.sHbcParams.hbcValidDramAddr = pDevice->HbcInfo.Memory.offset;
                sCmd.uCommand.sHbcParams.hbcDramAddr = (dramaddr_t)(pDevice->HbcInfo.Memory.offset+ (uintptr_t)(&((BDSP_Arm_P_HbcInfo *)NULL)->hbc));
                /*Send msg */
                ret = BTEE_Connection_SendMessage(
                    pDevice->armDspApp.hConnection,
                    &sCmd,
                    sizeof(BDSP_ArmDspSystemCmd)
                    );
                if (BERR_SUCCESS != ret) {
                    BDBG_ERR(("BDSP_ARM_P_Open: failed to sending HBC info msg"));
                    goto err_sending_msg;
                }
                sCmd.eArmSysMsg = BDSP_ARM_DSP_MSG_INIT_PARAMS;

                /* Physical address of command and generic responce queue */
                sCmd.uCommand.sInitParams.cmdQueueHandlePhyAddr    = ((uintptr_t)pDevice->sArmInterfaceQ.Memory.offset+(sizeof(BDSP_AF_P_sDRAM_CIRCULAR_BUFFER)*pDevice->hCmdQueue->MsgQueueHandleIndex));
                sCmd.uCommand.sInitParams.genRspQueueHandlePhyAddr = ((uintptr_t)pDevice->sArmInterfaceQ.Memory.offset+(sizeof(BDSP_AF_P_sDRAM_CIRCULAR_BUFFER)*pDevice->hGenRspQueue->MsgQueueHandleIndex));
                sCmd.uCommand.sInitParams.QueueHandleArryPhyAddr   = pDevice->sArmInterfaceQ.Memory.offset;
                sCmd.uCommand.sInitParams.ui32NumQueueHandle = BDSP_ARM_NUM_INTERFACE_QUEUE_HANDLE;

                /*Send msg */
                ret = BTEE_Connection_SendMessage(
                    pDevice->armDspApp.hConnection,
                    &sCmd,
                    sizeof(BDSP_ArmDspSystemCmd)
                    );
                if (BERR_SUCCESS != ret) {
                    BDBG_ERR(("BDSP_ARM_P_Open: failed to sending init params msg"));
                    goto err_sending_msg;
                }

            }
            BKNI_Memset(MapTable,0,(BDSP_ARM_MAX_ALLOC_DEVICE*sizeof(BDSP_MAP_Table_Entry)));
            BDSP_Arm_P_RetrieveEntriesToMap(&(pDevice->sDeviceMapTable[0]), &MapTable[0], &ui32NumEntries, BDSP_ARM_MAX_ALLOC_DEVICE);
            ret = BDSP_Arm_P_SendMapCommand(pDeviceHandle, &MapTable[0], ui32NumEntries);
            if (BERR_SUCCESS != ret)
            {
                BDBG_ERR(("BDSP_ARM_P_Open: Send ARM MAP Command failed!!!!"));
                goto err_send_map_cmd;
            }
        }
    }

    goto open_success;

err_send_map_cmd:
err_sending_msg:
	BTEE_Connection_Close(pDevice->armDspApp.hConnection);
err_open_connection:
err_peer_start:
err_armdsp_fw_dwnld:
err_armdsp_dwnld:
err_hbc_start:
err_armdsp_client_open:
    BTEE_Client_Destroy(pDevice->armDspApp.hClient);
err_event_create:
err_create_genqueue:
    BDSP_Arm_P_DestroyMsgQueue(pDevice,pDevice->hCmdQueue);
err_create_cmdqueue:
    BDSP_Arm_P_DestroyMsgQueue(pDevice, pDevice->hGenRspQueue);
err_downloadfw:
    BDSP_Arm_P_FreeScratchISbuffer(pDeviceHandle);
err_allocate_scratchISmem:
    BDSP_Arm_P_FreeInitMemory(pDeviceHandle);

err_allocate_initmem:
err_updating_rdb_map_table:
    if (pDevice->deviceWatchdogFlag == false)
    {
        BKNI_DestroyMutex(pDevice->taskDetails.taskIdMutex);
        BKNI_DestroyMutex(pDevice->captureMutex);
        BKNI_DestroyMutex(pDevice->watchdogMutex);
        BKNI_DestroyMutex(pDevice->armInterfaceQHndlMutex);
    }

open_success:
    BDBG_LEAVE(BDSP_Arm_P_Open);
    return ret;
}

/***********************************************************************
Name            :   BDSP_Arm_P_Close

Type            :   PI Interface

Input           :   pDeviceHandle -Device Handle which needs to be closed.

Return          :   None

Functionality       :

***********************************************************************/

void BDSP_Arm_P_Close(
    void *pDeviceHandle
    )
{
    BDSP_Arm *pDevice = (BDSP_Arm *)pDeviceHandle;
    BDSP_ArmContext *pArmContext;
    BERR_Code err = BERR_SUCCESS;
    BDSP_MAP_Table_Entry MapTable[BDSP_ARM_MAX_ALLOC_DEVICE];
    uint32_t ui32NumEntries = 0;

    BDBG_ENTER(BDSP_Arm_P_Close);
    BDBG_OBJECT_ASSERT(pDevice, BDSP_Arm);

    BKNI_Memset(MapTable,0,(BDSP_ARM_MAX_ALLOC_DEVICE*sizeof(BDSP_MAP_Table_Entry)));
    BDSP_Arm_P_RetrieveEntriesToUnMap(&(pDevice->sDeviceMapTable[0]), &MapTable[0], &ui32NumEntries, BDSP_ARM_MAX_ALLOC_DEVICE);
    err = BDSP_Arm_P_SendUnMapCommand(pDeviceHandle, &MapTable[0], ui32NumEntries);
    if (BERR_SUCCESS != err)
    {
        BDBG_ERR(("BDSP_Arm_P_Close: Send ARM UNMAP Command failed!!!!"));
    }

    err = BDSP_Arm_P_ArmHbcClose(pDevice);
    if (BERR_SUCCESS != err)
    {
      BDBG_ERR(("BDSP_Arm_P_Close: BDSP_Arm_P_ArmHbcClose  failed!!!!"));
    }

    err = BDSP_Arm_P_ArmDspClose(pDevice);
    if (BERR_SUCCESS != err)
    {
      BDBG_ERR(("BDSP_Arm_P_Close: BDSP_Arm_P_ArmDspClose  failed!!!!"));
    }

    /* Destroy any contexts left open */
    while ( (pArmContext = BLST_S_FIRST(&pDevice->contextList)) )
    {
        BDBG_ERR(("BDSP_Arm_P_Close: Forcefully closing the context (%p)!!!!",(void *)pArmContext));
        BDSP_Context_Destroy(&pArmContext->context);
    }

    err = BDSP_Arm_P_DestroyMsgQueue(pDevice, pDevice->hCmdQueue);
    if (BERR_SUCCESS != err)
    {
        BDBG_ERR(("BDSP_Arm_P_Close: CMD queue destroy failed!"));
        err = BERR_TRACE(err);
    }

    err = BDSP_Arm_P_DestroyMsgQueue(pDevice, pDevice->hGenRspQueue);
    if (BERR_SUCCESS != err)
    {
        BDBG_ERR(("BDSP_Arm_P_Close: Generic RSP queue destroy failed!"));
        err = BERR_TRACE(err);
    }

    BDSP_Arm_P_FreeScratchISbuffer(pDeviceHandle);
    BDSP_Arm_P_FreeInitMemory(pDeviceHandle);

    {
		BDSP_MMA_Memory Addr;
		Addr.pAddr = (void *) (BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR+BCHP_PHYSICAL_OFFSET);
		Addr.offset = (BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR+BCHP_PHYSICAL_OFFSET);
        err = BDSP_Arm_P_DeleteEntry_MapTable(&(pDevice->sDeviceMapTable[0]), &Addr, BDSP_ARM_MAX_ALLOC_DEVICE);
        if (BERR_SUCCESS != err)
        {
            BDBG_ERR(("BDSP_ARM_P_Close: Error in removing the MAP Table entries with Raaga RDB"));
        }
    }

    BKNI_DestroyEvent(pDevice->hDeviceEvent);

    BKNI_DestroyMutex(pDevice->taskDetails.taskIdMutex);
    BKNI_DestroyMutex(pDevice->captureMutex);
    BKNI_DestroyMutex(pDevice->armInterfaceQHndlMutex);
    BKNI_DestroyMutex(pDevice->watchdogMutex);

    /* Invalidate and free the device structure */
    BDBG_OBJECT_DESTROY(pDevice, BDSP_Arm);
    BKNI_Free(pDevice);

    BDBG_LEAVE(BDSP_Arm_P_Close);
}

/***********************************************************************
Name        :   BDSP_Arm_P_GetStatus

Type        :   PI Interface

Input       :   pDeviceHandle -Device Handle which needs to be closed.
                pStatus - pointer where the Data will be filled and returned to PI

Return      :   None

Functionality   :   Return the following data to the PI.
    1)  Number of DSPs supported.
    2)  Number of Watchdog Events.
    3)  Details of Firmware version.
***********************************************************************/

void BDSP_Arm_P_GetStatus(
    void *pDeviceHandle,
    BDSP_Status *pStatus
    )
{
    BDSP_Arm *pDevice = (BDSP_Arm *)pDeviceHandle;
    BDBG_OBJECT_ASSERT(pDevice, BDSP_Arm);
    pStatus->numDsp = BDSP_ARM_NUM_DSP;
    pStatus->numWatchdogEvents=0;
    pStatus->firmwareVersion.majorVersion = BDSP_ARM_MAJOR_VERSION;
    pStatus->firmwareVersion.minorVersion = BDSP_ARM_MINOR_VERSION;
    pStatus->firmwareVersion.branchVersion = 0;
    pStatus->firmwareVersion.branchSubVersion = 0;


    return;
}



/***********************************************************************
Name        :   BDSP_Arm_P_GetAlgorithmInfo

Type        :   PI Interface

Input       :   algorithm -The algorithm for the which the data is requested by PI.
                pInfo - Pointer where the specific data releated to algorithm is returned back to the PI.

Return      :   None

Functionality   :   Returns the following data back to the PI.
    1)  Name of the algorithm.
    2)  Whether the algorithm is supported or not.
    3)  To which category the algothm belongs to.
    4)  The size of the User Configuration required by the algorithm.
    5)  The size of the Status Buffer required by the algorithm.
***********************************************************************/

void BDSP_Arm_P_GetAlgorithmInfo_isrsafe(
    BDSP_Algorithm algorithm,
    BDSP_AlgorithmInfo *pInfo /* [out] */
    )
{
    const BDSP_Arm_P_AlgorithmInfo *pAlgoInfo;

    BDBG_ASSERT(pInfo);

    pAlgoInfo = BDSP_Arm_P_LookupAlgorithmInfo(algorithm);
    pInfo->pName = pAlgoInfo->pName;
    pInfo->supported = pAlgoInfo->supported;
    pInfo->type = pAlgoInfo->type;
    pInfo->settingsSize = pAlgoInfo->userConfigSize;
    pInfo->statusSize = pAlgoInfo->statusBufferSize;
}


/***********************************************************************
Name        :   BDSP_P_GetDefaultContextSettings

Type        :   PI Interface

Input       :   pDeviceHandle -Device Handle which needs to be closed.
                contextType - The Type of context to be opened like Audio/Video Decode/Video Encode/SCM
                pSettings - pointer where the Default Data will be filled and returned to PI.

Return      :   None

Functionality   :   Returns the default date for create settings to the PI depending the type of Context to be created.
                Mostly the default data is zeros unless specifically programed.
***********************************************************************/

void BDSP_Arm_P_GetDefaultContextSettings(
    void *pDeviceHandle,
    BDSP_ContextType contextType,
    BDSP_ContextCreateSettings *pSettings
    )
{
    BDSP_Arm *pDevice = (BDSP_Arm *)pDeviceHandle;
    BDBG_OBJECT_ASSERT(pDevice, BDSP_Arm);

    BKNI_Memset(pSettings, 0, sizeof(*pSettings));

    if (contextType == BDSP_ContextType_eAudio)
    {
        pSettings->maxTasks = BDSP_ARM_MAX_FW_TASK_PER_CTXT;
        pSettings->contextType = contextType;
        pSettings->maxBranch = BDSP_ARM_MAX_BRANCH;
        pSettings->maxStagePerBranch = BDSP_ARM_MAX_STAGE_PER_BRANCH;
    }
    else
    {
        BDBG_ERR(("Trying to create a Context other that Audio on ARM which is not supported!!!!!"));
    }
}

/***********************************************************************
Name        :   BDSP_Arm_P_CreateContext

Type        :   PI Interface

Input       :   pDeviceHandle - Device handle for which the context needs to created.
                pSettings - input settings for creating the context.
                pContextHandle -pointer returned to the PI for its use.

Return      :   Error Code to return SUCCESS or FAILURE

Functionality   :   Following are the operations performed.
        1)  Allocate the memory that is required for holding the properties of the context.
        2)  Intialise the Context handle for the context to store the address.
        3)  Intialise all the function pointers which will be used by the PI for further processing.
        4)  Install the interrupts used at the Context level.
        5)  Allocate the Context level memory(VOM table) required for the context.
        6)  Error handling if any errors occurs.
***********************************************************************/
BERR_Code BDSP_Arm_P_CreateContext(
    void *pDeviceHandle,
    const BDSP_ContextCreateSettings *pSettings,
    BDSP_ContextHandle *pContextHandle
    )
{
    BDSP_Arm *pDevice = (BDSP_Arm *)pDeviceHandle;
    BDSP_ArmContext *pArmContext;
    BERR_Code errCode=BERR_SUCCESS;

    BDBG_ENTER(BDSP_Arm_P_CreateContext);
    BDBG_OBJECT_ASSERT(pDevice, BDSP_Arm);

    /* Alloc Arm context */
    pArmContext = BKNI_Malloc(sizeof(BDSP_ArmContext));
    if ( NULL == pArmContext )
    {
        errCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto end;
    }

    BKNI_Memset(pArmContext, 0, sizeof(BDSP_ArmContext));
    BDBG_OBJECT_SET(pArmContext, BDSP_ArmContext);
    BLST_S_INIT(&pArmContext->freeTaskList);
    BKNI_AcquireMutex(pDevice->captureMutex);
    BLST_S_INIT(&pArmContext->allocTaskList);
    BKNI_ReleaseMutex(pDevice->captureMutex);
    pArmContext->pDevice = pDevice;

    /* Init context */
    BDSP_P_InitContext(&pArmContext->context, pArmContext);

    pArmContext->context.destroy = BDSP_Arm_P_DestroyContext;
    pArmContext->context.getDefaultStageCreateSettings = BDSP_Arm_P_GetDefaultStageCreateSettings;
    pArmContext->context.createStage = BDSP_Arm_P_CreateStage;
    pArmContext->context.getDefaultTaskSettings = BDSP_Arm_P_GetDefaultTaskSettings;
    pArmContext->context.createTask = BDSP_Arm_P_CreateTask;


    pArmContext->context.getInterruptHandlers = NULL; /*BDSP_Arm_P_GetInterruptHandlers*/
    pArmContext->context.setInterruptHandlers = NULL; /*BDSP_Arm_P_SetInterruptHandlers*/
    pArmContext->context.processWatchdogInterrupt = BDSP_Arm_P_ProcessWatchdogInterrupt;
    pArmContext->context.createInterTaskBuffer = NULL; /*BDSP_Arm_P_InterTaskBuffer_Create*/
    pArmContext->context.createCapture = BDSP_Arm_P_AudioCaptureCreate;

    /* Support for RDB Queue Addition */
    pArmContext->context.getDefaultQueueSettings = NULL; /*BDSP_Arm_P_GetDefaultCreateQueueSettings;*/
    pArmContext->context.createQueue = NULL; /*BDSP_Arm_P_Queue_Create;*/

    pArmContext->settings = *pSettings;

    /* TODO - check whether anthing needs to be allocated as part of Context memory */
    errCode = BDSP_Arm_P_AllocateContextMemory(pContextHandle);
    if ( errCode != BERR_SUCCESS )
    {
        errCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto err_context_mem;
    }
    /* Success, add to device's context list */
    BKNI_AcquireMutex(pDevice->captureMutex);
    BLST_S_INSERT_HEAD(&pDevice->contextList, pArmContext, node);
    *pContextHandle= &pArmContext->context;
    BKNI_ReleaseMutex(pDevice->captureMutex);
    goto end;

err_context_mem:
    BDSP_Arm_P_FreeContextMemory(pContextHandle);
    /* Invalidate and free object */
    BDBG_OBJECT_DESTROY(pArmContext, BDSP_ArmContext);
    BKNI_Free(pArmContext);

end:
    BDBG_LEAVE(BDSP_Arm_P_CreateContext);
    return errCode;
}

/***********************************************************************
Name        :   BDSP_Arm_P_DestroyContext

Type        :   PI Interface

Input       :   pContextHandle - Context handle which needs to be closed.

Return      :   None

Functionality   :   Following are the operations performed.
        1)  Free the memory that was allocated for holding the Context level memoryfor the context.
        2)  Free any that was accidently left open under the Context.
        3)  Un-Install the interrupts used at the Context level.
        4)  Free the Context memory that was allocated.
***********************************************************************/
void BDSP_Arm_P_DestroyContext(
    void *pContextHandle
    )
{
    BDSP_ArmContext *pArmContext = (BDSP_ArmContext *)pContextHandle;
    BDSP_Arm *pDevice;

    BDBG_ENTER(BDSP_Arm_P_DestroyContext);
    BDBG_OBJECT_ASSERT(pArmContext, BDSP_ArmContext);

    pDevice = pArmContext->pDevice;

    /* TODO - check whether anthing needs to be de-allocated as part of Context memory */
    BDSP_Arm_P_FreeContextMemory(pContextHandle);

    /* TODO - CDN */
    /* Remove the Task registered for this context if any */

    /* Remove from the Device List */
    BKNI_AcquireMutex(pDevice->captureMutex);
    BLST_S_REMOVE(&pDevice->contextList, pArmContext, BDSP_ArmContext, node);
    BKNI_ReleaseMutex(pDevice->captureMutex);

    /* Invalidate and free object */
    BDBG_OBJECT_DESTROY(pArmContext, BDSP_ArmContext);
    BKNI_Free(pArmContext);

    BDBG_LEAVE(BDSP_Arm_P_DestroyContext);
    return;
}

BERR_Code BDSP_Arm_P_PopulateGateOpenFMMStages(
                                void *pPrimaryStageHandle,
                                BDSP_AF_P_TASK_sFMM_GATE_OPEN_CONFIG *sTaskFmmGateOpenConfig,
                                uint32_t ui32MaxIndepDelay
                            )
{
    BERR_Code errCode = BERR_SUCCESS;
    unsigned output;
    unsigned ui32NumPorts = 0, channel, ui32FMMContentType, ui32FMMDstType;
    unsigned ui32BufferDepthThreshold;
    BDSP_ArmStage *pArmPrimaryStage = (BDSP_ArmStage *)pPrimaryStageHandle;
    BDSP_MMA_Memory IOBuffer_Source, IOBuffer_Destination;

    BDBG_ASSERT(NULL != pArmPrimaryStage);

    IOBuffer_Destination = pArmPrimaryStage->pArmTask->taskMemGrants.sTaskGateOpenBufInfo.Buffer;

    BDSP_ARM_STAGE_TRAVERSE_LOOP_BEGIN(pArmPrimaryStage, pArmConnectStage)
    BSTD_UNUSED(macroBrId);
    BSTD_UNUSED(macroStId);
    {
        for(output=0;output<BDSP_AF_P_MAX_OP_FORKS;output++)
        {
            if(pArmConnectStage->sStageOutput[output].eConnectionType == BDSP_ConnectionType_eFmmBuffer &&
                pArmConnectStage->sStageOutput[output].eNodeValid==BDSP_AF_P_eValid)
            {
                switch ( pArmConnectStage->eStageOpBuffDataType[output])
                {
                case BDSP_AF_P_DistinctOpType_e7_1_MixPcm:
                case BDSP_AF_P_DistinctOpType_e5_1_MixPcm:
                case BDSP_AF_P_DistinctOpType_eStereo_MixPcm:
                case BDSP_AF_P_DistinctOpType_e7_1_PCM:
                case BDSP_AF_P_DistinctOpType_e5_1_PCM:
                case BDSP_AF_P_DistinctOpType_eStereo_PCM:
                case BDSP_AF_P_DistinctOpType_eMono_PCM:
                    ui32FMMContentType = BDSP_AF_P_FmmContentType_ePcm;
                    ui32FMMDstType = BDSP_AF_P_FmmDstFsRate_eBaseRate;
                    ui32BufferDepthThreshold = (BDSP_AF_P_STANDARD_BUFFER_THRESHOLD + BDSP_AF_P_SAMPLE_PADDING);
                    break;
                case BDSP_AF_P_DistinctOpType_eCompressed:
                case BDSP_AF_P_DistinctOpType_eAuxilaryDataOut:
                case BDSP_AF_P_DistinctOpType_eGenericIsData:
                case BDSP_AF_P_DistinctOpType_eCdb:
                case BDSP_AF_P_DistinctOpType_eItb:
                case BDSP_AF_P_DistinctOpType_eDolbyReEncodeAuxDataOut:
                    ui32FMMContentType = BDSP_AF_P_FmmContentType_eCompressed;
                    ui32FMMDstType = BDSP_AF_P_FmmDstFsRate_eBaseRate;
                    ui32BufferDepthThreshold = (BDSP_AF_P_STANDARD_BUFFER_THRESHOLD + BDSP_AF_P_SAMPLE_PADDING);
                    break;
                case BDSP_AF_P_DistinctOpType_eCompressed4x:
                /*SW7425-6056: It is based on ui32FMMContentType that the FW decides on a type of zero fill.
                Without this check, Spdif preambles were filled in during a zero fill for BTSC
                SWRAA-162: New FMM Dest type added to address the same*/
                    if (pArmConnectStage->algorithm == BDSP_Algorithm_eBtscEncoder)
                    {
                        ui32FMMContentType = BDSP_AF_P_FmmContentType_eAnalogCompressed;
                    }else{
                        ui32FMMContentType = BDSP_AF_P_FmmContentType_eCompressed;
                    }
                    ui32FMMDstType = BDSP_AF_P_FmmDstFsRate_e4xBaseRate;
                    ui32BufferDepthThreshold = (BDSP_AF_P_COMPRESSED4X_BUFFER_THRESHOLD + BDSP_AF_P_SAMPLE_PADDING);
                    break;
                case BDSP_AF_P_DistinctOpType_eCompressedHBR:
                    ui32FMMContentType = BDSP_AF_P_FmmContentType_eCompressed;
                    ui32FMMDstType = BDSP_AF_P_FmmDstFsRate_e16xBaseRate;
                    if(pArmConnectStage->algorithm == BDSP_Algorithm_eMlpPassthrough)
                    {
                        /* Note: MLP Passthru uses a different logic for threshold and hence no need to add residual collection. The calculation is explained in the macro defination*/
                        ui32BufferDepthThreshold = BDSP_AF_P_COMPRESSED16X_MLP_BUFFER_THRESHOLD;
                    }
                    else
                    {
                        ui32BufferDepthThreshold = (BDSP_AF_P_COMPRESSED16X_BUFFER_THRESHOLD + BDSP_AF_P_SAMPLE_PADDING);
                    }
                    break;
                default:
                    ui32FMMContentType = BDSP_AF_P_FmmContentType_eInvalid;
                    ui32FMMDstType = BDSP_AF_P_FmmDstFsRate_eInvalid;
                    ui32BufferDepthThreshold = 0;
                    break;
                }

                for(channel=0;channel<pArmConnectStage->sStageOutput[output].IoBuffer.ui32NumBuffers ;channel++)
                {   /*the buffer itself was allocated by PI and sent as FMM descriptor, just passed on here for GATE Open configuration*/
                    sTaskFmmGateOpenConfig->sFmmGateOpenConfig[ui32NumPorts].uin32RingBufStartWrPointAddr[channel] = pArmConnectStage->sStageOutput[output].IoBuffer.sCircBuffer[channel].ui32ReadAddr + 20;
                }

#if 0
                sTaskFmmGateOpenConfig->sFmmGateOpenConfig[ui32NumPorts].uin32DramIoConfigAddr = pArmConnectStage->sStageOutput[output].ui32StageIOBuffCfgAddr;
#else
                IOBuffer_Source = pArmConnectStage->sStageOutput[output].IoBuffDesc;
                BKNI_Memcpy(IOBuffer_Destination.pAddr, IOBuffer_Source.pAddr, SIZEOF(BDSP_AF_P_sIO_BUFFER));
                BDSP_MMA_P_FlushCache(IOBuffer_Destination, SIZEOF(BDSP_AF_P_sIO_BUFFER));
                sTaskFmmGateOpenConfig->sFmmGateOpenConfig[ui32NumPorts].uin32DramIoConfigAddr = IOBuffer_Destination.offset;

                IOBuffer_Destination.pAddr = (void *)((uint8_t *)IOBuffer_Destination.pAddr + SIZEOF(BDSP_AF_P_sIO_BUFFER));
                IOBuffer_Destination.offset = IOBuffer_Destination.offset + SIZEOF(BDSP_AF_P_sIO_BUFFER);
#endif
                sTaskFmmGateOpenConfig->sFmmGateOpenConfig[ui32NumPorts].ui32IndepDelay= pArmConnectStage->sStageOutput[output].connectionDetails.fmm.descriptor.delay;
                sTaskFmmGateOpenConfig->sFmmGateOpenConfig[ui32NumPorts].eFMMContentType = ui32FMMContentType;
                sTaskFmmGateOpenConfig->sFmmGateOpenConfig[ui32NumPorts].eFmmDstFsRate = ui32FMMDstType;
                sTaskFmmGateOpenConfig->sFmmGateOpenConfig[ui32NumPorts].ui32BufferDepthThreshold = ui32BufferDepthThreshold;
                ui32NumPorts++;
            }
        }
    }
    BDSP_ARM_STAGE_TRAVERSE_LOOP_END(pArmConnectStage)

    sTaskFmmGateOpenConfig->ui32NumPorts = ui32NumPorts;
    sTaskFmmGateOpenConfig->ui32MaxIndepDelay = ui32MaxIndepDelay;

    return errCode;
}

BERR_Code BDSP_Arm_P_RetrieveGateOpenSettings(
                            void *pTaskHandle,
                            BDSP_TaskGateOpenSettings *pSettings   /* [out] */
    )
{
    BERR_Code errCode = BERR_SUCCESS;
    BDSP_ArmTask *pArmTask = (BDSP_ArmTask *)pTaskHandle;
    BDSP_AF_P_TASK_sFMM_GATE_OPEN_CONFIG sTaskFmmGateOpenConfig;
    int32_t index, index2;
    BDSP_AF_P_sFMM_GATE_OPEN_CONFIG *psFmmGateOpenConfig;
    BDSP_AF_P_sIO_BUFFER *pIOBuffer;

    if(pArmTask->startSettings.gateOpenReqd == true)
    {
        BDBG_ERR(("BDSP_Arm_P_RetrieveGateOpenSettings: Trying to retrieve Gate Open Settings for a task whose Gate Open already has been performed"));
        return BERR_INVALID_PARAMETER;
    }

    BKNI_Memset(&sTaskFmmGateOpenConfig,0,sizeof(BDSP_AF_P_TASK_sFMM_GATE_OPEN_CONFIG));
    errCode = BDSP_Arm_P_PopulateGateOpenFMMStages((void *)pArmTask->startSettings.primaryStage->pStageHandle,
        &sTaskFmmGateOpenConfig,
        pArmTask->startSettings.maxIndependentDelay);

    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Arm_P_RetrieveGateOpenSettings: Error in Retrieving the GateOpen Settings for the Task Requested"));
        return errCode;
    }

    /* Modify the Addresses to be returned to Generic Address*/
    psFmmGateOpenConfig = (BDSP_AF_P_sFMM_GATE_OPEN_CONFIG *)&sTaskFmmGateOpenConfig.sFmmGateOpenConfig;
    for(index = 0; index < (int32_t)sTaskFmmGateOpenConfig.ui32NumPorts; index++)
    {
        for(index2 = 0; index2 < (int32_t)BDSP_AF_P_MAX_CHANNELS; index2++)
        {
            psFmmGateOpenConfig->uin32RingBufStartWrPointAddr[index2]=(psFmmGateOpenConfig->uin32RingBufStartWrPointAddr[index2]-BCHP_PHYSICAL_OFFSET);
        }

		pIOBuffer =(BDSP_AF_P_sIO_BUFFER *)((BDSP_AF_P_sIO_BUFFER *)pArmTask->taskMemGrants.sTaskGateOpenBufInfo.Buffer.pAddr + index);
        for(index2=0;index2 < (int32_t)pIOBuffer->ui32NumBuffers; index2++)/*audio channels*/
        {   /*ensure that the descriptors for FMM and RAVE that are passed are physical address*/
           pIOBuffer->sCircBuffer[index2].ui32BaseAddr = (pIOBuffer->sCircBuffer[index2].ui32BaseAddr-BCHP_PHYSICAL_OFFSET);
           pIOBuffer->sCircBuffer[index2].ui32EndAddr  = (pIOBuffer->sCircBuffer[index2].ui32EndAddr-BCHP_PHYSICAL_OFFSET);
           pIOBuffer->sCircBuffer[index2].ui32ReadAddr = (pIOBuffer->sCircBuffer[index2].ui32ReadAddr-BCHP_PHYSICAL_OFFSET);
           pIOBuffer->sCircBuffer[index2].ui32WrapAddr = (pIOBuffer->sCircBuffer[index2].ui32WrapAddr-BCHP_PHYSICAL_OFFSET);
           pIOBuffer->sCircBuffer[index2].ui32WriteAddr= (pIOBuffer->sCircBuffer[index2].ui32WriteAddr-BCHP_PHYSICAL_OFFSET);
        }

        psFmmGateOpenConfig++;
    }
	BDSP_MMA_P_FlushCache(pArmTask->taskMemGrants.sTaskGateOpenBufInfo.Buffer,(sTaskFmmGateOpenConfig.ui32NumPorts*sizeof(BDSP_AF_P_sIO_BUFFER)));
	pSettings->PortAddr = pArmTask->taskMemGrants.sTaskGateOpenBufInfo.Buffer;

    pSettings->ui32NumPorts      = sTaskFmmGateOpenConfig.ui32NumPorts;
    pSettings->ui32MaxIndepDelay = sTaskFmmGateOpenConfig.ui32MaxIndepDelay;
    BKNI_Memcpy(pSettings->psFmmGateOpenConfig,
        (void *)&sTaskFmmGateOpenConfig.sFmmGateOpenConfig[0],
        (sTaskFmmGateOpenConfig.ui32NumPorts*sizeof(BDSP_AF_P_sFMM_GATE_OPEN_CONFIG)));

    return errCode;
}

/***********************************************************************
Name        :   BDSP_Arm_P_GetDefaultTaskSettings

Type        :   PI Interface

Input       :   pContextHandle  -   Context handle of the task for which settings are required.
                pSettings           -   Pointer of Task Create settings which needs to set to default settings.

Return      :   None

Functionality   :   Following are the operations performed.
        1)  Clear the memory that was provided by PI.
        2)  Set the master task variable by default as FALSE.
***********************************************************************/
void BDSP_Arm_P_GetDefaultTaskSettings(
    void *pContextHandle,
    BDSP_TaskCreateSettings *pSettings
    )
{
    BDSP_ArmContext *pArmContext = (BDSP_ArmContext *)pContextHandle;

    BDBG_ENTER(BDSP_Arm_P_GetDefaultTaskSettings);
    BDBG_OBJECT_ASSERT(pArmContext, BDSP_ArmContext);

    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->masterTask = false;

    BDBG_LEAVE(BDSP_Arm_P_GetDefaultTaskSettings);
}

static BERR_Code   BDSP_Arm_P_FreeAndInvalidateTask(
    void *pTaskHandle
    )
{
    BERR_Code   err=BERR_SUCCESS;
    BDSP_ArmTask *pArmTask = (BDSP_ArmTask *)pTaskHandle;
    BDSP_ArmContext *pArmContext = pArmTask->pContext;
    BDSP_Arm  *pDevice= pArmContext->pDevice;

    BDBG_OBJECT_ASSERT(pArmTask, BDSP_ArmTask);

    /* Remove from the Alloc and Mark task as free */
    BKNI_AcquireMutex(pDevice->captureMutex);
    BLST_S_REMOVE(&pArmTask->pContext->allocTaskList, pArmTask, BDSP_ArmTask, node);
    BKNI_ReleaseMutex(pDevice->captureMutex);
    BLST_S_INSERT_HEAD(&pArmTask->pContext->freeTaskList, pArmTask, node);
    pArmTask->allocated = false;

    return err;
}

static BERR_Code   BDSP_Arm_P_InitParams_CreateTask(
    void *pContextHandle,
    void *pTaskHandle,
    const BDSP_TaskCreateSettings *pSettings
    )
{
    BERR_Code   err=BERR_SUCCESS;
    BDSP_ArmContext *pArmContext = (BDSP_ArmContext *)pContextHandle;
    BDSP_ArmTask *pArmTask = (BDSP_ArmTask *)pTaskHandle;
    BDSP_Arm *pDevice= pArmContext->pDevice;

    BDBG_ENTER(BDSP_Arm_P_InitParams_CreateTask);

    BDBG_OBJECT_ASSERT(pArmContext, BDSP_ArmContext);
    BDBG_OBJECT_ASSERT(pArmTask, BDSP_ArmTask);

    /* Mark task as in use and save settings */
    BLST_S_REMOVE_HEAD(&pArmContext->freeTaskList, node);
    BKNI_AcquireMutex(pDevice->captureMutex);
    BLST_S_INSERT_HEAD(&pArmContext->allocTaskList, pArmTask, node);
    pArmTask->allocated = true;
    BKNI_Memcpy(&pArmTask->settings, pSettings, sizeof(*pSettings));

    pArmTask->isStopped = true;
    pArmTask->lastEventType= 0xFFFF;
    pArmTask->settings = *pSettings;
    pArmTask->commandCounter =0;
    /*pArmTask->paused=false;
    pArmTask->frozen=false;
    pArmTask->decLocked=false;*/

    BKNI_ReleaseMutex(pDevice->captureMutex);
    BDBG_LEAVE(BDSP_Arm_P_InitParams_CreateTask);
    return err;
}

/***********************************************************************
Name        :   BDSP_Arm_P_CreateTask

Type        :   PI Interface

Input       :   pContextHandle  -   Handle of the Context for which the task needs to created.
                pSettings           -   Settings of the Task to be created.
                pTask           -   Handle of the Task which is returned back to PI.

Return      :   Error Code to return SUCCESS or FAILURE

Functionality   :   Following are the operations performed.
        1)  Allocate and Intialise the memory for the Task Descriptor.
        2)  Intialise all the function pointers which will be used by the PI for further processing.
        3)  Allocate the Task related memory - CIT/Queues/Swap Buffer,etc.
        4)  Initliase the Task related parameters of Task Descriptors.
        5)  Install the Task Related Interrupts.
        6)  Error Handling if any required.
***********************************************************************/

BERR_Code BDSP_Arm_P_CreateTask(
    void *pContextHandle,
    const BDSP_TaskCreateSettings *pSettings,
    BDSP_TaskHandle *pTask
    )
{
    BDSP_ArmContext *pArmContext = (BDSP_ArmContext *)pContextHandle;
    BDSP_ArmTask *pArmTask;
    BERR_Code   err=BERR_SUCCESS;

    BDBG_ENTER(BDSP_Arm_P_CreateTask);

    BDBG_OBJECT_ASSERT(pArmContext, BDSP_ArmContext);

    *pTask = NULL;

    pArmTask = BKNI_Malloc(sizeof(BDSP_ArmTask));
    if ( NULL == pArmTask )
    {
        BDBG_ERR(("BDSP_Arm_P_CreateTask: No memory availabe to create the task. Hence exit"));
        err = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto err_malloc_task;
    }
    BKNI_Memset(pArmTask, 0, sizeof(BDSP_ArmTask));
    BDBG_OBJECT_SET(pArmTask, BDSP_ArmTask);

    /* Init task */
    BDSP_P_InitTask(&pArmTask->task, pArmTask);

    /* Add to free task list */
    BLST_S_INSERT_HEAD(&pArmContext->freeTaskList, pArmTask, node);

    pArmTask->task.destroy = BDSP_Arm_P_DestroyTask;
    pArmTask->task.start = BDSP_Arm_P_StartTask;
    pArmTask->task.stop = BDSP_Arm_P_StopTask;
    pArmTask->task.getDefaultTaskStartSettings = BDSP_Arm_P_GetDefaultTaskStartSettings;
    pArmTask->task.retreiveGateOpenSettings = BDSP_Arm_P_RetrieveGateOpenSettings;

    pArmTask->pContext = pArmContext;

    err = BDSP_Arm_P_AllocateTaskMemory((void *)pArmTask, pSettings);
    if ( err != BERR_SUCCESS )
    {
        err = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto err_task_mem;
    }

    err =BDSP_Arm_P_InitParams_CreateTask(pContextHandle,(void *)pArmTask,pSettings);

    if ( BERR_SUCCESS!=err )
    {
        err = BERR_TRACE(err);
        goto err_init_task;
    }

    *pTask = &pArmTask->task;
    goto create_task_success;

err_init_task:
    BDSP_Arm_P_FreeAndInvalidateTask(pTask);
err_task_mem:
    BDSP_Arm_P_FreeTaskMemory(pTask);

    /* Remove from free task list */
    BLST_S_REMOVE_HEAD(&pArmContext->freeTaskList, node);

    /* Destroy task */
    BDBG_OBJECT_DESTROY(pArmTask, BDSP_ArmTask);
    BKNI_Free(pArmTask);
err_malloc_task:
create_task_success:
    BDBG_LEAVE(BDSP_Arm_P_CreateTask);
    return err;
}

/***********************************************************************
Name        :   BDSP_Arm_P_DestroyTask

Type        :   PI Interface

Input       :   pTaskHandle -   Handle of the Task which needs to be destroyed.

Return      :   None

Functionality   :   Following are the operations performed.
        1)  Stop the task if it was not, before Destroying it.
        2)  Unistall the Interrupts for the task.
        3)  Remove the task from the allocated list for the Context.
        4)  Free the Task related memory - CIT/Queues/Swap Buffer,etc.
        5)  Free the Task descriptor.
***********************************************************************/
void BDSP_Arm_P_DestroyTask(
    void *pTaskHandle
    )
{
    BDSP_ArmTask *pTask = (BDSP_ArmTask *)pTaskHandle;
    BERR_Code err = BERR_SUCCESS;
    BDSP_MAP_Table_Entry MapTable[BDSP_ARM_MAX_ALLOC_TASK];
    uint32_t ui32NumEntries = 0;

    BDBG_ENTER(BDSP_Arm_P_DestroyTask);

    BDBG_OBJECT_ASSERT(pTask, BDSP_ArmTask);
    BDBG_ASSERT(pTask->allocated);

    if (pTask->isStopped == false)
    {
        BDBG_WRN(("BDSP_Arm_P_DestroyTask: Task is still in Start state. Stopping it."));
        BDSP_Arm_P_StopTask(pTaskHandle);
    }

    err =BDSP_Arm_P_FreeAndInvalidateTask(pTaskHandle);
    if ( BERR_SUCCESS!=err )
    {
        err = BERR_TRACE(err);
    }

    BKNI_Memset(MapTable,0,(BDSP_ARM_MAX_ALLOC_TASK*sizeof(BDSP_MAP_Table_Entry)));
    BDSP_Arm_P_RetrieveEntriesToUnMap(&(pTask->sTaskMapTable[0]), &MapTable[0], &ui32NumEntries, BDSP_ARM_MAX_ALLOC_TASK);
    err = BDSP_Arm_P_SendUnMapCommand(pTask->pContext->pDevice, &MapTable[0], ui32NumEntries);
    if (BERR_SUCCESS != err)
    {
        BDBG_ERR(("BDSP_Arm_P_DestroyTask: Send ARM UNMAP Command failed!!!!"));
    }

    err = BDSP_Arm_P_FreeTaskMemory(pTaskHandle);
    if ( BERR_SUCCESS!=err )
    {
        err = BERR_TRACE(err);
    }

    /* Destroy task */
    BLST_S_REMOVE_HEAD(&pTask->pContext->freeTaskList, node);
    BDBG_OBJECT_DESTROY(pTask, BDSP_ArmTask);
    BKNI_Free(pTask);

    BDBG_LEAVE(BDSP_Arm_P_DestroyTask);
    return;
}

/***********************************************************************
Name        :   BDSP_Arm_P_SendCitReconfigCommand

Type        :   BDSP Internal

Input       :   pArmTask        -   Handle of the Task for which the config change command needs to sent.
                psWorkingTaskCitBuffAddr_Cached -   Cached address of the working CIT buffer.

Return      :   Error Code to return SUCCESS or FAILURE

Functionality   :   Following are the operations performed.
        1)  Analyse the CIT network and flush the data into working CIT buffer.
        2)  Update the command with the new structures.
        3)  Issue the command and wait for it.
    Note: This command is used when we need to reconfigure the running task.
***********************************************************************/
BERR_Code BDSP_Arm_P_SendCitReconfigCommand(BDSP_ArmTask *pArmTask)
{

    BERR_Code err = BERR_SUCCESS;
    BDSP_Arm  *pDevice;
    BDSP_Arm_P_Command *psCommand;
    BDSP_P_MsgType eMsgType;
    BDSP_Arm_P_Response sRsp;

    BDBG_ENTER(BDSP_Arm_P_SendCitReconfigCommand);

    psCommand = BKNI_Malloc(sizeof(BDSP_Arm_P_Command));
    if ( NULL == psCommand )
    {
        err = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto err_malloc_command;
    }

    BKNI_Memset(psCommand, 0, sizeof(*psCommand));

    pDevice = pArmTask->pContext->pDevice;

	BKNI_Memcpy((void *)&pArmTask->citOutput.sCit, pArmTask->taskMemGrants.sSpareCitStruct.Buffer.pAddr, sizeof(BDSP_ARM_AF_P_sTASK_CONFIG));
    BDSP_Arm_P_Analyse_CIT(pArmTask, true);

    /*Prepare command to stop the task */
    psCommand->sCommandHeader.ui32CommandID = BDSP_RECONFIGURATION_COMMAND_ID;
    psCommand->sCommandHeader.ui32CommandCounter = 0;
    psCommand->sCommandHeader.ui32TaskID = pArmTask->taskId;
    psCommand->sCommandHeader.eResponseType = BDSP_P_ResponseType_eResponseRequired;
    psCommand->sCommandHeader.ui32CommandSizeInBytes =  sizeof(BDSP_Arm_P_Command);

    psCommand->uCommand.sCitReconfigCommand.ui32ModifiedCitAddr = pArmTask->taskMemGrants.sSpareCitStruct.Buffer.offset;
    psCommand->uCommand.sCitReconfigCommand.ui32RunningTaskCitAddr = pArmTask->taskMemGrants.sCitStruct.Buffer.offset;
    psCommand->uCommand.sCitReconfigCommand.ui32SizeInBytes = pArmTask->taskMemGrants.sCitStruct.ui32Size;

    pArmTask->lastEventType = psCommand->sCommandHeader.ui32CommandID;

    BKNI_ResetEvent(pArmTask->hEvent);
    err = BDSP_Arm_P_SendCommand(pDevice->hCmdQueue, psCommand,(void *)pArmTask);

    if (BERR_SUCCESS != err)
    {
        BDBG_ERR(("BDSP_Arm_P_SendCitReconfigCommand: CFG_Command failed!"));
        err = BERR_TRACE(err);
        goto end;
    }
    /* Wait for Ack_Response_Received event w/ timeout */
    err = BKNI_WaitForEvent(pArmTask->hEvent, BDSP_ARM_EVENT_TIMEOUT_IN_MS);
    if (BERR_TIMEOUT == err)
    {
        BDBG_ERR(("BDSP_Arm_P_SendCitReconfigCommand: CFG_Command TIMEOUT!"));
        err = BERR_TRACE(err);
        goto end;
    }
    BSTD_UNUSED(eMsgType);
    BSTD_UNUSED(sRsp);

end:
    BKNI_Free(psCommand);
err_malloc_command:
    BDBG_LEAVE(BDSP_Arm_P_SendCitReconfigCommand);
    return err;
}

/***********************************************************************
Name        :   BDSP_Arm_P_AddInterTaskBufferInput

Type        :   PI Interface

Input       :   pStageHandle        -   Handle of the Stage whose input will be Intertask buffer.
                dataType            -   Type of data present in the Intertask buffer.
                pBufferHandle       -   Intertask buffer descriptor provided by the PI.
                pInputIndex     -   Index of the input which is associated for the Intertask buffer.

Return      :   Error Code to return SUCCESS or FAILURE

Functionality   :   Following are the operations performed.
        1)  Retreive the the free input index for the stage which can be used for this Intertask buffer.
        2)  Populate the Source details for the Stage with appropriate values using the Intertask descriptor provided by the PI.
        3)  Populate the Address of the I/O and I/O Generic buffers.
        4)  Increment the number of inputs from Intertask buffer for this stage and total inputs to stage.
        5)  Set the Intertask buffer's InUse variable if some stage is already connected to feed data into the Intertask buffer.
        6)  Fill the input index pointer for the PI for later use.
        7)  If the stage is already running/inside watchdog recovery then perform the CIT reconfiguration.
***********************************************************************/
BERR_Code BDSP_Arm_P_AddInterTaskBufferInput(
    void *pStageHandle,
    BDSP_DataType dataType,
    const BDSP_InterTaskBuffer *pBufferHandle,
    unsigned *pInputIndex)
{
    BERR_Code err = BERR_SUCCESS;
    BDSP_ArmStage *pArmStage = (BDSP_ArmStage *)pStageHandle;
    BDSP_P_InterTaskBuffer *pArmInterTaskBuffer = (BDSP_P_InterTaskBuffer *)pBufferHandle;
    unsigned ipIndex;
    BDSP_AF_P_sIO_BUFFER *pIoBuffDesc_Cached = NULL;

    BDBG_ENTER(BDSP_Arm_P_AddInterTaskBufferInput);

    BDBG_OBJECT_ASSERT(pArmStage, BDSP_ArmStage);
    BDBG_OBJECT_ASSERT(pArmInterTaskBuffer, BDSP_P_InterTaskBuffer);
    BDBG_ASSERT(NULL != pInputIndex);
    BDBG_ASSERT(dataType == pArmInterTaskBuffer->dataType);

    BDSP_P_GetFreeInputPortIndex(&(pArmStage->sStageInput[0]), &ipIndex);
    BDBG_ASSERT(ipIndex < BDSP_AF_P_MAX_IP_FORKS);

    /* Connect the inter-task buffer to the destination stage's input */
    pArmStage->sStageInput[ipIndex].eNodeValid = BDSP_AF_P_eValid;
    pArmStage->sStageInput[ipIndex].eConnectionType = BDSP_ConnectionType_eInterTaskBuffer;
    pArmStage->sStageInput[ipIndex].connectionHandle = NULL;
    pArmStage->sStageInput[ipIndex].connectionDetails.interTask.hInterTask
        = (BDSP_InterTaskBufferHandle)&pArmInterTaskBuffer->interTaskBuffer;

	pArmStage->sStageInput[ipIndex].StageIOBuffDescAddr = pArmInterTaskBuffer->IoBufferDesc.offset;
	pArmStage->sStageInput[ipIndex].StageIOGenericBuffDescAddr = pArmInterTaskBuffer->IoBufferGenericDesc.offset;

    err = BDSP_Arm_P_InsertEntry_MapTable(&(pArmStage->sStageMapTable[0]),
                                &pArmInterTaskBuffer->IoBufferDesc,
                                sizeof(BDSP_AF_P_sIO_BUFFER),
                                (BDSP_ARM_AF_P_Map_eDram),
                                BDSP_ARM_MAX_ALLOC_STAGE);

    err = BDSP_Arm_P_InsertEntry_MapTable(&(pArmStage->sStageMapTable[0]),
                                &pArmInterTaskBuffer->IoBufferGenericDesc,
                                sizeof(BDSP_AF_P_sIO_GENERIC_BUFFER),
                                (BDSP_ARM_AF_P_Map_eDram),
                                BDSP_ARM_MAX_ALLOC_STAGE);

    err = BDSP_Arm_P_InsertEntry_MapTable(&(pArmStage->sStageMapTable[0]),
                                &pArmInterTaskBuffer->IoBuffer,
                                pArmInterTaskBuffer->numChans*BDSP_AF_P_INTERTASK_IOBUFFER_SIZE,
                                (BDSP_ARM_AF_P_Map_eDram),
                                BDSP_ARM_MAX_ALLOC_STAGE);

    err = BDSP_Arm_P_InsertEntry_MapTable(&(pArmStage->sStageMapTable[0]),
                                &pArmInterTaskBuffer->IoGenBuffer,
                                BDSP_AF_P_INTERTASK_IOGENBUFFER_SIZE,
                                (BDSP_ARM_AF_P_Map_eDram),
                                BDSP_ARM_MAX_ALLOC_STAGE);

    /* Flush cache for all the above memory */
    BDSP_MMA_P_FlushCache(pArmInterTaskBuffer->IoBuffer, (pArmInterTaskBuffer->numChans*BDSP_AF_P_INTERTASK_IOBUFFER_SIZE));
    BDSP_MMA_P_FlushCache(pArmInterTaskBuffer->IoGenBuffer, BDSP_AF_P_INTERTASK_IOGENBUFFER_SIZE);
    BDSP_MMA_P_FlushCache(pArmInterTaskBuffer->IoBufferDesc, sizeof(BDSP_AF_P_sIO_BUFFER));
    BDSP_MMA_P_FlushCache(pArmInterTaskBuffer->IoBufferGenericDesc, sizeof(BDSP_AF_P_sIO_GENERIC_BUFFER));
    pArmStage->numInputs[BDSP_ConnectionType_eInterTaskBuffer] += 1;

    pArmInterTaskBuffer->dstIndex = ipIndex;
    pArmInterTaskBuffer->dstHandle = pStageHandle;

    *pInputIndex = ipIndex;
    pArmStage->totalInputs++;

    /* Send the cit re-configuration command to the DSP to indicate an added input port */
    /* if the dest stage is running and Watchdog recovery is not underway */
    if ((pArmStage->running)&&(!pArmStage->pContext->contextWatchdogFlag))
    {
        BDSP_ArmTask *pArmTask = pArmStage->pArmTask;


        BDSP_ARM_AF_P_sNODE_CONFIG  *psNodeConfig;
        uint32_t ui32NodeIpBuffCfgAddr, ui32NodeIpGenericDataBuffCfgAddr;
        BDSP_ARM_AF_P_sTASK_CONFIG  *psWorkingTaskCitBuffAddr_Cached = NULL;

        BDBG_OBJECT_ASSERT(pArmTask, BDSP_ArmTask);

        psWorkingTaskCitBuffAddr_Cached = (BDSP_ARM_AF_P_sTASK_CONFIG *)pArmTask->taskMemGrants.sSpareCitStruct.Buffer.pAddr;

        /* Assuming that the input inter-task buffer will be added only to stage 0 */
        pIoBuffDesc_Cached = (BDSP_AF_P_sIO_BUFFER *)pArmInterTaskBuffer->IoBufferDesc.pAddr;

        /* SIPS is only supported if the Inter task buffer type is DRAM */
        if (pIoBuffDesc_Cached->eBufferType != BDSP_AF_P_BufferType_eDRAM)
        {
            BDBG_ERR(("Cannot add input port seamlessly for non-dram type (%d) buffers",
                pIoBuffDesc_Cached->eBufferType));
        }

        ui32NodeIpBuffCfgAddr = pArmInterTaskBuffer->IoBufferDesc.offset;
        ui32NodeIpGenericDataBuffCfgAddr = pArmInterTaskBuffer->IoBufferGenericDesc.offset;

        psNodeConfig = &psWorkingTaskCitBuffAddr_Cached->sNodeConfig[0];
        /* Making the Input port valid */
        psNodeConfig->eNodeIpValidFlag[ipIndex] = BDSP_AF_P_eValid;
        /* Setting the IO and IO Generic buffer structure addresses for the Node 0 */
        psNodeConfig->ui32NodeIpBuffCfgAddr[ipIndex] = ui32NodeIpBuffCfgAddr;
        psNodeConfig->ui32NodeIpGenericDataBuffCfgAddr[ipIndex] = ui32NodeIpGenericDataBuffCfgAddr;
        psNodeConfig->ui32NumSrc++;

        /* Setting the IO and IO Generic buffer structure addresses and enabling the Valid Flag for the Node 1 */
        psNodeConfig = &psWorkingTaskCitBuffAddr_Cached->sNodeConfig[BDSP_CIT_P_NUM_SPECIAL_NODES];
        psNodeConfig->ui32NodeIpBuffCfgAddr[ipIndex] = ui32NodeIpBuffCfgAddr;
        psNodeConfig->ui32NodeIpGenericDataBuffCfgAddr[ipIndex] = ui32NodeIpGenericDataBuffCfgAddr;
        psNodeConfig->eNodeIpValidFlag[ipIndex] = BDSP_AF_P_eValid;
        psNodeConfig->ui32NumSrc++;

        BDSP_MMA_P_FlushCache(pArmTask->taskMemGrants.sSpareCitStruct.Buffer, pArmTask->taskMemGrants.sSpareCitStruct.ui32Size);

        /* Send the cit re-configuration command to DSP */
        err = BDSP_Arm_P_SendCitReconfigCommand(pArmTask);
        if (err)
        {
            goto err;
        }
    }
    if (pArmInterTaskBuffer->srcHandle)
    {
        pArmInterTaskBuffer->inUse = true;
    }

    goto end;

err:
    *pInputIndex = BDSP_AF_P_MAX_IP_FORKS;
    pArmStage->totalInputs--;
    pArmStage->eStageOpBuffDataType[ipIndex] = BDSP_AF_P_DistinctOpType_eMax;
    pArmInterTaskBuffer->dstIndex = -1;
    pArmInterTaskBuffer->dstHandle = NULL;
    pArmStage->numInputs[BDSP_ConnectionType_eInterTaskBuffer] -= 1;

    pArmStage->sStageInput[ipIndex].eNodeValid = false;
    pArmStage->sStageInput[ipIndex].eConnectionType = BDSP_ConnectionType_eMax;
    pArmStage->sStageInput[ipIndex].connectionHandle = NULL;

    BKNI_Memset(&pArmStage->sStageInput[ipIndex].connectionDetails, 0,
                sizeof(pArmStage->sStageInput[ipIndex].connectionDetails));

end:
    BDBG_LEAVE(BDSP_Arm_P_AddInterTaskBufferInput);
    return err;
}


/***********************************************************************
Name        :   BDSP_Arm_P_AddInterTaskBufferOutput

Type        :   PI Interface

Input       :   pStageHandle        -   Handle of the Stage whose output will be Intertask buffer.
                dataType            -   Type of data present in the Intertask buffer.
                pBufferHandle       -   Intertask buffer descriptor provided by the PI.
                pOutputIndex        -   Index of the output which is associated for the Intertask buffer.

Return      :   Error Code to return SUCCESS or FAILURE

Functionality   :   Following are the operations performed.
        1)  Retreive the the free output index for the stage which can be used for this Intertask buffer.
        2)  Populate the Destination details for the Stage with appropriate values using the Intertask descriptor provided by the PI.
        3)  Populate the Address of the I/O and I/O Generic buffers.
        4)  Increment the number of outputs to Intertask buffer from this stage and total outputs from this stage.
        5)  Set the Intertask buffer's InUse variable if some stage is already connected to consume data from this  Intertask buffer.
        6)  Fill the input index pointer for the PI for later use.
***********************************************************************/

BERR_Code BDSP_Arm_P_AddInterTaskBufferOutput(
    void *pStageHandle,
    BDSP_DataType dataType,
    const BDSP_InterTaskBuffer *pBufferHandle,
    unsigned *pOutputIndex)
{
    BERR_Code err = BERR_SUCCESS;
    BDSP_ArmStage *pArmStage = (BDSP_ArmStage *)pStageHandle;
    BDSP_P_InterTaskBuffer *pArmInterTaskBuffer = (BDSP_P_InterTaskBuffer *)pBufferHandle;
    unsigned opIndex;

    BDBG_ENTER(BDSP_Arm_P_AddInterTaskBufferOutput);
    BDBG_OBJECT_ASSERT(pArmStage, BDSP_ArmStage);
    BDBG_OBJECT_ASSERT(pArmInterTaskBuffer, BDSP_P_InterTaskBuffer);
    BDBG_ASSERT(NULL != pOutputIndex);
    BDBG_ASSERT(dataType == pArmInterTaskBuffer->dataType);

    BDSP_P_GetFreeOutputPortIndex(&(pArmStage->sStageOutput[0]), &opIndex);
    BDBG_ASSERT(opIndex < BDSP_AF_P_MAX_OP_FORKS);

    /* Connect the inter-task buffer to the source stage's output */
    pArmStage->sStageOutput[opIndex].eNodeValid = true;
    pArmStage->sStageOutput[opIndex].eConnectionType = BDSP_ConnectionType_eInterTaskBuffer;
    pArmStage->sStageOutput[opIndex].connectionHandle = NULL;
    pArmStage->sStageOutput[opIndex].connectionDetails.interTask.hInterTask
        = (BDSP_InterTaskBufferHandle)&pArmInterTaskBuffer->interTaskBuffer;

	pArmStage->sStageOutput[opIndex].StageIOBuffDescAddr = pArmInterTaskBuffer->IoBufferDesc.offset;
	pArmStage->sStageOutput[opIndex].StageIOGenericBuffDescAddr = pArmInterTaskBuffer->IoBufferGenericDesc.offset;
    err = BDSP_Arm_P_InsertEntry_MapTable(&(pArmStage->sStageMapTable[0]),
                                &pArmInterTaskBuffer->IoBufferDesc,
                                sizeof(BDSP_AF_P_sIO_BUFFER),
                                (BDSP_ARM_AF_P_Map_eDram),
                                BDSP_ARM_MAX_ALLOC_STAGE);

    err = BDSP_Arm_P_InsertEntry_MapTable(&(pArmStage->sStageMapTable[0]),
                                &pArmInterTaskBuffer->IoBufferGenericDesc,
                                sizeof(BDSP_AF_P_sIO_GENERIC_BUFFER),
                                (BDSP_ARM_AF_P_Map_eDram),
                                BDSP_ARM_MAX_ALLOC_STAGE);

    err = BDSP_Arm_P_InsertEntry_MapTable(&(pArmStage->sStageMapTable[0]),
                                &pArmInterTaskBuffer->IoBuffer,
                                pArmInterTaskBuffer->numChans*BDSP_AF_P_INTERTASK_IOBUFFER_SIZE,
                                (BDSP_ARM_AF_P_Map_eDram),
                                BDSP_ARM_MAX_ALLOC_STAGE);

    err = BDSP_Arm_P_InsertEntry_MapTable(&(pArmStage->sStageMapTable[0]),
                                &pArmInterTaskBuffer->IoGenBuffer,
                                BDSP_AF_P_INTERTASK_IOGENBUFFER_SIZE,
                                (BDSP_ARM_AF_P_Map_eDram),
                                BDSP_ARM_MAX_ALLOC_STAGE);
    pArmStage->numOutputs[BDSP_ConnectionType_eInterTaskBuffer] += 1;
    pArmStage->eStageOpBuffDataType[opIndex] = pArmInterTaskBuffer->distinctOp;

    pArmInterTaskBuffer->srcIndex = opIndex;
    pArmInterTaskBuffer->srcHandle = pStageHandle;

    if (pArmInterTaskBuffer->dstHandle)
    {
        pArmInterTaskBuffer->inUse = true;
    }

    *pOutputIndex = opIndex;
    pArmStage->totalOutputs++;

    BDBG_LEAVE(BDSP_Arm_P_AddInterTaskBufferOutput);

    return err;
}

/***********************************************************************
Name        :   BDSP_Arm_P_AddFmmOutput

Type        :   PI Interface

Input       :   pStageHandle        -   Handle of the Stage for which output must be fed to FMM Buffer.
                dataType            -   Type of data that will fed into FMM buffer.
                pDescriptor     -   Descriptor for the FMM buffer.
                pOutputIndex        -   Index of the Ouput from this Stage returned back to PI.

Return      :   Error Code to return SUCCESS or FAILURE

Functionality   :   Following are the operations performed.
        1)  Retreive the the free output index for the stage which can be used for this FMM buffer.
        2)  Depending on the datatype, figure out the number of channels and output type.
        3)  Populate the Dstination details for the Stage with appropriate values using the FMM descriptor provided by the PI.
        4)  Populate the Address of the I/O buffers and rate control data.
        5)  Increment the number of ouputs from this stage and also the number of FMM buffers in the eco-system.
        6)  Fill the Ouput index pointer for the PI for later use.
***********************************************************************/

BERR_Code BDSP_Arm_P_AddFmmOutput(
    void *pStageHandle,
    BDSP_DataType dataType,
    const BDSP_FmmBufferDescriptor *pDescriptor,
    unsigned *pOutputIndex
    )
{
    BDSP_ArmStage *pArmStage = (BDSP_ArmStage *)pStageHandle;
    BERR_Code errCode=BERR_SUCCESS;
    unsigned channels=0, i, opIndex;

    BDBG_ENTER(BDSP_Arm_P_AddFmmOutput);

    BDSP_P_GetFreeOutputPortIndex(&(pArmStage->sStageOutput[0]), &opIndex);
    BDBG_ASSERT(opIndex < BDSP_AF_P_MAX_OP_FORKS);

    BDSP_P_GetDistinctOpTypeAndNumChans(dataType, &channels, &pArmStage->eStageOpBuffDataType[opIndex]);
    if(channels!=pDescriptor->numBuffers)
    {
        errCode=BERR_TRACE(BERR_INVALID_PARAMETER);
        BDBG_ERR(("BDSP_Arm_P_AddFmmOutput::FMM Output not added!"));
        goto err_valid_channels;
    }

    pArmStage->sStageOutput[opIndex].eNodeValid=BDSP_AF_P_eValid;
    pArmStage->sStageOutput[opIndex].IoBuffer.ui32NumBuffers=pDescriptor->numBuffers;
    pArmStage->sStageOutput[opIndex].IoBuffer.eBufferType=BDSP_AF_P_BufferType_eFMM;

    pArmStage->sStageOutput[opIndex].eConnectionType=BDSP_ConnectionType_eFmmBuffer;
    pArmStage->sStageOutput[opIndex].connectionHandle = (BDSP_StageHandle)NULL;
    pArmStage->sStageOutput[opIndex].connectionDetails.fmm.descriptor = *pDescriptor;

    pArmStage->sStageOutput[opIndex].IoBuffer.eBufferType=BDSP_AF_P_BufferType_eFMM;
    pArmStage->sStageOutput[opIndex].IoBuffer.ui32NumBuffers=channels;

    /*not required to explicitly mention this, I guess pkr*/
    pArmStage->sStageOutput[opIndex].Metadata.IoGenericBuffer.ui32NumBuffers=0;

    for(i=0;i<pDescriptor->numBuffers;i++)/*audio channels*/
    {
        pArmStage->sStageOutput[opIndex].IoBuffer.sCircBuffer[i].ui32BaseAddr  = pDescriptor->buffers[i].base;
        pArmStage->sStageOutput[opIndex].IoBuffer.sCircBuffer[i].ui32EndAddr   = pDescriptor->buffers[i].end;
        pArmStage->sStageOutput[opIndex].IoBuffer.sCircBuffer[i].ui32ReadAddr  = pDescriptor->buffers[i].read;
        pArmStage->sStageOutput[opIndex].IoBuffer.sCircBuffer[i].ui32WrapAddr  = pDescriptor->buffers[i].end;
        pArmStage->sStageOutput[opIndex].IoBuffer.sCircBuffer[i].ui32WriteAddr = pDescriptor->buffers[i].write;
    }

    for(i=0;i<BDSP_AF_P_MAX_CHANNEL_PAIR;i++)/*rate controller Initialize per pair of channels*/
    {
        pArmStage->sStageOutput[opIndex].Metadata.rateController[i].wrcnt=-1;/*Initialise*/
    }

    for(i=0;i<((pDescriptor->numBuffers+1)>>1);i++)/*rate controller per pair of channels*/
    {
        pArmStage->sStageOutput[opIndex].Metadata.rateController[i].wrcnt=pDescriptor->rateControllers[i].wrcnt;
    }

	pArmStage->sStageOutput[opIndex].StageIOBuffDescAddr = pArmStage->sStageOutput[opIndex].IoBuffDesc.offset;
	pArmStage->sStageOutput[opIndex].StageIOGenericBuffDescAddr = pArmStage->sStageOutput[opIndex].IoGenBuffDesc.offset;

    pArmStage->numOutputs[BDSP_ConnectionType_eFmmBuffer]+=1;

    *pOutputIndex = opIndex;/*Total outputs going out from this stage, both IS and FMM*/
    pArmStage->totalOutputs += 1;

err_valid_channels:
    BDBG_LEAVE(BDSP_Arm_P_AddFmmOutput);
    return(errCode);
}

#ifdef BDSP_QUEUE_DEBUG

/***********************************************************************
Name        :   BDSP_Arm_P_AddQueueOutput

Type        :   PI Interface

Input       :   pStageHandle        -   Handle of the Stage whose output will be to a Queue/RDB Buffer.
                pQueueHandle        -   Handle of the Queue provided by the PI.
                pOutputIndex        -   Index of the output from this stage provided back to PI.

Return      :   Error Code to return SUCCESS or FAILURE

Functionality   :   Following are the operations performed.
        1)  Retreive the the free output index for the stage which can be used for this Queue.
        2)  Populate the Destination details for the Stage with appropriate values using the Queue descriptor provided by the PI.
            The type of output will be treated as RDB type each for CDB and ITB. Separate buffers are allocated for CDB and ITB
            and not treated as individual/seperate outputs.
        3)  Populate the Address of the I/O buffer using the FIFO address filled during Create Queue. There is no I/O Generic buffer.
        4)  Increment the number of outputs to RDB buffer for this stage and total outputs for the stage.
        5)  Fill the output index pointer for the PI for later use.
***********************************************************************/

BERR_Code BDSP_Arm_P_AddQueueOutput(
                                    void     *pStageHandle,
                                    void     *pQueueHandle,
                                    unsigned *pOutputIndex /* [out] */
                                    )
{
    BDSP_ArmStage *pArmStage = (BDSP_ArmStage *)pStageHandle;
    BDSP_RaagaQueue *pRaagaQueue = (BDSP_RaagaQueue *)pQueueHandle;
    BERR_Code errCode = BERR_SUCCESS;
    BDSP_RaagaContext   *pRaagaContext; /*src or dest*/
    BDSP_Raaga *pDevice;
    unsigned opIndex, i;

    BDBG_ENTER(BDSP_Arm_P_AddQueueOutput);

    BDBG_ASSERT(NULL != pStageHandle);
    BDBG_ASSERT(NULL != pQueueHandle);
    BDBG_ASSERT(NULL != pOutputIndex);

    BDBG_ERR(("BOSS REMOVE BDSP_Arm_P_AddQueueOutput as this has been added for Debug purpose!!!!!!"));
    pRaagaContext = (BDSP_RaagaContext *)pRaagaQueue->pRaagaContext; /*src or dest*/
    pDevice = (BDSP_Raaga *)pRaagaContext->pDevice;

    BDSP_P_GetFreeOutputPortIndex(&(pArmStage->sStageOutput[0]), &opIndex);
    BDBG_ASSERT(opIndex < BDSP_AF_P_MAX_OP_FORKS);

    /* Note CDB and ITB are separate Index/Queue Handle - Needs to be taken care */
    /* Output */
    pArmStage->sStageOutput[opIndex].eNodeValid = BDSP_AF_P_eValid;
    pArmStage->sStageOutput[opIndex].IoBuffer.ui32NumBuffers = pRaagaQueue->numBuf;
    pArmStage->sStageOutput[opIndex].IoBuffer.eBufferType = BDSP_AF_P_BufferType_eRDB;
    if(pRaagaQueue->dataType == BDSP_DataType_eRDBPool)
    {
        pArmStage->sStageOutput[opIndex].IoBuffer.eBufferType = BDSP_AF_P_BufferType_eRDBPool;
    }

    pArmStage->sStageOutput[opIndex].eConnectionType = BDSP_ConnectionType_eRDBBuffer;
    pArmStage->sStageOutput[opIndex].connectionHandle = (BDSP_StageHandle)NULL;
    pArmStage->sStageOutput[opIndex].connectionDetails.rdb.pQHandle = (void *)pRaagaQueue; /* Need to verify the warning */

    /*not required to explicitly mention this, I guess pkr*/
    pArmStage->sStageOutput[opIndex].Metadata.IoGenericBuffer.ui32NumBuffers=0;

    for(i = 0; i< pRaagaQueue->numBuf; i++)
    {
        pArmStage->sStageOutput[opIndex].IoBuffer.sCircBuffer[i].ui32BaseAddr  = BDSP_REGSET_PHY_ADDR_FOR_DSP(pRaagaQueue->FIFOdata[i]->ui32BaseAddr);
        pArmStage->sStageOutput[opIndex].IoBuffer.sCircBuffer[i].ui32EndAddr   = BDSP_REGSET_PHY_ADDR_FOR_DSP(pRaagaQueue->FIFOdata[i]->ui32EndAddr);
        pArmStage->sStageOutput[opIndex].IoBuffer.sCircBuffer[i].ui32ReadAddr  = BDSP_REGSET_PHY_ADDR_FOR_DSP(pRaagaQueue->FIFOdata[i]->ui32ReadAddr);
        pArmStage->sStageOutput[opIndex].IoBuffer.sCircBuffer[i].ui32WrapAddr  = BDSP_REGSET_PHY_ADDR_FOR_DSP(pRaagaQueue->FIFOdata[i]->ui32EndAddr);
        pArmStage->sStageOutput[opIndex].IoBuffer.sCircBuffer[i].ui32WriteAddr = BDSP_REGSET_PHY_ADDR_FOR_DSP(pRaagaQueue->FIFOdata[i]->ui32WriteAddr);

        errCode = BDSP_Arm_P_InsertEntry_MapTable(pArmStage->pContext->pDevice->memHandle,
                                    &(pArmStage->sStageMapTable[0]),
                                    pRaagaQueue->FIFOdata[i]->pBaseAddr,
                                    pRaagaQueue->FIFOdata[i]->ui32Size,
                                    (BDSP_ARM_AF_P_Map_eDram),
                                    BDSP_ARM_MAX_ALLOC_STAGE);
    }

    pArmStage->eStageOpBuffDataType[opIndex] = pRaagaQueue->distinctOp;

    /* This needs to be done only for non-interstage connections, since the interstage buffer descriptors
        are allocated with the interstage buffers */
	pArmStage->sStageOutput[opIndex].StageIOBuffDescAddr = pArmStage->sStageOutput[opIndex].IoBuffDesc.offset;
	pArmStage->sStageOutput[opIndex].StageIOGenericBuffDescAddr = pArmStage->sStageOutput[opIndex].IoGenBuffDesc.offset;
    pArmStage->numOutputs[BDSP_ConnectionType_eRDBBuffer]+=1;
    pArmStage->totalOutputs+=1;

    /* The index returned is always the ITB output index, so at remove output,
            the stage output for index-1 is also invalidated */
    *pOutputIndex = opIndex;/*Total outputs going out from this stage, both IS and FMM*/

    BDBG_LEAVE(BDSP_Arm_P_AddQueueOutput);
    return(errCode);
}
#endif /* BDSP_QUEUE_DEBUG */
/***********************************************************************
Name        :   BDSP_Arm_P_AddOutputStage

Type        :   PI Interface

Input       :   pSrcStageHandle -   Handle of the Source Stage for which output must be fed to another Stage.
                dataType            -   Type of Data which is output of the Stage.
                pDstStageHandle -   Handle of the Destination Stage which must be connected at Output.
                pSourceOutputIndex  -   Index of the Ouput for the Source Stage.
                pDestinationInputIndex  -   Index of the Input for the Destination Stage.

Return      :   Error Code to return SUCCESS or FAILURE

Functionality   :   Following are the operations performed.
        1)  Retreive the the free output and index for the Source and Destination Stage respectively.
        2)  Interconnect the Stages but populating the Output and Input structures of Source Stage and Destination Stage using a InterStage Buffer.
        3)  Return the Source Output index and Destination Intput Index.
***********************************************************************/

BERR_Code BDSP_Arm_P_AddOutputStage(
    void *pSrcStageHandle,
    BDSP_DataType dataType,
    void *pDstStageHandle,
    unsigned *pSourceOutputIndex,
    unsigned *pDestinationInputIndex
    )
{
    BDSP_AF_P_sIO_BUFFER *pIoBuffDesc_Cached = NULL;
    BDSP_AF_P_sIO_GENERIC_BUFFER *pIoGenBuffDesc_Cached = NULL;
    BDSP_ArmStage *pArmSrcStage = pSrcStageHandle;
    BDSP_ArmStage *pArmDstStage = pDstStageHandle;
    BERR_Code errCode = BERR_SUCCESS;
    unsigned numBuffers = 1, ipIndex, opIndex;

    BDBG_ASSERT(pSrcStageHandle);
    BDBG_ASSERT(pDstStageHandle);

    BDBG_ASSERT(pArmSrcStage);

    BDSP_P_GetFreeInputPortIndex(&(pArmDstStage->sStageInput[0]), &ipIndex);
    BDBG_ASSERT(ipIndex < BDSP_AF_P_MAX_IP_FORKS);

    BDSP_P_GetFreeOutputPortIndex(&(pArmSrcStage->sStageOutput[0]), &opIndex);
    BDBG_ASSERT(opIndex < BDSP_AF_P_MAX_OP_FORKS);

    /*Populate the distinct output type which can be used to prepare node configuration*/
    BDSP_P_GetDistinctOpTypeAndNumChans(dataType, &numBuffers, &pArmSrcStage->eStageOpBuffDataType[opIndex]);

    pArmSrcStage->sStageOutput[opIndex].eNodeValid=BDSP_AF_P_eValid;
    pArmSrcStage->sStageOutput[opIndex].eConnectionType=BDSP_ConnectionType_eStage;
    pArmSrcStage->sStageOutput[opIndex].connectionHandle = &pArmDstStage->stage;
    pArmSrcStage->sStageOutput[opIndex].connectionDetails.stage.hStage=&pArmDstStage->stage;

    pIoBuffDesc_Cached = (BDSP_AF_P_sIO_BUFFER *)pArmSrcStage->sStageOutput[opIndex].IoBuffDesc.pAddr;
    pIoGenBuffDesc_Cached = (BDSP_AF_P_sIO_GENERIC_BUFFER *)pArmSrcStage->sStageOutput[opIndex].IoGenBuffDesc.pAddr;
    pIoBuffDesc_Cached->eBufferType = BDSP_AF_P_BufferType_eDRAM_IS;
    pIoBuffDesc_Cached->ui32NumBuffers = numBuffers;

    /* DDRE does a DMA out to the input buffers and the core code always runs as 5.1. For a stereo case this will fail in the new CIT
    as we will only allocate stereo buffers. Hardcoding to 8 for now and this will have to be removed once the DDRE implementation
    is changed to DMA required data to scratch and use the scratch space instead of the input buffer */

    if (pArmDstStage->algorithm == BDSP_Algorithm_eDdre)
    {
        pIoBuffDesc_Cached->ui32NumBuffers = 8;
    }

    /*the actual buffers itself allocated will be assigned during start time as we dont know the node network now*/
    pIoGenBuffDesc_Cached->eBufferType = BDSP_AF_P_BufferType_eDRAM_IS;
    pIoGenBuffDesc_Cached->ui32NumBuffers = 1;

    BDSP_MMA_P_FlushCache(pArmSrcStage->sStageOutput[opIndex].IoBuffDesc, sizeof(BDSP_AF_P_sIO_BUFFER));
    BDSP_MMA_P_FlushCache(pArmSrcStage->sStageOutput[opIndex].IoGenBuffDesc, sizeof(BDSP_AF_P_sIO_GENERIC_BUFFER));

	pArmSrcStage->sStageOutput[opIndex].StageIOBuffDescAddr = pArmSrcStage->sStageOutput[opIndex].IoBuffDesc.offset;
	pArmSrcStage->sStageOutput[opIndex].StageIOGenericBuffDescAddr = pArmSrcStage->sStageOutput[opIndex].IoGenBuffDesc.offset;

    pArmDstStage->sStageInput[ipIndex].eNodeValid=BDSP_AF_P_eValid;
    pArmDstStage->sStageInput[ipIndex].eConnectionType=BDSP_ConnectionType_eStage;
    pArmDstStage->sStageInput[ipIndex].connectionHandle = &pArmSrcStage->stage;
    pArmDstStage->sStageInput[ipIndex].connectionDetails.stage.hStage=&pArmSrcStage->stage;

    /*We have allocated one descriptor for every input and output,
    so for a connection we have one o/p descriptor from the input stage and one i/p descriptor for the output stage*/
    /*ONE DESCRIPTOR FOR ONE CONNECTION AND LET THAT BE THE INPUT(SRC) STAGE'S OUTPUT DESCRIPTOR ie pArmSrcStage->sStageOutput*/

    /*the actual buffers itself allocated will be assigned during start time as we dont know the node network now*/
	/* copy the address descriptor for IO and IO GEN of the source stage directly to destination stage*/
	pArmDstStage->sStageInput[ipIndex].StageIOBuffDescAddr = pArmDstStage->sStageOutput[opIndex].StageIOBuffDescAddr;
	pArmDstStage->sStageInput[ipIndex].StageIOGenericBuffDescAddr = pArmDstStage->sStageOutput[opIndex].StageIOGenericBuffDescAddr;

    pArmSrcStage->numOutputs[BDSP_ConnectionType_eStage]+=1;

    *pSourceOutputIndex = opIndex;
    pArmSrcStage->totalOutputs+=1;


    pArmDstStage->numInputs[BDSP_ConnectionType_eStage]+=1;

    *pDestinationInputIndex = ipIndex;
    pArmDstStage->totalInputs+=1;

    /*The Interstage buffers that are allocated during open time can be assigned to stages in the start time fw download during traversing
    or during the actual cit generation itself*/

    return(errCode);
}


/***********************************************************************
Name        :   BDSP_Arm_P_RemoveOutput

Type        :   PI Interface

Input       :   pStageHandle        -   Handle of the Stage for which output must be removed.
                outputIndex     -   The index of the Output in the Stage.

Return      :   None

Functionality   :   Following are the operations performed.
        1)  Make sure the Stage is not running, when attempt is made to remove the output.
        2)  Retrieve the Connection type used at the output.
        3)  If the connection type is Intertask task buffer, remove the source handle of the intertask buffer.
        4)  If destination handle of the intertask buffer is NULL, then inUse variable is set to false.
            Now the Intertask buffer can be destroyed by PI later.
        5)  Special handling is required if in watchdog recovery, Intertask buffer needs to be destroyed.
        6)  Disconnect the Output but resetting Stage Output structure.
        7)  If the Output is RAVE buffer then disconnection needs to be done for both CDB and ITB buffer.
***********************************************************************/

void BDSP_Arm_P_RemoveOutput(
    void *pStageHandle,
    unsigned outputIndex)
{
    BERR_Code err;
    BDSP_ArmStage *pArmStage = (BDSP_ArmStage *)pStageHandle;
    BDSP_ConnectionType connectionType;
    BDSP_MAP_Table_Entry MapTable[BDSP_ARM_MAX_ALLOC_STAGE];
    uint32_t ui32NumEntries = 0;

    BDBG_ENTER(BDSP_Arm_P_RemoveOutput);

    BDBG_OBJECT_ASSERT(pArmStage, BDSP_ArmStage);
    BDBG_ASSERT(outputIndex < BDSP_AF_P_MAX_OP_FORKS);

    connectionType = pArmStage->sStageOutput[outputIndex].eConnectionType;

    if (connectionType != BDSP_ConnectionType_eFmmBuffer)
    {
        BDBG_MSG(("WE DONT SUPPORT ANY OTHER OUTPUT PORT OTHER THAN FMM BUFFER"));
        return;
    }

    if (pArmStage->running)
    {
        BDBG_ERR(("Cannot remove outputs when the stage is running"));
        return;
    }

    /*Added Specially for ARM */
    BKNI_Memset(MapTable,0,(BDSP_ARM_MAX_ALLOC_STAGE*sizeof(BDSP_MAP_Table_Entry)));
    BDSP_Arm_P_RetrieveEntriesToUnMap(&(pArmStage->sStageMapTable[0]), &MapTable[0], &ui32NumEntries, BDSP_ARM_MAX_ALLOC_STAGE);
    err = BDSP_Arm_P_SendUnMapCommand(pArmStage->pContext->pDevice, &MapTable[0], ui32NumEntries);
    if (BERR_SUCCESS != err)
    {
        BDBG_ERR(("BDSP_Arm_P_RemoveOutput: Send ARM UNMAP Command failed!!!!"));
    }

    if(BDSP_ConnectionType_eFmmBuffer == connectionType)
    {
        int32_t i;
        for(i=0; i<(int32_t)pArmStage->sStageOutput[outputIndex].IoBuffer.ui32NumBuffers;i++)
        {
			BDSP_MMA_Memory Memory;
			/* We are trying to UNMAP the registers here, hence we pass the same in both physical and virtual Address */
			Memory.pAddr = (void *)((uintptr_t)(pArmStage->sStageOutput[outputIndex].IoBuffer.sCircBuffer[i].ui32ReadAddr|BCHP_PHYSICAL_OFFSET));
			Memory.offset = (pArmStage->sStageOutput[outputIndex].IoBuffer.sCircBuffer[i].ui32ReadAddr|BCHP_PHYSICAL_OFFSET);
			err = BDSP_Arm_P_DeleteEntry_MapTable(
				&(pArmStage->sStageMapTable[0]),
				&Memory,
				BDSP_ARM_MAX_ALLOC_STAGE);
            if (BERR_SUCCESS != err)
            {
                BDBG_ERR(("BDSP_Arm_P_RemoveOutput: Unable to delete entry form the Table for FMM register!!!!"));
            }

			/* The Memory we are trying to UNMAP, is actual Buffer which is allocated by APE/AIO for the FMM buffer. The address is physical and stored in the FMM register.
			Since Insert enrty table function expects both physical and virtual. The processing inside the function is based on virtual and at the end we pass the physical
			address to Firmware. We read the value and pass it as both physical and virtual address, so that Delete Entry function processes it correctly. */
			Memory.offset= BDSP_Read32(pArmStage->pContext->pDevice->regHandle, pArmStage->sStageOutput[outputIndex].IoBuffer.sCircBuffer[i].ui32BaseAddr);
			Memory.pAddr = (void *)((uintptr_t)BDSP_Read32(pArmStage->pContext->pDevice->regHandle, pArmStage->sStageOutput[outputIndex].IoBuffer.sCircBuffer[i].ui32BaseAddr));
			err = BDSP_Arm_P_DeleteEntry_MapTable(
				&(pArmStage->sStageMapTable[0]),
				&Memory,
				BDSP_ARM_MAX_ALLOC_STAGE);
            if (BERR_SUCCESS != err)
            {
                BDBG_ERR(("BDSP_Arm_P_RemoveOutput: Unable to delete entry form the Table for the actual buffer used for FMM!!!!"));
            }
        }
    }
#if 0
    else if(BDSP_ConnectionType_eInterTaskBuffer == connectionType)
    {
        BDSP_P_InterTaskBuffer *pArmInterTaskBuffer=NULL;
        pArmInterTaskBuffer = (BDSP_P_InterTaskBuffer *)pArmStage->sStageInput[outputIndex].connectionDetails.interTask.hInterTask->pInterTaskBufferHandle;
		err = BDSP_Arm_P_DeleteEntry_MapTable(
			&(pArmStage->sStageMapTable[0]),
			&pArmInterTaskBuffer->IoBuffer,
			BDSP_ARM_MAX_ALLOC_STAGE);
		if (BERR_SUCCESS != err)
		{
			BDBG_ERR(("BDSP_Arm_P_RemoveOutput: Unable to delete entry form the Table for Intertask IO buffer!!!!"));
		}

		err = BDSP_Arm_P_DeleteEntry_MapTable(
			&(pArmStage->sStageMapTable[0]),
			&pArmInterTaskBuffer->IoGenBuffer,
			BDSP_ARM_MAX_ALLOC_STAGE);
		if (BERR_SUCCESS != err)
		{
			BDBG_ERR(("BDSP_Arm_P_RemoveOutput: Unable to delete entry form the Table for Intertask IO GEN buffer!!!!"));
		}

		err = BDSP_Arm_P_DeleteEntry_MapTable(
			&(pArmStage->sStageMapTable[0]),
			&pArmInterTaskBuffer->IoBufferDesc,
			BDSP_ARM_MAX_ALLOC_STAGE);
		if (BERR_SUCCESS != err)
		{
			BDBG_ERR(("BDSP_Arm_P_RemoveOutput: Unable to delete entry form the Table for Intertask IO buffer Descriptor!!!!"));
		}

		err = BDSP_Arm_P_DeleteEntry_MapTable(
			&(pArmStage->sStageMapTable[0]),
			&pArmInterTaskBuffer->IoBufferGenericDesc,
			BDSP_ARM_MAX_ALLOC_STAGE);
		if (BERR_SUCCESS != err)
		{
			BDBG_ERR(("BDSP_Arm_P_RemoveOutput: Unable to delete entry form the Table for Intertask IO Generic buffer Descriptor!!!!"));
		}
    }
#endif

    /* Reset the last stage output structure */
    pArmStage->sStageOutput[outputIndex].eNodeValid = false;
    pArmStage->sStageOutput[outputIndex].eConnectionType = BDSP_ConnectionType_eMax;
    pArmStage->sStageOutput[outputIndex].connectionHandle = NULL;

    BKNI_Memset(&pArmStage->sStageOutput[outputIndex].connectionDetails, 0,
                sizeof(pArmStage->sStageOutput[outputIndex].connectionDetails));

    pArmStage->eStageOpBuffDataType[outputIndex] = BDSP_AF_P_DistinctOpType_eMax;
    pArmStage->numOutputs[connectionType] -= 1;
    pArmStage->totalOutputs--;

    BDBG_LEAVE(BDSP_Arm_P_RemoveOutput);
}


/***********************************************************************
Name        :   BDSP_Arm_P_RemoveAllOutputs

Type        :   PI Interface

Input       :   pStageHandle        -   Handle of the Stage for which all outputs must be removed.

Return      :   None

Functionality   :   Following are the operations performed.
        1)  Recursively remove the all the outputs connected to the stage.
        2)  Special handling is done for RAVE buffer as both the outputs CDB and ITB buffers are removed in one instance.
***********************************************************************/

void BDSP_Arm_P_RemoveAllOutputs(
    void *pStageHandle)
{
    BDSP_ArmStage *pArmStage = (BDSP_ArmStage *)pStageHandle;
    unsigned i, numOutputs;

    BDBG_ENTER(BDSP_Arm_P_RemoveAllOutputs);

    BDBG_OBJECT_ASSERT(pArmStage, BDSP_ArmStage);
    numOutputs = pArmStage->totalOutputs;

    for (i = 0; i < numOutputs; i++)
    {
        BDSP_Arm_P_RemoveOutput(pStageHandle, i);
    }

    BDBG_ASSERT(0 == pArmStage->totalOutputs);

    for (i = 0; i < BDSP_ConnectionType_eMax; i++)
    {
        BDBG_ASSERT(pArmStage->numOutputs[i] == 0);
    }

    BDBG_LEAVE(BDSP_Arm_P_RemoveAllOutputs);
}

/***********************************************************************
Name        :   BDSP_Arm_P_RemoveInput

Type        :   PI Interface

Input       :   pStageHandle        -   Handle of the Stage from which the input must be removed.
                inputIndex      -   The index of the input for the Stage.

Return      :   None

Functionality   :   Following are the operations performed.
        1)  Retrieve the Connection type used at the input.
        2)  Make sure the Stage is not running if the input is FMM or Intertask buffer, when attempt is made to remove the input.
        3)  If the Stage is running or in Wtachdog recovery, then CIT Reconfigure needs to be performed.
        4)  If the connection type is Intertask task buffer, remove the destination handle of the intertask buffer.
        5)  If source handle of the intertask buffer is NULL, then inUse variable is set to false.
            Now the Intertask buffer can be destroyed by PI later.
        6)  Special handling is required if in watchdog recovery, Intertask buffer needs to be destroyed.
        7)  Disconnect the Input but resetting Stage Input structure.
***********************************************************************/
void BDSP_Arm_P_RemoveInput(
    void *pStageHandle,
    unsigned inputIndex)
{
    BERR_Code err;
    BDSP_ArmStage *pArmStage = (BDSP_ArmStage *)pStageHandle;
    BDSP_ConnectionType connectionType;
    BDSP_P_InterTaskBuffer *pArmInterTaskBuffer=NULL;
    BDSP_AF_P_sIO_BUFFER *pIoBuffDesc_Cached = NULL;
    BDSP_MAP_Table_Entry MapTable[BDSP_ARM_MAX_ALLOC_STAGE];
    uint32_t ui32NumEntries = 0;

    BDBG_ENTER(BDSP_Arm_P_RemoveInput);

    BDBG_OBJECT_ASSERT(pArmStage, BDSP_ArmStage);
    BDBG_ASSERT(inputIndex < BDSP_AF_P_MAX_IP_FORKS);

    if ( pArmStage->totalInputs == 0 )
    {
        return;
    }

    connectionType = pArmStage->sStageInput[inputIndex].eConnectionType;

    if ((pArmStage->running)
        && ((connectionType != BDSP_ConnectionType_eInterTaskBuffer)
         && (connectionType != BDSP_ConnectionType_eFmmBuffer)))
    {
        BDBG_ERR(("Cannot remove inputs when the stage is running"));
        return;
    }

    /* Get the inter task buffer handle */

    /* Update the node config and send the cit reconfiguration command if the
            dest stage is running and Watchdog recovery is not underway */
    if ((pArmStage->running) && (!pArmStage->pContext->contextWatchdogFlag))
    {
        BDSP_ArmTask *pArmTask = pArmStage->pArmTask;
        BDSP_ARM_AF_P_sNODE_CONFIG  *psNodeConfig;
        BDSP_ARM_AF_P_sTASK_CONFIG  *psWorkingTaskCitBuffAddr_Cached = NULL;

        BDBG_OBJECT_ASSERT(pArmTask, BDSP_ArmTask);

        psWorkingTaskCitBuffAddr_Cached = (BDSP_ARM_AF_P_sTASK_CONFIG *)pArmTask->taskMemGrants.sSpareCitStruct.Buffer.pAddr;

        /* For inter task connections, the output from the source stage
        also needs to be removed. */
        if (connectionType == BDSP_ConnectionType_eInterTaskBuffer)
        {
            pArmInterTaskBuffer = (BDSP_P_InterTaskBuffer *)pArmStage->sStageInput[inputIndex].connectionDetails.interTask.hInterTask->pInterTaskBufferHandle;
            pArmInterTaskBuffer->dstHandle = NULL;
            pArmInterTaskBuffer->dstIndex  = -1;
			pIoBuffDesc_Cached = (BDSP_AF_P_sIO_BUFFER *)pArmInterTaskBuffer->IoBufferDesc.pAddr;

            /* SIPS is only supported if the Inter task buffer type is DRAM */
            if (pIoBuffDesc_Cached->eBufferType != BDSP_AF_P_BufferType_eDRAM)
            {
                BDBG_ERR(("Cannot remove input port seamlessly for non-dram type (%d) buffers",
                    pIoBuffDesc_Cached->eBufferType));
            }
        }
        psNodeConfig = &psWorkingTaskCitBuffAddr_Cached->sNodeConfig[0];
        /* Making the Input port invalid */
        psNodeConfig->eNodeIpValidFlag[inputIndex] = BDSP_AF_P_eInvalid;
        /* Setting the IO and IO Generic buffer structure addresses for the Node 0 */
        psNodeConfig->ui32NodeIpBuffCfgAddr[inputIndex] = 0;
        psNodeConfig->ui32NodeIpGenericDataBuffCfgAddr[inputIndex] = 0;
        if( psNodeConfig->ui32NumSrc != 0 )
        psNodeConfig->ui32NumSrc--;

        /* Setting the IO and IO Generic buffer structure addresses and enabling the Valid Flag for the Node 1 */
        psNodeConfig = &psWorkingTaskCitBuffAddr_Cached->sNodeConfig[BDSP_CIT_P_NUM_SPECIAL_NODES];
        psNodeConfig->ui32NodeIpBuffCfgAddr[inputIndex] = 0;
        psNodeConfig->ui32NodeIpGenericDataBuffCfgAddr[inputIndex] = 0;
        psNodeConfig->eNodeIpValidFlag[inputIndex] = BDSP_AF_P_eInvalid;
        if( psNodeConfig->ui32NumSrc != 0 )
        psNodeConfig->ui32NumSrc--;
        BDSP_MMA_P_FlushCache(pArmTask->taskMemGrants.sSpareCitStruct.Buffer, pArmTask->taskMemGrants.sSpareCitStruct.ui32Size);

        /* Send the cit re-configuration command to DSP */
        err = BDSP_Arm_P_SendCitReconfigCommand(pArmTask);

        if (err)
        {
            BDBG_ERR(("Failed to re-configure the cit after removing input"));
            return;
        }
    }
    /*Added Specially for ARM */
    BKNI_Memset(MapTable,0,(BDSP_ARM_MAX_ALLOC_STAGE*sizeof(BDSP_MAP_Table_Entry)));
    BDSP_Arm_P_RetrieveEntriesToUnMap(&(pArmStage->sStageMapTable[0]), &MapTable[0], &ui32NumEntries, BDSP_ARM_MAX_ALLOC_STAGE);
    err = BDSP_Arm_P_SendUnMapCommand(pArmStage->pContext->pDevice, &MapTable[0], ui32NumEntries);
    if (BERR_SUCCESS != err)
    {
        BDBG_ERR(("BDSP_Arm_P_RemoveInput: Send ARM UNMAP Command failed!!!!"));
    }

    if ((connectionType == BDSP_ConnectionType_eInterTaskBuffer))
    {
        pArmInterTaskBuffer = (BDSP_P_InterTaskBuffer *)pArmStage->sStageInput[inputIndex].connectionDetails.interTask.hInterTask->pInterTaskBufferHandle;
        pArmInterTaskBuffer->dstHandle = NULL;
        pArmInterTaskBuffer->dstIndex  = -1;
        if(NULL == pArmInterTaskBuffer->srcHandle)
        {
            /* Update inter task buffer structure */
            pArmInterTaskBuffer->inUse = false;
            pArmInterTaskBuffer->distinctOp = BDSP_AF_P_DistinctOpType_eMax;
        }
		BDSP_Arm_P_DeleteEntry_MapTable(&(pArmStage->sStageMapTable[0]), &pArmInterTaskBuffer->IoBufferDesc, BDSP_ARM_MAX_ALLOC_STAGE);
		BDSP_Arm_P_DeleteEntry_MapTable(&(pArmStage->sStageMapTable[0]), &pArmInterTaskBuffer->IoBufferGenericDesc, BDSP_ARM_MAX_ALLOC_STAGE);
		BDSP_Arm_P_DeleteEntry_MapTable(&(pArmStage->sStageMapTable[0]), &pArmInterTaskBuffer->IoBuffer, BDSP_ARM_MAX_ALLOC_STAGE);
		BDSP_Arm_P_DeleteEntry_MapTable(&(pArmStage->sStageMapTable[0]), &pArmInterTaskBuffer->IoGenBuffer, BDSP_ARM_MAX_ALLOC_STAGE);
    }

    /* Reset the stage input structure */
    pArmStage->sStageInput[inputIndex].eNodeValid = false;
    pArmStage->sStageInput[inputIndex].eConnectionType = BDSP_ConnectionType_eMax;
    pArmStage->sStageInput[inputIndex].connectionHandle = NULL;

    BKNI_Memset(&pArmStage->sStageInput[inputIndex].connectionDetails, 0,
                sizeof(pArmStage->sStageInput[inputIndex].connectionDetails));

    pArmStage->numInputs[connectionType] -= 1;
    pArmStage->totalInputs--;

    BDBG_LEAVE(BDSP_Arm_P_RemoveInput);
    return;
}

/***********************************************************************
Name        :   BDSP_Arm_P_RemoveAllInputs

Type        :   PI Interface

Input       :   pStageHandle        -   Handle of the Stage for which all inputs must be removed.

Return      :   None

Functionality   :   Following are the operations performed.
        1)  Recursively remove the all the inputs connected to the stage.
***********************************************************************/

void BDSP_Arm_P_RemoveAllInputs(
    void *pStageHandle)
{
    BDSP_ArmStage *pArmStage = (BDSP_ArmStage *)pStageHandle;
    unsigned i, numInputs;

    BDBG_ENTER(BDSP_Arm_P_RemoveAllInputs);

    BDBG_OBJECT_ASSERT(pArmStage, BDSP_ArmStage);
    numInputs = pArmStage->totalInputs;

    for (i = 0; i < numInputs; i++)
    {
        BDSP_Arm_P_RemoveInput(pStageHandle, i);
    }

    BDBG_ASSERT(0 == pArmStage->totalInputs);

    for (i = 0; i < BDSP_ConnectionType_eMax; i++)
    {
        BDBG_ASSERT(pArmStage->numInputs[i] == 0);
    }

    BDBG_LEAVE(BDSP_Arm_P_RemoveAllInputs);
    return;
}

/***********************************************************************
Name        :   BDSP_Arm_P_GetDefaultStageCreateSettings

Type        :   PI Interface

Input       :   algoType        -   The algorithm type of the stage for which default settings is requested.
                pSettings       -   Stage creating settings pointer where the default settings is filled and returned to PI.

Return      :   None

Functionality   :   Following are the operations performed.
        1)  Validate if the algorithm type is supported or not.
        2)  Fill the maximum inputs and ouputs supported for the algorithm type requested.
***********************************************************************/

void BDSP_Arm_P_GetDefaultStageCreateSettings(
    BDSP_AlgorithmType algoType,
    BDSP_StageCreateSettings *pSettings /* [out] */
    )
{
    int i;
    const BDSP_Arm_P_AlgorithmInfo *pAlgoInfo;

    BDBG_ENTER(BDSP_Arm_P_GetDefaultStageCreateSettings);

    BDBG_ASSERT(NULL != pSettings);
    BDBG_ASSERT(algoType < BDSP_AlgorithmType_eMax);

    pSettings->algoType = algoType;

    for (i = 0; i < BDSP_Algorithm_eMax; i++)
    {
        pAlgoInfo = BDSP_Arm_P_LookupAlgorithmInfo(i);

        if (pSettings->algoType == pAlgoInfo->type)
        {
            pSettings->algorithmSupported[i] = pAlgoInfo->supported;
        }
        else
        {
            pSettings->algorithmSupported[i] = false;
        }
    }

    switch (algoType)
    {
        case BDSP_AlgorithmType_eAudioDecode:
            pSettings->maxInputs = 1;
            pSettings->maxOutputs = 3;
            break;
        case BDSP_AlgorithmType_eAudioPassthrough:  /* Enable all supported audio passthrough codecs by default. Max inputs = 2 */
            BDBG_ERR(("Audio Passthru not Supported on ARM!!!!."));
            break;
        case BDSP_AlgorithmType_eAudioEncode: /* Enable all supported audio encode codecs by default. Max inputs = 1 */
            pSettings->maxInputs = 1;
            pSettings->maxOutputs = 1;
            break;
        case BDSP_AlgorithmType_eAudioMixer: /* Enable mixer algorithm by default.  Max inputs = 3 */
            BDBG_ERR(("Audio Mixixng not Supported on ARM...Inspect your Create Seetings for Stage!!!!."));
            BDBG_ASSERT(0);
            break;
        case BDSP_AlgorithmType_eAudioEchoCanceller: /* Enable all echo cancellation algorithms by default.  Max inputs = 2 */
            BDBG_ERR(("Audio Echocanceller not Supported on ARM...Inspect your Create Seetings for Stage!!!!."));
            BDBG_ASSERT(0);
            break;
        case BDSP_AlgorithmType_eAudioProcessing: /* Nothing enabled by default.  Max inputs = 1 */
            BDBG_ERR(("Audio PostProcess not Supported on ARM...Inspect your Create Seetings for Stage!!!!."));
            BDBG_ASSERT(0);
            break;
        case BDSP_AlgorithmType_eVideoDecode:
            BDBG_ERR(("Video Decode not Supported on ARM...Inspect your Create Seetings for Stage!!!!."));
            BDBG_ASSERT(0);
            break;
        case BDSP_AlgorithmType_eVideoEncode:
            BDBG_ERR(("Video Encode not Supported on ARM...Inspect your Create Seetings for Stage!!!!."));
            BDBG_ASSERT(0);
            break;
        case BDSP_AlgorithmType_eSecurity:
            BDBG_ERR(("Security not Supported on ARM...Inspect your Create Seetings for Stage!!!!."));
            BDBG_ASSERT(0);
            break;
        case BDSP_AlgorithmType_eCustom:
            BDBG_ASSERT(0);
            BDBG_ERR(("Custom not Supported on ARM...Inspect your Create Seetings for Stage!!!!."));
        default:
            /* Nothing to be done because the assert at start will ensure the default case is never hit */
            break;
    }
    BDBG_LEAVE(BDSP_Arm_P_GetDefaultStageCreateSettings);
}

/***********************************************************************
Name        :   BDSP_Arm_P_CreateStage

Type        :   PI Interface

Input       :   pContextHandle  -   Handle of the Context for which the stage needs to created.
                pSettings           -   Settings of the Stage to be created.
                pStageHandle        -   Handle of the Stage which is returned back to PI.

Return      :   Error Code to return SUCCESS or FAILURE

Functionality   :   Following are the operations performed.
        1)  Allocate and Intialise the memory for the Stage Descriptor.
        2)  Allocate the memory required for the Stage.
        3)  Intialise all the function pointers which will be used by the PI for further processing.
        4)  Set the Algorithm for the Stage.
        5)  Perform Error Handling if any required.
***********************************************************************/
BERR_Code BDSP_Arm_P_CreateStage(
    void *pContextHandle,
    const BDSP_StageCreateSettings *pSettings,
    BDSP_StageHandle *pStageHandle /* [out] */
    )
{
    BERR_Code errCode = BERR_SUCCESS;
    BDSP_ArmStage *pStage;
    unsigned i;

    BDBG_ENTER(BDSP_Arm_P_CreateStage);
    pStage = BKNI_Malloc(sizeof(BDSP_ArmStage));
    if ( NULL == pStage )
    {
        errCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto err_malloc_stage;
    }

    BKNI_Memset(pStage, 0, sizeof(*pStage));
    BDSP_P_InitStage(&pStage->stage, pStage);

    pStage->algorithm = BDSP_Algorithm_eMax;
    pStage->settings = *pSettings;
    pStage->pContext = pContextHandle;
    BDBG_OBJECT_SET(pStage, BDSP_ArmStage);

    errCode = BDSP_Arm_P_AllocateStageMemory((void *)pStage);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_stage_memory;
    }

    pStage->running  = false;
    pStage->pArmTask = NULL;

    pStage->totalInputs = 0;
    pStage->totalOutputs = 0;
    for (i = 0; i < BDSP_ConnectionType_eMax; i++)
    {
        pStage->numInputs[i] = 0;
        pStage->numOutputs[i] = 0;
    }

    /* Initialize the stage apis */
    pStage->stage.destroy = BDSP_Arm_P_DestroyStage;
    pStage->stage.setAlgorithm = BDSP_Arm_P_SetAlgorithm;
    pStage->stage.getStageSettings = BDSP_Arm_P_GetStageSettings;
    pStage->stage.setStageSettings = BDSP_Arm_P_SetStageSettings;

    pStage->stage.getStageStatus = NULL; /*BDSP_Arm_P_GetStageStatus;*/
    pStage->stage.getTsmSettings_isr = NULL; /*BDSP_Arm_P_GetTsmSettings_isr;*/
    pStage->stage.setTsmSettings_isr = NULL; /*BDSP_Arm_P_SetTsmSettings_isr;*/
    pStage->stage.getTsmStatus_isr = NULL; /*BDSP_Arm_P_GetTsmStatus_isr;*/

    pStage->stage.getDatasyncSettings = BDSP_Arm_P_GetDatasyncSettings;
    pStage->stage.setDatasyncSettings = BDSP_Arm_P_SetDatasyncSettings;

    pStage->stage.getDatasyncStatus_isr = NULL; /*BDSP_Arm_P_GetDatasyncStatus_isr;*/

    pStage->stage.addFmmOutput = BDSP_Arm_P_AddFmmOutput;
    pStage->stage.addRaveOutput = NULL;
    pStage->stage.addOutputStage = BDSP_Arm_P_AddOutputStage;
#if !B_REFSW_MINIMAL
    pStage->stage.removeOutput = BDSP_Arm_P_RemoveOutput;
#endif /*!B_REFSW_MINIMAL*/
    pStage->stage.removeAllOutputs = BDSP_Arm_P_RemoveAllOutputs;
    pStage->stage.addFmmInput = NULL;
    pStage->stage.addRaveInput = NULL;
    pStage->stage.removeInput = BDSP_Arm_P_RemoveInput;
    pStage->stage.removeAllInputs = BDSP_Arm_P_RemoveAllInputs;

    pStage->stage.addInterTaskBufferInput = BDSP_Arm_P_AddInterTaskBufferInput;
    pStage->stage.addInterTaskBufferOutput = BDSP_Arm_P_AddInterTaskBufferOutput;

    pStage->stage.addSoftFmmOutput = NULL;
    pStage->stage.addSoftFmmInput = NULL;


#if !B_REFSW_MINIMAL
    pStage->stage.addQueueInput = NULL;
#endif /*!B_REFSW_MINIMAL*/
#ifdef BDSP_QUEUE_DEBUG
    pStage->stage.addQueueOutput = BDSP_Arm_P_AddQueueOutput;
#endif /* BDSP_QUEUE_DEBUG */
	pStage->stage.getStageContext = BDSP_Arm_P_StageGetContext;

    errCode = BERR_INVALID_PARAMETER;
    for ( i = 0; i < BDSP_Algorithm_eMax; i++ )
    {
        if (pSettings->algorithmSupported[i])
        {
            errCode = BDSP_Arm_P_SetAlgorithm(pStage, i);
            break;
        }
    }

    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_algorithm;
    }

    /* Sucess, assign the handle. This should be done just before we exit with success */
    *pStageHandle = &pStage->stage;
    errCode= BERR_SUCCESS;
    goto end;

err_algorithm:
    BDSP_Arm_P_FreeStageMemory(pStage);
    BDBG_OBJECT_DESTROY(pStage, BDSP_ArmStage);
err_stage_memory:
    BKNI_Free(pStage);
err_malloc_stage:
end:
    BDBG_LEAVE(BDSP_Arm_P_CreateStage);
    return errCode;
}

/***********************************************************************
Name        :   BDSP_Arm_P_DestroyStage

Type        :   PI Interface

Input       :   pStageHandle        -   Handle of the Stage which needs to be destroyed.

Return      :   None

Functionality   :   Following are the operations performed.
        1)  Free all the memory was allocated/associated with the Stage.
        2)  Free the Stage Descriptor memory.
***********************************************************************/

void BDSP_Arm_P_DestroyStage(
    void *pStageHandle
    )
{
    BERR_Code err = BERR_SUCCESS;
    BDSP_ArmStage *pArmStage = (BDSP_ArmStage *)pStageHandle;
    BDSP_MAP_Table_Entry MapTable[BDSP_ARM_MAX_ALLOC_STAGE];
    uint32_t ui32NumEntries = 0;

    BDBG_ENTER(BDSP_Arm_P_DestroyStage);

    BKNI_Memset(MapTable,0,(BDSP_ARM_MAX_ALLOC_STAGE*sizeof(BDSP_MAP_Table_Entry)));
    BDSP_Arm_P_RetrieveEntriesToUnMap(&(pArmStage->sStageMapTable[0]), &MapTable[0], &ui32NumEntries, BDSP_ARM_MAX_ALLOC_STAGE);
    err = BDSP_Arm_P_SendUnMapCommand(pArmStage->pContext->pDevice, &MapTable[0], ui32NumEntries);
    if (BERR_SUCCESS != err)
    {
        BDBG_ERR(("BDSP_Arm_P_DestroyStage: Send ARM UNMAP Command failed!!!!"));
    }

	{
		BDSP_ArmCapture *pArmCapture;
		for (pArmCapture = BLST_S_FIRST(&pArmStage->captureList);
		pArmCapture != NULL;
		pArmCapture = BLST_S_NEXT(pArmCapture, node) )
		{
			BLST_S_REMOVE(&pArmStage->captureList, pArmCapture, BDSP_ArmCapture, node);
			pArmCapture->stageDestroyed = true;
		}
	}

    BDSP_Arm_P_FreeStageMemory(pStageHandle);

    BDBG_OBJECT_DESTROY(pArmStage, BDSP_ArmStage);
    BKNI_Free(pArmStage);

    BDBG_LEAVE(BDSP_Arm_P_DestroyStage);
}


/***********************************************************************
Name        :   BDSP_Arm_P_GetStageSettings

Type        :   PI Interface

Input       :   pStageHandle        -   Handle of the Stage whose settings needs to be retrieved.
                pSettingsBuffer -   Pointer to the memory where the Stage Settings are filled.
                settingsSize        -   Size of the Settings buffer provided by PI.

Return      :   Error Code to return SUCCESS or FAILURE

Functionality   :   Following are the operations performed.
        1)  Retrieve the Algorithm Settings of the Stage using the BDSP_Arm_P_GetStageSettings call.
***********************************************************************/

BERR_Code BDSP_Arm_P_GetStageSettings(
    void *pStageHandle,
    void *pSettingsBuffer,
    size_t settingsSize)
{
    BDSP_ArmStage *pArmStage = (BDSP_ArmStage *)pStageHandle;
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(pArmStage, BDSP_ArmStage);
    BDBG_ASSERT(pSettingsBuffer);

    BDBG_ASSERT(pArmStage->algorithm < BDSP_Algorithm_eMax);
    errCode = BDSP_Arm_P_GetAlgorithmSettings(
        pArmStage->algorithm,
        &pArmStage->sDramUserConfigBuffer.Buffer,
        /* Since the buffer is shared between ids and decode stage, the
        buffer size available for the decode stage is the offset to ids buffer */
        pArmStage->sFrameSyncOffset.ui32UserCfgOffset,
        pSettingsBuffer,
        settingsSize);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    return BERR_SUCCESS;
}

/***********************************************************************
Name        :   BDSP_Arm_P_SetStageSettings

Type        :   PI Interface

Input       :   pStageHandle        -   Handle of the Stage for which settings need to be applied.
                pSettingsBuffer -   Pointer to the memory where the settings are provided by the PI .
                settingsSize        -   Size of the Settings buffer provided by PI.

Return      :   Error Code to return SUCCESS or FAILURE

Functionality   :   Following are the operations performed.
        1)  Set the Algorithm settings for the stage.
        2)  If the Stage is already running then use spare config buffer else config buffer.
        3)  If the Satge is already running then, CFG change command is issued to apply the settings.
***********************************************************************/

BERR_Code BDSP_Arm_P_SetStageSettings(
    void *pStageHandle,
    const void *pSettingsBuffer,
    size_t settingsSize)
{
    BDSP_ArmStage *pArmStage;
    BDSP_Arm  *pDevice;
    BERR_Code err = BERR_SUCCESS;
    unsigned int  uiConfigBufAddr =0;
    unsigned int  uiConfigBufSize;
    BDSP_Arm_P_Command *psCommand;
    BDSP_Arm_P_Response sRsp;
    BDSP_P_MsgType      eMsgType;

    pArmStage = (BDSP_ArmStage *)pStageHandle;
    pDevice = pArmStage->pContext->pDevice;

    BDBG_OBJECT_ASSERT(pArmStage, BDSP_ArmStage);
    BDBG_ASSERT(pSettingsBuffer);

    BDBG_ASSERT(pArmStage->algorithm < BDSP_Algorithm_eMax);

    psCommand = BKNI_Malloc(sizeof(BDSP_Arm_P_Command));
    if ( NULL == psCommand )
    {
        err = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto err_malloc_command;
    }

    BKNI_Memset(psCommand, 0, sizeof(*psCommand));
    if (pArmStage->running)
    {
        uiConfigBufAddr = pArmStage->sDramUserConfigSpareBuffer.Buffer.offset;
        uiConfigBufSize = pArmStage->sFrameSyncOffset.ui32UserCfgOffset;
        err = BDSP_Arm_P_SetAlgorithmSettings(
            pArmStage->algorithm,
            &pArmStage->sDramUserConfigSpareBuffer.Buffer,
            uiConfigBufSize,
            pSettingsBuffer,
            settingsSize);
    }
    else
    {
        uiConfigBufAddr = pArmStage->sDramUserConfigBuffer.Buffer.offset;
        /* Since the buffer is shared between ids and decode stage, the
        buffer size availabel for the decode stage is the offset to ids buffer */
        uiConfigBufSize = pArmStage->sFrameSyncOffset.ui32UserCfgOffset;
        err = BDSP_Arm_P_SetAlgorithmSettings(
            pArmStage->algorithm,
            &pArmStage->sDramUserConfigBuffer.Buffer,
            uiConfigBufSize,
            pSettingsBuffer,
            settingsSize);
    }
    if ( err )
    {
        err = BERR_TRACE(err);
        goto end;
    }

    if (pArmStage->running)
    {

        /*Send CFG Change Command to FW*/
        psCommand->sCommandHeader.ui32CommandID = BDSP_ALGO_PARAMS_CFG_COMMAND_ID;
        psCommand->sCommandHeader.ui32CommandCounter = pArmStage->pArmTask->commandCounter++;
        psCommand->sCommandHeader.ui32TaskID = pArmStage->pArmTask->taskId;
        psCommand->sCommandHeader.eResponseType = BDSP_P_ResponseType_eResponseRequired;
        psCommand->sCommandHeader.ui32CommandSizeInBytes =  sizeof(BDSP_Arm_P_Command);

        psCommand->uCommand.sCfgChange.ui32HostConfigParamBuffAddr = uiConfigBufAddr;
		psCommand->uCommand.sCfgChange.ui32DspConfigParamBuffAddr = pArmStage->sDramUserConfigBuffer.Buffer.offset;
        psCommand->uCommand.sCfgChange.ui32SizeInBytes = settingsSize;

        pArmStage->pArmTask->lastEventType = psCommand->sCommandHeader.ui32CommandID;
        BKNI_ResetEvent(pArmStage->pArmTask->hEvent);
        err = BDSP_Arm_P_SendCommand(pDevice->hCmdQueue, psCommand,(void *)pArmStage->pArmTask);

        if (BERR_SUCCESS != err)
        {
            BDBG_ERR(("BDSP_Arm_P_SetStageSettings: CFG_Command failed!"));
            err = BERR_TRACE(err);
            goto end;
        }
        /* Wait for Ack_Response_Received event w/ timeout */
        err = BKNI_WaitForEvent(pArmStage->pArmTask->hEvent, BDSP_ARM_EVENT_TIMEOUT_IN_MS);
        if (BERR_TIMEOUT == err)
        {
            BDBG_ERR(("BDSP_Arm_P_SetStageSettings: CFG_Command TIMEOUT!"));
            err = BERR_TRACE(err);
            goto end;
        }
        BSTD_UNUSED(eMsgType);
        BSTD_UNUSED(sRsp);
    }
end:
    BKNI_Free(psCommand);
err_malloc_command:
    return err;
}

/***********************************************************************
Name        :   BDSP_Arm_P_GetDatasyncSettings

Type        :   PI Interface

Input       :   pStageHandle        -   Handle of the Stage for getting data sync settings.
                pSettings           -   Pointer of memory where the data is filled.

Return      :   Error Code to return SUCCESS or FAILURE

Functionality   :   This is a leaf function, which will internally call the BDSP_Arm_P_GetDatasyncSettings ISR call.
***********************************************************************/

BERR_Code BDSP_Arm_P_GetDatasyncSettings(
    void *pStageHandle,
    BDSP_AudioTaskDatasyncSettings *pSettings)
{
    BERR_Code err = BERR_SUCCESS;
    BDBG_ASSERT(pStageHandle);
    BDBG_ASSERT(pSettings);

    BKNI_EnterCriticalSection();
    err =BDSP_Arm_P_GetDatasyncSettings_isr(pStageHandle, pSettings);
    BKNI_LeaveCriticalSection();
    return err;
}

/***********************************************************************
Name        :   BDSP_Arm_P_SetDatasyncSettings

Type        :   PI Interface

Input       :   pStageHandle        -   Handle of the Stage for setting data sync settings.
                pSettings           -   Pointer of data to be Set

Return      :   Error Code to return SUCCESS or FAILURE

Functionality   :   This is a leaf function, which will internally call the BDSP_Arm_P_SetDatasyncSettings ISR call.
***********************************************************************/

BERR_Code BDSP_Arm_P_SetDatasyncSettings(
    void *pStageHandle,
    const BDSP_AudioTaskDatasyncSettings *pSettings)
{
    BERR_Code err=BERR_SUCCESS;
    BDBG_ASSERT(pStageHandle);
    BDBG_ASSERT(pSettings);

    BKNI_EnterCriticalSection();
    err =BDSP_Arm_P_SetDatasyncSettings_isr(pStageHandle, pSettings);
    BKNI_LeaveCriticalSection();
    return err;
}

/***********************************************************************
Name        :   BDSP_Arm_P_GetDatasyncSettings_isr
Type        :   ISR
Input       :   pStageHandle        -   Handle of the Stage for which Data Sync settings are requested.
                pSettings           -   Pointer to which the data is filled and returned.
Return      :   Error Code to return SUCCESS or FAILURE
Functionality   :   Following are the operations performed.
        1)  Retreive the Frame Sync and TSM settings using the BDSP_Arm_P_GetFrameSyncTsmStageConfigParams_isr" call.
        2)  Note the information to be retrieved is from HOST/Spare Config buffer and not DSP/Config buffer to avoid RACE condition.
        3)  Return only the Audio Data Sync Settings back to the PI.
Design Note:
        a)  Inside "BDSP_Arm_P_SetDatasyncSettings_isr", if the stage is running, then the data is copied from
            HOST to DSP buffer by the FW. HOST needs to only modify the HOST/Spare buffer only.
        b)  Inside "BDSP_Arm_P_SetDatasyncSettings_isr", if the Stage is not running, then the data is Copied
        both into HOST/DSP buffer. This is required for consistency of the data.
        c)  The "BDSP_Arm_P_GetDatasyncSettings_isr" will retrieve the information from HOST
        buffer only.
***********************************************************************/

BERR_Code BDSP_Arm_P_GetDatasyncSettings_isr(
    void *pStageHandle,
    BDSP_AudioTaskDatasyncSettings *pSettings)
{
    BERR_Code       err = BERR_SUCCESS;
    uint32_t        ui32FsUserCfgSize;
	BDSP_MMA_Memory UserCfgMemory;
    BDSP_ArmStage *pArmStage = (BDSP_ArmStage *)pStageHandle;
    BDSP_P_Audio_FrameSyncTsmConfigParams  sFrameSyncTsmConfigParams;

    BDBG_OBJECT_ASSERT(pArmStage, BDSP_ArmStage);
    BDBG_ASSERT(pSettings);

	UserCfgMemory = pArmStage->sDramUserConfigSpareBuffer.Buffer;
	UserCfgMemory.pAddr = (void *)((uint8_t *)UserCfgMemory.pAddr +
						pArmStage->sFrameSyncOffset.ui32UserCfgOffset);
	UserCfgMemory.offset = UserCfgMemory.offset +
						pArmStage->sFrameSyncOffset.ui32UserCfgOffset;
	ui32FsUserCfgSize = pArmStage->sDramUserConfigSpareBuffer.ui32Size -
						pArmStage->sFrameSyncOffset.ui32UserCfgOffset;

	BDSP_Arm_P_GetFrameSyncTsmStageConfigParams_isr(
		pArmStage->algorithm, &UserCfgMemory, ui32FsUserCfgSize,
		&sFrameSyncTsmConfigParams, sizeof(sFrameSyncTsmConfigParams));

	BKNI_Memcpy((void *)(volatile void*)pSettings,
        (void *)&sFrameSyncTsmConfigParams.sFrameSyncConfigParams,
        sizeof(BDSP_AudioTaskDatasyncSettings));

    return err;
}

/***********************************************************************
Name        :   BDSP_Arm_P_SetDatasyncSettings_isr

Type        :   ISR

Input       :   pStageHandle        -   Handle of the Stage for which Data Sync settings needs to be Set.
                pSettings           -   Pointer of the Settings provided by PI.

Return      :   Error Code to return SUCCESS or FAILURE

Functionality   :   Following are the operations performed.
        1)  Retrieve the whole Frame Sync information from HOST/Spare buffer  into a local buffer using
            "BDSP_Arm_P_GetFrameSyncTsmStageConfigParams_isr".
        2)  Update the Data Sync settings only into the local buffer.
        3)  If the Stage is Stopped, then update the both DSP/Config buffer and HOST/Spare buffer else
            update only the HOST/Spare buffer only.
        4)  If the Stage is running then send the Config change command and don't wait for response.

 Design Note:
    a)  Inside "BDSP_Arm_P_SetDatasyncSettings_isr", if the stage is running, then the data is copied from
            HOST to DSP buffer by the FW. HOST needs to only modify the HOST/Spare buffer only.
        b)  Inside "BDSP_Arm_P_SetDatasyncSettings_isr", if the Stage is not running, then the data is Copied
        both into HOST/DSP buffer. This is required for consistency of the data.
        c)  The "BDSP_Arm_P_GetDatasyncSettings_isr" will retrieve the information from HOST
        buffer only.
***********************************************************************/

BERR_Code BDSP_Arm_P_SetDatasyncSettings_isr(
    void *pStageHandle,
    const BDSP_AudioTaskDatasyncSettings *pSettings)
{
    BDSP_ArmStage *pArmStage;
    BDSP_Arm  *pDevice;
    BERR_Code err = BERR_SUCCESS;
	BDSP_MMA_Memory ConfigBuf;
    dramaddr_t      ui32ConfigBufAddr;
    unsigned int    uiConfigBufSize=0;
    BDSP_Arm_P_Command *psCommand;
    BDSP_P_Audio_FrameSyncTsmConfigParams   sFrameSyncTsmConfigParams;

	BDSP_MMA_Memory TsmUserCfg;
    dramaddr_t      TsmBufAddr;
    uint32_t        ui32FsTsmUserCfgSize;

    pArmStage = (BDSP_ArmStage *)pStageHandle;
    pDevice = pArmStage->pContext->pDevice;

    BDBG_OBJECT_ASSERT(pArmStage, BDSP_ArmStage);
    BDBG_ASSERT(pSettings);

    psCommand = BKNI_Malloc(sizeof(BDSP_Arm_P_Command));
    if ( NULL == psCommand )
    {
        err = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto err_malloc_command;
    }

    BKNI_Memset(psCommand, 0, sizeof(*psCommand));

	/*HOST Buffer details */
	ConfigBuf = pArmStage->sDramUserConfigSpareBuffer.Buffer;
	ConfigBuf.pAddr = (void *)((uint8_t *)ConfigBuf.pAddr+ pArmStage->sFrameSyncOffset.ui32UserCfgOffset);
	ConfigBuf.offset= ConfigBuf.offset + pArmStage->sFrameSyncOffset.ui32UserCfgOffset;
	uiConfigBufSize = pArmStage->sDramUserConfigSpareBuffer.ui32Size - pArmStage->sFrameSyncOffset.ui32UserCfgOffset;

	/*DSP Buffer details */
	TsmUserCfg = pArmStage->sDramUserConfigBuffer.Buffer;
	TsmUserCfg.pAddr = (void *)((uint8_t *)TsmUserCfg.pAddr+ pArmStage->sFrameSyncOffset.ui32UserCfgOffset);
	TsmUserCfg.offset= TsmUserCfg.offset + pArmStage->sFrameSyncOffset.ui32UserCfgOffset;
	ui32FsTsmUserCfgSize = pArmStage->sDramUserConfigBuffer.ui32Size - pArmStage->sFrameSyncOffset.ui32UserCfgOffset;

	BDSP_Arm_P_GetFrameSyncTsmStageConfigParams_isr(
		pArmStage->algorithm,
		&ConfigBuf, uiConfigBufSize,
		&sFrameSyncTsmConfigParams, sizeof(sFrameSyncTsmConfigParams));

    BKNI_Memcpy((void *)(volatile void*)&(sFrameSyncTsmConfigParams.sFrameSyncConfigParams) ,(void *)pSettings ,sizeof(BDSP_AudioTaskDatasyncSettings));

    if ( !pArmStage->running )
    {
		/* If the Task is not running then write the data into DSP buffer too */
		BDSP_Arm_P_SetFrameSyncTsmStageConfigParams_isr(
			pArmStage->algorithm,
			&TsmUserCfg, ui32FsTsmUserCfgSize,
			&sFrameSyncTsmConfigParams, sizeof(sFrameSyncTsmConfigParams));
    }
	/*Always write the data into HOST buffer*/
	BDSP_Arm_P_SetFrameSyncTsmStageConfigParams_isr(
		pArmStage->algorithm,
		&ConfigBuf, uiConfigBufSize,
		&sFrameSyncTsmConfigParams, sizeof(sFrameSyncTsmConfigParams));
    if (pArmStage->running)
    {
		ui32ConfigBufAddr = ConfigBuf.offset;
		TsmBufAddr = TsmUserCfg.offset;

        /*Send CFG Change Command to FW*/
        psCommand->sCommandHeader.ui32CommandID = BDSP_ALGO_PARAMS_CFG_COMMAND_ID;
        psCommand->sCommandHeader.ui32CommandCounter = pArmStage->pArmTask->commandCounter++;
        psCommand->sCommandHeader.ui32TaskID = pArmStage->pArmTask->taskId;
        psCommand->sCommandHeader.eResponseType = BDSP_P_ResponseType_eNone;
        psCommand->sCommandHeader.ui32CommandSizeInBytes =  sizeof(BDSP_Arm_P_Command);
        psCommand->uCommand.sCfgChange.ui32HostConfigParamBuffAddr = ui32ConfigBufAddr;
        psCommand->uCommand.sCfgChange.ui32DspConfigParamBuffAddr = TsmBufAddr;
        psCommand->uCommand.sCfgChange.ui32SizeInBytes = sizeof(BDSP_P_Audio_FrameSyncTsmConfigParams);

        pArmStage->pArmTask->lastEventType = psCommand->sCommandHeader.ui32CommandID;
        err = BDSP_Arm_P_SendCommand_isr(pDevice->hCmdQueue, psCommand,(void *)pArmStage->pArmTask);

        if (BERR_SUCCESS != err)
        {
            BDBG_ERR(("BDSP_Arm_P_SetStageSettings: CFG_Command failed!"));
            err = BERR_TRACE(err);
            goto end;
        }
    }
end:
    BKNI_Free(psCommand);
err_malloc_command:
    return err;
}

static BERR_Code BDSP_Arm_P_InitInterframeBuffer(void *pStageHandle)
{
    BERR_Code   rc = BERR_SUCCESS;

    BDSP_ArmStage *pArmStage;
    BDSP_Arm      *pDevice;
    void *pIfAddr;
    uint32_t ui32IfBuffSize;
    BDSP_ARM_AF_P_AlgoId algoId;
    void *pIfAddr_Cached = NULL;
    /*void *pMemory_Cached = NULL;*/
    /*uint32_t ui32ImgId;*/
    const BDSP_Arm_P_AlgorithmInfo *pAlgoInfo;

    /*uint32_t ui32IfBuffEncodedSize = 0;*/
    BDBG_ENTER(BDSP_Arm_P_InitInterframeBuffer);

    BDBG_ASSERT(NULL != pStageHandle);
    pArmStage = (BDSP_ArmStage *)pStageHandle;

    BDSP_ARM_STAGE_TRAVERSE_LOOP_BEGIN(pArmStage, pStageIterator)
    BSTD_UNUSED(macroStId);
    BSTD_UNUSED(macroBrId);
    {
        pAlgoInfo = BDSP_Arm_P_LookupAlgorithmInfo(pStageIterator->algorithm);
        /* Load the interframe for decoder */
        algoId = pAlgoInfo->algoExecInfo.eAlgoIds[1];
        pIfAddr = pStageIterator->sDramInterFrameBuffer.Buffer.pAddr;
        ui32IfBuffSize = pStageIterator->sDramInterFrameBuffer.ui32Size;
        if (algoId == BDSP_ARM_AF_P_AlgoId_eInvalid)
        {
            BDBG_ERR(("You are here as there was a problem in the connection handles between stages. "));
            BDBG_ERR(("The Algo Id is invalid for this stage algo %d", pStageIterator->algorithm));
            BDBG_ASSERT(0);
        }
#if 0  /*SR_TBD: Enabled when Interframe download is enabled*/
        pDevice = (BDSP_Arm *)pArmStage->pContext->pDevice;
        ui32ImgId = BDSP_ARM_IMG_ID_IFRAME(algoId);

        pIfAddr_Cached = pIfAddr;
        pMemory_Cached = pDevice->imgCache[ui32ImgId].pMemory ;
        ui32IfBuffEncodedSize = pDevice->imgCache[ui32ImgId].size; /* This is size of the run length encoded inter-frame array. Actual size is calculated below */

        rc = BDSP_DSP_P_InterframeRunLengthDecode(pMemory_Cached, pIfAddr_Cached, ui32IfBuffEncodedSize, ui32IfBuffSize);
        if (rc != BERR_SUCCESS)
        {
            BDBG_ERR(("Error in decoding the inter-frame array for algoid=%d", algoId));;
            return BERR_TRACE(rc);
        }
#else
        pIfAddr_Cached = pIfAddr;
        BKNI_Memset((void *)(volatile void*)pIfAddr_Cached, 0, ui32IfBuffSize);
        BSTD_UNUSED(pDevice);
#endif /*SR_TBD: Enabled when Interframe download is enabled*/
        BDSP_MMA_P_FlushCache(pStageIterator->sDramInterFrameBuffer.Buffer, ui32IfBuffSize);

        /* Load the interframe for datasync */
        algoId = pAlgoInfo->algoExecInfo.eAlgoIds[0];
        if (algoId != BDSP_ARM_AF_P_AlgoId_eInvalid)
        {
            pIfAddr = pStageIterator->sDramInterFrameBuffer.Buffer.pAddr;
            ui32IfBuffSize = pStageIterator->sDramInterFrameBuffer.ui32Size;
#if 0  /*SR_TBD: Enabled when Interframe download is enabled*/
            ui32ImgId = BDSP_ARM_IMG_ID_IFRAME(algoId);

            pIfAddr_Cached = pIfAddr ;
            pMemory_Cached = pDevice->imgCache[ui32ImgId].pMemory;
            ui32IfBuffEncodedSize = pDevice->imgCache[ui32ImgId].size; /* This is size of the run length encoded inter-frame array. Actual size is calculated below */

            rc = BDSP_DSP_P_InterframeRunLengthDecode(pMemory_Cached, pIfAddr_Cached, ui32IfBuffEncodedSize, ui32IfBuffSize);
            if (rc != BERR_SUCCESS)
            {
                BDBG_ERR(("Error in decoding the inter-frame array for algoid=%d", algoId));;
                return BERR_TRACE(rc);
            }
#else
            pIfAddr_Cached = pIfAddr ;
            BKNI_Memset((void *)(volatile void*)pIfAddr_Cached, 0, ui32IfBuffSize);
#endif /*SR_TBD: Enabled when Interframe download is enabled*/
            BDSP_MMA_P_FlushCache(pStageIterator->sDramInterFrameBuffer.Buffer, ui32IfBuffSize);
        }
    }
    BDSP_ARM_STAGE_TRAVERSE_LOOP_END(pStageIterator)

    BDBG_LEAVE(BDSP_Arm_P_InitInterframeBuffer);
    return rc;
}

/***********************************************************************
Name        :   BDSP_Arm_P_GetDefaultTaskStartSettings

Type        :   PI Interface

Input       :   pTaskHandle -   Handle of the Task for which default start settings is required.
                pSettings       -   Pointer to the memory where default start settings are filled.

Return      :   None

Functionality   :   Following are the operations performed.
        1)  Set the Scheduling mode to Standalone, task realtime mode to Realtime.
        2)  Set the primary stage for the task as NULL and Master task as NULL.
        3)  Disable PPM correction and Gate Open at Start to False.
        4)  Disable the STC trigger and external interrupts.
        5)  Reset the STC address and RDB address for Video Encode.
        5)  Set the Maximum Independent delay to 500ms.
***********************************************************************/
void BDSP_Arm_P_GetDefaultTaskStartSettings(
    void *pTaskHandle,
    BDSP_TaskStartSettings *pSettings    /* [out] */
    )
{

    BDSP_ArmTask *pArmTask;
    pArmTask = (BDSP_ArmTask *)pTaskHandle;
    BDBG_ENTER(BDSP_Arm_P_GetDefaultTaskStartSettings);

    BDBG_OBJECT_ASSERT(pArmTask, BDSP_ArmTask);
    BDBG_ASSERT(NULL != pSettings);

    BKNI_Memset((void *)pSettings,0,sizeof(BDSP_TaskStartSettings));

    pSettings->primaryStage = NULL;
    pSettings->schedulingMode = BDSP_TaskSchedulingMode_eStandalone;
    pSettings->realtimeMode = BDSP_TaskRealtimeMode_eRealTime;
    pSettings->masterTask = NULL;
    pSettings->audioTaskDelayMode = BDSP_AudioTaskDelayMode_eDefault;
    pSettings->timeBaseType = BDSP_AF_P_TimeBaseType_e45Khz;
    pSettings->ppmCorrection = false;
    pSettings->openGateAtStart = false;
    pSettings->stcIncrementConfig.enableStcTrigger = false;
    pSettings->extInterruptConfig.enableInterrupts = false;
    /*Enable Zero Phase Correction in default settings*/
    pSettings->eZeroPhaseCorrEnable=true;

    pSettings->pSampleRateMap = NULL;
    pSettings->maxIndependentDelay = BDSP_AF_P_MAX_INDEPENDENT_DELAY;

    pSettings->gateOpenReqd = true;

    BDBG_LEAVE(BDSP_Arm_P_GetDefaultTaskStartSettings);
}

static BERR_Code   BDSP_Arm_P_InitParams_StartTask(
    void *pContextHandle,
    void *pTaskHandle,
    const BDSP_TaskStartSettings *pStartSettings
    )
{
    BERR_Code   err=BERR_SUCCESS;
    BDSP_ArmContext *pArmContext = (BDSP_ArmContext *)pContextHandle;
    BDSP_ArmTask *pArmTask = (BDSP_ArmTask *)pTaskHandle;
    BDSP_Arm    *pDevice= pArmContext->pDevice;
    unsigned taskId;

    BDBG_ENTER(BDSP_Arm_P_InitParams_StartTask);

    BDBG_OBJECT_ASSERT(pArmContext, BDSP_ArmContext);
    BDBG_OBJECT_ASSERT(pArmTask, BDSP_ArmTask);

    pArmTask->schedulingMode = pStartSettings->schedulingMode;
    pArmTask->masterTaskId   = BDSP_P_INVALID_TASK_ID;

    if(BDSP_TaskSchedulingMode_eStandalone != pStartSettings->schedulingMode)
    {
        BDBG_ERR(("BDSP_Arm_P_InitParams_StartTask: We support only Standalone task on ARM"));
        err = BERR_TRACE(BERR_INVALID_PARAMETER);
        goto end;
    }

    BKNI_AcquireMutex(pDevice->taskDetails.taskIdMutex);
    for (taskId = 0 ; taskId < BDSP_ARM_MAX_FW_TASK_PER_DSP; taskId++)
    {
        if (pDevice->taskDetails.taskId[taskId] == true)
        {
             pDevice->taskDetails.taskId[taskId] = false;
             break;
        }
    }

    if (taskId >=BDSP_ARM_MAX_FW_TASK_PER_DSP)
    {
     BKNI_ReleaseMutex(pDevice->taskDetails.taskIdMutex);
     BDBG_ERR(("Unable to find free Task Instance!"));
     err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
     goto end;
    }

    pArmTask->taskId  = taskId + BDSP_ARM_TASK_ID_START_OFFSET;

    pDevice->taskDetails.pArmTask[taskId]  = pArmTask;

    BKNI_ReleaseMutex(pDevice->taskDetails.taskIdMutex);

    err = BKNI_CreateEvent(&(pArmTask->hEvent));
    if (BERR_SUCCESS != err)
    {
        BDBG_ERR(("BDSP_Arm_P_InitParams_StartTask: Unable to create event"));
        err = BERR_TRACE(err);
        goto end;
    }
    BKNI_ResetEvent(pArmTask->hEvent);

end:
    BDBG_LEAVE(BDSP_Arm_P_InitParams_StartTask);
    return err;
}

static BERR_Code BDSP_Arm_P_CopyStartTaskParams(
    BDSP_ArmTask           *pArmTask,
    BDSP_TaskStartSettings *pStartSettings
    )
{
    BERR_Code   err=BERR_SUCCESS;
    BDSP_ArmContext *pArmContext = (BDSP_ArmContext *)pArmTask->pContext;

    /* Directly Copy whole structure first provided by PI*/
    pArmTask->startSettings = *pStartSettings;

    /*Pointers provided by PI are allocated by them respectively and should not hold them*/
    pArmTask->startSettings.pSampleRateMap      = NULL;
    pArmTask->startSettings.psVDecoderIPBuffCfg = NULL;
    pArmTask->startSettings.psVEncoderIPConfig  = NULL;

    if(BDSP_ContextType_eVideo == pArmContext->settings.contextType )
    {
        if(NULL == pStartSettings->psVDecoderIPBuffCfg)
        {
            BDBG_ERR(("BDSP_Arm_P_CopyStartTaskParams: Input Config structure not provided for Video Decode Task by PI"));
            err = BERR_TRACE(BERR_INVALID_PARAMETER);
            return err;
        }
        pArmTask->startSettings.psVDecoderIPBuffCfg = (BDSP_sVDecoderIPBuffCfg *)BKNI_Malloc(sizeof(BDSP_sVDecoderIPBuffCfg));
        BKNI_Memcpy((void*)pArmTask->startSettings.psVDecoderIPBuffCfg,
            (void *)pStartSettings->psVDecoderIPBuffCfg,
            sizeof(BDSP_sVDecoderIPBuffCfg));
        goto end;
    }

    if(BDSP_ContextType_eVideoEncode == pArmContext->settings.contextType )
    {
        if(NULL == pStartSettings->psVEncoderIPConfig)
        {
            BDBG_ERR(("BDSP_Arm_P_CopyStartTaskParams: Input Config structure not provided for Video Encode Task by PI"));
            err = BERR_TRACE(BERR_INVALID_PARAMETER);
            return err;
        }
        pArmTask->startSettings.psVEncoderIPConfig = (BDSP_sVEncoderIPConfig *)BKNI_Malloc(sizeof(BDSP_sVEncoderIPConfig));
        BKNI_Memcpy((void*)pArmTask->startSettings.psVEncoderIPConfig,
            (void *)pStartSettings->psVEncoderIPConfig,
            sizeof(BDSP_sVEncoderIPConfig));
        goto end;
    }
    if(BDSP_ContextType_eAudio == pArmContext->settings.contextType )
    {
        /* If APE doesnt provide the Sample Rate Map table then BDSP will internally fill it. No error handling required*/
        if(NULL != pStartSettings->pSampleRateMap)
        {
            pArmTask->startSettings.pSampleRateMap = (BDSP_AF_P_sOpSamplingFreq *)BKNI_Malloc(sizeof(BDSP_AF_P_sOpSamplingFreq));
            BKNI_Memcpy((void*)pArmTask->startSettings.pSampleRateMap,
                (void *)pStartSettings->pSampleRateMap,
                sizeof(BDSP_AF_P_sOpSamplingFreq));
        }
    }
end:
    return BERR_SUCCESS;
}

static BERR_Code BDSP_Arm_P_CalcThresholdZeroFillTimeAudOffset(
                    BDSP_CTB_Input              *psCtbInput,
                    void                            *pStageHandle,
                    BDSP_CTB_Output             *psCTB_OutputStructure
                )
{
    BERR_Code   err = BERR_SUCCESS;
    BDSP_ArmStage *pArmStage = (BDSP_ArmStage *)pStageHandle;

    BDBG_ENTER(BDSP_Arm_P_CalcThresholdZeroFillTimeAudOffset);
    BDBG_OBJECT_ASSERT (pArmStage, BDSP_ArmStage);

    BSTD_UNUSED(psCtbInput);

    psCTB_OutputStructure->ui32Threshold = (BDSP_AF_P_MAX_THRESHOLD + BDSP_AF_P_SAMPLE_PADDING);
    psCTB_OutputStructure->ui32BlockTime = 42;
    psCTB_OutputStructure->ui32AudOffset = 128;

    BDBG_LEAVE(BDSP_Arm_P_CalcThresholdZeroFillTimeAudOffset);
    return err;
}
static BERR_Code BDSP_Arm_P_CheckConfigLimitations(BDSP_AudioTaskDelayMode eDelayMode,
                                                void *pTaskHandle)
{
    BDSP_ArmTask *pArmTask = (BDSP_ArmTask *)pTaskHandle;
    BSTD_UNUSED(eDelayMode);
    BSTD_UNUSED(pArmTask);
    BDBG_MSG(("Right now there are no limitations on configuartion of task for ARM. Kept for Future Use!!!!!!"));
    return BERR_SUCCESS;
}

BERR_Code BDSP_Arm_P_SendMapCommand(
    void *pDeviceHandle,
    BDSP_MAP_Table_Entry *pMapTableEntries,
    uint32_t ui32NumEntries
    )
{
    BERR_Code err = BERR_SUCCESS;
    BDSP_Arm *pDevice = (BDSP_Arm *)pDeviceHandle;
    BDSP_Arm_P_Command *psCommand;
    BDSP_MAP_Table *pMapTable = NULL;
    uint32_t physAddress = 0;

    BDBG_ENTER(BDSP_Arm_P_SendMapCommand);

    if(ui32NumEntries == 0)
    {
        BDBG_MSG(("BDSP_Arm_P_SendMapCommand: Number of entries send to MAP is ZERO"));
        return err;
    }

    psCommand = BKNI_Malloc(sizeof(BDSP_Arm_P_Command));
    if ( NULL == psCommand )
    {
        err = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto err_malloc_command;
    }

	pMapTable = (BDSP_MAP_Table *)pDevice->memInfo.sMapTable.Buffer.pAddr;
    if(NULL == pMapTable)
    {
        BDBG_ERR((" Allocated Address for MAP Table is not proper"));
        err = BERR_TRACE (BDSP_ERR_BAD_DEVICE_STATE);
        goto end;
    }
    BKNI_Memset(pMapTable, 0,sizeof(BDSP_MAP_Table));
    BKNI_Memset(psCommand,0,sizeof(*psCommand));

    BKNI_Memcpy((void *)(&(psCommand->uCommand.sMapCommand.sMapEntries[0])),pMapTableEntries,(ui32NumEntries*sizeof(BDSP_MAP_Table_Entry)));
    BDSP_MMA_P_FlushCache(pDevice->memInfo.sMapTable.Buffer, (ui32NumEntries*sizeof(BDSP_MAP_Table_Entry)));
    physAddress = pDevice->memInfo.sMapTable.Buffer.offset;
    psCommand->sCommandHeader.ui32CommandID = BDSP_ARM_MAP_COMMAND_ID;
    psCommand->sCommandHeader.ui32CommandCounter = 0;
    psCommand->sCommandHeader.ui32TaskID = 0;
    psCommand->sCommandHeader.eResponseType = BDSP_P_ResponseType_eNone;
    psCommand->sCommandHeader.ui32CommandSizeInBytes =  sizeof(BDSP_Arm_P_Command);

    psCommand->uCommand.sMapCommand.ui32NumEntries        = ui32NumEntries;
    /*sCommand.uCommand.sMapCommand.ui32HostMapTableAddr  = physAddress;*/ /*CDN_TBD*/
    BSTD_UNUSED(physAddress);

    BKNI_ResetEvent(pDevice->hDeviceEvent);
    err = BDSP_Arm_P_SendCommand(pDevice->hCmdQueue, psCommand,(void *)NULL);

    if (BERR_SUCCESS != err)
    {
        BDBG_ERR(("BDSP_Arm_P_SendMapCommand: MAP Command failed!"));
        err = BERR_TRACE(err);
        goto end;
    }

    /* Wait for Ack_Response_Received event w/ timeout */
    err = BKNI_WaitForEvent(pDevice->hDeviceEvent, BDSP_ARM_START_STOP_EVENT_TIMEOUT_IN_MS);
    if (BERR_TIMEOUT == err)
    {
        BDBG_ERR(("BDSP_Arm_P_SendMapCommand: MAP_CMD ACK timeout!"));
        err = BERR_TRACE(err);
        goto end;
    }
end:
    BKNI_Free(psCommand);
err_malloc_command:
    BDBG_LEAVE(BDSP_Arm_P_SendMapCommand);
    return err;
}

BERR_Code BDSP_Arm_P_SendUnMapCommand(
    void *pDeviceHandle,
    BDSP_MAP_Table_Entry *pMapTableEntries,
    uint32_t ui32NumEntries
    )
{
    BERR_Code err = BERR_SUCCESS;
    BDSP_Arm *pDevice = (BDSP_Arm *)pDeviceHandle;
    BDSP_Arm_P_Command *psCommand;
    BDSP_MAP_Table *pMapTable = NULL;
    uint32_t physAddress = 0;

    BDBG_ENTER(BDSP_Arm_P_SendUnMapCommand);

    if(ui32NumEntries == 0)
    {
        BDBG_MSG(("BDSP_Arm_P_SendUnMapCommand: Number of entries send to UNMAP is ZERO"));
        return err;
    }

    psCommand = BKNI_Malloc(sizeof(BDSP_Arm_P_Command));
    if ( NULL == psCommand )
    {
        err = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto err_malloc_command;
    }

	pMapTable = (BDSP_MAP_Table *)pDevice->memInfo.sMapTable.Buffer.pAddr;
    if(NULL == pMapTable)
    {
        BDBG_ERR((" Allocated Address for MAP Table is not proper"));
        err = BERR_TRACE(BDSP_ERR_BAD_DEVICE_STATE);
		goto end;
    }
    BKNI_Memset(pMapTable, 0,sizeof(BDSP_MAP_Table));
    BKNI_Memset(psCommand,0,sizeof(*psCommand));

    BKNI_Memcpy((void *)(&(psCommand->uCommand.sUnMapCommand.sMapEntries[0])),pMapTableEntries,(ui32NumEntries*sizeof(BDSP_MAP_Table_Entry)));
    BDSP_MMA_P_FlushCache(pDevice->memInfo.sMapTable.Buffer, (ui32NumEntries*sizeof(BDSP_MAP_Table_Entry)));
    physAddress = pDevice->memInfo.sMapTable.Buffer.offset;
    if(pDevice->deviceWatchdogFlag == false)
    {
        psCommand->sCommandHeader.ui32CommandID = BDSP_ARM_UNMAP_COMMAND_ID;
        psCommand->sCommandHeader.ui32CommandCounter = 0;
        psCommand->sCommandHeader.ui32TaskID = 0;
        psCommand->sCommandHeader.eResponseType = BDSP_P_ResponseType_eNone;
        psCommand->sCommandHeader.ui32CommandSizeInBytes =  sizeof(BDSP_Arm_P_Command);

        psCommand->uCommand.sUnMapCommand.ui32NumEntries        = ui32NumEntries;
        /*sCommand.uCommand.sUnMapCommand.ui32HostUnMapTableAddr= physAddress;*/ /*CDN_TBD*/
        BSTD_UNUSED(physAddress);

        BKNI_ResetEvent(pDevice->hDeviceEvent);
        err = BDSP_Arm_P_SendCommand(pDevice->hCmdQueue, psCommand,(void *)NULL);

        if (BERR_SUCCESS != err)
        {
            BDBG_ERR(("BDSP_Arm_P_SendUnMapCommand: UnMAP Command failed!"));
            err = BERR_TRACE(err);
            goto end;
        }

        /* Wait for Ack_Response_Received event w/ timeout */
        err = BKNI_WaitForEvent(pDevice->hDeviceEvent, BDSP_ARM_START_STOP_EVENT_TIMEOUT_IN_MS);
        if (BERR_TIMEOUT == err)
        {
            BDBG_ERR(("BDSP_Arm_P_SendUnMapCommand: UNMAP_CMD ACK timeout!"));
            err = BERR_TRACE(err);
            goto end;
        }
    }
end:
    BKNI_Free(psCommand);
err_malloc_command:
    BDBG_LEAVE(BDSP_Arm_P_SendUnMapCommand);
    return err;
}

BERR_Code BDSP_Arm_P_PrepareAndSendMapCommand(
    void *pTaskHandle
    )
{
    BERR_Code   err = BERR_SUCCESS;
    BDSP_ArmTask *pArmTask = (BDSP_ArmTask *)pTaskHandle;
    BDSP_MAP_Table_Entry *pMapTable = NULL;
    uint32_t ui32NumEntries = 0, ui32NumStageIterationEntries = 0;

    BDBG_ENTER(BDSP_Arm_P_PrepareAndSendMapCommand);

    pMapTable = (BDSP_MAP_Table_Entry *)BKNI_Malloc(BDSP_ARM_MAX_MAP_TABLE_ENTRY*sizeof(BDSP_MAP_Table_Entry));
    if ( NULL == pMapTable )
    {
        err = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto err_malloc_maptable;
    }

    BKNI_Memset(pMapTable,0,(BDSP_ARM_MAX_MAP_TABLE_ENTRY*sizeof(BDSP_MAP_Table_Entry)));

    BDSP_Arm_P_RetrieveEntriesToMap(&(pArmTask->sTaskMapTable[0]), &pMapTable[0], &ui32NumStageIterationEntries, BDSP_ARM_MAX_ALLOC_TASK);
    ui32NumEntries += ui32NumStageIterationEntries;
    BDBG_MSG(("Number of Entries for the TASK = %d",ui32NumStageIterationEntries));


    BDSP_ARM_STAGE_TRAVERSE_LOOP_BEGIN((BDSP_ArmStage *)pArmTask->startSettings.primaryStage->pStageHandle, pStageIterator)
    BSTD_UNUSED(macroStId);
    BSTD_UNUSED(macroBrId);
    {
        ui32NumStageIterationEntries = 0;
        BDSP_Arm_P_RetrieveEntriesToMap(&(pStageIterator->sStageMapTable[0]), &pMapTable[ui32NumEntries], &ui32NumStageIterationEntries, BDSP_ARM_MAX_ALLOC_STAGE);
        ui32NumEntries += ui32NumStageIterationEntries;
        BDBG_MSG(("Number of Entries for the STAGE = %d",ui32NumStageIterationEntries));
    }
    BDSP_ARM_STAGE_TRAVERSE_LOOP_END(pStageIterator)

    BDBG_MSG(("Total Number of Entries for the TASK = %d",ui32NumEntries));

    err = BDSP_Arm_P_SendMapCommand(pArmTask->pContext->pDevice, &pMapTable[0], ui32NumEntries);
    if (BERR_SUCCESS != err)
    {
        BDBG_ERR(("BDSP_ARM_P_Open: Send ARM MAP Command failed!!!!"));
    }

    BKNI_Free(pMapTable);
err_malloc_maptable:
    BDBG_LEAVE(BDSP_Arm_P_PrepareAndSendMapCommand);
    return err;
}

static BERR_Code BDSP_Arm_P_FindSlaveTaskSchedulingBuffer(
            const BDSP_TaskStartSettings *pStartSettings,
            uint32_t *pui32DramSchedulingBuffCfgAddr,
            BDSP_AF_P_BufferType    *pBufferType)
{
    BERR_Code   err = BERR_SUCCESS;
    unsigned i;

    *pBufferType = BDSP_AF_P_BufferType_eDRAM;
    *pui32DramSchedulingBuffCfgAddr = 0;

    BDSP_ARM_STAGE_TRAVERSE_LOOP_BEGIN((BDSP_ArmStage *)pStartSettings->primaryStage->pStageHandle, pStageIterator)
    BSTD_UNUSED(macroStId);
    BSTD_UNUSED(macroBrId);
    {
        for (i = 0; i < pStageIterator->totalOutputs; i++)
        {
            if (pStageIterator->sStageOutput[i].eConnectionType == BDSP_ConnectionType_eInterTaskBuffer)
            {
                BDSP_P_InterTaskBuffer *pArmInterTaskBuffer;
                pArmInterTaskBuffer = (BDSP_P_InterTaskBuffer *)pStageIterator->sStageOutput[i].connectionDetails.interTask.hInterTask->pInterTaskBufferHandle;
                *pui32DramSchedulingBuffCfgAddr = pArmInterTaskBuffer->IoBufferDesc.offset;
                break;
            }
        }
    }
    BDSP_ARM_STAGE_TRAVERSE_LOOP_END(pStageIterator)

    if (*pui32DramSchedulingBuffCfgAddr == 0)
    {
        err = BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    return err;
}

#if 0 /*SR_TBD */
static BERR_Code BDSP_Arm_P_FindSchedulingBuffer(
    const BDSP_TaskCreateSettings *pSettings,
    const BDSP_TaskStartSettings *pStartSettings,
    BDSP_CIT_P_FwStgSrcDstType    *eSchedulingBufferType,
    unsigned *pBufferId,
    BDSP_AF_P_sDRAM_CIRCULAR_BUFFER    *pBufferPtr,
    unsigned *pDelay,
    BDSP_AF_P_FmmDstFsRate *pDstRate,
    unsigned *pSchedulingBufferthreshold
    )
{
    unsigned compressed=(unsigned)-1, compressedDelay=0;
    BDSP_AF_P_FmmDstFsRate compressedRate=BDSP_AF_P_FmmDstFsRate_eBaseRate;
    /* Holds the list of stage handles that have not been examined */
    uint32_t i, ui32Buff0AddrStart, ui32Buff2BuffOffset;
    unsigned int uiSchedulingBufferThreshold = (BDSP_ARM_CIT_P_STANDARD_BUFFER_THRESHOLD + BDSP_ARM_CIT_P_MAXIMUM_RESIDUAL_COLLECTION);

    BDBG_ASSERT(NULL != pSettings);
    BDBG_ASSERT(NULL != pBufferId);
    BDBG_ASSERT(NULL != pDelay);
    BDBG_ASSERT(NULL != pDstRate);
    *pBufferId = 0;
    *pDelay = 0;
    *pDstRate = BDSP_AF_P_FmmDstFsRate_eBaseRate;
    *pSchedulingBufferthreshold = (BDSP_ARM_CIT_P_STANDARD_BUFFER_THRESHOLD + BDSP_ARM_CIT_P_MAXIMUM_RESIDUAL_COLLECTION);

    BDSP_ARM_STAGE_TRAVERSE_LOOP_BEGIN((BDSP_ArmStage *)pStartSettings->primaryStage->pStageHandle, pStageIterator)
    BSTD_UNUSED(macroStId);
    BSTD_UNUSED(macroBrId);
    {
        /* Check if the current stage has any PCM/Compressed FMM outputs
            if Yes, Found scheduling buffer -> Break */
        for (i = 0; i < BDSP_AF_P_MAX_OP_FORKS; i++)
        {
            if (pStartSettings->realtimeMode == BDSP_TaskRealtimeMode_eNonRealTime)
            {
#if 0
                if (pStageIterator->sStageOutput[i].eConnectionType == BDSP_ConnectionType_eRaveBuffer &&
                    pStageIterator->sStageOutput[i].eNodeValid == BDSP_AF_P_eValid)
                {
                    *eSchedulingBufferType = BDSP_CIT_P_FwStgSrcDstType_eRaveBuf;
                    pBufferPtr->ui32BaseAddr = BCHP_PHYSICAL_OFFSET + pStageIterator->sStageOutput[i].connectionDetails.rave.pContextMap->CDB_Base;
                    pBufferPtr->ui32EndAddr = BCHP_PHYSICAL_OFFSET + pStageIterator->sStageOutput[i].connectionDetails.rave.pContextMap->CDB_End;
                    pBufferPtr->ui32ReadAddr = BCHP_PHYSICAL_OFFSET + pStageIterator->sStageOutput[i].connectionDetails.rave.pContextMap->CDB_Read;
                    pBufferPtr->ui32WriteAddr = BCHP_PHYSICAL_OFFSET + pStageIterator->sStageOutput[i].connectionDetails.rave.pContextMap->CDB_Valid;
                    pBufferPtr->ui32WrapAddr = BCHP_PHYSICAL_OFFSET + pStageIterator->sStageOutput[i].connectionDetails.rave.pContextMap->CDB_Wrap;
                    *pDelay = 0;
                    return BERR_SUCCESS;
                }
#else
                BDBG_ERR(("Non Realtime task not supported"));
                return BERR_TRACE(BERR_INVALID_PARAMETER);
#endif
            }
            else if(pStartSettings->realtimeMode == BDSP_TaskRealtimeMode_eOnDemand)
            {
#if 0
                if(pStageIterator->sStageOutput[i].eConnectionType == BDSP_ConnectionType_eRDBBuffer &&
                   pStageIterator->sStageOutput[i].eNodeValid == BDSP_AF_P_eValid && pStageIterator->algorithm == BDSP_Algorithm_eOutputFormatter)
                    {
                        pRaagaQueue = (BDSP_RaagaQueue *)pStageIterator->sStageOutput[i].connectionDetails.rdb.pQHandle;
                        *eSchedulingBufferType = BDSP_CIT_P_FwStgSrcDstType_eRDBPool;
                        pBufferPtr->ui32BaseAddr = BDSP_RAAGA_REGSET_ADDR_FOR_DSP( pRaagaQueue->FIFOdata[0]->uiBaseAddr );
                        pBufferPtr->ui32EndAddr = BDSP_RAAGA_REGSET_ADDR_FOR_DSP( pRaagaQueue->FIFOdata[0]->uiEndAddr );
                        pBufferPtr->ui32ReadAddr = BDSP_RAAGA_REGSET_ADDR_FOR_DSP( pRaagaQueue->FIFOdata[0]->uiReadPtr );
                        pBufferPtr->ui32WriteAddr = BDSP_RAAGA_REGSET_ADDR_FOR_DSP( pRaagaQueue->FIFOdata[0]->uiWritePtr );
                        pBufferPtr->ui32WrapAddr = BDSP_RAAGA_REGSET_ADDR_FOR_DSP( pRaagaQueue->FIFOdata[0]->uiEndAddr );
                        *pDelay = 0;
                        BDBG_MSG(("sched,readptr = %x,writeptr = %x",pBufferPtr->ui32ReadAddr,pBufferPtr->ui32WriteAddr));
                        return BERR_SUCCESS;
                    }
#else
                BDBG_ERR(("On Demand Task task not supported"));
                return BERR_TRACE(BERR_INVALID_PARAMETER);
#endif
            }
            else
            {
                if (pStageIterator->sStageOutput[i].eConnectionType == BDSP_ConnectionType_eFmmBuffer &&
                    pStageIterator->sStageOutput[i].eNodeValid == BDSP_AF_P_eValid)
                {
#ifdef BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR
                    ui32Buff0AddrStart  = BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR;
                    ui32Buff2BuffOffset = BCHP_AUD_FMM_BF_CTRL_RINGBUF_1_RDADDR -
                                                BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR;
#else
                    ui32Buff0AddrStart  = BCHP_AUD_FMM_BF_CTRL_SOURCECH_RINGBUF_0_RDADDR;
                    ui32Buff2BuffOffset = BCHP_AUD_FMM_BF_CTRL_SOURCECH_RINGBUF_1_RDADDR -
                                                BCHP_AUD_FMM_BF_CTRL_SOURCECH_RINGBUF_0_RDADDR;
#endif
                    if ( (pStageIterator->eStageOpBuffDataType[i] == BDSP_AF_P_DistinctOpType_eCompressed)
                         || (pStageIterator->eStageOpBuffDataType[i] == BDSP_AF_P_DistinctOpType_eCompressed4x)
                         || (pStageIterator->eStageOpBuffDataType[i] == BDSP_AF_P_DistinctOpType_eCompressedHBR))
                    {
                        if ( compressed == (unsigned)-1 )
                        {
                            compressed = (pStageIterator->sStageOutput[i].connectionDetails.fmm.descriptor.buffers[0].read - ui32Buff0AddrStart)/(ui32Buff2BuffOffset);
                            compressedDelay = pStageIterator->sStageOutput[i].connectionDetails.fmm.descriptor.delay;

                            /* Changes for setting up the eFmmDstFsRate compressed passthru / BTSC Encoder */
                            if (pStageIterator->eStageOpBuffDataType[i] == BDSP_AF_P_DistinctOpType_eCompressedHBR)
                            {
                                compressedRate = BDSP_AF_P_FmmDstFsRate_e16xBaseRate;
                                if(pStageIterator->algorithm == BDSP_Algorithm_eMlpPassthrough)
                                {
                                    /* Note: MLP Passthru uses a different logic for threshold and hence no need to add residual collection. The calculation is explained in the macro defination*/
                                    uiSchedulingBufferThreshold = BDSP_ARM_CIT_P_COMPRESSED16X_MLP_BUFFER_THRESHOLD;
                                }
                                else
                                {
                                    uiSchedulingBufferThreshold = (BDSP_ARM_CIT_P_COMPRESSED16X_BUFFER_THRESHOLD + BDSP_ARM_CIT_P_MAXIMUM_RESIDUAL_COLLECTION);
                                }
                            }
                            else if (pStageIterator->eStageOpBuffDataType[i] == BDSP_AF_P_DistinctOpType_eCompressed4x)
                            {
                                compressedRate = BDSP_AF_P_FmmDstFsRate_e4xBaseRate;
                                uiSchedulingBufferThreshold = (BDSP_ARM_CIT_P_COMPRESSED4X_BUFFER_THRESHOLD + BDSP_ARM_CIT_P_MAXIMUM_RESIDUAL_COLLECTION);
                            }
                            else
                            {
                                compressedRate = BDSP_AF_P_FmmDstFsRate_eBaseRate;
                                uiSchedulingBufferThreshold = (BDSP_ARM_CIT_P_STANDARD_BUFFER_THRESHOLD + BDSP_ARM_CIT_P_MAXIMUM_RESIDUAL_COLLECTION);
                            }

                            if ((pStageIterator->eStageOpBuffDataType[i] == BDSP_AF_P_DistinctOpType_eCompressed)
                                && (pStageIterator->algorithm == BDSP_Algorithm_eBtscEncoder))
                            {
                                compressedRate = BDSP_AF_P_FmmDstFsRate_e4xBaseRate;
                                uiSchedulingBufferThreshold = (BDSP_ARM_CIT_P_STANDARD_BUFFER_THRESHOLD + BDSP_ARM_CIT_P_MAXIMUM_RESIDUAL_COLLECTION);
                            }
                        }
                        BDBG_MSG(("Scheduling buffer associated with %s(%d)",BDSP_Arm_P_LookupAlgorithmInfo(pStageIterator->algorithm)->pName,pStageIterator->algorithm ));

                    }
                    else
                    {
                        *pBufferId = (pStageIterator->sStageOutput[i].connectionDetails.fmm.descriptor.buffers[0].read - ui32Buff0AddrStart)/(ui32Buff2BuffOffset);
                        *pDelay = pStageIterator->sStageOutput[i].connectionDetails.fmm.descriptor.delay;
                        *pDstRate = BDSP_AF_P_FmmDstFsRate_eBaseRate;
                        *eSchedulingBufferType = BDSP_CIT_P_FwStgSrcDstType_eFMMBuf;
                        *pSchedulingBufferthreshold = (BDSP_ARM_CIT_P_STANDARD_BUFFER_THRESHOLD + BDSP_ARM_CIT_P_MAXIMUM_RESIDUAL_COLLECTION);

                        BDBG_MSG(("Scheduling buffer: associated with %s(%d)",BDSP_Arm_P_LookupAlgorithmInfo(pStageIterator->algorithm)->pName,pStageIterator->algorithm ));
                        BDBG_MSG(("Scheduling buffer: *pBufferId=%d, *pDelay=%d, ",*pBufferId,*pDelay));
                        return BERR_SUCCESS;
                    }
                }
            }
        }
    }
    BDSP_ARM_STAGE_TRAVERSE_LOOP_END(pStageIterator)

    /* If we reach here, return success if we found a compressed buffer */
    if ( compressed != (unsigned)-1 )
    {
        *pBufferId = compressed;
        *pDelay = compressedDelay;
        *pDstRate = compressedRate;
        *eSchedulingBufferType = BDSP_CIT_P_FwStgSrcDstType_eFMMBuf;
        *pSchedulingBufferthreshold = uiSchedulingBufferThreshold ;
        BDBG_MSG(("Find Sch buffer,*pBufferId=%d, *pDelay=%d, *pDstRate=%d",*pBufferId,*pDelay, *pDstRate));

        return BERR_SUCCESS;
    }
    /* Nothing to schedule off */
    return BERR_TRACE(BERR_INVALID_PARAMETER);
}
#endif /*SR_TBD */

BERR_Code BDSP_Arm_P_StartTask(
    void *pTaskHandle,
    BDSP_TaskStartSettings *pStartSettings    /* [out] */
    )
{
    BERR_Code   err = BERR_SUCCESS;
    BDSP_ArmTask *pArmTask = (BDSP_ArmTask *)pTaskHandle;
    BDSP_ArmContext *pArmContext = (BDSP_ArmContext *)pArmTask->pContext;
    BDSP_Arm  *pDevice = pArmContext->pDevice;
    BDSP_ArmStage *pArmPrimaryStage;
    BDSP_Arm_P_Command *psCommand;
    BDSP_Arm_P_Response sRsp;
    BDSP_P_MsgType  eMsgType;
    BDSP_P_TaskParamInfo *pTaskParams = NULL;
    BDSP_CTB_Input sctbInput;
    uint32_t    ui32PhysAddr = 0;
    unsigned    uiSchedulingBufId, uiSchedulingBufDelay = 0, ui32RbufOffset = 0, uiSchedulingBufferThreshold = 0;
    BDSP_AF_P_FmmDstFsRate eschedulingBufferRate = BDSP_AF_P_FmmDstFsRate_eBaseRate;

    BDBG_ENTER(BDSP_Arm_P_StartTask);
    BDBG_OBJECT_ASSERT(pArmTask, BDSP_ArmTask);

    BDBG_MSG(("Start task: DSP %d",pArmTask->settings.dspIndex));

    psCommand = BKNI_Malloc(sizeof(BDSP_Arm_P_Command));
    if ( NULL == psCommand )
    {
        err = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto err_malloc_command;
    }

    BKNI_Memset(psCommand,0,sizeof(*psCommand));
    BKNI_Memset(&sRsp,0,sizeof(sRsp));

    err = BDSP_Arm_P_CopyStartTaskParams(pArmTask, pStartSettings);
    if (BERR_SUCCESS != err)
    {
        BDBG_ERR(("BDSP_Arm_P_StartTask: Unable to Copy the Start Task params provided by PI"));
        err = BERR_TRACE(err);
        goto end;
    }
    pArmPrimaryStage = (BDSP_ArmStage *)pStartSettings->primaryStage->pStageHandle;

    err = BDSP_Arm_P_InitMsgQueue(pDevice, pArmTask->hSyncMsgQueue);
    if (BERR_SUCCESS != err)
    {
        BDBG_ERR(("BDSP_Arm_P_StartTask: Sync Queue Init failed!"));
        err = BERR_TRACE(err);
        goto end;
    }

    err = BDSP_Arm_P_InitMsgQueue(pDevice, pArmTask->hAsyncMsgQueue);
    if (BERR_SUCCESS != err)
    {
        BDBG_ERR(("BDSP_Arm_P_StartTask: ASync Queue Init failed!"));
        err = BERR_TRACE(err);
        goto end;
    }

    err = BDSP_Arm_P_InitParams_StartTask((void *)pArmContext, (void *)pArmTask, pStartSettings);
    if (BERR_SUCCESS != err)
    {
        BDBG_ERR(("BDSP_Arm_P_StartTask: Error in Initialising the Start Task"));
        err =BERR_TRACE(err);
        goto end;
    }

    /* Populate the branch and stage id */
    BDBG_ASSERT(NULL != pArmPrimaryStage);
    BDSP_ARM_STAGE_TRAVERSE_LOOP_V1_BEGIN(pArmPrimaryStage, pStageIterator, branchId, stageId)
    {
        pStageIterator->ui32BranchId = branchId;
        pStageIterator->ui32StageId = stageId;
    }
    BDSP_ARM_STAGE_TRAVERSE_LOOP_END(pStageIterator)

    /*Keep this Download Time FWExec during Start time, as only then all info related to stages of a Task will be known
    Previously it was done based on the Task create settings, where the branch info was known, now it will be done based on the Stage handles.*/
    if (false == pDevice->memInfo.sDwnldMemInfo.IsImagePreLoaded)
    {
        /* Download the firmware binaries required by the complete network */
        err = BDSP_Arm_P_DownloadStartTimeFWExec((void*)pArmContext, (void*)pStartSettings->primaryStage->pStageHandle);
        if ( BERR_SUCCESS !=err )
        {
            err = BERR_TRACE(BDSP_ERR_DOWNLOAD_FAILED);
            goto err_download_fw;
        }
	}
	else /* Image is available in the dram, hence only copy it to astra memory */
	{
#if 0
		err = BDSP_Arm_P_DownloadFwToAstra(pDevice->armDspApp.hClient,pDevice,	algoId);
		if ( err ){
			err = BERR_TRACE(errCode);
			goto err_download_fw;
		}
#endif
    }

    /* Initialize to eDisable by default */
    psCommand->uCommand.sStartTask.eOpenGateAtStart = BDSP_AF_P_eDisable;

    if(BDSP_ContextType_eAudio!= pArmContext->settings.contextType)
    {
        BDBG_ERR(("BDSP_Arm_P_StartTask: Trying to start a non audio task on ARM"));
        err = BERR_TRACE(BERR_INVALID_PARAMETER);
        goto end;
    }

    err = BDSP_Arm_P_GenCit(pTaskHandle);
    if (BERR_SUCCESS != err)
    {
        BDBG_ERR(("ERROR returned from ARM Cit module %d!",err));
        err =BERR_TRACE(err);
        goto err_gen_citinput;
    }

    err = BDSP_Arm_P_PrepareAndSendMapCommand(pTaskHandle);
    if( err != BERR_SUCCESS)
    {
        BDBG_ERR(("ERROR in sending the MAP Command %d!",err));
		err = BERR_TRACE(err);
        goto err_gen_citinput;
	}

    sctbInput.audioTaskDelayMode = pStartSettings->audioTaskDelayMode;
    sctbInput.realtimeMode = pStartSettings->realtimeMode;

    err = BDSP_Arm_P_CalcThresholdZeroFillTimeAudOffset(
        &sctbInput,
        (void*)pStartSettings->primaryStage->pStageHandle, &(pArmTask->ctbOutput));

    if( err != BERR_SUCCESS)
        return BERR_TRACE(err);

    /* If there are any configuartion limitaions that needs to be implemented, add the check inside the function */
    err = BDSP_Arm_P_CheckConfigLimitations(pStartSettings->audioTaskDelayMode, pTaskHandle);
    if( err != BERR_SUCCESS)
        return BERR_TRACE(err);

    /* Download CIT structure into DSP/RUNNING CIT Buffer DRAM */
    err = BDSP_MMA_P_CopyDataToDram(&pArmTask->taskMemGrants.sCitStruct.Buffer, (void *)&(pArmTask->citOutput.sCit), pArmTask->taskMemGrants.sCitStruct.ui32Size);
    if (BERR_SUCCESS != err)
    {
        BDBG_ERR(("Error in Copying data to CIT Buffer DRAM"));
        err = BERR_TRACE(err);
        goto err_gen_citinput;
    }

    /* Download CIT structure into HOST/SPARE CIT Buffer DRAM */
    err = BDSP_MMA_P_CopyDataToDram(&pArmTask->taskMemGrants.sSpareCitStruct.Buffer, (void *)&(pArmTask->citOutput.sCit), pArmTask->taskMemGrants.sSpareCitStruct.ui32Size);
    if (BERR_SUCCESS != err)
    {
        BDBG_ERR(("Error in Copying data to SPARE CIT Buffer DRAM"));
        err = BERR_TRACE(err);
        goto err_gen_citinput;
    }

    BDSP_Arm_P_Analyse_CIT(pArmTask, false);
    /* Initialize interframe buffers for all the nodes */
    err = BDSP_Arm_P_InitInterframeBuffer((void *)pStartSettings->primaryStage->pStageHandle);
    if (BERR_SUCCESS != err)
    {
        BDBG_ERR(("Error in initializing Interframe buffers for Task id %d",pArmTask->taskId));
        err = BERR_TRACE(err);
        goto err_gen_citinput;
    }
    pArmTask->eventEnabledMask |= BDSP_ARM_DEFAULT_EVENTS_ENABLE_MASK;

    BDBG_MSG(("Context Type = %d",pArmContext->settings.contextType));
    BDBG_MSG(("Task Id = %d",pArmTask->taskId));

    if (pArmContext->settings.contextType == BDSP_ContextType_eAudio)
    {
        /* Send Start Task Command */
        psCommand->sCommandHeader.ui32CommandID = BDSP_START_TASK_COMMAND_ID;
        psCommand->sCommandHeader.ui32CommandCounter = pArmTask->commandCounter++;
        psCommand->sCommandHeader.ui32TaskID = pArmTask->taskId;
        psCommand->sCommandHeader.eResponseType = BDSP_P_ResponseType_eResponseRequired;
        psCommand->sCommandHeader.ui32CommandSizeInBytes =  sizeof(BDSP_Arm_P_Command);
        psCommand->uCommand.sStartTask.eZeroPhaseCorrEnable = pStartSettings->eZeroPhaseCorrEnable;
        switch ( BDSP_ARM_P_ALGORITHM_TYPE(((BDSP_ArmStage *)pStartSettings->primaryStage->pStageHandle)->algorithm) )
        {
            default:
            case BDSP_AlgorithmType_eAudioEncode:
                psCommand->uCommand.sStartTask.eTaskAlgoType  = BDSP_P_AlgoType_eEncode;
                psCommand->uCommand.sStartTask.ePPMCorrEnable = BDSP_AF_P_eDisable;
                break;
        }
        /* Enable mute frame rendering at start based on the flag openGateAtStart */
        if (pStartSettings->openGateAtStart)
        {
            psCommand->uCommand.sStartTask.eOpenGateAtStart = BDSP_AF_P_eEnable;
        }
        else
        {
            psCommand->uCommand.sStartTask.eOpenGateAtStart = BDSP_AF_P_eDisable;
        }

        /* Fill up start task parameters */
        pTaskParams = (BDSP_P_TaskParamInfo *)(pArmTask->taskMemGrants.sTaskInfo.Buffer.pAddr);
        if (pArmTask->startSettings.realtimeMode== BDSP_TaskRealtimeMode_eNonRealTime)
        {
            psCommand->uCommand.sStartTask.eDeadlineComputationFuncType =
                BDSP_P_DeadLineComputeFuncType_eNonRealtimeDecode;
            psCommand->uCommand.sStartTask.eTaskType = BDSP_P_TaskType_eNonRealtime;

            pTaskParams->sNonRealTimeTaskParams.ui32MaxTimeSlice = 1350;
            pTaskParams->sNonRealTimeTaskParams.ui32MinTimeThreshold = 45;
        }
        else if(pArmTask->startSettings.realtimeMode== BDSP_TaskRealtimeMode_eOnDemand)
        {
            psCommand->uCommand.sStartTask.eTaskType = BDSP_P_TaskType_eOnDemand;
            psCommand->uCommand.sStartTask.eDeadlineComputationFuncType =
                BDSP_P_DeadLineComputeFuncType_eOnDemand;
        }
        else
        {
            psCommand->uCommand.sStartTask.eTaskType = BDSP_P_TaskType_eRealtime;
            psCommand->uCommand.sStartTask.eDeadlineComputationFuncType =
                BDSP_P_DeadLineComputeFuncType_eRealtimeDecode;
        }

        if (pArmTask->startSettings.schedulingMode == BDSP_TaskSchedulingMode_eSlave)
        {
            psCommand->uCommand.sStartTask.eSchedulingMode = BDSP_P_SchedulingMode_eSlave;
        }
        else
        {
            psCommand->uCommand.sStartTask.eSchedulingMode = BDSP_P_SchedulingMode_eMaster;
        }

        if ((pArmTask->startSettings.schedulingMode== BDSP_TaskSchedulingMode_eSlave)&&(pArmTask->masterTaskId != BDSP_P_INVALID_TASK_ID))
        {
            psCommand->uCommand.sStartTask.ui32MasterTaskId = pArmTask->masterTaskId;
        }
        else
        {
            psCommand->uCommand.sStartTask.ui32MasterTaskId = BDSP_P_INVALID_TASK_ID;
        }

        psCommand->uCommand.sStartTask.ui32DramDeadlineConfigStructAddr = pArmTask->taskMemGrants.sTaskInfo.Buffer.offset;
        psCommand->uCommand.sStartTask.ui32DramTaskConfigAddr = pArmTask->taskMemGrants.sCitStruct.Buffer.offset;

		ui32PhysAddr = pDevice->sArmInterfaceQ.Memory.offset +
			((uint8_t *)pArmTask->hSyncMsgQueue->psQueuePointer - (uint8_t *)pDevice->sArmInterfaceQ.Memory.pAddr);
        psCommand->uCommand.sStartTask.ui32SyncQueueId = ui32PhysAddr;

		ui32PhysAddr = pDevice->sArmInterfaceQ.Memory.offset +
			((uint8_t *)pArmTask->hAsyncMsgQueue->psQueuePointer - (uint8_t *)pDevice->sArmInterfaceQ.Memory.pAddr);
        psCommand->uCommand.sStartTask.ui32AsyncQueueId = ui32PhysAddr;

        psCommand->uCommand.sStartTask.sDramStackBuffer.ui32DramBufferAddress = pArmTask->taskMemGrants.sStackSwapBuff.Buffer.offset;
        psCommand->uCommand.sStartTask.sDramStackBuffer.ui32BufferSizeInBytes = pArmTask->taskMemGrants.sStackSwapBuff.ui32Size;

        pTaskParams->ui32SamplingFrequency = 48000;
        pTaskParams->ui32SchedulingBufferThreshold = pArmTask->ctbOutput.ui32Threshold;
        pTaskParams->ui32BlockTime = pArmTask->ctbOutput.ui32BlockTime;

        /*TODO : ui32FrameSize is not required anymore by FW. remove this parameter */
        pTaskParams->ui32FrameSize = 1536;
        pTaskParams->eBufferType = BDSP_AF_P_BufferType_eFMM;

        /* Fill the Scheduling buffer information */
        if (pArmTask->schedulingMode == BDSP_TaskSchedulingMode_eSlave)
        {
            err = BDSP_Arm_P_FindSlaveTaskSchedulingBuffer(&pArmTask->startSettings, &pTaskParams->sDspSchedulingBuffInfo.ui32DramSchedulingBuffCfgAddr, &pTaskParams->eBufferType);
            if (BERR_SUCCESS != err)
            {
                BDBG_ERR(("Unable to find scheduling buffer for the slave task : %d", pArmTask->taskId));
                err = BERR_TRACE(err);
                goto end;
            }
            pTaskParams->ui32IndepDelay = 0;
            pTaskParams->ui32MaxIndepDelay = 0;
        }
        else
        {
            BDSP_CIT_P_FwStgSrcDstType         eSchedulingBufferType;
            BDSP_AF_P_sDRAM_CIRCULAR_BUFFER    sBufferPtr;
            BKNI_Memset(&sBufferPtr,0,sizeof(BDSP_AF_P_sDRAM_CIRCULAR_BUFFER));

            eSchedulingBufferType = BDSP_CIT_P_FwStgSrcDstType_eInvalid;
            BSTD_UNUSED(ui32RbufOffset);
            BSTD_UNUSED(uiSchedulingBufId);
            BSTD_UNUSED(eSchedulingBufferType);

            pTaskParams->ui32IndepDelay                = uiSchedulingBufDelay;
            pTaskParams->ui32SchedulingBufferThreshold = uiSchedulingBufferThreshold;

            /* For slave task ui32IndepDelay and ui32MaxIndepDelay should be 0*/
            pTaskParams->ui32MaxIndepDelay = pStartSettings->maxIndependentDelay;
        }

        pTaskParams->eFmmDstFsRate = eschedulingBufferRate;

        /* Feedback buffer setting */
        if ((pArmTask->schedulingMode== BDSP_TaskSchedulingMode_eMaster)
            ||(pArmTask->schedulingMode == BDSP_TaskSchedulingMode_eSlave))
        {
            pTaskParams->ui32MasterTaskFeedbackBuffCfgAddr = pArmTask->FeedbackBuffer.offset;
            pTaskParams->ui32MasterTaskFeedbackBuffValid = 1;
            BDBG_MSG(("%s: pTaskParams->ui32MasterTaskFeedbackBuffCfgAddr = 0x%x",(psCommand->uCommand.sStartTask.eSchedulingMode == BDSP_P_SchedulingMode_eSlave)?"slave":"master", pTaskParams->ui32MasterTaskFeedbackBuffCfgAddr));
        }
        else
        {
            pTaskParams->ui32MasterTaskFeedbackBuffCfgAddr = 0;
            pTaskParams->ui32MasterTaskFeedbackBuffValid = 0;
            BDBG_MSG(("%s: pTaskParams->ui32MasterTaskFeedbackBuffCfgAddr = 0x%x",(psCommand->uCommand.sStartTask.eSchedulingMode == BDSP_P_SchedulingMode_eSlave)?"slave":"master", pTaskParams->ui32MasterTaskFeedbackBuffCfgAddr));
        }

        BDSP_MMA_P_FlushCache(pArmTask->taskMemGrants.sTaskInfo.Buffer, pArmTask->taskMemGrants.sTaskInfo.ui32Size);
        psCommand->uCommand.sStartTask.ui32EventEnableMask = pArmTask->eventEnabledMask ;
        pArmTask->lastEventType = psCommand->sCommandHeader.ui32CommandID;
        BKNI_ResetEvent(pArmTask->hEvent);

        BDBG_MSG(("==========================================="));
        BDBG_MSG(("===========Command information============="));
        BDBG_MSG(("==========================================="));
        BDBG_MSG(("sCommand.uCommand.sStartTask.eTaskAlgoType = %d",psCommand->uCommand.sStartTask.eTaskAlgoType));
        BDBG_MSG(("sCommand.uCommand.sStartTask.eTaskType = %d",psCommand->uCommand.sStartTask.eTaskType));
        BDBG_MSG(("sCommand.uCommand.sStartTask.eSchedulingMode = %d",psCommand->uCommand.sStartTask.eSchedulingMode));
        BDBG_MSG(("sCommand.uCommand.sStartTask.ui32MasterTaskId = %d",psCommand->uCommand.sStartTask.ui32MasterTaskId));
        BDBG_MSG(("sCommand.uCommand.sStartTask.eDeadlineComputationFuncType = %d",psCommand->uCommand.sStartTask.eDeadlineComputationFuncType));
        BDBG_MSG(("sCommand.uCommand.sStartTask.ui32DramDeadlineConfigStructAddr = 0x%x",psCommand->uCommand.sStartTask.ui32DramDeadlineConfigStructAddr));
        BDBG_MSG(("sCommand.uCommand.sStartTask.ui32DramTaskConfigAddr = 0x%x",psCommand->uCommand.sStartTask.ui32DramTaskConfigAddr));
        BDBG_MSG(("sCommand.uCommand.sStartTask.ui32SyncQueueId = %d",psCommand->uCommand.sStartTask.ui32SyncQueueId));
        BDBG_MSG(("sCommand.uCommand.sStartTask.ui32AsyncQueueId = %d",psCommand->uCommand.sStartTask.ui32AsyncQueueId));
        BDBG_MSG(("sCommand.uCommand.sStartTask.sDramStackBuffer.ui32DramBufferAddress = 0x%x",psCommand->uCommand.sStartTask.sDramStackBuffer.ui32DramBufferAddress));
        BDBG_MSG(("sCommand.uCommand.sStartTask.sDramStackBuffer.ui32BufferSizeInBytes = %d",psCommand->uCommand.sStartTask.sDramStackBuffer.ui32BufferSizeInBytes));
        BDBG_MSG(("sCommand.uCommand.sStartTask.ui32EventEnableMask = %d",psCommand->uCommand.sStartTask.ui32EventEnableMask));
        BDBG_MSG(("sCommand.uCommand.sStartTask.ePPMCorrEnable = %d",psCommand->uCommand.sStartTask.ePPMCorrEnable));
        BDBG_MSG(("sCommand.uCommand.sStartTask.eOpenGateAtStart = %d",psCommand->uCommand.sStartTask.eOpenGateAtStart));
        BDBG_MSG(("==========================================="));

        err = BDSP_Arm_P_SendCommand(pDevice->hCmdQueue, psCommand,(void *)pArmTask);
        /*Accept the other Commands , After posting Start task Command */
        if (BERR_SUCCESS != err)
        {
            BDBG_ERR(("BDSP_Arm_P_StartTask: START_TASK creation failed!"));
            err = BERR_TRACE(err);
            goto end;
        }

        /* Wait for Ack_Response_Received event w/ timeout */
        err = BKNI_WaitForEvent(pArmTask->hEvent, BDSP_ARM_START_STOP_EVENT_TIMEOUT_IN_MS);
        if (BERR_TIMEOUT == err)
        {
            BDBG_ERR(("BDSP_Arm_P_StartTask: START_TASK ACK timeout!"));
            err = BERR_TRACE(err);
            goto end;
        }
        /* Send command for the task , Only if the ack for the Start of the task is recieved */
        pArmTask->isStopped = false;

        BSTD_UNUSED(eMsgType);

    }
    else
    {
        BDBG_ERR((" ************ Something has gone wrong and you are trying to start a task not of audio context on ARM ************"));
    }

    /* Traverse through all stages in the task and set the running flag and task handle */
    BDSP_ARM_STAGE_TRAVERSE_LOOP_BEGIN(pArmTask->startSettings.primaryStage->pStageHandle, pStageIterator)
    BSTD_UNUSED(macroStId);
    BSTD_UNUSED(macroBrId);
    {
        BDBG_ASSERT(NULL != pStageIterator);
        pStageIterator->running = true;
        pStageIterator->pArmTask = pArmTask;
    }
    BDSP_ARM_STAGE_TRAVERSE_LOOP_END(pStageIterator);
    goto end;
err_gen_citinput:
err_download_fw:
    pDevice->taskDetails.numActiveTasks--;
end:
    BKNI_Free(psCommand);
err_malloc_command:
    BDBG_LEAVE(BDSP_Arm_P_StartTask);
    return err;
}

BERR_Code BDSP_Arm_P_StopTask(
    void *pTaskHandle
    )
{
    BERR_Code   err = BERR_SUCCESS;
    BDSP_ArmTask *pArmTask = (BDSP_ArmTask *)pTaskHandle;
    BDSP_ArmContext *pArmContext = (BDSP_ArmContext *)pArmTask->pContext;
    BDSP_Arm  *pDevice = pArmContext->pDevice;

    BDSP_Arm_P_Command *psCommand;
    BDSP_Arm_P_Response sRsp;
    BDSP_P_MsgType      eMsgType;

    BDBG_ENTER(BDSP_Arm_P_StopTask);

    BDBG_OBJECT_ASSERT(pArmTask, BDSP_ArmTask);
    psCommand = BKNI_Malloc(sizeof(BDSP_Arm_P_Command));
    if ( NULL == psCommand )
    {
        err = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto err_malloc_command;
    }

    BKNI_Memset(psCommand,0,sizeof(*psCommand));
    BKNI_Memset(&sRsp,0,sizeof(sRsp));

    /* Traverse through all stages in the task and reset the running flag and task handle */
    BDSP_ARM_STAGE_TRAVERSE_LOOP_BEGIN(pArmTask->startSettings.primaryStage->pStageHandle, pStageIterator)
    BSTD_UNUSED(macroStId);
    BSTD_UNUSED(macroBrId);
    {
        pStageIterator->running = false;
        pStageIterator->pArmTask = NULL;
        if(pArmContext->contextWatchdogFlag != true)
        {
            /* Don't Release the Algorithm explicity here in Watchdog recovery.
                 All the algorithms are released together in  Open during such recovery.
                 In a normal routine each task will release their algorithms individually*/
            BDSP_Arm_P_ReleaseAlgorithm(pDevice, pStageIterator->algorithm);
        }
    }
    BDSP_ARM_STAGE_TRAVERSE_LOOP_END(pStageIterator)

    if(BDSP_ContextType_eAudio!= pArmContext->settings.contextType)
    {
        BDBG_ERR(("BDSP_Arm_P_StopTask: Trying to stop a non audio task on ARM"));
        err = BERR_TRACE(BERR_INVALID_PARAMETER);
        goto end;
    }
    else
    {
        if (pArmContext->contextWatchdogFlag == false)
        {

            /* Send stop Task Command */
            psCommand->sCommandHeader.ui32CommandID = BDSP_STOP_TASK_COMMAND_ID;
            psCommand->sCommandHeader.ui32CommandCounter = pArmTask->commandCounter++;
            psCommand->sCommandHeader.ui32TaskID = pArmTask->taskId;
            psCommand->sCommandHeader.eResponseType = BDSP_P_ResponseType_eResponseRequired;
            psCommand->sCommandHeader.ui32CommandSizeInBytes =  sizeof(BDSP_Arm_P_Command);

            if (pArmTask->masterTaskId != BDSP_P_INVALID_TASK_ID)
            {
                psCommand->uCommand.sStopTask.eSchedulingMode = BDSP_P_SchedulingMode_eSlave;
                psCommand->uCommand.sStopTask.ui32MasterTaskId = pArmTask->masterTaskId;
            }
            else
            {
                psCommand->uCommand.sStopTask.eSchedulingMode = BDSP_P_SchedulingMode_eMaster;
                psCommand->uCommand.sStopTask.ui32MasterTaskId = BDSP_P_INVALID_TASK_ID;
            }

            if (pArmTask->startSettings.realtimeMode == BDSP_TaskRealtimeMode_eNonRealTime)
            {
                psCommand->uCommand.sStopTask.eTaskType = BDSP_P_TaskType_eNonRealtime;
            }
            else
            {
                psCommand->uCommand.sStopTask.eTaskType = BDSP_P_TaskType_eRealtime;
            }

            pArmTask->lastEventType = psCommand->sCommandHeader.ui32CommandID;

            BKNI_ResetEvent(pArmTask->hEvent);

            err = BDSP_Arm_P_SendCommand(pDevice->hCmdQueue, psCommand,(void *)pArmTask);
            /*Accept the other Commands , After posting Start task Command */
            if (BERR_SUCCESS != err)
            {
                BDBG_ERR(("BDSP_Arm_P_StopTask: STOP_TASK creation failed!"));
                err = BERR_TRACE(err);
                goto end;
            }

            /* Wait for Ack_Response_Received event w/ timeout */
            err = BKNI_WaitForEvent(pArmTask->hEvent, BDSP_ARM_START_STOP_EVENT_TIMEOUT_IN_MS);
            if (BERR_TIMEOUT == err)
            {
                BDBG_ERR(("BDSP_Arm_P_StopTask: STOP_TASK ACK timeout!"));
                err = BERR_TRACE(err);
                goto end;
            }
            /* Send command for the task , Only if the ack for the Start of the task is recieved */
            pArmTask->isStopped = true;
            BSTD_UNUSED(eMsgType);

        }
        else
        {
            pArmTask->isStopped = true;
        }

    }

    {
        BDSP_ARM_CIT_P_Output       *psCitOp   = &(pArmTask->citOutput);
        BDSP_ARM_AF_P_sNODE_CONFIG  *psNodeCfg = &(psCitOp->sCit.sNodeConfig[1]);
		BDSP_MMA_Memory Memory;
        BERR_Code   err1 = BERR_SUCCESS;
        BDSP_ArmStage *pArmStage = (BDSP_ArmStage *)pArmTask->startSettings.primaryStage->pStageHandle;
        BDSP_MAP_Table_Entry MapTable[((BDSP_ARM_MAX_ALLOC_STAGE>BDSP_ARM_MAX_ALLOC_TASK)? BDSP_ARM_MAX_ALLOC_STAGE: BDSP_ARM_MAX_ALLOC_TASK)];
        uint32_t ui32NumEntries = 0;

        /*Added Specially for ARM */
        BKNI_Memset(MapTable,0,(BDSP_ARM_MAX_ALLOC_STAGE*sizeof(BDSP_MAP_Table_Entry)));
        BDSP_Arm_P_RetrieveEntriesToUnMap(&(pArmStage->sStageMapTable[0]), &MapTable[0], &ui32NumEntries, BDSP_ARM_MAX_ALLOC_STAGE);
        err = BDSP_Arm_P_SendUnMapCommand(pArmStage->pContext->pDevice, &MapTable[0], ui32NumEntries);
        if (BERR_SUCCESS != err)
        {
            BDBG_ERR(("BDSP_Arm_P_StopTask: Send ARM UNMAP Command failed!!!!"));
        }
		Memory.pAddr = pDevice->imgCache[BDSP_ARM_IMG_ID_TABLE(psNodeCfg->eAlgoId)].pMemory;
		Memory.offset=	pDevice->imgCache[BDSP_ARM_IMG_ID_TABLE(psNodeCfg->eAlgoId)].offset;
		err1 = BDSP_Arm_P_DeleteEntry_MapTable(
			&(pArmStage->sStageMapTable[0]),
			&Memory,
			BDSP_ARM_MAX_ALLOC_STAGE);
        if (BERR_SUCCESS != err1)
        {
            BDBG_ERR(("BDSP_Arm_P_StopTask: Unable to delete entry form the Table for LUT of NODE-1 which was a hack put-in in NODE CONFIG"));
        }

        if(pArmContext->contextWatchdogFlag == true)
        {
            BKNI_Memset(MapTable,0,(BDSP_ARM_MAX_ALLOC_TASK*sizeof(BDSP_MAP_Table_Entry)));
            BDSP_Arm_P_RetrieveEntriesToUnMap(&(pArmTask->sTaskMapTable[0]), &MapTable[0], &ui32NumEntries, BDSP_ARM_MAX_ALLOC_TASK);
            err = BDSP_Arm_P_SendUnMapCommand(pArmTask->pContext->pDevice, &MapTable[0], ui32NumEntries);
            if (BERR_SUCCESS != err)
            {
                BDBG_ERR(("BDSP_Arm_P_StopTask: Send ARM UNMAP Command for task failed!!!!"));
            }
        }

    }
    pArmTask->commandCounter     = 0;

    if((pArmContext->contextWatchdogFlag == false)||(pDevice->settings.authenticationEnabled == true))
    {
        /* During normal scenario, unregister the task from the device where it is running on and clear the start settings*/
        pDevice->taskDetails.pArmTask[BDSP_ARM_GET_TASK_INDEX(pArmTask->taskId)]  = NULL;
        BKNI_AcquireMutex(pDevice->taskDetails.taskIdMutex);
        pDevice->taskDetails.taskId[BDSP_ARM_GET_TASK_INDEX(pArmTask->taskId)]   = true;
        BKNI_ReleaseMutex(pDevice->taskDetails.taskIdMutex);
        BKNI_Memset((void *)&pArmTask->startSettings, 0, sizeof(pArmTask->startSettings));
    }

    pArmTask->isStopped = true;
    pArmTask->lastEventType= 0xFFFF;
    pArmTask->masterTaskId= BDSP_P_INVALID_TASK_ID;
    pArmTask->commandCounter =0;
    /*pArmTask->paused=false;
    pArmTask->decLocked=false;*/

    if((pDevice->deviceWatchdogFlag == false) && (pArmContext->contextWatchdogFlag == true))
    {
        /* This is done to take care of stop task which is in watchdog recovery but device open has already happened.
             Imagine the scenario of both VEE and APE involved.
                   Book keeping functionalities like unregistering themselves from the Device is already happened at RaagaOpen
                   Stop Task now just need to clear the startsettings */
            BKNI_Memset((void *)&pArmTask->startSettings, 0, sizeof(pArmTask->startSettings));
    }


    BKNI_DestroyEvent(pArmTask->hEvent);
end:
    BKNI_Free(psCommand);
err_malloc_command:
    BDBG_LEAVE(BDSP_Arm_P_StopTask);
    return err;
}

/***********************************************************************
Name        :   BDSP_Arm_P_GetAlgorithmDefaultSettings

Type        :   PI Interface

Input       :   pDeviceHandle -Device Handle which needs to be closed.
                algorithm - Algorithms for which default userconfig is requested by PI.
                pSettingsBuffer - Pointer where default userconfig is returned to PI.
                settingsBufferSize - Size of the default userconfig is requested by PI.

Return      :   None

Functionality   :   Return the Default UserConfig of the algorithm requested by the PI.
***********************************************************************/

void BDSP_Arm_P_GetAlgorithmDefaultSettings(
    BDSP_Algorithm algorithm,
    BDSP_MMA_Memory *pMemory,
    size_t settingsBufferSize
    )
{
    const BDSP_Arm_P_AlgorithmInfo *pAlgoInfo;
	BDBG_ASSERT(NULL != pMemory->pAddr);
    BDBG_ASSERT(algorithm < BDSP_Algorithm_eMax);

    pAlgoInfo = BDSP_Arm_P_LookupAlgorithmInfo(algorithm);

    if(pAlgoInfo->userConfigSize != settingsBufferSize)
    {
        BDBG_ERR(("settingsBufferSize (%d) is not equal to Config size (%d) of Algorithm %s",
            (unsigned int)settingsBufferSize,(unsigned int)pAlgoInfo->userConfigSize,
            pAlgoInfo->pName));
        BDBG_ASSERT(0);;
    }

    BKNI_Memcpy(pMemory->pAddr, (void *)pAlgoInfo->pDefaultUserConfig, settingsBufferSize);
    BDSP_MMA_P_FlushCache((*pMemory), settingsBufferSize);
}


/***********************************************************************
Name        :   BDSP_Arm_P_SetAlgorithm

Type        :   PI Interface

Input       :   pStageHandle        -   Handle of the Stage whose algorithm needs to be Set.
                algorithm           -   Algorithm to be Set for the Stage

Return      :   Error Code to return SUCCESS or FAILURE

Functionality   :   Following are the operations performed.
        1)  Retreive the information for algorithm provided by the PI and the algorithm already set for the Stage.
        2)  Check if algorithm is supported or if Stage is already running.
        3)  Check if total inputs and outputs are non-zero for the Stage.
        4)  Check if there is an attempt to change the Algorithm Type for the stage.
        5)  Load the default algorithm settings for the stage.
        6)  Load the default ids settings if IDS is required for the algorithm.
        7)  Reset the Status buffer for the Algorithm.
***********************************************************************/
BERR_Code BDSP_Arm_P_SetAlgorithm(
    void *pStageHandle,
    BDSP_Algorithm algorithm
    )
{
    BERR_Code   err = BERR_SUCCESS;
    BDSP_ArmStage *pArmStage;
    const BDSP_Arm_P_AlgorithmInfo *pAlgoInfo;
    const BDSP_Arm_P_AlgorithmInfo *pCurrAlgoInfo;
    bool validType;

    BDBG_ENTER(BDSP_Arm_P_SetAlgorithm);
    BDBG_ASSERT(NULL != pStageHandle);
    pArmStage = (BDSP_ArmStage *)pStageHandle;
    pAlgoInfo = BDSP_Arm_P_LookupAlgorithmInfo(algorithm);
    pCurrAlgoInfo = BDSP_Arm_P_LookupAlgorithmInfo(pArmStage->algorithm);

    BDBG_MSG(("Setting Algorithm to %s(%u) for stage %p", pAlgoInfo->pName,algorithm, (void *)pStageHandle));

    /*  Returns error if the algorithm is not supported */
    if (!(pAlgoInfo->supported))
    {
        BDBG_ERR(("BDSP_Algorithm (%d) not supported", algorithm));
        err = BERR_TRACE(BERR_NOT_SUPPORTED);
        goto end;
    }

    if (pArmStage->totalOutputs)
    {
        BDBG_ERR(("Stage has non-zero (%d) output connections at set algorithm", pArmStage->totalOutputs));
        err = BERR_TRACE(BERR_NOT_SUPPORTED);
        goto end;
    }

    if (pArmStage->totalInputs)
    {
        BDBG_ERR(("Stage has non-zero (%d) input connections at set algorithm", pArmStage->totalInputs));
        err = BERR_TRACE(BERR_NOT_SUPPORTED);
        goto end;
    }

    /*  Return error if the stage is running.*/
    if (pArmStage->running)
    {
        BDBG_ERR(("Cannot set algorithm when the stage is running : stage handle = %p : Current algo = %s",
                   (void *)pStageHandle, pCurrAlgoInfo->pName));
        err = BERR_TRACE(BERR_NOT_SUPPORTED);
        goto end;
    }

    if( !pArmStage->settings.algorithmSupported[algorithm])
    {
        BDBG_ERR((" algorithm %s (%d) being passed in %s was not enabled during CreateStage call ",
                    pAlgoInfo->pName,algorithm, BSTD_FUNCTION));
        err = BERR_TRACE(BERR_NOT_SUPPORTED);
        goto end;
    }

    /*  Return error if there is an attempt to change the stage type */
    if ( pArmStage->settings.algoType == BDSP_AlgorithmType_eAudioDecode )
    {
        switch ( pAlgoInfo->type )
        {
        case BDSP_AlgorithmType_eAudioDecode:
        case BDSP_AlgorithmType_eAudioPassthrough:
            validType = true;
            break;
        default:
            validType = false;
            break;
        }
    }
    else
    {
        validType = (pArmStage->settings.algoType == pAlgoInfo->type) ? true : false;
    }
    if ( !validType )
    {
        BDBG_ERR(("Cannot change the algo type of the stage from %d (%s) to %d (%s)",
                    pCurrAlgoInfo->type, pCurrAlgoInfo->pName, pAlgoInfo->type, pAlgoInfo->pName));
        err = BERR_TRACE(BERR_NOT_SUPPORTED);
        goto end;
    }

    pArmStage->algorithm = pAlgoInfo->algorithm;

    if(pAlgoInfo->userConfigSize)
    {
        /* Load the default algorithm settings */
        BDSP_Arm_P_GetAlgorithmDefaultSettings(pArmStage->algorithm,
                    &(pArmStage->sDramUserConfigBuffer.Buffer),
                    pAlgoInfo->userConfigSize);
    }

    /* Load the default ids settings */
    if (pAlgoInfo->idsConfigSize)
    {
        BDSP_MMA_Memory Memory;
        Memory = pArmStage->sDramUserConfigBuffer.Buffer;
        Memory.pAddr = (void *)(((uintptr_t)Memory.pAddr)+pArmStage->sFrameSyncOffset.ui32UserCfgOffset);

        BKNI_Memcpy(Memory.pAddr, (void *)pAlgoInfo->pDefaultIdsConfig, pAlgoInfo->idsConfigSize);
        BDSP_MMA_P_FlushCache(Memory, pAlgoInfo->idsConfigSize);

        /* Initilaise both the HOST and DSP Frame Sync Settings to maintain consistence while the Stage is Stopped.
         Get Settings retrieves the data from HOST buffer(spare) and if this data is not initialised then, there may be case
         of junk data fed to the FW. */

        Memory = pArmStage->sDramUserConfigSpareBuffer.Buffer;
        Memory.pAddr = (void *)(((uintptr_t)Memory.pAddr)+pArmStage->sFrameSyncOffset.ui32UserCfgOffset);

        BKNI_Memcpy(Memory.pAddr, (void *)pAlgoInfo->pDefaultIdsConfig, pAlgoInfo->idsConfigSize);
        BDSP_MMA_P_FlushCache(Memory, pAlgoInfo->idsConfigSize);
    }

    /* Reset the Status buffer */
	if( pArmStage->sDramStatusBuffer.ui32Size)
	{
        BKNI_Memset(pArmStage->sDramStatusBuffer.Buffer.pAddr, 0xFF, pArmStage->sDramStatusBuffer.ui32Size);
        BDSP_MMA_P_FlushCache(pArmStage->sDramStatusBuffer.Buffer, pArmStage->sDramStatusBuffer.ui32Size);
	}

end:
    BDBG_LEAVE(BDSP_Arm_P_SetAlgorithm);
    return err;
}

BERR_Code BDSP_Arm_P_GetDownloadStatus(
    void  *pDeviceHandle,
    BDSP_DownloadStatus *pStatus /* [out] */
    )
{

    BDSP_Arm *pDevice = (BDSP_Arm *)pDeviceHandle;

    BDBG_ENTER(BDSP_Arm_P_GetDownloadStatus);
    /* Assert the function arguments*/
    BDBG_OBJECT_ASSERT(pDevice, BDSP_Arm);

    /*If Firmware Firmware authentication is Disabled*/
    if(pDevice->settings.authenticationEnabled==false)
    {
        BDBG_ERR(("BDSP_Arm_P_GetDownloadStatus should be called only if bFwAuthEnable is true"));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

	pStatus->pBaseAddress = pDevice->pFwHeapMemory;
	pStatus->physicalAddress = pDevice->FwHeapOffset;
    /*Size of the executable download */
    pStatus->length = pDevice->fwHeapSize;

    BDBG_LEAVE(BDSP_Arm_P_GetDownloadStatus);

    return BERR_SUCCESS;
}

BERR_Code BDSP_Arm_P_Initialize(
    void  *pDeviceHandle
)
{
    BERR_Code rc = BERR_SUCCESS ;
    BDSP_Arm *pDevice = (BDSP_Arm *)pDeviceHandle;
    BDSP_MAP_Table_Entry MapTable[BDSP_ARM_MAX_ALLOC_DEVICE];
    uint32_t ui32NumEntries = 0;

    BDBG_ENTER(BDSP_Arm_P_Initialize);
    /* Assert the function arguments*/
    BDBG_OBJECT_ASSERT(pDevice, BDSP_Arm);

    /*If Firmware Firmware authentication is Disabled*/
    if(pDevice->settings.authenticationEnabled==false)
    {
        BDBG_ERR(("BDSP_Arm_StartDsp should be called only if bFwAuthEnable is true"));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
	rc = BDSP_Arm_P_StartHbcMonitor(pDevice);
	if (BERR_SUCCESS != rc) {
			BDBG_ERR(("failed to start hbc_monitor"));
			goto err_hbc_start;
	}

	/* Write the downloaded application code into Astra secure memory */
	rc = BDSP_Arm_P_DownloadSystemCodeToAstra(pDevice->armDspApp.hClient,pDevice,BDSP_ARM_SystemImgId_eSystemCode);
	if (BERR_SUCCESS != rc) {
		BDBG_ERR(("failed to start armdsp_system"));
		goto err_peer_start;
	}

    rc = BTEE_Application_Open(
        pDevice->armDspApp.hClient,
        "armdsp_system",
        "armdsp_system.elf",
        &pDevice->armDspApp.hApplication);

    if (BERR_SUCCESS != rc) {
        BDBG_ERR(("failed to start armdsp_system"));
        goto err_peer_start;
    }
    BKNI_Memset(MapTable,0,(BDSP_ARM_MAX_ALLOC_DEVICE*sizeof(BDSP_MAP_Table_Entry)));
    BDSP_Arm_P_RetrieveEntriesToMap(&(pDevice->sDeviceMapTable[0]), &MapTable[0], &ui32NumEntries, BDSP_ARM_MAX_ALLOC_DEVICE);
    rc = BDSP_Arm_P_SendMapCommand(pDevice, &MapTable[0], ui32NumEntries);
    if (BERR_SUCCESS != rc)
    {
        BDBG_ERR(("BDSP_Arm_P_Initialize: Send ARM MAP Command failed!!!!"));
        goto err_send_map_cmd;
    }

err_hbc_start:
err_peer_start:
err_send_map_cmd:

    BDBG_LEAVE(BDSP_Arm_P_Initialize);
    return rc;
}

BERR_Code BDSP_Arm_P_StageGetContext(
    void *pStageHandle,
    BDSP_ContextHandle *pContextHandle /* [out] */
    )
{
	BERR_Code errCode = BERR_SUCCESS;
	BDSP_ArmStage *pArmStage = (BDSP_ArmStage *)pStageHandle;

	BDBG_ENTER(BDSP_Arm_P_StageGetContext);
	BDBG_OBJECT_ASSERT(pArmStage, BDSP_ArmStage);

	*pContextHandle = &pArmStage->pContext->context;
	BDBG_LEAVE(BDSP_Arm_P_StageGetContext);
	return errCode;
}

/***********************************************************************
Name        :   BDSP_Arm_P_AudioCaptureCreate

Type        :   PI Interface

Input       :   pContextHandle      -   Handle of the Context for which the capture needs to created.
				pCaptureCreateSettings  -   Settings of the Creating the capture.
				pCapture                -   Handle of the Capture which is returned back to PI.

Return      :   Error Code to return SUCCESS or FAILURE

Functionality   :   T.B.D
***********************************************************************/

BERR_Code BDSP_Arm_P_AudioCaptureCreate(
	void *pContextHandle,
	const BDSP_AudioCaptureCreateSettings *pCaptureCreateSettings,
	BDSP_AudioCaptureHandle *pCapture)
{
	BERR_Code errCode = BERR_SUCCESS;
	BDSP_ArmContext *pArmContext = (BDSP_ArmContext *)pContextHandle;
	BDSP_ArmCapture *pArmCapture;

    BDBG_ENTER(BDSP_Arm_P_AudioCaptureCreate);
	BDBG_OBJECT_ASSERT(pArmContext, BDSP_ArmContext);
	*pCapture = NULL;

	/* Allocate capture structure and add it to the task's capture list */
	pArmCapture = (BDSP_ArmCapture *)BKNI_Malloc(sizeof(BDSP_ArmCapture));
	if ( NULL == pArmCapture )
	{
		errCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
		goto err_malloc_stage;
	}

	/* Update the capture information using the capture create settings */
	BKNI_Memset((void *)pArmCapture, 0, sizeof(*pArmCapture));
	pArmCapture->capture.pCapHandle = pArmCapture;
	/* Use the heap provided */
	if (pCaptureCreateSettings->hHeap)
	{
		pArmCapture->hHeap = pCaptureCreateSettings->hHeap;
	}
	else
	{
		pArmCapture->hHeap = pArmContext->pDevice->memHandle;
	}

	pArmCapture->pDevice = pArmContext->pDevice;

	errCode = BDSP_Arm_P_InitAudioCaptureInfo(pArmCapture, pCaptureCreateSettings);

	pArmCapture->capture.destroy = BDSP_Arm_P_AudioCaptureDestroy;
	pArmCapture->capture.addToStage = BDSP_Arm_P_AudioCaptureAddToStage;
	pArmCapture->capture.removeFromStage = BDSP_Arm_P_AudioCaptureRemoveFromStage;
	pArmCapture->capture.getBuffer = BDSP_Arm_P_AudioCaptureGetBuffer;
	pArmCapture->capture.consumeData = BDSP_Arm_P_AudioCaptureConsumeData;

	BDBG_OBJECT_SET(pArmCapture, BDSP_ArmCapture);

	*pCapture = &pArmCapture->capture;

err_malloc_stage:
    BDBG_LEAVE(BDSP_Arm_P_AudioCaptureCreate);
	return errCode;
}

void BDSP_Arm_P_AudioCaptureDestroy (
	void *pCapHandle
	)
{
	BDSP_ArmCapture *pArmCapture = pCapHandle;

    BDBG_ENTER(BDSP_Arm_P_AudioCaptureDestroy);
	BDBG_OBJECT_ASSERT(pArmCapture, BDSP_ArmCapture);

	if ( NULL != pArmCapture->pStage)
	{
		BDBG_ERR(("Please call BDSP_AudioTask_RemoveCapture() before calling BDSP_AudioCapture_Destroy()"));
		BDBG_ASSERT(NULL == pArmCapture->pStage);
	}

	pArmCapture->enabled = false;
	BDSP_MMA_P_FreeMemory(&pArmCapture->captureBuffer);
	BKNI_Free(pArmCapture);
    BDBG_LEAVE(BDSP_Arm_P_AudioCaptureDestroy);
}

BERR_Code BDSP_Arm_P_AudioCaptureAddToStage(
	void *pCapHandle,
	void *pStageHandle,
	unsigned outputId,
	const BDSP_StageAudioCaptureSettings *pSettings
	)
{
	BERR_Code err = BERR_SUCCESS;
	int j;
	BDSP_ArmCapture *pArmCapture = pCapHandle;
	BDSP_ArmStage *pArmStage = pStageHandle;
	BDSP_AF_P_CIRCULAR_BUFFER *pBuffer;

    BDBG_ENTER(BDSP_Arm_P_AudioCaptureAddToStage);
	BDBG_ASSERT(NULL != pSettings);
	BDBG_ASSERT(NULL != pArmCapture);
	if ( outputId >= BDSP_AF_P_MAX_OP_FORKS )
	{
		return BERR_TRACE(BERR_INVALID_PARAMETER);
	}

	BKNI_AcquireMutex(pArmCapture->pDevice->captureMutex);
	BLST_S_INSERT_HEAD(&pArmStage->captureList, pArmCapture, node);

	/* Populate the output buffer type, num buffers and the output buffer pointers */
	err = BDSP_Arm_P_GetAudioOutputPointers(&pArmStage->sStageOutput[outputId], pArmCapture);
	if (err != BERR_SUCCESS)
	{
		BLST_S_REMOVE(&pArmStage->captureList, pArmCapture, BDSP_ArmCapture, node);
		BKNI_ReleaseMutex(pArmCapture->pDevice->captureMutex);
		BDBG_ERR(("Cannot add capture for an invalid output of the stage (%p)", (void *)pArmStage));
		return BERR_TRACE(BERR_NOT_SUPPORTED);
	}

	pArmCapture->pStage = pArmStage;
	pArmCapture->stageDestroyed = false;

	BKNI_ReleaseMutex(pArmCapture->pDevice->captureMutex);

	pArmCapture->updateRead = pSettings->updateRead;

	/* Initialize the read/write pointers to base */
	for (j = 0; j < (int)BDSP_AF_P_MAX_CHANNELS; j++)
	{
		pBuffer = &pArmCapture->capPtrs[j].captureBufferPtr;
		pBuffer->pReadPtr = pBuffer->pBasePtr;
		pBuffer->pWritePtr = pBuffer->pBasePtr;
	}

	/* Set the shadow read pointers and last write pointer */
	switch (pArmCapture->eBuffType)
	{
		case BDSP_AF_P_BufferType_eFMM:
		case BDSP_AF_P_BufferType_eRDB:
		case BDSP_AF_P_BufferType_eRAVE:
			for(j=0; j<(int)pSettings->numChannelPair; j++)
			{
				/* Since the allocation in APE was for pair of channels, BDSP deals with individual channels. Hence splitting the buffer*/
				pArmCapture->capPtrs[2*j].OutputBufferMemory = pSettings->channelPairInfo[j].outputBuffer;
				pArmCapture->capPtrs[2*j+1].OutputBufferMemory = pSettings->channelPairInfo[j].outputBuffer;
				pArmCapture->capPtrs[2*j+1].OutputBufferMemory.pAddr =
					(void *)((uint8_t *)pArmCapture->capPtrs[2*j+1].OutputBufferMemory.pAddr + (pSettings->channelPairInfo[j].bufferSize/2));
				pArmCapture->capPtrs[2*j+1].OutputBufferMemory.offset =
					(pArmCapture->capPtrs[2*j+1].OutputBufferMemory.offset + (pSettings->channelPairInfo[j].bufferSize/2));
			}
			for (j = 0; j < pArmCapture->numBuffers; j++)
			{
				pArmCapture->capPtrs[j].shadowRead = BREG_Read32(
					pArmCapture->pStage->pContext->pDevice->regHandle,
					pArmCapture->capPtrs[j].outputBufferPtr.ui32BaseAddr);
				pArmCapture->capPtrs[j].lastWrite = BREG_Read32(
					pArmCapture->pStage->pContext->pDevice->regHandle,
					pArmCapture->capPtrs[j].outputBufferPtr.ui32WriteAddr);
			}
			break;
		case BDSP_AF_P_BufferType_eDRAM:
		default:
			for (j = 0; j < pArmCapture->numBuffers; j++)
			{
				pArmCapture->capPtrs[j].shadowRead = pArmCapture->capPtrs[j].outputBufferPtr.ui32BaseAddr;
				pArmCapture->capPtrs[j].lastWrite = pArmCapture->capPtrs[j].outputBufferPtr.ui32WriteAddr;
			}
			break;
	}

	/* Enable the capture */
	pArmCapture->enabled = true;

    BDBG_LEAVE(BDSP_Arm_P_AudioCaptureAddToStage);
	return BERR_SUCCESS;
}

void BDSP_Arm_P_AudioCaptureRemoveFromStage(
	void *pCapHandle,
	void *pStageHandle
	)
{
	BDSP_ArmCapture *pArmCapture = pCapHandle;
	BDSP_ArmStage *pArmStage = pStageHandle;

	BSTD_UNUSED(pArmStage);

    BDBG_ENTER(BDSP_Arm_P_AudioCaptureRemoveFromStage);
	BDBG_OBJECT_ASSERT(pArmCapture, BDSP_ArmCapture);
	/*BDBG_ASSERT(pStageHandle == pArmCapture->pStage);*/

	BKNI_AcquireMutex(pArmCapture->pDevice->captureMutex);
	/*BLST_S_REMOVE(&pArmStage->captureList, pArmCapture, BDSP_ArmCapture, node);*/
	if(pArmCapture->stageDestroyed != true)
	{
		BLST_S_REMOVE(&pArmCapture->pStage->captureList, pArmCapture, BDSP_ArmCapture, node);
	}
	BKNI_ReleaseMutex(pArmCapture->pDevice->captureMutex);

	pArmCapture->enabled = false;
	pArmCapture->pStage = NULL;
    BDBG_LEAVE(BDSP_Arm_P_AudioCaptureRemoveFromStage);
}

static bool BDSP_Arm_P_CheckAudioCaptureIsReady(
	void *pCapHandle,
	BREG_Handle hReg)
{

	BDSP_ArmCapture *pArmCapture = pCapHandle;
	bool retval = true;
	int j = 0;
	dramaddr_t  ui32WriteAddr, ui32StartWriteAddr;

    BDBG_ENTER(BDSP_Arm_P_CheckAudioCaptureIsReady);

	for(j= 0; j<pArmCapture->numBuffers; j++)
	{
		ui32StartWriteAddr = BREG_Read32(hReg, pArmCapture->capPtrs[j].ui32StartWriteAddr);
		ui32WriteAddr = BREG_Read32(hReg, pArmCapture->capPtrs[j].outputBufferPtr.ui32WriteAddr);

		if(ui32WriteAddr >= ui32StartWriteAddr)
		{
				retval &= true;
		}
		else
		{
				retval &= false;
		}
    }

    BDBG_LEAVE(BDSP_Arm_P_CheckAudioCaptureIsReady);
	return retval;
}


/***********************************************************************
Name        :   BDSP_Arm_P_ProcessAudioCapture

Type        :   PI Interface

Input       :   pDevice - Void pointer of the device to be typecasted and used.

Return      :   Error Code to return SUCCESS or FAILURE

Functionality   :   T.B.D
***********************************************************************/

BERR_Code BDSP_Arm_P_ProcessAudioCapture(
	void *pDevice /* [in] device handle */
	)
{
	int j;
	BERR_Code err = BERR_SUCCESS;

	BDSP_Arm *pArmDevice = (BDSP_Arm *)pDevice;
	BDSP_ArmTask *pArmTask = NULL;
	BDSP_ArmContext *pArmContext = NULL;
	BDSP_ArmCapture *pArmCapture = NULL;

	bool retval;
	size_t opBuffDepth, capBuffSpace, bytesToCopy;
	uint8_t  *pCaptureWrite, *pCaptureRead, *pCaptureBase;

	BDSP_MMA_Memory ReadMemory, WriteMemory;

    BDBG_ENTER(BDSP_Arm_P_ProcessAudioCapture);
	BDBG_ASSERT(NULL != pArmDevice);

	/* Need to iterate through all contexts -> tasks -> captures and
	copy the data from output buffers to capture buffers */

	BKNI_AcquireMutex(pArmDevice->captureMutex);
	for ( pArmContext = BLST_S_FIRST(&pArmDevice->contextList);
		pArmContext != NULL;
		pArmContext = BLST_S_NEXT(pArmContext, node) )
	{
		for ( pArmTask = BLST_S_FIRST(&pArmContext->allocTaskList);
			pArmTask != NULL;
			pArmTask = BLST_S_NEXT(pArmTask, node) )
		{
			if (!(pArmTask->isStopped))
			{
				BDSP_ARM_STAGE_TRAVERSE_LOOP_BEGIN(pArmTask->startSettings.primaryStage->pStageHandle, pStageIterator)
				BSTD_UNUSED(macroStId);
				BSTD_UNUSED(macroBrId);
				{
					for ( pArmCapture = BLST_S_FIRST(&pStageIterator->captureList);
						pArmCapture != NULL;
						pArmCapture = BLST_S_NEXT(pArmCapture, node) )
					{
						if (pArmCapture->enabled)
						{

                            if(pArmCapture->eBuffType == BDSP_AF_P_BufferType_eFMM && pArmCapture->StartCapture == false)
                            {
                                pArmCapture->StartCapture = BDSP_Arm_P_CheckAudioCaptureIsReady(pArmCapture,
                                                                        pArmCapture->pStage->pContext->pDevice->regHandle);
								if (true == pArmCapture->StartCapture)
								{
										BDBG_MSG(("BDSP_Arm_P_AudioCaptureProcessing: Initilaise the Shadow Read and Last write to Start Write"));
										for (j = 0; j < pArmCapture->numBuffers; j++)
										{
												pArmCapture->capPtrs[j].shadowRead = BREG_Read32(
																pArmCapture->pStage->pContext->pDevice->regHandle,
																pArmCapture->capPtrs[j].ui32StartWriteAddr);
												pArmCapture->capPtrs[j].lastWrite = pArmCapture->capPtrs[j].shadowRead;
										}
								}
                            }
                            else
                            {
                                pArmCapture->StartCapture = true;

                            }
                            if(true == pArmCapture->StartCapture)
                            {
                                for (j = 0; j < pArmCapture->numBuffers; j++)
                                {
                                    /* Detect read crossing shadow read and print debug error */
                                    retval = BDSP_Arm_P_DetectAudioCaptureError(&pArmCapture->capPtrs[j].outputBufferPtr,
                                                                     pArmCapture->capPtrs[j].shadowRead,
                                                                     pArmCapture->capPtrs[j].lastWrite,
                                                                     pArmCapture->eBuffType,
                                                                     pArmCapture->pStage->pContext->pDevice->regHandle);

                                    if (retval)
                                    {
                                        BDBG_ERR(("!!! Capture error detected for buffer [%d] in capture %p", j, (void *)pArmCapture));
                                        continue;
                                    }

                                    /* GetBufferDepth of output buffer */
                                    opBuffDepth = BDSP_Arm_P_GetAudioBufferDepthLinear(&pArmCapture->capPtrs[j].outputBufferPtr,
                                                                            pArmCapture->capPtrs[j].shadowRead,
                                                                            &pArmCapture->capPtrs[j].lastWrite,
                                                                            pArmCapture->eBuffType,
                                                                            pArmCapture->pStage->pContext->pDevice->regHandle);

                                    /* GetFreeSpace to end in the capture buffer */
                                    capBuffSpace = BDSP_Arm_P_GetAudioCaptureBufferFreeSpaceLinear(&pArmCapture->capPtrs[j].captureBufferPtr);

                                    /* Copy the minimum */
                                    bytesToCopy = (opBuffDepth < capBuffSpace) ? opBuffDepth : capBuffSpace;

                                    /* Copy the data from the output buffers to the capture buffers */
                                    pCaptureWrite = pArmCapture->capPtrs[j].captureBufferPtr.pWritePtr;

                                    /* Convert physical offset to virtual address */
                                    if (pArmCapture->eBuffType == BDSP_AF_P_BufferType_eFMM)
                                    {
                                        /* Clear the wrap bit while copying the data from FMM buffers */
                                        ReadMemory = pArmCapture->capPtrs[j].OutputBufferMemory;
                                        ReadMemory.pAddr = (void *)((uint8_t *)ReadMemory.pAddr +
                                                ((pArmCapture->capPtrs[j].shadowRead & 0x7FFFFFFF) -
                                                BREG_Read32(pArmDevice->regHandle, pArmCapture->capPtrs[j].outputBufferPtr.ui32BaseAddr)));
                                        WriteMemory = pArmCapture->capPtrs[j].CaptureBufferMemory;
                                        WriteMemory.pAddr = pCaptureWrite;
                                    }
                                    else
                                    {
                                        ReadMemory = pArmCapture->capPtrs[j].OutputBufferMemory;
                                        ReadMemory.pAddr = (void *)((uint8_t *)ReadMemory.pAddr +
                                                (pArmCapture->capPtrs[j].shadowRead - pArmCapture->capPtrs[j].outputBufferPtr.ui32BaseAddr));
                                        WriteMemory = pArmCapture->capPtrs[j].CaptureBufferMemory;
                                        WriteMemory.pAddr = pCaptureWrite;
                                    }
                                    BDSP_MMA_P_FlushCache(ReadMemory, bytesToCopy);
                                    BKNI_Memcpy(WriteMemory.pAddr, ReadMemory.pAddr, bytesToCopy);
                                    BDSP_MMA_P_FlushCache(WriteMemory, bytesToCopy);

                                    /* Update the capture buffer write pointer and detect capture buffer overflow */
                                    pCaptureWrite += bytesToCopy;
                                    pCaptureBase = pArmCapture->capPtrs[j].captureBufferPtr.pBasePtr;
                                    pCaptureRead = pArmCapture->capPtrs[j].captureBufferPtr.pReadPtr;
                                    if (pCaptureWrite > pArmCapture->capPtrs[j].captureBufferPtr.pEndPtr)
                                    {
                                        BDBG_ERR(("!!! Error in capture logic: non-contiguous capture detected"));
                                    }

                                    if (pCaptureWrite == pArmCapture->capPtrs[j].captureBufferPtr.pEndPtr)
                                    {
                                        if (pCaptureBase == pCaptureRead)
                                        {
                                            BDBG_ERR(("!!! Capture buffer overflow for buffer [%d] for capture %p", j, (void *)pArmCapture));
                                            continue;
                                        }
                                        else
                                        {
                                            pCaptureWrite = pCaptureBase;
                                        }
                                    }

                                    pArmCapture->capPtrs[j].captureBufferPtr.pWritePtr = pCaptureWrite;

                                    /* Update the shadow read pointer and last write pointer */
                                    BDSP_Arm_P_GetUpdatedShadowReadAndLastWrite(&pArmCapture->capPtrs[j].outputBufferPtr,
                                                                          &pArmCapture->capPtrs[j].shadowRead,
                                                                          &pArmCapture->capPtrs[j].lastWrite,
                                                                          pArmCapture->eBuffType,
                                                                          bytesToCopy,
                                                                          pArmTask->pContext->pDevice->regHandle);

                                    /* Update the output buffer read pointer in required */
                                    if (pArmCapture->updateRead)
                                    {
                                        BDSP_Arm_P_UpdateReadPointer(&pArmCapture->capPtrs[j].outputBufferPtr,
                                                               pArmCapture->eBuffType,
                                                               pArmCapture->capPtrs[j].shadowRead,
                                                               pArmTask->pContext->pDevice->regHandle);
                                    }
                                } /* Looping through numBuffers */
                            }


						} /* if capture is enabled */
					} /* Looping through captureList*/
				} /* looping through stage */
				BDSP_ARM_STAGE_TRAVERSE_LOOP_END(pStageIterator)
			} /* if Task is running */
		} /* looping through tasklooping through context */
	} /* looping through context */
	BKNI_ReleaseMutex(pArmDevice->captureMutex);

    BDBG_LEAVE(BDSP_Arm_P_ProcessAudioCapture);
	return err;
}

BERR_Code BDSP_Arm_P_AudioCaptureGetBuffer(
	void *pCapture,   /* [in] capture handle */
	BDSP_BufferDescriptor *pBuffers     /* [out] pointer to buffer descriptor */
	)
{
	int j;
	uint32_t size, minSize, chunk1 = 0;
	uint8_t *base = 0, *read = 0, *end = 0, *write = 0;
	BDSP_ArmCapture *pArmCapture;
	BERR_Code err = BERR_SUCCESS;
	BDSP_MMA_Memory Memory;
	pArmCapture = (BDSP_ArmCapture *)pCapture;

    BDBG_ENTER(BDSP_Arm_P_AudioCaptureGetBuffer);
	BDBG_ASSERT(NULL != pArmCapture);

	pBuffers->interleaved = false;
	pBuffers->numBuffers = pArmCapture->numBuffers;

	if (pArmCapture->enabled)
	{
		minSize = 0xFFFFFFFF;
		for (j = 0; j < pArmCapture->numBuffers; j++)
		{
			base = pArmCapture->capPtrs[j].captureBufferPtr.pBasePtr;
			end = pArmCapture->capPtrs[j].captureBufferPtr.pEndPtr;
			read = pArmCapture->capPtrs[j].captureBufferPtr.pReadPtr;
			write = pArmCapture->capPtrs[j].captureBufferPtr.pWritePtr;

			size = write - read;
			chunk1 = size;
			if (read > write)
			{
				chunk1 = end - read;
				size = chunk1 + (write - base);
			}

			if (size < minSize)
			{
				minSize = size;
			}
			Memory = pArmCapture->capPtrs[j].CaptureBufferMemory;
			Memory.pAddr = (void *)read;
			pBuffers->buffers[j].buffer = Memory;
			pBuffers->buffers[j].wrapBuffer = pArmCapture->capPtrs[j].CaptureBufferMemory;
		}

		/* Use chunk1 size from the last buffer as this
		is expected to be the same for all buffers */
		if (minSize <= chunk1)
		{
			/* Atleast one buffer did not wrap around */
			for (j = 0; j < pArmCapture->numBuffers; j++)
			{
				pBuffers->buffers[j].wrapBuffer.pAddr = NULL;
				pBuffers->buffers[j].wrapBuffer.offset=0;
				pBuffers->buffers[j].wrapBuffer.hBlock= NULL;
			}
			pBuffers->bufferSize = minSize;
			pBuffers->wrapBufferSize = 0;
		}
		else
		{
			pBuffers->bufferSize = chunk1;
			pBuffers->wrapBufferSize = minSize - pBuffers->bufferSize;
		}
	}
	else
	{
		pBuffers->bufferSize = 0;
		pBuffers->wrapBufferSize = 0;
	}

    BDBG_LEAVE(BDSP_Arm_P_AudioCaptureGetBuffer);
	return err;
}

BERR_Code BDSP_Arm_P_AudioCaptureConsumeData(
	void *pCapture, /* [in] capture handle */
	uint32_t numBytes /* [in] sizes of data read from each intermediate buffer */
	)
{
	int j;
	uint8_t *pCaptureRead;
	BDSP_ArmCapture *pArmCapture;
	BERR_Code err = BERR_SUCCESS;

	pArmCapture = (BDSP_ArmCapture *)pCapture;

    BDBG_ENTER(BDSP_Arm_P_AudioCaptureConsumeData);
	BDBG_ASSERT(NULL != pArmCapture);

	if (pArmCapture->enabled)
	{
		for (j = 0; j < pArmCapture->numBuffers; j++)
		{
			pCaptureRead = pArmCapture->capPtrs[j].captureBufferPtr.pReadPtr;
			pCaptureRead += numBytes;
			if (pCaptureRead >= pArmCapture->capPtrs[j].captureBufferPtr.pEndPtr)
			{
				pCaptureRead = pArmCapture->capPtrs[j].captureBufferPtr.pBasePtr +
							   (pCaptureRead - pArmCapture->capPtrs[j].captureBufferPtr.pEndPtr);
			}
			pArmCapture->capPtrs[j].captureBufferPtr.pReadPtr = pCaptureRead;
		}
	}

    BDBG_LEAVE(BDSP_Arm_P_AudioCaptureConsumeData);
	return err;
}

uint32_t BDSP_Arm_P_GetAudioBufferDepthLinear(
	BDSP_AF_P_sDRAM_CIRCULAR_BUFFER *pBuffer,
	uint32_t ui32ShadowRead,
	dramaddr_t *pLastWrite,
	BDSP_AF_P_BufferType eType,
	BREG_Handle hReg)
{
	uint32_t depth;
	dramaddr_t ui32BaseAddr, ui32EndAddr, ui32ReadAddr, ui32WriteAddr, ui32WrapAddr;

    BDBG_ENTER(BDSP_Arm_P_GetAudioBufferDepthLinear);

	switch (eType)
	{
		case BDSP_AF_P_BufferType_eRAVE:
			ui32BaseAddr = BREG_Read32(hReg, pBuffer->ui32BaseAddr);
			ui32EndAddr = BREG_Read32(hReg, pBuffer->ui32EndAddr);
			ui32ReadAddr = ui32ShadowRead;
			ui32WriteAddr = BREG_Read32(hReg, pBuffer->ui32WriteAddr);
			ui32WrapAddr = BREG_Read32(hReg, pBuffer->ui32WrapAddr);

			if (ui32WrapAddr == 0)
				ui32WrapAddr = ui32EndAddr;

			depth = ui32WriteAddr - ui32ReadAddr;
			if (ui32WriteAddr < ui32ReadAddr)
			{
				/* Return only the contiguous chunk size */
				depth = (ui32WrapAddr - ui32ReadAddr) + 1;
			}
			break;

		case BDSP_AF_P_BufferType_eFMM:
			ui32BaseAddr = BREG_Read32(hReg, pBuffer->ui32BaseAddr);
			ui32EndAddr = BREG_Read32(hReg, pBuffer->ui32EndAddr);
			ui32WriteAddr = BREG_Read32(hReg, pBuffer->ui32WriteAddr);
			ui32ReadAddr = ui32ShadowRead;

			/* Buffer full condition */
			if ((ui32ReadAddr ^ ui32WriteAddr) == 0x80000000)
			{
				depth = (ui32EndAddr - ui32BaseAddr) + 1;
			}
			else
			{
				ui32ReadAddr &= 0x7FFFFFFF;
				ui32WriteAddr &= 0x7FFFFFFF;

				depth = ui32WriteAddr - ui32ReadAddr;
				if (ui32WriteAddr < ui32ReadAddr)
				{
					/* Return only the contiguous chunk size */
					depth = (ui32EndAddr - ui32ReadAddr) + 1;
				}
			}

			break;

		case BDSP_AF_P_BufferType_eRDB:
			ui32BaseAddr = BREG_Read32(hReg, pBuffer->ui32BaseAddr);
			ui32EndAddr = BREG_Read32(hReg, pBuffer->ui32EndAddr);
			ui32ReadAddr = ui32ShadowRead;
			ui32WriteAddr = BREG_Read32(hReg, pBuffer->ui32WriteAddr);

			depth = ui32WriteAddr - ui32ReadAddr;
			if (ui32WriteAddr < ui32ReadAddr)
			{
				/* Return only the contiguous chunk size */
				depth = (ui32EndAddr - ui32ReadAddr);
			}
			break;

		case BDSP_AF_P_BufferType_eDRAM:
		default:
			ui32BaseAddr = pBuffer->ui32BaseAddr;
			ui32EndAddr = pBuffer->ui32EndAddr;
			ui32ReadAddr = ui32ShadowRead;
			ui32WriteAddr = pBuffer->ui32WriteAddr;

			depth = ui32WriteAddr - ui32ReadAddr;
			if (ui32WriteAddr < ui32ReadAddr)
			{
				/* Return only the contiguous chunk size */
				depth = (ui32EndAddr - ui32ReadAddr);
			}
			break;
	}

	*pLastWrite = ui32WriteAddr;
    BDBG_LEAVE(BDSP_Arm_P_GetAudioBufferDepthLinear);
	return depth;
}

size_t BDSP_Arm_P_GetAudioCaptureBufferFreeSpaceLinear(BDSP_AF_P_CIRCULAR_BUFFER *pBuffer)
{
	size_t space;
	uint8_t *pEndPtr, *pReadPtr, *pWritePtr;

    BDBG_ENTER(BDSP_Arm_P_GetAudioCaptureBufferFreeSpaceLinear);

	pEndPtr = pBuffer->pEndPtr;
	pReadPtr = pBuffer->pReadPtr;
	pWritePtr = pBuffer->pWritePtr;

	space = pReadPtr - pWritePtr;
	if (pWritePtr >= pReadPtr)
	{
		space = pEndPtr - pWritePtr;
	}

    BDBG_LEAVE(BDSP_Arm_P_GetAudioCaptureBufferFreeSpaceLinear);
	return space;
}

bool BDSP_Arm_P_DetectAudioCaptureError(
	BDSP_AF_P_sDRAM_CIRCULAR_BUFFER *pBuffer,
	uint32_t ui32ShadowRead,
	uint32_t ui32LastWrite,
	BDSP_AF_P_BufferType eType,
	BREG_Handle hReg)
{
	bool retval = false;
	dramaddr_t ui32WriteAddr;

    BDBG_ENTER(BDSP_Arm_P_DetectAudioCaptureError);

	switch (eType)
	{
		case BDSP_AF_P_BufferType_eRAVE:
			ui32WriteAddr = BREG_Read32(hReg, pBuffer->ui32WriteAddr);
			break;
		case BDSP_AF_P_BufferType_eFMM:
			ui32WriteAddr = BREG_Read32(hReg, pBuffer->ui32WriteAddr);
			/* Clear the wrap bit */
			ui32WriteAddr &= 0x7FFFFFFF;
			ui32ShadowRead &= 0x7FFFFFFF;
			break;
		case BDSP_AF_P_BufferType_eRDB:
			ui32WriteAddr = BREG_Read32(hReg, pBuffer->ui32WriteAddr);
			break;
		case BDSP_AF_P_BufferType_eDRAM:
		default:
			ui32WriteAddr = pBuffer->ui32WriteAddr;
			break;
	}

	/* It is not possible to detect a capture error if shadow read and last write are the same */
	if (ui32ShadowRead == ui32LastWrite)
	{
		retval = false;
	}
	else
	{
		if (ui32ShadowRead > ui32LastWrite)
		{
			if ((ui32WriteAddr > ui32ShadowRead)
				|| (ui32WriteAddr < ui32LastWrite))
			{
				retval = true;
			}
		}
		else
		{
			if ((ui32WriteAddr > ui32ShadowRead)
				&& (ui32WriteAddr < ui32LastWrite))
			{
				retval = true;
			}
		}
	}

	if (retval)
	{
		BDBG_ERR(("shadow = %x : write = " BDSP_MSG_FMT ": last wr = %x", ui32ShadowRead, BDSP_MSG_ARG(ui32WriteAddr), ui32LastWrite));
	}

    BDBG_LEAVE(BDSP_Arm_P_DetectAudioCaptureError);
	return retval;
}


void BDSP_Arm_P_GetUpdatedShadowReadAndLastWrite(
	BDSP_AF_P_sDRAM_CIRCULAR_BUFFER *pBuffer,
	dramaddr_t *pShadowRead,
	dramaddr_t *pLastWrite,
	BDSP_AF_P_BufferType eType,
	uint32_t bytesRead,
	BREG_Handle hReg)
{
	dramaddr_t shadowReadAddr;
	dramaddr_t baseAddr, endAddr;

    BDBG_ENTER(BDSP_Arm_P_GetUpdatedShadowReadAndLastWrite);

	shadowReadAddr = *pShadowRead + bytesRead;

	switch (eType)
	{
		case BDSP_AF_P_BufferType_eRAVE:
			*pLastWrite = BREG_Read32(hReg, pBuffer->ui32WriteAddr);
			baseAddr = BREG_Read32(hReg, pBuffer->ui32BaseAddr);
			endAddr = BREG_Read32(hReg, pBuffer->ui32WrapAddr);
			if (endAddr == 0)
			{
				endAddr = BREG_Read32(hReg, pBuffer->ui32EndAddr);
			}

			/* TBD: Need to verify if this is correct. Code borrowed from raaga test */
			if (shadowReadAddr >= endAddr)
			{
				shadowReadAddr = baseAddr + (shadowReadAddr - endAddr);
			}
			break;
		case BDSP_AF_P_BufferType_eFMM:
#if 0
			*pLastWrite = BREG_Read32(hReg, pBuffer->ui32WriteAddr);
			/* Clear the wrap bit */
			*pLastWrite &= 0x7FFFFFFF;
#endif
			baseAddr = BREG_Read32(hReg, pBuffer->ui32BaseAddr);
			endAddr = BREG_Read32(hReg, pBuffer->ui32EndAddr);

			if ((shadowReadAddr & 0x7FFFFFFF) > endAddr)
			{
				shadowReadAddr = baseAddr + (shadowReadAddr - endAddr) - 1;
				/* Flip bit 31 on a wrap */
				shadowReadAddr ^= 0x80000000;
			}
			break;
		case BDSP_AF_P_BufferType_eRDB:
			*pLastWrite = BREG_Read32(hReg, pBuffer->ui32WriteAddr);
			baseAddr = BREG_Read32(hReg, pBuffer->ui32BaseAddr);
			endAddr = BREG_Read32(hReg, pBuffer->ui32EndAddr);

			if (shadowReadAddr >= endAddr)
			{
				shadowReadAddr = baseAddr + (shadowReadAddr - endAddr);
			}
			break;
		case BDSP_AF_P_BufferType_eDRAM:
		default:
			*pLastWrite = pBuffer->ui32WriteAddr;
			baseAddr = pBuffer->ui32BaseAddr;
			endAddr = pBuffer->ui32EndAddr;
			if (shadowReadAddr >= endAddr)
			{
				shadowReadAddr = baseAddr + (shadowReadAddr - endAddr);
			}
			break;
	}

	*pShadowRead = shadowReadAddr;

    BDBG_LEAVE(BDSP_Arm_P_GetUpdatedShadowReadAndLastWrite);
}

void BDSP_Arm_P_UpdateReadPointer(
	BDSP_AF_P_sDRAM_CIRCULAR_BUFFER *pBuffer,
	BDSP_AF_P_BufferType eType,
	dramaddr_t ui32ReadAddr,
	BREG_Handle hReg)
{
    BDBG_ENTER(BDSP_Arm_P_UpdateReadPointer);

	switch (eType)
	{
		case BDSP_AF_P_BufferType_eRAVE:
		case BDSP_AF_P_BufferType_eFMM:
		case BDSP_AF_P_BufferType_eRDB:
			BREG_Write32(hReg, pBuffer->ui32ReadAddr, ui32ReadAddr);
			break;
		case BDSP_AF_P_BufferType_eDRAM:
		default:
			pBuffer->ui32ReadAddr = ui32ReadAddr;
			break;
	}

    BDBG_LEAVE(BDSP_Arm_P_UpdateReadPointer);
}

BERR_Code BDSP_Arm_P_InitAudioCaptureInfo(BDSP_ArmCapture *pArmCapture, const BDSP_AudioCaptureCreateSettings *pSettings)
{
	unsigned j;
	uint32_t memRequired;
	BERR_Code err = BERR_SUCCESS;
	BDSP_MMA_Memory CaptureMemory;
	uint8_t *ptr, *endptr  = NULL;
	BDSP_AF_P_CIRCULAR_BUFFER *pBuffer;

	BDBG_ENTER(BDSP_Arm_P_InitAudioCaptureInfo);

	BDBG_ASSERT(NULL != pArmCapture);

	pArmCapture->enabled = false;
	pArmCapture->maxBuffers = pSettings->maxChannels;

	/* Allocate memory for the worst case output */
	memRequired = pSettings->maxChannels*pSettings->channelBufferSize;

	/* Allocate the intermediate capture buffers */
	err = BDSP_MMA_P_AllocateAlignedMemory(pArmCapture->hHeap,
					(memRequired + 32),
					&pArmCapture->captureBuffer,
					BDSP_MMA_Alignment_32bit);
	if(err != BERR_SUCCESS)
	{
		BDBG_ERR(("Unable to Allocate memory for Capture buffers !"));
		err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
		return err;
	}
	CaptureMemory = pArmCapture->captureBuffer;
	/* Split the allocated space into individual capture buffers */
	for (j = 0; j < pSettings->maxChannels; j++)
	{
		pBuffer = &pArmCapture->capPtrs[j].captureBufferPtr;
		pArmCapture->capPtrs[j].CaptureBufferMemory = CaptureMemory;
		ptr = (uint8_t *)CaptureMemory.pAddr;
		endptr = ptr + pSettings->channelBufferSize;
		pBuffer->pBasePtr  = ptr;
		pBuffer->pReadPtr  = ptr;
		pBuffer->pWritePtr = ptr;
		pBuffer->pEndPtr   = endptr;
		pBuffer->pWrapPtr  = endptr;
		CaptureMemory.pAddr  = (void *)((uint8_t *)CaptureMemory.pAddr + pSettings->channelBufferSize);
		CaptureMemory.offset = CaptureMemory.offset + pSettings->channelBufferSize;
	}

	/* Reset the pointers for the rest of the buffers */
	for ( ; j < BDSP_AF_P_MAX_CHANNELS; j++)
	{
		pBuffer = &pArmCapture->capPtrs[j].captureBufferPtr;
		pBuffer->pBasePtr = 0;
		pBuffer->pReadPtr = 0;
		pBuffer->pWritePtr = 0;
		pBuffer->pEndPtr = 0;
		pBuffer->pWrapPtr = 0;
	}

	BDBG_LEAVE(BDSP_Arm_P_InitAudioCaptureInfo);
	return err;
}

BERR_Code BDSP_Arm_P_GetAudioOutputPointers(BDSP_StageSrcDstDetails *pDstDetails, BDSP_ArmCapture *pArmCapture)
{
	int i, numBuffers;
	BERR_Code err = BERR_SUCCESS;
	BDSP_AF_P_sDRAM_CIRCULAR_BUFFER *psCircBuffer;
	BDSP_P_InterTaskBuffer *pArmInterTaskBuffer;

	if (pDstDetails->eNodeValid != BDSP_AF_P_eValid)
	{
		BDBG_ERR(("Cannot add capture for an invalid output of the stage"));
		return BERR_TRACE(BERR_NOT_SUPPORTED);
	}

	BDBG_ENTER(BDSP_Arm_P_GetAudioOutputPointers);

	switch (pDstDetails->eConnectionType)
	{
		case BDSP_ConnectionType_eFmmBuffer:
			numBuffers = pDstDetails->connectionDetails.fmm.descriptor.numBuffers;
			for(i = 0; i < numBuffers; i++)
			{
				psCircBuffer = &pArmCapture->capPtrs[i].outputBufferPtr;
				psCircBuffer->ui32ReadAddr  = pDstDetails->connectionDetails.fmm.descriptor.buffers[i].read;
				psCircBuffer->ui32WriteAddr = pDstDetails->connectionDetails.fmm.descriptor.buffers[i].write;
				psCircBuffer->ui32BaseAddr  = pDstDetails->connectionDetails.fmm.descriptor.buffers[i].base;
				psCircBuffer->ui32EndAddr   = pDstDetails->connectionDetails.fmm.descriptor.buffers[i].end;
				psCircBuffer->ui32WrapAddr = pDstDetails->connectionDetails.fmm.descriptor.buffers[i].end;

				pArmCapture->capPtrs[i].ui32StartWriteAddr = psCircBuffer->ui32EndAddr + 8;
			}

			pArmCapture->StartCapture =  false;/** Will only set to true once write pointer is >= start write pointer  **/
			pArmCapture->eBuffType = BDSP_AF_P_BufferType_eFMM;
			break;

		case BDSP_ConnectionType_eInterTaskBuffer:
			pArmInterTaskBuffer = (BDSP_P_InterTaskBuffer *)pDstDetails->connectionDetails.interTask.hInterTask->pInterTaskBufferHandle;

			numBuffers = pArmInterTaskBuffer->numChans;
			for(i = 0; i < numBuffers; i++)
			{
				psCircBuffer = &pArmCapture->capPtrs[i].outputBufferPtr;
				*psCircBuffer = ((BDSP_AF_P_sIO_BUFFER *)(pArmInterTaskBuffer->IoBufferDesc.pAddr))->sCircBuffer[i];
			}

			pArmCapture->eBuffType = BDSP_AF_P_BufferType_eDRAM;
			break;

		case BDSP_ConnectionType_eRaveBuffer:
			numBuffers = pDstDetails->IoBuffer.ui32NumBuffers;

			for (i = 0; i < numBuffers; i++)
			{
				psCircBuffer = &pArmCapture->capPtrs[i].outputBufferPtr;
				/* The rave pointers are programmed with the chip physical offset added. Masking the same */
				psCircBuffer->ui32BaseAddr = pDstDetails->IoBuffer.sCircBuffer[i].ui32BaseAddr & 0x0FFFFFFF;
				psCircBuffer->ui32ReadAddr = pDstDetails->IoBuffer.sCircBuffer[i].ui32ReadAddr & 0x0FFFFFFF;
				psCircBuffer->ui32WriteAddr = pDstDetails->IoBuffer.sCircBuffer[i].ui32WriteAddr & 0x0FFFFFFF;
				psCircBuffer->ui32EndAddr = pDstDetails->IoBuffer.sCircBuffer[i].ui32EndAddr & 0x0FFFFFFF;
				psCircBuffer->ui32WrapAddr = pDstDetails->IoBuffer.sCircBuffer[i].ui32WrapAddr & 0x0FFFFFFF;
			}

			pArmCapture->eBuffType = BDSP_AF_P_BufferType_eRAVE;
			break;
		default:
			BDBG_ERR(("Output Capture not supported for buffer type (%d)", pDstDetails->eConnectionType));
			return BERR_TRACE(BERR_INVALID_PARAMETER);
			break;
	}

	pArmCapture->numBuffers = numBuffers;

	BDBG_LEAVE(BDSP_Arm_P_GetAudioOutputPointers);

	return err;
}
/***********************************************************************
Name        :   BDSP_Arm_P_GetMemoryEstimate

Type        :   PI Interface

Input       :   pSettings       -   Handle of the Stage whose settings needs to be retrieved.
				pUsage      -   Pointer to usage case scenario from which we determine the runtime memory.
				boxHandle   -     BOX Mode Handle for which the memory needs to be estimated.
				pEstimate   -   Pointer to the memory where the Stage Settings are filled.

Return      :   Error Code to return SUCCESS or FAILURE

Functionality   :   Following are the operations performed.
		1)  Calculate the Init memory required for the system.
		2)  Calculate Scratch, InterstageIO and InterstageIOGen for the system.
		3)  Calculate the Firware memory requied by the system.
		4)  Return the Firmware memory and the Heap memory required by the system.
***********************************************************************/
BERR_Code BDSP_Arm_P_GetMemoryEstimate(
	const BDSP_ArmSettings  *pSettings,
	const BDSP_UsageOptions *pUsage,
	BBOX_Handle              boxHandle,
	BDSP_MemoryEstimate     *pEstimate /*[out]*/
)
{
	BERR_Code ret= BERR_SUCCESS;
	BDSP_Arm_P_DwnldMemInfo   *psDwnldMemInfo;
	BDSP_ArmImgCacheEntry     *pImgCache;
	unsigned AudioEncoderMemory = 0;
	unsigned TaskMemory = 0;

	/* This allocation is done to reuse the functionality of BDSP_MM_P_GetFwMemRequired and nothing else*/
	psDwnldMemInfo = BKNI_Malloc(sizeof(BDSP_Arm_P_DwnldMemInfo));
	BKNI_Memset(psDwnldMemInfo, 0 , sizeof(BDSP_Arm_P_DwnldMemInfo));

	pImgCache = BKNI_Malloc(sizeof(BDSP_ArmImgCacheEntry)*BDSP_ARM_IMG_ID_MAX);
	BKNI_Memset(pImgCache, 0 , sizeof(BDSP_ArmImgCacheEntry)*BDSP_ARM_IMG_ID_MAX);

	/* Initialise the values */
	pEstimate->FirmwareMemory = 0;
	pEstimate->GeneralMemory  = 0;
	BDSP_Arm_P_CalculateInitMemory(&pEstimate->GeneralMemory);

	/*Calculate the FIRMWARE heap memory required by the system */
	ret = BDSP_Arm_P_GetFwMemRequired(pSettings, psDwnldMemInfo,(void *) pImgCache, false, pUsage);
	pEstimate->FirmwareMemory = psDwnldMemInfo->ui32AllocwithGuardBand;

	/* Memory Allocated for Task per task is calculated.
		 Worst Case Estimate is taken for 4 tasks */
	BDSP_Arm_P_CalculateTaskMemory(&TaskMemory);
	pEstimate->GeneralMemory += (TaskMemory * BDSP_ARM_MAX_FW_TASK_PER_DSP);

#if 0
	/* Memory Allocation associated with Decoder */
	if(pUsage->NumAudioDecoders)
	{
		ret = BDSP_Arm_P_CalculateStageMemory(BDSP_AlgorithmType_eAudioDecode, &DecodeMemory, pUsage);
		pEstimate->GeneralMemory += (DecodeMemory*pUsage->NumAudioDecoders);
	}

	/* Memory Allocation associated with Post Processing */
	if(pUsage->NumAudioPostProcesses)
	{
		ret = BDSP_Arm_P_CalculateStageMemory(BDSP_AlgorithmType_eAudioProcessing, &PostProcessingMemory, pUsage);
		pEstimate->GeneralMemory += (PostProcessingMemory*pUsage->NumAudioPostProcesses);
	}

	/* Memory Allocation associated with Passthru */
	if(pUsage->NumAudioPassthru)
	{
		ret = BDSP_Arm_P_CalculateStageMemory(BDSP_AlgorithmType_eAudioPassthrough, &PassthruMemory, pUsage);
		pEstimate->GeneralMemory += (PassthruMemory*pUsage->NumAudioPassthru);
	}
#endif

	/* Memory Allocation associated with Audio Encoder */
	if(pUsage->NumAudioEncoders)
	{
		BDSP_Arm_P_CalculateStageMemory(BDSP_AlgorithmType_eAudioEncode, &AudioEncoderMemory, pUsage);
		pEstimate->GeneralMemory += (AudioEncoderMemory*pUsage->NumAudioEncoders);
	}

#if 0
	/* Memory Allocation associated with Mixer
		 We estimate that we can connect atmost 3 Intertask buffers per Mixer*/
	if(pUsage->NumAudioMixers)
	{
		ret = BDSP_Arm_P_CalculateStageMemory(BDSP_AlgorithmType_eAudioMixer, &MixerMemory, pUsage);
		pEstimate->GeneralMemory += (MixerMemory*pUsage->NumAudioMixers);
		pEstimate->GeneralMemory += (IntertaskBufferMemory*pUsage->NumAudioMixers*BDSP_MAX_INTERTASKBUFFER_INPUT_TO_MIXER);
	}

	/* Memory Allocation associated with Echocanceller
		 We estimate that we can connect atmost 1 Intertask buffers per Echocanceller*/
	if(pUsage->NumAudioEchocancellers)
	{
		ret = BDSP_Arm_P_CalculateStageMemory(BDSP_AlgorithmType_eAudioEchoCanceller, &EchocancellerMemory, pUsage);
		pEstimate->GeneralMemory += (EchocancellerMemory*pUsage->NumAudioEchocancellers);
		pEstimate->GeneralMemory += (IntertaskBufferMemory*pUsage->NumAudioEchocancellers*BDSP_MAX_INTERTASKBUFFER_INPUT_TO_ECHOCANCELLER);
	}

	/* Memory Allocation associated with Video Decoder */
	if(pUsage->NumVideoDecoders)
	{
		ret = BDSP_Arm_P_CalculateStageMemory(BDSP_AlgorithmType_eVideoDecode, &VideoDecodeMemory, pUsage);
		pEstimate->GeneralMemory += (VideoDecodeMemory*pUsage->NumVideoDecoders);
	}

	/* Memory Allocation associated with Video Encoder */
	if(pUsage->NumVideoEncoders)
	{
		ret = BDSP_Arm_P_CalculateStageMemory(BDSP_AlgorithmType_eVideoEncode, &VideoEncodeMemory, pUsage);
		pEstimate->GeneralMemory += (VideoEncodeMemory*pUsage->NumVideoEncoders);
	}
#endif

	BDBG_MSG(("Memory Required FIRMWARE = %d      GENERAL = %d",pEstimate->FirmwareMemory, pEstimate->GeneralMemory));
	BKNI_Free(psDwnldMemInfo);
	BKNI_Free(pImgCache);
	BSTD_UNUSED(boxHandle);
	return ret;
}
