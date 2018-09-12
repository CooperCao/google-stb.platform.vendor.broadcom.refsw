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

#ifndef BDSP_ARM_SOFT_FMM_PRIV_H_
#define BDSP_ARM_SOFT_FMM_PRIV_H_

#include "bdsp_arm_priv_include.h"
#include "bdsp_soft_fmm.h"
#include "bdsp_soft_fmm_struct.h"



BDBG_OBJECT_ID_DECLARE(BDSP_ArmSoftFMM);
typedef struct BDSP_ArmSoftFMM
{
    BDBG_OBJECT(BDSP_ArmSoftFMM)
    BDSP_SoftFMM softFMM;
    BDSP_Arm *pDevice;

    uint32_t ui32FMMId;
    uint32_t eventEnabledMask;

    BDSP_P_MsgQueueHandle hSyncQueue;
    BDSP_P_MsgQueueHandle hAsyncQueue;

    BDSP_P_SoftFMMMemoryInfo processMemInfo;

    uint32_t ui32CommandCounter;

    uint32_t numOutputs;
    void *pOutput[BDSP_MAX_SOFT_FMM_OUTPUTS];

    uint32_t numInputs;
    void *pInput[BDSP_MAX_SOFT_FMM_INPUTS];

    uint32_t numMixers;
    void *pMixer[BDSP_MAX_SOFT_FMM_MIXERS];
    BKNI_EventHandle hEvent;

    BLST_S_HEAD(BDSP_SoftFMM_OutputList, BDSP_ArmSoftFMM_Output) OutputList;
    BLST_S_HEAD(BDSP_SoftFMM_InputList, BDSP_ArmSoftFMM_Input) InputList;
    BLST_S_HEAD(BDSP_SoftFMM_MixerList, BDSP_ArmSoftFMM_Mixer) MixerList;
}BDSP_ArmSoftFMM;

BDBG_OBJECT_ID_DECLARE(BDSP_ArmSoftFMM_Output);
typedef struct BDSP_ArmSoftFMM_Output
{
    BDBG_OBJECT(BDSP_ArmSoftFMM_Output)

    BDSP_SoftFMM_Output softFMMOutput;
    const BDSP_ArmSoftFMM *pArmSoftFMM;

    BDSP_SoftFMMOutputInterruptHandlers interruptHandlers;
    BDSP_SoftFMM_OutputSettings settings;
    BDSP_SoftFMM_Output_HWConfig hWConfig;
    BDSP_SoftFMMBufferDescriptor bufferDescriptor;

    uint32_t ui32OutputIndex;
    bool isConnectedtoMixer;

    BLST_S_ENTRY(BDSP_ArmSoftFMM_Output) node;
}BDSP_ArmSoftFMM_Output;

BDBG_OBJECT_ID_DECLARE(BDSP_ArmSoftFMM_Input);

typedef struct BDSP_ArmSoftFMM_Input
{
    BDBG_OBJECT(BDSP_ArmSoftFMM_Input)

    BDSP_SoftFMM_Input softFMMInput;
    const BDSP_ArmSoftFMM *pArmSoftFMM;

    BDSP_SoftFMMInputInterruptHandlers interruptHandlers;
    BDSP_SoftFMM_InputSettings settings;

    uint32_t ui32InputIndex;

    bool isConnectedtoMixer;

    BLST_S_ENTRY(BDSP_ArmSoftFMM_Input) node;
}BDSP_ArmSoftFMM_Input;

BDBG_OBJECT_ID_DECLARE(BDSP_ArmSoftFMM_Mixer);

typedef struct BDSP_ArmSoftFMM_Mixer
{
    BDBG_OBJECT(BDSP_ArmSoftFMM_Mixer)

    BDSP_SoftFMM_Mixer softFMMMixer;
    const BDSP_ArmSoftFMM *pArmSoftFMM;

    uint32_t ui32NumActiveInputs;
    uint32_t ui32NumActiveOutputs;

    uint32_t ui32MixerIndex;

    bool started;

    BDSP_SoftFMM_MixerSettings settings;
    BDSP_P_SoftFMMStageMemoryInfo sMemInfo;

    BLST_S_ENTRY(BDSP_ArmSoftFMM_Mixer) node;
    BLST_S_HEAD(BDSP_SoftFMM_Mixer_InputList, BDSP_ArmSoftFMM_Input) MixerInputList;
    BLST_S_HEAD(BDSP_SoftFMM_Mixer_OutputList, BDSP_ArmSoftFMM_Output) MixerOutputList;
}BDSP_ArmSoftFMM_Mixer;



BERR_Code BDSP_Arm_P_SoftFMMOpen(
    void                *pDeviceHandle,
    BDSP_SoftFMMHandle  *pSoftFMM   /* [out] Returns the SoftFMM Handle */
);

