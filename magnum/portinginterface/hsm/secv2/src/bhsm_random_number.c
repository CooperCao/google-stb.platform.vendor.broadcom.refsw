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
#include "bhsm.h"
#include "bkni.h"
#include "bhsm_priv.h"
#include "bhsm_p_crypto.h"
#include "bhsm_random_number.h"

BDBG_MODULE( BHSM );


BERR_Code BHSM_GetRandomNumber( BHSM_Handle hHsm,
                                BHSM_GetRandomNumberData *pRequest )
{
    BERR_Code rc = BERR_UNKNOWN;
    BHSM_P_CryptoRng bspConfig;
    unsigned itterations; /* # of times we'll call BSP for max length random number. */
    unsigned residual;    /* # of bytes required from last call that is a multiple of 4  */
    unsigned modulus;     /* # of bytes random number length is not a multiple of 4 */
    uint8_t  *pDest;

    BDBG_ENTER( BHSM_GetRandomNumber );

    if( !hHsm ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }
    BDBG_OBJECT_ASSERT( hHsm, BHSM_P_Handle );
    if( !pRequest ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }
    if( !pRequest->pRandomNumber ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }

    pDest = pRequest->pRandomNumber;

    itterations = pRequest->randomNumberSize / sizeof( bspConfig.out.randomNumber);
    modulus = pRequest->randomNumberSize % 4;
    residual = (pRequest->randomNumberSize % sizeof( bspConfig.out.randomNumber )) - modulus;

    if( itterations ) {

        BKNI_Memset( &bspConfig, 0, sizeof(bspConfig) );
        bspConfig.in.randomNumberLength = sizeof( bspConfig.out.randomNumber);

        do{
            bspConfig.out.randomNumberSize_inPlace = bspConfig.in.randomNumberLength;
            bspConfig.out.pRandomNumber_inPlace = pDest;

            rc = BHSM_P_Crypto_Rng( hHsm, &bspConfig );
            if( rc != BERR_SUCCESS ) { return BERR_TRACE(rc); }

            pDest += bspConfig.in.randomNumberLength;

        }while( --itterations );
    }

    if( residual ) {

        BKNI_Memset( &bspConfig, 0, sizeof(bspConfig) );
        bspConfig.in.randomNumberLength = residual;

        bspConfig.out.randomNumberSize_inPlace = bspConfig.in.randomNumberLength;
        bspConfig.out.pRandomNumber_inPlace = pDest;

        rc = BHSM_P_Crypto_Rng( hHsm, &bspConfig );
        if( rc != BERR_SUCCESS ) { return BERR_TRACE(rc); }

        pDest += bspConfig.in.randomNumberLength;
    }

    if( modulus ) {
        uint8_t buffer[4]; /* minimum size random number that BSP will return. */

        BKNI_Memset( &bspConfig, 0, sizeof(bspConfig) );
        bspConfig.in.randomNumberLength = sizeof(buffer);
        bspConfig.out.randomNumberSize_inPlace = bspConfig.in.randomNumberLength;
        bspConfig.out.pRandomNumber_inPlace = buffer;

        rc = BHSM_P_Crypto_Rng( hHsm, &bspConfig );
        if( rc != BERR_SUCCESS ) { return BERR_TRACE(rc); }

        BKNI_Memcpy( pDest, buffer, modulus );
    }

    BDBG_LEAVE( BHSM_GetRandomNumber );
    return rc;
}
