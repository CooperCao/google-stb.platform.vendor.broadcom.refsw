/***************************************************************************
 *     Copyright (c) 2006-2010, Broadcom Corporation
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
 * Module Description: Audio Decoder Interface
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
#include "bchp_aud_fmm_dp_ctrl0.h"
#include "bchp_aud_fmm_src_ctrl0.h"
#include "bchp_aud_fmm_bf_ctrl.h"

BDBG_MODULE(bape_resources);

static BERR_Code BAPE_P_AllocateSingleMixer(BAPE_Handle handle, uint8_t *pId);
static BERR_Code BAPE_P_AllocateMixerGroup(BAPE_Handle handle, unsigned numChannelPairs, uint8_t *pIdBase);
static BERR_Code BAPE_P_AllocateSingleSource(BAPE_Handle handle, uint8_t *pSourceChannel, uint8_t *pSrc, uint8_t *pPlayback, bool ramp);
static BERR_Code BAPE_P_AllocateSourceGroup(BAPE_Handle handle, unsigned numChannelPairs, uint8_t sourceChannels[BAPE_ChannelPair_eMax], uint8_t srcs[BAPE_ChannelPair_eMax], uint8_t playbacks[BAPE_ChannelPair_eMax], bool ramp);
static void BAPE_P_GarbageCollect(BAPE_Handle handle);

static void BAPE_P_SetPlaybackLinkage(BAPE_Handle handle, uint32_t playbackId, uint32_t fciId);
static void BAPE_P_InvalidatePlayback(BAPE_Handle handle, uint32_t playbackId);
static void BAPE_P_SetSrcLinkage(BAPE_Handle handle, uint32_t srcId, uint32_t fciId, uint32_t groupId, bool rampEnabled);
static void BAPE_P_SetSourceChannelGroup(BAPE_Handle handle, uint32_t sourceChannelId, uint32_t groupId);

/*************************************************************************** 
  The resource pools are managed in a simple manner to permit grouping and
  attempt to avoid fragmentation of resources.  For any given resource,
  if only one channel pair is required (not grouped), resources are allocated
  from the bottom of the pool (0..n).  For grouped resources, they are
  allocated from the top of the pool (n..0).  
***************************************************************************/

BERR_Code BAPE_P_AllocateMixers(BAPE_MixerHandle mixer, unsigned numOutputs, unsigned numChannelPairs)
{
    uint32_t regAddr, regVal;
    unsigned i, j;
    BERR_Code errCode=BERR_SUCCESS;
    uint8_t idBase=0;

    BDBG_OBJECT_ASSERT(mixer, BAPE_Mixer);
    BDBG_ASSERT(numOutputs > 0);
    BDBG_ASSERT(numChannelPairs > 0 && numChannelPairs < BAPE_ChannelPair_eMax);
    
#if BDBG_DEBUG_BUILD
    /* Sanity check, make sure all resources are marked invalid in the mixer */
    for ( i = 0; i < BAPE_CHIP_MAX_MIXERS; i++ )
    {
        for ( j = 0; j < BAPE_ChannelPair_eMax; j++ )
        {
            BDBG_ASSERT(mixer->dpMixers[j][i] == 0xff);
        }
    }
#endif

    /* Handle stereo and compressed by allocating low -> high */
    if ( 1 == numChannelPairs )
    {
        BDBG_MSG(("Attempting to allocate %u DP mixers for mixer %p (%u)", numOutputs, mixer, mixer->index));
        for ( i = 0; i < numOutputs; i++ )
        {
            errCode = BAPE_P_AllocateSingleMixer(mixer->deviceHandle, &mixer->dpMixers[BAPE_ChannelPair_eLeftRight][i]);
            if ( errCode )
            {
                errCode = BERR_TRACE(errCode);
                goto error;
            }
        }
    }
    else
    {
        BDBG_MSG(("Attempting to allocate %u DP mixer groups (%u pairs) for mixer %p (%u)", numOutputs, numChannelPairs, mixer, mixer->index));
        for ( i = 0; i < numOutputs; i++ )
        {
            errCode = BAPE_P_AllocateMixerGroup(mixer->deviceHandle, 
                                                numChannelPairs,
                                                &idBase);
            if ( errCode )
            {
                errCode = BERR_TRACE(errCode);
                goto error;
            }
            for ( j = 0; j < numChannelPairs; j++ )
            {
                mixer->dpMixers[j][i] = idBase + j;
            }
        }
    }

    /* Setup the mixer group IDs */
    for ( i = 0; i < numOutputs; i++ )
    {
        for ( j = 0; j < numChannelPairs; j++ )
        {
            regAddr = BAPE_P_GetMixerConfigAddress(mixer->dpMixers[j][i]);
            regVal = BREG_Read32(mixer->deviceHandle->regHandle, regAddr);
            regVal &= ~(BCHP_MASK(AUD_FMM_DP_CTRL0_MIXER0_CONFIG, MIXER_GROUP_BEGIN)|
                        BCHP_MASK(AUD_FMM_DP_CTRL0_MIXER0_CONFIG, VOLUME_ENA));
            regVal |= BCHP_FIELD_DATA(AUD_FMM_DP_CTRL0_MIXER0_CONFIG, MIXER_GROUP_BEGIN, (uint32_t)mixer->dpMixers[BAPE_ChannelPair_eLeftRight][i]);
            if ( !mixer->compressed )
            {
                regVal |= BCHP_FIELD_DATA(AUD_FMM_DP_CTRL0_MIXER0_CONFIG, VOLUME_ENA, 1);
            }
            BREG_Write32(mixer->deviceHandle->regHandle, regAddr, regVal);
            BDBG_MSG(("Set mixer %u group ID to %u", mixer->dpMixers[j][i], mixer->dpMixers[BAPE_ChannelPair_eLeftRight][i]));
        }
    }

    /* Done */
    return BERR_SUCCESS;

error:
    BAPE_P_FreeMixers(mixer);
    return errCode;        
}

