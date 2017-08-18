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
#include "nexus_hmac.h"


BDBG_MODULE(nexus_hmac);

struct NEXUS_Hmac
{
    NEXUS_OBJECT(NEXUS_Hmac);
    unsigned reserved;
};

NEXUS_HmacHandle NEXUS_Hmac_Create(void)
{
    NEXUS_HmacHandle handle = NULL;

    BDBG_ENTER(NEXUS_Hmac_Create);

    handle = BKNI_Malloc( sizeof(struct NEXUS_Hmac) );
    if( !handle ) { BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY); return NULL; }
    NEXUS_OBJECT_INIT(NEXUS_Hmac, handle);


    BDBG_LEAVE(NEXUS_Hmac_Create);
    return handle;
}

NEXUS_OBJECT_CLASS_MAKE( NEXUS_Hmac, NEXUS_Hmac_Destroy );

void NEXUS_Hmac_P_Finalizer( NEXUS_HmacHandle handle )
{
    BDBG_ENTER(NEXUS_Hmac_P_Finalizer);

    NEXUS_OBJECT_DESTROY(NEXUS_Hmac, handle);
    BKNI_Free( handle );


    BDBG_LEAVE(NEXUS_Hmac_P_Finalizer);
    return;
}

void NEXUS_Hmac_GetSettings( NEXUS_HmacHandle handle, NEXUS_HmacSettings *pSettings )
{
    BDBG_ENTER(NEXUS_Hmac_GetSettings);
    NEXUS_OBJECT_ASSERT( NEXUS_Hmac, handle );
    BSTD_UNUSED( handle );
    BSTD_UNUSED( pSettings );

    BDBG_LEAVE(NEXUS_Hmac_GetSettings);
    return;
}

NEXUS_Error NEXUS_Hmac_SetSettings( NEXUS_HmacHandle handle, const NEXUS_HmacSettings *pSettings )
{
    BDBG_ENTER(NEXUS_Hmac_SetSettings);
    NEXUS_OBJECT_ASSERT( NEXUS_Hmac, handle );
    BSTD_UNUSED( handle );
    BSTD_UNUSED( pSettings );

    BDBG_LEAVE(NEXUS_Hmac_SetSettings);
    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_Hmac_SubmitData( NEXUS_HmacHandle handle,
                                   NEXUS_Addr dataOffset,
                                   unsigned dataSize,
                                   bool last )
{
    BDBG_ENTER(NEXUS_Hmac_SubmitData);
    NEXUS_OBJECT_ASSERT( NEXUS_Hmac, handle );
    BSTD_UNUSED( handle );
    BSTD_UNUSED( dataOffset );
    BSTD_UNUSED( dataSize );
    BSTD_UNUSED( last );

    BDBG_LEAVE(NEXUS_Hmac_SubmitData);
    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_Hmac_GetResult( NEXUS_HmacHandle handle, NEXUS_HmacResult *pResult )
{
    BDBG_ENTER(NEXUS_Hmac_GetResult);
    NEXUS_OBJECT_ASSERT( NEXUS_Hmac, handle );
    BSTD_UNUSED( handle );
    BSTD_UNUSED( pResult );

    BDBG_LEAVE(NEXUS_Hmac_GetResult);
    return NEXUS_SUCCESS;
}
