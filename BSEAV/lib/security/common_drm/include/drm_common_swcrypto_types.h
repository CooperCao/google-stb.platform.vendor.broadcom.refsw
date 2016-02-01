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

#ifndef DRM_COMMON_TYPES_H_
#define DRM_COMMON_TYPES_H_

#include <openssl/rc4.h>
#include <openssl/rsa.h>

#define DRM_COMMON_MAX_THREAD_SUPPORT	(16)

#define DRM_COMMON_SHA1_DIGEST_SIZE   (20)

typedef struct DrmCommonSha1_t
{
	uint32_t curr_free_ctx_num;
	uint32_t thread_table[DRM_COMMON_MAX_THREAD_SUPPORT];
}DrmCommonSha1_t;

/***************************************************************************
** Summary:
** Partial SHA-1 enum
**
** Description:
** The called passes DrmCommon_PartialSha1Type_Init for the first packet to
** be hashed, DrmCommon_PartialSha1Type_Update for the middle packets and
** DrmCommon_PartialSha1Type_Finalize for the last one.
****************************************************************************/
typedef enum DrmCommon_PartialSha1Type
{
	DrmCommon_PartialSha1Type_Init = 0,
	DrmCommon_PartialSha1Type_Update = 1,
	DrmCommon_PartialSha1Type_Finalize = 2
}DrmCommon_PartialSha1Type;

/***************************************************************************
** Summary:
** Parameters for Elliptic Curve.
**
** Description:
** This Structure defines the parameters for Elliptic Curve. It contains modulus P, elliptic curve
** coefficient A and B, the base point G with components X and Y, and the order of base point G.
****************************************************************************/
typedef struct DrmCommon_ECCParam
{
	char	*P;				/* In: Modulus */
	char	*A;				/* In: Elliptic curve coefficient a */
	char	*B;				/* In: Elliptic curve coefficient b */
	char	*G_x;			/* In: Base point, X component */
	char	*G_y;			/* In: Base point, Y component */
	char	*N;				/* In: Order of base point G */
} DrmCommon_ECCParam_t;


/***************************************************************************
** Summary:
** Parameters for a point on an EC curve.
**
** Description:
** This Structure defines the parameters a point on an EC curve. It contains the components
** x and y of the point on an EC curve.
****************************************************************************/
typedef struct DrmCommon_ECCPoint
{
	char	*x;				/* In: Component x of the point in EC curve*/
	char	*y;				/* In: Component y of the point in EC curve*/
} DrmCommon_ECCPoint_t;


/*
 * Signature
 */
typedef enum DrmCommon_ECCSig_Format_t
{
	DrmCommon_ECCSig_Format_Decimal = 0,
	DrmCommon_ECCSig_Format_Hex
}	DrmCommon_ECCSig_Format_t;


/***************************************************************************
Summary:
Parameters for a ECC signature.

Description:
This Structure defines the parameters for ECC signature. It contains r, s (signature) and the
signature format.

See Also:
DrmCommon_ECCParam, DrmCommon_ECCPoint
****************************************************************************/
typedef struct DrmCommon_ECCSig
{
	char					*r;				/* In: Component r of ECC signature*/
	char					*s;				/* In: Component s of ECC signature*/
	DrmCommon_ECCSig_Format_t	format;		/* In: Signature format*/
} DrmCommon_ECCSig_t;


/*
 * Public key
 */
typedef DrmCommon_ECCPoint_t DrmCommon_ECCPublicKey_t;



/***************************************************************************
Summary:
Parameters for a ECDSA Verify.

Description:
This Structure defines the parameters for ECDSA Verify. It contains EC curve parameters,
signature, public key, hash string and its length, and a flag indicating if signature is valid.

See Also:
DrmCommon_ECCParam, DrmCommon_ECCPoint, DrmCommon_ECCSig
DrmCommon_ECDSASw_Sign, DrmCommon_ECDSASw_Multiply
****************************************************************************/
typedef struct DrmCommon_ECDSASw_Verify_t
{
	DrmCommon_ECCParam_t		eccParm; 	/* In: EC curve parameters */
	DrmCommon_ECCSig_t			sig;			/* In: ECC Signature */
	DrmCommon_ECCPublicKey_t	publicKey;	/* In: Public key */
	unsigned char			*hash;			/* In: A pointer to hash string */
	unsigned char			hashLen;		/* In: Length of hash */
	bool					sigIsValid;		/* Out: A flag indicating signature is valid */
}  DrmCommon_ECDSASw_Verify_t;



/***************************************************************************
Summary:
Parameters for a ECDSA Sign.

Description:
This Structure defines the parameters for ECDSA Sign. It contains EC curve parameters,
signature, private key, hash string and its length, and a random integer.

See Also:
DrmCommon_ECCParam, DrmCommon_ECCPoint, DrmCommon_ECCSig
DrmCommon_ECDSASw_Verify, DrmCommon_ECDSASw_Multiply
****************************************************************************/
typedef struct DrmCommon_ECDSASw_Sign
{
	DrmCommon_ECCParam_t		eccParm; 	/* In: EC curve parameters */
	DrmCommon_ECCSig_t			sig;			/* In: ECC Signature */
	unsigned char			*hash;			/* In: A pointer to hash string */
	unsigned char			hashLen;			/* In: Length of hash */
	char					*k;				/* In: A random integer k */
	char					*privateKey;		/* In: Private key */
}  DrmCommon_ECDSASw_Sign_t;