BERR_Code BAPE_P_AllocateInputResources(BAPE_Handle deviceHandle, BAPE_MixerInput input)
{
    BERR_Code errCode = BERR_SUCCESS;
    unsigned i, numChannelPairs=0;
    bool ramp;

    /* This will allocate the input resources required for an object that branches at SRC */
    BDBG_OBJECT_ASSERT(deviceHandle, BAPE_Device);
    BDBG_OBJECT_ASSERT(input, BAPE_MixerInput);
    BDBG_ASSERT(true == input->mustBranchAtSrc);
    BDBG_ASSERT(0 == input->activeLinks);
    
#if BDBG_DEBUG_BUILD
    /* Sanity check, make sure all resources are marked invalid in the input */
    for ( i = 0; i < BAPE_ChannelPair_eMax; i++ )
    {
        BDBG_ASSERT(0xff == input->sourceChannelId[i]);
        BDBG_ASSERT(0xff == input->srcId[i]);
        BDBG_ASSERT(0xff == input->playbackId[i]);
    }
#else
    BSTD_UNUSED(i);
#endif
        
    numChannelPairs = input->numChannelPairs;        

    if ( !input->compressed && deviceHandle->settings.rampPcmSamples )
    {
        ramp = true;
    }
    else
    {
        ramp = false;
    }

    if ( numChannelPairs == 1 )
    {
        errCode = BAPE_P_AllocateSingleSource(deviceHandle, &input->sourceChannelId[0], &input->srcId[0], &input->playbackId[0], ramp);
        if ( errCode )
        {
            errCode = BERR_TRACE(errCode);
            goto error;
        }
    }
    else
    {
        errCode = BAPE_P_AllocateSourceGroup(deviceHandle, numChannelPairs, input->sourceChannelId, input->srcId, input->playbackId, ramp);
        if ( errCode )
        {
            errCode = BERR_TRACE(errCode);
            goto error;
        }
    }

    return BERR_SUCCESS;
error:
    BAPE_P_FreeInputResources(deviceHandle, input);
    return errCode;
}

