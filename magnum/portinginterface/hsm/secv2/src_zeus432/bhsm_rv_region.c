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
#include "bmma.h"
#include "berr_ids.h"
#include "bhsm.h"
#include "bhsm_priv.h"
#include "bhsm_common.h"
#include "bhsm_rv_region.h"
#include "bhsm_rv_region_priv.h"
#include "bhsm_bsp_msg.h"
#include "bsp_s_commands.h"
#include "bsp_s_mem_auth.h"
#include "bsp_s_keycommon.h"
#include "bsp_s_download.h"


BDBG_MODULE( BHSM );


#define BHSM_eSESSION_MEM_AUTH_DisableRegion    BCMD_cmdType_eSESSION_MEM_AUTH
#define BHSM_eSESSION_MEM_AUTH_ConfigureRegion  BCMD_cmdType_eSESSION_MEM_AUTH
#define BHSM_eSESSION_MEM_AUTH_Enable           BCMD_cmdType_eSESSION_MEM_AUTH
#define BHSM_eSESSION_MEM_AUTH_StatusRegion     BCMD_cmdType_eSESSION_MEM_AUTH
#define BHSM_eSESSION_MEM_AUTH_QueryStatus      BCMD_cmdType_eSESSION_MEM_AUTH
#define BHSM_eSESSION_MEM_AUTH_IsRegionVerified BCMD_cmdType_eSESSION_MEM_AUTH

#define BHSM_BSP_ERROR_ALREADY_CONFIGURED  0xB0
#define BHSM_BSP_ERROR_NOT_OTP_ENABLED     0xD4
#define BHSM_BSP_ERROR_ALREADY_DISABLED    0xB1
#define BHSM_BSP_ERROR_NOT_DEFINED         0xB7



typedef struct
{
/* The upper and lower addressed of a memory region */
    struct {
        BMMA_DeviceOffset address;
        unsigned size;
    } region,signature;

    bool                 resetOnVerifyFailure;   /* Applies to Region 2 SSBL; BSP FW does chip reset upon verification failure */
    unsigned             scbBurstSize;           /* Effective MIPS Cache line fill size */
    unsigned             intervalCheckBandwidth; /* Number of interval checks to perform every 16 cycles (valid values 1-16) */
    unsigned             keyLadderIndex;         /* SCPU Virtual key ladder for key used in SCPU FSBL decryption */
    unsigned             keyLadderLayer;         /* SCPU key layer for key used in SCPU FSBL decryption */
    unsigned             rsaKeyId;               /* RSA Key ID  */
    unsigned             svpFwReleaseVersion;    /* The SVP version of the FW. Applicable for BFW400+ */
    bool                 codeRelocatable;        /* whether this region should have non-relocatable code  */
    uint32_t             marketId;               /* Market ID  >= ZEUS 2.0 */
    uint32_t             marketIdMask;           /* Market ID Mask  if >= ZEUS 2.0 */
    unsigned             epochSelect;            /* Selected Epoch */
    uint32_t             epochMask;              /* Epoch Mask  if >= ZEUS 2.0 */
    uint32_t             epoch;                  /* Epoch for the code  --- to be checked with the OTP Epoch */
    bool                 enforceAuth;            /* Force region authentication irrespective of OTP. */
    bool                 allowRegionDisable;     /* Disallow the region to be disabled once enalbed. */
    bool                 backgroundCheck;        /* SCPU. True to enable background checking for the region  */
    bool                 instrCheck;             /* SCPU  True to enable instruction checker for the region */
    unsigned             signatureVersion;       /* Signature Version */
    BHSM_RvSignatureType signatureType;          /* Signature type */

} _RvRegionConfigure_t;



BERR_Code _RvRegionConfigure( BHSM_RvRegionHandle hRegion, _RvRegionConfigure_t *pConf );
static uint8_t _BspCpuType( BHSM_RegionId regionId );



