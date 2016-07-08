/******************************************************************************
* Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
*
* This program is the proprietary software of Broadcom and/or its
* licensors, and may only be used, duplicated, modified or distributed pursuant
* to the terms and conditions of a separate, written license agreement executed
* between you and Broadcom (an "Authorized License").  Except as set forth in
* an Authorized License, Broadcom grants no license (express or implied), right
* to use, or waiver of any kind with respect to the Software, and Broadcom
* expressly reserves all rights in and to the Software and all intellectual
* property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
* HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
* NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
* Except as expressly set forth in the Authorized License,
*
* 1. This program, including its structure, sequence and organization,
*    constitutes the valuable trade secrets of Broadcom, and you shall use all
*    reasonable efforts to protect the confidentiality thereof, and to use
*    this information only in connection with your use of Broadcom integrated
*    circuit products.
*
* 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
*    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
*    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
*    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
*    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
*    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
*    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
*    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
*    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. , WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
******************************************************************************/

#include "bstd.h"
#include "bkni.h"
#include "bhsm.h"
#include "bhsm_datatypes.h"
#include "bhsm_private.h"
#include "bhsm_verify_reg.h"
#include "bhsm_bsp_msg.h"
#include "bhsm_otpmsp.h"
#include "bchp_common.h"
#include "bhsm_keyladder_enc.h"
#ifdef BHSM_SUPPORT_HDDTA
 #include "bsp_restricted_mem_auth_dta_generic.h"
#endif

BDBG_MODULE(BHSM);


#define BHSM_eSESSION_MEM_AUTH_DisableRegion    BCMD_cmdType_eSESSION_MEM_AUTH
#define BHSM_eSESSION_MEM_AUTH_ConfigureRegion  BCMD_cmdType_eSESSION_MEM_AUTH
#define BHSM_eSESSION_MEM_AUTH_Enable           BCMD_cmdType_eSESSION_MEM_AUTH
#define BHSM_eSESSION_MEM_AUTH_StatusRegion     BCMD_cmdType_eSESSION_MEM_AUTH
#define BHSM_eSESSION_MEM_AUTH_QueryStatus      BCMD_cmdType_eSESSION_MEM_AUTH
#define BHSM_eSESSION_MEM_AUTH_IsRegionVerified BCMD_cmdType_eSESSION_MEM_AUTH


BERR_Code BHSM_RegionVerification_Init( BHSM_Handle hHsm )
{
    BDBG_ENTER( BHSM_RegionVerification_Init );
    BSTD_UNUSED( hHsm );

    BDBG_LEAVE( BHSM_RegionVerification_Init );
    return BERR_SUCCESS;
}

/* Uninitialise Region verifcaiton on shutdown. */
BERR_Code BHSM_RegionVerification_UnInit( BHSM_Handle hHsm )
{
    BDBG_ENTER( BHSM_RegionVerification_UnInit );
    BSTD_UNUSED( hHsm );

    BDBG_LEAVE( BHSM_RegionVerification_UnInit );
    return BERR_SUCCESS;
}

