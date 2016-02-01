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
 *         $Date: 2002/10/16 16:56:01Z $
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

//static int appsInd = 0;                /* trigger for apps_startup in validate function */
static unsigned char *pErrBuf1 = NULL; /* error buffer to be loaded in callback function */
static int errBufLength;               /* length of the error buffer in validate function */
static int errIndicator = 0;           /* return code for validate function - assigned in callback function */


/* callback function used in cert verification */
static int MS_CALLBACK cb(int ok, X509_STORE_CTX *ctx);
/* decode method for certificate issues */
static X509* decodeDERCert( unsigned char *pCertBuf, unsigned int bufferLength );

static int v_verbose=0;



/*
*****************************************************************************
**
**  Function Name: secCertGetPublicKey
**
**  PURPOSE:
**     Retrieve public key and its length. Key buffer contains key itself in HEX
**
**  PARAMETERS:
**      1. pCertBuf - pointer to the certicate buffer.
**      2. pCertBufLen - length of the certificate buffer
**      3. key - pointer to the internal repr. of RSA key, used as an output parameter
**      4. pPubKeyId - key identifier
**
**  RETURNS:
**     succes or error code - unsigned long
**
**  NOTES:
**
*****************************************************************************
*/
unsigned long secCertGetPublicKey( unsigned char *pCertBuf,
      unsigned int pCertBufLen,
      RSA_key *key,
      unsigned int *pPubKeyId )
{
   X509 *x509p;           /* certificate */
   EVP_PKEY *publicKeyp;  /* key envelope */
   char temp[500];               /* local helper */


   x509p = decodeDERCert( pCertBuf, pCertBufLen );

   if ( !x509p )
   {
      return MEM_ALLOC_ERROR;
   }

   /* is key there ? */
   if ( !(publicKeyp = X509_get_pubkey( x509p )) )
   {
      return BAD_DATA;
   }

   /* check for key type */

   if ( publicKeyp->type == EVP_PKEY_RSA )
   {
      /* key ID */
      *pPubKeyId = EVP_PKEY_RSA;

      /* key length in bits */
      key->keyLenBits = BN_num_bits(publicKeyp->pkey.rsa->n);

      /* get the public key in binary */
      key->modBufLen = BN_bn2bin( publicKeyp->pkey.rsa->n, temp );
      key->mod = (unsigned char*)malloc( key->modBufLen );
      memcpy( key->mod, temp, key->modBufLen );

      key->expBufLen = BN_bn2bin( publicKeyp->pkey.rsa->e, temp );
      key->exp = (unsigned char*)malloc( key->expBufLen );
      memcpy( key->exp, temp, key->expBufLen );

      /* free the certificate storage */
      X509_free( x509p );

      EVP_PKEY_free( publicKeyp );

      ERR_remove_state( 0 );

      return OK;
   }

#if 0
   else if ( publicKeyp->type == EVP_PKEY_DSA )
   {
      /* key ID */
      *pPubKeyId = EVP_PKEY_DSA;

      /* key length in bits */
      *pPubKeyLenBits = BN_num_bits(publicKeyp->pkey.dsa->p);

      /* get the public key in binary */
      bytesWritten = BN_bn2bin( publicKeyp->pkey.dsa->priv_key, temp );

      /* check for input buffer length */
      if ( *pPubKeyBufLen <= bytesWritten )
      {
         /* buffer too short, return error */
         return BUF_SHORT;
      }

      /* buffer is ok, update the length and copy the key to the proper place */
      *pPubKeyBufLen = bytesWritten;
      memcpy( pPubKeyBuf, temp, *pPubKeyBufLen );

      /* free the certificate storage */
      X509_free( x509p );
      EVP_PKEY_free( publicKeyp );
      ERR_remove_state( 0 );

      return OK;
   }
#endif

   /* free the certificate storage */
   X509_free( x509p );
   EVP_PKEY_free( publicKeyp );
   ERR_remove_state( 0 );
   /* we have two types of keys defined RSA and DSA, others won't do */
   return BAD_DATA;
}