/* called internally on platform initialisation */
BERR_Code BHSM_RvRegion_Init( BHSM_Handle hHsm, BHSM_RvRegionModuleSettings *pSettings )
{
    BDBG_ENTER( BHSM_RvRegion_Init );
    BSTD_UNUSED( pSettings );

    hHsm->modules.pRvRegions = (BHSM_RvRegionModule*)BKNI_Malloc( sizeof(BHSM_RvRegionModule) );
    if( !hHsm->modules.pRvRegions ) { return BERR_TRACE( BERR_OUT_OF_SYSTEM_MEMORY ); }

    BKNI_Memset( hHsm->modules.pRvRegions, 0, sizeof(BHSM_RvRegionModule) );
    hHsm->modules.pRvRegions->hHsm = hHsm;

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
    BERR_Code rc = BERR_UNKNOWN;
    BHSM_P_RvRegion *pRegion = (BHSM_P_RvRegion*)handle;
    _RvRegionConfigure_t bspRvConfig;
    BHSM_RvRsaInfo rsaInfo;

    BDBG_ENTER( BHSM_RvRegion_SetSettings );

    if( !pRegion->allocated ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }
    if( !pSettings ) return BERR_TRACE(BERR_INVALID_PARAMETER );
    if( !pSettings->rvRsaHandle ) return BERR_TRACE(BERR_INVALID_PARAMETER );

    pRegion->settings = *pSettings;

    rc = BHSM_RvRsa_GetInfo( pSettings->rvRsaHandle, &rsaInfo );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE( rc ); }

    BKNI_Memset( &bspRvConfig, 0, sizeof(bspRvConfig) );
    bspRvConfig.region.address = pSettings->range[0].address;
    bspRvConfig.region.size = pSettings->range[0].size;
    if( pSettings->keyLadderHandle )
    {
        BHSM_KeyLadderInfo keyLadderInfo;
        rc = BHSM_KeyLadder_GetInfo( pSettings->keyLadderHandle, &keyLadderInfo );
        if( rc != BERR_SUCCESS ) { return BERR_TRACE( rc ); }

        bspRvConfig.keyLadderIndex = keyLadderInfo.index;
        bspRvConfig.keyLadderLayer = pSettings->keyLadderLayer;
    }
    bspRvConfig.rsaKeyId = rsaInfo.rsaKeyId;
    bspRvConfig.scbBurstSize = 0; /* default */
    bspRvConfig.intervalCheckBandwidth = pSettings->intervalCheckBandwidth;
    bspRvConfig.resetOnVerifyFailure = pSettings->resetOnVerifyFailure;
    bspRvConfig.enforceAuth = pSettings->enforceAuth;
    bspRvConfig.allowRegionDisable = pSettings->allowRegionDisable;
    bspRvConfig.backgroundCheck = pSettings->backgroundCheck;
    bspRvConfig.instrCheck = pSettings->instrCheck;
    bspRvConfig.signatureType = pSettings->signatureType;
    bspRvConfig.signatureVersion = pSettings->signatureVersion;
    bspRvConfig.codeRelocatable = pSettings->codeRelocatable;
    bspRvConfig.marketId = pSettings->marketId;
    bspRvConfig.marketIdMask = pSettings->marketIdMask;
    bspRvConfig.epochSelect = pSettings->epochSelect;
    bspRvConfig.epochMask = pSettings->epochMask;
    bspRvConfig.epochMask = pSettings->epochMask;
    bspRvConfig.epoch = pSettings->epoch;

    rc = _RvRegionConfigure( handle, &bspRvConfig );
    switch( rc ) {
        case BERR_SUCCESS:
        case BHSM_STATUS_REGION_ALREADY_CONFIGURED:
        case BHSM_STATUS_REGION_VERIFICATION_NOT_ENABLED: break;
        default: BERR_TRACE( rc ); break;
    }

    pRegion->configured = true;
    BDBG_LEAVE( BHSM_RvRegion_SetSettings );
    return BERR_SUCCESS;
}

BERR_Code BHSM_RvRegion_Enable( BHSM_RvRegionHandle handle )
{
    BERR_Code rc = BERR_UNKNOWN;
    BHSM_P_RvRegion *pRegion = (BHSM_P_RvRegion*)handle;
    BHSM_BspMsg_h hMsg = NULL;
    BHSM_BspMsgHeader_t header;
    uint8_t status = 0;

    BDBG_ENTER( BHSM_RvRegion_Enable );

    rc = BHSM_BspMsg_Create( pRegion->hHsm, &hMsg );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE( rc ); }

    BHSM_BspMsg_GetDefaultHeader( &header );
    BHSM_BspMsg_Header( hMsg, BHSM_eSESSION_MEM_AUTH_Enable, &header );

    BHSM_BspMsg_Pack8( hMsg, BCMD_MemAuth_InCmdField_eRegionOp, BCMD_MemAuth_Operation_eDefineRegion );

    rc = BHSM_BspMsg_SubmitCommand( hMsg );
    if( rc != BERR_SUCCESS ) { BERR_TRACE(rc); goto exit; }

    BHSM_BspMsg_Get8( hMsg, BCMD_MemAuth_OutCmdField_eStatus, &status );
    if( status != BERR_SUCCESS ) {
        rc = BERR_TRACE( BHSM_STATUS_BSP_ERROR );
        BDBG_WRN(("%s: ERROR[0x%0X]", BSTD_FUNCTION, status ));
        goto exit;
    }

    BDBG_MSG(("%s: Enable Verification.", BSTD_FUNCTION ));
