/***************************************************************************
*     (c)2004-2011 Broadcom Corporation
*  
*  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
* $brcm_Workfile: $
* $brcm_Revision: $
* $brcm_Date: $
*
* API Description:
*   API name: AudioAssociation
*    API for raptor association resource management
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/
#include "nexus_audio_module.h"

BDBG_MODULE(nexus_audio_association);

/* Lifted from brap_priv.h, it would be nice if this was public */
#define MAX_OUTPUTS_PER_ASSOCIATION 8

static void NEXUS_AudioAssociation_P_PrintChannels(BRAP_AssociatedChanSettings *pSettings);

BDBG_OBJECT_ID(NEXUS_AudioAssociation);
typedef struct NEXUS_AudioAssociation
{
    BDBG_OBJECT(NEXUS_AudioAssociation)
    int usageCount;
    BRAP_AssociatedChanSettings settings;
    BRAP_AssociatedChannelHandle association;
    struct
    {
        NEXUS_AudioOutputHandle handle;
        BRAP_DestinationHandle destination;
        BRAP_DestinationHandle privateDestination;   /* for multichannel pcm hdmi,need to added to the same destination as mai */  
    } outputs[MAX_OUTPUTS_PER_ASSOCIATION];
} NEXUS_AudioAssociation;

static NEXUS_AudioAssociation g_associations[BRAP_MAX_ASSOCIATED_GROUPS];

NEXUS_AudioAssociationHandle NEXUS_AudioAssociation_P_Create(
    BRAP_AssociatedChanSettings *pSettings
    )
{
    int i;
    BERR_Code errCode;
    NEXUS_AudioAssociationHandle handle=NULL;

    #if 0
    /* PR 41825 */
    int j, k;
    #endif

    BDBG_ASSERT(NULL != pSettings);

    #if 0
    /* PR 41825: Only one association may be created per channel */
    for ( i = 0; i < BRAP_MAX_ASSOCIATED_CHANNELS_IN_GRP; i++ )             /* For each channel in this association request */
    {
        /* If any channel in this request is part of another association, share the other one */
        /* This will result in data being mixed that was not expected, but we will live with */
        /* this until the PI has been fixed */
        for ( j = 0; j < BRAP_MAX_ASSOCIATED_GROUPS; j++ )                  /* For each possible association */
        {
            /* If this associatoin has been created... */
            if ( g_associations[j].usageCount > 0 )
            {
                for ( k = 0; k < BRAP_MAX_ASSOCIATED_CHANNELS_IN_GRP; k++ ) /* for each channel in this open association */
                {
                    BRAP_ChannelHandle channel = g_associations[j].settings.sChDetail[k].hRapCh;

                    if ( NULL == channel )
                    {
                        break;
                    }
                    else if ( channel == pSettings->sChDetail[i].hRapCh )
                    {
                        BDBG_WRN(("Attempting to use the same channel (%p) in multiple associations.  Sharing association %p instead.", 
                                  channel, &g_associations[j]));
                        g_associations[j].usageCount++;
                        BDBG_MSG(("Incrementing association %d usage count to %d", j, g_associations[j].usageCount));
                        return &g_associations[j];
                    }
                }
            }
        }
    }
    #else
    /* Share duplicate requests */
    for ( i = 0; i < BRAP_MAX_ASSOCIATED_GROUPS; i++ )
    {
        if ( g_associations[i].usageCount > 0 )
        {
            if ( !BKNI_Memcmp(&g_associations[i], pSettings, sizeof(*pSettings)) )
            {
                BDBG_MSG(("Duplicate association request received.  Sharing association %p", (void *)&g_associations[i]));
                g_associations[i].usageCount++;
                BDBG_MSG(("Incrementing association %d usage count to %d", i, g_associations[i].usageCount));
                return &g_associations[i];
                          
            }
        }
    }
    #endif

    /* Find a free slot */
    for ( i = 0; i < BRAP_MAX_ASSOCIATED_GROUPS; i++ )
    {
        if ( 0 == g_associations[i].usageCount )
        {
            handle = &g_associations[i];
            break;
        }
    }

    /* If we didn't find one */
    if ( NULL == handle )
    {
        BDBG_ERR(("Out of channel associations"));
        return NULL;
    }

    /* We did find one */
    BKNI_Memset(handle, 0, sizeof(*handle));

    /* Create association in RAP */
    BDBG_MSG(("Creating association %d", i));
    NEXUS_AudioAssociation_P_PrintChannels(pSettings);
    errCode = BRAP_CreateAssociation(&handle->association, pSettings);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        return NULL;
    }
    /* Save settings */
    handle->settings = *pSettings;
    /* Set usage count */
    handle->usageCount = 1;
    /* Mark object */
    BDBG_OBJECT_SET(handle, NEXUS_AudioAssociation);

    /* Success */
    return handle;
}

