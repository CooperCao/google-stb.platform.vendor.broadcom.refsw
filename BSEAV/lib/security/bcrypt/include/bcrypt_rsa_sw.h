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
#ifndef BCRYPT_RSA_H_
#define BCRYPT_RSA_H_



/***************************************************************************
Summary:
Struct to be used in the RSA key parameter.


Description:
This Structure is used for defining RSA key parameters. It includes an stream of data and
its length.


See Also:
BCRYPT_RSAKey

****************************************************************************/
typedef struct BCRYPT_RSAKey_Params
{
	unsigned char *pData;				/* In: A pointer to data */
	unsigned long len;					/* In: Length of data */
} BCRYPT_RSAKey_Params_t;


/***************************************************************************
Summary:
Parameters for defining RSA key.


Description:
This Structure defines the parameters RSA key. The main parameters are modulus n,
public exponent e, and private exponent d. In order to perfom RSA operations, at least
n, e, and d should contain valid values. The rest of parameters are used for perfoming RSA
operations using the Chinese Reminder Theorem. These are p, q, dmp1, dmq1, and iqmp.


See Also:
BCRYPT_RSAKey_Params, BCRYPT_S_RSASwParam

****************************************************************************/
typedef struct BCRYPT_RSAKey
{
	BCRYPT_RSAKey_Params_t	n;		/* In: Public key modulus */
	BCRYPT_RSAKey_Params_t	e;		/* In: Public exponent */
	BCRYPT_RSAKey_Params_t	d;		/* In: Private exponent */
	BCRYPT_RSAKey_Params_t	p;		/* In: 1st prime number */
	BCRYPT_RSAKey_Params_t	q;		/* In: 2nd prime number */
	BCRYPT_RSAKey_Params_t	dmp1;	/* In: Exponent corresponding to the 1st prime */
	BCRYPT_RSAKey_Params_t	dmq1;	/* In: Exponent corresponding to the 2nd prime */
	BCRYPT_RSAKey_Params_t	iqmp;	/* In: Inverse of q mod p */
} BCRYPT_RSAKey_t;



/***************************************************************************
Summary:
Parameters for RSA operations in software.


Description:
This Structure defines the parameters for perfoming RSA operations in software. In includes
RSA key, input data, size of the input data, output data, size of the output data, and the
rsa operation parameter. Different RSA operations that are defined using the rsaoperations
enum are RSA Enceyption, RSA Decrytion, RSA Decryption using CRT (Chinese Reminder
Theorem), RSA Sign, RSA Sign using CRT, and RSA Verify.


See Also:
BCRYPT_RSAKey_Params, BCRYPT_RSAKey

****************************************************************************/
typedef struct BCRYPT_S_RSASwParam
{
	BCRYPT_RSAKey_t	*key;		/* In: A pointer to RSA key (public or private key)*/
	/* In: This input specifies which RSA operation to be perfomed */
	enum rsaoperations {rsaenc, rsadec, rsadec_crt, rsasign, rsasign_crt, rsaverify} bRSAop ;
	unsigned char *pbDataIn;		/* In: A pointer to input data */
	unsigned long cbDataIn;		/* In: size of input data in bytes */
	/* In: A string that defines type of algorithm for RSA sign and verify */
       unsigned char *psAlgorithmId;
	/* In: Size of the string that defines type of algorithm for RSA sign and verify */
       unsigned long csAlgorithmId;	
	unsigned char *pbDataOut;		/* Out: A pointer to output data */
	unsigned long *cbDataOut;		/* Out: size of encrypted data in bytes*/
    enum rsapadtypes {PKCS1 =1,SSLV23 ,NOPAD, OAEP, X931, PSS }bRSAPadType;
} BCRYPT_S_RSASwParam_t;



/* Basic Module Functions */


