/******************************************************************************
 *  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#include "nexus_security_module.h"
#include "nexus_regver_rsa.h"
#include "priv/nexus_regver_rsa_priv.h"
#include "priv/nexus_security_priv.h"
#include "priv/nexus_regver_priv.h"
#include "priv/nexus_core.h"
#include "bhsm.h"
#include "bhsm_rv_region.h"
#include "bhsm_otp_msp.h"
#include "nexus_otp_msp_indexes.h"
#if NEXUS_HAS_SAGE
#include "bchp_bsp_glb_control.h"
#endif

#if NEXUS_SECURITY_FW_SIGN
#include "nexus_regver_signatures.inc"
#include "nexus_regver_key.inc"
#else
#include "nexus_regver_stub_signatures.c"
#include "nexus_regver_stub_key.c"
#endif

#define MIN(a,b) ((a<b)?(a):(b))

/* OTP enums */
#if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(5,1)
#define OTP_ENUM_RAVE      (142)
#define OTP_ENUM_RAAGA0    (NEXUS_OTPMSP_RAAGA_A_VERIFY_ENABLE)
#define OTP_ENUM_RAAGA1    (NEXUS_OTPMSP_RAAGA_B_VERIFY_ENABLE)
#define OTP_ENUM_VIDEO     (NEXUS_OTPMSP_VIDEO_VERIFY_ENABLE)
#define OTP_ENUM_VICE      (NEXUS_OTPMSP_VICE_VERIFY_ENABLE)
#define OTP_ENUM_SAGE_FSBL (164)
#else
#define OTP_ENUM_RAVE   (78)
#define OTP_ENUM_RAAGA0 (76)
#define OTP_ENUM_VIDEO  (77)
#define OTP_ENUM_RAAGA1 (193)
#define OTP_ENUM_VICE   (131)
#define OTP_ENUM_SID    (142)
#define OTP_ENUM_SAGE_FSBL (209)
#endif

/*
    Verification data and state for one region.
*/
typedef struct {
    NEXUS_SecurityRegverRegionID regionId;    /* The Id of this region. */
    bool verificationSupported;               /* Region verifcation is supported for this component. */
    bool verificationRequired;                /* OTP requires region to be verifed. */
    bool verified;                            /* Region has been verifed. */

    uint8_t* pCertificate;                    /* points to signature of region. Includes header at end. */

    NEXUS_RegVerRsaHandle rvRsaKeyHandle;       /* if false, use defaultRsaKeyHandle */

    bool enableInstructionChecker;            /* Required for SAGE/BHSM_SUPPORT_HDDTA */
    bool enableBackgroundChecker;             /* Required for SAGE */
    bool enforceAuthentication;               /* force verification */
    unsigned scmVersion;                      /* Required for BHSM_SUPPORT_HDDTA */

    char description[30];                     /* textual description of region. */

    BHSM_RvRegionHandle  rvRegionHandle;       /* HSM region verification */


}regionData_t;


/*  Region Verification module data.  */
typedef struct {

    regionData_t region[NEXUS_REGVER_MAX_REGIONS];
    bool moduleInitialised;              /* the Module has previously been initialised. */

    NEXUS_RegVerRsaHandle defaultRsaKeyHandle;

}regVerModuleData;


BDBG_MODULE(nexus_regver);

/* module data instance. */
static regVerModuleData gRegVerModuleData;

/* local functions */
static NEXUS_Error _LoadDefaultCertificates( void );
#define _InitialiseRegion(region,otpIndex,enf) _InitialiseRegion_dbg(region,otpIndex,enf,#region)
static void _InitialiseRegion_dbg( NEXUS_SecurityRegverRegionID regionId, unsigned otpIndex, bool enforceAuthentication, char * pRegionStr );
static NEXUS_Error _LoadDefaultRsaKey(void);
static NEXUS_Error _LoadCertificate( unsigned regionId,  const uint8_t *pSigData );
static regionData_t* _GetRegionData( NEXUS_SecurityRegverRegionID regionId );

void  NEXUS_Security_GetDefaultRegionVerificationModuleSettings(  NEXUS_SecurityRegionModuleSettings *pSettings )
{
    BDBG_ENTER( NEXUS_Security_GetDefaultRegionVerificationModuleSettings );
    NEXUS_ASSERT_MODULE();

    BKNI_Memset( pSettings, 0, sizeof(*pSettings) );

    BDBG_LEAVE( NEXUS_Security_GetDefaultRegionVerificationModuleSettings );
    return;
}


