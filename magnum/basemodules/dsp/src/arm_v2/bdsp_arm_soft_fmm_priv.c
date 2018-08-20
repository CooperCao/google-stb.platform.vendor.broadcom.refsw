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
#include "bdsp_arm_soft_fmm_priv.h"
#include "bchp_common.h"
#ifdef BCHP_AUD2711RATE_REG_START
#include "bchp_aud2711rate.h"
#endif


BDBG_MODULE(bdsp_arm_soft_fmm_priv);
BDBG_OBJECT_ID(BDSP_ArmSoftFMM_Output);
BDBG_OBJECT_ID(BDSP_ArmSoftFMM_Input);
BDBG_OBJECT_ID(BDSP_ArmSoftFMM_Mixer);
BDBG_OBJECT_ID(BDSP_ArmSoftFMM);


BERR_Code BDSP_Arm_P_SoftFMMOpen(
    void                             *pDeviceHandle,
    BDSP_SoftFMMHandle               *pSoftFMM
)
{
    BERR_Code errCode = BERR_SUCCESS;
    BDSP_Arm *pDevice = (BDSP_Arm *)pDeviceHandle;
    BDSP_ArmSoftFMM *pArmSoftFMM;
    unsigned MemoryRequired=0;

    BDBG_ENTER(BDSP_Arm_P_SoftFMMOpen);
    BDBG_OBJECT_ASSERT(pDevice, BDSP_Arm);
    *pSoftFMM = NULL;
    pArmSoftFMM = (BDSP_ArmSoftFMM *)BKNI_Malloc(sizeof(BDSP_ArmSoftFMM));
    if(NULL == pArmSoftFMM)
    {
        BDBG_ERR(("pArmSoftFMM: Unable to allocate Memory for Opening the SoftFMM"));
        errCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto end;
    }
    BKNI_Memset(pArmSoftFMM, 0, sizeof(*pArmSoftFMM));
    BDBG_OBJECT_SET(pArmSoftFMM, BDSP_ArmSoftFMM);
    pArmSoftFMM->pDevice  = pDevice;

    pDevice->softFMMDetails[0].pSoftFMM[0] = pArmSoftFMM;
    /*pArmSoftFMM->settings = *pSettings;*/

    BDSP_P_InitSoftFMM(&pArmSoftFMM->softFMM, pArmSoftFMM);

    pArmSoftFMM->softFMM.close = BDSP_Arm_P_SoftFMM_Close;
    pArmSoftFMM->softFMM.getDefaultMixerSettings = BDSP_Arm_P_SoftFMM_GetDefaultMixerSettings;
    pArmSoftFMM->softFMM.createMixer  = BDSP_Arm_P_SoftFMM_MixerCreate;
    pArmSoftFMM->softFMM.getDefaultOutputSettings = BDSP_Arm_P_SoftFMM_GetDefaultOutputSettings;
    pArmSoftFMM->softFMM.createOutput = BDSP_Arm_P_SoftFMMOutputCreate;
    pArmSoftFMM->softFMM.getDefaultInputSettings = BDSP_Arm_P_SoftFMM_GetDefaultInputSettings;
    pArmSoftFMM->softFMM.createInput = BDSP_Arm_P_SoftFMMInputCreate;

    errCode = BDSP_Arm_P_InitSoftFMM(pArmSoftFMM);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Arm_P_SoftFMMOpen: Unable to Initialise Parameters for Soft FMM"));
        errCode = BERR_TRACE(errCode);
        goto end;
    }

    BDSP_Arm_P_CalculateSoftFMMProcessMemory(&MemoryRequired);
    errCode = BDSP_MMA_P_AllocateAlignedMemory(pDevice->memHandle,
                            MemoryRequired,
                            &(pArmSoftFMM->processMemInfo.sMemoryPool.Memory),
                            BDSP_MMA_Alignment_4KByte);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Arm_P_SoftFMMOpen: Unable to Allocate Memory for Soft FMM Process %p",(void *)pArmSoftFMM));
        errCode = BERR_TRACE(errCode);
        goto end;
    }
    pArmSoftFMM->processMemInfo.sMemoryPool.ui32Size     = MemoryRequired;
    pArmSoftFMM->processMemInfo.sMemoryPool.ui32UsedSize = 0;

    errCode = BDSP_Arm_P_AssignSoftFMMProcessMemory((void *)pArmSoftFMM);
    if (errCode != BERR_SUCCESS )
    {
        BDBG_ERR(("BDSP_Arm_P_SoftFMMOpen: Unable to assign process memory for Soft FMM Process %p",(void *)pArmSoftFMM));
        errCode = BERR_TRACE(errCode);
        goto end;
    }

    errCode = BDSP_P_CreateMsgQueue(
            &pArmSoftFMM->processMemInfo.syncQueueParams,
            pDevice->regHandle,
            0,
            &pArmSoftFMM->hSyncQueue);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Arm_P_SoftFMMOpen: Unable to Create Sync Resp Queue for Soft FMM Process %p",(void *)pArmSoftFMM));
        errCode = BERR_TRACE(errCode);
        goto end;
    }

    errCode = BDSP_P_CreateMsgQueue(
            &pArmSoftFMM->processMemInfo.asyncQueueParams,
            pDevice->regHandle,
            0,
            &pArmSoftFMM->hAsyncQueue);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Arm_P_SoftFMMOpen: Unable to Create ASync Resp Queue for Soft FMM Process %p",(void *)pArmSoftFMM));
        errCode = BERR_TRACE(errCode);
        goto end;
    }

    errCode = BDSP_Arm_P_StartSoftFMMProcess(pArmSoftFMM);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Arm_P_SoftFMMOpen: Failed to start Soft FMM Process %p",(void *)pArmSoftFMM));
        errCode = BERR_TRACE(errCode);
        goto end;
    }

#if 0
    BKNI_AcquireMutex(pDevice->deviceMutex);
    BLST_S_INIT(&pArmSoftFMM->InputList);
    BLST_S_INIT(&pArmSoftFMM->OutputList);
    BLST_S_INIT(&pArmSoftFMM->MixerList);
    BKNI_ReleaseMutex(pDevice->deviceMutex);
#endif

    *pSoftFMM = &pArmSoftFMM->softFMM;

    BDBG_ERR(("Soft FMM Open Successful"));
    goto end;

end:
    BDBG_LEAVE( BDSP_Arm_P_SoftFMMOpen);
    return errCode;
}

BERR_Code BDSP_Arm_P_SoftFMM_Close(
    void *pSoftFMMHandle
)
{
    BERR_Code errCode = BERR_SUCCESS;
    BDSP_ArmSoftFMM *pArmSoftFMM = (BDSP_ArmSoftFMM *)pSoftFMMHandle;
    BDSP_ArmSoftFMM_Output *pArmSoftFMMOutput;
    BDSP_ArmSoftFMM_Input *pArmSoftFMMInput;
    BDSP_ArmSoftFMM_Mixer *pArmSoftFMMMixer;

    BDBG_ENTER(BDSP_Arm_P_SoftFMM_Close);
    BDBG_OBJECT_ASSERT(pArmSoftFMM, BDSP_ArmSoftFMM);

    while((pArmSoftFMMOutput = BLST_S_FIRST(&pArmSoftFMM->OutputList)))
    {
        BDSP_Arm_P_SoftFMMOutputDestroy((void *)pArmSoftFMMOutput);
    }

    while((pArmSoftFMMInput = BLST_S_FIRST(&pArmSoftFMM->InputList)))
    {
        BDSP_Arm_P_SoftFMMInputDestroy((void *)pArmSoftFMMInput);
    }

    while((pArmSoftFMMMixer = BLST_S_FIRST(&pArmSoftFMM->MixerList)))
    {
        BDSP_Arm_P_SoftFMMMixerDestroy((void *)pArmSoftFMMMixer);
    }

    BDSP_P_DestroyMsgQueue(pArmSoftFMM->hSyncQueue);

    BDSP_P_DestroyMsgQueue(pArmSoftFMM->hAsyncQueue);

    BDSP_MMA_P_FreeMemory(&(pArmSoftFMM->processMemInfo.sMemoryPool.Memory));

    BKNI_DestroyEvent(pArmSoftFMM->hEvent);

    BKNI_Free(pArmSoftFMM->processMemInfo.hostAsyncQueue.pAddr);

    /* Invalidate and free the SoftFMM structure */
    BDBG_OBJECT_DESTROY(pArmSoftFMM, BDSP_ArmSoftFMM);
    BKNI_Free(pArmSoftFMM);
    goto end;

end:
    BDBG_LEAVE(BDSP_Arm_P_SoftFMM_Close);
    return errCode;
}


/***************************************************************************
Summary:
Get Default SoftFMM Mixer Settings
***************************************************************************/
BERR_Code BDSP_Arm_P_SoftFMM_GetDefaultMixerSettings(
    void *pSoftFMMHandle,
    BDSP_SoftFMM_MixerSettings *pSettings     /* [out] */
)
{
    BERR_Code errCode = BERR_SUCCESS;
    BDSP_ArmSoftFMM *pArmSoftFMM = (BDSP_ArmSoftFMM *)pSoftFMMHandle;
    uint32_t ui32ChannelIndex;

    BDBG_ENTER(BDSP_Arm_P_SoftFMM_GetDefaultMixerSettings);

    BDBG_OBJECT_ASSERT(pArmSoftFMM, BDSP_ArmSoftFMM);

    /*pSettings->eDataType = BDSP_DataType_ePcmStereo;*/
    pSettings->ui32SampleRate = 48000;
    pSettings->sVolumeControlConfigParams.ui32Enable = 0;
    pSettings->sVolumeControlConfigParams.ui32MuteEnable = 0;
    for(ui32ChannelIndex = 0; ui32ChannelIndex < BDSP_AF_P_MAX_CHANNELS; ui32ChannelIndex++)
    {
        pSettings->sVolumeControlConfigParams.ui32RampEnable[ui32ChannelIndex] = 0;
        pSettings->sVolumeControlConfigParams.ui32RampStep[ui32ChannelIndex] = 0;
        pSettings->sVolumeControlConfigParams.ui32DesiredVolume[ui32ChannelIndex] = 0;
    }

    goto end;

end:
    BDBG_LEAVE(BDSP_Arm_P_SoftFMM_GetDefaultMixerSettings);
    return errCode;
}


