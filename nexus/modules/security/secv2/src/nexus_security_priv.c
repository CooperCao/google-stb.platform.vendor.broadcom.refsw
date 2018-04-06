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
#include "nexus_security.h"
#include "priv/nexus_security_priv.h"
#include "priv/nexus_core.h"
#include "bhsm_exceptions.h"

BDBG_MODULE(nexus_security);

/*
Description:
    Function will itterate over all ARCHes of each available MEM Controller and print any detected violations.
*/
void NEXUS_Security_PrintArchViolation_priv(void)
{
    BERR_Code hsmRc;
    BHSM_ModuleCapabilities caps;
    BHSM_ExceptionMemcArch_t exception;
    BCHP_MemoryInfo memInfo;
    BHSM_Handle hHsm;
    unsigned maxMemc = 0;
    unsigned memcIndex;
    unsigned archIndex;

    BDBG_ENTER(NEXUS_Security_PrintArchViolation_priv);
    NEXUS_ASSERT_MODULE();

    NEXUS_Security_GetHsm_priv( &hHsm );
    if( !hHsm ) { BERR_TRACE( NEXUS_NOT_INITIALIZED ); return; }

    BKNI_Memset( &memInfo, 0, sizeof( memInfo ) );
    hsmRc = BCHP_GetMemoryInfo( g_pCoreHandles->chp, &memInfo );
    if( hsmRc != BERR_SUCCESS ) { BERR_TRACE( hsmRc ); return; }

    hsmRc = BHSM_GetCapabilities( hHsm, &caps );
    if( hsmRc != BERR_SUCCESS ) { BERR_TRACE( hsmRc ); return; }

    maxMemc = sizeof(memInfo.memc)/sizeof(memInfo.memc[0]);

    BKNI_Memset( &exception, 0, sizeof(exception) );

    exception.keepStatus = false;

    for( memcIndex = 0; memcIndex < maxMemc; memcIndex++ )  /* iterate over mem controllers. */
    {
        if( memInfo.memc[memcIndex].valid )              /* if the MEMC is in use. */
        {
            exception.memcIndex = memcIndex;

            for( archIndex = 0; archIndex < caps.archesPerMemc; archIndex++ )   /* itterate over ARCHes. */
            {
                exception.archIndex = archIndex;

                hsmRc = BHSM_Exception_GetMemcArch( hHsm, &exception );
                if( hsmRc != BERR_SUCCESS ) { BERR_TRACE( hsmRc );  continue; }

                if( exception.endAddress ) /* if there has been a violation */
                {
                    BDBG_ERR(("MEMC ARCH Violation. MEMC[%u]ARCH[%u] Addr start [" BDBG_UINT64_FMT  \
                              "] end[" BDBG_UINT64_FMT "] numBlocks[%u] scbClientId[%u:%s] requestType[%#x:%s]",
                              memcIndex,
                              archIndex,
                              BDBG_UINT64_ARG(exception.startAddress),
                              BDBG_UINT64_ARG(exception.endAddress),
                              exception.numBlocks,
                              exception.scbClientId,
                              BMRC_Checker_GetClientName( memcIndex, exception.scbClientId ),
                              exception.requestType,
                              BMRC_Monitor_GetRequestTypeName_isrsafe( exception.requestType ) ));
                }
            }
        }
    }

    BDBG_LEAVE(NEXUS_Security_PrintArchViolation_priv);
    return;
}


void NEXUS_SecurityModule_Sweep_priv(void)
{
    NEXUS_KeySlotHandle keyslot;

    while( ( keyslot = NEXUS_KeySlot_P_GetDeferredDestroy() ) )
    {
        BDBG_ASSERT( keyslot->security.data ); /* if anyone else defers keyslot destroy, we need to enhance api */
        NEXUS_KeySlot_Free_priv( keyslot );
        NEXUS_KeySlot_P_DeferredDestroy( keyslot );
    }

    #if 0
    while (1) {
        NEXUS_PidChannelHandle pidChannel;
        NEXUS_Module_Lock(g_security.moduleSettings.transport);
        pidChannel = NEXUS_PidChannel_GetBypassKeyslotCleanup_priv();
        NEXUS_Module_Unlock(g_security.moduleSettings.transport);
        if (!pidChannel) break;
        NEXUS_SetPidChannelBypassKeySlot_priv(pidChannel, NEXUS_BypassKeySlot_eG2GR);
    }
    #endif
}
