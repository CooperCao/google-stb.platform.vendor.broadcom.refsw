/***************************************************************************
*  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
*
*  This program is the proprietary software of Broadcom and/or its licensors,
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
* API Description:
*   API name: DtsEncode
*    Specific APIs related to DTS Audio Encoding
*
***************************************************************************/

/* Not supported on 7400/7401 */

#include "nexus_audio_module.h"

BDBG_MODULE(nexus_audio_encoder);

typedef struct NEXUS_AudioEncoder {
    NEXUS_OBJECT(NEXUS_AudioEncoder);
} NEXUS_AudioEncoder;

/***************************************************************************
Summary:
    Get default settings for an Audio Encoder stage
***************************************************************************/
void NEXUS_AudioEncoder_GetDefaultSettings(
    NEXUS_AudioEncoderSettings *pSettings   /* [out] default settings */
    )
{
    BSTD_UNUSED(pSettings);
}

/***************************************************************************
Summary:
    Open an Audio Encoder stage
***************************************************************************/
NEXUS_AudioEncoderHandle NEXUS_AudioEncoder_Open( /* attr{destructor=NEXUS_AudioEncoder_Close}  */
    const NEXUS_AudioEncoderSettings *pSettings     /* Pass NULL for default settings */
    )
{
    NEXUS_Error errCode;

    BSTD_UNUSED(pSettings);
    errCode = BERR_TRACE(BERR_NOT_SUPPORTED);
    return NULL;
}

/***************************************************************************
Summary:
    Close an DTS Encode stage
    
Description:
    Input to the stage must be removed prior to closing.
***************************************************************************/
static void NEXUS_AudioEncoder_P_Finalizer(
    NEXUS_AudioEncoderHandle handle
    )
{
    BSTD_UNUSED(handle);
}
NEXUS_OBJECT_CLASS_MAKE(NEXUS_AudioEncoder, NEXUS_AudioEncoder_Close);

/***************************************************************************
Summary:
    Get Settings for an DTS Encode stage
***************************************************************************/
void NEXUS_AudioEncoder_GetSettings(
    NEXUS_AudioEncoderHandle handle,
    NEXUS_AudioEncoderSettings *pSettings    /* [out] Settings */
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);
}


/***************************************************************************
Summary:
    Set Settings for an DTS Encode stage
***************************************************************************/
NEXUS_Error NEXUS_AudioEncoder_SetSettings(
    NEXUS_AudioEncoderHandle handle,
    const NEXUS_AudioEncoderSettings *pSettings
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

void NEXUS_AudioEncoder_GetDefaultCodecSettings(
    NEXUS_AudioCodec codec, /* the codec for which you are retrieving settings. */
    NEXUS_AudioEncoderCodecSettings *pSettings    /* [out] Settings */
    )
{
    BSTD_UNUSED(codec);
    BSTD_UNUSED(pSettings);
}

/***************************************************************************
Summary:
    Get Codec-Specific Settings for an Audio Encoder stage
***************************************************************************/
void NEXUS_AudioEncoder_GetCodecSettings(
    NEXUS_AudioEncoderHandle handle,
    NEXUS_AudioCodec codec, /* the codec for which you are retrieving settings. */
    NEXUS_AudioEncoderCodecSettings *pSettings    /* [out] Settings */
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(codec);
    BSTD_UNUSED(pSettings);
}


/***************************************************************************
Summary:
    Set Codec-Specific Settings for an Audio Encoder stage
***************************************************************************/
NEXUS_Error NEXUS_AudioEncoder_SetCodecSettings(
    NEXUS_AudioEncoderHandle handle,
    const NEXUS_AudioEncoderCodecSettings *pSettings
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

/***************************************************************************
Summary:
    Get the audio connector for an Audio Encoder stage

Description:
This is used for a direct connection to SPDIF, as follows:

    NEXUS_AudioOutput_AddInput(NEXUS_SpdifOutput_GetConnector(spdif), NEXUS_AudioEncoder_GetConnector(audioEncoder));

***************************************************************************/
NEXUS_AudioInput NEXUS_AudioEncoder_GetConnector( /* attr{shutdown=NEXUS_AudioInput_Shutdown} */
    NEXUS_AudioEncoderHandle handle
    )
{
    BSTD_UNUSED(handle);
    return NULL;
}

/***************************************************************************
Summary:
Add an input to this processing stage
***************************************************************************/
NEXUS_Error NEXUS_AudioEncoder_AddInput(
    NEXUS_AudioEncoderHandle handle,
    NEXUS_AudioInput input
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(input);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

/***************************************************************************
Summary:
Remove an input from this processing stage
***************************************************************************/
NEXUS_Error NEXUS_AudioEncoder_RemoveInput(
    NEXUS_AudioEncoderHandle handle,
    NEXUS_AudioInput input
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(input);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

/***************************************************************************
Summary:
Remove all inputs from this processing stage
***************************************************************************/
NEXUS_Error NEXUS_AudioEncoder_RemoveAllInputs(
    NEXUS_AudioEncoderHandle handle
    )
{
    BSTD_UNUSED(handle);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}