/***************************************************************************
Summary:
Create a DSP SoftFMM Mixer
***************************************************************************/
BERR_Code BDSP_Arm_P_SoftFMM_MixerCreate(
    void *pSoftFMMHandle,
    const BDSP_SoftFMM_MixerSettings *pSettings,
    BDSP_SoftFMMMixerHandle *pSoftFMMMixer    /* [out] */
)
{
    BERR_Code errCode = BERR_SUCCESS;
    BDSP_ArmSoftFMM *pArmSoftFMM = (BDSP_ArmSoftFMM *)pSoftFMMHandle;
    BDSP_Arm *pDevice;
    BDSP_ArmSoftFMM_Mixer *pArmSoftFMMMixer;
	/*const BDSP_P_AlgorithmInfo        *pAlgoInfo;*/
	const BDSP_P_AlgorithmCodeInfo    *pAlgoCodeInfo;
    unsigned MemoryRequired = 0;
    uint32_t ui32Size =0;
    BDSP_MMA_Memory Memory;
    BDSP_P_SoftFMMCreateMixerCommand sSoftFMMCreateMixerCommand;

    BDBG_ENTER(BDSP_Arm_P_SoftFMM_MixerCreate);

    BDBG_OBJECT_ASSERT(pArmSoftFMM, BDSP_ArmSoftFMM);
    pDevice = (BDSP_Arm *)pArmSoftFMM->pDevice;
    BDBG_OBJECT_ASSERT(pDevice,BDSP_Arm);

    *pSoftFMMMixer = NULL;
    pArmSoftFMMMixer = (BDSP_ArmSoftFMM_Mixer *)BKNI_Malloc(sizeof(BDSP_ArmSoftFMM_Mixer));
    if(NULL == pArmSoftFMMMixer)
    {
        BDBG_ERR(("pArmSoftFMMMixer: Unable to allocate Memory for Opening the SoftFMM Mixer"));
        errCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto end;
    }
    BKNI_Memset(pArmSoftFMMMixer, 0, sizeof(*pArmSoftFMMMixer));
    BDBG_OBJECT_SET(pArmSoftFMMMixer, BDSP_ArmSoftFMM_Mixer);
    pArmSoftFMMMixer->pArmSoftFMM = pSoftFMMHandle;
    pArmSoftFMMMixer->settings = *pSettings;
    pArmSoftFMMMixer->started = false;
    pArmSoftFMMMixer->ui32MixerIndex = pArmSoftFMM->numMixers;

    BDSP_P_InitSoftFMMMixer(&pArmSoftFMMMixer->softFMMMixer, pArmSoftFMMMixer);

    pArmSoftFMMMixer->softFMMMixer.destroy          = BDSP_Arm_P_SoftFMMMixerDestroy;
    pArmSoftFMMMixer->softFMMMixer.getSettings      = BDSP_Arm_P_SoftFMMMixerGetSettings;
    pArmSoftFMMMixer->softFMMMixer.setSettings      = BDSP_Arm_P_SoftFMMMixerSetSettings;
    pArmSoftFMMMixer->softFMMMixer.setSettings_isr  = BDSP_Arm_P_SoftFMMMixerSetSettings_isr;
    pArmSoftFMMMixer->softFMMMixer.getStatus        = BDSP_Arm_P_SoftFMMMixerGetStatus;
    pArmSoftFMMMixer->softFMMMixer.addInput         = BDSP_Arm_P_SoftFMMMixerAddInput;
    pArmSoftFMMMixer->softFMMMixer.removeInput      = BDSP_Arm_P_SoftFMMMixerRemoveInput;
    pArmSoftFMMMixer->softFMMMixer.removeAllInputs  = BDSP_Arm_P_SoftFMMMixerRemoveAllInputs;
    pArmSoftFMMMixer->softFMMMixer.addOutput        = BDSP_Arm_P_SoftFMMMixerAddOutput;
    pArmSoftFMMMixer->softFMMMixer.removeOutput     = BDSP_Arm_P_SoftFMMMixerRemoveOutput;
    pArmSoftFMMMixer->softFMMMixer.removeAllOutputs = BDSP_Arm_P_SoftFMMMixerRemoveAllOutputs;
    pArmSoftFMMMixer->softFMMMixer.start            = BDSP_Arm_P_SoftFMMMixerStart;
    pArmSoftFMMMixer->softFMMMixer.stop             = BDSP_Arm_P_SoftFMMMixerStop;


    /*pAlgoInfo = BDSP_P_LookupAlgorithmInfo(BDSP_Algorithm_eSoftFMM);*/
    pAlgoCodeInfo = BDSP_Arm_P_LookupAlgorithmCodeInfo(BDSP_Algorithm_eSoftFMM);

    MemoryRequired += pAlgoCodeInfo->interFrameSize;
    MemoryRequired += sizeof(BDSP_SoftFmmConfigParams);
    MemoryRequired += sizeof(BDSP_SoftFMMStatusInfo);
    MemoryRequired = BDSP_ALIGN_SIZE(MemoryRequired, 4096);

	errCode = BDSP_MMA_P_AllocateAlignedMemory(pDevice->memHandle,
							MemoryRequired,
							&(pArmSoftFMMMixer->sMemInfo.sMemoryPool.Memory),
							BDSP_MMA_Alignment_4KByte);
    pArmSoftFMMMixer->sMemInfo.sMemoryPool.ui32Size 	 = MemoryRequired;

    sSoftFMMCreateMixerCommand.MixerIndex = pArmSoftFMMMixer->ui32MixerIndex;
    sSoftFMMCreateMixerCommand.sSoftFMMMixerSettings = pArmSoftFMMMixer->settings;

    ui32Size = pAlgoCodeInfo->interFrameSize;
    errCode = BDSP_P_RequestMemory(&pArmSoftFMMMixer->sMemInfo.sMemoryPool, ui32Size, &Memory);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Arm_P_SoftFMMMixerStart: Unable to assign Interframe memory for Soft FMM Stage"));
        goto end;
    }
    pArmSoftFMMMixer->sMemInfo.sInterframe.Buffer  = Memory;
    pArmSoftFMMMixer->sMemInfo.sInterframe.ui32Size= ui32Size;

    errCode = BDSP_P_InterframeRunLengthDecode(
			Memory.pAddr, /*Destination*/
			pDevice->codeInfo.imgInfo[BDSP_ARM_IMG_ID_IFRAME(BDSP_Algorithm_eSoftFMM)].Buffer.pAddr,/*Encoded Interframe Image Address*/
			pDevice->codeInfo.imgInfo[BDSP_ARM_IMG_ID_IFRAME(BDSP_Algorithm_eSoftFMM)].ui32Size,/*Encoded Interframe Image Size */
			ui32Size);
	if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Arm_P_SoftFMMMixerStart: Error in Init Interframe"));
		goto end;
	}
	BDSP_MMA_P_FlushCache(Memory, ui32Size);

    ui32Size = sizeof(BDSP_SoftFmmConfigParams);
    errCode = BDSP_P_RequestMemory(&pArmSoftFMMMixer->sMemInfo.sMemoryPool, ui32Size, &Memory);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Arm_P_SoftFMMMixerStart: Unable to assign Usercfg memory for Soft FMM Stage"));
        goto end;
    }
    pArmSoftFMMMixer->sMemInfo.sUserCfg.Buffer = Memory;
    pArmSoftFMMMixer->sMemInfo.sUserCfg.ui32Size = ui32Size;

    ui32Size = sizeof(BDSP_SoftFMMStatusInfo);
    errCode = BDSP_P_RequestMemory(&pArmSoftFMMMixer->sMemInfo.sMemoryPool, ui32Size, &Memory);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Arm_P_SoftFMMMixerStart: Unable to assign Status memory for Soft FMM Stage"));
        goto end;
    }
    pArmSoftFMMMixer->sMemInfo.sAlgoStatus.Buffer = Memory;
    pArmSoftFMMMixer->sMemInfo.sAlgoStatus.ui32Size = ui32Size;

    errCode = BDSP_Arm_P_ProcessCreateFMMMixerCommand(pArmSoftFMM,&sSoftFMMCreateMixerCommand);
    if(BERR_SUCCESS != errCode)
    {
        BDBG_ERR(("BDSP_Arm_P_SoftFMM_MixerCreate: Failed in sending command to firmware"));
        errCode = BERR_TRACE(errCode);
        goto end;
    }

    BKNI_AcquireMutex(pDevice->deviceMutex);
    BLST_S_INIT(&pArmSoftFMMMixer->MixerInputList);
    BLST_S_INIT(&pArmSoftFMMMixer->MixerOutputList);
    BLST_S_INSERT_HEAD(&pArmSoftFMM->MixerList, pArmSoftFMMMixer, node);
    *pSoftFMMMixer = &pArmSoftFMMMixer->softFMMMixer;
    pArmSoftFMM->pMixer[pArmSoftFMM->numMixers++] =  (void *)pArmSoftFMMMixer;
    BKNI_ReleaseMutex(pDevice->deviceMutex);

    goto end;

end:
    BDBG_LEAVE(BDSP_Arm_P_SoftFMM_MixerCreate);
    return errCode;
}

/***************************************************************************
Summary:
Get Default SoftFMM Output Settings
***************************************************************************/
BERR_Code BDSP_Arm_P_SoftFMM_GetDefaultOutputSettings(
    void *pSoftFMMHandle,
    BDSP_SoftFMM_OutputSettings *pSettings     /* [out] */
)
{
    BERR_Code errCode = BERR_SUCCESS;
    BDSP_ArmSoftFMM *pArmSoftFMM = (BDSP_ArmSoftFMM *)pSoftFMMHandle;

    BDBG_ENTER(BDSP_Arm_P_SoftFMM_GetDefaultOutputSettings);

    BDBG_OBJECT_ASSERT(pArmSoftFMM, BDSP_ArmSoftFMM);

    pSettings->eSoftFMMOutputType = BDSP_SoftFMM_Output_eMAI;
    pSettings->ui32Enable = 0;
    pSettings->ui32SampleRate = 48000;
    pSettings->sSPDIFPacketizationConfig.eBurstFillType = BDSP_AF_P_BurstFill_ePauseBurst;
    pSettings->sSPDIFPacketizationConfig.ui32BitsPerSample = 24;
    pSettings->sSPDIFPacketizationConfig.ui32BurstPadding = 0;
    pSettings->sSPDIFPacketizationConfig.ui32BurstPeriod = 6;
    pSettings->sSPDIFPacketizationConfig.ui32ChannelStatusValid = 0;
    pSettings->sSPDIFPacketizationConfig.ui32Enable = 0;
    pSettings->sSPDIFPacketizationConfig.ui32StopBit = 0;
    pSettings->sSPDIFPacketizationConfig.uiLengthCode = 0;

    goto end;

end:
    BDBG_LEAVE(BDSP_Arm_P_SoftFMM_GetDefaultOutputSettings);
    return errCode;
}


/***************************************************************************
Summary:
Create a DSP SoftFMM Output
***************************************************************************/
BERR_Code BDSP_Arm_P_SoftFMMOutputCreate(
    void *pSoftFMMHandle,
    const BDSP_SoftFMM_OutputSettings *pSettings,
    BDSP_SoftFMMOutputHandle *pSoftFMMOutput    /* [out] */
)
{
    BERR_Code errCode = BERR_SUCCESS;
    BDSP_ArmSoftFMM *pArmSoftFMM = (BDSP_ArmSoftFMM *)pSoftFMMHandle;
    BDSP_Arm *pDevice;
    BDSP_ArmSoftFMM_Output *pArmSoftFMMOutput;

    BDBG_ENTER(BDSP_Arm_P_SoftFMMOutputCreate);

    BDBG_OBJECT_ASSERT(pArmSoftFMM, BDSP_ArmSoftFMM);
    pDevice = (BDSP_Arm *)pArmSoftFMM->pDevice;
    BDBG_OBJECT_ASSERT(pDevice,BDSP_Arm);

    if(BDSP_MAX_SOFT_FMM_OUTPUTS == pArmSoftFMM->numOutputs)
    {
        BDBG_ERR(("BDSP_Arm_P_SoftFMMOutputCreate: Cannot create more than %d Soft FMM Outputs",BDSP_MAX_SOFT_FMM_OUTPUTS));
        errCode = BERR_TRACE(BERR_NOT_SUPPORTED);
        goto end;
    }
    *pSoftFMMOutput = NULL;
    pArmSoftFMMOutput = (BDSP_ArmSoftFMM_Output *)BKNI_Malloc(sizeof(BDSP_ArmSoftFMM_Output));
    if(NULL == pArmSoftFMMOutput)
    {
        BDBG_ERR(("pArmSoftFMMOutput: Unable to allocate Memory for Opening the SoftFMM Output"));
        errCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto end;
    }
    BKNI_Memset(pArmSoftFMMOutput, 0, sizeof(*pArmSoftFMMOutput));
    BDBG_OBJECT_SET(pArmSoftFMMOutput, BDSP_ArmSoftFMM_Output);
    pArmSoftFMMOutput->pArmSoftFMM = pSoftFMMHandle;
    pArmSoftFMMOutput->settings = *pSettings;
    pArmSoftFMMOutput->hWConfig.ui32valid = 0;
    pArmSoftFMMOutput->ui32OutputIndex = pArmSoftFMM->numOutputs;

    BDSP_P_InitSoftFMMOutput(&pArmSoftFMMOutput->softFMMOutput, pArmSoftFMMOutput);

    pArmSoftFMMOutput->softFMMOutput.destroy                    = BDSP_Arm_P_SoftFMMOutputDestroy;
    pArmSoftFMMOutput->softFMMOutput.getInterruptHandlers_isr   = BDSP_Arm_P_GetSoftFMMOutputInterruptHandlers_isr;
    pArmSoftFMMOutput->softFMMOutput.setInterruptHandlers_isr   = BDSP_Arm_P_SetSoftFMMOutputInterruptHandlers_isr;
    pArmSoftFMMOutput->softFMMOutput.getSettings                = BDSP_Arm_P_SoftFMMOutputGetSettings;
    pArmSoftFMMOutput->softFMMOutput.setSettings                = BDSP_Arm_P_SoftFMMOutputSetSettings;
    pArmSoftFMMOutput->softFMMOutput.setSettings_isr            = BDSP_Arm_P_SoftFMMOutputSetSettings_isr;
    pArmSoftFMMOutput->softFMMOutput.getStatus                  = BDSP_Arm_P_SoftFMMOutputGetStatus;
    pArmSoftFMMOutput->softFMMOutput.setBufferConfig            = BDSP_Arm_P_SoftFMMOutputSetBufferConfig;

    BKNI_AcquireMutex(pDevice->deviceMutex);
    BLST_S_INSERT_HEAD(&pArmSoftFMM->OutputList, pArmSoftFMMOutput, node);
    *pSoftFMMOutput = &pArmSoftFMMOutput->softFMMOutput;
    pArmSoftFMM->pOutput[pArmSoftFMM->numOutputs++] =  (void *)pArmSoftFMMOutput;
    BKNI_ReleaseMutex(pDevice->deviceMutex);

    goto end;

end:
    BDBG_LEAVE(BDSP_Arm_P_SoftFMMOutputCreate);
    return errCode;
}