/*
*****************************************************************************
**
**  Function Name: secCertValidate
**
**  PURPOSE:
**     Verify certificate against provided CA certificate.
**
**  PARAMETERS:
**      1. pCertBuf - pointer to the certicate buffer.
**      2. pCertBufLen - length of the certificate buffer
**      3. pCertCALen - length of the CA certificate buffer
**      4. pCertCA -  pointer to CA certificate buffer
**      5. pErrBuf - pointer to the error buffer, used as an output parameter
**      6. errBufLen - length of error buffer
**
**  RETURNS:
**     succes or error code - unsigned long
**
**  NOTES:
**
*****************************************************************************
*/
unsigned long secCertValidate( unsigned char *pCertBuf,
      unsigned int pCertBufLen,
      unsigned int pCertCALen,
      unsigned char *pCertCA,
      unsigned char *pErrBuf,
      int errBufLen )
{

	X509_STORE *cert_ctx;
	X509_LOOKUP *lookup;
   X509 *x509p;
   X509 *x509CA;
   X509_STORE_CTX *csc;
   unsigned int currentCertLen, totalLen = 0;
   unsigned int temp;

   x509p = decodeDERCert( pCertBuf, pCertBufLen );

   if ( !x509p )
   {
      return MEM_ALLOC_ERROR;
   }

   /* set static to point to output buffer */
   pErrBuf1 = pErrBuf;

   /* assign value to static varible */
   errBufLength = errBufLen;

   /* this piece of code is stripped down from OpenSSL apps/verify.c file */
   cert_ctx=X509_STORE_new();

   if (cert_ctx == NULL) return BAD_DATA;

	X509_STORE_set_verify_cb_func(cert_ctx,cb);

	//ERR_load_crypto_strings();


   apps_startup();
   /* run apps_startup only once - controlled by static variable appsInd */
   /*if ( !appsInd )
   {
      apps_startup();
      appsInd = 1;
   }*/

   lookup=X509_STORE_add_lookup(cert_ctx,X509_LOOKUP_file() );

	if (lookup == NULL) return BAD_DATA;

   /* start parsing out the CA buffer */
   while (1)
   {
      if ( *(pCertCA+1) == 0x82 )
      {
         temp = ( *(pCertCA+2) << 8 ) | ( *(pCertCA+3) );
         currentCertLen = temp + 4;
      }
      else
      {
         temp = *(pCertCA+2);
         currentCertLen = temp +3;
      }

      totalLen +=currentCertLen;

      if ( totalLen > pCertCALen )
      {
         return BAD_DATA;
      }

      x509CA = decodeDERCert( pCertCA, currentCertLen );

      if ( !x509CA )
      {
         return MEM_ALLOC_ERROR;
      }

      /* add CA cert to lookup */
      X509_STORE_add_cert( lookup->store_ctx, x509CA );

      /* free the certificate storage */
      X509_free( x509CA );

      pCertCA = pCertCA+currentCertLen;

      if ( totalLen == pCertCALen )
      {
         break;
      }

   }

	ERR_clear_error();

   csc = X509_STORE_CTX_new();

   /* add certificate to be verified */
   X509_STORE_CTX_init(csc,cert_ctx,x509p,NULL);

   /* set purpose */
   X509_STORE_CTX_set_purpose(csc, 7);

   /* verify the certificate */
   X509_verify_cert(csc);

   /* free dynamic memory */
   X509_STORE_CTX_free(csc);

	/* return success or failure */
   if (cert_ctx != NULL) X509_STORE_free(cert_ctx);

   /* free the certificate storage */
   X509_free( x509p );

   /* clean all ciphers&digests set by apps_startup() */
   EVP_cleanup();

   ERR_remove_state( 0 );
   /* errIndicator contains appropriate return code - assigned in callback function */
   return errIndicator;


}

