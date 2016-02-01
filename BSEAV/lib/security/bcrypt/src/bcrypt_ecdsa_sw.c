/***************************************************************************
 *     (c)2014 Broadcom Corporation
 *
 *  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 *
 **************************************************************************/

#include "bstd.h"
#include "stdio.h"
#include "string.h"

#if !defined(USES_BORINGSSL)
#include <openssl/opensslconf.h>
#endif
#include <openssl/ossl_typ.h>
#include <openssl/bio.h>
#include <openssl/bn.h>
#if defined(USES_BORINGSSL)
#include <openssl/mem.h>
#endif

#include "bcrypt.h"
#include "bcrypt_ecdsa_sw.h"
#include "bcrypt_ecc_sw.h"

BDBG_MODULE( BCRYPT );

/* #define ECDSA_TRACE */            /* define this to turn on debug statements */

/****************************************************************
* BCrypt_ECDSAVerifySw
*
* INPUTS:    hBcrypt - bcrypt handle
*            inoutp_ecdsaSwIO - pointer to input/output parameter structure
*
* OUTPUTS:    inoutp_ecdsaSwIO->sigIsValid -  true if signature is valid
*                                            false if signature is not valid
* RETURNS:    bcrypt error code
* FUNCTION: This function performs ECDSA signature verification.
*            The EC curve parameters are passed in by the caller,
*            along with the signature, hash value, and public key.
*
****************************************************************/
BCRYPT_STATUS_eCode BCrypt_ECDSAVerifySw(
        BCRYPT_Handle                 hBcrypt,
        BCRYPT_ECDSASw_Verify_t     *inoutp_ecdsaSwIO
)
{
    BCRYPT_STATUS_eCode errCode = BCRYPT_STATUS_eOK;
    BIGNUM    *p, *a, *b,*n, *e = NULL;    /* curve params, hash */
    BIGNUM    *r, *s, *rv = NULL;            /* signature */
    BIGNUM    *u1 = NULL, *u2 = NULL, *sInv = NULL;
    BN_CTX    *ctx = NULL;
    ECC        *ec = NULL;
    ECCpt    Rv,pt1,pt2;
    ECCpt    g, Qu;

#ifdef ECDSA_TRACE
    BIO    *out;
    char *buf;

    out = BIO_new_fp(stdout, BIO_NOCLOSE);
#endif

    BDBG_MSG(("Inside BCrypt_ECDSAVerifySw****************************************************\n"));
    BDBG_ENTER(BCrypt_ECDSAVerifySw);
    BDBG_ASSERT( hBcrypt );
    BCRYPT_P_CHECK_ERR_CODE_CONDITION( errCode, BCRYPT_STATUS_eFAILED,
        (hBcrypt->ulMagicNumber != BCRYPT_P_HANDLE_MAGIC_NUMBER ) );

    inoutp_ecdsaSwIO->sigIsValid = false;

    /*
     * EC curve parameters
     */
    p =BN_new();  a =BN_new();  b =BN_new();  n=BN_new();
    ECCpt_init(&g);
    BN_dec2bn (&p,         inoutp_ecdsaSwIO->eccParm.P);
    BN_dec2bn (&a,         inoutp_ecdsaSwIO->eccParm.A);
    BN_dec2bn (&b,         inoutp_ecdsaSwIO->eccParm.B);
    BN_dec2bn (&(g.x),    inoutp_ecdsaSwIO->eccParm.G_x);
    BN_dec2bn (&(g.y),     inoutp_ecdsaSwIO->eccParm.G_y);
    BN_dec2bn (&n,         inoutp_ecdsaSwIO->eccParm.N);
    ec = ECC_new_set(p,a,b,g,n);

    /*
     * Hash
     */
    /* e = BN_new(); do not need to allocate it is allocated by BN_bin2bn */
    e = BN_bin2bn(inoutp_ecdsaSwIO->hash, inoutp_ecdsaSwIO->hashLen, e); /* assume E=H here */

    /*
     * Signature
     */
    r = BN_new();
    s = BN_new();
    rv = BN_new();

    if (inoutp_ecdsaSwIO->sig.format == BCRYPT_ECCSig_Format_Decimal)
    {
        BN_dec2bn(&r, inoutp_ecdsaSwIO->sig.r);
        BN_dec2bn(&s, inoutp_ecdsaSwIO->sig.s);
    }
    else if (inoutp_ecdsaSwIO->sig.format == BCRYPT_ECCSig_Format_Hex)
    {
        BN_hex2bn(&r, inoutp_ecdsaSwIO->sig.r);
        BN_hex2bn(&s, inoutp_ecdsaSwIO->sig.s);
    }
    else
    {
        errCode = BCRYPT_STATUS_eFAILED;
        goto BCRYPT_P_DONE_LABEL;
    }

    /*
     * Public key
     */
    ECCpt_init(&Qu);
    BN_dec2bn(&Qu.x, inoutp_ecdsaSwIO->publicKey.x);
    BN_dec2bn(&Qu.y, inoutp_ecdsaSwIO->publicKey.y);

#ifdef ECDSA_TRACE
    printf("ec->p = ");
    BN_print(out, ec->modulus); printf("\n");
    printf("ec->a = "); BN_print(out, ec->a); printf("\n");
    printf("ec->b = ");  BN_print(out, ec->b); printf("\n");
    printf("ec->g.x = "); BN_print(out, ec->generator.x); printf("\n");
    printf("ec->g.y = "); BN_print(out, ec->generator.y); printf("\n");
    printf("ec->n = "); BN_print(out, ec->n); printf("\n");

    printf("Qu.xu = "); BN_print(out, Qu.x); printf("\n");
    buf=BN_bn2dec(Qu.x);
    printf("Qu.xu(dec) = %s\n",buf); free(buf);
    printf("Qu.yu = "); BN_print(out, Qu.y); printf("\n");
    buf=BN_bn2dec(Qu.y);
    printf("Qu.yu(dec) = %s\n",buf); free(buf);

    printf("ECDSA signature (r,s)\n");
    printf("ec_sig->r = "); BN_print(out, r); printf("\n");
    printf("ec_sig->s = "); BN_print(out, s); printf("\n");

    printf("Hash\n");
    printf("e = "); BN_print(out, e); printf("\n");
#endif

    if(BN_is_zero(Qu.x) && BN_is_zero(Qu.y)){
#ifdef ECDSA_TRACE
        printf("ERROR: incorrect signature\n");
#endif
        goto ExitFunc2;
    }

    u1=BN_new();
    u2=BN_new();
    /* sInv=BN_new(); Abbas fix mem leak should not allocate it will be allocated by BN_mod_inverse */
    ctx=BN_CTX_new();

    ECCpt_init(&Rv);
    ECCpt_init(&pt1);
    ECCpt_init(&pt2);

    sInv=BN_mod_inverse(NULL,s,n,ctx);        /* sInv */
    /* check sInv return value*/
    if (!sInv) {
#ifdef ECDSA_TRACE
        printf("ERROR: BN_MOD_inverse failed\n");
#endif
        errCode = BCRYPT_STATUS_eFAILED;
        goto BCRYPT_P_DONE_LABEL;
    }
    BN_mod_mul(u1,e,sInv,n,ctx);             /* u1=e*sinv mod n */
    BN_mod_mul(u2,r,sInv,n,ctx);             /* u2=r*sinv mod n */
    ECCpt_mul(&pt1,&g,u1,ec);                 /* pt1=u1*G */
    ECCpt_mul(&pt2,&Qu,u2,ec);                 /* pt2=u2*Qu */
    ECCpt_add(&Rv,&pt1,&pt2,ec);
    BN_mod(rv, Rv.x, n, ctx);

#ifdef ECDSA_TRACE
    buf=BN_bn2dec(u1);
    printf("u1(dec) = %s\n",buf); free(buf);
    buf=BN_bn2dec(u2);
    printf("u2(dec) = %s\n",buf); free(buf);
    buf=BN_bn2hex(Rv.x);
    printf("Rv.x(dec) = %s\n",buf); free(buf);
    buf=BN_bn2hex(Rv.y);
    printf("Rv.y(dec) = %s\n",buf); free(buf);
    printf("rv = "); BN_print(out, rv); printf("\n");
    printf("r = "); BN_print(out, r); printf("\n");

#endif
#if 0
    if (BN_cmp(Rv.x,r)==0)                    /* v=Rv.x r=R.x, compare v&r */
#endif
    if(BN_cmp(rv, r) == 0)
    {
#ifdef ECDSA_TRACE
        printf("correct signature\n");
#endif
        inoutp_ecdsaSwIO->sigIsValid = true;
    }
    else
    {
#ifdef ECDSA_TRACE
        printf("ERROR: incorrect signature\n");
#endif
        inoutp_ecdsaSwIO->sigIsValid = false;
    }

BCRYPT_P_DONE_LABEL:

    BN_CTX_free(ctx);
    ECCpt_free(&Rv);
    ECCpt_free(&pt2);
    ECCpt_free(&pt1);

    /*uninit ECDSA verify*/
    BN_free(u1);
    BN_free(u2);
    BN_free(sInv);
ExitFunc2:
    ECCpt_free(&Qu);

    BN_free(p);
    BN_free(a);
    BN_free(b);
    BN_free(n);
    BN_free(e);

    BN_free(r);
    BN_free(s);
    BN_free(rv);

    ECCpt_free(&g);
    ECC_free(ec);

#ifdef ECDSA_TRACE
    if (out != NULL)
        BIO_free(out);
#endif

    BDBG_LEAVE(BCrypt_ECDSAVerifySw);
    return( errCode );
}

