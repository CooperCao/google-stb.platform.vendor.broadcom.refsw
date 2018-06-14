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
#include "nexus_security_module.h"
#include "nexus_base.h"
#include "nexus_types.h"
#include "nexus_security_common.h"
#include "nexus_pcie_window.h"
#include "nexus_signed_command.h"
#include "priv/nexus_security_priv.h"
#include "bhsm_pcie_window.h"
#include "priv/nexus_core.h"


BDBG_MODULE(nexus_pcie_window);


void NEXUS_Security_GetDefaultPciEMaxWindowSizeSettings( NEXUS_SecurityPciEMaxWindowSizeSettings *pConfig )
{
    BDBG_ENTER( NEXUS_Security_GetDefaultPciEMaxWindowSizeSettings );

    if( !pConfig ) { BERR_TRACE( NEXUS_INVALID_PARAMETER ); return; }

    BKNI_Memset( pConfig, 0, sizeof(*pConfig) );

    BDBG_LEAVE( NEXUS_Security_GetDefaultPciEMaxWindowSizeSettings );
    return;
}


NEXUS_Error NEXUS_Security_SetPciEMaxWindowSize( const NEXUS_SecurityPciEMaxWindowSizeSettings *pConfig )
{

  #if NEXUS_ZEUS_VERSION >= NEXUS_ZEUS_VERSION_CALC(5,0)
    NEXUS_Error rc = NEXUS_UNKNOWN;
    NEXUS_SignedCommand *pSignedCommand = NULL;

    BDBG_ENTER( NEXUS_Security_SetPciEMaxWindowSize );

    if( !pConfig ) { return BERR_TRACE( NEXUS_INVALID_PARAMETER ); }
    if( pConfig->signedCommandLength != NEXUS_SIGNED_COMMAND_SIZE ) { return BERR_TRACE( NEXUS_INVALID_PARAMETER ); }

    rc = NEXUS_Memory_Allocate( sizeof(NEXUS_SignedCommand), NULL, (void**)&pSignedCommand );
    if( rc != NEXUS_SUCCESS ) { BERR_TRACE( rc ); goto error; }

    NEXUS_GetDefaultSignedCommand( pSignedCommand );

    BKNI_Memcpy( &pSignedCommand->signedCommand, pConfig->signedCommand, NEXUS_SIGNED_COMMAND_SIZE );
    pSignedCommand->signingAuthority = pConfig->signingAuthority;

    rc = NEXUS_SetSignedCommand( pSignedCommand );
    if( rc != NEXUS_SUCCESS ) { BERR_TRACE( rc ); goto error; }

error:
    if( pSignedCommand ) NEXUS_Memory_Free( pSignedCommand );

    return rc;
   #else
    BSTD_UNUSED( pConfig );
    return NEXUS_NOT_SUPPORTED;
   #endif
}


NEXUS_Error NEXUS_Security_SetPciERestrictedRange( NEXUS_Addr baseOffset, size_t size, unsigned index )
{
  #if NEXUS_ZEUS_VERSION >= NEXUS_ZEUS_VERSION_CALC(5,0)
    BERR_Code rc = NEXUS_SUCCESS;
    BHSM_PcieWindowSettings pcieWindow;
    BHSM_Handle hHsm;
    unsigned i = 0;
    unsigned numMemc = 0;
    const BCHP_MemoryLayout *pMemcLayout; /* the layout of the memory controllers. */

    BDBG_ENTER( NEXUS_Security_SetPciERestrictedRange );

    NEXUS_Security_GetHsm_priv( &hHsm );
    if( !hHsm ) { return BERR_TRACE( NEXUS_NOT_INITIALIZED ); }

    pMemcLayout = &g_pCoreHandles->memoryLayout;

    BDBG_CASSERT( NEXUS_MAX_MEMC == sizeof(pMemcLayout->memc)/sizeof(pMemcLayout->memc[0]) );

    numMemc = sizeof(pMemcLayout->memc)/sizeof(pMemcLayout->memc[0]);

    for( i = 0; i < numMemc; i++ )
    {
        if( ( baseOffset >=  pMemcLayout->memc[i].region[0].addr ) &&
            ( baseOffset < ( pMemcLayout->memc[i].region[0].addr + pMemcLayout->memc[i].region[0].size ) ) )
        {
            BDBG_MSG(("OPENING PCIE[%d] MEMC[%d] address[" BDBG_UINT64_FMT "] size[%lu]", index,  i, BDBG_UINT64_ARG(baseOffset), (long int)size ));

            BKNI_Memset( &pcieWindow, 0, sizeof(pcieWindow) );
            pcieWindow.baseOffset = baseOffset;
            pcieWindow.size = size;
            pcieWindow.index = index;

            rc = BHSM_PcieWindow_Set( hHsm, &pcieWindow );
            if( rc != BERR_SUCCESS ) { return BERR_TRACE( rc ); }
        }
        else if( pMemcLayout->memc[i].region[0].addr && pMemcLayout->memc[i].region[0].size )
        {
            BDBG_MSG(("LOCKING PCIE[%d] MEMC[%d] address[" BDBG_UINT64_FMT "]", index, i, BDBG_UINT64_ARG(baseOffset) ));
            BKNI_Memset( &pcieWindow, 0, sizeof(pcieWindow) );
            pcieWindow.baseOffset = pMemcLayout->memc[i].region[0].addr; /* must be different from the input value of baseOffset */
            pcieWindow.size = 0; /* lock the MEMC.  */
            pcieWindow.index = index;

            rc = BHSM_PcieWindow_Set( hHsm, &pcieWindow );
            if( rc != BERR_SUCCESS ) { return BERR_TRACE( rc ); }
        }
    }

    BDBG_LEAVE( NEXUS_Security_SetPciERestrictedRange );
    return NEXUS_SUCCESS;
   #else
    BSTD_UNUSED( baseOffset );
    BSTD_UNUSED( size );
    BSTD_UNUSED( index );
    return NEXUS_NOT_SUPPORTED;
   #endif
}
