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
#include "bhsm.h"
#include "bhsm_keyslot.h"
#include "bhsm_keyslot_priv.h"
#include "bhsm_priv.h"
#include "bhsm_bsp_msg.h"
#include "bsp_types.h"
#include "bhsm_p_keyslot.h"
#include "bhsm_p_keyladder.h"

BDBG_MODULE(BHSM);

#define BHSM_MAX_KEYSLOTS               (228)/*(1024)*/   /* TODO move to headerfile  */
/* External Keys */
#define BHSM_EXTERNAL_KEYSLOT_KEY_SIZE  (2)      /* 2*64bits */
#define BHSM_EXTERNAL_KEYSLOT_SLOT_SIZE (8)      /* number of 64bit locations per slot  */
#define BHSM_EXTERNAL_KEYSLOTS_MAX      (512/BHSM_EXTERNAL_KEYSLOT_SLOT_SIZE)

#define BHSM_PRESERVE_OWNERSHIP (true)

#define BHSM_NUM_BYPASS_KEYSLOTS  (1)

typedef struct{

    bool allocated;            /* external slot is allocated to a ragular keyslot*/
    unsigned slotPtr;

    unsigned offsetIv;
    unsigned offsetKey;

}BHSM_P_ExternalKeySlot;


typedef struct
{
    bool configured;
    BHSM_KeyslotEntrySettings settings; /* keyslot entry settings. */

    BHSM_P_ExternalKeySlot *pExternalSlot;

}BHSM_P_KeyEntry;

typedef struct
{
    bool configured;
    BHSM_KeyslotType slotType;  /* The keyslot type   */
    unsigned number;            /* The keyslot number */

    BHSM_KeyslotSettings settings;              /* generic keyslot settings. */
    BHSM_P_KeyEntry entry[BHSM_KeyslotBlockEntry_eMax];

    struct BHSM_KeySlotModule *pModule; /* pointer back to module data. */

}BHSM_P_KeySlot;


typedef struct BHSM_KeySlotModule
{
    BHSM_Handle hHsm;                         /* HSM handle. */

    BHSM_P_KeySlot *pKeySlotHandles[BHSM_MAX_KEYSLOTS]; /* array of dynamically allocated keyslots */

    struct{
        unsigned offset;     /* offset into pKeySlotHandles of first slot of this type */
        unsigned maxNumber;  /* number of slots of this type. */
    }types[BHSM_KeyslotType_eMax];

    BHSM_P_ExternalKeySlot externalKeySlots[BHSM_EXTERNAL_KEYSLOTS_MAX];

    BHSM_KeyslotHandle hBypassKeyslot;

}BHSM_KeySlotModule;

static BHSM_Handle _GetHsmHandle( BHSM_KeyslotHandle handle );
static BERR_Code _SetEntryKey( BHSM_KeyslotHandle handle, BHSM_KeyslotBlockEntry entry, const BHSM_KeyslotKey *pKey );
static BERR_Code _SetEntryIv( BHSM_KeyslotHandle handle, BHSM_KeyslotBlockEntry entry, const BHSM_KeyslotIv *pIv, const BHSM_KeyslotIv* pIv2);
static BERR_Code _InvalidateSlot( BHSM_KeyslotHandle handle, bool preserveOwner );
static BERR_Code _InvalidateEntry( BHSM_KeyslotHandle handle, BHSM_KeyslotBlockEntry entry );
static BHSM_KeyslotSettings* _GetSlotSetting( BHSM_KeyslotHandle handle );
static BHSM_P_KeyEntry* _GetEntry( BHSM_KeyslotHandle handle, BHSM_KeyslotBlockEntry entry );
static BERR_Code compileControl0_GlobalHi( BHSM_KeyslotHandle handle );
static BERR_Code compileControl1_GlobalLo( BHSM_KeyslotHandle handle );
static BERR_Code compileControl2_ModeHi( BHSM_KeyslotHandle handle, BHSM_KeyslotBlockEntry entry );
static BERR_Code compileControl3_ModeLo( BHSM_KeyslotHandle handle, BHSM_KeyslotBlockEntry entry );
static uint8_t _convertSlotType( BHSM_KeyslotType type );
static BHSM_KeyslotBlockType _GetEntryBlockType( BHSM_KeyslotBlockEntry entry );
static BHSM_KeyslotPolarity _GetEntryPolarity( BHSM_KeyslotBlockEntry entry );
static BERR_Code _SetKeyslotOwnership( BHSM_KeyslotHandle handle, BHSM_SecurityCpuContext owner );
static BERR_Code _GetKeyslotOwnership( BHSM_KeyslotHandle handle, BHSM_SecurityCpuContext* pOwner );
#if BDBG_DEBUG_BUILD
static char* _KeyslotOwnerToString( BHSM_SecurityCpuContext owner );
#endif
#ifndef BHSM_BUILD_HSM_FOR_SAGE
static BERR_Code _Keyslot_Init( BHSM_Handle hHsm, BHSM_KeyslotModuleSettings *pSettings );
#else
static BERR_Code _Keyslot_LoadConfigurationFromBsp( BHSM_Handle hHsm );
#endif




BERR_Code BHSM_Keyslot_Init( BHSM_Handle hHsm, BHSM_KeyslotModuleSettings *pSettings )
{
    BERR_Code rc = BERR_SUCCESS;
    BHSM_KeySlotModule* pModule;
    unsigned i;
#ifndef BHSM_BUILD_HSM_FOR_SAGE
    unsigned count = 0;
#else
    BSTD_UNUSED(pSettings);
#endif
    BDBG_ENTER( BHSM_Keyslot_Init );

    if( !pSettings ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }
    if( hHsm->modules.pKeyslots ) { return BERR_TRACE( BERR_NOT_AVAILABLE ); } /* keyslot module already initialised. */

    pModule = (BHSM_KeySlotModule*)BKNI_Malloc( sizeof(BHSM_KeySlotModule) );
    if( !pModule ) { return BERR_TRACE( BERR_OUT_OF_SYSTEM_MEMORY ); }

    BKNI_Memset( pModule, 0, sizeof(BHSM_KeySlotModule) );

    pModule->hHsm = hHsm;
    hHsm->modules.pKeyslots = pModule;

   #ifndef BHSM_BUILD_HSM_FOR_SAGE

    /* reserve keyslot for bypass. */
    pSettings->numKeySlotsForType[BHSM_KeyslotType_eIvPerSlot] += BHSM_NUM_BYPASS_KEYSLOTS;

    rc =  _Keyslot_Init( hHsm, pSettings );
    if( rc != BERR_SUCCESS ) { BERR_TRACE(rc); goto error; }

    for( i = 0; i < BHSM_KeyslotType_eMax; i++ ){
        pModule->types[i].offset = count;
        pModule->types[i].maxNumber = pSettings->numKeySlotsForType[i];
        count += pSettings->numKeySlotsForType[i];
    }
   #else
    rc =  _Keyslot_LoadConfigurationFromBsp( hHsm );
    if( rc != BERR_SUCCESS ) { BERR_TRACE(rc); goto error; }
   #endif

    /* initialise the external key table */
    for( i = 0; i < BHSM_EXTERNAL_KEYSLOTS_MAX; i++ )
    {
        pModule->externalKeySlots[i].allocated = false;
        pModule->externalKeySlots[i].slotPtr   = (i * BHSM_EXTERNAL_KEYSLOT_SLOT_SIZE);
        pModule->externalKeySlots[i].offsetIv  = 0;
        pModule->externalKeySlots[i].offsetKey = BHSM_EXTERNAL_KEYSLOT_KEY_SIZE;
    }

    rc = BHSM_InitialiseBypassKeyslots( hHsm );
    if( rc != BERR_SUCCESS ) { BERR_TRACE(rc); goto error; }

    BDBG_LEAVE( BHSM_Keyslot_Init );
    return BERR_SUCCESS;
error:

    if( hHsm->modules.pKeyslots ) BKNI_Free( hHsm->modules.pKeyslots );
    hHsm->modules.pKeyslots = NULL;

    BDBG_LEAVE( BHSM_Keyslot_Init );
    return rc;
}


void BHSM_Keyslot_Uninit( BHSM_Handle hHsm )
{
    BDBG_ENTER( BHSM_Keyslot_Uninit );

    if( hHsm->modules.pKeyslots->hBypassKeyslot ){
        BHSM_Keyslot_Free( hHsm->modules.pKeyslots->hBypassKeyslot );
        hHsm->modules.pKeyslots->hBypassKeyslot = NULL;
    }

    if ( hHsm->modules.pKeyslots == NULL ) {
        BERR_TRACE( BERR_INVALID_PARAMETER ); return;
    }

    BKNI_Free( hHsm->modules.pKeyslots );
    hHsm->modules.pKeyslots = NULL;

    BDBG_LEAVE( BHSM_Keyslot_Uninit );
    return;
}

void BHSM_Keyslot_GetDefaultAllocateSettings( BHSM_KeyslotAllocateSettings *pSettings )
{
    BDBG_ENTER( BHSM_Keyslot_GetDefaultAllocateSettings );

    BKNI_Memset( pSettings, 0, sizeof(*pSettings) );

    BDBG_LEAVE( BHSM_Keyslot_GetDefaultAllocateSettings );
    return;
}

