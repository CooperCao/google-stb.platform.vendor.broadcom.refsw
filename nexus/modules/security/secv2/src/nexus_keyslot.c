/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

 ******************************************************************************/

#include "bstd.h"
#include "nexus_security_module.h"
#include "nexus_base.h"
#include "nexus_types.h"
#include "nexus_keyslot.h"
#include "nexus_security_common.h"
#include "priv/nexus_security_priv.h"
#include "bhsm.h"
#include "bhsm_keyslot.h"
#include "priv/nexus_pid_channel_priv.h"
#include "nexus_keyslot_priv.h"

BDBG_MODULE(nexus_keyslot);

typedef struct keySlotInstance_t{
    NEXUS_OBJECT(keySlotInstance_t);

    bool configured;
    NEXUS_KeySlotSettings settings;

    struct
    {
        bool configured;
        NEXUS_KeySlotEntrySettings settings;
    }entry[NEXUS_KeySlotBlockEntry_eMax];

    NEXUS_PidChannelHandle dmaPidChannelHandle;

    BHSM_KeyslotHandle hsmKeyslotHandle;

}keySlotInstance_t;


static BHSM_KeyslotType _convertSlotTypeToHsm(NEXUS_KeySlotType type);

/* called on platform initialisation from NEXUS_SecurityModule_Init */
NEXUS_Error NEXUS_KeySlot_Init( void )
{
    return NEXUS_SUCCESS;
}

/* called on platform shutdown from NEXUS_SecurityModule_Uninit */
void NEXUS_KeySlot_Uninit( void )
{
    NEXUS_SecurityModule_Sweep_priv();
    return;
}

void NEXUS_KeySlot_GetDefaultAllocateSettings( NEXUS_KeySlotAllocateSettings *pSettings )
{
    BDBG_ENTER(NEXUS_KeySlot_GetDefaultAllocateSettings);

    if( !pSettings ) { BERR_TRACE(NEXUS_INVALID_PARAMETER); return; }

    BKNI_Memset( pSettings, 0, sizeof(*pSettings) );
    pSettings->owner = NEXUS_SecurityCpuContext_eHost;
    pSettings->slotType = NEXUS_KeySlotType_eIvPerSlot;

    BDBG_LEAVE(NEXUS_KeySlot_GetDefaultAllocateSettings);
    return;
}

/*
    Allocate a Keyslot. NULL if none is available
*/
NEXUS_KeySlotHandle NEXUS_KeySlot_Allocate( const NEXUS_KeySlotAllocateSettings *pSettings )
{
    NEXUS_KeySlotHandle handle = NULL;
    keySlotInstance_t *pKeySlot = NULL;
    BHSM_KeyslotAllocateSettings hsmKeyslotSettings;
    BHSM_Handle hHsm;

    NEXUS_SecurityModule_Sweep_priv();

    pKeySlot = BKNI_Malloc( sizeof(*pKeySlot) );
    if( !pKeySlot ) { BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY); return NULL; }

    NEXUS_Security_GetHsm_priv( &hHsm );

    BKNI_Memset( pKeySlot, 0, sizeof(*pKeySlot) );

    handle = NEXUS_KeySlot_Create();
    if( !handle ){ BERR_TRACE(NEXUS_NOT_AVAILABLE); goto error; }

    handle->deferDestroy = true;

    BHSM_Keyslot_GetDefaultAllocateSettings( &hsmKeyslotSettings );
    BKNI_Memset( &hsmKeyslotSettings, 0, sizeof(hsmKeyslotSettings) );
    hsmKeyslotSettings.owner        = pSettings->owner;
    hsmKeyslotSettings.slotType     = _convertSlotTypeToHsm( pSettings->slotType );
    hsmKeyslotSettings.useWithDma   = pSettings->useWithDma;

    pKeySlot->hsmKeyslotHandle = BHSM_Keyslot_Allocate( hHsm, &hsmKeyslotSettings );
    if( !pKeySlot->hsmKeyslotHandle ) { BERR_TRACE(NEXUS_INVALID_PARAMETER); goto error; }

    if( pSettings->useWithDma )
    {
        BERR_Code rcHsm;
        unsigned pidChannelIndex;

        NEXUS_Security_LockTransport( true );
        pKeySlot->dmaPidChannelHandle = NEXUS_PidChannel_OpenDma_Priv( 0, NULL );
        NEXUS_Security_LockTransport( false );
        if( !pKeySlot->dmaPidChannelHandle ) { BERR_TRACE(NEXUS_NOT_AVAILABLE); goto error; }

        pidChannelIndex = NEXUS_PidChannel_GetIndex_isrsafe( pKeySlot->dmaPidChannelHandle );

        handle->dma.pidChannelIndex = pidChannelIndex;
        handle->dma.valid = true;

        rcHsm = BHSM_Keyslot_AddPidChannel( pKeySlot->hsmKeyslotHandle, pidChannelIndex );
        if( rcHsm != BERR_SUCCESS ) { BERR_TRACE( rcHsm ); goto error; }
    }

    handle->security.data = (void*)pKeySlot;
    handle->settings.keySlotEngine = NEXUS_SecurityEngine_eM2m;

    return handle;