/*
******************************************************************************
**  Function Name: secCertGetSerNum
**
**  PURPOSE:
**     Get certificate serial number
**
**  PARAMETERS:
**      1. pCertBuf - pointer to the certicate buffer.
**      2. pCertBufLen - length of the certificate buffer
**      3. pSerialNumber -  certificate serial number, used as an output parameter
**
**  RETURNS:
**     succes or error code - unsigned long
**
**  NOTES:
**
*****************************************************************************
*/
unsigned long secCertGetSerNum( unsigned char *pCertBuf,
      unsigned int pCertBufLen,
      unsigned long *pSerialNumber )
{
    X509 *x509p; /* certificate */


    x509p = decodeDERCert( pCertBuf, pCertBufLen );

    if ( !x509p )
    {
       return MEM_ALLOC_ERROR;
    }

    /* serial number is valid ? */
    if ( !(&x509p->cert_info->serialNumber) )
    {
       return BAD_DATA;
    }

    /* copy serial number value to the input buffer and return succes */
    *pSerialNumber = (unsigned long)ASN1_INTEGER_get(*(&x509p->cert_info->serialNumber));

    /* free the certificate storage */
    X509_free( x509p );

    ERR_remove_state( 0 );
    return OK;
}


/*
******************************************************************************
**  Function Name: secCertGetSigAlgId
**
**  PURPOSE:
**     Get certificate signature algorithm ID
**
**  PARAMETERS:
**      1. pCertBuf - pointer to the certicate buffer.
**      2. pCertBufLen - length of the certificate buffer
**      3. pAlgId -  certificate signature algorithm identifier, an output parameter
**
**  RETURNS:
**     succes or error code - unsigned long
**
**  NOTES:
**
******************************************************************************
*/
unsigned long secCertGetSigAlgId( unsigned char *pCertBuf, unsigned int pCertBufLen, unsigned long *pAlgId )
{
   X509 *x509p;  /* certificate */


   x509p = decodeDERCert( pCertBuf, pCertBufLen );

   if ( !x509p )
   {
      return MEM_ALLOC_ERROR;
   }

   /* signature alg id exists ? */
   if ( !(&x509p->cert_info->signature->algorithm) )
   {
      return BAD_DATA;
   }

   /* get the sig ID, copy it to the buffer and return success */
   *pAlgId = OBJ_obj2nid(*(&x509p->cert_info->signature->algorithm));

   /* free the certificate storage */
   X509_free( x509p );

   ERR_remove_state( 0 );
   return OK;
}


/*
*****************************************************************************
**  Function Name: secCertGetNotAfterDate
**
**  PURPOSE:
**      Get certificate not after date
**
**  PARAMETERS:
**      1. pCertBuf - pointer to the certicate buffer.
**      2. pCertBufLen - length of the certificate buffer
**      3. pNotAfterBuf - pointer to the not after date buffer, output paramete
**      4. pNotAfterBufLen - length of not after date buffer, I/O parameter
**
**  RETURNS:
**     succes or error code - unsigned long
**
**  NOTES:
**
******************************************************************************
*/
unsigned long secCertGetNotAfterDate( unsigned char *pCertBuf,
      unsigned int pCertBufLen,
      unsigned char *pNotAfterBuf,
      unsigned int *pNotAfterBufLen )
{
   X509 *x509p;  /* certificate */
   unsigned int temp;   /* local helper */



   x509p = decodeDERCert( pCertBuf, pCertBufLen );

   if ( !x509p )
   {
      return MEM_ALLOC_ERROR;
   }

   /* time is there ? */
   if ( !(&x509p->cert_info->validity->notAfter->data) )
   {
      return BAD_DATA;
   }


   /* get the length of time buffer */
   temp = strlen(*(&x509p->cert_info->validity->notAfter->data));

   /* check for input buffer length */
   if ( *pNotAfterBufLen < temp )
   {
      /* length is too short, return error */
      return BUF_SHORT;
   }

   /* copy the info to the buffers and return success */
   strncpy( pNotAfterBuf, *(&x509p->cert_info->validity->notAfter->data), temp );
   *pNotAfterBufLen = temp;

   /* free the certificate storage */
   X509_free( x509p );

   ERR_remove_state( 0 );
   return OK;
}