BHSM_KeyslotHandle BHSM_Keyslot_Allocate( BHSM_Handle hHsm, const BHSM_KeyslotAllocateSettings *pSettings )
{
    BHSM_KeySlotModule* pModule;
    BERR_Code rc = BERR_SUCCESS;
    BHSM_P_KeySlot *pSlot = NULL;
    unsigned offset; /* offset to keyslot type */
    unsigned maxNumber;
   #ifndef BHSM_BUILD_HSM_FOR_SAGE
    unsigned i;
   #endif
    BDBG_ENTER( BHSM_Keyslot_Allocate );

    if( !pSettings ) { BERR_TRACE( BERR_INVALID_PARAMETER ); return NULL; }
    if( pSettings->slotType >= BHSM_KeyslotType_eMax ) { BERR_TRACE( BERR_INVALID_PARAMETER ); return NULL; }
    if((pSettings->owner != BHSM_SecurityCpuContext_eHost) && (pSettings->owner != BHSM_SecurityCpuContext_eSage))
        { BERR_TRACE( BERR_INVALID_PARAMETER ); return NULL; }

    pModule = hHsm->modules.pKeyslots;

    /* find a free slot. */
    offset    = pModule->types[pSettings->slotType].offset;
    maxNumber = pModule->types[pSettings->slotType].maxNumber;

   #ifndef BHSM_BUILD_HSM_FOR_SAGE
    /* search for a free slot.*/
    for( i = offset; i < offset+maxNumber; i++ )
    {
        if( pModule->pKeySlotHandles[i] == NULL )
        {
            pSlot = (BHSM_P_KeySlot*) BKNI_Malloc( sizeof(BHSM_P_KeySlot) );
            if ( !pSlot ) { BERR_TRACE( BERR_OUT_OF_SYSTEM_MEMORY ); goto error; }

            BKNI_Memset( pSlot, 0, sizeof(*pSlot) );
            pSlot->slotType = pSettings->slotType;
            pSlot->number = i-offset;
            pSlot->pModule = pModule;

            if( pSettings->owner == BHSM_SecurityCpuContext_eHost )
            {
                rc = _SetKeyslotOwnership( (BHSM_KeyslotHandle)pSlot, pSettings->owner );
                if( rc != BERR_SUCCESS ) { BERR_TRACE(rc); goto error; }

                rc = _InvalidateSlot( (BHSM_KeyslotHandle)pSlot, BHSM_PRESERVE_OWNERSHIP );
                if( rc != BERR_SUCCESS ) { BERR_TRACE(rc); goto  error;}
            }

            pModule->pKeySlotHandles[i] = pSlot;

            break;
        }
    }
    if( !pSlot ){
        BDBG_ERR(("Failed to allocate keyslot. Type[%u] Max[%u]", pSettings->slotType, maxNumber ));
        goto error;
    }

   #else /* we're on SAGE. The exact keyslot type/number is specified.*/
    if( pSettings->keySlotNumber >= maxNumber ){
        BERR_TRACE( BERR_INVALID_PARAMETER );
        goto error; /* slot number greater that number reserved for that type. */
    }

    if( pModule->pKeySlotHandles[offset + pSettings->keySlotNumber] != NULL ){
        BERR_TRACE( BERR_INVALID_PARAMETER );
        goto error;  /* specified slot (type,number) is not available. */
    }

    BDBG_MSG(("slot being acquired by requested ownership: %s", _KeyslotOwnerToString(pSettings->owner) ));
    pSlot = BKNI_Malloc( sizeof(BHSM_P_KeySlot) );
    if( !pSlot ) { BERR_TRACE( BERR_OUT_OF_SYSTEM_MEMORY ); goto error; }

    BKNI_Memset( pSlot, 0, sizeof(*pSlot) );
    pSlot->slotType = pSettings->slotType;
    pSlot->number = pSettings->keySlotNumber;
    pSlot->pModule = pModule;

    rc = _SetKeyslotOwnership( (BHSM_KeyslotHandle)pSlot, pSettings->owner );
    if( rc != BERR_SUCCESS ) { BERR_TRACE(rc); goto  error;}

    pModule->pKeySlotHandles[offset + pSettings->keySlotNumber] = pSlot;

    rc = _InvalidateSlot( (BHSM_KeyslotHandle)pSlot, BHSM_PRESERVE_OWNERSHIP );
    if( rc != BERR_SUCCESS ) { BERR_TRACE(rc); goto  error;}
   #endif

    BDBG_LEAVE( BHSM_Keyslot_Allocate );
    return (BHSM_KeyslotHandle)pSlot;

 error:
    if( pSlot ){ BKNI_Free( pSlot ); }

    BDBG_LEAVE( BHSM_Keyslot_Allocate );
    return NULL;

}

void BHSM_Keyslot_Free( BHSM_KeyslotHandle handle )
{
    BERR_Code rc = BERR_SUCCESS;
    unsigned offset;
    unsigned i;
    BHSM_P_KeySlot *pSlot;

    BDBG_ENTER( BHSM_Keyslot_Free );

    pSlot = (BHSM_P_KeySlot*)handle;

    if( pSlot->slotType >= BHSM_KeyslotType_eMax ) { BERR_TRACE( BERR_INVALID_PARAMETER ); return; }

    rc = _InvalidateSlot( handle, !BHSM_PRESERVE_OWNERSHIP );
    if( rc != BERR_SUCCESS ) { BERR_TRACE(rc); } /* continue. Best effort. */

    /* free external keyslots. */
    for( i = 0; i  < BHSM_KeyslotBlockEntry_eMax; i++ )
    {
        BHSM_P_KeyEntry *pEntry = &pSlot->entry[i];

        if( pEntry->configured && pEntry->pExternalSlot )
        {
            pEntry->pExternalSlot->allocated = false;
        }
    }

    offset = pSlot->pModule->types[pSlot->slotType].offset + pSlot->number;

    if( offset < BHSM_MAX_KEYSLOTS ){
        pSlot->pModule->pKeySlotHandles[offset] = NULL;
    }
    else{ BERR_TRACE(rc); }


    BKNI_Free( pSlot );

    BDBG_LEAVE( BHSM_Keyslot_Free );
    return;
}

void  BHSM_Keyslot_GetSettings( BHSM_KeyslotHandle handle, BHSM_KeyslotSettings *pSettings )
{
    BHSM_P_KeySlot *pSlot = (BHSM_P_KeySlot*)handle;

    BDBG_ENTER( BHSM_Keyslot_GetSettings );

    if( !pSettings ) { BERR_TRACE( BERR_INVALID_PARAMETER ); return; }

    *pSettings = pSlot->settings;

    BDBG_LEAVE( BHSM_Keyslot_GetSettings );
    return;
}

BERR_Code BHSM_Keyslot_SetSettings( BHSM_KeyslotHandle handle, const BHSM_KeyslotSettings *pSettings )
{
    BHSM_P_KeySlot *pSlot = (BHSM_P_KeySlot*)handle;

    BDBG_ENTER( BHSM_Keyslot_SetSettings );

    if( !pSettings ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }

    pSlot->settings = *pSettings;
    pSlot->configured = true;

    BDBG_LEAVE( BHSM_Keyslot_SetSettings );
    return BERR_SUCCESS;
}

void BHSM_Keyslot_GetEntrySettings( BHSM_KeyslotHandle handle,
                                    BHSM_KeyslotBlockEntry entry,
                                    BHSM_KeyslotEntrySettings *pSettings )
{
    BHSM_P_KeyEntry *pEntry;

    BDBG_ENTER( BHSM_Keyslot_GetEntrySettings );

    if( !pSettings ) { BERR_TRACE( BERR_INVALID_PARAMETER ); return; }
    if( entry >= BHSM_KeyslotBlockEntry_eMax ) { BERR_TRACE( BERR_INVALID_PARAMETER ); return; }

    pEntry = _GetEntry( handle, entry );

    *pSettings = pEntry->settings;

    BDBG_LEAVE( BHSM_Keyslot_GetEntrySettings );
    return;
}


BERR_Code BHSM_Keyslot_SetEntrySettings( BHSM_KeyslotHandle handle,
                                         BHSM_KeyslotBlockEntry entry,
                                         const BHSM_KeyslotEntrySettings *pSettings )
{
    BHSM_P_KeySlot *pSlot = (BHSM_P_KeySlot*)handle;
    BHSM_KeySlotModule* pModule;
    BHSM_P_KeyEntry *pEntry;
    unsigned i;
    BERR_Code rc = BERR_SUCCESS;

    BDBG_ENTER( BHSM_Keyslot_SetEntrySettings );

    if( !pSettings )                           { return BERR_TRACE( BERR_INVALID_PARAMETER ); }
    if( entry >= BHSM_KeyslotBlockEntry_eMax ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }

    pModule = pSlot->pModule;
    pEntry = _GetEntry( handle, entry );

    /* check if external keyslot is available. */
    if( pSettings->external.key || pSettings->external.iv )
    {
        if( !pEntry->pExternalSlot )
        {
            for( i = 0; i < BHSM_EXTERNAL_KEYSLOTS_MAX; i++ )
            {
                if( pModule->externalKeySlots[i].allocated == false )
                {
                    pModule->externalKeySlots[i].allocated = true;
                    pEntry->pExternalSlot = &pModule->externalKeySlots[i];
                    break;
                }
            }
            if( !pEntry->pExternalSlot ) /* failed to get a slot. */
            {
                return BERR_TRACE( BERR_NOT_AVAILABLE );
            }
        }
    }
    else
    {
        if( pEntry->pExternalSlot )
        {
            pEntry->pExternalSlot->allocated = false;
            pEntry->pExternalSlot = NULL;
        }
    }

    pEntry->settings = *pSettings;
    pEntry->configured = true;

    /* update BSP with KeySlot entry Configuration. */
    if( pEntry->pExternalSlot )
    {
        rc = _SetEntryKey( handle, entry, NULL );
        if( rc != BERR_SUCCESS ) return BERR_TRACE(rc);
    }

    BDBG_LEAVE( BHSM_Keyslot_SetEntrySettings );
    return BERR_SUCCESS;
}