error:

    if( pKeySlot->dmaPidChannelHandle ) {
        NEXUS_PidChannel_CloseDma_Priv( pKeySlot->dmaPidChannelHandle );
        handle->dma.valid = false;
    }
    if( pKeySlot->hsmKeyslotHandle ) { BHSM_Keyslot_Free( pKeySlot->hsmKeyslotHandle ); }
    BKNI_Free( pKeySlot );

    if( handle ) NEXUS_KeySlot_Destroy( handle );

    return NULL;
}


void NEXUS_KeySlot_Free_priv( NEXUS_KeySlotHandle handle )
{
    keySlotInstance_t *pKeySlot = (keySlotInstance_t*)handle->security.data;
    BDBG_ENTER(NEXUS_KeySlot_Free_priv);

    if( pKeySlot->dmaPidChannelHandle ){
        NEXUS_Security_LockTransport( true );
        NEXUS_PidChannel_CloseDma_Priv( pKeySlot->dmaPidChannelHandle );
        NEXUS_Security_LockTransport( false );
        handle->dma.valid = false;
    }

    BHSM_Keyslot_Free( pKeySlot->hsmKeyslotHandle );
    BKNI_Free( pKeySlot );

    handle->security.data = NULL;

    BDBG_LEAVE(NEXUS_KeySlot_Free_priv);
    return;
}




/*
    Free a Keyslot
*/
void NEXUS_KeySlot_Free( NEXUS_KeySlotHandle handle )
{
    BDBG_ENTER(NEXUS_KeySlot_Free);

    NEXUS_KeySlot_Free_priv( handle );

    handle->deferDestroy = false;
    NEXUS_KeySlot_Destroy( handle );

    BDBG_LEAVE(NEXUS_KeySlot_Free);
    return;
}


void NEXUS_KeySlot_GetSettings( NEXUS_KeySlotHandle handle, NEXUS_KeySlotSettings *pSettings )
{
    keySlotInstance_t *pKeySlot = (keySlotInstance_t*)handle->security.data;

    if(!pSettings) { BERR_TRACE(NEXUS_INVALID_PARAMETER); return; }
    BKNI_Memset( pSettings, 0, sizeof(*pSettings) );

    if( pKeySlot->configured ) {
        *pSettings = pKeySlot->settings;
        return;
    }

    /* Default settings. */
    pSettings->regions.source[NEXUS_SecurityRegion_eGlr] = true;
    pSettings->regions.destinationRPipe[NEXUS_SecurityRegion_eGlr] = true;
    pSettings->regions.destinationGPipe[NEXUS_SecurityRegion_eGlr] = true;

    return;
}

