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

#ifndef BDSP_SOFT_FMM_H_
#define BDSP_SOFT_FMM_H_


#include "bchp.h"
#include "bint.h"
#include "breg_mem.h"
#include "bmma.h"
#include "btmr.h"
#include "bimg.h"
#include "bdsp_types.h"
#include "bdsp_raaga_fw.h"
#include "bdsp_raaga_fw_status.h"
#include "bdsp_raaga_fw_settings.h"
#include "bdsp.h"
#include "bdsp_priv.h"
#include "bdsp_soft_fmm_struct.h"


#define BDSP_MAX_SOFT_FMM_MIXER_INPUTS  6
#define BDSP_MAX_SOFT_FMM_MIXERS        2
#define BDSP_MAX_SOFT_FMM_OUTPUTS       BDSP_MAX_SOFT_FMM_MIXERS
#define BDSP_MAX_SOFT_FMM_INPUTS        6

/***************************************************************************
Summary:
Get Current Interrupt Handlers for SoftFMM Input
***************************************************************************/
void BDSP_SoftFMM_Input_GetInterruptHandlers_isr(
    BDSP_SoftFMMInputHandle softFMMInput,
    BDSP_SoftFMMInputInterruptHandlers *pHandlers   /* [out] */
    );

/***************************************************************************
Summary:
Set Current Interrupt Handlers for SoftFMM Input
***************************************************************************/
BERR_Code BDSP_SoftFMM_Input_SetInterruptHandlers_isr(
    BDSP_SoftFMMInputHandle softFMMInput,
    const BDSP_SoftFMMInputInterruptHandlers *pHandlers
    );

/***************************************************************************
Summary:
Get Current Interrupt Handlers for SoftFMM Output
***************************************************************************/
void BDSP_SoftFMM_Output_GetInterruptHandlers_isr(
    BDSP_SoftFMMOutputHandle softFMMOutput,
    BDSP_SoftFMMOutputInterruptHandlers *pHandlers   /* [out] */
    );

/***************************************************************************
Summary:
Set Current Interrupt Handlers for SoftFMM Output
***************************************************************************/
BERR_Code BDSP_SoftFMM_Output_SetInterruptHandlers_isr(
    BDSP_SoftFMMOutputHandle softFMMOutput,
    const BDSP_SoftFMMOutputInterruptHandlers *pHandlers
    );

/***************************************************************************
Summary:
Get Default SoftFMM Mixer Settings
***************************************************************************/
BERR_Code BDSP_SoftFMM_GetDefaultMixerSettings(
    BDSP_SoftFMMHandle softFMM,
    BDSP_SoftFMM_MixerSettings *pSettings     /* [out] */
    );

/***************************************************************************
Summary:
SoftFMM Close
***************************************************************************/
BERR_Code BDSP_SoftFMM_Close(
    BDSP_SoftFMMHandle softFMM
    );

/***************************************************************************
Summary:
Create a DSP SoftFMM Mixer
***************************************************************************/
BERR_Code BDSP_SoftFMM_MixerCreate(
    BDSP_SoftFMMHandle softFMM,
    const BDSP_SoftFMM_MixerSettings *pSettings,
    BDSP_SoftFMMMixerHandle *pSoftFMMMixer    /* [out] */
    );

/***************************************************************************
Summary:
Get Default SoftFMM Output Settings
***************************************************************************/
BERR_Code BDSP_SoftFMM_GetDefaultOutputSettings(
    BDSP_SoftFMMHandle softFMM,
    BDSP_SoftFMM_OutputSettings *pSettings     /* [out] */
    );


/***************************************************************************
Summary:
Create a DSP SoftFMM Output
***************************************************************************/
BERR_Code BDSP_SoftFMM_Output_Create(
    BDSP_SoftFMMHandle softFMM,
    const BDSP_SoftFMM_OutputSettings *pSettings,
    BDSP_SoftFMMOutputHandle *pSoftFMMOutput    /* [out] */
    );

/***************************************************************************
Summary:
Get Default SoftFMM Input Settings
***************************************************************************/
BERR_Code BDSP_SoftFMM_GetDefaultInputSettings(
    BDSP_SoftFMMHandle softFMM,
    BDSP_SoftFMM_InputSettings *pSettings     /* [out] */
    );


/***************************************************************************
Summary:
Create a DSP SoftFMM Input
***************************************************************************/
BERR_Code BDSP_SoftFMM_Input_Create(
    BDSP_SoftFMMHandle softFMM,
    const BDSP_SoftFMM_InputSettings *pSettings,
    BDSP_SoftFMMInputHandle *pSoftFMMInput    /* [out] */
    );

/***************************************************************************
Summary:
Destroy a DSP Soft FMM Mixer
***************************************************************************/
void BDSP_SoftFMM_Mixer_Destroy(
    BDSP_SoftFMMMixerHandle mixer
    );

/***************************************************************************
Summary:
Start a DSP Soft FMM Mixer
***************************************************************************/
BERR_Code BDSP_SoftFMM_Mixer_Start(
    BDSP_SoftFMMMixerHandle mixer
    );

/***************************************************************************
Summary:
Stop a DSP Soft FMM Mixer
***************************************************************************/
BERR_Code BDSP_SoftFMM_Mixer_Stop(
    BDSP_SoftFMMMixerHandle mixer
    );

/***************************************************************************
Summary:
Get DSP Soft FMM Mixer Status
***************************************************************************/
BERR_Code BDSP_SoftFMM_Mixer_GetStatus(
    BDSP_SoftFMMMixerHandle mixer,
    BDSP_SoftFMM_MixerStatus *pstatus
    );

