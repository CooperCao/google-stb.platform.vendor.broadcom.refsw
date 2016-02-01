/******************************************************************************
 *    (c)2011 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 *****************************************************************************/
#include <openssl/x509.h>
#include "drm_common_swcrypto_types.h"

#ifndef DRM_COMMON_SWCRYPTO_H_
#define DRM_COMMON_SWCRYPTO_H_

/******************************************************************************
** FUNCTION
**   DRM_Common_SwSha1Sw
**
** DESCRIPTION:
**    Performs a SHA-1 on the buffer pointed to by 'pSrc' (of size 'size')and
**    returns the calculated digest in 'pDigest'
**
** PARAMETERS:
**   pSrc - pointer to the buffer containing the source data
**   pDigest - pointer to a buffer of at least 20 bytes that will contain
**   			the returned SHA-1 diagest.
**   size - size of the data to perform the SHA-1 calculation on
**
** RETURNS:
**   Drm_Success when the operation is successful or an error.
**
******************************************************************************/
DrmRC DRM_Common_SwSha1(
		uint8_t *pSrc,
		uint8_t *pDigest,
		uint32_t size
);


/******************************************************************************
** FUNCTION
**   DRM_Common_SwPartialSha1Sw
**
** DESCRIPTION:
**    Performs a partial SHA-1 on the buffer pointed to by 'pSrc' (of size 'size')and
**    returns the calculated digest in 'pDigest'
**
** PARAMETERS:
**   pSrc - pointer to the buffer containing the source data
**   pDigest - pointer to a buffer of at least 20 bytes that will contain
**   			the returned SHA-1 diagest.
**   size - size of the data to perform the SHA-1 calculation on
**   partialSha1Type - specifies whether the current packet to be hashed is
**   					an 'init', 'middle' or 'finalize' packet.
**
** RETURNS:
**   Drm_Success when the operation is successful or an error.
******************************************************************************/
DrmRC DRM_Common_SwPartialSha1(
		uint8_t *pSrc,
		uint8_t *pDigest,
		uint32_t size,
		DrmCommon_PartialSha1Type partialSha1Type,
		uint32_t *returnContext
);


/******************************************************************************
** FUNCTION
**   DRM_Common_SwSha256Sw
**
** DESCRIPTION:
**    Performs a SHA-256 on the buffer pointed to by 'pSrc' (of size 'size')and
**    returns the calculated digest in 'pDigest'
**
** PARAMETERS:
**   pSrc - pointer to the buffer containing the source data
**   pDigest - pointer to a buffer of at least 32 bytes that will contain
**   			the returned SHA-256 diagest.
**   size - size of the data to perform the SHA-256 calculation on
**
** RETURNS:
**   Drm_Success when the operation is successful or an error.
**
******************************************************************************/
#ifdef __cplusplus

extern "C" {
#endif

DrmRC DRM_Common_SwSha256(
		uint8_t *pSrc,
		uint8_t *pDigest,
		uint32_t size);


#ifdef __cplusplus

}
#endif

/****************************************************************
** DRM_Common_SwEcdsaVerifySw
**
** INPUTS:	hBcrypt - bcrypt handle
** 			inoutp_ecdsaSwIO - pointer to input/output parameter structure
**
** OUTPUTS:	inoutp_ecdsaSwIO->sigIsValid -  true if signature is valid
** 											false if signature is not valid
** RETURNS:	bcrypt error code
** FUNCTION: This function performs ECDSA signature verification.
** 			The EC curve parameters are passed in by the caller,
** 			along with the signature, hash value, and public key.
**
****************************************************************/
DrmRC DRM_Common_SwEcdsaVerify(
		DrmCommon_ECDSASw_Verify_t 	*inoutp_ecdsaSwIO);


/*****************************************************************************
** Summary:
** This function implements the ECDSA Sign algorithm in SW.
**
** Performance and Timing:
** This is a synchronous/blocking function that will not return until it is done or failed.
**
** Input:
** inoutp_ecdsaSwIO - BCRYPT_ECDSASw_Sign_t, a ref/pointer to the parameters
** 		for the SW implementation of the ECDSA Sign algorithm.
**
** Returns:
** 	Drm_Success - success or other to indicate a failure
******************************************************************************/
DrmRC DRM_Common_SwEcdsaSign(
		DrmCommon_ECDSASw_Sign_t 	*inoutp_ecdsaSwIO
);