BERR_Code BHSM_RegionVerification_Configure( BHSM_Handle hHsm, BHSM_RegionNumber region, BHSM_RegionConfiguration_t *pConf )
{
    BERR_Code rc = BERR_SUCCESS;
    BHSM_BspMsg_h hMsg = NULL;
    BHSM_BspMsgHeader_t header;
    uint8_t status = 0;

    BDBG_ENTER( BHSM_RegionVerification_Configure );

    if( (rc = BHSM_BspMsg_Create( hHsm, &hMsg ) ) != BERR_SUCCESS ) {
        BERR_TRACE( rc );
        goto BHSM_P_DONE_LABEL;
    }

    /* Configure BSP message header */
    BHSM_BspMsg_GetDefaultHeader( &header );
    BHSM_BspMsg_Header( hMsg, BHSM_eSESSION_MEM_AUTH_ConfigureRegion, &header );

    BHSM_BspMsg_Pack8( hMsg, BCMD_MemAuth_InCmdField_eRegionOp, BCMD_MemAuth_Operation_eEnableRegion );
    BHSM_BspMsg_Pack8( hMsg, BCMD_MemAuth_InCmdField_eRegionNum, region );

    /* region address */
   #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,0)
    BHSM_BspMsg_Pack32( hMsg, BCMD_MemAuth_InCmdField_eStartAddrMsb,    pConf->region.startAddressMsb  );
   #endif
    BHSM_BspMsg_Pack32( hMsg, BCMD_MemAuth_InCmdField_eStartAddr,       pConf->region.startAddress     );
   #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,0)
    BHSM_BspMsg_Pack32( hMsg, BCMD_MemAuth_InCmdField_eEndAddrMsb,      pConf->region.endAddressMsb    );
   #endif
    BHSM_BspMsg_Pack32( hMsg, BCMD_MemAuth_InCmdField_eEndAddr,         pConf->region.endAddress       );

    /* signature address */
   #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,0)
    BHSM_BspMsg_Pack32( hMsg, BCMD_MemAuth_InCmdField_eSigStartAddrMsb, pConf->signature.startAddressMsb  );
   #endif
    BHSM_BspMsg_Pack32( hMsg, BCMD_MemAuth_InCmdField_eSigStartAddr,    pConf->signature.startAddress);
   #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,0)
    BHSM_BspMsg_Pack32( hMsg, BCMD_MemAuth_InCmdField_eSigEndAddrMsb,   pConf->signature.endAddressMsb);
   #endif
    BHSM_BspMsg_Pack32( hMsg, BCMD_MemAuth_InCmdField_eSigEndAddr,      pConf->signature.endAddress);

    BHSM_BspMsg_Pack8 ( hMsg, BCMD_MemAuth_InCmdField_eIntervalCheckBw, pConf->ucIntervalCheckBw );
    BHSM_BspMsg_Pack8 ( hMsg, BCMD_MemAuth_InCmdField_eScbBurstSize,    pConf->SCBBurstSize );

   #if (FLASHMAP_VERSION >= FLASHMAP_VERSION_V3)
    BHSM_BspMsg_Pack8( hMsg, BCMD_MemAuth_InCmdField_eResetOnVerifyFailure, pConf->verifyFailAction );
   #endif

   #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(3,0)

    /* When VKL is actually allocated and configured, by SCPU. */
    if ((pConf->vkl != BCMD_VKL_KeyRam_eMax) && (pConf->cpuType == BCMD_MemAuth_CpuType_eScpu) )
    {
        BHSM_BspMsg_Pack8( hMsg, BCMD_MemAuth_InCmdField_eVKLId,             BHSM_RemapVklId(pConf->vkl ));
        BHSM_BspMsg_Pack8( hMsg, BCMD_MemAuth_InCmdField_eKeyLayer,          pConf->keyLayer );
    }

   #endif

    BHSM_BspMsg_Pack8( hMsg, BCMD_MemAuth_InCmdField_eRsaKeyId,          pConf->unRSAKeyID );
    /*BHSM_BspMsg_Pack8( hMsg, BCMD_MemAuth_InCmdField_eCodeRules,          );*/
    BHSM_BspMsg_Pack8( hMsg, BCMD_MemAuth_InCmdField_eNoRelocatableCode, pConf->codeRelocatable );
    BHSM_BspMsg_Pack8( hMsg, BCMD_MemAuth_InCmdField_eCpuType,           pConf->cpuType );

   #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,2)
    BHSM_BspMsg_Pack8( hMsg, BCMD_MemAuth_InCmdField_eSvpFwReleaseVersion, pConf->svpFwReleaseVersion );
   #endif

    BHSM_BspMsg_Pack8( hMsg, BCMD_MemAuth_InCmdField_eEpoch,            pConf->unEpoch );

   #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,2)
    BHSM_BspMsg_Pack8( hMsg, BCMD_MemAuth_InCmdField_eEnforceAuthentication, pConf->enforceAuthentication ? BCMD_MemAuth_EnforceAuthentication_Enforce
                                                                                                          : BCMD_MemAuth_EnforceAuthentication_NoEnforce );

    BHSM_BspMsg_Pack8( hMsg, BCMD_MemAuth_InCmdField_eAllowRegionDisable, pConf->disallowDisable ? BCMD_MemAuth_RegionDisable_Disallow
                                                                                                 : BCMD_MemAuth_RegionDisable_Allow );
   #endif

#if (FLASHMAP_VERSION >= FLASHMAP_VERSION_V3)
    BHSM_BspMsg_Pack32( hMsg, BCMD_MemAuth_InCmdField_eMarketID,        pConf->unMarketID );
    BHSM_BspMsg_Pack32( hMsg, BCMD_MemAuth_InCmdField_eMarketIDMask,    pConf->unMarketIDMask );
    BHSM_BspMsg_Pack8( hMsg, BCMD_MemAuth_InCmdField_eEpochMask,        pConf->unEpochMask);

   #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(3,0)
    BHSM_BspMsg_Pack8( hMsg, BCMD_MemAuth_InCmdField_eBgCheck, pConf->bgCheck ? BCMD_MemAuth_BgCheck_eEnable
                                                                              : BCMD_MemAuth_BgCheck_eDisable );
    BHSM_BspMsg_Pack8( hMsg, BCMD_MemAuth_InCmdField_eInstrCheck, pConf->instrChecker ? BCMD_MemAuth_InstrCheck_eEnable
                                                                                      : BCMD_MemAuth_InstrCheck_eDisable );
   #endif

   #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,2)
    BHSM_BspMsg_Pack8( hMsg, BCMD_MemAuth_InCmdField_eEpochSel,         pConf->ucEpochSel );
    BHSM_BspMsg_Pack8( hMsg, BCMD_MemAuth_InCmdField_eSigVersion,       pConf->ucSigVersion );
    BHSM_BspMsg_Pack8( hMsg, BCMD_MemAuth_InCmdField_eSigType,          pConf->ucSigType  );
   #endif

   #ifdef BHSM_SUPPORT_HDDTA
    BHSM_BspMsg_Pack8( hMsg, BCMD_MemAuth_InCmdField_eSCMVersion,       pConf->SCMVersion );
   #endif

   #if defined(AVS)
    BHSM_BspMsg_Pack32( hMsg, BCMD_MemAuth_InCmdField_eAvsDMEMStartAddr,pConf->AVSDMEMStartAddress  );
    BHSM_BspMsg_Pack32( hMsg, BCMD_MemAuth_InCmdField_eAvsDMEMEndAddr,  pConf->AVSDMEMEndAddress );
   #endif
#endif /* FLASHMAP_VERSION_V3 */

    if( ( rc = BHSM_BspMsg_SubmitCommand( hMsg ) ) != BERR_SUCCESS ) {
        (void)BERR_TRACE(rc);
        goto BHSM_P_DONE_LABEL;
    }

    BHSM_BspMsg_Get8( hMsg, BCMD_MemAuth_OutCmdField_eStatus, &status );
    switch( status )
    {
        case 0:
        {
            BDBG_MSG(("%s: Region [0x%02X] Configured.", __FUNCTION__, region ));
            break; /* success */
        }
        case BHSM_RegionVerificationStatus_eAlreadyConfigured:
        {
            BDBG_WRN(("%s: Region [0x%02X] Already Configured.", __FUNCTION__, region ));
            break; /* success */
        }
       case BHSM_RegionVerificationStatus_eNotOtpEnabled:
        {
            rc = BHSM_STATUS_REGION_VERIFICATION_NOT_ENABLED;
            BDBG_WRN(("%s: Region [0x%02X] Verification not enalbed in OTP", __FUNCTION__, region ));
            break;
        }
        default:
        {
            rc = BHSM_STATUS_BSP_ERROR;
            BDBG_WRN(("%s: ERROR[0x%0X] region[0x%0X]", __FUNCTION__, status, region ));
            break;
        }
    }

BHSM_P_DONE_LABEL:
    if( hMsg != NULL ) {
        (void)BHSM_BspMsg_Destroy( hMsg );
    }

    BDBG_LEAVE( BHSM_RegionVerification_Configure );
    return rc;
}