/****************************************************************
* BCrypt_ECDSASignSw
*
* INPUTS:    hBcrypt - bcrypt handle
*            inoutp_ecdsaSwIO - pointer to input/output parameter structure
*
* OUTPUTS:    inoutp_ecdsaSwIO->sigIsValid -  true if signature is valid
*                                            false if signature is not valid
* RETURNS:    bcrypt error code
* FUNCTION: This function performs ECDSA signature signing.
*            The EC curve parameters are passed in by the caller,
*            along with the signature, hash value, and public key.
*
****************************************************************/
BCRYPT_STATUS_eCode BCrypt_ECDSASignSw(
        BCRYPT_Handle             hBcrypt,
        BCRYPT_ECDSASw_Sign_t     *inoutp_ecdsaSwIO
)
{
    BCRYPT_STATUS_eCode errCode = BCRYPT_STATUS_eOK;
    BIGNUM    *p, *a, *b,*n, *e = NULL;    /* curve params, hash */
    BIGNUM    *s = NULL, *r = NULL;        /* signature */
    BIGNUM    *du;                        /* private key */
    BIGNUM    *k;                            /* randomly chosen integer k */
    BIGNUM    *kinv = NULL, *xr = NULL;
    BN_CTX    *ctx = NULL;
    ECC        *ec = NULL;
    ECCpt    R;
    ECCpt    g;

#ifdef ECDSA_TRACE
    BIO    *out;

    out = BIO_new_fp(stdout, BIO_NOCLOSE);
#endif

    BDBG_MSG(("Inside BCrypt_ECDSASignSw\n"));
    BDBG_ENTER(BCrypt_ECDSASignSw);
    BDBG_ASSERT( hBcrypt );
    BCRYPT_P_CHECK_ERR_CODE_CONDITION( errCode, BCRYPT_STATUS_eFAILED,
        (hBcrypt->ulMagicNumber != BCRYPT_P_HANDLE_MAGIC_NUMBER ) );

    /*
     * EC curve parameters
     */
    p =BN_new();  a =BN_new();  b =BN_new();  n=BN_new();
    ECCpt_init(&g);

    BN_dec2bn (&p,         inoutp_ecdsaSwIO->eccParm.P);
    BN_dec2bn (&a,         inoutp_ecdsaSwIO->eccParm.A);
    BN_dec2bn (&b,         inoutp_ecdsaSwIO->eccParm.B);
    BN_dec2bn (&(g.x),    inoutp_ecdsaSwIO->eccParm.G_x);
    BN_dec2bn (&(g.y),     inoutp_ecdsaSwIO->eccParm.G_y);
    BN_dec2bn (&n,         inoutp_ecdsaSwIO->eccParm.N);
    ec = ECC_new_set(p,a,b,g,n);

    /*
     * Hash
     */
    /* e = BN_new(); do not need to allocate it is allocated by BN_bin2bn */
    e = BN_bin2bn(inoutp_ecdsaSwIO->hash, inoutp_ecdsaSwIO->hashLen, e); /* assume E=H here */

    /*
     * Private key
     */
    du=BN_new();
    BN_dec2bn(&du, inoutp_ecdsaSwIO->privateKey);

    /*
     * Randomly selected integer k within the range [1, n-1]
     */
    k=BN_new();
    BN_dec2bn(&k, inoutp_ecdsaSwIO->k);

#ifdef ECDSA_TRACE
    printf("ec->p = ");
    BN_print(out, ec->modulus); printf("\n");
    printf("ec->a = "); BN_print(out, ec->a); printf("\n");
    printf("ec->b = ");  BN_print(out, ec->b); printf("\n");
    printf("ec->g.x = "); BN_print(out, ec->generator.x); printf("\n");
    printf("ec->g.y = "); BN_print(out, ec->generator.y); printf("\n");
    printf("ec->n = "); BN_print(out, ec->n); printf("\n");

    printf("du = "); BN_print(out, du); printf("\n");
    printf("k = "); BN_print(out, k); printf("\n");
    printf("Hash e = "); BN_print(out, e); printf("\n");
#endif

    ECCpt_init(&R);
    ECCpt_mul(&R,&g,k,ec); /* r=R.x */

    /*Compute s=kinv*(e + r*du)mod n*/
    /* kinv=BN_new();  Abbas fix mem leak,  Do not need to allocate it will be allocated BN_mod_inverse(NULL,k,n,ctx); with NUll */

    xr=BN_new(); s=BN_new();r=BN_new();
    ctx=BN_CTX_new();
    kinv=BN_mod_inverse(NULL,k,n,ctx); /* kinv */
    /* check return value*/
    if (!kinv) {
#ifdef ECDSA_TRACE
        BDBG_ERR(("ERROR: BN_MOD_inverse failed\n"));
#endif
        errCode = BCRYPT_STATUS_eFAILED;
        goto BCRYPT_P_DONE_LABEL;
    }

    BN_mod_mul(xr,R.x,du,n,ctx); /* xr = r*du mod n */
    BN_add(s, xr, e); /* s = e + xr */
    if (BN_cmp(s,n) > 0)
      BN_sub(s,s,n);
    BN_mod_mul(s,s,kinv,n,ctx); /* s=s*kinv mod n */
    BN_mod(r, R.x, n, ctx);        /* r = R.x mod n */

#ifdef ECDSA_TRACE
    printf("ECDSA signature (r,s)\n");
    printf("R.x = "); BN_print(out, R.x); printf("\n");
    printf("R.x mod n = "); BN_print(out, r); printf("\n");
    printf("s = "); BN_print(out, s); printf("\n");
#endif

    if (inoutp_ecdsaSwIO->sig.format == BCRYPT_ECCSig_Format_Decimal)
    {
        /*inoutp_ecdsaSwIO->sig.r = BN_bn2dec(R.x); */
        inoutp_ecdsaSwIO->sig.r = BN_bn2dec(r);
        inoutp_ecdsaSwIO->sig.s = BN_bn2dec(s);
    }
    else if (inoutp_ecdsaSwIO->sig.format == BCRYPT_ECCSig_Format_Hex)
    {
        /*inoutp_ecdsaSwIO->sig.r = BN_bn2hex(R.x); */
        inoutp_ecdsaSwIO->sig.r = BN_bn2hex(r);
        inoutp_ecdsaSwIO->sig.s = BN_bn2hex(s);
    }
    else
    {
        errCode = BCRYPT_STATUS_eFAILED;
        goto BCRYPT_P_DONE_LABEL;
    }

BCRYPT_P_DONE_LABEL:

    /* uninit ECDSA sign */
    BN_free(p);
    BN_free(a);
    BN_free(b);
    BN_free(n);
    BN_free(e);

    BN_free(s);
    BN_free(r);
    BN_free(kinv);
    BN_free(xr);

    BN_free(du);
    BN_free(k);

    ECCpt_free(&R);
    ECCpt_free(&g);

    ECC_free(ec);
    BN_CTX_free(ctx);

#ifdef ECDSA_TRACE
    if (out != NULL)
        BIO_free(out);
#endif

    BDBG_LEAVE(BCrypt_ECDSASignSw);
    return( errCode );
}