/*****************************************************************************
Summary:
This function implements different RSA operations in SW.


Description:
This function implements different RSA operations in software.
The inputs include RSA key, input data, size of the input data, output data, size of the output
data, and the rsa operation parameter.
Different RSA operations that are supported are RSA Enceyption, RSA Decrytion, RSA
Decryption using CRT (Chinese Reminder Theorem), RSA Sign, RSA Sign using CRT, and
RSA Verify.
The RSA key should at least contain the valid values for n, e, and d, which are required for
the RSA operations. However, if the RSA operation based on the Chinese Reminder Theorem
is neede, then besides n, e, and d, the values of p, q, dmp1, dmq1, and iqmp should be
initialized too. The RSA CRT is faster than the regular RSA algorithm.


Calling Context:
The function shall be called from application level or from driver level, for example in Linux
during insmod.


Performance and Timing:
This is a synchronous/blocking function that will not return until it is done or failed.


Input:
hBcrypt  - BCRYPT_Handle
pInputParam - BCRYPT_S_RSASwParam_t, a ref/pointer to the parameters for SW
			RSA operations.

			
Returns:
BCRYPT_STATUS_eOK - success
BCRYPT_STATUS_eFAILED - failure


See Also:
BCRYPT_RSAKey_Params, BCRYPT_RSAKey, BCRYPT_S_RSASwParam

******************************************************************************/
BCRYPT_STATUS_eCode BCrypt_RSASw (	BCRYPT_Handle  hBcrypt, 				
					BCRYPT_S_RSASwParam_t *pInputParam );


/*****************************************************************************
Summary:
This function populates RSA public key from the subject public key info.

Description:
Below is ASN1 encoded structure of Public key info. Publickey bit string is
extracted and converted to RSA structure from which the requisite modulus and
exponent are populated to output.

	PublicKeyInfo ::= SEQUENCE {
	algorithm AlgorithmIdentifier,
	PublicKey BIT STRING
	}
	AlgorithmIdentifier ::= SEQUENCE {
	algorithm ALGORITHM.id,
	parameters ALGORITHM.type OPTIONAL
	}
	Depending on the algorithmid, the publick key will be either rsa public key or dsa public key.
	Rsa public key again will be as follows

	RSAPublicKey ::= SEQUENCE {
	modulus INTEGER,
	publicExponent INTEGER
	}

Calling Context:
The function shall be called from application level or from driver level, for example in Linux
during insmod.


Performance and Timing:
This is a synchronous/blocking function that will not return until it is done or failed.


Input:
hBcrypt  - BCRYPT_Handle
buf - ref/pointer to input key info
len - length of key info

Output:
pBcrypt_rsaSw - BCRYPT_RSAKey_t, a ref/pointer to the parameters for SW
			RSA operations.
Returns:
BCRYPT_STATUS_eOK - success
BCRYPT_STATUS_eFAILED - failure


See Also:

******************************************************************************/
BCRYPT_STATUS_eCode BCrypt_GetRSA_From_SubjectPublicKeyInfo(	BCRYPT_Handle  hBcrypt,
									uint8_t *pBuf, uint32_t len,
									BCRYPT_RSAKey_t *pBcrypt_rsaSw);

/*****************************************************************************
Summary:
This function implements different RSA operations in SW.


Description:
Below is the ASN1 structure of private key info and the RSAPrivateKey
PrivateKeyInfo ::= SEQUENCE {
       version INTEGER,
       privateKeyAlgorithm AlgorithmIdentifier {{PrivateKeyAlgorithms}},
       privateKey OCTET STRING,
       attributes [0] SET OF Attribute OPTIONAL // NOT SUPPORTING ATTRIBUTES
   }
RSAPrivateKey ::= SEQUENCE {
       version           INTEGER,
       modulus           INTEGER,  -- n
       publicExponent    INTEGER,  -- e
       privateExponent   INTEGER,  -- d
       prime1            INTEGER,  -- p
       prime2            INTEGER,  -- q
       exponent1         INTEGER,  -- d mod (p-1)
       exponent2         INTEGER,  -- d mod (q-1)
       coefficient       INTEGER,  -- (inverse of q) mod p
       otherPrimeInfos   OtherPrimeInfos OPTIONAL
   }

Calling Context:
The function shall be called from application level or from driver level, for example in Linux
during insmod.


Performance and Timing:
This is a synchronous/blocking function that will not return until it is done or failed.



Input:
hBcrypt  - BCRYPT_Handle
buf - ref/pointer to input key info
len - length of key info

Output:
pBcrypt_rsaSw - BCRYPT_RSAKey_t, a ref/pointer to the parameters for SW
			RSA operations.

Returns:
BCRYPT_STATUS_eOK - success
BCRYPT_STATUS_eFAILED - failure


See Also:

******************************************************************************/
BCRYPT_STATUS_eCode BCrypt_GetRSA_From_PrivateKeyInfo(BCRYPT_Handle  hBcrypt,
                                                      uint8_t *pBuf, uint32_t len,
                                                      BCRYPT_RSAKey_t *pBcrypt_rsaSw);

#endif /* BCRYPT_RSA_H_ */