/* Enable verification on currenlty configured regions. */
BERR_Code BHSM_RegionVerification_Enable( BHSM_Handle hHsm )
{
    BERR_Code rc = BERR_SUCCESS;
    BHSM_BspMsg_h hMsg = NULL;
    BHSM_BspMsgHeader_t header;
    uint8_t status = 0;

    BDBG_ENTER( BHSM_RegionVerification_Enable );

    if( (rc = BHSM_BspMsg_Create( hHsm, &hMsg ) ) != BERR_SUCCESS ) {
        BERR_TRACE( rc );
        goto BHSM_P_DONE_LABEL;
    }

    BHSM_BspMsg_GetDefaultHeader( &header );
    BHSM_BspMsg_Header( hMsg, BHSM_eSESSION_MEM_AUTH_Enable, &header );

    BHSM_BspMsg_Pack8( hMsg, BCMD_MemAuth_InCmdField_eRegionOp, BCMD_MemAuth_Operation_eDefineRegion );

    if( ( rc = BHSM_BspMsg_SubmitCommand( hMsg ) ) != BERR_SUCCESS ) {
        (void)BERR_TRACE(rc);
        goto BHSM_P_DONE_LABEL;
    }

    BHSM_BspMsg_Get8( hMsg, BCMD_MemAuth_OutCmdField_eStatus, &status );
    switch( status )
    {
        case 0:
        {
            BDBG_MSG(("%s: Enable Verification.", __FUNCTION__ ));
            break;
        }
        default:
        {
            BDBG_WRN(("%s: ERROR[0x%0X]", __FUNCTION__, status ));
            break;
        }
    }

BHSM_P_DONE_LABEL:

    if( hMsg != NULL ) {
        (void)BHSM_BspMsg_Destroy( hMsg );
    }

    BDBG_LEAVE( BHSM_RegionVerification_Enable );
    return rc;
}