/***************************************************************************
Summary:
Get Default SoftFMM Input Settings
***************************************************************************/
BERR_Code BDSP_Arm_P_SoftFMM_GetDefaultInputSettings(
    void *pSoftFMMHandle,
    BDSP_SoftFMM_InputSettings *pSettings     /* [out] */
)
{
    BERR_Code errCode = BERR_SUCCESS;
    BDSP_ArmSoftFMM *pArmSoftFMM = (BDSP_ArmSoftFMM *)pSoftFMMHandle;
    uint32_t ui32ChannelIndex;

    BDBG_ENTER(BDSP_Arm_P_SoftFMM_GetDefaultInputSettings);
    BDBG_OBJECT_ASSERT(pArmSoftFMM, BDSP_ArmSoftFMM);

    for(ui32ChannelIndex = 0; ui32ChannelIndex < BDSP_AF_P_MAX_CHANNELS; ui32ChannelIndex++)
    {
        pSettings->sSoftFMMInputBufferDescriptor.pInterface[ui32ChannelIndex] = NULL;
    }
    pSettings->sZeroInsertionConfigParams.ui32Enable = 0;
    pSettings->sRateControlConfigParams.ui32Enable = 0;
    pSettings->sSrcConfigParams.ui32Enable = 0;
    pSettings->sMixerConfigParams.ui32Enable = 0;

    goto end;

end:
    BDBG_LEAVE(BDSP_Arm_P_SoftFMM_GetDefaultInputSettings);
    return errCode;
}


/***************************************************************************
Summary:
Create a DSP SoftFMM Input
***************************************************************************/
BERR_Code BDSP_Arm_P_SoftFMMInputCreate(
    void *pSoftFMMHandle,
    const BDSP_SoftFMM_InputSettings *pSettings,
    BDSP_SoftFMMInputHandle *pSoftFMMInput    /* [out] */
)
{
    BERR_Code errCode = BERR_SUCCESS;
    BDSP_ArmSoftFMM *pArmSoftFMM = (BDSP_ArmSoftFMM *)pSoftFMMHandle;
    BDSP_Arm *pDevice;
    BDSP_ArmSoftFMM_Input *pArmSoftFMMInput;


    BDBG_ENTER(BDSP_Arm_P_SoftFMMInputCreate);

    BDBG_OBJECT_ASSERT(pArmSoftFMM, BDSP_ArmSoftFMM);
    pDevice = (BDSP_Arm *)pArmSoftFMM->pDevice;
    BDBG_OBJECT_ASSERT(pDevice,BDSP_Arm);

    *pSoftFMMInput = NULL;
    pArmSoftFMMInput = (BDSP_ArmSoftFMM_Input *)BKNI_Malloc(sizeof(BDSP_ArmSoftFMM_Input));
    if(NULL == pArmSoftFMMInput)
    {
        BDBG_ERR(("pArmSoftFMMInput: Unable to allocate Memory for Opening the SoftFMM Input"));
        errCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto end;
    }
    BKNI_Memset(pArmSoftFMMInput, 0, sizeof(*pArmSoftFMMInput));
    BDBG_OBJECT_SET(pArmSoftFMMInput, BDSP_ArmSoftFMM_Input);
    pArmSoftFMMInput->pArmSoftFMM = pSoftFMMHandle;
    pArmSoftFMMInput->settings = *pSettings;
    pArmSoftFMMInput->ui32InputIndex = pArmSoftFMM->numInputs;

    BDSP_P_InitSoftFMMInput(&pArmSoftFMMInput->softFMMInput, pArmSoftFMMInput);

    pArmSoftFMMInput->softFMMInput.destroy                  = BDSP_Arm_P_SoftFMMInputDestroy;
    pArmSoftFMMInput->softFMMInput.getInterruptHandlers_isr = BDSP_Arm_P_GetSoftFMMInputInterruptHandlers_isr;
    pArmSoftFMMInput->softFMMInput.setInterruptHandlers_isr = BDSP_Arm_P_SetSoftFMMInputInterruptHandlers_isr;
    pArmSoftFMMInput->softFMMInput.getSettings              = BDSP_Arm_P_SoftFMMInputGetSettings;
    pArmSoftFMMInput->softFMMInput.setSettings              = BDSP_Arm_P_SoftFMMInputSetSettings;
    pArmSoftFMMInput->softFMMInput.setSettings_isr          = BDSP_Arm_P_SoftFMMInputSetSettings_isr;
    pArmSoftFMMInput->softFMMInput.getStatus                = BDSP_Arm_P_SoftFMMInputGetStatus;

    BKNI_AcquireMutex(pDevice->deviceMutex);
    BLST_S_INSERT_HEAD(&pArmSoftFMM->InputList, pArmSoftFMMInput, node);
    *pSoftFMMInput = &pArmSoftFMMInput->softFMMInput;
    pArmSoftFMM->pInput[pArmSoftFMM->numInputs++] =  (void *)pArmSoftFMMInput;
    BKNI_ReleaseMutex(pDevice->deviceMutex);

    goto end;


end:
    BDBG_LEAVE(BDSP_Arm_P_SoftFMMInputCreate);
    return errCode;
}

/***************************************************************************
Summary:
Destroy a DSP Soft FMM Mixer
***************************************************************************/
void BDSP_Arm_P_SoftFMMMixerDestroy(
    void *pMixerHandle
)
{
    BDSP_ArmSoftFMM *pArmSoftFMM;
    BDSP_Arm *pDevice;
    BDSP_ArmSoftFMM_Mixer *pArmSoftFMMMixer = (BDSP_ArmSoftFMM_Mixer *)pMixerHandle;

    BDBG_ENTER(BDSP_Arm_P_SoftFMMMixerDestroy);

    BDBG_OBJECT_ASSERT(pArmSoftFMMMixer, BDSP_ArmSoftFMM_Mixer);
    pArmSoftFMM = (BDSP_ArmSoftFMM *)pArmSoftFMMMixer->pArmSoftFMM;
    BDBG_OBJECT_ASSERT(pArmSoftFMM, BDSP_ArmSoftFMM);
    pDevice = (BDSP_Arm *)pArmSoftFMM->pDevice;
    BDBG_OBJECT_ASSERT(pDevice,BDSP_Arm);

    if(true == pArmSoftFMMMixer->started)
    {
        BDBG_ERR(("BDSP_Arm_P_SoftFMMMixerDestroy: Mixer (%p) is started, Stopping it by force", (void *)pArmSoftFMMMixer));
        BDSP_Arm_P_SoftFMMMixerStop(pMixerHandle);
    }

    BDSP_Arm_P_SoftFMMMixerRemoveAllInputs(pMixerHandle);
    BDSP_Arm_P_SoftFMMMixerRemoveAllOutputs(pMixerHandle);

    BDSP_MMA_P_FreeMemory(&(pArmSoftFMMMixer->sMemInfo.sMemoryPool.Memory));

    BKNI_AcquireMutex(pDevice->deviceMutex);
    BLST_S_REMOVE(&pArmSoftFMM->MixerList, pArmSoftFMMMixer, BDSP_ArmSoftFMM_Mixer, node);
    BKNI_ReleaseMutex(pDevice->deviceMutex);

    pArmSoftFMM->numMixers--;
    pArmSoftFMM->pMixer[pArmSoftFMMMixer->ui32MixerIndex] = NULL;

    BDSP_MMA_P_FreeMemory(&(pArmSoftFMMMixer->sMemInfo.sMemoryPool.Memory));

    BDBG_OBJECT_DESTROY(pArmSoftFMMMixer, BDSP_ArmSoftFMM_Mixer);
    BKNI_Free(pArmSoftFMMMixer);

    goto end;

end:
    BDBG_LEAVE(BDSP_Arm_P_SoftFMMMixerDestroy);
}

/***************************************************************************
Summary:
Start a DSP Soft FMM Mixer
***************************************************************************/
BERR_Code BDSP_Arm_P_SoftFMMMixerStart(
    void *pMixerHandle
)
{
    BERR_Code errCode = BERR_SUCCESS;
    BDSP_ArmSoftFMM_Mixer *pArmSoftFMMMixer = (BDSP_ArmSoftFMM_Mixer *)pMixerHandle;
    BDSP_ArmSoftFMM *pArmSoftFMM;
    BDSP_Arm_P_CodeDownloadInfo *pCodeInfo;
    BDSP_P_SoftFMMStartMixerCommand sStartFMMMixerCommand;

    BDBG_ENTER(BDSP_Arm_P_SoftFMMMixerStart);
    BDBG_OBJECT_ASSERT(pArmSoftFMMMixer, BDSP_ArmSoftFMM_Mixer);
    pArmSoftFMM = (BDSP_ArmSoftFMM *)pArmSoftFMMMixer->pArmSoftFMM;
    BDBG_OBJECT_ASSERT(pArmSoftFMM, BDSP_ArmSoftFMM);

    if(true == pArmSoftFMMMixer->started)
    {
        BDBG_MSG(("Mixer(%p) already started",pMixerHandle));
        return errCode;
    }

    pCodeInfo  = &pArmSoftFMMMixer->pArmSoftFMM->pDevice->codeInfo;


    sStartFMMMixerCommand.sStageConfig.sStageMemoryInfo.BaseAddr = pArmSoftFMMMixer->sMemInfo.sMemoryPool.Memory.offset;
    sStartFMMMixerCommand.sStageConfig.sStageMemoryInfo.Size = pArmSoftFMMMixer->sMemInfo.sMemoryPool.ui32Size;

    sStartFMMMixerCommand.sStageConfig.sAlgoCodeInfo.BaseAddr = pCodeInfo->imgInfo[BDSP_ARM_IMG_ID_CODE(BDSP_Algorithm_eSoftFMM)].Buffer.offset;
	sStartFMMMixerCommand.sStageConfig.sAlgoCodeInfo.Size     = pCodeInfo->imgInfo[BDSP_ARM_IMG_ID_CODE(BDSP_Algorithm_eSoftFMM)].ui32Size;

    sStartFMMMixerCommand.sStageConfig.sInterFrameInfo.BaseAddr 	   = pArmSoftFMMMixer->sMemInfo.sInterframe.Buffer.offset;
    sStartFMMMixerCommand.sStageConfig.sInterFrameInfo.Size     	   = pArmSoftFMMMixer->sMemInfo.sInterframe.ui32Size;

    sStartFMMMixerCommand.sStageConfig.sUserCfgInfo.BaseAddr           = pArmSoftFMMMixer->sMemInfo.sUserCfg.Buffer.offset;
    sStartFMMMixerCommand.sStageConfig.sUserCfgInfo.Size               = pArmSoftFMMMixer->sMemInfo.sUserCfg.ui32Size;

    sStartFMMMixerCommand.sStageConfig.sStatusInfo.BaseAddr           = pArmSoftFMMMixer->sMemInfo.sAlgoStatus.Buffer.offset;
    sStartFMMMixerCommand.sStageConfig.sStatusInfo.Size               = pArmSoftFMMMixer->sMemInfo.sAlgoStatus.ui32Size;

    sStartFMMMixerCommand.MixerIndex = pArmSoftFMMMixer->ui32MixerIndex;

    errCode = BDSP_Arm_P_ProcessStartFMMMixerCommand(pArmSoftFMM,&sStartFMMMixerCommand);
    if(BERR_SUCCESS != errCode)
    {
        BDBG_ERR(("BDSP_Arm_P_SoftFMMMixerStart: Failed in sending command to firmware"));
        errCode = BERR_TRACE(errCode);
        goto end;
    }

    pArmSoftFMMMixer->started = true;
    goto end;

end:
    BDBG_LEAVE(BDSP_Arm_P_SoftFMMMixerStart);
    return errCode;
}