BERR_Code BHSM_Keyslot_SetEntryKey ( BHSM_KeyslotHandle handle,
                                     BHSM_KeyslotBlockEntry entry,
                                     const BHSM_KeyslotKey *pKey )
{
    BERR_Code rc = BERR_SUCCESS;
    BHSM_P_KeySlot *pSlot = (BHSM_P_KeySlot*)handle;

    BDBG_ENTER( BHSM_Keyslot_SetEntryKey );

    if( !pKey )                                { return BERR_TRACE( BERR_INVALID_PARAMETER ); }
    if( entry >= BHSM_KeyslotBlockEntry_eMax ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }

    if( pSlot->configured == false )              { return BERR_TRACE( BERR_INVALID_PARAMETER ); }
    if( pSlot->entry[entry].configured == false ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }

    rc = _SetEntryKey( handle, entry, pKey );
    if( rc != BERR_SUCCESS ) return BERR_TRACE(rc);

    BDBG_LEAVE( BHSM_Keyslot_SetEntryKey );
    return BERR_SUCCESS;
}



BERR_Code BHSM_Keyslot_SetEntryIv( BHSM_KeyslotHandle handle,
                                    BHSM_KeyslotBlockEntry entry,
                                    const BHSM_KeyslotIv *pIv,
                                    const BHSM_KeyslotIv *pIv2 )
{
    BERR_Code rc = BERR_SUCCESS;
    BHSM_P_KeySlot *pSlot = (BHSM_P_KeySlot*)handle;

    BDBG_ENTER( BHSM_Keyslot_SetEntryIv );

    if( !pIv && !pIv2 )                        { return BERR_TRACE( BERR_INVALID_PARAMETER ); } /* we need oen IV */
    if( entry >= BHSM_KeyslotBlockEntry_eMax ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }

    if( pSlot->configured == false )              { return BERR_TRACE( BERR_INVALID_PARAMETER ); }
    if( pSlot->entry[entry].configured == false ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }

    rc = _SetEntryIv( handle, entry, pIv, pIv2 );
    if( rc != BERR_SUCCESS ) return BERR_TRACE(rc);

    BDBG_LEAVE( BHSM_Keyslot_SetEntryIv );
    return BERR_SUCCESS;
}


BERR_Code BHSM_Keyslot_Invalidate( BHSM_KeyslotHandle handle )
{
    BERR_Code rc = BERR_SUCCESS;
    BDBG_ENTER( BHSM_Keyslot_Invalidate );
    BSTD_UNUSED(handle);

    rc = _InvalidateSlot( handle, BHSM_PRESERVE_OWNERSHIP );
    if( rc != BERR_SUCCESS ) return BERR_TRACE(rc);

    BDBG_LEAVE( BHSM_Keyslot_Invalidate );
    return BERR_SUCCESS;
}


BERR_Code BHSM_Keyslot_GetEntryExternalKeySettings( BHSM_KeyslotHandle handle,
                                                    BHSM_KeyslotBlockEntry entry,
                                                    BHSM_KeyslotExternalKeyData *pData )
{
    BHSM_P_KeySlot *pSlot = (BHSM_P_KeySlot*)handle;
    BHSM_P_KeyEntry *pEntry;

    BDBG_ENTER( BHSM_Keyslot_GetEntryExternalKeySettings );

    if( !pData )                               { return BERR_TRACE( BERR_INVALID_PARAMETER ); }
    if( entry >= BHSM_KeyslotBlockEntry_eMax ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }
    if( pSlot->configured == false )           { return BERR_TRACE( BERR_INVALID_PARAMETER ); }

    pEntry = _GetEntry( handle, entry );
    if( !pEntry ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }
    if( pEntry->configured == false ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }
    if( pEntry->pExternalSlot == false ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }

    BKNI_Memset( pData, 0, sizeof(*pData) );

    pData->slotIndex = pEntry->pExternalSlot->slotPtr;

    if( pEntry->settings.external.key ) {
        pData->key.valid = true;
        pData->key.offset = pEntry->pExternalSlot->offsetKey;
    }
    if( pEntry->settings.external.iv ) {
        pData->iv.valid = true;
        pData->iv.offset = pEntry->pExternalSlot->offsetIv;
    }

    BDBG_LEAVE( BHSM_Keyslot_GetEntryExternalKeySettings );
    return BERR_SUCCESS;
}


BERR_Code BHSM_Keyslot_InvalidateEntry( BHSM_KeyslotHandle handle,
                                         BHSM_KeyslotBlockEntry entry )
{
    BERR_Code rc = BERR_SUCCESS;
    BDBG_ENTER( BHSM_Keyslot_InvalidateEntry );

    rc = _InvalidateEntry( handle, entry );
    if( rc != BERR_SUCCESS ) return BERR_TRACE(rc);

    BDBG_LEAVE( BHSM_Keyslot_InvalidateEntry );
    return BERR_SUCCESS;
}



BERR_Code BHSM_Keyslot_AddPidChannel_WithSettings( BHSM_KeyslotHandle handle,
                                                   unsigned pidChannelIndex,
                                                   const BHSM_KeyslotAddPidChannelSettings *pSettings )
{
    BERR_Code rc = BERR_SUCCESS;
    BHSM_P_KeySlotPidAdd hsmAddPid;
    BHSM_P_KeySlot *pSlot = (BHSM_P_KeySlot*)handle;

    BSTD_UNUSED( pSettings );

    BDBG_ENTER( BHSM_Keyslot_AddPidChannel );

    if( !pSlot ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }

    BKNI_Memset( &hsmAddPid, 0, sizeof(hsmAddPid) );

    hsmAddPid.in.pidChanStart  = pidChannelIndex;
    hsmAddPid.in.keySlotType   = _convertSlotType( pSlot->slotType );
    hsmAddPid.in.keySlotNumber = pSlot->number;

    #if 0
    hsmAddPid.keySlotTypeB; hsmAddPid.keySlotNumberB; hsmAddPid.setMultiplePidChan;
    hsmAddPid.spidUsePointerB; hsmAddPid.estinationPipeSel;
    #endif

    rc = BHSM_P_KeySlot_PidAdd( _GetHsmHandle(handle), &hsmAddPid );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE( rc ); }

    BDBG_LEAVE( BHSM_Keyslot_AddPidChannel );
    return BERR_SUCCESS;
}

BERR_Code BHSM_Keyslot_AddPidChannel( BHSM_KeyslotHandle handle, unsigned pidChannelIndex )
{
    BERR_Code rc = BERR_SUCCESS;

    rc = BHSM_Keyslot_AddPidChannel_WithSettings( handle, pidChannelIndex, NULL );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE( rc ); }

    return rc;
}

BERR_Code BHSM_Keyslot_RemovePidChannel( BHSM_KeyslotHandle handle,
                                          unsigned pidChannelIndex )
{
    BERR_Code           rc = BERR_SUCCESS;
    BHSM_P_KeySlot *pSlot = (BHSM_P_KeySlot*)handle;
    BHSM_P_KeySlotPidRemove bspRemovePid;

    BDBG_ENTER( BHSM_Keyslot_RemovePidChannel );

    if( !pSlot ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }

    BKNI_Memset( &bspRemovePid, 0, sizeof(bspRemovePid) );
    bspRemovePid.in.pidChanStart = pidChannelIndex;
    bspRemovePid.in.pidChanEnd = pidChannelIndex;

    rc = BHSM_P_KeySlot_PidRemove( _GetHsmHandle(handle), &bspRemovePid );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE( rc ); }

    BDBG_LEAVE( BHSM_Keyslot_RemovePidChannel );
    return BERR_SUCCESS;
}


BERR_Code BHSM_GetKeySlotInfo( BHSM_KeyslotHandle handle, BHSM_KeyslotInfo *pInfo )
{
    BHSM_P_KeySlot *pSlot = (BHSM_P_KeySlot*)handle;

    BDBG_ENTER( BHSM_GetKeySlotInfo );

    if( !pSlot ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }
    if( !pInfo ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }

    pInfo->type   = pSlot->slotType;
    pInfo->number = pSlot->number;

    BDBG_LEAVE( BHSM_GetKeySlotInfo );

    return BERR_SUCCESS;
}

