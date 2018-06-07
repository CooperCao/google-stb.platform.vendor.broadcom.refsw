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
#include "nexus_security_module.h"
#include "nexus_security.h"
#include "nexus_security_init.h"
#include "nexus_power_management.h"
#include "nexus_base.h"
#include "priv/nexus_core.h"
#include "priv/nexus_security_priv.h"
#include "priv/nexus_rsa_priv.h"
#include "priv/nexus_pid_channel_priv.h"
#include "priv/nexus_security_standby_priv.h"
#include "priv/nexus_regver_priv.h"
#include "priv/nexus_transport_priv.h"
#include "nexus_security_datatypes.h"
#include "bhsm.h"
#include "bhsm_keyslot.h"

BDBG_MODULE(nexus_security);

static BERR_Code initialiseHsm( const NEXUS_SecurityModuleSettings * pSettings );
static void uninitialiseHsm( void );
static NEXUS_Error secureFirmwareRave( void );

NEXUS_ModuleHandle NEXUS_P_SecurityModule = NULL;

static struct {
    BHSM_Handle hsmHandle;
    NEXUS_SecurityModuleSettings settings;
    NEXUS_SecurityModuleInternalSettings moduleSettings;
} g_security;

/****************************************
 * Module functions
 ****************************************/

void NEXUS_GetSecurityCapabilities( NEXUS_SecurityCapabilities *pCaps )
{
    unsigned x = 0;
    BHSM_ModuleCapabilities hsmCaps;
    BHSM_Handle hHsm;
    BERR_Code rc = BERR_UNKNOWN;

    BDBG_ENTER(NEXUS_GetSecurityCapabilities);

    if( !pCaps ) { BERR_TRACE(NEXUS_INVALID_PARAMETER); return; };

    BKNI_Memset( pCaps, 0, sizeof(*pCaps) );

    NEXUS_Security_GetHsm_priv( &hHsm );
    if( !hHsm ) { BERR_TRACE( NEXUS_NOT_INITIALIZED ); return; }

    BKNI_Memset( &hsmCaps, 0, sizeof(hsmCaps) );
    rc = BHSM_GetCapabilities( hHsm,  &hsmCaps );
    if( rc != BERR_SUCCESS ) { BERR_TRACE( NEXUS_NOT_AVAILABLE ); return; }

    pCaps->version.zeus.major    = hsmCaps.version.zeus.major;
    pCaps->version.zeus.minor    = hsmCaps.version.zeus.minor;
    pCaps->version.zeus.subminor = hsmCaps.version.zeus.subminor;
    pCaps->version.bfw.major     = hsmCaps.version.bfw.major;
    pCaps->version.bfw.minor     = hsmCaps.version.bfw.minor;
    pCaps->version.bfw.subminor  = hsmCaps.version.bfw.subminor;

    pCaps->firmwareEpoch.valid = hsmCaps.firmwareEpoch.valid;
    pCaps->firmwareEpoch.value = hsmCaps.firmwareEpoch.value;

    BDBG_CASSERT( (unsigned)NEXUS_KeySlotType_eMax == (unsigned)BHSM_KeyslotType_eMax );

    for( x = 0; x < NEXUS_KeySlotType_eMax; x++ )
    {
        pCaps->numKeySlotsForType[x] = hsmCaps.numKeyslotsForType[x];
    }

    BDBG_LEAVE(NEXUS_GetSecurityCapabilities);
    return;
}

