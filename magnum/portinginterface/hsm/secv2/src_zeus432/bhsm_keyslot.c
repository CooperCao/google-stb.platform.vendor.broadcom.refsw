/******************************************************************************
 *  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#include "bsp_s_keycommon.h"

BDBG_MODULE(BHSM);

#define BHSM_MAX_KEYSLOTS 1024 /* TODO move to headerfile  */

typedef struct
{
    bool configured;
    BHSM_KeyslotEntrySettings settings; /* keyslot entry settings. */
    struct{
        unsigned key;
        unsigned iv;
    } externalOffset;

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

    BHSM_P_KeySlot *pSlotHandles[BHSM_MAX_KEYSLOTS]; /* array of dynamically allocated keyslots */

    struct{
        unsigned offset;     /* offset into pSlotHandles of first slot of this type */
        unsigned maxNumber;  /* number of slots of this type. */
    }types[BHSM_KeyslotType_eMax];

}BHSM_KeySlotModule;


static BHSM_Handle _GetHsmHandle( BHSM_KeyslotHandle handle );
static BERR_Code _SetEntryKey( BHSM_KeyslotHandle handle, BHSM_KeyslotBlockEntry entry, const BHSM_KeyslotKey *pKey );
static BERR_Code _SetEntryIv( BHSM_KeyslotHandle handle, BHSM_KeyslotBlockEntry entry, const BHSM_KeyslotIv *pIv, const BHSM_KeyslotIv* pIv2);
static BERR_Code _InvalidateSlot( BHSM_KeyslotHandle handle );
static BERR_Code _InvalidateEntry( BHSM_KeyslotHandle handle, BHSM_KeyslotBlockEntry entry );
static BHSM_KeyslotSettings* _GetSlotSetting( BHSM_KeyslotHandle handle );
static BHSM_P_KeyEntry* _GetEntry( BHSM_KeyslotHandle handle, BHSM_KeyslotBlockEntry entry );
static BHSM_KeyslotBlockType _GetEntryBlockType( BHSM_KeyslotBlockEntry entry );
static BHSM_KeyslotPolarity _GetEntryPolarity( BHSM_KeyslotBlockEntry entry );
static BERR_Code compileControl0_GlobalHi( BHSM_KeyslotHandle handle );
static BERR_Code compileControl1_GlobalLo( BHSM_KeyslotHandle handle );
static BERR_Code compileControl2_ModeHi( BHSM_KeyslotHandle handle, BHSM_KeyslotBlockEntry entry );
static BERR_Code compileControl3_ModeLo( BHSM_KeyslotHandle handle, BHSM_KeyslotBlockEntry entry );
static BERR_Code _Keyslot_Init( BHSM_Handle hHsm, BHSM_KeyslotModuleSettings *pSettings );
static uint8_t _convertSlotType( BHSM_KeyslotType type );
#ifdef BHSM_BUILD_HSM_FOR_SAGE
static BERR_Code _Keyslot_LoadConfigurationFromBsp( BHSM_Handle hHsm );
#endif

#if 0 /* will delete at a later point. */
static void  _DumpEntry( const char* pString, BHSM_KeyslotHandle handle, BHSM_KeyslotBlockEntry entry );
static void  _DumpSlot( const char* pString, BHSM_KeyslotHandle handle );
#endif

BERR_Code BHSM_Keyslot_Init( BHSM_Handle hHsm, BHSM_KeyslotModuleSettings *pSettings )
{
    BERR_Code rc = BERR_SUCCESS;
    BHSM_KeySlotModule* pModule;
    unsigned i;
    unsigned count = 0;

    BDBG_ENTER( BHSM_Keyslot_Init );

    if( hHsm->modules.pKeyslots ) {
        return BERR_TRACE( BERR_INVALID_PARAMETER ); /* keyslot module aready initialised. */
    }

    pModule = (BHSM_KeySlotModule*)BKNI_Malloc( sizeof(BHSM_KeySlotModule) );
    if( !pModule ) {
        return BERR_TRACE( BERR_OUT_OF_SYSTEM_MEMORY );
    }
    BKNI_Memset( pModule, 0, sizeof(BHSM_KeySlotModule) );

    pModule->hHsm = hHsm;
    hHsm->modules.pKeyslots = pModule;

   #ifndef BHSM_BUILD_HSM_FOR_SAGE
    rc =  _Keyslot_Init( hHsm, pSettings );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE(rc); }

    for( i = 0; i < BHSM_KeyslotType_eMax; i++ ){
        pModule->types[i].offset = count;
        pModule->types[i].maxNumber = pSettings->numKeySlotsForType[i];
        count += pSettings->numKeySlotsForType[i];
    }
   #else
    rc =  _Keyslot_LoadConfigurationFromBsp( hHsm );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE(rc); }
   #endif

    BDBG_LEAVE( BHSM_Keyslot_Init );
    return BERR_SUCCESS;
}


void BHSM_Keyslot_Uninit( BHSM_Handle hHsm )
{
    BDBG_ENTER( BHSM_Keyslot_Uninit );

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
    BHSM_P_KeySlot *pSlot = NULL;
    unsigned offset; /* offset to keyslot type */
    unsigned maxNumber;
    unsigned i;

    BDBG_ENTER( BHSM_Keyslot_Allocate );

    if( !pSettings ) { BERR_TRACE( BERR_INVALID_PARAMETER ); return NULL; }
    if( pSettings->slotType >= BHSM_KeyslotType_eMax ) { BERR_TRACE( BERR_INVALID_PARAMETER ); return NULL; }

    pModule = hHsm->modules.pKeyslots;

    /* find a free slot. */
    offset    = pModule->types[pSettings->slotType].offset;
    maxNumber = pModule->types[pSettings->slotType].maxNumber;

   #ifndef BHSM_BUILD_HSM_FOR_SAGE
    for( i = offset; i < offset+maxNumber; i++ )
    {
        if( pModule->pSlotHandles[i] == NULL )
        {
            pSlot = BKNI_Malloc( sizeof(BHSM_P_KeySlot) );
            if ( !pSlot ) { BERR_TRACE( BERR_OUT_OF_SYSTEM_MEMORY ); return NULL; }

            BKNI_Memset( pSlot, 0, sizeof(*pSlot) );
            pSlot->slotType = pSettings->slotType;
            pSlot->number = i-offset;
            pSlot->pModule = pModule;

            pModule->pSlotHandles[i] = pSlot;
            break;
        }
    }
    if( !pSlot ){
        BDBG_ERR(("Failed to allocate keyslot. Type[%d] offset[%d] num[%d]", pSettings->slotType, offset, maxNumber ));
        return NULL;
    }
   #else /* we're on SAGE*/

    if( keySlotNumber >= maxNumber ){
        BERR_TRACE( BERR_INVALID_PARAMETER );
        return NULL; /* slot number greater that number reserved for that type. */
    }

    if( pModule->pSlotHandles[offset+keySlotNumber] != NULL ){
            BERR_TRACE( BERR_INVALID_PARAMETER );
            return NULL;  /* specified slot (type,number) is not available. */
    }

    pSlot = BKNI_Malloc( sizeof(BHSM_P_KeySlot) );
    if( !pSlot ) { BERR_TRACE( BERR_OUT_OF_SYSTEM_MEMORY ); return NULL; }

    BKNI_Memset( pSlot, 0, sizeof(*pSlot) );
    pSlot->slotType = pSettings->slotType;
    pSlot->number = keySlotNumbe;
    pSlot->pModule = pModule;

    pModule->pSlotHandles[offset+keySlotNumber] = pSlot;

   #endif

    BDBG_LEAVE( BHSM_Keyslot_Allocate );
    return (BHSM_KeyslotHandle)pSlot;
}