static BERR_Code _SetEntryIv( BHSM_KeyslotHandle handle, BHSM_KeyslotBlockEntry entry, const BHSM_KeyslotIv *pIv, const BHSM_KeyslotIv *pIv2)
{
    BERR_Code rc = BERR_SUCCESS;
    BHSM_P_KeySlot *pSlot = (BHSM_P_KeySlot*)handle;
    BHSM_P_KeyEntry *pEntry;
    BHSM_P_KeySlotClearIvSet bspSetIv;
    unsigned keyOffset = 0;

    BDBG_ENTER( _SetEntryIv );

    pEntry = _GetEntry( handle, entry );
    if( pEntry == NULL ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }

    if( !pIv && !pIv2 ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }

    BKNI_Memset( &bspSetIv, 0, sizeof(bspSetIv) );

    bspSetIv.in.blockType = _GetEntryBlockType(entry);
    bspSetIv.in.entryType = _GetEntryPolarity(entry);
    bspSetIv.in.keySlotType = _convertSlotType(pSlot->slotType);
    bspSetIv.in.keySlotNumber = pSlot->number;

    if( pIv )
    {
        switch( pIv->size ) {
            case 16: { break;}
            case 8:  { break;}
            default: return BERR_TRACE( BERR_INVALID_PARAMETER );
        }

        keyOffset = 0; /* offset of Iv1 in bytes */
        BHSM_Mem32cpy( &bspSetIv.in.iv[keyOffset], pIv->iv, pIv->size );
        bspSetIv.in.ivType = Bsp_KeySlotIvType_eIv;

        rc = BHSM_P_KeySlot_ClearIvSet( _GetHsmHandle(handle), &bspSetIv );
        if( rc != BERR_SUCCESS ) { return BERR_TRACE( rc ); }
    }

    if( pIv2 )
    {
        switch( pIv2->size ) {
            case 16: { break;}
            case 8:  { break;}
            default: return BERR_TRACE( BERR_INVALID_PARAMETER );
        }
        keyOffset = 0; /* offset of Iv2 in bytes */
        BHSM_Mem32cpy( &bspSetIv.in.iv[keyOffset], pIv2->iv, pIv2->size ) ;
        bspSetIv.in.ivType = Bsp_KeySlotIvType_eAesShortIv;

        rc = BHSM_P_KeySlot_ClearIvSet( _GetHsmHandle(handle), &bspSetIv );
        if( rc != BERR_SUCCESS ) { return BERR_TRACE( rc ); }
    }

    BDBG_LEAVE( _SetEntryIv );
    return BERR_SUCCESS;
}

static BERR_Code _SetEntryKey( BHSM_KeyslotHandle handle, BHSM_KeyslotBlockEntry entry, const BHSM_KeyslotKey *pKey )
{
    BERR_Code rc = BERR_SUCCESS;
    BHSM_P_KeySlotClearKeySet bspSetKey;
    BHSM_P_KeySlot *pSlot = (BHSM_P_KeySlot*)handle;
    BHSM_P_KeyEntry *pEntry;
    Bsp_KeySize_e keySize;

    BDBG_ENTER( _SetEntryKey );

    pEntry = _GetEntry( handle, entry );
    if( pEntry == NULL ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }

    BKNI_Memset( &bspSetKey, 0, sizeof(bspSetKey) );

    if( pKey ) {
        unsigned keyOffset = 0;

        switch( pKey->size ) {
            case 8:  { keySize = Bsp_KeySize_e64; break;}
            case 16: { keySize = Bsp_KeySize_e128; break;}
            case 24: { keySize = Bsp_KeySize_e192; break;}
            case 32: { keySize = Bsp_KeySize_e256; break;}
            default: return BERR_TRACE( BERR_INVALID_PARAMETER );
        }
        keyOffset = (8-(2*(keySize+1))); /* offset of key in bytes */

        if( keyOffset >= sizeof(bspSetKey.in.keyData) ||
            keyOffset + pKey->size > sizeof(bspSetKey.in.keyData) ) {
            return BERR_TRACE( BERR_INVALID_PARAMETER );
        }
        BHSM_Mem32cpy( &bspSetKey.in.keyData[keyOffset], pKey->key, pKey->size );

        if( pKey->size  == 8 ) {
            keyOffset -= 2;
            if( keyOffset >= sizeof(bspSetKey.in.keyData) ||
                keyOffset + pKey->size > sizeof(bspSetKey.in.keyData) ) {
                return BERR_TRACE( BERR_INVALID_PARAMETER );
            }
            BHSM_Mem32cpy( &bspSetKey.in.keyData[keyOffset], pKey->key, pKey->size ) ;
        }
    }
    else {
        unsigned i;
        /* dummy key data. All zero are not be allowed for some configurations.  */
        for( i = 0; i < sizeof(bspSetKey.in.keyData)/sizeof(bspSetKey.in.keyData[0]); i++ ) {
            bspSetKey.in.keyData[i] = 0xFFFFFF00 | i;
        }
    }
    bspSetKey.in.blockType = _GetEntryBlockType(entry);
    bspSetKey.in.entryType = _GetEntryPolarity(entry);
    bspSetKey.in.keySlotType = _convertSlotType(pSlot->slotType);
    bspSetKey.in.keySlotNumber = pSlot->number;
    bspSetKey.in.keyMode = 0 /*Bsp_KeyMode_eRegular */;

    bspSetKey.in.modeWords[0] = compileControl0_GlobalHi( handle );
    bspSetKey.in.modeWords[1] = compileControl1_GlobalLo( handle );
    bspSetKey.in.modeWords[2] = compileControl2_ModeHi( handle, entry );
    bspSetKey.in.modeWords[3] = compileControl3_ModeLo( handle, entry );

    if( pEntry->settings.external.key && pEntry->pExternalSlot ) {
        bspSetKey.in.extKeyPtr = pEntry->pExternalSlot->offsetKey + pEntry->pExternalSlot->slotPtr;
    }
    if( pEntry->settings.external.iv )  {
        bspSetKey.in.extIvPtr = pEntry->pExternalSlot->offsetIv + pEntry->pExternalSlot->slotPtr;
    }

    rc = BHSM_P_KeySlot_ClearKeySet( _GetHsmHandle(handle), &bspSetKey );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE( rc ); }

    BDBG_LEAVE( _SetEntryKey );
    return BERR_SUCCESS;
}

static BERR_Code _InvalidateSlot( BHSM_KeyslotHandle handle, bool preserveOwner )
{
    BERR_Code rc = BERR_SUCCESS;
    BHSM_P_KeySlot *pSlot = (BHSM_P_KeySlot*)handle;
    BHSM_P_KeySlotInvalidate bspInvalidate;
    BHSM_SecurityCpuContext owner = BHSM_SecurityCpuContext_eNone;

    BDBG_ENTER( _InvalidateSlot );

    if( !pSlot ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }

    if( preserveOwner ) {
        rc = _GetKeyslotOwnership( handle, &owner );
        if( rc != BERR_SUCCESS ) { BERR_TRACE( rc ); goto BHSM_P_DONE_LABEL; }
    }

    BKNI_Memset( &bspInvalidate, 0, sizeof(bspInvalidate) );
    bspInvalidate.in.invalidateMethod  = 1 /*Bsp_KeySlotInvalidateOperation_eInvalidateAll*/;
    bspInvalidate.in.keySlotType       = _convertSlotType( pSlot->slotType );
    bspInvalidate.in.keySlotNumber     = pSlot->number;

    rc = BHSM_P_KeySlot_Invalidate( _GetHsmHandle(handle), &bspInvalidate );
    if( rc != BERR_SUCCESS ) { BERR_TRACE( rc ); goto BHSM_P_DONE_LABEL; }

    if( preserveOwner && (owner != BHSM_SecurityCpuContext_eNone) )
    {
        rc = _SetKeyslotOwnership( handle, owner );
        if( rc != BERR_SUCCESS ) { BERR_TRACE( rc ); goto BHSM_P_DONE_LABEL; }
    }

BHSM_P_DONE_LABEL:

    BDBG_LEAVE( _InvalidateSlot );
    return BERR_SUCCESS;
}

static BERR_Code _InvalidateEntry( BHSM_KeyslotHandle handle, BHSM_KeyslotBlockEntry entry )
{
    BERR_Code rc = BERR_SUCCESS;
    BHSM_P_KeySlot *pSlot = (BHSM_P_KeySlot*)handle;
    uint8_t block = 0;
    uint8_t polarity = 0;
    BHSM_P_KeySlotInvalidate bspInvalidate;

    BDBG_ENTER( _InvalidateEntry );

    if( !pSlot ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }

    switch(_GetEntryBlockType(entry))
    {
        case BHSM_KeyslotBlockType_eCpd: {block = 0; break;}
        case BHSM_KeyslotBlockType_eCa:  {block = 1; break;}
        case BHSM_KeyslotBlockType_eCps: {block = 2; break;}
        default: return BERR_TRACE( BERR_INVALID_PARAMETER );
    }
    switch(_GetEntryPolarity(entry))
    {
        case BHSM_KeyslotPolarity_eOdd:   {polarity = 0; break;}
        case BHSM_KeyslotPolarity_eEven:  {polarity = 1; break;}
        case BHSM_KeyslotPolarity_eClear: {polarity = 2; break;}
        default: return BERR_TRACE( BERR_INVALID_PARAMETER );
    }

    BKNI_Memset( &bspInvalidate, 0, sizeof(bspInvalidate) );
    bspInvalidate.in.blockType         = block;
    bspInvalidate.in.entryType         = polarity;
    bspInvalidate.in.keySlotType       = _convertSlotType( pSlot->slotType );
    bspInvalidate.in.keySlotNumber     = pSlot->number;
    /*bspInvalidate.sc01ModeWordMapping*/
    bspInvalidate.in.invalidateMethod  = 0 /*Bsp_KeySlotInvalidateOperation_eInvalidateOneEntry */;

    rc = BHSM_P_KeySlot_Invalidate( _GetHsmHandle(handle), &bspInvalidate );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE( rc ); }

    BDBG_LEAVE( _InvalidateEntry );
    return BERR_SUCCESS;
}

