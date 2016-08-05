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
#include "blst_squeue.h"
#include "nexus_audio_module.h"


#define MAX_STAGES_PER_EQUALIZER (5)

BDBG_MODULE(nexus_audio_equalizer);

typedef struct NEXUS_AudioEqualizerOutputNode
{
    BLST_S_ENTRY(NEXUS_AudioEqualizerOutputNode) node;
    NEXUS_AudioOutputHandle output;           /* Output attached to this equalizer */
} NEXUS_AudioEqualizerOutputNode;

typedef struct NEXUS_AudioEqualizerStage
{
    NEXUS_OBJECT(NEXUS_AudioEqualizerStage);
    NEXUS_AudioEqualizerStageSettings settings;
    BAPE_EqualizerStageHandle hStage;
    unsigned usageCount;
}NEXUS_AudioEqualizerStage;


typedef struct NEXUS_AudioEqualizerStageNode
{
    NEXUS_AudioEqualizerStageHandle stage;
    BLST_SQ_ENTRY(NEXUS_AudioEqualizerStageNode) node;
} NEXUS_AudioEqualizerStageNode;

typedef struct NEXUS_AudioEqualizer
{
    NEXUS_OBJECT(NEXUS_AudioEqualizer);
    NEXUS_AudioEqualizerSettings settings;
    BAPE_EqualizerStageHandle pApeStages[MAX_STAGES_PER_EQUALIZER];
    unsigned numStages;
    /* List of outputs connected to this equalizer */
    BLST_S_HEAD(EqualizerOutputList, NEXUS_AudioEqualizerOutputNode) outputList;
    BLST_SQ_HEAD(EqualizerStageList, NEXUS_AudioEqualizerStageNode) stageList;
} NEXUS_AudioEqualizer;


static NEXUS_Error NEXUS_AudioEqualizer_P_CreateConnection(NEXUS_AudioEqualizerHandle handle, NEXUS_AudioOutputHandle output);
static void NEXUS_AudioEqualizer_P_DestroyConnection(NEXUS_AudioEqualizerHandle handle, NEXUS_AudioOutputHandle output);
static void NEXUS_AudioEqualizer_P_PopulateStageArray(NEXUS_AudioEqualizerHandle equalizer);



static NEXUS_AudioEqualizerWindowStep NEXUS_AudioEqualizer_P_WindowStepToNexus(BAPE_EqualizerWindowStep magnum)
{
    switch ( magnum )
    {
    default:
    case BAPE_EqualizerWindowStep_eNone:
        return NEXUS_AudioEqualizerWindowStep_eNone;
    case BAPE_EqualizerWindowStep_e170_6:
        return NEXUS_AudioEqualizerWindowStep_e170_6;
    case BAPE_EqualizerWindowStep_e85_3:
        return NEXUS_AudioEqualizerWindowStep_e85_3;
    case BAPE_EqualizerWindowStep_e42_6:
        return NEXUS_AudioEqualizerWindowStep_e42_6;
    case BAPE_EqualizerWindowStep_e21_3:
        return NEXUS_AudioEqualizerWindowStep_e21_3;
    case BAPE_EqualizerWindowStep_e10_6:
        return NEXUS_AudioEqualizerWindowStep_e10_6;
    case BAPE_EqualizerWindowStep_e5_3:
        return NEXUS_AudioEqualizerWindowStep_e5_3;
    case BAPE_EqualizerWindowStep_e2_6:
        return NEXUS_AudioEqualizerWindowStep_e2_6;
    }
}


static BAPE_EqualizerWindowStep NEXUS_AudioEqualizer_P_WindowStepToMagnum(NEXUS_AudioEqualizerWindowStep nexus)
{
    switch ( nexus )
    {
    default:
    case NEXUS_AudioEqualizerWindowStep_eNone:
        return BAPE_EqualizerWindowStep_eNone;
    case NEXUS_AudioEqualizerWindowStep_e170_6:
        return BAPE_EqualizerWindowStep_e170_6;
    case NEXUS_AudioEqualizerWindowStep_e85_3:
        return BAPE_EqualizerWindowStep_e85_3;
    case NEXUS_AudioEqualizerWindowStep_e42_6:
        return BAPE_EqualizerWindowStep_e42_6;
    case NEXUS_AudioEqualizerWindowStep_e21_3:
        return BAPE_EqualizerWindowStep_e21_3;
    case NEXUS_AudioEqualizerWindowStep_e10_6:
        return BAPE_EqualizerWindowStep_e10_6;
    case NEXUS_AudioEqualizerWindowStep_e5_3:
        return BAPE_EqualizerWindowStep_e5_3;
    case NEXUS_AudioEqualizerWindowStep_e2_6:
        return BAPE_EqualizerWindowStep_e2_6;
    }
}

