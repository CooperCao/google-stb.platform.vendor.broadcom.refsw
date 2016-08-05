/******************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *****************************************************************************/

#include "nexus_video_encoder_module.h"
#include "nexus_video_encoder_security.h"
#if NEXUS_HAS_SECURITY
 #include "priv/nexus_security_regver_priv.h"
#endif

#include "bvce.h"           /* xvd interface */

/* use RDB to determine NUM_VCE_DEVICES */
#include "bchp_common.h"

#if defined BCHP_VICE2_ARCSS_ESS_CTRL_0_REG_START
  #include "bchp_vice2_arcss_ess_ctrl_0.h"
  #define BCHP_VICE2_ARCSS_ESS_CTRL_0_0_INIT_PROC_START BCHP_VICE2_ARCSS_ESS_CTRL_0_INIT_PROC_START
  #define NUM_VCE_DEVICES 1
#elif defined BCHP_VICE2_ARCSS_ESS_CTRL_0_0_REG_START
  #include "bchp_vice2_arcss_ess_ctrl_0_0.h"
    #if defined BCHP_VICE2_ARCSS_ESS_CTRL_0_1_REG_START
      #include "bchp_vice2_arcss_ess_ctrl_0_1.h"
      #define NUM_VCE_DEVICES 2
    #else
      #define NUM_VCE_DEVICES 1
    #endif
 #else
   #error not supported
#endif

#ifdef NEXUS_ENABLE_VICH
  #include "nexus_video_encoder_register_protection.h"
#endif

BDBG_MODULE(nexus_encoder_security);
BDBG_OBJECT_ID(NEXUS_VideoEncoderSecurity);

#define VIDEOENCODER_MAX_ARCH 2

#if NEXUS_HAS_SECURITY
 NEXUS_Error verifyFirmwareVideoEncoder( BAFL_FirmwareInfo *pstArc, unsigned int deviceId  );
 static NEXUS_Error getRegionId( NEXUS_SecurityRegverRegionID *pRegionId, unsigned deviceId, unsigned arch);

 #define LOCK_SECURITY() NEXUS_Module_Lock(g_NEXUS_VideoEncoder_P_State.config.security)
 #define UNLOCK_SECURITY() NEXUS_Module_Unlock(g_NEXUS_VideoEncoder_P_State.config.security)
#endif

static NEXUS_Error secureFirmwareVideoEncoder( void *pContext, const BAFL_BootInfo *pstViceBootInfo );

/* retrieve function pointers to enable region verification */
void NEXUS_VideoEncoder_P_GetSecurityCallbacks( BVCE_OpenSettings *pSettings, unsigned deviceId )
{
    BDBG_ASSERT ( pSettings != NULL );

    if( deviceId >= NUM_VCE_DEVICES ) {
        BERR_TRACE( NEXUS_INVALID_PARAMETER );  /*invalid index*/
        return;
    }

    pSettings->pARCBootCallback = secureFirmwareVideoEncoder;
    pSettings->pARCBootCallbackData = (void *)deviceId;
    return;
}


void NEXUS_VideoEncoder_P_DisableFwVerification( unsigned deviceId )
{
    unsigned i;

    if( deviceId >= NUM_VCE_DEVICES ) {
        BERR_TRACE( NEXUS_INVALID_PARAMETER );
        return;
    }

    for( i = 0; i < VIDEOENCODER_MAX_ARCH; i++ )
    {
        #if NEXUS_HAS_SECURITY
        NEXUS_Error rc = NEXUS_SUCCESS;
        NEXUS_SecurityRegverRegionID regionId;

        rc = getRegionId( &regionId, deviceId, i );
        if( rc != NEXUS_SUCCESS ) {
            BERR_TRACE( rc ); /* Continue. Best effort. */
            continue;
        }
        LOCK_SECURITY();
        NEXUS_Security_RegionVerifyDisable_priv( regionId );
        UNLOCK_SECURITY();
        #endif
    }
    return;
}

