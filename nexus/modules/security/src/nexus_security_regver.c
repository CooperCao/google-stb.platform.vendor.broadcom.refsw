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
#include "priv/nexus_security_priv.h"
#include "priv/nexus_security_regver_priv.h"
#include "priv/nexus_core.h"
#include "bhsm.h"
#include "bhsm_verify_reg.h"
#include "bhsm_misc.h"
#include "bhsm_otpmsp.h"
#include "bsp_s_otp_common.h"
#include "bhsm_bseck.h"
#if NEXUS_HAS_SAGE
#include "bchp_bsp_glb_control.h"
#endif

#if NEXUS_SECURITY_FW_DEV_SIGN
#include "nexus_security_regver_signatures.inc"
#include "nexus_security_regver_key.inc"
#else
#include "nexus_security_regver_signatures_stub.c"
#include "nexus_security_regver_key_stub.c"
#endif

#define MIN(a,b) ((a<b)?(a):(b))

/* OTP enums */
#define OTP_ENUM_RAVE   (78)
#define OTP_ENUM_RAAGA0 (76)
#define OTP_ENUM_VIDEO  (77)
#define OTP_ENUM_RAAGA1 (193)
#define OTP_ENUM_VICE   (131)
#define OTP_ENUM_SID    (142)
#define OTP_ENUM_SAGE_FSBL (209)

#define DEFAULT_RSA_KEY_INDEX                  (2)
#define INVALID_RSA_KEY_INDEX                  (0xFFFFFFFF)

/* Verification data and state for one region. */
typedef struct {
    bool verificationSupported;               /* Region verifcation is supported for this component. */
    NEXUS_SecurityRegverRegionID regionId;    /* The Id of this region. */
    bool verificationRequired;                /* OTP requires region to be verifed. */
    bool verified;                            /* Region has been verifed. */

    uint8_t* pSignature;                      /* points to signature of region. Includes header at end. */
    NEXUS_SecuritySignedAtrributes signedAttributes; /* paramters that the signature is signed against */

    unsigned rsaKeyIndex;                     /* The RSA key index */
    bool useManagedRsaKey;                    /* if true, use the RSA key managed by NEXUS. */
    bool enableInstructionChecker;            /* Required for SAGE/BHSM_SUPPORT_HDDTA */
    bool enableBackgroundChecker;             /* Required for SAGE */
    unsigned scmVersion;                      /* Required for BHSM_SUPPORT_HDDTA */

    uint8_t epochSelect;                      /* todo ... will be read from signature file */

    NEXUS_SecurityVirtualKeyladderID  keyLadderId;     /* Required for SCPU FSBL region */
    NEXUS_SecurityKeySource           keyLadderLayer;  /* Required for SCPU FSBL region*/

    char description[30];                     /* textual description of region. */
    bool enforceAuthentication;               /* force verification */

}regionData_t;

/* Region Verification module data. */
typedef struct {

    regionData_t region[NEXUS_REGVER_MAX_REGIONS];
    bool moduleInitialised;              /* the Module has previously been initialised. */
    bool defaultRsaKeyLoaded;

}regionVerificationModuleData_t;


static regionVerificationModuleData_t gRegVerModuleData; /* module data instance. */

BDBG_MODULE(nexus_security_verify_reg);

static NEXUS_Error verifyRegion( NEXUS_SecurityRegverRegionID regionId, NEXUS_Addr regionAddress, unsigned regionSize );
#define initialiseRegion(region,otpIndex,enf) initialiseRegion_dbg(region,otpIndex,enf,#region)
static void initialiseRegion_dbg( NEXUS_SecurityRegverRegionID regionId,
                                     BCMD_Otp_CmdMsp_e otpIndex,
                                     bool enforceAuthentication,
                                     char * pRegionStr );
static NEXUS_Error disableRegion( NEXUS_SecurityRegverRegionID regionId );

static void parseSignatureHeader( NEXUS_SecuritySignedAtrributes *pSigHeader, const uint8_t* pSignature );


static NEXUS_Error loadDefaultRegionVerificationKey(void);

static regionData_t* NEXUS_Security_GetRegionData_priv( NEXUS_SecurityRegverRegionID regionId );

static NEXUS_Error NEXUS_Security_CalculateCpuType_priv( BCMD_MemAuth_CpuType_e *pCpuType, NEXUS_SecurityRegverRegionID region );


/* load an individual predefined region signature. */
static NEXUS_Error loadSignature( unsigned regionId,  const uint8_t *pSigData )
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_MemoryAllocationSettings memSetting;
    regionData_t *pRegionData = NULL;
    void **ppSig;

    BDBG_ENTER( loadSignature );

    BDBG_ASSERT( regionId < NEXUS_REGVER_MAX_REGIONS );

    pRegionData = NEXUS_Security_GetRegionData_priv( regionId );

    if( pRegionData == NULL )
    {
        return NEXUS_SUCCESS; /* nothing to do .. region not enabled.*/
    }

    parseSignatureHeader( &pRegionData->signedAttributes, pSigData );

    ppSig = (void**)(&pRegionData->pSignature);

    NEXUS_Memory_GetDefaultAllocationSettings( &memSetting );
    memSetting.alignment = 32;
    memSetting.heap = g_pCoreHandles->heap[g_pCoreHandles->defaultHeapIndex].nexus;
    rc = NEXUS_Memory_Allocate( NEXUS_REGIONVERIFY_SIGNATURE_PLUS_HEADER_SIZE, &memSetting, ppSig );
    if( rc != NEXUS_SUCCESS )
    {
        return BERR_TRACE( NEXUS_OUT_OF_DEVICE_MEMORY );
    }

    BKNI_Memcpy( *ppSig, pSigData, NEXUS_REGIONVERIFY_SIGNATURE_PLUS_HEADER_SIZE );

    pRegionData->rsaKeyIndex = DEFAULT_RSA_KEY_INDEX; /* hardcoded signatures are signed with the default RSA KEY. */
    pRegionData->useManagedRsaKey = true;

    NEXUS_FlushCache( *ppSig, NEXUS_REGIONVERIFY_SIGNATURE_PLUS_HEADER_SIZE );

    BDBG_LEAVE( loadSignature );
    return NEXUS_SUCCESS;
}

