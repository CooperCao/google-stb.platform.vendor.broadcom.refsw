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
#include <stdlib.h>
#include "bstd.h"
#include <string.h>


#include <openssl/x509.h>
#include <openssl/err.h>
#include <openssl/engine.h>
#include <openssl/pem.h>


#include "bcrypt.h"
#include "bcrypt_x509_sw.h"
#include "bcrypt_rsa_sw.h"


BDBG_MODULE( BCRYPT );



#define CONVERT_LEN(x) ((x / 8) + (x % 8))

/*Local variables */
/*static X509* m_pCertificate = NULL; */ /*X509-internal pointer */




BCRYPT_STATUS_eCode BCrypt_x509ASN1DerDecode(BCRYPT_Handle  hBcrypt, const unsigned char*  x509Data, int nDataLen, X509** pCertificate)
{
    /*FILE* fp; */

    BCRYPT_STATUS_eCode res = BCRYPT_STATUS_eFAILED;
    X509* m_pCertificate = NULL;

    BSTD_UNUSED(hBcrypt);


    /*Convert from X509 DER to X509 internal (X509*)  */
    d2i_X509(&m_pCertificate,&x509Data, nDataLen);


    if (m_pCertificate)
    {

    /*The certificate has been correctly decoded: */

    res = BCRYPT_STATUS_eOK;

    /*X509_print_fp(stdout, m_pCertificate); */


    *pCertificate = m_pCertificate;

    }


    return res;
}

BCRYPT_STATUS_eCode BCrypt_x509GetDigestAlgorithm(BCRYPT_Handle  hBcrypt, X509* m_pCertificate, char* szAlgorithm, int len)
{
    BCRYPT_STATUS_eCode res = BCRYPT_STATUS_eFAILED;
#if (OPENSSL_VERSION_NUMBER > 0x10100000L)
    ASN1_BIT_STRING *psig;
    X509_ALGOR *palg;
#endif

    int i ;


    BSTD_UNUSED(hBcrypt);

    if (m_pCertificate)
    {
        BIO *mem = BIO_new(BIO_s_mem());
        char* p = NULL;
#if (OPENSSL_VERSION_NUMBER > 0x10100000L)
        X509_get0_signature((const ASN1_BIT_STRING**)&psig, (const X509_ALGOR**)&palg, (const X509 *)m_pCertificate);
        /*Transfer from integer to ASCII in a BIO object: */
        i2a_ASN1_OBJECT(mem, (const ASN1_OBJECT *)palg);
#else
        /*Transfer from integer to ASCII in a BIO object: */
        i2a_ASN1_OBJECT(mem, m_pCertificate->sig_alg->algorithm);
#endif
        /*Get a pointer to the data: */
        i=(int)BIO_ctrl(mem, BIO_CTRL_INFO, 0, (char *)&p);

        if (i > 0)
        {
            if (i < len)
            {
                strncpy(szAlgorithm, p, i);
                szAlgorithm[i] = '\0';
                res = BCRYPT_STATUS_eOK;
            }
            else
            {
                res = BCRYPT_STATUS_eFAILED;
            }
        }

        BIO_free(mem);

    }

    return res;
}

