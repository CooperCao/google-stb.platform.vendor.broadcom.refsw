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
#include "bmma.h"
#include "berr_ids.h"
#include "bsp_types.h"
#include "bhsm_common.h"
#include "bhsm_rv_region.h"
#include "bhsm_priv.h"
#include "bhsm_p_rv.h"
#include "bhsm_rv_region_priv.h"
#if BHSM_RV_REGION_EXTENSION
#include "bhsm_rv_region_sage_restricted.h"
#endif


BDBG_MODULE( BHSM );


/* called internally on platform initialisation */
BERR_Code BHSM_RvRegion_Init_priv( BHSM_Handle hHsm, BHSM_RvRegionModuleSettings *pSettings )
{
    BDBG_ENTER( BHSM_RvRegion_Init_priv );

    if( !hHsm ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }
    BSTD_UNUSED( pSettings );

    BDBG_CASSERT( (unsigned)BHSM_RegionId_eMax == (unsigned)Bsp_CmdRv_RegionId_eRegionMax );

    hHsm->modules.pRvRegions = (BHSM_RvRegionModule*)BKNI_Malloc( sizeof(BHSM_RvRegionModule) );
    if( !hHsm->modules.pRvRegions ) { return BERR_TRACE( BERR_OUT_OF_SYSTEM_MEMORY ); }

    BKNI_Memset( hHsm->modules.pRvRegions, 0, sizeof(BHSM_RvRegionModule) );

    BDBG_LEAVE( BHSM_RvRegion_Init_priv );
    return BERR_SUCCESS;
}


/* called internally on platform un-initialisation */
void BHSM_RvRegion_Uninit_priv( BHSM_Handle hHsm )
{
    BDBG_ENTER( BHSM_RvRegion_Uninit_priv );

    if( !hHsm ) { BERR_TRACE( BERR_INVALID_PARAMETER ); return; }
    if( !hHsm->modules.pRvRegions ){ BERR_TRACE( BERR_INVALID_PARAMETER ); return; }

    BKNI_Free( hHsm->modules.pRvRegions );
    hHsm->modules.pRvRegions = NULL;

    BDBG_LEAVE( BHSM_RvRegion_Uninit_priv );
    return;
}



BHSM_RvRegionHandle BHSM_RvRegion_Allocate( BHSM_Handle hHsm, const BHSM_RvRegionAllocateSettings *pSettings )
{
    BHSM_P_RvRegion *pRegion = NULL;

    BDBG_ENTER( BHSM_RvRegion_Allocate );

    if( !hHsm ) { BERR_TRACE( BERR_INVALID_PARAMETER ); return NULL; }
    if( !hHsm->modules.pRvRegions ) { BERR_TRACE( BERR_NOT_INITIALIZED ); return NULL; }
    if( !pSettings ) { BERR_TRACE( BERR_INVALID_PARAMETER ); return NULL; }
    if( pSettings->regionId >= BHSM_RegionId_eMax ) { BERR_TRACE( BERR_INVALID_PARAMETER ); return NULL; }

    pRegion = &hHsm->modules.pRvRegions->instances[pSettings->regionId];

    if( pRegion->allocated ) { BERR_TRACE( BERR_NOT_AVAILABLE ); return NULL; }

   #if BHSM_RV_REGION_EXTENSION
    if( BHSM_RvRegion_AllocateExtension_priv( pRegion ) != BERR_SUCCESS ) {
        BERR_TRACE( BERR_NOT_AVAILABLE );
        return NULL;
    }
   #endif

    pRegion->allocated = true;
    pRegion->regionId = pSettings->regionId;
    pRegion->hHsm = hHsm;

    BDBG_LEAVE( BHSM_RvRegion_Allocate );
    return (BHSM_RvRegionHandle)pRegion;
}


void BHSM_RvRegion_Free( BHSM_RvRegionHandle handle )
{
    BHSM_P_RvRegion *pRegion = (BHSM_P_RvRegion*)handle;

    BDBG_ENTER( BHSM_RvRegion_Free );

    if( !pRegion ){ BERR_TRACE( BERR_INVALID_PARAMETER ); return; }
    if( !pRegion->allocated ) { BERR_TRACE( BERR_INVALID_PARAMETER ); return; }

   #if BHSM_RV_REGION_EXTENSION
    BHSM_RvRegionExtension_Free_priv( pRegion );
   #endif

    BKNI_Memset( pRegion, 0, sizeof(*pRegion) );
    pRegion->allocated = false;

    BDBG_LEAVE( BHSM_RvRegion_Free );
    return;
}


