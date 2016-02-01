/***************************************************************************
 *     Copyright (c) 2008 Broadcom Corporation
 *     All Rights Reserved
 *     No portions of this material may be reproduced in any form without the
 *     written permission of:
 *              Broadcom Corporation
 *              16251 Laguna Canyon Road
 *              Irvine, California  92618
 *     All information contained in this document is Broadcom Corporation
 *     company private, proprietary, and trade secret.
 *
 ****************************************************************************
 *
 *     Filename:  SecLib.c
 *     Creation Date: 06 September 2000 (v0.00)
 *     VSS Info:
 *         $Revision: 1.2 $
 *         $Date: 2002/10/16 16:56:02Z $
 *
 ****************************************************************************
 *     Description:
 *
 *       This file contains implementations of methods required by
 *       CablexChange Security purposes
 *
 *     Notes:
 *        OpenSSL package is used widely underneath.
 *
 ****************************************************************************/

/*
*****************************************************************************
* INCLUDE FILES
*****************************************************************************
*/

/* system includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


/* OpenSSL includes */
#include "crypto.h"
#include "dh.h"
#include "rand.h"
#include "asn1.h"
#include "x509.h"
#include "bio.h"
#include "x509v3.h"
#include "pem.h"
#include "rsa.h"
#include "bn.h"
#include "objects.h"
#include "apps.h"
#include "err.h"
#include "pem.h"
#include "sha.h"
#include "err.h"

/* library header file */
#include "seclib.h"



/*
*****************************************************************************
* STATIC DECLARATIONS
*****************************************************************************
*/

/* random seed */
static const char rnd_seed[] = "string to make the random number generator think it has entropy";
static int randLatch = 0;


/*
******************************************************************************
** FUNCTION:   secCryptoDhGener
**
** PURPOSE:    To retrieve Diffie-Hellman private and public key given the parameters
**
** PARAMETERS:
**    1. pPrime - pointer to prime number buffer
**    2. primeLen - length of prime number buffer in bytes
**    3. generator - DH generator
**    4. pPubKey - public key buffer, used an output parameter
**    5. pPubKeyLen - length of public key buffer in bytes
**    6. pPrivKey - private key buffer, used as an ouput
**    7. privKeyLen - length of private key buffer in bytes
**
**
** RETURNS:   Success or error code - unsigned long
**
** NOTE:
**
*****************************************************************************
*/
unsigned int secCryptoDhGener( unsigned char *pPrime,
      unsigned long primeLen,
      unsigned long generator,
      unsigned char **pPubKey,
      unsigned long *pubKeyLen,
      unsigned char **pPrivKey,
      unsigned long *privKeyLen )

{

   DH *dhKeyStruc;               /* Diffie-Hellman key data structure */
   unsigned int returnCode;      /* code to be returned */
   unsigned long size;           /* local varible to hold key sizes */


   if ( (!pPrime) || (primeLen < 1) || ((generator != 2) && (generator != 5)) )
   {
      return BAD_DATA;
   }

   /* seed the random number generator only once */
   if ( !randLatch )
   {
      RAND_seed( rnd_seed, sizeof rnd_seed );
      randLatch = 1;
   }

   /* allocate the space for DH key */
   dhKeyStruc = DH_new();

   /* assign the prime number field in DH key to prime input buffer */
   dhKeyStruc->p = BN_bin2bn( pPrime, primeLen, NULL );

   /* set the generator and check for error */
   dhKeyStruc->g=BN_new();

   if (!BN_set_word(dhKeyStruc->g,generator))
   {
      /* free memory */
      DH_free( dhKeyStruc );

      return GENERATOR_ERROR;
   }


   /* generate private and public key and check for error */
   returnCode = DH_generate_key( dhKeyStruc );

   if ( !returnCode )
   {
      /* free memory */
      DH_free( dhKeyStruc );

      return KEY_GEN_ERROR;
   }


   /* allocate memory for public key */
   size = BN_num_bytes( dhKeyStruc->pub_key );
   *pPubKey = (unsigned char*)malloc(size);

   /* put public key in output buffer */
   *pubKeyLen = BN_bn2bin( dhKeyStruc->pub_key, *pPubKey );

   /* allocate memory for private key */
   size = BN_num_bytes( dhKeyStruc->priv_key );
   *pPrivKey = (unsigned char*)malloc(size);

   /* put private key in ouput buffer */
   *privKeyLen = BN_bn2bin( dhKeyStruc->priv_key, *pPrivKey );

   /* free memory */
   DH_free( dhKeyStruc );

   /* return success */
   return OK;

}