BCRYPT_STATUS_eCode BCrypt_x509GetRsaPublicKeyLen(BCRYPT_Handle  hBcrypt, X509* m_pCertificate, unsigned long* p_nLen, unsigned long* p_eLen)
{
    BCRYPT_STATUS_eCode res = BCRYPT_STATUS_eFAILED;

    EVP_PKEY *pkey=NULL;
#if (OPENSSL_VERSION_NUMBER > 0x10100000L)
    RSA * pRsaKey=NULL;
    BIGNUM *n=NULL,*e=NULL,*d=NULL;
#endif
    BSTD_UNUSED(hBcrypt);

    if (m_pCertificate)
    {
        pkey=X509_get_pubkey(m_pCertificate);


        if (pkey == NULL)
        {
            res = BCRYPT_STATUS_eFAILED;
        }
        else
        {
            res = BCRYPT_STATUS_eOK;
#if !defined(USES_BORINGSSL) && (OPENSSL_VERSION_NUMBER > 0x10100000L)
            if (EVP_PKEY_base_id(pkey) == EVP_PKEY_RSA)
            {
                pRsaKey = EVP_PKEY_get0_RSA(pkey);
                RSA_get0_key(pRsaKey, (const BIGNUM**)&n, (const BIGNUM**)&e, (const BIGNUM**)&d);
                *p_nLen = CONVERT_LEN(BN_num_bits(n));
                *p_eLen = CONVERT_LEN(BN_num_bits(e));
            }
#else
            if (pkey->type == EVP_PKEY_RSA)
            {
                *p_nLen = CONVERT_LEN(BN_num_bits(pkey->pkey.rsa->n));
                *p_eLen = CONVERT_LEN(BN_num_bits(pkey->pkey.rsa->e));
            }
#endif
            else
            {
                res = BCRYPT_STATUS_eFAILED;
            }
            EVP_PKEY_free(pkey);
        }

        if (res != BCRYPT_STATUS_eOK)
        {
            *p_nLen = 0;
            *p_eLen = 0;
        }
    }
    return res;

}



BCRYPT_STATUS_eCode BCrypt_x509GetRsaPublicKey(BCRYPT_Handle  hBcrypt, X509* m_pCertificate, BCRYPT_RSAKey_t * rsa_key)
{
    BCRYPT_STATUS_eCode res = BCRYPT_STATUS_eFAILED;

    EVP_PKEY *pkey=NULL;
#if (OPENSSL_VERSION_NUMBER > 0x10100000L)
    RSA * pRsaKey=NULL;
    BIGNUM *n=NULL,*e=NULL,*d=NULL;
#endif
    BDBG_MSG(("IN BCrypt_x509GetRsaPublicKey"));


    BSTD_UNUSED(hBcrypt);


    rsa_key->n.pData = NULL;
    rsa_key->e.pData = NULL;

    if (m_pCertificate)
    {
        pkey = X509_get_pubkey(m_pCertificate);

       /* VFS_LOG(NS_Utils::DEBUG3, "IN GetPublicKey"); */

        if (pkey == NULL)
        {
            res = BCRYPT_STATUS_eFAILED;
        }
        else
        {
#if !defined(USES_BORINGSSL) && (OPENSSL_VERSION_NUMBER > 0x10100000L)
            if (EVP_PKEY_base_id(pkey) == EVP_PKEY_RSA)
            {
                /*Allocate memory for modulus & exponent: */

                pRsaKey = EVP_PKEY_get0_RSA(pkey);
                RSA_get0_key(pRsaKey, (const BIGNUM**)&n, (const BIGNUM**)&e, (const BIGNUM**)&d);
                rsa_key->n.pData = (unsigned char*)malloc(CONVERT_LEN(BN_num_bits(n)));
                rsa_key->e.pData = (unsigned char*)malloc(CONVERT_LEN(BN_num_bits(e)));

                rsa_key->n.len = BN_bn2bin(n, rsa_key->n.pData);
                rsa_key->e.len = BN_bn2bin(e, rsa_key->e.pData);

                res = BCRYPT_STATUS_eOK;
            }
#else
            if (pkey->type == EVP_PKEY_RSA)
            {
                /*Allocate memory for modulus & exponent: */

                rsa_key->n.pData = (unsigned char*)malloc(CONVERT_LEN(BN_num_bits(pkey->pkey.rsa->n)));
                rsa_key->e.pData = (unsigned char*)malloc(CONVERT_LEN(BN_num_bits(pkey->pkey.rsa->e)));


                rsa_key->n.len = BN_bn2bin(pkey->pkey.rsa->n, rsa_key->n.pData);
                rsa_key->e.len = BN_bn2bin(pkey->pkey.rsa->e, rsa_key->e.pData);

                res = BCRYPT_STATUS_eOK;
            }
#endif
            else
            {
                BDBG_MSG(("ERROR, Public key is not RSA type"));
            }
            EVP_PKEY_free(pkey);

        }
    }
    BDBG_MSG(("OUT BCrypt_x509GetRsaPublicKey"));
    return res;

}