/***************************************************************************
Summary:
Stop a DSP Soft FMM Mixer
***************************************************************************/
BERR_Code BDSP_Arm_P_SoftFMMMixerStop(
    void *pMixerHandle
)
{
    BERR_Code errCode = BERR_SUCCESS;
    BDSP_ArmSoftFMM_Mixer *pArmSoftFMMMixer = (BDSP_ArmSoftFMM_Mixer *)pMixerHandle;
    BDSP_ArmSoftFMM *pArmSoftFMM;
    BDSP_P_SoftFMMStopMixerCommand softFMMStopMixerCommand;

    BDBG_ENTER(BDSP_Arm_P_SoftFMMMixerStop);
    BDBG_OBJECT_ASSERT(pArmSoftFMMMixer, BDSP_ArmSoftFMM_Mixer);
    pArmSoftFMM = (BDSP_ArmSoftFMM *)pArmSoftFMMMixer->pArmSoftFMM;
    BDBG_OBJECT_ASSERT(pArmSoftFMM, BDSP_ArmSoftFMM);

    if(false == pArmSoftFMMMixer->started)
    {
        BDBG_MSG(("Mixer(%p) already stopped",pMixerHandle));
        return errCode;
    }

    softFMMStopMixerCommand.MixerIndex = pArmSoftFMMMixer->ui32MixerIndex;

    errCode = BDSP_Arm_P_ProcessStopFMMMixerCommand(pArmSoftFMM,&softFMMStopMixerCommand);
    if(BERR_SUCCESS != errCode)
    {
        BDBG_ERR(("BDSP_Arm_P_SoftFMMMixerStop: Failed in sending command to firmware"));
        errCode = BERR_TRACE(errCode);
        goto end;
    }

    pArmSoftFMMMixer->started = false;

    goto end;

end:
    BDBG_LEAVE(BDSP_Arm_P_SoftFMMMixerStop);
    return errCode;
}

/***************************************************************************
Summary:
Get DSP Soft FMM Mixer Status
***************************************************************************/
BERR_Code BDSP_Arm_P_SoftFMMMixerGetStatus(
    void *pMixerHandle,
    BDSP_SoftFMM_MixerStatus *pstatus
)
{
    BERR_Code errCode = BERR_SUCCESS;
    BDSP_ArmSoftFMM_Mixer *pArmSoftFMMMixer = (BDSP_ArmSoftFMM_Mixer *)pMixerHandle;
    BSTD_UNUSED(pstatus);

    BDBG_ENTER(BDSP_Arm_P_SoftFMMMixerGetStatus);
    BDBG_OBJECT_ASSERT(pArmSoftFMMMixer, BDSP_ArmSoftFMM_Mixer);
    errCode = BERR_TRACE(BERR_NOT_SUPPORTED);
    goto end;

end:
    BDBG_LEAVE(BDSP_Arm_P_SoftFMMMixerGetStatus);
    return errCode;
}

/***************************************************************************
Summary:
Get DSP Soft FMM Mixer Settings
***************************************************************************/
BERR_Code BDSP_Arm_P_SoftFMMMixerGetSettings(
    void *pMixerHandle,
    BDSP_SoftFMM_MixerSettings *pSettings
)
{
    BERR_Code errCode = BERR_SUCCESS;
    BDSP_ArmSoftFMM_Mixer *pArmSoftFMMMixer = (BDSP_ArmSoftFMM_Mixer *)pMixerHandle;

    BDBG_ENTER(BDSP_Arm_P_SoftFMMMixerGetSettings);
    BDBG_OBJECT_ASSERT(pArmSoftFMMMixer, BDSP_ArmSoftFMM_Mixer);

    *pSettings = pArmSoftFMMMixer->settings;

    goto end;

end:
    BDBG_LEAVE(BDSP_Arm_P_SoftFMMMixerGetSettings);
    return errCode;
}

/***************************************************************************
Summary:
Set DSP Soft FMM Mixer Settings
***************************************************************************/
BERR_Code BDSP_Arm_P_SoftFMMMixerSetSettings(
    void *pMixerHandle,
    BDSP_SoftFMM_MixerSettings *pSettings
)
{
    BERR_Code errCode = BERR_SUCCESS;
    BDSP_ArmSoftFMM_Mixer *pArmSoftFMMMixer = (BDSP_ArmSoftFMM_Mixer *)pMixerHandle;
    BDSP_P_SoftFMMMixerReConfigCommand sPayload;
    BDSP_ArmSoftFMM *pArmSoftFMM;

    BDBG_ENTER(BDSP_Arm_P_SoftFMMMixerSetSettings);
    BDBG_OBJECT_ASSERT(pArmSoftFMMMixer, BDSP_ArmSoftFMM_Mixer);
    pArmSoftFMM = (BDSP_ArmSoftFMM *)pArmSoftFMMMixer->pArmSoftFMM;
    BDBG_OBJECT_ASSERT(pArmSoftFMM, BDSP_ArmSoftFMM);

    pArmSoftFMMMixer->settings = *pSettings;

    sPayload.MixerIndex = pArmSoftFMMMixer->ui32MixerIndex;
    sPayload.sSoftFMMMixerSettings = *pSettings;

    errCode = BDSP_Arm_P_ProcessSoftFMMMixerReconfigCommand(pArmSoftFMM,&sPayload);
    if(BERR_SUCCESS != errCode)
    {
        BDBG_ERR(("BDSP_Arm_P_SoftFMMMixerSetSettings: Error (%d) in Set Settings command processing for SoftFMMMixer(%p)",errCode,pMixerHandle));
        errCode = BERR_TRACE(errCode);
        goto end;
    }

end:
    BDBG_LEAVE(BDSP_Arm_P_SoftFMMMixerSetSettings);
    return errCode;
}

/***************************************************************************
Summary:
Set DSP Soft FMM Mixer Settings ISR context
***************************************************************************/
BERR_Code BDSP_Arm_P_SoftFMMMixerSetSettings_isr(
    void *pMixerHandle,
    BDSP_SoftFMM_MixerSettings *pSettings
)
{
    BERR_Code errCode = BERR_SUCCESS;
    BDSP_ArmSoftFMM_Mixer *pArmSoftFMMMixer = (BDSP_ArmSoftFMM_Mixer *)pMixerHandle;
    BDSP_P_SoftFMMMixerReConfigCommand sPayload;
    BDSP_ArmSoftFMM *pArmSoftFMM;

    BDBG_ENTER(BDSP_Arm_P_SoftFMMMixerSetSettings_isr);
    BDBG_OBJECT_ASSERT(pArmSoftFMMMixer, BDSP_ArmSoftFMM_Mixer);
    pArmSoftFMM = (BDSP_ArmSoftFMM *)pArmSoftFMMMixer->pArmSoftFMM;
    BDBG_OBJECT_ASSERT(pArmSoftFMM, BDSP_ArmSoftFMM);

    pArmSoftFMMMixer->settings = *pSettings;

    sPayload.MixerIndex = pArmSoftFMMMixer->ui32MixerIndex;
    sPayload.sSoftFMMMixerSettings = *pSettings;

    errCode = BDSP_Arm_P_ProcessSoftFMMMixerReconfigCommand_isr(pArmSoftFMM,&sPayload);
    if(BERR_SUCCESS != errCode)
    {
        BDBG_ERR(("BDSP_Arm_P_SoftFMMMixerSetSettings_isr: Error (%d) in Set Settings command processing for SoftFMMMixer(%p)",errCode,pMixerHandle));
        errCode = BERR_TRACE(errCode);
        goto end;
    }

end:
    BDBG_LEAVE(BDSP_Arm_P_SoftFMMMixerSetSettings_isr);
    return errCode;
}

/***************************************************************************
Summary:
Add an input to DSP Soft FMM Mixer
***************************************************************************/
BERR_Code BDSP_Arm_P_SoftFMMMixerAddInput(
    void *pMixerHandle,
    void *pInputHandle
)
{
    BERR_Code errCode = BERR_SUCCESS;
    BDSP_ArmSoftFMM_Mixer *pArmSoftFMMMixer = (BDSP_ArmSoftFMM_Mixer *)pMixerHandle;
    BDSP_ArmSoftFMM_Input *pArmSoftFMMInput = (BDSP_ArmSoftFMM_Input *)pInputHandle;
    BDSP_ArmSoftFMM_Input *pExistingInput;
    BDSP_ArmSoftFMM *pArmSoftFMM;
    BDSP_Arm *pDevice;
    uint32_t ui32ChannelIndex;
    BDSP_P_SoftFMMAddInputCommand softFMMAddInputCommand;
    BDSP_AF_P_sIoPort *pIOPort;

    BDBG_ENTER(BDSP_Arm_P_SoftFMMMixerAddInput);
    BDBG_OBJECT_ASSERT(pArmSoftFMMMixer, BDSP_ArmSoftFMM_Mixer);
    pArmSoftFMM = (BDSP_ArmSoftFMM *)pArmSoftFMMMixer->pArmSoftFMM;
    BDBG_OBJECT_ASSERT(pArmSoftFMM, BDSP_ArmSoftFMM);
    pDevice = (BDSP_Arm *)pArmSoftFMM->pDevice;
    BDBG_OBJECT_ASSERT(pDevice,BDSP_Arm);
    BDBG_OBJECT_ASSERT(pArmSoftFMMInput, BDSP_ArmSoftFMM_Input);

    for (pExistingInput = BLST_S_FIRST(&pArmSoftFMMMixer->MixerInputList);
			pExistingInput != NULL;
			pExistingInput = BLST_S_NEXT(pExistingInput, node) )
    {
        if(pExistingInput == pArmSoftFMMInput)
        {
            errCode = BERR_SUCCESS;
            goto end;
        }
    }

    for(ui32ChannelIndex = 0; ui32ChannelIndex < pArmSoftFMMInput->settings.sSoftFMMInputBufferDescriptor.numBuffers; ui32ChannelIndex++)
    {
        if(NULL == pArmSoftFMMInput->settings.sSoftFMMInputBufferDescriptor.pInterface[ui32ChannelIndex])
        {
            BDBG_ERR(("BDSP_Arm_P_SoftFMMMixerAddInput: Input (%p) buffer interface not configured correctly",pInputHandle));
            errCode = BERR_TRACE(BERR_NOT_INITIALIZED);
            goto end;
        }
    }

    /* TODO : Send Command to firmware */
    pIOPort = &(softFMMAddInputCommand.sInputPort);
    /* TODO: Remove the below line if format is coming in properly */
    /*pArmSoftFMMInput->settings.sSoftFMMInputBufferDescriptor.pInterface[0]->format = 0x41;*/
    pArmSoftFMMInput->settings.sRateControlConfigParams.ui64RateControlCfgAddress =
            (uint64_t)((9 *sizeof(dramaddr_t)) + (pArmSoftFMMInput->settings.sSoftFMMInputBufferDescriptor.interfaceOffset[0]));
    BDSP_Arm_P_CreateSoftFMMIOPort(pDevice,&(pArmSoftFMMInput->settings.sSoftFMMInputBufferDescriptor),pIOPort);

    softFMMAddInputCommand.InputIndex = pArmSoftFMMInput->ui32InputIndex;
    softFMMAddInputCommand.MixerIndex = pArmSoftFMMMixer->ui32MixerIndex;

    softFMMAddInputCommand.sSoftFMMInputPortConfig.ui32PortIndex = pArmSoftFMMMixer->ui32MixerIndex;
    softFMMAddInputCommand.sSoftFMMInputPortConfig.sMixerConfigParams           = pArmSoftFMMInput->settings.sMixerConfigParams;
    softFMMAddInputCommand.sSoftFMMInputPortConfig.sRateControlConfigParams     = pArmSoftFMMInput->settings.sRateControlConfigParams;
    softFMMAddInputCommand.sSoftFMMInputPortConfig.sSrcConfigParams             = pArmSoftFMMInput->settings.sSrcConfigParams;
    softFMMAddInputCommand.sSoftFMMInputPortConfig.sZeroInsertionConfigParams   = pArmSoftFMMInput->settings.sZeroInsertionConfigParams;

    errCode = BDSP_Arm_P_ProcessAddSoftFMMInputCommand(pArmSoftFMM,&softFMMAddInputCommand);
    if(BERR_SUCCESS != errCode)
    {
        BDBG_ERR(("BDSP_Arm_P_SoftFMMMixerAddInput: Failed in sending command to firmware"));
        errCode = BERR_TRACE(errCode);
        goto end;
    }

    BKNI_AcquireMutex(pDevice->deviceMutex);
    BLST_S_INSERT_HEAD(&pArmSoftFMMMixer->MixerInputList, pArmSoftFMMInput, node);
    BKNI_ReleaseMutex(pDevice->deviceMutex);

    pArmSoftFMMInput->isConnectedtoMixer = true;

    pArmSoftFMMMixer->ui32NumActiveInputs++;

    goto end;

end:
    BDBG_LEAVE(BDSP_Arm_P_SoftFMMMixerAddInput);
    return errCode;
}

