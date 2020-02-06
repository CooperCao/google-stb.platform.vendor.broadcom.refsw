/******************************************************************************
 *  Copyright (C) 2019 Broadcom.
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

#include <stdio.h>
#include <string.h>
#include "bstd.h"

#include <openssl/bn.h>
#include <openssl/rsa.h>
#include <openssl/x509.h>
#include <openssl/rand.h>

#include <openssl/objects.h>

#include "bcrypt.h"
#include "bcrypt_rsa_sw.h"
#include "bcrypt_asn1_sw.h"

#if 0 /* Commenting out to prevent compiler warning (because it's static and never called). */
static BCRYPT_STATUS_eCode BCrypt_RSA_CRT_CalculatePrivateExponent(RSA *rsaKey, BCRYPT_RSAKey_t *bcryptKey);
#endif
BDBG_MODULE( BCRYPT );

/*
 * This function converts bcrypt input key to OPENSSL RSA key
 */
void BCrypt_RSASetKey_4CRT (RSA *rsaKey, BCRYPT_RSAKey_t *key)
{
#if (OPENSSL_VERSION_NUMBER > 0x10100000L)
    BIGNUM *n=NULL,*e=NULL,*d=NULL;
    BIGNUM *p=NULL,*q=NULL;
    BIGNUM *dmp1=NULL,*dmq1=NULL,*iqmp=NULL;

    RSA_get0_key(rsaKey, (const BIGNUM **)&n, (const BIGNUM **)&e, (const BIGNUM **)&d);
    RSA_get0_factors(rsaKey, (const BIGNUM **)&p, (const BIGNUM **)&q);
    RSA_get0_crt_params(rsaKey, (const BIGNUM **)&dmp1, (const BIGNUM **)&dmq1, (const BIGNUM **)&iqmp);

    n = BN_bin2bn(key->n.pData, key->n.len, n);
    e = BN_bin2bn(key->e.pData, key->e.len, e);
    d = BN_bin2bn(key->d.pData, key->d.len, d);
    p = BN_bin2bn(key->p.pData, key->p.len, p);
    q = BN_bin2bn(key->q.pData, key->q.len, q);
    dmp1 = BN_bin2bn(key->dmp1.pData, key->dmp1.len, dmp1);
    dmq1 = BN_bin2bn(key->dmq1.pData, key->dmq1.len, dmq1);
    iqmp = BN_bin2bn(key->iqmp.pData, key->iqmp.len, iqmp);

    RSA_set0_key(rsaKey, n, e, d);
    RSA_set0_factors(rsaKey, p, q);
    RSA_set0_crt_params(rsaKey, dmp1, dmq1, iqmp);
#else
    rsaKey->n = BN_bin2bn(key->n.pData, key->n.len, rsaKey->n);
    rsaKey->e = BN_bin2bn(key->e.pData, key->e.len, rsaKey->e);
    rsaKey->d = BN_bin2bn(key->d.pData, key->d.len, rsaKey->d);
    rsaKey->p = BN_bin2bn(key->p.pData, key->p.len, rsaKey->p);
    rsaKey->q = BN_bin2bn(key->q.pData, key->q.len, rsaKey->q);
    rsaKey->dmp1 = BN_bin2bn(key->dmp1.pData, key->dmp1.len, rsaKey->dmp1);
    rsaKey->dmq1 = BN_bin2bn(key->dmq1.pData, key->dmq1.len, rsaKey->dmq1);
    rsaKey->iqmp = BN_bin2bn(key->iqmp.pData, key->iqmp.len, rsaKey->iqmp);
#endif
}


void BCrypt_RSASetPrivKey(RSA *rsaKey, BCRYPT_RSAKey_t *key)
{
#if (OPENSSL_VERSION_NUMBER > 0x10100000L)
    BIGNUM *n=NULL,*e=NULL,*d=NULL;
    RSA_get0_key(rsaKey, (const BIGNUM **)&n, (const BIGNUM **)&e, (const BIGNUM **)&d);
    n = BN_bin2bn(key->n.pData, key->n.len, n);
    e = BN_bin2bn(key->e.pData, key->e.len, e);
    d = BN_bin2bn(key->d.pData, key->d.len, d);
    RSA_set0_key(rsaKey, n, e, d);
#else
    rsaKey->n = BN_bin2bn(key->n.pData, key->n.len, rsaKey->n);
    rsaKey->e = BN_bin2bn(key->e.pData, key->e.len, rsaKey->e);
    rsaKey->d = BN_bin2bn(key->d.pData, key->d.len, rsaKey->d);
#endif
}


