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
#include "nexus_security_module.h"
#include "nexus_types.h"
#include "nexus_security_datatypes.h"
#include "nexus_region_verification.h"
#include "priv/nexus_security_regver_priv.h"


BDBG_MODULE(nexus_region_verification);

struct NEXUS_RegionVerify
{
    NEXUS_OBJECT(NEXUS_RegionVerify);
    unsigned allocated;
    NEXUS_SecurityRegverRegionID regionId;

    NEXUS_RegionVerifySettings settings;
};

#define NEXUS_REGION_VERIFICATION_MAX_INSTANCES 2

static void GetDefaultSettings( NEXUS_RegionVerifySettings *pSettings );

static struct NEXUS_RegionVerify g_instance[NEXUS_REGION_VERIFICATION_MAX_INSTANCES];

NEXUS_Error NEXUS_RegionVerify_Init( void )
{
    BDBG_ENTER( NEXUS_RegionVerify_Init );

    BKNI_Memset( g_instance, 0, sizeof(g_instance) );

    g_instance[0].regionId = NEXUS_SecurityRegverRegionID_eHost02;
    g_instance[1].regionId = NEXUS_SecurityRegverRegionID_eHost03;

    BDBG_LEAVE( NEXUS_RegionVerify_Init );
    return NEXUS_SUCCESS;
}


void NEXUS_RegionVerify_Uninit( void )
{
    BDBG_ENTER( NEXUS_RegionVerify_Uninit );

    BDBG_LEAVE( NEXUS_RegionVerify_Uninit );
    return;
}

NEXUS_RegionVerifyHandle NEXUS_RegionVerify_Open( unsigned index )
{
    unsigned x = 0;
    NEXUS_RegionVerifyHandle handle = NULL;

    BDBG_ENTER( NEXUS_RegionVerify_Open );

    if( index >= NEXUS_REGION_VERIFICATION_MAX_INSTANCES && index != NEXUS_ANY_ID ) {
        BERR_TRACE( NEXUS_INVALID_PARAMETER );
        return NULL;
    }

    if( index < NEXUS_REGION_VERIFICATION_MAX_INSTANCES ) {

        if( g_instance[index].allocated ) {
           BERR_TRACE( NEXUS_NOT_AVAILABLE );
           goto done;
        }

        g_instance[index].allocated = true;
        handle = &g_instance[index];
        goto done;
    }

    for( x = 0; !handle && x < NEXUS_REGION_VERIFICATION_MAX_INSTANCES; x++ )
    {
        if( g_instance[x].allocated == false )
        {
            g_instance[x].allocated = true;
            handle = &g_instance[x];
        }
    }

done:

    if( !handle ) { BERR_TRACE( NEXUS_NOT_AVAILABLE ); return NULL; }

    GetDefaultSettings( &handle->settings );

    NEXUS_OBJECT_SET(NEXUS_RegionVerify, handle);

    BDBG_LEAVE( NEXUS_RegionVerify_Open );
    return handle;
}

NEXUS_OBJECT_CLASS_MAKE( NEXUS_RegionVerify, NEXUS_RegionVerify_Close );

static void NEXUS_RegionVerify_P_Finalizer( NEXUS_RegionVerifyHandle handle )
{
    BDBG_ENTER( NEXUS_RegionVerify_P_Finalizer );

    NEXUS_OBJECT_ASSERT( NEXUS_RegionVerify, handle );

    handle->allocated = false;
    NEXUS_OBJECT_DESTROY( NEXUS_RegionVerify, handle );

    BDBG_ENTER( NEXUS_RegionVerify_P_Finalizer );
    return;
}


void NEXUS_RegionVerify_GetSettings( NEXUS_RegionVerifyHandle handle,
                                     NEXUS_RegionVerifySettings *pSettings )
{
    BDBG_ENTER( NEXUS_RegionVerify_GetSettings );

    NEXUS_OBJECT_ASSERT( NEXUS_RegionVerify, handle );

    if( !pSettings ) { BERR_TRACE( NEXUS_INVALID_PARAMETER ); return; }

    *pSettings = handle->settings;

    BDBG_LEAVE( NEXUS_RegionVerify_GetSettings );
    return;
}


NEXUS_Error NEXUS_RegionVerify_SetSettings( NEXUS_RegionVerifyHandle handle,
                                            const NEXUS_RegionVerifySettings *pSettings )
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_SecurityRegionConfiguration internalConfg;

    BDBG_ENTER( NEXUS_RegionVerify_SetSettings );

    if( !pSettings ) { return BERR_TRACE( NEXUS_INVALID_PARAMETER ); }

    if( handle->settings.enabled == pSettings->enabled )
    {
        goto done;
    }

    if( pSettings->enabled == false )
    {
        NEXUS_Security_RegionVerifyDisable_priv( handle->regionId );

        GetDefaultSettings( &handle->settings ); /* reset setting to default. */
        goto done;
    }

    /* configure and enable region verification. */

    if( pSettings->signature.size > NEXUS_REGIONVERIFY_SIGNATURE_PLUS_HEADER_SIZE ) {
        rc = BERR_TRACE( NEXUS_INVALID_PARAMETER );
        goto done;
    }

    NEXUS_Security_RegionGetDefaultConfig_priv( handle->regionId, &internalConfg );

    internalConfg.signature.size = pSettings->signature.size;
    BKNI_Memcpy( internalConfg.signature.data, pSettings->signature.data, pSettings->signature.size );

    internalConfg.signedAttributesValid         = true;
    internalConfg.signedAttributes.cpuType      = NEXUS_SecurityCpuType_eHost;
    internalConfg.signedAttributes.marketId     = pSettings->signature.attributes.marketId;
    internalConfg.signedAttributes.marketIdMask = pSettings->signature.attributes.marketIdMask;
    internalConfg.signedAttributes.epoch        = pSettings->signature.attributes.systemEpoch;
    internalConfg.signedAttributes.epochMask    = pSettings->signature.attributes.systemEpochMask;
   #if NEXUS_ZEUS_VERSION >= NEXUS_ZEUS_VERSION_CALC(4,2)
    /*internalConfg.signedAttributes.svpFwReleaseVersion = 0;*/
    internalConfg.signedAttributes.signatureType    = NEXUS_SecuritySignatureType_eCode;
    internalConfg.signedAttributes.signatureVersion = 1;
    internalConfg.signedAttributes.epochSelect      = pSettings->signature.attributes.systemEpochSelect;
   #endif
    internalConfg.rsaKeyIndex                   = pSettings->rsaKeyIndex;
    internalConfg.useManagedRsaKey              = false;
    internalConfg.forceVerification             = true;

    rc = NEXUS_Security_RegionConfig_priv( handle->regionId, &internalConfg );
    if( rc ){ BERR_TRACE( rc ); goto done; }

    rc =  NEXUS_Security_RegionVerifyEnable_priv( handle->regionId, pSettings->regionAddress, pSettings->regionSize );
    if( rc ){ BERR_TRACE( rc ); goto done; }

    handle->settings = *pSettings;

done:

    BDBG_LEAVE( NEXUS_RegionVerify_SetSettings );
    return rc;
}

static void GetDefaultSettings( NEXUS_RegionVerifySettings *pSettings )
{
    if( pSettings ) {
        BKNI_Memset( pSettings, 0, sizeof(*pSettings) );
    }

    return;
}