NEXUS_Error NEXUS_Security_RegionVerification_Init_priv( const NEXUS_SecurityRegionModuleSettings *pSettings )
{
    BDBG_ENTER( NEXUS_Security_RegionVerification_Init_priv );
    NEXUS_ASSERT_MODULE();

    if( gRegVerModuleData.moduleInitialised == false )
    {
        NEXUS_Error rc;

        BKNI_Memset( &gRegVerModuleData, 0, sizeof(gRegVerModuleData) );

        /* initialise module data. */
        _InitialiseRegion( NEXUS_SecurityRegverRegionID_eRave,          OTP_ENUM_RAVE,     pSettings->enforceAuthentication[NEXUS_SecurityFirmwareType_eTransport] );
        _InitialiseRegion( NEXUS_SecurityRegverRegionID_eRaaga0,        OTP_ENUM_RAAGA0,   pSettings->enforceAuthentication[NEXUS_SecurityFirmwareType_eAudioDecoder] );
        _InitialiseRegion( NEXUS_SecurityRegverRegionID_eRaaga1,        OTP_ENUM_RAAGA1,   pSettings->enforceAuthentication[NEXUS_SecurityFirmwareType_eAudioDecoder] );
        _InitialiseRegion( NEXUS_SecurityRegverRegionID_eVDEC0_IL2A,    OTP_ENUM_VIDEO,    pSettings->enforceAuthentication[NEXUS_SecurityFirmwareType_eVideoDecoder] );
        _InitialiseRegion( NEXUS_SecurityRegverRegionID_eVDEC0_ILA,     OTP_ENUM_VIDEO,    pSettings->enforceAuthentication[NEXUS_SecurityFirmwareType_eVideoDecoder] );
        _InitialiseRegion( NEXUS_SecurityRegverRegionID_eVDEC0_OLA,     OTP_ENUM_VIDEO,    pSettings->enforceAuthentication[NEXUS_SecurityFirmwareType_eVideoDecoder] );
        _InitialiseRegion( NEXUS_SecurityRegverRegionID_eVDEC1_ILA,     OTP_ENUM_VIDEO,    pSettings->enforceAuthentication[NEXUS_SecurityFirmwareType_eVideoDecoder] );
        _InitialiseRegion( NEXUS_SecurityRegverRegionID_eVDEC1_OLA,     OTP_ENUM_VIDEO,    pSettings->enforceAuthentication[NEXUS_SecurityFirmwareType_eVideoDecoder] );
        _InitialiseRegion( NEXUS_SecurityRegverRegionID_eVDEC2_ILA,     OTP_ENUM_VIDEO,    pSettings->enforceAuthentication[NEXUS_SecurityFirmwareType_eVideoDecoder] );
        _InitialiseRegion( NEXUS_SecurityRegverRegionID_eVDEC2_OLA,     OTP_ENUM_VIDEO,    pSettings->enforceAuthentication[NEXUS_SecurityFirmwareType_eVideoDecoder] );
        _InitialiseRegion( NEXUS_SecurityRegverRegionID_eVice0Pic,      OTP_ENUM_VICE,     pSettings->enforceAuthentication[NEXUS_SecurityFirmwareType_eVideoEncoder] );
        _InitialiseRegion( NEXUS_SecurityRegverRegionID_eVice0MacroBlk, OTP_ENUM_VICE,     pSettings->enforceAuthentication[NEXUS_SecurityFirmwareType_eVideoEncoder] );
        _InitialiseRegion( NEXUS_SecurityRegverRegionID_eVice1Pic,      OTP_ENUM_VICE,     pSettings->enforceAuthentication[NEXUS_SecurityFirmwareType_eVideoEncoder] );
        _InitialiseRegion( NEXUS_SecurityRegverRegionID_eVice1MacroBlk, OTP_ENUM_VICE,     pSettings->enforceAuthentication[NEXUS_SecurityFirmwareType_eVideoEncoder] );
       #if BHSM_ZEUS_VERSION < BHSM_ZEUS_VERSION_CALC(5,0)
        _InitialiseRegion( NEXUS_SecurityRegverRegionID_eSid0,          OTP_ENUM_SID,      pSettings->enforceAuthentication[NEXUS_SecurityFirmwareType_ePictureDecoder] );
       #endif
        _InitialiseRegion( NEXUS_SecurityRegverRegionID_eScpuFsbl,      OTP_ENUM_SAGE_FSBL,false );

        /* load static signatures. */
        rc = _LoadDefaultCertificates( );
        if( rc != NEXUS_SUCCESS ) { return BERR_TRACE( rc ); /* failed to initailse signatures */ }

        gRegVerModuleData.moduleInitialised = true;
    }

    BDBG_LEAVE( NEXUS_Security_RegionVerification_Init_priv );
    return NEXUS_SUCCESS;
}