/****************************************************************
* BCrypt_ECDSAMultiply
*
* INPUTS:    hBcrypt - bcrypt handle
*            inoutp_ecdsaSwIO - pointer to input/output parameter structure
*
* OUTPUTS:    inoutp_ecdsaSwIO->outPoint -  result of scalar multiplication
* RETURNS:    bcrypt error code
* FUNCTION: This function performs ECDSA scalar multiplication
*
****************************************************************/
BCRYPT_STATUS_eCode BCrypt_ECDSAMultiplySw(
        BCRYPT_Handle                 hBcrypt,
        BCRYPT_ECDSASw_Multiply_t     *inoutp_ecdsaSwIO
)
{
    BCRYPT_STATUS_eCode errCode = BCRYPT_STATUS_eOK;
    BIGNUM    *p, *a, *b,*n;            /* curve params */
    BIGNUM    *scalar;
    ECC        *ec = NULL;
    ECCpt    g;
    ECCpt    inpt, outpt;

#ifdef ECDSA_TRACE
    BIO    *out;

    out = BIO_new_fp(stdout, BIO_NOCLOSE);
#endif

    BDBG_MSG(("Inside BCrypt_ECDSAMultiplySw *****************************************\n"));
    BDBG_ENTER(BCrypt_ECDSAMultiplySw);
    BDBG_ASSERT( hBcrypt );
    BCRYPT_P_CHECK_ERR_CODE_CONDITION( errCode, BCRYPT_STATUS_eFAILED,
        (hBcrypt->ulMagicNumber != BCRYPT_P_HANDLE_MAGIC_NUMBER ) );

    /*
     * EC curve parameters
     */
    p =BN_new();  a =BN_new();  b =BN_new();  n=BN_new();
    scalar =BN_new();
    ECCpt_init(&g);

    BN_dec2bn (&p,         inoutp_ecdsaSwIO->eccParm.P);
    BN_dec2bn (&a,         inoutp_ecdsaSwIO->eccParm.A);
    BN_dec2bn (&b,         inoutp_ecdsaSwIO->eccParm.B);
    BN_dec2bn (&(g.x),    inoutp_ecdsaSwIO->eccParm.G_x);
    BN_dec2bn (&(g.y),     inoutp_ecdsaSwIO->eccParm.G_y);
    BN_dec2bn (&n,         inoutp_ecdsaSwIO->eccParm.N);
    ec = ECC_new_set(p,a,b,g,n);

    /*
     * Scalar value
     */
    BN_dec2bn (&scalar, inoutp_ecdsaSwIO->scalar);

    /*
     * Starting point
     */
    ECCpt_init(&inpt);
    ECCpt_init(&outpt);
    BN_dec2bn (&(inpt.x), inoutp_ecdsaSwIO->inPoint.x);
    BN_dec2bn (&(inpt.y), inoutp_ecdsaSwIO->inPoint.y);

#ifdef ECDSA_TRACE
    printf("ec->p = ");
    BN_print(out, ec->modulus); printf("\n");
    printf("ec->a = "); BN_print(out, ec->a); printf("\n");
    printf("ec->b = ");  BN_print(out, ec->b); printf("\n");
    printf("ec->g.x = "); BN_print(out, ec->generator.x); printf("\n");
    printf("ec->g.y = "); BN_print(out, ec->generator.y); printf("\n");
    printf("ec->n = "); BN_print(out, ec->n); printf("\n");

    printf("scalar = "); BN_print(out, scalar); printf("\n");

    printf("Input point:\n");
    printf("x = "); BN_print(out, inpt.x); printf("\n");
    printf("y = "); BN_print(out, inpt.y); printf("\n");
#endif

    /*
     * Perform the multiplication
     */
    ECCpt_mul(&outpt, &inpt, scalar, ec);

    /*
     * Result for caller
     */
    inoutp_ecdsaSwIO->outPoint.x = BN_bn2dec(outpt.x);
    inoutp_ecdsaSwIO->outPoint.y = BN_bn2dec(outpt.y);

#ifdef ECDSA_TRACE
    printf("Output point:\n");
    printf("x = "); BN_print(out, outpt.x); printf("\n");
    printf("y = "); BN_print(out, outpt.y); printf("\n");
#endif

BCRYPT_P_DONE_LABEL:

    /* uninit ECDSA multiply */
    BN_free(p);
    BN_free(a);
    BN_free(b);
    BN_free(n);

    BN_free(scalar);

    ECCpt_free(&g);
    ECCpt_free(&inpt);
    ECCpt_free(&outpt);
    ECC_free(ec);

#ifdef ECDSA_TRACE
    if (out != NULL)
        BIO_free(out);
#endif

    BDBG_LEAVE(BCrypt_ECDSAMultiplySw);
    return( errCode );
}


