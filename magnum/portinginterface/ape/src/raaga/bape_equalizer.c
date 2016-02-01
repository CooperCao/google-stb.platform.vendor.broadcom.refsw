/***************************************************************************
 *     Copyright (c) 2006-2013, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 * 
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description: Audio Equalizer Interface
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#include "bape.h"
#include "bape_priv.h"
#include "bape_eq_coeff_priv.h"

BDBG_MODULE(bape_equalizer);

BDBG_OBJECT_ID(BAPE_Equalizer);

#ifdef BCHP_AUD_FMM_SRC_CTRL0_SRC_CFGi_SRC_TYPE_IIR

#define BAPE_MAX_STAGES_PER_EQUALIZER   BAPE_CHIP_MAX_SRCS   /* Increase if required,with SRCs availability assured since a few stages can be packed into single SRC */
#define BAPE_P_MAX_CASCADED_SRCS_PER_EQUALIZER    BAPE_CHIP_MAX_SRCS
#define BAPE_P_MAX_EQSTAGE_INSTANCES    BAPE_CHIP_MAX_SRCS

typedef struct BAPE_Equalizer
{
    BDBG_OBJECT(BAPE_Equalizer)
    BAPE_Handle         deviceHandle;
    BAPE_Connector      input;
    BAPE_PathNode       pathNode;
    bool                resourcesAllocated;
    bool                running;
    unsigned            numCascadeSrcs;
    BAPE_SrcGroupHandle src[BAPE_P_MAX_CASCADED_SRCS_PER_EQUALIZER];
    BAPE_EqualizerStageHandle   eqStage[BAPE_MAX_STAGES_PER_EQUALIZER];
}BAPE_Equalizer;

typedef struct BAPE_EqualizerStage
{
    unsigned                    sampleRate;
    BAPE_SrcGroupHandle         srcGroups[BAPE_P_MAX_EQSTAGE_INSTANCES]; /* currently no EQ operation needs more than 1 SRC per Channel Pair */
    BAPE_EqualizerStageSettings settings;
}BAPE_EqualizerStage;


/********** Default Tone Control Settings *******************/
static const BAPE_EqualizerToneControlSettings deftoneControlsettings =
{
    /* bass */
    0,
    100,
    BAPE_ToneControlEqType_eShelving,
    100,
    /* treble */
    0,
    10000,
    BAPE_ToneControlEqType_eShelving,
    10000
};

/********** Default Five Band Settings *******************/
static const BAPE_EqualizerFiveBandSettings deffiveBandSettings =
{
    0,0,0,0,0   /* Gains at 100,300,1000,3000 and 10000 hz respectively */
};

/********** Default Seven Band Settings *******************/
static const BAPE_EqualizerSevenBandSettings defsevenBandSettings = 
{
    /* bandSettings[7] */
    {
        {
            1000,   /* peak */
            0,      /* gain */
            100     /* q */
        },
        {
            1000,   /* peak */
            0,      /* gain */
            100     /* q */
        },
        {
            1000,   /* peak */
            0,      /* gain */
            100     /* q */
        },
        {
            1000,   /* peak */
            0,      /* gain */
            100     /* q */
        },
        {
            1000,   /* peak */
            0,      /* gain */
            100     /* q */
        },
        {
            1000,   /* peak */
            0,      /* gain */
            100     /* q */
        },
        {
            1000,   /* peak */
            0,      /* gain */
            100     /* q */
        }
    },
    7       /* windowStep */
};

/********** Default Subsonic Filter Settings *******************/
static const BAPE_EqualizerSubsonicSettings defsubsonicSettings = 
{
    0,
    BAPE_EqualizerFilterOrder_e4,
    BAPE_EqualizerFilterType_eButterworth
};

/********** Default Subwoofer Filter Settings *******************/
static const BAPE_EqualizerSubwooferSettings defsubwooferSettings = 
{
    0,
    BAPE_EqualizerFilterOrder_e4,
    BAPE_EqualizerFilterType_eButterworth
};

/********** Default Custom Equalizer Settings *******************/
static const BAPE_EqualizerCustomSettings defcustomSettings = 
{
    8,
    { /* filtercoefficients[8] */
        {
            0x4000000,
            0,0,0,0
        },
        {
            0x4000000,
            0,0,0,0
        },
        {
            0x4000000,
            0,0,0,0
        },
        {
            0x4000000,
            0,0,0,0
        },
        {
            0x4000000,
            0,0,0,0
        },
        {
            0x4000000,
            0,0,0,0
        },
        {
            0x4000000,
            0,0,0,0
        },
        {
            0x4000000,
            0,0,0,0
        }
    }
};


/*************************************************************************/
/* Allocate mixer resources */
static BERR_Code BAPE_Equalizer_P_AllocatePathFromInput(
    BAPE_PathNode *pNode,
    BAPE_PathConnection *pConnection
    );

/*************************************************************************/
/* Release mixer resources */
static void BAPE_Equalizer_P_FreePathFromInput(
    BAPE_PathNode *pNode,
    BAPE_PathConnection *pConnection
    );

/*************************************************************************/
static BERR_Code BAPE_Equalizer_P_ConfigPathFromInput(
    BAPE_PathNode *pNode, 
    BAPE_PathConnection *pConnection
    );

/*************************************************************************/
static BERR_Code BAPE_Equalizer_P_StartPathFromInput(
    BAPE_PathNode *pNode, 
    BAPE_PathConnection *pConnection
    );

/*************************************************************************/
static void BAPE_Equalizer_P_StopPathFromInput(
    BAPE_PathNode *pNode, 
    BAPE_PathConnection *pConnection
    );

/*************************************************************************/
static BERR_Code BAPE_Equalizer_P_AllocateResources(BAPE_EqualizerHandle equalizer, BAPE_PathConnection *pConnection);

/*************************************************************************/
static void BAPE_Equalizer_P_FreeResources(BAPE_EqualizerHandle handle);

