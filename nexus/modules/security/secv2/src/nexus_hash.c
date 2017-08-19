/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#include "bstd.h"
#include "nexus_security_module.h"
#include "nexus_base.h"
#include "nexus_types.h"
#include "nexus_security_common.h"
#include "nexus_hash.h"


BDBG_MODULE(nexus_hash);

struct NEXUS_Hash
{
    NEXUS_OBJECT(NEXUS_Hash);

    unsigned reserved;
};


NEXUS_HashHandle NEXUS_Hash_Create( void )
{
    NEXUS_HashHandle handle = NULL;

    BDBG_ENTER( NEXUS_Hash_Create );

    handle = BKNI_Malloc( sizeof(struct NEXUS_Hash) );
    if( !handle ) { BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY); return NULL; }

    NEXUS_OBJECT_INIT(NEXUS_Hash, handle);

    BDBG_LEAVE( NEXUS_Hash_Create );
    return handle;
}

NEXUS_OBJECT_CLASS_MAKE( NEXUS_Hash, NEXUS_Hash_Destroy );

static void NEXUS_Hash_P_Finalizer( NEXUS_HashHandle handle )
{
    BDBG_ENTER( NEXUS_Hash_P_Finalizer );

    NEXUS_OBJECT_DESTROY(NEXUS_Hash, handle);
    BKNI_Free( handle );

    BDBG_LEAVE( NEXUS_Hash_P_Finalizer );
    return;
}

void NEXUS_Hash_GetSettings( NEXUS_HashHandle handle, NEXUS_HashSettings *pSettings )
{

    BDBG_ENTER( NEXUS_Hash_GetSettings );
    BSTD_UNUSED( pSettings );

    NEXUS_OBJECT_ASSERT( NEXUS_Hash, handle );

    BDBG_LEAVE( NEXUS_Hash_GetSettings );
    return;
}


NEXUS_Error NEXUS_Hash_SetSettings( NEXUS_HashHandle handle, const NEXUS_HashSettings *pSettings )
{
    BDBG_ENTER( NEXUS_Hash_SetSettings );
    BSTD_UNUSED( pSettings );

    NEXUS_OBJECT_ASSERT( NEXUS_Hash, handle );

    BDBG_LEAVE( NEXUS_Hash_SetSettings );
    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_Hash_SubmitData( NEXUS_HashHandle handle,
                                  NEXUS_Addr dataOffset,
                                  unsigned dataSize,
                                  bool last )
{
    BDBG_ENTER( NEXUS_Hash_SubmitData );

    BSTD_UNUSED( dataOffset );
    BSTD_UNUSED( dataSize );
    BSTD_UNUSED( last );

    NEXUS_OBJECT_ASSERT( NEXUS_Hash, handle );

    BDBG_LEAVE( NEXUS_Hash_SubmitData );
    return NEXUS_SUCCESS;
}


NEXUS_Error NEXUS_Hash_GetResult( NEXUS_HashHandle handle, NEXUS_HashResult *pResult )
{
    BDBG_ENTER( NEXUS_Hash_GetResult );

    BSTD_UNUSED( pResult );

    NEXUS_OBJECT_ASSERT( NEXUS_Hash, handle );

    BDBG_LEAVE( NEXUS_Hash_GetResult );
    return NEXUS_SUCCESS;
}