/***************************************************************************
Summary:
Remove an input from DSP Soft FMM Mixer
***************************************************************************/
void BDSP_Arm_P_SoftFMMMixerRemoveInput(
    void *pMixerHandle,
    void *pInputHandle
)
{
    BERR_Code errCode = BERR_SUCCESS;
    BDSP_ArmSoftFMM_Mixer *pArmSoftFMMMixer = (BDSP_ArmSoftFMM_Mixer *)pMixerHandle;
    BDSP_ArmSoftFMM_Input *pTmpArmSoftFMMInput, *pArmSoftFMMInput = (BDSP_ArmSoftFMM_Input *)pInputHandle;
    BDSP_ArmSoftFMM *pArmSoftFMM;
    BDSP_Arm *pDevice;
    BDSP_P_SoftFMMRemoveInputCommand softFMMRemoveInputCommand;

    BDBG_ENTER(BDSP_Arm_P_SoftFMMMixerRemoveInput);
    BDBG_OBJECT_ASSERT(pArmSoftFMMMixer, BDSP_ArmSoftFMM_Mixer);
    pArmSoftFMM = (BDSP_ArmSoftFMM *)pArmSoftFMMMixer->pArmSoftFMM;
    BDBG_OBJECT_ASSERT(pArmSoftFMM, BDSP_ArmSoftFMM);
    pDevice = (BDSP_Arm *)pArmSoftFMM->pDevice;
    BDBG_OBJECT_ASSERT(pDevice,BDSP_Arm);
    BDBG_OBJECT_ASSERT(pArmSoftFMMInput, BDSP_ArmSoftFMM_Input);

    for ( pTmpArmSoftFMMInput = BLST_S_FIRST(&pArmSoftFMMMixer->MixerInputList);
        pTmpArmSoftFMMInput != NULL;
        pTmpArmSoftFMMInput = BLST_S_NEXT(pTmpArmSoftFMMInput, node) )
    {
        if ( pTmpArmSoftFMMInput == pArmSoftFMMInput )
        {
            break;
        }
    }

    if ( pTmpArmSoftFMMInput == pArmSoftFMMInput )
    {
        /* TODO : Send Command to firmware */
        softFMMRemoveInputCommand.InputIndex = pArmSoftFMMInput->ui32InputIndex;
        softFMMRemoveInputCommand.MixerIndex = pArmSoftFMMMixer->ui32MixerIndex;

        errCode = BDSP_Arm_P_ProcessRemoveSoftFMMInputCommand(pArmSoftFMM,&softFMMRemoveInputCommand);
        if(BERR_SUCCESS != errCode)
        {
            BDBG_ERR(("BDSP_Arm_P_SoftFMMMixerRemoveInput: Failed in sending command to firmware"));
            errCode = BERR_TRACE(errCode);
            goto end;
        }

        BKNI_AcquireMutex(pDevice->deviceMutex);
        BLST_S_REMOVE(&pArmSoftFMMMixer->MixerInputList, pArmSoftFMMInput, BDSP_ArmSoftFMM_Input, node);
        BKNI_ReleaseMutex(pDevice->deviceMutex);

        pArmSoftFMMInput->isConnectedtoMixer = false;

        pArmSoftFMMMixer->ui32NumActiveInputs--;
    }

    goto end;

end:
    BDBG_LEAVE(BDSP_Arm_P_SoftFMMMixerRemoveInput);
}

/***************************************************************************
Summary:
Remove all inputs from DSP Soft FMM Mixer
***************************************************************************/
void BDSP_Arm_P_SoftFMMMixerRemoveAllInputs(
    void *pMixerHandle
)
{
    BDSP_ArmSoftFMM_Mixer *pArmSoftFMMMixer = (BDSP_ArmSoftFMM_Mixer *)pMixerHandle;
    BDSP_ArmSoftFMM_Input *pArmSoftFMMInput;

    BDBG_ENTER(BDSP_Arm_P_SoftFMMMixerRemoveAllInputs);
    BDBG_OBJECT_ASSERT(pArmSoftFMMMixer, BDSP_ArmSoftFMM_Mixer);

    while((pArmSoftFMMInput = BLST_S_FIRST(&pArmSoftFMMMixer->MixerInputList)))
    {
        BDSP_Arm_P_SoftFMMMixerRemoveInput(pMixerHandle,&pArmSoftFMMInput->softFMMInput);
    }

    goto end;

end:
    BDBG_LEAVE(BDSP_Arm_P_SoftFMMMixerRemoveAllInputs);
}

/***************************************************************************
Summary:
Add an output to DSP Soft FMM Mixer
***************************************************************************/
BERR_Code BDSP_Arm_P_SoftFMMMixerAddOutput(
    void *pMixerHandle,
    void *pOutputHandle
)
{
    BERR_Code errCode = BERR_SUCCESS;
    BDSP_ArmSoftFMM_Mixer *pArmSoftFMMMixer = (BDSP_ArmSoftFMM_Mixer *)pMixerHandle;
    BDSP_ArmSoftFMM_Output *pArmSoftFMMOutput = (BDSP_ArmSoftFMM_Output *)pOutputHandle;
    BDSP_ArmSoftFMM_Output *pExistingOutput;
    BDSP_ArmSoftFMM *pArmSoftFMM;
    BDSP_Arm *pDevice;
    uint32_t ui32ChannelIndex;
    BDSP_P_SoftFMMAddOutputCommand softFMMAddOutputCommand;
    BDSP_AF_P_sIoPort *pIOPort;

    BDBG_ENTER(BDSP_Arm_P_SoftFMMMixerAddOutput);
    BDBG_OBJECT_ASSERT(pArmSoftFMMMixer, BDSP_ArmSoftFMM_Mixer);
    pArmSoftFMM = (BDSP_ArmSoftFMM *)pArmSoftFMMMixer->pArmSoftFMM;
    BDBG_OBJECT_ASSERT(pArmSoftFMM, BDSP_ArmSoftFMM);
    pDevice = (BDSP_Arm *)pArmSoftFMM->pDevice;
    BDBG_OBJECT_ASSERT(pDevice,BDSP_Arm);
    BDBG_OBJECT_ASSERT(pArmSoftFMMOutput, BDSP_ArmSoftFMM_Output);

    for (pExistingOutput = BLST_S_FIRST(&pArmSoftFMMMixer->MixerOutputList);
            pExistingOutput != NULL;
            pExistingOutput = BLST_S_NEXT(pExistingOutput, node) )
    {
        if(pExistingOutput == pArmSoftFMMOutput)
        {
            errCode = BERR_SUCCESS;
            goto end;
        }
    }

    if(true == pArmSoftFMMMixer->started)
    {
        BDBG_ERR(("BDSP_Arm_P_SoftFMMMixerAddOutput: Mixer (%p) is started. Cannot add outputs", (void *)pArmSoftFMMMixer));
        errCode = BERR_TRACE(BERR_NOT_SUPPORTED);
        goto end;
    }
    for(ui32ChannelIndex = 0; ui32ChannelIndex < pArmSoftFMMOutput->bufferDescriptor.numBuffers; ui32ChannelIndex++)
    {
        if(NULL == pArmSoftFMMOutput->bufferDescriptor.pInterface[ui32ChannelIndex])
        {
            BDBG_ERR(("BDSP_Arm_P_SoftFMMMixerAddOutput: Output (%p) buffer interface not configured correctly",pOutputHandle));
            errCode = BERR_TRACE(BERR_NOT_INITIALIZED);
            goto end;
        }
    }

#if 0 /* TODO: Confirm if the below check is needed. If not, remove it */
    if(0 == pArmSoftFMMOutput->hWConfig.ui32valid)
    {
        BDBG_ERR(("BDSP_Arm_P_SoftFMMMixerAddOutput: Output (%p) HW configuration incorrect",pOutputHandle));
        errCode = BERR_TRACE(BERR_NOT_INITIALIZED);
        goto end;
    }
#endif

    /* TODO : Send Command to firmware */
    pIOPort = &(softFMMAddOutputCommand.sOutputPort);
    BDSP_Arm_P_CreateSoftFMMIOPort(pDevice,&pArmSoftFMMOutput->bufferDescriptor,pIOPort);

    BKNI_Memcpy((void *)&(softFMMAddOutputCommand.sHWConfig),
                (void *)&(pArmSoftFMMOutput->hWConfig),
                sizeof(BDSP_SoftFMM_Output_HWConfig));

    BKNI_Memcpy((void *)&(softFMMAddOutputCommand.sSoftFMMOutputSettings),
                (void *)&(pArmSoftFMMOutput->settings),
                sizeof(BDSP_SoftFMM_OutputSettings));

    softFMMAddOutputCommand.MixerIndex  = pArmSoftFMMMixer->ui32MixerIndex;
    softFMMAddOutputCommand.OutputIndex = pArmSoftFMMOutput->ui32OutputIndex;

    errCode = BDSP_Arm_P_ProcessAddSoftFMMOutputCommand(pArmSoftFMM,&softFMMAddOutputCommand);
    if(BERR_SUCCESS != errCode)
    {
        BDBG_ERR(("BDSP_Arm_P_SoftFMMMixerAddOutput: Failed in sending command to firmware"));
        errCode = BERR_TRACE(errCode);
        goto end;
    }

    /*TODO:Enabling the output buffer interface*/
    for(ui32ChannelIndex = 0; ui32ChannelIndex < pArmSoftFMMOutput->bufferDescriptor.numBuffers; ui32ChannelIndex++)
    {
        pArmSoftFMMOutput->bufferDescriptor.pInterface[ui32ChannelIndex]->config |= 0x1;
    }

    BKNI_AcquireMutex(pDevice->deviceMutex);
    BLST_S_INSERT_HEAD(&pArmSoftFMMMixer->MixerOutputList, pArmSoftFMMOutput, node);
    BKNI_ReleaseMutex(pDevice->deviceMutex);

    pArmSoftFMMOutput->isConnectedtoMixer = true;

    pArmSoftFMMMixer->ui32NumActiveOutputs++;

    goto end;

end:
    BDBG_LEAVE(BDSP_Arm_P_SoftFMMMixerAddOutput);
    return errCode;
}

