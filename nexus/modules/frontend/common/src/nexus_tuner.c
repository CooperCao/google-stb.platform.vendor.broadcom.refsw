/***************************************************************************
*     (c)2004-2013 Broadcom Corporation
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
*   API name: Frontend Tuner
*    Generic APIs for tuner device control
*
* Revision History:
*
* $brcm_Log: $
*
***************************************************************************/

#include "nexus_frontend_module.h"

BDBG_MODULE(nexus_tuner);

extern NEXUS_TunerList g_tunerList;

NEXUS_Error NEXUS_Tuner_P_CheckDeviceOpen(NEXUS_TunerHandle handle)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_FrontendDeviceHandle deviceHandle = NULL;

    deviceHandle = handle->pGenericDeviceHandle;
    if(deviceHandle == NULL) goto done;

    rc = NEXUS_FrontendDevice_P_CheckOpen(deviceHandle);
done:
    return rc;
}

void NEXUS_Tuner_GetDefaultAcquireSettings( NEXUS_TunerAcquireSettings *pSettings )
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
}

NEXUS_TunerHandle NEXUS_Tuner_Acquire( const NEXUS_TunerAcquireSettings *pSettings )
{
    NEXUS_TunerHandle Tuner;
    unsigned index;
    for (Tuner = BLST_SQ_FIRST(&g_tunerList.tuners), index = 0; Tuner; Tuner = BLST_SQ_NEXT(Tuner, link), index++) {
        BDBG_OBJECT_ASSERT(Tuner, NEXUS_Tuner);
        if (Tuner->acquired) continue;
        if (pSettings->index == index) {
            Tuner->acquired = true;
            /*NEXUS_OBJECT_REGISTER(NEXUS_TunerConnector, Tuner->connector, Acquire);*/
            return Tuner;
        }    
    }
    return NULL;
}

static void NEXUS_Tuner_UninstallCallbacks_priv(NEXUS_TunerHandle handle)
{

    BDBG_ASSERT(NULL != handle);

    if(NEXUS_Tuner_P_CheckDeviceOpen(handle)){
        BERR_TRACE(NEXUS_NOT_INITIALIZED);
        return;
    }

    if(handle->uninstallCallbacks)
    {
        handle->uninstallCallbacks(handle->pDeviceHandle);
    }

    return;
}
    
void NEXUS_Tuner_Release( NEXUS_TunerHandle Tuner )
{
    BDBG_OBJECT_ASSERT(Tuner, NEXUS_Tuner);
    NEXUS_Tuner_UninstallCallbacks_priv(Tuner);
    NEXUS_Tuner_Untune(Tuner);
    Tuner->acquired = false;
}

/***************************************************************************
Summary:
    Set the frequency of a tuner device
 ***************************************************************************/