BCRYPT_STATUS_eCode BCrypt_RSAReadPrivateKeyPem(BCRYPT_Handle  hBcrypt, FILE* fp_privKeyIn, BCRYPT_RSAKey_t* rsa_key, long* p_nSize )
{
    BCRYPT_STATUS_eCode nRet = BCRYPT_STATUS_eFAILED;

    EVP_PKEY *pkey = NULL;
#if (OPENSSL_VERSION_NUMBER > 0x10100000L)
    RSA * pRsaKey=NULL;
    BIGNUM *n=NULL,*e=NULL,*d=NULL;
    BIGNUM *p=NULL,*q=NULL;
    BIGNUM *dmp1=NULL,*dmq1=NULL,*iqmp=NULL;
#endif
    BSTD_UNUSED(hBcrypt);


    rsa_key->n.pData    = NULL;
    rsa_key->e.pData    = NULL;
    rsa_key->d.pData    = NULL;
    rsa_key->p.pData    = NULL;
    rsa_key->q.pData    = NULL;
    rsa_key->dmp1.pData = NULL;
    rsa_key->dmq1.pData = NULL;
    rsa_key->iqmp.pData = NULL;

    if (fp_privKeyIn)
    {
       PEM_read_PrivateKey(fp_privKeyIn, &pkey, NULL, NULL);

        if (pkey)
        {
#if !defined(USES_BORINGSSL) && (OPENSSL_VERSION_NUMBER > 0x10100000L)
            if (EVP_PKEY_base_id(pkey) == EVP_PKEY_RSA)
            {
                pRsaKey = EVP_PKEY_get0_RSA(pkey);
                RSA_get0_key(pRsaKey, (const BIGNUM**)&n, (const BIGNUM**)&e, (const BIGNUM**)&d);
                RSA_get0_factors(pRsaKey, (const BIGNUM**)&p, (const BIGNUM**)&q);
                RSA_get0_crt_params(pRsaKey, (const BIGNUM**)&dmp1, (const BIGNUM**)&dmq1, (const BIGNUM**)&iqmp);
                /* Allocate memory for key: */
                rsa_key->n.pData    = (unsigned char*)malloc(CONVERT_LEN(BN_num_bits(n)));
                rsa_key->e.pData    = (unsigned char*)malloc(CONVERT_LEN(BN_num_bits(e)));
                rsa_key->d.pData    = (unsigned char*)malloc(CONVERT_LEN(BN_num_bits(d)));
                rsa_key->p.pData    = (unsigned char*)malloc(CONVERT_LEN(BN_num_bits(p)));
                rsa_key->q.pData    = (unsigned char*)malloc(CONVERT_LEN(BN_num_bits(q)));
                rsa_key->dmp1.pData = (unsigned char*)malloc(CONVERT_LEN(BN_num_bits(dmp1)));
                rsa_key->dmq1.pData = (unsigned char*)malloc(CONVERT_LEN(BN_num_bits(dmq1)));
                rsa_key->iqmp.pData = (unsigned char*)malloc(CONVERT_LEN(BN_num_bits(iqmp)));

                rsa_key->n.len      = BN_bn2bin(n, rsa_key->n.pData);
                rsa_key->e.len      = BN_bn2bin(e, rsa_key->e.pData);
                rsa_key->d.len      = BN_bn2bin(d, rsa_key->d.pData);
                rsa_key->p.len      = BN_bn2bin(p, rsa_key->p.pData);
                rsa_key->q.len      = BN_bn2bin(q, rsa_key->q.pData);
                rsa_key->dmp1.len   = BN_bn2bin(dmp1, rsa_key->dmp1.pData);
                rsa_key->dmq1.len   = BN_bn2bin(dmq1, rsa_key->dmq1.pData);
                rsa_key->iqmp.len   = BN_bn2bin(iqmp, rsa_key->iqmp.pData);

                *p_nSize = RSA_size(pRsaKey);

                nRet = BCRYPT_STATUS_eOK;


            }
#else
            if (pkey->type == EVP_PKEY_RSA)
            {
                /* Allocate memory for key: */
                rsa_key->n.pData    = (unsigned char*)malloc(CONVERT_LEN(BN_num_bits(pkey->pkey.rsa->n)));
                rsa_key->e.pData    = (unsigned char*)malloc(CONVERT_LEN(BN_num_bits(pkey->pkey.rsa->e)));
                rsa_key->d.pData    = (unsigned char*)malloc(CONVERT_LEN(BN_num_bits(pkey->pkey.rsa->d)));
                rsa_key->p.pData    = (unsigned char*)malloc(CONVERT_LEN(BN_num_bits(pkey->pkey.rsa->p)));
                rsa_key->q.pData    = (unsigned char*)malloc(CONVERT_LEN(BN_num_bits(pkey->pkey.rsa->q)));
                rsa_key->dmp1.pData = (unsigned char*)malloc(CONVERT_LEN(BN_num_bits(pkey->pkey.rsa->dmp1)));
                rsa_key->dmq1.pData = (unsigned char*)malloc(CONVERT_LEN(BN_num_bits(pkey->pkey.rsa->dmq1)));
                rsa_key->iqmp.pData = (unsigned char*)malloc(CONVERT_LEN(BN_num_bits(pkey->pkey.rsa->iqmp)));

                rsa_key->n.len      = BN_bn2bin(pkey->pkey.rsa->n, rsa_key->n.pData);
                rsa_key->e.len      = BN_bn2bin(pkey->pkey.rsa->e, rsa_key->e.pData);
                rsa_key->d.len      = BN_bn2bin(pkey->pkey.rsa->d,      rsa_key->d.pData);
                rsa_key->p.len      = BN_bn2bin(pkey->pkey.rsa->p,      rsa_key->p.pData);
                rsa_key->q.len      = BN_bn2bin(pkey->pkey.rsa->q,      rsa_key->q.pData);
                rsa_key->dmp1.len   = BN_bn2bin(pkey->pkey.rsa->dmp1,   rsa_key->dmp1.pData);
                rsa_key->dmq1.len   = BN_bn2bin(pkey->pkey.rsa->dmq1,   rsa_key->dmq1.pData);
                rsa_key->iqmp.len   = BN_bn2bin(pkey->pkey.rsa->iqmp,   rsa_key->iqmp.pData);

                *p_nSize = RSA_size(pkey->pkey.rsa);

                nRet = BCRYPT_STATUS_eOK;


            }
#endif
            else
            {
                BDBG_MSG(("ERROR, Public key is not RSA type"));
            }
            EVP_PKEY_free(pkey);
        }
        else
        {
            BDBG_MSG(("ERROR, no PKEY*"));
        }
    }
    return nRet;
}