/***************************************************************************
Summary:
Allocate Input Link Resources for inputs that use multiple Source Channels
***************************************************************************/
BERR_Code BAPE_P_AllocateInputLinkResources(BAPE_Handle deviceHandle, BAPE_MixerInputLink *pLink)
{
    BERR_Code errCode = BERR_SUCCESS;
    unsigned i, numChannelPairs=0;
    BAPE_MixerInput input;
    bool ramp;

    /* This will allocate the input resources required for an object that has multiple source channels */
    BDBG_OBJECT_ASSERT(deviceHandle, BAPE_Device);
    BDBG_OBJECT_ASSERT(pLink, BAPE_MixerInputLink);
    BDBG_ASSERT(false == pLink->input->mustBranchAtSrc);
    
    input = pLink->input;

#if BDBG_DEBUG_BUILD
    /* Sanity check, make sure all resources are marked invalid in the input */
    for ( i = 0; i < BAPE_ChannelPair_eMax; i++ )
    {
        BDBG_ASSERT(0xff == pLink->sourceChannelId[i]);
        BDBG_ASSERT(0xff == pLink->srcId[i]);
        BDBG_ASSERT(0xff == pLink->playbackId[i]);
    }
#else
    BSTD_UNUSED(i);
#endif

    if ( !input->compressed && deviceHandle->settings.rampPcmSamples )
    {
        ramp = true;
    }
    else
    {
        ramp = false;
    }

    numChannelPairs = input->numChannelPairs;

    if ( numChannelPairs == 1 )
    {
        errCode = BAPE_P_AllocateSingleSource(deviceHandle, &pLink->sourceChannelId[0], &pLink->srcId[0], &pLink->playbackId[0], ramp);
        if ( errCode )
        {
            errCode = BERR_TRACE(errCode);
            goto error;
        }
    }
    else
    {
        errCode = BAPE_P_AllocateSourceGroup(deviceHandle, numChannelPairs, pLink->sourceChannelId, pLink->srcId, pLink->playbackId, ramp);
        if ( errCode )
        {
            errCode = BERR_TRACE(errCode);
            goto error;
        }
    }

    return BERR_SUCCESS;
error:
    BAPE_P_FreeInputLinkResources(deviceHandle, pLink);
    return errCode;
}

/***************************************************************************
Summary:
Release mixers and playbacks to the resource pool
***************************************************************************/
void BAPE_P_FreeMixers(BAPE_MixerHandle mixer)
{
    unsigned i, j;
    BDBG_OBJECT_ASSERT(mixer, BAPE_Mixer);
    
    /* Release the mixers back to the pool */
    for ( i = 0; i < BAPE_ChannelPair_eMax; i++ )
    {
        for ( j = 0; j < BAPE_CHIP_MAX_MIXERS; j++ )
        {
            if ( mixer->dpMixers[i][j] != 0xff )
            {
                BDBG_ASSERT(mixer->dpMixers[i][j] < BAPE_CHIP_MAX_MIXERS);
                BDBG_MSG(("Releasing mixer %u to the pool", mixer->dpMixers[i][j]));
                mixer->deviceHandle->mixerAllocated[mixer->dpMixers[i][j]] = false;
                mixer->dpMixers[i][j] = 0xff;
            }
        }
    }
}