void BHSM_Keyslot_Free( BHSM_KeyslotHandle handle )
{
    unsigned offset;
    BHSM_P_KeySlot *pSlot;

    BDBG_ENTER( BHSM_Keyslot_Free );

    pSlot = (BHSM_P_KeySlot*)handle;

    if( pSlot->slotType >= BHSM_KeyslotType_eMax ) { BERR_TRACE( BERR_INVALID_PARAMETER ); return; }

    offset = pSlot->pModule->types[pSlot->slotType].offset + pSlot->number;
    pSlot->pModule->pSlotHandles[offset] = NULL;

    BKNI_Free( pSlot );

    BDBG_LEAVE( BHSM_Keyslot_Free );
    return;
}

void  BHSM_Keyslot_GetSettings( BHSM_KeyslotHandle handle, BHSM_KeyslotSettings *pSettings )
{
    BHSM_P_KeySlot *pSlot = (BHSM_P_KeySlot*)handle;

    BDBG_ENTER( BHSM_Keyslot_GetSettings );

    if( !pSettings ) { BERR_TRACE( BERR_INVALID_PARAMETER ); return; }

    /*  *pSettings = pSlot->settings;  */
    BSTD_UNUSED(pSlot); /*TODO .. return setting if configured. */

    BKNI_Memset( pSettings, 0, sizeof(*pSettings) );

    BDBG_LEAVE( BHSM_Keyslot_GetSettings );
    return;
}

BERR_Code BHSM_Keyslot_SetSettings( BHSM_KeyslotHandle handle, const BHSM_KeyslotSettings *pSettings )
{
    BHSM_P_KeySlot *pSlot = (BHSM_P_KeySlot*)handle;

    BDBG_ENTER( BHSM_Keyslot_SetSettings );

    if( !pSettings ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }

   /* _DumpSlot( BSTD_FUNCTION, handle );*/

    pSlot->settings = *pSettings;
    pSlot->configured = true;

   /* _DumpSlot( BSTD_FUNCTION, handle );*/

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

    /* _DumpEntry(BSTD_FUNCTION, handle, entry ); */

    pEntry = _GetEntry( handle, entry );

    if( pEntry->configured )
    {
        *pSettings = pEntry->settings;
    }
    else
    {
        BKNI_Memset( pSettings, 0, sizeof(*pSettings) );
    }

    BDBG_LEAVE( BHSM_Keyslot_GetEntrySettings );
    return;
}


BERR_Code BHSM_Keyslot_SetEntrySettings( BHSM_KeyslotHandle handle,
                                         BHSM_KeyslotBlockEntry entry,
                                         const BHSM_KeyslotEntrySettings *pSettings )
{
    BHSM_P_KeyEntry *pEntry;

    BDBG_ENTER( BHSM_Keyslot_SetEntrySettings );

    /* _DumpEntry( BSTD_FUNCTION, handle, entry );*/

    if( !pSettings )                           { return BERR_TRACE( BERR_INVALID_PARAMETER ); }
    if( entry >= BHSM_KeyslotBlockEntry_eMax ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }
    pEntry = _GetEntry( handle, entry );

    pEntry->settings = *pSettings;
    pEntry->configured = true;

   /* _DumpEntry(BSTD_FUNCTION , handle, entry ); */

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
    BSTD_UNUSED(pIv);
    BSTD_UNUSED(pIv2);

    if( !pIv && !pIv2 )                        { return BERR_TRACE( BERR_INVALID_PARAMETER ); } /* we need oen IV */
    if( entry >= BHSM_KeyslotBlockEntry_eMax ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }

    if( pSlot->configured == false )              { return BERR_TRACE( BERR_INVALID_PARAMETER ); }
    if( pSlot->entry[entry].configured == false ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }

    /* TODO .. compile command and write key for Iv2 etc. */
    rc = _SetEntryIv( handle, entry, pIv, pIv2);
    if( rc != BERR_SUCCESS ) return BERR_TRACE(rc);

    BDBG_LEAVE( BHSM_Keyslot_SetEntryIv );
    return BERR_SUCCESS;
}


BERR_Code BHSM_Keyslot_Invalidate( BHSM_KeyslotHandle handle )
{
    BERR_Code rc = BERR_SUCCESS;
    BDBG_ENTER( BHSM_Keyslot_Invalidate );
    BSTD_UNUSED(handle);

    rc = _InvalidateSlot( handle );
    if( rc != BERR_SUCCESS ) return BERR_TRACE(rc);

    BDBG_LEAVE( BHSM_Keyslot_Invalidate );
    return BERR_SUCCESS;
}