/***************************************************************************
Summary:
Get default open settings for an equalizer stage.
***************************************************************************/
void NEXUS_AudioEqualizerStage_GetDefaultSettings(
    NEXUS_AudioEqualizerStageType type,
    NEXUS_AudioEqualizerStageSettings *pSettings     /* [out] */
    )
{
    BAPE_EqualizerStageSettings settings;
    unsigned i;
    
    BDBG_ASSERT(NULL != pSettings);
    
    pSettings->type = type;
    pSettings->rampSettings.enable = false;
    pSettings->rampSettings.stepSize = 0xA;
    pSettings->enabled = true;
    
    switch(pSettings->type)
    {
        case NEXUS_AudioEqualizerStageType_eToneControl:

			BAPE_EqualizerStage_GetDefaultSettings(BAPE_EqualizerStageType_eToneControl,&settings);
			pSettings->modeSettings.toneControl.bassSettings.bandwidthFreq = settings.modeSettings.toneControl.bassBandwidthFreq;
			pSettings->modeSettings.toneControl.bassSettings.eqType = settings.modeSettings.toneControl.bassEqType;
			pSettings->modeSettings.toneControl.bassSettings.freq = settings.modeSettings.toneControl.bassFreq;
			pSettings->modeSettings.toneControl.bassSettings.gain = settings.modeSettings.toneControl.bassGain;
			pSettings->modeSettings.toneControl.trebleSettings.bandwidthFreq = settings.modeSettings.toneControl.trebleBandwidthFreq;
			pSettings->modeSettings.toneControl.trebleSettings.eqType = settings.modeSettings.toneControl.trebleEqType;
			pSettings->modeSettings.toneControl.trebleSettings.freq = settings.modeSettings.toneControl.trebleFreq;
			pSettings->modeSettings.toneControl.trebleSettings.gain = settings.modeSettings.toneControl.trebleGain;
            break;
        case NEXUS_AudioEqualizerStageType_eFiveBand:
			BAPE_EqualizerStage_GetDefaultSettings(BAPE_EqualizerStageType_eFiveBand,&settings);
			pSettings->modeSettings.fiveBand.gain10000Hz = settings.modeSettings.fiveBand.gain10000Hz;
			pSettings->modeSettings.fiveBand.gain1000Hz = settings.modeSettings.fiveBand.gain1000Hz;
			pSettings->modeSettings.fiveBand.gain100Hz = settings.modeSettings.fiveBand.gain100Hz;
			pSettings->modeSettings.fiveBand.gain3000Hz = settings.modeSettings.fiveBand.gain3000Hz;
			pSettings->modeSettings.fiveBand.gain300Hz = settings.modeSettings.fiveBand.gain300Hz;
            break;
        case NEXUS_AudioEqualizerStageType_eSevenBand:
			BAPE_EqualizerStage_GetDefaultSettings(BAPE_EqualizerStageType_eSevenBand,&settings);

		    for ( i = 0; i < 7; i++ )
		    {
				pSettings->modeSettings.sevenBand.bandSettings[i].peak = settings.modeSettings.sevenBand.bandSettings[i].peak;
				pSettings->modeSettings.sevenBand.bandSettings[i].gain = settings.modeSettings.sevenBand.bandSettings[i].gain;
				pSettings->modeSettings.sevenBand.bandSettings[i].q = settings.modeSettings.sevenBand.bandSettings[i].q;
			}
			pSettings->modeSettings.sevenBand.windowStep = NEXUS_AudioEqualizer_P_WindowStepToNexus(settings.modeSettings.sevenBand.windowStep);
		
            break;
        case NEXUS_AudioEqualizerStageType_eSubsonic:
			BAPE_EqualizerStage_GetDefaultSettings(BAPE_EqualizerStageType_eSubsonic,&settings);
			pSettings->modeSettings.subsonic.filterOrder = settings.modeSettings.subsonic.filterOrder;
			pSettings->modeSettings.subsonic.filterType = settings.modeSettings.subsonic.filterType;
			pSettings->modeSettings.subsonic.frequency = settings.modeSettings.subsonic.frequency;
            break;
        case NEXUS_AudioEqualizerStageType_eSubwoofer:
			BAPE_EqualizerStage_GetDefaultSettings(BAPE_EqualizerStageType_eSubwoofer,&settings);
			pSettings->modeSettings.subwoofer.filterOrder = settings.modeSettings.subwoofer.filterOrder;
			pSettings->modeSettings.subwoofer.filterType = settings.modeSettings.subwoofer.filterType;
			pSettings->modeSettings.subwoofer.frequency = settings.modeSettings.subwoofer.frequency;
	        break;
        default:
            BDBG_ERR(("Unsupported Equalizer Stage Type %d", pSettings->type));
            break;
            /*return BERR_NOT_SUPPORTED;*/
    }
    
}


