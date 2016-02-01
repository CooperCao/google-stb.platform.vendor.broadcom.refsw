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
 * Module Description: Mixer Interface
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#include "bstd.h"
#include "bkni.h"
#include "bape.h"
#include "bape_priv.h"

BDBG_MODULE(bape_mixer);

BDBG_OBJECT_ID(BAPE_Mixer);
BDBG_OBJECT_ID(BAPE_OutputPort);

/*************************************************************************/
/* Generic mixer functions (independent of mixer type)                   */
/*************************************************************************/
void BAPE_Mixer_GetDefaultSettings(
    BAPE_MixerSettings *pSettings   /* [out] */
    )
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->defaultSampleRate = 48000;
    #if BAPE_CHIP_MAX_NCOS > 0
    pSettings->outputNco = BAPE_Nco_e0;
    #else
    pSettings->outputNco = BAPE_Nco_eMax;       /* eMax => Don't use an NCO  */
    #endif
}

/*************************************************************************/
void BAPE_Mixer_GetDefaultAddInputSettings(
    BAPE_MixerAddInputSettings *pSettings   /* [out] */
    )
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
}

/*************************************************************************/
unsigned BAPE_Mixer_P_FindInputIndex_isrsafe(BAPE_MixerHandle handle, BAPE_Connector input)
{
    unsigned i;

    for ( i = 0; i < BAPE_CHIP_MAX_MIXER_INPUTS; i++ )
    {
        if ( handle->inputs[i] == input )
        {
            return i;
        }
    }

    return BAPE_MIXER_INPUT_INDEX_INVALID;
}

/*************************************************************************/
void BAPE_Mixer_P_GetDefaultInputVolume(BAPE_MixerInputVolume *pVolume)
{
    unsigned i;
    BKNI_Memset(pVolume, 0, sizeof(*pVolume));
    for ( i = 0; i < BAPE_Channel_eMax; i++ )
    {
        pVolume->coefficients[i][i] = BAPE_VOLUME_NORMAL;
    }
}

/*************************************************************************/
/* Some functions more related to outputs than mixers.                   */
/*************************************************************************/
BERR_Code BAPE_GetOutputVolume(
    BAPE_OutputPort output,
    BAPE_OutputVolume *pVolume      /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(output, BAPE_OutputPort);
    BDBG_ASSERT(NULL != pVolume);
    
    *pVolume = output->volume;
    return BERR_SUCCESS;
}

/*************************************************************************/
BERR_Code BAPE_SetOutputVolume(
    BAPE_OutputPort output,
    const BAPE_OutputVolume *pVolume
    )
{
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(output, BAPE_OutputPort);
    BDBG_ASSERT(NULL != pVolume);

    output->volume = *pVolume;
    if ( output->mixer && output->mixer->running )
    {
        errCode = BAPE_Mixer_P_ApplyOutputVolume(output->mixer, output);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    }

    return BERR_SUCCESS;
}

/*************************************************************************/
void BAPE_GetOutputDelayStatus(
    BAPE_OutputPort output,
    BAPE_OutputDelayStatus *pStatus     /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(output, BAPE_OutputPort);
    BDBG_ASSERT(NULL != pStatus);

    pStatus->pathDelay = BAPE_CHIP_MAX_PATH_DELAY;
    pStatus->additionalDelay = (output->mixer)?output->mixer->settings.outputDelay:0;    
}   

