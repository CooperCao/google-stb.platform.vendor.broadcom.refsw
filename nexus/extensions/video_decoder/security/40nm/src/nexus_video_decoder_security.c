/******************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2011-2016 Broadcom. All rights reserved.
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
 *
 *****************************************************************************/

#include "nexus_video_decoder_module.h"
#include "nexus_video_decoder_security.h"
#if NEXUS_HAS_SECURITY
 #include "nexus_security_datatypes.h"
 #include "priv/nexus_security_regver_priv.h"
 #include "bafl.h"
#endif

#include "bxvd.h"           /* xvd interface */
#include "bchp_common.h"

#ifndef BCHP_HEVD_OL_CPU_REGS_0_REG_START
 #include "bchp_decode_ind_sdram_regs2_0.h"
 #include "bchp_decode_cpuaux2_0.h"
 #if (NEXUS_NUM_XVD_DEVICES==2)
   #include "bchp_decode_ind_sdram_regs2_1.h"
   #include "bchp_decode_cpuaux2_1.h"
 #endif
 #include "bchp_decode_ind_sdram_regs_0.h"
#else
 #include "bchp_hevd_ol_cpu_regs_0.h"
 #include "bchp_hevd_ol_cpu_debug_0.h"
 #if (NEXUS_NUM_XVD_DEVICES>=2)
  #include "bchp_hevd_ol_cpu_regs_1.h"
  #include "bchp_hevd_ol_cpu_debug_1.h"
 #endif
 #if (NEXUS_NUM_XVD_DEVICES==3)
   #include "bchp_hevd_ol_cpu_regs_2.h"
   #include "bchp_hevd_ol_cpu_debug_2.h"
 #endif
#endif

#if defined(NEXUS_HAS_SECURITY) && defined(NEXUS_ENABLE_VICH)
#include "nexus_video_decoder_register_protection.h"
#endif

#ifdef BCHP_HEVD_OL_CPU_REGS_0_REG_START
/* #include "bchp_hevd_ol_cpu_dma_0.h"*/
/* #include "bchp_hevd_ol_cpu_dma_1.h" */
/* #include "bchp_hevd_ol_cpu_dma_2.h" */
#endif


#if  (NEXUS_NUM_XVD_DEVICES)

BDBG_MODULE(nexus_decoder_security);
BDBG_OBJECT_ID(NEXUS_VideoDecoderSecurity);

static NEXUS_VideoDecoderSecureBootContext gVideoDecoderSecurityContext[NEXUS_NUM_XVD_DEVICES];

#if defined(NEXUS_HAS_SECURITY) && defined(NEXUS_ENABLE_VICH)
static NEXUS_Error startVideoDecoderSecure( unsigned deviceId );
#else
static NEXUS_Error startVideoDecoderUnSecure( unsigned deviceId );
#endif

#if NEXUS_HAS_SECURITY
 NEXUS_Error verifyFirmwareVideoDecoder( BAFL_FirmwareInfo *pstArc, unsigned int deviceID );
 static NEXUS_Error getRegionId( NEXUS_SecurityRegverRegionID *pRegionId, unsigned deviceId, unsigned arch);
 #define LOCK_SECURITY()  NEXUS_Module_Lock(g_NEXUS_videoDecoderModuleInternalSettings.security);
 #define UNLOCK_SECURITY() NEXUS_Module_Unlock(g_NEXUS_videoDecoderModuleInternalSettings.security);
#endif
static NEXUS_Error secureFirmwareVideoDecoder(void *pPrivateData, BAFL_BootInfo *pstAVDBootInfo);


static NEXUS_Error secureFirmwareVideoDecoder(void *pPrivateData, BAFL_BootInfo *pstAVDBootInfo)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_VideoDecoderSecureBootContext *pContext = (NEXUS_VideoDecoderSecureBootContext *)pPrivateData;

    BDBG_ASSERT(pPrivateData);
    BDBG_ASSERT(pstAVDBootInfo);
    BDBG_ENTER(secureFirmwareVideoDecoder);

    switch( pstAVDBootInfo->eMode )
    {
        case BAFL_BootMode_eNormal:   { BDBG_MSG(("Boot Mode: Normal (%d)", pstAVDBootInfo->eMode)); break;   }
        case BAFL_BootMode_eWatchdog: { BDBG_MSG(("Boot Mode: Watchdog (%d)", pstAVDBootInfo->eMode)); break; }
        default:                      { BDBG_MSG(("Boot Mode: Unknown (%d)", pstAVDBootInfo->eMode));         }
    }


    #if NEXUS_HAS_SECURITY
    rc = verifyFirmwareVideoDecoder( pstAVDBootInfo->pstArc, pContext->deviceID );
    if ( rc != NEXUS_SUCCESS ) { return BERR_TRACE(rc); }
    # endif

    #if defined(NEXUS_HAS_SECURITY) && defined(NEXUS_ENABLE_VICH)
    rc = startVideoDecoderSecure( pContext->deviceID );  /* Start video with register protection. */
    #else
    rc = startVideoDecoderUnSecure( pContext->deviceID ); /* Start video without register protection.*/
    #endif
    if ( rc != NEXUS_SUCCESS ) { return BERR_TRACE(rc); }

    BDBG_LEAVE(secureFirmwareVideoDecoder);
    return rc;
}