/*
 * This function sets up the key for encoding.  We only need modulus and public exponent
 */
void BCrypt_RSASetEncodeKey (RSA *rsaKey, BCRYPT_RSAKey_t *key)
{
#if (OPENSSL_VERSION_NUMBER > 0x10100000L)
    BIGNUM *n=NULL,*e=NULL,*d=NULL;
    RSA_get0_key(rsaKey, (const BIGNUM **)&n, (const BIGNUM **)&e, (const BIGNUM **)&d);
    n = BN_bin2bn(key->n.pData, key->n.len, n);
    e = BN_bin2bn(key->e.pData, key->e.len, e);
    RSA_set0_key(rsaKey, n, e, d);
#else
    rsaKey->n = BN_bin2bn(key->n.pData, key->n.len, rsaKey->n);
    rsaKey->e = BN_bin2bn(key->e.pData, key->e.len, rsaKey->e);
#endif
}
BCRYPT_STATUS_eCode BCrypt_GetRSA_From_SubjectPublicKeyInfo(BCRYPT_Handle  hBcrypt,
                                    uint8_t *pBuf, uint32_t len,
                                    BCRYPT_RSAKey_t *pBcrypt_rsaSw)
{
    RSA * RSA_PubKey;
    uint32_t lengthDecoded;
    const unsigned char  *pDecoded;
    BCRYPT_STATUS_eCode res = BCRYPT_STATUS_eOK;
#if (OPENSSL_VERSION_NUMBER > 0x10100000L)
    BIGNUM *n=NULL,*e=NULL,*d=NULL;
#endif

    BDBG_ASSERT(hBcrypt);
    BDBG_ASSERT(pBuf != NULL);
    BDBG_ASSERT(pBcrypt_rsaSw != NULL);
    BDBG_ASSERT(len != 0);


    res = BCrypt_ASN1DerDecode(hBcrypt, (const unsigned char **)&pBuf, len, (unsigned char **)&pDecoded,&lengthDecoded);
    if(res != BCRYPT_STATUS_eOK)
    {
        BDBG_ERR(("%s:Error in ASN1 parsing",BSTD_FUNCTION));
        goto ErrorExit;
    }

    RSA_PubKey =  d2i_RSA_PUBKEY(NULL, &pDecoded, lengthDecoded);
#if (OPENSSL_VERSION_NUMBER > 0x10100000L)
    RSA_get0_key(RSA_PubKey, (const BIGNUM **)&n, (const BIGNUM **)&e, (const BIGNUM **)&d);

    pBcrypt_rsaSw->n.len= BN_bn2bin(n, pBcrypt_rsaSw->n.pData );
    pBcrypt_rsaSw->e.len = BN_bn2bin(e, pBcrypt_rsaSw->e.pData);
#else
    pBcrypt_rsaSw->n.len= BN_bn2bin(RSA_PubKey->n, pBcrypt_rsaSw->n.pData );
    pBcrypt_rsaSw->e.len = BN_bn2bin(RSA_PubKey->e, pBcrypt_rsaSw->e.pData);
#endif

ErrorExit:
    return res;
}