BERR_Code BHSM_Keyslot_GetEntryExternalKeySettings( BHSM_KeyslotHandle handle,
                                                    BHSM_KeyslotBlockEntry entry,
                                                    BHSM_KeyslotExternalKeyData *pData )
{
    BHSM_P_KeySlot *pSlot = (BHSM_P_KeySlot*)handle;

    BDBG_ENTER( BHSM_Keyslot_GetEntryExternalKeySettings );

    if( !pData )                               { return BERR_TRACE( BERR_INVALID_PARAMETER ); }
    if( entry >= BHSM_KeyslotBlockEntry_eMax ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }

    if( pSlot->entry[entry].configured )
    {

    }

    BERR_TRACE( BERR_INVALID_PARAMETER );


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
    BERR_Code           rc = BERR_SUCCESS;
    BHSM_BspMsg_h       hMsg = NULL;
    BHSM_BspMsgHeader_t header;
    BHSM_P_KeySlot *pSlot = (BHSM_P_KeySlot*)handle;
    uint32_t            tmp = 0;
    uint8_t             status = 0;

    BDBG_ENTER( BHSM_Keyslot_AddPidChannel );

    if( !pSlot ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }

    rc = BHSM_BspMsg_Create( _GetHsmHandle(handle), &hMsg );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE( rc ); }

    BHSM_BspMsg_GetDefaultHeader( &header );
    BHSM_BspMsg_Header( hMsg, BCMD_cmdType_eSESSION_CONFIG_PIDKEYPOINTERTABLE, &header );

    tmp = pidChannelIndex;
    if( pSettings && pSettings->secondary ){ tmp |= ( 1 << 31 ); }
    BHSM_BspMsg_Pack32( hMsg, BCMD_KeyPointer_InCmdCfg_ePidChan, tmp );

    BHSM_BspMsg_Pack8( hMsg, BCMD_KeyPointer_InCmdCfg_eSlotType, _convertSlotType(pSlot->slotType) );
    BHSM_BspMsg_Pack8( hMsg, BCMD_KeyPointer_InCmdCfg_eSlotNumber, pSlot->number );

    if( pSettings && pSettings->secondary )
    {
        BHSM_BspMsg_Pack8( hMsg, BCMD_KeyPointer_InCmdCfg_eSlotTypeB, _convertSlotType(pSlot->slotType) );
        BHSM_BspMsg_Pack8( hMsg, BCMD_KeyPointer_InCmdCfg_eSlotNumberB, pSlot->number );
    }

    BHSM_BspMsg_Pack8( hMsg, BCMD_KeyPointer_InCmdCfg_eKeyPointerSel, 0 );

    rc = BHSM_BspMsg_SubmitCommand ( hMsg );
    if( rc != BERR_SUCCESS ) { BERR_TRACE( rc); goto BHSM_P_DONE_LABEL; }

    BHSM_BspMsg_Get8( hMsg, BCMD_CommonBufferFields_eStatus, &status );
    if( status != 0 ) {
        rc = BHSM_STATUS_BSP_ERROR;
        BDBG_ERR(("%s BSP Status error [0x%X]", BSTD_FUNCTION, status ));
        goto BHSM_P_DONE_LABEL;
    }

BHSM_P_DONE_LABEL:

    (void)BHSM_BspMsg_Destroy( hMsg );

    BDBG_LEAVE( BHSM_Keyslot_AddPidChannel );
    return rc;
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
    BHSM_BspMsg_h       hMsg = NULL;
    BHSM_BspMsgHeader_t header;
    BHSM_P_KeySlot *pSlot = (BHSM_P_KeySlot*)handle;
    uint32_t            tmp = 0;
    uint8_t             status = 0;

    BDBG_ENTER( BHSM_Keyslot_RemovePidChannel );

    if( !pSlot ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }

    rc = BHSM_BspMsg_Create( _GetHsmHandle(handle), &hMsg );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE( rc ); }

    BHSM_BspMsg_GetDefaultHeader( &header );
    BHSM_BspMsg_Header( hMsg, BCMD_cmdType_eSESSION_CONFIG_PIDKEYPOINTERTABLE, &header );

    tmp = pidChannelIndex;
    tmp |= (1 << 30); /* reset PID to default entry */

    BHSM_BspMsg_Pack32( hMsg, BCMD_KeyPointer_InCmdCfg_ePidChan, tmp );

    BHSM_BspMsg_Pack8( hMsg, BCMD_KeyPointer_InCmdCfg_eSlotType, _convertSlotType(pSlot->slotType) );
    BHSM_BspMsg_Pack8( hMsg, BCMD_KeyPointer_InCmdCfg_eSlotNumber, pSlot->number );

    BHSM_BspMsg_Pack8( hMsg, BCMD_KeyPointer_InCmdCfg_eKeyPointerSel, 0 );

    rc = BHSM_BspMsg_SubmitCommand ( hMsg );
    if( rc != BERR_SUCCESS ) { BERR_TRACE( rc); goto BHSM_P_DONE_LABEL; }

    BHSM_BspMsg_Get8( hMsg, BCMD_CommonBufferFields_eStatus, &status );
    if( status != 0 ) {
        rc = BHSM_STATUS_BSP_ERROR;
        BDBG_ERR(("%s BSP Status error [0x%X]", BSTD_FUNCTION, status ));
        goto BHSM_P_DONE_LABEL;
    }