#if defined(NEXUS_HAS_SECURITY) && defined(NEXUS_ENABLE_VICH)
/* protect video decoder registers and start video decoder via BSP.*/
static NEXUS_Error startVideoDecoderSecure( unsigned deviceId )
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_VideoDecoderRegisterProtectionConfig config;

    BDBG_ENTER(startVideoDecoderSecure);

    config.index = deviceId;
    rc = NEXUS_VideoDecoder_P_EnableRegisterProtection( &config );
    if ( rc != NEXUS_SUCCESS ) {
       /* Continue. Need to log SecureStart debug. */
    }

    rc = NEXUS_VideoDecoder_P_SecureStart( &config );
    if ( rc != NEXUS_SUCCESS ) {
        return BERR_TRACE( rc );
    }

    BDBG_LEAVE(startVideoDecoderSecure);
    return rc;
}
#else /* NEXUS_ENABLE_VICH */
/* start the video decoder. */
static NEXUS_Error startVideoDecoderUnSecure( unsigned deviceId )
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    BREG_Handle hReg = g_pCoreHandles->reg;

    BDBG_ENTER(startVideoDecoderUnSecure);

    if( deviceId == 0 )
    {
       #ifndef BCHP_HEVD_OL_CPU_REGS_0_REG_START
        BREG_Write32( hReg, BCHP_DECODE_IND_SDRAM_REGS2_0_REG_CPU_DBG, 1 );
        BREG_Write32( hReg, (BCHP_DECODE_CPUAUX2_0_CPUAUX_REG+0x18),   0 );
        BREG_Write32( hReg, (BCHP_DECODE_CPUAUX2_0_CPUAUX_REG+0x28),   0 );
       #else
        BREG_Write32( hReg, BCHP_HEVD_OL_CPU_REGS_0_DEBUG_CTL, 1                                     );
        BREG_Write32( hReg,  BCHP_HEVD_OL_CPU_DEBUG_0_AUX_REGi_ARRAY_BASE+0x18  /*  ARC_PC */, 0     );
        BREG_Write32( hReg,  BCHP_HEVD_OL_CPU_DEBUG_0_AUX_REGi_ARRAY_BASE+0x28  /*  ARC_STATUS */, 0 );
       #endif
    }

    #if (NEXUS_NUM_XVD_DEVICES>=2)
    else if ( deviceId == 1 )
    {
       #ifndef BCHP_HEVD_OL_CPU_REGS_0_REG_START
        BREG_Write32(hReg, BCHP_DECODE_IND_SDRAM_REGS2_1_REG_CPU_DBG, 1 );
        BREG_Write32(hReg, (BCHP_DECODE_CPUAUX2_1_CPUAUX_REG+0x18),   0 );
        BREG_Write32(hReg, (BCHP_DECODE_CPUAUX2_1_CPUAUX_REG+0x28),   0 );  /*Taking AVD core out of debug mode */
       #else
        BREG_Write32(hReg,    BCHP_HEVD_OL_CPU_REGS_1_DEBUG_CTL, 1);
        BREG_Write32(hReg,    BCHP_HEVD_OL_CPU_DEBUG_1_AUX_REGi_ARRAY_BASE+0x18  /*  ARC_PC */, 0);
        BREG_Write32(hReg,    BCHP_HEVD_OL_CPU_DEBUG_1_AUX_REGi_ARRAY_BASE+0x28  /*  ARC_STATUS */, 0);
       #endif
    }
    #endif

    #if (NEXUS_NUM_XVD_DEVICES>=3) && defined(BCHP_HEVD_OL_CPU_REGS_0_REG_START )
    else if ( deviceId == 2 )
    {
        BREG_Write32(hReg,    BCHP_HEVD_OL_CPU_REGS_2_DEBUG_CTL, 1);
        BREG_Write32(hReg,    BCHP_HEVD_OL_CPU_DEBUG_2_AUX_REGi_ARRAY_BASE + 0x18  /*  ARC_PC */, 0);
        BREG_Write32(hReg,    BCHP_HEVD_OL_CPU_DEBUG_2_AUX_REGi_ARRAY_BASE + 0x28  /*  ARC_STATUS */, 0);
    }
    #endif

    BDBG_LEAVE(startVideoDecoderUnSecure);
    return rc;
}
#endif