void NEXUS_SecurityModule_GetDefaultSettings( NEXUS_SecurityModuleSettings *pSettings )
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    NEXUS_GetDefaultCommonModuleSettings(&pSettings->common);
    if ((BHSM_ZEUS_VERSION == BHSM_ZEUS_VERSION_CALC(4,2)) && (BHSM_ZEUS_VER_SUBMINOR == 2) ) {
        /* Zeus 4.2.2 */
        pSettings->numKeySlotsForType[NEXUS_KeySlotType_eIvPerSlot]  = 10;
        pSettings->numKeySlotsForType[NEXUS_KeySlotType_eIvPerBlock] = 4;
        pSettings->numKeySlotsForType[NEXUS_KeySlotType_eIvPerEntry] = 6;
    } else {
#if defined(NEXUS_HAS_NSK2HDI) && (NEXUS_SECURITY_ZEUS_VERSION_MAJOR < 5)
        /* Zeus 4 layout */
        pSettings->numKeySlotsForType[NEXUS_KeySlotType_eIvPerSlot]  = 0;
        pSettings->numKeySlotsForType[NEXUS_KeySlotType_eIvPerBlock] = 50;
        pSettings->numKeySlotsForType[NEXUS_KeySlotType_eIvPerEntry] = 0;
        pSettings->numKeySlotsForType[NEXUS_KeySlotType_eOxford1]  = 32;
        pSettings->numKeySlotsForType[NEXUS_KeySlotType_eOxford2] = 50;
        pSettings->numKeySlotsForType[NEXUS_KeySlotType_eOxford3] = 1;
#else
        /* Zeus 2, 3, and 4.x and 5*/
        pSettings->numKeySlotsForType[NEXUS_KeySlotType_eIvPerSlot]  = 21;
        pSettings->numKeySlotsForType[NEXUS_KeySlotType_eIvPerBlock] = 21;
        pSettings->numKeySlotsForType[NEXUS_KeySlotType_eIvPerEntry] = 6;
#endif
    }

    if ( NEXUS_SECURITY_ZEUS_VERSION_MAJOR >= 5) {
        pSettings->numKeySlotsForType[NEXUS_KeySlotType_eIvPerBlock256] = 11;
        pSettings->numKeySlotsForType[NEXUS_KeySlotType_eIvPerEntry256] = 5;
    }

    #if NEXUS_ENFORCE_REGION_VERIFICATION && 0 /*TODO ... enable selection of regions to be checked during automation tests.*/
    pSettings->enforceAuthentication[NEXUS_SecurityFirmwareType_eVideoDecoder]   = true;
    pSettings->enforceAuthentication[NEXUS_SecurityFirmwareType_eAudioDecoder]   = true;
    #endif

    return;
}

void NEXUS_SecurityModule_GetDefaultInternalSettings( NEXUS_SecurityModuleInternalSettings *pSettings )
{
    BKNI_Memset( pSettings, 0, sizeof(*pSettings) );
    return;
}