/*
*****************************************************************************
**  Function Name: secCertGetNotBeforeDate
**
**  PURPOSE:
**      Get certificate not before date
**
**  PARAMETERS:
**      1. pCertBuf - pointer to the certicate buffer.
**      2. pCertBufLen - length of the certificate buffer
**      3. pNotAfterBuf - pointer to the not before date buffer, output parameter
**      4. pNotAfterBufLen - length of not before date buffer, I/O parameter
**
**  RETURNS:
**     succes or error code - unsigned long
**
**  NOTES:
**
******************************************************************************
*/
unsigned long secCertGetNotBeforeDate( unsigned char *pCertBuf,
      unsigned int pCertBufLen,
      unsigned char *pNotBeforeBuf,
      unsigned int *pNotBeforeBufLen )
{
   X509 *x509p;  /* certificate */
   unsigned int temp;   /* local helper */


   x509p = decodeDERCert( pCertBuf, pCertBufLen );

   if ( !x509p )
   {
      return MEM_ALLOC_ERROR;
   }

   /* time is there ? */
   if ( !(&x509p->cert_info->validity->notBefore->data) )
   {
      return BAD_DATA;
   }

   /* get the length of time buffer */
   temp = strlen(*(&x509p->cert_info->validity->notBefore->data));


   /* check for input buffer length */
   if ( *pNotBeforeBufLen < temp )
   {
      /* length is too short, return error */
      return BUF_SHORT;
   }

   /* copy the info to the buffers and return success */
   strncpy( pNotBeforeBuf, *(&x509p->cert_info->validity->notBefore->data), temp );
   *pNotBeforeBufLen = temp;

   /* free the certificate storage */
   X509_free( x509p );

   ERR_remove_state( 0 );
   return OK;
}

/*
*****************************************************************************
**  Function Name: secCertGetSubjectName
**
**  PURPOSE:
**      Get certificate subject name as a printable string
**
**  PARAMETERS:
**      1. pCertBuf - pointer to the certicate buffer.
**      2. pCertBufLen - length of the certificate buffer
**      3. pSubjName - pointer to the subject name buffer, output parameter
**      4. subjNameLen - length of the subject name buffer, output parameter
**
**  RETURNS:
**     succes or error code - unsigned long
**
**  NOTES:
**
******************************************************************************
*/
unsigned long secCertGetSubjectName( unsigned char *pCertBuf,
      unsigned long pCertBufLen,
      unsigned char **pSubjName,
      unsigned long *subjNameLen )
{
   X509 *x509p;        /* certificate */
   char buf[256];             /* buffer to store name */


   /* decode cert from DER format */
   x509p = decodeDERCert( pCertBuf, pCertBufLen );

   if ( !x509p )
   {
      return MEM_ALLOC_ERROR;
   }

   /* subject name exists ? */
   if ( !x509p->cert_info->subject )
   {
      return BAD_DATA;
   }

   /* get the subject name */
   if ( !X509_NAME_oneline( X509_get_subject_name( x509p ), buf, 256 ) )
   {
      return BAD_DATA;
   }

   /* get the size of the buffer */
   *subjNameLen = strlen( buf );

   /* allocate space for output buffer */
   *pSubjName = (unsigned char*)malloc( *subjNameLen );

   /* copy the subject name to the buffer */
   memcpy( *pSubjName, buf, *subjNameLen );

   /* free the certificate storage */
   X509_free( x509p );

   ERR_remove_state( 0 );
   /* return success */
   return OK;
   }