/*
******************************************************************************
** FUNCTION:   secCryptoDhComputeSecret
**
** PURPOSE:    To compute and return Diffie-Hellman shared secret
**
** PARAMETERS:
**       1. pSecretKey - pointer to secret key buffer, output parameter
**       2. secretKeyLen - lenght of secret key buffer in bytes, output parameter
**       3. pPrivKey - private key buffer, used as an ouput
**       4. privKeyLen - length of private key buffer in bytes
**       5. pPubKeyOther - other party public key buffer
**       6. pubKeyOtherLen - length of other party public key in bytes
**       7. pPrime - pointer to prime number buffer
**       8. primeLen - length of prime number buffer in bytes
**       9. generator - DH generator
**
**
** RETURNS:   Success or error code - unsigned long
**
** NOTE:
**
*****************************************************************************
*/
unsigned int secCryptoDhComputeSecret( unsigned char **pSecretKey,
      unsigned long *secretKeyLen,
      unsigned char *pPrivKey,
      unsigned long privKeyLen,
      unsigned char *pPubKeyOther,
      unsigned long pubKeyOtherLen,
      unsigned char *pPrime,
      unsigned long primeLen,
      unsigned long generator )

{
   DH *dhKeyStruc;				/* Diffie-Hellman key structure */
   BIGNUM* pubKeyOtherBN;		/* big number representation of other party public key */
   unsigned int size;			/* local variable */

   if ( !pPrivKey || !pPubKeyOther || !pPrime || ( (generator != 2) && (generator != 5) ) )
   {
      return BAD_DATA;
   }

   /* seed the random number generator only once */
   if ( !randLatch )
   {
      RAND_seed( rnd_seed, sizeof rnd_seed );
      randLatch = 1;
   }


   /* allocate the space for DH key */
   dhKeyStruc = DH_new();

   /* assign the prime number field in DH key to prime input buffer */
   dhKeyStruc->p = BN_bin2bn( pPrime, primeLen, NULL );

   /* set the generator and check for error */
   dhKeyStruc->g=BN_new();

   if (!BN_set_word(dhKeyStruc->g,generator))
   {
      /* free memory */
      DH_free( dhKeyStruc );

      return GENERATOR_ERROR;
   }

   /* assign the private key field in DH key to private key input buffer */
   dhKeyStruc->priv_key = BN_bin2bn( pPrivKey, privKeyLen, NULL );

   /* convert otherPublicKey to big number representation */
   pubKeyOtherBN = BN_bin2bn( pPubKeyOther, pubKeyOtherLen, NULL );

   /* allocate memory for secret key */
   size = DH_size( dhKeyStruc );

   *pSecretKey = (unsigned char*)malloc(size);

   /* compute the secret key and check for error */
   if ( !(*secretKeyLen = DH_compute_key( *pSecretKey, pubKeyOtherBN, dhKeyStruc ) ) )
   {
      /* free memory */
      DH_free( dhKeyStruc );

      return KEY_GEN_ERROR;
   }

    /* free memory */
   DH_free( dhKeyStruc );
   BN_free( pubKeyOtherBN );
   /* return success */
   return OK;
}

/*
*****************************************************************************
** FUNCTION:   secCryptoBitStringEnc
**
** PURPOSE:    To get ASN1 bitstring representation of input buffer with given leength
**
** PARAMETERS:
**    1. pKeyBuffer - pointer to the input buffer, usually it is a key
**    2. KeyBufferLen - length of input buffer in bytes
**    3. ppBitStrBuf - output buffer, output parameter
**    4. bitStrBufLen - length of the ouput buffer, used as an output parameter
**
** RETURNS:   Success or error code - unsigned long
**
** NOTE:
**
******************************************************************************
*/