/***************************************************************************
Summary:
Parameters for a  EC Scalar Multiplication.

Description:
This Structure defines the parameters for  EC Scalar Multiplication. It contains EC curve parameters,
scalar, input point, and the output point.

See Also:
DrmCommon_ECCParam, DrmCommon_ECCPoint, DrmCommon_ECCSig
DrmCommon_ECDSASw_Sign, DrmCommon_ECDSASw_Verify
****************************************************************************/
typedef struct DrmCommon_ECDSASw_Multiply
{
	DrmCommon_ECCParam_t	eccParm; 	/* In: EC curve parameters */
	char					*scalar;		/* In: Scalar value */
	DrmCommon_ECCPoint_t	inPoint;		/* In: Input point */
	DrmCommon_ECCPoint_t	outPoint;	/* Out: output point, result of Scalar Multiplication */
}  DrmCommon_ECDSASw_Multiply_t;


/********************************************************************************************/
/*							AES definitions													*/
/********************************************************************************************/
typedef struct DrmCommon_AesEcbSw_t{
	unsigned char 	*pIn;	/* In: A pointer to the input data steam */
	unsigned char *pkey; 		/* In: A pointer to the encryption key */
	unsigned long 	len;			/* In: Length of the input data steam */
	bool bEncFlag; 			/* In: Encrypt flag: 1: encrypt, 0: decrypt */
	unsigned long keybits;  	/* In: key length in bits. 128, 192, 256 only.*/

	unsigned char  *pOut;		/* Out: A pointer to the encrypted output data */

} DrmCommon_AesEcbSw_t;

typedef struct DrmCommon_AesCbcSw_t
{
	unsigned char 	*pIn;	/* In: A pointer to the input data steam */
	unsigned char *piv;  		/* In: A pointer to the CBC initialization vector. */
	unsigned char *pkey; 		/* In: A pointer to the encryption key */
	unsigned long 	len;			/* In: Length of the input data steam */
	bool bEncFlag; 			/* In: Encrypt flag: 1: encrypt, 0: decrypt */
	unsigned long keybits;  	/* In: key length in bits. 128, 192, 256 only.*/

	unsigned char  *pOut;		/* Out: A pointer to the encrypted output data */

} DrmCommon_AesCbcSw_t;


/********************************************************************************************/
/*							CMAC definitions												*/
/********************************************************************************************/
typedef struct DrmCommon_CmacSw_t
{
	unsigned char *pBuffer;		/* In: A pointer to the message buffer */
	unsigned long length;		/* In: the message length. */
	unsigned char *pKey;		/* In: A pointer to the input key. */
	unsigned char *pMac;		/* Out: A pointer to the MAC value. */
} DrmCommon_CmacSw_t;


/********************************************************************************************/
/*							DES definitions												    */
/********************************************************************************************/
typedef struct DrmCommon_DesEcbSw_t{
	unsigned char 	*pIn;	/* In: A pointer to the input data steam */
	unsigned char *pkey; 		/* In: A pointer to the encryption key */
	unsigned long 	len;			/* In: Length of the input data steam */
	bool bEncFlag; 			/* In: Encrypt flag: 1: encrypt, 0: decrypt */

	unsigned char  *pOut;		/* Out: A pointer to the encrypted output data */
} DrmCommon_DesEcbSw_t;

typedef struct DrmCommon_DesCbcSw_t{
	unsigned char 	*pIn;	/* In: A pointer to the input data steam */
	unsigned char *piv;  		/* In: A pointer to the CBC initialization vector. */
	unsigned char *pkey; 		/* In: A pointer to the encryption key */
	unsigned long 	len;			/* In: Length of the input data steam */
	bool bEncFlag; 			/* In: Encrypt flag: 1: encrypt, 0: decrypt */

	unsigned char  *pOut;		/* Out: A pointer to the encrypted output data */
} DrmCommon_DesCbcSw_t;


/********************************************************************************************/
/*							DSA definitions												    */
/********************************************************************************************/
typedef struct DrmCommon_DsaKeyParam_t
{
	unsigned char *pData;				/* In: A pointer to data */
	unsigned long len;					/* In: Length of data */
} DrmCommon_DsaKeyParam_t;

typedef struct DrmCommon_DsaKey_t
{
	DrmCommon_DsaKeyParam_t	p;			/* In: 1st prime number */
	DrmCommon_DsaKeyParam_t	q;			/* In: 2nd prime number */
	DrmCommon_DsaKeyParam_t	g;			/* In: Generator  */
	DrmCommon_DsaKeyParam_t	pubkey;		/* In: public key */
	DrmCommon_DsaKeyParam_t	privkey;		/* In: private key */
	DrmCommon_DsaKeyParam_t	k;			/* In:  kinv, Inverse of k */
} DrmCommon_DsaKey_t;