/* Disable verification of a specified region  */
BERR_Code BHSM_RegionVerification_Disable( BHSM_Handle hHsm, BHSM_RegionNumber region )
{
    BERR_Code rc = BERR_SUCCESS;
    BHSM_BspMsg_h hMsg = NULL;
    BHSM_BspMsgHeader_t header;
    uint8_t status = 0;

    BDBG_ENTER( BHSM_RegionVerification_Disable );

    if( (rc = BHSM_BspMsg_Create( hHsm, &hMsg ) ) != BERR_SUCCESS ) {
        BERR_TRACE( rc );
        goto BHSM_P_DONE_LABEL;
    }

    BHSM_BspMsg_GetDefaultHeader( &header );
    BHSM_BspMsg_Header( hMsg, BHSM_eSESSION_MEM_AUTH_DisableRegion, &header );

    BHSM_BspMsg_Pack8( hMsg, BCMD_MemAuth_InCmdField_eRegionOp, BCMD_MemAuth_Operation_eDisableRegion );
    BHSM_BspMsg_Pack8( hMsg, BCMD_MemAuth_InCmdField_eRegionNum, region );

    if( ( rc = BHSM_BspMsg_SubmitCommand( hMsg ) ) != BERR_SUCCESS ) {
        (void)BERR_TRACE(rc);
        goto BHSM_P_DONE_LABEL;
    }

    BHSM_BspMsg_Get8( hMsg, BCMD_MemAuth_OutCmdField_eStatus, &status );
    switch( status )
    {
        case 0:
        case BHSM_RegionVerificationStatus_eAlreadyDisabled:
        case BHSM_RegionVerificationStatus_eNotDefined:
        {
            BDBG_MSG(("%s: Region[0x%02X]  Disabled[0x%02X]", __FUNCTION__, region, status ));
            break; /* success */
        }
        case BHSM_RegionVerificationStatus_eNotOtpEnabled:
        {
            rc = BHSM_STATUS_REGION_VERIFICATION_NOT_ENABLED;
            BDBG_WRN(("%s: Region [0x%02X] Verification not enalbed in OTP", __FUNCTION__, region ));
            break;
        }
        default:
        {
            rc = BHSM_STATUS_BSP_ERROR;
            BDBG_WRN(("%s: error[0x%0X] region[0x%02X]", __FUNCTION__, status, region ));
            break;
        }
    }

BHSM_P_DONE_LABEL:

    if( hMsg != NULL ) {
        (void)BHSM_BspMsg_Destroy( hMsg );
    }

    BDBG_LEAVE( BHSM_RegionVerification_Disable );
    return rc;
}