void NEXUS_Security_RegionVerification_UnInit_priv( void )
{
    regionData_t *pRegionData;
    unsigned regionId = 0;

    BDBG_ENTER( NEXUS_Security_RegionVerification_UnInit_priv );
    NEXUS_ASSERT_MODULE();

    /* Disable region verification  */
    for( regionId = 0; regionId < NEXUS_REGVER_MAX_REGIONS; regionId++ )
    {
        pRegionData = _GetRegionData( regionId );
        if( pRegionData )
        {
            if( pRegionData->verified )
            {
                NEXUS_Security_RegionVerifyDisable_priv( regionId );
            }
                    /*   BDBG_LOG(("UNINIT regionId[%d]", regionId ));*/
            /* Free the signature memory */
            if( pRegionData->pCertificate )
            {
                NEXUS_Memory_Free( pRegionData->pCertificate );
                pRegionData->pCertificate = NULL;
            }
        }
    }

    if( gRegVerModuleData.defaultRsaKeyHandle ){
        NEXUS_RegVerRsa_Free( gRegVerModuleData.defaultRsaKeyHandle );
        gRegVerModuleData.defaultRsaKeyHandle = NULL;
    }

    gRegVerModuleData.moduleInitialised = false;

    BDBG_LEAVE( NEXUS_Security_RegionVerification_UnInit_priv );
    return;
}



void NEXUS_Security_RegionGetDefaultConfig_priv ( NEXUS_SecurityRegverRegionID regionId, /* defaults may be region specific. */
                                                  NEXUS_SecurityRegionConfiguration *pConfiguration )
{
    BSTD_UNUSED( regionId );
    BDBG_ENTER( NEXUS_Security_RegionGetDefaultConfig_priv );
    NEXUS_ASSERT_MODULE();

    if( pConfiguration == NULL ){ BERR_TRACE( NEXUS_INVALID_PARAMETER ); return; }

    BKNI_Memset( pConfiguration, 0, sizeof( *pConfiguration ) );

    BDBG_LEAVE( NEXUS_Security_RegionGetDefaultConfig_priv );
    return;
}

NEXUS_Error NEXUS_Security_RegionConfig_priv( NEXUS_SecurityRegverRegionID regionId, const NEXUS_SecurityRegionConfiguration *pConfiguration )
{
    NEXUS_MemoryAllocationSettings memSetting;
    regionData_t *pRegionData;
    NEXUS_Error rc = NEXUS_SUCCESS;

    BDBG_ENTER( NEXUS_Security_RegionConfig_priv );
    NEXUS_ASSERT_MODULE();

    if( pConfiguration == NULL ) { return BERR_TRACE(NEXUS_INVALID_PARAMETER); }
    if( pConfiguration->signature.size > NEXUS_REGION_VERIFY_SIGNATURE_PLUS_HEADER_SIZE ) { return BERR_TRACE(NEXUS_INVALID_PARAMETER); }
    if( regionId >= NEXUS_REGVER_MAX_REGIONS ) { return BERR_TRACE( NEXUS_INVALID_PARAMETER ); }

    pRegionData = &gRegVerModuleData.region[regionId];

    if( pConfiguration->forceVerification )
    {
        pRegionData->verificationRequired = true;
        pRegionData->verificationSupported = true;
    }

    if( pRegionData->verificationRequired == false ) {
        BDBG_MSG(("[%s] Region [0x%02X][%s] Verification not required.", BSTD_FUNCTION, regionId, pRegionData->description ));
        return NEXUS_SUCCESS;
    }

    if( pConfiguration->signature.size > 0 )
    {
        if( (pConfiguration->signature.size > NEXUS_REGION_VERIFY_SIGNATURE_PLUS_HEADER_SIZE) ||
            (pConfiguration->signature.size < NEXUS_REGION_VERIFY_SIGNATURE_SIZE) )
        {
            return BERR_TRACE( NEXUS_INVALID_PARAMETER );
        }

        if( pRegionData->pCertificate == NULL ) /* Allocate memory for signature and header. */
        {
            NEXUS_Memory_GetDefaultAllocationSettings(&memSetting);
            memSetting.alignment = 32;
            memSetting.heap = g_pCoreHandles->heap[g_pCoreHandles->defaultHeapIndex].nexus;
            rc = NEXUS_Memory_Allocate( NEXUS_REGION_VERIFY_SIGNATURE_PLUS_HEADER_SIZE, &memSetting, (void **)&pRegionData->pCertificate );
            if( rc != NEXUS_SUCCESS )
            {
                return BERR_TRACE( NEXUS_OUT_OF_DEVICE_MEMORY );
            }
        }

        BKNI_Memcpy( pRegionData->pCertificate, pConfiguration->signature.data, pConfiguration->signature.size );
        NEXUS_FlushCache( (const void*)pRegionData->pCertificate, pConfiguration->signature.size );
    }
    BDBG_LEAVE( NEXUS_Security_RegionConfig_priv );
    return NEXUS_SUCCESS;
}