/*
*****************************************************************************
**  Function Name: secCertGetSubjectNameOS
**
**  PURPOSE:
**      Get certificate subject name ASN1 encoded as an octet string
**
**  PARAMETERS:
**      1. pCertBuf - pointer to the certicate buffer.
**      2. pCertBufLen - length of the certificate buffer
**      3. pSubjName - pointer to the subject name buffer, output parameter
**      4. subjNameLen - length of the subject name buffer, output parameter
**
**  RETURNS:
**     succes or error code - unsigned long
**
**  NOTES:
**
******************************************************************************
*/
unsigned long secCertGetSubjectNameOS( unsigned char *pCertBuf,
      unsigned long pCertBufLen,
      unsigned char **pSubjName,
      unsigned long *subjNameLen )
{
   X509 *x509p;               /* certificate */
   X509_NAME *name;           /* subject name */


   /* decode cert from DER format */
   x509p = decodeDERCert( pCertBuf, pCertBufLen );

   if ( !x509p )
   {
      return MEM_ALLOC_ERROR;
   }

   /* subject name exists ? */
   if ( !x509p->cert_info->subject )
   {
      return BAD_DATA;
   }

   /* get the subject name */
   name = X509_get_subject_name( x509p );

   /* get the size of the buffer */
   *subjNameLen = name->bytes->length;

   /* allocate space for output buffer */
   *pSubjName = (unsigned char*)malloc( *subjNameLen );

   /* copy the subject name to the buffer in ASN1 encoded form */
   memcpy( *pSubjName, name->bytes->data, *subjNameLen );

   /* free the certificate storage */
   X509_free( x509p );

   ERR_remove_state( 0 );
   /* return success */
   return OK;
   }

/*
******************************************************************************
**  Function Name: secCertGetIssuerName
**
**  PURPOSE:
**      Get certificate issuer name ASN1 as a printable string
**
**  PARAMETERS:
**      1. pCertBuf - pointer to the certicate buffer.
**      2. pCertBufLen - length of the certificate buffer
**      3. pIssuerName - pointer to the subject name buffer, output parameter
**      4. issuerNameLen - length of the subject name buffer, output parameter
**
**  RETURNS:
**     succes or error code - unsigned long
**
**  NOTES:
**
******************************************************************************
*/
unsigned long secCertGetIssuerName( unsigned char *pCertBuf,
      unsigned long pCertBufLen,
      unsigned char **pIssuerName,
      unsigned long *issuerNameLen )
{
   X509 *x509p;      /* certificate */
   char buf[256];    /* buffer to store the name */


   /* decode the cert from DER format */
   x509p = decodeDERCert( pCertBuf, pCertBufLen );

   if ( !x509p )
   {
      return MEM_ALLOC_ERROR;
   }

   /* issuer name exists ? */
   if ( !x509p->cert_info->issuer )
   {
      return BAD_DATA;
   }

   /* get the issuer name */
   if ( !X509_NAME_oneline( X509_get_issuer_name( x509p ), buf, 256 ) )
   {
      return BAD_DATA;
   }

   /* get the size of the buffer */
   *issuerNameLen = strlen( buf );

   /* allocate space for output buffer */
   *pIssuerName = (unsigned char*)malloc( *issuerNameLen );

   /* copy the subject name to the proper place */
   memcpy( *pIssuerName, buf, *issuerNameLen );

   X509_free( x509p );

   ERR_remove_state( 0 );
   /* return success */
   return OK;

}

