/***************************************************************************
 *     (c)2003-2011 Broadcom Corporation
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
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description: Header file for RSA software implementation.
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#ifndef BCRYPT_x509_H_
#define BCRYPT_x509_H_


/* Added to remove header file dependency */
#include <openssl/x509.h>
#include "bcrypt_rsa_sw.h"



/* Basic Module Functions */


/*****************************************************************************
Summary:
This function decodes a x509 certificate file (ASN.1 DER encoded).


Description:
This function decodes a x509 certificate file. The input must be a ASN.1 DER encoded format.
The output is a pointer to the certificate in the x509 structure as defined in the openssl.
The input is an array of byte. The calling function call needs to open the ASN.1 DER encoded 
file and copy its contents to memory before calling this routine.


Calling Context:
The function shall be called from application level or from driver level, for example in Linux
during insmod.


Performance and Timing:
This is a synchronous/blocking function that will not return until it is done or failed.


Input:
hBcrypt  - BCRYPT_Handle
x509Data - const unsigned char, a ref/pointer to input data (ASN.1 DER encoded format) 
			that needs to be decoded. 
nDataLen - int, length of input data.


Output:
pCertificate - X509**, a pointer to another ref/pointer to x509 structure defined in openssl.

			
Returns:
BCRYPT_STATUS_eOK - success
BCRYPT_STATUS_eFAILED - failure 


See Also:
BCrypt_x509GetDigestAlgorithm, BCrypt_x509Free

******************************************************************************/
BCRYPT_STATUS_eCode BCrypt_x509ASN1DerDecode(
							BCRYPT_Handle  hBcrypt, 
							const unsigned char*  x509Data, 
                                          	int nDataLen, 
                                          	X509** pCertificate);



/*****************************************************************************
Summary:
This routine frees the memory used to store the x509 structure.


Description:
This routine frees the memory used to store the x509 structure.
Before calling this routine, the user must finish decoding the x509 file, getting the public key 
and getting the digest algorithm.


Calling Context:
The function shall be called from application level or from driver level, for example in Linux
during insmod.


Performance and Timing:
This is a synchronous/blocking function that will not return until it is done or failed.


Input:
hBcrypt  - BCRYPT_Handle
pCertificate - X509*, a ref/pointer to x509 structure.

			
Returns:
BCRYPT_STATUS_eOK - success
BCRYPT_STATUS_eFAILED - failure 


See Also:
BCrypt_x509ASN1DerDecode, BCrypt_x509GetDigestAlgorithm

******************************************************************************/
void BCrypt_x509Free(BCRYPT_Handle  hBcrypt, X509* m_pCertificate);



/*****************************************************************************
Summary:
This routine gets the digest algorithm string that is used in the x509 structure.


Description:
This routine gets the digest algorithm string that is used in the x509 structure. Before calling
this function, the user must call the BCrypt_x509ASN1DerDecode API to convert the 
ASN.1 DER encoded format to the x509 sturcture that is defined in openssl.


Calling Context:
The function shall be called from application level or from driver level, for example in Linux
during insmod.


Performance and Timing:
This is a synchronous/blocking function that will not return until it is done or failed.


Input:
hBcrypt  - BCRYPT_Handle
pCertificate - X509*, a ref/pointer to x509 structure as defined in the openssl.


Output:
szAlgorithm -  char*, a string that contains the algorithm used in the x509 certificate.
len - int, the size of the output algorithm.

			
Returns:
BCRYPT_STATUS_eOK - success
BCRYPT_STATUS_eFAILED - failure 


See Also:
BCrypt_x509ASN1DerDecode, BCrypt_x509Free

******************************************************************************/
BCRYPT_STATUS_eCode BCrypt_x509GetDigestAlgorithm(
							BCRYPT_Handle  hBcrypt, 
							X509* m_pCertificate,
							char* szAlgorithm, 
							int len);


