/******************************************************************************
 * Copyright (C) 2006-2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 * Module Description: Dummysink Output Interface
 *
 ***************************************************************************/

#include "bstd.h"
#include "bkni.h"
#include "bape.h"
#include "bape_priv.h"

BDBG_MODULE(bape_dummy_output);

BDBG_OBJECT_ID(BAPE_DummyOutput);

typedef struct BAPE_DummyOutput
{
    BDBG_OBJECT(BAPE_DummyOutput)
    BAPE_Handle deviceHandle;
    unsigned index;
    BAPE_OutputPortObject outputPort;
    BAPE_DummysinkGroupHandle hDummysinkGroup;
#if BAPE_CHIP_MAX_FS > 0
    unsigned fsNum;
#else
    BAPE_MclkSource mclkSource;
    unsigned pllChannel;
    unsigned mclkFreqToFsRatio;
#endif
    bool enabled;
    char name[14];   /* DummyOutput %d */
} BAPE_DummyOutput;

/* Static function prototypes */
static BERR_Code BAPE_DummyOutput_P_OpenHw(BAPE_DummyOutputHandle handle);

/* Output port callbacks */
static BERR_Code BAPE_DummyOutput_P_Enable(BAPE_OutputPort output);
static void BAPE_DummyOutput_P_Disable(BAPE_OutputPort output);
static void BAPE_DummyOutput_P_GetEnableParams(BAPE_OutputPort output, bool enable, BAPE_OutputPort_P_EnableParams * params);

/* Timing callbacks */
#if BAPE_CHIP_MAX_FS > 0
static void BAPE_DummyOutput_P_SetFs(BAPE_OutputPort output, unsigned fsNum);
#else
static void BAPE_DummyOutput_P_SetMclk_isr(BAPE_OutputPort output, BAPE_MclkSource mclkSource, unsigned pllChannel, unsigned mclkFreqToFsRatio);
#endif

/***************************************************************************
        Public APIs: From bape_output.h
***************************************************************************/

void BAPE_DummyOutput_GetDefaultOpenSettings(
    BAPE_DummyOutputOpenSettings *pSettings       /* [out] */
    )
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));    

    pSettings->maxMultichannelFormat = BAPE_MultichannelFormat_e2_0;
}

/**************************************************************************/