static NEXUS_Error InitialiseSignatures( void )
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    BDBG_ENTER( InitialiseSignatures );

    #ifdef SIGNATURE_REGION_0x00
    rc = loadSignature( 0x00, signatureRegion_0x00 );
    if( rc != NEXUS_SUCCESS ) { return BERR_TRACE( rc ); }
    #endif

    #ifdef SIGNATURE_REGION_0x01
    rc = loadSignature( 0x01, signatureRegion_0x01 );
    if( rc != NEXUS_SUCCESS ) { return BERR_TRACE( rc ); }
    #endif

    #ifdef SIGNATURE_REGION_0x02
    rc = loadSignature( 0x02, signatureRegion_0x02 );
    if( rc != NEXUS_SUCCESS ) { return BERR_TRACE( rc ); }
    #endif

    #ifdef SIGNATURE_REGION_0x03
    rc = loadSignature( 0x03, signatureRegion_0x03 );
    if( rc != NEXUS_SUCCESS ) { return BERR_TRACE( rc ); }
    #endif

    #ifdef SIGNATURE_REGION_0x04
    rc = loadSignature( 0x04, signatureRegion_0x04 );
    if( rc != NEXUS_SUCCESS ) { return BERR_TRACE( rc ); }
    #endif

    #ifdef SIGNATURE_REGION_0x05
    rc = loadSignature( 0x05, signatureRegion_0x05 );
    if( rc != NEXUS_SUCCESS ) { return BERR_TRACE( rc ); }
    #endif

    #ifdef SIGNATURE_REGION_0x06
    rc = loadSignature( 0x06, signatureRegion_0x06 );
    if( rc != NEXUS_SUCCESS ) { return BERR_TRACE( rc ); }
    #endif

    #ifdef SIGNATURE_REGION_0x07
    rc = loadSignature( 0x07, signatureRegion_0x07 );
    if( rc != NEXUS_SUCCESS ) { return BERR_TRACE( rc ); }
    #endif

    #ifdef SIGNATURE_REGION_0x08
    rc = loadSignature( 0x08, signatureRegion_0x08 );
    if( rc != NEXUS_SUCCESS ) { return BERR_TRACE( rc ); }
    #endif

    #ifdef SIGNATURE_REGION_0x09
    rc = loadSignature( 0x09, signatureRegion_0x09 );
    if( rc != NEXUS_SUCCESS ) { return BERR_TRACE( rc ); }
    #endif

    #ifdef SIGNATURE_REGION_0x0A
    rc = loadSignature( 0x0A, signatureRegion_0x0A );
    if( rc != NEXUS_SUCCESS ) { return BERR_TRACE( rc ); }
    #endif

    #ifdef SIGNATURE_REGION_0x0B
    rc = loadSignature( 0x0B, signatureRegion_0x0B );
    if( rc != NEXUS_SUCCESS ) { return BERR_TRACE( rc ); }
    #endif

    #ifdef SIGNATURE_REGION_0x0C
    rc = loadSignature( 0x0C, signatureRegion_0x0C );
    if( rc != NEXUS_SUCCESS ) { return BERR_TRACE( rc ); }
    #endif

    #ifdef SIGNATURE_REGION_0x0D
    rc = loadSignature( 0x0D, signatureRegion_0x0D );
    if( rc != NEXUS_SUCCESS ) { return BERR_TRACE( rc ); }
    #endif

    #ifdef SIGNATURE_REGION_0x0E
    rc = loadSignature( 0x0E, signatureRegion_0x0E );
    if( rc != NEXUS_SUCCESS ) { return BERR_TRACE( rc ); }
    #endif

    #ifdef SIGNATURE_REGION_0x0F
    rc = loadSignature( 0x0F, signatureRegion_0x0F );
    if( rc != NEXUS_SUCCESS ) { return BERR_TRACE( rc ); }
    #endif

    #ifdef SIGNATURE_REGION_0x10
    rc = loadSignature( 0x10, signatureRegion_0x10 );
    if( rc != NEXUS_SUCCESS ) { return BERR_TRACE( rc ); }
    #endif

    #ifdef SIGNATURE_REGION_0x11
    rc = loadSignature( 0x11, signatureRegion_0x11 );
    if( rc != NEXUS_SUCCESS ) { return BERR_TRACE( rc ); }
    #endif

    #ifdef SIGNATURE_REGION_0x12
    rc = loadSignature( 0x12, signatureRegion_0x12 );
    if( rc != NEXUS_SUCCESS ) { return BERR_TRACE( rc ); }
    #endif

    #ifdef SIGNATURE_REGION_0x13
    rc = loadSignature( 0x13, signatureRegion_0x13 );
    if( rc != NEXUS_SUCCESS ) { return BERR_TRACE( rc ); }
    #endif

    #ifdef SIGNATURE_REGION_0x14
    rc = loadSignature( 0x14, signatureRegion_0x14 );
    if( rc != NEXUS_SUCCESS ) { return BERR_TRACE( rc ); }
    #endif

    #ifdef SIGNATURE_REGION_0x15
    rc = loadSignature( 0x15, signatureRegion_0x15 );
    if( rc != NEXUS_SUCCESS ) { return BERR_TRACE( rc ); }
    #endif

    #ifdef SIGNATURE_REGION_0x16
    rc = loadSignature( 0x16, signatureRegion_0x16 );
    if( rc != NEXUS_SUCCESS ) { return BERR_TRACE( rc ); }
    #endif

    #ifdef SIGNATURE_REGION_0x17
    rc = loadSignature( 0x17, signatureRegion_0x17 );
    if( rc != NEXUS_SUCCESS ) { return BERR_TRACE( rc ); }
    #endif

    #ifdef SIGNATURE_REGION_0x18
    rc = loadSignature( 0x18, signatureRegion_0x18 );
    if( rc != NEXUS_SUCCESS ) { return BERR_TRACE( rc ); }
    #endif

    #ifdef SIGNATURE_REGION_0x19
    rc = loadSignature( 0x19, signatureRegion_0x19 );
    if( rc != NEXUS_SUCCESS ) { return BERR_TRACE( rc ); }
    #endif

    #ifdef SIGNATURE_REGION_0x1A
    rc = loadSignature( 0x1A, signatureRegion_0x1A );
    if( rc != NEXUS_SUCCESS ) { return BERR_TRACE( rc ); }
    #endif

    #ifdef SIGNATURE_REGION_0x1B
    rc = loadSignature( 0x1B, signatureRegion_0x1B );
    if( rc != NEXUS_SUCCESS ) { return BERR_TRACE( rc ); }
    #endif

    #ifdef SIGNATURE_REGION_0x1C
    rc = loadSignature( 0x1C, signatureRegion_0x1C );
    if( rc != NEXUS_SUCCESS ) { return BERR_TRACE( rc ); }
    #endif

    #ifdef SIGNATURE_REGION_0x1D
    rc = loadSignature( 0x1D, signatureRegion_0x1D );
    if( rc != NEXUS_SUCCESS ) { return BERR_TRACE( rc ); }
    #endif

    #ifdef SIGNATURE_REGION_0x1E
    rc = loadSignature( 0x1E, signatureRegion_0x1E );
    if( rc != NEXUS_SUCCESS ) { return BERR_TRACE( rc ); }
    #endif

    #ifdef SIGNATURE_REGION_0x1F
    rc = loadSignature( 0x1F, signatureRegion_0x1F );
    if( rc != NEXUS_SUCCESS ) { return BERR_TRACE( rc ); }
    #endif

    BDBG_LEAVE( InitialiseSignatures );
    return NEXUS_SUCCESS;
}

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
    NEXUS_SecurityRegionInfoQuery  regionSatus;
    regionData_t *pRegionData;
    unsigned x;
    NEXUS_Error rc;
    bool sageRunning=false;

    BDBG_ENTER( NEXUS_Security_RegionVerification_Init_priv );
    NEXUS_ASSERT_MODULE();

