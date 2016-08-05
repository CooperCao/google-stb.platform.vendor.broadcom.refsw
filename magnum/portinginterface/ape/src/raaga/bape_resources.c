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
 * Module Description: Audio Decoder Interface
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

/*************************************************************************** 
  The resource pools are managed in a simple manner to permit grouping and
  attempt to avoid fragmentation of resources.  For any given resource,
  if only one channel pair is required (not grouped), resources are allocated
  from the bottom of the pool (0..n).  For grouped resources, they are
  allocated from the top of the pool (n..0).  
***************************************************************************/

void BAPE_P_ReleaseUnusedPathResources(BAPE_Handle handle)
{
    unsigned i;

#if BAPE_CHIP_MAX_DECODERS > 0
    for ( i = 0; i < BAPE_CHIP_MAX_DECODERS; i++ )
    {
        if ( NULL != handle->decoders[i] )
        {
            BAPE_PathNode_P_ReleasePathResources(&handle->decoders[i]->node);
        }
    }
#endif
#if BAPE_CHIP_MAX_INPUT_CAPTURES > 0
    for ( i = 0; i < BAPE_CHIP_MAX_INPUT_CAPTURES; i++ )
    {
        if ( NULL != handle->inputCaptures[i] )
        {
            BAPE_PathNode_P_ReleasePathResources(&handle->inputCaptures[i]->node);
        }
    }
#endif
#if BAPE_CHIP_MAX_PLAYBACKS > 0
    for ( i = 0; i < BAPE_CHIP_MAX_PLAYBACKS; i++ )
    {
        if ( NULL != handle->playbacks[i] )
        {
            BAPE_PathNode_P_ReleasePathResources(&handle->playbacks[i]->node);
        }
    }
#endif
}

BERR_Code BAPE_P_AllocateInputBuffers(BAPE_Handle handle, BAPE_Connector input)
{
    unsigned i, numChannelPairs;
    bool needToAllocate=false;
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(handle, BAPE_Device);
    BDBG_OBJECT_ASSERT(input, BAPE_PathConnector);
    BDBG_ASSERT(input->useBufferPool);

    numChannelPairs = BAPE_FMT_P_GetNumChannelPairs_isrsafe(&input->format);

    /* Make sure we actually need buffers.  This may be called multiple times. */
    for ( i = 0; i < BAPE_ChannelPair_eMax; i++ )
    {
        if ( (i < numChannelPairs && NULL == input->pBuffers[i]) ||
             (i >= numChannelPairs && NULL != input->pBuffers[i]) )
        {
            needToAllocate=true;
            break;
        }
        else if ( input->pBuffers[i] && !BAPE_FMT_P_FormatSupported_isrsafe(&handle->buffers[input->pBuffers[i]->poolIndex].capabilities, &input->format))
        {
            needToAllocate=true;
            break;
        }
    }

    if ( needToAllocate )
    {
        BDBG_MSG(("Need to allocate buffers for %s %s", input->pParent->pName, input->pName));
        BAPE_P_FreeInputBuffers(handle, input);
    }
    else
    {
        BDBG_MSG(("Do not need to allocate buffers for %s %s", input->pParent->pName, input->pName));
        return BERR_SUCCESS;
    }

    errCode = BAPE_P_AllocateBuffers(handle, &input->format, input->pBuffers);
    if ( errCode )
    {
        return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
    }

    return BERR_SUCCESS;
}

void BAPE_P_FreeInputBuffers(BAPE_Handle handle, BAPE_Connector input)
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Device);
    BDBG_OBJECT_ASSERT(input, BAPE_PathConnector);
    BDBG_ASSERT(input->useBufferPool);

    BDBG_MSG(("Free buffers for %s %s", input->pParent->pName, input->pName));
    BAPE_P_FreeBuffers(handle, input->pBuffers);
}

