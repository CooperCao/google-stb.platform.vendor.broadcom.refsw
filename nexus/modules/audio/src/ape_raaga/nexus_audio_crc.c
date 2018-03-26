/***************************************************************************
*  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
*   API name: AudioCrc
*    CRC capture for Audio data
*
***************************************************************************/

#include "nexus_audio_module.h"
#include "nexus_client_resources.h"

BDBG_MODULE(nexus_audio_crc);

#if NEXUS_NUM_AUDIO_CRCS > 0
typedef struct NEXUS_AudioCrc
{
    NEXUS_OBJECT(NEXUS_AudioCrc);

    bool opened;
    BAPE_CrcHandle apeHandle;
    
    NEXUS_AudioCrcOpenSettings settings;
    NEXUS_AudioCrcInputSettings inputSettings;

#define RESOLVE_ALIAS(handle) do {(handle) = ((handle)->alias.master?(handle)->alias.master:(handle));}while(0)
#define IS_ALIAS(handle) ((handle)->alias.master != NULL)
    struct {
        NEXUS_AudioCrcHandle master, slave;
    } alias;
} NEXUS_AudioCrc;

static NEXUS_AudioCrc g_crc[NEXUS_MAX_AUDIO_CRC_OUTPUTS];

#define min(A,B) ((A)<(B)?(A):(B))

/***************************************************************************
Summary:
    Get Default CRC Open Settings

Description:
***************************************************************************/
void NEXUS_AudioCrc_GetDefaultOpenSettings(
    NEXUS_AudioCrcOpenSettings *pSettings   /* [out] default settings */
    )
{
    BAPE_CrcOpenSettings piSettings;
    BDBG_ASSERT(pSettings != NULL);

    BAPE_Crc_GetDefaultOpenSettings(&piSettings);
    pSettings->captureMode = NEXUS_AudioCrcMode_eFreeRun;
    pSettings->numEntries = 48000/256;
    pSettings->samplingPeriod = 48000/2;
    pSettings->dataWidth = 24;
    pSettings->initialValue = 0;
    pSettings->numChannelPairs = 1;
}