/*************************************************************************/
static void BAPE_Equalizer_P_RemoveInputCallback(BAPE_PathNode *pNode, BAPE_PathConnector *pConnector);

/*************************************************************************/
static void BAPE_Equalizer_P_InputSampleRateChange_isr(
    struct BAPE_PathNode *pNode, 
    struct BAPE_PathConnection *pConnection,
    unsigned newSampleRate
    );

/*************************************************************************/
static void BAPE_EqualizerStage_P_GenerateCoefficients_isr(
                BAPE_EqualizerStageHandle   eqStage, 
                BAPE_SRC_P_IIRCoeff         *pCoeff);

/*************************************************************************/
static void BAPE_EqualizerStage_P_UpdateCoefficients_isr(
                BAPE_EqualizerStageHandle   eqStage,
                BAPE_SRC_P_IIRCoeff         *pCoeff);

/***************************************************************************
Summary:
Get default open settings for an equalizer stage
***************************************************************************/
void BAPE_EqualizerStage_GetDefaultSettings(
    BAPE_EqualizerStageType type,
    BAPE_EqualizerStageSettings *pSettings     /* [out] */
    )
{
    BDBG_ASSERT(pSettings);

    pSettings->type = type;
    pSettings->rampSettings.enable = false;
    pSettings->rampSettings.stepSize = 0xA;
    pSettings->bypassEnabled = false;
    
    switch(pSettings->type)
    {
        case BAPE_EqualizerStageType_eToneControl:
            pSettings->modeSettings.toneControl = deftoneControlsettings;
            break;
        case BAPE_EqualizerStageType_eFiveBand:
            pSettings->modeSettings.fiveBand = deffiveBandSettings;
            break;
        case BAPE_EqualizerStageType_eSevenBand:
            pSettings->modeSettings.sevenBand = defsevenBandSettings;
            break;
        case BAPE_EqualizerStageType_eSubsonic:
            pSettings->modeSettings.subsonic = defsubsonicSettings;
            break;
        case BAPE_EqualizerStageType_eSubwoofer:
            pSettings->modeSettings.subwoofer = defsubwooferSettings;
            break;
        case BAPE_EqualizerStageType_eCustom:
            pSettings->modeSettings.custom = defcustomSettings;
            break;
        default:
            BDBG_ERR(("Unsupported Equalizer Stage Type %d", pSettings->type));
            /*return BERR_NOT_SUPPORTED;*/
    }
    
}


BERR_Code BAPE_EqualizerStage_Create(
    BAPE_Handle deviceHandle,
    const BAPE_EqualizerStageSettings *pSettings, 
    BAPE_EqualizerStageHandle         *pHandle     /* [out] */
    )
{
    BAPE_EqualizerStageHandle   handle=NULL;
    BAPE_EqualizerStageSettings defaultSettings;
    
    BDBG_ASSERT(pHandle);
    BSTD_UNUSED(deviceHandle);
        
    handle = BKNI_Malloc(sizeof(BAPE_EqualizerStage));
    if(NULL == handle)
    {
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }
    BKNI_Memset(handle, 0, sizeof(BAPE_EqualizerStage));
    
    if ( NULL == pSettings )
    {
        BAPE_EqualizerStage_GetDefaultSettings(BAPE_EqualizerStageType_eToneControl, &defaultSettings);
        pSettings = &defaultSettings;
    }
    handle->settings = *pSettings;

    *pHandle = handle;
    
    return BERR_SUCCESS;    
}

void BAPE_EqualizerStage_Destroy(
    BAPE_EqualizerStageHandle handle
    )
{
    unsigned i;
    BDBG_ASSERT(handle);

    for(i=0; i<BAPE_P_MAX_EQSTAGE_INSTANCES; i++)
    {
        if(NULL != handle->srcGroups[i])
        {
            BDBG_ERR(("Equalizer Stage is still in use, so cannot be destroyed"));
            return;
        }
    }

    BKNI_Free(handle);
}

void BAPE_EqualizerStage_GetSettings(
    BAPE_EqualizerStageHandle handle,
    BAPE_EqualizerStageSettings *pSettings /* [out] */
    )
{
    BDBG_ASSERT(pSettings);

    *pSettings = handle->settings;
}

BERR_Code BAPE_EqualizerStage_SetSettings(
    BAPE_EqualizerStageHandle handle,
    const BAPE_EqualizerStageSettings *pSettings
    )
{
    unsigned i;

    if(pSettings->rampSettings.enable != handle->settings.rampSettings.enable)
    {
        BDBG_ERR(("Equalizer ramp enable cannot be changed on the fly."));
        return BERR_NOT_SUPPORTED;
    }
    handle->settings = *pSettings;
    
    for(i=0; i<BAPE_P_MAX_EQSTAGE_INSTANCES; i++)
    {
        if(NULL != handle->srcGroups[i])
        {
            break;
        }
    }
    if(i < BAPE_P_MAX_EQSTAGE_INSTANCES)
    {
        BAPE_SRC_P_IIRCoeff         coeff;
        
        /* Initialize to Bypass */
        BKNI_Memset(&coeff, 0, sizeof(BAPE_SRC_P_IIRCoeff));
        for(i=0; i<BAPE_CHIP_P_MAX_IIR_FILTERS_PER_SRC; i++)
        {    
            coeff.b0[i] = 0x02000000; /* Coefficients registers are 3.25 format */
        }

        BKNI_EnterCriticalSection();
        BAPE_EqualizerStage_P_GenerateCoefficients_isr(handle, &coeff); /* call from non-critical section once Book-keeping is implemented */
        BAPE_EqualizerStage_P_UpdateCoefficients_isr(handle, &coeff);
        BKNI_LeaveCriticalSection();
    }
    else
    {
        BDBG_MSG(("Equalizer Stage is not active, the settings are saved, not programmed")); 
    } 
        
    return BERR_SUCCESS;    
}

void BAPE_Equalizer_GetDefaultSettings(
    BAPE_EqualizerSettings *pSettings   /* [out] */
    )
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(BAPE_EqualizerSettings));
}

