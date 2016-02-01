/***************************************************************************
*     (c)2004-2012 Broadcom Corporation
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
*   API name: AutoVolumeLevel
*    Specific APIs related to AVL audio processing
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/

#include "nexus_audio_module.h"

BDBG_MODULE(nexus_auto_volume_level);

typedef struct NEXUS_AutoVolumeLevel  {
    NEXUS_OBJECT(NEXUS_AutoVolumeLevel);
} NEXUS_AutoVolumeLevel;

/***************************************************************************
Summary:
    Get default settings for an AVL processing stage
See Also:
    NEXUS_AutoVolumeLevel_Open
 ***************************************************************************/
void NEXUS_AutoVolumeLevel_GetDefaultSettings(
    NEXUS_AutoVolumeLevelSettings *pSettings   /* [out] default settings */
    )
{
    BSTD_UNUSED(pSettings);
}

/***************************************************************************
Summary:
    Open a AVL processing stage
See Also:
    NEXUS_AutoVolumeLevel_Close
 ***************************************************************************/
NEXUS_AutoVolumeLevelHandle NEXUS_AutoVolumeLevel_Open(
    const NEXUS_AutoVolumeLevelSettings *pSettings     /* Pass NULL for default settings */
    )
{
    BSTD_UNUSED(pSettings);
    return (NULL);
}

/***************************************************************************
Summary:
    Close a AVL processing stage
Description:
    Input to the stage must be removed prior to closing.
See Also:
    NEXUS_AutoVolumeLevel_Open
    NEXUS_AudioOutput_RemoveInput
    NEXUS_AudioOutput_RemoveAllInputs
 ***************************************************************************/
static void NEXUS_AutoVolumeLevel_P_Finalizer(
    NEXUS_AutoVolumeLevelHandle handle
    )
{
    BSTD_UNUSED(handle);
}


/***************************************************************************
Summary:
    Get Settings for a AVL processing stage
See Also:
    NEXUS_AutoVolumeLevel_SetSettings
 ***************************************************************************/
void NEXUS_AutoVolumeLevel_GetSettings(
    NEXUS_AutoVolumeLevelHandle handle,
    NEXUS_AutoVolumeLevelSettings *pSettings    /* [out] Settings */
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);
}


/***************************************************************************
Summary:
    Set Settings for a AVL processing stage
See Also:
    NEXUS_AutoVolumeLevel_GetSettings
 ***************************************************************************/
NEXUS_Error NEXUS_AutoVolumeLevel_SetSettings(
    NEXUS_AutoVolumeLevelHandle handle,
    const NEXUS_AutoVolumeLevelSettings *pSettings
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

/***************************************************************************
Summary:
    Get the audio connector for a AVL processing stage
See Also:

 ***************************************************************************/
NEXUS_AudioInput NEXUS_AutoVolumeLevel_GetConnector(
    NEXUS_AutoVolumeLevelHandle handle
    )
{
    BSTD_UNUSED(handle);
    return (NULL);
}

/***************************************************************************
Summary:
    Add an input to this processing stage
See Also:
    NEXUS_AutoVolumeLevel_RemoveInput
    NEXUS_AutoVolumeLevel_RemoveAllInputs
 ***************************************************************************/
NEXUS_Error NEXUS_AutoVolumeLevel_AddInput(
    NEXUS_AutoVolumeLevelHandle handle,
    NEXUS_AudioInput input
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(input);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

/***************************************************************************
Summary:
    Add an input to this processing stage
See Also:
    NEXUS_AutoVolumeLevel_AddInput
    NEXUS_AutoVolumeLevel_RemoveAllInputs
 ***************************************************************************/
NEXUS_Error NEXUS_AutoVolumeLevel_RemoveInput(
    NEXUS_AutoVolumeLevelHandle handle,
    NEXUS_AudioInput input
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(input);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

/***************************************************************************
Summary:
    Add an input to this processing stage
See Also:
    NEXUS_AutoVolumeLevel_AddInput
    NEXUS_AutoVolumeLevel_RemoveInput
 ***************************************************************************/
NEXUS_Error NEXUS_AutoVolumeLevel_RemoveAllInputs(
    NEXUS_AutoVolumeLevelHandle handle
    )
{
    BSTD_UNUSED(handle);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

NEXUS_OBJECT_CLASS_MAKE(NEXUS_AutoVolumeLevel, NEXUS_AutoVolumeLevel_Close);