/***************************************************************************
Summary:
    Open Audio CRC 

Description:
    Open an Audio CRC.  Audio chain must be stopped to 
    perform this operation.
***************************************************************************/
NEXUS_AudioCrcHandle NEXUS_AudioCrc_Open(
    unsigned index,                                 /* index to CRC instance */
    const NEXUS_AudioCrcOpenSettings *pSettings     /* attr{null_allowed=y} Pass NULL for default settings */
    )
{
    NEXUS_AudioCrcHandle handle;
    NEXUS_AudioCrcHandle master = NULL;
    const NEXUS_AudioCrcOpenSettings * pOpenSettings;
    NEXUS_AudioCrcOpenSettings defaults;
    BAPE_CrcOpenSettings apeOpenSettings;
    BERR_Code errCode;
    unsigned org_index = index;
    NEXUS_AudioCapabilities audioCapabilities;
    unsigned outputCount;

    BDBG_CASSERT((int)NEXUS_AudioCrcSourceType_ePlaybackBuffer == (int)BAPE_CrcSourceType_ePlaybackBuffer);
    BDBG_CASSERT((int)NEXUS_AudioCrcSourceType_eOutputPort == (int)BAPE_CrcSourceType_eOutputPort);
    BDBG_CASSERT((int)NEXUS_AudioCrcSourceType_eMax == (int)BAPE_CrcSourceType_eMax);

    BDBG_CASSERT((int)NEXUS_AudioCrcMode_eFreeRun == (int)BAPE_CrcMode_eFreeRun);
    BDBG_CASSERT((int)NEXUS_AudioCrcMode_eSingle == (int)BAPE_CrcMode_eSingle);
    BDBG_CASSERT((int)NEXUS_AudioCrcMode_eMax == (int)BAPE_CrcMode_eMax);

    NEXUS_GetAudioCapabilities(&audioCapabilities);
    outputCount = min(audioCapabilities.numCrcs, NEXUS_NUM_AUDIO_CRCS);

    errCode = NEXUS_CLIENT_RESOURCES_ACQUIRE(audioCrc,IdList,org_index);
    if (errCode) { errCode = BERR_TRACE(errCode); goto err_acquire; }

    if (index >= NEXUS_ALIAS_ID && index-NEXUS_ALIAS_ID < outputCount) {
        BDBG_MSG(("%d aliasing %d(%p)", index, index-NEXUS_ALIAS_ID, (void *)&g_crc[index-NEXUS_ALIAS_ID]));
        index -= NEXUS_ALIAS_ID;
        master = &g_crc[index];
        if (!master->opened) {
            BDBG_ERR(("cannot alias %d because it is not opened", index));
            goto err_index;
        }
        if (master->alias.slave) {
            BDBG_ERR(("cannot alias %d a second time", index));
            goto err_index;
        }
    }
    if ( index >= outputCount )
    {
        BDBG_ERR(("index out of range."));
        (void)BERR_TRACE(BERR_INVALID_PARAMETER);
        goto err_index;
    }

    if (!master) {
        handle = &g_crc[index];
        if ( handle->opened )
        {
            BDBG_ERR(("Audio CRC %u already open", index));
            (void)BERR_TRACE(BERR_NOT_SUPPORTED);
            goto err_index;
        }
    }
    else {
        handle = BKNI_Malloc(sizeof(NEXUS_AudioCrc));
        if ( !handle )
        {
            (void)BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
            goto err_index;
        }
        BKNI_Memset(handle, 0, sizeof(*handle));
    }

    NEXUS_OBJECT_SET(NEXUS_AudioCrc, handle);

    if (master) {
        handle->alias.master = master;
        master->alias.slave = handle;
        return handle;
    }

    if ( pSettings == NULL )
    {
        NEXUS_AudioCrc_GetDefaultOpenSettings(&defaults);
        pOpenSettings = &defaults;
    }
    else
    {
        pOpenSettings = pSettings;
    }

    BAPE_Crc_GetDefaultOpenSettings(&apeOpenSettings);
    switch ( pOpenSettings->dataWidth )
    {
        case 16:
            apeOpenSettings.dataWidth = BAPE_CrcDataWidth_e16;
            break;
        case 20:
            apeOpenSettings.dataWidth = BAPE_CrcDataWidth_e16;
            break;
        case 24:
            apeOpenSettings.dataWidth = BAPE_CrcDataWidth_e16;
            break;
        default:
            (void)BERR_TRACE(BERR_INVALID_PARAMETER);
            goto err_open;
            break;
    }
    apeOpenSettings.bufferSize = pOpenSettings->numEntries * sizeof(BAPE_CrcEntry);
    apeOpenSettings.initialValue = pOpenSettings->initialValue;
    apeOpenSettings.memHandle = NULL;
    apeOpenSettings.numChannelPairs = pOpenSettings->numChannelPairs;
    apeOpenSettings.samplingPeriod = pOpenSettings->samplingPeriod;

    errCode = BAPE_Crc_Open(NEXUS_AUDIO_DEVICE_HANDLE, index, &apeOpenSettings, &handle->apeHandle);
    if ( errCode )
    {
        (void)BERR_TRACE(errCode);
        goto err_open;
    }
    handle->opened = true;
    handle->inputSettings.sourceType = NEXUS_AudioCrcSourceType_eMax;
    handle->settings = *pOpenSettings;

    return handle;

err_open:
    BDBG_OBJECT_DESTROY(handle, NEXUS_AudioCrc);
err_index:
    NEXUS_CLIENT_RESOURCES_RELEASE(audioCapture,IdList,org_index);
err_acquire:
    return NULL;

}