void  BHSM_RvRegion_GetSettings( BHSM_RvRegionHandle handle, BHSM_RvRegionSettings *pSettings )
{
    BHSM_P_RvRegion *pRegion = (BHSM_P_RvRegion*)handle;
    BDBG_ENTER( BHSM_RvRegion_GetSettings );

    if( !pRegion ){ BERR_TRACE( BERR_INVALID_PARAMETER ); return; }
    if( !pSettings ){ BERR_TRACE( BERR_INVALID_PARAMETER ); return; }
    if( !pRegion->allocated ) { BERR_TRACE( BERR_INVALID_PARAMETER ); return; }

    *pSettings = pRegion->settings;

    BDBG_LEAVE( BHSM_RvRegion_GetSettings );
    return;
}


BERR_Code BHSM_RvRegion_SetSettings( BHSM_RvRegionHandle handle, const BHSM_RvRegionSettings *pSettings )
{
    BHSM_P_RvRegion *pRegion = (BHSM_P_RvRegion*)handle;
    BDBG_ENTER( BHSM_RvRegion_SetSettings );

    if( !pRegion ){ return BERR_TRACE( BERR_INVALID_PARAMETER ); }
    if( !pSettings ){ return BERR_TRACE( BERR_INVALID_PARAMETER ); }
    if( !pRegion->allocated ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }

    pRegion->settings = *pSettings;
    pRegion->configured = true;

    BDBG_LEAVE( BHSM_RvRegion_SetSettings );
    return BERR_SUCCESS;
}

BERR_Code BHSM_RvRegion_Enable( BHSM_RvRegionHandle handle )
{
    BHSM_P_RvRegion *pRegion = (BHSM_P_RvRegion*)handle;
    BHSM_P_RvEnableRegion bspEnableRegion;
    BSTD_DeviceOffset endOffset;
    BERR_Code rc = BERR_UNKNOWN;

    BDBG_ENTER( BHSM_RvRegion_Enable );

    if( !pRegion ){ return BERR_TRACE( BERR_INVALID_PARAMETER ); }
    if( !pRegion->allocated ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }

    BKNI_Memset( &bspEnableRegion, 0, sizeof(bspEnableRegion) );

    bspEnableRegion.in.regionId = pRegion->regionId;

    if( pRegion->settings.range[0].size )
    {
        bspEnableRegion.in.startAddrMsb = ( pRegion->settings.range[0].address >> 32 ) & 0xFF;
        bspEnableRegion.in.startAddr    = ( pRegion->settings.range[0].address       ) & 0xFFFFFFFF;
        endOffset = pRegion->settings.range[0].address + pRegion->settings.range[0].size-1;
        bspEnableRegion.in.endAddrMsb =   ( endOffset >> 32 ) & 0xFF;
        bspEnableRegion.in.endAddr    =   endOffset & 0xFFFFFFFF;
        bspEnableRegion.in.dstStartAddrMsb = ( pRegion->settings.range[0].destAddress >> 32 ) & 0xFF;
        bspEnableRegion.in.dstStartAddr    = ( pRegion->settings.range[0].destAddress       ) & 0xFFFFFFFF;
    }

    if( pRegion->settings.range[1].size )
    {
        bspEnableRegion.in.secondRangeStartAddrMsb = ( pRegion->settings.range[1].address >> 32 ) & 0xFF;
        bspEnableRegion.in.secondRangeStartAddr = pRegion->settings.range[1].address & 0xFFFFFFFF;
        endOffset = pRegion->settings.range[1].address + pRegion->settings.range[1].size-1;
        bspEnableRegion.in.secondRangeEndAddrMsb = ( endOffset >> 32 ) & 0xFF;
        bspEnableRegion.in.secondRangeEndAddr = endOffset & 0xFFFFFFFF;
        bspEnableRegion.in.secondDstStartAddrMsb = ( pRegion->settings.range[1].destAddress >> 32 ) & 0xFF;
        bspEnableRegion.in.secondDstStartAddr    = ( pRegion->settings.range[1].destAddress       ) & 0xFFFFFFFF;
    }

    if( pRegion->settings.signature.size )
    {
        bspEnableRegion.in.sigStartAddrMsb = ( pRegion->settings.signature.address >> 32 ) & 0xFF;
        bspEnableRegion.in.sigStartAddr = pRegion->settings.signature.address & 0xFFFFFFFF;
    }

    if( pRegion->settings.parameters.address )
    {
        bspEnableRegion.in.sigMetaDataStartAddrMsb = ( pRegion->settings.parameters.address >> 32 ) & 0xFF;
        bspEnableRegion.in.sigMetaDataStartAddr = pRegion->settings.parameters.address & 0xFFFFFFFF;
    }

    if( pRegion->settings.intervalCheckBandwidth < 0x01 || pRegion->settings.intervalCheckBandwidth > 0x10 ) {
        return BERR_TRACE( BERR_INVALID_PARAMETER );
    }
    bspEnableRegion.in.intervalCheckBw = pRegion->settings.intervalCheckBandwidth;

    bspEnableRegion.in.secondRangeAvailable = pRegion->settings.range[1].size?true:false;

    bspEnableRegion.in.resetOnVerifyFailure = pRegion->settings.resetOnVerifyFailure ? Bsp_CmdRv_ResetOnVerifyFailure_eReset
                                                                                       : Bsp_CmdRv_ResetOnVerifyFailure_eNoReset;

    if( pRegion->settings.rvRsaHandle )
    {
        BHSM_RvRsaInfo rvRsaInfo;
        rc = BHSM_RvRsa_GetInfo( pRegion->settings.rvRsaHandle, &rvRsaInfo );
        if( rc != BERR_SUCCESS ) { return BERR_TRACE( rc ); }
        bspEnableRegion.in.rsaKeyId = rvRsaInfo.rsaKeyId;
    }

    if( pRegion->settings.keyLadderHandle )
    {
        BHSM_KeyLadderInfo keyLadderInfo;
        rc = BHSM_KeyLadder_GetInfo( pRegion->settings.keyLadderHandle, &keyLadderInfo );
        if( rc != BERR_SUCCESS ) { return BERR_TRACE( rc ); }
        bspEnableRegion.in.keyLayer = pRegion->settings.keyLadderLayer;
        bspEnableRegion.in.vklId = keyLadderInfo.index;
        bspEnableRegion.in.decryptionSel = Bsp_CmdRv_DecryptionMode_eKeyLadderKey;
    }
    bspEnableRegion.in.bgCheck = pRegion->settings.backgroundCheck ? Bsp_CmdRv_BgCheck_eEnable
                                                                     : Bsp_CmdRv_BgCheck_eDisable;
    bspEnableRegion.in.allowRegionDisable = pRegion->settings.allowRegionDisable ? Bsp_CmdRv_RegionDisable_eAllow
                                                                                   : Bsp_CmdRv_RegionDisable_eDisallow;
    bspEnableRegion.in.enforceAuth = pRegion->settings.enforceAuth ? Bsp_CmdRv_EnforceAuth_eEnforce
                                                                     : Bsp_CmdRv_EnforceAuth_eNoEnforce;

   #if BHSM_RV_REGION_EXTENSION
    rc = BHSM_RvRegion_CompileCommandExtension_priv( pRegion, &bspEnableRegion );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE( rc ); }
   #endif

    rc = BHSM_P_Rv_EnableRegion( pRegion->hHsm, &bspEnableRegion );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE( rc ); }

    BDBG_LEAVE( BHSM_RvRegion_Enable );
    return BERR_SUCCESS;
}