BERR_Code BAPE_P_AllocateBuffers(
    BAPE_Handle deviceHandle,
    const BAPE_FMT_Descriptor *pDesc, 
    BAPE_BufferNode *pBuffers[BAPE_ChannelPair_eMax]
    )
{
    unsigned pool, i, numChannelPairs;
    unsigned attempt;

    BDBG_OBJECT_ASSERT(deviceHandle, BAPE_Device);
    BDBG_ASSERT(NULL != pDesc);
    BDBG_ASSERT(NULL != pBuffers);

    numChannelPairs = BAPE_FMT_P_GetNumChannelPairs_isrsafe(pDesc);

    /* Try twice to acquire resources and garbage collect in between if resources are unavailable */
    for ( attempt=0; attempt<2; attempt++ )
    {
        for ( pool = 0; pool < BAPE_MAX_BUFFER_POOLS; pool++ )
        {
            /* If this pool can support the format and has sufficient buffers... */
            if ( BAPE_FMT_P_FormatSupported_isrsafe(&deviceHandle->buffers[pool].capabilities, pDesc) &&
                 deviceHandle->buffers[pool].numFreeBuffers >= numChannelPairs )
            {
                /* Grab the required buffers */
                for ( i = 0; i < numChannelPairs; i++ )
                {
                    BDBG_ASSERT(NULL == pBuffers[i]);
                    pBuffers[i] = BLST_S_FIRST(&deviceHandle->buffers[pool].freeList);
                    BDBG_ASSERT(NULL != pBuffers[i]);

                    BLST_S_REMOVE_HEAD(&deviceHandle->buffers[pool].freeList, node);
                    BLST_S_INSERT_HEAD(&deviceHandle->buffers[pool].allocatedList, pBuffers[i], node);
                    deviceHandle->buffers[pool].numFreeBuffers--;
                    pBuffers[i]->allocated = true;
                    #if BDBG_DEBUG_BUILD
                    deviceHandle->buffers[pool].maxUsed = BAPE_P_MAX((deviceHandle->buffers[pool].numBuffers - deviceHandle->buffers[pool].numFreeBuffers), deviceHandle->buffers[pool].maxUsed);
                    #endif


                    BDBG_MSG(("Allocated Buffer node %p", (void *)pBuffers[i]));
                }
                return BERR_SUCCESS;
            }
        }

        /* If we reached here, the attempt failed.  Try to garbage collect. */
        BAPE_P_ReleaseUnusedPathResources(deviceHandle);
    }
    /* If we reach here, we don't have enough buffers to fulfill the request. */
    BDBG_ERR(("Insufficient buffers available to support output format %s.  Buffers can be increased in BAPE_Settings.", BAPE_FMT_P_GetTypeName_isrsafe(pDesc)));
    return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
}

void BAPE_P_FreeBuffers(
    BAPE_Handle deviceHandle,
    BAPE_BufferNode *pBuffers[BAPE_ChannelPair_eMax]
    )
{
    unsigned i, pool;

    BDBG_OBJECT_ASSERT(deviceHandle, BAPE_Device);
    BDBG_ASSERT(NULL != pBuffers);

    for ( i = 0; i < BAPE_ChannelPair_eMax; i++ )
    {
        if ( pBuffers[i] )
        {
            BDBG_MSG(("Free Buffer node %p", (void *)pBuffers[i]));
            BDBG_OBJECT_ASSERT(pBuffers[i], BAPE_BufferNode);
            pool = pBuffers[i]->poolIndex;
            if ( pool >= BAPE_MAX_BUFFER_POOLS )
            {
                BDBG_ASSERT(pool < BAPE_MAX_BUFFER_POOLS);
                BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
                return;
            }

            BLST_S_REMOVE(&deviceHandle->buffers[pool].allocatedList, pBuffers[i], BAPE_BufferNode, node);
            BLST_S_INSERT_HEAD(&deviceHandle->buffers[pool].freeList, pBuffers[i], node);
            deviceHandle->buffers[pool].numFreeBuffers++;
            pBuffers[i]->allocated = false;
            pBuffers[i] = NULL;
        }
    }
}

