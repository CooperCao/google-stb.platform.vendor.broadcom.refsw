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
#include "bhsm.h"
#include "bhsm_hash.h"

BDBG_MODULE( BHSM );


BHSM_HashHandle BHSM_Hash_Create( BHSM_Handle hHsm )
{
    BDBG_ENTER( BHSM_Hash_Create );
    BSTD_UNUSED( hHsm );

    BDBG_LEAVE( BHSM_Hash_Create );
    return NULL;
}



void BHSM_Hash_Destroy( BHSM_HashHandle handle )
{
    BDBG_ENTER( BHSM_Hash_Destroy );
    BSTD_UNUSED( handle );


    BDBG_LEAVE( BHSM_Hash_Destroy );
    return;
}


void BHSM_Hash_GetSettings( BHSM_HashHandle handle, BHSM_HashSettings *pSettings )
{
    BDBG_ENTER( BHSM_Hash_GetSettings );
    BSTD_UNUSED( handle );
    BSTD_UNUSED( pSettings );


    BDBG_LEAVE( BHSM_Hash_GetSettings );
    return;
}


BERR_Code BHSM_Hash_SetSettings( BHSM_HashHandle handle, const BHSM_HashSettings *pSettings )
{
    BERR_Code rc = BERR_SUCCESS;

    BDBG_ENTER( BHSM_Hash_SetSettings );
    BSTD_UNUSED( handle );
    BSTD_UNUSED( pSettings );


    BDBG_LEAVE( BHSM_Hash_SetSettings );
    return rc;
}


BERR_Code BHSM_Hash_SubmitData( BHSM_HashHandle handle,
                                BSTD_DeviceOffset dataOffset,
                                unsigned dataSize,
                                bool last
                                )
{
    BERR_Code rc = BERR_SUCCESS;

    BDBG_ENTER( BHSM_Hash_SubmitData );
    BSTD_UNUSED( handle );
    BSTD_UNUSED( dataOffset );
    BSTD_UNUSED( dataSize );
    BSTD_UNUSED( last );


    BDBG_LEAVE( BHSM_Hash_SubmitData );
    return rc;
}


BERR_Code BHSM_Hash_GetResult( BHSM_HashHandle handle,
                               BHSM_HashResult *pResult /* [out] */ )
{
    BERR_Code rc = BERR_SUCCESS;

    BDBG_ENTER( BHSM_Hash_GetResult );
    BSTD_UNUSED( handle );
    BSTD_UNUSED( pResult );


    BDBG_LEAVE( BHSM_Hash_GetResult );
    return rc;
}