/***************************************************************************
Summary:
Free Input Resources for inputs that branch at SRC
***************************************************************************/
void BAPE_P_FreeInputResources(BAPE_Handle deviceHandle, BAPE_MixerInput input)
{
    unsigned i;

    BDBG_OBJECT_ASSERT(deviceHandle, BAPE_Device);
    BDBG_OBJECT_ASSERT(input, BAPE_MixerInput);
    BDBG_ASSERT(true == input->mustBranchAtSrc);
    BDBG_ASSERT(0 == input->activeLinks);

    for ( i = 0; i < BAPE_ChannelPair_eMax; i++ )
    {
        /* All three must be valid if source channel is valid. */
        if ( input->sourceChannelId[i] != 0xff )
        {
            uint32_t regAddr, x;
            BDBG_ASSERT(input->sourceChannelId[i] < BAPE_CHIP_MAX_SOURCE_CHANNELS);
            BDBG_ASSERT(input->srcId[i] < BAPE_CHIP_MAX_SRCS);
            BDBG_ASSERT(input->playbackId[i] < BAPE_CHIP_MAX_MIXER_PLAYBACKS);
            BDBG_MSG(("Releasing source channel %u, src %u, and playback %u to the pool.", 
                      input->sourceChannelId[i], input->srcId[i], input->playbackId[i]));
            deviceHandle->sourceChannelAllocated[input->sourceChannelId[i]] = false;
            deviceHandle->srcAllocated[input->srcId[i]] = false;
            BAPE_P_InvalidatePlayback(deviceHandle, input->playbackId[i]);
            deviceHandle->playbackAllocated[input->playbackId[i]] = false;
            /* Clear ringbuf registers for this SFIFO */
            regAddr = BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR + 
                (BCHP_AUD_FMM_BF_CTRL_RINGBUF_2_RDADDR - BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR)*(uint32_t)input->sourceChannelId[i];
            for ( x = 0; x < (BCHP_AUD_FMM_BF_CTRL_RINGBUF_2_RDADDR - BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR); x+= 4 )
            {
                BREG_Write32(deviceHandle->regHandle, regAddr+x, 0);
            }
            input->sourceChannelId[i] = input->srcId[i] = input->playbackId[i] = 0xff;
        }
    }
}

/***************************************************************************
Summary:
Free Input Link Resources for inputs that use multiple Source Channels
***************************************************************************/
void BAPE_P_FreeInputLinkResources(BAPE_Handle deviceHandle, BAPE_MixerInputLink *pLink)
{
    unsigned i;

    /* This will allocate the input resources required for an object that has multiple source channels */
    BDBG_OBJECT_ASSERT(deviceHandle, BAPE_Device);
    BDBG_OBJECT_ASSERT(pLink, BAPE_MixerInputLink);
    BDBG_ASSERT(false == pLink->input->mustBranchAtSrc);

    for ( i = 0; i < BAPE_ChannelPair_eMax; i++ )
    {
        /* All three must be valid if source channel is valid. */
        if ( pLink->sourceChannelId[i] != 0xff )
        {
            uint32_t regAddr, x;
            BDBG_ASSERT(pLink->sourceChannelId[i] < BAPE_CHIP_MAX_SOURCE_CHANNELS);
            BDBG_ASSERT(pLink->srcId[i] < BAPE_CHIP_MAX_SRCS);
            BDBG_ASSERT(pLink->playbackId[i] < BAPE_CHIP_MAX_MIXER_PLAYBACKS);
            BDBG_MSG(("Releasing source channel %u, src %u, and playback %u to the pool.", 
                      pLink->sourceChannelId[i], pLink->srcId[i], pLink->playbackId[i]));
            deviceHandle->sourceChannelAllocated[pLink->sourceChannelId[i]] = false;
            deviceHandle->srcAllocated[pLink->srcId[i]] = false;
            BAPE_P_InvalidatePlayback(deviceHandle, pLink->playbackId[i]);
            deviceHandle->playbackAllocated[pLink->playbackId[i]] = false;
            /* Clear ringbuf registers for this SFIFO */
            regAddr = BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR + 
                (BCHP_AUD_FMM_BF_CTRL_RINGBUF_2_RDADDR - BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR)*(uint32_t)pLink->sourceChannelId[i];
            for ( x = 0; x < (BCHP_AUD_FMM_BF_CTRL_RINGBUF_2_RDADDR - BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR); x+= 4 )
            {
                BREG_Write32(deviceHandle->regHandle, regAddr+x, 0);
            }
            pLink->sourceChannelId[i] = pLink->srcId[i] = pLink->playbackId[i] = 0xff;
        }
    }
}

static void BAPE_P_InvalidatePlayback(BAPE_Handle handle, uint32_t playbackId)
{
    BAPE_P_SetPlaybackLinkage(handle, playbackId, 0);
}

static void BAPE_P_SetPlaybackLinkage(BAPE_Handle handle, uint32_t playbackId, uint32_t fciId)
{
    uint32_t regAddr;

    BDBG_OBJECT_ASSERT(handle, BAPE_Device);
    BDBG_ASSERT(playbackId < BAPE_CHIP_MAX_MIXER_PLAYBACKS);
    
    regAddr = BCHP_AUD_FMM_DP_CTRL0_PB_FCI_IDi_ARRAY_BASE + (4*playbackId);
    BREG_Write32(handle->regHandle, regAddr, fciId);
}