/*****************************************************************************
Summary:
This function implements the ECDSA scalar multiplications algorithm in SW.

Performance and Timing:
This is a synchronous/blocking function that will not return until it is done or failed.

Input:
inoutp_ecdsaSwIO - BCRYPT_ECDSASw_Multiply_t, a ref/pointer to the parameters
		for the SW implementation of the ECDSA scalar multiplication algorithm.

Returns:
	Drm_Success - success or other to indicate a failure
******************************************************************************/
DrmRC DRM_Common_SwEcdsaMultiply(
		DrmCommon_ECDSASw_Multiply_t 	*inoutp_ecdsaSwIO
);


/*****************************************************************************
** Summary:
** 	This function implements the AES algorithm in the ECB mode of operation in SW.
**
** Description:
** This function implements the AES algorithm in the ECB mode of operation in software. It is
** based on the AES implementation of the openssl code. All key lengths of 128, 192, and 256
** are supported. The binaries of the openssl library is already included into the build
** environment. Since this is a software implementation for the AES algorithm in the ECB mode
** of operation, it can only be used with the clear key.
** If the application requires an encrypted key, then the key must be decrypted before sending
** in to this function call.
**
** Performance and Timing:
** 	This is a synchronous/blocking function that will not return until it is done or failed.
**
** Input:
** inoutp_AESECBSwCtrl - DrmCommon_AesEcbSw_t, a ref/pointer to the parameters
** 		for the SW implementation of the AES algorithm in the ECB mode of operation.
**
** Returns:
** 	Drm_Success (success) or other for failure
******************************************************************************/
DrmRC DRM_Common_SwAesEcb(
		DrmCommon_AesEcbSw_t 	*inoutp_aesecbSwIO
);

/*****************************************************************************
** Summary:
** This function implements the AES algorithm in the CBC mode of operation in SW.
**
** Description:
** This function implements the AES algorithm in the CBC mode of operation in software. It is
** based on the AES implementation of the openssl code. All key lengths of 128, 192, and 256
** are supported. The binaries of the openssl library is already included into the build
** environment. Since this is a software implementation for the AES algorithm in the CBC mode
** of operation, it can only be used with the clear key.
** If the application requires an encrypted key, then the key must be decrypted before sending
** in to this function call.
**
** Performance and Timing:
** This is a synchronous/blocking function that will not return until it is done or failed.
**
** Input:
** inoutp_AESCBCSwCtrl - BCRYPT_S_AESCBCSwCtrl_t, a ref/pointer to the parameters
** 		for the SW implementation of the AES algorithm in the CBC mode of operation.
**
** Returns:
** 	Drm_Success (success) or other for failure
******************************************************************************/
DrmRC DRM_Common_SwAesCbc(
		DrmCommon_AesCbcSw_t 	*inoutp_aescbcSwIO
);


/*********************************************************************************
** CMAC function
** SW implementation of the CMAC algorithm
**
** Returns:
** 	Drm_Success (success) or other for failure
*********************************************************************************/
DrmRC DRM_Common_SwCmac(
		DrmCommon_CmacSw_t 	*inoutp_cmacSwIO
);


/*********************************************************************************
** DES CBC function
** SW implementation of the DES-CBC algorithm
**
** Returns:
** 	Drm_Success (success) or other for failure
*********************************************************************************/
DrmRC DRM_Common_SwDesCbc(
		DrmCommon_DesCbcSw_t *inoutp_desCbcSw
);


/*********************************************************************************
** DES ECB function
** SW implementation of the DES-ECB algorithm
**
** Returns:
** 	Drm_Success (success) or other for failure
*********************************************************************************/
DrmRC DRM_Common_SwDesEcb(
		DrmCommon_DesEcbSw_t *inoutp_desEcbSw
);


/*****************************************************************************
** Summary:
** This function implements different DSA operations in SW.
**
** Description:
** This function implements different DSA operations in software.
** The inputs include DSA key, input data, size of the input data, output signature, size of the
** output signature, the values of r and s for the signature to be verified, and the dsa operation
** parameter. Different DSA operations that are supported are DSA Sign and DSA Verify.
**
**
** Performance and Timing:
** This is a synchronous/blocking function that will not return until it is done or failed.
**
** Input:
** pInputParam - DrmCommon_DsaSw_t, a ref/pointer to the parameters for SW
** 			DSA operations.
**
** Returns:
** 	Drm_Success for success (or other for failure)
******************************************************************************/
DrmRC DRM_Common_SwDsa(
		DrmCommon_DsaSw_t *pInputParam
);