/* retrieve function pointers to enable region verification and register protection */
void NEXUS_VideoDecoder_P_GetSecurityCallbacks( BXVD_Settings *pSettings/*out*/, unsigned deviceId )
{
    BDBG_ASSERT ( pSettings != NULL );

    BDBG_ENTER(NEXUS_VideoDecoder_P_GetSecurityCallbacks);

    if( deviceId >= NEXUS_NUM_XVD_DEVICES ) {
        BERR_TRACE(NEXUS_INVALID_PARAMETER);
        return;
    }

    gVideoDecoderSecurityContext[deviceId].deviceID = deviceId;
    pSettings->pAVDBootCallback = secureFirmwareVideoDecoder;
    pSettings->pAVDBootCallbackData = &gVideoDecoderSecurityContext[deviceId];
#if (BCHP_CHIP!=7445)&& (BCHP_CHIP!=7252) && (BCHP_CHIP!=7439) && (BCHP_CHIP!=74371)
   #if defined(NEXUS_HAS_SECURITY) && defined(NEXUS_ENABLE_VICH)
    pSettings->pAVDResetCallback = NEXUS_VideoDecoder_P_AVDResetCallback;
    pSettings->pAVDResetCallbackData = &gVideoDecoderSecurityContext[deviceId];
   #endif
#endif

    BDBG_LEAVE(NEXUS_VideoDecoder_P_GetSecurityCallbacks);
    return;
}

void NEXUS_VideoDecoder_P_DisableFwVerification( unsigned deviceId )
{
    unsigned i;

    if( deviceId >= NEXUS_NUM_XVD_DEVICES  )
    {
        BERR_TRACE(NEXUS_INVALID_PARAMETER);
        return;
    }

    BDBG_ENTER(NEXUS_VideoDecoder_P_DisableFwVerification);

    for( i = 0; i < VIDEODECODER_MAX_ARCH; i++ )
    {
        if( gVideoDecoderSecurityContext[deviceId].arch[i].verifed == true )
        {
           #if NEXUS_HAS_SECURITY
            NEXUS_Error rc = NEXUS_SUCCESS;

            NEXUS_SecurityRegverRegionID regionId;
            rc = getRegionId( &regionId, deviceId, i );
            if( rc != NEXUS_SUCCESS ) {
                BERR_TRACE( rc );
                continue;   /* Continue. Best effort. */
            }
            LOCK_SECURITY();
            NEXUS_Security_RegionVerifyDisable_priv( regionId );
            UNLOCK_SECURITY();
           #endif
            gVideoDecoderSecurityContext[deviceId].arch[i].verifed = false;
        }
    }

    BDBG_LEAVE(NEXUS_VideoDecoder_P_DisableFwVerification);
    return;
}



/*
Summary:
   For each firmware image:
        - get the region id
        - Configure security to verify the region ( accept defaults for the region id.)
        - Verify the securiyt region.
*/
#if NEXUS_HAS_SECURITY
NEXUS_Error verifyFirmwareVideoDecoder( BAFL_FirmwareInfo *pstArc, unsigned int deviceID )
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_SecurityRegionConfiguration regionConfig;
    NEXUS_SecurityRegverRegionID regionId;

    BDBG_ENTER(verifyFirmwareVideoDecoder);

    while( pstArc )
    {
        if( pstArc->stCode.uiSize ) /* if code size is greater than zero*/
        {
            rc = getRegionId( &regionId, deviceID, pstArc->uiArcInstance );
            if( rc ) { return BERR_TRACE(rc); }

            LOCK_SECURITY();
            NEXUS_Security_RegionGetDefaultConfig_priv( regionId, &regionConfig );/* use defaults. */
            rc = NEXUS_Security_RegionConfig_priv( regionId, &regionConfig );
            UNLOCK_SECURITY();
            if( rc ) { return BERR_TRACE(rc);  }

            NEXUS_FlushCache( (const void*)(pstArc->stCode.pStartAddress), pstArc->stCode.uiSize );

            LOCK_SECURITY();
            rc = NEXUS_Security_RegionVerifyEnable_priv ( regionId, pstArc->stCode.pStartAddress, pstArc->stCode.uiSize );
            UNLOCK_SECURITY();
            if( rc != NEXUS_SUCCESS ) { return BERR_TRACE(rc); } /*Failed to verify video decoder region */

            gVideoDecoderSecurityContext[deviceID].arch[pstArc->uiArcInstance].verifed = true;

            BDBG_MSG(("Region[0x%02X] DeviceID[%x] ARC[%x] Addr[%p] size[%d]", regionId, deviceID, pstArc->uiArcInstance,
                      (void*)pstArc->stCode.pStartAddress, pstArc->stCode.uiSize ));
        }
        pstArc = pstArc->pNext;
    }

    BDBG_LEAVE(verifyFirmwareVideoDecoder);
    return NEXUS_SUCCESS;
}