#define PKCS8_OK        0
#define PKCS8_NO_OCTET      1
#define PKCS8_EMBEDDED_PARAM    2
#define PKCS8_NS_DB     3
#define PKCS8_NEG_PRIVKEY   4
/*********************************************************************************/
/* RSA function                                                                  */
/*********************************************************************************/
BCRYPT_STATUS_eCode BCrypt_GetRSA_From_PrivateKeyInfo(BCRYPT_Handle  hBcrypt,
                                                      uint8_t *pBuf, uint32_t len,
                                                      BCRYPT_RSAKey_t *pBcrypt_rsaSw)
{
    BCRYPT_STATUS_eCode errCode = BCRYPT_STATUS_eOK;
    RSA *priv_rsa = NULL;
    PKCS8_PRIV_KEY_INFO *p8;
    X509_ALGOR *a;
    const unsigned char *p;
    int pkeylen;
#if (OPENSSL_VERSION_NUMBER > 0x10100000L)
    BIGNUM *n=NULL,*e=NULL,*d=NULL;
    ASN1_OBJECT *algoid;
#endif

    BDBG_ASSERT( hBcrypt );
    BDBG_ASSERT(pBuf != NULL);
    BDBG_ASSERT(pBcrypt_rsaSw != NULL);
    BDBG_ASSERT(len != 0);
    (void)hBcrypt;

    p8 = d2i_PKCS8_PRIV_KEY_INFO(NULL, (const unsigned char **)&pBuf, (long) len);
#if (OPENSSL_VERSION_NUMBER > 0x10100000L)
    if (!PKCS8_pkey_get0((const ASN1_OBJECT **)&algoid, &p, &pkeylen,
                          (const X509_ALGOR**)&a, (const PKCS8_PRIV_KEY_INFO*)p8))
    {
        BDBG_ERR(("%s %d d2i_RSAPrivateKey() failed ",BSTD_FUNCTION, __LINE__));
        errCode = BCRYPT_STATUS_eFAILED;
        goto ErrorExit;
    }
#else

    if(p8->pkey->type == V_ASN1_OCTET_STRING)
    {
        p8->broken = PKCS8_OK;
        p = p8->pkey->value.octet_string->data;
        pkeylen = p8->pkey->value.octet_string->length;
    }
    else
    {
        p8->broken = PKCS8_NO_OCTET;
        p = p8->pkey->value.sequence->data;
        pkeylen = p8->pkey->value.sequence->length;
    }

    a = p8->pkeyalg;
#endif
    switch (OBJ_obj2nid(a->algorithm))
    {
        case NID_rsaEncryption:
        if (!(priv_rsa = d2i_RSAPrivateKey (NULL, &p, pkeylen)))
        {
            BDBG_ERR(("%s %d d2i_RSAPrivateKey() failed ",BSTD_FUNCTION, __LINE__));
            errCode = BCRYPT_STATUS_eFAILED;
            goto ErrorExit;
        }
        break;
        default:
            BDBG_ERR(("%s:Invalid algorithm",BSTD_FUNCTION));
            errCode = BCRYPT_STATUS_eFAILED;
            goto ErrorExit;
        break;
    }
#if (OPENSSL_VERSION_NUMBER > 0x10100000L)
    RSA_get0_key(priv_rsa, (const BIGNUM **)&n, (const BIGNUM **)&e, (const BIGNUM **)&d);
    BDBG_MSG(("%s %d n %p e %p d %p data: n %p e %p d %p\n",
        BSTD_FUNCTION, __LINE__,
        (void *)n,(void *)e,(void *)d,
        (void *)pBcrypt_rsaSw->n.pData, (void *)pBcrypt_rsaSw->e.pData, (void *)pBcrypt_rsaSw->d.pData));

    pBcrypt_rsaSw->n.len = BN_bn2bin(n, pBcrypt_rsaSw->n.pData );
    pBcrypt_rsaSw->e.len  = BN_bn2bin(e,pBcrypt_rsaSw->e.pData );
    pBcrypt_rsaSw->d.len  = BN_bn2bin(d,pBcrypt_rsaSw->d.pData );
#else
    BDBG_MSG(("%s %d n %p e %p d %p data: n %p e %p d %p\n",
        BSTD_FUNCTION, __LINE__,
        (void *)priv_rsa->n,(void *)priv_rsa->e,(void *)priv_rsa->d,
        (void *)pBcrypt_rsaSw->n.pData, (void *)pBcrypt_rsaSw->e.pData, (void *)pBcrypt_rsaSw->d.pData));

    pBcrypt_rsaSw->n.len = BN_bn2bin(priv_rsa->n, pBcrypt_rsaSw->n.pData );
    pBcrypt_rsaSw->e.len  = BN_bn2bin(priv_rsa->e,pBcrypt_rsaSw->e.pData );
    pBcrypt_rsaSw->d.len  = BN_bn2bin(priv_rsa->d,pBcrypt_rsaSw->d.pData );
#endif

ErrorExit:
    return errCode;
}