BHSM_P_DONE_LABEL:

    (void)BHSM_BspMsg_Destroy( hMsg );

    BDBG_LEAVE( BHSM_Keyslot_RemovePidChannel );
    return rc;
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
    BHSM_BspMsg_h hMsg = NULL;
    BHSM_BspMsgHeader_t header;
    BHSM_P_KeySlot *pSlot = (BHSM_P_KeySlot*)handle;
    BHSM_P_KeyEntry *pEntry;
    unsigned keyOffset = 0;
    uint8_t status = 0;
    BCMD_KeySize_e keySize;

    BDBG_ENTER( _SetEntryIv );

    pEntry = _GetEntry( handle, entry );
    if( pEntry == NULL ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }

    rc = BHSM_BspMsg_Create( _GetHsmHandle(handle), &hMsg );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE( rc ); }

    BHSM_BspMsg_GetDefaultHeader( &header );
    BHSM_BspMsg_Header( hMsg, BCMD_cmdType_eSESSION_LOAD_ROUTE_USERKEY, &header );

    switch( pIv->size )
    {
        case 16: { keySize = BCMD_KeySize_e128; break;}
        default: return BERR_TRACE( BERR_INVALID_PARAMETER );
    }

    keyOffset = (8-(2*(keySize+1))) * 4; /* offset of key in bytes */

    BHSM_BspMsg_Pack8 ( hMsg, BCMD_LoadUseKey_InCmd_eKeyLength, (uint16_t)keySize );

    BHSM_BspMsg_PackArray(  hMsg,
                            BCMD_LoadUseKey_InCmd_eKeyData+keyOffset,   /* offset */
                            pIv->iv,                                    /* data   */
                            pIv->size                                   /* length */ );

    if (pIv2) {
        keyOffset -= (8-(2*(keySize+1))) * 4; /* offset of higher key in bytes */
        BHSM_BspMsg_PackArray(  hMsg,
                                BCMD_LoadUseKey_InCmd_eKeyData+keyOffset,   /* offset */
                                pIv2->iv,                                    /* data   */
                                pIv2->size                                   /* length */ );
    }

    BHSM_BspMsg_Pack8( hMsg, BCMD_LoadUseKey_InCmd_eRouteKeyFlag, 1 );

    BDBG_CASSERT( BHSM_KeyslotBlockType_eCpd == 0 );
    BDBG_CASSERT( BHSM_KeyslotBlockType_eCa  == 1 );
    BDBG_CASSERT( BHSM_KeyslotBlockType_eCps == 2 );

    BDBG_CASSERT( BHSM_KeyslotPolarity_eOdd  == 0 );
    BDBG_CASSERT( BHSM_KeyslotPolarity_eEven == 1 );
    BDBG_CASSERT( BHSM_KeyslotPolarity_eClear == 2 );

    BHSM_BspMsg_Pack8( hMsg, BCMD_LoadUseKey_InCmd_eBlkType, _GetEntryBlockType(entry) );
    BHSM_BspMsg_Pack8( hMsg, BCMD_LoadUseKey_InCmd_eEntryType, _GetEntryPolarity(entry) );

    BHSM_BspMsg_Pack8( hMsg, BCMD_LoadUseKey_InCmd_eIVType, BCMD_KeyDestIVType_eIV ); /* its a key! */
    BHSM_BspMsg_Pack8( hMsg, BCMD_LoadUseKey_InCmd_eKeySlotType, _convertSlotType(pSlot->slotType) );
    BHSM_BspMsg_Pack8( hMsg, BCMD_LoadUseKey_InCmd_eKeySlotNumber, pSlot->number );

    BHSM_BspMsg_Pack8( hMsg, BCMD_LoadUseKey_InCmd_eKeyMode, BCMD_KeyMode_eRegular ); /* TODO. Expand. */



    rc = BHSM_BspMsg_SubmitCommand ( hMsg );
    if( rc != BERR_SUCCESS ) { BERR_TRACE( rc ); goto _DONE_LABEL; }

    BHSM_BspMsg_Get8( hMsg, BCMD_CommonBufferFields_eStatus, &status );
    if( status != BERR_SUCCESS ) {
        BDBG_ERR(("%s BSP status[0x%02X] error", BSTD_FUNCTION, status ));
        rc = BERR_TRACE(BHSM_STATUS_BSP_ERROR);
        goto _DONE_LABEL;
    }

_DONE_LABEL:

    (void)BHSM_BspMsg_Destroy( hMsg );

    BDBG_LEAVE( _SetEntryIv );
    return rc;
}

static BERR_Code _SetEntryKey( BHSM_KeyslotHandle handle, BHSM_KeyslotBlockEntry entry, const BHSM_KeyslotKey *pKey )
{
    BERR_Code rc = BERR_SUCCESS;
    BHSM_BspMsg_h hMsg = NULL;
    BHSM_BspMsgHeader_t header;
    BHSM_P_KeySlot *pSlot = (BHSM_P_KeySlot*)handle;
    BHSM_P_KeyEntry *pEntry;
    unsigned keyOffset = 0;
    uint8_t status = 0;
    BCMD_KeySize_e keySize;

    BDBG_ENTER( _SetEntryKey );

   /*  _DumpEntry( BSTD_FUNCTION, handle, entry );*/

    pEntry = _GetEntry( handle, entry );
    if( pEntry == NULL ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }

    rc = BHSM_BspMsg_Create( _GetHsmHandle(handle), &hMsg );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE( rc ); }

    BHSM_BspMsg_GetDefaultHeader( &header );
    BHSM_BspMsg_Header( hMsg, BCMD_cmdType_eSESSION_LOAD_ROUTE_USERKEY, &header );

    switch( pKey->size )
    {
        case 8:  { keySize = BCMD_KeySize_e64; break;}
        case 16: { keySize = BCMD_KeySize_e128; break;}
        case 24: { keySize = BCMD_KeySize_e192; break;}
        case 32: { keySize = BCMD_KeySize_e256; break;}
        default: return BERR_TRACE( BERR_INVALID_PARAMETER );
    }

   #if 0
    keySize = (( pKey->size * 2 ) + 2) * 4;     /* size of algorithm in bytes*/
   #endif
    keyOffset = (8-(2*(keySize+1))) * 4; /* offset of key in bytes */

    BHSM_BspMsg_Pack8 ( hMsg, BCMD_LoadUseKey_InCmd_eKeyLength, (uint16_t)keySize );

    BHSM_BspMsg_PackArray(  hMsg,
                            BCMD_LoadUseKey_InCmd_eKeyData+keyOffset,   /* offset */
                            pKey->key,                                  /* data   */
                            pKey->size                                  /* length */ );

    BHSM_BspMsg_Pack8( hMsg, BCMD_LoadUseKey_InCmd_eRouteKeyFlag, 1 );

    BDBG_CASSERT( BHSM_KeyslotBlockType_eCpd == 0 );
    BDBG_CASSERT( BHSM_KeyslotBlockType_eCa  == 1 );
    BDBG_CASSERT( BHSM_KeyslotBlockType_eCps == 2 );

    BDBG_CASSERT( BHSM_KeyslotPolarity_eOdd  == 0 );
    BDBG_CASSERT( BHSM_KeyslotPolarity_eEven == 1 );
    BDBG_CASSERT( BHSM_KeyslotPolarity_eClear == 2 );

    BHSM_BspMsg_Pack8( hMsg, BCMD_LoadUseKey_InCmd_eBlkType, _GetEntryBlockType(entry) );
    BHSM_BspMsg_Pack8( hMsg, BCMD_LoadUseKey_InCmd_eEntryType, _GetEntryPolarity(entry) );

    BHSM_BspMsg_Pack8( hMsg, BCMD_LoadUseKey_InCmd_eIVType, BCMD_KeyDestIVType_eNoIV ); /* its a key! */
    BHSM_BspMsg_Pack8( hMsg, BCMD_LoadUseKey_InCmd_eKeySlotType, _convertSlotType(pSlot->slotType) );
    BHSM_BspMsg_Pack8( hMsg, BCMD_LoadUseKey_InCmd_eKeySlotNumber, pSlot->number );

#if 0
   #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(3,0)
    BHSM_BspMsg_Pack8( hMsg, BCMD_LoadUseKey_InCmd_eSC01ModeWordMapping,inoutp_loadRouteUserKeyIO->SC01ModeMapping & 0xFF );
   #endif

   #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(2,0)
    BHSM_BspMsg_Pack8( hMsg, BCMD_LoadUseKey_InCmd_eGPipeSC01Value, inoutp_loadRouteUserKeyIO->GpipeSC01Val & 0x03 );
    BHSM_BspMsg_Pack8( hMsg, BCMD_LoadUseKey_InCmd_eRPipeSC01Value, inoutp_loadRouteUserKeyIO->RpipeSC01Val & 0x03 );
   #endif