/*****************************************************************************
** Summary:
** This function implements the MD5 algorithm in SW.
**
** Description:
** This function implements the MD5 algorithm in software. It is
** based on the MD5 implementation of the openssl code.
**
** Performance and Timing:
** This is a synchronous/blocking function that will not return until it is done or failed.
**
** Input:
** pInputParam - DrmCommon_Md5Sw_t, a ref/pointer to the parameters
** 		for the SW implementation of the MD5 algorithm.
**
** Returns:
** 	Drm_Success for success (or other for failure)
******************************************************************************/
DrmRC DRM_Common_SwMd5(
		DrmCommon_Md5Sw_t *pInputParam
);



/*****************************************************************************
** Summary:
** This function sets the RC4 key.
**
** Performance and Timing:
** This is a synchronous/blocking function that will not return until it is done or failed.
**
** Input:
** key  - DrmCommon_Rc4Key_t, a pointer/ref to the Rc4 key.
** keyLen - uint32_t.
** keyData - uint32_t.
**
** Returns:
** Void
******************************************************************************/
void DRM_Common_SwRc4SetKey (
		DrmCommon_Rc4Key_t *key,
		uint32_t keyLen,
		uint8_t *keyData
);

/*****************************************************************************
** Summary:
** This function implements the RC4 algorithm in SW.
**
** Description:
** This function implements the MD5 algorithm in software. It is
** based on the RC4 implementation of the openssl code.
**
** Performance and Timing:
** This is a synchronous/blocking function that will not return until it is done or failed.
**
** Input:
** pInputParam - DrmCommon_Rc4Sw_t, a ref/pointer to the parameters
** 		for the SW implementation of the MD5 algorithm.
**
** Returns:
** 	Drm_Success for success (or other for failure)
******************************************************************************/
DrmRC DRM_Common_SwRc4(
		DrmCommon_Rc4Sw_t *pInputParam
);

/*****************************************************************************
** Summary:
** This function implements the RNG algorithm in SW.
**
** Description:
** This function implements the random numnber generation process in software. It is
** based on the RNG implementation of the openssl code.
**
** Performance and Timing:
** This is a synchronous/blocking function that will not return until it is done or failed.
**
** Input:
** inoutp_RNGSwIO - BCRYPT_S_RNGSwCtrl_t, a ref/pointer to the parameters
** 		for the SW implementation of the random number generation.
**
** Returns:
** 	Drm_Success for success (or other for failure)
******************************************************************************/
DrmRC DRM_Common_SwRng(
		DrmCommon_RngSw_t 	*inoutp_RNGSwIO
);


/********************************************************************************/
/*						RSA Functionality										*/
/********************************************************************************/
DrmRC DRM_Common_SwRsa(
		DrmCommon_RsaSwParam_t 	*inoutp_rsaSwIO
);

/*****************************************************************************
** Summary:
** This function extract the RSA key from Subject Public Info.
**
** Description:
**  This function parses the ASN1 encoded key buf and extracts out the bit
**	string public key from the subject public key info. Bit string public
**	key is converted to RSA public key, from which the requisite RSA key are
**	populated to output.
**
** Input:
**  buf: Pointer to the subject public key info.
**  length: Length of the subject public key info
**
** Output:
**  SwRsaKey: RSA key populated with public modulus and exponent values.
**
**
** Returns:
** 	Drm_Success for success (or other for failure)
******************************************************************************/
DrmRC DRM_Common_GetRSA_From_SubjectPublicKeyInfo(
		uint8_t *pBuf, uint32_t length,
		DrmCommon_RsaKey_t 	*pSwRsaKey
);
/*****************************************************************************
** Summary:
** 	This function extracts RSA key from private key info
**
** Description:
**  This function parses the ASN1 encoded private key info and extract out the
**  public modulus and exponent and private exponent of the RSA private key.
**
** Input:
**	buf: Pointer to the subject public key info.
** 	length: Length of the subject public key info.
**
** Output:
** 	SwRsaKey: RSA key populated with public modulus and exponent values.
**
** Returns:
** 	Drm_Success for success (or other for failure)
******************************************************************************/
DrmRC DRM_Common_GetRsa_From_PrivateKeyInfo(
		uint8_t *pBuf, uint32_t length,
		DrmCommon_RsaKey_t 	*pSwRsaKey
);

