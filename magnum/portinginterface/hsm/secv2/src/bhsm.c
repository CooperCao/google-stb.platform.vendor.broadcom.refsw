/******************************************************************************
 *  Copyright (C) 2018 Broadcom.
 *  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to
 *  the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied),
 *  right to use, or waiver of any kind with respect to the Software, and
 *  Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 *  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 *  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization,
 *  constitutes the valuable trade secrets of Broadcom, and you shall use all
 *  reasonable efforts to protect the confidentiality thereof, and to use this
 *  information only in connection with your use of Broadcom integrated circuit
 *  products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 *  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 *  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 *  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 *  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 *  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 *  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 *  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/

#include "bstd.h"
#include "bkni.h"
#include "bhsm.h"
#include "bhsm_priv.h"
#include "bhsm_keyslot_priv.h"
#include "bhsm_rv_rsa.h"
#include "bhsm_rv_region_priv.h"
#include "bhsm_otp_key.h"
#include "bhsm_bsp_msg.h"
#include "bhsm_rsa.h"
#include "bsp_p_hw.h"
#ifdef BHSM_DEBUG_BSP
 #include "bhsm_bsp_debug.h"
#endif

BDBG_MODULE(BHSM);
BDBG_OBJECT_ID(BHSM_P_Handle);

BHSM_Handle BHSM_Open( const BHSM_ModuleSettings *pSettings )
{
    BERR_Code rc = BERR_SUCCESS;
    BHSM_P_Handle *pHandle = NULL;
    BHSM_KeyslotModuleSettings keyslotSettings;
    unsigned i = 0;

    BDBG_ENTER( BHSM_Open );

    if( !pSettings ) { BERR_TRACE(BERR_INVALID_PARAMETER); return NULL; }

    pHandle = BKNI_Malloc( sizeof(*pHandle) );
    if ( pHandle == NULL ) { BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); return NULL; }

    BKNI_Memset( pHandle, 0, sizeof(*pHandle) );
    BDBG_OBJECT_SET( pHandle, BHSM_P_Handle );

    pHandle->regHandle = pSettings->hReg;
    pHandle->chipHandle = pSettings->hChip;
    pHandle->interruptHandle = pSettings->hInterrupt;
    pHandle->mmaHeap = pSettings->mmaHeap;

   #ifdef BHSM_DEBUG_BSP
    rc = BHSM_BspDebug_Init( pHandle, NULL );
    if( rc != BERR_SUCCESS ){ (void)BERR_TRACE( rc ); goto error; }
   #endif

    rc = BHSM_BspMsg_Init( pHandle, NULL );
    if( rc != BERR_SUCCESS ) { (void)BERR_TRACE( rc ); goto error; }

    BKNI_Memset( &keyslotSettings, 0, sizeof(keyslotSettings) );
    for( i = 0; i < BHSM_KeyslotType_eMax; i++ )
    {
        keyslotSettings.numKeySlotsForType[i] = pSettings->numKeySlotsForType[i];
    }
    rc = BHSM_Keyslot_Init( pHandle, &keyslotSettings );
    if( rc != BERR_SUCCESS ) { BERR_TRACE(rc); goto error; }

    rc = BHSM_KeyLadder_Init( pHandle, NULL );
    if( rc != BERR_SUCCESS ) { BERR_TRACE(rc); goto error; }

    rc = BHSM_RvRsa_Init( pHandle, NULL );
    if( rc != BERR_SUCCESS ) { BERR_TRACE(rc); goto error; }

    rc = BHSM_RvRegion_Init_priv( pHandle, NULL );
    if( rc != BERR_SUCCESS ) { BERR_TRACE(rc); goto error; }

    rc = BHSM_Rsa_Init( pHandle, NULL );
    if( rc != BERR_SUCCESS ) { BERR_TRACE(rc); goto error; }

    rc = BHSM_OtpKey_Init( pHandle, NULL );
    if( rc != BERR_SUCCESS ) { BERR_TRACE(rc); goto error; }

    BDBG_LEAVE( BHSM_Open );
    return pHandle;

error:
    if( pHandle ) BHSM_Close( pHandle );

    return NULL;
}


BERR_Code BHSM_Close( BHSM_Handle hHsm )
{
    BDBG_ENTER( BHSM_Close );

    if( !hHsm ) { return BERR_TRACE(BERR_INVALID_PARAMETER); }
    BDBG_OBJECT_ASSERT( hHsm, BHSM_P_Handle );

    BHSM_OtpKey_Uninit( hHsm );
    BHSM_Rsa_Uninit( hHsm );
    BHSM_RvRegion_Uninit_priv( hHsm );
    BHSM_RvRsa_Uninit( hHsm );
    BHSM_KeyLadder_Uninit( hHsm );
    BHSM_Keyslot_Uninit( hHsm );
    BHSM_BspMsg_Uninit( hHsm );
   #ifdef BHSM_DEBUG_BSP
    BHSM_BspDebug_Uninit( hHsm );
   #endif

    BDBG_OBJECT_DESTROY( hHsm, BHSM_P_Handle );
    BKNI_Free( hHsm );

    BDBG_LEAVE( BHSM_Close );
    return BERR_SUCCESS;
}


BERR_Code BHSM_GetCapabilities( BHSM_Handle hHsm,  BHSM_ModuleCapabilities *pCaps )
{
    BHSM_KeyslotModuleCapabilities keyslotCaps;
    BERR_Code rc = BERR_UNKNOWN;
    unsigned i;

    BDBG_ENTER( BHSM_GetCapabilities );

    if( !hHsm ) { return BERR_TRACE(BERR_INVALID_PARAMETER); }
    if( !pCaps ) { return BERR_TRACE(BERR_INVALID_PARAMETER); }

    BDBG_OBJECT_ASSERT( hHsm, BHSM_P_Handle );

    BKNI_Memset( pCaps, 0, sizeof(*pCaps) );

    pCaps->version.zeus.major    = hHsm->bfwVersion.version.zeus.major;
    pCaps->version.zeus.minor    = hHsm->bfwVersion.version.zeus.minor;
    pCaps->version.zeus.subminor = hHsm->bfwVersion.version.zeus.subminor;
    pCaps->version.bfw.major    = hHsm->bfwVersion.version.bfw.major;
    pCaps->version.bfw.minor    = hHsm->bfwVersion.version.bfw.minor;
    pCaps->version.bfw.subminor = hHsm->bfwVersion.version.bfw.subminor;

    rc = BHSM_P_KeyslotModule_GetCapabilities(hHsm, &keyslotCaps);
    if( rc != BERR_SUCCESS ) { return BERR_TRACE(rc); }

    for( i = 0; i < BHSM_KeyslotType_eMax; i++ )
    {
        pCaps->numKeyslotsForType[i] = keyslotCaps.numKeySlotsForType[i];
    }

    pCaps->archesPerMemc = BSP_ARCHES_PER_MEMC;

    BDBG_LEAVE( BHSM_GetCapabilities );
    return BERR_SUCCESS;
}


BERR_Code BHSM_MemcpySwap( void* pDest, const void* pSrc, unsigned byteSize )
{
    unsigned wordSize = byteSize/4;
    uint32_t *pS = (uint32_t*)pSrc;
    uint32_t *pD = (uint32_t*)pDest;
    unsigned i;

    if( !pDest ) { return BERR_TRACE(BERR_INVALID_PARAMETER); }
    if( !pSrc ) { return BERR_TRACE(BERR_INVALID_PARAMETER); }
    if( byteSize % 4) return BERR_TRACE( BERR_INVALID_PARAMETER );
    if( wordSize == 0) return BERR_TRACE( BERR_INVALID_PARAMETER );

    for( i = 0; i < wordSize; i++ )
    {
        *pD =  ( *pS & 0x000000FF ) << 24 |
               ( *pS & 0x0000FF00 ) << 8  |
               ( *pS & 0x00FF0000 ) >> 8  |
               ( *pS & 0xFF000000 ) >> 24;
        pD++; pS++;
    }

    return BERR_SUCCESS;
}
