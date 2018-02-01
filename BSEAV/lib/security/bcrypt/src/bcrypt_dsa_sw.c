/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#include <stdio.h>
#include "bstd.h"


#include <openssl/objects.h>

#include "bcrypt.h"
#include "bcrypt_dsa_sw.h"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>


#include <openssl/crypto.h>
#include <openssl/rand.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/bn.h>

#include <openssl/dsa.h>


/*static BIO *bio_err=NULL;*/



BDBG_MODULE( BCRYPT );

/*
 * This function converts bcrypt input key to OPENSSL DSA key
 */
void BCrypt_DSASetPubKey (DSA *dsaKey, BCRYPT_DSAKey_t *key)
{
    dsaKey->p = BN_bin2bn(key->p.pData, key->p.len, dsaKey->p);
    dsaKey->q = BN_bin2bn(key->q.pData, key->q.len, dsaKey->q);
    dsaKey->g = BN_bin2bn(key->g.pData, key->g.len, dsaKey->g);
    dsaKey->pub_key = BN_bin2bn(key->pubkey.pData, key->pubkey.len, dsaKey->pub_key);
}


void BCrypt_DSASetPrivKey (DSA *dsaKey, BCRYPT_DSAKey_t *key)
{

    dsaKey->p = BN_bin2bn(key->p.pData, key->p.len, dsaKey->p);
    dsaKey->q = BN_bin2bn(key->q.pData, key->q.len, dsaKey->q);
    dsaKey->g = BN_bin2bn(key->g.pData, key->g.len, dsaKey->g);
    dsaKey->priv_key = BN_bin2bn(key->privkey.pData, key->privkey.len, dsaKey->priv_key);
}

/*********************************************************************************/
/* DSA function                                                                  */
/*********************************************************************************/


BCRYPT_STATUS_eCode BCrypt_DSASw (    BCRYPT_Handle  hBcrypt,
                                    BCRYPT_S_DSASwParam_t *pInputParam     )
{
    BCRYPT_STATUS_eCode errCode = BCRYPT_STATUS_eOK;
    static const char rnd_seed[] = "string to make the random number generator think it has entropy";
    DSA *key = NULL;
    int outDataLen =0;

    int i;
    unsigned char inputsig[]= {
    0x30, 0x2c, 0x02, 0x14,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x02, 0x14,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    } ;

    BN_CTX *ctx;
#if !defined(USES_BORINGSSL)
    BIGNUM *kinv=NULL,*r=NULL,*k=NULL;
#endif


    if ((ctx=BN_CTX_new()) == NULL) goto myerr;
#if !defined(USES_BORINGSSL)
    if ((r=BN_new()) == NULL) goto myerr;
    kinv=NULL;
#endif


    BDBG_MSG(("Inside BCrypt_DSASw\n"));
    BDBG_ENTER(BCrypt_DSASw);
    BDBG_ASSERT( hBcrypt );
    BCRYPT_P_CHECK_ERR_CODE_CONDITION( errCode, BCRYPT_STATUS_eFAILED,
        (hBcrypt->ulMagicNumber != BCRYPT_P_HANDLE_MAGIC_NUMBER ) );

    RAND_seed(rnd_seed, sizeof rnd_seed); /* or OAEP may fail */


    key = DSA_new();



    if (pInputParam->bDSASign == true)
    {
        /* Changed by Alireza */

        /* This is for sign */

        BCrypt_DSASetPrivKey(key, pInputParam->key);

#if !defined(USES_BORINGSSL)
        k = BN_bin2bn(pInputParam->key->k.pData, pInputParam->key->k.len, k);


        /* Compute r = (g^k mod p) mod q */
        if (!BN_mod_exp_mont(r,key->g,k,key->p,ctx,
            (BN_MONT_CTX *)key->method_mont_p)) goto myerr;
        if (!BN_mod(r,r,key->q,ctx)) goto myerr;

        /* Compute  part of 's = inv(k) (m + xr) mod q' */
        if ((kinv=BN_mod_inverse(NULL,k,key->q,ctx)) == NULL) goto myerr;


        key->kinv= kinv;
        key->r=r;
#endif


        outDataLen = DSA_sign(0, pInputParam->pbDataIn, pInputParam->cbDataIn, pInputParam->sigout, &pInputParam->sigoutlen, key);



        if (outDataLen == 0)
            {
                printf("DSA sign fails !!!\n");
            errCode = BCRYPT_STATUS_eFAILED;
            }
        else
        {
            printf("DSA signature is generated\n");

            }

    }

    if (pInputParam->bDSASign == false)
    {
        /* Changed by Alireza */

        /* This is for verify */


        BCrypt_DSASetPubKey(key, pInputParam->key);

        /* Set up the signature */
        for(i=0; i<20; i++)
            inputsig[i+4] = pInputParam->sigin_r[i] ;
        for(i=0; i<20; i++)
            inputsig[i+26] = pInputParam->sigin_s[i] ;

        outDataLen = DSA_verify(0, pInputParam->pbDataIn, pInputParam->cbDataIn,
                                inputsig, sizeof(inputsig), key);


        if (outDataLen == 0)
            {
            printf("DSA verification fails !!!\n");
            errCode = BCRYPT_STATUS_eFAILED;
            }

        if (outDataLen == 1)
        {
            printf("DSA verification passed \n");

            }

        if (outDataLen == -1)
        {
            printf("DSA verification Error \n");
            errCode = BCRYPT_STATUS_eFAILED;
            }

    }


    goto BCRYPT_P_DONE_LABEL ;



myerr:

    printf("DSA_F_DSA_SIGN_SETUP,ERR_R_BN_LIB\n");

#if !defined(USES_BORINGSSL)
        if (kinv != NULL) BN_clear_free(kinv);
        if (r != NULL) BN_clear_free(r);
        if (k != NULL) BN_clear_free(k);
#endif

BCRYPT_P_DONE_LABEL:
    DSA_free(key);

    BDBG_LEAVE(BCrypt_DSASw);
    return( errCode );
}