static BERR_Code BAPE_P_AllocateResourceGroup(BAPE_Handle handle, bool *pResourceArray, unsigned arraySize, unsigned numRequired, bool mchFromEnd, unsigned *pFirstResource)
{
    unsigned i, j, attempt;

    BDBG_OBJECT_ASSERT(handle, BAPE_Device);
    BDBG_ASSERT(NULL != pResourceArray);
    BDBG_ASSERT(arraySize > 0);
    BDBG_ASSERT(numRequired > 0);
    BDBG_ASSERT(NULL != pFirstResource);

    /* Try twice to acquire resources and garbage collect in between if resources are unavailable */
    for ( attempt=0; attempt<2; attempt++ )
    {
        /* Allocate grouped resources from the bottom of the pool and un-grouped resources from the top */
        BDBG_MSG(("resource alloc: required %d, arraySize %d, mchFromEnd %d", numRequired, arraySize, mchFromEnd));
        if ( numRequired > 1 )
        {
            unsigned base = 0xffffffff;

            /* Grouped */
            if ( mchFromEnd )
            {
                for ( i = arraySize - 1; (i + 1) >= numRequired; i-- )
                {
                    bool success = true;
                    for ( j = 0; j < numRequired; j++ )
                    {
                        if ( pResourceArray[i-j] )
                        {
                            BDBG_MSG(("  resource[%d] allocated", pResourceArray[i-j]));
                            success = false;
                            break;
                        }
                    }
                    if ( success )
                    {
                        base = i + 1 - numRequired;
                        break;
                    }
                }
            }
            else
            {
                for ( i = 0; (i + numRequired) <= arraySize; i++ )
                {
                    bool success = true;
                    for ( j = 0; j < numRequired; j++ )
                    {
                        if ( pResourceArray[i+j] )
                        {
                            BDBG_MSG(("  resource[%d] allocated", pResourceArray[i+j]));
                            success = false;
                            break;
                        }
                    }
                    if ( success )
                    {
                        BDBG_MSG(("  resource base %d", i));
                        base = i;
                        break;
                    }
                }
            }

            /* if we found space, allocate the resource */
            if ( base != 0xffffffff )
            {
                /* mark them as used */
                for ( j = 0; j < numRequired; j++ )
                {
                    pResourceArray[base+j] = true;
                }
                *pFirstResource = base;
                return BERR_SUCCESS;
            }
        }
        else
        {
            /* Ungrouped */
            for ( i = 0; i < arraySize; i++ )
            {
                if ( !pResourceArray[i] )
                {
                    /* We found an available resource.  Mark it as used. */
                    pResourceArray[i] = true;
                    *pFirstResource = i;
                    return BERR_SUCCESS;
                }
            }
        }

        /* If we reached here, the attempt failed.  Try to garbage collect. */
        BAPE_P_ReleaseUnusedPathResources(handle);
    }

    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

static BERR_Code BAPE_P_GetFmmResourceArray(BAPE_Handle handle, BAPE_FmmResourceType resourceType, bool **ppArray, unsigned *pSize, bool *mchFromEnd)
{
    bool *pResourceArray;
    unsigned arraySize;
    BERR_Code errCode = BERR_SUCCESS;
    bool multichannelFromEnd = true;

    BDBG_OBJECT_ASSERT(handle, BAPE_Device);
    BDBG_ASSERT(NULL != ppArray);
    BDBG_ASSERT(NULL != pSize);

    switch ( resourceType )
    {
    case BAPE_FmmResourceType_eSfifo:
        pResourceArray = handle->sfifoAllocated;
        arraySize = BAPE_CHIP_MAX_SFIFOS;
        break;
    case BAPE_FmmResourceType_eDfifo:
        pResourceArray = handle->dfifoAllocated;
        arraySize = BAPE_CHIP_MAX_DFIFOS;
        break;
    case BAPE_FmmResourceType_eSrc:
        pResourceArray = handle->srcAllocated;
        arraySize = BAPE_CHIP_MAX_SRCS;
        break;
    case BAPE_FmmResourceType_eMixer:
        pResourceArray = handle->mixerAllocated;
        arraySize = BAPE_CHIP_MAX_MIXERS;
        break;
    case BAPE_FmmResourceType_ePlayback:
        pResourceArray = handle->playbackAllocated;
        arraySize = BAPE_CHIP_MAX_MIXER_PLAYBACKS;
        break;
    case BAPE_FmmResourceType_eDummysink:
        pResourceArray = handle->dummysinkAllocated;
        arraySize = BAPE_CHIP_MAX_DUMMYSINKS;
        break;
    case BAPE_FmmResourceType_eLoopback:
        pResourceArray = handle->loopbackAllocated;
        arraySize = BAPE_CHIP_MAX_LOOPBACKS;
        break;  
#if BAPE_CHIP_MAX_FS > 0
    case BAPE_FmmResourceType_eFs:
        pResourceArray = handle->fsAllocated;
        arraySize = BAPE_CHIP_MAX_FS;
        break;
#endif
    case BAPE_FmmResourceType_eAdaptiveRate:
        pResourceArray = handle->adaptRateAllocated;
        arraySize = BAPE_CHIP_MAX_ADAPTRATE_CONTROLLERS;
        break;
#if BAPE_CHIP_MAX_FCI_SPLITTERS > 0
    case BAPE_FmmResourceType_eFciSplitter:
        pResourceArray = handle->fciSplitterAllocated;
        arraySize = BAPE_CHIP_MAX_FCI_SPLITTERS;
        multichannelFromEnd = false;
        break;
#endif
#if BAPE_CHIP_MAX_FCI_SPLITTER_OUTPUTS > 0
    case BAPE_FmmResourceType_eFciSplitterOutput:
        pResourceArray = handle->fciSplitterOutputAllocated;
        arraySize = BAPE_CHIP_MAX_FCI_SPLITTER_OUTPUTS;
        multichannelFromEnd = false;
        break;
#endif
    default:
        BDBG_ERR(("Unrecognized resource type %u", resourceType));
        errCode = BERR_TRACE(BERR_INVALID_PARAMETER);
        pResourceArray = NULL;
        arraySize = 0;
        break;
    }
    *ppArray = pResourceArray;
    *pSize = arraySize;
    *mchFromEnd = multichannelFromEnd;
    return errCode;
}

BERR_Code BAPE_P_AllocateFmmResource_tagged(BAPE_Handle handle, BAPE_FmmResourceType resourceType, unsigned numChannelPairs, unsigned *pFirstResource, const char *pFile, unsigned line)
{
    bool *pResourceArray=NULL;
    unsigned arraySize=0;
    BERR_Code errCode;
    bool mchFromEnd = true;

    BDBG_MSG(("Allocate resource %s:%u type %u num %u", pFile, line, resourceType, numChannelPairs));

    BDBG_OBJECT_ASSERT(handle, BAPE_Device);
    BDBG_ASSERT(numChannelPairs > 0);
    BDBG_ASSERT(NULL != pFirstResource);

    /* TODO: Add resource tracking to determine current and max resource usage since BAPE_Open() */

    errCode = BAPE_P_GetFmmResourceArray(handle, resourceType, &pResourceArray, &arraySize, &mchFromEnd);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    errCode = BAPE_P_AllocateResourceGroup(handle, pResourceArray, arraySize, numChannelPairs, mchFromEnd, pFirstResource);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    return BERR_SUCCESS;
}

static void BAPE_P_FreeResourceGroup(BAPE_Handle handle, bool *pResourceArray, unsigned arraySize, unsigned numResources, unsigned firstResource)
{
    unsigned i;
    
    BDBG_OBJECT_ASSERT(handle, BAPE_Device);
    BDBG_ASSERT(NULL != pResourceArray);
    BDBG_ASSERT(arraySize > 0);
    BDBG_ASSERT(numResources > 0);
    BDBG_ASSERT((firstResource + numResources) <= arraySize);
    
    for ( i = 0; i < numResources; i++ )
    {
        /* Ensure the resource isn't being freed twice */
        BDBG_ASSERT(pResourceArray[firstResource+i] == true);
        pResourceArray[firstResource+i] = false;
    }
}

void BAPE_P_FreeFmmResource(BAPE_Handle handle, BAPE_FmmResourceType resourceType, unsigned numChannelPairs, unsigned firstResource)
{
    bool *pResourceArray=NULL;
    unsigned arraySize=0;
    BERR_Code errCode;
    bool mchFromEnd = true;
    
    BDBG_OBJECT_ASSERT(handle, BAPE_Device);
    BDBG_ASSERT(numChannelPairs > 0);
    
    /* TODO: Add resource tracking to determine current and max resource usage since BAPE_Open() */
    
    errCode = BAPE_P_GetFmmResourceArray(handle, resourceType, &pResourceArray, &arraySize, &mchFromEnd);
    if ( errCode )
    {
        (void)BERR_TRACE(errCode);
        return;
    }
    
    BAPE_P_FreeResourceGroup(handle, pResourceArray, arraySize, numChannelPairs, firstResource);
}