BERR_Code BDSP_Arm_P_SoftFMM_Close(
    void *pSoftFMMHandle
    );


/***************************************************************************
Summary:
Get Default SoftFMM Mixer Settings
***************************************************************************/
BERR_Code BDSP_Arm_P_SoftFMM_GetDefaultMixerSettings(
    void *pSoftFMMHandle,
    BDSP_SoftFMM_MixerSettings *pSettings     /* [out] */
    );


/***************************************************************************
Summary:
Create a DSP SoftFMM Mixer
***************************************************************************/
BERR_Code BDSP_Arm_P_SoftFMM_MixerCreate(
    void *pSoftFMMHandle,
    const BDSP_SoftFMM_MixerSettings *pSettings,
    BDSP_SoftFMMMixerHandle *pSoftFMMMixer    /* [out] */
    );

/***************************************************************************
Summary:
Get Default SoftFMM Output Settings
***************************************************************************/
BERR_Code BDSP_Arm_P_SoftFMM_GetDefaultOutputSettings(
    void *pSoftFMMHandle,
    BDSP_SoftFMM_OutputSettings *pSettings     /* [out] */
    );


/***************************************************************************
Summary:
Create a DSP SoftFMM Output
***************************************************************************/
BERR_Code BDSP_Arm_P_SoftFMMOutputCreate(
    void *pSoftFMMHandle,
    const BDSP_SoftFMM_OutputSettings *pSettings,
    BDSP_SoftFMMOutputHandle *pSoftFMMOutput    /* [out] */
    );

/***************************************************************************
Summary:
Get Default SoftFMM Input Settings
***************************************************************************/
BERR_Code BDSP_Arm_P_SoftFMM_GetDefaultInputSettings(
    void *pSoftFMMHandle,
    BDSP_SoftFMM_InputSettings *pSettings     /* [out] */
    );


/***************************************************************************
Summary:
Create a DSP SoftFMM Input
***************************************************************************/
BERR_Code BDSP_Arm_P_SoftFMMInputCreate(
    void *pSoftFMMHandle,
    const BDSP_SoftFMM_InputSettings *pSettings,
    BDSP_SoftFMMInputHandle *pSoftFMMInput    /* [out] */
    );

/***************************************************************************
Summary:
Destroy a DSP Soft FMM Mixer
***************************************************************************/
void BDSP_Arm_P_SoftFMMMixerDestroy(
    void *pMixerHandle
    );

/***************************************************************************
Summary:
Start a DSP Soft FMM Mixer
***************************************************************************/
BERR_Code BDSP_Arm_P_SoftFMMMixerStart(
    void *pMixerHandle
    );

/***************************************************************************
Summary:
Stop a DSP Soft FMM Mixer
***************************************************************************/
BERR_Code BDSP_Arm_P_SoftFMMMixerStop(
    void *pMixerHandle
    );

/***************************************************************************
Summary:
Get DSP Soft FMM Mixer Status
***************************************************************************/
BERR_Code BDSP_Arm_P_SoftFMMMixerGetStatus(
    void *pMixerHandle,
    BDSP_SoftFMM_MixerStatus *pstatus
    );

/***************************************************************************
Summary:
Get DSP Soft FMM Mixer Settings
***************************************************************************/
BERR_Code BDSP_Arm_P_SoftFMMMixerGetSettings(
    void *pMixerHandle,
    BDSP_SoftFMM_MixerSettings *pSettings
    );

/***************************************************************************
Summary:
Set DSP Soft FMM Mixer Settings
***************************************************************************/
BERR_Code BDSP_Arm_P_SoftFMMMixerSetSettings(
    void *pMixerHandle,
    BDSP_SoftFMM_MixerSettings *pSettings
    );

/***************************************************************************
Summary:
Set DSP Soft FMM Mixer Settings ISR context
***************************************************************************/
BERR_Code BDSP_Arm_P_SoftFMMMixerSetSettings_isr(
    void *pMixerHandle,
    BDSP_SoftFMM_MixerSettings *pSettings
    );

/***************************************************************************
Summary:
Add an input to DSP Soft FMM Mixer
***************************************************************************/
BERR_Code BDSP_Arm_P_SoftFMMMixerAddInput(
    void *pMixerHandle,
    void *pInputHandle
    );

/***************************************************************************
Summary:
Remove an input from DSP Soft FMM Mixer
***************************************************************************/
void BDSP_Arm_P_SoftFMMMixerRemoveInput(
    void *pMixerHandle,
    void *pInputHandle
    );

/***************************************************************************
Summary:
Remove all inputs from DSP Soft FMM Mixer
***************************************************************************/
void BDSP_Arm_P_SoftFMMMixerRemoveAllInputs(
    void *pMixerHandle
    );