exit:

    BHSM_BspMsg_Destroy( hMsg );

    BDBG_LEAVE( BHSM_RvRegion_Enable );
    return rc;
}

BERR_Code BHSM_RvRegion_GetStatus( BHSM_RvRegionHandle handle, BHSM_RvRegionStatus *pStatus )
{
    BERR_Code rc = BERR_SUCCESS;
    BHSM_BspMsg_h hMsg = NULL;
    BHSM_BspMsgHeader_t header;
    BHSM_P_RvRegion *pRegion = (BHSM_P_RvRegion*)handle;
    uint8_t status = 0;
    uint16_t regionStatus = 0;
    unsigned regionId;

    BDBG_ENTER( BHSM_RegionVerification_QueryStatus );

    if( !pStatus ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }

    rc = BHSM_BspMsg_Create( pRegion->hHsm, &hMsg );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE( rc ); }

    BHSM_BspMsg_GetDefaultHeader( &header );
    BHSM_BspMsg_Header( hMsg, BHSM_eSESSION_MEM_AUTH_QueryStatus, &header );

    BHSM_BspMsg_Pack8( hMsg, BCMD_MemAuth_InCmdField_eRegionOp, BCMD_MemAuth_Operation_eQueryRegionInfo );

    rc = BHSM_BspMsg_SubmitCommand( hMsg );
    if( rc != BERR_SUCCESS ) { BERR_TRACE( rc ); goto exit; }

    BHSM_BspMsg_Get8( hMsg, BCMD_MemAuth_OutCmdField_eStatus, &status );
    if( status ) {
        rc = BERR_TRACE( BHSM_STATUS_BSP_ERROR );
        BDBG_WRN(("%s: ERROR[0x%0X] ", BSTD_FUNCTION, status ));
        goto exit;
    }

    regionId = pRegion->regionId;
    #define BHSM_eStatusQuery ((7<<2) + 2) /* BCMD_MemAuth_OutCmdField_eRegion0Status is one byte off.  */
    BHSM_BspMsg_Get16( hMsg, BHSM_eStatusQuery+(regionId*4), &regionStatus );

    pStatus->configured = pRegion->configured;
    pStatus->status = regionStatus;

exit:
    BHSM_BspMsg_Destroy( hMsg );

    BDBG_LEAVE( BHSM_RvRegion_GetStatus );
    return rc;
}


BERR_Code BHSM_RvRegion_Disable( BHSM_RvRegionHandle handle )
{
    BERR_Code rc = BERR_SUCCESS;
    BHSM_P_RvRegion *pRegion = (BHSM_P_RvRegion*)handle;
    BHSM_BspMsg_h hMsg = NULL;
    BHSM_BspMsgHeader_t header;
    uint8_t status = 0;

    BDBG_ENTER( BHSM_RegionVerification_Disable );

    rc = BHSM_BspMsg_Create( pRegion->hHsm, &hMsg );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE( rc ); }

    BHSM_BspMsg_GetDefaultHeader( &header );
    BHSM_BspMsg_Header( hMsg, BHSM_eSESSION_MEM_AUTH_DisableRegion, &header );
    BHSM_BspMsg_Pack8( hMsg, BCMD_MemAuth_InCmdField_eRegionOp, BCMD_MemAuth_Operation_eDisableRegion );
    BHSM_BspMsg_Pack8( hMsg, BCMD_MemAuth_InCmdField_eRegionNum, pRegion->regionId );
    rc = BHSM_BspMsg_SubmitCommand( hMsg );
    if( rc != BERR_SUCCESS ) { BERR_TRACE(rc); goto exit; }

    BHSM_BspMsg_Get8( hMsg, BCMD_MemAuth_OutCmdField_eStatus, &status );
    switch( status ) {
        case 0:
        case BHSM_BSP_ERROR_ALREADY_DISABLED:
        case BHSM_BSP_ERROR_NOT_DEFINED: {
            BDBG_MSG(("%s: Region[0x%02X]  Disabled[0x%02X]", BSTD_FUNCTION, pRegion->regionId, status ));
            break; /* success */
        }
        case BHSM_BSP_ERROR_NOT_OTP_ENABLED: {
            rc = BHSM_STATUS_REGION_VERIFICATION_NOT_ENABLED;
            BDBG_WRN(("%s: Region [0x%02X] Verification not enabled in OTP", BSTD_FUNCTION, pRegion->regionId ));
            break;
        }
        default: {
            rc = BERR_TRACE( BHSM_STATUS_BSP_ERROR );
            BDBG_WRN(("%s: error[0x%0X] region[0x%02X]", BSTD_FUNCTION, status, pRegion->regionId ));
            break;
        }
    }