static uint32_t compileControl0_GlobalHi( BHSM_KeyslotHandle handle )
{
    BSTD_UNUSED( handle );
    /* nothing! */
    return 0;
}

static uint32_t compileControl1_GlobalLo( BHSM_KeyslotHandle handle )
{
    BHSM_KeyslotSettings *pSlotSettings;
    uint32_t control = 0;
    unsigned x;

    pSlotSettings = _GetSlotSetting( handle );
    if( pSlotSettings == NULL ) { BERR_TRACE( BERR_INVALID_PARAMETER ); return control; }

    /* pack source regions */
    for( x = 0; x < BHSM_SecurityRegion_eMax; x++ ) {
        if( pSlotSettings->regions.source[x] ) {
            control |= 1 << (0+x);
        }
    }
    /* pack RPipe regions */
    for( x = 0; x < BHSM_SecurityRegion_eMax; x++ ) {
        if( pSlotSettings->regions.destinationRPipe[x] ) {
            control |= 1 << (8+x);
        }
    }
    /* pack GPipe regions */
    for( x = 0; x < BHSM_SecurityRegion_eMax; x++ ) {
        if( pSlotSettings->regions.destinationGPipe[x] ) {
            control |= 1 << (16+x);
        }
    }
    /* pack encrypt before rave. */
    if( pSlotSettings->encryptBeforeRave ) {
        control |= 1 << 24;
    }

    return control;
}

static uint32_t compileControl2_ModeHi( BHSM_KeyslotHandle handle, BHSM_KeyslotBlockEntry entry )
{
    BHSM_P_KeyEntry *pEntry;
    uint32_t control = 0;
    unsigned polarity = 0;

    pEntry = _GetEntry( handle, entry );
    if( pEntry == NULL ) { BERR_TRACE( BERR_INVALID_PARAMETER ); return control; }

    /* pack external Key allowed */
    if( pEntry->settings.external.key ) { control |= 1; }         /* 1 bit. b00 */

    /* pack external IV allowed */
    if( pEntry->settings.external.iv ) { control |= (1 << 1); }  /* 1 bit. b01 */

    /* pack RPIPE_EN */
    if( pEntry->settings.rPipeEnable ) { control |= (1 << 2); }  /* 1 bit. b02 */

    /* pack GPIPE_EN */
    if( pEntry->settings.gPipeEnable ) { control |= (1 << 3); }  /* 1 bit. b03 */

    if( pEntry->settings.outputPolarity.specify ) {

       /* pack RPIPE_SC_VALUE */
       switch( pEntry->settings.outputPolarity.rPipe )
       {
            case BHSM_KeyslotPolarity_eClear:    { polarity = 0x0; break;}
            case BHSM_KeyslotPolarity_eReserved: { polarity = 0x1; break;}
            case BHSM_KeyslotPolarity_eEven:     { polarity = 0x2; break;}
            case BHSM_KeyslotPolarity_eOdd:      { polarity = 0x3; break;}
            default:{ polarity = 0x0;  BERR_TRACE( pEntry->settings.outputPolarity.rPipe ); } /* unsupported polarity. */
       }
       control |= ( (polarity & 0x3) << 4);         /* 2 bit. b04-b05 */

        /* pack GPIPE_SC_VALUE */
        switch( pEntry->settings.outputPolarity.gPipe )
        {
            case BHSM_KeyslotPolarity_eClear:    { polarity = 0x0; break;}
            case BHSM_KeyslotPolarity_eReserved: { polarity = 0x1; break;}
            case BHSM_KeyslotPolarity_eEven:     { polarity = 0x2; break;}
            case BHSM_KeyslotPolarity_eOdd:      { polarity = 0x3; break;}
            default:{ polarity = 0x0; BERR_TRACE( pEntry->settings.outputPolarity.gPipe ); } /* unsupported polarity. */
        }
        control |= ( (polarity & 0x3) << 6);         /* 2 bit. b06-b07 */
    }

    /* Multi2 */
    if( pEntry->settings.algorithm == BHSM_CryptographicAlgorithm_eMulti2 ) {
        control |= ( ( pEntry->settings.multi2.roundCount & 0x7)  << 21 );       /* 3 bit. b21-23 */
        control |= ( ( pEntry->settings.multi2.keySelect  & 0x3F) << 15 );       /* 6 bit. b15-20 */
    }

    return control;
}

static uint32_t compileControl3_ModeLo( BHSM_KeyslotHandle handle, BHSM_KeyslotBlockEntry entry )
{
    uint32_t control = 0;
    unsigned algorithm = 0;
    unsigned cipherMode = 0;
    unsigned terminationMode = 0;
    unsigned counterSize = 0;
    unsigned ivMode = 0;
    BHSM_P_KeyEntry *pEntry;

    pEntry = _GetEntry( handle, entry );
    if( pEntry == NULL ) { BERR_TRACE( BERR_INVALID_PARAMETER ); return control; }

    /* pack algorithm */
    switch( pEntry->settings.algorithm )
    {
        case BHSM_CryptographicAlgorithm_eDvbCsa2:   { algorithm = 0x0; break; }
        case BHSM_CryptographicAlgorithm_eDvbCsa3:   { algorithm = 0x1; break; }
        case BHSM_CryptographicAlgorithm_eMulti2:    { algorithm = 0x2; break; }
        case BHSM_CryptographicAlgorithm_eDes:       { algorithm = 0x3; break; }
        case BHSM_CryptographicAlgorithm_e3DesAba:   { algorithm = 0x4; break; }
        case BHSM_CryptographicAlgorithm_e3DesAbc:   { algorithm = 0x5; break; }
        case BHSM_CryptographicAlgorithm_eAes128:    { algorithm = 0x6; break; }
        case BHSM_CryptographicAlgorithm_eAes192:    { algorithm = 0x7; break; }
        case BHSM_CryptographicAlgorithm_eAes256:    { algorithm = 0x8; break; }
        case BHSM_CryptographicAlgorithm_eCam128:    { algorithm = 0xa; break; }
        case BHSM_CryptographicAlgorithm_eCam192:    { algorithm = 0xb; break; }
        case BHSM_CryptographicAlgorithm_eCam256:    { algorithm = 0xc; break; }
        case BHSM_CryptographicAlgorithm_eGhash:     { algorithm = 0xd; break; }

        /* Followings are not listed in the key_mode_lo registers, to be supported. */
        case BHSM_CryptographicAlgorithm_eMsMultiSwapMac: { algorithm = 0xe; break; }
        case BHSM_CryptographicAlgorithm_eWmDrmPd:   { algorithm = 0xf; break; }
        case BHSM_CryptographicAlgorithm_eAes128G:   { algorithm = 0x10; break; }
        case BHSM_CryptographicAlgorithm_eHdDvd:     { algorithm = 0x11; break; }
        case BHSM_CryptographicAlgorithm_eBrDvd:     { algorithm = 0x12; break; }
        case BHSM_CryptographicAlgorithm_eReserved19:{ algorithm = 0x13; break; }
        default:BERR_TRACE( BERR_INVALID_PARAMETER ); /* unsupported algorithm. */
    }
    control |= (algorithm & 0x1f); /* 5 bits. b00-b04 */

    if( pEntry->settings.algorithm == BHSM_CryptographicAlgorithm_eDvbCsa2 ) {
        /* pack DVB-CSA2 mode */
        if( pEntry->settings.pesMode ) {
            control |= 1<<5;
        }
    }
    else {
        /* pack cipher mode */
        switch( pEntry->settings.algorithmMode )
        {
            case BHSM_CryptographicAlgorithmMode_eEcb:     { cipherMode = 0x0; break; }
            case BHSM_CryptographicAlgorithmMode_eCbc:     { cipherMode = 0x1; break; }
            case BHSM_CryptographicAlgorithmMode_eCounter: { cipherMode = 0x2; break; }
            case BHSM_CryptographicAlgorithmMode_eRcbc:    { cipherMode = 0x3; break; }
            default:BERR_TRACE( BERR_INVALID_PARAMETER );   /* unsupported cipher mode. */
        }
        control |= ((cipherMode & 0x7) << 5); /* 3 bits. b05-b07 */
    }

    if( pEntry->settings.algorithmMode == BHSM_CryptographicAlgorithmMode_eCounter ) {
        /* pack counter mode */
        control |= ( (pEntry->settings.counterMode & 0xf) << 8);  /* 4 bits. b08-b11  */
    }
    else{
        /* pack termination mode. */
        switch( pEntry->settings.terminationMode )
        {
            case BHSM_Keyslot_TerminationMode_eClear:             { terminationMode = 0x0; break; }
            case BHSM_Keyslot_TerminationMode_eScte52:            { terminationMode = 0x1; break; }
            case BHSM_Keyslot_TerminationMode_eCtsEcb:            { terminationMode = 0x2; break; }
            case BHSM_KeySlot_TerminationMode_eCoronado:          { terminationMode = 0x3; break; }
            case BHSM_Keyslot_TerminationMode_eCtsDvbCpcm:        { terminationMode = 0x4; break; }
            case BHSM_Keyslot_TerminationMode_eFrontResidue:      { terminationMode = 0x5; break; }
            case BHSM_Keyslot_TerminationMode_eMsc:               { terminationMode = 0x6; break; }
            case BHSM_Keyslot_TerminationMode_eReserved0x06:      { terminationMode = 0x7; break; }
            case BHSM_Keyslot_TerminationMode_eTsAndPacket:       { terminationMode = 0x8; break; }
            case BHSM_Keyslot_TerminationMode_ePacket:            { terminationMode = 0x9; break; }
            case BHSM_Keyslot_TerminationMode_eCbcMac:            { terminationMode = 0xa; break; }
            case BHSM_Keyslot_TerminationMode_eCMac:              { terminationMode = 0xb; break; }
            default:BERR_TRACE( BERR_INVALID_PARAMETER );   /* unsupported cipher mode. */
        }
        control |= ((terminationMode & 0xf) << 8); /* 4 bits. b08-b11 */
    }

    if( pEntry->settings.algorithmMode == BHSM_CryptographicAlgorithmMode_eCounter ) {
        /* pack counter size */
        switch( pEntry->settings.counterSize )
        {
            case 32:  { counterSize = 0x0; break; }
            case 64:  { counterSize = 0x1; break; }
            case 96:  { counterSize = 0x2; break; }
            case 128: { counterSize = 0x3; break; }
            default:BERR_TRACE( BERR_INVALID_PARAMETER );   /* unsupported counter size. */
        }
        control |= ((counterSize & 0x3) << 12); /* 2 bits. b12-b13 */
    }
    else {
        /* Iv Mode */
        switch( pEntry->settings.ivMode )
        {
            case BHSM_Keyslot_IvMode_eNoPreProc:          { ivMode = 0x00; break; };
            case BHSM_Keyslot_IvMode_eMdi:                { ivMode = 0x01; break; };
            case BHSM_Keyslot_IvMode_eMdd:                { ivMode = 0x02; break; };
            case BHSM_Keyslot_IvMode_eNoPreProcWriteBack: { ivMode = 0x03; break; };
            default:BERR_TRACE( BERR_INVALID_PARAMETER );   /* unsupported iv mode. */
        }
        control |= ((ivMode & 0x3) << 12); /* 2 bits, b12-b13 */
    }

    /* solitary block processing */
    control |= ((pEntry->settings.solitaryMode & 0x7) << 14); /* 3 bits, b14-b16 */

    /* Key Offset */   /* TODO ... maybe not be required. */
    if( pEntry->settings.external.key && pEntry->pExternalSlot ) {
        control |= ((pEntry->pExternalSlot->offsetKey & 0x7F) << 17); /* 7 bits, b17-b23 */
    }

    /* Iv  Offset */ /* TODO ... maybe not be required. */
    if( pEntry->settings.external.iv && pEntry->pExternalSlot ) {
        control |= ((pEntry->pExternalSlot->offsetIv & 0x7F) << 24); /* 7 bits, b24-b30 */
    }

    return control;
}