/***************************************************************************
Summary:
    Close Audio CRC 

Description:
    Close this CRC.  Audio chain must be stopped, and CRC input must be 
    removed to perform this operation.
***************************************************************************/
static void NEXUS_AudioCrc_P_Finalizer(
    NEXUS_AudioCrcHandle handle
    )
{
    BERR_Code errCode;
    unsigned index;
    NEXUS_OBJECT_ASSERT(NEXUS_AudioCrc, handle);

    if (handle->alias.master) {
        index = NEXUS_ALIAS_ID + (handle->alias.master - g_crc);
        handle->alias.master->alias.slave = NULL;
        NEXUS_CLIENT_RESOURCES_RELEASE(audioCrc,IdList, index);
        NEXUS_OBJECT_DESTROY(NEXUS_AudioCrc, handle);
        BKNI_Free(handle);
        return;
    }
    else {
        /* close slave */
        if (handle->alias.slave) {
            NEXUS_OBJECT_UNREGISTER(NEXUS_AudioCrc, handle->alias.slave, Close);
            NEXUS_AudioCrc_Close(handle->alias.slave);
        }
    }

    BDBG_ASSERT(handle->opened);
    BDBG_ASSERT(handle->apeHandle != NULL);

    errCode = BAPE_Crc_Close(handle->apeHandle);
    if ( errCode )
    {
        BERR_TRACE(errCode);
    }
    handle->opened = false;
    index = handle - g_crc;
    NEXUS_CLIENT_RESOURCES_RELEASE(audioCrc,IdList,index);
    BKNI_Memset(handle, 0, sizeof(NEXUS_AudioCrc));
}

static void NEXUS_AudioCrc_P_Release( NEXUS_AudioCrcHandle handle )
{
    if (!IS_ALIAS(handle)) {
        NEXUS_OBJECT_UNREGISTER(NEXUS_AudioCrc, handle, Close);
    }
    return;
}

NEXUS_OBJECT_CLASS_MAKE_WITH_RELEASE(NEXUS_AudioCrc, NEXUS_AudioCrc_Close);

/***************************************************************************
Summary:
    Get Default CRC Input Settings

Description:
***************************************************************************/
void NEXUS_AudioCrc_GetDefaultInputSettings(
    NEXUS_AudioCrcInputSettings *pSettings   /* [out] default settings */
    )
{
    BDBG_ASSERT(pSettings != NULL);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->sourceType = NEXUS_AudioCrcSourceType_eMax;
}

/***************************************************************************
Summary:
    Set the input for this CRC 

Description:
    Add the input for this CRC.  Audio chain must be stopped to 
    perform this operation.
***************************************************************************/
NEXUS_Error NEXUS_AudioCrc_SetInput(
    NEXUS_AudioCrcHandle handle,
    const NEXUS_AudioCrcInputSettings * pSettings
    )
{
    NEXUS_OBJECT_ASSERT(NEXUS_AudioCrc, handle);
    RESOLVE_ALIAS(handle);
    BDBG_ASSERT(handle->apeHandle != NULL);
    BDBG_ASSERT(pSettings != NULL);

    if ( pSettings->sourceType == NEXUS_AudioCrcSourceType_eOutputPort )
    {
        BDBG_OBJECT_ASSERT(pSettings->output, NEXUS_AudioOutput);
        pSettings->output->pCrc = handle;
    }
    else
    {
        BDBG_OBJECT_ASSERT(pSettings->input, NEXUS_AudioInput);
        BDBG_OBJECT_ASSERT(pSettings->output, NEXUS_AudioOutput);
        pSettings->input->pCrc = handle;
        pSettings->output->pCrc = handle;
    }

    handle->inputSettings = *pSettings;

    return BERR_SUCCESS;
}

/***************************************************************************
Summary:
    Clear the input from this CRC 

Description:
    Removes the input from this CRC.  Audio chain should be stopped to 
    perform this operation.
***************************************************************************/
void NEXUS_AudioCrc_ClearInput(
    NEXUS_AudioCrcHandle handle
    )
{
    NEXUS_Error errCode;
    NEXUS_OBJECT_ASSERT(NEXUS_AudioCrc, handle);
    RESOLVE_ALIAS(handle);
    BDBG_ASSERT(handle->apeHandle != NULL);

    errCode = BAPE_Crc_RemoveInput(handle->apeHandle);
    if ( errCode )
    {
        BERR_TRACE(errCode);
        return;
    }

    if ( handle->inputSettings.sourceType == NEXUS_AudioCrcSourceType_eOutputPort )
    {
        if ( handle->inputSettings.output )
        {
            handle->inputSettings.output->pCrc = NULL;
        }
    }
    else
    {
        if ( handle->inputSettings.output )
        {
            handle->inputSettings.output->pCrc = NULL;
        }
        if ( handle->inputSettings.input )
        {
            handle->inputSettings.input->pCrc = NULL;
        }
    }

    BKNI_Memset(&handle->inputSettings, 0, sizeof(handle->inputSettings));
    handle->inputSettings.sourceType = NEXUS_AudioCrcSourceType_eMax;
}