exit:

    BHSM_BspMsg_Destroy( hMsg );

    BDBG_LEAVE( BHSM_RvRegion_Disable );
    return rc;
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


BERR_Code _RvRegionConfigure( BHSM_RvRegionHandle hRegion, _RvRegionConfigure_t *pConf )
{
    BERR_Code rc = BERR_SUCCESS;
    BHSM_P_RvRegion *pRegion = (BHSM_P_RvRegion*)hRegion;
    BHSM_BspMsg_h hMsg = NULL;
    BHSM_BspMsgHeader_t header;
    uint8_t status = 0;
    BCMD_ScbBurstSize_e scbBurstSize = 0;
    BCMD_MemAuth_CpuType_e cpuType;
    BCMD_SignatureType_e signatureType;
    BMMA_DeviceOffset endAddress;

    BDBG_ENTER( _RvRegionConfigure );

    rc = BHSM_BspMsg_Create( pRegion->hHsm, &hMsg );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE( rc ); }

    /* Configure BSP message header */
    BHSM_BspMsg_GetDefaultHeader( &header );
    BHSM_BspMsg_Header( hMsg, BHSM_eSESSION_MEM_AUTH_ConfigureRegion, &header );

    BHSM_BspMsg_Pack8( hMsg, BCMD_MemAuth_InCmdField_eRegionOp, BCMD_MemAuth_Operation_eEnableRegion );
    BHSM_BspMsg_Pack8( hMsg, BCMD_MemAuth_InCmdField_eRegionNum, (uint8_t)pRegion->regionId );

    /* region address */
    BHSM_BspMsg_Pack32( hMsg, BCMD_MemAuth_InCmdField_eStartAddrMsb,    ((pConf->region.address >> 32) & 0xFF) );
    BHSM_BspMsg_Pack32( hMsg, BCMD_MemAuth_InCmdField_eStartAddr,       ( pConf->region.address & 0xFFFFFFFF) );
    endAddress = pConf->region.address + pConf->region.size - 1;
    BHSM_BspMsg_Pack32( hMsg, BCMD_MemAuth_InCmdField_eEndAddrMsb,      ((endAddress >> 32) & 0xFF) );
    BHSM_BspMsg_Pack32( hMsg, BCMD_MemAuth_InCmdField_eEndAddr,         ( endAddress & 0xFFFFFFFF) );

    /* signature address */
    BHSM_BspMsg_Pack32( hMsg, BCMD_MemAuth_InCmdField_eSigStartAddrMsb, ((pConf->signature.address >> 32) & 0xFF) );
    BHSM_BspMsg_Pack32( hMsg, BCMD_MemAuth_InCmdField_eSigStartAddr,    ( pConf->signature.address & 0xFFFFFFFF) );
    endAddress = pConf->signature.address + pConf->signature.size - 1;
    BHSM_BspMsg_Pack32( hMsg, BCMD_MemAuth_InCmdField_eSigEndAddrMsb,   ((endAddress >> 32) & 0xFF) );
    BHSM_BspMsg_Pack32( hMsg, BCMD_MemAuth_InCmdField_eSigEndAddr,      ( endAddress & 0xFFFFFFFF) );

    if( pConf->intervalCheckBandwidth < 0x01 || pConf->intervalCheckBandwidth > 0x10 ) {
        rc = BERR_TRACE( BERR_INVALID_PARAMETER ); goto exit;
    }
    BHSM_BspMsg_Pack8 ( hMsg, BCMD_MemAuth_InCmdField_eIntervalCheckBw, (uint8_t)pConf->intervalCheckBandwidth );

    switch( pConf->scbBurstSize ){
        case 0:   /* allow a defult. map to 64. */
        case 64:  scbBurstSize = BCMD_ScbBurstSize_e64;  break;
        case 128: scbBurstSize = BCMD_ScbBurstSize_e128; break;
        case 256: scbBurstSize = BCMD_ScbBurstSize_e256; break;
        default: { rc = BERR_TRACE( BERR_INVALID_PARAMETER ); goto exit; }
    }
    BHSM_BspMsg_Pack8 ( hMsg, BCMD_MemAuth_InCmdField_eScbBurstSize, (uint8_t)pConf->scbBurstSize );

    BHSM_BspMsg_Pack8( hMsg, BCMD_MemAuth_InCmdField_eResetOnVerifyFailure, pConf->resetOnVerifyFailure ? BCMD_MemAuth_ResetOnVerifyFailure_eReset
                                                                                                  : BCMD_MemAuth_ResetOnVerifyFailure_eNoReset );
    cpuType = _BspCpuType( pRegion->regionId );

    if( cpuType == BCMD_MemAuth_CpuType_eScpu )
    {
        if( pConf->keyLadderIndex >= BCMD_VKL_KeyRam_eMax ) { rc = BERR_TRACE( BERR_INVALID_PARAMETER ); goto exit; }
        BHSM_BspMsg_Pack8( hMsg, BCMD_MemAuth_InCmdField_eVKLId,             (uint8_t)pConf->keyLadderIndex );

        if( pConf->keyLadderLayer < BCMD_KeyRamBuf_eKey3 ){ rc = BERR_TRACE( BERR_INVALID_PARAMETER ); goto exit; }
        if( pConf->keyLadderLayer >= BCMD_KeyRamBuf_eMax ){ rc = BERR_TRACE( BERR_INVALID_PARAMETER ); goto exit; }
        BHSM_BspMsg_Pack8( hMsg, BCMD_MemAuth_InCmdField_eKeyLayer,          (uint8_t)pConf->keyLadderLayer );
    }

    if( pConf->rsaKeyId >= BCMD_SecondTierKeyId_eMax ) { rc = BERR_TRACE( BERR_INVALID_PARAMETER ); goto exit; }
    BHSM_BspMsg_Pack8( hMsg, BCMD_MemAuth_InCmdField_eRsaKeyId,            (uint8_t)pConf->rsaKeyId );
    BHSM_BspMsg_Pack8( hMsg, BCMD_MemAuth_InCmdField_eNoRelocatableCode,   pConf->codeRelocatable?0:1 ); /* 0..relocatable. 1..address part of signature.  */
    BHSM_BspMsg_Pack8( hMsg, BCMD_MemAuth_InCmdField_eCpuType,             (uint8_t)cpuType );
    BHSM_BspMsg_Pack8( hMsg, BCMD_MemAuth_InCmdField_eSvpFwReleaseVersion, (uint8_t)pConf->svpFwReleaseVersion );
    BHSM_BspMsg_Pack8( hMsg, BCMD_MemAuth_InCmdField_eEpoch,                pConf->epoch );
    BHSM_BspMsg_Pack8( hMsg, BCMD_MemAuth_InCmdField_eEnforceAuthentication, pConf->enforceAuth ? BCMD_MemAuth_EnforceAuthentication_Enforce
                                                                                                : BCMD_MemAuth_EnforceAuthentication_NoEnforce );
    BHSM_BspMsg_Pack8( hMsg, BCMD_MemAuth_InCmdField_eAllowRegionDisable,  pConf->allowRegionDisable ? BCMD_MemAuth_RegionDisable_Allow
                                                                                                     : BCMD_MemAuth_RegionDisable_Disallow );
    BHSM_BspMsg_Pack32( hMsg, BCMD_MemAuth_InCmdField_eMarketID,           pConf->marketId );
    BHSM_BspMsg_Pack32( hMsg, BCMD_MemAuth_InCmdField_eMarketIDMask,       pConf->marketIdMask );
    BHSM_BspMsg_Pack8( hMsg, BCMD_MemAuth_InCmdField_eEpochMask,           (uint8_t)pConf->epochMask);
    BHSM_BspMsg_Pack8( hMsg, BCMD_MemAuth_InCmdField_eBgCheck,             pConf->backgroundCheck ? BCMD_MemAuth_BgCheck_eEnable
                                                                                                  : BCMD_MemAuth_BgCheck_eDisable );
    BHSM_BspMsg_Pack8( hMsg, BCMD_MemAuth_InCmdField_eInstrCheck,          pConf->instrCheck ? BCMD_MemAuth_InstrCheck_eEnable
                                                                                             : BCMD_MemAuth_InstrCheck_eDisable );
    BHSM_BspMsg_Pack8( hMsg, BCMD_MemAuth_InCmdField_eEpochSel,            (uint8_t)pConf->epochSelect );
    BHSM_BspMsg_Pack8( hMsg, BCMD_MemAuth_InCmdField_eSigVersion,          (uint8_t)pConf->signatureVersion );

    switch( pConf->signatureType ) {
        case BHSM_RvSignatureType_eKeys:            signatureType = BCMD_SigType_eKeys; break;
        case BHSM_RvSignatureType_eCode:            signatureType = BCMD_SigType_eCode; break;
        case BHSM_RvSignatureType_ePciEWinSize:     signatureType = BCMD_SigType_ePCIEWinSize; break;
        default: { rc = BERR_TRACE( BERR_INVALID_PARAMETER ); goto exit; } /* you may have to add type[s] to BHSM_RvSignatureType */
    }
    BHSM_BspMsg_Pack8( hMsg, BCMD_MemAuth_InCmdField_eSigType, (uint8_t)signatureType );

    rc = BHSM_BspMsg_SubmitCommand( hMsg );
    if( rc != BERR_SUCCESS ) { BERR_TRACE( rc ); goto exit; }

    BHSM_BspMsg_Get8( hMsg, BCMD_MemAuth_OutCmdField_eStatus, &status );
    switch( status ) {
        case 0: {
            BDBG_MSG(("%s: Region [0x%02X] Configured.", BSTD_FUNCTION, pRegion->regionId ));
            break; /* success */
        }
        case BHSM_BSP_ERROR_ALREADY_CONFIGURED: {
            BDBG_WRN(("%s: Region [0x%02X] Already Configured.", BSTD_FUNCTION, pRegion->regionId ));
            rc = BHSM_STATUS_REGION_ALREADY_CONFIGURED;
            break;
        }
       case BHSM_BSP_ERROR_NOT_OTP_ENABLED: {
            rc = BHSM_STATUS_REGION_VERIFICATION_NOT_ENABLED;
            BDBG_WRN(("%s: Region [0x%02X] Verification not enalbed in OTP", BSTD_FUNCTION, pRegion->regionId ));
            break;
        }
        default: {
            rc = BHSM_STATUS_BSP_ERROR;
            BDBG_WRN(("%s: ERROR[0x%0X] region[0x%0X]", BSTD_FUNCTION, status, pRegion->regionId ));
            break;
        }
    }