void NEXUS_AudioAssociation_P_Destroy(
    NEXUS_AudioAssociationHandle handle
    )
{
    int index;

    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioAssociation);

    /* Determine which association we're freeing and sanity check */
    index = handle - &g_associations[0];
    BDBG_ASSERT(index < BRAP_MAX_ASSOCIATED_GROUPS);

    BDBG_MSG(("Destroying association %d request - usage count %d", index, handle->usageCount));

    /* If this is the last user, remove all destinations and destroy association */
    if ( 1 == handle->usageCount )
    {
        int i;

        for ( i = 0; i < MAX_OUTPUTS_PER_ASSOCIATION; i++ )
        {
            /* Make sure all destinations were closed */
            if ( handle->outputs[i].handle )
            {
                BDBG_ERR(("Destination %p was not removed from association %d [output port %d]",
                          (void *)handle->outputs[i].destination,
                          index,
                          handle->outputs[i].handle->port));
                NEXUS_AudioAssociation_P_RemoveOutput(handle, handle->outputs[i].handle);
            }
        }

        BDBG_MSG(("Destroying association %d", index));
        BRAP_DestroyAssociation(handle->association);
        BKNI_Memset(handle, 0, sizeof(*handle));    /* This also clears the debug object */
        return;
    }

    /* Decrement usage count */
    handle->usageCount--;
    BDBG_MSG(("Destroying association %d request - usage count %d", index, handle->usageCount));
}

NEXUS_Error NEXUS_AudioAssociation_P_AddOutput(
    NEXUS_AudioAssociationHandle handle,
    NEXUS_AudioInputFormat format,
    NEXUS_AudioOutputHandle output,
    bool downmix
    )
{
    int index, i;
    BERR_Code errCode;
    BRAP_DstDetails dstDetails;
    BRAP_DstDetails privDstDetails;

    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioAssociation);
    BDBG_ASSERT(NULL != output);

    /* Determine which association we're using and sanity check */
    index = handle - &g_associations[0];
    BDBG_ASSERT(index < BRAP_MAX_ASSOCIATED_GROUPS);

    /* Make sure we never try to add the same output to more than one association */
    for ( i = 0; i < BRAP_MAX_ASSOCIATED_GROUPS; i++ )
    {
        if ( g_associations[i].usageCount > 0 )
        {
            int j;
            for ( j = 0; j < MAX_OUTPUTS_PER_ASSOCIATION; j++ )
            {
                if ( g_associations[i].outputs[j].handle == output )
                {
                    BDBG_ERR(("Attempting to add output %p [output port %d] to multiple associations (adding to %d, in %d).",
                              (void *)output, output->port, index, i));
                    return BERR_TRACE(BERR_NOT_SUPPORTED);
                }
            }
        }
    }

    /* Find an empty slot */
    for ( i = 0; i < MAX_OUTPUTS_PER_ASSOCIATION; i++ )
    {
        if ( handle->outputs[i].handle == NULL )
        {
            break;
        }
    }
    if ( i >= MAX_OUTPUTS_PER_ASSOCIATION )
    {
        BDBG_ERR(("Exceeded max number of outputs per association"));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    /* Add the output to the association */
    BKNI_Memset(&dstDetails, 0, sizeof(dstDetails));
    BKNI_Memset(&privDstDetails, 0, sizeof(privDstDetails));
    if(output->objectType == NEXUS_AudioOutputType_eHdmi && format == NEXUS_AudioInputFormat_ePcm5_1)
    {
        errCode = NEXUS_AudioOutput_P_GetDestinationDetails(output, format, 
                                                            handle->settings.sChDetail[0].hRapCh, &dstDetails,&privDstDetails);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
        privDstDetails.uDstDetails.sOpDetails.bDownmixedOpPort = false;
        errCode = BRAP_AddDestination(handle->association, &privDstDetails, &handle->outputs[i].privateDestination);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
        dstDetails.uDstDetails.sOpDetails.bDownmixedOpPort = false;
        errCode = BRAP_AddDestination(handle->association, &dstDetails, &handle->outputs[i].destination);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    }
    else
    {
        errCode = NEXUS_AudioOutput_P_GetDestinationDetails(output, format, 
                                                        handle->settings.sChDetail[0].hRapCh, &dstDetails, NULL);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
        if ( BRAP_AudioDst_eOutputPort == dstDetails.eAudioDst )
        {
            if ( format != NEXUS_AudioInputFormat_ePcmStereo )
            {
                dstDetails.uDstDetails.sOpDetails.bDownmixedOpPort = false;
            }
            else
            {
                dstDetails.uDstDetails.sOpDetails.bDownmixedOpPort = downmix;
            }
        }
        errCode = BRAP_AddDestination(handle->association, &dstDetails, &handle->outputs[i].destination);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
        handle->outputs[i].privateDestination = NULL;
    }
    
    NEXUS_AudioOutput_P_SetDestination(output, handle->outputs[i].destination);

    BDBG_MSG(("Adding output %p [output port %d] to association %d", (void *)output, output->port, index));

    
    /* Mark slot as used */
    handle->outputs[i].handle = output;
    return BERR_SUCCESS;
}