/* Returns the verifcation status of a specifed region. */
BERR_Code BHSM_RegionVerification_Status( BHSM_Handle hHsm, BHSM_RegionNumber region, BHSM_RegionStatus_t *pStatus )
{
    BERR_Code rc = BERR_SUCCESS;
    BHSM_BspMsg_h hMsg = NULL;
    BHSM_BspMsgHeader_t header;
    uint8_t status = 0;
    unsigned i = 0;
    unsigned offset_addr = 0;
    unsigned offset_addrMsb = 0;
    BHSM_RegionNumber reportedRegion;

    BDBG_ENTER( BHSM_RegionVerification_Status );

    if( (rc = BHSM_BspMsg_Create( hHsm, &hMsg ) ) != BERR_SUCCESS ) {
        BERR_TRACE( rc );
        goto BHSM_P_DONE_LABEL;
    }

    BHSM_BspMsg_GetDefaultHeader( &header );
    BHSM_BspMsg_Header( hMsg, BHSM_eSESSION_MEM_AUTH_StatusRegion, &header );

    BHSM_BspMsg_Pack8( hMsg, BCMD_MemAuth_InCmdField_eRegionOp, BCMD_MemAuth_Operation_eRegionVerified );
    BHSM_BspMsg_Pack8( hMsg, BCMD_MemAuth_InCmdField_eRegionNum, region );

    if( ( rc = BHSM_BspMsg_SubmitCommand( hMsg ) ) != BERR_SUCCESS ) {
        (void)BERR_TRACE(rc);
        goto BHSM_P_DONE_LABEL;
    }

    BHSM_BspMsg_Get8( hMsg, BCMD_MemAuth_OutCmdField_eStatus, &status );
    switch( status )
    {
        case 0:
        {
            BDBG_MSG(("%s: Region [0x%02X] Status.", __FUNCTION__, region ));
            break;
        }
        case BHSM_RegionVerificationStatus_eInProgress:
        {
            BDBG_MSG(("%s: Region [0x%02X] Verification In Progress.", __FUNCTION__, region ));
            break;
        }
        case BHSM_RegionVerificationStatus_eFailed:
        {
            rc = BHSM_STATUS_BSP_ERROR;
            BDBG_WRN(("%s: Region [0x%02X] Check failed.", __FUNCTION__, region ));
            break;
        }
        case BHSM_RegionVerificationStatus_eNotOtpEnabled:
        {
            rc = BHSM_STATUS_REGION_VERIFICATION_NOT_ENABLED;
            BDBG_WRN(("%s: Region [0x%02X] Verification not OTP enalbed.", __FUNCTION__, region ));
            break;
        }
        case BHSM_RegionVerificationStatus_eNotDefined:
        {
            rc = BHSM_STATUS_REGION_VERIFICATION_NOT_DEFINED;
            BDBG_MSG(("%s: Region [0x%02X] not defined.", __FUNCTION__, region ));
            break;
        }
        default:
        {
            rc = BHSM_STATUS_BSP_ERROR;
            BDBG_WRN(("%s: ERROR[0x%0X] region[0x%0X]", __FUNCTION__, status, region ));
            break;
        }
    }

    pStatus->state = (BHSM_RegionsVerificationState_e)status;

    BHSM_BspMsg_Get8( hMsg, BCMD_MemAuth_OutCmdField_eRegionNum, (uint8_t*)&reportedRegion );

    /* read the region addresses.  */
    i = 0;
    offset_addrMsb = BCMD_MemAuth_OutCmdField_eRegion0Status;
    offset_addr    = ((offset_addrMsb/4)*4);

    if( ( status == BHSM_RegionVerificationStatus_eVerified )    ||
        ( status == BHSM_RegionVerificationStatus_eInProgress ) ||
        ( status == BHSM_RegionVerificationStatus_ePending )    ||
        ( status == BHSM_RegionVerificationStatus_eFailed ) )
    {
        #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,0)
        BHSM_BspMsg_Get8 ( hMsg, offset_addrMsb + (i*4),  (uint8_t*)&pStatus->region.startAddressMsb );     i++;
        #endif
        BHSM_BspMsg_Get32( hMsg, offset_addr    + (i*4),            &pStatus->region.startAddress );        i++;
        #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,0)
        BHSM_BspMsg_Get8 ( hMsg, offset_addrMsb + (i*4),  (uint8_t*)&pStatus->region.endAddressMsb );       i++;
        #endif
        BHSM_BspMsg_Get32( hMsg, offset_addr    + (i*4),            &pStatus->region.endAddress );          i++;
        #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,0)
        BHSM_BspMsg_Get8 ( hMsg, offset_addrMsb + (i*4),  (uint8_t*)&pStatus->signature.startAddressMsb );  i++;
        #endif
        BHSM_BspMsg_Get32( hMsg, offset_addr    + (i*4),            &pStatus->signature.startAddress );     i++;
        #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,0)
        BHSM_BspMsg_Get8 ( hMsg, offset_addrMsb + (i*4),  (uint8_t*)&pStatus->signature.endAddressMsb );    i++;
        #endif
        BHSM_BspMsg_Get32( hMsg, offset_addr    + (i*4),            &pStatus->signature.endAddress );       i++;

        if( region == 0 )
        {
            #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,0)
            BHSM_BspMsg_Get8 ( hMsg, offset_addrMsb +(i*4),  (uint8_t*)&pStatus->region01.startAddressMsb );   i++;
            #endif
            BHSM_BspMsg_Get32( hMsg, offset_addr    +(i*4),            &pStatus->region01.startAddress );      i++;
            #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,0)
            BHSM_BspMsg_Get8 ( hMsg, offset_addrMsb +(i*4),  (uint8_t*)&pStatus->region01.endAddressMsb );     i++;
            #endif
            BHSM_BspMsg_Get32( hMsg, offset_addr    +(i*4),            &pStatus->region01.endAddress );        i++;
            #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,0)
            BHSM_BspMsg_Get8 ( hMsg, offset_addrMsb +(i*4),  (uint8_t*)&pStatus->signature01.startAddressMsb );i++;
            #endif
            BHSM_BspMsg_Get32( hMsg, offset_addr    +(i*4),            &pStatus->signature01.startAddress );   i++;
            #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,0)
            BHSM_BspMsg_Get8 ( hMsg, offset_addrMsb +(i*4),  (uint8_t*)&pStatus->signature01.endAddressMsb );  i++;
            #endif
            BHSM_BspMsg_Get32( hMsg, offset_addr    +(i*4),            &pStatus->signature01.endAddress );     i++;
        }
    }

BHSM_P_DONE_LABEL:

    if( hMsg != NULL ) {
        (void)BHSM_BspMsg_Destroy( hMsg );
    }

    BDBG_LEAVE( BHSM_RegionVerification_Status );
    return rc;
}