BERR_Code BHSM_RvRegion_QueryAll(BHSM_Handle hHsm, BHSM_RvRegionStatusAll *pStatus)
{
    BERR_Code rc = BERR_UNKNOWN;
    BHSM_P_RvQueryAllRegions regionsStatus; /* the status of all regions. */
    uint32_t numBspRegions = sizeof(regionsStatus.out.regionStatus)/sizeof(regionsStatus.out.regionStatus[0]);
    uint32_t i;

    BDBG_ENTER( BHSM_RvRegion_QueryAll );

    if( !hHsm ){ return BERR_TRACE( BERR_INVALID_PARAMETER ); }
    if( !pStatus ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }
    if( numBspRegions > BHSM_RegionId_eMax ) { return BERR_TRACE( BERR_NOT_SUPPORTED ); }

    BKNI_Memset( &regionsStatus, 0, sizeof(regionsStatus) );
    BKNI_Memset( pStatus, 0, sizeof(*pStatus) );

    rc = BHSM_P_Rv_QueryAllRegions( hHsm, &regionsStatus );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE( rc ); }

    for( i = 0; i < numBspRegions; i++) {
        pStatus->region[i].status = regionsStatus.out.regionStatus[i];
        pStatus->region[i].configured = hHsm->modules.pRvRegions->instances[i].configured;
    }

    BDBG_LEAVE( BHSM_RvRegion_QueryAll );
    return BERR_SUCCESS;
}