/* Verify the specifed region. */
NEXUS_Error NEXUS_Security_RegionVerifyEnable_priv( NEXUS_SecurityRegverRegionID regionId, NEXUS_Addr regionAddress, unsigned regionSize )
{
    BHSM_Handle hHsm;
    BERR_Code rc = NEXUS_SUCCESS;
    regionData_t *pRegionData;
    BHSM_RvRegionAllocateSettings rvAllocSettings;
    BHSM_RvRegionSettings rvSettings;
    unsigned countDown = 0;
    BHSM_RvRegionStatus regionStatus;
    NEXUS_RegVerRsaInfo_priv rvRsaInfoPriv;
    NEXUS_RegVerRsaHandle rvRsaHandle;

    NEXUS_Security_GetHsm_priv( &hHsm );
    if( !hHsm ) { return BERR_TRACE( NEXUS_NOT_INITIALIZED ); }

    pRegionData = _GetRegionData( regionId );
    if( pRegionData == NULL ) {
        BDBG_ERR(("[%s] Region [0x%02X] not available.", BSTD_FUNCTION, regionId ));
        return BERR_TRACE( NEXUS_INVALID_PARAMETER );
    }

   #if NEXUS_REGION_VERIFICATION_DUMP_FIRMWARE || NEXUS_REGION_VERIFICATION_DUMP_FIRMWARE_RAW /*dump binaries */
    {
        NEXUS_SecurityDumpFirmwareData dumpData;

        BKNI_Memset( &dumpData, 0, sizeof(dumpData) );
        dumpData.regionId = regionId;
        dumpData.regionAddress = regionAddress;
        dumpData.regionSize = regionSize;
        dumpData.pDescription = pRegionData->description;
        NEXUS_Security_DumpFirmwareBinary_priv( &dumpData );
    }
   #endif

    if( pRegionData->verificationRequired == false ) {
        BDBG_MSG(("[%s] Region [0x%02X][%s] Verification not required.", BSTD_FUNCTION, regionId, pRegionData->description ));
        return NEXUS_SUCCESS;
    }

    BDBG_MSG(("REGION [0x%02X][%s] Verifying size[%d].", regionId, pRegionData->description, regionSize ));

    if( pRegionData->verified == true ) {
        BDBG_WRN(("[%s] Region [0x%02X][%s] Already verified.", BSTD_FUNCTION, regionId, pRegionData->description  ));
        return NEXUS_SUCCESS;
    }

    rc =  _LoadDefaultRsaKey( );
    if( rc != NEXUS_SUCCESS ) { return BERR_TRACE( rc ); }

    BKNI_Memset( &rvAllocSettings, 0, sizeof(rvAllocSettings) );
    rvAllocSettings.regionId = regionId;
    pRegionData->rvRegionHandle = BHSM_RvRegion_Allocate( hHsm, &rvAllocSettings );
    if( pRegionData->rvRegionHandle  == NULL ) { rc = BERR_TRACE( NEXUS_NOT_AVAILABLE ); goto error; }

    BKNI_Memset( &rvSettings, 0, sizeof(rvSettings) );

    rvSettings.range[0].address = regionAddress;
    rvSettings.range[0].size =  regionSize;

    rvSettings.signature.address = NEXUS_AddrToOffset( pRegionData->pCertificate );
    rvSettings.signature.size = 256;
    rvSettings.parameters.address = NEXUS_AddrToOffset( pRegionData->pCertificate ) + rvSettings.signature.size;
    rvRsaHandle =  pRegionData->rvRsaKeyHandle ? pRegionData->rvRsaKeyHandle
                                               : gRegVerModuleData.defaultRsaKeyHandle;

    rc = NEXUS_RegVerRsa_GetInfo_priv( rvRsaHandle, &rvRsaInfoPriv );
    if( rc != NEXUS_SUCCESS ) { rc = BERR_TRACE( NEXUS_INVALID_PARAMETER ); goto error; }

    rvSettings.rvRsaHandle            = rvRsaInfoPriv.rvRsaKeyHandle;
    rvSettings.intervalCheckBandwidth = 1;
    rvSettings.resetOnVerifyFailure   = false;
    rvSettings.backgroundCheck        = false;
    rvSettings.allowRegionDisable     = true;
    rvSettings.enforceAuth            = true;

    rc = BHSM_RvRegion_SetSettings( pRegionData->rvRegionHandle, &rvSettings );
    if( rc != NEXUS_SUCCESS ) {  BERR_TRACE( NEXUS_UNKNOWN ); goto error; }

    rc = BHSM_RvRegion_Enable( pRegionData->rvRegionHandle );
    if( rc != NEXUS_SUCCESS ) { rc = BERR_TRACE( NEXUS_UNKNOWN ); goto error; }

    countDown = 500;

    do{
        BKNI_Sleep (5);
        rc =  BHSM_RvRegion_GetStatus( pRegionData->rvRegionHandle, &regionStatus );
        if( rc != NEXUS_SUCCESS ) { rc = BERR_TRACE( NEXUS_UNKNOWN ); goto error; }

    } while( --countDown > 0 && !(regionStatus.status & BHSM_RV_REGION_STATUS_ENABLED) );

    if( countDown == 0 )  { rc = BERR_TRACE( NEXUS_TIMEOUT ); goto error; }

    pRegionData->verified = regionStatus.status & BHSM_RV_REGION_STATUS_FAST_CHECK_RESULT;

    BDBG_LEAVE( NEXUS_Security_RegionVerifyEnable_priv );
    return NEXUS_SUCCESS;

error:

    if( pRegionData->rvRegionHandle ) {
        BHSM_RvRegion_Free( pRegionData->rvRegionHandle );
        pRegionData->rvRegionHandle = NULL;
    }

    return rc;
}