/*
    Configure keyslot parameters.
*/
NEXUS_Error NEXUS_KeySlot_SetSettings( NEXUS_KeySlotHandle handle, const NEXUS_KeySlotSettings *pSettings )
{
    keySlotInstance_t *pKeySlot = (keySlotInstance_t*)handle->security.data;
    BHSM_KeyslotSettings hsmKeyslotSettings;
    BERR_Code rcHsm;
    unsigned i;

    if(!pSettings) { return BERR_TRACE(NEXUS_INVALID_PARAMETER); }

    BKNI_Memset( &hsmKeyslotSettings, 0, sizeof(hsmKeyslotSettings) );

    for( i = 0; i < BHSM_SecurityRegion_eMax; i++ )
    {
        hsmKeyslotSettings.regions.source[i]           =  pSettings->regions.source[i];
        hsmKeyslotSettings.regions.destinationRPipe[i] =  pSettings->regions.destinationRPipe[i];
        hsmKeyslotSettings.regions.destinationGPipe[i] =  pSettings->regions.destinationGPipe[i];

        /* BDBG_LOG(("source[%d]", hsmKeyslotSettings.regions.source[i] )); */
        /* BDBG_LOG(("Rpipe[%d] ", hsmKeyslotSettings.regions.destinationRPipe[i] ));*/
        /* BDBG_LOG(("source[%d]", hsmKeyslotSettings.regions.destinationGPipe[i] )); */
    }
    hsmKeyslotSettings.encryptBeforeRave = pSettings->encryptBeforeRave;

    rcHsm = BHSM_Keyslot_SetSettings( pKeySlot->hsmKeyslotHandle, &hsmKeyslotSettings );
    if( rcHsm != BERR_SUCCESS ){ return BERR_TRACE(rcHsm); }

    pKeySlot->settings = *pSettings;
    pKeySlot->configured = true;

    return NEXUS_SUCCESS;
}


void NEXUS_KeySlot_GetEntrySettings( NEXUS_KeySlotHandle handle,
                                     NEXUS_KeySlotBlockEntry entry,         /* block (cps/ca/cpd) and entry (odd/even/clear) */
                                     NEXUS_KeySlotEntrySettings *pSettings )
{
    keySlotInstance_t *pKeySlot = (keySlotInstance_t*)handle->security.data;

    if(!pSettings) { BERR_TRACE(NEXUS_INVALID_PARAMETER); return; }
    if( entry >= NEXUS_KeySlotBlockEntry_eMax ) {  BERR_TRACE(NEXUS_INVALID_PARAMETER); return; }

    if( pKeySlot->entry[entry].configured )
    {
        *pSettings = pKeySlot->entry[entry].settings;
        return;
    }

    /* default */
    BKNI_Memset( pSettings, 0, sizeof(*pSettings) );
    pSettings->rPipeEnable = true;
    pSettings->gPipeEnable = true;

    return;
}


