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

#include "bstd.h"
#include "bhsm.h"
#include "bkni.h"
#include "bhsm_priv.h"
#include "bhsm_p_crypto.h"
#include "bhsm_random_number.h"

BDBG_MODULE( BHSM );


BERR_Code BHSM_GetRandomNumber( BHSM_Handle hHsm,
                                BHSM_GetRandomNumberData *pData )
{
    BERR_Code rc = BERR_SUCCESS;
    BHSM_P_CryptoRng bspConfig;
    unsigned itterations; /* # of time we'll call BSP for full load. */
    unsigned residual;    /* # of bytes required from last call */
    unsigned count = 0;
    uint8_t  *pDest;

    BDBG_ENTER( BHSM_GetRandomNumber );

    pDest = pData->pRandomNumber;

    BKNI_Memset( &bspConfig, 0, sizeof(bspConfig) );

    itterations = pData->randomNumberSize / sizeof( bspConfig.out.randomNumber);
    while( count++ < itterations )
    {
        bspConfig.in.randomNumberLength = sizeof( bspConfig.out.randomNumber);

        rc = BHSM_P_Crypto_Rng( hHsm, &bspConfig );
        if( rc != BERR_SUCCESS ) {BERR_TRACE(rc ); goto end; }

        BHSM_MemcpySwap( (void*)pDest, (void*)bspConfig.out.randomNumber, sizeof( bspConfig.out.randomNumber) );
        pDest += sizeof( bspConfig.out.randomNumber );
    }

    residual = pData->randomNumberSize % sizeof( bspConfig.out.randomNumber );
    if( residual )
    {
        bspConfig.in.randomNumberLength = residual;
        if(bspConfig.in.randomNumberLength & 0x3)
        {
            bspConfig.in.randomNumberLength = ((bspConfig.in.randomNumberLength + 4) & (~0x3));
        }

        rc = BHSM_P_Crypto_Rng( hHsm, &bspConfig );
        if( rc != BERR_SUCCESS ) {BERR_TRACE(rc ); goto end; }

        BHSM_Mem32cpy( (void*)bspConfig.out.randomNumber, (void*)bspConfig.out.randomNumber, bspConfig.in.randomNumberLength);
        BKNI_Memcpy(pDest, bspConfig.out.randomNumber, residual);
    }

end:

    BDBG_LEAVE( BHSM_GetRandomNumber );
    return rc;
}
