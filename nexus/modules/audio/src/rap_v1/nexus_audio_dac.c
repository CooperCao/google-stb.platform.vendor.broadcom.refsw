/***************************************************************************
*     (c)2004-2013 Broadcom Corporation
*  
*  This program is the proprietary software of Broadcom Corporation and/or its licensors,
*  and may only be used, duplicated, modified or distributed pursuant to the terms and
*  conditions of a separate, written license agreement executed between you and Broadcom
*  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
*  no license (express or implied), right to use, or waiver of any kind with respect to the
*  Software, and Broadcom expressly reserves all rights in and to the Software and all
*  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
*  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
*  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.  
*   
*  Except as expressly set forth in the Authorized License,
*   
*  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
*  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
*  and to use this information only in connection with your use of Broadcom integrated circuit products.
*   
*  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS" 
*  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR 
*  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO 
*  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES 
*  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, 
*  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION 
*  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF 
*  USE OR PERFORMANCE OF THE SOFTWARE.
*  
*  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS 
*  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR 
*  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR 
*  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF 
*  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT 
*  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE 
*  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF 
*  ANY LIMITED REMEDY.
*
* $brcm_Workfile: $
* $brcm_Revision: $
* $brcm_Date: $
*
* API Description:
*   API name: AudioDac
*    Specific APIs related to audio DAC outputs.
*
* Revision History:
*
* $brcm_Log: $
*
***************************************************************************/
#include "nexus_audio_module.h"

BDBG_MODULE(nexus_audio_dac);

typedef struct NEXUS_AudioDac
{
    NEXUS_OBJECT(NEXUS_AudioDac);
    bool opened;
    NEXUS_AudioDacSettings settings;
    NEXUS_AudioOutputObject connector;
} NEXUS_AudioDac;

static NEXUS_AudioDac g_dacs[NEXUS_NUM_AUDIO_DACS];

/* Convert an integer to a DAC enum tag */
#define NEXUS_DAC_OUTPUT_PORT(i) ((i==0)?BRAP_OutputPort_eDac0:BRAP_OutputPort_eDac1)

/***************************************************************************
Summary:
	Get default settings for an audio DAC
See Also:

 ***************************************************************************/
void NEXUS_AudioDac_GetDefaultSettings(
    NEXUS_AudioDacSettings *pSettings   /* [out] default settings */
    )
{
    BERR_Code errCode;
    BRAP_OutputSettings outputSettings;

    /* Use DAC 0 */
    errCode = BRAP_GetOutputDefaultConfig(
        g_NEXUS_audioModuleData.hRap,
        BRAP_OutputPort_eDac0,
        &outputSettings);

    if ( errCode )
    {
        errCode=BERR_TRACE(errCode);
    }

    switch ( outputSettings.uOutputPortSettings.sDacSettings.eMuteType )
    {
    case BRAP_OP_DacMuteType_ConstantLow:
        pSettings->muteType = NEXUS_AudioDacMuteType_eConstantLow;
        break;
    case BRAP_OP_DacMuteType_ConstantHigh:
        pSettings->muteType = NEXUS_AudioDacMuteType_eConstantHigh;
        break;
    case BRAP_OP_DacMuteType_SquareWaveOpp:
        pSettings->muteType = NEXUS_AudioDacMuteType_eSquareWaveOpp;
        break;
    case BRAP_OP_DacMuteType_SquareWaveSame:
        pSettings->muteType = NEXUS_AudioDacMuteType_eSquareWaveSame;
        break;
    default:
        BDBG_ERR(("Invalid DAC mute type"));
        BDBG_ASSERT(0);
        return;
    }
}

/***************************************************************************
Summary:
	Init the DAC interface
See Also:

 ***************************************************************************/
NEXUS_Error NEXUS_AudioDac_P_Init(void)
{
    int i;
    BERR_Code errCode;

    BDBG_MSG(("Initializing %d DACs", NEXUS_NUM_AUDIO_DACS));

    for ( i = 0; i < NEXUS_NUM_AUDIO_DACS; i++ )
    {
        BRAP_OutputSettings settings;
        BRAP_OutputPort port = NEXUS_DAC_OUTPUT_PORT(i);

        BDBG_MSG(("Initializing DAC %d (RAP port %d)", i, port));

        /* setup default config */
        errCode = BRAP_GetOutputDefaultConfig(
            g_NEXUS_audioModuleData.hRap,
            port,
            &settings);

        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }

        /* 
            This "opens" the output port in raptor.  
            It must be done for each output you wish to use 
        */

        errCode = BRAP_SetOutputConfig(
            g_NEXUS_audioModuleData.hRap,
            &settings);

        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }

        /* Populate settings with defaults */
        NEXUS_AudioDac_GetDefaultSettings(&g_dacs[i].settings);
    }

    BDBG_MSG(("Successfully initialized DACs"));
    return BERR_SUCCESS;
}

/***************************************************************************
Summary:
	Open an audio DAC
See Also:
    NEXUS_AudioDac_Close
 ***************************************************************************/