#endif
    BHSM_BspMsg_Pack8( hMsg, BCMD_LoadUseKey_InCmd_eKeyMode, BCMD_KeyMode_eRegular ); /* TODO. Expand. */


   #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,0)
    BHSM_BspMsg_Pack32( hMsg, BCMD_LoadUseKey_InCmd_eCtrlWord0, compileControl0_GlobalHi( handle ) );
    BHSM_BspMsg_Pack32( hMsg, BCMD_LoadUseKey_InCmd_eCtrlWord1, compileControl1_GlobalLo( handle ) );
   #endif
    BHSM_BspMsg_Pack32( hMsg, BCMD_LoadUseKey_InCmd_eCtrlWord2, compileControl2_ModeHi( handle, entry ) );
    BHSM_BspMsg_Pack32( hMsg, BCMD_LoadUseKey_InCmd_eCtrlWord3, compileControl3_ModeLo( handle, entry ) );

   #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(3,0)
    if( pEntry->settings.external.key ) {
        BHSM_BspMsg_Pack16( hMsg, BCMD_LoadUseKey_InCmd_eExtKeyPtr, 0 /*TODO offset*/ );
    }

    if( pEntry->settings.external.iv ) {
        BHSM_BspMsg_Pack16( hMsg, BCMD_LoadUseKey_InCmd_eExtIVPtr, 0 /* TODO offset */  );
    }
   #endif

    rc = BHSM_BspMsg_SubmitCommand ( hMsg );
    if( rc != BERR_SUCCESS ) { BERR_TRACE( rc ); goto _DONE_LABEL; }

    BHSM_BspMsg_Get8( hMsg, BCMD_CommonBufferFields_eStatus, &status );
    if( status != BERR_SUCCESS ) {
        BDBG_ERR(("%s BSP status[0x%02X] error", BSTD_FUNCTION, status ));
        rc = BERR_TRACE(BHSM_STATUS_BSP_ERROR);
        goto _DONE_LABEL;
    }

_DONE_LABEL:

    (void)BHSM_BspMsg_Destroy( hMsg );

    BDBG_LEAVE( _SetEntryKey );
    return rc;
}

static BERR_Code _InvalidateSlot( BHSM_KeyslotHandle handle )
{
    BERR_Code rc = BERR_SUCCESS;
    BHSM_BspMsg_h hMsg = NULL;
    BHSM_BspMsgHeader_t header;
    BHSM_P_KeySlot *pSlot = (BHSM_P_KeySlot*)handle;
    uint8_t status;

    BDBG_ENTER( _InvalidateSlot );

    if( !pSlot ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }

    rc = BHSM_BspMsg_Create( _GetHsmHandle(handle), &hMsg );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE(rc); }

    BHSM_BspMsg_GetDefaultHeader( &header );
    BHSM_BspMsg_Header( hMsg, BCMD_cmdType_eSESSION_INVALIDATE_KEY, &header );

    BHSM_BspMsg_Pack8( hMsg, BCMD_InvalidateKey_InCmd_eKeyFlag, BCMD_InvalidateKey_Flag_eDestKeyOnly );
    BHSM_BspMsg_Pack8( hMsg, BCMD_InvalidateKey_InCmd_eIsAllKTS, 1 );
    BHSM_BspMsg_Pack8( hMsg, BCMD_InvalidateKey_InCmd_eKeySlotType, _convertSlotType(pSlot->slotType) );
    BHSM_BspMsg_Pack8( hMsg, BCMD_InvalidateKey_InCmd_eKeySlotNumber, pSlot->number );

    rc = BHSM_BspMsg_SubmitCommand( hMsg );
    if( rc != BERR_SUCCESS ) { BERR_TRACE( rc ); goto BHSM_P_DONE_LABEL; }

    BHSM_BspMsg_Get8( hMsg, BCMD_CommonBufferFields_eStatus, &status );
    if( status != 0 ) {
        BDBG_ERR(("%s BSP error status[0x%02X]. type[%d] number[%d]", BSTD_FUNCTION, status,
                                                                     _convertSlotType(pSlot->slotType),
                                                                     pSlot->number ));

        rc =  BERR_TRACE(BHSM_STATUS_BSP_ERROR);
        goto BHSM_P_DONE_LABEL;
    }

BHSM_P_DONE_LABEL:

    (void)BHSM_BspMsg_Destroy( hMsg );

    BDBG_LEAVE( _InvalidateSlot );
    return rc;
}

static BERR_Code _InvalidateEntry( BHSM_KeyslotHandle handle, BHSM_KeyslotBlockEntry entry )
{
    BERR_Code rc = BERR_SUCCESS;
    BHSM_BspMsg_h hMsg = NULL;
    BHSM_BspMsgHeader_t header;
    BHSM_P_KeySlot *pSlot = (BHSM_P_KeySlot*)handle;
    uint8_t status;
    uint8_t block = 0;
    uint8_t polarity = 0;

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

    rc = BHSM_BspMsg_Create( _GetHsmHandle(handle), &hMsg );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE(rc); }

    BHSM_BspMsg_GetDefaultHeader( &header );
    BHSM_BspMsg_Header( hMsg, BCMD_cmdType_eSESSION_INVALIDATE_KEY, &header );

    BHSM_BspMsg_Pack8( hMsg, BCMD_InvalidateKey_InCmd_eKeyFlag, BCMD_InvalidateKey_Flag_eDestKeyOnly );
    BHSM_BspMsg_Pack8( hMsg, BCMD_InvalidateKey_InCmd_eBlkType, block );
    BHSM_BspMsg_Pack8( hMsg, BCMD_InvalidateKey_InCmd_eEntryType, polarity );
    BHSM_BspMsg_Pack8( hMsg, BCMD_InvalidateKey_InCmd_eKeySlotType, _convertSlotType(pSlot->slotType) );
    BHSM_BspMsg_Pack8( hMsg, BCMD_InvalidateKey_InCmd_eKeySlotNumber, pSlot->number );

    rc = BHSM_BspMsg_SubmitCommand( hMsg );
    if( rc != BERR_SUCCESS ) { BERR_TRACE( rc ); goto BHSM_P_DONE_LABEL; }

    BHSM_BspMsg_Get8( hMsg, BCMD_CommonBufferFields_eStatus, &status );
    if( status != 0 ) {
        BDBG_ERR(("%s BSP error status[0x%02X]. type[%d] number[%d]", BSTD_FUNCTION, status,
                                                                     _convertSlotType(pSlot->slotType),
                                                                     pSlot->number ));

        rc =  BERR_TRACE(BHSM_STATUS_BSP_ERROR);
        goto BHSM_P_DONE_LABEL;
    }