void NEXUS_AudioAssociation_P_RemoveOutput(
    NEXUS_AudioAssociationHandle handle,
    NEXUS_AudioOutputHandle output
    )
{
    int index, i;

    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioAssociation);
    BDBG_ASSERT(NULL != output);

    /* Determine which association we're using and sanity check */
    index = handle - &g_associations[0];
    BDBG_ASSERT(index < BRAP_MAX_ASSOCIATED_GROUPS);

    for ( i = 0; i < MAX_OUTPUTS_PER_ASSOCIATION; i++ )
    {
        if ( handle->outputs[i].handle == output )
        {
            BDBG_MSG(("Removing destination %p from asssociation %d [output port %d]", 
                      (void *)handle->outputs[i].destination,
                      index,
                      output->port));
#if NEXUS_NUM_AUDIO_CAPTURES
            if ( output->objectType == NEXUS_AudioOutputType_eCapture )
            {
                NEXUS_AudioCapture_P_Disconnect(output->pObjectHandle);
            }
#endif
            NEXUS_AudioOutput_P_SetDestination(output, NULL);
            BRAP_RemoveDestination(handle->association, handle->outputs[i].destination);
            if ( handle->outputs[i].privateDestination )
            {
                BRAP_RemoveDestination(handle->association, handle->outputs[i].privateDestination);
            }
            handle->outputs[i].handle=NULL;
            handle->outputs[i].destination=NULL;
            handle->outputs[i].privateDestination=NULL;
            return;
        }
    }

    BDBG_ERR(("Association %d is not connected to output %p [output port %d]",
              index,
              (void *)output,
              output->port));
    BDBG_ASSERT(false);         /* Should never get here */
}

BRAP_DestinationHandle NEXUS_AudioAssociation_P_GetOutputDestination(
    NEXUS_AudioAssociationHandle handle,
    NEXUS_AudioOutputHandle output,
    BRAP_DestinationHandle *pPrivDestination
    )
{
    int i, index;

    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioAssociation);
    BDBG_ASSERT(NULL != output);

    /* Determine which association we're using and sanity check */
    index = handle - &g_associations[0];
    BDBG_ASSERT(index < BRAP_MAX_ASSOCIATED_GROUPS);

    BDBG_MSG(("Searching association %d for output %p [port=%d]", index, (void *)output, output->port));

    if ( pPrivDestination )
    {
        *pPrivDestination = NULL;
    }

    for ( i = 0; i < MAX_OUTPUTS_PER_ASSOCIATION; i++ )
    {
        if ( handle->outputs[i].handle == output )
        {

            if ( pPrivDestination )
            {
                *pPrivDestination = handle->outputs[i].privateDestination;
            }
            return handle->outputs[i].destination;
        }
    }

    return NULL;
}

static void NEXUS_AudioAssociation_P_PrintChannels(BRAP_AssociatedChanSettings *pSettings)
{
#if BDBG_DEBUG_BUILD
    int numChannels;

    BDBG_ASSERT(NULL != pSettings);

    for ( numChannels = 0; numChannels < BRAP_MAX_ASSOCIATED_CHANNELS_IN_GRP; numChannels++ )
    {
        if ( pSettings->sChDetail[numChannels].hRapCh == NULL )
        {
            break;
        }
    }

    switch ( numChannels )
    {
    default:
        BDBG_MSG(("Unexpected number of channels found (%d)", numChannels));
        return;
    case 1:
        BDBG_MSG(("Association Settings has 1 channel %p", (void *)pSettings->sChDetail[0].hRapCh));
        return;
    case 2:
        BDBG_MSG(("Association Settings has 2 channels %p,%p",
                  (void *)pSettings->sChDetail[0].hRapCh, (void *)pSettings->sChDetail[1].hRapCh));
        return;
    case 3:
        BDBG_MSG(("Association Settings has 3 channels %p,%p,%p",
                  (void *)pSettings->sChDetail[0].hRapCh, (void *)pSettings->sChDetail[1].hRapCh, (void *)pSettings->sChDetail[2].hRapCh));
        return;
    case 4:
        BDBG_MSG(("Association Settings has 4 channels %p,%p,%p,%p",
                  (void *)pSettings->sChDetail[0].hRapCh, (void *)pSettings->sChDetail[1].hRapCh,
                  (void *)pSettings->sChDetail[2].hRapCh, (void *)pSettings->sChDetail[3].hRapCh));
        return;
    }
#else
    BSTD_UNUSED(pSettings);
#endif
}