/****************************************************************
* BCrypt_ConvBinToStr
*
* INPUTS:    hBcrypt - bcrypt handle
*            data    - data to be converted
*            byteLen - length of data in bytes
*            format    - format of output string, decimal or hex
*            outStr    - output string
*
* OUTPUTS:    outStr    - output string
* RETURNS:    bcrypt error code
* FUNCTION: This function performs data conversion from binary to string
*
****************************************************************/
BCRYPT_STATUS_eCode BCrypt_ConvBinToStr(
        BCRYPT_Handle                 hBcrypt,    /* [in] bcrypt handle */
        unsigned char                 *data,        /* [in] pointer to data to be converted */
        unsigned long                byteLen,    /* [in] length of data in bytes */
        BCRYPT_String_Format_t        format,        /* [in] format of input string */
        char                        *outStr        /* [out] points to output string */
)
{
    BCRYPT_STATUS_eCode errCode = BCRYPT_STATUS_eOK;
    BIGNUM    *bnVal = NULL;
    char    *string = NULL;

    BDBG_ASSERT( hBcrypt );
    BCRYPT_P_CHECK_ERR_CODE_CONDITION( errCode, BCRYPT_STATUS_eFAILED,
        (hBcrypt->ulMagicNumber != BCRYPT_P_HANDLE_MAGIC_NUMBER ) );

    /*
     * First convert binary value to BN
     */
    /* bnVal = BN_new(); do not need to allocate it is allocated by BN_bin2bn */
    bnVal = BN_bin2bn(data, byteLen, bnVal);
    if (bnVal == NULL)
        return BCRYPT_STATUS_eFAILED;


    /*
     * Now convert BN to desired format string
     */
    string = (format == BCRYPT_String_Format_Decimal) ? BN_bn2dec (bnVal) : BN_bn2hex (bnVal);
    strncpy (outStr, string, strlen(string)+1);    /* copy output string for user */
    /* Added by Abbas to fix the memory leak */
    free(string);  /* Must OPEN SSL free string. */

BCRYPT_P_DONE_LABEL:
    if (bnVal)
        BN_free(bnVal);

    return errCode;
}