/*************************************************************************/
/* Dispatchers for type-specific mixer functions                         */
/*************************************************************************/
BERR_Code BAPE_Mixer_Create(
    BAPE_Handle deviceHandle,
    const BAPE_MixerSettings *pSettings,
    BAPE_MixerHandle *pHandle               /* [out] */
    )
{
    BERR_Code errCode;
    BAPE_MixerSettings defaultSettings;

    BDBG_OBJECT_ASSERT(deviceHandle, BAPE_Device);
    BDBG_ASSERT(NULL != pHandle);

    *pHandle = NULL;

    if ( NULL == pSettings )
    {
        BAPE_Mixer_GetDefaultSettings(&defaultSettings);
        pSettings = &defaultSettings;
    }

    switch ( pSettings->type )
    {
    #if BAPE_CHIP_MAX_MIXERS
    case BAPE_MixerType_eStandard:
        errCode = BAPE_StandardMixer_P_Create( deviceHandle, pSettings, pHandle );
        break;
    #endif
        
    #if BAPE_CHIP_MAX_BYPASS_MIXERS
    case BAPE_MixerType_eBypass:
        errCode = BAPE_BypassMixer_P_Create( deviceHandle, pSettings, pHandle );
        break;
    #endif
        
    #if BAPE_CHIP_MAX_DSP_MIXERS
    case BAPE_MixerType_eDsp:
        errCode = BAPE_DspMixer_P_Create( deviceHandle, pSettings, pHandle );
        break;
    #endif
    
    default:
        BDBG_ERR(("MixerType %u is invalid or not supported", pSettings->type ));
        errCode = BERR_INVALID_PARAMETER;
        break;
    }

    return BERR_TRACE(errCode);
}

