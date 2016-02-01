/***************************************************************************
 *     Copyright (c) 2006-2014, Broadcom Corporation
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
 * Module Description: Video Processing Interface for DSP
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#include "bstd.h"
#include "bkni.h"
#include "bvde.h"
#include "bvde_priv.h"
#include "bdsp_raaga.h"

#if BCHP_PWR_SUPPORT
#include "bchp_pwr.h"
#endif

BDBG_MODULE(bvde);

BDBG_OBJECT_ID(BVDE_Device);
BDBG_OBJECT_ID(BVDE_BufferNode);

void BVDE_GetDefaultSettings(
    BVDE_Settings *pSettings    /* [out] */
    )
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->maxDspTasks = BVDE_MAX_DSP_TASKS;
    pSettings->stripeWidth = 128;
}

BERR_Code BVDE_Open(
    BVDE_Handle *pHandle,   /* [out] returned handle */
    BCHP_Handle chpHandle,
    BREG_Handle regHandle,
    BMMA_Heap_Handle mmaHandle,
    BINT_Handle intHandle,
    BTMR_Handle tmrHandle,
    BDSP_Handle dspHandle,
    const BVDE_Settings *pSettings  /* NULL will use default settings */
    )
{
    BVDE_Settings defaultSettings;
    BDSP_ContextCreateSettings dspContextSettings;
    BVDE_Handle handle;
    BERR_Code errCode;

    /* BDBG_OBJECT_ASSERT(dspHandle, BDSP_Raaga); */

    BDBG_ASSERT(NULL != pHandle);
    BDBG_ASSERT(NULL != chpHandle);
    BDBG_ASSERT(NULL != regHandle);
    BDBG_ASSERT(NULL != mmaHandle);
    BDBG_ASSERT(NULL != intHandle);
    BDBG_ASSERT(NULL != tmrHandle);
    BDBG_ASSERT(NULL != dspHandle);
    
    if ( NULL == pSettings )
    {
        BDBG_WRN (("pSettings is NULL. Using Defaults"));
        BVDE_GetDefaultSettings(&defaultSettings);
        pSettings = &defaultSettings;
    }

    /* Allocate device structure */
    handle = BKNI_Malloc(sizeof(BVDE_Device));
    if ( NULL == handle )
    {
        errCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto err_handle;
    }
    /* Initialize structure */    
    BKNI_Memset(handle, 0, sizeof(BVDE_Device));
    BDBG_OBJECT_SET(handle, BVDE_Device);
    handle->chpHandle = chpHandle;
    handle->regHandle = regHandle;
    handle->mmaHandle = mmaHandle;
    handle->intHandle = intHandle;
    handle->tmrHandle = tmrHandle;
    handle->dspHandle = dspHandle;
    handle->settings = *pSettings;

    /* Create DSP Context */
    BDSP_Context_GetDefaultCreateSettings(handle->dspHandle, BDSP_ContextType_eVideo, &dspContextSettings);
    dspContextSettings.maxTasks = pSettings->maxDspTasks;
    errCode = BDSP_Context_Create(handle->dspHandle, &dspContextSettings, &handle->dspContext);
    if ( errCode )
    {
        (void)BERR_TRACE(errCode);
        goto err_context;
    }
    
    /* Success */
    *pHandle = handle;

    return BERR_SUCCESS;

err_context:
    BDBG_OBJECT_DESTROY(handle, BVDE_Device);
    BKNI_Free(handle);
err_handle:
    *pHandle = NULL;

    return errCode;
}

void BVDE_Close(
    BVDE_Handle handle
    )
{
    unsigned i=0;

    BDBG_OBJECT_ASSERT(handle, BVDE_Device);

    /* Stop all potential mixer inputs first */
#if BVDE_MAX_CHANNELS > 0
    for ( i = 0; i < BVDE_MAX_CHANNELS; i++ )
    {
        if ( handle->channels[i] )
        {
            BDBG_MSG(("Stopping channel %p (%d)", handle->channels[i], i));
            BVDE_Channel_Stop(handle->channels[i]);
        }
    }
#endif

    if ( handle->dspContext )
    {
        BDSP_Context_Destroy(handle->dspContext);
        handle->dspContext = NULL;
    }

    BDBG_OBJECT_DESTROY(handle, BVDE_Device);
    BKNI_Free(handle);
}

void BVDE_GetInterruptHandlers(
    BVDE_Handle handle,
    BVDE_InterruptHandlers *pInterrupts     /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(handle, BVDE_Device);
    BDBG_ASSERT(NULL != pInterrupts);
    *pInterrupts = handle->interrupts;
}

BERR_Code BVDE_SetInterruptHandlers(
    BVDE_Handle handle,
    const BVDE_InterruptHandlers *pInterrupts
    )
{
    BDBG_OBJECT_ASSERT(handle, BVDE_Device);
    BDBG_ASSERT(NULL != pInterrupts);

    if ( NULL != handle->dspContext )
    {
        BERR_Code errCode;
        BDSP_ContextInterruptHandlers contextInterrupts;

        BDSP_Context_GetInterruptHandlers(handle->dspContext, &contextInterrupts);
        contextInterrupts.watchdog.pCallback_isr = pInterrupts->watchdog.pCallback_isr;
        contextInterrupts.watchdog.pParam1 = pInterrupts->watchdog.pParam1;
        contextInterrupts.watchdog.param2 = pInterrupts->watchdog.param2;
        errCode = BDSP_Context_SetInterruptHandlers(handle->dspContext, &contextInterrupts);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    }

    handle->interrupts = *pInterrupts;

    return BERR_SUCCESS;
}

BERR_Code BVDE_ProcessWatchdogInterrupt(
    BVDE_Handle handle
    )
{
    BERR_Code errCode;
    unsigned i;

    BDBG_OBJECT_ASSERT(handle, BVDE_Device);

    if ( handle->dspContext )
    {
        /* Stop all running channels */
        for ( i = 0; i < BVDE_MAX_CHANNELS; i++ )
        {
            if ( handle->channels[i] )
            {
                handle->channelWatchdogInfo[i].state = handle->channels[i]->state;
                if ( handle->channelWatchdogInfo[i].state != BVDE_ChannelState_eStopped )
                {
                    handle->channelWatchdogInfo[i].startSettings = handle->channels[i]->startSettings;
                    BVDE_Channel_Stop(handle->channels[i]);
                }
            }
            else
            {
                handle->channelWatchdogInfo[i].state = BVDE_ChannelState_eMax;
            }
        }

        /* Reboot the DSP */
        errCode = BDSP_Context_ProcessWatchdogInterrupt(handle->dspContext);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }

        /* Reset all channel state */
        for ( i = 0; i < BVDE_MAX_CHANNELS; i++ )
        {
            if ( handle->channels[i] )
            {
                if ( handle->channelWatchdogInfo[i].state != BVDE_ChannelState_eStopped )
                {
                    /* Restart channel */
                    errCode = BVDE_Channel_Start(handle->channels[i], &handle->channelWatchdogInfo[i].startSettings);
                    if ( errCode )
                    {
                        BDBG_ERR(("Error restarting channel %d", i));
                        errCode = BERR_TRACE(errCode);
                    }
                }
            }
        }
    }

    return BERR_SUCCESS;
}