BERR_Code BAPE_DummyOutput_Open(
    BAPE_Handle deviceHandle,
    unsigned index,
    const BAPE_DummyOutputOpenSettings *pSettings,
    BAPE_DummyOutputHandle *pHandle             /* [out] */
    )
{
    BERR_Code errCode;
    BAPE_DummyOutputHandle handle;
    unsigned numChannelPairs=1;
    BAPE_DummysinkGroupCreateSettings dummysinkCreateSettings;
    BAPE_DummyOutputOpenSettings defaultSettings;

    BDBG_OBJECT_ASSERT(deviceHandle, BAPE_Device);
    BDBG_ASSERT(NULL != pHandle);

    BDBG_MSG(("%s: Opening DummyOutput: %u", BSTD_FUNCTION, index));

    if ( index >= BAPE_CHIP_MAX_DUMMYSINKS )
    {
        BDBG_ERR(("Unable to open dummysink %u.  Max dummysinks is %u on this platform.", index, BAPE_CHIP_MAX_DUMMYSINKS));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if ( deviceHandle->dummyOutputs[index] )
    {
        BDBG_ERR(("DummyOutput %d already open", index));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* Init to specified settings */
    if ( NULL == pSettings )
    {
        BAPE_DummyOutput_GetDefaultOpenSettings(&defaultSettings);
        pSettings = &defaultSettings;
    }

    /* Allocate the device structure, then fill in all the fields. */
    handle = BKNI_Malloc(sizeof(BAPE_DummyOutput));
    if ( NULL == handle )
    {
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }
    BKNI_Memset(handle, 0, sizeof(BAPE_DummyOutput));
    BDBG_OBJECT_SET(handle, BAPE_DummyOutput);
    handle->deviceHandle = deviceHandle;
    handle->index = index;
    BAPE_P_InitOutputPort(&handle->outputPort, BAPE_OutputPortType_eDummyOutput, index, handle);
    BAPE_FMT_P_EnableSource(&handle->outputPort.capabilities, BAPE_DataSource_eFci);
    BAPE_FMT_P_EnableType(&handle->outputPort.capabilities, BAPE_DataType_ePcmStereo);
    BAPE_FMT_P_EnableType(&handle->outputPort.capabilities, BAPE_DataType_ePcmRf);
    BAPE_FMT_P_EnableType(&handle->outputPort.capabilities, BAPE_DataType_eIec61937);
    BAPE_FMT_P_EnableType(&handle->outputPort.capabilities, BAPE_DataType_eIec61937x4);
    switch ( pSettings->maxMultichannelFormat )
    {
    case BAPE_MultichannelFormat_e2_0:
        numChannelPairs = 1;
        break;
    case BAPE_MultichannelFormat_e5_1:
        numChannelPairs = 3;
        BAPE_FMT_P_EnableType(&handle->outputPort.capabilities, BAPE_DataType_ePcm5_1);
        break;
    case BAPE_MultichannelFormat_e7_1:
        numChannelPairs = 4;
        BAPE_FMT_P_EnableType(&handle->outputPort.capabilities, BAPE_DataType_ePcm5_1);
        BAPE_FMT_P_EnableType(&handle->outputPort.capabilities, BAPE_DataType_ePcm7_1);
        break;
    default:
        BDBG_ERR(("Unsupported multichannel format %u", pSettings->maxMultichannelFormat));
        BAPE_DummyOutput_Close(handle);
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    handle->outputPort.enable = BAPE_DummyOutput_P_Enable;
    handle->outputPort.disable = BAPE_DummyOutput_P_Disable;
    handle->outputPort.getEnableParams = BAPE_DummyOutput_P_GetEnableParams;
#if BAPE_CHIP_MAX_FS > 0
    handle->outputPort.fsTiming = true;
    handle->outputPort.setFs = BAPE_DummyOutput_P_SetFs;
    handle->fsNum = (unsigned)-1;
#else
    handle->outputPort.setMclk_isr = BAPE_DummyOutput_P_SetMclk_isr;
    handle->mclkSource = BAPE_MclkSource_eNone;
    handle->pllChannel = 0;
    handle->mclkFreqToFsRatio = BAPE_BASE_PLL_TO_FS_RATIO;
#endif
    BKNI_Snprintf(handle->name, sizeof(handle->name), "Dummysink %u", index); 
    handle->outputPort.pName = handle->name;

    BAPE_DummysinkGroup_P_GetDefaultCreateSettings(&dummysinkCreateSettings);
    dummysinkCreateSettings.numChannelPairs = numChannelPairs;
    errCode = BAPE_DummysinkGroup_P_Create(deviceHandle, &dummysinkCreateSettings, &handle->hDummysinkGroup);
    if ( errCode )
    {
        BAPE_DummyOutput_Close(handle);
        return BERR_TRACE(errCode);
    }

    errCode = BAPE_DummyOutput_P_OpenHw(handle);
    if ( errCode )
    {
        BAPE_DummyOutput_Close(handle);
        return BERR_TRACE(errCode);
    }

    *pHandle = handle;
    handle->deviceHandle->dummyOutputs[index] = handle;
    return BERR_SUCCESS;
}

/**************************************************************************/

void BAPE_DummyOutput_Close(
    BAPE_DummyOutputHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_DummyOutput);

    if ( handle->hDummysinkGroup )
    {
        BAPE_DummysinkGroup_P_Destroy(handle->hDummysinkGroup);
    }

    handle->deviceHandle->dummyOutputs[handle->index] = NULL;
    BDBG_OBJECT_DESTROY(handle, BAPE_DummyOutput);
    BKNI_Free(handle);
}

/**************************************************************************/

void BAPE_DummyOutput_GetOutputPort(
    BAPE_DummyOutputHandle handle,
    BAPE_OutputPort *pOutputPort
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_DummyOutput);
    BDBG_ASSERT(NULL != pOutputPort);
    *pOutputPort = &handle->outputPort;
}

/***************************************************************************
        BAPE Internal APIs: From bape_fmm_priv.h
***************************************************************************/

BERR_Code BAPE_DummyOutput_P_ResumeFromStandby(BAPE_Handle bapeHandle)
{
    BERR_Code   errCode = BERR_SUCCESS;
    unsigned    dummyOutputIndex;

    BDBG_OBJECT_ASSERT(bapeHandle, BAPE_Device);

    /* For each opened DummyOutput, call the functions necessary to restore the
     * hardware to it's appropriate state.
     */
    for ( dummyOutputIndex=0 ; dummyOutputIndex<BAPE_CHIP_MAX_DUMMYSINKS ; dummyOutputIndex++ )
    {
        if ( bapeHandle->dummyOutputs[dummyOutputIndex] )       /* If this DummyOutput is open... */
        {
            BAPE_DummyOutputHandle hDummyOutput = bapeHandle->dummyOutputs[dummyOutputIndex];

            /* Put the HW into the generic open state. */
            errCode = BAPE_DummyOutput_P_OpenHw(hDummyOutput);
            if ( errCode ) return BERR_TRACE(errCode);
            
            /* Now apply changes for the settings struct. */
                /* Nothing to do for that because there's no SetSettings function. */

#if BAPE_CHIP_MAX_FS > 0
            /* Now restore the timebase and sampleRate from the values saved in the device struct. */
            if ( hDummyOutput->fsNum !=  (unsigned)-1 )
            {
                BAPE_DummyOutput_P_SetFs(&hDummyOutput->outputPort, hDummyOutput->fsNum);
            }
#else
            if ( hDummyOutput->mclkSource != BAPE_MclkSource_eNone )
            {
                BKNI_EnterCriticalSection();
                BAPE_DummyOutput_P_SetMclk_isr(&hDummyOutput->outputPort, hDummyOutput->mclkSource, hDummyOutput->pllChannel, hDummyOutput->mclkFreqToFsRatio);
                BKNI_LeaveCriticalSection();                
            }
#endif
        } /* end if this DummyOutput was open */
    } /* end for each DummyOutput */
    return errCode;
}


/***************************************************************************
        Private callbacks: Protyped above
***************************************************************************/
static BERR_Code BAPE_DummyOutput_P_Enable(BAPE_OutputPort output)
{
    BAPE_DummyOutputHandle handle;
    BAPE_DummysinkGroupSettings dummysinkSettings;
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(output, BAPE_OutputPort);

    handle = output->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_DummyOutput);

    BDBG_ASSERT(false == handle->enabled);

    /* Set Input Parameters */
    BAPE_DummysinkGroup_P_GetSettings_isrsafe(handle->hDummysinkGroup, &dummysinkSettings);
    dummysinkSettings.input = handle->outputPort.sourceMixerFci;

    BKNI_EnterCriticalSection();
    errCode = BAPE_DummysinkGroup_P_SetSettings_isr(handle->hDummysinkGroup, &dummysinkSettings);
    BKNI_LeaveCriticalSection();
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_dummysink_settings;
    }

    /* Enable each dummysink */
    errCode = BAPE_DummysinkGroup_P_Start(handle->hDummysinkGroup);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_dummysink_start;
    }

    /* Done */
    handle->enabled = true;

