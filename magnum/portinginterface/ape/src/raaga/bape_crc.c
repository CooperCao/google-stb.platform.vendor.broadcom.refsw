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
 * Module Description: Audio CRC Interface
 *
***************************************************************************/

#include "bstd.h"
#include "bkni.h"
#include "bape.h"
#include "bape_priv.h"
#include "bape_buffer.h"

#ifdef BCHP_AIO_INTH_REG_START
    #include "bchp_int_id_aio_inth.h"
#endif

#ifdef BCHP_AUD_INTH_REG_START
    #include "bchp_int_id_aud_inth.h"
#endif

BDBG_MODULE(bape_crc);

#if BAPE_CHIP_MAX_CRCS > 0   /* Only define API if CRC capability exists, else skip all of this and just put in stub funcs at bottom of file. */

BDBG_OBJECT_ID(BAPE_Crc);

/* 16 bits per CRC, Assume worst case - 2 seconds worth of CRC data,
   minimum sample period of 10 stereo samples, max samplerate of 192k */
#define BAPE_CRC_BUFFER_SIZE    (192000/10)*4        

typedef struct BAPE_Crc_P_Resource
{
    BAPE_BufferHandle buffer;
    unsigned hwIndex;
    unsigned fciId;
} BAPE_Crc_P_Resource;

typedef struct BAPE_Crc
{
    BDBG_OBJECT(BAPE_Crc)
    BAPE_Handle deviceHandle;

    unsigned index;

    BAPE_CrcOpenSettings settings;
    BAPE_CrcSourceType sourceType;
    BAPE_CrcInputSettings inputSettings;

    BAPE_BufferHandle buffer[BAPE_CHIP_MAX_CRCS];
    BAPE_Crc_P_Resource resources[BAPE_CHIP_MAX_CRCS];
    BAPE_CrcEntry last[BAPE_CHIP_MAX_CRCS];

    BAPE_CrcInterruptHandlers interrupts;
    BINT_CallbackHandle intHandle;

    bool inputConnected;
    bool started;
    char name[6];   /* CRC %d */
} BAPE_Crc;

/* local function prototypes */
static void BAPE_Crc_P_DataReady_isr(BAPE_Handle devicehandle);
static void BAPE_P_CRC_isr (void * pParam1, int    iParam2);


/***************************************************************************
Summary:
CRC Get Default Settings
***************************************************************************/
void BAPE_Crc_GetDefaultOpenSettings(
    BAPE_CrcOpenSettings *pSettings /*[out] */
    )
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
}

/***************************************************************************
Summary:
CRC Get Default Settings
***************************************************************************/
void BAPE_Crc_GetDefaultInputSettings(
    BAPE_CrcSourceType sourceType,
    BAPE_CrcInputSettings *pInputSettings /*[out] */
    )
{
    BDBG_ASSERT(NULL != pInputSettings);
    switch (sourceType)
    {
        case BAPE_CrcSourceType_ePlaybackBuffer:
            BKNI_Memset(&pInputSettings->source.playbackBuffer, 0, sizeof(pInputSettings->source.playbackBuffer));
            break;
        case BAPE_CrcSourceType_eOutputPort:
            BKNI_Memset(&pInputSettings->source.outputPort, 0, sizeof(pInputSettings->source.outputPort));
            break;
        default:
            BDBG_ERR(("ERROR - invalid sourceType %d", sourceType));
            break;
    }
}