/*
Summary
    Disable region verification. Loop until verifcation has been disabled.
*/
void NEXUS_Security_RegionVerifyDisable_priv( NEXUS_SecurityRegverRegionID regionId )
{
    BHSM_Handle hHsm;
    BERR_Code hsmRc;
    regionData_t *pRegionData;

    BDBG_ENTER( NEXUS_Security_RegionVerifyDisable_priv );
    NEXUS_ASSERT_MODULE();

    NEXUS_Security_GetHsm_priv( &hHsm );
    if( !hHsm ) { BERR_TRACE( NEXUS_NOT_INITIALIZED ); return; }

    pRegionData = _GetRegionData( regionId );
    if( pRegionData == NULL ) { BERR_TRACE( NEXUS_INVALID_PARAMETER ); return; }

    if( pRegionData->verificationRequired == false ) {
        BDBG_MSG(("[%s] Region [0x%02X][%s] Verification not OTP enalbed.", BSTD_FUNCTION, regionId, pRegionData->description ));
        return;
    }

    if( pRegionData->verified != true ) {
        BDBG_WRN(("[%s] Region [0x%02X][%s] Not verified.", BSTD_FUNCTION, regionId, pRegionData->description ));
        return;
    }

    hsmRc = BHSM_RvRegion_Disable( pRegionData->rvRegionHandle );
    if( hsmRc != BERR_SUCCESS ) { BERR_TRACE( NEXUS_INVALID_PARAMETER ); } /* continue. */

    BHSM_RvRegion_Free( pRegionData->rvRegionHandle );

    pRegionData->rvRegionHandle = NULL;

    pRegionData->verified = false;

    BDBG_LEAVE( NEXUS_Security_RegionVerifyDisable_priv );
    return;
}


bool  NEXUS_Security_RegionVerification_IsRequired_priv( NEXUS_SecurityRegverRegionID regionId )
{
#if NEXUS_REGION_VERIFICATION_DUMP_FIRMWARE || NEXUS_REGION_VERIFICATION_DUMP_FIRMWARE_RAW
    BSTD_UNUSED( regionId );
    return true;
#else
    regionData_t *pRegionData;

    BDBG_ENTER( NEXUS_Security_RegionVerification_IsRequired_priv );
    NEXUS_ASSERT_MODULE();

    pRegionData = _GetRegionData( regionId );
    if( pRegionData == NULL ) {
        (void)BERR_TRACE( NEXUS_INVALID_PARAMETER ); /* Currently unsupported Region ID */
        return true;
    }

    BDBG_LEAVE( NEXUS_Security_RegionVerification_IsRequired_priv );

    return pRegionData->verificationRequired;
#endif
}



NEXUS_Error NEXUS_Security_RegionQueryInformation_priv( NEXUS_SecurityRegionInfoQuery  *pRegionQuery )
{
    BSTD_UNUSED( pRegionQuery );
#if 0
    BHSM_Handle hHsm;
    BERR_Code rc;
    uint32_t index = 0;
    BHSM_VerifcationStatus_t status;

    BDBG_ENTER( NEXUS_Security_RegionQueryInformation_priv );
    NEXUS_ASSERT_MODULE();

    if( !pRegionQuery ) {
        return BERR_TRACE( NEXUS_INVALID_PARAMETER );
    }

    NEXUS_Security_GetHsm_priv (&hHsm);
    if( !hHsm ){ return BERR_TRACE( NEXUS_NOT_INITIALIZED ); }

    BKNI_Memset( &status, 0, sizeof(status) );

    if( ( rc = BHSM_RegionVerification_QueryStatus( hHsm,  &status ) ) != BERR_SUCCESS )
    {
        return BERR_TRACE( rc );
    }

    BDBG_MSG(("[%s] Query Complete", BSTD_FUNCTION ));

    BDBG_CASSERT( MAX_REGION_NUMBER == NEXUS_REGVER_MAX_REGIONS );

    for (index = 0; index < NEXUS_REGVER_MAX_REGIONS; index++)
    {
        pRegionQuery->regionStatus[index] = status.region[index];
    }

    BDBG_LEAVE( NEXUS_Security_RegionQueryInformation_priv );
#endif
    return NEXUS_SUCCESS;
}