/***************************************************************************
Summary:
Get default open settings for an equalizer.
***************************************************************************/
void NEXUS_AudioEqualizer_GetDefaultSettings(
    NEXUS_AudioEqualizerSettings *pSettings    /* [out] Default Settings */
    )
{
    BDBG_ASSERT(NULL != pSettings);
    
    BKNI_Memset(pSettings, 0, sizeof(NEXUS_AudioEqualizerSettings));        
}

/***************************************************************************
Summary:
Create an audio equalizer.
***************************************************************************/
NEXUS_AudioEqualizerHandle NEXUS_AudioEqualizer_Create(
    const NEXUS_AudioEqualizerSettings *pSettings	/* attr{null_allowed=y} */
    )
{
    NEXUS_AudioEqualizerSettings defaults;
    NEXUS_AudioEqualizerHandle handle = NULL;
    
    if ( NULL == pSettings )
    {
        NEXUS_AudioEqualizer_GetDefaultSettings(&defaults);
        pSettings = &defaults;
    }
    
    handle = BKNI_Malloc(sizeof(NEXUS_AudioEqualizer));
    if ( NULL == handle )
    {
        (void)BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto err_malloc;
    }
   
    NEXUS_OBJECT_INIT(NEXUS_AudioEqualizer, handle);

    handle->settings = *pSettings;

	/*Success*/
	return handle;

err_malloc:
    return NULL;	    
}

/***************************************************************************
Summary:
Create an audio equalizer stage.
***************************************************************************/
NEXUS_AudioEqualizerStageHandle NEXUS_AudioEqualizerStage_Create(
    const NEXUS_AudioEqualizerStageSettings *pSettings	/* Settings must be initialized by NEXUS_AudioEqualizerStage_GetDefaultSettings() 
                                                               					for the expected equalizer stage type */
    )
{
    NEXUS_AudioEqualizerStageHandle   handle = NULL;
    NEXUS_AudioEqualizerStageSettings defaultSettings;
    BERR_Code errCode;

    if(NULL == pSettings)
    {
		NEXUS_AudioEqualizerStage_GetDefaultSettings(NEXUS_AudioEqualizerStageType_eToneControl, &defaultSettings);
		pSettings = &defaultSettings;

    }

    handle = BKNI_Malloc(sizeof(NEXUS_AudioEqualizerStage));
    if ( NULL == handle )
    {
    	(void)BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto err_malloc;
	}
    
	NEXUS_OBJECT_INIT(NEXUS_AudioEqualizerStage, handle);
    /*	BDBG_OBJECT_SET(handle, NEXUS_AudioEqualizerStage);		*/

	handle->settings = *pSettings;
    handle->usageCount = 0;
    

	errCode = BAPE_EqualizerStage_Create(NEXUS_AUDIO_DEVICE_HANDLE,NULL,&(handle->hStage));
	if ( errCode )
	{
		errCode = BERR_TRACE(errCode);
		goto err_create;								  
	}		

	NEXUS_AudioEqualizerStage_SetSettings( handle, &(handle->settings));

	/**Success **/	
	return handle;

err_create:
    NEXUS_OBJECT_DESTROY(NEXUS_AudioEqualizerStage, handle);
    BKNI_Free(handle);
err_malloc:
    return NULL;	

}