NEXUS_ModuleHandle NEXUS_SecurityModule_Init( const NEXUS_SecurityModuleInternalSettings *pModuleSettings,
                                              const NEXUS_SecurityModuleSettings *pSettings )
{
    NEXUS_ModuleSettings moduleSettings;
    NEXUS_SecurityRegionModuleSettings rvSettings;
    BERR_Code rc = NEXUS_SUCCESS;
    int i;

    BDBG_ENTER(NEXUS_SecurityModule_Init);

    if( NEXUS_P_SecurityModule ) { BERR_TRACE(NEXUS_INVALID_PARAMETER); return NULL; }; /*Already initialised. */
    if( !pSettings ) { BERR_TRACE(NEXUS_INVALID_PARAMETER); return NULL; };
    #if NEXUS_HAS_XPT_DMA
    /* if( !pModuleSettings->transport ) { BERR_TRACE(NEXUS_INVALID_PARAMETER); return NULL; } */ /* security module requires transport module */
    #endif

    /* init global module handle */
    NEXUS_Module_GetDefaultSettings(&moduleSettings);
    moduleSettings.priority = NEXUS_AdjustModulePriority(NEXUS_ModulePriority_eLow, &pSettings->common);
    NEXUS_P_SecurityModule = NEXUS_Module_Create("security", &moduleSettings);
    if( NEXUS_P_SecurityModule == NULL ) { BERR_TRACE(NEXUS_UNKNOWN);  return NULL; }

    NEXUS_LockModule();

    g_security.settings = *pSettings;
    g_security.moduleSettings = *pModuleSettings;

    rc = initialiseHsm( pSettings );
    if (rc) {rc = BERR_TRACE(rc); goto err_init;}

    NEXUS_Security_GetDefaultRegionVerificationModuleSettings( &rvSettings );

    for( i = 0; i < NEXUS_SecurityFirmwareType_eMax; i++ ){
        rvSettings.enforceAuthentication[i] = pSettings->enforceAuthentication[i];
    }

    rc = NEXUS_Security_RegionVerification_Init_priv( &rvSettings ); /* region verification private interface. */
    if (rc) { rc = BERR_TRACE(rc); goto err_init; }

    rc = NEXUS_Rsa_Init( );
    if (rc) { rc = BERR_TRACE(rc); goto err_init; }

  #if 0
    rc = NEXUS_RegionVerify_Init( );                     /* region verification public interface. */
    if (rc) { rc = BERR_TRACE(rc); goto err_init; }
  #endif

    NEXUS_UnlockModule();

    return NEXUS_P_SecurityModule;

err_init:
    NEXUS_UnlockModule();
    NEXUS_Module_Destroy(NEXUS_P_SecurityModule);
    NEXUS_P_SecurityModule = NULL;

    BDBG_LEAVE(NEXUS_SecurityModule_Init);

    return NULL;
}

NEXUS_Error NEXUS_SecurityModule_InitTransport_priv(void)
{
    NEXUS_Error rc;

    NEXUS_Module_Lock( g_security.moduleSettings.transport );
    rc = NEXUS_TransportModule_PostInit_priv( secureFirmwareRave );
    NEXUS_Module_Unlock( g_security.moduleSettings.transport );
    if (rc) return BERR_TRACE( rc );

    return NEXUS_SUCCESS;
}

void NEXUS_SecurityModule_Uninit(void)
{
    BDBG_ENTER(NEXUS_SecurityModule_Uninit);

    NEXUS_LockModule();

    NEXUS_Rsa_Uninit( );

    NEXUS_Security_RegionVerification_UnInit_priv( );

    uninitialiseHsm();

    NEXUS_UnlockModule();

    NEXUS_Module_Destroy( NEXUS_P_SecurityModule );
    NEXUS_P_SecurityModule = NULL;

    BDBG_LEAVE(NEXUS_SecurityModule_Uninit);
    return;
}

