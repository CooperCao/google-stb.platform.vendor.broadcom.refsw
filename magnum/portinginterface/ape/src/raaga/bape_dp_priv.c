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
 * Module Description: Audio PI Device Level Interface
 *
 ***************************************************************************/

#include "bstd.h"
#include "bkni.h"
#include "bape.h"
#include "bape_priv.h"
#include "bchp_aud_fmm_dp_ctrl0.h"

BDBG_MODULE(bape_dp_priv);
BDBG_FILE_MODULE(bape_mixer_input_coeffs);
BDBG_FILE_MODULE(bape_fci);

#define BAPE_PLAYBACK_ID_INVALID (0xffffffff)

#define BAPE_MIXER_DEFAULT_RAMP_STEP (0xa00)
#define BAPE_DP_MAX_RAMP_STEP (0x0800000)
#define BAPE_DP_DEFAULT_RAMP_INTERVAL (32) /* 32 microseconds for slow SR 32,000Hz */

/***************************************************************************
Summary:
Mixer Group
***************************************************************************/
typedef struct BAPE_MixerGroup
{
    bool allocated;
    uint8_t blockId;
    unsigned numChannelPairs;
    BAPE_Handle deviceHandle;
    uint32_t mixerIds[BAPE_ChannelPair_eMax];
    unsigned numRunningInputs;
    struct
    {
        BAPE_MixerGroupInputSettings settings;
        unsigned numChannelPairs;
        uint32_t playbackIds[BAPE_ChannelPair_eMax];
        bool started;
        bool linked;
    } inputs[BAPE_CHIP_MAX_MIXER_INPUTS];
    unsigned numRunningOutputs;
    struct
    {
        BAPE_MixerGroupOutputSettings settings;     
        bool started;
    } outputs[BAPE_CHIP_MAX_MIXER_OUTPUTS];
    BAPE_MixerGroupSettings settings;
    unsigned sampleRate;
} BAPE_MixerGroup;

static void BAPE_DpMixer_P_SetGroup(BAPE_Handle deviceHandle, uint32_t mixerId, uint32_t groupId);
static void BAPE_DpMixer_P_LoadInputCoefs(BAPE_Handle deviceHandle, uint32_t mixerId, unsigned inputIndex, 
                                          uint32_t leftToLeft, uint32_t rightToLeft, uint32_t leftToRight, uint32_t rightToRight, uint32_t rampStep);
static void BAPE_DpMixer_P_LoadOutputCoefs(BAPE_Handle deviceHandle, uint32_t mixerId, unsigned outputIndex, uint32_t left, uint32_t right);

static BERR_Code BAPE_MixerGroup_P_LinkInput(BAPE_MixerGroupHandle handle, unsigned inputIndex);
static void BAPE_MixerGroup_P_UnlinkInput(BAPE_MixerGroupHandle handle, unsigned inputIndex);
static void BAPE_MixerGroup_P_ApplyInputCoefficients(BAPE_MixerGroupHandle handle, unsigned inputIndex);
static void BAPE_MixerGroup_P_ApplyOutputCoefficients(BAPE_MixerGroupHandle handle, unsigned outputIndex);
static uint32_t BAPE_DpMixer_P_GetConfigAddress(unsigned mixerId);
static uint32_t BAPE_DpMixer_P_GetInputConfigAddress(unsigned mixerId, unsigned inputId);
#if !(defined BCHP_AUD_FMM_DP_CTRL0_MIXER0_CONFIG_VOLUME_RAMP_DISABLE_OUTPUT0_MASK || defined BCHP_AUD_FMM_DP_CTRL0_MIXER_CONFIGi_VOLUME_RAMP_ENA_OUTPUT0_MASK)
static bool BAPE_DpMixer_P_MixerOutputIsActive(BAPE_MixerGroupHandle handle, unsigned mixerIndex);
static uint32_t BAPE_DpMixer_P_GetActiveMixerInputs(BAPE_MixerGroupHandle handle, unsigned mixerIndex);
static bool BAPE_DpMixer_P_MixerRampingActive(BAPE_MixerGroupHandle handle);
#endif



/***************************************************************************
Summary:
Initialize the DP block data structures
***************************************************************************/
BERR_Code BAPE_P_InitDpSw(
    BAPE_Handle handle
    )
{
    unsigned i;

    BDBG_OBJECT_ASSERT(handle, BAPE_Device);

    /* TODO: Register for ESR's to catch HW Failures?? */

    /* Setup Group Handles */
    BDBG_MSG(("Allocating %u MIXER Groups", BAPE_CHIP_MAX_MIXER_GROUPS));
    handle->mixerGroups[0] = BKNI_Malloc(BAPE_CHIP_MAX_MIXER_GROUPS*sizeof(BAPE_MixerGroup));
    if ( NULL == handle->mixerGroups[0] )
    {
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }
    BKNI_Memset(handle->mixerGroups[0], 0, BAPE_CHIP_MAX_MIXER_GROUPS*sizeof(BAPE_MixerGroup));
    for ( i = 1; i < BAPE_CHIP_MAX_MIXER_GROUPS; i++ )
    {
        handle->mixerGroups[i] = handle->mixerGroups[0] + i;
    }

    /* Set the default ramp step value. */
    handle->outputVolumeRampStep = BAPE_MIXER_DEFAULT_RAMP_STEP;

    return BERR_SUCCESS;
}

/***************************************************************************
Summary:
Initialize the DP block
***************************************************************************/
BERR_Code BAPE_P_InitDpHw(
    BAPE_Handle handle
    )
{
    uint32_t regAddr;
    BREG_Handle regHandle;
    unsigned i, j;

    BDBG_OBJECT_ASSERT(handle, BAPE_Device);

    regHandle = handle->regHandle;

    BDBG_MSG(("Clearing all DP registers"));

    for ( i = 0; i < BAPE_CHIP_MAX_MIXERS; i++ )
    {
        BAPE_DpMixer_P_SetGroup(handle, i, i);
        for ( j = 0; j < BAPE_CHIP_MAX_MIXER_INPUTS; j++ )
        {
            BAPE_DpMixer_P_LoadInputCoefs(handle, i, j, 0, 0, 0, 0, BAPE_MIXER_DEFAULT_RAMP_STEP);
        }
        for ( j = 0; j < BAPE_CHIP_MAX_MIXER_OUTPUTS; j++ )
        {
            BAPE_DpMixer_P_LoadOutputCoefs(handle, i, j, 0, 0);
        }
        /* Make sure to disable the "volume ramp at zero cross" setting (if it exists). */
        regAddr = BAPE_DpMixer_P_GetConfigAddress(i);
        #if defined BCHP_AUD_FMM_DP_CTRL0_MIXER0_CONFIG_DISABLE_VOL_RAMP_AT_ZERO_CROSS_MASK
                BAPE_Reg_P_UpdateEnum(handle, regAddr, AUD_FMM_DP_CTRL0_MIXER0_CONFIG, DISABLE_VOL_RAMP_AT_ZERO_CROSS, Disable);
        #elif defined BCHP_AUD_FMM_DP_CTRL0_MIXER_CONFIGi
                BAPE_Reg_P_UpdateEnum(handle, regAddr, AUD_FMM_DP_CTRL0_MIXER_CONFIGi, VOL_RAMP_AT_ZERO_CROSS_ENA, Disable);
        #endif
    }

#ifdef BCHP_AUD_FMM_DP_CTRL0_PING_COEFF_RAMP_STEPi_ARRAY_BASE
    regAddr = BCHP_AUD_FMM_DP_CTRL0_PING_COEFF_RAMP_STEPi_ARRAY_BASE;
    for ( i=BCHP_AUD_FMM_DP_CTRL0_PING_COEFF_RAMP_STEPi_ARRAY_START; i<=BCHP_AUD_FMM_DP_CTRL0_PING_COEFF_RAMP_STEPi_ARRAY_END; i++ )
    {
        BREG_Write32(regHandle, regAddr, BAPE_MIXER_DEFAULT_RAMP_STEP);
        regAddr += 4;
    }
#else
    BREG_Write32(regHandle, BCHP_AUD_FMM_DP_CTRL0_PING_COEFF_RAMP_STEP, BAPE_MIXER_DEFAULT_RAMP_STEP);
#endif
#ifdef BCHP_AUD_FMM_DP_CTRL0_PONG_COEFFICIENTSi_ARRAY_BASE
#ifdef BCHP_AUD_FMM_DP_CTRL0_PONG_COEFF_RAMP_STEPi_ARRAY_BASE
    regAddr = BCHP_AUD_FMM_DP_CTRL0_PONG_COEFF_RAMP_STEPi_ARRAY_BASE;
    for ( i=BCHP_AUD_FMM_DP_CTRL0_PONG_COEFF_RAMP_STEPi_ARRAY_START; i<=BCHP_AUD_FMM_DP_CTRL0_PONG_COEFF_RAMP_STEPi_ARRAY_END; i++ )
    {
        BREG_Write32(regHandle, regAddr, BAPE_MIXER_DEFAULT_RAMP_STEP);
        regAddr += 4;
    }
#else
    BREG_Write32(regHandle, BCHP_AUD_FMM_DP_CTRL0_PONG_COEFF_RAMP_STEP, BAPE_MIXER_DEFAULT_RAMP_STEP);
#endif
#endif

    BDBG_MSG(("Setting RAMP STEP to %u", handle->outputVolumeRampStep));
    (void)BAPE_SetOutputVolumeRampStep(handle, handle->outputVolumeRampStep);

   /* Init Playback FCI ID's to Invalid */
    for ( i = 0; i < BAPE_CHIP_MAX_MIXER_PLAYBACKS; i++ )
    {
        regAddr = BCHP_AUD_FMM_DP_CTRL0_PB_FCI_IDi_ARRAY_BASE + 4*(i);  
        BREG_Write32(regHandle, regAddr, BAPE_FCI_ID_INVALID);
    }

    /* TODO: Register for ESR's to catch HW Failures?? */

    return BERR_SUCCESS;
}

/***************************************************************************
Summary:
Un-Initialize the DP block
***************************************************************************/
void BAPE_P_UninitDpSw(
    BAPE_Handle handle
    )
{
    if ( handle->mixerGroups[0] )
    {
        BKNI_Free(handle->mixerGroups[0]);
        BKNI_Memset(handle->mixerGroups, 0, sizeof(handle->mixerGroups));
    }
}

static void BAPE_DpMixer_P_SetGroup(
    BAPE_Handle deviceHandle, 
    uint32_t mixerId, 
    uint32_t groupId
    )
{
    uint32_t regAddr, regVal;
    BDBG_OBJECT_ASSERT(deviceHandle, BAPE_Device);
    regAddr = BAPE_DpMixer_P_GetConfigAddress(mixerId);
    regVal = BREG_Read32(deviceHandle->regHandle, regAddr);
#ifdef BCHP_AUD_FMM_DP_CTRL0_MIXER0_CONFIG
    regVal &= ~BCHP_MASK(AUD_FMM_DP_CTRL0_MIXER0_CONFIG, MIXER_GROUP_BEGIN);
    regVal |= BCHP_FIELD_DATA(AUD_FMM_DP_CTRL0_MIXER0_CONFIG, MIXER_GROUP_BEGIN, groupId);
#else
    regVal &= ~BCHP_MASK(AUD_FMM_DP_CTRL0_MIXER_CONFIGi, MIXER_GROUP_BEGIN);
    regVal |= BCHP_FIELD_DATA(AUD_FMM_DP_CTRL0_MIXER_CONFIGi, MIXER_GROUP_BEGIN, groupId);    
#endif
    BREG_Write32(deviceHandle->regHandle, regAddr, regVal);
}

void BAPE_MixerGroup_P_GetDefaultCreateSettings(
    BAPE_MixerGroupCreateSettings *pSettings    /* [out] */
    )
{
    BDBG_ASSERT(NULL != pSettings);
    pSettings->numChannelPairs = 1;
    pSettings->blockId = 0;
}