NEXUS_AudioDacHandle NEXUS_AudioDac_Open(
    unsigned index,
    const NEXUS_AudioDacSettings *pSettings     /* Pass NULL for default settings */
    )
{
    NEXUS_AudioDacHandle handle;
    
    /* Sanity check */
    if ( index >= NEXUS_NUM_AUDIO_DACS )
    {
        BDBG_ERR(("Dac Index %u out of range", index));
        return NULL;
    }

    handle = &g_dacs[index];
    if ( handle->opened )
    {
        BDBG_ERR(("Dac %d already open", index));
        return NULL;
    }

    handle->opened = true;
    NEXUS_AUDIO_OUTPUT_INIT(&handle->connector, NEXUS_AudioOutputType_eDac, handle);
    NEXUS_OBJECT_REGISTER(NEXUS_AudioOutput, &handle->connector, Open);
    handle->connector.port = NEXUS_DAC_OUTPUT_PORT(index);
    NEXUS_OBJECT_SET(NEXUS_AudioDac, handle);
    NEXUS_AudioDac_SetSettings(handle, pSettings);

    /* Success */
    return handle;
}


/***************************************************************************
Summary:
    Close an audio DAC
Description:
    Input to the DAC must be removed prior to closing.
See Also:
    NEXUS_AudioDac_Open
    NEXUS_AudioOutput_RemoveInput
    NEXUS_AudioOutput_RemoveAllInputs
 ***************************************************************************/
static void NEXUS_AudioDac_P_Finalizer(
    NEXUS_AudioDacHandle handle
    )
{
    NEXUS_OBJECT_ASSERT(NEXUS_AudioDac, handle);
    BDBG_ASSERT(handle->opened);

    NEXUS_AudioOutput_Shutdown(&handle->connector);
    NEXUS_OBJECT_CLEAR(NEXUS_AudioDac, handle);
}

static void NEXUS_AudioDac_P_Release( NEXUS_AudioDacHandle handle)
{
    NEXUS_OBJECT_UNREGISTER(NEXUS_AudioOutput, &handle->connector, Close);
    return;
}

NEXUS_OBJECT_CLASS_MAKE_WITH_RELEASE(NEXUS_AudioDac, NEXUS_AudioDac_Close);

/***************************************************************************
Summary:
    Get Settings for an audio DAC
See Also:
    NEXUS_AudioDac_SetSettings
 ***************************************************************************/
void NEXUS_AudioDac_GetSettings(
    NEXUS_AudioDacHandle handle,
    NEXUS_AudioDacSettings *pSettings    /* [out] Settings */
    )
{
    /* Sanity Check */
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioDac);
    BDBG_ASSERT(NULL != pSettings);

    /* Copy settings */
    *pSettings = handle->settings;
}

/***************************************************************************
Summary:
	Set Settings for an audio DAC
See Also:
	NEXUS_AudioDac_GetSettings
 ***************************************************************************/
NEXUS_Error NEXUS_AudioDac_SetSettings(
    NEXUS_AudioDacHandle handle,
    const NEXUS_AudioDacSettings *pSettings    /* [in] Settings */
    )
{
    BRAP_OutputSettings outputSettings;
    NEXUS_AudioDacSettings settings;
    NEXUS_Error errCode;

    /* Sanity Check */
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioDac);

    if ( NULL == pSettings )
    {
        NEXUS_AudioDac_GetDefaultSettings(&settings);
        pSettings = &settings;
    }

    errCode = BRAP_GetOutputConfig(g_NEXUS_audioModuleData.hRap,
                                   handle->connector.port,
                                   &outputSettings);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    /* Adjust RAP parameters to match mute type */
    switch ( pSettings->muteType )
    {
    case NEXUS_AudioDacMuteType_eConstantLow:
        outputSettings.uOutputPortSettings.sDacSettings.eMuteType = BRAP_OP_DacMuteType_ConstantLow;
        break;
    case NEXUS_AudioDacMuteType_eConstantHigh:
        outputSettings.uOutputPortSettings.sDacSettings.eMuteType = BRAP_OP_DacMuteType_ConstantHigh;
        break;
    case NEXUS_AudioDacMuteType_eSquareWaveOpp:
        outputSettings.uOutputPortSettings.sDacSettings.eMuteType = BRAP_OP_DacMuteType_SquareWaveOpp;
        break;
    case NEXUS_AudioDacMuteType_eSquareWaveSame:
        outputSettings.uOutputPortSettings.sDacSettings.eMuteType = BRAP_OP_DacMuteType_SquareWaveSame;
        break;
    default:
        BDBG_ERR(("Invalid DAC mute type"));
        BDBG_ASSERT(0);
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    errCode = BRAP_GetOutputConfig(g_NEXUS_audioModuleData.hRap,
                                   handle->connector.port,
                                   &outputSettings);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    handle->settings = *pSettings;

    return BERR_SUCCESS;
}

/***************************************************************************
Summary:
	Get the audio connector for an audio DAC
See Also:
    	
 ***************************************************************************/
NEXUS_AudioOutputHandle NEXUS_AudioDac_GetConnector(
    NEXUS_AudioDacHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioDac);
    return &handle->connector;
}

/***************************************************************************
Summary:
	Apply output-specific start-time settings to the raptor structure
Description:
    This routine will be called when the channels up-stream are started.
    It requires knowledge of raptor structures, but no actual raptor function 
    calls.  This is required for things like LR swap because some part of the 
    logic is in a downmix mode and some is in the DAC settings.  It's also
    required for outputs that are not part of the audio module (e.g. HDMI)
 ***************************************************************************/
NEXUS_Error NEXUS_AudioDac_P_AdjustRapSettings(
    NEXUS_AudioDacHandle handle,
    BRAP_OutputSettings *pSettings    /* [in/out] */
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioDac);
    BDBG_ASSERT(NULL != pSettings);
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);

    return BERR_SUCCESS;
}