BERR_Code BAPE_Equalizer_Create(
    BAPE_Handle deviceHandle,
    const BAPE_EqualizerSettings *pSettings,
    BAPE_EqualizerHandle *pHandle               /* [out] */
    )
{
    BERR_Code errCode;
    BAPE_EqualizerHandle handle;
    BAPE_EqualizerSettings defaultSettings;
    BAPE_FMT_Descriptor format;
    BAPE_FMT_Capabilities caps;

    BDBG_OBJECT_ASSERT(deviceHandle, BAPE_Device);
    BDBG_ASSERT(NULL != pHandle);
    BSTD_UNUSED(defaultSettings);
    BSTD_UNUSED(pSettings);

    *pHandle = NULL;

    handle = BKNI_Malloc(sizeof(BAPE_Equalizer));
    if ( NULL == handle )
    {
        errCode = BERR_OUT_OF_SYSTEM_MEMORY;
        return BERR_TRACE(errCode);
    }

    BKNI_Memset(handle, 0, sizeof(BAPE_Equalizer));
    BDBG_OBJECT_SET(handle, BAPE_Equalizer);

    handle->deviceHandle = deviceHandle;
    BAPE_P_InitPathNode(&handle->pathNode, BAPE_PathNodeType_eEqualizer, 0, 1, deviceHandle, handle);
    handle->pathNode.pName = "Equalizer";

    BAPE_Connector_P_GetFormat(&handle->pathNode.connectors[0], &format);
    format.sampleRate = 0;
    format.source = BAPE_DataSource_eFci;
    format.type = BAPE_DataType_ePcmStereo;
    errCode = BAPE_Connector_P_SetFormat(&handle->pathNode.connectors[0], &format);
    if ( errCode ) { errCode = BERR_TRACE(errCode); goto err_format; }

    BAPE_PathNode_P_GetInputCapabilities(&handle->pathNode, &caps);
    BAPE_FMT_P_EnableSource(&caps, BAPE_DataSource_eFci);
    BAPE_FMT_P_EnableType(&caps, BAPE_DataType_ePcmStereo);
    errCode = BAPE_PathNode_P_SetInputCapabilities(&handle->pathNode, &caps);
    if ( errCode ) { errCode = BERR_TRACE(errCode); goto err_caps; }

    /* Fill in node connection routines */
    handle->pathNode.allocatePathFromInput = BAPE_Equalizer_P_AllocatePathFromInput;
    handle->pathNode.freePathFromInput = BAPE_Equalizer_P_FreePathFromInput;
    handle->pathNode.configPathFromInput = BAPE_Equalizer_P_ConfigPathFromInput;
    handle->pathNode.startPathFromInput = BAPE_Equalizer_P_StartPathFromInput;
    handle->pathNode.stopPathFromInput = BAPE_Equalizer_P_StopPathFromInput;
    handle->pathNode.inputSampleRateChange_isr = BAPE_Equalizer_P_InputSampleRateChange_isr;
    /*handle->pathNode.inputFormatChange = BAPE_StandardMixer_P_InputFormatChange;*/
    handle->pathNode.removeInput = BAPE_Equalizer_P_RemoveInputCallback;

    *pHandle = handle;

    /* TODO: Attach to device handle for resource tracking */
    
    return BERR_SUCCESS;

err_caps:
err_format:
    BAPE_Equalizer_Destroy(handle);

    return errCode;
}

void BAPE_Equalizer_Destroy(
    BAPE_EqualizerHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Equalizer); 

    BDBG_ASSERT(!handle->running);
    
    /* Remove all stages */
    BAPE_Equalizer_ClearStages(handle);
            
    /* Remove all inputs */
    BAPE_Equalizer_RemoveAllInputs(handle);

    /* Break any downstream connections */
    BAPE_Connector_P_RemoveAllConnections(&handle->pathNode.connectors[0]);

    #if 0 /* TODO */    
    /* Unlink from device */
    BLST_S_REMOVE(&handle->deviceHandle->equalizerList, handle, BAPE_Mixer, pathNode);
    #endif
    BDBG_OBJECT_DESTROY(handle, BAPE_Equalizer);
    BKNI_Free(handle);
}

