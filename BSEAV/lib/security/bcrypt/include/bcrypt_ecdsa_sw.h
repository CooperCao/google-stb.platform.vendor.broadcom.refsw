/***************************************************************************
 *     (c)2006-2011 Broadcom Corporation
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
 * Module Description: 
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#ifndef BCRYPT_ECDSA_SW_H__
#define BCRYPT_ECDSA_SW_H__



/***************************************************************************
Summary:
Parameters for Elliptic Curve.


Description:
This Structure defines the parameters for Elliptic Curve. It contains modulus P, elliptic curve
coefficient A and B, the base point G with components X and Y, and the order of base point
G. 


See Also:
BCRYPT_ECCPoint

****************************************************************************/
typedef struct BCRYPT_ECCParam
{
	char	*P;				/* In: Modulus */
	char	*A;				/* In: Elliptic curve coefficient a */
	char	*B;				/* In: Elliptic curve coefficient b */
	char	*G_x;			/* In: Base point, X component */
	char	*G_y;			/* In: Base point, Y component */
	char	*N;				/* In: Order of base point G */
} BCRYPT_ECCParam_t;


/***************************************************************************
Summary:
Parameters for a point on an EC curve.


Description:
This Structure defines the parameters a point on an EC curve. It contains the components 
x and y of the point on an EC curve.


See Also:
BCRYPT_ECCParam

****************************************************************************/
typedef struct BCRYPT_ECCPoint		
{
	char	*x;				/* In: Component x of the point in EC curve*/
	char	*y;				/* In: Component y of the point in EC curve*/
} BCRYPT_ECCPoint_t;


/*
 * Signature
 */
typedef enum
{
	BCRYPT_ECCSig_Format_Decimal = 0,
	BCRYPT_ECCSig_Format_Hex
}	BCRYPT_ECCSig_Format_t;


/***************************************************************************
Summary:
Parameters for a ECC signature.


Description:
This Structure defines the parameters for ECC signature. It contains r, s (signature) and the 
signature format.


See Also:
BCRYPT_ECCParam, BCRYPT_ECCPoint

****************************************************************************/
typedef struct BCRYPT_ECCSig
{
	char					*r;				/* In: Component r of ECC signature*/
	char					*s;				/* In: Component s of ECC signature*/
	BCRYPT_ECCSig_Format_t	format;		/* In: Signature format*/
} BCRYPT_ECCSig_t;


/*
 * Public key
 */
typedef BCRYPT_ECCPoint_t BCRYPT_ECCPublicKey_t;



/***************************************************************************
Summary:
Parameters for a ECDSA Verify.


Description:
This Structure defines the parameters for ECDSA Verify. It contains EC curve parameters, 
signature, public key, hash string and its length, and a flag indicating if signature is valid.


See Also:
BCRYPT_ECCParam, BCRYPT_ECCPoint, BCRYPT_ECCSig
BCRYPT_ECDSASw_Sign, BCRYPT_ECDSASw_Multiply

****************************************************************************/
typedef struct BCRYPT_ECDSASw_Verify 
{
	BCRYPT_ECCParam_t		eccParm; 	/* In: EC curve parameters */
	BCRYPT_ECCSig_t			sig;			/* In: ECC Signature */
	BCRYPT_ECCPublicKey_t	publicKey;	/* In: Public key */
	unsigned char			*hash;			/* In: A pointer to hash string */
	unsigned char			hashLen;		/* In: Length of hash */
	bool					sigIsValid;		/* Out: A flag indicating signature is valid */
}  BCRYPT_ECDSASw_Verify_t;



/***************************************************************************
Summary:
Parameters for a ECDSA Sign.


Description:
This Structure defines the parameters for ECDSA Sign. It contains EC curve parameters, 
signature, private key, hash string and its length, and a random integer.


See Also:
BCRYPT_ECCParam, BCRYPT_ECCPoint, BCRYPT_ECCSig
BCRYPT_ECDSASw_Verify, BCRYPT_ECDSASw_Multiply

****************************************************************************/
typedef struct BCRYPT_ECDSASw_Sign
{
	BCRYPT_ECCParam_t		eccParm; 	/* In: EC curve parameters */
	BCRYPT_ECCSig_t			sig;			/* In: ECC Signature */
	unsigned char			*hash;			/* In: A pointer to hash string */
	unsigned char			hashLen;			/* In: Length of hash */
	char					*k;				/* In: A random integer k */
	char					*privateKey;		/* In: Private key */
}  BCRYPT_ECDSASw_Sign_t;