NEXUS_Error NEXUS_Tuner_SetFrequency(
    NEXUS_TunerHandle handle,
    NEXUS_TunerMode mode,               /* Tuner Mode */
    unsigned frequency                  /* In Hz */
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_Tuner);

    if(NEXUS_Tuner_P_CheckDeviceOpen(handle)){
        return BERR_TRACE(NEXUS_NOT_INITIALIZED);
    }

    if ( handle->setFrequency )
    {
        BDBG_MSG(("Setting tuner frequency to %d, mode %d", frequency, mode));
        return handle->setFrequency(handle->pDeviceHandle, mode, frequency);
    }
    else
    {
        if ( NULL != handle->tune )
        {
            BDBG_ERR(("Please use NEXUS_Tuner_Tune() instead of "
                "NEXUS_Tuner_SetFrequency() for this tuner device."));
        }
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

NEXUS_TunerHandle NEXUS_Tuner_OpenGeneric( void )
{
    return NULL;
}

void NEXUS_Tuner_Untune( NEXUS_TunerHandle tuner )
{
    BDBG_OBJECT_ASSERT(tuner, NEXUS_Tuner);

    if(NEXUS_Tuner_P_CheckDeviceOpen(tuner)){
        BDBG_ERR(("Device open failed. Cannot init."));
        BERR_TRACE(NEXUS_NOT_INITIALIZED);
        return;
    }

    if ( tuner->untune )
    {
        tuner->untune(tuner->pDeviceHandle);
    }
    else
    {
        BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

NEXUS_Error NEXUS_Tuner_Standby_priv( NEXUS_TunerHandle tuner, bool enabled, const NEXUS_StandbySettings *pSettings )
{
#if NEXUS_POWER_MANAGEMENT
    BDBG_OBJECT_ASSERT(tuner, NEXUS_Tuner);

    if(NEXUS_Tuner_P_CheckDeviceOpen(tuner)){
        return BERR_TRACE(NEXUS_NOT_INITIALIZED);
    }

    if ( tuner->standby )
    {
        return tuner->standby(tuner->pDeviceHandle, enabled, pSettings);
    }
    else
    {
        /* Frontend does not have a standby api. This is a valid case */
        return NEXUS_SUCCESS;
    }
#else
    BSTD_UNUSED(tuner);
    BSTD_UNUSED(enabled);
    BSTD_UNUSED(pSettings);
    return NEXUS_SUCCESS;
#endif
}

/***************************************************************************
Summary:
    Close a tuner handle
***************************************************************************/
static void NEXUS_Tuner_P_Release(NEXUS_TunerHandle handle)
{
    NEXUS_OBJECT_UNREGISTER(NEXUS_Tuner, handle, Destroy);
}

static void NEXUS_Tuner_P_Finalizer(
    NEXUS_TunerHandle handle
    )
{
    NEXUS_OBJECT_ASSERT(NEXUS_Tuner, handle);
    BDBG_ASSERT(handle->close != NULL);
    BLST_SQ_REMOVE(&g_tunerList.tuners, handle, NEXUS_Tuner, link);
    handle->close(handle->pDeviceHandle);
    NEXUS_OBJECT_DESTROY(NEXUS_Tuner, handle);
    BKNI_Free(handle);
    handle = NULL;
}

NEXUS_OBJECT_CLASS_MAKE_WITH_RELEASE(NEXUS_Tuner, NEXUS_Tuner_Close);

/***************************************************************************
 * Generic method to create a nexus tuner.  It will be automatically
 * destroyed when NEXUS_Tuner_Close is called.
 ***************************************************************************/
NEXUS_TunerHandle NEXUS_Tuner_P_Create(
    void *pDeviceHandle
    )
{
    NEXUS_Tuner *pTuner;

    pTuner = BKNI_Malloc(sizeof(NEXUS_Tuner));
    if (!pTuner) {
        BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }

    NEXUS_OBJECT_INIT(NEXUS_Tuner, pTuner);
    pTuner->pDeviceHandle = pDeviceHandle;
    /* must be append for implicit index to work */
    BLST_SQ_INSERT_TAIL(&g_tunerList.tuners, pTuner, link); 

    NEXUS_OBJECT_REGISTER(NEXUS_Tuner, pTuner, Create);
    
    return pTuner;
}

/***************************************************************************
Summary:
    Get default settings for a frontend extension
****************************************************************************/
void NEXUS_Tuner_P_GetDefaultExtensionSettings(
    NEXUS_TunerExtensionSettings *pSettings          /* [out] */
    )
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(NEXUS_TunerExtensionSettings));
}

/***************************************************************************
Summary:
    Create a nexus tuner handle from an extension

Description:
    This interface allows a custom platform to easily integrate tuner 
    functions for a tuner into the nexus framework.  Using this, the standard
    nexus frontend routines can control both your tuner and demodulator if 
    desired.
****************************************************************************/
NEXUS_TunerHandle NEXUS_Tuner_P_CreateExtension(
    const NEXUS_TunerExtensionSettings *pSettings
    )
{
    NEXUS_Tuner *pTuner;

    BDBG_ASSERT(NULL != pSettings);
    BDBG_ASSERT(NULL != pSettings->close);
    BDBG_ASSERT(NULL != pSettings->setFrequency);

    pTuner = NEXUS_Tuner_P_Create(pSettings->pDeviceHandle);
    if ( pTuner )
    {
        pTuner->ifFrequency = pSettings->ifFrequency;
        pTuner->close = pSettings->close;
        pTuner->setFrequency = pSettings->setFrequency;
    }

    return pTuner;
}

void NEXUS_Tuner_Init(
    NEXUS_TunerHandle tuner
    )
{
    BDBG_OBJECT_ASSERT(tuner, NEXUS_Tuner);

    if(NEXUS_Tuner_P_CheckDeviceOpen(tuner)){
        BDBG_ERR(("Device open failed. Cannot init tuner."));
        BERR_TRACE(NEXUS_NOT_INITIALIZED);
        return;
    }

    if ( NULL != tuner->init )
    {
        tuner->init(tuner->pDeviceHandle);
    }
    else
    {
        (void)BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

NEXUS_Error NEXUS_Tuner_RequestAsyncStatus(
    NEXUS_TunerHandle tuner
    )
{
    BDBG_OBJECT_ASSERT(tuner, NEXUS_Tuner);

    if(NEXUS_Tuner_P_CheckDeviceOpen(tuner)){
        return BERR_TRACE(NEXUS_NOT_INITIALIZED);
    }

    if ( tuner->requestAsyncStatus )
    {
        return tuner->requestAsyncStatus(tuner->pDeviceHandle);
    }
    else
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

NEXUS_Error NEXUS_Tuner_GetAsyncStatus(
    NEXUS_TunerHandle tuner,
    NEXUS_TunerStatus *pStatus  /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(tuner, NEXUS_Tuner);
    BDBG_ASSERT(NULL != pStatus);

    if(NEXUS_Tuner_P_CheckDeviceOpen(tuner)){
        return BERR_TRACE(NEXUS_NOT_INITIALIZED);
    }

    if ( tuner->getAsyncStatus )
    {
        return tuner->getAsyncStatus(tuner->pDeviceHandle, pStatus);
    }
    else
    {
        BKNI_Memset(pStatus, 0, sizeof(*pStatus));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

void NEXUS_Tuner_GetStatus(
    NEXUS_TunerHandle tuner,
    NEXUS_TunerStatus *pStatus  /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(tuner, NEXUS_Tuner);
    BDBG_ASSERT(NULL != pStatus);

    if(NEXUS_Tuner_P_CheckDeviceOpen(tuner)){
        BDBG_ERR(("Device open failed. Cannot get status."));
        BERR_TRACE(NEXUS_NOT_INITIALIZED);
        return;
    }

    if ( tuner->getStatus )
    {
        tuner->getStatus(tuner->pDeviceHandle, pStatus);
    }
    else
    {
        BKNI_Memset(pStatus, 0, sizeof(*pStatus));
        (void)BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

void NEXUS_Tuner_GetDefaultTuneSettings(
    NEXUS_TunerMode mode,
    NEXUS_TunerTuneSettings *pSettings  /* [out] */
    )
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->mode = mode;
    /* TODO: If specifics are required per-mode, set them here */
    /* TODO: If each tuner really needs its own GetDefaultTuneSettings, we need a handle passed into this function also. */
}

void *NEXUS_Tuner_P_GetAgcScript(NEXUS_TunerHandle tuner)
{
    BDBG_OBJECT_ASSERT(tuner, NEXUS_Tuner);

    if(NEXUS_Tuner_P_CheckDeviceOpen(tuner)){
        BDBG_ERR(("Device open failed. Cannot get agc script."));
        BERR_TRACE(NEXUS_NOT_INITIALIZED);
        return NULL;
    }

    if ( tuner->getAgcScript )
    {
        return tuner->getAgcScript(tuner->pDeviceHandle);
    }
    else
    {
        return NULL;
    }
}

NEXUS_Error NEXUS_Tuner_Tune(
    NEXUS_TunerHandle tuner,
    const NEXUS_TunerTuneSettings *pSettings
    )
{
    BDBG_OBJECT_ASSERT(tuner, NEXUS_Tuner);
    BDBG_ASSERT(NULL != pSettings);

    if(NEXUS_Tuner_P_CheckDeviceOpen(tuner)){
        return BERR_TRACE(NEXUS_NOT_INITIALIZED);
    }

    if ( tuner->tune )
    {
        return tuner->tune(tuner->pDeviceHandle, pSettings);
    }
    else if ( tuner->setFrequency )
    {
        return tuner->setFrequency(tuner->pDeviceHandle, pSettings->mode, pSettings->frequency);
    }
    else
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

void NEXUS_Tuner_GetSettings(
    NEXUS_TunerHandle tuner,
    NEXUS_TunerSettings *pSettings  /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(tuner, NEXUS_Tuner);
    BDBG_ASSERT(NULL != pSettings);

    if(NEXUS_Tuner_P_CheckDeviceOpen(tuner)){
        BDBG_ERR(("Device open failed. Cannot get settings."));
        BERR_TRACE(NEXUS_NOT_INITIALIZED);
        return;
    }

    if ( tuner->getSettings )
    {
        tuner->getSettings(tuner->pDeviceHandle, pSettings);
    }
    else
    {
        (void)BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

NEXUS_Error NEXUS_Tuner_SetSettings(
    NEXUS_TunerHandle tuner,
    const NEXUS_TunerSettings *pSettings
    )
{
    BDBG_OBJECT_ASSERT(tuner, NEXUS_Tuner);
    BDBG_ASSERT(NULL != pSettings);

    if(NEXUS_Tuner_P_CheckDeviceOpen(tuner)){
        return BERR_TRACE(NEXUS_NOT_INITIALIZED);
    }

    if ( tuner->setSettings )
    {
        return tuner->setSettings(tuner->pDeviceHandle, pSettings);
    }
    else
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

void NEXUS_Tuner_GetAttributes(
    NEXUS_TunerHandle tuner,
    NEXUS_TunerAttributes *pAttributes /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(tuner, NEXUS_Tuner);
    BDBG_ASSERT(NULL != pAttributes);


    if(NEXUS_Tuner_P_CheckDeviceOpen(tuner)){
        BDBG_ERR(("Device open failed. Cannot get attributes."));
        BERR_TRACE(NEXUS_NOT_INITIALIZED);
        return;
    }

    if ( tuner->getAttributes )
    {
        tuner->getAttributes(tuner->pDeviceHandle, pAttributes);
    }
    else
    {
        BKNI_Memset(pAttributes, 0, sizeof(*pAttributes));
        (void)BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

NEXUS_Error NEXUS_Tuner_ReadPowerLevel(
    NEXUS_TunerHandle tuner,
    int *pPowerLevel
    )
{
    BDBG_OBJECT_ASSERT(tuner, NEXUS_Tuner);
    BDBG_ASSERT(NULL != pPowerLevel);


    if(NEXUS_Tuner_P_CheckDeviceOpen(tuner)){
        return BERR_TRACE(NEXUS_NOT_INITIALIZED);
    }

    if ( tuner->readPowerLevel )
    {
        return tuner->readPowerLevel(tuner->pDeviceHandle, pPowerLevel);
    }
    else
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

#if NEXUS_TUNER_SUPPORT
static NEXUS_Error NEXUS_Tuner_P_SetFrequencyTNR(
    void *handle, 
    NEXUS_TunerMode mode, 
    unsigned frequency
    )
{
    BTNR_Handle tnrHandle = handle;
    return BTNR_SetTunerRfFreq(tnrHandle,
                               (uint32_t)frequency,
                               (BTNR_TunerMode) mode);
}

static void NEXUS_Tuner_P_CloseTNR(
    void *handle
    )
{
    BTNR_Handle tnrHandle = handle;
    BTNR_Close(tnrHandle);
}

/***************************************************************************
 * Method to create a tuner from a BTNR handle
 ***************************************************************************/
NEXUS_Tuner *NEXUS_Tuner_P_CreateFromBTNR(
    BTNR_Handle tnrHandle
    )
{
    NEXUS_Tuner *pTuner;

    pTuner = NEXUS_Tuner_P_Create(tnrHandle);
    if ( pTuner )
    {
        pTuner->pDeviceHandle = tnrHandle;
        pTuner->close = NEXUS_Tuner_P_CloseTNR;
        pTuner->setFrequency = NEXUS_Tuner_P_SetFrequencyTNR;
    }
    return pTuner;
}
#endif