static void NEXUS_AudioEqualizer_P_PopulateStageArray(NEXUS_AudioEqualizerHandle equalizer)
{
    NEXUS_AudioEqualizerStageNode *pNode;
    unsigned i;
 
    i=0;
    for ( pNode = BLST_SQ_FIRST(&equalizer->stageList);
          NULL != pNode && i < MAX_STAGES_PER_EQUALIZER;
          pNode = BLST_SQ_NEXT(pNode, node) )
    {
        equalizer->pApeStages[i++] = pNode->stage->hStage;
    }
    for ( ; i < MAX_STAGES_PER_EQUALIZER; i++ )
    {
        equalizer->pApeStages[i] = NULL;
    }
}
 
/***************************************************************************
Summary:
Remove a stage from an equalizer
***************************************************************************/
NEXUS_Error NEXUS_AudioEqualizer_AddStage(NEXUS_AudioEqualizerHandle equalizer, NEXUS_AudioEqualizerStageHandle stage)
{
    NEXUS_AudioEqualizerStageNode *pNode;

    NEXUS_OBJECT_ASSERT(NEXUS_AudioEqualizer, equalizer);
    NEXUS_OBJECT_ASSERT(NEXUS_AudioEqualizerStage, stage);

    if ( equalizer->numStages >= MAX_STAGES_PER_EQUALIZER )
	{
        BDBG_ERR(("Equalizer %p can not have more than %u stages", (void *)equalizer, MAX_STAGES_PER_EQUALIZER));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
	}

    /* Check if the stage is already used by this eq */
    for ( pNode = BLST_SQ_FIRST(&equalizer->stageList);
          NULL != pNode;
          pNode = BLST_SQ_NEXT(pNode, node) )
            {
        if ( pNode->stage == stage )
                {
            BDBG_ERR(("An equalizer stage can not be used multiple times by the same equalizer"));
                    return BERR_TRACE(BERR_NOT_SUPPORTED);
                }
        }

    pNode = BKNI_Malloc(sizeof(NEXUS_AudioEqualizerStageNode));
    if ( NULL == pNode )
    {
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }

    pNode->stage = stage;
    BLST_SQ_INSERT_TAIL(&equalizer->stageList, pNode, node);

    equalizer->numStages++;
    stage->usageCount++;

    return BERR_SUCCESS;
    }