/****************************************************************
* BCrypt_ConvStrToBin
*
* INPUTS:    hBcrypt - bcrypt handle
*            inStr    - string to be converted
*            format    - format of string, decimal or hex
*            data    - point to buffer to store converted data
*            byteLen    - length of converted data
*
* OUTPUTS:    data    - output buffer
*            byteLen    - length of output
* RETURNS:    bcrypt error code
* FUNCTION: This function performs data conversion from string to binary
*
****************************************************************/
BCRYPT_STATUS_eCode BCrypt_ConvStrToBin(
        BCRYPT_Handle                 hBcrypt,    /* [in] bcrypt handle */
        char                        *inStr,        /* [in] points to input string */
        BCRYPT_String_Format_t        format,        /* [in] format of input string */
        unsigned char                 *data,        /* [out] pointer to binary data */
        unsigned long                *byteLen    /* [out] length of data in bytes */
)
{
    BCRYPT_STATUS_eCode errCode = BCRYPT_STATUS_eOK;
    BIGNUM    *bnVal = NULL;

    BDBG_ASSERT( hBcrypt );
    BCRYPT_P_CHECK_ERR_CODE_CONDITION( errCode, BCRYPT_STATUS_eFAILED,
        (hBcrypt->ulMagicNumber != BCRYPT_P_HANDLE_MAGIC_NUMBER ) );

    /*
     * First convert the string to BN
     */
    bnVal = BN_new();
    if (bnVal == NULL)
        return BCRYPT_STATUS_eFAILED;

    if (format == BCRYPT_String_Format_Decimal)
        BN_dec2bn(&bnVal, inStr);
    else
        BN_hex2bn(&bnVal, inStr);

    /*
     * Now convert BN to binary
     */
    *byteLen = BN_bn2bin (bnVal, data);

BCRYPT_P_DONE_LABEL:
    if (bnVal)
        BN_free(bnVal);

    return errCode;
}