BHSM_P_DONE_LABEL:

    (void)BHSM_BspMsg_Destroy( hMsg );

    BDBG_LEAVE( _InvalidateEntry );
    return rc;
}

static uint32_t compileControl0_GlobalHi( BHSM_KeyslotHandle handle )
{
/*     BDBG_ENTER( compileControl0_GlobalHi );*/
    BSTD_UNUSED( handle );
    /* nothing! */
/*    BDBG_LEAVE( compileControl0_GlobalHi );*/
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
    if( pEntry->settings.external.iv )  { control |= (1 << 1); }  /* 1 bit. b01 */

    /* pack RPIPE_EN */
    if( pEntry->settings.rPipeEnable )       { control |= (1 << 2); }  /* 1 bit. b02 */

    /* pack GPIPE_EN */
    if( pEntry->settings.gPipeEnable )       { control |= (1 << 3); }  /* 1 bit. b03 */

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
        control |= ( ( pEntry->settings.multi2.roundCount & 0x7)  << 20 );       /* 3 bit. b20-22 */
        control |= ( ( pEntry->settings.multi2.keySelect  & 0x3F) << 14 );       /* 6 bit. b14-19 */
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
        case BHSM_CryptographicAlgorithm_eMulti2:    { algorithm = 0x1; break; }
        case BHSM_CryptographicAlgorithm_eDes:       { algorithm = 0x2; break; }
        case BHSM_CryptographicAlgorithm_e3DesAba:   { algorithm = 0x3; break; }
        case BHSM_CryptographicAlgorithm_e3DesAbc:   { algorithm = 0x4; break; }
        case BHSM_CryptographicAlgorithm_eDvbCsa3:   { algorithm = 0x5; break; }
        case BHSM_CryptographicAlgorithm_eAes128:    { algorithm = 0x6; break; }
        case BHSM_CryptographicAlgorithm_eAes192:    { algorithm = 0x7; break; }
        case BHSM_CryptographicAlgorithm_eC2:       { algorithm = 0x9; break; }
        case BHSM_CryptographicAlgorithm_eCss:      { algorithm = 0xa; break; }
        case BHSM_CryptographicAlgorithm_eM6Ke:     { algorithm = 0xb; break; }
        case BHSM_CryptographicAlgorithm_eM6S:      { algorithm = 0xc; break; }
        case BHSM_CryptographicAlgorithm_eRc4:      { algorithm = 0xd; break; }
        case BHSM_CryptographicAlgorithm_eMsMultiSwapMac: { algorithm = 0xe; break; }
        case BHSM_CryptographicAlgorithm_eWmDrmPd:  { algorithm = 0xf; break; }
        case BHSM_CryptographicAlgorithm_eAes128G:  { algorithm = 0x10; break; }
        case BHSM_CryptographicAlgorithm_eHdDvd:    { algorithm = 0x11; break; }
        case BHSM_CryptographicAlgorithm_eBrDvd:    { algorithm = 0x12; break; }
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
            case BHSM_Keyslot_TerminationMode_eCtsDvbCpcm:        { terminationMode = 0x3; break; }
            case BHSM_Keyslot_TerminationMode_eFrontResidue:      { terminationMode = 0x4; break; }
            case BHSM_Keyslot_TerminationMode_eMsc:               { terminationMode = 0x5; break; }
            case BHSM_Keyslot_TerminationMode_eReserved0x06:      { terminationMode = 0x6; break; }
            case BHSM_Keyslot_TerminationMode_eTsAndPacket:       { terminationMode = 0x7; break; }
            case BHSM_Keyslot_TerminationMode_ePacket:            { terminationMode = 0x8; break; }
            case BHSM_Keyslot_TerminationMode_eCbcMac:            { terminationMode = 0x9; break; }
            case BHSM_Keyslot_TerminationMode_eCMac:              { terminationMode = 0xa; break; }
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

    /* Key Offset */
    if( pEntry->settings.external.key ) {
        control |= ((pEntry->externalOffset.key & 0x7F) << 17); /* 7 bits, b17-b23 */
    }

    /* Iv  Offset */
    if( pEntry->settings.external.iv ) {
        control |= ((pEntry->externalOffset.iv & 0x7F) << 24); /* 7 bits, b24-b30 */
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



#define  BHSM_BSP_INIT_KEYSLOT_eMulti2 ( BCMD_InitKeySlot_InCmdCfg_eConfigMulti2Slot )
#define  BHSM_BSP_INIT_KEYSLOT_eType0  ( BCMD_InitKeySlot_InCmdCfg_eSlotNumber       )
#define  BHSM_BSP_INIT_KEYSLOT_eType1  ( BCMD_InitKeySlot_InCmdCfg_eSlotNumber+(1<<2))
#define  BHSM_BSP_INIT_KEYSLOT_eType2  ( BCMD_InitKeySlot_InCmdCfg_eSlotNumber+(2<<2))
#define  BHSM_BSP_INIT_KEYSLOT_eType3  ( BCMD_InitKeySlot_InCmdCfg_eSlotNumber+(3<<2))
#define  BHSM_BSP_INIT_KEYSLOT_eType4  ( BCMD_InitKeySlot_InCmdCfg_eSlotNumber+(4<<2))
#define  BHSM_BSP_INIT_KEYSLOT_eType5  ( BCMD_InitKeySlot_InCmdCfg_eSlotNumber+(5<<2))

static BERR_Code _Keyslot_Init( BHSM_Handle hHsm, BHSM_KeyslotModuleSettings *pSettings )
{
    BERR_Code           rc = BERR_SUCCESS;
    BHSM_BspMsg_h       hMsg = NULL;
    BHSM_BspMsgHeader_t header;
    uint8_t  status;

    BDBG_ENTER( _Keyslot_Init );

    if( pSettings == NULL ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }

    rc = BHSM_BspMsg_Create( hHsm, &hMsg );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE( rc ); }

    BHSM_BspMsg_GetDefaultHeader( &header );
    BHSM_BspMsg_Header( hMsg, BCMD_cmdType_eSESSION_INIT_KEYSLOT, &header );

    #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,0)
    BHSM_BspMsg_Pack8( hMsg, BHSM_BSP_INIT_KEYSLOT_eMulti2, pSettings->numKeySlotsForType[BHSM_KeyslotType_eMulti2] );
    #else
    /* BHSM_BspMsg_Pack8( hMsg, BHSM_BSP_INIT_KEYSLOT_eMulti2, pInitKeySlot->numMulti2KeySlots ? 1 : 0 ); */
    #endif
    BHSM_BspMsg_Pack8( hMsg, BHSM_BSP_INIT_KEYSLOT_eType0,  pSettings->numKeySlotsForType[BHSM_KeyslotType_eIvPerSlot] );
    BHSM_BspMsg_Pack8( hMsg, BHSM_BSP_INIT_KEYSLOT_eType1,  0 );
    BHSM_BspMsg_Pack8( hMsg, BHSM_BSP_INIT_KEYSLOT_eType2,  pSettings->numKeySlotsForType[BHSM_KeyslotType_eIvPerEntry] );
    #if BHSM_ZEUS_VERSION != BHSM_ZEUS_VERSION_CALC(3,0)
    BHSM_BspMsg_Pack8( hMsg, BHSM_BSP_INIT_KEYSLOT_eType3,  pSettings->numKeySlotsForType[BHSM_KeyslotType_eIvPerBlock] );
    #endif
    BHSM_BspMsg_Pack8( hMsg, BHSM_BSP_INIT_KEYSLOT_eType4,  0 );
    #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,1)
    BHSM_BspMsg_Pack8( hMsg, BHSM_BSP_INIT_KEYSLOT_eType5,  0 );
    #endif

    rc = BHSM_BspMsg_SubmitCommand( hMsg );
    if( rc != BERR_SUCCESS ) { BERR_TRACE( rc ); goto BHSM_P_DONE_LABEL; }

    BHSM_BspMsg_Get8( hMsg, BCMD_CommonBufferFields_eStatus, &status ); /* grab the status byte! */
    switch( status )
    {
        case 0: break; /* all OK! */
       #if BHSM_ZEUS_VERSION < BHSM_ZEUS_VERSION_CALC(4,0)
        case 0x20:
        {
            BDBG_WRN(("%s Keyslots already initialised. OK. [0x%X]", BSTD_FUNCTION, status ));
            break;
        }
       #endif
        default:
        {
           BDBG_ERR(("%s  KeySlot initialisation failed [0x%X]", BSTD_FUNCTION, status ));
           goto BHSM_P_DONE_LABEL;
        }
    }

BHSM_P_DONE_LABEL:

    (void)BHSM_BspMsg_Destroy( hMsg );

    BDBG_LEAVE( _Keyslot_Init );
    return rc;
}

#ifdef BHSM_BUILD_HSM_FOR_SAGE
/* function that configures loads HSM configuration from BSP  */
static BERR_Code _Keyslot_LoadConfigurationFromBsp( BHSM_Handle hHsm )
{
    BERR_Code           rc = BERR_SUCCESS;
    BHSM_BspMsg_h       hMsg = NULL;
    BHSM_BspMsgHeader_t header;
    BHSM_KeySlotModule* pModule;
    uint8_t             status = 0;
    uint8_t             numSlots[BHSM_KeyslotType_eMax] =  {0};
    unsigned            count;
    unsigned            i;

    BDBG_ENTER( _Keyslot_LoadConfigurationFromBsp );

    rc = BHSM_BspMsg_Create( hHsm, &hMsg );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE( rc ); }

    BHSM_BspMsg_GetDefaultHeader( &header );
    BHSM_BspMsg_Header( hMsg, BCMD_cmdType_eKEY_SLOT_STATUS_QUERY, &header );

    rc = BHSM_BspMsg_SubmitCommand ( hMsg );
    if( rc != BERR_SUCCESS ) { BERR_TRACE( rc); goto BHSM_P_DONE_LABEL; }

    BHSM_BspMsg_Get8( hMsg, BCMD_KeySlotStatusQuery_OutCmd_eStatus, &status );
    if( status != 0 ) {
        rc = BHSM_STATUS_BSP_ERROR;
        BDBG_ERR(("%s BSP Status error [0x%X]", BSTD_FUNCTION, status ));
        goto BHSM_P_DONE_LABEL;
    }

    BHSM_BspMsg_Get8( hMsg, BCMD_KeySlotStatusQuery_OutCmd_eStatus, &status );

    BHSM_BspMsg_Get8( hMsg, BCMD_KeySlotStatusQuery_OutCmd_eSlotNumberSlotType0,   &numSlots[BHSM_KeyslotType_eIvPerSlot]  );    /* ca */
    BHSM_BspMsg_Get8( hMsg, BCMD_KeySlotStatusQuery_OutCmd_eSlotNumberSlotType3,   &numSlots[BHSM_KeyslotType_eIvPerBlock] );    /* block */
    BHSM_BspMsg_Get8( hMsg, BCMD_KeySlotStatusQuery_OutCmd_eSlotNumberSlotType2,   &numSlots[BHSM_KeyslotType_eIvPerEntry] ); /* jumbo.*/
    BHSM_BspMsg_Get8( hMsg, BCMD_KeySlotStatusQuery_OutCmd_eMulti2SystemKeyConfig, &numSlots[BHSM_KeyslotType_eMulti2]     );
    /*  BHSM_BspMsg_Get8( hMsg, BCMD_KeySlotStatusQuery_OutCmd_eSlotNumberSlotType1,   ); */
    /*  BHSM_BspMsg_Get8( hMsg, BCMD_KeySlotStatusQuery_OutCmd_eSlotNumberSlotType4,   ); */
    /*  BHSM_BspMsg_Get8( hMsg, BCMD_KeySlotStatusQuery_OutCmd_eSlotNumberSlotType5,   ); */
    /*  BHSM_BspMsg_Get8( hMsg, BCMD_KeySlotStatusQuery_OutCmd_eKeySlotOwnership,      ); */

    BDBG_LOG(("#IvPerSlot   [%d]", numSlots[BHSM_KeyslotType_eIvPerSlot] ));
    BDBG_LOG(("#IvPerBlock  [%d]", numSlots[BHSM_KeyslotType_eIvPerBlock] ));
    BDBG_LOG(("#IvPerEntry  [%d]", numSlots[BHSM_KeyslotType_eIvPerEntry] ));
    BDBG_LOG(("#IvPerMulti2 [%d]", numSlots[BHSM_KeyslotType_eMulti2] ));

    pModule = hHsm->modules.pKeyslots;
    for( i = 0; i < BHSM_KeyslotType_eMax; i++ ){

        pModule->types[i].offset = count;
        pModule->types[i].maxNumber = numSlots[i];
        count += numSlots[i];
    }

BHSM_P_DONE_LABEL:

    (void)BHSM_BspMsg_Destroy( hMsg );

    BDBG_LEAVE( _Keyslot_LoadConfigurationFromBsp );
    return rc;
}
#endif

