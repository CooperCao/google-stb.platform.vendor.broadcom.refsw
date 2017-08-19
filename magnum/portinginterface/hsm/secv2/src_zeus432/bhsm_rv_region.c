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
#include "bkni.h"
#include "bhsm.h"
#include "bmma.h"
#include "berr_ids.h"
/*#include "bsp_types.h"*/
#include "bhsm_common.h"
#include "bhsm_rv_region.h"
#include "bhsm_priv.h"
#include "bhsm_rv_region_priv.h"


BDBG_MODULE( BHSM );


/* called internally on platform initialisation */
BERR_Code BHSM_RvRegion_Init( BHSM_Handle hHsm, BHSM_RvRegionModuleSettings *pSettings )
{
    BDBG_ENTER( BHSM_RvRegion_Init );

    BSTD_UNUSED( pSettings );

    hHsm->modules.pRvRegions = (BHSM_RvRegionModule*)BKNI_Malloc( sizeof(BHSM_RvRegionModule) );
    if( !hHsm->modules.pRvRegions ) { return BERR_TRACE( BERR_OUT_OF_SYSTEM_MEMORY ); }

    BKNI_Memset( hHsm->modules.pRvRegions, 0, sizeof(BHSM_RvRegionModule) );

    BDBG_LEAVE( BHSM_RvRegion_Init );
    return BERR_SUCCESS;
}


/* called internally on platform un-initialisation */
void BHSM_RvRegion_Uninit( BHSM_Handle hHsm )
{
    BDBG_ENTER( BHSM_RvRegion_Uninit );

    if( !hHsm->modules.pRvRegions ){
        BERR_TRACE( BERR_INVALID_PARAMETER );
        return;  /* function called out of sequence.  */
    }

    BKNI_Free( hHsm->modules.pRvRegions );
    hHsm->modules.pRvRegions = NULL;

    BDBG_LEAVE( BHSM_RvRegion_Uninit );
    return;
}



BHSM_RvRegionHandle BHSM_RvRegion_Allocate( BHSM_Handle hHsm, const BHSM_RvRegionAllocateSettings *pSettings )
{
    BHSM_P_RvRegion *pRegion = NULL;

    BDBG_ENTER( BHSM_RvRegion_Allocate );

    if( pSettings->regionId >= BHSM_RegionId_eMax ) { BERR_TRACE( BERR_INVALID_PARAMETER ); return NULL; }

    pRegion = &(((BHSM_RvRegionModule*)hHsm->modules.pRvRegions)->instances[pSettings->regionId]);

    if( pRegion->allocated ) { BERR_TRACE( BERR_NOT_AVAILABLE ); return NULL; }

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

    BKNI_Memset( pRegion, 0, sizeof(*pRegion) );
    pRegion->allocated = false;

    BDBG_LEAVE( BHSM_RvRegion_Free );
    return;
}


void  BHSM_RvRegion_GetSettings( BHSM_RvRegionHandle handle, BHSM_RvRegionSettings *pSettings )
{
    BHSM_P_RvRegion *pRegion = (BHSM_P_RvRegion*)handle;
    BDBG_ENTER( BHSM_RvRegion_GetSettings );

    if( !pRegion->allocated ) { BERR_TRACE( BERR_INVALID_PARAMETER ); return; }

    *pSettings = pRegion->settings;

    BDBG_LEAVE( BHSM_RvRegion_GetSettings );
    return;
}


BERR_Code BHSM_RvRegion_SetSettings( BHSM_RvRegionHandle handle, const BHSM_RvRegionSettings *pSettings )
{
    BHSM_P_RvRegion *pRegion = (BHSM_P_RvRegion*)handle;
    BDBG_ENTER( BHSM_RvRegion_SetSettings );

    if( !pRegion->allocated ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }

    pRegion->settings = *pSettings;

    pRegion->configured = true;

    BDBG_LEAVE( BHSM_RvRegion_SetSettings );
    return BERR_SUCCESS;
}