/***************************************************************************
Summary:
Remove an output from DSP Soft FMM Mixer
***************************************************************************/
void BDSP_Arm_P_SoftFMMMixerRemoveOutput(
    void *pMixerHandle,
    void *pOutputHandle
)
{
    BERR_Code errCode = BERR_SUCCESS;
    BDSP_ArmSoftFMM_Mixer *pArmSoftFMMMixer = (BDSP_ArmSoftFMM_Mixer *)pMixerHandle;
    BDSP_ArmSoftFMM_Output *pTmpArmSoftFMMOutput, *pArmSoftFMMOutput = (BDSP_ArmSoftFMM_Output *)pOutputHandle;
    BDSP_ArmSoftFMM *pArmSoftFMM;
    BDSP_Arm *pDevice;
    BDSP_P_SoftFMMRemoveOutputCommand softFMMRemoveOutputCommand;

    BDBG_ENTER(BDSP_Arm_P_SoftFMMMixerRemoveOutput);
    BDBG_OBJECT_ASSERT(pArmSoftFMMMixer, BDSP_ArmSoftFMM_Mixer);
    pArmSoftFMM = (BDSP_ArmSoftFMM *)pArmSoftFMMMixer->pArmSoftFMM;
    BDBG_OBJECT_ASSERT(pArmSoftFMM, BDSP_ArmSoftFMM);
    pDevice = (BDSP_Arm *)pArmSoftFMM->pDevice;
    BDBG_OBJECT_ASSERT(pDevice,BDSP_Arm);
    BDBG_OBJECT_ASSERT(pArmSoftFMMOutput, BDSP_ArmSoftFMM_Output);
    if(true == pArmSoftFMMMixer->started)
    {
        BDBG_ERR(("BDSP_Arm_P_SoftFMMMixerRemoveOutput: Mixer (%p) is started. Cannot remove outputs", (void *)pArmSoftFMMMixer));
        BERR_TRACE(BERR_NOT_SUPPORTED);
        goto end;
    }

    for ( pTmpArmSoftFMMOutput = BLST_S_FIRST(&pArmSoftFMMMixer->MixerOutputList);
        pTmpArmSoftFMMOutput != NULL;
        pTmpArmSoftFMMOutput = BLST_S_NEXT(pTmpArmSoftFMMOutput, node) )
    {
        if ( pTmpArmSoftFMMOutput == pArmSoftFMMOutput )
        {
            break;
        }
    }

    if ( pTmpArmSoftFMMOutput == pArmSoftFMMOutput )
    {
        /* TODO : Send Command to firmware */
        softFMMRemoveOutputCommand.OutputIndex = pArmSoftFMMOutput->ui32OutputIndex;
        softFMMRemoveOutputCommand.MixerIndex = pArmSoftFMMMixer->ui32MixerIndex;

        errCode = BDSP_Arm_P_ProcessRemoveSoftFMMOutputCommand(pArmSoftFMM,&softFMMRemoveOutputCommand);
        if(BERR_SUCCESS != errCode)
        {
            BDBG_ERR(("BDSP_Arm_P_SoftFMMMixerRemoveOutput: Failed in sending command to firmware"));
            errCode = BERR_TRACE(errCode);
            goto end;
        }

        BKNI_AcquireMutex(pDevice->deviceMutex);
        BLST_S_REMOVE(&pArmSoftFMMMixer->MixerOutputList, pArmSoftFMMOutput, BDSP_ArmSoftFMM_Output, node);
        BKNI_ReleaseMutex(pDevice->deviceMutex);

        pArmSoftFMMOutput->isConnectedtoMixer = false;

        pArmSoftFMMMixer->ui32NumActiveOutputs--;
    }

    goto end;

end:
    BDBG_LEAVE(BDSP_Arm_P_SoftFMMMixerRemoveOutput);
}

/***************************************************************************
Summary:
Remove all outputs from DSP Soft FMM Mixer
***************************************************************************/
void BDSP_Arm_P_SoftFMMMixerRemoveAllOutputs(
    void *pMixerHandle
)
{
    BDSP_ArmSoftFMM_Mixer *pArmSoftFMMMixer = (BDSP_ArmSoftFMM_Mixer *)pMixerHandle;
    BDSP_ArmSoftFMM_Output *pArmSoftFMMOutput;

    BDBG_ENTER(BDSP_Arm_P_SoftFMMMixerRemoveAllOutputs);
    BDBG_OBJECT_ASSERT(pArmSoftFMMMixer, BDSP_ArmSoftFMM_Mixer);

    while((pArmSoftFMMOutput = BLST_S_FIRST(&pArmSoftFMMMixer->MixerOutputList)))
    {
        BDSP_Arm_P_SoftFMMMixerRemoveOutput(pMixerHandle,&pArmSoftFMMOutput->softFMMOutput);
    }

    goto end;

end:
    BDBG_LEAVE(BDSP_Arm_P_SoftFMMMixerRemoveAllOutputs);
}

/***************************************************************************
Summary:
Destroy a DSP Soft FMM Input
***************************************************************************/
void BDSP_Arm_P_SoftFMMInputDestroy(
    void *pInputHandle
)
{
    BDSP_ArmSoftFMM_Input *pArmSoftFMMInput = (BDSP_ArmSoftFMM_Input *)pInputHandle;
    BDSP_ArmSoftFMM *pArmSoftFMM;
    BDSP_Arm *pDevice;

    BDBG_ENTER(BDSP_Arm_P_SoftFMMInputDestroy);
    BDBG_OBJECT_ASSERT(pArmSoftFMMInput, BDSP_ArmSoftFMM_Input);
    pArmSoftFMM = (BDSP_ArmSoftFMM *)pArmSoftFMMInput->pArmSoftFMM;
    BDBG_OBJECT_ASSERT(pArmSoftFMM, BDSP_ArmSoftFMM);
    pDevice = (BDSP_Arm *)pArmSoftFMM->pDevice;
    BDBG_OBJECT_ASSERT(pDevice,BDSP_Arm);

    if(true == pArmSoftFMMInput->isConnectedtoMixer)
    {
        BDBG_ERR(("BDSP_Arm_P_SoftFMMInputDestroy: Input (%p) connected to mixer. Cannot be destroyed",pInputHandle));
        BERR_TRACE(BERR_NOT_SUPPORTED);
        goto end;
    }

    BKNI_AcquireMutex(pDevice->deviceMutex);
    BLST_S_REMOVE(&pArmSoftFMM->InputList, pArmSoftFMMInput, BDSP_ArmSoftFMM_Input, node);
    BKNI_ReleaseMutex(pDevice->deviceMutex);

    pArmSoftFMM->numInputs--;
    pArmSoftFMM->pInput[pArmSoftFMMInput->ui32InputIndex] = NULL;

    BDBG_OBJECT_DESTROY(pArmSoftFMMInput, BDSP_ArmSoftFMM_Input);
    BKNI_Free(pArmSoftFMMInput);

    goto end;

end:
    BDBG_LEAVE(BDSP_Arm_P_SoftFMMInputDestroy);
}

/***************************************************************************
Summary:
Get DSP Soft FMM Input Status
***************************************************************************/
BERR_Code BDSP_Arm_P_SoftFMMInputGetStatus(
    void *pInputHandle,
    BDSP_SoftFMM_InputStatus *pstatus
)
{
    BERR_Code errCode = BERR_SUCCESS;
    BDSP_ArmSoftFMM_Input *pArmSoftFMMInput = (BDSP_ArmSoftFMM_Input *)pInputHandle;
    BSTD_UNUSED(pstatus);

    BDBG_ENTER(BDSP_Arm_P_SoftFMMInputGetStatus);
    BDBG_OBJECT_ASSERT(pArmSoftFMMInput, BDSP_ArmSoftFMM_Input);
    errCode = BERR_TRACE(BERR_NOT_SUPPORTED);
    goto end;

end:
    BDBG_LEAVE(BDSP_Arm_P_SoftFMMInputGetStatus);
    return errCode;
}

/***************************************************************************
Summary:
Get DSP Soft FMM Input Settings
***************************************************************************/
BERR_Code BDSP_Arm_P_SoftFMMInputGetSettings(
    void *pInputHandle,
    BDSP_SoftFMM_InputSettings *pSettings
)
{
    BERR_Code errCode = BERR_SUCCESS;
    BDSP_ArmSoftFMM_Input *pArmSoftFMMInput = (BDSP_ArmSoftFMM_Input *)pInputHandle;

    BDBG_ENTER(BDSP_Arm_P_SoftFMMInputGetSettings);
    BDBG_OBJECT_ASSERT(pArmSoftFMMInput, BDSP_ArmSoftFMM_Input);

    *pSettings = pArmSoftFMMInput->settings;

    goto end;

end:
    BDBG_LEAVE(BDSP_Arm_P_SoftFMMInputGetSettings);
    return errCode;
}

/***************************************************************************
Summary:
Set DSP Soft FMM Input Settings
***************************************************************************/
BERR_Code BDSP_Arm_P_SoftFMMInputSetSettings(
    void *pInputHandle,
    BDSP_SoftFMM_InputSettings *pSettings
)
{
    BERR_Code errCode = BERR_SUCCESS;
    BDSP_ArmSoftFMM_Input *pArmSoftFMMInput = (BDSP_ArmSoftFMM_Input *)pInputHandle;
    BDSP_P_SoftFMMInputReConfigCommand sPayload;
    BDSP_ArmSoftFMM *pArmSoftFMM;

    BDBG_ENTER(BDSP_Arm_P_SoftFMMInputSetSettings);
    BDBG_OBJECT_ASSERT(pArmSoftFMMInput, BDSP_ArmSoftFMM_Input);
    pArmSoftFMM = (BDSP_ArmSoftFMM *)pArmSoftFMMInput->pArmSoftFMM;
    BDBG_OBJECT_ASSERT(pArmSoftFMM, BDSP_ArmSoftFMM);

    pArmSoftFMMInput->settings = *pSettings;

    if(true == pArmSoftFMMInput->isConnectedtoMixer)
    {
        sPayload.InputIndex = pArmSoftFMMInput->ui32InputIndex;
        sPayload.sSoftFMMInputPortConfig.ui32SampleRate             = pSettings->ui32SampleRate;
        sPayload.sSoftFMMInputPortConfig.sMixerConfigParams         = pSettings->sMixerConfigParams;
        sPayload.sSoftFMMInputPortConfig.sRateControlConfigParams   = pSettings->sRateControlConfigParams;
        sPayload.sSoftFMMInputPortConfig.sZeroInsertionConfigParams = pSettings->sZeroInsertionConfigParams;
        sPayload.sSoftFMMInputPortConfig.sSrcConfigParams           = pSettings->sSrcConfigParams;

        errCode = BDSP_Arm_P_ProcessSoftFMMInputReconfigCommand(pArmSoftFMM,&sPayload);
        if(BERR_SUCCESS != errCode)
        {
            BDBG_ERR(("BDSP_Arm_P_SoftFMMInputSetSettings: Error (%d) in Set Settings command processing for SoftFMMInput(%p)",errCode,pInputHandle));
            errCode = BERR_TRACE(errCode);
            goto end;
        }
    }

    goto end;

end:
    BDBG_LEAVE(BDSP_Arm_P_SoftFMMInputSetSettings);
    return errCode;
}

/***************************************************************************
Summary:
Set DSP Soft FMM Input Settings ISR context
***************************************************************************/
BERR_Code BDSP_Arm_P_SoftFMMInputSetSettings_isr(
    void *pInputHandle,
    BDSP_SoftFMM_InputSettings *pSettings
)
{
    BERR_Code errCode = BERR_SUCCESS;
    BDSP_ArmSoftFMM_Input *pArmSoftFMMInput = (BDSP_ArmSoftFMM_Input *)pInputHandle;
    BDSP_P_SoftFMMInputReConfigCommand sPayload;
    BDSP_ArmSoftFMM *pArmSoftFMM;

    BDBG_ENTER(BDSP_Arm_P_SoftFMMInputSetSettings_isr);
    BDBG_OBJECT_ASSERT(pArmSoftFMMInput, BDSP_ArmSoftFMM_Input);
    pArmSoftFMM = (BDSP_ArmSoftFMM *)pArmSoftFMMInput->pArmSoftFMM;
    BDBG_OBJECT_ASSERT(pArmSoftFMM, BDSP_ArmSoftFMM);

    pArmSoftFMMInput->settings = *pSettings;

    if(true == pArmSoftFMMInput->isConnectedtoMixer)
    {
        sPayload.InputIndex = pArmSoftFMMInput->ui32InputIndex;
        sPayload.sSoftFMMInputPortConfig.ui32SampleRate             = pSettings->ui32SampleRate;
        sPayload.sSoftFMMInputPortConfig.sMixerConfigParams         = pSettings->sMixerConfigParams;
        sPayload.sSoftFMMInputPortConfig.sRateControlConfigParams   = pSettings->sRateControlConfigParams;
        sPayload.sSoftFMMInputPortConfig.sZeroInsertionConfigParams = pSettings->sZeroInsertionConfigParams;
        sPayload.sSoftFMMInputPortConfig.sSrcConfigParams           = pSettings->sSrcConfigParams;

        errCode = BDSP_Arm_P_ProcessSoftFMMInputReconfigCommand_isr(pArmSoftFMM,&sPayload);
        if(BERR_SUCCESS != errCode)
        {
            BDBG_ERR(("BDSP_Arm_P_SoftFMMInputSetSettings_isr: Error (%d) in Set Settings command processing for SoftFMMInput(%p)",errCode,pInputHandle));
            errCode = BERR_TRACE(errCode);
            goto end;
        }
    }

    goto end;

end:
    BDBG_LEAVE(BDSP_Arm_P_SoftFMMInputSetSettings_isr);
    return errCode;
}