static BHSM_KeyslotBlockType _GetEntryBlockType( BHSM_KeyslotBlockEntry entry )
{
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
        default:
        {
            BERR_TRACE( entry );
        }
    }

    return BHSM_KeyslotBlockType_eCa;
}
static BHSM_KeyslotPolarity _GetEntryPolarity( BHSM_KeyslotBlockEntry entry )
{
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
        default:
        {
            BERR_TRACE( entry );
        }
    }

    return BHSM_KeyslotPolarity_eClear;
}



#if 0 /* will delete at a later point. */
static void  _DumpEntry( const char* pString,  BHSM_KeyslotHandle handle, BHSM_KeyslotBlockEntry entry )
{
    BHSM_P_KeyEntry *pEntry = _GetEntry( handle, entry );

    if( pString ){
        BDBG_LOG(("ENTRY ---- [%s] ---- ", pString  ));
    }

    BDBG_LOG(("pEntry[%p]", (void*)pEntry ));
    BDBG_LOG(("pEntry->configured[%d]", pEntry->configured ));
    BDBG_LOG(("pEntry->externalOffset.key[%d]", pEntry->externalOffset.key ));
    BDBG_LOG(("pEntry->externalOffset.iv[%d]", pEntry->externalOffset.iv ));
    BDBG_LOG(("pEntry->settings.algorithm[%d]", pEntry->settings.algorithm ));
    BDBG_LOG(("pEntry->settings.algorithmMode[%d]", pEntry->settings.algorithmMode ));
    BDBG_LOG(("pEntry->settings.counterSize[%d]", pEntry->settings.counterSize));
    BDBG_LOG(("pEntry->settings.terminationMode[%d]", pEntry->settings.terminationMode));
    BDBG_LOG(("pEntry->settings.solitaryMode[%d]", pEntry->settings.solitaryMode));
    BDBG_LOG(("pEntry->settings.external.key[%d]", pEntry->settings.external.key ));
    BDBG_LOG(("pEntry->settings.external.iv[%d]", pEntry->settings.external.iv ));
    BDBG_LOG(("pEntry->settings.pesMode[%d]", pEntry->settings.pesMode));
    BDBG_LOG(("pEntry->settings.outputPolarity.specify[%d]", pEntry->settings.outputPolarity.specify));
    BDBG_LOG(("pEntry->settings.outputPolarity.rPipe[%d]", pEntry->settings.outputPolarity.rPipe));
    BDBG_LOG(("pEntry->settings.outputPolarity.gPipe[%d]", pEntry->settings.outputPolarity.gPipe));
    BDBG_LOG(("pEntry->settings.rPipeEnable[%d]", pEntry->settings.rPipeEnable));
    BDBG_LOG(("pEntry->settings.gPipeEnable[%d]", pEntry->settings.gPipeEnable));

    return;
}