/*****************************************************************************
** Summary:
** This function decodes a x509 certificate file (ASN.1 DER encoded).
**
** Description:
** This function decodes a x509 certificate file. The input must be a ASN.1 DER encoded format.
** The output is a pointer to the certificate in the x509 structure as defined in the openssl.
** The input is an array of byte. The calling function call needs to open the ASN.1 DER encoded
** file and copy its contents to memory before calling this routine.
**
** Input:
** x509Data - const unsigned char, a ref/pointer to input data (ASN.1 DER encoded format)
** 			that needs to be decoded.
** nDataLen - int, length of input data.
**
** Output:
** pCertificate - X509**, a pointer to another ref/pointer to x509 structure defined in openssl.
**
** Returns:
** 	Drm_Success for success (or other for failure)
******************************************************************************/
DrmRC DRM_Common_Swx509ASN1DerDecode(
		const unsigned char*  x509Data,
		int nDataLen,
		X509** pCertificate
);



/*****************************************************************************
** Summary:
** This routine frees the memory used to store the x509 structure.
**
** Description:
** This routine frees the memory used to store the x509 structure.
** Before calling this routine, the user must finish decoding the x509 file, getting the public key
** and getting the digest algorithm.
** Input:
** pCertificate - X509*, a ref/pointer to x509 structure.
**
** Returns:
** 	Drm_Success for success (or other for failure)
******************************************************************************/
void DRM_Common_Swx509Free(
		X509* m_pCertificate
);



/*****************************************************************************
** Summary:
** This routine gets the digest algorithm string that is used in the x509 structure.
**
** Description:
** This routine gets the digest algorithm string that is used in the x509 structure. Before calling
** this function, the user must call the DRM_Common_Swx509ASN1DerDecode API to convert the
** ASN.1 DER encoded format to the x509 sturcture that is defined in openssl.
**
** Input:
** pCertificate - X509*, a ref/pointer to x509 structure as defined in the openssl.
**
** Output:
** szAlgorithm -  char*, a string that contains the algorithm used in the x509 certificate.
** len - int, the size of the output algorithm.
**
** Returns:
** 	Drm_Success for success (or other for failure)
******************************************************************************/
DrmRC DRM_Common_Swx509GetDigestAlgorithm(
							X509* m_pCertificate,
							char* szAlgorithm,
							int len);


/*****************************************************************************
** Summary:
** This routine gets the RSA public key's modulus and exponent out of the x509 structure.
**
** Description:
** This function gets the RSA public key's modulus and exponent out of the x509 structure as
** as defined in the openssl. If the input is a certificate in ASN.1 DER encoded format then it
** should be decoded to the x509 certifcate structure using the DRM_Common_Swx509ASN1DerDecode
** API before this function can be files.
**
** Input:
** pCertificate - X509*, a ref/pointer to x509 structure as defined in the openssl.
**
** Output:
** rsa_key -  DrmCommon_RsaKey_t, a ref/pointer to the RSA key.
**
** Returns:
** 	Drm_Success for success (or other for failure)
******************************************************************************/
DrmRC DRM_Common_Swx509GetRsaPublicKey(
							X509* m_pCertificate,
							DrmCommon_RsaKey_t * rsa_key);



/*****************************************************************************
** Summary:
** This routine frees the memory used to store the RSA public key.
**
** Description:
** This routine frees the memory used to store the RSA public key.
**
** Input:
** rsa_key -  DrmCommon_RsaKey_t, a ref/pointer to the RSA public key.
**
** Returns:
** 	Drm_Success for success (or other for failure)
******************************************************************************/
void DRM_Common_SwRsaPublicKeyFree(
		DrmCommon_RsaKey_t* rsa_key
);



/*****************************************************************************
** Summary:
** This routine reads the RSA private key from a input file in the PEM format.
**
** Description:
** This routine reads the RSA private key from a input file in the PEM format.  The input file
** should be in teh PEM format and the ouput is in the RSA key struct format and the size
** of the private key.
**
** Input:
** fp_privKeyIn - FILE*, the input file that contains the RSA private key in PEM format.
**
** Output:
** rsa_key -  DrmCommon_RsaKey_t, a ref/pointer to the RSA key.
** p_nSize - long*, a pointer to the size of the private key.
**
** Returns:
** 	Drm_Success for success (or other for failure)
******************************************************************************/
DrmRC DRM_Common_SwRSAReadPrivateKeyPem(
			FILE* fp_privKeyIn,
			DrmCommon_RsaKey_t* rsa_key,
			long* p_nSize
);