/*
    Copy the last part of a enumerator from string ... i.e., "Description" from  "Enumerator_eDescription"
*/
static void cropDescriptionFromStr( char* pDest, unsigned destSize, char* pStr )
{
    char* pLocation = NULL;
    unsigned boundsCheck = 120; /*stop run away*/

    if(!pDest) return;

    BKNI_Memset( pDest, 0, destSize );

    if(!pStr) return;

    while( *pStr && boundsCheck-- )
    {
        if(*pStr == '_' && *(pStr+1) == 'e')
        {
            pLocation = pStr+2;
        }
        pStr++;
    }

    if((pLocation == NULL) || (boundsCheck == 0)) /*nothing found or runaway*/
    {
        BKNI_Memcpy( pDest, "unknown", 7 );
        return;
    }

    BKNI_Memcpy( pDest, pLocation, MIN( (unsigned)(destSize-1), (unsigned)(pStr-pLocation)) );

    return;
}

/* Return module data associated with region.  */
static regionData_t* _GetRegionData( NEXUS_SecurityRegverRegionID regionId )
{
    regionData_t* pRegionData = NULL;

    if( regionId >= NEXUS_REGVER_MAX_REGIONS ){ BERR_TRACE( NEXUS_UNKNOWN ); return NULL; }

    pRegionData = &gRegVerModuleData.region[regionId];

    if( pRegionData->verificationSupported == false ){ return NULL; }

    return pRegionData;
}


/*
Summary
    Load key2 for region verification
*/
static NEXUS_Error  _LoadDefaultRsaKey( void )
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_RegVerRsaAllocateSettings allocSettings;
    NEXUS_RegVerRsaSettings rsaSettings;

    if( gRegVerModuleData.defaultRsaKeyHandle ) { return NEXUS_SUCCESS; } /* already loaded. */

    NEXUS_RegVerRsa_GetDefaultAllocateSettings( &allocSettings );
    allocSettings.rsaKeyId = NEXUS_ANY_ID;

    gRegVerModuleData.defaultRsaKeyHandle = NEXUS_RegVerRsa_Allocate( &allocSettings );
    if( !gRegVerModuleData.defaultRsaKeyHandle )  { rc = BERR_TRACE( NEXUS_NOT_SUPPORTED ); goto error; }

    BKNI_Memset( &rsaSettings, 0, sizeof(rsaSettings) );
    BDBG_CASSERT( sizeof(rsaSettings.rsaKey) >= sizeof(gRegionVerificationKey2) );
    BKNI_Memcpy( rsaSettings.rsaKey, gRegionVerificationKey2, sizeof(rsaSettings.rsaKey) );
    rsaSettings.rootKey = NEXUS_RegVerRsaRootKey_e0Prime;

    rc = NEXUS_RegVerRsa_SetSettings( gRegVerModuleData.defaultRsaKeyHandle, &rsaSettings );
    if( rc != NEXUS_SUCCESS ) { BERR_TRACE( rc ); goto error; }

    return NEXUS_SUCCESS;

error:

    if( gRegVerModuleData.defaultRsaKeyHandle ){
        NEXUS_RegVerRsa_Free( gRegVerModuleData.defaultRsaKeyHandle );
        gRegVerModuleData.defaultRsaKeyHandle = NULL;
    }
    return NEXUS_SUCCESS;
}


/*
Summary
    _InitialiseRegion_dbg is the debug implementation of _InitialiseRegion. It configure the initial state of each region. I needs to be called during platform initialisation.
*/
static void _InitialiseRegion_dbg( NEXUS_SecurityRegverRegionID regionId,
                                     unsigned otpIndex,
                                     bool enforceAuthentication,
                                     char * pRegionStr )
{
    BERR_Code rc = BERR_SUCCESS;
    regionData_t *pRegionData;

    BDBG_ENTER(_InitialiseRegion );

    BDBG_ASSERT( regionId < NEXUS_REGVER_MAX_REGIONS );

    pRegionData = &gRegVerModuleData.region[regionId];
    pRegionData->regionId = regionId;
    pRegionData->verificationSupported = true;
    pRegionData->verificationRequired  = false; /* may change below. */

   #if NEXUS_REGION_VERIFICATION_OVERRIDE_OTP
    BDBG_WRN(("Overriding RV OTP Region[0x%02X][%s]", regionId, pRegionData->description ));
    pRegionData->verificationRequired = true;
   #endif

    if( enforceAuthentication )
    {
        pRegionData->verificationRequired = true;
        pRegionData->enforceAuthentication = true;
    }

    if( !pRegionData->verificationRequired )
    {
        BHSM_Handle hHsm;
        BHSM_OtpMspRead otpRead;

        NEXUS_Security_GetHsm_priv (&hHsm);
        if( !hHsm ){ BERR_TRACE( NEXUS_NOT_INITIALIZED ); return; }

        BKNI_Memset( &otpRead, 0, sizeof(otpRead) );
        otpRead.index = otpIndex;
        rc = BHSM_OtpMsp_Read( hHsm, &otpRead );
        if( rc != BERR_SUCCESS ){ BERR_TRACE( rc ); return; }

        if( otpRead.data & otpRead.valid ) {
            pRegionData->verificationRequired = true;
        }
    }

    cropDescriptionFromStr( pRegionData->description, sizeof(pRegionData->description), pRegionStr );

    BDBG_MSG(("Region Verification [0x%02X][%s] %s", regionId, pRegionData->description, pRegionData->verificationRequired?"REQUIRED":"NOT required" ));

    BDBG_LEAVE( _InitialiseRegion );
    return;
}