BERR_Code BAPE_Equalizer_SetStages(
    BAPE_EqualizerHandle handle,
    BAPE_EqualizerStageHandle *pStages,
    unsigned numStages
    )
{
    unsigned i;
    
    BDBG_OBJECT_ASSERT(handle,BAPE_Equalizer);
    
    if ( numStages > 0 )
    {
        BDBG_ASSERT(pStages);
    }
    
    if ( handle->running )
    {
        BDBG_ERR(("Cannot change Equalizer stages while running"));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    if ( numStages > BAPE_MAX_STAGES_PER_EQUALIZER )
    {
        BDBG_ERR(("Exceeded maximum number of stages per Equalizer %u on this chipset", BAPE_MAX_STAGES_PER_EQUALIZER));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* Release stored resources */
    BAPE_Equalizer_P_FreeResources(handle);
    
    /* Invalidate all stage handles */
    BKNI_Memset(handle->eqStage, 0, sizeof(handle->eqStage));    
    /* Copy stage Handles */
    for ( i = 0; i < numStages; i++ )
    {
        handle->eqStage[i] = pStages[i];
    }
    handle->numCascadeSrcs = numStages;    /* TODO: Is this accurate?  Seems like this may be able to pack some stages (e.g. TC + Subwoofer or TC + Subsonic) */
    
    return BERR_SUCCESS;
}

void BAPE_Equalizer_ClearStages(
    BAPE_EqualizerHandle handle
    )
{
    (void)BAPE_Equalizer_SetStages(handle, NULL, 0);
}

BERR_Code BAPE_Equalizer_AddInput(
    BAPE_EqualizerHandle handle,
    BAPE_Connector input
    )
{
    BERR_Code errCode;
    BDBG_OBJECT_ASSERT(handle, BAPE_Equalizer);
    BDBG_OBJECT_ASSERT(input, BAPE_PathConnector);

    if ( NULL != handle->input )
    {
        BDBG_ERR(("Can not have more than one input"));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    errCode = BAPE_PathNode_P_AddInput(&handle->pathNode, input);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    handle->input = input;
    return BERR_SUCCESS;    
}

BERR_Code BAPE_Equalizer_RemoveInput(
    BAPE_EqualizerHandle handle,
    BAPE_Connector input
    )
{
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(handle, BAPE_Equalizer);
    BDBG_OBJECT_ASSERT(input, BAPE_PathConnector);

    if ( input != handle->input )
    {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    
    /* Make sure we remove all resources */
    BAPE_Equalizer_P_FreeResources(handle);
    errCode = BAPE_PathNode_P_RemoveInput(&handle->pathNode, input);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    handle->input = NULL;
    return BERR_SUCCESS;
}

BERR_Code BAPE_Equalizer_RemoveAllInputs(
    BAPE_EqualizerHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Equalizer);

    if ( handle->input )
    {
        return BAPE_Equalizer_RemoveInput(handle, handle->input);
    }
    return BERR_SUCCESS;
}

void BAPE_Equalizer_GetConnector(
    BAPE_EqualizerHandle handle,
    BAPE_Connector *pConnector     /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Equalizer);
    BDBG_ASSERT(NULL != pConnector);
    *pConnector = &handle->pathNode.connectors[0];
}


/*************************************************************************/
/*          Private Routines                                             */
/*************************************************************************/

/*************************************************************************/
/* Allocate mixer resources */
static BERR_Code BAPE_Equalizer_P_AllocatePathFromInput(
    BAPE_PathNode *pNode,
    BAPE_PathConnection *pConnection
    )
{
    BAPE_EqualizerHandle handle;
    BERR_Code errCode;
    unsigned i;

    BDBG_OBJECT_ASSERT(pNode, BAPE_PathNode);
    
    handle = pNode->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_Equalizer);

    /* Grab SRC resources */
    errCode = BAPE_Equalizer_P_AllocateResources(handle, pConnection);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

	/** This will set the src coefficients with the saved settings ***/
	for(i=0; i< handle->numCascadeSrcs; i++)
	{
		BAPE_EqualizerStage_SetSettings(handle->eqStage[i],&(handle->eqStage[i]->settings));

	}

    return BERR_SUCCESS;
}

/*************************************************************************/
/* Release mixer resources */
static void BAPE_Equalizer_P_FreePathFromInput(
    BAPE_PathNode *pNode,
    BAPE_PathConnection *pConnection
    )
{
    BAPE_EqualizerHandle handle;

    BDBG_OBJECT_ASSERT(pNode, BAPE_PathNode);
    BSTD_UNUSED(pConnection);
    
    handle = pNode->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_Equalizer);

    /* Free mixer-level resources if needed  */
    BAPE_Equalizer_P_FreeResources(handle);
}

/*************************************************************************/
static BERR_Code BAPE_Equalizer_P_ConfigPathFromInput(
    BAPE_PathNode *pNode, 
    BAPE_PathConnection *pConnection
    )
{
    BAPE_SrcGroupSettings srcSettings;
    BERR_Code errCode;
    
    BAPE_EqualizerHandle handle;

    BDBG_OBJECT_ASSERT(pNode, BAPE_PathNode);

    handle = pNode->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_Equalizer);

    BDBG_ENTER(BAPE_Equalizer_P_ConfigPathFromInput);

    /* Make sure we have allocated at least one SRC */
    if ( NULL == handle->src[0] )
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    
    /* Set the first SRC's input */
    BAPE_SrcGroup_P_GetSettings(handle->src[0], &srcSettings);
    BAPE_FciIdGroup_Init(&srcSettings.input);
        
    /* This should set the input to the first SRC */
    switch ( pConnection->pSource->format.source )
    {
    case BAPE_DataSource_eFci:  /* Mixer/EQ */
        srcSettings.input = pConnection->inputFciGroup;
        break;
    default:
        /* Other connection types are not currently supported.  Ideally, we should support a buffer source and allocate sfifos if required. */
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    
    errCode = BAPE_SrcGroup_P_SetSettings(handle->src[0], &srcSettings);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }    

#if 0 /* Book keep Coeff for 32,44.1 and 48khz */   
    /* Update SRC Coefficients */
    BKNI_EnterCriticalSection();
    BAPE_Equalizer_P_UpdateCoefficients_isr(handle);
    BKNI_LeaveCriticalSection();
#endif    
    
    BDBG_LEAVE(BAPE_Equalizer_P_ConfigPathFromInput);
    return BERR_SUCCESS;
}

/*************************************************************************/
static BERR_Code BAPE_Equalizer_P_StartPathFromInput(
    BAPE_PathNode *pNode, 
    BAPE_PathConnection *pConnection
    )
{
    int srcCascadeIndex;
    BAPE_EqualizerHandle handle;
    BAPE_Connector input;
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(pNode, BAPE_PathNode);

    BDBG_ENTER(BAPE_Equalizer_P_StartPathFromInput);
    handle = pNode->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_Equalizer);
    input = pConnection->pSource;
    BDBG_OBJECT_ASSERT(input, BAPE_PathConnector);

    BDBG_MSG(("Input %s, %s starting", input->pParent->pName, input->pName));  

    BKNI_EnterCriticalSection();
    BAPE_Equalizer_P_InputSampleRateChange_isr(pNode, pConnection,pConnection->pSource->format.sampleRate );
    BKNI_LeaveCriticalSection();

    /* Start SRCs in consumer..producer order */
    for ( srcCascadeIndex = (int)handle->numCascadeSrcs-1; srcCascadeIndex >= 0; srcCascadeIndex-- )
    {
        errCode = BAPE_SrcGroup_P_Start(handle->src[srcCascadeIndex]);
        if ( errCode )
        {
            errCode = BERR_TRACE(errCode);
            goto err_src;
        }
    } 

    handle->running = true;
    
    BDBG_LEAVE(BAPE_Equalizer_P_StartPathFromInput);
    return BERR_SUCCESS;