static BHSM_KeyslotSettings* _GetSlotSetting( BHSM_KeyslotHandle handle )
{
    BHSM_P_KeySlot *pSlot = (BHSM_P_KeySlot*)handle;

    if( pSlot == NULL )              { BERR_TRACE( BERR_INVALID_PARAMETER ); return NULL; }
    if( pSlot->configured == false ) { BERR_TRACE( BERR_INVALID_PARAMETER ); return NULL; }

    return &(pSlot->settings);
}

static BHSM_P_KeyEntry* _GetEntry( BHSM_KeyslotHandle handle, BHSM_KeyslotBlockEntry entry )
{
    BHSM_P_KeySlot *pSlot = (BHSM_P_KeySlot*)handle;

    if( pSlot == NULL ) { BERR_TRACE( BERR_INVALID_PARAMETER ); return NULL; }
    if( entry >= BHSM_KeyslotBlockEntry_eMax ) { BERR_TRACE( BERR_INVALID_PARAMETER ); return NULL; }

    return &(pSlot->entry[entry]);
}

static BHSM_Handle _GetHsmHandle( BHSM_KeyslotHandle handle )
{
    BHSM_P_KeySlot *pSlot = (BHSM_P_KeySlot*)handle;
    return pSlot->pModule->hHsm;
}

#ifndef BHSM_BUILD_HSM_FOR_SAGE
static BERR_Code _Keyslot_Init( BHSM_Handle hHsm, BHSM_KeyslotModuleSettings *pSettings )
{
    BERR_Code rc = BERR_SUCCESS;
    BHSM_P_KeySlotInit hsmInit;

    BDBG_ENTER( _Keyslot_Init );

    if( pSettings == NULL ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }

    BKNI_Memset( &hsmInit, 0, sizeof(hsmInit) );

    hsmInit.in.keySlotNumberIvPerSlot128  = pSettings->numKeySlotsForType[BHSM_KeyslotType_eIvPerSlot];
    hsmInit.in.keySlotNumberIvPerBlock128 = pSettings->numKeySlotsForType[BHSM_KeyslotType_eIvPerBlock];
    hsmInit.in.keySlotNumberIvPerBlock256 = pSettings->numKeySlotsForType[BHSM_KeyslotType_eIvPerBlock256];
    hsmInit.in.keySlotNumberIvPerEntry256 = pSettings->numKeySlotsForType[BHSM_KeyslotType_eIvPerEntry] +
                                            pSettings->numKeySlotsForType[BHSM_KeyslotType_eIvPerEntry256];

    rc = BHSM_P_KeySlot_Init( hHsm, &hsmInit );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }

    if( hsmInit.out.keySlotNumberIvPerSlot128  != pSettings->numKeySlotsForType[BHSM_KeyslotType_eIvPerSlot] ||
        hsmInit.out.keySlotNumberIvPerBlock128 != pSettings->numKeySlotsForType[BHSM_KeyslotType_eIvPerBlock] ||
        hsmInit.out.keySlotNumberIvPerBlock256 != pSettings->numKeySlotsForType[BHSM_KeyslotType_eIvPerBlock256] )
    {
        return BERR_TRACE( BERR_NOT_AVAILABLE ); /* not able to provide the requested number of keyslots. */
    }

    BDBG_LOG(("KeySlot types: IvPerSlot128[%u] IvPerBlock128[%u] IvPerBlock256[%u] IvPerEntry256[%u]"
                                                             , hsmInit.out.keySlotNumberIvPerSlot128
                                                             , hsmInit.out.keySlotNumberIvPerBlock128
                                                             , hsmInit.out.keySlotNumberIvPerBlock256
                                                             , hsmInit.out.keySlotNumberIvPerEntry256 ));

    BDBG_LEAVE( _Keyslot_Init );
    return rc;
}
#else
/* function that configures loads HSM configuration from BSP  */
static BERR_Code _Keyslot_LoadConfigurationFromBsp( BHSM_Handle hHsm )
{
    BERR_Code           rc = BERR_SUCCESS;
    BHSM_KeySlotModule* pModule;
    BHSM_P_KeySlotQuery bspQuery;
    uint8_t             numSlots[BHSM_KeyslotType_eMax] =  {0};
    unsigned            count = 0;
    unsigned            i;

    BDBG_ENTER( _Keyslot_LoadConfigurationFromBsp );

    BKNI_Memset( &bspQuery, 0, sizeof(bspQuery) );

    rc = BHSM_P_KeySlot_Query( hHsm, &bspQuery );
    BDBG_LOG((" BHSM_P_KeySlot_Query BSP ERROR workaround in place"));
    if( rc != BERR_SUCCESS ) {
    /*    return BERR_TRACE( BERR_NOT_AVAILABLE );*/
    }

    BDBG_LOG(("KeySlot types: IvPerSlot128[%u] IvPerBlock128[%u] IvPerBlock256[%u] IvPerEntry256[%u]"
                                                                , bspQuery.out.keySlotNumberIvPerSlot128
                                                                , bspQuery.out.keySlotNumberIvPerBlock128
                                                                , bspQuery.out.keySlotNumberIvPerBlock256
                                                                , bspQuery.out.keySlotNumberIvPerEntry256 ));

    pModule = hHsm->modules.pKeyslots;
    for( i = 0; i < BHSM_KeyslotType_eMax; i++ ){
        switch(i)
        {
            case BHSM_KeyslotType_eIvPerSlot:
                numSlots[i] = bspQuery.out.keySlotNumberIvPerSlot128;
                break;
            case BHSM_KeyslotType_eIvPerBlock:
                numSlots[i] = bspQuery.out.keySlotNumberIvPerBlock128;
                break;
            case BHSM_KeyslotType_eIvPerEntry:
                numSlots[i] = 0; /*BSP doesn't support this for now?*/
                break;
            case BHSM_KeyslotType_eIvPerBlock256:
                numSlots[i] = bspQuery.out.keySlotNumberIvPerBlock256;
                break;
            case BHSM_KeyslotType_eIvPerEntry256:
                numSlots[i] = bspQuery.out.keySlotNumberIvPerEntry256;
                break;
            default:
                break;
        }
        pModule->types[i].offset = count;
        pModule->types[i].maxNumber = numSlots[i];
        count += numSlots[i];
    }

    BDBG_LEAVE( _Keyslot_LoadConfigurationFromBsp );
    return BERR_SUCCESS;
}
#endif