BCRYPT_STATUS_eCode BCrypt_RSASw (  BCRYPT_Handle  hBcrypt,
                                    BCRYPT_S_RSASwParam_t *pInputParam  )
{
    BCRYPT_STATUS_eCode errCode = BCRYPT_STATUS_eOK;
    static const char rnd_seed[] = "string to make the random number generator think it has entropy";
    RSA *key = NULL;
    int outDataLen;

    BDBG_MSG(("%s - Entered function", BSTD_FUNCTION));
    BDBG_ENTER(BCrypt_RSASw);
    BDBG_ASSERT( hBcrypt );
    BCRYPT_P_CHECK_ERR_CODE_CONDITION( errCode, BCRYPT_STATUS_eFAILED, (hBcrypt->ulMagicNumber != BCRYPT_P_HANDLE_MAGIC_NUMBER ) );

    RAND_seed(rnd_seed, sizeof rnd_seed); /* or OAEP may fail */

    key = RSA_new();

    if (pInputParam->bRSAop == rsasign)
    {
        BDBG_MSG(("%s - RSA sign operation specified", BSTD_FUNCTION));

        BCrypt_RSASetPrivKey(key, pInputParam->key);

        if (strcmp(LN_sha1WithRSAEncryption, (const char*) pInputParam->psAlgorithmId) == 0)
        {
             BDBG_MSG(("%s - regular sign + sha1 with encryption", BSTD_FUNCTION));
             outDataLen = RSA_sign(NID_sha1WithRSAEncryption, pInputParam->pbDataIn, pInputParam->cbDataIn,
             pInputParam->pbDataOut, (unsigned int*) pInputParam->cbDataOut, key);
        }
        else
        {
                 if (strcmp("RSA-SHA256", (const char*) pInputParam->psAlgorithmId) == 0)
                 {
                    BDBG_MSG(("%s - regular sign + sha256 with encryption", BSTD_FUNCTION));
                    outDataLen = RSA_sign(NID_sha256, pInputParam->pbDataIn, pInputParam->cbDataIn,
                                    pInputParam->pbDataOut, (unsigned int*) pInputParam->cbDataOut, key);
                 }
                 else
                 {
                   BDBG_MSG(("%s - regular sign + sha1 only", BSTD_FUNCTION));
                   outDataLen = RSA_sign(NID_sha1, pInputParam->pbDataIn, pInputParam->cbDataIn,
                   pInputParam->pbDataOut, (unsigned int*) pInputParam->cbDataOut, key);
                 }
           }

        if (outDataLen == 0)
        {
               BDBG_ERR(("%s - RSA sign operation failed !!!", BSTD_FUNCTION));
            errCode = BCRYPT_STATUS_eFAILED;
        }

    }
    else if (pInputParam->bRSAop == rsasign_crt)
    {
        BDBG_MSG(("%s - RSA sign operation specified (CRT)", BSTD_FUNCTION));

        BCrypt_RSASetKey_4CRT(key, pInputParam->key);

           if (strcmp(LN_sha1WithRSAEncryption, (const char*) pInputParam->psAlgorithmId) == 0)
        {
               BDBG_MSG(("%s - CRT sign + sha1 with encryption", BSTD_FUNCTION));
            outDataLen = RSA_sign(NID_sha1WithRSAEncryption, pInputParam->pbDataIn, pInputParam->cbDataIn,
                                    pInputParam->pbDataOut, (unsigned int*) pInputParam->cbDataOut, key);

        }
        else
        {
            BDBG_MSG(("%s - CRT sign + sha1 only", BSTD_FUNCTION));
            outDataLen = RSA_sign(NID_sha1, pInputParam->pbDataIn, pInputParam->cbDataIn,
                              pInputParam->pbDataOut, (unsigned int*) pInputParam->cbDataOut, key);
        }

        if (outDataLen == 0)
        {
            BDBG_ERR(("%s - RSA sign operation (CRT) failed !!!", BSTD_FUNCTION));
            errCode = BCRYPT_STATUS_eFAILED;
        }
    }
    else if (pInputParam->bRSAop == rsaverify)
    {
        BDBG_MSG(("%s - RSA verify operation specified", BSTD_FUNCTION));

        BCrypt_RSASetEncodeKey(key, pInputParam->key);

        if (strcmp(LN_sha1WithRSAEncryption, (const char*) pInputParam->psAlgorithmId) == 0)
        {
                outDataLen = RSA_verify(NID_sha1WithRSAEncryption, pInputParam->pbDataIn, pInputParam->cbDataIn,
                                pInputParam->pbDataOut, *pInputParam->cbDataOut, key);
        }
        else
        {
            if (strcmp("RSA-SHA256", (const char*) pInputParam->psAlgorithmId) == 0)
            {
                BDBG_MSG(("%s - sha256 verification only", BSTD_FUNCTION));
                outDataLen = RSA_verify(NID_sha256, pInputParam->pbDataIn, pInputParam->cbDataIn, pInputParam->pbDataOut, *pInputParam->cbDataOut, key);
            }
            else
            {
                BDBG_MSG(("%s - sha1 verification only", BSTD_FUNCTION));
                outDataLen = RSA_verify(NID_sha1, pInputParam->pbDataIn, pInputParam->cbDataIn,    pInputParam->pbDataOut, *pInputParam->cbDataOut, key);
            }
           }

        if (outDataLen == 0)
        {
            BDBG_ERR(("%s - RSA verification operation FAILED !!!", BSTD_FUNCTION));
            errCode = BCRYPT_STATUS_eFAILED;
        }
        else
        {
            BDBG_MSG(("%s - RSA verification operation SUCCESS !!!", BSTD_FUNCTION));
        }
    }
    else if (pInputParam->bRSAop == rsaenc)
    {
        BDBG_MSG(("%s - RSA encryption operation specified", BSTD_FUNCTION));

        BCrypt_RSASetEncodeKey(key, pInputParam->key);
        outDataLen = RSA_public_encrypt(pInputParam->cbDataIn, pInputParam->pbDataIn,
                                        pInputParam->pbDataOut, key, pInputParam->bRSAPadType /*RSA_PKCS1_PADDING*/);
        if (outDataLen == -1)
        {
            BDBG_ERR(("%s - RSA encryption operation failed !!!", BSTD_FUNCTION));
            errCode = BCRYPT_STATUS_eFAILED;
        }
        else
        {
            *(pInputParam->cbDataOut) = (unsigned long)outDataLen;
        }
    }
    else if (pInputParam->bRSAop == rsadec)
    {
        BDBG_MSG(("%s - RSA decryption operation specified", BSTD_FUNCTION));
        BCrypt_RSASetPrivKey(key, pInputParam->key);

        /* Perform RSA data decryption */

        outDataLen = RSA_private_decrypt(pInputParam->cbDataIn, pInputParam->pbDataIn,
                                        pInputParam->pbDataOut, key,pInputParam->bRSAPadType /* RSA_PKCS1_PADDING*/ /*RSA_PKCS1_OAEP_PADDING :vgd:commented for adobe . need to clean this up*/);
        if (outDataLen == -1)
        {
            BDBG_ERR(("%s - RSA decryption operation failed !!!", BSTD_FUNCTION));
            errCode = BCRYPT_STATUS_eFAILED;
        }
        else
        {
            *(pInputParam->cbDataOut) = (unsigned long)outDataLen;
        }
    }
    else if (pInputParam->bRSAop == rsadec_crt)
    {
        BCrypt_RSASetKey_4CRT(key, pInputParam->key);

        /* Perform RSA data decryption with CRT*/

        outDataLen = RSA_private_decrypt(pInputParam->cbDataIn, pInputParam->pbDataIn,
                                        pInputParam->pbDataOut, key, RSA_PKCS1_OAEP_PADDING);
        if (outDataLen == -1)
        {
            BDBG_ERR(("%s - RSA decryption failed (crt) !!!", BSTD_FUNCTION));
            errCode = BCRYPT_STATUS_eFAILED;
        }
        else
        {
            *(pInputParam->cbDataOut) = (unsigned long)outDataLen;
        }
    }
    else
    {
        BDBG_ERR(("%s - Operation not supported (%u)", BSTD_FUNCTION, pInputParam->bRSAop));
        errCode = BCRYPT_STATUS_eFAILED;
    }


BCRYPT_P_DONE_LABEL:
    if(key != NULL){
        RSA_free(key);
    }

    BDBG_MSG(("%s - Exiting function", BSTD_FUNCTION));
    return( errCode );
}