err_src:
    BAPE_Equalizer_P_StopPathFromInput(pNode, pConnection);
             
    return errCode;
}

/*************************************************************************/
static void BAPE_Equalizer_P_StopPathFromInput(
    BAPE_PathNode *pNode, 
    BAPE_PathConnection *pConnection
    )
{
    unsigned srcCascadeIndex;
    BAPE_EqualizerHandle handle;
    BAPE_Connector input;

    BDBG_OBJECT_ASSERT(pNode, BAPE_PathNode);

    BDBG_ENTER(BAPE_Equalizer_P_StopPathFromInput);
    handle = pNode->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_Equalizer);
    input = pConnection->pSource;
    BDBG_OBJECT_ASSERT(input, BAPE_PathConnector);

    BDBG_MSG(("Input %s, %s stopping", input->pParent->pName, input->pName));  

    /* Sop SRCs in producer..consumer order */
    for ( srcCascadeIndex = 0; srcCascadeIndex < handle->numCascadeSrcs;srcCascadeIndex++ )
    {
        BAPE_SrcGroup_P_Stop(handle->src[srcCascadeIndex]);
    } 

    handle->running = false;
    
    BDBG_LEAVE(BAPE_Equalizer_P_StopPathFromInput);
}

/*************************************************************************/
static void BAPE_Equalizer_P_RemoveInputCallback(BAPE_PathNode *pNode, BAPE_PathConnector *pConnector)
{
    (void)BAPE_Equalizer_RemoveInput(pNode->pHandle, pConnector);
}

/*************************************************************************/
static BERR_Code BAPE_Equalizer_P_AllocateResources(BAPE_EqualizerHandle equalizer, BAPE_PathConnection *pConnection)
{
    unsigned numChannelPairs=0,i=0, j=0;
    BAPE_SrcGroupSettings       srcSettings;    
    BAPE_SrcGroupCreateSettings srcCreateSettings;
    BAPE_FMT_Descriptor format;
    BERR_Code errCode=BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(equalizer, BAPE_Equalizer);

    if ( equalizer->resourcesAllocated )
    {
        BDBG_MSG(("equalizer %p already has resources allocated", (void *)equalizer));
        return BERR_SUCCESS;
    }

    numChannelPairs = BAPE_FMT_P_GetNumChannelPairs_isrsafe(&pConnection->pSource->format);
    
    BAPE_Connector_P_GetFormat(pConnection->pSource, &format);
    errCode = BAPE_Connector_P_SetFormat(&equalizer->pathNode.connectors[0], &format);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    BDBG_MSG(("equalizer %p : Allocating equalizer with %u channel pairs.", 
              (void *)equalizer, numChannelPairs));

    BAPE_SrcGroup_P_GetDefaultCreateSettings(&srcCreateSettings);
    srcCreateSettings.numChannelPairs = numChannelPairs;
    
    /* Shallow EQ object - Allocate one SRC in Bypass */
    if( NULL == equalizer->eqStage[0] )
    {
        equalizer->numCascadeSrcs = 1;
        srcCreateSettings.mode = BAPE_SrcMode_eBypass;
    }
    else
    {
        srcCreateSettings.mode = BAPE_SrcMode_eIir;    
    }
    
    for(i=0; i < equalizer->numCascadeSrcs; i++)
    {
        if(BAPE_SrcMode_eIir == srcCreateSettings.mode)
        {
            srcCreateSettings.equalizerSettings.rampEnable = equalizer->eqStage[i]->settings.rampSettings.enable;
        }

        /* Only one SRC Group per EQ object supported */
        /* Allocate required number of SRCs */
        errCode = BAPE_SrcGroup_P_Create(equalizer->deviceHandle, &srcCreateSettings, &equalizer->src[i]);
        if ( errCode )
        {
            (void)BERR_TRACE(errCode);
            goto err_src_create;
        }

        if(equalizer->eqStage[i])
        {
            for(j=0; j<BAPE_P_MAX_EQSTAGE_INSTANCES; j++)
            {
                if(NULL == equalizer->eqStage[i]->srcGroups[j])
                {
                    equalizer->eqStage[i]->srcGroups[j] = equalizer->src[i];
                    break;
                }
            }
            if(j >= BAPE_P_MAX_EQSTAGE_INSTANCES)
            {
                BDBG_ERR(("The number of SrcGroup handle entries in EQ Stage already reached maximum."
                    "Check if the entries are not cleared upon Free resources"));
            }
        }

        /* Setup settings and Cascade Linkage */
        BAPE_SrcGroup_P_GetSettings(equalizer->src[i], &srcSettings);
        srcSettings.rampEnabled = false;
        srcSettings.startupRampEnabled = false;

        if ( i > 0 )
        {            
            BAPE_SrcGroup_P_GetOutputFciIds(equalizer->src[i-1], &srcSettings.input);
        }
        /* TODO: Coef memory */
        errCode = BAPE_SrcGroup_P_SetSettings(equalizer->src[i], &srcSettings);
        if ( errCode )
        {
            (void)BERR_TRACE(errCode);
            goto err_src_settings;
        }        
    }
    equalizer->resourcesAllocated = true;

#if 1   /* JDG: We should review this.  It should really be in AllocatePathToOutput but that might require some bape_path_priv or bape_mixer_standard changes. */
    i = equalizer->numCascadeSrcs - 1;
    /* Setup Downstream connections as well */
    for ( pConnection = BLST_SQ_FIRST(&equalizer->pathNode.connectors[0].connectionList);
          pConnection != NULL;
          pConnection = BLST_SQ_NEXT(pConnection, downstreamNode) )
    {
        BAPE_SrcGroup_P_GetOutputFciIds(equalizer->src[i], &pConnection->inputFciGroup);
    }
#endif          

    return BERR_SUCCESS;

err_src_create:
err_src_settings:
    for ( i = 0; i < equalizer->numCascadeSrcs; i++ )
    {
        if ( equalizer->src[i] )
        {
            if(equalizer->eqStage[i])
            {
                for(j=0; j<BAPE_P_MAX_EQSTAGE_INSTANCES; j++)
                {
                    if(equalizer->src[i] == equalizer->eqStage[i]->srcGroups[j])
                    {
                        equalizer->eqStage[i]->srcGroups[j] = NULL;
                    }
                }
            }         
            BAPE_SrcGroup_P_Destroy(equalizer->src[i]);
            equalizer->src[i] = NULL;
        }
    }    
    BAPE_Equalizer_P_FreeResources(equalizer);

    return BERR_SUCCESS;    
}