static NEXUS_AudioCrcEntry * NEXUS_AudioCrc_P_GetEntryByIndex(
    const NEXUS_AudioCrcData * crcData,
    unsigned index
    )

{
    BDBG_ASSERT(crcData != NULL);

    switch ( index )
    {
        default:
        case 0:
            return (NEXUS_AudioCrcEntry*)&crcData->crc0;
            break;
        case 1:
            return (NEXUS_AudioCrcEntry*)&crcData->crc1;
            break;
        case 2:
            return (NEXUS_AudioCrcEntry*)&crcData->crc2;
            break;
        case 3:
            return (NEXUS_AudioCrcEntry*)&crcData->crc3;
            break;
    }

    return NULL;
}

/***************************************************************************
Summary:
    Get a buffer descriptor containing the CRC buffer(s).

Description:
    Get CRC entries. numEntries will always be populated ( >= 0 ), 
    even if CRC input is not connected. An error will also be
    returned if there is no active CRC input.

    NEXUS_AudioCrc_GetCrcEntries is non-destructive. You can safely call it 
    multiple times.
***************************************************************************/
NEXUS_Error NEXUS_AudioCrc_GetCrcData(
    NEXUS_AudioCrcHandle handle,
    NEXUS_AudioCrcData *pData, /* [out] attr{nelem=numEntries;nelem_out=pNumReturned} pointer to CRC entries. */
    unsigned numEntries,
    unsigned *pNumReturned
    )
{
    NEXUS_Error errCode;
    BAPE_BufferDescriptor apeDescriptor;
    unsigned i,j;

    NEXUS_OBJECT_ASSERT(NEXUS_AudioCrc, handle);
    RESOLVE_ALIAS(handle);
    BDBG_ASSERT(pData != NULL);
    BDBG_ASSERT(handle->apeHandle != NULL);
    BDBG_ASSERT(pNumReturned != NULL);

    BKNI_Memset(pData, 0, sizeof(*pData));
    *pNumReturned = 0;

    errCode = BAPE_Crc_GetBuffer(handle->apeHandle, &apeDescriptor);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    if ( apeDescriptor.bufferSize > 0 )
    {
        unsigned numBuffers = apeDescriptor.numBuffers;
        unsigned baseEntries = apeDescriptor.bufferSize / sizeof(BAPE_CrcEntry);
        unsigned wrapEntries = apeDescriptor.wrapBufferSize / sizeof(BAPE_CrcEntry);

        if ( numEntries > (baseEntries + wrapEntries) )
        {
            numEntries = (baseEntries + wrapEntries);
        }

        for ( j = 0; (j < baseEntries) && (*pNumReturned < numEntries); j++ )
        {
            for ( i = 0; i < numBuffers; i++ )
            {
                NEXUS_AudioCrcEntry * entry;
                BAPE_CrcEntry * apeEntry;

                entry = NEXUS_AudioCrc_P_GetEntryByIndex(&(pData[*pNumReturned]), i);

                BDBG_ASSERT(apeDescriptor.buffers[i].pBuffer != NULL);
                apeEntry = (BAPE_CrcEntry*)apeDescriptor.buffers[i].pBuffer + j;
                entry->seqNumber = apeEntry->seqNumber;
                entry->value = apeEntry->value;
            }
            (*pNumReturned)++;
        }

        for ( j = 0; (j < wrapEntries) && (*pNumReturned < numEntries); j++ )
        {
            for ( i = 0; i < numBuffers; i++ )
            {
                NEXUS_AudioCrcEntry * entry;
                BAPE_CrcEntry * apeEntry;

                entry = NEXUS_AudioCrc_P_GetEntryByIndex(&(pData[*pNumReturned]), i);

                BDBG_ASSERT(apeDescriptor.buffers[i].pWrapBuffer != NULL);
                apeEntry = (BAPE_CrcEntry*)apeDescriptor.buffers[i].pWrapBuffer + j;
                entry->seqNumber = apeEntry->seqNumber;
                entry->value = apeEntry->value;
            }
            (*pNumReturned)++;
        }
    }

    errCode = BAPE_Crc_ConsumeData(handle->apeHandle, *pNumReturned * sizeof(BAPE_CrcEntry));
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    return BERR_SUCCESS;
}