static BERR_Code initialiseHsm(const NEXUS_SecurityModuleSettings * pSettings)
{
    BHSM_ModuleSettings hsmSettings;
    BERR_Code rc = NEXUS_SUCCESS;
    unsigned x = 0;

    BDBG_ENTER(initialiseHsm);
    NEXUS_PowerManagement_SetCoreState(NEXUS_PowerManagementCore_eHsm, true);

    BKNI_Memset( &hsmSettings, 0, sizeof(hsmSettings));
    #if NEXUS_HAS_SAGE
    hsmSettings.sageEnabled = true;
    #endif
    hsmSettings.hReg = g_pCoreHandles->reg;
    hsmSettings.hChip = g_pCoreHandles->chp;
    hsmSettings.hInterrupt = g_pCoreHandles->bint;
    hsmSettings.mmaHeap = g_pCoreHandles->heap[g_pCoreHandles->defaultHeapIndex].mma;

    BDBG_CASSERT( (unsigned)BHSM_KeyslotType_eIvPerSlot  == (unsigned)NEXUS_KeySlotType_eIvPerSlot );
    BDBG_CASSERT( (unsigned)BHSM_KeyslotType_eIvPerBlock == (unsigned)NEXUS_KeySlotType_eIvPerBlock );
    BDBG_CASSERT( (unsigned)BHSM_KeyslotType_eIvPerEntry == (unsigned)NEXUS_KeySlotType_eIvPerEntry );
    BDBG_CASSERT( (unsigned)BHSM_KeyslotType_eIvPerEntry256 == (unsigned)NEXUS_KeySlotType_eIvPerEntry256 );
    BDBG_CASSERT( (unsigned)BHSM_KeyslotType_eIvPerBlock256 == (unsigned)NEXUS_KeySlotType_eIvPerBlock256 );
    BDBG_CASSERT( (unsigned)BHSM_KeyslotType_eMax        == (unsigned)NEXUS_KeySlotType_eMax );

    for( x = 0; x < BHSM_KeyslotType_eMax; x++ ) {
        hsmSettings.numKeySlotsForType[x] = pSettings->numKeySlotsForType[x];
    }

    g_security.hsmHandle = BHSM_Open( &hsmSettings );
    if( g_security.hsmHandle == NULL ) { return BERR_TRACE(NEXUS_NOT_AVAILABLE); }

    #if NEXUS_HAS_XPT_DMA
    rc = BHSM_SetPidChannelBypassKeyslot(g_security.hsmHandle, NEXUS_PidChannel_GetBypassKeySlotIndex_isrsafe(NEXUS_BypassKeySlot_eG2GR), NEXUS_BypassKeySlot_eG2GR);
    if (rc) {BERR_TRACE(rc);} /* keep going */
    rc = BHSM_SetPidChannelBypassKeyslot(g_security.hsmHandle, NEXUS_PidChannel_GetBypassKeySlotIndex_isrsafe(NEXUS_BypassKeySlot_eGR2R), NEXUS_BypassKeySlot_eGR2R);
    if (rc) {BERR_TRACE(rc);} /* keep going */
    rc = BHSM_SetPidChannelBypassKeyslot(g_security.hsmHandle, NEXUS_PidChannel_GetBypassKeySlotIndex_isrsafe(NEXUS_BypassKeySlot_eGT2T), NEXUS_BypassKeySlot_eGT2T);
    if (rc) {BERR_TRACE(rc);} /* keep going */
    #endif

    NEXUS_PowerManagement_SetCoreState(NEXUS_PowerManagementCore_eHsm, false);

    BDBG_LEAVE(initialiseHsm);
    return rc;
}

static void uninitialiseHsm( void )
{
    BHSM_Close(g_security.hsmHandle);
    g_security.hsmHandle = NULL;
    return;
}

void NEXUS_Security_GetHsm_priv( BHSM_Handle *pHsm )
{
    NEXUS_ASSERT_MODULE();

    if( !g_security.hsmHandle ){ BERR_TRACE(NEXUS_INVALID_PARAMETER); }

    *pHsm = g_security.hsmHandle;

    return;
}

void NEXUS_Security_LockTransport( bool lock )
{
    if( !g_security.moduleSettings.transport ) { BERR_TRACE(NEXUS_NOT_INITIALIZED); return; }

    if( lock ) {
        NEXUS_Module_Lock( g_security.moduleSettings.transport );
        return;
    }
    else {
        NEXUS_Module_Unlock( g_security.moduleSettings.transport );
    }

    return;
}