/*
******************************************************************************
**  Function Name: secCertGetIssuerNameOS
**
**  PURPOSE:
**      Get certificate issuer name ASN1 encoded as an octet string
**
**  PARAMETERS:
**      1. pCertBuf - pointer to the certicate buffer.
**      2. pCertBufLen - length of the certificate buffer
**      3. pIssuerName - pointer to the subject name buffer, output parameter
**      4. issuerNameLen - length of the subject name buffer, output parameter
**
**  RETURNS:
**     succes or error code - unsigned long
**
**  NOTES:
**
*****************************************************************************
*/
unsigned long secCertGetIssuerNameOS( unsigned char *pCertBuf,
      unsigned long pCertBufLen,
      unsigned char **pIssuerName,
      unsigned long *issuerNameLen )
{
   X509 *x509p;     /* certificate */
   X509_NAME *name;        /* subject name */

   /* decode the cert from DER format */
   x509p = decodeDERCert( pCertBuf, pCertBufLen );

   if ( !x509p )
   {
      return MEM_ALLOC_ERROR;
   }

   /* issuer name exists ? */
   if ( !x509p->cert_info->issuer )
   {
      return BAD_DATA;
   }

   /* get the issuer name */
   name = X509_get_issuer_name( x509p );

   /* get the size of the buffer */
   *issuerNameLen = name->bytes->length;

   /* allocate space for output buffer */
   *pIssuerName = (unsigned char*)malloc( *issuerNameLen );

   /* copy the subject name to the proper place */
   memcpy( *pIssuerName, name->bytes->data, *issuerNameLen );

   /* free the certificate storage */
   X509_free( x509p );

   ERR_remove_state( 0 );

   /* return success */
   return OK;

}


/*
*****************************************************************************
**  Function Name: secSign
**
**  PURPOSE:
**     To sign the data in input buffer using the input private key buffer
**     Key used is RSA key and the digest method is SHA1
**
**  PARAMETERS:
**      1. inBuffer - input buffer.
**      2. inBufferLen - length of the input buffer
**      3. key - iternal representation of RSA private key buffer
**      4. outSignature - signature buffer, used as an output parameter
**      5. outSignatureLen - length of the signature buffer, an output paramete
**
**  RETURNS:
**     succes or error code - unsigned long
**
**  NOTES:
**
******************************************************************************
*/
unsigned long secSign( unsigned char *inBuffer,
      unsigned long inBufferLen,
      RSA_key *key,
      unsigned char **outSignature,
      unsigned int *outSignatureLen )
{
   RSA *keyLocal;                               /* OpenSSL RSA key structure */
   int cypherLen;                               /* RSA key length */
   unsigned char *message;                      /* holds pieces of original message */
   unsigned char SHA1digest[SHA_DIGEST_LENGTH]; /* holds digests */
   div_t divResult;                             /* div_t type to hold multiple of key lengths in input data */
   unsigned int localLength;                    /* holds signature length */
   int offset = 0;                              /* offset in the input buffer */
   int counter;                                 /* while loop index */
   int status;                                  /* status code for RSA_sign function, returned value */

   /* allocate RSA key */
   keyLocal = RSA_new();

   /* put key modulos and private exponent in big number repr in newly allocated RSA key */
   keyLocal->n = BN_bin2bn( key->mod, key->modBufLen, NULL );
   keyLocal->d = BN_bin2bn( key->exp, key->expBufLen, NULL );

   /* key length */
   cypherLen = RSA_size( keyLocal );


   /* calculate mutiple number - how many key lengths in input buffer */
   divResult = div( inBufferLen, cypherLen );

   /* intialize counter */
   counter = divResult.quot;

   /* clear the output parameter */
   *outSignatureLen = 0;

   /* key is bigger than the input buffer ? */
   if ( divResult.quot == 0 )
   {
      /* get the message digest */
      SHA1( inBuffer, inBufferLen, SHA1digest );

      /* allocate space for output parameter, size is key length */
      *outSignature = (unsigned char*)malloc( cypherLen );

      /* sign the digest */
      status = RSA_sign( NID_sha1, SHA1digest, SHA_DIGEST_LENGTH, *outSignature, outSignatureLen, keyLocal );

      /* check for success */
      if ( status )
      {
         RSA_free( keyLocal );
         return OK;
      }
      else
      {
         RSA_free( keyLocal );
         return SIGN_ERROR;
      }
   }

   /* input buffer length is bigger than key length */
   else
   {

      /* allocate enough space for outSignature */
      if ( divResult.rem != 0 )
      {
         *outSignature = (unsigned char*)malloc( (divResult.quot+1)*cypherLen );
      }
      else
      {
         *outSignature = (unsigned char*)malloc( divResult.quot*cypherLen );
      }

      /* allocate space for pieces of input buffer */
      message = (unsigned char*)malloc( cypherLen );

      /* do it counter times */
      while ( counter != 0 )
      {

         /* get the message */
         memcpy( message, inBuffer + offset, cypherLen );

         /* get the digest */
         SHA1( message, cypherLen, SHA1digest );

         /* sign the digest and check for success */
         if ( !RSA_sign( NID_sha1, SHA1digest, SHA_DIGEST_LENGTH, *outSignature + offset, &localLength, keyLocal) )
         {
            /* error - free the memory and return */
            free( message );
            RSA_free( keyLocal );

            return SIGN_ERROR;
         }

         /* update the offset, counter, and output length  */
         offset = offset + cypherLen;
         counter--;
         *outSignatureLen = *outSignatureLen + localLength;
      }

      /* more data in input buffer - if so do the same above for the last time */
      if ( divResult.rem != 0 )
      {
         memcpy(message, inBuffer + offset, divResult.rem );

         SHA1( message, divResult.rem, SHA1digest );

         if ( !RSA_sign( NID_sha1, SHA1digest, SHA_DIGEST_LENGTH, *outSignature + offset, &localLength, keyLocal) )
         {
            free( message );
            RSA_free( keyLocal );
            return SIGN_ERROR;
         }

         *outSignatureLen = *outSignatureLen + localLength;
      }

      /* free the memory and return OK */
      free( message );
      RSA_free( keyLocal );
      return OK;
   }
}