BERR_Code BAPE_MixerGroup_P_Create(
    BAPE_Handle deviceHandle,
    const BAPE_MixerGroupCreateSettings *pSettings,
    BAPE_MixerGroupHandle *pHandle  /* [out] */
    )
{
    BERR_Code errCode;
    unsigned i, j, mixer;
    BAPE_MixerGroupHandle handle=NULL;

    BDBG_OBJECT_ASSERT(deviceHandle, BAPE_Device);
    BDBG_ASSERT(NULL != pSettings);
    BDBG_ASSERT(NULL != pHandle);
    BDBG_ASSERT(pSettings->numChannelPairs <= BAPE_ChannelPair_eMax);
    BDBG_ASSERT(pSettings->blockId == 0);   /* TODO: Handle more than one */

    /* Find an available group handle */
    for ( i = 0; i < BAPE_CHIP_MAX_MIXER_GROUPS; i++ )
    {
        BDBG_ASSERT(NULL != deviceHandle->mixerGroups[i]);
        if ( !deviceHandle->mixerGroups[i]->allocated )
        {
            handle = deviceHandle->mixerGroups[i];
            break;
        }
    }

    /* If none found, return error */
    if ( NULL == handle )
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    /* Now search for the correct number of resources */
    errCode = BAPE_P_AllocateFmmResource(deviceHandle, BAPE_FmmResourceType_eMixer, pSettings->numChannelPairs, &mixer);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_alloc_mixer;
    }
    BDBG_MSG(("Allocating %d mixer pairs, starting with DP Mixer %d", pSettings->numChannelPairs, mixer));

    /* Successfully allocated resources.  Initialize Group */
    BKNI_Memset(handle, 0, sizeof(BAPE_MixerGroup));
    handle->allocated = true;
    handle->blockId = pSettings->blockId;
    handle->numChannelPairs = pSettings->numChannelPairs;
    handle->deviceHandle = deviceHandle;
    handle->settings.volumeControlEnabled = true;
    BKNI_Memset(handle->mixerIds, 0xff, sizeof(handle->mixerIds));
    for ( i = 0; i < pSettings->numChannelPairs; i++ )
    {
        handle->mixerIds[i] = mixer + i;
        /* Setup Grouping */
        BAPE_DpMixer_P_SetGroup(deviceHandle, mixer + i, mixer);
    }
    for ( i = 0; i < BAPE_CHIP_MAX_MIXER_INPUTS; i++ )
    {
        BAPE_FciIdGroup_Init(&handle->inputs[i].settings.input);
        handle->inputs[i].settings.rampStep = BAPE_MIXER_DEFAULT_RAMP_STEP;
        for ( j = 0; j < pSettings->numChannelPairs; j++ )
        {
            handle->inputs[i].settings.coefficients[j][0][0] = BAPE_VOLUME_NORMAL;
            handle->inputs[i].settings.coefficients[j][1][1] = BAPE_VOLUME_NORMAL;
        }
        BKNI_Memset(handle->inputs[i].playbackIds, 0xff, sizeof(handle->inputs[i].playbackIds));
    }
    for ( i = 0; i < BAPE_CHIP_MAX_MIXER_OUTPUTS; i++ )
    {
        for ( j = 0; j < 2*pSettings->numChannelPairs; j++ )
        {
            handle->outputs[i].settings.coefficients[j] = BAPE_VOLUME_NORMAL;
        }
    }

    *pHandle = handle;
    return BERR_SUCCESS;

    err_alloc_mixer:
    return errCode; 
}


void BAPE_MixerGroup_P_Destroy(
    BAPE_MixerGroupHandle handle
    )
{
    unsigned i;

    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(handle->allocated);

    BDBG_ASSERT(handle->numRunningInputs == 0);
    BDBG_ASSERT(handle->numRunningOutputs == 0);

    /* Make sure we release all playbacks to the pool */
    for ( i = 0; i < BAPE_CHIP_MAX_MIXER_INPUTS; i++ )
    {
        if ( handle->inputs[i].linked )
        {
            BAPE_MixerGroup_P_UnlinkInput(handle, i);
        }
    }

    /* Make sure Mixer Group ID's are reset to un-grouped */
    for ( i = 0; i < handle->numChannelPairs; i++ )
    {
        BAPE_DpMixer_P_SetGroup(handle->deviceHandle, handle->mixerIds[i], handle->mixerIds[i]);
    }

    /* Release Resources */
    BAPE_P_FreeFmmResource(handle->deviceHandle, BAPE_FmmResourceType_eMixer, handle->numChannelPairs, handle->mixerIds[0]);
    BKNI_Memset(handle->mixerIds, 0xff, sizeof(handle->mixerIds));

    /* Done */
    handle->allocated = false;
}

void BAPE_MixerGroup_P_GetSettings(
    BAPE_MixerGroupHandle handle,
    BAPE_MixerGroupSettings *pSettings  /* [out] */
    )
{
    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(handle->allocated);
    *pSettings = handle->settings;
}

BERR_Code BAPE_MixerGroup_P_SetSettings(
    BAPE_MixerGroupHandle handle,
    const BAPE_MixerGroupSettings *pSettings
    )
{
    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(handle->allocated);

    /* Soft Limiting is not currently supported */
    if ( pSettings->softLimitEnabled )
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    if ( handle->numRunningOutputs > 0 )
    {
        uint32_t regVal, regAddr;
        unsigned i;
        /* Support changing VOLUME_ENA on the fly.  Others do not change. */
        for ( i = 0; i < handle->numChannelPairs; i++ )
        {
            regAddr = BAPE_DpMixer_P_GetConfigAddress(handle->mixerIds[i]);
            regVal = BREG_Read32(handle->deviceHandle->regHandle, regAddr);
#ifdef BCHP_AUD_FMM_DP_CTRL0_MIXER0_CONFIG
            regVal &= ~BCHP_MASK(AUD_FMM_DP_CTRL0_MIXER0_CONFIG, VOLUME_ENA);
            regVal |= BCHP_FIELD_DATA(AUD_FMM_DP_CTRL0_MIXER0_CONFIG, VOLUME_ENA, pSettings->volumeControlEnabled?1:0);
#else
            regVal &= ~BCHP_MASK(AUD_FMM_DP_CTRL0_MIXER_CONFIGi, VOLUME_ENA);
            regVal |= BCHP_FIELD_DATA(AUD_FMM_DP_CTRL0_MIXER_CONFIGi, VOLUME_ENA, pSettings->volumeControlEnabled?1:0);
#endif
            BREG_Write32(handle->deviceHandle->regHandle, regAddr, regVal);
        }       
    }
    handle->settings = *pSettings;

    return BERR_SUCCESS;
}

void BAPE_MixerGroup_P_GetInputSettings(
    BAPE_MixerGroupHandle handle,
    unsigned inputIndex,
    BAPE_MixerGroupInputSettings *pSettings  /* [out] */
    )
{
    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(handle->allocated);
    if ( inputIndex >= BAPE_CHIP_MAX_MIXER_INPUTS )
    {
        BERR_TRACE(BERR_INVALID_PARAMETER);
        BDBG_ASSERT(inputIndex < BAPE_CHIP_MAX_MIXER_INPUTS);
        return;
    }
    BDBG_ASSERT(NULL != pSettings);

    *pSettings = handle->inputs[inputIndex].settings;
}

/***************************************************************************
Summary:
Get the actual Dp Mixer id used by this index
***************************************************************************/
unsigned BAPE_MixerGroup_P_GetDpMixerId(
    BAPE_MixerGroupHandle handle,
    unsigned idx
    )
{
    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(handle->allocated);

    return handle->mixerIds[idx];
}

static bool BAPE_MixerGroup_P_CoefficientsChanged(const BAPE_MixerGroupInputSettings *pSettings1, const BAPE_MixerGroupInputSettings *pSettings2)
{
    unsigned i,j,k;

    BDBG_ASSERT(NULL != pSettings1);
    BDBG_ASSERT(NULL != pSettings2);

    for ( i = 0; i < BAPE_ChannelPair_eMax; i++ )
    {
        for ( j = 0; j < 2; j++ )
        {
            for ( k = 0; k < 2; k++ )
            {
                if ( pSettings1->coefficients[i][j][k] != pSettings2->coefficients[i][j][k] )
                {
                    return true;
                }
            }
        }
    }

    return false;
}

BERR_Code BAPE_MixerGroup_P_SetInputSettings(
    BAPE_MixerGroupHandle handle,
    unsigned inputIndex,
    const BAPE_MixerGroupInputSettings *pSettings
    )
{
    BERR_Code errCode;
    unsigned numInputPairs=0;
    bool inputChanged = false;
    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(handle->allocated);
    if ( inputIndex >= BAPE_CHIP_MAX_MIXER_INPUTS )
    {
        BDBG_ASSERT(inputIndex < BAPE_CHIP_MAX_MIXER_INPUTS);
        return BERR_TRACE(BERR_INVALID_PARAMETER);;
    }
    BDBG_ASSERT(NULL != pSettings);

    if ( handle->inputs[inputIndex].started )
    {
        /* We can't change FCI ID while running - only check volume */
        if ( BAPE_MixerGroup_P_CoefficientsChanged(&handle->inputs[inputIndex].settings, pSettings) )
        {
            /* Coefficients have changed.  Copy and update. */
            BKNI_Memcpy(handle->inputs[inputIndex].settings.coefficients, pSettings->coefficients, sizeof(pSettings->coefficients));
            BAPE_MixerGroup_P_ApplyInputCoefficients(handle, inputIndex);
        }
        BDBG_MSG(("Mixer group %p, input index %lu: started, only applying new coeffs", (void*)handle, (unsigned long)inputIndex));
        return BERR_SUCCESS;
    }

    /* If not started, we need to check for an input change */
    if ( !BAPE_FciIdGroup_IsEqual(handle->inputs[inputIndex].settings.input, pSettings->input) )
    {
        /* Make sure new FCI ID Group is legal */
        numInputPairs = BAPE_FciIdGroup_GetNumChannelPairs(pSettings->input);
        if ( numInputPairs > handle->numChannelPairs )
        {
            BDBG_ERR(("Input %u has more channel pairs (%u) than mixers in the group (%u).", inputIndex, numInputPairs, handle->numChannelPairs));
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }
        if ( handle->inputs[inputIndex].linked )
        {
            BDBG_MSG(("Unlinking Input to Mixer group %p, inputIndex %u, numInputPairs %u", (void*)handle, inputIndex, numInputPairs));
            BAPE_MixerGroup_P_UnlinkInput(handle, inputIndex);
        }
        inputChanged = true;
    }

    /* Update stored settings */
    handle->inputs[inputIndex].settings = *pSettings;

    /* Setup Playback Binding if Required */
    if ( inputChanged )
    {
        handle->inputs[inputIndex].numChannelPairs = numInputPairs;
        if ( numInputPairs > 0 )
        {
            #if BDBG_DEBUG_BUILD
            BDBG_MODULE_MSG(bape_fci, ("Link Mixer group %p, inputIndex %u, numInPairs %u", (void*)handle, inputIndex, numInputPairs));
            {
                unsigned i;
                for ( i=0; i<numInputPairs; i++ )
                {
                   BDBG_MODULE_MSG(bape_fci, ("  fci[%u]=%x -> DP Mixer %u Input[%u]", i, pSettings->input.ids[i], handle->mixerIds[i], inputIndex));
                }
            }
            #endif
            errCode = BAPE_MixerGroup_P_LinkInput(handle, inputIndex);
            if ( errCode )
            {
                /* Likely unable to retrieve resources.  Reset Stored FCI ID to default. */
                BAPE_FciIdGroup_Init(&handle->inputs[inputIndex].settings.input);
                handle->inputs[inputIndex].numChannelPairs = 0;
                return BERR_TRACE(errCode);
            }
        }
        else
        {
            BDBG_MSG(("Mixer group %p No input to link at index %u", (void*)handle, inputIndex));
        }
    }

    return BERR_SUCCESS;
}