/*************************************************************************/
static void BAPE_Equalizer_P_FreeResources(BAPE_EqualizerHandle equalizer)
{
    BAPE_PathConnection *pConnection;
    unsigned i,j;
    
    BSTD_UNUSED(equalizer);

    if ( equalizer->running )
    {
        BDBG_ERR(("Can't release resources while equalizer %p is running", (void *)equalizer));
        BDBG_ASSERT(!equalizer->running);
        return;
    }

    if ( !equalizer->resourcesAllocated )
    {
        /* Nothing to do */
        return;
    }

    for ( i = 0; i < BAPE_P_MAX_CASCADED_SRCS_PER_EQUALIZER; i++ )
    {
        if ( equalizer->src[i] )
        {
            if(equalizer->eqStage[i])
            {
                for(j=0; j<BAPE_P_MAX_EQSTAGE_INSTANCES; j++)
                {
                    if(equalizer->src[i] == equalizer->eqStage[i]->srcGroups[j])
                    {
                        equalizer->eqStage[i]->srcGroups[j] = NULL;
                    }
                }
            }        
            BAPE_SrcGroup_P_Destroy(equalizer->src[i]);  
            equalizer->src[i] = NULL;
        }
    }    
    equalizer->resourcesAllocated = false;
    
    /* JDG: We should review this as well. */        
    /* Invalidate Downstream connections as well */
    for ( pConnection = BLST_SQ_FIRST(&equalizer->pathNode.connectors[0].connectionList);
          pConnection != NULL;
          pConnection = BLST_SQ_NEXT(pConnection, downstreamNode) )
    {
        BAPE_FciIdGroup_Init(&pConnection->inputFciGroup);
    }

    equalizer->resourcesAllocated = false;    
}

/*************************************************************************/
static void BAPE_Equalizer_P_InputSampleRateChange_isr(
    struct BAPE_PathNode *pNode, 
    struct BAPE_PathConnection *pConnection,
    unsigned newSampleRate
    )
{
    unsigned i;
    BAPE_EqualizerHandle handle;
    BAPE_SRC_P_IIRCoeff  coeff;

    BDBG_OBJECT_ASSERT(pNode, BAPE_PathNode);
    BKNI_ASSERT_ISR_CONTEXT();

    handle = pNode->pHandle;    
    BSTD_UNUSED(pConnection);

    /* Initialize to Bypass */
    BKNI_Memset(&coeff, 0, sizeof(BAPE_SRC_P_IIRCoeff));

	for(i=0; i<BAPE_CHIP_P_MAX_IIR_FILTERS_PER_SRC; i++)
    {    
        coeff.b0[i] = 0x02000000; /* Coefficients registers are 3.25 format */
    }    
    
    pConnection->format.sampleRate = newSampleRate;
    for(i=0; i<BAPE_MAX_STAGES_PER_EQUALIZER; i++)
    {
        if(NULL == handle->eqStage[i])
            break;
        handle->eqStage[i]->sampleRate = newSampleRate;

        BAPE_EqualizerStage_P_GenerateCoefficients_isr(handle->eqStage[i], &coeff); /* Remove once Book keeping is implemented */
        BAPE_EqualizerStage_P_UpdateCoefficients_isr(handle->eqStage[i], &coeff);
    }

    BAPE_Connector_P_SetSampleRate_isr(&pNode->connectors[0], newSampleRate);
    
}