static BERR_Code BAPE_P_AllocateSingleMixer(BAPE_Handle handle, uint8_t *pId)
{
    unsigned i;
    bool retry=false;

    BDBG_OBJECT_ASSERT(handle, BAPE_Device);
    BDBG_ASSERT(NULL != pId);

retry:
    for ( i = 0; i < BAPE_CHIP_MAX_MIXERS; i++ )
    {
        if ( !handle->mixerAllocated[i] )
        {
            *pId = (uint8_t)i;
            handle->mixerAllocated[i] = true;
            return BERR_SUCCESS;
        }
    }

    if ( false == retry )
    {
        BDBG_MSG(("Unable to allocate mixer.  Attempting garbage collection."));
        BAPE_P_GarbageCollect(handle);
        retry = true;
        goto retry;
    }

    BDBG_ERR(("Unable to allocate mixer."));
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

static BERR_Code BAPE_P_AllocateMixerGroup(BAPE_Handle handle, unsigned numChannelPairs, uint8_t *pIdBase)
{
    unsigned i, j;
    bool retry=false;

    BDBG_OBJECT_ASSERT(handle, BAPE_Device);
    BDBG_ASSERT(numChannelPairs > 1 && numChannelPairs < BAPE_ChannelPair_eMax);
    BDBG_ASSERT(NULL != pIdBase);

    /* Must find enough consecutive mixers to fulfill the request, starting at the top instead of the bottom of the pool. */
retry:
    for ( i = BAPE_CHIP_MAX_MIXERS-1; i >= numChannelPairs; i-- )
    {
        bool success = true;
        for ( j = 0; j < numChannelPairs; j++ )
        {
            if ( handle->mixerAllocated[i-j] )
            {
                success = false;
                break;
            }
        }
        if ( !success )
        {
            /* Not enough consecutive mixers */
            continue;
        }
        /* mark them as used */
        for ( j = 0; j < numChannelPairs; j++ )
        {
            handle->mixerAllocated[i-j] = true;
        }
        *pIdBase = i - (numChannelPairs-1);
        return BERR_SUCCESS;
    }

    if ( false == retry )
    {
        BDBG_MSG(("Unable to allocate mixer group (%u channel pairs).  Attempting garbage collection.", numChannelPairs));
        BAPE_P_GarbageCollect(handle);
        retry = true;
        goto retry;
    }

    BDBG_ERR(("Unable to allocate mixer group (%u channel pairs).", numChannelPairs));
    return BERR_TRACE(BERR_NOT_SUPPORTED);    
}

static BERR_Code BAPE_P_AllocateSingleSource(BAPE_Handle handle, uint8_t *pSourceChannel, uint8_t *pSrc, uint8_t *pPlayback, bool ramp)
{
    unsigned i;
    bool retry=false;
    uint8_t sourceChannel, src, playback;

    BDBG_OBJECT_ASSERT(handle, BAPE_Device);
    BDBG_ASSERT(NULL != pSourceChannel);
    BDBG_ASSERT(NULL != pSrc);
    BDBG_ASSERT(NULL != pPlayback);

retry:
    sourceChannel = src = playback = 0xff;
    for ( i = 0; i < BAPE_CHIP_MAX_SOURCE_CHANNELS; i++ )
    {
        if ( !handle->sourceChannelAllocated[i] )
        {
            sourceChannel = i;
            break;
        }
    }
    for ( i = 0; i < BAPE_CHIP_MAX_SRCS; i++ )
    {
        if ( !handle->srcAllocated[i] )
        {
            src = i;
            break;
        }
    }
    for ( i = 0; i < BAPE_CHIP_MAX_MIXER_PLAYBACKS; i++ )
    {
        if ( !handle->playbackAllocated[i] )
        {
            playback = i;
            break;
        }
    }
    if ( sourceChannel == 0xff || src == 0xff || playback == 0xff )
    {
        if ( !retry )
        {
            retry = true;
            BDBG_MSG(("Unable to allocate source channel/src/playback.  Attempting garbage collection."));
            BAPE_P_GarbageCollect(handle);
            goto retry;
        }
        else
        {
            BDBG_ERR(("Unable to allocate source channel/src/playback."));            
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }

    /* If we get here, we have the resources.  Program them. */
    /* Set playback FCI ID to SRC output */
    BDBG_MSG(("Linking playback %u to SRC %u output (ungrouped)", playback, src));
    BAPE_P_SetPlaybackLinkage(handle, playback, (0x2<<6)|(uint32_t)src);
    BDBG_MSG(("Linking Source Channel %u to SRC %u (ungrouped)", sourceChannel, src));
    BAPE_P_SetSrcLinkage(handle, src, sourceChannel, src, ramp);
    BAPE_P_SetSourceChannelGroup(handle, sourceChannel, sourceChannel);
    *pSourceChannel = sourceChannel;
    *pSrc = src;
    *pPlayback = playback;
    handle->sourceChannelAllocated[sourceChannel] = true;
    handle->srcAllocated[src] = true;
    handle->playbackAllocated[playback] = true;

    return BERR_SUCCESS;
}

static BERR_Code BAPE_P_AllocateSourceGroup(BAPE_Handle handle, unsigned numChannelPairs, uint8_t sourceChannels[BAPE_ChannelPair_eMax], uint8_t srcs[BAPE_ChannelPair_eMax], uint8_t playbacks[BAPE_ChannelPair_eMax], bool ramp)
{
    unsigned i, j;
    bool retry=false, success;
    uint8_t sourceChannel, src, playback;

    BDBG_OBJECT_ASSERT(handle, BAPE_Device);
    BDBG_ASSERT(NULL != sourceChannels);
    BDBG_ASSERT(NULL != srcs);
    BDBG_ASSERT(NULL != playbacks);

    /* Must find enough consecutive resources to fulfill the request, starting at the top instead of the bottom of the pools. */
retry:
    sourceChannel = src = playback = 0xff;
    for ( i = BAPE_CHIP_MAX_SOURCE_CHANNELS-1; i >= numChannelPairs; i-- )
    {
        success = true;
        for ( j = 0; j < numChannelPairs; j++ )
        {
            BDBG_MSG(("SourceChannelAllocated[%u] = %u", i-j, handle->sourceChannelAllocated[i-j]));
            if ( handle->sourceChannelAllocated[i-j] )
            {
                success = false;
                break;
            }
        }
        if ( success )
        {
            sourceChannel = i-(numChannelPairs-1);
            break;
        }
    }
    for ( i = BAPE_CHIP_MAX_SRCS-1; i >= numChannelPairs; i-- )
    {
        success = true;
        for ( j = 0; j < numChannelPairs; j++ )
        {
            if ( handle->srcAllocated[i-j] )
            {
                success = false;
                break;
            }
        }
        if ( success )
        {
            src = i-(numChannelPairs-1);
            break;
        }
    }
    for ( i = BAPE_CHIP_MAX_MIXER_PLAYBACKS-1; i >= numChannelPairs; i-- )
    {
        success = true;
        for ( j = 0; j < numChannelPairs; j++ )
        {
            if ( handle->playbackAllocated[i] )
            {
                success = false;
                break;
            }
        }
        if ( success )
        {
            playback = i-(numChannelPairs-1);
            break;
        }
    }

    if ( sourceChannel == 0xff || src == 0xff || playback == 0xff )
    {
        if ( !retry )
        {
            retry = true;
            BDBG_MSG(("Unable to allocate source channel/src/playback (group size %u).  Attempting garbage collection.", numChannelPairs));
            BAPE_P_GarbageCollect(handle);
            goto retry;
        }
        else
        {
            BDBG_ERR(("Unable to allocate source channel/src/playback (group size %u).", numChannelPairs));
            BAPE_P_GarbageCollect(handle);
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }

    /* If we get here, we have the resources.  Program them. */
    for ( j = 0; j < numChannelPairs; j++ )
    {
        /* Set playback FCI ID to SRC output */
        BDBG_MSG(("Linking playback %u to SRC %u output (group size %u)", playback+j, src+j, numChannelPairs));
        BAPE_P_SetPlaybackLinkage(handle, playback+j, (0x2<<6)|((uint32_t)src+j));
        BDBG_MSG(("Linking Source Channel %u to SRC %u (group size %u)", sourceChannel+j, src+j, numChannelPairs));
        BAPE_P_SetSrcLinkage(handle, src+j, sourceChannel+j, src, ramp);
        BAPE_P_SetSourceChannelGroup(handle, sourceChannel+j, sourceChannel);
        sourceChannels[j] = sourceChannel+j;
        srcs[j] = src + j;
        playbacks[j] = playback+j;
        handle->sourceChannelAllocated[sourceChannel+j] = true;
        handle->srcAllocated[src+j] = true;
        handle->playbackAllocated[playback+j] = true;
    }

    return BERR_SUCCESS;
}

static void BAPE_P_GarbageCollect(BAPE_Handle handle)
{
#if 0
    unsigned i;
#endif

    BDBG_OBJECT_ASSERT(handle, BAPE_Device);

#if 0
    /* This logic is not correct.  The proper logic must take into consideration the entire filter graph and
       not steal resources from other mixers that are part of the currently starting source(s).  This logic can
       steal resources from other mixers that are starting at the same time as this one and eventually cause
       them to fail later in the mixer start routine.  */
    for ( i = 0; i < BAPE_CHIP_MAX_MIXERS; i++ )
    {
        /* Look for any idle mixers that have resources allocated.  This will not affect the mixer we're currently
           allocating for because we check for ready first and the mixer allocation routine(s) set ready only
           after all resources have been allocated. */
        if ( handle->mixers[i] && handle->mixers[i]->ready && !handle->mixers[i]->running )
        {
            BDBG_MSG(("Attempting to reclaim resources from mixer %p (%u)", handle->mixers[i], i));
            BAPE_Mixer_P_ReleaseResources(handle->mixers[i]);
        }
    }
#endif
}

static void BAPE_P_SetSrcLinkage(BAPE_Handle handle, uint32_t srcId, uint32_t fciId, uint32_t groupId, bool rampEnabled)
{
    uint32_t regAddr, regVal;

    BDBG_OBJECT_ASSERT(handle, BAPE_Device);
    BDBG_ASSERT(srcId < BAPE_CHIP_MAX_SRCS);
    BDBG_ASSERT(groupId < BAPE_CHIP_MAX_SRCS);

    /* Program the SRC FCI ID first (registers are paired) */
    regAddr = BCHP_AUD_FMM_SRC_CTRL0_IDMAP10 + ((srcId/2)*(BCHP_AUD_FMM_SRC_CTRL0_IDMAP32-BCHP_AUD_FMM_SRC_CTRL0_IDMAP10));
    regVal = BREG_Read32(handle->regHandle, regAddr);
    if ( srcId & 1 )
    {
        /* Odd */
        regVal &= ~BCHP_MASK(AUD_FMM_SRC_CTRL0_IDMAP10, STREAM1);
        regVal |= BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_IDMAP10, STREAM1, fciId);
    }
    else
    {
        /* Even */
        regVal &= ~BCHP_MASK(AUD_FMM_SRC_CTRL0_IDMAP10, STREAM0);
        regVal |= BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_IDMAP10, STREAM0, fciId);
    }
    BREG_Write32(handle->regHandle, regAddr, regVal);

    /* Program the SRC group next */
    regAddr = BCHP_AUD_FMM_SRC_CTRL0_STRM_CFGi_ARRAY_BASE + (4*srcId);
    regVal = BREG_Read32(handle->regHandle, regAddr);
    regVal &= ~(BCHP_MASK(AUD_FMM_SRC_CTRL0_STRM_CFGi, GROUP)|
                BCHP_MASK(AUD_FMM_SRC_CTRL0_STRM_CFGi, RAMP_ENA)|
                BCHP_MASK(AUD_FMM_SRC_CTRL0_STRM_CFGi, STARTUP_RAMP_ENA));    
    regVal |= BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_STRM_CFGi, GROUP, groupId);
    if ( rampEnabled )
    {
        regVal |= BCHP_FIELD_ENUM(AUD_FMM_SRC_CTRL0_STRM_CFGi, RAMP_ENA, Enable);
        regVal |= BCHP_FIELD_ENUM(AUD_FMM_SRC_CTRL0_STRM_CFGi, STARTUP_RAMP_ENA, Enable);
    }
    else
    {
        regVal |= BCHP_FIELD_ENUM(AUD_FMM_SRC_CTRL0_STRM_CFGi, RAMP_ENA, Disable);
        regVal |= BCHP_FIELD_ENUM(AUD_FMM_SRC_CTRL0_STRM_CFGi, STARTUP_RAMP_ENA, Disable);
    }
    BREG_Write32(handle->regHandle, regAddr, regVal);
}