#if 0 /* Commenting out to prevent compiler warning (because it's static and never called). */
/*
 * d = 0x10001exp(-1) * (mod ((P-1) * (Q-1))).
 * */
BCRYPT_STATUS_eCode BCrypt_RSA_CRT_CalculatePrivateExponent(RSA *rsaKey, BCRYPT_RSAKey_t *bcryptKey)
{
    BCRYPT_STATUS_eCode errCode = BCRYPT_STATUS_eOK;
    BIGNUM * result_p_minus_1 = NULL ;
    BIGNUM * result_q_minus_1 = NULL ;
    BIGNUM * mod_arg = NULL;
    BN_CTX *ctx = NULL;

    BDBG_MSG(("%s - Entered function", BSTD_FUNCTION));

    if(bcryptKey->p.pData == NULL || bcryptKey->p.len == 0)
    {
        BDBG_ERR(("%s - Error, 'p' not provided", BSTD_FUNCTION));
        errCode = BCRYPT_STATUS_eFAILED;
        goto ErrorExit;
    }

    if(bcryptKey->q.pData == NULL || bcryptKey->q.len == 0)
    {
        BDBG_ERR(("%s - Error, 'q' not provided", BSTD_FUNCTION));
        errCode = BCRYPT_STATUS_eFAILED;
        goto ErrorExit;
    }

    if(bcryptKey->e.pData == NULL || bcryptKey->e.len == 0)
    {
        BDBG_ERR(("%s - Error, 'e' not provided", BSTD_FUNCTION));
        errCode = BCRYPT_STATUS_eFAILED;
        goto ErrorExit;
    }

    if ((ctx = BN_CTX_new()) == NULL)
    {
        BDBG_ERR(("%s - Error creating context", BSTD_FUNCTION));
        errCode = BCRYPT_STATUS_eFAILED;
        goto ErrorExit;
    }

    result_p_minus_1 = BN_new() ;
    result_q_minus_1 = BN_new() ;
    mod_arg = BN_new();

    rsaKey->e = BN_bin2bn(bcryptKey->e.pData, bcryptKey->e.len, rsaKey->e);
    rsaKey->p = BN_bin2bn(bcryptKey->p.pData, bcryptKey->p.len, rsaKey->p);
    rsaKey->q = BN_bin2bn(bcryptKey->q.pData, bcryptKey->q.len, rsaKey->q);

    if(BN_sub(result_p_minus_1, rsaKey->p, BN_value_one()) == 0)
    {
        errCode = BCRYPT_STATUS_eFAILED;
        BDBG_ERR(("%s - Error subtracting", BSTD_FUNCTION));
        goto ErrorExit;
    }

    BN_sub(result_q_minus_1, rsaKey->q, BN_value_one());

    if(BN_mul(mod_arg, result_p_minus_1, result_q_minus_1, ctx) == 0)
    {
        errCode = BCRYPT_STATUS_eFAILED;
        BDBG_ERR(("%s - Error multiplying", BSTD_FUNCTION));
        goto ErrorExit;
    }

    BN_mod_inverse(rsaKey->d, rsaKey->e, mod_arg, ctx);

ErrorExit:
    /*
     * FREE EVERYTHING
     * */
    if(ctx != NULL){
        BN_CTX_free(ctx);
    }
    BN_free(result_p_minus_1);
    BN_free(result_q_minus_1);
    BN_free(mod_arg);
    BDBG_MSG(("%s -  Exiting function", BSTD_FUNCTION));
    return errCode;
}
#endif