BERR_Code BHSM_RvRegion_GetStatus( BHSM_RvRegionHandle handle, BHSM_RvRegionStatus *pStatus )
{
    BHSM_P_RvRegion *pRegion = (BHSM_P_RvRegion*)handle;
    BHSM_P_RvQueryAllRegions regionsStatus; /* the status of all regions. */
    uint8_t regionId;
    BERR_Code rc = BERR_UNKNOWN;

    BDBG_ENTER( BHSM_RvRegion_GetStatus );

    BDBG_CASSERT( BHSM_RV_REGION_STATUS_ENABLED                == (1<<Bsp_CmdRv_QueryStatusBits_eEnabled) );
    BDBG_CASSERT( BHSM_RV_REGION_STATUS_AUTH_ENFORCED          == (1<<Bsp_CmdRv_QueryStatusBits_eAuthEnforce) );
    BDBG_CASSERT( BHSM_RV_REGION_STATUS_SAGE_OWNED             == (1<<Bsp_CmdRv_QueryStatusBits_eSageOwned) );
    BDBG_CASSERT( BHSM_RV_REGION_STATUS_LIVE_MERGE_IN_PROGRESS == (1<<Bsp_CmdRv_QueryStatusBits_eLiveMergeInProg) );
    BDBG_CASSERT( BHSM_RV_REGION_STATUS_LIVE_MERGE_FAILED        == (1<<Bsp_CmdRv_QueryStatusBits_eLiveMergeFail) );
    BDBG_CASSERT( BHSM_RV_REGION_STATUS_LIVE_MERGE_PASS        == (1<<Bsp_CmdRv_QueryStatusBits_eLiveMergePass) );
    BDBG_CASSERT( BHSM_RV_REGION_STATUS_FAST_CHECK_STARTED     == (1<<Bsp_CmdRv_QueryStatusBits_eFastChkStarted) );
    BDBG_CASSERT( BHSM_RV_REGION_STATUS_FAST_CHECK_FINISHED    == (1<<Bsp_CmdRv_QueryStatusBits_eFastChkFinished) );
    BDBG_CASSERT( BHSM_RV_REGION_STATUS_FAST_CHECK_FAILED      == (1<<Bsp_CmdRv_QueryStatusBits_eFastChkResult) );
    BDBG_CASSERT( BHSM_RV_REGION_STATUS_BG_CHECK_ENABLED       == (1<<Bsp_CmdRv_QueryStatusBits_eBgChkEnabled) );
    BDBG_CASSERT( BHSM_RV_REGION_STATUS_BG_CHECK_STARTED       == (1<<Bsp_CmdRv_QueryStatusBits_eBgChkStarted) );
    BDBG_CASSERT( BHSM_RV_REGION_STATUS_BG_CHECK_FINISHED      == (1<<Bsp_CmdRv_QueryStatusBits_eBgChkFinished) );
    BDBG_CASSERT( BHSM_RV_REGION_STATUS_BG_CHECK_FAILED        == (1<<Bsp_CmdRv_QueryStatusBits_eBgChkResult) );

    if( !pRegion ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }
    if( !pStatus ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }
    if( !pRegion->configured ) { return BERR_TRACE(BERR_NOT_INITIALIZED); }

    BKNI_Memset( pStatus, 0, sizeof(*pStatus) );

    pStatus->configured = true;

    BKNI_Memset( &regionsStatus, 0, sizeof(regionsStatus) );
    rc = BHSM_P_Rv_QueryAllRegions( pRegion->hHsm, &regionsStatus );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE( rc ); }

    regionId = pRegion->regionId;
    pStatus->status = regionsStatus.out.regionStatus[regionId];

    BDBG_LEAVE( BHSM_RvRegion_GetStatus );
    return BERR_SUCCESS;
}


BERR_Code BHSM_RvRegion_Disable( BHSM_RvRegionHandle handle )
{
    BHSM_P_RvRegion *pRegion = (BHSM_P_RvRegion*)handle;
    BHSM_P_RvDisableRegion disableRegion;
    BERR_Code rc = BERR_UNKNOWN;

    BDBG_ENTER( BHSM_RvRegion_Disable );

    if( !pRegion ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }
    if( !pRegion->allocated ) { return BERR_TRACE( BERR_NOT_INITIALIZED ); }

    BKNI_Memset( &disableRegion, 0, sizeof(disableRegion) );

    disableRegion.in.regionId = pRegion->regionId;

    rc = BHSM_P_Rv_DisableRegion( pRegion->hHsm, &disableRegion );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE( rc ); }

    BDBG_LEAVE( BHSM_RvRegion_Disable );
    return BERR_SUCCESS;
}


BERR_Code BHSM_GetRvRegionInfo( BHSM_RvRegionHandle handle, BHSM_RvRegionInfo *pInfo )
{
    BHSM_P_RvRegion *pRegion = (BHSM_P_RvRegion*)handle;

    BDBG_ENTER( BHSM_GetRvRegionInfo );

    if( !pRegion ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }
    if( !pRegion->allocated ) { return BERR_TRACE( BERR_NOT_INITIALIZED ); }
    if( !pInfo ) return BERR_TRACE( BERR_INVALID_PARAMETER );

    pInfo->regionId = pRegion->regionId;

    BDBG_LEAVE( BHSM_GetRvRegionInfo );
    return BERR_SUCCESS;
}