static NEXUS_Error _LoadDefaultCertificates( void )
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    BDBG_ENTER( _LoadDefaultCertificates );

    #ifdef SIGNATURE_REGION_0x00
    rc = _LoadCertificate( 0x00, signatureRegion_0x00 );
    if( rc != NEXUS_SUCCESS ) { return BERR_TRACE( rc ); }
    #endif

    #ifdef SIGNATURE_REGION_0x01
    rc = _LoadCertificate( 0x01, signatureRegion_0x01 );
    if( rc != NEXUS_SUCCESS ) { return BERR_TRACE( rc ); }
    #endif

    #ifdef SIGNATURE_REGION_0x02
    rc = _LoadCertificate( 0x02, signatureRegion_0x02 );
    if( rc != NEXUS_SUCCESS ) { return BERR_TRACE( rc ); }
    #endif

    #ifdef SIGNATURE_REGION_0x03
    rc = _LoadCertificate( 0x03, signatureRegion_0x03 );
    if( rc != NEXUS_SUCCESS ) { return BERR_TRACE( rc ); }
    #endif

    #ifdef SIGNATURE_REGION_0x04
    rc = _LoadCertificate( 0x04, signatureRegion_0x04 );
    if( rc != NEXUS_SUCCESS ) { return BERR_TRACE( rc ); }
    #endif

    #ifdef SIGNATURE_REGION_0x05
    rc = _LoadCertificate( 0x05, signatureRegion_0x05 );
    if( rc != NEXUS_SUCCESS ) { return BERR_TRACE( rc ); }
    #endif

    #ifdef SIGNATURE_REGION_0x06
    rc = _LoadCertificate( 0x06, signatureRegion_0x06 );
    if( rc != NEXUS_SUCCESS ) { return BERR_TRACE( rc ); }
    #endif

    #ifdef SIGNATURE_REGION_0x07
    rc = _LoadCertificate( 0x07, signatureRegion_0x07 );
    if( rc != NEXUS_SUCCESS ) { return BERR_TRACE( rc ); }
    #endif

    #ifdef SIGNATURE_REGION_0x08
    rc = _LoadCertificate( 0x08, signatureRegion_0x08 );
    if( rc != NEXUS_SUCCESS ) { return BERR_TRACE( rc ); }
    #endif

    #ifdef SIGNATURE_REGION_0x09
    rc = _LoadCertificate( 0x09, signatureRegion_0x09 );
    if( rc != NEXUS_SUCCESS ) { return BERR_TRACE( rc ); }
    #endif

    #ifdef SIGNATURE_REGION_0x0A
    rc = _LoadCertificate( 0x0A, signatureRegion_0x0A );
    if( rc != NEXUS_SUCCESS ) { return BERR_TRACE( rc ); }
    #endif

    #ifdef SIGNATURE_REGION_0x0B
    rc = _LoadCertificate( 0x0B, signatureRegion_0x0B );
    if( rc != NEXUS_SUCCESS ) { return BERR_TRACE( rc ); }
    #endif

    #ifdef SIGNATURE_REGION_0x0C
    rc = _LoadCertificate( 0x0C, signatureRegion_0x0C );
    if( rc != NEXUS_SUCCESS ) { return BERR_TRACE( rc ); }
    #endif

    #ifdef SIGNATURE_REGION_0x0D
    rc = _LoadCertificate( 0x0D, signatureRegion_0x0D );
    if( rc != NEXUS_SUCCESS ) { return BERR_TRACE( rc ); }
    #endif

    #ifdef SIGNATURE_REGION_0x0E
    rc = _LoadCertificate( 0x0E, signatureRegion_0x0E );
    if( rc != NEXUS_SUCCESS ) { return BERR_TRACE( rc ); }
    #endif

    #ifdef SIGNATURE_REGION_0x0F
    rc = _LoadCertificate( 0x0F, signatureRegion_0x0F );
    if( rc != NEXUS_SUCCESS ) { return BERR_TRACE( rc ); }
    #endif

    #ifdef SIGNATURE_REGION_0x10
    rc = _LoadCertificate( 0x10, signatureRegion_0x10 );
    if( rc != NEXUS_SUCCESS ) { return BERR_TRACE( rc ); }
    #endif

    #ifdef SIGNATURE_REGION_0x11
    rc = _LoadCertificate( 0x11, signatureRegion_0x11 );
    if( rc != NEXUS_SUCCESS ) { return BERR_TRACE( rc ); }
    #endif

    #ifdef SIGNATURE_REGION_0x12
    rc = _LoadCertificate( 0x12, signatureRegion_0x12 );
    if( rc != NEXUS_SUCCESS ) { return BERR_TRACE( rc ); }
    #endif

    #ifdef SIGNATURE_REGION_0x13
    rc = _LoadCertificate( 0x13, signatureRegion_0x13 );
    if( rc != NEXUS_SUCCESS ) { return BERR_TRACE( rc ); }
    #endif

    #ifdef SIGNATURE_REGION_0x14
    rc = _LoadCertificate( 0x14, signatureRegion_0x14 );
    if( rc != NEXUS_SUCCESS ) { return BERR_TRACE( rc ); }
    #endif

    #ifdef SIGNATURE_REGION_0x15
    rc = _LoadCertificate( 0x15, signatureRegion_0x15 );
    if( rc != NEXUS_SUCCESS ) { return BERR_TRACE( rc ); }
    #endif

    #ifdef SIGNATURE_REGION_0x16
    rc = _LoadCertificate( 0x16, signatureRegion_0x16 );
    if( rc != NEXUS_SUCCESS ) { return BERR_TRACE( rc ); }
    #endif

    #ifdef SIGNATURE_REGION_0x17
    rc = _LoadCertificate( 0x17, signatureRegion_0x17 );
    if( rc != NEXUS_SUCCESS ) { return BERR_TRACE( rc ); }
    #endif

    #ifdef SIGNATURE_REGION_0x18
    rc = _LoadCertificate( 0x18, signatureRegion_0x18 );
    if( rc != NEXUS_SUCCESS ) { return BERR_TRACE( rc ); }
    #endif

    #ifdef SIGNATURE_REGION_0x19
    rc = _LoadCertificate( 0x19, signatureRegion_0x19 );
    if( rc != NEXUS_SUCCESS ) { return BERR_TRACE( rc ); }
    #endif

    #ifdef SIGNATURE_REGION_0x1A
    rc = _LoadCertificate( 0x1A, signatureRegion_0x1A );
    if( rc != NEXUS_SUCCESS ) { return BERR_TRACE( rc ); }
    #endif

    #ifdef SIGNATURE_REGION_0x1B
    rc = _LoadCertificate( 0x1B, signatureRegion_0x1B );
    if( rc != NEXUS_SUCCESS ) { return BERR_TRACE( rc ); }
    #endif

    #ifdef SIGNATURE_REGION_0x1C
    rc = _LoadCertificate( 0x1C, signatureRegion_0x1C );
    if( rc != NEXUS_SUCCESS ) { return BERR_TRACE( rc ); }
    #endif

    #ifdef SIGNATURE_REGION_0x1D
    rc = _LoadCertificate( 0x1D, signatureRegion_0x1D );
    if( rc != NEXUS_SUCCESS ) { return BERR_TRACE( rc ); }
    #endif

    #ifdef SIGNATURE_REGION_0x1E
    rc = _LoadCertificate( 0x1E, signatureRegion_0x1E );
    if( rc != NEXUS_SUCCESS ) { return BERR_TRACE( rc ); }
    #endif

    #ifdef SIGNATURE_REGION_0x1F
    rc = _LoadCertificate( 0x1F, signatureRegion_0x1F );
    if( rc != NEXUS_SUCCESS ) { return BERR_TRACE( rc ); }
    #endif

    BDBG_LEAVE( _LoadDefaultCertificates );
    return NEXUS_SUCCESS;
}