/*****************************************************************************
Summary:
This routine gets the RSA public key's modulus and exponent out of the x509 structure. 


Description:
This function gets the RSA public key's modulus and exponent out of the x509 structure as 
as defined in the openssl. If the input is a certificate in ASN.1 DER encoded format then it 
should be decoded to the x509 certifcate structure using the BCrypt_x509ASN1DerDecode 
API before this function can be files.


Calling Context:
The function shall be called from application level or from driver level, for example in Linux
during insmod.


Performance and Timing:
This is a synchronous/blocking function that will not return until it is done or failed.


Input:
hBcrypt  - BCRYPT_Handle
pCertificate - X509*, a ref/pointer to x509 structure as defined in the openssl.


Output:
rsa_key -  BCRYPT_RSAKey_t, a ref/pointer to the RSA key. 

			
Returns:
BCRYPT_STATUS_eOK - success
BCRYPT_STATUS_eFAILED - failure 


See Also:
BCrypt_publicKeyFree

******************************************************************************/
BCRYPT_STATUS_eCode BCrypt_x509GetRsaPublicKey(
							BCRYPT_Handle  hBcrypt, 
							X509* m_pCertificate, 
							BCRYPT_RSAKey_t * rsa_key);



/*****************************************************************************
Summary:
This routine frees the memory used to store the RSA public key.


Description:
This routine frees the memory used to store the RSA public key.


Calling Context:
The function shall be called from application level or from driver level, for example in Linux
during insmod.


Performance and Timing:
This is a synchronous/blocking function that will not return until it is done or failed.


Input:
hBcrypt  - BCRYPT_Handle
rsa_key -  BCRYPT_RSAKey_t, a ref/pointer to the RSA public key. 

			
Returns:
BCRYPT_STATUS_eOK - success
BCRYPT_STATUS_eFAILED - failure 


See Also:
BCrypt_x509GetRsaPublicKey

******************************************************************************/
void BCrypt_publicKeyFree(BCRYPT_Handle  hBcrypt, BCRYPT_RSAKey_t* rsa_key);



/*****************************************************************************
Summary:
This routine reads the RSA private key from a input file in the PEM format. 


Description:
This routine reads the RSA private key from a input file in the PEM format.  The input file
should be in teh PEM format and the ouput is in the RSA key struct format and the size
of the private key.


Calling Context:
The function shall be called from application level or from driver level, for example in Linux
during insmod.


Performance and Timing:
This is a synchronous/blocking function that will not return until it is done or failed.


Input:
hBcrypt  - BCRYPT_Handle
fp_privKeyIn - FILE*, the input file that contains the RSA private key in PEM format.


Output:
rsa_key -  BCRYPT_RSAKey_t, a ref/pointer to the RSA key. 
p_nSize - long*, a pointer to the size of the private key.

			
Returns:
BCRYPT_STATUS_eOK - success
BCRYPT_STATUS_eFAILED - failure 


See Also:
BCrypt_privateKeyFree

******************************************************************************/
BCRYPT_STATUS_eCode BCrypt_RSAReadPrivateKeyPem(
							BCRYPT_Handle  hBcrypt, 
							FILE* fp_privKeyIn,
							BCRYPT_RSAKey_t* rsa_key, 
							long* p_nSize );



/*****************************************************************************
Summary:
This routine frees the memory used to store the RSA private key.


Description:
This routine frees the memory used to store the RSA private key.


Calling Context:
The function shall be called from application level or from driver level, for example in Linux
during insmod.


Performance and Timing:
This is a synchronous/blocking function that will not return until it is done or failed.


Input:
hBcrypt  - BCRYPT_Handle
rsa_key -  BCRYPT_RSAKey_t, a ref/pointer to the RSA private key. 

			
Returns:
BCRYPT_STATUS_eOK - success
BCRYPT_STATUS_eFAILED - failure 


See Also:
BCrypt_RSAReadPrivateKeyPem

******************************************************************************/
void BCrypt_privateKeyFree(BCRYPT_Handle  hBcrypt, BCRYPT_RSAKey_t* rsa_key);



#endif /* BCRYPT_x509_H_ */