static BHSM_KeyslotBlockType _GetEntryBlockType( BHSM_KeyslotBlockEntry entry )
{

    BDBG_CASSERT( BHSM_KeyslotBlockType_eCpd == 0 );
    BDBG_CASSERT( BHSM_KeyslotBlockType_eCa  == 1 );
    BDBG_CASSERT( BHSM_KeyslotBlockType_eCps == 2 );

    switch( entry )
    {
        case BHSM_KeyslotBlockEntry_eCpdOdd:
        case BHSM_KeyslotBlockEntry_eCpdEven:
        case BHSM_KeyslotBlockEntry_eCpdClear:
        {
            return BHSM_KeyslotBlockType_eCpd;
        }
        case BHSM_KeyslotBlockEntry_eCaOdd:
        case BHSM_KeyslotBlockEntry_eCaEven:
        case BHSM_KeyslotBlockEntry_eCaClear:
        {
            return BHSM_KeyslotBlockType_eCa;
        }
        case BHSM_KeyslotBlockEntry_eCpsOdd:
        case BHSM_KeyslotBlockEntry_eCpsEven:
        case BHSM_KeyslotBlockEntry_eCpsClear:
        {
            return BHSM_KeyslotBlockType_eCps;
        }
        default: { BERR_TRACE( entry ); }
    }

    return BHSM_KeyslotBlockType_eCa;
}

static BHSM_KeyslotPolarity _GetEntryPolarity( BHSM_KeyslotBlockEntry entry )
{
    BDBG_CASSERT( BHSM_KeyslotPolarity_eOdd == 0 );
    BDBG_CASSERT( BHSM_KeyslotPolarity_eEven == 1 );
    BDBG_CASSERT( BHSM_KeyslotPolarity_eClear == 2 );

    switch( entry )
    {
        case BHSM_KeyslotBlockEntry_eCpdOdd:
        case BHSM_KeyslotBlockEntry_eCaOdd:
        case BHSM_KeyslotBlockEntry_eCpsOdd:
        {
            return  BHSM_KeyslotPolarity_eOdd;
        }
        case BHSM_KeyslotBlockEntry_eCpdEven:
        case BHSM_KeyslotBlockEntry_eCaEven:
        case BHSM_KeyslotBlockEntry_eCpsEven:
        {
            return  BHSM_KeyslotPolarity_eEven;
        }
        case BHSM_KeyslotBlockEntry_eCpdClear:
        case BHSM_KeyslotBlockEntry_eCaClear:
        case BHSM_KeyslotBlockEntry_eCpsClear:
        {
            return  BHSM_KeyslotPolarity_eClear;
        }
        default: { BERR_TRACE( entry ); }
    }

    return BHSM_KeyslotPolarity_eClear;
}

uint8_t _convertSlotType( BHSM_KeyslotType type )
{

    switch ( type )
    {
        case BHSM_KeyslotType_eIvPerSlot:   return 0;
        case BHSM_KeyslotType_eIvPerBlock:  return 1;
        case BHSM_KeyslotType_eIvPerEntry:  return 3;  /* no 128bit per entry exists on zeus5 */
        case BHSM_KeyslotType_eIvPerBlock256:  return 2;
        case BHSM_KeyslotType_eIvPerEntry256:  return 3;  /* no 128bit per entry exists on zeus5 */
        default:  BERR_TRACE( type );
    }

    return 0;
}

BERR_Code BHSM_KeySlot_SetMulti2Key( BHSM_Handle hHsm, const BHSM_KeySlotSetMulti2Key *pKeyData )
{
    BERR_Code rc = BERR_SUCCESS;
    BHSM_P_KeySlotMulti2SysKeySet bspConfig;

    BDBG_ENTER( BHSM_KeySlot_SetMulti2Key );

    if( pKeyData == NULL ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }

    BKNI_Memset( &bspConfig, 0, sizeof(bspConfig) );

    BDBG_CASSERT( sizeof(bspConfig.in.systemKeys) == sizeof(pKeyData->key) );
    BHSM_Mem32cpy( (void*)bspConfig.in.systemKeys, (void*)pKeyData->key, sizeof(pKeyData->key) );
    bspConfig.in.whichSysKey = pKeyData->keySelect;

    rc = BHSM_P_KeySlot_Multi2SysKeySet( hHsm, &bspConfig );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE( rc ); }

    BDBG_LEAVE( BHSM_KeySlot_SetMulti2Key );
    return BERR_SUCCESS;
}

static BERR_Code _SetKeyslotOwnership( BHSM_KeyslotHandle handle, BHSM_SecurityCpuContext owner )
{
    BHSM_P_KeySlotSetOwnership ownership;
    BHSM_P_KeySlot *pSlot;
    BERR_Code rc = BERR_SUCCESS;

    if (!handle) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }

    pSlot = (BHSM_P_KeySlot*)handle;

    BDBG_MSG(("KeySlot Type[%d] Number[%u] to owner[%s]", pSlot->slotType, pSlot->number, _KeyslotOwnerToString(owner) ));

    BKNI_Memset( &ownership, 0, sizeof(ownership) );
    ownership.in.keySlotType = _convertSlotType( pSlot->slotType );
    ownership.in.keySlotNumber = pSlot->number;

    switch( owner ) {
        case BHSM_SecurityCpuContext_eHost: ownership.in.setKtsOwnership = Bsp_KeySlotOwnership_eShared; break;
        case BHSM_SecurityCpuContext_eSage: ownership.in.setKtsOwnership = Bsp_KeySlotOwnership_eSage; break;
        case BHSM_SecurityCpuContext_eNone: ownership.in.setKtsOwnership = Bsp_KeySlotOwnership_eFree; break;
        default:  return BERR_TRACE( BERR_INVALID_PARAMETER );
    }

    rc = BHSM_P_KeySlot_SetOwnership( _GetHsmHandle(handle), &ownership );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE(rc); }

    return BERR_SUCCESS;
}


static BERR_Code _GetKeyslotOwnership( BHSM_KeyslotHandle handle, BHSM_SecurityCpuContext* pOwner )
{
    BERR_Code rc = BERR_SUCCESS;
    BHSM_P_KeySlotQuery bspQuery;
    BHSM_P_KeySlot *pSlot = (BHSM_P_KeySlot*)handle;
    uint16_t index;

    if( !pOwner ) return BERR_TRACE( BERR_INVALID_PARAMETER );
    if( !pSlot ) return BERR_TRACE( BERR_INVALID_PARAMETER );
    if( pSlot->slotType >= BHSM_KeyslotType_eMax ) return BERR_TRACE( BERR_INVALID_PARAMETER );

    BKNI_Memset( &bspQuery, 0, sizeof(bspQuery) );

    rc = BHSM_P_KeySlot_Query( _GetHsmHandle(handle), &bspQuery );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE( rc ); }

    index = pSlot->pModule->types[pSlot->slotType].offset + pSlot->number;

    if( index >= sizeof(bspQuery.out.keySlotOwnership) ) return BERR_TRACE( BERR_INVALID_PARAMETER );

    switch( bspQuery.out.keySlotOwnership[index] )
    {
        case Bsp_KeySlotOwnership_eFree: *pOwner = BHSM_SecurityCpuContext_eNone; break;
        case Bsp_KeySlotOwnership_eSage: *pOwner = BHSM_SecurityCpuContext_eSage; break;
        case Bsp_KeySlotOwnership_eShared: *pOwner = BHSM_SecurityCpuContext_eHost; break;
        default:  return BERR_TRACE( BERR_INVALID_PARAMETER );
    }

    BDBG_MSG(( "Keyslot type[%d] Number[%u] owned by[%s]", pSlot->slotType, pSlot->number, _KeyslotOwnerToString(*pOwner) ));

    return BERR_SUCCESS;
}

#if BDBG_DEBUG_BUILD
static char* _KeyslotOwnerToString( BHSM_SecurityCpuContext owner )
{
    switch( owner )
    {
        case BHSM_SecurityCpuContext_eNone: return "None";
        case BHSM_SecurityCpuContext_eSage: return "Sage";
        case BHSM_SecurityCpuContext_eHost: return "Host";
        default: return "undetermined";
    }
}
#endif


/*  Conofigure a bypass keyslot that will allow GR to R transfers.
    This is a temporary function.  */