NEXUS_Error NEXUS_SecurityModule_Standby_priv( bool enabled, const NEXUS_StandbySettings *pSettings )
{
#if NEXUS_POWER_MANAGEMENT
    BERR_Code rc;

    BDBG_ENTER(NEXUS_SecurityModule_Standby_priv);

    /* NEXUS_SecurityModule_Sweep_priv();*/

    if( enabled ) /* if *enter* standby */
    {
        if( pSettings->mode != NEXUS_StandbyMode_eDeepSleep )
        {
            /* NEXUS_PowerManagement_SetCoreState is called when keyslots are allocated and free'd.
               in non-S3, this does not have to occur, so we power down as many times as the number of keyslots opened */
            NEXUS_PowerManagement_SetCoreState(NEXUS_PowerManagementCore_eHsm, false);
        }
        else
        {
            uninitialiseHsm();
        }
    }
    else
    {
        if (g_security.hsmHandle)
        { /* not S3 */
            NEXUS_PowerManagement_SetCoreState(NEXUS_PowerManagementCore_eHsm, true);
        }
        else
        {
            NEXUS_SecurityRegionModuleSettings rvSettings;

            rc = initialiseHsm( &g_security.settings );
            if (rc) { return BERR_TRACE(NEXUS_NOT_SUPPORTED); }

            /* Reinitialise Region Verification module. */
            NEXUS_Security_GetDefaultRegionVerificationModuleSettings( &rvSettings );
            rc = NEXUS_Security_RegionVerification_Init_priv( &rvSettings );
            if (rc) { return BERR_TRACE(rc); }
           #if 0
            rc = NEXUS_RegionVerify_Init( );
            if (rc) { return BERR_TRACE(rc); }
           #endif
        }

        if( g_security.moduleSettings.callTransportPostInit )
        {
            NEXUS_Module_Lock(g_security.moduleSettings.transport);
            rc = NEXUS_TransportModule_PostResume_priv();
            NEXUS_Module_Unlock(g_security.moduleSettings.transport);
            if (rc) return BERR_TRACE(rc);
        }
    }

#else
    BSTD_UNUSED(pSettings);
    BSTD_UNUSED(enabled);
#endif

    BDBG_LEAVE(NEXUS_SecurityModule_Standby_priv);
    return NEXUS_SUCCESS;
}

/* Function to authenticate/verify rave firmware. */
static NEXUS_Error secureFirmwareRave( void )
{
    NEXUS_Error rc;
    NEXUS_SecurityRegionConfiguration regionConfig;
    void *pRaveFirmware = NULL;
    unsigned raveFirmwareSize = 0;
    NEXUS_Addr regionAddress = 0;
    void *pRaveFirmwareDeviceMem = NULL;
    NEXUS_MemoryAllocationSettings memSettings;

    BDBG_ENTER(secureFirmwareRave);

    NEXUS_Security_RegionGetDefaultConfig_priv( NEXUS_SecurityRegverRegionID_eRave, &regionConfig );
    /* use defaults. */
    rc = NEXUS_Security_RegionConfig_priv( NEXUS_SecurityRegverRegionID_eRave, &regionConfig );
    if( rc ) { return BERR_TRACE(rc); }

    /* locate Rave Firmware. */
    NEXUS_TransportModule_GetRaveFirmware_isrsafe( &pRaveFirmware, &raveFirmwareSize );

    NEXUS_Memory_GetDefaultAllocationSettings(&memSettings);
    memSettings.heap = g_pCoreHandles->heap[g_pCoreHandles->defaultHeapIndex].nexus;
    rc = NEXUS_Memory_Allocate( raveFirmwareSize, &memSettings, &pRaveFirmwareDeviceMem );
    if( rc ) { return BERR_TRACE(rc); }

    BKNI_Memcpy( pRaveFirmwareDeviceMem, pRaveFirmware, raveFirmwareSize );
    regionAddress =  NEXUS_AddrToOffset( pRaveFirmwareDeviceMem );

    rc = NEXUS_Security_RegionVerifyEnable_priv( NEXUS_SecurityRegverRegionID_eRave,
                                                 regionAddress,
                                                 raveFirmwareSize );

    if( pRaveFirmwareDeviceMem ) { NEXUS_Memory_Free( pRaveFirmwareDeviceMem ); }

    if( rc ) { return BERR_TRACE( rc ); }

    BDBG_LEAVE(secureFirmwareRave);
    return BERR_SUCCESS;
}

void NEXUS_SecurityModule_SetTransportModule(NEXUS_ModuleHandle transport)
{
    if( g_security.moduleSettings.transport && !transport ) {
        NEXUS_SecurityModule_Sweep_priv();
    }

    g_security.moduleSettings.transport = transport;
}