/***************************************************************************
Summary:
Destroy a DSP Soft FMM Output
***************************************************************************/
void BDSP_Arm_P_SoftFMMOutputDestroy(
    void *pOutputHandle
)
{
    BDSP_ArmSoftFMM_Output *pArmSoftFMMOutput = (BDSP_ArmSoftFMM_Output *)pOutputHandle;
    BDSP_ArmSoftFMM *pArmSoftFMM;
    BDSP_Arm *pDevice;

    BDBG_ENTER(BDSP_Arm_P_SoftFMMOutputDestroy);
    BDBG_OBJECT_ASSERT(pArmSoftFMMOutput, BDSP_ArmSoftFMM_Output);
    pArmSoftFMM = (BDSP_ArmSoftFMM *)pArmSoftFMMOutput->pArmSoftFMM;
    BDBG_OBJECT_ASSERT(pArmSoftFMM, BDSP_ArmSoftFMM);
    pDevice = (BDSP_Arm *)pArmSoftFMM->pDevice;
    BDBG_OBJECT_ASSERT(pDevice,BDSP_Arm);

    if(true == pArmSoftFMMOutput->isConnectedtoMixer)
    {
        BDBG_ERR(("BDSP_Arm_P_SoftFMMOutputDestroy: Output (%p) connected to mixer. Cannot be destroyed",pOutputHandle));
        BERR_TRACE(BERR_NOT_SUPPORTED);
        goto end;
    }

    BKNI_AcquireMutex(pDevice->deviceMutex);
    BLST_S_REMOVE(&pArmSoftFMM->OutputList, pArmSoftFMMOutput, BDSP_ArmSoftFMM_Output, node);
    BKNI_ReleaseMutex(pDevice->deviceMutex);

    pArmSoftFMM->numOutputs--;
    pArmSoftFMM->pOutput[pArmSoftFMMOutput->ui32OutputIndex] = NULL;

    BDBG_OBJECT_DESTROY(pArmSoftFMMOutput, BDSP_ArmSoftFMM_Output);
    BKNI_Free(pArmSoftFMMOutput);

    goto end;

end:
    BDBG_LEAVE(BDSP_Arm_P_SoftFMMOutputDestroy);
}

/***************************************************************************
Summary:
Get DSP Soft FMM Output Status
***************************************************************************/
BERR_Code BDSP_Arm_P_SoftFMMOutputGetStatus(
    void *pOutputHandle,
    BDSP_SoftFMM_OutputStatus *pstatus
)
{
    BERR_Code errCode = BERR_SUCCESS;
    BDSP_ArmSoftFMM_Output *pArmSoftFMMOutput = (BDSP_ArmSoftFMM_Output *)pOutputHandle;
    BSTD_UNUSED(pstatus);

    BDBG_ENTER(BDSP_Arm_P_SoftFMMOutputGetStatus);
    BDBG_OBJECT_ASSERT(pArmSoftFMMOutput, BDSP_ArmSoftFMM_Output);
    errCode = BERR_TRACE(BERR_NOT_SUPPORTED);
    goto end;

end:
    BDBG_LEAVE(BDSP_Arm_P_SoftFMMOutputGetStatus);
    return errCode;
}

/***************************************************************************
Summary:
Get DSP Soft FMM Output Settings
***************************************************************************/
BERR_Code BDSP_Arm_P_SoftFMMOutputGetSettings(
    void *pOutputHandle,
    BDSP_SoftFMM_OutputSettings *pSettings
)
{
    BERR_Code errCode = BERR_SUCCESS;
    BDSP_ArmSoftFMM_Output *pArmSoftFMMOutput = (BDSP_ArmSoftFMM_Output *)pOutputHandle;

    BDBG_ENTER(BDSP_Arm_P_SoftFMMOutputGetSettings);
    BDBG_OBJECT_ASSERT(pArmSoftFMMOutput, BDSP_ArmSoftFMM_Output);

    *pSettings = pArmSoftFMMOutput->settings;

    goto end;

end:
    BDBG_LEAVE(BDSP_Arm_P_SoftFMMOutputGetSettings);
    return errCode;
}

/***************************************************************************
Summary:
Set DSP Soft FMM Output Settings
***************************************************************************/
BERR_Code BDSP_Arm_P_SoftFMMOutputSetSettings(
    void *pOutputHandle,
    BDSP_SoftFMM_OutputSettings *pSettings
)
{
    BERR_Code errCode = BERR_SUCCESS;
    BDSP_ArmSoftFMM_Output *pArmSoftFMMOutput = (BDSP_ArmSoftFMM_Output *)pOutputHandle;
    BDSP_P_SoftFMMOutputReConfigCommand sPayload;
    BDSP_ArmSoftFMM *pArmSoftFMM;

    BDBG_ENTER(BDSP_Arm_P_SoftFMMOutputSetSettings);
    BDBG_OBJECT_ASSERT(pArmSoftFMMOutput, BDSP_ArmSoftFMM_Output);
    pArmSoftFMM = (BDSP_ArmSoftFMM *)pArmSoftFMMOutput->pArmSoftFMM;
    BDBG_OBJECT_ASSERT(pArmSoftFMM, BDSP_ArmSoftFMM);

    pArmSoftFMMOutput->settings = *pSettings;

    if(true == pArmSoftFMMOutput->isConnectedtoMixer)
    {
        sPayload.sSoftFMMOutputSettings = *pSettings;
        sPayload.OutputIndex = pArmSoftFMMOutput->ui32OutputIndex;

        errCode = BDSP_Arm_P_ProcessSoftFMMOutputReconfigCommand(pArmSoftFMM,&sPayload);
        if(BERR_SUCCESS != errCode)
        {
            BDBG_ERR(("BDSP_Arm_P_SoftFMMOutputSetSettings: Error (%d) in Set Settings command processing for SoftFMMOutput(%p)",errCode,pOutputHandle));
            errCode = BERR_TRACE(errCode);
            goto end;
        }
    }

end:
    BDBG_LEAVE(BDSP_Arm_P_SoftFMMOutputSetSettings);
    return errCode;

}

/***************************************************************************
Summary:
Set DSP Soft FMM Output Settings
***************************************************************************/
BERR_Code BDSP_Arm_P_SoftFMMOutputSetSettings_isr(
    void *pOutputHandle,
    BDSP_SoftFMM_OutputSettings *pSettings
)
{
    BERR_Code errCode = BERR_SUCCESS;
    BDSP_ArmSoftFMM_Output *pArmSoftFMMOutput = (BDSP_ArmSoftFMM_Output *)pOutputHandle;
    BDSP_P_SoftFMMOutputReConfigCommand sPayload;
    BDSP_ArmSoftFMM *pArmSoftFMM;

    BDBG_ENTER(BDSP_Arm_P_SoftFMMOutputSetSettings_isr);
    BDBG_OBJECT_ASSERT(pArmSoftFMMOutput, BDSP_ArmSoftFMM_Output);
    pArmSoftFMM = (BDSP_ArmSoftFMM *)pArmSoftFMMOutput->pArmSoftFMM;
    BDBG_OBJECT_ASSERT(pArmSoftFMM, BDSP_ArmSoftFMM);

    pArmSoftFMMOutput->settings = *pSettings;

    if(true == pArmSoftFMMOutput->isConnectedtoMixer)
    {
        sPayload.sSoftFMMOutputSettings = *pSettings;
        sPayload.OutputIndex = pArmSoftFMMOutput->ui32OutputIndex;

        errCode = BDSP_Arm_P_ProcessSoftFMMOutputReconfigCommand_isr(pArmSoftFMM,&sPayload);
        if(BERR_SUCCESS != errCode)
        {
            BDBG_ERR(("BDSP_Arm_P_SoftFMMOutputSetSettings_isr: Error (%d) in Set Settings command processing for SoftFMMOutput(%p)",errCode,pOutputHandle));
            errCode = BERR_TRACE(errCode);
            goto end;
        }
    }

end:
    BDBG_LEAVE(BDSP_Arm_P_SoftFMMOutputSetSettings_isr);
    return errCode;

}


/***************************************************************************
Summary:
Set DSP Soft FMM Output Buffer Config
***************************************************************************/
BERR_Code BDSP_Arm_P_SoftFMMOutputSetBufferConfig(
    void *pOutputHandle,
    BDSP_SoftFMMBufferDescriptor  *pBufferDescriptor,
    BDSP_SoftFMM_Output_HWConfig  *pHWConfig
)
{
    BERR_Code errCode = BERR_SUCCESS;
    BDSP_ArmSoftFMM_Output *pArmSoftFMMOutput = (BDSP_ArmSoftFMM_Output *)pOutputHandle;

    BDBG_ENTER(BDSP_Arm_P_SoftFMMOutputSetBufferConfig);
    BDBG_OBJECT_ASSERT(pArmSoftFMMOutput, BDSP_ArmSoftFMM_Output);

    pArmSoftFMMOutput->bufferDescriptor = *pBufferDescriptor;

#if 0
    if(1 == pHWConfig->ui32valid)
    {
        pArmSoftFMMOutput->hWConfig = *pHWConfig;
    }
#else
    BSTD_UNUSED(pHWConfig);
#ifdef BCHP_AUD2711RATE_CONTROL
    pArmSoftFMMOutput->hWConfig.ui32valid = 1;
    pArmSoftFMMOutput->hWConfig.stcPrimingCountRegAddr = (uint64_t)((uint64_t)BCHP_PHYSICAL_OFFSET + (uint64_t)BCHP_AUD2711RATE_MAI0_STC_PRIMING_COUNT);
    pArmSoftFMMOutput->hWConfig.stcSnapshotCntrRegAddr = (uint64_t)((uint64_t)BCHP_PHYSICAL_OFFSET + (uint64_t)BCHP_AUD2711RATE_MAI0_STC_SNAPSHOT_CNTR);
    pArmSoftFMMOutput->hWConfig.stcFrameSizeRegAddr    = (uint64_t)((uint64_t)BCHP_PHYSICAL_OFFSET + (uint64_t)BCHP_AUD2711RATE_MAI0_STC_FRAME_SIZE);
    pArmSoftFMMOutput->hWConfig.stcSnapshotCtrlRegAddr = (uint64_t)((uint64_t)BCHP_PHYSICAL_OFFSET + (uint64_t)BCHP_AUD2711RATE_MAI0_STC_SNAPSHOT_CTRL);
    pArmSoftFMMOutput->hWConfig.scCtrlRegAddr          = (uint64_t)((uint64_t)BCHP_PHYSICAL_OFFSET + (uint64_t)BCHP_AUD2711RATE_SC_CTRL);
#else
    pArmSoftFMMOutput->hWConfig.ui32valid = 0;
#endif
#endif

    goto end;

end:
    BDBG_LEAVE(BDSP_Arm_P_SoftFMMOutputSetBufferConfig);
    return errCode;
}


BERR_Code BDSP_Arm_P_InitSoftFMM(
    BDSP_ArmSoftFMM *pArmSoftFMM
)
{
    BERR_Code errCode = BERR_SUCCESS;

    BDBG_ENTER(BDSP_Arm_P_InitSoftFMM);

    pArmSoftFMM->numOutputs = 0;
    pArmSoftFMM->numInputs = 0;

    errCode = BKNI_CreateEvent(&pArmSoftFMM->hEvent);
    if (BERR_SUCCESS != errCode)
    {
        BDBG_ERR(("Unable to create event for Soft FMM %p",(void *)pArmSoftFMM));
        errCode = BERR_TRACE(errCode);
        goto end;
    }
    BKNI_ResetEvent(pArmSoftFMM->hEvent);

    BKNI_AcquireMutex(pArmSoftFMM->pDevice->deviceMutex);
    BLST_S_INIT(&pArmSoftFMM->InputList);
    BLST_S_INIT(&pArmSoftFMM->OutputList);
    BLST_S_INIT(&pArmSoftFMM->MixerList);
    BKNI_ReleaseMutex(pArmSoftFMM->pDevice->deviceMutex);

end:
    BDBG_LEAVE(BDSP_Arm_P_InitSoftFMM);
    return errCode;
}