void BCrypt_x509Free(BCRYPT_Handle  hBcrypt, X509* m_pCertificate)
{

    BSTD_UNUSED(hBcrypt);

    if (m_pCertificate)
    {
        X509_free(m_pCertificate);
        m_pCertificate = NULL;
    }
}

void BCrypt_privateKeyFree(BCRYPT_Handle  hBcrypt, BCRYPT_RSAKey_t* rsa_key)
{

    BSTD_UNUSED(hBcrypt);

    if (rsa_key->n.pData)
        free((void*)rsa_key->n.pData    );

    if (rsa_key->e.pData   )
        free((void*)rsa_key->e.pData    );

    if (rsa_key->d.pData   )
        free((void*)rsa_key->d.pData    );

    if (rsa_key->p.pData   )
        free((void*)rsa_key->p.pData    );

    if (rsa_key->q.pData   )
        free((void*)rsa_key->q.pData    );

    if (rsa_key->dmp1.pData)
        free((void*)rsa_key->dmp1.pData );

    if (rsa_key->dmq1.pData)
        free((void*)rsa_key->dmq1.pData );

    if (rsa_key->iqmp.pData)
        free((void*)rsa_key->iqmp.pData );
}

void BCrypt_publicKeyFree(BCRYPT_Handle  hBcrypt, BCRYPT_RSAKey_t* rsa_key)
{

    BSTD_UNUSED(hBcrypt);

    /*Free modulus: */
    if (rsa_key->n.pData){
        free((void*)rsa_key->n.pData    );
    }
    /*Free exponent: */
    if (rsa_key->e.pData){
        free((void*)rsa_key->e.pData    );
    }
}