static BERR_Code BAPE_MixerGroup_P_LinkInput(
    BAPE_MixerGroupHandle handle, 
    unsigned inputIndex
    )
{
    unsigned i, j, playback=(unsigned)-1;
    BAPE_FciId firstFci;
    BAPE_Handle deviceHandle;
    BERR_Code errCode;
    uint32_t regAddr;

    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(handle->allocated);
    if ( inputIndex >= BAPE_CHIP_MAX_MIXER_INPUTS )
    {
        BDBG_ASSERT(inputIndex < BAPE_CHIP_MAX_MIXER_INPUTS);
        return BERR_TRACE(BERR_INVALID_PARAMETER);;
    }

    deviceHandle = handle->deviceHandle;
    BDBG_OBJECT_ASSERT(deviceHandle, BAPE_Device);

    /* Make sure Free was previously called */
    BDBG_ASSERT(handle->inputs[inputIndex].linked == false);

    /* Check for no input */
    if ( handle->inputs[inputIndex].numChannelPairs == 0 )
    {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* Determine if FCI ID's are already used by another playback */
    firstFci = handle->inputs[inputIndex].settings.input.ids[0];
    for ( i = 0; i <= (BAPE_CHIP_MAX_MIXER_PLAYBACKS-(handle->inputs[inputIndex].numChannelPairs)); i++ )
    {
        if ( deviceHandle->playbackReferenceCount[i] > 0 &&
             deviceHandle->playbackFci[i] == firstFci )
        {
            /* We have found a match.  Sanity check that other group members are also present or we have an
               unrecoverable failure.  */
            for ( j = 1; j < handle->inputs[inputIndex].numChannelPairs; j++ )
            {
                if ( deviceHandle->playbackReferenceCount[i+j] == 0 ||
                     deviceHandle->playbackFci[i+j] != handle->inputs[inputIndex].settings.input.ids[j] )
                {
                    BDBG_ERR(("FCI ID %x is already in use by DP Playback %u but does not follow proper grouping.", firstFci, i));
                    return BERR_TRACE(BERR_NOT_SUPPORTED);
                }
            }

            playback = i;
            goto update_reference_counts;
        }
    }

    /* If we reach here, we didn't find a playback already in use.  Allocate a new set. */
    errCode = BAPE_P_AllocateFmmResource(deviceHandle, BAPE_FmmResourceType_ePlayback, handle->inputs[inputIndex].numChannelPairs, &playback);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    update_reference_counts:
    /* Update Reference Counts */
    for ( j = 0; j < handle->inputs[inputIndex].numChannelPairs; j++ )
    {
        handle->inputs[inputIndex].playbackIds[j] = playback+j;
        if ( 0 == deviceHandle->playbackReferenceCount[playback+j] )
        {
            regAddr = BCHP_AUD_FMM_DP_CTRL0_PB_FCI_IDi_ARRAY_BASE + 4*(playback+j);
            BREG_Write32(deviceHandle->regHandle, regAddr, handle->inputs[inputIndex].settings.input.ids[j]);
            deviceHandle->playbackFci[playback+j] = handle->inputs[inputIndex].settings.input.ids[j];

        }
        deviceHandle->playbackReferenceCount[playback+j]++;
    }
    handle->inputs[inputIndex].linked = true;

    return BERR_SUCCESS;
}

static void BAPE_MixerGroup_P_UnlinkInput(
    BAPE_MixerGroupHandle handle, 
    unsigned inputIndex
    )
{
    unsigned i;
    uint32_t regAddr;
    BAPE_Handle deviceHandle;

    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(handle->allocated);
    if ( inputIndex >= BAPE_CHIP_MAX_MIXER_INPUTS )
    {
        BERR_TRACE(BERR_INVALID_PARAMETER);
        BDBG_ASSERT(inputIndex < BAPE_CHIP_MAX_MIXER_INPUTS);
        return;
    }

    deviceHandle = handle->deviceHandle;
    BDBG_OBJECT_ASSERT(deviceHandle, BAPE_Device);

    /* Make sure Free was not previously called and we're not releasing a running resource */
    BDBG_ASSERT(handle->inputs[inputIndex].linked == true);
    BDBG_ASSERT(handle->inputs[inputIndex].started == false);

    for ( i = 0; i < handle->inputs[inputIndex].numChannelPairs; i++ )
    {
        unsigned playback = handle->inputs[inputIndex].playbackIds[i];
        /* If this is the last user of the playback, we will need to free it. */
        if ( deviceHandle->playbackReferenceCount[playback] == 1 )
        {
            /* Free this playback */
            BAPE_P_FreeFmmResource(handle->deviceHandle, BAPE_FmmResourceType_ePlayback, 1, playback);
            regAddr = BCHP_AUD_FMM_DP_CTRL0_PB_FCI_IDi_ARRAY_BASE + 4*(playback+i);
            deviceHandle->playbackFci[playback] = BAPE_FCI_ID_INVALID;
            BREG_Write32(deviceHandle->regHandle, regAddr, BAPE_FCI_ID_INVALID);
        }
        deviceHandle->playbackReferenceCount[playback]--;
    }
    handle->inputs[inputIndex].linked = false;
}

BERR_Code BAPE_MixerGroup_P_StartInput(
    BAPE_MixerGroupHandle handle,
    unsigned inputIndex
    )
{
    unsigned i;
    BAPE_Handle deviceHandle;
    uint32_t regVal, regAddr;

    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(handle->allocated);
    if ( inputIndex >= BAPE_CHIP_MAX_MIXER_INPUTS )
    {
        BDBG_ASSERT(inputIndex < BAPE_CHIP_MAX_MIXER_INPUTS);
        return BERR_TRACE(BERR_INVALID_PARAMETER);;
    }

    deviceHandle = handle->deviceHandle;
    BDBG_OBJECT_ASSERT(deviceHandle, BAPE_Device);

    /* Make sure Free was not previously called and we're not starting twice */
    BDBG_ASSERT(handle->inputs[inputIndex].linked == true);
    BDBG_ASSERT(handle->inputs[inputIndex].started == false);

    BDBG_MSG(("Start MixerGroup(%p) Inputs numChannelPairs %d:", (void *)handle, handle->numChannelPairs));
    /* Setup Input Linkage */
    for ( i = 0; i < handle->numChannelPairs; i++ )
    {
        uint32_t pbId;

        /* Handle "Type1 vs Type2 - see MIXER_GROUP_BEGIN field in RDB docs */
        pbId = (i < handle->inputs[inputIndex].numChannelPairs)?handle->inputs[inputIndex].playbackIds[i]:handle->inputs[inputIndex].playbackIds[handle->inputs[inputIndex].numChannelPairs-1];
        BDBG_MSG(("  ch pair %d, pbId %d", i, pbId));

        /* Correlate mixer ID to register base */
        regAddr = BAPE_DpMixer_P_GetInputConfigAddress(handle->mixerIds[i], inputIndex);
        regVal = BREG_Read32(handle->deviceHandle->regHandle, regAddr);

#ifdef BCHP_AUD_FMM_DP_CTRL0_MIXER0_INPUT10_CONFIG
        if ( (inputIndex % 2) == 0 )
        {
            regVal &= ~BCHP_MASK(AUD_FMM_DP_CTRL0_MIXER0_INPUT10_CONFIG, MIXER_INPUT0_PB_NUMBER);
            regVal |= BCHP_FIELD_DATA(AUD_FMM_DP_CTRL0_MIXER0_INPUT10_CONFIG, MIXER_INPUT0_PB_NUMBER, pbId);
        }
        else
        {
            regVal &= ~BCHP_MASK(AUD_FMM_DP_CTRL0_MIXER0_INPUT10_CONFIG, MIXER_INPUT1_PB_NUMBER);
            regVal |= BCHP_FIELD_DATA(AUD_FMM_DP_CTRL0_MIXER0_INPUT10_CONFIG, MIXER_INPUT1_PB_NUMBER, pbId);
        }
#else
        regVal &= ~BCHP_MASK(AUD_FMM_DP_CTRL0_MIXER0_INPUT_CONFIGi, MIXER_INPUT_PB_NUMBER);
        regVal |= BCHP_FIELD_DATA(AUD_FMM_DP_CTRL0_MIXER0_INPUT_CONFIGi, MIXER_INPUT_PB_NUMBER, pbId);
#endif

        BREG_Write32(handle->deviceHandle->regHandle, regAddr, regVal);     
    }

    /* Apply Group-Level Settings */
    for ( i = 0; i < handle->numChannelPairs; i++ )
    {
#ifdef BCHP_AUD_FMM_DP_CTRL0_MIXER0_INPUT10_CONFIG
        /* Volume -- Add soft limit here also if later required. */
        regAddr = BAPE_DpMixer_P_GetConfigAddress(handle->mixerIds[i]);
        regVal = BREG_Read32(handle->deviceHandle->regHandle, regAddr);
        regVal &= ~BCHP_MASK(AUD_FMM_DP_CTRL0_MIXER0_CONFIG, VOLUME_ENA);
        regVal |= BCHP_FIELD_DATA(AUD_FMM_DP_CTRL0_MIXER0_CONFIG, VOLUME_ENA, handle->settings.volumeControlEnabled?1:0);
        BREG_Write32(handle->deviceHandle->regHandle, regAddr, regVal);
        /* Priority */
    #ifdef BCHP_AUD_FMM_DP_CTRL0_MIXER_HIGH_PRIORITY
        regAddr = BCHP_AUD_FMM_DP_CTRL0_MIXER_HIGH_PRIORITY;
        regVal = BREG_Read32(handle->deviceHandle->regHandle, regAddr);
        regVal &= ~(BCHP_MASK(AUD_FMM_DP_CTRL0_MIXER_HIGH_PRIORITY, MIXER0_HIGH_PRIORITY)<<handle->mixerIds[i]);
        regVal |= (BCHP_FIELD_DATA(AUD_FMM_DP_CTRL0_MIXER_HIGH_PRIORITY, MIXER0_HIGH_PRIORITY, handle->settings.highPriority?1:0)<<handle->mixerIds[i]);
        BREG_Write32(handle->deviceHandle->regHandle, regAddr, regVal);
    #else
        regAddr = BCHP_AUD_FMM_DP_CTRL0_MIXER_OUTPUT_ENA;
        regVal = BREG_Read32(handle->deviceHandle->regHandle, regAddr);
        regVal &= ~(BCHP_MASK(AUD_FMM_DP_CTRL0_MIXER_OUTPUT_ENA, MIXER0_HIGH_PRIORITY)<<handle->mixerIds[i]);
        regVal |= (BCHP_FIELD_DATA(AUD_FMM_DP_CTRL0_MIXER_OUTPUT_ENA, MIXER0_HIGH_PRIORITY, handle->settings.highPriority?1:0)<<handle->mixerIds[i]);
        BREG_Write32(handle->deviceHandle->regHandle, regAddr, regVal);
    #endif
#else
        /* New 7429-style mixer combines these bits in the config register */
        regAddr = BAPE_DpMixer_P_GetConfigAddress(handle->mixerIds[i]);
        regVal = BREG_Read32(handle->deviceHandle->regHandle, regAddr);
        regVal &= ~BCHP_MASK(AUD_FMM_DP_CTRL0_MIXER_CONFIGi, VOLUME_ENA);
        regVal |= BCHP_FIELD_DATA(AUD_FMM_DP_CTRL0_MIXER_CONFIGi, VOLUME_ENA, handle->settings.volumeControlEnabled?1:0);
        regVal &= ~BCHP_MASK(AUD_FMM_DP_CTRL0_MIXER_CONFIGi, MIXER_HIGH_PRIORITY);
        regVal |= (BCHP_FIELD_DATA(AUD_FMM_DP_CTRL0_MIXER_CONFIGi, MIXER_HIGH_PRIORITY, handle->settings.highPriority?1:0)<<handle->mixerIds[i]);
        BREG_Write32(handle->deviceHandle->regHandle, regAddr, regVal);        
#endif
    }           

    /* Apply Input Scaling Coefs */
    BAPE_MixerGroup_P_ApplyInputCoefficients(handle, inputIndex);

    /* Enable the mixer input */
    for ( i = 0; i < handle->numChannelPairs; i++ )
    {
        uint32_t bitmask;
        uint32_t pbId, fci;

        /* Find input config register (they are paired) */
        regAddr = BAPE_DpMixer_P_GetInputConfigAddress(handle->mixerIds[i], inputIndex);

#ifdef BCHP_AUD_FMM_DP_CTRL0_MIXER0_INPUT10_CONFIG
        if ( 0 == (inputIndex%2) )
        {
            /* Even.  Write the first enable. */
            bitmask = BCHP_MASK(AUD_FMM_DP_CTRL0_MIXER0_INPUT10_CONFIG, MIXER_INPUT0_ENA);
            pbId = BAPE_Reg_P_ReadField(handle->deviceHandle, regAddr, AUD_FMM_DP_CTRL0_MIXER0_INPUT10_CONFIG, MIXER_INPUT0_PB_NUMBER);
        }
        else
        {
            /* Odd.  Write the second enable. */
            bitmask = BCHP_MASK(AUD_FMM_DP_CTRL0_MIXER0_INPUT10_CONFIG, MIXER_INPUT1_ENA);
            pbId = BAPE_Reg_P_ReadField(handle->deviceHandle, regAddr, AUD_FMM_DP_CTRL0_MIXER0_INPUT10_CONFIG, MIXER_INPUT1_PB_NUMBER);
        }
#else
        bitmask = BCHP_MASK(AUD_FMM_DP_CTRL0_MIXER0_INPUT_CONFIGi, MIXER_INPUT_ENA);
        pbId = BAPE_Reg_P_ReadField(handle->deviceHandle, regAddr, AUD_FMM_DP_CTRL0_MIXER0_INPUT_CONFIGi, MIXER_INPUT_PB_NUMBER);
#endif
        fci = BREG_Read32(handle->deviceHandle->regHandle, BCHP_AUD_FMM_DP_CTRL0_PB_FCI_IDi_ARRAY_BASE + 4*pbId);

        BDBG_MSG(("Enabling mixer %u input port %u (PB %u FCI %u)", handle->mixerIds[i], inputIndex,
                  pbId, fci));
        regVal = BREG_Read32(handle->deviceHandle->regHandle, regAddr);
        regVal |= bitmask;
        BREG_Write32(handle->deviceHandle->regHandle, regAddr, regVal);     
    }

    handle->inputs[inputIndex].started = true;
    handle->numRunningInputs++;
    BDBG_MSG(("Mixer Group %p now has %u running inputs", (void *)handle, handle->numRunningInputs));

    return BERR_SUCCESS;
}

void BAPE_MixerGroup_P_StopInput(
    BAPE_MixerGroupHandle handle,
    unsigned inputIndex
    )
{
    unsigned i;
    BAPE_Handle deviceHandle;
    uint32_t regVal, regAddr;

    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(handle->allocated);
    if ( inputIndex >= BAPE_CHIP_MAX_MIXER_INPUTS )
    {
        BERR_TRACE(BERR_INVALID_PARAMETER);
        BDBG_ASSERT(inputIndex < BAPE_CHIP_MAX_MIXER_INPUTS);
        return;
    }

    deviceHandle = handle->deviceHandle;
    BDBG_OBJECT_ASSERT(deviceHandle, BAPE_Device);

    /* Make sure Free was not previously called and we're not stopping twice */
    BDBG_ASSERT(handle->inputs[inputIndex].linked == true);
    if ( false == handle->inputs[inputIndex].started )
    {
        return;
    }

    /* Disable the mixer input */
    for ( i = 0; i < handle->numChannelPairs; i++ )
    {
        uint32_t bitmask;
        uint32_t pbId, fci;

        /* Find input config register */
        regAddr = BAPE_DpMixer_P_GetInputConfigAddress(handle->mixerIds[i], inputIndex);
#ifdef BCHP_AUD_FMM_DP_CTRL0_MIXER0_INPUT10_CONFIG
        if ( 0 == (inputIndex%2) )
        {
            /* Even.  Write the first enable. */
            bitmask = BCHP_MASK(AUD_FMM_DP_CTRL0_MIXER0_INPUT10_CONFIG, MIXER_INPUT0_ENA);
            pbId = BAPE_Reg_P_ReadField(handle->deviceHandle, regAddr, AUD_FMM_DP_CTRL0_MIXER0_INPUT10_CONFIG, MIXER_INPUT0_PB_NUMBER);
        }
        else
        {
            /* Odd.  Write the second enable. */
            bitmask = BCHP_MASK(AUD_FMM_DP_CTRL0_MIXER0_INPUT10_CONFIG, MIXER_INPUT1_ENA);
            pbId = BAPE_Reg_P_ReadField(handle->deviceHandle, regAddr, AUD_FMM_DP_CTRL0_MIXER0_INPUT10_CONFIG, MIXER_INPUT1_PB_NUMBER);
        }
#else
        bitmask = BCHP_MASK(AUD_FMM_DP_CTRL0_MIXER0_INPUT_CONFIGi, MIXER_INPUT_ENA);
        pbId = BAPE_Reg_P_ReadField(handle->deviceHandle, regAddr, AUD_FMM_DP_CTRL0_MIXER0_INPUT_CONFIGi, MIXER_INPUT_PB_NUMBER);
#endif

        /* wait for ramping to complete on this mixer group */
        if ( BAPE_MixerGroup_P_WaitForRamping(handle) == BERR_TIMEOUT )
        {
            BDBG_ERR(("WARNING - %s - Vol Ramp timed out...", __FUNCTION__));
        }

        fci = BREG_Read32(handle->deviceHandle->regHandle, BCHP_AUD_FMM_DP_CTRL0_PB_FCI_IDi_ARRAY_BASE + 4*pbId);

        BDBG_MSG(("Disabling mixer %u input port %u (PB %u FCI %u)", handle->mixerIds[i], inputIndex,
                  pbId, fci));
        regVal = BREG_Read32(handle->deviceHandle->regHandle, regAddr);
        regVal &= ~bitmask;
        BREG_Write32(handle->deviceHandle->regHandle, regAddr, regVal);     
    }

    handle->inputs[inputIndex].started = false;
    BDBG_ASSERT(handle->numRunningInputs > 0);
    handle->numRunningInputs--; 
}

void BAPE_MixerGroup_P_GetOutputSettings(
    BAPE_MixerGroupHandle handle,
    unsigned outputIndex,
    BAPE_MixerGroupOutputSettings *pSettings  /* [out] */
    )
{
    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(handle->allocated);
    if ( outputIndex >= BAPE_CHIP_MAX_MIXER_OUTPUTS )
    {
        BERR_TRACE(BERR_INVALID_PARAMETER);
        BDBG_ASSERT(outputIndex < BAPE_CHIP_MAX_MIXER_OUTPUTS);
        return;
    }
    BDBG_ASSERT(NULL != pSettings);
    *pSettings = handle->outputs[outputIndex].settings;
}

BERR_Code BAPE_MixerGroup_P_SetOutputSettings(
    BAPE_MixerGroupHandle handle,
    unsigned outputIndex,
    const BAPE_MixerGroupOutputSettings *pSettings
    )
{
    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(handle->allocated);
    if ( outputIndex >= BAPE_CHIP_MAX_MIXER_OUTPUTS )
    {
        BDBG_ASSERT(outputIndex < BAPE_CHIP_MAX_MIXER_OUTPUTS);
        return BERR_TRACE(BERR_INVALID_PARAMETER);;
    }
    BDBG_ASSERT(NULL != pSettings);

    handle->outputs[outputIndex].settings = *pSettings;

    if ( handle->outputs[outputIndex].started )
    {
        BAPE_MixerGroup_P_ApplyOutputCoefficients(handle, outputIndex);
    }

    return BERR_SUCCESS;
}

#if !B_REFSW_MINIMAL
void BAPE_MixerGroup_P_GetOutputStatus(
    BAPE_MixerGroupHandle handle,
    unsigned outputIndex,
    BAPE_MixerGroupOutputStatus *pStatus    /* [out] */
    )
{
    unsigned i;
    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(handle->allocated);
    if ( outputIndex >= BAPE_CHIP_MAX_MIXER_OUTPUTS )
    {
        BERR_TRACE(BERR_INVALID_PARAMETER);
        BDBG_ASSERT(outputIndex < BAPE_CHIP_MAX_MIXER_OUTPUTS);
        return;
    }
    BDBG_ASSERT(NULL != pStatus);

    BKNI_Memset(pStatus, 0, sizeof(BAPE_MixerGroupOutputStatus));
    if ( handle->outputs[outputIndex].started )
    {
        uint32_t rampStatus, regAddr;

#ifdef BCHP_AUD_FMM_DP_CTRL0_VOLUME_RAMP_STATUS0
        /* Read back ramp status from mixer */
        regAddr = (outputIndex == 0) ? BCHP_AUD_FMM_DP_CTRL0_VOLUME_RAMP_STATUS0 : BCHP_AUD_FMM_DP_CTRL0_VOLUME_RAMP_STATUS1;
        rampStatus = BREG_Read32(handle->deviceHandle->regHandle, regAddr);

        for ( i = 0; i < handle->numChannelPairs; i++ )
        {
            uint32_t bitmask;
            bitmask = BCHP_MASK(AUD_FMM_DP_CTRL0_VOLUME_RAMP_STATUS0, MIXER0_LEFT_VOLUME_RAMP) << handle->mixerIds[i];
            pStatus->rampActive[2*i] = (rampStatus & bitmask) ? true : false;
            bitmask = BCHP_MASK(AUD_FMM_DP_CTRL0_VOLUME_RAMP_STATUS0, MIXER0_RIGHT_VOLUME_RAMP) << handle->mixerIds[i];
            pStatus->rampActive[(2*i)+1] = (rampStatus & bitmask) ? true : false;
        }
#else        
        regAddr = BCHP_AUD_FMM_DP_CTRL0_VOLUME_RAMP_STATUSi_ARRAY_BASE + ((BCHP_AUD_FMM_DP_CTRL0_VOLUME_RAMP_STATUSi_ARRAY_ELEMENT_SIZE/8)*outputIndex);
        rampStatus = BREG_Read32(handle->deviceHandle->regHandle, regAddr);

        for ( i = 0; i < handle->numChannelPairs; i++ )
        {
            uint32_t bitmask;
            bitmask = (outputIndex == 0) ? 
                BCHP_MASK(AUD_FMM_DP_CTRL0_VOLUME_RAMP_STATUSi, MIXER_OUTPUT0_LEFT_VOLUME_RAMP) :
                BCHP_MASK(AUD_FMM_DP_CTRL0_VOLUME_RAMP_STATUSi, MIXER_OUTPUT1_LEFT_VOLUME_RAMP);
            pStatus->rampActive[2*i] = (rampStatus & bitmask) ? true : false;
            bitmask = (outputIndex == 0) ? 
                BCHP_MASK(AUD_FMM_DP_CTRL0_VOLUME_RAMP_STATUSi, MIXER_OUTPUT0_RIGHT_VOLUME_RAMP) :
                BCHP_MASK(AUD_FMM_DP_CTRL0_VOLUME_RAMP_STATUSi, MIXER_OUTPUT1_RIGHT_VOLUME_RAMP);
            pStatus->rampActive[(2*i)+1] = (rampStatus & bitmask) ? true : false;
        }
#endif
    }
}
#endif

BERR_Code BAPE_MixerGroup_P_StartOutput(
    BAPE_MixerGroupHandle handle,
    unsigned outputIndex
    )
{
    unsigned i, outputChannelPairs;
    uint32_t regAddr, regVal, bitmask;

    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(handle->allocated);
    if ( outputIndex >= BAPE_CHIP_MAX_MIXER_OUTPUTS )
    {
        BDBG_ASSERT(outputIndex < BAPE_CHIP_MAX_MIXER_OUTPUTS);
        return BERR_TRACE(BERR_INVALID_PARAMETER);;
    }
    BDBG_ASSERT(handle->outputs[outputIndex].started == false);

    /* Refresh output scaling coefficients first */
    BAPE_MixerGroup_P_ApplyOutputCoefficients(handle, outputIndex);

    /* Start */
    if ( handle->outputs[outputIndex].settings.numChannelPairs == 0 )
    {
        outputChannelPairs = handle->numChannelPairs;
    }
    else
    {
        outputChannelPairs = handle->outputs[outputIndex].settings.numChannelPairs;
    }
    for ( i = 0; i < outputChannelPairs; i++ )
    {
        /* Start mixer output */
#ifdef BCHP_AUD_FMM_DP_CTRL0_MIXER_OUTPUT_ENA
        regAddr = BCHP_AUD_FMM_DP_CTRL0_MIXER_OUTPUT_ENA;
        bitmask = (0 == outputIndex) ?
                  BCHP_MASK(AUD_FMM_DP_CTRL0_MIXER_OUTPUT_ENA, MIXER0_OUTPUT0_ENA) :
                  BCHP_MASK(AUD_FMM_DP_CTRL0_MIXER_OUTPUT_ENA, MIXER0_OUTPUT1_ENA);

        BDBG_MSG(("Enabling mixer %u output port %u", handle->mixerIds[i], outputIndex));
        bitmask <<= handle->mixerIds[i];
        regVal = BREG_Read32(handle->deviceHandle->regHandle, regAddr);
        regVal |= bitmask;
        BREG_Write32(handle->deviceHandle->regHandle, regAddr, regVal);
#else
        regAddr = BAPE_DpMixer_P_GetConfigAddress(handle->mixerIds[i]);
        bitmask = (0 == outputIndex) ?
                  BCHP_MASK(AUD_FMM_DP_CTRL0_MIXER_CONFIGi, MIXER_OUTPUT0_ENA) :
                  BCHP_MASK(AUD_FMM_DP_CTRL0_MIXER_CONFIGi, MIXER_OUTPUT1_ENA);

        BDBG_MSG(("Enabling mixer %u output port %u", handle->mixerIds[i], outputIndex));
        regVal = BREG_Read32(handle->deviceHandle->regHandle, regAddr);
        regVal |= bitmask;
        BREG_Write32(handle->deviceHandle->regHandle, regAddr, regVal);
#endif
    }

    handle->outputs[outputIndex].started = true;
    handle->numRunningOutputs++;

    return BERR_SUCCESS;
}

void BAPE_MixerGroup_P_StopOutput(
    BAPE_MixerGroupHandle handle,
    unsigned outputIndex
    )
{
    unsigned i;
    uint32_t regAddr, regVal, bitmask;

    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(handle->allocated);
    if ( outputIndex >= BAPE_CHIP_MAX_MIXER_OUTPUTS )
    {
        BERR_TRACE(BERR_INVALID_PARAMETER);
        BDBG_ASSERT(outputIndex < BAPE_CHIP_MAX_MIXER_OUTPUTS);
        return;
    }
    if ( handle->outputs[outputIndex].started == false )
    {
        return;
    }

    /* Stop */
    for ( i = 0; i < handle->numChannelPairs; i++ )
    {
        /* Stop mixer output */
#ifdef BCHP_AUD_FMM_DP_CTRL0_MIXER_OUTPUT_ENA
        regAddr = BCHP_AUD_FMM_DP_CTRL0_MIXER_OUTPUT_ENA;
        bitmask = (0 == outputIndex) ?
                  BCHP_MASK(AUD_FMM_DP_CTRL0_MIXER_OUTPUT_ENA, MIXER0_OUTPUT0_ENA) :
                  BCHP_MASK(AUD_FMM_DP_CTRL0_MIXER_OUTPUT_ENA, MIXER0_OUTPUT1_ENA);

        BDBG_MSG(("Disabling mixer %u output port %u", handle->mixerIds[i], outputIndex));
        bitmask <<= handle->mixerIds[i];
        regVal = BREG_Read32(handle->deviceHandle->regHandle, regAddr);
        regVal &= ~bitmask;
        BREG_Write32(handle->deviceHandle->regHandle, regAddr, regVal);
#else
        regAddr = BAPE_DpMixer_P_GetConfigAddress(handle->mixerIds[i]);
        bitmask = (0 == outputIndex) ?
                  BCHP_MASK(AUD_FMM_DP_CTRL0_MIXER_CONFIGi, MIXER_OUTPUT0_ENA) :
                  BCHP_MASK(AUD_FMM_DP_CTRL0_MIXER_CONFIGi, MIXER_OUTPUT1_ENA);

        BDBG_MSG(("Disabling mixer %u output port %u", handle->mixerIds[i], outputIndex));
        regVal = BREG_Read32(handle->deviceHandle->regHandle, regAddr);
        regVal &= ~bitmask;
        BREG_Write32(handle->deviceHandle->regHandle, regAddr, regVal);
#endif
    }

    handle->outputs[outputIndex].started = false;
    BDBG_ASSERT(handle->numRunningOutputs > 0);
    handle->numRunningOutputs--;
}

void BAPE_MixerGroup_P_GetOutputFciIds(
    BAPE_MixerGroupHandle handle,
    unsigned outputIndex,
    BAPE_FciIdGroup *pFciGroup      /* [out] */
    )
{
    unsigned i;

    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(handle->allocated);
    BDBG_ASSERT(outputIndex < BAPE_CHIP_MAX_MIXER_OUTPUTS);
    BDBG_ASSERT(NULL != pFciGroup);

    BAPE_FciIdGroup_Init(pFciGroup);

    for ( i = 0; i < handle->numChannelPairs; i++ )
    {
        pFciGroup->ids[i] = (BAPE_FCI_BASE_MIXER | (handle->mixerIds[i]*BAPE_CHIP_MAX_MIXER_OUTPUTS) | outputIndex);
    }
}

BERR_Code BAPE_MixerGroup_P_ApplyOutputVolume(
    BAPE_MixerGroupHandle handle,
    const BAPE_OutputVolume * pVolume,
    const BAPE_FMT_Descriptor * pFormat,
    unsigned outputIndex
    )
{
    bool pcm;
    unsigned i;
    BERR_Code errCode;
    BAPE_MixerGroupOutputSettings outputSettings;

    BDBG_ASSERT(handle != NULL);
    BDBG_ASSERT(pFormat != NULL);
    BDBG_ASSERT(pVolume != NULL);

    BAPE_MixerGroup_P_GetOutputSettings(handle, outputIndex, &outputSettings);

    pcm = BAPE_FMT_P_IsLinearPcm_isrsafe(pFormat);

    for ( i = 0; i < BAPE_Channel_eMax; i++ )
    {
        outputSettings.coefficients[i] = (pcm) ? pVolume->volume[i] : BAPE_VOLUME_NORMAL;
        BDBG_MSG(("Apply Volume %x for mixergroup %d,... , output %d(PCM=%d)", outputSettings.coefficients[i], handle->mixerIds[0], outputIndex, pcm));
    }

    outputSettings.muted = pVolume->muted;
    outputSettings.volumeRampDisabled = !pcm;  /* Disable volume ramp if input is compressed for a pseudo-compressed-mute */

    errCode = BAPE_MixerGroup_P_SetOutputSettings(handle, outputIndex, &outputSettings);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    return BERR_SUCCESS;
}

unsigned BAPE_Mixer_P_MixerFormatToNumChannels(
    BAPE_MixerFormat format
    )
{
    unsigned numChs = 0;

    switch ( format )
    {
    default:
    case BAPE_MixerFormat_eAuto:
    case BAPE_MixerFormat_eMax:
        break;
    case BAPE_MixerFormat_ePcmStereo:
        numChs = 2;
        break;
    case BAPE_MixerFormat_ePcm5_1:
        numChs = 6;
        break;
    case BAPE_MixerFormat_ePcm7_1:
        numChs = 8;
        break;
    }

    return numChs;
}

static void BAPE_DpMixer_P_LoadInputCoefs(
    BAPE_Handle deviceHandle, 
    uint32_t mixerId, 
    unsigned inputIndex, 
    uint32_t left0, 
    uint32_t right0, 
    uint32_t left1, 
    uint32_t right1,
    uint32_t rampStep
    )
{
    uint32_t regVal, regAddr, regOffset;
    uint32_t mixerNum, portNum;
    unsigned i;

    mixerNum = mixerId;
    portNum = inputIndex;

#ifdef BCHP_AUD_FMM_DP_CTRL0_USE_NEW_SCALING_COEFF
    /* On chips without a ping/pong setup - program the mixer to freeze the coefficients while updating */
    regVal = BREG_Read32(deviceHandle->regHandle, BCHP_AUD_FMM_DP_CTRL0_USE_NEW_SCALING_COEFF);
    regVal &= ~(1<<mixerNum);
    BREG_Write32(deviceHandle->regHandle, BCHP_AUD_FMM_DP_CTRL0_USE_NEW_SCALING_COEFF, regVal);
#endif

    /* -- From the RDB for 7408: (Same for other chips except for number of mixers, inputs, and coefficients)
    Lout = Left_Coef_0 * Lin + Right_Coef_0 * Rin;;;;  Rout = Left_Coef_1 * Lin + Right_Coef_1 * Rin.
    For input M of mixer N (where M ranges from 0 to 3 and N are ranges from 0 to 5), the index to the scaling coefficient array is at follow:
    Left_Coef_0 = coef[N*16 + M*4 + 0], Right_Coef_0 = coef[N*16 + M*4 + 1];;;;  Left_Coef_1 = coef[N*16 + M*4 + 2], Right_Coef_1 = coef[N*16 + M*4 + 3],
    For example, the scaling coefficients for input 0 of mixer 0 are
    Left_Coef_0 = coef[0], Right_Coef_0 = coef[1];;;;  Left_Coef_1 = coef[2], Right_Coef_1 = coef[3].
    The scaling coefficients for input 3 of mixer 1 are
    Left_Coef_0 = coef[28], Right_Coef_0 = coef[29];;;;  Left_Coef_1 = coef[30], Right_Coef_1 = coef[31].
    */
    BDBG_MSG(("Load coeffs for mixer %d, input %d, left0 %x, right0 %x, left1 %x, right1 %x",
             mixerId, inputIndex, left0, right0, left1, right1));
    /* Compute left coefficient 0 offset */
    regOffset = (mixerNum * 4 * BAPE_CHIP_MAX_MIXER_INPUTS) + (portNum * 4);
    BDBG_ASSERT(regOffset <= (BCHP_AUD_FMM_DP_CTRL0_PING_COEFFICIENTSi_ARRAY_END-3));   /* Sanity check */

    /* Setup Ramp Steps prior to setting coefficients */   
    for ( i = 0; i < 4; i++ )
    {
        regAddr = (regOffset + i) * 4;
        BREG_Write32(deviceHandle->regHandle, BCHP_AUD_FMM_DP_CTRL0_PING_COEFF_RAMP_STEPi_ARRAY_BASE+regAddr, rampStep);
        BREG_Write32(deviceHandle->regHandle, BCHP_AUD_FMM_DP_CTRL0_PONG_COEFF_RAMP_STEPi_ARRAY_BASE+regAddr, rampStep);
    }

    /* Setup Volume Coefs */    
    regAddr = (regOffset * 4) + BCHP_AUD_FMM_DP_CTRL0_PING_COEFFICIENTSi_ARRAY_BASE;
    regVal = left0 & BCHP_MASK(AUD_FMM_DP_CTRL0_PING_COEFFICIENTSi, COEFFICIENTS);
    BREG_Write32(deviceHandle->regHandle, regAddr, regVal);
#ifdef BCHP_AUD_FMM_DP_CTRL0_PONG_COEFFICIENTSi_ARRAY_BASE
    /* Write pong coef also */
    BDBG_CASSERT(BCHP_AUD_FMM_DP_CTRL0_PONG_COEFFICIENTSi_ARRAY_BASE > BCHP_AUD_FMM_DP_CTRL0_PING_COEFFICIENTSi_ARRAY_BASE);
    BREG_Write32(deviceHandle->regHandle, regAddr + (BCHP_AUD_FMM_DP_CTRL0_PONG_COEFFICIENTSi_ARRAY_BASE-BCHP_AUD_FMM_DP_CTRL0_PING_COEFFICIENTSi_ARRAY_BASE), regVal);
#endif

    regAddr += 4;   /* Jump to Right_Coef_0 */
    regVal = right0 & BCHP_MASK(AUD_FMM_DP_CTRL0_PING_COEFFICIENTSi, COEFFICIENTS);
    BREG_Write32(deviceHandle->regHandle, regAddr, regVal);
#ifdef BCHP_AUD_FMM_DP_CTRL0_PONG_COEFFICIENTSi_ARRAY_BASE
    /* Write pong coef also */
    BREG_Write32(deviceHandle->regHandle, regAddr + (BCHP_AUD_FMM_DP_CTRL0_PONG_COEFFICIENTSi_ARRAY_BASE-BCHP_AUD_FMM_DP_CTRL0_PING_COEFFICIENTSi_ARRAY_BASE), regVal);
#endif

    regAddr += 4;   /* Jump to Left_Coef_1 */
    regVal = left1 & BCHP_MASK(AUD_FMM_DP_CTRL0_PING_COEFFICIENTSi, COEFFICIENTS);
    BREG_Write32(deviceHandle->regHandle, regAddr, regVal);
#ifdef BCHP_AUD_FMM_DP_CTRL0_PONG_COEFFICIENTSi_ARRAY_BASE
    /* Write pong coef also */
    BREG_Write32(deviceHandle->regHandle, regAddr + (BCHP_AUD_FMM_DP_CTRL0_PONG_COEFFICIENTSi_ARRAY_BASE-BCHP_AUD_FMM_DP_CTRL0_PING_COEFFICIENTSi_ARRAY_BASE), regVal);
#endif

    regAddr += 4;   /* Jump to Right_Coef_1 */
    regVal = right1 & BCHP_MASK(AUD_FMM_DP_CTRL0_PING_COEFFICIENTSi, COEFFICIENTS);
    BREG_Write32(deviceHandle->regHandle, regAddr, regVal);
#ifdef BCHP_AUD_FMM_DP_CTRL0_PONG_COEFFICIENTSi_ARRAY_BASE
    /* Write pong coef also */
    BREG_Write32(deviceHandle->regHandle, regAddr + (BCHP_AUD_FMM_DP_CTRL0_PONG_COEFFICIENTSi_ARRAY_BASE-BCHP_AUD_FMM_DP_CTRL0_PING_COEFFICIENTSi_ARRAY_BASE), regVal);
#endif

#ifdef BCHP_AUD_FMM_DP_CTRL0_USE_NEW_SCALING_COEFF
    /* On chips without a ping/pong setup - program the mixer to load the coefficients after updating */
    regVal = BREG_Read32(deviceHandle->regHandle, BCHP_AUD_FMM_DP_CTRL0_USE_NEW_SCALING_COEFF);
    regVal |= (1<<mixerNum);
    BREG_Write32(deviceHandle->regHandle, BCHP_AUD_FMM_DP_CTRL0_USE_NEW_SCALING_COEFF, regVal);
#endif
}

static void BAPE_MixerGroup_P_ApplyInputCoefficients(
    BAPE_MixerGroupHandle handle, 
    unsigned inputIndex
    )
{
    unsigned i;
    int32_t leftToLeft, rightToLeft, leftToRight, rightToRight;

    BDBG_MODULE_MSG(bape_mixer_input_coeffs, ("Commiting DP Mixer Coeffs to HW:"));
    for ( i = 0; i < handle->numChannelPairs; i++ )
    {
        /* It is up to the calling APIs to properly clear unused coeffs prior
           to making this call - We are just going to blindly set coeffs here */
        /* Index by [Channel Pair][input channel][output channel] */
        leftToLeft = handle->inputs[inputIndex].settings.coefficients[i][0][0];
        rightToLeft = handle->inputs[inputIndex].settings.coefficients[i][1][0];
        leftToRight = handle->inputs[inputIndex].settings.coefficients[i][0][1];
        rightToRight = handle->inputs[inputIndex].settings.coefficients[i][1][1];
        BDBG_MODULE_MSG(bape_mixer_input_coeffs, ("  Input %d -> Mxr %2d Coeffs:  [%2d] %6x %6x %6x %6x", inputIndex, handle->mixerIds[i], i,
                  leftToLeft, rightToLeft, leftToRight, rightToRight));
        BAPE_DpMixer_P_LoadInputCoefs(handle->deviceHandle, handle->mixerIds[i], inputIndex, 
                                      leftToLeft, rightToLeft, leftToRight, rightToRight, handle->inputs[inputIndex].settings.rampStep);
    }
}

static void BAPE_DpMixer_P_LoadOutputCoefs(
    BAPE_Handle deviceHandle, 
    uint32_t mixerId, 
    unsigned outputIndex, 
    uint32_t left, 
    uint32_t right
    )
{
    uint32_t regAddr, regVal;

#if defined BCHP_AUD_FMM_DP_CTRL0_MIXER00_0_RT_VOL_LEVEL
    const uint32_t rightRegOffset = BCHP_AUD_FMM_DP_CTRL0_MIXER00_0_RT_VOL_LEVEL-BCHP_AUD_FMM_DP_CTRL0_MIXER00_0_LT_VOL_LEVEL;
    const uint32_t mixerRegOffset = BCHP_AUD_FMM_DP_CTRL0_MIXER01_0_LT_VOL_LEVEL-BCHP_AUD_FMM_DP_CTRL0_MIXER00_0_LT_VOL_LEVEL;

    if ( 1 == outputIndex )
    {
        regAddr = BCHP_AUD_FMM_DP_CTRL0_MIXER00_1_LT_VOL_LEVEL;
    }
    else
    {
        regAddr = BCHP_AUD_FMM_DP_CTRL0_MIXER00_0_LT_VOL_LEVEL;
    }

    left = BCHP_FIELD_DATA(AUD_FMM_DP_CTRL0_MIXER00_0_LT_VOL_LEVEL, MIXER_OUTPUT_LEFT_VOLUME_LEVEL, left) &
           BCHP_MASK(AUD_FMM_DP_CTRL0_MIXER00_0_LT_VOL_LEVEL, MIXER_OUTPUT_LEFT_VOLUME_LEVEL);
    right = BCHP_FIELD_DATA(AUD_FMM_DP_CTRL0_MIXER00_0_RT_VOL_LEVEL, MIXER_OUTPUT_RIGHT_VOLUME_LEVEL, right) &
            BCHP_MASK(AUD_FMM_DP_CTRL0_MIXER00_0_RT_VOL_LEVEL, MIXER_OUTPUT_RIGHT_VOLUME_LEVEL);

    regAddr += mixerRegOffset * ((uint32_t) mixerId);
    regVal = BREG_Read32(deviceHandle->regHandle, regAddr);
    regVal &= ~BCHP_MASK(AUD_FMM_DP_CTRL0_MIXER00_0_LT_VOL_LEVEL, MIXER_OUTPUT_LEFT_VOLUME_LEVEL);            
    regVal |= left;
    BREG_Write32(deviceHandle->regHandle, regAddr, regVal);

    regAddr += rightRegOffset;
    regVal = BREG_Read32(deviceHandle->regHandle, regAddr);
    regVal &= ~BCHP_MASK(AUD_FMM_DP_CTRL0_MIXER00_0_RT_VOL_LEVEL, MIXER_OUTPUT_RIGHT_VOLUME_LEVEL);
    regVal |= right;
    BREG_Write32(deviceHandle->regHandle, regAddr, regVal);
#elif defined BCHP_AUD_FMM_DP_CTRL0_MIXER00_RT_VOL_LEVEL
    const uint32_t rightRegOffset = BCHP_AUD_FMM_DP_CTRL0_MIXER00_RT_VOL_LEVEL-BCHP_AUD_FMM_DP_CTRL0_MIXER00_LT_VOL_LEVEL;
    const uint32_t mixerRegOffset = BCHP_AUD_FMM_DP_CTRL0_MIXER10_LT_VOL_LEVEL-BCHP_AUD_FMM_DP_CTRL0_MIXER00_LT_VOL_LEVEL;

    if ( 1 == outputIndex )
    {
        regAddr = BCHP_AUD_FMM_DP_CTRL0_MIXER01_LT_VOL_LEVEL;
    }
    else
    {
        regAddr = BCHP_AUD_FMM_DP_CTRL0_MIXER00_LT_VOL_LEVEL;
    }

    left = BCHP_FIELD_DATA(AUD_FMM_DP_CTRL0_MIXER00_LT_VOL_LEVEL, MIXER_OUTPUT_LEFT_VOLUME_LEVEL, left) &
           BCHP_MASK(AUD_FMM_DP_CTRL0_MIXER00_LT_VOL_LEVEL, MIXER_OUTPUT_LEFT_VOLUME_LEVEL);
    right = BCHP_FIELD_DATA(AUD_FMM_DP_CTRL0_MIXER00_RT_VOL_LEVEL, MIXER_OUTPUT_RIGHT_VOLUME_LEVEL, right) &
            BCHP_MASK(AUD_FMM_DP_CTRL0_MIXER00_RT_VOL_LEVEL, MIXER_OUTPUT_RIGHT_VOLUME_LEVEL);

    regAddr += mixerRegOffset * ((uint32_t) mixerId);
    regVal = BREG_Read32(deviceHandle->regHandle, regAddr);
    regVal &= ~BCHP_MASK(AUD_FMM_DP_CTRL0_MIXER00_LT_VOL_LEVEL, MIXER_OUTPUT_LEFT_VOLUME_LEVEL);            
    regVal |= left;
    BREG_Write32(deviceHandle->regHandle, regAddr, regVal);

    regAddr += rightRegOffset;
    regVal = BREG_Read32(deviceHandle->regHandle, regAddr);
    regVal &= ~BCHP_MASK(AUD_FMM_DP_CTRL0_MIXER00_RT_VOL_LEVEL, MIXER_OUTPUT_RIGHT_VOLUME_LEVEL);
    regVal |= right;
    BREG_Write32(deviceHandle->regHandle, regAddr, regVal);
#elif defined BCHP_AUD_FMM_DP_CTRL0_MIXER_OUTPUT0_LT_VOL_LEVELi_ARRAY_BASE
    const uint32_t rightRegOffset = BCHP_AUD_FMM_DP_CTRL0_MIXER_OUTPUT0_RT_VOL_LEVELi_ARRAY_BASE - BCHP_AUD_FMM_DP_CTRL0_MIXER_OUTPUT0_LT_VOL_LEVELi_ARRAY_BASE;
    const uint32_t mixerRegOffset = BCHP_AUD_FMM_DP_CTRL0_MIXER_OUTPUT0_RT_VOL_LEVELi_ARRAY_ELEMENT_SIZE/8;

    if ( 1 == outputIndex )
    {
        regAddr = BCHP_AUD_FMM_DP_CTRL0_MIXER_OUTPUT1_LT_VOL_LEVELi_ARRAY_BASE;
    }
    else
    {
        regAddr = BCHP_AUD_FMM_DP_CTRL0_MIXER_OUTPUT0_LT_VOL_LEVELi_ARRAY_BASE;
    }

    left = BCHP_FIELD_DATA(AUD_FMM_DP_CTRL0_MIXER_OUTPUT0_LT_VOL_LEVELi, MIXER_OUTPUT_LEFT_VOLUME_LEVEL, left) &
           BCHP_MASK(AUD_FMM_DP_CTRL0_MIXER_OUTPUT0_LT_VOL_LEVELi, MIXER_OUTPUT_LEFT_VOLUME_LEVEL);
    right = BCHP_FIELD_DATA(AUD_FMM_DP_CTRL0_MIXER_OUTPUT0_RT_VOL_LEVELi, MIXER_OUTPUT_RIGHT_VOLUME_LEVEL, right) &
            BCHP_MASK(AUD_FMM_DP_CTRL0_MIXER_OUTPUT0_RT_VOL_LEVELi, MIXER_OUTPUT_RIGHT_VOLUME_LEVEL);

    regAddr += mixerRegOffset * ((uint32_t) mixerId);
    regVal = BREG_Read32(deviceHandle->regHandle, regAddr);
    regVal &= ~BCHP_MASK(AUD_FMM_DP_CTRL0_MIXER_OUTPUT0_LT_VOL_LEVELi, MIXER_OUTPUT_LEFT_VOLUME_LEVEL);            
    regVal |= left;
    BREG_Write32(deviceHandle->regHandle, regAddr, regVal);

    regAddr += rightRegOffset;
    regVal = BREG_Read32(deviceHandle->regHandle, regAddr);
    regVal &= ~BCHP_MASK(AUD_FMM_DP_CTRL0_MIXER_OUTPUT0_RT_VOL_LEVELi, MIXER_OUTPUT_RIGHT_VOLUME_LEVEL);
    regVal |= right;
    BREG_Write32(deviceHandle->regHandle, regAddr, regVal);    
#else
#error Mixer output volume registers undefined
#endif
}

#if !(defined BCHP_AUD_FMM_DP_CTRL0_MIXER0_CONFIG_VOLUME_RAMP_DISABLE_OUTPUT0_MASK || defined BCHP_AUD_FMM_DP_CTRL0_MIXER_CONFIGi_VOLUME_RAMP_ENA_OUTPUT0_MASK)
static bool BAPE_DpMixer_P_MixerOutputIsActive( 
    BAPE_MixerGroupHandle handle, 
    unsigned mixerIndex
    )
{
    uint32_t enabledMixers;

    if ( mixerIndex < BAPE_CHIP_MAX_MIXERS )
    {
        enabledMixers = BREG_Read32(handle->deviceHandle->regHandle, BCHP_AUD_FMM_DP_CTRL0_MIXER_OUTPUT_ENA);
        enabledMixers = (enabledMixers % BAPE_CHIP_MAX_MIXERS) | (enabledMixers >> BAPE_CHIP_MAX_MIXERS); /* combine outputs 0/1 */

        return ( (enabledMixers>>mixerIndex) & 1 );
    }

    return false;
}

static uint32_t BAPE_DpMixer_P_GetActiveMixerInputs( 
    BAPE_MixerGroupHandle handle, 
    unsigned mixerIndex
    )
{
    uint32_t enabledInputs = 0;
    int i;

    if ( mixerIndex < BAPE_CHIP_MAX_MIXERS )
    {
        for ( i = 0; i < BAPE_CHIP_MAX_MIXER_INPUTS; i++ )
        {
            uint32_t bitmask;
            uint32_t regVal, regAddr;

            /* Find input config register (they are paired) */
            regAddr = BAPE_DpMixer_P_GetInputConfigAddress(mixerIndex, i);
            regVal = BREG_Read32(handle->deviceHandle->regHandle, regAddr);

#ifdef BCHP_AUD_FMM_DP_CTRL0_MIXER0_INPUT10_CONFIG
            if ( 0 == (i%2) )
            {
                /* Even.  Read the first enable. */
                bitmask = BCHP_MASK(AUD_FMM_DP_CTRL0_MIXER0_INPUT10_CONFIG, MIXER_INPUT0_ENA);
            }
            else
            {
                /* Odd.  Read the second enable. */
                bitmask = BCHP_MASK(AUD_FMM_DP_CTRL0_MIXER0_INPUT10_CONFIG, MIXER_INPUT1_ENA);
            }
#else
            bitmask = BCHP_MASK(AUD_FMM_DP_CTRL0_MIXER0_INPUT_CONFIGi, MIXER_INPUT_ENA);
#endif

            if ( (regVal & bitmask) != 0 )
            {
                enabledInputs |= (1<<i);
            }
        }
        return enabledInputs;
    }

    return 0;
}

static bool BAPE_DpMixer_P_MixerRampingActive( 
    BAPE_MixerGroupHandle handle
    )
{
    int i;
    uint32_t rampMask;

    rampMask = ( BREG_Read32(handle->deviceHandle->regHandle, BCHP_AUD_FMM_DP_CTRL0_VOLUME_RAMP_STATUS0) |
                 BREG_Read32(handle->deviceHandle->regHandle, BCHP_AUD_FMM_DP_CTRL0_VOLUME_RAMP_STATUS1) );

    for ( i = 0; i < BAPE_CHIP_MAX_MIXERS; i++ )
    {
        if ( (BAPE_DpMixer_P_GetActiveMixerInputs(handle, i) > 0) &&
              BAPE_DpMixer_P_MixerOutputIsActive(handle, i) &&
              (rampMask & (1<<i | 1<<(i+BAPE_CHIP_MAX_MIXERS))) )
        {
            return true;
        }                
    }

    return false;
}
#endif

BERR_Code BAPE_MixerGroup_P_WaitForRamping(
    BAPE_MixerGroupHandle handle
    )
{
#if !(defined BCHP_AUD_FMM_DP_CTRL0_MIXER0_CONFIG_VOLUME_RAMP_DISABLE_OUTPUT0_MASK || defined BCHP_AUD_FMM_DP_CTRL0_MIXER_CONFIGi_VOLUME_RAMP_ENA_OUTPUT0_MASK)
    unsigned rampInterval = (handle->sampleRate > 0) ? (1000000/handle->sampleRate + 1) : BAPE_DP_DEFAULT_RAMP_INTERVAL;
    unsigned timeout = 1000000 - rampInterval;
    
    /* We need to wait for a milisecond to ensure ramping has begun */
    BKNI_Delay(rampInterval);
    /* Spin for all pending ramps to finish */
    while ( BAPE_DpMixer_P_MixerRampingActive(handle) &&
            timeout > 0 )
    {
        /*BDBG_MSG(("Waiting for ramp to complete..."));*/
        BKNI_Delay(1); /* 1 micro second */
        timeout--;
    }

    if ( timeout == 0 )
    {
        BDBG_ERR(("Timed out waiting for ramps to complete"));
        return BERR_TIMEOUT;
    }
    /*BDBG_ERR(("Waited %u us for ramping to complete (RI=%u)", 1000000 - timeout, rampInterval));*/
#else
    BSTD_UNUSED(handle);
#endif

    return BERR_SUCCESS;
}

static void BAPE_MixerGroup_P_ApplyOutputCoefficients(
    BAPE_MixerGroupHandle handle, 
    unsigned outputIndex
    )
{
    bool wasMuted;
    unsigned i;
    uint32_t configRegVal, configRegAddr, bitmask;
#if !(defined BCHP_AUD_FMM_DP_CTRL0_MIXER0_CONFIG_VOLUME_RAMP_DISABLE_OUTPUT0_MASK || defined BCHP_AUD_FMM_DP_CTRL0_MIXER_CONFIGi_VOLUME_RAMP_ENA_OUTPUT0_MASK)
    uint32_t oldRampStep=0;
#endif

    /* Cases to be handled.  */
    /* Mute -> Unmute - Load Coefs first, then unmute so mixer ramps properly. */
    /* Unmute -> Mute - Set Mute First, starts ramping immediately.  Then set coefs. */
    /* Unmute -> Unmute (change in coefs only) */

#if !(defined BCHP_AUD_FMM_DP_CTRL0_MIXER0_CONFIG_VOLUME_RAMP_DISABLE_OUTPUT0_MASK || defined BCHP_AUD_FMM_DP_CTRL0_MIXER_CONFIGi_VOLUME_RAMP_ENA_OUTPUT0_MASK)
    /* Ugh.  No direct support for disabling output ramp.  Need to spin while any pending ramp is going on, jam the ramp step to max, change vol, restore */
    if ( handle->outputs[outputIndex].settings.volumeRampDisabled )
    {
        if ( BAPE_MixerGroup_P_WaitForRamping(handle) == BERR_TIMEOUT )
        {
            BDBG_ERR(("WARNING - %s - Vol Ramp timed out...(DISABLE)", __FUNCTION__));
        }

        /* Set max ramp step value */
        BAPE_GetOutputVolumeRampStep(handle->deviceHandle, &oldRampStep);
        BAPE_SetOutputVolumeRampStep(handle->deviceHandle, BAPE_DP_MAX_RAMP_STEP);
    }
#endif

    for ( i = 0; i < handle->numChannelPairs; i++ )
    {
        /* Get Current Mute Status */
        configRegAddr = BAPE_DpMixer_P_GetConfigAddress(handle->mixerIds[i]);
        configRegVal = BREG_Read32(handle->deviceHandle->regHandle, configRegAddr);
#ifdef BCHP_AUD_FMM_DP_CTRL0_MIXER0_CONFIG
        bitmask = (outputIndex == 0) ?
                  BCHP_MASK(AUD_FMM_DP_CTRL0_MIXER0_CONFIG, VOLUME_MUTE_ENA_OUTPUT0):
                  BCHP_MASK(AUD_FMM_DP_CTRL0_MIXER0_CONFIG, VOLUME_MUTE_ENA_OUTPUT1);
#else
        bitmask = (outputIndex == 0) ?
                  BCHP_MASK(AUD_FMM_DP_CTRL0_MIXER_CONFIGi, VOLUME_MUTE_ENA_OUTPUT0):
                  BCHP_MASK(AUD_FMM_DP_CTRL0_MIXER_CONFIGi, VOLUME_MUTE_ENA_OUTPUT1);
#endif
        wasMuted = (configRegVal & bitmask) ? true : false;

        /* Handle Ramp Disable First */
#if defined BCHP_AUD_FMM_DP_CTRL0_MIXER0_CONFIG_VOLUME_RAMP_DISABLE_OUTPUT0_MASK
        /* Sweet, the mixer supports it on a per-output basis. */
        if ( outputIndex == 0 )
        {
            configRegVal &= ~BCHP_MASK(AUD_FMM_DP_CTRL0_MIXER0_CONFIG, VOLUME_RAMP_DISABLE_OUTPUT0);
            configRegVal |= BCHP_FIELD_DATA(AUD_FMM_DP_CTRL0_MIXER0_CONFIG, VOLUME_RAMP_DISABLE_OUTPUT0, handle->outputs[outputIndex].settings.volumeRampDisabled?1:0);
        }
        else
        {
            configRegVal &= ~BCHP_MASK(AUD_FMM_DP_CTRL0_MIXER0_CONFIG, VOLUME_RAMP_DISABLE_OUTPUT1);
            configRegVal |= BCHP_FIELD_DATA(AUD_FMM_DP_CTRL0_MIXER0_CONFIG, VOLUME_RAMP_DISABLE_OUTPUT1, handle->outputs[outputIndex].settings.volumeRampDisabled?1:0);
        }
#elif defined BCHP_AUD_FMM_DP_CTRL0_MIXER_CONFIGi_VOLUME_RAMP_ENA_OUTPUT0_MASK
        /* Sweet, the mixer supports it on a per-output basis. */
        if ( outputIndex == 0 )
        {
            configRegVal &= ~BCHP_MASK(AUD_FMM_DP_CTRL0_MIXER_CONFIGi, VOLUME_RAMP_ENA_OUTPUT0);
            configRegVal |= BCHP_FIELD_DATA(AUD_FMM_DP_CTRL0_MIXER_CONFIGi, VOLUME_RAMP_ENA_OUTPUT0, handle->outputs[outputIndex].settings.volumeRampDisabled?0:1);
        }
        else
        {
            configRegVal &= ~BCHP_MASK(AUD_FMM_DP_CTRL0_MIXER_CONFIGi, VOLUME_RAMP_ENA_OUTPUT0);
            configRegVal |= BCHP_FIELD_DATA(AUD_FMM_DP_CTRL0_MIXER_CONFIGi, VOLUME_RAMP_ENA_OUTPUT0, handle->outputs[outputIndex].settings.volumeRampDisabled?0:1);
        }        
#endif

        /* Determine if we're going to mute and if so do that first to start the ramp. */
        if ( !wasMuted && handle->outputs[outputIndex].settings.muted )
        {
            configRegVal |= bitmask;
        }

        /* Update config register with any changes */
        BREG_Write32(handle->deviceHandle->regHandle, configRegAddr, configRegVal);

        /* Now set coefficients */
        if ( !handle->outputs[outputIndex].settings.muted )
        {
            BAPE_DpMixer_P_LoadOutputCoefs(handle->deviceHandle, handle->mixerIds[i], outputIndex, 
                                           handle->outputs[outputIndex].settings.coefficients[2*i],
                                           handle->outputs[outputIndex].settings.coefficients[(2*i)+1]);

            if ( wasMuted )
            {
                /* Done with coefficients.  Now Unmute if we need to. */
                configRegVal &= ~bitmask;
                BREG_Write32(handle->deviceHandle->regHandle, configRegAddr, configRegVal);
            }
        }
    }

    /* Restore ramp step if required. */
#if !(defined BCHP_AUD_FMM_DP_CTRL0_MIXER0_CONFIG_VOLUME_RAMP_DISABLE_OUTPUT0_MASK || defined BCHP_AUD_FMM_DP_CTRL0_MIXER_CONFIGi_VOLUME_RAMP_ENA_OUTPUT0_MASK)
    if ( handle->outputs[outputIndex].settings.volumeRampDisabled )
    {
        if ( BAPE_MixerGroup_P_WaitForRamping(handle) == BERR_TIMEOUT )
        {
            BDBG_ERR(("WARNING - %s - Vol Ramp timed out...(RESTORE)", __FUNCTION__));
        }

        /* Restore original value */
        BAPE_SetOutputVolumeRampStep(handle->deviceHandle, oldRampStep);
    }
#endif
}

/***************************************************************************
Summary:
Set the Samplerate for the Mixer Group.

The Samplerate is used to calculate the ramp step interval.
The Ramp Interval is based on the samplerate and is needed to know the
amount of time to wait after changing coeffs prior to reading the ramp status.
This only applies to older 40nm chips that have polling type ramp status.
***************************************************************************/
void BAPE_MixerGroup_P_SetRampSamplerate_isr(
    BAPE_MixerGroupHandle handle,
    unsigned sampleRate
    )
{
    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(handle->allocated);

    handle->sampleRate = sampleRate;
}

void BAPE_GetOutputVolumeRampStep(
    BAPE_Handle deviceHandle,
    uint32_t *pRampStep                 /* All mixers output output volume is changed by this amount
                                                                            every Fs while ramping.  Specified in 4.23 format. 
                                                                            Ignored for compressed data. */
    )
{
    BDBG_OBJECT_ASSERT(deviceHandle, BAPE_Device);
    BDBG_ASSERT(NULL != pRampStep);

    *pRampStep = deviceHandle->outputVolumeRampStep;
    return;
}

BERR_Code BAPE_SetOutputVolumeRampStep(
    BAPE_Handle deviceHandle,
    uint32_t rampStep                   /* All mixers output output volume is changed by this amount
                                                                                 every Fs while ramping.  Specified in 4.23 format. 
                                                                                 Ignored for compressed data. */
    )
{
    uint32_t regVal;
    BDBG_OBJECT_ASSERT(deviceHandle, BAPE_Device);

#if defined BCHP_AUD_FMM_DP_CTRL0_VOLUME_RAMP_UP_STEP
    regVal = BREG_Read32(deviceHandle->regHandle, BCHP_AUD_FMM_DP_CTRL0_VOLUME_RAMP_UP_STEP);
    regVal &= ~BCHP_MASK(AUD_FMM_DP_CTRL0_VOLUME_RAMP_UP_STEP, VOLUME_RAMP_UP_STEP);
    regVal |= BCHP_FIELD_DATA(AUD_FMM_DP_CTRL0_VOLUME_RAMP_UP_STEP, VOLUME_RAMP_UP_STEP, rampStep);
    BREG_Write32(deviceHandle->regHandle, BCHP_AUD_FMM_DP_CTRL0_VOLUME_RAMP_UP_STEP, regVal);
    BREG_Write32(deviceHandle->regHandle, BCHP_AUD_FMM_DP_CTRL0_VOLUME_RAMP_DOWN_STEP, regVal);

    /* Save the setting in the BAPE_Device struct.  Use the value from the register field in case the user's value was truncated. */
    deviceHandle->outputVolumeRampStep = BCHP_GET_FIELD_DATA(regVal, AUD_FMM_DP_CTRL0_VOLUME_RAMP_UP_STEP, VOLUME_RAMP_UP_STEP);

#elif defined BCHP_AUD_FMM_DP_CTRL0_VOLUME_RAMP_STEP
    regVal = BREG_Read32(deviceHandle->regHandle, BCHP_AUD_FMM_DP_CTRL0_VOLUME_RAMP_STEP);
    regVal &= ~BCHP_MASK(AUD_FMM_DP_CTRL0_VOLUME_RAMP_STEP, VOLUME_RAMP_STEP);
    regVal |= BCHP_FIELD_DATA(AUD_FMM_DP_CTRL0_VOLUME_RAMP_STEP, VOLUME_RAMP_STEP, rampStep);
    BREG_Write32(deviceHandle->regHandle, BCHP_AUD_FMM_DP_CTRL0_VOLUME_RAMP_STEP, regVal);

    /* Save the setting in the BAPE_Device struct.  Use the value from the register field in case the user's value was truncated. */
    deviceHandle->outputVolumeRampStep = BCHP_GET_FIELD_DATA(regVal, AUD_FMM_DP_CTRL0_VOLUME_RAMP_STEP, VOLUME_RAMP_STEP);

#else
#error Mixer output ramp step not defined
#endif
    return BERR_SUCCESS;
}

static uint32_t BAPE_DpMixer_P_GetConfigAddress(
    unsigned mixerId
    )
{
    switch ( mixerId )
    {
    #if BCHP_CHIP == 7408
    case 5:
        /* 7408 made this discontiguous for some reason... */
        return BCHP_AUD_FMM_DP_CTRL0_MIXER5_CONFIG;
    #endif
    default:
        BDBG_ASSERT(mixerId < BAPE_CHIP_MAX_MIXERS);
    #ifdef BCHP_AUD_FMM_DP_CTRL0_MIXER0_CONFIG
        return BCHP_AUD_FMM_DP_CTRL0_MIXER0_CONFIG + ((BCHP_AUD_FMM_DP_CTRL0_MIXER1_CONFIG-BCHP_AUD_FMM_DP_CTRL0_MIXER0_CONFIG)*mixerId);
    #else
        return BCHP_AUD_FMM_DP_CTRL0_MIXER_CONFIGi_ARRAY_BASE + ((BCHP_AUD_FMM_DP_CTRL0_MIXER_CONFIGi_ARRAY_ELEMENT_SIZE/8)*mixerId);
    #endif
    }
}

static uint32_t BAPE_DpMixer_P_GetInputConfigAddress(
    unsigned mixerId,
    unsigned inputId
    )
{
    uint32_t regAddr;
#ifdef BCHP_AUD_FMM_DP_CTRL0_MIXER0_INPUT10_CONFIG
    regAddr = BCHP_AUD_FMM_DP_CTRL0_MIXER0_INPUT10_CONFIG + (mixerId*(BCHP_AUD_FMM_DP_CTRL0_MIXER1_INPUT10_CONFIG - BCHP_AUD_FMM_DP_CTRL0_MIXER0_INPUT10_CONFIG));
    regAddr += 4*(inputId/2);
#else
    switch ( mixerId )
    {
    case 0:
        regAddr = BCHP_AUD_FMM_DP_CTRL0_MIXER0_INPUT_CONFIGi_ARRAY_BASE; 
        break;
#ifdef BCHP_AUD_FMM_DP_CTRL0_MIXER1_INPUT_CONFIGi_ARRAY_BASE
    case 1:
        regAddr = BCHP_AUD_FMM_DP_CTRL0_MIXER1_INPUT_CONFIGi_ARRAY_BASE; 
        break;
#endif
#ifdef BCHP_AUD_FMM_DP_CTRL0_MIXER2_INPUT_CONFIGi_ARRAY_BASE
    case 2:
        regAddr = BCHP_AUD_FMM_DP_CTRL0_MIXER2_INPUT_CONFIGi_ARRAY_BASE; 
        break;
#endif
#ifdef BCHP_AUD_FMM_DP_CTRL0_MIXER3_INPUT_CONFIGi_ARRAY_BASE
    case 3:
        regAddr = BCHP_AUD_FMM_DP_CTRL0_MIXER3_INPUT_CONFIGi_ARRAY_BASE; 
        break;
#endif
#ifdef BCHP_AUD_FMM_DP_CTRL0_MIXER4_INPUT_CONFIGi_ARRAY_BASE
    case 4:
        regAddr = BCHP_AUD_FMM_DP_CTRL0_MIXER4_INPUT_CONFIGi_ARRAY_BASE; 
        break;
#endif
#ifdef BCHP_AUD_FMM_DP_CTRL0_MIXER5_INPUT_CONFIGi_ARRAY_BASE
    case 5:
        regAddr = BCHP_AUD_FMM_DP_CTRL0_MIXER5_INPUT_CONFIGi_ARRAY_BASE; 
        break;
#endif
#ifdef BCHP_AUD_FMM_DP_CTRL0_MIXER6_INPUT_CONFIGi_ARRAY_BASE
    case 6:
        regAddr = BCHP_AUD_FMM_DP_CTRL0_MIXER6_INPUT_CONFIGi_ARRAY_BASE; 
        break;
#endif
#ifdef BCHP_AUD_FMM_DP_CTRL0_MIXER7_INPUT_CONFIGi_ARRAY_BASE
    case 7:
        regAddr = BCHP_AUD_FMM_DP_CTRL0_MIXER7_INPUT_CONFIGi_ARRAY_BASE; 
        break;
#endif
#ifdef BCHP_AUD_FMM_DP_CTRL0_MIXER8_INPUT_CONFIGi_ARRAY_BASE
    case 8:
        regAddr = BCHP_AUD_FMM_DP_CTRL0_MIXER8_INPUT_CONFIGi_ARRAY_BASE; 
        break;
#endif
#ifdef BCHP_AUD_FMM_DP_CTRL0_MIXER9_INPUT_CONFIGi_ARRAY_BASE
    case 9:
        regAddr = BCHP_AUD_FMM_DP_CTRL0_MIXER9_INPUT_CONFIGi_ARRAY_BASE; 
        break;
#endif
#ifdef BCHP_AUD_FMM_DP_CTRL0_MIXER10_INPUT_CONFIGi_ARRAY_BASE
    case 10:
        regAddr = BCHP_AUD_FMM_DP_CTRL0_MIXER10_INPUT_CONFIGi_ARRAY_BASE; 
        break;
#endif
#ifdef BCHP_AUD_FMM_DP_CTRL0_MIXER11_INPUT_CONFIGi_ARRAY_BASE
    case 11:
        regAddr = BCHP_AUD_FMM_DP_CTRL0_MIXER11_INPUT_CONFIGi_ARRAY_BASE; 
        break;
#endif
#ifdef BCHP_AUD_FMM_DP_CTRL0_MIXER12_INPUT_CONFIGi_ARRAY_BASE
        #error need to add support for additional mixers
#endif
    default:
        BDBG_ASSERT(mixerId < BAPE_CHIP_MAX_MIXERS);
        return 0xffffffff;
    }
    regAddr += (BCHP_AUD_FMM_DP_CTRL0_MIXER0_INPUT_CONFIGi_ARRAY_ELEMENT_SIZE/8)*inputId;
#endif
    return regAddr;
}