static void BAPE_EqualizerStage_P_GenerateCoefficients_isr(
                BAPE_EqualizerStageHandle   eqStage,
                BAPE_SRC_P_IIRCoeff         *pCoeff)
{
    unsigned i=0;
    int32_t geqdB[5];
    int32_t b[3], a[3]; /* to hold b0, b1, b2, a1, a2 */
        
    BDBG_ASSERT(NULL != eqStage);

    geqdB[0] = geqdB[1] = geqdB[2] = geqdB[3] = geqdB[4] = 0;

    for(i=0; i<BAPE_P_MAX_EQSTAGE_INSTANCES; i++)
    {
        if(NULL != eqStage->srcGroups[i])
        {
            break;
        }
    }
    if(i >= BAPE_P_MAX_EQSTAGE_INSTANCES)
    {
        BDBG_ERR(("No valid SRC handle in EQ handle %p", (void *)eqStage));
    }

    BDBG_MSG(("Equalizer Stage Type %d, Sample Rate %u",eqStage->settings.type, eqStage->sampleRate));    
    switch(eqStage->settings.type)
    {
        case BAPE_EqualizerStageType_eToneControl:
           
            /* Cut off Freq of Bass and Treble hardcoded to 100 and 10000 for 
                now */
            BDBG_MSG(("\n bassGain = %d trebleGain = %d \n bassFreq = %u trebleFreq = %u"
            "\n bassEqType = %d trebleEqType = %d \n bassBandwidthFreq = %u trebleBandwidthFreq = %u",
                eqStage->settings.modeSettings.toneControl.bassGain,                 
                eqStage->settings.modeSettings.toneControl.trebleGain,
                eqStage->settings.modeSettings.toneControl.bassFreq,
                eqStage->settings.modeSettings.toneControl.trebleFreq,
                eqStage->settings.modeSettings.toneControl.bassEqType,
                eqStage->settings.modeSettings.toneControl.trebleEqType,
                eqStage->settings.modeSettings.toneControl.bassBandwidthFreq,
                eqStage->settings.modeSettings.toneControl.trebleBandwidthFreq));
            
            EQ_generate_tone_control_isrsafe (
                eqStage->sampleRate, 
                (eqStage->settings.modeSettings.toneControl.bassGain)/10,                 
                (eqStage->settings.modeSettings.toneControl.trebleGain)/10,                 
                eqStage->settings.modeSettings.toneControl.bassFreq,
                eqStage->settings.modeSettings.toneControl.trebleFreq,
                eqStage->settings.modeSettings.toneControl.bassEqType,
                eqStage->settings.modeSettings.toneControl.trebleEqType,
                eqStage->settings.modeSettings.toneControl.bassBandwidthFreq,
                eqStage->settings.modeSettings.toneControl.trebleBandwidthFreq,
                &pCoeff->b0[0], 
                &pCoeff->b1[0], 
                &pCoeff->b2[0], 
                &pCoeff->a1[0], 
                &pCoeff->a2[0]
                );
            break;
        case BAPE_EqualizerStageType_eFiveBand:
        
            geqdB[0] = (eqStage->settings.modeSettings.fiveBand.gain100Hz)/10;
            geqdB[1] = (eqStage->settings.modeSettings.fiveBand.gain300Hz)/10;
            geqdB[2] = (eqStage->settings.modeSettings.fiveBand.gain1000Hz)/10;
            geqdB[3] = (eqStage->settings.modeSettings.fiveBand.gain3000Hz)/10;
            geqdB[4] = (eqStage->settings.modeSettings.fiveBand.gain10000Hz)/10; 

            BDBG_MSG(("Gain at Frequencies: \n 100: %d \n 300: %d \n 1000: %d \n 3000: %d \n 10000: %d",
                eqStage->settings.modeSettings.fiveBand.gain100Hz,eqStage->settings.modeSettings.fiveBand.gain300Hz,
                eqStage->settings.modeSettings.fiveBand.gain1000Hz,
                eqStage->settings.modeSettings.fiveBand.gain3000Hz,
                eqStage->settings.modeSettings.fiveBand.gain10000Hz));

          
            EQ_generate_geq_isrsafe (
                eqStage->sampleRate, 
                geqdB, 
                &pCoeff->b0[0], 
                &pCoeff->b1[0], 
                &pCoeff->b2[0], 
                &pCoeff->a1[0], 
                &pCoeff->a2[0]
                );
        break;
        
        case BAPE_EqualizerStageType_eSevenBand:    
            for (i=0;i<7;i++)
        	{
                EQ_generate_peq_isrsafe(
                    eqStage->settings.modeSettings.sevenBand.bandSettings[i].peak,
                    eqStage->settings.modeSettings.sevenBand.bandSettings[i].q,
                    (eqStage->settings.modeSettings.sevenBand.bandSettings[i].gain)/10,
                    eqStage->sampleRate,
                    70, /* Percentage of Peak Edge to Peak Gain */
                    b,
                    a
                    );
        		pCoeff->b0[i] = b[0];
        		pCoeff->b1[i] = b[1];
        		pCoeff->b2[i] = b[2];
        		pCoeff->a1[i] = a[1];
        		pCoeff->a2[i] = a[2];
            }
            break;            
            
        case BAPE_EqualizerStageType_eSubsonic:

            EQ_subsonic_HPF_isrsafe(
                eqStage->sampleRate, 
                eqStage->settings.modeSettings.subsonic.frequency,
                eqStage->settings.modeSettings.subsonic.filterType,
                eqStage->settings.modeSettings.subsonic.filterOrder,
                &pCoeff->b0[0], 
                &pCoeff->b1[0], 
                &pCoeff->b2[0], 
                &pCoeff->a1[0], 
                &pCoeff->a2[0]
                );
            break;
            
        case BAPE_EqualizerStageType_eSubwoofer:    

            EQ_subwoofer_LPF_isrsafe(
                eqStage->sampleRate, 
                eqStage->settings.modeSettings.subwoofer.frequency,
                eqStage->settings.modeSettings.subwoofer.filterType,
                eqStage->settings.modeSettings.subwoofer.filterOrder,
                &pCoeff->b0[0], 
                &pCoeff->b1[0], 
                &pCoeff->b2[0], 
                &pCoeff->a1[0], 
                &pCoeff->a2[0]
                );
            break;   
            
        case BAPE_EqualizerStageType_eCustom:    

            {
                BAPE_EqualizerCustomSettings    *pcustomSettings;

                pcustomSettings = &(eqStage->settings.modeSettings.custom);
                if(pcustomSettings->numFilters > BAPE_EQUALIZER_MAX_CUSTOM_FILTERS)
                {
                    BDBG_ERR(("Number of Custom EQ filters cannot be "
                              "more than %d. No programming done.", 
                              BAPE_EQUALIZER_MAX_CUSTOM_FILTERS));
                }

                for (i=0; i < pcustomSettings->numFilters; i++)
                {
        
                    /* The 35230 SRC hardware has 28 bits of resolution for the coef
                       in 3.25 format (28 bits + 4unused). Earlier (in 3548) though 
                       the total bits were 28, the format was 2.26. Hence the validity
                       for the coeff range will be from 0xF0000000 - 0x0FFFFFFF */
                    if(((pcustomSettings->filterCoefficients[i].b0 < (int32_t) 0xF0000000) ||
                        (pcustomSettings->filterCoefficients[i].b0 > (int32_t) 0x0FFFFFFF))||
                       ((pcustomSettings->filterCoefficients[i].b1 < (int32_t) 0xF0000000) ||
                        (pcustomSettings->filterCoefficients[i].b1 > (int32_t) 0x0FFFFFFF))||
                       ((pcustomSettings->filterCoefficients[i].b2 < (int32_t) 0xF0000000) ||
                        (pcustomSettings->filterCoefficients[i].b2 > (int32_t) 0x0FFFFFFF))||
                       ((pcustomSettings->filterCoefficients[i].a1 < (int32_t) 0xF0000000) ||
                        (pcustomSettings->filterCoefficients[i].a1 > (int32_t) 0x0FFFFFFF))||
                       ((pcustomSettings->filterCoefficients[i].a2 < (int32_t) 0xF0000000) ||               
                        (pcustomSettings->filterCoefficients[i].a2 > (int32_t) 0x0FFFFFFF)))
                    {
                        BDBG_ERR(("One or more of Custom EQ settings are out of bounds."
                                  "No programming done."));
                    }
                }
                
                for (i=0; i < pcustomSettings->numFilters; i++)
                {
                    /* User coeff in 2.26 --> SRC-EQ in 35230 is in 3.25 format 
                       Hence divide by 2 */
                    pCoeff->b0[i] = pcustomSettings->filterCoefficients[i].b0/2;
                    pCoeff->b1[i] = pcustomSettings->filterCoefficients[i].b1/2;
                    pCoeff->b2[i] = pcustomSettings->filterCoefficients[i].b2/2;
                    pCoeff->a1[i] = pcustomSettings->filterCoefficients[i].a1/2;
                    pCoeff->a2[i] = pcustomSettings->filterCoefficients[i].a2/2;
                } 
            }
            break;  
            
        default:
            return;
    }
}