/***************************************************************************
Summary:
Parameters for a  EC Scalar Multiplication.


Description:
This Structure defines the parameters for  EC Scalar Multiplication. It contains EC curve parameters, 
scalar, input point, and the output point.

See Also:
BCRYPT_ECCParam, BCRYPT_ECCPoint, BCRYPT_ECCSig
BCRYPT_ECDSASw_Sign, BCRYPT_ECDSASw_Verify

****************************************************************************/
typedef struct BCRYPT_ECDSASw_Multiply
{
	BCRYPT_ECCParam_t	eccParm; 	/* In: EC curve parameters */
	char					*scalar;		/* In: Scalar value */
	BCRYPT_ECCPoint_t	inPoint;		/* In: Input point */
	BCRYPT_ECCPoint_t	outPoint;	/* Out: output point, result of Scalar Multiplication */
}  BCRYPT_ECDSASw_Multiply_t;




/* ECDSA functions */


/*****************************************************************************
Summary:
This function implements the ECDSA Verify algorithm in SW.


Description:
This function implements the ECDSA Verify algorithm in software. It is based on the ECDSA 
implementation of the openssl code. The binaries of the openssl library is already included 
into the build environment.


Calling Context:
The function shall be called from application level or from driver level, for example in Linux
during insmod.


Performance and Timing:
This is a synchronous/blocking function that will not return until it is done or failed.


Input:
hBcrypt  - BCRYPT_Handle
inoutp_ecdsaSwIO - BCRYPT_ECDSASw_Verify_t, a ref/pointer to the parameters 
		for the SW implementation of the ECDSA Verify algorithm. 

			
Returns:
BCRYPT_STATUS_eOK - success
BCRYPT_STATUS_eFAILED - failure 


See Also:
BCrypt_ECDSASignSw, BCrypt_ECDSAMultiplySw

******************************************************************************/
BCRYPT_STATUS_eCode BCrypt_ECDSAVerifySw( 
		BCRYPT_Handle 				hBcrypt,
		BCRYPT_ECDSASw_Verify_t 	*inoutp_ecdsaSwIO
);



/*****************************************************************************
Summary:
This function implements the ECDSA Sign algorithm in SW.


Description:
This function implements the ECDSA Sign algorithm in software. It is based on the ECDSA 
implementation of the openssl code. The binaries of the openssl library is already included 
into the build environment.


Calling Context:
The function shall be called from application level or from driver level, for example in Linux
during insmod.


Performance and Timing:
This is a synchronous/blocking function that will not return until it is done or failed.


Input:
hBcrypt  - BCRYPT_Handle
inoutp_ecdsaSwIO - BCRYPT_ECDSASw_Sign_t, a ref/pointer to the parameters 
		for the SW implementation of the ECDSA Sign algorithm. 

			
Returns:
BCRYPT_STATUS_eOK - success
BCRYPT_STATUS_eFAILED - failure 


See Also:
BCrypt_ECDSAVerifySw, BCrypt_ECDSAMultiplySw

******************************************************************************/
BCRYPT_STATUS_eCode BCrypt_ECDSASignSw( 
		BCRYPT_Handle 				hBcrypt,
		BCRYPT_ECDSASw_Sign_t 		*inoutp_ecdsaSwIO
);


/*****************************************************************************
Summary:
This function cleans up the ECDSA paramter allocated by OpenSSL.


Description:
This function cleans the ECDSA paramter allocated by OpenSSL. Specifically, during the 
call to ECDSASignSw the function allocates memory for the sig.r or sig.s
using the BN_bn2dec or BN_bn2hex these needs to be freed by calling the OPNESSL_free once.
Thius function allows the user of the ECDSASignSw to free up these memory
once it has done using them. 

Calling Context:
The function shall be called from application level or from driver level, for example in Linux
during insmod.

Performance and Timing:
This is a synchronous/blocking function that will not return until it is done or failed.


Input:
hBcrypt  - BCRYPT_Handle
inoutp_ecdsaSwIO - BCRYPT_ECDSASw_Sign_t, a ref/pointer to the parameters 
		for the SW implementation of the ECDSA Sign algorithm. 

			
Returns:
BCRYPT_STATUS_eOK - success
BCRYPT_STATUS_eFAILED - failure 


See Also:
BCrypt_ECDSAVerifySw, BCrypt_ECDSAMultiplySw

******************************************************************************/
BCRYPT_STATUS_eCode BCrypt_Sign_ECDSAClean( 
		BCRYPT_Handle 				hBcrypt,
		BCRYPT_ECDSASw_Sign_t 		*inoutp_ecdsaSwIO
);