typedef struct DrmCommon_DsaSw_t
{
	DrmCommon_DsaKey_t	*key;		/* In: A pointer to RSA key (public or private key)*/
	bool	bDSASign;				/* In: DSA operation, true for sign, false for verify */
	unsigned char *pbDataIn;		/* In: A pointer to input data */
	int cbDataIn;					/* In: size of input data in bytes */
	unsigned char sigout[256];		/* Out: A pointer to output signature */
	unsigned int sigoutlen;			/* Out: size of output signature in bytes */
	unsigned char *sigin_r;		/* In: The r value of the signature to be verified */
	unsigned char *sigin_s;		/* In: The s value of the signature to be verified */

} DrmCommon_DsaSw_t;

/********************************************************************************************/
/*							MD5 definitions												    */
/********************************************************************************************/
typedef struct DrmCommon_Md5Sw_t
{
	uint8_t		*pbInData;			/* In: A pointer to the  input data */
	uint32_t	cbInData;				/* In: The number of bytes of input data */
	uint8_t		*pbOutData;			/* Out: A pointer to output MD5 digest  */
} DrmCommon_Md5Sw_t;


/********************************************************************************************/
/*							RC4 definitions												    */
/********************************************************************************************/
#define DrmCommon_Rc4Key_t RC4_KEY
typedef struct DrmCommon_Rc4Sw_t
{
	DrmCommon_Rc4Key_t	*key;			/* In: A pointer to RC4 key */
	unsigned long cbLen;				/* In: Size of the input data in bytes */
	unsigned char *pbDataIn;			/* In: A pointer to input data */
	unsigned char *pbDataOut;			/* Out: A pointer to output data */
} DrmCommon_Rc4Sw_t;

/********************************************************************************************/
/*							RNG definitions												    */
/********************************************************************************************/
typedef struct DrmCommon_RngSw_t
{
	unsigned char *pucBuf; 		/* In: A pointer to the data buffer. */
	unsigned long ulLen; 			/* In: Length of random number in bytes. */
} DrmCommon_RngSw_t;



/****************************************************************************/
/*					RSA definitions											*/
/****************************************************************************/
typedef struct DrmCommon_RsaKey_Params_t
{
	unsigned char *pData;				/* In: A pointer to data */
	unsigned long len;					/* In: Length of data */
} DrmCommon_RsaKey_Params_t;



typedef struct DrmCommon_RsaKey_t
{
	DrmCommon_RsaKey_Params_t	n;		/* In: Public key modulus */
	DrmCommon_RsaKey_Params_t	e;		/* In: Public exponent */
	DrmCommon_RsaKey_Params_t	d;		/* In: Private exponent */
	DrmCommon_RsaKey_Params_t	p;		/* In: 1st prime number */
	DrmCommon_RsaKey_Params_t	q;		/* In: 2nd prime number */
	DrmCommon_RsaKey_Params_t	dmp1;	/* In: Exponent corresponding to the 1st prime */
	DrmCommon_RsaKey_Params_t	dmq1;	/* In: Exponent corresponding to the 2nd prime */
	DrmCommon_RsaKey_Params_t	iqmp;	/* In: Inverse of q mod p */
} DrmCommon_RsaKey_t;


typedef enum DrmCommon_RSAPaddingType_e
{
    DrmCommon_RSAPaddingType_ePKCS1         = 1,
    DrmCommon_RSAPaddingType_eSSLV23        = 2,
    DrmCommon_RSAPaddingType_eNOPADDING     = 3,
    DrmCommon_RSAPaddingType_eOAEP          = 4,
    DrmCommon_RSAPaddingType_eX931          = 5,
    DrmCommon_RSAPaddingType_ePSS           = 6,
    DrmCommon_RSAPaddingType_eMax
}	DrmCommon_RSAPaddingType_e;


typedef struct DrmCommon_RsaSwParam_t
{
	DrmCommon_RsaKey_t	*key;		/* In: A pointer to RSA key (public or private key)*/
	/* In: This input specifies which RSA operation to be perfomed */
	enum drmRsaOperations {drmRsaenc, drmRsadec, drmRsadec_crt, drmRsasign, drmRsasign_crt, drmRsaverify} bRSAop ;
	unsigned char *pbDataIn;		/* In: A pointer to input data */
	unsigned long cbDataIn;		/* In: size of input data in bytes */
	/* In: A string that defines type of algorithm for RSA sign and verify */
    unsigned char *psAlgorithmId;
	/* In: Size of the string that defines type of algorithm for RSA sign and verify */
    unsigned long csAlgorithmId;
	unsigned char *pbDataOut;		/* Out: A pointer to output data */
	unsigned long *cbDataOut;		/* Out: size of encrypted data in bytes*/
    DrmCommon_RSAPaddingType_e padType;
} DrmCommon_RsaSwParam_t;

typedef enum DrmCommon_ConvertFormat_e
{
	DrmCommon_CovertFormat_eDecimal = 0,
	DrmCommon_CovertFormat_eHex
}	DrmCommon_ConvertFormat_e;

#endif /* DRM_COMMON_TYPES_H_ */