BERR_Code BDSP_Arm_P_CalculateSoftFMMProcessMemory(
    unsigned *pMemoryRequired
)
{
    BERR_Code errCode = BERR_SUCCESS;
    unsigned MemoryRequired = 0;
    const BDSP_P_AlgorithmCodeInfo *pAlgoCodeInfo;

    BDBG_ENTER(BDSP_Arm_P_CalculateSoftFMMProcessMemory);
    *pMemoryRequired = 0;
    pAlgoCodeInfo = BDSP_Arm_P_LookupAlgorithmCodeInfo(BDSP_Algorithm_eSoftFMM);

    MemoryRequired += BDSP_ARM_IMG_SOFT_FMM_MP_AP_SHARED_MEMORY_SIZE;
    MemoryRequired += pAlgoCodeInfo->scratchBufferSize;
    MemoryRequired += (BDSP_MAX_MSGS_PER_QUEUE * sizeof(BDSP_P_Response));
    MemoryRequired += (BDSP_MAX_ASYNC_MSGS_PER_QUEUE * sizeof(BDSP_P_AsynMsg));
#if 0
    MemoryRequired += sizeof(BDSP_AF_P_sTASK_CONFIG); /* CIT Memory */
    MemoryRequired += sizeof(BDSP_AF_P_sTASK_CONFIG); /* CIT Re-Config Memory */
    MemoryRequired += sizeof(BDSP_AF_P_sOpSamplingFreq); /*Sample Rate LUT*/
    MemoryRequired += sizeof(BDSP_AF_P_sTASK_SCHEDULING_CONFIG);
    MemoryRequired += sizeof(BDSP_AF_P_sTASK_GATEOPEN_CONFIG);
    MemoryRequired += sizeof(BDSP_AF_P_sSTC_TRIGGER_CONFIG);
#endif
    MemoryRequired += BDSP_MAX_HOST_DSP_L2C_SIZE;/* Hole memory to beat Cache coherency*/
    MemoryRequired = BDSP_ALIGN_SIZE(MemoryRequired, 4096);  /* Align the size to 4k */
    *pMemoryRequired = MemoryRequired;

    BDBG_MSG(("BDSP_Arm_P_CalculateSoftFMMProcessMemory: Memory Allocated for Soft FMM Process = %d", MemoryRequired));
    BDBG_LEAVE(BDSP_Arm_P_CalculateSoftFMMProcessMemory);

    return errCode;
}

BERR_Code BDSP_Arm_P_AssignSoftFMMProcessMemory(
    BDSP_ArmSoftFMM *pArmSoftFMM
)
{
    BERR_Code errCode = BERR_SUCCESS;
    BDSP_Arm *pDevice;
    unsigned dspIndex =0;
    uint32_t ui32Size =0;
    BDSP_MMA_Memory Memory;
    const BDSP_P_AlgorithmCodeInfo *pAlgoCodeInfo;

    BDBG_ENTER(BDSP_Arm_P_AssignSoftFMMProcessMemory);
    BDBG_OBJECT_ASSERT(pArmSoftFMM, BDSP_ArmSoftFMM);

    pDevice = (BDSP_Arm *)pArmSoftFMM->pDevice;
    BDBG_OBJECT_ASSERT(pDevice, BDSP_Arm);

    /*Host Copy of Async Response Queue */
    pArmSoftFMM->processMemInfo.hostAsyncQueue.pAddr = (void *)BKNI_Malloc(BDSP_MAX_ASYNC_MSGS_PER_QUEUE * sizeof(BDSP_P_AsynMsg));
    if(NULL == pArmSoftFMM->processMemInfo.hostAsyncQueue.pAddr)
    {
        BDBG_ERR(("BDSP_Arm_P_AssignSoftFMMProcessMemory: Unable to allocate BKNI Memory for Async buffer copy of HOST %p for Soft FMM",(void *)pArmSoftFMM));
        goto end;
    }
    pArmSoftFMM->processMemInfo.hostAsyncQueue.ui32Size = (BDSP_MAX_ASYNC_MSGS_PER_QUEUE * sizeof(BDSP_P_AsynMsg));

#if 1
    ui32Size = BDSP_MAX_HOST_DSP_L2C_SIZE;
    errCode  = BDSP_P_RequestMemory(&pArmSoftFMM->processMemInfo.sMemoryPool, ui32Size, &Memory);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Arm_P_AssignSoftFMMProcessMemory: Unable to assign Cache Hole Memory of SoftFMM %p",(void *)pArmSoftFMM));
        goto end;
    }
    pArmSoftFMM->processMemInfo.sCacheHole.ui32Size = ui32Size;
    pArmSoftFMM->processMemInfo.sCacheHole.Buffer   = Memory;
#endif
    errCode = BDSP_Arm_P_AssignFreeFIFO(pDevice, dspIndex,&(pArmSoftFMM->processMemInfo.syncQueueParams.ui32FifoId), 1);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Arm_P_AssignSoftFMMProcessMemory: Unable to find free fifo for SYNC QUEUE of SoftFMM %p",(void *)pArmSoftFMM));
        goto end;
    }
    ui32Size = (BDSP_MAX_MSGS_PER_QUEUE * sizeof(BDSP_P_Response));
    errCode = BDSP_P_RequestMemory(&pArmSoftFMM->processMemInfo.sMemoryPool, ui32Size, &Memory);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Arm_P_AssignSoftFMMProcessMemory: Unable to assign RW memory for SYNC QUEUE of SoftFMM %p",(void *)pArmSoftFMM));
        goto end;
    }
    pArmSoftFMM->processMemInfo.syncQueueParams.ui32Size = ui32Size;
    pArmSoftFMM->processMemInfo.syncQueueParams.Memory   = Memory;

    errCode = BDSP_Arm_P_AssignFreeFIFO(pDevice, dspIndex,&(pArmSoftFMM->processMemInfo.asyncQueueParams.ui32FifoId), 1);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Arm_P_AssignSoftFMMProcessMemory: Unable to find free fifo for ASYNC QUEUE of SoftFMM %p",(void *)pArmSoftFMM));
        goto end;
    }
    ui32Size = (BDSP_MAX_ASYNC_MSGS_PER_QUEUE * sizeof(BDSP_P_AsynMsg));
    errCode = BDSP_P_RequestMemory(&pArmSoftFMM->processMemInfo.sMemoryPool, ui32Size, &Memory);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Arm_P_AssignSoftFMMProcessMemory: Unable to assign RW memory for ASYNC QUEUE of SoftFMM %p",(void *)pArmSoftFMM));
        goto end;
    }
    pArmSoftFMM->processMemInfo.asyncQueueParams.ui32Size = ui32Size;
    pArmSoftFMM->processMemInfo.asyncQueueParams.Memory   = Memory;

    ui32Size = BDSP_ARM_IMG_SOFT_FMM_MP_AP_SHARED_MEMORY_SIZE;
    errCode = BDSP_P_RequestMemory(&pArmSoftFMM->processMemInfo.sMemoryPool, ui32Size, &Memory);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Arm_P_AssignSoftFMMProcessMemory: Unable to assign RW memory for MP Shared Memory of SoftFMM %p",(void *)pArmSoftFMM));
        goto end;
    }
    pArmSoftFMM->processMemInfo.sMPSharedMemory.ui32Size = ui32Size;
    pArmSoftFMM->processMemInfo.sMPSharedMemory.Buffer     = Memory;

    pAlgoCodeInfo = BDSP_Arm_P_LookupAlgorithmCodeInfo(BDSP_Algorithm_eSoftFMM);
    ui32Size = pAlgoCodeInfo->scratchBufferSize;
    errCode = BDSP_P_RequestMemory(&pArmSoftFMM->processMemInfo.sMemoryPool, ui32Size, &Memory);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Arm_P_AssignSoftFMMProcessMemory: Unable to assign RW memory for Scratch Memory of SoftFMM %p",(void *)pArmSoftFMM));
        goto end;
    }
    pArmSoftFMM->processMemInfo.scratchMemory.ui32Size = ui32Size;
    pArmSoftFMM->processMemInfo.scratchMemory.Buffer   = Memory;

end:
    BDBG_LEAVE(BDSP_Arm_P_AssignSoftFMMProcessMemory);
    return errCode;
}

BERR_Code BDSP_Arm_P_StartSoftFMMProcess(BDSP_ArmSoftFMM *pArmSoftFMM)
{
    BERR_Code errCode = BERR_SUCCESS;
    BDSP_Arm      *pDevice;
    unsigned dspIndex =0;
    BDSP_P_StartFMMCommand sPayload;
    const BDSP_P_AlgorithmCodeInfo *psAlgoCodeInfo;
    BDSP_Arm_P_CodeDownloadInfo *pCodeInfo;

    BDBG_ENTER(BDSP_Arm_P_StartSoftFMMProcess);
    BDBG_OBJECT_ASSERT(pArmSoftFMM, BDSP_ArmSoftFMM);
    pDevice = (BDSP_Arm *)pArmSoftFMM->pDevice;
    BDBG_OBJECT_ASSERT(pDevice, BDSP_Arm);

    BKNI_Memset(&sPayload,0,sizeof(BDSP_P_StartFMMCommand));

    errCode = BDSP_Arm_P_InitMsgQueue(pArmSoftFMM->hSyncQueue, pDevice->memInfo.softFifo[dspIndex].Buffer);
    if (BERR_SUCCESS != errCode)
    {
        BDBG_ERR(("BDSP_Arm_P_StartSoftFMMProcess: Sync Queue Init failed for Soft FMM %p",(void *)pArmSoftFMM));
        errCode = BERR_TRACE(errCode);
        goto end;
    }

    errCode = BDSP_Arm_P_InitMsgQueue(pArmSoftFMM->hAsyncQueue, pDevice->memInfo.softFifo[dspIndex].Buffer);
    if (BERR_SUCCESS != errCode)
    {
        BDBG_ERR(("BDSP_Arm_P_StartSoftFMMProcess: ASync Queue Init failed for Soft FMM %p",(void *)pArmSoftFMM));
        errCode = BERR_TRACE(errCode);
        goto end;
    }

    if(pDevice->codeInfo.preloadImages == false)
    {
        errCode = BDSP_Arm_P_DownloadAlgorithm(pDevice, BDSP_Algorithm_eSoftFMM);
        if (errCode != BERR_SUCCESS)
        {
            errCode = BERR_TRACE(errCode);
            BDBG_ERR(("BDSP_Arm_P_StartSoftFMMProcess: Error in downloading Algorithm(%d)",BDSP_Algorithm_eSoftFMM));
            goto end;
        }
    }

    psAlgoCodeInfo = BDSP_Arm_P_LookupAlgorithmCodeInfo(BDSP_Algorithm_eSoftFMM);
    sPayload.processConfig.ui32ScratchSize = psAlgoCodeInfo->scratchBufferSize;
    pCodeInfo  = &pArmSoftFMM->pDevice->codeInfo;
    sPayload.processConfig.sLookUpTableInfo.BaseAddr= pCodeInfo->imgInfo[BDSP_ARM_IMG_ID_TABLE(BDSP_Algorithm_eSoftFMM)].Buffer.offset;
    sPayload.processConfig.sLookUpTableInfo.Size = pCodeInfo->imgInfo[BDSP_ARM_IMG_ID_TABLE(BDSP_Algorithm_eSoftFMM)].ui32Size;
    sPayload.eSchedulingMode = BDSP_P_SchedulingMode_eMaster;
    sPayload.eTaskType = BDSP_P_TaskType_eRMS;
    sPayload.ui32SchedulingLevel = 1;
    sPayload.ui32TaskId          = 0x100;
    sPayload.ui32SyncQueueFifoId = pArmSoftFMM->hSyncQueue->ui32FifoId;
    sPayload.ui32AsyncQueueFifoId= pArmSoftFMM->hAsyncQueue->ui32FifoId;
    sPayload.ui32EventEnableMask = pArmSoftFMM->eventEnabledMask;
    sPayload.sTaskMemoryInfo.BaseAddr   = pArmSoftFMM->processMemInfo.sMemoryPool.Memory.offset;
    sPayload.sTaskMemoryInfo.Size       = pArmSoftFMM->processMemInfo.sMemoryPool.ui32Size;
    sPayload.sSharedMemoryInfo.BaseAddr = pArmSoftFMM->processMemInfo.sMPSharedMemory.Buffer.offset;
    sPayload.sSharedMemoryInfo.Size     = pArmSoftFMM->processMemInfo.sMPSharedMemory.ui32Size;
    sPayload.sScratchMemoryInfo.BaseAddr = pArmSoftFMM->processMemInfo.scratchMemory.Buffer.offset;
    sPayload.sScratchMemoryInfo.Size    = pArmSoftFMM->processMemInfo.scratchMemory.ui32Size;

    errCode = BDSP_Arm_P_ProcessStartFMMCommand(pArmSoftFMM, &sPayload);
    if (BERR_SUCCESS != errCode)
    {
        BDBG_ERR(("BDSP_Arm_P_StartSoftFMMProcess: Error in Start Soft FMM Process %p",(void *)pArmSoftFMM));
        errCode = BERR_TRACE(errCode);
        goto end;
    }

    goto end;

end:
    BDBG_LEAVE(BDSP_Arm_P_StartSoftFMMProcess);
    return errCode;

}
