/***************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *
 * Module Description: Audio Decoder Interface
 *
 ***************************************************************************/

#include "bape.h"
#include "bape_priv.h"

BDBG_MODULE(bape_audioreturnchannel);

#if BAPE_CHIP_MAX_AUDIO_RETURN_CHANNELS > 0   /* If no audio return channel outputs, then skip all of this and just put in stub funcs at bottom of file. */

BDBG_OBJECT_ID(BAPE_AudioReturnChannel);

typedef struct BAPE_AudioReturnChannel
{
    BDBG_OBJECT(BAPE_AudioReturnChannel)
    unsigned index;
    BAPE_Handle deviceHandle;
    BAPE_AudioReturnChannelSettings settings;
    char name[20];   /* Audio Rtn Chan %d */
} BAPE_AudioReturnChannel;


/* Static function prototypes */
static BERR_Code BAPE_AudioReturnChannel_P_ApplySettings(
    BAPE_AudioReturnChannelHandle handle,
    const BAPE_AudioReturnChannelSettings *pSettings,
    bool force
    );

/* Output port callbacks */
/*     None. */


/***************************************************************************
        Public APIs: From bape_output.h
***************************************************************************/
void BAPE_AudioReturnChannel_GetDefaultSettings(
    BAPE_AudioReturnChannelSettings *pSettings
    )
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
}

/**************************************************************************/

