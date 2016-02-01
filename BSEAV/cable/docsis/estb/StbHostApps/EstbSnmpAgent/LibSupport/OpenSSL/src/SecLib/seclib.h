/***************************************************************************
*     Copyright (c) 2008 Broadcom Corporation
*    All Rights Reserved
*    No portions of this material may be reproduced in any form without the
*    written permission of:
*             Broadcom Corporation
*             16251 Laguna Canyon Road
*             Irvine, California  92618
*    All information contained in this document is Broadcom Corporation
*    company private, proprietary, and trade secret.
*
****************************************************************************
*
*    Filename:  SecLib.h 
*    Creation Date: 06 September 2000 (v0.00)
*    VSS Info:
*        $Revision: 1.1 $
*        $Date: 2002/04/18 18:50:20Z $
*
****************************************************************************
*    Description:
*
*      This file contains data type defintions, new defines, and function    
*      type prototypes for SecLib library.  
*
*    Notes:
*          
****************************************************************************/
#include <string.h>

#ifndef SECLIB_H
#define SECLIB_H


/* error codes */
#define BAD_DATA  1
#define GENERATOR_ERROR  2
#define KEY_GEN_ERROR 3
#define MEM_ALLOC_ERROR 4
#define STRING_SET_ERROR 5
#define BUF_SHORT 6
/* verification errors */
#define SELF_SIGN_OR_PATH 7
#define CERT_EXPIRED 8
#define CA_INVALID 9
#define SIGN_ERROR 10
#define VERIFY_ERROR 11

#define OK  0

/* public key envelope macros */
#define EVP_PKEY_RSA	NID_rsaEncryption
#define EVP_PKEY_DSA	NID_dsa


/* algorithm's specific defines */
#define LN_rsaEncryption		"rsaEncryption"
#define NID_rsaEncryption		6

#define LN_dsa				"dsaEncryption"
#define NID_dsa				116

#define LN_md5WithRSAEncryption		"md5WithRSAEncryption"
#define NID_md5WithRSAEncryption	8


#define LN_shaWithRSAEncryption		"shaWithRSAEncryption"
#define NID_shaWithRSAEncryption	42

#define LN_sha1WithRSAEncryption	"sha1WithRSAEncryption"
#define NID_sha1WithRSAEncryption	65

#define LN_md2WithRSAEncryption		"md2WithRSAEncryption"
#define NID_md2WithRSAEncryption	7

typedef struct 
{
   unsigned char *mod;
   unsigned long modBufLen;
   unsigned char *exp;
   unsigned long expBufLen;
   unsigned long keyLenBits;
}RSA_key;


/* Macro definitions */
#define ALLOC_RSA_KEY( pRSAkey )  \
        { \
            pRSAkey = malloc(sizeof(RSA_key)); \
            memset( pRSAkey, 0, sizeof(RSA_key)); \
        }


#define CREATE_RSA_KEY( pRSAkey, m, d, size_m, size_d ) \
         { \
            pRSAkey->mod = m; \
            pRSAkey->exp = d; \
            pRSAkey->modBufLen = size_m; \
            pRSAkey->expBufLen = size_d; \
         }

#define FREE_RSA_KEY( pRSAkey, indicator ) \
         { \
            if ( indicator ) free(pRSAkey->mod); \
            if ( indicator ) free(pRSAkey->exp); \
            if (pRSAkey) free(pRSAkey); \
         }

/* function prototypes */


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
******************************************************************************
*/
unsigned int secCryptoDhGener( unsigned char *pPrime, 
      unsigned long primeLen, 
      unsigned long generator,                           
      unsigned char **pPubKey, 
      unsigned long *pubKeyLen, 
      unsigned char **pPrivKey,
      unsigned long *privKeyLen );