/*
    Configure one entry of a keyslot.
*/
NEXUS_Error NEXUS_KeySlot_SetEntrySettings( NEXUS_KeySlotHandle handle,
                                            NEXUS_KeySlotBlockEntry entry,
                                            const NEXUS_KeySlotEntrySettings *pSettings )
{
    keySlotInstance_t *pKeySlot = (keySlotInstance_t*)handle->security.data;
    BHSM_KeyslotEntrySettings hsmEntrySettings;
    BERR_Code rcHsm;

    if( entry >= NEXUS_KeySlotBlockEntry_eMax ) { return BERR_TRACE(NEXUS_INVALID_PARAMETER); }
    if( !pSettings ) { return BERR_TRACE(NEXUS_INVALID_PARAMETER); }

    BKNI_Memset( &hsmEntrySettings, 0, sizeof(hsmEntrySettings) );

    hsmEntrySettings.algorithm         = pSettings->algorithm;
    hsmEntrySettings.algorithmMode     = pSettings->algorithmMode;
    hsmEntrySettings.counterSize       = pSettings->counterSize;
    hsmEntrySettings.terminationMode   = pSettings->terminationMode;
    hsmEntrySettings.solitaryMode      = pSettings->solitaryMode;
    hsmEntrySettings.external.key      = pSettings->external.key;
    hsmEntrySettings.external.iv       = pSettings->external.iv;
    hsmEntrySettings.pesMode           = pSettings->pesMode;
    hsmEntrySettings.counterMode       = pSettings->counterMode;
    hsmEntrySettings.outputPolarity.specify = pSettings->outputPolarity.specify;
    hsmEntrySettings.outputPolarity.rPipe   = pSettings->outputPolarity.rPipe;
    hsmEntrySettings.outputPolarity.gPipe   = pSettings->outputPolarity.gPipe;
    hsmEntrySettings.rPipeEnable = pSettings->rPipeEnable;
    hsmEntrySettings.gPipeEnable = pSettings->gPipeEnable;
    hsmEntrySettings.multi2.keySelect = pSettings->multi2.keySelect;
    hsmEntrySettings.multi2.roundCount = pSettings->multi2.roundCount;

    rcHsm = BHSM_Keyslot_SetEntrySettings( pKeySlot->hsmKeyslotHandle, entry, &hsmEntrySettings );
    if( rcHsm != BERR_SUCCESS ){ return BERR_TRACE(rcHsm); }

    pKeySlot->entry[entry].settings = *pSettings;
    pKeySlot->entry[entry].configured = true;

    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_KeySlot_GetEntryExternalKeySettings( NEXUS_KeySlotHandle handle,
                                                       NEXUS_KeySlotBlockEntry entry, /* block (cps/ca/cpd) and entry (odd/even/clear) */
                                                       NEXUS_KeySlotExternalKeyData *pData
                                                      )
{
    keySlotInstance_t *pKeySlot = (keySlotInstance_t*)handle->security.data;
    BHSM_KeyslotExternalKeyData hsmExternalKeyData;
    BERR_Code rcHsm;

    if( entry >= NEXUS_KeySlotBlockEntry_eMax ) { return BERR_TRACE(NEXUS_INVALID_PARAMETER); }
    if( !pData ) { return BERR_TRACE(NEXUS_INVALID_PARAMETER); }

    if( !pKeySlot->entry[entry].configured ) { return BERR_TRACE(NEXUS_INVALID_PARAMETER); }

    if( !pKeySlot->entry[entry].settings.external.key &&
        !pKeySlot->entry[entry].settings.external.iv ) { return BERR_TRACE(NEXUS_INVALID_PARAMETER); }

    BKNI_Memset( &hsmExternalKeyData, 0, sizeof(hsmExternalKeyData) );

    rcHsm = BHSM_Keyslot_GetEntryExternalKeySettings( pKeySlot->hsmKeyslotHandle,
                                                      entry,
                                                      &hsmExternalKeyData );
    if( rcHsm != BERR_SUCCESS ){ return BERR_TRACE(rcHsm); }

    pData->slotIndex  = hsmExternalKeyData.slotIndex;

    pData->key.valid  = hsmExternalKeyData.key.valid;
    pData->key.offset = hsmExternalKeyData.key.offset;
    pData->iv.valid   = hsmExternalKeyData.iv.valid;
    pData->iv.offset  = hsmExternalKeyData.iv.offset;

    return NEXUS_SUCCESS;
}



NEXUS_Error NEXUS_KeySlot_SetEntryKey ( NEXUS_KeySlotHandle handle,
                                      NEXUS_KeySlotBlockEntry entry,
                                      const NEXUS_KeySlotKey *pKey )
{
    keySlotInstance_t *pKeySlot = (keySlotInstance_t*)handle->security.data;
    BHSM_KeyslotKey hsmKey;
    BERR_Code rcHsm;

    if( entry >= NEXUS_KeySlotBlockEntry_eMax )  { return BERR_TRACE(NEXUS_INVALID_PARAMETER); }
    if( !pKey )                                  { return BERR_TRACE(NEXUS_INVALID_PARAMETER); }
    if( pKey->size > BHSM_KEYSLOT_MAX_KEY_SIZE ) { return BERR_TRACE(NEXUS_INVALID_PARAMETER); }

    BKNI_Memset( &hsmKey, 0, sizeof(hsmKey) );

    hsmKey.size = pKey->size;
    BKNI_Memcpy( hsmKey.key, pKey->key, pKey->size );

    rcHsm =  BHSM_Keyslot_SetEntryKey ( pKeySlot->hsmKeyslotHandle, entry, &hsmKey );
    if( rcHsm != BERR_SUCCESS ){ return BERR_TRACE(rcHsm); }

    return NEXUS_SUCCESS;
}

/*
    Write an IV into a keyslot entry.
*/
NEXUS_Error NEXUS_KeySlot_SetEntryIv ( NEXUS_KeySlotHandle handle,
                                       NEXUS_KeySlotBlockEntry entry,
                                       const NEXUS_KeySlotIv *pIv,
                                       const NEXUS_KeySlotIv *pIv2 )
{
    keySlotInstance_t *pKeySlot = (keySlotInstance_t*)handle->security.data;
    BHSM_KeyslotIv hsmIv;
    BHSM_KeyslotIv hsmIv2;
    BERR_Code rcHsm;

    if( entry >= NEXUS_KeySlotBlockEntry_eMax ) { return BERR_TRACE(NEXUS_INVALID_PARAMETER); }
    if( !pIv && !pIv2 ) { return BERR_TRACE(NEXUS_INVALID_PARAMETER); }


    BKNI_Memset( &hsmIv, 0, sizeof(hsmIv) );
    BKNI_Memset( &hsmIv2, 0, sizeof(hsmIv2) );

    if( pIv ) {
        if( pIv->size > BHSM_KEYSLOT_MAX_IV_SIZE ) { return BERR_TRACE(NEXUS_INVALID_PARAMETER); }

        hsmIv.size = pIv->size;
        BKNI_Memcpy( hsmIv.iv, pIv->iv, pIv->size );
    }

    if( pIv2 ) {
        if( pIv2->size > BHSM_KEYSLOT_MAX_IV_SIZE ) { return BERR_TRACE(NEXUS_INVALID_PARAMETER); }

        hsmIv2.size = pIv2->size;
        BKNI_Memcpy( hsmIv2.iv, pIv2->iv, pIv2->size );
        rcHsm = BHSM_Keyslot_SetEntryIv( pKeySlot->hsmKeyslotHandle, entry, pIv ? &hsmIv : NULL, &hsmIv2 );
    }
    else
    {
        rcHsm = BHSM_Keyslot_SetEntryIv( pKeySlot->hsmKeyslotHandle, entry, &hsmIv, NULL );
    }

    if( rcHsm != BERR_SUCCESS ){ return BERR_TRACE(rcHsm); }

    return NEXUS_SUCCESS;
}



NEXUS_Error NEXUS_KeySlot_InvalidateEntry(  NEXUS_KeySlotHandle handle,
                                            NEXUS_KeySlotBlockEntry entry )
{
    keySlotInstance_t *pKeySlot = (keySlotInstance_t*)handle->security.data;
    BERR_Code rcHsm;

    if( entry >= NEXUS_KeySlotBlockEntry_eMax ) { return BERR_TRACE(NEXUS_INVALID_PARAMETER); }

    rcHsm = BHSM_Keyslot_InvalidateEntry( pKeySlot->hsmKeyslotHandle, entry );
    if( rcHsm != BERR_SUCCESS ){ return BERR_TRACE(rcHsm); }

    return NEXUS_SUCCESS;
}


NEXUS_Error NEXUS_KeySlot_Invalidate(  NEXUS_KeySlotHandle handle )
{
    keySlotInstance_t *pKeySlot = (keySlotInstance_t*)handle->security.data;
    BERR_Code rcHsm;

    rcHsm = BHSM_Keyslot_Invalidate( pKeySlot->hsmKeyslotHandle );
    if( rcHsm != BERR_SUCCESS ){ return BERR_TRACE(rcHsm); }

    return NEXUS_SUCCESS;
}


void NEXUS_KeySlot_GetDefaultAddPidChannelSettings(
    NEXUS_KeySlotAddPidChannelSettings *pSettings /* [out] */
    )
{
    if( !pSettings ) { BERR_TRACE(NEXUS_INVALID_PARAMETER); return; }
    BKNI_Memset( pSettings, 0, sizeof(*pSettings) );
    return;
}

NEXUS_Error NEXUS_KeySlot_AddPidChannelWithSettings(
    NEXUS_KeySlotHandle handle,
    NEXUS_PidChannelHandle pidChannel,
    const NEXUS_KeySlotAddPidChannelSettings *pSettings /* attr{null_allowed=y} NULL is allowed for default settings. */
    )
{
    keySlotInstance_t *pKeySlot = (keySlotInstance_t*)handle->security.data;
    BHSM_KeyslotAddPidChannelSettings hsmSettings;
    BERR_Code rcHsm;

    BKNI_Memset( &hsmSettings, 0, sizeof(hsmSettings) );

    if( pSettings ){
        hsmSettings.secondary = pSettings->secondary;
    }

    rcHsm = BHSM_Keyslot_AddPidChannel_WithSettings( pKeySlot->hsmKeyslotHandle,
                                                     NEXUS_PidChannel_GetIndex_isrsafe( pidChannel ),
                                                     &hsmSettings );
    if( rcHsm != BERR_SUCCESS ){ return BERR_TRACE(rcHsm); }

    return NEXUS_SUCCESS;
}


NEXUS_Error NEXUS_KeySlot_AddPidChannel( NEXUS_KeySlotHandle handle,
                                         NEXUS_PidChannelHandle pidChannel )
{
    NEXUS_Error rc;

    rc = NEXUS_KeySlot_AddPidChannelWithSettings( handle, pidChannel, NULL );
    if( rc != NEXUS_SUCCESS ) { return BERR_TRACE( rc ); }

    return rc;
}


NEXUS_Error NEXUS_KeySlot_RemovePidChannel(
    NEXUS_KeySlotHandle handle,
    NEXUS_PidChannelHandle pidChannel
    )
{
    keySlotInstance_t *pKeySlot = (keySlotInstance_t*)handle->security.data;
    BERR_Code rcHsm;

    rcHsm = BHSM_Keyslot_RemovePidChannel( pKeySlot->hsmKeyslotHandle,
                                           NEXUS_PidChannel_GetIndex_isrsafe( pidChannel ) );
    if( rcHsm != BERR_SUCCESS ){ return BERR_TRACE(rcHsm); }

    return NEXUS_SUCCESS;
}



NEXUS_Error NEXUS_KeySlot_GetInformation( NEXUS_KeySlotHandle handle,
                                   NEXUS_KeySlotInformation *pInfo )
{
    BERR_Code rcHsm;
    BHSM_KeyslotInfo slotInfo;
    keySlotInstance_t *pKeySlot = (keySlotInstance_t*)handle->security.data;

    if(!handle) { return BERR_TRACE(NEXUS_INVALID_PARAMETER); }
    if(!pInfo) { return BERR_TRACE(NEXUS_INVALID_PARAMETER); }

    rcHsm = BHSM_GetKeySlotInfo( pKeySlot->hsmKeyslotHandle, &slotInfo );
    if( rcHsm != BERR_SUCCESS ){ return BERR_TRACE(rcHsm); }

    pInfo->slotType = slotInfo.type;
    pInfo->slotNumber = slotInfo.number;

    return NEXUS_SUCCESS;
}



NEXUS_Error NEXUS_KeySlot_GetHsmHandle_priv( NEXUS_KeySlotHandle handle, BHSM_KeyslotHandle *pHsmKeySlotHandle )
{
    keySlotInstance_t *pKeySlot;

    if( !handle ) { return BERR_TRACE(NEXUS_INVALID_PARAMETER); }

    pKeySlot = (keySlotInstance_t*)handle->security.data;

    *pHsmKeySlotHandle = pKeySlot->hsmKeyslotHandle;

    return NEXUS_SUCCESS;
}

static BHSM_KeyslotType _convertSlotTypeToHsm(NEXUS_KeySlotType type)
{

    switch (type) {
        case NEXUS_KeySlotType_eIvPerSlot:      return BHSM_KeyslotType_eIvPerSlot;
        case NEXUS_KeySlotType_eIvPerBlock:     return BHSM_KeyslotType_eIvPerBlock;
        case NEXUS_KeySlotType_eIvPerEntry:     return BHSM_KeyslotType_eIvPerEntry;
        case NEXUS_KeySlotType_eIvPerBlock256:  return BHSM_KeyslotType_eIvPerBlock256;
        case NEXUS_KeySlotType_eIvPerEntry256:  return BHSM_KeyslotType_eIvPerEntry256;
        case NEXUS_KeySlotType_eMulti2:         return BHSM_KeyslotType_eMulti2;
        default:
            BDBG_ERR(("Not supported keyslot type %d", type));
            BERR_TRACE( BERR_INVALID_PARAMETER );
    }

    return 0;
}


void NEXUS_KeySlot_GetDefaultMulti2Key( NEXUS_KeySlotSetMulti2Key *pKeyData )
{
    if( !pKeyData ) { BERR_TRACE( BERR_INVALID_PARAMETER ); return; }

    BKNI_Memset( pKeyData, 0, sizeof(*pKeyData) );

    return;
}

NEXUS_Error NEXUS_KeySlot_SetMulti2Key( const NEXUS_KeySlotSetMulti2Key *pKeyData )
{
    BERR_Code rcHsm;
    BHSM_Handle hHsm;
    BHSM_KeySlotSetMulti2Key hsmConfig;

    NEXUS_Security_GetHsm_priv( &hHsm );

    BKNI_Memset( &hsmConfig, 0, sizeof(hsmConfig) );
    hsmConfig.keySelect = pKeyData->keySelect;
    BKNI_Memcpy( hsmConfig.key, pKeyData->key, sizeof(hsmConfig.key) );

    rcHsm = BHSM_KeySlot_SetMulti2Key( hHsm, &hsmConfig );
    if( rcHsm != BERR_SUCCESS ){ return BERR_TRACE(rcHsm); }

    return BERR_SUCCESS;
}