/***************************************************************************
Summary:
CRC Open
***************************************************************************/
BERR_Code BAPE_Crc_Open(
    BAPE_Handle deviceHandle,
    unsigned index, 
    const BAPE_CrcOpenSettings * pSettings, 
    BAPE_CrcHandle * pHandle /* [out] */
    )
{
    BAPE_CrcHandle handle;
#if SOURCE_TYPE_OPEN
    BAPE_CrcSourceType sourceType = BAPE_CrcSourceType_eMax;
#endif
    BERR_Code errCode;
    unsigned i = 0;
    unsigned j;
    unsigned freeCrcs = BAPE_CHIP_MAX_CRCS;
    BINT_CallbackHandle intHandle = NULL;

    BDBG_OBJECT_ASSERT(deviceHandle, BAPE_Device);

    if ( index >= BAPE_CHIP_MAX_CRCS )
    {
        BDBG_ERR(("Request to open CRC %d but chip only has %u CRCs", index, BAPE_CHIP_MAX_CRCS));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* Init to specified settings */
    if ( NULL == pSettings )
    {
        BDBG_ERR(("Settings cannot be NULL"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if ( deviceHandle->crcs[index] )
    {
        BDBG_ERR(("CRC %u already open", index));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    for ( i = 0; i < BAPE_CHIP_MAX_CRCS; i++ )
    {
        if ( deviceHandle->crcAllocated[i] )
        {
            freeCrcs--;
        }
    }

    for ( i = 0; i < BAPE_CHIP_MAX_CRCS; i++ )
    {
        if ( deviceHandle->crcs[i] != NULL )
        {
            /* grab the HW interrupt handler from an open CRC.
            if none are open, it will be allocated below */
            if ( intHandle == NULL )
            {
                intHandle = deviceHandle->crcs[i]->intHandle;
            }
#if SOURCE_TYPE_OPEN
            /* force source type to match existing CRCs */
            if ( sourceType == BAPE_CrcSourceType_eMax )
            {
                sourceType = deviceHandle->crcs[i]->settings.sourceType;
            }
            break;
#endif
        }
    }

#if SOURCE_TYPE_OPEN
    BDBG_MSG(("%s - sourceType %d, existing sourceType %d, requested numChannelPairs %d, freeCrcs %d", __FUNCTION__, 
        pSettings->sourceType, sourceType, pSettings->numChannelPairs, freeCrcs));
    if ( sourceType == BAPE_CrcSourceType_eMax )
    {
        sourceType = pSettings->sourceType;
    }
#else
    BDBG_MSG(("%s - requested numChannelPairs %d, freeCrcs %d", __FUNCTION__, 
        pSettings->numChannelPairs, freeCrcs));
#endif

    /* verify we have enough free slots */
    if ( pSettings->numChannelPairs > freeCrcs )
    {
        BDBG_ERR(("%s - not enough free CRC slots.  requested %d, but only %d free", __FUNCTION__, pSettings->numChannelPairs, freeCrcs));
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }

#if SOURCE_TYPE_OPEN
    /* sourceType much match for all configured CRCs */
    if ( pSettings->sourceType == BAPE_CrcSourceType_eMax || 
         pSettings->sourceType != sourceType )
    {
        BDBG_ERR(("%s - source type mismatch or invalid sourceType %d, existing sourceType %d", __FUNCTION__, pSettings->sourceType, sourceType));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
#endif

    /* Allocate the device structure, then fill in all the fields. */
    handle = BKNI_Malloc(sizeof(BAPE_Crc));
    if ( NULL == handle )
    {
        BDBG_ERR(("%s - not enough memory to allocate CRC handle", __FUNCTION__));
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }

    BKNI_Memset(handle, 0, sizeof(BAPE_Crc));
    BDBG_OBJECT_SET(handle, BAPE_Crc);
    handle->deviceHandle = deviceHandle;
    handle->index = index;
    handle->sourceType = BAPE_CrcSourceType_eMax;
    BKNI_Snprintf(handle->name, sizeof(handle->name), "CRC %u", index);
    handle->settings = *pSettings;

    /* Allocate CRC buffer(s) */
    for ( i = 0; i < pSettings->numChannelPairs; i++ )
    {
        BAPE_BufferSettings bufferSettings;
        BAPE_BufferHandle bufferHandle;

        BAPE_Buffer_GetDefaultSettings(&bufferSettings);
        if ( pSettings->memHandle )
        {
            bufferSettings.heap = pSettings->memHandle;
        } 
        else
        {
            bufferSettings.heap = deviceHandle->memHandle;
        }

        if ( pSettings->bufferSize > 0 )
        {
            bufferSettings.bufferSize = (pSettings->bufferSize / sizeof(BAPE_CrcEntry)) * sizeof(BAPE_CrcEntry);
        }
        else
        {
            bufferSettings.bufferSize = BAPE_CRC_BUFFER_SIZE;
        }

        errCode = BAPE_Buffer_Open(&bufferSettings, &bufferHandle);
        if ( errCode )
        {
            BDBG_ERR(("%s - unable to allocate CRC Buffer", __FUNCTION__));
            BAPE_Crc_Close(handle);
            return BERR_TRACE(errCode);
        }

        handle->resources[i].buffer = bufferHandle;
        handle->resources[i].hwIndex = 0xffffffff;

        for ( j = 0; j < BAPE_CHIP_MAX_CRCS; j++)
        {
            if ( !deviceHandle->crcAllocated[j] )
            {
                handle->resources[i].hwIndex = j;
                deviceHandle->crcAllocated[j] = true;
                break;
            }
        }

        if ( handle->resources[i].hwIndex == 0xffffffff )
        {
            /* this should never happen... would indicate bad detection code above */
            BDBG_ERR(("%s - unable to find free HW slot", __FUNCTION__));
            BAPE_Crc_Close(handle);
            return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        }
    }

    if ( intHandle == NULL )
    {
        /* Install CRC interrupt handler */
        errCode = BINT_CreateCallback(&intHandle,
                                    handle->deviceHandle->intHandle,
                                    BCHP_INT_ID_AUD_CRC,
                                    BAPE_P_CRC_isr,
                                    handle->deviceHandle,
                                    0);
        if ( errCode )
        {
            BDBG_ERR(("%s - unable to register CRC interrupt with BINT", __FUNCTION__));
            BAPE_Crc_Close(handle);
            return BERR_TRACE(errCode);
        }
    }

    BKNI_EnterCriticalSection();
    handle->intHandle = intHandle;
    deviceHandle->crcs[index] = handle;
    BKNI_LeaveCriticalSection();

    *pHandle = handle;

    return BERR_SUCCESS;
}

/***************************************************************************
Summary:
CRC Close
***************************************************************************/
BERR_Code BAPE_Crc_Close(
    BAPE_CrcHandle handle
    )
{
    unsigned i;
    unsigned crcCount = 0;
    BDBG_OBJECT_ASSERT(handle, BAPE_Crc);

    if ( handle->started )
    {
        BDBG_ERR(("%s - CRC must be stopped before it can be closed", __FUNCTION__));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    if ( handle->inputConnected )
    {
        BDBG_ERR(("%s - CRC input must be removed before it can be closed", __FUNCTION__));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    /* look for existing CRCs */
    for ( i = 0; i < BAPE_CHIP_MAX_CRCS; i++ )
    {
        if ( handle->deviceHandle->crcs[i] && 
             handle->deviceHandle->crcs[i]->index != handle->index )
        {
            crcCount++;
        }        
    }

    for ( i = 0; i < handle->settings.numChannelPairs; i++)
    {
        handle->deviceHandle->crcAllocated[handle->resources[i].hwIndex] = false;
    }

    /* if we are the last crc, destroy the BINT callback */
    if ( crcCount == 0 )
    {
        if ( handle->intHandle )
        {
            BINT_DestroyCallback(handle->intHandle);
            handle->intHandle = NULL;
        }
    }

    /* Destroy buffer pool */    
    for ( i = 0; i < handle->settings.numChannelPairs; i++ )
    {
        BAPE_Buffer_Close(handle->resources[i].buffer);
        handle->resources[i].buffer = NULL;
    }

    /* delete handle */
    handle->deviceHandle->crcs[handle->index] = NULL;    
    BDBG_OBJECT_DESTROY(handle, BAPE_Crc);
    BKNI_Free(handle);

    return BERR_SUCCESS;
}

/***************************************************************************
Summary:
CRC AddInput
***************************************************************************/
BERR_Code BAPE_Crc_AddInput(
    BAPE_CrcHandle handle, 
    BAPE_CrcSourceType sourceType,
    const BAPE_CrcInputSettings * pInputSettings
    )
{
#if !SOURCE_TYPE_OPEN
    BAPE_CrcSourceType curSourceType = BAPE_CrcSourceType_eMax;
    unsigned i;
#endif
    unsigned inputIndex;
    BDBG_OBJECT_ASSERT(handle, BAPE_Crc);

    if ( NULL == pInputSettings )
    {
        BDBG_ERR(("%s - pSettings is NULL", __FUNCTION__));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if ( handle->inputConnected )
    {
        BDBG_ERR(("%s - CRC %d is already connected, remove previous connection before adding the input again", __FUNCTION__, handle->index));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    if ( handle->started )
    {
        BDBG_ERR(("%s - CRC %d is already started, stop before adding the input", __FUNCTION__, handle->index));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
#if !SOURCE_TYPE_OPEN
    for ( i = 0; i < BAPE_CHIP_MAX_CRCS; i++ )
    {
        if ( handle->deviceHandle->crcs[i] != NULL )
        {
            /* force source type to match existing CRCs */
            if ( curSourceType == BAPE_CrcSourceType_eMax )
            {
                curSourceType = handle->deviceHandle->crcs[i]->sourceType;
            }
            break;
        }
    }

    BDBG_MSG(("%s - sourceType %d, existing sourceType %d", __FUNCTION__, 
        sourceType, curSourceType));
    if ( curSourceType == BAPE_CrcSourceType_eMax )
    {
        curSourceType = sourceType;
    }

    /* sourceType much match for all configured CRCs */
    if ( sourceType == BAPE_CrcSourceType_eMax || 
         sourceType != curSourceType )
    {
        BDBG_ERR(("%s - source type mismatch or invalid sourceType %d, existing sourceType %d", __FUNCTION__, sourceType, curSourceType));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
#endif

    switch (sourceType)
    {
        case BAPE_CrcSourceType_ePlaybackBuffer:
            BDBG_OBJECT_ASSERT(pInputSettings->source.playbackBuffer.mixer, BAPE_Mixer);
            BDBG_OBJECT_ASSERT(pInputSettings->source.playbackBuffer.input, BAPE_PathConnector);
            if (pInputSettings->source.playbackBuffer.mixer->running > 0 )
            {
                BDBG_ERR(("CRC capture on Mixer Input must be connected before any inputs to a mixer are started"));
                return BERR_TRACE(BERR_NOT_SUPPORTED);
            }

            inputIndex = BAPE_Mixer_P_FindInputIndex_isrsafe(pInputSettings->source.playbackBuffer.mixer, pInputSettings->source.playbackBuffer.input);
            if ( inputIndex == BAPE_MIXER_INPUT_INDEX_INVALID )
            {
                BDBG_ERR(("input must be connected to mixer prior to adding CRC connection"));
                return BERR_TRACE(BERR_NOT_SUPPORTED);
            }

            pInputSettings->source.playbackBuffer.mixer->crcs[inputIndex] = handle;
            break;
        case BAPE_CrcSourceType_eOutputPort:
            BDBG_OBJECT_ASSERT(pInputSettings->source.outputPort.outputPort, BAPE_OutputPort);

            if ( pInputSettings->source.outputPort.outputPort->mixer == NULL ) 
            {
                BDBG_ERR(("Warning - Output must be connected to mixer in order for CRC to function"));
            }
            else if ( pInputSettings->source.outputPort.outputPort->mixer->running > 0 )
            {
                BDBG_ERR(("Warning - CRC capture on Output Port must be connected before mixer outputs are started"));
                return BERR_TRACE(BERR_NOT_SUPPORTED);
            }

            pInputSettings->source.outputPort.outputPort->crc = handle;
            break;
        default:
            BDBG_ERR(("%s - invalid sourceType %d", __FUNCTION__, handle->sourceType));
            return BERR_TRACE(BERR_INVALID_PARAMETER);
            break;
    }

    handle->inputSettings = *pInputSettings;
    handle->sourceType = sourceType;
    handle->inputConnected = true;

    return BERR_SUCCESS;
}

/***************************************************************************
Summary:
CRC RemoveInput
***************************************************************************/
BERR_Code BAPE_Crc_RemoveInput(
    BAPE_CrcHandle handle
    )
{
    unsigned inputIndex;
    BDBG_OBJECT_ASSERT(handle, BAPE_Crc);

    if ( !handle->inputConnected )
    {
        /* nothing to do */
        return BERR_SUCCESS;
    }

    if ( handle->started )
    {
        BDBG_ERR(("%s - CRC %d is started, stop before removing the input", __FUNCTION__, handle->index));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    switch (handle->sourceType)
    {
        case BAPE_CrcSourceType_ePlaybackBuffer:
            BDBG_OBJECT_ASSERT(handle->inputSettings.source.playbackBuffer.mixer, BAPE_Mixer);
            BDBG_OBJECT_ASSERT(handle->inputSettings.source.playbackBuffer.input, BAPE_PathConnector);
            if (handle->inputSettings.source.playbackBuffer.mixer->running > 0 )
            {
                BDBG_ERR(("Mixer Input must be stopped before removing CRC Input"));
                return BERR_TRACE(BERR_NOT_SUPPORTED);
            }

            inputIndex = BAPE_Mixer_P_FindInputIndex_isrsafe(handle->inputSettings.source.playbackBuffer.mixer, handle->inputSettings.source.playbackBuffer.input);
            if ( inputIndex != BAPE_MIXER_INPUT_INDEX_INVALID )
            {
                handle->inputSettings.source.playbackBuffer.mixer->crcs[inputIndex] = NULL;
            }
            else
            {
                int i;
                BDBG_ERR(("Could not find input, looking for linkage by hand"));

                for ( i=0; i<BAPE_CHIP_MAX_MIXER_INPUTS; i++ )
                {
                    if ( handle->inputSettings.source.playbackBuffer.mixer->crcs[i] == handle )
                    {
                        handle->inputSettings.source.playbackBuffer.mixer->crcs[i] = NULL;
                    }
                }
            }
            break;
        case BAPE_CrcSourceType_eOutputPort:
            BDBG_OBJECT_ASSERT(handle->inputSettings.source.outputPort.outputPort, BAPE_OutputPort);

            if (handle->inputSettings.source.outputPort.outputPort->mixer->running > 0 )
            {
                BDBG_ERR(("Mixer must be stopped before removing CRC Input"));
                return BERR_TRACE(BERR_NOT_SUPPORTED);
            }

            handle->inputSettings.source.outputPort.outputPort->crc = NULL;
            break;
        default:
            BDBG_ERR(("%s - invalid sourceType %d", __FUNCTION__, handle->sourceType));
            return BERR_TRACE(BERR_INVALID_PARAMETER);
            break;
    }

    handle->sourceType = BAPE_CrcSourceType_eMax;
    handle->inputConnected = false;

    return BERR_SUCCESS;
}

/***************************************************************************
Summary:
CRC Start
***************************************************************************/
BERR_Code BAPE_Crc_P_Start(
    BAPE_CrcHandle handle
    )
{
    unsigned i;

    BDBG_OBJECT_ASSERT(handle, BAPE_Crc);

    if ( !handle->inputConnected )
    {
        BDBG_ERR(("%s - CRC %d does not have a valid input, cannot start", __FUNCTION__, handle->index));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    if ( handle->started )
    {
        BDBG_ERR(("%s - CRC %d is already started", __FUNCTION__, handle->index));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    BKNI_EnterCriticalSection();
    for ( i = 0; i < handle->settings.numChannelPairs; i++ )
    {
        uint32_t regAddr, regVal;
        BAPE_PathConnection * pConnection;
        BAPE_FciIdGroup fciIdGroup;

        BAPE_Reg_P_UpdateField_isr(handle->deviceHandle, 
                               BAPE_Reg_P_GetArrayAddress(AUD_MISC_CRC_PERIODi, handle->resources[i].hwIndex),
                               AUD_MISC_CRC_PERIODi, 
                               SMPL_PERIOD, 
                               handle->settings.samplingPeriod);

        switch (handle->sourceType)
        {
            case BAPE_CrcSourceType_ePlaybackBuffer:
                /* fci ids from BAPE_SfifoGroup_P_GetOutputFciIds(pConnection->sfifoGroup, &fciIdGroup)
                   Start - BAPE_StandardMixer_P_AllocateConnectionResources()
                   Stop - BAPE_StandardMixer_P_FreeConnectionResources()*/
                BAPE_Reg_P_UpdateEnum(handle->deviceHandle, BCHP_AUD_MISC_CRC_FCI_BLOCK_ID, AUD_MISC_CRC_FCI_BLOCK_ID, FCI_BLOCK_ID, AIO_BF_PLY);
                pConnection = BAPE_Connector_P_GetConnectionToSink(handle->inputSettings.source.playbackBuffer.input, &handle->inputSettings.source.playbackBuffer.mixer->pathNode);
                BAPE_SfifoGroup_P_GetOutputFciIds(pConnection->sfifoGroup, &fciIdGroup);
                handle->resources[i].fciId = fciIdGroup.ids[i];
                break;
            case BAPE_CrcSourceType_eOutputPort:
                /* fci ids from BAPE_OutputPortObject.sourceMixerFci
                   Start - BAPE_StandardMixer_P_AllocateResources()
                   Stop - BAPE_StandardMixer_P_FreeResources()*/
                BAPE_Reg_P_UpdateEnum(handle->deviceHandle, BCHP_AUD_MISC_CRC_FCI_BLOCK_ID, AUD_MISC_CRC_FCI_BLOCK_ID, FCI_BLOCK_ID, AIO_DP0);
                handle->resources[i].fciId = handle->inputSettings.source.outputPort.outputPort->sourceMixerFci.ids[i];
                break;
            default:
                BDBG_ERR(("%s - invalid sourceType %d", __FUNCTION__, handle->sourceType));
                return BERR_TRACE(BERR_INVALID_PARAMETER);
                break;
        }

        regVal = BAPE_Reg_P_Read_isr(handle->deviceHandle, BCHP_AUD_MISC_CRC_ESR_MASK_CLEAR);
        regVal |= 1 << handle->resources[i].hwIndex;
        BAPE_Reg_P_Write_isr(handle->deviceHandle, BCHP_AUD_MISC_CRC_ESR_MASK_CLEAR, regVal);

        regAddr = BAPE_Reg_P_GetArrayAddress(AUD_MISC_CRC_CFGi, handle->resources[i].hwIndex);

        BDBG_MSG(("%s - START CRC devIndex %d, hwIndex %d, regAddr %x", __FUNCTION__, handle->index, handle->resources[i].hwIndex, regAddr));
        BDBG_MSG(("\t fci %x, mode %x, width %x, polarity %x", 
            handle->resources[i].fciId, handle->settings.mode, handle->settings.dataWidth, (handle->settings.initialValue == 0) ? 0 : 1));
        
        BAPE_Reg_P_UpdateField_isr(handle->deviceHandle, regAddr, AUD_MISC_CRC_CFGi, FCI_CHANNEL_ID, 0x3f & handle->resources[i].fciId);
        BAPE_Reg_P_UpdateField_isr(handle->deviceHandle, regAddr, AUD_MISC_CRC_CFGi, CRC_MODE, handle->settings.mode);
        BAPE_Reg_P_UpdateField_isr(handle->deviceHandle, regAddr, AUD_MISC_CRC_CFGi, CRC_DAT_WIDTH, handle->settings.dataWidth);
        BAPE_Reg_P_UpdateField_isr(handle->deviceHandle, regAddr, AUD_MISC_CRC_CFGi, CRC_INIT_POL, (handle->settings.initialValue == 0) ? 0 : 1);
        BAPE_Reg_P_UpdateField_isr(handle->deviceHandle, regAddr, AUD_MISC_CRC_CFGi, CRC_ENA, 1);
    }
    BKNI_LeaveCriticalSection();

    if ( handle->intHandle )
    {
        BDBG_MSG(("%s - enabling callback", __FUNCTION__));
        BINT_EnableCallback(handle->intHandle);
    }

    handle->started = true;

    BKNI_EnterCriticalSection();
    if ( handle->interrupts.captureStarted.pCallback_isr != NULL )
    {
        handle->interrupts.captureStarted.pCallback_isr(handle->interrupts.captureStarted.pParam1, handle->interrupts.captureStarted.param2);
    }
    BKNI_LeaveCriticalSection();

    return BERR_SUCCESS;
}

/***************************************************************************
Summary:
CRC Stop
***************************************************************************/
void BAPE_Crc_P_Stop(
    BAPE_CrcHandle handle
    )
{
    unsigned i;
    unsigned crcCount = 0;

    BDBG_OBJECT_ASSERT(handle, BAPE_Crc);

    BKNI_EnterCriticalSection();
    for ( i = 0; i < handle->settings.numChannelPairs; i++ )
    {
        uint32_t regAddr, regVal;

        regAddr = BAPE_Reg_P_GetArrayAddress(AUD_MISC_CRC_CFGi, handle->resources[i].hwIndex);
        BAPE_Reg_P_UpdateField_isr(handle->deviceHandle, regAddr, AUD_MISC_CRC_CFGi, CRC_ENA, 0);
        regVal = BAPE_Reg_P_Read_isr(handle->deviceHandle, BCHP_AUD_MISC_CRC_ESR_MASK_SET);
        regVal |= 1 << handle->resources[i].hwIndex;
        BAPE_Reg_P_Write_isr(handle->deviceHandle, BCHP_AUD_MISC_CRC_ESR_MASK_SET, regVal);
    }
    BKNI_LeaveCriticalSection();

    /* look for existing CRCs */
    for ( i = 0; i < BAPE_CHIP_MAX_CRCS; i++ )
    {
        if ( handle->deviceHandle->crcs[i] && 
             handle->deviceHandle->crcs[i]->index != handle->index )
        {
            crcCount++;
        }        
    }

    /* if we are the last crc, destroy the BINT callback */
    if ( crcCount == 0 )
    {
        if ( handle->intHandle )
        {
            BINT_DisableCallback(handle->intHandle);
        }
    }
    
    handle->started = false;

    BKNI_EnterCriticalSection();
    if ( handle->interrupts.captureStopped.pCallback_isr != NULL )
    {
        handle->interrupts.captureStopped.pCallback_isr(handle->interrupts.captureStopped.pParam1, handle->interrupts.captureStopped.param2);
    }
    BKNI_LeaveCriticalSection();
}
 
/***************************************************************************
Summary:
CRC Get Buffer
***************************************************************************/
BERR_Code BAPE_Crc_GetBuffer(
    BAPE_CrcHandle handle, 
    BAPE_BufferDescriptor * pBuffers /* [out] */
    )
{
    BAPE_SimpleBufferDescriptor descriptors[BAPE_CHIP_MAX_CRCS];
    unsigned size = 0xffffffff;
    unsigned bufferSize = 0xffffffff;
    unsigned wrapBufferSize = 0xffffffff;
    unsigned i;

    BDBG_OBJECT_ASSERT(handle, BAPE_Crc);
    BDBG_ASSERT(pBuffers);

    BKNI_Memset(pBuffers, 0, sizeof(BAPE_BufferDescriptor));
    BKNI_Memset(descriptors, 0, sizeof(BAPE_SimpleBufferDescriptor[BAPE_CHIP_MAX_CRCS]));

    /*BKNI_EnterCriticalSection();*/
    for ( i = 0; i < handle->settings.numChannelPairs; i++ )
    {
        size = BAPE_MIN(size, BAPE_Buffer_Read_isr(handle->resources[i].buffer, &(descriptors[i])));
        bufferSize = BAPE_MIN(bufferSize, descriptors[i].bufferSize);
        wrapBufferSize = BAPE_MIN(bufferSize, descriptors[i].wrapBufferSize);
        /* TBDBMMA need BMMA block here? */
        pBuffers->buffers[i].pBuffer = descriptors[i].pBuffer;
        pBuffers->buffers[i].pWrapBuffer = descriptors[i].pWrapBuffer;
        BDBG_MSG(("read %d bytes, bufferSize %d, wrapBufferSize %d", size, bufferSize, wrapBufferSize));
    }

    if ( size == 0 )
    {
        BDBG_MSG(("buffers are empty"));
        BKNI_Memset(pBuffers, 0, sizeof(BAPE_BufferDescriptor));
        return BERR_SUCCESS;
    }

    /* translate buffers to APE descriptor */
    pBuffers->interleaved = true;
    pBuffers->bufferSize = bufferSize;
    pBuffers->wrapBufferSize = wrapBufferSize;
    pBuffers->numBuffers = handle->settings.numChannelPairs;
    /*BKNI_LeaveCriticalSection();*/

    return BERR_SUCCESS;
}

/***************************************************************************
Summary:
CRC Consume Data
***************************************************************************/
BERR_Code BAPE_Crc_ConsumeData(
    BAPE_CrcHandle handle, 
    unsigned numBytes
    )
{
    BAPE_SimpleBufferDescriptor descriptor;
    unsigned size = numBytes;
    unsigned i;

    BDBG_OBJECT_ASSERT(handle, BAPE_Crc);

    if (numBytes == 0)
    {
        return BERR_SUCCESS;
    }

    /*BKNI_EnterCriticalSection();*/
    for ( i = 0; i < handle->settings.numChannelPairs; i++ )
    {
        size = BAPE_MIN(size, BAPE_Buffer_Read_isr(handle->resources[i].buffer, &descriptor));
    }

    if ( size != numBytes )
    {
        BDBG_WRN(("can only consume %d of the requested %d bytes", size, numBytes));
    }

    for ( i = 0; i < handle->settings.numChannelPairs; i++ )
    {
        unsigned retSize;
        retSize = BAPE_Buffer_Advance_isr(handle->resources[i].buffer, size);

        if (retSize != size)
        {
            BDBG_WRN(("buffers are out of sync, could only advance %d of %d bytes in buffer[%d]", retSize, size, i));
        }
    }
    /*BKNI_LeaveCriticalSection();*/

    return BERR_SUCCESS;
}

#if !B_REFSW_MINIMAL
/***************************************************************************
Summary:
CRC Get Interrupt Handlers
***************************************************************************/
void BAPE_Crc_GetInterruptHandlers(
    BAPE_CrcHandle handle,
    BAPE_CrcInterruptHandlers *pInterrupts /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Crc);
    BDBG_ASSERT(NULL != pInterrupts);
    BKNI_EnterCriticalSection();
    *pInterrupts = handle->interrupts;
    BKNI_LeaveCriticalSection();
}

/***************************************************************************
Summary:
CRC Set Interrupt Handlers
***************************************************************************/
BERR_Code BAPE_Crc_SetInterruptHandlers(
    BAPE_CrcHandle handle,
    const BAPE_CrcInterruptHandlers *pInterrupts
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Crc);
    BDBG_ASSERT(NULL != pInterrupts);
    BKNI_EnterCriticalSection();
    handle->interrupts = *pInterrupts;
    BKNI_LeaveCriticalSection();
    return BERR_SUCCESS;
}

#endif

static void BAPE_Crc_P_DataReady_isr(BAPE_Handle deviceHandle)
{
    unsigned i, j;

    BDBG_MSG(("%s - checking for new CRC data", __FUNCTION__));
    for ( i = 0; i < BAPE_CHIP_MAX_CRCS; i++)
    {
        if ( deviceHandle->crcs[i] && deviceHandle->crcs[i]->started )
        {
            bool wroteData = false;
            BAPE_CrcHandle handle = deviceHandle->crcs[i];

            BDBG_MSG(("%s - Processing APE CRC idx %d, numChannelPairs %d", __FUNCTION__, i, handle->settings.numChannelPairs));
            for ( j = 0; j < handle->settings.numChannelPairs; j++ )
            {
                uint32_t regAddr;
                BAPE_CrcEntry entry;
                bool enabled;

                regAddr = BAPE_Reg_P_GetArrayAddress(AUD_MISC_CRC_CFGi, handle->resources[j].hwIndex);
                enabled = (BAPE_Reg_P_ReadField_isr(deviceHandle, regAddr, AUD_MISC_CRC_CFGi, CRC_ENA) != 0);

                /*BDBG_MSG(("\tCRC enabled=%d", enabled));*/
                if ( enabled ) {
                    unsigned written = 0;
                    regAddr = BAPE_Reg_P_GetArrayAddress(AUD_MISC_CRC_STATUSi, handle->resources[j].hwIndex);

                    entry.seqNumber = BAPE_Reg_P_ReadField_isr(deviceHandle, regAddr, AUD_MISC_CRC_STATUSi, CRC_CNTR);
                    entry.value = BAPE_Reg_P_ReadField_isr(deviceHandle, regAddr, AUD_MISC_CRC_STATUSi, CRC_VALUE);
                   if ( (handle->last[j].seqNumber == 0 || handle->last[j].seqNumber != entry.seqNumber) &&
                        entry.seqNumber != 0 )
                    {
                        BDBG_MSG(("\tCRC %d, writing seqNumber %d, value %d", handle->index, entry.seqNumber, entry.value));
                        written = BAPE_Buffer_Write_isr(handle->resources[j].buffer, &entry, sizeof(BAPE_CrcEntry));
                        wroteData = true;

                        if ( written < sizeof(BAPE_CrcEntry) )
                        {
                            BDBG_WRN(("%s - WARNING - only wrote %u of %lu bytes to CRC software Rbuf", __FUNCTION__, written, (unsigned long)sizeof(BAPE_CrcEntry)));
                        }
                    }
                    else
                    {
                        BDBG_MSG(("\tNo new data for CRC %d/%d - last->seqNumber %d, entry.seqNumber %d", i, j, handle->last[j].seqNumber, entry.seqNumber));
                    }
                    handle->last[j] = entry;
                }
                else
                {
                    BDBG_WRN(("%s - WARNING - hw crc %d disabled, used by ape crc %d, index %d", __FUNCTION__, handle->resources[j].hwIndex, i, j));
                }
            }

            if ( wroteData && handle->interrupts.dataReady.pCallback_isr )
            {
                handle->interrupts.dataReady.pCallback_isr(handle->interrupts.dataReady.pParam1, handle->interrupts.dataReady.param2);
            }
        }
    }
}

/**************************************************************************/

static void BAPE_P_CRC_isr (
        void * pParam1, /* [in] Ape deviceHandle */
        int    iParam2  /* [in] Not used */        
)
{
    BAPE_Handle     deviceHandle;
    uint32_t        ui32IntStatus=0;
    uint32_t        ui32MaskStatus=0;
    
    BDBG_ENTER (BAPE_P_CRC_isr);

    BKNI_ASSERT_ISR_CONTEXT();
    BDBG_ASSERT (pParam1);
    BSTD_UNUSED(iParam2);
        
    deviceHandle = pParam1;
    BDBG_OBJECT_ASSERT(deviceHandle, BAPE_Device);

    #if defined BCHP_AUD_MISC_CRC_ESR_STATUS
        ui32IntStatus = BAPE_Reg_P_Read_isr(deviceHandle,  BCHP_AUD_MISC_CRC_ESR_STATUS);
        ui32MaskStatus = BAPE_Reg_P_Read_isr(deviceHandle, BCHP_AUD_MISC_CRC_ESR_MASK);
    #endif

    ui32IntStatus &= ~ui32MaskStatus;
            
    BDBG_MSG(("CRC_ISR: ESR_STATUS (unmasked): 0x%x", ui32IntStatus));

    if (ui32IntStatus)
    {
        #if defined BCHP_AUD_MISC_CRC_ESR_STATUS
            BAPE_Reg_P_Write_isr(deviceHandle, BCHP_AUD_MISC_CRC_ESR_STATUS_CLEAR, ui32IntStatus);
        #endif

        BAPE_Crc_P_DataReady_isr(deviceHandle);
    }
    BDBG_LEAVE (BAPE_P_CRC_isr);
    return;    
}
 
#else /* BAPE_CHIP_MAX_CRCS */

/***************************************************************************
Summary:
CRC Get Default Settings
***************************************************************************/
void BAPE_Crc_GetDefaultOpenSettings(
    BAPE_CrcOpenSettings *pSettings /*[out] */
    )
{
    BSTD_UNUSED(pSettings);
}

/***************************************************************************
Summary:
CRC Get Default Settings
***************************************************************************/
void BAPE_Crc_GetDefaultInputSettings(
    BAPE_CrcSourceType sourceType,
    BAPE_CrcInputSettings *pInputSettings /*[out] */
    )
{
    BSTD_UNUSED(sourceType);
    BSTD_UNUSED(pInputSettings);
}

/***************************************************************************
Summary:
CRC Open
***************************************************************************/
BERR_Code BAPE_Crc_Open(
    BAPE_Handle deviceHandle,
    unsigned index, 
    const BAPE_CrcOpenSettings * pSettings, 
    BAPE_CrcHandle * pHandle /* [out] */
    )
{
    BSTD_UNUSED(deviceHandle);
    BSTD_UNUSED(index);
    BSTD_UNUSED(pSettings);

    *pHandle = NULL;

    return BERR_NOT_SUPPORTED;
}

/***************************************************************************
Summary:
CRC Close
***************************************************************************/
BERR_Code BAPE_Crc_Close(
    BAPE_CrcHandle handle
    )
{
    BSTD_UNUSED(handle);

    return BERR_NOT_SUPPORTED;
}
 
/***************************************************************************
Summary:
CRC AddInput
***************************************************************************/
BERR_Code BAPE_Crc_AddInput(
    BAPE_CrcHandle handle, 
    BAPE_CrcSourceType sourceType,
    const BAPE_CrcInputSettings * pInputSettings
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(sourceType);
    BSTD_UNUSED(pInputSettings);

    return BERR_NOT_SUPPORTED;
}

/***************************************************************************
Summary:
CRC RemoveInput
***************************************************************************/
BERR_Code BAPE_Crc_RemoveInput(
    BAPE_CrcHandle handle
    )
{
    BSTD_UNUSED(handle);

    return BERR_NOT_SUPPORTED;
}

/***************************************************************************
Summary:
CRC Start
***************************************************************************/
BERR_Code BAPE_Crc_P_Start(
    BAPE_CrcHandle handle
    )
{
    BSTD_UNUSED(handle);

    return BERR_NOT_SUPPORTED;
}

/***************************************************************************
Summary:
CRC Stop
***************************************************************************/
void BAPE_Crc_P_Stop(
    BAPE_CrcHandle handle
    )
{
    BSTD_UNUSED(handle);
}
 
/***************************************************************************
Summary:
CRC Get Buffer
***************************************************************************/
BERR_Code BAPE_Crc_GetBuffer(
    BAPE_CrcHandle handle, 
    BAPE_BufferDescriptor * pBuffers /* [out] */
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pBuffers);

    return BERR_NOT_SUPPORTED;
}

/***************************************************************************
Summary:
CRC Consume Data
***************************************************************************/
BERR_Code BAPE_Crc_ConsumeData(
    BAPE_CrcHandle handle, 
    unsigned numBytes
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(numBytes);

    return BERR_NOT_SUPPORTED;
}


#if !B_REFSW_MINIMAL
/***************************************************************************
Summary:
CRC Get Interrupt Handlers
***************************************************************************/
void BAPE_Crc_GetInterruptHandlers(
    BAPE_CrcHandle handle,
    BAPE_CrcInterruptHandlers *pInterrupts /* [out] */
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pInterrupts);
}

/***************************************************************************
Summary:
CRC Set Interrupt Handlers
***************************************************************************/
BERR_Code BAPE_Crc_SetInterruptHandlers(
    BAPE_CrcHandle handle,
    const BAPE_CrcInterruptHandlers *pInterrupts
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pInterrupts);

    return BERR_NOT_SUPPORTED;
}
#endif

#endif /* BAPE_CHIP_MAX_CRCS */