/*
******************************************************************************
** FUNCTION:   secCryptoDhComputeSecret
**
** PURPOSE:    To compute and return Diffie-Hellman shared secret
**
** PARAMETERS:       
**       1. pSecretKey - pointer to secret key buffer, output parameter   
**       2. secretKeyLen - lenght of secret key buffer in bytes, output paramet
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
******************************************************************************
*/
unsigned int secCryptoDhComputeSecret( unsigned char **pSecretKey, 
      unsigned long *secretKeyLen, 
      unsigned char *pPrivKey,                                  
      unsigned long privKeyLen, 
      unsigned char *pPubKeyOther, 
      unsigned long pubKeyOtherLen,                 
      unsigned char *pPrime, 
      unsigned long primeLen, 
      unsigned long generator );

/*
***************************************************************************************
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
***************************************************************************************
*/  
unsigned int secCryptoBitStringEnc( unsigned char *pKeyBuffer, 
      unsigned long keyBufferLen,                                  
      unsigned char **ppBitStrBuf, 
      unsigned long *bitStrBufLen);



/*
******************************************************************************
**     
**  Function Name: secCertGetPublicKey  
**         
**  PURPOSE:
**     Retrieve public key and its length. Key buffer contains key itself in HEX.    
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
******************************************************************************
*/
unsigned long secCertGetPublicKey( unsigned char *pCertBuf, 
      unsigned int pCertBufLen, 
      RSA_key *key,                               
      unsigned int *pPubKeyId );


/* 
****************************************************************************
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
******************************************************************************
*/
unsigned long secCertValidate( unsigned char *pCertBuf, 
      unsigned int pCertBufLen, 
      unsigned int pCertCALen,                        
      unsigned char *pCertCA, 
      unsigned char *pErrBuf, 
      int errBufLen );


/*
*****************************************************************************
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
      unsigned long *pSerialNumber );


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
unsigned long secCertGetSigAlgId( unsigned char *pCertBuf, 
      unsigned int pCertBufLen, 
      unsigned long *pAlgld );


/*
******************************************************************************
**  Function Name: secCertGetNotAfterDate  
**  
**  PURPOSE:   
**      Get certificate not after date   
**   
**  PARAMETERS:      
**      1. pCertBuf - pointer to the certicate buffer. 
**      2. pCertBufLen - length of the certificate buffer      
**      3. pNotAfterBuf - pointer to the not after date buffer, output parameter
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
      unsigned int *pNotAfterBufLen );


/*
******************************************************************************
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
      unsigned int *pNotBeforeBufLen );

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
*****************************************************************************
*/
unsigned long secCertGetSubjectName( unsigned char *pCertBuf, 
      unsigned long pCertBufLen, 
      unsigned char **pSubjName,
      unsigned long *subjNameLen );


/*
******************************************************************************
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
      unsigned long *subjNameLen );

/*
*****************************************************************************
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
*****************************************************************************
*/
unsigned long secCertGetIssuerName( unsigned char *pCertBuf, 
      unsigned long pCertBufLen, 
      unsigned char **pIssuerName,
      unsigned long *issuerNameLen );


/*
*****************************************************************************
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
      unsigned long *issuerNameLen);


/* 
*****************************************************************************
**  Function Name: secSign   
**           
**  PURPOSE: 
**     To sign the data in input buffer using the input private key buffer.
**     Key used is RSA key and the digest method is SHA1.
**        
**  PARAMETERS: 
**      1. inBuffer - input buffer.          
**      2. inBufferLen - length of the input buffer  
**      3. key - iternal representation of RSA private key buffe
**      4. outSignature - signature buffer, used as an output parameter  
**      5. outSignatureLen - length of the signature buffer, an output parameter
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
      unsigned int *outSignatureLen );

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
***************************************************************************
*/
unsigned long secSignVerify( unsigned char *inData, 
      unsigned long inDataLen, 
      unsigned char *pSignature, 
      unsigned long pSigLen, 
      RSA_key *key );


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
      unsigned char *pKeyBuffer );

#endif