/* this callback will be invoked after the FW is loaded to memory and before it is started. */
static NEXUS_Error secureFirmwareVideoEncoder( void *pContext, const BAFL_BootInfo *pstViceBootInfo )
{
    BREG_Handle hReg = g_pCoreHandles->reg;
    NEXUS_Error rc = NEXUS_SUCCESS;
    unsigned deviceId = (unsigned)pContext;

    BDBG_ASSERT( pstViceBootInfo );
    BDBG_ENTER( secureFirmwareVideoEncoder );

    switch( pstViceBootInfo->eMode )
    {
        case BAFL_BootMode_eNormal:   { BDBG_MSG(("Boot Mode: Normal (%d)",   pstViceBootInfo->eMode)); break; }
        case BAFL_BootMode_eWatchdog: { BDBG_MSG(("Boot Mode: Watchdog (%d)", pstViceBootInfo->eMode)); break; }
        default:                      { BDBG_MSG(("Boot Mode: Unknown (%d)",  pstViceBootInfo->eMode)); }
    }

    #ifdef NEXUS_HAS_SECURITY
    rc = verifyFirmwareVideoEncoder( pstViceBootInfo->pstArc, deviceId );
    if ( rc != NEXUS_SUCCESS) { return BERR_TRACE(rc); }
    #endif

    #if defined(NEXUS_HAS_SECURITY) && defined(NEXUS_ENABLE_VICH)
    {
        rc = NEXUS_VideoEncoder_P_EnableRegisterProtection( deviceId );
        if( rc != NEXUS_SUCCESS ) {
            BDBG_WRN(("VICE register protection might have been enabled.  \n"));
        }
    }
    #endif

    if( deviceId == 0 )
    {
        BREG_Write32( hReg, BCHP_VICE2_ARCSS_ESS_CTRL_0_0_INIT_PROC_START, 1 );
    }
    #if NUM_VCE_DEVICES > 1
    else if( deviceId == 1 )
    {
        BREG_Write32( hReg, BCHP_VICE2_ARCSS_ESS_CTRL_0_1_INIT_PROC_START, 1 );
    }
    #endif

    BDBG_LEAVE(secureFirmwareVideoEncoder);
    return rc;
}


#if NEXUS_HAS_SECURITY
NEXUS_Error verifyFirmwareVideoEncoder( BAFL_FirmwareInfo *pstArc, unsigned deviceId  )
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_SecurityRegionConfiguration regionConfig;
    NEXUS_SecurityRegverRegionID regionId = NEXUS_SecurityRegverRegionID_eMax;

    while( pstArc )
    {
        if( pstArc->stCode.uiSize )
        {
            rc = getRegionId( &regionId, deviceId, pstArc->uiArcInstance );
            if( rc != NEXUS_SUCCESS ) { return BERR_TRACE(rc); }

            NEXUS_FlushCache( (const void*)(pstArc->stCode.pStartAddress), pstArc->stCode.uiSize );

            LOCK_SECURITY();
            NEXUS_Security_RegionGetDefaultConfig_priv( regionId, &regionConfig ); /* use defaults */
            rc = NEXUS_Security_RegionConfig_priv( regionId, &regionConfig );
            UNLOCK_SECURITY();
            if( rc != NEXUS_SUCCESS ) { return BERR_TRACE(rc); }

            LOCK_SECURITY();
            rc = NEXUS_Security_RegionVerifyEnable_priv( regionId,
                                                         NEXUS_AddrToOffset( pstArc->stCode.pStartAddress ),
                                                         pstArc->stCode.uiSize );
            UNLOCK_SECURITY();
            if ( rc != NEXUS_SUCCESS ) { return BERR_TRACE( rc ); }
        }
        pstArc = pstArc->pNext;
    }

    return NEXUS_SUCCESS;
}

/* Map the device firmware to a "security region Id" */
static NEXUS_Error getRegionId( NEXUS_SecurityRegverRegionID *pRegionId, unsigned deviceId, unsigned arch )
{
    switch( deviceId )
    {
        case 0:
        {
            switch( arch )
            {
                case 0:  { *pRegionId = NEXUS_SecurityRegverRegionID_eVice0Pic; break; }
                case 1:  { *pRegionId = NEXUS_SecurityRegverRegionID_eVice0MacroBlk; break; }
                default: { return BERR_TRACE( NEXUS_INVALID_PARAMETER ); }
            }
            break;
        }
        #if (NEXUS_NUM_VCE_DEVICES > 1)
        case 1:
        {
            switch( arch )
            {
                case 0:  { *pRegionId = NEXUS_SecurityRegverRegionID_eVice1Pic; break; }
                case 1:  { *pRegionId = NEXUS_SecurityRegverRegionID_eVice1MacroBlk; break; }
                default: { return BERR_TRACE( NEXUS_INVALID_PARAMETER ); }
            }
            break;
        }
        #endif
        default:
        {
            return BERR_TRACE( NEXUS_INVALID_PARAMETER );
        }
    }

    return NEXUS_SUCCESS;
}
#endif