/*
******************************************************************************
**  Function Name: secSignVerify
**
**  PURPOSE:
**     To verify the signed data stored in input buffer using the input public key buffer.
**     The original data is stored in input buffer inData.
**     Key used is RSA key and the digest method is SHA1.
**
**  PARAMETERS:
**      1. inData - input buffer, original data - to verify against
**      2. inDataLen - length of the input buffer
**      3. pSignature - signature buffer to be verified
**      4. sigLen - length of the signature buffer
**      5. key - iternal representation of RSA public key buffer
**
**  RETURNS:
**     succes or error code - unsigned long
**
**  NOTES:
**
******************************************************************************
*/
unsigned long secSignVerify( unsigned char *inData,
      unsigned long inDataLen,
      unsigned char *pSignature,
      unsigned long pSigLen, RSA_key *key )
{

   RSA *keyLocal;                               /* OpenSSL RSA key structure */
   int cypherLen;                               /* RSA key length */
   unsigned char *message;                      /* holds pieces of original message */
   unsigned char SHA1digest[SHA_DIGEST_LENGTH]; /* holds digests */
   div_t divResult;                             /* div_t type to hold multiple of key lengths in input data */
   int offset = 0;                              /* offset in the input buffer */
   int counter;                                 /* while loop index */
   int status;                                  /* status code for RSA_sign function, returned value */

   /* allocate the RSA key */
   keyLocal = RSA_new();

   /* put the modulos and the public exponent in the key */
   keyLocal->n = BN_bin2bn( key->mod, key->modBufLen, NULL );
   keyLocal->e = BN_bin2bn( key->exp, key->expBufLen, NULL );


   /* get the key length */
   cypherLen = RSA_size( keyLocal );


   /* calculate multiple number */
   divResult = div( inDataLen, cypherLen );
   counter = divResult.quot;

   /* key is bigger than the input data ? */
   if ( divResult.quot == 0 )
   {
      /* get the digest of message */
      SHA1( inData, inDataLen, SHA1digest );

      /* verify the digest against the signature */
      status = RSA_verify( NID_sha1, SHA1digest, SHA_DIGEST_LENGTH, pSignature, cypherLen, keyLocal );

      /* check for errror and return */
      if ( status )
      {
         RSA_free( keyLocal );
         return OK;
      }
      else
      {
         RSA_free( keyLocal );
         return VERIFY_ERROR;
      }
   }

   /* input data is bigger than the key */
   else
   {

      /* allocate the space for message */
      message = (unsigned char*)malloc( cypherLen );

      /* do it counter times */
      while ( counter != 0 )
      {

         /* get the message */
         memcpy( message, inData + offset, cypherLen );

         /* get the digest */
         SHA1( message, cypherLen, SHA1digest );

         /* verify the message and check for error */
         if ( !RSA_verify( NID_sha1, SHA1digest, SHA_DIGEST_LENGTH, pSignature + offset, cypherLen, keyLocal) )
         {
            /* free the memory and return error */
            free( message );
            RSA_free( keyLocal );

            return VERIFY_ERROR;
         }

         /* update the counter and the offset */
         offset = offset + cypherLen;
         counter--;
      }

      /* any input data left - if so do the same as above for the last time */
      if ( divResult.rem != 0 )
      {
         memcpy(message, inData + offset, divResult.rem );

         SHA1( message, divResult.rem, SHA1digest );

         if ( !RSA_verify( NID_sha1, SHA1digest, SHA_DIGEST_LENGTH, pSignature + offset, cypherLen, keyLocal) )
         {
            free( message );
            RSA_free( keyLocal );
            return VERIFY_ERROR;
         }

      }

      /* free the memory and return OK */
      free( message );
      RSA_free( keyLocal );
      return OK;
   }
}