exit:
    BHSM_BspMsg_Destroy( hMsg );

    BDBG_LEAVE( _RvRegionConfigure );
    return rc;
}



static uint8_t _BspCpuType( BHSM_RegionId regionId )
{
    switch( regionId ) {
        case BHSM_RegionId_eCpu0:
        case BHSM_RegionId_eCpu1:
        case BHSM_RegionId_eCpu2:
        case BHSM_RegionId_eCpu3:
        case BHSM_RegionId_eCpu4:
        case BHSM_RegionId_eCpu5:
        case BHSM_RegionId_eCpu6:
        case BHSM_RegionId_eCpu7:           return BCMD_MemAuth_CpuType_eHost;
        case BHSM_RegionId_eRave0:          return BCMD_MemAuth_CpuType_eRave;
        case BHSM_RegionId_eAudio0:         return BCMD_MemAuth_CpuType_eRaaga;
        case BHSM_RegionId_eVdec0_Il2a:
        case BHSM_RegionId_eVdec0_Ila:
        case BHSM_RegionId_eVdec0_Ola:
        case BHSM_RegionId_eVdec1_Ila:
        case BHSM_RegionId_eVdec1_Ola:      return BCMD_MemAuth_CpuType_eVdec;
        case BHSM_RegionId_eVice0_Pic:
        case BHSM_RegionId_eVice0_Mb:       return BCMD_MemAuth_CpuType_eVice;
        case BHSM_RegionId_eRedacted_eSid:  return BCMD_MemAuth_CpuType_eSid;
        case BHSM_RegionId_eRedacted_0x0A:
        case BHSM_RegionId_eRedacted_0x0E:
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
        case BHSM_RegionId_eRedacted_0x1E:  return BCMD_MemAuth_CpuType_eRESERVED7;
        default: {
            BERR_TRACE( BERR_INVALID_PARAMETER );
            return BCMD_MemAuth_CpuType_eMax;
        }
    }
}