/*Determine the Region Id from the Device Id and the Arch number*/
static NEXUS_Error getRegionId( NEXUS_SecurityRegverRegionID *pRegionId, unsigned deviceId, unsigned arch )
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    switch( deviceId )
    {
        case 0:
        {
            switch( arch )
            {
               #ifndef NEXUS_VIDEO_DECODER_SECURITY_NO_SVD
                case 0:
                {
                    #if NEXUS_ZEUS_VERSION >= NEXUS_ZEUS_VERSION_CALC(4,2)
                    *pRegionId = NEXUS_SecurityRegverRegionID_eVDEC0_OLA;
                    #else
                    *pRegionId = NEXUS_SecurityRegverRegionID_eSvd0Outer;
                    #endif
                    break;
                }
                case 1:
                {
                    #if NEXUS_ZEUS_VERSION >= NEXUS_ZEUS_VERSION_CALC(4,2)
                      *pRegionId = NEXUS_SecurityRegverRegionID_eVDEC0_ILA;
                    #else
                      *pRegionId = NEXUS_SecurityRegverRegionID_eSvd0Inner;
                    #endif
                      break;
                }
                case 2:
                {
                    #if NEXUS_ZEUS_VERSION >= NEXUS_ZEUS_VERSION_CALC(4,2)
                    *pRegionId = NEXUS_SecurityRegverRegionID_eVDEC0_IL2A;
                    #else
                    *pRegionId = NEXUS_SecurityRegverRegionID_eSvd0Bl;
                    #endif
                    break;
                }
               #else
                case 0:
                {
                    *pRegionId = NEXUS_SecurityRegverRegionID_eAvd0Outer;
                    break;
                }
                case 1:
                {
                    *pRegionId = NEXUS_SecurityRegverRegionID_eAvd0Inner;
                    break;
                }
               #endif
                default:
                {
                    BDBG_WRN(("Unsupported ARCH deviceId [%d] ARCH[%d]", deviceId, arch ));
                    rc = NEXUS_INVALID_PARAMETER;
                    break;
                }
            }
            break;
        }

       #if NEXUS_NUM_XVD_DEVICES > 1
        case 1:
        {
            switch( arch )
            {
                case 0:
                {
                    #if NEXUS_ZEUS_VERSION >= NEXUS_ZEUS_VERSION_CALC(4,2)
                    *pRegionId = NEXUS_SecurityRegverRegionID_eVDEC1_OLA;
                    #else
                    *pRegionId = NEXUS_SecurityRegverRegionID_eAvd0Outer;
                    #endif
                    break;
                }
                case 1:
                {
                    #if NEXUS_ZEUS_VERSION >= NEXUS_ZEUS_VERSION_CALC(4,2)
                    *pRegionId = NEXUS_SecurityRegverRegionID_eVDEC1_ILA;
                    #else
                    *pRegionId = NEXUS_SecurityRegverRegionID_eAvd0Inner;
                    #endif
                    break;
                }
                default:
                {
                    BDBG_WRN(("Unsupported ARCH deviceId [%d] ARCH[%d]", deviceId, arch ));
                    rc = NEXUS_INVALID_PARAMETER;
                    break;
                }
            }
            break;
        }
       #endif

       #if NEXUS_NUM_XVD_DEVICES > 2
        case 2:
        {
            switch( arch )
            {
                case 0:
                {
                    *pRegionId = NEXUS_SecurityRegverRegionID_eVDEC2_OLA;
                    break;
                }
                case 1:
                {
                    *pRegionId = NEXUS_SecurityRegverRegionID_eVDEC2_ILA;
                    break;
                }
                default:
                {
                    BDBG_WRN(("Unsupported ARCH deviceId [%d] ARCH[%d]", deviceId, arch ));
                    rc = NEXUS_INVALID_PARAMETER;
                    break;
                }
            }
            break;
        }
       #endif

        default:
        {
            BDBG_WRN(("Unsupported Device ID [%d] ARCH[%d]", deviceId, arch ));
            rc = NEXUS_INVALID_PARAMETER;
        }
    }

    return rc;
}
#endif /* NEXUS_HAS_SECURITY */

#endif /* NEXUS_NUM_XVD_DEVICES */