/*
******************************************************************************
**  Function Name: cb
**
**  PURPOSE:
**     Callback function used in certificate verification - stored error/warnings if any
**
**  PARAMETERS:
**      1. ok - error indicator passed to function by some other method
**      2. ctx - certificate storage buffer, contains all info about certificat
**
**  RETURNS:
**     succes or error code
**
**  NOTES:
**
*****************************************************************************
*/
static int MS_CALLBACK cb(int ok, X509_STORE_CTX *ctx)
{
	char buf[256];
   char temp[256];

	if (!ok)
	{
		errIndicator = SELF_SIGN_OR_PATH;

      X509_NAME_oneline(
				X509_get_subject_name(ctx->current_cert),buf,256);

      /* put the warning/error in the buffer */
      sprintf( temp,"%s %s", buf, X509_verify_cert_error_string(ctx->error) );
      strncpy( pErrBuf1, temp, errBufLength );


		if (ctx->error == X509_V_ERR_CERT_HAS_EXPIRED)
      {
         ok=1;
         errIndicator = CERT_EXPIRED;
      }

		/* since we are just checking the certificates, it is
		 * ok if they are self signed. But we should still warn
		 * the user.
 		 */
		if (ctx->error == X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT) ok=1;
		/* Continue after extension errors too */

      if (ctx->error == X509_V_ERR_INVALID_CA)
      {
         ok=1;
         errIndicator = CA_INVALID;
      }

		if (ctx->error == X509_V_ERR_PATH_LENGTH_EXCEEDED) ok=1;

	}
	if (!v_verbose)
		ERR_clear_error();
	return(ok);
}

/*
******************************************************************************
**  Function Name: decodeDERCert
**
**  PURPOSE:
**     To decode certificate content from DER format
**
**  PARAMETERS:
**      1. pCertBuf - pointer to the certicate buffer.
**      2. bufferLength - length of the certificate buffer
**
**  RETURNS:
**     Pointer to the certificate data structure - decoded
**
**  NOTES:
**
******************************************************************************
*/
static X509* decodeDERCert( unsigned char *pCertBuf, unsigned int bufferLength )
{

   X509 *x509;   /* certificate to be returned */
   BIO *in;             /* intermediate buffer */

   /* allocate BIO buffer with appropriate method */
   in = BIO_new( BIO_s_mem());

   if ( !in )
   {
      return NULL;
   }

   /* write to BIO buffer from certificate buffer */
   in->method->bwrite( in, pCertBuf, bufferLength );

   /* decode DER content to pure x509 structure */
   x509 = d2i_X509_bio( in, NULL );

   /* free BIO buffer */
   BIO_free( in );

   /* return decoded certificate */
   return x509;
}        