static void  _DumpSlot( const char* pString, BHSM_KeyslotHandle handle )
{
    BHSM_P_KeySlot *pSlot = (BHSM_P_KeySlot*)handle;
    unsigned i;

    if( pString ){
        BDBG_LOG(("SLOT  ---- [%s] ---- ", pString  ));
    }

    BDBG_LOG(("pSlot->configured[%d]", pSlot->configured ));
    BDBG_LOG(("pSlot->slotType[%d]",   _convertSlotType(pSlot->slotType) ));
    BDBG_LOG(("pSlot->number[%d]",     pSlot->number ));

    for( i = 0; i < BHSM_SecurityRegion_eMax; i++ )
    {
        BDBG_LOG(("pSlot->regions.source          [%d] [%#x]"          ,i , pSlot->settings.regions.source[i] ));
        BDBG_LOG(("pSlot->regions.destinationRPipe[%d] [%#x]",i , pSlot->settings.regions.destinationRPipe[i] ));
        BDBG_LOG(("pSlot->regions.destinationGPipe[%d] [%#x]",i , pSlot->settings.regions.destinationGPipe[i] ));
    }

    BDBG_LOG(("pSlot->settings.encryptBeforeRave[%d]", pSlot->settings.encryptBeforeRave ));

    return;
}
#endif


uint8_t _convertSlotType( BHSM_KeyslotType type )
{

    switch ( type )
    {
        case BHSM_KeyslotType_eIvPerSlot:   return 0;
        case BHSM_KeyslotType_eIvPerBlock:  return 3;
        case BHSM_KeyslotType_eIvPerEntry:  return 2;
        default:
            BDBG_ERR(("Not supported keyslot type %d", type));
            BERR_TRACE( BERR_INVALID_PARAMETER );
    }

    return 0;
}

BERR_Code BHSM_KeySlot_SetMulti2Key( BHSM_Handle hHsm, const BHSM_KeySlotSetMulti2Key *pKeyData )
{
    BERR_Code rc = BERR_SUCCESS;
    BHSM_BspMsg_h hMsg = NULL;
    BHSM_BspMsgHeader_t header;
    uint8_t status;

    BDBG_ENTER( BHSM_KeySlot_SetMulti2Key );

    if( pKeyData == NULL ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }

    rc = BHSM_BspMsg_Create( hHsm, &hMsg );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE(rc); }

    BHSM_BspMsg_GetDefaultHeader( &header );
    BHSM_BspMsg_Header( hMsg, BCMD_cmdType_eCONFIG_MULTI2, &header );

    BHSM_BspMsg_Pack8( hMsg, BCMD_Multi2_InCmdCfg_eWhichSysKey, pKeyData->keySelect );
    BHSM_BspMsg_PackArray( hMsg, BCMD_Multi2_InCmdCfg_eSystemKeys, pKeyData->key, sizeof(pKeyData->key) );

    rc = BHSM_BspMsg_SubmitCommand( hMsg );
    if( rc != BERR_SUCCESS ) { BERR_TRACE( rc ); goto BHSM_P_DONE_LABEL; }

    BHSM_BspMsg_Get8( hMsg, BCMD_CommonBufferFields_eStatus, &status );
    if( status != 0 ) {
        BDBG_ERR(("%s BSP error status[0x%02X]", BSTD_FUNCTION, status ));
        rc =  BERR_TRACE(BHSM_STATUS_BSP_ERROR);
        goto BHSM_P_DONE_LABEL;
    }

BHSM_P_DONE_LABEL:

    (void)BHSM_BspMsg_Destroy( hMsg );

    BDBG_LEAVE( BHSM_KeySlot_SetMulti2Key );
    return rc;
}
