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
*   API name: I2sOutput
*    Specific APIs related to I2S audio outputs.
*
* Revision History:
*
* $brcm_Log: $
*
***************************************************************************/

#include "nexus_audio_module.h"
#include "priv/nexus_core_audio.h"

BDBG_MODULE(nexus_i2s_output);

#if NEXUS_NUM_I2S_OUTPUTS

/***************************************************************************
Summary:
    Move and convert fields from a NEXUS_I2sOutputSettings struct to a
    BAPE_I2sOutputSettings struct.
See Also:
    NEXUS_I2sOutput_P_ConvertSettingsFromBape
 ***************************************************************************/
static NEXUS_Error NEXUS_I2sOutput_P_ConvertSettingsToBape(
    BAPE_I2sOutputSettings  *pBapeSettings,
    const NEXUS_I2sOutputSettings *pSettings
    )
{
    NEXUS_Error errCode;

    switch ( pSettings->lsbAtLRClock )
    {
    case false:
        pBapeSettings->justification = BAPE_I2sJustification_eMsbFirst;
        break;
    default:
    case true:
        pBapeSettings->justification = BAPE_I2sJustification_eLsbFirst;
        break;
    }

    switch ( pSettings->alignedToLRClock )
    {
    case false:
        pBapeSettings->dataAlignment = BAPE_I2sDataAlignment_eDelayed;
        break;
    default:
    case true:
        pBapeSettings->dataAlignment = BAPE_I2sDataAlignment_eAligned;
        break;
    }

    switch ( pSettings->lrClkPolarity )
    {
    case false:
        pBapeSettings->lrPolarity = BAPE_I2sLRClockPolarity_eLeftLow;
        break;
    default:
    case true:
        pBapeSettings->lrPolarity = BAPE_I2sLRClockPolarity_eLeftHigh;
        break;
    }

    switch ( pSettings->sclkRate )
    {
    case NEXUS_I2sOutputSclkRate_e64Fs:
        pBapeSettings->sclkRate = BAPE_SclkRate_e64Fs;
        break;
    case NEXUS_I2sOutputSclkRate_e128Fs:
        pBapeSettings->sclkRate = BAPE_SclkRate_e128Fs;
        break;
    default:
        BDBG_ERR(("NEXUS_I2sOutputSettings.sclkRate:%d is invalid", pSettings->sclkRate));
        errCode = BERR_TRACE(BERR_INVALID_PARAMETER);
        return errCode;
    }

    if ( NEXUS_I2sOutputMclkRate_eAuto !=  pSettings->mclkRate )
    {
        BDBG_ERR(("NEXUS_I2sOutputSettings.mclkRate:%d is invalid. Must be set to NEXUS_I2sOutputMclkRate_eAuto ", pSettings->mclkRate));
        errCode = BERR_TRACE(BERR_INVALID_PARAMETER);
        return errCode;
    }

    return NEXUS_SUCCESS;
}

/***************************************************************************
Summary:
    Move and convert fields from a NEXUS_I2sOutputSettings struct to a
    BAPE_I2sOutputSettings struct.
See Also:
    NEXUS_I2sOutput_P_ConvertSettingsToBape
 ***************************************************************************/
static NEXUS_Error NEXUS_I2sOutput_P_ConvertSettingsFromBape(
    NEXUS_I2sOutputSettings *pSettings,
    BAPE_I2sOutputSettings  *pBapeSettings
    )
{
    switch ( pBapeSettings->justification )
    {
    default:
    case BAPE_I2sJustification_eMsbFirst:
        pSettings->lsbAtLRClock = false;
        break;
    case BAPE_I2sJustification_eLsbFirst:
        pSettings->lsbAtLRClock = true;
        break;
    }

    switch ( pBapeSettings->dataAlignment )
    {
    default:
    case BAPE_I2sDataAlignment_eDelayed:
        pSettings->alignedToLRClock = false;
        break;
    case BAPE_I2sDataAlignment_eAligned:
        pSettings->alignedToLRClock = true;
        break;
    }

    switch ( pBapeSettings->lrPolarity )
    {
    default:
    case BAPE_I2sLRClockPolarity_eLeftLow:
        pSettings->lrClkPolarity = false;
        break;
    case BAPE_I2sLRClockPolarity_eLeftHigh:
        pSettings->lrClkPolarity = true;
        break;
    }

    switch ( pBapeSettings->sclkRate )
    {
    default:
    case BAPE_SclkRate_e64Fs:
        pSettings->sclkRate = NEXUS_I2sOutputSclkRate_e64Fs;
        break;
    case BAPE_SclkRate_e128Fs:
        pSettings->sclkRate = NEXUS_I2sOutputSclkRate_e128Fs;
        break;
    }

    /* BAPE doesn't expose this, just set it to Auto.  */
    pSettings->mclkRate = NEXUS_I2sOutputMclkRate_eAuto;

    return NEXUS_SUCCESS;
}