/*****************************************************************************
** Summary:
** This routine frees the memory used to store the RSA private key.
**
** Description:
** This routine frees the memory used to store the RSA private key.
**
** Input:
** rsa_key -  DrmCommon_RsaKey_t, a ref/pointer to the RSA private key.
**
** Returns:
** 	Drm_Success for success (or other for failure)
******************************************************************************/
void DRM_Common_SwRsaPrivateKeyFree(
		DrmCommon_RsaKey_t* rsa_key
);


/*****************************************************************************
** DRM_Common_ConvBinToStr
**
** Summary:
** Used to convert decimal or hex data to a string
**
** Input:
** data - unsigned char,  a ref/pointer to data to be converted .
** byteLen - unsigned long, length of data in bytes.
** drm_common_format - DrmCommonCovertFormat_eDecimal or DrmCommonCovertFormat_eHex.
**
** Output:
** outStr - buffer to contain output string
**
** Returns:
** 	Drm_Success for success (or other for failure)
******************************************************************************/
DrmRC DRM_Common_ConvBinToStr(
		uint8_t* data,
		uint32_t byteLen,
		DrmCommon_ConvertFormat_e drm_common_format,
		char* outStr);


/*****************************************************************************
** DRM_Common_SwDHFromPem
**
** Summary:
** This routine Allocates and Initializes a Diffie Hellman's instance.
**
** Description:
** This routine Allocates and Initializes a Diffie Hellman 's instance.
** Shared parameters are extracted from the given PEM string to initialize the DH instance.
** To release all ressources related to that instance, use DRM_Common_SwDHCleanup()
**
** Input:
** pem - void pointer,  a ref/pointer to read pem string from.
** pemLen - int, length of pem in bytes.
**
** Output:
** pHandle - pointer on handle, a ref/pointer to store the DH instance context handle
**
** Returns:
** 	Drm_Success for success (or other for failure)
******************************************************************************/
DrmRC DRM_Common_SwDHFromPem(const char * pem,
                             int pemLen,
                             void * *handle);


/*****************************************************************************
** DRM_Common_SwDHCleanup
**
** Summary:
** This routine Cleanups and Frees a Diffie Hellman's instance.
**
** Input:
** handle - void pointer, a ref/pointer that holds Diffie Hellman instance handle
**
** Returns:
** 	Drm_Success for success (or other for failure)
******************************************************************************/
DrmRC DRM_Common_SwDHCleanup(void * handle);


/*****************************************************************************
** DRM_Common_SwDHComputeSharedSecret
**
** Summary:
** This routine compute the Difie Hellman 's shared secret.
**
** Description:
** This routine compute the Difie Hellman 's shared secret.
** It uses remote public key against local DH's public key to extract shared secret.
**
** Input:
** handle - void pointer, a ref/pointer that holds Diffie Hellman instance handle
** remotePublicKey - const uint8_t *, a ref/pointer to remote public key
** keyLen - int, remote public key length in bytes
**
** Output:
** pSharedSecret - uint8_t *, ref/pointer to hold shared secret pointer
** pSharedSecretLen - int, shared secret length, in bytes
**
** Returns:
** 	Drm_Success for success (or other for failure)
******************************************************************************/
DrmRC DRM_Common_SwDHComputeSharedSecret(void * handle,
                                         const uint8_t * remotePublicKey,
                                         int keyLen,
                                         uint8_t ** pSharedSecret,
                                         int *pSharedSecretLen);


/*****************************************************************************
** DRM_Common_SwDHGetPublicKey
**
** Summary:
** This routine retrieve the Difie Hellman 's public key.
**
** Input:
** handle - void pointer, a ref/pointer that holds Diffie Hellman instance handle
**
** Output:
** pPublicKey - uint8_t *, ref/pointer to hold public key pointer
** pPublicKeyLen - int, public key length, in bytes
**
** Returns:
** 	Drm_Success for success (or other for failure)
******************************************************************************/
DrmRC DRM_Common_SwDHGetPublicKey(void * handle,
                                  uint8_t ** pPublicKey,
                                  int * pPublicKeyLen);


#endif /* DRM_COMMON_SWCRYPTO_H_ */