/*************************************************************************/
void BAPE_Mixer_Destroy(
    BAPE_MixerHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Mixer);
    BDBG_ASSERT(NULL != handle->interface);

    if(handle->interface->destroy) {
        handle->interface->destroy(handle);
    } else {
        (void) BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

/**************************************************************************/
void BAPE_Mixer_GetSettings(
    BAPE_MixerHandle hMixer,
    BAPE_MixerSettings *pSettings   /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(hMixer, BAPE_Mixer);
    BDBG_ASSERT(NULL != pSettings);
    *pSettings = hMixer->settings;
}

/**************************************************************************/
BERR_Code BAPE_Mixer_SetSettings(
    BAPE_MixerHandle hMixer,
    const BAPE_MixerSettings *pSettings
    )
{
    BDBG_OBJECT_ASSERT(hMixer, BAPE_Mixer);
    BDBG_ASSERT(NULL != pSettings);
    BDBG_ASSERT(NULL != hMixer->interface);

#if 0
    if ( hMixer->running && hMixer->settings.type != BAPE_MixerType_eDsp )
    {
        BDBG_ERR(("Cannot change mixer settings while running.  Please stop all inputs."));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
#endif

    if ( hMixer->settings.type != pSettings->type )
    {
        BDBG_ERR(("Mixer type cannot be changed on the fly."));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    if ( hMixer->interface->setSettings )
    {
        return hMixer->interface->setSettings(hMixer, pSettings);
    }
    else
    {   
        BDBG_ERR(("This mixer type does not support changing settings on the fly."));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }   
}

/*************************************************************************/
BERR_Code BAPE_Mixer_Start(
    BAPE_MixerHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Mixer);
    BDBG_ASSERT(NULL != handle->interface);
    
    if(handle->interface->start) {
        return handle->interface->start(handle);
    } else {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

/*************************************************************************/
void BAPE_Mixer_Stop(
    BAPE_MixerHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Mixer);
    BDBG_ASSERT(NULL != handle->interface);

    if(handle->interface->stop) {
        handle->interface->stop(handle);
    } else {
        (void) BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

/*************************************************************************/
BERR_Code BAPE_Mixer_AddInput(
    BAPE_MixerHandle handle,
    BAPE_Connector input,
    const BAPE_MixerAddInputSettings *pSettings     /* Optional, pass NULL for default settings */
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Mixer);
    BDBG_ASSERT(NULL != handle->interface);

    if(handle->interface->addInput) {
        return handle->interface->addInput(handle, input, pSettings);
    } else {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

/*************************************************************************/
BERR_Code BAPE_Mixer_RemoveInput(
    BAPE_MixerHandle handle,
    BAPE_Connector input
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Mixer);
    BDBG_ASSERT(NULL != handle->interface);

    if(handle->interface->removeInput) {
        return handle->interface->removeInput(handle, input);
    } else {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

/*************************************************************************/
BERR_Code BAPE_Mixer_RemoveAllInputs(
    BAPE_MixerHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Mixer);
    BDBG_ASSERT(NULL != handle->interface);

    if(handle->interface->removeAllInputs) {
        return handle->interface->removeAllInputs(handle);
    } else {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

/*************************************************************************/
BERR_Code BAPE_Mixer_AddOutput(
    BAPE_MixerHandle handle,
    BAPE_OutputPort output
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Mixer);
    BDBG_ASSERT(NULL != handle->interface);

    if(handle->interface->addOutput) {
        return handle->interface->addOutput(handle, output);
    } else {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

/*************************************************************************/
BERR_Code BAPE_Mixer_RemoveOutput(
    BAPE_MixerHandle handle,
    BAPE_OutputPort output
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Mixer);
    BDBG_ASSERT(NULL != handle->interface);

    if(handle->interface->removeOutput) {
        return handle->interface->removeOutput(handle, output);
    } else {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

/*************************************************************************/
BERR_Code BAPE_Mixer_RemoveAllOutputs(
    BAPE_MixerHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Mixer);
    BDBG_ASSERT(NULL != handle->interface);

    if(handle->interface->removeAllOutputs) {
        return handle->interface->removeAllOutputs(handle);
    } else {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

/*************************************************************************/
BERR_Code BAPE_Mixer_GetInputVolume(
    BAPE_MixerHandle handle,
    BAPE_Connector input,
    BAPE_MixerInputVolume *pVolume      /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Mixer);
    BDBG_ASSERT(NULL != handle->interface);

    if(handle->interface->getInputVolume) {
        return handle->interface->getInputVolume(handle, input, pVolume);
    } else {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

/*************************************************************************/
BERR_Code BAPE_Mixer_SetInputVolume(
    BAPE_MixerHandle handle,
    BAPE_Connector input,
    const BAPE_MixerInputVolume *pVolume
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Mixer);
    BDBG_ASSERT(NULL != handle->interface);

    if(handle->interface->setInputVolume) {
        return handle->interface->setInputVolume(handle, input, pVolume);
    } else {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

/*************************************************************************/
BERR_Code BAPE_Mixer_P_ApplyOutputVolume(BAPE_MixerHandle handle, BAPE_OutputPort output)
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Mixer);
    BDBG_ASSERT(NULL != handle->interface);

    if(handle->interface->applyOutputVolume) {
        return handle->interface->applyOutputVolume(handle, output);
    } else {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

void BAPE_Mixer_GetConnector(
    BAPE_MixerHandle handle,
    BAPE_Connector *pConnector /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Mixer);
    BDBG_ASSERT(NULL != pConnector);
    *pConnector = &handle->pathNode.connectors[0];
}

/*************************************************************************/
const char *BAPE_Mixer_P_MclkSourceToText_isrsafe( BAPE_MclkSource mclkSource )
{
    BDBG_CASSERT((int)BAPE_MclkSource_eMax == (int)14);  /* This should cause compile error if somebody adds to the BAPE_MclkSource enum */

    return ( mclkSource == BAPE_MclkSource_eNone       ?  "None"      : 
             mclkSource == BAPE_MclkSource_ePll0       ?  "Pll0"      : 
             mclkSource == BAPE_MclkSource_ePll1       ?  "Pll1"      : 
             mclkSource == BAPE_MclkSource_ePll2       ?  "Pll2"      : 
             mclkSource == BAPE_MclkSource_eHifidac0   ?  "Hifidac0"  : 
             mclkSource == BAPE_MclkSource_eHifidac1   ?  "Hifidac1"  : 
             mclkSource == BAPE_MclkSource_eHifidac2   ?  "Hifidac2"  : 
             mclkSource == BAPE_MclkSource_eNco0       ?  "Nco0"      : 
             mclkSource == BAPE_MclkSource_eNco1       ?  "Nco1"      : 
             mclkSource == BAPE_MclkSource_eNco2       ?  "Nco2"      : 
             mclkSource == BAPE_MclkSource_eNco3       ?  "Nco3"      : 
             mclkSource == BAPE_MclkSource_eNco4       ?  "Nco4"      : 
             mclkSource == BAPE_MclkSource_eNco5       ?  "Nco5"      :
             mclkSource == BAPE_MclkSource_eNco6       ?  "Nco6"      :
             mclkSource == BAPE_MclkSource_eMax        ?  "<eMax>"    : 
                                                          "<Undefined>" );
}
/*************************************************************************/

BERR_Code BAPE_Mixer_P_DetermineOutputDataType(BAPE_MixerHandle handle, BAPE_DataType *pDataType)
{
    unsigned i;
    BAPE_DataType dataType = BAPE_DataType_eMax;
    bool pcm=true;
    unsigned numChannels=0;

    BDBG_OBJECT_ASSERT(handle, BAPE_Mixer);
    BDBG_ASSERT(NULL != pDataType);

    if ( handle->explicitFormat != BAPE_MixerFormat_eMax )
    {
        BDBG_MSG(("%s - explicitFormat %d", __FUNCTION__, handle->explicitFormat));
    }
    /* set the initial PCM mode, if requested */
    switch ( handle->explicitFormat )
    {
        case BAPE_MixerFormat_ePcmStereo:
            dataType = BAPE_DataType_ePcmStereo;
            numChannels = 2;
            break;
        case BAPE_MixerFormat_ePcm5_1:
            dataType = BAPE_DataType_ePcm5_1;
            numChannels = 6;
            break;
        case BAPE_MixerFormat_ePcm7_1:
            dataType = BAPE_DataType_ePcm7_1;
            numChannels = 8;
            break;
        case BAPE_MixerFormat_eCompressed:
        default:
            /* let inputs dictate output data type */
            break;
    }

    for ( i = 0; i < BAPE_CHIP_MAX_MIXER_INPUTS; i++ )
    {
        if ( NULL != handle->inputs[i] )
        {
            BAPE_Connector input = handle->inputs[i];

            BDBG_MSG(("Input %s %s [%u] data format %s, current dataType %d", input->pParent->pName, input->pName, i, BAPE_FMT_P_GetTypeName_isrsafe(&input->format), dataType));
            if ( dataType == BAPE_DataType_eMax )
            {
                /* First valid input */
                dataType = input->format.type;
                pcm = BAPE_FMT_P_IsLinearPcm_isrsafe(&input->format);
                numChannels = BAPE_FMT_P_GetNumChannels_isrsafe(&input->format);
            }
            else
            {
                if ( BAPE_FMT_P_IsLinearPcm_isrsafe(&input->format) != pcm )
                {
                    BDBG_ERR(("PCM and non-PCM inputs can not be mixed."));
                    return BERR_TRACE(BERR_NOT_SUPPORTED);
                }
                if ( pcm )
                {
                    if ( BAPE_FMT_P_GetNumChannels_isrsafe(&input->format) > numChannels )
                    {
                        dataType = input->format.type;
                        numChannels = BAPE_FMT_P_GetNumChannels_isrsafe(&input->format);
                        switch ( handle->explicitFormat )
                        {
                            case BAPE_MixerFormat_eCompressed:
                                BDBG_ERR(("Mixer is already running in Compressed mode, but input attached is PCM"));
                                return BERR_TRACE(BERR_NOT_SUPPORTED);
                                break;
                            case BAPE_MixerFormat_ePcmStereo:
                                if ( numChannels > 2 )
                                {
                                    BDBG_ERR(("Mixer is already running in Stereo mode, but input attached has %d channels", numChannels));
                                    return BERR_TRACE(BERR_NOT_SUPPORTED);
                                }
                                break;
                            case BAPE_MixerFormat_ePcm5_1:
                                if ( numChannels > 6 )
                                {
                                    BDBG_ERR(("Mixer is already running in 5.1ch mode, but input attached has %d channels", numChannels));
                                    return BERR_TRACE(BERR_NOT_SUPPORTED);
                                }
                                break;
                            case BAPE_MixerFormat_ePcm7_1:
                            default:
                                /* do nothing */
                                break;
                        }
                    }
                }
                else
                {
                    /* Don't allow mixing of data types for non-PCM sources. */
                    if ( dataType != input->format.type )
                    {
                        BDBG_ERR(("Can not mix non-PCM data of different formats"));
                        return BERR_TRACE(BERR_NOT_SUPPORTED);
                    }
                    switch ( handle->explicitFormat )
                    {
                        case BAPE_MixerFormat_ePcmStereo:
                        case BAPE_MixerFormat_ePcm5_1:
                        case BAPE_MixerFormat_ePcm7_1:
                            BDBG_ERR(("Mixer is already running in PCM mode, but input attached is Compressed"));
                            return BERR_TRACE(BERR_NOT_SUPPORTED);
                            break;
                        case BAPE_MixerFormat_eCompressed:
                        default:
                            /* do nothing */
                            break;
                    }
                }
            }
        }
    }

    /* if no inputs were connected, default to the requested mixer type, or stereo */
    if ( dataType == BAPE_DataType_eMax )
    {
        switch ( handle->explicitFormat )
        {
            case BAPE_MixerFormat_ePcm5_1:
                dataType = BAPE_DataType_ePcm5_1;
                break;
            case BAPE_MixerFormat_ePcm7_1:
                dataType = BAPE_DataType_ePcm7_1;
                break;
            case BAPE_MixerFormat_eCompressed:
                dataType = BAPE_DataType_eIec61937;
                break;
            case BAPE_MixerFormat_ePcmStereo:
            default:
                dataType = BAPE_DataType_ePcmStereo;
                break;
        }
    }

    *pDataType = dataType;
    if ( handle->explicitFormat != BAPE_MixerFormat_eMax )
    {
        BDBG_MSG(("%s - explicitFormat %d, returning dataType %d", __FUNCTION__, handle->explicitFormat, *pDataType));
    }
    return BERR_SUCCESS;
}

#if BDBG_DEBUG_BUILD 

BDBG_FILE_MODULE(bape_diag);

static BERR_Code BAPE_Mixer_P_PrintOutputPortObjectInfo( BAPE_OutputPortObject  *pOutputPortObject, int level)
{
    BERR_Code errCode=BERR_SUCCESS;

    #if 0
        
            /* When we find a way to get info about the specific output device, then
             * we can print info specific to the output type like this:
             */
        
            if ( BAPE_OutputPortType_eI2sOutput == pOutputPortObject->type  )
            {
                BAPE_I2sOutputHandle  i2sOutputHandle = pOutputPortObject->pHandle;
        
                BDBG_MODULE_LOG(bape_diag,("%*sOutputPort: %s (%p) muted:%u Fs:%u enab:%u", level*4, "", 
                                            pOutputPortObject->pName, (void *) pOutputPortObject,
                                            pOutputPortObject->volume.muted  ));
            }
            else
    #endif
    {
        BDBG_LOG(("%*sOutputPort(%p): %s(%p)", level*4, "",
            (void *) pOutputPortObject, pOutputPortObject->pName, (void *)pOutputPortObject->pHandle ));
    }

    return( errCode );
}


BERR_Code BAPE_Mixer_P_PrintInputPortInfo( BAPE_PathNode *pPathNode, int level, int index)
{
    BERR_Code errCode=BERR_SUCCESS;
    BAPE_InputPort  inputPort = NULL;
    BAVC_XptContextMap *pContextMap = NULL;
    BSTD_UNUSED(index);

    /* This function only works for nodes that can have an input port connected to them.
     * That means either a decoder, or input capture nodes.   */
    if ( BAPE_PathNodeType_eDecoder == pPathNode->type  )
    {
        BAPE_DecoderHandle      decoderHandle;

        /* The node is a decoder, so try to get either the input port or the RAVE context map.  */
        decoderHandle = pPathNode->pHandle;
        if (decoderHandle)
        {
            if (decoderHandle->startSettings.inputPort)
            {
                inputPort = decoderHandle->startSettings.inputPort;
            }
            else if (decoderHandle->startSettings.pContextMap)
            {
                pContextMap = &decoderHandle->contextMap;   
            }
        }
    }
    else if ( BAPE_PathNodeType_eInputCapture == pPathNode->type    )
    {
        BAPE_InputCaptureHandle      inputCaptureHandle;

        /* The node is an InputCapture, so get the connected InputPort.  */
        inputCaptureHandle = pPathNode->pHandle;
        if (inputCaptureHandle)
        {
            if (inputCaptureHandle->startSettings.input)
            {
                inputPort = inputCaptureHandle->startSettings.input;
            }
        }
    }

    if (inputPort)
    {
        /* Print out a line to describe the input port.  */
        BDBG_LOG(("%*sInputPort(%p): %s(%p) Format:%s Fs:%u", level*4, "",
                                    (void *) inputPort, inputPort->pName, (void *)inputPort->pHandle,
                                    BAPE_FMT_P_GetTypeName_isrsafe(&inputPort->format),
                                    inputPort->format.sampleRate
                                    ));
    }
    else if (pContextMap)
    {
        /* For a RAVE context input, just print the context index.  */
        BDBG_LOG(("%*sRAVE Context Index:%u", level*4, "", 
                                    pContextMap->ContextIdx ));
    }
    return( errCode );
}


BERR_Code BAPE_Mixer_P_PrintNodeInfo( BAPE_PathNode *pPathNode, int level, int index)
{
    BERR_Code errCode=BERR_SUCCESS;
    BSTD_UNUSED(index);

    /*  BDBG_ERR(("%s : %d : pPathNode:%p  level:%d  index:%u", __FUNCTION__, __LINE__ , (void *)pPathNode, level, index )); */
#if BAPE_DSP_SUPPORT
    if ( BAPE_PathNodeType_eDecoder == pPathNode->type  )
    {
        BAPE_DecoderHandle      decoderHandle;

        decoderHandle = pPathNode->pHandle;
        if (decoderHandle)
        {
            const BAPE_CodecAttributes  *pCodecAttributes = BAPE_P_GetCodecAttributes_isrsafe(decoderHandle->startSettings.codec);

            BDBG_LOG(("%*sPathNode(%p): %s(%p) Type:%s Codec:%s DSP Index: %d", level*4, "",
                                        (void *) pPathNode, pPathNode->pName, (void *)pPathNode->pHandle,
                                        BAPE_PathNode_P_PathNodeTypeToText(pPathNode->type), 
                                        pCodecAttributes->pName, decoderHandle->dspIndex ));
        }
    }
    else
#endif
        if ( BAPE_PathNodeType_eMixer == pPathNode->type  )
    {
        BAPE_MixerHandle mixerHandle = pPathNode->pHandle;
        BAPE_OutputPortObject *pOutputPortObject;
        BAPE_MixerSettings mixerSettings;
        char mixerIndex[13] = "";

        BAPE_Mixer_GetSettings(mixerHandle, &mixerSettings);
        if (mixerSettings.type == BAPE_MixerType_eDsp)
        {
            BKNI_Snprintf(mixerIndex, sizeof(mixerIndex), "DSP Index: %u",mixerSettings.dspIndex);
        }
        BDBG_LOG(("%*sPathNode(%p): %s(%p) Type:%s %s(%u) Format:%s Fs:%u MclkSource:%s %s", level*4, "",
                                    (void *) pPathNode, pPathNode->pName, (void *)pPathNode->pHandle,
                                    BAPE_PathNode_P_PathNodeTypeToText(pPathNode->type),
                                    mixerHandle->running ? "RUNNING" : "stopped",
                                    mixerHandle->running , 
                                    BAPE_FMT_P_GetTypeName_isrsafe(&pPathNode->connectors[0].format),
                                    pPathNode->connectors[0].format.sampleRate,
                                    BAPE_Mixer_P_MclkSourceToText_isrsafe(mixerHandle->mclkSource),
                                    mixerIndex));
        
        for ( pOutputPortObject = BLST_S_FIRST(&mixerHandle->outputList);
              pOutputPortObject != NULL;
              pOutputPortObject = BLST_S_NEXT(pOutputPortObject, node) )
        {
            level++;
                BAPE_Mixer_P_PrintOutputPortObjectInfo( pOutputPortObject, level );
            level--;
        }

    }
    else
    {
        BDBG_LOG(("%*sPathNode(%p): %s(%p) Type:%s ", level*4, "",
            (void *) pPathNode, pPathNode->pName, (void *)pPathNode->pHandle,
            BAPE_PathNode_P_PathNodeTypeToText(pPathNode->type) ));
    
    }
    return( errCode );
}


static const char  *BAPE_Mixer_P_CheckBdbgEnabled(bool *enabled) {
    *enabled = true;
     return "";
}


BDBG_FILE_MODULE(bape_mixerprint);
BERR_Code BAPE_Mixer_P_PrintMixers(BAPE_Handle deviceHandle)
{
    BERR_Code errCode=BERR_SUCCESS;
    BAPE_MixerHandle mixer;
    BAPE_PathNode   *pPathNode;
    unsigned mixerIndex;
    bool  dbug_enabled = false;

    /* To determine if our debuglevel is enabled, we'll need to make two calls to BDBG_MODULE_MSG.  The first call
     * is to make sure that the module gets registered, then the second will only evaluate the argument list
     * if the BDBG level is enabled. 
     */ 
    BDBG_MODULE_MSG(bape_mixerprint,("bape_mixerprint is enabled"));                                                      /* Make sure that bape_mixerprint is registered with BDBG   */
    BDBG_MODULE_MSG(bape_mixerprint,("Printing Audio Filter Graph...%s", BAPE_Mixer_P_CheckBdbgEnabled(&dbug_enabled)));  /* Set dbug_enabled if bape_mixerprint is enabled           */
    if (!dbug_enabled)
    {
        return BERR_SUCCESS;    /* if debug level not enabled, just return without printing */
    }

    for ( mixer = BLST_S_FIRST(&deviceHandle->mixerList), mixerIndex=0;
          mixer != NULL;
          mixer = BLST_S_NEXT(mixer, node), mixerIndex++ )
    {
        BDBG_MODULE_LOG(bape_diag,("==================  Mixer:%u (%p) Begin  =====================================",
                                    mixerIndex, (void *) mixer ));

        pPathNode = &mixer->pathNode;
        BAPE_PathNode_P_EnumerateUpstreamPathNodes(pPathNode, 0, BAPE_Mixer_P_PrintNodeInfo );
        BAPE_Mixer_P_PrintNodeInfo( pPathNode, 0, 0);
        BAPE_PathNode_P_EnumerateDownstreamPathNodes(pPathNode, 0, BAPE_Mixer_P_PrintNodeInfo );

        BDBG_MODULE_LOG(bape_diag,("===================  Mixer:%u (%p) End  ======================================",
                                    mixerIndex, (void *) mixer ));

        BDBG_MODULE_LOG(bape_diag,(" "));
    }

    return( errCode );
}


BDBG_FILE_MODULE(bape_startprint);
BERR_Code BAPE_Mixer_P_PrintDownstreamNodes(BAPE_PathNode *pPathNode)
{
    BERR_Code errCode=BERR_SUCCESS;
    bool  dbug_enabled = false;

    /* To determine if our debuglevel is enabled, we'll need to make two calls to BDBG_MODULE_MSG.  The first call
     * is to make sure that the module gets registered, then the second will only evaluate the argument list
     * if the BDBG level is enabled. 
     */ 
    BDBG_MODULE_MSG(bape_startprint,("bape_startprint is enabled"));                                                 /* Make sure that bape_startprint is registered with BDBG   */
    BDBG_MODULE_MSG(bape_startprint,("Printing Started Nodes...%s", BAPE_Mixer_P_CheckBdbgEnabled(&dbug_enabled)));  /* Set dbug_enabled if bape_startprint is enabled           */
    if (!dbug_enabled)
    {
        return BERR_SUCCESS;    /* if debug level not enabled, just return without printing */
    }

        BDBG_MODULE_LOG(bape_diag,("==================  Started node(%p): %s(%p) Begin  =====================================",
                                    (void *)pPathNode, pPathNode->pName, (void *)pPathNode->pHandle));

        /* First, try to print the InputPort or RAVE context that's attached to our starting node. */
        BAPE_Mixer_P_PrintInputPortInfo(pPathNode, 0, 0);

        /* Then print the starting node... */
        BAPE_Mixer_P_PrintNodeInfo( pPathNode, 1, 0);
        /* Then recurse down through the remaining connected nodes.  */
        BAPE_PathNode_P_EnumerateDownstreamPathNodes(pPathNode, 1, BAPE_Mixer_P_PrintNodeInfo );

        BDBG_MODULE_LOG(bape_diag,("==================  Started node(%p): %s(%p) End  =====================================",
                                    (void *)pPathNode, pPathNode->pName, (void *)pPathNode->pHandle));

        BDBG_MODULE_LOG(bape_diag,(" "));

    return( errCode );
}


#else  /* BDBG_DEBUG_BUILD is false */
BERR_Code BAPE_Mixer_P_PrintMixers(BAPE_Handle deviceHandle)
{
    BERR_Code errCode=BERR_SUCCESS;
    BSTD_UNUSED(deviceHandle);
    return errCode;
}

#endif /* BDBG_DEBUG_BUILD */