#if NEXUS_HAS_SAGE
    if(!(BREG_Read32(g_pCoreHandles->reg, BCHP_BSP_GLB_CONTROL_SCPU_SW_INIT)
        & BCHP_BSP_GLB_CONTROL_SCPU_SW_INIT_SCPU_SW_INIT_MASK))
    {
        sageRunning=true;
    }
#endif

    if( gRegVerModuleData.moduleInitialised == false )
    {
        BKNI_Memset( &gRegVerModuleData, 0, sizeof(gRegVerModuleData) );

        /* initialise module data. */
        initialiseRegion( BHSM_VerificationRegionId_eRave,          OTP_ENUM_RAVE, pSettings->enforceAuthentication[NEXUS_SecurityFirmwareType_eTransport] );
        initialiseRegion( BHSM_VerificationRegionId_eRaaga0,        OTP_ENUM_RAAGA0, pSettings->enforceAuthentication[NEXUS_SecurityFirmwareType_eAudioDecoder] );
        #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(3,0)
        initialiseRegion( BHSM_VerificationRegionId_eRaaga1,        OTP_ENUM_RAAGA1, pSettings->enforceAuthentication[NEXUS_SecurityFirmwareType_eAudioDecoder] );
        #endif

        #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,2)
        initialiseRegion( BHSM_VerificationRegionId_eVdec0_Il2A,    OTP_ENUM_VIDEO, pSettings->enforceAuthentication[NEXUS_SecurityFirmwareType_eVideoDecoder] );
        initialiseRegion( BHSM_VerificationRegionId_eVdec0_Ila,     OTP_ENUM_VIDEO, pSettings->enforceAuthentication[NEXUS_SecurityFirmwareType_eVideoDecoder] );
        initialiseRegion( BHSM_VerificationRegionId_eVdec0_Ola,     OTP_ENUM_VIDEO, pSettings->enforceAuthentication[NEXUS_SecurityFirmwareType_eVideoDecoder] );
        initialiseRegion( BHSM_VerificationRegionId_eVdec1_Ils,     OTP_ENUM_VIDEO, pSettings->enforceAuthentication[NEXUS_SecurityFirmwareType_eVideoDecoder] );
        initialiseRegion( BHSM_VerificationRegionId_eVdec1_Ols,     OTP_ENUM_VIDEO, pSettings->enforceAuthentication[NEXUS_SecurityFirmwareType_eVideoDecoder] );
        initialiseRegion( BHSM_VerificationRegionId_eVdec2_Ila,     OTP_ENUM_VIDEO, pSettings->enforceAuthentication[NEXUS_SecurityFirmwareType_eVideoDecoder] );
        initialiseRegion( BHSM_VerificationRegionId_eVdec2_Ola,     OTP_ENUM_VIDEO, pSettings->enforceAuthentication[NEXUS_SecurityFirmwareType_eVideoDecoder] );
        #else
        initialiseRegion( BHSM_VerificationRegionId_eAvd0Inner,     OTP_ENUM_VIDEO, pSettings->enforceAuthentication[NEXUS_SecurityFirmwareType_eVideoDecoder] );
        initialiseRegion( BHSM_VerificationRegionId_eAvd0Outer,     OTP_ENUM_VIDEO, pSettings->enforceAuthentication[NEXUS_SecurityFirmwareType_eVideoDecoder] );
        initialiseRegion( BHSM_VerificationRegionId_eHvd0_Ila,      OTP_ENUM_VIDEO, pSettings->enforceAuthentication[NEXUS_SecurityFirmwareType_eVideoDecoder] );
        initialiseRegion( BHSM_VerificationRegionId_eHvd0_Ola,      OTP_ENUM_VIDEO, pSettings->enforceAuthentication[NEXUS_SecurityFirmwareType_eVideoDecoder] );
        initialiseRegion( BHSM_VerificationRegionId_eSvd0Bl,        OTP_ENUM_VIDEO, pSettings->enforceAuthentication[NEXUS_SecurityFirmwareType_eVideoDecoder] );
        #endif
        initialiseRegion( BHSM_VerificationRegionId_eVice0Pic,      OTP_ENUM_VICE, pSettings->enforceAuthentication[NEXUS_SecurityFirmwareType_eVideoEncoder] );
        initialiseRegion( BHSM_VerificationRegionId_eVice0MacroBlk, OTP_ENUM_VICE, pSettings->enforceAuthentication[NEXUS_SecurityFirmwareType_eVideoEncoder] );
        initialiseRegion( BHSM_VerificationRegionId_eVice1Pic,      OTP_ENUM_VICE, pSettings->enforceAuthentication[NEXUS_SecurityFirmwareType_eVideoEncoder] );
        initialiseRegion( BHSM_VerificationRegionId_eVice1MacroBlk, OTP_ENUM_VICE, pSettings->enforceAuthentication[NEXUS_SecurityFirmwareType_eVideoEncoder] );
        initialiseRegion( BHSM_VerificationRegionId_eSid0,          OTP_ENUM_SID,  pSettings->enforceAuthentication[NEXUS_SecurityFirmwareType_ePictureDecoder] );

        #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,0)
        initialiseRegion( BHSM_VerificationRegionId_eScpuFsbl,      OTP_ENUM_SAGE_FSBL, false );
        #endif

        /* load static signatures. */
        if( ( rc = InitialiseSignatures( ) ) != NEXUS_SUCCESS )
        {
            return BERR_TRACE( rc ); /*failed to initailse signatures*/
        }

        /* Cleanup after abnormal applicaiton termination. (including Crtl^C app exit) */
        NEXUS_Security_RegionQueryInformation_priv( &regionSatus );
        for( x = 0; x < NEXUS_REGVER_MAX_REGIONS; x++ )
        {
            pRegionData = NEXUS_Security_GetRegionData_priv( x );

            if( pRegionData )
            {
                BDBG_MSG(("Region Status [0x%02X] [%02X] [%s] %s %s %s %s %s %s %s %s %s %s", x, regionSatus.regionStatus[x]
                                                           , pRegionData->description
                                                           , regionSatus.regionStatus[x] & REGION_STATUS_DEFINED       ? "defined"   :""
                                                           , regionSatus.regionStatus[x] & REGION_STATUS_VERIFIED      ? "verified"  :""
                                                           , regionSatus.regionStatus[x] & REGION_STATUS_ENABLED       ? "enabled"   :""
                                                           , regionSatus.regionStatus[x] & REGION_STATUS_FASTCHKSTART  ? "FtCkStart" :""
                                                           , regionSatus.regionStatus[x] & REGION_STATUS_FASTCHKFINISH ? "FtCkFin"   :""
                                                           , regionSatus.regionStatus[x] & REGION_STATUS_FASTCHKPADDINGRESULT ? "FtCkPaddingRes":""
                                                           , regionSatus.regionStatus[x] & REGION_STATUS_FASTCHKRESULT ? "FtCkBk"    :""
                                                           , regionSatus.regionStatus[x] & REGION_STATUS_BKCHKSTART    ? "BkCkStart" :""
                                                           , regionSatus.regionStatus[x] & REGION_STATUS_BHCHKFINISH   ? "BkCkFin"   :""
                                                           , regionSatus.regionStatus[x] & REGION_STATUS_BKCHKRESULT   ? "BkResult"  :"" ));

                if( regionSatus.regionStatus[x] & REGION_STATUS_ENABLED )
                {
                    /* Special case for SAGE if it's already running */
                    if(( pRegionData->regionId != NEXUS_SecurityRegverRegionID_eScpuFsbl ) ||
                        ( !sageRunning ))
                    {
                        disableRegion( pRegionData->regionId );
                    }
                }
            }
        }

        gRegVerModuleData.moduleInitialised = true;
    }
    else
    {
        /* lite initialisation if already initialised. Will be executed on exit from S3*/
        for( x = 0; x < NEXUS_REGVER_MAX_REGIONS; x++ )
        {
            pRegionData = NEXUS_Security_GetRegionData_priv( x );

            if( pRegionData )
            {
                pRegionData->verified = false;
            }
        }
        gRegVerModuleData.defaultRsaKeyLoaded = false; /*default rsa key needs to be reloaded. */
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
        pRegionData = NEXUS_Security_GetRegionData_priv( regionId );
        if( pRegionData )
        {
            if( pRegionData->verified )
            {
                disableRegion( regionId );
            }

            /* Free the signature memory */
            if( pRegionData->pSignature != NULL )
            {
                NEXUS_Memory_Free( pRegionData->pSignature );
                pRegionData->pSignature = NULL;
            }
        }
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

    if( pConfiguration == NULL )
    {
       BERR_TRACE( NEXUS_INVALID_PARAMETER );
       return;
    }

    BKNI_Memset( pConfiguration, 0, sizeof( *pConfiguration ) );
    pConfiguration->rsaKeyIndex = DEFAULT_RSA_KEY_INDEX;
    pConfiguration->useManagedRsaKey = true;

    /* By default, VKL is not used. When it is used, client will specify and configure it. */
    pConfiguration->keyLadderId = NEXUS_SecurityVirtualKeyladderID_eVKLDummy;

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

    if( pConfiguration == NULL || pConfiguration->signature.size > NEXUS_REGIONVERIFY_SIGNATURE_PLUS_HEADER_SIZE) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    if( regionId >= NEXUS_REGVER_MAX_REGIONS )
    {
        return BERR_TRACE( NEXUS_INVALID_PARAMETER );
    }

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

    pRegionData->rsaKeyIndex              = pConfiguration->rsaKeyIndex;
    pRegionData->useManagedRsaKey         = pConfiguration->useManagedRsaKey;
    pRegionData->enableInstructionChecker = pConfiguration->enableInstructionChecker;
    pRegionData->enableBackgroundChecker  = pConfiguration->enableBackgroundChecker;
    pRegionData->scmVersion               = pConfiguration->scmVersion;
    pRegionData->keyLadderId              = pConfiguration->keyLadderId;
    pRegionData->keyLadderLayer           = pConfiguration->keyLadderLayer;

    if( pConfiguration->signedAttributesValid )
    {
        pRegionData->signedAttributes = pConfiguration->signedAttributes;
    }

    if( pConfiguration->signature.size > 0 )
    {
        if( (pConfiguration->signature.size > NEXUS_REGIONVERIFY_SIGNATURE_PLUS_HEADER_SIZE) ||
            (pConfiguration->signature.size < NEXUS_REGIONVERIFY_SIGNATURE_SIZE) )
        {
            return BERR_TRACE( NEXUS_INVALID_PARAMETER );
        }

        if( pRegionData->pSignature == NULL ) /* Allocate memory for signature and header. */
        {
            NEXUS_Memory_GetDefaultAllocationSettings(&memSetting);
            memSetting.alignment = 32;
            memSetting.heap = g_pCoreHandles->heap[g_pCoreHandles->defaultHeapIndex].nexus;
            rc = NEXUS_Memory_Allocate( NEXUS_REGIONVERIFY_SIGNATURE_PLUS_HEADER_SIZE, &memSetting, (void **)&pRegionData->pSignature );
            if( rc != NEXUS_SUCCESS )
            {
                return BERR_TRACE( NEXUS_OUT_OF_DEVICE_MEMORY );
            }
        }

        BKNI_Memcpy( pRegionData->pSignature, pConfiguration->signature.data, pConfiguration->signature.size );
        NEXUS_FlushCache( (const void*)pRegionData->pSignature, pConfiguration->signature.size );

        if( pConfiguration->signature.size == NEXUS_REGIONVERIFY_SIGNATURE_PLUS_HEADER_SIZE )
        {
            BDBG_MSG(("[%s] Load region parameters from signature REGION[%d]", BSTD_FUNCTION, regionId ));
            parseSignatureHeader( &pRegionData->signedAttributes, pConfiguration->signature.data );
        }
    }

    BDBG_LEAVE( NEXUS_Security_RegionConfig_priv );
    return NEXUS_SUCCESS;
}