BERR_Code BHSM_RvRegion_Enable( BHSM_RvRegionHandle handle )
{

    BSTD_UNUSED( handle );
#if 0
    BHSM_P_RvRegion *pRvRegion = (BHSM_P_RvRegion*)handle;
    BHSM_P_RvEnableRegion bspEnableRegion;
    BMMA_DeviceOffset endOffset;
    BERR_Code rc;

    BDBG_ENTER( BHSM_RvRegion_Enable );

    BKNI_Memset( &bspEnableRegion, 0, sizeof(bspEnableRegion) );

    bspEnableRegion.in.regionId = pRvRegion->regionId;

    if( pRvRegion->settings.range[0].size )
    {
        bspEnableRegion.in.startAddrMsb = ( pRvRegion->settings.range[0].address >> 32 ) & 0xFF;
        bspEnableRegion.in.startAddr    = ( pRvRegion->settings.range[0].address       ) & 0xFFFFFFFF;
        endOffset = pRvRegion->settings.range[0].address + pRvRegion->settings.range[0].size-1;
        bspEnableRegion.in.endAddrMsb =   ( endOffset >> 32 ) & 0xFF;
        bspEnableRegion.in.endAddr    =   endOffset & 0xFFFFFFFF;
    }

    if( pRvRegion->settings.range[1].size )
    {
        bspEnableRegion.in.secondRangeStartAddrMsb = ( pRvRegion->settings.range[1].address >> 32 ) & 0xFF;
        bspEnableRegion.in.secondRangeStartAddr = pRvRegion->settings.range[1].address & 0xFFFFFFFF;
        endOffset = pRvRegion->settings.range[1].address + pRvRegion->settings.range[1].size-1;
        bspEnableRegion.in.secondRangeEndAddrMsb = ( endOffset >> 32 ) & 0xFF;
        bspEnableRegion.in.secondRangeEndAddr = endOffset & 0xFFFFFFFF;
    }

    if( pRvRegion->settings.signature.size )
    {
        bspEnableRegion.in.sigStartAddrMsb = ( pRvRegion->settings.signature.address >> 32 ) & 0xFF;
        bspEnableRegion.in.sigStartAddr = pRvRegion->settings.signature.address & 0xFFFFFFFF;
        endOffset = pRvRegion->settings.signature.address + pRvRegion->settings.signature.size-1;
        bspEnableRegion.in.sigEndAddrMsb = ( endOffset >> 32 ) & 0xFF;
        bspEnableRegion.in.sigEndAddr = endOffset & 0xFFFFFFFF;
    }

    if( pRvRegion->settings.intervalCheckBandwidth < 0x01 || pRvRegion->settings.intervalCheckBandwidth > 0x10 ) {
        return BERR_TRACE( BERR_INVALID_PARAMETER );
    }
    bspEnableRegion.in.intervalCheckBw = pRvRegion->settings.intervalCheckBandwidth;

    if( pRvRegion->settings.resetOnVerifyFailure ){
        bspEnableRegion.in.resetOnVerifyFailure = 0x05; /* Bsp_CmdRv_ResetOnVerifyFailure_eReset */
    }

    if( pRvRegion->settings.rvRsaHandle )
    {
        BHSM_RvRsaInfo rvRsaInfo;
        rc = BHSM_RvRsa_GetInfo( pRvRegion->settings.rvRsaHandle, &rvRsaInfo );
        if( rc != BERR_SUCCESS ) { return BERR_TRACE( rc ); }
        bspEnableRegion.in.rsaKeyId = rvRsaInfo.rsaKeyId;
    }

    if( pRvRegion->settings.keyLadderHandle )
    {
        BHSM_KeyLadderInfo keyLadderInfo;
        rc = BHSM_KeyLadder_GetInfo( pRvRegion->settings.keyLadderHandle, &keyLadderInfo );
        if( rc != BERR_SUCCESS ) { return BERR_TRACE( rc ); }
        bspEnableRegion.in.keyLayer = pRvRegion->settings.keyLadderLayer;
        bspEnableRegion.in.vklId = keyLadderInfo.index;
    }

    switch( pRvRegion->regionId )
    {
		case BHSM_RegionId_eCpu0:
		case BHSM_RegionId_eCpu1:
		case BHSM_RegionId_eCpu2:
		case BHSM_RegionId_eCpu3:
		case BHSM_RegionId_eCpu4:
		case BHSM_RegionId_eCpu5:
		case BHSM_RegionId_eCpu6:
		case BHSM_RegionId_eCpu7:           { bspEnableRegion.in.cpuType = 0; /* HOST*/ break; 	}
		case BHSM_RegionId_eRave0:          { bspEnableRegion.in.cpuType = 3;  /*Rave*/ break; }
		case BHSM_RegionId_eAudio0:         { bspEnableRegion.in.cpuType = 1;  /* Audio */ break; }
		case BHSM_RegionId_eVdec0_Il2a:
		case BHSM_RegionId_eVdec0_Ila:
		case BHSM_RegionId_eVdec0_Ola:
		case BHSM_RegionId_eVdec1_Ila:
		case BHSM_RegionId_eVdec1_Ola:      { bspEnableRegion.in.cpuType = 4;  /* VDEC */ break; }
		case BHSM_RegionId_eVice0_Pic:
		case BHSM_RegionId_eVice0_Mb:       { bspEnableRegion.in.cpuType = 5;  /* VICE */ break; }
		case BHSM_RegionId_eRedacted_0x0A:
		case BHSM_RegionId_eRedacted_0x0E:
		case BHSM_RegionId_eRedacted_0x11:
		case BHSM_RegionId_eRedacted_0x12:
		case BHSM_RegionId_eRedacted_0x13:
		case BHSM_RegionId_eRedacted_0x14:
		case BHSM_RegionId_eRedacted_0x17:
		case BHSM_RegionId_eRedacted_0x18:
		case BHSM_RegionId_eRedacted_0x19:
		case BHSM_RegionId_eRedacted_0x1A:
		case BHSM_RegionId_eRedacted_0x1B:
		case BHSM_RegionId_eRedacted_0x1C:
		case BHSM_RegionId_eRedacted_0x1D:
		case BHSM_RegionId_eRedacted_0x1E:  { bspEnableRegion.in.cpuType = 7; break; }
        default: { return BERR_TRACE( BERR_INVALID_PARAMETER ); }
    }

    bspEnableRegion.in.sigType = 4; /* if we do need to support more sigTyes, we can switch on regionId */
                                    /* Keys:1;   BootCode:2;   Code:4;   AssymUnlock:5;   PcieWinSize:6; */
    bspEnableRegion.in.sigVersion = 2;

    bspEnableRegion.in.instrCheck = pRvRegion->settings.instrCheck?0x0C:0;
    bspEnableRegion.in.bgCheck = pRvRegion->settings.backgroundCheck?0x0D:0;
    bspEnableRegion.in.allowRegionDisable = pRvRegion->settings.allowRegionDisable?0x0E:0;
    bspEnableRegion.in.enforceAuth = pRvRegion->settings.enforceAuth?0x0B:0;

    rc = BHSM_P_Rv_EnableRegion( pRvRegion->hHsm, &bspEnableRegion );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE( rc ); }