/****************************************************************
* BCrypt_Sign_ECDSAClean
*
* INPUTS:    hBcrypt - bcrypt handle
*            inoutp_ecdsaSwIO - pointer to input/output parameter structure
*
* OUTPUTS:    None
*
* RETURNS:    bcrypt error code
* FUNCTION: This function cleans up the ECDSA paramter allocated by OpenSSL.
*
****************************************************************/

BCRYPT_STATUS_eCode BCrypt_Sign_ECDSAClean(
        BCRYPT_Handle                 hBcrypt,
        BCRYPT_ECDSASw_Sign_t         *inoutp_ecdsaSwIO
)
{
    BCRYPT_STATUS_eCode errCode = BCRYPT_STATUS_eOK;

    BDBG_MSG(("Inside BCrypt_Sign_ECDSAClean\n"));

    BDBG_ENTER(BCrypt_Sign_ECDSAClean);
    BDBG_ASSERT( hBcrypt );
    BCRYPT_P_CHECK_ERR_CODE_CONDITION( errCode, BCRYPT_STATUS_eFAILED,
        (hBcrypt->ulMagicNumber != BCRYPT_P_HANDLE_MAGIC_NUMBER ) );

    if ( inoutp_ecdsaSwIO->sig.r != NULL) free(inoutp_ecdsaSwIO->sig.r);
    if ( inoutp_ecdsaSwIO->sig.s != NULL) free(inoutp_ecdsaSwIO->sig.s);
BCRYPT_P_DONE_LABEL:
    BDBG_LEAVE(BCrypt_Sign_ECDSAClean);
    return( errCode );
}