static void BAPE_P_SetSourceChannelGroup(BAPE_Handle handle, uint32_t sourceChannelId, uint32_t groupId)
{
    uint32_t regAddr, regVal;

    BDBG_OBJECT_ASSERT(handle, BAPE_Device);
    BDBG_ASSERT(sourceChannelId < BAPE_CHIP_MAX_SOURCE_CHANNELS);
    BDBG_ASSERT(groupId < BAPE_CHIP_MAX_SOURCE_CHANNELS);

    regAddr = BCHP_AUD_FMM_BF_CTRL_SOURCECH_GRPi_ARRAY_BASE + (4*sourceChannelId);
    regVal = BREG_Read32(handle->regHandle, regAddr);
    regVal &= ~BCHP_MASK(AUD_FMM_BF_CTRL_SOURCECH_GRPi, GROUP_ID);
    regVal |= BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_SOURCECH_GRPi, GROUP_ID, groupId);
    BREG_Write32(handle->regHandle, regAddr, regVal);
}

BERR_Code BAPE_P_AllocateInputBuffers(BAPE_Handle handle, BAPE_MixerInput input)
{
    unsigned i;

    BDBG_OBJECT_ASSERT(handle, BAPE_Device);
    BDBG_OBJECT_ASSERT(input, BAPE_MixerInput);
    BDBG_ASSERT(input->useBufferPool);

    /* Only alloc if we have connected mixers */
    if ( NULL == BLST_S_FIRST(&input->mixerList) )
    {
        return BERR_SUCCESS;
    }

    for ( i = 0; i < input->numChannelPairs; i++ )
    {
        BDBG_ASSERT(NULL == input->pBuffers[i]);
        if ( input->compressed )
        {
            input->pBuffers[i] = BLST_S_FIRST(&handle->compressedBufferList);
            if ( NULL == input->pBuffers[i] )
            {
                BDBG_ERR(("Insufficient Compressed Buffers available."));
                return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
            }
            BLST_S_REMOVE_HEAD(&handle->compressedBufferList, node);
        }
        else
        {
            input->pBuffers[i] = BLST_S_FIRST(&handle->pcmBufferList);
            if ( NULL == input->pBuffers[i] )
            {
                BDBG_ERR(("Insufficient PCM Buffers available."));
                return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
            }
            BLST_S_REMOVE_HEAD(&handle->pcmBufferList, node);
        }
    }

    return BERR_SUCCESS;
}

void BAPE_P_FreeInputBuffers(BAPE_Handle handle, BAPE_MixerInput input)
{
    unsigned i;

    BDBG_OBJECT_ASSERT(handle, BAPE_Device);
    BDBG_OBJECT_ASSERT(input, BAPE_MixerInput);
    BDBG_ASSERT(input->useBufferPool);

    for ( i = 0; i < input->numChannelPairs; i++ )
    {
        if ( input->pBuffers[i] )
        {
            if ( input->compressed )
            {
                BLST_S_INSERT_HEAD(&handle->compressedBufferList, input->pBuffers[i], node);
            }
            else
            {
                BLST_S_INSERT_HEAD(&handle->pcmBufferList, input->pBuffers[i], node);
            }
            input->pBuffers[i] = NULL;
        }
    }
}

