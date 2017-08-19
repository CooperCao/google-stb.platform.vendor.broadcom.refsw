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
#include "nexus_base.h"
#include "nexus_security_module.h"
#include "nexus_types.h"
#include "nexus_rsa.h"

BDBG_MODULE(nexus_rsa);


/* RSA Instance data, one instance per handle. */
struct NEXUS_Rsa  /* Instance handle structure */
{
    NEXUS_OBJECT(NEXUS_Rsa);

    BLST_S_ENTRY(NEXUS_Rsa) next;
    NEXUS_RsaExponentiateSettings settings; /* the instace settings/data. */
};



NEXUS_Error NEXUS_Rsa_Init( void )
{
    BDBG_ENTER( NEXUS_Rsa_Init );


    BDBG_LEAVE( NEXUS_Rsa_Init );
    return NEXUS_SUCCESS;
}

void NEXUS_Rsa_Uninit( void )
{
    BDBG_ENTER( NEXUS_Rsa_Uninit );


    BDBG_LEAVE( NEXUS_Rsa_Uninit );
    return;
}


NEXUS_RsaHandle NEXUS_Rsa_Open( unsigned index )
{
    NEXUS_RsaHandle handle;

    BDBG_ENTER( NEXUS_Rsa_Open );
    BSTD_UNUSED( index );

    handle = BKNI_Malloc( sizeof(struct NEXUS_Rsa) );
    if( !handle ) { BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY); return NULL; }

    NEXUS_OBJECT_INIT(NEXUS_Rsa, handle);


    BDBG_LEAVE( NEXUS_Rsa_Open );
    return handle;
}

NEXUS_OBJECT_CLASS_MAKE( NEXUS_Rsa, NEXUS_Rsa_Close );

static void NEXUS_Rsa_P_Finalizer( NEXUS_RsaHandle handle )
{
    BDBG_ENTER( NEXUS_Rsa_P_Finalizer );
    NEXUS_OBJECT_ASSERT( NEXUS_Rsa, handle );

    NEXUS_OBJECT_DESTROY(NEXUS_Rsa, handle);
    BKNI_Free( handle );

    BDBG_LEAVE( NEXUS_Rsa_P_Finalizer );
	return;
}

void  NEXUS_Rsa_GetDefaultExponentiateSettings( NEXUS_RsaHandle handle, NEXUS_RsaExponentiateSettings *pSettings )
{
    BDBG_ENTER( NEXUS_Rsa_GetDefaultExponentiateSettings );
    NEXUS_OBJECT_ASSERT( NEXUS_Rsa, handle );
    BSTD_UNUSED( pSettings );


    BDBG_LEAVE( NEXUS_Rsa_GetDefaultExponentiateSettings );
    return;
}

NEXUS_Error NEXUS_Rsa_Exponentiate( NEXUS_RsaHandle handle, const NEXUS_RsaExponentiateSettings *pSettings )
{
    BDBG_ENTER( NEXUS_Rsa_Exponentiate );
    NEXUS_OBJECT_ASSERT( NEXUS_Rsa, handle );
    BSTD_UNUSED( pSettings );


    BDBG_LEAVE( NEXUS_Rsa_Exponentiate );
    return NEXUS_SUCCESS;
}

void NEXUS_Rsa_GetDefaultResult( NEXUS_RsaExponentiateResult *pResult )
{
    BDBG_ENTER( NEXUS_Rsa_GetDefaultResult );
    BSTD_UNUSED( pResult );


    BDBG_LEAVE( NEXUS_Rsa_GetDefaultResult );
    return;
}

NEXUS_Error NEXUS_Rsa_GetResult( NEXUS_RsaHandle handle, NEXUS_RsaExponentiateResult *pResult )
{
    BDBG_ENTER( NEXUS_Rsa_GetResult );
    NEXUS_OBJECT_ASSERT( NEXUS_Rsa, handle );

    BSTD_UNUSED( pResult );


    BDBG_LEAVE( NEXUS_Rsa_GetResult );
    return NEXUS_SUCCESS;
}