unsigned int secCryptoBitStringEnc( unsigned char *pKeyBuffer,
      unsigned long keyBufferLen,
      unsigned char **ppBitStrBuf,
      unsigned long *bitStrBufLen)
{
   ASN1_BIT_STRING *bitString;             /* OpenSSL ASN1 type */
   unsigned int bytesNumberForSize;    /* local variable, number of bytes needed for size */


   /* check input parameters */
   if ( !pKeyBuffer || !keyBufferLen )
   {
      return BAD_DATA;
   }

   /* create new bit string and check for errors */
   if ( !(bitString = ASN1_STRING_new()) )
   {
      return MEM_ALLOC_ERROR;
   }


   /* assign value to bit string and check for erorrs */
   if ( !(ASN1_STRING_set( bitString, pKeyBuffer, keyBufferLen )) )
   {
      /* free the ASN1 type */
      ASN1_STRING_free( bitString );

      return STRING_SET_ERROR;
   }

   /*
    * Get number of bytes for buffer size,
    * if it needs more than 3 bytes, consider
    * it as an error.
   */
   if ( keyBufferLen <= 0xff )
   {
      bytesNumberForSize = 1;
   }
   else if ( keyBufferLen <= 0xffff )
   {
      bytesNumberForSize = 2;
   }
   else if ( keyBufferLen <= 0xffffff)
   {
      bytesNumberForSize = 3;
   }
   else
   {
      /* free the ASN1 type */
      ASN1_STRING_free( bitString );

      return BAD_DATA;
   }

   /*
    * allocate space for output buffer: 1 for BIT STRING specifier,
    * 1 for padding byte, bytesNumberForSize for buffer size length,
    * and keyBufferLen-1 for size of input buffer
   */
   *ppBitStrBuf = (unsigned char*)malloc( 1 + 1 + bytesNumberForSize + keyBufferLen-1 );

   /* encode and put bit string into output buffer */
   *bitStrBufLen = i2d_ASN1_BIT_STRING( bitString, ppBitStrBuf );

   /* update pointer to the beggining of the buffer */
   *ppBitStrBuf = *ppBitStrBuf - *bitStrBufLen;

   /* free the ASN1 type */
   ASN1_STRING_free( bitString );
   /* if here return success */
   return OK;
}



/*
******************************************************************************
** FUNCTION:   secCryptoPkcs5KeyGener
**
** PURPOSE:    To compute key according to the PKCS5 Password-Based Cryptography Standard
**
** PARAMETERS:
**    1. pPassword - pointer to password buffer
**    2. passwordLen - length of password buffer in bytes
**    3. pSalt - salt buffer
**    4. saltLen - length of salt buffer
**    5. iteration - number of iterations
**    6. keyBufferLen - length of output key buffer, input parameter
**    7. pKeyBuffer - output key buffer
**
**
** RETURNS:   Success or error code - unsigned long
**
** NOTE:
**
******************************************************************************
*/
int secCryptoPkcs5KeyGener( const char *pPassword,
      int passwordLen,
      unsigned char *pSalt,
      int saltLen,
      int iteration,
      int keyBufferLen,
      unsigned char *pKeyBuffer )
{

   /* only keyLengths in interval [1,20] are accepted - SHA1 digest length is 20 */
   if ( !( passwordLen && saltLen) || ( keyBufferLen > SHA_DIGEST_LENGTH ) ||
         ( keyBufferLen < 0 ) || (pKeyBuffer == NULL) )
   {
      return BAD_DATA;
   }

   /* call to corresponding method in OpenSSL */
   PKCS5_PBKDF2_HMAC_SHA1( pPassword, passwordLen, pSalt, saltLen, iteration,
                              keyBufferLen, pKeyBuffer );
   return OK;
}