static void BAPE_EqualizerStage_P_UpdateCoefficients_isr(
                BAPE_EqualizerStageHandle   eqStage,
                BAPE_SRC_P_IIRCoeff         *pCoeff)
{
    unsigned i, *pStepSize = NULL;

    if((BAPE_EqualizerStageType_eSevenBand != eqStage->settings.type) &&
       (BAPE_EqualizerStageType_eCustom != eqStage->settings.type))
    {
        /* Divide by 4 the first Bi-Quad */
        pCoeff->b0[0] /= (-4);
        pCoeff->b1[0] /= (-4);
        pCoeff->b2[0] /= (-4);

        /* Multiply by 4 the last Bi-Quad */
        pCoeff->b0[7] = 0x8000000;
        pCoeff->b1[7] = 0;
        pCoeff->b2[7] = 0;
        pCoeff->a1[7] = 0;
        pCoeff->a2[7] = 0;  
    }
        
    /* Program Coefficients for all Paths(SRCs) that have this EQ stage operation.
       In future if Coefficient memory is shared, control the programming here,
       i.e., if Coefficient memory is updated then program only SRC link to it */
    if(eqStage->settings.rampSettings.enable)   
    {
        pStepSize = &(eqStage->settings.rampSettings.stepSize);
    }
    for(i=0; i<BAPE_P_MAX_EQSTAGE_INSTANCES; i++)
    {
        if(NULL != eqStage->srcGroups[i])
        {
        	BAPE_SrcGroup_P_UpdateCoefficients_isr(eqStage->srcGroups[i], pCoeff, pStepSize);
        }
    }    
    
}

#else

/***************************************************************************
Summary:
Get default open settings for an equalizer stage
***************************************************************************/
void BAPE_EqualizerStage_GetDefaultSettings(
    BAPE_EqualizerStageType type,
    BAPE_EqualizerStageSettings *pSettings     /* [out] */
    )
{
    BSTD_UNUSED(type);
    BSTD_UNUSED(pSettings);
}

BERR_Code BAPE_EqualizerStage_Create(
    BAPE_Handle deviceHandle,
    const BAPE_EqualizerStageSettings *pSettings, 
    BAPE_EqualizerStageHandle         *pHandle     /* [out] */
    )
{
    BSTD_UNUSED(deviceHandle);
    BSTD_UNUSED(pSettings);
    BSTD_UNUSED(pHandle);    
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

void BAPE_EqualizerStage_Destroy(
    BAPE_EqualizerStageHandle handle
    )
{
    BSTD_UNUSED(handle);
}

void BAPE_EqualizerStage_GetSettings(
    BAPE_EqualizerStageHandle handle,
    BAPE_EqualizerStageSettings *pSettings /* [out] */
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);    
}

BERR_Code BAPE_EqualizerStage_SetSettings(
    BAPE_EqualizerStageHandle handle,
    const BAPE_EqualizerStageSettings *pSettings
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);    
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

void BAPE_Equalizer_GetDefaultSettings(
    BAPE_EqualizerSettings *pSettings   /* [out] */
    )
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(BAPE_EqualizerSettings));
}

BERR_Code BAPE_Equalizer_Create(
    BAPE_Handle deviceHandle,
    const BAPE_EqualizerSettings *pSettings,
    BAPE_EqualizerHandle *pHandle               /* [out] */
    )
{
    BSTD_UNUSED(deviceHandle);
    BSTD_UNUSED(pSettings);    
    BSTD_UNUSED(pHandle);        
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

void BAPE_Equalizer_Destroy(
    BAPE_EqualizerHandle handle
    )
{
    BSTD_UNUSED(handle);    
}
BERR_Code BAPE_Equalizer_SetStages(
    BAPE_EqualizerHandle handle,
    BAPE_EqualizerStageHandle *pStages,
    unsigned numStages
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pStages);    
    BSTD_UNUSED(numStages);  
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

void BAPE_Equalizer_ClearStages(
    BAPE_EqualizerHandle handle
    )
{
    BSTD_UNUSED(handle);
}

BERR_Code BAPE_Equalizer_AddInput(
    BAPE_EqualizerHandle handle,
    BAPE_Connector input
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(input);    
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

BERR_Code BAPE_Equalizer_RemoveInput(
    BAPE_EqualizerHandle handle,
    BAPE_Connector input
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(input);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

BERR_Code BAPE_Equalizer_RemoveAllInputs(
    BAPE_EqualizerHandle handle
    )
{
    BSTD_UNUSED(handle);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

void BAPE_Equalizer_GetConnector(
    BAPE_EqualizerHandle handle,
    BAPE_Connector *pConnector     /* [out] */
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pConnector);
}
#endif

