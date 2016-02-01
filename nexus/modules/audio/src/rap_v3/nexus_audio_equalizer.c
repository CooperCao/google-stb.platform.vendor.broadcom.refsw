/******************************************************************************
 *    (c)2011-2013 Broadcom Corporation
 * 
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 * 
 * Module Description:
 * 
 * Revision History:
 * 
 * $brcm_Log: $
 * 
 *****************************************************************************/
#include "nexus_audio_module.h"

BDBG_MODULE(nexus_audio_equalizer);

typedef struct NEXUS_AudioEqualizer
{
    NEXUS_OBJECT(NEXUS_AudioEqualizer);
    void *pDummy;
} NEXUS_AudioEqualizer;

typedef struct NEXUS_AudioEqualizerStage
{
    NEXUS_OBJECT(NEXUS_AudioEqualizerStage);
    void *pDummy;
} NEXUS_AudioEqualizerStage;

/***************************************************************************
Summary:
Get default open settings for an equalizer stage
***************************************************************************/
void NEXUS_AudioEqualizerStage_GetDefaultSettings(
    NEXUS_AudioEqualizerStageType type,
    NEXUS_AudioEqualizerStageSettings *pSettings     /* [out] */
    )
{
    BSTD_UNUSED(type);
    BSTD_UNUSED(pSettings);
}

/***************************************************************************
Summary:
Create an audio equalizer stage
***************************************************************************/
NEXUS_AudioEqualizerStageHandle NEXUS_AudioEqualizerStage_Create(	/* attr{destructor=NEXUS_AudioEqualizerStage_Destroy} */
    const NEXUS_AudioEqualizerStageSettings *pSettings	            /* attr{null_allowed=y} */
    )
{
    BSTD_UNUSED(pSettings);
    (void)BERR_TRACE(BERR_NOT_SUPPORTED);
    return NULL;
}

/***************************************************************************
Summary:
Destroy an audio equalizer stage
***************************************************************************/
static void NEXUS_AudioEqualizerStage_P_Finalizer(
    NEXUS_AudioEqualizerStageHandle handle
    )
{
    BSTD_UNUSED(handle);
}

NEXUS_OBJECT_CLASS_MAKE(NEXUS_AudioEqualizerStage, NEXUS_AudioEqualizerStage_Destroy);

/***************************************************************************
Summary:
Get settings for an audio equalizer stage
***************************************************************************/
void NEXUS_AudioEqualizerStage_GetSettings(
    NEXUS_AudioEqualizerStageHandle handle,
    NEXUS_AudioEqualizerStageSettings *pSettings /* [out] */
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);
}

/***************************************************************************
Summary:
Set settings for an audio equalizer stage
***************************************************************************/
NEXUS_Error NEXUS_AudioEqualizerStage_SetSettings(
    NEXUS_AudioEqualizerStageHandle handle,
    const NEXUS_AudioEqualizerStageSettings *pSettings
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

/***************************************************************************
Summary:
Get Default Equalizer Settings
***************************************************************************/
void NEXUS_AudioEqualizer_GetDefaultSettings(
    NEXUS_AudioEqualizerSettings *pSettings   /* [out] */
    )
{
    BSTD_UNUSED(pSettings);
}

/***************************************************************************
Summary:
Create an equalizer
***************************************************************************/
NEXUS_AudioEqualizerHandle NEXUS_AudioEqualizer_Create( /* attr{destructor=NEXUS_AudioEqualizer_Destroy} */
    const NEXUS_AudioEqualizerSettings *pSettings	    /* attr{null_allowed=y} */
    )
{
    BSTD_UNUSED(pSettings);
    (void)BERR_TRACE(BERR_NOT_SUPPORTED);
    return NULL;
}

/***************************************************************************
Summary:
Destroy an equalizer
***************************************************************************/
static void NEXUS_AudioEqualizer_P_Finalizer(
    NEXUS_AudioEqualizerHandle handle
    )
{
    BSTD_UNUSED(handle);
}

NEXUS_OBJECT_CLASS_MAKE(NEXUS_AudioEqualizer, NEXUS_AudioEqualizer_Destroy);

/***************************************************************************
Summary:
Remove a stage from an equalizer
***************************************************************************/
NEXUS_Error NEXUS_AudioEqualizer_AddStage(
	NEXUS_AudioEqualizerHandle equalizer,
	NEXUS_AudioEqualizerStageHandle stage
	)
{
    BSTD_UNUSED(equalizer);
    BSTD_UNUSED(stage);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}
 
/***************************************************************************
Summary:
Remove a stage from an equalizer
***************************************************************************/
NEXUS_Error NEXUS_AudioEqualizer_RemoveStage(
	NEXUS_AudioEqualizerHandle equalizer,
	NEXUS_AudioEqualizerStageHandle stage
    )
{
    BSTD_UNUSED(equalizer);
    BSTD_UNUSED(stage);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

/***************************************************************************
Summary:
Remove all stages from an equalizer
***************************************************************************/
NEXUS_Error NEXUS_AudioEqualizer_RemoveAllStages(
	NEXUS_AudioEqualizerHandle equalizer
    )
{
    BSTD_UNUSED(equalizer);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

/***************************************************************************
Summary:
Connect an equalizer to an output

Description:
This can only be called when all inputs to the specified output are 
stopped.
***************************************************************************/
NEXUS_Error NEXUS_AudioOutput_SetEqualizer(
    NEXUS_AudioOutputHandle output,
    NEXUS_AudioEqualizerHandle equalizer    /* Pass NULL to remove any equalizer connected to this output */
    )
{
    BSTD_UNUSED(output);
    BSTD_UNUSED(equalizer);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

/***************************************************************************
Summary:
Remove equalizer from the output.
***************************************************************************/
void NEXUS_AudioOutput_ClearEqualizer(
	NEXUS_AudioOutputHandle output
	)
{
    BSTD_UNUSED(output);
}