/****************************************************************
* BCrypt_Multiply_ECDSAClean
*
* INPUTS:    hBcrypt - bcrypt handle
*            inoutp_ecdsaSwIO - pointer to input/output parameter structure
*
* OUTPUTS:    None
*
* RETURNS:    bcrypt error code
* FUNCTION: This function cleans up the ECDSA paramter allocated by OpenSSL.
*
****************************************************************/

BCRYPT_STATUS_eCode BCrypt_Multiply_ECDSAClean(
        BCRYPT_Handle                 hBcrypt,
        BCRYPT_ECDSASw_Multiply_t         *inoutp_ecdsaSwIO
)
{
    BCRYPT_STATUS_eCode errCode = BCRYPT_STATUS_eOK;

    BDBG_MSG(("Inside BCrypt_ECDSAClean\n"));

    BDBG_ENTER(BCrypt_Multiply_ECDSAClean);
    BDBG_ASSERT( hBcrypt );
    BCRYPT_P_CHECK_ERR_CODE_CONDITION( errCode, BCRYPT_STATUS_eFAILED,
        (hBcrypt->ulMagicNumber != BCRYPT_P_HANDLE_MAGIC_NUMBER ) );

    if ( inoutp_ecdsaSwIO->outPoint.x != NULL) free(inoutp_ecdsaSwIO->outPoint.x);
    if ( inoutp_ecdsaSwIO->outPoint.y != NULL) free(inoutp_ecdsaSwIO->outPoint.y);
BCRYPT_P_DONE_LABEL:
    BDBG_LEAVE(BCrypt_Multiply_ECDSAClean);
    return( errCode );
}