/*****************************************************************************
Summary:
This function cleans up the ECDSA paramter allocated by OpenSSL.


Description:
This function cleans the ECDSA paramter allocated by OpenSSL. Specifically, during the 
call to ECDSAMultiplySw the function allocates memory for the sig.r or sig.s
using the BN_bn2dec or BN_bn2hex these needs to be freed by calling the OPNESSL_free once.
Thius function allows the user of the ECDSAMultiplySw to free up these memory
once it has done using them. 

Calling Context:
The function shall be called from application level or from driver level, for example in Linux
during insmod.

Performance and Timing:
This is a synchronous/blocking function that will not return until it is done or failed.


Input:
hBcrypt  - BCRYPT_Handle
inoutp_ecdsaSwIO - BCRYPT_ECDSASw_Multiply_t, a ref/pointer to the parameters 
		for the SW implementation of the ECDSA Sign algorithm. 

			
Returns:
BCRYPT_STATUS_eOK - success
BCRYPT_STATUS_eFAILED - failure 


See Also:
BCrypt_Sign_ECDSAClean

******************************************************************************/
BCRYPT_STATUS_eCode BCrypt_Multiply_ECDSAClean( 
		BCRYPT_Handle 				hBcrypt,
		BCRYPT_ECDSASw_Multiply_t 		*inoutp_ecdsaSwIO
);




/*****************************************************************************
Summary:
This function implements the EC Scalar Multiplication algorithm in SW.


Description:
This function implements theEC Scalar Multiplication algorithm in software. It is based on the 
EC Scalar Multiplication implementation of the openssl code. The binaries of the openssl library 
is already included into the build environment.


Calling Context:
The function shall be called from application level or from driver level, for example in Linux
during insmod.


Performance and Timing:
This is a synchronous/blocking function that will not return until it is done or failed.


Input:
hBcrypt  - BCRYPT_Handle
inoutp_ecdsaSwIO - BCRYPT_ECDSASw_Multiply_t, a ref/pointer to the parameters 
		for the SW implementation of the EC Scalar Multiplication algorithm. 

			
Returns:
BCRYPT_STATUS_eOK - success
BCRYPT_STATUS_eFAILED - failure 


See Also:
BCrypt_ECDSASignSw, BCrypt_ECDSAVerifySw

******************************************************************************/
BCRYPT_STATUS_eCode BCrypt_ECDSAMultiplySw( 
		BCRYPT_Handle 				hBcrypt,
		BCRYPT_ECDSASw_Multiply_t 	*inoutp_ecdsaSwIO
);



/* Number conversion utility functions */
typedef enum
{
	BCRYPT_String_Format_Decimal = 0,
	BCRYPT_String_Format_Hex
}	BCRYPT_String_Format_t;



/*****************************************************************************
Summary:
This function Converts Binary to String.


Description:
This function Converts Binary to String. It is based on the implementation of the openssl code. 
The binaries of the openssl library is already included into the build environment.


Calling Context:
The function shall be called from application level or from driver level, for example in Linux
during insmod.


Performance and Timing:
This is a synchronous/blocking function that will not return until it is done or failed.


Input:
hBcrypt  - BCRYPT_Handle
data - unsigned char,  a ref/pointer to data to be converted .
byteLen - unsigned long, length of data in bytes.
format - BCRYPT_String_Format_t,  format of input string. 


Output:
outStr - char, a ref/pointer to output string.

			
Returns:
BCRYPT_STATUS_eOK - success
BCRYPT_STATUS_eFAILED - failure 


See Also:
BCrypt_ConvStrToBin

******************************************************************************/
BCRYPT_STATUS_eCode BCrypt_ConvBinToStr( 
		BCRYPT_Handle 				hBcrypt,	
		unsigned char 				*data,		
		unsigned long				byteLen,	
		BCRYPT_String_Format_t		format,		
		char						*outStr	
);


/*****************************************************************************
Summary:
This function Converts String to Binary.


Description:
This function Converts String to Binary. It is based on the implementation of the openssl code. 
The binaries of the openssl library is already included into the build environment.


Calling Context:
The function shall be called from application level or from driver level, for example in Linux
during insmod.


Performance and Timing:
This is a synchronous/blocking function that will not return until it is done or failed.


Input:
hBcrypt  - BCRYPT_Handle
inStr - char, a ref/pointer to input string.
format - BCRYPT_String_Format_t,  format of input string. 


Output:
data - unsigned char,  a ref/pointer to binary data.
byteLen - unsigned long, a ref/pointer to the length of data in bytes.

			
Returns:
BCRYPT_STATUS_eOK - success
BCRYPT_STATUS_eFAILED - failure 


See Also:
BCrypt_ConvBinToStr

******************************************************************************/
BCRYPT_STATUS_eCode BCrypt_ConvStrToBin( 
		BCRYPT_Handle 				hBcrypt,	
		char						*inStr,		
		BCRYPT_String_Format_t		format,		
		unsigned char 				*data,		
		unsigned long				*byteLen	
);

#endif /* BCRYPT_ECDSA_SW_H__ */