#endif
    BDBG_LEAVE( BHSM_RvRegion_Enable );
    return BERR_SUCCESS;
}

BERR_Code BHSM_RvRegion_GetStatus( BHSM_RvRegionHandle handle, BHSM_RvRegionStatus *pStatus )
{
    BSTD_UNUSED( handle );
    BSTD_UNUSED( pStatus );
#if 0
    BHSM_P_RvRegion *pRegion = (BHSM_P_RvRegion*)handle;
    BHSM_P_RvQueryAllRegions regionsStatus; /* the status if all regions. */
    uint8_t regionId;
    BERR_Code rc;

    BDBG_ENTER( BHSM_RvRegion_GetStatus );

    if( !pStatus ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }

    BKNI_Memset( pStatus, 0, sizeof(*pStatus) );

    if( !pRegion->configured ) { return BERR_SUCCESS; } /*nothing more to do.*/

    pStatus->configured = true;

    BKNI_Memset( &regionsStatus, 0, sizeof(regionsStatus) );
    rc = BHSM_P_Rv_QueryAllRegions( pRegion->hHsm, &regionsStatus );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE( rc ); }

    regionId = pRegion->regionId;

    if( regionsStatus.out.regionStatus[regionId] & 0x01 ){ /* Bsp_CmdRv_QueryStatusBits_eEnabled  */
        pStatus->verified = true;
    }
#endif

    BDBG_LEAVE( BHSM_RvRegion_GetStatus );
    return BERR_SUCCESS;
}


BERR_Code BHSM_RvRegion_Disable( BHSM_RvRegionHandle handle )
{
    BSTD_UNUSED( handle );
#if 0
    BHSM_P_RvRegion *pRegion = (BHSM_P_RvRegion*)handle;
    BHSM_P_RvDisableRegion disableRegion;
    BERR_Code rc;

    BDBG_ENTER( BHSM_RvRegion_Disable );

    if( !pRegion->allocated ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }

    BKNI_Memset( &disableRegion, 0, sizeof(disableRegion) );

    disableRegion.in.regionId = pRegion->regionId;

    rc = BHSM_P_Rv_DisableRegion( pRegion->hHsm, &disableRegion );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE( rc ); }
#endif
    BDBG_LEAVE( BHSM_RvRegion_Disable );
    return BERR_SUCCESS;
}


BERR_Code BHSM_GetRvRegionInfo( BHSM_RvRegionHandle handle, BHSM_RvRegionInfo *pInfo )
{
    BHSM_P_RvRegion *pRegion = (BHSM_P_RvRegion*)handle;

    BDBG_ENTER( BHSM_GetRvRegionInfo );

    if( !pInfo ) return BERR_TRACE( BERR_INVALID_PARAMETER );
    if( !pRegion->allocated ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }

    pInfo->regionId = pRegion->regionId;

    BDBG_LEAVE( BHSM_GetRvRegionInfo );
    return BERR_SUCCESS;
}