typedef struct NEXUS_I2sOutput
{
    NEXUS_OBJECT(NEXUS_I2sOutput);
    bool opened;
    BAPE_I2sOutputHandle handle;
    NEXUS_I2sOutputSettings settings;
    NEXUS_AudioOutputObject connector;
    char name[13];   /* I2S OUTPUT %d */
    
} NEXUS_I2sOutput;

#if NEXUS_NUM_I2S_OUTPUTS
static NEXUS_I2sOutput g_i2sOutputs[NEXUS_NUM_I2S_OUTPUTS];
#else
#ifndef NEXUS_NUM_I2S_OUTPUTS
#define NEXUS_NUM_I2S_OUTPUTS 0
#endif
static NEXUS_I2sOutput g_i2sOutputs[1];
#endif

static NEXUS_Error NEXUS_I2sOutput_P_SetChannelMode(void *pHandle, NEXUS_AudioChannelMode channelMode);

/***************************************************************************
Summary:
	Get default settings for an I2S output
See Also:

 ***************************************************************************/
void NEXUS_I2sOutput_GetDefaultSettings(
    NEXUS_I2sOutputSettings *pSettings   /* [out] default settings */
    )
{
    BAPE_I2sOutputSettings bapeI2sOutputSettings;
    NEXUS_Error errCode;

    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));

    BAPE_I2sOutput_GetDefaultSettings(&bapeI2sOutputSettings);

    errCode = NEXUS_I2sOutput_P_ConvertSettingsFromBape(pSettings, &bapeI2sOutputSettings);
    BDBG_ASSERT(errCode == NEXUS_SUCCESS);  /* We should never get bad stuff from BAPE */
}

/***************************************************************************
Summary:
	Open an I2S Output device
See Also:
    NEXUS_I2sOutput_Close
 ***************************************************************************/
NEXUS_I2sOutputHandle NEXUS_I2sOutput_Open(
    unsigned index,
    const NEXUS_I2sOutputSettings *pSettings
    )
{
    BAPE_I2sOutputSettings i2sSettings;
    BAPE_I2sOutputHandle i2sHandle;
    NEXUS_Error errCode;
    BAPE_OutputPort connector;

    if ( (int)index >= NEXUS_NUM_I2S_OUTPUTS )
    {
        BDBG_ERR(("I2sOutput %u not supported on this chipset", index));
        errCode = BERR_TRACE(BERR_INVALID_PARAMETER);
        return NULL;
    }

    if ( g_i2sOutputs[index].opened )
    {
        BDBG_ERR(("I2sOutput %u is already open", index));
        return NULL;
    }
    NEXUS_OBJECT_SET(NEXUS_I2sOutput, &g_i2sOutputs[index]);

    BAPE_I2sOutput_GetDefaultSettings(&i2sSettings);
    i2sSettings.allowCompressed = g_NEXUS_audioModuleData.settings.allowI2sCompressed;
    errCode = BAPE_I2sOutput_Open( NEXUS_AUDIO_DEVICE_HANDLE, index, &i2sSettings, &i2sHandle );
    if ( errCode )
    {
        (void)BERR_TRACE(errCode);
        return NULL;
    }

    g_i2sOutputs[index].opened = true;
    /* Initialize connector */
    g_i2sOutputs[index].handle = i2sHandle;
    BKNI_Snprintf(g_i2sOutputs[index].name, sizeof( g_i2sOutputs[index].name), "I2S OUTPUT %u", index);
    NEXUS_AUDIO_OUTPUT_INIT(&g_i2sOutputs[index].connector, NEXUS_AudioOutputType_eI2s, &g_i2sOutputs[index]);
    NEXUS_OBJECT_REGISTER(NEXUS_AudioOutput, &g_i2sOutputs[index].connector, Open);
    g_i2sOutputs[index].connector.pName = g_i2sOutputs[index].name;
    g_i2sOutputs[index].connector.setChannelMode = NEXUS_I2sOutput_P_SetChannelMode;
    BDBG_OBJECT_SET(&g_i2sOutputs[index], NEXUS_I2sOutput);
    NEXUS_I2sOutput_SetSettings(&g_i2sOutputs[index], pSettings);

    BAPE_I2sOutput_GetOutputPort(i2sHandle, &connector);
    g_i2sOutputs[index].connector.port = (size_t)connector;

    /* Success */
    return &g_i2sOutputs[index];
}