/***************************************************************************
Summary:
Get DSP Soft FMM Mixer Settings
***************************************************************************/
BERR_Code BDSP_SoftFMM_Mixer_GetSettings(
    BDSP_SoftFMMMixerHandle mixer,
    BDSP_SoftFMM_MixerSettings *pSettings
    );

/***************************************************************************
Summary:
Set DSP Soft FMM Mixer Settings
***************************************************************************/
BERR_Code BDSP_SoftFMM_Mixer_SetSettings(
    BDSP_SoftFMMMixerHandle mixer,
    BDSP_SoftFMM_MixerSettings *pSettings
    );

/***************************************************************************
Summary:
Set DSP Soft FMM Mixer Settings ISR context
***************************************************************************/
BERR_Code BDSP_SoftFMM_Mixer_SetSettings_isr(
    BDSP_SoftFMMMixerHandle mixer,
    BDSP_SoftFMM_MixerSettings *pSettings
    );

/***************************************************************************
Summary:
Add an input to DSP Soft FMM Mixer
***************************************************************************/
BERR_Code BDSP_SoftFMM_Mixer_AddInput(
    BDSP_SoftFMMMixerHandle mixer,
    BDSP_SoftFMMInputHandle input
    );

/***************************************************************************
Summary:
Remove an input from DSP Soft FMM Mixer
***************************************************************************/
void BDSP_SoftFMM_Mixer_RemoveInput(
    BDSP_SoftFMMMixerHandle mixer,
    BDSP_SoftFMMInputHandle input
    );

/***************************************************************************
Summary:
Remove all inputs from DSP Soft FMM Mixer
***************************************************************************/
void BDSP_SoftFMM_Mixer_RemoveAllInputs(
    BDSP_SoftFMMMixerHandle mixer
    );

/***************************************************************************
Summary:
Add an output to DSP Soft FMM Mixer
***************************************************************************/
BERR_Code BDSP_SoftFMM_Mixer_AddOutput(
    BDSP_SoftFMMMixerHandle mixer,
    BDSP_SoftFMMOutputHandle output
    );

/***************************************************************************
Summary:
Remove an output from DSP Soft FMM Mixer
***************************************************************************/
void BDSP_SoftFMM_Mixer_RemoveOutput(
    BDSP_SoftFMMMixerHandle mixer,
    BDSP_SoftFMMOutputHandle output
    );

/***************************************************************************
Summary:
Remove all outputs from DSP Soft FMM Mixer
***************************************************************************/
void BDSP_SoftFMM_Mixer_RemoveAllOutputs(
    BDSP_SoftFMMMixerHandle mixer
    );

/***************************************************************************
Summary:
Destroy a DSP Soft FMM Input
***************************************************************************/
void BDSP_SoftFMM_Input_Destroy(
    BDSP_SoftFMMInputHandle input
    );

/***************************************************************************
Summary:
Get DSP Soft FMM Input Status
***************************************************************************/
BERR_Code BDSP_SoftFMM_Input_GetStatus(
    BDSP_SoftFMMInputHandle input,
    BDSP_SoftFMM_InputStatus *pstatus
    );

/***************************************************************************
Summary:
Get DSP Soft FMM Input Settings
***************************************************************************/
BERR_Code BDSP_SoftFMM_Input_GetSettings(
    BDSP_SoftFMMInputHandle input,
    BDSP_SoftFMM_InputSettings *pSettings
    );

/***************************************************************************
Summary:
Set DSP Soft FMM Input Settings
***************************************************************************/
BERR_Code BDSP_SoftFMM_Input_SetSettings(
    BDSP_SoftFMMInputHandle input,
    BDSP_SoftFMM_InputSettings *pSettings
    );

/***************************************************************************
Summary:
Set DSP Soft FMM Input Settings ISR context
***************************************************************************/
BERR_Code BDSP_SoftFMM_Input_SetSettings_isr(
    BDSP_SoftFMMInputHandle input,
    BDSP_SoftFMM_InputSettings *pSettings
    );


/***************************************************************************
Summary:
Destroy a DSP Soft FMM Output
***************************************************************************/
void BDSP_SoftFMM_Output_Destroy(
    BDSP_SoftFMMOutputHandle output
    );

/***************************************************************************
Summary:
Get DSP Soft FMM Output Status
***************************************************************************/
BERR_Code BDSP_SoftFMM_Output_GetStatus(
    BDSP_SoftFMMOutputHandle output,
    BDSP_SoftFMM_OutputStatus *pstatus
    );

/***************************************************************************
Summary:
Get DSP Soft FMM Output Settings
***************************************************************************/
BERR_Code BDSP_SoftFMM_Output_GetSettings(
    BDSP_SoftFMMOutputHandle output,
    BDSP_SoftFMM_OutputSettings *pSettings
    );

/***************************************************************************
Summary:
Set DSP Soft FMM Output Settings
***************************************************************************/
BERR_Code BDSP_SoftFMM_Output_SetSettings(
    BDSP_SoftFMMOutputHandle output,
    BDSP_SoftFMM_OutputSettings *pSettings
    );

/***************************************************************************
Summary:
Set DSP Soft FMM Output Settings ISR context
***************************************************************************/
BERR_Code BDSP_SoftFMM_Output_SetSettings_isr(
    BDSP_SoftFMMOutputHandle output,
    BDSP_SoftFMM_OutputSettings *pSettings
    );

/***************************************************************************
Summary:
Set DSP Soft FMM Output Buffer Config
***************************************************************************/
BERR_Code BDSP_SoftFMM_Output_SetBufferConfig(
    BDSP_SoftFMMOutputHandle output,
    BDSP_SoftFMMBufferDescriptor  *pBufferDescriptor,
    BDSP_SoftFMM_Output_HWConfig  *pHWConfig
    );

#endif /* BDSP_SOFT_FMM_H_ */