BERR_Code BHSM_InitialiseBypassKeyslots( BHSM_Handle hHsm )
{
    BERR_Code rc = BERR_UNKNOWN;
    BHSM_KeySlotModule* pModule;
    BHSM_KeyslotHandle hBypassKeyslot = NULL;
    BHSM_KeyslotAllocateSettings keyslotAllocSettings;
    BHSM_KeyslotSettings keyslotSettings;
    BHSM_KeyslotEntrySettings keyslotEntrySettings;
    BHSM_KeyslotKey keyslotKey;
    BHSM_KeyslotInfo keyslotInfo;
    unsigned i = 0;

    if( !hHsm->modules.pKeyslots ) { return BERR_TRACE( BERR_NOT_INITIALIZED ); }

    pModule = hHsm->modules.pKeyslots;
    hBypassKeyslot = pModule->hBypassKeyslot;

    /* allocate keyslot */
    if( hBypassKeyslot ){
        BDBG_WRN(("Bypass Keyslot already intialised." ));
        return BERR_SUCCESS;
    }

    BHSM_Keyslot_GetDefaultAllocateSettings( &keyslotAllocSettings );
    keyslotAllocSettings.owner = BHSM_SecurityCpuContext_eHost;
    keyslotAllocSettings.slotType = BHSM_KeyslotType_eIvPerSlot;
   #ifdef BHSM_BUILD_HSM_FOR_SAGE
    keyslotAllocSettings.keySlotNumber = 0;
   #endif
    hBypassKeyslot = BHSM_Keyslot_Allocate( hHsm, &keyslotAllocSettings );
    if( !hBypassKeyslot ) { rc = BERR_TRACE( BERR_NOT_AVAILABLE ); goto error; }

    rc = BHSM_GetKeySlotInfo( hBypassKeyslot, &keyslotInfo );
    if( rc != BERR_SUCCESS ) { rc = BERR_TRACE( BERR_UNKNOWN ); goto error; }

    /* we're assuming that the first allocated keyslot has number zero. */
    if( keyslotInfo.number != 0 ) { return BERR_TRACE( BERR_NOT_AVAILABLE ); }

    pModule->hBypassKeyslot = hBypassKeyslot;

    /* keyslot configuration. */
    BHSM_Keyslot_GetSettings( hBypassKeyslot, &keyslotSettings );

    /* allowed sources */
    keyslotSettings.regions.source[BHSM_SecurityRegion_eGlr] = true;                /* G  */
   #ifdef BHSM_BUILD_HSM_FOR_SAGE
    keyslotSettings.regions.source[BHSM_SecurityRegion_eCrr] = true;                /* R  */
   #endif
    /* allowed destinations */                                                      /* to */
    keyslotSettings.regions.destinationRPipe[BHSM_SecurityRegion_eCrr] = true;      /* R  */
    keyslotSettings.regions.destinationGPipe[BHSM_SecurityRegion_eCrr] = true;

    rc = BHSM_Keyslot_SetSettings( hBypassKeyslot, &keyslotSettings );
    if( rc != BERR_SUCCESS ) { rc = BERR_TRACE( BERR_NOT_AVAILABLE ); goto error; }

    /* entry configuration. We use the CPD **Clear** entry. */
    BHSM_Keyslot_GetEntrySettings( hBypassKeyslot, BHSM_KeyslotBlockEntry_eCpdClear, &keyslotEntrySettings );

    keyslotEntrySettings.algorithm = BHSM_CryptographicAlgorithm_eAes128; /* any alg compatiable with CPD.*/
    /* ensure that the Clear TS SC bits remain clear */
    keyslotEntrySettings.outputPolarity.specify = true;
    keyslotEntrySettings.outputPolarity.rPipe = BHSM_KeyslotPolarity_eClear;
    keyslotEntrySettings.outputPolarity.gPipe = BHSM_KeyslotPolarity_eClear;
    /* output TS SC bits */
    keyslotEntrySettings.rPipeEnable = false; /* don't apply decryption to transport packets. */
    keyslotEntrySettings.gPipeEnable = false;

    rc = BHSM_Keyslot_SetEntrySettings( hBypassKeyslot, BHSM_KeyslotBlockEntry_eCpdClear, &keyslotEntrySettings );
    if( rc != BERR_SUCCESS ) { rc = BERR_TRACE( BERR_NOT_AVAILABLE ); goto error; }

    BKNI_Memset( &keyslotKey, 0, sizeof(keyslotKey) );
    keyslotKey.size = 16;
    for( i = 0; i < keyslotKey.size; i++ ) { keyslotKey.key[i] = i; } /* random non zero key. */

    rc = BHSM_Keyslot_SetEntryKey( hBypassKeyslot, BHSM_KeyslotBlockEntry_eCpdClear, &keyslotKey );
    if( rc != BERR_SUCCESS ) { rc = BERR_TRACE( BERR_NOT_AVAILABLE ); goto error; }

    BDBG_MSG(("Bypass Keyslot initialised. Type[%d] Number[%u]", keyslotInfo.type, keyslotInfo.number ));

    return BERR_SUCCESS;

error:

    if( hBypassKeyslot ) BHSM_Keyslot_Free( hBypassKeyslot );
    pModule->hBypassKeyslot = NULL;

    return rc;
}


BERR_Code  BHSM_SetPidChannelBypassKeyslot( BHSM_Handle hHsm,
                                            unsigned pidChannelIndex,
                                            BHSM_BypassKeySlot_e bypassKeyslot )
{
    BERR_Code rc = BERR_UNKNOWN;
    BHSM_KeySlotModule* pModule = NULL;
    BHSM_KeyslotHandle hBypassKeyslot = NULL;

    pModule = hHsm->modules.pKeyslots;
    hBypassKeyslot = pModule->hBypassKeyslot;
    if( !hBypassKeyslot ) { return BERR_TRACE( BERR_NOT_AVAILABLE ); }

    if( bypassKeyslot == BHSM_BypassKeySlot_eGR2R ) {
        rc = BHSM_Keyslot_AddPidChannel( hBypassKeyslot, pidChannelIndex );
        if( rc != BERR_SUCCESS ) { return BERR_TRACE( BERR_NOT_AVAILABLE ); }
    }
    else {
        BHSM_P_KeySlotPidRemove bspRemovePid;

        BKNI_Memset( &bspRemovePid, 0, sizeof(bspRemovePid) );
        bspRemovePid.in.pidChanStart = pidChannelIndex;
        bspRemovePid.in.pidChanEnd = pidChannelIndex;

        rc = BHSM_P_KeySlot_PidRemove( hHsm, &bspRemovePid );
        if( rc != BERR_SUCCESS ) { return BERR_TRACE( rc ); }
    }

    return BERR_SUCCESS;
}

BERR_Code  BHSM_GetPidChannelBypassKeyslot( BHSM_Handle hHsm,
                                            unsigned pidChannelIndex,
                                            BHSM_BypassKeySlot_e *pBypassKeyslot )
{
    BSTD_UNUSED( hHsm );
    BSTD_UNUSED( pidChannelIndex );
    BSTD_UNUSED( pBypassKeyslot );

    BERR_TRACE( BERR_NOT_AVAILABLE ); /* not available for now! */

    return 0;
}

BERR_Code BHSM_P_Keyslot_GetDetails( BHSM_KeyslotHandle handle,
                                     BHSM_KeyslotBlockEntry entry,
                                     BHSM_KeyslotDetails *pDetails )
{
    BHSM_P_KeySlot *pSlot = (BHSM_P_KeySlot*)handle;

    BDBG_ENTER( BHSM_P_Keyslot_GetDetails );

    if( !pSlot->configured ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }

    BKNI_Memset(pDetails, 0, sizeof(BHSM_KeyslotDetails));

    pDetails->ctrlWord0 = compileControl0_GlobalHi( handle );
    pDetails->ctrlWord1 = compileControl1_GlobalLo( handle );

    pDetails->slotType = pSlot->slotType;
    pDetails->number =  pSlot->number;

    pDetails->hHsm = pSlot->pModule->hHsm;

    if(entry < BHSM_KeyslotBlockEntry_eMax)
    {
        BHSM_P_KeyEntry *pEntry = _GetEntry( handle, entry );

        if( !pEntry->configured ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }

        pDetails->ctrlWord2 = compileControl2_ModeHi( handle, entry );
        pDetails->ctrlWord3 = compileControl3_ModeLo( handle, entry );

        pDetails->polarity = _GetEntryPolarity( entry );
        pDetails->blockType = _GetEntryBlockType( entry );

        if( pEntry->settings.external.iv )
        {
            pDetails->externalIvValid = true;
            pDetails->externalIvOffset = pEntry->pExternalSlot->offsetIv + pEntry->pExternalSlot->slotPtr;
        }

        if( pDetails->blockType >= BHSM_KeyslotBlockType_eMax )  { return BERR_TRACE( BERR_INVALID_PARAMETER ); }

        pDetails->sc01.useEntry = pSlot->settings.sc01[pDetails->blockType].useEntry;
        pDetails->sc01.outputPolarity.rPipe = pSlot->settings.sc01[pDetails->blockType].outputPolarity.rPipe;
        pDetails->sc01.outputPolarity.gPipe = pSlot->settings.sc01[pDetails->blockType].outputPolarity.gPipe;
    }
    else
    {
        pDetails->polarity = BHSM_KeyslotPolarity_eMax;
        pDetails->blockType = BHSM_KeyslotBlockType_eMax;
    }

    BDBG_LEAVE( BHSM_P_Keyslot_GetDetails );
    return BERR_SUCCESS;
}
