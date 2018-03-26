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
#include "priv/nexus_core.h"
#include "priv/nexus_security_priv.h"
#include "priv/nexus_pid_channel_priv.h"
#include "nexus_base.h"
#include "bhsm.h"
#include "bhsm_keyslot.h"

BDBG_MODULE(nexus_security);

static NEXUS_Error _SetPidChannelBypassKeyslot( NEXUS_PidChannelHandle pidChannel, NEXUS_BypassKeySlot bypassKeySlot );


NEXUS_Error NEXUS_SetPidChannelBypassKeyslot( NEXUS_PidChannelHandle pidChannel, NEXUS_BypassKeySlot bypassKeySlot )
{
    NEXUS_SecurityModule_Sweep_priv();
    return _SetPidChannelBypassKeyslot( pidChannel, bypassKeySlot );
}

static NEXUS_Error _SetPidChannelBypassKeyslot( NEXUS_PidChannelHandle pidChannel, NEXUS_BypassKeySlot bypassKeySlot )
{
    NEXUS_Error rc = NEXUS_UNKNOWN;
    BHSM_Handle hHsm;

    BDBG_CASSERT( (int)NEXUS_BypassKeySlot_eG2GR == (int)BHSM_BypassKeySlot_eG2GR );
    BDBG_CASSERT( (int)NEXUS_BypassKeySlot_eGR2R == (int)BHSM_BypassKeySlot_eGR2R );
    BDBG_CASSERT( (int)NEXUS_BypassKeySlot_eGT2T == (int)BHSM_BypassKeySlot_eGT2T );
    BDBG_CASSERT( (int)NEXUS_BypassKeySlot_eMax  == (int)BHSM_BypassKeySlot_eInvalid );

    NEXUS_Security_GetHsm_priv( &hHsm );
    if( !hHsm ) { return BERR_TRACE( NEXUS_NOT_INITIALIZED ); }

    rc = BHSM_SetPidChannelBypassKeyslot( hHsm, NEXUS_PidChannel_GetIndex_isrsafe(pidChannel), bypassKeySlot );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE(NEXUS_UNKNOWN); }

    NEXUS_Security_LockTransport( true );
    NEXUS_PidChannel_SetBypassKeyslot_priv( pidChannel, bypassKeySlot != NEXUS_BypassKeySlot_eG2GR );
    NEXUS_Security_LockTransport( false );

    return NEXUS_SUCCESS;
}

void NEXUS_GetPidChannelBypassKeyslot( NEXUS_PidChannelHandle pidChannel, NEXUS_BypassKeySlot *pBypassKeySlot )
{
    NEXUS_Error rc = NEXUS_UNKNOWN;
    BHSM_Handle hHsm;

    BHSM_BypassKeySlot_e bypassKeySlot;

    if( pBypassKeySlot == NULL ) { BERR_TRACE( NEXUS_INVALID_PARAMETER ); return; }

    *pBypassKeySlot = NEXUS_BypassKeySlot_eMax;

    NEXUS_Security_GetHsm_priv( &hHsm );
    if( !hHsm ) { BERR_TRACE( NEXUS_NOT_INITIALIZED ); return; }

    rc = BHSM_GetPidChannelBypassKeyslot( hHsm, NEXUS_PidChannel_GetIndex_isrsafe(pidChannel), &bypassKeySlot );
    if( rc != BERR_SUCCESS ) { BERR_TRACE( rc ); return; }

    switch( bypassKeySlot )
    {
        case BHSM_BypassKeySlot_eG2GR: { *pBypassKeySlot = NEXUS_BypassKeySlot_eG2GR; break; }
        case BHSM_BypassKeySlot_eGR2R: { *pBypassKeySlot = NEXUS_BypassKeySlot_eGR2R; break; }
        case BHSM_BypassKeySlot_eGT2T: { *pBypassKeySlot = NEXUS_BypassKeySlot_eGT2T; break; }
        default: BERR_TRACE( NEXUS_UNKNOWN ); return;
    }

    return;
}
