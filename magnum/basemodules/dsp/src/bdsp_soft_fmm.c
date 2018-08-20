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

 #include "bdsp_soft_fmm.h"



 /***************************************************************************
Summary:
Get Current Interrupt Handlers for SoftFMM Input
***************************************************************************/
void BDSP_SoftFMM_Input_GetInterruptHandlers_isr(
    BDSP_SoftFMMInputHandle softFMMInput,
    BDSP_SoftFMMInputInterruptHandlers *pHandlers   /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(softFMMInput, BDSP_SoftFMM_Input);
    BDBG_ASSERT(NULL != pHandlers);

    if ( softFMMInput->getInterruptHandlers_isr)
    {
        (void)softFMMInput->getInterruptHandlers_isr(softFMMInput->pSoftFMMInput,pHandlers);
    }
    else
    {
        BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

/***************************************************************************
Summary:
Set Current Interrupt Handlers for SoftFMM Input
***************************************************************************/
BERR_Code BDSP_SoftFMM_Input_SetInterruptHandlers_isr(
    BDSP_SoftFMMInputHandle softFMMInput,
    const BDSP_SoftFMMInputInterruptHandlers *pHandlers
    )
{
    BDBG_OBJECT_ASSERT(softFMMInput, BDSP_SoftFMM_Input);
    BDBG_ASSERT(NULL != pHandlers);

    if ( softFMMInput->setInterruptHandlers_isr)
    {
        return softFMMInput->setInterruptHandlers_isr(softFMMInput->pSoftFMMInput,pHandlers);
    }
    else
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

/***************************************************************************
Summary:
Get Current Interrupt Handlers for Soft FMM Output
***************************************************************************/
void BDSP_SoftFMM_Output_GetInterruptHandlers_isr(
    BDSP_SoftFMMOutputHandle softFMMOutput,
    BDSP_SoftFMMOutputInterruptHandlers *pHandlers   /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(softFMMOutput, BDSP_SoftFMM_Output);
    BDBG_ASSERT(NULL != pHandlers);

    if ( softFMMOutput->getInterruptHandlers_isr)
    {
        (void)softFMMOutput->getInterruptHandlers_isr(softFMMOutput->pSoftFMMOutput,pHandlers);
    }
    else
    {
        BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

/***************************************************************************
Summary:
Set Current Interrupt Handlers for SoftFMM Output
***************************************************************************/
BERR_Code BDSP_SoftFMM_Output_SetInterruptHandlers_isr(
    BDSP_SoftFMMOutputHandle softFMMOutput,
    const BDSP_SoftFMMOutputInterruptHandlers *pHandlers   /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(softFMMOutput, BDSP_SoftFMM_Output);
    BDBG_ASSERT(NULL != pHandlers);

    if ( softFMMOutput->setInterruptHandlers_isr)
    {
        return softFMMOutput->setInterruptHandlers_isr(softFMMOutput->pSoftFMMOutput,pHandlers);
    }
    else
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

/***************************************************************************
Summary:
SoftFMM Close
***************************************************************************/
BERR_Code BDSP_SoftFMM_Close(
    BDSP_SoftFMMHandle softFMM
    )
{
    BDBG_OBJECT_ASSERT(softFMM, BDSP_SoftFMM);
    if(softFMM->close)
    {
        return softFMM->close(softFMM->pSoftFMM);
    }
    else
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

/***************************************************************************
Summary:
Get Default SoftFMM Mixer Settings
***************************************************************************/
BERR_Code BDSP_SoftFMM_GetDefaultMixerSettings(
    BDSP_SoftFMMHandle softFMM,
    BDSP_SoftFMM_MixerSettings *pSettings     /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(softFMM, BDSP_SoftFMM);
    if(softFMM->getDefaultMixerSettings)
    {
        return softFMM->getDefaultMixerSettings(softFMM->pSoftFMM,pSettings);
    }
    else
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}


/***************************************************************************
Summary:
Create a DSP SoftFMM Mixer
***************************************************************************/
BERR_Code BDSP_SoftFMM_MixerCreate(
    BDSP_SoftFMMHandle softFMM,
    const BDSP_SoftFMM_MixerSettings *pSettings,
    BDSP_SoftFMMMixerHandle *pSoftFMMMixer    /* [out] */
)
{
    BDBG_OBJECT_ASSERT(softFMM, BDSP_SoftFMM);
    if(softFMM->createMixer)
    {
        return softFMM->createMixer(softFMM->pSoftFMM,pSettings,pSoftFMMMixer);
    }
    else
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

/***************************************************************************
Summary:
Get Default SoftFMM Output Settings
***************************************************************************/
BERR_Code BDSP_SoftFMM_GetDefaultOutputSettings(
    BDSP_SoftFMMHandle softFMM,
    BDSP_SoftFMM_OutputSettings *pSettings     /* [out] */
)
{
    BDBG_OBJECT_ASSERT(softFMM, BDSP_SoftFMM);
    if(softFMM->getDefaultOutputSettings)
    {
        return softFMM->getDefaultOutputSettings(softFMM->pSoftFMM,pSettings);
    }
    else
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}


/***************************************************************************
Summary:
Create a DSP SoftFMM Output
***************************************************************************/
BERR_Code BDSP_SoftFMM_Output_Create(
    BDSP_SoftFMMHandle softFMM,
    const BDSP_SoftFMM_OutputSettings *pSettings,
    BDSP_SoftFMMOutputHandle *pSoftFMMOutput    /* [out] */
)
{
    BDBG_OBJECT_ASSERT(softFMM, BDSP_SoftFMM);
    if(softFMM->createOutput)
    {
        return softFMM->createOutput(softFMM->pSoftFMM,pSettings,pSoftFMMOutput);
    }
    else
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

/***************************************************************************
Summary:
Get Default SoftFMM Input Settings
***************************************************************************/
BERR_Code BDSP_SoftFMM_GetDefaultInputSettings(
    BDSP_SoftFMMHandle softFMM,
    BDSP_SoftFMM_InputSettings *pSettings     /* [out] */
)
{
    BDBG_OBJECT_ASSERT(softFMM, BDSP_SoftFMM);
    if(softFMM->getDefaultInputSettings)
    {
        return softFMM->getDefaultInputSettings(softFMM->pSoftFMM,pSettings);
    }
    else
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}


/***************************************************************************
Summary:
Create a DSP SoftFMM Input
***************************************************************************/
BERR_Code BDSP_SoftFMM_Input_Create(
    BDSP_SoftFMMHandle softFMM,
    const BDSP_SoftFMM_InputSettings *pSettings,
    BDSP_SoftFMMInputHandle *pSoftFMMInput    /* [out] */
)
{
    BDBG_OBJECT_ASSERT(softFMM, BDSP_SoftFMM);
    if(softFMM->createInput)
    {
        return softFMM->createInput(softFMM->pSoftFMM,pSettings,pSoftFMMInput);
    }
    else
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

/***************************************************************************
Summary:
Destroy a DSP Soft FMM Mixer
***************************************************************************/
void BDSP_SoftFMM_Mixer_Destroy(
    BDSP_SoftFMMMixerHandle mixer
)
{
    BDBG_OBJECT_ASSERT(mixer, BDSP_SoftFMM_Mixer);
    if(mixer->destroy)
    {
        mixer->destroy(mixer->pSoftFMMMixer);
    }
    else
    {
        BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

/***************************************************************************
Summary:
Start a DSP Soft FMM Mixer
***************************************************************************/
BERR_Code BDSP_SoftFMM_Mixer_Start(
    BDSP_SoftFMMMixerHandle mixer
)
{
    BDBG_OBJECT_ASSERT(mixer, BDSP_SoftFMM_Mixer);
    if(mixer->start)
    {
        return mixer->start(mixer->pSoftFMMMixer);
    }
    else
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

/***************************************************************************
Summary:
Stop a DSP Soft FMM Mixer
***************************************************************************/
BERR_Code BDSP_SoftFMM_Mixer_Stop(
    BDSP_SoftFMMMixerHandle mixer
)
{
    BDBG_OBJECT_ASSERT(mixer, BDSP_SoftFMM_Mixer);
    if(mixer->stop)
    {
        return mixer->stop(mixer->pSoftFMMMixer);
    }
    else
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

/***************************************************************************
Summary:
Get DSP Soft FMM Mixer Status
***************************************************************************/
BERR_Code BDSP_SoftFMM_Mixer_GetStatus(
    BDSP_SoftFMMMixerHandle mixer,
    BDSP_SoftFMM_MixerStatus *pstatus
)
{
    BDBG_OBJECT_ASSERT(mixer, BDSP_SoftFMM_Mixer);
    if(mixer->getStatus)
    {
        return mixer->getStatus(mixer->pSoftFMMMixer,pstatus);
    }

    else
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

/***************************************************************************
Summary:
Get DSP Soft FMM Mixer Settings
***************************************************************************/
BERR_Code BDSP_SoftFMM_Mixer_GetSettings(
    BDSP_SoftFMMMixerHandle mixer,
    BDSP_SoftFMM_MixerSettings *pSettings
)
{
    BDBG_OBJECT_ASSERT(mixer, BDSP_SoftFMM_Mixer);
    if(mixer->getSettings)
    {
        return mixer->getSettings(mixer->pSoftFMMMixer,pSettings);
    }

    else
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

/***************************************************************************
Summary:
Set DSP Soft FMM Mixer Settings
***************************************************************************/
BERR_Code BDSP_SoftFMM_Mixer_SetSettings(
    BDSP_SoftFMMMixerHandle mixer,
    BDSP_SoftFMM_MixerSettings *pSettings
)
{
    BDBG_OBJECT_ASSERT(mixer, BDSP_SoftFMM_Mixer);
    if(mixer->setSettings)
    {
        return mixer->setSettings(mixer->pSoftFMMMixer,pSettings);
    }

    else
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

/***************************************************************************
Summary:
Set DSP Soft FMM Mixer Settings
***************************************************************************/
BERR_Code BDSP_SoftFMM_Mixer_SetSettings_isr(
    BDSP_SoftFMMMixerHandle mixer,
    BDSP_SoftFMM_MixerSettings *pSettings
)
{
    BDBG_OBJECT_ASSERT(mixer, BDSP_SoftFMM_Mixer);
    if(mixer->setSettings_isr)
    {
        return mixer->setSettings_isr(mixer->pSoftFMMMixer,pSettings);
    }

    else
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

/***************************************************************************
Summary:
Add an input to DSP Soft FMM Mixer
***************************************************************************/
BERR_Code BDSP_SoftFMM_Mixer_AddInput(
    BDSP_SoftFMMMixerHandle mixer,
    BDSP_SoftFMMInputHandle input
)
{
    BDBG_OBJECT_ASSERT(mixer, BDSP_SoftFMM_Mixer);
    BDBG_OBJECT_ASSERT(input, BDSP_SoftFMM_Input);
    if(mixer->addInput)
    {
        return mixer->addInput(mixer->pSoftFMMMixer,input->pSoftFMMInput);
    }

    else
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

/***************************************************************************
Summary:
Remove an input from DSP Soft FMM Mixer
***************************************************************************/
void BDSP_SoftFMM_Mixer_RemoveInput(
    BDSP_SoftFMMMixerHandle mixer,
    BDSP_SoftFMMInputHandle input
)
{
    BDBG_OBJECT_ASSERT(mixer, BDSP_SoftFMM_Mixer);
    BDBG_OBJECT_ASSERT(input, BDSP_SoftFMM_Input);
    if(mixer->removeInput)
    {
        mixer->removeInput(mixer->pSoftFMMMixer,input->pSoftFMMInput);
    }

    else
    {
        BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

/***************************************************************************
Summary:
Remove all inputs from DSP Soft FMM Mixer
***************************************************************************/
void BDSP_SoftFMM_Mixer_RemoveAllInputs(
    BDSP_SoftFMMMixerHandle mixer
)
{
    BDBG_OBJECT_ASSERT(mixer, BDSP_SoftFMM_Mixer);
    if(mixer->removeAllInputs)
    {
        mixer->removeAllInputs(mixer->pSoftFMMMixer);
    }

    else
    {
        BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

/***************************************************************************
Summary:
Add an output to DSP Soft FMM Mixer
***************************************************************************/
BERR_Code BDSP_SoftFMM_Mixer_AddOutput(
    BDSP_SoftFMMMixerHandle mixer,
    BDSP_SoftFMMOutputHandle output
)
{
    BDBG_OBJECT_ASSERT(mixer, BDSP_SoftFMM_Mixer);
    BDBG_OBJECT_ASSERT(output, BDSP_SoftFMM_Output);
    if(mixer->addOutput)
    {
        return mixer->addOutput(mixer->pSoftFMMMixer,output->pSoftFMMOutput);
    }

    else
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

/***************************************************************************
Summary:
Remove an output from DSP Soft FMM Mixer
***************************************************************************/
void BDSP_SoftFMM_Mixer_RemoveOutput(
    BDSP_SoftFMMMixerHandle mixer,
    BDSP_SoftFMMOutputHandle output
)
{
    BDBG_OBJECT_ASSERT(mixer, BDSP_SoftFMM_Mixer);
    BDBG_OBJECT_ASSERT(output, BDSP_SoftFMM_Output);
    if(mixer->removeOutput)
    {
        mixer->removeOutput(mixer->pSoftFMMMixer,output->pSoftFMMOutput);
    }

    else
    {
        BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

/***************************************************************************
Summary:
Remove all outputs from DSP Soft FMM Mixer
***************************************************************************/
void BDSP_SoftFMM_Mixer_RemoveAllOutputs(
    BDSP_SoftFMMMixerHandle mixer
)
{
    BDBG_OBJECT_ASSERT(mixer, BDSP_SoftFMM_Mixer);
    if(mixer->removeAllOutputs)
    {
        mixer->removeAllOutputs(mixer->pSoftFMMMixer);
    }

    else
    {
        BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

/***************************************************************************
Summary:
Destroy a DSP Soft FMM Input
***************************************************************************/
void BDSP_SoftFMM_Input_Destroy(
    BDSP_SoftFMMInputHandle input
)
{
    BDBG_OBJECT_ASSERT(input, BDSP_SoftFMM_Input);
    if(input->destroy)
    {
        input->destroy(input->pSoftFMMInput);
    }
    else
    {
        BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

/***************************************************************************
Summary:
Get DSP Soft FMM Input Status
***************************************************************************/
BERR_Code BDSP_SoftFMM_Input_GetStatus(
    BDSP_SoftFMMInputHandle input,
    BDSP_SoftFMM_InputStatus *pstatus
)
{
    BDBG_OBJECT_ASSERT(input, BDSP_SoftFMM_Input);
    if(input->getStatus)
    {
        return input->getStatus(input->pSoftFMMInput,pstatus);
    }
    else
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

/***************************************************************************
Summary:
Get DSP Soft FMM Input Settings
***************************************************************************/
BERR_Code BDSP_SoftFMM_Input_GetSettings(
    BDSP_SoftFMMInputHandle input,
    BDSP_SoftFMM_InputSettings *pSettings
)
{
    BDBG_OBJECT_ASSERT(input, BDSP_SoftFMM_Input);
    if(input->getSettings)
    {
        return input->getSettings(input->pSoftFMMInput,pSettings);
    }
    else
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

/***************************************************************************
Summary:
Set DSP Soft FMM Input Settings
***************************************************************************/
BERR_Code BDSP_SoftFMM_Input_SetSettings(
    BDSP_SoftFMMInputHandle input,
    BDSP_SoftFMM_InputSettings *pSettings
)
{
    BDBG_OBJECT_ASSERT(input, BDSP_SoftFMM_Input);
    if(input->setSettings)
    {
        return input->setSettings(input->pSoftFMMInput,pSettings);
    }
    else
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

/***************************************************************************
Summary:
Set DSP Soft FMM Input Settings ISR context
***************************************************************************/
BERR_Code BDSP_SoftFMM_Input_SetSettings_isr(
    BDSP_SoftFMMInputHandle input,
    BDSP_SoftFMM_InputSettings *pSettings
)
{
    BDBG_OBJECT_ASSERT(input, BDSP_SoftFMM_Input);
    if(input->setSettings_isr)
    {
        return input->setSettings_isr(input->pSoftFMMInput,pSettings);
    }
    else
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

/***************************************************************************
Summary:
Destroy a DSP Soft FMM Output
***************************************************************************/
void BDSP_SoftFMM_Output_Destroy(
    BDSP_SoftFMMOutputHandle output
)
{
    BDBG_OBJECT_ASSERT(output, BDSP_SoftFMM_Output);
    if(output->destroy)
    {
        output->destroy(output->pSoftFMMOutput);
    }
    else
    {
        BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

/***************************************************************************
Summary:
Get DSP Soft FMM Output Status
***************************************************************************/
BERR_Code BDSP_SoftFMM_Output_GetStatus(
    BDSP_SoftFMMOutputHandle output,
    BDSP_SoftFMM_OutputStatus *pstatus
)
{
    BDBG_OBJECT_ASSERT(output, BDSP_SoftFMM_Output);
    if(output->getStatus)
    {
        return output->getStatus(output->pSoftFMMOutput,pstatus);
    }
    else
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

/***************************************************************************
Summary:
Get DSP Soft FMM Output Settings
***************************************************************************/
BERR_Code BDSP_SoftFMM_Output_GetSettings(
    BDSP_SoftFMMOutputHandle output,
    BDSP_SoftFMM_OutputSettings *pSettings
)
{
    BDBG_OBJECT_ASSERT(output, BDSP_SoftFMM_Output);
    if(output->getSettings)
    {
        return output->getSettings(output->pSoftFMMOutput,pSettings);
    }
    else
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

/***************************************************************************
Summary:
Set DSP Soft FMM Output Settings
***************************************************************************/
BERR_Code BDSP_SoftFMM_Output_SetSettings(
    BDSP_SoftFMMOutputHandle output,
    BDSP_SoftFMM_OutputSettings *pSettings
)
{
    BDBG_OBJECT_ASSERT(output, BDSP_SoftFMM_Output);
    if(output->setSettings)
    {
        return output->setSettings(output->pSoftFMMOutput,pSettings);
    }
    else
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

/***************************************************************************
Summary:
Set DSP Soft FMM Output Settings ISR context
***************************************************************************/
BERR_Code BDSP_SoftFMM_Output_SetSettings_isr(
    BDSP_SoftFMMOutputHandle output,
    BDSP_SoftFMM_OutputSettings *pSettings
)
{
    BDBG_OBJECT_ASSERT(output, BDSP_SoftFMM_Output);
    if(output->setSettings_isr)
    {
        return output->setSettings_isr(output->pSoftFMMOutput,pSettings);
    }
    else
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

/***************************************************************************
Summary:
Set DSP Soft FMM Output Buffer Config
***************************************************************************/
BERR_Code BDSP_SoftFMM_Output_SetBufferConfig(
    BDSP_SoftFMMOutputHandle output,
    BDSP_SoftFMMBufferDescriptor  *pBufferDescriptor,
    BDSP_SoftFMM_Output_HWConfig  *pHWConfig
)
{
    BDBG_OBJECT_ASSERT(output, BDSP_SoftFMM_Output);
    if(output->setBufferConfig)
    {
        return output->setBufferConfig(output->pSoftFMMOutput,pBufferDescriptor,pHWConfig);
    }
    else
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}
