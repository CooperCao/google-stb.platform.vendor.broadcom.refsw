/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
 * Module Description:
 * 
 *****************************************************************************/

#include "nexus_audio_module.h"

BDBG_MODULE(nexus_ac3_encode);

struct NEXUS_Ac3Encode {
    NEXUS_OBJECT(NEXUS_Ac3Encode);
    NEXUS_AudioEncoderHandle audioEncoder;
};

/* This API is provided only for backward compatibility.  New codecs should only be added to NEXUS_AudioEncoder */

/***************************************************************************
Summary:
    Get default settings for an AC3 Encode stage
***************************************************************************/
void NEXUS_Ac3Encode_GetDefaultSettings(
    NEXUS_Ac3EncodeSettings *pSettings   /* [out] default settings */
    )
{
    /* No way to get codec specific defaults from PI, but the default is always to have spdif header generation enabled */
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->spdifHeaderEnabled = true;
}

/***************************************************************************
Summary:
    Open an AC3 Encode stage
***************************************************************************/
NEXUS_Ac3EncodeHandle NEXUS_Ac3Encode_Open( /* attr{destructor=NEXUS_Ac3Encode_Close}  */
    const NEXUS_Ac3EncodeSettings *pSettings     /* Pass NULL for default settings */
    )
{
    NEXUS_AudioEncoderSettings encoderSettings;
    NEXUS_AudioEncoderCodecSettings codecSettings;
    NEXUS_AudioEncoderHandle handle;
    NEXUS_Error errCode;

    NEXUS_AudioEncoder_GetDefaultSettings(&encoderSettings);
    encoderSettings.codec = NEXUS_AudioCodec_eAc3;
    handle = NEXUS_AudioEncoder_Open(&encoderSettings);
    if ( NULL == handle )
    {
        (void)BERR_TRACE(BERR_NOT_SUPPORTED);
        return NULL;
    }

    if ( NULL != pSettings )
    {
        NEXUS_AudioEncoder_GetCodecSettings(handle, NEXUS_AudioCodec_eAc3, &codecSettings);
        BKNI_Memcpy(&codecSettings.codecSettings.ac3, pSettings, sizeof(NEXUS_Ac3EncodeSettings));
        errCode = NEXUS_AudioEncoder_SetCodecSettings(handle, &codecSettings);
        if ( errCode )
        {
            (void)BERR_TRACE(errCode);
            NEXUS_AudioEncoder_Close(handle);
            return NULL;
        }
    }

    {
        NEXUS_Ac3EncodeHandle ac3Encode;
        ac3Encode = BKNI_Malloc(sizeof(*ac3Encode));
        if(!ac3Encode) {
            NEXUS_AudioEncoder_Close(handle);
            return NULL;
        }
        NEXUS_OBJECT_INIT(NEXUS_Ac3Encode, ac3Encode);
        ac3Encode->audioEncoder = handle;
        return ac3Encode;
    }
}

/***************************************************************************
Summary:
    Close an AC3 Encode stage
    
Description:
    Input to the stage must be removed prior to closing.
***************************************************************************/
static void NEXUS_Ac3Encode_P_Finalizer(
    NEXUS_Ac3EncodeHandle handle
    )
{
    NEXUS_OBJECT_ASSERT(NEXUS_Ac3Encode, handle);
    NEXUS_AudioEncoder_Close(handle->audioEncoder);
    NEXUS_OBJECT_DESTROY(NEXUS_Ac3Encode, handle);
    BKNI_Free(handle);
    return;
}

NEXUS_OBJECT_CLASS_MAKE(NEXUS_Ac3Encode, NEXUS_Ac3Encode_Close);

/***************************************************************************
Summary:
    Get Settings for an AC3 Encode stage
***************************************************************************/
void NEXUS_Ac3Encode_GetSettings(
    NEXUS_Ac3EncodeHandle handle,
    NEXUS_Ac3EncodeSettings *pSettings    /* [out] Settings */
    )
{
    NEXUS_AudioEncoderCodecSettings codecSettings;

    NEXUS_OBJECT_ASSERT(NEXUS_Ac3Encode, handle);

    NEXUS_AudioEncoder_GetCodecSettings(handle->audioEncoder, NEXUS_AudioCodec_eAc3, &codecSettings);
    BKNI_Memcpy(pSettings, &codecSettings.codecSettings.ac3, sizeof(NEXUS_Ac3EncodeSettings));
}

/***************************************************************************
Summary:
    Set Settings for an AC3 Encode stage
***************************************************************************/
NEXUS_Error NEXUS_Ac3Encode_SetSettings(
    NEXUS_Ac3EncodeHandle handle,
    const NEXUS_Ac3EncodeSettings *pSettings
    )
{
    NEXUS_AudioEncoderCodecSettings codecSettings;
    NEXUS_Error errCode;

    NEXUS_OBJECT_ASSERT(NEXUS_Ac3Encode, handle);

    NEXUS_AudioEncoder_GetCodecSettings(handle->audioEncoder, NEXUS_AudioCodec_eAc3, &codecSettings);
    BKNI_Memcpy(&codecSettings.codecSettings.ac3, pSettings, sizeof(NEXUS_Ac3EncodeSettings));
    errCode = NEXUS_AudioEncoder_SetCodecSettings(handle->audioEncoder, &codecSettings);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    return BERR_SUCCESS;
}

/***************************************************************************
Summary:
    Get the audio connector for an AC3 Encode stage
***************************************************************************/
NEXUS_AudioInputHandle NEXUS_Ac3Encode_GetConnector( /* attr{shutdown=NEXUS_AudioInput_Shutdown} */
    NEXUS_Ac3EncodeHandle handle
    )
{
    NEXUS_OBJECT_ASSERT(NEXUS_Ac3Encode, handle);
    return NEXUS_AudioEncoder_GetConnector(handle->audioEncoder);
}

/***************************************************************************
Summary:
Add an input to this processing stage
***************************************************************************/
NEXUS_Error NEXUS_Ac3Encode_AddInput(
    NEXUS_Ac3EncodeHandle handle,
    NEXUS_AudioInputHandle input
    )
{
    NEXUS_Error errCode;

    NEXUS_OBJECT_ASSERT(NEXUS_Ac3Encode, handle);
    errCode = NEXUS_AudioEncoder_AddInput(handle->audioEncoder, input);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    return BERR_SUCCESS;
}

/***************************************************************************
Summary:
Remove an input from this processing stage
***************************************************************************/
NEXUS_Error NEXUS_Ac3Encode_RemoveInput(
    NEXUS_Ac3EncodeHandle handle,
    NEXUS_AudioInputHandle input
    )
{
    NEXUS_Error errCode;

    NEXUS_OBJECT_ASSERT(NEXUS_Ac3Encode, handle);
    errCode = NEXUS_AudioEncoder_RemoveInput(handle->audioEncoder, input);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    return BERR_SUCCESS;
}


/***************************************************************************
Summary:
Remove all inputs from this processing stage
***************************************************************************/
NEXUS_Error NEXUS_Ac3Encode_RemoveAllInputs(
    NEXUS_Ac3EncodeHandle handle
    )
{
    NEXUS_Error errCode;

    NEXUS_OBJECT_ASSERT(NEXUS_Ac3Encode, handle);
    errCode = NEXUS_AudioEncoder_RemoveAllInputs(handle->audioEncoder);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    return BERR_SUCCESS;
}