/* returns a status indications of all of the regions */
BERR_Code BHSM_RegionVerification_QueryStatus( BHSM_Handle hHsm, BHSM_VerifcationStatus_t *pStatus )
{
    BERR_Code rc = BERR_SUCCESS;
    BHSM_BspMsg_h hMsg = NULL;
    BHSM_BspMsgHeader_t header;
    uint8_t status = 0;
    unsigned i = 0;

    BDBG_ENTER( BHSM_RegionVerification_QueryStatus );

    if( (rc = BHSM_BspMsg_Create( hHsm, &hMsg ) ) != BERR_SUCCESS ) {
        BERR_TRACE( rc );
        goto BHSM_P_DONE_LABEL;
    }

    BHSM_BspMsg_GetDefaultHeader( &header );
    BHSM_BspMsg_Header( hMsg, BHSM_eSESSION_MEM_AUTH_QueryStatus, &header );

    BHSM_BspMsg_Pack8( hMsg, BCMD_MemAuth_InCmdField_eRegionOp, BCMD_MemAuth_Operation_eQueryRegionInfo );

    if( ( rc = BHSM_BspMsg_SubmitCommand( hMsg ) ) != BERR_SUCCESS ) {
        (void)BERR_TRACE(rc);
        goto BHSM_P_DONE_LABEL;
    }

    BHSM_BspMsg_Get8( hMsg, BCMD_MemAuth_OutCmdField_eStatus, &status );
    switch( status )
    {
        case 0:
        {
            BDBG_MSG(("%s: Query Status.", __FUNCTION__ ));
            break;
        }
        default:
        {
            rc = BHSM_STATUS_BSP_ERROR;
            BDBG_WRN(("%s: ERROR[0x%0X] ", __FUNCTION__, status ));
            break;
        }
    }

    #define BHSM_eStatusQuery ((7<<2) + 2) /* BCMD_MemAuth_OutCmdField_eRegion0Status is one byte off.  */
    for( i = 0; i < MAX_REGION_NUMBER; i++ )
    {
        BHSM_BspMsg_Get16( hMsg, BHSM_eStatusQuery+(i*4), (uint16_t*)(&pStatus->region[i]) );
    }

BHSM_P_DONE_LABEL:

    if( hMsg != NULL ) {
        (void)BHSM_BspMsg_Destroy( hMsg );
    }

    BDBG_LEAVE( BHSM_RegionVerification_QueryStatus );
    return rc;
}