/***************************************************************************
Summary:
	Close an I2S Output device
See Also:
    NEXUS_I2sOutput_Open
 ***************************************************************************/
static void NEXUS_I2sOutput_P_Finalizer(
    NEXUS_I2sOutputHandle handle
    )
{
    NEXUS_OBJECT_ASSERT(NEXUS_I2sOutput, handle);

    NEXUS_AudioOutput_Shutdown(&handle->connector);
    BAPE_I2sOutput_Close(handle->handle);
    /* This is equivalent to BDBG_OBJECT_UNSET and also clears other flags */
    BKNI_Memset(handle, 0, sizeof(NEXUS_I2sOutput));
}

static void NEXUS_I2sOutput_P_Release(NEXUS_I2sOutputHandle handle)
{
    NEXUS_OBJECT_UNREGISTER(NEXUS_AudioOutput, &handle->connector, Close);
    return;
}

NEXUS_OBJECT_CLASS_MAKE_WITH_RELEASE(NEXUS_I2sOutput, NEXUS_I2sOutput_Close);


/***************************************************************************
Summary:
	Get settings for an I2S output
See Also:
	NEXUS_I2sOutput_SetSettings
 ***************************************************************************/
void NEXUS_I2sOutput_GetSettings(
    NEXUS_I2sOutputHandle handle,
    NEXUS_I2sOutputSettings *pSettings  /* [out] Settings */
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_I2sOutput);
    BDBG_ASSERT(NULL != pSettings);

    *pSettings = handle->settings;
}

/***************************************************************************
Summary:
	Set settings for an I2S output
See Also:
	NEXUS_I2sOutput_GetSettings
 ***************************************************************************/
NEXUS_Error NEXUS_I2sOutput_SetSettings(
    NEXUS_I2sOutputHandle handle,
    const NEXUS_I2sOutputSettings *pSettings    /* [in] Settings */
    )
{
    NEXUS_I2sOutputSettings settings;
    BAPE_I2sOutputSettings bapeI2sSettings;
    NEXUS_Error errCode;

    /* Sanity Check */
    BDBG_OBJECT_ASSERT(handle, NEXUS_I2sOutput);

    if ( NULL == pSettings )
    {
        NEXUS_I2sOutput_GetDefaultSettings(&settings);
        pSettings = &settings;
    }
    BAPE_I2sOutput_GetSettings(handle->handle, &bapeI2sSettings);

    errCode = NEXUS_I2sOutput_P_ConvertSettingsToBape(&bapeI2sSettings, pSettings);
    if (errCode)
    {
        return BERR_TRACE(errCode);
    }

    errCode = BAPE_I2sOutput_SetSettings(handle->handle, &bapeI2sSettings);
    if (errCode)
    {
        return BERR_TRACE(errCode);
    }

    handle->settings = *pSettings;

    /* success */
    return BERR_SUCCESS;
}

/***************************************************************************
Summary:
    Get the audio connector for an I2S output
See Also:

 ***************************************************************************/
NEXUS_AudioOutputHandle NEXUS_I2sOutput_GetConnector(
    NEXUS_I2sOutputHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_I2sOutput);
    return &handle->connector;
}