/*
Summary
    Verify the specified region.
*/
NEXUS_Error NEXUS_Security_RegionVerifyEnable_priv( NEXUS_SecurityRegverRegionID regionId, NEXUS_Addr regionAddress, unsigned regionSize )
{
    BERR_Code rc = NEXUS_SUCCESS;
    BHSM_Handle hHsm;
    unsigned loopCount = 0;
    regionData_t *pRegionData;
    BHSM_VerifcationStatus_t verificationStatus;

    BDBG_ENTER( NEXUS_Security_RegionVerifyEnable_priv );
    NEXUS_ASSERT_MODULE();

    NEXUS_Security_GetHsm_priv( &hHsm );
    if( !hHsm ) { return BERR_TRACE( NEXUS_INVALID_PARAMETER ); }

    pRegionData = NEXUS_Security_GetRegionData_priv( regionId );
    if( pRegionData == NULL ) { return BERR_TRACE( NEXUS_INVALID_PARAMETER ); }

    #if NEXUS_REGION_VERIFICATION_DUMP_FIRMWARE || NEXUS_REGION_VERIFICATION_DUMP_FIRMWARE_RAW /*dump binaries */
    {
        NEXUS_SecurityDumpFirmwareData dumpData;

        BKNI_Memset( &dumpData, 0, sizeof(dumpData) );
        dumpData.regionId = regionId;
        dumpData.regionAddress = regionAddress;
        dumpData.regionSize = regionSize;
        dumpData.pDescription = pRegionData->description;
        NEXUS_Security_CalculateCpuType_priv( &dumpData.cpuType, regionId );
        dumpData.epochSelect = pRegionData->epochSelect;
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

    rc = verifyRegion( regionId, regionAddress, regionSize );
    if ( rc != NEXUS_SUCCESS )
    {
        if( rc == NEXUS_SECURITY_REGION_NOT_OTP_ENABLED )
        {
            BDBG_WRN(("Region [0x%02X][%s] not OTP enabled.", regionId, pRegionData->description ));
            return NEXUS_SUCCESS; /* region verification not required */
        }

        return BERR_TRACE( NEXUS_INVALID_PARAMETER ); /* Region verificaiton failed. */
    }

    if( BHSM_RegionVerification_Enable( hHsm ) != BERR_SUCCESS )
    {
        return BERR_TRACE( NEXUS_INVALID_PARAMETER );
    }

    loopCount = 500;
    do
    {
        BKNI_Memset( &verificationStatus, 0, sizeof(verificationStatus) );

        rc = BHSM_RegionVerification_QueryStatus( hHsm, &verificationStatus );
        if ( rc != NEXUS_SUCCESS )
        {
            return BERR_TRACE( NEXUS_UNKNOWN );
        }

   #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(3,0)
        if (verificationStatus.region[regionId] & REGION_STATUS_FASTCHKFINISH) {
            BDBG_MSG(("RegionVer fast check finished."));
            BKNI_Sleep (3);
            break;
        }
   #else
        if (regionId == NEXUS_SecurityRegverRegionID_eRave) {
            /* There is no background check for RAVE. */
            if (verificationStatus.region[regionId] & REGION_STATUS_VERIFIED) {
                BDBG_MSG(("RAVE RegionVer verified."));
                BKNI_Sleep (3);
                break;
            }
            /* For other regions, keep bg check on. */
        } else  if (verificationStatus.region[regionId] & REGION_STATUS_FASTCHKFINISH) {
            BDBG_MSG(("RegionVer fast check finished."));
            BKNI_Sleep (3);
            break;
        }
   #endif


        BKNI_Sleep (5);

    } while( --loopCount );


    if( loopCount == 0 )
    {
        return BERR_TRACE( NEXUS_TIMEOUT );  /* Timed out waiting for region verification to complete. */
    }

    pRegionData->verified = true;

    if (verificationStatus.region[regionId] & REGION_STATUS_FASTCHKRESULT) {
        pRegionData->verified = false;
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    if (verificationStatus.region[regionId] & REGION_STATUS_FASTCHKPADDINGRESULT) {
        pRegionData->verified = false;
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    if (verificationStatus.region[regionId] & REGION_STATUS_BKCHKRESULT) {
        pRegionData->verified = false;
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    BDBG_LEAVE( NEXUS_Security_RegionVerifyEnable_priv );
    return NEXUS_SUCCESS;
}


/*
Summary
    Disable region verification. Loop until verifcation has been disabled.
*/
void NEXUS_Security_RegionVerifyDisable_priv( NEXUS_SecurityRegverRegionID regionId )
{
    BHSM_Handle hHsm;
    regionData_t *pRegionData;

    BDBG_ENTER( NEXUS_Security_RegionVerifyDisable_priv );
    NEXUS_ASSERT_MODULE();

    NEXUS_Security_GetHsm_priv( &hHsm );
    if( !hHsm )
    {
        BERR_TRACE( NEXUS_INVALID_PARAMETER );
        return;
    }

    pRegionData = NEXUS_Security_GetRegionData_priv( regionId );
    if( pRegionData == NULL ) {
        BERR_TRACE( NEXUS_INVALID_PARAMETER ); /* Currently unsupported Region ID */
        return;
    }

    if( pRegionData->verificationRequired == false ) {
        BDBG_MSG(("[%s] Region [0x%02X][%s] Verification not OTP enalbed.", BSTD_FUNCTION, regionId, pRegionData->description ));
        return;
    }

    if( pRegionData->verified != true ) {
        BDBG_WRN(("[%s] Region [0x%02X][%s] Not verified.", BSTD_FUNCTION, regionId, pRegionData->description ));
        return;
    }

    if( disableRegion( regionId ) != NEXUS_SUCCESS )
    {
        BERR_TRACE( NEXUS_UNKNOWN );
    }

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

    pRegionData = NEXUS_Security_GetRegionData_priv( regionId );
    if( pRegionData == NULL ) {
        (void)BERR_TRACE( NEXUS_INVALID_PARAMETER ); /* Currently unsupported Region ID */
        return true;
    }

    BDBG_LEAVE( NEXUS_Security_RegionVerification_IsRequired_priv );

    return pRegionData->verificationRequired;
#endif
}


static NEXUS_Error verifyRegion( NEXUS_SecurityRegverRegionID regionId, NEXUS_Addr regionAddress, unsigned regionSize )
{
    BHSM_Handle hHsm;
    BHSM_RegionConfiguration_t  regionConfiguration;
    BHSM_VerifcationStatus_t regionStatus;
    BERR_Code rc = BERR_SUCCESS;
    regionData_t *pRegionData;
    NEXUS_Addr startAddress = 0;
    NEXUS_Addr endAddress = 0;

    BDBG_ENTER( verifyRegion );

    NEXUS_Security_GetHsm_priv (&hHsm);
    if ( !hHsm )
    {
        return BERR_TRACE( NEXUS_INVALID_PARAMETER );
    }

    pRegionData = NEXUS_Security_GetRegionData_priv( regionId );
    if( pRegionData == NULL ) {
        return BERR_TRACE( NEXUS_INVALID_PARAMETER ); /* Currently unsupported Region ID */
    }

    if( pRegionData->pSignature == NULL )
    {
        BDBG_ERR(("No signature for Region [0x%02X].", regionId ));
        return BERR_TRACE( NEXUS_INVALID_PARAMETER ); /* Signature not defined; neither statically, nor via NEXUS_Security_RegionConfig_priv */
    }

    BKNI_Memset( &regionStatus, 0, sizeof(regionStatus) );
    if( ( rc = BHSM_RegionVerification_QueryStatus( hHsm,  &regionStatus ) ) != BERR_SUCCESS )
    {
        return BERR_TRACE( MAKE_HSM_ERR( rc ) );
    }
    if( regionStatus.region[regionId] & REGION_STATUS_ENABLED ||
        regionStatus.region[regionId] & REGION_STATUS_DEFINED )
    {
        disableRegion( regionId );
    }

    if( pRegionData->rsaKeyIndex == DEFAULT_RSA_KEY_INDEX )
    {
        if( pRegionData->useManagedRsaKey )
        {
            if( gRegVerModuleData.defaultRsaKeyLoaded == false )
            {
                rc = loadDefaultRegionVerificationKey( );
                if( rc != NEXUS_SUCCESS )
                {
                    /* failed to load default key required to authenticate region verification signatures. */
                    return BERR_TRACE( NEXUS_SECURITY_FAILED_TO_LOAD_REGVER_KEY );
                }

                gRegVerModuleData.defaultRsaKeyLoaded = true;
            }
        }
        else
        {
            /* trigger a reload of the key next time (it's likely the client will have overwritten it. ) */
            gRegVerModuleData.defaultRsaKeyLoaded = false;
        }
    }

    BKNI_Memset( &regionConfiguration, 0, sizeof(regionConfiguration));

    if( regionId == NEXUS_SecurityRegverRegionID_eRave )
    {
        regionConfiguration.region.startAddress = 0;
        regionConfiguration.region.startAddressMsb = 0;
        regionConfiguration.region.endAddress = regionSize -1;
        regionConfiguration.region.endAddressMsb = 0;
    }
    else
    {
        startAddress = regionAddress;
        endAddress = regionAddress + regionSize - 1;
        regionConfiguration.region.startAddress       = (uint32_t)(startAddress & 0xFFFFFFFF);
        regionConfiguration.region.startAddressMsb    = (uint32_t)((startAddress>>32) & 0xFF);
        regionConfiguration.region.endAddress         = (uint32_t)(endAddress & 0xFFFFFFFF);
        regionConfiguration.region.endAddressMsb      = (uint32_t)((endAddress>>32) & 0xFF);
    }
    startAddress = NEXUS_AddrToOffset( pRegionData->pSignature );
    endAddress   = NEXUS_AddrToOffset((void*)((char*)pRegionData->pSignature + NEXUS_REGIONVERIFY_SIGNATURE_SIZE - 1));
    regionConfiguration.signature.startAddress    = (uint32_t)(startAddress & 0xFFFFFFFF);
    regionConfiguration.signature.startAddressMsb = (uint32_t)((startAddress>>32) & 0xFF);
    regionConfiguration.signature.endAddress      = (uint32_t)(endAddress & 0xFFFFFFFF);
    regionConfiguration.signature.endAddressMsb   = (uint32_t)((endAddress>>32) & 0xFF);
    regionConfiguration.ucIntervalCheckBw        = 0x10;
    regionConfiguration.SCBBurstSize             = BCMD_ScbBurstSize_e64;
    regionConfiguration.unRSAKeyID               = pRegionData->rsaKeyIndex;
    regionConfiguration.codeRelocatable          = BHSM_CodeLocationRule_eRelocatable;

    if( NEXUS_Security_CalculateCpuType_priv ( &(regionConfiguration.cpuType), regionId ) != NEXUS_SUCCESS )
    {
        return BERR_TRACE( NEXUS_INVALID_PARAMETER ); /* Unsupported region. */
    }

    regionConfiguration.unEpoch                  = pRegionData->signedAttributes.epoch;
   #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,2)
    regionConfiguration.svpFwReleaseVersion      = pRegionData->signedAttributes.svpFwReleaseVersion;
    regionConfiguration.ucEpochSel               = pRegionData->signedAttributes.epochSelect;
    regionConfiguration.ucSigVersion             = pRegionData->signedAttributes.signatureVersion;
    regionConfiguration.ucSigType                = pRegionData->signedAttributes.signatureType;
    regionConfiguration.enforceAuthentication    = pRegionData->enforceAuthentication;
   #endif
   #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(2,0)
    regionConfiguration.verifyFailAction         = BCMD_MemAuth_ResetOnVerifyFailure_eNoReset; /* Ignored for  most region IDs*/
    regionConfiguration.unEpochMask              = pRegionData->signedAttributes.epochMask;
    regionConfiguration.unMarketID               = pRegionData->signedAttributes.marketId;
    regionConfiguration.unMarketIDMask           = pRegionData->signedAttributes.marketIdMask;
   #endif
   #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(3,0)
    if ( regionConfiguration.cpuType == BCMD_MemAuth_CpuType_eScpu )  /* Support for SCPU */
    {
        regionConfiguration.keyLayer                 = NEXUS_Security_P_mapNexus2Hsm_KeySource( pRegionData->keyLadderLayer );
        regionConfiguration.vkl                      = NEXUS_Security_P_mapNexus2Hsm_VklId( pRegionData->keyLadderId );
        regionConfiguration.bgCheck                  = pRegionData->enableBackgroundChecker;
        regionConfiguration.instrChecker             = pRegionData->enableInstructionChecker;
        regionConfiguration.SCMVersion               = pRegionData->scmVersion;
    }
   #endif

    rc = BHSM_RegionVerification_Configure( hHsm, regionId, &regionConfiguration );
    if( rc == BHSM_STATUS_REGION_VERIFICATION_NOT_ENABLED )
    {
        BDBG_MSG(("[%s] Region [0x%02X] not OTP enabled.", BSTD_FUNCTION, regionId));
        return NEXUS_SECURITY_REGION_NOT_OTP_ENABLED;
    }
    if( (rc != BERR_SUCCESS) && (rc != BHSM_STATUS_REGION_ALREADY_CONFIGURED) )
    {
        return BERR_TRACE( NEXUS_NOT_SUPPORTED );
    }

    BDBG_LEAVE( verifyRegion );
    return NEXUS_SUCCESS;
}



NEXUS_Error disableRegion( NEXUS_SecurityRegverRegionID regionId )
{
    BHSM_Handle hHsm;
    BHSM_VerifcationStatus_t verificationStatus;
    bool regionDefined = true;
    BERR_Code rc = BERR_SUCCESS;
    regionData_t *pRegionData;
    unsigned loopCount;

    BDBG_ENTER( disableRegion );

    NEXUS_Security_GetHsm_priv( &hHsm );
    if ( hHsm == NULL )
    {
        return BERR_TRACE( NEXUS_INVALID_PARAMETER );
    }

    pRegionData = NEXUS_Security_GetRegionData_priv( regionId );
    if( pRegionData == NULL ) {
        return BERR_TRACE( NEXUS_INVALID_PARAMETER ); /* Currently unsupported Region ID */
    }

    if( pRegionData->verificationRequired == false ) {
        BDBG_MSG(("[%s] Region [0x%02X][%s] Verification not required.", BSTD_FUNCTION, regionId,pRegionData->description  ));
        return NEXUS_SUCCESS;
    }

    BDBG_MSG(("REGION [0x%02X][%s] disabling", regionId, pRegionData->description ));

    /* disable even if pRegionData->verified indicates that its not currently enabled. */

    rc = BHSM_RegionVerification_Disable( hHsm, regionId );
    if( rc == BHSM_STATUS_REGION_VERIFICATION_NOT_ENABLED )
    {
        BDBG_WRN(("[%s]Region [0x%02X][%s] not OTP enabled.", BSTD_FUNCTION, regionId, pRegionData->description ));
        return NEXUS_SECURITY_REGION_NOT_OTP_ENABLED;
    }
    if( rc != BERR_SUCCESS )
    {
        return BERR_TRACE( NEXUS_NOT_SUPPORTED );
    }

    BKNI_Memset( &verificationStatus, 0, sizeof(verificationStatus) );

    loopCount = 100;

    /* wait until the region becomes undefined. */
    do
    {
        if( ( rc = BHSM_RegionVerification_QueryStatus( hHsm, &verificationStatus ) ) != BERR_SUCCESS )
        {
            return BERR_TRACE( NEXUS_NOT_SUPPORTED );   /* error reading status of verification regions */
        }

        if( verificationStatus.region[regionId] & REGION_STATUS_DEFINED ||
            verificationStatus.region[regionId] & REGION_STATUS_ENABLED )
        {
            BKNI_Sleep(5);
        }
        else
        {
            regionDefined = false;  /* region has become undefined */
        }

    } while( regionDefined && --loopCount );

    pRegionData->verified = false;

    BDBG_LEAVE( disableRegion );
    return NEXUS_SUCCESS;
}



NEXUS_Error NEXUS_Security_RegionQueryInformation_priv( NEXUS_SecurityRegionInfoQuery  *pRegionQuery )
{
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
    if( !hHsm ){
        return BERR_TRACE( NEXUS_INVALID_PARAMETER );
    }

    BKNI_Memset( &status, 0, sizeof(status) );

    if( ( rc = BHSM_RegionVerification_QueryStatus( hHsm,  &status ) ) != BERR_SUCCESS )
    {
        return BERR_TRACE( MAKE_HSM_ERR( rc ) );
    }

    BDBG_MSG(("[%s] Query Complete", BSTD_FUNCTION ));

    BDBG_CASSERT( MAX_REGION_NUMBER == NEXUS_REGVER_MAX_REGIONS );

    for (index = 0; index < NEXUS_REGVER_MAX_REGIONS; index++)
    {
        pRegionQuery->regionStatus[index] = status.region[index];
    }

    BDBG_LEAVE( NEXUS_Security_RegionQueryInformation_priv );
    return NEXUS_SUCCESS;
}

/*
Summary
    Determine the CPU type from the region ID
*/
static NEXUS_Error NEXUS_Security_CalculateCpuType_priv( BCMD_MemAuth_CpuType_e *pCpuType, NEXUS_SecurityRegverRegionID region )
{
    BERR_Code rc = NEXUS_SUCCESS;

    BDBG_ENTER( calculateCpuType );

    switch( region )
    {
        case NEXUS_SecurityRegverRegionID_eRave:
        {
            *pCpuType = BCMD_MemAuth_CpuType_eRave;
            break;
        }
        case NEXUS_SecurityRegverRegionID_eVice0Pic:
        case NEXUS_SecurityRegverRegionID_eVice0MacroBlk:
        case NEXUS_SecurityRegverRegionID_eVice1Pic:
        case NEXUS_SecurityRegverRegionID_eVice1MacroBlk:
        {
            *pCpuType = BCMD_MemAuth_CpuType_eVice;
            break;
        }
        case NEXUS_SecurityRegverRegionID_eRaaga0:
        case NEXUS_SecurityRegverRegionID_eRaaga1:
        {
            *pCpuType = BCMD_MemAuth_CpuType_eRaaga;
            break;
        }
        case NEXUS_SecurityRegverRegionID_eSid0:
        {
            *pCpuType = BCMD_MemAuth_CpuType_eSid;
            break;
        }

        #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(3,0)
        case NEXUS_SecurityRegverRegionID_eScpuFsbl:
        case NEXUS_SecurityRegverRegionID_eScpuOsApp:
        case NEXUS_SecurityRegverRegionID_eScpuGeneric:
        case NEXUS_SecurityRegverRegionID_eScpuScm:
        {
            *pCpuType = BCMD_MemAuth_CpuType_eScpu;
            break;
        }
        #endif

        #if BHSM_ZEUS_VERSION <= BHSM_ZEUS_VERSION_CALC(4,1)
        case NEXUS_SecurityRegverRegionID_eAvd0Inner:
        case NEXUS_SecurityRegverRegionID_eAvd0Outer:
        {
            *pCpuType = BCMD_MemAuth_CpuType_eAvd;
            break;
        }
        #endif

        #if BHSM_ZEUS_VERSION <= BHSM_ZEUS_VERSION_CALC(3,0)
        case NEXUS_SecurityRegverRegionID_eSvd0Inner:
        case NEXUS_SecurityRegverRegionID_eSvd0Outer:
        case NEXUS_SecurityRegverRegionID_eSvd0Bl:
        {
            *pCpuType = BCMD_MemAuth_CpuType_eSvd;
            break;
        }
        #endif
        #if BHSM_ZEUS_VERSION == BHSM_ZEUS_VERSION_CALC(4,1) /*only supported on Zeus4.1*/
        case NEXUS_SecurityRegverRegionID_eHVD1_ILA:
        case NEXUS_SecurityRegverRegionID_eHVD1_OLA:
        case NEXUS_SecurityRegverRegionID_eHVD2_ILA:
        case NEXUS_SecurityRegverRegionID_eHVD2_OLA:
        {
            *pCpuType = BCMD_MemAuth_CpuType_eHvd;
            break;
        }
        #endif
        #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,2)
        case NEXUS_SecurityRegverRegionID_eVDEC0_IL2A:
        case NEXUS_SecurityRegverRegionID_eVDEC0_ILA:
        case NEXUS_SecurityRegverRegionID_eVDEC0_OLA:
        case NEXUS_SecurityRegverRegionID_eVDEC1_ILA:
        case NEXUS_SecurityRegverRegionID_eVDEC1_OLA:
        case NEXUS_SecurityRegverRegionID_eVDEC2_ILA:
        case NEXUS_SecurityRegverRegionID_eVDEC2_OLA:
        {
            *pCpuType = BCMD_MemAuth_CpuType_eVdec;
            break;
        }
        #endif
        case NEXUS_SecurityRegverRegionID_eHost00:
        case NEXUS_SecurityRegverRegionID_eHost01:
        case NEXUS_SecurityRegverRegionID_eHost02:
        case NEXUS_SecurityRegverRegionID_eHost03:
        case NEXUS_SecurityRegverRegionID_eHost04:
        case NEXUS_SecurityRegverRegionID_eHost05:
        case NEXUS_SecurityRegverRegionID_eHost06:
        case NEXUS_SecurityRegverRegionID_eHost07:
        {

        #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,0)
            *pCpuType = BCMD_MemAuth_CpuType_eHost;
        #else
            *pCpuType = BCMD_MemAuth_CpuType_eMips;
        #endif
            break;
        }
        default:
        {
            BDBG_ERR(("Region [0x%02X] not mapped to CPU type", region ));
            rc = BERR_TRACE( NEXUS_INVALID_PARAMETER ); /* Unsupported region. */
        }
    }

    BDBG_LEAVE( calculateCpuType );
    return rc;
}


/*
Summary
    Extract configuration parameters embedded in the augmented/extended signature.
*/
static void parseSignatureHeader( NEXUS_SecuritySignedAtrributes *pSigHeader, const uint8_t* pSignature )
{
    uint8_t *pRaw;

    BDBG_ASSERT( pSigHeader );
    BDBG_ASSERT( pSignature );

    BDBG_ENTER( parseSignatureHeader );

    BKNI_Memset( pSigHeader, 0, sizeof(*pSigHeader) );

    pRaw = (uint8_t*)(pSignature + NEXUS_REGIONVERIFY_SIGNATURE_SIZE); /* header is just after the signature */


    pSigHeader->cpuType  = pRaw[NEXUS_SECURITY_P_SIGNATURE_HEADER_OFFSET_CPU_TYPE];
    pSigHeader->marketId =     (pRaw[NEXUS_SECURITY_P_SIGNATURE_HEADER_OFFSET_MARKET_ID+0] << 24) +
                               (pRaw[NEXUS_SECURITY_P_SIGNATURE_HEADER_OFFSET_MARKET_ID+1] << 16) +
                               (pRaw[NEXUS_SECURITY_P_SIGNATURE_HEADER_OFFSET_MARKET_ID+2] << 8)  +
                               (pRaw[NEXUS_SECURITY_P_SIGNATURE_HEADER_OFFSET_MARKET_ID+3] << 0);

    pSigHeader->marketIdMask = (pRaw[NEXUS_SECURITY_P_SIGNATURE_HEADER_OFFSET_MARKET_ID_MASK+0] << 24) +
                               (pRaw[NEXUS_SECURITY_P_SIGNATURE_HEADER_OFFSET_MARKET_ID_MASK+1] << 16) +
                               (pRaw[NEXUS_SECURITY_P_SIGNATURE_HEADER_OFFSET_MARKET_ID_MASK+2] << 8)  +
                               (pRaw[NEXUS_SECURITY_P_SIGNATURE_HEADER_OFFSET_MARKET_ID_MASK+3] << 0);

    pSigHeader->epochMask =  pRaw[NEXUS_SECURITY_P_SIGNATURE_HEADER_OFFSET_EPOCH_MASK];
    pSigHeader->epoch =  pRaw[NEXUS_SECURITY_P_SIGNATURE_HEADER_OFFSET_EPOCH];
   #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,2)
    pSigHeader->svpFwReleaseVersion = pRaw[NEXUS_SECURITY_P_SIGNATURE_HEADER_OFFSET_SVP_REL_VER];
    pSigHeader->epochSelect = pRaw[NEXUS_SECURITY_P_SIGNATURE_HEADER_OFFSET_EPOCH_SELECT];
    pSigHeader->signatureVersion =  pRaw[NEXUS_SECURITY_P_SIGNATURE_HEADER_OFFSET_SIG_VERSION];
    pSigHeader->signatureType =  pRaw[NEXUS_SECURITY_P_SIGNATURE_HEADER_OFFSET_SIG_TYPE];
   #endif

    BDBG_LEAVE( parseSignatureHeader );
    return;
}

/*
Summary
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

/*
Summary
    Return module data associated with region.
*/
static regionData_t* NEXUS_Security_GetRegionData_priv( NEXUS_SecurityRegverRegionID regionId )
{
    regionData_t* pRegionData = NULL;

    if( regionId >= NEXUS_REGVER_MAX_REGIONS ){
        BERR_TRACE( NEXUS_UNKNOWN );
        return NULL;
    }

    pRegionData = &gRegVerModuleData.region[regionId];

    if( pRegionData->verificationSupported == false ){
        return NULL;
    }

    return pRegionData;
}


/*
Summary
    Load key2 for region verification
*/
static BERR_Code loadDefaultRegionVerificationKey( void )
{
    NEXUS_Error rc;
    BHSM_Handle                    hHsm;
    BHSM_VerifySecondTierKeyIO_t   sndTierKey;
    unsigned char *                pKey = NULL;
    NEXUS_MemoryAllocationSettings memSetting;

    BDBG_ENTER( loadDefaultRegionVerificationKey );

    NEXUS_Security_GetHsm_priv (&hHsm);
    if ( !hHsm )
    {
        return NEXUS_INVALID_PARAMETER;
    }

    NEXUS_Memory_GetDefaultAllocationSettings(&memSetting);
    memSetting.alignment = 32;
    memSetting.heap = g_pCoreHandles->heap[g_pCoreHandles->defaultHeapIndex].nexus;
    rc = NEXUS_Memory_Allocate( sizeof(gRegionVerificationKey2), &memSetting, (void**)&pKey );
    if ( rc != NEXUS_SUCCESS || pKey == NULL )
    {
        return BERR_TRACE( NEXUS_OUT_OF_DEVICE_MEMORY ); /* failed to allocate memory for region verification key.  */
    }
    BKNI_Memcpy( pKey, gRegionVerificationKey2, sizeof(gRegionVerificationKey2) );
    NEXUS_FlushCache( (const void*)pKey, sizeof(gRegionVerificationKey2) );

    sndTierKey.bChipResetOnFail      = false;
    sndTierKey.bMultiTierRootKeySrc  = false;
    sndTierKey.eFirstTierRootKeySrc  = BCMD_FirstTierKeyId_eKey0Prime;
    sndTierKey.eKeyIdentifier        = BCMD_SecondTierKeyId_eKey2;
    sndTierKey.eSecondTierRootKeySrc = BCMD_FirstTierKeyId_eKey0Prime;
    sndTierKey.keyAddr               = NEXUS_AddrToOffset( (void *)pKey );

    rc = BHSM_VerifySecondTierKey ( hHsm, &sndTierKey);

    NEXUS_Memory_Free( pKey );

    if( rc )
    {
        BDBG_WRN(( "Failed to load Region Verification Key, key2" ));
        return BERR_TRACE( NEXUS_INVALID_PARAMETER );
    }

    BDBG_MSG(( "Loaded region verification key2 RSA[%d]", (int)sndTierKey.eKeyIdentifier ));

    BDBG_LEAVE( loadDefaultRegionVerificationKey );

    return NEXUS_SUCCESS;
}


/*
Summary
    initialiseRegion_dbg is the debug implementation of initialiseRegion. It configure the initial state of each region. I needs to be called during platform initialisation.
*/
static void initialiseRegion_dbg( NEXUS_SecurityRegverRegionID regionId,
                                     BCMD_Otp_CmdMsp_e otpIndex,
                                     bool enforceAuthentication,
                                     char * pRegionStr )
{
    BHSM_ReadMspIO_t otp;
    BERR_Code rc = BERR_SUCCESS;
    BHSM_Handle hHsm;
    regionData_t *pRegionData;

    BDBG_ENTER(initialiseRegion );

    BDBG_ASSERT( regionId < NEXUS_REGVER_MAX_REGIONS );

    pRegionData = &gRegVerModuleData.region[regionId];
    pRegionData->regionId = regionId;
    pRegionData->verificationSupported = true;
    pRegionData->verificationRequired  = false; /* may change below. */
    pRegionData->rsaKeyIndex = INVALID_RSA_KEY_INDEX; /* initialise to invalid rsa key index. */

    NEXUS_Security_GetHsm_priv (&hHsm);
    if( !hHsm ) {
        BERR_TRACE( NEXUS_INVALID_PARAMETER );
        return;
    }

    otp.readMspEnum = otpIndex;
    rc = BHSM_ReadMSP( hHsm, &otp );
    if( rc != BERR_SUCCESS ) {
        BERR_TRACE( rc ); /* ERROR reading OTP. Verification of Region is assumed to be not required.  */
        return;
    }

    if( otp.aucMspData[3] ) /* OTP in BYTE 3, LSB */
    {
        pRegionData->verificationRequired = true;
    }
    #if NEXUS_REGION_VERIFICATION_OVERRIDE_OTP
    else {
        BDBG_WRN(("Overriding RV OTP Region[0x%02X][%s]", regionId, pRegionData->description ));
        pRegionData->verificationRequired = true;
    }
    #endif

    if( enforceAuthentication )
    {
        pRegionData->verificationRequired = true;
        pRegionData->enforceAuthentication = true;
    }

    cropDescriptionFromStr( pRegionData->description, sizeof(pRegionData->description), pRegionStr );

    BDBG_LEAVE( initialiseRegion );

    BDBG_MSG(("Region Verification [0x%02X][%s] %s", regionId, pRegionData->description, pRegionData->verificationRequired?"REQUIRED":"NOT required" ));
    return;
}