/***************************************************************************
Summary:
Remove a stage from an equalizer
***************************************************************************/
NEXUS_Error NEXUS_AudioEqualizer_RemoveStage(NEXUS_AudioEqualizerHandle equalizer, NEXUS_AudioEqualizerStageHandle stage)
{
    NEXUS_AudioEqualizerStageNode *pNode;

    NEXUS_OBJECT_ASSERT(NEXUS_AudioEqualizer, equalizer);
    NEXUS_OBJECT_ASSERT(NEXUS_AudioEqualizerStage, stage);

    /* Check if the stage is already used by this eq */
    for ( pNode = BLST_SQ_FIRST(&equalizer->stageList);
          NULL != pNode;
          pNode = BLST_SQ_NEXT(pNode, node) )
    {
        if ( pNode->stage == stage )
        {
            break;
        }
    }

    if ( NULL == pNode )
    {
        BDBG_ERR(("Stage %p is not used by equalizer %p", (void *)stage, (void *)equalizer));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    BLST_SQ_REMOVE(&equalizer->stageList, pNode, NEXUS_AudioEqualizerStageNode, node);
    equalizer->numStages--;
    stage->usageCount--;
    BKNI_Free(pNode);

	return BERR_SUCCESS;
}


/***************************************************************************
Summary:
Remove all stages from an equalizer
***************************************************************************/
NEXUS_Error NEXUS_AudioEqualizer_RemoveAllStages(NEXUS_AudioEqualizerHandle equalizer)
{
    NEXUS_AudioEqualizerStageNode *pNode;
    NEXUS_Error errCode;
	
    while ( (pNode = BLST_SQ_FIRST(&equalizer->stageList)) )
    {
        errCode = NEXUS_AudioEqualizer_RemoveStage(equalizer, pNode->stage);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    }

    return BERR_SUCCESS;
}

/***************************************************************************
Summary:
Get settings for an equalizer.
***************************************************************************/
void NEXUS_AudioEqualizerStage_GetSettings(
    NEXUS_AudioEqualizerStageHandle handle,
    NEXUS_AudioEqualizerStageSettings *pSettings /* [out] */
    )
{
	NEXUS_OBJECT_ASSERT(NEXUS_AudioEqualizerStage, handle);
    *pSettings = handle->settings;
}


/***************************************************************************
Summary:
Set settings for an equalizer.
***************************************************************************/
NEXUS_Error NEXUS_AudioEqualizerStage_SetSettings(
    NEXUS_AudioEqualizerStageHandle handle,
    const NEXUS_AudioEqualizerStageSettings *pSettings
    )
{
	BERR_Code errCode;
	BAPE_EqualizerStageSettings piSettings;
	unsigned i;


    NEXUS_OBJECT_ASSERT(NEXUS_AudioEqualizerStage, handle);

	BAPE_EqualizerStage_GetSettings(handle->hStage, &piSettings );

	if(pSettings->enabled == true)
	{
		piSettings.bypassEnabled = false;
	}
	else
	{
		piSettings.bypassEnabled = true;
	}
	piSettings.rampSettings.enable = pSettings->rampSettings.enable;
	piSettings.rampSettings.stepSize = pSettings->rampSettings.stepSize;
	piSettings.type = pSettings->type;

	/*** TODO convert the following to element copy instead of struct copy ***/
	switch(pSettings->type)
	{
		case NEXUS_AudioEqualizerStageType_eToneControl:
		piSettings.modeSettings.toneControl.bassBandwidthFreq = pSettings->modeSettings.toneControl.bassSettings.bandwidthFreq;
		piSettings.modeSettings.toneControl.bassEqType = pSettings->modeSettings.toneControl.bassSettings.eqType;
		piSettings.modeSettings.toneControl.bassFreq = pSettings->modeSettings.toneControl.bassSettings.freq;
		piSettings.modeSettings.toneControl.bassGain = pSettings->modeSettings.toneControl.bassSettings.gain;
		piSettings.modeSettings.toneControl.trebleBandwidthFreq = pSettings->modeSettings.toneControl.trebleSettings.bandwidthFreq;
		piSettings.modeSettings.toneControl.trebleEqType = pSettings->modeSettings.toneControl.trebleSettings.eqType;
		piSettings.modeSettings.toneControl.trebleFreq = pSettings->modeSettings.toneControl.trebleSettings.freq;
		piSettings.modeSettings.toneControl.trebleGain = pSettings->modeSettings.toneControl.trebleSettings.gain;
		break;
		case NEXUS_AudioEqualizerStageType_eFiveBand:
		piSettings.modeSettings.fiveBand.gain10000Hz = pSettings->modeSettings.fiveBand.gain10000Hz;
		piSettings.modeSettings.fiveBand.gain1000Hz = pSettings->modeSettings.fiveBand.gain1000Hz;
		piSettings.modeSettings.fiveBand.gain100Hz = pSettings->modeSettings.fiveBand.gain100Hz;
		piSettings.modeSettings.fiveBand.gain3000Hz = pSettings->modeSettings.fiveBand.gain3000Hz;
		piSettings.modeSettings.fiveBand.gain300Hz = pSettings->modeSettings.fiveBand.gain300Hz;		
		break;

        case NEXUS_AudioEqualizerStageType_eSevenBand:
		for ( i = 0; i < 7; i++ )
	    {
			piSettings.modeSettings.sevenBand.bandSettings[i].peak = pSettings->modeSettings.sevenBand.bandSettings[i].peak;
			piSettings.modeSettings.sevenBand.bandSettings[i].gain = pSettings->modeSettings.sevenBand.bandSettings[i].gain;
			piSettings.modeSettings.sevenBand.bandSettings[i].q = pSettings->modeSettings.sevenBand.bandSettings[i].q;
		}
		
		piSettings.modeSettings.sevenBand.windowStep = NEXUS_AudioEqualizer_P_WindowStepToMagnum(pSettings->modeSettings.sevenBand.windowStep);
        break;

		case NEXUS_AudioEqualizerStageType_eSubsonic:
		piSettings.modeSettings.subsonic.filterOrder = pSettings->modeSettings.subsonic.filterOrder;
		piSettings.modeSettings.subsonic.filterType = pSettings->modeSettings.subsonic.filterType;
        BDBG_CASSERT((int)NEXUS_AudioEqualizerFilterType_eButterworth == (int)BAPE_EqualizerFilterType_eButterworth);
        BDBG_CASSERT((int)NEXUS_AudioEqualizerFilterType_eLinkwitzRiley == (int)BAPE_EqualizerFilterType_eLinkwitzRiley);
        BDBG_CASSERT((int)NEXUS_AudioEqualizerFilterType_eMax == (int)BAPE_EqualizerFilterType_eMax);
		piSettings.modeSettings.subsonic.frequency = pSettings->modeSettings.subsonic.frequency;
		break;

		case NEXUS_AudioEqualizerStageType_eSubwoofer:
		piSettings.modeSettings.subwoofer.filterOrder = pSettings->modeSettings.subwoofer.filterOrder;
		piSettings.modeSettings.subwoofer.filterType = pSettings->modeSettings.subwoofer.filterType;
        BDBG_CASSERT((int)NEXUS_AudioEqualizerFilterType_eButterworth == (int)BAPE_EqualizerFilterType_eButterworth);
        BDBG_CASSERT((int)NEXUS_AudioEqualizerFilterType_eLinkwitzRiley == (int)BAPE_EqualizerFilterType_eLinkwitzRiley);
        BDBG_CASSERT((int)NEXUS_AudioEqualizerFilterType_eMax == (int)BAPE_EqualizerFilterType_eMax);
		piSettings.modeSettings.subwoofer.frequency = pSettings->modeSettings.subwoofer.frequency;		
		break;
		
		default:
            BDBG_ERR(("Unsupported Equalizer Stage Type %d", pSettings->type));
        break;
	}
	
	errCode = BAPE_EqualizerStage_SetSettings( handle->hStage, &piSettings);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    handle->settings = *pSettings;
    return BERR_SUCCESS;
}


/***************************************************************************
Summary:
    Destroy an audio equalizer
    
***************************************************************************/
static void NEXUS_AudioEqualizer_P_Finalizer(
    NEXUS_AudioEqualizerHandle handle
    )
{
    NEXUS_Error errCode;
    NEXUS_AudioInputHandle input;
    NEXUS_AudioOutputData *pData;
    NEXUS_AudioEqualizerOutputNode *pNode;

    NEXUS_OBJECT_ASSERT(NEXUS_AudioEqualizer, handle);

   
    for ( pNode = BLST_S_FIRST(&handle->outputList);
          NULL != pNode;
          pNode = BLST_S_NEXT(pNode, node) )
    {
        /* Break the connection between the output and input to force release all the PI resources */
        pData = pNode->output->pMixerData;
        if ( pData )
        {
            input = pData->input;
            if ( input )
            {
                /* Forcibly disconnect the output.  If running, this will make all inputs stop for a safe shutdown. */
                (void)NEXUS_AudioOutput_RemoveAllInputs(pNode->output);
                errCode = NEXUS_AudioOutput_AddInput(pNode->output, input);
                BDBG_ASSERT(errCode == NEXUS_SUCCESS);
            }
        }
    }
    
    /* Remove any connected stages */
    NEXUS_AudioEqualizer_RemoveAllStages(handle);

    /* Go through all connected outputs and break the connections */
    while ( NULL != (pNode = BLST_S_FIRST(&handle->outputList)) )
    {
        NEXUS_AudioOutput_ClearEqualizer(pNode->output);
    }
    
    NEXUS_OBJECT_DESTROY(NEXUS_AudioEqualizer, handle);
    BKNI_Free(handle);
}


NEXUS_OBJECT_CLASS_MAKE(NEXUS_AudioEqualizer, NEXUS_AudioEqualizer_Destroy);

/***************************************************************************
Summary:
Get settings for an audio equalizer stage
***************************************************************************/
void NEXUS_AudioEqualizer_GetSettings(
    NEXUS_AudioEqualizerHandle handle,
    NEXUS_AudioEqualizerSettings *pSettings    /* [out] Current Settings */
    )
{
    NEXUS_OBJECT_ASSERT(NEXUS_AudioEqualizer, handle);
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memcpy(pSettings, &handle->settings, sizeof(NEXUS_AudioEqualizerSettings));
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
    NEXUS_AudioOutputData *pData;
    NEXUS_Error errCode;
    
    BDBG_OBJECT_ASSERT(output, NEXUS_AudioOutput);
    if ( NULL != equalizer )
    {
        /* Passing NULL to remove the equalizer is acceptable */
        BDBG_OBJECT_ASSERT(equalizer, NEXUS_AudioEqualizer);
    }
    
    if ( NEXUS_GetEnv("audio_equalizer_disabled") )
    {
        return BERR_SUCCESS;
    }

    pData = output->pMixerData;
    if ( NULL == pData )
    {
        pData = NEXUS_AudioOutput_P_CreateData(output);
        if ( NULL == pData )
        {
            return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        }
    }
    
    if ( equalizer != pData->equalizer )
    {
        if ( NEXUS_AudioOutput_P_IsRunning(output) )
        {
            BDBG_ERR(("Can not change equalizers while an output is running."));
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
        if ( pData->equalizer )
        {
            /* If connected to an EQ already, break the connection */
            NEXUS_AudioEqualizer_P_DestroyConnection(pData->equalizer, output);
            pData->equalizer = NULL;
        }
        if ( equalizer )
        {
            errCode = NEXUS_AudioEqualizer_P_CreateConnection(equalizer, output);
            if ( errCode )
            {
                return BERR_TRACE(errCode);
            }
            pData->equalizer = equalizer;
        }
        if ( pData->input )
        {
            /* Mixer reconfiguration will be required on the next start/stop */
            (void)NEXUS_AudioInput_P_OutputSettingsChanged(pData->input, output);
        }
    }
    
    return BERR_SUCCESS;
}

/***************************************************************************
Summary:
Remove equalizer from the output.
***************************************************************************/
void NEXUS_AudioOutput_ClearEqualizer( NEXUS_AudioOutputHandle output)
{
	(void)NEXUS_AudioOutput_SetEqualizer(output, NULL);
}

/***************************************************************************
Summary:
Create connection for an audio equalizer.
***************************************************************************/
static NEXUS_Error NEXUS_AudioEqualizer_P_CreateConnection(NEXUS_AudioEqualizerHandle handle, NEXUS_AudioOutputHandle output)
{
    NEXUS_AudioOutputData *pData;
    NEXUS_AudioEqualizerOutputNode *pNode;

    NEXUS_OBJECT_ASSERT(NEXUS_AudioEqualizer, handle);
    BDBG_OBJECT_ASSERT(output, NEXUS_AudioOutput);

    pData = output->pMixerData;
    if ( NULL == pData )
    {
        pData = NEXUS_AudioOutput_P_CreateData(output);
        if ( NULL == pData )
        {
            return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        }
    }

    /* Make sure no other connection exists */
    BDBG_ASSERT(pData->equalizer == NULL);

    /* Create node for output */
    pNode = BKNI_Malloc(sizeof(NEXUS_AudioEqualizerOutputNode));
    if ( NULL == pNode )
    {
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }
    BKNI_Memset(pNode, 0, sizeof(NEXUS_AudioEqualizerOutputNode));

    pNode->output = output;
    BLST_S_INSERT_HEAD(&handle->outputList, pNode, node);
    pData->equalizer = handle;

    return NEXUS_SUCCESS;
}


/***************************************************************************
Summary:
Destroy connection for an audio equalizer.
***************************************************************************/
static void NEXUS_AudioEqualizer_P_DestroyConnection(NEXUS_AudioEqualizerHandle handle, NEXUS_AudioOutputHandle output)
{
    NEXUS_AudioOutputData *pData;
    NEXUS_AudioEqualizerOutputNode *pPrev=NULL, *pNode;
    
    NEXUS_OBJECT_ASSERT(NEXUS_AudioEqualizer, handle);
    BDBG_OBJECT_ASSERT(output, NEXUS_AudioOutput);
    
    pData = output->pMixerData;
    if ( pData )
    {
        NEXUS_AudioInputHandle input = pData->input;
        
        /* Make sure these are actually connected */
        BDBG_ASSERT(pData->equalizer == handle);    

        if ( input )
        {
            NEXUS_Error errCode;

            if ( NEXUS_AudioInput_P_IsRunning(input) )
            {
                BDBG_WRN(("Forcing input %p to stop on equalizer %p shutdown", (void *)input, (void *)handle));
                NEXUS_AudioInput_P_ForceStop(input);
            }

            /* Make sure the actual equalizer is destroyed by removing and re-adding the output now that we're sure it's stopped */
            NEXUS_AudioOutput_RemoveAllInputs(output);
            errCode = NEXUS_AudioOutput_AddInput(output, input);
            /* This had best not fail */
            BDBG_ASSERT(errCode == BERR_SUCCESS);
        }
    }
    for ( pNode = BLST_S_FIRST(&handle->outputList);
          NULL != pNode;
          pNode = BLST_S_NEXT(pNode, node) )
    {
        if ( pNode->output == output )
        {
            /* Unlink the output from the equalizer */
            if ( pPrev )
            {
                BLST_S_REMOVE_NEXT(&handle->outputList, pPrev, node);
            }
            else
            {
                BLST_S_REMOVE_HEAD(&handle->outputList, node);
            }
            BKNI_Memset(pNode, 0, sizeof(NEXUS_AudioEqualizerOutputNode));
            BKNI_Free(pNode);
            break;
        }
        pPrev = pNode;
    }
}

/***************************************************************************
Summary:
    Destroy an audio equalizer stage

Description:
	All the audio equalizers that were using the stage must be destryed before destroying the stage.
    
***************************************************************************/
static void NEXUS_AudioEqualizerStage_P_Finalizer( NEXUS_AudioEqualizerStageHandle handle)
{
	NEXUS_OBJECT_ASSERT(NEXUS_AudioEqualizerStage, handle);

    if ( handle->usageCount > 0 )
    {
        BDBG_ERR(("Equalizer Stage %p is still connected to %u equalizers.  Please remove the stage from all equalizers before destroying", (void *)handle, handle->usageCount));
        BDBG_ASSERT(handle->usageCount == 0);
    }

	BAPE_EqualizerStage_Destroy(handle->hStage);

    NEXUS_OBJECT_DESTROY(NEXUS_AudioEqualizerStage, handle);
	BKNI_Free(handle);
}



NEXUS_OBJECT_CLASS_MAKE(NEXUS_AudioEqualizerStage, NEXUS_AudioEqualizerStage_Destroy);


/***************************************************************************
Summary:
    Get an audio equalizer

***************************************************************************/
NEXUS_AudioEqualizerHandle NEXUS_AudioOutput_P_GetEqualizer(
    NEXUS_AudioOutputHandle output
    )
{
    NEXUS_AudioOutputData *pData;

    BDBG_OBJECT_ASSERT(output, NEXUS_AudioOutput);

    pData = output->pMixerData;
    if ( pData )
    {
        return pData->equalizer;
    }
    return NULL;
}

/***************************************************************************
Summary:
    Get audio equalizer stages.

***************************************************************************/
void NEXUS_AudioEqualizer_P_GetStages(
    NEXUS_AudioEqualizerHandle handle,
    BAPE_EqualizerStageHandle **pStages,
    unsigned *pNumStages
    )
{
    NEXUS_OBJECT_ASSERT(NEXUS_AudioEqualizer, handle);
    BDBG_ASSERT(NULL != pStages);
    BDBG_ASSERT(NULL != pNumStages);

    NEXUS_AudioEqualizer_P_PopulateStageArray(handle);

    *pStages = handle->pApeStages;
    *pNumStages = handle->numStages;
}