BERR_Code BAPE_AudioReturnChannel_Open(
    BAPE_Handle deviceHandle,
    unsigned index,
    const BAPE_AudioReturnChannelSettings *pSettings,
    BAPE_AudioReturnChannelHandle *pHandle             /* [out] */
    )
{
    BAPE_AudioReturnChannelSettings defaultSettings;
    BAPE_AudioReturnChannelHandle handle;
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(deviceHandle, BAPE_Device);
    BDBG_ASSERT(NULL != pHandle);

    BDBG_MSG(("%s: Opening AudioReturnChannel Output: %u", __FUNCTION__, index));

    *pHandle = NULL;    /* Set up to return null handle in case of error. */

    if ( index >= BAPE_CHIP_MAX_AUDIO_RETURN_CHANNELS )
    {
        BDBG_ERR(("Request to open Audio Return Channel %d but chip only has %u", index, BAPE_CHIP_MAX_AUDIO_RETURN_CHANNELS));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if ( deviceHandle->audioReturnChannels[index] )
    {
        BDBG_ERR(("Audio Rtn Chan %u already open", index));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    handle = BKNI_Malloc(sizeof(BAPE_AudioReturnChannel));
    if ( NULL == handle )
    {
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }

    BKNI_Memset(handle, 0, sizeof(BAPE_AudioReturnChannel));
    BDBG_OBJECT_SET(handle, BAPE_AudioReturnChannel);
    handle->deviceHandle = deviceHandle;
    handle->index = index;

    BKNI_Snprintf(handle->name, sizeof(handle->name), "Audio Rtn Chan %u", index);

    if ( NULL == pSettings )
    {
        BAPE_AudioReturnChannel_GetDefaultSettings(&defaultSettings);
        pSettings = &defaultSettings;
    }

    errCode = BAPE_AudioReturnChannel_SetSettings(handle, pSettings);
    if ( errCode )
    {
        BAPE_AudioReturnChannel_Close(handle);
        return BERR_TRACE(errCode);
    }

    *pHandle = handle;
    handle->deviceHandle->audioReturnChannels[index] = handle;
    return BERR_SUCCESS;
}

/**************************************************************************/

void BAPE_AudioReturnChannel_Close(
    BAPE_AudioReturnChannelHandle handle
    )
{
    BAPE_AudioReturnChannelSettings defaults;

    BDBG_OBJECT_ASSERT(handle, BAPE_AudioReturnChannel);

    /* Set ARC settings back to default values. */
    BAPE_AudioReturnChannel_GetDefaultSettings(&defaults);
    BAPE_AudioReturnChannel_SetSettings(handle, &defaults);

    handle->deviceHandle->audioReturnChannels[handle->index] = NULL;
    BDBG_OBJECT_DESTROY(handle, BAPE_AudioReturnChannel);
    BKNI_Free(handle);
}

/**************************************************************************/

void BAPE_AudioReturnChannel_GetSettings(
    BAPE_AudioReturnChannelHandle handle,
    BAPE_AudioReturnChannelSettings *pSettings     /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_AudioReturnChannel);
    BDBG_ASSERT(NULL != pSettings);
    *pSettings = handle->settings;
}

/**************************************************************************/

BERR_Code BAPE_AudioReturnChannel_SetSettings(
    BAPE_AudioReturnChannelHandle handle,
    const BAPE_AudioReturnChannelSettings *pSettings
    )
{
    BERR_Code   errCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(handle, BAPE_AudioReturnChannel);
    BDBG_ASSERT(NULL != pSettings);

    errCode = BAPE_AudioReturnChannel_P_ApplySettings(handle, pSettings, false); /* false => don't force (only update HW for changes) */
    return errCode;
}

/***************************************************************************
        BAPE Internal APIs: From bape_fmm_priv.h
***************************************************************************/

BERR_Code BAPE_AudioReturnChannel_P_ResumeFromStandby(BAPE_Handle bapeHandle)
{
    BERR_Code   errCode = BERR_SUCCESS;
    unsigned    index;

    BDBG_OBJECT_ASSERT(bapeHandle, BAPE_Device);

    /* For each opened AudioReturnChannel Output, call the functions necessary to
     * restore the hardware to it's appropriate state.
     */
    for ( index=0 ; index<BAPE_CHIP_MAX_AUDIO_RETURN_CHANNELS ; index++ )
    {
        if ( bapeHandle->audioReturnChannels[index] )       /* If this AudioReturnChannel is open... */
        {
            BAPE_AudioReturnChannelHandle hAudioReturnChannel = bapeHandle->audioReturnChannels[index];

            /* Put the HW into the generic open state. */
                /* Doesn't apply here.  */
            
            /* Now apply changes for the settings struct. */
            errCode = BAPE_AudioReturnChannel_P_ApplySettings(hAudioReturnChannel, &hAudioReturnChannel->settings, true);   /* true => force update of HW */
            if ( errCode ) return BERR_TRACE(errCode);

            /* Now restore the dynamic stuff from the values saved in the device struct. */
                /* Nothing to do here for AudioReturnChannel. */
        }
    }
    return errCode;
}

/***************************************************************************
        Private callbacks: Protyped above
***************************************************************************/
    /* No callbacks for AudioReturnChannel. */


/***************************************************************************
        Private functions: Protyped above
***************************************************************************/

static BERR_Code BAPE_AudioReturnChannel_P_ApplySettings(
    BAPE_AudioReturnChannelHandle handle,
    const BAPE_AudioReturnChannelSettings *pSettings,
    bool force
    )
{
    BSTD_UNUSED(force);  /* Not needed because hardware is always updated (even if nothing changes). */

    BDBG_OBJECT_ASSERT(handle, BAPE_AudioReturnChannel);
    BDBG_ASSERT(NULL != pSettings);

    /* See if they want to change the  master. */
    if (pSettings->master != handle->settings.master)
    {
        /* If they've passed in a new master, make sure it's valid. */
        if (pSettings->master)
        {
            if ( pSettings->master->type != BAPE_OutputPortType_eSpdifOutput ) {
                BDBG_ERR(("Master: %s is not a SpdifOutput, giving up.", pSettings->master->pName ));
                return (BERR_INVALID_PARAMETER);
            }
            BDBG_ASSERT(  pSettings->master->index < BAPE_CHIP_MAX_SPDIF_OUTPUTS );
        }

        /* Print the old master if there was one. */
        if (handle->settings.master)
        {
            BDBG_OBJECT_ASSERT(handle->settings.master, BAPE_OutputPort);
            BDBG_MSG(("Removing master: %s from %s", handle->settings.master->pName, handle->name ));
        }

        /* Print the new master if there is one. */
        if (pSettings->master)
        {
            BDBG_OBJECT_ASSERT(pSettings->master, BAPE_OutputPort);
            BDBG_MSG(("Assigning master: %s to %s", pSettings->master->pName, handle->name ));
        }

        /* Now update the master in the device handle. */
        handle->settings.master = pSettings->master;


        /* Currently, we only support one Audio Return Channel maximum, and if there is an
         * audio return channel, there can only be one Spdif output.  For anything beyond
         * this, the registers haven't been defined yet. 
         *  
         * So anyway, make the check here.
         * */ 

        /* TODO: Allow any Spdif to master any Audio Rtn Chan */
        /* TODO: Add support for multiple Audio Rtn Chans   */

        #if BAPE_CHIP_MAX_SPDIF_OUTPUTS > 1  
            #error "I don't know how to handle Audio Rtn Chan with multiple Spdif outputs"
        #endif
        #if BAPE_CHIP_MAX_AUDIO_RETURN_CHANNELS > 1
            #error "I don't know how to handle multiple Audio Rtn Channels"
        #endif

    }

    /* See if they want to change the mute setting. */
    if (pSettings->muted != handle->settings.muted)
    {
        /* Now update the master in the device handle. */
        handle->settings.muted = pSettings->muted;
    }

    /* Now do the register updates.  If we have a master and if
     * we're unmuted, then enable the master's output to go to
     * the audio return channel. 
     */ 
    if ( handle->settings.master && ! handle->settings.muted  )
    {
            BAPE_Reg_P_UpdateEnum(handle->deviceHandle, BCHP_AUD_MISC_SEROUT_SEL, AUD_MISC_SEROUT_SEL, HDMI_RX_ARC_ENABLE, ENABLE);
    }
    else
    {
            BAPE_Reg_P_UpdateEnum(handle->deviceHandle, BCHP_AUD_MISC_SEROUT_SEL, AUD_MISC_SEROUT_SEL, HDMI_RX_ARC_ENABLE, DISABLE);
    }

    return BERR_SUCCESS;
}

/***************************************************************************
    Define stub functions for when there are no Audio Return Channel audio outputs. 
***************************************************************************/
#else /* BAPE_CHIP_MAX_AUDIO_RETURN_CHANNELS <= 0 */
    /* No Audio Return Channel outputs, just use stubbed out functions. */

void BAPE_AudioReturnChannel_GetDefaultSettings(
    BAPE_AudioReturnChannelSettings *pSettings
    )
{
    BSTD_UNUSED(pSettings);
}

BERR_Code BAPE_AudioReturnChannel_Open(
    BAPE_Handle deviceHandle,
    unsigned index,
    const BAPE_AudioReturnChannelSettings *pSettings,
    BAPE_AudioReturnChannelHandle *pHandle             /* [out] */
    )
{
    BSTD_UNUSED(deviceHandle);
    BSTD_UNUSED(index);
    BSTD_UNUSED(pSettings);

    *pHandle = NULL;

    return BERR_NOT_SUPPORTED;
}

void BAPE_AudioReturnChannel_Close(
    BAPE_AudioReturnChannelHandle handle
    )
{
    BSTD_UNUSED(handle);
}

void BAPE_AudioReturnChannel_GetSettings(
    BAPE_AudioReturnChannelHandle handle,
    BAPE_AudioReturnChannelSettings *pSettings     /* [out] */
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);
}

BERR_Code BAPE_AudioReturnChannel_SetSettings(
    BAPE_AudioReturnChannelHandle handle,
    const BAPE_AudioReturnChannelSettings *pSettings
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);

    return BERR_NOT_SUPPORTED;
}
BERR_Code BAPE_AudioReturnChannel_P_ResumeFromStandby(BAPE_Handle bapeHandle)
{
    BSTD_UNUSED(bapeHandle);
    return BERR_SUCCESS;
}

#endif /* BAPE_CHIP_MAX_AUDIO_RETURN_CHANNELS */