/***************************************************************************
Summary:
Add an output to DSP Soft FMM Mixer
***************************************************************************/
BERR_Code BDSP_Arm_P_SoftFMMMixerAddOutput(
    void *pMixerHandle,
    void *pOutputHandle
    );

/***************************************************************************
Summary:
Remove an output from DSP Soft FMM Mixer
***************************************************************************/
void BDSP_Arm_P_SoftFMMMixerRemoveOutput(
    void *pMixerHandle,
    void *pOutputHandle
    );

/***************************************************************************
Summary:
Remove all outputs from DSP Soft FMM Mixer
***************************************************************************/
void BDSP_Arm_P_SoftFMMMixerRemoveAllOutputs(
    void *pMixerHandle
    );

/***************************************************************************
Summary:
Destroy a DSP Soft FMM Input
***************************************************************************/
void BDSP_Arm_P_SoftFMMInputDestroy(
    void *pInputHandle
    );

/***************************************************************************
Summary:
Get DSP Soft FMM Input Status
***************************************************************************/
BERR_Code BDSP_Arm_P_SoftFMMInputGetStatus(
    void *pInputHandle,
    BDSP_SoftFMM_InputStatus *pstatus
    );

/***************************************************************************
Summary:
Get DSP Soft FMM Input Settings
***************************************************************************/
BERR_Code BDSP_Arm_P_SoftFMMInputGetSettings(
    void *pInputHandle,
    BDSP_SoftFMM_InputSettings *pSettings
    );

/***************************************************************************
Summary:
Set DSP Soft FMM Input Settings
***************************************************************************/
BERR_Code BDSP_Arm_P_SoftFMMInputSetSettings(
    void *pInputHandle,
    BDSP_SoftFMM_InputSettings *pSettings
    );

/***************************************************************************
Summary:
Set DSP Soft FMM Input Settings ISR context
***************************************************************************/
BERR_Code BDSP_Arm_P_SoftFMMInputSetSettings_isr(
    void *pInputHandle,
    BDSP_SoftFMM_InputSettings *pSettings
    );


/***************************************************************************
Summary:
Destroy a DSP Soft FMM Output
***************************************************************************/
void BDSP_Arm_P_SoftFMMOutputDestroy(
    void *pOutputHandle
    );

/***************************************************************************
Summary:
Get DSP Soft FMM Output Status
***************************************************************************/
BERR_Code BDSP_Arm_P_SoftFMMOutputGetStatus(
    void *pOutputHandle,
    BDSP_SoftFMM_OutputStatus *pstatus
    );

/***************************************************************************
Summary:
Get DSP Soft FMM Output Settings
***************************************************************************/
BERR_Code BDSP_Arm_P_SoftFMMOutputGetSettings(
    void *pOutputHandle,
    BDSP_SoftFMM_OutputSettings *pSettings
    );

/***************************************************************************
Summary:
Set DSP Soft FMM Output Settings
***************************************************************************/
BERR_Code BDSP_Arm_P_SoftFMMOutputSetSettings(
    void *pOutputHandle,
    BDSP_SoftFMM_OutputSettings *pSettings
    );

/***************************************************************************
Summary:
Set DSP Soft FMM Output Settings
***************************************************************************/
BERR_Code BDSP_Arm_P_SoftFMMOutputSetSettings_isr(
    void *pOutputHandle,
    BDSP_SoftFMM_OutputSettings *pSettings
    );

/***************************************************************************
Summary:
Set DSP Soft FMM Output Buffer Config
***************************************************************************/
BERR_Code BDSP_Arm_P_SoftFMMOutputSetBufferConfig(
    void *pOutputHandle,
    BDSP_SoftFMMBufferDescriptor  *pBufferDescriptor,
    BDSP_SoftFMM_Output_HWConfig  *pHWConfig
    );

/***************************************************************************
Summary:
Init Soft FMM Params
***************************************************************************/
BERR_Code BDSP_Arm_P_InitSoftFMM(
    BDSP_ArmSoftFMM *pArmSoftFMM
    );

BERR_Code BDSP_Arm_P_CalculateSoftFMMProcessMemory(
    unsigned *pMemoryRequired
    );

BERR_Code BDSP_Arm_P_AssignSoftFMMProcessMemory(
    BDSP_ArmSoftFMM *pArmSoftFMM
    );

/***************************************************************************
Summary:
Start the Soft FMM Process in firmware
***************************************************************************/
BERR_Code BDSP_Arm_P_StartSoftFMMProcess(
    BDSP_ArmSoftFMM *pArmSoftFMM
    );

#endif /* BDSP_ARM_SOFT_FMM_PRIV_H_ */