err_dummysink_start:    
err_dummysink_settings:
    return errCode;
}

/**************************************************************************/

static void BAPE_DummyOutput_P_Disable(BAPE_OutputPort output)
{
    BAPE_DummyOutputHandle handle;

    BDBG_OBJECT_ASSERT(output, BAPE_OutputPort);

    handle = output->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_DummyOutput);

    /* disable each dummysink */
    BAPE_DummysinkGroup_P_Stop(handle->hDummysinkGroup);

    /* Done */
    handle->enabled = false;
}

static void BAPE_DummyOutput_P_GetEnableParams(BAPE_OutputPort output, bool enable, BAPE_OutputPort_P_EnableParams* params)
{
    BAPE_DummyOutputHandle handle;

    BDBG_OBJECT_ASSERT(output, BAPE_OutputPort);

    handle = output->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_DummyOutput);

    BAPE_DummysinkGroup_P_GetOutputEnableParams(BAPE_DummyOutput_P_GetDummysinkGroup(handle), enable, params);
}

/**************************************************************************/

#if BAPE_CHIP_MAX_FS > 0
static void BAPE_DummyOutput_P_SetFs(BAPE_OutputPort output, unsigned fsNum)
{
    BAPE_DummyOutputHandle handle;
    BAPE_DummysinkGroupSettings dummysinkSettings;
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(output, BAPE_OutputPort);

    handle = output->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_DummyOutput);

    BDBG_ASSERT(false == handle->enabled);

    /* Set Input Parameters */
    BAPE_DummysinkGroup_P_GetSettings_isrsafe(handle->hDummysinkGroup, &dummysinkSettings);
    dummysinkSettings.fs = fsNum;

    BKNI_EnterCriticalSection();
    errCode = BAPE_DummysinkGroup_P_SetSettings_isr(handle->hDummysinkGroup, &dummysinkSettings);
    BKNI_LeaveCriticalSection();
    handle->fsNum = fsNum;

    BDBG_ASSERT(BERR_SUCCESS == errCode);
}
#else
static void BAPE_DummyOutput_P_SetMclk_isr(BAPE_OutputPort output, BAPE_MclkSource mclkSource, unsigned pllChannel, unsigned mclkFreqToFsRatio)
{
    BAPE_DummyOutputHandle handle;
    BAPE_DummysinkGroupSettings dummysinkSettings;
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(output, BAPE_OutputPort);

    handle = output->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_DummyOutput);

    /* Set Input Parameters */
    BAPE_DummysinkGroup_P_GetSettings_isrsafe(handle->hDummysinkGroup, &dummysinkSettings);
    dummysinkSettings.mclkSource = mclkSource;
    dummysinkSettings.pllChannel = pllChannel;
    dummysinkSettings.mclkFreqToFsRatio = mclkFreqToFsRatio;
    errCode = BAPE_DummysinkGroup_P_SetSettings_isr(handle->hDummysinkGroup, &dummysinkSettings);

    handle->mclkSource = mclkSource;
    handle->pllChannel = pllChannel;
    handle->mclkFreqToFsRatio = mclkFreqToFsRatio;

    BDBG_ASSERT(BERR_SUCCESS == errCode);
}
#endif

/***************************************************************************
        Private functions: Protyped above
***************************************************************************/

static BERR_Code BAPE_DummyOutput_P_OpenHw(BAPE_DummyOutputHandle handle)
{
    BDBG_OBJECT_ASSERT(handle, BAPE_DummyOutput);

    BSTD_UNUSED(handle);
    /* Nothing to do for now. */

    return BERR_SUCCESS;
}


BAPE_DummysinkGroupHandle BAPE_DummyOutput_P_GetDummysinkGroup(
    BAPE_DummyOutputHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_DummyOutput);

    return handle->hDummysinkGroup;
}
