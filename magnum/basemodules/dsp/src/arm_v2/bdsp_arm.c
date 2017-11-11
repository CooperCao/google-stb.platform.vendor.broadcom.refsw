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

BDBG_MODULE(bdsp_arm);

void BDSP_Arm_GetDefaultSettings(
    BDSP_ArmSettings *pSettings     /* [out] */
)
{

    BDBG_ENTER( BDSP_Arm_GetDefaultSettings );
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));

#if BDSP_IMG_SUPPORT
    pSettings->pImageContext = BDSP_ARM_IMG_Context;
    pSettings->pImageInterface = &BDSP_ARM_IMG_Interface;
#endif

    pSettings->preloadImages = false;
    pSettings->authenticationEnabled =false;

    pSettings->maxAlgorithms[BDSP_AlgorithmType_eAudioDecode]        = BDSP_MAX_DECODE_CTXT;
    pSettings->maxAlgorithms[BDSP_AlgorithmType_eAudioPassthrough]   = BDSP_MAX_PASSTHROUGH_CTXT;
    pSettings->maxAlgorithms[BDSP_AlgorithmType_eAudioEncode]        = BDSP_MAX_ENCODE_CTXT;
    pSettings->maxAlgorithms[BDSP_AlgorithmType_eAudioMixer]         = BDSP_MAX_MIXER_CTXT;
    pSettings->maxAlgorithms[BDSP_AlgorithmType_eAudioEchoCanceller] = BDSP_MAX_ECHOCANCELLER_CTXT;
    pSettings->maxAlgorithms[BDSP_AlgorithmType_eAudioProcessing]    = BDSP_MAX_AUDIO_PROCESSING_CTXT;

    pSettings->maxAlgorithms[BDSP_AlgorithmType_eVideoDecode]        = 0;
    pSettings->maxAlgorithms[BDSP_AlgorithmType_eVideoEncode]        = 0;
    pSettings->maxAlgorithms[BDSP_AlgorithmType_eSecurity]           = 0;

    pSettings->NumDevices = BDSP_ARM_MAX_DSP;
    BDBG_LEAVE( BDSP_Arm_GetDefaultSettings );
}

BERR_Code BDSP_Arm_Open(
    BDSP_Handle             *pDsp,     /* [out] */
    BCHP_Handle              chpHandle,
    BREG_Handle              regHandle,
    BMMA_Heap_Handle         memHandle,
    BINT_Handle              intHandle,
    BTMR_Handle              tmrHandle,
    const BDSP_ArmSettings  *pSettings
)
{
    BERR_Code errCode = BERR_SUCCESS;
    BDSP_Arm *pArm;

    BDBG_ENTER(BDSP_Arm_Open);
    BDBG_ASSERT(NULL != pDsp);
    BDBG_ASSERT(NULL != chpHandle);
    BDBG_ASSERT(NULL != regHandle);
    BDBG_ASSERT(NULL != memHandle);
    BDBG_ASSERT(NULL != intHandle);
    BDBG_ASSERT(NULL != pSettings);
    /* tmr is not required */
    BSTD_UNUSED(tmrHandle);

    pArm = BKNI_Malloc(sizeof(BDSP_Arm));
    if ( NULL == pArm )
    {
        BDBG_ERR(("BDSP_Arm_Open: Unable to allocate memory for Arm device"));
        errCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto end;
    }
    BKNI_Memset(pArm, 0, sizeof(BDSP_Arm));

    /* Init device */
    BDSP_P_InitDevice(&pArm->device, pArm);

    pArm->device.close = BDSP_Arm_P_Close;
    pArm->device.getDefaultContextSettings = BDSP_Arm_P_GetDefaultContextSettings;
    pArm->device.createContext = BDSP_Arm_P_CreateContext;
    pArm->device.getStatus= BDSP_Arm_P_GetStatus;
    pArm->device.powerStandby= NULL; /* BDSP_Arm_P_PowerStandby; */
    pArm->device.powerResume= NULL; /* BDSP_Arm_P_PowerResume; */
    pArm->device.getAlgorithmInfo= BDSP_Arm_P_GetAlgorithmInfo;
    pArm->device.allocateExternalInterrupt = NULL; /* BDSP_Arm_P_AllocateExternalInterrupt;*/
    pArm->device.freeExternalInterrupt = NULL; /* BDSP_Arm_P_FreeExternalInterrupt;*/
    pArm->device.getExternalInterruptInfo = NULL; /* BDSP_Arm_P_GetExternalInterruptInfo;*/
    pArm->device.processAudioCapture = NULL;
    pArm->device.getDebugBuffer = NULL;
    pArm->device.consumeDebugData = NULL;
    pArm->device.getCoreDumpStatus = NULL;
    pArm->device.getDownloadStatus = BDSP_Arm_P_GetDownloadStatus;
    pArm->device.initialize = BDSP_Arm_P_Initialize;
#if !B_REFSW_MINIMAL
    pArm->device.getDefaultDatasyncSettings = BDSP_P_GetDefaultDatasyncSettings;
#endif /*!B_REFSW_MINIMAL*/
    pArm->device.getDefaultTsmSettings = BDSP_P_GetDefaultTsmSettings;

    /* Init context lists */
    BLST_S_INIT(&pArm->contextList);

    /* Save Settings and params */
    pArm->deviceSettings = *pSettings;
    pArm->chpHandle = chpHandle;
    pArm->regHandle = regHandle;
    pArm->memHandle = memHandle;
    pArm->intHandle = intHandle;
    BDBG_OBJECT_SET(pArm, BDSP_Arm);

    errCode = BDSP_Arm_P_ValidateVersion(&pArm->deviceSettings);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Arm_Open: Error in Validating the versions for Arm"));
        errCode = BERR_TRACE(errCode);
        goto error;
    }

    errCode = BDSP_Arm_P_InitDeviceSettings(pArm);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Arm_Open: Error in Initialising Settings for Arm"));
        errCode = BERR_TRACE(errCode);
        goto error;
    }

    errCode = BDSP_Arm_P_Open(pArm);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Arm_Open: Error in Opening and processing Open for Arm"));
        errCode = BERR_TRACE(errCode);
        goto error;
    }

    if(pArm->deviceSettings.authenticationEnabled == false)
    {
        errCode = BDSP_Arm_P_OpenUserApp(pArm);
        if(errCode != BERR_SUCCESS)
        {
            BDBG_ERR(("BDSP_Arm_Open: Unable to Open the Arm User App"));
            errCode = BERR_TRACE(errCode);
            goto error;
        }

        errCode = BDSP_Arm_P_CheckDspAlive(pArm);
        if (errCode!=BERR_SUCCESS)
        {
            BDBG_ERR(("BDSP_Arm_Open: DSP not alive"));
            errCode = BERR_TRACE(errCode);
            goto error;
        }
    }

    *pDsp = &pArm->device;
    goto end;

error:
    BDBG_OBJECT_DESTROY(pArm, BDSP_Arm);
    BKNI_Free(pArm);
end:
    BDBG_LEAVE(BDSP_Arm_Open);
    return errCode;
}