/***************************************************************************
Summary:
    Get the current input settings.

Description:
    Get the current input settings.

    If no inputs are connected, an Error will be returned.
***************************************************************************/
NEXUS_Error NEXUS_AudioCrc_P_GetCurrentInputSettings(
    NEXUS_AudioCrcHandle handle,
    NEXUS_AudioCrcInputSettings * inputSettings /* [out] */
    )
{
    NEXUS_OBJECT_ASSERT(NEXUS_AudioCrc, handle);
    RESOLVE_ALIAS(handle);
    BDBG_ASSERT(inputSettings != NULL);

    *inputSettings = handle->inputSettings;
    return BERR_SUCCESS;
}

/***************************************************************************
Summary:
    Get the APE CRC handle
 ***************************************************************************/
BAPE_CrcHandle NEXUS_AudioCrc_P_GetPIHandle(
    NEXUS_AudioCrcHandle handle
    )
{
    NEXUS_OBJECT_ASSERT(NEXUS_AudioCrc, handle);
    RESOLVE_ALIAS(handle);

    return handle->apeHandle;
}

#else /* Stub */
typedef struct NEXUS_AudioCrc
{
    NEXUS_OBJECT(NEXUS_AudioCrc);
} NEXUS_AudioCrc;


void NEXUS_AudioCrc_GetDefaultOpenSettings(
    NEXUS_AudioCrcOpenSettings *pSettings   /* [out] default settings */
    )
{
    BSTD_UNUSED(pSettings);
}

NEXUS_AudioCrcHandle NEXUS_AudioCrc_Open(
    unsigned index,                                 /* index to CRC instance */
    const NEXUS_AudioCrcOpenSettings *pSettings     /* attr{null_allowed=y} Pass NULL for default settings */
    )
{
    BSTD_UNUSED(index);
    BSTD_UNUSED(pSettings);
    (void)BERR_TRACE(BERR_NOT_SUPPORTED);
    return NULL;
}

void NEXUS_AudioCrc_Close(
    NEXUS_AudioCrcHandle handle
    )
{
    BSTD_UNUSED(handle);
}

NEXUS_OBJECT_CLASS_MAKE(NEXUS_AudioCrc, NEXUS_AudioCrc_Destroy);

static void NEXUS_AudioCrc_P_Finalizer(
    NEXUS_AudioCrcHandle handle
    )
{
    BSTD_UNUSED(handle);
}

void NEXUS_AudioCrc_GetDefaultInputSettings(
    NEXUS_AudioCrcInputSettings *pSettings   /* [out] default settings */
    )
{
    BSTD_UNUSED(pSettings);
}

NEXUS_Error NEXUS_AudioCrc_SetInput(
    NEXUS_AudioCrcHandle handle,
    const NEXUS_AudioCrcInputSettings * pSettings
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

void NEXUS_AudioCrc_ClearInput(
    NEXUS_AudioCrcHandle handle
    )
{
    BSTD_UNUSED(handle);
    (void)BERR_TRACE(BERR_NOT_SUPPORTED);
}

NEXUS_Error NEXUS_AudioCrc_GetCrcData(
    NEXUS_AudioCrcHandle handle,
    NEXUS_AudioCrcData *pData, /* [out] attr{nelem=numEntries;nelem_out=pNumReturned} pointer to CRC entries. */
    unsigned numEntries,
    unsigned *pNumReturned
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pData);
    BSTD_UNUSED(numEntries);
    BSTD_UNUSED(pNumReturned);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

NEXUS_Error NEXUS_AudioCrc_P_GetCurrentInputSettings(
    NEXUS_AudioCrcHandle handle,
    NEXUS_AudioCrcInputSettings * inputSettings /* [out] */
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(inputSettings);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

BAPE_CrcHandle NEXUS_AudioCrc_P_GetPIHandle(
    NEXUS_AudioCrcHandle handle
    )
{
    BSTD_UNUSED(handle);

    return NULL;
}

#endif