static NEXUS_Error NEXUS_I2sOutput_P_SetChannelMode(void *pHandle, NEXUS_AudioChannelMode channelMode)
{
    BERR_Code errCode;
    BAPE_I2sOutputSettings i2sOutputSettings;

    NEXUS_I2sOutputHandle handle = (NEXUS_I2sOutputHandle) pHandle;
    BDBG_OBJECT_ASSERT(handle, NEXUS_I2sOutput);
    
    BAPE_I2sOutput_GetSettings(handle->handle, &i2sOutputSettings);
    switch ( channelMode )
    {
    default:
    case NEXUS_AudioChannelMode_eStereo:
        i2sOutputSettings.stereoMode = BAPE_StereoMode_eLeftRight;
        break;
    case NEXUS_AudioChannelMode_eLeft:
        i2sOutputSettings.stereoMode = BAPE_StereoMode_eLeftLeft;
        break;
    case NEXUS_AudioChannelMode_eRight:
        i2sOutputSettings.stereoMode = BAPE_StereoMode_eRightRight;
        break;
    case NEXUS_AudioChannelMode_eSwapped:
        i2sOutputSettings.stereoMode = BAPE_StereoMode_eRightLeft;
        break;
    }

    errCode = BAPE_I2sOutput_SetSettings(handle->handle, &i2sOutputSettings);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    return BERR_SUCCESS;
}

#else

typedef struct NEXUS_I2sOutput
{
    NEXUS_OBJECT(NEXUS_I2sOutput);
} NEXUS_I2sOutput;

/***************************************************************************
Summary:
	Get default settings for an I2S output
See Also:

 ***************************************************************************/
void NEXUS_I2sOutput_GetDefaultSettings(
    NEXUS_I2sOutputSettings *pSettings   /* [out] default settings */
    )
{
    BSTD_UNUSED(pSettings);
    BDBG_WRN(("I2S Output not enabled on this chipset / platform"));
    BERR_TRACE(BERR_NOT_SUPPORTED);
}

/***************************************************************************
Summary:
	Open an I2S Output device
See Also:
    NEXUS_I2sOutput_Close
 ***************************************************************************/
NEXUS_I2sOutputHandle NEXUS_I2sOutput_Open(
    unsigned index,
    const NEXUS_I2sOutputSettings *pSettings
    )
{
    BSTD_UNUSED(index);
    BSTD_UNUSED(pSettings);
    BDBG_WRN(("I2S Output not enabled on this chipset / platform"));
    BERR_TRACE(BERR_NOT_SUPPORTED);
    return NULL;
}

/***************************************************************************
Summary:
	Close an I2S Output device
See Also:
    NEXUS_I2sOutput_Open
 ***************************************************************************/
static void NEXUS_I2sOutput_P_Finalizer(
    NEXUS_I2sOutputHandle handle
    )
{
    BSTD_UNUSED(handle);
    BDBG_WRN(("I2S Output not enabled on this chipset / platform"));
    BERR_TRACE(BERR_NOT_SUPPORTED);
}

NEXUS_OBJECT_CLASS_MAKE(NEXUS_I2sOutput, NEXUS_I2sOutput_Close);

/***************************************************************************
Summary:
	Get settings for an I2S output
See Also:
	NEXUS_I2sOutput_SetSettings
 ***************************************************************************/
void NEXUS_I2sOutput_GetSettings(
    NEXUS_I2sOutputHandle handle,
    NEXUS_I2sOutputSettings *pSettings  /* [out] Settings */
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);
    BDBG_WRN(("I2S Output not enabled on this chipset / platform"));
    BERR_TRACE(BERR_NOT_SUPPORTED);
}

/***************************************************************************
Summary:
	Set settings for an I2S output
See Also:
	NEXUS_I2sOutput_GetSettings
 ***************************************************************************/
NEXUS_Error NEXUS_I2sOutput_SetSettings(
    NEXUS_I2sOutputHandle handle,
    const NEXUS_I2sOutputSettings *pSettings    /* [in] Settings */
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);
    BDBG_WRN(("I2S Output not enabled on this chipset / platform"));
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

/***************************************************************************
Summary:
    Get the audio connector for an I2S output
See Also:

 ***************************************************************************/
NEXUS_AudioOutputHandle NEXUS_I2sOutput_GetConnector(
    NEXUS_I2sOutputHandle handle
    )
{
    BSTD_UNUSED(handle);
    BDBG_WRN(("I2S Output not enabled on this chipset / platform"));
    BERR_TRACE(BERR_NOT_SUPPORTED);
    return NULL;
}

#endif