/* load an individual predefined region signature. */
static NEXUS_Error _LoadCertificate( unsigned regionId,  const uint8_t *pSigData )
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_MemoryAllocationSettings memSetting;
    regionData_t *pRegionData = NULL;

    BDBG_ENTER( _LoadCertificate );

    BDBG_ASSERT( regionId < NEXUS_REGVER_MAX_REGIONS );

    pRegionData = _GetRegionData( regionId );
    if( pRegionData == NULL ) { return NEXUS_SUCCESS; } /* nothing to do .. region not enabled.*/


    NEXUS_Memory_GetDefaultAllocationSettings( &memSetting );
    memSetting.alignment = 32;
    memSetting.heap = g_pCoreHandles->heap[g_pCoreHandles->defaultHeapIndex].nexus;
    rc = NEXUS_Memory_Allocate( NEXUS_REGION_VERIFY_SIGNATURE_PLUS_HEADER_SIZE, &memSetting, (void**)&pRegionData->pCertificate );
    if( rc != NEXUS_SUCCESS ) { return BERR_TRACE( NEXUS_OUT_OF_DEVICE_MEMORY ); }

    BKNI_Memcpy( pRegionData->pCertificate, pSigData, NEXUS_REGION_VERIFY_SIGNATURE_PLUS_HEADER_SIZE );

    NEXUS_FlushCache( pRegionData->pCertificate, NEXUS_REGION_VERIFY_SIGNATURE_PLUS_HEADER_SIZE );

    BDBG_LEAVE( _LoadCertificate );
    return NEXUS_SUCCESS;
}
